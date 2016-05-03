//---------------------------------Spheral++----------------------------------//
// SPHHydroBaseRZ -- The SPH/ASPH hydrodynamic package for Spheral++,
//                   specialized for 2D RZ (cylindrical) geometry.
//
// Based on the methodology described in the dissertation
// de Catalunya Departament de Física i Enginyeria Nuclear, U. P., García Senz, D., (2012).
// AxisSPH:devising and validating an axisymmetric smoothed particle hydrodynamics code.
// TDX (Tesis Doctorals en Xarxa). Universitat Politècnica de Catalunya.
//
// Note this version is currently abusing our ordinary 2D geometric types,
// implicitly mapping x->z, y->r.
//
// Created by JMO, Tue Apr 26 16:06:21 PDT 2016
//----------------------------------------------------------------------------//
#ifndef __Spheral_SPHHydroBaseRZ_hh__
#define __Spheral_SPHHydroBaseRZ_hh__

#include <string>

#include "SPHHydroBase.hh"
#include "Geometry/Dimension.hh"

namespace Spheral {
namespace SPHSpace {

class SPHHydroBaseRZ: public SPHHydroBase<Dim<2> > {

public:
  //--------------------------- Public Interface ---------------------------//
  typedef Dim<2> Dimension;
  typedef Dimension::Scalar Scalar;
  typedef Dimension::Vector Vector;
  typedef Dimension::Tensor Tensor;
  typedef Dimension::SymTensor SymTensor;

  typedef PhysicsSpace::Physics<Dimension>::ConstBoundaryIterator ConstBoundaryIterator;

  // Constructors.
  SPHHydroBaseRZ(const NodeSpace::SmoothingScaleBase<Dimension>& smoothingScaleMethod,
                 ArtificialViscositySpace::ArtificialViscosity<Dimension>& Q,
                 const KernelSpace::TableKernel<Dimension>& W,
                 const KernelSpace::TableKernel<Dimension>& WPi,
                 const double filter,
                 const double cfl,
                 const bool useVelocityMagnitudeForDt,
                 const bool compatibleEnergyEvolution,
                 const bool evolveTotalEnergy,
                 const bool gradhCorrection,
                 const bool XSPH,
                 const bool correctVelocityGradient,
                 const bool sumMassDensityOverAllNodeLists,
                 const PhysicsSpace::MassDensityType densityUpdate,
                 const PhysicsSpace::HEvolutionType HUpdate,
                 const double epsTensile,
                 const double nTensile,
                 const Vector& xmin,
                 const Vector& xmax);

  // Destructor.
  virtual ~SPHHydroBaseRZ();

  // Evaluate the derivatives for the principle hydro variables:
  // mass density, velocity, and specific thermal energy.
  virtual
  void evaluateDerivatives(const Scalar time,
                           const Scalar dt,
                           const DataBaseSpace::DataBase<Dimension>& dataBase,
                           const State<Dimension>& state,
                           StateDerivatives<Dimension>& derivatives) const;

  // Finalize the hydro at the completion of an integration step.
  virtual
  void finalize(const Scalar time,
                const Scalar dt,
                DataBaseSpace::DataBase<Dimension>& dataBase,
                State<Dimension>& state,
                StateDerivatives<Dimension>& derivs);

  // Apply boundary conditions to the physics specific fields.
  virtual
  void applyGhostBoundaries(State<Dimension>& state,
                            StateDerivatives<Dimension>& derivs);

  // Enforce boundary conditions for the physics specific fields.
  virtual
  void enforceBoundaries(State<Dimension>& state,
                         StateDerivatives<Dimension>& derivs);
               
  //****************************************************************************
  // Methods required for restarting.
  virtual std::string label() const { return "SPHHydroBaseRZ"; }
  //****************************************************************************

private:
  //--------------------------- Private Interface ---------------------------//
  // No default constructor, copying, or assignment.
  SPHHydroBaseRZ();
  SPHHydroBaseRZ(const SPHHydroBaseRZ&);
  SPHHydroBaseRZ& operator=(const SPHHydroBaseRZ&);
};

}
}

#else

// Forward declaration.
namespace Spheral {
  namespace SPHSpace {
    class SPHHydroBaseRZ;
  }
}

#endif
