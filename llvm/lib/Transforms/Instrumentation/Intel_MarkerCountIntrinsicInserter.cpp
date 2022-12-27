#if INTEL_FEATURE_MARKERCOUNT
//==- Intel_MarkerCountIntrinsicInserter.cpp Mid-end marker count inserter--===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===-----------------------------------------------------------------------===//
//
// This file implements the MarkerCountIntrinsicInserterPass class which is used
// to insert mid-end marker count.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Instrumentation/Intel_MarkerCountIntrinsicInserter.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_MarkerCountInfo.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"

using namespace llvm;

unsigned
MarkerCountIntrinsicInserterPass::getMarkerCount(StringRef FunctionName) const {
  std::string Name = FunctionName.str();
  return OverrideMarkerCount.count(Name) ? OverrideMarkerCount.at(Name)
                                         : MarkerCountKind;
}

MarkerCountIntrinsicInserterPass::MarkerCountIntrinsicInserterPass(
    unsigned MarkerCountKind, StringRef OverrideMarkerCountFile)
    : MarkerCountKind(MarkerCountKind) {
  MarkerCount::parseMarkerCountFile(OverrideMarkerCount, MarkerCountKind,
                                    OverrideMarkerCountFile);
}

static bool isReturnBlock(const BasicBlock &BB) {
  return isa<ReturnInst>(BB.getTerminator());
}

bool MarkerCountIntrinsicInserterPass::insertMarkerCountIntrinsic(
    Function &F, unsigned MCK, LoopInfo *LI) {
  bool MarkPrologEpilog = MCK & MarkerCount::Function_ME;
  bool MarkLoopHeader = MCK & MarkerCount::Loop_ME;
  assert((MarkPrologEpilog || MarkLoopHeader) &&
         "expect at least one kind of marker count");
  // Bail out early
  if (!MarkPrologEpilog && LI->empty())
    return false;

  bool Changed = false;
  Module *M = F.getParent();
  // Loopless since function has only one entry basic block
  if (MarkPrologEpilog) {
    Function *IntrinsicFn =
        Intrinsic::getDeclaration(M, Intrinsic::mark_prolog);
    CallInst::Create(IntrinsicFn, "", F.getEntryBlock().getFirstNonPHI());
    Changed = true;
  }
  for (BasicBlock &BB : F) {
    if (isReturnBlock(BB) && MarkPrologEpilog) {
      Function *IntrinsicFn =
          Intrinsic::getDeclaration(M, Intrinsic::mark_epilog);
      CallInst::Create(IntrinsicFn, "", BB.getTerminator());
      assert(Changed &&
             "should already be set due to the insertion in entry block");
    }
    if (MarkLoopHeader && LI->isLoopHeader(&BB)) {
      Function *IntrinsicFn =
          Intrinsic::getDeclaration(M, Intrinsic::mark_loop_header);
      CallInst::Create(IntrinsicFn, "", BB.getFirstNonPHI());
      Changed = true;
    }
  }
  return Changed;
}

PreservedAnalyses
MarkerCountIntrinsicInserterPass::run(Function &F,
                                      FunctionAnalysisManager &AM) {
  unsigned MCK = getMarkerCount(F.getName());
  if (!(MCK & MarkerCount::ME))
    return PreservedAnalyses::all();

  LoopInfo *LI =
      MCK & MarkerCount::Loop_ME ? &AM.getResult<LoopAnalysis>(F) : nullptr;

  if (!insertMarkerCountIntrinsic(F, MCK, LI))
    return PreservedAnalyses::all();

  // This pass only inserts intrinsic that accesses memory that is not
  // accessible by the module being compiled and will return. The intrinsic
  // should be assumed zero-cost by compiler. Please preserve the analyses as
  // possbile here.
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<GlobalsAA>();
  PA.preserve<AndersensAA>();
  return PA;
}

#endif // INTEL_FEATURE_MARKERCOUNT
