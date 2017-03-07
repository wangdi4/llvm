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
#include "roidefs.H"
#include "ROIanalysis.H"

bool init_done = false;
struct ROIinfo ROIinfoArray[MAX_ROICOUNT+1]; // Indexing starts at 1
struct ROIbbinfo ROIbbinfoArray[2*MAX_ROICOUNT+1];  // Indexing starts at 1
   // extreme case all start/end bbids are distinct hence size 2*MAX_ROICOUNT
UINT32 ROIcount; // number of regions, assumed contiguous starting from 1
UINT32 ROIbbcount; // number of unique bbs that start/stop ROIs
UINT32 firstBbId;
string firstBbDesc;

extern void ROIHandler (unsigned int regionNumber, unsigned int bbId,
    uint64_t bbCount, double weight, string bbDesc, bool isStarting );
void        ROIProgramStart(unsigned int firstBbId, string firstBbDesc);

static VOID ThreadStart(THREADID tid)
{
    ASSERTX(tid < MAX_THREADS);
    atexit(ROIProgramEnd);
}

extern "C" {

INT32 ROIBlock(UINT32 bbIndex) 
{
    if(!init_done)
    {
        std::ifstream ifs("roiinfo.txt");
        boost::archive::text_iarchive ia(ifs);
        ia >> ROIcount;
        ia >> ROIinfoArray;
        ia >> ROIbbcount;
        ia >> ROIbbinfoArray;
        ia >> firstBbId;
        ia >> firstBbDesc;
        ROIProgramStart(firstBbId, firstBbDesc);
        ThreadStart(0);
        init_done = true;
    }
    ROIbbinfoArray[bbIndex].execcount++;

    //Check for ROI end first as another ROI may start here
    if(ROIbbinfoArray[bbIndex].nextEndingROI)
    {
        UINT32 endingROI = ROIbbinfoArray[bbIndex].nextEndingROI;
        if (ROIbbinfoArray[bbIndex].execcount == 
                ROIinfoArray[endingROI].endBbCount)
        {
            UINT32 bbid = ROIbbinfoArray[bbIndex].bbid;
            ROIHandler(endingROI, bbid, ROIbbinfoArray[bbIndex].execcount, 
                ROIinfoArray[endingROI].weight,ROIbbinfoArray[bbIndex].bbDesc,
                false);
            //update nextEndingROI
            UINT32 nextROI = 0;
            for(UINT32 roi=1; roi <=ROIcount; roi++)
            {
            // look for the roi with smallest endBbCount greater than execcount
                if(ROIinfoArray[roi].endBbid == bbid)
                {
                    if(ROIinfoArray[roi].endBbCount > ROIbbinfoArray[bbIndex].execcount)
                    {
                    // endBbCount greater than execcount
                        if(nextROI == 0) // first candidate roi
                            nextROI = roi;
                        else if (ROIinfoArray[roi].endBbCount < ROIinfoArray[nextROI].endBbCount)
                        {
                            // this roi endBbCount is closer to execcount
                            nextROI = roi;
                        }
                    }
                }
            }
            ROIbbinfoArray[bbIndex].nextEndingROI = nextROI;
        }
    }
    if(ROIbbinfoArray[bbIndex].nextStartingROI)
    {
        UINT32 startingROI = ROIbbinfoArray[bbIndex].nextStartingROI;
        if (ROIbbinfoArray[bbIndex].execcount == ROIinfoArray[startingROI].startBbCount)
        {
            //update nextStartingROI
            UINT32 bbid = ROIbbinfoArray[bbIndex].bbid;
            ROIHandler(startingROI, bbid, ROIbbinfoArray[bbIndex].execcount, 
                ROIinfoArray[startingROI].weight,ROIbbinfoArray[bbIndex].bbDesc,
                true);
            UINT32 nextROI = 0;
            for(UINT32 roi=1; roi <=ROIcount; roi++)
            {
            // look for the roi with smallest startBbCount greater than execcount
                if(ROIinfoArray[roi].startBbid == bbid)
                {
                    if(ROIinfoArray[roi].startBbCount > ROIbbinfoArray[bbIndex].execcount)
                    {
                    // startBbCount greater than execcount
                        if(nextROI == 0) // first candidate roi
                            nextROI = roi;
                        else if (ROIinfoArray[roi].startBbCount < ROIinfoArray[nextROI].startBbCount)
                        {
                            // this roi startBbCount is closer to execcount
                            nextROI = roi;
                        }
                    }
                }
            }
            ROIbbinfoArray[bbIndex].nextStartingROI = nextROI;
        }
    }
#if 0
    cerr << "PGM:ROIBlock bbIndex " << bbIndex << "\n";
#endif
    return 0;
}

VOID ROIProgramEnd() //ANALYSIS
{
    THREADID tid = 0;
    for(UINT32 i=1; i<=ROIbbcount; i++)
    {
        cerr << " bbindex " << i;
        cerr << " bbid " << ROIbbinfoArray[i].bbid;
        cerr << " execcount " << ROIbbinfoArray[i].execcount;
        cerr << endl;
    }
    return;
}
}
