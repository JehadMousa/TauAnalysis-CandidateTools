#ifndef TauAnalysis_CandidateTools_SVfitLikelihoodDiTauMEt_h
#define TauAnalysis_CandidateTools_SVfitLikelihoodDiTauMEt_h

/** \class SVfitLikelihoodDiTauMEt
 *
 * Plugin for computing likelihood for neutrinos produced in tau lepton decays
 * to match missing transverse momentum reconstructed in the event
 *
 * \author Evan Friis, Christian Veelken; UC Davis
 *
 * \version $Revision: 1.8 $
 *
 * $Id: SVfitLikelihoodDiTauMEt.h,v 1.8 2011/01/18 16:47:16 friis Exp $
 *
 */

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/Common/interface/Handle.h"

#include "TauAnalysis/CandidateTools/interface/SVfitDiTauLikelihoodBase.h"
#include "TauAnalysis/CandidateTools/interface/SVfitLegLikelihoodBase.h"

#include "AnalysisDataFormats/TauAnalysis/interface/CompositePtrCandidateT1T2MEt.h"
#include "AnalysisDataFormats/TauAnalysis/interface/SVfitDiTauSolution.h"

#include <TFormula.h>

template <typename T1, typename T2>
class SVfitLikelihoodDiTauMEt : public SVfitDiTauLikelihoodBase<T1,T2>
{
 public:
  SVfitLikelihoodDiTauMEt(const edm::ParameterSet&);
  ~SVfitLikelihoodDiTauMEt();

  void beginEvent(const edm::Event&, const edm::EventSetup&);
  void beginCandidate(const CompositePtrCandidateT1T2MEt<T1,T2>&);

  bool isFittedParameter(int) const;

  double operator()(const CompositePtrCandidateT1T2MEt<T1,T2>&, const SVfitDiTauSolution&) const;
 private:
  TFormula* parSigma_;
  TFormula* parBias_;

  TFormula* perpSigma_;
  TFormula* perpBias_;

  double qX_;
  double qY_;
  double qT_;

  // Whether or not to allow the phi variable to vary
  bool varyPhi_;

  edm::InputTag srcPFCandidates_;
  edm::Handle<reco::PFCandidateCollection> pfCandidates_;
};

#endif
