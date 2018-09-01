//---------------------------------Spheral++------------------------------------
// Compute the CRKSPH corrections.
//------------------------------------------------------------------------------
#ifndef __Spheral__computeCRKSPHEvaluation__
#define __Spheral__computeCRKSPHEvaluation__

namespace Spheral {

// Forward declarations.
template<typename Dimension> class ConnectivityMap;
template<typename Dimension> class TableKernel;
template<typename Dimension, typename DataType> class FieldList;

template<typename Dimension>
void
computeCRKSPHEvaluation(const NeighborSpace::ConnectivityMap<Dimension>& connectivityMap,
                       const KernelSpace::TableKernel<Dimension>& W,
                       const FieldSpace::FieldList<Dimension, typename Dimension::Scalar>& weight,
                       const FieldSpace::FieldList<Dimension, typename Dimension::Vector>& position,
                       const FieldSpace::FieldList<Dimension, typename Dimension::SymTensor>& H,
                       size_t nodeListi, const int i, typename Dimension::Vector reval,
                       const bool coupleNodeLists, typename Dimension::Scalar& WCRKSPH, typename Dimension::Vector& gradWCRKSPH);

}

#endif
