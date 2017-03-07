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
#include <set>
#include <map>
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/Debug.h"
#include "roidefs.H"
#include "ROIinstrument.H"
#include "ROIanalysis.H"
extern set<UINT32> ROIbbset;
extern struct ROIinfo ROIinfoArray[]; 
extern struct ROIbbinfo ROIbbinfoArray[];
extern map<UINT32,UINT32> bbinfoIndex;
extern UINT32 ROIcount;
extern UINT32 ROIbbcount;

using namespace llvm;
Function *ROIBlockFunc;
Function *ROIProgramEndFunc;

VOID ProcessLLVMBB(llvm::BasicBlock *bbl, UINT32 bblId)
{
    if(ROIbbset.count(bblId) == 0) return; 
    UINT32 bblIndex = bbinfoIndex[bblId];
    string fnName =  bbl->getParent()->getName().str();
    string bbName =   bbl->getName().str();
    cerr << "fnName " << fnName << endl;
    cerr << "bbName " << bbName << endl;
    ROIbbinfoArray[bblIndex].bbDesc =  fnName + ":" + bbName;
    Instruction *I = bbl->getTerminator();
    if(I)
    {
        std::vector<Value *> funcArgs;
        funcArgs.clear();
   
        funcArgs.push_back(ConstantInt::get(Type::getInt32Ty(bbl->getParent()->getParent()->getContext()), bblIndex)); //bblIndex

        CallInst::Create(ROIBlockFunc, funcArgs, llvm::Twine("ROIBlock"), I);
    }
}
