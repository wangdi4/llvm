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
//===- LLVMROIController.cpp Pass" ---------------===//
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "LLVMControlManager.H"
#include "LLVMROIController.H"
#include "ROIinstrument.H"
#include "roidefs.H"

using namespace CONTROLLER;
CONTROL_MANAGER *control_manager;
set<UINT32> ROIbbset;
struct ROIinfo ROIinfoArray[MAX_ROICOUNT+1]; // Indexing starts at 1
struct ROIbbinfo ROIbbinfoArray[2*MAX_ROICOUNT+1];  // Indexing starts at 1
   // extreme case all start/end bbids are distinct hence size 2*MAX_ROICOUNT
map<UINT32,UINT32> bbinfoIndex;
UINT32 ROIcount; // number of regions, assumed contiguous starting from 1
UINT32 ROIbbcount; // number of unique bbs that start/stop ROIs
UINT32 firstBbId;
string firstBbDesc;

bool LLVMROIController::runOnModuleBegin() {
      control_manager = new CONTROL_MANAGER();
      control_manager->Activate();
      std::vector<Type *> paramType;
      paramType.clear();

      paramType.push_back(Type::getInt32Ty(mod->getContext()));// UINT32 bbindex

      FunctionType *ROIBlock_type = FunctionType::get(Type::getInt32Ty(mod->getContext()), makeArrayRef(paramType), false);
      ROIBlockFunc = cast<Function>(mod->getOrInsertFunction("ROIBlock", ROIBlock_type));

      FunctionType *ROIProgramEnd_type = FunctionType::get(Type::getInt32Ty(mod->getContext()), makeArrayRef(paramType), false);
      ROIProgramEndFunc = cast<Function>(mod->getOrInsertFunction("ROIProgramEnd", ROIProgramEnd_type));

      return false; // no modifications to the IR was made
}

bool LLVMROIController::runOnModuleEnd() {
    std::ofstream ofs("roiinfo.txt");
    boost::archive::text_oarchive oa(ofs);
    oa << ROIcount;
    oa << ROIinfoArray;
    oa << ROIbbcount;
    oa << ROIbbinfoArray;
    oa << firstBbId;
    oa << firstBbDesc;
    return false;
}

bool LLVMROIController::runOnBasicBlock(BasicBlock &BB) 
{
    UINT32 bblId = 0;
    string bbname = BB.getName();
    istringstream iss(bbname);
    string tok;
    if ( !getline(iss, tok, ':'))
    {
        errs() << " Basic block number not found. Run BasicBlockNumberer pass first." << "\n";
        exit(1);
    } 
    bblId = atoi(tok.c_str());
    if ( bblId == 0 )
    {
        //Assumption: bb numbering starts at 1
        errs() << " Basic block number not found. Run BasicBlockNumberer pass first." << "\n";
        exit(1);
    }
    ProcessLLVMBB(&BB, bblId);
    return true;
}

bool LLVMROIController::runOnModule(Module &M) {
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

void LLVMROIController::getAnalysisUsage(AnalysisUsage &AU) const{
    AU.setPreservesCFG(); // Adds calls but they do not break BBs
    //AU.addRequired<BasicBlockNumberer>();
}

char LLVMROIController::ID = 0;
static RegisterPass<LLVMROIController> X("roi", "Region of interest (ROI) instrumentation Pass");
