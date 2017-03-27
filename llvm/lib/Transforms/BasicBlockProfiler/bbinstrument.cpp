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
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/DebugInfo.h"
#include "bbinstrument.H"
#include "bbanalysis.H"
#define DEBUG_TYPE "bbprofile-debug"

using namespace llvm;
Function *countBlockFunc;
Function *emitProgramEndFunc;

VOID ProcessLLVMBB(llvm::BasicBlock *bbl, BBPROFILE * bbprofile, UINT32 bblId)
{
    // find the block in the map or add it if new.
    BLOCK * block = bbprofile->LookupBlock((void *)bbl, bblId);
    
    //Instruction *I = bbl->end();
    //Instruction *I = bbl->getFirstNonPHI();
    Instruction *I = bbl->getTerminator();
    stringstream ss;

    const DebugLoc& DL = I->getDebugLoc();
    if (! DL) {
        ss << "nofile:0"; 
    } else {
        unsigned Lin = DL.getLine();
        DIScope *Scope = cast<DIScope>(DL.getScope());
        StringRef File = Scope->getFilename();
        ss << File.str() << ":" << dec << Lin;
    }

    block->_sourceInfo = ss.str();
    //IRBuilder<> builder(I);
    if(I)
    {
        std::vector<Value *> funcArgs;
        funcArgs.clear();
   
        funcArgs.push_back(ConstantInt::get(Type::getInt32Ty(bbl->getParent()->getParent()->getContext()), bblId)); //bblid

        CallInst::Create(countBlockFunc, funcArgs, llvm::Twine("CountBlock"), I);
    }
}
