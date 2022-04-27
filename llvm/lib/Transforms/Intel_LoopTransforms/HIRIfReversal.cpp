// ===- HIRIfReversal.cpp - Implement HIR If Reversal Transformation -===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===---------------------------------------------------------------------===//
// HIR If Reversal Example
//
// [ORIGINAL]                       [AFTER REVERSAL]
//
// if (Pred){                       if (!Pred){
//   A(i-1) = B(i)                     B(i) = A(i)
// } else {                         } else {
//   B(i) = A(i)                       A(i-1) = B(i)
// }                                }
//===---------------------------------------------------------------------===//
//
// This file implements HIR If Reversal Transformation. The primary motivation
// is to resolve lexically backwards dependencies that exist based on if/else
// branches. This transformation is always safe but not always profitable.
// As such the main concern is compile time and targeting specific loops/ifs.
//
// Available options:
// -hir-if-reversal:          Perform HIR If Reversal
// -disable-hir-if-reversal:  Flag to Disable/Bypass HIR If Reversal
//
//
//

#include "llvm/Transforms/Intel_LoopTransforms/HIRIfReversalPass.h"

#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"

#include "HIRIfReversal.h"

#define DEBUG_TYPE "hir-if-reversal"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    DisableHIRIfReversal("disable-hir-if-reversal", cl::init(false), cl::Hidden,
                         cl::desc("Disable HIR If Reversal Transformation"));

static bool areInDivergingIfPaths(const HLIf *If, const HLNode *Node1,
                                  const HLNode *Node2) {
  bool NodeInThenPath = If->isThenChild(Node1) || If->isThenChild(Node2);
  bool NodeInElsePath = If->isElseChild(Node1) || If->isElseChild(Node2);

  return NodeInThenPath && NodeInElsePath;
}

bool HIRIfReversal::findProfitableCandidates(const HLLoop *InnermostLoop,
                                             SmallSet<HLIf *, 2> &IfSet) {
  // If Reversal is primarily used to resolve backwards dependencies blocking
  // vectorization. We can ignore if it won't help vectorization.
  if (!InnermostLoop->isDo() || InnermostLoop->hasVectorizeDisablingPragma() ||
      InnermostLoop->isInSIMDRegion()) {
    return false;
  }

  // Do Structural checks
  auto LS = HLS.getTotalLoopStatistics(InnermostLoop);
  if (LS.hasSwitches() || LS.hasCallsWithUnsafeSideEffects() ||
      LS.hasLabels() || !LS.hasIfs()) {
    return false;
  }

  SmallVector<HLIf *, 4> IfCandidates;

  for (auto &Node :
       make_range(InnermostLoop->child_begin(), InnermostLoop->child_end())) {
    auto IfNode = dyn_cast<HLIf>(&Node);

    if (IfNode && IfNode->getNumPredicates() == 1 &&
        IfNode->hasThenChildren() && IfNode->hasElseChildren()) {
      IfCandidates.push_back(const_cast<HLIf *>(IfNode));
    }
  }

  if (IfCandidates.empty()) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "If Reversal initial candidates:\n";);
  LLVM_DEBUG(for (auto If : IfCandidates) { If->dump(); });

  unsigned InnermostLoopLevel = InnermostLoop->getNestingLevel();
  DDGraph DDG = HDDA.getGraph(InnermostLoop);
  LLVM_DEBUG(DDG.dump(););

  // Check ifs for backwards dependency in if/else paths. Starting with each
  // memref in else path, check the DDEdge to see if there is a backwards
  // dependency that originates from the then path.
  for (auto If : IfCandidates) {
    for (auto &Node : make_range(If->else_begin(), If->else_end())) {
      auto Inst = dyn_cast<HLInst>(&Node);
      if (!Inst) {
        continue;
      }

      for (const RegDDRef *Ref :
           llvm::make_range(Inst->ddref_begin(), Inst->ddref_end())) {
        // Only check memrefs for backwards dependency
        if (!Ref->isMemRef()) {
          continue;
        }

        // DDEdge does not exist between reads
        for (auto Edge : DDG.outgoing(Ref)) {
          if (Edge->preventsVectorization(InnermostLoopLevel)) {
            LLVM_DEBUG(dbgs() << "DDEdge Candidate: "; Edge->dump(); dbgs() << "\n");
            DDRef *Src = Edge->getSrc();
            DDRef *Dst = Edge->getSink();
            auto *SrcNode = Src->getHLDDNode();
            auto *DstNode = Dst->getHLDDNode();
            const auto DV =
                Edge->getDVAtLevel(InnermostLoop->getNestingLevel());
            bool IsLessThan = DV == DVKind::LT;
            if (IsLessThan && areInDivergingIfPaths(If, SrcNode, DstNode)) {
              LLVM_DEBUG(
                  dbgs() << "Candidate Found " << If->getNumber() << "\n";;);
              IfSet.insert(If);
              continue;
            }

            // If we can't resolve this edge, bailout
            LLVM_DEBUG(dbgs() << "Bailout: couldn't resolve edge.\n";);
            return false;
          }
        }
      }
    }
  }

  LLVM_DEBUG(dbgs() << "If Reversal final candidates:\n";);
  LLVM_DEBUG(for (auto If : IfSet) { If->dump(); });
  return !IfSet.empty();
}

bool HIRIfReversal::run() {
  if (DisableHIRIfReversal) {
    LLVM_DEBUG(
        dbgs() << "HIR If Reversal Transformation Disabled or Skipped\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIRIfReversal on Function : "
                    << HIRF.getFunction().getName() << "\n");

  SmallVector<HLLoop *, 64> InnermostLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(InnermostLoops);

  if (InnermostLoops.empty()) {
    return false;
  }

  for (auto &Lp : InnermostLoops) {
    SmallSet<HLIf *, 2> IfSet;
    if (!findProfitableCandidates(Lp, IfSet)) {
      continue;
    }

    for (auto If : IfSet) {
      LLVM_DEBUG(dbgs() << "IF Reversal for If number: <" << If->getNumber()
                        << ">\n";
                 If->dump(););
      If->invertPredAndReverse();
    }

    // Preserve statistics and reduction analysis
    HIRInvalidationUtils::invalidateBody<HIRLoopStatistics,
                                         HIRSafeReductionAnalysis>(Lp);
  }

  return false;
}

PreservedAnalyses HIRIfReversalPass::runImpl(llvm::Function &F,
                                             llvm::FunctionAnalysisManager &AM,
                                             HIRFramework &HIRF) {
  HIRIfReversal(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
                AM.getResult<HIRLoopStatisticsAnalysis>(F))
      .run();

  return PreservedAnalyses::all();
}

class HIRIfReversalLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRIfReversalLegacyPass() : HIRTransformPass(ID) {
    initializeHIRIfReversalLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRIfReversal(getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
                         getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
                         getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS())
        .run();
  }
};

char HIRIfReversalLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRIfReversalLegacyPass, "hir-if-reversal",
                      "HIR If Reversal", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRIfReversalLegacyPass, "hir-if-reversal",
                    "HIR If Reversal", false, false)

FunctionPass *llvm::createHIRIfReversalPass() {
  return new HIRIfReversalLegacyPass();
}
