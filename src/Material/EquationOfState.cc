//---------------------------------Spheral++----------------------------------//
// EquationOfState -- Abstract base class for the equation of state classes.
//
// Created by JMO, Mon Dec  6 21:36:45 PST 1999
//----------------------------------------------------------------------------//

#include "EquationOfState.hh"
#include "Geometry/Dimension.hh"

namespace Spheral {
namespace Material {

//------------------------------------------------------------------------------
// Default constructor.
//------------------------------------------------------------------------------
template<typename Dimension>
EquationOfState<Dimension>::EquationOfState(const double minimumPressure,
                                            const double maximumPressure):
  mMinimumPressure(minimumPressure),
  mMaximumPressure(maximumPressure) {
  REQUIRE(mMinimumPressure <= mMaximumPressure);
}

//------------------------------------------------------------------------------
// Destructor.
//------------------------------------------------------------------------------
template<typename Dimension>
EquationOfState<Dimension>::~EquationOfState() {
}

//------------------------------------------------------------------------------
// Explicit instantiation.
//------------------------------------------------------------------------------
template class EquationOfState< Dim<1> >;
template class EquationOfState< Dim<2> >;
template class EquationOfState< Dim<3> >;

}
}
