#ifndef TauAnalysis_CandidateTools_svFitAuxFunctions_h
#define TauAnalysis_CandidateTools_svFitAuxFunctions_h

#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"

#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/JetReco/interface/GenJet.h"

namespace SVfit_namespace
{
  //-----------------------------------------------------------------------------
  // define masses, widths and lifetimes of particles
  // relevant for computing values of likelihood functions in SVfit algorithm
  //
  // NOTE: the values are taken from
  //        K. Nakamura et al. (Particle Data Group),
  //        J. Phys. G 37, 075021 (2010)
  //
  const double electronMass = 0.51100e-3; // GeV
  const double electronMass2 = electronMass*electronMass;
  const double muonMass = 0.10566; // GeV
  const double muonMass2 = muonMass*muonMass; 
  
  const double chargedPionMass = 0.13957; // GeV
  const double chargedPionMass2 = chargedPionMass*chargedPionMass;
  const double neutralPionMass = 0.13498; // GeV
  const double neutralPionMass2 = neutralPionMass*neutralPionMass;

  const double rhoMesonMass = 0.77549; // GeV
  const double rhoMesonMass2 = rhoMesonMass*rhoMesonMass;
  const double rhoMesonWidth = 0.1491; // GeV

  const double a1MesonMass = 1.230; // GeV
  const double a1MesonMass2 = a1MesonMass*a1MesonMass;
  const double a1MesonWidth = 0.600; // GeV (upper limit of range quoted for width of a1 meson resonance in PDG summary tables)

  const double tauLeptonMass = 1.77685; // GeV
  const double tauLeptonMass2 = tauLeptonMass*tauLeptonMass;
  const double tauLeptonMass3 = tauLeptonMass*tauLeptonMass*tauLeptonMass;
  const double cTauLifetime = 8.711e-3; // centimeters

  const double mZ = 91.188; // GeV
  const double gammaZ = 2.495; // GeV

  const double alphaZ = 1./128.9; // fine-structure constant @ Z0 mass

  const double sinTheta_weinberg2 = 0.231;
  const double sinTheta_weinberg = TMath::Sqrt(sinTheta_weinberg2);
  const double cosTheta_weinberg = TMath::Sqrt(1. - sinTheta_weinberg2);
  const double qTau  = -1.;
  const double vTau  = (-1. + 4.*sinTheta_weinberg2)/(4.*sinTheta_weinberg*cosTheta_weinberg);     // -0.044
  const double aTau  = -1./(4.*sinTheta_weinberg*cosTheta_weinberg);                               // -0.593
  const double qUp   = +2./3;
  const double vUp   = (1. - (8./3)*sinTheta_weinberg2)/(4.*sinTheta_weinberg*cosTheta_weinberg);  //  0.227
  const double aUp   = 1./(4.*sinTheta_weinberg*cosTheta_weinberg);                                //  0.593
  const double qDown = -1./3;
  const double vDown = (-1. + (4./3)*sinTheta_weinberg2)/(4.*sinTheta_weinberg*cosTheta_weinberg); // -0.410
  const double aDown = -1./(4.*sinTheta_weinberg*cosTheta_weinberg);                               // -0.593
  //-----------------------------------------------------------------------------

  inline double square(double x)
  {
    return x*x;
  }

  inline double cube(double x)
  {
    return x*x*x;
  }

  inline double fourth(double x)
  {
    return x*x*x*x;
  }

  inline double fifth(double x)
  {
    return x*x*x*x*x;
  }

  /// Boost a lorentz vector given in the laboratory frame into the rest frame of another lorentz vector.
  reco::Candidate::LorentzVector boostToCOM(
      const reco::Candidate::LorentzVector &comSystem,
      const reco::Candidate::LorentzVector &p4ToBoost
  );

  /// Boost a lorentz vector given in the rest frame of another lorentz vector to the laboratory frame.
  reco::Candidate::LorentzVector boostToLab(
      const reco::Candidate::LorentzVector &comSystem,
      const reco::Candidate::LorentzVector &p4ToBoost
  );

  inline double energyFromMomentum(double momentum, double mass) {
    return TMath::Sqrt(square(mass)+square(momentum));
  }

  /// Determine Gottfried-Jackson angle from visible energy fraction X
  double gjAngleFromX(double, double, double, double, double);

  /// Determine visible tau rest frame energy given visible mass and neutrino mass
  double pVisRestFrame(double, double, double);

  /// Convert the Gottfried-Jackson angle into a lab frame opening angle
  double gjAngleToLabFrame(double, double, double);

  /// Determine the tau momentum in the lab frame given the rest frame assumptions
  double motherMomentumLabFrame(double, double, double, double, double);

  /// Determine the tau direction given our parameterization
  reco::Candidate::Vector motherDirection(const reco::Candidate::Vector&, double, double);

  /// Compute the tau four vector given the tau direction and momentum
    reco::Candidate::LorentzVector motherP4(const reco::Candidate::Vector&, double, double);

  /// Compute decay angle in rest frame given momentum of tau lepton and visible decay product in lab frame
  double decayAngleFromLabMomenta(const reco::Candidate::LorentzVector&, const reco::Candidate::LorentzVector&);

  /// Compute logarithm of Gaussion probability density function
  /// in one/N dimensions
  double logGaussian(double, double);

  /// Compute the log likelihood from a residual and n-dim gaussian.  Will
  /// compute determinant and inverse of covariance in place.
  template<typename T1, typename T2>
  double logGaussianNd(const T1& residual, const T2& cov)
  {
    //std::cout << "<logGaussianNd>:" << std::endl;

    //std::cout << "residual:" << std::endl;
    //residual.Print(std::cout);
    //std::cout << std::endl;

    //std::cout << "cov:" << std::endl;
    //cov.Print(std::cout);
    //std::cout << std::endl;
    unsigned numDimensions = residual.Dim();
    if ( cov.Diagonal().Dim() != numDimensions ) {
      edm::LogError ("logGaussianNd")
	<< " Dimension of covariance matrix = "
        << cov.Diagonal().Dim() << "x" << cov.Diagonal().Dim()
	<< " does not match dimension = " << numDimensions
        << " of residual vector !!";
      return std::numeric_limits<float>::min();
    }

    double det = 0.;
    cov.Det2(det);
    if ( det == 0. ) {
      edm::LogError ("logGaussianNd")
	<< " Cannot invert " << numDimensions << "x" << numDimensions
        << " covariance matrix, det = " << det << " !!";
      return std::numeric_limits<float>::min();
    }

    T2 covInverse(cov);
    bool flag = covInverse.Invert();
    if ( flag == false ) {
      edm::LogError ("logGaussianNd")
	<< " Failed to invert covariance matrix, error flag = " << flag << " !!";
      return std::numeric_limits<float>::min();
    }

    //std::cout << "covInverse:" << std::endl;
    //covInverse.Print(std::cout);
    //std::cout << std::endl;

    //std::cout << "--> residual^T V^-1 redidual = " << ROOT::Math::Dot(residual, covInverse*residual) << std::endl;
    return logGaussianNdInvertedCovariance(residual, covInverse, det);
  }

  /// Compute multivariate gaussian likelihood with a pre-computed
  /// inverted covariance matrix and determinant.
  template<typename T1, typename T2>
  double logGaussianNdInvertedCovariance(
      const T1& residual, const T2& covInverse, double det){
    //std::cout << "<logGaussianNd>:" << std::endl;

    //std::cout << "residual:" << std::endl;
    //residual.Print(std::cout);
    //std::cout << std::endl;

    //std::cout << "cov:" << std::endl;
    //cov.Print(std::cout);
    //std::cout << std::endl;

    unsigned numDimensions = residual.Dim();
    if ( covInverse.Diagonal().Dim() != numDimensions ) {
      edm::LogError ("logGaussianNd")
	<< " Dimension of covariance matrix = " << covInverse.Diagonal().Dim()
        << "x" << covInverse.Diagonal().Dim()
	<< " does not match dimension = " << numDimensions
        << " of residual vector !!";
      return std::numeric_limits<float>::min();
    }

    //std::cout << "covInverse:" << std::endl;
    //covInverse.Print(std::cout);
    //std::cout << std::endl;

    //std::cout << "--> residual^T V^-1 redidual = " << ROOT::Math::Dot(residual, covInverse*residual) << std::endl;

    return -0.5*numDimensions*TMath::Log(2*TMath::Pi())
      - 0.5*TMath::Log(det)
      - 0.5*(ROOT::Math::Dot(residual, covInverse*residual));
  }
}

#endif
