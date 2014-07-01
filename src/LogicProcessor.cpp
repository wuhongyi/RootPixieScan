/** \file LogicProcessor.cpp
 * \brief handling of logic events, derived from MtcProcessor.cpp
 *
 * Start subtype corresponds to leading edge
 * Stop subtype corresponds to trailing edge
 */

#include <iostream>
#include <string>
#include <vector>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "LogicProcessor.hpp"

using namespace std;
using namespace dammIds::logic;

namespace dammIds {
    namespace logic {
        const int D_COUNTER_START = 0;
        const int D_COUNTER_STOP  = 5;
        const int D_TDIFF_STARTX  = 10;
        const int D_TDIFF_STOPX   = 20;
        const int D_TDIFF_SUMX    = 30;
        const int D_TDIFF_LENGTHX = 50;
        const int DD_RUNTIME_LOGIC = 80;
    }
} // logic namespace


LogicProcessor::LogicProcessor(void) : 
    EventProcessor(OFFSET, RANGE), lastStartTime(MAX_LOGIC, NAN), lastStopTime(MAX_LOGIC, NAN),
    logicStatus(MAX_LOGIC), stopCount(MAX_LOGIC), startCount(MAX_LOGIC)
{
    name = "Logic";
    associatedTypes.insert("logic");
    plotSize = SA;
}

void LogicProcessor::DeclarePlots(void)
{
    const int counterBins = S4;
    const int timeBins = SC;

    DeclareHistogram1D(D_COUNTER_START, counterBins, "logic start counter");
    DeclareHistogram1D(D_COUNTER_STOP, counterBins, "logic stop counter");
    for (int i=0; i < MAX_LOGIC; i++) {
        DeclareHistogram1D(D_TDIFF_STARTX + i, timeBins, "tdiff btwn logic starts, 10 us/bin");
        DeclareHistogram1D(D_TDIFF_STOPX + i, timeBins, "tdiff btwn logic stops, 10 us/bin");
        DeclareHistogram1D(D_TDIFF_SUMX + i, timeBins, "tdiff btwn both logic, 10 us/bin");
        DeclareHistogram1D(D_TDIFF_LENGTHX + i, timeBins, "logic high time, 10 us/bin");
    }

    DeclareHistogram2D(DD_RUNTIME_LOGIC, plotSize, plotSize, "runtime logic [1ms]");
    for (int i=1; i < MAX_LOGIC; i++) {
	DeclareHistogram2D(DD_RUNTIME_LOGIC+i, plotSize, plotSize, "runtime logic [1ms]");
    }
}

bool LogicProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
	return false;

    BasicProcessing(event);
    TriggerProcessing(event);
    
    EndProcess(); // update processing time
    return true;
}

void LogicProcessor::BasicProcessing(RawEvent &event) {
    const double logicPlotResolution = 10e-6 / pixie::clockInSeconds;
    
    static const vector<ChanEvent*> &events = sumMap["logic"]->GetList();
    
    for (vector<ChanEvent*>::const_iterator it = events.begin(); it != events.end(); it++) {
	ChanEvent *chan = *it;
        
	string subtype   = chan->GetChanID().GetSubtype();
	unsigned int loc = chan->GetChanID().GetLocation();
	double time = chan->GetTime();
	double timediff;

	if(subtype == "start") {
	    if (!isnan(lastStartTime.at(loc))) {
	        timediff = time - lastStartTime.at(loc);
	        //PackRoot(timediff, loc, true);
		plot(D_TDIFF_STARTX + loc, timediff / logicPlotResolution);
		plot(D_TDIFF_SUMX + loc,   timediff / logicPlotResolution);
	    }

	    //? bounds checking
	    lastStartTime.at(loc) = time;
	    logicStatus.at(loc) = true;

	    startCount.at(loc)++;
	    plot(D_COUNTER_START, loc);
	} else if (subtype == "stop") {
  	    if (!isnan(lastStopTime.at(loc))) {
		timediff = time - lastStopTime.at(loc);
		//PackRoot(timediff, loc, false);
		plot(D_TDIFF_STOPX + loc, timediff / logicPlotResolution);
		plot(D_TDIFF_SUMX + loc,  timediff / logicPlotResolution);
		if (!isnan(lastStartTime.at(loc))) {
  		    double moveTime = time - lastStartTime.at(loc);    
		    plot(D_TDIFF_LENGTHX + loc, moveTime / logicPlotResolution);
		}
	    }
	    //? bounds checking
	    lastStopTime.at(loc) = time;
	    logicStatus.at(loc) = false;

	    stopCount.at(loc)++;
	    plot(D_COUNTER_STOP, loc);	  
	}
    }
}

void LogicProcessor::TriggerProcessing(RawEvent &event) {
    const double logicPlotResolution = 1e-3 / pixie::clockInSeconds;
    const long maxBin = plotSize * plotSize;
    
    static DetectorSummary *stopsSummary = event.GetSummary("logic:stop");
    static DetectorSummary *triggersSummary = event.GetSummary("logic:trigger");

    static const vector<ChanEvent*> &stops = stopsSummary->GetList();
    static const vector<ChanEvent*> &triggers = triggersSummary->GetList();
    static int firstTimeBin = -1;
    
    for (vector<ChanEvent*>::const_iterator it = stops.begin(); it != stops.end(); it++) {
	ChanEvent *chan = *it;
        
	unsigned int loc = chan->GetChanID().GetLocation();
	int timeBin = int(chan->GetTime() / logicPlotResolution);
	int startTimeBin = 0;
        
	if (!isnan(lastStartTime.at(loc))) {
            startTimeBin = int(lastStartTime.at(loc) / logicPlotResolution);
            if (firstTimeBin == -1) {
                firstTimeBin = startTimeBin;
            }
	} else if (firstTimeBin == -1) {
	    firstTimeBin = startTimeBin;
	}
	startTimeBin = max(0, startTimeBin - firstTimeBin);
	timeBin -= firstTimeBin;
        
	for (int bin=startTimeBin; bin < timeBin; bin++) {
            int row = bin / plotSize;
            int col = bin % plotSize;
            plot(DD_RUNTIME_LOGIC, col, row, loc + 1); // add one since first logic location might be 0
            plot(DD_RUNTIME_LOGIC + loc, col, row, 1);
	}
    }
    for (vector<ChanEvent*>::const_iterator it = triggers.begin(); it != triggers.end(); it++) {
        int timeBin = int((*it)->GetTime() / logicPlotResolution);
        timeBin -= firstTimeBin;
        PackRoot((*it)->GetEnergy());
        if (timeBin >= maxBin || timeBin < 0)
            continue;
        
        int row = timeBin / plotSize;
        int col = timeBin % plotSize;
        
        std::cout << "here2\n";
        plot(DD_RUNTIME_LOGIC, col, row, 20);
        for (int i=1; i < MAX_LOGIC; i++) {
            plot(DD_RUNTIME_LOGIC + i, col, row, 5);
        }
    }
}

// Initialize for root output
bool LogicProcessor::InitRoot(){
	std::cout << " LogicProcessor: Initializing\n";
	if(outputInit){
		std::cout << " LogicProcessor: Warning! Output already initialized\n";
		return false;
	}
	
	// Create the branch
	local_tree = new TTree(name.c_str(),name.c_str());
	//local_branch = local_tree->Branch("Logic", &structure, "tdiff/D:location/i:start/O");
	local_branch = local_tree->Branch("Runtime", &structure, "energy/D");
	outputInit = true;
	return true;
}

// Fill the root variables with processed data
bool LogicProcessor::PackRoot(double energy_){
	if(!outputInit){ return false; }
	structure.energy = energy_;

        local_tree->Fill();
        return true;
}
/*bool LogicProcessor::PackRoot(double tdiff_, unsigned int location_, bool is_start_){
	if(!outputInit){ return false; }
	structure.tdiff = tdiff_;
	structure.location = location_;
	structure.is_start = is_start_;

        local_tree->Fill();
        return true;
}*/

// Write the local tree to file
// Should only be called once per execution
bool LogicProcessor::WriteRoot(TFile* masterFile){
	if(!masterFile || !local_tree){ return false; }
	masterFile->cd();
	local_tree->Write();
	std::cout << local_tree->GetEntries() << " entries\n";
	return true;
}

bool LogicProcessor::InitDamm(){
	return false;
}

bool LogicProcessor::PackDamm(){
	return false;
}
