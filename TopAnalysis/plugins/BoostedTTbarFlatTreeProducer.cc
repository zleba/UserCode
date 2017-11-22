#include <string>
#include <cmath>
#include <functional>
#include <vector>
#include <cassert>
#include "Math/SpecFuncMathMore.h"
#include "TMath.h"
#include "TVectorD.h"
#include "TMatrixDSym.h"
#include "TMatrixDSymEigen.h"
#include "fastjet/contrib/Njettiness.hh"
#include "fastjet/tools/MassDropTagger.hh"
#include "fastjet/contrib/SoftDrop.hh"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "JetMETCorrections/Objects/interface/JetCorrectionsRecord.h"
#include <cassert>

#include "KKousour/TopAnalysis/plugins/BoostedTTbarFlatTreeProducer.h"

using namespace std;
using namespace reco;
using namespace fastjet;

BoostedTTbarFlatTreeProducer::BoostedTTbarFlatTreeProducer(edm::ParameterSet const& cfg) : p(cfg, consumesCollector())
{ }


//////////////////////////////////////////////////////////////////////////////////////////
void BoostedTTbarFlatTreeProducer::beginJob() 
{
  //--- book the trigger histograms ---------
  triggerNamesHisto_ = fs_->make<TH1F>("TriggerNames","TriggerNames",1,0,1);
  triggerNamesHisto_->SetCanExtend(TH1::kAllAxes);
  for(unsigned i=0;i<p.triggerNames_.size();i++) {
    triggerNamesHisto_->Fill(p.triggerNames_[i].c_str(),1);
  }
  triggerPassHisto_ = fs_->make<TH1F>("TriggerPass","TriggerPass",1,0,1);
  triggerPassHisto_->SetCanExtend(TH1::kAllAxes);

  cutFlowHisto_ = fs_->make<TH1F>("CutFlow","CutFlow",1,0,1);
  cutFlowHisto_->SetCanExtend(TH1::kAllAxes);
 
  //--- book the tree ----------------------------------
  outTree_ = fs_->make<TTree>("events","events");
  outTree_->Branch("runNo"                ,&run_               ,"run_/I");
  outTree_->Branch("evtNo"                ,&evt_               ,"evt_/I");
  outTree_->Branch("lumi"                 ,&lumi_              ,"lumi_/I");
  outTree_->Branch("nvtx"                 ,&nVtx_              ,"nVtx_/I");
  outTree_->Branch("nJets"                ,&nJets_             ,"nJets_/I");
  outTree_->Branch("pvRho"                ,&pvRho_             ,"pvRho_/F");
  outTree_->Branch("pvz"                  ,&pvz_               ,"pvz_/F");
  outTree_->Branch("pvchi2"               ,&pvchi2_            ,"pvchi2_/F");
  outTree_->Branch("pvndof"               ,&pvndof_            ,"pvndof_/F");
  outTree_->Branch("rho"                  ,&rho_               ,"rho_/F");
  outTree_->Branch("ht"                   ,&ht_                ,"ht_/F");
  //------------------------------------------------------------------
  flavor_         = new std::vector<int>;
  flavorHadron_   = new std::vector<int>;
  pt_             = new std::vector<float>;
  unc_            = new std::vector<float>;
  eta_            = new std::vector<float>;
  phi_            = new std::vector<float>;
  mass_           = new std::vector<float>;
  energy_         = new std::vector<float>;
  chf_            = new std::vector<float>;
  nhf_            = new std::vector<float>;
  phf_            = new std::vector<float>;
  muf_            = new std::vector<float>;
  elf_            = new std::vector<float>;
  chm_            = new std::vector<int>;
  nhm_            = new std::vector<int>;
  phm_            = new std::vector<int>;
  elm_            = new std::vector<int>;
  mum_            = new std::vector<int>;
  outTree_->Branch("jetPt"                ,"vector<float>"     ,&pt_);
  outTree_->Branch("jetUnc"               ,"vector<float>"     ,&unc_);
  outTree_->Branch("jetEta"               ,"vector<float>"     ,&eta_);
  outTree_->Branch("jetPhi"               ,"vector<float>"     ,&phi_);
  outTree_->Branch("jetMass"              ,"vector<float>"     ,&mass_);
  outTree_->Branch("jetEnergy"            ,"vector<float>"     ,&energy_);
  outTree_->Branch("jetChf"               ,"vector<float>"     ,&chf_);
  outTree_->Branch("jetNhf"               ,"vector<float>"     ,&nhf_);
  outTree_->Branch("jetPhf"               ,"vector<float>"     ,&phf_);
  outTree_->Branch("jetMuf"               ,"vector<float>"     ,&muf_);
  outTree_->Branch("jetElf"               ,"vector<float>"     ,&elf_);
  outTree_->Branch("jetChm"               ,"vector<int>"       ,&chm_);
  outTree_->Branch("jetNhm"               ,"vector<int>"       ,&nhm_);
  outTree_->Branch("jetPhm"               ,"vector<int>"       ,&phm_);
  outTree_->Branch("jetElm"               ,"vector<int>"       ,&elm_);
  outTree_->Branch("jetMum"               ,"vector<int>"       ,&mum_);
  //------------------------------------------------------------------
  triggerBit_ = new std::vector<bool>;
  triggerPre_ = new std::vector<int>;
  outTree_->Branch("triggerBit"           ,"vector<bool>"      ,&triggerBit_);
  outTree_->Branch("triggerPre"           ,"vector<int>"       ,&triggerPre_);
  cout<<"Begin job finished"<<endl;
}
//////////////////////////////////////////////////////////////////////////////////////////
void BoostedTTbarFlatTreeProducer::endJob() 
{  
  delete flavor_;
  delete flavorHadron_;
  delete pt_;
  delete unc_;
  delete eta_;
  delete phi_;
  delete mass_;
  delete energy_;
  delete chf_;
  delete nhf_;
  delete phf_;
  delete muf_;
  delete elf_;
  delete chm_;
  delete nhm_;
  delete phm_;
  delete elm_;
  delete mum_;
}
//////////////////////////////////////////////////////////////////////////////////////////
void BoostedTTbarFlatTreeProducer::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) 
{
  if (p.isMC_ && p.debug_) {
    iRun.getByToken(p.runInfoToken,runInfo);
    for(vector<LHERunInfoProduct::Header>::const_iterator it = runInfo->headers_begin();it != runInfo->headers_end(); it++) {
      cout<<it->tag()<<endl;
      vector<string> lines = it->lines();
      for(unsigned int iLine = 0; iLine < lines.size(); iLine++) {
        cout<< lines.at(iLine);
      }
    }
  }
}
//////////////////////////////////////////////////////////////////////////////////////////
void BoostedTTbarFlatTreeProducer::endRun(edm::Run const& iRun, edm::EventSetup const& iSetup) 
{
}
//////////////////////////////////////////////////////////////////////////////////////////
bool BoostedTTbarFlatTreeProducer::isGoodJet(const pat::Jet &jet)
{
  bool res  = true; // by default is good, unless fails a cut bellow
/*  float chf = jet.chargedHadronEnergyFraction();
  float nhf = jet.neutralHadronEnergyFraction();
  float phf = jet.photonEnergyFraction();
  float muf = jet.muonEnergyFraction();
  float elf = jet.electronEnergyFraction();
  int chm   = jet.chargedHadronMultiplicity();
  int npr   = jet.neutralMultiplicity()+jet.chargedMultiplicity();
  float eta = fabs(jet.eta());
  float pt  = jet.pt();
  bool idL  = (npr>1 && phf<0.99 && nhf<0.99);
  //bool idM = (idL && ((eta<=2.4 && nhf<0.9 && phf<0.9 && elf<0.99 && muf<0.99 && chf>0 && chm>0) || eta>2.4));
  bool idT = (idL && ((eta<=2.4 && nhf<0.9 && phf<0.9 && elf<0.9 && muf<0.9 && chf>0 && chm>0) || eta>2.4));
  //if (!idT) res = false;
  if (pt < ptMin_) res = false;
  if (eta > etaMax_) res = false;
  cout << "Jet Parameters "<< idT << " "<< pt <<" "<< eta << endl;
  //if (jet.userFloat("ak8PFJetsCHSSoftDropMass") < massMin_) res = false;
  //if ((jet.subjets("SoftDrop")).size() < 2) res = false;*/
  return res;
}
//////////////////////////////////////////////////////////////////////////////////////////
void BoostedTTbarFlatTreeProducer::analyze(edm::Event const& iEvent, edm::EventSetup const& iSetup) 
{
  initialize();

  if(p.isPrint_) cout<<"**** EVENT ****"<<endl;

  iEvent.getByToken(p.jetsToken,jets);
  iEvent.getByToken(p.candsToken,cands);
  iEvent.getByToken(p.rhoToken,rho);
  iEvent.getByToken(p.recVtxsToken,recVtxs);  
  iEvent.getByToken(p.triggerResultsToken,triggerResults);  
  iEvent.getByToken(p.triggerPrescalesToken,triggerPrescales); 
  iEvent.getByToken(p.triggerObjects_, triggerObjects);

  triggerBit_->clear();
  triggerPre_->clear();

  //-------------- Trigger Info -----------------------------------
  const edm::TriggerNames &names = iEvent.triggerNames(*triggerResults);  

  //assert(triggerObjects-);
  //cout << "TrigArr val " <<  triggerObjects << endl;
  //cout << "Trig size " << triggerObjects->size() << endl;
  //for (pat::TriggerObjectStandAlone obj : *triggerObjects) { // note: not "const &" since we want to call unpackPathNames
      //cout << "hi Radek" << endl;
      //obj.unpackPathNames(names);

      //std::vector<std::string> pathNamesAll  = obj.pathNames(false);
      //std::vector<std::string> pathNamesLast = obj.pathNames(true);

      //cout << "RADEK " << obj.pt() <<" "<< pathNamesAll.size() << " "<< pathNamesLast.size() << endl;
      //TLorentzVector P4;                                                                                                                                         
      //P4.SetPtEtaPhiM(obj.pt(),obj.eta(),obj.phi(),obj.mass());                                                                                                      
      //LorentzVector qcdhltobj(P4.Px(),P4.Py(),P4.Pz(),P4.E());             
  //}


  triggerPassHisto_->Fill("totalEvents",1);
  bool passTrigger(false);
  for(unsigned int k=0; k<p.triggerNames_.size(); ++k) {
    bool bit(false);
    int pre(1);
    for(unsigned int itrig=0; itrig<triggerResults->size(); ++itrig) {
      //cout<<"Trigger name of index "<<itrig<<" "<<string(names.triggerName(itrig))<<endl;
      string trigger_name = string(names.triggerName(itrig));
      //--- erase the last character, i.e. the version number----
      trigger_name.pop_back();
      if (trigger_name == p.triggerNames_[k]) {
        cout << "Puppi found " << trigger_name  << endl;
        bit = true;//triggerResults->accept(itrig);
        pre = triggerPrescales->getPrescaleForIndex(itrig);
        if (bit) {
          triggerPassHisto_->Fill(p.triggerNames_[k].c_str(),1);
        } 
      }
    }
    //--- if at least one monitored trigger has fired passTrigger becomes true
    passTrigger += bit;
    triggerBit_->push_back(bit); 
    triggerPre_->push_back(pre);   
  }   
  //----- at least one good vertex -----------
  bool cut_vtx = (recVtxs->size() > 0);
  if (cut_vtx) {
    pvRho_    = (*recVtxs)[0].position().Rho();
    pvz_    = (*recVtxs)[0].z();
    pvndof_ = (*recVtxs)[0].ndof();
    pvchi2_ = (*recVtxs)[0].chi2();
  }// if vtx
  //----- PF jets ------------------------------
  nJets_  = 0;
  nGenJets_  = 0;
  nBJets_ = 0;
  ht_     = 0.0;

  edm::ESHandle<JetCorrectorParametersCollection> PFJetCorParCollCHS;
  iSetup.get<JetCorrectionsRecord>().get("AK4PFchs",PFJetCorParCollCHS);
  JetCorrectorParameters const& PFJetCorParCHS = (*PFJetCorParCollCHS)["Uncertainty"];
  mPFUncCHS = new JetCorrectionUncertainty(PFJetCorParCHS);//"Summer16_23Sep2016V4_MC_Uncertainty_AK8PFchs.txt");

  double unc=0.0;
  
  if(jets->size() > 0)
      cout << "Number of Jets "<< jets->size() <<" "<< jets->begin()->pt() <<  endl;
  for(pat::JetCollection::const_iterator ijet =jets->begin();ijet != jets->end(); ++ijet) {
      if(ijet->pt() < 20) continue;
      //if (isGoodJet(*ijet)) {
          cout << "Good jet " << ijet->pt() << endl;
          flavor_        ->push_back(ijet->partonFlavour());
          flavorHadron_  ->push_back(ijet->hadronFlavour());
          chf_           ->push_back(ijet->chargedHadronEnergyFraction());
          nhf_           ->push_back(ijet->neutralHadronEnergyFraction());
          phf_           ->push_back(ijet->photonEnergyFraction());
          elf_           ->push_back(ijet->electronEnergyFraction());
          muf_           ->push_back(ijet->muonEnergyFraction());
          chm_           ->push_back(ijet->chargedHadronMultiplicity());
          nhm_           ->push_back(ijet->neutralHadronMultiplicity());
          phm_           ->push_back(ijet->photonMultiplicity());
          elm_           ->push_back(ijet->electronMultiplicity());
          mum_           ->push_back(ijet->muonMultiplicity());

          pt_            ->push_back(ijet->pt());
          phi_           ->push_back(ijet->phi());
          eta_           ->push_back(ijet->eta());
          mass_          ->push_back(ijet->mass());
          energy_        ->push_back(ijet->energy());

          mPFUncCHS->setJetEta(ijet->eta());
          mPFUncCHS->setJetPt(ijet->pt()); // here you must use the CORRECTED jet pt
          unc = mPFUncCHS->getUncertainty(true);
          unc_           ->push_back(unc);
          ht_ += ijet->pt();
          ++nJets_;
      //}// if good jet
  }// jet loop       
  rho_    = *rho;
  nVtx_   = recVtxs->size();
  run_    = iEvent.id().run();
  evt_    = iEvent.id().event();
  lumi_   = iEvent.id().luminosityBlock();

  bool cut_RECO = (nJets_ >= 1);  
 
  cutFlowHisto_->Fill("All",1);
  if (iEvent.isRealData()) {
    outTree_->Fill();
    /*
    if (cut_RECO) {
      cutFlowHisto_->Fill("At least one jet",1);
      if (passTrigger || 1) {
        cutFlowHisto_->Fill("Trigger",1);
      }
    }
    */

  } 
  else {
    if (cut_RECO) {
      cutFlowHisto_->Fill("At least one jet (reco || gen)",1);
      outTree_->Fill();
    }
  }

}



//////////////////////////////////////////////////////////////////////////////////////////
void BoostedTTbarFlatTreeProducer::initialize()
{
  run_            = -1;
  evt_            = -1;
  lumi_           = -1;
  nVtx_           = -1;
  nJets_          = -1;
  rho_            = -1;
  pvRho_          = -999;
  pvz_            = -999;
  pvndof_         = -999;
  pvchi2_         = -999;
  flavor_         ->clear();
  flavorHadron_   ->clear();
  pt_             ->clear();
  unc_             ->clear();
  eta_            ->clear();
  phi_            ->clear();
  mass_           ->clear();
  energy_         ->clear();
  chf_            ->clear();
  nhf_            ->clear();
  phf_            ->clear();
  elf_            ->clear();
  muf_            ->clear();
  chm_            ->clear();
  nhm_            ->clear();
  phm_            ->clear();
  elm_            ->clear();
  mum_            ->clear();
}
//////////////////////////////////////////////////////////////////////////////////////////
BoostedTTbarFlatTreeProducer::~BoostedTTbarFlatTreeProducer() 
{
}

DEFINE_FWK_MODULE(BoostedTTbarFlatTreeProducer);
















