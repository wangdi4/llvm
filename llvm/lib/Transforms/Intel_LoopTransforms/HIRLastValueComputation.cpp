//===- HIRLastValueComputation.cpp - Implements LastValueComputation class
//------------===//
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
#include "llvm/InitializePasses.h"
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

static cl::opt<unsigned> NumOperationsThreshold(
    OPT_SWITCH "-num-operations-threshold", cl::init(5), cl::Hidden,
    cl::desc("Threshold for number of operations in upper bound above which "
             "code generation is not triggered."));

static cl::opt<unsigned>
    TripCountThreshold(OPT_SWITCH "-trip-count-threshold", cl::init(10),
                       cl::Hidden,
                       cl::desc("Minimum trip count of the multi exit loop "
                                "which triggers code generation"));

// Returns true if the number of arithmetic operations in \p UBCE is greater
// than the threshold.
static bool isUpperBoundComplicated(CanonExpr *UBCE) {
  auto &BU = UBCE->getBlobUtils();
  unsigned Num = 0;

  for (auto Blob = UBCE->blob_begin(), E = UBCE->blob_end(); Blob != E;
       ++Blob) {

    // TODO:  Include other operations like additions between blob operands or
    //      // C0 addition, IV multiplications and etc.
    Num += BU.getNumOperations(Blob->Index, nullptr);
  }

  return Num > NumOperationsThreshold;
}

bool HIRLastValueComputation::isLegalAndProfitable(HLLoop *Lp, HLInst *HInst,
                                                   unsigned LoopLevel,
                                                   CanonExpr *UBCE,
                                                   bool IsUpperBoundComplicated,
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

      if (IsUpperBoundComplicated) {

        for (CanonExpr *CE :
             make_range(OpRef->canon_begin(), OpRef->canon_end())) {
          unsigned BlobCoeff = CE->getIVBlobCoeff(LoopLevel);

          if (BlobCoeff != InvalidBlobIndex) {
            return false;
          }
        }
      }

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

  DDGraph DDG = HDDA.getGraph(Lp);

  if (DDG.getNumOutgoingEdges(LRef) != 0) {
    return false;
  }

  return true;
}

static bool isLiveOutOfEdge(unsigned Symbase, BasicBlock *SrcBB,
                            BasicBlock *TargetBB, HLRegion *ParRegion) {
  bool IsLiveout = false;

  // Check if the symbase of any liveout value going from src -> target bblock
  // matches incoming symbase.
  for (auto &Phi : TargetBB->phis()) {
    auto *LiveoutInst =
        dyn_cast<Instruction>(Phi.getIncomingValueForBlock(SrcBB));

    if (!LiveoutInst || !ParRegion->containsBBlock(LiveoutInst->getParent()) ||
        (ParRegion->getLiveOutSymbase(LiveoutInst) != Symbase)) {
      continue;
    }

    IsLiveout = true;
    break;
  }

  return IsLiveout;
}

static bool isLiveOutOfNormalExit(unsigned Symbase, HLLoop *Lp) {
  auto *ParRegion = Lp->getParentRegion();

  if (Lp != ParRegion->getLastChild()) {
    // This information cannot be computed easily for other cases so we give
    // up.
    return true;
  }

  auto *SrcBB = ParRegion->getExitBBlock();
  auto *TargetBB = ParRegion->getSuccBBlock();

  if (!SrcBB || !TargetBB) {
    // Can we assert on this?
    return false;
  }

  return isLiveOutOfEdge(Symbase, SrcBB, TargetBB, ParRegion);
}

static bool isLiveOutOfGoto(unsigned Symbase, HLGoto *Goto,
                            HLRegion *ParRegion) {
  if (!Goto->isExternal()) {
    // This information cannot be computed easily for internal jumps so we give
    // up.
    return true;
  }

  auto *SrcBB = Goto->getSrcBBlock();
  auto *TargetBB = Goto->getTargetBBlock();

  return isLiveOutOfEdge(Symbase, SrcBB, TargetBB, ParRegion);
}

static bool
processEarlyExits(SmallVectorImpl<HLGoto *> &Gotos, HLInst *HInst,
                  SmallDenseMap<HLGoto *, HLNode *, 16> &GotoInsertPosition) {

  auto *ParRegion = HInst->getParentRegion();
  unsigned LvalSymbase = HInst->getLvalDDRef()->getSymbase();
  bool Cloned = false;

  for (auto &Goto : Gotos) {
    HLInst *CloneInst = HInst->clone();

    if (HInst->getTopSortNum() < Goto->getTopSortNum() &&
        isLiveOutOfGoto(LvalSymbase, Goto, ParRegion)) {

      // We are transforming this-
      //
      // DO i1 = 0, N
      //   t = i1;
      //   if (){
      //     goto Exit;
      //   }
      // END DO
      //
      // To-
      //
      // DO i1 = 0, N
      //  if (){
      //    t = i1;
      //    goto Exit;
      //  }
      // END DO
      //   t = N;
      auto It = GotoInsertPosition.find(Goto);

      if (It == GotoInsertPosition.end()) {
        HLNodeUtils::insertBefore(Goto, CloneInst);
      } else {
        HLNodeUtils::insertBefore(It->second, CloneInst);
      }

      GotoInsertPosition[Goto] = CloneInst;
      Cloned = true;
    }
  }

  return Cloned;
}

bool HIRLastValueComputation::doLastValueComputation(HLLoop *Lp) {
  // Get Loop's UpperBound (UB)
  CanonExpr *UBCE = Lp->getUpperCanonExpr();

  unsigned LoopLevel = Lp->getNestingLevel();
  bool IsNSW = Lp->isNSW();
  SmallVector<HLInst *, 64> CandidateInsts;

  bool IsUpperBoundComplicated = isUpperBoundComplicated(UBCE);

  for (auto It = Lp->child_rbegin(), End = Lp->child_rend(); It != End; ++It) {
    auto HInst = dyn_cast<HLInst>(&*It);

    if (!isLegalAndProfitable(Lp, HInst, LoopLevel, UBCE,
                              IsUpperBoundComplicated, IsNSW)) {
      continue;
    }

    CandidateInsts.push_back(HInst);
  }

  if (CandidateInsts.empty()) {
    return false;
  }

  SmallVector<HLGoto *, 16> Gotos;

  bool ShouldGenCode = !IsUpperBoundComplicated;
  bool IsMultiExit = Lp->getNumExits() > 1;

  if (IsMultiExit) {
    uint64_t TripCount = 0;
    Lp->populateEarlyExits(Gotos);

    // We want to have a conservative heuristic on code generation because last
    // value computation is not profitable enough
    if (!Lp->isConstTripLoop(&TripCount) || TripCount <= TripCountThreshold ||
        Gotos.size() > 1) {
      ShouldGenCode = false;
    }
  }

  const SmallVector<const RegDDRef *, 1> Aux = {Lp->getUpperDDRef()};
  SmallDenseMap<HLGoto *, HLNode *, 16> GotoInsertPosition;

  for (auto *HInst : CandidateInsts) {

    unsigned LvalSymbase = HInst->getLvalDDRef()->getSymbase();

    if (IsMultiExit) {
      bool Cloned = processEarlyExits(Gotos, HInst, GotoInsertPosition);

      if (!Cloned) {
        Lp->removeLiveOutTemp(LvalSymbase);
      }

      if (!isLiveOutOfNormalExit(LvalSymbase, Lp)) {
        HLNodeUtils::remove(HInst);
        continue;
      }
    } else {
      Lp->removeLiveOutTemp(LvalSymbase);
    }

    for (auto I = HInst->op_ddref_begin(), E = HInst->op_ddref_end(); I != E;
         I++) {
      RegDDRef *OpRef = *I;

      DDRefUtils::replaceIVByCanonExpr(OpRef, LoopLevel, UBCE, IsNSW);
      OpRef->makeConsistent(Aux, LoopLevel - 1);
    }

    HLNodeUtils::moveAsFirstPostexitNode(Lp, HInst);
  }

  // Mark the loop and its parent loop/region have been changed
  if (ShouldGenCode) {
    Lp->getParentRegion()->setGenCode();
  }

  HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(Lp);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<HIRLoopStatistics>(Lp);
  HLNodeUtils::removeEmptyNodes(Lp);
  return true;
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
    if (Lp->isUnknown() || !Lp->isNormalized()) {
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
