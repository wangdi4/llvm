#if INTEL_COLLAB
//===-- VPOParoptTransform.cpp - Transformation of WRegion for threading --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Authors:
// --------
// Xinmin Tian (xinmin.tian@intel.com)
//
// Major Revisions:
// ----------------
// Dec 2015: Initial Implementation of MT-code generation (Xinmin Tian)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// VPOParoptTransform.cpp implements the interface to outline a work
/// region formed from parallel loop/regions/tasks into a new function,
/// replacing it with a call to the threading runtime call by passing new
/// function pointer to the runtime for parallel execution.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptAtomics.h"
#include "llvm/Transforms/VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/Debug.h"

#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/PredIteratorCache.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"

#include "llvm/PassAnalysisSupport.h"

#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"

#include "llvm/Transforms/Utils/GeneralUtils.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"
#include "llvm/Transforms/Utils/LoopRotationUtils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#endif  // INTEL_CUSTOMIZATION

#include <algorithm>
#include <set>
#include <vector>

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-transform"

#if INTEL_CUSTOMIZATION
// External storage for -loopopt-use-omp-region.
bool llvm::vpo::UseOmpRegionsInLoopoptFlag;
static cl::opt<bool, true> UseOmpRegionsInLoopopt(
    "loopopt-use-omp-region", cl::desc("Handle OpenMP directives in LoopOpt"),
    cl::Hidden, cl::location(UseOmpRegionsInLoopoptFlag), cl::init(false));
#endif  // INTEL_CUSTOMIZATION

//
// Use with the WRNVisitor class (in WRegionUtils.h) to walk the WRGraph
// (DFS) to gather all WRegion Nodes;
//
class VPOWRegionVisitor {

public:
  WRegionListTy &WRNList;
  bool &FoundNeedForTID; // found a WRN that needs TID
  bool &FoundNeedForBID; // found a WRN that needs BID

  VPOWRegionVisitor(WRegionListTy &WL, bool &T, bool &B) :
                    WRNList(WL), FoundNeedForTID(T), FoundNeedForBID(B) {}

  void preVisit(WRegionNode *W) {}

  // use DFS visiting of WRegionNode
  void postVisit(WRegionNode *W) { WRNList.push_back(W);
                                   FoundNeedForTID |= W->needsTID();
                                   FoundNeedForBID |= W->needsBID(); }

  bool quitVisit(WRegionNode *W) { return false; }
};

void VPOParoptTransform::gatherWRegionNodeList(bool &NeedTID, bool &NeedBID) {
  LLVM_DEBUG(dbgs() << "\nSTART: Gather WRegion Node List\n");

  NeedTID = NeedBID = false;
  VPOWRegionVisitor Visitor(WRegionList, NeedTID, NeedBID);
  WRegionUtils::forwardVisit(Visitor, WI->getWRGraph());

  LLVM_DEBUG(dbgs() << "\nEND: Gather WRegion Node List\n");
  return;
}

static void debugPrintHeader(WRegionNode *W, bool IsPrepare) {
  if (IsPrepare)
    LLVM_DEBUG(dbgs() << "\n\n === VPOParopt Prepare: ");
  else
    LLVM_DEBUG(dbgs() << "\n\n === VPOParopt Transform: ");

  LLVM_DEBUG(dbgs() << W->getName().upper() << " construct\n\n");
}

// Generate the placeholders for the loop lower bound and upper bound.
void VPOParoptTransform::genLoopBoundUpdatePrep(
    WRegionNode *W, unsigned Idx, AllocaInst *&LowerBnd, AllocaInst *&UpperBnd,
    AllocaInst *&SchedStride, AllocaInst *&TeamLowerBnd,
    AllocaInst *&TeamUpperBnd, AllocaInst *&TeamStride, Value *&UpperBndVal) {
  Loop *L = W->getWRNLoopInfo().getLoop(Idx);
  assert(L->isLoopSimplifyForm() &&
         "genLoopBoundUpdatePrep: Expect the loop is in SimplifyForm.");

  Type *LoopIndexType = WRegionUtils::getOmpCanonicalInductionVariable(L)
                            ->getIncomingValue(0)
                            ->getType();

  IntegerType *IndValTy = cast<IntegerType>(LoopIndexType);
  assert(IndValTy->getIntegerBitWidth() >= 32 &&
         "Omp loop index type width must be equal or greater than 32 bit");

  Value *InitVal = WRegionUtils::getOmpLoopLowerBound(L);

  Instruction *InsertPt =
      cast<Instruction>(L->getLoopPreheader()->getTerminator());

  IRBuilder<> Builder(InsertPt);
  LowerBnd = Builder.CreateAlloca(IndValTy, nullptr, "lower.bnd");

  UpperBnd = Builder.CreateAlloca(IndValTy, nullptr, "upper.bnd");

  SchedStride = Builder.CreateAlloca(IndValTy, nullptr, "sched.inc");

  bool IsDistParLoop = isa<WRNDistributeParLoopNode>(W);
  bool IsDistForLoop = isa<WRNDistributeNode>(W);

  if ((IsDistParLoop || IsDistForLoop) &&
      // No team partitioning for SPMD mode.
      !VPOParoptUtils::useSPMDMode(W)) {
    TeamLowerBnd = Builder.CreateAlloca(IndValTy, nullptr, "team.lb");

    TeamUpperBnd = Builder.CreateAlloca(IndValTy, nullptr, "team.ub");

    TeamStride = Builder.CreateAlloca(IndValTy, nullptr, "team.inc");
  }

  if (InitVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    InitVal = Builder.CreateSExtOrTrunc(InitVal, IndValTy);

  Builder.CreateStore(InitVal, LowerBnd);

  UpperBndVal =
      VPOParoptUtils::computeOmpUpperBound(W, InsertPt, ".for.update");

  if (UpperBndVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    UpperBndVal = Builder.CreateSExtOrTrunc(UpperBndVal, IndValTy);

  Builder.CreateStore(UpperBndVal, UpperBnd);
}

// Initialize the incoming array Arg with the constant Idx.
void VPOParoptTransform::initArgArray(SmallVectorImpl<Value *> *Arg,
                                      unsigned Idx) {
  LLVMContext &C = F->getContext();

  switch (Idx) {
  case 0: {
    ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);
    Arg->push_back(ValueZero);
  } break;
  case 1: {
    ConstantInt *ValueOne = ConstantInt::get(Type::getInt32Ty(C), 1);
    Arg->push_back(ValueOne);
  } break;
  case 2: {
    ConstantInt *ValueTwo = ConstantInt::get(Type::getInt32Ty(C), 2);
    Arg->push_back(ValueTwo);
  } break;
  }
}

// If the user provides the clause collapse(n), where n is greater
// than 1, the utility getNormIVSize() returns the value n.
// For this case, the compiler simply sets the
// DistSchedKind to be TargetScheduleKind in order
// to simplify the transformation under OpenCL based OMP offloading.
void VPOParoptTransform::
  setSchedKindForMultiLevelLoops(WRegionNode *W,
                                 WRNScheduleKind &SchedKind,
                                 WRNScheduleKind TargetScheduleKind) {
  if (W->getWRNLoopInfo().getNormIVSize() > 1) {
    if (SchedKind != TargetScheduleKind)
      LLVM_DEBUG(dbgs() << "DistSchedKind is forced to be StaticEven."
                        << "\n");
    SchedKind = TargetScheduleKind;
  }
}

// #pragma omp target teams distribute
// for (i = lb; i <= ub; i++)
// The output of loop partitioning for dimension Idx.
// as below, where 0 <= Idx <= 2.
//
// team_chunk_size = (ub - lb + get_num_groups(Idx)) / get_num_groups(Idx);
// new_team_lb = lb + get_group_id(Idx) * team_chunk_size;
// new_team_ub = min(new_team_lb + team_chunk_size - 1, ub);
//
// for (i = new_team_lb; i <= new_team_ub; i += TeamStride)
//
// Idx -- the dimension index.
// LowerBnd -- the stack variable which holds the loop lower bound.
// UpperBnd -- the stack variable which holds the loop upper bound.
void VPOParoptTransform::genOCLDistParLoopBoundUpdateCode(
    WRegionNode *W, unsigned Idx, AllocaInst *LowerBnd, AllocaInst *UpperBnd,
    AllocaInst *TeamLowerBnd, AllocaInst *TeamUpperBnd, AllocaInst *TeamStride,
    WRNScheduleKind &DistSchedKind, Instruction *&TeamLB, Instruction *&TeamUB,
    Instruction *&TeamST) {
  if (!W->getIsDistribute() ||
      // Each iteration of the original loop is executed by a single WI,
      // and we rely on OpenCL paritioning of WIs across WGs.
      // Thus, there is no need to compute the team bounds.
      VPOParoptUtils::useSPMDMode(W))
    return;
  assert(W->getIsOmpLoop() &&
         "genOCLDistParLoopBoundUpdateCode: W is not a loop-type WRN");
  Loop *L = W->getWRNLoopInfo().getLoop(Idx);
  assert(L && "genOCLDistParLoopBoundUpdateCode: Expect non-empty loop.");
  Instruction *InsertPt =
      cast<Instruction>(L->getLoopPreheader()->getTerminator());
  IRBuilder<> Builder(InsertPt);

  // Call the function get_num_group to get the group size for dimension Idx.
  SmallVector<Value *, 3> Arg;
  initArgArray(&Arg, Idx);
  // get_num_groups() returns a value of type size_t by specification,
  // and the return value is greater than 0.
  CallInst *NumGroupsCall =
      VPOParoptUtils::genOCLGenericCall("_Z14get_num_groupsj",
                                        GeneralUtils::getSizeTTy(F),
                                        Arg, InsertPt);
  Value *LB = Builder.CreateLoad(LowerBnd);
  Value *UB = Builder.CreateLoad(UpperBnd);
  // LB and UB have the type of the canonical induction variable.
  auto *ItSpaceType = LB->getType();
  // The subtraction cannot overflow, because the normalized lower bound
  // is a non-negative value.
  Value *ItSpace = Builder.CreateSub(UB, LB);

  DistSchedKind = VPOParoptUtils::getDistLoopScheduleKind(W);

  setSchedKindForMultiLevelLoops(W, DistSchedKind,
                                 WRNScheduleDistributeStaticEven);

  // Compute team_chunk_size
  Value *Chunk = nullptr;
  Value *NumGroups = Builder.CreateZExtOrTrunc(NumGroupsCall, ItSpaceType);
  if (DistSchedKind == WRNScheduleDistributeStaticEven) {
    // FIXME: this add may actually overflow.
    Value *ItSpaceRounded = Builder.CreateAdd(ItSpace, NumGroups);
    Chunk = Builder.CreateSDiv(ItSpaceRounded, NumGroups);
  } else if (DistSchedKind == WRNScheduleDistributeStatic)
    Chunk = W->getDistSchedule().getChunkExpr();
  else
    llvm_unreachable(
        "Unsupported distribute schedule type in OpenCL based offloading!");
  Chunk = Builder.CreateSExtOrTrunc(Chunk, ItSpaceType);
  // FIXME: this multiplication may actually overflow,
  //        if big chunk size is specified in the schedule() clause.
  Value *TeamStrideVal = Builder.CreateMul(NumGroups, Chunk);
  Builder.CreateStore(TeamStrideVal, TeamStride);

  // get_group_id returns a value of type size_t by specification,
  // and the return value is non-negative.
  CallInst *GroupIdCall =
      VPOParoptUtils::genOCLGenericCall("_Z12get_group_idj",
                                        GeneralUtils::getSizeTTy(F),
                                        Arg, InsertPt);
  Value *GroupId = Builder.CreateZExtOrTrunc(GroupIdCall, ItSpaceType);

  // Compute new_team_lb
  // FIXME: this multiplication may actually overflow,
  //        if big chunk size is specified in the schedule() clause.
  Value *LBDiff = Builder.CreateMul(GroupId, Chunk);
  LB = Builder.CreateAdd(LB, LBDiff);
  Builder.CreateStore(LB, LowerBnd);
  Builder.CreateStore(LB, TeamLowerBnd);
  Builder.CreateStore(UB, TeamUpperBnd);

  // Compute new_team_ub
  ConstantInt *ValueOne = ConstantInt::get(cast<IntegerType>(ItSpaceType), 1);
  Value *Ch = Builder.CreateSub(Chunk, ValueOne);
  Value *NewUB = Builder.CreateAdd(LB, Ch);

  Value *Compare = Builder.CreateICmp(ICmpInst::ICMP_ULT, NewUB, UB);
  Instruction *ThenTerm = SplitBlockAndInsertIfThen(
      Compare, InsertPt, false,
      MDBuilder(F->getContext()).createBranchWeights(99999, 100000), DT, LI);
  BasicBlock *ThenBB = ThenTerm->getParent();
  ThenBB->setName("then.bb.");
  IRBuilder<> BuilderThen(ThenBB);
  BuilderThen.SetInsertPoint(ThenBB->getTerminator());
  BuilderThen.CreateStore(NewUB, UpperBnd);
  BuilderThen.CreateStore(NewUB, TeamUpperBnd);

  Builder.SetInsertPoint(InsertPt);
  TeamLB = Builder.CreateLoad(TeamLowerBnd);
  TeamUB = Builder.CreateLoad(TeamUpperBnd);
  TeamST = Builder.CreateLoad(TeamStride);
}

// Generate the OCL loop bound update code.
//
// #pragma omp target teams distribute
// for (i = lb; i <= ub; i++)
// The output of loop partitioning for the Idxth dimension.
// chunk_size = (new_team_ub - new_team_lb + get_local_size(Idx)) /
//              get_local_size(Idx);
// new_lb = new_team_lb + get_local_id(Idx) * chunk_size;
// new_ub = min(new_lb + chunk_size - 1, new_team_ub);
//
// for (i = new_lb; i <= new_ub; i++)
//
// Idx -- the dimension index.
// LowerBnd -- the stack variable which holds the loop lower bound.
// UpperBnd -- the stack variable which holds the loop upper bound.
void VPOParoptTransform::genOCLLoopBoundUpdateCode(WRegionNode *W, unsigned Idx,
                                                   AllocaInst *LowerBnd,
                                                   AllocaInst *UpperBnd,
                                                   AllocaInst *TeamLowerBnd,
                                                   AllocaInst *TeamUpperBnd,
                                                   AllocaInst *SchedStride) {
  assert(W->getIsOmpLoop() && "genOCLLoopBoundUpdateCode: W is not a loop-type WRN");
  assert (LowerBnd->getType() == UpperBnd->getType() &&
          "Expected LowerBnd and UpperBnd to be of same type");
  assert (LowerBnd->getType() == SchedStride->getType() &&
          "Expected LowerBnd and SchedStride to be of same type");

  Loop *L = W->getWRNLoopInfo().getLoop(Idx);
  assert(L && "genOCLLoopBoundUpdateCode: Expect non-empty loop.");
  Instruction *InsertPt =
      cast<Instruction>(L->getLoopPreheader()->getTerminator());
  IRBuilder<> Builder(InsertPt);
  SmallVector<Value *, 3> Arg;
  initArgArray(&Arg, Idx);
  CallInst *LocalSize =
      VPOParoptUtils::genOCLGenericCall("_Z14get_local_sizej",
                                        GeneralUtils::getSizeTTy(F),
                                        Arg, InsertPt);
  assert(((TeamLowerBnd && TeamUpperBnd) ||
          (!TeamLowerBnd && !TeamUpperBnd)) &&
         "genOCLLoopBoundUpdateCode: team lower/upper bounds "
         "must be created together.");

  Value *LB = nullptr;
  Value *UB = nullptr;

  if (TeamLowerBnd) {
    // Update lower/upper bounds from the team bounds.
    LB = Builder.CreateLoad(TeamLowerBnd);
    Builder.CreateStore(LB, LowerBnd);
    UB = Builder.CreateLoad(TeamUpperBnd);
    Builder.CreateStore(UB, UpperBnd);
  }
  else {
    LB = Builder.CreateLoad(LowerBnd);
    UB = Builder.CreateLoad(UpperBnd);
  }

  WRNScheduleKind SchedKind = VPOParoptUtils::getLoopScheduleKind(W);

  setSchedKindForMultiLevelLoops(W, SchedKind, WRNScheduleStaticEven);

  // All operands of math expressions below will be of LBType
  auto LBType = LB->getType();
  Value *NewUB = nullptr;

  if (!VPOParoptUtils::useSPMDMode(W)) {
    Value *Chunk = nullptr;
    Value *NumThreads = Builder.CreateSExtOrTrunc(LocalSize, LBType);
    if (SchedKind == WRNScheduleStaticEven) {
      Value *ItSpace = Builder.CreateSub(UB, LB);
      Value *ItSpaceRounded = Builder.CreateAdd(ItSpace, NumThreads);
      Chunk = Builder.CreateSDiv(ItSpaceRounded, NumThreads);
    } else if (SchedKind == WRNScheduleStatic) {
      Chunk = W->getSchedule().getChunkExpr();
      Chunk = Builder.CreateSExtOrTrunc(Chunk, LBType);
    } else
      llvm_unreachable(
          "Unsupported loop schedule type in OpenCL based offloading!");

    Value *SchedStrideVal = Builder.CreateMul(NumThreads, Chunk);
    Builder.CreateStore(SchedStrideVal, SchedStride);

    CallInst *LocalId =
        VPOParoptUtils::genOCLGenericCall("_Z12get_local_idj",
                                          GeneralUtils::getSizeTTy(F),
                                          Arg, InsertPt);
    Value *LocalIdCasted = Builder.CreateSExtOrTrunc(LocalId, LBType);

    Value *LBDiff = Builder.CreateMul(LocalIdCasted, Chunk);
    LB = Builder.CreateAdd(LB, LBDiff);
    Builder.CreateStore(LB, LowerBnd);

    ConstantInt *ValueOne = ConstantInt::get(cast<IntegerType>(LBType), 1);
    Value *Ch = Builder.CreateSub(Chunk, ValueOne);
    NewUB = Builder.CreateAdd(LB, Ch);
  } else {
    // Use SPMD mode by setting lower and upper bound to get_global_id().
    // This will let each WI execute just one iteration of the loop.
    CallInst *GlobalId =
      VPOParoptUtils::genOCLGenericCall("_Z13get_global_idj",
                                        GeneralUtils::getSizeTTy(F),
                                        Arg, InsertPt);
    Value *GlobalIdCasted = Builder.CreateSExtOrTrunc(GlobalId, LBType);
    Builder.CreateStore(GlobalIdCasted, LowerBnd);
    NewUB = GlobalIdCasted;
  }

  Value *Compare = Builder.CreateICmp(ICmpInst::ICMP_ULT, NewUB, UB);
  Instruction *ThenTerm = SplitBlockAndInsertIfThen(
      Compare, InsertPt, false,
      MDBuilder(F->getContext()).createBranchWeights(99999, 100000), DT, LI);
  BasicBlock *ThenBB = ThenTerm->getParent();
  ThenBB->setName("then.bb.");
  IRBuilder<> BuilderThen(ThenBB);
  BuilderThen.SetInsertPoint(ThenBB->getTerminator());
  BuilderThen.CreateStore(NewUB, UpperBnd);
}

// Generate the GPU loop scheduling code.
void VPOParoptTransform::genOCLLoopPartitionCode(
    WRegionNode *W, unsigned Idx, AllocaInst *LowerBnd, AllocaInst *UpperBnd,
    AllocaInst *SchedStride, AllocaInst *TeamLowerBnd, AllocaInst *TeamUpperBnd,
    AllocaInst *TeamStride, Value *UpperBndVal, WRNScheduleKind DistSchedKind,
    Instruction *TeamLB, Instruction *TeamUB, Instruction *TeamST) {

  assert(W->getIsOmpLoop() && "genOCLLoopPartitionCode: W is not a loop-type WRN");

  Loop *L = W->getWRNLoopInfo().getLoop(Idx);
  assert(L && "genOCLLoopPartitionCode: Expect non-empty loop.");
  DenseMap<Value *, std::pair<Value *, BasicBlock *>> ValueToLiveinMap;
  SmallSetVector<Instruction *, 8> LiveOutVals;
  EquivalenceClasses<Value *> ECs;
  wrnUpdateSSAPreprocess(L, ValueToLiveinMap, LiveOutVals, ECs);

  Instruction *InsertPt =
      cast<Instruction>(L->getLoopPreheader()->getTerminator());
  IRBuilder<> Builder(InsertPt);

  LoadInst *LoadLB = Builder.CreateLoad(LowerBnd);
  LoadInst *LoadUB = Builder.CreateLoad(UpperBnd);

  BasicBlock *StaticInitBB = InsertPt->getParent();

  PHINode *PN = WRegionUtils::getOmpCanonicalInductionVariable(L);
  PN->removeIncomingValue(L->getLoopPreheader());
  PN->addIncoming(LoadLB, L->getLoopPreheader());

  BasicBlock *LoopExitBB = WRegionUtils::getOmpExitBlock(L);

  ICmpInst *CompInst =
      new ICmpInst(InsertPt, ICmpInst::ICMP_SLE, LoadLB, LoadUB, "");

  VPOParoptUtils::updateOmpPredicateAndUpperBound(W, LoadUB, InsertPt);

  BranchInst *PreHdrInst = cast<BranchInst>(InsertPt);
  assert(PreHdrInst->getNumSuccessors() == 1 &&
         "Expect preheader BB has one exit!");

  BasicBlock *LoopRegionExitBB =
      SplitBlock(LoopExitBB, LoopExitBB->getFirstNonPHI(), DT, LI);
  LoopRegionExitBB->setName("loop.region.exit");

  if (LoopExitBB == W->getExitBBlock())
    W->setExitBBlock(LoopRegionExitBB);

  std::swap(LoopExitBB, LoopRegionExitBB);
  Instruction *NewTermInst =
      BranchInst::Create(PreHdrInst->getSuccessor(0), LoopExitBB, CompInst);
  ReplaceInstWithInst(InsertPt, NewTermInst);

  WRNScheduleKind SchedKind = VPOParoptUtils::getLoopScheduleKind(W);

  setSchedKindForMultiLevelLoops(W, SchedKind, WRNScheduleStaticEven);

  if (SchedKind == WRNScheduleStaticEven ||
      // Default to static even scheduling, when we use SPMD mode.
      VPOParoptUtils::useSPMDMode(W)) {
    if (DT)
      DT->changeImmediateDominator(LoopExitBB, StaticInitBB);

    wrnUpdateLiveOutVals(L, LoopRegionExitBB, LiveOutVals, ECs);
    rewriteUsesOfOutInstructions(ValueToLiveinMap, LiveOutVals, ECs);
  } else if (SchedKind == WRNScheduleStatic) {
    // With "teams distribute" the workitem's loop iteration count
    // threshold is the team's upper bound, otherwise, it is
    // the original loop's upper bound.
    Loop *OuterLoop = genDispatchLoopForStatic(
        L, LoadLB, LoadUB, LowerBnd, UpperBnd,
        TeamUB ? TeamUB : UpperBndVal,
        SchedStride, LoopExitBB, StaticInitBB, LoopRegionExitBB);
    wrnUpdateLiveOutVals(OuterLoop, LoopRegionExitBB, LiveOutVals, ECs);
    wrnUpdateSSAPreprocessForOuterLoop(OuterLoop, ValueToLiveinMap, LiveOutVals,
                                       ECs);
    rewriteUsesOfOutInstructions(ValueToLiveinMap, LiveOutVals, ECs);
  } else
    llvm_unreachable(
        "Unsupported loop schedule type in OpenCL based offloading!");

  if (DistSchedKind == WRNScheduleDistributeStatic &&
      // No team paritioning for SPMD mode.
      !VPOParoptUtils::useSPMDMode(W)) {

    Loop *OuterLoop = genDispatchLoopForTeamDistirbute(
        L, TeamLB, TeamUB, TeamST, TeamLowerBnd, TeamUpperBnd, TeamStride,
        UpperBndVal, LoopExitBB, LoopRegionExitBB, TeamST->getParent(),
        LoopExitBB, nullptr);
    wrnUpdateLiveOutVals(OuterLoop, LoopRegionExitBB, LiveOutVals, ECs);
    wrnUpdateSSAPreprocessForOuterLoop(OuterLoop, ValueToLiveinMap, LiveOutVals,
                                       ECs);
    rewriteUsesOfOutInstructions(ValueToLiveinMap, LiveOutVals, ECs);
  }
}

// Generate dispatch loop for teams distriubte.
Loop *VPOParoptTransform::genDispatchLoopForTeamDistirbute(
    Loop *L, Instruction *TeamLB, Instruction *TeamUB, Instruction *TeamST,
    AllocaInst *TeamLowerBnd, AllocaInst *TeamUpperBnd, AllocaInst *TeamStride,
    Value *UpperBndVal, BasicBlock *LoopExitBB, BasicBlock *LoopRegionExitBB,
    BasicBlock *TeamInitBB, BasicBlock *TeamExitBB,
    Instruction *TeamExitBBSplit) {

  //                          |
  //                team.dispatch.header <---------------+
  //                       |       |                     |
  //                       |   team.dispatch.min.ub      |
  //                       |       |                     |
  //   +------------- team.dispatch.body                 |
  //   |                   |                             |
  //   |                   |                             |
  //   |           team.dispatch.inner.body              |
  //   |                      |                          |
  //   |                  par loop  <-------+            |
  //   |                      |             |            |
  //   |                    .....           |            |
  //   |                      |             |            |
  //   |            par loop bottom test ---+            |
  //   |                      |                          |
  //   |                      |                          |
  //   |               team.dispatch.inc                 |
  //   |                      |                          |
  //   |                      +--------------------------+
  //   |
  //   +------------> team dispatch.latch
  //                          |

  // Generate dispatch header BBlock
  BasicBlock *TeamDispHeaderBB = SplitBlock(TeamInitBB, TeamLB, DT, LI);
  TeamDispHeaderBB->setName("team.dispatch.header");

  // Generate a upper bound load instruction at top of TeamDispHeaderBB
  LoadInst *TmpUB = new LoadInst(TeamUpperBnd, "team.ub.tmp", TeamLB);

  // Generate a upper bound load instruction at top of TeamDispHeaderBB
  Value *TmpUD = UpperBndVal;

  BasicBlock *TeamDispBodyBB = SplitBlock(TeamDispHeaderBB, TeamLB, DT, LI);
  TeamDispBodyBB->setName("team.dispatch.body");

  ICmpInst *MinUB;

  Instruction *TermInst = TeamDispHeaderBB->getTerminator();

  MinUB =
      new ICmpInst(TermInst, ICmpInst::ICMP_SLE, TmpUB, TmpUD, "team.ub.min");

  StoreInst *NewUB = new StoreInst(TmpUD, TeamUpperBnd, false, TermInst);

  BasicBlock *TeamDispMinUBB = SplitBlock(TeamDispHeaderBB, NewUB, DT, LI);
  TeamDispMinUBB->setName("team.dispatch.min.ub");

  TermInst = TeamDispHeaderBB->getTerminator();

  // Generate branch for team.dispatch.cond for get MIN upper bound
  Instruction *NewTermInst =
      BranchInst::Create(TeamDispBodyBB, TeamDispMinUBB, MinUB);
  ReplaceInstWithInst(TermInst, NewTermInst);

  BasicBlock *TeamInnerBodyBB = SplitBlock(TeamDispBodyBB, TeamST, DT, LI);
  TeamInnerBodyBB->setName("team.dispatch.inner.body");

  ICmpInst *TeamTopTest;

  TermInst = TeamDispBodyBB->getTerminator();

  TeamTopTest = new ICmpInst(TermInst, ICmpInst::ICMP_SLE, TeamLB, TeamUB,
                             "team.top.test");

  // Generate branch for team.dispatch.cond for get MIN upper bound
  Instruction *TeamTopTestBI =
      BranchInst::Create(TeamInnerBodyBB, TeamExitBB, TeamTopTest);
  ReplaceInstWithInst(TermInst, TeamTopTestBI);

  // Generate dispatch chunk increment BBlock
  Instruction *SplitPoint =
      TeamExitBBSplit ?
          TeamExitBBSplit->getNextNonDebugInstruction() : &TeamExitBB->front();

  BasicBlock *TeamDispLatchBB = SplitBlock(TeamExitBB, SplitPoint, DT, LI);

  TermInst = TeamExitBB->getTerminator();
  TeamExitBB->setName("team.dispatch.inc");
  IRBuilder<> Builder(TermInst);

  LoadInst *TeamStrideVal =
      Builder.CreateLoad(TeamStride, false, "team.st.inc");

  // Generate team.inc.lb = team.new.lb + team.new.st
  BinaryOperator *IncLB =
      BinaryOperator::CreateAdd(TeamLB, TeamStrideVal, "team.inc.lb");
  IncLB->insertBefore(TermInst);

  // Generate team.inc.ub = team.new.lb + team.new.st
  BinaryOperator *IncUB =
      BinaryOperator::CreateAdd(TeamUB, TeamStrideVal, "team.inc.ub");
  IncUB->insertBefore(TermInst);

  StoreInst *NewIncLB = new StoreInst(IncLB, TeamLowerBnd, false, TermInst);
  NewIncLB->setAlignment(4);

  StoreInst *NewIncUB = new StoreInst(IncUB, TeamUpperBnd, false, TermInst);
  NewIncUB->setAlignment(4);

  TermInst->setSuccessor(0, TeamDispHeaderBB);

  TeamDispLatchBB->setName("team.dispatch.latch");

  TermInst = TeamDispBodyBB->getTerminator();

  TermInst->setSuccessor(1, TeamDispLatchBB);

  if (DT) {
    DT->changeImmediateDominator(TeamDispHeaderBB, TeamInitBB);
    DT->changeImmediateDominator(TeamDispBodyBB, TeamDispHeaderBB);
    DT->changeImmediateDominator(TeamDispMinUBB, TeamDispHeaderBB);
    DT->changeImmediateDominator(TeamInnerBodyBB, TeamDispBodyBB);
    DT->changeImmediateDominator(TeamDispLatchBB, TeamDispBodyBB);
  }
  Loop *OuterLoop = WRegionUtils::createLoop(L, L->getParentLoop(), LI);
  WRegionUtils::updateBBForLoop(TeamDispHeaderBB, OuterLoop, L->getParentLoop(),
                                LI);
  WRegionUtils::updateBBForLoop(TeamDispMinUBB, OuterLoop, L->getParentLoop(),
                                LI);
  WRegionUtils::updateBBForLoop(TeamDispBodyBB, OuterLoop, L->getParentLoop(),
                                LI);
  WRegionUtils::updateBBForLoop(TeamExitBB, OuterLoop, L->getParentLoop(), LI);
  WRegionUtils::updateBBForLoop(LoopRegionExitBB, OuterLoop, L->getParentLoop(),
                                LI);
  OuterLoop->moveToHeader(TeamDispHeaderBB);
  return OuterLoop;
}

// Generate dispatch loop for static chunk.
// FIXME: get rid of LoopExitBB and StaticInitBB parameters.
Loop *VPOParoptTransform::genDispatchLoopForStatic(
    Loop *L, LoadInst *LoadLB, LoadInst *LoadUB, AllocaInst *LowerBnd,
    AllocaInst *UpperBnd, Value *UpperBndVal, AllocaInst *SchedStride,
    BasicBlock *LoopExitBB, BasicBlock *StaticInitBB,
    BasicBlock *LoopRegionExitBB) {
  //                          |
  //                    dispatch.header <----------------+
  //                       |       |                     |
  //                       |   dispatch.min.ub           |
  //                       |       |                     |
  //   +---------------- dispatch.body                   |
  //   |                      |                          |
  //   |                  loop body <------+             |
  //   |                      |            |             |
  //   |                    .....          |             |
  //   |                      |            |             |
  //   |               loop bottom test ---+             |
  //   |                      |                          |
  //   |                      |                          |
  //   |                dispatch.inc                     |
  //   |                      |                          |
  //   |                      +--------------------------+
  //   |
  //   +--------------> dispatch.latch
  //                          |

  // LoadLB is a load instruction producing a value for ZTT generated
  // around the loop during lowering. Split its block so that
  // both LoadLB and LoadUB and the ZTT are in a new "dispatch" block.
  StaticInitBB = LoadLB->getParent();

  // Generate dispatch header BBlock
  BasicBlock *DispatchHeaderBB = SplitBlock(StaticInitBB, LoadLB, DT, LI);
  DispatchHeaderBB->setName("dispatch.header");

  // Generate a upper bound load instruction at top of DispatchHeaderBB
  LoadInst *TmpUB = new LoadInst(UpperBnd, "ub.tmp", LoadLB);

  BasicBlock *DispatchBodyBB = SplitBlock(DispatchHeaderBB, LoadLB, DT, LI);
  DispatchBodyBB->setName("dispatch.body");

  Instruction *TermInst = DispatchHeaderBB->getTerminator();

  ICmpInst *MinUB =
      new ICmpInst(TermInst, ICmpInst::ICMP_SLE, TmpUB, UpperBndVal, "ub.min");

  StoreInst *NewUB = new StoreInst(UpperBndVal, UpperBnd, false, TermInst);

  BasicBlock *DispatchMinUBB = SplitBlock(DispatchHeaderBB, NewUB, DT, LI);
  DispatchMinUBB->setName("dispatch.min.ub");

  TermInst = DispatchHeaderBB->getTerminator();

  // Generate branch for dispatch.cond for get MIN upper bound
  Instruction *NewTermInst =
      BranchInst::Create(DispatchBodyBB, DispatchMinUBB, MinUB);
  ReplaceInstWithInst(TermInst, NewTermInst);

  // Generate dispatch chunk increment BBlock
  BasicBlock *DispatchLatchBB =
      SplitBlock(LoopRegionExitBB, LoopRegionExitBB->getTerminator(), DT, LI);

  TermInst = LoopRegionExitBB->getTerminator();
  LoopRegionExitBB->setName("dispatch.inc");
  IRBuilder<> Builder(TermInst);

  // Load Stride value to st.new
  LoadInst *StrideVal = Builder.CreateLoad(SchedStride, false, "st.inc");

  // Generate inc.lb.new = lb.new + st.new
  auto IncLB =
      Builder.CreateAdd(Builder.CreateLoad(LowerBnd), StrideVal, "lb.inc");

  // Generate inc.lb.new = lb.new + st.new
  auto IncUB =
      Builder.CreateAdd(Builder.CreateLoad(UpperBnd), StrideVal, "ub.inc");

  Builder.CreateStore(IncLB, LowerBnd);
  Builder.CreateStore(IncUB, UpperBnd);

  TermInst->setSuccessor(0, DispatchHeaderBB);

  DispatchLatchBB->setName("dispatch.latch");

  // DispatchBodyBB ends with the ZTT that was originally in StaticInitBB.
  // This ZTT controls a branch to either the loop body, or the LoopExitBB.
  // Reroute the LoopExitBB edge to DispatchLatchBB.
  // Note that DispatchLatchBB now becomes the only predecessor of LoopExitBB.
  TermInst = DispatchBodyBB->getTerminator();
  TermInst->setSuccessor(1, DispatchLatchBB);

  if (DT) {
    // Some of the new blocks have correct immediate dominators set
    // by SplitBlock.  We only need to correct it for some.
    DT->changeImmediateDominator(DispatchHeaderBB, StaticInitBB);
    DT->changeImmediateDominator(DispatchBodyBB, DispatchHeaderBB);
    DT->changeImmediateDominator(DispatchLatchBB, DispatchBodyBB);
  }

  Loop *OuterLoop = WRegionUtils::createLoop(L, L->getParentLoop(), LI);
  WRegionUtils::updateBBForLoop(DispatchHeaderBB, OuterLoop, L->getParentLoop(),
                                LI);
  WRegionUtils::updateBBForLoop(DispatchMinUBB, OuterLoop, L->getParentLoop(),
                                LI);
  WRegionUtils::updateBBForLoop(DispatchBodyBB, OuterLoop, L->getParentLoop(),
                                LI);
  WRegionUtils::updateBBForLoop(LoopRegionExitBB, OuterLoop, L->getParentLoop(),
                                LI);
  OuterLoop->moveToHeader(DispatchHeaderBB);

  return OuterLoop;
}

// Generate the iteration space partitioning code based on OpenCL.
// Given a loop as follows.
//   for (i = lb; i <= ub; i++)
// The output of partitioning as below.
//   chunk_size = (ub - lb + get_local_size()) / get_local_size();
//   new_lb = lb + get_local_id * chunk_size;
//   new_ub = min(new_lb + chunk_size - 1, ub);
//   for (i = new_lb; i <= new_ub; i++)
// Here we assume the global_size is equal to local_size, which means
// there is only one workgroup.
//
bool VPOParoptTransform::genOCLParallelLoop(WRegionNode *W) {

  AllocaInst *LowerBnd, *UpperBnd, *SchedStride;
  AllocaInst *TeamLowerBnd, *TeamUpperBnd, *TeamStride;
  Value *UpperBndVal;
  WRNScheduleKind DistSchedKind;
  Instruction *TeamLB, *TeamUB, *TeamST;

  LowerBnd = nullptr;
  UpperBnd = nullptr;
  SchedStride = nullptr;
  TeamLowerBnd = nullptr;
  TeamUpperBnd = nullptr;
  TeamStride = nullptr;
  UpperBndVal = nullptr;
  TeamLB = nullptr;
  TeamUB = nullptr;
  TeamST = nullptr;

  for (unsigned I = W->getWRNLoopInfo().getNormIVSize(); I > 0; --I) {
    genLoopBoundUpdatePrep(W, I - 1, LowerBnd, UpperBnd, SchedStride,
                           TeamLowerBnd, TeamUpperBnd, TeamStride, UpperBndVal);

    genOCLDistParLoopBoundUpdateCode(W, I - 1, LowerBnd, UpperBnd, TeamLowerBnd,
                                     TeamUpperBnd, TeamStride, DistSchedKind,
                                     TeamLB, TeamUB, TeamST);

    if (isa<WRNParallelSectionsNode>(W) || isa<WRNParallelLoopNode>(W) ||
        isa<WRNDistributeParLoopNode>(W))
      genOCLLoopBoundUpdateCode(W, I - 1, LowerBnd, UpperBnd,
                                TeamLowerBnd, TeamUpperBnd, SchedStride);

    genOCLLoopPartitionCode(W, I - 1, LowerBnd, UpperBnd, SchedStride,
                            TeamLowerBnd, TeamUpperBnd, TeamStride, UpperBndVal,
                            DistSchedKind, TeamLB, TeamUB, TeamST);
  }

  W->resetBBSet(); // CFG changed; clear BBSet
  return true;
}

bool VPOParoptTransform::renameAndReplaceLibatomicCallsForSPIRV(Function *F) {
  bool Changed = false;
  for (inst_iterator II = inst_begin(F), E = inst_end(F); II != E; ++II) {
    Instruction *I = &*II;
    if (!isa<CallInst>(I))
      continue;

    auto *CI = cast<CallInst>(I);
    Function *CF = CI->getCalledFunction();
    if (!CF || !CF->hasName())
      continue;

    StringRef FName = CF->getName();
    if (FName != "__atomic_load" && FName != "__atomic_store" &&
        FName != "__atomic_compare_exchange")
      continue;

    const auto &Attributes = CF->getAttributes();
    Module *M = F->getParent();
    IRBuilder<> Builder(CI);

    Type *PtrTy =
        PointerType::get(Builder.getInt8Ty(), 4 /*ADDRESS_SPACE_GENERIC*/);
    Type *VoidTy = Builder.getVoidTy();
    Type *I32Ty = Builder.getInt32Ty();
    Type *I64Ty = Builder.getInt64Ty();
    Type *I1Ty = Builder.getInt1Ty();

    auto castArgumentToAddressSpaceGeneric = [&Builder, &PtrTy,
                                              &CI](unsigned Idx) {
      CI->setArgOperand(Idx, Builder.CreatePointerBitCastOrAddrSpaceCast(
                                 CI->getArgOperand(Idx), PtrTy));
    };

    auto castArgumentToI64 = [&Builder, &I64Ty, &CI](unsigned Idx) {
      CI->setArgOperand(
          Idx, Builder.CreateIntCast(CI->getArgOperand(Idx), I64Ty, false));
    };

    FunctionCallee NewFC;
    if (FName == "__atomic_load") {
      NewFC = M->getOrInsertFunction("__kmpc_atomic_load", Attributes, VoidTy,
                                     I64Ty, PtrTy, PtrTy, I32Ty);
      CI->setCalledFunction(NewFC);
    } else if (FName == "__atomic_store") {
      NewFC = M->getOrInsertFunction("__kmpc_atomic_store", Attributes, VoidTy,
                                     I64Ty, PtrTy, PtrTy, I32Ty);
      CI->setCalledFunction(NewFC);
    } else if (FName == "__atomic_compare_exchange") {
      NewFC = M->getOrInsertFunction("__kmpc_atomic_compare_exchange",
                                     Attributes, I1Ty, I64Ty, PtrTy, PtrTy,
                                     PtrTy, I32Ty, I32Ty);
      CI->setCalledFunction(NewFC);
      castArgumentToAddressSpaceGeneric(3);
    } else
      llvm_unreachable("Unexpected function name");

    castArgumentToI64(0); // Needed as it is I32 for __atomic* for spir target
    castArgumentToAddressSpaceGeneric(1);
    castArgumentToAddressSpaceGeneric(2);

    cast<Function>(NewFC.getCallee())->setDSOLocal(true);
    Changed = true;
  }

  return Changed;
}

// Generate this code:
//
//   %temp = alloca i1                                                  (1)
//   %0 = llvm.region.entry()[... "QUAL.OMP.JUMP.TO.END.IF" (i1* %temp) (2)
//   %temp.load = load volatile i1, i1* %temp                           (3)
//   %cmp = icmp ne i1 %temp.load, false                                (4)
//   br i1 %cmp, label %REGION.END, label %REGION.BODY                  (5)
//
//   REGION.BODY:
//   ...
//
//   REGION.END:
//   llvm.region.exit(%0)
void VPOParoptTransform::addBranchToEndDirective(WRegionNode *W) {
  assert(W && "Null WRegionNode.");
  Instruction *EntryDirective = W->getEntryDirective();

  assert(EntryDirective->hasOneUse() &&
         "More than one uses of region entry directive.");
  assert(GeneralUtils::hasNextUniqueInstruction(EntryDirective) &&
         "More than one successors of region entry directive.");

  Instruction *InsertPt = GeneralUtils::nextUniqueInstruction(EntryDirective);

  Instruction *ExitDirective =
      cast<Instruction>(*(EntryDirective->user_begin()));
  BasicBlock *ExitBB = ExitDirective->getParent();
  BasicBlock *NewExitBB = SplitBlock(ExitBB, ExitDirective, DT, LI);

  IRBuilder<> AllocaBuilder(EntryDirective);
  AllocaInst *TempAddr = AllocaBuilder.CreateAlloca(
      AllocaBuilder.getInt1Ty(), nullptr, "end.dir.temp"); //           (1)

  IRBuilder<> Builder(InsertPt);
  Value *GlobLoad = Builder.CreateLoad(TempAddr, true, "temp.load"); // (3)
  Value *CmpInst =
      Builder.CreateICmpNE(GlobLoad, Builder.getInt1(0), "cmp"); //     (4)

  SplitBlockAndInsertIfThen(CmpInst, InsertPt, false, nullptr, nullptr, nullptr,
                            NewExitBB); //                              (5)

  StringRef ClauseString =
      VPOAnalysisUtils::getClauseString(QUAL_OMP_JUMP_TO_END_IF);

  CallInst *CI = cast<CallInst>(EntryDirective);
  CI = VPOParoptUtils::addOperandBundlesInCall(
      CI, {{ClauseString, {TempAddr}}}); //                             (2)
  W->setEntryDirective(CI);

  LLVM_DEBUG(dbgs() << __FUNCTION__
                    << ": Created a branch from region.entry to region.exit "
                       "for WRegion '"
                    << W->getNumber() << "', using '";
             TempAddr->printAsOperand(dbgs()); dbgs() << "'.\n";);

  return;
}

//
// ParPrepare mode:
//   Paropt prepare transformations for lowering and privatizing
//
// ParTrans mode:
//   Paropt transformations for loop partitioning and outlining
//
bool VPOParoptTransform::paroptTransforms() {
  LLVMContext &C = F->getContext();
  bool RoutineChanged = false;

#if INTEL_CUSTOMIZATION
  // Following two variables are used when generating remarks using
  // Loop Opt Report framework (under -qopt-report).
  LoopInfo *ORLinfo = nullptr;
  Loop *ORLoop = nullptr;
#endif  // INTEL_CUSTOMIZATION

  IdentTy = VPOParoptUtils::getIdentStructType(F);

  StringRef S = F->getName();

  if (!S.compare_lower(StringRef("@main"))) {
    BasicBlock::iterator I = F->getEntryBlock().begin();
    CallInst *RI = VPOParoptUtils::genKmpcBeginCall(F, &*I, IdentTy);
    RI->insertBefore(&*I);

    for (BasicBlock &BB : *F) {
      if (isa<ReturnInst>(BB.getTerminator())) {
        Instruction *Inst = BB.getTerminator();

        CallInst *RI = VPOParoptUtils::genKmpcEndCall(F, Inst, IdentTy);
        RI->insertBefore(Inst);
      }
    }
  }

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  // Lower OpenMP runtime library calls for CSA in prepare pass.
  if (isTargetCSA() && (Mode & ParPrepare))
    RoutineChanged |= translateCSAOmpRtlCalls();
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION

  if (WI->WRGraphIsEmpty()) {
    LLVM_DEBUG(
        dbgs() << "\n... No WRegion Candidates for Parallelization ...\n\n");
    return RoutineChanged;
  }

  bool IsTargetSPIRV = VPOAnalysisUtils::isTargetSPIRV(F->getParent()) &&
                       hasOffloadCompilation();
  bool NeedTID, NeedBID;

  LLVM_DEBUG(dbgs()<<"\n In transform "<<IsTargetSPIRV <<" Dump the function ::"<<*F);
  if (IsTargetSPIRV && (Mode & ParPrepare))
    RoutineChanged |= renameAndReplaceLibatomicCallsForSPIRV(F);

  // Collects the list of WRNs into WRegionList, and sets NeedTID and NeedBID
  // to true/false depending on whether it finds a WRN that needs the TID or
  // BID, respectively.
  gatherWRegionNodeList(NeedTID, NeedBID);

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  if (isTargetCSA()) {
    NeedTID = false;
    NeedBID = false;
  }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION

  Type *Int32Ty = Type::getInt32Ty(C);

  if (NeedTID)
    TidPtrHolder = F->getParent()->getOrInsertGlobal("@tid.addr", Int32Ty);

  if (NeedBID && (Mode & OmpPar) && (Mode & ParTrans))
    BidPtrHolder = F->getParent()->getOrInsertGlobal("@bid.addr", Int32Ty);

  //
  // Walk through W-Region list, the outlining / lowering is performed from
  // inner to outer
  //
  for (auto I = WRegionList.begin(), E = WRegionList.end(); I != E; ++I) {

    WRegionNode *W = *I;

    assert(W->isBBSetEmpty() &&
           "WRNs should not have BBSET populated initially");

    // Init 'Changed' to false before processing W;
    // If W is transformed, set 'Changed' to true.
    bool Changed = false;

    bool RemoveDirectives = false;
    bool HandledWithoutRemovingDirectives = false;

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
    if (isTargetCSA() && !isSupportedOnCSA(W)) {
      if (Mode & ParPrepare)
        switch (W->getWRegionKindID()) {
          case WRegionNode::WRNAtomic:
          case WRegionNode::WRNCritical:
          case WRegionNode::WRNTaskwait:
            Changed |= removeCompilerGeneratedFences(W);
            break;
        }
      RemoveDirectives = true;
    }
    else
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
    if (W->getIsOmpLoop() && !W->getIsSections()
        &&  W->getWRNLoopInfo().getLoop()==nullptr) {
      // The WRN is a loop-type construct, but the loop is missing, most likely
      // because it has been optimized away. We skip the code transforms for
      // this WRN, and simply remove its directives.
      RemoveDirectives = true;
    }
    else {
      bool IsPrepare = Mode & ParPrepare;

      switch (W->getWRegionKindID()) {

      // 1. Constructs that need to perform outlining:
      //      Parallel [for|sections], task, taskloop, etc.

      case WRegionNode::WRNTeams:
      case WRegionNode::WRNParallel:
        debugPrintHeader(W, IsPrepare);
        if (Mode & ParPrepare) {
          Changed |= canonicalizeGlobalVariableReferences(W);
          Changed |= renameOperandsUsingStoreThenLoad(W);
          Changed |= propagateCancellationPointsToIR(W);
        }
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed |= clearCancellationPointAllocasFromIR(W);
          WRegionUtils::collectNonPointerValuesToBeUsedInOutlinedRegion(W);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
          if (isTargetCSA()) {
            improveAliasForOutlinedFunc(W);
            if (W->getIsPar())
              Changed |= genCSAParallel(W);
            else
              llvm_unreachable("Unexpected work region kind");
            RemoveDirectives = true;
            break;
          }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          if (!IsTargetSPIRV) {
#if INTEL_CUSTOMIZATION
            improveAliasForOutlinedFunc(W);
#endif  // INTEL_CUSTOMIZATION
            // Privatization is enabled for Transform pass
            Changed |= genPrivatizationCode(W);
            Changed |= genFirstPrivatizationCode(W);
            Changed |= genReductionCode(W);
            Changed |= genCancellationBranchingCode(W);
            Changed |= genDestructorCode(W);
            Changed |= captureAndAddCollectedNonPointerValuesToSharedClause(W);
            Changed |= genMultiThreadedCode(W);

            RemoveDirectives = true;
          } else {
            // The directive gets removed, when processing the target region,
            // do not remove it here, since guardSideEffects needs the
            // parallel directive to insert barriers.
            RemoveDirectives = false;
            HandledWithoutRemovingDirectives = true;
          }

          LLVM_DEBUG(dbgs()<<"\n Parallel W-Region::"<<*W->getEntryBBlock());
        }
        break;
      case WRegionNode::WRNParallelSections:
      case WRegionNode::WRNParallelLoop:
      case WRegionNode::WRNDistributeParLoop:
        debugPrintHeader(W, IsPrepare);
        if (Mode & ParPrepare) {
          if (W->getIsParSections())
            Changed = addNormUBsToParents(W);

          Changed |= regularizeOMPLoop(W);
          Changed |= canonicalizeGlobalVariableReferences(W);
          Changed |= renameOperandsUsingStoreThenLoad(W);
          Changed |= propagateCancellationPointsToIR(W);
        }
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed |= clearCancellationPointAllocasFromIR(W);
          WRegionUtils::collectNonPointerValuesToBeUsedInOutlinedRegion(W);
          Changed |= regularizeOMPLoop(W, false);
#if INTEL_CUSTOMIZATION
          improveAliasForOutlinedFunc(W);
#endif  // INTEL_CUSTOMIZATION

          // For the case of target parallel for (OpenCL), the compiler
          // constructs a loop parameter before the target region.
          Changed |= constructNDRangeInfo(W);

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
          if (isTargetCSA()) {
            // Generate remarks using Loop Opt Report framework
            // (under -qopt-report).
            if (isa<WRNParallelLoopNode>(W)) {
              ORLinfo = W->getWRNLoopInfo().getLoopInfo();
              ORLoop = W->getWRNLoopInfo().getLoop();
              if (ORLoop != nullptr)
                LORBuilder(*ORLoop, *ORLinfo).addRemark(OptReportVerbosity::Low,
                           "CSA: OpenMP parallel loop will be pipelined");
            }

            if (W->getIsParSections()) {
              Changed |= genCSASections(W);
              RemoveDirectives = true;
            }
            else if (W->getIsParLoop()) {
              auto Res = genCSALoop(W);
              Changed |= Res.first;
              RemoveDirectives = Res.second;
            }
            else
              llvm_unreachable("Unexpected work region kind");
            break;
          }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          // The compiler does not need to generate the outlined function
          // for omp parallel for loop.
          if (IsTargetSPIRV) {
            Changed |= genOCLParallelLoop(W);
            Changed |= genPrivatizationCode(W);
            Changed |= genReductionCode(W);
          } else {
#if INTEL_CUSTOMIZATION
            // Generate remarks using Loop Opt Report framework (under -qopt-report).
            if (isa<WRNParallelLoopNode>(W)) {
               ORLinfo = W->getWRNLoopInfo().getLoopInfo();
               ORLoop = W->getWRNLoopInfo().getLoop();
               if (ORLoop != nullptr) {
                   LORBuilder(*ORLoop, *ORLinfo).addRemark(OptReportVerbosity::Low,
                              "OpenMP: Outlined parallel loop");

                   // Add remark to enclosing loop (if any).
                   if (ORLoop->getParentLoop() != nullptr)
                      // An enclosing loop is present.
                      LORBuilder(*(ORLoop->getParentLoop()), *ORLinfo).addRemark(OptReportVerbosity::Low,
                                 "OpenMP: Parallel loop was outlined");
                   }
               }
#endif  // INTEL_CUSTOMIZATION

            Instruction *InsertLastIterCheckBefore = nullptr;
            Instruction *OMPLBForLinearClosedForm = nullptr;
            AllocaInst *IsLastVal = nullptr;
            BasicBlock *IfLastIterBB = nullptr;
            Instruction *OMPZtt = nullptr;
            Changed |=
                genLoopSchedulingCode(W, IsLastVal, InsertLastIterCheckBefore,
                                      OMPLBForLinearClosedForm, OMPZtt);
            // Privatization is enabled for Transform pass
            Changed |= genPrivatizationCode(W);
            Changed |= genLastIterationCheck(W, IsLastVal, IfLastIterBB,
                                             InsertLastIterCheckBefore);
            Changed |= genLinearCode(W, IfLastIterBB, OMPLBForLinearClosedForm);
            // Must be in this order, if vars are both FP and LP
            Changed |= genLastPrivatizationCode(
                W, IfLastIterBB, OMPLBForLinearClosedForm,
                InsertLastIterCheckBefore, OMPZtt);
            Changed |= genFirstPrivatizationCode(W);
            Changed |= genReductionCode(W);
            Changed |= genCancellationBranchingCode(W);
            Changed |= genDestructorCode(W);
            Changed |= captureAndAddCollectedNonPointerValuesToSharedClause(W);
            Changed |= genMultiThreadedCode(W);
          }
          Changed |= sinkSIMDDirectives(W);
          RemoveDirectives = true;

          LLVM_DEBUG(dbgs()<<"\n Parallel W-Region::"<<*W->getEntryBBlock());

          if (IsTargetSPIRV) {
            // The directive gets removed, when processing the target region,
            // do not remove it here, since guardSideEffects needs the
            // parallel directive to insert barriers.
            RemoveDirectives = false;
            HandledWithoutRemovingDirectives = true;
          }
        }
        break;
      case WRegionNode::WRNTask:
        if (Mode & ParPrepare) {
          Changed |= canonicalizeGlobalVariableReferences(W);
          Changed |= renameOperandsUsingStoreThenLoad(W);
          Changed |= propagateCancellationPointsToIR(W);
        }
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed |= clearCancellationPointAllocasFromIR(W);
          debugPrintHeader(W, false);
#if INTEL_CUSTOMIZATION
          improveAliasForOutlinedFunc(W);
#endif  // INTEL_CUSTOMIZATION
          StructType *KmpTaskTTWithPrivatesTy;
          StructType *KmpSharedTy;
          Value *LastIterGep;
          BasicBlock *IfLastIterBB = nullptr;
          Changed = genTaskInitCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy,
                                    LastIterGep);
          Changed |= genPrivatizationCode(W);
          Changed |= genFirstPrivatizationCode(W);
          Changed |= genBarrierForFpLpAndLinears(W);
          Changed |= genLastIterationCheck(W, LastIterGep, IfLastIterBB);
          Changed |= genLastPrivatizationCode(W, IfLastIterBB);
          Changed |= genSharedCodeForTaskGeneric(W);
          Changed |= genRedCodeForTaskGeneric(W);
          Changed |= genCancellationBranchingCode(W);
          Changed |= genTaskCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNTaskloop:
        debugPrintHeader(W, IsPrepare);
        if (Mode & ParPrepare) {
          Changed = regularizeOMPLoop(W);
          Changed |= canonicalizeGlobalVariableReferences(W);
          Changed |= renameOperandsUsingStoreThenLoad(W);
        }
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed |= regularizeOMPLoop(W, false);
#if INTEL_CUSTOMIZATION
          improveAliasForOutlinedFunc(W);
#endif  // INTEL_CUSTOMIZATION
          StructType *KmpTaskTTWithPrivatesTy;
          StructType *KmpSharedTy;
          Value *LBPtr, *UBPtr, *STPtr, *LastIterGep;
          BasicBlock *IfLastIterBB = nullptr;
          Changed |=
              genTaskLoopInitCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy,
                                  LBPtr, UBPtr, STPtr, LastIterGep);
          Changed |= genPrivatizationCode(W);
          Changed |= genLastIterationCheck(W, LastIterGep, IfLastIterBB);
          Changed |= genLastPrivatizationCode(W, IfLastIterBB);
          Changed |= genFirstPrivatizationCode(W);
          Changed |= genSharedCodeForTaskGeneric(W);
          Changed |= genRedCodeForTaskGeneric(W);
          Changed |= genTaskGenericCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy,
                                        LBPtr, UBPtr, STPtr);
          Changed |= sinkSIMDDirectives(W);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNTaskwait:
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          debugPrintHeader(W, false);
          Changed |= genTaskWaitCode(W);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNTarget:
        if (DisableOffload) {
          // Ignore TARGET construct
          LLVM_DEBUG(dbgs()<<"VPO: Ignored " << W->getName() << " construct\n");
          RemoveDirectives = true;
          break;
        }
        debugPrintHeader(W, IsPrepare);
        if (Mode & ParPrepare) {
          // Override function linkage for the target compilation to prevent
          // functions with target regions from being deleted by LTO.
          if (hasOffloadCompilation())
            F->setLinkage(GlobalValue::LinkageTypes::ExternalLinkage);
          Changed |= canonicalizeGlobalVariableReferences(W);
          Changed |= renameOperandsUsingStoreThenLoad(W);
        } else if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed |= promoteClauseArgumentUses(W);
          // The purpose is to generate place holder for global variable.
          Changed |= genGlobalPrivatizationLaunderIntrin(W);
          WRegionUtils::collectNonPointerValuesToBeUsedInOutlinedRegion(W);
#if INTEL_CUSTOMIZATION
          improveAliasForOutlinedFunc(W);
#endif  // INTEL_CUSTOMIZATION
          Changed |= genPrivatizationCode(W);
          Changed |= genFirstPrivatizationCode(W);
          Changed |= captureAndAddCollectedNonPointerValuesToSharedClause(W);
          Changed |= genTargetOffloadingCode(W);
          Changed |= clearLaunderIntrinBeforeRegion(W);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNTargetEnterData:
      case WRegionNode::WRNTargetExitData:
      case WRegionNode::WRNTargetUpdate:
        if (DisableOffload) {
          // Ignore TARGET UPDATE and TARGET ENTER/EXIT DATA constructs
          LLVM_DEBUG(dbgs()<<"VPO: Ignored " << W->getName() << " construct\n");
          RemoveDirectives = true;
          break;
        }
        // These constructs do not have to be transformed during
        // the target compilation, hence, hasOffloadCompilation()
        // check below.
        debugPrintHeader(W, IsPrepare);
        if (Mode & ParPrepare) {
          if (!hasOffloadCompilation()) {
            Changed |= canonicalizeGlobalVariableReferences(W);
            Changed |= renameOperandsUsingStoreThenLoad(W);
          }
        } else if ((Mode & OmpPar) && (Mode & ParTrans)) {
          if (!hasOffloadCompilation()) {
            // The purpose is to generate place holder for global variable.
            //
            // For WRNTargetEnterData and WRNTargetExitData we may
            // avoid laundering the global variables that are declared target,
            // unless they are mapped as ALWAYS.  We do not need to pass
            // them to the target runtime library, as long as they have
            // infinite reference count, and will not require data motion.
            Changed |= genGlobalPrivatizationLaunderIntrin(W);
            Changed |= genTargetOffloadingCode(W);
            Changed |= clearLaunderIntrinBeforeRegion(W);
          }
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNTargetData:
        if (DisableOffload) {
          // Ignore TARGET DATA construct
          LLVM_DEBUG(dbgs()<<"VPO: Ignored " << W->getName() << " construct\n");
          RemoveDirectives = true;
          break;
        }
        // This construct does not have to be transformed during
        // the target compilation, hence, hasOffloadCompilation()
        // check below.
        debugPrintHeader(W, IsPrepare);
        if (Mode & ParPrepare) {
          if (!hasOffloadCompilation()) {
            Changed |= canonicalizeGlobalVariableReferences(W);
            Changed |= renameOperandsUsingStoreThenLoad(W);
          }
        } else if ((Mode & OmpPar) && (Mode & ParTrans)) {
          if (!hasOffloadCompilation()) {
            // The purpose is to generate place holder for global variable.
            Changed |= genGlobalPrivatizationLaunderIntrin(W);
#if INTEL_CUSTOMIZATION
            improveAliasForOutlinedFunc(W);
#endif  // INTEL_CUSTOMIZATION
            Changed |= genTargetOffloadingCode(W);
            Changed |= clearLaunderIntrinBeforeRegion(W);
          }
          RemoveDirectives = true;
        }
        break;

      // 2. Below are constructs that do not need to perform outlining.
      //    E.g., simd, taskgroup, atomic, for, sections, etc.

      case WRegionNode::WRNTargetVariant:
        if (DisableOffload) {
          // Ignore TARGET VARIANT DISPATCH construct
          LLVM_DEBUG(dbgs()<<"VPO: Ignored " << W->getName() << " construct\n");
          RemoveDirectives = true;
          break;
        }
        // The target variant dispatch construct does not need outlining so
        // it is codegen'ed during the Prepare phase of the HOST compilation
        if (Mode & ParPrepare) {
          debugPrintHeader(W, true);
          if (!hasOffloadCompilation()) // for host only
            Changed |= genTargetVariantDispatchCode(W);
          RemoveDirectives = true;
        }
        break;

      case WRegionNode::WRNTaskgroup:
        debugPrintHeader(W, IsPrepare);
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed = genTaskgroupRegion(W);
          RemoveDirectives = true;
        }
        break;

      case WRegionNode::WRNVecLoop:
        if (Mode & ParPrepare) {
          Changed = regularizeOMPLoop(W);
          Changed |= canonicalizeGlobalVariableReferences(W);
          Changed |= renameOperandsUsingStoreThenLoad(W);
        }
        // Privatization is enabled for SIMD Transform passes
        if ((Mode & OmpVec) && (Mode & ParTrans)) {
          debugPrintHeader(W, false);
          Changed = regularizeOMPLoop(W, false);
          Changed |= genPrivatizationCode(W);
          if (W->getWRNLoopInfo().getNormIVSize() != 0) {
            // Code for SIMD clauses, such as lastprivate, must be inserted
            // outside of the loop. And this insertion must be coordinated
            // with code inserted for enclosing regions related to the same
            // loop. Here we only handle standalone SIMD regions.
            // Lasprivatization, linearization, etc. will be done for variables
            // during transformation of the enclosing region in case of
            // combined OpenMP constructs.
            //
            // Standalone SIMD regions will have normalized IV and UB.
            // Normalized IV and UB are not present for SIMD regions
            // combined with other loop type regions.
            //
            // Note that handling of PRIVATE does not require new code
            // (except new alloca) outside of the loop, so it can be done
            // always.
            auto *LoopExitBB = getLoopExitBB(W);
            // Last value update must happen in the loop's exit block,
            // i.e. under a ZTT check, if one was created around the loop.
//            Changed |= genLinearCode(W, LoopExitBB);
            Changed |= genLastPrivatizationCode(W, LoopExitBB);
            Changed |= genReductionCode(W);
          }
          // keep SIMD directives; will be processed by the Vectorizer
          RemoveDirectives = false;
        }
        break;
      case WRegionNode::WRNAtomic:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, true);
          if (IsTargetSPIRV)
            Changed |= removeCompilerGeneratedFences(W);
          Changed = VPOParoptAtomics::handleAtomic(
              cast<WRNAtomicNode>(W), IdentTy, TidPtrHolder, IsTargetSPIRV);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNWksLoop:
      case WRegionNode::WRNSections:
      case WRegionNode::WRNDistribute:
        debugPrintHeader(W, IsPrepare);
        if (Mode & ParPrepare) {
          if (W->getIsSections())
            Changed = addNormUBsToParents(W);

          Changed |= regularizeOMPLoop(W);
          Changed |= canonicalizeGlobalVariableReferences(W);
          Changed |= renameOperandsUsingStoreThenLoad(W);
          Changed |= propagateCancellationPointsToIR(W);
        }
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed |= clearCancellationPointAllocasFromIR(W);
          Changed |= regularizeOMPLoop(W, false);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
          if (isTargetCSA()) {
            // Generate remarks using Loop Opt Report framework (under -qopt-report).
            if (isa<WRNWksLoopNode>(W)) {
               ORLinfo = W->getWRNLoopInfo().getLoopInfo();
               ORLoop = W->getWRNLoopInfo().getLoop();
               if (ORLoop != nullptr)
                  LORBuilder(*ORLoop, *ORLinfo).addRemark(OptReportVerbosity::Low,
                             "CSA: OpenMP worksharing loop will be pipelined");
            }

            if (W->getIsSections()) {
              Changed |= genCSASections(W);
              RemoveDirectives = true;
            }
            else if (W->getIsOmpLoop()) {
              auto Res = genCSALoop(W);
              Changed |= Res.first;
              RemoveDirectives = Res.second;
            }
            else
              llvm_unreachable("Unexpected work region kind");
            break;
          }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          Changed |= constructNDRangeInfo(W);

          if (IsTargetSPIRV) {
            Changed |= genOCLParallelLoop(W);
            Changed |= genPrivatizationCode(W);
            if (!W->getIsDistribute())
              Changed |= genReductionCode(W);
          } else {
#if INTEL_CUSTOMIZATION
            // Generate remarks using Loop Opt Report framework (under -qopt-report).
            if (isa<WRNWksLoopNode>(W)) {
               ORLinfo = W->getWRNLoopInfo().getLoopInfo();
               ORLoop = W->getWRNLoopInfo().getLoop();
               if (ORLoop != nullptr)
                  LORBuilder(*ORLoop, *ORLinfo).addRemark(OptReportVerbosity::Low,
                             "OpenMP: Worksharing loop");
            }
#endif  // INTEL_CUSTOMIZATION

            AllocaInst *IsLastVal = nullptr;
            BasicBlock *IfLastIterBB = nullptr;
            Instruction *InsertLastIterCheckBefore = nullptr;
            Instruction *OMPLBForLinearClosedForm = nullptr;
            Instruction *OMPZtt = nullptr;
            Changed |=
                genLoopSchedulingCode(W, IsLastVal, InsertLastIterCheckBefore,
                                      OMPLBForLinearClosedForm, OMPZtt);
            Changed |= genPrivatizationCode(W);
            Changed |= genLastIterationCheck(W, IsLastVal, IfLastIterBB,
                                             InsertLastIterCheckBefore);
            Changed |= genLinearCode(W, IfLastIterBB, OMPLBForLinearClosedForm);
            Changed |= genLastPrivatizationCode(
                W, IfLastIterBB, OMPLBForLinearClosedForm,
                InsertLastIterCheckBefore, OMPZtt);
            Changed |= genFirstPrivatizationCode(W);
            if (!W->getIsDistribute()) {
              Changed |= genReductionCode(W);
              Changed |= genCancellationBranchingCode(W);
            }
            Changed |= genDestructorCode(W);
            if (!W->getIsDistribute() && !W->getNowait())
              Changed |= genBarrier(W, false);
          }
          Changed |= sinkSIMDDirectives(W);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNSingle:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, true);
          // Changed = genPrivatizationCode(W);
          // Changed |= genFirstPrivatizationCode(W);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
          if (isTargetCSA()) {
            Changed |= removeCompilerGeneratedFences(W);
            Changed |= genCSASingle(W);
            RemoveDirectives = true;
            break;
          }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          if (IsTargetSPIRV)
            Changed |= removeCompilerGeneratedFences(W);

          AllocaInst *IsSingleThread = nullptr;
          Changed = genSingleThreadCode(W, IsSingleThread);
          Changed |= genCopyPrivateCode(W, IsSingleThread);
          Changed |= genPrivatizationCode(W);
          Changed |= genFirstPrivatizationCode(W);
          // Changed |= genDestructorCode(W);
          if (!W->getNowait())
            Changed |= genBarrier(W, false);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNMaster:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, true);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
          if (isTargetCSA()) {
            Changed |= removeCompilerGeneratedFences(W);
            RemoveDirectives = true;
            break;
          }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          if (IsTargetSPIRV)
            Changed |= removeCompilerGeneratedFences(W);
          Changed = genMasterThreadCode(W, IsTargetSPIRV);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNCritical:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, true);
          Changed = genCriticalCode(cast<WRNCriticalNode>(W));
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNOrdered:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, true);
          if (W->getIsDoacross()) {
            Changed = genDoacrossWaitOrPost(cast<WRNOrderedNode>(W));
          } else {
            Changed = genOrderedThreadCode(W);
          }

          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNBarrier:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, true);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
          if (isTargetCSA()) {
            Changed |= removeCompilerGeneratedFences(W);
            RemoveDirectives = true;
            break;
          }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          if (IsTargetSPIRV)
            Changed |= removeCompilerGeneratedFences(W);
          Changed = genBarrier(W, true, IsTargetSPIRV);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNCancel:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, true);
          Changed = genCancelCode(cast<WRNCancelNode>(W));
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNFlush:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, true);
          Changed = genFlush(W);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNGenericLoop:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, true);
          Changed = replaceGenericLoop(W);
          Changed |= regularizeOMPLoop(W);
          Changed |= canonicalizeGlobalVariableReferences(W);
          Changed |= renameOperandsUsingStoreThenLoad(W);
          RemoveDirectives = false;
        }
        break;
      default:
        break;
      } // switch
    }

    // Emit opt-report remarks for handled/ignored constructs.
    if (RemoveDirectives || HandledWithoutRemovingDirectives) {
      if (Changed) {
        OptimizationRemark R("openmp", "Region", W->getEntryDirective());
        R << ore::NV("Construct", W->getName()) << " construct transformed";
        ORE.emit(R);
      } else {
        OptimizationRemarkMissed R("openmp", "Region", W->getEntryDirective());
        R << ore::NV("Construct", W->getName()) << " construct ignored";
        ORE.emit(R);
      }
    }

    // Remove calls to directive intrinsics since the LLVM back end does not
    // know how to translate them.
    if (RemoveDirectives) {
      bool DirRemoved = VPOUtils::stripDirectives(W);
      assert(DirRemoved && "Directive intrinsics not removed for WRN.\n");
      (void) DirRemoved;
    } else if (Mode & ParPrepare)
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  if (!isTargetCSA())
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
      addBranchToEndDirective(W);

    if (Changed) { // Code transformations happened for this WRN
      RoutineChanged = true;
      LLVM_DEBUG(dbgs() << "   === WRN #" << W->getNumber()
                        << " transformed.\n\n");
    }
    else
      LLVM_DEBUG(dbgs() << "   === WRN #" << W->getNumber()
                        << " NOT transformed.\n\n");
  }

  WRegionList.clear();
  return RoutineChanged;
}

Value *VPOParoptTransform::genReductionMinMaxInit(ReductionItem *RedI,
                                                  Type *Ty, bool IsMax) {
  Value *V = nullptr;

  if (Ty->isIntOrIntVectorTy()) {
    LLVMContext &C = F->getContext();
    bool IsUnsigned = RedI->getIsUnsigned();
    V = VPOParoptUtils::getMinMaxIntVal(C, Ty, IsUnsigned, !IsMax);
#if 0
    uint64_t val = IsMax ? VPOParoptUtils::getMinInt(Ty, IsUnsigned) :
                           VPOParoptUtils::getMaxInt(Ty, IsUnsigned);
    V = ConstantInt::get(Ty, val);
#endif
  }
  else if (Ty->isFPOrFPVectorTy())
    V = IsMax ? ConstantFP::getInfinity(Ty, true) :  // max: negative inf
                ConstantFP::getInfinity(Ty, false);  // min: positive inf
  else
    llvm_unreachable("Unsupported type in OMP reduction!");

  return V;
}

// Generate the reduction intialization instructions.
Value *VPOParoptTransform::genReductionScalarInit(ReductionItem *RedI,
                                                  Type *ScalarTy) {
  Value *V = nullptr;
  switch (RedI->getType()) {
  case ReductionItem::WRNReductionAdd:
  case ReductionItem::WRNReductionSub:
    V = ScalarTy->isIntOrIntVectorTy() ? ConstantInt::get(ScalarTy, 0)
                                       : ConstantFP::get(ScalarTy, 0.0);
    break;
  case ReductionItem::WRNReductionMult:
    V = ScalarTy->isIntOrIntVectorTy() ? ConstantInt::get(ScalarTy, 1)
                                       : ConstantFP::get(ScalarTy, 1.0);
    break;
  case ReductionItem::WRNReductionAnd:
    V = ConstantInt::get(ScalarTy, 1);
    break;
  case ReductionItem::WRNReductionOr:
  case ReductionItem::WRNReductionBxor:
  case ReductionItem::WRNReductionBor:
    V = ConstantInt::get(ScalarTy, 0);
    break;
  case ReductionItem::WRNReductionBand:
    V = ConstantInt::get(ScalarTy, -1);
    break;
  case ReductionItem::WRNReductionMax:
    V = genReductionMinMaxInit(RedI, ScalarTy, true);
    break;
  case ReductionItem::WRNReductionMin:
    V = genReductionMinMaxInit(RedI, ScalarTy, false);
    break;
  default:
    llvm_unreachable("Unspported reduction operator!");
  }
  return V;
}

// Generate the reduction fini code for bool and/or.
// Given the directive pragma omp parallel reduction( +: a1 ) reduction(&&: a2
// ), here is the output for bool "and" opererator.
//
//  if.end5:                                          ; preds = %if.then4,
//  %if.end
//    %my.tid = load i32, i32* %tid, align 4
//    call void @__kmpc_critical({ i32, i32, i32, i32, i8* }* @.kmpc_loc.0.0.3,
//    i32 %my.tid, [8 x i32]* @.gomp_critical_user_.var)
//    br label %if.end5.split
//
//  if.end5.split:                                    ; preds = %if.end5
//    %1 = load i8, i8* %a1
//    %2 = load i8, i8* %a1.red
//    %3 = add i8 %1, %2
//    store i8 %3, i8* %a1
//    br label %if.end5.split.split
//
//  if.end5.split.split:                              ; preds = %if.end5.split
//    %4 = load i8, i8* %a2
//    %5 = load i8, i8* %a2.red
//    %6 = sext i8 %4 to i32
//    %tobool = icmp ne i32 %6, 0
//    br i1 %tobool, label %land.rhs, label %land.end
//
//  land.rhs:                                         ; preds =
//  %if.end5.split.split
//    %7 = sext i8 %5 to i32
//    %tobool15 = icmp ne i32 %7, 0
//    br label %land.end
//
//  land.end:                                         ; preds =
//  %if.end5.split.split, %land.rhs
//    %8 = phi i1 [ false, %if.end5.split.split ], [ %tobool15, %land.rhs ]
//    %9 = zext i1 %8 to i32
//    %10 = trunc i32 %9 to i8
//    store i8 %10, i8* %a2
//    br label %if.end5.split.split.split
//
//  if.end5.split.split.split:                        ; preds = %land.end
//    %my.tid16 = load i32, i32* %tid, align 4
//    call void @__kmpc_end_critical({ i32, i32, i32, i32, i8* }*
//    @.kmpc_loc.0.0.5, i32 %my.tid16, [8 x i32]* @.gomp_critical_user_.var)
//  br label %exitStub
//
// Similiarly, here is the output for bool "or" operator given the direcitive in
// the form of #pragma omp parallel reduction( +: a1 ) reducion( ||: a2 ).
//
//  if.end5:                                          ; preds = %if.then4,
//  %if.end
//    %my.tid = load i32, i32* %tid, align 4
//    call void @__kmpc_critical({ i32, i32, i32, i32, i8* }* @.kmpc_loc.0.0.3,
//    i32 %my.tid, [8 x i32]* @.gomp_critical_user_.var)
//    br label %if.end5.split
//
//  if.end5.split:                                    ; preds = %if.end5
//    %1 = load i8, i8* %a1
//    %2 = load i8, i8* %a1.red
//    %3 = add i8 %1, %2
//    store i8 %3, i8* %a1
//    br label %if.end5.split.split
//
//  if.end5.split.split:                              ; preds = %if.end5.split
//    %4 = load i8, i8* %a2
//    %5 = load i8, i8* %a2.red
//    %6 = sext i8 %4 to i32
//    %tobool = icmp ne i32 %6, 0
//    br i1 %tobool, label %lor.end, label %lor.rhs
//
//  lor.end:                                          ; preds =
//  %if.end5.split.split, %lor.rhs
//    %7 = phi i1 [ false, %if.end5.split.split ], [ %tobool15, %lor.rhs ]
//    %8 = zext i1 %7 to i32
//    %9 = trunc i32 %8 to i8
//    store i8 %9, i8* %a2
//    br label %if.end5.split.split.split
//
//  lor.rhs:                                          ; preds =
//  %if.end5.split.split
//    %10 = sext i8 %5 to i32
//    %tobool15 = icmp ne i32 %10, 0
//    br label %lor.end
//
//  if.end5.split.split.split:                        ; preds = %lor.end
//    %my.tid16 = load i32, i32* %tid, align 4
//    call void @__kmpc_end_critical({ i32, i32, i32, i32, i8* }*
//    @.kmpc_loc.0.0.5, i32 %my.tid16, [8 x i32]* @.gomp_critical_user_.var)
//    br label %exitStub
//
Value* VPOParoptTransform::genReductionFiniForBoolOps(ReductionItem *RedI,
                                          Value *Rhs1, Value *Rhs2,
                                          Type *ScalarTy,
                                          IRBuilder<> &Builder,
                                          bool IsAnd) {
  LLVMContext &C = F->getContext();
  // FIXME: handle FP types here, and also make sure that
  //        significant bits are not truncated before
  //        comparing the value with zero.
  auto Conv = Builder.CreateSExtOrTrunc(Rhs1, Type::getInt32Ty(C));
  ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);
  auto IsTrue = Builder.CreateICmpNE(Conv, ValueZero, "tobool");
  auto EntryBB = Builder.GetInsertBlock();
  Instruction *InsertPt = &*Builder.GetInsertPoint();
  auto ContBB = SplitBlock(EntryBB, InsertPt, DT, LI);
  ContBB->setName(IsAnd ? "land.rhs" : "lor.rhs");

  auto RhsBB = SplitBlock(ContBB, ContBB->getTerminator(), DT, LI);
  RhsBB->setName(IsAnd ? "land.end" : "lor.end");

  EntryBB->getTerminator()->eraseFromParent();
  Builder.SetInsertPoint(EntryBB);
  Builder.CreateCondBr(IsTrue, IsAnd ? ContBB : RhsBB, IsAnd ? RhsBB : ContBB);

  Builder.SetInsertPoint(ContBB->getTerminator());
  auto ConvRed = Builder.CreateSExtOrTrunc(Rhs2, Type::getInt32Ty(C));
  auto IsTrueRed = Builder.CreateICmpNE(ConvRed, ValueZero, "tobool");

  Builder.SetInsertPoint(RhsBB->getTerminator());
  PHINode *PN = Builder.CreatePHI(Type::getInt1Ty(C), 2, "");
  auto PhiEntryBBVal = IsAnd ? ConstantInt::getFalse(C) :
                               ConstantInt::getTrue(C);
  PN->addIncoming(PhiEntryBBVal, EntryBB);
  PN->addIncoming(IsTrueRed, ContBB);
  auto Ext = Builder.CreateZExtOrBitCast(PN, Type::getInt32Ty(C));
  auto ConvFini = Builder.CreateSExtOrTrunc(Ext, ScalarTy);

  return ConvFini;
}

Value* VPOParoptTransform::genReductionMinMaxFini(ReductionItem *RedI,
                                                  Value *Rhs1, Value *Rhs2,
                                                  Type *ScalarTy,
                                                  IRBuilder<> &Builder,
                                                  bool IsMax) {
  Value *IsGT = nullptr; // compares Rhs1 > Rhs2

  if (ScalarTy->isIntOrIntVectorTy())
    if(RedI->getIsUnsigned())
      IsGT = Builder.CreateICmpUGT(Rhs1, Rhs2, "isUGT"); // unsigned
    else
      IsGT = Builder.CreateICmpSGT(Rhs1, Rhs2, "isSGT"); // signed
  else if (ScalarTy->isFPOrFPVectorTy())
    IsGT = Builder.CreateFCmpOGT(Rhs1, Rhs2, "isOGT");   // FP
  else
    llvm_unreachable("Unsupported type in OMP reduction!");

  Value *Op1, *Op2;
  const char* Name;

  if (IsMax) {
    Op1  = Rhs1;
    Op2  = Rhs2;
    Name = "max";
  } else {
    Op1  = Rhs2;
    Op2  = Rhs1;
    Name = "min";
  }

  Value *minmax = Builder.CreateSelect(IsGT, Op1, Op2, Name);
  return minmax;
}

// Generate the reduction update instructions.
bool VPOParoptTransform::genReductionScalarFini(
    WRegionNode *W, ReductionItem *RedI,
    Value *ReductionVar, Value *ReductionValueLoc,
    Type *ScalarTy, IRBuilder<> &Builder) {
  Value *Res = nullptr;
  auto *Rhs2 = Builder.CreateLoad(ReductionValueLoc);
  auto *Rhs1 = Builder.CreateLoad(ReductionVar);

  switch (RedI->getType()) {
  case ReductionItem::WRNReductionAdd:
  case ReductionItem::WRNReductionSub:
    Res = ScalarTy->isIntOrIntVectorTy() ? Builder.CreateAdd(Rhs1, Rhs2)
                                         : Builder.CreateFAdd(Rhs1, Rhs2);
    break;
  case ReductionItem::WRNReductionMult:
    Res = ScalarTy->isIntOrIntVectorTy() ? Builder.CreateMul(Rhs1, Rhs2)
                                         : Builder.CreateFMul(Rhs1, Rhs2);
    break;
  case ReductionItem::WRNReductionBand:
    Res = Builder.CreateAnd(Rhs1, Rhs2);
    break;
  case ReductionItem::WRNReductionBor:
    Res = Builder.CreateOr(Rhs1, Rhs2);
    break;
  case ReductionItem::WRNReductionBxor:
    Res = Builder.CreateXor(Rhs1, Rhs2);
    break;
  case ReductionItem::WRNReductionAnd:
    Res = genReductionFiniForBoolOps(RedI, Rhs1, Rhs2, ScalarTy, Builder,
                                     true);
    break;
  case ReductionItem::WRNReductionOr:
    Res = genReductionFiniForBoolOps(RedI, Rhs1, Rhs2, ScalarTy, Builder,
                                     false);
    break;
  case ReductionItem::WRNReductionMax:
    Res = genReductionMinMaxFini(RedI, Rhs1, Rhs2, ScalarTy, Builder, true);
    break;
  case ReductionItem::WRNReductionMin:
    Res = genReductionMinMaxFini(RedI, Rhs1, Rhs2, ScalarTy, Builder, false);
    break;
  default:
    llvm_unreachable("Reduction operator not yet supported!");
  }
  Instruction *Tmp0 = Builder.CreateStore(Res, ReductionVar);

  if (isa<WRNVecLoopNode>(W))
    // Reduction update does not have to be atomic for SIMD loop.
    return false;

  if (isTargetSPIRV()) {
    // This method may insert a new call before the store instruction (Tmp0)
    // and erase the store instruction, but in any case it does not invalidate
    // the IRBuilder.
    auto *AtomicCall =
        VPOParoptAtomics::handleAtomicUpdateInBlock(W, Tmp0->getParent(),
                                                    nullptr, nullptr, true);

    if (AtomicCall) {
      OptimizationRemark R(DEBUG_TYPE, "ReductionAtomic", AtomicCall);
      R << ore::NV("Kind", RedI->getOpName()) << " reduction update of type " <<
        ore::NV("Type", ScalarTy) << " made atomic";
      ORE.emit(R);

      return false;
    }

    // Try to generate a horizontal (sub-group) reduction.
    // Without the horizontal reduction the generated code may be
    // incorrectly widened by the device compiler.

    // We clone the Rhs2 definition, because we have to use Rhs2 value
    // in the horizontal reduction call, and then replace all uses
    // of Rhs2 with the call's return value. The cloning just makes
    // it easier.
    //
    // Insert new instruction(s) after the definition of the private
    // reduction value.
    auto *TempRedLoad = Rhs2->clone();
    TempRedLoad->insertAfter(Rhs2);
    TempRedLoad->takeName(Rhs2);
    auto HRed = VPOParoptUtils::genSPIRVHorizontalReduction(
        RedI, ScalarTy, TempRedLoad, spirv::Scope::Subgroup);

    if (HRed)
      Rhs2->replaceAllUsesWith(HRed);
    else
      LLVM_DEBUG(dbgs() << __FUNCTION__ <<
                 ": SPIRV horizontal reduction is not available "
                 "for critical section reduction: " << RedI->getOpName() <<
                 " with type " << *ScalarTy << "\n");

    OptimizationRemarkMissed R(DEBUG_TYPE, "ReductionAtomic", Tmp0);
    R << ore::NV("Kind", RedI->getOpName()) << " reduction update of type " <<
        ore::NV("Type", ScalarTy) << " cannot be done using atomic API";
    ORE.emit(R);
  }

  return true;
}

// Generate the reduction update code.
// Here is one example for the reduction update for the scalar.
//   sum = 4.0;
//   #pragma omp parallel for reduction(+:sum)
//   for (i=0; i < n; i++)
//     sum = sum + (a[i] * b[i]);
//
// The output of the reduction update for the variable sum
// is as follows.
//
//   /* B[%for.end15]  */
//   %my.tid = load i32, i32* %tid, align 4
//   call void @__kmpc_critical({ i32, i32, i32, i32, i8* }* @.kmpc_loc.0.0.2,
//   i32 %my.tid, [8 x i32]* @.gomp_critical_user_.var)
//   br label %for.end15.split
//
//
//   /* B[%for.end15.split]  */
//   %9 = load float, float* %sum
//   %10 = load float, float* %sum.red
//   %11 = fadd float %9, %10
//   store float %11, float* %sum, align 4
//   br label %for.end15.split.split
//
//
//   /* B[%for.end15.split.split]  */
//   %my.tid31 = load i32, i32* %tid, align 4
//   call void @__kmpc_end_critical({ i32, i32, i32, i32, i8* }*
//   @.kmpc_loc.0.0.4, i32 %my.tid31, [8 x i32]* @.gomp_critical_user_.var)
//   br label %exitStub
//
bool VPOParoptTransform::genReductionFini(WRegionNode *W,
                                          ReductionItem *RedI, Value *OldV,
                                          Instruction *InsertPt,
                                          DominatorTree *DT) {
  Value *NewAI = RedI->getNew();
  // NewAI is either AllocaInst or an AddrSpaceCastInst of AllocaInst.
  assert(GeneralUtils::isOMPItemLocalVAR(NewAI) &&
         "genReductionFini: Expect non-empty optionally casted "
         "alloca instruction.");
  Type *AllocaTy =
      GeneralUtils::getOMPItemLocalVARPointerType(NewAI)->getElementType();

  IRBuilder<> Builder(InsertPt);
  // For by-refs, do a pointer dereference to reach the actual operand.
  if (RedI->getIsByRef()) {
    OldV = Builder.CreateLoad(OldV);
  }

  bool NeedsKmpcCritical = false;

  if (RedI->getIsArraySection() ||
      AllocaTy->isArrayTy())
    NeedsKmpcCritical |=
        genRedAggregateInitOrFini(W, RedI, NewAI, OldV, InsertPt, false, DT);
  else {
    assert(VPOUtils::canBeRegisterized(
               AllocaTy, InsertPt->getModule()->getDataLayout()) &&
           "genReductionFini: Expect incoming scalar type.");
    Type *ScalarTy = AllocaTy->getScalarType();

    NeedsKmpcCritical |=
        genReductionScalarFini(W, RedI, OldV, NewAI, ScalarTy, Builder);
  }

  return NeedsKmpcCritical;
}

// Generate the reduction initialization/update for array.
// Here is one example for the reduction initialization/update for array.
// #pragma omp parallel for reduction(+:sum)
//   for (i=0; i < n; i++)
//       for (j=0;j<n;j++)
//            sum[i] = sum[i] + (a[i][j] * b[i][j]);
//
//   The output of the reduction array initialization is as follows.
//
//   /* B[%for.end16.split]  */
//   %array.begin = getelementptr inbounds [100 x float], [100 x float]*
//   %sum.red, i32 0, i32 0
//   %1 = getelementptr float, float* %array.begin, i32 100
//   %red.init.isempty = icmp eq float* %array.begin, %1
//   br i1 %red.init.isempty, label %red.init.done, label %red.init.body
//   if (%red.init.isempty == false) {
//      do {
//
//         /* B[%red.init.body]  */
//         %red.cpy.dest.ptr = phi float* [ %array.begin, %for.end16.split ], [
//         %red.cpy.dest.inc, %red.init.body ]
//         store float 0.000000e+00, float* %red.cpy.dest.ptr
//         %red.cpy.dest.inc = getelementptr float, float* %red.cpy.dest.ptr,
//         i32 1
//         %red.cpy.done = icmp eq float* %red.cpy.dest.inc, %1
//         br i1 %red.cpy.done, label %red.init.done, label %red.init.body
//
//
//      } while (%red.cpy.done == false)
//   }
//
//   /* B[%red.init.done]  */
//   br label %dir.exit
//
//   The output of the reduction array update is as follows.
//
//   /* B[%for.end44.split]  */
//   %array.begin79 = getelementptr inbounds [100 x float], [100 x float]* %sum,
//   i32 0, i32 0
//   %array.begin80 = getelementptr inbounds [100 x float], [100 x float]*
//   %sum.red, i32 0, i32 0
//   %7 = getelementptr float, float* %array.begin79, i32 100
//   %red.update.isempty = icmp eq float* %array.begin79, %7
//   br i1 %red.update.isempty, label %red.update.done, label %red.update.body
//   if (%red.update.isempty == false) {
//      do {
//
//         /* B[%red.update.body]  */
//         %red.cpy.dest.ptr82 = phi float* [ %array.begin79, %for.end44.split
//         ], [ %red.cpy.dest.inc83, %red.update.body ]
//         %red.cpy.src.ptr = phi float* [ %array.begin80, %for.end44.split ], [
//         %red.cpy.src.inc, %red.update.body ]
//         %8 = load float, float* %red.cpy.dest.ptr82
//         %9 = load float, float* %red.cpy.src.ptr
//         %10 = fadd float %8, %9
//         store float %10, float* %red.cpy.dest.ptr82, align 4
//         %red.cpy.dest.inc83 = getelementptr float, float*
//         %red.cpy.dest.ptr82, i32 1
//         %red.cpy.src.inc = getelementptr float, float* %red.cpy.src.ptr, i32
//         1
//         %red.cpy.done84 = icmp eq float* %red.cpy.dest.inc83, %7
//         br i1 %red.cpy.done84, label %red.update.done, label %red.update.body
//
//
//      } while (%red.cpy.done84 == false)
//   }
//
//   /* B[%red.update.done]  */
//   br label %for.end44.split.split
//
//
//   /* B[%for.end44.split.split]  */
//   %my.tid85 = load i32, i32* %tid, align 4
//   call void @__kmpc_end_critical({ i32, i32, i32, i32, i8* }*
//   @.kmpc_loc.0.0.5, i32 %my.tid85, [8 x i32]* @.gomp_critical_user_.var)
//   br label %exitStub
//
bool VPOParoptTransform::genRedAggregateInitOrFini(WRegionNode *W,
                                                   ReductionItem *RedI,
                                                   Value *AI, Value *OldV,
                                                   Instruction *InsertPt,
                                                   bool IsInit,
                                                   DominatorTree *DT) {

  bool NeedsKmpcCritical = false;
  IRBuilder<> Builder(InsertPt);
  auto EntryBB = Builder.GetInsertBlock();

  Type *DestElementTy = nullptr;
  Value *DestBegin = nullptr;
  Value *SrcBegin = nullptr;
  Value *NumElements = nullptr;

  if (IsInit)
    genAggrReductionInitDstInfo(*RedI, AI, InsertPt, Builder, NumElements,
                                DestBegin, DestElementTy);
  else
    genAggrReductionFiniSrcDstInfo(*RedI, AI, OldV, InsertPt, Builder,
                                   NumElements, SrcBegin, DestBegin,
                                   DestElementTy);

  assert(DestBegin && "Null destination address for reduction init/fini.");
  assert(DestElementTy && "Null element type for reduction init/fini.");
  assert(NumElements && "Null number of elements for reduction init/fini.");
  assert((IsInit || SrcBegin) && "Null source address for reduction fini.");

  auto DestEnd = Builder.CreateGEP(DestBegin, NumElements);
  auto IsEmpty = Builder.CreateICmpEQ(
      DestBegin, DestEnd, IsInit ? "red.init.isempty" : "red.update.isempty");

  auto BodyBB = SplitBlock(EntryBB, InsertPt, DT, LI);
  BodyBB->setName(IsInit ? "red.init.body" : "red.update.body");

  auto DoneBB = SplitBlock(BodyBB, BodyBB->getTerminator(), DT, LI);
  DoneBB->setName(IsInit ? "red.init.done" : "red.update.done");

  EntryBB->getTerminator()->eraseFromParent();
  Builder.SetInsertPoint(EntryBB);
  Builder.CreateCondBr(IsEmpty, DoneBB, BodyBB);

  Builder.SetInsertPoint(BodyBB);
  BodyBB->getTerminator()->eraseFromParent();
  PHINode *DestElementPHI =
      Builder.CreatePHI(DestBegin->getType(), 2, "red.cpy.dest.ptr");
  DestElementPHI->addIncoming(DestBegin, EntryBB);

  PHINode *SrcElementPHI = nullptr;
  if (!IsInit) {
    SrcElementPHI =
        Builder.CreatePHI(SrcBegin->getType(), 2, "red.cpy.src.ptr");
    SrcElementPHI->addIncoming(SrcBegin, EntryBB);
  }

  if (IsInit) {
    Value *V = genReductionScalarInit(RedI, DestElementTy);
    Builder.CreateStore(V, DestElementPHI);
  } else {
    NeedsKmpcCritical |=
        genReductionScalarFini(W, RedI, DestElementPHI, SrcElementPHI,
                               DestElementTy, Builder);
  }

  auto DestElementNext =
      Builder.CreateConstGEP1_32(DestElementPHI, 1, "red.cpy.dest.inc");
  Value *SrcElementNext = nullptr;
  if (!IsInit)
    SrcElementNext =
        Builder.CreateConstGEP1_32(SrcElementPHI, 1, "red.cpy.src.inc");

  auto Done = Builder.CreateICmpEQ(DestElementNext, DestEnd, "red.cpy.done");

  Builder.CreateCondBr(Done, DoneBB, BodyBB);
  DestElementPHI->addIncoming(DestElementNext, Builder.GetInsertBlock());
  if (!IsInit)
    SrcElementPHI->addIncoming(SrcElementNext, Builder.GetInsertBlock());

  if (DT) {
    DT->changeImmediateDominator(BodyBB, EntryBB);
    DT->changeImmediateDominator(DoneBB, EntryBB);
  }

  return NeedsKmpcCritical;
}

// Generate the firstprivate initialization code.
// Here is one example for the firstprivate initialization for the array.
// num_type    a[100];
// #pragma omp parallel for schedule( static, 1 ) firstprivate( a )
// The output of the array initialization is as follows.
//
//    %a = alloca [100 x float]
//    br label %DIR.OMP.PARALLEL.LOOP.1.split
//
// DIR.OMP.PARALLEL.LOOP.1.split:                    ; preds =
// %DIR.OMP.PARALLEL.LOOP.1
//    %1 = bitcast [100 x float]* %a to i8*
//    call void @llvm.memcpy.p0i8.p0i8.i64(i8* %1, i8* bitcast ([100 x float]*
//    @a to i8*), i64 400, i32 0, i1 false)
//
void VPOParoptTransform::genFprivInit(FirstprivateItem *FprivI,
                                      Instruction *InsertPt) {
  auto *NewV = FprivI->getNew();
  // NewV is either AllocaInst, GEP, or an AddrSpaceCastInst of AllocaInst.
  assert((GeneralUtils::isOMPItemLocalVAR(NewV) ||
          isa<GetElementPtrInst>(FprivI->getNew())) &&
         "genFprivInit: Expect valid memory pointer instruction");
  VPOParoptUtils::genCopyByAddr(NewV, FprivI->getOrig(), InsertPt,
                                FprivI->getCopyConstructor(),
                                FprivI->getIsByRef());
}

// Generate the lastprivate update code. The same mechanism is also applied
// for copyprivate.
// Here is one example for the lastprivate update for the array.
// num_type    a[100];
// #pragma omp parallel for schedule( static, 1 ) lastprivate( a )
// The output of the array update is as follows.
//
//    %a = alloca [100 x float]
//    br label %for.end
//  ...
//  for.end:                                          ; preds = %dispatch.latch,
//    %1 = bitcast [100 x float]* %a to i8*
//    call void @llvm.memcpy.p0i8.p0i8.i64(i8* bitcast ([100 x float]* @a to
//    i8*), i8* %1, i64 400, i32 0, i1 false)
//    br label %for.end.split
//
void VPOParoptTransform::genLprivFini(Value *NewV, Value *OldV,
                                      Instruction *InsertPt) {
  // NewV is either AllocaInst or an AddrSpaceCastInst of AllocaInst.
  assert(GeneralUtils::isOMPItemLocalVAR(NewV) &&
         "genLprivFini: Expect non-empty optionally casted "
         "alloca instruction.");
  // todo: copy constructor call needed?
  VPOParoptUtils::genCopyByAddr(OldV, NewV,
                                InsertPt->getParent()->getTerminator());
}

// genLprivFini interface to support nonPOD with call to CopyAssign
void VPOParoptTransform::genLprivFini(LastprivateItem *LprivI,
                                      Instruction *InsertPt) {
  Value *NewV = LprivI->getNew();
  Value *OldV = LprivI->getOrig();
  // For by-refs, do a pointer dereference to reach the actual operand.
  if (LprivI->getIsByRef())
    OldV = new LoadInst(OldV, "", InsertPt);

  if (Function *CpAssn = LprivI->getCopyAssign())
    VPOParoptUtils::genCopyAssignCall(CpAssn, OldV, NewV, InsertPt);
  else
    genLprivFini(NewV, OldV, InsertPt);
}

void VPOParoptTransform::collectStoresToLastprivateNewI(
    WRegionNode *W, LastprivateItem *LprivI,
    SmallVectorImpl<Instruction *> &LprivIStores) {
  assert(LprivI && "Null lastprivate item");
  Value *LprivINew = LprivI->getNew();

  SmallSetVector<Value *, 8> ValuesToCheck;

  // For By-refs, the store is done through the pointee of the by-ref pointer.
  if (!LprivI->getIsByRef()) {
    ValuesToCheck.insert(LprivINew);
  } else {
    // There should only be one use of the LprivINew at this point, which
    // is a store to a pointer, which in turn would be used inside the region.
    Value *LprivINewByrefPtr = nullptr;
    for (auto *LprivINewUser : LprivINew->users()) {
      if (StoreInst *LprivINewStore = dyn_cast<StoreInst>(LprivINewUser)) {
        assert(!LprivINewByrefPtr && "Unexpected number of stores of private "
                                     "copy of byref lastprivate.");
        LprivINewByrefPtr = LprivINewStore->getPointerOperand();
      }
    }

    // Now, the stores to conditional lastprivate operand would happen
    // through the byref pointer. So we need to find all loads of the
    // byref pointer, and then all stores to those loads.
    for (auto *ByrefPtrUser : LprivINewByrefPtr->users()) {
      if (auto *ByrefPtrLoad = dyn_cast<LoadInst>(ByrefPtrUser))
        ValuesToCheck.insert(ByrefPtrLoad);
    }
  }

  // Now, collect stores to all items in ValuesToCheck.
  for (unsigned I = 0; I < ValuesToCheck.size(); ++I) {
    Value *V = ValuesToCheck[I];

    SmallVector<Instruction *, 8> VUses;
    WRegionUtils::findUsersInRegion(W, V, &VUses, false);
    for (auto *VU : VUses) {
      if (isa<StoreInst>(VU) && cast<StoreInst>(VU)->getPointerOperand() == V)
        LprivIStores.push_back(VU);
      else if (isa<CastInst>(VU))
        // It is possible for there to be a bitcast or another cast to the
        // pointer, before a store is done to it. Add these casts to
        // ValuesToCheck.
        ValuesToCheck.insert(VU);
    }
  }
}

// For conditional lastprivate variables, emit code like this:
// ------------------------------------------------------------------
// For static-even scheduling:
//   int y = 0;
//   ...
//   #pragma omp for lastprivate(conditional:y)
//   for (int i = 0; i < 10; i++) {
//     if (<src_condition>) {
//       y = i;
//     }
//     ...
//   }
//
// Generated code would look like:
// ------------------------------------------------------------------
//
//   @y.global.max.idx = private global i32 0                           (1)
//   ...
//
//   %y.local.max.idx = alloca i32                                      (2)
//   store i32 0, i32* %y.local.max.idx                                 (3)
//   %y.modified.by.thread = alloca i1                                  (4)
//   store i1 false, i1* %y.modified.by.thread                          (5)
//   %y.lpriv = alloca i32                                              (6)
//   ...
//
//   call void @__kmpc_for_static_init_4(...i32* %lower.bnd...)
//
//   %lb.new = load i32, i32* %lower.bnd                                (7)
//   %omp.ztt = icmp sle i32 %lb.new, %ub.new                           (8)
//   %omp.ztt.cast = zext i1 %omp.ztt to i32                            (9)
//   %omp.lb.or.zero = mul i32 %lb.new, %omp.ztt.cast                   (10)
//   store i32 %omp.lb.or.zero, i32* %y.local.max.idx                   (11)
//
//   if (%omp.ztt) {
//     for (%omp.lb to %omp.ub) {
//       ...
//       if (<src_condition>) {
//         store i1 true, i1* %y.modified.by.thread                     (12)
//         store i32 <value_of_i>, i32* %y.lpriv
//       }
//       ...
//     }
//   }
//   call void @__kmpc_for_static_fini(...)
//   ...
//
//   %9 = load i1, i1* %y.modified.by.thread                            (13)
//   %y.written.by.thread = icmp eq i1 %9, true                         (14)
//
//   if (%y.written.by.thread) {                                        (15)
//     %11 = load i32, i32* %y.local.max.idx                            (16)
//
//     call void @__kmpc_critical(...)                                  (17)
//       %12 = load i32, i32* @y.global.max.idx                         (18)
//       %y.is.local.idx.higher = icmp uge i32 %11, %12                 (19)
//       if (%y.is.local.idx.higher) {                                  (20)
//         store i32 %11, i32* @y.global.max.idx                        (21)
//       }
//     call void @__kmpc_end_critical(...)                              (22)
//   }
//
//   call void @__kmpc_barrier(...)                                     (23)
//
//   %16 = load i32, i32* %y.local.max.idx                              (24)
//   %17 = load i32, i32* @y.global.max.idx                             (25)
//   %y.copyout.or.not = icmp eq i32 %16, %17                           (26)
//   if (%y.copyout.or.not) {                                           (27)
//     %19 = load i32, i32* %y.lpriv                                    (28)
//     store i32 %19, i32* @y                                           (29)
//   }
//
// ------------------------------------------------------------------
// For scheduling involving chunks (dynamic/static/runtime etc.):
//   int y = 0;
//   ...
//   #pragma omp for lastprivate(conditional:y) schedule(dynamic)
//   for (int i = 0; i < 10; i++) {
//     if (<src_condition>) {
//       y = i;
//     }
//     ...
//   }
//
// Generated code would look like:
// ------------------------------------------------------------------
//
//   @y.global.max.idx = private global i32 0                           (30)
//   ...
//
//   %y.local.max.idx = alloca i32                                      (31)
//   store i32 0, i32* %y.local.max.idx                                 (32)
//   %y.modified.by.thread = alloca i1                                  (33)
//   store i1 false, i1* %y.modified.by.thread                          (34)
//   %y.modified.by.chunk = alloca i1                                   (35)
//   store i1 false, i1* %y.modified.by.chunk                           (36)
//   %y.lpriv = alloca i32                                              (37)
//   %y.lpriv.local.last = alloca i32                                   (38)
//   ...
//
//   call void @__kmpc_dispatch_init_4(...)
//
//   DISPATCH_NEXT:
//   %4 = call i32 @__kmpc_dispatch_next_4(...i32* %lower.bnd...)
//   if (%4) {
//     ...
//     store i1 false, i1* %y.modified.by.chunk                         (39)
//     %lb.new = load i32, i32* %lower.bnd                              (40)
//     ...
//     if (%omp.ztt) {
//       for (%lb.new to %ub.new) {
//         ...
//         if (<src_condition>) {
//           ...
//           store i1 true, i1* %y.modified.by.chunk                    (41)
//           store i32 <value_of_i>, i32* %y.lpriv
//         }
//         ...
//       }
//
//       %10 = load i1, i1* %y.modified.by.chunk                        (42)
//       %y.modified = icmp eq i1 %10, true                             (43)
//       %11 = load i32, i32* %y.local.max.idx                          (44)
//       %y.chunk.is.higher = icmp uge i32 %lb.new, %11                 (45)
//       %y.modified.and.chunk.is.higher = and i1 %y.modified,          (46)
//                                                %y.chunk.is.higher
//       if (%y.modified.and.chunk.is.higher) {                         (47)
//         store i1 true, i1* %y.modified.by.thread                     (48)
//         store i32 %lb.new, i32* %y.local.max.idx                     (49)
//         %13 = load i32, i32* %y.lpriv                                (50)
//         store i32 %13, i32* %y.lpriv.local.last                      (51)
//       }
//       goto DISPATCH_NEXT;                                            (52)
//     }
//   }
//
//   %15 = load i1, i1* %y.modified.by.thread                           (53)
//   %y.written.by.thread = icmp eq i1 %15, true                        (54)
//
//   if (%y.written.by.thread) {                                        (55)
//     %17 = load i32, i32* %y.local.max.idx                            (56)
//     call void @__kmpc_critical(...)                                  (57)
//       %18 = load i32, i32* @y.global.max.idx                         (58)
//       %y.is.local.idx.higher = icmp uge i32 %17, %18                 (59)
//       if (%y.is.local.idx.higher) {                                  (60)
//         store i32 %17, i32* @y.global.max.idx                        (61)
//       }
//     call void @__kmpc_end_critical(...)                              (62)
//   }
//
//   call void @__kmpc_barrier(...)                                     (63)
//
//   %22 = load i32, i32* %y.local.max.idx                              (64)
//   %23 = load i32, i32* @y.global.max.idx                             (65)
//   %y.copyout.or.not = icmp eq i32 %22, %23                           (66)
//   if (%y.copyout.or.not) {                                           (67)
//     %25 = load i32, i32* %y.lpriv.local.last                         (68)
//     store i32 %25, i32* %y.lpriv                                     (69)
//     %26 = load i32, i32* %y.lpriv                                    (70)
//     store i32 %26, i32* @y                                           (71)
//   }
//
void VPOParoptTransform::genConditionalLPCode(
    WRegionNode *W, LastprivateItem *LprivI,
    Instruction *OMPLBForChunk,          // (7), (40)
    Instruction *OMPZtt,                 // (8)
    Instruction *BranchToNextChunk,      // (52)
    Instruction *ConditionalLPBarrier) { // (23), (63)

  assert(LprivI && "Null Lastprivate Item");
  assert(OMPLBForChunk && "Null OMP LB for per chunk init code.");
  assert(ConditionalLPBarrier && "Null conditional lastprivate barrier.");

  StringRef NamePrefix = LprivI->getOrig()->getName();
  Loop *L = W->getWRNLoopInfo().getLoop();
  Instruction *LoopIdx = WRegionUtils::getOmpCanonicalInductionVariable(L);
  assert(LoopIdx && "Null loop idx.");
  Type *LoopIdxTy = LoopIdx->getType();
  Instruction *LprivINew = cast<Instruction>(LprivI->getNew()); //    (6), (37)

  // Create variables that keep track of last chunk per thread that modified the
  // variable, whether a variable was ever modified in the current chunk, or
  // whether it was ever modified by the current thread.
  IRBuilder<> AllocaBuilder(LprivINew);
  Instruction *MaxLocalIndex = AllocaBuilder.CreateAlloca(
      LoopIdxTy, nullptr, NamePrefix + ".local.max.idx"); //          (2), (31)
  AllocaBuilder.CreateStore(                              //          (3), (32)
      AllocaBuilder.getIntN(LoopIdxTy->getIntegerBitWidth(), 0), MaxLocalIndex);

  Instruction *ModifiedByCurrentThread = AllocaBuilder.CreateAlloca( // (4), (5)
      AllocaBuilder.getInt1Ty(), nullptr, NamePrefix + ".modified.by.thread");
  AllocaBuilder.CreateStore(AllocaBuilder.getFalse(),
                            ModifiedByCurrentThread); //              (33), (34)

  Instruction *ValInMaxLocalIndex = nullptr;
  Instruction *ModifiedByCurrentChunk = nullptr;

  if (!BranchToNextChunk) { // Static-even scheduling. A thread executes only
                            // one chunk.
    ModifiedByCurrentChunk = ModifiedByCurrentThread;

    // Initialize MaxLocalIndex to omp.lb if omp.ztt is true, otherwise 0.
    assert(OMPZtt && "Null Omp ztt.");
    assert(OMPZtt->getType()->isIntegerTy(1) && "Unexpected type of Omp ztt.");
    IRBuilder<> MaxLocalIndexInitBuilder(OMPZtt->getParent()->getTerminator());
    auto *OMPZttCast = MaxLocalIndexInitBuilder.CreateIntCast(
        OMPZtt, LoopIdxTy, false, "omp.ztt.cast");       //           (9)
    auto *LBOrZero = MaxLocalIndexInitBuilder.CreateMul( //           (10)
        OMPLBForChunk, OMPZttCast, "omp.lb.or.zero");
    MaxLocalIndexInitBuilder.CreateStore(LBOrZero, MaxLocalIndex); // (11)
  } else {
    ValInMaxLocalIndex = LprivINew->clone(); //                       (38)
    ValInMaxLocalIndex->setName(LprivINew->getName() + ".local.last");
    ValInMaxLocalIndex->insertAfter(LprivINew);

    ModifiedByCurrentChunk = AllocaBuilder.CreateAlloca( //           (35)
        AllocaBuilder.getInt1Ty(), nullptr, NamePrefix + ".modified.by.chunk");
    AllocaBuilder.CreateStore(AllocaBuilder.getFalse(),
                              ModifiedByCurrentChunk); //             (36)

    // Generate code that is executed in the beginning of each chunk.
    IRBuilder<> ChunkInitBuilder(OMPLBForChunk);
    ChunkInitBuilder.CreateStore(ChunkInitBuilder.getFalse(),
                                 ModifiedByCurrentChunk); //          (39)
  }

  // Generate code that is executed in the End of each chunk, if needed.
  if (BranchToNextChunk) {
    IRBuilder<> ChunkFiniBuilder(BranchToNextChunk);
    Value *ModifiedByChunk =
        ChunkFiniBuilder.CreateLoad(ModifiedByCurrentChunk); //       (42)
    Value *IsModified = ChunkFiniBuilder.CreateICmpEQ(       //       (43)
        ModifiedByChunk, ChunkFiniBuilder.getTrue(), NamePrefix + ".modified");
    Value *MaxLBToModifyVarForThreadTillNow =
        ChunkFiniBuilder.CreateLoad(MaxLocalIndex); //                (44)
    Value *IsChunkLBGreaterThanMaxLocalIndex = ChunkFiniBuilder.CreateICmpUGE(
        OMPLBForChunk, MaxLBToModifyVarForThreadTillNow,
        NamePrefix + ".chunk.is.higher"); //                          (45)
    Value *ModifiedByHigherChunk = ChunkFiniBuilder.CreateAnd(
        IsModified, IsChunkLBGreaterThanMaxLocalIndex,
        NamePrefix + ".modified.and.chunk.is.higher"); //             (46)

    Instruction *IfModifiedByHigherChunkThen =
        SplitBlockAndInsertIfThen(ModifiedByHigherChunk, BranchToNextChunk,
                                  false, nullptr, DT, LI); //         (47)
    IRBuilder<> ModifiedBuilder(IfModifiedByHigherChunkThen);
    ModifiedBuilder.CreateStore(ModifiedBuilder.getTrue(),
                                ModifiedByCurrentThread);          // (48)
    ModifiedBuilder.CreateStore(OMPLBForChunk, MaxLocalIndex);     // (49)
    auto *ValueInChunk = ModifiedBuilder.CreateLoad(LprivINew);    // (50)
    ModifiedBuilder.CreateStore(ValueInChunk, ValInMaxLocalIndex); // (51)
  }

  // If there is a store to the lastprivate variable, set the flag
  // ModifiedByChunk to true.
  //
  // NOTE: This expects lastprivate code to be generated before firstprivate
  // code. Otherwise, the initialization of the local var for firstprivate would
  // count as a store making the logic incorrect. This already happens for
  // loop/parallel-loops. And conditional lastprivate doesn't work for taskloops
  // at the moment.
  SmallVector<Instruction *, 8> LprivIStores;
  collectStoresToLastprivateNewI(W, LprivI, LprivIStores);

  for (Instruction *LprivIUse : LprivIStores) {
    IRBuilder<> LPrivIModifiedBuilder(LprivIUse);
    LPrivIModifiedBuilder.CreateStore(LPrivIModifiedBuilder.getTrue(),
                                      ModifiedByCurrentChunk); //     (12), (41)
  }

  // No participation in max computation is needed if the current thread didn't
  // write to the var.
  IRBuilder<> GlobalMaxComputationBuilder(ConditionalLPBarrier);
  auto *ModifiedByCurrentThreadLoad =
      GlobalMaxComputationBuilder.CreateLoad(ModifiedByCurrentThread); //  (53)
  auto *DidThreadWriteAnything = GlobalMaxComputationBuilder.CreateICmpEQ(
      ModifiedByCurrentThreadLoad, GlobalMaxComputationBuilder.getTrue(),
      NamePrefix + ".written.by.thread"); //                               (54)
  Instruction *IfThreadWroteSomethingThen = SplitBlockAndInsertIfThen( //  (55)
      DidThreadWriteAnything, ConditionalLPBarrier, false, nullptr, DT, LI);

  // Create global variable to store the global max idx and use
  // reduction-using-critical to set it after a thread is done with all its
  // chunks.
  GlobalMaxComputationBuilder.SetInsertPoint(IfThreadWroteSomethingThen);
  GlobalVariable *MaxGlobalIndex = new GlobalVariable(
      *ConditionalLPBarrier->getModule(), LoopIdxTy, false /*not constant*/,
      GlobalValue::PrivateLinkage,
      GlobalMaxComputationBuilder.getIntN(LoopIdxTy->getIntegerBitWidth(), 0),
      NamePrefix + ".global.max.idx"); //                             (1), (30)

  auto *FinalLocalMaxIndex =
      GlobalMaxComputationBuilder.CreateLoad(MaxLocalIndex); //       (56)
  auto *GlobalMaxIndexLoad =
      GlobalMaxComputationBuilder.CreateLoad(MaxGlobalIndex); //      (58)
  auto *IsLocalGreaterThanGlobal = GlobalMaxComputationBuilder.CreateICmpUGE(
      FinalLocalMaxIndex, GlobalMaxIndexLoad,
      NamePrefix + ".is.local.idx.higher"); //                        (59)
  Instruction *IfHighestChunkIsModifiedByThreadThen = SplitBlockAndInsertIfThen(
      IsLocalGreaterThanGlobal, IfThreadWroteSomethingThen, false, nullptr, DT,
      LI); //                                                         (60)
  StoreInst *MaxStore = new StoreInst(FinalLocalMaxIndex, MaxGlobalIndex);
  MaxStore->insertBefore(IfHighestChunkIsModifiedByThreadThen); //    (61)

  // TODO: This critical section should be switched with either atomic-max
  // reduction, or atomic operation
  GlobalMaxComputationBuilder.SetInsertPoint(ConditionalLPBarrier);
  VPOParoptUtils::genKmpcCriticalSection(
      W, IdentTy, TidPtrHolder, GlobalMaxIndexLoad, IfThreadWroteSomethingThen,
      isTargetSPIRV(), (NamePrefix + ".max.lock.var").str()); //      (57), (62)

  // Now emit the final copyout code, which checks if the current thread's max
  // local index is same as the final global max index, and it actually wrote to
  // the variable, and if so, copies the final value of the lastprivate variable
  // back to the original.
  Instruction *BarrierSuccessorInst =
      GeneralUtils::nextUniqueInstruction(ConditionalLPBarrier);
  IRBuilder<> LPCopyoutBuilder(BarrierSuccessorInst);
  auto *FinalMaxLocalIndexLoad =
      LPCopyoutBuilder.CreateLoad(MaxLocalIndex); //                  (24), (64)
  auto *FinalMaxGlobalIndexLoad =
      LPCopyoutBuilder.CreateLoad(MaxGlobalIndex); //                 (25), (65)
  auto *ShouldThreadDoCopyout = LPCopyoutBuilder.CreateICmpEQ(
      FinalMaxLocalIndexLoad, FinalMaxGlobalIndexLoad,
      NamePrefix + ".copyout.or.not"); //                             (26), (66)
  Instruction *IfShouldDoCopyoutThen = SplitBlockAndInsertIfThen( //  (27), (67)
      ShouldThreadDoCopyout, BarrierSuccessorInst, false, nullptr, DT, LI);

  if (ValInMaxLocalIndex) {
    // Do copyout using ValInMaxLocalIndex instead of LprivINew.
    IRBuilder<> CopyoutBuilder(IfShouldDoCopyoutThen);
    auto *ValInMaxLocalIndexLoad =
        CopyoutBuilder.CreateLoad(ValInMaxLocalIndex); //             (68)
    CopyoutBuilder.CreateStore(ValInMaxLocalIndexLoad, LprivINew); // (69)
  }

  genLprivFini(LprivI, IfShouldDoCopyoutThen); // (28), (29), (70), (71)

  LLVM_DEBUG(dbgs() << __FUNCTION__
                    << ": Emitted Conditional Lastprivate code for '";
             LprivI->getOrig()->printAsOperand(dbgs()); dbgs() << "'.\n");

  return;
}

// Generate the reduction initialization code.
// Here is one example for the reduction initialization for scalar.
//   sum = 4.0;
//   #pragma omp parallel for reduction(+:sum)
//   for (i=0; i < n; i++)
//     sum = sum + (a[i] * b[i]);
//
// The output of the reduction initialization for the variable sum
// is as follows.
//
//    /* B[%DIR.OMP.PARALLEL.LOOP.1.split]  */
//    store float 0.000000e+00, float* %sum.red
//    br label %dir.exit
//
void VPOParoptTransform::genReductionInit(WRegionNode *W,
                                          ReductionItem *RedI,
                                          Instruction *InsertPt,
                                          DominatorTree *DT) {
  Value *AI = RedI->getNew();
  // AI is either AllocaInst or an AddrSpaceCastInst of AllocaInst.
  assert(GeneralUtils::isOMPItemLocalVAR(AI) &&
         "genReductionInit: Expect non-empty optionally casted "
         "alloca instruction.");
  Type *AllocaTy =
      GeneralUtils::getOMPItemLocalVARPointerType(AI)->getElementType();
  Type *ScalarTy = AllocaTy->getScalarType();

  IRBuilder<> Builder(InsertPt);
  if (RedI->getIsArraySection() ||
      AllocaTy->isArrayTy())
    genRedAggregateInitOrFini(W, RedI, AI, nullptr, InsertPt, true, DT);
  else {
    assert(VPOUtils::canBeRegisterized(AllocaTy,
           InsertPt->getModule()->getDataLayout()) &&
           "genReductionInit: Expect incoming scalar type.");
    Value *V = genReductionScalarInit(RedI, ScalarTy);
    Builder.CreateStore(V, AI);
  }
}

// Prepare the empty basic block for the array reduction initialization.
void VPOParoptTransform::createEmptyPrvInitBB(WRegionNode *W,
                                              BasicBlock *&PrivBB) {
  BasicBlock *EntryBB = W->getEntryBBlock();
  PrivBB = SplitBlock(EntryBB, EntryBB->getTerminator(), DT, LI);
}

// Prepare the empty basic block for the array reduction update.
void VPOParoptTransform::createEmptyPrivFiniBB(WRegionNode *W,
                                               BasicBlock *&PrivEntryBB,
                                               bool HonorZTT) {
  BasicBlock *ExitBlock = W->getExitBBlock();
  BasicBlock *PrivExitBB;
  if (W->getIsOmpLoop()) {
    // If the loop has ztt block, the compiler has to generate the lastprivate
    // update code at the exit block of the loop.
    BasicBlock *ZttBlock = W->getWRNLoopInfo().getZTTBB();

    if (ZttBlock && HonorZTT) {
      while (distance(pred_begin(ExitBlock), pred_end(ExitBlock)) == 1)
        ExitBlock = *pred_begin(ExitBlock);
      assert(distance(pred_begin(ExitBlock), pred_end(ExitBlock)) == 2 &&
             "Expect two predecessors for the omp loop region exit.");
      auto PI = pred_begin(ExitBlock);
      auto Pred1 = *PI++;
      auto Pred2 = *PI++;

      BasicBlock *LoopExitBB = nullptr;
      if (Pred1 == ZttBlock && Pred2 != ZttBlock)
        LoopExitBB = Pred2;
      else if (Pred2 == ZttBlock && Pred1 != ZttBlock)
        LoopExitBB = Pred1;
      else
        llvm_unreachable("createEmptyPrivFiniBB: unsupported exit block");
      PrivExitBB = SplitBlock(LoopExitBB, LoopExitBB->getTerminator(), DT, LI);
      PrivEntryBB = PrivExitBB;
      return;
    }
  }
  PrivExitBB = SplitBlock(ExitBlock, ExitBlock->getFirstNonPHI(), DT, LI);
  W->setExitBBlock(PrivExitBB);
  PrivEntryBB = ExitBlock;
}

// Generate the reduction code for reduction clause.
bool VPOParoptTransform::genReductionCode(WRegionNode *W) {
  bool Changed = false;
  SetVector<Value *> RedUses;

  BasicBlock *EntryBB = W->getEntryBBlock();

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genReductionCode\n");

  ReductionClause &RedClause = W->getRed();
  if (!RedClause.empty()) {

    W->populateBBSet();

    bool NeedsKmpcCritical = false;
    BasicBlock *RedInitEntryBB = nullptr;
    BasicBlock *RedUpdateEntryBB = nullptr;
    // Insert reduction update at the region's exit block
    // for SPIRV target, so that potential __kmpc_[end_]critical
    // calls are executed the same number of times for all work
    // items. The results should be the same, as long as the reduction
    // variable is initialized at the region's entry block.
    createEmptyPrivFiniBB(W, RedUpdateEntryBB, !isTargetSPIRV());

    for (ReductionItem *RedI : RedClause.items()) {
      Value *NewRedInst;
      Value *Orig = RedI->getOrig();

/*
      assert((isa<GlobalVariable>(Orig) || isa<AllocaInst>(Orig)) &&
             "genReductionCode: Unexpected reduction variable");
*/

      Instruction *InsertPt = &EntryBB->front();

      computeArraySectionTypeOffsetSize(*RedI, InsertPt);

      NewRedInst = genPrivatizationAlloca(RedI, InsertPt, ".red");
      RedI->setNew(NewRedInst);

      Value *ReplacementVal = getClauseItemReplacementValue(RedI, InsertPt);
      genPrivatizationReplacement(W, Orig, ReplacementVal);

      createEmptyPrvInitBB(W, RedInitEntryBB);
      genReductionInit(W, RedI, RedInitEntryBB->getTerminator(), DT);

      BasicBlock *BeginBB;
      createEmptyPrivFiniBB(W, BeginBB, !isTargetSPIRV());
      NeedsKmpcCritical |= genReductionFini(W, RedI, RedI->getOrig(),
                                            BeginBB->getTerminator(), DT);

      LLVM_DEBUG(dbgs() << "genReductionCode: reduced " << *Orig << "\n");
    }

    if (NeedsKmpcCritical) {
      // Wrap the reduction fini code inside a critical region.
      // EndBB is created to be used as the insertion point for end_critical().
      //
      // This insertion point cannot be W->getExitBBlock()->begin() because
      // we don't want the END DIRECTIVE of the construct to be inside the
      // critical region
      //
      // This insertion point cannot be BeginBB->getTerminator() either, which
      // would work for scalar reduction but not for array reduction, in which
      // case the end_critical() would get emitted before the copy-out loop that
      // the critical section is trying to guard.
      BasicBlock *EndBB;
      createEmptyPrivFiniBB(W, EndBB, !isTargetSPIRV());
      VPOParoptUtils::genKmpcCriticalSection(
          W, IdentTy, TidPtrHolder,
          dyn_cast<Instruction>(RedUpdateEntryBB->begin()),
          EndBB->getTerminator(), isTargetSPIRV(), ".reduction");

      OptimizationRemark R(DEBUG_TYPE, "Reduction", &EntryBB->front());
      R << "Critical section was generated for reduction update(s)";
      ORE.emit(R);
    }

    Changed = true;
  } // if (!RedClause.empty())
  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genReductionCode\n");

  W->resetBBSetIfChanged(Changed); // Clear BBSet if transformed
  return Changed;
}

// For array [section] reduction init loop, compute the base address of the
// destination array, number of elements, and destination element type.
void VPOParoptTransform::genAggrReductionInitDstInfo(
    const ReductionItem &RedI, Value *AI, Instruction *InsertPt,
    IRBuilder<> &Builder, Value *&NumElements, Value *&DestArrayBegin,
    Type *&DestElementTy) {

  bool IsArraySection = RedI.getIsArraySection();

  if (IsArraySection) {
    const ArraySectionInfo &ArrSecInfo = RedI.getArraySectionInfo();
    NumElements = ArrSecInfo.getSize();
    DestElementTy = ArrSecInfo.getElementType();
    DestArrayBegin = RedI.getNew();
  } else
    NumElements = VPOParoptUtils::genArrayLength(AI, AI, InsertPt, Builder,
                                                 DestElementTy, DestArrayBegin);

  auto *DestPointerTy = DestArrayBegin->getType();
  assert(isa<PointerType>(DestPointerTy) &&
         "Reduction destination must have pointer type.");
  DestArrayBegin = Builder.CreateBitCast(
      DestArrayBegin,
      PointerType::get(DestElementTy,
                       cast<PointerType>(DestPointerTy)->getAddressSpace()));
}

// For array [section] reduction finalization loop, compute the base address
// of the source and destination arrays, number of elements, and the type of
// destination array elements.
void VPOParoptTransform::genAggrReductionFiniSrcDstInfo(
    const ReductionItem &RedI, Value *AI, Value *OldV,
    Instruction *InsertPt, IRBuilder<> &Builder, Value *&NumElements,
    Value *&SrcArrayBegin, Value *&DestArrayBegin, Type *&DestElementTy) {

  bool IsArraySection = RedI.getIsArraySection();
  Type *SrcElementTy = nullptr;

  if (!IsArraySection) {
    NumElements = VPOParoptUtils::genArrayLength(AI, OldV, InsertPt, Builder,
                                                 DestElementTy, DestArrayBegin);
    auto *DestPointerTy = DestArrayBegin->getType();
    assert(isa<PointerType>(DestPointerTy) &&
           "Reduction destination must have pointer type.");
    DestArrayBegin = Builder.CreateBitCast(
        DestArrayBegin,
        PointerType::get(DestElementTy,
                         cast<PointerType>(DestPointerTy)->getAddressSpace()));

    VPOParoptUtils::genArrayLength(AI, AI, InsertPt, Builder, SrcElementTy,
                                   SrcArrayBegin);
    auto *SrcPointerTy = SrcArrayBegin->getType();
    assert(isa<PointerType>(SrcPointerTy) &&
           "Reduction source must have pointer type.");
    SrcArrayBegin = Builder.CreateBitCast(
        SrcArrayBegin,
        PointerType::get(SrcElementTy,
                         cast<PointerType>(SrcPointerTy)->getAddressSpace()));
    return;
  }

  // Example for an array section on a pointer to an array:
  //
  //   static int (*yarrptr)[3][4][5];
  //   #pragma omp parallel for reduction(+:yarrptr[3][1][2:2][1:3])
  //
  const ArraySectionInfo &ArrSecInfo = RedI.getArraySectionInfo();
  NumElements = ArrSecInfo.getSize();          // 6 for the above example
  DestElementTy = ArrSecInfo.getElementType(); // i32 for the above example

  SrcElementTy = DestElementTy;
  SrcArrayBegin = RedI.getNew();
  auto *SrcPointerTy = SrcArrayBegin->getType();
  assert(isa<PointerType>(SrcPointerTy) &&
         "Reduction source must have pointer type.");
  SrcArrayBegin = Builder.CreateBitCast(
      SrcArrayBegin,
      PointerType::get(SrcElementTy,
                       cast<PointerType>(SrcPointerTy)->getAddressSpace()));

  // Generated IR for destination starting address for the above example:
  //
  //   %_yarrptr.load = load [3 x [4 x [5 x i32]]]*, @_yarrptr          ; (1)
  //   %_yarrptr.load.cast = bitcast %_yarrptr.load to i32*             ; (2)
  //   %_yarrptr.load.cast.plus.offset = gep %_yarrptr.load.cast, 211   ; (3)
  //
  //   %_yarrptr.load.cast.plus.offset is the final DestArrayBegin.

  DestArrayBegin = OldV;
  bool ArraySectionBaseIsPtr = ArrSecInfo.getBaseIsPointer();
  if (ArraySectionBaseIsPtr)
    DestArrayBegin = Builder.CreateLoad(
        DestArrayBegin, DestArrayBegin->getName() + ".load"); //          (1)

  assert(DestArrayBegin && isa<PointerType>(DestArrayBegin->getType()) &&
         "Illegal Destination Array for reduction fini.");

  auto *DestPointerTy = DestArrayBegin->getType();
  assert(isa<PointerType>(DestPointerTy) &&
         "Reduction destination must have pointer type.");
  DestArrayBegin = Builder.CreateBitCast(
      DestArrayBegin,
      PointerType::get(DestElementTy,
                       cast<PointerType>(DestPointerTy)->getAddressSpace()),
      DestArrayBegin->getName() + ".cast"); //                            (2)
  DestArrayBegin =
      Builder.CreateGEP(DestArrayBegin, ArrSecInfo.getOffset(),
                        DestArrayBegin->getName() + ".plus.offset"); //   (3)
}

void VPOParoptTransform::computeArraySectionTypeOffsetSize(
    Item &CI, Instruction *InsertPt) {

  bool IsArraySection = false;
  ArraySectionInfo *ArrSecInfo = nullptr;

  if (auto *RI = dyn_cast<ReductionItem>(&CI)) {
    IsArraySection = RI->getIsArraySection();
    ArrSecInfo = &RI->getArraySectionInfo();
  } else if (auto *MI = dyn_cast<MapItem>(&CI)) {
    IsArraySection = MI->getIsArraySection();
    ArrSecInfo = &MI->getArraySectionInfo();
  } else
    llvm_unreachable("computeArraySectionTypeOffsetSize: the clause Item "
                     "must be either ReductionItem or MapItem.");

  if (!IsArraySection)
    return;

  IRBuilder<> Builder(InsertPt);

  Value *Orig = CI.getOrig();
  Type *CITy = Orig->getType();
  Type *ElemTy = cast<PointerType>(CITy)->getElementType();

  if (CI.getIsByRef())
    // Strip away one pointer for by-refs.
    ElemTy = cast<PointerType>(ElemTy)->getElementType();

  bool BaseIsPointer = false;
  if (isa<PointerType>(ElemTy)) {
    // It is possible to have an array section on a pointer. Examples:
    //
    // int *yptr, (*yarrptr)[10];
    // reduction(+:yptr[1:4], yarrptr[1][2:5])
    //
    // In these cases, the IR will have the type of the operands as `**`:
    //
    // "...REDUCTION.ADD:ARRSECT"(i32** @yptr, 1, 1, 4, 1)
    // "...REDUCTION.ADD:ARRSECT"([10 x i32]** @yarrptr, 2, 1, 1, 1, 2, 5, 1)
    //
    // In these cases, the we need to get the pointee type one extra time to
    // reach the base element type (i32 for yptr) or the array type ([10 x i32]
    // for yarrptr).
    BaseIsPointer = true;
    ElemTy = cast<PointerType>(ElemTy)->getElementType();
  }

  // At this point, ElemTy is the base element type for 1D array sections. For
  // sections with 2 or more dimensions, it should have the underlying array
  // type. If so, we need to extract the size of each dimension of that array.
  // We do that next. At the end of the following loop, ArrayDims should have
  // the size of each dimension of the underlying array from higher to lower.
  // For example, it should contain {3, 4, 5} for `[3 x [4 x [5 x i32]]]`. We
  // will use this to compute the offset in terms of an equivalent 1D array.
  Type *CurElementTy = ElemTy;
  SmallVector<uint64_t, 4> ArrayDims;
  while (auto *ArrayTy = dyn_cast<ArrayType>(CurElementTy)) {
    ArrayDims.push_back(ArrayTy->getNumElements());
    CurElementTy = ArrayTy->getElementType();
  }

  // This is the number to be multiplied to a dimension's lower bound during
  // offset computation.
  const DataLayout &DL = InsertPt->getModule()->getDataLayout();
  const unsigned PtrSz = DL.getPointerSizeInBits();

  uint64_t ArraySizeTillDim = 1;
  Value *ArrSecSize = Builder.getIntN(PtrSz, 1);
  Value *ArrSecOff = Builder.getIntN(PtrSz, 0);
  const auto &ArraySectionDims = ArrSecInfo->getArraySectionDims();
  const int NumDims = ArraySectionDims.size();

  // We go through the array section dims in the reverse order to go from lower
  // to higher dimensions. For example, in case of:
  //
  //   int (*zarrptr)[3][4][5];
  //   #pragma omp for reduction (+:zarrptr[3][1][2:2][1:3]).
  //
  // Array section size is a simple multiplication of the size of each
  // dimension, ie. 3*2*1*1 = 6. The offset and element type are computed in the
  // loop below. At the beginning of each iteration, the values will look like
  // this (note that `BaseIsPointer` is true for this case):
  //
  //  I | LB | ArraySizeTillDim | ArrSecOff | ArrayDims | ElemTy
  // ---+----+------------------+-----------+-----------+-----------------------
  //  3 | 1  | 1                | 0         | {3,4,5}   | [3 x [4 x [5 x i32 ]]]
  //  2 | 2  | 5                | 1         | {3,4}     | [4 x [5 x i32 ]]
  //  1 | 1  | 20               | 11        | {3}       | [5 x i32 ]
  //  0 | 3  | 60               | 31        | {}        | i32
  // --------+------------------+-----------+-----------+-----------------------
  // final   | 60               | 221       | {}        | i32
  //
  for (int I = NumDims - 1; I >= 0; --I) {
    auto const &Dim = ArraySectionDims[I];

    Value *DimLB = std::get<0>(Dim);
    Value *SectionDimSize = std::get<1>(Dim);

    ConstantInt *ArraySizeTillDimVal = Builder.getIntN(PtrSz, ArraySizeTillDim);

    Value *SizeXLB = Builder.CreateMul(ArraySizeTillDimVal, DimLB);
    ArrSecOff = Builder.CreateAdd(SizeXLB, ArrSecOff, "offset");
    ArrSecSize = Builder.CreateMul(ArrSecSize, SectionDimSize, "size");

    if (I == 0 && BaseIsPointer)
      continue; // If `BaseIsPointer`, getElmentType() has already been called
                // once, so we skip it in the last iteration.

    ArraySizeTillDim *= ArrayDims.pop_back_val();
    ElemTy = cast<ArrayType>(ElemTy)->getElementType();
  }

  // TODO: This assert needs to be updated when UDR support is added.
  assert(!isa<PointerType>(ElemTy) && !isa<ArrayType>(ElemTy) &&
         "Unexpected array section element type.");

  ArrSecInfo->setSize(ArrSecSize);
  ArrSecInfo->setOffset(ArrSecOff);
  ArrSecInfo->setElementType(ElemTy);
  ArrSecInfo->setBaseIsPointer(BaseIsPointer);

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Operand '";
             Orig->printAsOperand(dbgs()); dbgs() << "':: ";
             ArrSecInfo->print(dbgs(), false); dbgs() << "\n");
}

Value *
VPOParoptTransform::getClauseItemReplacementValue(Item *ClauseI,
                                                  Instruction *InsertPt) {

  assert(ClauseI && "Null clause item.");

  Value *ReplacementVal = nullptr;
  bool IsByref = ClauseI->getIsByRef();
  bool IsArraySection = isa<ReductionItem>(ClauseI) &&
                        cast<ReductionItem>(ClauseI)->getIsArraySection();

  if (IsArraySection)
    ReplacementVal = VPOParoptTransform::getArrSecReductionItemReplacementValue(
        *(cast<ReductionItem>(ClauseI)), InsertPt);
  else if (ClauseI->getNewOnTaskStack())
    ReplacementVal = ClauseI->getNewOnTaskStack();
  else
    ReplacementVal = ClauseI->getNew();

  // For a by-ref, we need to add an extra address-of before replacing the
  // original value with the local value.
  if (IsByref) {
    IRBuilder<> Builder(InsertPt);
    AllocaInst *ByRefAddr = Builder.CreateAlloca(
        ReplacementVal->getType(), nullptr, ReplacementVal->getName() + ".ref");
    Builder.CreateStore(ReplacementVal, ByRefAddr);
    ReplacementVal = ByRefAddr;
  }

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Replacement Value for '";
             ClauseI->getOrig()->printAsOperand(dbgs()); dbgs() << "':: ";
             ReplacementVal->printAsOperand(dbgs()); dbgs() << "\n");
  return ReplacementVal;
}

Value *VPOParoptTransform::getArrSecReductionItemReplacementValue(
    ReductionItem const &RedI, Instruction *InsertPt) {
  // For array section reduction, such as:
  //
  //   int y[10];
  //   #pragma omp for reduction(+: y[offset:5])
  //
  // The local copy of the operand is a VLA of size 5: `y.new = alloca i32, 5`
  // Accesses to `y[offset]` should point to `y.new[0]`. So, the uses of `y`
  // need to be replaced with `y.new - offset` inside the region:
  //   %y.new.minus.offset = getelementptr %y.new, -offset              ; (1)

  IRBuilder<> Builder(InsertPt);
  Value *NewRedInst = RedI.getNew();
  const ArraySectionInfo &ArrSecInfo = RedI.getArraySectionInfo();
  Value *Offset = ArrSecInfo.getOffset();
  Value *NegOffset = Builder.CreateNeg(Offset, "neg.offset");
  Value *NewMinusOffset = Builder.CreateGEP(
      NewRedInst, NegOffset, NewRedInst->getName() + ".minus.offset"); // (1)

  if (!ArrSecInfo.getBaseIsPointer()) {
    // To replace uses of original %y ( [10 x i32]* with %y.new.minus.offset
    // (i32*), we need to create a bitcast to the type of $y. For by-refs,
    // we create this cast to the pointee of the original item.
    Value *Orig = RedI.getOrig();
    Type *OrigArrayType =
        RedI.getIsByRef()
            ? cast<PointerType>(Orig->getType())->getPointerElementType()
            : Orig->getType();
    return Builder.CreateBitCast(NewMinusOffset, OrigArrayType,
                                 NewMinusOffset->getName());
  }

  // In case of array section on a pointer, like:
  //
  //   int *x;
  //   #pragma omp for reduction(+:x[1:5])
  //
  // The type of `x` in IR is `i32**`. So an access to x[1] in the IR will look
  // like:
  //
  //   %1 = load i32* %x
  //   %2 = getelementpointer %1, 1
  //
  // However, the type of the local copy `x.new` is i32*. So x needs to be
  // replaced with address of 'x.new - offset':
  //
  //   %x.new = alloca i32, 5
  //   %x.new.minus.offset = getelementpointer %x.new, -1               ; (1)
  //   %x.new.minus.offset.addr = alloca i32*                           ; (2)
  //   store %x.new.minus.offset, %x.new.minus.offset.addr              ; (3)
  //
  const DataLayout &DL = InsertPt->getModule()->getDataLayout();
  AllocaInst *NewMinusOffsetAddr = Builder.CreateAlloca(
      PointerType::get(ArrSecInfo.getElementType(), DL.getAllocaAddrSpace()),
      nullptr, NewMinusOffset->getName() + ".addr");       //             (2)
  Builder.CreateStore(NewMinusOffset, NewMinusOffsetAddr); //             (3)

  return NewMinusOffsetAddr;
}

// Extract the type and size of local Alloca to be created to privatize
// OrigValue.
void VPOParoptTransform::getItemInfoFromValue(Value *OrigValue,
                                              Type *&ElementType,    // out
                                              Value *&NumElements,   // out
                                              unsigned &AddrSpace) { // out

  assert(OrigValue && "Null input value.");

  ElementType = nullptr;
  NumElements = nullptr;

  if (AllocaInst *AI = dyn_cast<AllocaInst>(OrigValue)) {
    ElementType = AI->getAllocatedType();
    AddrSpace = AI->getType()->getAddressSpace();
    if (AI->isArrayAllocation())
      NumElements = AI->getArraySize();

    return;
  }

  if (GlobalVariable *GV = dyn_cast<GlobalVariable>(OrigValue)) {
    ElementType = GV->getValueType();
    AddrSpace = GV->getAddressSpace();
    return;
  }

  assert(
      (isa<Argument>(OrigValue) || isa<GetElementPtrInst>(OrigValue) ||
       isa<LoadInst>(OrigValue) || isa<BitCastInst>(OrigValue) ||
       GeneralUtils::isOMPItemLocalVAR(OrigValue) ||
       (isa<CallInst>(OrigValue) && isFenceCall(cast<CallInst>(OrigValue)))) &&
      "unsupported input Value");

  ElementType = cast<PointerType>(OrigValue->getType())->getElementType();
  AddrSpace = cast<PointerType>(OrigValue->getType())->getAddressSpace();
}

// Generate an AllocaInst for an array of Type ElementType, size NumElements,
// and name VarName.
Instruction *VPOParoptTransform::genPrivatizationAlloca(Type *ElementType,
                                                        Value *NumElements,
                                                        Instruction *InsertPt,
                                                        const Twine &VarName,
                                                        unsigned AddrSpace) {
  assert(ElementType && "Null element type.");
  assert(InsertPt && "Null insertion anchor.");

  Module *M = InsertPt->getModule();
  const DataLayout &DL = M->getDataLayout();
  IRBuilder<> Builder(InsertPt);

  auto AI = Builder.CreateAlloca(ElementType, DL.getAllocaAddrSpace(),
                                 NumElements, VarName);

  if (AddrSpace == 0)
    return AI;

  auto *ASCI = dyn_cast<Instruction>(
      Builder.CreateAddrSpaceCast(
          AI, AI->getAllocatedType()->getPointerTo(AddrSpace),
          AI->getName() + ".ascast"));

  assert(ASCI && "genPrivatizationAlloca: AddrSpaceCast for an AllocaInst "
         "must be an Instruction.");

  return ASCI;
}

// Extract the type and size of local Alloca to be created to privatize I.
void VPOParoptTransform::getItemInfo(Item *I,
                                     Type *&ElementType,    // out
                                     Value *&NumElements,   // out
                                     unsigned &AddrSpace) { // out
  assert(I && "Null Clause Item.");

  Value *Orig = I->getOrig();
  assert(Orig && "Null original Value in clause item.");

  auto getItemInfoIfArraySection = [I, &ElementType, &NumElements,
                                    &AddrSpace]() -> bool {
    if (ReductionItem *RedI = dyn_cast<ReductionItem>(I))
      if (RedI->getIsArraySection()) {
        const ArraySectionInfo &ArrSecInfo = RedI->getArraySectionInfo();
        ElementType = ArrSecInfo.getElementType();
        NumElements = ArrSecInfo.getSize();
        auto *ItemTy = RedI->getOrig()->getType();
        assert(isa<PointerType>(ItemTy) &&
               "Array section item has to have pointer type.");
        AddrSpace = cast<PointerType>(ItemTy)->getAddressSpace();
        return true;
      }
    return false;
  };

  if (!getItemInfoIfArraySection()) {
    getItemInfoFromValue(Orig, ElementType, NumElements, AddrSpace);
    assert(ElementType && "Failed to find element type for reduction operand.");

    if (I->getIsByRef()) {
      assert(isa<PointerType>(ElementType) &&
             "Expected a pointer type for byref operand.");
      assert(!NumElements &&
             "Unexpected number of elements for byref pointer.");

      ElementType = cast<PointerType>(ElementType)->getPointerElementType();
    }
  }
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Local Element Info for '";
             Orig->printAsOperand(dbgs()); dbgs() << "':: Type: ";
             ElementType->print(dbgs()); if (NumElements) {
               dbgs() << ", NumElements: ";
               NumElements->printAsOperand(dbgs());
             } dbgs() << "\n");
}

// Generate an AllocaInst for the local copy of OrigValue.
Instruction *VPOParoptTransform::genPrivatizationAlloca(
    Value *OrigValue, Instruction *InsertPt, const Twine &NameSuffix,
    bool ForceDefaultAddressSpace) {

  assert(OrigValue && "genPrivatizationAlloca: Null input value.");

  Type *ElementType = nullptr;
  Value *NumElements = nullptr;
  unsigned AddrSpace = 0;

  getItemInfoFromValue(OrigValue, ElementType, NumElements, AddrSpace);
  auto *NewVal = genPrivatizationAlloca(
      ElementType, NumElements, InsertPt, OrigValue->getName() + NameSuffix,
      ForceDefaultAddressSpace ? 0 : AddrSpace);
  assert(NewVal && "Failed to create local copy.");

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": New Alloca for operand '";
             OrigValue->printAsOperand(dbgs()); dbgs() << "':: ";
             NewVal->printAsOperand(dbgs()); dbgs() << "\n");
  return NewVal;
}

// Generate an AllocaInst for the local copy of ClauseItem I for various
// data-sharing clauses.
Instruction *VPOParoptTransform::genPrivatizationAlloca(
    Item *I, Instruction *InsertPt,
    const Twine &NameSuffix,
    bool ForceDefaultAddressSpace) {
  assert(I && "Null Clause Item.");

  Value *Orig = I->getOrig();
  assert(Orig && "Null original Value in clause item.");

  Type *ElementType = nullptr;
  Value *NumElements = nullptr;
  unsigned AddrSpace = 0;

  getItemInfo(I, ElementType, NumElements, AddrSpace);
  assert(ElementType && "Could not find Type of local element.");

  auto *NewVal = genPrivatizationAlloca(
      ElementType, NumElements, InsertPt, Orig->getName() + NameSuffix,
      ForceDefaultAddressSpace ? 0 : AddrSpace);
  assert(NewVal && "Failed to create local copy.");

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": New Alloca for operand '";
             Orig->printAsOperand(dbgs()); dbgs() << "':: ";
             NewVal->printAsOperand(dbgs()); dbgs() << "\n");
  return NewVal;
}

// Replace the variable with the privatized variable
void VPOParoptTransform::genPrivatizationReplacement(WRegionNode *W,
                                                     Value *PrivValue,
                                                     Value *NewPrivValue) {

  // Find instructions in W that use V
  SmallVector<Instruction *, 8> PrivUses;
  if (!WRegionUtils::findUsersInRegion(W, PrivValue, &PrivUses, false))
    return; // Found no applicable uses of PrivValue in W's body

  // Replace all USEs of each PrivValue with its NewPrivValue in the
  // W-Region (parallel loop/region/section ... etc.)
  while (!PrivUses.empty()) {
    Instruction *UI = PrivUses.pop_back_val();
    UI->replaceUsesOfWith(PrivValue, NewPrivValue);

    if (GeneralUtils::isOMPItemGlobalVAR(PrivValue)) {
      // If PrivValue is a global, its uses could be in ConstantExprs
      SmallVector<Instruction *, 2> NewInstArr;
      GeneralUtils::breakExpressions(UI, &NewInstArr);
      for (Instruction *NewInstr : NewInstArr) {
        NewInstr->replaceUsesOfWith(PrivValue, NewPrivValue);
      }
    }
  }

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Replaced uses of '";
             PrivValue->printAsOperand(dbgs()); dbgs() << "' with '";
             NewPrivValue->printAsOperand(dbgs()); dbgs() << "'\n");
}

// Generates code for linear variables for the WRegion W.
//
// The following needs to be done for handling a linear var:
//
//
// * (A) Create two local copies of the linear vars. One to capture the starting
// value. Another to be the local linear variable which replaces all uses of the
// original inside the region. (1), (2)
//
// * (B) Capture original value of linear vars before entering the loop. (1),
// (3), (4)
//
// * (C) Use the captured value along with the specified step to initialize the
// local linear var in each iteration of the loop. (5) - (11)
//
// * (D) At the end of the last loop iteration, copy the value of the local var
// back to the original linear var. (12), (13)
//
//  +------------EntryBB----------------+
//  |  %linear.start = alloca i16       |                            ; (1)
//  |  %y = alloca i16                  |                            ; (2)
//  +----------------+------------------+
//                   |
//  +-----------LinearInitBB------------+
//  |  %2 = load i16, i16* @y           |                            ; (3)
//  |  store i16 %2, i16* %linear.start |                            ; (4)
//  +----------------+------------------+
//                    |
//           __kmpc_barrier(...)    ; inserted by genBarrierForFpLpAndLinears()
//                   |
//         __kmpc_static_init(...)
//                   |
//  +-----------LoopBodyBB--------------+
//  |  %omp.ivi.0 = phi ...             |
//  |  %6 = load i16, i16* %linear.start|                            ; (5)
//  |  %7 = sext i16 %step to i64       |                            ; (6)
//  |  %8 = mul i64 %.omp.iv.0, i64 %7  |                            ; (7)
//  |  %9 = sext i16 %6 to i64          |                            ; (8)
//  |  %10 = add i64 %9, %8             |                            ; (9)
//  |  %11 = trunc i64 %10 to i16       |                            ; (10)
//  |  store i16 %11, i16* %y           |                            ; (11)
//  |                ...                |
//  +-----------------+-----------------+
//                    |
//         __kmpc_static_fini(...)
//                    |
//               if(%is_last)       ; inserted by genLastIterationCheck()
//                    |   \
//                   yes   no
//                    |     +---------+
//                    |               |
//     +--------LinearFiniBB------+   |
//     |   %17 = load i16, i16* %y|   |                              ; (12)
//     |   store i16 %17, i16* @y |   |                              ; (13)
//     +--------------+-----------+   |
//                    |               |
//                    |               |
//                    |  +------------+
//                    |  |
//     +--------------+--+--------+
//     |   llvm.region.exit(...)  |
//     +--------------------------+
//
bool VPOParoptTransform::genLinearCode(WRegionNode *W, BasicBlock *LinearFiniBB,
                                       Instruction *OMPLBForLinearClosedForm) {
  if (!W->canHaveLinear())
    return false;

  LinearClause &LrClause = W->getLinear();
  if (LrClause.empty())
    return false;

  assert(LinearFiniBB && "genLinearCode: Null LinearFiniBB.");

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genLinearCode\n");

  W->populateBBSet();
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *LinearInitBB = nullptr;
  BasicBlock *LoopBodyBB = nullptr;
  Value *NewLinearVar = nullptr;
  Value *LinearStartVar = nullptr;

  // Create empty BBlocks for capturing linear operand's starting value.
  createEmptyPrvInitBB(W, LinearInitBB);
  assert(LinearInitBB && "genLinearCode: Couldn't create LinearInitBB.");
  IRBuilder<> CaptureBuilder(LinearInitBB->getTerminator());

  // Create IRBuilder for copying back local linear vars' final value to the
  // original vars.
  IRBuilder<> FiniBuilder(LinearFiniBB->getTerminator());

  // Get the First BB for the loop body. The initialization of local linear vars
  // per iteration, using the starting value, index and step will go here.
  WRNLoopInfo &WRNLI = W->getWRNLoopInfo();
  Loop *L = WRNLI.getLoop();
  assert(L && "genLinearCode: Null Loop.");
  assert(L->getNumBlocks() > 0 && "genLinearCode: No BBlocks in the loop.");
  auto *LoopBodyBBIter = L->block_begin();
  LoopBodyBB = *LoopBodyBBIter;
  assert(LoopBodyBB && "genLinearCode: Null loop body.");
  Instruction *NewLinearInsertPt = EntryBB->getFirstNonPHI();
  IRBuilder<> InitBuilder(
      OMPLBForLinearClosedForm
          ? GeneralUtils::nextUniqueInstruction(OMPLBForLinearClosedForm)
          : LoopBodyBB->getFirstNonPHI());
  Value *Index = OMPLBForLinearClosedForm
                     ? OMPLBForLinearClosedForm
                     : WRegionUtils::getOmpCanonicalInductionVariable(L);
  assert(Index && "genLinearCode: Null Loop index.");

  for (LinearItem *LinearI : LrClause.items()) {
    Value *Orig = LinearI->getOrig();
    // (A) Create private copy of the linear var to be used instead of the
    // original var inside the WRegion. (2)
    NewLinearVar = genPrivatizationAlloca(LinearI, NewLinearInsertPt,
                                          ".linear"); //                   (2)
    LinearI->setNew(NewLinearVar);

    // Create a copy of the linear variable to capture its starting value (1)
    LinearStartVar =
        genPrivatizationAlloca(LinearI, NewLinearInsertPt); //             (1)
    LinearStartVar->setName("linear.start");

    // Replace original var with the new var inside the region.
    Value *ReplacementVal =
        getClauseItemReplacementValue(LinearI, NewLinearInsertPt);
    genPrivatizationReplacement(W, Orig, ReplacementVal);

    // For by-refs, do a pointer dereference to reach the actual operand.
    if (LinearI->getIsByRef())
      Orig = new LoadInst(Orig, "", NewLinearInsertPt);

    // (B) Capture value of linear variable before entering the loop
    LoadInst *LoadOrig = CaptureBuilder.CreateLoad(Orig);               // (3)
    CaptureBuilder.CreateStore(LoadOrig, LinearStartVar);               // (4)

    // (C) Initialize the linear variable using closed form inside the loop
    // body: %y.linear = %y.linear.start + %omp.iv * %step

    Value *LinearStart = InitBuilder.CreateLoad(LinearStartVar);        // (5)
    Type *LinearTy = LinearStart->getType();

    Value *Step = LinearI->getStep();

    // If the sizes of step/index/linear var don't match, we need to create
    // some type casts when doing the closed-form computation.
    Type *IndexTy = Index->getType();
    Type *StepTy = Step->getType();
    assert(isa<IntegerType>(IndexTy) && "genLinearCode: Index is not an int.");
    assert(isa<IntegerType>(StepTy) && "genLinearCode: Step is not an int.");

    if (IndexTy->getIntegerBitWidth() < StepTy->getIntegerBitWidth())
      Index = InitBuilder.CreateIntCast(Index, StepTy, true);
    else if (IndexTy->getIntegerBitWidth() > StepTy->getIntegerBitWidth())
      Step = InitBuilder.CreateIntCast(Step, IndexTy, true);            // (6)
    auto *Mul = InitBuilder.CreateMul(Index, Step);                     // (7)

    Value *Add = nullptr;
    if (isa<PointerType>(LinearTy))
      Add = InitBuilder.CreateInBoundsGEP(LinearStart, {Mul});
    else {
      Type *MulTy = Mul->getType();

      if (LinearTy->getIntegerBitWidth() < MulTy->getIntegerBitWidth())
        LinearStart = InitBuilder.CreateIntCast(LinearStart, MulTy, true); //(8)
      else if (LinearTy->getIntegerBitWidth() > MulTy->getIntegerBitWidth())
        Mul = InitBuilder.CreateIntCast(Mul, LinearTy, true);

      Add = InitBuilder.CreateAdd(LinearStart, Mul);                    // (9)
      Add = InitBuilder.CreateIntCast(Add, LinearTy, true);             // (10)
    }

    InitBuilder.CreateStore(Add, NewLinearVar);                         // (11)

    // (D) Insert the final linear copy-out from local vars to the original.
    LoadInst *FiniLoad = FiniBuilder.CreateLoad(NewLinearVar);          // (12)
    FiniBuilder.CreateStore(FiniLoad, Orig);                            // (13)

    LLVM_DEBUG(dbgs() << "genLinearCode: generated " << *Orig << "\n");
  }

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genLinearCode\n");

  W->resetBBSet(); // CFG changed; clear BBSet
  return true;
}

bool VPOParoptTransform::genFirstPrivatizationCode(WRegionNode *W) {

  bool Changed = false;

  LLVM_DEBUG(
      dbgs() << "\nEnter VPOParoptTransform::genFirstPrivatizationCode\n");

  assert(W->canHaveFirstprivate() &&
         "genFirstPrivatizationCode: WRN doesn't take a firstprivate var");

  FirstprivateClause &FprivClause = W->getFpriv();
  if (!FprivClause.empty()) {
    auto *RegPredBlock = W->getEntryBBlock();
    W->setEntryBBlock(SplitBlock(RegPredBlock, &RegPredBlock->front(), DT, LI));
    // Force BBSet rebuild due to the entry block change.
    W->populateBBSet(true);
    BasicBlock *EntryBB = W->getEntryBBlock();
    BasicBlock *PrivInitEntryBB = nullptr;
    bool ForTask = W->getIsTask();

    for (FirstprivateItem *FprivI : FprivClause.items()) {

      if (FprivI->getInMap())
        // For some reason clang may put a variable both into map() and
        // firstprivate clause, e.g.:
        // void foo(int n) {
        //   #pragma omp target map(from:n)
        //   #pragma omp teams num_teams(n)
        //   ...
        // }
        //
        // The generated region will look like this:
        // %0 = call token @llvm.directive.region.entry() [
        //          "DIR.OMP.TARGET"(),
        //          "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
        //          "QUAL.OMP.MAP.FROM"(i32* %n.addr),
        //          "QUAL.OMP.FIRSTPRIVATE"(i32* %n.addr) ]
        //
        // We do not need to generate any special code for firstprivate()
        // clause, as long as all the data movement will be handled
        // by the map() processing.
        continue;

      Value *NewPrivInst = nullptr;
      Value *Orig = FprivI->getOrig();

      // assert((isa<GlobalVariable>(Orig) || isa<AllocaInst>(Orig)) &&
      //      "genFirstPrivatizationCode: Unexpected firstprivate variable");
      //
      LastprivateItem *LprivI = FprivI->getInLastprivate();

      if (!LprivI) {
        // Only make new storage if !LprivI.
        // If the item was lastprivate, we should use the lastprivate
        // new version as the item's storage.
        Instruction *InsertPt = EntryBB->getFirstNonPHI();
        if (!ForTask) {
          NewPrivInst = genPrivatizationAlloca(FprivI, InsertPt, ".fpriv");
          FprivI->setNewOnTaskStack(NewPrivInst);
        } else
          NewPrivInst = FprivI->getNew(); // Use the task thunk directly
        Value *ReplacementVal = getClauseItemReplacementValue(FprivI, InsertPt);
        genPrivatizationReplacement(W, Orig, ReplacementVal);
        FprivI->setNew(NewPrivInst);
      } else if (!ForTask) { // && LprivI
        // Lastprivate codegen has replaced the original var with the
        // lastprivate new version.
        // Parallel-for: Set up the firstprivate new version to point to the
        // lastprivate new version. Then the copy codegen below, will
        // copy the original var's value to that location.
        // We don't do this for tasks because the firstprivate new version
        // already has the correct data (it points to the incoming task thunk).
        LLVM_DEBUG(dbgs() << "\n  genFirstPrivatizationCode: (" << *Orig
                          << ") is also lastprivate\n");
        FprivI->setNew(LprivI->getNew());
      }

      if (!ForTask) {
        // Copy the original data to the firstprivate new version.
        // Note: We must run lastprivate codegen before firstprivate codegen.
        // Otherwise the lastprivate var replacement will mess up the
        // copy instructions.

        createEmptyPrvInitBB(W, PrivInitEntryBB);
        if (!NewPrivInst ||
            // Note that NewPrivInst will be nullptr, if the variable
            // is also lastprivate.
            FprivI->getIsByRef() ||
            !NewPrivInst->getType()->isPointerTy() ||
            !NewPrivInst->getType()->getPointerElementType()->isPointerTy())
          // Copy the firstprivate data from the original version to the
          // private copy. In the case of lastprivate, this is the lastprivate
          // copy. The lastprivate codegen will replace all original vars
          // with the lastprivate copy.
          genFprivInit(FprivI, PrivInitEntryBB->getTerminator());
        else {
          // If the firstprivate() item is a pointer to pointer,
          // then we can avoid double dereference and pass the
          // pointee's value directly.  This is not an optimization.
          // We have to do this for targets that return opaque
          // objects to libomptarget vs actual target pointers.
          // Let's consider the following example:
          // double *out = &x;
          // #pragma omp target data map(tofrom:out[0:1])
          // #pragma omp target firstprivate(out)
          //
          // "target data" will map 'out' as PTR_AND_OBJ, and libomptarget
          // will store the target counterpart of 'out' pointer into
          // the shadow location.  "target" code will load
          // the value of 'out' from the shadow location, but this may be
          // an opaque pointer, which does not make sense inside
          // the target code. By passing 'out' to the target region
          // "by value" we guarantee that it is mapped to the target
          // object created by "target data".
          // We also avoid an extra dereference, which is profitable.
          IRBuilder<> PredBuilder(RegPredBlock->getTerminator());
          auto *Load = PredBuilder.CreateLoad(Orig);
          IRBuilder<> RegBuilder(PrivInitEntryBB->getTerminator());
          RegBuilder.CreateStore(Load, FprivI->getNew());

          FprivI->setOrig(Load);
          FprivI->setIsPointer(true);
        }
      }
      else if (LprivI) { // && ForTask
        // For taskloop, the firstprivate new version already contains
        // the correct data (it points to the task thunk). Copy the
        // data to the lastprivate new version, which is the version used
        // inside the taskloop.
        createEmptyPrvInitBB(W, PrivInitEntryBB);
        VPOParoptUtils::genCopyByAddr(LprivI->getNew(), FprivI->getNew(),
                                      PrivInitEntryBB->getTerminator(),
                                      nullptr,
                                      FprivI->getIsByRef());
      }

      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": firstprivatized '";
                 Orig->printAsOperand(dbgs()); dbgs() << "'\n");
    } // for
    Changed = true;
  } // if (!FprivClause.empty())

  LLVM_DEBUG(
      dbgs() << "\nExit VPOParoptTransform::genFirstPrivatizationCode\n");

  W->resetBBSetIfChanged(Changed); // Clear BBSet if transformed
  return Changed;
}

bool VPOParoptTransform::genLastPrivatizationCode(
    WRegionNode *W, BasicBlock *IfLastIterBB, Instruction *OMPLBForChunk,
    Instruction *BranchToNextChunk, Instruction *OMPZtt) {
  LLVM_DEBUG(
      dbgs() << "\nEnter VPOParoptTransform::genLastPrivatizationCode\n");

  if (!W->canHaveLastprivate())
    return false;

  Instruction *BarrierForConditionalLP = genBarrierForConditionalLP(W);

  bool Changed = false;
  LastprivateClause &LprivClause = W->getLpriv();
  if (!LprivClause.empty()) {

    BasicBlock *EntryBB = W->getEntryBBlock();

    if (isa<WRNVecLoopNode>(W))
      // Insert privatization code (e.g. allocas, constructor calls)
      // in a block preceeding the SIMD region's entry block.
      // We should minimize the code between SIMD entry/exit points
      // that is not related to the loop itself, otherwise, vectorizer
      // may complain. Note that the last value copy is still generated
      // inside the SIMD region - if this is a problem, we should run
      // sinkSIMDDirectives() for standalone SIMD regions as well.
      //
      // FIXME: if there is no parent region that will be outlined, then
      //        the allocas will remain inside the function body, and
      //        may not be handled by optimizations (e.g. promote memory
      //        to register pass). In such case we have to insert allocas
      //        inside the function's entry block. Moreover, insertion
      //        of allocas next to the SIMD region is not correct, if
      //        there is an enclosing loop - such an alloca may cause
      //        stack saturation.
      //        This is relevant to WRNWksLoop as well.
      //
      // Note that EntryBB points to the new empty block after the call
      // below.
      W->setEntryBBlock(SplitBlock(EntryBB, &EntryBB->front(), DT, LI));

    W->populateBBSet();

    bool ForTask = W->getWRegionKindID() == WRegionNode::WRNTaskloop ||
                   W->getWRegionKindID() == WRegionNode::WRNTask;
    bool IsVecLoop = isa<WRNVecLoopNode>(W);

    for (LastprivateItem *LprivI : LprivClause.items()) {
      Value *Orig = LprivI->getOrig();
      bool IsConditionalLP = LprivI->getIsConditional();
      assert((!IsConditionalLP ||
              (!LprivI->getConstructor() && !LprivI->getDestructor())) &&
             "Conditional lastprivate is expected only for scalar variables.");
      assert((!IsConditionalLP || !ForTask) &&
             "Conditional lastprivate is not yet supported for taskloops.");
      assert((IfLastIterBB || IsConditionalLP) &&
             "genLastPrivatizationCode: Null BB for last iteration check.");

      Value *NewPrivInst;
      Instruction *InsertPt = &EntryBB->front();
      if (!ForTask)
        NewPrivInst = genPrivatizationAlloca(LprivI, InsertPt, ".lpriv");
      else {
        NewPrivInst = LprivI->getNew();
        InsertPt = cast<Instruction>(NewPrivInst)->getParent()->getTerminator();
      }
      LprivI->setNew(NewPrivInst);

      Value *ReplacementVal = getClauseItemReplacementValue(LprivI, InsertPt);
      genPrivatizationReplacement(W, Orig, ReplacementVal);

      // Emit constructor call for lastprivate var if it is not also a
      // firstprivate (in which case the firstprivate init emits a cctor).
      if (LprivI->getInFirstprivate() == nullptr)
        VPOParoptUtils::genConstructorCall(LprivI->getConstructor(),
                                           NewPrivInst, NewPrivInst);
      // Generate the if-last-then-copy-out code.
      if (IsConditionalLP && !IsVecLoop)
        genConditionalLPCode(W, LprivI, OMPLBForChunk, OMPZtt,
                             BranchToNextChunk, BarrierForConditionalLP);
      else if (!ForTask)
        genLprivFini(LprivI, IfLastIterBB->getTerminator());
      else
        genLprivFiniForTaskLoop(LprivI, IfLastIterBB->getTerminator());

      // For tasks, call the destructor for the internal lastprivate var
      // at the very end (after the thread may have copied-out)
      // If the var is also firstprivate, the runtime will destruct it.
      if (ForTask && LprivI->getDestructor() &&
          LprivI->getInFirstprivate() == nullptr) {
        VPOParoptUtils::genDestructorCall(LprivI->getDestructor(),
                                          LprivI->getNew(),
                                          W->getExitBBlock()->getTerminator());
      }

      if (IsVecLoop && IsConditionalLP) {
        // For the following case:
        //   int x = 0;
        //   #pragma omp simd lastprivate(conditional: x)
        //   for (...)
        //     if (cond)
        //       x = ...;
        //
        // We generate IR equivalent to the following code:
        //   int x = 0;
        //   int x.lpriv = x;
        //   #pragma omp simd lastprivate(conditional: x.lpriv)
        //   for (...)
        //     if (cond)
        //       x.lpriv = ...;
        //   x = x.lpriv;
        //
        // Even though OpenMP specification does not seem to explicitly
        // describe the case, when (cond) is never true, we implement
        // conditional lastprivates such that the original list item ('x')
        // keeps it original (before the construct) value, if (cond)
        // is always false.
        //
        // The code below produces 'x.lpriv = x' assignment just reusing
        // the firstprivatization code.
        FirstprivateItem FprivI(Orig);
        FprivI.setNew(NewPrivInst);
        FprivI.setIsByRef(LprivI->getIsByRef());
        // We do not care about CopyConstructor of the fake firstprivate
        // item, because conditional lastprivate may only reference
        // a scalar variable (potentially, by reference).
        genFprivInit(&FprivI, EntryBB->getTerminator());
      }
    }
    Changed = true;
  } // if (!LprivClause.empty())

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genLastPrivatizationCode\n");

  W->resetBBSetIfChanged(Changed); // Clear BBSet if transformed
  return Changed;
}

// Generate destructor calls for [first|last]private variables
bool VPOParoptTransform::genDestructorCode(WRegionNode *W) {
  if (!WRegionUtils::needsDestructors(W)) {
    LLVM_DEBUG(dbgs() << "\nVPOParoptTransform::genDestructorCode: No dtors\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genDestructorCode\n");

  // Create a BB before ExitBB in which to insert dtor calls
  BasicBlock *NewBB = nullptr;
  createEmptyPrivFiniBB(W, NewBB);
  Instruction* InsertBeforePt = NewBB->getTerminator();

  // Destructors for privates
  if (W->canHavePrivate())
    for (PrivateItem *PI : W->getPriv().items())
      VPOParoptUtils::genDestructorCall(PI->getDestructor(), PI->getNew(),
                                        InsertBeforePt);
  // Destructors for firstprivates
  if (W->canHaveFirstprivate())
    for (FirstprivateItem *FI : W->getFpriv().items())
      VPOParoptUtils::genDestructorCall(FI->getDestructor(), FI->getNew(),
                                        InsertBeforePt);
  // Destructors for lastprivates (that are not also firstprivate)
  if (W->canHaveLastprivate())
    for (LastprivateItem *LI : W->getLpriv().items())
      if (LI->getInFirstprivate() == nullptr)
        VPOParoptUtils::genDestructorCall(LI->getDestructor(), LI->getNew(),
                                          InsertBeforePt);
      // else do nothing; dtor already emitted for Firstprivates above

  /* TODO: emit Dtors for UDR
  if (W->canHaveReduction())
    for (ReductionItem *RI : W->getRed().items())
      VPOParoptUtils::genDestructorCall(RI->getDestructor(), LI->getNew(),
                                        InsertBeforePt);
  */

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genDestructorCode\n");
  W->resetBBSet(); // CFG changed; clear BBSet
  return true;
}

// Create a pointer, store address of V to the pointer, and replace uses of V
// with a load from that pointer.
//
//   %v = alloca i32                          ; V
//   ...
//   %v.addr = alloca i32*                    ; (1)
//   ...
//   store i32* %v, i32** %v.addr             ; (2)
//           ; <InsertPtForStore>
//
//   +- <EntryBB>:
//   | ...
//   | %0 = llvm.region.entry() [... "PRIVATE" (i32* %v) ]
//   | ...
//   | %v1 = load i32*, i32** %v.addr          ; (3)
//   +-
//   ... Replace uses of %v with %v1
Value *VPOParoptTransform::replaceWithStoreThenLoad(
    WRegionNode *W, Value *V, Instruction *InsertPtForStore,
    bool InsertLoadInBeginningOfEntryBB) {

  // Find instructions in W that use V
  SmallVector<Instruction *, 8> Users;
  WRegionUtils::findUsersInRegion(W, V, &Users,
                                  !InsertLoadInBeginningOfEntryBB);

  Instruction *AllocaInsertPt =
      W->getEntryBBlock()->getParent()->getEntryBlock().getTerminator();
  IRBuilder<> AllocaBuilder(AllocaInsertPt);
  AllocaInst *VAddr = //                                  (1)
      AllocaBuilder.CreateAlloca(V->getType(), nullptr, V->getName() + ".addr");

  IRBuilder<> StoreBuilder(InsertPtForStore);
  StoreBuilder.CreateStore(V, VAddr); //                  (2)

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Stored '"; V->printAsOperand(dbgs());
             dbgs() << "' to '"; VAddr->printAsOperand(dbgs());
             dbgs() << "'.\n";);

  if (Users.empty())
    return VAddr; // Nothing to replace inside the region. Just capture the
                  // address of V to VAddr and return it.

  BasicBlock *EntryBB = W->getEntryBBlock();
  Instruction *InsertPtForLoad = InsertLoadInBeginningOfEntryBB
                                     ? EntryBB->getFirstNonPHI()
                                     : EntryBB->getTerminator();
  IRBuilder<> BuilderInner(InsertPtForLoad);
  LoadInst *VRenamed = BuilderInner.CreateLoad(VAddr); // (3)
  VRenamed->setName(V->getName());

  // Replace uses of V with VRenamed
  for (Instruction * User : Users) {

    User->replaceUsesOfWith(V, VRenamed);

    // Some uses of V are in a ConstantExpr, in which case the User is the
    // instruction using the ConstantExpr. For example, the use of @u below is
    // the GEP expression (a ConstantExpr), not the instruction itself, so
    // doing User->replaceUsesOfWith(V, NewI) does not replace @u
    //
    //     %12 = load i32, i32* getelementptr inbounds (%struct.t_union_,
    //           %struct.t_union_* @u, i32 0, i32 0), align 4
    //
    // The solution is to access the ConstantExpr as instruction(s) in order to
    // do the replacement. NewInstArr below keeps such instruction(s).
    SmallVector<Instruction *, 2> NewInstArr;
    GeneralUtils::breakExpressions(User, &NewInstArr);
    for (Instruction *NewInstr : NewInstArr) {
      NewInstr->replaceUsesOfWith(V, VRenamed);
    }
  }
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Loaded '";
             VAddr->printAsOperand(dbgs()); dbgs() << "' into '";
             VRenamed->printAsOperand(dbgs()); dbgs() << "'.\n";);
  return VAddr;
}

// Return true if the instuction is a call to
// @llvm.launder.invariant.group
CallInst*  VPOParoptTransform::isFenceCall(Instruction *I) {
  if (CallInst *CI = dyn_cast<CallInst>(I))
    if (CI->getCalledFunction() && CI->getCalledFunction()->getIntrinsicID() ==
                                       Intrinsic::launder_invariant_group)
      return CI;
  return nullptr;
}

// Transform the loop branch predicate from sle/ule to sgt/ugt in order
// to faciliate the scev based loop trip count calculation.
//
//         do {
//             %omp.iv = phi(%omp.lb, %omp.inc)
//             ...
//             %omp.inc = %omp.iv + 1;
//          }while (%omp.inc <= %omp.ub)
//          ===>
//         do {
//             %omp.iv = phi(%omp.lb, %omp.inc)
//             ...
//             %omp.inc = %omp.iv + 1;
//          }while (%omp.ub + 1 > %omp.inc)
void VPOParoptTransform::fixOmpBottomTestExpr(Loop *L) {
  BasicBlock *Backedge = L->getLoopLatch();
  Instruction *TermInst = Backedge->getTerminator();
  BranchInst *ExitBrInst = cast<BranchInst>(TermInst);
  ICmpInst *CondInst = cast<ICmpInst>(ExitBrInst->getCondition());
  assert((CondInst->getPredicate() == CmpInst::ICMP_SLE ||
         CondInst->getPredicate() == CmpInst::ICMP_ULE) &&
         "Expect incoming loop predicate is SLE or ULE");
  ICmpInst::Predicate NewPred = CondInst->getInversePredicate();
  CondInst->swapOperands();
  Value *Ub = CondInst->getOperand(0);
  IntegerType *UpperBoundTy = cast<IntegerType>(Ub->getType());
  ConstantInt *ValueOne  = ConstantInt::get(UpperBoundTy, 1);
  IRBuilder<> Builder(CondInst);
  Value* NewUb = Builder.CreateAdd(Ub, ValueOne);
  CondInst->replaceUsesOfWith(Ub, NewUb);
  CondInst->setPredicate(NewPred);
}

// Transform the given do-while loop loop into the canonical form as follows.
//         do {
//             %omp.iv = phi(%omp.lb, %omp.inc)
//             ...
//             %omp.inc = %omp.iv + 1;
//          }while (%omp.inc <= %omp.ub)
//
void VPOParoptTransform::fixOMPDoWhileLoop(WRegionNode *W, Loop *L) {

  if (WRegionUtils::isDoWhileLoop(L)) {
    fixOmpDoWhileLoopImpl(L);
    if (W->getWRegionKindID() == WRegionNode::WRNVecLoop)
      fixOmpBottomTestExpr(L);
  } else if (WRegionUtils::isWhileLoop(L))
    llvm_unreachable(
        "fixOMPLoop: Unexpected OMP while loop after the loop is rotated.");
  else
    llvm_unreachable("fixOMPLoop: Unexpected OMP loop type");
}

// Utility to transform the given do-while loop loop into the
// canonical do-while loop.
void VPOParoptTransform::fixOmpDoWhileLoopImpl(Loop *L) {

  BasicBlock *H = L->getHeader();
  BasicBlock *Backedge = L->getLoopLatch();

  for (BasicBlock::iterator I = H->begin(); isa<PHINode>(I); ++I) {
    PHINode *PN = cast<PHINode>(I);
    if (Instruction *Inc =
            dyn_cast<Instruction>(PN->getIncomingValueForBlock(Backedge))) {
      if (Inc->getOpcode() == Instruction::Add &&
          (Inc->getOperand(1) ==
               ConstantInt::get(Type::getInt32Ty(F->getContext()), 1) ||
           Inc->getOperand(1) ==
               ConstantInt::get(Type::getInt64Ty(F->getContext()), 1))) {
        Instruction *TermInst = Inc->getParent()->getTerminator();
        BranchInst *ExitBrInst = dyn_cast<BranchInst>(TermInst);
        if (!ExitBrInst)
          continue;
        ICmpInst *CondInst = dyn_cast<ICmpInst>(ExitBrInst->getCondition());
        if (!CondInst)
          continue;
        ICmpInst::Predicate Pred = CondInst->getPredicate();
        if (Pred == CmpInst::ICMP_SLE || Pred == CmpInst::ICMP_ULE) {
          Value *Operand = CondInst->getOperand(0);
          if (isa<SExtInst>(Operand) || isa<ZExtInst>(Operand))
            Operand = cast<CastInst>(Operand)->getOperand(0);

          if (Operand == Inc)
            return;
          else
            llvm_unreachable("cannot fix omp do-while loop");
        } else if (Pred == CmpInst::ICMP_SGT || Pred == CmpInst::ICMP_UGT) {
          Value *Operand = CondInst->getOperand(0);
          if (isa<SExtInst>(Operand) || isa<ZExtInst>(Operand))
            Operand = cast<CastInst>(Operand)->getOperand(0);

          if (Operand == Inc) {
            if (Pred == CmpInst::ICMP_SGT)
              CondInst->setPredicate(CmpInst::ICMP_SLE);
            else
              CondInst->setPredicate(CmpInst::ICMP_ULE);
            ExitBrInst->swapSuccessors();
            return;
          } else
            llvm_unreachable("cannot fix omp do-while loop");
        } else if (Pred == CmpInst::ICMP_SLT || Pred == CmpInst::ICMP_ULT) {
          Value *Operand = CondInst->getOperand(1);
          if (isa<SExtInst>(Operand) || isa<ZExtInst>(Operand))
            Operand = cast<CastInst>(Operand)->getOperand(0);

          if (Operand == Inc) {
            if (Pred == CmpInst::ICMP_SLT)
              CondInst->setPredicate(CmpInst::ICMP_SLE);
            else
              CondInst->setPredicate(CmpInst::ICMP_ULE);
            CondInst->swapOperands();
            ExitBrInst->swapSuccessors();
            return;
          }
        }
      }
    }
  }
}

Value *VPOParoptTransform::genRegionPrivateValue(
    WRegionNode *W, Value *V, bool IsFirstPrivate) {
  // V is either AllocaInst or an AddrSpaceCastInst of AllocaInst.
  assert(GeneralUtils::isOMPItemLocalVAR(V) &&
         "genRegionPrivateValue: Expect non-empty optionally casted "
         "alloca instruction.");

  // Create a fake firstprivate item referencing the original alloca.
  FirstprivateItem FprivI(V);
  auto *EntryBB = W->getEntryBBlock();
  auto *InsertPt = EntryBB->getFirstNonPHI();

  // Generate new alloca in the region's entry block.
  // Note that the alloca will be inserted right before the region
  // entry directive.  We tried to fix this in genPrivatizationCode(),
  // but did not succeed.  In the current use cases the generated
  // alloca will be removed by PromoteMemToReg, so it is currently
  // not a problem.
  // FIXME: we have to set ForceDefaultAddressSpace to true,
  //        because otherwise PromoteMemToReg will not be able
  //        to promote the new AllocaInst. This should be fixed
  //        in PromoteMemToReg.
  auto *NewAlloca = genPrivatizationAlloca(&FprivI, InsertPt, ".local", true);
  FprivI.setNew(NewAlloca);

  // Replace all uses of the original alloca's result with the new alloca
  // definition.
  Value *ReplacementVal = getClauseItemReplacementValue(&FprivI, InsertPt);
  genPrivatizationReplacement(W, V, ReplacementVal);

  if (IsFirstPrivate) {
    // Copy value from the original "variable" to the new one.
    // We have to create an empty block after the region entry
    // directive to simplify the insertion.
    BasicBlock *PrivInitEntryBB;
    createEmptyPrvInitBB(W, PrivInitEntryBB);
    genFprivInit(&FprivI, PrivInitEntryBB->getTerminator());
  }

  return NewAlloca;
}

bool VPOParoptTransform::sinkSIMDDirectives(WRegionNode *W) {
  // Check if there is a child SIMD region associated with
  // this region's loop.
  WRNVecLoopNode *SimdNode = WRegionUtils::getEnclosedSimdForSameLoop(W);

  if (!SimdNode)
    return false;

  // The lambda looks for OpenMP directive call inside the given
  // block. If found, it returns the call instruction,
  // nullptr - otherwise.
  auto FindDirectiveCall = [](BasicBlock *BB) -> Instruction * {
    auto DirI =
        std::find_if(BB->begin(), BB->end(),
                     [](Instruction &I) {
                       return VPOAnalysisUtils::isRegionDirective(&I);
                     });

    return (DirI != BB->end()) ? &*DirI : nullptr;
  };

  // Look for a directive call in the SIMD region's entry block.
  auto *EntryBB = SimdNode->getEntryBBlock();
  auto *EntryDir = FindDirectiveCall(EntryBB);

  // Look for a directive call in the SIMD region's exit block.
  auto *ExitBB = SimdNode->getExitBBlock();
  auto *ExitDir = FindDirectiveCall(ExitBB);

  if (!EntryDir && !ExitDir)
    // The SIMD region must have been removed.
    return false;

  // If the SIMD region was not removed, we must be able
  // to find both region's entry and exit directives.
  assert(EntryDir && ExitDir && "Malformed SIMD region.");

  // Find the loop's exit block, and find or create the loop's
  // pre-header block.
  bool Changed = false;
  auto *L = SimdNode->getWRNLoopInfo().getLoop();
  assert(L == W->getWRNLoopInfo().getLoop() &&
         "Mismatched loops for region and its SIMD child.");

  auto *LoopExitBB = WRegionUtils::getOmpExitBlock(L);
  assert(LoopExitBB && "SIMD loop with no exit block.");

  auto *PreheaderBB = L->getLoopPreheader();
  if (!PreheaderBB) {
    // Insert a pre-header.
    // FIXME: pass false for PreserveLCSSA for the time being.
    //        Pass the actual value, when it is clear, how
    //        to compute it with the new pass manager.
    PreheaderBB = InsertPreheaderForLoop(L, DT, LI, nullptr, false);
    Changed = true;
  }

  assert(PreheaderBB && "Pre-header insertion failed for SIMD loop.");

  if (PreheaderBB != EntryBB) {
    // Move the directive calls from the SIMD region's entry block
    // to the loop's pre-header block.
#ifndef NDEBUG
    // Before moving directives into the pre-header, verify that
    // the pre-header block does not contain other directives.
    // If it does, moving SIMD directives into this block will
    // break the assumption that a block may contain only
    // one set of directives.
    assert(!FindDirectiveCall(PreheaderBB) &&
           "Pre-header block already contains directives.");
#endif  // NDEBUG
    PreheaderBB->getInstList().splice(
        PreheaderBB->getTerminator()->getIterator(), EntryBB->getInstList(),
        EntryDir->getIterator(), ++(EntryDir->getIterator()));
    Changed = true;
  }

  if (LoopExitBB != ExitBB) {
    // Move the directive calls from the SIMD region's exit block
    // to the loop's exit block.
#ifndef NDEBUG
    assert(!FindDirectiveCall(LoopExitBB) &&
           "Loop exit block already contains directives.");
#endif  // NDEBUG
    LoopExitBB->getInstList().splice(LoopExitBB->begin(), ExitBB->getInstList(),
                                     ExitDir->getIterator(),
                                     ++(ExitDir->getIterator()));
    Changed = true;
  }

  W->resetBBSetIfChanged(Changed); // Clear BBSet if transformed
  return Changed;
}

// Transform the Ith level of the loop in the region W into the
// OMP canonical loop form.
void VPOParoptTransform::regularizeOMPLoopImpl(WRegionNode *W, unsigned Index) {
  Loop *L = W->getWRNLoopInfo().getLoop(Index);
  const DataLayout &DL = L->getHeader()->getModule()->getDataLayout();
  const SimplifyQuery SQ = {DL, TLI, DT, AC};
  LoopRotation(L, LI, TTI, AC, DT, SE, nullptr, SQ, true, unsigned(-1), true);
  std::vector<AllocaInst *> Allocas;
  SmallVector<std::pair<Value *, bool>, 3> LoopEssentialValues;
  if (Index < W->getWRNLoopInfo().getNormIVSize()) {
    auto *OrigAlloca = W->getWRNLoopInfo().getNormIV(Index);
    // OrigAlloca may actually be an AddrSpaceCastInst, which
    // operand is the real AllocaInst. PromoteMemToReg will not work
    // in this case, so we have to privatize the normalized induction
    // variable in the region. This is not a big deal, since
    // normalized induction variable *is* private for the region.
    auto *NewAlloca = genRegionPrivateValue(W, OrigAlloca);
    // We apply PromoteMemToReg to the private version generated
    // by genRegionPrivateValue(). We cannot apply PromoteMemToReg
    // to the original normalized induction variable.
    LoopEssentialValues.push_back(std::make_pair(NewAlloca, true));
    LoopEssentialValues.push_back(std::make_pair(OrigAlloca, false));
  }

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  Value *NormUB = nullptr;
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION

  if (Index < W->getWRNLoopInfo().getNormUBSize()) {
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
    if (isTargetCSA()) {
      NormUB = W->getWRNLoopInfo().getNormUB(Index);
      LoopEssentialValues.push_back(std::make_pair(NormUB, true));
    } else {
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
    // Create a local copy of the normalized upper bound.
    //
    // Promoting the normalized upper bound to a register may cause
    // new live-in values, which are not present in the region's
    // clauses.  At the same time, we do want to use registerized
    // upper bound in the loop so that we can canonicalize the loop.
    // We resolve this by introducing a local copy of the normalized
    // upper bound, use this local copy inside the loop and promote
    // it to a register.
    //
    // FIXME: this is a potential performance problem, because
    //        a constant normalized upper bound is not propagated
    //        into the region any more.  We need to find a way
    //        to propagate constant normalized upper bounds and
    //        do not propagate non-constant ones.
    auto *OrigAlloca = W->getWRNLoopInfo().getNormUB(Index);
    auto *NewAlloca = genRegionPrivateValue(W, OrigAlloca, true);
    // The original load/stores from/to the normalized upper bound
    // "variable" may be marked volatile, e.g.:
    //
    // entry:
    //     %orig.omp.ub = alloca i64
    //     %0 = load volatile i64, i64* %orig.omp.ub
    //     br label %region.entry
    //
    // region.entry:
    //     %1 = call token @llvm.directive.region.entry()
    //              [ "QUAL.OMP.NORMALIZED.UB"(i64* %orig.omp.ub) ]
    //     %2 = load volatile i64, i64* %orig.omp.ub
    //
    // genRegionPrivateValue() inserts a new alloca %new.omp.ub and replaces
    // all uses of %orig.omp.ub inside the region with %new.omp.ub.
    // It also generates code to copy data from *(%orig.omp.ub)
    // to *(%new.omp.ub):
    //
    // entry:
    //     %orig.omp.ub = alloca i64
    //     %0 = load volatile i64, i64* %orig.omp.ub
    //     br label %region.entry
    //
    // region.entry:
    //     %new.omp.ub = alloca i64
    //     %1 = call token @llvm.directive.region.entry()
    //              [ "QUAL.OMP.NORMALIZED.UB"(i64* %new.omp.ub) ]
    //     %3 = load i64, i64* %orig.omp.ub
    //     store i64 %3, i64* %new.omp.ub
    //     %2 = load volatile i64, i64* %new.omp.ub
    //
    // Note that %0 load in the entry block still uses %orig.omp.ub,
    // and it is volatile.  Also note that volatile %3 load is now using
    // %new.omp.ub.
    //
    // Now, we want to promote %new.omp.ub to a register.
    // Before we can do this, we need to clear its use inside
    // the QUAL.OMP.NORMALIZED.UB clause, otherwise, the promotion
    // will fail.
    //
    // Code below clears volatile from accesses via all pointers
    // in LoopEssentialValues vector.  The promotion to registers
    // is only run for %new.omp.ub and normalized-iv (added above)
    // elements of the vector (the corresponding boolean is set to
    // true for them).  Allocas that need promotion are also
    // cleared from the clauses before the promotion.
    //
    // The final code looks like this:
    //
    // entry:
    //     %orig.omp.ub = alloca i64
    //     %0 = load i64, i64* %orig.omp.ub
    //     br label %region.entry
    //
    // region.entry:
    //     %1 = call token @llvm.directive.region.entry()
    //              [ "QUAL.OMP.NORMALIZED.UB"(i64* null) ]
    //     %3 = load i64, i64* %orig.omp.ub
    //     %2 = %3
    //
    LoopEssentialValues.push_back(std::make_pair(NewAlloca, true));
    // Do not promote the original alloca to register.
    LoopEssentialValues.push_back(std::make_pair(OrigAlloca, false));
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
    }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
  }

  for (auto &P : LoopEssentialValues) {
    auto *V = P.first;
    bool PromoteToReg = P.second;

    // V is either AllocaInst or an AddrSpaceCastInst of AllocaInst.
    assert(GeneralUtils::isOMPItemLocalVAR(V) &&
           "Expect optionally casted alloca instruction for omp_iv or omp_ub.");

    for (auto IB = V->user_begin(), IE = V->user_end(); IB != IE; ++IB) {
      if (LoadInst *LdInst = dyn_cast<LoadInst>(*IB))
        LdInst->setVolatile(false);
      if (StoreInst *StInst = dyn_cast<StoreInst>(*IB))
        StInst->setVolatile(false);
    }
    if (PromoteToReg) {
      resetValueInOmpClauseGeneric(W, V);
      // Only AllocaInst can be here.
      auto *AI = dyn_cast<AllocaInst>(V);
      assert(AI && "Trying mem-to-reg for not an AllocaInst.");
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
      if (AI == NormUB && !isAllocaPromotable(AI))
        continue;
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
      Allocas.push_back(AI);
    }
  }

  PromoteMemToReg(Allocas, *DT, AC);
  fixOMPDoWhileLoop(W, L);

  BasicBlock *ZTTBB = nullptr;
  if (auto *CmpI = WRegionUtils::getOmpLoopZeroTripTest(L, W->getEntryBBlock()))
    ZTTBB = CmpI->getParent();

  // If the loop does not have zero-trip test, then explicitly
  // set nullptr as ZTT basic block. getZTTBB() asserts that a Loop
  // has its ZTT basic block set even if it is nullptr. The assertion
  // is there to make sure that we do not miss ZTT block setup
  // for a Loop.
  W->getWRNLoopInfo().setZTTBB(ZTTBB, Index);
}

// The OMP loop is converted into bottom test loop to facilitate the
// code generation of VPOParopt transform and vectorization. This
// regularization is required for the program which is compiled at -O0
// and above.
bool VPOParoptTransform::regularizeOMPLoop(WRegionNode *W, bool First) {
  if (!W->getWRNLoopInfo().getLoop())
    return false;

  // For the case of #pragma omp parallel for simd, clang only
  // generates the bundle omp.iv for the parallel for region.
  if (W->getWRNLoopInfo().getNormIVSize() == 0)
    return false;

  W->populateBBSet();
  if (!First)
    for (unsigned I = W->getWRNLoopInfo().getNormIVSize(); I > 0; --I)
      regularizeOMPLoopImpl(W, I - 1);
  else {
    std::vector<AllocaInst *> Allocas;
    SmallVector<Value *, 2> LoopEssentialValues;
    if (W->getWRNLoopInfo().getNormIV())
      for (unsigned I = 0; I < W->getWRNLoopInfo().getNormIVSize(); ++I)
        LoopEssentialValues.push_back(W->getWRNLoopInfo().getNormIV(I));

    if (W->getWRNLoopInfo().getNormUB())
      for (unsigned I = 0; I < W->getWRNLoopInfo().getNormUBSize(); ++I)
        LoopEssentialValues.push_back(W->getWRNLoopInfo().getNormUB(I));

    for (auto V : LoopEssentialValues) {
      assert(GeneralUtils::isOMPItemLocalVAR(V) &&
             "Expect optionally casted alloca instruction "
             "for omp_iv or omp_ub.");
      for (auto IB = V->user_begin(), IE = V->user_end(); IB != IE; ++IB) {
        if (LoadInst *LdInst = dyn_cast<LoadInst>(*IB))
          LdInst->setVolatile(true);
        if (StoreInst *StInst = dyn_cast<StoreInst>(*IB))
          StInst->setVolatile(true);
      }
    }
  }
  W->resetBBSet(); // CFG changed; clear BBSet
  return true;
}

//       Before                       |     After
//  ----------------------------------+------------------------------------
//  %size.val = load i32, i32* %size (1)    %size.val = load i32, i32* %size
//  %vla = alloca i32, i32 %size.val  |     %vla = alloca i32, i32 %size.val
//  ...                               |     ...
//                                    |
//  <OldEntryBB>:                     |  <OldEntryBB>:
//                                    |
//                                   (2)    %size2 = alloca i32
//                                   (3)    store i32 %size.val, i32* size2
//                                    |
//                                    |   <NewEntryBB>:
//                                   (4)    %size.val2 = load i32, i32* size2
//  ...                               |     ...
//  %vla.private = alloca i32,        |     %vla.private = alloca i32,
//                 i32 %size.val     (5)                   i32 %size.val2
//                                    |
//                        <entry directive for W>
//  (..."DIR.OMP.PARALLEL"...)        |     (..."DIR.OMP.PARALLEL"...
//                                   (6) ;   "QUAL.OMP.SHARED" (i32* size2))
//                                    |
//                                    |  ; Note: `%size2` is added as shared to
//                                    |  ; W, but the directive is not modified
//
bool VPOParoptTransform::captureAndAddCollectedNonPointerValuesToSharedClause(
    WRegionNode *W) {

  if (!W->needsOutlining())
    return false;

  if (!isa<WRNParallelNode>(W) && !isa<WRNParallelLoopNode>(W) &&
      !isa<WRNParallelSectionsNode>(W) && !isa<WRNTargetNode>(W))
    // TODO: Remove this to enable the function for all outlined WRNs.
    return false;

  const auto &DirectlyUsedNonPointerVals = W->getDirectlyUsedNonPointerValues();
  if (DirectlyUsedNonPointerVals.empty())
    return false;

  bool Changed = false;

  // Insert an empty BBlock before EntryBB of W to insert the capturing code.
  BasicBlock *OldEntryBB = W->getEntryBBlock();
  BasicBlock *NewEntryBB =
      SplitBlock(OldEntryBB, OldEntryBB->getFirstNonPHI(), DT, LI);
  Instruction *InsertStoreBefore = OldEntryBB->getTerminator();

  W->setEntryBBlock(NewEntryBB);
  W->populateBBSet(true); // rebuild BBSet unconditionlly as EntryBB changed

  for (Value *ValToCapture : DirectlyUsedNonPointerVals) { //           (1)
    // Make the changes (2), (3), (4), (5)
    Value *CapturedValAddr = //                                         (2)
        replaceWithStoreThenLoad(W, ValToCapture, InsertStoreBefore,
                                 true); // Insert (4) in beginning of NewEntryBB
    if (!CapturedValAddr)
      continue;

    // We mark the pointer CapturedValAddr as 'shared' for non-target
    // constructs, and 'map(to)' for targets.
    if (!isa<WRNTargetNode>(W)) {
      SharedClause &ShrClause = W->getShared();
      WRegionUtils::addToClause(ShrClause, CapturedValAddr); //         (6)
    } else {
      MapClause &MpClause = W->getMap();
      WRegionUtils::addToClause(MpClause, CapturedValAddr);
      MpClause.back()->setIsMapTo();
    }
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": Added implicit shared/map(to) clause for: '";
               CapturedValAddr->printAsOperand(dbgs()); dbgs() << "'\n");
    Changed = true;
  }

  W->resetBBSetIfChanged(Changed); // Clear BBSet if transformed
  return Changed;
}

// Rename operands of various clauses by replacing them with a
// store-then-load.
bool VPOParoptTransform::renameOperandsUsingStoreThenLoad(WRegionNode *W) {
  bool Changed = false;
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *NewEntryBB =
      SplitBlock(EntryBB, EntryBB->getFirstNonPHI(), DT, LI);
  Instruction *InsertBefore = EntryBB->getTerminator();

  W->setEntryBBlock(NewEntryBB);
  W->populateBBSet(true); // rebuild BBSet unconditionlly as EntryBB changed

  SmallPtrSet<Value *, 16> HandledVals;
  using OpndAddrPairTy = SmallVector<Value *, 2>;
  SmallVector<OpndAddrPairTy, 16> OpndAddrPairs;
  auto rename = [&](Value *Orig, bool CheckAlreadyHandled) {
    if (CheckAlreadyHandled && HandledVals.find(Orig) != HandledVals.end())
      return false;

    HandledVals.insert(Orig);
    Value *RenamedAddr = replaceWithStoreThenLoad(W, Orig, InsertBefore);
    if (!RenamedAddr)
      return false;

    OpndAddrPairs.push_back({Orig, RenamedAddr});
    return true;
  };

  if (W->canHavePrivate()) {
    PrivateClause &PrivClause = W->getPriv();
    for (PrivateItem *PrivI : PrivClause.items())
      Changed |= rename(PrivI->getOrig(), false);
  }

  if (W->canHaveFirstprivate()) {
    FirstprivateClause &FprivClause = W->getFpriv();
    for (FirstprivateItem *FprivI : FprivClause.items())
      Changed |= rename(FprivI->getOrig(), false);
  }

  if (W->canHaveShared()) {
    SharedClause &ShaClause = W->getShared();
    for (SharedItem *ShaI : ShaClause.items())
      Changed |= rename(ShaI->getOrig(), false);
  }

  if (W->canHaveReduction()) {
    ReductionClause &RedClause = W->getRed();
    for (ReductionItem *RedI : RedClause.items())
      Changed |= rename(RedI->getOrig(), false);
  }

  if (W->canHaveLastprivate()) {
    LastprivateClause &LprivClause = W->getLpriv();
    for (LastprivateItem *LprivI : LprivClause.items())
      Changed |= rename(LprivI->getOrig(), true);
  }

  if (W->canHaveLinear()) {
    LinearClause &LrClause = W->getLinear();
    for (LinearItem *LrI : LrClause.items())
      Changed |= rename(LrI->getOrig(), true);
  }

  if (W->canHaveMap()) {
    MapClause const &MpClause = W->getMap();
    for (MapItem *MapI : MpClause.items()) {
      if (MapI->getIsMapChain()) {
        MapChainTy const &MapChain = MapI->getMapChain();
        for (unsigned I = 0; I < MapChain.size(); ++I) {
          MapAggrTy *Aggr = MapChain[I];
          Changed |= rename(Aggr->getSectionPtr(), true);
          Changed |= rename(Aggr->getBasePtr(), true);
        }
      }
      Changed |= rename(MapI->getOrig(), true);
    }
  }

  W->resetBBSetIfChanged(Changed); // Clear BBSet if transformed

  if (!Changed)
    return false;

  assert(!OpndAddrPairs.empty() && "Something changed without any renaming.");
  CallInst *CI = cast<CallInst>(W->getEntryDirective());
  SmallVector<std::pair<StringRef, ArrayRef<Value *>>, 8> BundleOpndAddrs;
  StringRef OperandAddrClauseString =
      VPOAnalysisUtils::getClauseString(QUAL_OMP_OPERAND_ADDR);
  for (auto &OpndAddrPair : OpndAddrPairs)
    BundleOpndAddrs.emplace_back(OperandAddrClauseString,
                                 makeArrayRef(OpndAddrPair));

  CI = VPOParoptUtils::addOperandBundlesInCall(CI, BundleOpndAddrs);
  W->setEntryDirective(CI);

  return true;
}

bool VPOParoptTransform::genPrivatizationCode(WRegionNode *W) {

  bool Changed = false;

  BasicBlock *EntryBB = W->getEntryBBlock();

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genPrivatizationCode\n");

  // Process all PrivateItems in the private clause
  PrivateClause &PrivClause = W->getPriv();
  if (!PrivClause.empty()) {

    if (isa<WRNVecLoopNode>(W))
      // Insert privatization code (e.g. allocas, constructor calls)
      // in a block preceeding the SIMD region's entry block.
      // We should minimize the code between SIMD entry/exit points
      // that is not related to the loop itself, otherwise, vectorizer
      // may complain. Note that the last value copy is still generated
      // inside the SIMD region - if this is a problem, we should run
      // sinkSIMDDirectives() for standalone SIMD regions as well.
      //
      // FIXME: if there is no parent region that will be outlined, then
      //        the allocas will remain inside the function body, and
      //        may not be handled by optimizations (e.g. promote memory
      //        to register pass). In such case we have to insert allocas
      //        inside the function's entry block. Moreover, insertion
      //        of allocas next to the SIMD region is not correct, if
      //        there is an enclosing loop - such an alloca may cause
      //        stack saturation.
      //        This is relevant to WRNWksLoop as well.
      //
      // Note that EntryBB points to the new empty block after the call
      // below.
      W->setEntryBBlock(SplitBlock(EntryBB, &EntryBB->front(), DT, LI));

    W->populateBBSet();

    bool ForTask = W->getWRegionKindID() == WRegionNode::WRNTaskloop ||
                   W->getWRegionKindID() == WRegionNode::WRNTask;
    BasicBlock *DestrBlock = nullptr;
    // Walk through each PrivateItem list in the private clause to perform
    // privatization for each Value item
    for (PrivateItem *PrivI : PrivClause.items()) {
      Value *Orig = PrivI->getOrig();

      if (isa<GlobalVariable>(Orig) ||
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
          // On CSA we also need to do privatization for function arguments
          // because parallel region are not getting outlined. Thus we have to
          // create private instances for function arguments which are
          // annotated as private to avoid modification of the original
          // argument.
          (isa<Argument>(Orig) && isTargetCSA()) ||
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          GeneralUtils::isOMPItemLocalVAR(Orig) ||
          isa<GetElementPtrInst>(Orig) || isa<BitCastInst>(Orig) ||
          (isa<CallInst>(Orig) && isFenceCall(cast<CallInst>(Orig))) ||
          (isa<LoadInst>(Orig) && isa<PointerType>(Orig->getType()))) {
        Value *NewPrivInst;

        // Insert alloca for privatization right after the BEGIN directive.
        // Note: do not hoist the following AllocaInsertPt computation out of
        // this for-loop. AllocaInsertPt may be a clause directive that is
        // removed by genPrivatizationReplacement(), so we need to recompute
        // AllocaInsertPt at every iteration of this for-loop.

#if INTEL_CUSTOMIZATION
        // For now, back out this change to AllocaInsertPt until we figure
        // out why it causes an assert in VPOCodeGen::getVectorPrivateBase
        // when running run_gf_channels (gridfusion4.3_tuned_channels).
        //
        //   Instruction *AllocaInsertPt = EntryBB->front().getNextNode();

        // TODO: Restructure this code so that the alloca is only done for
        // non-tasks. We could just use the thunk's private space pointer
        // directly in the task body.
#endif // INTEL_CUSTOMIZATION
        Instruction *AllocaInsertPt = EntryBB->getFirstNonPHI();
        NewPrivInst = genPrivatizationAlloca(PrivI, AllocaInsertPt, ".priv");
        PrivI->setNew(NewPrivInst);
        Value *ReplacementVal =
            getClauseItemReplacementValue(PrivI, AllocaInsertPt);
        genPrivatizationReplacement(W, Orig, ReplacementVal);

        // checks for constructor existence
        VPOParoptUtils::genConstructorCall(PrivI->getConstructor(), NewPrivInst,
                                           NewPrivInst);
        if (ForTask && PrivI->getDestructor()) {
          // For tasks, call the destructor at the end of the region.
          // For non-tasks, genDestructorCode takes care of this.
          if (!DestrBlock)
            createEmptyPrivFiniBB(W, DestrBlock);
          VPOParoptUtils::genDestructorCall(PrivI->getDestructor(),
                                            PrivI->getNew(),
                                            DestrBlock->getTerminator());
        }

        LLVM_DEBUG(dbgs() << __FUNCTION__ << ": privatized '";
                   Orig->printAsOperand(dbgs()); dbgs() << "'\n");
      } else
        LLVM_DEBUG(dbgs() << __FUNCTION__ << ": '";
                   Orig->printAsOperand(dbgs());
                   dbgs() << "' is already private.\n");
    } // for

    Changed = true;

    // After Privatization is done, the SCEV should be re-generated.
    // This should apply to all loop-type constructs; ie, WRNs whose
    // "IsOmpLoop" attribute is true.
    if (SE && W->getIsOmpLoop()) {
        Loop *L = W->getWRNLoopInfo().getLoop();
        SE->forgetLoop(L);
    }
  } // if (!PrivClause.empty())

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genPrivatizationCode\n");
  W->resetBBSetIfChanged(Changed); // Clear BBSet if transformed
  return Changed;
}

// Replace the live-in value of the phis at the loop header with
// the loop carried value.
void VPOParoptTransform::wrnUpdateSSAPreprocessForOuterLoop(
    Loop *L,
    DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
    SmallSetVector<Instruction *, 8> &LiveOutVals,
    EquivalenceClasses<Value *> &ECs) {
  BasicBlock *BB = L->getHeader();

  for (Instruction &I : *BB) {
    if (!isa<PHINode>(I))
      break;
    PHINode *PN = dyn_cast<PHINode>(&I);
    unsigned NumPHIValues = PN->getNumIncomingValues();
    unsigned II;
    Value *V = nullptr, *OV = nullptr;
    bool Match = false;
    for (II = 0; II < NumPHIValues; II++) {
      V = PN->getIncomingValue(II);
      if (!ValueToLiveinMap.count(V))
        continue;
      Instruction *UR = dyn_cast<Instruction>(V);
      if (UR) {
        Match = false;
        for (auto LI : LiveOutVals) {
          if (ECs.findLeader(UR) == ECs.findLeader(LI)) {
            Match = true;
            OV = LI;
            break;
          }
        }
        if (Match)
          break;
      }
    }
    if (Match) {
      for (unsigned I = 0; I < NumPHIValues; I++)
        if (I != II)
          PN->setIncomingValue(I, OV);
    }
  }
  for (auto SubL : L->getSubLoops())
    wrnUpdateSSAPreprocessForOuterLoop(SubL, ValueToLiveinMap, LiveOutVals,
                                       ECs);
}

// Collect the live-in values for the given loop.
void VPOParoptTransform::wrnCollectLiveInVals(
    Loop &L,
    DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
    EquivalenceClasses<Value *> &ECs) {
  BasicBlock *PreheaderBB = L.getLoopPreheader();
  assert(PreheaderBB && "wrnUpdateSSAPreprocess: Loop preheader not found");
  BasicBlock *BB = L.getHeader();

  for (Instruction &I : *BB) {
    if (!isa<PHINode>(I))
      break;
    PHINode *PN = cast<PHINode>(&I);
    unsigned NumPHIValues = PN->getNumIncomingValues();
    unsigned II;
    BasicBlock *InBB;
    Value *IV = nullptr;
    for (II = 0; II < NumPHIValues; ++II) {
      InBB = PN->getIncomingBlock(II);
      if (InBB == PreheaderBB) {
        IV = PN->getIncomingValue(II);
        break;
      }
    }
    if (II != NumPHIValues) {
      Value *Leader = ECs.getOrInsertLeaderValue(PN);
      for (unsigned I = 0; I < NumPHIValues; ++I) {
        BasicBlock *InBB = PN->getIncomingBlock(I);
        if (InBB != PreheaderBB) {
          Value *V = PN->getIncomingValue(I);
          ValueToLiveinMap[V] = {IV, PreheaderBB};
          ECs.unionSets(Leader, V);
        }
      }
    }
  }
}

// The utility to build the equivalence class for the value phi.
void VPOParoptTransform::AnalyzePhisECs(Loop *L, Value *PV, Value *V,
                                        EquivalenceClasses<Value *> &ECs,
                                        SmallPtrSet<PHINode *, 16> &PhiUsers) {

  if (Instruction *I = dyn_cast<Instruction>(V)) {
    if (L->contains(I->getParent())) {
      ECs.unionSets(PV, I);
      if (PHINode *PN = dyn_cast<PHINode>(I))
        if (PhiUsers.insert(PN).second)
          AnalyzePhisECs(L, PV, PN, ECs, PhiUsers);
    }
  }
}

// Build the equivalence class for the value a, b if there exists some phi node
// e.g. a = phi(b).
void VPOParoptTransform::buildECs(Loop *L, PHINode *PN,
                                  EquivalenceClasses<Value *> &ECs) {
  SmallPtrSet<PHINode *, 16> PhiUsers;
  Value *Leader = ECs.getOrInsertLeaderValue(PN);
  unsigned NumPHIValues = PN->getNumIncomingValues();
  unsigned II;
  for (II = 0; II < NumPHIValues; II++)
    AnalyzePhisECs(L, Leader, PN->getIncomingValue(II), ECs, PhiUsers);
}

// Collect the live-out value in the loop.
void VPOParoptTransform::wrnCollectLiveOutVals(
    Loop &L, SmallSetVector<Instruction *, 8> &LiveOutVals,
    EquivalenceClasses<Value *> &ECs) {
  for (Loop::block_iterator II = L.block_begin(), E = L.block_end(); II != E;
       ++II) {
    for (Instruction &I : *(*II)) {
      if (I.getType()->isTokenTy())
        continue;

      for (const Use &U : I.uses()) {
        const Instruction *UI = cast<Instruction>(U.getUser());
        const BasicBlock *UserBB = UI->getParent();
        if (const PHINode *P = dyn_cast<PHINode>(UI))
          UserBB = P->getIncomingBlock(U);

        if (!L.contains(UserBB)) {
          LiveOutVals.insert(&I);
          if (isa<PHINode>(I))
            buildECs(&L, cast<PHINode>(&I), ECs);
        }
      }
    }
  }
  // Any variable except the loop index which has loop carried dependence
  // has to be added into the live-out list.

  for (Instruction &I : *L.getLoopLatch()) {
    if (!isa<PHINode>(I))
      break;
    if (WRegionUtils::getOmpCanonicalInductionVariable(&L) == &I)
      continue;
    // If any use occurs in the loop header, the loop carried dependence
    // exists.
    bool Match = false;
    for (const Use &U : I.uses()) {
      const Instruction *UI = cast<Instruction>(U.getUser());
      const BasicBlock *UserBB = UI->getParent();
      if (UserBB == L.getHeader()) {
        Match = true;
        break;
      }
    }
    if (Match) {
      LiveOutVals.insert(&I);
      buildECs(&L, dyn_cast<PHINode>(&I), ECs);
    }
  }
}

// The utility to update the liveout set from the given BB.
void VPOParoptTransform::wrnUpdateLiveOutVals(
    Loop *L, BasicBlock *BB, SmallSetVector<Instruction *, 8> &LiveOutVals,
    EquivalenceClasses<Value *> &ECs) {
  for (auto I = BB->begin(); I != BB->end(); ++I) {
    Value *ExitVal = &*I;
    if (ExitVal->use_empty())
      continue;
    PHINode *PN = dyn_cast<PHINode>(ExitVal);
    if (!PN)
      break;
    LiveOutVals.insert(PN);
    buildECs(L, PN, ECs);
  }
}

// Collect the live-in value for the phis at the loop header.
void VPOParoptTransform::wrnUpdateSSAPreprocess(
    Loop *L,
    DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
    SmallSetVector<Instruction *, 8> &LiveOutVals,
    EquivalenceClasses<Value *> &ECs) {

  wrnCollectLiveInVals(*L, ValueToLiveinMap, ECs);
  wrnCollectLiveOutVals(*L, LiveOutVals, ECs);
}

// Update the SSA form after the basic block LoopExitBB's successor
// is added one more incoming edge.
void VPOParoptTransform::rewriteUsesOfOutInstructions(
    DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
    SmallSetVector<Instruction *, 8> &LiveOutVals,
    EquivalenceClasses<Value *> &ECs) {
  SmallVector<PHINode *, 2> InsertedPHIs;
  SSAUpdater SSA(&InsertedPHIs);

  PredIteratorCache PredCache;

  // The following code updates the value %split at the BB %omp.inner.for.end
  // by inserting a phi node at the BB %loop.region.exit
  //
  // Before the SSA update:
  // omp.inner.for.cond.omp.inner.for.end_crit_edge:
  //   %split = phi i32 [ %inc, %omp.inner.for.inc ]
  //   br label %loop.region.exit
  //
  // loop.region.exit:
  //   br label %omp.inner.for.end
  //
  // omp.inner.for.end:
  //   %l.0.lcssa = phi i32 [ %split, %loop.region.exit ],
  //                        [ 0, %DIR.OMP.LOOP.2 ]
  //   br label %omp.loop.exit
  //
  // After the SSA update:
  //
  // omp.inner.for.cond.omp.inner.for.end_crit_edge:
  //   %split = phi i32 [ %inc, %omp.inner.for.inc ]
  //   br label %loop.region.exit
  //
  // loop.region.exit:
  //   %split18 = phi i32 [ 0, %omp.inner.for.body.lr.ph ],
  //              [ %split, %omp.inner.for.cond.omp.inner.for.end_crit_edge ]
  //   br label %omp.inner.for.end
  //
  // omp.inner.for.end:
  //   %l.0.lcssa = phi i32 [ %split18, %loop.region.exit ],
  //                        [ 0, %DIR.OMP.LOOP.2 ]
  //   br label %omp.loop.exit
  //

  BasicBlock *FirstLoopExitBB;
  for (auto I : LiveOutVals) {
    Value *ExitVal = I;
    if (ExitVal->use_empty())
      continue;
    Instruction *EI = dyn_cast<Instruction>(ExitVal);
    if (!EI)
      continue;
    FirstLoopExitBB = EI->getParent();
    SSA.Initialize(ExitVal->getType(), ExitVal->getName());

    BasicBlock *OrigPreheader = nullptr;
    Value *OrigPreHeaderVal = nullptr;

    for (auto M : ValueToLiveinMap) {
      if (ECs.findLeader(M.first) == ECs.findLeader(EI)) {
        OrigPreheader = M.second.second;
        OrigPreHeaderVal = M.second.first;
        SSA.AddAvailableValue(M.second.second, M.second.first);
        break;
      }
    }
    SSA.AddAvailableValue(EI->getParent(), EI);
    // OrigPreheader can be empty since the instrution EI may not have
    // other incoming value.
    for (Value::use_iterator UI = ExitVal->use_begin(), UE = ExitVal->use_end();
         UI != UE;) {
      Use &U = *UI;
      ++UI;

      Instruction *UserInst = cast<Instruction>(U.getUser());
      if (!isa<PHINode>(UserInst)) {
        BasicBlock *UserBB = UserInst->getParent();

        if (UserBB == FirstLoopExitBB)
          continue;

        if (!OrigPreheader && UserBB == OrigPreheader) {
          U = OrigPreHeaderVal;
          continue;
        }
      }

      SSA.RewriteUse(U);
    }
  }
}

// Replace the use of OldV within region W with the value NewV.
void VPOParoptTransform::replaceUseWithinRegion(WRegionNode *W, Value *OldV,
                                                Value *NewV) {
  SmallVector<Instruction *, 8> OldUses;
  Loop *L = W->getWRNLoopInfo().getLoop();
  for (auto IB = OldV->user_begin(), IE = OldV->user_end(); IB != IE; ++IB) {
    if (Instruction *User = dyn_cast<Instruction>(*IB))
      if (L->contains(User->getParent()))
        OldUses.push_back(User);
  }

  while (!OldUses.empty()) {
    Instruction *UI = OldUses.pop_back_val();
    UI->replaceUsesOfWith(OldV, NewV);
  }
}

bool VPOParoptTransform::genLoopSchedulingCode(
    WRegionNode *W, AllocaInst *&IsLastVal,
    Instruction *&InsertLastIterCheckBeforeOut, Instruction *&NewOmpLBInstOut,
    Instruction *&NewOmpZttOut) {
  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genLoopSchedulingCode\n");

  assert(W->getIsOmpLoop() && "genLoopSchedulingCode: not a loop-type WRN");

  W->populateBBSet();

  Loop *L = W->getWRNLoopInfo().getLoop();
  assert(L && "genLoopSchedulingCode: Loop not found");
  // The following assertion guarantees that the loop has
  // a pre-header and a latch block.
  assert(L->isLoopSimplifyForm() &&
         "genLoopSchedulingCode: loop must be in normal form.");

  LLVM_DEBUG(dbgs() << "--- Parallel For LoopInfo: \n" << *L);
  LLVM_DEBUG(dbgs() << "--- Loop Preheader: " << *(L->getLoopPreheader())
                    << "\n");
  LLVM_DEBUG(dbgs() << "--- Loop Header: " << *(L->getHeader()) << "\n");
  LLVM_DEBUG(dbgs() << "--- Loop Latch: " << *(L->getLoopLatch()) << "\n\n");

  // If there is an ORDERED(n) clause, we consider this a doacross loop,
  // even though there may be no ordered constructs inside the loop.
  bool IsDoacrossLoop =
      ((isa<WRNParallelLoopNode>(W) || isa<WRNWksLoopNode>(W)) &&
       W->getOrdered() > 0);
  bool IsDistForLoop = isa<WRNDistributeNode>(W);
  bool IsDistParLoop = isa<WRNDistributeParLoopNode>(W);
  bool IsDistChunkedParLoop = false;

  LLVMContext &C = F->getContext();
  IntegerType *Int32Ty = Type::getInt32Ty(C);

  DenseMap<Value *, std::pair<Value *, BasicBlock *>> ValueToLiveinMap;
  SmallSetVector<Instruction *, 8> LiveOutVals;
  EquivalenceClasses<Value *> ECs;
  wrnUpdateSSAPreprocess(L, ValueToLiveinMap, LiveOutVals, ECs);

  //
  // This is initial implementation of parallel loop scheduling to get
  // a simple loop to work end-to-end.
  //
  // TBD: handle all loop forms: Top test loop, bottom test loop, with
  // PHI and without PHI nodes as SCEV bails out for many cases
  //
  Type *LoopIndexType =
      WRegionUtils::getOmpCanonicalInductionVariable(L)->
          getIncomingValue(0)->getType();

  IntegerType *IndValTy = cast<IntegerType>(LoopIndexType);
  assert(IndValTy->getIntegerBitWidth() >= 32 &&
         "Omp loop index type width is less than 32 bit.");

  Value *LBInitVal = WRegionUtils::getOmpLoopLowerBound(L);
  Instruction *PHTerm =
      cast<Instruction>(L->getLoopPreheader()->getTerminator());
  IRBuilder<> PHBuilder(PHTerm);

  // Insert all new alloca instructions at the region's entry block.
  // We used to insert them in the loop pre-header block and rely
  // on the code extractor to hoist them to the entry block of the
  // outlined function. But this does not work for "omp for" loop,
  // which is not oulined, so we used to end up with alloca instructions
  // in the middle of the function body.
  // FIXME: for regions that are not outlined (e.g. "omp for"),
  //        we need to insert new alloca instructions in an entry block
  //        of a parent region that will be outlined, or in the function's
  //        entry block.
  IRBuilder<> REBuilder(&(W->getEntryBBlock()->front()));

  // Create variables for the loop sharing initialization.
  // The required variables are LowerBnd, UpperBnd, Stride and UpperD.
  // The last one is only need for distribute loop.
  IsLastVal = REBuilder.CreateAlloca(Int32Ty, nullptr, "is.last");
  IsLastVal->setAlignment(4);
  // Initialize %is.last with zero.
  REBuilder.CreateAlignedStore(REBuilder.getInt32(0), IsLastVal, 4);

  AllocaInst *LowerBnd = REBuilder.CreateAlloca(IndValTy, nullptr, "lower.bnd");
  LowerBnd->setAlignment(4);

  AllocaInst *UpperBnd = REBuilder.CreateAlloca(IndValTy, nullptr, "upper.bnd");
  UpperBnd->setAlignment(4);

  AllocaInst *Stride = REBuilder.CreateAlloca(IndValTy, nullptr, "stride");
  Stride->setAlignment(4);

  // UpperD is for distribute loop
  AllocaInst *UpperD = REBuilder.CreateAlloca(IndValTy, nullptr, "upperD");
  UpperD->setAlignment(4);

  // Get Schedule kind and chunk information from W-Region node
  // Default: static_even.
  WRNScheduleKind SchedKind = VPOParoptUtils::getLoopScheduleKind(W);
  ConstantInt *SchedType = REBuilder.getInt32(SchedKind);
  Value *DistChunkVal = REBuilder.getInt32(1);

  if (IsDistParLoop || IsDistForLoop) {
    // Get dist_schedule kind and chunk information from W-Region node
    // Default: DistributeStaticEven.
    if (VPOParoptUtils::getDistLoopScheduleKind(W) ==
        WRNScheduleDistributeStatic) {
      DistChunkVal = W->getDistSchedule().getChunkExpr();
#if 0
      // FIXME: enable this back, when FE starts capturing dist_schedule
      //        chunk size.
      if (!isa<Constant>(DistChunkVal)) {
        resetValueInOmpClauseGeneric(W, DistChunkVal);
        DistChunkVal =
            VPOParoptUtils::cloneInstructions(DistChunkVal, PHTerm);
        PHBuilder.SetInsertPoint(PHTerm);
      }
#endif
      IsDistChunkedParLoop = IsDistParLoop;
    }
  }

  // Initialize arguments for loop sharing init call.
  LoadInst *LoadTid = PHBuilder.CreateAlignedLoad(TidPtrHolder, 4, "my.tid");

  // Cast the original lower bound value to the type of the induction
  // variable.
  if (LBInitVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    LBInitVal = PHBuilder.CreateSExtOrTrunc(LBInitVal, IndValTy);

  PHBuilder.CreateAlignedStore(LBInitVal, LowerBnd, 4);

  Value *UpperBndVal =
      VPOParoptUtils::computeOmpUpperBound(W, PHTerm, ".for.scheduling");
  assert(UpperBndVal &&
         "genLoopSchedulingCode: Expect non-empty loop upper bound");
  PHBuilder.SetInsertPoint(PHTerm);

  if (UpperBndVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    UpperBndVal = PHBuilder.CreateSExtOrTrunc(UpperBndVal, IndValTy);

  PHBuilder.CreateAlignedStore(UpperBndVal, UpperBnd, 4);

  bool IsNegStride;
  Value *StrideVal = WRegionUtils::getOmpLoopStride(L, IsNegStride);
  StrideVal = VPOParoptUtils::cloneInstructions(StrideVal, PHTerm);
  PHBuilder.SetInsertPoint(PHTerm);

  if (IsNegStride)
    StrideVal = PHBuilder.CreateSub(ConstantInt::get(IndValTy, 0), StrideVal);

  if (StrideVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    StrideVal = PHBuilder.CreateSExtOrTrunc(StrideVal, IndValTy);

  PHBuilder.CreateAlignedStore(StrideVal, Stride, 4);
  PHBuilder.CreateAlignedStore(UpperBndVal, UpperD, 4);

  ICmpInst* LoopBottomTest = WRegionUtils::getOmpLoopBottomTest(L);

  bool IsUnsigned = LoopBottomTest->isUnsigned();
  // FIXME: this variable is not needed, because we can always compute
  //        the size in the routines it is passed to.
  int Size = IndValTy->getIntegerBitWidth();

  Value *ChunkVal =
      (SchedKind == WRNScheduleStaticEven ||
       SchedKind == WRNScheduleOrderedStaticEven) ?
          ConstantInt::get(IndValTy, 1) : W->getSchedule().getChunkExpr();

  if (!isa<Constant>(ChunkVal) && !isa<WRNWksLoopNode>(W)) {
    resetValueInOmpClauseGeneric(W, ChunkVal);
    ChunkVal = VPOParoptUtils::cloneInstructions(ChunkVal, PHTerm);
    PHBuilder.SetInsertPoint(PHTerm);
  }

  LLVM_DEBUG(dbgs() << "--- Schedule Chunk Value: " << *ChunkVal << "\n\n");

  AllocaInst *TeamIsLast = nullptr;
  AllocaInst *TeamLowerBnd = nullptr;
  AllocaInst *TeamUpperBnd = nullptr;
  AllocaInst *TeamStride = nullptr;
  AllocaInst *TeamUpperD = nullptr;
  CallInst *KmpcTeamInitCI = nullptr;
  LoadInst *TeamLB = nullptr;
  LoadInst *TeamUB = nullptr;
  LoadInst *TeamST = nullptr;

  if (IsDistChunkedParLoop) {
    // Create variables for the team distribution initialization.
    // Insert alloca instructions in the region's entry block.
    TeamIsLast = REBuilder.CreateAlloca(Int32Ty, nullptr, "team.is.last");
    TeamIsLast->setAlignment(4);
    // Initialize %team.is.last with zero.
    REBuilder.CreateAlignedStore(REBuilder.getInt32(0), TeamIsLast, 4);

    TeamLowerBnd = REBuilder.CreateAlloca(IndValTy, nullptr, "team.lower.bnd");
    TeamLowerBnd->setAlignment(4);

    TeamUpperBnd = REBuilder.CreateAlloca(IndValTy, nullptr, "team.upper.bnd");
    TeamUpperBnd->setAlignment(4);

    TeamStride = REBuilder.CreateAlloca(IndValTy, nullptr, "team.stride");
    TeamStride->setAlignment(4);

    TeamUpperD = REBuilder.CreateAlloca(IndValTy, nullptr, "team.upperD");
    TeamUpperD->setAlignment(4);

    // Initialize arguments for team distribution init call.
    // Insert store instructions and the call in the loop pre-header block.
    PHBuilder.CreateAlignedStore(LBInitVal, TeamLowerBnd, 4);
    PHBuilder.CreateAlignedStore(UpperBndVal, TeamUpperBnd, 4);
    PHBuilder.CreateAlignedStore(StrideVal, TeamStride, 4);
    PHBuilder.CreateAlignedStore(UpperBndVal, TeamUpperD, 4);

    // Generate __kmpc_team_static_init_4{u}/8{u} Call Instruction
    // FIXME: we'd better pass the builder instead of the PHTerm.
    KmpcTeamInitCI = VPOParoptUtils::genKmpcTeamStaticInit(W, IdentTy,
                               LoadTid, TeamIsLast, TeamLowerBnd,
                               TeamUpperBnd, TeamStride, StrideVal,
                               DistChunkVal, Size, IsUnsigned, PHTerm);
    // FIXME: we'd better pass PHBuilder to genKmpcTeamStaticInit,
    //        and avoid potential PHBuilder invalidation.
    PHBuilder.SetInsertPoint(PHTerm);

    // If we generate dispatch loop for team distribute, TeamLB
    // will be the splitting point, where the team dispatch header
    // will start.
    TeamLB = PHBuilder.CreateAlignedLoad(TeamLowerBnd, 4, "team.new.lb");
    TeamUB = PHBuilder.CreateAlignedLoad(TeamUpperBnd, 4, "team.new.ub");
    TeamST = PHBuilder.CreateAlignedLoad(TeamStride, 4, "team.new.st");
    auto *TeamUD = PHBuilder.CreateAlignedLoad(TeamUpperBnd, 4, "team.new.ud");

    // Store the team bounds as the loop's initial bounds
    // for further work sharing.
    PHBuilder.CreateAlignedStore(TeamLB, LowerBnd, 4);
    PHBuilder.CreateAlignedStore(TeamUB, UpperBnd, 4);
    PHBuilder.CreateAlignedStore(TeamST, Stride, 4);
    PHBuilder.CreateAlignedStore(TeamUD, UpperD, 4);
  }

  CallInst *KmpcInitCI = nullptr;
  CallInst *KmpcNextCI = nullptr;

  // Worksharing-loop with ORDERED clause requires dispatching - in this case
  // the schedule kind will be one of the Ordered.
  // When scheduling is static or static-even, the dispatching
  // is not required.  Note that static-even is the default
  // for distribute-for loops (WRNDistributeNode), which also do not
  // require dispatching.  Thus, we do not check for distribute-for
  // loops here, and rely on SchedKind instead.
  bool DoesNotRequireDispatch =
    (SchedKind == WRNScheduleStatic || SchedKind == WRNScheduleStaticEven);

  if (DoesNotRequireDispatch) {
    // Generate either __kmpc_for_static_init or __kmpc_dist_for_static_init
    // call instruction at the end of the loop's pre-header.
    KmpcInitCI =
        VPOParoptUtils::genKmpcStaticInit(
            W, IdentTy, LoadTid, IsLastVal,
            LowerBnd, UpperBnd, UpperD,
            Stride, StrideVal,
            IsDistForLoop ? DistChunkVal : ChunkVal,
            IsUnsigned, PHTerm);

    if (isa<WRNDistributeParLoopNode>(W) &&
        VPOParoptUtils::getDistLoopScheduleKind(W) ==
        WRNScheduleDistributeStaticEven) {
      // For "distribute parallel for schedule(static, N)" we need to take
      // minimum between the upper bound and the upper bound of the dist_chunk
      // returned by __kmpc_dist_for_static_init.  Update UpperBndVal with
      // the value of upperD returned by the run-time.  It will be used
      // by genDispatchLoopForStatic() below.
      PHBuilder.SetInsertPoint(PHTerm);
      UpperBndVal = PHBuilder.CreateAlignedLoad(UpperD, 4, "static.upperD");
    }
  } else {
    // Generate __kmpc_dispatch_init_4{u}/8{u} Call Instruction

    // If team distribution is involved, then we need to call initialize
    // with the team's bounds.  We could have used TeamLB and TeamUB
    // here directly, but having the explicit loads makes unit testing
    // easier.
    auto *DispInitLB = PHBuilder.CreateAlignedLoad(LowerBnd, 4, "disp.init.lb");
    auto *DispInitUB = PHBuilder.CreateAlignedLoad(UpperBnd, 4, "disp.init.ub");
    KmpcInitCI =
        VPOParoptUtils::genKmpcDispatchInit(W, IdentTy, LoadTid, SchedType,
                                            IsLastVal, DispInitLB, DispInitUB,
                                            StrideVal, ChunkVal, Size,
                                            IsUnsigned, PHTerm);

    // Generate __kmpc_dispatch_next_4{u}/8{u} Call Instruction
    KmpcNextCI =
        VPOParoptUtils::genKmpcDispatchNext(W, IdentTy, LoadTid, IsLastVal,
                                            LowerBnd, UpperBnd, Stride, Size,
                                            IsUnsigned, PHTerm);
  }

  // Add implicit barrier for FP+LP/Linear variables before the init call.
  genBarrierForFpLpAndLinears(W, KmpcInitCI);

  // Insert doacross_init call for ordered(n)
  if (IsDoacrossLoop)
    VPOParoptUtils::genKmpcDoacrossInit(W, IdentTy, LoadTid, KmpcInitCI,
                                        W->getOrderedTripCounts());

  // Update the PHBuilder after the above calls.
  // FIXME: we should actually pass the builder to these functions.
  PHBuilder.SetInsertPoint(PHTerm);

  // Generate zero trip-count check for the bounds computed
  // by the run-time init call.  The insertion point is
  // the end of the loop's pre-header block.

  // First, load the bounds initialized with run-time values.
  // NOTE: LoadLB is used as a split point for dispatch.
  LoadInst *LoadLB = PHBuilder.CreateAlignedLoad(LowerBnd, 4, "lb.new");
  LoadInst *LoadUB = PHBuilder.CreateAlignedLoad(UpperBnd, 4, "ub.new");

  // Fixup the induction variable's PHI to take the initial value
  // from the value of the lower bound returned by the run-time init.
  PHINode *PN = WRegionUtils::getOmpCanonicalInductionVariable(L);
  PN->removeIncomingValue(L->getLoopPreheader());
  PN->addIncoming(LoadLB, L->getLoopPreheader());

  // We have the loop's structure intact so far.
  // Now we start chaging CFG, so it is hard to use IRBuilder below.
  //
  // Currently we have the following:
  //   if (optional_original_ztt) {
  //   loop_preheader:
  //     <static init code>
  //
  //     do { // loop_header
  //       <loop body>
  //     } while (lb < ub);
  //
  //   loop_exit_block:
  //   } // optional
  //
  //   region_exit_block:
  //
  assert(isa<BranchInst>(PHTerm) && PHTerm->getNumSuccessors() == 1 &&
         "Expect preheader BB has one exit!");

  bool IsLeft;
  CmpInst::Predicate PD =
      VPOParoptUtils::computeOmpPredicate(
          WRegionUtils::getOmpPredicate(L, IsLeft));
  auto *NewZTTCompInst =
      PHBuilder.CreateICmp(PD, LoadLB, LoadUB, "omp.ztt");
  NewOmpLBInstOut = cast<Instruction>(LoadLB);
  NewOmpZttOut = cast<Instruction>(NewZTTCompInst);

  // Update the loop's predicate to use LoadUB value for the upper
  // bound.
  VPOParoptUtils::updateOmpPredicateAndUpperBound(W, LoadUB, PHTerm);

  // Split the loop's exit block to simplify further CFG manipulations.
  BasicBlock *LoopExitBB = WRegionUtils::getOmpExitBlock(L);

  BasicBlock *LoopRegionExitBB =
      SplitBlock(LoopExitBB, LoopExitBB->getFirstNonPHI(), DT, LI);
  LoopRegionExitBB->setName("loop.region.exit");

  // Update the region's exit block pointer, if needed.
  if (LoopExitBB == W->getExitBBlock())
    W->setExitBBlock(LoopRegionExitBB);

  //  auto *NewLoopExitBB = LoopExitBB;
  //  auto *NewLoopRegionExitBB = LoopRegionExitBB;

  std::swap(LoopExitBB, LoopRegionExitBB);
  Instruction *NewTermInst =
      BranchInst::Create(PHTerm->getSuccessor(0), LoopExitBB, NewZTTCompInst);
  ReplaceInstWithInst(PHTerm, NewTermInst);

  if (DT)
    DT->changeImmediateDominator(LoopExitBB, NewTermInst->getParent());

  // We have the following now:
  //   if (optional_original_ztt) {
  //   loop_preheader:
  //     <static init code>
  //     if (new_ztt) {
  //       do { // loop_header
  //         <loop body>
  //       } while (lb < ub);
  //
  //     loop_region_exit_block:
  //       <optional PHIs>
  //     }
  //   loop_exit_block:
  //     <...>
  //   } // optional
  //
  //   region_exit_block:

  CallInst *KmpcFiniCI = nullptr;

  if (DoesNotRequireDispatch) {
    if (SchedKind == WRNScheduleStaticEven) {

      // Insert fini call so that it is paired with the static init call.
      // The LoopExitBB may be split after the fini calls for team
      // distribution loop.
      auto *FiniInsertPt = &LoopExitBB->front();

      KmpcFiniCI =
          VPOParoptUtils::genKmpcStaticFini(W, IdentTy, LoadTid,
                                            FiniInsertPt);

      // Insert doacross_fini call for ordered(n) after the fini call.
      if (IsDoacrossLoop)
        KmpcFiniCI =
            VPOParoptUtils::genKmpcDoacrossFini(W, IdentTy, LoadTid,
                                                FiniInsertPt);

      // Since we inserted a new ZTT branch, we need to update SSA for
      // values living out of the loop_region_exit_block.
      wrnUpdateLiveOutVals(L, LoopRegionExitBB, LiveOutVals, ECs);
      rewriteUsesOfOutInstructions(ValueToLiveinMap, LiveOutVals, ECs);

    } else if (SchedKind == WRNScheduleStatic) {

      // Insert fini call so that it is paired with the static init call.
      // The LoopExitBB may be split after the fini calls for team
      // distribution loop.
      auto *FiniInsertPt = &LoopExitBB->front();

      KmpcFiniCI =
          VPOParoptUtils::genKmpcStaticFini(W, IdentTy, LoadTid,
                                            FiniInsertPt);

      // Insert doacross_fini call for ordered(n) after the fini call.
      if (IsDoacrossLoop)
        KmpcFiniCI =
            VPOParoptUtils::genKmpcDoacrossFini(W, IdentTy, LoadTid,
                                                FiniInsertPt);

      // Generate a loop to iterate over chunks computed by the run-time.
      BasicBlock *StaticInitBB = KmpcInitCI->getParent();
      Loop *OuterLoop = genDispatchLoopForStatic(
          L, LoadLB, LoadUB, LowerBnd, UpperBnd, UpperBndVal, Stride,
          LoopExitBB, StaticInitBB, LoopRegionExitBB);

      // Do linear, lpriv copyout before incrementing lb/ub for the next chunk.
      // This is needed because otherwise, linear copyout would use the value
      // from the closed-form computation using 'lb + stride` after the end of
      // the last chunk.
      InsertLastIterCheckBeforeOut = LoopRegionExitBB->getFirstNonPHI();

      // Update SSA to account values living out of the loop_region_exit_block
      // (note that we did not introduce new live outs, even though we added
      // a dispatch latch block after loop_region_exit_block), and also
      // the new loop created.
      wrnUpdateLiveOutVals(OuterLoop, LoopRegionExitBB, LiveOutVals, ECs);
      wrnUpdateSSAPreprocessForOuterLoop(OuterLoop, ValueToLiveinMap,
                                         LiveOutVals, ECs);
      rewriteUsesOfOutInstructions(ValueToLiveinMap, LiveOutVals, ECs);
    } else
      llvm_unreachable("Unexpected scheduling kind.");
  } else {
    //                |
    //      Disptach Loop HeaderBB <-----------+
    //             lb < ub                     |
    //              | |                        |
    //        +-----+ |                        |
    //        |       |                        |
    //        |   Loop HeaderBB: <--+          |
    //        |  i = phi(lb,i')     |          |
    //        |    /  |  ...        |          |
    //        |   /   |  ...        |          |
    //        |  |    |             |  <Lastpivate copyout>
    //        |   \   |             |          |
    //        |    i' = i + 1 ------+          |
    //        |     i' < ub                    |
    //        |       |                        |
    //        |       |                        |
    //        |  Dispatch Loop Latch           |
    //        |       |                        |
    //        |       |------------------------+
    //        |       |
    //        +-->Loop ExitBB
    //                |

    // FIXME: isolate this code into an utility routine.
    if (isa<WRNWksLoopNode>(W) && W->getOrdered() == 0)
      // Dispatch fini must be emitted inside the loop,
      // so that it is executed on every iteration.
      VPOParoptUtils::genKmpcDispatchFini(W, IdentTy, LoadTid, Size,
                                          IsUnsigned,
                                          LoopBottomTest);

    // Insert doacross_fini call for ordered(n)
    if (IsDoacrossLoop)
      VPOParoptUtils::genKmpcDoacrossFini(W, IdentTy, LoadTid,
                                          &LoopExitBB->front());

    BasicBlock *DispatchInitBB = KmpcNextCI->getParent();

    BasicBlock *DispatchHeaderBB =
        SplitBlock(DispatchInitBB, KmpcNextCI, DT, LI);
    DispatchHeaderBB->setName("dispatch.header" + Twine(W->getNumber()));

    BasicBlock *DispatchBodyBB = SplitBlock(DispatchHeaderBB, LoadLB, DT, LI);
    DispatchBodyBB->setName("dispatch.body" + Twine(W->getNumber()));

    Instruction *TermInst = DispatchHeaderBB->getTerminator();

    ICmpInst* CondInst =
        new ICmpInst(TermInst, ICmpInst::ICMP_NE,
                     KmpcNextCI, ConstantInt::getSigned(Int32Ty, 0),
                     "dispatch.cond" + Twine(W->getNumber()));

    Instruction *NewTermInst = BranchInst::Create(DispatchBodyBB,
                                                  LoopExitBB, CondInst);
    ReplaceInstWithInst(TermInst, NewTermInst);

    TermInst = LoopRegionExitBB->getTerminator();
    TermInst->setSuccessor(0, DispatchHeaderBB);

    // To support non-monotonic scheduling, lastprivate copyout should happen
    // before the jump back to the dispatch_next call.
    InsertLastIterCheckBeforeOut = TermInst;

    // Update Dispatch Header BB Branch instruction
    TermInst = DispatchHeaderBB->getTerminator();
    TermInst->setSuccessor(1, LoopExitBB);

    if (DT) {
      DT->changeImmediateDominator(DispatchHeaderBB, DispatchInitBB);
      DT->changeImmediateDominator(DispatchBodyBB, DispatchHeaderBB);

      DT->changeImmediateDominator(LoopExitBB, DispatchHeaderBB);
    }
    Loop *OuterLoop = WRegionUtils::createLoop(L, L->getParentLoop(), LI);
    WRegionUtils::updateBBForLoop(DispatchHeaderBB, OuterLoop,
                                  L->getParentLoop(), LI);
    WRegionUtils::updateBBForLoop(DispatchBodyBB, OuterLoop, L->getParentLoop(),
                                  LI);
    WRegionUtils::updateBBForLoop(LoopRegionExitBB, OuterLoop,
                                  L->getParentLoop(), LI);
    OuterLoop->moveToHeader(DispatchHeaderBB);

    wrnUpdateLiveOutVals(OuterLoop, LoopRegionExitBB, LiveOutVals, ECs);
    wrnUpdateSSAPreprocessForOuterLoop(OuterLoop, ValueToLiveinMap, LiveOutVals,
                                       ECs);
    rewriteUsesOfOutInstructions(ValueToLiveinMap, LiveOutVals, ECs);
  }

  if (IsDistChunkedParLoop) {
    // FIXME: clean-up this function's parameters.
    Loop *OuterLoop = genDispatchLoopForTeamDistirbute(
        L, TeamLB, TeamUB, TeamST, TeamLowerBnd, TeamUpperBnd, TeamStride,
        UpperBndVal, LoopExitBB, LoopRegionExitBB, KmpcTeamInitCI->getParent(),
        LoopExitBB, KmpcFiniCI);
    wrnUpdateLiveOutVals(OuterLoop, LoopRegionExitBB, LiveOutVals, ECs);
    wrnUpdateSSAPreprocessForOuterLoop(OuterLoop, ValueToLiveinMap, LiveOutVals,
                                       ECs);
    rewriteUsesOfOutInstructions(ValueToLiveinMap, LiveOutVals, ECs);

    //// LLVM_DEBUG(dbgs() << "After distribute par Loop scheduling: "
    ////                   << *TeamInitBB->getParent() << "\n\n");
  }

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genLoopSchedulingCode\n");

  // There are new BBlocks generated, so we need to reset BBSet
  W->resetBBSet();
  return true;
}

// Collects the alloc stack variables where the tid stores.
void VPOParoptTransform::getAllocFromTid(CallInst *Tid) {
  Instruction *User;
  for (auto IB = Tid->user_begin(), IE = Tid->user_end();
       IB != IE; IB++) {
    User = dyn_cast<Instruction>(*IB);
    if (User) {
      StoreInst *S0 = dyn_cast<StoreInst>(User);
      if (S0) {
        assert(S0->isSimple() && "Expect non-volatile store instruction.");
        Value *V = S0->getPointerOperand();
        AllocaInst *AI = dyn_cast<AllocaInst>(V);
        if (AI)
          TidAndBidInstructions.insert(AI);
        else
          llvm_unreachable("Expect the stack alloca instruction.");
      }
    }
  }
}

bool VPOParoptTransform::genMultiThreadedCode(WRegionNode *W) {
  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genMultiThreadedCode\n");

  W->populateBBSet();

  // Remove references from num_teams/thread_limit or num_threads
  // clauses so that they do not appear as live-ins for the code extractor.
  resetValueInNumTeamsAndThreadsClause(W);

  // Set up Fn Attr for the new function
  Function *NewF = VPOParoptUtils::genOutlineFunction(*W, DT, AC);
  if (hasParentTarget(W))
    NewF->addFnAttr("target.declare", "true");

  CallInst *NewCall = cast<CallInst>(NewF->user_back());
  CallSite CS(NewCall);

  unsigned int TidArgNo = 0;
  bool IsTidArg = false;

  for (auto I = CS.arg_begin(), E = CS.arg_end(); I != E; ++I) {
    if (*I == TidPtrHolder) {
      IsTidArg = true;
      LLVM_DEBUG(dbgs() << " NewF Tid Argument: " << *(*I) << "\n");
      break;
    }
    ++TidArgNo;
  }

  // Finalized multithreaded Function declaration and definition
  Function *MTFn = finalizeExtractedMTFunction(W, NewF, IsTidArg, TidArgNo);

  std::vector<Value *> MTFnArgs;

  // Pass tid and bid arguments.
  MTFnArgs.push_back(TidPtrHolder);
  MTFnArgs.push_back(BidPtrHolder);
  genThreadedEntryActualParmList(W, MTFnArgs);

  LLVM_DEBUG(dbgs() << " New Call to MTFn: " << *NewCall << "\n");
  // Pass all the same arguments of the extracted function.
  for (auto I = CS.arg_begin(), E = CS.arg_end(); I != E; ++I) {
    if (*I != TidPtrHolder) {
      LLVM_DEBUG(dbgs() << " NewF Arguments: " << *(*I) << "\n");
      MTFnArgs.push_back((*I));
    }
  }

  CallInst *MTFnCI = CallInst::Create(MTFn, MTFnArgs, "", NewCall);
  MTFnCI->setCallingConv(CS.getCallingConv());

  // Copy isTailCall attribute
  if (NewCall->isTailCall())
    MTFnCI->setTailCall();

  MTFnCI->setDebugLoc(NewCall->getDebugLoc());

  // MTFnArgs.clear();

  if (!NewCall->use_empty())
    NewCall->replaceAllUsesWith(MTFnCI);

  // Keep the orginal extraced function name after finalization
  MTFnCI->takeName(NewCall);
  BasicBlock *MTFnBB = MTFnCI->getParent();

  if (GeneralUtils::hasNextUniqueInstruction(MTFnCI)) {
    Instruction* NextI = GeneralUtils::nextUniqueInstruction(MTFnCI);
    SplitBlock(MTFnBB, NextI, DT, LI);
  }

  // Remove the orginal serial call to extracted NewF from the program,
  // reducing the use-count of NewF
  NewCall->eraseFromParent();

  // Finally, nuke the original extracted function.
  NewF->eraseFromParent();

  // Generate __kmpc_fork_call for multithreaded execution of MTFn call
  CallInst* ForkCI = genForkCallInst(W, MTFnCI);

  // Generate __kmpc_ok_to_fork test Call Instruction
  CallInst* ForkTestCI = VPOParoptUtils::genKmpcForkTest(W, IdentTy, ForkCI);

  //
  // Genrerate __kmpc_ok_to_fork test for taking either __kmpc_fork_call
  // or serial call branch, and update CFG and DomTree
  //
  //  ForkTestBB(codeRepl)
  //         /       \
  //        /         \
  // ThenForkBB   ElseCallBB
  //        \       /
  //         \     /
  //  SuccessorOfThenForkBB
  //
  BasicBlock *ForkTestBB = ForkTestCI->getParent();

  BasicBlock *ForkBB = ForkCI->getParent();

  BasicBlock *ThenForkBB = SplitBlock(ForkBB, ForkCI, DT, LI);
  ThenForkBB->setName("if.then.fork." + Twine(W->getNumber()));

  BasicBlock *CallBB = MTFnCI->getParent();

  BasicBlock *ElseCallBB = SplitBlock(CallBB, MTFnCI, DT, LI);
  ElseCallBB->setName("if.else.call." + Twine(W->getNumber()));

  Function *F = ForkTestBB->getParent();
  LLVMContext &C = F->getContext();

  ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);

  Instruction *TermInst = ForkTestBB->getTerminator();

  Value *IfClauseValue = nullptr;

  if (!W->getIsTeams())
    IfClauseValue = W->getIf();

  ICmpInst* CondInst = nullptr;

  if (IfClauseValue) {
    IRBuilder<> Builder(TermInst);

    // FE may generate if() condition as a boolean value,
    // so we need to cast it to the type of __kmpc_ok_to_fork().
    IfClauseValue = Builder.CreateZExtOrTrunc(IfClauseValue,
                                              ForkTestCI->getType());
    auto *IfAndForkTestCI =
        Builder.CreateAnd(IfClauseValue, ForkTestCI, "and.if.clause");
    cast<Instruction>(IfAndForkTestCI)->setDebugLoc(TermInst->getDebugLoc());
    CondInst = new ICmpInst(TermInst, ICmpInst::ICMP_NE,
                            IfAndForkTestCI, ValueZero, "if.fork.test");
  }
  else
    CondInst = new ICmpInst(TermInst, ICmpInst::ICMP_NE,
                            ForkTestCI, ValueZero, "fork.test");

  Instruction *NewTermInst = BranchInst::Create(ThenForkBB, ElseCallBB,
                                                CondInst);
  ReplaceInstWithInst(TermInst, NewTermInst);

  Instruction *NewForkBI = BranchInst::Create(
                               ElseCallBB->getTerminator()->getSuccessor(0));

  ReplaceInstWithInst(ThenForkBB->getTerminator(), NewForkBI);

  DT->changeImmediateDominator(ThenForkBB, ForkTestCI->getParent());
  DT->changeImmediateDominator(ElseCallBB, ForkTestCI->getParent());
  DT->changeImmediateDominator(ThenForkBB->getTerminator()->getSuccessor(0),
                               ForkTestCI->getParent());

  // Generate __kmpc_push_num_threads(...) Call Instruction
  Value *NumThreads = nullptr;
  Value *NumTeams = nullptr;
  if (W->getIsTeams()) {
    NumTeams = W->getNumTeams();
    assert((!VPOParoptUtils::useSPMDMode(W) || !NumTeams) &&
           "SPMD mode cannot be used with num_teams.");
    NumThreads = W->getThreadLimit();
  } else
    NumThreads = W->getNumThreads();

  if (NumThreads || NumTeams) {
    LoadInst *Tid = new LoadInst(TidPtrHolder, "my.tid", ForkCI);
    Tid->setAlignment(4);
    if (W->getIsTeams())
      VPOParoptUtils::genKmpcPushNumTeams(W, IdentTy, Tid, NumTeams,
                                          NumThreads, ForkCI);
    else
      VPOParoptUtils::genKmpcPushNumThreads(W, IdentTy, Tid, NumThreads,
                                            ForkCI);
  }

  // Remove the serial call to MTFn function from the program, reducing
  // the use-count of MTFn
  // MTFnCI->eraseFromParent();

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genMultiThreadedCode\n");

  W->resetBBSet(); // CFG changed; clear BBSet
  return true;
}

FunctionType *VPOParoptTransform::getKmpcMicroTaskPointerTy() {
  if (!KmpcMicroTaskTy) {
    LLVMContext &C = F->getContext();
    Type *MicroParams[] = {PointerType::getUnqual(Type::getInt32Ty(C)),
                           PointerType::getUnqual(Type::getInt32Ty(C))};
    KmpcMicroTaskTy = FunctionType::get(Type::getVoidTy(C),
                                    MicroParams, true);
  }
  return KmpcMicroTaskTy;
}

CallInst* VPOParoptTransform::genForkCallInst(WRegionNode *W, CallInst *CI) {
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  // Get MicroTask Function for __kmpc_fork_call
  Function *MicroTaskFn = CI->getCalledFunction();
  assert(MicroTaskFn && "genForkCallInst: Expect non-empty function.");
  FunctionType *MicroTaskFnTy = getKmpcMicroTaskPointerTy();
  //MicroTaskFn->getFunctionType();

  // Get MicroTask Function for __kmpc_fork_call
  //
  // Need to add global_tid and bound_tid to Micro Task Function,
  // finalizeExtractedMTFunction is implemented for adding Tid and Bid
  // arguments :
  //   void (*kmpc_micro)(kmp_int32 global_tid, kmp_int32 bound_tid,...)
  //
  // geneate void __kmpc_fork_call(ident_t *loc,
  //                               kmp_int32 argc, (*kmpc_microtask)(), ...);
  //
  Type *ForkParams[] = {PointerType::getUnqual(IdentTy), Type::getInt32Ty(C),
                        PointerType::getUnqual(MicroTaskFnTy)};

  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), ForkParams, true);

  Function *ForkCallFn = (!isa<WRNTeamsNode>(W)) ?
                         M->getFunction("__kmpc_fork_call") :
                         M->getFunction("__kmpc_fork_teams");

  if (!ForkCallFn) {
    if (isa<WRNTeamsNode>(W))
      ForkCallFn = Function::Create(FnTy, GlobalValue::ExternalLinkage,
                                    "__kmpc_fork_teams", M);
    else
      ForkCallFn = Function::Create(FnTy, GlobalValue::ExternalLinkage,
                                    "__kmpc_fork_call", M);

    ForkCallFn->setCallingConv(CallingConv::C);
  }

  AttributeList ForkCallFnAttr;
  SmallVector<AttributeList, 4> Attrs;

  AttributeList FnAttrSet;
  AttrBuilder B;
  FnAttrSet = AttributeList::get(C, ~0U, B);

  Attrs.push_back(FnAttrSet);
  ForkCallFnAttr = AttributeList::get(C, Attrs);

  ForkCallFn->setAttributes(ForkCallFnAttr);

  // get source location information from DebugLoc
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *ExitBB = W->getExitBBlock();

  GlobalVariable *KmpcLoc = VPOParoptUtils::genKmpcLocfromDebugLoc(
      F, CI, IdentTy, KMP_IDENT_KMPC, EntryBB, ExitBB);

  CallSite CS(CI);
  ConstantInt *NumArgs = ConstantInt::get(Type::getInt32Ty(C),
                                          CS.getNumArgOperands()-2);

  std::vector<Value *> Params;
  Params.push_back(KmpcLoc);
  Params.push_back(NumArgs);
  IRBuilder<> Builder(EntryBB);
  Value *Cast =Builder.CreateBitCast(MicroTaskFn,
                           PointerType::getUnqual(MicroTaskFnTy));
  Params.push_back(Cast);

  auto InitArg = CS.arg_begin(); ++InitArg; ++InitArg;

  for (auto I = InitArg, E = CS.arg_end(); I != E; ++I) {
    Params.push_back((*I));
  }

  CallInst *ForkCallInst = CallInst::Create(ForkCallFn, Params, "", CI);

  // CI->replaceAllUsesWith(NewCI);

  ForkCallInst->setCallingConv(CallingConv::C);
  ForkCallInst->setTailCall(false);

  return ForkCallInst;
}

// Generates the actual parameters in the outlined function for
// copyin variables.
void VPOParoptTransform::genThreadedEntryActualParmList(
    WRegionNode *W, std::vector<Value *> &MTFnArgs) {
  if (!W->canHaveCopyin())
    return;
  CopyinClause &CP = W->getCopyin();
  for (auto C : CP.items())
    MTFnArgs.push_back(C->getOrig());
}

// Generates the formal parameters in the outlined function for
// copyin variables. It can be extended for other variables including
// firstprivate, shared, etc.
void VPOParoptTransform::genThreadedEntryFormalParmList(
    WRegionNode *W, std::vector<Type *> &ParamsTy) {
  if (!W->canHaveCopyin())
    return;
  CopyinClause &CP = W->getCopyin();
  for (auto C : CP.items())
    ParamsTy.push_back(C->getOrig()->getType());
}

// Fix the name of copyin formal parameters for outlined function.
void VPOParoptTransform::fixThreadedEntryFormalParmName(WRegionNode *W,
                                                        Function *NFn) {
  if (!W->canHaveCopyin())
    return;
  CopyinClause &CP = W->getCopyin();
  if (!CP.empty()) {
    Function::arg_iterator NewArgI = NFn->arg_begin();
    ++NewArgI;
    ++NewArgI;
    for (auto C : CP.items()) {
      NewArgI->setName("tpv_"+C->getOrig()->getName());
      ++NewArgI;
    }
  }
}

// Emit the code for copyin variable. One example is as follows.
//   %0 = ptrtoint i32* %tpv_a to i64
//   %1 = icmp ne i64 %0, ptrtoint (i32* @a to i64)
//   br i1 %1, label %copyin.not.master, label %copyin.not.master.end
//
// copyin.not.master:                                ; preds = %newFuncRoot
//   %2 = bitcast i32* %tpv_a to i8*
//   call void @llvm.memcpy.p0i8.p0i8.i32(i8*
//                bitcast (i32* @a to i8*), i8* %2, i32 4, i32 4, i1 false)
//   br label %copyin.not.master.end
//
// copyin.not.master.end:       ; preds = %newFuncRoot, %copyin.not.master
//
void VPOParoptTransform::genTpvCopyIn(WRegionNode *W,
                                      Function *NFn) {
  if (!W->canHaveCopyin())
    return;
  CopyinClause &CP = W->getCopyin();
  if (!CP.empty()) {
    Function::arg_iterator NewArgI = NFn->arg_begin();
    Value *FirstArgOfOutlineFunc = &*NewArgI;
    ++NewArgI;
    ++NewArgI;
    const DataLayout NDL=NFn->getParent()->getDataLayout();
    bool FirstArg = true;

    for (auto C : CP.items()) {
      Instruction *Term;
      if (FirstArg) {
        FirstArg = false;

        IRBuilder<> Builder(&NFn->getEntryBlock());
        Builder.SetInsertPoint(NFn->getEntryBlock().getTerminator());

        // The instruction to cast the tpv pointer to int for later comparison
        // instruction. One example is as follows.
        //   %0 = ptrtoint i32* %tpv_a to i64
        Value *TpvArg = Builder.CreatePtrToInt(
                                        &*NewArgI,Builder.getIntPtrTy(NDL));
        Value *OldTpv = Builder.CreatePtrToInt(
                                        C->getOrig(),Builder.getIntPtrTy(NDL));

        // The instruction to compare between the address of tpv formal
        // arugment and the tpv accessed in the outlined function.
        // One example is as follows.
        //   %1 = icmp ne i64 %0, ptrtoint (i32* @a to i64)
        Value *PtrCompare = Builder.CreateICmpNE(TpvArg, OldTpv);
        Term = SplitBlockAndInsertIfThen(PtrCompare,
                                         NFn->getEntryBlock().getTerminator(),
                                         false, nullptr, DT, LI);

        // Set the name for the newly generated basic blocks.
        Term->getParent()->setName("copyin.not.master");
        BasicBlock *CopyinEndBB = NFn->getEntryBlock().getTerminator()
            ->getSuccessor(1);
        CopyinEndBB->setName("copyin.not.master.end");
        // Emit a barrier after copyin code for threadprivate variable.
        VPOParoptUtils::genKmpcBarrier(W, FirstArgOfOutlineFunc,
           CopyinEndBB->getTerminator(), IdentTy, true);

      }
      VPOUtils::genMemcpy(
          C->getOrig(), &*NewArgI, NDL,
          dyn_cast<GlobalVariable>(C->getOrig())->getAlignment(),
          Term->getParent());

      ++NewArgI;
    }
  }
}

Function *VPOParoptTransform::finalizeExtractedMTFunction(WRegionNode *W,
                                                          Function *Fn,
                                                          bool IsTidArg,
                                                          unsigned int TidArgNo,
                                                          bool hasBid) {

  LLVMContext &C = Fn->getContext();

  // Computing a new prototype for the function, which is the same as
  // the old function with two new parameters for passing tid and bid
  // required by OpenMP runtime library.
  FunctionType *FnTy = Fn->getFunctionType();

  std::vector<Type *> ParamsTy;

  if (hasBid) {
    ParamsTy.push_back(PointerType::getUnqual(Type::getInt32Ty(C)));
    ParamsTy.push_back(PointerType::getUnqual(Type::getInt32Ty(C)));
  } else
    ParamsTy.push_back(Type::getInt32Ty(C));

  genThreadedEntryFormalParmList(W, ParamsTy);

  unsigned int TidParmNo = 0;
  for (auto ArgTyI = FnTy->param_begin(), ArgTyE = FnTy->param_end();
       ArgTyI != ArgTyE; ++ArgTyI) {

    // Matching formal argument and actual argument for Thread ID
    if (!IsTidArg || TidParmNo != TidArgNo)
      ParamsTy.push_back(*ArgTyI);

    ++TidParmNo;
  }

  Type *RetTy = FnTy->getReturnType();
  FunctionType *NFnTy = FunctionType::get(RetTy, ParamsTy, false);

  // Create the new function body and insert it into the module...
  Function *NFn = Function::Create(NFnTy, Fn->getLinkage());

  NFn->copyAttributesFrom(Fn);
  if (W->getWRegionKindID() == WRegionNode::WRNTaskloop ||
      W->getWRegionKindID() == WRegionNode::WRNTask)
    NFn->addFnAttr("task-mt-func", "true");
  else
    NFn->addFnAttr("mt-func", "true");

  Fn->getParent()->getFunctionList().insert(Fn->getIterator(), NFn);
  NFn->takeName(Fn);

  // Since we have now created the new function, splice the body of the old
  // function right into the new function, leaving the old rotting hulk of
  // the function empty.
  NFn->getBasicBlockList().splice(NFn->begin(), Fn->getBasicBlockList());

  // Loop over the argument list, transferring uses of the old arguments over
  // to the new arguments, also transferring over the names as well.
  Function::arg_iterator NewArgI = NFn->arg_begin();


  // The first argument is *tid - thread id argument
  NewArgI->setName("tid");
  ++NewArgI;

  // The second argument is *bid - binding thread id argument
  if (hasBid) {
    NewArgI->setName("bid");
    ++NewArgI;
  }

  fixThreadedEntryFormalParmName(W, NFn);
  genTpvCopyIn(W, NFn);

  if (W->canHaveCopyin()) {
    unsigned Cnt =  W->getCopyin().size();
    NewArgI += Cnt;
  }

  // For each argument, move the name and users over to the new version.
  TidParmNo = 0;
  for (Function::arg_iterator I = Fn->arg_begin(),
                              E = Fn->arg_end(); I != E; ++I) {
    // Matching formal argument and actual argument for Thread ID
    if (IsTidArg && TidParmNo == TidArgNo) {
      Function::arg_iterator TidArgI = NFn->arg_begin();
      I->replaceAllUsesWith(&*TidArgI);
      TidArgI->takeName(&*I);
    } else {
      I->replaceAllUsesWith(&*NewArgI);
      NewArgI->takeName(&*I);
      ++NewArgI;
    }
    ++TidParmNo;
  }

  DenseMap<const Function *, DISubprogram *> FunctionDIs;

  // Patch the pointer to LLVM function in debug info descriptor.
  auto DI = FunctionDIs.find(Fn);
  if (DI != FunctionDIs.end()) {
    DISubprogram *SP = DI->second;
    // SP->replaceFunction(NFn);

    // Ensure the map is updated so it can be reused on non-varargs argument
    // eliminations of the same function.
    FunctionDIs.erase(DI);
    FunctionDIs[NFn] = SP;
  }
  return NFn;
}

// Generate code for master/end master construct and update LLVM control-flow
// and dominator tree accordingly
bool VPOParoptTransform::genMasterThreadCode(WRegionNode *W,
                                             bool IsTargetSPIRV) {
  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genMasterThreadCode\n");
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *ExitBB = W->getExitBBlock();

  Instruction *InsertPt = EntryBB->getTerminator();

  // Generate __kmpc_master Call Instruction
  CallInst *MasterCI = VPOParoptUtils::genKmpcMasterOrEndMasterCall(
      W, IdentTy, TidPtrHolder, InsertPt, true, IsTargetSPIRV);
  MasterCI->insertBefore(InsertPt);

  // LLVM_DEBUG(dbgs() << " MasterCI: " << *MasterCI << "\n\n");

  Instruction *InsertEndPt = ExitBB->getTerminator();

  // Generate __kmpc_end_master Call Instruction
  CallInst *EndMasterCI = VPOParoptUtils::genKmpcMasterOrEndMasterCall(
      W, IdentTy, TidPtrHolder, InsertEndPt, false, IsTargetSPIRV);
  EndMasterCI->insertBefore(InsertEndPt);

  // Generate (int)__kmpc_master(&loc, tid) test for executing code using
  // Master thread.
  //
  // __kmpc_master return: 1: master thread, 0: non master thread
  //
  //      MasterBBTest
  //         /    \
  //        /      \
  //   MasterBB   emptyBB
  //        \      /
  //         \    /
  //   SuccessorOfMasterBB
  //
  BasicBlock *MasterTestBB = MasterCI->getParent();
  BasicBlock *MasterBB = EndMasterCI->getParent();

  BasicBlock *ThenMasterBB = MasterTestBB->getTerminator()->getSuccessor(0);
  BasicBlock *SuccEndMasterBB = MasterBB->getTerminator()->getSuccessor(0);

  ThenMasterBB->setName("if.then.master." + Twine(W->getNumber()));

  Function *F = MasterTestBB->getParent();
  LLVMContext &C = F->getContext();

  ConstantInt *ValueOne = ConstantInt::get(Type::getInt32Ty(C), 1);

  Instruction *TermInst = MasterTestBB->getTerminator();

  ICmpInst* CondInst = new ICmpInst(TermInst, ICmpInst::ICMP_EQ,
                                    MasterCI, ValueOne, "");

  Instruction *NewTermInst = BranchInst::Create(ThenMasterBB,
                                                   SuccEndMasterBB, CondInst);
  ReplaceInstWithInst(TermInst, NewTermInst);

  DT->changeImmediateDominator(ThenMasterBB,
                               MasterCI->getParent());
  DT->changeImmediateDominator(ThenMasterBB->getTerminator()->getSuccessor(0),
                               MasterCI->getParent());

  W->resetBBSet(); // Invalidate BBSet
  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genMasterThreadCode\n");
  return true; // Changed
}

// Generate code for single/end single construct and update LLVM control-flow
// and dominator tree accordingly
bool VPOParoptTransform::genSingleThreadCode(WRegionNode *W,
                                             AllocaInst *&IsSingleThread) {
  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genSingleThreadCode\n");
  W->populateBBSet();
  BasicBlock *EntryBB = W->getEntryBBlock();

  Instruction *InsertPt = EntryBB->getTerminator();
  CopyprivateClause &CprivClause = W->getCpriv();

  IRBuilder<> Builder(InsertPt);
  if (!CprivClause.empty()) {
    IsSingleThread = Builder.CreateAlloca(Type::getInt32Ty(F->getContext()),
                                          nullptr, "is.single.thread");
    Builder.CreateStore(
        ConstantInt::getSigned(Type::getInt32Ty(F->getContext()), 0),
        IsSingleThread);
  }

  // Generate __kmpc_single Call Instruction
  CallInst *SingleCI = VPOParoptUtils::genKmpcSingleOrEndSingleCall(
      W, IdentTy, TidPtrHolder, InsertPt, true);
  SingleCI->insertBefore(InsertPt);

  // InsertEndPt should be right before ExitBB->begin(), so create a new BB
  // that is split from the ExitBB to be used as InsertEndPt.
  // Reuse the util that does this for Reduction and Lastprivate fini code.
  //
  // Note: InsertEndPt should not be ExitBB->rbegin() because the
  // _kmpc_end_single() should be emitted above the END SINGLE directive, not
  // after it.
  BasicBlock *NewBB = nullptr;
  createEmptyPrivFiniBB(W, NewBB);
  Instruction *InsertEndPt = NewBB->getTerminator();

  if (!CprivClause.empty()) {
    Builder.SetInsertPoint(InsertEndPt);
    Builder.CreateStore(
        ConstantInt::getSigned(Type::getInt32Ty(F->getContext()), 1),
        IsSingleThread);
  }

  // Generate __kmpc_end_single Call Instruction
  CallInst *EndSingleCI = VPOParoptUtils::genKmpcSingleOrEndSingleCall(
      W, IdentTy, TidPtrHolder, InsertEndPt, false);
  EndSingleCI->insertBefore(InsertEndPt);

  // Generate (int)__kmpc_single(&loc, tid) test for executing code using
  // Single thread, the  __kmpc_single return:
  //
  //    1: the single region can be executed by the current encounting
  //       thread in the team.
  //
  //    0: the single region can not be executed by the current encounting
  //       thread, as it has been executed by another thread in the team.
  //
  //      SingleBBTest
  //         /    \
  //        /      \
  //   SingleBB   emptyBB
  //        \      /
  //         \    /
  //   SuccessorOfSingleBB
  //
  BasicBlock *SingleTestBB = SingleCI->getParent();
  BasicBlock *EndSingleBB = EndSingleCI->getParent();

  BasicBlock *ThenSingleBB = SingleTestBB->getTerminator()->getSuccessor(0);
  BasicBlock *EndSingleSuccBB = EndSingleBB->getTerminator()->getSuccessor(0);

  ThenSingleBB->setName("if.then.single." + Twine(W->getNumber()));

  Function *F = SingleTestBB->getParent();
  LLVMContext &C = F->getContext();

  ConstantInt *ValueOne = ConstantInt::get(Type::getInt32Ty(C), 1);

  Instruction *TermInst = SingleTestBB->getTerminator();

  ICmpInst* CondInst = new ICmpInst(TermInst, ICmpInst::ICMP_EQ,
                                    SingleCI, ValueOne, "");

  Instruction *NewTermInst = BranchInst::Create(ThenSingleBB,
                                                   EndSingleSuccBB, CondInst);
  ReplaceInstWithInst(TermInst, NewTermInst);

  DT->changeImmediateDominator(ThenSingleBB, SingleCI->getParent());
  DT->changeImmediateDominator(ThenSingleBB->getTerminator()->getSuccessor(0),
                               SingleCI->getParent());

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genSingleThreadCode\n");

  W->resetBBSet(); // Invalidate BBSet
  return true;  // Changed
}

// Generate code for ordered/end ordered construct for preserving ordered
// region execution order
bool VPOParoptTransform::genOrderedThreadCode(WRegionNode *W) {
  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genOrderedThreadCode\n");
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *ExitBB = W->getExitBBlock();

  // Generate (void)__kmpc_ordered(&loc, tid) and
  //          (void)__kmpc_end_ordered(&loc, tid) calls
  // for executing the ordered code region
  //
  //       OrderedBB
  //         /    \
  //        /      \
  //       BB ...  BB
  //        \      /
  //         \    /
  //      EndOrderedBB

  Instruction *InsertPt = EntryBB->getTerminator();

  // Generate __kmpc_ordered Call Instruction
  CallInst *OrderedCI = VPOParoptUtils::genKmpcOrderedOrEndOrderedCall(
      W, IdentTy, TidPtrHolder, InsertPt, true);
  OrderedCI->insertBefore(InsertPt);

  Instruction *InsertEndPt = ExitBB->getTerminator();

  // Generate __kmpc_end_ordered Call Instruction
  CallInst *EndOrderedCI = VPOParoptUtils::genKmpcOrderedOrEndOrderedCall(
      W, IdentTy, TidPtrHolder, InsertEndPt, false);
  EndOrderedCI->insertBefore(InsertEndPt);

  // BasicBlock *OrderedBB = OrderedCI->getParent();
  // LLVM_DEBUG(dbgs() << " Ordered Entry BBlock: " << *OrderedBB << "\n\n");

  // BasicBlock *EndOrderedBB = EndOrderedCI->getParent();
  // LLVM_DEBUG(dbgs() << " Ordered Exit BBlock: " << *EndOrderedBB << "\n\n");

  W->resetBBSet(); // Invalidate BBSet
  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genOrderedThreadCode\n");
  return true;  // Changed
}

// Emit __kmpc_doacross_post/wait call for an 'ordered depend(source/sink)'
// construct.
bool VPOParoptTransform::genDoacrossWaitOrPost(WRNOrderedNode *W) {
  assert(W &&"genDoacrossWaitOrPost: Null WRN");
  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genDoacrossWaitOrPost\n");
  BasicBlock *EntryBB = W->getEntryBBlock();
  Instruction *InsertPt = EntryBB->getTerminator();

  auto *WParent = W->getParent();
  (void)WParent;
  assert(WParent && "Orphaned ordered depend source/sink construct.");
  assert(WParent->getIsOmpLoop() && "Parent is not a loop-type WRN");

  // Emit doacross post call for 'depend(source)'
  if (!W->getDepSource().empty()) {
    assert(W->getDepSource().size() == 1 && "More than one depend(source)");
    DepSourceItem *DSI = W->getDepSource().back();
    // Generate __kmpc_doacross_post call
    CallInst *DoacrossPostCI = VPOParoptUtils::genDoacrossWaitOrPostCall(
        W, IdentTy, TidPtrHolder, InsertPt, DSI->getDepExprs(),
        true); // 'depend (source)'
    (void)DoacrossPostCI;
    assert(DoacrossPostCI && "Failed to emit doacross_post call.");
  }

  // Emit doacross wait call(s) for 'depend(sink:...)'
  for (DepSinkItem *DSI : W->getDepSink().items()) {
    // Generate __kmpc_doacross_wait call
    CallInst *DoacrossWaitCI = VPOParoptUtils::genDoacrossWaitOrPostCall(
        W, IdentTy, TidPtrHolder, InsertPt, DSI->getDepExprs(),
        false); // 'depend (sink:...)'
    (void)DoacrossWaitCI;
    assert(DoacrossWaitCI && "Failed to emit doacross_wait call.");
  }

  W->resetBBSet(); // Invalidate BBSet
  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genDoacrossWaitOrPost\n");
  return true; // Changed
}

// Generates code for the OpenMP critical construct.
bool VPOParoptTransform::genCriticalCode(WRNCriticalNode *CriticalNode) {
  if (isTargetSPIRV()) {
    F->getContext().diagnose(ParoptDiagInfo(*F,
        CriticalNode->getEntryDirective()->getDebugLoc(),
        "OpenMP critical is not supported"));
    return removeCompilerGeneratedFences(CriticalNode);
  }

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genCriticalCode\n");
  assert(CriticalNode != nullptr && "Critical node is null.");

  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(TidPtrHolder != nullptr && "TidPtr is null.");

  // genKmpcCriticalSection() needs BBSet for error checking only;
  // In the future consider getting rid of this call to populateBBSet.
  CriticalNode->populateBBSet();

  StringRef LockNameSuffix = CriticalNode->getUserLockName();

  bool CriticalCallsInserted =
      LockNameSuffix.empty()
          ? VPOParoptUtils::genKmpcCriticalSection(CriticalNode, IdentTy,
                                                   TidPtrHolder,
                                                   isTargetSPIRV())
          : VPOParoptUtils::genKmpcCriticalSection(
                CriticalNode, IdentTy, TidPtrHolder, isTargetSPIRV(),
                LockNameSuffix);

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Handling of Critical Node: "
                    << (CriticalCallsInserted ? "Successful" : "Failed")
                    << ".\n");

  assert(CriticalCallsInserted && "Failed to create critical section. \n");

  CriticalNode->resetBBSet(); // Invalidate BBSet
  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genCriticalCode\n");
  return CriticalCallsInserted;
}

// Emits an implicit barrier at the end of WRgion W if W contains
// variables that are linear, or both firstprivate-lastprivate. e.g.
//
//   #pragma omp for firstprivate(x) lastprivate(x) nowait
//
// Emitted pseudocode:
//
//   %x.local = @x                         ; (1) firstpricate copyin
//   __kmpc_static_init(...)
//   ...
//   __kmpc_static_fini(...)
//
//   __kmpc_barrier(...)                   ; (2)
//   @x = %x.local                         ; (3) lastprivate copyout
//
//  The barrier (2) is needed to prevent a race between (1) and (3), which
//  read/write to/from @x.
//
//  For supporting non-monotonic scheduling of loops, the barrier needs to
//  be inserted before the init call shown , which is done by passing in
//  the insert point as InsertBefore.
bool VPOParoptTransform::genBarrierForFpLpAndLinears(
    WRegionNode *W, Instruction *InsertBefore) {

  // A barrier is needed for capturing the initial value of linear
  // variables.
  bool BarrierNeeded = W->canHaveLinear() && !((W->getLinear()).empty());

  // A barrier is also needed if a variable is marked as both firstprivate and
  // non-conditional lastprivate.
  if (!BarrierNeeded && (W->canHaveLastprivate() && W->canHaveFirstprivate())) {
    LastprivateClause &LprivClause = W->getLpriv();
    for (LastprivateItem *LprivI : LprivClause.items()) {
      if (LprivI->getIsConditional())
        continue;
      if (LprivI->getInFirstprivate()) {
        BarrierNeeded = true;
        break;
      }
    }
  }

  if (!BarrierNeeded)
    return false;

  LLVM_DEBUG(
      dbgs()
      << __FUNCTION__
      << ": Emitting implicit barrier for FP-LP/Linear clause operands.\n");

  genBarrier(W, false /*IsExplicit*/, false /*IsTargetSPIRV*/, InsertBefore);

  W->resetBBSet(); // CFG changed; clear BBSet
  return true;
}

Instruction *VPOParoptTransform::genBarrierForConditionalLP(WRegionNode *W) {

  if (!W->getIsOmpLoop() || W->getIsTask() || isa<WRNVecLoopNode>(W))
    // Conditional lastprivate is either not supported, or doesn't need a
    // barrier.
    return nullptr;

  LastprivateClause &LprivClause = W->getLpriv();

  bool BarrierNeeded =
      !LprivClause.empty() &&
      std::any_of(LprivClause.items().begin(), LprivClause.items().end(),
                  [](LastprivateItem *LI) { return LI->getIsConditional(); });

  if (!BarrierNeeded)
    return nullptr;

  LLVM_DEBUG(
      dbgs()
      << __FUNCTION__
      << ": Emitting implicit barrier for conditional LP clause operands.\n");

  Instruction *Barrier = nullptr;
  genBarrier(W, false /*IsExplicit*/, false /*IsTargetSPIRV*/, nullptr,
             &Barrier);

  W->resetBBSet(); // CFG changed; clear BBSet
  return Barrier;
}

// Emits an if-then branch using IsLastVal and sets IfLastIterOut to
// the if-then BBlock. This is used for emitting the final copy-out code for
// linear and lastprivate clause operands.
//
// Code generated looks like:
//
//                          |
//                         (1)  %15 = load i32, i32* %is.last
//                         (2)  %16 = icmp ne i32 %15, 0
//                          |   br i1 %16, label %last.then, label %last.done
//                          |
//                          |   last.then:        ; IfLastIterOut
//                          |   ...
//                          |   br last.done
//                          |
//                          |   last.done:
//                          |   br exit.BB.predecessor
//                          |
//                         (3)  exit.BB.predecessor:
//                          |   br exit.BB
//                          |
// exit.BB:                 |   exit.BB:
// llvm.region.exit(...)    |   llvm.region.exit(...)
//                          |
bool VPOParoptTransform::genLastIterationCheck(
    WRegionNode *W, Value *IsLastVal, BasicBlock *&IfLastIterOut,
    Instruction *InsertBefore) {

  // No need to emit the branch if W doesn't have any linear or non-conditional
  // lastprivate var.
  if ((!W->canHaveLastprivate() || (W->getLpriv()).empty() ||
       std::all_of(
           W->getLpriv().items().begin(), W->getLpriv().items().end(),
           [](LastprivateItem *LI) { return LI->getIsConditional(); })) &&
      (!W->canHaveLinear() || (W->getLinear()).empty()))
    return false;

  assert(IsLastVal && "genLastIterationCheck: IsLastVal is null.");

  if (!InsertBefore) {
    BasicBlock *ExitBBPredecessor = nullptr;
    // First, create an empty predecessor BBlock for ExitBB of the WRegion.  (3)
    createEmptyPrivFiniBB(W, ExitBBPredecessor);
    assert(ExitBBPredecessor && "genLoopLastIterationCheck: Couldn't create "
                                "empty BBlock before the exit BB.");
    InsertBefore = ExitBBPredecessor->getTerminator();
  }

  // Next, we insert the branching code before InsertBefore.
  IRBuilder<> Builder(InsertBefore);

  LoadInst *LastLoad = Builder.CreateLoad(IsLastVal);                   // (1)
  ConstantInt *ValueZero =
      ConstantInt::getSigned(Type::getInt32Ty(F->getContext()), 0);
  Value *LastCompare = Builder.CreateICmpNE(LastLoad, ValueZero);       // (2)

  Instruction *Term = SplitBlockAndInsertIfThen(LastCompare, InsertBefore,
                                                false, nullptr, DT, LI);
  Term->getParent()->setName("last.then");
  InsertBefore->getParent()->setName("last.done");

  IfLastIterOut = Term->getParent();

  LLVM_DEBUG(
      dbgs() << __FUNCTION__
             << ": Emitted if-then branch for checking last iteration.\n");

  W->resetBBSet(); // CFG changed; clear BBSet
  return true;
}

// Insert a call to __kmpc_barrier() at the end of the construct (or before
// InsertBefore).
bool VPOParoptTransform::genBarrier(WRegionNode *W, bool IsExplicit,
                                    bool IsTargetSPIRV,
                                    Instruction *InsertBefore,
                                    Instruction **BarrierOut) {

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genBarrier [explicit="
                    << IsExplicit << "]\n");

  bool InsertPtProvided = (InsertBefore != nullptr);

  if (!InsertPtProvided) {
    // Create a new BB split from W's ExitBB to be used as InsertPt.
    // Reuse the util that does this for Reduction and Lastprivate fini code.
    BasicBlock *NewBB = nullptr;
    createEmptyPrivFiniBB(W, NewBB);
    InsertBefore = NewBB->getTerminator();
  }

  Instruction *Barrier = VPOParoptUtils::genKmpcBarrier(
      W, TidPtrHolder, InsertBefore, IdentTy, IsExplicit, IsTargetSPIRV);
  if (BarrierOut)
    *BarrierOut = Barrier;

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genBarrier\n");

  if (!InsertPtProvided)
    W->resetBBSet(); // CFG changed because of NewBB; clear BBSet
  return true;
}

// Create a __kmpc_flush() call and insert it into W's EntryBB
bool VPOParoptTransform::genFlush(WRegionNode *W) {

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genFlush\n");

  BasicBlock *EntryBB = W->getEntryBBlock();
  Instruction *InsertPt = EntryBB->getTerminator();
  VPOParoptUtils::genKmpcFlush(W, IdentTy, InsertPt);

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genFlush\n");
  return true;
}

// Insert a call to __kmpc_cancel/__kmpc_cancellation_point at the end of the
// construct
bool VPOParoptTransform::genCancelCode(WRNCancelNode *W) {

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genCancelCode\n");

  BasicBlock *EntryBB = W->getEntryBBlock();
  Instruction *InsertPt = EntryBB->getTerminator();

  auto *IfExpr = W->getIf();
  if (IfExpr) {
    assert(!W->getIsCancellationPoint() && "IF expr on a Cancellation Point.");

    // If the construct has an 'IF' clause, we need to generate code like:
    // if (if_expr != 0) {
    //   %1 = __kmpc_cancel(...);
    // } else {
    //   %2 = __kmpc_cancellationpoint(...);
    // }
    Function *F = EntryBB->getParent();
    LLVMContext &C = F->getContext();
    ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);

    auto *CondInst = new ICmpInst(InsertPt, ICmpInst::ICMP_NE, IfExpr,
                                  ValueZero, "cancel.if");

    Instruction *IfCancelThen = nullptr;
    Instruction *IfCancelElse = nullptr;

    SplitBlockAndInsertIfThenElse(CondInst, InsertPt, &IfCancelThen,
                                  &IfCancelElse);
    assert(IfCancelThen && "genCancelCode: Null Then Inst for Cancel If");
    assert(IfCancelElse && "genCancelCode: Null Else Inst for Cancel If");

    IfCancelThen->getParent()->setName(CondInst->getName() + ".then");
    IfCancelElse->getParent()->setName(CondInst->getName() + ".else");

    LLVM_DEBUG(
        dbgs() << "genCancelCode: Emitted If-Then-Else for IF EXPR: if (";
        IfExpr->printAsOperand(dbgs());
        dbgs() << ") then <%x = __kmpc_cancel>; else <%y = "
                  "__kmpc_cancellationpoint>;.\n");

    // Insert the __kmpc_cancellationpoint for the ELSE branch here.
    CallInst *ElseCPCall = VPOParoptUtils::genKmpcCancelOrCancellationPointCall(
        W, IdentTy, TidPtrHolder, IfCancelElse, W->getCancelKind(), true);
    (void)ElseCPCall;
    assert(ElseCPCall && "genCancelCode: Failed to emit cancellationpoint call "
                         "for cancel 'if'.");

    // Set the insert point for the __kmpc_cancel call.
    InsertPt = IfCancelThen;
  }

  CallInst *CancelCall = VPOParoptUtils::genKmpcCancelOrCancellationPointCall(
      W, IdentTy, TidPtrHolder, InsertPt, W->getCancelKind(),
      W->getIsCancellationPoint());

  (void)CancelCall;
  assert(CancelCall && "genCancelCode: Failed to emit call");

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genCancelCode\n");

  W->resetBBSet(); // CFG changed; clear BBSet
  return true;
}

// Propagate all cancellation points from the body of W, to the 'region.entry'
// directive for the region.
bool VPOParoptTransform::propagateCancellationPointsToIR(WRegionNode *W) {

  if (!W->canHaveCancellationPoints())
    return false;

  auto &CancellationPoints = W->getCancellationPoints();
  if (CancellationPoints.empty())
    return false;

  // Find the region.entry() directive intrinsic.
  BasicBlock *EntryBB = W->getEntryBBlock();
  Instruction *Inst = EntryBB->getFirstNonPHI();
  CallInst *CI = dyn_cast<CallInst>(Inst);

  assert(CI && "propagateCancellationPointsToIR: Exit BBlocks's first "
               "non-PHI Instruction is not a Call");
  assert(
      VPOAnalysisUtils::isOpenMPDirective(CI) &&
      "propagateCancellationPointsToIR: Cannot find region.exit() directive");

  // We first create Allocas to store the return values of the runtime calls.
  // The allocas (e.g. '%cp') are used in the region entry directive to provide
  // a handle on the cancellation points (e.g. '%1') inside the region. The
  // alloca '%cp' is needed because:
  //   (i) '%1' is defined after the region entry directive.
  //   (ii) Adding '%1' to the region exit directive needs 'polluting' the IR
  //   with unnecessary PHIs.
  //
  // So %cp is used in the directive, and %1 is stored to '%cp'.
  //
  // The IR after propagateCancellationPointsToIR will look like:
  //
  //    %cp = alloca i32                                              ; (1)
  //    ...
  //    region.entry(..., %cp, ...)                                   ; (2)
  //    ...
  //    %1 = call i32 __kmpc_cancel_barrier(...)                      ; (3)
  //    store i32 %1, i32* %cp                                        ; (4)
  //    ...
  SmallVector<Value *, 2> CancellationPointAllocas;
  Function *F = EntryBB->getParent();
  LLVMContext &C = F->getContext();
  Type *I32Type = Type::getInt32Ty(C);

  BasicBlock &FunctionEntry = F->getEntryBlock();
  IRBuilder<> AllocaBuilder(FunctionEntry.getFirstNonPHI());

  for (auto *CancellationPoint : CancellationPoints) {               // (3)
    AllocaInst *CPAlloca =
        AllocaBuilder.CreateAlloca(I32Type, nullptr, "cp");          // (1)

    StoreInst *CPStore = new StoreInst(CancellationPoint, CPAlloca); // (4)
    CPStore->insertAfter(CancellationPoint);
    CancellationPointAllocas.push_back(CPAlloca);
  }

  // (2) Add the list of allocas where cancellation points' return values are
  // stored, as an operand bundle in the region.entry() directive.
  CI = VPOParoptUtils::addOperandBundlesInCall(
      CI, {{"QUAL.OMP.CANCELLATION.POINTS", CancellationPointAllocas}});

  LLVM_DEBUG(dbgs() << "propagateCancellationPointsToIR: Added "
                    << CancellationPoints.size()
                    << " Cancellation Points to: " << *CI << ".\n");

  W->setEntryDirective(CI);
  W->resetBBSet(); // CFG changed; clear BBSet
  return true;
}

// Removes from IR, the allocas and stores created by
// propagateCancellationPointsToIR(). This is done in the vpo-paropt
// transformation pass after the information has already been consumed. The
// function also removes these allocas from the "QUAL.OMP.CANCELLATION.POINTS"
// clause on the region.entry intrinsic.
bool VPOParoptTransform::clearCancellationPointAllocasFromIR(WRegionNode *W) {
  if (!W->canHaveCancellationPoints())
    return false;

  auto &CancellationPointAllocas = W->getCancellationPointAllocas();
  if (CancellationPointAllocas.empty())
    return false;

  LLVM_DEBUG(
      dbgs()
      << "\nEnter VPOParoptTransform::clearCancellationPointAllocasFromIR\n");

  bool Changed = false;

  // The IR at this point looks like:
  //
  //    %cp = alloca i32                                              ; (1)
  //    ...
  //    region.entry(..., %cp, ...)                                   ; (2)
  //    ...
  //    %1 = call i32 __kmpc_cancel_barrier(...)
  //    store i32 %1, i32* %cp                                        ; (3)
  //    ...
  for (AllocaInst *CPAlloca : CancellationPointAllocas) {            // (1)

    // The only uses of CPAlloca (1) should be in the intrinsic (2) and the
    // store (3), and maybe some bitcasts used in lifetime begin/end markers
    // for the variable that the inliner may insert.

    IntrinsicInst *RegionEntry = nullptr;                            // (2)
    SmallVector<Instruction *, 4> CPAllocaUsersToDelete;

    for (auto It = CPAlloca->user_begin(), IE = CPAlloca->user_end(); It != IE;
         ++It) {
      if (IntrinsicInst *Intrinsic = dyn_cast<IntrinsicInst>(*It))
        RegionEntry = Intrinsic;
      else if (StoreInst *CPStore = dyn_cast<StoreInst>(*It)) { //      (3)
        CPAllocaUsersToDelete.push_back(CPStore);
      } else if (auto *CastI = dyn_cast<CastInst>(*It)) {
        for (User *CastUser : CastI->users()) {
          assert(isa<IntrinsicInst>(CastUser) &&
                 (cast<IntrinsicInst>(CastUser)->getIntrinsicID() ==
                      Intrinsic::lifetime_start ||
                  cast<IntrinsicInst>(CastUser)->getIntrinsicID() ==
                      Intrinsic::lifetime_end) &&
                 "Unexpected cast on cancellation point alloca.");
          CPAllocaUsersToDelete.push_back(cast<Instruction>(CastUser));
        }
        CPAllocaUsersToDelete.push_back(CastI);
      } else
        llvm_unreachable("Unexpected user of cancellation point alloca.");
    }

    assert(RegionEntry &&
           "Unable to find intrinsic using cancellation point alloca.");
    assert(VPOAnalysisUtils::isOpenMPDirective(RegionEntry) &&
           "Unexpected user of cancellation point alloca.");

    LLVMContext &C = F->getContext();

    // Remove the cancellation point alloca from the `region.entry` directive.
    //    region.entry(..., null, ...)                                  ; (2)
    RegionEntry->replaceUsesOfWith(
        CPAlloca, ConstantPointerNull::get(Type::getInt8PtrTy(C)));

    // Next, we delete (1), (3), and any other users of CPAlloca. Now, CPStore
    // may have been removed by some dead-code elimination optimization. e.g.
    //
    //   if (expr) {
    //     %1 = __kmpc_cancel(...)
    //     store %1, %cp
    //   }
    //
    // 'expr' may be always false, and %1 and the store can be optimized away.
    for (auto *CPAU : CPAllocaUsersToDelete)
      CPAU->eraseFromParent();

    CPAlloca->eraseFromParent();
    Changed = true;
  }

  LLVM_DEBUG(
      dbgs()
      << "\nExit VPOParoptTransform::clearCancellationPointAllocasFromIR\n");

  W->resetBBSetIfChanged(Changed); // Clear BBSet if transformed
  return Changed;
}

// Insert If-Then-Else branches from each Cancellation Point in W, to
// jump to the end of W if the Cancellation Point is non-zero.
bool VPOParoptTransform::genCancellationBranchingCode(WRegionNode *W) {

  if (!W->canHaveCancellationPoints())
    return false;

  auto &CancellationPoints = W->getCancellationPoints();
  if (CancellationPoints.empty())
    return false;

  LLVM_DEBUG(
      dbgs() << "\nEnter VPOParoptTransform::genCancellationBranchingCode\n");

  W->populateBBSet();

  Function *F = W->getEntryBBlock()->getParent();
  LLVMContext &C = F->getContext();
  ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);
  bool DTNeedsRecalculation = false;

  // For a loop construct with static [even] scheduling,
  // __kmpc_static_fini(...) call should be made even if the construt is
  // cancelled.
  bool NeedStaticFiniCall =
      (W->getIsOmpLoop() &&
       (W->getIsSections() ||
        (VPOParoptUtils::getLoopScheduleKind(W) == WRNScheduleStaticEven ||
         VPOParoptUtils::getLoopScheduleKind(W) == WRNScheduleStatic)));

  auto isCancelBarrier = [](Instruction *I) {
    return cast<CallInst>(I)->getCalledFunction()->getName() ==
           "__kmpc_cancel_barrier";
  };

  // For a parallel construct, 'kmpc_cancel' and 'kmpc_cancellationpoint', when
  // cancelled, should call '__kmpc_cancel_barrier'. This is needed to free up
  // threads that are waiting at existing 'kmpc_cancel_barrier's.
  //
  //
  //   T1              T2             T3             T4
  //    |              |              |              |
  //    |    <cancellationpoint>      |              |
  //    |               \             |              |
  //    |                \            |              |
  // <cancel>             |           |              |
  //    \                 |           |              |
  //     \                |           |              |
  //      |               |           |              |
  // -----|---------------|-----------+--------------+-- <cancelbarrier>
  //     /               /            \              \
  //    /               /              \              \
  //    |               |               |              |
  // ---+---------------+---------------|--------------|- <cancelbarrier for
  //    |               |              /              /    cancel[lationpoint]>
  //    |               |             /              /
  //    |               |             |              |
  // ---+---------------+-------------+--------------+--- <fork/join barrier>
  //

  bool NeedCancelBarrierForNonBarriers =
      isa<WRNParallelNode>(W) &&
      std::any_of(CancellationPoints.begin(), CancellationPoints.end(),
                  isCancelBarrier);

  assert((!NeedStaticFiniCall || !NeedCancelBarrierForNonBarriers) &&
         "genCancellationBranchingCode: Cannot need both kmpc_static_fini and "
         "kmpc_cancel_barrier calls in cancelled BB.");

  BasicBlock *CancelExitBBWithStaticFini = nullptr;

  //           +--CancelExitBB--+
  //           +-------+--------+
  //                   |
  //           +-------+--------+
  //           |  region.exit() |
  //           +----------------+
  BasicBlock *CancelExitBB = nullptr;
  createEmptyPrivFiniBB(W, CancelExitBB);

  assert(CancelExitBB &&
         "genCancellationBranchingCode: Failed to create Cancel Exit BB");
  LLVM_DEBUG(dbgs() << "genCancellationBranchingCode: Created CancelExitBB: [";
             CancelExitBB->printAsOperand(dbgs()); dbgs() << "]\n");

  BasicBlock *CancelExitBBForNonBarriers = nullptr;

  for (auto &CancellationPoint : CancellationPoints) {
    assert(CancellationPoint &&
           "genCancellationBranchingCode: Illegal cancellation point");

    bool CancellationPointIsBarrier = isCancelBarrier(CancellationPoint);

    // At this point, IR looks like:
    //
    //    +--------OrgBB----------+
    //    | %x = kmpc_cancel(...) |           ; CancellationPoint
    //    | <NextI>               |
    //    | ...                   |
    //    +-----------+-----------+
    //                |
    //                |
    //    +------CancelExitBB-----+
    //    +-----------+-----------+
    //                |
    //    +-----------+-----------+
    //    | region.exit(...)      |
    //    +-----------------------+
    BasicBlock *OrgBB = CancellationPoint->getParent();

    assert(GeneralUtils::hasNextUniqueInstruction(CancellationPoint) &&
           "genCancellationBranchingCode: Cannot find successor of "
           "Cancellation Point");
    auto *NextI = GeneralUtils::nextUniqueInstruction(CancellationPoint);

    auto *CondInst = new ICmpInst(NextI, ICmpInst::ICMP_NE, CancellationPoint,
                                  ValueZero, "cancel.check");

    BasicBlock *NotCancelledBB = SplitBlock(OrgBB, NextI, DT, LI);
    assert(
        NotCancelledBB &&
        "genCancellationBranchingCode: Cannot split BB at Cancellation Point");

    BasicBlock *CurrentCancelExitBB =
        (CancelExitBBForNonBarriers && !CancellationPointIsBarrier)
            ? CancelExitBBForNonBarriers
            : CancelExitBB;

    OrgBB = CancellationPoint->getParent();
    Instruction *TermInst = OrgBB->getTerminator();
    Instruction *NewTermInst =
        BranchInst::Create(CurrentCancelExitBB, NotCancelledBB, CondInst);
    ReplaceInstWithInst(TermInst, NewTermInst);

    LLVM_DEBUG(
        auto &OS = dbgs();
        OS << "genCancellationBranchingCode: Inserted If-Then-Else: if (";
        CancellationPoint->printAsOperand(OS); OS << ") then [";
        OrgBB->printAsOperand(OS); OS << "] --> [";
        CurrentCancelExitBB->printAsOperand(OS); OS << "], else [";
        OrgBB->printAsOperand(OS); OS << "] --> [";
        NotCancelledBB->printAsOperand(OS); OS << "].\n");

    // The IR now looks like:
    //
    //    +------------OrgBB--------------+
    //    | %x = kmpc_cancel(...)         |           ; CancellationPoint
    //    | %cancel.check = icmp ne %x, 0 |           ; CondInst
    //    +--------------+---+------------+
    //                 0 |   | 1
    //                   |   +-----------------+
    //                   |                     |
    //    +---------NotCancelledBB--------+    |
    //    | <NextI>                       |    |
    //    | ...                           |    |
    //    +--------------+----------------+    |
    //                   |                     |
    //                   |   +-----------------+
    //    +---------CancelExitBB----------+
    //    +--------------+----------------+
    //                   |
    //    +--------------+----------------+
    //    | region.exit(...)              |
    //    +-------------------------------+
    //

    if (DT) {
      // There can be multiple CancellationPoints. We need to update the
      // immediate dominator of CancelExitBB when emitting code for each.
      //
      //               ...
      //                | \
      //                |  \
      //                |   \
      //               ... OrgBB1
      //                |    / |
      //                |   /  |
      //                |  /0  |1
      //                | /    |
      //               ...     |
      //                |      |
      //              OrgBB2   |
      //                |  \   |
      //              0 |  1\  |
      //                |    \ |
      //               ...   CancelExitBB
      //

      if (!DT->getNode(CurrentCancelExitBB))
        // It is spossible for the user program to have code like:
        //   #pragma omp parallel for
        //   while (true) {
        //     #pragma omp cancel if (...)
        //   }
        //
        // In cases like this, ExitBB for the WRegion for the parallel-for is
        // unreachable (as there is an infinite loop before it). So when we try
        // to make it reachable by adding a branch from the cancellation check,
        // DT won't be able to handle the update. So, we need to re-build it
        // from scratch.
        DTNeedsRecalculation = true;
      else {
        auto *CancelExitBBDominator =
            DT->findNearestCommonDominator(CurrentCancelExitBB, OrgBB);
        DT->changeImmediateDominator(CurrentCancelExitBB,
                                     CancelExitBBDominator);
      }
    }

    if (NeedStaticFiniCall && !CancelExitBBWithStaticFini) {
      // If we need a `__kmpc_static_fini` call before CancelExitBB, we create a
      // separate BBlock with the call. This happens only when handling the
      // first CancellationPoint. We use the new CancelExitBBWithStaticFini in
      // place of CancelExitBB as the target of `cancel.check` branches for
      // subsequent CancellationPoints.
      //
      //               OrgBB
      //               0 |   | 1
      //                 |   +----------------------+
      //                 |                          |
      //                ...            +--CancelExitBBWithStaticFini--+
      //           NotCancelledBB      |   __kmpc_static_fini(...)    |
      //                ...            +------------------------------+
      //                 |                          |
      //                 |   +----------------------+
      //           CancelExitBB
      //
      CancelExitBBWithStaticFini = SplitEdge(OrgBB, CancelExitBB, DT, LI);
      auto *InsertPt = CancelExitBBWithStaticFini->getTerminator();

      LoadInst *LoadTid = new LoadInst(TidPtrHolder, "my.tid", InsertPt);
      LoadTid->setAlignment(4);
      VPOParoptUtils::genKmpcStaticFini(W, IdentTy, LoadTid, InsertPt);

      CancelExitBB = CancelExitBBWithStaticFini;

      LLVM_DEBUG(
          dbgs() << "genCancellationBranchingCode: Created predecessor of "
                    "CancelExitBB: [";
          CancelExitBBWithStaticFini->printAsOperand(dbgs());
          dbgs() << "] containing '__kmpc_static_fini' call.\n");
    }

    if (NeedCancelBarrierForNonBarriers && !CancelExitBBForNonBarriers &&
        !CancellationPointIsBarrier) {

      // If we need a `__kmpc_cancel_barrier` call for branches to CancelExitBB
      // from __kmpc_cancel and __kmpc_cancellationpoint calls, we create a
      // separate BBlock with the call. This happens only when handling the
      // first non-barrier CancellationPoint. We use the new
      // CancelExitBBForNonBarriers in place of CancelExitBB as the target of
      // `cancel.check` branches for subsequent non-barrier CancellationPoints.
      //
      //                                            %2 = kmpc_cancellationpoint
      //                           %1 = kmpc_cancel            /1
      //                                      |1              /
      //    %3 = kmpc_cancel_barrier          |              /
      //         |   \                        |             /
      //        0|  1 \              +-CancelExitBBForNonBarriers-+
      //        ...    \             |  %1 =  kmpc_cancel_barrier |
      //                \            +/---------------------------+
      //                 \           /
      //                  \         /
      //                   \       /
      //                    \     /
      //                 CancelExitBB
      //
      CancelExitBBForNonBarriers = SplitEdge(OrgBB, CancelExitBB, DT, LI);
      auto *InsertPt = CancelExitBBForNonBarriers->getTerminator();

      VPOParoptUtils::genKmpcBarrierImpl(W, TidPtrHolder, InsertPt, IdentTy,
                                         false /*not explicit*/,
                                         true /*cancel barrrier*/);

      LLVM_DEBUG(dbgs() << "genCancellationBranchingCode: Created BB for "
                           "non-barrier cancellation points: [";
                 CancelExitBBForNonBarriers->printAsOperand(dbgs());
                 dbgs() << "] containing '__kmpc_cancel_barrier' call.\n");
    } // if
  } // for

  LLVM_DEBUG(
      dbgs() << "\nExit VPOParoptTransform::genCancellationBranchingCode\n");

  W->resetBBSet(); // CFG changed; clear BBSet
  if (DTNeedsRecalculation) {
    DT->recalculate(*F);
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Recalculating DT.\n");
  }
  return true;
}

// Propagate NormUBs of sections to parent constructs to be outlined
bool VPOParoptTransform::addNormUBsToParents(WRegionNode* W) {

  WRegionNode *P = W;

  assert(P && "Null WRegionNode.");

  if (!isa<WRNParallelSectionsNode>(P) && !isa<WRNSectionsNode>(P))
    return false;

  auto &LI = P->getWRNLoopInfo();

  if (LI.getNormUBSize() <= 0)
    return false;

  SmallVector <Value*,2> NormUBs;

  for (unsigned I = 0; I < LI.getNormUBSize(); ++I)
    NormUBs.push_back(LI.getNormUB(I));

  bool Changed = false;
  while ((P = P->getParent())) {
    if (isa<WRNParallelNode>(P)) {
      SharedClause &ShrClause = P->getShared();

      for (Value *V: NormUBs)
        WRegionUtils::addToClause(ShrClause, V);

      StringRef ClauseString =
          VPOAnalysisUtils::getClauseString(QUAL_OMP_SHARED);

      CallInst *CI = cast<CallInst>(P->getEntryDirective());
      CI = VPOParoptUtils::addOperandBundlesInCall(CI,
                                                   {{ClauseString, {NormUBs}}});
      P->setEntryDirective(CI);
      Changed = true;

    } else if (isa<WRNTargetNode>(P)) {
      MapClause &MpClause = P->getMap();

      for (Value *V: NormUBs)
        WRegionUtils::addToClause(MpClause, V);

      MpClause.back()->setIsMapTo();
      StringRef ClauseString =
          VPOAnalysisUtils::getClauseString(QUAL_OMP_MAP_TO);

      CallInst *CI = cast<CallInst>(P->getEntryDirective());
      CI = VPOParoptUtils::addOperandBundlesInCall(CI,
                                                   {{ClauseString, {NormUBs}}});
      P->setEntryDirective(CI);
      Changed = true;
    }
  }

  return Changed;
}

// Set the values in the private clause to be empty.
void VPOParoptTransform::resetValueInPrivateClause(WRegionNode *W) {

  if (!W->canHavePrivate())
    return;

  PrivateClause &PrivClause = W->getPriv();

  if (PrivClause.empty())
    return;

  for (auto *I : PrivClause.items()) {
    resetValueInOmpClauseGeneric(W, I->getOrig());
  }
}

// Set the the operands V of OpenMP clauses in W to be empty.
void VPOParoptTransform::resetValueInOmpClauseGeneric(WRegionNode *W,
                                                        Value *V) {
  if (!V)
    return;

  // The WRegionNode::contains() method requires BBSet to be computed.
  W->populateBBSet();

  SmallVector<Instruction *, 8> IfUses;
  for (auto IB = V->user_begin(), IE = V->user_end(); IB != IE; ++IB) {
    if (Instruction *User = dyn_cast<Instruction>(*IB))
      if (W->contains(User->getParent()))
        IfUses.push_back(User);
  }

  while (!IfUses.empty()) {
    Instruction *UI = IfUses.pop_back_val();
    if (VPOAnalysisUtils::isOpenMPDirective(UI)) {
      LLVMContext &C = F->getContext();
      UI->replaceUsesOfWith(V, ConstantPointerNull::get(Type::getInt8PtrTy(C)));
      break;
    }
  }

  // This routine doesn't change the CFG so no need to invalidate the BBSet
  // before returning.
}

// Generate the copyprivate code. Here is one example.
// #pragma omp single copyprivate ( a,b )
// LLVM IR output:
//     %copyprivate.agg.5 = alloca %__struct.kmp_copy_privates_t, align 8
//     %14 = bitcast %__struct.kmp_copy_privates_t* %copyprivate.agg.5 to i8**
//     store i8* %.0, i8** %14, align 8
//     %15 = getelementptr inbounds %__struct.kmp_copy_privates_t,
//           %__struct.kmp_copy_privates_t* %copyprivate.agg.5, i64 0, i32 1
//     store float* %b.fpriv, float** %15, align 8
//     %16 = load i32, i32* %tid, align 4
//     %17 = bitcast %__struct.kmp_copy_privates_t* %copyprivate.agg.5 to i8*
//     call void @__kmpc_copyprivate({ i32, i32, i32, i32, i8* }*
//          nonnull @.kmpc_loc.0.0.16, i32 %16, i32 16, i8* nonnull %17,
//          i8* bitcast (void (%__struct.kmp_copy_privates_t*,
//          %__struct.kmp_copy_privates_t*)* @test_copy_priv_5 to i8*), i32 %13)
//          #11
//
bool VPOParoptTransform::genCopyPrivateCode(WRegionNode *W,
                                            AllocaInst *IsSingleThread) {
  CopyprivateClause &CprivClause = W->getCpriv();
  if (CprivClause.empty())
    return false;

  W->populateBBSet();
  Instruction *InsertPt = W->getExitBBlock()->getTerminator();
  IRBuilder<> Builder(InsertPt);

  SmallVector<Type *, 4> KmpCopyPrivatesVars;
  for (CopyprivateItem *CprivI : CprivClause.items()) {
    Value *Orig = CprivI->getOrig();
    KmpCopyPrivatesVars.push_back(Orig->getType());
  }

  LLVMContext &C = F->getContext();
  StructType *KmpCopyPrivateTy = StructType::create(
      C, makeArrayRef(KmpCopyPrivatesVars.begin(), KmpCopyPrivatesVars.end()),
      "__struct.kmp_copy_privates_t", false);

  AllocaInst *CopyPrivateBase = Builder.CreateAlloca(
      KmpCopyPrivateTy, nullptr, "copyprivate.agg." + Twine(W->getNumber()));
  SmallVector<Value *, 4> Indices;

  unsigned cnt = 0;
  for (CopyprivateItem *CprivI : CprivClause.items()) {
    Indices.clear();
    Indices.push_back(Builder.getInt32(0));
    Indices.push_back(Builder.getInt32(cnt++));
    Value *Gep =
        Builder.CreateInBoundsGEP(KmpCopyPrivateTy, CopyPrivateBase, Indices);
    Builder.CreateStore(CprivI->getOrig(), Gep);
  }

  Function *FnCopyPriv = genCopyPrivateFunc(W, KmpCopyPrivateTy);
  assert(isa<PointerType>(CopyPrivateBase->getType()) &&
           "genCopyPrivateCode: Expect non-empty pointer type");
  PointerType *PtrTy = cast<PointerType>(CopyPrivateBase->getType());
  const DataLayout &DL = F->getParent()->getDataLayout();
  uint64_t Size = DL.getTypeAllocSize(PtrTy->getElementType());
  VPOParoptUtils::genKmpcCopyPrivate(
      W, IdentTy, TidPtrHolder, Size, CopyPrivateBase, FnCopyPriv,
      Builder.CreateLoad(IsSingleThread), InsertPt);

  W->resetBBSet(); // CFG changed; clear BBSet
  return true;
}
// Generate the helper function for copying the copyprivate data.
// TODO: nonPOD support
Function *VPOParoptTransform::genCopyPrivateFunc(WRegionNode *W,
                                                 StructType *KmpCopyPrivateTy) {
  LLVMContext &C = F->getContext();
  Module *M = F->getParent();

  Type *CopyPrivParams[] = {PointerType::getUnqual(KmpCopyPrivateTy),
                            PointerType::getUnqual(KmpCopyPrivateTy)};
  FunctionType *CopyPrivFnTy =
      FunctionType::get(Type::getVoidTy(C), CopyPrivParams, false);

  Function *FnCopyPriv =
      Function::Create(CopyPrivFnTy, GlobalValue::InternalLinkage,
                       F->getName() + "_copy_priv_" + Twine(W->getNumber()), M);
  FnCopyPriv->setCallingConv(CallingConv::C);

  auto I = FnCopyPriv->arg_begin();
  Value *DstArg = &*I;
  I++;
  Value *SrcArg = &*I;

  BasicBlock *EntryBB = BasicBlock::Create(C, "entry", FnCopyPriv);

  DominatorTree DT;
  DT.recalculate(*FnCopyPriv);

  IRBuilder<> Builder(EntryBB);
  Builder.CreateRetVoid();

  Builder.SetInsertPoint(EntryBB->getTerminator());

  unsigned cnt = 0;
  CopyprivateClause &CprivClause = W->getCpriv();
  SmallVector<Value *, 4> Indices;
  for (CopyprivateItem *CprivI : CprivClause.items()) {
    StringRef OrigName = CprivI->getOrig()->getName();
    Indices.clear();
    Indices.push_back(Builder.getInt32(0));
    Indices.push_back(Builder.getInt32(cnt++));
    Value *SrcGep = Builder.CreateInBoundsGEP(KmpCopyPrivateTy, SrcArg, Indices,
                                              OrigName + ".src.gep");
    Value *DstGep = Builder.CreateInBoundsGEP(KmpCopyPrivateTy, DstArg, Indices,
                                              OrigName + ".dst.gep");
    LoadInst *SrcLoad = Builder.CreateLoad(SrcGep, OrigName + ".src");
    LoadInst *DstLoad = Builder.CreateLoad(DstGep, OrigName + ".dst");
    Value *NewCopyPrivInst =
        genPrivatizationAlloca(CprivI, EntryBB->getTerminator(), ".cp.priv");
    genLprivFini(NewCopyPrivInst, DstLoad, EntryBB->getTerminator());
    NewCopyPrivInst->replaceAllUsesWith(SrcLoad);
    cast<AllocaInst>(NewCopyPrivInst)->eraseFromParent();
  }

  return FnCopyPriv;
}
#if INTEL_CUSTOMIZATION

// Add alias_scope and no_alias metadata to improve the alias
// results in the outlined function.
void VPOParoptTransform::improveAliasForOutlinedFunc(WRegionNode *W) {
  if (OptLevel < 2)
    return;
  W->populateBBSet();
  VPOUtils::genAliasSet(makeArrayRef(W->bbset_begin(), W->bbset_end()), AA,
                        &(F->getParent()->getDataLayout()));
}
#endif  // INTEL_CUSTOMIZATION

template <typename Range>
static bool removeFirstFence(Range &&R, AtomicOrdering AO) {
  for (auto &I : R)
    if (auto *Fence = dyn_cast<FenceInst>(&I)) {
      if (Fence->getOrdering() == AO) {
        Fence->eraseFromParent();
        return true;
      }
      break;
    }
  return false;
}

bool VPOParoptTransform::removeCompilerGeneratedFences(WRegionNode *W) {
  bool Changed = false;
  switch (W->getWRegionKindID()) {
  case WRegionNode::WRNAtomic:
  case WRegionNode::WRNCritical:
  case WRegionNode::WRNMaster:
  case WRegionNode::WRNSingle:
    if (auto *BB = W->getEntryBBlock()->getSingleSuccessor())
      Changed |= removeFirstFence(make_range(BB->begin(), BB->end()),
                                  AtomicOrdering::Acquire);
    if (auto *BB = W->getExitBBlock()->getSinglePredecessor())
      Changed |= removeFirstFence(make_range(BB->rbegin(), BB->rend()),
                                  AtomicOrdering::Release);
    break;
  case WRegionNode::WRNBarrier:
  case WRegionNode::WRNTaskwait:
    if (auto *BB = W->getEntryBBlock()->getSingleSuccessor())
      Changed |= removeFirstFence(make_range(BB->begin(), BB->end()),
                                  AtomicOrdering::AcquireRelease);
    break;
  default:
    llvm_unreachable("unexpected work region kind");
  }
  W->resetBBSetIfChanged(Changed); // Clear BBSet if transformed
  return Changed;
}

BasicBlock *VPOParoptTransform::getLoopExitBB(WRegionNode *W, unsigned Idx) {
  assert(W->getIsOmpLoop() && "getLoopExitBB: not a loop-type region.");
  auto *L = W->getWRNLoopInfo().getLoop(Idx);
  assert(L && "getLoopExitBB: failed to find Loop.");
  auto *LoopExitBB = WRegionUtils::getOmpExitBlock(L);
  assert(LoopExitBB && "getLoopExitBB: failed to find the loop's exit block.");
  return LoopExitBB;
}

// Initial Implementation for loop construct in OpenMP 5.0
//   The loop construct will be mapped to underlying loop scheme according to
//   binding rules and parent region/directive. See details in mapLoopScheme
//   function.
bool VPOParoptTransform::replaceGenericLoop(WRegionNode *W) {
  WRNGenericLoopNode *WL = cast<WRNGenericLoopNode>(W);

  // Map loop construct to underlying loop scheme
  bool Changed = WL->mapLoopScheme();
  assert(Changed &&
         "Loop directive must be mapped to right parallization scheme.");

  // replace entry directive with the mapped directive
  StringRef MappedEntryDir =
      VPOAnalysisUtils::getDirectiveString(WL->getMappedDir());
  LLVM_DEBUG(dbgs() << "Entry Directive: " << W->getEntryDirective()
                    << " maps to Directive: " << MappedEntryDir << "\n");

  CallInst *EntryCI = cast<CallInst>(W->getEntryDirective());

  SmallVector<OperandBundleDef, 8> OpBundles;
  EntryCI->getOperandBundlesAsDefs(OpBundles);
  for (unsigned i = 0; i < OpBundles.size(); i++) {
    EntryCI = VPOParoptUtils::removeOperandBundlesFromCall(
        EntryCI, OpBundles[i].getTag());
  }
  SmallVector<std::pair<StringRef, ArrayRef<Value *>>, 8> OpBundlesToAdd;
  OpBundlesToAdd.emplace_back(MappedEntryDir, ArrayRef<Value *>{});
  for (unsigned i = 1; i < OpBundles.size(); i++) {
    // Skip bind clause since it's used for loop contruct
    if (VPOAnalysisUtils::isBindClause(OpBundles[i].getTag()))
      continue;
    OpBundlesToAdd.emplace_back(OpBundles[i].getTag(), OpBundles[i].inputs());
  }
  EntryCI = VPOParoptUtils::addOperandBundlesInCall(EntryCI, OpBundlesToAdd);
  W->setEntryDirective(EntryCI);

  // replace exit directive accordingly
  CallInst *ExitCI =
      dyn_cast<CallInst>(VPOAnalysisUtils::getEndRegionDir(EntryCI));
  StringRef ExitDir = VPOAnalysisUtils::getDirectiveString(ExitCI);

  StringRef MappedExitDir = VPOAnalysisUtils::getDirectiveString(
      VPOAnalysisUtils::getMatchingEndDirective(WL->getMappedDir()));
  LLVM_DEBUG(dbgs() << "Exit Directive: " << ExitDir
                    << " maps to directive: " << MappedExitDir << "\n");

  ExitCI = VPOParoptUtils::removeOperandBundlesFromCall(ExitCI, {ExitDir});
  ExitCI = VPOParoptUtils::addOperandBundlesInCall(
      ExitCI, {{MappedExitDir, ArrayRef<Value *>{}}});

  return Changed;
}

// Targets that support non-default address spaces may have the following
// representation for global variables referenced in OpenMP clauses:
//   %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
//       "QUAL.OMP.MAP.TOFROM"([1000 x float] addrspace(4)*
//           addrspacecast ([1000 x float] addrspace(1)* @global_var
//               to [1000 x float] addrspace(4)*)) ]
//
// At the same time, references of the same global variable inside
// the region may look differently (e.g. due to constant folding):
//   %9 = getelementptr inbounds float,
//       float addrspace(4)* addrspacecast (
//           float addrspace(1)* getelementptr inbounds (
//               [1000 x float],
//               [1000 x float] addrspace(1)* @global_var,
//               i32 0, i32 0)
//       to float addrspace(4)*), i64 %8
//
// Different places in Paropt rely on finding the original (source code)
// references of the global variable by just looking for users
// of the clause item (which is addrspacecast ConstantExpr).
// In this example, the region refers @global_var via getelementptr
// ConstantExpr, which prevents correct handling of the clause item.
//
// This routine fixes up the clauses (in IR) and the corresponding
// parsed representation (Orig members of Item), so that
// a clause item used for finding the references is always
// the global variable itself, e.g. this routine will transform
// the region's entry call to the following:
//   %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
//       "QUAL.OMP.MAP.TOFROM"([1000 x float] addrspace(1)* @global_var) ]
bool VPOParoptTransform::canonicalizeGlobalVariableReferences(WRegionNode *W) {
  if (!isTargetSPIRV())
    return false;

  SmallPtrSet<Value *, 16> HandledASCasts;

  auto getGlobalVarItem = [](Value *Orig) {
    GlobalVariable *GVar = nullptr;

    if (GeneralUtils::isOMPItemGlobalVAR(Orig))
      // isOMPItemGlobalVAR() is an optionally addrspacecasted
      // GlobalVariable. The GlobalVariable must be in the global
      // address space.
      if (auto *CE = dyn_cast<ConstantExpr>(Orig))
        if (CE->getOpcode() == Instruction::AddrSpaceCast) {
          GVar = dyn_cast<GlobalVariable>(CE->getOperand(0));
          assert(GVar &&
                 GVar->getAddressSpace() == vpo::ADDRESS_SPACE_GLOBAL &&
                 "Unexpected form of global variable item.");
        }

    return GVar;
  };

  auto rename = [&getGlobalVarItem, &HandledASCasts](Item *I) {
    auto *Orig = I->getOrig();
    if (auto *GVar = getGlobalVarItem(Orig)) {
      HandledASCasts.insert(Orig);
      I->setOrig(GVar);
    }
  };

  // Look at the clauses and update Orig items, if they
  // refer an addrspacecast of a GlobalVariable.
  if (W->canHavePrivate())
    for (PrivateItem *PrivI : W->getPriv().items())
      rename(PrivI);

  if (W->canHaveFirstprivate())
    for (FirstprivateItem *FprivI : W->getFpriv().items())
      rename(FprivI);

  if (W->canHaveShared())
    for (SharedItem *ShaI : W->getShared().items())
      rename(ShaI);

  if (W->canHaveReduction())
    for (ReductionItem *RedI : W->getRed().items())
      rename(RedI);

  if (W->canHaveLastprivate())
    for (LastprivateItem *LprivI : W->getLpriv().items())
      rename(LprivI);

  if (W->canHaveLinear())
    for (LinearItem *LrI : W->getLinear().items())
      rename(LrI);

  if (W->canHaveIsDevicePtr())
    for (IsDevicePtrItem *DPtrI : W->getIsDevicePtr().items())
      rename(DPtrI);

  if (W->canHaveMap()) {
    for (MapItem *MapI : W->getMap().items()) {
      if (MapI->getIsMapChain()) {
        MapChainTy const &MapChain = MapI->getMapChain();
        for (unsigned I = 0, IE = MapChain.size(); I < IE; ++I) {
          MapAggrTy *Aggr = MapChain[I];
          // It is not strictly necessary to transform section pointer
          // and the base pointer, because they will not be used
          // for references replacements. At the same time,
          // we will fix all references of an addrspacecast
          // of a GlobalVariable in the IR, so we'd rather
          // have consistency between IR and the parsed representation.
          if (auto *GVar = getGlobalVarItem(Aggr->getSectionPtr())) {
            HandledASCasts.insert(Aggr->getSectionPtr());
            Aggr->setSectionPtr(GVar);
          }
          if (auto *GVar = getGlobalVarItem(Aggr->getBasePtr())) {
            HandledASCasts.insert(Aggr->getBasePtr());
            Aggr->setBasePtr(GVar);
          }
        }
      }

      rename(MapI);
    }
  }

  // Fixup the region's entry call in IR.
  CallInst *CI = cast<CallInst>(W->getEntryDirective());
  for (auto *ASCast : HandledASCasts) {
    auto *GVar = getGlobalVarItem(ASCast);
    assert(GVar && "Invalid element in HandledASCasts.");
    CI->replaceUsesOfWith(ASCast, GVar);
  }

  return !HandledASCasts.empty();
}

bool VPOParoptTransform::constructNDRangeInfo(WRegionNode *W) {
  if (VPOParoptUtils::getSPIRExecutionScheme() != spirv::ImplicitSIMDSPMDES)
    return false;

  if (!deviceTriplesHasSPIRV())
    return false;

  WRegionNode *WTarget =
      WRegionUtils::getParentRegion(W, WRegionNode::WRNTarget);

  if (!WTarget) {
    if (isTargetSPIRV())
      // Emit warning only during SPIR compilation.
      F->getContext().diagnose(ParoptDiagInfo(*F,
          W->getEntryDirective()->getDebugLoc(),
          Twine("'") + Twine(spirv::ExecutionSchemeOptionName) +
          Twine("' option ignored for OpenMP region, since ") +
          Twine("no enclosing 'target' region was found")));
    return false;
  }

  // It is not really necessary to generate ND-range parameter
  // during SPIR compilation, because it is only used in host code
  // for passing to libomptarget. At the same time, we have to
  // call setParLoopNdInfoAlloca() with some non-null value below,
  // so we just do it for both host and target compilations.
  auto *NDInfoAI = genTgtLoopParameter(WTarget, W);

  if (!NDInfoAI)
    return false;

  // NDInfoAI is not null here, so we've made some changed to IR
  // inside genTgtLoopParameter().

  // Check if there is an inclosing teams region with num_teams() clause.
  // num_teams() overrules ImplicitSIMDSPMDES mode.
  WRegionNode *WTeams = WRegionUtils::getParentRegion(W, WRegionNode::WRNTeams);

  if (WTeams && WTeams->getNumTeams()) {
    if (isTargetSPIRV())
      // Emit warning only during SPIR compilation.
      F->getContext().diagnose(ParoptDiagInfo(*F,
          W->getEntryDirective()->getDebugLoc(),
          Twine("'") + Twine(spirv::ExecutionSchemeOptionName) +
          Twine("' option ignored for OpenMP region, since ") +
          Twine("the enclosing teams region specifies num_teams.")));
    return true;
  }

  // Finally set ND-range info for the target region.
  WTarget->setParLoopNdInfoAlloca(NDInfoAI);
  return true;
}

#endif // INTEL_COLLAB
