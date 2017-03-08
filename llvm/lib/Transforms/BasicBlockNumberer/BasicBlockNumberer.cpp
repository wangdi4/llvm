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
//===- BasicBlockNumberer.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "BasicBlockNumberer.H"

static UINT32 bblCount = 1; // SimPoint expects bbs to be numbered starting @1
cl::opt<string> KnobOutputFile( "bbn:o", cl::desc("specify bbinfo file name"),
             cl::Hidden, cl::init(""));

//STATISTIC(HelloCounter, "Counts number of functions greeted");


bool BasicBlockNumberer::runOnModuleBegin() {
      if(!KnobOutputFile.empty()) bbNumberFile.open(KnobOutputFile.c_str());
      return false; // no modifications to the IR was made
}

void BasicBlockNumberer::runOnModuleEnd() {
      if(!KnobOutputFile.empty()) 
      {
        bbNumberFile.close();
      }
}

bool BasicBlockNumberer::runOnBasicBlock(BasicBlock &BB) 
{
    stringstream ss;

    ss << dec << bblCount << ":";
    Function * parent = BB.getParent();
    if(BB.hasName())
    {
        BB.setName(ss.str()+BB.getName());
    }
    else
    {
        BB.setName(ss.str()+"NoName");
    }
    if(bbNumberFile.is_open())
    {
        bbNumberFile << "Basic Block #: " << bblCount << " " << BB.getName().str() << ":" << parent->getName().str() << "\n";
    }
    bblCount++;
    return true;
}

bool BasicBlockNumberer::runOnModule(Module &M) {
    mod = &M;
    bool modified = false;
    runOnModuleBegin();

    for(Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
        for(Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB) {
            modified |= runOnBasicBlock(*BB);
        }
    }
    runOnModuleEnd();
    return modified;
}

char BasicBlockNumberer::ID = 0;
static RegisterPass<BasicBlockNumberer> X("bbnumber", "Basic Block Numbering Pass");
