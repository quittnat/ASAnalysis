#ifndef Utilities_hh
#define Utilities_hh

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "TROOT.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TString.h"
#include "TMath.h"
#include "TFile.h"


namespace Util {
  //__________________________________________________________________________
  inline TString MakeOutputDir(TString dir){
    if(!dir.EndsWith("/")) dir += "/";
    // Create directory if needed
    //  >> NOTE: This function needs to be called before the booking functions!
    char cmd[100];
    sprintf(cmd,"mkdir -p %s", dir.Data());
    system(cmd);
    return dir;
  }

  //__________________________________________________________________________
  inline void SetStyle(){
    TStyle *style = new TStyle("ETHStyle", "Standard Plain");
    style->SetCanvasColor(0);
    style->SetFrameFillColor(0);
    style->SetFrameBorderMode(0);
    style->SetFrameBorderSize(0);
    style->SetPalette(1,0);
    style->SetOptTitle(0);
    style->SetOptStat(111111);
    style->SetStatColor(0);
    style->SetStatStyle(3001);
    style->SetStatBorderSize(1);

    // Fonts
    Int_t font = 42;
    style->SetStatFont(font);
    style->SetTextFont(font);
    style->SetLabelFont(font, "xyz");
    style->SetTitleFont(font, "xyz");

    // Histograms
    style->SetHistFillColor(15);
    style->SetHistFillStyle(1001);
    style->SetHistLineWidth(2);
    gROOT->SetStyle("ETHStyle");
    gROOT->ForceStyle();
  }

 //__________________________________________________________________________
  inline void PrintPNG(TCanvas *cin, TString name, TString dir){
    // Prints a ROOT TCanvas Object to a .png file
    //  name is the bare output filename, e.g. "fit_4_8",
    //  dir is the output directory (inside the overall output dir.)
    // Create sub directories if needed
    if(!dir.EndsWith("/")) dir += "/";
    char cmd[100];
    sprintf(cmd,"mkdir -p %s", dir.Data());
    system(cmd);

    dir += name;
    dir += ".png";
    cin->Print(dir,"png");
  }

  //__________________________________________________________________________
  inline void PrintPDF(TCanvas *cin, TString name, TString dir){
    // Prints a ROOT TCanvas Object to a .png file
    //  name is the bare output filename, e.g. "fit_4_8",
    //  dir is the output directory (inside the overall output dir.)
    // Create sub directories if needed
    if(!dir.EndsWith("/")) dir += "/";
    char cmd[100];
    sprintf(cmd,"mkdir -p %s", dir.Data());
    system(cmd);

    dir += name;
    dir += ".pdf";
    cin->Print(dir,"pdf");
  }

  //__________________________________________________________________________
  inline void PrintEPS(TCanvas *cin, TString name, TString dir){
    // Prints a ROOT TCanvas Object to a .eps file
    //  name is the bare output filename, e.g. "fit_4_8",
    //  dir is the output directory (inside the overall output dir.)
    // Create sub directories if needed
    if(!dir.EndsWith("/")) dir += "/";
    char cmd[100];
    sprintf(cmd,"mkdir -p %s", dir.Data());
    system(cmd);

    dir += name;
    dir += ".eps";
    cin->Print(dir,"eps");
  }

  //__________________________________________________________________________
  inline TDirectory* FindOrCreate( TString& dir, TFile* file ) {
    // Look for a directory and create it if it does not exist

    // Start from the root of the file
    file->cd();
    // Remove deadly '/'s
    while ( dir.BeginsWith("/") ) dir = dir.Strip(TString::kLeading,'/');
    dir.ReplaceAll("//","/");

    // Loop over sub-directories to create (ROOT's mkdir has no -p option...)
    TString cdir(dir);
    while ( cdir.First('/')>0 || cdir.Length()>0 ) {
       // Create new subdirectory
       Size_t index = (cdir.First('/')>0 ? cdir.First('/') : cdir.Length());
       TString subdir = cdir(0,index);
       if ( !gDirectory->GetDirectory(subdir) ) {
          std::cout << "Creating directory " << subdir.Data() << std::endl;
          gDirectory->mkdir( subdir.Data() );
       }
       gDirectory->cd(subdir);
       cdir = cdir(index+1,cdir.Length());
    }
    return file->GetDirectory(dir);
    
  }
  
  //__________________________________________________________________________
  inline void SaveAll(TCanvas *cin, TString dir, TFile* file) {
    // Save all objects in a canvas to a file
    //   dir is a sub-directory in the file
    //   file is the file object (need to be already open)
    
    // A few checks
    if ( !file || !file->IsOpen() ) {
      std::cerr << "*** Util::SaveAll: file " << (file?file->GetName():"") << " does not exist" << std::endl;
      exit(-1);
    } 

    // Go to directory (create it if needed)
    TDirectory* cdir = Util::FindOrCreate(dir,file);
    if ( !cdir) {
      std::cerr << "Couldn't create directory " << dir << std::endl;
      exit(-1);
    }
    cdir->cd();
    
    // Loop over canvas object and save some of them
    TIter next(cin->GetListOfPrimitives());
    while (TObject *obj = next()) {
      if ( !strcmp(obj->ClassName(),"TFrame") ) continue;
      if ( !strcmp(obj->ClassName(),"TLine") ) continue;
      if ( !strcmp(obj->ClassName(),"TArrow") ) continue;
      if ( !strcmp(obj->ClassName(),"TLatex") ) continue;
      obj->Write(obj->GetName(),TObject::kOverwrite);
    }
  }

  //__________________________________________________________________________
  inline void Print(TCanvas *cin, TString name, TString dir, TFile* file=0) {
    // Print plot (PNG, EPS and to file)
    Util::PrintPNG(cin,name,dir);
    Util::PrintEPS(cin,name,dir);
    if ( file ) Util::SaveAll(cin,dir,file); 
  }

   //__________________________________________________________________________
  inline double DeltaPhi(double v1, double v2){
    // Computes the correctly normalized phi difference
    // v1, v2 = phi of object 1 and 2
    double diff = fabs(v2 - v1);
    double corr = TMath::TwoPi() - diff;
    if (diff < TMath::Pi()){ return diff;} else { return corr;}
  }

  //__________________________________________________________________________
  inline double GetDeltaR(double eta1, double eta2, double phi1, double phi2){
    // Computes the DeltaR of two objects from their eta and phi values
    return sqrt( (eta1-eta2)*(eta1-eta2)
                 + Util::DeltaPhi(phi1, phi2)*Util::DeltaPhi(phi1, phi2) );
  }


}

#endif
