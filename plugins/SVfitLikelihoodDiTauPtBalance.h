#ifndef TauAnalysis_CandidateTools_SVfitLikelihoodDiTauPtBalance_h
#define TauAnalysis_CandidateTools_SVfitLikelihoodDiTauPtBalance_h

/** \class SVfitLikelihoodDiTauPtBalance
 *
 * Plugin for computing likelihood for two tau leptons
 * to balance each other in transverse momentum
 *
 * \author Evan Friis, Christian Veelken; UC Davis
 *
 * \version $Revision: 1.5 $
 *
 * $Id: SVfitLikelihoodDiTauPtBalance.h,v 1.5 2011/01/18 16:47:16 friis Exp $
 *
 */

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "TauAnalysis/CandidateTools/interface/SVfitDiTauLikelihoodBase.h"
#include "TauAnalysis/CandidateTools/interface/SVfitLegLikelihoodBase.h"

#include "AnalysisDataFormats/TauAnalysis/interface/CompositePtrCandidateT1T2MEt.h"
#include "AnalysisDataFormats/TauAnalysis/interface/SVfitDiTauSolution.h"

template <typename T1, typename T2>
class SVfitLikelihoodDiTauPtBalance : public SVfitDiTauLikelihoodBase<T1,T2>
{
 public:
  SVfitLikelihoodDiTauPtBalance(const edm::ParameterSet&);
  ~SVfitLikelihoodDiTauPtBalance();

  double operator()(const CompositePtrCandidateT1T2MEt<T1,T2>&, const SVfitDiTauSolution&) const;
};

#endif
