#ifndef TauAnalysis_CandidateTools_SVfitAlgorithm_h
#define TauAnalysis_CandidateTools_SVfitAlgorithm_h

/** \class SVfitAlgorithm
 *
 * Class used to reconstruct di-tau invariant mass via fitting/likelihood techniques.
 *
 * The fit is performed by passing log-likelihood functions to be minimized to Minuit;
 * the actual likelihood functions are implememted as plugins,
 * providing flexibility to reconstruct multiple di-tau invariant mass solutions
 * using different combinations of
 *  kinematic, missing transverse momentum, tau lepton lifetime,...
 * information.
 * 
 * \author Evan Friis, Christian Veelken; UC Davis
 *
 * \version $Revision: 1.13 $
 *
 * $Id: SVfitAlgorithm.h,v 1.13 2010/09/08 13:27:14 veelken Exp $
 *
 */

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "TauAnalysis/CandidateTools/interface/SVfitDiTauLikelihoodBase.h"
#include "TauAnalysis/CandidateTools/interface/SVfitEventVertexRefitter.h"
#include "TauAnalysis/CandidateTools/interface/SVfitLegTrackExtractor.h"
#include "TauAnalysis/CandidateTools/interface/svFitAuxFunctions.h"

#include "AnalysisDataFormats/TauAnalysis/interface/SVfitDiTauSolution.h"
#include "AnalysisDataFormats/TauAnalysis/interface/SVfitLegSolution.h"

#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include <TFitterMinuit.h>
#include <Minuit2/FCNBase.h>
#include <TMath.h>
#include <TMatrixDSym.h>
#include <TDecompChol.h>
#include <TRandom3.h>

#include <Math/VectorUtil.h>

#include <vector>
#include <algorithm>
#include <iostream>

// forward declaration of SVfitAlgorithm class
template<typename T1, typename T2> class SVfitAlgorithm;

template<typename T1, typename T2>
class SVfitMinuitFCNadapter : public ROOT::Minuit2::FCNBase
{
 public:
  SVfitMinuitFCNadapter()
    : svFitAlgorithm_(0)
  {}
  ~SVfitMinuitFCNadapter() {}

  void setSVfitAlgorithm(const SVfitAlgorithm<T1,T2>* svFitAlgorithm) { svFitAlgorithm_ = svFitAlgorithm; }

  /// define "objective" function called by Minuit
  double operator()(const std::vector<double>& x) const { return svFitAlgorithm_->negLogLikelihood(x); }

  /// define increase in "objective" function used by Minuit to determine one-sigma error contours;
  /// in case of (negative) log-likelihood functions, the value needs to be 0.5
  double Up() const { return 0.5; }
 private:
  const SVfitAlgorithm<T1,T2>* svFitAlgorithm_;
};

namespace SVfit_namespace 
{
  enum fitParameter { kPrimaryVertexX, kPrimaryVertexY, kPrimaryVertexZ,
                      kLeg1thetaRest, kLeg1phiLab, kLeg1flightPathLab, kLeg1nuInvMass,
		      kLeg1thetaVMrho, kLeg1thetaVMa1, kLeg1thetaVMa1r, kLeg1phiVMa1r,
                      kLeg2thetaRest, kLeg2phiLab, kLeg2flightPathLab, kLeg2nuInvMass,
		      kLeg2thetaVMrho, kLeg2thetaVMa1, kLeg2thetaVMa1r, kLeg2phiVMa1r };
  enum tauDecayProducts { kLeg1, kLeg2 };
}

template<typename T1, typename T2>
class SVfitAlgorithm
{
 public:
  SVfitAlgorithm(const edm::ParameterSet& cfg)
    : currentDiTau_(0)
  {
    name_ = cfg.getParameter<std::string>("name");

    eventVertexRefitAlgorithm_ = new SVfitEventVertexRefitter(cfg);

    likelihoodsSupportPolarization_ = false;

    typedef std::vector<edm::ParameterSet> vParameterSet;
    vParameterSet cfgLikelihoodFunctions = cfg.getParameter<vParameterSet>("likelihoodFunctions");
    for ( vParameterSet::const_iterator cfgLikelihoodFunction = cfgLikelihoodFunctions.begin();
	  cfgLikelihoodFunction != cfgLikelihoodFunctions.end(); ++cfgLikelihoodFunction ) {
      std::string pluginType = cfgLikelihoodFunction->getParameter<std::string>("pluginType");
      typedef edmplugin::PluginFactory<SVfitDiTauLikelihoodBase<T1,T2>* (const edm::ParameterSet&)> SVfitDiTauLikelihoodPluginFactory;
      SVfitDiTauLikelihoodBase<T1,T2>* likelihoodFunction 
	= SVfitDiTauLikelihoodPluginFactory::get()->create(pluginType, *cfgLikelihoodFunction);
      likelihoodsSupportPolarization_ |= likelihoodFunction->supportsPolarization();
      likelihoodFunctions_.push_back(likelihoodFunction);
    }
    
//--- initialize Minuit    
    minuitFCNadapter_.setSVfitAlgorithm(this);

    minuit_ = new TFitterMinuit();
    minuit_->SetMinuitFCN(&minuitFCNadapter_);
//--- set Minuit strategy = 2,
//    in order to enable reliable error estimates
//    ( cf. http://www-cdf.fnal.gov/physics/statistics/recommendations/minuit.html )
    minuit_->SetStrategy(2);
    minuit_->SetMaxIterations(1000);

    std::cout << "<SVfitAlgorithm::SVfitAlgorithm>:" << std::endl;
    std::cout << " disabling MINUIT output..." << std::endl;
    minuit_->SetPrintLevel(-1);
    minuit_->SetErrorDef(0.5);

    minuit_->CreateMinimizer();
        
    minuitFittedParameterValues_.resize(minuitNumParameters_);
    
    if ( cfg.exists("estUncertainties") ) {
      edm::ParameterSet cfgEstUncertainties = cfg.getParameter<edm::ParameterSet>("estUncertainties");
      numSamplings_ = cfgEstUncertainties.getParameter<int>("numSamplings");

//--- make numSamplings parameter always an odd number,
//    so that the median of numSamplings random values is well defined
      if ( (numSamplings_ % 2) == 0 ) ++numSamplings_;
    }
    
    print(std::cout);
  }

  ~SVfitAlgorithm()
  {
    delete eventVertexRefitAlgorithm_;

    for ( typename std::vector<SVfitDiTauLikelihoodBase<T1,T2>*>::iterator it = likelihoodFunctions_.begin();
	  it != likelihoodFunctions_.end(); ++it ) {
      delete (*it);
    }
  }

  void beginEvent(edm::Event& evt, const edm::EventSetup& es)
  {
    eventVertexRefitAlgorithm_->beginEvent(evt, es);

    for ( typename std::vector<SVfitDiTauLikelihoodBase<T1,T2>*>::const_iterator likelihoodFunction = likelihoodFunctions_.begin();
	  likelihoodFunction != likelihoodFunctions_.end(); ++likelihoodFunction ) {
      (*likelihoodFunction)->beginEvent(evt, es);
    }
  }

  void print(std::ostream& stream) const
  {
    stream << "<SVfitAlgorithm::print>" << std::endl;    
    stream << " name = " << name_ << std::endl;
    for ( typename std::vector<SVfitDiTauLikelihoodBase<T1,T2>*>::const_iterator likelihoodFunction = likelihoodFunctions_.begin();
	  likelihoodFunction != likelihoodFunctions_.end(); ++likelihoodFunction ) {
      (*likelihoodFunction)->print(stream);
    }
    stream << " minuitNumParameters = " << minuitNumParameters_ << std::endl;
    stream << " numSamplings = " << numSamplings_ << std::endl;
    stream << std::endl;
  }

  std::vector<SVfitDiTauSolution> fit(const CompositePtrCandidateT1T2MEt<T1,T2>& diTauCandidate)
  {
    //std::cout << "<SVfitAlgorithm::fit>:" << std::endl;

    std::vector<SVfitDiTauSolution> solutions;
    
//--- refit primary event vertex
//    excluding tracks associated to tau decay products
    std::vector<reco::TrackBaseRef> leg1Tracks = leg1TrackExtractor_(*diTauCandidate.leg1());
    std::vector<reco::TrackBaseRef> leg2Tracks = leg2TrackExtractor_(*diTauCandidate.leg2());
    TransientVertex pv = eventVertexRefitAlgorithm_->refit(leg1Tracks, leg2Tracks);
    
//--- check if at least one likelihood function supports polarization 
//    (i.e. returns a polarization depent likelihood value);
//    in case none of the likelihood functions support polarization,
//    run the fit only once (for "unknown" polarization), 
//    in order to save computing time
    if ( likelihoodsSupportPolarization_ ) {
      for ( int leg1PolarizationHypothesis = SVfitLegSolution::kLeftHanded; 
	    leg1PolarizationHypothesis <= SVfitLegSolution::kRightHanded; ++leg1PolarizationHypothesis ) {
	for ( int leg2PolarizationHypothesis = SVfitLegSolution::kLeftHanded; 
	      leg2PolarizationHypothesis <= SVfitLegSolution::kRightHanded; ++leg2PolarizationHypothesis ) {
	  currentDiTauSolution_ = SVfitDiTauSolution((SVfitLegSolution::polarizationHypothesisType)leg1PolarizationHypothesis, 
						     (SVfitLegSolution::polarizationHypothesisType)leg2PolarizationHypothesis);
	  fitPolarizationHypothesis(diTauCandidate, currentDiTauSolution_, pv);
	  solutions.push_back(currentDiTauSolution_);
	} 
      }
    } else {
      currentDiTauSolution_ = SVfitDiTauSolution(SVfitLegSolution::kUnknown, SVfitLegSolution::kUnknown);
      fitPolarizationHypothesis(diTauCandidate, currentDiTauSolution_, pv);
      solutions.push_back(currentDiTauSolution_);
    }
    
    return solutions;
  }
  
  double negLogLikelihood(const std::vector<double>& x) const
  {
    //std::cout << "<SVfitAlgorithm::negLogLikelihood>:" << std::endl;
    //std::cout << " numParameters = " << numParameters << std::endl;

    if ( !currentDiTau_ ) {
      edm::LogError("SVfitAlgorithm::logLikelihood") 
	<< " Pointer to currentDiTau has not been initialized --> skipping !!";
      return 0.;
    }
    
    applyParameters(currentDiTauSolution_, x);
    
    double negLogLikelihood = 0.;    
    for ( typename std::vector<SVfitDiTauLikelihoodBase<T1,T2>*>::const_iterator likelihoodFunction = likelihoodFunctions_.begin();
	  likelihoodFunction != likelihoodFunctions_.end(); ++likelihoodFunction ) {
      negLogLikelihood += (**likelihoodFunction)(*currentDiTau_, currentDiTauSolution_);
    }
    
    return negLogLikelihood;
  }
  
 private:

  void fitPolarizationHypothesis(const CompositePtrCandidateT1T2MEt<T1,T2>& diTauCandidate,
				 SVfitDiTauSolution& solution,
				 const TransientVertex& pv)
  {
    if ( verbosity_ ) std::cout << "<SVfitAlgorithm::fitPolarizationHypothesis>:" << std::endl;
    
//--- initialize pointer to current diTau object
    currentDiTau_ = &diTauCandidate;

//--- initialize data-members of diTauSolution object
    if ( pv.isValid() ) {
      currentDiTauSolution_.eventVertexPosition_.SetXYZ(pv.position().x(), pv.position().y(), pv.position().z());
      currentDiTauSolution_.eventVertexPositionErr_ = pv.positionError().matrix();
    }
    currentDiTauSolution_.eventVertexIsValid_ = pv.isValid();
    currentDiTauSolution_.leg1_.p4Vis_ = diTauCandidate.leg1()->p4();
    currentDiTauSolution_.leg2_.p4Vis_ = diTauCandidate.leg2()->p4();
    
//--- initialize start-values of Minuit fit parameters
//
//    CV: how to deal with measurement errors in the visible momenta of the two tau decay "legs"
//        when setting the maximum mass of the neutrino system ?
//
    double pvPositionX, pvPositionXerr, pvPositionY, pvPositionYerr, pvPositionZ, pvPositionZerr;
    if ( pv.isValid() ) {
      pvPositionX = pv.position().x();
      pvPositionXerr = pv.positionError().cxx();
      pvPositionY = pv.position().y();
      pvPositionYerr = pv.positionError().cyy();
      pvPositionZ = pv.position().z();
      pvPositionZerr = pv.positionError().czz();
    } else {
      pvPositionX = 0.;
      pvPositionXerr = 0.1;
      pvPositionY = 0.;
      pvPositionYerr = 0.1;
      pvPositionZ = 0.;
      pvPositionZerr = 10.;
    }

    minuit_->SetParameter(SVfit_namespace::kPrimaryVertexX, "pv_x", pvPositionX, pvPositionXerr,  -1.,  +1.);
    minuit_->SetParameter(SVfit_namespace::kPrimaryVertexY, "pv_y", pvPositionY, pvPositionYerr,  -1.,  +1.);
    minuit_->SetParameter(SVfit_namespace::kPrimaryVertexZ, "pv_z", pvPositionZ, pvPositionZerr, -50., +50.);
    minuit_->SetParameter(SVfit_namespace::kLeg1thetaRest, "sv1_thetaRest", 0.25*TMath::Pi(), 0.5*TMath::Pi(), 0., TMath::Pi());
    minuit_->SetParameter(SVfit_namespace::kLeg1phiLab, "sv1_phiLab", 0., TMath::Pi(), 0., 0.); // do not set limits for phiLab
    double leg1Radius0 = diTauCandidate.leg1()->energy()*SVfit_namespace::cTauLifetime/SVfit_namespace::tauLeptonMass;
    minuit_->SetParameter(SVfit_namespace::kLeg1flightPathLab, "sv1_radiusLab", leg1Radius0, leg1Radius0, 0., 100.*leg1Radius0); 
    double leg1NuMass0, leg1NuMassErr, leg1NuMassMax;
    if ( !SVfit_namespace::isMasslessNuSystem<T1>() ) {
      leg1NuMass0 = 0.8;
      leg1NuMassErr = 0.4;
      leg1NuMassMax = SVfit_namespace::tauLeptonMass - diTauCandidate.leg1()->mass();
    } else {
      leg1NuMass0 = 0.;
      leg1NuMassErr = 1.;
      leg1NuMassMax = 0.;
    }
    //std::cout << " leg1NuMassMax = " << leg1NuMassMax << std::endl;
    minuit_->SetParameter(SVfit_namespace::kLeg1nuInvMass, "sv1_m12", leg1NuMass0, leg1NuMassErr, 0., leg1NuMassMax);
    minuit_->SetParameter(SVfit_namespace::kLeg1thetaVMrho, "sv1_thetaVMrho", 0.25*TMath::Pi(), 0.5*TMath::Pi(), 0., TMath::Pi());
    minuit_->SetParameter(SVfit_namespace::kLeg1thetaVMa1, "sv1_thetaVMa1", 0.25*TMath::Pi(), 0.5*TMath::Pi(), 0., TMath::Pi());
    minuit_->SetParameter(SVfit_namespace::kLeg1thetaVMa1r, "sv1_thetaVMa1r", 0.25*TMath::Pi(), 0.5*TMath::Pi(), 0., TMath::Pi());
    minuit_->SetParameter(SVfit_namespace::kLeg1phiVMa1r, "sv1_phiVMa1r", 0., TMath::Pi(), 0., 0.); // do not set limits for phiVMa1r
    minuit_->SetParameter(SVfit_namespace::kLeg2thetaRest, "sv2_thetaRest", 0.25*TMath::Pi(), 0.5*TMath::Pi(), 0., TMath::Pi());
    minuit_->SetParameter(SVfit_namespace::kLeg2phiLab, "sv2_phiLab", 0., TMath::Pi(), 0., 0.); // do not set limits for phiLab
    double leg2Radius0 = diTauCandidate.leg2()->energy()*SVfit_namespace::cTauLifetime/SVfit_namespace::tauLeptonMass;
    minuit_->SetParameter(SVfit_namespace::kLeg2flightPathLab, "sv2_radiusLab", leg2Radius0, leg2Radius0, 0., 100.*leg2Radius0); 
    double leg2NuMass0, leg2NuMassErr, leg2NuMassMax;
    if ( !SVfit_namespace::isMasslessNuSystem<T2>() ) {
      leg2NuMass0 = 0.8;
      leg2NuMassErr = 0.4;
      leg2NuMassMax = SVfit_namespace::tauLeptonMass - diTauCandidate.leg2()->mass();
    } else {
      leg2NuMass0 = 0.;
      leg2NuMassErr = 1.;
      leg2NuMassMax = 0.;
    }
    //std::cout << " leg2NuMassMax = " << leg2NuMassMax << std::endl;
    minuit_->SetParameter(SVfit_namespace::kLeg2nuInvMass, "sv2_m12", leg2NuMass0, leg2NuMassErr, 0., leg2NuMassMax);
    minuit_->SetParameter(SVfit_namespace::kLeg2thetaVMrho, "sv2_thetaVMrho", 0.25*TMath::Pi(), 0.5*TMath::Pi(), 0., TMath::Pi());
    minuit_->SetParameter(SVfit_namespace::kLeg2thetaVMa1, "sv2_thetaVMa1", 0.25*TMath::Pi(), 0.5*TMath::Pi(), 0., TMath::Pi());
    minuit_->SetParameter(SVfit_namespace::kLeg2thetaVMa1r, "sv2_thetaVMa1r", 0.25*TMath::Pi(), 0.5*TMath::Pi(), 0., TMath::Pi());
    minuit_->SetParameter(SVfit_namespace::kLeg2phiVMa1r, "sv2_phiVMa1r", 0., TMath::Pi(), 0., 0.); // do not set limits for phiVMa1r
    
    for ( typename std::vector<SVfitDiTauLikelihoodBase<T1,T2>*>::const_iterator likelihoodFunction = likelihoodFunctions_.begin();
	  likelihoodFunction != likelihoodFunctions_.end(); ++likelihoodFunction ) {
      (*likelihoodFunction)->beginCandidate(diTauCandidate);
    }

//--- lock (i.e. set to fixed values) Minuit parameters
//    which are not constrained by any likelihood function
    for ( unsigned iParameter = 0; iParameter < minuitNumParameters_; ++iParameter ) {
      bool minuitLockParameter = true;
      for ( typename std::vector<SVfitDiTauLikelihoodBase<T1,T2>*>::const_iterator likelihoodFunction = likelihoodFunctions_.begin();
	    likelihoodFunction != likelihoodFunctions_.end(); ++likelihoodFunction ) {
	if ( (*likelihoodFunction)->isFittedParameter(iParameter) ) minuitLockParameter = false;
      }

      if (  minuitLockParameter && !minuit_->IsFixed(iParameter) ) minuit_->FixParameter(iParameter);
      if ( !minuitLockParameter &&  minuit_->IsFixed(iParameter) ) minuit_->ReleaseParameter(iParameter);

      std::cout << " Parameter #" << iParameter << ": ";
      if ( minuitLockParameter ) std::cout << "LOCKED";
      else std::cout << "FITTED";
      std::cout << std::endl;
    }
      
    minuitNumFreeParameters_ = minuit_->GetNumberFreeParameters();
    minuitNumFixedParameters_ = minuit_->GetNumberTotalParameters() - minuitNumFreeParameters_;
    
    std::cout << " minuitNumParameters = " << minuit_->GetNumberTotalParameters()
	      << " (free = " << minuitNumFreeParameters_ << ", fixed = " << minuitNumFixedParameters_ << ")" << std::endl;
    assert((minuitNumFreeParameters_ + minuitNumFixedParameters_) == minuitNumParameters_);

    int minuitStatus = minuit_->Minimize();
    edm::LogInfo("SVfitAlgorithm::fit") 
      << " Minuit fit Status = " << minuitStatus << std::endl;
	
    for ( unsigned iParameter = 0; iParameter < minuitNumParameters_; ++iParameter ) {
      minuitFittedParameterValues_[iParameter] = minuit_->GetParameter(iParameter);
      if ( verbosity_ ) std::cout << " Parameter #" << iParameter << " = " << minuitFittedParameterValues_[iParameter] << std::endl;
    }
    applyParameters(currentDiTauSolution_, minuitFittedParameterValues_);
    
    for ( typename std::vector<SVfitDiTauLikelihoodBase<T1,T2>*>::const_iterator likelihoodFunction = likelihoodFunctions_.begin();
	  likelihoodFunction != likelihoodFunctions_.end(); ++likelihoodFunction ) {
      double likelihoodFunctionValue = (**likelihoodFunction)(*currentDiTau_, currentDiTauSolution_);
      currentDiTauSolution_.logLikelihoods_.insert(std::make_pair((*likelihoodFunction)->name(), likelihoodFunctionValue));
    }

    currentDiTauSolution_.minuitStatus_ = minuitStatus;

    if ( numSamplings_ > 0 ) compErrorEstimates();
  }
  
  void applyParameters(SVfitDiTauSolution& diTauSolution, const std::vector<double>& x) const 
  {
//--- set primary event vertex position (tau lepton production vertex)
    diTauSolution.eventVertexPositionCorr_.SetX(x[SVfit_namespace::kPrimaryVertexX]);
    diTauSolution.eventVertexPositionCorr_.SetY(x[SVfit_namespace::kPrimaryVertexY]);
    diTauSolution.eventVertexPositionCorr_.SetZ(x[SVfit_namespace::kPrimaryVertexZ]);

//--- build first tau decay "leg"
    applyParametersToLeg(SVfit_namespace::kLeg1thetaRest, diTauSolution.leg1_, x);

//--- build second tau decay "leg"
    applyParametersToLeg(SVfit_namespace::kLeg2thetaRest, diTauSolution.leg2_, x);
  }

  void applyParametersToLeg(int index0, SVfitLegSolution& legSolution, const std::vector<double>& x) const 
  {
    int legOffset = index0 - SVfit_namespace::kLeg1thetaRest;
		     
    double gjAngle        = x[legOffset + SVfit_namespace::kLeg1thetaRest];
    double phiLab         = x[legOffset + SVfit_namespace::kLeg1phiLab];
    double flightDistance = x[legOffset + SVfit_namespace::kLeg1flightPathLab];
    double massNuNu       = x[legOffset + SVfit_namespace::kLeg1nuInvMass];

    const reco::Candidate::LorentzVector& p4Vis = legSolution.p4Vis();

    // Compute the tau momentum in the rest frame
    double pVisRestFrame = SVfit_namespace::pVisRestFrame(p4Vis.mass(), massNuNu);
    // Get the opening angle in the lab frame
    double angleVisLabFrame = SVfit_namespace::gjAngleToLabFrame(pVisRestFrame, gjAngle, p4Vis.P());
    // Compute the tau momentum in the lab frame
    double momentumLabFrame = SVfit_namespace::tauMomentumLabFrame(p4Vis.mass(), pVisRestFrame, gjAngle, p4Vis.P());
    // Determine the direction of the tau
    reco::Candidate::Vector direction = SVfit_namespace::tauDirection(p4Vis.Vect().Unit(), angleVisLabFrame, phiLab);

    reco::Candidate::LorentzVector tauP4 = SVfit_namespace::tauP4(direction, momentumLabFrame);

    // Build the tau four vector. By construction, the neutrino is tauP4 - visP4
    legSolution.p4Invis_ =  tauP4 - p4Vis;

    // Build boost vector and compute the rest frame quanitites
    reco::Candidate::Vector boost = tauP4.BoostToCM();
    legSolution.p4VisRestFrame_ = ROOT::Math::VectorUtil::boost(legSolution.p4Vis_, boost);
    legSolution.p4InvisRestFrame_ = ROOT::Math::VectorUtil::boost(legSolution.p4Invis_, boost);

    // Set the flight path
    legSolution.tauFlightPath_ = direction*flightDistance;

    // Set meson decay angles for tau- --> rho- nu --> pi- pi0 nu 
    // and tau- --> a1- nu --> pi- pi0 pi0 nu, tau- --> a1- nu --> pi- pi+ pi- nu decay modes
    // (needed in case likelihood functions for decays of polarized tau leptons are used)
    legSolution.thetaVMrho_ = x[legOffset + SVfit_namespace::kLeg1thetaVMrho];
    legSolution.thetaVMa1_  = x[legOffset + SVfit_namespace::kLeg1thetaVMa1];
    legSolution.thetaVMa1r_ = x[legOffset + SVfit_namespace::kLeg1thetaVMa1r];
    legSolution.phiVMa1r_   = x[legOffset + SVfit_namespace::kLeg1phiVMa1r];
  }

  void compErrorEstimates() const
  {
    //std::cout << "<SVfitAlgorithm::compErrorEstimates>:" << std::endl;

//--- compute estimate for error matrix using Minuit's HESSE algorithm
    minuit_->ExecuteCommand("HESSE", 0, 0);

//--- copy error matrix estimated by Minuit into TMatrixDSym object;
//    only copy elements corresponding to "free" fit parameters,
//    to avoid problems with Cholesky decomposition
    std::vector<unsigned> lutFreeToMinuitParameters(minuitNumFreeParameters_);
    std::vector<int> lutMinuitToFreeParameters(minuitNumParameters_);
    unsigned freeParameter_index = 0;
    for ( unsigned iParameter = 0; iParameter < minuitNumParameters_; ++iParameter ) {
      if ( !minuit_->IsFixed(iParameter) ) {
	lutMinuitToFreeParameters[iParameter] = freeParameter_index;
	lutFreeToMinuitParameters[freeParameter_index] = iParameter;
	++freeParameter_index;
      } else {
	lutMinuitToFreeParameters[iParameter] = -1;
      }
    }    

    TMatrixDSym freeErrorMatrix(minuitNumFreeParameters_);
    for ( unsigned iRow = 0; iRow < minuitNumFreeParameters_; ++iRow ) {
      //unsigned minuitRow_index = lutFreeToMinuitParameters[iRow];
      //std::cout << "iRow = " << iRow << ": minuitRow_index = " << minuitRow_index << std::endl;
      for ( unsigned iColumn = 0; iColumn < minuitNumFreeParameters_; ++iColumn ) {
	//unsigned minuitColumn_index = lutFreeToMinuitParameters[iColumn];
	//std::cout << "iColumn = " << iColumn << ": minuitColumn_index = " << minuitColumn_index << std::endl;
//--- CV: note that Minuit error matrix 
//        contains "free" fit parameters only
//       (i.e. is of dimension nF x nF, where nF is the number of "free" fit parameters)
	//freeErrorMatrix(iRow, iColumn) = minuit_->GetCovarianceMatrixElement(minuitRow_index, minuitColumn_index);
	freeErrorMatrix(iRow, iColumn) = minuit_->GetCovarianceMatrixElement(iRow, iColumn);
      }
    }
    //freeErrorMatrix.Print();

//--- decompose "physical" error matrix A
//    into "square-root" matrix A, with U * U^T = A
    TDecompChol choleskyDecompAlgorithm;
    choleskyDecompAlgorithm.SetMatrix(freeErrorMatrix);
    choleskyDecompAlgorithm.Decompose();
    TMatrixD freeErrorMatrix_sqrt = choleskyDecompAlgorithm.GetU(); 
    //freeErrorMatrix_sqrt.Print();

//--- generate random variables distributed according to N-dimensional normal distribution 
//    for mean vector m (vector of best fit paramaters determined by Minuit)
//    and covariance V (error matrix estimated by Minuit)
//
//    NB: correlations in the N-dimensional normal distribution
//        are generated by the affine transformation 
//          rndCorrelated = mu + U * rndUncorrelated 
//        described in section "Drawing values from the distribution" 
//        of http://en.wikipedia.org/wiki/Multivariate_normal_distribution
//
    std::vector<double> massValues(numSamplings_);
    std::vector<double> x1Values(numSamplings_);
    std::vector<double> x2Values(numSamplings_);
    TVectorD rndFreeParameterValues(minuitNumFreeParameters_);
    std::vector<double> rndParameterValues(minuitNumParameters_);
    SVfitDiTauSolution rndDiTauSolution(currentDiTauSolution_);
    int iSampling = 0;    
    while ( iSampling < numSamplings_ ) {      
      for ( unsigned iFreeParameter = 0; iFreeParameter < minuitNumFreeParameters_; ++iFreeParameter ) {
	rndFreeParameterValues(iFreeParameter) = rnd_.Gaus(0., 1.);
      }
      
      rndFreeParameterValues *= freeErrorMatrix_sqrt;
      
      for ( unsigned iParameter = 0; iParameter < minuitNumParameters_; ++iParameter ) {
	int freeParameter_index = lutMinuitToFreeParameters[iParameter];
	if ( freeParameter_index != -1 ) {
	  rndParameterValues[iParameter] = minuitFittedParameterValues_[iParameter] + rndFreeParameterValues(freeParameter_index);
	} else {
          rndParameterValues[iParameter] = minuitFittedParameterValues_[iParameter];
	}
      }

      applyParameters(rndDiTauSolution, rndParameterValues);

      double mass = rndDiTauSolution.mass();
      double x1 = rndDiTauSolution.leg1().x();
      double x2 = rndDiTauSolution.leg2().x();

      if ( !(TMath::IsNaN(mass) || TMath::IsNaN(x1) || TMath::IsNaN(x2)) ) {
	massValues[iSampling] = mass;
	x1Values[iSampling] = x1;
	x2Values[iSampling] = x2;
	++iSampling;
      }
    }

    std::sort(massValues.begin(), massValues.end());
    std::sort(x1Values.begin(), x1Values.end());
    std::sort(x2Values.begin(), x2Values.end());

    int median_index = TMath::Nint(0.50*numSamplings_);
    int oneSigmaUp_index = TMath::Nint(0.84*numSamplings_);
    int oneSigmaDown_index = TMath::Nint(0.16*numSamplings_);

    currentDiTauSolution_.hasErrorEstimates_ = true;
    currentDiTauSolution_.massErrUp_ = massValues[oneSigmaUp_index] - massValues[median_index];
    currentDiTauSolution_.massErrDown_ = massValues[median_index] - massValues[oneSigmaDown_index];

    currentDiTauSolution_.leg1_.hasErrorEstimates_ = true;
    currentDiTauSolution_.leg1_.xErrUp_ = x1Values[oneSigmaUp_index] - x1Values[median_index];
    currentDiTauSolution_.leg1_.xErrDown_ = x1Values[median_index] - x1Values[oneSigmaDown_index];

    currentDiTauSolution_.leg2_.hasErrorEstimates_ = true;
    currentDiTauSolution_.leg2_.xErrUp_ = x2Values[oneSigmaUp_index] - x2Values[median_index];
    currentDiTauSolution_.leg2_.xErrDown_ = x2Values[median_index] - x2Values[oneSigmaDown_index];
  }

  std::string name_;
  
  SVfitEventVertexRefitter* eventVertexRefitAlgorithm_;
  SVfitLegTrackExtractor<T1> leg1TrackExtractor_;
  SVfitLegTrackExtractor<T2> leg2TrackExtractor_;

  std::vector<SVfitDiTauLikelihoodBase<T1,T2>*> likelihoodFunctions_;
  bool likelihoodsSupportPolarization_;
  
  mutable const CompositePtrCandidateT1T2MEt<T1,T2>* currentDiTau_;
  mutable SVfitDiTauSolution currentDiTauSolution_;
  
  mutable TFitterMinuit* minuit_;
  SVfitMinuitFCNadapter<T1,T2> minuitFCNadapter_;
  const static unsigned minuitNumParameters_ = 19;
  mutable unsigned minuitNumFreeParameters_;
  mutable unsigned minuitNumFixedParameters_;
  mutable std::vector<double> minuitFittedParameterValues_;

  int numSamplings_;
  mutable TRandom3 rnd_;

  static const int verbosity_ = 0;
};

#endif
