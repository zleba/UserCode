#define matching_cxx
// The class definition in matching.h has been generated automatically
// by the ROOT utility TTree::MakeSelector(). This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.


// The following methods are defined in this file:
//    Begin():        called every time a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// root> T->Process("matching.C")
// root> T->Process("matching.C","some options")
// root> T->Process("matching.C+")
//


#include "matching.h"
#include <TH2.h>
#include <TStyle.h>
#include "TRandom.h"


//double dist2(double eta1, double phi1, double eta2, double phi2)
double dist2(const ROOT::Math::PtEtaPhiM4D<float> &j1, const ROOT::Math::PtEtaPhiM4D<float> &j2)
{
    double dPhi = abs(j2.Phi() - j1.Phi());
    dPhi = min(dPhi, 2*M_PI - dPhi);
    double dEta = abs(j2.Eta() - j1.Eta());

    return (dEta*dEta + dPhi*dPhi);
}

const bool withoutRes = true; //without residual corrections?

const string jecTagCHS = "Summer16_07Aug2017";
const int versionCHS = 5;

//const string jecTagPUPPI = "Summer16_07Aug2017";
//const int versionPUPPI = 5;

const string jecTagPUPPI = "Spring16_23Sep2016";
const int versionPUPPI = 2;


bool isLeptonVetoJet(const QCDjet &jet) {

    auto NHF = jet.nhf; //OK
    auto NEMF = jet.phf;//OK
    auto NumConst = jet.chm + jet.nhm + jet.phm + jet.elm + jet.mum; //OK

    auto MUF = jet.muf;//OK
    auto eta = jet.p4.Eta();//OK
    auto CHF = jet.chf;//OK
    auto CHM = jet.chm;//OK
    auto CEMF= jet.elf + jet.muf;//OK

    bool tightLepVetoJetID = true;

    if(abs(eta)<=2.7)
       tightLepVetoJetID = (NHF<0.90 && NEMF<0.90 && NumConst>1 && MUF<0.8) && ((abs(eta)<=2.4 && CHF>0 && CHM>0 && CEMF<0.90) || abs(eta)>2.4);
    
    return tightLepVetoJetID;
}



//#ifdef __MAKECINT__
//#pragma link C++ class vector<Jet>+;
//#endif

//gInterpreter->GenerateDictionary("Jet","match.h");
//#include "match.h"


//#pragma link C++ class vector<Jet> +;

struct Histogram {
   static const int nPer = 2 + 'H'-'B'; 

   TString name, title; 
   int nDim = 0;
   vector<TH1 *> hist;

};

struct HistoManager {



};

#define SF TString::Format 





void matching::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();
}


void matching::Histos::Init(TList *fOutput_)
{
    fOutput = fOutput_;

    TH1::SetDefaultSumw2();
    const vector<double> etaBins2  = { 0, 0.261, 0.522, 0.783, 1.044, 1.305, 1.479, 1.653, 1.93, 2.172, 2.322, 2.5, 2.65, 2.853, 2.964, 3.139, 3.489, 3.839, 5.191};

    const vector<double> Ptbinning = {0, 1, 5, 6, 8, 10, 12, 15, 18, 21, 24, 28, 32, 37, 43, 49, 56, 64, 74, 84,97, 114, 133, 153, 174, 196, 220, 245, 272, 300, 330, 362, 395, 430, 468,507, 548, 592, 638, 686, 737, 790, 846, 905, 967,1032, 1101, 1172, 1248, 1327, 1410, 1497, 1588, 1684, 1784, 1890,  2000,2116, 2238, 2366, 2500, 2640, 2787, 2941, 3103, 3273, 3450, 3637, 3832,4037, 4252, 4477, 4713, 4961, 5220, 5492, 5777, 6076, 6389, 6717,    7000};

    const int nAsym = 120;
    vector<double> asymBins;
    for(int i = 0; i <= nAsym; ++i) asymBins.push_back( 0.6 + 0.8 * i /(nAsym+0.0));


    vector<double> jecBins;
    for(double s = 0.9; s <= 1.6; s += 0.7/70)
        jecBins.push_back(s);

    for(int i = 0; i < nPer; ++i) {
        char per = 'A' + i;
        hBalEtaPt[i] = new TH3D(SF("hBalEtaPt_%c",per), SF("hBalEtaPt_%c",per), etaBins2.size()-1, etaBins2.data(), Ptbinning.size()-1, Ptbinning.data(),  asymBins.size()-1, asymBins.data()); //,  60, 0.8, 1.2);
        fOutput->Add(hBalEtaPt[i]);
        hJetPt[i] = new TH1D(SF("hJetPt_%c",per), SF("hJetPt_%c",per), Ptbinning.size()-1, Ptbinning.data());
        fOutput->Add(hJetPt[i]);

        hJECpuppi[i] = new TH3D(SF("hJECpuppi_%c",per), SF("hJECpuppi_%c",per), etaBins2.size()-1, etaBins2.data(),
                                                                                Ptbinning.size()-1, Ptbinning.data(),
                                                                                jecBins.size()-1, jecBins.data());
        fOutput->Add(hJECpuppi[i]);
        hJECchs[i] = new TH3D(SF("hJECchs_%c",per), SF("hJECchs_%c",per), etaBins2.size()-1, etaBins2.data(),
                                                                          Ptbinning.size()-1, Ptbinning.data(),
                                                                          jecBins.size()-1, jecBins.data());

        fOutput->Add(hJECchs[i]);
        hEtaPtCHS[i] =  new TH2D(SF("hEtaPtCHS_%c",per), SF("hEtaPtCHS_%c",per), etaBins2.size()-1, etaBins2.data(),Ptbinning.size()-1, Ptbinning.data() );
        fOutput->Add(hEtaPtCHS[i]);
        hEtaPtPUPPI[i] = new TH2D(SF("hEtaPtPUPPI_%c",per), SF("hEtaPtPUPPI_%c",per), etaBins2.size()-1, etaBins2.data(),Ptbinning.size()-1, Ptbinning.data() );
        fOutput->Add(hEtaPtPUPPI[i]);
        hEtaPtPUPPIalone[i] = new TH2D(SF("hEtaPtPUPPIalone_%c",per), SF("hEtaPtPUPPIalone_%c",per), etaBins2.size()-1, etaBins2.data(),Ptbinning.size()-1, Ptbinning.data() );
        fOutput->Add(hEtaPtPUPPIalone[i]);

    }

}

void matching::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

   h.Init(fOutput);
   lum.LoadLumis();


}



void CorrectJets(TTreeReaderValue<std::vector<QCDjet> > &Jets, double rho, JECs &jetCorrs ) {
    for (unsigned i = 0; i < Jets->size(); ++i) {
        double pt  = Jets->at(i).p4.Pt();
        double eta = Jets->at(i).p4.Eta();
        double area = Jets->at(i).area;

        //cout <<"R "<<  i <<" "<< pt << " "<< eta <<" "<< area <<" "<< rho <<  endl;
        double CorFactorRes, Unc;
        vector<string> dumy;

        double corr = jetCorrs.JEC_CHScorrections(pt, eta, area, rho, dumy, CorFactorRes, Unc);
        //cout << "Corr " << corr << endl;

        corr *= isLeptonVetoJet(Jets->at(i));
        if(withoutRes) corr /= CorFactorRes;

        Jets->at(i).p4.SetPt(pt * corr);

    }
    sort(Jets->begin(), Jets->end(), [](const QCDjet &a, const QCDjet &b) {return a.p4.Pt() > b.p4.Pt();});
}




Bool_t matching::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // When processing keyed objects with PROOF, the object is already loaded
   // and is available via the fObject pointer.
   //
   // This function should contain the \"body\" of the analysis. It can contain
   // simple or elaborate selection criteria, run algorithms on the data
   // of the event and typically fill histograms.
   //
   // The processing can be stopped by calling Abort().
   //
   // Use fStatus to set the return value of TTree::Process().
   //
   // The return value is currently not used.

   fReader.SetEntry(entry);

   if(entry % 100000 == 0)
       cout << "Event " << entry << endl;

    TString fileName =  fReader.GetTree()->GetCurrentFile()->GetName();
    int fileId = fileName[fileName.Length()-6] -'A';
    if(fileName != currFile) {
        cout << "Filename is " <<   fReader.GetTree()->GetCurrentFile()->GetName() << endl;

        //string jecTag = "Summer16_07Aug2017";
        //int version = 4;
        char period =  fileName[fileName.Length()-6];
        vector<string> dumy;
        bool isMC = false;

        jetCorrsCHS.Init(isMC, jecTagCHS, period, versionCHS, "AK4PFchs", "", dumy);
        jetCorrsPUPPI.Init(isMC, jecTagPUPPI, period, versionPUPPI, "AK4PFPuppi", "", dumy);
        currFile = fileName;
    }


    CorrectJets(chsJets,   *rho, jetCorrsCHS);
    CorrectJets(puppiJets, *rho, jetCorrsPUPPI);


    const double jetSel = 5;


    if(chsJets->size() < 1 || puppiJets->size() < 1) return 0;


    double wgt, wgtTot;
    int id;
    tie(wgt,wgtTot,id) = lum.GetWeightID(fileId, chsJets->at(0).p4.Pt() );
    if(wgt == 0 || id < 0)  return 0;
    h.w = wgt;
    h.wTot = wgtTot;
    h.fileId = fileId;

    //cout << "Info " << CHSjetPt[0]<<" "<< id <<" "<< (*CHStriggerBit).at(id) << endl;
    if(triggerBit->at(id) != 1) return 0;


    h.Fill1D(h.hJetPt, chsJets->at(0).p4.Pt());

    //cout << CHSjetPt[0] << " "<< CHSjetPt[0]*(1+CHSjetUnc[0]) <<" "<< CHSjetPt[0]*(1-CHSjetUnc[0]) << endl;


    for (unsigned i = 0; i < puppiJets->size(); ++i) {
        double aEta = abs(puppiJets->at(i).p4.Eta());
        double pt = puppiJets->at(i).p4.Pt();
        h.Fill3D(h.hJECpuppi, aEta, pt, 1);
    }
    for (unsigned i = 0; i < chsJets->size(); ++i) {
        double aEta = abs(chsJets->at(i).p4.Eta());
        double pt   = chsJets->at(i).p4.Pt();
        h.Fill3D(h.hJECchs, aEta, pt, 1);

        if (pt >= jetSel) {
            h.Fill2D(h.hEtaPtCHS, aEta, pt);
        }
    }

    set<int> Indx;
    for(unsigned i = 0; i < chsJets->size(); ++i)
        Indx.insert(i);


    for(unsigned i = 0; i < puppiJets->size(); ++i) {
        double pt   = puppiJets->at(i).p4.Pt();
        double aEta = abs(puppiJets->at(i).p4.Eta());
        if(pt < jetSel) continue;

        h.Fill2D(h.hEtaPtPUPPI, aEta, pt);

        /*
           int m = -1;
           for(int j = 0; j < CHSjetPt.GetSize(); ++j) {
           double d2 = dist2(PUPPIjetEta[i], PUPPIjetPhi[i], CHSjetEta[j], CHSjetPhi[j]);
           if(d2 < 0.2*0.2) {
           m = j;
           break;
           }
           }
           */

        int m = -1;
        for(int ind :  Indx) {
            //cout << ind << endl;
            double d2 = dist2(puppiJets->at(i).p4, chsJets->at(ind).p4);
            if(d2 < 0.2*0.2) {
                m = ind;
                Indx.erase(m);
                break;
            }
        }


        if(m != -1) {
            //cout << "match " << PUPPIjetPt[i] << " "<< PUPPIjetPt[i] / CHSjetPt[m] << endl;
            double r = puppiJets->at(i).p4.Pt() / chsJets->at(m).p4.Pt();
            h.Fill3D(h.hBalEtaPt, aEta, pt, r);

            //cout <<"Event " <<  PUPPIjetJEC[i] << " "<< CHSjetJEC[m] << endl;
        }
        else { //not matched
            h.Fill2D(h.hEtaPtPUPPIalone, aEta, pt);
        }
    }


   return kTRUE;
}

void matching::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

void matching::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.


    cout << "Saving histos" << endl;
    //return;

    gRandom->SetSeed(0);
    TString outputFilename = SF("histos/%sV%d__%sV%d%s_%d.root", jecTagCHS.c_str(), versionCHS,    jecTagPUPPI.c_str(), versionPUPPI,
           withoutRes ? "noRes" : "", gRandom->Integer(999999));
    TFile* outputFile = new TFile(outputFilename,"recreate");

    for(const auto&& obj: *fOutput) {
        if (obj->InheritsFrom("TH1"))
            obj->Write();
    }

    outputFile->Close();

}
