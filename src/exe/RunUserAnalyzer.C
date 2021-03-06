// C++ includes
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <vector>

// ROOT includes
#include <TROOT.h>
#include <TTree.h>
#include <TChain.h>

#include "FWCore/FWLite/interface/AutoLibraryLoader.h"

#include "UserAnalyzer.hh"

using namespace std;

//________________________________________________________________________________________
// Print out usage
void usage( int status = 0 ) {
	cout << "Usage: RunUserAnalyzer [-n max] [-d dir] [-v verbose] [-l] file1 [... filen]" << endl;
	cout << "  where:" << endl;
	cout << "     dir      is the output directory               " << endl;
	cout << "               default is TempOutput/               " << endl;
	cout << "     verbose  sets the verbose level                " << endl;
	cout << "               default is 0 (quiet mode)            " << endl;
	cout << "     filen    are the input files (by default: ROOT files)" << endl;
	cout << "               with option -l, these are read as text files" << endl;
	cout << "               with one ROOT file name per line      " << endl;
        cout << "     max      maximum number of events to process   " << endl;
	cout << endl;
	exit(status);
}

//________________________________________________________________________________________
int main(int argc, char* argv[]) {

  AutoLibraryLoader::enable();

  // Default options
  bool isList = false;
  TString outputdir = "TempOutput/";
  int verbose = 0;
  Long64_t maxEvents = -1;

  // Parse options
  char ch;
  while ((ch = getopt(argc, argv, "d:v:n:lh?")) != -1 ) {
    switch (ch) {
    case 'd': outputdir = TString(optarg); break;
    case 'v': verbose = atoi(optarg); break;
    case 'l': isList = true; break;
    case 'n': maxEvents = atoi(optarg); break;
    case '?':
    case 'h': usage(0); break;
    default:
      cerr << "*** Error: unknown option " << optarg << std::endl;
      usage(-1);
    }
  }

  argc -= optind;
  argv += optind;

  // Check arguments
  if( argc<1 ) {
    usage(-1);
  }

  std::vector<std::string> fileList;
  for(int i = 0; i < argc; i++){
    if( !isList ){
      fileList.push_back(argv[i]);
      printf(" Adding file: %s\n",argv[i]);
    } else {
      TString rootFile;
      ifstream is(argv[i]);
      while(rootFile.ReadLine(is) && (!rootFile.IsNull())){
        if(rootFile[0] == '#') continue;
        fileList.push_back(rootFile.Data());
        printf(" Adding file: %s\n", rootFile.Data());
      }
    }
  }

  cout << "--------------" << endl;
  cout << "OutputDir is:     " << outputdir << endl;
  cout << "Verbose level is: " << verbose << endl;
  cout << "Number of files: " << fileList.size() << endl;
  cout << "Number of events: " << maxEvents << endl;
  cout << "--------------" << endl;

  UserAnalyzer *tA = new UserAnalyzer(fileList);
  tA->SetOutputDir(outputdir);
  tA->SetVerbose(verbose);
  tA->SetMaxEvents(maxEvents);
  tA->BeginJob();
  tA->Loop();
  tA->EndJob();
  delete tA;
  return 0;
}

