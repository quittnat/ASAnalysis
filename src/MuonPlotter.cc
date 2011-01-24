/*****************************************************************************
*   Collection of tools for producing plots for Muon Fake Rate Analysis      *
*                                                                            *
*                                                  (c) 2010 Benjamin Stieger *
*****************************************************************************/
#include "MuonPlotter.hh"
#include "helper/AnaClass.hh"
#include "helper/Utilities.hh"
#include "helper/FPRatios.hh"
#include "helper/Monitor.hh"

#include "TLorentzVector.h"
#include "TGraphAsymmErrors.h"
#include "TEfficiency.h"

#include <iomanip>

using namespace std;

static const float gMMU = 0.1057;
static const float gMEL = 0.0005;
static const float gMZ  = 91.2;

// Muon Binning //////////////////////////////////////////////////////////////////
static const int gNMuPtbins_0  = 3;
static const double gMuPtbins_0 [gNMuPtbins_0+1]  = {20., 30., 40., 100.};

static const int gNMuPtbins_1  = 4;
static const double gMuPtbins_1 [gNMuPtbins_1+1]  = { 5., 10., 20., 30., 40}; // ., 50., 65., 80.};

static const int gNMuPt2bins_0 = 4;
static const double gMuPt2bins_0[gNMuPt2bins_0+1] = {10., 20., 30., 40., 100.};

static const int gNMuPt2bins_1 = 4;
static const double gMuPt2bins_1[gNMuPt2bins_1+1] = { 5., 10., 20., 30., 40}; // ., 50., 65., 80.};

static const int    gNMuEtabins = 1;
static const double gMuEtabins[gNMuEtabins+1] = {-2.4, 2.4};

// Electron Binning //////////////////////////////////////////////////////////////
static const int gNElPtbins_0  = 3;
static const double gElPtbins_0 [gNElPtbins_0+1]  = {20., 30., 40., 100.};

static const int gNElPtbins_1  = 4;
static const double gElPtbins_1 [gNElPtbins_1+1]  = { 5., 10., 20., 30., 40}; // ., 50., 65., 80.};

static const int gNElPt2bins_0 = 4;
static const double gElPt2bins_0[gNElPt2bins_0+1] = {10., 20., 30., 40., 100.};

static const int gNElPt2bins_1 = 4;
static const double gElPt2bins_1[gNElPt2bins_1+1] = { 5., 10., 20., 30., 40}; // ., 50., 65., 80.};

static const int    gNElEtabins = 1;
static const double gElEtabins[gNElEtabins+1] = {-2.4, 2.4};
//////////////////////////////////////////////////////////////////////////////////


//____________________________________________________________________________
MuonPlotter::MuonPlotter(){
// Default constructor, no samples are set
}

//____________________________________________________________________________
MuonPlotter::MuonPlotter(TString outputdir){
// Explicit constructor with output directory
	setOutputDir(outputdir);
}

//____________________________________________________________________________
MuonPlotter::MuonPlotter(TString outputdir, TString outputfile){
// Explicit constructor with output directory and output file
	setOutputDir(outputdir);
	setOutputFile(outputfile);
}

//____________________________________________________________________________
MuonPlotter::~MuonPlotter(){
	if(fOutputFile->IsOpen()) fOutputFile->Close();
}

//____________________________________________________________________________
void MuonPlotter::init(TString filename){
	if(fVerbose > 0) cout << "------------------------------------" << endl;
	if(fVerbose > 0) cout << "Initializing MuonPlotter ... " << endl;
	if(fVerbose > 0){
		cout << " ... using ";
		if(fSelectionSwitch == 0)      cout << "UCSD/UCSB/FNAL selection." << endl;
		else if(fSelectionSwitch == 1) cout << "UFlorida selection." << endl;
		else cout << "???" << endl;
		if(fChargeSwitch == 1) cout << " ... using opposite-sign charge selection" << endl;
	}
	Util::SetStyle();
	loadSamples(filename);
	readVarNames("anavarnames.dat");
	fOutputFileName = fOutputDir + "Yields.root";

	// fLumiNorm = fSamples[0].lumi; // Normalize everything to this lumi in /pb
	fLumiNorm = 100; // Normalize everything to this lumi in /pb
	fBinWidthScale = 10.; // Normalize Y axis to this binwidth
	fDoCounting = false; // Disable counters by default
	fMinPt1 = 20.;
	fMinPt2 = 10.;

	// Prevent root from adding histograms to current file
	TH1::AddDirectory(kFALSE);

	fMCBG.push_back(TTbar);
	fMCBG.push_back(WJets);
	fMCBG.push_back(ZJets);
	fMCBG.push_back(AstarJets);
	fMCBG.push_back(VVJets);
	fMCBG.push_back(QCD15);
	fMCBG.push_back(QCD30);
	fMCBG.push_back(QCD80);
	fMCBG.push_back(QCD170);
	fMCBG.push_back(SSWWDPS);
	fMCBG.push_back(SSWWSPSPos);
	fMCBG.push_back(SSWWSPSNeg);

	fMCBGMuEnr.push_back(TTbar);
	fMCBGMuEnr.push_back(WJets);
	fMCBGMuEnr.push_back(ZJets);
	fMCBGMuEnr.push_back(AstarJets);
	fMCBGMuEnr.push_back(VVJets);
	fMCBGMuEnr.push_back(InclMu);
	fMCBGMuEnr.push_back(SSWWDPS);
	fMCBGMuEnr.push_back(SSWWSPSPos);
	fMCBGMuEnr.push_back(SSWWSPSNeg);

	fMCBGSig = fMCBG;
	fMCBGSig.push_back(LM0);

	fMuData.push_back(MuA);
	fMuData.push_back(MuB);
	fEGData.push_back(EGA);
	fEGData.push_back(EGB);
	fJMData.push_back(JMA);
	fJMData.push_back(JMB);
	fJMData.push_back(MultiJet);

	fAllSamples.push_back(MuA);
	fAllSamples.push_back(MuB);
	fAllSamples.push_back(EGA);
	fAllSamples.push_back(EGB);
	fAllSamples.push_back(JMA);
	fAllSamples.push_back(JMB);
	fAllSamples.push_back(MultiJet);
	fAllSamples.push_back(TTbar);
	fAllSamples.push_back(WJets);
	fAllSamples.push_back(ZJets);
	fAllSamples.push_back(AstarJets);
	fAllSamples.push_back(VVJets);
	fAllSamples.push_back(QCD15);
	fAllSamples.push_back(QCD30);
	fAllSamples.push_back(QCD80);
	fAllSamples.push_back(QCD170);
	fAllSamples.push_back(SSWWDPS);
	fAllSamples.push_back(SSWWSPSPos);
	fAllSamples.push_back(SSWWSPSNeg);
	fAllSamples.push_back(LM0);
	fAllSamples.push_back(InclMu);

	bookHistos();
}

//____________________________________________________________________________
void MuonPlotter::loadSamples(const char* filename){
	char buffer[200];
	ifstream IN(filename);

	char ParName[100], StringValue[1000];
	float ParValue;

	if(fVerbose > 2) cout << "------------------------------------" << endl;
	if(fVerbose > 2) cout << "Sample File  " << filename << endl;
	int counter(0);

	while( IN.getline(buffer, 200, '\n') ){
		// ok = false;
		if (buffer[0] == '#') continue; // Skip lines commented with '#'
		if( !strcmp(buffer, "SAMPLE")){
			sample s;

			IN.getline(buffer, 200, '\n');
			sscanf(buffer, "Name\t%s", StringValue);
			s.name = TString(StringValue);

			IN.getline(buffer, 200, '\n');
			sscanf(buffer, "SName\t%s", StringValue);
			s.sname = TString(StringValue);

			IN.getline(buffer, 200, '\n');
			sscanf(buffer, "File\t%s", StringValue);
			TFile *f = TFile::Open(StringValue);
			s.file = f;
			s.tree = (TTree*)f->Get("Analysis");
			if(s.tree == NULL){ cout << " Tree not found in file!" << endl; break; }

			IN.getline(buffer, 200, '\n');
			sscanf(buffer, "Lumi\t%f", &ParValue);
			s.lumi = ParValue;

			IN.getline(buffer, 200, '\n');
			sscanf(buffer, "IsData\t%f", &ParValue);
			s.isdata = (bool)ParValue;

			IN.getline(buffer, 200, '\n');
			sscanf(buffer, "Color\t%f", &ParValue);
			s.color = ParValue;

			if(fVerbose > 2){
				cout << " ---- " << endl;
				cout << "  New sample added: " << s.name << endl;
				cout << "   Sample no.  " << counter << endl;
				cout << "   Short name: " << s.sname << endl;
				cout << "   File:       " << (s.file)->GetName() << endl;
				cout << "   Events:     " << s.tree->GetEntries() << endl;
				cout << "   Lumi:       " << s.lumi << endl;
				cout << "   Color:      " << s.color << endl;
				cout << "   IsData:     " << s.isdata << endl;
			}
			fSampleMap[s.sname] = counter;
			fSamples.push_back(s);
			counter++;
		}
	}
	if(fVerbose > 2) cout << "------------------------------------" << endl;
}

//____________________________________________________________________________
const int     MuonPlotter::getNMuPtBins(){
	if(fSelectionSwitch == 0) return gNMuPtbins_0;
	if(fSelectionSwitch == 1) return gNMuPtbins_1;
}
const double *MuonPlotter::getMuPtBins(){
	if(fSelectionSwitch == 0) return gMuPtbins_0;
	if(fSelectionSwitch == 1) return gMuPtbins_1;
}
const int     MuonPlotter::getNMuPt2Bins(){
	if(fSelectionSwitch == 0) return gNMuPt2bins_0;
	if(fSelectionSwitch == 1) return gNMuPt2bins_1;
}
const double *MuonPlotter::getMuPt2Bins(){
	if(fSelectionSwitch == 0) return gMuPt2bins_0;
	if(fSelectionSwitch == 1) return gMuPt2bins_1;
}
const int     MuonPlotter::getNMuEtaBins(){
	return gNMuEtabins;
	// if(fSelectionSwitch == 0) return gNMuEtabins_0;
	// if(fSelectionSwitch == 1) return gNMuEtabins_1;
}
const double *MuonPlotter::getMuEtaBins(){
	return gMuEtabins;
	// if(fSelectionSwitch == 0) return gMuEtabins_0;
	// if(fSelectionSwitch == 1) return gMuEtabins_1;
}

//____________________________________________________________________________
const int     MuonPlotter::getNElPtBins(){
	if(fSelectionSwitch == 0) return gNElPtbins_0;
	if(fSelectionSwitch == 1) return gNElPtbins_1;
}
const double *MuonPlotter::getElPtBins(){
	if(fSelectionSwitch == 0) return gElPtbins_0;
	if(fSelectionSwitch == 1) return gElPtbins_1;
}
const int     MuonPlotter::getNElPt2Bins(){
	if(fSelectionSwitch == 0) return gNElPt2bins_0;
	if(fSelectionSwitch == 1) return gNElPt2bins_1;
}
const double *MuonPlotter::getElPt2Bins(){
	if(fSelectionSwitch == 0) return gElPt2bins_0;
	if(fSelectionSwitch == 1) return gElPt2bins_1;
}
const int     MuonPlotter::getNElEtaBins(){
	return gNElEtabins;
	// if(fSelectionSwitch == 0) return gNElEtabins_0;
	// if(fSelectionSwitch == 1) return gNElEtabins_1;
}
const double *MuonPlotter::getElEtaBins(){
	return gElEtabins;
	// if(fSelectionSwitch == 0) return gElEtabins_0;
	// if(fSelectionSwitch == 1) return gElEtabins_1;
}

//____________________________________________________________________________
void MuonPlotter::doAnalysis(){
	if(readHistos(fOutputFileName) != 0) return;
	
	fLumiNorm = 35.;
	// printYields(Muon);
	// printYields(Electron);
	// printYields(EMu);

	// printYields(Muon,     fLumiNorm);
	// printYields(Electron, fLumiNorm);
	// printYields(EMu,      fLumiNorm);

	// printYieldsShort();

	// makeIntPrediction(fOutputDir + "Yields.txt");
	// makeMuIsolationPlot();
	
	makeMufRatioPlots();
	makeMupRatioPlots();
	makeElfRatioPlots();
	makeElpRatioPlots();

	// makeDiffPredictionPlots();
	// makeDataClosurePlots();
	// makeNT012Plots();

	// makeMuIsoVsPtPlot(fMCBG, 0, &MuonPlotter::isSigSupMuEvent, &MuonPlotter::isLooseMuon, fMuData, 0, &MuonPlotter::isSigSupMuEventTRG, &MuonPlotter::isLooseMuon, "IsoVsPt_SigSuppressed", false);
}

//____________________________________________________________________________
void MuonPlotter::doLoop(){
	vector<int> samples = fAllSamples;
	int step = 5000;
	fDoCounting = true;
	TString eventfilename = fOutputDir + "InterestingEvents.txt";
	fOUTSTREAM.open(eventfilename.Data(), ios::trunc);

	// Sample loop
	for(size_t i = 0; i < samples.size(); ++i){
		int index = samples[i];

		TTree *tree = fSamples[index].tree;
		const bool isdata = fSamples[index].isdata;

		fCurrentSample = gSample(index);

		// Stuff to execute for each sample BEFORE looping on the events
		initCounters();

		// Event loop
		tree->ResetBranchAddresses();
		Init(tree);
		if (fChain == 0) return;
		Long64_t nentries = fChain->GetEntriesFast();
		Long64_t nbytes = 0, nb = 0;
		for (Long64_t jentry=0; jentry<nentries;jentry++) {
			// if(fVerbose > 1) cout << " Processing : " << setw(8) << setprecision(1) << jentry<< "\r" << flush;
			step = nentries/20;
			if( step < 200 ) step = 200;
			if( step > 10000 ) step = 10000;
			printProgress(jentry, nentries, fSamples[index].name, step);
			// if(fVerbose > 1 && (jentry%step == 0 || (jentry+1 == nentries)) ) cout << " Processing " << setw(40) << left << fSamples[index].name + "... " << setw(4) << setprecision(3) << (int)((float)(jentry+1)/(float)nentries*100.) << "%      \r" << flush;
			// if(fVerbose > 1 && jentry%step == 0) cout << " Processing " << setw(40) << left << fSamples[index].name + "... " << setw(4) << setprecision(3) << (float)jentry/(float)nentries*100. << "%  \r" << flush;

			Long64_t ientry = LoadTree(jentry);
			if (ientry < 0) break;
			nb = fChain->GetEntry(jentry);   nbytes += nb;

			fCounters[fCurrentSample][Muon]    .fill("All events");
			fCounters[fCurrentSample][EMu]     .fill("All events");
			fCounters[fCurrentSample][Electron].fill("All events");
		
			// Select mutually exclusive runs for Jet and MultiJet datasets
			if(!isGoodRun(index)) continue;
		
			fCounters[fCurrentSample][Muon]    .fill(" ... is good run");
			fCounters[fCurrentSample][EMu]     .fill(" ... is good run");
			fCounters[fCurrentSample][Electron].fill(" ... is good run");
		
			fillYields();

		}
		cout << endl;
		
		// Stuff do execute for each sample AFTER looping on the events
		storeNumbers();
		
	}
	fOUTSTREAM.close();
	writeHistos();
	printCutFlows(fOutputDir + "CutFlow.txt");
	fDoCounting = false;
}

//____________________________________________________________________________
void MuonPlotter::makeDiffPredictionPlots(){
	// Fill the ratios
	fLumiNorm = 1000.;
	// printYields(fLumiNorm);

	cout << "Producing prediction for :" << endl;
	for(size_t i = 0; i < fMCBGSig.size(); ++i){
		int ind = fMCBGSig[i];
		cout << " " << fSamples[ind].sname << flush;
	}
	cout << endl;

	// fillfRatio(fJMData, 0);
	// fillpRatio(fMuData, 0);
	// 
	// makeSSPredictionPlots(fMuData);

	fillMufRatio(fMCBGSig, 0);
	fillMupRatio(fMCBGSig, 0);

	makeSSPredictionPlots(fMCBGSig);
}

//____________________________________________________________________________
void MuonPlotter::makeDataClosurePlots(){
	// Fill the ratios
	fLumiNorm = 35.;

	vector<int> samples;
	if(fSelectionSwitch == 0) samples = fMuData;
	if(fSelectionSwitch == 1) samples = fJMData;

	fillMufRatio(fJMData, 0);
	fillMupRatio(fMuData, 0);

	// Prediction: /////////////////////////////////////////////////////////////////////
	// Observation: ////////////////////////////////////////////////////////////////////
	TH1D *H_nsigpred = new TH1D("Nsigpred", "Predicted N_sig in Pt1 bins",       getNMuPt2Bins(),  getMuPt2Bins());
	TH1D *H_nfppred  = new TH1D("Nfppred",  "Predicted N_fp in Pt1 bins",        getNMuPt2Bins(),  getMuPt2Bins());
	TH1D *H_nffpred  = new TH1D("Nffpred",  "Predicted N_ff in Pt1 bins",        getNMuPt2Bins(),  getMuPt2Bins());
	TH1D *H_nFpred   = new TH1D("NFpred",   "Total predicted fakes in Pt1 bins", getNMuPt2Bins(),  getMuPt2Bins());
	TH1D *H_nt2obs   = new TH1D("Nt2obs",   "Observed Nt2 in Pt1 bins",          getNMuPt2Bins(),  getMuPt2Bins());

	H_nsigpred->Sumw2();
	H_nfppred ->Sumw2();
    H_nffpred ->Sumw2();
    H_nFpred  ->Sumw2();
    H_nt2obs  ->Sumw2();

	bool output = true;

	for(size_t i = 0; i < samples.size(); ++i){
		int index = samples[i];
		// float scale = fLumiNorm/fSamples[samples[i]].lumi;

		TH2D *h_nt2_temp = new TH2D(Form("Nt2_temp_%s", fSamples[index].sname.Data()), "Nt2_temp", getNMuPt2Bins(), getMuPt2Bins(), getNMuPt2Bins(), getMuPt2Bins());
		TH2D *h_nt1_temp = new TH2D(Form("Nt1_temp_%s", fSamples[index].sname.Data()), "Nt1_temp", getNMuPt2Bins(), getMuPt2Bins(), getNMuPt2Bins(), getMuPt2Bins());
		TH2D *h_nt0_temp = new TH2D(Form("Nt0_temp_%s", fSamples[index].sname.Data()), "Nt0_temp", getNMuPt2Bins(), getMuPt2Bins(), getNMuPt2Bins(), getMuPt2Bins());
		
		// Event loop
		TTree *tree = fSamples[index].tree;
		tree->ResetBranchAddresses();
		Init(tree);
		if (fChain == 0) return;
		Long64_t nentries = fChain->GetEntriesFast();
		Long64_t nbytes = 0, nb = 0;
		for (Long64_t jentry=0; jentry<nentries;jentry++) {
			printProgress(jentry, nentries, fSamples[index].name);

			Long64_t ientry = LoadTree(jentry);
			if (ientry < 0) break;
			nb = fChain->GetEntry(jentry);   nbytes += nb;

			int mu1(-1), mu2(-1);
			if(!isSigSupSSMuMuEvent(mu1, mu2)) continue;

			if( isTightMuon(mu1) &&  isTightMuon(mu2)) H_nt2obs->Fill(MuPt[mu1]);

			if( isTightMuon(mu1) &&  isTightMuon(mu2)) h_nt2_temp->Fill(MuPt[mu1], MuPt[mu2]);
			if( isTightMuon(mu1) && !isTightMuon(mu2)) h_nt1_temp->Fill(MuPt[mu1], MuPt[mu2]);
			if(!isTightMuon(mu1) &&  isTightMuon(mu2)) h_nt1_temp->Fill(MuPt[mu2], MuPt[mu1]);
			if(!isTightMuon(mu1) && !isTightMuon(mu2)) h_nt0_temp->Fill(MuPt[mu1], MuPt[mu2]);
		}

		vector<TH1D*> prediction = MuMuFPPrediction(fH2D_MufRatio, fH2D_MupRatio, h_nt2_temp, h_nt1_temp, h_nt0_temp, output);
		H_nsigpred->Add(prediction[0]);
		H_nfppred ->Add(prediction[1]);
		H_nffpred ->Add(prediction[2]);
		
		delete h_nt2_temp;
		delete h_nt1_temp;
		delete h_nt0_temp;
	}
	// Output
	H_nFpred->Add(H_nfppred);
	H_nFpred->Add(H_nffpred);
	H_nFpred->SetXTitle(convertVarName("MuPt[0]"));
	H_nFpred->SetYTitle(Form("Events / %2.0f GeV", fBinWidthScale));
	H_nt2obs->SetXTitle(convertVarName("MuPt[0]"));
	H_nt2obs->SetYTitle(Form("Events / %2.0f GeV", fBinWidthScale));

	// Normalize to binwidth
	H_nsigpred = normHistBW(H_nsigpred, fBinWidthScale);
	H_nfppred  = normHistBW(H_nfppred,  fBinWidthScale);
	H_nffpred  = normHistBW(H_nffpred,  fBinWidthScale);
	H_nFpred   = normHistBW(H_nFpred,   fBinWidthScale);
	H_nt2obs   = normHistBW(H_nt2obs,   fBinWidthScale);

	H_nt2obs->SetFillColor(kBlue);
	H_nt2obs->SetLineColor(kBlue);
	H_nt2obs->SetFillStyle(3004);
	H_nt2obs->SetLineWidth(2);

	H_nsigpred->SetFillColor(8);
	H_nsigpred->SetMarkerColor(8);
	H_nsigpred->SetMarkerStyle(20);
	H_nsigpred->SetLineColor(8);
	H_nsigpred->SetLineWidth(2);

	H_nfppred->SetFillColor(kRed);
	H_nfppred->SetMarkerColor(kRed);
	H_nfppred->SetMarkerStyle(20);
	H_nfppred->SetLineColor(kRed);
	H_nfppred->SetLineWidth(2);

	H_nffpred->SetFillColor(13);
	H_nffpred->SetMarkerColor(13);
	H_nffpred->SetLineColor(13);
	H_nffpred->SetMarkerStyle(20);
	H_nffpred->SetLineWidth(2);

	plotOverlay4H(H_nt2obs, "N_{ t2}", H_nsigpred, "N_{ pp}" , H_nfppred, "N_{ f p}", H_nffpred, "N_{ f f}");

	H_nFpred->SetMinimum(0.);
	H_nt2obs->SetMinimum(0.);
	H_nsigpred->SetMinimum(0.);

	H_nsigpred->SetMarkerColor(kRed);
	H_nFpred->SetMarkerColor(kRed);
	H_nFpred->SetMarkerStyle(20);
	H_nsigpred->SetMaximum(14.);
	H_nsigpred->SetMinimum(0.);

	plotPredOverlay2HWithRatio(H_nFpred, "Predicted Fakes", H_nt2obs,  "Obs. N_{t2}", false, false);
}

//____________________________________________________________________________
void MuonPlotter::makeNT012Plots(){
	vector<int> mcsamples = fMCBG;
	const bool read = false;

	THStack *hnt2_stack = new THStack("nt2_stack", "Observed Nt2");
	THStack *hnt1_stack = new THStack("nt1_stack", "Observed Nt1");
	THStack *hnt0_stack = new THStack("nt0_stack", "Observed Nt0");
	const unsigned int nmcsamples = mcsamples.size();
	TH1D *hnt2[nmcsamples];
	TH1D *hnt1[nmcsamples];
	TH1D *hnt0[nmcsamples];

	if(read){
		hnt2_stack = (THStack*)fOutputFile->Get(fOutputDir + "/nt2_stack");
		hnt1_stack = (THStack*)fOutputFile->Get(fOutputDir + "/nt1_stack");
		hnt0_stack = (THStack*)fOutputFile->Get(fOutputDir + "/nt0_stack");
		TList *list2 = hnt2_stack->GetHists();
		TList *list1 = hnt1_stack->GetHists();
		TList *list0 = hnt0_stack->GetHists();
		if(list2->GetSize() != nmcsamples || list1->GetSize() != nmcsamples || list0->GetSize() != nmcsamples) return;
		for(size_t i = 0; i < nmcsamples; ++i){
			int index = mcsamples[i];
			hnt2[i] = (TH1D*)list2->At(i);
			hnt2[i]->SetFillColor(fSamples[index].color);
			hnt1[i] = (TH1D*)list1->At(i);
			hnt1[i]->SetFillColor(fSamples[index].color);
			hnt0[i] = (TH1D*)list0->At(i);
			hnt0[i]->SetFillColor(fSamples[index].color);
		}
	}
	
	if(!read){
		TTree *tree = NULL;
		for(size_t i = 0; i < mcsamples.size(); ++i){
			int index = mcsamples[i];
			tree = fSamples[index].tree;
			hnt2[i] = new TH1D(Form("nt2_%s", fSamples[index].sname.Data()), "Observed Nt2", getNMuPt2Bins(), getMuPt2Bins());
			hnt1[i] = new TH1D(Form("nt1_%s", fSamples[index].sname.Data()), "Observed Nt1", getNMuPt2Bins(), getMuPt2Bins());
			hnt0[i] = new TH1D(Form("nt0_%s", fSamples[index].sname.Data()), "Observed Nt0", getNMuPt2Bins(), getMuPt2Bins());
			hnt2[i]->SetFillColor(fSamples[index].color);
			hnt1[i]->SetFillColor(fSamples[index].color);
			hnt0[i]->SetFillColor(fSamples[index].color);
			hnt2[i]->Sumw2();
			hnt1[i]->Sumw2();
			hnt0[i]->Sumw2();
			float scale = fLumiNorm / fSamples[index].lumi;
			tree->ResetBranchAddresses();
			Init(tree);
			if (fChain == 0) return;
			Long64_t nentries = fChain->GetEntriesFast();
			Long64_t nbytes = 0, nb = 0;
			for (Long64_t jentry=0; jentry<nentries;jentry++) {
				Long64_t ientry = LoadTree(jentry);
				if (ientry < 0) break;
				if(fVerbose > 1) printProgress(jentry, nentries, fSamples[index].name);
				nb = fChain->GetEntry(jentry);   nbytes += nb;

				int mu1(-1), mu2(-1);
				if(!isSigSupSSMuMuEvent(mu1, mu2)) continue;

				if( isTightMuon(mu1) &&  isTightMuon(mu2)) hnt2[i]->Fill(MuPt[mu1], scale);
				if( isTightMuon(mu1) && !isTightMuon(mu2)) hnt1[i]->Fill(MuPt[mu1], scale);
				if(!isTightMuon(mu1) &&  isTightMuon(mu2)) hnt1[i]->Fill(MuPt[mu2], scale);
				if(!isTightMuon(mu1) && !isTightMuon(mu2)) hnt0[i]->Fill(MuPt[mu1], scale);
			}
			hnt2_stack->Add(hnt2[i]);
			hnt1_stack->Add(hnt1[i]);
			hnt0_stack->Add(hnt0[i]);
			if(fVerbose > 1) cout << endl;
		}
	}
	
	hnt2_stack->Draw();
	hnt2_stack->GetXaxis()->SetTitle(convertVarName("MuPt[0]"));
	hnt1_stack->Draw();
	hnt1_stack->GetXaxis()->SetTitle(convertVarName("MuPt[0]"));
	hnt0_stack->Draw();
	hnt0_stack->GetXaxis()->SetTitle(convertVarName("MuPt[0]"));
		
	TCanvas *c2 = new TCanvas("Nt2", "Observed Nt2", 0, 0, 800, 600);
	TCanvas *c1 = new TCanvas("Nt1", "Observed Nt1", 0, 0, 800, 600);
	TCanvas *c0 = new TCanvas("Nt0", "Observed Nt0", 0, 0, 800, 600);

	// TLegend *leg = new TLegend(0.13,0.60,0.38,0.88);
	TLegend *leg = new TLegend(0.75,0.60,0.89,0.88);
	for(size_t i = 0; i < nmcsamples; ++i){
		int index = mcsamples[i];
		leg->AddEntry(hnt2[i], fSamples[index].sname.Data(), "f");
	}
	leg->SetFillStyle(0);
	leg->SetTextFont(42);
	leg->SetBorderSize(0);

	c2->cd();
	hnt2_stack->Draw("hist");
	leg->Draw();

	c1->cd();
	hnt1_stack->Draw("hist");
	leg->Draw();

	c0->cd();
	hnt0_stack->Draw("hist");
	leg->Draw();

	Util::PrintNoEPS(c2, "ObservedNt2", fOutputDir, fOutputFile);
	Util::PrintNoEPS(c1, "ObservedNt1", fOutputDir, fOutputFile);
	Util::PrintNoEPS(c0, "ObservedNt0", fOutputDir, fOutputFile);
}

//____________________________________________________________________________
void MuonPlotter::makeMufRatioPlots(){
	// TH1D *h_fdata  = fillRatioPt(fMuData, 0, &MuonPlotter::isSigSupMuEventTRG, &MuonPlotter::isLooseMuon);      // JetMET Dataset (Single Muon Selection)
	// TH1D *h_fttbar = fillRatioPt(TTbar,   0, &MuonPlotter::isGoodMuEvent,                  &MuonPlotter::isFakeTTbarMuon); // TTbarJets MC
	// TH1D *h_fallmc = fillRatioPt(fMCBG,   0, &MuonPlotter::isSigSupMuEvent,    &MuonPlotter::isLooseMuon);      // QCD MC
	fLumiNorm = fSamples[JMA].lumi + fSamples[JMB].lumi;
	TH1D *h_fdata1 = fillMuRatioPt(fJMData, 1);      // JetMET Dataset (Single Muon Selection)
	TH1D *h_fdata2 = fillMuRatioPt(fMuData, 1);
	TH1D *h_fallmc = fillMuRatioPt(fMCBG,   1);      // QCD MC
	TH1D *h_fttbar = fillMuRatioPt(TTbar,   0, &MuonPlotter::isSigSupMuEvent, &MuonPlotter::isFakeTTbarMuon); // TTbarJets MC
	h_fdata1->SetName("MufRatioData");
	h_fdata2->SetName("MufRatioDataMu");
	h_fttbar->SetName("MufRatioTTbar");
	h_fallmc->SetName("MufRatioAllMC");

	// setPlottingRange(h_fdata1, h_fttbar, h_fallmc);

	h_fdata1->SetMinimum(0.);
	h_fdata2->SetMinimum(0.);
	h_fttbar->SetMinimum(0.);
	h_fallmc->SetMinimum(0.);

	h_fdata1->SetMarkerColor(kBlack);
	h_fdata2->SetMarkerColor(kBlue);
	h_fttbar->SetMarkerColor(kBlue);
	h_fallmc->SetMarkerColor(kRed);

	h_fdata1->SetMarkerStyle(20);
	h_fdata2->SetMarkerStyle(20);
	h_fttbar->SetMarkerStyle(20);
	h_fallmc->SetMarkerStyle(20);

	// h_fdata1->SetMarkerSize(2);
	// h_fdata2->SetMarkerSize(2);
	// h_fttbar->SetMarkerSize(2);
	// h_fallmc->SetMarkerSize(2);

	h_fdata1->SetLineWidth(2);
	h_fdata2->SetLineWidth(2);
	h_fttbar->SetLineWidth(2);
	h_fallmc->SetLineWidth(2);

	h_fdata1->SetLineColor(kBlack);
	h_fdata2->SetLineColor(kBlue);
	h_fttbar->SetLineColor(kBlue);
	h_fallmc->SetLineColor(kRed);

	h_fdata1->SetFillColor(kBlack);
	h_fdata2->SetFillColor(kBlue);
	h_fttbar->SetFillColor(kBlue);
	h_fallmc->SetFillColor(kRed);

	plotRatioOverlay3H(h_fdata1, "Data (Jet, L = 21.7 pb^{-1})", h_fttbar, "t#bar{t} Fake GenMatch", h_fallmc, "QCD, t#bar{t}+jets, V+jets");
	// setPlottingRange(h_fdata1, h_fdata2);
	plotRatioOverlay2H(h_fdata1, "Data (Jet, L = 21.7 pb^{-1})", h_fdata2, "Data (Muon, L = 21.3 pb^{-1})");
	// plotRatioOverlay3H(h_fdata2, "Data (Mu, L = 21.3 pb^{-1})", h_fttbar, "t#bar{t} Fake GenMatch", h_fallmc, "QCD, t#bar{t}+jets, V+jets");

	vector<int> mcsamples = fMCBGSig;
	THStack *hntight_stack = new THStack("ntight_stack", "Stack of tight muons");
	THStack *hnloose_stack = new THStack("nloose_stack", "Stack of loose muons");
	const unsigned int nmcsamples = mcsamples.size();
	TH1D *hntight[nmcsamples];
	TH1D *hnloose[nmcsamples];

	for(size_t i = 0; i < mcsamples.size(); ++i){
		int index = mcsamples[i];
		channel *cha = &fSamples[index].mumu;
		hntight[i] = cha->fhistos.h_ntight_pt;
		hnloose[i] = cha->fhistos.h_nloose_pt;
		hntight[i]->SetFillColor(fSamples[index].color);
		hnloose[i]->SetFillColor(fSamples[index].color);
		float scale = fLumiNorm / fSamples[index].lumi;
		hntight[i]->Scale(scale);
		hnloose[i]->Scale(scale);
		hntight_stack->Add(hntight[i]);
		hnloose_stack->Add(hnloose[i]);
	}
	hntight_stack->Draw();
	hntight_stack->GetXaxis()->SetTitle(convertVarName("MuPt[0]"));
	hnloose_stack->Draw();
	hnloose_stack->GetXaxis()->SetTitle(convertVarName("MuPt[0]"));
	
	TCanvas *c_tight = new TCanvas("MufStackTight", "Tight Stack", 0, 0, 800, 600);
	TCanvas *c_loose = new TCanvas("MufStackLoose", "Loose Stack", 0, 0, 800, 600);

	// TLegend *leg = new TLegend(0.13,0.60,0.38,0.88);
	TLegend *leg = new TLegend(0.75,0.60,0.89,0.88);
	for(size_t i = 0; i < nmcsamples; ++i){
		int index = mcsamples[i];
		leg->AddEntry(hntight[i], fSamples[index].sname.Data(), "f");
	}
	leg->SetFillStyle(0);
	leg->SetTextFont(42);
	leg->SetBorderSize(0);

	c_tight->cd();
	// gPad->SetLogy();
	hntight_stack->Draw("hist");
	leg->Draw();

	c_loose->cd();
	// gPad->SetLogy();
	hnloose_stack->Draw("hist");
	leg->Draw();

	Util::PrintNoEPS(c_tight, "MufRatioTightStack", fOutputDir, fOutputFile);
	Util::PrintNoEPS(c_loose, "MufRatioLooseStack", fOutputDir, fOutputFile);	
}

//____________________________________________________________________________
void MuonPlotter::makeElfRatioPlots(){
	fLumiNorm = 35.;
	TH1D *h_fdata1 = fillElRatioPt(fJMData, 1);      // JetMET Dataset (Single Muon Selection)
	TH1D *h_fdata2 = fillElRatioPt(fEGData, 1);
	TH1D *h_fallmc = fillElRatioPt(fMCBG,   1);      // QCD MC
	h_fdata1->SetName("ElfRatioData");
	h_fdata2->SetName("ElfRatioDataEl");
	h_fallmc->SetName("ElfRatioAllMC");

	// setPlottingRange(h_fdata1, h_fallmc);

	h_fdata1->SetMinimum(0.);
	h_fdata2->SetMinimum(0.);
	h_fallmc->SetMinimum(0.);

	h_fdata1->SetMarkerColor(kBlack);
	h_fdata2->SetMarkerColor(kBlue);
	h_fallmc->SetMarkerColor(kRed);

	h_fdata1->SetMarkerStyle(20);
	h_fdata2->SetMarkerStyle(20);
	h_fallmc->SetMarkerStyle(20);

	// h_fdata1->SetMarkerSize(2);
	// h_fdata2->SetMarkerSize(2);
	// h_fallmc->SetMarkerSize(2);

	h_fdata1->SetLineWidth(2);
	h_fdata2->SetLineWidth(2);
	h_fallmc->SetLineWidth(2);

	h_fdata1->SetLineColor(kBlack);
	h_fdata2->SetLineColor(kBlue);
	h_fallmc->SetLineColor(kRed);

	h_fdata1->SetFillColor(kBlack);
	h_fdata2->SetFillColor(kBlue);
	h_fallmc->SetFillColor(kRed);

	plotRatioOverlay2H(h_fdata1, "Data (Jet, L = 35 pb^{-1})", h_fallmc, "QCD, t#bar{t}+jets, V+jets");
	// setPlottingRange(h_fdata1, h_fdata2);
	plotRatioOverlay2H(h_fdata1, "Data (Jet, L = 35 pb^{-1})", h_fdata2, "Data (EGamma, L = 35 pb^{-1})");

	vector<int> mcsamples = fMCBGSig;
	THStack *hntight_stack = new THStack("ntight_stack", "Stack of tight electrons");
	THStack *hnloose_stack = new THStack("nloose_stack", "Stack of loose electrons");
	const unsigned int nmcsamples = mcsamples.size();
	TH1D *hntight[nmcsamples];
	TH1D *hnloose[nmcsamples];

	for(size_t i = 0; i < mcsamples.size(); ++i){
		int index = mcsamples[i];
		channel *cha = &fSamples[index].mumu;
		hntight[i] = cha->fhistos.h_ntight_pt;
		hnloose[i] = cha->fhistos.h_nloose_pt;
		hntight[i]->SetFillColor(fSamples[index].color);
		hnloose[i]->SetFillColor(fSamples[index].color);
		float scale = fLumiNorm / fSamples[index].lumi;
		hntight[i]->Scale(scale);
		hnloose[i]->Scale(scale);
		hntight_stack->Add(hntight[i]);
		hnloose_stack->Add(hnloose[i]);
	}
	hntight_stack->Draw();
	hntight_stack->GetXaxis()->SetTitle(convertVarName("ElPt[0]"));
	hnloose_stack->Draw();
	hnloose_stack->GetXaxis()->SetTitle(convertVarName("ElPt[0]"));
	
	TCanvas *c_tight = new TCanvas("ElfStackTight", "Tight Stack", 0, 0, 800, 600);
	TCanvas *c_loose = new TCanvas("ElfStackLoose", "Loose Stack", 0, 0, 800, 600);

	// TLegend *leg = new TLegend(0.13,0.60,0.38,0.88);
	TLegend *leg = new TLegend(0.75,0.60,0.89,0.88);
	for(size_t i = 0; i < nmcsamples; ++i){
		int index = mcsamples[i];
		leg->AddEntry(hntight[i], fSamples[index].sname.Data(), "f");
	}
	leg->SetFillStyle(0);
	leg->SetTextFont(42);
	leg->SetBorderSize(0);

	c_tight->cd();
	// gPad->SetLogy();
	hntight_stack->Draw("hist");
	leg->Draw();

	c_loose->cd();
	// gPad->SetLogy();
	hnloose_stack->Draw("hist");
	leg->Draw();

	Util::PrintNoEPS(c_tight, "ElfRatioTightStack", fOutputDir, fOutputFile);
	Util::PrintNoEPS(c_loose, "ElfRatioLooseStack", fOutputDir, fOutputFile);	
}

//____________________________________________________________________________
void MuonPlotter::makeMupRatioPlots(){
	// TH1D *h_pdata  = fillRatioPt(fMuData, 0, &MuonPlotter::isZMuMuEventTRG, &MuonPlotter::isLooseMuon); // Mu Dataset (Di Muon Selection)
	// TH1D *h_pttbar = fillRatioPt(TTbar,   0, &MuonPlotter::isGoodMuEvent, &MuonPlotter::isPromptTTbarMuon); // TTbar
	// TH1D *h_pallmc = fillRatioPt(fMCBG,   0, &MuonPlotter::isZMuMuEvent,    &MuonPlotter::isLooseMuon); // all MC
	fLumiNorm = fSamples[MuA].lumi + fSamples[MuB].lumi;
	TH1D *h_pdata  = fillMuRatioPt(fMuData, 2);
	TH1D *h_pallmc = fillMuRatioPt(fMCBG,   2);
	TH1D *h_pttbar = fillMuRatioPt(TTbar,   0, &MuonPlotter::isGoodMuEvent, &MuonPlotter::isPromptTTbarMuon); // TTbar
	h_pdata ->SetName("MupRatioData");
	h_pttbar->SetName("MupRatioTTbar");
	h_pallmc->SetName("MupRatioAllMC");

	setPlottingRange(h_pdata, h_pttbar, h_pallmc);

	h_pdata ->Draw("goff");
	h_pttbar->Draw("goff");
	h_pallmc->Draw("goff");

	h_pdata ->SetMinimum(0.);
	h_pttbar->SetMinimum(0.);
	h_pallmc->SetMinimum(0.);
	h_pttbar->SetMaximum(1.3);
	h_pallmc->SetMaximum(1.3);

	h_pdata ->SetLineWidth(2);
	h_pttbar->SetLineWidth(2);
	h_pallmc->SetLineWidth(2);

	h_pdata ->SetMarkerColor(kBlack);
	h_pttbar->SetMarkerColor(kBlue);
	h_pallmc->SetMarkerColor(kRed);

	h_pdata ->SetMarkerStyle(20);
	h_pttbar->SetMarkerStyle(20);
	h_pallmc->SetMarkerStyle(20);

	// h_pdata ->SetMarkerSize(2);
	// h_pttbar->SetMarkerSize(2);
	// h_pallmc->SetMarkerSize(2);

	h_pdata ->SetLineColor(kBlack);
	h_pttbar->SetLineColor(kBlue);
	h_pallmc->SetLineColor(kRed);

	h_pdata ->SetFillColor(kBlack);
	h_pttbar->SetFillColor(kBlue);
	h_pallmc->SetFillColor(kRed);

	h_pdata ->SetDrawOption("E1");
	h_pttbar->SetDrawOption("E1");
	h_pallmc->SetDrawOption("E1");

	// plotRatioOverlay3H(h_pdata, "Data (Jet, L = 21.7 pb^{-1})", h_pttbar, "t#bar{t} Prompt GenMatch", h_pallmc, "QCD, t#bar{t}+jets, V+jets");
	plotRatioOverlay3H(h_pdata, "Data (Mu, L = 21.3 pb^{-1})", h_pttbar, "t#bar{t} Prompt GenMatch", h_pallmc, "QCD, t#bar{t}+jets, V+jets");

	vector<int> mcsamples = fMCBGSig;
	THStack *hntight_stack = new THStack("ntight_stack", "Stack of tight muons");
	THStack *hnloose_stack = new THStack("nloose_stack", "Stack of loose muons");
	const unsigned int nmcsamples = mcsamples.size();
	TH1D *hntight[nmcsamples];
	TH1D *hnloose[nmcsamples];

	for(size_t i = 0; i < mcsamples.size(); ++i){
		int index = mcsamples[i];
		channel *cha = &fSamples[index].mumu;
		hntight[i] = cha->phistos.h_ntight_pt;
		hnloose[i] = cha->phistos.h_nloose_pt;
		hntight[i]->SetFillColor(fSamples[index].color);
		hnloose[i]->SetFillColor(fSamples[index].color);
		float scale = fLumiNorm / fSamples[index].lumi;
		hntight[i]->Scale(scale);
		hnloose[i]->Scale(scale);
		hntight_stack->Add(hntight[i]);
		hnloose_stack->Add(hnloose[i]);
	}
	hntight_stack->Draw();
	hntight_stack->GetXaxis()->SetTitle(convertVarName("MuPt[0]"));
	hnloose_stack->Draw();
	hnloose_stack->GetXaxis()->SetTitle(convertVarName("MuPt[0]"));
	
	TCanvas *c_tight = new TCanvas("MupStackTight", "Tight Stack", 0, 0, 800, 600);
	TCanvas *c_loose = new TCanvas("MupStackLoose", "Loose Stack", 0, 0, 800, 600);

	// TLegend *leg = new TLegend(0.13,0.60,0.38,0.88);
	TLegend *leg = new TLegend(0.75,0.60,0.89,0.88);
	for(size_t i = 0; i < nmcsamples; ++i){
		int index = mcsamples[i];
		leg->AddEntry(hntight[i], fSamples[index].sname.Data(), "f");
	}
	leg->SetFillStyle(0);
	leg->SetTextFont(42);
	leg->SetBorderSize(0);

	c_tight->cd();
	// gPad->SetLogy();
	hntight_stack->Draw("hist");
	leg->Draw();

	c_loose->cd();
	// gPad->SetLogy();
	hnloose_stack->Draw("hist");
	leg->Draw();

	Util::PrintNoEPS(c_tight, "MupRatioTightStack", fOutputDir, fOutputFile);
	Util::PrintNoEPS(c_loose, "MupRatioLooseStack", fOutputDir, fOutputFile);	
}

//____________________________________________________________________________
void MuonPlotter::makeElpRatioPlots(){
	fLumiNorm = fSamples[MuA].lumi + fSamples[MuB].lumi;
	TH1D *h_pdata  = fillMuRatioPt(fEGData, 2);
	TH1D *h_pallmc = fillMuRatioPt(fMCBG,   2);
	// TH1D *h_pttbar = fillMuRatioPt(TTbar,   0, &MuonPlotter::isGoodMuEvent, &MuonPlotter::isPromptTTbarMuon); // TTbar
	h_pdata ->SetName("ElpRatioData");
	// h_pttbar->SetName("ElpRatioTTbar");
	h_pallmc->SetName("ElpRatioAllMC");

	setPlottingRange(h_pdata, h_pallmc);

	h_pdata ->Draw("goff");
	// h_pttbar->Draw("goff");
	h_pallmc->Draw("goff");

	h_pdata ->SetMinimum(0.);
	// h_pttbar->SetMinimum(0.);
	h_pallmc->SetMinimum(0.);
	// h_pttbar->SetMaximum(1.3);
	// h_pallmc->SetMaximum(1.3);

	h_pdata ->SetLineWidth(2);
	// h_pttbar->SetLineWidth(2);
	h_pallmc->SetLineWidth(2);

	h_pdata ->SetMarkerColor(kBlack);
	// h_pttbar->SetMarkerColor(kBlue);
	h_pallmc->SetMarkerColor(kRed);

	h_pdata ->SetMarkerStyle(20);
	// h_pttbar->SetMarkerStyle(20);
	h_pallmc->SetMarkerStyle(20);

	// h_pdata ->SetMarkerSize(2);
	// h_pttbar->SetMarkerSize(2);
	// h_pallmc->SetMarkerSize(2);

	h_pdata ->SetLineColor(kBlack);
	// h_pttbar->SetLineColor(kBlue);
	h_pallmc->SetLineColor(kRed);

	h_pdata ->SetFillColor(kBlack);
	// h_pttbar->SetFillColor(kBlue);
	h_pallmc->SetFillColor(kRed);

	h_pdata ->SetDrawOption("E1");
	// h_pttbar->SetDrawOption("E1");
	h_pallmc->SetDrawOption("E1");

	// plotRatioOverlay3H(h_pdata, "Data (Jet, L = 21.7 pb^{-1})", h_pttbar, "t#bar{t} Prompt GenMatch", h_pallmc, "QCD, t#bar{t}+jets, V+jets");
	// plotRatioOverlay3H(h_pdata, "Data (Mu, L = 21.3 pb^{-1})", h_pttbar, "t#bar{t} Prompt GenMatch", h_pallmc, "QCD, t#bar{t}+jets, V+jets");
	plotRatioOverlay2H(h_pdata, "Data (EG, L = 35 pb^{-1})", h_pallmc, "QCD, t#bar{t}+jets, V+jets");

	vector<int> mcsamples = fMCBGSig;
	THStack *hntight_stack = new THStack("ntight_stack", "Stack of tight electrons");
	THStack *hnloose_stack = new THStack("nloose_stack", "Stack of loose electrons");
	const unsigned int nmcsamples = mcsamples.size();
	TH1D *hntight[nmcsamples];
	TH1D *hnloose[nmcsamples];

	for(size_t i = 0; i < mcsamples.size(); ++i){
		int index = mcsamples[i];
		channel *cha = &fSamples[index].mumu;
		hntight[i] = cha->phistos.h_ntight_pt;
		hnloose[i] = cha->phistos.h_nloose_pt;
		hntight[i]->SetFillColor(fSamples[index].color);
		hnloose[i]->SetFillColor(fSamples[index].color);
		float scale = fLumiNorm / fSamples[index].lumi;
		hntight[i]->Scale(scale);
		hnloose[i]->Scale(scale);
		hntight_stack->Add(hntight[i]);
		hnloose_stack->Add(hnloose[i]);
	}
	hntight_stack->Draw();
	hntight_stack->GetXaxis()->SetTitle(convertVarName("ElPt[0]"));
	hnloose_stack->Draw();
	hnloose_stack->GetXaxis()->SetTitle(convertVarName("ElPt[0]"));

	TCanvas *c_tight = new TCanvas("ElpStackTight", "Tight Stack", 0, 0, 800, 600);
	TCanvas *c_loose = new TCanvas("ElpStackLoose", "Loose Stack", 0, 0, 800, 600);

	// TLegend *leg = new TLegend(0.13,0.60,0.38,0.88);
	TLegend *leg = new TLegend(0.75,0.60,0.89,0.88);
	for(size_t i = 0; i < nmcsamples; ++i){
		int index = mcsamples[i];
		leg->AddEntry(hntight[i], fSamples[index].sname.Data(), "f");
	}
	leg->SetFillStyle(0);
	leg->SetTextFont(42);
	leg->SetBorderSize(0);

	c_tight->cd();
	// gPad->SetLogy();
	hntight_stack->Draw("hist");
	leg->Draw();

	c_loose->cd();
	// gPad->SetLogy();
	hnloose_stack->Draw("hist");
	leg->Draw();

	Util::PrintNoEPS(c_tight, "ElpRatioTightStack", fOutputDir, fOutputFile);
	Util::PrintNoEPS(c_loose, "ElpRatioLooseStack", fOutputDir, fOutputFile);	
}

//____________________________________________________________________________
void MuonPlotter::makeMuIsolationPlot(){
	// vector<int> mcsamples = fMCBG;
	vector<int> mcsamples = fMCBGMuEnr;
	// vector<int> datasamples = fJMData;
	vector<int> datasamples = fMuData;
	const bool read = false;
	const float ptcut = 20.;
	const int nbins = 20.;

	int step = 5000;
	TH1D *hiso_data = new TH1D("iso_data", "Muon Isolation in Data", nbins, 0., 1.);
	THStack *hiso_mc_stack = new THStack("iso_mc_stack", "Muon Isolation in MC");
	const unsigned int nmcsamples = mcsamples.size();
	TH1D *hiso_mc[nmcsamples];

	if(read){
		hiso_data     = (TH1D*)   fOutputFile->Get(fOutputDir + "/iso_data");
		hiso_mc_stack = (THStack*)fOutputFile->Get(fOutputDir + "/iso_mc_stack");
		TList *list = hiso_mc_stack->GetHists();
		if(list->GetSize() != nmcsamples) return;
		for(size_t i = 0; i < nmcsamples; ++i){
			int index = mcsamples[i];
			hiso_mc[i] = (TH1D*)list->At(i);
			hiso_mc[i]->SetFillColor(fSamples[index].color);
		}
	}
	
	if(!read){
		TTree *tree = NULL;
		for(size_t i = 0; i < datasamples.size(); ++i){
			int index = datasamples[i];

			tree = fSamples[index].tree;
			tree->ResetBranchAddresses();
			Init(tree);
			if (fChain == 0) return;
			Long64_t nentries = fChain->GetEntriesFast();
			Long64_t nbytes = 0, nb = 0;
			for (Long64_t jentry=0; jentry<nentries;jentry++) {
				Long64_t ientry = LoadTree(jentry);
				if (ientry < 0) break;
				if(fVerbose > 1) printProgress(jentry, nentries, fSamples[index].name);
				nb = fChain->GetEntry(jentry);   nbytes += nb;

				if(isGoodMuEvent() == false) continue;
				if(isMuTriggeredEvent() == false) continue;
  				// if(isSigSupMuEventTRG() == false) continue;

				if(isLooseMuon(0)       == false) continue;
				if(MuPt[0] < ptcut) continue;

				hiso_data->Fill(MuIso[0]);
			}
			if(fVerbose > 1) cout << endl;
		}
		for(size_t i = 0; i < mcsamples.size(); ++i){
			int index = mcsamples[i];
			tree = fSamples[index].tree;
			hiso_mc[i] = new TH1D(Form("iso_mc_%s", fSamples[index].sname.Data()), "Muon Isolation in MC", nbins, 0., 1.);
			hiso_mc[i]->SetFillColor(fSamples[index].color);
			hiso_mc[i]->Sumw2();
			float scale = fLumiNorm / fSamples[index].lumi;
			tree->ResetBranchAddresses();
			Init(tree);
			if (fChain == 0) return;
			Long64_t nentries = fChain->GetEntriesFast();
			Long64_t nbytes = 0, nb = 0;
			for (Long64_t jentry=0; jentry<nentries;jentry++) {
				Long64_t ientry = LoadTree(jentry);
				if (ientry < 0) break;
				if(fVerbose > 1) printProgress(jentry, nentries, fSamples[index].name);
				nb = fChain->GetEntry(jentry);   nbytes += nb;

				if(isGoodMuEvent() == false) continue;
				if(isMuTriggeredEvent() == false) continue;
				// if(isSigSupMuEventTRG() == false) continue;

				if(isLooseMuon(0)       == false) continue;
				if(MuPt[0] < ptcut) continue;

				hiso_mc[i]->Fill(MuIso[0], scale);
			}
			hiso_mc_stack->Add(hiso_mc[i]);
			if(fVerbose > 1) cout << endl;
		}
	}

	hiso_data->SetXTitle(convertVarName("MuIso[0]"));
	hiso_data->SetLineWidth(3);
	hiso_data->SetLineColor(kBlack);
	hiso_data->SetMarkerStyle(8);
	hiso_data->SetMarkerColor(kBlack);
	hiso_data->SetMarkerSize(1.2);
	
	hiso_mc_stack->Draw();
	hiso_mc_stack->GetXaxis()->SetTitle(convertVarName("MuIso[0]"));
	
	// // Apply fudge factor:
	// const float fudge = 2.0;
	// for(size_t i = 0; i < nmcsamples; ++i){
	// 	hiso_mc[i]->Scale(fudge);
	// }
	
	double max1 = hiso_mc_stack->GetMaximum();
	double max2 = hiso_data->GetMaximum();
	double max = max1>max2?max1:max2;
	
	hiso_mc_stack->SetMinimum(0);
	hiso_data->SetMinimum(0);
	hiso_mc_stack->SetMaximum(1.2*max);
	
	TCanvas *c_temp = new TCanvas("Iso", "Isolation in Data vs MC", 0, 0, 800, 600);
	c_temp->cd();

	TLegend *leg = new TLegend(0.13,0.60,0.38,0.88);
	// TLegend *leg = new TLegend(0.75,0.60,0.89,0.88);
	leg->AddEntry(hiso_data, "Data","p");
	for(size_t i = 0; i < nmcsamples; ++i){
		int index = mcsamples[i];
		leg->AddEntry(hiso_mc[i], fSamples[index].sname.Data(), "f");
	}
	leg->SetFillStyle(0);
	leg->SetTextFont(42);
	leg->SetBorderSize(0);
	
	
	hiso_mc_stack->Draw("hist");
	hiso_data->DrawCopy("PE X0 same");
	leg->Draw();

	Util::PrintNoEPS(c_temp, "Isolation", fOutputDir, fOutputFile);	
}

//____________________________________________________________________________
void MuonPlotter::makeMuIsolationPlots(){
	// void MuonPlotter::makeIsoVsPtPlot(vector<int> samples1, int muon1, bool(MuonPlotter::*eventSelector1)(), bool(MuonPlotter::*muonSelector1)(int), vector<int> samples2, int muon2, bool(MuonPlotter::*eventSelector2)(), bool(MuonPlotter::*muonSelector2)(int), TString outputname, bool logy){
	const bool logy = false;
	const int nbins = 30;
	TH2D *h2_data = new TH2D("h2_data", "Isolation vs Pt for muons in data", nbins, 0., 1., getNMuPt2Bins(), getMuPt2Bins());
	h2_data->SetXTitle(convertVarName("MuIso[0]"));
	h2_data->SetYTitle(convertVarName("MuPt[0]"));
	const unsigned int nmcsamples = fMCBG.size();
	TH2D *h2_mc[nmcsamples];
	for(size_t i = 0; i < nmcsamples; ++i){
		TString name = fSamples[i].sname;
		h2_mc[i] = new TH2D(Form("h2_mc_%s", name.Data()), Form("Isolation vs Pt for muons in %s", name.Data()), nbins, 0., 1., getNMuPt2Bins(), getMuPt2Bins());
		h2_mc[i]  ->SetXTitle(convertVarName("MuIso[0]"));
		h2_mc[i]  ->SetYTitle(convertVarName("MuPt[0]"));
	}

	TTree *tree = NULL;
	for(size_t i = 0; i < fJMData.size(); ++i){
		int index = fJMData[i];

		tree = fSamples[index].tree;
		tree->ResetBranchAddresses();
		Init(tree);
		if (fChain == 0) return;
		Long64_t nentries = fChain->GetEntriesFast();
		Long64_t nbytes = 0, nb = 0;
		for (Long64_t jentry=0; jentry<nentries;jentry++) {
			Long64_t ientry = LoadTree(jentry);
			if (ientry < 0) break;
			nb = fChain->GetEntry(jentry);   nbytes += nb;

			if(isSigSupMuEventTRG() == false) continue;
			if(isLooseMuon(0)       == false) continue;

			h2_data->Fill(MuIso[0], MuPt[0]);
		}
	}

	for(size_t i = 0; i < nmcsamples; ++i){
		int index = fMCBG[i];

		tree = fSamples[index].tree;
		float scale = fLumiNorm / fSamples[index].lumi;
		tree->ResetBranchAddresses();
		Init(tree);
		if (fChain == 0) return;
		Long64_t nentries = fChain->GetEntriesFast();
		Long64_t nbytes = 0, nb = 0;
		for (Long64_t jentry=0; jentry<nentries;jentry++) {
			Long64_t ientry = LoadTree(jentry);
			if (ientry < 0) break;
			nb = fChain->GetEntry(jentry);   nbytes += nb;

			if(isSigSupMuEvent() == false) continue;
			if(isLooseMuon(0)    == false) continue;
			h2_mc[i]->Fill(MuIso[0], MuPt[0], scale);
		}
	}

	TLatex *lat = new TLatex();
	lat->SetNDC(kTRUE);
	lat->SetTextColor(kBlack);
	lat->SetTextSize(0.06);

	TCanvas *c_temp = new TCanvas("IsoVsPt", "Isolating in Pt bins", 0, 0, 1200, 800);
	c_temp->Divide(3,2);
	// TCanvas *c_temp = new TCanvas("IsoVsPt", "Isolating in Pt bins", 0, 0, 1200, 1200);
	// c_temp->Divide(3,3);
	// c_temp->cd(9);
	// if(logy) gPad->SetLogz(1);
	// h2_data->DrawCopy("colz");
	// lat->DrawLatex(0.11,0.92, fSamples[TTbar].sname);

	for(unsigned i = 1; i <= getNMuPt2Bins(); ++i){
		c_temp->cd(i);
		gStyle->SetOptStat(1111);
		TH1D *h1 = h2_data->ProjectionX(Form("h2_datax_%d", i), i, i);
		THStack *hs2 = new THStack(Form("h2_mcx_stack_%d", i), "Stack of MC Histos");
		for(size_t j = 0; j < nmcsamples; ++j){
			int index = fMCBG[i];
			TH1D *h2_tmp = h2_mc[j]->ProjectionX(Form("h2_bgx_%d_%s", i, fSamples[index].sname.Data()), i, i);
			h2_tmp->SetFillColor(fSamples[index].color);
			hs2->Add(h2_tmp);
		}
		h1->SetXTitle(convertVarName("MuIso[0]"));
		h1->SetLineWidth(2);
		h1->SetLineColor(kBlack);
		h1->SetMarkerStyle(8);
		h1->SetMarkerColor(kBlack);
		// h1->SetFillColor(15);
		// h1->SetFillStyle(1001);

		// hs2->SetLineWidth(2);
		// hs2->SetLineColor(kBlue);
		// hs2->SetFillColor(kBlue);
		// hs2->SetFillStyle(3004);

		if(logy) gPad->SetLogy(1);
		gPad->SetFillStyle(0);
		h1->Sumw2();
		// hs2->Sumw2();

		// Scaling
		// if(h1->GetEntries() > 0 ) h1->Scale(1.0/h1->Integral());
		// if(hs2->GetEntries() > 0 ) hs2->Scale(1.0/h2->Integral());

		// setPlottingRange(h1, h2);

		// Determine plotting range
		double max1 = h1->GetMaximum();
		double max2 = hs2->GetMaximum();
		double max  = (max1>max2)?max1:max2;
		if(logy) max = 5*max;
		else max = 1.05*max;
		h1->SetMaximum(max);
		hs2->SetMaximum(max);
		if(!logy){
			h1->SetMinimum(0.0);
			hs2->SetMinimum(0.0);
		}

		TLegend *leg = new TLegend(0.45,0.75,0.65,0.88);
		if(i == 1){
			leg->AddEntry(h1,  "JM Data","f");
			leg->AddEntry(hs2, "All MC","f");
			leg->SetFillStyle(0);
			leg->SetTextFont(42);
			leg->SetBorderSize(0);
		}

		TH1D *h1_temp = (TH1D*)h1->DrawCopy("PE");
		hs2->Draw("same");
		h1_temp->SetName(Form("h1_%d",i));
		// hs2_temp->SetName(Form("hs2_%d",i));
		gPad->Update();
		TPaveStats *s1 = (TPaveStats*)h1_temp->GetListOfFunctions()->FindObject("stats");
		// TPaveStats *s2 = (TPaveStats*)hs2->GetListOfFunctions()->FindObject("stats");
		// s2->SetTextColor(kBlue); s2->SetLineColor(kBlue);
		// s2->SetY1NDC(s1->GetY1NDC() - (s1->GetY2NDC() - s1->GetY1NDC()));
		// s2->SetY2NDC(s1->GetY1NDC());

		if(i==1) leg->Draw();

		// double max1 = h1->GetYaxis()->GetXmax();
		// double max2 = h2->GetYaxis()->GetXmax();
		// double max  = (max1>max2)?max1:max2;
		double min1 = h1->GetYaxis()->GetXmin();
		double min2 = hs2->GetYaxis()->GetXmin();
		double min  = (min1<min2)?min1:min2;

		TLine *l1 = new TLine(0.15, min, 0.15, max);
		l1->SetLineColor(kRed);
		l1->SetLineWidth(2);
		l1->Draw();

		lat->SetTextColor(kBlack);
		lat->SetTextSize(0.05);
		lat->DrawLatex(0.11,0.92, Form("p_{T}(#mu) %3.0f - %3.0f GeV", getMuPt2Bins()[i-1], getMuPt2Bins()[i]));

		// int bin0 = h1->FindBin(0.00);
		// int bin15 = h1->FindBin(0.15);
		// int bin1 = h1->FindBin(1.00);
		// float f1 = h1->Integral(bin0, bin15) / h1->Integral(bin0, bin1);
		// float f2 = hs2->Integral(bin0, bin15) / hs2->Integral(bin0, bin1);
		// lat->SetTextSize(0.04);
		// lat->DrawLatex(0.55,0.905, Form("ratio = %4.2f", f1));
		// lat->SetTextColor(kBlue);
		// lat->DrawLatex(0.55,0.945, Form("ratio = %4.2f", f2));

		gPad->RedrawAxis();
	}
	Util::PrintNoEPS(c_temp, "IsoPlots", fOutputDir, fOutputFile);
}

//____________________________________________________________________________
void MuonPlotter::makeMuPtPlots(){
	const int nbins = 40;
	TH1D *h_prompt   = new TH1D("h_prompt",  "Pt for prompt Muons in WJets",                   nbins, 0., 100.);
	TH1D *h_fakew    = new TH1D("h_fakew",   "Pt for fake Muons in WJets events",              nbins, 0., 100.);
	TH1D *h_fakeqcd  = new TH1D("h_fakeqcd", "Pt for fake Muons in QCD events",                nbins, 0., 100.);
	TH1D *h_wtau     = new TH1D("h_wtau",    "Pt for fake Muons from tau in WJets events",     nbins, 0., 100.);
	TH1D *h_wnotau   = new TH1D("h_wnotau",  "Pt for fake Muons not from tau in WJets events", nbins, 0., 100.);
	TH1D *h_ttp      = new TH1D("h_ttp",     "Pt for prompt Muons in ttbar",                   nbins, 0., 100.);
	TH1D *h_ttp2     = new TH1D("h_ttp2",    "Pt for prompt Muons in ttbar",                   nbins, 0., 200.);
	TH1D *h_ttf      = new TH1D("h_ttf",     "Pt for non prompt Muons in ttbar",               nbins, 0., 100.);
	TH1D *h_ttftau   = new TH1D("h_ttftau",  "Pt for Muons from tau in ttbar",                 nbins, 0., 100.);
	TH1D *h_ttfnotau = new TH1D("h_ttfnotau","Pt for Muons not from tau in ttbar",             nbins, 0., 100.);
	TH1D *h_qcdb     = new TH1D("h_qcdb",   "Pt for muons from bottom hadrons in QCD",         nbins, 0., 100.);
	TH1D *h_qcdpik   = new TH1D("h_qcdpik", "Pt for muons from pions/kaons in QCD",            nbins, 0., 100.);
	TH1D *h_ttb      = new TH1D("h_ttb",    "Pt for muons from bottom hadrons in ttbar",       nbins, 0., 100.);
	TH1D *h_z        = new TH1D("h_z",      "Pt for muons from Z boson decays",                nbins, 0., 100.);

	h_fakeqcd->SetXTitle(convertVarName("MuPt[0]"));
	h_qcdb->SetXTitle(convertVarName("MuPt[0]"));
	h_prompt->SetXTitle(convertVarName("MuPt[0]"));
	h_ttp2->SetXTitle(convertVarName("MuPt[0]"));

	fSamples[6].tree->Project("h_prompt",  "MuPt[0]", "abs(MuGenMoID[0])==24");
	fSamples[5].tree->Project("h_z",       "MuPt[0]", "abs(MuGenMoID[0])==23");
	fSamples[1].tree->Project("h_fakew",   "MuPt[1]", "abs(MuGenMoID[1])!=24&&abs(MuGenGMoID[1])!=24");
	fSamples[0].tree->Project("h_fakeqcd", "MuPt[0]", "");
	fSamples[1].tree->Project("h_wtau",    "MuPt[1]", "abs(MuGenMoID[1])==15");
	fSamples[1].tree->Project("h_wnotau",  "MuPt[1]", "abs(MuGenMoID[1])!=24&&abs(MuGenMoID[1])!=15");
	fSamples[2].tree->Project("h_ttp",     "MuPt[0]", "abs(MuGenMoID[0])==24");
	fSamples[2].tree->Project("h_ttp2",    "MuPt[0]", "abs(MuGenMoID[0])==24");
	fSamples[2].tree->Project("h_ttf",     "MuPt[0]", "abs(MuGenMoID[0])!=24");
	fSamples[2].tree->Project("h_ttftau",  "MuPt[0]", "abs(MuGenMoID[0])==15");
	fSamples[2].tree->Project("h_ttfnotau","MuPt[0]", "abs(MuGenMoID[0])!=24&&abs(MuGenMoID[0])!=15");
	fSamples[2].tree->Project("h_ttb",     "MuPt[1]", "MuGenMoType[1]==15||MuGenMoType[1]==17||MuGenMoType[1]==21");
	fSamples[4].tree->Project("h_qcdb",    "MuPt[0]", "MuGenMoType[0]==15||MuGenMoType[0]==17||MuGenMoType[0]==21");
	fSamples[4].tree->Project("h_qcdpik",  "MuPt[0]", "MuGenMoType[0]==11||MuGenMoType[0]==12||MuGenMoType[0]==13");

	cout << h_fakeqcd->GetEntries() << " " << h_prompt->GetEntries() << " " << h_fakew->GetEntries() << endl;

	// h_ttp2->SetMinimum(0);
	printHisto(h_ttp2, "TTbarPt", "Pt of prompt ttbar Muons", "hist");

	plotOverlay3H(h_fakeqcd, "QCD",         h_prompt, "W-jets",       h_ttp,      "ttbar",         true);
	plotOverlay3H(h_z,       "Z-jets",      h_prompt, "W-jets",       h_ttp,      "ttbar",         true);
	plotOverlay3H(h_fakeqcd, "QCD",         h_wnotau, "W-jets",       h_ttfnotau, "ttbar",         true);

	// plotOverlay3H(h_fakeqcd, "Fake in QCD", h_prompt, "Prompt",       h_fakew,    "Fake in WJets", true);
	// plotOverlay3H(h_fakeqcd, "QCD",         h_wtau,   "W: tau",       h_wnotau,   "W: No tau",     true);
	// plotOverlay3H(h_prompt,  "Prompt W",    h_ttp,    "Prompt ttbar", h_ttftau,   "ttbar tau",     true);
	// plotOverlay3H(h_fakeqcd, "QCD",         h_wnotau, "W: no tau",    h_ttfnotau, "ttbar: no tau", true);
	// plotOverlay3H(h_qcdb,    "QCD: b",      h_qcdpik, "QCD: #pi/K",   h_ttb,      "ttbar: b",      true);

	// plotOverlay2H(h_fakeqcd, "QCD", h_fakew, "WJets", false, 0.15);
}

//____________________________________________________________________________
void MuonPlotter::makeMuIsoVsPtPlot(int sample1, int muon1, TCut c1, int sample2, int muon2, TCut c2, TString outputname, bool logy){
	const int nbins = 20;
	TH2D *h2_sig = new TH2D("h2_sig", "Isolation vs Pt for fake muons in signal", nbins, 0., 1., getNMuPtBins(), getMuPtBins());
	TH2D *h2_bg  = new TH2D("h2_bg", "Isolation vs Pt for muons in background",   nbins, 0., 1., getNMuPtBins(), getMuPtBins());
	h2_sig->SetXTitle(convertVarName("MuIso[0]"));
	h2_sig->SetYTitle(convertVarName("MuPt[0]"));
	h2_bg->SetXTitle(convertVarName("MuIso[0]"));
	h2_bg->SetYTitle(convertVarName("MuPt[0]"));

	fSamples[sample1].tree->Project("h2_bg", Form("MuPt[%d]:MuIso[%d]", muon1, muon1), c1);
	fSamples[sample2].tree->Project("h2_sig", Form("MuPt[%d]:MuIso[%d]", muon2, muon2), c2);

	TLatex *lat = new TLatex();
	lat->SetNDC(kTRUE);
	lat->SetTextColor(kBlack);
	lat->SetTextSize(0.06);

	TCanvas *c_temp = new TCanvas("IsoVsPt", "Isolating in Pt bins", 0, 0, 1200, 800);
	c_temp->Divide(3,2);
	c_temp->cd(6);
	if(logy) gPad->SetLogz(1);
	h2_bg->DrawCopy("colz");
	lat->DrawLatex(0.11,0.92, fSamples[sample1].sname);

	for(size_t i = 1; i <= 5; ++i){
		c_temp->cd(i);
		gStyle->SetOptStat(1111);
		TH1D *h1 = h2_bg->ProjectionX("h2_bgx",i, i);	
		TH1D *h2 = h2_sig->ProjectionX("h2_sigx",i, i);
		h1->SetXTitle(convertVarName("MuIso[0]"));
		h1->SetLineWidth(2);
		h1->SetFillColor(15);
		h1->SetFillStyle(1001);
		h2->SetLineWidth(2);
		h2->SetLineColor(kBlue);
		h2->SetFillColor(kBlue);
		h2->SetFillStyle(3004);

		if(logy) gPad->SetLogy(1);
		gPad->SetFillStyle(0);
		h1->Sumw2();
		h2->Sumw2();

		// Scaling
		if(h1->GetEntries() > 0 ) h1->Scale(1.0/h1->Integral());
		if(h2->GetEntries() > 0 ) h2->Scale(1.0/h2->Integral());

		// Determine plotting range
		double max1 = h1->GetMaximum();
		double max2 = h2->GetMaximum();
		double max  = (max1>max2)?max1:max2;
		if(logy) max = 5*max;
		else max = 1.05*max;
		h1->SetMaximum(max);
		h2->SetMaximum(max);
		if(!logy){
			h1->SetMinimum(0.0);
			h2->SetMinimum(0.0);
		}

		TLegend *leg = new TLegend(0.55,0.75,0.75,0.88);
		if(i == 1){
			leg->AddEntry(h1, fSamples[sample1].sname,"f");
			leg->AddEntry(h2, fSamples[sample2].sname,"f");
			leg->SetFillStyle(0);
			leg->SetTextFont(42);
			leg->SetBorderSize(0);
		}

		TH1D *h1_temp = (TH1D*)h1->DrawCopy("hist");
		TH1D *h2_temp = (TH1D*)h2->DrawCopy("histsames");
		gPad->Update();
		TPaveStats *s1 = (TPaveStats*)h1_temp->GetListOfFunctions()->FindObject("stats");
		TPaveStats *s2 = (TPaveStats*)h2_temp->GetListOfFunctions()->FindObject("stats");
		s2->SetTextColor(kBlue); s2->SetLineColor(kBlue);
		s2->SetY1NDC(s1->GetY1NDC() - (s1->GetY2NDC() - s1->GetY1NDC()));
		s2->SetY2NDC(s1->GetY1NDC());

		if(i==1) leg->Draw();

		double min1 = h1->GetYaxis()->GetXmin();
		double min2 = h2->GetYaxis()->GetXmin();
		double min  = (min1<min2)?min1:min2;

		TLine *l1 = new TLine(0.15, min, 0.15, max);
		l1->SetLineColor(kRed);
		l1->SetLineWidth(2);
		l1->Draw();

		lat->SetTextColor(kBlack);
		lat->SetTextSize(0.05);
		lat->DrawLatex(0.11,0.92, Form("p_{T}(#mu) %3.0f - %3.0f GeV", getMuPtBins()[i-1], getMuPtBins()[i]));

		int bin0 = h1->FindBin(0.00);
		int bin15 = h1->FindBin(0.15);
		int bin1 = h1->FindBin(1.00);
		float f1 = h1->Integral(bin0, bin15) / h1->Integral(bin0, bin1);
		float f2 = h2->Integral(bin0, bin15) / h2->Integral(bin0, bin1);
		lat->SetTextSize(0.04);
		lat->DrawLatex(0.55,0.905, Form("ratio = %4.2f", f1));
		lat->SetTextColor(kBlue);
		lat->DrawLatex(0.55,0.945, Form("ratio = %4.2f", f2));

		gPad->RedrawAxis();
	}
	Util::PrintNoEPS(c_temp, outputname, fOutputDir, fOutputFile);
}

//____________________________________________________________________________
void MuonPlotter::makeMuIsoVsPtPlot(int sample1, int muon1, bool(MuonPlotter::*eventSelector1)(), bool(MuonPlotter::*muonSelector1)(int), int sample2, int muon2, bool(MuonPlotter::*eventSelector2)(), bool(MuonPlotter::*muonSelector2)(int), TString outputname, bool logy){
	vector<int> samples1; samples1.push_back(sample1);
	vector<int> samples2; samples2.push_back(sample2);
	makeMuIsoVsPtPlot(samples1, muon1, eventSelector1, muonSelector1, samples2, muon2, eventSelector2, muonSelector2, outputname, logy);
}
void MuonPlotter::makeMuIsoVsPtPlot(vector<int> samples1, int muon1, bool(MuonPlotter::*eventSelector1)(), bool(MuonPlotter::*muonSelector1)(int), vector<int> samples2, int muon2, bool(MuonPlotter::*eventSelector2)(), bool(MuonPlotter::*muonSelector2)(int), TString outputname, bool logy){
	const int nbins = 30;
	TH2D *h2_sig = new TH2D("h2_sig", "Isolation vs Pt for muons in data",       nbins, 0., 1., getNMuPt2Bins(), getMuPt2Bins());
	TH2D *h2_bg  = new TH2D("h2_bg",  "Isolation vs Pt for fake muons in ttbar", nbins, 0., 1., getNMuPt2Bins(), getMuPt2Bins());
	h2_sig->SetXTitle(convertVarName("MuIso[0]"));
	h2_bg ->SetXTitle(convertVarName("MuIso[0]"));
	h2_sig->SetYTitle(convertVarName("MuPt[0]"));
	h2_bg ->SetYTitle(convertVarName("MuPt[0]"));
	
	TTree *tree = NULL;
	for(size_t i = 0; i < samples1.size(); ++i){
		int index = samples1[i];
	
		tree = fSamples[index].tree;
		float scale = fLumiNorm / fSamples[index].lumi;
		if(fSamples[index].isdata) scale = 1.;
		tree->ResetBranchAddresses();
		Init(tree);
		if (fChain == 0) return;
		Long64_t nentries = fChain->GetEntriesFast();
		Long64_t nbytes = 0, nb = 0;
		for (Long64_t jentry=0; jentry<nentries;jentry++) {
			Long64_t ientry = LoadTree(jentry);
			if (ientry < 0) break;
			nb = fChain->GetEntry(jentry);   nbytes += nb;

			if((*this.*eventSelector1)() == false) continue;
			if((*this.*muonSelector1)(muon1) == false) continue;
			h2_sig->Fill(MuIso[muon1], MuPt[muon1], scale);
		}
	}

	for(size_t i = 0; i < samples1.size(); ++i){
		int index = samples2[i];

		tree = fSamples[index].tree;
		float scale = fLumiNorm / fSamples[index].lumi;
		if(fSamples[index].isdata) scale = 1.;
		tree->ResetBranchAddresses();
		Init(tree);
		if (fChain == 0) return;
		Long64_t nentries = fChain->GetEntriesFast();
		Long64_t nbytes = 0, nb = 0;
		for (Long64_t jentry=0; jentry<nentries;jentry++) {
			Long64_t ientry = LoadTree(jentry);
			if (ientry < 0) break;
			nb = fChain->GetEntry(jentry);   nbytes += nb;

			if((*this.*eventSelector2)() == false) continue;
			if((*this.*muonSelector2)(muon2) == false) continue;
			h2_bg->Fill(MuIso[muon2], MuPt[muon2], scale);
		}
	}

	// fSamples[sample1].tree->Project("h2_bg", Form("MuPt[%d]:MuIso[%d]", muon1, muon1), c1);
	// fSamples[sample2].tree->Project("h2_sig", Form("MuPt[%d]:MuIso[%d]", muon2, muon2), c2);

	TLatex *lat = new TLatex();
	lat->SetNDC(kTRUE);
	lat->SetTextColor(kBlack);
	lat->SetTextSize(0.06);

	TCanvas *c_temp = new TCanvas("IsoVsPt", "Isolating in Pt bins", 0, 0, 1200, 800);
	c_temp->Divide(3,2);
	// TCanvas *c_temp = new TCanvas("IsoVsPt", "Isolating in Pt bins", 0, 0, 1200, 1200);
	// c_temp->Divide(3,3);
	// c_temp->cd(9);
	// if(logy) gPad->SetLogz(1);
	// h2_sig->DrawCopy("colz");
	// lat->DrawLatex(0.11,0.92, fSamples[TTbar].sname);

	for(unsigned i = 1; i <= getNMuPt2Bins(); ++i){
		c_temp->cd(i);
		gStyle->SetOptStat(1111);
		TH1D *h1 = h2_sig->ProjectionX(Form("h2_sigx_%d", i), i, i);	
		TH1D *h2 = h2_bg ->ProjectionX(Form("h2_bgx_%d",  i), i, i);
		h1->SetXTitle(convertVarName("MuIso[0]"));
		h1->SetLineWidth(2);
		h1->SetFillColor(15);
		h1->SetFillStyle(1001);
		h2->SetLineWidth(2);
		h2->SetLineColor(kBlue);
		h2->SetFillColor(kBlue);
		h2->SetFillStyle(3004);

		if(logy) gPad->SetLogy(1);
		gPad->SetFillStyle(0);
		h1->Sumw2();
		h2->Sumw2();

		// Scaling
		if(h1->GetEntries() > 0 ) h1->Scale(1.0/h1->Integral());
		if(h2->GetEntries() > 0 ) h2->Scale(1.0/h2->Integral());

		// setPlottingRange(h1, h2);

		// Determine plotting range
		double max1 = h1->GetMaximum();
		double max2 = h2->GetMaximum();
		double max  = (max1>max2)?max1:max2;
		if(logy) max = 5*max;
		else max = 1.05*max;
		h1->SetMaximum(max);
		h2->SetMaximum(max);
		if(!logy){
			h1->SetMinimum(0.0);
			h2->SetMinimum(0.0);
		}

		TLegend *leg = new TLegend(0.45,0.75,0.65,0.88);
		if(i == 1){
			leg->AddEntry(h1, fSamples[TTbar].sname,"f");
			leg->AddEntry(h2, fSamples[MuB].sname,"f");
			leg->SetFillStyle(0);
			leg->SetTextFont(42);
			leg->SetBorderSize(0);
		}

		TH1D *h1_temp = (TH1D*)h1->DrawCopy("hist");
		TH1D *h2_temp = (TH1D*)h2->DrawCopy("histsames");
		h1_temp->SetName(Form("h1_%d",i));
		h2_temp->SetName(Form("h2_%d",i));
		gPad->Update();
		TPaveStats *s1 = (TPaveStats*)h1_temp->GetListOfFunctions()->FindObject("stats");
		TPaveStats *s2 = (TPaveStats*)h2_temp->GetListOfFunctions()->FindObject("stats");
		s2->SetTextColor(kBlue); s2->SetLineColor(kBlue);
		s2->SetY1NDC(s1->GetY1NDC() - (s1->GetY2NDC() - s1->GetY1NDC()));
		s2->SetY2NDC(s1->GetY1NDC());

		if(i==1) leg->Draw();

		// double max1 = h1->GetYaxis()->GetXmax();
		// double max2 = h2->GetYaxis()->GetXmax();
		// double max  = (max1>max2)?max1:max2;
		double min1 = h1->GetYaxis()->GetXmin();
		double min2 = h2->GetYaxis()->GetXmin();
		double min  = (min1<min2)?min1:min2;

		TLine *l1 = new TLine(0.15, min, 0.15, max);
		l1->SetLineColor(kRed);
		l1->SetLineWidth(2);
		l1->Draw();

		lat->SetTextColor(kBlack);
		lat->SetTextSize(0.05);
		lat->DrawLatex(0.11,0.92, Form("p_{T}(#mu) %3.0f - %3.0f GeV", getMuPt2Bins()[i-1], getMuPt2Bins()[i]));

		int bin0 = h1->FindBin(0.00);
		int bin15 = h1->FindBin(0.15);
		int bin1 = h1->FindBin(1.00);
		float f1 = h1->Integral(bin0, bin15) / h1->Integral(bin0, bin1);
		float f2 = h2->Integral(bin0, bin15) / h2->Integral(bin0, bin1);
		lat->SetTextSize(0.04);
		lat->DrawLatex(0.55,0.905, Form("ratio = %4.2f", f1));
		lat->SetTextColor(kBlue);
		lat->DrawLatex(0.55,0.945, Form("ratio = %4.2f", f2));

		gPad->RedrawAxis();
	}
	Util::PrintNoEPS(c_temp, outputname, fOutputDir, fOutputFile);
}

//____________________________________________________________________________
void MuonPlotter::makeMuIsoVsNJetsPlot(int sample1, int muon1, TCut c1, int sample2, int muon2, TCut c2, TString outputname, bool logy){
	const int nbins = 20;
	const int Nnjetbins = 4;
	const double njetbins[Nnjetbins+1] = {0.,1.,2.,3.,4.};
	TH2D *h2_sig = new TH2D("h2_sig", "Isolation vs NJets for fake muons in signal", nbins, 0., 1., Nnjetbins, njetbins);
	TH2D *h2_bg  = new TH2D("h2_bg", "Isolation vs NJets for muons in background",   nbins, 0., 1., Nnjetbins, njetbins);
	h2_sig->SetXTitle(convertVarName("MuIso[0]"));
	h2_sig->SetYTitle("NJets");
	h2_bg->SetXTitle(convertVarName("MuIso[0]"));
	h2_bg->SetYTitle("NJets");

	fSamples[sample1].tree->Project("h2_bg",  Form("NJets:MuIso[%d]", muon1), c1);
	fSamples[sample2].tree->Project("h2_sig", Form("NJets:MuIso[%d]", muon2), c2);


	TLatex *lat = new TLatex();
	lat->SetNDC(kTRUE);
	lat->SetTextSize(0.06);
	lat->SetTextColor(kBlack);

	TCanvas *c_temp = new TCanvas("IsoVsNJets", "Isolating in bins of jet multiplicity", 0, 0, 1200, 800);
	c_temp->Divide(3,2);
	c_temp->cd(5);
	if(logy) gPad->SetLogz(1);
	h2_sig->DrawCopy("colz");
	lat->DrawLatex(0.11,0.92, fSamples[sample2].sname);
	c_temp->cd(6);
	if(logy) gPad->SetLogz(1);
	h2_bg->DrawCopy("colz");
	lat->DrawLatex(0.11,0.92, fSamples[sample1].sname);

	for(size_t i = 1; i <= Nnjetbins; ++i){
		c_temp->cd(i);
		gStyle->SetOptStat(1111);
		int lastbin = i;
		if(i == Nnjetbins) lastbin = i+1;
		TH1D *h1 = h2_bg->ProjectionX("h2_bgx",i, lastbin);
		TH1D *h2 = h2_sig->ProjectionX("h2_sigx",i, lastbin);
		h1->SetXTitle(convertVarName("MuIso[0]"));
		h1->SetLineWidth(2);
		h1->SetFillColor(15);
		h1->SetFillStyle(1001);
		h2->SetLineWidth(2);
		h2->SetLineColor(kBlue);
		h2->SetFillColor(kBlue);
		h2->SetFillStyle(3004);

		if(logy) gPad->SetLogy(1);
		gPad->SetFillStyle(0);
		h1->Sumw2();
		h2->Sumw2();
		if(h1->GetEntries() > 0 ) h1->Scale(1.0/h1->Integral());
		if(h2->GetEntries() > 0 ) h2->Scale(1.0/h2->Integral());

		// Determine plotting range
		double max1 = h1->GetMaximum();
		double max2 = h2->GetMaximum();
		double max  = (max1>max2)?max1:max2;
		if(logy) max = 5*max;
		else max = 1.05*max;
		h1->SetMaximum(max);
		h2->SetMaximum(max);
		if(!logy){
			h1->SetMinimum(0.0);
			h2->SetMinimum(0.0);
		}

		TLegend *leg = new TLegend(0.55,0.75,0.75,0.88);
		if(i == 1){
			leg->AddEntry(h1, fSamples[sample1].sname,"f");
			leg->AddEntry(h2, fSamples[sample2].sname,"f");
			leg->SetFillStyle(0);
			leg->SetTextFont(42);
			leg->SetBorderSize(0);
		}

		TH1D *h1_temp = (TH1D*)h1->DrawCopy("hist");
		TH1D *h2_temp = (TH1D*)h2->DrawCopy("histsames");
		gPad->Update();
		TPaveStats *s1 = (TPaveStats*)h1_temp->GetListOfFunctions()->FindObject("stats");
		TPaveStats *s2 = (TPaveStats*)h2_temp->GetListOfFunctions()->FindObject("stats");
		s2->SetTextColor(kBlue); s2->SetLineColor(kBlue);
		s2->SetY1NDC(s1->GetY1NDC() - (s1->GetY2NDC() - s1->GetY1NDC()));
		s2->SetY2NDC(s1->GetY1NDC());

		if(i==1) leg->Draw();

		double min1 = h1->GetYaxis()->GetXmin();
		double min2 = h2->GetYaxis()->GetXmin();
		double min  = (min1<min2)?min1:min2;

		TLine *l1 = new TLine(0.15, min, 0.15, max);
		l1->SetLineColor(kRed);
		l1->SetLineWidth(2);
		l1->Draw();

		lat->SetTextColor(kBlack);
		lat->SetTextSize(0.05);
		if(i < 4) lat->DrawLatex(0.11,0.92, Form("NJets = %1.0f",  njetbins[i-1]));
		else      lat->DrawLatex(0.11,0.92, Form("NJets >= %1.0f", njetbins[i-1]));

		int bin0 = h1->FindBin(0.00);
		int bin15 = h1->FindBin(0.15);
		int bin1 = h1->FindBin(1.00);
		float f1 = h1->Integral(bin0, bin15) / h1->Integral(bin0, bin1);
		float f2 = h2->Integral(bin0, bin15) / h2->Integral(bin0, bin1);
		lat->SetTextSize(0.04);
		lat->DrawLatex(0.55,0.905, Form("ratio = %4.2f", f1));
		lat->SetTextColor(kBlue);
		lat->DrawLatex(0.55,0.945, Form("ratio = %4.2f", f2));

		gPad->RedrawAxis();
	}
	Util::PrintNoEPS(c_temp, outputname, fOutputDir, fOutputFile);
}

//____________________________________________________________________________
void MuonPlotter::produceMuRatio(int sample, int muon, bool(MuonPlotter::*eventSelector)(), bool(MuonPlotter::*muonSelector)(int), TH2D *&h_2d, TH1D *&h_pt, TH1D *&h_eta, bool output){
	vector<int> samples; samples.push_back(sample);
	produceMuRatio(samples, muon, eventSelector, muonSelector, h_2d, h_pt, h_eta, output);
}
void MuonPlotter::produceMuRatio(vector<int> samples, int muon, bool(MuonPlotter::*eventSelector)(), bool(MuonPlotter::*muonSelector)(int), TH2D *&h_2d, TH1D *&h_pt, TH1D *&h_eta, bool output){
// Base function for production of all ratios
/*
	TODO Fix treatment of statistical errors and luminosity scaling here!
*/
	gStyle->SetOptStat(0);
	h_2d->Sumw2();
	h_pt->Sumw2();
	h_eta->Sumw2();

	TH2D *H_ntight = new TH2D("MuNTight", "NTight Muons", h_2d->GetNbinsX(), h_2d->GetXaxis()->GetXbins()->GetArray(), h_2d->GetNbinsY(),  h_2d->GetYaxis()->GetXbins()->GetArray());
	TH2D *H_nloose = new TH2D("MuNLoose", "NLoose Muons", h_2d->GetNbinsX(), h_2d->GetXaxis()->GetXbins()->GetArray(), h_2d->GetNbinsY(),  h_2d->GetYaxis()->GetXbins()->GetArray());
	H_ntight->Sumw2();
	H_nloose->Sumw2();

	if(fVerbose>2) cout << "---------------" << endl;
	for(size_t i = 0; i < samples.size(); ++i){
		int sample = samples[i];
		TTree *tree = fSamples[sample].tree;
		if(fVerbose>2) cout << "Producing ratios for " << fSamples[sample].sname << endl;
		tree->ResetBranchAddresses();
		Init(tree);
		if (fChain == 0) return;
		Long64_t nentries = fChain->GetEntriesFast();

		float scale = fLumiNorm / fSamples[sample].lumi;
		if(fSamples[sample].isdata) scale = 1;

		Long64_t nbytes = 0, nb = 0;
		for (Long64_t jentry=0; jentry<nentries;jentry++) {
			Long64_t ientry = LoadTree(jentry);
			if (ientry < 0) break;
			nb = fChain->GetEntry(jentry);   nbytes += nb;
			printProgress(jentry, nentries, fSamples[sample].name);

			if((*this.*eventSelector)() == false) continue;
			if((*this.*muonSelector)(muon) == false) continue;

			if(isLooseMuon(muon)) H_nloose->Fill(MuPt[muon], MuEta[muon], scale); // Tight or loose
			if(isTightMuon(muon)) H_ntight->Fill(MuPt[muon], MuEta[muon], scale); // Tight
		}
		cout << endl;

		if(fVerbose>2) cout << " Tight entries so far: " << H_ntight->GetEntries() << " / " << H_ntight->Integral() << endl;
		if(fVerbose>2) cout << " Loose entries so far: " << H_nloose->GetEntries() << " / " << H_nloose->Integral() << endl;
		if(fVerbose>2) cout << "  Ratio: " << (double)H_ntight->GetEntries()/(double)H_nloose->GetEntries() << endl;
	}
	h_2d->Divide(H_ntight, H_nloose);

	TH1D *hmuloosept  = H_nloose->ProjectionX();
	TH1D *hmulooseeta = H_nloose->ProjectionY();
	TH1D *hmutightpt  = H_ntight->ProjectionX();
	TH1D *hmutighteta = H_ntight->ProjectionY();
	h_pt ->Divide(hmutightpt, hmuloosept);
	h_eta->Divide(hmutighteta, hmulooseeta);

	h_pt ->SetXTitle(convertVarName("MuPt[1]"));
	h_eta->SetXTitle(convertVarName("MuEta[1]"));
	h_pt ->SetYTitle("# Tight / # Loose");
	h_eta->SetYTitle("# Tight / # Loose");
	h_2d->SetXTitle(convertVarName("MuPt[1]"));
	h_2d->SetYTitle(convertVarName("MuEta[1]"));
	TString name = "";
	for(size_t i = 0; i < samples.size(); ++i){
		int sample = samples[i];
		name += h_2d->GetName();
		name += "_";
		name += fSamples[sample].sname;
	}
	if(output){
		printHisto(h_2d,  TString("MuRatio")    + name, "Muon Fake Ratio vs pt/eta", "colz");
		printHisto(h_pt,  TString("MuRatioPt")  + name, "Muon Fake Ratio vs pt",     "PE1");
		printHisto(h_eta, TString("MuRatioEta") + name, "Muon Fake Ratio vs eta",    "PE1");
	}
	delete H_ntight, H_nloose, hmuloosept, hmulooseeta, hmutightpt, hmutighteta;
}

//____________________________________________________________________________
void MuonPlotter::produceElRatio(int sample, int eleind, bool(MuonPlotter::*eventSelector)(), bool(MuonPlotter::*eleSelector)(int), TH2D *&h_2d, TH1D *&h_pt, TH1D *&h_eta, bool output){
	vector<int> samples; samples.push_back(sample);
	produceElRatio(samples, eleind, eventSelector, eleSelector, h_2d, h_pt, h_eta, output);
}
void MuonPlotter::produceElRatio(vector<int> samples, int eleind, bool(MuonPlotter::*eventSelector)(), bool(MuonPlotter::*eleSelector)(int), TH2D *&h_2d, TH1D *&h_pt, TH1D *&h_eta, bool output){
// Base function for production of all ratios
/*
	TODO Fix treatment of statistical errors and luminosity scaling here!
*/
	gStyle->SetOptStat(0);
	h_2d->Sumw2();
	h_pt->Sumw2();
	h_eta->Sumw2();

	TH2D *H_ntight = new TH2D("ElNTight", "NTight Electrons", h_2d->GetNbinsX(), h_2d->GetXaxis()->GetXbins()->GetArray(), h_2d->GetNbinsY(),  h_2d->GetYaxis()->GetXbins()->GetArray());
	TH2D *H_nloose = new TH2D("ElNLoose", "NLoose Electrons", h_2d->GetNbinsX(), h_2d->GetXaxis()->GetXbins()->GetArray(), h_2d->GetNbinsY(),  h_2d->GetYaxis()->GetXbins()->GetArray());
	H_ntight->Sumw2();
	H_nloose->Sumw2();

	if(fVerbose>2) cout << "---------------" << endl;
	for(size_t i = 0; i < samples.size(); ++i){
		int sample = samples[i];
		TTree *tree = fSamples[sample].tree;
		if(fVerbose>2) cout << "Producing ratios for " << fSamples[sample].sname << endl;
		tree->ResetBranchAddresses();
		Init(tree);
		if (fChain == 0) return;
		Long64_t nentries = fChain->GetEntriesFast();

		float scale = fLumiNorm / fSamples[sample].lumi;
		if(fSamples[sample].isdata) scale = 1;

		Long64_t nbytes = 0, nb = 0;
		for (Long64_t jentry=0; jentry<nentries;jentry++) {
			Long64_t ientry = LoadTree(jentry);
			if (ientry < 0) break;
			nb = fChain->GetEntry(jentry);   nbytes += nb;
			printProgress(jentry, nentries, fSamples[sample].name);

			if((*this.*eventSelector)() == false) continue;
			if((*this.*eleSelector)(eleind) == false) continue;

			if(isLooseElectron(eleind)) H_nloose->Fill(ElPt[eleind], ElEta[eleind], scale); // Tight or loose
			if(isTightElectron(eleind)) H_ntight->Fill(ElPt[eleind], ElEta[eleind], scale); // Tight
		}
		cout << endl;

		if(fVerbose>2) cout << " Tight entries so far: " << H_ntight->GetEntries() << " / " << H_ntight->Integral() << endl;
		if(fVerbose>2) cout << " Loose entries so far: " << H_nloose->GetEntries() << " / " << H_nloose->Integral() << endl;
		if(fVerbose>2) cout << "  Ratio: " << (double)H_ntight->GetEntries()/(double)H_nloose->GetEntries() << endl;
	}
	h_2d->Divide(H_ntight, H_nloose);

	TH1D *hloosept  = H_nloose->ProjectionX();
	TH1D *hlooseeta = H_nloose->ProjectionY();
	TH1D *htightpt  = H_ntight->ProjectionX();
	TH1D *htighteta = H_ntight->ProjectionY();
	h_pt ->Divide(htightpt,  hloosept);
	h_eta->Divide(htighteta, hlooseeta);

	h_pt ->SetXTitle(convertVarName("ElPt[1]"));
	h_eta->SetXTitle(convertVarName("ElEta[1]"));
	h_pt ->SetYTitle("# Tight / # Loose");
	h_eta->SetYTitle("# Tight / # Loose");
	h_2d->SetXTitle(convertVarName("ElPt[1]"));
	h_2d->SetYTitle(convertVarName("ElEta[1]"));
	TString name = "";
	for(size_t i = 0; i < samples.size(); ++i){
		int sample = samples[i];
		name += h_2d->GetName();
		name += "_";
		name += fSamples[sample].sname;
	}
	if(output){
		printHisto(h_2d,  TString("ElRatio")    + name, "Electron Fake Ratio vs pt/eta", "colz");
		printHisto(h_pt,  TString("ElRatioPt")  + name, "Electron Fake Ratio vs pt",     "PE1");
		printHisto(h_eta, TString("ElRatioEta") + name, "Electron Fake Ratio vs eta",    "PE1");
	}
	delete H_ntight, H_nloose, hloosept, hlooseeta, htightpt, htighteta;
}

//____________________________________________________________________________
void MuonPlotter::fillMufRatio(int sample, int muon){          fillMufRatio(sample,  muon, getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins()); }
void MuonPlotter::fillMufRatio(vector<int> samples, int muon){ fillMufRatio(samples, muon, getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins()); }
void MuonPlotter::fillMufRatio(int sample, int muon, const int nptbins, const double *ptbins, const int netabins, const double *etabins){
	vector<int> samples; samples.push_back(sample);
	fillMufRatio(samples, muon, nptbins, ptbins, netabins, etabins);
}
void MuonPlotter::fillMufRatio(vector<int> samples, int muon, const int nptbins, const double *ptbins, const int netabins, const double *etabins){
	gStyle->SetOptStat(0);
	fH2D_MufRatio    = new TH2D("MufRatio", "Ratio of tight to loose Muons vs Pt vs Eta", nptbins, ptbins, netabins, etabins);
	fH1D_MufRatioPt  = new TH1D("MufRatioPt",  "Ratio of tight to loose Muons vs Pt",     nptbins, ptbins);
	fH1D_MufRatioEta = new TH1D("MufRatioEta", "Ratio of tight to loose Muons vs Eta",    netabins, etabins);

	fH1D_MufRatioPt->SetXTitle(convertVarName("MuPt[0]"));
	fH1D_MufRatioEta->SetXTitle(convertVarName("MuEta[0]"));
	fH2D_MufRatio->SetXTitle(convertVarName("MuPt[0]"));
	fH2D_MufRatio->SetYTitle(convertVarName("MuEta[0]"));
	fH1D_MufRatioPt ->SetYTitle("# Tight / # Loose");
	fH1D_MufRatioEta->SetYTitle("# Tight / # Loose");

	produceMuRatio(samples, muon, &MuonPlotter::isSigSupMuEventTRG, &MuonPlotter::isLooseMuon, fH2D_MufRatio, fH1D_MufRatioPt, fH1D_MufRatioEta, true);
	// calculateRatio(samples, Muon, 1, fH2D_MufRatio, fH1D_MufRatioPt, fH1D_MufRatioEta, false);
}

//____________________________________________________________________________
void MuonPlotter::fillMupRatio(int sample, int muon){          fillMupRatio(sample,  muon, getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins()); }
void MuonPlotter::fillMupRatio(vector<int> samples, int muon){ fillMupRatio(samples, muon, getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins()); }
void MuonPlotter::fillMupRatio(int sample, int muon, const int nptbins, const double *ptbins, const int netabins, const double *etabins){
	vector<int> samples; samples.push_back(sample);
	fillMupRatio(samples, muon, nptbins, ptbins, netabins, etabins);
}
void MuonPlotter::fillMupRatio(vector<int> samples, int muon, const int nptbins, const double *ptbins, const int netabins, const double *etabins){
	gStyle->SetOptStat(0);
	// This is supposed to be run on a ZJets selection!
	fH2D_MupRatio    = new TH2D("MupRatio",    "Ratio of tight to loose Muons vs Pt vs Eta", nptbins, ptbins, netabins, etabins);
	fH1D_MupRatioPt  = new TH1D("MupRatioPt",  "Ratio of tight to loose Muons vs Pt", nptbins, ptbins);
	fH1D_MupRatioEta = new TH1D("MupRatioEta", "Ratio of tight to loose Muons vs Eta", netabins, etabins);
	fH1D_MupRatioPt->SetXTitle(convertVarName("MuPt[0]"));
	fH1D_MupRatioEta->SetXTitle(convertVarName("MuEta[0]"));
	fH2D_MupRatio->SetXTitle(convertVarName("MuPt[0]"));
	fH2D_MupRatio->SetYTitle(convertVarName("MuEta[0]"));
	fH1D_MupRatioPt ->SetYTitle("# Tight / # Loose");
	fH1D_MupRatioEta->SetYTitle("# Tight / # Loose");

	produceMuRatio(samples, muon, &MuonPlotter::isZMuMuEventTRG, &MuonPlotter::isLooseMuon, fH2D_MupRatio, fH1D_MupRatioPt, fH1D_MupRatioEta, true);
	// calculateRatio(samples, Muon, 2, fH2D_MupRatio, fH1D_MupRatioPt, fH1D_MupRatioEta, false);
}

//____________________________________________________________________________
void MuonPlotter::plotMuRatio(int sample, int muon, bool(MuonPlotter::*eventSelector)(), bool(MuonPlotter::*muonSelector)(int), TString tag){
	vector<int> samples; samples.push_back(sample);
	plotMuRatio(samples, muon, eventSelector, muonSelector, tag);
}
void MuonPlotter::plotMuRatio(vector<int> samples, int muon, bool(MuonPlotter::*eventSelector)(), bool(MuonPlotter::*muonSelector)(int), TString tag){
	gStyle->SetOptStat(0);
	TH2D *h_2d  = new TH2D("MuRatio",    Form("Ratio of tight to loose Muons vs Pt vs Eta : %s", tag.Data()), getNMuPtBins(), getMuPtBins(), getNMuEtaBins(), getMuEtaBins());
	TH1D *h_pt  = new TH1D("MuRatioPt",  Form("Ratio of tight to loose Muons vs Pt : %s",        tag.Data()), getNMuPtBins(), getMuPtBins());
	TH1D *h_eta = new TH1D("MuRatioEta", Form("Ratio of tight to loose Muons vs Eta : %s",       tag.Data()), getNMuEtaBins(), getMuEtaBins());
	h_pt->SetXTitle(convertVarName("MuPt[0]"));
	h_eta->SetXTitle(convertVarName("MuEta[0]"));
	h_2d->SetXTitle(convertVarName("MuPt[0]"));
	h_2d->SetYTitle(convertVarName("MuEta[0]"));
	h_pt ->SetYTitle("# Tight / # Loose");
	h_eta->SetYTitle("# Tight / # Loose");
	h_pt->GetYaxis()->SetTitleOffset(1.2);
	h_eta->GetYaxis()->SetTitleOffset(1.2);

	produceMuRatio(samples, muon, eventSelector, muonSelector, h_2d, h_pt, h_eta, true);
}

//____________________________________________________________________________
TH1D* MuonPlotter::fillMuRatioPt(int sample, int muon, bool(MuonPlotter::*eventSelector)(), bool(MuonPlotter::*muonSelector)(int), bool output){
	vector<int> samples; samples.push_back(sample);
	return fillMuRatioPt(samples, muon, eventSelector, muonSelector, output);
}
TH1D* MuonPlotter::fillMuRatioPt(vector<int> samples, int muon, bool(MuonPlotter::*eventSelector)(), bool(MuonPlotter::*muonSelector)(int), bool output){
	gStyle->SetOptStat(0);
	TH2D *h_2d  = new TH2D("MuRatio",    "Ratio of tight to loose Muons vs Pt vs Eta", getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins());
	TH1D *h_pt  = new TH1D("MuRatioPt",  "Ratio of tight to loose Muons vs Pt",        getNMuPt2Bins(), getMuPt2Bins());
	TH1D *h_eta = new TH1D("MuRatioEta", "Ratio of tight to loose Muons vs Eta",       getNMuEtaBins(), getMuEtaBins());

	h_pt->SetXTitle(convertVarName("MuPt[0]"));
	h_pt ->SetYTitle("# Tight / # Loose");
	h_pt->GetYaxis()->SetTitleOffset(1.2);

	produceMuRatio(samples, muon, eventSelector, muonSelector, h_2d, h_pt, h_eta, output);
	return h_pt;
}
TH1D* MuonPlotter::fillMuRatioPt(vector<int> samples, int muon, bool(MuonPlotter::*eventSelector)(), bool(MuonPlotter::*muonSelector)(int), const int nptbins, const double* ptbins, const int netabins, const double* etabins, bool output){
	gStyle->SetOptStat(0);
	TH2D *h_2d  = new TH2D("MuRatio",    "Ratio of tight to loose Muons vs Pt vs Eta", nptbins, ptbins, netabins, etabins);
	TH1D *h_pt  = new TH1D("MuRatioPt",  "Ratio of tight to loose Muons vs Pt",        nptbins, ptbins);
	TH1D *h_eta = new TH1D("MuRatioEta", "Ratio of tight to loose Muons vs Eta",       netabins, etabins);

	h_pt->SetXTitle(convertVarName("MuPt[0]"));
	h_pt ->SetYTitle("# Tight / # Loose");
	h_pt->GetYaxis()->SetTitleOffset(1.2);

	produceMuRatio(samples, muon, eventSelector, muonSelector, h_2d, h_pt, h_eta, output);
	return h_pt;
}

//____________________________________________________________________________
TH2D* MuonPlotter::fillMuRatio(int sample, int muon, bool(MuonPlotter::*eventSelector)(), bool(MuonPlotter::*muonSelector)(int), const int nptbins, const double* ptbins, const int netabins, const double* etabins){
	vector<int> samples; samples.push_back(sample);
	return fillMuRatio(samples, muon, eventSelector, muonSelector, nptbins, ptbins, netabins, etabins);
}
TH2D* MuonPlotter::fillMuRatio(vector<int> samples, int muon, bool(MuonPlotter::*eventSelector)(), bool(MuonPlotter::*muonSelector)(int), const int nptbins, const double* ptbins, const int netabins, const double* etabins){
	gStyle->SetOptStat(0);
	TH2D *h_2d  = new TH2D("MuRatio",    "Ratio of tight to loose Muons vs Pt vs Eta", nptbins, ptbins, netabins, etabins);
	TH1D *h_pt  = new TH1D("MuRatioPt",  "Ratio of tight to loose Muons vs Pt",        nptbins, ptbins);
	TH1D *h_eta = new TH1D("MuRatioEta", "Ratio of tight to loose Muons vs Eta",       netabins, etabins);

	h_2d->SetXTitle(convertVarName("MuPt[0]"));
	h_2d->SetYTitle(convertVarName("MuEta[0]"));

	produceMuRatio(samples, muon, eventSelector, muonSelector, h_2d, h_pt, h_eta, false);
	return h_2d;
}

//____________________________________________________________________________
TH1D* MuonPlotter::fillMuRatioPt(int sample, int forp, bool output){
	vector<int> samples; samples.push_back(sample);
	return fillMuRatioPt(samples, forp, output);
}
TH1D* MuonPlotter::fillMuRatioPt(vector<int> samples, int forp, bool output){
	gStyle->SetOptStat(0);
	TH2D *h_2d  = new TH2D("MuRatio",    "Ratio of tight to loose Muons vs Pt vs Eta", getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins());
	TH1D *h_pt  = new TH1D("MuRatioPt",  "Ratio of tight to loose Muons vs Pt",        getNMuPt2Bins(), getMuPt2Bins());
	TH1D *h_eta = new TH1D("MuRatioEta", "Ratio of tight to loose Muons vs Eta",       getNMuEtaBins(), getMuEtaBins());

	h_pt->SetXTitle(convertVarName("MuPt[0]"));
	h_pt->SetYTitle("# Tight / # Loose");
	h_pt->GetYaxis()->SetTitleOffset(1.2);

	calculateRatio(samples, Muon, forp, h_2d, h_pt, h_eta, output);
	return h_pt;
};

//____________________________________________________________________________
TH1D* MuonPlotter::fillElRatioPt(int sample, int forp, bool output){
	vector<int> samples; samples.push_back(sample);
	return fillMuRatioPt(samples, forp, output);
}
TH1D* MuonPlotter::fillElRatioPt(vector<int> samples, int forp, bool output){
	gStyle->SetOptStat(0);
	TH2D *h_2d  = new TH2D("ElRatio",    "Ratio of tight to loose Electrons vs Pt vs Eta", getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins());
	TH1D *h_pt  = new TH1D("ElRatioPt",  "Ratio of tight to loose Electrons vs Pt",        getNMuPt2Bins(), getMuPt2Bins());
	TH1D *h_eta = new TH1D("ElRatioEta", "Ratio of tight to loose Electrons vs Eta",       getNMuEtaBins(), getMuEtaBins());

	h_pt->SetXTitle(convertVarName("ElPt[0]"));
	h_pt->SetYTitle("# Tight / # Loose");
	h_pt->GetYaxis()->SetTitleOffset(1.2);

	calculateRatio(samples, Electron, forp, h_2d, h_pt, h_eta, output);
	return h_pt;
};

//____________________________________________________________________________
void MuonPlotter::calculateRatio(vector<int> samples, gChannel chan, int forp, TH2D*& h_2d, bool output){
	TH1D *h_dummy1 = new TH1D("dummy1", "dummy1", 1, 0.,1.);
	TH1D *h_dummy2 = new TH1D("dummy2", "dummy2", 1, 0.,1.);
	calculateRatio(samples, chan, forp, h_2d, h_dummy1, h_dummy2, output);
	delete h_dummy1, h_dummy2;
}
void MuonPlotter::calculateRatio(vector<int> samples, gChannel chan, int forp, TH2D*& h_2d, TH1D*& h_pt, TH1D*&h_eta, bool output){
/*
TODO Fix treatment of statistical errors and luminosity scaling here!
*/
	gStyle->SetOptStat(0);
	h_2d->Sumw2();
	h_pt->Sumw2();
	h_eta->Sumw2();

	TH2D *H_ntight = new TH2D("NTight", "NTight Muons", h_2d->GetNbinsX(), h_2d->GetXaxis()->GetXbins()->GetArray(), h_2d->GetNbinsY(),  h_2d->GetYaxis()->GetXbins()->GetArray());
	TH2D *H_nloose = new TH2D("NLoose", "NLoose Muons", h_2d->GetNbinsX(), h_2d->GetXaxis()->GetXbins()->GetArray(), h_2d->GetNbinsY(),  h_2d->GetYaxis()->GetXbins()->GetArray());
	H_ntight->Sumw2();
	H_nloose->Sumw2();

	if(fVerbose>2) cout << "---------------" << endl;
	for(size_t i = 0; i < samples.size(); ++i){
		int sample = samples[i];
		TTree *tree = fSamples[sample].tree;
		if(fVerbose>2) cout << "Calculating ratios for " << fSamples[sample].sname << endl;

		float scale = fLumiNorm / fSamples[sample].lumi;

		channel *cha;
		if(chan == Muon)     cha = &fSamples[sample].mumu;
		if(chan == Electron) cha = &fSamples[sample].ee;

		if(forp == 1){
			H_ntight->Add(cha->fhistos.h_ntight, scale);
			H_nloose->Add(cha->fhistos.h_nloose, scale);			
		}
		if(forp == 2){
			H_ntight->Add(cha->phistos.h_ntight, scale);
			H_nloose->Add(cha->phistos.h_nloose, scale);			
		}

		if(fVerbose>2) cout << " Tight entries so far: " << H_ntight->GetEntries() << " / " << H_ntight->Integral() << endl;
		if(fVerbose>2) cout << " Loose entries so far: " << H_nloose->GetEntries() << " / " << H_nloose->Integral() << endl;
		if(fVerbose>2) cout << "  Ratio so far       : " << (double)H_ntight->GetEntries()/(double)H_nloose->GetEntries() << endl;
	}
	h_2d->Divide(H_ntight, H_nloose);

	TH1D *hmuloosept  = H_nloose->ProjectionX();
	TH1D *hmulooseeta = H_nloose->ProjectionY();
	TH1D *hmutightpt  = H_ntight->ProjectionX();
	TH1D *hmutighteta = H_ntight->ProjectionY();

	h_pt ->Divide(hmutightpt, hmuloosept);
	h_eta->Divide(hmutighteta, hmulooseeta);

	// TGraphAsymmErrors *asymErrors = new TGraphAsymmErrors(h_pt);
	// asymErrors->SetName("asymErrors");
	// asymErrors->BayesDivide(hmutightpt, hmuloosept);

	// TCanvas *c1 = makeCanvas("asymErrors");
	// c1->cd();
	// h_pt->SetLineColor(kBlue);
	// h_pt->Draw();
	// asymErrors->Draw("same");
	// Util::PrintNoEPS(c1, "test", fOutputDir);

	h_pt ->SetXTitle(convertVarName("MuPt[1]"));
	h_eta->SetXTitle(convertVarName("MuEta[1]"));
	h_pt ->SetYTitle("# Tight / # Loose");
	h_eta->SetYTitle("# Tight / # Loose");
	h_2d->SetXTitle(convertVarName("MuPt[1]"));
	h_2d->SetYTitle(convertVarName("MuEta[1]"));
	delete H_ntight, H_nloose, hmuloosept, hmulooseeta, hmutightpt, hmutighteta;
	TString name = "";
	for(size_t i = 0; i < samples.size(); ++i){
		int sample = samples[i];
		name += h_2d->GetName();
		name += "_";
		name += fSamples[sample].sname;
	}
	if(output){
		printHisto(h_2d,  TString("Ratio")    + name, "Fake Ratio vs pt/eta", "colz");
		printHisto(h_pt,  TString("RatioPt")  + name, "Fake Ratio vs pt",     "PE1");
		printHisto(h_eta, TString("RatioEta") + name, "Fake Ratio vs eta",    "PE1");
	}
}
void MuonPlotter::calculateRatio(vector<int> samples, gChannel chan, int forp, float &ratio, float &ratioe, bool output){
	double ntight(0.), nloose(0.);
	double ntighte2(0.), nloosee2(0.);
	vector<int> v_ntight, v_nloose;
	vector<float> v_scale;
	vector<TString> v_name;
	for(size_t i = 0; i < samples.size(); ++i){
		int index = samples[i];
		
		int ntight_sam(0), nloose_sam(0);
		v_name.push_back(fSamples[index].sname);
		
		float scale = fLumiNorm/fSamples[index].lumi; // Normalize all
		if(fSamples[index].isdata) scale = 1;
		channel *cha;
		if(chan == Muon)     cha = &fSamples[index].mumu;
		if(chan == Electron) cha = &fSamples[index].ee;
		if(forp == 1){
			ntight += scale * cha->numbers.nsst;
			nloose += scale * cha->numbers.nssl;
			
			ntight_sam += cha->numbers.nsst;
			nloose_sam += cha->numbers.nssl;
		}
		if(forp == 2){
			ntight += scale * cha->numbers.nzt;
			nloose += scale * cha->numbers.nzl;

			ntight_sam += cha->numbers.nzt;
			nloose_sam += cha->numbers.nzl;
		}
		v_ntight.push_back(ntight_sam);
		v_nloose.push_back(nloose_sam);
		v_scale.push_back(scale);
	}
	ratioWithBinomErrors(ntight, nloose, ratio, ratioe);
	// ratioWithPoissErrors(ntight, nloose, ratio, ratioe);
	if(fVerbose > 2){
		cout << "--------------------------------------------------------" << endl;
		TString s_forp;
		if(forp == 1) s_forp = "f Ratio";
		if(forp == 2) s_forp = "p Ratio";
		TString s_channel;
		if(chan == Muon)     s_channel = "Muon";
		if(chan == Electron) s_channel = "Electron";
		cout << " Calling calculateRatio for " << s_forp << " in " << s_channel << " channel..." << endl;
		for(size_t i = 0; i < v_ntight.size(); ++i){
			cout << setw(9) << v_name[i] << ": ";
			cout << " Nt:    " << setw(8) << setprecision(2) << v_ntight[i];
			cout << " Nl:    " << setw(8) << setprecision(2) << v_nloose[i];
			cout << " Scale: " << v_scale[i] << endl;
		}
		cout << "--------------------------------------------------------" << endl;
		cout << " Total: ";
		cout << " Nt:    " << setw(8) << ntight;
		cout << " Nl:    " << setw(8) << nloose;
		cout << " Ratio: " << ratio << endl;
		cout << "--------------------------------------------------------" << endl;
	}
}
void MuonPlotter::calculateRatio(vector<int> samples, gChannel chan, int forp, float &ratio, float &ratioeup, float &ratioelow, bool output){
	// Careful, this method only takes integer numbers for passed/total events, therefore
	// only makes sense for application on data right now.
	int ntight(0), nloose(0);
	float ntighte2(0.), nloosee2(0.);
	vector<int> v_ntight, v_nloose;
	vector<TString> v_name;
	for(size_t i = 0; i < samples.size(); ++i){
		int index = samples[i];
		
		int ntight_sam(0), nloose_sam(0);
		v_name.push_back(fSamples[index].sname);
		
		// float scale = fLumiNorm/fSamples[index].lumi; // Normalize all
		// if(fSamples[index].isdata) scale = 1;
		channel *cha;
		if(chan == Muon)     cha = &fSamples[index].mumu;
		if(chan == Electron) cha = &fSamples[index].ee;
		if(forp == 1){
			ntight += cha->numbers.nsst;
			nloose += cha->numbers.nssl;
			
			ntight_sam += cha->numbers.nsst;
			nloose_sam += cha->numbers.nssl;
		}
		if(forp == 2){
			ntight += cha->numbers.nzt;
			nloose += cha->numbers.nzl;

			ntight_sam += cha->numbers.nzt;
			nloose_sam += cha->numbers.nzl;
		}
		v_ntight.push_back(ntight_sam);
		v_nloose.push_back(nloose_sam);
	}
	ratioWithAsymmCPErrors(ntight, nloose, ratio, ratioeup, ratioelow);
	if(fVerbose > 2){
		cout << "--------------------------------------------------------" << endl;
		TString s_forp;
		if(forp == 1) s_forp = "f Ratio";
		if(forp == 2) s_forp = "p Ratio";
		TString s_channel;
		if(chan == Muon)     s_channel = "Muon";
		if(chan == Electron) s_channel = "Electron";
		cout << " Calling calculateRatio for " << s_forp << " in " << s_channel << " channel..." << endl;
		for(size_t i = 0; i < v_ntight.size(); ++i){
			cout << setw(9) << v_name[i] << ": ";
			cout << " Nt:    " << setw(8) << setprecision(2) << v_ntight[i];
			cout << " Nl:    " << setw(8) << setprecision(2) << v_nloose[i];
			cout << endl;
		}
		cout << "--------------------------------------------------------" << endl;
		cout << " Total: ";
		cout << " Nt:    " << setw(8) << ntight;
		cout << " Nl:    " << setw(8) << nloose;
		cout << " Ratio: " << ratio << " + " << ratioeup << " - " << ratioelow << endl;
		cout << "--------------------------------------------------------" << endl;
	}
}

//____________________________________________________________________________
void MuonPlotter::ratioWithBinomErrors(float ntight, float nloose, float &ratio, float &error){
	ratio = ntight/nloose;
	error = TMath::Sqrt( ntight*(1.0-ntight/nloose) ) / nloose;                  // Binomial
}
void MuonPlotter::ratioWithPoissErrors(float ntight, float nloose, float &ratio, float &error){
	ratio = ntight/nloose;
	error = TMath::Sqrt( ntight*ntight*(nloose+ntight)/(nloose*nloose*nloose) ); // Poissonian	
}
void MuonPlotter::ratioWithAsymmCPErrors(int passed, int total, float &ratio, float &upper, float &lower){
	TEfficiency *eff = new TEfficiency("TempEfficiency", "TempEfficiency", 1, 0., 1.);
	eff->SetStatisticOption(TEfficiency::kFCP); // Frequentist Clopper Pearson = default
	eff->SetConfidenceLevel(0.683); // 1-sigma = default
	if( eff->SetTotalEvents(1, total) && eff->SetPassedEvents(1, passed) ){
		ratio = eff->GetEfficiency(1);
		upper = eff->GetEfficiencyErrorUp(1);
		lower = eff->GetEfficiencyErrorLow(1);
	}
	else{
		ratio = 1;
		upper = 1;
		lower = 0;
	};
	delete eff;
}

//____________________________________________________________________________
void MuonPlotter::makeSSPredictionPlots(vector<int> samples){
	// Need filled ratios before calling this function!

	// Prediction: /////////////////////////////////////////////////////////////////////
	TH1D *H_nsigpred = new TH1D("Nsigpred", "Predicted N_sig in Pt1 bins",       getNMuPt2Bins(),  getMuPt2Bins());
	TH1D *H_nfppred  = new TH1D("Nfppred",  "Predicted N_fp in Pt1 bins",        getNMuPt2Bins(),  getMuPt2Bins());
	TH1D *H_nffpred  = new TH1D("Nffpred",  "Predicted N_ff in Pt1 bins",        getNMuPt2Bins(),  getMuPt2Bins());
	TH1D *H_nFpred   = new TH1D("NFpred",   "Total predicted fakes in Pt1 bins", getNMuPt2Bins(),  getMuPt2Bins());
	bool output = false;
	for(size_t i = 0; i < samples.size(); ++i){
		int index = samples[i];
		float scale = fLumiNorm/fSamples[samples[i]].lumi;
		// vector<TH1D*> prediction = MuMuFPPrediction(index, output);

		channel cha = fSamples[index].mumu;
		vector<TH1D*> prediction = MuMuFPPrediction(fH2D_MufRatio, fH2D_MupRatio, cha.nthistos.h_nt2, cha.nthistos.h_nt10, cha.nthistos.h_nt0, output);
		H_nsigpred->Add(prediction[0], scale);
		H_nfppred ->Add(prediction[1], scale);
		H_nffpred ->Add(prediction[2], scale);
	}

	// Observation: ////////////////////////////////////////////////////////////////////
	TH1D *H_nsigobs  = new TH1D("Nsigobs", "Observed N_sig in Pt1 bins",  getNMuPt2Bins(),  getMuPt2Bins());
	vector<int> lm0sample; lm0sample.push_back(LM0);
	NObs(H_nsigobs, lm0sample, &MuonPlotter::isGenMatchedSUSYDiLepEvent);

	TH1D *H_nt2obs  = new TH1D("Nt2obs", "Observed N_t2 in Pt1 bins",  getNMuPt2Bins(),  getMuPt2Bins());
	NObs(H_nt2obs, samples);
	// NObs(H_nt2obs, samples, &MuonPlotter::isSSTTEvent);

	TH1D *H_nt2obsttbar = new TH1D("Nt2obsttbar", "Observed N_t2 in Pt1 bins, ttbar only",  getNMuPt2Bins(),  getMuPt2Bins());
	vector<int> ttbarsample; ttbarsample.push_back(TTbar);
	NObs(H_nt2obsttbar, fMCBG);
	// NObs(H_nt2obsttbar, ttbarsample, &MuonPlotter::isSSTTEvent);	

	// Output
	H_nsigobs->SetXTitle(convertVarName("MuPt[0]"));
	H_nsigobs->SetYTitle(Form("Events / %2.0f GeV", fBinWidthScale));

	H_nFpred->Add(H_nfppred);
	H_nFpred->Add(H_nffpred);
	H_nFpred->SetXTitle(convertVarName("MuPt[0]"));
	H_nFpred->SetYTitle(Form("Events / %2.0f GeV", fBinWidthScale));
	H_nt2obs->SetXTitle(convertVarName("MuPt[0]"));
	H_nt2obs->SetYTitle(Form("Events / %2.0f GeV", fBinWidthScale));
	H_nt2obsttbar->SetXTitle(convertVarName("MuPt[0]"));
	H_nt2obsttbar->SetYTitle(Form("Events / %2.0f GeV", fBinWidthScale));

	// Normalize to binwidth
	H_nsigpred    = normHistBW(H_nsigpred,    fBinWidthScale);
	H_nsigobs     = normHistBW(H_nsigobs,     fBinWidthScale);
	H_nfppred     = normHistBW(H_nfppred,     fBinWidthScale);
	H_nffpred     = normHistBW(H_nffpred,     fBinWidthScale);
	H_nFpred      = normHistBW(H_nFpred,      fBinWidthScale);
	H_nt2obs      = normHistBW(H_nt2obs,      fBinWidthScale);
	H_nt2obsttbar = normHistBW(H_nt2obsttbar, fBinWidthScale);

	H_nt2obs->SetFillColor(kBlue);
	H_nt2obs->SetLineColor(kBlue);
	H_nt2obs->SetFillStyle(3004);
	H_nt2obs->SetLineWidth(2);

	H_nsigpred->SetFillColor(8);
	H_nsigpred->SetMarkerColor(8);
	H_nsigpred->SetMarkerStyle(20);
	H_nsigpred->SetLineColor(8);
	H_nsigpred->SetLineWidth(2);

	H_nfppred->SetFillColor(kRed);
	H_nfppred->SetMarkerColor(kRed);
	H_nfppred->SetMarkerStyle(20);
	H_nfppred->SetLineColor(kRed);
	H_nfppred->SetLineWidth(2);

	H_nffpred->SetFillColor(13);
	H_nffpred->SetMarkerColor(13);
	H_nffpred->SetLineColor(13);
	H_nffpred->SetMarkerStyle(20);
	H_nffpred->SetLineWidth(2);

	plotOverlay4H(H_nt2obs, "N_{ t2}", H_nsigpred, "N_{ pp}" , H_nfppred, "N_{ f p}", H_nffpred, "N_{ f f}");

	H_nFpred->SetMinimum(0.);
	H_nt2obs->SetMinimum(0.);
	H_nsigobs->SetMinimum(0.);
	H_nsigpred->SetMinimum(0.);
	H_nt2obsttbar->SetMinimum(0.);

	H_nsigpred->SetMarkerColor(kRed);
	H_nFpred->SetMarkerColor(kRed);
	H_nFpred->SetMarkerStyle(20);
	H_nsigobs->SetMaximum(14.);
	H_nsigpred->SetMaximum(14.);
	H_nsigpred->SetMinimum(0.);
	plotPredOverlay2HWithRatio(H_nsigobs, "Observed N_{sig}", H_nsigpred, "Predicted N_{sig}");
	plotPredOverlay3HWithRatio(H_nFpred, "Predicted Fakes", H_nt2obs,  "Obs. N_{t2} (QCD, t#bar{t}+jets, V+jets, LM0)", H_nt2obsttbar, "Obs. N_{t2} (QCD, t#bar{t}+jets, V+jets)", false, false);
}

//____________________________________________________________________________
void MuonPlotter::NObs(TH1D *&hist, vector<int> samples, bool(MuonPlotter::*eventSelector)()){
/* This fills a histogram with the pt of the first muon for a given selection */
	hist->Sumw2();
	for(size_t i = 0; i < samples.size(); ++i){
		int index = samples[i];
		float scale = fLumiNorm / fSamples[index].lumi;

		TTree *tree = fSamples[index].tree;
		tree->ResetBranchAddresses();
		Init(tree);
		if (fChain == 0) return;
		Long64_t nentries = fChain->GetEntriesFast();
		Long64_t nbytes = 0, nb = 0;
		for (Long64_t jentry=0; jentry<nentries;jentry++) {
			Long64_t ientry = LoadTree(jentry);
			if (ientry < 0) break;
			nb = fChain->GetEntry(jentry);   nbytes += nb;
			printProgress(jentry, nentries, fSamples[index].name);

			if((*this.*eventSelector)() == false) continue;
			hist->Fill(MuPt[0], scale);
		}
	}	
}

//____________________________________________________________________________
void MuonPlotter::NObs(TH1D *&hist, vector<int> samples){
	hist->Sumw2();
	for(size_t i = 0; i < samples.size(); ++i){
		int index = samples[i];
		float scale = fLumiNorm / fSamples[index].lumi;
		channel cha = fSamples[index].mumu;
		TH1D *h_temp = cha.nthistos.h_nt2_pt;
		for(int j = 1; j <= h_temp->GetNbinsX(); ++j){
			double bincenter = h_temp->GetBinCenter(j);
			double content = h_temp->GetBinContent(j);
			int newbin = hist->FindBin(bincenter);

			double oldcontent = hist->GetBinContent(newbin);
			double olderror = hist->GetBinError(newbin);
			double newerror = TMath::Sqrt(olderror*olderror + content*scale*scale);
			double newcontent = oldcontent + content*scale;
			hist->SetBinContent(newbin, newcontent);
			hist->SetBinError(newbin, newerror);
			
		}
	}	
}

//____________________________________________________________________________
void MuonPlotter::makeIntPrediction(TString filename){
	ofstream OUT(filename.Data(), ios::trunc);

	vector<int> musamples;
	vector<int> elsamples;
	vector<int> emusamples;

	if(fSelectionSwitch == 0){
		musamples = fMuData;
		elsamples = fEGData;
		emusamples.push_back(MuA);
		emusamples.push_back(MuB);
		emusamples.push_back(EGA);
		emusamples.push_back(EGB);
	}
	if(fSelectionSwitch == 1){
		musamples  = fJMData;
		elsamples  = fJMData;
		emusamples = fJMData;
	}

	OUT << "/////////////////////////////////////////////////////////////////////////////" << endl;
	OUT << " Producing integrated predictions" << endl << endl;

	///////////////////////////////////////////////////////////////////////////////////
	// RATIOS /////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	float mufratio_data(0.),  mufratio_data_elow(0.), mufratio_data_eup(0.);
	float mupratio_data(0.),  mupratio_data_elow(0.), mupratio_data_eup(0.);
	float elfratio_data(0.),  elfratio_data_elow(0.), elfratio_data_eup(0.);
	float elpratio_data(0.),  elpratio_data_elow(0.), elpratio_data_eup(0.);
	float mufratio_allmc(0.), mufratio_allmc_e(0.);
	float mupratio_allmc(0.), mupratio_allmc_e(0.);
	float elfratio_allmc(0.), elfratio_allmc_e(0.);
	float elpratio_allmc(0.), elpratio_allmc_e(0.);

	calculateRatio(fJMData, Muon, 1, mufratio_data, mufratio_data_elow);
	calculateRatio(fMuData, Muon, 2, mupratio_data, mupratio_data_elow);
	// calculateRatio(fJMData, Muon, 1, mufratio_data, mufratio_data_elow , mufratio_data_eup);
	// calculateRatio(fMuData, Muon, 2, mupratio_data, mupratio_data_elow , mupratio_data_eup);

	vector<int> elesample;
	if(fSelectionSwitch == 0) elesample = fEGData;
	if(fSelectionSwitch == 1) elesample = fJMData;
	calculateRatio(elesample, Electron, 1, elfratio_data, elfratio_data_elow);
	calculateRatio(fEGData,   Electron, 2, elpratio_data, elpratio_data_elow);
	// calculateRatio(elesample, Electron, 1, elfratio_data, elfratio_data_elow, elfratio_data_eup);
	// calculateRatio(fEGData,   Electron, 2, elpratio_data, elpratio_data_elow, elpratio_data_eup);

	calculateRatio(fMCBG, Muon,     1, mufratio_allmc, mufratio_allmc_e);
	calculateRatio(fMCBG, Muon,     2, mupratio_allmc, mupratio_allmc_e);
	calculateRatio(fMCBG, Electron, 1, elfratio_allmc, elfratio_allmc_e);
	calculateRatio(fMCBG, Electron, 2, elpratio_allmc, elpratio_allmc_e);

	///////////////////////////////////////////////////////////////////////////////////
	// SYSTEMATICS ////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	// // TTbar genmatch ratios for muons:
	// const int nptbins = 1; // Dummy binning
	// const double ptbins[nptbins+1] = {5., 1000.};
	// const int netabins = 1;
	// const double etabins[netabins+1] = {-2.5, 2.5};
	// 
	// TH2D *H_fratio_ttbar = fillRatio(TTbar, 0, &MuonPlotter::isGoodMuEvent, &MuonPlotter::isFakeTTbarMuon,   nptbins, ptbins, netabins, etabins);
	// TH2D *H_pratio_ttbar = fillRatio(TTbar, 0, &MuonPlotter::isGoodMuEvent, &MuonPlotter::isPromptTTbarMuon, nptbins, ptbins, netabins, etabins);
	// H_fratio_ttbar->SetName("MufRatioTTbar");
	// H_pratio_ttbar->SetName("MupRatioTTbar");
	// 
	// float mufratio_ttbar   = H_fratio_ttbar->GetBinContent(1,1);
	// float mufratio_ttbar_e = H_fratio_ttbar->GetBinError(1,1);
	// float mupratio_ttbar   = H_pratio_ttbar->GetBinContent(1,1);
	// float mupratio_ttbar_e = H_pratio_ttbar->GetBinError(1,1);

	// // Add deviation as systematic (only muons)
	// float deviation(0.), olderror(0.), newerror(0.);
	// deviation = fabs(mufratio_allmc - mufratio_ttbar);
	// // Add to mc ratios
	// olderror = mufratio_allmc_e;
	// newerror = sqrt(olderror*olderror + deviation*deviation);
	// mufratio_allmc_e = newerror;
	// // Add to data ratios
	// olderror = mufratio_data_e;
	// newerror = sqrt(olderror*olderror + deviation*deviation);
	// mufratio_data_e = newerror;
	// 
	// deviation  = fabs(mupratio_allmc - mupratio_ttbar);
	// // Add to mc ratios
	// olderror = mupratio_allmc_e;
	// newerror = sqrt(olderror*olderror + deviation*deviation);
	// mupratio_allmc_e = newerror;
	// // Add to data ratios
	// olderror = mupratio_data_e;
	// newerror = sqrt(olderror*olderror + deviation*deviation);
	// mupratio_data_e = newerror;
	// 
	// // Add hardcoded systematic error on electron ratios:
	// float adderror = 0.05; // add this to e f ratio
	// olderror = elfratio_data_e;
	// newerror = sqrt(olderror*olderror + adderror*adderror);
	// elfratio_data_e = newerror;
	// 
	// adderror = 0.01; // add this to e p ratio
	// olderror = elpratio_data_e;
	// newerror = sqrt(olderror*olderror + adderror*adderror);
	// elpratio_data_e = newerror;

	///////////////////////////////////////////////////////////////////////////////////
	// OBSERVATIONS ///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	float nt2_mumu(0.),    nt10_mumu(0.),    nt0_mumu(0.);
	float nt2_mumu_e2(0.), nt10_mumu_e2(0.), nt0_mumu_e2(0.);
	float nt2_emu(0.),     nt10_emu(0.),     nt01_emu(0.),    nt0_emu(0.);
	float nt2_emu_e2(0.),  nt10_emu_e2(0.),  nt01_emu_e2(0.), nt0_emu_e2(0.);	
	float nt2_ee(0.),      nt10_ee(0.),      nt0_ee(0.);
	float nt2_ee_e2(0.),   nt10_ee_e2(0.),   nt0_ee_e2(0.);
	

	for(size_t i = 0; i < musamples.size(); ++i){
		int index = musamples[i];
		channel *mumu = &fSamples[index].mumu;
		nt2_mumu  += mumu->numbers.nt2;
		nt10_mumu += mumu->numbers.nt10;
		nt0_mumu  += mumu->numbers.nt0;
		nt2_mumu_e2  += mumu->numbers.nt2;
		nt10_mumu_e2 += mumu->numbers.nt10;
		nt0_mumu_e2  += mumu->numbers.nt0;
	}		
	for(size_t i = 0; i < emusamples.size(); ++i){
		int index = emusamples[i];
		channel *emu = &fSamples[index].emu;
		nt2_emu  += emu->numbers.nt2;
		nt10_emu += emu->numbers.nt10;
		nt01_emu += emu->numbers.nt01;
		nt0_emu  += emu->numbers.nt0;
		nt2_emu_e2  += emu->numbers.nt2;
		nt10_emu_e2 += emu->numbers.nt10;
		nt01_emu_e2 += emu->numbers.nt01;
		nt0_emu_e2  += emu->numbers.nt0;
	}		
	for(size_t i = 0; i < elsamples.size(); ++i){
		int index = elsamples[i];
		channel *ee = &fSamples[index].ee;
		nt2_ee  += ee->numbers.nt2;
		nt10_ee += ee->numbers.nt10;
		nt0_ee  += ee->numbers.nt0;
		nt2_ee_e2  += ee->numbers.nt2;
		nt10_ee_e2 += ee->numbers.nt10;
		nt0_ee_e2  += ee->numbers.nt0;
	}		
	
	///////////////////////////////////////////////////////////////////////////////////
	// PREDICTIONS ////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	FPRatios *fMuMuFPRatios_lo = new FPRatios();
	// FPRatios *fMuMuFPRatios_up = new FPRatios();
	FPRatios *fEMuFPRatios     = new FPRatios();
	FPRatios *fEEFPRatios      = new FPRatios();

	fMuMuFPRatios_lo->SetVerbose(fVerbose);
	// fMuMuFPRatios_up->SetVerbose(fVerbose);
	fEMuFPRatios ->SetVerbose(fVerbose);
	fEEFPRatios  ->SetVerbose(fVerbose);

	fEEFPRatios  ->SetElFratios(elfratio_data, elfratio_data_elow);
	fEEFPRatios  ->SetElPratios(elpratio_data, elpratio_data_elow);

	fEMuFPRatios ->SetFratios(elfratio_data, elfratio_data_elow, mufratio_data, mufratio_data_elow);
	fEMuFPRatios ->SetPratios(elpratio_data, elpratio_data_elow, mupratio_data, mupratio_data_elow);


	// MuMu Channel
	fMuMuFPRatios_lo->SetMuFratios(mufratio_data, mufratio_data_elow);
	fMuMuFPRatios_lo->SetMuPratios(mupratio_data, mupratio_data_elow);
	// fMuMuFPRatios_up->SetMuFratios(mufratio_data, mufratio_data_eup);
	// fMuMuFPRatios_up->SetMuPratios(mupratio_data, mupratio_data_eup);

	vector<double> nt_mumu;
	nt_mumu.push_back(nt0_mumu);
	nt_mumu.push_back(nt10_mumu); // mu passes
	nt_mumu.push_back(nt2_mumu);
	
	fMuMuFPRatios_lo->NevtTopol(0, 2, nt_mumu);
	// fMuMuFPRatios_up->NevtTopol(0, 2, nt_mumu);

	vector<double> vpt, veta;
	vpt.push_back(30.); vpt.push_back(30.); // Fake pts and etas (first electron then muon)
	veta.push_back(0.); veta.push_back(0.);

	vector<double> MuMu_Nev      = fMuMuFPRatios_lo->NevtPass(vpt, veta);
	vector<double> MuMu_Estat_lo = fMuMuFPRatios_lo->NevtPassErrStat();
	vector<double> MuMu_Esyst_lo = fMuMuFPRatios_lo->NevtPassErrSyst();

	// vector<double> MuMu_Nev_up   = fMuMuFPRatios_up->NevtPass(vpt, veta);
	// vector<double> MuMu_Estat_up = fMuMuFPRatios_up->NevtPassErrStat();
	// vector<double> MuMu_Esyst_up = fMuMuFPRatios_up->NevtPassErrSyst();

	
	// EMu Channel
	vector<double> nt_emu;
	nt_emu.push_back(nt0_emu);
	nt_emu.push_back(nt01_emu); // e passes
	nt_emu.push_back(nt10_emu); // mu passes
	nt_emu.push_back(nt2_emu);
	
	fEMuFPRatios->NevtTopol(1, 1, nt_emu);

	vector<double> EMu_nevFP      = fEMuFPRatios->NevtPass(vpt, veta);
	vector<double> EMu_nevFPEstat = fEMuFPRatios->NevtPassErrStat();
	vector<double> EMu_nevFPEsyst = fEMuFPRatios->NevtPassErrSyst();
	
	// EE Channel
	vector<double> nt_ee;
	nt_ee.push_back(nt0_ee);
	nt_ee.push_back(nt10_ee); // el passes
	nt_ee.push_back(nt2_ee);
	
	fEEFPRatios->NevtTopol(2, 0, nt_ee);

	vector<double> EE_nevFP      = fEEFPRatios->NevtPass(vpt, veta);
	vector<double> EE_nevFPEstat = fEEFPRatios->NevtPassErrStat();
	vector<double> EE_nevFPEsyst = fEEFPRatios->NevtPassErrSyst();
	
	///////////////////////////////////////////////////////////////////////////////////
	// PRINTOUT ///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	OUT << "--------------------------------------------------------------------------------------------------" << endl;
	OUT << "  RATIOS  ||     Mu-fRatio      |     Mu-pRatio      ||     El-fRatio      |     El-pRatio      ||" << endl;
	OUT << "--------------------------------------------------------------------------------------------------" << endl;
	OUT << setw(10) << " allMC    ||";
	OUT << setw(7)  << setprecision(2) << mufratio_allmc << " +/- " << setw(7) << setprecision(2) << mufratio_allmc_e << " |";
	OUT << setw(7)  << setprecision(2) << mupratio_allmc << " +/- " << setw(7) << setprecision(2) << mupratio_allmc_e << " ||";
	OUT << setw(7)  << setprecision(2) << elfratio_allmc << " +/- " << setw(7) << setprecision(2) << elfratio_allmc_e << " |";
	OUT << setw(7)  << setprecision(2) << elpratio_allmc << " +/- " << setw(7) << setprecision(2) << elpratio_allmc_e << " ||";
	OUT << endl;
	OUT << setw(10) << " data     ||";
	OUT << setw(7)  << setprecision(2) << mufratio_data  << " +/- " << setw(7) << setprecision(2) << mufratio_data_elow  << " |";
	OUT << setw(7)  << setprecision(2) << mupratio_data  << " +/- " << setw(7) << setprecision(2) << mupratio_data_elow  << " ||";
	OUT << setw(7)  << setprecision(2) << elfratio_data  << " +/- " << setw(7) << setprecision(2) << elfratio_data_elow  << " |";
	OUT << setw(7)  << setprecision(2) << elpratio_data  << " +/- " << setw(7) << setprecision(2) << elpratio_data_elow  << " ||";
	OUT << endl;
	OUT << "--------------------------------------------------------------------------------------------------" << endl << endl;
	OUT << "-------------------------------------------------------------------------------------------------------------------" << endl;
	OUT << "          ||            Mu/Mu            ||                   E/Mu                ||             E/E             ||" << endl;
	OUT << "  YIELDS  ||   Nt2   |   Nt1   |   Nt0   ||   Nt2   |   Nt10  |   Nt01  |   Nt0   ||   Nt2   |   Nt1   |   Nt0   ||" << endl;
	OUT << "-------------------------------------------------------------------------------------------------------------------" << endl;
	
	float nt2sum_mumu(0.), nt10sum_mumu(0.), nt0sum_mumu(0.);
	float nt2sum_emu(0.), nt10sum_emu(0.), nt01sum_emu(0.), nt0sum_emu(0.);
	float nt2sum_ee(0.), nt10sum_ee(0.), nt0sum_ee(0.);
	for(size_t i = 0; i < fMCBG.size(); ++i){
		int index = fMCBG[i];
		float scale = fLumiNorm / fSamples[index].lumi;
		nt2sum_mumu  += scale*fSamples[index].mumu.numbers.nt2;
		nt10sum_mumu += scale*fSamples[index].mumu.numbers.nt10;
		nt0sum_mumu  += scale*fSamples[index].mumu.numbers.nt0;
		nt2sum_emu   += scale*fSamples[index].emu .numbers.nt2;
		nt10sum_emu  += scale*fSamples[index].emu .numbers.nt10;
		nt01sum_emu  += scale*fSamples[index].emu .numbers.nt01;
		nt0sum_emu   += scale*fSamples[index].emu .numbers.nt0;
		nt2sum_ee    += scale*fSamples[index].ee  .numbers.nt2;
		nt10sum_ee   += scale*fSamples[index].ee  .numbers.nt10;
		nt0sum_ee    += scale*fSamples[index].ee  .numbers.nt0;

		OUT << setw(9) << fSamples[index].sname << " || ";
		OUT << setw(7)  << setprecision(2) << scale*fSamples[index].mumu.numbers.nt2  << " | ";
		OUT << setw(7)  << setprecision(2) << scale*fSamples[index].mumu.numbers.nt10 << " | ";
		OUT << setw(7)  << setprecision(2) << scale*fSamples[index].mumu.numbers.nt0  << " || ";
		OUT << setw(7)  << setprecision(2) << scale*fSamples[index].emu.numbers.nt2   << " | ";
		OUT << setw(7)  << setprecision(2) << scale*fSamples[index].emu.numbers.nt10  << " | ";
		OUT << setw(7)  << setprecision(2) << scale*fSamples[index].emu.numbers.nt01  << " | ";
		OUT << setw(7)  << setprecision(2) << scale*fSamples[index].emu.numbers.nt0   << " || ";
		OUT << setw(7)  << setprecision(2) << scale*fSamples[index].ee.numbers.nt2    << " | ";
		OUT << setw(7)  << setprecision(2) << scale*fSamples[index].ee.numbers.nt10   << " | ";
		OUT << setw(7)  << setprecision(2) << scale*fSamples[index].ee.numbers.nt0    << " || ";
		OUT << endl;
	}	
	OUT << "-------------------------------------------------------------------------------------------------------------------" << endl;
	OUT << setw(9) << "MC sum" << " || ";
	OUT << setw(7) << setprecision(2) << nt2sum_mumu  << " | ";
	OUT << setw(7) << setprecision(2) << nt10sum_mumu << " | ";
	OUT << setw(7) << setprecision(2) << nt0sum_mumu  << " || ";
	OUT << setw(7) << setprecision(2) << nt2sum_emu   << " | ";
	OUT << setw(7) << setprecision(2) << nt10sum_emu  << " | ";
	OUT << setw(7) << setprecision(2) << nt01sum_emu  << " | ";
	OUT << setw(7) << setprecision(2) << nt0sum_emu   << " || ";
	OUT << setw(7) << setprecision(2) << nt2sum_ee    << " | ";
	OUT << setw(7) << setprecision(2) << nt10sum_ee   << " | ";
	OUT << setw(7) << setprecision(2) << nt0sum_ee    << " || ";
	OUT << endl;
	OUT << "-------------------------------------------------------------------------------------------------------------------" << endl;
	OUT << setw(9) << fSamples[LM0].sname << " || ";
	OUT << setw(7)  << setprecision(2) << fLumiNorm / fSamples[LM0].lumi * fSamples[LM0].mumu.numbers.nt2  << " | ";
	OUT << setw(7)  << setprecision(2) << fLumiNorm / fSamples[LM0].lumi * fSamples[LM0].mumu.numbers.nt10 << " | ";
	OUT << setw(7)  << setprecision(2) << fLumiNorm / fSamples[LM0].lumi * fSamples[LM0].mumu.numbers.nt0  << " || ";
	OUT << setw(7)  << setprecision(2) << fLumiNorm / fSamples[LM0].lumi * fSamples[LM0].emu .numbers.nt2  << " | ";
	OUT << setw(7)  << setprecision(2) << fLumiNorm / fSamples[LM0].lumi * fSamples[LM0].emu .numbers.nt10 << " | ";
	OUT << setw(7)  << setprecision(2) << fLumiNorm / fSamples[LM0].lumi * fSamples[LM0].emu .numbers.nt01 << " | ";
	OUT << setw(7)  << setprecision(2) << fLumiNorm / fSamples[LM0].lumi * fSamples[LM0].emu .numbers.nt0  << " || ";
	OUT << setw(7)  << setprecision(2) << fLumiNorm / fSamples[LM0].lumi * fSamples[LM0].ee  .numbers.nt2  << " | ";
	OUT << setw(7)  << setprecision(2) << fLumiNorm / fSamples[LM0].lumi * fSamples[LM0].ee  .numbers.nt10 << " | ";
	OUT << setw(7)  << setprecision(2) << fLumiNorm / fSamples[LM0].lumi * fSamples[LM0].ee  .numbers.nt0  << " || ";
	OUT << endl;
	OUT << "-------------------------------------------------------------------------------------------------------------------" << endl;
	OUT << setw(9) << "data"  << " || ";
	OUT << setw(7) << setprecision(2) << nt2_mumu  << " | ";
	OUT << setw(7) << setprecision(2) << nt10_mumu << " | ";
	OUT << setw(7) << setprecision(2) << nt0_mumu  << " || ";
	OUT << setw(7) << setprecision(2) << nt2_emu   << " | ";
	OUT << setw(7) << setprecision(2) << nt10_emu  << " | ";
	OUT << setw(7) << setprecision(2) << nt01_emu  << " | ";
	OUT << setw(7) << setprecision(2) << nt0_emu   << " || ";
	OUT << setw(7) << setprecision(2) << nt2_ee    << " | ";
	OUT << setw(7) << setprecision(2) << nt10_ee   << " | ";
	OUT << setw(7) << setprecision(2) << nt0_ee    << " || ";
	OUT << endl;
	OUT << "-------------------------------------------------------------------------------------------------------------------" << endl;
	OUT << endl;
	OUT << "  PREDICTIONS" << endl;
	OUT << "--------------------------------------------------------------" << endl;
	OUT << " Mu/Mu Channel:" << endl;
	// OUT << "  Npp:           " << setw(7) << setprecision(3) << MuMu_Nev[0];
	// OUT << " +/- "             << setw(7) << setprecision(3) << MuMu_Estat_lo[0] << " (stat)";
	// OUT << " + "               << setw(7) << setprecision(3) << MuMu_Esyst_up[0];
	// OUT << " - "               << setw(7) << setprecision(3) << MuMu_Esyst_lo[0] << " (syst)" << endl;
	// OUT << "  Nfp:           " << setw(7) << setprecision(3) << MuMu_Nev[1];
	// OUT << " +/- "             << setw(7) << setprecision(3) << MuMu_Estat_lo[1] << " (stat)";
	// OUT << " + "               << setw(7) << setprecision(3) << MuMu_Esyst_up[1];
	// OUT << " - "               << setw(7) << setprecision(3) << MuMu_Esyst_lo[1] << " (syst)" << endl;
	// OUT << "  Nff:           " << setw(7) << setprecision(3) << MuMu_Nev[2];
	// OUT << " +/- "             << setw(7) << setprecision(3) << MuMu_Estat_lo[2] << " (stat)";
	// OUT << " + "               << setw(7) << setprecision(3) << MuMu_Esyst_up[2];
	// OUT << " - "               << setw(7) << setprecision(3) << MuMu_Esyst_lo[2] << " (syst)" << endl;
	// OUT << "  Total fakes:   " << setw(7) << setprecision(3) << MuMu_Nev[1]+MuMu_Nev[2];
	// OUT << " +/- "             << setw(7) << setprecision(3) << TMath::Sqrt(MuMu_Estat_lo[1]*MuMu_Estat_lo[1] + MuMu_Estat_lo[2]*MuMu_Estat_lo[2]) << " (stat)";
	// OUT << " + "               << setw(7) << setprecision(3) << TMath::Sqrt(MuMu_Esyst_up[1]*MuMu_Esyst_up[1] + MuMu_Esyst_up[2]*MuMu_Esyst_up[2]);
	// OUT << " - "               << setw(7) << setprecision(3) << TMath::Sqrt(MuMu_Esyst_lo[1]*MuMu_Esyst_lo[1] + MuMu_Esyst_lo[2]*MuMu_Esyst_lo[2]) << " (syst)" << endl;
	OUT << "  Npp:           " << setw(7) << setprecision(3) << MuMu_Nev[0];
	OUT << " +/- "             << setw(7) << setprecision(3) << MuMu_Estat_lo[0];
	OUT << " (stat) +/- "      << setw(7) << setprecision(3) << MuMu_Esyst_lo[0] << " (syst)" << endl;
	OUT << "  Nfp:           " << setw(7) << setprecision(3) << MuMu_Nev[1];
	OUT << " +/- "             << setw(7) << setprecision(3) << MuMu_Estat_lo[1];
	OUT << " (stat) +/- "      << setw(7) << setprecision(3) << MuMu_Esyst_lo[1] << " (syst)" << endl;
	OUT << "  Nff:           " << setw(7) << setprecision(3) << MuMu_Nev[2];
	OUT << " +/- "             << setw(7) << setprecision(3) << MuMu_Estat_lo[2];
	OUT << " (stat) +/- "      << setw(7) << setprecision(3) << MuMu_Esyst_lo[2] << " (syst)" << endl;
	OUT << "  Total fakes:   " << setw(7) << setprecision(3) << MuMu_Nev[1]+MuMu_Nev[2];
	OUT << " +/- "             << setw(7) << setprecision(3) << TMath::Sqrt(MuMu_Estat_lo[1]*MuMu_Estat_lo[1] + MuMu_Estat_lo[2]*MuMu_Estat_lo[2]);
	OUT << " (stat) +/- "      << setw(7) << setprecision(3) << TMath::Sqrt(MuMu_Esyst_lo[1]*MuMu_Esyst_lo[1] + MuMu_Esyst_lo[2]*MuMu_Esyst_lo[2]) << " (syst)" << endl;
	OUT << "--------------------------------------------------------------" << endl;
	OUT << " E/Mu Channel:" << endl;
	OUT << "  Npp:           " << setw(7) << setprecision(3) << EMu_nevFP[0];
	OUT << " +/- "             << setw(7) << setprecision(3) << EMu_nevFPEstat[0];
	OUT << " (stat) +/- "      << setw(7) << setprecision(3) << EMu_nevFPEsyst[0] << " (syst)" << endl;
	OUT << "  Nfp:           " << setw(7) << setprecision(3) << EMu_nevFP[1];
	OUT << " +/- "             << setw(7) << setprecision(3) << EMu_nevFPEstat[1];
	OUT << " (stat) +/- "      << setw(7) << setprecision(3) << EMu_nevFPEsyst[1] << " (syst)" << endl;
	OUT << "  Npf:           " << setw(7) << setprecision(3) << EMu_nevFP[2];
	OUT << " +/- "             << setw(7) << setprecision(3) << EMu_nevFPEstat[2];
	OUT << " (stat) +/- "      << setw(7) << setprecision(3) << EMu_nevFPEsyst[2] << " (syst)" << endl;
	OUT << "  Nff:           " << setw(7) << setprecision(3) << EMu_nevFP[3];
	OUT << " +/- "             << setw(7) << setprecision(3) << EMu_nevFPEstat[3];
	OUT << " (stat) +/- "      << setw(7) << setprecision(3) << EMu_nevFPEsyst[3] << " (syst)" << endl;
	OUT << "  Total fakes:   " << setw(7) << setprecision(3) << EMu_nevFP[1]+EMu_nevFP[2]+EMu_nevFP[3];
	OUT << " +/- "             << setw(7) << setprecision(3) << TMath::Sqrt(EMu_nevFPEstat[1]*EMu_nevFPEstat[1] + EMu_nevFPEstat[2]*EMu_nevFPEstat[2] + EMu_nevFPEstat[3]*EMu_nevFPEstat[3]);
	OUT << " (stat) +/- "      << setw(7) << setprecision(3) << TMath::Sqrt(EMu_nevFPEsyst[1]*EMu_nevFPEsyst[1] + EMu_nevFPEsyst[2]*EMu_nevFPEsyst[2] + EMu_nevFPEsyst[3]*EMu_nevFPEsyst[3]) << " (syst)" << endl;
	OUT << "--------------------------------------------------------------" << endl;
	OUT << " E/E Channel:" << endl;
	OUT << "  Npp:           " << setw(7) << setprecision(3) << EE_nevFP[0];
	OUT << " +/- "             << setw(7) << setprecision(3) << EE_nevFPEstat[0];
	OUT << " (stat) +/- "      << setw(7) << setprecision(3) << EE_nevFPEsyst[0] << " (syst)" << endl;
	OUT << "  Nfp:           " << setw(7) << setprecision(3) << EE_nevFP[1];
	OUT << " +/- "             << setw(7) << setprecision(3) << EE_nevFPEstat[1];
	OUT << " (stat) +/- "      << setw(7) << setprecision(3) << EE_nevFPEsyst[1] << " (syst)" << endl;
	OUT << "  Nff:           " << setw(7) << setprecision(3) << EE_nevFP[2];
	OUT << " +/- "             << setw(7) << setprecision(3) << EE_nevFPEstat[2];
	OUT << " (stat) +/- "      << setw(7) << setprecision(3) << EE_nevFPEsyst[2] << " (syst)" << endl;
	OUT << "  Total fakes:   " << setw(7) << setprecision(3) << EE_nevFP[1]+EE_nevFP[2];
	OUT << " +/- "             << setw(7) << setprecision(3) << TMath::Sqrt(EE_nevFPEstat[1]*EE_nevFPEstat[1] + EE_nevFPEstat[2]*EE_nevFPEstat[2]);
	OUT << " (stat) +/- "      << setw(7) << setprecision(3) << TMath::Sqrt(EE_nevFPEsyst[1]*EE_nevFPEsyst[1] + EE_nevFPEsyst[2]*EE_nevFPEsyst[2]) << " (syst)" << endl;
	OUT << "--------------------------------------------------------------" << endl;
	OUT << "/////////////////////////////////////////////////////////////////////////////" << endl;

	OUT.close();
	delete fMuMuFPRatios_lo;
	// delete fMuMuFPRatios_up;
	delete fEMuFPRatios;
	delete fEEFPRatios;
}

//____________________________________________________________________________
void MuonPlotter::makeIntPredictionMuMu(vector<int> inputsamples){
	cout << "/////////////////////////////////////////////////////////////////////////////" << endl;
	cout << " Producing prediction for MuMu channel" << endl << endl;
	bool data = true; // Use ratios from data or mc?

	// Dummy binning
	const int nptbins = 1;
	const double ptbins[nptbins+1] = {5., 1000.};
	const int netabins = 1;
	const double etabins[netabins+1] = {-2.5, 2.5};

	// Fill the ratios
	TH2D *H_fratio_allmc = new TH2D("fRatioAllMC", "fRatio for all MC", nptbins, ptbins, netabins, etabins);
	TH2D *H_fratio_data  = new TH2D("fRatioData", "fRatio for Mu Data", nptbins, ptbins, netabins, etabins);
	TH2D *H_pratio_allmc = new TH2D("pRatioAllMC", "pRatio for all MC", nptbins, ptbins, netabins, etabins);
	TH2D *H_pratio_data  = new TH2D("pRatioData", "pRatio for Mu Data", nptbins, ptbins, netabins, etabins);

	TH2D *H_fratio_ttbar = fillMuRatio(TTbar, 0, &MuonPlotter::isGoodMuEvent, &MuonPlotter::isFakeTTbarMuon, nptbins, ptbins, netabins, etabins);
	TH2D *H_pratio_ttbar = fillMuRatio(TTbar, 0, &MuonPlotter::isGoodMuEvent, &MuonPlotter::isPromptTTbarMuon, nptbins, ptbins, netabins, etabins);
	H_fratio_ttbar->SetName("fRatioTTbar");
	H_pratio_ttbar->SetName("pRatioTTbar");

	float fratio_allmc(0.), fratio_allmc_e(0.);
	float fratio_data(0.), fratio_data_e(0.);
	float pratio_allmc(0.), pratio_allmc_e(0.);
	float pratio_data(0.), pratio_data_e(0.);

	float fratio_ttbar   = H_fratio_ttbar->GetBinContent(1,1);
	float fratio_ttbar_e = H_fratio_ttbar->GetBinError(1,1);

	float pratio_ttbar   = H_pratio_ttbar->GetBinContent(1,1);
	float pratio_ttbar_e = H_pratio_ttbar->GetBinError(1,1);

	calculateRatio(fMCBG,   Muon, 1, fratio_allmc, fratio_allmc_e);
	calculateRatio(fJMData, Muon, 1, fratio_data,  fratio_data_e);
	calculateRatio(fMCBG,   Muon, 2, pratio_allmc, pratio_allmc_e);
	calculateRatio(fMuData, Muon, 2, pratio_data,  pratio_data_e);

	H_fratio_allmc->SetBinContent(1,1,fratio_allmc);
	H_fratio_allmc->SetBinError  (1,1,fratio_allmc_e);
	H_fratio_data ->SetBinContent(1,1,fratio_data);
	H_fratio_data ->SetBinError  (1,1,fratio_data_e);

	H_pratio_allmc->SetBinContent(1,1,pratio_allmc);
	H_pratio_allmc->SetBinError  (1,1,pratio_allmc_e);
	H_pratio_data ->SetBinContent(1,1,pratio_data);
	H_pratio_data ->SetBinError  (1,1,pratio_data_e);

	// Add systematics to ratios
	float deviation(0.), olderror(0.), newerror(0.);
	deviation = fabs(fratio_allmc - fratio_ttbar);
	// Add to mc ratios
	olderror = H_fratio_allmc->GetBinError(1,1);
	newerror = sqrt(olderror*olderror + deviation*deviation);
	H_fratio_allmc->SetBinError(1,1,newerror);
	fratio_allmc_e = newerror;
	// Add to data ratios
	olderror = H_fratio_data->GetBinError(1,1);
	newerror = sqrt(olderror*olderror + deviation*deviation);
	H_fratio_data->SetBinError(1,1,newerror);
	fratio_data_e = newerror;

	deviation  = fabs(pratio_allmc - pratio_ttbar);
	// Add to mc ratios
	olderror = pratio_allmc_e;
	newerror = sqrt(olderror*olderror + deviation*deviation);
	H_pratio_allmc->SetBinError(1,1,newerror);
	pratio_allmc_e = newerror;
	// Add to data ratios
	olderror = pratio_data_e;
	newerror = sqrt(olderror*olderror + deviation*deviation);
	H_pratio_data->SetBinError(1,1,newerror);
	pratio_data_e = newerror;

	double nt2(0.), nt1(0.), nt0(0.);
	double nt2_e2(0.), nt1_e2(0.), nt0_e2(0.);
	for(size_t i = 0; i < inputsamples.size(); ++i){
		int index = inputsamples[i];
		float scale = fLumiNorm/fSamples[index].lumi; // Normalize all
		channel cha = fSamples[index].mumu;
		if(data) scale = 1.;
		nt2 += scale * cha.numbers.nt2;
		nt1 += scale * cha.numbers.nt10;
		nt0 += scale * cha.numbers.nt0;
		nt2_e2 += scale*scale * cha.numbers.nt2;
		nt1_e2 += scale*scale * cha.numbers.nt10;
		nt0_e2 += scale*scale * cha.numbers.nt0;
	}

	// Make prediction
	FPRatios *fFPRatios = new FPRatios();
	fFPRatios->SetVerbose(fVerbose);
	if(data){
		fFPRatios->SetMuFratios(H_fratio_data);
		fFPRatios->SetMuPratios(H_pratio_data);
	}
	else{
		fFPRatios->SetMuFratios(H_fratio_allmc);
		fFPRatios->SetMuPratios(H_pratio_allmc);
	}
	vector<double> nt;
	nt.push_back(nt0); nt.push_back(nt1); nt.push_back(nt2);
	fFPRatios->NevtTopol(0, 2, nt);

	vector<double> vpt, veta;
	vpt.push_back(30.); vpt.push_back(30.); // Fake pts and etas
	veta.push_back(0.); veta.push_back(0.);

	vector<double> nevFP = fFPRatios->NevtPass(vpt, veta);
	vector<double> nevFPEstat = fFPRatios->NevtPassErrStat();
	vector<double> nevFPEsyst = fFPRatios->NevtPassErrSyst();

	cout << "          |       fRatio       |       pRatio       |" << endl;
	cout << "-----------------------------------------------------" << endl;
	cout << setw(10) << " allMC    |";
	cout << setw(7)  << setprecision(2) << fratio_allmc << " +/- " << setw(7) << setprecision(2) << fratio_allmc_e << " |";
	cout << setw(7)  << setprecision(2) << pratio_allmc << " +/- " << setw(7) << setprecision(2) << pratio_allmc_e << " |";
	cout << endl;
	cout << setw(10) << " data     |";
	cout << setw(7)  << setprecision(2) << fratio_data  << " +/- " << setw(7) << setprecision(2) << fratio_data_e  << " |";
	cout << setw(7)  << setprecision(2) << pratio_data  << " +/- " << setw(7) << setprecision(2) << pratio_data_e  << " |";
	cout << endl;
	cout << setw(10) << " ttbar    |";
	cout << setw(7)  << setprecision(2) << fratio_ttbar << " +/- " << setw(7) << setprecision(2) << fratio_ttbar_e << " |";
	cout << setw(7)  << setprecision(2) << pratio_ttbar << " +/- " << setw(7) << setprecision(2) << pratio_ttbar_e << " |";
	cout << endl;
	cout << "-----------------------------------------------------" << endl;
	cout << "          |   Nt2   |   Nt1   |   Nt0   |" << endl;
	cout << "-----------------------------------------" << endl;
	cout << setw(9) << "data"  << " | ";
	cout << setw(7) << setprecision(2) << nt2 << " | ";
	cout << setw(7) << setprecision(2) << nt1 << " | ";
	cout << setw(7) << setprecision(2) << nt0 << " | ";
	cout << endl;
	cout << "-----------------------------------------" << endl;
	
	float nt2sum(0.), nt10sum(0.), nt01sum(0.), nt0sum(0.);
	for(size_t i = 0; i < fMCBG.size(); ++i){
		int index = fMCBG[i];
		channel *cha = &fSamples[index].mumu;
		numberset numbers = cha->numbers;
		float scale = fLumiNorm / fSamples[index].lumi;
		nt2sum += scale*numbers.nt2;
		nt10sum += scale*numbers.nt10;
		nt01sum += scale*numbers.nt01;
		nt0sum += scale*numbers.nt0;
		cout << setw(9) << fSamples[index].sname << " | ";
		cout << setw(7)  << setprecision(2) << scale*numbers.nt2  << " | ";
		cout << setw(7)  << setprecision(2) << scale*numbers.nt10 << " | ";
		cout << setw(7)  << setprecision(2) << scale*numbers.nt0  << " | ";
		cout << endl;
	}	
	cout << "-----------------------------------------" << endl;
	cout << setw(9) << "MC sum" << " | ";
	cout << setw(7) << setprecision(2) << nt2sum  << " | ";
	cout << setw(7) << setprecision(2) << nt10sum << " | ";
	cout << setw(7) << setprecision(2) << nt0sum << " | ";
	cout << endl;
	cout << "-----------------------------------------" << endl;
	cout << setw(9) << fSamples[LM0].sname << " | ";
	cout << setw(7)  << setprecision(2) << fLumiNorm / fSamples[LM0].lumi * fSamples[LM0].mumu.numbers.nt2  << " | ";
	cout << setw(7)  << setprecision(2) << fLumiNorm / fSamples[LM0].lumi * fSamples[LM0].mumu.numbers.nt10 << " | ";
	cout << setw(7)  << setprecision(2) << fLumiNorm / fSamples[LM0].lumi * fSamples[LM0].mumu.numbers.nt0  << " | ";
	cout << endl << endl;
	cout << "--------------------------------------------------------------" << endl;
	cout << "  Predictions:" << endl;
	cout << "--------------------------------------------------------------" << endl;
	cout << "  Npp:           " << setw(7) << setprecision(3) << nevFP[0];
	cout << " +/- "             << setw(7) << setprecision(3) << nevFPEstat[0];
	cout << " (stat) +/- "      << setw(7) << setprecision(3) << nevFPEsyst[0] << " (syst)" << endl;
	cout << "  Nfp:           " << setw(7) << setprecision(3) << nevFP[1];
	cout << " +/- "             << setw(7) << setprecision(3) << nevFPEstat[1];
	cout << " (stat) +/- "      << setw(7) << setprecision(3) << nevFPEsyst[1] << " (syst)" << endl;
	cout << "  Nff:           " << setw(7) << setprecision(3) << nevFP[2];
	cout << " +/- "             << setw(7) << setprecision(3) << nevFPEstat[2];
	cout << " (stat) +/- "      << setw(7) << setprecision(3) << nevFPEsyst[2] << " (syst)" << endl;
	cout << "  Total fakes:   " << setw(7) << setprecision(3) << nevFP[1]+nevFP[2];
	cout << " +/- "             << setw(7) << setprecision(3) << TMath::Sqrt(nevFPEstat[1]*nevFPEstat[1] + nevFPEstat[2]*nevFPEstat[2]);
	cout << " (stat) +/- "      << setw(7) << setprecision(3) << TMath::Sqrt(nevFPEsyst[1]*nevFPEsyst[1] + nevFPEsyst[2]*nevFPEsyst[2]) << " (syst)" << endl;
	cout << "--------------------------------------------------------------" << endl;
	cout << "/////////////////////////////////////////////////////////////////////////////" << endl;
	delete fFPRatios;
}

//____________________________________________________________________________
vector<TH1D*> MuonPlotter::MuMuFPPrediction(TH2D* H_fratio, TH2D* H_pratio, TH2D* H_nt2mes, TH2D* H_nt1mes, TH2D* H_nt0mes, bool output){
	///////////////////////////////////////////////////////////////////////////
	// Note: Careful, this is only a workaround at the moment, and is only
	//       really valid for one single eta bin!
	//       In the future we should rewrite the interface to FPRatios and
	//       pass it the ratios directly to have full control
	///////////////////////////////////////////////////////////////////////////
	vector<TH1D*> res;

	TH1D *H_dummy    = new TH1D("Dummyhist", "Dummy",                      H_nt2mes->GetNbinsX(), H_nt2mes->GetXaxis()->GetXbins()->GetArray());
	TH1D *H_nsigpred = new TH1D("Nsigpred", "Predicted N_sig in Pt1 bins", H_nt2mes->GetNbinsX(), H_nt2mes->GetXaxis()->GetXbins()->GetArray());
	TH1D *H_nfppred  = new TH1D("Nfppred",  "Predicted N_fp in Pt1 bins",  H_nt2mes->GetNbinsX(), H_nt2mes->GetXaxis()->GetXbins()->GetArray());
	TH1D *H_nffpred  = new TH1D("Nffpred",  "Predicted N_ff in Pt1 bins",  H_nt2mes->GetNbinsX(), H_nt2mes->GetXaxis()->GetXbins()->GetArray());	
	// TH1D *H_dummy    = new TH1D("Dummyhist", "Dummy", getNMuPt2Bins(), getMuPt2Bins());
	// TH1D *H_nsigpred = new TH1D("Nsigpred", "Predicted N_sig in Pt1 bins", getNMuPtBins(), getMuPtBins());
	// TH1D *H_nfppred  = new TH1D("Nfppred",  "Predicted N_fp in Pt1 bins",  getNMuPtBins(), getMuPtBins());
	// TH1D *H_nffpred  = new TH1D("Nffpred",  "Predicted N_ff in Pt1 bins",  getNMuPtBins(), getMuPtBins());	

	if(fVerbose > 2){
		cout << " Found " << H_nt2mes->GetEntries() << " events with tight-tight (Nt2)" << endl;
		cout << " Found " << H_nt1mes->GetEntries() << " events with tight-loose (Nt1)" << endl;
		cout << " Found " << H_nt0mes->GetEntries() << " events with loose-loose (Nt0)" << endl;
	}

	for(size_t i = 1; i <= getNMuPt2Bins(); ++i){
		double pt1 = H_dummy->GetBinCenter(i);
		if(fVerbose > 2){
			cout << "=======================================" << endl;
			cout << "Pt1 = " << pt1 << endl;
		}
		double eta1 = 0.0;
		double npppred(0.0), npppredEstat2(0.0), npppredEsyst2(0.0);
		double nfppred(0.0), nfppredEstat2(0.0), nfppredEsyst2(0.0);
		double nffpred(0.0), nffpredEstat2(0.0), nffpredEsyst2(0.0);
		for(size_t j = 1; j <= getNMuPt2Bins(); ++j){
			if(fVerbose > 2) cout << " --------" << endl;
			double pt2 = H_dummy->GetBinCenter(j);

			double eta2 = 0.0;
			double nt2 = H_nt2mes->GetBinContent(i,j);
			double nt1 = H_nt1mes->GetBinContent(i,j);
			double nt0 = H_nt0mes->GetBinContent(i,j);

			if(fVerbose > 2) cout << "   Pt2 = " << pt2 << endl;
			if(fVerbose > 2) cout << "   nt2: " << nt2 << "  nt1: " << nt1 << "  nt0: " << nt0 << endl;

			FPRatios *fFPRatios = new FPRatios();
			fFPRatios->SetVerbose(fVerbose);
			fFPRatios->SetMuFratios(H_fratio);
			fFPRatios->SetMuPratios(H_pratio);
			vector<double> nt;
			nt.push_back(nt0); nt.push_back(nt1); nt.push_back(nt2);
			fFPRatios->NevtTopol(0, 2, nt);

			vector<double> vpt, veta;
			vpt.push_back(pt1); vpt.push_back(pt2);
			veta.push_back(eta1); veta.push_back(eta2);

			vector<double> nevFP = fFPRatios->NevtPass(vpt, veta);
			vector<double> nevFPEstat = fFPRatios->NevtPassErrStat();
			vector<double> nevFPEsyst = fFPRatios->NevtPassErrSyst();
			npppred += nevFP[0];
			nfppred += nevFP[1];
			nffpred += nevFP[2];
			npppredEstat2 += nevFPEstat[0]*nevFPEstat[0];
			npppredEsyst2 += nevFPEsyst[0]*nevFPEsyst[0];
			nfppredEstat2 += nevFPEstat[1]*nevFPEstat[1];
			nfppredEsyst2 += nevFPEsyst[1]*nevFPEsyst[1];
			nffpredEstat2 += nevFPEstat[2]*nevFPEstat[2];
			nffpredEsyst2 += nevFPEsyst[2]*nevFPEsyst[2];
			delete fFPRatios;
		}
		Int_t bin = H_nsigpred->FindBin(pt1);
		H_nsigpred->SetBinContent(bin, npppred);
		H_nfppred ->SetBinContent(bin, nfppred);
		H_nffpred ->SetBinContent(bin, nffpred);
		H_nsigpred->SetBinError(bin, sqrt(npppredEstat2 + npppredEsyst2));
		H_nfppred ->SetBinError(bin, sqrt(nfppredEstat2 + nfppredEsyst2));
		H_nffpred ->SetBinError(bin, sqrt(nffpredEstat2 + nffpredEsyst2));
	}	

	if(fVerbose > 2) cout << " Predict " << H_nsigpred->Integral() << " signal events (Nsig = p^2*Npp) from this sample" << endl;
	if(fVerbose > 2) cout << " Predict " << H_nfppred->Integral() <<  " fake-prompt events (f*p*Nfp) from this sample" << endl;
	if(fVerbose > 2) cout << " Predict " << H_nffpred->Integral() <<  " fake-fake events (f*f*Nff) from this sample" << endl;
	// Output
	H_nsigpred->SetXTitle(convertVarName("MuPt[1]"));
	if(output){
		H_nt2mes->SetXTitle(convertVarName("MuPt[1]"));
		H_nt2mes->SetYTitle(convertVarName("MuPt[1]"));
		H_nt1mes->SetXTitle(convertVarName("MuPt[1]"));
		H_nt1mes->SetYTitle(convertVarName("MuPt[1]"));
		H_nt0mes->SetXTitle(convertVarName("MuPt[1]"));
		H_nt0mes->SetYTitle(convertVarName("MuPt[1]"));
		// if(H_nsigpred->GetMinimum() < 0) H_nsigpred->SetMaximum(0);
		// if(H_nsigpred->GetMinimum() > 0) H_nsigpred->SetMinimum(0);
		printHisto(H_nsigpred, H_nsigpred->GetName(), H_nsigpred->GetTitle());
		printHisto(H_nt2mes, H_nt2mes->GetName(), H_nt2mes->GetTitle(), "colz");
		printHisto(H_nt1mes, H_nt1mes->GetName(), H_nt1mes->GetTitle(), "colz");
		printHisto(H_nt0mes, H_nt0mes->GetName(), H_nt0mes->GetTitle(), "colz");
	}
	res.push_back(H_nsigpred);
	res.push_back(H_nfppred);
	res.push_back(H_nffpred);
	return res;
}

//____________________________________________________________________________
void MuonPlotter::fillYields(){
	// MuMu Channel
	fCurrentChannel = Muon;
	int mu1(-1), mu2(-1);
	bool isdata = fSamples[fCurrentSample].isdata;
	if( (isdata && isSSLLMuEventTRG(mu1, mu2)) || (!isdata && isSSLLMuEvent(mu1, mu2)) ){
	// if(isSSLLMuEventTRG()){
		if(  isTightMuon(mu1) &&  isTightMuon(mu2) ){ // Tight-tight
			fCounters[fCurrentSample][Muon].fill(" ... first muon passes tight cut");
			fCounters[fCurrentSample][Muon].fill(" ... second muon passes tight cut");
			fCounters[fCurrentSample][Muon].fill(" ... both muons pass tight cut");
			fSamples[fCurrentSample].mumu.nthistos.h_nt2    ->Fill(MuPt[mu1], MuPt[mu2]);
			fSamples[fCurrentSample].mumu.nthistos.h_nt2_pt ->Fill(MuPt[mu1]);
			fSamples[fCurrentSample].mumu.nthistos.h_nt2_eta->Fill(MuEta[mu1]);
			if(isdata) fOUTSTREAM << " Mu/Mu Tight-Tight event in run " << setw(7) << Run << " event " << setw(13) << Event << " lumisection " << setw(5) << LumiSec << " in dataset " << setw(9) << fSamples[fCurrentSample].sname << endl;
		}
		if(  isTightMuon(mu1) && !isTightMuon(mu2) ){ // Tight-loose
			fCounters[fCurrentSample][Muon].fill(" ... first muon passes tight cut");
			fSamples[fCurrentSample].mumu.nthistos.h_nt10    ->Fill(MuPt[mu1], MuPt[mu2]);
			fSamples[fCurrentSample].mumu.nthistos.h_nt10_pt ->Fill(MuPt[mu1]);
			fSamples[fCurrentSample].mumu.nthistos.h_nt10_eta->Fill(MuEta[mu1]);
		}
		if( !isTightMuon(mu1) &&  isTightMuon(mu2) ){ // Loose-tight
			fCounters[fCurrentSample][Muon].fill(" ... second muon passes tight cut");
			fSamples[fCurrentSample].mumu.nthistos.h_nt10    ->Fill(MuPt[mu2], MuPt[mu1]);
			fSamples[fCurrentSample].mumu.nthistos.h_nt10_pt ->Fill(MuPt[mu2]);
			fSamples[fCurrentSample].mumu.nthistos.h_nt10_eta->Fill(MuEta[mu2]);
		}
		if( !isTightMuon(mu1) && !isTightMuon(mu2) ){ // Loose-loose
			fSamples[fCurrentSample].mumu.nthistos.h_nt0    ->Fill(MuPt[mu1], MuPt[mu2]);
			fSamples[fCurrentSample].mumu.nthistos.h_nt0_pt ->Fill(MuPt[mu1]);
			fSamples[fCurrentSample].mumu.nthistos.h_nt0_eta->Fill(MuEta[mu1]);
		}
	}
	if( (isdata && isSigSupMuEventTRG()) || (!isdata && isSigSupMuEvent()) ){ // f Ratio
	// if(isSigSupMuEventTRG()){ // f Ratio
		if( isLooseMuon(0) ){
			fSamples[fCurrentSample].mumu.fhistos.h_nloose    ->Fill(MuPt[0], MuEta[0]);
			fSamples[fCurrentSample].mumu.fhistos.h_nloose_pt ->Fill(MuPt[0]);
			fSamples[fCurrentSample].mumu.fhistos.h_nloose_eta->Fill(MuEta[0]);
		}
		if( isTightMuon(0) ){
			fSamples[fCurrentSample].mumu.fhistos.h_ntight    ->Fill(MuPt[0], MuEta[0]);
			fSamples[fCurrentSample].mumu.fhistos.h_ntight_pt ->Fill(MuPt[0]);
			fSamples[fCurrentSample].mumu.fhistos.h_ntight_eta->Fill(MuEta[0]);
		}
	}
	if( (isdata && isZMuMuEventTRG())    || (!isdata && isZMuMuEvent()) ){ // p Ratio
	// if(isZMuMuEventTRG()){ // p Ratio
		if( isLooseMuon(0) ){
			fSamples[fCurrentSample].mumu.phistos.h_nloose    ->Fill(MuPt[0], MuEta[0]);
			fSamples[fCurrentSample].mumu.phistos.h_nloose_pt ->Fill(MuPt[0]);
			fSamples[fCurrentSample].mumu.phistos.h_nloose_eta->Fill(MuEta[0]);
		}
		if( isTightMuon(0) ){
			fSamples[fCurrentSample].mumu.phistos.h_ntight    ->Fill(MuPt[0], MuEta[0]);
			fSamples[fCurrentSample].mumu.phistos.h_ntight_pt ->Fill(MuPt[0]);
			fSamples[fCurrentSample].mumu.phistos.h_ntight_eta->Fill(MuEta[0]);
		}
	}				
	
	// EE Channel
	fCurrentChannel = Electron;
	int el1(-1), el2(-1);
	if( (isdata && isSSLLElEventTRG(el1, el2)) || (!isdata && isSSLLElEvent(el1, el2)) ){
		if(  isTightElectron(el1) &&  isTightElectron(el2) ){ // Tight-tight
			fCounters[fCurrentSample][Electron].fill(" ... first electron passes tight cut");
			fCounters[fCurrentSample][Electron].fill(" ... second electron passes tight cut");
			fCounters[fCurrentSample][Electron].fill(" ... both electrons pass tight cut");
			fSamples[fCurrentSample].ee.nthistos.h_nt2    ->Fill(ElPt[el1], ElPt[el2]);
			fSamples[fCurrentSample].ee.nthistos.h_nt2_pt ->Fill(ElPt[el1]);
			fSamples[fCurrentSample].ee.nthistos.h_nt2_eta->Fill(ElEta[el1]);
			if(isdata) fOUTSTREAM << " E/E Tight-Tight event in run   " << setw(7) << Run << " event " << setw(13) << Event << " lumisection " << setw(5) << LumiSec << " in dataset " << setw(9) << fSamples[fCurrentSample].sname << endl;
		}
		if(  isTightElectron(el1) && !isTightElectron(el2) ){ // Tight-loose
			fCounters[fCurrentSample][Electron].fill(" ... first electron passes tight cut");
			fSamples[fCurrentSample].ee.nthistos.h_nt10    ->Fill(ElPt[el1], ElPt[el2]);
			fSamples[fCurrentSample].ee.nthistos.h_nt10_pt ->Fill(ElPt[el1]);
			fSamples[fCurrentSample].ee.nthistos.h_nt10_eta->Fill(ElEta[el1]);
		}
		if( !isTightElectron(el1) &&  isTightElectron(el2) ){ // Loose-tight
			fCounters[fCurrentSample][Electron].fill(" ... second electron passes tight cut");
			fSamples[fCurrentSample].ee.nthistos.h_nt10    ->Fill(ElPt[el2], ElPt[el1]);
			fSamples[fCurrentSample].ee.nthistos.h_nt10_pt ->Fill(ElPt[el2]);
			fSamples[fCurrentSample].ee.nthistos.h_nt10_eta->Fill(ElEta[el2]);
		}
		if( !isTightElectron(el1) && !isTightElectron(el2) ){ // Loose-loose
			fSamples[fCurrentSample].ee.nthistos.h_nt0    ->Fill(ElPt[el1], ElPt[el2]);
			fSamples[fCurrentSample].ee.nthistos.h_nt0_pt ->Fill(ElPt[el1]);
			fSamples[fCurrentSample].ee.nthistos.h_nt0_eta->Fill(ElEta[el1]);
		}
	}
	if( (isdata && isSigSupElEventTRG())   || (!isdata && isSigSupElEvent()) ){ // f Ratio
		if( isLooseElectron(0) ){
			fSamples[fCurrentSample].ee.fhistos.h_nloose    ->Fill(ElPt[0], ElEta[0]);
			fSamples[fCurrentSample].ee.fhistos.h_nloose_pt ->Fill(ElPt[0]);
			fSamples[fCurrentSample].ee.fhistos.h_nloose_eta->Fill(ElEta[0]);
		}
		if( isTightElectron(0) ){
			fSamples[fCurrentSample].ee.fhistos.h_ntight    ->Fill(ElPt[0], ElEta[0]);
			fSamples[fCurrentSample].ee.fhistos.h_ntight_pt ->Fill(ElPt[0]);
			fSamples[fCurrentSample].ee.fhistos.h_ntight_eta->Fill(ElEta[0]);
		}
	}
	int elind;
	if( (isdata && isZElElEventTRG(elind)) || (!isdata && isZElElEvent(elind)) ){ // p Ratio
		if( isLooseElectron(elind) ){
			fSamples[fCurrentSample].ee.phistos.h_nloose    ->Fill(ElPt [elind], ElEta[elind]);
			fSamples[fCurrentSample].ee.phistos.h_nloose_pt ->Fill(ElPt [elind]);
			fSamples[fCurrentSample].ee.phistos.h_nloose_eta->Fill(ElEta[elind]);
		}
		if( isTightElectron(elind) ){
			fSamples[fCurrentSample].ee.phistos.h_ntight    ->Fill(ElPt [elind], ElEta[elind]);
			fSamples[fCurrentSample].ee.phistos.h_ntight_pt ->Fill(ElPt [elind]);
			fSamples[fCurrentSample].ee.phistos.h_ntight_eta->Fill(ElEta[elind]);
		}
	}
				
	// EMu Channel
	fCurrentChannel = EMu;
	int mu(-1), el(-1);
	if( (isdata && isSSLLElMuEventTRG(mu, el)) || (!isdata && isSSLLElMuEvent(mu, el)) ){
		if(  isTightElectron(el) &&  isTightMuon(mu) ){ // Tight-tight
			fCounters[fCurrentSample][EMu].fill(" ... muon passes tight cut");
			fCounters[fCurrentSample][EMu].fill(" ... electron passes tight cut");
			fCounters[fCurrentSample][EMu].fill(" ... both e and mu pass tight cuts");
			fSamples[fCurrentSample].emu.nthistos.h_nt2    ->Fill(MuPt [mu], ElPt[el]);
			fSamples[fCurrentSample].emu.nthistos.h_nt2_pt ->Fill(MuPt [mu]);
			fSamples[fCurrentSample].emu.nthistos.h_nt2_eta->Fill(MuEta[mu]);
			if(isdata) fOUTSTREAM << " E/Mu Tight-Tight event in run  " << setw(7) << Run << " event " << setw(13) << Event << " lumisection " << setw(5) << LumiSec << " in dataset " << setw(9) << fSamples[fCurrentSample].sname << endl;
		}
		if( !isTightElectron(el) &&  isTightMuon(mu) ){ // Tight-loose
			fCounters[fCurrentSample][EMu].fill(" ... muon passes tight cut");
			fSamples[fCurrentSample].emu.nthistos.h_nt10    ->Fill(MuPt [mu], ElPt[el]);
			fSamples[fCurrentSample].emu.nthistos.h_nt10_pt ->Fill(MuPt [mu]);
			fSamples[fCurrentSample].emu.nthistos.h_nt10_eta->Fill(MuEta[mu]);
		}
		if(  isTightElectron(el) && !isTightMuon(mu) ){ // Loose-tight
			fCounters[fCurrentSample][EMu].fill(" ... electron passes tight cut");
			fSamples[fCurrentSample].emu.nthistos.h_nt01    ->Fill(MuPt [mu], ElPt[el]);
			fSamples[fCurrentSample].emu.nthistos.h_nt01_pt ->Fill(MuPt [mu]);
			fSamples[fCurrentSample].emu.nthistos.h_nt01_eta->Fill(MuEta[mu]);
		}
		if( !isTightElectron(0) && !isTightMuon(0) ){ // Loose-loose
			fSamples[fCurrentSample].emu.nthistos.h_nt0    ->Fill(MuPt [mu], ElPt[el]);
			fSamples[fCurrentSample].emu.nthistos.h_nt0_pt ->Fill(MuPt [mu]);
			fSamples[fCurrentSample].emu.nthistos.h_nt0_eta->Fill(MuEta[mu]);
		}
	}
}

//____________________________________________________________________________
void MuonPlotter::storeNumbers(){
	storeNumbers(Muon);
	storeNumbers(Electron);
	storeNumbers(EMu);
}
void MuonPlotter::storeNumbers(gChannel chan){
	numberset numbers;
	channel CHAN;
	if(chan = Muon)     CHAN = fSamples[fCurrentSample].mumu;
	if(chan = Electron) CHAN = fSamples[fCurrentSample].ee;
	if(chan = EMu)      CHAN = fSamples[fCurrentSample].emu;
	numbers.nt2  = CHAN.nthistos.h_nt2->GetEntries();
	numbers.nt10 = CHAN.nthistos.h_nt10->GetEntries();
	numbers.nt01 = CHAN.nthistos.h_nt01->GetEntries();
	numbers.nt0  = CHAN.nthistos.h_nt0->GetEntries();
	numbers.nsst = CHAN.fhistos.h_ntight->GetEntries();
	numbers.nssl = CHAN.fhistos.h_nloose->GetEntries();
	numbers.nzt  = CHAN.phistos.h_ntight->GetEntries();
	numbers.nzl  = CHAN.phistos.h_nloose->GetEntries();
	CHAN.numbers = numbers;	
}

//____________________________________________________________________________
void MuonPlotter::initCounters(int sample){
	if(sample < 0) sample = fCurrentSample;
	fCounters[sample][Muon]    .fill("All events",                             0.);
	fCounters[sample][Muon]    .fill(" ... is good run",                       0.);
	fCounters[sample][Muon]    .fill(" ... passes triggers",                   0.);
	fCounters[sample][Muon]    .fill(" ... has at least 2 jets, 1 loose muon", 0.);
	fCounters[sample][Muon]    .fill(" ... has 2 loose muons",                 0.);
	fCounters[sample][Muon]    .fill(" ... passes Z veto",                     0.);
	fCounters[sample][Muon]    .fill(" ... passes Minv veto",                  0.);
	fCounters[sample][Muon]    .fill(" ... passes HT cut",                     0.);
	fCounters[sample][Muon]    .fill(" ... passes MET cut",                    0.);
	if(fChargeSwitch == 0) fCounters[sample][Muon]    .fill(" ... has same-sign muons",     0.);
	if(fChargeSwitch == 1) fCounters[sample][Muon]    .fill(" ... has opposite-sign muons", 0.);
	fCounters[sample][Muon]    .fill(" ... second muon passes pt cut",         0.);
	fCounters[sample][Muon]    .fill(" ... first muon passes pt cut",          0.);
	fCounters[sample][Muon]    .fill(" ... first muon passes tight cut",       0.);
	fCounters[sample][Muon]    .fill(" ... second muon passes tight cut",      0.);
	fCounters[sample][Muon]    .fill(" ... both muons pass tight cut",         0.);

	fCounters[sample][EMu]     .fill("All events",                             0.);
	fCounters[sample][EMu]     .fill(" ... is good run",                       0.);
	fCounters[sample][EMu]     .fill(" ... passes triggers",                   0.);
	fCounters[sample][EMu]     .fill(" ... has at least 1 j, loose e/mu pair", 0.);
	fCounters[sample][EMu]     .fill(" ... passes Z veto",                     0.);
	fCounters[sample][EMu]     .fill(" ... passes Minv veto",                  0.);
	fCounters[sample][EMu]     .fill(" ... passes HT cut",                     0.);
	fCounters[sample][EMu]     .fill(" ... passes MET cut",                    0.);
	if(fChargeSwitch == 0) fCounters[sample][EMu]     .fill(" ... has same-sign electron muon pair", 0.);
	if(fChargeSwitch == 1) fCounters[sample][EMu]     .fill(" ... has opp.-sign electron muon pair", 0.);
	fCounters[sample][EMu]     .fill(" ... muon passes pt cut",                0.);
	fCounters[sample][EMu]     .fill(" ... electron passes pt cut",            0.);
	fCounters[sample][EMu]     .fill(" ... muon passes tight cut",             0.);
	fCounters[sample][EMu]     .fill(" ... electron passes tight cut",         0.);
	fCounters[sample][EMu]     .fill(" ... both e and mu pass tight cuts",     0.);

	fCounters[sample][Electron].fill("All events",                             0.);
	fCounters[sample][Electron].fill(" ... is good run",                       0.);
	fCounters[sample][Electron].fill(" ... passes triggers",                   0.);
	fCounters[sample][Electron].fill(" ... has at least 2 jets, 1 loose el",   0.);
	fCounters[sample][Electron].fill(" ... has 2 loose electrons",             0.);
	fCounters[sample][Electron].fill(" ... passes Z veto",                     0.);
	fCounters[sample][Electron].fill(" ... passes Minv veto",                  0.);
	fCounters[sample][Electron].fill(" ... passes HT cut",                     0.);
	fCounters[sample][Electron].fill(" ... passes MET cut",                    0.);
	if(fChargeSwitch == 0) fCounters[sample][Electron].fill(" ... has same-sign electrons",     0.);
	if(fChargeSwitch == 1) fCounters[sample][Electron].fill(" ... has opposite-sign electrons", 0.);
	fCounters[sample][Electron].fill(" ... second electron passes pt cut",     0.);
	fCounters[sample][Electron].fill(" ... first electron passes pt cut",      0.);
	fCounters[sample][Electron].fill(" ... first electron passes tight cut",   0.);
	fCounters[sample][Electron].fill(" ... second electron passes tight cut",  0.);
	fCounters[sample][Electron].fill(" ... both electrons pass tight cut",     0.);
}

//____________________________________________________________________________
void MuonPlotter::printCutFlows(TString filename){
	// Remove existing cutflow file
	// char cmd[100];
	// sprintf(cmd,"rm -f %s", filename.Data());
	// system(cmd);

	ofstream OUT(filename.Data(), ios::trunc);
	vector<string>::const_iterator countit;
	
	OUT << " Printing Cutflow for Mu/Mu channel..." << endl;
	OUT << "------------------------------------------------------------------------------------------------------------------------------" << endl;
	OUT << " Cutname                                 | ";
	for(gSample i = sample_begin; i < gNSAMPLES; i=gSample(i+1)){
		if(fSamples[i].isdata == 0) continue;
		OUT << setw(9) << fSamples[i].sname;
		OUT << " | ";
	}
	OUT << endl;
	OUT << "------------------------------------------------------------------------------------------------------------------------------" << endl;

    for( countit = fCounters[0][Muon].countNames.begin(); countit != fCounters[0][Muon].countNames.end(); ++countit ){
		OUT << setw(40) << *countit;
		OUT << " | ";
		
		for(gSample i = sample_begin; i < gNSAMPLES; i=gSample(i+1)){
			if(fSamples[i].isdata == 0) continue;
			OUT << setw(9) << setprecision(3) << fCounters[i][Muon].counts(*countit) << " | ";
		}
		OUT << endl;
	}
	OUT << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
	OUT << " Cutname                                 | ";
	for(gSample i = sample_begin; i < gNSAMPLES; i=gSample(i+1)){
		if(fSamples[i].isdata == 1) continue;
		OUT << setw(9) << fSamples[i].sname;
		OUT << " | ";
	}
	OUT << endl;
	OUT << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;

    for( countit = fCounters[0][Muon].countNames.begin(); countit != fCounters[0][Muon].countNames.end(); ++countit ){
		OUT << setw(40) << *countit;
		OUT << " | ";
		
		for(gSample i = sample_begin; i < gNSAMPLES; i=gSample(i+1)){
			if(fSamples[i].isdata == 1) continue;
			OUT << setw(9) << setprecision(3) << fCounters[i][Muon].counts(*countit) << " | ";
		}
		OUT << endl;
	}
	OUT << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
	OUT << endl << endl;
	OUT << " Printing Cutflow for E/Mu channel..." << endl;
	OUT << "------------------------------------------------------------------------------------------------------------------------------" << endl;
	OUT << " Cutname                                 | ";
	for(gSample i = sample_begin; i < gNSAMPLES; i=gSample(i+1)){
		if(fSamples[i].isdata == 0) continue;
		OUT << setw(9) << fSamples[i].sname;
		OUT << " | ";
	}
	OUT << endl;
	OUT << "------------------------------------------------------------------------------------------------------------------------------" << endl;

    for( countit = fCounters[0][EMu].countNames.begin(); countit != fCounters[0][EMu].countNames.end(); ++countit ){
		OUT << setw(40) << *countit;
		OUT << " | ";
		
		for(gSample i = sample_begin; i < gNSAMPLES; i=gSample(i+1)){
			if(fSamples[i].isdata == 0) continue;
			OUT << setw(9) << setprecision(3) << fCounters[i][EMu].counts(*countit) << " | ";
		}
		OUT << endl;
	}
	OUT << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
	OUT << " Cutname                                 | ";
	for(gSample i = sample_begin; i < gNSAMPLES; i=gSample(i+1)){
		if(fSamples[i].isdata == 1) continue;
		OUT << setw(9) << fSamples[i].sname;
		OUT << " | ";
	}
	OUT << endl;
	OUT << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;

    for( countit = fCounters[0][EMu].countNames.begin(); countit != fCounters[0][EMu].countNames.end(); ++countit ){
		OUT << setw(40) << *countit;
		OUT << " | ";
		
		for(gSample i = sample_begin; i < gNSAMPLES; i=gSample(i+1)){
			if(fSamples[i].isdata == 1) continue;
			OUT << setw(9) << setprecision(3) << fCounters[i][EMu].counts(*countit) << " | ";
		}
		OUT << endl;
	}
	OUT << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
	OUT << endl << endl;
	OUT << " Printing Cutflow for E/E channel..." << endl;
	OUT << "------------------------------------------------------------------------------------------------------------------------------" << endl;
	OUT << " Cutname                                 | ";
	for(gSample i = sample_begin; i < gNSAMPLES; i=gSample(i+1)){
		if(fSamples[i].isdata == 0) continue;
		OUT << setw(9) << fSamples[i].sname;
		OUT << " | ";
	}
	OUT << endl;
	OUT << "------------------------------------------------------------------------------------------------------------------------------" << endl;

    for( countit = fCounters[0][Electron].countNames.begin(); countit != fCounters[0][Electron].countNames.end(); ++countit ){
		OUT << setw(40) << *countit;
		OUT << " | ";
		
		for(gSample i = sample_begin; i < gNSAMPLES; i=gSample(i+1)){
			if(fSamples[i].isdata == 0) continue;
			OUT << setw(9) << setprecision(3) << fCounters[i][Electron].counts(*countit) << " | ";
		}
		OUT << endl;
	}
	OUT << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
	OUT << " Cutname                                 | ";
	for(gSample i = sample_begin; i < gNSAMPLES; i=gSample(i+1)){
		if(fSamples[i].isdata == 1) continue;
		OUT << setw(9) << fSamples[i].sname;
		OUT << " | ";
	}
	OUT << endl;
	OUT << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;

    for( countit = fCounters[0][Electron].countNames.begin(); countit != fCounters[0][Electron].countNames.end(); ++countit ){
		OUT << setw(40) << *countit;
		OUT << " | ";
		
		for(gSample i = sample_begin; i < gNSAMPLES; i=gSample(i+1)){
			if(fSamples[i].isdata == 1) continue;
			OUT << setw(9) << setprecision(3) << fCounters[i][Electron].counts(*countit) << " | ";
		}
		OUT << endl;
	}
	OUT << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
	OUT.close();
}

//____________________________________________________________________________
void MuonPlotter::printYields(gChannel chan){ printYields(chan, fAllSamples); }
void MuonPlotter::printYields(gChannel chan, float scale){ printYields(chan, fAllSamples, scale); }
void MuonPlotter::printYields(gChannel chan, int sample, float lumiscale){ vector<int> samples; samples.push_back(sample); printYields(chan, samples, lumiscale); }
void MuonPlotter::printYields(gChannel chan, vector<int> samples, float lumiscale){
	cout << setfill('-') << setw(94) << "-" << endl;
	TString name = "Mu/Mu";
	if(chan == Electron) name = "E/E";
	if(chan == EMu) name = "E/Mu";
	cout << " Printing yields for " << name << " channel..." << endl;
	if(lumiscale > -1.0) cout << " Numbers scaled to " << lumiscale << " /pb" << endl;
	cout << "        Name |   Nt2   |   Nt10  |   Nt01  |   Nt0   |   Nsst  |   Nssl  |   NZ t  |   NZ l  |" << endl;
	cout << setfill('-') << setw(94) << "-" << endl;
	cout << setfill(' ');
	for(size_t i = 0; i < samples.size(); ++i){
		int index = samples[i];
		channel *cha = &fSamples[index].mumu;
		if(chan == Electron) cha = &fSamples[index].ee;
		if(chan == EMu)      cha = &fSamples[index].emu;
		numberset numbers = cha->numbers;
		float scale = lumiscale / fSamples[index].lumi;
		if(scale < 0) scale = 1;
		if(fSamples[index].isdata) scale = 1;
		cout << setw(12) << fSamples[index].sname << " |";
		cout << setw(8)  << setprecision(3) << scale*numbers.nt2  << " |";
		cout << setw(8)  << setprecision(3) << scale*numbers.nt10 << " |";
		cout << setw(8)  << setprecision(3) << scale*numbers.nt01 << " |";
		cout << setw(8)  << setprecision(3) << scale*numbers.nt0  << " |";
		cout << setw(8)  << setprecision(3) << scale*numbers.nsst << " |"; 
		cout << setw(8)  << setprecision(3) << scale*numbers.nssl << " |";
		cout << setw(8)  << setprecision(3) << scale*numbers.nzt  << " |";
		cout << setw(8)  << setprecision(3) << scale*numbers.nzl  << " |";
		cout << endl;
	}

	cout << setfill('-') << setw(94) << "-" << endl;
}

void MuonPlotter::printYieldsShort(float luminorm){
	vector<int> musamples;
	vector<int> elsamples;
	vector<int> emusamples;

	if(fSelectionSwitch == 0){
		cout << "---------------------------------------------------" << endl;
		cout << "Printing yields for UCSD/UCSB/FNAL selection" << endl;
		musamples = fMuData;
		elsamples = fEGData;
		emusamples.push_back(MuA);
		emusamples.push_back(MuB);
		emusamples.push_back(EGA);
		emusamples.push_back(EGB);
	}
	if(fSelectionSwitch == 1){
		cout << "---------------------------------------------------" << endl;
		cout << "Printing yields for Florida selection" << endl;
		musamples  = fJMData;
		elsamples  = fJMData;
		emusamples = fJMData;
	}

	float nt2_mumu(0.),    nt10_mumu(0.),    nt0_mumu(0.);
	float nt2_mumu_e2(0.), nt10_mumu_e2(0.), nt0_mumu_e2(0.);
	float nt2_emu(0.),    nt10_emu(0.),    nt01_emu(0.),    nt0_emu(0.);
	float nt2_emu_e2(0.), nt10_emu_e2(0.), nt01_emu_e2(0.), nt0_emu_e2(0.);	
	float nt2_ee(0.),    nt10_ee(0.),    nt0_ee(0.);
	float nt2_ee_e2(0.), nt10_ee_e2(0.), nt0_ee_e2(0.);
	

	for(size_t i = 0; i < musamples.size(); ++i){
		int index = musamples[i];
		channel *mumu = &fSamples[index].mumu;
		nt2_mumu  += mumu->numbers.nt2;
		nt10_mumu += mumu->numbers.nt10;
		nt0_mumu  += mumu->numbers.nt0;
		nt2_mumu_e2  += mumu->numbers.nt2;
		nt10_mumu_e2 += mumu->numbers.nt10;
		nt0_mumu_e2  += mumu->numbers.nt0;
	}		
	for(size_t i = 0; i < emusamples.size(); ++i){
		int index = emusamples[i];
		channel *emu = &fSamples[index].emu;
		nt2_emu  += emu->numbers.nt2;
		nt10_emu += emu->numbers.nt10;
		nt01_emu += emu->numbers.nt01;
		nt0_emu  += emu->numbers.nt0;
		nt2_emu_e2  += emu->numbers.nt2;
		nt10_emu_e2 += emu->numbers.nt10;
		nt01_emu_e2 += emu->numbers.nt01;
		nt0_emu_e2  += emu->numbers.nt0;
	}		
	for(size_t i = 0; i < elsamples.size(); ++i){
		int index = elsamples[i];
		channel *ee = &fSamples[index].ee;
		nt2_ee  += ee->numbers.nt2;
		nt10_ee += ee->numbers.nt10;
		nt0_ee  += ee->numbers.nt0;
		nt2_ee_e2  += ee->numbers.nt2;
		nt10_ee_e2 += ee->numbers.nt10;
		nt0_ee_e2  += ee->numbers.nt0;
	}
	cout << "-------------------------------------------------------------------------------------------------------------------" << endl;
	cout << "          ||            Mu/Mu            ||                   E/Mu                ||             E/E             ||" << endl;
	cout << "  YIELDS  ||   Nt2   |   Nt1   |   Nt0   ||   Nt2   |   Nt10  |   Nt01  |   Nt0   ||   Nt2   |   Nt1   |   Nt0   ||" << endl;
	cout << "-------------------------------------------------------------------------------------------------------------------" << endl;
	
	float nt2sum_mumu(0.), nt10sum_mumu(0.), nt0sum_mumu(0.);
	float nt2sum_emu(0.), nt10sum_emu(0.), nt01sum_emu(0.), nt0sum_emu(0.);
	float nt2sum_ee(0.), nt10sum_ee(0.), nt0sum_ee(0.);
	for(size_t i = 0; i < fMCBG.size(); ++i){
		int index = fMCBG[i];
		float scale = luminorm / fSamples[index].lumi;
		if(luminorm < 0) scale = 1;
		nt2sum_mumu  += scale*fSamples[index].mumu.numbers.nt2;
		nt10sum_mumu += scale*fSamples[index].mumu.numbers.nt10;
		nt0sum_mumu  += scale*fSamples[index].mumu.numbers.nt0;
		nt2sum_emu   += scale*fSamples[index].emu .numbers.nt2;
		nt10sum_emu  += scale*fSamples[index].emu .numbers.nt10;
		nt01sum_emu  += scale*fSamples[index].emu .numbers.nt01;
		nt0sum_emu   += scale*fSamples[index].emu .numbers.nt0;
		nt2sum_ee    += scale*fSamples[index].ee  .numbers.nt2;
		nt10sum_ee   += scale*fSamples[index].ee  .numbers.nt10;
		nt0sum_ee    += scale*fSamples[index].ee  .numbers.nt0;

		cout << setw(9) << fSamples[index].sname << " || ";
		cout << setw(7)  << scale*fSamples[index].mumu.numbers.nt2  << " | ";
		cout << setw(7)  << scale*fSamples[index].mumu.numbers.nt10 << " | ";
		cout << setw(7)  << scale*fSamples[index].mumu.numbers.nt0  << " || ";
		cout << setw(7)  << scale*fSamples[index].emu.numbers.nt2   << " | ";
		cout << setw(7)  << scale*fSamples[index].emu.numbers.nt10  << " | ";
		cout << setw(7)  << scale*fSamples[index].emu.numbers.nt01  << " | ";
		cout << setw(7)  << scale*fSamples[index].emu.numbers.nt0   << " || ";
		cout << setw(7)  << scale*fSamples[index].ee.numbers.nt2    << " | ";
		cout << setw(7)  << scale*fSamples[index].ee.numbers.nt10   << " | ";
		cout << setw(7)  << scale*fSamples[index].ee.numbers.nt0    << " || ";
		cout << endl;
	}
	if(luminorm > 0){
		cout << "-------------------------------------------------------------------------------------------------------------------" << endl;
		cout << setw(9) << "MC sum" << " || ";
		cout << setw(7) << nt2sum_mumu  << " | ";
		cout << setw(7) << nt10sum_mumu << " | ";
		cout << setw(7) << nt0sum_mumu  << " || ";
		cout << setw(7) << nt2sum_emu   << " | ";
		cout << setw(7) << nt10sum_emu  << " | ";
		cout << setw(7) << nt01sum_emu  << " | ";
		cout << setw(7) << nt0sum_emu   << " || ";
		cout << setw(7) << nt2sum_ee    << " | ";
		cout << setw(7) << nt10sum_ee   << " | ";
		cout << setw(7) << nt0sum_ee    << " || ";
		cout << endl;
	}
	cout << "-------------------------------------------------------------------------------------------------------------------" << endl;
	cout << setw(9) << fSamples[LM0].sname << " || ";
	float scale = luminorm / fSamples[LM0].lumi;
	if(luminorm < 0) scale = 1;
	cout << setw(7) << scale * fSamples[LM0].mumu.numbers.nt2  << " | ";
	cout << setw(7) << scale * fSamples[LM0].mumu.numbers.nt10 << " | ";
	cout << setw(7) << scale * fSamples[LM0].mumu.numbers.nt0  << " || ";
	cout << setw(7) << scale * fSamples[LM0].emu .numbers.nt2  << " | ";
	cout << setw(7) << scale * fSamples[LM0].emu .numbers.nt10 << " | ";
	cout << setw(7) << scale * fSamples[LM0].emu .numbers.nt01 << " | ";
	cout << setw(7) << scale * fSamples[LM0].emu .numbers.nt0  << " || ";
	cout << setw(7) << scale * fSamples[LM0].ee  .numbers.nt2  << " | ";
	cout << setw(7) << scale * fSamples[LM0].ee  .numbers.nt10 << " | ";
	cout << setw(7) << scale * fSamples[LM0].ee  .numbers.nt0  << " || ";
	cout << endl;
	cout << "-------------------------------------------------------------------------------------------------------------------" << endl;
	cout << setw(9) << "data"  << " || ";
	cout << setw(7) << nt2_mumu  << " | ";
	cout << setw(7) << nt10_mumu << " | ";
	cout << setw(7) << nt0_mumu  << " || ";
	cout << setw(7) << nt2_emu   << " | ";
	cout << setw(7) << nt10_emu  << " | ";
	cout << setw(7) << nt01_emu  << " | ";
	cout << setw(7) << nt0_emu   << " || ";
	cout << setw(7) << nt2_ee    << " | ";
	cout << setw(7) << nt10_ee   << " | ";
	cout << setw(7) << nt0_ee    << " || ";
	cout << endl;
	cout << "-------------------------------------------------------------------------------------------------------------------" << endl;
	
}

void MuonPlotter::bookHistos(){
	for(size_t i = 0; i < fSamples.size(); ++i){
		TString name = fSamples[i].sname;
		fSamples[i].mumu.nthistos.h_nt2       = new TH2D(name + "_MuMu_NT2",        "NT2",        getNMuPt2Bins(), getMuPt2Bins(), getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].mumu.nthistos.h_nt2_pt    = new TH1D(name + "_MuMu_NT2_pt",     "NT2",        getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].mumu.nthistos.h_nt2_eta   = new TH1D(name + "_MuMu_NT2_eta",    "NT2",        getNMuEtaBins(), getMuEtaBins());
		fSamples[i].mumu.nthistos.h_nt10      = new TH2D(name + "_MuMu_NT10",       "NT10",       getNMuPt2Bins(), getMuPt2Bins(), getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].mumu.nthistos.h_nt10_pt   = new TH1D(name + "_MuMu_NT10_pt",    "NT10 vs pt", getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].mumu.nthistos.h_nt10_eta  = new TH1D(name + "_MuMu_NT10_eta",   "NT10 vs eta",getNMuEtaBins(), getMuEtaBins());
		fSamples[i].mumu.nthistos.h_nt01      = new TH2D(name + "_MuMu_NT01",       "NT01",       getNMuPt2Bins(), getMuPt2Bins(), getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].mumu.nthistos.h_nt01_pt   = new TH1D(name + "_MuMu_NT01_pt",    "NT01 vs pt", getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].mumu.nthistos.h_nt01_eta  = new TH1D(name + "_MuMu_NT01_eta",   "NT01 vs eta",getNMuEtaBins(), getMuEtaBins());
		fSamples[i].mumu.nthistos.h_nt0       = new TH2D(name + "_MuMu_NT0",        "NT0",        getNMuPt2Bins(), getMuPt2Bins(), getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].mumu.nthistos.h_nt0_pt    = new TH1D(name + "_MuMu_NT0_pt",     "NT0 vs pt",  getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].mumu.nthistos.h_nt0_eta   = new TH1D(name + "_MuMu_NT0_eta",    "NT0 vs eta", getNMuEtaBins(), getMuEtaBins());
		fSamples[i].mumu.fhistos.h_ntight     = new TH2D(name + "_MuMu_fTight",     "NTight Muons for sig. supp. selection",      getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins());
		fSamples[i].mumu.fhistos.h_nloose     = new TH2D(name + "_MuMu_fLoose",     "NLoose Muons for sig. supp. selection",      getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins());
		fSamples[i].mumu.fhistos.h_ntight_pt  = new TH1D(name + "_MuMu_fTight_pt",  "NTight Muons for sig. supp. selection",      getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].mumu.fhistos.h_nloose_pt  = new TH1D(name + "_MuMu_fLoose_pt",  "NLoose Muons for sig. supp. selection",      getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].mumu.fhistos.h_ntight_eta = new TH1D(name + "_MuMu_fTight_eta", "NTight Muons for sig. supp. selection",      getNMuEtaBins(), getMuEtaBins());
		fSamples[i].mumu.fhistos.h_nloose_eta = new TH1D(name + "_MuMu_fLoose_eta", "NLoose Muons for sig. supp. selection",      getNMuEtaBins(), getMuEtaBins());
		fSamples[i].mumu.phistos.h_ntight     = new TH2D(name + "_MuMu_pTight",     "NTight Muons for Z decay selection",      getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins());
		fSamples[i].mumu.phistos.h_nloose     = new TH2D(name + "_MuMu_pLoose",     "NLoose Muons for Z decay selection",      getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins());
		fSamples[i].mumu.phistos.h_ntight_pt  = new TH1D(name + "_MuMu_pTight_pt",  "NTight Muons for Z decay selection",      getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].mumu.phistos.h_nloose_pt  = new TH1D(name + "_MuMu_pLoose_pt",  "NLoose Muons for Z decay selection",      getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].mumu.phistos.h_ntight_eta = new TH1D(name + "_MuMu_pTight_eta", "NTight Muons for Z decay selection",      getNMuEtaBins(), getMuEtaBins());
		fSamples[i].mumu.phistos.h_nloose_eta = new TH1D(name + "_MuMu_pLoose_eta", "NLoose Muons for Z decay selection",      getNMuEtaBins(), getMuEtaBins());		

		fSamples[i].mumu.nthistos.h_nt2->Sumw2();  fSamples[i].mumu.nthistos.h_nt2_pt->Sumw2();  fSamples[i].mumu.nthistos.h_nt2_eta->Sumw2();
		fSamples[i].mumu.nthistos.h_nt10->Sumw2(); fSamples[i].mumu.nthistos.h_nt10_pt->Sumw2(); fSamples[i].mumu.nthistos.h_nt10_eta->Sumw2();
		fSamples[i].mumu.nthistos.h_nt01->Sumw2(); fSamples[i].mumu.nthistos.h_nt01_pt->Sumw2(); fSamples[i].mumu.nthistos.h_nt01_eta->Sumw2();
		fSamples[i].mumu.nthistos.h_nt0->Sumw2();  fSamples[i].mumu.nthistos.h_nt0_pt->Sumw2();  fSamples[i].mumu.nthistos.h_nt0_eta->Sumw2();

		fSamples[i].mumu.fhistos.h_ntight     ->Sumw2(); fSamples[i].mumu.fhistos.h_nloose     ->Sumw2();
		fSamples[i].mumu.fhistos.h_ntight_pt  ->Sumw2(); fSamples[i].mumu.fhistos.h_nloose_pt  ->Sumw2();
		fSamples[i].mumu.fhistos.h_ntight_eta ->Sumw2(); fSamples[i].mumu.fhistos.h_nloose_eta ->Sumw2();

		fSamples[i].mumu.phistos.h_ntight     ->Sumw2(); fSamples[i].mumu.phistos.h_nloose     ->Sumw2();
		fSamples[i].mumu.phistos.h_ntight_pt  ->Sumw2(); fSamples[i].mumu.phistos.h_nloose_pt  ->Sumw2();
		fSamples[i].mumu.phistos.h_ntight_eta ->Sumw2(); fSamples[i].mumu.phistos.h_nloose_eta ->Sumw2();

		fSamples[i].emu.nthistos.h_nt2       = new TH2D(name + "_EMu_NT2",        "NT2",        getNMuPt2Bins(), getMuPt2Bins(), getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].emu.nthistos.h_nt2_pt    = new TH1D(name + "_EMu_NT2_pt",     "NT2",        getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].emu.nthistos.h_nt2_eta   = new TH1D(name + "_EMu_NT2_eta",    "NT2",        getNMuEtaBins(), getMuEtaBins());
		fSamples[i].emu.nthistos.h_nt10      = new TH2D(name + "_EMu_NT10",       "NT10",       getNMuPt2Bins(), getMuPt2Bins(), getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].emu.nthistos.h_nt10_pt   = new TH1D(name + "_EMu_NT10_pt",    "NT10 vs pt", getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].emu.nthistos.h_nt10_eta  = new TH1D(name + "_EMu_NT10_eta",   "NT10 vs eta",getNMuEtaBins(), getMuEtaBins());
		fSamples[i].emu.nthistos.h_nt01      = new TH2D(name + "_EMu_NT01",       "NT01",       getNMuPt2Bins(), getMuPt2Bins(), getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].emu.nthistos.h_nt01_pt   = new TH1D(name + "_EMu_NT01_pt",    "NT01 vs pt", getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].emu.nthistos.h_nt01_eta  = new TH1D(name + "_EMu_NT01_eta",   "NT01 vs eta",getNMuEtaBins(), getMuEtaBins());
		fSamples[i].emu.nthistos.h_nt0       = new TH2D(name + "_EMu_NT0",        "NT0",        getNMuPt2Bins(), getMuPt2Bins(), getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].emu.nthistos.h_nt0_pt    = new TH1D(name + "_EMu_NT0_pt",     "NT0 vs pt",  getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].emu.nthistos.h_nt0_eta   = new TH1D(name + "_EMu_NT0_eta",    "NT0 vs eta", getNMuEtaBins(), getMuEtaBins());
		fSamples[i].emu.fhistos.h_ntight     = new TH2D(name + "_EMu_fTight",     "NTight Muons for sig. supp. selection",      getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins());
		fSamples[i].emu.fhistos.h_nloose     = new TH2D(name + "_EMu_fLoose",     "NLoose Muons for sig. supp. selection",      getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins());
		fSamples[i].emu.fhistos.h_ntight_pt  = new TH1D(name + "_EMu_fTight_pt",  "NTight Muons for sig. supp. selection",      getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].emu.fhistos.h_nloose_pt  = new TH1D(name + "_EMu_fLoose_pt",  "NLoose Muons for sig. supp. selection",      getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].emu.fhistos.h_ntight_eta = new TH1D(name + "_EMu_fTight_eta", "NTight Muons for sig. supp. selection",      getNMuEtaBins(), getMuEtaBins());
		fSamples[i].emu.fhistos.h_nloose_eta = new TH1D(name + "_EMu_fLoose_eta", "NLoose Muons for sig. supp. selection",      getNMuEtaBins(), getMuEtaBins());
		fSamples[i].emu.phistos.h_ntight     = new TH2D(name + "_EMu_pTight",     "NTight Muons for Z decay selection",      getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins());
		fSamples[i].emu.phistos.h_nloose     = new TH2D(name + "_EMu_pLoose",     "NLoose Muons for Z decay selection",      getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins());
		fSamples[i].emu.phistos.h_ntight_pt  = new TH1D(name + "_EMu_pTight_pt",  "NTight Muons for Z decay selection",      getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].emu.phistos.h_nloose_pt  = new TH1D(name + "_EMu_pLoose_pt",  "NLoose Muons for Z decay selection",      getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].emu.phistos.h_ntight_eta = new TH1D(name + "_EMu_pTight_eta", "NTight Muons for Z decay selection",      getNMuEtaBins(), getMuEtaBins());
		fSamples[i].emu.phistos.h_nloose_eta = new TH1D(name + "_EMu_pLoose_eta", "NLoose Muons for Z decay selection",      getNMuEtaBins(), getMuEtaBins());		

		fSamples[i].emu.nthistos.h_nt2->Sumw2();       fSamples[i].emu.nthistos.h_nt2_pt->Sumw2();  fSamples[i].emu.nthistos.h_nt2_eta->Sumw2();
		fSamples[i].emu.nthistos.h_nt10->Sumw2();      fSamples[i].emu.nthistos.h_nt10_pt->Sumw2(); fSamples[i].emu.nthistos.h_nt10_eta->Sumw2();
		fSamples[i].emu.nthistos.h_nt01->Sumw2();      fSamples[i].emu.nthistos.h_nt01_pt->Sumw2(); fSamples[i].emu.nthistos.h_nt01_eta->Sumw2();
		fSamples[i].emu.nthistos.h_nt0->Sumw2();       fSamples[i].emu.nthistos.h_nt0_pt->Sumw2();  fSamples[i].emu.nthistos.h_nt0_eta->Sumw2();

		// fSamples[i].emu.fhistos.h_ntight    ->Sumw2(); fSamples[i].emu.fhistos.h_nloose     ->Sumw2();
		// fSamples[i].emu.fhistos.h_ntight_pt ->Sumw2(); fSamples[i].emu.fhistos.h_nloose_pt  ->Sumw2();
		// fSamples[i].emu.fhistos.h_ntight_eta->Sumw2(); fSamples[i].emu.fhistos.h_nloose_eta ->Sumw2();

		// fSamples[i].emu.phistos.h_ntight    ->Sumw2(); fSamples[i].emu.phistos.h_nloose     ->Sumw2();
		// fSamples[i].emu.phistos.h_ntight_pt ->Sumw2(); fSamples[i].emu.phistos.h_nloose_pt  ->Sumw2();
		// fSamples[i].emu.phistos.h_ntight_eta->Sumw2(); fSamples[i].emu.phistos.h_nloose_eta ->Sumw2();

		fSamples[i].ee.nthistos.h_nt2       = new TH2D(name + "_EE_NT2",        "NT2",        getNMuPt2Bins(), getMuPt2Bins(), getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].ee.nthistos.h_nt2_pt    = new TH1D(name + "_EE_NT2_pt",     "NT2",        getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].ee.nthistos.h_nt2_eta   = new TH1D(name + "_EE_NT2_eta",    "NT2",        getNMuEtaBins(), getMuEtaBins());
		fSamples[i].ee.nthistos.h_nt10      = new TH2D(name + "_EE_NT10",       "NT10",       getNMuPt2Bins(), getMuPt2Bins(), getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].ee.nthistos.h_nt10_pt   = new TH1D(name + "_EE_NT10_pt",    "NT10 vs pt", getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].ee.nthistos.h_nt10_eta  = new TH1D(name + "_EE_NT10_eta",   "NT10 vs eta",getNMuEtaBins(), getMuEtaBins());
		fSamples[i].ee.nthistos.h_nt01      = new TH2D(name + "_EE_NT01",       "NT01",       getNMuPt2Bins(), getMuPt2Bins(), getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].ee.nthistos.h_nt01_pt   = new TH1D(name + "_EE_NT01_pt",    "NT01 vs pt", getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].ee.nthistos.h_nt01_eta  = new TH1D(name + "_EE_NT01_eta",   "NT01 vs eta",getNMuEtaBins(), getMuEtaBins());
		fSamples[i].ee.nthistos.h_nt0       = new TH2D(name + "_EE_NT0",        "NT0",        getNMuPt2Bins(), getMuPt2Bins(), getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].ee.nthistos.h_nt0_pt    = new TH1D(name + "_EE_NT0_pt",     "NT0 vs pt",  getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].ee.nthistos.h_nt0_eta   = new TH1D(name + "_EE_NT0_eta",    "NT0 vs eta", getNMuEtaBins(), getMuEtaBins());
		fSamples[i].ee.fhistos.h_ntight     = new TH2D(name + "_EE_fTight",     "NTight Muons for sig. supp. selection",      getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins());
		fSamples[i].ee.fhistos.h_nloose     = new TH2D(name + "_EE_fLoose",     "NLoose Muons for sig. supp. selection",      getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins());
		fSamples[i].ee.fhistos.h_ntight_pt  = new TH1D(name + "_EE_fTight_pt",  "NTight Muons for sig. supp. selection",      getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].ee.fhistos.h_nloose_pt  = new TH1D(name + "_EE_fLoose_pt",  "NLoose Muons for sig. supp. selection",      getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].ee.fhistos.h_ntight_eta = new TH1D(name + "_EE_fTight_eta", "NTight Muons for sig. supp. selection",      getNMuEtaBins(), getMuEtaBins());
		fSamples[i].ee.fhistos.h_nloose_eta = new TH1D(name + "_EE_fLoose_eta", "NLoose Muons for sig. supp. selection",      getNMuEtaBins(), getMuEtaBins());
		fSamples[i].ee.phistos.h_ntight     = new TH2D(name + "_EE_pTight",     "NTight Muons for Z decay selection",      getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins());
		fSamples[i].ee.phistos.h_nloose     = new TH2D(name + "_EE_pLoose",     "NLoose Muons for Z decay selection",      getNMuPt2Bins(), getMuPt2Bins(), getNMuEtaBins(), getMuEtaBins());
		fSamples[i].ee.phistos.h_ntight_pt  = new TH1D(name + "_EE_pTight_pt",  "NTight Muons for Z decay selection",      getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].ee.phistos.h_nloose_pt  = new TH1D(name + "_EE_pLoose_pt",  "NLoose Muons for Z decay selection",      getNMuPt2Bins(), getMuPt2Bins());
		fSamples[i].ee.phistos.h_ntight_eta = new TH1D(name + "_EE_pTight_eta", "NTight Muons for Z decay selection",      getNMuEtaBins(), getMuEtaBins());
		fSamples[i].ee.phistos.h_nloose_eta = new TH1D(name + "_EE_pLoose_eta", "NLoose Muons for Z decay selection",      getNMuEtaBins(), getMuEtaBins());		

		fSamples[i].ee.nthistos.h_nt2->Sumw2();       fSamples[i].ee.nthistos.h_nt2_pt->Sumw2();  fSamples[i].ee.nthistos.h_nt2_eta->Sumw2();
		fSamples[i].ee.nthistos.h_nt10->Sumw2();      fSamples[i].ee.nthistos.h_nt10_pt->Sumw2(); fSamples[i].ee.nthistos.h_nt10_eta->Sumw2();
		fSamples[i].ee.nthistos.h_nt01->Sumw2();      fSamples[i].ee.nthistos.h_nt01_pt->Sumw2(); fSamples[i].ee.nthistos.h_nt01_eta->Sumw2();
		fSamples[i].ee.nthistos.h_nt0->Sumw2();       fSamples[i].ee.nthistos.h_nt0_pt->Sumw2();  fSamples[i].ee.nthistos.h_nt0_eta->Sumw2();

		fSamples[i].ee.fhistos.h_ntight    ->Sumw2(); fSamples[i].ee.fhistos.h_nloose     ->Sumw2();
		fSamples[i].ee.fhistos.h_ntight_pt ->Sumw2(); fSamples[i].ee.fhistos.h_nloose_pt  ->Sumw2();
		fSamples[i].ee.fhistos.h_ntight_eta->Sumw2(); fSamples[i].ee.fhistos.h_nloose_eta ->Sumw2();

		fSamples[i].ee.phistos.h_ntight    ->Sumw2(); fSamples[i].ee.phistos.h_nloose     ->Sumw2();
		fSamples[i].ee.phistos.h_ntight_pt ->Sumw2(); fSamples[i].ee.phistos.h_nloose_pt  ->Sumw2();
		fSamples[i].ee.phistos.h_ntight_eta->Sumw2(); fSamples[i].ee.phistos.h_nloose_eta ->Sumw2();
	}
}

void MuonPlotter::writeHistos(){
	TFile *pFile = new TFile(fOutputFileName, "RECREATE");
	pFile->cd();
	for(size_t i = 0; i < fSamples.size(); ++i){
		TDirectory* cdir = Util::FindOrCreate(fSamples[i].sname, pFile);
		cdir->cd();

		for(size_t ch = 0; ch < 3; ++ch){ // Loop over channels, mumu, emu, ee
			channel cha;
			if(ch == Muon)     cha = fSamples[i].mumu;
			if(ch == EMu)      cha = fSamples[i].emu;
			if(ch == Electron) cha = fSamples[i].ee;
			cha.nthistos.h_nt2      ->Write(cha.nthistos.h_nt2      ->GetName(), TObject::kWriteDelete);
			cha.nthistos.h_nt2_pt   ->Write(cha.nthistos.h_nt2_pt   ->GetName(), TObject::kWriteDelete);
			cha.nthistos.h_nt2_eta  ->Write(cha.nthistos.h_nt2_eta  ->GetName(), TObject::kWriteDelete);
			cha.nthistos.h_nt10     ->Write(cha.nthistos.h_nt10     ->GetName(), TObject::kWriteDelete);
			cha.nthistos.h_nt10_pt  ->Write(cha.nthistos.h_nt10_pt  ->GetName(), TObject::kWriteDelete);
			cha.nthistos.h_nt10_eta ->Write(cha.nthistos.h_nt10_eta ->GetName(), TObject::kWriteDelete);
			cha.nthistos.h_nt01     ->Write(cha.nthistos.h_nt01     ->GetName(), TObject::kWriteDelete);
			cha.nthistos.h_nt01_pt  ->Write(cha.nthistos.h_nt01_pt  ->GetName(), TObject::kWriteDelete);
			cha.nthistos.h_nt01_eta ->Write(cha.nthistos.h_nt01_eta ->GetName(), TObject::kWriteDelete);
			cha.nthistos.h_nt0      ->Write(cha.nthistos.h_nt0      ->GetName(), TObject::kWriteDelete);
			cha.nthistos.h_nt0_pt   ->Write(cha.nthistos.h_nt0_pt   ->GetName(), TObject::kWriteDelete);
			cha.nthistos.h_nt0_eta  ->Write(cha.nthistos.h_nt0_eta  ->GetName(), TObject::kWriteDelete);
			cha.fhistos.h_ntight    ->Write(cha.fhistos.h_ntight    ->GetName(), TObject::kWriteDelete);
			cha.fhistos.h_nloose    ->Write(cha.fhistos.h_nloose    ->GetName(), TObject::kWriteDelete);
			cha.fhistos.h_ntight_pt ->Write(cha.fhistos.h_ntight_pt ->GetName(), TObject::kWriteDelete);
			cha.fhistos.h_nloose_pt ->Write(cha.fhistos.h_nloose_pt ->GetName(), TObject::kWriteDelete);
			cha.fhistos.h_ntight_eta->Write(cha.fhistos.h_ntight_eta->GetName(), TObject::kWriteDelete);
			cha.fhistos.h_nloose_eta->Write(cha.fhistos.h_nloose_eta->GetName(), TObject::kWriteDelete);
			cha.phistos.h_ntight    ->Write(cha.phistos.h_ntight    ->GetName(), TObject::kWriteDelete);
			cha.phistos.h_nloose    ->Write(cha.phistos.h_nloose    ->GetName(), TObject::kWriteDelete);
			cha.phistos.h_ntight_pt ->Write(cha.phistos.h_ntight_pt ->GetName(), TObject::kWriteDelete);
			cha.phistos.h_nloose_pt ->Write(cha.phistos.h_nloose_pt ->GetName(), TObject::kWriteDelete);
			cha.phistos.h_ntight_eta->Write(cha.phistos.h_ntight_eta->GetName(), TObject::kWriteDelete);
			cha.phistos.h_nloose_eta->Write(cha.phistos.h_nloose_eta->GetName(), TObject::kWriteDelete);
		}
	}
	pFile->Write();
	pFile->Close();
}

int MuonPlotter::readHistos(TString filename){
	TFile *pFile = TFile::Open(filename, "READ");
	if(pFile == NULL){
		cout << "File " << filename << " does not exist!" << endl;
		return 1;
	}
	pFile->cd();
	for(size_t i = 0; i < fSamples.size(); ++i){
		for(size_t ch = 0; ch < 3; ++ch){ // Loop over channels, mumu, emu, ee
			channel *cha;
			if(ch == 0) cha = &fSamples[i].mumu;
			if(ch == 1) cha = &fSamples[i].emu;
			if(ch == 2) cha = &fSamples[i].ee;
		
			cha->nthistos.h_nt2       = (TH2D*)pFile->Get(fSamples[i].sname + "/" + cha->nthistos.h_nt2      ->GetName());
			cha->nthistos.h_nt2_pt    = (TH1D*)pFile->Get(fSamples[i].sname + "/" + cha->nthistos.h_nt2_pt   ->GetName());
			cha->nthistos.h_nt2_eta   = (TH1D*)pFile->Get(fSamples[i].sname + "/" + cha->nthistos.h_nt2_eta  ->GetName());
			cha->nthistos.h_nt10      = (TH2D*)pFile->Get(fSamples[i].sname + "/" + cha->nthistos.h_nt10     ->GetName());
			cha->nthistos.h_nt10_pt   = (TH1D*)pFile->Get(fSamples[i].sname + "/" + cha->nthistos.h_nt10_pt  ->GetName());
			cha->nthistos.h_nt10_eta  = (TH1D*)pFile->Get(fSamples[i].sname + "/" + cha->nthistos.h_nt10_eta ->GetName());
			cha->nthistos.h_nt01      = (TH2D*)pFile->Get(fSamples[i].sname + "/" + cha->nthistos.h_nt01     ->GetName());
			cha->nthistos.h_nt01_pt   = (TH1D*)pFile->Get(fSamples[i].sname + "/" + cha->nthistos.h_nt01_pt  ->GetName());
			cha->nthistos.h_nt01_eta  = (TH1D*)pFile->Get(fSamples[i].sname + "/" + cha->nthistos.h_nt01_eta ->GetName());
			cha->nthistos.h_nt0       = (TH2D*)pFile->Get(fSamples[i].sname + "/" + cha->nthistos.h_nt0      ->GetName());
			cha->nthistos.h_nt0_pt    = (TH1D*)pFile->Get(fSamples[i].sname + "/" + cha->nthistos.h_nt0_pt   ->GetName());
			cha->nthistos.h_nt0_eta   = (TH1D*)pFile->Get(fSamples[i].sname + "/" + cha->nthistos.h_nt0_eta  ->GetName());
			cha->fhistos.h_ntight     = (TH2D*)pFile->Get(fSamples[i].sname + "/" + cha->fhistos.h_ntight    ->GetName());
			cha->fhistos.h_nloose     = (TH2D*)pFile->Get(fSamples[i].sname + "/" + cha->fhistos.h_nloose    ->GetName());
			cha->fhistos.h_ntight_pt  = (TH1D*)pFile->Get(fSamples[i].sname + "/" + cha->fhistos.h_ntight_pt ->GetName());
			cha->fhistos.h_nloose_pt  = (TH1D*)pFile->Get(fSamples[i].sname + "/" + cha->fhistos.h_nloose_pt ->GetName());
			cha->fhistos.h_ntight_eta = (TH1D*)pFile->Get(fSamples[i].sname + "/" + cha->fhistos.h_ntight_eta->GetName());
			cha->fhistos.h_nloose_eta = (TH1D*)pFile->Get(fSamples[i].sname + "/" + cha->fhistos.h_nloose_eta->GetName());
			cha->phistos.h_ntight     = (TH2D*)pFile->Get(fSamples[i].sname + "/" + cha->phistos.h_ntight    ->GetName());
			cha->phistos.h_nloose     = (TH2D*)pFile->Get(fSamples[i].sname + "/" + cha->phistos.h_nloose    ->GetName());
			cha->phistos.h_ntight_pt  = (TH1D*)pFile->Get(fSamples[i].sname + "/" + cha->phistos.h_ntight_pt ->GetName());
			cha->phistos.h_nloose_pt  = (TH1D*)pFile->Get(fSamples[i].sname + "/" + cha->phistos.h_nloose_pt ->GetName());
			cha->phistos.h_ntight_eta = (TH1D*)pFile->Get(fSamples[i].sname + "/" + cha->phistos.h_ntight_eta->GetName());
			cha->phistos.h_nloose_eta = (TH1D*)pFile->Get(fSamples[i].sname + "/" + cha->phistos.h_nloose_eta->GetName());
			numberset numbers;
			numbers.nt2  = cha->nthistos.h_nt2->GetEntries();
			numbers.nt10 = cha->nthistos.h_nt10->GetEntries();
			numbers.nt01 = cha->nthistos.h_nt01->GetEntries();
			numbers.nt0  = cha->nthistos.h_nt0->GetEntries();
			numbers.nsst = cha->fhistos.h_ntight->GetEntries();
			numbers.nssl = cha->fhistos.h_nloose->GetEntries();
			numbers.nzt  = cha->phistos.h_ntight->GetEntries();
			numbers.nzl  = cha->phistos.h_nloose->GetEntries();
			cha->numbers = numbers;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
// Event Selections:
//____________________________________________________________________________
bool MuonPlotter::isGoodEvent(){
	// Some global cuts, select events with >1 jets
	if(!passesNJetCut(2)) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isGoodMuEvent(){
	// Ask for >0 loose muons, if 2 muons ask for second to be loose too
	if(!isGoodEvent()) return false;
	if(NMus < 1) return false;
	if(isLooseMuon(0) == false) return false;
	if(NMus > 1) if(isLooseMuon(1) == false) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isGoodElEvent(){
	// Ask for >0 loose electrons, if 2 electrons ask for second to be loose too
	if(!isGoodEvent()) return false;
	if(NEls < 1) return false;
	if(isLooseElectron(0) == false) return false;
	if(NEls > 1) if(isLooseElectron(1) == false) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isGoodElMuEvent(){
	// Ask for >0 loose electrons and muons
	if(!isGoodEvent()) return false;
	if(NEls < 1 || NMus < 1) return false;
	for(size_t i = 0; i < NEls; ++i) if(!isLooseElectron(i)) return false;
	for(size_t i = 0; i < NMus; ++i) if(!isLooseMuon(i)) return false;
	// if(isLooseElectron(0) == false) return false;
	// if(isLooseMuon(0) == false) return false;
	return true;
}

//____________________________________________________________________________
int MuonPlotter::isSSLLEvent(int &ind1, int &ind2){
	// Looks for a SS loose/loose pair of leptons
	// Return the channel: 0 = none found
	//                     1 / -1 = mu+mu+ / mu-mu- pair
	//                     2 / -2 = e+e+   / e-e-   pair
	//                     3 / -3 = mu+e+  / mu-e-  pair
	// The indices in the argument given are sorted by pt unless
	// it's a e/mu event when they are sorted such that the muon
	// is ind1
	// The event selected is the one with hardest pt1 + pt2
	const float MMU = 0.1057;
	const float MEL = 0.0005;
	vector<lepton> tmp_looseLeps_p;
	vector<lepton> tmp_looseLeps_m;

	// First store all loose leptons in two vectors according to their charges
	for(size_t i = 0; i < NMus; ++i){
		if(!isLooseMuon(i)) continue;
		if(MuCharge[i] == 1 ){
			lepton tmpLepton;
			TLorentzVector pmu;
			pmu.SetPtEtaPhiM(MuPt[i], MuEta[i], MuPhi[i], gMMU);
			tmpLepton.p      = pmu;
			tmpLepton.charge = 1;
			tmpLepton.type   = 0;
			tmpLepton.index  = i;
			tmp_looseLeps_p.push_back(tmpLepton);
		}
		if(MuCharge[i] == -1){
			lepton tmpLepton;
			TLorentzVector p;
			p.SetPtEtaPhiM(MuPt[i], MuEta[i], MuPhi[i], gMMU);
			tmpLepton.p      = p;
			tmpLepton.charge = -1;
			tmpLepton.type   = 0;
			tmpLepton.index  = i;
			tmp_looseLeps_m.push_back(tmpLepton);
		}
	}
	for(size_t i = 0; i < NEls; ++i){
		if(!isLooseElectron(i)) continue;
		if(ElCh[i] == 1 ){
			lepton tmpLepton;
			TLorentzVector p;
			p.SetPtEtaPhiM(ElPt[i], ElEta[i], ElPhi[i], gMEL);
			tmpLepton.p      = p;
			tmpLepton.charge = 1;
			tmpLepton.type   = 1;
			tmpLepton.index  = i;
			tmp_looseLeps_p.push_back(tmpLepton);
		}
		if(ElCh[i] == -1){
			lepton tmpLepton;
			TLorentzVector p;
			p.SetPtEtaPhiM(ElPt[i], ElEta[i], ElPhi[i], gMEL);
			tmpLepton.p      = p;
			tmpLepton.charge = -1;
			tmpLepton.type   = 1;
			tmpLepton.index  = i;
			tmp_looseLeps_m.push_back(tmpLepton);
		}
	}

	// Sort these vectors by pt
	vector<lepton> looseLeps_p;
	vector<lepton> looseLeps_m;
	looseLeps_p = sortLeptonsByPt(tmp_looseLeps_p);
	looseLeps_m = sortLeptonsByPt(tmp_looseLeps_m);

	// Proceed to select one ss pair
	double ptmax(0.);
	int select(0);
	if(looseLeps_p.size() > 1){
		ptmax = looseLeps_p[0].p.Pt() + looseLeps_p[1].p.Pt();
		select = 1;
	}
	if(looseLeps_m.size() > 1){
		double ptsum = looseLeps_m[0].p.Pt() + looseLeps_m[1].p.Pt();		
		if(ptsum > ptmax){
			ptmax = ptsum;
			select = -1;
		}
		// if(looseLeps_p.size() > 1) cout << " Event with TWO SS pairs: r" << setw(7) << Run << "/e" << setw(13) << Event << "/l" << setw(5) << LumiSec << " in dataset " << setw(9) << fSamples[fCurrentSample].sname << endl;		
	}
	if(select == 0) return 0; // this ensures we have at least one pair
	
	vector<lepton> selectedPair;
	if(select == 1){ // positive
		selectedPair.push_back(looseLeps_p[0]);
		selectedPair.push_back(looseLeps_p[1]);
	}
	if(select == -1){ // negative
		selectedPair.push_back(looseLeps_m[0]);
		selectedPair.push_back(looseLeps_m[1]);
	}
	// Switch if el/mu combination (want ind1 to be mu, ind2 to be el)
	if(selectedPair[0].type == 1 && selectedPair[1].type == 0){
		lepton el = selectedPair[0];
		lepton mu = selectedPair[1];
		selectedPair[0] = mu;
		selectedPair[1] = el;
	}

	int result = 0;
	if(selectedPair[0].type == 0 && selectedPair[1].type == 0) result = 1; // mu/mu
	if(selectedPair[0].type == 1 && selectedPair[1].type == 1) result = 2; // el/el
	if(selectedPair[0].type == 0 && selectedPair[1].type == 1) result = 3; // mu/el
	result *= select; // Add charge to result
	
	// Result
	ind1 = selectedPair[0].index;
	ind2 = selectedPair[1].index;
	return result;
}

//____________________________________________________________________________
int MuonPlotter::isOSLLEvent(int &ind1, int &ind2){
	// Looks for a OS loose/loose pair of leptons
	// Return the channel: 0 = none found
	//                     1 / -1 = mu+mu- / mu-mu+ pair
	//                     2 / -2 = e+e-   / e-e+   pair
	//                     3 / -3 = mu+e-  / mu-e+  pair
	// The indices in the argument given are sorted by pt unless
	// it's a e/mu event when they are sorted such that the muon
	// is ind1
	// Return value has sign of harder of the two, or the mu if
	// it's a mu/e pair
	// The event selected is the one with hardest pt1 + pt2
	const float MMU = 0.1057;
	const float MEL = 0.0005;
	vector<lepton> tmp_looseLeps;

	// First store all loose leptons in a vector
	for(size_t i = 0; i < NMus; ++i){
		if(!isLooseMuon(i)) continue;
			lepton tmpLepton;
			TLorentzVector pmu;
			pmu.SetPtEtaPhiM(MuPt[i], MuEta[i], MuPhi[i], gMMU);
			tmpLepton.p      = pmu;
			tmpLepton.charge = MuCharge[i];
			tmpLepton.type   = 0;
			tmpLepton.index  = i;
			tmp_looseLeps.push_back(tmpLepton);
	}
	for(size_t i = 0; i < NEls; ++i){
		if(!isLooseElectron(i)) continue;
			lepton tmpLepton;
			TLorentzVector p;
			p.SetPtEtaPhiM(ElPt[i], ElEta[i], ElPhi[i], gMEL);
			tmpLepton.p      = p;
			tmpLepton.charge = ElCh[i];
			tmpLepton.type   = 1;
			tmpLepton.index  = i;
			tmp_looseLeps.push_back(tmpLepton);
	}

	// Sort these vector by pt
	vector<lepton> looseLeps;
	looseLeps = sortLeptonsByPt(tmp_looseLeps);

	// Proceed to select one os pair
	if(looseLeps.size() < 2) return 0;
	
	vector<lepton> selectedPair;
	selectedPair.push_back(looseLeps[0]);
	for(size_t i = 1; i < looseLeps.size(); ++i){
		if(selectedPair[0].charge == looseLeps[i].charge) continue;
		selectedPair.push_back(looseLeps[i]);
		break;
	}
	if(selectedPair.size() < 2) return 0;

	// Switch if el/mu combination (want ind1 to be mu, ind2 to be el)
	if(selectedPair[0].type == 1 && selectedPair[1].type == 0){
		lepton el = selectedPair[0];
		lepton mu = selectedPair[1];
		selectedPair[0] = mu;
		selectedPair[1] = el;
	}

	int result = 0;
	if(selectedPair[0].type == 0 && selectedPair[1].type == 0) result = 1; // mu/mu
	if(selectedPair[0].type == 1 && selectedPair[1].type == 1) result = 2; // el/el
	if(selectedPair[0].type == 0 && selectedPair[1].type == 1) result = 3; // mu/el
	result *= selectedPair[0].charge; // Add charge to result
	
	// Result
	ind1 = selectedPair[0].index;
	ind2 = selectedPair[1].index;
	return result;
}

//____________________________________________________________________________
bool momentumComparator(MuonPlotter::lepton i, MuonPlotter::lepton j){ return (i.p.Pt()>j.p.Pt()); }
vector<MuonPlotter::lepton> MuonPlotter::sortLeptonsByPt(vector<lepton>& leptons){
	vector<lepton> theLep = leptons;
	sort (theLep.begin(), theLep.end(), momentumComparator);
	return theLep;
}

//____________________________________________________________________________
bool MuonPlotter::passesNJetCut(int cut){
	int njets(0);
	for(size_t i = 0; i < NJets; ++i) if(isGoodJet(i)) njets++;
	return njets>=cut;
}

//____________________________________________________________________________
bool MuonPlotter::passesNJetCut_LooseLep(int cut){
	// This is TIGHTER than passesNJetCut, so can be called on top of the other
	int njets(0);
	for(size_t i = 0; i < NJets; ++i) if(isGoodJet_LooseLep(i)) njets++;
	return njets>=cut;
}

//____________________________________________________________________________
bool MuonPlotter::passesHTCut(float cut){
	float ht(0.);
	for(size_t i = 0; i < NJets; ++i) if(isGoodJet(i)) ht += JetPt[i];
	return ht >= cut;
}

//____________________________________________________________________________
bool MuonPlotter::passesMETCut(float cut){
	if(pfMET < cut) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::passesZVeto(float dm){
// Checks if any combination of opposite sign, same flavor tight leptons (e or mu)
// has invariant mass closer than dm to the Z mass, returns true if none found
// Default for dm is 15 GeV

	if(NMus > 1){
		unsigned start = 0;
		if(fCurrentChannel == Muon) start = 1; // For mumu, ignore first mu
		// First muon
		for(size_t i = start; i < NMus-1; ++i){
			if(isTightMuon(i)){
				TLorentzVector pmu1, pmu2;
				pmu1.SetPtEtaPhiM(MuPt[i], MuEta[i], MuPhi[i], gMMU);

				// Second muon
				for(size_t j = i+1; j < NMus; ++j){ 
					if(isTightMuon(j) && (MuCharge[i] != MuCharge[j]) ){
						pmu2.SetPtEtaPhiM(MuPt[j], MuEta[j], MuPhi[j], gMMU);
						if(fabs((pmu1+pmu2).M() - gMZ) < dm) return false;
					}
				}
			}
		}
	}
	
	if(NEls > 1){
		unsigned start = 0;
		if(fCurrentChannel == Electron) start = 1; // For ee, ignore first e
		// First electron
		for(size_t i = start; i < NEls-1; ++i){
			if(isTightElectron(i)){
				TLorentzVector pel1, pel2;
				pel1.SetPtEtaPhiM(ElPt[i], ElEta[i], ElPhi[i], gMEL);

				// Second electron
				for(size_t j = i+1; j < NEls; ++j){
					if(isTightElectron(j) && (ElCh[i] != ElCh[j]) ){
						pel2.SetPtEtaPhiM(ElPt[j], ElEta[j], ElPhi[j], gMEL);
						if(fabs((pel1+pel2).M() - gMZ) < dm) return false;
					}
				}
			}
		}		
	}
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::passesMllEventVeto(float cut){
// Checks if any combination of opposite sign, same flavor tight leptons (e or mu)
// has invariant mass smaller than cut, returns true if none found

	if(NMus > 1){
		// First muon
		for(size_t i = 0; i < NMus-1; ++i){
			if(isTightMuon(i)){
				TLorentzVector pmu1, pmu2;
				pmu1.SetPtEtaPhiM(MuPt[i], MuEta[i], MuPhi[i], gMMU);

				// Second muon
				for(size_t j = i+1; j < NMus; ++j){ 
					if(isTightMuon(j)){/*
						TODO Check if they really use OS or also SS
					*/
						pmu2.SetPtEtaPhiM(MuPt[j], MuEta[j], MuPhi[j], gMMU);
						if((pmu1+pmu2).M() < cut) return false;
					}
				}
			}
		}		
	}

	if(NEls > 1){
		// First electron
		for(size_t i = 0; i < NEls-1; ++i){
			if(isTightElectron(i)){
				TLorentzVector pel1, pel2;
				pel1.SetPtEtaPhiM(ElPt[i], ElEta[i], ElPhi[i], gMEL);

				// Second electron
				for(size_t j = i+1; j < NEls; ++j){
					if(isTightElectron(j)){
						pel2.SetPtEtaPhiM(ElPt[j], ElEta[j], ElPhi[j], gMEL);
						if((pel1+pel2).M() < cut) return false;
					}
				}
			}
		}
	}
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isMuTriggeredEvent(){
	if(HLT_Mu9 == 0 &&
	   HLT_Mu11 == 0 &&
	   HLT_Mu13_v1 == 0 &&
	   HLT_Mu15 == 0 &&
	   HLT_Mu15_v1 == 0 &&
	   HLT_DoubleMu0 == 0 &&
	   HLT_DoubleMu3 == 0 &&
	   HLT_DoubleMu3_v2 == 0 &&
	   HLT_DoubleMu5_v2 == 0
	   ) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isElTriggeredEvent(){
	// Leptonic triggers from UCSD/UCSB/FNAL list
	if(Run == 1)                       return  HLT_Ele10_LW_L1R;
	if(Run >  1      && Run <  138000) return( HLT_Ele10_LW_L1R ||
	                                           HLT_Ele10_SW_L1R ||
	                                           HLT_Ele15_LW_L1R ||
	                                           HLT_DoubleEle5_SW_L1R );
	if(Run >= 138000 && Run <= 141900) return( HLT_Ele15_LW_L1R ||
	                                           HLT_Ele15_SW_L1R ||
	                                           HLT_Ele10_LW_EleId_L1R ||
	                                           HLT_DoubleEle5_SW_L1R);
	if(Run >  141900)                  return( HLT_Ele10_SW_EleId_L1R ||
	                                           HLT_Ele15_SW_CaloEleId_L1R ||
	                                           HLT_Ele15_SW_EleId_L1R ||
	                                           HLT_Ele17_SW_LooseEleId_L1R ||
	                                           HLT_Ele17_SW_CaloEleId_L1R ||
	                                           HLT_Ele17_SW_EleId_L1R ||
	                                           HLT_Ele17_SW_TightEleId_L1R ||
	                                           HLT_Ele17_SW_TighterEleId_L1R_v1 ||
	                                           HLT_Ele20_SW_L1R ||
	                                           HLT_Ele22_SW_TighterEleId_L1R_v2 ||
	                                           HLT_Ele22_SW_TighterEleId_L1R_v3 ||
	                                           HLT_Ele27_SW_TightCaloEleIdTrack_L1R_v1 ||
	                                           HLT_Ele32_SW_TightCaloEleIdTrack_L1R_v1 ||
	                                           HLT_Ele32_SW_TighterEleId_L1R_v2 ||
	                                           HLT_DoubleEle10_SW_L1R ||
	                                           HLT_DoubleEle15_SW_L1R_v1 ||
	                                           HLT_DoubleEle17_SW_L1R_v1 );
	return false;
}

//____________________________________________________________________________
bool MuonPlotter::isJetTriggeredEvent(){
	if(HLT_Jet15U == 0  && 
	   HLT_Jet30U == 0  && 
	   HLT_Jet50U == 0  &&
	   HLT_Jet70U == 0  &&
	   HLT_Jet100U == 0 &&
	   HLT_Jet100U_v2 == 0 &&
	   HLT_Jet100U_v3 == 0
	) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isHTTriggeredEvent(){
	// Selects run ranges and HT triggers
	if(Run == 1) if(HLT_HT100U == 1) return true;
	if(Run >= 140160 && Run <= 147116 ) if(HLT_HT100U    == 1) return true; // RunA
	if(Run >= 147196 && Run <= 148058 ) if(HLT_HT140U    == 1) return true; // Jet dataset
	if(Run >= 148822 && Run <= 149294 ) if(HLT_HT150U_v3 == 1) return true; // Multijet dataset
	if(!passesHTCut(300.)) return false;
	return false;
}

//____________________________________________________________________________
bool MuonPlotter::isGoodRun(int sample){
	// Select runs such that JetB and MultiJet datasets are mutually exclusive
	// if(gSample(sample) == JMB)      if(Run > 147195) return false;
	// if(gSample(sample) == MultiJet) if(Run < 147196) return false;
	if(gSample(sample) == JMB)      if(Run > 148058) return false;
	if(gSample(sample) == MultiJet) if(Run < 148822) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isSigSupMuEvent(){
	if(isGoodMuEvent() == false) return false;
	if(MuMT > 20.) return false;
	if(pfMET > 20.) return false;
	if(NMus > 1) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isSigSupMuEventTRG(){
	if(!isHTTriggeredEvent()) return false;
	// if(!isMuTriggeredEvent()) return false;
	if(!isSigSupMuEvent()) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isSigSupOSMuMuEvent(int& mu1, int& mu2){
	// This should include all the cuts for the final selection
	if(NMus < 1) return false;
	if(isLooseMuon(0) == false) return false;

	// if(!isGoodMuEvent()) return false; // >1 jets, >0 loose muons

	// if(MuMT  > 20.) return false;
	if(pfMET > 20.) return false;
	if(NMus < 2)    return false;    // >1 muons

	if(abs(isOSLLEvent(mu1, mu2)) != 1) return false;

	TLorentzVector pmu1, pmu2;
	pmu1.SetPtEtaPhiM(MuPt[mu1], MuEta[mu1], MuPhi[mu1], gMMU);
	pmu2.SetPtEtaPhiM(MuPt[mu2], MuEta[mu2], MuPhi[mu2], gMMU);
	if(fabs((pmu1+pmu2).M() - gMZ) < 25.) return false;

	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isSigSupSSMuMuEvent(int& mu1, int& mu2){
	// This should include all the cuts for the final selection
	if(NMus < 1) return false;
	if(isLooseMuon(0) == false) return false;

	// if(!isGoodMuEvent()) return false; // >1 jets, >0 loose muons

	// if(MuMT  > 20.) return false;
	if(pfMET > 20.) return false;
	if(NMus < 2)    return false;    // >1 muons

	if(abs(isSSLLEvent(mu1, mu2)) != 1) return false;

	TLorentzVector pmu1, pmu2;
	pmu1.SetPtEtaPhiM(MuPt[mu1], MuEta[mu1], MuPhi[mu1], gMMU);
	pmu2.SetPtEtaPhiM(MuPt[mu2], MuEta[mu2], MuPhi[mu2], gMMU);
	if(fabs((pmu1+pmu2).M() - gMZ) < 25.) return false;

	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isZMuMuEvent(){
	if(!passesNJetCut_LooseLep(2)) return false;
	if(isGoodMuEvent() == false) return false;
	if(NMus != 2) return false;
	if(isLooseMuon(0) == false || isLooseMuon(1) == false) return false;
	if(MuCharge[0] == MuCharge[1]) return false;

	// Z mass window cut
	TLorentzVector p1, p2;
	p1.SetPtEtaPhiM(MuPt[0], MuEta[0], MuPhi[0], 0.1057);
	p2.SetPtEtaPhiM(MuPt[1], MuEta[1], MuPhi[1], 0.1057);
	double m = (p1+p2).M();
	if(fabs(91.2 - m) > 15.) return false;

	if(pfMET > 20.) return false;

	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isZMuMuEventTRG(){
	if(isMuTriggeredEvent() == false) return false;
	if(isZMuMuEvent() == false) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isZElElEvent(int &elind){
	if(!passesNJetCut_LooseLep(2)) return false;
	if(isGoodElEvent() == false) return false;
	if(NEls != 2) return false;
	if(!isLooseElectron(0) || !isLooseElectron(1)) return false; // both loose
	if(!isTightElectron(0) && !isTightElectron(1)) return false; // at least one tight

	if(ElCh[0] == ElCh[1]) return false;

	// Z mass window cut
	TLorentzVector p1, p2;
	p1.SetPtEtaPhiM(ElPt[0], ElEta[0], ElPhi[0], 0.0005);
	p2.SetPtEtaPhiM(ElPt[1], ElEta[1], ElPhi[1], 0.0005);
	double m = (p1+p2).M();
	if(fabs(91.2 - m) > 15.) return false;

	if(pfMET > 20.) return false;

	// If only the harder one tight or both tight, return the softer one
	// If only the softer one tight, return the harder one
	elind = 1;
	if(isTightElectron(1) && !isTightElectron(0)) elind = 0;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isZElElEventTRG(int &elind){
	if(isElTriggeredEvent() == false) return false;
	if(isZElElEvent(elind) == false) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isSigSupElEvent(){
	if(!isGoodElEvent()) return false;
	if(fSelectionSwitch == 1) if(!passesHTCut(300.)) return false;
	if(ElMT[0] > 20.) return false;
	if(pfMET > 20.)   return false;
	if(NEls > 1)      return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isSigSupElEventTRG(){
	// if(!isElTriggeredEvent()) return false;
	if(fSelectionSwitch == 0) if(!isElTriggeredEvent()) return false;
	if(fSelectionSwitch == 1) if(!isHTTriggeredEvent()) return false;
	if(!isSigSupElEvent()) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isGenMatchedSUSYDiLepEvent(){
	int ind1(-1), ind2(-1);
	return isGenMatchedSUSYDiLepEvent(ind1, ind2);
}
bool MuonPlotter::isGenMatchedSUSYDiLepEvent(int &mu1, int &mu2){
	if(isGoodMuEvent() == false) return false;
	// if(isMuTriggeredEvent() == false) return false;
	if(!isSSTTMuEvent(mu1, mu2)) return false;
	if(isPromptSUSYMuon(mu1) && isPromptSUSYMuon(mu2)){
		// PT Cuts??
		if(isTightMuon(mu1) == 1 && isTightMuon(mu2) == 1) return true;
	}
	return false;
}

//____________________________________________________________________________
bool MuonPlotter::isSSLLMuEvent(int& mu1, int& mu2){
	// This should include all the cuts for the final selection
	if(fDoCounting) fCounters[fCurrentSample][Muon].fill(" ... passes triggers");

	if(!isGoodMuEvent()) return false; // >1 jets, >0 loose muons
	if(fDoCounting) fCounters[fCurrentSample][Muon].fill(" ... has at least 2 jets, 1 loose muon");
	if(NMus < 2) return false;         // >1 muons
	if(fDoCounting) fCounters[fCurrentSample][Muon].fill(" ... has 2 loose muons");

	if(!passesZVeto()) return false; // no Zs in event
	if(fDoCounting) fCounters[fCurrentSample][Muon].fill(" ... passes Z veto");
	
	if(fSelectionSwitch == 0) if(!passesMllEventVeto(12.)) return false; // no low mass OSSF pairs
	if(fSelectionSwitch == 1) if(!passesMllEventVeto(5.) ) return false;
	if(fDoCounting) fCounters[fCurrentSample][Muon].fill(" ... passes Minv veto");
	
	if(fSelectionSwitch == 0) if(!passesHTCut(60.) )  return false;    // ht cut
	if(fSelectionSwitch == 1) if(!passesHTCut(300.))  return false;
	if(fDoCounting) fCounters[fCurrentSample][Muon].fill(" ... passes HT cut");
	
	if(fSelectionSwitch == 0) if(!passesMETCut(30.) ) return false;    // met cut
	if(fSelectionSwitch == 1) if(!passesMETCut(30.))  return false;
	if(fDoCounting) fCounters[fCurrentSample][Muon].fill(" ... passes MET cut");

	if(fChargeSwitch == 0){
		if(abs(isSSLLEvent(mu1, mu2)) != 1) return false;
		if(fDoCounting) fCounters[fCurrentSample][Muon].fill(" ... has same-sign muons");
	}
	if(fChargeSwitch == 1){
		if(abs(isOSLLEvent(mu1, mu2)) != 1) return false;
		if(fDoCounting) fCounters[fCurrentSample][Muon].fill(" ... has opposite-sign muons");		
	}

	if(!isGoodSecMuon(mu2)) return false; // pt cuts
	if(fDoCounting) fCounters[fCurrentSample][Muon].fill(" ... second muon passes pt cut");

	if(!isGoodPrimMuon(mu1)) return false;
	if(fDoCounting) fCounters[fCurrentSample][Muon].fill(" ... first muon passes pt cut");		

	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isSSLLMuEventTRG(int& mu1, int& mu2){
	if(fSelectionSwitch == 0) if(!isMuTriggeredEvent()) return false;
	if(fSelectionSwitch == 1) if(!isHTTriggeredEvent()) return false;
	if(!isSSLLMuEvent(mu1, mu2)) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isSSTTMuEvent(int& mu1, int& mu2){
	if(!isSSLLMuEvent(mu1, mu2)) return false;
	if(!isTightMuon(mu1) || !isTightMuon(mu2)) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isSSTTMuEventTRG(int& mu1, int& mu2){
	if(!isSSLLMuEventTRG(mu1, mu2)) return false;
	if(!isTightMuon(mu1) || !isTightMuon(mu2)) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isSSLLElEvent(int& el1, int& el2){
	// This should include all the cuts for the final selection
	if(fDoCounting) fCounters[fCurrentSample][Electron].fill(" ... passes triggers");

	if(!isGoodElEvent()) return false; // >1 jets, >0 loose eles
	if(fDoCounting) fCounters[fCurrentSample][Electron].fill(" ... has at least 2 jets, 1 loose el");
	if(NEls < 2) return false;         // >1 eles
	if(fDoCounting) fCounters[fCurrentSample][Electron].fill(" ... has 2 loose electrons");

	if(!passesZVeto()) return false; // no Zs in event
	if(fDoCounting) fCounters[fCurrentSample][Electron].fill(" ... passes Z veto");

	if(fSelectionSwitch == 0) if(!passesMllEventVeto(12.)) return false; // no low mass OSSF pairs
	if(fSelectionSwitch == 1) if(!passesMllEventVeto(5.) ) return false;
	if(fDoCounting) fCounters[fCurrentSample][Electron].fill(" ... passes Minv veto");

	if(fSelectionSwitch == 0) if(!passesHTCut(60.) )  return false;    // ht cut
	if(fSelectionSwitch == 1) if(!passesHTCut(300.))  return false;
	if(fDoCounting) fCounters[fCurrentSample][Electron].fill(" ... passes HT cut");

	if(fSelectionSwitch == 0) if(!passesMETCut(30.) ) return false;    // met cut
	if(fSelectionSwitch == 1) if(!passesMETCut(30.))  return false;
	if(fDoCounting) fCounters[fCurrentSample][Electron].fill(" ... passes MET cut");

	if(fChargeSwitch == 0){
		if(abs(isSSLLEvent(el1, el2)) != 2) return false;
		if(fDoCounting) fCounters[fCurrentSample][Electron].fill(" ... has same-sign electrons");
	}
	if(fChargeSwitch == 1){
		if(abs(isOSLLEvent(el1, el2)) != 2) return false;
		if(fDoCounting) fCounters[fCurrentSample][Electron].fill(" ... has opposite-sign electrons");
	}

	if(!isGoodSecElectron(el2)) return false; // pt cuts
	if(fDoCounting) fCounters[fCurrentSample][Electron].fill(" ... second electron passes pt cut");
	
	if(!isGoodPrimElectron(el1)) return false;
	if(fDoCounting) fCounters[fCurrentSample][Electron].fill(" ... first electron passes pt cut");
	
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isSSLLElEventTRG(int& el1, int& el2){
	if(fSelectionSwitch == 0) if(!isElTriggeredEvent()) return false;
	if(fSelectionSwitch == 1) if(!isHTTriggeredEvent()) return false;
	if(!isSSLLElEvent(el1, el2)) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isSSTTElEvent(int& el1, int& el2){
	if(!isSSLLElEvent(el1, el2)) return false;
	if(!isTightElectron(el1) || !isTightElectron(el2)) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isSSTTElEventTRG(int& el1, int& el2){
	if(!isSSLLElEventTRG(el1, el2)) return false;
	if(!isTightElectron(el1) || !isTightElectron(el2)) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isSSLLElMuEvent(int& mu, int& el){
	// This should include all the cuts for the final selection
	if(fDoCounting) fCounters[fCurrentSample][EMu].fill(" ... passes triggers");

	if(!isGoodElMuEvent()) return false; // >1 jets, >0 loose eles or muons
	if(fDoCounting) fCounters[fCurrentSample][EMu].fill(" ... has at least 1 j, loose e/mu pair");

	if(!passesZVeto())       return false;
	if(fDoCounting) fCounters[fCurrentSample][EMu].fill(" ... passes Z veto");

	if(fSelectionSwitch == 0) if(!passesMllEventVeto(12.)) return false; // no low mass OSSF pairs
	if(fSelectionSwitch == 1) if(!passesMllEventVeto(5.) ) return false;
	if(fDoCounting) fCounters[fCurrentSample][EMu].fill(" ... passes Minv veto");

	if(fSelectionSwitch == 0) if(!passesHTCut(60.) )  return false;    // ht cut
	if(fSelectionSwitch == 1) if(!passesHTCut(300.))  return false;
	if(fDoCounting) fCounters[fCurrentSample][EMu].fill(" ... passes HT cut");

	if(fSelectionSwitch == 0) if(!passesMETCut(20.) ) return false;    // met cut
	if(fSelectionSwitch == 1) if(!passesMETCut(30.))  return false;
	if(fDoCounting) fCounters[fCurrentSample][EMu].fill(" ... passes MET cut");

	if(fChargeSwitch == 0){
		if(abs(isSSLLEvent(mu, el)) != 3) return false;
		if(fDoCounting) fCounters[fCurrentSample][EMu].fill(" ... has same-sign electron muon pair");
	}
	if(fChargeSwitch == 1){
		if(abs(isOSLLEvent(mu, el)) != 3) return false;
		if(fDoCounting) fCounters[fCurrentSample][EMu].fill(" ... has opp.-sign electron muon pair");
	}

	if(MuPt[mu] > ElPt[el]){
		if(!isGoodPrimMuon(mu))    return false;
		if(fDoCounting) fCounters[fCurrentSample][EMu].fill(" ... muon passes pt cut");
		if(!isGoodSecElectron(el)) return false;
		if(fDoCounting) fCounters[fCurrentSample][EMu].fill(" ... electron passes pt cut");
	}
	else if(MuPt[mu] < ElPt[el]){
		if(!isGoodPrimElectron(el)) return false;
		if(fDoCounting) fCounters[fCurrentSample][EMu].fill(" ... electron passes pt cut");
		if(!isGoodSecMuon(mu))      return false;
		if(fDoCounting) fCounters[fCurrentSample][EMu].fill(" ... muon passes pt cut");
	}
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isSSLLElMuEventTRG(int& mu, int& el){
	// For UCSD/SB/FNAL just use OR of all lepton triggers
	if(fSelectionSwitch == 0){
		// Take all muon triggered events from muon datasets
		if(fCurrentSample == MuA || fCurrentSample == MuB) if(!isMuTriggeredEvent()) return false;
		// Take only electron but NOT muon triggered events from electron datasets
		if(fCurrentSample == EGA || fCurrentSample == EGB) if(!isElTriggeredEvent() ||  isMuTriggeredEvent()) return false;
		if(!isElTriggeredEvent() && !isMuTriggeredEvent()) return false;
	}
	if(fSelectionSwitch == 1) if(!isHTTriggeredEvent()) return false;
	if(isSSLLElMuEvent(mu, el)) return true;
	return false;
}

//____________________________________________________________________________
bool MuonPlotter::isSSTTElMuEvent(int& mu, int& el){
	if(!isSSLLElMuEvent(mu, el)) return false;
	if(!isTightElectron(el) || !isTightMuon(mu)) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isSSTTElMuEventTRG(int& mu, int& el){
	/*
		TODO fixme: If this method is ever used, check trigger and dataset selections!
	*/
	if(!isSSLLElMuEventTRG(mu, el)) return false;
	if(!isTightElectron(el) || !isTightMuon(mu)) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// Object selections:
//////////////////////////////////////////////////////////////////////////////
// Muons
//____________________________________________________________________________
bool MuonPlotter::isGoodMuon(int muon){
	float ptcut(5.);
	if(fSelectionSwitch == 0) ptcut = 10.;
	if(fSelectionSwitch == 1) ptcut = 5.;
	if(MuPt[muon] < ptcut) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isLooseMuon(int muon){
	if(isGoodMuon(muon) == false)  return false;
	if(fSelectionSwitch == 0) if(MuIsoHybrid[muon] > 1.00) return false;
	if(fSelectionSwitch == 1) if(MuIso[muon]       > 1.00) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isTightMuon(int muon){
	if(isGoodMuon(muon) == false)  return false;
	if(isLooseMuon(muon) == false) return false;
	if(fSelectionSwitch == 0) if(MuIsoHybrid[muon] > 0.10) return false;
	if(fSelectionSwitch == 1) if(MuIso[muon]       > 0.15) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isGoodPrimMuon(int muon){
	if(isLooseMuon(muon) == false) return false;
	if(fSelectionSwitch == 0) if(MuPt[muon] < 20.) return false;
	if(fSelectionSwitch == 1) if(MuPt[muon] < 5. ) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isGoodSecMuon(int muon){
	if(isLooseMuon(muon) == false) return false;
	if(fSelectionSwitch == 0) if(MuPt[muon] < 10.) return false;
	if(fSelectionSwitch == 1) if(MuPt[muon] < 5. ) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isFakeTTbarMuon(int muon){
	if(isLooseMuon(muon) == false) return false;
	if(abs(MuGenMoID[muon]) == 24 || abs(MuGenMoID[muon]) == 15) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isPromptTTbarMuon(int muon){
	if(isLooseMuon(muon) == false) return false;
	if(abs(MuGenMoID[muon] == 24 && abs(MuGenGMoID[muon]) == 6))  return true;
	if(abs(MuGenMoID[muon] == 15 && abs(MuGenGMoID[muon]) == 24)) return true;
	return false;
}

//____________________________________________________________________________
bool MuonPlotter::isPromptSUSYMuon(int muon){
	if(isLooseMuon(muon) == false) return false;
	if( abs(MuGenMoType[muon]) == 9 || abs(MuGenMoType[muon]) == 4  || abs(MuGenMoType[muon]) == 2 ) return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////////
// Electrons
//____________________________________________________________________________
bool MuonPlotter::isGoodElectron(int ele){
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isLooseElectron(int ele){
	// All electrons are already loose in the high-pt selection (hybiso)
	if(fSelectionSwitch == 1){
		if( fabs(ElEta[ele]) < 1.479 ) if(ElRelIso[ele] > 1.00) return false;
		else                           if(ElRelIso[ele] > 0.60) return false;		
	}
	if(ElChIsCons[ele] != 1) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isTightElectron(int ele){
	if(!isLooseElectron(ele))       return false;
	if(ElIsGoodElId_WP80[ele] != 1) return false;

	if(fSelectionSwitch == 0) if(ElHybRelIso[ele] > 0.10) return false;
	if(fSelectionSwitch == 1) if(ElRelIso[ele]    > 0.15) return false;
	
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isGoodPrimElectron(int ele){
	if(isLooseElectron(ele) == false) return false;
	if(fSelectionSwitch == 0) if(ElPt[ele] < 20.) return false;
	if(fSelectionSwitch == 1) if(ElPt[ele] < 10. ) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isGoodSecElectron(int ele){
	if(isLooseElectron(ele) == false) return false;
	if(fSelectionSwitch == 0) if(ElPt[ele] < 10.) return false;
	if(fSelectionSwitch == 1) if(ElPt[ele] < 10. ) return false;
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isGoodJet(int jet){
	float minDR = 0.4;
	for(size_t imu = 0; imu < NMus; ++imu){
		if(!isTightMuon(imu)) continue;
		if(!isGoodSecMuon(imu)) continue;
		if(Util::GetDeltaR(MuEta[imu], JetEta[jet], MuPhi[imu], JetPhi[jet]) > minDR ) continue;
		return false;
	}
	for(size_t iel = 0; iel < NEls; ++iel){
		if(!isTightElectron(iel)) continue;
		if(!isGoodSecElectron(iel)) continue;
		if(Util::GetDeltaR(ElEta[iel], JetEta[jet], ElPhi[iel], JetPhi[jet]) > minDR ) continue;
		return false;
	}
	return true;
}

//____________________________________________________________________________
bool MuonPlotter::isGoodJet_LooseLep(int jet){
	float minDR = 0.4;
	for(size_t imu = 0; imu < NMus; ++imu){
		if(!isLooseMuon(imu)) continue;
		if(!isGoodSecMuon(imu)) continue;
		if(Util::GetDeltaR(MuEta[imu], JetEta[jet], MuPhi[imu], JetPhi[jet]) > minDR ) continue;
		return false;
	}
	for(size_t iel = 0; iel < NEls; ++iel){
		if(!isLooseElectron(iel)) continue;
		if(!isGoodSecElectron(iel)) continue;
		if(Util::GetDeltaR(ElEta[iel], JetEta[jet], ElPhi[iel], JetPhi[jet]) > minDR ) continue;
		return false;
	}
	return true;
}
