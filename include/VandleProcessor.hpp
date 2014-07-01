/** \file VandleProcessor.hpp
 * \brief Class for handling Vandle Bars
 */

#ifndef __VANDLEPROCESSOR_HPP_
#define __VANDLEPROCESSOR_HPP_

#include "EventProcessor.hpp"

class TFile;

struct VandleDataStructure{
    double tof, lqdc, rqdc, tsLow, tsHigh;
    double lMaxVal, rMaxVal, qdc, energy;
    unsigned int multiplicity, location; 
};

class VandleProcessor : public EventProcessor{
 public:
    VandleProcessor(); // no virtual c'tors
    VandleProcessor(const int VML_OFFSET, const int RANGE);
    VandleProcessor(const int RP_OFFSET, const int RANGE, int i);
    virtual void DeclarePlots(void);
    virtual bool Process(RawEvent &event);
    virtual bool InitRoot();
    virtual bool WriteRoot(TFile*);
    bool PackRoot(unsigned int, const vmlData*, unsigned int);
    bool InitDamm();
    bool PackDamm();
        
    VMLMap vmlMap; 
    VandleDataStructure structure;

 protected:
    //define the maps
    BarMap barMap;
    TimingDataMap bigMap;
    TimingDataMap smallMap;
    TimingDataMap startMap;
    TimingDataMap tvandleMap;
 
 private:
    virtual bool RetrieveData(RawEvent &event);
    virtual void AnalyzeData(RawEvent& rawev);
    virtual void ClearMaps(void);
    virtual void CrossTalk(void);
    virtual void Tvandle(void);
    virtual void BuildBars(const TimingDataMap &endMap, const std::string &type, BarMap &barMap);    
    virtual void FillMap(const vector<ChanEvent*> &eventList, const std::string type, TimingDataMap &eventMap);    
    virtual void WalkBetaVandle(const TimingInformation::TimingDataMap &beta, const TimingInformation::BarData &bar);
    virtual double CorrectTOF(const double &TOF, const double &corRadius, const double &z0) {return((z0/corRadius)*TOF);};

    bool hasDecay;
    double decayTime;
    int counter;

    typedef std::pair<unsigned int, unsigned int> CrossTalkKey; 
    typedef std::map<CrossTalkKey, double> CrossTalkMap;
    std::map<CrossTalkKey, double> crossTalk;
}; //Class VandleProcessor

#endif // __VANDLEPROCESSOR_HPP_