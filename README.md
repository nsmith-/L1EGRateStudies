For using this with `L1TrackTriggerObjects62X`, one can in principle follow this:

https://twiki.cern.ch/twiki/bin/viewauth/CMS/L1TrackTriggerObjects62X#Recipe_in_6_2_0_SLHC12

Preferably, one should do this:
```bash
cmsrel CMSSW_6_2_0_SLHC12
cd CMSSW_6_2_0_SLHC12/src
cmsenv
git cms-init
echo /RecoHI/HiJetAlgos/ >> .git/info/sparse-checkout
echo /SLHCUpgradeSimulations/L1CaloTrigger/ >> .git/info/sparse-checkout
echo /SLHCUpgradeSimulations/L1TrackTrigger/  >> .git/info/sparse-checkout
echo /SimDataFormats/SLHC/  >> .git/info/sparse-checkout
echo /DataFormats/L1DTPlusTrackTrigger/ >> .git/info/sparse-checkout
echo /DataFormats/L1TrackTrigger/ >> .git/info/sparse-checkout
echo /DataFormats/L1Trigger/ >> .git/info/sparse-checkout
git remote add ep git@github.com:EmanuelPerez/cmssw.git
git fetch ep
git checkout TTI_62X_TrackTriggerObjects
git read-tree -mu HEAD
cd SLHCUpgradeSimulations
```
Then, assuming all is well in your enviroment, checkout this repository:

`git clone https://github.com/nsmith-/L1EGRateStudies.git`

Some example run configurations are in `test/`
