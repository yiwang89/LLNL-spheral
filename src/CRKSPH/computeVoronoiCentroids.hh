//---------------------------------Spheral++------------------------------------
// Compute centroids for each point using the Vornonoi tessellation.
//------------------------------------------------------------------------------
#ifndef __Spheral__computeVoronoiCentroids__
#define __Spheral__computeVoronoiCentroids__

#include "Geometry/Dimension.hh"

namespace Spheral {

  // Forward declarations.
  namespace NeighborSpace {
    template<typename Dimension> class ConnectivityMap;
  }
  namespace KernelSpace {
    template<typename Dimension> class TableKernel;
  }
  namespace FieldSpace {
    template<typename Dimension, typename DataType> class FieldList;
  }

  FieldSpace::FieldList<Dim<1>, Dim<1>::Vector>
  computeVoronoiCentroids(const FieldSpace::FieldList<Dim<1>, Dim<1>::Vector>& position);

  FieldSpace::FieldList<Dim<2>, Dim<2>::Vector>
  computeVoronoiCentroids(const FieldSpace::FieldList<Dim<2>, Dim<2>::Vector>& position);

  FieldSpace::FieldList<Dim<3>, Dim<3>::Vector>
  computeVoronoiCentroids(const FieldSpace::FieldList<Dim<3>, Dim<3>::Vector>& position);

}

#endif