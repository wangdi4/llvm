//===- HIRVecDirInsert.cpp - Implements HIRVecDirInsert class -------------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements HIRVecDirInsert transformation. It identifies
// vectorization candidate loops and mark them via directive intrinsics.
//
// Available options:
//   -disable-hir-vec-dir-insert   Disable auto-vectorization (O2 and above)
//   -disable-hir-vec-outer        Disable outer loop auto-vectorization (O3)
//
// See also HIRParVecAnalysis.cpp for diagnostic related flags.
//
//===----------------------------------------------------------------------===//

#include "ParVecDirectiveInsertion.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRVecDirInsert.h"

#define DEBUG_TYPE "parvec-transform"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    NoAutoVec("disable-hir-vec-dir-insert", cl::init(false), cl::Hidden,
              cl::desc("Disable Auto Vectorization at O2 and above"));
static cl::opt<bool>
    OuterVecDisabled("disable-hir-vec-outer", cl::init(true), cl::Hidden,
                     cl::desc("Disable Outer Loop Auto Vectorization at O3"));

namespace {

/// \brief Analyze auto-vectorizability of the loops.
static bool shouldRunOnFunction(Function &F, bool OuterVec) {
  if (NoAutoVec) {
    LLVM_DEBUG(dbgs() << "Vec Directive Insertion disabled"
                         " due to -disable-hir-vec-dir-insert.\n");
    return false;
  }
  if (HIRParVecAnalysis::isSIMDEnabledFunction(F)) {
    LLVM_DEBUG(dbgs() << "Vec Directive Insertion skipped"
                         " for vector variants of SIMD Enabled Function : "
                      << F.getName() << "\n");
    return false;
  }
  LLVM_DEBUG(dbgs() << "Vec Directive Insertion (Outer Loop "
                    << (OuterVec ? "Enabled" : "Disabled")
                    << ") for Function : " << F.getName() << "\n");
  return true;
}

/// \brief Invoke auto-vectorization legality analysis and insert
/// auto-vectorization candidate directive to the loops. When the directive
/// is inserted to a loop, further analysis will be performed by the vectorizer
/// before final auto-vectorization decision is made.
class HIRVecDirInsert : public HIRTransformPass {
  ParVecDirectiveInsertion Impl;
  bool OuterVec;

public:
  static char ID;

  HIRVecDirInsert(bool OuterVec = true)
      : HIRTransformPass(ID),
        Impl(OuterVec && !OuterVecDisabled
                 ? ParVecInfo::VectorForVectorizer
                 : ParVecInfo::VectorForVectorizerInnermost),
        OuterVec(OuterVec && !OuterVecDisabled) {
    initializeHIRVecDirInsertPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;
    if (!shouldRunOnFunction(F, OuterVec))
      return false;
    auto HIRF = &getAnalysis<HIRFrameworkWrapperPass>().getHIR();
    auto HPVA = &getAnalysis<HIRParVecAnalysisWrapperPass>().getHPVA();
    return Impl.runOnFunction(F, HIRF, HPVA);
  }

  void releaseMemory() override {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRParVecAnalysisWrapperPass>();
    AU.setPreservesAll();
  }
};

} // unnamed namespace

char HIRVecDirInsert::ID = 0;
INITIALIZE_PASS_BEGIN(HIRVecDirInsert, "hir-vec-dir-insert",
                      "HIR Vec Directive Insertion Pass", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRParVecAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRVecDirInsert, "hir-vec-dir-insert",
                    "HIR Vec Directive Insertion Pass", false, false)

FunctionPass *llvm::createHIRVecDirInsertPass(bool OuterVec) {
  return new HIRVecDirInsert(OuterVec);
}

namespace llvm {

namespace loopopt {

PreservedAnalyses HIRVecDirInsertPass::run(Function &F,
                                           FunctionAnalysisManager &AM) {
  auto HIRF = &AM.getResult<HIRFrameworkAnalysis>(F);
  auto HPVA = &AM.getResult<HIRParVecAnalysisPass>(F);
  const bool OuterVec = this->OuterVec && !OuterVecDisabled;
  if (shouldRunOnFunction(F, OuterVec)) {
    ParVecDirectiveInsertion(OuterVec
                                 ? ParVecInfo::VectorForVectorizer
                                 : ParVecInfo::VectorForVectorizerInnermost)
        .runOnFunction(F, HIRF, HPVA);
  }
  return PreservedAnalyses::all();
}

} // namespace loopopt
} // namespace llvm
