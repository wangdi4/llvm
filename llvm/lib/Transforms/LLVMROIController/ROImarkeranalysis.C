/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2016-2017 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
#include <stdlib.h>
#include <map>
#include <list>
#include "roidefs.H"
extern "C" VOID ROIMarkerProgramEnd();

string markerFile = "roimarker.txt";

std::map<int, struct markerInfo *> markerInfoMap;
std::map<int, std::list<struct markerROIInfo *> > markerROIInfoMap;
typedef std::map<int, struct markerInfo *>::iterator mInfoIterator;
typedef std::map<int, std::list<struct markerROIInfo *> >::iterator mROIInfoIterator;

static bool init_done = false;

static UINT32 ROIcount; // number of regions, assumed contiguous starting from 1

extern void ROIMarkerHandler (string funcName, bool isStarting );

static bool compareMRI( const struct markerROIInfo *first, const struct markerROIInfo *second) 
{
  return first->triggerCount < second->triggerCount;
}

static VOID MarkerThreadStart(THREADID tid)
{
    ASSERTX(tid < MAX_THREADS);
    atexit(ROIMarkerProgramEnd);
    ifstream fin;
    fin.open(markerFile.c_str());
    std::string line;
    if (!fin.good()){
      cerr << "\t **** ERROR could not open " << markerFile << endl; 
    }
    const char *delim = " ";
    while (std::getline(fin, line)) {
      std::vector<char> rawLine(line.size()+1);
      strncpy(&rawLine[0], line.c_str(), line.length());
      rawLine[line.length()] = '\0';
      char *token = std::strtok(&rawLine[0], delim);
      if (token != NULL) {
        if (strcmp(token, "start") == 0 ||  (strcmp(token, "stop") == 0)) {
          bool isStart = (std::strcmp(token, "start")==0);
          token = std::strtok(NULL, delim);
          std::string markerString(token);
          int marker = std::strtol(markerString.c_str(), NULL, 0);
          token = std::strtok(NULL, delim);
          std::string countString(token);
          UINT64 count = std::strtol(countString.c_str(), NULL, 0);
          //cerr << (isStart?"start":"stop") 
           //<< "\t **** marker " << marker << " count " << count << endl; 
          struct markerROIInfo *mri = new struct markerROIInfo;
          mri->triggerCount = count;
          mri->isStart = isStart;
          markerROIInfoMap[marker].push_back(mri);
        } else {
           cerr << "\t **** ERROR invalid record " << line << endl; 
        }
      }
    }
    // now sort the list of triggercounts for each marker
    for( mROIInfoIterator it=markerROIInfoMap.begin(); 
        it!=markerROIInfoMap.end(); ++it)
    {
      it->second.sort(compareMRI);
    }
}

extern "C" {

void ROIMarker(int marker, const char * funcName) 
{
    //cerr << "\t **** Marker " << marker << " function " << funcName << endl; 
    if(!init_done)
    {
        // open and process `roimarker.txt'
        MarkerThreadStart(0);
        init_done = true;
    }
    // lookup marker in markerInfoMap; add if absent
    // incrment markerCurrentCount
    mInfoIterator mit = markerInfoMap.find(marker);
    if (mit == markerInfoMap.end()) {
      struct markerInfo *mi = new struct markerInfo;
      markerInfoMap.insert(std::pair<int,struct markerInfo *>(marker,mi));
      mit = markerInfoMap.find(marker);
    }
    mit->second->currentCount++;

    mROIInfoIterator mrit = markerROIInfoMap.find(marker);

    if (mrit != markerROIInfoMap.end()) {
      std::list<struct markerROIInfo *> mriList = mrit->second;
    // if markerCurrentCount matches the triggerCount in the first
    // element of the list for marker in markerROIInfoMap
    //   remove the first element from the list
    //   call ROIMarkerHandler(fn, isStart)
      if(!mriList.empty()){
        if(mit->second->currentCount == mriList.front()->triggerCount){
          cerr << "\t **** ROI " << (mriList.front()->isStart?"start":"stop" ) << endl;
          ROIMarkerHandler(mit->second->funcName, mriList.front()->isStart);
          mrit->second.pop_front();
        }
      }
    }
    return ;
}

VOID ROIMarkerProgramEnd() //ANALYSIS
{
  // print marker trigger stats
  cerr << "\t **** ROIMarkerProgramEnd() " << endl;
  for( mROIInfoIterator it=markerROIInfoMap.begin(); 
        it!=markerROIInfoMap.end(); ++it)
  {
    int marker = it->first;
    for (std::list<struct markerROIInfo *>::iterator mrit=it->second.begin();
      mrit != it->second.end(); ++mrit)
    {
      struct markerROIInfo * mri = *mrit;
      bool isStart = mri->isStart;
      cerr << (isStart?"start":"stop") 
        << "\t **** marker " << marker << " trigger-count " << mri->triggerCount
        << " triggered? " << mri->isTriggered << endl; 
    }
  }
  cerr << std::flush;
  return;
}
}
