#include "TauAnalysis/CandidateTools/plugins/SVfitLegLikelihoodPhaseSpace.h"

#include "TauAnalysis/CandidateTools/interface/svFitAuxFunctions.h"

#include "DataFormats/Candidate/interface/Candidate.h"

#include <TMath.h>

using namespace SVfit_namespace;

template <typename T>
SVfitLegLikelihoodPhaseSpace<T>::SVfitLegLikelihoodPhaseSpace(const edm::ParameterSet& cfg)
  : SVfitLegLikelihoodBase<T>(cfg)
{
// nothing to be done yet...
}

template <typename T>
SVfitLegLikelihoodPhaseSpace<T>::~SVfitLegLikelihoodPhaseSpace()
{
// nothing to be done yet...
}

template <typename T>
double SVfitLegLikelihoodPhaseSpace<T>::operator()(const T& leg, const SVfitLegSolution& solution) const
{
//--- compute negative log-likelihood for tau lepton decay "leg"
//    to be compatible with three-body decay,
//    assuming constant matrix element, 
//    so that energy and angular distribution of decay products is solely determined by phase-space
//
//    NOTE: the parametrization of the three-body decay phase-space is taken from the PDG:
//          K. Nakamura et al. (Particle Data Group), J. Phys. G 37, 075021 (2010);
//          formulas 38.20a, 38.20b
//
  reco::Candidate::LorentzVector legP4 = solution.p4();
  
  double EVAN_PLEASE_IMPLEMENT = -1.; // FIXME !!

  double thetaRestFrame = EVAN_PLEASE_IMPLEMENT;
  double nuMass = solution.p4InvisRestFrame().mass();
  double visMass = solution.p4VisRestFrame().mass();

  double logLikelihood = TMath::Log(TMath::Sin(thetaRestFrame));
  if ( nuMass > 0. ) {
    double logP1 = TMath::Log(nuMass) - TMath::Log(2.);
    double logP3 = 0.5*TMath::Log((tauLeptonMass2 - square(nuMass + visMass))*(tauLeptonMass2 - square(nuMass - visMass)))
                  - TMath::Log(2*tauLeptonMass);
    logLikelihood += (logP1 + logP3);
  }

  return -logLikelihood;
}

#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/Candidate/interface/Candidate.h"

typedef SVfitLegLikelihoodPhaseSpace<pat::Electron> SVfitElectronLikelihoodPhaseSpace;
typedef SVfitLegLikelihoodPhaseSpace<pat::Muon> SVfitMuonLikelihoodPhaseSpace;
typedef SVfitLegLikelihoodPhaseSpace<pat::Tau> SVfitTauLikelihoodPhaseSpace;
typedef SVfitLegLikelihoodPhaseSpace<reco::Candidate> SVfitCandidateLikelihoodPhaseSpace;

#include "FWCore/Framework/interface/MakerMacros.h"

DEFINE_EDM_PLUGIN(SVfitElectronLikelihoodBasePluginFactory, SVfitElectronLikelihoodPhaseSpace, "SVfitElectronLikelihoodPhaseSpace");
DEFINE_EDM_PLUGIN(SVfitMuonLikelihoodBasePluginFactory, SVfitMuonLikelihoodPhaseSpace, "SVfitMuonLikelihoodPhaseSpace");
DEFINE_EDM_PLUGIN(SVfitTauLikelihoodBasePluginFactory, SVfitTauLikelihoodPhaseSpace, "SVfitTauLikelihoodPhaseSpace");
DEFINE_EDM_PLUGIN(SVfitCandidateLikelihoodBasePluginFactory, SVfitCandidateLikelihoodPhaseSpace, "SVfitCandidateLikelihoodPhaseSpace");