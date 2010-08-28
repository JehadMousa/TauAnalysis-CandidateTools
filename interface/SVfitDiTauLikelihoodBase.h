#ifndef TauAnalysis_CandidateTools_SVfitDiTauLikelihoodBase_h
#define TauAnalysis_CandidateTools_SVfitDiTauLikelihoodBase_h

/** \class SVfitDiTauLikelihoodBase
 *
 * Abstract base-class for plugins computing likelihood for tau lepton pair;
 * used by SVfit algorithm
 * 
 * \author Evan Friis, Christian Veelken; UC Davis
 *
 * \version $Revision: 1.3 $
 *
 * $Id: SVfitDiTauLikelihoodBase.h,v 1.3 2010/08/27 12:08:04 veelken Exp $
 *
 */

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "AnalysisDataFormats/TauAnalysis/interface/CompositePtrCandidateT1T2MEt.h"
#include "AnalysisDataFormats/TauAnalysis/interface/SVfitDiTauSolution.h"

#include <string>
#include <iostream>

template <typename T1, typename T2>
class SVfitDiTauLikelihoodBase
{
 public:
  SVfitDiTauLikelihoodBase(const edm::ParameterSet& cfg)
  {
    pluginType_ = cfg.getParameter<std::string>("pluginType");
  }
  virtual ~SVfitDiTauLikelihoodBase() {}

  virtual void print(std::ostream& stream) const
  {
    stream << "<SVfitDiTauLikelihoodBase::print>:" << std::endl;
    stream << " pluginType = " << pluginType_ << std::endl;
  }

  virtual bool isFittedParameter(int) const
  {
    return false;
  }

  virtual bool supportsPolarization() const
  {
    return false;
  }

  virtual double operator()(const CompositePtrCandidateT1T2MEt<T1,T2>&, const SVfitDiTauSolution&) const = 0;
 protected:
  std::string pluginType_;
};

#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Tau.h"

typedef SVfitDiTauLikelihoodBase<pat::Electron, pat::Tau> SVfitElecTauPairLikelihoodBase;
typedef SVfitDiTauLikelihoodBase<pat::Muon, pat::Tau> SVfitMuTauPairLikelihoodBase;
typedef SVfitDiTauLikelihoodBase<pat::Tau, pat::Tau> SVfitDiTauPairLikelihoodBase;
typedef SVfitDiTauLikelihoodBase<pat::Electron, pat::Muon> SVfitElecMuPairLikelihoodBase;

#include "DataFormats/Candidate/interface/Candidate.h"

typedef SVfitDiTauLikelihoodBase<reco::Candidate, reco::Candidate> SVfitDiCandidatePairLikelihoodBase;

#include "FWCore/PluginManager/interface/PluginFactory.h"

typedef edmplugin::PluginFactory<SVfitElecTauPairLikelihoodBase* (const edm::ParameterSet&)> SVfitElecTauPairLikelihoodBasePluginFactory;
typedef edmplugin::PluginFactory<SVfitMuTauPairLikelihoodBase* (const edm::ParameterSet&)> SVfitMuTauPairLikelihoodBasePluginFactory;
typedef edmplugin::PluginFactory<SVfitDiTauPairLikelihoodBase* (const edm::ParameterSet&)> SVfitDiTauPairLikelihoodBasePluginFactory;
typedef edmplugin::PluginFactory<SVfitElecMuPairLikelihoodBase* (const edm::ParameterSet&)> SVfitElecMuPairLikelihoodBasePluginFactory;
typedef edmplugin::PluginFactory<SVfitDiCandidatePairLikelihoodBase* (const edm::ParameterSet&)> SVfitDiCandidatePairLikelihoodBasePluginFactory;

#endif