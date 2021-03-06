text = """
//---------------------------------Spheral++----------------------------------//
// GaussianKernel -- The gaussian interpolation kernel.
//
// Created by JMO, Wed Dec  1 14:38:51 PST 1999
//----------------------------------------------------------------------------//
#include <math.h>

#include "Kernel.hh"
#include "GaussianKernel.hh"

//------------------------------------------------------------------------------
// Explicit instantiation.
//------------------------------------------------------------------------------
namespace Spheral {
  namespace KernelSpace {
    template class GaussianKernel< Dim< %(ndim)s >  >;
  }
}
"""
