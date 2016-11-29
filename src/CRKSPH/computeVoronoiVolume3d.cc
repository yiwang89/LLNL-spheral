//---------------------------------Spheral++------------------------------------
// Compute the volume per point based on the Voronoi tessellation.
//------------------------------------------------------------------------------
extern "C" {
#include "r3d/r3d.h"
}

#include <algorithm>

#include "computeVoronoiVolume.hh"
#include "Field/Field.hh"
#include "Field/FieldList.hh"
#include "NodeList/NodeList.hh"
#include "Neighbor/ConnectivityMap.hh"
#include "Utilities/allReduce.hh"
#include "Utilities/pointOnPolyhedron.hh"
#include "Utilities/FastMath.hh"

namespace Spheral {
namespace CRKSPHSpace {

using namespace std;

using namespace FastMath;

using FieldSpace::Field;
using FieldSpace::FieldList;
using NodeSpace::NodeList;
using NeighborSpace::Neighbor;
using NeighborSpace::ConnectivityMap;

namespace {  // anonymous namespace

//------------------------------------------------------------------------------
// A special comparator to sort r3d planes by distance.
//------------------------------------------------------------------------------
bool compareR3Dplanes(const r3d_plane& lhs, const r3d_plane& rhs) {
  return lhs.d < rhs.d;
}

//------------------------------------------------------------------------------
// Find the 1D extent of an R3D cell along the given direction.
//------------------------------------------------------------------------------
void findPolyhedronExtent(double& xmin, double& xmax, const Dim<3>::Vector& nhat, const r3d_poly& celli) {
  REQUIRE(fuzzyEqual(nhat.magnitude(), 1.0));
  const unsigned nverts = celli.nverts;
  double xi;
  xmin = std::numeric_limits<double>::max();
  xmax = -std::numeric_limits<double>::max();
  for (unsigned i = 0; i != nverts; ++i) {
    xi = (celli.verts[i].pos.x * nhat.x() +
          celli.verts[i].pos.y * nhat.y() +
          celli.verts[i].pos.z * nhat.z());
    xmin = std::min(xmin, xi);
    xmax = std::max(xmax, xi);
  }
  xmin = std::min(0.0, xmin);
  xmax = std::max(0.0, xmax);
}

}           // anonymous namespace

//------------------------------------------------------------------------------
// 3D
//------------------------------------------------------------------------------
void
computeVoronoiVolume(const FieldList<Dim<3>, Dim<3>::Vector>& position,
                     const FieldList<Dim<3>, Dim<3>::SymTensor>& H,
                     const FieldSpace::FieldList<Dim<3>, Dim<3>::Scalar>& rho,
                     const FieldSpace::FieldList<Dim<3>, Dim<3>::Vector>& gradRho,
                     const ConnectivityMap<Dim<3> >& connectivityMap,
                     const Dim<3>::Scalar kernelExtent,
                     const std::vector<Dim<3>::FacetedVolume>& boundaries,
                     const std::vector<std::vector<Dim<3>::FacetedVolume> >& holes,
                     FieldList<Dim<3>, int>& surfacePoint,
                     FieldList<Dim<3>, Dim<3>::Scalar>& vol,
                     FieldSpace::FieldList<Dim<3>, Dim<3>::Vector>& deltaMedian,
                     FieldSpace::FieldList<Dim<3>, Dim<3>::FacetedVolume>& cells) {

  typedef Dim<3>::Scalar Scalar;
  typedef Dim<3>::Vector Vector;
  typedef Dim<3>::SymTensor SymTensor;
  typedef Dim<3>::FacetedVolume FacetedVolume;
  typedef Dim<3>::FacetedVolume::Facet Facet;

  const unsigned numGens = position.numNodes();
  const unsigned numNodeLists = position.size();
  const unsigned numGensGlobal = allReduce(numGens, MPI_SUM, Communicator::communicator());
  const unsigned numBounds = boundaries.size();
  const bool haveBoundaries = numBounds == numNodeLists;
  const bool returnSurface = surfacePoint.size() == numNodeLists;
  const bool returnCells = cells.size() == numNodeLists;

  REQUIRE(numBounds == 0 or numBounds == numNodeLists);

  if (numGensGlobal > 0) {

    // (Square) of the distance to a facet in an icosahedon.
    const Scalar rin2 = 0.25*kernelExtent*kernelExtent * (15.0*(5.0+sqrt(5.0)))/900.0;

    // Build an approximation of the starting kernel shape (in eta space) as an icosahedron.
    const unsigned nverts = 12;
    const unsigned nfaces = 20;
    r3d_int faces[nfaces][3] = {
      // 5 faces around point 0
      {0, 11, 5},
      {0, 5, 1},
      {0, 1, 7},
      {0, 7, 10},
      {0, 10, 11},
      // 5 adjacent faces
      {1, 5, 9},
      {5, 11, 4},
      {11, 10, 2},
      {10, 7, 6},
      {7, 1, 8},
      // 5 faces around point 3
      {3, 9, 4},
      {3, 4, 2},
      {3, 2, 6},
      {3, 6, 8},
      {3, 8, 9},
      // 5 adjacent faces
      {4, 9, 5},
      {2, 4, 11},
      {6, 2, 10},
      {8, 6, 7},
      {9, 8, 1},
    };
    r3d_int** facesp = new r3d_int*[nfaces];
    for (unsigned j = 0; j != nfaces; ++j) {
      facesp[j] = new r3d_int[3];
      for (unsigned k = 0; k != 3; ++k) facesp[j][k] = faces[j][k];
    }
    r3d_int nvertsperface[nfaces] = {  // Array of number of vertices per face.
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
    };
    const double t = (1.0 + sqrt(5.0)) / 2.0;
    r3d_rvec3 verts[nverts];           // Array of vertex coordinates.
    verts[0].x =  -1; verts[0].y =  t; verts[0].z =   0;
    verts[1].x =   1; verts[1].y =  t; verts[1].z =   0;
    verts[2].x =  -1; verts[2].y = -t; verts[2].z =   0;
    verts[3].x =   1; verts[3].y = -t; verts[3].z =   0;
    verts[4].x =   0; verts[4].y = -1; verts[4].z =   t;
    verts[5].x =   0; verts[5].y =  1; verts[5].z =   t;
    verts[6].x =   0; verts[6].y = -1; verts[6].z =  -t;
    verts[7].x =   0; verts[7].y =  1; verts[7].z =  -t;
    verts[8].x =   t; verts[8].y =  0; verts[8].z =  -1;
    verts[9].x =   t; verts[9].y =  0; verts[9].z =   1;
    verts[10].x = -t; verts[10].y = 0; verts[10].z = -1;
    verts[11].x = -t; verts[11].y = 0; verts[11].z =  1;
    r3d_poly initialCell;
    r3d_init_poly(&initialCell, verts, nverts, facesp, nvertsperface, nfaces);
    CHECK(r3d_is_good(&initialCell));

    // Deallocate that damn memory. I hate this syntax, but don't know enough C to know if there's a better way.
    for (unsigned j = 0; j != nfaces; ++j) delete[] facesp[j];
    delete[] facesp;

    // Scale the template icosahedron to have the initial volume of a sphere of radius kernelExtent.
    r3d_real voli[1], firstmom[4];
    r3d_reduce(&initialCell, voli, 0);
    CHECK(voli[0] > 0.0);
    const double volscale = Dim<3>::rootnu(4.0/3.0*M_PI/voli[0])*kernelExtent;
    r3d_scale(&initialCell, volscale);
    CHECK(r3d_is_good(&initialCell));
    BEGIN_CONTRACT_SCOPE
    {
      r3d_reduce(&initialCell, voli, 0);
      CHECK2(fuzzyEqual(voli[0], 4.0/3.0*M_PI*Dim<3>::pownu(kernelExtent), 1.0e-10), voli[0] << " " << 4.0/3.0*M_PI*Dim<3>::pownu(kernelExtent) << " " << volscale);
    }
    END_CONTRACT_SCOPE
    
    // Walk the points.
    for (unsigned nodeListi = 0; nodeListi != numNodeLists; ++nodeListi) {
      const unsigned n = vol[nodeListi]->numInternalElements();
      const Neighbor<Dim<3> >& neighbor = position[nodeListi]->nodeListPtr()->neighbor();
      for (unsigned i = 0; i != n; ++i) {

        const Vector& ri = position(nodeListi, i);
        const SymTensor& Hi = H(nodeListi, i);
        const Scalar rhoi = rho(nodeListi, i);
        Vector gradRhoi = gradRho(nodeListi, i);
        const Vector grhat = gradRhoi.unitVector();
        const Scalar Hdeti = Hi.Determinant();
        const SymTensor Hinv = Hi.Inverse();

        // Grab this points neighbors and build all the planes.
        // We simultaneously build a very conservative limiter for the density gradient.
        Scalar phi = 1.0;
        vector<r3d_plane> pairPlanes;
        const vector<vector<int> >& fullConnectivity = connectivityMap.connectivityForNode(nodeListi, i);
        for (unsigned nodeListj = 0; nodeListj != numNodeLists; ++nodeListj) {
          for (vector<int>::const_iterator jItr = fullConnectivity[nodeListj].begin();
               jItr != fullConnectivity[nodeListj].end();
               ++jItr) {
            const unsigned j = *jItr;
            const Vector& rj = position(nodeListj, j);
            const Scalar rhoj = rho(nodeListj, j);

            // Build the planes for our clipping half-spaces.
            const Vector rij = ri - rj;
            const Vector nhat = rij.unitVector();
            pairPlanes.push_back(r3d_plane());
            pairPlanes.back().n.x = nhat.x();
            pairPlanes.back().n.y = nhat.y();
            pairPlanes.back().n.z = nhat.z();
            pairPlanes.back().d = 0.5*rij.magnitude();

            // Check the density gradient limiter.
            const Scalar fdir = FastMath::pow4(rij.unitVector().dot(grhat));
            phi = min(phi, max(0.0, max(1.0 - fdir, rij.dot(gradRhoi)*safeInv(rhoi - rhoj))));
          }
        }

        // If provided boundaries, we implement them as additional neighbor clipping planes.
        if (haveBoundaries) {
          const vector<Facet>& facets = boundaries[nodeListi].facets();
          BOOST_FOREACH(const Facet& facet, facets) {
            const Vector p = facet.closestPoint(ri);
            Vector rij = ri - p;
            if (rij.magnitude2() < kernelExtent*kernelExtent) {
              Vector nhat;
              if (rij.magnitude2() < 1.0e-3*facet.area()) {
                rij.Zero();
                nhat = -facet.normal();
              } else {
                nhat = rij.unitVector();
              }
              pairPlanes.push_back(r3d_plane());
              pairPlanes.back().n.x = nhat.x();
              pairPlanes.back().n.y = nhat.y();
              pairPlanes.back().n.z = nhat.z();
              pairPlanes.back().d = rij.magnitude();
            }
          }

          // Same thing with holes.
          BOOST_FOREACH(const FacetedVolume& hole, holes[nodeListi]) {
            const vector<Facet>& facets = hole.facets();
            BOOST_FOREACH(const Facet& facet, facets) {
              const Vector p = facet.closestPoint(ri);
              Vector rij = ri - p;
              if (rij.magnitude2() < kernelExtent*kernelExtent) {
                Vector nhat;
                if (rij.magnitude2() < 1.0e-3*facet.area()) {
                  rij.Zero();
                  nhat = facet.normal();
                } else {
                  nhat = rij.unitVector();
                }
                pairPlanes.push_back(r3d_plane());
                pairPlanes.back().n.x = nhat.x();
                pairPlanes.back().n.y = nhat.y();
                pairPlanes.back().n.z = nhat.z();
                pairPlanes.back().d = rij.magnitude();
              }
            }
          }
        }

        // Sort the planes by distance -- let's us clip more efficiently.
        std::sort(pairPlanes.begin(), pairPlanes.end(), compareR3Dplanes);

        // Initialize our seed cell shape.
        r3d_poly celli = initialCell;
        BOOST_FOREACH(r3d_vertex& vert, celli.verts) {
          const Vector vi = Hinv*Vector(vert.pos.x, vert.pos.y, vert.pos.z);
          vert.pos.x = vi.x();
          vert.pos.y = vi.y();
          vert.pos.z = vi.z();
        }
        CHECK2(r3d_is_good(&celli), "Bad initial polyhedron!");

        // Clip the local cell.
        r3d_clip(&celli, &pairPlanes[0], pairPlanes.size());
        CHECK2(r3d_is_good(&celli), "Bad polyhedron!");

        // Check if the final polyhedron is entirely within our "interior" check radius.
        bool interior = true;
        {
          unsigned k = 0;
          while (interior and k != celli.nverts) {
            interior = (Hi*Vector(celli.verts[k].pos.x, celli.verts[k].pos.y, celli.verts[k].pos.z)).magnitude2() < rin2;
            ++k;
          }
        }

        if (interior) {
          if (returnSurface) surfacePoint(nodeListi, i) = 0;

          // Compute the centroidal motion and volume.
          r3d_reduce(&celli, firstmom, 1);
          CHECK(firstmom[0] > 0.0);
          vol(nodeListi, i) = firstmom[0];
          const Vector deltaCentroidi = Vector(firstmom[1], firstmom[2], firstmom[3])/firstmom[0];

          // Apply the gradient limiter;
          gradRhoi *= phi;

          // Is there a significant density gradient?
          if (gradRhoi.magnitude()*Dim<3>::rootnu(vol(nodeListi, i)) >= 1e-8*rhoi) {

            const Vector nhat1 = gradRhoi.unitVector();
            double x1, x2;
            findPolyhedronExtent(x1, x2, nhat1, celli);
            CHECK2(x1 <= 0.0 and x2 >= 0.0, nodeListi << " " << i << " " << ri << " " << x1 << " " << x2);
            const Scalar b = gradRhoi.magnitude();

            // This version uses the medial position.
            const Scalar thpt = sqrt(abs(rhoi*rhoi + rhoi*b*(x1 + x2) + 0.5*b*b*(x1*x1 + x2*x2)));
            const Scalar xm1 = -(rhoi + thpt)/b;
            const Scalar xm2 = (-rhoi + thpt)/b;
            if (xm1 >= x1 and xm1 <= x2) {
              deltaMedian(nodeListi, i) = xm1*nhat1 - deltaCentroidi.dot(nhat1)*nhat1 + deltaCentroidi;
            } else {
              deltaMedian(nodeListi, i) = xm2*nhat1 - deltaCentroidi.dot(nhat1)*nhat1 + deltaCentroidi;
            }

            // // This version simply tries rho^2 weighting.
            // deltaMedian(nodeListi, i) = ((0.5*rhoi*(x2*x2 - x1*x1) +
            //                               2.0/3.0*rhoi*b*(x2*x2*x2 - x1*x1*x1) +
            //                               0.25*b*b*(x2*x2*x2*x2 - x1*x1*x1*x1))/
            //                              (pow3(rhoi + b*x2) - pow3(rhoi + b*x1)/(3.0*b)))*nhat1 - deltaCentroidi.dot(nhat1)*nhat1 + deltaCentroidi;

          } else {

            // Otherwise just use the centroid.
            deltaMedian(nodeListi, i) = deltaCentroidi;

          }

          // OK, this is an interior point from the perspective that it was clipped within our critical
          // radius on all sides.  However, if we have a bounding polygon we may still want to call it a
          // surface if in fact there are still facets from that bounding polygon on this cell.
          if (haveBoundaries and returnSurface) {
            unsigned j = 0;
            while (interior and j != celli.nverts) {
              interior = not pointOnPolyhedron(ri + Vector(celli.verts[j].pos.x, celli.verts[j].pos.y, celli.verts[j].pos.z),
                                               boundaries[nodeListi],
                                               1.0e-8);
              ++j;
            }

            if (not interior) {
              // This is a point that touches the bounding polygon.  Flag it as surface.
              if (returnSurface) surfacePoint(nodeListi, i) = 1;
            }
          }

        } else {

          // This point touches a free boundary, so flag it.
          if (returnSurface) surfacePoint(nodeListi, i) = 1;
          deltaMedian(nodeListi, i) = Vector::zero;

        }

        // Check if the candidate motion is still in the boundary.  If not, project back.
        if (haveBoundaries and not boundaries[nodeListi].contains(ri + deltaMedian(nodeListi, i))) {
          deltaMedian(nodeListi, i) = boundaries[nodeListi].closestPoint(ri + deltaMedian(nodeListi, i)) - ri;
          // cerr << "Correcting " << nodeListi << " " << i << " to new position " << (ri + deltaMedian(nodeListi, i)) << endl;
        }

      }
    }

  }
}

}
}
