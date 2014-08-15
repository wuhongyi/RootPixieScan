/** \file LogicProcessor.hpp
 * \brief Class to handle logic signals 
 * derived originally from MTC processor
 */

#ifndef __LOGICPROCESSOR_HPP_
#define __LOGICPROCESSOR_HPP_

#include <vector>

#include "EventProcessor.hpp"
#include "Structures.h"

class LogicProcessor : public EventProcessor {
private:
    bool BasicProcessing(RawEvent &event);
    bool TriggerProcessing(RawEvent &event);
    bool save_waveforms;
    int plotSize;
    
protected:
    std::vector<double> lastStartTime; //< time of last leading edge
    std::vector<double> lastStopTime;  //< time of last trailing edge
    std::vector<bool> logicStatus;   //< current level of the logic signal

    std::vector<unsigned long> stopCount;  //< number of stops received
    std::vector<unsigned long> startCount; //< number of starts received
    
 public:
    LogicProcessor();
    LogicProcessor(bool);
    virtual bool InitDamm();
    virtual bool InitRoot(TTree*);
    virtual bool Process(RawEvent &event);
    virtual void Zero(){ structure.Zero(); }
    virtual bool LogicStatus(size_t loc) const {
      return logicStatus.at(loc);
    }
    unsigned long StopCount(size_t loc) const {
	return stopCount.at(loc);
    }
    unsigned long StartCount(size_t loc) const {
	return startCount.at(loc);
    }
    double TimeOff(size_t loc, double t) const {
	return (!LogicStatus(loc) ? (t-lastStopTime.at(loc)) : 0.);
    }
    double TimeOn(size_t loc, double t) const {
	return (LogicStatus(loc) ? (t-lastStartTime.at(loc)) : 0.);
    }

    RuntimeStructure structure;
};

#endif // __LOGICPROCESSOR_HPP_
