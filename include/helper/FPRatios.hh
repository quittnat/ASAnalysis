#ifndef FPRatios_hh
#define FPRatios_hh


#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "TH2D.h"

using namespace std;

class FPRatios {
// Class to handle the estimation of prompt and fake contributions
//
// Authors: Filip Moortgat, Luc pape, Daniel Treille
// 
// Applies the formalism of CMS AN 2010/xxx
//  for the general case of any number of leptons, electrons and/or muons

public:
// constructor
	FPRatios();

// destructor
	virtual ~FPRatios();

// set the level of verbosity (default = 0, no printout except initialization)
	void SetVerbose(int verb) {verbose = verb;};

// ***********************************************************************
//
// Definitions:
// topologies = number of observed electrons and muons, the N_ti of the note
// FPconfigs = number of events with given number of fake and prompt electrons
// and muons, the N_nfe,npe,nfm,npm in the Note
//
// Simplest approach is (recommended):
// - the user runs his/her UserAnalysis over the selected event sample and computes:
//   + the numbers of events in all topologies N_ti
//   + the average fake and prompt ratios and their uncertainties
// - at the end of the job, the FPRatio class is instantiated to compute
//   + the numbers of events in all FPconfigs
//   + their statistical and systematic uncertainties
//   + the variances of all two-by-two combinations of the FPconfigs
//
// More complicated:
// - the user can call FPWeight event by event to compute the weights (and errors)
// - see the code of NevtTopol for how to handle this
// - in this case the functions giving the fake and prompt ratios and errors have to be provided
//
// ***********************************************************************

// initialization defining the passing and failing leptons
// nemaxi = number of electrons
// nmmaxi = number of muons
// npassi = vector with number of events in each topology (Nti in the note)
//   in the order Nt0,0, Nt0,1, Nt0,2 ... , Nt1,0, ...
	int NevtTopol(int nemaxi, int nmmaxi, vector<double> npassi);

// initialization providing the fake and prompt ratios and errors for e and mu
	double fFRatioe1, fFRatiom1, fFRatioErre1, fFRatioErrm1;
	double fPRatioe1, fPRatiom1, fPRatioErre1, fPRatioErrm1;
	double fFRatioe2, fFRatiom2, fFRatioErre2, fFRatioErrm2;
	double fPRatioe2, fPRatiom2, fPRatioErre2, fPRatioErrm2;

	void SetPratios(double, double, double, double);
	void SetFratios(double, double, double, double);
	void SetPratios2(double, double, double, double);
	void SetFratios2(double, double, double, double);

	inline void SetMuFratio(double f, double e){ fFRatiom1 = f; fFRatioErrm1 = e;};
	inline void SetMuPratio(double f, double e){ fPRatiom1 = f; fPRatioErrm1 = e;};
	inline void SetMuFratio(double f){ fFRatiom1 = f; fFRatioErrm1 = 0.1*f;}; // 10% error if none provided
	inline void SetMuPratio(double f){ fPRatiom1 = f; fPRatioErrm1 = 0.1*f;};

	inline void SetElFratio(double f, double e){ fFRatioe1 = f; fFRatioErre1 = e;};
	inline void SetElPratio(double f, double e){ fPRatioe1 = f; fPRatioErre1 = e;};
	inline void SetElFratio(double f){ fFRatioe1 = f; fFRatioErre1 = 0.1*f;}; // 10% error if none provided
	inline void SetElPratio(double f){ fPRatioe1 = f; fPRatioErre1 = 0.1*f;};
	
	inline void SetMu2Fratio(double f, double e){ fFRatiom2 = f; fFRatioErrm2 = e;};
	inline void SetMu2Pratio(double f, double e){ fPRatiom2 = f; fPRatioErrm2 = e;};
	inline void SetMu2Fratio(double f){ fFRatiom2 = f; fFRatioErrm2 = 0.1*f;}; // 10% error if none provided
	inline void SetMu2Pratio(double f){ fPRatiom2 = f; fPRatioErrm2 = 0.1*f;};

	inline void SetEl2Fratio(double f, double e){ fFRatioe2 = f; fFRatioErre2 = e;};
	inline void SetEl2Pratio(double f, double e){ fPRatioe2 = f; fPRatioErre2 = e;};
	inline void SetEl2Fratio(double f){ fFRatioe2 = f; fFRatioErre2 = 0.1*f;}; // 10% error if none provided
	inline void SetEl2Pratio(double f){ fPRatioe2 = f; fPRatioErre2 = 0.1*f;};
	
// Histograms containing the fake ratios, and methods to set them:
	TH2D *fElefRatio, *fMufRatio, *fElepRatio, *fMupRatio;
	inline void SetFratios(TH2D* hel, TH2D* hmu){ fElefRatio = hel; fMufRatio = hmu; };
	inline void SetElFratios(TH2D* h){ fElefRatio = h;};
	inline void SetMuFratios(TH2D* h){ fMufRatio = h;};
	inline void SetPratios(TH2D* hel, TH2D* hmu){ fElepRatio = hel; fMupRatio = hmu; };
	inline void SetElPratios(TH2D* h){ fElepRatio = h;};
	inline void SetMuPratios(TH2D* h){ fMupRatio = h;};

// Get the numbers of events:
// returns a vector with the number of events for all FPconfigs, the N_nfe,npe,nfm,npm in the Note
// order is N_nfe,nfm = N_0,0, N_0,1, N_0,2, ... , N_1,0, ...
	vector<double> NevtPass();
	vector<double> NevtPass(vector<double>, vector<double>);
	vector<double> NevtPass(vector<double>, vector<double>, vector<double>, vector<double>);
// and the corresponding statistical and systematic uncertainties
	vector<double> NevtPassErrStat();
	vector<double> NevtPassErrSyst();

// to get all stat and syst FPcombi variance combinations
// order is V(N_0,0, N_0,1),  V(N_0,0, N_0,2), ...,  V(N_0,1 N_0,2), ...,  V(N_1,0, N_0,1), ...
	vector<double> VarStat();
	vector<double> VarSyst();

// This is all for the simple case

// if wanted, it is possible to get the individual weights
//   for a given FPconfig and topology
// this can be called event by event,
// for the usage, look into e.g. function NevtPass
	double FPWeight (int npe, int nfe, int npm, int nfm, vector<double> vpt, vector<double> veta, vector<int> ipass);
	double FPWeight (int npe, int nfe, int npm, int nfm, vector<double> f, vector<double> ferr, vector<double> p, vector<double> perr, vector<int> ipass);
	double FPWeightError (int i);
	vector<double> GetWgtError(int i);
	vector<double> GetVarWeights(int ntopol,int itop1, int itop2);

// the user should also replace the dummy versions of the user functions (from FPRatio.cc)
// user defined functions returning fake and prompt ratios and their errors
//  as a function of the pT and eta of the lepton
//  emu = 1 for electron, = 2 for muon
	double FakeRatio (int emu, double pt, double eta);
	double PromptRatio (int emu, double pt, double eta);
	double FakeRatioError (int emu, double pt, double eta);
	double PromptRatioError (int emu, double pt, double eta);

private:

	void Varnf1nf2();


	int verbose;

	int initTopol;
	int initVar;
	int ne, nm, ntopol;
	vector<double> npass;
	vector<int> nfetop, npetop, nfmtop, npmtop;
	vector<double> ntotfp;

	vector<double> wgtitot;
	double wgtie, wgtim;
	double dwgtdpe, dwgtdfe;
	double dwgtdpm, dwgtdfm;
	vector<double> dwgtdptope, dwgtdftope;
	vector<double> dwgtdptopm, dwgtdftopm;

	vector<double> dNtotstat, dNtotsyst;

// the following are vectors of vectors containing w_ij C_p,ij dp and similar for df
// the 1st vector has in sequence:
//   the combinations of np, nf
//     for each the various topologies
// the 2nd vector contains the various permutations of fakes and prompts 
//     for a given entry in the 1st vector
// all are stored in the order in which they are produced
// Example:
//  0 npe = 2, nfe = 0, npass = 0
//  1 npe = 2, nfe = 0, npass = 1
//  2 npe = 2, nfe = 0, npass = 2
//  3 npe = 1, nfe = 1, npass = 0 (2 entries in 2nd vector)
//  4 npe = 1, nfe = 1, npass = 1 (2 entries in 2nd vector)
//  5 npe = 1, nfe = 1, npass = 2 (2 entries in 2nd vector)
//  6 npe = 1, nfe = 1, npass = 0
//  7 npe = 1, nfe = 1, npass = 1
//  8 npe = 1, nfe = 1, npass = 2
	vector<vector<double> > dwgtdptote, dwgtdftote, dwgtdptotm, dwgtdftotm;

	vector<double> wgtitote, wgtitotm;

	vector<double> varstat, varsyst;


};
#endif
