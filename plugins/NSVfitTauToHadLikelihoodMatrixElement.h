#ifndef TauAnalysis_CandidateTools_NSVfitTauToHadLikelihoodMatrixElement_h
#define TauAnalysis_CandidateTools_NSVfitTauToHadLikelihoodMatrixElement_h

/** \class NSVfitTauToHadLikelihoodMatrixElement
 *
 * Plugin for computing likelihood for tau lepton decay 
 *  tau- --> X nu
 * to be compatible with matrix element of V-A electroweak decay
 *
 * NOTE: the system of hadrons X can either be pi-, rho- --> pi- pi0,
 *       a1- --> pi- pi0 pi0 or a1- --> pi- pi+ pi-;
 *       tau decays into pi- pi+ pi- pi0 are **not** supported
 *
 * \author Christian Veelken, Lorenzo Bianchini; LLR
 *
 * \version $Revision: 1.1 $
 *
 * $Id: NSVfitTauToHadLikelihoodMatrixElement.h,v 1.1 2012/03/22 11:08:14 veelken Exp $
 *
 */

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "TauAnalysis/CandidateTools/interface/NSVfitSingleParticleLikelihood.h"

#include "AnalysisDataFormats/TauAnalysis/interface/NSVfitSingleParticleHypothesisBase.h"

#include "FWCore/ParameterSet/interface/FileInPath.h"

#include <TFile.h>
#include <TGraph.h>

class NSVfitTauToHadLikelihoodMatrixElement : public NSVfitSingleParticleLikelihood
{
 public:
  NSVfitTauToHadLikelihoodMatrixElement(const edm::ParameterSet&);
  ~NSVfitTauToHadLikelihoodMatrixElement();

  void beginJob(NSVfitAlgorithmBase*);

  double operator()(const NSVfitSingleParticleHypothesis*, int) const;

 private:
  bool applySinThetaFactor_; 

  edm::FileInPath inputFileName_;
  TFile* inputFile_;
  TGraph* rhoLPlus_;
  TGraph* rhoNormLPlus_; 
  TGraph* rhoLMinus_;
  TGraph* rhoNormLMinus_; 
  TGraph* rhoTPlus_;
  TGraph* rhoNormTPlus_;
  TGraph* rhoTMinus_;
  TGraph* rhoNormTMinus_;

  TGraph* a1LPlus_;
  TGraph* a1NormLPlus_; 
  TGraph* a1LMinus_;
  TGraph* a1NormLMinus_; 
  TGraph* a1TPlus_;
  TGraph* a1NormTPlus_;
  TGraph* a1TMinus_;
  TGraph* a1NormTMinus_;

  TGraph* a1Lz_;
  TGraph* a1Tz_;

  TGraph* rhoyMinus_;
  TGraph* rhoyNormMinus_;
  TGraph* rhoyPlus_;
  TGraph* rhoyNormPlus_;
  TGraph* a1yMinus_;
  TGraph* a1yNormMinus_;
  TGraph* a1yPlus_;
  TGraph* a1yNormPlus_;


};

#endif
