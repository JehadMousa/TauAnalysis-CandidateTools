#ifndef TauAnalysis_CandidateTools_XNTFit_h
#define TauAnalysis_CandidateTools_XNTFit_h
#include <sstream>
#include <iostream>

#include "TFile.h"
#include "TLorentzVector.h"
#include "TRandom2.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TF2.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TMatrixD.h"

/*
 Code by P. Harris
*/

class NTFit { 
 public:
  
  enum Options { 
    kFlat  = 0,
    kFlat2 = 1,
    kME    = 2,
    kHistLikelihood  = 3,
    kHistLikelihood2 = 4
  };
  enum CalcOptions { 
    kMean  = 0,
    kMax   = 1,
    kHist  = 2
  };
  
  //Setup 
  //NTFit(std::string iLikelihoodFile="tmp.root",int iType=1, Options iOption=kHistLikelihood,int iNEvents=1000.,bool iCorrectMet=true);
  NTFit(int iType,TLorentzVector lorentz1,TLorentzVector lorentz2,double met,double phi,TMatrixD covMet,std::string iLikelihoodFile="$CMSSW_BASE/src/TauAnalysis/CandidateTools/data/params.root",Options iOption=kHistLikelihood, CalcOptions iCalcOption=kMean,int iNEvents=500.,bool iCorrectMet=false);
  ~NTFit() { }
  void loadLikelihood(std::string iVar,TH1F **iZTau,TH2F **iZLep,TH1F **iHTau,TH2F **iHLep);
  void loadLikelihoodVariables();
  void computeLikelihoodFunction(double iPt,double iMet,double iFrac);
  //Calculation
  void buildFourVector(int iNEvents,double iPt1,double iEta1,double iPhi1,double iM1,double iPt2,double iEta2,double iPhi2, double iM2, double iMEX,double iMEY,bool iClear=true);
  double cosTheta(int iId,  double iPt1,double iPhi1,double iEta1,double iM1,double iPt2,double iPhi2,double iEta2,double iM2);
  //Comopute kinematics for a toy event
  TLorentzVector NeutrinoToy(TRandom2 &iRand,double &iX,double &iCosPhi,TLorentzVector &iTau,double iPt,double iPhi, double iEta,double iMLep,bool i3Body=true,double iXMin=0.2);
  //Main Function
  TLorentzVector NeutrinoToys(int iNProng,int iNJet,double iD01,double iD02);
  //Output
  TLorentzVector uncVec();
  TLorentzVector tau1Vec();
  TLorentzVector tau2Vec();
  //Books
  void fillPars(double iWeight, TLorentzVector &iSys,TLorentzVector &iTau1,TLorentzVector &iTau2);
  void divide();
  void maxPars (double iWeight, TLorentzVector &iSys,TLorentzVector &iTau1,TLorentzVector &iTau2);
  void histPars(double iWeight, TLorentzVector &iSys);
  void clear(TH1* iH);
  void clearPars();
  void calculate();
  void calculateHist();
  //Tools
  double mass(double iVal,double iMLep);
  double eta (double iTheta);
  double restMomentum(double iMNN,double iMLep);
  double beta(double iGamma);
  double gamma(TLorentzVector iVec,double iP,double iTheta,double iMLep);
  double sinTheta2(double iGamma,double iP,double iTheta,double iM);
  double p        (double iGamma,double iP,double iTheta,double iM);
  void   Nu       (TLorentzVector &iVis,TLorentzVector &iNu,double iCos,double iPNu,double iMNN,double iPhi);

  //Stuff
 private:
  // input parameter
  TLorentzVector fVec0, fVec1;
  double fMet, fMetPhi;
  TMatrixD fcovMet;

  Options     fOption;
  CalcOptions fCalcOption;
  int    fNEvents;
  bool   fCorrectMet;
  bool   f13Body;
  bool   f23Body;
  double fX1;
  double fX2;
  double fCosPhi1;
  double fCosPhi2;
  double fMass;
  double fPt;
  double fPhi;
  double fEta;
  double fM1;
  double fPt1;
  double fPhi1;
  double fEta1;
  double fM2;
  double fPt2;
  double fPhi2;
  double fEta2;
  double fMetSys;
  double fM1Sys;
  double fM2Sys;
  double fW1Sys;
  double fW2Sys;
  double fTotWeight;
  double fMLep;
  double fMTau;
  double fTauL;
  double fResp; 
  double fTempWeight;
  TF2    *fF1;
  TF1    *fF2;
  TF1    *fGaus;
  TFile  *fFile;
  TH1F   *fY;
  TH2F   *f2D;
 
  TH1F  **fPHTau;
  TH2F  **fPHLep;
  TH1F  **fPZTau;
  TH2F  **fPZLep;

  TH1F  **fMHTau;
  TH2F  **fMHLep;
  TH1F  **fMZTau;
  TH2F  **fMZLep;

  TH1F    *fHX1;
  TH1F    *fHX2;
  TH1F    *fHCosPhi1;
  TH1F    *fHCosPhi2;
  //4 Vector
  TH1F    *fHMass;
  TH1F    *fHPt;
  TH1F    *fHPhi;
  TH1F    *fHEta;
  //Rest
  TRandom2 *fRand;
  TLorentzVector fTau1;
  TLorentzVector fTau2;
  TLorentzVector fBoson;
  TLorentzVector fUnc;
  TLorentzVector fHBoson;
  TLorentzVector fZBoson;
  TMatrixD      *fCov;
};
#endif
