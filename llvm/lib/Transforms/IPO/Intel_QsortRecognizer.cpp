#if INTEL_FEATURE_SW_ADVANCED
//===------- Intel_QsortRecognizer.cpp --------------------------------===//
//
// Copyright (C) 2019-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This implements a qsort recognition pass. It looks through the module and
// attempts to identify Functions that implement a qsort of the type described
// in the paper "Engineering a Sort Function" by Jon L. Bentley and M. Douglas
// McIlroy (in Software -- Practice and Experience, Volume 23, Issue 11). If
// such a Function is identified, it will mark it with the "is-qsort"
// Function attribute. This qsort is a well-tuned implementation of quicksort
// which degenerates to insertion sort for sufficiently small arrays.
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/IPO/Intel_QsortRecognizer.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;

#define DEBUG_TYPE "qsortrecognizer"

STATISTIC(QsortsRecognized, "Number of qsort functions recognized");

//
// Return 'true' if 'F' should be recognized as a qsort like 'spec_qsort'.
//
static bool isQsort(Function *F) {
  if (!F->hasFnAttribute("is-qsort-spec_qsort"))
    return false;
  for (auto &I : instructions(*F)) {
    auto CB = dyn_cast<CallBase>(&I);
    if (!CB || isa<DbgInfoIntrinsic>(I))
      continue;
    Function *Callee = CB->getCalledFunction();
    if (!Callee || Callee == F)
      continue;
    if (Callee->isIntrinsic()) {
      switch (Callee->getIntrinsicID()) {      
      case Intrinsic::smin:
        continue;
      default:
        return false;
      }
    }
    if (CB->hasFnAttr("must-be-qsort-compare") &&
        Callee->hasFnAttribute("is-qsort-compare"))
      continue;
    if (Callee->hasFnAttribute("must-be-qsort-med3") &&
        Callee->hasFnAttribute("is-qsort-med3"))
      continue;
    if (Callee->hasFnAttribute("must-be-qsort-swapfunc") &&
        Callee->hasFnAttribute("is-qsort-swapfunc"))
      continue;
    return false;
  }
  return true;
}

//
// Return 'true' if some Function in 'M' is recognized as a qsort. In such a
// case, set the 'is-qsort' attribute on the Function.
//
static bool QsortRecognizerImpl(Module &M) {
  bool SawQsort = false;
  for (auto &F : M.functions()) {
    if (isQsort(&F)) {
      F.addFnAttr("is-qsort");
      QsortsRecognized++;
      SawQsort = true;
      LLVM_DEBUG(dbgs() << "FOUND QSORT " << F.getName() <<"\n");
    }
  }
  return SawQsort;
}

namespace {

struct QsortRecognizerLegacyPass : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  QsortRecognizerLegacyPass(void) : ModulePass(ID) {
    initializeQsortRecognizerLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    return QsortRecognizerImpl(M);
  }
};

} // namespace

char QsortRecognizerLegacyPass::ID = 0;
INITIALIZE_PASS(QsortRecognizerLegacyPass, "qsortrecognizer", "QsortRecognizer",
                false, false)

ModulePass *llvm::createQsortRecognizerLegacyPass(void) {
  return new QsortRecognizerLegacyPass();
}

QsortRecognizerPass::QsortRecognizerPass(void) {}

PreservedAnalyses QsortRecognizerPass::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  QsortRecognizerImpl(M);
  return PreservedAnalyses::all();
}
#endif // INTEL_FEATURE_SW_ADVANCED
