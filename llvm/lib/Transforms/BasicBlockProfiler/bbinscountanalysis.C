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
