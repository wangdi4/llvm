//===- LocalBufferAnalysis.cpp - DPC++ kernel local buffer analysis -------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LocalBufferAnalysis.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DevLimits.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-local-buffer-analysis"

void LocalBufferInfo::updateLocalsMap(GlobalValue *LocalVal, User *U) {
  // Instruction, Operator and Constant are the only possible subtypes of U
  if (isa<Instruction>(U)) {
    Instruction *I = cast<Instruction>(U);

    // declaring variables for debugging purposes shouldn't affect local
    // buffers.
    if (MDNode *mdn = I->getMetadata("dbg_declare_I")) {
      if (mdconst::extract<ConstantInt>(mdn->getOperand(0))->isAllOnesValue()) {
        return;
      }
    }
    // Parent of Instruction is BasicBlock
    // Parent of BasicBlock is Function
    Function *F = I->getParent()->getParent();
    // Add LocalVal to the set of local values used by F
    LocalUsageMap[F].insert(LocalVal);
  } else if (isa<Constant>(U)) {
    // Recursievly locate all Us of the constant value
    for (auto It = U->user_begin(); It != U->user_end(); ++It) {
      updateLocalsMap(LocalVal, *It);
    }
  } else {
    // Operator is an internal llvm class, so we do not expect it to be a U
    // of GlobalValue.
    llvm_unreachable("Unexpected user type");
  }
}

void LocalBufferInfo::updateDirectLocals(Module &M) {
  // Get a list of all the global values in the module
  Module::GlobalListType &Globals = M.getGlobalList();

  // Find globals that appear in the origin kernel as local variables and add
  // update mapping accordingly
  for (Module::GlobalListType::iterator It = Globals.begin(), E = Globals.end();
       It != E; ++It) {
    GlobalValue *Val = &*It;

    const PointerType *TP = cast<PointerType>(Val->getType());
    if (TP->getAddressSpace() !=
        DPCPPKernelCompilationUtils::ADDRESS_SPACE_LOCAL) {
      // LOCL_VALUE_ADDRESS_SPACE = '3' is a magic number for global variables
      // that were in origin local kernel variable!
      continue;
    }

    // If we reached here, then Val is a global value that was originally a
    // local value.
    for (GlobalValue::user_iterator UI = Val->user_begin(),
                                    UE = Val->user_end();
         UI != UE; ++UI) {
      updateLocalsMap(Val, *UI);
    }
  } // Find globals done.
}

size_t LocalBufferInfo::calculateLocalsSize(Function *F, unsigned MaxDepth) {
  --MaxDepth;

  if (!F || F->isDeclaration() || !MaxDepth) {
    // Not module function, no need for local buffer, return size zero.
    return 0;
  }

  if (LocalSizeMap.count(F)) {
    // local size of this function already calculated.
    return LocalSizeMap[F];
  }

  DataLayout DL(M);
  size_t LocalBufferSize = 0;

  for (GlobalValue *LclBuff : LocalUsageMap[F]) {
    assert(LclBuff &&
           "locals container contains something other than GlobalValue!");

    // Calculate required buffer size.
    size_t ArraySize =
        DL.getTypeAllocSize(LclBuff->getType()->getElementType());
    assert(0 != ArraySize && "local buffer size is zero!");

    // Advance total implicit size.
    LocalBufferSize += ADJUST_SIZE_TO_MAXIMUM_ALIGN(ArraySize);
  }

  // Update direct local size of this function.
  DirectLocalSizeMap[F] = LocalBufferSize;

  size_t ExtraLocalBufferSize = 0;
  // look for calls to other kernels.
  for (auto &N : *(*CG)[F]) {
    auto *CI = cast<CallInst>(*N.first);
    size_t CallLocalSize = calculateLocalsSize(CI->getCalledFunction(), MaxDepth);
    if (ExtraLocalBufferSize < CallLocalSize) {
      // Found Function that needs more local size,
      // update max ExtraLocalBufferSize
      ExtraLocalBufferSize = CallLocalSize;
    }
  }

  LocalBufferSize += ExtraLocalBufferSize;

  // Update the local size of this function
  LocalSizeMap[F] = LocalBufferSize;
  return LocalBufferSize;
}

void LocalBufferInfo::analyzeModule(CallGraph *CG) {
  this->CG = CG;
  LocalUsageMap.clear();
  LocalSizeMap.clear();
  DirectLocalSizeMap.clear();

  // Initialize localUsageMap
  updateDirectLocals(*M);

  for (auto &F : *M) {
    auto FMD = DPCPPKernelMetadataAPI::FunctionMetadataAPI(&F);
    bool isRecursive = FMD.RecursiveCall.hasValue() && FMD.RecursiveCall.get();
    unsigned MaxDepth = isRecursive ? MAX_RECURSION_DEPTH : UINT_MAX;
    calculateLocalsSize(&F, MaxDepth);
  }
}

// Provide a definition for the static class member used to identify passes.
AnalysisKey LocalBufferAnalysis::Key;

LocalBufferInfo LocalBufferAnalysis::run(Module &M,
                                         AnalysisManager<Module> &AM) {
  CallGraph *CG = &AM.getResult<CallGraphAnalysis>(M);
  LocalBufferInfo WPAResult(&M);
  WPAResult.analyzeModule(CG);

  return WPAResult;
}

INITIALIZE_PASS_BEGIN(LocalBufferAnalysisLegacy, DEBUG_TYPE,
                      "Provide local values analysis info", false, true)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_END(LocalBufferAnalysisLegacy, DEBUG_TYPE,
                    "Provide local values analysis info", false, true)

char LocalBufferAnalysisLegacy::ID = 0;

LocalBufferAnalysisLegacy::LocalBufferAnalysisLegacy() : ModulePass(ID) {
  initializeLocalBufferAnalysisLegacyPass(*PassRegistry::getPassRegistry());
}

bool LocalBufferAnalysisLegacy::runOnModule(Module &M) {
  CallGraph *CG = &getAnalysis<CallGraphWrapperPass>().getCallGraph();
  LocalBufferInfo *LBAResult = new LocalBufferInfo(&M);
  LBAResult->analyzeModule(CG);

  Result.reset(LBAResult);

  return false;
}

ModulePass *llvm::createLocalBufferAnalysisLegacyPass() {
  return new LocalBufferAnalysisLegacy();
}
