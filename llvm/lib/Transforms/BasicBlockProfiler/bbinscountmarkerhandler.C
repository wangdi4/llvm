#include <iostream>
#include <stdlib.h>
#include "bbutils.H"
#include "bbanalysis.H"

BBPROFILE *globalBbProfile;
bool init_done = false;
extern UINT32 firstBblId[]; 
   // Will be updated to hold the id of the first BBL executed

static VOID ThreadStart(THREADID tid)
{
    ASSERTX(tid < PIN_MAX_THREADS);
    atexit(EmitProgramEnd);
    globalBbProfile->profiles[tid]->OpenFile(tid, globalBbProfile->Pid,
                 globalBbProfile->outputFileName.c_str());
    globalBbProfile->profiles[tid]->active = true;
}

extern "C" {
INT32 CountBlock(UINT32 bblid) 
{
    if(!init_done)
    {
        firstBblId[0] = bblid; // First BBL executed
        globalBbProfile = new BBPROFILE;
        std::ifstream ifs("bbprofile.txt");
        boost::archive::text_iarchive ia(ifs);
        ia >> *globalBbProfile;
        ThreadStart(0);
        init_done = true;
    }
    BLOCK * block = &globalBbProfile->block_array[bblid];
    block->Execute(0);
    globalBbProfile->profiles[0]->GlobalInstructionCount 
            += block->StaticInstructionCount(0);
    return 0;
}

VOID EmitProgramEnd() //ANALYSIS
{
    THREADID tid = 0;
    globalBbProfile->profiles[tid]->BbFile << "Dynamic instruction count "
         << dec << globalBbProfile->profiles[tid]->GlobalInstructionCount << endl;
    globalBbProfile->profiles[tid]->BbFile.close();
    return;
}
}

using namespace std;
static int regionNumber = 0;
static bool inRegion = false;
static INT64 startIcount = 0;
static INT64 stopIcount = 0;
void ROIMarkerHandler (const char * fnName, bool isStarting )
{
    THREADID tid = 0;
    if(isStarting)
    {
        if(inRegion){
          cerr << "\t **** Already in region " << regionNumber << ". IGNORING START\n";
        }
        else {
          regionNumber++;
          inRegion = true;
          startIcount = globalBbProfile->profiles[tid]->GlobalInstructionCount; 
          cerr << "\t **** Starting ROI " << regionNumber << " function " << fnName << " startIcount " << startIcount <<  endl; 
        }
    }
    else
    {
        if(!inRegion){
          cerr << "\t **** NOT in a region. IGNORING STOP\n";
        }
        else {
          stopIcount = globalBbProfile->profiles[tid]->GlobalInstructionCount; 
          cerr << "\t **** Ending ROI " << regionNumber << " function " << fnName << " stopIcount " << stopIcount  << " region:Icount " << stopIcount-startIcount <<  endl; 
          inRegion = false;
        }
    }
}
