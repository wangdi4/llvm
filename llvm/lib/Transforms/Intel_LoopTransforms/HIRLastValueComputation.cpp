//===- HIRLastValueComputation.cpp - Implements LastValueComputation class -===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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
#include "llvm/Transforms/Intel_LoopTransforms/HIRLastValueComputationPass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/ADT/SmallSet.h"

#include "HIRLastValueComputation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
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

static cl::opt<bool> DisablePassForMultiExitLoops(
    "disable-" OPT_SWITCH "-multi-exit-loop", cl::init(false), cl::Hidden,
    cl::desc("Disable " OPT_DESC " pass for multi-exit loops"));

static cl::opt<unsigned> NumOperationsThreshold(
    OPT_SWITCH "-num-operations-threshold", cl::init(4), cl::Hidden,
    cl::desc("Threshold for number of operations in upper bound above which "
             "code generation is not triggered."));

static cl::opt<unsigned>
    TripCountThreshold(OPT_SWITCH "-trip-count-threshold", cl::init(10),
                       cl::Hidden,
                       cl::desc("Minimum trip count of the multi exit loop "
                                "which triggers code generation"));

bool HIRLastValueComputation::isLegalAndProfitable(
    HLLoop *Lp, HLInst *HInst, unsigned LoopLevel, CanonExpr *UBCE,
    bool IsUpperBoundComplicated, bool HasSignedIV, bool &HasIV) {
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
      HasIV = true;

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
      if (!DDRefUtils::canReplaceIVByCanonExpr(OpRef, LoopLevel, UBCE,
                                               HasSignedIV)) {
        return false;
      }
    }
  }

  DDGraph DDG = HDDA.getGraph(Lp);

  if (DDG.getNumOutgoingEdges(LRef) != 0) {
    return false;
  }

  return true;
}

static bool isLiveOutOfEdge(unsigned Symbase, const BasicBlock *SrcBB,
                            const BasicBlock *TargetBB,
                            const HLRegion *ParRegion) {
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

static bool isLiveOutOfNormalExit(unsigned Symbase, const HLLoop *Lp) {

  auto *ParRegion = Lp->getParentRegion();

  if (!ParRegion->isLiveOut(Symbase)) {
    // Conservatively return true if temp is not live out of region. The current
    // checks do not work for temps internal to the region.
    return true;
  }

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

struct TempFinder final : public HLNodeVisitorBase {
  unsigned TempSymbase;
  const HLGoto *EndGoto;
  bool ReachedEndGoto;
  bool FoundTempDef;

  TempFinder(unsigned TempSymbase, const HLGoto *EndGoto)
      : TempSymbase(TempSymbase), EndGoto(EndGoto), ReachedEndGoto(false),
        FoundTempDef(false) {}

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  void visit(const HLInst *Inst) {

    if (auto *LvalRef = Inst->getLvalDDRef()) {
      if (LvalRef->getSymbase() == TempSymbase) {
        FoundTempDef = true;
      }
    }
  }

  void visit(const HLGoto *Goto) {
    if (Goto == EndGoto) {
      ReachedEndGoto = true;
    }
  }

  bool foundTempDef() const { return FoundTempDef; }
  bool isDone() const { return (ReachedEndGoto || FoundTempDef); }
};

// Returns two booleans as a result. First boolean indicates that \p Symbase is
// liveout of this \p Goto exit. Second boolean indicates that \p Symbase is
// liveout using \p KnownDef and not some other temp def.
static std::pair<bool, bool>
isLiveOutOfGoto(unsigned Symbase, const HLInst *KnownDef, const HLGoto *Goto,
                const HLLoop *Lp, const HLRegion *ParRegion) {

  bool IsKnownDefBeforeGoto =
      (KnownDef->getTopSortNum() < Goto->getTopSortNum());

  if (!Goto->isExternal()) {
    // TODO: could there still be another definition of temp after KnownDef
    // requiring us to always use TempFinder?
    if (IsKnownDefBeforeGoto) {
      return std::make_pair(true, true);
    }

    TempFinder TF(Symbase, Goto);
    HLNodeUtils::visitRange(TF, Lp->child_begin(), Lp->child_end());

    return std::make_pair(TF.foundTempDef(), false);
  }

  auto *SrcBB = Goto->getSrcBBlock();
  auto *TargetBB = Goto->getTargetBBlock();

  bool IsLiveOut = isLiveOutOfEdge(Symbase, SrcBB, TargetBB, ParRegion);

  if (IsLiveOut) {
    return std::make_pair(true, IsKnownDefBeforeGoto);
  }

  return std::make_pair(false, false);
}

/// Returns true if lval of HInst is liveout of any early exit of the loop.
static bool
processEarlyExits(SmallVectorImpl<HLGoto *> &Gotos, HLInst *HInst,
                  const HLLoop *Lp,
                  SmallDenseMap<HLGoto *, HLNode *, 16> &GotoInsertPosition) {

  auto *ParRegion = HInst->getParentRegion();
  unsigned LvalSymbase = HInst->getLvalDDRef()->getSymbase();
  bool IsLiveOutOfEarlyExit = false;

  for (auto &Goto : Gotos) {
    HLInst *CloneInst = HInst->clone();

    bool IsLiveOut = false;
    bool IsLiveOutUsingInst = false;

    std::tie(IsLiveOut, IsLiveOutUsingInst) =
        isLiveOutOfGoto(LvalSymbase, HInst, Goto, Lp, ParRegion);

    IsLiveOutOfEarlyExit = IsLiveOutOfEarlyExit || IsLiveOut;

    if (IsLiveOutUsingInst) {

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
    }
  }

  return IsLiveOutOfEarlyExit;
}

bool HIRLastValueComputation::doLastValueComputation(HLLoop *Lp) {
  // Get Loop's UpperBound (UB)
  CanonExpr *UBCE = Lp->getUpperCanonExpr();

  unsigned LoopLevel = Lp->getNestingLevel();
  bool HasSignedIV = Lp->hasSignedIV();
  SmallVector<HLInst *, 64> CandidateInsts;

  bool IsUpperBoundComplicated =
      UBCE->getNumOperations() > NumOperationsThreshold;
  bool ContainIV = false;
  bool NeedConvertToStandAloneBlob = false;

  if (UBCE->getDenominator() != 1) {
    if (!UBCE->canConvertToStandAloneBlobOrConstant()) {
      return false;
    } else {
      NeedConvertToStandAloneBlob = true;
    }
  }

  for (auto It = Lp->child_rbegin(), End = Lp->child_rend(); It != End; ++It) {
    auto HInst = dyn_cast<HLInst>(&*It);
    bool HasIV = false;

    if (!isLegalAndProfitable(Lp, HInst, LoopLevel, UBCE,
                              IsUpperBoundComplicated, HasSignedIV, HasIV)) {
      continue;
    }

    ContainIV |= HasIV;

    CandidateInsts.push_back(HInst);
  }

  if (CandidateInsts.empty()) {
    return false;
  }

  SmallVector<HLGoto *, 16> Gotos;

  bool IsMultiExit = Lp->getNumExits() > 1;

  // If the loop is not an empty node after the transformation and all the
  // candidate insts are without iv, the code generation will be suppressed
  bool ShouldGenCode = (!IsUpperBoundComplicated && ContainIV) ||
                       (CandidateInsts.size() == Lp->getNumChildren());

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

  if (NeedConvertToStandAloneBlob) {
    UBCE->convertToStandAloneBlobOrConstant();
  }

  OptReportBuilder &ORBuilder =
      Lp->getHLNodeUtils().getHIRFramework().getORBuilder();

  for (auto *HInst : CandidateInsts) {

    unsigned LvalSymbase = HInst->getLvalDDRef()->getSymbase();

    if (IsMultiExit) {
      bool IsLiveOutOfEarlyExit =
          processEarlyExits(Gotos, HInst, Lp, GotoInsertPosition);

      if (!IsLiveOutOfEarlyExit) {
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

      DDRefUtils::replaceIVByCanonExpr(OpRef, LoopLevel, UBCE, HasSignedIV);
      OpRef->makeConsistent(Aux, LoopLevel - 1);
    }

    unsigned HInstLineNum = 0;

    if (HInst->getDebugLoc()) {
      HInstLineNum = HInst->getDebugLoc().getLine();
    }

    // remark: Stmt at line %d sinked after loop using last value computation
    ORBuilder(*Lp).addRemark(OptReportVerbosity::Low,
                             OptRemarkID::StmtSunkAfterLoopLastValue,
                             HInstLineNum);

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

    // For explicit early-exit loops we want to disable this optimization as
    // liveout instructions get inlined into the exit block which is tricky to
    // handle in downstream vectorizer.
    if (DisablePassForMultiExitLoops && Lp->isDoMultiExit())
      continue;

    Result = doLastValueComputation(Lp) || Result;
  }

  return Result;
}

PreservedAnalyses HIRLastValueComputationPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  ModifiedHIR =
      HIRLastValueComputation(HIRF, AM.getResult<HIRDDAnalysisPass>(F)).run();
  return PreservedAnalyses::all();
}