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
UINT32 firstBblId[PIN_MAX_THREADS]; 
   // Will be updated to hold the id of the first BBL executed
BBPROFILE::BBPROFILE() // UTIL
{
    Pid = 0;
    maxBblCount = 0;
}

INT32 BBPROFILE::Usage() // UTIL
{
    cerr <<
        "This tool collects frequency vectors  for SimPoint.\n"
        "\n";
    return -1;
}

VOID BBPROFILE::EmitSliceStartInfo(UINT32 endMarker, INT64 markerCount, THREADID tid)  //ANALYSIS
{
        BLOCK *block = & block_array[endMarker];
        profiles[tid]->BbFile << "S: " << endMarker << " " <<
            dec << markerCount << " " <<  block->FnName()<< ":" 
            << block->BbName() <<" " <<  block->SourceInfo() << "\n";
}
    
    
VOID BBPROFILE::EmitSliceEnd(UINT32 endMarker, THREADID tid) //ANALYSIS
{
    INT64 markerCount = 0;

    if (profiles[tid]->first == true)
    {
        BLOCK *first_block = & block_array[firstBblId[tid]];
        // Input merging will change the name of the input
        profiles[tid]->BbFile << "I: 0" << endl;
        profiles[tid]->BbFile << "P: " << dec << tid << endl;
        profiles[tid]->BbFile << "C: sum:dummy Command:" 
            << commandLine << endl;
        profiles[tid]->BbFile << "B: " << dec << first_block->_id << " " << first_block->_fnName << ":" << first_block->_bbName  << " "  << first_block->_sourceInfo << endl;
    }
        
    profiles[tid]->BbFile << "# Slice ending at " << dec 
        << profiles[tid]->GlobalInstructionCount << endl;
    
    //if ( !profiles[tid]->first) FIXME
        profiles[tid]->BbFile << "T" ;

    for (UINT32 bbid = 1; bbid <= maxBblCount; bbid++)
    {
        BLOCK *block = & block_array[bbid];
        if (bbid == endMarker ) markerCount += block->GlobalBlockCount(tid);
        
        // if ( !profiles[tid]->first) FIXME
            block->EmitSliceEnd(tid, profiles[tid]);
    }

    // if ( !profiles[tid]->first) FIXME
        profiles[tid]->BbFile << endl;

    if ( profiles[tid]->active  )
    {
#if 0 // FIXME
        if (KnobNoSymbolic)
        {
            profiles[tid]->BbFile << "M: " << hex << endMarker 
                << " " << dec << markerCount << endl;
        }
        else
#endif
        {
            EmitSliceStartInfo(endMarker, markerCount, tid);
        }
    }

    profiles[tid]->BbFile.flush(); 
    profiles[tid]->first = false;            
}
    
BOOL BBPROFILE::DoInsertGetFirstIpInstrumentation()// UTIL
{
    UINT32 i;
    BOOL do_instrument = false;
    
    for ( i = 0; i < PIN_MAX_THREADS; i++ )
    {
        //cerr << " " << profiles[i]->active;
        if ( profiles[i]->active )
        {
            do_instrument |= !profiles[i]->first_eip;
            //cerr << ":" << !profiles[i]->first_eip;
        }
    }
    //cerr << " -> " << do_instrument << endl;    
    return do_instrument;
}
    
    
#if 0
static VOID BBPROFILE::ThreadFini(UINT32 tid, const CONTEXT *ctxt, INT32 code, VOID *v) //ANALYSIS
{
    BBPROFILE * bbprofile = reinterpret_cast<BBPROFILE *>(v);
        
    if ( bbprofile->KnobEmitLastSlice &&
        bbprofile->profiles[tid]->SliceTimer != 
            bbprofile->profiles[tid]->CurrentSliceSize )
    {
        bbprofile->CountBlock_Then(bbprofile->profiles[tid]->last_block,
             tid, bbprofile);
    }
    bbprofile->profiles[tid]->active = false;    
    bbprofile->EmitProgramEnd(tid, bbprofile);
    bbprofile->profiles[tid]->BbFile << "End of bb" << endl;
    bbprofile->profiles[tid]->BbFile.close();
}
    
    
VOID BBPROFILE::GetCommand(int argc, char *argv[])// UTIL
{
    for (INT32 i = 0; i < argc; i++)
    {
        commandLine += " ";
        commandLine += argv[i];
    }
}
#endif

    


VOID BLOCK::Execute(THREADID tid, const BLOCK* prev_block, BBPROFILE *bbprofile) //ANALYSIS
{
    _sliceBlockCount[tid]++;

    // Keep track of previous blocks and their counts only if we 
    // will be outputting them later.
}

VOID BLOCK::EmitSliceEnd(THREADID tid, PROFILE *profile) //ANALYSIS
{
    if (_sliceBlockCount[tid] == 0)
        return;
    
    profile->BbFile << ":" << dec << Id() << ":" << dec 
        << SliceInstructionCount(tid) << " ";
    _globalBlockCount[tid] += _sliceBlockCount[tid];
    _sliceBlockCount[tid] = 0;
}


/* ===================================================================== */
BLOCK::BLOCK()
{
    _staticInstructionCount=0;
    _id=0;
    for (THREADID tid = 0; tid < PIN_MAX_THREADS; tid++)
    {
        _sliceBlockCount[tid] = 0;
        _globalBlockCount[tid] = 0;
    }
}

VOID BLOCK::EmitProgramEnd(THREADID tid, PROFILE *profile) const //ANALYSIS
{
    if (_globalBlockCount[tid] == 0)
        return;

    profile->BbFile << "Block id: " << dec << _id << " " << hex 
        << _fnName << ":" << _bbName << " " << _sourceInfo << dec
        << " static-instructions: " << _staticInstructionCount
        << " block-count: " << _globalBlockCount[tid] << "\n";
}

PROFILE::PROFILE(INT64 slice_size)// UTIL
{
    first = true;
    active = false;
    first_eip = 0;
    GlobalInstructionCount = 0;
    SliceTimer = slice_size; 
    CurrentSliceSize = slice_size;
    last_block = NULL;
}

PROFILE::PROFILE()// UTIL
{
    first = true;
    active = false;
    first_eip = 0;
    GlobalInstructionCount = 0;
    SliceTimer = GLOBAL_SLICE_SIZE;
    CurrentSliceSize = GLOBAL_SLICE_SIZE;
    last_block = NULL;
}

//VOID BBPROFILE::activate(int argc, char** argv) // TODO
VOID BBPROFILE::Activate(INT64 inSliceSize, string outputFile)
{
    sliceSize = inSliceSize;
    outputFileName = outputFile;
    InitProfile(sliceSize);
}

VOID BBPROFILE::InitProfile(INT64 sliceSize)
{
    //GetCommand(argc, argv);
        
    //maxThread = MaxThreadsKnob.ValueInt64();
        
#if 0 // FIXME
    if (KnobPid)
    {
        Pid = getpid();
    }
#endif
        
    for (THREADID tid = 0; tid < PIN_MAX_THREADS; tid++)
    {
        profiles[tid] = new PROFILE(sliceSize);
    }
}

VOID PROFILE::OpenFile(THREADID tid, UINT32 pid, string output_file)// UTIL
{
    if ( !BbFile.is_open() )
    {
        char num[100];
        if (pid)
        {
            sprintf(num, ".T.%u.%d", (unsigned)pid, (int)tid);
        }
        else
        {
            sprintf(num, ".T.%d", (int)tid);
        }
        string tname = num;
        BbFile.open((output_file+tname+".bb").c_str());
        BbFile.setf(ios::showbase);
    }
}
