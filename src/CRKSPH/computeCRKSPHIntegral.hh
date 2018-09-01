//---------------------------------Spheral++------------------------------------
// Compute the CRKSPH corrections.
//------------------------------------------------------------------------------
#ifndef __Spheral__computeCRKSPHIntegral__
#define __Spheral__computeCRKSPHIntegral__

namespace Spheral {

// Forward declarations.
template<typename Dimension> class ConnectivityMap;
template<typename Dimension> class TableKernel;
template<typename Dimension, typename DataType> class FieldList;

template<typename Dimension>
std::pair<typename Dimension::Vector,typename Dimension::Vector>
computeCRKSPHIntegral(const NeighborSpace::ConnectivityMap<Dimension>& connectivityMap,
                       const KernelSpace::TableKernel<Dimension>& W,
                       const FieldSpace::FieldList<Dimension, typename Dimension::Scalar>& weight,
                       const FieldSpace::FieldList<Dimension, typename Dimension::Vector>& position,
                       const FieldSpace::FieldList<Dimension, typename Dimension::SymTensor>& H,
                       size_t nodeListi, const int i, size_t nodeListj, const int j, int mydim, const int order,
                       typename Dimension::Vector rmin, typename Dimension::Vector rmax);

}

#endif
