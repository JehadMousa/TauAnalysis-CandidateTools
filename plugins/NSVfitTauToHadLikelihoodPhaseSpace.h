#ifndef TauAnalysis_CandidateTools_NSVfitTauToHadLikelihoodPhaseSpace_h
#define TauAnalysis_CandidateTools_NSVfitTauToHadLikelihoodPhaseSpace_h

/** \class NSVfitTauToLeptonLikelihoodPhaseSpace
 *
 * Plugin to compute likelihood for system of hadrons to be compatible 
 * with tau --> tau-jet + nu two-body decay,
 * assuming constant matrix element, so that energy and angular distribution 
 * of decay products are solely determined by phase-space
 * 
 * \author Evan Friis, Christian Veelken; UC Davis
 *
 * \version $Revision: 1.9 $
 *
 * $Id: NSVfitTauToHadLikelihoodPhaseSpace.h,v 1.9 2012/03/22 09:37:13 veelken Exp $
 *
 */

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "TauAnalysis/CandidateTools/interface/NSVfitSingleParticleLikelihood.h"

#include "AnalysisDataFormats/TauAnalysis/interface/NSVfitSingleParticleHypothesisBase.h"

class NSVfitTauToHadLikelihoodPhaseSpace : public NSVfitSingleParticleLikelihood
{
 public:
  NSVfitTauToHadLikelihoodPhaseSpace(const edm::ParameterSet&);
  ~NSVfitTauToHadLikelihoodPhaseSpace();

  void beginJob(NSVfitAlgorithmBase*);

  double operator()(const NSVfitSingleParticleHypothesis*, int) const;

 private:
  bool applySinThetaFactor_; 

  bool applyVisMassFactor_;
  TH1* histogram_;
  int firstBin_;
  int lastBin_;
};

#endif
