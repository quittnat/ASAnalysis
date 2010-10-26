#ifndef SSDLAnalysis_hh
#define SSDLAnalysis_hh

#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <time.h>

#include <TH2D.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TCanvas.h>
#include <TTree.h>
#include <TStyle.h>
#include <TLatex.h>
#include <TLeaf.h>
#include <TBranch.h>

#include "base/TreeReader.hh"
#include "helper/Davismt2.h"
#include "helper/AnaClass.hh"
#include "helper/Utilities.hh"
#include "base/UserAnalysisBase.hh"

using namespace std;

class SSDLAnalysis : public UserAnalysisBase{
public:
	SSDLAnalysis(TreeReader *tr = 0);
	virtual ~SSDLAnalysis();
	
	Davismt2 *fMT2;
	
	void Begin(const char* filename = "SSDLTree.root");
	void Analyze();
	void End();

	virtual void BookTree				();
	virtual void BookTriggerVariables	(TTree* fAnalysisTree);
	virtual void BookMuonVariables		(TTree* fAnalysisTree);
	virtual void BookElectronVariables	(TTree* fAnalysisTree);
	virtual void BookAuxVariables		(TTree* fAnalysisTree);
	virtual void BookFPRVariables		(TTree* fAnalysisTree);
	virtual void ResetTree				();
	virtual void ResetTriggerVariables	();
	virtual void ResetMuonVariables		();
	virtual void ResetElectronVariables	();
	virtual void ResetAuxVariables		();
	virtual void ResetFPRVariables		();
	virtual void FillTree				();
	
	TLorentzVector	jetTotalP		(vector<int>& qualJetInd);
	TLorentzVector	elTotalP		(vector<int>& qualElInd);
	TLorentzVector	muTotalP		(vector<int>& qualMuInd);
	TLorentzVector	phoTotalP		(vector<int>& qualPhoInd);
	float			jetHT			(vector<int>& qualJetInd);	
	float			minDRtoJet		(float lepEta, float lepPhi);
	void			transverseMasses(TLorentzVector p1, TLorentzVector p2, TVector3 jtotPT, float &fTLepminv, float &fTLepmtinv, float &fTLepmCT, float &fTLepmCTorth, float &fTLepmCTparl, float &fTLepmt2_0, float &fTLepmt2_50, float &fTLepmt2_100, float &fTLepmT2orth_0, float &fTLepmT2orth_50, float &fTLepmT2orth_100 );	
	void			transverseAlphas(vector<int> qualElInd, vector<int> qualMuInd, vector<int> qualPhoInd, vector<int> qualJetInd, float &alphaT_h, float &alphaCT_h, float &alphaT, float &alphaCT, float &alphaT_new, float &alphaCT_new);

	void			DumpRunAndTiggerProperties		();
	void			DumpJetMETProperties			(vector<int>& selectedJetInd);
	void			DumpPhotonProperties			(vector<int>& selectedPhoInd,	TVector3 jtotPT);
	void			DumpMuonProperties				(vector<int>& selectedMuInd,	TVector3 jtotPT);
	void			DumpElectronProperties			(vector<int>& selectedElInd,	TVector3 jtotPT);
	void			DumpFPRatioProperties			();
	void			DumpElectronLooseAndTighPtAndEta(int elindex, float &elLoosePt, float &elTightPt, float &elLooseEta, float &elTightEta);
	void			DumpTwoElectronPtAndEta			(int el1index, int el2index, float &el1Pt, float &el2Pt, float &el1Eta, float &el2Eta);	

private:
	static const int maxNjets	= 30;
	static const int maxNmus	= 5;
	static const int maxNeles	= 5;
	static const int maxNphos	= 5;

	TTree*	fAnalysisTree;
	
	// run/sample properties
	int		fTRunNumber;
	int		fTEventNumber;
	int		fTLumiSection;
	float	fTextxslo;
	float	fTintxs;

	// trigger properties
	int		fT_HLTMu9;
	int		fT_HLTMu11;
	int		fT_HLTMu15;
	int		fT_HLTDoubleMu3;
	int		fT_HLTDoubleMu0;
	int		fTHLT_Jet15U;
	int		fTHLT_Jet30U;
	int		fTHLT_Jet50U;
	int		fTHLT_Jet70U;
	int		fTHLT_Jet100U;
	int		fTHLT_HT100U;
	int		fTHLT_HT120U;
	int		fTHLT_HT140U;
	int		fTHLT_HT150U;
	int		fTHLT_Ele10_LW_L1R;
	int		fTHLT_Ele10_SW_L1R;
	int		fTHLT_Ele15_LW_L1R;
	int		fTHLT_Ele15_SW_L1R;
	int		fTHLT_Ele15_SW_CaloEleId_L1R;
	int		fTHLT_Ele20_SW_L1R;
	int		fTHLT_DoubleEle5_SW_L1R;
	int		fTHLT_DoubleEle10_SW_L1R;
	int		fTHLT_DoubleEle15_SW_L1R_v1;
	int		fTHTL_GoodElEvent;
	int		fTHTL_GoodElFakesEvent;
	int		fTHTL_GoodHadronicEvent;
	int		fTHTL_GoodMuEvent;
	// jet-MET properties
	int		fTnqjets;
	float	fTJetpt[maxNjets];
	float	fTJeteta[maxNjets];
	float	fTJetphi[maxNjets];
	float	fTHT;
	float	fTSumEt;
	float	fTtcMET;
	float	fTpfMET;
	float	fTMuCorrMET;
	float	fTdPhiMJ1;
	float	fTdPhiMJ2;
	float	fTR12;
	float	fTR21;
	float	fTR12plusR21;
	// photon properties
	int		fTnqphos;
	float	fTPhopt[maxNphos];
	float	fTPhoeta[maxNphos];
	float	fTPhophi[maxNphos];
	float	fTPhoRelIso[maxNphos];
	float	fTPhoDRjet[maxNphos];
	float	fTPhoDRhardestjet[maxNphos];
	
	//muon properties
	int		fTnqmus;
	float	fTmupt[maxNmus];
	float	fTmueta[maxNmus];
	float	fTmuphi[maxNmus];
	float	fTmuiso[maxNmus];
	float	fTmuisohyb[maxNmus];
	int		fTmucharge[maxNmus];
	int		fTmutight[maxNmus]; // 0 for loose (but not tight), 1 for tight
	float	fTmuDRjet[maxNmus];
	float	fTmuDRhardestjet[maxNmus];
	float	fTmud0[maxNmus];
	float	fTmudz[maxNmus];
	float	fTmud0bs[maxNmus];
	float	fTmudzbs[maxNmus];
	float	fTmuptE[maxNmus];
	int		fTmuid[maxNmus];
	int		fTmumoid[maxNmus];
	int		fTmugmoid[maxNmus];
	int		fTmutype[maxNmus];
	int		fTmumotype[maxNmus];
	int		fTmugmotype[maxNmus];
	float	fTmuMT;
	float	fTmuMinv;
	
	// electron properties
	int		fTnqels;
	int		fTElcharge[maxNeles];
	int		fTElChargeIsCons[maxNeles];
	int		fTElChargeIsGenCons[maxNeles];
	float	fTElpt[maxNeles];
	float	fTEleta[maxNeles];
	float	fTElphi[maxNeles];
	float	fTEld0[maxNeles];
	float	fTElD0Err[maxNeles];
	float	fTElEoverP[maxNeles];
	float	fTElHoverE[maxNeles];
	float	fTElSigmaIetaIeta[maxNeles];
	float	fTElDeltaPhiSuperClusterAtVtx[maxNeles];
	float	fTElDeltaEtaSuperClusterAtVtx[maxNeles];
	float	fTElIDsimpleWP80relIso[maxNeles];
	float	fTElIDsimpleWPrelIso[maxNeles];
	float	fTElIDsimpleWP95relIso[maxNeles];	
	float	fTElRelIso[maxNeles];
	float	fTElDR04TkSumPt[maxNeles];
	float	fTElDR04EcalRecHitSumEt[maxNeles];
	float	fTElDR04HcalTowerSumEt[maxNeles];
	float	fTElS4OverS1[maxNeles];
	float	fTElConvPartnerTrkDist[maxNeles];
	float	fTElConvPartnerTrkDCot[maxNeles];
	float	fTElChargeMisIDProb[maxNeles];
	float	fTElMT[maxNeles];
	float	fTElDRjet[maxNeles];
	float	fTElDRhardestjet[maxNeles];
	int		fTElGenID[maxNeles];
	int		fTElGenStatus[maxNeles];
	int		fTElGenMID[maxNeles];
	int		fTElGenMStatus[maxNeles];
	int		fTElGenGMID[maxNeles];
	int		fTElGenGMStatus[maxNeles];
	int		fTElTight[maxNeles];
	float	fTElHybRelIso[maxNeles];
	float	fTElminv;
	float	fTElmtinv;

	// other properties
	float	fTElmt2_0;
	float	fTElmt2_50;
	float	fTElmt2_100;
	float	fTElmCT;
	float	fTElmCTorth;
	float	fTElmCTparl;
	float	fTElmT2orth_0;
	float	fTElmT2orth_50;
	float	fTElmT2orth_100;	
	float	fTMumt2_0;
	float	fTMumt2_50;
	float	fTMumt2_100;
	float	fTMumCT;
	float	fTMumCTorth;
	float	fTMumCTparl;
	float	fTMumT2orth_0;
	float	fTMumT2orth_50;
	float	fTMumT2orth_100;
	float	fTalphaT_h;
	float	fTalphaCT_h;	
	float	fTalphaT;
	float	fTalphaCT;	
	float	fTalphaT_new;
	float	fTalphaCT_new;

	// fake ratio propeties
	int		fTisSE_QCDLike;
	int		fTSE_QCDLike_FakeElGenID;
	float	fTSE_QCDLike_ElLoosePt;
	float	fTSE_QCDLike_ElTightPt;
	float	fTSE_QCDLike_ElLooseEta;
	float	fTSE_QCDLike_ElTightEta;
	
	int		fTisSE_AntiQCDLike;
	int		fTSE_AntiQCDLike_FakeElGenID;
	float	fTSE_AntiQCDLike_ElLoosePt;
	float	fTSE_AntiQCDLike_ElTightPt;
	float	fTSE_AntiQCDLike_ElLooseEta;
	float	fTSE_AntiQCDLike_ElTightEta;
	
	int		fTisDE_ZJetsLike;
	float	fTDE_ZJetsLike_ElLoosePt;
	float	fTDE_ZJetsLike_ElTightPt;
	float	fTDE_ZJetsLike_ElLooseEta;
	float	fTDE_ZJetsLike_ElTightEta;
	float	fTDE_ZJetsLike_PromptElGenLoosePt;
	float	fTDE_ZJetsLike_PromptElGenTightPt;
	float	fTDE_ZJetsLike_PromptElGenLooseEta;
	float	fTDE_ZJetsLike_PromptElGenTightEta;

	int		fTisDE_WJetsLike;
	int		fTDE_WJetsLike_FakeElGenID;
	float	fTDE_WJetsLike_ElLoosePt;
	float	fTDE_WJetsLike_ElTightPt;
	float	fTDE_WJetsLike_ElLooseEta;
	float	fTDE_WJetsLike_ElTightEta;
	float	fTDE_WJetsLike_ElTightMT;
	float	fTDE_WJetsLike_FakeElGenLoosePt;
	float	fTDE_WJetsLike_FakeElGenTightPt;
	float	fTDE_WJetsLike_FakeElGenLooseEta;
	float	fTDE_WJetsLike_FakeElGenTightEta;
	float	fTDE_WJetsLike_PromptElGenLoosePt;
	float	fTDE_WJetsLike_PromptElGenTightPt;
	float	fTDE_WJetsLike_PromptElGenLooseEta;
	float	fTDE_WJetsLike_PromptElGenTightEta;
	float	fTDE_WJetsLike_PromptElGenMT;
	
	int		fTisDE_AntiWJetsLike;
	int		fTDE_AntiWJetsLike_FakeElGenID;
	float	fTDE_AntiWJetsLike_ElLoosePt;
	float	fTDE_AntiWJetsLike_ElTightPt;
	float	fTDE_AntiWJetsLike_ElLooseEta;
	float	fTDE_AntiWJetsLike_ElTightEta;
	float	fTDE_AntiWJetsLike_ElTightMT;
	float	fTDE_AntiWJetsLike_FakeElGenLoosePt;
	float	fTDE_AntiWJetsLike_FakeElGenTightPt;
	float	fTDE_AntiWJetsLike_FakeElGenLooseEta;
	float	fTDE_AntiWJetsLike_FakeElGenTightEta;
	float	fTDE_AntiWJetsLike_PromptElGenLoosePt;
	float	fTDE_AntiWJetsLike_PromptElGenTightPt;
	float	fTDE_AntiWJetsLike_PromptElGenLooseEta;
	float	fTDE_AntiWJetsLike_PromptElGenTightEta;
	float	fTDE_AntiWJetsLike_PromptElGenMT;
	
	int		fTisDE_SignalLike;
	float	fTDE_Ntt_El1Pt;
	float	fTDE_Ntt_El2Pt;
	float	fTDE_Ntl_El1Pt;
	float	fTDE_Ntl_El2Pt;
	float	fTDE_Nlt_El1Pt;
	float	fTDE_Nlt_El2Pt;
	float	fTDE_Nll_El1Pt;
	float	fTDE_Nll_El2Pt;
	float	fTDE_Ntt_El1Eta;
	float	fTDE_Ntt_El2Eta;
	float	fTDE_Ntl_El1Eta;
	float	fTDE_Ntl_El2Eta;
	float	fTDE_Nlt_El1Eta;
	float	fTDE_Nlt_El2Eta;
	float	fTDE_Nll_El1Eta;
	float	fTDE_Nll_El2Eta;
};
#endif