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
#include "bbutils.H"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/Function.h"
extern llvm::cl::opt<string> KnobOutputFile;
extern llvm::cl::opt<INT64>  KnobSliceSize;
extern llvm::cl::opt<BOOL>  KnobNoSymbolic;
extern llvm::cl::opt<BOOL>  KnobEmitFirstSlice;
extern llvm::cl::opt<BOOL>  KnobEmitLastSlice;
extern llvm::cl::opt<BOOL>  KnobPid;


// Create a new one and return it if it doesn't already exist.
BLOCK * BBPROFILE::LookupBlock(void *vbbl, UINT32 bblId)// UTIL
{
    llvm::BasicBlock * bbl = reinterpret_cast<llvm::BasicBlock *>(vbbl);
    ASSERTX(bblId < MAX_BBLCOUNT);
    string bbName = bbl->getName().str();
    string fnName =  bbl->getParent()->getName().str();
    BLOCK * block = &block_array[bblId];
    if((block->FnName() == fnName ) && (block->BbName() == bbName))
        return block;

    block->_staticInstructionCount = bbl->size();
    block->_id = bblId;
    block->_fnName = fnName;
    block->_bbName = bbName;
    if(bblId > maxBblCount) maxBblCount = bblId;

    return block;
}


// Find a BLOCK for a given LLVM basic block
BLOCK * BBPROFILE::FindBlock(void *vbbl)// UTIL
{
    llvm::BasicBlock * bbl = reinterpret_cast<llvm::BasicBlock *>(vbbl);
    string bbName = bbl->getName().str();
    string fnName =  bbl->getParent()->getName().str();
    for (UINT32 bbid = 0; bbid <= maxBblCount; bbid++)
    {
        BLOCK * block = & block_array[bbid];
        if((block->FnName() == fnName ) && (block->BbName() == bbName))
            return block;
    }
    return NULL;
}

BOOL BBPROFILE::ParseFilenameTid(const string& str, string *fn, UINT32 *tidp)// UTIL
{
    size_t tidpos = str.find(":tid");
    if(tidpos == string::npos) return FALSE;
    string tidstr = str.substr(tidpos+4);
    *fn=str.substr(0, tidpos);
    *tidp = atoi(tidstr.c_str());
    return TRUE;
}

