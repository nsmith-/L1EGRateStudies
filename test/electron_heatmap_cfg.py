import FWCore.ParameterSet.Config as cms

process = cms.Process("test")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.categories = cms.untracked.vstring('L1EGRateStudies', 'FwkReport')
process.MessageLogger.cerr.FwkReport = cms.untracked.PSet(
   reportEvery = cms.untracked.int32(10)
)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.source = cms.Source("PoolSource",
   fileNames = cms.untracked.vstring("/store/user/ncsmith/L1TrackTriggerMC/m1_SingleElectron_E2023TTI_PU140.root")
)

# All this stuff just runs the various EG algorithms that we are studying

# ---- Global Tag :
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'PH2_1K_FB_V3::All', '')

process.load('Configuration.Geometry.GeometryExtended2023TTIReco_cff')

process.load('Configuration/StandardSequences/L1HwVal_cff')
process.load('Configuration.StandardSequences.RawToDigi_cff')
process.load("SLHCUpgradeSimulations.L1CaloTrigger.SLHCCaloTrigger_cff")

# bug fix for missing HCAL TPs in MC RAW
from SimCalorimetry.HcalTrigPrimProducers.hcaltpdigi_cff import HcalTPGCoderULUT
HcalTPGCoderULUT.LUTGenerationMode = cms.bool(True)
process.valRctDigis.hcalDigis             = cms.VInputTag(cms.InputTag('valHcalTriggerPrimitiveDigis'))
process.L1CaloTowerProducer.HCALDigis =  cms.InputTag("valHcalTriggerPrimitiveDigis")

process.slhccalo = cms.Path( process.RawToDigi + process.valHcalTriggerPrimitiveDigis+process.SLHCCaloTrigger)

# run L1Reco to produce the L1EG objects corresponding
# to the current trigger
#process.load('Configuration.StandardSequences.L1Reco_cff')
#process.L1Reco = cms.Path( process.l1extraParticles )

# --------------------------------------------------------------------------------------------
#
# ----    Produce the L1EGCrystal clusters (code of Sasha Savin)

# first you need the ECAL RecHIts :
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.reconstruction_step = cms.Path( process.calolocalreco )

process.L1EGammaCrystalsProducer = cms.EDProducer("L1EGCrystalClusterProducer",
   EtminForStore = cms.double(0.),
   useECalEndcap = cms.bool(False)
)
process.pSasha = cms.Path( process.L1EGammaCrystalsProducer )

# --------------------------------------------------------------------------------------------
#
# ----  Match the L1EG stage-2 objects created by the SLHCCaloTrigger sequence above
#	with the crystal-level clusters.
#	This produces a new collection of L1EG objects, starting from the original
#	L1EG collection. The eta and phi of the L1EG objects is corrected using the
#	information of the xtal level clusters.

process.l1ExtraCrystalProducer = cms.EDProducer("L1ExtraCrystalPosition",
   eGammaSrc = cms.InputTag("SLHCL1ExtraParticles","EGamma"),
   eClusterSrc = cms.InputTag("L1EGammaCrystalsProducer","EGCrystalCluster")
)
process.egcrystal_producer = cms.Path(process.l1ExtraCrystalProducer)


# ----------------------------------------------------------------------------------------------
# 
# Do offline reconstruction step to get cluster pt

process.load('RecoEcal.Configuration.RecoEcal_cff')
process.ecalClusters = cms.Path(process.ecalClustersNoPFBox)


# ----------------------------------------------------------------------------------------------
# 
# Analyzer starts here

process.analyzer = cms.EDAnalyzer('L1EGCrystalsHeatMap',
   L1CrystalClustersInputTag = cms.InputTag("L1EGammaCrystalsProducer","EGCrystalCluster"),
   L1EGammaOtherAlgs = cms.VInputTag(
   ),
   debug = cms.untracked.bool(False),
   useOfflineClusters = cms.untracked.bool(False),
   range = cms.untracked.int32(20),
   clusterPtCut = cms.untracked.double(15.)
)

process.panalyzer = cms.Path(process.analyzer)

process.TFileService = cms.Service("TFileService", 
   fileName = cms.string("electronHeatmap.root"), 
   closeFileFast = cms.untracked.bool(True)
)
