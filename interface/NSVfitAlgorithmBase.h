#ifndef TauAnalysis_CandidateTools_NSVfitAlgorithmBase_h
#define TauAnalysis_CandidateTools_NSVfitAlgorithmBase_h

/** \class SVfitAlgorithmBase
 *
 * Abstract base-class for plugins finding best (n)SVfit solution,
 * either by integration or by fitting
 *
 * \author Christian Veelken, UC Davis
 *
 * \version $Revision: 1.5 $
 *
 * $Id: NSVfitAlgorithmBase.h,v 1.5 2011/03/03 13:04:47 veelken Exp $
 *
 */

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/Common/interface/Ptr.h"

#include "TauAnalysis/CandidateTools/interface/NSVfitEventLikelihood.h"
#include "TauAnalysis/CandidateTools/interface/NSVfitResonanceLikelihood.h"
#include "TauAnalysis/CandidateTools/interface/NSVfitSingleParticleLikelihood.h"
#include "TauAnalysis/CandidateTools/interface/NSVfitEventBuilderBase.h"

#include "AnalysisDataFormats/TauAnalysis/interface/NSVfitEventHypothesis.h"
#include "AnalysisDataFormats/TauAnalysis/interface/NSVfitResonanceHypothesis.h"
#include "AnalysisDataFormats/TauAnalysis/interface/NSVfitSingleParticleHypothesisBase.h"

#include <vector>
#include <string>

namespace SVfit_namespace
{
  enum fitParameter { 
    // fit parameters related to shifts of primary event vertex
    kPV_shiftX, kPV_shiftY, kPV_shiftZ,
    // fit parameters specific to tau decays
    kTau_visEnFracX, kTau_phi_lab, kTau_decayDistance_lab, kTau_nuInvMass,
    kTauVM_theta_rho, kTauVM_theta_a1, kTauVM_theta_a1r, kTauVM_phi_a1r,
    // fit parameters specific to electrons, muons not originating from tau decays
    kLep_shiftEn,
    // fit parameters specific to neutrinos (not originating from tau decays)
    kNu_energy_lab, kNu_phi_lab
  };
}

class NSVfitAlgorithmBase 
{
 public:
  NSVfitAlgorithmBase(const edm::ParameterSet&);
  virtual ~NSVfitAlgorithmBase();

  virtual void beginJob();
  virtual void beginEvent(const edm::Event&, const edm::EventSetup&);

  virtual void requestFitParameter(const std::string&, int, const std::string&);
  
  struct fitParameterType
  {
    std::string name_;
    int type_;
    std::vector<std::string> usedBy_; // list of SingleParticle likelihoods depending on this fitParameter
    double lowerLimit_;
    double upperLimit_;
    int idx_;
  };

  virtual fitParameterType* getFitParameter(const std::string&, int);
 
  virtual void print(std::ostream&) const {}

  typedef edm::Ptr<reco::Candidate> CandidatePtr;
  typedef std::map<std::string, CandidatePtr> inputParticleMap;
  virtual NSVfitEventHypothesis* fit(const inputParticleMap&) const;

  virtual double nll(double*, double*) const;

  static const NSVfitAlgorithmBase* gNSVfitAlgorithm;

 protected:
  virtual void fitImp() const = 0;

  std::string pluginName_;
  std::string pluginType_;

  struct daughterModelType
  {
    daughterModelType(const std::string& daughterName, const edm::ParameterSet& cfg, 
		      std::vector<NSVfitLikelihoodBase*>& allLikelihoods)
      : daughterName_(daughterName),
	prodParticleLabel_(cfg.getParameter<std::string>("prodParticleLabel"))
    {    
      typedef std::vector<edm::ParameterSet> vParameterSet;
      vParameterSet cfg_likelihoods = cfg.getParameter<vParameterSet>("likelihoodFunctions");
      for ( vParameterSet::iterator cfg_likelihood = cfg_likelihoods.begin();
	    cfg_likelihood != cfg_likelihoods.end(); ++cfg_likelihood ) {
	cfg_likelihood->addParameter<std::string>("prodParticleLabel", prodParticleLabel_);
	std::string pluginType = cfg_likelihood->getParameter<std::string>("pluginType");
	NSVfitSingleParticleLikelihood* likelihood = 
	  NSVfitSingleParticleLikelihoodPluginFactory::get()->create(pluginType, *cfg_likelihood);
	likelihoods_.push_back(likelihood);
	allLikelihoods.push_back(likelihood);
      }
    }
    ~daughterModelType()
    {
      for ( std::vector<NSVfitSingleParticleLikelihood*>::iterator it = likelihoods_.begin();
	    it != likelihoods_.end(); ++it ) {
	delete (*it);
      }
    }    
    void beginCandidate(const NSVfitSingleParticleHypothesisBase* hypothesis)
    {
      for ( std::vector<NSVfitSingleParticleLikelihood*>::const_iterator likelihood = likelihoods_.begin();
	    likelihood != likelihoods_.end(); ++likelihood ) {
	(*likelihood)->beginCandidate(hypothesis);
      }
    }
    double nll(NSVfitSingleParticleHypothesisBase* hypothesis) const
    {
      double retVal = 0.;
      for ( std::vector<NSVfitSingleParticleLikelihood*>::const_iterator likelihood = likelihoods_.begin();
	    likelihood != likelihoods_.end(); ++likelihood ) {
	retVal += (**likelihood)(hypothesis);
      }
      return retVal;
    }
    std::string daughterName_;
    std::string prodParticleLabel_;
    std::vector<NSVfitSingleParticleLikelihood*> likelihoods_;
  };

  struct resonanceModelType
  {
    resonanceModelType(const std::string& resonanceName, const edm::ParameterSet& cfg, 
		       std::vector<NSVfitLikelihoodBase*>& allLikelihoods)
      : resonanceName_(resonanceName)
    {
      typedef std::vector<edm::ParameterSet> vParameterSet;
      vParameterSet cfg_likelihoods = cfg.getParameter<vParameterSet>("likelihoodFunctions");
      for ( vParameterSet::const_iterator cfg_likelihood = cfg_likelihoods.begin();
	    cfg_likelihood != cfg_likelihoods.end(); ++cfg_likelihood ) {
	std::string pluginType = cfg_likelihood->getParameter<std::string>("pluginType");
	NSVfitResonanceLikelihood* likelihood = 
	  NSVfitResonanceLikelihoodPluginFactory::get()->create(pluginType, *cfg_likelihood);
	  likelihoods_.push_back(likelihood);
	  allLikelihoods.push_back(likelihood);	  
      }

      edm::ParameterSet cfg_daughters = cfg.getParameter<edm::ParameterSet>("daughters");
      typedef std::vector<std::string> vstring;
      vstring daughterNames = cfg_daughters.getParameterNamesForType<edm::ParameterSet>();
      for ( vstring::const_iterator daughterName = daughterNames.begin();
	    daughterName != daughterNames.end(); ++daughterName ) {
        edm::ParameterSet cfg_daughter = cfg_daughters.getParameter<edm::ParameterSet>(*daughterName);
	cfg_daughter.addParameter<std::string>("prodParticleLabel", *daughterName);
	daughters_.push_back(new daughterModelType(*daughterName, cfg_daughter, allLikelihoods));
      }
      numDaughters_ = daughters_.size();
    }
    ~resonanceModelType()
    {
      for ( std::vector<NSVfitResonanceLikelihood*>::iterator it = likelihoods_.begin();
	    it != likelihoods_.end(); ++it ) {
	delete (*it);
      }
      for ( std::vector<daughterModelType*>::iterator it = daughters_.begin();
	    it != daughters_.end(); ++it ) {
	delete (*it);
      }
    }
    void beginCandidate(const NSVfitResonanceHypothesis* hypothesis)
    {
      for ( std::vector<NSVfitResonanceLikelihood*>::const_iterator likelihood = likelihoods_.begin();
	    likelihood != likelihoods_.end(); ++likelihood ) {
	(*likelihood)->beginCandidate(hypothesis);
      }
      const std::vector<NSVfitSingleParticleHypothesisBase*> daughterHypotheses = hypothesis->daughters();
      assert(daughterHypotheses.size() == numDaughters_);
      for ( unsigned iDaughter = 0; iDaughter < numDaughters_; ++iDaughter ) {
	daughters_[iDaughter]->beginCandidate(daughterHypotheses[iDaughter]);
      }
    }
    double nll(NSVfitResonanceHypothesis* hypothesis) const
    {
      double retVal = 0.;
      for ( std::vector<NSVfitResonanceLikelihood*>::const_iterator likelihood = likelihoods_.begin();
	    likelihood != likelihoods_.end(); ++likelihood ) {
	retVal += (**likelihood)(hypothesis);
      }
      const std::vector<NSVfitSingleParticleHypothesisBase*> daughterHypotheses = hypothesis->daughters();
      assert(daughterHypotheses.size() == numDaughters_);
      for ( unsigned iDaughter = 0; iDaughter < numDaughters_; ++iDaughter ) {
	retVal += daughters_[iDaughter]->nll(daughterHypotheses[iDaughter]);
      }
      return retVal;
    }
    std::string resonanceName_;
    std::vector<NSVfitResonanceLikelihood*> likelihoods_;
    std::vector<daughterModelType*> daughters_;
    unsigned numDaughters_;
  };

  struct eventModelType
  {
    eventModelType(const edm::ParameterSet& cfg, std::vector<NSVfitLikelihoodBase*>& allLikelihoods)
    {
      edm::ParameterSet cfg_builder = cfg.getParameter<edm::ParameterSet>("builder");
      cfg_builder.addParameter<edm::ParameterSet>("resonances", cfg.getParameter<edm::ParameterSet>("resonances"));
      std::string pluginType_builder = cfg_builder.getParameter<std::string>("pluginType");
      builder_ = NSVfitEventBuilderPluginFactory::get()->create(pluginType_builder, cfg_builder);

      typedef std::vector<edm::ParameterSet> vParameterSet;
      vParameterSet cfg_likelihoods = cfg.getParameter<vParameterSet>("likelihoodFunctions");
      for ( vParameterSet::const_iterator cfg_likelihood = cfg_likelihoods.begin();
	    cfg_likelihood != cfg_likelihoods.end(); ++cfg_likelihood ) {
	std::string pluginType = cfg_likelihood->getParameter<std::string>("pluginType");
	NSVfitEventLikelihood* likelihood = 
	  NSVfitEventLikelihoodPluginFactory::get()->create(pluginType, *cfg_likelihood);
	likelihoods_.push_back(likelihood);
	allLikelihoods.push_back(likelihood);
      }

      edm::ParameterSet cfg_resonances = cfg.getParameter<edm::ParameterSet>("resonances");
      typedef std::vector<std::string> vstring;
      vstring resonanceNames = cfg_resonances.getParameterNamesForType<edm::ParameterSet>();
      for ( vstring::const_iterator resonanceName = resonanceNames.begin();
	    resonanceName != resonanceNames.end(); ++resonanceName ) {
        edm::ParameterSet cfg_resonance = cfg_resonances.getParameter<edm::ParameterSet>(*resonanceName);
	resonances_.push_back(new resonanceModelType(*resonanceName, cfg_resonance, allLikelihoods));
      }
      numResonances_ = resonances_.size();
    }
    ~eventModelType()
    {
      delete builder_;
      for ( std::vector<NSVfitEventLikelihood*>::iterator it = likelihoods_.begin();
	    it != likelihoods_.end(); ++it ) {
	delete (*it);
      }
      for ( std::vector<resonanceModelType*>::iterator it = resonances_.begin();
	    it != resonances_.end(); ++it ) {
	delete (*it);
      }
    }
    void beginCandidate(const NSVfitEventHypothesis* hypothesis)
    {
      for ( std::vector<NSVfitEventLikelihood*>::const_iterator likelihood = likelihoods_.begin();
	    likelihood != likelihoods_.end(); ++likelihood ) {
	(*likelihood)->beginCandidate(hypothesis);
      }
      const std::vector<NSVfitResonanceHypothesis*> resonanceHypotheses = hypothesis->resonances();
      assert(resonanceHypotheses.size() == numResonances_);
      for ( unsigned iResonance = 0; iResonance < numResonances_; ++iResonance ) {
	resonances_[iResonance]->beginCandidate(resonanceHypotheses[iResonance]);
      }
    }
    double nll(NSVfitEventHypothesis* hypothesis) const
    {
      double retVal = 0.;
      for ( std::vector<NSVfitEventLikelihood*>::const_iterator likelihood = likelihoods_.begin();
	    likelihood != likelihoods_.end(); ++likelihood ) {
	retVal += (**likelihood)(hypothesis);
      }
      const std::vector<NSVfitResonanceHypothesis*> resonanceHypotheses = hypothesis->resonances();
      assert(resonanceHypotheses.size() == numResonances_);
      for ( unsigned iResonance = 0; iResonance < numResonances_; ++iResonance ) {
	retVal += resonances_[iResonance]->nll(resonanceHypotheses[iResonance]);
      }
      return retVal;
    }
    NSVfitEventBuilderBase* builder_;
    std::vector<NSVfitEventLikelihood*> likelihoods_;
    std::vector<resonanceModelType*> resonances_;
    unsigned numResonances_;
  };

  eventModelType* eventModel_;

  std::vector<NSVfitLikelihoodBase*> allLikelihoods_; // list of all event, resonance and single particle likelihood plugins
                                                      // NOTE: plugins in list are **not** owned by NSVfitAlgorithmBase

  mutable NSVfitEventHypothesis* currentEventHypothesis_;

  std::vector<fitParameterType> fitParameters_;
  int fitParameterCounter_;

  int verbosity_;

  typedef std::pair<double, double> pdouble;
  static std::vector<pdouble> fitParameterLimits_;
};

#include "FWCore/PluginManager/interface/PluginFactory.h"

typedef edmplugin::PluginFactory<NSVfitAlgorithmBase* (const edm::ParameterSet&)> NSVfitAlgorithmPluginFactory;

#endif
