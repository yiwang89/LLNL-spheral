//---------------------------------Spheral++------------------------------------
// Detect surface particles leveraging the zeroth and first moments
//------------------------------------------------------------------------------
#include <stdio.h>

#include "detectSurface.hh"
#include "Field/Field.hh"
#include "Field/FieldList.hh"
#include "Neighbor/ConnectivityMap.hh"
#include "Kernel/TableKernel.hh"
#include "NodeList/NodeList.hh"
#include "Hydro/HydroFieldNames.hh"
#include "Geometry/outerProduct.hh"
#include "Geometry/innerProduct.hh"
#include "Geometry/innerDoubleProduct.hh"
#include "Geometry/invertRankNTensor.hh"

namespace Spheral {
namespace CRKSPHSpace {
using namespace std;
using std::min;
using std::max;
using std::abs;
    
using FieldSpace::Field;
using FieldSpace::FieldList;
using NeighborSpace::ConnectivityMap;
using KernelSpace::TableKernel;
using NodeSpace::NodeList;
using Geometry::outerProduct;
using Geometry::innerProduct;
using Geometry::innerDoubleProduct;
    
template<typename Dimension>
void
detectSurface(const NeighborSpace::ConnectivityMap<Dimension>& connectivityMap,
              const FieldSpace::FieldList<Dimension, typename Dimension::Scalar>& m0,
              const FieldSpace::FieldList<Dimension, typename Dimension::Vector>& m1,
              const FieldSpace::FieldList<Dimension, typename Dimension::Vector>& position,
              const FieldSpace::FieldList<Dimension, typename Dimension::SymTensor>& H,
              const double detectThreshold,
              const double detectRange,
              const double sweepAngle,
              FieldSpace::FieldList<Dimension, int>& surfacePoint) {

  // Pre-conditions.
  const size_t numNodeLists = m0.size();
  REQUIRE(m1.size() == numNodeLists);
  REQUIRE(position.size() == numNodeLists);
  REQUIRE(H.size() == numNodeLists);
  REQUIRE(surfacePoint.size() == numNodeLists);
        
  typedef typename Dimension::Scalar Scalar;
  typedef typename Dimension::Vector Vector;
  typedef typename Dimension::Tensor Tensor;
  typedef typename Dimension::SymTensor SymTensor;
        
  const Scalar nPerh = m0.nodeListPtrs()[0]->nodesPerSmoothingScale();
  const Scalar cosSweepAngle = cos(sweepAngle);
        
  // Walk the FluidNodeLists.
  for (size_t nodeListi = 0; nodeListi != numNodeLists; ++nodeListi) {
    const NodeList<Dimension>& nodeList = m0[nodeListi]->nodeList();
            
    // Iterate over nodes in nodeListi
    for (typename ConnectivityMap<Dimension>::const_iterator iItr = connectivityMap.begin(nodeListi);
         iItr != connectivityMap.end(nodeListi);
         ++iItr) {
      const int i = *iItr;
                
      // Get the state for node i.
      const Vector& ri    = position(nodeListi, i);
      const SymTensor& Hi = H(nodeListi, i);
      const Scalar m0i    = m0(nodeListi, i);
      const Vector& m1i   = m1(nodeListi, i);
      const Vector m1ih   = m1i.unitVector();
      bool particleDetected = false;
      // Zero out surfacePoint to get fresh maps at each step
      surfacePoint(nodeListi, i) = 0;
                
      if (std::abs(m0i - 1.0) > detectThreshold) {
        // Get neighbors
        const vector<vector<int> >& fullConnectivity = connectivityMap.connectivityForNode(nodeListi, i);
        CHECK(fullConnectivity.size() == numNodeLists);
        // Loop over them
        size_t nodeListj = 0;
        while (!particleDetected and nodeListj != numNodeLists) {
          const vector<int>& connectivity = fullConnectivity[nodeListj];
          for (vector<int>::const_iterator jItr = connectivity.begin();
               jItr != connectivity.end();
               ++jItr) {
            const int j = *jItr;

            // Get the state for node j.
            const Vector& rj    = position(nodeListj, j);
            const SymTensor& Hj = H(nodeListj, j);
            const Vector rji    = rj - ri;  // pointing away from i as does m1
            const Vector rjih   = rji.unitVector();
            const Scalar etamin = sqrt(std::min((Hi*rji).magnitude2(), (Hj*rji).magnitude2()));
                            
            // Check angle
            const double cangle = m1ih.dot(rjih);  // because they're both unit vectors!
            if ((cangle >= cosSweepAngle) && (etamin < detectRange)) {
              particleDetected = true;
              break;
            }
          }
          ++nodeListj;
        }
        if (!particleDetected) surfacePoint(nodeListi, i) = 1;
      }
                
    }
  }
}

}
}
