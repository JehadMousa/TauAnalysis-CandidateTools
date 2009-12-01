#ifndef TauAnalysis_CandidateTools_candidateAuxFunctions_h
#define TauAnalysis_CandidateTools_candidateAuxFunctions_h

#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

const reco::GenParticle* findGenParticle(const reco::Candidate::LorentzVector&, 
					 const reco::GenParticleCollection&, double = 0.5, int = -1);

void findDaughters(const reco::GenParticle*, std::vector<const reco::GenParticle*>&, int = -1);

reco::Candidate::LorentzVector getVisMomentum(const std::vector<const reco::GenParticle*>&, int = -1);

#endif
