//---------------------------------Spheral++----------------------------------//
// Cullen adn Dehnen Viscosity 
//----------------------------------------------------------------------------//
#include "CullenDehnenViscosity.hh"
#include "DataOutput/Restart.hh"
#include "Field/FieldList.hh"
#include "DataBase/DataBase.hh"
#include "DataBase/State.hh"
#include "DataBase/StateDerivatives.hh"
#include "DataBase/IncrementFieldList.hh"
#include "NodeList/FluidNodeList.hh"
#include "Neighbor/Neighbor.hh"
#include "Material/EquationOfState.hh"
#include "Boundary/Boundary.hh"
#include "Hydro/HydroFieldNames.hh"
#include "DataBase/IncrementState.hh"
#include "DataBase/IncrementBoundedFieldList.hh"
#include "FileIO/FileIO.hh"

namespace Spheral {
namespace ArtificialViscositySpace {
    
using namespace std;
using std::min;
using std::max;
using std::abs;
using FileIOSpace::FileIO;

using PhysicsSpace::Physics;
using DataOutput::Restart;
using FieldSpace::Field;
using FieldSpace::FieldList;
using DataBaseSpace::DataBase;
using NodeSpace::NodeList;
using NodeSpace::FluidNodeList;
using NodeSpace::SmoothingScaleBase;

using KernelSpace::TableKernel;
using NeighborSpace::ConnectivityMap;



//------------------------------------------------------------------------------
// Construct with the given value for the linear and quadratic coefficients.
//------------------------------------------------------------------------------
template<typename Dimension>
CullenDehnenViscosity<Dimension>::
    CullenDehnenViscosity(ArtificialViscosity<Dimension>& q,
                                    const TableKernel<Dimension>& W,
                                    const Scalar alphMax, //Parameter = 2.0 in Hopkins 2014 and Price 2004, = 1.5 in Rosswog 2000
                                    const Scalar alphMin, //Parameter = 0.02 in Hopkins 2014  
                                    const Scalar betaC, //Parameter = 0.7 Hopkins 2014
                                    const Scalar betaD, //Parameter = 0.05 Hopkins 2014
                                    const Scalar betaE, //Parameter = 1.0 in Hopkins 2014, = 2.0 in Cullen 2010
				    const Scalar fKern, //Parameter = 1/3 Hopkins 2014 for quinitc spline
                                    const bool boolHopkins): //use Hopkins Reformulation
    Physics<Dimension>(),
    mPrevDvDt(FieldSpace::Copy),
    mPrevDivV(FieldSpace::Copy),
    mCullAlpha(FieldSpace::Copy),
    mPrevDivV2(FieldSpace::Copy),
    mCullAlpha2(FieldSpace::Copy),
    malphMax(alphMax),
    malphMin(alphMin),
    mbetaC(betaC),
    mbetaD(betaD),
    mbetaE(betaE),
    mfKern(fKern),
    mboolHopkins(boolHopkins),
    myq(q),
    mKernel(W),
    mRestart(DataOutput::registerWithRestart(*this)){
}

//------------------------------------------------------------------------------
// Destructor.
//------------------------------------------------------------------------------
template<typename Dimension>
CullenDehnenViscosity<Dimension>::
~CullenDehnenViscosity() {
}

    
// Accessor Fns
    template<typename Dimension>
    void
    CullenDehnenViscosity<Dimension>::
    alphMax(Scalar val)
    {
        malphMax = val;
    }

    template<typename Dimension>
    void
    CullenDehnenViscosity<Dimension>::
    alphMin(Scalar val)
    {
        malphMin = val;
    }

    template<typename Dimension>
    void
    CullenDehnenViscosity<Dimension>::
    betaE(Scalar val)
    {
        mbetaE = val;
    }

    template<typename Dimension>
    void
    CullenDehnenViscosity<Dimension>::
    betaD(Scalar val)
    {
        mbetaD = val;
    }

    template<typename Dimension>
    void
    CullenDehnenViscosity<Dimension>::
    betaC(Scalar val)
    {
        mbetaC = val;
    }

    template<typename Dimension>
    void
    CullenDehnenViscosity<Dimension>::
    fKern(Scalar val)
    {
        mfKern = val;
    }

    template<typename Dimension>
    void
    CullenDehnenViscosity<Dimension>::
    boolHopkins(bool val)
    {
        mboolHopkins = val;
    }
    


    //------------------------------------------------------------------------------
    // Access the main kernel
    //------------------------------------------------------------------------------
    template<typename Dimension>
    inline
    const KernelSpace::TableKernel<Dimension>&
    CullenDehnenViscosity<Dimension>::kernel() const {
      return mKernel;
    }

    template<typename Dimension>
    inline
    const FieldSpace::FieldList<Dimension, typename Dimension::Vector>&
    CullenDehnenViscosity<Dimension>::PrevDvDt() const {
       return mPrevDvDt;
    }
    template<typename Dimension>
    inline
    const FieldSpace::FieldList<Dimension, typename Dimension::Scalar>&
    CullenDehnenViscosity<Dimension>::PrevDivV() const {
       return mPrevDivV;
    }
    template<typename Dimension>
    inline
    const FieldSpace::FieldList<Dimension, typename Dimension::Scalar>&
    CullenDehnenViscosity<Dimension>::PrevDivV2() const {
       return mPrevDivV2;
    }
    template<typename Dimension>
    inline
    const FieldSpace::FieldList<Dimension, typename Dimension::Scalar>&
    CullenDehnenViscosity<Dimension>::CullAlpha() const {
       return mCullAlpha;
    }
    template<typename Dimension>
    inline
    const FieldSpace::FieldList<Dimension, typename Dimension::Scalar>&
    CullenDehnenViscosity<Dimension>::CullAlpha2() const {
       return mCullAlpha2;
    }

    template<typename Dimension>
    typename Dimension::Scalar
    CullenDehnenViscosity<Dimension>::
    alphMax() const{return malphMax;}

    template<typename Dimension>
    typename Dimension::Scalar
    CullenDehnenViscosity<Dimension>::
    alphMin() const{return malphMin;}

    template<typename Dimension>
    typename Dimension::Scalar
    CullenDehnenViscosity<Dimension>::
    betaE() const{return mbetaE;}

    template<typename Dimension>
    typename Dimension::Scalar
    CullenDehnenViscosity<Dimension>::
    betaD() const{return mbetaD;}

    template<typename Dimension>
    typename Dimension::Scalar
    CullenDehnenViscosity<Dimension>::
    betaC() const{return mbetaC;}

    template<typename Dimension>
    typename Dimension::Scalar
    CullenDehnenViscosity<Dimension>::
    fKern() const{return mfKern;}

    template<typename Dimension>
    bool CullenDehnenViscosity<Dimension>::
    boolHopkins() const{return mboolHopkins;}
    
    
//------------------------------------------------------------------------------
// On problem start up, we need to initialize our internal data.
//------------------------------------------------------------------------------
template<typename Dimension>
void
CullenDehnenViscosity<Dimension>::
initializeProblemStartup(DataBase<Dimension>& dataBase) {
    mPrevDvDt = dataBase.newFluidFieldList(Vector::zero, "mPrevDvDt");
    mPrevDivV = dataBase.newFluidFieldList(0.0, "mPrevDivV");
    mCullAlpha = dataBase.newFluidFieldList(1.0, "mCullAlpha");
    mPrevDivV2 = dataBase.newFluidFieldList(0.0, "mPrevDivV2");
    mCullAlpha2 = dataBase.newFluidFieldList(1.0, "mCullAlpha2");
}
    

//------------------------------------------------------------------------------
// Register the state we need/are going to evolve.
//------------------------------------------------------------------------------
template<typename Dimension>
void
    CullenDehnenViscosity<Dimension>::
    registerState(DataBase<Dimension>& dataBase,
                  State<Dimension>& state) {
        typedef typename State<Dimension>::PolicyPointer PolicyPointer;
        dataBase.resizeFluidFieldList(mPrevDvDt, Vector::zero, "mPrevDvDt", false);
        dataBase.resizeFluidFieldList(mPrevDivV, 0.0, "mPrevDivV", false);
        dataBase.resizeFluidFieldList(mCullAlpha, 1.0, "mCullAlpha", false);
        state.enroll(mPrevDvDt);
        state.enroll(mPrevDivV);
        state.enroll(mCullAlpha);

        FieldList<Dimension, Scalar>& rvAlphaQ = myq.reducingViscosityMultiplierQ();
        FieldList<Dimension, Scalar>& rvAlphaL = myq.reducingViscosityMultiplierL();
        dataBase.resizeFluidFieldList(rvAlphaQ, 1.0, HydroFieldNames::reducingViscosityMultiplierQ,false);
        dataBase.resizeFluidFieldList(rvAlphaL, 1.0, HydroFieldNames::reducingViscosityMultiplierL,false);
        state.enroll(rvAlphaQ);
        state.enroll(rvAlphaL);

}
    
//------------------------------------------------------------------------------
// Register the state derivative fields.
//------------------------------------------------------------------------------
template<typename Dimension>
void
CullenDehnenViscosity<Dimension>::
registerDerivatives(DataBase<Dimension>& dataBase,
                    StateDerivatives<Dimension>& derivs) {

    dataBase.resizeFluidFieldList(mPrevDivV2, 0.0, "mPrevDivV2", false);
    dataBase.resizeFluidFieldList(mCullAlpha2, 1.0, "mCullAlpha2", false);
    derivs.enroll(mPrevDivV2);
    derivs.enroll(mCullAlpha2);

}

template<typename Dimension>
void
CullenDehnenViscosity<Dimension>::
    evaluateDerivatives(const typename Dimension::Scalar time,
                        const typename Dimension::Scalar dt,
                    const DataBase<Dimension>& dataBase,
                    const State<Dimension>& state,
                    StateDerivatives<Dimension>& derivs) const {
}
//------------------------------------------------------------------------------
// Determine the Cullen Coefficients
//------------------------------------------------------------------------------
template<typename Dimension>
void
CullenDehnenViscosity<Dimension>::
initialize(const typename Dimension::Scalar time,
                    const typename Dimension::Scalar dt,
                    const DataBase<Dimension>& dataBase,
                    State<Dimension>& state,
                    StateDerivatives<Dimension>& derivs) {
    // The kernels
    const TableKernel<Dimension>& W = this->kernel();

    // The connectivity.
    const ConnectivityMap<Dimension>& connectivityMap = dataBase.connectivityMap();
    const vector<const NodeList<Dimension>*>& nodeLists = connectivityMap.nodeLists();
    const size_t numNodeLists = nodeLists.size();
    
    //State Fluid Lists
    FieldList<Dimension, Scalar> reducingViscosityMultiplierQ = state.fields(HydroFieldNames::reducingViscosityMultiplierQ, 0.0);
    FieldList<Dimension, Scalar> reducingViscosityMultiplierL = state.fields(HydroFieldNames::reducingViscosityMultiplierL, 0.0);

    const FieldList<Dimension, Vector> position = state.fields(HydroFieldNames::position, Vector::zero);
    const FieldList<Dimension, Scalar> mass = state.fields(HydroFieldNames::mass, 0.0);
    const FieldList<Dimension, Vector> velocity = state.fields(HydroFieldNames::velocity, Vector::zero);
    const FieldList<Dimension, Scalar> massDensity = state.fields(HydroFieldNames::massDensity, 0.0);
    const FieldList<Dimension, SymTensor> H = state.fields(HydroFieldNames::H, SymTensor::zero);
    const FieldList<Dimension, Scalar> soundSpeed = state.fields(HydroFieldNames::soundSpeed, 0.0);
    const FieldList<Dimension, Scalar> PSPHpbar = state.fields(HydroFieldNames::PSPHpbar, 0.0);
  
    CHECK(mass.size() == numNodeLists);
    CHECK(position.size() == numNodeLists);
    CHECK(velocity.size() == numNodeLists);
    CHECK(massDensity.size() == numNodeLists);
    CHECK(H.size() == numNodeLists);
    CHECK(soundSpeed.size() == numNodeLists);
  
    
    CHECK(reducingViscosityMultiplierQ.size() == numNodeLists);
    CHECK(reducingViscosityMultiplierL.size() == numNodeLists);


    // Start our big loop over all FluidNodeLists.
    size_t nodeListi = 0;

    FieldList<Dimension, Vector> prevDvDt = state.fields("mPrevDvDt", Vector::zero);
    FieldList<Dimension, Scalar> prevDivV = state.fields("mPrevDivV", 0.0);
    FieldList<Dimension, Scalar> cullAlpha = state.fields("mCullAlpha", 0.0);
    FieldList<Dimension, Scalar> prevDivV2 = derivs.fields("mPrevDivV2", 0.0);
    FieldList<Dimension, Scalar> cullAlpha2 = derivs.fields("mCullAlpha2", 0.0);
    FieldList<Dimension, Tensor> cull_D(FieldSpace::Copy);
    FieldList<Dimension, Tensor> cull_T(FieldSpace::Copy);
    FieldList<Dimension, Tensor> cull_Da(FieldSpace::Copy);
    FieldList<Dimension, Scalar> cull_R(FieldSpace::Copy);
    FieldList<Dimension, Scalar> cull_sigv(FieldSpace::Copy);
    FieldList<Dimension, Scalar> temp_arr(FieldSpace::Copy);
    for (size_t nodeListk = 0; nodeListk != numNodeLists; ++nodeListk) {
      const NodeList<Dimension>& nodeList = position[nodeListk]->nodeList();
      cull_D.appendNewField("Cullen D Tensor", nodeList, Tensor::zero);
      cull_T.appendNewField("Cullen T Tensor", nodeList, Tensor::zero);
      cull_Da.appendNewField("Cullen another D Tensor for calculating A Tensor", nodeList, Tensor::zero);
      cull_R.appendNewField("Cullen R Scalar", nodeList, 0.0);
      cull_sigv.appendNewField("Cullen signal velocity Scalar", nodeList, 0.0);
      temp_arr.appendNewField("Scractch", nodeList, 0.0);
    }
  
    for (typename DataBase<Dimension>::ConstFluidNodeListIterator itr = dataBase.fluidNodeListBegin();
         itr != dataBase.fluidNodeListEnd();
         ++itr, ++nodeListi) {
      const NodeList<Dimension>& nodeList = **itr;
  
      // Iterate over the internal nodes in this NodeList.
      for (typename ConnectivityMap<Dimension>::const_iterator iItr = connectivityMap.begin(nodeListi);
           iItr != connectivityMap.end(nodeListi);
           ++iItr) {
        const int i = *iItr;
  
        // Get the state for node i.
        const Vector& ri = position(nodeListi, i);
        const SymTensor& Hi = H(nodeListi, i);
        const Scalar Hdeti = Hi.Determinant();
        const Vector& vi = velocity(nodeListi, i);
        const Scalar& rhoi = massDensity(nodeListi, i);
        const Scalar& ci = soundSpeed(nodeListi, i);
  
        // Get the connectivity info for this node.
        const vector< vector<int> >& fullConnectivity = connectivityMap.connectivityForNode(&nodeList, i);
  
        // Iterate over the NodeLists.
        for (size_t nodeListj = 0; nodeListj != numNodeLists; ++nodeListj) {
  
          // Connectivity of this node with this NodeList.  We only need to proceed if
          // there are some nodes in this list.
          const vector<int>& connectivity = fullConnectivity[nodeListj];
          if (connectivity.size() > 0) {
  
            // Loop over the neighbors.
  #pragma vector always
            for (vector<int>::const_iterator jItr = connectivity.begin();
                 jItr != connectivity.end();
                 ++jItr) {
                const int j = *jItr;
  
                // Get the state for node j
                const Vector& rj = position(nodeListj, j);
                const Scalar& mj = mass(nodeListj, j);
                const SymTensor& Hj = H(nodeListj, j);
                const Scalar Hdetj = Hj.Determinant();
                const Vector& vj = velocity(nodeListj, j);
                const Scalar& rhoj = massDensity(nodeListj, j);
                const Scalar& cj = soundSpeed(nodeListj, j);
                CHECK(mj > 0.0);
                CHECK(rhoj > 0.0);
                CHECK(Hdetj > 0.0);
  
                // Node displacement.
                const Vector rij = ri - rj;
                const Vector etai = Hi*rij;
                const Vector etaj = Hj*rij;
                const Scalar etaMagi = etai.magnitude();
                const Scalar etaMagj = etaj.magnitude();
                CHECK(etaMagi >= 0.0);
                CHECK(etaMagj >= 0.0);
  
                // Symmetrized kernel weight and gradient.
                const Vector Hetai = Hi*etai.unitVector();
                const std::pair<double, double> WWi = W.kernelAndGradValue(etaMagi, Hdeti);
                const Scalar Wi = WWi.first;
                const Scalar gWi = WWi.second;
                const Vector gradWi = gWi*Hetai;
  
                const Vector Hetaj = Hj*etaj.unitVector();
                const std::pair<double, double> WWj = W.kernelAndGradValue(etaMagj, Hdetj);
                const Scalar Wj = WWj.first;
                const Scalar gWj = WWj.second;
                const Vector gradWj = gWj*Hetaj;
  
          
                const Vector dvij= prevDvDt(nodeListi, i) - prevDvDt(nodeListj, j); 
                const Vector vij = vi - vj;
                const Scalar til_wij = mj*gWi*safeInv(Hdeti*etaMagi*rhoj);//Cullen weights \tilde{w_{ij}} = w'(|eta_ij|)/|eta_ij|, and W=detH w(|eta|)
                cull_D(nodeListi,i) += vij.dyad(rij)*til_wij;
                cull_Da(nodeListi,i) += dvij.dyad(rij)*til_wij;
                cull_T(nodeListi,i) += rij.selfdyad()*til_wij;
                const Scalar& divV = prevDivV(nodeListj, j);
                cull_R(nodeListi,i) += mj*Wi*((0.0 < divV)-(divV < 0.0));
                Scalar& sigvi= cull_sigv(nodeListi,i);
                sigvi = max((0.5*(ci+cj)-min(0.0,vij.dot(rij.unitVector()))),sigvi);
            }
          }
        }
        cull_R(nodeListi,i) = cull_R(nodeListi,i)/rhoi;
      }
    }
  
    for (ConstBoundaryIterator boundaryItr = this->boundaryBegin(); 
         boundaryItr != this->boundaryEnd();
         ++boundaryItr) {
    (*boundaryItr)->applyFieldListGhostBoundary(cull_D);
    (*boundaryItr)->applyFieldListGhostBoundary(cull_Da);
    (*boundaryItr)->applyFieldListGhostBoundary(cull_T);
    (*boundaryItr)->applyFieldListGhostBoundary(cull_R);
    (*boundaryItr)->applyFieldListGhostBoundary(cull_sigv);
    (*boundaryItr)->applyFieldListGhostBoundary(prevDvDt);
    (*boundaryItr)->applyFieldListGhostBoundary(prevDivV);
    (*boundaryItr)->applyFieldListGhostBoundary(cullAlpha);
    (*boundaryItr)->applyFieldListGhostBoundary(prevDivV2);
    (*boundaryItr)->applyFieldListGhostBoundary(cullAlpha2);
    }
    for (ConstBoundaryIterator boundaryItr = this->boundaryBegin(); 
           boundaryItr != this->boundaryEnd();
           ++boundaryItr) (*boundaryItr)->finalizeGhostBoundary();
    nodeListi=0; //Reset nodeListi to zero


    // Start our big loop over all FluidNodeLists.
    // size_t nodeListi = 0;
    for (typename DataBase<Dimension>::ConstFluidNodeListIterator itr = dataBase.fluidNodeListBegin();
         itr != dataBase.fluidNodeListEnd();
         ++itr, ++nodeListi) {
      const NodeList<Dimension>& nodeList = **itr;

      // Iterate over the internal nodes in this NodeList.
      for (typename ConnectivityMap<Dimension>::const_iterator iItr = connectivityMap.begin(nodeListi);
           iItr != connectivityMap.end(nodeListi);
           ++iItr) {
        const int i = *iItr;
  
        const SymTensor& Hi = H(nodeListi, i);
        const Scalar& ci = soundSpeed(nodeListi, i);
        const Scalar invhi = (Hi.Trace()/Dimension::nDim);
        const Tensor hat_Vi = cull_D(nodeListi, i)*(cull_T(nodeListi, i).Inverse());
        const Tensor hat_Ai = cull_Da(nodeListi, i)*(cull_T(nodeListi, i).Inverse());
        //Scalar& div_Vi = prevDivV2(nodeListi, i);
        Scalar& div_Vi = prevDivV(nodeListi, i);
        div_Vi = hat_Vi.Trace();
        const Scalar DdivViDt = (hat_Ai-hat_Vi*hat_Vi).Trace();
        const Tensor Si = 0.5*(hat_Vi+hat_Vi.Transpose())-div_Vi/Dimension::nDim*Tensor::one;
        const Scalar cull_etaConsti = pow(abs(mbetaE*pow((1.0-cull_R(nodeListi, i)),4.0)*div_Vi),2.0);
        const Scalar cull_etai = cull_etaConsti*safeInv(cull_etaConsti+((Si*(Si.Transpose())).Trace()));
        const Scalar Ai = cull_etai*max(-DdivViDt,0.0);
        const Scalar alph_loci = malphMax*Ai*safeInv(Ai+cull_sigv(nodeListi, i)*cull_sigv(nodeListi, i)*invhi*invhi);
        
        const Scalar taui = safeInv(invhi*2.0*mbetaD*cull_sigv(nodeListi, i));
        const Scalar old_alpha_i = cullAlpha(nodeListi, i);
        const Scalar alph_tmpi = (DdivViDt < 0.0 ) ? malphMax*abs(DdivViDt)*safeInv(abs(DdivViDt)+mbetaC*ci*ci*invhi*invhi/mfKern/mfKern): 0.0;
        const Scalar alph_zeroi = (alph_tmpi >= old_alpha_i) ? alph_tmpi : alph_tmpi + (old_alpha_i-alph_tmpi)*exp(-mbetaD*dt*abs(cull_sigv(nodeListi, i))*invhi/2.0/mfKern);
        //Scalar& alpha_i = cullAlpha2(nodeListi, i);
        Scalar& alpha_i = cullAlpha(nodeListi, i);
        temp_arr(nodeListi, i)=alph_loci;
        if(old_alpha_i < alph_loci)
          alpha_i = alph_loci;  
        else
          alpha_i = alph_loci + (old_alpha_i-alph_loci)*exp(-dt*safeInv(taui));

        if(mboolHopkins)alpha_i = max(cull_etai*alph_zeroi,malphMin);//Use Hopkins Reformulated Alpha
        reducingViscosityMultiplierQ(nodeListi, i) = alpha_i;
        reducingViscosityMultiplierL(nodeListi, i) = alpha_i;
        if(mboolHopkins)alpha_i = alph_zeroi;//Hopkins evolves alpha_zero, not alpha
        
                  
  /*
        if(i== 10 && nodeListi==0){
                  //cout << "AFTER i=" << i << " temp_arr=" << temp_arr(nodeListi, i) << endl;
                  //cout << "AFTER i=" << i << " prevDivVi=" << prevDivV(nodeListi, i) << " prevDivVi2=" << prevDivV2(nodeListi, i) << endl;
                  //cout << "AFTER i=" << i << " cullAlpha=" << cullAlpha(nodeListi, i) << " cullAlpha2=" << cullAlpha2(nodeListi, i) <<endl;
        }
  */
  
      }
    }
        
}
//------------------------------------------------------------------------------
// Finalize
//------------------------------------------------------------------------------
template<typename Dimension>
void
CullenDehnenViscosity<Dimension>::
finalize(const typename Dimension::Scalar time,
         const typename Dimension::Scalar dt,
         DataBase<Dimension>& dataBase,
         State<Dimension>& state,
         StateDerivatives<Dimension>& derivs) {
    FieldList<Dimension, Vector> prevDvDt = state.fields("mPrevDvDt", Vector::zero);
    FieldList<Dimension, Scalar> prevDivV = state.fields("mPrevDivV", 0.0);
    FieldList<Dimension, Scalar> cullAlpha = state.fields("mCullAlpha", 0.0);
    FieldList<Dimension, Scalar> prevDivV2 = derivs.fields("mPrevDivV2", 0.0);
    FieldList<Dimension, Scalar> cullAlpha2 = derivs.fields("mCullAlpha2", 0.0);
    const FieldList<Dimension, Vector> DvDt = derivs.fields(IncrementFieldList<Dimension, Vector>::prefix() + HydroFieldNames::velocity, Vector::zero);
    const unsigned numNodeLists = prevDivV.size();
    for (unsigned nodeListi = 0; nodeListi != numNodeLists; ++nodeListi) {
      const unsigned n = prevDvDt[nodeListi]->numInternalElements();
      for (unsigned i = 0; i != n; ++i) {
/*
      if(i== 10 && nodeListi==0){
                cout << "BEFORE2 i=" << i << " prevDivVi=" << prevDivV(nodeListi, i) << " prevDivVi2=" << prevDivV2(nodeListi, i) << endl;
                cout << "BEFORE2 i=" << i << " cullAlpha=" << cullAlpha(nodeListi, i) << " cullAlpha2=" << cullAlpha2(nodeListi, i) <<endl;
      }
*/
          prevDvDt(nodeListi, i)= DvDt(nodeListi, i);//Copy over the stored values from the previous timestep
          //prevDivV(nodeListi, i)=prevDivV2(nodeListi, i);
         // cullAlpha(nodeListi, i)=cullAlpha2(nodeListi, i);
/*
      if(i== 10 && nodeListi==0){
                cout << "AFTER2 i=" << i << " prevDivVi=" << prevDivV(nodeListi, i) << " prevDivVi2=" << prevDivV2(nodeListi, i) << endl;
                cout << "AFTER2 i=" << i << " cullAlpha=" << cullAlpha(nodeListi, i) << " cullAlpha2=" << cullAlpha2(nodeListi, i) <<endl;
      }
*/
      }
    }
}

//------------------------------------------------------------------------------
// Dump the current state to the given file.
//------------------------------------------------------------------------------
template<typename Dimension>
void
CullenDehnenViscosity<Dimension>::
dumpState(FileIO& file, string pathName) const {
    file.write(mPrevDvDt, pathName + "/prevDvDt");
    file.write(mPrevDivV, pathName + "/prevDivV");
    file.write(mCullAlpha, pathName + "/cullAlpha");
    file.write(mPrevDivV2, pathName + "/prevDivV2");
    file.write(mCullAlpha2, pathName + "/cullAlpha2");
}
    
//------------------------------------------------------------------------------
// Restore the state from the given file.
//------------------------------------------------------------------------------
template<typename Dimension>
void
CullenDehnenViscosity<Dimension>::
restoreState(const FileIO& file, string pathName) {
    file.read(mPrevDvDt, pathName + "/prevDvDt");
    file.read(mPrevDivV, pathName + "/prevDivV");
    file.read(mCullAlpha, pathName + "/cullAlpha");
    file.read(mPrevDivV2, pathName + "/prevDivV2");
    file.read(mCullAlpha2, pathName + "/cullAlpha2");
}
    
//------------------------------------------------------------------------------
// Boundaries
//------------------------------------------------------------------------------
template<typename Dimension>
void
CullenDehnenViscosity<Dimension>::
    applyGhostBoundaries(State<Dimension>& state,
                         StateDerivatives<Dimension>& derivs)
{
    FieldList<Dimension, Vector> prevDvDt = state.fields("mPrevDvDt", Vector::zero);
    FieldList<Dimension, Scalar> prevDivV = state.fields("mPrevDivV", 0.0);
    FieldList<Dimension, Scalar> cullAlpha = state.fields("mCullAlpha", 0.0);
    FieldList<Dimension, Scalar> prevDivV2 = derivs.fields("mPrevDivV2", 0.0);
    FieldList<Dimension, Scalar> cullAlpha2 = derivs.fields("mCullAlpha2", 0.0);
    for (ConstBoundaryIterator boundaryItr = this->boundaryBegin();
       boundaryItr != this->boundaryEnd();
       ++boundaryItr) {
      (*boundaryItr)->applyFieldListGhostBoundary(prevDvDt);
      (*boundaryItr)->applyFieldListGhostBoundary(prevDivV);
      (*boundaryItr)->applyFieldListGhostBoundary(cullAlpha);
      (*boundaryItr)->applyFieldListGhostBoundary(prevDivV2);
      (*boundaryItr)->applyFieldListGhostBoundary(cullAlpha2);
    }

}
    
template<typename Dimension>
void
CullenDehnenViscosity<Dimension>::
enforceBoundaries(State<Dimension>& state,
                     StateDerivatives<Dimension>& derivs)
{
    FieldList<Dimension, Vector> prevDvDt = state.fields("mPrevDvDt", Vector::zero);
    FieldList<Dimension, Scalar> prevDivV = state.fields("mPrevDivV", 0.0);
    FieldList<Dimension, Scalar> cullAlpha = state.fields("mCullAlpha", 0.0);
    FieldList<Dimension, Scalar> prevDivV2 = derivs.fields("mPrevDivV2", 0.0);
    FieldList<Dimension, Scalar> cullAlpha2 = derivs.fields("mCullAlpha2", 0.0);
    for (ConstBoundaryIterator boundaryItr = this->boundaryBegin();
       boundaryItr != this->boundaryEnd();
       ++boundaryItr) {
      (*boundaryItr)->enforceFieldListBoundary(prevDvDt);
      (*boundaryItr)->enforceFieldListBoundary(prevDivV);
      (*boundaryItr)->enforceFieldListBoundary(cullAlpha);
      (*boundaryItr)->enforceFieldListBoundary(prevDivV2);
      (*boundaryItr)->enforceFieldListBoundary(cullAlpha2);
    }
}
    
//------------------------------------------------------------------------------
// Vote on a time step.
//------------------------------------------------------------------------------
template<typename Dimension>
typename CullenDehnenViscosity<Dimension>::TimeStepType
CullenDehnenViscosity<Dimension>::
dt(const DataBase<Dimension>& dataBase,
   const State<Dimension>& state,
   const StateDerivatives<Dimension>& derivs,
   const Scalar currentTime) const {
    return TimeStepType(1.0e100, "Rate of viscosity change -- NO VOTE.");
}

}    
}