import FWCore.ParameterSet.Config as cms
import copy

from TauAnalysis.RecoTools.pftauPatSelector_cfi import *

# require muon and tau-jet to be separated in eta-phi,
# in order to ensure that both do not refer to one and the same physical particle
# (NOTE: cut is already applied during skimming,
#        so should not reject any events)
selectedMuTauPairsAntiOverlapVeto = cms.EDFilter("PATMuTauPairSelector",
     src = cms.InputTag("allMuTauPairs"),
     cut = cms.string('dR12 > 0.7'),
     filter = cms.bool(False)
)

# require muon and tau not to be back-to-back
selectedMuTauPairsAcoplanarityIndividual = cms.EDFilter("PATMuTauPairSelector",
     src = selectedMuTauPairsAntiOverlapVeto.src,
     cut = cms.string('dPhi12 < 2.4'),
     filter = cms.bool(False)
)

selectedMuTauPairsAcoplanarityCumulative = copy.deepcopy(selectedMuTauPairsAcoplanarityIndividual)
selectedMuTauPairsAcoplanarityCumulative.src = cms.InputTag("selectedMuTauPairsAntiOverlapVeto")

# require muon and tau to form a zero-charge pair
selectedMuTauPairsZeroChargeIndividual = cms.EDFilter("PATMuTauPairSelector",
     src = selectedMuTauPairsAntiOverlapVeto.src,
     cut = cms.string('charge = 0'),
     filter = cms.bool(False)
)

selectedMuTauPairsZeroChargeCumulative = copy.deepcopy(selectedMuTauPairsZeroChargeIndividual)
selectedMuTauPairsZeroChargeCumulative.src = cms.InputTag("selectedMuTauPairsAcoplanarityCumulative")

selectMuTauPairs = cms.Sequence( selectedMuTauPairsAntiOverlapVeto
                                *selectedMuTauPairsAcoplanarityIndividual
                                *selectedMuTauPairsAcoplanarityCumulative
                                *selectedMuTauPairsZeroChargeIndividual
                                *selectedMuTauPairsZeroChargeCumulative )
