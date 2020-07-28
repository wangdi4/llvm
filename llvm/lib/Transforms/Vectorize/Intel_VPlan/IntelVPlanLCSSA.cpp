//===-- IntelVPlanLCSSA.cpp -----------------------------------------------===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the simplified utility to transform VPlan into LCSSA
/// form. The assumption is that it is run post loop exits canonicalization and
/// so doesn't need to employ the full blown SSAUpdater-based phi nodes
/// placement.
///
//===----------------------------------------------------------------------===//

#include "IntelVPlanLCSSA.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanLoopInfo.h"

using namespace llvm;
using namespace llvm::vpo;

static void computeBlocksDominatingExit(
    const VPLoop &L, const VPDominatorTree &DT,
    SmallVectorImpl<VPBasicBlock *> &BlocksDominatingExit) {
  VPBasicBlock *Header = L.getHeader();
  VPBasicBlock *BB = L.getExitBlock();
  assert(BB && "Loop exits canonicalization hasn't been done!");
  assert(DT.dominates(Header, BB) && "Loop has multiple entries!");
  while (BB != Header) {
    BB = DT.getNode(BB)->getIDom()->getBlock();

    // LCSSA.cpp has a check for the following:
    //
    // |---- A
    // |     |
    // |     B<--
    // |     |  |
    // |---> C --
    //       |
    //       D
    //
    // We filtered it out by the DT.dominates(Header, Exit) assert above.

    BlocksDominatingExit.push_back(BB);
  }
}

static void formLCSSA(const VPLoop &L, const VPDominatorTree &DT,
                      const VPLoopInfo &LI, bool SkipTopLoop) {
  if (SkipTopLoop && !L.getParentLoop())
    return;

  SmallVector<VPBasicBlock *, 8> BlocksDominatingExit;
  computeBlocksDominatingExit(L, DT, BlocksDominatingExit);
  VPBasicBlock *Exit = L.getExitBlock();
  assert(Exit && Exit->getSinglePredecessor() && "Loop exit isn't canonical!");
  VPBuilder Builder;
  Builder.setInsertPointFirstNonPhi(Exit);

  for (VPBasicBlock *BB : BlocksDominatingExit) {
    // Skip blocks that are part of any sub-loops, they must be in LCSSA
    // already.
    if (LI.getLoopFor(BB) != &L)
      continue;

    for (VPInstruction &Inst : *BB) {
      VPPHINode *LCSSAPhi = nullptr;
      SmallVector<VPUser *, 8> UsersToRewrite;
      for (VPUser *U : Inst.users()) {
        if (auto *UserInst = dyn_cast<VPInstruction>(U))
          if (L.contains(UserInst))
            // Not live-out.
            continue;

        if (auto *Phi = dyn_cast<VPPHINode>(U))
          if (Phi->getParent() == Exit) {
            // LCSSA phi, record it. Having multiple LCSSA phis for a given
            // live-out value is OK - we will use the last one.
            LCSSAPhi = Phi;
            continue;
          }

        UsersToRewrite.push_back(U);
      }

      if (UsersToRewrite.empty())
        continue;

      if (!LCSSAPhi) {
        LCSSAPhi = Builder.createPhiInstruction(Inst.getType(),
                                                Inst.getName() + ".lcssa");
        LCSSAPhi->addIncoming(&Inst, Exit->getSinglePredecessor());
        if (VPlanDivergenceAnalysis *DA =
                LCSSAPhi->getParent()->getParent()->getVPlanDA())
          DA->updateDivergence(*LCSSAPhi);
      }

      for (VPUser *U : UsersToRewrite)
        U->replaceUsesOfWith(&Inst, LCSSAPhi);
    }
  }
}

// TODO: Any invalidation for VPlanScalarEvolution similar to LCSSA.cpp?
static void formLCSSARecursively(const VPLoop &L, const VPDominatorTree &DT,
                                 const VPLoopInfo &LI, bool SkipTopLoop) {
  // Recurse depth-first through inner loops.
  for (VPLoop *SubLoop : L.getSubLoops())
    formLCSSARecursively(*SubLoop, DT, LI, SkipTopLoop);

  formLCSSA(L, DT, LI, SkipTopLoop);
}

void llvm::vpo::formLCSSA(VPlan &Plan, bool SkipTopLoop) {
  const VPLoopInfo &LI = *Plan.getVPLoopInfo();
  const VPDominatorTree &DT = *Plan.getDT();
  for (auto *TopLevelLoop : LI)
    formLCSSARecursively(*TopLevelLoop, DT, LI, SkipTopLoop);
}
