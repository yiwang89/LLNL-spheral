//------------------------------------------------------------------------------
// Explicit instantiation.
//------------------------------------------------------------------------------
#include "computeSPHOmegaGradhCorrection.cc"
#include "Geometry/Dimension.hh"

namespace Spheral {
  namespace SPHSpace {
    template void computeSPHOmegaGradhCorrection(const ConnectivityMap<Dim<1> >&, 
                                                 const TableKernel<Dim<1> >&, 
                                                 const FieldList<Dim<1>, Dim<1>::Vector>&,
                                                 const FieldList<Dim<1>, Dim<1>::SymTensor>&,
                                                 FieldList<Dim<1>, Dim<1>::Scalar>&);
    template void computeSPHOmegaGradhCorrection(const ConnectivityMap<Dim<2> >&, 
                                                 const TableKernel<Dim<2> >&, 
                                                 const FieldList<Dim<2>, Dim<2>::Vector>&,
                                                 const FieldList<Dim<2>, Dim<2>::SymTensor>&,
                                                 FieldList<Dim<2>, Dim<2>::Scalar>&);
    template void computeSPHOmegaGradhCorrection(const ConnectivityMap<Dim<3> >&, 
                                                 const TableKernel<Dim<3> >&, 
                                                 const FieldList<Dim<3>, Dim<3>::Vector>&,
                                                 const FieldList<Dim<3>, Dim<3>::SymTensor>&,
                                                 FieldList<Dim<3>, Dim<3>::Scalar>&);
  }
}

