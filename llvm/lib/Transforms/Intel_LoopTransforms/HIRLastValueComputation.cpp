//===- HIRLastValueComputation.cpp - Implements LastValueComputation class
//------------===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass sinks a temp assignment to postexit by replacing IV by upper bound.
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRLastValueComputation.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/ADT/SmallSet.h"

#include "HIRLastValueComputationImpl.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#define OPT_SWITCH "hir-last-value-computation"
#define OPT_DESC "HIR Last Value Computation"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::lastvaluecomputation;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

static cl::opt<unsigned> NumOperationsThreshold(OPT_SWITCH
                                                "-num-operations-threshold",
                                                cl::init(5), cl::Hidden);

bool HIRLastValueComputation::isLegalAndProfitable(HLLoop *Lp, HLInst *HInst,
                                                   unsigned LoopLevel,
                                                   CanonExpr *UBCE,
                                                   bool IsNSW) {
  if (!HInst || HInst->isCallInst()) {
    return false;
  }

  RegDDRef *LRef = HInst->getLvalDDRef();

  if (!LRef || !LRef->isTerminalRef()) {
    return false;
  }

  HLNode *LastChild = Lp->getLastChild();

  if (HInst != LastChild && !HLNodeUtils::dominates(HInst, LastChild)) {
    return false;
  }

  bool HasIV = false;
  for (auto I = HInst->rval_op_ddref_begin(), End = HInst->rval_op_ddref_end();
       I != End; I++) {
    auto *OpRef = *I;
    if (OpRef->isMemRef()) {
      return false;
    }

    if (OpRef->isNonLinear()) {
      return false;
    }

    if (OpRef->hasIV(LoopLevel)) {
      // TODO: creating an instruction of the form t1 = UB in the postexit and
      // replacing IV by t1.
      if (!DDRefUtils::canReplaceIVByCanonExpr(OpRef, LoopLevel, UBCE, IsNSW)) {
        return false;
      }
      HasIV = true;
    }
  }

  if (!HasIV) {
    return false;
  }

  DDGraph DDG = HDDA.getGraph(Lp, false);

  if (DDG.getNumOutgoingEdges(LRef) != 0) {
    return false;
  }

  return true;
}

static bool isUpperBoundComplicated(CanonExpr *UBCE) {
  auto &BU = UBCE->getBlobUtils();
  unsigned Num = 0;

  for (auto Blob = UBCE->blob_begin(), E = UBCE->blob_end(); Blob != E;
       ++Blob) {
    Num += BU.getNumOperations(Blob->Index, nullptr);
  }

  return Num > NumOperationsThreshold;
}

bool HIRLastValueComputation::doLastValueComputation(HLLoop *Lp) {
  bool Sinked = false;
  unsigned LoopLevel = Lp->getNestingLevel();
  bool IsNSW = Lp->isNSW();

  // Get Loop's UpperBound (UB)
  CanonExpr *UBCE = Lp->getUpperCanonExpr();

  if (isUpperBoundComplicated(UBCE)) {
    return false;
  }

  for (auto It = Lp->child_rbegin(), Next = It, End = Lp->child_rend();
       It != End; It = Next) {
    ++Next;
    auto HInst = dyn_cast<HLInst>(&*It);

    if (!isLegalAndProfitable(Lp, HInst, LoopLevel, UBCE, IsNSW)) {
      continue;
    }

    Sinked = true;

    const SmallVector<const RegDDRef *, 1> Aux = {Lp->getUpperDDRef()};

    for (auto I = HInst->op_ddref_begin(), E = HInst->op_ddref_end(); I != E;
         I++) {
      RegDDRef *OpRef = *I;

      DDRefUtils::replaceIVByCanonExpr(OpRef, LoopLevel, UBCE, IsNSW);
      OpRef->makeConsistent(&Aux, LoopLevel - 1);
    }

    HLNodeUtils::moveAsFirstPostexitNode(Lp, HInst);
    Lp->removeLiveOutTemp(HInst->getLvalDDRef()->getSymbase());
  }

  if (Sinked) {
    // Mark the loop and its parent loop/region have been changed
    Lp->getParentRegion()->setGenCode();
    HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(Lp);
    HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<HIRLoopStatistics>(
        Lp);
    HLNodeUtils::removeEmptyNodes(Lp);
  }

  return Sinked;
}

bool HIRLastValueComputation::run() {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << "HIR Last Value Computation Disabled \n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIR Last Value Computation on Function : "
                    << HIRF.getFunction().getName() << "\n");

  // Gather all inner-most Loop Candidates
  SmallVector<HLLoop *, 64> CandidateLoops;

  HLNodeUtils &HNU = HIRF.getHLNodeUtils();
  HNU.gatherInnermostLoops(CandidateLoops);

  if (CandidateLoops.empty()) {
    LLVM_DEBUG(dbgs() << HIRF.getFunction().getName()
                      << "() has no inner-most loop\n ");
    return false;
  }
  // LLVM_DEBUG(dbgs() << " # Innermost Loops: " << CandidateLoops.size() <<
  // "\n");

  bool Result = false;

  for (auto &Lp : CandidateLoops) {
    if (!Lp->isDo()) {
      continue;
    }

    Result = doLastValueComputation(Lp) || Result;
  }

  return Result;
}

PreservedAnalyses
HIRLastValueComputationPass::run(llvm::Function &F,
                                 llvm::FunctionAnalysisManager &AM) {
  HIRLastValueComputation(AM.getResult<HIRFrameworkAnalysis>(F),
                          AM.getResult<HIRDDAnalysisPass>(F))
      .run();
  return PreservedAnalyses::all();
}

class HIRLastValueComputationLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRLastValueComputationLegacyPass() : HIRTransformPass(ID) {
    initializeHIRLastValueComputationLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRLastValueComputation(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
               getAnalysis<HIRDDAnalysisWrapperPass>().getDDA())
        .run();
  }
};

char HIRLastValueComputationLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLastValueComputationLegacyPass, OPT_SWITCH, OPT_DESC,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRLastValueComputationLegacyPass, OPT_SWITCH, OPT_DESC,
                    false, false)

FunctionPass *llvm::createHIRLastValueComputationPass() {
  return new HIRLastValueComputationLegacyPass();
}
