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
//===- BasicBlockProfiler.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "BasicBlockProfiler.H"
#include "bbutils.H"
#include "bbinstrument.H"
#include "bbanalysis.H"
//#include "BasicBlockNumberer.H"

BBPROFILE globalBbProfile;
cl::opt<string> KnobOutputFile( "bbp:o", cl::desc("specify bb file name"),
             cl::Hidden, cl::init("bbv.out")); 
cl::opt<UINT32>  KnobSliceSize( "bbp:slice_size", 
            cl::desc("slice size in instructions"),cl::Hidden,
            cl::init(100)); 
cl::opt<BOOL>  KnobNoSymbolic( "bbp:nosymbolic",
            cl::desc("Do not emit symbolic information for markers"),
             cl::Hidden,cl::init( 0));
cl::opt<BOOL>  KnobEmitVectors( "bbp:emit_vectors",
            cl::desc("Emit frequency (bb/reuse-dist) vectors at the end of each slice."),
              cl::Hidden,cl::init(1));
cl::opt<BOOL>  KnobEmitFirstSlice( "bbp:emit_first", 
            cl::desc("Emit the first interval (higher overhead to find out first IP)"),
             cl::Hidden,cl::init(1));
cl::opt<BOOL>  KnobEmitLastSlice( "bbp:emit_last", 
            cl::desc("Emit the last interval even if it is less than slice_size"),
            cl::Hidden,cl::init(1));



//STATISTIC(HelloCounter, "Counts number of functions greeted");


bool BasicBlockProfiler::runOnModuleBegin() {
      globalBbProfile.Activate(KnobSliceSize,KnobOutputFile); 
      std::vector<Type *> paramType;
      paramType.clear();

      paramType.push_back(Type::getInt32Ty(mod->getContext()));// UINT32 bblid

      FunctionType *countBlock_type = FunctionType::get(Type::getInt32Ty(mod->getContext()), makeArrayRef(paramType), false);
      countBlockFunc = cast<Function>(mod->getOrInsertFunction("CountBlock", countBlock_type));

      FunctionType *emitProgramEnd_type = FunctionType::get(Type::getInt32Ty(mod->getContext()), makeArrayRef(paramType), false);
      emitProgramEndFunc = cast<Function>(mod->getOrInsertFunction("EmitProgramEnd", emitProgramEnd_type));

      return false; // no modifications to the IR was made
}

bool BasicBlockProfiler::runOnModuleEnd() {
    std::ofstream ofs("bbprofile.txt");
    boost::archive::text_oarchive oa(ofs);
    oa << globalBbProfile;
    cerr << "COMPILER:maxBblCount " << globalBbProfile.maxBblCount << "\n";
#if 0
    for (UINT32 bbid = 1; bbid <= globalBbProfile.maxBblCount; bbid++)
    {
        BLOCK *block = & globalBbProfile.block_array[bbid];
        errs() << "COMPILER:bbid " << block->Id() << " " << block->FnName()<< ":" << block->BbName() << " inscount " << block->StaticInstructionCount(0) << "\n";
            
    }
#endif
    return false;
}

bool BasicBlockProfiler::runOnBasicBlock(BasicBlock &BB) 
{
    UINT32 bblCount = 0;
    string bbname = BB.getName();
    istringstream iss(bbname);
    string tok;
    if ( !getline(iss, tok, ':'))
    {
        errs() << " Basic block number not found. Run BasicBlockNumberer pass first." << "\n";
        exit(1);
    } 
    bblCount = atoi(tok.c_str());
    if ( bblCount == 0 )
    {
        errs() << " Basic block number not found. Run BasicBlockNumberer pass first." << "\n";
        exit(1);
    }
#if 0
    Function * parent = BB.getParent();
    errs() << "Basic Block #: " << bblCount << " parent function " << parent->getName() << "\n";
#endif
    ProcessLLVMBB(&BB, &globalBbProfile, bblCount);
    return true;
}

void BasicBlockProfiler::scan_for_indirect_and_pthread_call(Function *func, CallGraphNode *CGN)
{
    for (auto it = CGN->begin(), et = CGN->end(); it != et; it++) {
        CallGraphNode *calledGraphNode = it->second;
        if(!calledGraphNode->getFunction()) {
            cerr << " Indirect call seen in " << func->getName().str() << "\n";
            continue;
        }
        if(calledGraphNode->getFunction()->getName().find("pthread_create")
                !=std::string::npos)
        {
            std::cerr << "WARNING: call to pthread_create() found in " << CGN->getFunction()->getName().str() << "\n";
        }
    }
}

bool BasicBlockProfiler::runOnModule(Module &M) {
    mod = &M;
    bool modified = false;
    
    callGraph = &getAnalysis<CallGraphWrapperPass>().getCallGraph();
    for (auto &F : M) {
        if (!F.isDeclaration()) {
            scan_for_indirect_and_pthread_call(&F,
                 callGraph->getOrInsertFunction(&F));
       }
    }
    runOnModuleBegin();

    for(Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
        for(Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB) {
            modified |= runOnBasicBlock(*BB);
        }
    }
    runOnModuleEnd();
    return modified;
}

void BasicBlockProfiler::getAnalysisUsage(AnalysisUsage &AU) const{
    AU.setPreservesCFG(); // Adds calls but they do not break BBs
    AU.addRequired<CallGraphWrapperPass>();
    //AU.addRequired<BasicBlockNumberer>();
}

#if 0
bool BasicBlockProfiler::runOnMain(Function &F) 
{
    errs() << "COMPILER: Function " << F.getName() << "\n";
    if(F.getName()=="main")
        instrumentProgramEnd(F.end(), &globalBbProfile);
}
#endif

char BasicBlockProfiler::ID = 0;
static RegisterPass<BasicBlockProfiler> X("bbprofile", "Basic Block Profiling Pass");
