/** \file Initialize.cpp
 * \brief C++ wrapper to upak's hd1d/hd2d functions
 * \author David Miller
 * \date Aug. 2009
 */

#include <iostream>

#include "DetectorDriver.hpp"
#include "MapFile.hpp"
//#include "PathHolder.hpp"

#ifdef USE_HHIRF
// DAMM initialization call
extern "C" void drrmake_();
// DAMM declaration wrap-up call
extern "C" void endrr_();
#endif

/*! This function defines the histograms to be used in the analysis */
extern "C" void drrsub_(unsigned int& iexist)
{
    //PathHolder* conf_path = new PathHolder("setup.cfg");
    //delete conf_path;
    MapFile theMapFile = MapFile();
#ifdef USE_HHIRF
    drrmake_();
    DetectorDriver::get()->DeclarePlots(theMapFile);
    endrr_(); 
#endif
}
