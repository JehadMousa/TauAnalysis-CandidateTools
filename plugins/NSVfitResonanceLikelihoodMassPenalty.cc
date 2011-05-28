#include "TauAnalysis/CandidateTools/plugins/NSVfitResonanceLikelihoodMassPenalty.h"
#include "TMath.h"

NSVfitResonanceLikelihoodMassPenalty::NSVfitResonanceLikelihoodMassPenalty(
    const edm::ParameterSet& pset):NSVfitResonanceLikelihood(pset) {
  penaltyFactor_ = pset.getParameter<double>("penaltyFactor");
}

double NSVfitResonanceLikelihoodMassPenalty::operator()(
    const NSVfitResonanceHypothesis* resonance) const {
  assert(resonance);
  double mass = resonance->p4_fitted().mass();
  return penaltyFactor_*TMath::Log(mass);
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_EDM_PLUGIN(NSVfitResonanceLikelihoodPluginFactory,
    NSVfitResonanceLikelihoodMassPenalty, "NSVfitResonanceLikelihoodMassPenalty");
