#include "DQMOffline/Trigger/plugins/BPHMonitor.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DQM/TrackingMonitor/interface/GetLumi.h"

#include "CommonTools/TriggerUtils/interface/GenericTriggerEventFlag.h"


// -----------------------------
//  constructors and destructor
// -----------------------------

BPHMonitor::BPHMonitor( const edm::ParameterSet& iConfig ) : 
  folderName_             ( iConfig.getParameter<std::string>("FolderName") )
  , muoToken_             ( mayConsume<reco::MuonCollection>       (iConfig.getParameter<edm::InputTag>("muons")     ) )  
  , bsToken_              ( mayConsume<reco::BeamSpot>             (iConfig.getParameter<edm::InputTag>("beamSpot")))
  , PVsToken_             ( mayConsume<reco::VertexCollection>     (iConfig.getParameter<edm::InputTag>("offlinePVs")))
  , phi_binning_          ( getHistoPSet   (iConfig.getParameter<edm::ParameterSet>("histoPSet").getParameter<edm::ParameterSet>   ("phiPSet")    ) )
  , pt_binning_          ( getHistoPSet   (iConfig.getParameter<edm::ParameterSet>("histoPSet").getParameter<edm::ParameterSet>   ("ptPSet")    ) )
  , eta_binning_          ( getHistoPSet   (iConfig.getParameter<edm::ParameterSet>("histoPSet").getParameter<edm::ParameterSet>   ("etaPSet")    ) )
  , d0_binning_           ( getHistoLSPSet (iConfig.getParameter<edm::ParameterSet>("histoPSet").getParameter<edm::ParameterSet>   ("d0PSet")     ) )
  , z0_binning_           ( getHistoLSPSet (iConfig.getParameter<edm::ParameterSet>("histoPSet").getParameter<edm::ParameterSet>   ("z0PSet")     ) )
  , num_genTriggerEventFlag_(new GenericTriggerEventFlag(iConfig.getParameter<edm::ParameterSet>("numGenericTriggerEventPSet"),consumesCollector(), *this))
  , den_genTriggerEventFlag_(new GenericTriggerEventFlag(iConfig.getParameter<edm::ParameterSet>("denGenericTriggerEventPSet"),consumesCollector(), *this))
  , muoSelection_ ( iConfig.getParameter<std::string>("muoSelection") )
  , nmuons_     ( iConfig.getParameter<int>("nmuons" )     )
{

  muPhi_.numerator   = nullptr;
  muPhi_.denominator = nullptr;
  muEta_.numerator   = nullptr;
  muEta_.denominator = nullptr;
  muPt_.numerator   = nullptr;
  muPt_.denominator = nullptr;
  mud0_.numerator   = nullptr;
  mud0_.denominator   = nullptr;
  muz0_.numerator   = nullptr;
  muz0_.denominator = nullptr;

  JpsiPhi_.numerator   = nullptr;
  JpsiPhi_.denominator = nullptr;
  JpsiEta_.numerator   = nullptr;
  JpsiEta_.denominator = nullptr;
  JpsiPt_.numerator   = nullptr;
  JpsiPt_.denominator = nullptr;
  JpsiM_.numerator   = nullptr;
  JpsiM_.denominator = nullptr;
}

BPHMonitor::~BPHMonitor()
{
  if (num_genTriggerEventFlag_) delete num_genTriggerEventFlag_;
  if (den_genTriggerEventFlag_) delete den_genTriggerEventFlag_;
}

MEbinning BPHMonitor::getHistoPSet(edm::ParameterSet pset)
{
  return MEbinning{
    pset.getParameter<int32_t>("nbins"),
      pset.getParameter<double>("xmin"),
      pset.getParameter<double>("xmax"),
      };
}

MEbinning BPHMonitor::getHistoLSPSet(edm::ParameterSet pset)
{
  return MEbinning{
    pset.getParameter<int32_t>("nbins"),
      0.,
      double(pset.getParameter<int32_t>("nbins"))
      };
}

void BPHMonitor::setMETitle(METME& me, std::string titleX, std::string titleY)
{
  me.numerator->setAxisTitle(titleX,1);
  me.numerator->setAxisTitle(titleY,2);
  me.denominator->setAxisTitle(titleX,1);
  me.denominator->setAxisTitle(titleY,2);

}

void BPHMonitor::bookME(DQMStore::IBooker &ibooker, METME& me, std::string& histname, std::string& histtitle, int& nbins, double& min, double& max)
{
  me.numerator   = ibooker.book1D(histname+"_numerator",   histtitle+" (numerator)",   nbins, min, max);
  me.denominator = ibooker.book1D(histname+"_denominator", histtitle+" (denominator)", nbins, min, max);
}
void BPHMonitor::bookME(DQMStore::IBooker &ibooker, METME& me, std::string& histname, std::string& histtitle, std::vector<double> binning)
{
  int nbins = binning.size()-1;
  std::vector<float> fbinning(binning.begin(),binning.end());
  float* arr = &fbinning[0];
  me.numerator   = ibooker.book1D(histname+"_numerator",   histtitle+" (numerator)",   nbins, arr);
  me.denominator = ibooker.book1D(histname+"_denominator", histtitle+" (denominator)", nbins, arr);
}
void BPHMonitor::bookME(DQMStore::IBooker &ibooker, METME& me, std::string& histname, std::string& histtitle, int& nbinsX, double& xmin, double& xmax, double& ymin, double& ymax)
{
  me.numerator   = ibooker.bookProfile(histname+"_numerator",   histtitle+" (numerator)",   nbinsX, xmin, xmax, ymin, ymax);
  me.denominator = ibooker.bookProfile(histname+"_denominator", histtitle+" (denominator)", nbinsX, xmin, xmax, ymin, ymax);
}
void BPHMonitor::bookME(DQMStore::IBooker &ibooker, METME& me, std::string& histname, std::string& histtitle, int& nbinsX, double& xmin, double& xmax, int& nbinsY, double& ymin, double& ymax)
{
  me.numerator   = ibooker.book2D(histname+"_numerator",   histtitle+" (numerator)",   nbinsX, xmin, xmax, nbinsY, ymin, ymax);
  me.denominator = ibooker.book2D(histname+"_denominator", histtitle+" (denominator)", nbinsX, xmin, xmax, nbinsY, ymin, ymax);
}
void BPHMonitor::bookME(DQMStore::IBooker &ibooker, METME& me, std::string& histname, std::string& histtitle, std::vector<double> binningX, std::vector<double> binningY)
{
  int nbinsX = binningX.size()-1;
  std::vector<float> fbinningX(binningX.begin(),binningX.end());
  float* arrX = &fbinningX[0];
  int nbinsY = binningY.size()-1;
  std::vector<float> fbinningY(binningY.begin(),binningY.end());
  float* arrY = &fbinningY[0];

  me.numerator   = ibooker.book2D(histname+"_numerator",   histtitle+" (numerator)",   nbinsX, arrX, nbinsY, arrY);
  me.denominator = ibooker.book2D(histname+"_denominator", histtitle+" (denominator)", nbinsX, arrX, nbinsY, arrY);
}

void BPHMonitor::bookHistograms(DQMStore::IBooker     & ibooker,
				 edm::Run const        & iRun,
				 edm::EventSetup const & iSetup) 
{  
  
  std::string histname, histtitle;

  std::string currentFolder = folderName_ ;
  ibooker.setCurrentFolder(currentFolder.c_str());
  
  histname = "muPt"; histtitle = "mu_P_{t}";
  bookME(ibooker,muPt_,histname,histtitle, pt_binning_.nbins, pt_binning_.xmin, pt_binning_.xmax);
  setMETitle(muPt_,"Mu_Pt[GeV]","events/1GeV");

  histname = "muPhi"; histtitle = "muPhi";
  bookME(ibooker,muPhi_,histname,histtitle, phi_binning_.nbins, phi_binning_.xmin, phi_binning_.xmax);
  setMETitle(muPhi_," mu_#phi","events / 0.1 rad");

  histname = "muEta"; histtitle = "mu_Eta";
  bookME(ibooker,muEta_,histname,histtitle, eta_binning_.nbins,eta_binning_.xmin, eta_binning_.xmax);
  setMETitle(muEta_," mu_#eta","events / ");

  histname = "mu_d0"; histtitle = "mu_d0";
  bookME(ibooker,mud0_,histname,histtitle, d0_binning_.nbins,d0_binning_.xmin, d0_binning_.xmax);
  setMETitle(mud0_," mu_d0","events /1cm ");

  histname = "mu_z0"; histtitle = "mu_z0";
  bookME(ibooker,muz0_,histname,histtitle, z0_binning_.nbins,z0_binning_.xmin, z0_binning_.xmax);
  setMETitle(muz0_," mu_z0","events /1cm ");

  // Initialize the GenericTriggerEventFlag
  if ( num_genTriggerEventFlag_ && num_genTriggerEventFlag_->on() ) num_genTriggerEventFlag_->initRun( iRun, iSetup );
  if ( den_genTriggerEventFlag_ && den_genTriggerEventFlag_->on() ) den_genTriggerEventFlag_->initRun( iRun, iSetup );

}

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "Geometry/Records/interface/TrackerTopologyRcd.h"
void BPHMonitor::analyze(edm::Event const& iEvent, edm::EventSetup const& iSetup)  {

//  edm::Handle<reco::BeamSpot> const& beamSpot;
  // Filter out events if Trigger Filtering is requested
  if (den_genTriggerEventFlag_->on() && ! den_genTriggerEventFlag_->accept( iEvent, iSetup) ) return;

  
  edm::Handle<reco::BeamSpot> beamSpot;
  iEvent.getByToken( bsToken_,  beamSpot);
  edm::Handle<reco::MuonCollection> muoHandle;
  iEvent.getByToken( muoToken_, muoHandle );

  if ( int(muoHandle->size()) < nmuons_ ) return;
  std::vector<reco::Muon> muons;
  for ( auto const & m : *muoHandle ) {
    if ( muoSelection_( m ) ) muons.push_back(m);
  }
  if ( int(muons.size()) < nmuons_ ) return;
  for (int i=0;i<(int(muons.size())+1) ;i++) {
    muPhi_.denominator->Fill(muons[i].phi());
    muEta_.denominator->Fill(muons[i].eta());
    muPt_.denominator ->Fill(muons[i].pt());
    const reco::Track * track = 0;
    if (muons[i].isTrackerMuon()) track = & * muons[i].innerTrack();
    else if (muons[i].isStandAloneMuon()) track = & * muons[i].outerTrack();
    if (track) {
      double d0 = track->dxy(beamSpot->position());
      double z0 = track->dz(beamSpot->position());
      mud0_.denominator ->Fill(d0);
      muz0_.denominator ->Fill(z0);
    }
  }
  // filling histograms (denominator)  
//  int ls = iEvent.id().luminosityBlock();//TODO LumiBlock, do we need that? 
//  metVsLS_.denominator -> Fill(ls, met);
  
  // applying selection for numerator
  if (num_genTriggerEventFlag_->on() && ! num_genTriggerEventFlag_->accept( iEvent, iSetup) ) return;

  for (int i=0;i<int((muons.size())+1);i++) {
    muPhi_.numerator->Fill(muons[i].phi());
    muEta_.numerator->Fill(muons[i].eta());
    muPt_.numerator ->Fill(muons[i].pt());
    const reco::Track * track = 0;
    if (muons[i].isTrackerMuon()) track = & * muons[i].innerTrack();
    else if (muons[i].isStandAloneMuon()) track = & * muons[i].outerTrack();
    if (track) {
      double d0 = track->dxy(beamSpot->position());
      double z0 = track->dz(beamSpot->position());
      mud0_.numerator ->Fill(d0);
      muz0_.numerator ->Fill(z0);
    }
  }


  // filling histograms (num_genTriggerEventFlag_)  

}

void BPHMonitor::fillHistoPSetDescription(edm::ParameterSetDescription & pset)
{
  pset.add<int>   ( "nbins");
  pset.add<double>( "xmin" );
  pset.add<double>( "xmax" );
}

void BPHMonitor::fillHistoLSPSetDescription(edm::ParameterSetDescription & pset)
{
  pset.add<int>   ( "nbins", 2500);
}

void BPHMonitor::fillDescriptions(edm::ConfigurationDescriptions & descriptions)
{
  edm::ParameterSetDescription desc;
  desc.add<std::string>  ( "FolderName", "HLT/BPH/" );

  desc.add<edm::InputTag>( "tracks",  edm::InputTag("generalTracks") );
  desc.add<edm::InputTag>( "offlinePVs",     edm::InputTag("offlinePrimaryVertices") );
  desc.add<edm::InputTag>( "beamSpot",edm::InputTag("offlineBeamSpot") );
  desc.add<edm::InputTag>( "muons",    edm::InputTag("muons") );
  desc.add<std::string>("muoSelection", "abs(eta)<1.4 & isPFMuon & isGlobalMuon  & innerTrack.hitPattern.trackerLayersWithMeasurement>5 & innerTrack.hitPattern.numberOfValidPixelHits>0");
  desc.add<int>("nmuons",     1);

  edm::ParameterSetDescription genericTriggerEventPSet;
  genericTriggerEventPSet.add<bool>("andOr");
  genericTriggerEventPSet.add<edm::InputTag>("dcsInputTag", edm::InputTag("scalersRawToDigi") );
  genericTriggerEventPSet.add<std::vector<int> >("dcsPartitions",{});
  genericTriggerEventPSet.add<bool>("andOrDcs", false);
  genericTriggerEventPSet.add<bool>("errorReplyDcs", true);
  genericTriggerEventPSet.add<std::string>("dbLabel","");
  genericTriggerEventPSet.add<bool>("andOrHlt", true);
  genericTriggerEventPSet.add<edm::InputTag>("hltInputTag", edm::InputTag("TriggerResults::HLT") );
  genericTriggerEventPSet.add<std::vector<std::string> >("hltPaths",{});
  genericTriggerEventPSet.add<std::string>("hltDBKey","");
  genericTriggerEventPSet.add<bool>("errorReplyHlt",false);
  genericTriggerEventPSet.add<unsigned int>("verbosityLevel",1);

  desc.add<edm::ParameterSetDescription>("numGenericTriggerEventPSet", genericTriggerEventPSet);
  desc.add<edm::ParameterSetDescription>("denGenericTriggerEventPSet", genericTriggerEventPSet);

  edm::ParameterSetDescription histoPSet;
  edm::ParameterSetDescription phiPSet;
  edm::ParameterSetDescription etaPSet;
  edm::ParameterSetDescription ptPSet;
  edm::ParameterSetDescription d0PSet;
  edm::ParameterSetDescription z0PSet;
  fillHistoPSetDescription(phiPSet);
  fillHistoPSetDescription(ptPSet);
  fillHistoPSetDescription(etaPSet);
  fillHistoPSetDescription(z0PSet);
  fillHistoPSetDescription(d0PSet);
  histoPSet.add<edm::ParameterSetDescription>("d0PSet", d0PSet);
  histoPSet.add<edm::ParameterSetDescription>("etaPSet", etaPSet);
  histoPSet.add<edm::ParameterSetDescription>("phiPSet", phiPSet);
  histoPSet.add<edm::ParameterSetDescription>("ptPSet", ptPSet);
  histoPSet.add<edm::ParameterSetDescription>("z0PSet", z0PSet);
  std::vector<double> bins = {0.,20.,40.,60.,80.,90.,100.,110.,120.,130.,140.,150.,160.,170.,180.,190.,200.,220.,240.,260.,280.,300.,350.,400.,450.,1000.};
  histoPSet.add<std::vector<double> >("metBinning", bins);

  edm::ParameterSetDescription lsPSet;
  fillHistoLSPSetDescription(lsPSet);
  histoPSet.add<edm::ParameterSetDescription>("lsPSet", lsPSet);

  desc.add<edm::ParameterSetDescription>("histoPSet",histoPSet);

  descriptions.add("bphMonitoring", desc);
}

// Define this as a plug-in
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(BPHMonitor);
