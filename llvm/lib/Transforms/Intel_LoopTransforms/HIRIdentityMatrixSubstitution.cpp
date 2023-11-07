//===- HIRIdentityMatrixSubstitution.cpp Implements
// HIRIdentityMatrixSubstitution class -===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass is used to test a utility to substitute the identity matrix.
// Identity matrix pattern can be recognized using a utility in HLNodeUtils
// and is called from the IdentityMatrixIdiomRecognition pass (separate from
// the transform). Substitution is handled after pre-vec unroll, where we can
// perform simplification to remove redundant or dead expressions before the
// vectorizer kicks in. This pass is used to test the utility that performs
// substitution.
//
// We are simplifying the use of the identity matrix for costly operations:
//
//  Do i = 0, 5
//    Do j = 0, 5
//      A(j,i) = 0.0
//    ENDDO
//    A(i,i) = 1.0
//  ENDDO
//
//  Do i = 0, 5
//    Do j = 0, 5
//      B(j,i) = t1 - 0.5 * dt * B(j,i) * A(j,i)
//    ENDDO
//  ENDDO
//
// After complete unroll, we can substitute 1s and 0s for A and cleanup the
// expression to remove extra zero multiplications.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRIdentityMatrixSubstitution.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#define OPT_SWITCH "hir-identity-matrix-substitution"
#define OPT_DESC "HIR Identity Matrix Substitution Pass for Testing"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> EnableIdentityMatrixSubstitution(
    "enable-identity-matrix-substitution", cl::init(false), cl::Hidden,
    cl::desc("Enable utility to detect/substitute identity matrix in " OPT_DESC
             " pass"));

namespace {

class HIRIdentityMatrixSubstitution {
  HIRFramework &HIRF;
  HIRLoopStatistics &HLS;

public:
  HIRIdentityMatrixSubstitution(HIRFramework &HIRF, HIRLoopStatistics &HLS)
      : HIRF(HIRF), HLS(HLS) {}

  bool run();

private:
};
} // namespace

// Finds the next sibling loop in region if it exists.
// Region:
// do i1
//   do i2
//     do i3
//       ...
//     end i3
//   end i2
// end i1
// do j1
//   ...
//
// For substitution we have a def and use loop. If the def loop is i3, the
// use loop candidate is j1, which is the next sibling loop of interest.
static HLLoop *getNextSiblingLoop(HLLoop *CurrLoop) {
  while (CurrLoop) {
    if (HLLoop *NextLoop = dyn_cast_or_null<HLLoop>(CurrLoop->getNextNode())) {
      return NextLoop;
    }
    CurrLoop = CurrLoop->getParentLoop();
  }
  return nullptr;
}

bool HIRIdentityMatrixSubstitution::run() {
  if (!EnableIdentityMatrixSubstitution) {
    LLVM_DEBUG(dbgs() << "HIR Identity Matrix Substitution Disabled \n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIR Identity Matrix Substitution on Function : "
                    << HIRF.getFunction().getName() << "\n");

  // Gather all inner-most loop and its parent loop as Candidates
  SmallVector<HLLoop *, 64> InnermostLoops;

  HLNodeUtils &HNU = HIRF.getHLNodeUtils();
  HNU.gatherInnermostLoops(InnermostLoops);

  if (InnermostLoops.empty()) {
    LLVM_DEBUG(dbgs() << HIRF.getFunction().getName()
                      << "() has no inner-most loop\n ");
    return false;
  }

  bool Result = false;

  for (auto &Lp : InnermostLoops) {
    SmallVector<const RegDDRef *, 2> IDMatRefs;
    HLNodeUtils::findInner2DIdentityMatrix(&HLS, Lp, IDMatRefs);

    if (IDMatRefs.empty()) {
      continue;
    }

    LLVM_DEBUG(dbgs() << HIRF.getFunction().getName()
                      << ": Found identity matrix in function\n ");

    // Find sibling loop for substitution.
    // Pass the Outer loop since identity ref has at minimum 2 dims.
    // TODO: Add legality check here since this is not checking
    // for redefinitions between loops.
    auto *UseLoop = getNextSiblingLoop(Lp->getParentLoop());
    if (!UseLoop) {
      continue;
    }

    // Find innermostloop for perfectly loopnest
    while (isa<HLLoop>(UseLoop->getFirstChild())) {
      UseLoop = cast<HLLoop>(UseLoop->getFirstChild());
    }

    if (!UseLoop->isInnermost()) {
      continue;
    }

    for (auto &IdentRef : IDMatRefs) {
      Result |=
          HIRTransformUtils::doIdentityMatrixSubstitution(UseLoop, IdentRef);
      HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(UseLoop);
      UseLoop->getParentRegion()->setGenCode();
    }
  }

  return Result;
}

PreservedAnalyses HIRIdentityMatrixSubstitutionPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  ModifiedHIR = HIRIdentityMatrixSubstitution(
                    HIRF, AM.getResult<HIRLoopStatisticsAnalysis>(F))
                    .run();
  return PreservedAnalyses::all();
}
