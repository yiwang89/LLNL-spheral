//---------------------------------Spheral++----------------------------------//
// CRKSPHHydroBase -- The CRKSPH/ACRKSPH hydrodynamic package for Spheral++.
//
// Created by JMO, Mon Jul 19 21:52:29 PDT 2010
//----------------------------------------------------------------------------//
#ifndef __Spheral_CRKSPHHydroBase_hh__
#define __Spheral_CRKSPHHydroBase_hh__

#include <string>

#include "Physics/GenericHydro.hh"
#include "CRKSPHCorrectionParams.hh"
#include "Boundary/CRKSPHVoidBoundary.hh"

namespace Spheral {
  template<typename Dimension> class State;
  template<typename Dimension> class StateDerivatives;
  namespace NodeSpace {
    template<typename Dimension> class SmoothingScaleBase;
  }
  namespace ArtificialViscositySpace {
    template<typename Dimension> class ArtificialViscosity;
  }
  namespace KernelSpace {
    template<typename Dimension> class TableKernel;
  }
  namespace DataBaseSpace {
    template<typename Dimension> class DataBase;
  }
  namespace FieldSpace {
    template<typename Dimension, typename DataType> class Field;
    template<typename Dimension, typename DataType> class FieldList;
  }
  namespace FileIOSpace {
    class FileIO;
  }
}

namespace Spheral {
namespace CRKSPHSpace {

template<typename Dimension>
class CRKSPHHydroBase: public PhysicsSpace::GenericHydro<Dimension> {

public:
  //--------------------------- Public Interface ---------------------------//
  typedef typename Dimension::Scalar Scalar;
  typedef typename Dimension::Vector Vector;
  typedef typename Dimension::Tensor Tensor;
  typedef typename Dimension::ThirdRankTensor ThirdRankTensor;
  typedef typename Dimension::FourthRankTensor FourthRankTensor;
  typedef typename Dimension::FifthRankTensor FifthRankTensor;
  typedef typename Dimension::SymTensor SymTensor;
  typedef typename Dimension::FacetedVolume FacetedVolume;

  typedef typename PhysicsSpace::Physics<Dimension>::ConstBoundaryIterator ConstBoundaryIterator;

  // Constructors.
  CRKSPHHydroBase(const NodeSpace::SmoothingScaleBase<Dimension>& smoothingScaleMethod,
                  ArtificialViscositySpace::ArtificialViscosity<Dimension>& Q,
                  const KernelSpace::TableKernel<Dimension>& W,
                  const KernelSpace::TableKernel<Dimension>& WPi,
                  const double filter,
                  const double cfl,
                  const bool useVelocityMagnitudeForDt,
                  const bool compatibleEnergyEvolution,
                  const bool evolveTotalEnergy,
                  const bool XSPH,
                  const PhysicsSpace::MassDensityType densityUpdate,
                  const PhysicsSpace::HEvolutionType HUpdate,
                  const CRKSPHSpace::CRKOrder correctionOrder,
                  const CRKSPHSpace::CRKVolumeType volumeType,
                  const bool detectSurfaces,
                  const double detectThreshold,
                  const double sweepAngle,
                  const double detectRange,
                  const double epsTensile,
                  const double nTensile);

  // Destructor.
  virtual ~CRKSPHHydroBase();

  // Tasks we do once on problem startup.
  virtual
  void initializeProblemStartup(DataBaseSpace::DataBase<Dimension>& dataBase) override;

  // Register the state Hydro expects to use and evolve.
  virtual 
  void registerState(DataBaseSpace::DataBase<Dimension>& dataBase,
                     State<Dimension>& state) override;

  // Register the derivatives/change fields for updating state.
  virtual
  void registerDerivatives(DataBaseSpace::DataBase<Dimension>& dataBase,
                           StateDerivatives<Dimension>& derivs) override;

  // Initialize the Hydro before we start a derivative evaluation.
  virtual
  void initialize(const Scalar time,
                  const Scalar dt,
                  const DataBaseSpace::DataBase<Dimension>& dataBase,
                  State<Dimension>& state,
                  StateDerivatives<Dimension>& derivs) override;
                          
  // Evaluate the derivatives for the principle hydro variables:
  // mass density, velocity, and specific thermal energy.
  virtual
  void evaluateDerivatives(const Scalar time,
                           const Scalar dt,
                           const DataBaseSpace::DataBase<Dimension>& dataBase,
                           const State<Dimension>& state,
                           StateDerivatives<Dimension>& derivatives) const override;

  // Finalize the derivatives.
  virtual
  void finalizeDerivatives(const Scalar time,
                           const Scalar dt,
                           const DataBaseSpace::DataBase<Dimension>& dataBase,
                           const State<Dimension>& state,
                           StateDerivatives<Dimension>& derivs) const override;

  // Hook called after the state has been updated and boundary conditions have been enforced.
  virtual 
  void postStateUpdate(const DataBaseSpace::DataBase<Dimension>& dataBase, 
                       State<Dimension>& state,
                       const StateDerivatives<Dimension>& derivatives) const override;

  // Finalize the hydro at the completion of an integration step.
  virtual
  void finalize(const Scalar time,
                const Scalar dt,
                DataBaseSpace::DataBase<Dimension>& dataBase,
                State<Dimension>& state,
                StateDerivatives<Dimension>& derivs) override;
                  
  // Apply boundary conditions to the physics specific fields.
  virtual
  void applyGhostBoundaries(State<Dimension>& state,
                            StateDerivatives<Dimension>& derivs) override;

  // Enforce boundary conditions for the physics specific fields.
  virtual
  void enforceBoundaries(State<Dimension>& state,
                         StateDerivatives<Dimension>& derivs) override;

  // // We need ghost connectivity to be computed.
  // virtual bool requireGhostConnectivity() const override { return true; }

  // Flag to choose whether we want to sum for density, or integrate
  // the continuity equation.
  PhysicsSpace::MassDensityType densityUpdate() const;
  void densityUpdate(const PhysicsSpace::MassDensityType type);

  // Flag to select how we want to evolve the H tensor.
  // the continuity equation.
  PhysicsSpace::HEvolutionType HEvolution() const;
  void HEvolution(const PhysicsSpace::HEvolutionType type);

  // Flag to choose CRK Correction Order
  CRKSPHSpace::CRKOrder correctionOrder() const;
  void correctionOrder(const CRKSPHSpace::CRKOrder order);

  // Flag for the CRK volume weighting definition
  CRKSPHSpace::CRKVolumeType volumeType() const;
  void volumeType(const CRKSPHSpace::CRKVolumeType x);

  // Flag to determine if we're using the total energy conserving compatible energy
  // evolution scheme.
  bool compatibleEnergyEvolution() const;
  void compatibleEnergyEvolution(const bool val);

  // Flag controlling if we evolve total or specific energy.
  bool evolveTotalEnergy() const;
  void evolveTotalEnergy(const bool val);

  // Flag to determine if we're using the grad h correction.
  bool gradhCorrection() const;
  void gradhCorrection(const bool val);

  // Flag to determine if we're using the XSPH algorithm.
  bool XSPH() const;
  void XSPH(const bool val);

  // The object defining how we evolve smoothing scales.
  const NodeSpace::SmoothingScaleBase<Dimension>& smoothingScaleMethod() const;

  // Fraction of centroidal filtering to apply.
  double filter() const;
  void filter(const double val);

  // Parameters for the tensile correction force at small scales.
  Scalar epsilonTensile() const;
  void epsilonTensile(const Scalar val);

  Scalar nTensile() const;
  void nTensile(const Scalar val);
    
  // Surface detection getters and setters
  bool detectSurfaces() const;
  void detectSurfaces(const bool val);
    
  double detectThreshold() const;
  void detectThreshold(const double val);
    
  double detectRange() const;
  void detectRange(const double val);
    
  double sweepAngle() const;
  void sweepAngle(const double val);

  // Limits to impose on node by node corrections.
  double correctionMin() const;
  void correctionMin(const double val);

  double correctionMax() const;
  void correctionMax(const double val);

  // We maintain a special boundary condition to handle void points.
  const BoundarySpace::CRKSPHVoidBoundary<Dimension>& voidBoundary() const;

  // The state field lists we're maintaining.
  const FieldSpace::FieldList<Dimension, int>&       timeStepMask() const;
  const FieldSpace::FieldList<Dimension, Scalar>&    pressure() const;
  const FieldSpace::FieldList<Dimension, Scalar>&    soundSpeed() const;
  const FieldSpace::FieldList<Dimension, Scalar>&    specificThermalEnergy0() const;
  const FieldSpace::FieldList<Dimension, Scalar>&    entropy() const;
  const FieldSpace::FieldList<Dimension, SymTensor>& Hideal() const;
  const FieldSpace::FieldList<Dimension, Scalar>&    maxViscousPressure() const;
  const FieldSpace::FieldList<Dimension, Scalar>&    effectiveViscousPressure() const;
  const FieldSpace::FieldList<Dimension, Scalar>&    viscousWork() const;
  const FieldSpace::FieldList<Dimension, Scalar>&    weightedNeighborSum() const;
  const FieldSpace::FieldList<Dimension, SymTensor>& massSecondMoment() const;
  const FieldSpace::FieldList<Dimension, Scalar>&    volume() const;
  const FieldSpace::FieldList<Dimension, Scalar>&    volume0() const;
  const FieldSpace::FieldList<Dimension, Vector>&    massDensityGradient() const;
  const FieldSpace::FieldList<Dimension, Vector>&    XSPHDeltaV() const;
  const FieldSpace::FieldList<Dimension, Vector>&    DxDt() const;

  const FieldSpace::FieldList<Dimension, Vector>&    DvDt() const;
  const FieldSpace::FieldList<Dimension, Scalar>&    DmassDensityDt() const;
  const FieldSpace::FieldList<Dimension, Scalar>&    DspecificThermalEnergyDt() const;
  const FieldSpace::FieldList<Dimension, SymTensor>& DHDt() const;
  const FieldSpace::FieldList<Dimension, Tensor>&    DvDx() const;
  const FieldSpace::FieldList<Dimension, Tensor>&    internalDvDx() const;
  const FieldSpace::FieldList<Dimension, std::vector<Vector> >& pairAccelerations() const;
  const FieldSpace::FieldList<Dimension, Vector>&    deltaCentroid() const;

  const FieldSpace::FieldList<Dimension, Scalar>&    A() const;
  const FieldSpace::FieldList<Dimension, Vector>&    B() const;
  const FieldSpace::FieldList<Dimension, Tensor>&    C() const;
  const FieldSpace::FieldList<Dimension, Vector>&    gradA() const;
  const FieldSpace::FieldList<Dimension, Tensor>&    gradB() const;
  const FieldSpace::FieldList<Dimension, ThirdRankTensor>&    gradC() const;
    
  const FieldList<Dimension, Scalar>&                m0() const;
  const FieldList<Dimension, Vector>&                m1() const;
  const FieldList<Dimension, SymTensor>&             m2() const;
  const FieldList<Dimension, ThirdRankTensor>&       m3() const;
  const FieldList<Dimension, FourthRankTensor>&      m4() const;
  const FieldList<Dimension, Vector>&                gradm0() const;
  const FieldList<Dimension, Tensor>&                gradm1() const;
  const FieldList<Dimension, ThirdRankTensor> &      gradm2() const;
  const FieldList<Dimension, FourthRankTensor>&      gradm3() const;
  const FieldList<Dimension, FifthRankTensor>&       gradm4() const;

  const FieldSpace::FieldList<Dimension, int>&       surfacePoint() const;
  const FieldSpace::FieldList<Dimension, int>&       voidPoint() const;
  const FieldSpace::FieldList<Dimension, std::vector<Vector>>& etaVoidPoints() const;

  //****************************************************************************
  // Methods required for restarting.
  virtual std::string label() const { return "CRKSPHHydroBase"; }
  virtual void dumpState(FileIOSpace::FileIO& file, std::string pathName) const;
  virtual void restoreState(const FileIOSpace::FileIO& file, std::string pathName);
  //****************************************************************************

protected:
  //--------------------------- Protected Interface ---------------------------//
  // The method defining how we evolve smoothing scales.
  const NodeSpace::SmoothingScaleBase<Dimension>& mSmoothingScaleMethod;

  // A bunch of switches.
  PhysicsSpace::MassDensityType mDensityUpdate;
  PhysicsSpace::HEvolutionType mHEvolution;
  CRKSPHSpace::CRKOrder mCorrectionOrder;
  CRKSPHSpace::CRKVolumeType mVolumeType;
  bool mCompatibleEnergyEvolution, mEvolveTotalEnergy, mGradhCorrection, mXSPH;
  double mfilter;
  Scalar mEpsTensile, mnTensile;
  bool mDetectSurfaces;
  double mDetectThreshold, mSweepAngle, mDetectRange;
  double mCorrectionMin, mCorrectionMax;

  // Some internal scratch fields.
  FieldSpace::FieldList<Dimension, int>       mTimeStepMask;
  FieldSpace::FieldList<Dimension, Scalar>    mPressure;
  FieldSpace::FieldList<Dimension, Scalar>    mSoundSpeed;
  FieldSpace::FieldList<Dimension, Scalar>    mSpecificThermalEnergy0;
  FieldSpace::FieldList<Dimension, Scalar>    mEntropy;

  FieldSpace::FieldList<Dimension, SymTensor> mHideal;
  FieldSpace::FieldList<Dimension, Scalar>    mMaxViscousPressure;
  FieldSpace::FieldList<Dimension, Scalar>    mEffViscousPressure;
  FieldSpace::FieldList<Dimension, Scalar>    mViscousWork;

  FieldSpace::FieldList<Dimension, Scalar>    mWeightedNeighborSum;
  FieldSpace::FieldList<Dimension, SymTensor> mMassSecondMoment;

  FieldSpace::FieldList<Dimension, Scalar>    mVolume;
  FieldSpace::FieldList<Dimension, Scalar>    mVolume0;
  FieldSpace::FieldList<Dimension, Vector>    mMassDensityGradient;

  FieldSpace::FieldList<Dimension, Vector>    mXSPHDeltaV;
  FieldSpace::FieldList<Dimension, Vector>    mDxDt;

  FieldSpace::FieldList<Dimension, Vector>    mDvDt;
  FieldSpace::FieldList<Dimension, Scalar>    mDmassDensityDt;
  FieldSpace::FieldList<Dimension, Scalar>    mDspecificThermalEnergyDt;
  FieldSpace::FieldList<Dimension, SymTensor> mDHDt;
  FieldSpace::FieldList<Dimension, Tensor>    mDvDx;
  FieldSpace::FieldList<Dimension, Tensor>    mInternalDvDx;
  FieldSpace::FieldList<Dimension, Vector>    mDeltaCentroid;

  FieldSpace::FieldList<Dimension, std::vector<Vector> > mPairAccelerations;

  FieldSpace::FieldList<Dimension, Scalar>    mA;
  FieldSpace::FieldList<Dimension, Vector>    mB;
  FieldSpace::FieldList<Dimension, Tensor>    mC;
  FieldSpace::FieldList<Dimension, Vector>    mGradA;
  FieldSpace::FieldList<Dimension, Tensor>    mGradB;
  FieldSpace::FieldList<Dimension, ThirdRankTensor>    mGradC;
    
  FieldList<Dimension, Scalar>                mM0;
  FieldList<Dimension, Vector>                mM1;
  FieldList<Dimension, SymTensor>             mM2;
  FieldList<Dimension, ThirdRankTensor>       mM3;
  FieldList<Dimension, FourthRankTensor>      mM4;
  FieldList<Dimension, Vector>                mGradm0;
  FieldList<Dimension, Tensor>                mGradm1;
  FieldList<Dimension, ThirdRankTensor>       mGradm2;
  FieldList<Dimension, FourthRankTensor>      mGradm3;
  FieldList<Dimension, FifthRankTensor>       mGradm4;

  FieldSpace::FieldList<Dimension, int>       mSurfacePoint;
  FieldSpace::FieldList<Dimension, int>       mVoidPoint;
  FieldSpace::FieldList<Dimension, std::vector<Vector>> mEtaVoidPoints;

  BoundarySpace::CRKSPHVoidBoundary<Dimension> mVoidBoundary;

private:
  //--------------------------- Private Interface ---------------------------//
  // The restart registration.
  DataOutput::RestartRegistrationType mRestart;

  // No default constructor, copying, or assignment.
  CRKSPHHydroBase();
  CRKSPHHydroBase(const CRKSPHHydroBase&);
  CRKSPHHydroBase& operator=(const CRKSPHHydroBase&);
};

}
}

#ifndef __GCCXML__
#include "CRKSPHHydroBaseInline.hh"
#endif

#else

// Forward declaration.
namespace Spheral {
  namespace CRKSPHSpace {
    template<typename Dimension> class CRKSPHHydroBase;
  }
}

#endif
