//===------------- DTransPaddedMalloc.h - DTransPaddedMalloc  -------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans padded malloc optimization pass.
//
//===----------------------------------------------------------------------===//

#if !INTEL_INCLUDE_DTRANS
#error DTrans.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_PADDEDMALLOC_H
#define INTEL_DTRANS_TRANSFORMS_PADDEDMALLOC_H

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include <vector>

namespace llvm {

namespace dtrans {

/// Pass to perform DTrans padded malloc.
class PaddedMallocPass : public PassInfoMixin<dtrans::PaddedMallocPass> {
public:
  PaddedMallocPass()
      : DTransPaddedMallocVar("PaddedMallocCounter"),
        DTransPaddedMallocFunc("PaddedMallocInterface"){};

  // Lambda function to collect the LoopInfo for a given function
  using LoopInfoFuncType = std::function<LoopInfo &(Function &)>;

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Actual implementation of the optimization
  bool runImpl(Module &M, DTransAnalysisInfo &DTInfo, LoopInfoFuncType &GetLI,
               const TargetLibraryInfo &TLInfo);

private:

  // Name of the global variable used as a counter. The constructor
  // will assign the value "PaddedMallocCounter".
  std::string DTransPaddedMallocVar;

  // Name of the interface generated that identifies if padded
  // malloc can be applied or not
  std::string DTransPaddedMallocFunc;

  // Apply the padded malloc optimization to the functions stored in
  // PaddedMallocFuncs.
  bool applyPaddedMalloc(std::vector<Function *> &PaddedMallocFuncs,
                         GlobalVariable *globCounter, Function *PMFunc,
                         Module *M, const TargetLibraryInfo &TLInfo,
                          DTransAnalysisInfo &DTInfo, bool UseOpenMP);

  // Build a new boolean function in the module M that checks if
  // globCounter has reached the limit or not.
  Function *buildInterfaceFunction(Module *M, GlobalVariable *globCounter);

  // Build the counter used to identify if padded malloc will be applied
  // or not
  GlobalVariable *buildGlobalVariableCounter(Module &M);

  // Return true if the input Instruction will be used in the input BranchInst,
  // else return false.
  bool checkDependence(Instruction *CheckInst, BranchInst *Branch);

  // Return true if the input BasicBlock is a comparison between
  // two pointer/array/vector entries in order to exit a loop.
  // Else, return false.
  bool exitDueToSearch(BasicBlock &BB);

  // Traverse through each field of the structures stored in DTInfo and check
  // if the memory allocation for each field only happens in one function. If
  // so, then collect that function and store it in PaddedMallocFuncs.
  bool findFieldSingleValueFuncs(DTransAnalysisInfo &DTInfo,
                                 std::vector<Function *> &PaddedMallocFuncs);

  // Return true if at least one Function in the input Module has a
  // search loop
  bool findSearchLoops(Module &M, LoopInfoFuncType &GetLI);

  // Return true if there is a search loop in the input Function,
  // else return false.
  bool funcHasSearchLoop(Function &Fn, LoopInfoFuncType &GetLI);

  // Return true if at least one successor of the input BasicBlock will
  // exit the input Loop, else return false.
  bool isExitLoop(Loop *LoopData, BasicBlock *BB);

  // Return true if the input GetElementPtrInst is valid to consider as a
  // an array, vector or a pointer memory space allocated.
  bool isValidType(GetElementPtrInst *ElemInst);

  // Return true if the padded malloc was applied in the input BasicBlock
  // that corresponds to the input Function. Else return false.
  bool updateBasicBlock(BasicBlock &BB, Function *F,
                        GlobalVariable *GlobalCounter,
                        const TargetLibraryInfo &TLInfo, Module *M,
                        bool UseOpenMP);
};

} // namespace dtrans

ModulePass *createDTransPaddedMallocWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_PADDEDMALLOC_H
