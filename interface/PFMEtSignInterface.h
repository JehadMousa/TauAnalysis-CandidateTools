#ifndef TauAnalysis_CandidateTools_PFMEtSignInterface_h
#define TauAnalysis_CandidateTools_PFMEtSignInterface_h

/** \class PFMEtSignInterface
 *
 * Auxiliary class interfacing CompositePtrCandidateT1T2MEtAlgorithm.h 
 * to the algorithm for computing (PF)MEt significance
 *  RecoMET/METAlgorithms/interface/significanceAlgo.h 
 * (see CMS AN-10/400 for description of the (PF)MEt significance computation)
 *
 * \author Christian Veelken, UC Davis
 *
 * \version $Revision: 1.5 $
 *
 * $Id: PFMEtSignInterface.h,v 1.5 2012/09/01 08:45:44 veelken Exp $
 *
 */

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/JetReco/interface/PFJetCollection.h"

#include "JetMETCorrections/METPUSubtraction/interface/PFMEtSignInterfaceBase.h"

#include <TMatrixD.h>

#include <list>

class PFMEtSignInterface : public PFMEtSignInterfaceBase
{
 public:

  PFMEtSignInterface(const edm::ParameterSet&);
  ~PFMEtSignInterface();

  void beginEvent(const edm::Event&, const edm::EventSetup&);

  TMatrixD operator()(const std::list<const reco::Candidate*>&) const; 

 private:

  edm::InputTag srcPFJets_;
  edm::InputTag srcPFCandidates_;

  std::list<const reco::PFJet*> pfJetList_;
  std::list<const reco::PFCandidate*> pfCandidateList_;

  double dRoverlapPFJet_;
  double dRoverlapPFCandidate_;

  int verbosity_;
};

#endif
