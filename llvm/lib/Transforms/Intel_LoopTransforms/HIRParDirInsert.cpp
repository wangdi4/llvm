//===- HIRParDirInsert.cpp - Implements HIRParDirInsert class -------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements HIRParDirInsert transformation.
// It identifies auto parallelization loops and mark them via
// directive intrinsics, which is later parallelized after HIRCG.
//
// Available options:
//   -hir-enable-par         Enable auto-parallelization (at O2 and above)
//
// See also HIRParVecAnalysis.cpp for diagnostic related flags.
//
//===----------------------------------------------------------------------===//

#include "ParVecDirectiveInsertion.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "parvec-transform"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    AutoPar("hir-enable-par", cl::init(false), cl::Hidden,
            cl::desc("Enable Auto Parallelization at O2 and above"));

namespace {

/// \brief Invoke auto-parallelizability analysis (including cost model) and
/// insert auto-parallelization directive to the loops. When the directive
/// is inserted to a loop, auto-parallelization decision is already made.
class HIRParDirInsert : public ParVecDirectiveInsertion {
public:
  static char ID;

  HIRParDirInsert()
      : ParVecDirectiveInsertion(ID, ParVecInfo::ParallelForThreadizer) {
    initializeHIRParDirInsertPass(*PassRegistry::getPassRegistry());
  }
  /// \brief Analyze auto-parallelizability of the loops.
  bool runOnFunction(Function &F) override {
    if (!AutoPar) {
      DEBUG(dbgs() << "Par Directive Insertion skipped"
                      " due to lack of -hir-enable-par.\n");
      return false;
    }
    if (HIRParVecAnalysis::isSIMDEnabledFunction(F)) {
      DEBUG(dbgs() << "Par Directive Insertion skipped"
                      " for vector variants of SIMD Enabled Function : "
                   << F.getName() << "\n");
      return false;
    }
    DEBUG(dbgs() << "Par Directive Insertion for Function : " << F.getName()
                 << "\n");
    return ParVecDirectiveInsertion::runOnFunction(F);
  }
};

} // unnamed namespace

char HIRParDirInsert::ID = 0;
INITIALIZE_PASS_BEGIN(HIRParDirInsert, "hir-par-dir-insert",
                      "HIR Par Directive Insertion Pass", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRParVecAnalysis)
INITIALIZE_PASS_END(HIRParDirInsert, "hir-par-dir-insert",
                    "HIR Par Directive Insertion Pass", false, false)

FunctionPass *llvm::createHIRParDirInsertPass() {
  return new HIRParDirInsert();
}

