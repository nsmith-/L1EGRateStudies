// This macro is to be run using `root -q -b drawRateEff.C+`
// Note: the ./plots/ directory must exist!

// rootools can be found in ~nsmith/src/rootools
// It's just a small collection of useful utilities
#include "rootools.h"

#include <memory>
#include "TBox.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TMultiGraph.h"
#include "TPaletteAxis.h"
#include "TPaveStats.h"
#include "TTree.h"
#include "THStack.h"
#include "TF1.h"
#include "TF2.h"

TLatex * drawCMSString(std::string title) {
   TLatex * cmsString = new TLatex(
      gPad->GetAbsXlowNDC()+gPad->GetAbsWNDC()-gPad->GetLeftMargin(), 
      gPad->GetAbsYlowNDC()+gPad->GetAbsHNDC()-gPad->GetTopMargin()+0.005, 
      title.c_str());
   cmsString->SetTextFont(42);
   cmsString->SetTextSize(0.03);
   cmsString->SetNDC(1);
   cmsString->SetTextAlign(31);
   cmsString->Draw();
   return cmsString;
}

void setLegStyle(TLegend * leg) {
   leg->SetBorderSize(0);
   leg->SetLineColor(1);
   leg->SetLineStyle(1);
   leg->SetLineWidth(.3);
   leg->SetFillColor(0);
   leg->SetFillStyle(0);
   leg->SetTextFont(42);
}

void drawRates(std::vector<TH1F*> hists, TCanvas * c, double ymax, std::pair<double, double> xrange = {0., 0.}) {
   const std::vector<int> colors{kBlack, kRed, kBlue, kGreen, kOrange, kGray};
   const std::vector<int> marker_styles{20, 24, 25, 26, 32};
   c->Clear();
   TMultiGraph mg("mg", c->GetTitle());

   std::vector<TGraphErrors*> graphs;
   auto color = begin(colors);
   auto style = begin(marker_styles);
   for(auto& hist: hists)
   {
      graphs.push_back(new TGraphErrors((TH1 *) hist));
      graphs.back()->SetLineColor(*color);
      graphs.back()->SetMarkerColor(*color++);
      graphs.back()->SetMarkerStyle(*style++);
      graphs.back()->SetMarkerSize(0.8);
      mg.Add(graphs.back());
   }
   mg.Draw("aplez");

   if ( c->GetLogy() == 0 ) // linear
      mg.SetMinimum(0.);
   else
      mg.SetMinimum(10.);

   if ( ymax != 0. )
      mg.SetMaximum(ymax);

   TLegend *leg = new TLegend(0.5,0.76,0.9,0.9);
   setLegStyle(leg);
   for (auto& graph : graphs) leg->AddEntry(graph, graph->GetTitle(),"lpe");
   leg->Draw("same");
   mg.GetXaxis()->SetTitle(hists[0]->GetXaxis()->GetTitle());
   if ( xrange.first != 0. || xrange.second != 0 )
     mg.GetXaxis()->SetRangeUser(xrange.first, xrange.second);
   mg.GetYaxis()->SetTitle(hists[0]->GetYaxis()->GetTitle());

   auto cmsString = drawCMSString("CMS Simulation, <PU>=140 bx=25, MinBias");

   c->Print(("plots/"+std::string(c->GetName())+".png").c_str());
   delete leg;
   delete cmsString;
}

void drawEfficiency(std::vector<TGraphAsymmErrors*> graphs, TCanvas * c, double ymax, std::pair<double, double> xrange = {0., 0.}, bool fit = false, std::vector<double> fitHint = {1., 15., 3., 0.}) {
   const std::vector<int> colors{kBlack, kRed, kBlue, kGreen, kOrange, kGray};
   const std::vector<int> marker_styles{20, 24, 25, 26, 32};
   c->Clear();
   TMultiGraph mg("mg", c->GetTitle());

   auto color = begin(colors);
   std::vector<int> lines = {kSolid, kDashed, kDotted};
   auto linestyle = begin(lines);
   auto style = begin(marker_styles);
   for(auto& graph : graphs)
   {
      graph->SetLineColor(*color);
      graph->SetMarkerColor(*color++);
      graph->SetMarkerStyle(*style++);
      graph->SetMarkerSize(0.8);
      mg.Add(graph);
   }
   mg.Draw("apez");

   if ( c->GetLogy() == 0 ) // linear
      mg.SetMinimum(0.);
   else
      mg.SetMinimum(10.);

   if ( ymax != 0. )
      mg.SetMaximum(ymax);

   if ( fit && xrange.second != xrange.first )
   {
      for(auto& graph : graphs)
      {
         TF1 shape("shape", "[0]/2*(1+TMath::Erf((x-[1])/([2]*sqrt(x))))+[3]*x", xrange.first, xrange.second);
         shape.SetParameters(fitHint[0], fitHint[1], fitHint[2], fitHint[3]);
         // Somehow, step size increases each time, have to find a way to control it...
         graph->Fit(&shape);
         graph->GetFunction("shape")->SetLineColor(graph->GetLineColor());
         graph->GetFunction("shape")->SetLineWidth(graph->GetLineWidth()*2);
         graph->GetFunction("shape")->SetLineStyle(*linestyle++);
      }
   }

   TLegend *leg = new TLegend(0.5,0.76,0.9,0.9);
   setLegStyle(leg);
   for (auto& graph: graphs) leg->AddEntry(graph, graph->GetTitle(),"lpe");
   leg->Draw("same");
   mg.GetXaxis()->SetTitle(graphs[0]->GetXaxis()->GetTitle());
   if ( xrange.first != 0. || xrange.second != 0 )
     mg.GetXaxis()->SetRangeUser(xrange.first, xrange.second);
   mg.GetYaxis()->SetTitle(graphs[0]->GetYaxis()->GetTitle());

   auto cmsString = drawCMSString("CMS Simulation, <PU>=140 bx=25, Single Electron");

   c->Print(("plots/"+std::string(c->GetName())+".png").c_str());
   delete leg;
   delete cmsString;
   for(auto& graph : graphs)
   {
      mg.RecursiveRemove(graph);
   }
}

void draw2DdeltaRHist(TH2F* hist, TCanvas * c) {
   c->Clear();
   c->cd();
   gStyle->SetOptTitle(0);
   double margin = 0.07;
   double histpad_size = 0.7;
   auto hist_pad = new TPad((c->GetName()+std::string("_hist")).c_str(), "subpad", margin, margin, histpad_size+margin, histpad_size+margin, c->GetFillColor());
   hist_pad->SetMargin(0., 0., 0., 0.);
   hist_pad->Draw();
   auto xprojection_pad = new TPad((c->GetName()+std::string("_xprojection")).c_str(), "subpad", margin, histpad_size+margin, histpad_size+margin, 1-margin, c->GetFillColor());
   xprojection_pad->SetMargin(0., 0., 0., 0.);
   xprojection_pad->Draw();
   auto yprojection_pad = new TPad((c->GetName()+std::string("_yprojection")).c_str(), "subpad", histpad_size+margin, margin, 1-margin, histpad_size+margin, c->GetFillColor());
   yprojection_pad->SetMargin(0., 0., 0., 0.);
   yprojection_pad->Draw();

   hist->Sumw2();
   hist->Scale(1./hist->Integral());
   auto xprojection_hist = hist->ProjectionX(hist->GetName(), 1, hist->GetNbinsY(), "e");
   auto xprojection = new TGraphErrors(xprojection_hist);
   xprojection->SetLineColor(kBlack);
   xprojection->SetMarkerColor(kBlack);
   auto yprojection_hist = hist->ProjectionY(hist->GetName(), 1, hist->GetNbinsX(), "e");
   auto yprojection = new TGraphErrors(yprojection_hist->GetXaxis()->GetNbins());
   yprojection->SetName((yprojection_hist->GetName()+std::string("_graph")).c_str());
   TAxis * yproj_xaxis = yprojection_hist->GetXaxis();
   for(int i=0; i<yprojection_hist->GetXaxis()->GetNbins(); i++)
   {
      double bin = yproj_xaxis->GetBinCenter(i+1);
      double width = yprojection_hist->GetBinWidth(i+1)*gStyle->GetErrorX();
      double count = yprojection_hist->GetBinContent(i+1);
      double err = yprojection_hist->GetBinError(i+1);
      yprojection->SetPoint(i, count, bin);
      yprojection->SetPointError(i, err, width);
   }

   // Draw 2D hist
   hist_pad->cd();
   hist->Draw("col");
   hist->GetYaxis()->SetTitleOffset(1.4);
   
   // Fit hist
   TF2 shape("2dshape", "[0]*exp(-[2]*(x[0]-[1])**2-[4]*(x[1]-[3])**2-2*[5]*(x[0]-[1])*(x[1]-[3]))", -0.05, 0.05, -0.05, 0.05);
   shape.SetParameters(0.003, 0., 3.769e4, 0., 4.215e4, -1.763e4);
   hist->Fit(&shape, "n");
   const double max = shape.GetParameter(0);
   const double contours[3] {max*exp(-4.5), max*exp(-2), max*exp(-0.5)};
   shape.SetContour(3, contours);
   shape.SetNpx(100);
   shape.SetNpy(100);
   shape.SetLineWidth(2);
   shape.SetLineColor(kRed);
   shape.Draw("cont3 same");
   
   // One crystal box
   TBox crystalBox(-0.0173/2, -0.0173/2, 0.0173/2, 0.0173/2);
   crystalBox.SetLineStyle(3);
   crystalBox.SetLineColor(kGray);
   crystalBox.SetLineWidth(2);
   crystalBox.SetFillStyle(0);
   crystalBox.Draw();

   // Draw x projection
   xprojection_pad->cd();
   xprojection->Draw("apez");
   xprojection->GetYaxis()->SetNdivisions(0);
   xprojection->GetXaxis()->SetRangeUser(hist->GetXaxis()->GetBinLowEdge(1), hist->GetXaxis()->GetBinUpEdge(hist->GetXaxis()->GetNbins()));
   xprojection->GetXaxis()->SetLabelSize(0.);
   xprojection->GetYaxis()->SetRangeUser(0., 0.22);
   TF1 shapeprojX("shapeprojX", "[0]*sqrt(([2]*[4]-[5]**2)/(TMath::Pi()*[2]))*exp(([5]**2-[2]*[4])*(x-[3])**2/[2])", -0.05, 0.05);
   shapeprojX.SetParameters(shape.GetParameters());
   shapeprojX.SetParameter(0, shape.GetParameter(0)/20);
   shapeprojX.SetLineWidth(2);
   shapeprojX.SetNpx(100);
   shapeprojX.SetLineColor(kRed);
   shapeprojX.Draw("same");

   // Draw y projection
   yprojection_pad->cd();
   yprojection->Draw("apez");
   yprojection->GetXaxis()->SetNdivisions(0);
   yprojection->GetXaxis()->SetRangeUser(0., 0.2);
   yprojection->GetYaxis()->SetRangeUser(hist->GetYaxis()->GetBinLowEdge(1), hist->GetYaxis()->GetBinUpEdge(hist->GetYaxis()->GetNbins()));
   yprojection->GetYaxis()->SetLabelSize(0.);
   TF1 shapeprojY("shapeprojY", "[0]*sqrt(([2]*[4]-[5]**2)/(TMath::Pi()*[4]))*exp(([5]**2-[2]*[4])*(x-[1])**2/[4])", -0.05, 0.05);
   shapeprojY.SetParameters(shape.GetParameters());
   shapeprojY.SetParameter(0, shape.GetParameter(0)/20);
   double shapeprojYpos[101], shapeprojYval[101];
   for(int i=0; i<101; ++i) {
      shapeprojYpos[i] = 1e-3*i-0.05;
      shapeprojYval[i] = shapeprojY.Eval(shapeprojYpos[i]);
   }
   TGraph shapeprojYLine(101, shapeprojYval, shapeprojYpos);
   shapeprojYLine.SetLineColor(kRed);
   shapeprojYLine.SetLineWidth(2);
   shapeprojYLine.Draw("l");

   // Draw Title
   c->cd();
   if ( *(c->GetTitle()) != '\0' ) {
      auto title = new TLatex(margin, 1-margin+0.01, "Crystal-level EG Trigger #DeltaR Distribution");
      title->SetTextSize(0.04);
      title->SetTextFont(42);
      title->SetNDC();
      title->Draw();
   }

   // CMS info string
   TLatex * cmsString = new TLatex(
      histpad_size+margin-0.005, 
      1-margin-0.005, 
      "CMS Simulation, <PU>=140 bx=25, Single Electron");
   cmsString->SetTextFont(42);
   cmsString->SetTextSize(0.02);
   cmsString->SetNDC(1);
   cmsString->SetTextAlign(33);
   cmsString->Draw();

   // Stats
   TLatex *stats[5];
   stats[0] = new TLatex(histpad_size+margin+0.01, histpad_size+margin+0.13, ("#mu_#eta = "+std::to_string(shape.GetParameter(1))).c_str());
   stats[1] = new TLatex(histpad_size+margin+0.01, histpad_size+margin+0.1, ("#mu_#phi = "+std::to_string(shape.GetParameter(3))).c_str());
   stats[2] = new TLatex(histpad_size+margin+0.01, histpad_size+margin+0.07, ("#sigma_#eta#eta = "+std::to_string(sqrt(0.5/shape.GetParameter(2)))).c_str());
   stats[3] = new TLatex(histpad_size+margin+0.01, histpad_size+margin+0.04, ("#sigma_#phi#phi = "+std::to_string(sqrt(0.5/shape.GetParameter(4)))).c_str());
   stats[4] = new TLatex(histpad_size+margin+0.01, histpad_size+margin+0.01, ("#sigma_#eta#phi = "+std::to_string(sqrt(-0.5/shape.GetParameter(5)))).c_str());
   for(int i=0; i<5; ++i) {
      stats[i]->SetTextSize(0.024);
      stats[i]->SetTextFont(42);
      stats[i]->SetNDC();
      stats[i]->Draw();
   }

   // Draw palette
   // (not working)
   gPad->Update();
   auto palette = new TPaletteAxis(1-margin+0.01, margin, 1-0.01, histpad_size+margin, hist);
   palette->Draw();
   gPad->Modified();
   gPad->Update();

   c->Print(("plots/"+std::string(c->GetName())+".png").c_str());
   delete yprojection;
   delete cmsString;
   
   gStyle->SetOptTitle(1);
}

void drawDRHists(std::vector<TH1F*> hists, TCanvas * c, double ymax) {
   const std::vector<int> colors{kBlack, kRed, kBlue, kGreen, kOrange, kGray};
   const std::vector<int> marker_styles{20, 24, 25, 26, 32};
   THStack hs("hs", c->GetTitle());
   auto color = begin(colors);
   auto style = begin(marker_styles);
   for(auto& hist : hists)
   {
      hist->Sumw2();
      hist->Scale(1./hist->Integral());
      hist->SetLineColor(*color);
      hist->SetMarkerColor(*color++);
      hist->SetMarkerStyle(*style++);
      hist->SetMarkerSize(0.8);
      hs.Add(hist, "ex0 hist");
   }
   c->Clear();
   if ( c->GetLogy() == 0 ) // linear
      hs.SetMinimum(0.);
   if ( ymax != 0. )
      hs.SetMaximum(ymax);

   hs.Draw("nostack");

   std::vector<TGraph> markers;
   for(auto& hist : hists)
   {
      markers.emplace_back(hist);
      markers.back().Draw("psame");
   }

   TF1 * fit = new TF1("doublegaus", "gaus+gaus(3)", 0., 0.25);
   fit->SetParameters(0.3, 0., 0.003, 0.1, 0., 0.02);
   //hists[0]->Fit(fit, "n");
   //fit->Draw("lsame");

   TLegend *leg = new TLegend(0.5,0.76,0.9,0.9);
   setLegStyle(leg);
   for(auto& hist : hists) leg->AddEntry(hist, hist->GetTitle(),"elp");
   leg->Draw("same");
   hs.GetXaxis()->SetTitle(hists[0]->GetXaxis()->GetTitle());
   hs.GetYaxis()->SetTitle(hists[0]->GetYaxis()->GetTitle());
   hs.GetYaxis()->SetTitleOffset(1.2);
   hs.GetYaxis()->SetTitle("Fraction of Events");
   //hs.GetXaxis()->SetRangeUser(0., 0.1);

   auto cmsString = drawCMSString("CMS Simulation, <PU>=140 bx=25, Single Electron");
               
   c->Print(("plots/"+std::string(c->GetName())+".png").c_str());
   c->Clear();
   markers.clear();

   // Now for integral
   for(auto& hist : hists)
   {
      hs.RecursiveRemove(hist);
      TH1F * intHist = (TH1F*) hist->Clone((hist->GetName()+std::string("_cdf")).c_str());
      double integral = 0;
      for(int bin=0; bin<=intHist->GetNbinsX(); ++bin)
      {
         integral += intHist->GetBinContent(bin);
         intHist->SetBinContent(bin, integral);
      }
      hs.Add(intHist, "ex0 hist");
      markers.emplace_back(intHist);
   }
   hs.SetMaximum(1.2);
   hs.GetYaxis()->SetTitle((std::string("Cumulative ")+hs.GetYaxis()->GetTitle()).c_str());
   hs.Draw("nostack");
   for(auto& m : markers) m.Draw("psame");
   leg->Draw("same");
   auto cmsString2 = drawCMSString("CMS Simulation, <PU>=140 bx=25, Single Electron");
   c->Print(("plots/"+std::string(c->GetName())+"_cdf.png").c_str());

   delete leg;
   delete cmsString;
   delete cmsString2;
}

void drawRateEff() {
   gStyle->SetOptStat(0);
   gStyle->SetTitleFont(42, "p");
   gStyle->SetTitleColor(1);
   gStyle->SetTitleTextColor(1);
   gStyle->SetTitleFillColor(10);
   gStyle->SetTitleFontSize(0.05);
   gStyle->SetTitleFont(42, "XYZ");
   gStyle->SetLabelFont(42, "XYZ");

   TCanvas * c = new TCanvas("canvas", "Canvas", 800, 600);
	
   TFile * rates = new TFile("egTriggerRates.root");
   TFile * eff = new TFile("egTriggerEff.root");

   auto newAlgRateHist = (TH1F *) rates->Get("analyzer/dyncrystalEG_rate");
   newAlgRateHist->SetTitle("L1EGamma_Crystal");
   auto oldAlgRateHist = (TH1F *) rates->Get("analyzer/SLHCL1ExtraParticles:EGamma_rate");
   oldAlgRateHist->SetTitle("Original L2 Algorithm");
   auto dynAlgRateHist = (TH1F *) rates->Get("analyzer/SLHCL1ExtraParticlesNewClustering:EGamma_rate");
   dynAlgRateHist->SetTitle("LLR Alg.");
   auto run1AlgRateHist = (TH1F *) rates->Get("analyzer/l1extraParticles:All_rate");
   run1AlgRateHist->SetTitle("Run 1 Alg.");
   auto crystalAlgRateHist = (TH1F *) rates->Get("analyzer/L1EGammaCrystalsProducer:EGammaCrystal_rate");
   crystalAlgRateHist->SetTitle("Crystal Trigger (prod.)");
   auto UCTAlgRateHist = (TH1F *) rates->Get("analyzer/l1extraParticlesUCT:All_rate");
   UCTAlgRateHist->SetTitle("Phase 1 TDR");

   auto effHistKeys = rootools::getKeysofClass(eff, "analyzer", "TGraphAsymmErrors");

   auto newAlgEtaHist = (TGraphAsymmErrors *) eff->Get("analyzer/divide_dyncrystalEG_efficiency_eta_by_gen_eta");
   newAlgEtaHist->SetTitle("L1EGamma_Crystal");
   auto newAlgPtHist = (TGraphAsymmErrors *) eff->Get("analyzer/divide_dyncrystalEG_efficiency_pt_by_gen_pt");
   newAlgPtHist->SetTitle("L1EGamma_Crystal");
   auto newAlgRecoPtHists = rootools::loadObjectsMatchingPattern<TGraphAsymmErrors>(effHistKeys, "divide_dyncrystalEG_threshold*_reco_pt");
   for(auto& hist : newAlgRecoPtHists) hist->SetTitle("Crystal Algorithm");
   auto newAlgGenPtHists = rootools::loadObjectsMatchingPattern<TGraphAsymmErrors>(effHistKeys, "divide_dyncrystalEG_threshold*_gen_pt");
   for(auto& hist : newAlgGenPtHists) hist->SetTitle("Crystal Algorithm");
   auto newAlgDRHist = (TH1F *) eff->Get("analyzer/dyncrystalEG_deltaR");
   newAlgDRHist->SetTitle("L1EGamma_Crystal");

   auto oldAlgEtaHist = (TGraphAsymmErrors *) eff->Get("analyzer/divide_SLHCL1ExtraParticles:EGamma_efficiency_eta_by_gen_eta");
   oldAlgEtaHist->SetTitle("Original L2 Algorithm");
   auto oldAlgPtHist = (TGraphAsymmErrors *) eff->Get("analyzer/divide_SLHCL1ExtraParticles:EGamma_efficiency_pt_by_gen_pt");
   oldAlgPtHist->SetTitle("Original L2 Algorithm");
   auto oldAlgRecoPtHists = rootools::loadObjectsMatchingPattern<TGraphAsymmErrors>(effHistKeys, "divide_SLHCL1ExtraParticles:EGamma_threshold*_reco_pt");
   for(auto& hist : oldAlgRecoPtHists) hist->SetTitle("Original L2 Algorithm");
   auto oldAlgDRHist = (TH1F *) eff->Get("analyzer/SLHCL1ExtraParticles:EGamma_deltaR");
   oldAlgDRHist->SetTitle("Original L2 Algorithm");

   auto dynAlgEtaHist = (TGraphAsymmErrors *) eff->Get("analyzer/divide_SLHCL1ExtraParticlesNewClustering:EGamma_efficiency_eta_by_gen_eta");
   dynAlgEtaHist->SetTitle("LLR Alg.");
   auto dynAlgPtHist = (TGraphAsymmErrors *) eff->Get("analyzer/divide_SLHCL1ExtraParticlesNewClustering:EGamma_efficiency_pt_by_gen_pt");
   dynAlgPtHist->SetTitle("LLR Alg.");
   auto dynAlgRecoPtHists = rootools::loadObjectsMatchingPattern<TGraphAsymmErrors>(effHistKeys, "divide_SLHCL1ExtraParticlesNewClustering:EGamma_threshold*_reco_pt");
   for(auto& hist : dynAlgRecoPtHists) hist->SetTitle("Tower Algorithm 2");
   auto dynAlgDRHist = (TH1F *) eff->Get("analyzer/SLHCL1ExtraParticlesNewClustering:EGamma_deltaR");
   dynAlgDRHist->SetTitle("LLR Alg.");

   auto run1AlgEtaHist = (TGraphAsymmErrors *) eff->Get("analyzer/divide_l1extraParticles:All_efficiency_eta_by_gen_eta");
   run1AlgEtaHist->SetTitle("Run 1 Alg.");
   auto run1AlgPtHist = (TGraphAsymmErrors *) eff->Get("analyzer/divide_l1extraParticles:All_efficiency_pt_by_gen_pt");
   run1AlgPtHist->SetTitle("Run 1 Alg.");
   auto run1AlgRecoPtHists = rootools::loadObjectsMatchingPattern<TGraphAsymmErrors>(effHistKeys, "divide_l1extraParticles:All_threshold*_reco_pt");
   for(auto& hist : run1AlgRecoPtHists) hist->SetTitle("Run 1 Alg.");
   auto run1AlgDRHist = (TH1F *) eff->Get("analyzer/l1extraParticles:All_deltaR");
   run1AlgDRHist->SetTitle("Run 1 Alg.");

   auto crystalAlgPtHist = (TGraphAsymmErrors *) eff->Get("analyzer/divide_L1EGammaCrystalsProducer:EGammaCrystal_efficiency_pt_by_gen_pt");
   crystalAlgPtHist->SetTitle("Crystal Trigger (prod.)");
   auto crystalAlgRecoPtHists = rootools::loadObjectsMatchingPattern<TGraphAsymmErrors>(effHistKeys, "divide_L1EGammaCrystalsProducer:EGammaCrystal_threshold*_reco_pt");
   for(auto& hist : crystalAlgRecoPtHists) hist->SetTitle("Crystal Algorithm");
   auto crystalAlgGenPtHists = rootools::loadObjectsMatchingPattern<TGraphAsymmErrors>(effHistKeys, "divide_L1EGammaCrystalsProducer:EGammaCrystal_threshold*_gen_pt");
   for(auto& hist : crystalAlgGenPtHists) hist->SetTitle("L1EGamma_Crystal");

   const char * title = "Phase 1 TDR";
   auto UCTAlgEtaHist = (TGraphAsymmErrors *) eff->Get("analyzer/divide_l1extraParticlesUCT:All_efficiency_eta_by_gen_eta");
   UCTAlgEtaHist->SetTitle(title);
   auto UCTAlgPtHist = (TGraphAsymmErrors *) eff->Get("analyzer/divide_l1extraParticlesUCT:All_efficiency_pt_by_gen_pt");
   UCTAlgPtHist->SetTitle(title);
   auto UCTAlgRecoPtHists = rootools::loadObjectsMatchingPattern<TGraphAsymmErrors>(effHistKeys, "divide_l1extraParticlesUCT:All_threshold*_reco_pt");
   for(auto& hist : UCTAlgRecoPtHists) hist->SetTitle(title);
   auto UCTAlgGenPtHists = rootools::loadObjectsMatchingPattern<TGraphAsymmErrors>(effHistKeys, "divide_l1extraParticlesUCT:All_threshold*_gen_pt");
   for(auto& hist : UCTAlgGenPtHists) hist->SetTitle(title);
   auto UCTAlgDRHist = (TH1F *) eff->Get("analyzer/l1extraParticlesUCT:All_deltaR");
   UCTAlgDRHist->SetTitle(title);

   c->SetLogy(1);
   c->SetGridx(1);
   c->SetGridy(1);
   gStyle->SetGridStyle(2);
   gStyle->SetGridColor(kGray+1);
   c->SetName("dyncrystalEG_rate");
   c->SetTitle("");
   drawRates({newAlgRateHist, UCTAlgRateHist}, c, 40000., {0., 50.});
   c->SetName("dyncrystalEG_rate_UW");
   c->SetTitle("EG Rates (UW only)");
   drawRates({newAlgRateHist, UCTAlgRateHist, dynAlgRateHist}, c, 40000., {0., 50.});
   c->SetLogy(0);

   c->SetName("dyncrystalEG_efficiency_eta");
   c->SetTitle("EG Efficiencies");
   drawEfficiency({newAlgEtaHist, UCTAlgEtaHist, dynAlgEtaHist}, c, 1.2, {-2.5, 2.5});
   c->SetName("dyncrystalEG_efficiency_pt_UW");
   c->SetTitle("EG Efficiencies (UW only)");
   drawEfficiency({newAlgPtHist, UCTAlgPtHist, dynAlgPtHist}, c, 1.2, {0., 50.}, true, {0.9, 2., 1., 0.});
   c->SetName("dyncrystalEG_efficiency_pt");
   c->SetTitle("");
   drawEfficiency({newAlgPtHist, UCTAlgPtHist}, c, 1.2, {0., 50.}, true, {0.9, 2., 1., 0.});
   c->SetName("dyncrystalEG_threshold20_efficiency_gen_pt");
   //c->SetTitle("EG Turn-On Efficiencies, 20GeV Threshold");
   drawEfficiency({crystalAlgGenPtHists[0], UCTAlgGenPtHists[0]}, c, 1.2, {0., 50.}, true, {0.9, 20., 1., 0.});
   c->SetName("dyncrystalEG_threshold30_efficiency_gen_pt");
   //c->SetTitle("EG Turn-On Efficiencies, 30GeV Threshold");
   drawEfficiency({crystalAlgGenPtHists[1], UCTAlgGenPtHists[1]}, c, 1.2, {0., 50.}, true, {0.95, 30., 1., 0.});
   c->SetName("dyncrystalEG_threshold16_efficiency_gen_pt");
   //c->SetTitle("EG Turn-On Efficiencies, 16GeV Threshold");
   drawEfficiency({crystalAlgGenPtHists[2], UCTAlgGenPtHists[2]}, c, 1.2, {0., 50.}, true, {0.95, 16., 1., 0.});

   // Offline reco pt
   auto crystal_tree = (TTree *) eff->Get("analyzer/crystal_tree");
   auto offlineRecoHist = new TH2F("offlineRecoHist", "Offline reco to gen. comparison;Gen. pT (GeV);(reco-gen)/gen;Counts", 60, 0., 50., 60, -0.5, 0.5);
   crystal_tree->Draw("(reco_pt-gen_pt)/gen_pt:gen_pt >> offlineRecoHist", "reco_pt>0", "colz");
   c->SetLogy(0);
   offlineRecoHist->Draw("colz");
   c->Print("plots/offlineReco_vs_gen.png");
   c->Clear();

   // DeltaR stuff
   c->SetGridx(0);
   c->SetGridy(0);
   c->SetName("dyncrystalEG_deltaR");
   c->SetTitle("");
   drawDRHists({newAlgDRHist, UCTAlgDRHist}, c, 0.);
   c->SetName("dyncrystalEG_deltaR_UW");
   c->SetTitle("");
   drawDRHists({newAlgDRHist, UCTAlgDRHist, dynAlgDRHist}, c, 0.);
   TH1F * newAlgDRCutsHist = new TH1F("newAlgDRCutsHist", "L1EGamma_Crystal", 50, 0., .25);
   crystal_tree->Draw("deltaR >> newAlgDRCutsHist", "passed && gen_pt > 20.", "goff");
   c->SetName("dyncrystalEG_deltaR_ptcut");
   drawDRHists({newAlgDRCutsHist}, c, 0.);

   c->Clear();
   auto brem_dphi = new TH2F("brem_dphi", ";d#phi;(uslE+lslE)/clusterEnergy", 50, -0.1, 0.1, 50, 0, 1);
   crystal_tree->Draw("(uslE+lslE)/cluster_energy : deltaPhi >> brem_dphi", "passed && cluster_pt > 10", "goff");
   brem_dphi->Draw("colz");
   c->Print("plots/brem_dphi_hist.png");

   auto dynCrystal2DdeltaRHist = (TH2F *) eff->Get("analyzer/dyncrystalEG_2DdeltaR_hist");
   c->SetCanvasSize(800, 700);
   c->SetName("dyncrystalEG_2D_deltaR");
   //c->SetTitle("#Delta R Distribution Fit");
   c->SetTitle("");
   draw2DdeltaRHist(dynCrystal2DdeltaRHist, c);

   c->Clear();
   c->SetCanvasSize(700, 600);
   c->SetGridx(1);
   c->SetGridy(1);
   c->SetRightMargin(0.14);
   c->SetTopMargin(0.13);
   auto recoGenPtHist = (TH2F *) eff->Get("analyzer/reco_gen_pt");
   //recoGenPtHist->SetTitle("Crystal EG algorithm pT resolution");
   recoGenPtHist->SetTitle("");
   recoGenPtHist->GetYaxis()->SetTitle("Relative Error (reco-gen)/gen");
   recoGenPtHist->GetYaxis()->SetTitleOffset(1.3);
   recoGenPtHist->SetMaximum(50);
   recoGenPtHist->Draw("colz");
   auto cmsString = drawCMSString("CMS Simulation, <PU>=140 bx=25, Single Electron");
   c->Print("plots/dyncrystalEG_reco_gen_pt.png");
   delete cmsString;

   c->Clear();
   recoGenPtHist->SetTitle("Crystal EG algorithm pT resolution");
   // auto oldAlgrecoGenPtHist = (TH2F *) eff->Get("analyzer/SLHCL1ExtraParticles:EGamma_reco_gen_pt");
   auto oldAlgrecoGenPtHist = (TH2F *) eff->Get("analyzer/l1extraParticlesUCT:All_reco_gen_pt");
   oldAlgrecoGenPtHist->SetTitle("Tower EG alg. momentum error");
   oldAlgrecoGenPtHist->GetYaxis()->SetTitle("Relative Error (reco-gen)/gen");
   oldAlgrecoGenPtHist->SetMaximum(50);
   oldAlgrecoGenPtHist->SetLineColor(kRed);
   c->SetCanvasSize(1200,600);
   c->Divide(2,1);
   c->cd(1);
   gPad->SetGridx(1);
   gPad->SetGridy(1);
   recoGenPtHist->Draw("colz");
   recoGenPtHist->GetYaxis()->SetTitleOffset(1.4);
   c->cd(2);
   gPad->SetGridx(1);
   gPad->SetGridy(1);
   oldAlgrecoGenPtHist->Draw("colz");
   oldAlgrecoGenPtHist->GetYaxis()->SetTitleOffset(1.4);
   c->Print("plots/reco_gen_pt.png");
}
