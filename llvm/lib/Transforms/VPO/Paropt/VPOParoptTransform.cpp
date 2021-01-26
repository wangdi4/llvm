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

#include "llvm/Transforms/VPO/Paropt/VPOParoptModuleTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptAtomics.h"
#include "llvm/Transforms/VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/Debug.h"

#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DIBuilder.h"
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
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"

#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"

#include "llvm/Transforms/Utils/GeneralUtils.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"
#include "llvm/Transforms/Utils/LoopRotationUtils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#endif  // INTEL_CUSTOMIZATION

#include <algorithm>
#include <set>
#include <unordered_set>
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

static cl::opt<bool> OptimizeScalarFirstprivate(
    "vpo-paropt-opt-scalar-fp", cl::Hidden, cl::init(true),
    cl::desc("Pass scalar firstprivates as literals."));

static cl::opt<bool> EmitSPIRVBuiltins(
    "vpo-paropt-emit-spirv-builtins", cl::Hidden, cl::init(false),
    cl::desc("Emit SPIR-V builtin calls to compute loop bounds for SIMD."));

static cl::opt<bool> AllowDistributeDimension(
    "vpo-paropt-allow-distribute-dimension",
     cl::Hidden, cl::init(true),
     cl::desc("Allow using separate ND-range dimension "
              "for OpenMP distribute."));

// Due to implicit widening for SPIR targets, we want to schedule a loop
// such that adjacent WIs process adjacent iterations of the loop.
static cl::opt<bool> AvoidStridedProcessing(
    "vpo-paropt-avoid-strided-processing", cl::Hidden, cl::init(true),
    cl::desc("For SPIR targets schedule parallel loops such that adjacent "
             "threads execute adjacent iterations of the loop."));

static cl::opt<bool> UseFastReduction("vpo-paropt-fast-reduction", cl::Hidden,
                                      cl::init(true),
                                      cl::desc("Enable fast reduction."));
static cl::opt<bool> UseFastRedAtomic("vpo-paropt-fast-reduction-atomic",
                                      cl::Hidden, cl::init(true),
                                      cl::desc("Allow to use atomic reduction "
                                               "within fast reduction."));
static cl::opt<uint32_t> FastReductionCtrl("vpo-paropt-fast-reduction-ctrl",
                                           cl::Hidden, cl::init(0x3),
                                           cl::desc("Control option for fast "
                                                    "reduction. Bit 0(default "
                                                    "on): Scalar variables are"
                                                    " used directly in "
                                                    "reduction code. Bit 1("
                                                    "default on): similar with"
                                                    " bit 0, but for array "
                                                    "reduction."));

static cl::opt<bool> EmitTargetFPCtorDtors(
    "vpo-paropt-emit-target-fp-ctor-dtor", cl::Hidden, cl::init(false),
    cl::desc("Enable emission of constructors/destructors for firstprivate "
             "operands on target constructs, during target compilation."));

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

static void debugPrintHeader(WRegionNode *W, int Mode) {
  if (Mode & ParPrepare)
    LLVM_DEBUG(dbgs() << "\n\n === VPOParopt Prepare: ");
  else if (Mode & ParTrans)
    LLVM_DEBUG(dbgs() << "\n\n === VPOParopt Transform: ");
  else if (Mode & OmpNoFECollapse)
    LLVM_DEBUG(dbgs() << "\n\n === VPOParopt Loop Collapse: ");
  else
    LLVM_DEBUG(dbgs() << "\n\n === VPOParopt ???: ");

  LLVM_DEBUG(dbgs() << W->getName().upper() << " construct\n\n");
}

// Generate the placeholders for the loop lower bound and upper bound.
void VPOParoptTransform::genLoopBoundUpdatePrep(
    WRegionNode *W, unsigned Idx, IRBuilder<> &AllocaBuilder,
    AllocaInst *&LowerBnd, AllocaInst *&UpperBnd,
    AllocaInst *&SchedStride, AllocaInst *&TeamLowerBnd,
    AllocaInst *&TeamUpperBnd, AllocaInst *&TeamStride,
    Value *&IsLastLoc, Value *&UpperBndVal, bool ChunkForTeams) {
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

  LowerBnd = AllocaBuilder.CreateAlloca(IndValTy, nullptr,
                                        "loop" + Twine(Idx) + ".lower.bnd");

  UpperBnd = AllocaBuilder.CreateAlloca(IndValTy, nullptr,
                                        "loop" + Twine(Idx) + ".upper.bnd");

  SchedStride = AllocaBuilder.CreateAlloca(IndValTy, nullptr,
                                           "loop" + Twine(Idx) + ".sched.inc");

  IsLastLoc = AllocaBuilder.CreateAlloca(AllocaBuilder.getInt32Ty(), nullptr,
                                         "loop" + Twine(Idx) + ".is.last");
  // Initialize with 'false' at the allocation.
  // If a WI does not execute an iteration of the inner loop,
  // e.g. because of ZTT for the outer loop, the value still
  // has to be initialized, since we read it outside of the
  // outermost loop.
  AllocaBuilder.CreateStore(AllocaBuilder.getInt32(0), IsLastLoc);

  if (ChunkForTeams) {
    TeamLowerBnd = AllocaBuilder.CreateAlloca(IndValTy, nullptr,
                                              "loop" + Twine(Idx) + ".team.lb");

    TeamUpperBnd = AllocaBuilder.CreateAlloca(IndValTy, nullptr,
                                              "loop" + Twine(Idx) + ".team.ub");

    TeamStride = AllocaBuilder.CreateAlloca(IndValTy, nullptr,
                                            "loop" + Twine(Idx) + ".team.inc");
  }

  InitVal = Builder.CreateSExtOrTrunc(InitVal, IndValTy);
  Builder.CreateStore(InitVal, LowerBnd);

  UpperBndVal =
      VPOParoptUtils::computeOmpUpperBound(W, Idx, InsertPt,
                                           ".loop" + Twine(Idx) + ".orig.ub");
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
WRNScheduleKind VPOParoptTransform::getSchedKindForMultiLevelLoops(
    WRegionNode *W, WRNScheduleKind SchedKind,
    WRNScheduleKind TargetScheduleKind) {
  if (W->getWRNLoopInfo().getNormIVSize() <= 1)
    return SchedKind;

  if (SchedKind != TargetScheduleKind)
    LLVM_DEBUG(dbgs() <<
               "SchedKind is forced to be " << TargetScheduleKind << "\n");

  return TargetScheduleKind;
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
    WRNScheduleKind DistSchedKind, Instruction *&TeamLB, Instruction *&TeamUB,
    Instruction *&TeamST) {
  assert(!TeamLowerBnd == !TeamUpperBnd &&
         !TeamUpperBnd == !TeamStride &&
         "genOCLDistParLoopBoundUpdateCode: team's lower/upper bounds "
         "and stride must be created together.");
  assert(W->getIsOmpLoop() &&
         "genOCLDistParLoopBoundUpdateCode: W is not a loop-type WRN");
  Loop *L = W->getWRNLoopInfo().getLoop(Idx);
  assert(L && "genOCLDistParLoopBoundUpdateCode: Expect non-empty loop.");
  // Insert calls outside of the outer loop to enable more loop
  // optimizations, e.g. invariant hoisting.
  Loop *OuterLoop = W->getWRNLoopInfo().getLoop(0);
  assert(OuterLoop && "genOCLDistParLoopBoundUpdateCode: no outer loop found.");
  Instruction *CallsInsertPt =
      cast<Instruction>(OuterLoop->getLoopPreheader()->getTerminator());
  Instruction *InsertPt =
      cast<Instruction>(L->getLoopPreheader()->getTerminator());
  IRBuilder<> Builder(InsertPt);

  // Call the function get_num_group to get the group size for dimension Idx.
  SmallVector<Value *, 3> Arg;
  unsigned NumLoops = W->getWRNLoopInfo().getNormIVSize();
  assert(NumLoops != 0 && "Zero loops in loop construct.");
  assert(NumLoops <= 3 && "Max 3 loops in a loop nest for SPIR-V targets.");
  assert(Idx < NumLoops && "Loop index is bigger than the number of loops "
         "in a loop nest.");
  unsigned DimNum = NumLoops - Idx - 1;
  DimNum += W->getWRNLoopInfo().getNDRangeStartDim();
  initArgArray(&Arg, DimNum);

  // get_num_groups() returns a value of type size_t by specification,
  // and the return value is greater than 0.
  CallInst *NumGroupsCall;
  if (EmitSPIRVBuiltins) {
    std::string fname;
    SmallVector<Value *, 1> Arg;
    switch (Idx) {
    case 0: fname = "_Z29__spirv_NumWorkgroups_xv";
      break;
    case 1: fname = "_Z29__spirv_NumWorkgroups_yv";
      break;
    case 2: fname = "_Z29__spirv_NumWorkgroups_zv";
      break;
    default:
      llvm_unreachable("Invalid dimentional index ");
    }
    NumGroupsCall = VPOParoptUtils::genOCLGenericCall(fname,
      GeneralUtils::getSizeTTy(F), Arg, CallsInsertPt);
  } else {
    NumGroupsCall = VPOParoptUtils::genOCLGenericCall("_Z14get_num_groupsj",
      GeneralUtils::getSizeTTy(F), Arg, CallsInsertPt);
  }

  Value *LB = Builder.CreateLoad(LowerBnd->getAllocatedType(), LowerBnd);
  Value *UB = Builder.CreateLoad(UpperBnd->getAllocatedType(), UpperBnd);
  // LB and UB have the type of the canonical induction variable.
  auto *ItSpaceType = LB->getType();
  // The subtraction cannot overflow, because the normalized lower bound
  // is a non-negative value.
  Value *ItSpace = Builder.CreateSub(UB, LB);

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

  if (TeamStride) {
    // FIXME: this multiplication may actually overflow,
    //        if big chunk size is specified in the schedule() clause.
    Value *TeamStrideVal = Builder.CreateMul(NumGroups, Chunk);
    Builder.CreateStore(TeamStrideVal, TeamStride);
  }

  // get_group_id returns a value of type size_t by specification,
  // and the return value is non-negative.
  CallInst *GroupIdCall;
  if (EmitSPIRVBuiltins) {
    std::string fname;
    SmallVector<Value *, 1> Arg;
    switch (Idx) {
    case 0: fname = "_Z21__spirv_WorkgroupId_xv";
      break;
    case 1: fname = "_Z21__spirv_WorkgroupId_yv";
      break;
    case 2: fname = "_Z21__spirv_WorkgroupId_zv";
      break;
    default:
        llvm_unreachable("Invalid dimentional index ");
    }
    GroupIdCall = VPOParoptUtils::genOCLGenericCall(fname,
      GeneralUtils::getSizeTTy(F), Arg, CallsInsertPt);
  } else {
    GroupIdCall = VPOParoptUtils::genOCLGenericCall("_Z12get_group_idj",
      GeneralUtils::getSizeTTy(F), Arg, CallsInsertPt);
  }

  Value *GroupId = Builder.CreateZExtOrTrunc(GroupIdCall, ItSpaceType);

  // Compute new_team_lb
  // FIXME: this multiplication may actually overflow,
  //        if big chunk size is specified in the schedule() clause.
  Value *LBDiff = Builder.CreateMul(GroupId, Chunk);
  LB = Builder.CreateAdd(LB, LBDiff);
  Builder.CreateStore(LB, LowerBnd);
  if (TeamLowerBnd) {
    Builder.CreateStore(LB, TeamLowerBnd);
    Builder.CreateStore(UB, TeamUpperBnd);
  }

  // Compute new_team_ub
  ConstantInt *ValueOne = ConstantInt::get(cast<IntegerType>(ItSpaceType), 1);
  Value *Ch = Builder.CreateSub(Chunk, ValueOne);
  Value *NewUB = Builder.CreateAdd(LB, Ch);

  // Compare bounds using signed/unsigned comparison based on the ZTT compare.
  // This helps optimizing CFG after Paropt. If ZTT is not found, then
  // use unsigned comparison.
  ICmpInst *ZTTCmpInst =
      WRegionUtils::getOmpLoopZeroTripTest(L, W->getEntryBBlock());
  bool IsSignedZTT = ZTTCmpInst ? ZTTCmpInst->isSigned() : false;

  Value *Compare = Builder.CreateICmp(
      IsSignedZTT ? ICmpInst::ICMP_SLT : ICmpInst::ICMP_ULT, NewUB, UB);
  Instruction *ThenTerm = SplitBlockAndInsertIfThen(
      Compare, InsertPt, false,
      MDBuilder(F->getContext()).createBranchWeights(99999, 100000), DT, LI);
  BasicBlock *ThenBB = ThenTerm->getParent();
  ThenBB->setName("then.bb.");
  IRBuilder<> BuilderThen(ThenBB);
  BuilderThen.SetInsertPoint(ThenBB->getTerminator());
  BuilderThen.CreateStore(NewUB, UpperBnd);

  if (TeamLowerBnd) {
    assert(!TeamLowerBnd == !TeamUpperBnd &&
           "genOCLDistParLoopBoundUpdateCode: team lower/upper bounds "
           "must be created together.");

    BuilderThen.CreateStore(NewUB, TeamUpperBnd);

    Builder.SetInsertPoint(InsertPt);
    TeamLB = Builder.CreateLoad(TeamLowerBnd->getAllocatedType(), TeamLowerBnd);
    TeamUB = Builder.CreateLoad(TeamUpperBnd->getAllocatedType(), TeamUpperBnd);
    TeamST = Builder.CreateLoad(TeamStride->getAllocatedType(), TeamStride);
    // Team distribute loop may insert minimum upper bound computation
    // between the above loads and the below stores, and update
    // TeamUpperBnd variable, so we need to reload the team bounds again
    // before initializing the loop bounds.
    Builder.CreateStore(Builder.CreateLoad(
        TeamLowerBnd->getAllocatedType(), TeamLowerBnd), LowerBnd);
    Builder.CreateStore(Builder.CreateLoad(
        TeamUpperBnd->getAllocatedType(), TeamUpperBnd), UpperBnd);
  }
}

// Generate the OCL loop bound update code.
//
// #pragma omp target teams distribute
// for (i = lb; i <= ub; i++)
// The output of loop partitioning for the Idxth dimension.
// chunk_size = (ub - lb + get_local_size(Idx)) /
//              get_local_size(Idx);
// new_lb = lb + get_local_id(Idx) * chunk_size;
// new_ub = min(new_lb + chunk_size - 1, ub);
//
// for (i = new_lb; i <= new_ub; i++)
//
// Idx -- the dimension index.
// LowerBnd -- the stack variable which holds the loop lower bound.
// UpperBnd -- the stack variable which holds the loop upper bound.
//
// Note that LowerBnd and UpperBnd values may not match the original
// loop's bounds. Depending on the number of teams running the loop
// these values are already adjusted to specify the team bounds
// somewhere before the end of the loop pre-header.
void VPOParoptTransform::genOCLLoopBoundUpdateCode(WRegionNode *W, unsigned Idx,
                                                   AllocaInst *LowerBnd,
                                                   AllocaInst *UpperBnd,
                                                   AllocaInst *&SchedStride) {
  assert(W->getIsOmpLoop() && "genOCLLoopBoundUpdateCode: W is not a loop-type WRN");
  assert (LowerBnd->getType() == UpperBnd->getType() &&
          "Expected LowerBnd and UpperBnd to be of same type");
  assert (LowerBnd->getType() == SchedStride->getType() &&
          "Expected LowerBnd and SchedStride to be of same type");

  unsigned NumLoops = W->getWRNLoopInfo().getNormIVSize();

  assert(NumLoops != 0 && "Zero loops in loop construct.");
  assert(NumLoops <= 3 && "Max 3 loops in a loop nest for SPIR-V targets.");
  assert(Idx < NumLoops && "Loop index is bigger than the number of loops "
         "in a loop nest.");

  Loop *L = W->getWRNLoopInfo().getLoop(Idx);
  assert(L && "genOCLLoopBoundUpdateCode: Expect non-empty loop.");
  // Insert calls outside of the outer loop to enable more loop
  // optimizations, e.g. invariant hoisting.
  Loop *OuterLoop = W->getWRNLoopInfo().getLoop(0);
  assert(OuterLoop && "genOCLLoopBoundUpdateCode: no outer loop found.");
  Instruction *CallsInsertPt =
      cast<Instruction>(OuterLoop->getLoopPreheader()->getTerminator());
  Instruction *InsertPt =
      cast<Instruction>(L->getLoopPreheader()->getTerminator());
  IRBuilder<> Builder(InsertPt);
  SmallVector<Value *, 3> Arg;
  // Map the innermost loop to the 1st ND-range dimension.
  unsigned DimNum = NumLoops - Idx - 1;
  DimNum += W->getWRNLoopInfo().getNDRangeStartDim();
  initArgArray(&Arg, DimNum);

  Value *LB = Builder.CreateLoad(LowerBnd->getAllocatedType(), LowerBnd);
  Value *UB = Builder.CreateLoad(UpperBnd->getAllocatedType(), UpperBnd);

  WRNScheduleKind SchedKind = getSchedKindForMultiLevelLoops(
      W, VPOParoptUtils::getLoopScheduleKind(W), WRNScheduleStaticEven);

  // There are two ways to process iterations of the chunk
  // provided by the teams distribution for static even scheduling:
  //   Strided:
  //     chunk_size =
  //         (ub - lb + get_local_size(Idx)) /
  //         get_local_size(Idx);
  //     new_lb = lb + get_local_id(Idx) * chunk_size;
  //     new_ub = min(new_lb + chunk_size - 1, ub);
  //     for (i = new_lb; i <= new_ub; i++)
  //
  //   Non-strided:
  //     new_lb = lb + get_local_id(Idx);
  //     new_ub = ub;
  //     for (i = new_lb; i <= new_ub; i += get_local_size(Idx))
  //
  // Due to implicit widening (combining adjacent WIs into a SIMD program),
  // in Strided case, vector representing induction variable 'i' will
  // contain values different from each other by stride equal to
  // chunk_size. This may result in inefficient strided memory accesses,
  // e.g. if the original code accesses some data as data[i].
  // In Non-strided case, the vector for 'i' will contain
  // consecutive values of the induction variable providing
  // better memory access pattern. Moreover, we are avoiding quite
  // a few arithmetic operations and a min() computation with
  // Non-strided scheduling.
  //
  // To be on the safe side, we only use Non-strided scheduling,
  // if the iteration processing pattern matches the canonical one,
  // i.e. the induction variable is incremented by 1 on each iteration.
  //
  // Non-strided scheduling is actually similar to schedule(static, 1),
  // except that for the latter the generated IR looks more complex:
  //   new_lb = lb + get_local_id(Idx);
  //   new_ub = min(new_lb, ub);
  //   while (new_lb <= new_ub) {
  //     new_ub = min(new_ub, ub);
  //     if (new_lb > new_ub)
  //       break;
  //     for (i = new_lb; i <= new_ub; i++) { ... }
  //     new_lb += get_local_size(Idx);
  //     new_ub += get_local_size(Idx);
  //   }
  //
  // It is much harder to optimize this IR, otherwise, we would
  // just use schedule(static, 1) as the default schedule under
  // AvoidStridedProcessing.
  //
  // Note that useSPMDMode() provides non-strided iterations processing
  // as well:
  //   new_lb = get_global_id(Idx);
  //   new_ub = min(new_lb, ub);
  //   for (i = new_lb; i <= new_ub; i++)
  //
  // For useSPMDMode() we do not encode the team distribution loop,
  // so ub is equal to the original normalized loop's upper bound.
  // If get_global_id(Idx) returns value outside of the loop's
  // iteration space, then the loop above will just not run,
  // otherwise, it will run exactly one iteration.
  bool IncrementByLocalSize = false;
  PHINode *PN = WRegionUtils::getOmpCanonicalInductionVariable(L);
  Instruction *IVInc =
      dyn_cast<Instruction>(PN->getIncomingValueForBlock(L->getLoopLatch()));
  uint32_t IVAddendOp = 0;
  if (AvoidStridedProcessing &&
      !VPOParoptUtils::useSPMDMode(W) &&
      // Do this only for schedule(static) for the time being.
      SchedKind == WRNScheduleStaticEven &&
      IVInc && IVInc->getOpcode() == Instruction::Add) {
    if (IVInc->getOperand(0) == PN)
      IVAddendOp = 1;

    auto *AddendConst = dyn_cast<ConstantInt>(IVInc->getOperand(IVAddendOp));
    if (AddendConst && AddendConst->isOneValue())
      // Make sure that the canonical induction variable is incremented
      // by 1 each iteration.
      IncrementByLocalSize = true;
  }

  // All operands of math expressions below will be of LBType
  auto LBType = LB->getType();
  Value *NewUB = nullptr;

  if (!VPOParoptUtils::useSPMDMode(W)) {
    Value *Chunk = nullptr;

    CallInst *LocalSize;
    if (EmitSPIRVBuiltins) {
      std::string fname;
      SmallVector<Value *, 1> Arg;
      switch (Idx) {
      case 0: fname = "_Z22__spirv_WorkgroupSize_xv";
        break;
      case 1: fname = "_Z22__spirv_WorkgroupSize_yv";
        break;
      case 2: fname = "_Z22__spirv_WorkgroupSize_zv";
        break;
      default:
        llvm_unreachable("Invalid dimentional index ");
      }
      LocalSize = VPOParoptUtils::genOCLGenericCall(fname,
                         GeneralUtils::getSizeTTy(F), Arg, CallsInsertPt);
    } else {
      LocalSize = VPOParoptUtils::genOCLGenericCall("_Z14get_local_sizej",
        GeneralUtils::getSizeTTy(F), Arg, CallsInsertPt);
    }

    Value *NumThreads = Builder.CreateSExtOrTrunc(LocalSize, LBType);
    if (SchedKind == WRNScheduleStaticEven) {
      if (!IncrementByLocalSize) {
        // The chunk size is not used for Non-strided scheduling.
        // Otherwise, it is equal to:
        //   chunk_size =
        //       (ub - lb + get_local_size(Idx)) /
        //       get_local_size(Idx);
        Value *ItSpace = Builder.CreateSub(UB, LB);
        Value *ItSpaceRounded = Builder.CreateAdd(ItSpace, NumThreads);
        Chunk = Builder.CreateSDiv(ItSpaceRounded, NumThreads);
      }
    } else if (SchedKind == WRNScheduleStatic) {
      Chunk = W->getSchedule().getChunkExpr();
      Chunk = Builder.CreateSExtOrTrunc(Chunk, LBType);
    } else
      llvm_unreachable(
          "Unsupported loop schedule type in OpenCL based offloading!");

    if (IncrementByLocalSize)
      // Static even scheduling does not create a scheduling loop
      // for chunks processing, so the scheduling stride is not needed.
      // Reset it to nullptr here to make sure all invalid users
      // of the scheduling stride fail right on its use.
      SchedStride = nullptr;
    else {
      Value *SchedStrideVal = Builder.CreateMul(NumThreads, Chunk);
      Builder.CreateStore(SchedStrideVal, SchedStride);
    }

    CallInst *LocalId;
    if (EmitSPIRVBuiltins) {
      std::string fname;
      SmallVector<Value *, 1> Arg;
      switch (Idx) {
        case 0: fname = "_Z27__spirv_LocalInvocationId_xv";
          break;
        case 1: fname = "_Z27__spirv_LocalInvocationId_yv";
          break;
        case 2: fname = "_Z27__spirv_LocalInvocationId_zv";
          break;
        default:
          llvm_unreachable("Invalid dimentional index ");
      }
      LocalId = VPOParoptUtils::genOCLGenericCall(fname,
                   GeneralUtils::getSizeTTy(F), Arg, CallsInsertPt);
    }
    else
     LocalId = VPOParoptUtils::genOCLGenericCall("_Z12get_local_idj",
                  GeneralUtils::getSizeTTy(F), Arg, CallsInsertPt);

    Value *LocalIdCasted = Builder.CreateSExtOrTrunc(LocalId, LBType);

    Value *LBDiff = nullptr;
    if (IncrementByLocalSize)
      // new_lb = lb + get_local_id(Idx);
      LBDiff = LocalIdCasted;
    else
      // new_lb = lb + get_local_id(Idx) * chunk_size;
      LBDiff = Builder.CreateMul(LocalIdCasted, Chunk);
    LB = Builder.CreateAdd(LB, LBDiff);
    Builder.CreateStore(LB, LowerBnd);

    if (IncrementByLocalSize) {
      // new_ub = ub;
      NewUB = UB;

      auto *LocalSizeCasted = CastInst::CreateIntegerCast(
          LocalSize, IVInc->getType(), false, "", IVInc);
      IVInc->replaceUsesOfWith(IVInc->getOperand(IVAddendOp), LocalSizeCasted);
    } else {
      // new_ub = min(new_lb + chunk_size - 1, ub);
      //
      // Compute the first operand of the min(). The min() computation
      // will be done below.
      ConstantInt *ValueOne = ConstantInt::get(cast<IntegerType>(LBType), 1);
      Value *Ch = Builder.CreateSub(Chunk, ValueOne);
      NewUB = Builder.CreateAdd(LB, Ch);
    }
  } else {
    // Use SPMD mode by setting lower and upper bound to get_global_id().
    // This will let each WI execute just one iteration of the loop.
    CallInst *GlobalId;

    if (EmitSPIRVBuiltins) {
      std::string fname;
      SmallVector<Value *, 1> Arg;
      switch (Idx) {
      case 0: fname = "_Z28__spirv_GlobalInvocationId_xv";
        break;
      case 1: fname = "_Z28__spirv_GlobalInvocationId_yv";
        break;
      case 2: fname = "_Z28__spirv_GlobalInvocationId_zv";
        break;
      default:
        llvm_unreachable("Invalid dimentional index ");
      }
      GlobalId = VPOParoptUtils::genOCLGenericCall(fname,
        GeneralUtils::getSizeTTy(F), Arg, CallsInsertPt);
    } else {
      GlobalId = VPOParoptUtils::genOCLGenericCall("_Z13get_global_idj",
        GeneralUtils::getSizeTTy(F), Arg, CallsInsertPt);
    }

    Value *GlobalIdCasted = Builder.CreateSExtOrTrunc(GlobalId, LBType);
    Builder.CreateStore(GlobalIdCasted, LowerBnd);
    NewUB = GlobalIdCasted;
  }

  // No need to update UpperBnd for Non-strided scheduling.
  if (!IncrementByLocalSize) {
    // Compare bounds using signed/unsigned comparison based on the ZTT compare.
    // This helps optimizing CFG after Paropt. If ZTT is not found, then
    // use unsigned comparison.
    ICmpInst *ZTTCmpInst =
        WRegionUtils::getOmpLoopZeroTripTest(L, W->getEntryBBlock());
    bool IsSignedZTT = ZTTCmpInst ? ZTTCmpInst->isSigned() : false;

    Value *Compare = Builder.CreateICmp(
        IsSignedZTT ? ICmpInst::ICMP_SLT : ICmpInst::ICMP_ULT, NewUB, UB);
    Instruction *ThenTerm = SplitBlockAndInsertIfThen(
        Compare, InsertPt, false,
        MDBuilder(F->getContext()).createBranchWeights(99999, 100000), DT, LI);
    BasicBlock *ThenBB = ThenTerm->getParent();
    ThenBB->setName("then.bb.");
    IRBuilder<> BuilderThen(ThenBB);
    BuilderThen.SetInsertPoint(ThenBB->getTerminator());
    BuilderThen.CreateStore(NewUB, UpperBnd);
  }
}

// Generate the GPU loop scheduling code.
void VPOParoptTransform::genOCLLoopPartitionCode(
    WRegionNode *W, unsigned Idx, AllocaInst *LowerBnd, AllocaInst *UpperBnd,
    AllocaInst *SchedStride, AllocaInst *TeamLowerBnd, AllocaInst *TeamUpperBnd,
    AllocaInst *TeamStride, Value *UpperBndVal, Value *IsLastLoc,
    bool GenDispLoop, Instruction *TeamLB, Instruction *TeamUB,
    Instruction *TeamST) {

  assert(!TeamLowerBnd == !TeamUpperBnd &&
         !TeamLowerBnd == !TeamStride &&
         !TeamLowerBnd == !TeamLB &&
         !TeamLowerBnd == !TeamUB &&
         !TeamLowerBnd == !TeamST &&
         "genOCLLoopPartitionCode: team distribution related values "
         "must be created together.");
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

  LoadInst *LoadLB = Builder.CreateLoad(LowerBnd->getAllocatedType(), LowerBnd);
  LoadInst *LoadUB = Builder.CreateLoad(UpperBnd->getAllocatedType(), UpperBnd);

  BasicBlock *StaticInitBB = InsertPt->getParent();

  PHINode *PN = WRegionUtils::getOmpCanonicalInductionVariable(L);
  PN->removeIncomingValue(L->getLoopPreheader());
  PN->addIncoming(LoadLB, L->getLoopPreheader());

  BasicBlock *LoopExitBB = WRegionUtils::getOmpExitBlock(L);

  Value *ZTTPredicate = Builder.CreateICmpSLE(LoadLB, LoadUB, "");
  // If the above ZTT is true, and the computed loop's upper bound
  // is equal to the original loop upper bound, then this WI will execute
  // the last iteration of the whole loop. Set IsLast accordingly.
  // Note that all uses of this IsLast value are expected to be
  // after this loop finishes, where the last iteration actually
  // has applied all its effects.
  Value *LastIterPredicate = Builder.CreateICmpEQ(LoadUB,
      Builder.CreateSExtOrTrunc(UpperBndVal, LoadUB->getType()));
  LastIterPredicate = Builder.CreateAnd(ZTTPredicate, LastIterPredicate);
  LastIterPredicate =
      Builder.CreateZExtOrTrunc(LastIterPredicate, Builder.getInt32Ty());
  Builder.CreateStore(LastIterPredicate, IsLastLoc);

  VPOParoptUtils::updateOmpPredicateAndUpperBound(W, Idx, LoadUB, InsertPt);

  BranchInst *PreHdrInst = cast<BranchInst>(InsertPt);
  assert(PreHdrInst->getNumSuccessors() == 1 &&
         "Expect preheader BB has one exit!");

  BasicBlock *LoopRegionExitBB =
      SplitBlock(LoopExitBB, LoopExitBB->getFirstNonPHI(), DT, LI);
  LoopRegionExitBB->setName("loop" + Twine(Idx) + ".region.exit");

  if (LoopExitBB == W->getExitBBlock())
    W->setExitBBlock(LoopRegionExitBB);

  std::swap(LoopExitBB, LoopRegionExitBB);
  Instruction *NewTermInst =
      BranchInst::Create(PreHdrInst->getSuccessor(0), LoopExitBB, ZTTPredicate);
  ReplaceInstWithInst(InsertPt, NewTermInst);

  WRNScheduleKind SchedKind = getSchedKindForMultiLevelLoops(
      W, VPOParoptUtils::getLoopScheduleKind(W), WRNScheduleStaticEven);

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

  if (GenDispLoop) {
    assert(TeamLowerBnd && TeamUpperBnd && TeamStride &&
           TeamLB && TeamUB && TeamST &&
           "genOCLLoopPartitionCode: uninitialized team bounds.");

    Loop *OuterLoop = genDispatchLoopForTeamDistribute(
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
Loop *VPOParoptTransform::genDispatchLoopForTeamDistribute(
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

  Type *UBTy = UpperBndVal->getType();
  // Generate a upper bound load instruction at top of TeamDispHeaderBB
  LoadInst *TmpUB = new LoadInst(UBTy, TeamUpperBnd, "team.ub.tmp", TeamLB);

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
      Builder.CreateLoad(UBTy, TeamStride, false, "team.st.inc");

  // Generate team.inc.lb = team.new.lb + team.new.st
  BinaryOperator *IncLB =
      BinaryOperator::CreateAdd(TeamLB, TeamStrideVal, "team.inc.lb");
  IncLB->insertBefore(TermInst);

  // Generate team.inc.ub = team.new.lb + team.new.st
  BinaryOperator *IncUB =
      BinaryOperator::CreateAdd(TeamUB, TeamStrideVal, "team.inc.ub");
  IncUB->insertBefore(TermInst);

  StoreInst *NewIncLB = new StoreInst(IncLB, TeamLowerBnd, false, TermInst);
  NewIncLB->setAlignment(Align(4));

  StoreInst *NewIncUB = new StoreInst(IncUB, TeamUpperBnd, false, TermInst);
  NewIncUB->setAlignment(Align(4));

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
  Type *UBTy = UpperBndVal->getType();

  // Generate dispatch header BBlock
  BasicBlock *DispatchHeaderBB = SplitBlock(StaticInitBB, LoadLB, DT, LI);
  DispatchHeaderBB->setName("dispatch.header");

  // Generate a upper bound load instruction at top of DispatchHeaderBB
  LoadInst *TmpUB = new LoadInst(UBTy, UpperBnd, "ub.tmp", LoadLB);

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
  LoadInst *StrideVal = Builder.CreateLoad(UBTy, SchedStride, false, "st.inc");

  // Generate inc.lb.new = lb.new + st.new
  auto IncLB = Builder.CreateAdd(Builder.CreateLoad(UBTy, LowerBnd), StrideVal,
                                 "lb.inc");

  // Generate inc.lb.new = lb.new + st.new
  auto IncUB = Builder.CreateAdd(Builder.CreateLoad(UBTy, UpperBnd), StrideVal,
                                 "ub.inc");

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
bool VPOParoptTransform::genOCLParallelLoop(
    WRegionNode *W, SmallVectorImpl<Value *> &IsLastLocs) {

  AllocaInst *LowerBnd = nullptr;
  AllocaInst *UpperBnd = nullptr;
  AllocaInst *SchedStride = nullptr;
  AllocaInst *TeamLowerBnd = nullptr;
  AllocaInst *TeamUpperBnd = nullptr;
  AllocaInst *TeamStride = nullptr;
  Value *UpperBndVal = nullptr;
  Instruction *TeamLB = nullptr;
  Instruction *TeamUB = nullptr;
  Instruction *TeamST = nullptr;
  const WRNScheduleKind DistSchedKind = getSchedKindForMultiLevelLoops(
      W, VPOParoptUtils::getDistLoopScheduleKind(W),
      WRNScheduleDistributeStaticEven);

  bool ChunkForTeams =
      (W->getIsDistribute() &&
       // Each iteration of the original distribute parallel loop is executed
       // by a single WI, and we rely on OpenCL paritioning of WIs across WGs.
       // Thus, there is no need to compute the team bounds.
       (WRegionUtils::isDistributeNode(W) || !VPOParoptUtils::useSPMDMode(W)));

  bool GenTeamDistDispatchLoop =
      // Team distribute dispatch loop is only needed for chunked
      // distribution.
      (DistSchedKind == WRNScheduleDistributeStatic && ChunkForTeams);

  IRBuilder<> AllocaBuilder(
      VPOParoptUtils::getInsertionPtForAllocas(W, F, /*OutsideRegion=*/false));

  bool DoNotPartition = false;
  // Do not partition parallel loop(s) lexically nested in other
  // parallel loop(s). The current partitioning will produce incorrect
  // results for such loops. For the time being, we execute them serially.
  //
  // Note that we have to stop looking for outer parallel loop(s)
  // if during the search we meet a target region.
  if (W->getIsParLoop() &&
      WRegionUtils::getParentRegion(W,
          [](WRegionNode *W) { return W->getIsParLoop(); },
          [](WRegionNode *W) { return !isa<WRNTargetNode>(W); }))
    DoNotPartition = true;

  for (unsigned I = W->getWRNLoopInfo().getNormIVSize(); I > 0; --I) {

    if (DoNotPartition) {
      Value *IsLastLoc = AllocaBuilder.CreateAlloca(
          AllocaBuilder.getInt32Ty(), nullptr,
          "loop" + Twine(I - 1) + ".is.last");
      // Since each "thread" will execute the loop completely, and
      // the last iteration check only matters after the loop, we may
      // always initialize the last iteration predicate to true
      // before the loop.
      AllocaBuilder.CreateStore(AllocaBuilder.getInt32(1), IsLastLoc);
      IsLastLocs.push_back(IsLastLoc);
      continue;
    }

    IsLastLocs.push_back(nullptr);

    genLoopBoundUpdatePrep(W, I - 1, AllocaBuilder,
                           LowerBnd, UpperBnd, SchedStride,
                           TeamLowerBnd, TeamUpperBnd, TeamStride,
                           IsLastLocs.back(), UpperBndVal, ChunkForTeams);

    if (ChunkForTeams)
      // Loop chunking must be done for distribute constructs.
      // Note that this does not necessarily means generating
      // the team distribution dispatch loop. Even if we do not
      // generate the dispatch loop, we have to update the lower/upper
      // bounds so that each team computes its chunk of work.
      genOCLDistParLoopBoundUpdateCode(W, I - 1, LowerBnd, UpperBnd,
                                       TeamLowerBnd, TeamUpperBnd, TeamStride,
                                       DistSchedKind, TeamLB, TeamUB, TeamST);

    if (isa<WRNParallelSectionsNode>(W) ||
        WRegionUtils::isDistributeParLoopNode(W) ||
        isa<WRNParallelLoopNode>(W) || isa<WRNWksLoopNode>(W) ||
        isa<WRNSectionsNode>(W))
      genOCLLoopBoundUpdateCode(W, I - 1, LowerBnd, UpperBnd, SchedStride);

    genOCLLoopPartitionCode(W, I - 1, LowerBnd, UpperBnd, SchedStride,
                            TeamLowerBnd, TeamUpperBnd, TeamStride, UpperBndVal,
                            IsLastLocs.back(), GenTeamDistDispatchLoop,
                            TeamLB, TeamUB, TeamST);
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
        PointerType::get(Builder.getInt8Ty(), vpo::ADDRESS_SPACE_GENERIC);
    Type *VoidTy = Builder.getVoidTy();
    Type *I32Ty = Builder.getInt32Ty();
    Type *I64Ty = Builder.getInt64Ty();
    Type *I1Ty = Builder.getInt1Ty();

    // TODO: OPAQUEPOINTER: Use of Opaque Pointer compliant APIs when created.
    // The AddrSpaceCast maybe needed. PointerCasts will be removed when
    // opaque pointers are used.
    auto castArgumentToAddressSpaceGeneric = [&Builder, &PtrTy,
                                              &CI](unsigned Idx) {
      CI->setArgOperand(Idx, Builder.CreatePointerBitCastOrAddrSpaceCast(
                                 CI->getArgOperand(Idx), PtrTy));
    };

    auto castArgumentToI64 = [&Builder, &I64Ty, &CI](unsigned Idx) {
      CI->setArgOperand(
          Idx, Builder.CreateIntCast(CI->getArgOperand(Idx), I64Ty, false));
    };

    // TODO: OPAQUEPOINTER:Use of Opaque Pointer compliant APIs when created.
    // Currently, Function Creation relies on FunctionTy-PointerTy.
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
bool VPOParoptTransform::addBranchToEndDirective(WRegionNode *W) {
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
  Value *GlobLoad = Builder.CreateLoad(AllocaBuilder.getInt1Ty(), TempAddr,
                                       true, "temp.load"); // (3)
  Value *CmpInst =
      Builder.CreateICmpNE(GlobLoad, Builder.getInt1(0), "cmp"); //     (4)

  SplitBlockAndInsertIfThen(CmpInst, InsertPt, false, nullptr, // INTEL
                            (DomTreeUpdater *)nullptr, nullptr, // INTEL
                            NewExitBB); //                              (5)

  StringRef ClauseString =
      VPOAnalysisUtils::getClauseString(QUAL_OMP_JUMP_TO_END_IF);

  CallInst *CI = cast<CallInst>(EntryDirective);
  CI = VPOUtils::addOperandBundlesInCall(
      CI, {{ClauseString, {TempAddr}}}); //                             (2)
  W->setEntryDirective(CI);

  LLVM_DEBUG(dbgs() << __FUNCTION__
                    << ": Created a branch from region.entry to region.exit "
                       "for WRegion '"
                    << W->getNumber() << "', using '";
             TempAddr->printAsOperand(dbgs()); dbgs() << "'.\n";);

  return true;
}

bool VPOParoptTransform::genLaunderIntrinIfPrivatizedInAncestor(
    WRegionNode *W) {

  std::unordered_set<Value *> GlobalValues;
  std::unordered_set<Value *> ValuesToChange;

  // Early bailout if W is not a nested WRegion.
  WRegionNode *Parent = W->getParent();
  if (Parent == nullptr)
    return false;

  // 1. Find all W clause item values that use a Global Variable and add them to
  // GlobalValues set.

  auto addValueIfGlobal = [&](Item *I) {
    if (GeneralUtils::isOMPItemGlobalVAR(I->getOrig()))
      GlobalValues.insert(I->getOrig());
  };
  // Check all clauses where a variable can be used.
  VPOParoptUtils::executeForEachItemInClause(W->getSharedIfSupported(),
                                             addValueIfGlobal);
  VPOParoptUtils::executeForEachItemInClause(W->getPrivIfSupported(),
                                             addValueIfGlobal);
  VPOParoptUtils::executeForEachItemInClause(W->getFprivIfSupported(),
                                             addValueIfGlobal);
  VPOParoptUtils::executeForEachItemInClause(W->getLprivIfSupported(),
                                             addValueIfGlobal);
  VPOParoptUtils::executeForEachItemInClause(W->getRedIfSupported(),
                                             addValueIfGlobal);
  VPOParoptUtils::executeForEachItemInClause(W->getLinearIfSupported(),
                                             addValueIfGlobal);
  VPOParoptUtils::executeForEachItemInClause(W->getMapIfSupported(),
                                             addValueIfGlobal);

  // 2. If a value in GlobalValues is privatized by an ancestor, add it to
  // ValuesToChange so it will be laundered.
  while (Parent != nullptr) {
    // If parent has any target construct, then launder all GlobalValues.
    if (Parent->getIsTarget()) {
      ValuesToChange = GlobalValues;
      break;
    }

    auto AddToFinalSet = [&](Item *I) {
      auto It = GlobalValues.find(I->getOrig());
      if (It != GlobalValues.end())
        ValuesToChange.insert(*It);
    };

    VPOParoptUtils::executeForEachItemInClause(Parent->getPrivIfSupported(),
                                               AddToFinalSet);
    VPOParoptUtils::executeForEachItemInClause(Parent->getFprivIfSupported(),
                                               AddToFinalSet);
    VPOParoptUtils::executeForEachItemInClause(Parent->getLprivIfSupported(),
                                               AddToFinalSet);
    VPOParoptUtils::executeForEachItemInClause(Parent->getRedIfSupported(),
                                               AddToFinalSet);
    VPOParoptUtils::executeForEachItemInClause(Parent->getLinearIfSupported(),
                                               AddToFinalSet);

    Parent = Parent->getParent();
  }

  return genGlobalPrivatizationLaunderIntrin(W, &ValuesToChange);
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

  if (isTargetSPIRV() && (Mode & ParPrepare))
    RoutineChanged |= renameAndReplaceLibatomicCallsForSPIRV(F);

  if (WI->WRGraphIsEmpty()) {
    LLVM_DEBUG(
        dbgs() << "\n... No WRegion Candidates for Parallelization ...\n\n");
    return RoutineChanged;
  }

  bool NeedTID, NeedBID;

  LLVM_DEBUG(dbgs() << "\n In transform (IsTargetSPIRV: " <<
             isTargetSPIRV() << ") Dump the function ::" << *F);

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

  // Early preprocessing for outlined work regions that regularizes loops
  // and generates aliasing information. This need to be done early before
  // outlining because outlining damages aliasing information.
  if ((Mode & OmpPar) && (Mode & ParTrans))
    for (auto *W : WRegionList) {
      switch (W->getWRegionKindID()) {
      case WRegionNode::WRNTeams:
      case WRegionNode::WRNParallel:
        if (!isTargetSPIRV())
          improveAliasForOutlinedFunc(W);
        break;
      case WRegionNode::WRNParallelSections:
      case WRegionNode::WRNParallelLoop:
      case WRegionNode::WRNDistributeParLoop:
        RoutineChanged |= regularizeOMPLoop(W, false);
        improveAliasForOutlinedFunc(W);
        break;
      case WRegionNode::WRNTask:
        improveAliasForOutlinedFunc(W);
        break;
      case WRegionNode::WRNTaskloop:
        RoutineChanged |= regularizeOMPLoop(W, false);
        improveAliasForOutlinedFunc(W);
        break;
      case WRegionNode::WRNTarget:
        if (!DisableOffload)
          improveAliasForOutlinedFunc(W);
        break;
      case WRegionNode::WRNTargetData:
        if (!DisableOffload && !hasOffloadCompilation())
          improveAliasForOutlinedFunc(W);
        break;
      }
      W->resetBBSet();
    }
#endif  // INTEL_CUSTOMIZATION

  if ((Mode & OmpPar) && (Mode & ParTrans) && !DisableOffload) {
    if (isTargetSPIRV())
      propagateSPIRVSIMDWidth();
    assignParallelDimensions();
  }

  if ((Mode & OmpPar) && (Mode & ParTrans))
    RoutineChanged |= addRangeMetadataToOmpCalls();

  Type *Int32Ty = Type::getInt32Ty(C);

  if (NeedTID)
    TidPtrHolder = F->getParent()->getOrInsertGlobal("@tid.addr", Int32Ty);

  if (NeedBID && (Mode & OmpPar) && (Mode & ParTrans))
    BidPtrHolder = F->getParent()->getOrInsertGlobal("@bid.addr", Int32Ty);

  auto emitWarning = [](WRegionNode *W, const Twine &Message) {
    Function *F = W->getEntryDirective()->getFunction();
    DiagnosticInfoOptimizationFailure DI("openmp", "implementation-warning",
                                         W->getEntryDirective()->getDebugLoc(),
                                         W->getEntryBBlock());
    DI << Message.str();
    F->getContext().diagnose(DI);
  };

  SmallPtrSet<WRegionNode *, 4> RegionsNeedingDummyBranchInsertion;
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
      if (isModeOmpNoFECollapse() &&
          W->canHaveCollapse()) {
        Changed |= collapseOmpLoops(W);
        RemoveDirectives = false;
      }

      switch (W->getWRegionKindID()) {

      // 1. Constructs that need to perform outlining:
      //      Parallel [for|sections], task, taskloop, etc.

      case WRegionNode::WRNTeams:
      case WRegionNode::WRNParallel:
        debugPrintHeader(W, Mode);
        if (Mode & ParPrepare) {
          Changed |= canonicalizeGlobalVariableReferences(W);
          if (isTargetSPIRV() && isa<WRNParallelNode>(W) && hasParentTarget(W))
            Changed |= callPopPushNumThreadsAtRegionBoundary(W);
          Changed |= renameOperandsUsingStoreThenLoad(W);
          Changed |= propagateCancellationPointsToIR(W);
        }
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed |= clearCancellationPointAllocasFromIR(W);
          Changed |= genLaunderIntrinIfPrivatizedInAncestor(W);
          WRegionUtils::collectNonPointerValuesToBeUsedInOutlinedRegion(W);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
          if (isTargetCSA()) {
            if (W->getIsPar())
              Changed |= genCSAParallel(W);
            else
              llvm_unreachable("Unexpected work region kind");
            RemoveDirectives = true;
            Changed |= clearLaunderIntrinBeforeRegion(W);
            break;
          }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          bool SkipTargetCodeGenForParallelRegion =
              isTargetSPIRV() && isFunctionOpenMPTargetDeclare() &&
              !isa<WRNParallelLoopNode>(W);
          if (SkipTargetCodeGenForParallelRegion)
            emitWarning(W,
                        "'" + W->getName() +
                            +"' construct, in a declare target function, was "
                             "ignored for calls from target regions.");
          if (!isTargetSPIRV()) {
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
            // TODO: enabling firstprivatization requires more optimizations
            //       to avoid performance tanking.
            // Changed |= genFirstPrivatizationCode(W);
            if (SkipTargetCodeGenForParallelRegion)
              RemoveDirectives = true;
            else {
              Changed |= genPrivatizationCode(W);
              Changed |= genReductionCode(W);
              Changed |= genDestructorCode(W);

              // The directive gets removed, when processing the target region,
              // do not remove it here, since guardSideEffects needs the
              // parallel directive to insert barriers.
              RemoveDirectives = false;
              HandledWithoutRemovingDirectives = true;
            }
          }
          Changed |= clearLaunderIntrinBeforeRegion(W);
          LLVM_DEBUG(dbgs()<<"\n Parallel W-Region::"<<*W->getEntryBBlock());
        }
        break;
      case WRegionNode::WRNParallelSections:
      case WRegionNode::WRNParallelLoop:
      case WRegionNode::WRNDistributeParLoop:
        debugPrintHeader(W, Mode);
        if (Mode & ParPrepare) {
          if (W->getIsParSections())
            Changed |= addNormUBsToParents(W);

          Changed |= regularizeOMPLoop(W);
          Changed |= canonicalizeGlobalVariableReferences(W);
          Changed |= renameOperandsUsingStoreThenLoad(W);
          if (isTargetSPIRV() && hasParentTarget(W))
            Changed |= callPopPushNumThreadsAtRegionBoundary(W);
          Changed |= propagateCancellationPointsToIR(W);
          Changed |= fixupKnownNDRange(W);
        }
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed |= clearCancellationPointAllocasFromIR(W);
          WRegionUtils::collectNonPointerValuesToBeUsedInOutlinedRegion(W);
          Changed |= genLaunderIntrinIfPrivatizedInAncestor(W);
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

          // Ignore parallel-for/sections constructs in declare-target functions
          bool SkipTargetCodeGenForParallelRegion =
              isTargetSPIRV() && isFunctionOpenMPTargetDeclare() &&
              !W->getIsDistribute();
          bool TreatDistributeParLoopAsDistribute =
              isTargetSPIRV() && isFunctionOpenMPTargetDeclare() &&
              W->getIsDistribute();

          if (SkipTargetCodeGenForParallelRegion)
            emitWarning(W,
                        "'" + W->getName() +
                            +"' construct, in a declare target function, was "
                             "ignored for calls from target regions.");
          else if (TreatDistributeParLoopAsDistribute) {
            W->setTreatDistributeParLoopAsDistribute(true);
            emitWarning(W, "'" + W->getName() +
                               "' construct, in a declare target function, was "
                               "interpreted as 'distribute', for calls from "
                               "target regions.");
          }

          // The compiler does not need to generate the outlined function
          // for omp parallel for loop.
          if (isTargetSPIRV()) {
            if (!SkipTargetCodeGenForParallelRegion) {
              SmallVector<Value *, 3> IsLastLocs;
              BasicBlock *IfLastIterBB = nullptr;
              Changed |= genOCLParallelLoop(W, IsLastLocs);
              Changed |= genPrivatizationCode(W);
              Changed |= genLastIterationCheck(W, IsLastLocs, IfLastIterBB);
              Changed |= genLastPrivatizationCode(W, IfLastIterBB);
              Changed |= genFirstPrivatizationCode(W);
              Changed |= genReductionCode(W);
              Changed |= genDestructorCode(W);
            }
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
            Changed |= genLastIterationCheck(W, {IsLastVal}, IfLastIterBB,
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
          Changed |= clearLaunderIntrinBeforeRegion(W);
          Changed |= sinkSIMDDirectives(W);
          Changed |= genParallelAccessMetadata(W);
          RemoveDirectives = true;

          LLVM_DEBUG(dbgs()<<"\n Parallel W-Region::"<<*W->getEntryBBlock());

          if (isTargetSPIRV() && !SkipTargetCodeGenForParallelRegion &&
              !TreatDistributeParLoopAsDistribute) {
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
          debugPrintHeader(W, Mode);
          StructType *KmpTaskTTWithPrivatesTy;
          StructType *KmpSharedTy;
          Value *LastIterGep;
          BasicBlock *IfLastIterBB = nullptr;
          // Task Construct Doesn't need a call to
          // genLaunderIntrinIfPrivatizedInAncestor because
          // outlining algorithm stores the global variables in the kmpc_struct
          // as it should.
          Changed |= genTaskInitCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy,
                                     LastIterGep);
          Changed |= genPrivatizationCode(W);
          Changed |= genFirstPrivatizationCode(W);
          Changed |= genBarrierForFpLpAndLinears(W);
          Changed |= genLastIterationCheck(W, {LastIterGep}, IfLastIterBB);
          Changed |= genLastPrivatizationCode(W, IfLastIterBB);
          Changed |= genSharedCodeForTaskGeneric(W);
          Changed |= genRedCodeForTaskGeneric(W);
          Changed |= genCancellationBranchingCode(W);
          Changed |= genTaskCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNTaskloop:
        debugPrintHeader(W, Mode);
        if (Mode & ParPrepare) {
          Changed |= regularizeOMPLoop(W);
          Changed |= canonicalizeGlobalVariableReferences(W);
          Changed |= renameOperandsUsingStoreThenLoad(W);
        }
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          StructType *KmpTaskTTWithPrivatesTy;
          StructType *KmpSharedTy;
          AllocaInst *LBPtr, *UBPtr, *STPtr;
          Value *LastIterGep;
          BasicBlock *IfLastIterBB = nullptr;
          // Taskloop Construct Doesn't need a call to
          // genLaunderIntrinIfPrivatizedInAncestor because
          // outlining algorithm stores the global variables in the kmpc_struct
          // as it should.
          Changed |= addFirstprivateForNormalizedUB(W);
          Changed |=
              genTaskLoopInitCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy,
                                  LBPtr, UBPtr, STPtr, LastIterGep);
          Changed |= genPrivatizationCode(W);
          Changed |= genLastIterationCheck(W, {LastIterGep}, IfLastIterBB);
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
          debugPrintHeader(W, Mode);
          Changed |= genTaskWaitCode(W);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNTarget:
        if (DisableOffload) {
          // Ignore TARGET construct, but maintain [FIRST]PRIVATE semantics
          LLVM_DEBUG(dbgs()<<"VPO: Ignored " << W->getName() << " construct\n");
          Changed |= genPrivatizationCode(W);
          Changed |= genFirstPrivatizationCode(W);
          RemoveDirectives = true;
          break;
        }
        debugPrintHeader(W, Mode);
        if (Mode & ParPrepare) {
          // Override function linkage for the target compilation to prevent
          // functions with target regions from being deleted by LTO.
          if (hasOffloadCompilation())
            F->setLinkage(GlobalValue::LinkageTypes::ExternalLinkage);
          Changed |= canonicalizeGlobalVariableReferences(W);
          if (isTargetSPIRV() && !hasParentTarget(W) &&
              !isFunctionOpenMPTargetDeclare())
            Changed |= callPushPopNumThreadsAtRegionBoundary(W, true);
          Changed |= renameOperandsUsingStoreThenLoad(W);
        } else if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed |= constructNDRangeInfo(W);
          Changed |= addMapAndPrivateForIsDevicePtr(W);
          Changed |= promoteClauseArgumentUses(W);
          // The purpose is to generate place holder for global variable.
          Changed |= genGlobalPrivatizationLaunderIntrin(W);
          WRegionUtils::collectNonPointerValuesToBeUsedInOutlinedRegion(W);
          Changed |= genPrivatizationCode(W);
          Changed |= genFirstPrivatizationCode(W);
          Changed |= captureAndAddCollectedNonPointerValuesToSharedClause(W);
          Changed |= genDestructorCode(W);
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
        debugPrintHeader(W, Mode);
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
        debugPrintHeader(W, Mode);
        if (Mode & ParPrepare) {
          if (!hasOffloadCompilation()) {
            Changed |= canonicalizeGlobalVariableReferences(W);
            Changed |= renameOperandsUsingStoreThenLoad(W);
          }
        } else if ((Mode & OmpPar) && (Mode & ParTrans)) {
          if (!hasOffloadCompilation()) {
            // The purpose is to generate place holder for global variable.
            Changed |= genGlobalPrivatizationLaunderIntrin(W);
            Changed |= addMapForUseDevicePtr(W);
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
          debugPrintHeader(W, Mode);
          if (!hasOffloadCompilation()) { // for host only
            // The purpose is to generate place holder for global use_device_ptr
            // operands.
            Changed |= genGlobalPrivatizationLaunderIntrin(W);
            Changed |= genTargetVariantDispatchCode(W);
            Changed |= clearLaunderIntrinBeforeRegion(W);
          }
          RemoveDirectives = true;
        }
        break;

      case WRegionNode::WRNTaskgroup:
        debugPrintHeader(W, Mode);
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed |= genTaskgroupRegion(W);
          RemoveDirectives = true;
        }
        break;

      case WRegionNode::WRNVecLoop:
        if (Mode & ParPrepare) {
          Changed |= regularizeOMPLoop(W);
          Changed |= canonicalizeGlobalVariableReferences(W);
          Changed |= renameOperandsUsingStoreThenLoad(W);
          Changed |= fixupKnownNDRange(W);
        }
        // Privatization is enabled for SIMD Transform passes
        if ((Mode & OmpVec) && (Mode & ParTrans)) {
          debugPrintHeader(W, Mode);
          Changed |= regularizeOMPLoop(W, false);
          Changed |= genPrivatizationCode(W);
          Changed |= genAlignedCode(W);
          Changed |= genNontemporalCode(W);
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
            Changed |= genLinearCodeForVecLoop(W, LoopExitBB);
            Changed |= genLastPrivatizationCode(W, LoopExitBB);
            Changed |= genReductionCode(W);
            Changed |= sinkSIMDDirectives(W);
            Changed |= genParallelAccessMetadata(W);
          }
          // keep SIMD directives; will be processed by the Vectorizer
          RemoveDirectives = false;
        }
        break;
      case WRegionNode::WRNAtomic:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, Mode);
          if (isTargetSPIRV())
            Changed |= removeCompilerGeneratedFences(W);
          Changed |= VPOParoptAtomics::handleAtomic(
              cast<WRNAtomicNode>(W), IdentTy, TidPtrHolder, DT, LI,
              isTargetSPIRV());
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNWksLoop:
      case WRegionNode::WRNSections:
      case WRegionNode::WRNDistribute:
        debugPrintHeader(W, Mode);
        if (Mode & ParPrepare) {
          if (W->getIsSections())
            Changed |= addNormUBsToParents(W);

          Changed |= regularizeOMPLoop(W);
          Changed |= canonicalizeGlobalVariableReferences(W);
          Changed |= renameOperandsUsingStoreThenLoad(W);
          Changed |= propagateCancellationPointsToIR(W);
          Changed |= fixupKnownNDRange(W);
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

          // Ignore Parallel-for/sections constructs in declare-target functions
          bool SkipTargetCodeGenForRegion =
              isTargetSPIRV() && isFunctionOpenMPTargetDeclare() &&
              !W->getIsDistribute() &&
              (W->getParent() && W->getParent()->getIsPar());
          if (SkipTargetCodeGenForRegion)
            emitWarning(W, "'" + W->getName() +
                               "' construct, in a declare target function, "
                               "was ignored for calls from target regions.");
          if (isTargetSPIRV()) {
            if (!SkipTargetCodeGenForRegion) {
              SmallVector<Value *, 3> IsLastLocs;
              BasicBlock *IfLastIterBB = nullptr;
              Changed |= genOCLParallelLoop(W, IsLastLocs);
              Changed |= genPrivatizationCode(W);
              Changed |= genLastIterationCheck(W, IsLastLocs, IfLastIterBB);
              Changed |= genLastPrivatizationCode(W, IfLastIterBB);
              Changed |= genFirstPrivatizationCode(W);
              if (!W->getIsDistribute())
                Changed |= genReductionCode(W);
            }
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
            Changed |= genLastIterationCheck(W, {IsLastVal}, IfLastIterBB,
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
          Changed |= genParallelAccessMetadata(W);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNSingle:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, Mode);
          // Changed |= genPrivatizationCode(W);
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
          if (isTargetSPIRV())
            Changed |= removeCompilerGeneratedFences(W);

          AllocaInst *IsSingleThread = nullptr;
          Changed |= genSingleThreadCode(W, IsSingleThread);
          Changed |= genCopyPrivateCode(W, IsSingleThread);
          Changed |= genPrivatizationCode(W);
          Changed |= genFirstPrivatizationCode(W);
          // Changed |= genDestructorCode(W);
          if (!W->getNowait())
            Changed |= genBarrier(W, false, isTargetSPIRV());
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNMaster:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, Mode);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
          if (isTargetCSA()) {
            Changed |= removeCompilerGeneratedFences(W);
            RemoveDirectives = true;
            break;
          }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          if (isTargetSPIRV())
            Changed |= removeCompilerGeneratedFences(W);
          Changed |= genMasterThreadCode(W, isTargetSPIRV());
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNCritical:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, Mode);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
          if (isTargetCSA()) {
            Changed |= removeCompilerGeneratedFences(W);
            Changed |= genCSACritical(cast<WRNCriticalNode>(W));
            RemoveDirectives = true;
            break;
          }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          Changed |= genCriticalCode(cast<WRNCriticalNode>(W));
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNOrdered:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, Mode);
          if (W->getIsDoacross()) {
            Changed |= genDoacrossWaitOrPost(cast<WRNOrderedNode>(W));
            RemoveDirectives = true;
          } else if (W->getIsThreads()) {
            Changed |= genOrderedThreadCode(W);
            if (W->getIsSIMD()) {
              StringRef OrderedThreadString =
                  VPOAnalysisUtils::getClauseString(QUAL_OMP_ORDERED_THREADS);
              CallInst *UpdatedDir = VPOUtils::removeOperandBundlesFromCall(
                  cast<CallInst>(W->getEntryDirective()),
                  {OrderedThreadString});
              W->setEntryDirective(UpdatedDir);
              RemoveDirectives = false;
            } else
              RemoveDirectives = true;
          } else
            // This is ORDERED SIMD and should not be removed.
            RemoveDirectives = false;
        }
        break;
      case WRegionNode::WRNBarrier:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, Mode);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
          if (isTargetCSA()) {
            Changed |= removeCompilerGeneratedFences(W);
            RemoveDirectives = true;
            break;
          }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          if (isTargetSPIRV())
            Changed |= removeCompilerGeneratedFences(W);
          Changed |= genBarrier(W, true, isTargetSPIRV());
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNCancel:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, Mode);
          Changed |= genCancelCode(cast<WRNCancelNode>(W));
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNFlush:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, Mode);
          Changed |= genFlush(W);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNGenericLoop:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, Mode);
          Changed |= replaceGenericLoop(W);
          Changed |= regularizeOMPLoop(W);
          Changed |= canonicalizeGlobalVariableReferences(W);
          Changed |= renameOperandsUsingStoreThenLoad(W);
          Changed |= fixupKnownNDRange(W);
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
      RegionsNeedingDummyBranchInsertion.insert(W);

    if (Changed) { // Code transformations happened for this WRN
      RoutineChanged = true;
      LLVM_DEBUG(dbgs() << "   === WRN #" << W->getNumber()
                        << " transformed.\n\n");
    }
    else
      LLVM_DEBUG(dbgs() << "   === WRN #" << W->getNumber()
                        << " NOT transformed.\n\n");
  }

  for (auto *W : WRegionList)
    if (RegionsNeedingDummyBranchInsertion.count(W))
      RoutineChanged |= addBranchToEndDirective(W);

  WRegionList.clear();
  return RoutineChanged;
}

// Generate calling constructor and initializer function for user-defined
// reduction.
void VPOParoptTransform::genReductionUdrInit(ReductionItem *RedI,
                                             Value *ReductionVar,
                                             Value *ReductionValueLoc,
                                             Type *ScalarTy,
                                             IRBuilder<> &Builder) {
  VPOParoptUtils::genConstructorCall(RedI->getConstructor(), ReductionValueLoc,
                                     Builder);

  Function *Fn = RedI->getInitializer();
  if (Fn != nullptr) {
    CallInst *Call = VPOParoptUtils::genCall(
        Fn->getParent(), Fn, {ReductionValueLoc, ReductionVar},
        {ReductionValueLoc->getType(), ReductionVar->getType()}, nullptr);
    Builder.Insert(Call);
    BasicBlock::iterator InsertPt = Builder.GetInsertPoint();
    if (Builder.GetInsertBlock()->end() != InsertPt)
      Call->setDebugLoc((&*InsertPt)->getDebugLoc());
  } else {
    // if initializer is null but constructor is not null, let constructor do
    // default initialization. Otherwise, initialize the variable to 0.
    if (RedI->getConstructor() != nullptr)
      return;

    Value *V = nullptr;
    if (RedI->getIsComplex()) {
      Constant *ValueZero =
          ConstantFP::get(ScalarTy->getStructElementType(0), 0.0);
      V = ConstantStruct::get(cast<StructType>(ScalarTy),
                              {ValueZero, ValueZero});
      Builder.CreateStore(V, ReductionValueLoc);
    } else if (ScalarTy->isIntOrIntVectorTy() || ScalarTy->isFPOrFPVectorTy()) {
      V = ScalarTy->isIntOrIntVectorTy() ? ConstantInt::get(ScalarTy, 0)
                                         : ConstantFP::get(ScalarTy, 0.0);
      Builder.CreateStore(V, ReductionValueLoc);
    } else {
      V = ConstantInt::get(Builder.getInt8Ty(), 0);
      const DataLayout &DL =
          Builder.GetInsertBlock()->getModule()->getDataLayout();
      uint64_t Size = DL.getTypeAllocSize(
          ReductionValueLoc->getType()->getPointerElementType());
      unsigned Alignment = 0;
      if (auto *AI = dyn_cast<AllocaInst>(RedI->getNew()->stripPointerCasts()))
        Alignment = AI->getAlignment();
      VPOUtils::genMemset(ReductionValueLoc, V, Size, Alignment, Builder);
    }
  }
}

Value *VPOParoptTransform::genReductionMinMaxInit(ReductionItem *RedI,
                                                  Type *Ty, bool IsMax) {
  Value *V = nullptr;

  if (Ty->isIntOrIntVectorTy()) {
    LLVMContext &C = F->getContext();
    bool IsUnsigned = RedI->getIsUnsigned();
    V = VPOParoptUtils::getMinMaxIntVal(C, Ty, IsUnsigned, !IsMax);
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

  assert(!RedI->getIsComplex() ||
         RedI->getType() == ReductionItem::WRNReductionAdd ||
         RedI->getType() == ReductionItem::WRNReductionSub ||
         RedI->getType() == ReductionItem::WRNReductionMult &&
             "Unsupported operation in OMP reduction for complex type!");

  switch (RedI->getType()) {
  case ReductionItem::WRNReductionAdd:
  case ReductionItem::WRNReductionSub:
    if (RedI->getIsComplex()) {
      Constant *ValueZero =
          ConstantFP::get(ScalarTy->getStructElementType(0), 0.0);
      V = ConstantStruct::get(cast<StructType>(ScalarTy),
                              {ValueZero, ValueZero});
    } else {
      V = ScalarTy->isIntOrIntVectorTy() ? ConstantInt::get(ScalarTy, 0)
                                         : ConstantFP::get(ScalarTy, 0.0);
    }
    break;
  case ReductionItem::WRNReductionMult:
    if (RedI->getIsComplex()) {
      Constant *ValueOne =
          ConstantFP::get(ScalarTy->getStructElementType(0), 1.0);
      Constant *ValueZero =
          ConstantFP::get(ScalarTy->getStructElementType(0), 0.0);
      V = ConstantStruct::get(cast<StructType>(ScalarTy),
                              {ValueOne, ValueZero});
    } else {
      V = ScalarTy->isIntOrIntVectorTy() ? ConstantInt::get(ScalarTy, 1)
                                         : ConstantFP::get(ScalarTy, 1.0);
    }
    break;
  case ReductionItem::WRNReductionAnd:
    V = ScalarTy->isIntOrIntVectorTy() ? ConstantInt::get(ScalarTy, 1)
                                       : ConstantFP::get(ScalarTy, 1.0);
    break;
#if INTEL_CUSTOMIZATION
  case ReductionItem::WRNReductionEqv:
    V = ConstantInt::get(ScalarTy, 1);
    break;
#endif // INTEL_CUSTOMIZATION
  case ReductionItem::WRNReductionOr:
    V = ScalarTy->isIntOrIntVectorTy() ? ConstantInt::get(ScalarTy, 0)
                                       : ConstantFP::get(ScalarTy, 0.0);
    break;
  case ReductionItem::WRNReductionBxor:
  case ReductionItem::WRNReductionBor:
#if INTEL_CUSTOMIZATION
  case ReductionItem::WRNReductionNeqv:
#endif // INTEL_CUSTOMIZATION
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
// Example 1 for bool and with char type
//   char v3 = 1;
//   #pragma omp parallel for reduction(&&: v3)
//   for( i = 0; i < 3; i++ ) {
//     v3 = v3 && 1;
//   }
// IR for reduction fini code
//   %7 = load i8, i8* %v3.fast_red, align 1
//   %8 = load i8, i8* %v3, align 1
//   %orig.bool = icmp ne i8 %8, 0                               ; (1)
//   %red.bool = icmp ne i8 %7, 0                                ; (2)
//   %9 = select i1 %orig.bool, i1 %red.bool, i1 %orig.bool      ; (3)
//   %10 = zext i1 %9 to i8                                      ; (4)
//   store i8 %10, i8* %v3, align 1
//
// Example 2 for bool or with char type
//   char v3 = 1;
//   #pragma omp parallel for reduction(||: v3)
//   for( i = 0; i < 3; i++ ) {
//     v3 = v3 || 0;
//   }
// IR for reduction fini code
//  %7 = load i8, i8* %v3.fast_red, align 1
//  %8 = load i8, i8* %v3, align 1
//  %orig.bool = icmp ne i8 %8, 0                                ; (1)
//  %red.bool = icmp ne i8 %7, 0                                 ; (2)
//  %9 = select i1 %orig.bool, i1 %orig.bool, i1 %red.bool       ; (3)
//  %10 = zext i1 %9 to i8                                       ; (4)
//  store i8 %10, i8* %v3, align 1
//
Value *VPOParoptTransform::genReductionFiniForBoolOps(Value *Rhs1, Value *Rhs2,
                                                      Type *ScalarTy,
                                                      IRBuilder<> &Builder,
                                                      bool IsAnd) {
  bool IsInteger;
  if (ScalarTy->isIntOrIntVectorTy())
    IsInteger = true;
  else if (ScalarTy->isFPOrFPVectorTy())
    IsInteger = false;
  else
    llvm_unreachable("Unsupported type in reduction operation (bool and/or).");

  Value *ValueZero = IsInteger ? ConstantInt::get(ScalarTy, 0)
                               : ConstantFP::get(ScalarTy, 0.0);
  auto IsTrue =
      IsInteger ? Builder.CreateICmpNE(Rhs1, ValueZero, "orig.bool")
                : Builder.CreateFCmpUNE(Rhs1, ValueZero, "orig.bool"); // (1)
  auto IsTrueRed =
      IsInteger ? Builder.CreateICmpNE(Rhs2, ValueZero, "red.bool")
                : Builder.CreateFCmpUNE(Rhs2, ValueZero, "red.bool"); // (2)
  auto BoolVal = Builder.CreateSelect(IsTrue, IsAnd ? IsTrueRed : IsTrue,
                                      IsAnd ? IsTrue : IsTrueRed); // (3)

  auto ConvFini = IsInteger ? Builder.CreateZExtOrBitCast(BoolVal, ScalarTy)
                            : Builder.CreateUIToFP(BoolVal, ScalarTy); // (4)

  return ConvFini;
}

// Generate the fini code for user-defined reduction, which calls combiner
// function.
bool VPOParoptTransform::genReductionUdrFini(ReductionItem *RedI,
                                             Value *ReductionVar,
                                             Value *ReductionValueLoc,
                                             IRBuilder<> &Builder) {
  // combiner must not be null
  assert((RedI->getCombiner() != nullptr) &&
         "NULL combiner for user defined reduction.");

  CallInst *Res = VPOParoptUtils::genCall(RedI->getCombiner()->getParent(),
      RedI->getCombiner(), {ReductionVar, ReductionValueLoc},
      {ReductionVar->getType(), ReductionValueLoc->getType()}, nullptr);
  Builder.Insert(Res);

  BasicBlock::iterator InsertPt = Builder.GetInsertPoint();
  if (Builder.GetInsertBlock()->end() != InsertPt)
    Res->setDebugLoc((&*InsertPt)->getDebugLoc());

  return true;
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
    WRegionNode *W, ReductionItem *RedI, Value *ReductionVar,
    Value *ReductionValueLoc, Type *ScalarTy, IRBuilder<> &Builder,
    DominatorTree *DT) {
  Value *Res = nullptr;
  auto *Rhs2 = Builder.CreateLoad(
      ReductionValueLoc->getType()->getPointerElementType(), ReductionValueLoc);
  auto *Rhs1 = Builder.CreateLoad(
      ReductionVar->getType()->getPointerElementType(), ReductionVar);

  switch (RedI->getType()) {
  case ReductionItem::WRNReductionAdd:
  case ReductionItem::WRNReductionSub:
    if (RedI->getIsComplex()) {
      Value *Rhs1Real = Builder.CreateExtractValue(Rhs1, 0, "complex_0");
      Value *Rhs2Real = Builder.CreateExtractValue(Rhs2, 0, "complex_0");
      Value *ResReal = Builder.CreateFAdd(Rhs1Real, Rhs2Real, "add");
      Value *Rhs1Img = Builder.CreateExtractValue(Rhs1, 1, "complex_1");
      Value *Rhs2Img = Builder.CreateExtractValue(Rhs2, 1, "complex_1");
      Value *ResImaginary = Builder.CreateFAdd(Rhs1Img, Rhs2Img, "add");
      Res = ConstantAggregateZero::get(Rhs1->getType());
      Res = Builder.CreateInsertValue(Res, ResReal, 0, "insertval");
      Res = Builder.CreateInsertValue(Res, ResImaginary, 1, "insertval");
    } else {
      Res = ScalarTy->isIntOrIntVectorTy() ? Builder.CreateAdd(Rhs1, Rhs2)
                                           : Builder.CreateFAdd(Rhs1, Rhs2);
    }
    break;
  case ReductionItem::WRNReductionMult:
    if (RedI->getIsComplex()) {
      // multiply two complex variables (N1=a+bi, N2=c+di) is equal to:
      //   N1*N2 = (ac-bd) + (ad+cb)i
      Value *Rhs1Real = Builder.CreateExtractValue(Rhs1, 0, "complex_0");
      Value *Rhs2Real = Builder.CreateExtractValue(Rhs2, 0, "complex_0");
      Value *Rhs1Imaginary = Builder.CreateExtractValue(Rhs1, 1, "complex_1");
      Value *Rhs2Imaginary = Builder.CreateExtractValue(Rhs2, 1, "complex_1");
      Value *Mult00 = Builder.CreateFMul(Rhs1Real, Rhs2Real, "mul");
      Value *Mult11 = Builder.CreateFMul(Rhs1Imaginary, Rhs2Imaginary, "mul");
      Value *Mult01 = Builder.CreateFMul(Rhs1Real, Rhs2Imaginary, "mul");
      Value *Mult10 = Builder.CreateFMul(Rhs1Imaginary, Rhs2Real, "mul");
      Value *ResReal = Builder.CreateFSub(Mult00, Mult11, "sub");
      Value *ResImaginary = Builder.CreateFAdd(Mult01, Mult10, "add");
      Res = ConstantAggregateZero::get(Rhs1->getType());
      Res = Builder.CreateInsertValue(Res, ResReal, 0, "insertval");
      Res = Builder.CreateInsertValue(Res, ResImaginary, 1, "insertval");
    } else {
      Res = ScalarTy->isIntOrIntVectorTy() ? Builder.CreateMul(Rhs1, Rhs2)
                                           : Builder.CreateFMul(Rhs1, Rhs2);
    }
    break;
  case ReductionItem::WRNReductionBand:
    Res = Builder.CreateAnd(Rhs1, Rhs2);
    break;
  case ReductionItem::WRNReductionBor:
    Res = Builder.CreateOr(Rhs1, Rhs2);
    break;
  case ReductionItem::WRNReductionBxor:
#if INTEL_CUSTOMIZATION
  case ReductionItem::WRNReductionNeqv:
#endif // INTEL_CUSTOMIZATION
    Res = Builder.CreateXor(Rhs1, Rhs2);
    break;
  case ReductionItem::WRNReductionAnd:
    Res = genReductionFiniForBoolOps(Rhs1, Rhs2, ScalarTy, Builder, true);
    break;
  case ReductionItem::WRNReductionOr:
    Res = genReductionFiniForBoolOps(Rhs1, Rhs2, ScalarTy, Builder, false);
    break;
  case ReductionItem::WRNReductionMax:
    Res = genReductionMinMaxFini(RedI, Rhs1, Rhs2, ScalarTy, Builder, true);
    break;
  case ReductionItem::WRNReductionMin:
    Res = genReductionMinMaxFini(RedI, Rhs1, Rhs2, ScalarTy, Builder, false);
    break;
#if INTEL_CUSTOMIZATION
  case ReductionItem::WRNReductionEqv:
    Res = Builder.CreateXor(Rhs1, Rhs2);
    Res = Builder.CreateNot(Res);
    break;
#endif // INTEL_CUSTOMIZATION
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
    Instruction *AtomicCall = VPOParoptAtomics::handleAtomicUpdateInBlock(
        W, Tmp0->getParent(), nullptr, nullptr, true);

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
    if (!isa<WRNTeamsNode>(W)) {
      // Only the master thread must execute the reduction update
      // code in teams region. Executing the horizontal reduction
      // will result in redundant reduction operations producing
      // incorrect result.
      auto *TempRedLoad = Rhs2->clone();
      TempRedLoad->insertAfter(Rhs2);
      TempRedLoad->takeName(Rhs2);
      auto *HRed = VPOParoptUtils::genSPIRVHorizontalReduction(
          RedI, ScalarTy, TempRedLoad, spirv::Scope::Subgroup);

      if (HRed)
        Rhs2->replaceAllUsesWith(HRed);
      else
        LLVM_DEBUG(dbgs() << __FUNCTION__ <<
                   ": SPIRV horizontal reduction is not available "
                   "for critical section reduction: " << RedI->getOpName() <<
                   " with type " << *ScalarTy << "\n");
    }

    OptimizationRemarkMissed R(DEBUG_TYPE, "ReductionAtomic", Tmp0);
    R << ore::NV("Kind", RedI->getOpName()) << " reduction update of type "
      << ore::NV("Type", ScalarTy) << " cannot be done using atomic API";
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
bool VPOParoptTransform::genReductionFini(WRegionNode *W, ReductionItem *RedI,
                                          Value *OldV, Instruction *InsertPt,
                                          DominatorTree *DT,
                                          bool NoNeedToOffsetOrDerefOldV) {
  Type *AllocaTy;
  Value *NumElements;
  std::tie(AllocaTy, NumElements, std::ignore) = getItemInfo(RedI);

  // TODO: for a VLA AllocaTy will be just a scalar type, and NumElements
  //       will specify the array size. Right now, VLA reductions are
  //       treated as scalar reductions. We either need to support NumElements
  //       below or ask FE to represent all array reductions (including VLAs)
  //       with array sections (which would have an explicit number of elements
  //       specified).

  Value *NewV = RedI->getNew();
  IRBuilder<> Builder(InsertPt);
  // For by-refs, do a pointer dereference to reach the actual operand.
  if (RedI->getIsByRef() && !NoNeedToOffsetOrDerefOldV)
    OldV = Builder.CreateLoad(OldV->getType()->getPointerElementType(), OldV);

#if INTEL_CUSTOMIZATION
  if (RedI->getIsF90DopeVector())
    return genRedAggregateInitOrFini(W, RedI, NewV, OldV, InsertPt, false, DT,
                                     NoNeedToOffsetOrDerefOldV);

#endif // INTEL_CUSTOMIZATION
  if (RedI->getIsArraySection() || AllocaTy->isArrayTy())
    return genRedAggregateInitOrFini(W, RedI, NewV, OldV, InsertPt, false, DT,
                                     NoNeedToOffsetOrDerefOldV);

  if (RedI->getType() == ReductionItem::WRNReductionUdr)
    return genReductionUdrFini(RedI, OldV, NewV, Builder);

  assert((VPOUtils::canBeRegisterized(AllocaTy,
                                      InsertPt->getModule()->getDataLayout()) ||
          RedI->getIsComplex()) &&
         "genReductionFini: Expect incoming scalar/complex type.");

  return genReductionScalarFini(W, RedI, OldV, NewV, AllocaTy, Builder, DT);
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
bool VPOParoptTransform::genRedAggregateInitOrFini(
    WRegionNode *W, ReductionItem *RedI, Value *AI, Value *OldV,
    Instruction *InsertPt, bool IsInit, DominatorTree *DT,
    bool NoNeedToOffsetOrDerefOldV) {

  bool NeedsKmpcCritical = false;
  IRBuilder<> Builder(InsertPt);
  auto EntryBB = Builder.GetInsertBlock();

  Type *DestElementTy = nullptr;
  Value *DestBegin = nullptr;
  Value *SrcBegin = nullptr;
  Value *NumElements = nullptr;

  Value *SrcVal = (IsInit ? OldV : AI);
  Value *DestVal = (IsInit ? AI : OldV);

  // For reduction init loop, if the reduction is UDR with non-null initializer,
  // Src and Dest are needed; otherwise, only Dest is needed.
  // For reduction fini loop, both Src and Dest are needed.
  if (SrcVal == nullptr)
    genAggrReductionInitDstInfo(*RedI, DestVal, InsertPt, Builder, NumElements,
                                DestBegin, DestElementTy);
  else
    genAggrReductionSrcDstInfo(*RedI, SrcVal, DestVal, InsertPt, Builder,
                               NumElements, SrcBegin, DestBegin, DestElementTy,
                               NoNeedToOffsetOrDerefOldV);

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
  if (SrcBegin != nullptr) {
    SrcElementPHI =
        Builder.CreatePHI(SrcBegin->getType(), 2, "red.cpy.src.ptr");
    SrcElementPHI->addIncoming(SrcBegin, EntryBB);
  }

  bool IsUDR = (RedI->getType() == ReductionItem::WRNReductionUdr);
  if (IsInit) {
    if (IsUDR)
      genReductionUdrInit(RedI, SrcElementPHI, DestElementPHI, DestElementTy,
                          Builder);
    else {
      Value *V = genReductionScalarInit(RedI, DestElementTy);
      Builder.CreateStore(V, DestElementPHI);
    }
  } else {
    if (IsUDR)
      NeedsKmpcCritical |=
          genReductionUdrFini(RedI, DestElementPHI, SrcElementPHI, Builder);
    else
      NeedsKmpcCritical |= genReductionScalarFini(
          W, RedI, DestElementPHI, SrcElementPHI, DestElementTy, Builder, DT);
  }

  auto DestElementNext =
      Builder.CreateConstGEP1_32(DestElementPHI, 1, "red.cpy.dest.inc");
  Value *SrcElementNext = nullptr;
  if (SrcElementPHI != nullptr)
    SrcElementNext =
        Builder.CreateConstGEP1_32(SrcElementPHI, 1, "red.cpy.src.inc");

  auto Done = Builder.CreateICmpEQ(DestElementNext, DestEnd, "red.cpy.done");

  Builder.CreateCondBr(Done, DoneBB, BodyBB);
  DestElementPHI->addIncoming(DestElementNext, Builder.GetInsertBlock());
  if (SrcElementPHI != nullptr)
    SrcElementPHI->addIncoming(SrcElementNext, Builder.GetInsertBlock());

  if (DT) {
    DT->changeImmediateDominator(BodyBB, EntryBB);
    DT->changeImmediateDominator(DoneBB, EntryBB);
  }

  return NeedsKmpcCritical;
}

// Utility to copy data from address "From" to address "To", with an optional
// copy constructor call (can be null) and by-ref dereference (false);
void VPOParoptTransform::genCopyByAddr(Item *I, Value *To, Value *From,
                                       Instruction *InsertPt, Function *Cctor,
                                       bool IsByRef) {
  IRBuilder<> Builder(InsertPt);
  const DataLayout &DL = InsertPt->getModule()->getDataLayout();
  AllocaInst *AI = dyn_cast<AllocaInst>(To);
  if (!AI)
    AI = dyn_cast<AllocaInst>(From);

  // For by-refs, do a pointer dereference to reach the actual operand.
  if (IsByRef)
    From = Builder.CreateLoad(From->getType()->getPointerElementType(), From);
  assert(From->getType()->isPointerTy() && To->getType()->isPointerTy());
  Type *ObjType = AI ? AI->getAllocatedType()
                     : From->getType()->getPointerElementType();
  if (Cctor) {
    genPrivatizationInitOrFini(I, Cctor, FK_CopyCtor, To, From, InsertPt, DT);
  } else if (!VPOUtils::canBeRegisterized(ObjType, DL) ||
             (AI && AI->isArrayAllocation())) {
    unsigned Alignment = DL.getABITypeAlignment(ObjType);
    uint64_t Size = DL.getTypeAllocSize(To->getType()->getPointerElementType());
    VPOUtils::genMemcpy(To, From, Size, Alignment, Builder);
  } else {
    Builder.CreateStore(
        Builder.CreateLoad(From->getType()->getPointerElementType(), From), To);
  }
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
  auto *OldV = FprivI->getOrig();
  // NewV is either AllocaInst, GEP, or an AddrSpaceCastInst of AllocaInst.
  assert(GeneralUtils::isOMPItemLocalVAR(NewV) &&
         "genFprivInit: Expect isOMPItemLocalVAR().");
#if INTEL_CUSTOMIZATION
  if (FprivI->getIsF90DopeVector()) {
    if (FprivI->getIsByRef())
      OldV = new LoadInst(FprivI->getNew()->getType(), OldV, "", InsertPt);
    VPOParoptUtils::genF90DVFirstprivateCopyCall(NewV, OldV, InsertPt,
                                                 isTargetSPIRV());
    return;
  }
#endif // INTEL_CUSTOMIZATION
  genCopyByAddr(FprivI, NewV, OldV, InsertPt, FprivI->getCopyConstructor(),
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
void VPOParoptTransform::genLprivFini(Item *I, Value *NewV, Value *OldV,
                                      Instruction *InsertPt) {
  // NewV is either AllocaInst or an AddrSpaceCastInst of AllocaInst.
  assert(GeneralUtils::isOMPItemLocalVAR(NewV) &&
         "genLprivFini: Expect isOMPItemLocalVAR().");
  // todo: copy constructor call needed?
  genCopyByAddr(I, OldV, NewV, InsertPt->getParent()->getTerminator());
}

// genLprivFini interface to support nonPOD with call to CopyAssign
void VPOParoptTransform::genLprivFini(LastprivateItem *LprivI,
                                      Instruction *InsertPt) {
  Value *NewV = LprivI->getNew();
  Value *OldV = LprivI->getOrig();
  // For by-refs, do a pointer dereference to reach the actual operand.
  if (LprivI->getIsByRef())
    OldV = new LoadInst(NewV->getType(), OldV, "", InsertPt);

#if INTEL_CUSTOMIZATION
  if (LprivI->getIsF90DopeVector()) {
    VPOParoptUtils::genF90DVLastprivateCopyCall(NewV, OldV, InsertPt,
                                                isTargetSPIRV());
    return;
  }
#endif // INTEL_CUSTOMIZATION

  if (Function *CpAssn = LprivI->getCopyAssign()) {
    genPrivatizationInitOrFini(LprivI, CpAssn, FK_CopyAssign, OldV, NewV,
                               InsertPt, DT);
    return;
  }

  genLprivFini(LprivI, NewV, OldV, InsertPt);
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
  AllocaInst *MaxLocalIndex = AllocaBuilder.CreateAlloca(
      LoopIdxTy, nullptr, NamePrefix + ".local.max.idx"); //          (2), (31)
  AllocaBuilder.CreateStore(                              //          (3), (32)
      AllocaBuilder.getIntN(LoopIdxTy->getIntegerBitWidth(), 0), MaxLocalIndex);

  AllocaInst *ModifiedByCurrentThread = AllocaBuilder.CreateAlloca( // (4), (5)
      AllocaBuilder.getInt1Ty(), nullptr, NamePrefix + ".modified.by.thread");
  AllocaBuilder.CreateStore(AllocaBuilder.getFalse(),
                            ModifiedByCurrentThread); //              (33), (34)

  Instruction *ValInMaxLocalIndex = nullptr;
  AllocaInst *ModifiedByCurrentChunk = nullptr;

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
    Value *ModifiedByChunk = ChunkFiniBuilder.CreateLoad( //          (42)
        ModifiedByCurrentChunk->getAllocatedType(),
        ModifiedByCurrentChunk);
    Value *IsModified = ChunkFiniBuilder.CreateICmpEQ(       //       (43)
        ModifiedByChunk, ChunkFiniBuilder.getTrue(), NamePrefix + ".modified");
    Value *MaxLBToModifyVarForThreadTillNow =
        ChunkFiniBuilder.CreateLoad(MaxLocalIndex->getAllocatedType(),
                                    MaxLocalIndex); //                (44)
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
    auto *ValueInChunk = ModifiedBuilder.CreateLoad(
        LprivINew->getType()->getPointerElementType(), LprivINew); // (50)
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
  auto *ModifiedByCurrentThreadLoad = GlobalMaxComputationBuilder.CreateLoad(
      ModifiedByCurrentThread->getAllocatedType(),
      ModifiedByCurrentThread); //                                         (53)
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

  auto *FinalLocalMaxIndex = GlobalMaxComputationBuilder.CreateLoad(
      MaxLocalIndex->getAllocatedType(),MaxLocalIndex); //            (56)
  auto *GlobalMaxIndexLoad = GlobalMaxComputationBuilder.CreateLoad(
      MaxGlobalIndex->getValueType(), MaxGlobalIndex); //             (58)
  auto *IsLocalGreaterThanGlobal = GlobalMaxComputationBuilder.CreateICmpUGE(
      FinalLocalMaxIndex, GlobalMaxIndexLoad,
      NamePrefix + ".is.local.idx.higher"); //                        (59)
  Instruction *IfHighestChunkIsModifiedByThreadThen = SplitBlockAndInsertIfThen(
      IsLocalGreaterThanGlobal, IfThreadWroteSomethingThen, false, nullptr, DT,
      LI); //                                                         (60)
  StoreInst *MaxStore =
      new StoreInst(FinalLocalMaxIndex, MaxGlobalIndex, false,
                    IfHighestChunkIsModifiedByThreadThen); //         (61)
  (void)MaxStore;

  // TODO: This critical section should be switched with either atomic-max
  // reduction, or atomic operation
  GlobalMaxComputationBuilder.SetInsertPoint(ConditionalLPBarrier);
  VPOParoptUtils::genKmpcCriticalSection(
      W, IdentTy, TidPtrHolder, GlobalMaxIndexLoad, IfThreadWroteSomethingThen,
      DT, LI, isTargetSPIRV(),
      Twine(NamePrefix) + ".max.lock.var"); //                        (57), (62)

  // Now emit the final copyout code, which checks if the current thread's max
  // local index is same as the final global max index, and it actually wrote to
  // the variable, and if so, copies the final value of the lastprivate variable
  // back to the original.
  Instruction *BarrierSuccessorInst =
      GeneralUtils::nextUniqueInstruction(ConditionalLPBarrier);
  IRBuilder<> LPCopyoutBuilder(BarrierSuccessorInst);
  auto *FinalMaxLocalIndexLoad = LPCopyoutBuilder.CreateLoad(
      MaxLocalIndex->getAllocatedType(), MaxLocalIndex); //           (24), (64)
  auto *FinalMaxGlobalIndexLoad = LPCopyoutBuilder.CreateLoad(
      MaxGlobalIndex->getValueType(), MaxGlobalIndex); //             (25), (65)
  auto *ShouldThreadDoCopyout = LPCopyoutBuilder.CreateICmpEQ(
      FinalMaxLocalIndexLoad, FinalMaxGlobalIndexLoad,
      NamePrefix + ".copyout.or.not"); //                             (26), (66)
  Instruction *IfShouldDoCopyoutThen = SplitBlockAndInsertIfThen( //  (27), (67)
      ShouldThreadDoCopyout, BarrierSuccessorInst, false, nullptr, DT, LI);

  if (ValInMaxLocalIndex) {
    // Do copyout using ValInMaxLocalIndex instead of LprivINew.
    IRBuilder<> CopyoutBuilder(IfShouldDoCopyoutThen);
    auto *ValInMaxLocalIndexLoad = CopyoutBuilder.CreateLoad(
        ValInMaxLocalIndex->getType()->getPointerElementType(),
        ValInMaxLocalIndex); //                                       (68)
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
  Type *AllocaTy;
  Value *NumElements;
  std::tie(AllocaTy, NumElements, std::ignore) = getItemInfo(RedI);

  // TODO: for a VLA AllocaTy will be just a scalar type, and NumElements
  //       will specify the array size. Right now, VLA reductions are
  //       treated as scalar reductions. We either need to support NumElements
  //       below or ask FE to represent all array reductions (including VLAs)
  //       with array sections (which would have an explicit number of elements
  //       specified).
  bool IsUDR = (RedI->getType() == ReductionItem::WRNReductionUdr);
  bool NeedSrc = IsUDR && (RedI->getInitializer() != nullptr);
  Value *OldV = RedI->getOrig();
  Value *NewV = RedI->getNew();
  if (NeedSrc) {
    IRBuilder<> Builder(InsertPt);
    // For by-refs, do a pointer dereference to reach the actual operand.
    if (RedI->getIsByRef())
      OldV = Builder.CreateLoad(OldV->getType()->getPointerElementType(), OldV);
  }

#if INTEL_CUSTOMIZATION
  if (RedI->getIsF90DopeVector()) {
    genRedAggregateInitOrFini(W, RedI, NewV, NeedSrc ? OldV : nullptr, InsertPt,
                              true, DT, true);
    return;
  }

#endif // INTEL_CUSTOMIZATION
  if (RedI->getIsArraySection() || AllocaTy->isArrayTy()) {
    genRedAggregateInitOrFini(W, RedI, NewV, NeedSrc ? OldV : nullptr, InsertPt,
                              true, DT, true);
    return;
  }
  IRBuilder<> Builder(InsertPt);
  if (IsUDR) {
    genReductionUdrInit(RedI, RedI->getOrig(), NewV, AllocaTy, Builder);
    return;
  }

  assert((VPOUtils::canBeRegisterized(AllocaTy,
                                      InsertPt->getModule()->getDataLayout()) ||
          RedI->getIsComplex()) &&
         "genReductionInit: Expect incoming scalar/complex type.");
  Value *V = genReductionScalarInit(RedI, AllocaTy);
  Builder.CreateStore(V, NewV);
}

// Prepare the empty basic block for the array reduction initialization.
BasicBlock *VPOParoptTransform::createEmptyPrivInitBB(WRegionNode *W) const {
  BasicBlock *EntryBB = W->getEntryBBlock();
  return SplitBlock(EntryBB, EntryBB->getTerminator(), DT, LI);
}

// Prepare the empty basic block for the array reduction update.
BasicBlock *VPOParoptTransform::createEmptyPrivFiniBB(WRegionNode *W,
                                                      bool HonorZTT) {
  BasicBlock *ExitBlock = W->getExitBBlock();
  // If the loop has ztt block, the compiler has to generate the lastprivate
  // update code at the exit block of the loop.
  if (HonorZTT && W->getIsOmpLoop())
    if (BasicBlock *ZttBlock = W->getWRNLoopInfo().getZTTBB()) {
      // FIXME: some prior transformations (e.g. reduction-via-critical
      //        for SPIR-V target) may introduce non-linear code outside
      //        the ZTT and before the exit block, so the loop below
      //        may not find the right block always.
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

      return SplitBlock(LoopExitBB, LoopExitBB->getTerminator(), DT, LI);
    }

  BasicBlock *PrivExitBB =
      SplitBlock(ExitBlock, ExitBlock->getFirstNonPHI(), DT, LI);
  W->setExitBBlock(PrivExitBB);
  return ExitBlock;
}

bool VPOParoptTransform::isArrayReduction(ReductionItem *I) {
#if INTEL_CUSTOMIZATION
  if (I->getIsF90DopeVector())
    return true;
#endif // INTEL_CUSTOMIZATION
  if (I->getIsArraySection())
    return true;

  Type *ElementType = I->getOrig()->getType()->getPointerElementType();
  if (ElementType->isArrayTy())
    return true;

  return false;
}

// Determine if we want to generate fast reduction code and which method will
// be generated (tree reduction only or tree + atomic reduction).
int VPOParoptTransform::checkFastReduction(WRegionNode *W) {
  if (!UseFastReduction)
    return FastReductionNoneMode;

  // TODO: add offloading support for fast reduction. Now only support CPU.
  if (isTargetSPIRV())
    return FastReductionNoneMode;

  bool IsAtomic = UseFastRedAtomic;
  if (IsAtomic) {
    ReductionClause &RedClause = W->getRed();
    for (ReductionItem *RedI : RedClause.items()) {
      // array section and UDR cannot use atomic
      if (isArrayReduction(RedI) ||
          (RedI->getType() == ReductionItem::WRNReductionUdr)) {
        IsAtomic = false;
        break;
      } else {
        // FIXME: enable atomic mode for and, or, max, min, eqv, neqv
        ReductionItem::WRNReductionKind RedKind = RedI->getType();
        if (RedKind == ReductionItem::WRNReductionAnd ||
            RedKind == ReductionItem::WRNReductionOr ||
            RedKind == ReductionItem::WRNReductionMax ||
            RedKind == ReductionItem::WRNReductionMin
#if INTEL_CUSTOMIZATION
            || RedKind == ReductionItem::WRNReductionEqv ||
            RedKind == ReductionItem::WRNReductionNeqv
#endif // INTEL_CUSTOMIZATION
        ) {
          IsAtomic = false;
          break;
        }
        // if type of reduction variable is not supported, atomic cannot be used
        Type *AllocaTy;
        std::tie(AllocaTy, std::ignore, std::ignore) = getItemInfo(RedI);
        if (!(AllocaTy->isIntegerTy() || AllocaTy->isFloatTy() ||
              AllocaTy->isDoubleTy())) {
          IsAtomic = false;
          break;
        }
      }
    }
  }

  FastReductionMode Mode =
      (IsAtomic ? FastReductionAtomicMode : FastReductionTreeOnlyMode);
  LLVM_DEBUG(dbgs() << "Fast reduction is "
                    << ((Mode == FastReductionNoneMode)
                            ? "disabled"
                            : ((Mode == FastReductionTreeOnlyMode)
                                   ? "enabled (tree-like only)"
                                   : "enabled (tree-like and atomic)"))
                    << ".\n");

  return Mode;
}

// Generate fast reduction callback routine.
// Here is example for scalar type.
// C source code:
// void foo() {
//   int i, sum = 0;
//   #pragma omp parallel for reduction(+:sum)
//   for (i=0; i<10; i++) {
//     sum+=i;
//   }
// }
//
// IR:
// define internal void @foo_tree_reduce_4(i8* %dst, i8* %src) {            (1)
// entry:                                                                   (2)
//   %dst.cast = bitcast i8* %dst to %struct.fast_red_t*                    (3)
//   %src.cast = bitcast i8* %src to %struct.fast_red_t*                    (4)
//   %dst.sum = getelementptr inbounds %struct.fast_red_t,
//   %struct.fast_red_t* %dst.cast, i32 0, i32 0                            (5)
//   %src.sum = getelementptr inbounds %struct.fast_red_t,
//   %struct.fast_red_t* %src.cast, i32 0, i32 0                            (6)
//   %0 = load i32, i32* %src.sum, align 4                                 (14)
//   %1 = load i32, i32* %dst.sum, align 4                                 (15)
//   %2 = add i32 %1, %0                                                   (16)
//   store i32 %2, i32* %dst.sum, align 4                                  (17)
//   ret void                                                              (23)
// }
//
//
// Another example for array section.
// C source code:
// void foo() {
//   int i, sum[100] = {0};
// #pragma omp parallel for reduction(+:sum[0:10])
//   for (i=0; i<10; i++) {
//     sum[i]+=i;
//   }
// }
//
// define internal void @foo_tree_reduce_4(i8* %dst, i8* %src) {            (1)
// entry:                                                                   (2)
//   %dst.cast = bitcast i8* %dst to %struct.fast_red_t*                    (3)
//   %src.cast = bitcast i8* %src to %struct.fast_red_t*                    (4)
//   %dst.sum = getelementptr inbounds %struct.fast_red_t,
//   %struct.fast_red_t* %dst.cast, i32 0, i32 0                            (5)
//   %src.sum = getelementptr inbounds %struct.fast_red_t,
//   %struct.fast_red_t* %src.cast, i32 0, i32 0                            (6)
//   %dst.sum.gep = getelementptr inbounds [10 x i32],
//   [10 x i32]* %dst.sum, i32 0, i32 0                                     (7)
//   %src.sum.gep = getelementptr inbounds [10 x i32],
//   [10 x i32]* %src.sum, i32 0, i32 0                                     (8)
//   %0 = getelementptr i32, i32* %dst.sum.gep, i64 10                      (9)
//   %red.update.isempty = icmp eq i32* %dst.sum.gep, %0                   (10)
//   br i1 %red.update.isempty, label %red.update.done,
//   label %red.update.body
//
// red.update.body:
//                                   ; preds = %red.update.body, %entry    (11)
//   %red.cpy.dest.ptr = phi i32* [ %dst.sum.gep, %entry ],
//   [ %red.cpy.dest.inc, %red.update.body ]                               (12)
//   %red.cpy.src.ptr = phi i32* [ %src.sum.gep, %entry ],
//   [ %red.cpy.src.inc, %red.update.body ]                                (13)
//   %1 = load i32, i32* %red.cpy.src.ptr, align 4                         (14)
//   %2 = load i32, i32* %red.cpy.dest.ptr, align 4                        (15)
//   %3 = add i32 %2, %1                                                   (16)
//   store i32 %3, i32* %red.cpy.dest.ptr, align 4                         (17)
//   %red.cpy.dest.inc = getelementptr i32, i32* %red.cpy.dest.ptr, i32 1  (18)
//   %red.cpy.src.inc = getelementptr i32, i32* %red.cpy.src.ptr, i32 1    (19)
//   %red.cpy.done = icmp eq i32* %red.cpy.dest.inc, %0                    (20)
//   br i1 %red.cpy.done, label %red.update.done, label %red.update.body   (21)
//
// red.update.done:
// ; preds = %red.update.body, %entry                                      (22)
//   ret void                                                              (23)
// }
//
RDECL VPOParoptTransform::genFastRedCallback(WRegionNode *W,
                                             StructType *FastRedStructTy) {
  LLVMContext &C = F->getContext();
  Module *M = F->getParent();

  Type *FastRedParams[] = {Type::getInt8PtrTy(C), Type::getInt8PtrTy(C)};
  FunctionType *FastRedFnTy =
      FunctionType::get(Type::getVoidTy(C), FastRedParams, false);
  // Create callback function for tree reduce
  Function *FnFastRed = Function::Create(
      FastRedFnTy, GlobalValue::InternalLinkage,
      F->getName() + "_tree_reduce_" + Twine(W->getNumber()), M); //         (1)
  FnFastRed->setCallingConv(CallingConv::C);
  // If the reduction is in target region, add target.declare attribute for the
  // callback function
  if (hasParentTarget(W))
    FnFastRed->addFnAttr("target.declare", "true");

  auto Arg = FnFastRed->arg_begin();
  Value *DstArg = &*Arg;
  DstArg->setName("dst");
  Arg++;
  Value *SrcArg = &*Arg;
  SrcArg->setName("src");

  BasicBlock *EntryBB =
      BasicBlock::Create(C, "entry", FnFastRed); //         (2)
  DominatorTree DT;
  DT.recalculate(*FnFastRed);

  IRBuilder<> Builder(EntryBB);
  ReturnInst *RetInst =
      Builder.CreateRetVoid(); //                          (23)

  ReductionClause &RedClause = W->getRed();
  assert(!RedClause.empty() && "Empty reduction clauses");

  int ItemIndex = 0;
  for (ReductionItem *RedI : RedClause.items()) {
    Value *Orig = RedI->getOrig();
    Builder.SetInsertPoint(RetInst);
    // Cast i8* to struct.fast_red_t type
    Value *DstVec =
        Builder.CreateBitCast(DstArg, PointerType::getUnqual(FastRedStructTy),
                              DstArg->getName() + Twine(".cast")); //        (3)
    Value *SrcVec =
        Builder.CreateBitCast(SrcArg, PointerType::getUnqual(FastRedStructTy),
                              SrcArg->getName() + Twine(".cast")); //        (4)

    // Get reduction variable from struct.fast_red_t structure
    Value *ValueIndex[2] = {Builder.getInt32(0), Builder.getInt32(ItemIndex++)};
    Value *DstGEP = Builder.CreateInBoundsGEP(
        FastRedStructTy, DstVec, ValueIndex,
        DstArg->getName() + Twine(".") + Orig->getName()); //                (5)
    Value *SrcGEP = Builder.CreateInBoundsGEP(
        FastRedStructTy, SrcVec, ValueIndex,
        SrcArg->getName() + Twine(".") + Orig->getName()); //                (6)

    Value *NumElements = nullptr;
    std::tie(std::ignore, NumElements, std::ignore) = getItemInfo(RedI);

    if (dyn_cast_or_null<ConstantInt>(NumElements)) {
      // Here we generated same types of variables as in genPrivatizationAlloca
      // for New (local reduction variable), which creates an addtional GEP with
      // result type i32* if original type is array section with constant size
      // ([10 x i32]).
      Value *ValueZeros[2] = {Builder.getInt32(0), Builder.getInt32(0)};
      DstGEP = Builder.CreateInBoundsGEP(
          FastRedStructTy->getElementType(ItemIndex - 1), DstGEP, ValueZeros,
          DstArg->getName() + Twine(".") + Orig->getName() +
              Twine(".gep")); // (7)
      SrcGEP = Builder.CreateInBoundsGEP(
          FastRedStructTy->getElementType(ItemIndex - 1), SrcGEP, ValueZeros,
          SrcArg->getName() + Twine(".") + Orig->getName() +
              Twine(".gep")); // (8)
    } else if (NumElements != nullptr) {
      // Create a LOAD with result type i32* if original type is array section
      // with variable length/size (i32 a[0:n]).
      DstGEP = Builder.CreateLoad(
          cast<GEPOperator>(DstGEP)->getResultElementType(), DstGEP,
          DstGEP->getName() + ".load");
      SrcGEP = Builder.CreateLoad(
          cast<GEPOperator>(SrcGEP)->getResultElementType(), SrcGEP,
          SrcGEP->getName() + ".load");
    }

    // For array section with non-constant size, the size is global variable
    // (thread private) and have to be loaded
    Value *NewSize = nullptr;
    if (RedI->getIsArraySection()) {
      if (GlobalVariable *GV = RedI->getArraySectionInfo().getGVSize()) {
        NewSize = Builder.CreateLoad(GV->getValueType(), GV,
                                     GV->getName() + ".load");
      }
    }

    // Generate reduction fini code with DstGEP
    genReductionFini(W, RedI, DstGEP, &*Builder.GetInsertPoint(), &DT,
                     true); // (9~22)

    // Replace New generated by fini code with SrcGEP
    VPOParoptUtils::replaceUsesInFunction(FnFastRed, RedI->getNew(), SrcGEP);
    if (NewSize != nullptr) {
      // Replace size with NewSize loaded above
      VPOParoptUtils::replaceUsesInFunction(
          FnFastRed, RedI->getArraySectionInfo().getSize(), NewSize);
    }
  }

  LLVM_DEBUG(dbgs() << "Generate fast reduction callback routine: "
                    << *FnFastRed << "\n");
  return FnFastRed;
}

// Create struct type and variable for fast reduction
std::pair<StructType *, Value *>
VPOParoptTransform::genFastRedTyAndVar(WRegionNode *W, int FastRedMode) {
  StructType *FastRedStructTy = nullptr;
  Value *FastRedInst = nullptr;

  if (FastRedMode == FastReductionNoneMode)
    return std::make_pair(nullptr, nullptr);

  // Create structure type for fast reduction
  SmallVector<Type *, 9> StructArgTys;
  MaybeAlign MaxAlignment(4);

  ReductionClause &RedClause = W->getRed();
  Instruction *InsertPt = VPOParoptUtils::getInsertionPtForAllocas(W, F, false);
  IRBuilder<> Builder(InsertPt);
  for (ReductionItem *RedI : RedClause.items()) {
    Align OrigAlignment =
        RedI->getOrig()->getPointerAlignment(F->getParent()->getDataLayout());
    MaxAlignment = max(OrigAlignment, MaxAlignment);

    computeArraySectionTypeOffsetSize(*RedI, InsertPt);

    if (RedI->getIsArraySection()) {
      ArraySectionInfo *ArrSecInfo = &RedI->getArraySectionInfo();
      Value *ArrSecSize = ArrSecInfo->getSize();
      if (!isa<ConstantInt>(ArrSecSize)) {
        GlobalVariable *GVArrSecSize =
            VPOParoptUtils::storeIntToThreadLocalGlobal(ArrSecSize, InsertPt,
                                                        "arrsec.size");
        ArrSecInfo->setGVSize(GVArrSecSize);
      }
    }

    Type *ElementType = nullptr;
    Value *NumElements = nullptr;
    std::tie(ElementType, NumElements, std::ignore) = getItemInfo(RedI);
    if (auto *CI = dyn_cast_or_null<ConstantInt>(NumElements)) {
      uint64_t Size = CI->getZExtValue();
      assert(Size > 0 && "Invalid size for new alloca.");
      ElementType = ArrayType::get(ElementType, Size);
    } else if (NumElements != nullptr) {
      // For array section with variable length (non-constant size), pointer to
      // its element type is added into struct.fast_red_t structure, instead of
      // array.
      const DataLayout &DL = InsertPt->getModule()->getDataLayout();
      ElementType = PointerType::get(ElementType, DL.getAllocaAddrSpace());
    }
    StructArgTys.push_back(ElementType);
  }
  LLVMContext &C = F->getContext();
  // Packed structure is needed, so that we can get actual size of this
  // structure, instead of the larger size including padding.
  FastRedStructTy =
      StructType::create(C, StructArgTys, "struct.fast_red_t", true);

  FastRedInst = VPOParoptUtils::genPrivatizationAlloca(
      FastRedStructTy, nullptr, MaybeAlign(MaxAlignment), InsertPt,
      isTargetSPIRV(), "fast_red_struct", llvm::None, llvm::None);

  unsigned Index = 0;
  for (ReductionItem *RedI : RedClause.items()) {
    if (FastRedStructTy->getElementType(Index)->isPointerTy()) {
      // For array section with variable length, private alloca is created and
      // stored into the pointer in struct.fast_red_t structure
      Value *ValueZero = Builder.getInt32(0);
      Value *ValueIndex = Builder.getInt32(Index);
      Value *ValueIndices[2] = {ValueZero, ValueIndex};
      Value *RecInst =
          Builder.CreateInBoundsGEP(FastRedStructTy, FastRedInst, ValueIndices,
                                    RedI->getOrig()->getName() + ".fast_red");
      Value *NewFastRedInst =
          genPrivatizationAlloca(RedI, InsertPt, ".fast_red.alloca");
      Builder.CreateStore(NewFastRedInst, RecInst);
    }
    Index++;
  }

  LLVM_DEBUG(dbgs() << __FUNCTION__
                    << ": Create alloca for fast reduction structure:: "
                    << *FastRedInst << ", type=" << *FastRedStructTy << "\n");

  return std::make_pair(FastRedStructTy, FastRedInst);
}

// Generate copy code for aggregate type from local reduction variable
// (%sum.red.gep) to record variable for fast reduction (%sum.fast_red.gep).
//
// %sum.fast_red.gep.minus.offset = getelementptr i32,
// i32* %sum.fast_red.gep, i64 0                                            (1)
// %sum.fast_red.gep.minus.offset16 = bitcast i32*
// %sum.fast_red.gep.minus.offset to [100 x i32]*                           (2)
// %sum.fast_red.gep.minus.offset16.cast = bitcast [100 x i32]*
// %sum.fast_red.gep.minus.offset16 to i32*                                 (3)
// %sum.fast_red.gep.minus.offset16.cast.plus.offset = getelementptr i32,
// i32* %sum.fast_red.gep.minus.offset16.cast, i64 0                        (4)
// %7 = getelementptr i32, i32*
// %sum.fast_red.gep.minus.offset16.cast.plus.offset, i64 10                (5)
// %fastred.update.isempty = icmp eq i32*
// %sum.fast_red.gep.minus.offset16.cast.plus.offset, %7                    (6)
// br i1 %fastred.update.isempty, label %fastred.update.done,
// label %fastred.update.body                                               (7)
//
// fastred.update.done:
//                               ; preds = %fastred.update.body,
//                               %loop.region.exit                          (8)
//   br label %loop.region.exit.split15
//
// fastred.update.body:
//                               ; preds = %fastred.update.body,
//                               %loop.region.exit                          (9)
//   %fastred.cpy.dest.ptr = phi i32* [
//   %sum.fast_red.gep.minus.offset16.cast.plus.offset,
//   %loop.region.exit ], [ %fastred.cpy.dest.inc, %fastred.update.body ]  (10)
//   %fastred.cpy.src.ptr = phi i32* [ %sum.red.gep, %loop.region.exit ],
//   [ %fastred.cpy.src.inc, %fastred.update.body ]                        (11)
//   %8 = load i32, i32* %fastred.cpy.src.ptr, align 4                     (12)
//   store i32 %8, i32* %fastred.cpy.dest.ptr, align 4                     (13)
//   %fastred.cpy.dest.inc = getelementptr i32, i32* %fastred.cpy.dest.ptr,
//   i32 1                                                                 (14)
//   %fastred.cpy.src.inc = getelementptr i32, i32* %fastred.cpy.src.ptr,
//   i32 1                                                                 (15)
//   %fastred.cpy.done = icmp eq i32* %fastred.cpy.dest.inc, %7            (16)
//   br i1 %fastred.cpy.done, label %fastred.update.done,
//   label %fastred.update.body                                            (17)
//
void VPOParoptTransform::genFastRedAggregateCopy(
    ReductionItem *RedI, Value *Src, Value *Dst, Instruction *InsertPt,
    DominatorTree *DT, bool NoNeedToOffsetOrDerefOldV) {
  IRBuilder<> Builder(InsertPt);
  auto EntryBB = Builder.GetInsertBlock();

  Type *DestElementTy = nullptr;
  Value *DestBegin = nullptr;
  Value *SrcBegin = nullptr;
  Value *NumElements = nullptr;

  // Generate source and destination information for reduction with aggregate
  // type
  genAggrReductionSrcDstInfo(*RedI, Src, Dst, InsertPt, Builder, NumElements,
                             SrcBegin, DestBegin, DestElementTy,
                             NoNeedToOffsetOrDerefOldV);

  assert(DestBegin && "Null destination address for fast reduction copy.");
  assert(DestElementTy && "Null element type for fast reduction copy.");
  assert(NumElements && "Null number of elements for fast reduction copy.");
  assert(SrcBegin && "Null source address for fast reduction copy.");

  // Create copy loop to update variable for fast reduction
  auto DestEnd = Builder.CreateGEP(DestBegin, NumElements); //             (5)
  auto IsEmpty =
      Builder.CreateICmpEQ(DestBegin, DestEnd, "fastred.update.isempty"); // (6)

  auto BodyBB = SplitBlock(EntryBB, InsertPt, DT, LI);
  BodyBB->setName("fastred.update.body"); //                                (9)

  auto DoneBB = SplitBlock(BodyBB, BodyBB->getTerminator(), DT, LI);
  DoneBB->setName("fastred.update.done"); //                                (8)

  EntryBB->getTerminator()->eraseFromParent();
  Builder.SetInsertPoint(EntryBB);
  Builder.CreateCondBr(IsEmpty, DoneBB, BodyBB); //                         (7)

  Builder.SetInsertPoint(BodyBB);
  BodyBB->getTerminator()->eraseFromParent();
  PHINode *DestElementPHI = Builder.CreatePHI(DestBegin->getType(), 2,
                                              "fastred.cpy.dest.ptr"); // (10)
  DestElementPHI->addIncoming(DestBegin, EntryBB);

  PHINode *SrcElementPHI = nullptr;
  if (SrcBegin != nullptr) {
    SrcElementPHI = Builder.CreatePHI(SrcBegin->getType(), 2,
                                      "fastred.cpy.src.ptr"); // (11)
    SrcElementPHI->addIncoming(SrcBegin, EntryBB);
  }

  genFastRedScalarCopy(DestElementPHI, SrcElementPHI, Builder); //       (12~13)

  auto DestElementNext = Builder.CreateConstGEP1_32(
      DestElementPHI, 1, "fastred.cpy.dest.inc"); // (14)
  Value *SrcElementNext = nullptr;
  if (SrcElementPHI != nullptr)
    SrcElementNext = Builder.CreateConstGEP1_32(SrcElementPHI, 1,
                                                "fastred.cpy.src.inc"); // (15)

  auto Done = Builder.CreateICmpEQ(DestElementNext, DestEnd,
                                   "fastred.cpy.done"); // (16)

  Builder.CreateCondBr(Done, DoneBB,
                       BodyBB); //                             (17)
  DestElementPHI->addIncoming(DestElementNext, Builder.GetInsertBlock());
  if (SrcElementPHI != nullptr)
    SrcElementPHI->addIncoming(SrcElementNext, Builder.GetInsertBlock());

  if (DT) {
    DT->changeImmediateDominator(BodyBB, EntryBB);
    DT->changeImmediateDominator(DoneBB, EntryBB);
  }
}

/// Generate copy code for scalar type (only used by fast reduction).
void VPOParoptTransform::genFastRedScalarCopy(Value *Dst, Value *Src,
                                              IRBuilder<> &Builder) {
  Value *V = Builder.CreateLoad(Src->getType()->getPointerElementType(), Src);
  Builder.CreateStore(V, Dst);
}

/// Generate the code to copy local reduction variable to local variable for
/// fast reduction.
void VPOParoptTransform::genFastRedCopy(ReductionItem *RedI, Value *Dst,
                                        Value *Src, Instruction *InsertPt,
                                        DominatorTree *DT,
                                        bool NoNeedToOffsetOrDerefOldV) {
  // Src is either AllocaInst or an AddrSpaceCastInst of AllocaInst.
  assert(GeneralUtils::isOMPItemLocalVAR(Src) &&
         "genFastRedCopy: Expect isOMPItemLocalVAR().");
  Type *AllocaTy;
  Value *NumElements;
  // TODO: OPAQUEPOINTER: Replace the use of getElemenType() with the
  // approproiate logic.
  std::tie(AllocaTy, NumElements) =
      GeneralUtils::getOMPItemLocalVARPointerTypeAndNumElem(
          Src, Src->getType()->getPointerElementType());
  assert(AllocaTy && "genFastRedCopy: item type cannot be deduced.");

  IRBuilder<> Builder(InsertPt);
  // For by-refs, do a pointer dereference to reach the actual operand.
  if (RedI->getIsByRef() && !NoNeedToOffsetOrDerefOldV)
    Dst = Builder.CreateLoad(Dst->getType()->getPointerElementType(), Dst);

#if INTEL_CUSTOMIZATION
  if (RedI->getIsF90DopeVector()) {
    // Allocate space for the pointee array, and initialize the dope vector in
    // the fast reduction struct.
    VPOParoptUtils::genF90DVInitCode(
        RedI, Src, Dst, InsertPt, DT, LI, isTargetSPIRV(),
        /*AllowOverrideInsertPt =*/false,
        /*CheckOrigAllocationBeforeAllocatingNew =*/false);
    // Copy pointee array from Src dope vector to Dst dope vector.
    genFastRedAggregateCopy(RedI, Src, Dst, InsertPt, DT,
                            NoNeedToOffsetOrDerefOldV);
    return;
  }

#endif // INTEL_CUSTOMIZATION
  // TODO: Is this possible to be true? AllocaTy as per the previous code was
  // always PointerTy.
  if (RedI->getIsArraySection() || AllocaTy->isArrayTy()) {
    genFastRedAggregateCopy(RedI, Src, Dst, InsertPt, DT,
                            NoNeedToOffsetOrDerefOldV);
    return;
  }

  genFastRedScalarCopy(Dst, Src, Builder);
}

/// Generate private reduction variable for fast reduction.
Value *VPOParoptTransform::genFastRedPrivateVariable(ReductionItem *RedI,
                                                     unsigned ItemIndex,
                                                     Type *FastRedStructTy,
                                                     Value *FastRedInst,
                                                     Instruction *InsertPt) {
  IRBuilder<> Builder(InsertPt);
  Value *ValueZero = Builder.getInt32(0);
  Value *ValueIndex = Builder.getInt32(ItemIndex);

  // Get private reduction variable from struct.fast_red_t structure
  Value *ValueIndices[2] = {ValueZero, ValueIndex};
  Value *RecInst =
      Builder.CreateInBoundsGEP(FastRedStructTy, FastRedInst, ValueIndices,
                                RedI->getOrig()->getName() + ".fast_red");

  Value *NumElements = nullptr;
  std::tie(std::ignore, NumElements, std::ignore) = getItemInfo(RedI);

  // For array section with constant size N, the private reduction variable type
  // got from structure above is [ N x ElementType ]*, and an additional GEP is
  // created to get ElementType* type as expected, which is same as allocating
  // private in genPrivatizationAlloca.
  // For array section with variable size V, the private reduction variable type
  // got from structure above is ElementType**, and a LOAD is created to get
  // ElementType* type as expected.
  if (dyn_cast_or_null<ConstantInt>(NumElements))
    RecInst = Builder.CreateInBoundsGEP(RecInst, {ValueZero, ValueZero},
                                        RecInst->getName() + Twine(".gep"));
  else if (NumElements != nullptr)
    RecInst = Builder.CreateLoad(
        cast<GEPOperator>(RecInst)->getResultElementType(), RecInst,
        RecInst->getName() + ".load");

  return RecInst;
}

/// Generate reduce blocks for tree and atomic reduction. The basic blocks are
/// organized as below:
/// EntryBB:
///   ...
///   br BeginBB
/// BeginBB:
///   BeginInst
///   ...
///   EndInst
///   br EndBB
/// EndBB:
///   ...
///   br AtomicUpdateEntryBB
/// AtomicUpdateEntryBB:
///   ...
///   br AtomicBeginBB
/// AtomicBeginBB:
///   AtomicBeginInst
///   ...
///   AtomicEndInst
///   br AtomicEndBB
/// AtomicEndBB:
///   ...
///
void VPOParoptTransform::genFastReduceBB(WRegionNode *W,
                                         FastReductionMode FastRedMode,
                                         StructType *FastRedStructTy,
                                         Value *FastRedVar, BasicBlock *EntryBB,
                                         BasicBlock *EndBB) {
  BasicBlock *AtomicUpdateEntryBB = nullptr, *AtomicEndBB = nullptr;
  if (FastRedMode == FastReductionAtomicMode) {
    ReductionClause &RedClause = W->getRed();
    // Create a basic block as the successor of tree reduce block
    AtomicUpdateEntryBB = createEmptyPrivFiniBB(W, !isTargetSPIRV());
    for (ReductionItem *RedI : RedClause.items()) {
      BasicBlock *AtomicBeginBB = createEmptyPrivFiniBB(W, !isTargetSPIRV());
      // Generate reduction fini code to update reduction variable
      genReductionFini(W, RedI, RedI->getOrig(), AtomicBeginBB->getTerminator(),
                       DT);
      // This method may insert a new call to atomic update routine before the
      // last store instruction and erase the store instruction.
      Instruction *AtomicCall = VPOParoptAtomics::handleAtomicUpdateInBlock(
          W, AtomicBeginBB, IdentTy, TidPtrHolder, isTargetSPIRV());
      assert(AtomicCall != nullptr &&
             "No atomic call is generated for fast reduction.");

      Type *AllocaTy;
      std::tie(AllocaTy, std::ignore, std::ignore) = getItemInfo(RedI);

      OptimizationRemark R(DEBUG_TYPE, "FastReductionAtomic", AtomicCall);
      R << ore::NV("Kind", RedI->getOpName()) << " reduction update of type "
        << ore::NV("Type", AllocaTy) << " made atomic";
      ORE.emit(R);
    }
    // AtomicEndBB is created to be used as the insertion point for
    // __kmpc_end_reduce in atomic reduction block.
    AtomicEndBB = createEmptyPrivFiniBB(W, !isTargetSPIRV());
  }

  // Generate fast reduction callback and function calls of __kmpc_reduce
  // and __kmpc_end_reduce
  RDECL FastRedCallback = genFastRedCallback(W, FastRedStructTy);
  VPOParoptUtils::genKmpcReduce(
      W, IdentTy, TidPtrHolder, FastRedVar, FastRedCallback,
      dyn_cast<Instruction>(EntryBB->begin()), EndBB->getTerminator(),
      (AtomicUpdateEntryBB != nullptr)
          ? dyn_cast<Instruction>(AtomicUpdateEntryBB->begin())
          : nullptr,
      (AtomicEndBB != nullptr) ? AtomicEndBB->getTerminator() : nullptr, DT, LI,
      isTargetSPIRV(), ".fast_reduction");
}

// Generate the reduction code for reduction clause.
bool VPOParoptTransform::genReductionCode(WRegionNode *W) {
  bool Changed = false;
  SetVector<Value *> RedUses;

  BasicBlock *EntryBB = W->getEntryBBlock();

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genReductionCode\n");

  ReductionClause &RedClause = W->getRed();
  if (!RedClause.empty()) {
    // Check if fast reduction is enabled and which mode should be used:
    // tree-like reduction only and tree-like + atomic reduction.
    FastReductionMode FastRedMode = (FastReductionMode)checkFastReduction(W);
    bool FastReductionEnabled = FastRedMode != FastReductionNoneMode;

    W->populateBBSet();

    bool NeedsKmpcCritical = false;
    BasicBlock *RedInitEntryBB = nullptr;
    // Insert reduction update at the region's exit block
    // for SPIRV target, so that potential __kmpc_[end_]critical
    // calls are executed the same number of times for all work
    // items. The results should be the same, as long as the reduction
    // variable is initialized at the region's entry block.
    BasicBlock *RedUpdateEntryBB = createEmptyPrivFiniBB(W, !isTargetSPIRV());

    StructType *FastRedStructTy = nullptr;
    Value *FastRedInst = nullptr;
    // Generate struct type and related variable for fast reduction.
    std::tie(FastRedStructTy, FastRedInst) = genFastRedTyAndVar(W, FastRedMode);

    int ItemIndex = 0;
    for (ReductionItem *RedI : RedClause.items()) {
      Value *NewRedInst;
      Value *Orig = RedI->getOrig();

/*
      assert((isa<GlobalVariable>(Orig) || isa<AllocaInst>(Orig)) &&
             "genReductionCode: Unexpected reduction variable");
*/

      // For SPIRV target, if an alloca is created with non-constant size, the
      // insertion point returned from getInsertionPtForAllocas() may appear
      // above the size's defintion point. getInsertPtForAllocas cannot handle
      // this case since we do not outline some regions for SPIRV target.
      Instruction *InsertPt =
          !FastReductionEnabled
              ? &EntryBB->front()
              : VPOParoptUtils::getInsertionPtForAllocas(W, F, false);

      // For fast reduction, the function is called in genFastRedTyAndVar so
      // it's only needed to call here if fast reduction is disabled
      if (!FastReductionEnabled)
        computeArraySectionTypeOffsetSize(*RedI, InsertPt);

      bool UseRecForScalar = ((FastReductionCtrl & 0x1) == 0);
      bool UseRecForArray = ((FastReductionCtrl & 0x2) == 0);
      bool UseRec = ((!isArrayReduction(RedI) && UseRecForScalar) ||
                     (isArrayReduction(RedI) && UseRecForArray));
      if (FastReductionEnabled && UseRec) {
        // Get private reduction variable from structure and set to NewRedInst
        NewRedInst = genFastRedPrivateVariable(
            RedI, ItemIndex++, FastRedStructTy, FastRedInst, InsertPt);
      } else {
        auto AllocaAddrSpace = getPrivatizationAllocaAddrSpace(W, RedI);
        NewRedInst =
            genPrivatizationAlloca(RedI, InsertPt, ".red", AllocaAddrSpace);
      }

      RedI->setNew(NewRedInst);

      Value *ReplacementVal = getClauseItemReplacementValue(RedI, InsertPt);
      genPrivatizationReplacement(W, Orig, ReplacementVal);
#if INTEL_CUSTOMIZATION
      if (RedI->getIsF90DopeVector())
        // For reduction operands, the OpenMP spec guarantees that the original
        // dope vector is allocated, so we can skip emitting a check for it.
        VPOParoptUtils::genF90DVInitCode(
            RedI, InsertPt, DT, LI, isTargetSPIRV(),
            /*AllowOverrideInsertPt =*/true,
            /*CheckOrigAllocationBeforeAllocatingNew =*/false,
            /*StoreNumElementsToGlobal =*/
            FastReductionEnabled); // For fast reduction, save num_elements to a
                                   // global variable, so that there is no need
                                   // to call _f90_dv_size(&dv) to get
                                   // num_elements in the reduction callback.
#endif // INTEL_CUSTOMIZATION

      RedInitEntryBB = createEmptyPrivInitBB(W);
      genReductionInit(W, RedI, RedInitEntryBB->getTerminator(), DT);

      if ((FastReductionEnabled) && !UseRec) {
        BasicBlock *RecInitEntryBB = createEmptyPrivInitBB(W);
        // Generate private variable (RecInst) used by fast reduction callback
        Value *RecInst = genFastRedPrivateVariable(
            RedI, ItemIndex++, FastRedStructTy, FastRedInst,
            RecInitEntryBB->getTerminator());
        BasicBlock *PrevBB = RedUpdateEntryBB->getSinglePredecessor();
        SplitBlock(PrevBB, cast<Instruction>(PrevBB->begin()), DT, LI);

        // And copy private reduction variable (NewRedInst) to private variable
        // (RecInst)
        RedI->setNew(RecInst);
        Value *ReplacementVal =
            getClauseItemReplacementValue(RedI, PrevBB->getTerminator());
        genFastRedCopy(RedI, ReplacementVal, NewRedInst,
                       PrevBB->getTerminator(), DT);
      }

      BasicBlock *BeginBB = createEmptyPrivFiniBB(W, !isTargetSPIRV());
      // FIXME: if NeedsKmpcCritical is true, then for SPIR targets
      //        all reduction update instructions will be wrapped
      //        into a loop. The loop is not needed for reductions
      //        that can be done via atomics or with horizontal
      //        reductions.
      //        We need to have three update sections:
      //          * updates using atomics (no need for the loop),
      //          * updates with __kmpc_critical and horizontal reductions
      //            (no need for the loop),
      //          * updates with __kmpc_critical and the loop.
      //        Note that the first two sections will work with the loop
      //        as well, but this may not be very efficient.
      NeedsKmpcCritical |= genReductionFini(W, RedI, RedI->getOrig(),
                                            BeginBB->getTerminator(), DT);
      LLVM_DEBUG(dbgs() << "genReductionCode: reduced " << *Orig << "\n");
    }

    if (NeedsKmpcCritical) {
      // Wrap the reduction fini code inside a critical region.
      // EndBB is created to be used as the insertion point for
      // end_critical()/end_reduce().
      //
      // This insertion point cannot be W->getExitBBlock()->begin() because
      // we don't want the END DIRECTIVE of the construct to be inside the
      // critical region
      //
      // This insertion point cannot be BeginBB->getTerminator() either, which
      // would work for scalar reduction but not for array reduction, in which
      // case the end_critical() would get emitted before the copy-out loop that
      // the critical section is trying to guard.
      BasicBlock *EndBB = createEmptyPrivFiniBB(W, !isTargetSPIRV());

      if (FastReductionEnabled)
        genFastReduceBB(W, FastRedMode, FastRedStructTy, FastRedInst,
                        RedUpdateEntryBB, EndBB);
      else
        VPOParoptUtils::genKmpcCriticalSection(
            W, IdentTy, TidPtrHolder,
            dyn_cast<Instruction>(RedUpdateEntryBB->begin()),
            EndBB->getTerminator(), DT, LI, isTargetSPIRV(), ".reduction");

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

// Generate code for aligned clause.
bool VPOParoptTransform::genAlignedCode(WRegionNode *W) {
  bool Changed = false;

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genAlignedCode\n");

  AlignedClause &Aligned = W->getAligned();
  if (!Aligned.empty()) {
    W->populateBBSet();

    BasicBlock *Entry = W->getEntryBBlock();
    assert(VPOAnalysisUtils::isBeginDirective(Entry));
    auto *EntryDir = cast<IntrinsicInst>(W->getEntryDirective());

    // Helper lambda that on demand creates a separate block for generating
    // aligned code. This block is a successor of the work region entry block.
    auto GetAlignedBlock = [this, Entry,
                            Block = (BasicBlock *)nullptr]() mutable {
      if (!Block)
        Block = SplitBlock(Entry, Entry->getTerminator(), DT, LI);
      return Block;
    };

    for (const AlignedItem *AI : Aligned) {
      Value *Ptr = AI->getOrig();
      if (!Ptr)
        continue;

      // Replace reference to the 'orig' value with 'null' in the directive
      // since there is no need to keep it in the directive after paropt
      // lowering.
      for (auto &BOI : make_range(std::next(EntryDir->bundle_op_info_begin()),
                                  EntryDir->bundle_op_info_end())) {
        // Get clause ID and check if it is a clause of interest.
        ClauseSpecifier CS(BOI.Tag->getKey());
        if (CS.getId() != QUAL_OMP_ALIGNED)
          continue;

        // Check bundle operand uses and zap the ones matching.
        for (unsigned I = BOI.Begin, E = BOI.End; I < E; ++I) {
          Use &U = EntryDir->getOperandUse(I);
          if (Ptr != U.get())
            continue;
          U.set(Constant::getNullValue(U->getType()));
        }
      }

      // According to the specification 'alignmnent' value is optional and when
      // it is not provided FE sets 'alignment' to 0. If this is the case, set
      // alignment to be equal to the size of the largest vector register.
      int Align = AI->getAlign();
      if (!Align)
        Align = TTI->getRegisterBitWidth(true) / 8;

      // Generate llvm.assume call for the specified value.
      IRBuilder<> Builder(GetAlignedBlock()->getTerminator());
      auto &DL = F->getParent()->getDataLayout();
      CallInst *Call = Builder.CreateAlignmentAssumption(DL, Ptr, Align);

      // And then add it to the assumption cache.
      AC->registerAssumption(Call);

      Changed = true;
    }
  }

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genAlignedCode\n");

  W->resetBBSetIfChanged(Changed);
  return Changed;
}

bool VPOParoptTransform::genNontemporalCode(WRegionNode *W) {
  W->populateBBSet();
  LLVMContext &Context = F->getContext();
  bool Changed = false;
  MDNode *NtmpMD = nullptr;

  // Find all memrefs derived from nontemporal bases in the region and attach
  // !nontemporal metadata to them. The incoming IR may look like:
  //
  //   @llvm.directive.region.entry() [ "QUAL.OMP.NONTEMPORAL"(float* %a.addr)]
  //   %ptridx10 = getelementptr inbounds float, float* %a.addr, i64 %indvars.iv
  //   %bcast = bitcast float* %ptridx10 to i32*
  //   store i32 %conv8, i32* %bcast
  //   call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  //
  for (NontemporalItem *NtmpItem : W->getNontemporal()) {
    SmallVector<Use *, 4> WorkList;
    SmallPtrSet<Use *, 8> VisitedSet;

    auto growWorkList = [W, &WorkList, &VisitedSet](Value *V) {
      for (Use *U = &V->user_begin().getUse(); U; U = U->getNext()) {
        if (VisitedSet.contains(U))
          continue;

        // Consider only those instructions that are inside the region.
        Instruction *I = dyn_cast<Instruction>(U->getUser());
        if (I && !W->contains(I->getParent()))
          continue;

        WorkList.push_back(U);
        VisitedSet.insert(U);
      }
    };

    growWorkList(NtmpItem->getOrig());

    while (!WorkList.empty()) {
      Use *U = WorkList.pop_back_val();
      User *Usr = U->getUser();

      // Check if the nontemporal address is passed as a pointer argument to a
      // store or load instruction.
      if ((isa<StoreInst>(Usr) && U->getOperandNo() == 1) ||
          isa<LoadInst>(Usr)) {
        // Attach !nontemporal metadata to the instruction.
        Instruction *Mrf = cast<Instruction>(Usr);
        if (!NtmpMD) {
          Constant *One = ConstantInt::get(Type::getInt32Ty(Context), 1);
          NtmpMD = MDNode::get(Context, ConstantAsMetadata::get(One));
          Changed = true;
        }
        Mrf->setMetadata(LLVMContext::MD_nontemporal, NtmpMD);
      } else if (isa<GEPOperator>(Usr) || isa<BitCastOperator>(Usr)) {
        // Look through getelementptr and bitcast operators.
        growWorkList(Usr);
      }
    }
  }

  return Changed;
}

// For array sections, generate a base + offset GEP corresponding to the
// section's starting address.
Value *VPOParoptTransform::genBasePlusOffsetGEPForArraySection(
    Value *Orig, const ArraySectionInfo &ArrSecInfo,
    Instruction *InsertBefore) {

  // Example for an array section on a pointer to an array:
  //
  //   static int (*yarrptr)[3][4][5];
  //   #pragma omp parallel for reduction(+:yarrptr[3][1][2:2][1:3])
  //

  // Generated IR for starting address for the above example:
  //
  //   %_yarrptr.load = load [3 x [4 x [5 x i32]]]*, @_yarrptr          ; (1)
  //   %_yarrptr.load.cast = bitcast %_yarrptr.load to i32*             ; (2)
  //   %_yarrptr.load.cast.plus.offset = gep %_yarrptr.load.cast, 211   ; (3)
  //
  //   %_yarrptr.load.cast.plus.offset is the final BeginAddr.

  Value *BeginAddr = Orig;
  IRBuilder<> Builder(InsertBefore);

  if (ArrSecInfo.getBaseIsPointer()) {
    // TODO: OPAQUEPOINTER: we just need to load a pointer value here.
    BeginAddr = Builder.CreateLoad(
        BeginAddr->getType()->getPointerElementType(), BeginAddr,
        BeginAddr->getName() + ".load"); //                               (1)
  }

  assert(BeginAddr && isa<PointerType>(BeginAddr->getType()) &&
         "Illegal Begin Addr for array section.");

  auto *BeginAddrPointerTy = BeginAddr->getType();
  assert(isa<PointerType>(BeginAddrPointerTy) &&
         "Array section begin address must have pointer type.");
  BeginAddr = Builder.CreateBitCast(
      BeginAddr,
      PointerType::get(
          ArrSecInfo.getElementType(),
          cast<PointerType>(BeginAddrPointerTy)->getAddressSpace()),
      BeginAddr->getName() + ".cast"); //                                 (2)
  BeginAddr = Builder.CreateGEP(BeginAddr, ArrSecInfo.getOffset(),
                                BeginAddr->getName() + ".plus.offset"); //(3)

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Gep for array section opnd '";
             Orig->printAsOperand(dbgs()); dbgs() << "':: ";
             BeginAddr->printAsOperand(dbgs()); dbgs() << "'\n");
  return BeginAddr;
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
    DestArrayBegin = AI;
  } else
#if INTEL_CUSTOMIZATION
  if (RedI.getIsF90DopeVector()) {
    VPOParoptUtils::genF90DVReductionInitDstInfo(
        &RedI, AI, DestArrayBegin, DestElementTy, NumElements, InsertPt);
  } else
#endif // INTEL_CUSTOMIZATION
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
// SrcVal is not null for reduction finish code, or reduction init code of UDR
// with user-defined initializer.
void VPOParoptTransform::genAggrReductionSrcDstInfo(
    const ReductionItem &RedI, Value *SrcVal, Value *DestVal,
    Instruction *InsertPt, IRBuilder<> &Builder, Value *&NumElements,
    Value *&SrcArrayBegin, Value *&DestArrayBegin, Type *&DestElementTy,
    bool NoNeedToOffsetOrDerefOldV) {
#if INTEL_CUSTOMIZATION
  if (RedI.getIsF90DopeVector()) {
    VPOParoptUtils::genF90DVReductionSrcDstInfo(
        &RedI, SrcVal, DestVal, SrcArrayBegin, DestArrayBegin, DestElementTy,
        NumElements, InsertPt);
    return;
  }
#endif // INTEL_CUSTOMIZATION

  bool IsArraySection = RedI.getIsArraySection();
  Type *SrcElementTy = nullptr;

  if (!IsArraySection) {
    NumElements =
        VPOParoptUtils::genArrayLength(RedI.getNew(), DestVal, InsertPt,
                                       Builder, DestElementTy, DestArrayBegin);
    auto *DestPointerTy = DestArrayBegin->getType();
    assert(isa<PointerType>(DestPointerTy) &&
           "Reduction destination must have pointer type.");
    DestArrayBegin = Builder.CreateBitCast(
        DestArrayBegin,
        PointerType::get(DestElementTy,
                         cast<PointerType>(DestPointerTy)->getAddressSpace()));

    VPOParoptUtils::genArrayLength(RedI.getNew(), SrcVal, InsertPt, Builder,
                                   SrcElementTy, SrcArrayBegin);
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
  SrcArrayBegin = SrcVal;

  auto *SrcPointerTy = SrcArrayBegin->getType();
  assert(isa<PointerType>(SrcPointerTy) &&
         "Reduction source must have pointer type.");
  auto SrcAddrSpace = cast<PointerType>(SrcPointerTy)->getAddressSpace();
  auto *SrcArrayBeginTy = PointerType::get(SrcElementTy, SrcAddrSpace);
  SrcArrayBegin = Builder.CreateBitCast(SrcArrayBegin, SrcArrayBeginTy);

  // Generated IR for destination starting address for the above example:
  //
  //   %_yarrptr.load = load [3 x [4 x [5 x i32]]]*, @_yarrptr          ; (1)
  //   %_yarrptr.load.cast = bitcast %_yarrptr.load to i32*             ; (2)
  //   %_yarrptr.load.cast.plus.offset = gep %_yarrptr.load.cast, 211   ; (3)
  //
  //   %_yarrptr.load.cast.plus.offset is the final DestArrayBegin.
  if (NoNeedToOffsetOrDerefOldV)
    DestArrayBegin = Builder.CreateBitCast(DestVal, SrcArrayBeginTy);
  else
    DestArrayBegin = VPOParoptTransform::genBasePlusOffsetGEPForArraySection(
        DestVal, ArrSecInfo, InsertPt);
}

void VPOParoptTransform::computeArraySectionTypeOffsetSize(
    Item &I, Instruction *InsertPt) {
  bool IsArraySection = false;
  ArraySectionInfo *ArrSecInfo = nullptr;

  if (auto *RI = dyn_cast<ReductionItem>(&I)) {
    IsArraySection = RI->getIsArraySection();
    ArrSecInfo = &RI->getArraySectionInfo();
  } else if (auto *MI = dyn_cast<MapItem>(&I)) {
    IsArraySection = MI->getIsArraySection();
    ArrSecInfo = &MI->getArraySectionInfo();
  } else
    llvm_unreachable("computeArraySectionTypeOffsetSize: the clause Item "
                     "must be either ReductionItem or MapItem.");

  if (!IsArraySection)
    return;

  assert(ArrSecInfo && "No array section info.");
  computeArraySectionTypeOffsetSize(I.getOrig(), *ArrSecInfo, I.getIsByRef(),
                                    InsertPt);
}

void VPOParoptTransform::computeArraySectionTypeOffsetSize(
    Value *Orig, ArraySectionInfo &ArrSecInfo, bool IsByRef,
    Instruction *InsertPt) {

  const auto &ArraySectionDims = ArrSecInfo.getArraySectionDims();
  if (ArraySectionDims.empty())
    return;

  IRBuilder<> Builder(InsertPt);

  Type *CITy = Orig->getType();
  Type *ElemTy = CITy->getPointerElementType();

  if (IsByRef)
    // Strip away one pointer for by-refs.
    ElemTy = ElemTy->getPointerElementType();

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
    ElemTy = ElemTy->getPointerElementType();
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

    assert(!ArrayDims.empty() &&
           "Unexpected: Is the array section non-contiguous?");

    ArraySizeTillDim *= ArrayDims.pop_back_val();
    ElemTy = cast<ArrayType>(ElemTy)->getElementType();
  }

  // TODO: This assert needs to be updated when UDR support is added.
  assert(!isa<PointerType>(ElemTy) && !isa<ArrayType>(ElemTy) &&
         "Unexpected array section element type.");

  ArrSecInfo.setSize(ArrSecSize);
  ArrSecInfo.setOffset(ArrSecOff);
  ArrSecInfo.setElementType(ElemTy);
  ArrSecInfo.setBaseIsPointer(BaseIsPointer);

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Operand '";
             Orig->printAsOperand(dbgs()); dbgs() << "':: ";
             ArrSecInfo.print(dbgs(), false); dbgs() << "\n");
}

Value *
VPOParoptTransform::getClauseItemReplacementValue(const Item *ClauseI,
                                                  Instruction *InsertPt) {

  assert(ClauseI && "Null clause item.");

  Value *ReplacementVal = nullptr;
  bool IsByref = ClauseI->getIsByRef();
  bool IsArraySection = isa<ReductionItem>(ClauseI) &&
                        cast<ReductionItem>(ClauseI)->getIsArraySection();

  if (IsArraySection)
    ReplacementVal = VPOParoptTransform::getArrSecReductionItemReplacementValue(
        *(cast<ReductionItem>(ClauseI)), InsertPt);
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
    // FIXME: Orig and New may have incompatible addrspaces here.
    //        It is impossible to trigger that situation now, but
    //        we will probably have to deal with it sometime.
    Type *OrigArrayType =
        RedI.getIsByRef()
            ? Orig->getType()->getPointerElementType()
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
  AllocaInst *NewMinusOffsetAddr = Builder.CreateAlloca( //               (2)
      NewMinusOffset->getType(), nullptr, NewMinusOffset->getName() + ".addr");
  Builder.CreateStore(NewMinusOffset, NewMinusOffsetAddr); //             (3)

  return NewMinusOffsetAddr;
}

// Extract the type and size of local Alloca to be created to privatize
// OrigValue.
void VPOParoptTransform::getItemInfoFromValue(Value *OrigValue,
                                              Type *OrigValueElemType,
                                              Type *&ElementType,    // out
                                              Value *&NumElements,   // out
                                              unsigned &AddrSpace) { // out
  assert(OrigValue && "Null input value.");

  ElementType = nullptr;
  NumElements = nullptr;

  if (GeneralUtils::isOMPItemGlobalVAR(OrigValue)) {
    ElementType = OrigValueElemType;
    AddrSpace = cast<PointerType>(OrigValue->getType())->getAddressSpace();
    return;
  }

  assert(GeneralUtils::isOMPItemLocalVAR(OrigValue) &&
         "getItemInfoFromValue: Expect isOMPItemLocalVAR().");

  std::tie(ElementType, NumElements) =
      GeneralUtils::getOMPItemLocalVARPointerTypeAndNumElem(OrigValue,
                                                            OrigValueElemType);
  assert(ElementType && "getItemInfoFromValue: item type cannot be deduced.");

  if (auto *ConstNumElements = dyn_cast<Constant>(NumElements))
    if (ConstNumElements->isOneValue())
      NumElements = nullptr;

  // The final addresspace is inherited from the clause's item.
  AddrSpace = cast<PointerType>(OrigValue->getType())->getAddressSpace();
}

// Extract the type and size of local Alloca to be created to privatize I.
std::tuple<Type *, Value *, unsigned>
VPOParoptTransform::getItemInfo(const Item *I) {
  Type *ElementType = nullptr;
  Value *NumElements = nullptr;
  unsigned AddrSpace = 0;
  assert(I && "Null Clause Item.");

  Value *Orig = I->getOrig();
  Type *OrigElemTy = I->getOrigElemType();
  assert(Orig && "Null original Value in clause item.");

  auto getItemInfoIfArraySection = [I, &ElementType, &NumElements,
                                    &AddrSpace]() -> bool {
    if (const ReductionItem *RedI = dyn_cast<ReductionItem>(I))
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
    getItemInfoFromValue(Orig, OrigElemTy, ElementType, NumElements, AddrSpace);
    assert(ElementType && "Failed to find element type for reduction operand.");

    if (I->getIsByRef()) {
      assert(isa<PointerType>(ElementType) &&
             "Expected a pointer type for byref operand.");
      assert(!NumElements &&
             "Unexpected number of elements for byref pointer.");

      ElementType = ElementType->getPointerElementType();
    }
  }
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Local Element Info for '";
             Orig->printAsOperand(dbgs()); dbgs() << "':: Type: ";
             ElementType->print(dbgs()); if (NumElements) {
               dbgs() << ", NumElements: ";
               NumElements->printAsOperand(dbgs());
             } dbgs() << "\n");
  return std::make_tuple(ElementType, NumElements, AddrSpace);
}

// Generate a private variable version for the local copy of OrigValue.
Value *VPOParoptTransform::genPrivatizationAlloca(
    Value *OrigValue, Type *OrigElemTy, Instruction *InsertPt,
    const Twine &NameSuffix, llvm::Optional<unsigned> AllocaAddrSpace,
    bool PreserveAddressSpace) const {

  assert(OrigValue && "genPrivatizationAlloca: Null input value.");

  Type *ElementType = nullptr;
  Value *NumElements = nullptr;
  unsigned AddrSpace = 0;
  MaybeAlign OrigAlignment =
      OrigValue->getPointerAlignment(InsertPt->getModule()->getDataLayout());

  getItemInfoFromValue(OrigValue, OrigElemTy, ElementType, NumElements,
                       AddrSpace);
  auto *NewVal = VPOParoptUtils::genPrivatizationAlloca(
      ElementType, NumElements, OrigAlignment, InsertPt,
      isTargetSPIRV(), OrigValue->getName() + NameSuffix, AllocaAddrSpace,
      !PreserveAddressSpace ? llvm::None : llvm::Optional<unsigned>(AddrSpace));
  assert(NewVal && "Failed to create local copy.");

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": New Alloca for operand '";
             OrigValue->printAsOperand(dbgs()); dbgs() << "':: ";
             NewVal->printAsOperand(dbgs()); dbgs() << "\n");
  return NewVal;
}

// Generate a private variable version for a ClauseItem I for various
// data-sharing clauses.
Value *VPOParoptTransform::genPrivatizationAlloca(
    Item *I, Instruction *InsertPt,
    const Twine &NameSuffix,
    llvm::Optional<unsigned> AllocaAddrSpace,
    bool PreserveAddressSpace) const {
  assert(I && "Null Clause Item.");

  Value *Orig = I->getOrig();
  assert(Orig && "Null original Value in clause item.");
  MaybeAlign OrigAlignment =
      Orig->getPointerAlignment(InsertPt->getModule()->getDataLayout());

  Type *ElementType = nullptr;
  Value *NumElements = nullptr;
  unsigned AddrSpace = 0;

  std::tie(ElementType, NumElements, AddrSpace) = getItemInfo(I);
  assert(ElementType && "Could not find Type of local element.");

  // If the list item being privatized also appears in an allocate clause,
  // then pass the AllocItem to VPOParoptUtils::genPrivatizationAlloca()
  // so omp_alloc() is called to allocate the private memory.
  AllocateItem *AllocItem = WRegionUtils::getAllocateItem(I);

  auto *NewVal = VPOParoptUtils::genPrivatizationAlloca(
      ElementType, NumElements, OrigAlignment, InsertPt, isTargetSPIRV(),
      Orig->getName() + NameSuffix, AllocaAddrSpace,
      !PreserveAddressSpace ? llvm::None : llvm::Optional<unsigned>(AddrSpace),
      AllocItem);
  assert(NewVal && "Failed to create local copy.");

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": New Alloca for operand '";
             Orig->printAsOperand(dbgs()); dbgs() << "':: ";
             NewVal->printAsOperand(dbgs()); dbgs() << "\n");
  return NewVal;
}

// Return the lexical scope associated with the specified region.
// The llvm.directive beginning the region is expected to have a debug location
// linking the directive to the lexical block scope defining the live range of
// privatized variables.  We don't actually validate the scope is a lexical
// block, because when using llvm-reduce the blocks are often eliminated. So,
// for testing purposes, we can reduce other scope types (typically
// subprograms) as the region scope.
static DIScope *FindDebugScopeForRegion(WRegionNode *W) {
  Instruction *EntryDirective = W->getEntryDirective();
  DILocation *Location = EntryDirective->getDebugLoc().get();
  if (Location == nullptr)
    return nullptr;
  DIScope *RegionScope = Location->getScope();
  return RegionScope;
}

// Generate debug information describing the mapping from PrivValue to the
// privatized NewPrivValue.
static void genPrivatizationDebug(WRegionNode *W,
                                  Value *PrivValue,
                                  Value *NewPrivValue) {
  Module *M = W->getEntryBBlock()->getModule();
  const bool AllowUnresolved = false;
  DICompileUnit *CU = nullptr;
  DIBuilder DIB(*M, AllowUnresolved, CU);
  TinyPtrVector<DbgVariableIntrinsic *> DVIs;

  // Local variables which are privatized into constant or global values are
  // not currently supported.
  if (isa<Constant>(NewPrivValue))
    return;

  DIScope *RegionScope = FindDebugScopeForRegion(W);
  if (!RegionScope)
    return;

  // Follow the PrivValue looking for debug information we can use to describe
  // the newly privatized value.
  Value *Current = PrivValue;
  while (Current) {
    // We don't currently support debugging constants (includes globals) which
    // are privatized into the parallel region.
    if (isa<Constant>(Current))
      break;

    // If the current value has any debug information available then use it.
    DVIs = FindDbgAddrUses(Current);
    if (!DVIs.empty())
      break;

    if (CastInst *CI = dyn_cast_or_null<CastInst>(Current)) {
      if (CI->isNoopCast(M->getDataLayout()))
        Current = CI->getOperand(0);
      else
        Current = nullptr;
    } else {
      // No support for handling this instruction type yet.
      Current = nullptr;
    }
  }

  // If no debug information was found then we're done.
  if (DVIs.empty())
    return;

  // Create a llvm.dbg intrinsic mapping each variable to the newly
  // privatiaized value. Note the NewPrivValue may be in fact be a global
  // value. There can be multiple variables which have storage represented
  // by a single value and we should privatize all of them.
  for (DbgVariableIntrinsic *ODVI : DVIs) {
    DILocalVariable *Variable = ODVI->getVariable();
    DILocation *Location = ODVI->getDebugLoc().get();
    DIExpression *Expression = ODVI->getExpression();

    // Create a new variable for the privatized value. This differs from the
    // original variable in these ways:
    //   1. The privatized variable is not a parameter.
    //   2. The privatized variable is inserted into the region scope.
    const bool AlwaysPreserve = true;
    Variable = DIB.createAutoVariable(
        RegionScope,
        Variable->getName(),
        Variable->getFile(),
        Variable->getLine(),
        Variable->getType(),
        AlwaysPreserve,
        Variable->getFlags(),
        Variable->getAlignInBits());
    LLVM_DEBUG(dbgs() << "[DEBUG] Created Variable: " << *Variable << "\n");

    // The variable is emitted into the scope tree according to the scope
    // associated with the llvm.dbg intrinsic and not the scope associated with
    // the variable. Create a new location in the region scope.
    DebugLoc DL = DILocation::get( // INTEL
        M->getContext(),           // INTEL
        Location->getLine(),
        Location->getColumn(),
        RegionScope,
        Location->getInlinedAt(),
        Location->isImplicitCode());
    Location = DL.get();

    assert(Variable->getScope()->getSubprogram() ==
           Location->getScope()->getSubprogram() &&
           "Variable and source location must be in the same subprogram!");

    // Find a location where we can insert llvm.dbg intrinsics for the
    // privatized value into the parallel region.
    Instruction *Before;
    if (Instruction *I = dyn_cast_or_null<Instruction>(NewPrivValue))
      Before = I->getNextNonDebugInstruction();
    else
      Before = W->getEntryBBlock()->getTerminator();

    // Insert a llvm.dbg intrinsic describing the location of the variable in
    // this parallel region.
    Instruction *NDVI;
    if (isa<DbgDeclareInst>(ODVI) || isa<DbgAddrIntrinsic>(ODVI))
      NDVI = DIB.insertDeclare(NewPrivValue, Variable, Expression, Location,
                               Before);
    else if (isa<DbgValueInst>(ODVI))
      NDVI = DIB.insertDbgValueIntrinsic(NewPrivValue, Variable, Expression,
                                         Location, Before);
    else
      NDVI = nullptr;
    if (NDVI)
      LLVM_DEBUG(dbgs() << "[DEBUG] Created: " << *NDVI << "\n");
  }
}

// Replace the variable with the privatized variable
void VPOParoptTransform::genPrivatizationReplacement(WRegionNode *W,
                                                     Value *PrivValue,
                                                     Value *NewPrivValue) {

  // Find instructions in W that use V
  SmallVector<Instruction *, 8> UserInsts;
  SmallPtrSet<ConstantExpr *, 8> UserExprs;
  if (!WRegionUtils::findUsersInRegion(W, PrivValue, &UserInsts, false,
                                       &UserExprs))
    return; // Found no applicable uses of PrivValue in W's body

  // Replace all USEs of each PrivValue with its NewPrivValue in the
  // W-Region (parallel loop/region/section ... etc.)
  bool FixupIncompatibleAddrSpaces = false;
  SmallVector<Instruction *, 8> FixupInsts;

  if (auto *PrivTy = dyn_cast<PointerType>(PrivValue->getType())) {
    auto *NewPrivTy = dyn_cast<PointerType>(NewPrivValue->getType());
    assert(NewPrivTy &&
           "Trying to replace pointer value with non-pointer value.");
    // Check if the original clause item and its replacement
    // are pointing to the same address space. If they are not, then
    // we need to recursively find all uses of the original value
    // and fixup their types' address spaces.
    //
    // For example, if the original clause item is a global variable
    // 'i32 addrspace(1)* @X', and the replacement is
    // 'i32 addrspace(3)* @X.priv.__local', and there is a use like:
    //   %0 = bitcast i32 addrspace(1)* @X to i8 addrspace(1)*
    //   store i8 10, i8 addrspace(1)* %0
    // we cannot just relink the use of @X in the bitcast.
    // We have to mutate the bitcast's result type to make it consistent.
    //
    // Note that we handle few operations, since most of the time
    // the fixup is expected to stop at an addrspacecast to addrspace(4).
    //
    // For example, this IR is considered to be invalid from the beginning:
    //   %0 = bitcast i32 addrspace(1)* @X to i8 addrspace(1)*
    //   store i8 addrspace(1)* %0, i8 addrspace(1)** %alloca
    // Fixing up such a store instruction will require modifying
    // the %alloca definition and then all its uses. This is too much
    // to handle.
    //
    // FEs must never generate such IR, and should use the following instead:
    //   %0 = addrspacecast i32 addrspace(1)* @X to i32 addrspace(4)*
    //   %1 = bitcast i32 addrspace(4)* %0 to i8 addrspace(4)*
    //   store i8 addrspace(4)* %0, i8 addrspace(4)** %alloca
    // The fixup walk will stop after fixing up the addrspacecast,
    // and the IR will be valid after that.
    if (!VPOParoptUtils::areCompatibleAddrSpaces(PrivTy->getAddressSpace(),
                                                 NewPrivTy->getAddressSpace(),
                                                 isTargetSPIRV()))
      FixupIncompatibleAddrSpaces = true;
  }

  auto InstMayNeedAddrSpaceFixup =
      [FixupIncompatibleAddrSpaces](Instruction *I) {
        if (!FixupIncompatibleAddrSpaces)
          return false;

        // This is a list of users that we are able to fixup.

        switch (I->getOpcode()) {
        case Instruction::GetElementPtr:
          return true;
        case Instruction::BitCast:
          if (isa<PointerType>(I->getType()))
            return true;
          LLVM_FALLTHROUGH;
        default:
          return false;
        }
      };

  while (!UserInsts.empty()) {
    Instruction *UI = UserInsts.pop_back_val();
    UI->replaceUsesOfWith(PrivValue, NewPrivValue);

    // In case we relinked the operand we have to check
    // whether the result type has to be mutated.
    if (InstMayNeedAddrSpaceFixup(UI))
      FixupInsts.push_back(UI);

    if (UserExprs.empty())
      continue;

    // If PrivValue is a ConstantExpr, its uses could be in ConstantExprs
    SmallVector<Instruction *, 2> NewInstArr;
    GeneralUtils::breakExpressions(UI, &NewInstArr, &UserExprs);
    for (Instruction *NewInst : NewInstArr)
      UserInsts.push_back(NewInst);
  }

  while (!FixupInsts.empty()) {
    Instruction *I = FixupInsts.pop_back_val();
    bool CollectUsers = false;

    switch (I->getOpcode()) {
    case Instruction::GetElementPtr: {
      auto *GEPI = cast<GetElementPtrInst>(I);
      unsigned SrcAS = GEPI->getAddressSpace();
      auto *DstTy = cast<PointerType>(GEPI->getType());
      unsigned DstAS = DstTy->getAddressSpace();
      // Mutate the result type to match the address space of the operand.
      if (SrcAS != DstAS) {
        // TODO: OPAQUEPOINTER: Use the appropriate API for getting PointerType
        // to a specific AddressSpace. The API currently needs the Element Type
        // as well.
        GEPI->mutateType(DstTy->getPointerElementType()->getPointerTo(SrcAS));
        CollectUsers = true;
      }
      break;
    }
    case Instruction::BitCast: {
      auto *BCI = cast<BitCastInst>(I);
      auto *SrcTy = cast<PointerType>(BCI->getSrcTy());
      unsigned SrcAS = SrcTy->getAddressSpace();
      auto *DstTy = cast<PointerType>(BCI->getDestTy());
      unsigned DstAS = DstTy->getAddressSpace();
      // Mutate the result type to match the address space of the operand.
      if (SrcAS != DstAS) {
        // TODO: OPAQUEPOINTER: Use the appropriate API for getting PointerType
        // to a specific AddressSpace. The API currently needs the Element Type
        // as well.
        BCI->mutateType(DstTy->getPointerElementType()->getPointerTo(SrcAS));
        CollectUsers = true;
      }
      break;
    }
    default:
      llvm_unreachable("Unexpected instruction to fix up.");
    }

    if (CollectUsers)
      for (User *U : I->users()) {
        auto *UI = cast<Instruction>(U);
        assert(W->contains(UI->getParent()) &&
               "Definition from a region is used outside of the region.");
        if (InstMayNeedAddrSpaceFixup(UI))
          FixupInsts.push_back(UI);
      }
  }

  if (W->getIsTask())
    // For tasks, NewPrivInst is not an alloca, but a GEP computed inside the
    // region, after entry directive. So its uses in the entry directive would
    // cause a use-before-def. We need to remove it from the directive.
    resetValueInOmpClauseGeneric(W, NewPrivValue);

  genPrivatizationDebug(W, PrivValue, NewPrivValue);

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

  assert((isa<WRNVecLoopNode>(W) ||
          std::none_of(LrClause.items().begin(), LrClause.items().end(),
                       [](LinearItem *LI) { return LI->getIsIV(); })) &&
         "Unexpected: 'Linear:IV' found on a non-SIMD construct.");

  assert(LinearFiniBB && "genLinearCode: Null LinearFiniBB.");

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genLinearCode\n");

  W->populateBBSet();
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *LoopBodyBB = nullptr;
  Value *NewLinearVar = nullptr;
  Value *LinearStartVar = nullptr;

  // Create empty BBlocks for capturing linear operand's starting value.
  BasicBlock *LinearInitBB = createEmptyPrivInitBB(W);
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
    Type *NewVTy = NewLinearVar->getType();
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
      Orig = new LoadInst(NewVTy, Orig, "", NewLinearInsertPt);

    NewVTy = NewVTy->getPointerElementType();

    // (B) Capture value of linear variable before entering the loop
    LoadInst *LoadOrig = CaptureBuilder.CreateLoad(NewVTy, Orig);       // (3)
    CaptureBuilder.CreateStore(LoadOrig, LinearStartVar);               // (4)

    // (C) Initialize the linear variable using closed form inside the loop
    // body: %y.linear = %y.linear.start + %omp.iv * %step

    Value *LinearStart = InitBuilder.CreateLoad(NewVTy, LinearStartVar); // (5)
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
    LoadInst *FiniLoad = FiniBuilder.CreateLoad(NewVTy, NewLinearVar);  // (12)
    FiniBuilder.CreateStore(FiniLoad, Orig);                            // (13)

    LLVM_DEBUG(dbgs() << "genLinearCode: generated " << *Orig << "\n");
  }

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genLinearCode\n");

  W->resetBBSet(); // CFG changed; clear BBSet
  return true;
}

// Emit privatization and copyin/copyout code for linear/linear:iv clause
// operands on SIMD directives.
// ----------------------------------+----------------------------------------
//          Before                   |        After
// ----------------------------------+----------------------------------------
//   %0 = ["DIR.OMP.PARALLEL"]       |    %0 = ["DIR.OMP.PARALLEL"]
//                                  (1)   %x.linear = alloca i32
//   ...                             |    ...
//                                   |
//                                  (2)   %x.val = load i32, i32* %x
//                                  (3)   store i32 %x.val, i32* %x.linear
//   %1 = ["DIR.OMP.SIMD"]           |    %1 = ["DIR.OMP.SIMD"]
//        ["LINEAR"(i32 *%x, i32 1) (4)        ["LINEAR"(i32 %x.linear, i32 1)
//                                   |
//   ...                             |    ...
//        %x                        (4)        %x.linear
//   ...                             |    ...
//                                  (5)   %x.lr.val = load i32, i32* %x.linear
//                                  (6)   store i32 %x.lr.val, i32* %x
//   ["DIR.OMP.END.SIMD"]            |    ["DIR.OMP.END.SIMD"]
//                                   |
// ----------------------------------+----------------------------------------
bool VPOParoptTransform::genLinearCodeForVecLoop(WRegionNode *W,
                                                 BasicBlock *LinearFiniBB) {
  if (!isa<WRNVecLoopNode>(W))
    return false;

  LinearClause &LrClause = W->getLinear();
  if (LrClause.empty())
    return false;

  assert(LinearFiniBB && "genLinearCodeForVecLoop: Null LinearFiniBB.");

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genLinearCodeForVecLoop\n");

  Instruction *EntryDirective = W->getEntryDirective();
  Instruction *LinearAllocaInsertPt =
      VPOParoptUtils::getInsertionPtForAllocas(W, F);

  // Create separate block for linear initialization code.
  BasicBlock *InitBB = W->getEntryBBlock();
  BasicBlock *EntryBB = SplitBlock(InitBB, EntryDirective, DT, LI);
  W->setEntryBBlock(EntryBB);
  W->populateBBSet(/*Always=*/true);

  // Create IRBuilders for initial copyin and final copyout to/from the local
  // copy of linear.
  IRBuilder<> InitBuilder(InitBB->getTerminator());
  IRBuilder<> FiniBuilder(LinearFiniBB->getTerminator());

  for (LinearItem *LinearI : LrClause.items()) {
    Value *Orig = LinearI->getOrig();
    bool IsLinearIV = LinearI->getIsIV();

    // Allocate the private copy before the region's entry directive.
    Value *NewLinearVar = genPrivatizationAlloca( //                       (1)
        LinearI, LinearAllocaInsertPt, IsLinearIV ? ".linear.iv" : ".linear");
    LinearI->setNew(NewLinearVar);

    // Replace original var with the new var inside the region.
    Value *ReplacementVal =
        getClauseItemReplacementValue(LinearI, EntryDirective);
    genPrivatizationReplacement(W, Orig, ReplacementVal); //               (4)

    Type *NewVTy = NewLinearVar->getType();
    // For by-refs, do a pointer dereference to reach the actual operand.
    if (LinearI->getIsByRef())
      Orig = InitBuilder.CreateLoad(NewVTy, Orig);

    NewVTy = NewVTy->getPointerElementType();
    // For LINEAR:IV, the initialization using closed-form is inserted in each
    // iteration of the loop by the frontend, so we don't need to do the
    // "firstprivate copyin" to the privatized linear var.
    if (!IsLinearIV) {
      // Capture value of linear variable before the entry directive
      LoadInst *LoadOrig = InitBuilder.CreateLoad(NewVTy, Orig); //        (2)
      InitBuilder.CreateStore(LoadOrig, NewLinearVar);   //                (3)
    }

    // Check if Linear.IV is a live-out.
    bool IsLiveOut = false;
    for (User *U : Orig->users()) {
      IntrinsicInst *II = dyn_cast<IntrinsicInst>(U);
      if (II && II->getIntrinsicID() == Intrinsic::directive_region_entry)
        continue;
      else if (auto *SI = dyn_cast<StoreInst>(U)) {
        // Linear.IV is in use if it is a ValueOperand of a store
        if (Orig == SI->getValueOperand()) {
          IsLiveOut = true;
          break;
        }
      }
      else if (auto *BI = dyn_cast<BitCastInst>(U)) {
        // When Linear.IV is in use of a bitcast instruction, be conservative,
        // we count and check the number of users of the bicast instruction.
        for (User *U : BI->users())
          if (isa<Instruction>(U)) {
            IsLiveOut = true;
            break;
          }
        if (IsLiveOut)
          break;
      }
      else {
        IsLiveOut = true;
        break;
      }
    }

    // Insert the final copy-out from the private copies to the original if
    // it is a live-out.
    if (IsLiveOut) {
      LLVM_DEBUG(dbgs() << *Orig << ": is a linear live-out value. " << "\n");
      LoadInst *FiniLoad = FiniBuilder.CreateLoad(NewVTy, NewLinearVar); // (5)
      FiniBuilder.CreateStore(FiniLoad, Orig); //                           (6)
    }

    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": handled '" << *Orig << "'\n");
  }

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genLinearCodeForVecLoop\n");
  W->resetBBSet(); // CFG changed; clear BBSet
  return true;
}

// For a taskloop, the NormalizedUB variable is added to Firstprivate Clause.
// This enables the existing code to create a variable in kmpc_struct_t
// structure for the NormalizedUB variable, to pass to an outlined function.
bool VPOParoptTransform::addFirstprivateForNormalizedUB(WRegionNode *W) {
  if (!isa<WRNTaskloopNode>(W))
    return false;
  WRNLoopInfo L = W->getWRNLoopInfo();
  unsigned NumNormUB = L.getNormUBSize();
  for (unsigned i = 0; i < NumNormUB; i++) {
    Value *V = L.getNormUB(i);
    FirstprivateClause &FP = W->getFpriv();
    FP.add(V);
    LLVM_DEBUG(dbgs() << "addFirstprivateForNormalizedUB: Created Firstprivate "
                         "Clause for NormalizedUB Var: "
                      << *V << "\n");
  }
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
#if INTEL_CUSTOMIZATION
        // For F90 dope vectors which are firstprivate, ifx frontend emits a
        // map in addition to a firstprivate clause. We need to honor both.
        if (!FprivI->getIsF90DopeVector())
#endif // INTEL_CUSTOMIZATION
        // In some cases, clang may put a variable both into map and
        // firstprivate clauses. Usually, in such cases, we do not need to
        // generate any for firstprivate clause, as all the data movement
        // is handled by the map clause processing.
        //
        // But for NONPODs, we need to emit constructors/destructors for them
        // while handling the firstprivate clause (at least for the host
        // fallback function). Whether these are emitted for target compilation
        // is determined based on EmitTargetFPCtorDtors.
        //
        //   Class C;
        //   void foo() {
        //     C c1;
        //     #pragma omp target firstprivate(c1)
        //     ...
        //   }
        //
        // The generated region will look like this:
        //   call token @llvm.directive.region.entry() [
        //   "DIR.OMP.TARGET"(), ...,
        //   "QUAL.OMP.FIRSTPRIVATE:NONPOD"(%class.C* %c1,
        //     void (%class.C*, %class.C*)* @_ZTS1C.omp.copy_constr,
        //     void (%class.C*)* @_ZTS1C.omp.destr),
        //   "QUAL.OMP.MAP.TO"(%class.C* %c1, %class.C* %c1, i64 4, i64 161) ]
        //
        if (!FprivI->getIsNonPod() ||
            (isTargetSPIRV() && !EmitTargetFPCtorDtors))
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
        if (!ForTask)
          NewPrivInst = genPrivatizationAlloca(FprivI, InsertPt, ".fpriv");
        else {
          NewPrivInst = FprivI->getNew(); // Use the task thunk directly
          InsertPt =
              cast<Instruction>(NewPrivInst)->getParent()->getTerminator();
        }
        FprivI->setNew(NewPrivInst);
        Value *ReplacementVal = getClauseItemReplacementValue(FprivI, InsertPt);
        genPrivatizationReplacement(W, Orig, ReplacementVal);
#if INTEL_CUSTOMIZATION
        if (!ForTask && FprivI->getIsF90DopeVector())
          VPOParoptUtils::genF90DVInitCode(FprivI, InsertPt, DT, LI,
                                           isTargetSPIRV());
#endif // INTEL_CUSTOMIZATION
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
#if INTEL_CUSTOMIZATION
        if (FprivI->getIsF90DopeVector())
          FprivI->setF90DVDataAllocationPoint(
              LprivI->getF90DVDataAllocationPoint());
#endif // INTEL_CUSTOMIZATION
      }

      if (!ForTask) {
        // Copy the original data to the firstprivate new version.
        // Note: We must run lastprivate codegen before firstprivate codegen.
        // Otherwise the lastprivate var replacement will mess up the
        // copy instructions.

        PrivInitEntryBB = createEmptyPrivInitBB(W);

        // For the given FIRSTPRIVATE item, check if the data may fit
        // a pointer type, so that it may be passed by value to the region's
        // outlined function. If the check passes, then the first returned
        // type is the integer type of the same size as the item's data type,
        // and the second returned type is the intptr_t type for the current
        // compilation target.
        auto GetPassAsScalarSizeInBits = [](Function *F, Value *Item) ->
            std::tuple<Type *, Type *> {
          if (!Item || !Item->getType()->isPointerTy())
            return std::make_tuple(nullptr, nullptr);

          // There is no clean way to recognize VLAs now.
          // They will look like this in IR:
          //   %vla = alloca i32, i64 %X
          //   ... "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %vla)
          //
          // It does look like a scalar firstprivate, but it is not,
          // so we have to recognize this and bail out.
          if (auto *NewAI =
              dyn_cast<AllocaInst>(Item->stripPointerCasts())) {
            bool IsVla = true;
            Value *Size = NewAI->getArraySize();
            if (auto *ConstSize = dyn_cast<ConstantInt>(Size))
              if (ConstSize->isOneValue())
                IsVla = false;

            if (IsVla)
              return std::make_tuple(nullptr, nullptr);
          }

          auto *ValueTy = Item->getType()->getPointerElementType();
          TypeSize ValueTySize = ValueTy->getPrimitiveSizeInBits();

          if (ValueTy->isPointerTy() ||
              // TODO: check if we can handle small structures as well.
              //       ValueTySize is zero for them here, and
              //       they are not isFirstClassType() (i.e. it is not
              //       a valid Value type).
              !ValueTy->isFirstClassType() ||
              ValueTySize.isScalable())
            return std::make_tuple(nullptr, nullptr);

          uint64_t ValueSize = ValueTySize.getFixedSize();

          // WARNING: in case host and target use differently sized
          //          pointers this optimization may be illegal
          //          for target regions. At the same time, libomptarget
          //          probably does not support such configurations anyway.
          unsigned PtrSize =
              F->getParent()->getDataLayout().getPointerSizeInBits(0);
          Type *IntPtrTy = Type::getIntNTy(Item->getContext(), PtrSize);

          if (ValueSize != 0 && ValueSize <= PtrSize)
            return std::make_tuple(
                Type::getIntNTy(Item->getContext(), ValueSize), IntPtrTy);

          return std::make_tuple(nullptr, nullptr);
        };

        Type *ValueIntTy = nullptr;
        Type *IntPtrTy = nullptr;
        // TODO: do the same thing for NORMALIZED.UB item(s).
        if (OptimizeScalarFirstprivate &&
            // FIXME: figure out whether we can do anything
            //        for getInMap() and getIsByRef() firstprivates.
            !FprivI->getInMap() && !FprivI->getIsByRef()) {

          std::tie(ValueIntTy, IntPtrTy) =
              GetPassAsScalarSizeInBits(F, NewPrivInst);
        }

        if (ValueIntTy && IntPtrTy) {
          // Prepare the IR so that the item is passed by value.
          // Original IR:
          // RegPredBlock:
          //   br label %EntryBB
          //
          // EntryBB:
          //   %x.fpriv = alloca float
          //   %0 = call token @llvm.directive.region.entry() [
          //       "DIR.OMP.PARALLEL"(),
          //       "QUAL.OMP.FIRSTPRIVATE"(float* %x.fpriv), ...
          //   br label %PrivInitEntryBB
          //
          // PrivInitEntryBB:
          //   ...
          //
          // New IR:
          // RegPredBlock:
          //   %x.val = load float, float* %x, align 4
          //   %x.val.bcast = bitcast float %x.val to i32
          //   %x.val.bcast.zext = zext i32 %x.val.bcast to i64
          //   br label %EntryBB
          //
          // EntryBB:
          //   %x.fpriv = alloca float
          //   %0 = call token @llvm.directive.region.entry() [
          //       "DIR.OMP.PARALLEL"(),
          //       "QUAL.OMP.FIRSTPRIVATE"(float* %x.fpriv), ...
          //   br label %PrivInitEntryBB
          //
          // PrivInitEntryBB:
          //   %x.val.bcast.zext.trunc = trunc i64 %x.val.bcast.zext to i32
          //   %x.val.bcast.zext.trunc.bcast =
          //       bitcast i32 %x.val.bcast.zext.trunc to float
          //   store float %x.val.bcast.zext.trunc.bcast, float* %x.fpriv
          //   ...
          IRBuilder<> PredBuilder(RegPredBlock->getTerminator());
          Value *NewArg = PredBuilder.CreateLoad(
              Orig->getType()->getPointerElementType(),
              Orig, Orig->getName() + Twine(".val"));
          NewArg = PredBuilder.CreateBitCast(NewArg, ValueIntTy,
              NewArg->getName() + Twine(".bcast"));
          NewArg = PredBuilder.CreateZExt(NewArg, IntPtrTy,
              NewArg->getName() + Twine(".zext"));
          FprivI->setOrig(NewArg);

          IRBuilder<> RegBuilder(PrivInitEntryBB->getTerminator());
          NewArg = RegBuilder.CreateTrunc(NewArg, ValueIntTy,
              NewArg->getName() + Twine(".trunc"));
          NewArg = RegBuilder.CreateBitCast(
              NewArg, NewPrivInst->getType()->getPointerElementType(),
              NewArg->getName() + Twine(".bcast"));
          RegBuilder.CreateStore(NewArg, NewPrivInst);
        } else if (!NewPrivInst ||
            // Note that NewPrivInst will be nullptr, if the variable
            // is also lastprivate.
            FprivI->getIsByRef() || !NewPrivInst->getType()->isPointerTy() ||
            !NewPrivInst->getType()->getPointerElementType()->isPointerTy()) {
          // Copy the firstprivate data from the original version to the
          // private copy. In the case of lastprivate, this is the lastprivate
          // copy. The lastprivate codegen will replace all original vars
          // with the lastprivate copy.
          Instruction *FprivInitInsertPt = PrivInitEntryBB->getTerminator();
#if INTEL_CUSTOMIZATION
          if (FprivI->getIsF90DopeVector())
            if (Instruction *AllocPt = FprivI->getF90DVDataAllocationPoint())
              // Firstprivate initialization needs to be done after
              // F90 DV's local data allocation is done.
              FprivInitInsertPt = AllocPt;
#endif // INTEL_CUSTOMIZATION
          genFprivInit(FprivI, FprivInitInsertPt);
        } else {
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
          auto *Load = PredBuilder.CreateLoad(
              Orig->getType()->getPointerElementType(), Orig);
          IRBuilder<> RegBuilder(PrivInitEntryBB->getTerminator());
          RegBuilder.CreateStore(Load, FprivI->getNew());

          FprivI->setOrig(Load);
          FprivI->setIsPointer(true);
        }
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
      if (!ForTask) {
        Instruction *AllocaInsertPt =
            IsVecLoop ? VPOParoptUtils::getInsertionPtForAllocas(W, F)
                      : InsertPt;
        NewPrivInst = genPrivatizationAlloca(LprivI, AllocaInsertPt, ".lpriv");
      } else {
        NewPrivInst = LprivI->getNew();
        InsertPt = cast<Instruction>(NewPrivInst)->getParent()->getTerminator();
      }
      LprivI->setNew(NewPrivInst);

      Value *ReplacementVal = getClauseItemReplacementValue(LprivI, InsertPt);
      genPrivatizationReplacement(W, Orig, ReplacementVal);
#if INTEL_CUSTOMIZATION
      if (!ForTask && LprivI->getIsF90DopeVector())
        VPOParoptUtils::genF90DVInitCode(LprivI, InsertPt, DT, LI,
                                         isTargetSPIRV());
#endif // INTEL_CUSTOMIZATION

      // Emit constructor call for lastprivate var if it is not also a
      // firstprivate (in which case the firstprivate init emits a cctor).
      if ((LprivI->getInFirstprivate() == nullptr) &&
          (LprivI->getConstructor() != nullptr))
        genPrivatizationInitOrFini(LprivI, LprivI->getConstructor(), FK_Ctor,
                                   NewPrivInst, nullptr,
                                   cast<Instruction>(NewPrivInst), DT);

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
          LprivI->getInFirstprivate() == nullptr)
        genPrivatizationInitOrFini(LprivI, LprivI->getDestructor(), FK_Dtor,
                                   LprivI->getNew(), nullptr,
                                   W->getExitBBlock()->getTerminator(), DT);

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

  // Create a BB before ExitBB in which to insert dtor calls.
  // FIXME: the destructors must be called outside of ZTT check,
  //        since the constructors are called before ZTT.
  BasicBlock *NewBB = createEmptyPrivFiniBB(W, !isTargetSPIRV());
  Instruction* InsertBeforePt = NewBB->getTerminator();

  // Destructors for privates
  if (W->canHavePrivate())
    for (PrivateItem *PI : W->getPriv().items())
      if (auto *Dtor = PI->getDestructor())
        genPrivatizationInitOrFini(PI, Dtor, FK_Dtor, PI->getNew(), nullptr,
                                   InsertBeforePt, DT);

  // Destructors for firstprivates
  if (W->canHaveFirstprivate())
    for (FirstprivateItem *FI : W->getFpriv().items())
      if (auto *Dtor = FI->getDestructor())
        if (auto *FNew = FI->getNew()) // getNew() can be null for firstprivate
                                       // on target constructs based on
                                       // EmitTargetFPCtorDtors.
          genPrivatizationInitOrFini(FI, Dtor, FK_Dtor, FNew, nullptr,
                                     InsertBeforePt, DT);

  // Destructors for lastprivates (that are not also firstprivate)
  if (W->canHaveLastprivate())
    for (LastprivateItem *LI : W->getLpriv().items())
      if (LI->getInFirstprivate() == nullptr &&
          (LI->getDestructor() != nullptr))
        genPrivatizationInitOrFini(LI, LI->getDestructor(), FK_Dtor,
                                   LI->getNew(), nullptr, InsertBeforePt, DT);
      // else do nothing; dtor already emitted for Firstprivates above

  if (W->canHaveReduction())
    for (ReductionItem *RI : W->getRed().items())
      VPOParoptUtils::genDestructorCall(RI->getDestructor(), RI->getNew(),
                                        InsertBeforePt);
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
//
//   If CastVAddrToAddrSpaceGeneric is true, then cast %v.addr to
//   address space 4 (generic) before doing the store/load.
Value *VPOParoptTransform::replaceWithStoreThenLoad(
    WRegionNode *W, Value *V, Instruction *InsertPtForStore,
    bool InsertLoadInBeginningOfEntryBB,
    bool SelectAllocaInsertPtBasedOnParentWRegion,
    bool CastToAddrSpaceGeneric) {

  // Find instructions in W that use V
  SmallVector<Instruction *, 8> UserInsts;
  SmallPtrSet<ConstantExpr *, 8> UserExprs;
  WRegionUtils::findUsersInRegion(W, V, &UserInsts,
                                  !InsertLoadInBeginningOfEntryBB, &UserExprs);

  Instruction *AllocaInsertPt =
      SelectAllocaInsertPtBasedOnParentWRegion
          ? VPOParoptUtils::getInsertionPtForAllocas(W, F, true)
          : W->getEntryBBlock()->getParent()->getEntryBlock().getTerminator();

  IRBuilder<> AllocaBuilder(AllocaInsertPt);
  Value *VAddr = //                                       (1)
      AllocaBuilder.CreateAlloca(V->getType(), nullptr, V->getName() + ".addr");

  IRBuilder<> StoreBuilder(InsertPtForStore);
  if (CastToAddrSpaceGeneric) {
    Value *VAddrCasted = StoreBuilder.CreatePointerBitCastOrAddrSpaceCast(
        VAddr, V->getType()->getPointerTo(vpo::ADDRESS_SPACE_GENERIC),
        VAddr->getName() + ".ascast");
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Casted '";
               VAddr->printAsOperand(dbgs()); dbgs() << "' to '";
               VAddrCasted->printAsOperand(dbgs()); dbgs() << "'.\n";);
    VAddr = VAddrCasted;
  }

  StoreBuilder.CreateStore(V, VAddr); //                  (2)

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Stored '"; V->printAsOperand(dbgs());
             dbgs() << "' to '"; VAddr->printAsOperand(dbgs());
             dbgs() << "'.\n";);

  if (UserInsts.empty())
    return VAddr; // Nothing to replace inside the region. Just capture the
                  // address of V to VAddr and return it.

  BasicBlock *EntryBB = W->getEntryBBlock();
  Instruction *InsertPtForLoad = InsertLoadInBeginningOfEntryBB
                                     ? EntryBB->getFirstNonPHI()
                                     : EntryBB->getTerminator();
  IRBuilder<> BuilderInner(InsertPtForLoad);
  LoadInst *VRenamed = BuilderInner.CreateLoad(V->getType(), VAddr); // (3)
  if (!InsertLoadInBeginningOfEntryBB)
    // InstCombine may transform:
    //   %1 = load float*, float** %.addr
    //   store float* %1, float** %X
    // into:
    //   %1 = bitcast float** %.addr to i64*
    //   %2 = load i64, i64* %1
    //   %3 = bitcast float** %X to i64*
    //   store i64 %2, i64* %3
    //
    // In this case VRenamed will be the %2 load of type i64,
    // VOrig will have type float*, so we will not be able
    // to restore the operand with just BitCasting float*
    // value to i64. We could have used IntToPtr, but
    // this will never be optimized. So we mark the load
    // as volatile to prevent InstCombine transformation
    // for this load.
    VRenamed->setVolatile(true);

  VRenamed->setName(V->getName());

  // Replace uses of V with VRenamed
  while (!UserInsts.empty()) {
    Instruction *User = UserInsts.pop_back_val();
    User->replaceUsesOfWith(V, VRenamed);

    if (UserExprs.empty())
      continue;

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
    GeneralUtils::breakExpressions(User, &NewInstArr, &UserExprs);
    for (Instruction *NewInst : NewInstArr)
      UserInsts.push_back(NewInst);
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

  PHINode *PN = WRegionUtils::getOmpCanonicalInductionVariable(L);
  assert(PN && "Cannot find canonical induction variable.");
  BasicBlock *Backedge = L->getLoopLatch();

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
        return;
      ICmpInst *CondInst = dyn_cast<ICmpInst>(ExitBrInst->getCondition());
      if (!CondInst)
        return;
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

Value *VPOParoptTransform::genRegionPrivateValue(
    WRegionNode *W, Value *V, bool IsFirstPrivate) {
  // V is either AllocaInst or an AddrSpaceCastInst of AllocaInst.
  assert(GeneralUtils::isOMPItemLocalVAR(V) &&
         "genRegionPrivateValue: Expect isOMPItemLocalVAR().");

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
  // FIXME: we have to set PreserveAddressSpace to false,
  //        because otherwise PromoteMemToReg will not be able
  //        to promote the new AllocaInst. This should be fixed
  //        in PromoteMemToReg.
  auto *NewAlloca =
      genPrivatizationAlloca(&FprivI, InsertPt, ".local", llvm::None, false);
  FprivI.setNew(NewAlloca);

  // Replace all uses of the original alloca's result with the new alloca
  // definition.
  Value *ReplacementVal = getClauseItemReplacementValue(&FprivI, InsertPt);
  genPrivatizationReplacement(W, V, ReplacementVal);

  if (IsFirstPrivate) {
    // Copy value from the original "variable" to the new one.
    // We have to create an empty block after the region entry
    // directive to simplify the insertion.
    BasicBlock *PrivInitEntryBB = createEmptyPrivInitBB(W);
    genFprivInit(&FprivI, PrivInitEntryBB->getTerminator());
  }

  return NewAlloca;
}

bool VPOParoptTransform::sinkSIMDDirectives(WRegionNode *W) {
  // Check if there is a child SIMD region associated with
  // this region's loop.
  WRNVecLoopNode *SimdNode = isa<WRNVecLoopNode>(W)
                                 ? cast<WRNVecLoopNode>(W)
                                 : WRegionUtils::getEnclosedSimdForSameLoop(W);

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
    LoopExitBB->getInstList().splice(
        LoopExitBB->getFirstInsertionPt(), ExitBB->getInstList(),
        ExitDir->getIterator(), ++(ExitDir->getIterator()));
    Changed = true;
  }

  W->resetBBSetIfChanged(Changed); // Clear BBSet if transformed
  return Changed;
}

// Add llvm.loop.parallel_accesses metadata to loops with non-monotonic
// property. This metadata refers to one or more access group metadata nodes
// (see llvm.access.group) attached to memory access instructions in a loop. It
// indicates that no loop-carried memory dependence exists between it and other
// instructions in the loop with this metadata (see detailed description at
// https://llvm.org/docs/LangRef.html#llvm-loop-parallel-accesses-metadata).
//
// loop:
//   ...
//   %val = load float, float* %ptr1, !llvm.access.group !0
//   ...
//   store float %val, float* %ptr2, !llvm.access.group !0
//   ...
//   br i1 %cond, label %end, label %loop, !llvm.loop !1
//
// end:
// ...
//
// !0 = distinct !{}
// !1 = distinct !{!1, !2}
// !2 = !{!"llvm.loop.parallel_accesses", !0}
//
#if INTEL_CUSTOMIZATION
// Also attach loop-level llvm.loop.vectorize.ivdep_loop metadata here.
#endif  // INTEL_CUSTOMIZATION
bool VPOParoptTransform::genParallelAccessMetadata(WRegionNode *W) {
  if (!W->getIsOmpLoop() || W->getIsSections() || isa<WRNDistributeNode>(W))
    return false;

  // Check if this loop can have parallel access metadata.
  if (W->getLoopOrder() != WRNLoopOrderConcurrent) {
    // With presence of 'safelen' loop-carried dependences are possible.
    if (isa<WRNVecLoopNode>(W) && W->getSafelen())
      return false;

    if (W->canHaveSchedule()) {
      // OpenMP 5.0, 2.9.2 Worksharing-Loop Construct, Description
      // If the static schedule kind is specified or if the ordered clause is
      // specified, and if the nonmonotonic modifier is not specified, the
      // effect is as if the monotonic modifier is specified. Otherwise, unless
      // the monotonic modifier is specified, the effect is as if the
      // nonmonotonic modifier is specified.
      const auto &Sched = W->getSchedule();
      bool IsMonotonic = W->getOrdered() >= 0 || Sched.getIsSchedMonotonic() ||
                         ((Sched.getKind() == WRNScheduleStatic ||
                           Sched.getKind() == WRNScheduleOrderedStatic) &&
                          !Sched.getIsSchedNonmonotonic());
      if (IsMonotonic)
        return false;
    }
  }

  LLVMContext &C = F->getContext();
  Loop *L = W->getWRNLoopInfo().getLoop();
  bool Modified = false;

#if INTEL_CUSTOMIZATION
  // Add llvm.loop.vectorize.ivdep_loop metadata to the loop.
  // This metadata is attached to the loop as per the LLVM
  // addStringMetadataToLoop/findStringMetadataForLoop interfaces. It indicates
  // that the loop carries no dependences through memory.
  if (!findStringMetadataForLoop(L, "llvm.loop.vectorize.ivdep_loop")) {
    addStringMetadataToLoop(L, "llvm.loop.vectorize.ivdep_loop");
    Modified = true;
  }
#endif  // INTEL_CUSTOMIZATION

  // Add access group metadata to all memory r/w instructions in the loop.
  MDNode *AG = nullptr;
  for (BasicBlock *BB : L->blocks())
    for (Instruction &I : *BB) {
      if (!I.mayReadOrWriteMemory())
        continue;

      if (!AG)
        AG = MDNode::getDistinct(C, {});

      if (MDNode *MD = I.getMetadata(LLVMContext::MD_access_group)) {
        // Existing access groups need to be retained for nested parallel loops.
        SmallVector<Metadata *, 8u> AGs{AG};
        if (MD->getNumOperands())
          copy(MD->operands(), std::back_inserter(AGs));
        else
          AGs.push_back(MD);
        I.setMetadata(LLVMContext::MD_access_group, MDNode::get(C, AGs));
      } else
        I.setMetadata(LLVMContext::MD_access_group, AG);
    }
  if (!AG)
    return Modified;

  // Now add parallel access metadata to the loop. If loop already has loop
  // metadata, then it need to be retained.
  SmallVector<Metadata *, 8u> MDs(1u);
  if (MDNode *ID = L->getLoopID())
    std::copy(std::next(ID->op_begin()), ID->op_end(), std::back_inserter(MDs));

  // Add parallel access metadata with the access group that was created for
  // this loop.
  MDs.push_back(
      MDNode::get(C, {MDString::get(C, "llvm.loop.parallel_accesses"), AG}));

  // And add new metadata to the loop.
  MDNode *NewID = MDNode::get(C, MDs);
  NewID->replaceOperandWith(0, NewID);
  L->setLoopID(NewID);
  Modified = true;

  return Modified;
}

void VPOParoptTransform::simplifyLoopPHINodes(
    const Loop &L, const SimplifyQuery &SQ) const {
  SmallVector<PHINode *, 8> PHIsToDelete;

  // TODO: it should be better to optimize PHINode instructions in the loop
  //       using reverse post_order walk (in the use-def graph of PHINodes).
  for (auto *BB : L.blocks()) {
    for (auto IB = BB->begin(), IE = BB->end(); IB != IE; ++IB) {
      auto *PN = dyn_cast<PHINode>(&*IB);
      if (!PN)
        // Stop processing the block at the first non-PHINode instruction.
        break;
      if (auto *V = SimplifyInstruction(PN, SQ)) {
        PN->replaceAllUsesWith(V);
        PHIsToDelete.push_back(PN);
      }
    }

    // Delete instructions that are not used anymore.
    for (auto *PN : PHIsToDelete) {
      PN->eraseFromParent();
    }
    PHIsToDelete.clear();
  }
}

void VPOParoptTransform::registerizeLoopEssentialValues(
    WRegionNode *W, unsigned Index) {
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
    assert(GeneralUtils::isOMPItemLocalVAR(V) && "Expect isOMPItemLocalVAR().");

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
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
      // For CSA UB does not need to be allocated as omp loops are not outlined.
      if (V == NormUB && (!AI || !isAllocaPromotable(AI)))
        continue;
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
      assert(AI && "Trying mem-to-reg for not an AllocaInst.");
      Allocas.push_back(AI);
    }
  }

  PromoteMemToReg(Allocas, *DT, AC);
}

// Transform the Ith level of the loop in the region W into the
// OMP canonical loop form.
void VPOParoptTransform::regularizeOMPLoopImpl(WRegionNode *W, unsigned Index) {
  Loop *L = W->getWRNLoopInfo().getLoop(Index);
  const DataLayout &DL = L->getHeader()->getModule()->getDataLayout();
  const SimplifyQuery SQ = {DL, TLI, DT, AC};
  LoopRotation(L, LI, TTI, AC, DT, SE, nullptr, SQ, true, unsigned(-1), true);
  // Critical edges splitting for loop nests done during loop rotation
  // may produce PHI nodes that we'd better optimize right away,
  // otherwise they will prevent IV recognition in
  // getOmpCanonicalInductionVariable().
  //
  // Example:
  //   bb1:
  //     %0 = load i64, i64* %outer.lb
  //     %ztt.outer = icmp sle %0, %outer.ub
  //     bt i1 %ztt.outer, label %outer.loop.ph, label %outer.exit
  //   outer.loop.ph:
  //     br label %outer.loop.body
  //   outer.loop.body:
  //     %outer.iv = phi i64 [ %0, %bb1 ], [ %outer.add, %outer.loop.inc ]
  //     %1 = load i64, i64* %inner.lb
  //     %ztt_inner = icmp sle %1, %inner.ub
  //     br i1 %ztt_inner, label %inner.loop.ph, label %inner.exit
  //   inner.loop.ph:
  //     br label %inner.loop.body
  //   inner.loop.body:
  //     ...
  //     br i1 %cmp1, label %inner.loop.body, label %crit.edge.block
  //   crit.edge.block:
  //     br label %inner.exit
  //   inner.exit:
  //     %outer.iv.phi = phi i64 [ %outer.iv, %crit.edge.block ],
  //                             [%outer.iv, %outer.loop.body ]
  //     %outer.add = add nsw i64 %outer.iv.phi, 1
  //     %cmp2 = icmp sle i64 %outer.add, %outer.ub
  //     br i1 %cmp2, label %outer.loop.body, label %crit.edge.exit.block
  //   crit.edge.exit.block:
  //     br label %outer.exit
  //   outer.exit:
  //     ...
  //
  // We need to simplify %outer.iv.phi before we can proceed to
  // getOmpCanonicalInductionVariable(). Moreover, the PHI nodes
  // may become more complex, if we do not reset 'volatile'
  // from the load/store instructions accessing the normalized IVs.
  // This is why we process all IVs in a loop nest before starting
  // rotating the loops.
  simplifyLoopPHINodes(*L, SQ);
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
  if (!First) {
    // First, we have to registerize the loops' essential values
    // (i.e. normalized IV and UB), so that PHI instructions
    // produced by loop rotation in regularizeOMPLoopImpl() can be simplified
    // to be recognizable by getOmpCanonicalInductionVariable().
    for (unsigned I = W->getWRNLoopInfo().getNormIVSize(); I > 0; --I)
      registerizeLoopEssentialValues(W, I - 1);
    // Regularize the loops, which includes the loops rotation,
    // simplification of PHI instructions and loop conditions,
    // and finding the loops' ZTT instructions.
    for (unsigned I = W->getWRNLoopInfo().getNormIVSize(); I > 0; --I)
      regularizeOMPLoopImpl(W, I - 1);
  } else {
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
             "Expect isOMPItemLocalVAR().");
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
      !isa<WRNParallelSectionsNode>(W) && !isa<WRNTargetNode>(W) &&
      !isa<WRNDistributeParLoopNode>(W))
    // TODO: Remove this to enable the function for all outlined WRNs.
    //
    // While this condition is here it should match with one in
    // WRegionUtils::collectNonPointerValuesToBeUsedInOutlinedRegion.
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
                                 true, // Insert (4) in beginning of NewEntryBB
                                 true, // Insert alloca based on parent WRegions
                                 isTargetSPIRV()); // Add addrspace cast to
                                                   // the captured addr for
                                                   // target spirv.
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

  if (W->canHaveUseDevicePtr()) {
    UseDevicePtrClause &UdpClause = W->getUseDevicePtr();
    for (UseDevicePtrItem *UdpI : UdpClause.items())
      Changed |= rename(UdpI->getOrig(), true);
  }

  if (W->canHaveIsDevicePtr()) {
    IsDevicePtrClause &IdpClause = W->getIsDevicePtr();
    for (IsDevicePtrItem *IdpI : IdpClause.items())
      Changed |= rename(IdpI->getOrig(), true);
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

  CI = VPOUtils::addOperandBundlesInCall(CI, BundleOpndAddrs);
  W->setEntryDirective(CI);

  return true;
}

llvm::Optional<unsigned> VPOParoptTransform::getPrivatizationAllocaAddrSpace(
    const WRegionNode *W, const Item *I) const {
  if (!isTargetSPIRV())
    return llvm::None;

  // FIXME: it's workaround for NONPOD array, and the address space is set to
  // private. It will be removed after global VLA is supported for SPIR-V
  // target. And genArrayLength should be updated to support global VLA too.
  Type *AllocaTy = nullptr;
  Value *NumElements = nullptr;
  std::tie(AllocaTy, NumElements, std::ignore) = getItemInfo(I);
  if (I->getIsNonPod() && (AllocaTy->isArrayTy() || (NumElements != nullptr)))
    return vpo::ADDRESS_SPACE_PRIVATE;

#if INTEL_CUSTOMIZATION
  if (I->getIsWILocal() || I->getIsF90DopeVector())
    // FIXME: Remove the check for F90 DV, when we have a way to privatize them
    // at module level/using maps.
    return vpo::ADDRESS_SPACE_PRIVATE;
#endif  // INTEL_CUSTOMIZATION

  if (WRegionUtils::isDistributeNode(W) ||
      isa<WRNTeamsNode>(W))
    return vpo::ADDRESS_SPACE_LOCAL;

  // Objects declared inside "omp target" must be accessible by all teams,
  // so they have to be __global.
  if (isa<WRNTargetNode>(W))
    return vpo::ADDRESS_SPACE_GLOBAL;

  return vpo::ADDRESS_SPACE_PRIVATE;
}

// For array privatization constructor/destructor/copy assignment/copy
// constructor loop, compute the base address of the source and destination
// array, number of elements, and destination element type.
void VPOParoptTransform::genPrivAggregateSrcDstInfo(
    Value *SrcVal, Value *DestVal, Instruction *InsertPt, IRBuilder<> &Builder,
    Value *&NumElements, Value *&SrcArrayBegin, Value *&DestArrayBegin,
    Type *&DestElementTy) {
  NumElements = VPOParoptUtils::genArrayLength(
      DestVal, DestVal, InsertPt, Builder, DestElementTy, DestArrayBegin);

  auto *DestPointerTy = DestArrayBegin->getType();
  assert(isa<PointerType>(DestPointerTy) &&
         "Privatization destination must have pointer type.");
  DestArrayBegin = Builder.CreateBitCast(
      DestArrayBegin,
      PointerType::get(DestElementTy,
                       cast<PointerType>(DestPointerTy)->getAddressSpace()));
  if (SrcVal != nullptr) {
    Type *SrcElementTy = nullptr;
    VPOParoptUtils::genArrayLength(DestVal, SrcVal, InsertPt, Builder,
                                   SrcElementTy, SrcArrayBegin);
    auto *SrcPointerTy = SrcArrayBegin->getType();
    assert(isa<PointerType>(SrcPointerTy) &&
           "Privatization source must have pointer type.");
    SrcArrayBegin = Builder.CreateBitCast(
        SrcArrayBegin,
        PointerType::get(SrcElementTy,
                         cast<PointerType>(SrcPointerTy)->getAddressSpace()));
  }
}

// Generate the constructor/destructor/copy assignment/copy constructor for
// privatized arrays.
//
// Source code:
// class A
// {
//   public:
//     A();
// };
// void fn1(int n) {
//   A e[n];
// #pragma omp parallel for private(e)
//   for(int d=0; d<n; d++);
// }
//
// IR:
// define internal void @_Z3fn1i.DIR.OMP.PARALLEL.LOOP.2.split15.split20(i32*
// %tid, i32* %bid, i64* %.addr, i64* %omp.vla.tmp, i32* %.omp.lb, i32* %.omp.ub
// ) #4 {
// newFuncRoot:
//   %.omp.lb.fpriv = alloca i32, align 4
//   %d.priv = alloca i32, align 4
//   %is.last = alloca i32, align 4
//   %lower.bnd = alloca i32, align 4
//   %upper.bnd = alloca i32, align 4
//   %stride = alloca i32, align 4
//   %upperD = alloca i32, align 4
//   br label %DIR.OMP.PARALLEL.LOOP.2.split15.split20
//
// DIR.OMP.PARALLEL.LOOP.2.split15.split20:          ; preds = %newFuncRoot
//   %0 = load i64, i64* %.addr, align 8
//   %vla.priv = alloca %class.A, i64 %0, align 16
//   ...
// DIR.OMP.PARALLEL.LOOP.2.split14:                  ; preds =
// %DIR.OMP.PARALLEL.LOOP.2.split15.split
//   %array.begin = getelementptr inbounds %class.A, %class.A* %vla.priv,
//   i32 0                                                                 (1)
//   %2 = getelementptr %class.A, %class.A* %array.begin, i64 %0           (2)
//   %priv.constr.isempty = icmp eq %class.A* %array.begin, %2             (3)
//   br i1 %priv.constr.isempty, label %priv.constr.done,
//   label %priv.constr.body                                               (4)
//
// priv.constr.done:                                 ; preds =
// %priv.constr.body, %DIR.OMP.PARALLEL.LOOP.2.split14
//   br label %DIR.OMP.PARALLEL.LOOP.2.split                               (5)
//
// priv.constr.body:                                 ; preds =
// %priv.constr.body, %DIR.OMP.PARALLEL.LOOP.2.split14
//   %priv.cpy.dest.ptr = phi %class.A* [ %array.begin,
//   %DIR.OMP.PARALLEL.LOOP.2.split14 ], [ %priv.cpy.dest.inc,
//   %priv.constr.body ]                                                   (6)
//   %3 = call %class.A* @_ZTS1A.omp.def_constr(%class.A*
//   %priv.cpy.dest.ptr)                                                   (7)
//   %priv.cpy.dest.inc = getelementptr %class.A, %class.A*
//   %priv.cpy.dest.ptr, i32 1                                             (8)
//   %priv.cpy.done = icmp eq %class.A* %priv.cpy.dest.inc, %2             (9)
//   br i1 %priv.cpy.done, label %priv.constr.done, label
//   %priv.constr.body                                                    (10)
//
void VPOParoptTransform::genPrivAggregateInitOrFini(
    Function *Fn, FunctionKind FuncKind, Value *DestVal, Value *SrcVal,
    Instruction *InsertPt, DominatorTree *DT) {
  LLVM_DEBUG(dbgs() << "Enter " << __FUNCTION__ << "\n");

  assert((FuncKind >= FK_Start && FuncKind <= FK_End) &&
         "Invalid function kind.");

  IRBuilder<> Builder(InsertPt);
  auto EntryBB = Builder.GetInsertBlock();

  Type *DestElementTy = nullptr;
  Value *DestBegin = nullptr;
  Value *SrcBegin = nullptr;
  Value *NumElements = nullptr;

  genPrivAggregateSrcDstInfo(SrcVal, DestVal, InsertPt, Builder, NumElements,
                             SrcBegin, DestBegin, DestElementTy); // (1)

  assert(DestBegin &&
         "Null destination address for privatization constructor/destructor.");
  assert(DestElementTy &&
         "Null element type for privatization constructor/destructor.");
  assert(NumElements &&
         "Null number of elements for privatization constructor/destructor.");
  assert((((FuncKind != FK_CopyAssign) && (FuncKind != FK_CopyCtor)) ||
          SrcBegin) &&
         "Null source address for copy assignment/constructor.");

  StringRef Prefix;
  if (FuncKind == FK_Ctor)
    Prefix = "priv.constr";
  else if (FuncKind == FK_Dtor)
    Prefix = "priv.destr";
  else if (FuncKind == FK_CopyAssign)
    Prefix = "priv.cpyassn";
  else
    Prefix = "priv.cpyctor";

  auto DestEnd = Builder.CreateGEP(DestBegin, NumElements); // (2)
  auto IsEmpty =
      Builder.CreateICmpEQ(DestBegin, DestEnd, Prefix + ".isempty"); // (3)

  auto BodyBB = SplitBlock(EntryBB, InsertPt, DT, LI);
  BodyBB->setName(Prefix + ".body");

  auto DoneBB = SplitBlock(BodyBB, BodyBB->getTerminator(), DT, LI);
  DoneBB->setName(Prefix + ".done"); // (5)

  EntryBB->getTerminator()->eraseFromParent();
  Builder.SetInsertPoint(EntryBB);
  Builder.CreateCondBr(IsEmpty, DoneBB, BodyBB); // (4)

  Builder.SetInsertPoint(BodyBB);
  BodyBB->getTerminator()->eraseFromParent();
  PHINode *DestElementPHI =
      Builder.CreatePHI(DestBegin->getType(), 2, "priv.cpy.dest.ptr"); // (6)
  DestElementPHI->addIncoming(DestBegin, EntryBB);

  PHINode *SrcElementPHI = nullptr;
  if (SrcBegin != nullptr) {
    SrcElementPHI =
        Builder.CreatePHI(SrcBegin->getType(), 2, "priv.cpy.src.ptr");
    SrcElementPHI->addIncoming(SrcBegin, EntryBB);
  }

  Instruction *DestElementNext =
      cast<Instruction>(Builder.CreateConstGEP1_32(DestElementPHI, 1,
                                                   "priv.cpy.dest.inc")); // (8)
  Value *SrcElementNext = nullptr;
  if (SrcElementPHI != nullptr)
    SrcElementNext =
        Builder.CreateConstGEP1_32(SrcElementPHI, 1, "priv.cpy.src.inc");

  if (FuncKind == FK_Ctor)
    VPOParoptUtils::genConstructorCall(Fn, DestElementPHI,
                                       DestElementPHI); // (7)
  else if (FuncKind == FK_Dtor)
    VPOParoptUtils::genDestructorCall(Fn, DestElementPHI, DestElementNext);
  else if (FuncKind == FK_CopyAssign)
    VPOParoptUtils::genCopyAssignCall(Fn, DestElementPHI, SrcElementPHI,
                                      DestElementNext);
  else
    VPOParoptUtils::genCopyConstructorCall(Fn, DestElementPHI, SrcElementPHI,
                                           DestElementNext);

  auto Done =
      Builder.CreateICmpEQ(DestElementNext, DestEnd, "priv.cpy.done"); // (9)

  Builder.CreateCondBr(Done, DoneBB, BodyBB); // (10)
  DestElementPHI->addIncoming(DestElementNext, Builder.GetInsertBlock());
  if (SrcElementPHI != nullptr)
    SrcElementPHI->addIncoming(SrcElementNext, Builder.GetInsertBlock());

  if (DT) {
    DT->changeImmediateDominator(BodyBB, EntryBB);
    DT->changeImmediateDominator(DoneBB, EntryBB);
  }
  LLVM_DEBUG(dbgs() << "Exit " << __FUNCTION__ << "\n");
}

// Generate the constructor/destructor/copy assignment/copy constructor for
// privatized variables.
void VPOParoptTransform::genPrivatizationInitOrFini(
    Item *I, Function *Fn, FunctionKind FuncKind, Value *DestVal, Value *SrcVal,
    Instruction *InsertPt, DominatorTree *DT) {
  // TODO: Remove checking per-element function (constructor/destructor/copy
  // assignment/copy constructor) for privatized arrays when the frontend only
  // emit per-element function. Now frontend has the option to switch between
  // per-element and array functions.
  FunctionType *FnTy = Fn->getFunctionType();
  Type *ElementTy = FnTy->getParamType(0)->getPointerElementType();
  Type *AllocaTy = nullptr;
  Value *NumElements = nullptr;
  std::tie(AllocaTy, NumElements, std::ignore) = getItemInfo(I);

  // If it's array (fixed size or variable length array), but the
  // argument is scalar type, loop for calling constructor will be
  // emitted.
  if ((AllocaTy->isArrayTy() || (NumElements != nullptr)) &&
      (ElementTy->isArrayTy() == false)) {
    if (!InsertPt->isTerminator()) {
      BasicBlock *BB = InsertPt->getParent();
      BasicBlock *EmptyBB = SplitBlock(BB, BB->getTerminator(), DT, LI);
      InsertPt = EmptyBB->getTerminator();
    }
    genPrivAggregateInitOrFini(Fn, FuncKind, DestVal, SrcVal, InsertPt, DT);
  } else {
    if (FuncKind == FK_Ctor)
      VPOParoptUtils::genConstructorCall(Fn, DestVal, InsertPt);
    else if (FuncKind == FK_Dtor)
      VPOParoptUtils::genDestructorCall(Fn, DestVal, InsertPt);
    else if (FuncKind == FK_CopyAssign)
      VPOParoptUtils::genCopyAssignCall(Fn, DestVal, SrcVal, InsertPt);
    else
      VPOParoptUtils::genCopyConstructorCall(Fn, DestVal, SrcVal, InsertPt);
  }
}

bool VPOParoptTransform::genPrivatizationCode(WRegionNode *W) {

  bool Changed = false;

  BasicBlock *EntryBB = W->getEntryBBlock();

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genPrivatizationCode\n");

  // Process all PrivateItems in the private clause
  PrivateClause &PrivClause = W->getPriv();
  if (!PrivClause.empty()) {
    W->populateBBSet();

    bool ForTask = W->getWRegionKindID() == WRegionNode::WRNTaskloop ||
                   W->getWRegionKindID() == WRegionNode::WRNTask;
    BasicBlock *DestrBlock = nullptr;
    // Walk through each PrivateItem list in the private clause to perform
    // privatization for each Value item
    for (PrivateItem *PrivI : PrivClause.items()) {
      Value *Orig = PrivI->getOrig();

      if (GeneralUtils::isOMPItemGlobalVAR(Orig) ||
          GeneralUtils::isOMPItemLocalVAR(Orig)) {
        Value *NewPrivInst;

        // Insert alloca for privatization right after the BEGIN directive for
        // all directives except simd. For simd allocas are inserted either into
        // the function's entry block or into the entry block of the parent
        // region that needs outlining. This is done to avoid adding dynamic
        // allocas to the function body.
        // Note: do not hoist the following InsertPt computation out of
        // this for-loop. InsertPt may be a clause directive that is
        // removed by genPrivatizationReplacement(), so we need to recompute
        // InsertPt at every iteration of this for-loop.
        Instruction *InsertPt = EntryBB->getFirstNonPHI();
        if (!ForTask) {
          Instruction *AllocaInsertPt =
              isa<WRNVecLoopNode>(W)
                  ? VPOParoptUtils::getInsertionPtForAllocas(W, F)
                  : InsertPt;
          auto AllocaAddrSpace = getPrivatizationAllocaAddrSpace(W, PrivI);
          NewPrivInst = genPrivatizationAlloca(PrivI, AllocaInsertPt, ".priv",
                                               AllocaAddrSpace);
        } else {
          NewPrivInst = PrivI->getNew();
          InsertPt =
              cast<Instruction>(NewPrivInst)->getParent()->getTerminator();
        }
        PrivI->setNew(NewPrivInst);
        Value *ReplacementVal = getClauseItemReplacementValue(PrivI, InsertPt);
        genPrivatizationReplacement(W, Orig, ReplacementVal);

        if (auto *Ctor = PrivI->getConstructor()) {
          Instruction *CtorInsertPt = dyn_cast<Instruction>(NewPrivInst);
          // NewPrivInst might be at module level. e.g. 'target private (x)'.
          if (!CtorInsertPt)
            CtorInsertPt = InsertPt;
          genPrivatizationInitOrFini(PrivI, Ctor, FK_Ctor, NewPrivInst, nullptr,
                                     CtorInsertPt, DT);
        }

#if INTEL_CUSTOMIZATION
        if (!ForTask && PrivI->getIsF90DopeVector())
          VPOParoptUtils::genF90DVInitCode(PrivI, InsertPt, DT, LI,
                                           isTargetSPIRV());
#endif // INTEL_CUSTOMIZATION
        if (ForTask && PrivI->getDestructor()) {
          // For tasks, call the destructor at the end of the region.
          // For non-tasks, genDestructorCode takes care of this.
          if (!DestrBlock)
            DestrBlock = createEmptyPrivFiniBB(W);

          if (auto *Dtor = PrivI->getDestructor())
            genPrivatizationInitOrFini(PrivI, Dtor, FK_Dtor, PrivI->getNew(),
                                       nullptr, DestrBlock->getTerminator(),
                                       DT);
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
// "LiveIn" Values are PHINode values with one incoming value from Loop
// preheader and others from other basic blocks/Loop Latch.
// LoopInductionVariables are not included in the set, since they are handled in
// a special way using threadID.
void VPOParoptTransform::wrnCollectLiveInVals(
    Loop &L,
    DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
    EquivalenceClasses<Value *> &ECs) {
  BasicBlock *PreheaderBB = L.getLoopPreheader();
  assert(PreheaderBB && "wrnUpdateSSAPreprocess: Loop preheader not found");
  BasicBlock *BB = L.getHeader();
  Instruction *InductionV = WRegionUtils::getOmpCanonicalInductionVariable(&L);
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
          if ((PN != InductionV) && (V != InductionV)) {
            ValueToLiveinMap[V] = {IV, PreheaderBB};
            ECs.unionSets(Leader, V);
          }
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
void VPOParoptTransform::buildECs(Loop *L, Value *V,
                                  EquivalenceClasses<Value *> &ECs) {
  if (!isa<PHINode>(V))
    ECs.getOrInsertLeaderValue(V);
  else {
    PHINode *PN = dyn_cast<PHINode>(V);
    SmallPtrSet<PHINode *, 16> PhiUsers;
    Value *Leader = ECs.getOrInsertLeaderValue(PN);
    unsigned NumPHIValues = PN->getNumIncomingValues();
    unsigned II;
    for (II = 0; II < NumPHIValues; II++)
      AnalyzePhisECs(L, Leader, PN->getIncomingValue(II), ECs, PhiUsers);
  }
}

// Collect the live-out value in the loop.
// A defined Value is considered "LiveOut" if it is used outside the loop or
// it has loop-carried dependence. A variable has loop-carried dependence if it
// is present in "LiveIn" set of the Loop (except Induction Variables.)
void VPOParoptTransform::wrnCollectLiveOutVals(
    Loop &L, SmallSetVector<Instruction *, 8> &LiveOutVals,
    DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
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

        if (!L.contains(UserBB) ||
            (ValueToLiveinMap.find((&I)) != ValueToLiveinMap.end())) {
          LiveOutVals.insert(&I);
          buildECs(&L, &I, ECs);
        }
      }
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

// API to check if the Loop Header has any PHI's with constant values only.
// If any constant value is coming in from the LoopLatch basic block, a PHI is
// added to represent this value in Loop Latch and LoopHeader is updated to use
// the new PHI created. This initializes LiveOut and LiveIn sets for the loop
// appropriately and the code handles loop carried dependence when creating
// dispatch loop.
//
// Eg:
// Before execution of this function:
// LoopHeader:                              ; preds = %LoopPreheader, %LoopLatch
// %flag = phi i32 [0, %LoopPreheader], [1, %LoopLatch]
// ...
//
// ...
//
// LoopLatch:                               ; preds = %if.then , %LoopBody
// ...
//
// -------
// After execution of this function:
// LoopHeader:                              ; preds = %LoopPreheader, %LoopLatch
// %flag = phi i32 [0, %LoopPreheader], [%const.value.phi0, %LoopLatch]
// ...
//
// ...
//
// LoopLatch:                               ; preds = %if.then , %LoopBody
// %const.value.phi0 = phi i32 [ 1, %if.then ], [ 1, %LoopBody ]
// ...
//
void updateConstantLoopHeaderPhis(Loop *L) {
  BasicBlock *BB = L->getHeader();
  BasicBlock *LatchBB = L->getLoopLatch();
  int Suffix = 0;

  for (PHINode &PN : BB->phis()) {
    unsigned NumPHIValues = PN.getNumIncomingValues();
    for (unsigned I = 0; I < NumPHIValues; I++) {
      auto InBB = PN.getIncomingBlock(I);
      if (InBB != LatchBB)
        continue;

      auto IV = PN.getIncomingValue(I);
      if (!isa<ConstantData>(IV))
        continue;

      IRBuilder<> ConstBuilder(LatchBB->getFirstNonPHI());
      unsigned NumPredecessors =
          distance(pred_begin(LatchBB), pred_end(LatchBB));
      PHINode *ConstPhi;
      ConstPhi = ConstBuilder.CreatePHI(
          IV->getType(), NumPredecessors,
          std::string("const.value.phi" + std::to_string(Suffix++)));
      for (BasicBlock *Pred : predecessors(LatchBB)) {
        Value *NewVal = IV;
        ConstPhi->addIncoming(NewVal, Pred);
      }
      // Replace the Value with the new PHINode value.
      PN.removeIncomingValue(I);
      PN.addIncoming(ConstPhi, LatchBB);
    }
  }
}

// Collect the live-in value for the phis at the loop header.
// Collect the live-out set for the Loop.
void VPOParoptTransform::wrnUpdateSSAPreprocess(
    Loop *L,
    DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
    SmallSetVector<Instruction *, 8> &LiveOutVals,
    EquivalenceClasses<Value *> &ECs) {

  wrnCollectLiveInVals(*L, ValueToLiveinMap, ECs);
  wrnCollectLiveOutVals(*L, LiveOutVals, ValueToLiveinMap, ECs);
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
  bool IsDistForLoop = WRegionUtils::isDistributeNode(W);
  bool IsDistParLoop = WRegionUtils::isDistributeParLoopNode(W);
  bool IsDistChunkedParLoop = false;

  LLVMContext &C = F->getContext();
  IntegerType *Int32Ty = Type::getInt32Ty(C);

  DenseMap<Value *, std::pair<Value *, BasicBlock *>> ValueToLiveinMap;
  SmallSetVector<Instruction *, 8> LiveOutVals;
  EquivalenceClasses<Value *> ECs;

  // Get Schedule kind information from W-Region node
  // Default: static_even.
  WRNScheduleKind SchedKind = VPOParoptUtils::getLoopScheduleKind(W);

  if (SchedKind != WRNScheduleStaticEven) {
    // Find and update the Phis in Loop Header that have a constant value
    // coming in from the Loop Latch.
    updateConstantLoopHeaderPhis(L);
  }

  wrnUpdateSSAPreprocess(L, ValueToLiveinMap, LiveOutVals, ECs);

  // Try to find average and maximum trip counts for the loop. If loop has
  // static or dynamic schedule and known chunk then both average and maximum
  // trip counts will be equal to the chunk value.
  Optional<APInt> MaxTC;
#if INTEL_CUSTOMIZATION
  Optional<APInt> AvgTC;
#endif // INTEL_CUSTOMIZATION
  if (W->canHaveSchedule() && (SchedKind == WRNScheduleStatic ||
                               SchedKind == WRNScheduleOrderedStatic ||
                               SchedKind == WRNScheduleDynamic ||
                               SchedKind == WRNScheduleOrderedDynamic))
    if (auto *Chunk =
            dyn_cast_or_null<ConstantInt>(W->getSchedule().getChunkExpr()))
      if (!Chunk->isZero() && !Chunk->isOne()) {
        MaxTC = Chunk->getValue();
#if INTEL_CUSTOMIZATION
        AvgTC = Chunk->getValue();
#endif // INTEL_CUSTOMIZATION
      }

  // Try to find loop's upper bound to get maximum trip count if it is not known
  // yet.
  // TODO: add support for collapsed loops with more than one normalized UB.
  if (!MaxTC && W->getWRNLoopInfo().getNormUBSize() == 1) {
    if (auto *UB = dyn_cast<AllocaInst>(W->getWRNLoopInfo().getNormUB(0))) {
      // Walk UB uses trying to find single store that initializes UB.
      StoreInst *SingleStore = nullptr;
      for (Use &U : UB->uses())
        if (auto *I = dyn_cast<Instruction>(U.getUser())) {
          if (VPOAnalysisUtils::isOpenMPDirective(I) || isa<LoadInst>(I) ||
              I->isLifetimeStartOrEnd() ||
              (isa<BitCastInst>(I) && all_of(I->uses(), [](Use &U) {
                if (auto *I = dyn_cast<Instruction>(U.getUser()))
                  return I->isLifetimeStartOrEnd();
                return false;
              })))
            continue;

          if (auto *SI = dyn_cast<StoreInst>(I))
            if (SI->getPointerOperand() == UB)
              if (!SingleStore) {
                SingleStore = SI;
                continue;
              }

          SingleStore = nullptr;
          break;
        }

      // If we have found single store check what is being stored. If it is a
      // constant integer then trip count is known.
      if (SingleStore)
        if (auto *CI = dyn_cast<ConstantInt>(SingleStore->getValueOperand()))
          MaxTC = CI->getValue() + 1u;
    }
  }

#if INTEL_CUSTOMIZATION
  if (MaxTC &&
      !findStringMetadataForLoop(L, "llvm.loop.intel.loopcount_maximum"))
    addStringMetadataToLoop(L, "llvm.loop.intel.loopcount_maximum",
                            MaxTC->getZExtValue());

  if (AvgTC &&
      !findStringMetadataForLoop(L, "llvm.loop.intel.loopcount_average"))
    addStringMetadataToLoop(L, "llvm.loop.intel.loopcount_average",
                            AvgTC->getZExtValue());
#endif // INTEL_CUSTOMIZATION

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
  IsLastVal->setAlignment(Align(4));
  // Initialize %is.last with zero.
  REBuilder.CreateAlignedStore(REBuilder.getInt32(0), IsLastVal, Align(4));

  AllocaInst *LowerBnd = REBuilder.CreateAlloca(IndValTy, nullptr, "lower.bnd");
  LowerBnd->setAlignment(Align(4));

  AllocaInst *UpperBnd = REBuilder.CreateAlloca(IndValTy, nullptr, "upper.bnd");
  UpperBnd->setAlignment(Align(4));

  AllocaInst *Stride = REBuilder.CreateAlloca(IndValTy, nullptr, "stride");
  Stride->setAlignment(Align(4));

  // UpperD is for distribute loop
  AllocaInst *UpperD = REBuilder.CreateAlloca(IndValTy, nullptr, "upperD");
  UpperD->setAlignment(Align(4));

  // Get Schedule chunk information from W-Region node
  ConstantInt *SchedType = REBuilder.getInt32(SchedKind);
  Value *DistChunkVal = REBuilder.getInt32(1);

  if (IsDistParLoop || IsDistForLoop) {
    // Get dist_schedule kind and chunk information from W-Region node
    // Default: DistributeStaticEven.
    if (VPOParoptUtils::getDistLoopScheduleKind(W) ==
        WRNScheduleDistributeStatic) {
      DistChunkVal = W->getDistSchedule().getChunkExpr();
      IsDistChunkedParLoop = IsDistParLoop;
    }
  }

  // Initialize arguments for loop sharing init call.
  LoadInst *LoadTid = PHBuilder.CreateAlignedLoad(
      PHBuilder.getInt32Ty(), TidPtrHolder, Align(4), "my.tid");

  // Cast the original lower bound value to the type of the induction
  // variable.
  if (LBInitVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    LBInitVal = PHBuilder.CreateSExtOrTrunc(LBInitVal, IndValTy);

  PHBuilder.CreateAlignedStore(LBInitVal, LowerBnd, Align(4));

  Value *UpperBndVal =
      VPOParoptUtils::computeOmpUpperBound(W, 0, PHTerm, ".for.scheduling");
  assert(UpperBndVal &&
         "genLoopSchedulingCode: Expect non-empty loop upper bound");
  PHBuilder.SetInsertPoint(PHTerm);

  if (UpperBndVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    UpperBndVal = PHBuilder.CreateSExtOrTrunc(UpperBndVal, IndValTy);

  PHBuilder.CreateAlignedStore(UpperBndVal, UpperBnd, Align(4));

  bool IsNegStride;
  Value *StrideVal = WRegionUtils::getOmpLoopStride(L, IsNegStride);
  StrideVal = VPOParoptUtils::cloneInstructions(StrideVal, PHTerm);
  PHBuilder.SetInsertPoint(PHTerm);

  if (IsNegStride)
    StrideVal = PHBuilder.CreateSub(ConstantInt::get(IndValTy, 0), StrideVal);

  if (StrideVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    StrideVal = PHBuilder.CreateSExtOrTrunc(StrideVal, IndValTy);

  PHBuilder.CreateAlignedStore(StrideVal, Stride, Align(4));
  PHBuilder.CreateAlignedStore(UpperBndVal, UpperD, Align(4));

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
    TeamIsLast->setAlignment(Align(4));
    // Initialize %team.is.last with zero.
    REBuilder.CreateAlignedStore(REBuilder.getInt32(0), TeamIsLast, Align(4));

    TeamLowerBnd = REBuilder.CreateAlloca(IndValTy, nullptr, "team.lower.bnd");
    TeamLowerBnd->setAlignment(Align(4));

    TeamUpperBnd = REBuilder.CreateAlloca(IndValTy, nullptr, "team.upper.bnd");
    TeamUpperBnd->setAlignment(Align(4));

    TeamStride = REBuilder.CreateAlloca(IndValTy, nullptr, "team.stride");
    TeamStride->setAlignment(Align(4));

    TeamUpperD = REBuilder.CreateAlloca(IndValTy, nullptr, "team.upperD");
    TeamUpperD->setAlignment(Align(4));

    // Initialize arguments for team distribution init call.
    // Insert store instructions and the call in the loop pre-header block.
    PHBuilder.CreateAlignedStore(LBInitVal, TeamLowerBnd, Align(4));
    PHBuilder.CreateAlignedStore(UpperBndVal, TeamUpperBnd, Align(4));
    PHBuilder.CreateAlignedStore(StrideVal, TeamStride, Align(4));
    PHBuilder.CreateAlignedStore(UpperBndVal, TeamUpperD, Align(4));

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
    TeamLB = PHBuilder.CreateAlignedLoad(IndValTy, TeamLowerBnd, Align(4),
                                         "team.new.lb");
    TeamUB = PHBuilder.CreateAlignedLoad(IndValTy, TeamUpperBnd, Align(4),
                                         "team.new.ub");
    TeamST = PHBuilder.CreateAlignedLoad(IndValTy, TeamStride, Align(4),
                                         "team.new.st");
    auto *TeamUD = PHBuilder.CreateAlignedLoad(IndValTy, TeamUpperBnd, Align(4),
                                               "team.new.ud");

    // Store the team bounds as the loop's initial bounds
    // for further work sharing.
    PHBuilder.CreateAlignedStore(TeamLB, LowerBnd, Align(4));
    PHBuilder.CreateAlignedStore(TeamUB, UpperBnd, Align(4));
    PHBuilder.CreateAlignedStore(TeamST, Stride, Align(4));
    PHBuilder.CreateAlignedStore(TeamUD, UpperD, Align(4));
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
            IsUnsigned, IndValTy, PHTerm);

    if (WRegionUtils::isDistributeParLoopNode(W) &&
        VPOParoptUtils::getDistLoopScheduleKind(W) ==
        WRNScheduleDistributeStaticEven) {
      // For "distribute parallel for schedule(static, N)" we need to take
      // minimum between the upper bound and the upper bound of the dist_chunk
      // returned by __kmpc_dist_for_static_init.  Update UpperBndVal with
      // the value of upperD returned by the run-time.  It will be used
      // by genDispatchLoopForStatic() below.
      PHBuilder.SetInsertPoint(PHTerm);
      UpperBndVal = PHBuilder.CreateAlignedLoad(IndValTy, UpperD, Align(4),
                                                "static.upperD");
    }
  } else {
    // Generate __kmpc_dispatch_init_4{u}/8{u} Call Instruction

    // If team distribution is involved, then we need to call initialize
    // with the team's bounds.  We could have used TeamLB and TeamUB
    // here directly, but having the explicit loads makes unit testing
    // easier.
    auto *DispInitLB = PHBuilder.CreateAlignedLoad(IndValTy, LowerBnd, Align(4),
                                                   "disp.init.lb");
    auto *DispInitUB = PHBuilder.CreateAlignedLoad(IndValTy, UpperBnd, Align(4),
                                                   "disp.init.ub");
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
  LoadInst *LoadLB =
      PHBuilder.CreateAlignedLoad(IndValTy, LowerBnd, Align(4), "lb.new");
  LoadInst *LoadUB =
      PHBuilder.CreateAlignedLoad(IndValTy, UpperBnd, Align(4), "ub.new");

  // If IV is signed add range metadata indicating that LB/UB are in positive
  // range [0, MaxValue), where MaxValue is the original loop's TC+1 if loop
  // bounds are known, or the maximum signed value of the IV's type otherwise.
  if (!IsUnsigned) {
    APInt MaxValue =
        MaxTC ? *MaxTC + 1u : APInt::getSignedMaxValue(IndValTy->getBitWidth());
    MDNode *RNode = MDBuilder(C).createRange(
        ConstantInt::get(IndValTy, 0),
        ConstantInt::get(IndValTy, MaxValue.getSExtValue()));
    LoadLB->setMetadata(llvm::LLVMContext::MD_range, RNode);
    LoadUB->setMetadata(llvm::LLVMContext::MD_range, RNode);
  }

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
  VPOParoptUtils::updateOmpPredicateAndUpperBound(W, 0, LoadUB, PHTerm);

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
    if (SchedKind == WRNScheduleStatic ||
        // In case of distribute loop with dist_schedule(static, N),
        // __kmpc_for_static_init returns the lower/upper bounds
        // and the stride that should be used for chunked processing
        // of the loop iterations by the teams' master threads.
        // And we need to generate the same dispatch loop
        // as the one we generate for chunked parallel loops.
        (IsDistForLoop &&
         VPOParoptUtils::getDistLoopScheduleKind(W) ==
         WRNScheduleDistributeStatic)) {

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
    } else if (SchedKind == WRNScheduleStaticEven) {

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
    } else {
      llvm_unreachable("Unexpected scheduling kind.");
    }
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
    if ((isa<WRNWksLoopNode>(W) || isa<WRNParallelLoopNode>(W)) &&
        W->getOrdered() == 0)
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
    Loop *OuterLoop = genDispatchLoopForTeamDistribute(
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
  if (!W->getIsTeams())
    resetValueInOmpClauseGeneric(W, W->getIf());

  // Set up Fn Attr for the new function
  Function *NewF = VPOParoptUtils::genOutlineFunction(*W, DT, AC);
  if (hasParentTarget(W))
    NewF->addFnAttr("target.declare", "true");

  CallInst *NewCall = cast<CallInst>(NewF->user_back());
  assert(NewCall->use_empty() && "Unexpected uses of outlined function call");

  unsigned int TidArgNo = 0;
  bool IsTidArg = false;

  for (auto I = NewCall->arg_begin(), E = NewCall->arg_end(); I != E; ++I) {
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
  for (auto I = NewCall->arg_begin(), E = NewCall->arg_end(); I != E; ++I) {
    if (*I != TidPtrHolder) {
      LLVM_DEBUG(dbgs() << " NewF Arguments: " << *(*I) << "\n");
      MTFnArgs.push_back((*I));
    }
  }

  auto CreateMtFnCall = [MTFn, NewCall, &MTFnArgs](Instruction *InsPt) {
    auto *CI =
        CallInst::Create(MTFn->getFunctionType(), MTFn, MTFnArgs, "", InsPt);
    CI->setCallingConv(NewCall->getCallingConv());

    // Copy isTailCall attribute
    if (NewCall->isTailCall())
      CI->setTailCall();

    CI->setDebugLoc(NewCall->getDebugLoc());
    return CI;
  };

  Instruction *ForkInsPt = NewCall;
  if (Value *IfClause = !W->getIsTeams() ? W->getIf() : nullptr) {
    // TODO: change this to assert once FE starts generating if clause condition
    // with i1 type.
    if (!IfClause->getType()->isIntegerTy(1))
      IfClause = new ICmpInst(NewCall, ICmpInst::ICMP_NE, IfClause,
                              ConstantInt::getSigned(IfClause->getType(), 0),
                              "if.cond");

    Instruction *ElseInsPt = nullptr;
    VPOParoptUtils::buildCFGForIfClause(IfClause, ForkInsPt, ElseInsPt, NewCall,
                                        DT);

    // Create serial call in 'else' block.
    CreateMtFnCall(ElseInsPt);
  }

  CallInst *MTFnCI = CreateMtFnCall(ForkInsPt);

  // Remove the orginal serial call to extracted NewF from the program,
  // reducing the use-count of NewF
  NewCall->eraseFromParent();

  // Finally, nuke the original extracted function.
  NewF->eraseFromParent();

  // Generate __kmpc_fork_call for multithreaded execution of MTFn call
  CallInst* ForkCI = genForkCallInst(W, MTFnCI);
  MTFnCI->eraseFromParent();

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
    Type *I32Ty = Type::getInt32Ty(F->getParent()->getContext());
    LoadInst *Tid = new LoadInst(I32Ty, TidPtrHolder, "my.tid", ForkCI);
    Tid->setAlignment(Align(4));
    if (W->getIsTeams())
      VPOParoptUtils::genKmpcPushNumTeams(W, IdentTy, Tid, NumTeams,
                                          NumThreads, ForkCI);
    else
      VPOParoptUtils::genKmpcPushNumThreads(W, IdentTy, Tid, NumThreads,
                                            ForkCI);
  }

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
  }

#if INTEL_CUSTOMIZATION
  if (!ForkCallFn->hasMetadata(LLVMContext::MD_callback)) {
    // Annotate the callback behavior of the call.
    MDBuilder MDB(C);
    ForkCallFn->addMetadata(LLVMContext::MD_callback,
                            *MDNode::get(C, {MDB.createCallbackEncoding(
                                                2, {-1, -1},
                                                /*VarArgsArePassed=*/true)}));
  }
#endif // INTEL_CUSTOMIZATION

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

  Constant *KmpcLoc = VPOParoptUtils::genKmpcLocfromDebugLoc(
      IdentTy, KMP_IDENT_KMPC, EntryBB, ExitBB);

  ConstantInt *NumArgs =
      ConstantInt::get(Type::getInt32Ty(C), CI->getNumArgOperands() - 2);

  std::vector<Value *> Params;
  Params.push_back(KmpcLoc);
  Params.push_back(NumArgs);
  IRBuilder<> Builder(EntryBB);
  Value *Cast =Builder.CreateBitCast(MicroTaskFn,
                           PointerType::getUnqual(MicroTaskFnTy));
  Params.push_back(Cast);

  auto InitArg = CI->arg_begin(); ++InitArg; ++InitArg;

  for (auto I = InitArg, E = CI->arg_end(); I != E; ++I) {
    Params.push_back((*I));
  }

  CallInst *ForkCallInst = CallInst::Create(ForkCallFn->getFunctionType(),
                                            ForkCallFn, Params, "", CI);

  // CI->replaceAllUsesWith(NewCI);

  VPOParoptUtils::setFuncCallingConv(ForkCallInst, ForkCallInst->getModule());
  ForkCallInst->setTailCall(false);
  ForkCallInst->setDebugLoc(CI->getDebugLoc());

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

// Utility to find Alignment of the COPYIN Variable passed.
// Default is 1.
unsigned VPOParoptTransform::getAlignmentCopyIn(Value *V, const DataLayout DL) {
  // Eg:
  // 1. Global Variable is passed.
  // C Code: int nder; ...
  //         #pragma omp parallel copyin(nder)
  // IR:
  // @nder = dso_local thread_private global i32 0, align 4
  // "QUAL.OMP.COPYIN"(i32* @nder)
  // Alignment : 4
  //
  // 2. Bitcast to get the pointer.
  // Fortran Code:
  //          INTEGER NDER,BBB
  //          COMMON/DERPAR/NDER,BBB  ...
  //          !$OMP PARALLEL copyin(NDER)
  // IR:
  // @derpar_ = common thread_private unnamed_addr global [8 x i8]
  //     zeroinitializer, align 32
  // "QUAL.OMP.COPYIN"(i32* bitcast ([8 x i8]* @derpar_ to i32*))
  // Alignment : 32
  //
  // 3. Bitcast with getelementpointer.
  // Fortran Code:
  //          CHARACTER X
  //          INTEGER NDER,BBB
  //          COMMON/DERPAR/X,NDER,BBB  ...
  //          !$OMP PARALLEL copyin(NDER)
  // IR:
  // @derpar_ = common thread_private unnamed_addr global [9 x i8]
  //     zeroinitializer, align 32
  // "QUAL.OMP.COPYIN"(i32* bitcast (i8* getelementptr inbounds
  //     ([9 x i8], [9 x i8]* @derpar_, i32 0, i64 1) to i32*))
  // Alignment : 1, BaseAlignment:32, Offset:1
  //
  GlobalVariable *GVPtr;
  if ((GVPtr = dyn_cast<GlobalVariable>(V)))
    return GVPtr->getAlignment();

  if (auto *BC = dyn_cast<BitCastOperator>(V)) {
    Value *BCOperand = BC->getOperand(0);
    if ((GVPtr = dyn_cast<GlobalVariable>(BCOperand)))
      return GVPtr->getAlignment();
    if (auto *GEP = dyn_cast<GEPOperator>(BCOperand)) {
      auto *BasePointer = GEP->getPointerOperand()->stripPointerCasts();
      if ((GVPtr = dyn_cast<GlobalVariable>(BasePointer))) {
        unsigned BaseAlignment = GVPtr->getAlignment();
        APInt ConstOffset(64, 0);
        GEP->accumulateConstantOffset(DL, ConstOffset);
        unsigned Offset = ConstOffset.getZExtValue();
        return ((Offset == 0) ? BaseAlignment
                              : greatestCommonDivisor(BaseAlignment, Offset));
      }
    }
  }
  return 1;
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

    Instruction *Term = nullptr;
    for (auto C : CP.items()) {
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

      uint64_t Size = NDL.getTypeAllocSize(
          C->getOrig()->getType()->getPointerElementType());
      VPOUtils::genMemcpy(C->getOrig(), &*NewArgI, Size,
                          getAlignmentCopyIn(C->getOrig(), NDL),
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

  // This is a map between new function's parameter index to the attribute set
  // that need to be applied to the parameter after finalizing the new function.
  // Parameter attributes are copied from the old function.
  DenseMap<unsigned, AttributeSet> Param2Attrs;

  unsigned int FnParmNo = 0;
  unsigned int TidParmNo = 0;
  for (auto ArgTyI = FnTy->param_begin(), ArgTyE = FnTy->param_end();
       ArgTyI != ArgTyE; ++ArgTyI) {

    // Matching formal argument and actual argument for Thread ID
    if (!IsTidArg || TidParmNo != TidArgNo) {
      Param2Attrs[ParamsTy.size()] =
          Fn->getAttributes().getParamAttributes(FnParmNo);
      ParamsTy.push_back(*ArgTyI);
    }

    // Remove parameter attributes from the old function.
    Fn->removeParamAttrs(FnParmNo,
                         Fn->getAttributes().getParamAttributes(FnParmNo));

    ++TidParmNo;
    ++FnParmNo;
  }

  Type *RetTy = FnTy->getReturnType();
  FunctionType *NFnTy = FunctionType::get(RetTy, ParamsTy, false);

  // Create the new function body and insert it into the module...
  Function *NFn = Function::Create(NFnTy, Fn->getLinkage());

  NFn->copyAttributesFrom(Fn);

  // Propagate old parameter attributes to the new function.
  for (auto &P : Param2Attrs)
    NFn->addParamAttrs(P.first, P.second);

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

  // Everything including the routine name has been moved to the new routine.
  // Do the same with the debug information.
  NFn->setSubprogram(Fn->getSubprogram());
  Fn->setSubprogram(nullptr);

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
  BasicBlock *NewBB = createEmptyPrivFiniBB(W);
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
  bool Changed = false;

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genCriticalCode\n");
  assert(CriticalNode != nullptr && "Critical node is null.");

  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(TidPtrHolder != nullptr && "TidPtr is null.");

  if (isTargetSPIRV())
    Changed = removeCompilerGeneratedFences(CriticalNode);

  // genKmpcCriticalSection() needs BBSet for error checking only;
  // In the future consider getting rid of this call to populateBBSet.
  CriticalNode->populateBBSet();

  StringRef LockNameSuffix = CriticalNode->getUserLockName();

  bool CriticalCallsInserted =
      VPOParoptUtils::genKmpcCriticalSection(CriticalNode, IdentTy,
                                             TidPtrHolder, DT, LI,
                                             isTargetSPIRV(),
                                             LockNameSuffix);

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Handling of Critical Node: "
                    << (CriticalCallsInserted ? "Successful" : "Failed")
                    << ".\n");

  assert(CriticalCallsInserted && "Failed to create critical section. \n");

  CriticalNode->resetBBSet(); // Invalidate BBSet
  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genCriticalCode\n");
  return Changed || CriticalCallsInserted;
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
    WRegionNode *W, const ArrayRef<Value *> IsLastLocs,
    BasicBlock *&IfLastIterOut, Instruction *InsertBefore) {

  // No need to emit the branch if W doesn't have any linear or non-conditional
  // lastprivate var.
  if ((!W->canHaveLastprivate() || (W->getLpriv()).empty() ||
       std::all_of(
           W->getLpriv().items().begin(), W->getLpriv().items().end(),
           [](LastprivateItem *LI) { return LI->getIsConditional(); })) &&
      (!W->canHaveLinear() || (W->getLinear()).empty()))
    return false;

  assert(!IsLastLocs.empty() && "genLastIterationCheck: IsLastLocs is null.");

  if (!InsertBefore) {
    // First, create an empty predecessor BBlock for ExitBB of the WRegion.  (3)
    BasicBlock *ExitBBPredecessor = createEmptyPrivFiniBB(W);
    assert(ExitBBPredecessor && "genLoopLastIterationCheck: Couldn't create "
                                "empty BBlock before the exit BB.");
    InsertBefore = ExitBBPredecessor->getTerminator();
  }

  // Next, we insert the branching code before InsertBefore.

  // Logically and all IsLast values.
  IRBuilder<> Builder(InsertBefore);
  Value *IsLastPredicate = nullptr;

  for (auto *Ptr : IsLastLocs) {
    auto *V = Builder.CreateLoad(Builder.getInt32Ty(), Ptr); //              (1)

    if (!IsLastPredicate) {
      IsLastPredicate = V;
      continue;
    }

    IsLastPredicate = Builder.CreateAnd(IsLastPredicate, V);
  }
  Value *LastCompare =
      Builder.CreateICmpNE(IsLastPredicate, Builder.getInt32(0)); //         (2)

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
    BasicBlock *NewBB = createEmptyPrivFiniBB(W);
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
    IRBuilder<> Builder(InsertPt);
    unsigned IfExprBitWidth = IfExpr->getType()->getIntegerBitWidth();
    auto *CondInst =
        IfExprBitWidth == 1
            ? IfExpr
            : Builder.CreateICmpNE(IfExpr, Builder.getIntN(IfExprBitWidth, 0),
                                   "cancel.if");

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
  Align I32Align = F->getParent()->getDataLayout().getABITypeAlign(I32Type);

  BasicBlock &FunctionEntry = F->getEntryBlock();
  IRBuilder<> AllocaBuilder(FunctionEntry.getFirstNonPHI());

  for (auto *CancellationPoint : CancellationPoints) {               // (3)
    AllocaInst *CPAlloca =
        AllocaBuilder.CreateAlloca(I32Type, nullptr, "cp");          // (1)

    StoreInst *CPStore =
        new StoreInst(CancellationPoint, CPAlloca, false, I32Align); // (4)
    CPStore->insertAfter(CancellationPoint);
    CancellationPointAllocas.push_back(CPAlloca);
  }

  // (2) Add the list of allocas where cancellation points' return values are
  // stored, as an operand bundle in the region.entry() directive.
  CI = VPOUtils::addOperandBundlesInCall(
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
  BasicBlock *CancelExitBB = createEmptyPrivFiniBB(W);

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

      Type *I32Ty = Type::getInt32Ty(InsertPt->getModule()->getContext());
      LoadInst *LoadTid = new LoadInst(I32Ty, TidPtrHolder, "my.tid", InsertPt);
      LoadTid->setAlignment(Align(4));
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
      CI = VPOUtils::addOperandBundlesInCall(CI, {{ClauseString, {NormUBs}}});
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
      CI = VPOUtils::addOperandBundlesInCall(CI, {{ClauseString, {NormUBs}}});
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
  const DataLayout &DL = F->getParent()->getDataLayout();
  uint64_t Size =
      DL.getTypeAllocSize(CopyPrivateBase->getType()->getPointerElementType());
  VPOParoptUtils::genKmpcCopyPrivate(
      W, IdentTy, TidPtrHolder, Size, CopyPrivateBase, FnCopyPriv,
      Builder.CreateLoad(IsSingleThread->getAllocatedType(), IsSingleThread),
      InsertPt);

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
    LoadInst *SrcLoad = Builder.CreateLoad(
        cast<GEPOperator>(SrcGep)->getResultElementType(), SrcGep,
        OrigName + ".src");
    LoadInst *DstLoad = Builder.CreateLoad(
        cast<GEPOperator>(DstGep)->getResultElementType(), DstGep,
        OrigName + ".dst");
    Value *NewCopyPrivInst =
        genPrivatizationAlloca(CprivI, EntryBB->getTerminator(), ".cp.priv");
    genLprivFini(CprivI, NewCopyPrivInst, DstLoad, EntryBB->getTerminator());
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
  int MappedDir = WL->getMappedDir();
  StringRef MappedEntryDirStr = VPOAnalysisUtils::getDirectiveString(MappedDir);
  CallInst *EntryCI = cast<CallInst>(W->getEntryDirective());
  LLVM_DEBUG(dbgs() << "Entry directive: "
                    << VPOAnalysisUtils::getDirectiveString(EntryCI)
                    << " maps to Directive: " << MappedEntryDirStr << "\n");

  SmallVector<OperandBundleDef, 8> OpBundles;
  EntryCI->getOperandBundlesAsDefs(OpBundles);
  for (unsigned i = 0; i < OpBundles.size(); i++) {
    EntryCI =
        VPOUtils::removeOperandBundlesFromCall(EntryCI, OpBundles[i].getTag());
  }
  SmallVector<std::pair<StringRef, ArrayRef<Value *>>, 8> OpBundlesToAdd;
  OpBundlesToAdd.emplace_back(MappedEntryDirStr, ArrayRef<Value *>{});
  for (unsigned i = 1; i < OpBundles.size(); i++) {
    // The clauses should be dropped during mapping, since it's only used for
    // loop construct but it's not supported by the mapped directive.
    // 1. BIND clause;
    // 2. SHARED clause if the mapped directive is DIR_OMP_LOOP or DIR_OMP_SIMD;
    // 3. FIRSTPRIVATE clause if the mapped directive is DIR_OMP_SIMD.
    StringRef Tag = OpBundles[i].getTag();
    int ClauseID = VPOAnalysisUtils::getClauseID(Tag);
    if (VPOAnalysisUtils::isBindClause(ClauseID) ||
        ((QUAL_OMP_SHARED == ClauseID) &&
         (MappedDir == DIR_OMP_LOOP || MappedDir == DIR_OMP_SIMD)) ||
        ((QUAL_OMP_FIRSTPRIVATE == ClauseID) && (MappedDir == DIR_OMP_SIMD)))
      continue;

    OpBundlesToAdd.emplace_back(Tag, OpBundles[i].inputs());
  }
  EntryCI = VPOUtils::addOperandBundlesInCall(EntryCI, OpBundlesToAdd);
  W->setEntryDirective(EntryCI);

  // replace exit directive accordingly
  CallInst *ExitCI =
      dyn_cast<CallInst>(VPOAnalysisUtils::getEndRegionDir(EntryCI));
  StringRef ExitDirStr = VPOAnalysisUtils::getDirectiveString(ExitCI);

  StringRef MappedExitDirStr = VPOAnalysisUtils::getDirectiveString(
      VPOAnalysisUtils::getMatchingEndDirective(MappedDir));
  LLVM_DEBUG(dbgs() << "Exit directive: " << ExitDirStr
                    << " maps to directive: " << MappedExitDirStr << "\n");

  ExitCI = VPOUtils::removeOperandBundlesFromCall(ExitCI, {ExitDirStr});
  ExitCI = VPOUtils::addOperandBundlesInCall(
      ExitCI, {{MappedExitDirStr, ArrayRef<Value *>{}}});

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
  WRNTargetNode *WT = cast<WRNTargetNode>(W);

  if (!deviceTriplesHasSPIRV())
    return false;

  // It is not really necessary to generate ND-range parameter
  // during SPIR compilation, because it is only used in host code
  // for passing to libomptarget. At the same time, we have to
  // call setParLoopNdInfoAlloca() with some non-null value below,
  // so we just do it for both host and target compilations.
  auto *NDInfoAI = genTgtLoopParameter(WT);
  if (!NDInfoAI)
    return false;

  WT->setParLoopNdInfoAlloca(NDInfoAI);
  return true;
}

bool VPOParoptTransform::collapseOmpLoops(WRegionNode *W) {
  auto Exiter = [FunctionName = __FUNCTION__, W](bool Changed) {
    W->resetBBSetIfChanged(Changed);
    LLVM_DEBUG(dbgs() << FunctionName << ": finished loops collapsing\n");
    return Changed;
  };

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": starting loops collapsing\n");

  if (!W->getIsOmpLoop()) {
    LLVM_DEBUG(dbgs() << "Not a loop construct.  Exiting.\n");
    return Exiter(false);
  }

  unsigned NumLoops = W->getWRNLoopInfo().getNormIVSize();

  if (NumLoops == 0) {
    // SIMD loops combined with other loop constructs do not have
    // associated loops.
    LLVM_DEBUG(dbgs() << "Loop is associated with an enclosing construct.\n");
    return Exiter(false);
  }

  assert(((W->getCollapse() == 0 && NumLoops == 1) ||
          // TODO: replace <= with ==, when we enable late collapsing
          //       for all compilations.
          NumLoops <= (unsigned)W->getCollapse()) &&
         "Number of loops does not match the collapse clause value.");

  // If there is a parent "omp target", we may want to hoist the computation
  // of the collapsed loop bounds before it to use the ND-range
  // parallelization for SPIR targets. Check if we can do the hoisting.
  // Note that the computations has to be hoisted on the host.
  bool CanHoistCombinedUBBeforeTarget = true;
  WRegionNode *WTarget =
      WRegionUtils::getParentRegion(W, WRegionNode::WRNTarget);
  if (!WTarget)
    CanHoistCombinedUBBeforeTarget = false;
  else
    // Check if all normalized UB pointers are firstprivate
    // for the "omp target" region.
    for (unsigned Idx = 0; Idx < NumLoops; ++Idx) {
      Value *NormUB = W->getWRNLoopInfo().getNormUB(Idx);

      if (std::none_of(WTarget->getFpriv().items().begin(),
                       WTarget->getFpriv().items().end(),
                       [NormUB](FirstprivateItem *FprivI) {
                         if (NormUB == FprivI->getOrig())
                           return true;
                         return false;
                       })) {
        CanHoistCombinedUBBeforeTarget = false;
        break;
      }
    }

  if (!CanHoistCombinedUBBeforeTarget &&
      VPOParoptUtils::getSPIRExecutionScheme() == spirv::ImplicitSIMDSPMDES &&
      isTargetSPIRV()) {
    // Emit opt-report only during SPIR compilation.
    OptimizationRemarkMissed R("openmp", "Target", W->getEntryDirective());
    R << "Consider using OpenMP combined construct "
      "with \"target\" to get optimal performance";
    ORE.emit(R);
  }

  // IsTopLevelTargetLoop is true only if the current region
  // is a single top-level, non-SIMD, loop region enclosed into a target region.
  //
  // For example:
  // #pragma omp target teams
  //   {
  //     <code without OpenMP loops>
  //     #pragma omp distribute/parallel for
  //     for (...) ...;
  //     <code without OpenMP loops>
  //   }
  // The distribute/parallel for loop is a top-level loop region.
  //
  // For example:
  // #pragma omp target teams
  //   {
  //     <code without OpenMP loops>
  //     #pragma omp distribute
  //     for (...) {
  //       <code without OpenMP loops>
  //       #pragma omp parallel for
  //       for (...) ...;
  //       <code without OpenMP loops>
  //     }
  //     <code without OpenMP loops>
  //   }
  // The distribute loop is a top-level loop region.
  // The parallel for is not a top-level loop region.
  bool IsTopLevelTargetLoop = true;
  if (!WTarget || isa<WRNVecLoopNode>(W)) {
    IsTopLevelTargetLoop = false;
  } else {
    auto IsNotTargetRegion = [](WRegionNode *W) {
                               return !isa<WRNTargetNode>(W);
                             };

    // Traverse ancestors up to the enclosing target region,
    // or up to the last one (if there is no enclosing target
    // region), and check if there is an enclosing loop region.
    if (WRegionUtils::getParentRegion(W,
            [](WRegionNode *W) {
              return W->getIsOmpLoop();
            },
            IsNotTargetRegion))
      IsTopLevelTargetLoop = false;

    // Traverse ancestors up to the enclosing target region,
    // or up to the last one (if there is no enclosing target
    // region), and check if any of the ancestors has more than
    // one child region.
    // For example:
    // #pragma omp target teams
    //   {
    //     #pragma omp distribute parallel for
    //     for (...) ...;
    //     #pragma omp distribute parallel for
    //     for (...) ...;
    //   }
    // Neither of the loop regions is a top-level loop region.
    //
    // We could have allowed that actually. For example, if loop bounds
    // computations may be hoisted outside of the target region for both
    // loops, we could set two NDRANGE clauses for the target region,
    // and then set the runtime ND-range to the maximum of the two.
    if (WRegionUtils::getParentRegion(W,
            [](WRegionNode *W) {
              return W->getNumChildren() > 1;
            },
            IsNotTargetRegion))
      IsTopLevelTargetLoop = false;
  }

  bool HoistCombinedUBBeforeTarget = false;
  bool SetNDRange = false;
  bool Use1DRange = false;

  if (IsTopLevelTargetLoop) {
    // Use NDRANGE clause only for top-level loops enclosed into
    // a target region.
    if (NumLoops > 3 ||
        // Always collapse "omp distribute" loop nests.
        // Ideally, on SPIR targets each iteration of the collapsed loop nest
        // must be run by one WG, but there is currently no way to communicate
        // this to the runtime.
        //
        // TODO:
        //   #pragma omp distribute collapse(2)
        //   for (int i = 0; i < 10; ++i)
        //     for (int j = 0; j < 10; ++j)
        //
        // When we collapse the loop, the global size becomes 100,
        // and the local size must be 1. Then, the parallel threads
        // running the inner "parallel" regions may be created by
        // using another ND-range dimension, e.g. with
        // global size (100, N), and local size (1, N).
        // N may not exceed the kernel limitations, and this limit
        // is only known in runtime. We need to communicate to runtime
        // that it has to choose some N and use it both for global
        // and local sizes for 0 dimension.
        WRegionUtils::isDistributeNode(W)) {
      // FIXME: this is a temporary limitation. We need to decide
      //        which loops to collapse for SPIR target and leave
      //        3 of them for 3D-range parallelization.
      if (CanHoistCombinedUBBeforeTarget &&
          VPOParoptUtils::getSPIRExecutionScheme() ==
          spirv::ImplicitSIMDSPMDES) {
        // Collapse the loop nest and use 1D range.
        HoistCombinedUBBeforeTarget = true;
        SetNDRange = true;
        Use1DRange = true;
      }
      // Otherwise, collapse the loop nest for all targets and the host
      // and do not use any ND-range.
    }
    else if (CanHoistCombinedUBBeforeTarget &&
             VPOParoptUtils::getSPIRExecutionScheme() ==
             spirv::ImplicitSIMDSPMDES) {
      // Do not collapse the loop nest for SPIR target.
      if (isTargetSPIRV()) {
        if (W->getCollapse() == 0)
          LLVM_DEBUG(dbgs() <<
                     "ND-range parallelization will be applied for loop.\n");
        else
          LLVM_DEBUG(dbgs() << "Loop nest left uncollapsed for SPIR target. " <<
                     "ND-range parallelization will be applied.\n");
        setNDRangeClause(WTarget, W, W->getWRNLoopInfo().getNormUBs());
        return Exiter(true);
      }

      // Collapse the loop nest for all other targets and the host
      // and use NumLoops ND-range. Note that we do not need to hoist
      // the combined upper bound computation before the target region.
      SetNDRange = true;
    }
  }

  if (NumLoops == 1) {
    LLVM_DEBUG(dbgs() << "No loop nest to collapse.  Exiting.\n");
    if (SetNDRange)
      setNDRangeClause(WTarget, W, W->getWRNLoopInfo().getNormUBs());
    return Exiter(true);
  }

  W->populateBBSet();

  // Collect all blocks composing the loop nest. They will be inside
  // the new loop that we will create.
  // Collect all region's blocks, as they all will be inside
  // the new loop that we will create.
  auto *OutermostLoop = W->getWRNLoopInfo().getLoop(0);

  // First, find the Loop's lower bound pointer definition.
  SmallVector<std::pair<Value *, Type *>, 3> LBPtrDefs;

  for (unsigned Idx = 0; Idx < NumLoops; ++Idx) {
    // Assumptions:
    //   Loop must be in top-test form currently.
    //   The header block must have two predecessors, and the predecessor
    //   located ouside of the loop must contain code that loads the lower
    //   bound and stores it into the induction variable.
    //   This is currently the way FE generates normalized OpenMP loops:
    //     %0 = load i64, i64* %.omp.uncollapsed.lb
    //     store i64 %0, i64* %.omp.uncollapsed.iv

    //
    // FIXME: request QUAL.OMP.NORMALIZED.LB clause to be generated by FE,
    //        so that we can get rid of the code looking for the lower
    //        bound pointer definition.
    LLVM_DEBUG(dbgs() << "\n" << __FUNCTION__ << ": processing loop #" <<
               Idx << "\n");

    auto *L = W->getWRNLoopInfo().getLoop(Idx);
    auto *Header = L->getHeader();
    assert(Header && Header->hasNPredecessors(2) && "Invalid loop header.");

    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": header block: " <<
               Header->getName() << "\n");

    BasicBlock *AssignmentBB = nullptr;
    for (auto *BB : predecessors(Header)) {
      if (!L->contains(BB)) {
        assert(!AssignmentBB &&
               "Neither predecessor of the header belongs to the Loop.");
        AssignmentBB = BB;
      }
    }

    assert(AssignmentBB &&
           "Both predecessors of the header belong to the Loop.");

    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": IV assignment block: " <<
               AssignmentBB->getName() << "\n");

    // Look for an instruction storing into IV.
    Value *IVPtrDef = W->getWRNLoopInfo().getNormIV(Idx);
    StoreInst *StoreToIV = nullptr;
    for (auto I = AssignmentBB->rbegin(), IE = AssignmentBB->rend();
         I != IE; ++I)
      if (auto *SI = dyn_cast<StoreInst>(&*I))
        if (SI->getPointerOperand() == IVPtrDef) {
          StoreToIV = SI;
          break;
        }

    assert(StoreToIV && "Cannot find store to IV.");

    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": IV assignment store: " <<
               *StoreToIV << "\n");

    // The store's operand must be a load from LB.
    LoadInst *LoadFromLB = dyn_cast<LoadInst>(StoreToIV->getOperand(0));
    assert(LoadFromLB && "Value stored into IV is not a load.");

    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": IV assignment value: " <<
               *LoadFromLB << "\n");

    LBPtrDefs.push_back(std::make_pair<Value *, Type *>(
        LoadFromLB->getPointerOperand(), LoadFromLB->getType()));

    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": LB pointer definition: "
                      << *(LBPtrDefs.back().first) << "\n");
  }

  // Second, compute the collapsed iteration space before the region.
  IRBuilder<> BeforeRegBuilder(
      HoistCombinedUBBeforeTarget ?
          WTarget->getEntryDirective() : W->getEntryDirective());
  // TODO: use I64 for the time being. We can probably rely on the type
  //       of the original upper bound(s) produced by FE.
  Type *CombinedUBType = BeforeRegBuilder.getInt64Ty();
  SmallVector<Value *, 3> MulOperands;
  for (unsigned Idx = 0; Idx < NumLoops; ++Idx) {
    auto *UBPtrDef = W->getWRNLoopInfo().getNormUB(Idx);
    auto *UBVal = BeforeRegBuilder.CreateLoad(
        UBPtrDef->getType()->getPointerElementType(), UBPtrDef,
        Twine(UBPtrDef->getName()) + Twine(".val"));
    MulOperands.push_back(
        BeforeRegBuilder.CreateAdd(
            BeforeRegBuilder.CreateZExtOrTrunc(UBVal, CombinedUBType, ".zext"),
            ConstantInt::get(CombinedUBType, 1),
            "", true, true));
  }

  Value *NewUpperBndVal = MulOperands.front();
  for (unsigned Idx = 1; Idx < NumLoops; ++Idx)
    NewUpperBndVal =
        BeforeRegBuilder.CreateMul(NewUpperBndVal, MulOperands[Idx],
                                   "", true, true);

  NewUpperBndVal =
      BeforeRegBuilder.CreateSub(
          NewUpperBndVal, ConstantInt::get(CombinedUBType, 1), "", true, true);
  NewUpperBndVal->setName("omp.collapsed.ub.value");

  // Create new variables for the collapsed loop bounds and IV.
  IRBuilder<> EntryBuilder(&F->front().front());
  const DataLayout &DL = F->getParent()->getDataLayout();
  // Inherit the pointer address space from the UB.
  // All three IV, LB and UB pointers should be in the same address space.
  unsigned PtrAddrSpace =
      W->getWRNLoopInfo().getNormUB(0)->getType()->getPointerAddressSpace();

  auto CreateAI =
      [CombinedUBType, &DL, &EntryBuilder, PtrAddrSpace](StringRef Name) {
    AllocaInst *NewAI =
        EntryBuilder.CreateAlloca(CombinedUBType, DL.getAllocaAddrSpace(),
                                  nullptr, Name);
    return EntryBuilder.CreateAddrSpaceCast(
        NewAI, NewAI->getAllocatedType()->getPointerTo(PtrAddrSpace),
        Twine(NewAI->getName()) + Twine(".ascast"));
  };

  Value *NewIVPtrDef = CreateAI("omp.collapsed.iv");
  Value *NewLBPtrDef = CreateAI("omp.collapsed.lb");
  Value *NewUBPtrDef = CreateAI("omp.collapsed.ub");

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": new collapsed IV: " <<
             *NewIVPtrDef << "\n");
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": new collapsed LB: " <<
             *NewLBPtrDef << "\n");
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": new collapsed UB: " <<
             *NewUBPtrDef << "\n");

  // Store 0 into new LB and the combined upper bound into new UB.
  BeforeRegBuilder.CreateStore(
      ConstantInt::get(CombinedUBType, 0), NewLBPtrDef);
  BeforeRegBuilder.CreateStore(NewUpperBndVal, NewUBPtrDef);

  // Compute the dimensions at the beginning of the region.
  BasicBlock *DimInitBB =
      SplitBlock(W->getEntryBBlock(), W->getEntryDirective()->getNextNode(),
                 DT, LI);
  IRBuilder<> DimInitBuilder(&DimInitBB->front());

  SmallVector<Value *, 3> Dimensions;
  for (unsigned Idx = 1; Idx < NumLoops; ++Idx) {
    auto *UBPtrDef = W->getWRNLoopInfo().getNormUB(Idx);
    auto *UBVal = DimInitBuilder.CreateLoad(
        UBPtrDef->getType()->getPointerElementType(), UBPtrDef,
        Twine(UBPtrDef->getName()) + Twine(".val"));
    Dimensions.push_back(
        DimInitBuilder.CreateAdd(
            DimInitBuilder.CreateZExtOrTrunc(UBVal, CombinedUBType, ".zext"),
            ConstantInt::get(CombinedUBType, 1),
            "", true, true));
  }
  // Last dimension is 1.
  Dimensions.push_back(ConstantInt::get(CombinedUBType, 1));

  for (int Idx = NumLoops - 2; Idx >= 0; --Idx)
    Dimensions[Idx] = DimInitBuilder.CreateMul(Dimensions[Idx],
                                               Dimensions[Idx + 1],
                                               "", true, true);

  // Wrap a loop around the region to iterate over the collapsed
  // iteration space.

  // For the sample source code:
  // void foo(int n) {
  // #pragma omp parallel for collapse(2)
  //   for (int i = 0; i < n; ++i)
  //     for (int j = 0; j < n; ++j);
  // }
  //
  // The expected IR looks along these lines:
  // omp.precond.then:
  //   %17 = call token @llvm.directive.region.entry() [
  //       "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.COLLAPSE"(i32 2),
  //       "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb),
  //       "QUAL.OMP.NORMALIZED.IV"(i64* %.omp.uncollapsed.iv,
  //                                i64* %.omp.uncollapsed.iv16),
  //       "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.uncollapsed.ub,
  //                                i64* %.omp.uncollapsed.ub24),
  //       "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb23),
  //       "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %j) ]
  //   %18 = load i64, i64* %.omp.uncollapsed.lb, align 8, !tbaa !6
  //   store i64 %18, i64* %.omp.uncollapsed.iv, align 8, !tbaa !6
  //   br label %omp.uncollapsed.loop.cond
  //
  // omp.uncollapsed.loop.cond:
  //     ; preds = %omp.uncollapsed.loop.inc44, %omp.precond.then
  //   %19 = load i64, i64* %.omp.uncollapsed.iv, align 8, !tbaa !6
  //   %20 = load i64, i64* %.omp.uncollapsed.ub, align 8, !tbaa !6
  //   %cmp31 = icmp sle i64 %19, %20
  //   br i1 %cmp31, label %omp.uncollapsed.loop.body,
  //                 label %omp.uncollapsed.loop.end46
  //
  // omp.uncollapsed.loop.body:
  //     ; preds = %omp.uncollapsed.loop.cond
  //   %21 = load i64, i64* %.omp.uncollapsed.lb23, align 8, !tbaa !6
  //   store i64 %21, i64* %.omp.uncollapsed.iv16, align 8, !tbaa !6
  //   br label %omp.uncollapsed.loop.cond33
  //
  // omp.uncollapsed.loop.cond33:
  //     ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  //   %22 = load i64, i64* %.omp.uncollapsed.iv16, align 8, !tbaa !6
  //   %23 = load i64, i64* %.omp.uncollapsed.ub24, align 8, !tbaa !6
  //   %cmp34 = icmp sle i64 %22, %23
  //   br i1 %cmp34, label %omp.uncollapsed.loop.body36,
  //                 label %omp.uncollapsed.loop.end
  //
  // omp.uncollapsed.loop.body36:
  //     ; preds = %omp.uncollapsed.loop.cond33
  //   <loop nest body>
  //
  // omp.uncollapsed.loop.inc:
  //     ; preds = %omp.uncollapsed.loop.body36
  //   %30 = load i64, i64* %.omp.uncollapsed.iv16, align 8, !tbaa !6
  //   %add43 = add nsw i64 %30, 1
  //   store i64 %add43, i64* %.omp.uncollapsed.iv16, align 8, !tbaa !6
  //   br label %omp.uncollapsed.loop.cond33
  //
  // omp.uncollapsed.loop.end:
  //     ; preds = %omp.uncollapsed.loop.cond33
  //   br label %omp.uncollapsed.loop.inc44
  //
  // omp.uncollapsed.loop.inc44:
  //     ; preds = %omp.uncollapsed.loop.end
  //   %31 = load i64, i64* %.omp.uncollapsed.iv, align 8, !tbaa !6
  //   %add45 = add nsw i64 %31, 1
  //   store i64 %add45, i64* %.omp.uncollapsed.iv, align 8, !tbaa !6
  //   br label %omp.uncollapsed.loop.cond
  //
  // omp.uncollapsed.loop.end46:
  //     ; preds = %omp.uncollapsed.loop.cond
  //   call void @llvm.directive.region.exit(token %17) [
  //       "DIR.OMP.END.PARALLEL.LOOP"() ]
  //   br label %omp.precond.end
  //
  // We are going to wrap a loop around the outermost loop,
  // and set up the original loops' lower/upper bounds such that
  // they become loops of exactly one iteration:
  // omp.precond.then:
  //   ; collapsed upper bound computation inserted here:
  //   %.omp.uncollapsed.ub.val = load i64, i64* %.omp.uncollapsed.ub
  //   %17 = add nuw nsw i64 %.omp.uncollapsed.ub.val, 1
  //   %.omp.uncollapsed.ub24.val = load i64, i64* %.omp.uncollapsed.ub24
  //   %18 = add nuw nsw i64 %.omp.uncollapsed.ub24.val, 1
  //   %19 = mul nuw nsw i64 %17, %18
  //   %omp.collapsed.ub.value = sub nuw nsw i64 %19, 1
  //   store i64 0, i64* %omp.collapsed.lb
  //   store i64 %omp.collapsed.ub.value, i64* %omp.collapsed.ub
  //   %20 = call token @llvm.directive.region.entry() [
  //       "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.COLLAPSE"(i32 2),
  //       "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb),
  //       "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb23),
  //       "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %j),
  //       ; collapsed IV/LB/UB clauses created later
  //       "QUAL.OMP.NORMALIZED.IV"(i64* %omp.collapsed.iv),
  //       "QUAL.OMP.NORMALIZED.UB"(i64* %omp.collapsed.ub),
  //       "QUAL.OMP.FIRSTPRIVATE"(i64* %omp.collapsed.lb),
  //       ; original IVs are made private
  //       "QUAL.OMP.PRIVATE"(i64* %.omp.uncollapsed.iv,
  //                          i64* %.omp.uncollapsed.iv16),
  //       ; original UBs are made private
  //       "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.ub,
  //                               i64* %.omp.uncollapsed.ub24) ]
  //   br label [[DimInitBB]]
  //
  // [[DimInitBB]]:
  //     ; preds = %omp.precond.then
  //   %.omp.uncollapsed.ub24.val1 = load i64, i64* %.omp.uncollapsed.ub24
  //   %21 = add nuw nsw i64 %.omp.uncollapsed.ub24.val1, 1
  //   %22 = mul nuw nsw i64 %21, 1
  //   br label [[OldOuterLoopPreheader]]
  //
  // [[OldOuterLoopPreheader]]:
  //     ; preds = [[DimInitBB]]
  //   %23 = load i64, i64* %.omp.uncollapsed.lb, align 8, !tbaa !6
  //   store i64 %23, i64* %.omp.uncollapsed.iv, align 8, !tbaa !6
  //   br label [[IVInitBB]]
  //
  // [[IVInitBB]]:
  //     ; [[OldOuterLoopPreheader]]
  //   %24 = load i64, i64* %omp.collapsed.lb
  //   store i64 %24, i64* %omp.collapsed.iv
  //   br label %omp.collapsed.loop.cond
  //
  // omp.collapsed.loop.cond:
  //     ; preds = %omp.collapsed.loop.inc, [[IVInitBB]]
  //   %25 = load i64, i64* %omp.collapsed.iv
  //   %26 = load i64, i64* %omp.collapsed.ub
  //   %27 = icmp sle i64 %25, %26
  //   br i1 %27, label %omp.collapsed.loop.body,
  //              label %omp.collapsed.loop.exit, !prof !8
  //
  // omp.collapsed.loop.body:
  //     ; preds = %omp.collapsed.loop.cond
  //   ; compute uncollapsed loop bounds based on the combined
  //   ; IV, and update the uncollapsed LB/UB variables.
  //   %omp.collapsed.iv.val = load i64, i64* %omp.collapsed.iv
  //   %28 = sdiv i64 %omp.collapsed.iv.val, %22
  //   store i64 %28, i64* %.omp.uncollapsed.lb
  //   store i64 %28, i64* %.omp.uncollapsed.ub
  //   %29 = srem i64 %omp.collapsed.iv.val, %22
  //   ; note that %.omp.uncollapsed.iv initialization is left
  //   ; outside of the new loop in [[OldOuterLoopPreheader]],
  //   ; thus we initialize it here:
  //   store i64 %28, i64* %.omp.uncollapsed.iv
  //   %30 = sdiv i64 %29, 1
  //   store i64 %30, i64* %.omp.uncollapsed.lb23
  //   store i64 %30, i64* %.omp.uncollapsed.ub24
  //   %31 = srem i64 %29, 1
  //   br label %omp.uncollapsed.loop.cond
  //
  // omp.uncollapsed.loop.cond:
  //     ; preds = %omp.uncollapsed.loop.inc44, %omp.collapsed.loop.body
  //   %19 = load i64, i64* %.omp.uncollapsed.iv, align 8, !tbaa !6
  //   %20 = load i64, i64* %.omp.uncollapsed.ub, align 8, !tbaa !6
  //   %cmp31 = icmp sle i64 %19, %20
  //   br i1 %cmp31, label %omp.uncollapsed.loop.body,
  //                 label %omp.collapsed.loop.inc
  //   ; note that the new exit for the old outermost loop
  //   ; is the new loop's increment block %omp.collapsed.loop.inc
  //
  // omp.uncollapsed.loop.body:
  //   ; old outermost loop's body
  //   ...
  //
  // omp.collapsed.loop.inc:
  //     ; preds = %omp.uncollapsed.loop.cond
  //   ; increment the collapsed IV
  //   %45 = load i64, i64* %omp.collapsed.iv
  //   %46 = add nuw nsw i64 %45, 1
  //   store i64 %46, i64* %omp.collapsed.iv
  //   br label %omp.collapsed.loop.cond
  //
  // omp.collapsed.loop.exit:
  //     ; preds = %omp.collapsed.loop.cond
  //   br label %omp.collapsed.loop.postexit

  // omp.collapsed.loop.postexit:
  //     ; preds = %omp.collapsed.loop.exit
  //   call void @llvm.directive.region.exit(token %20) [
  //       "DIR.OMP.END.PARALLEL.LOOP"() ]
  //   br label %omp.precond.end

  // Find blocks to serve as the new loop's header and increment.
  auto *OutermostLoopExit = OutermostLoop->getExitBlock();
  assert(OutermostLoopExit && "OpenMP loop must have one exit block.");
  // Normalized loops generated by FE for OpenMP loop constructs
  // have a pre-header, which is usually the region's entry block.
  auto *OutermostLoopPreheader = OutermostLoop->getLoopPreheader();
  assert(OutermostLoopPreheader && "OpenMP loop must hava a pre-header.");

  // The current region's exit block will become a back-edge block
  // of the new loop.
  BasicBlock *LoopIncBB = OutermostLoopExit;
  // This block will be inside the new loop.
  assert(LoopIncBB->hasNPredecessors(1) &&
         "Loop exit block must have one predecessor.");
  BasicBlock *LoopExitBB =
    SplitBlock(LoopIncBB, &LoopIncBB->front(), DT, LI);
  W->setExitBBlock(LoopExitBB);
  LoopExitBB->setName("omp.collapsed.loop.postexit");

  BasicBlock *IVInitBB =
      SplitBlock(OutermostLoopPreheader,
                 OutermostLoopPreheader->getTerminator(), DT, LI);
  BasicBlock *LoopContentStart = IVInitBB->getSingleSuccessor();
  assert(LoopContentStart && "IV init block must have one successor.");

  IRBuilder<> IVInitBuilder(&IVInitBB->front());
  IVInitBuilder.CreateStore(
      IVInitBuilder.CreateLoad(CombinedUBType, NewLBPtrDef), NewIVPtrDef);

  BasicBlock *LoopCondBB =
    SplitBlock(IVInitBB, IVInitBB->getTerminator(), DT, LI);
  // This block will be inside the new loop.
  LoopCondBB->setName("omp.collapsed.loop.cond");
  IRBuilder<> LoopCondBuilder(&LoopCondBB->front());
  Value *IVLoad = LoopCondBuilder.CreateLoad(CombinedUBType, NewIVPtrDef);
  Value *LoadUB = LoopCondBuilder.CreateLoad(CombinedUBType, NewUBPtrDef);
  Value *CmpI = LoopCondBuilder.CreateICmpSLE(IVLoad, LoadUB);
  Instruction *ElseTerm = LoopCondBB->getTerminator();
  // Split the block: the Then block will jump to the loop
  // body, the Else block is the loop exit, which will jump
  // to the loop post-exit (the current outermost loop's exit block).
  Instruction *ThenTerm =
      SplitBlockAndInsertIfThen(
          CmpI, LoopCondBB->getTerminator(), false,
          MDBuilder(F->getContext()).createBranchWeights(99, 1),
          DT, LI);
  // This block will be inside the new loop.
  BasicBlock *LoopBodyBB = ThenTerm->getParent();
  LoopBodyBB->setName("omp.collapsed.loop.body");
  ThenTerm->setSuccessor(0, LoopContentStart);
  ElseTerm->setSuccessor(0, LoopExitBB);
  ElseTerm->getParent()->setName("omp.collapsed.loop.exit");

  // Compute the original loops' iteration spaces (which is just
  // one iteration) based on the combined IV.
  IRBuilder<> ThenBuilder(ThenTerm);
  Value *NewBndVal =
      ThenBuilder.CreateLoad(CombinedUBType, NewIVPtrDef,
                             Twine(NewIVPtrDef->getName()) + Twine(".val"));
  for (unsigned Idx = 0; Idx < NumLoops; ++Idx) {
    Value *UpdateVal = ThenBuilder.CreateSDiv(NewBndVal, Dimensions[Idx]);
    // Store the value into LB and UB, so that the original loop runs
    // exactly one iteration.
    Value *StoreVal =
        ThenBuilder.CreateZExtOrTrunc(UpdateVal, LBPtrDefs[Idx].second);
    ThenBuilder.CreateStore(StoreVal, LBPtrDefs[Idx].first);
    Type *StorePtrElemTy = (W->getWRNLoopInfo().getNormUBElemTy(Idx));
    StoreVal = ThenBuilder.CreateZExtOrTrunc(UpdateVal, StorePtrElemTy);
    ThenBuilder.CreateStore(StoreVal, W->getWRNLoopInfo().getNormUB(Idx));
    NewBndVal = ThenBuilder.CreateSRem(NewBndVal, Dimensions[Idx]);

    // The original outermost loop's pre-header contains the following
    // initialization for the outermost loop's IV:
    //   %18 = load i64, i64* %.omp.uncollapsed.lb, align 8, !tbaa !6
    //   store i64 %18, i64* %.omp.uncollapsed.iv, align 8, !tbaa !6
    //
    // This code is not inside the collapsed loop anymore,
    // so we need to initialize the IV here. This needs to be done
    // only for the outermost IV, since other IVs initializations
    // are dominated by this block.
    if (Idx == 0) {
      StorePtrElemTy = (W->getWRNLoopInfo().getNormIVElemTy(0));
      StoreVal = ThenBuilder.CreateZExtOrTrunc(UpdateVal, StorePtrElemTy);
      ThenBuilder.CreateStore(StoreVal, W->getWRNLoopInfo().getNormIV(0));
    }
  }

  // Set up the back edge and the loop increment.
  LoopIncBB->setName("omp.collapsed.loop.inc");
  LoopIncBB->getTerminator()->setSuccessor(0, LoopCondBB);
  IRBuilder<> IncBuilder(LoopIncBB->getTerminator());
  IncBuilder.CreateStore(
      IncBuilder.CreateAdd(IncBuilder.CreateLoad(CombinedUBType, NewIVPtrDef),
                           ConstantInt::get(CombinedUBType, 1),
                           "", true, true),
      NewIVPtrDef);

  // Update the DominatorTree.
  if (DT) {
    DT->changeImmediateDominator(LoopContentStart, LoopBodyBB);
    DT->changeImmediateDominator(LoopExitBB, ElseTerm->getParent());
  }

  // Update the LoopInfo.
  if (LI) {
    Loop *ParentLoop = OutermostLoop->getParentLoop();
    Loop *NewLoop = WRegionUtils::createLoop(OutermostLoop, ParentLoop, LI);
    // These three blocks were split from the outermost
    // loop's pre-header. They are the only blocks that
    // belong to the new loop and do not belong to the inner
    // loops. If there is a parent loop, then these blocks
    // are currently owned by this parent loop, while
    // they have to be owned by the new loop.
    // And we have to add them into the new loop with addBlockEntry()
    // anyway.
    for (auto *BB : { LoopIncBB, LoopCondBB, LoopBodyBB }) {
      NewLoop->addBlockEntry(BB);
      LI->changeLoopFor(BB, NewLoop);
    }
    NewLoop->moveToHeader(LoopCondBB);
  }
#ifndef NDEBUG
  assert((!DT || DT->verify()) && "DominatorTree is invalid");
  if (LI && DT)
    LI->verify(*DT);
#endif  // NDEBUG

  // Update OpenMP clauses in IR.
  SmallVector<Value *, 3> OldIVPtrDefs;
  SmallVector<Value *, 3> OldUBPtrDefs;
  for (unsigned I = 0, IE = W->getWRNLoopInfo().getNormIVSize(); I != IE; ++I) {
    OldIVPtrDefs.push_back(W->getWRNLoopInfo().getNormIV(I));
    OldUBPtrDefs.push_back(W->getWRNLoopInfo().getNormUB(I));
  }
  CallInst *EntryCI = cast<CallInst>(W->getEntryDirective());
  StringRef IVString =
      VPOAnalysisUtils::getClauseString(QUAL_OMP_NORMALIZED_IV);
  StringRef UBString =
      VPOAnalysisUtils::getClauseString(QUAL_OMP_NORMALIZED_UB);
  EntryCI =
      VPOUtils::removeOperandBundlesFromCall(EntryCI, {IVString, UBString});
  StringRef FirstPrivateString =
    VPOAnalysisUtils::getClauseString(QUAL_OMP_FIRSTPRIVATE);
  StringRef PrivateString =
    VPOAnalysisUtils::getClauseString(QUAL_OMP_PRIVATE);
  assert(W->canHavePrivate() && "OpenMP loop region cannot have PRIVATE?");
  EntryCI = VPOUtils::addOperandBundlesInCall(EntryCI,
                                              {{IVString, {NewIVPtrDef}},
                                               {UBString, {NewUBPtrDef}},
                                               {PrivateString, OldIVPtrDefs}});
  if (W->canHaveFirstprivate()) {
    // SIMD cannot have firstprivate() clause.
    EntryCI = VPOUtils::addOperandBundlesInCall(
        EntryCI, {// We should probably make LB PRIVATE, since we assume that
                  // it is always 0. For the time being make it FIRSTPRIVATE,
                  // just as FE does.
                  {FirstPrivateString, {NewLBPtrDef}},
                  // The old UBs' values are used inside the region
                  // to compute the dimensions.
                  {FirstPrivateString, OldUBPtrDefs}});
  }
  W->setEntryDirective(EntryCI);

  WRegionNode *P = W;
  // We need to mark new LB and UB as firstprivate (or alternatively
  // pass their value) from the point of their initialization
  // up to the current region. If we hoisted the initialization
  // before the outter "omp target", then we need to mark
  // them up to this target region.
  bool PassedTarget = !HoistCombinedUBBeforeTarget;

  while ((P = P->getParent())) {
#if INTEL_CUSTOMIZATION
    // We generate explicit stores to the new LB and UB variables.
    // Stores inside target and teams regions are considered to be side-effect
    // instructions, which need to be guarded with master thread check
    // and synchronized with barriers for SPIR-V targets.
    // At the same time, inside the parallel regions we only really read
    // from the new LB/UB variables, so they perfectly fit for WILOCAL
    // markup for target and teams.
    bool MarkLBUBWILocal = isTargetSPIRV() &&
        (isa<WRNTeamsNode>(P) || isa<WRNTargetNode>(P));
#endif  // INTEL_CUSTOMIZATION
    StringRef FPString;
    if (P->canHaveFirstprivate())
      FPString = VPOAnalysisUtils::getClauseString(QUAL_OMP_FIRSTPRIVATE);
    else if (P->canHaveShared())
      FPString = VPOAnalysisUtils::getClauseString(QUAL_OMP_SHARED);

    StringRef PrivateString;
    if (P->canHavePrivate())
      PrivateString = VPOAnalysisUtils::getClauseString(QUAL_OMP_PRIVATE);
    else if (P->canHaveShared())
      PrivateString = VPOAnalysisUtils::getClauseString(QUAL_OMP_SHARED);

    CallInst *EntryCI = cast<CallInst>(P->getEntryDirective());
    if (PassedTarget) {
      if (!PrivateString.empty()) {
        std::string ClauseString = PrivateString.str();
#if INTEL_CUSTOMIZATION
        // WILOCAL modifier only makes sense for [FIRST]PRIVATE clauses.
        // Target and teams do support [FIRST]PRIVATE.
        if (MarkLBUBWILocal)
          ClauseString += ":WILOCAL";
#endif  // INTEL_CUSTOMIZATION
        EntryCI = VPOUtils::addOperandBundlesInCall(
            EntryCI, {{ClauseString, {NewLBPtrDef, NewUBPtrDef}}});
      }
    } else if (!FPString.empty()) {
      std::string ClauseString = FPString.str();
#if INTEL_CUSTOMIZATION
      if (MarkLBUBWILocal)
        ClauseString += ":WILOCAL";
#endif  // INTEL_CUSTOMIZATION
      EntryCI = VPOUtils::addOperandBundlesInCall(
          EntryCI, {{ClauseString, {NewLBPtrDef, NewUBPtrDef}}});
    }

    if (!PrivateString.empty())
      // IV is always private for the parent regions.
      EntryCI = VPOUtils::addOperandBundlesInCall(
          EntryCI, {{PrivateString, {NewIVPtrDef}}});

    P->setEntryDirective(EntryCI);

    if (!PassedTarget && isa<WRNTargetNode>(P))
      // Everything is private from now on.
      PassedTarget = true;
  }

  if (SetNDRange) {
    if (Use1DRange) {
      assert(HoistCombinedUBBeforeTarget &&
             "Inconsistent use of Use1DRange and HoistCombinedUBBeforeTarget.");
      // Add the combined upper bound to OFFLOAD.NDRANGE clause.
      // This is not strictly required to create OFFLOAD.NDRANGE,
      // because the ND-range parallelization will kick in just
      // by the fact that the combined UB is rematerializable before
      // the target region. Just add it for consistency.
      setNDRangeClause(WTarget, W, {NewUBPtrDef});
    } else
      // Add the original loops' upper bounds to OFFLOAD.NDRANGE clause.
      setNDRangeClause(WTarget, W, W->getWRNLoopInfo().getNormUBs());
  }
  return Exiter(true);
}

void VPOParoptTransform::setNDRangeClause(
    WRegionNode *WT, WRegionNode *WL, ArrayRef<Value *> NDRangeDims) const {
  assert(!NDRangeDims.empty() && NDRangeDims.size() <= 3 &&
         "Invalid number of ND-range dimensions.");
  CallInst *EntryCI = cast<CallInst>(WT->getEntryDirective());
  StringRef Clause =
      VPOAnalysisUtils::getClauseString(QUAL_OMP_OFFLOAD_NDRANGE);

  EntryCI = VPOUtils::addOperandBundlesInCall(EntryCI, {{Clause, NDRangeDims}});
  WT->setEntryDirective(EntryCI);

  // The region's loop(s) now have its tripcounts in the NDRANGE clause
  // of the "omp target" region. Mark it as such.
  EntryCI = cast<CallInst>(WL->getEntryDirective());
  Clause = VPOAnalysisUtils::getClauseString(QUAL_OMP_OFFLOAD_KNOWN_NDRANGE);
  EntryCI = VPOUtils::addOperandBundlesInCall(EntryCI, {{Clause, {}}});
  WL->setEntryDirective(EntryCI);
}

// Look for enclosed OpenMP loop regions and try to use
// different ND-range dimensions for different nesting levels.
void VPOParoptTransform::assignParallelDimensions() const {
  // Right now we only handle the case, where "omp distribute"
  // is lexically enclosed in "omp target".
  if (!AllowDistributeDimension)
    return;

  for (auto *W : WRegionList) {
    if (!W->getIsOmpLoop())
      continue;

    // Only handle "omp distribute" with known loop bounds.
    if (!W->getWRNLoopInfo().isKnownNDRange())
      continue;

    if (!WRegionUtils::isDistributeNode(W))
      continue;

    // "omp distribute" will start with dimension 1.
    // Currently, we always collapse "omp distribute" loops,
    // so there is no way to overflow 3 dimensions during
    // partitioning.
    W->getWRNLoopInfo().setNDRangeStartDim(1);

    WRegionNode *WTarget =
        WRegionUtils::getParentRegion(W, WRegionNode::WRNTarget);
    assert(WTarget &&
           "Unexpected known ND-range with no parent target region.");
    // Mark dimension 1 as the "distribute dimension". This tells
    // the runtime to make sure that all lower dimensions specify
    // exactly one working group, e.g. GlobalSize[0] == 16 and
    // LocalSize[0] == 16, whereas dimension 1 will use known
    // loop bounds of the distribute loop, i.e. GlobalSize[1] == N,
    // LocalSize[1] == 1. This way each iteration of the distribute
    // loop will be run by one WG, and the inner parallel loops (if any)
    // will be run by WIs in this WG.
    WTarget->setNDRangeDistributeDim(1);
  }
}

// Reset QUAL_OMP_OFFLOAD_KNOWN_NDRANGE clauses for OpenMP loop regions
// that should not use SPIR partitioning with known loop bounds.
bool VPOParoptTransform::fixupKnownNDRange(WRegionNode *W) const {
  if (!W->getIsOmpLoop())
    return false;

  if (!W->getWRNLoopInfo().isKnownNDRange())
    return false;

  assert(VPOParoptUtils::getSPIRExecutionScheme() ==
         spirv::ImplicitSIMDSPMDES &&
         "Unexpected known ND-range with disabled ND-range parallelization.");

  WRegionNode *WTarget =
      WRegionUtils::getParentRegion(W, WRegionNode::WRNTarget);
  assert(WTarget && "Unexpected known ND-range with no parent target region.");

  bool RemoveKnownNDRange = false;

  // Reductions require global locks. With SPMD mode there will be
  // too many of them (basically, a lock per each sub-group) and it
  // will result in too long serial sequence of updates.
#if INTEL_CUSTOMIZATION
  // CMPLRLLVM-10535, CMPLRLLVM-10704, CMPLRLLVM-11080.
#endif  // INTEL_CUSTOMIZATION
  if (W->canHaveReduction() && !W->getRed().items().empty())
    RemoveKnownNDRange = true;

  // Check if there is an enclosing teams region.
  WRegionNode *WTeams = WRegionUtils::getParentRegion(W, WRegionNode::WRNTeams);
  if (!WTeams && !VPOParoptUtils::getSPIRImplicitMultipleTeams())
    // "omp target parallel for" is not allowed to use multiple WGs implicitly.
    // It has to use one team/WG by specification.
    RemoveKnownNDRange = true;

  // "omp teams" with num_teams() clause overrules ImplicitSIMDSPMDES mode.
  //
  // Emit opt-report only if we can actually use ND-range parallelization,
  // i.e. NDInfoAI is not null, otherwise, the report will be misleading.
  if (WTeams && WTeams->getNumTeams()) {
    if (isTargetSPIRV()) {
      // Emit opt-report only during SPIR compilation.
      OptimizationRemarkMissed R("openmp", "Target", W->getEntryDirective());
      R << "Performance may be reduced due to the enclosing teams region " <<
          "specifying num_teams";
      ORE.emit(R);
    }
    RemoveKnownNDRange = true;
  }

  if (!RemoveKnownNDRange)
    return false;

  // Remove QUAL.OMP.OFFLOAD.KNOWN.NDRANGE clause from the loop region.
  CallInst *EntryCI = cast<CallInst>(W->getEntryDirective());
  StringRef ClauseName =
      VPOAnalysisUtils::getClauseString(QUAL_OMP_OFFLOAD_KNOWN_NDRANGE);
  EntryCI = VPOUtils::removeOperandBundlesFromCall(EntryCI, ClauseName);
  W->setEntryDirective(EntryCI);
  W->getWRNLoopInfo().resetKnownNDRange();

  // Remove QUAL.OMP.OFFLOAD.NDRANGE from the parent target region.
  EntryCI = cast<CallInst>(WTarget->getEntryDirective());
  ClauseName = VPOAnalysisUtils::getClauseString(QUAL_OMP_OFFLOAD_NDRANGE);
  EntryCI = VPOUtils::removeOperandBundlesFromCall(EntryCI, ClauseName);
  WTarget->setEntryDirective(EntryCI);
  WTarget->resetUncollapsedNDRangeDimensions();

  return true;
}

bool VPOParoptTransform::addRangeMetadataToOmpCalls() const {
  // Set of function's blocks outside of any work region. Initially it contains
  // all function's block, but we'll be removing work region's blocks from it
  // while processing them.
  SmallPtrSet<BasicBlock *, 8u> OrphanBlocks;
  for (BasicBlock &BB : *F)
    OrphanBlocks.insert(&BB);

  auto GetConstantValue = [](Value *V) -> Optional<APInt> {
    if (auto *CI = dyn_cast_or_null<ConstantInt>(V))
      return CI->getValue();
    return None;
  };

  auto GetParentValue = [](WRegionNode *W,
                           SmallDenseMap<WRegionNode *, Optional<APInt>> &Map)
      -> Optional<APInt> {
    if (WRegionNode *PW = W->getParent())
      return Map[PW];
    return None;
  };

  // Propagate number of teams and threads from outer regions to inner.
  SmallDenseMap<WRegionNode *, Optional<APInt>> W2NumTeams;
  SmallDenseMap<WRegionNode *, Optional<APInt>> W2NumThreads;
  for (WRegionNode *W : reverse(WRegionList)) {
    W->populateBBSet();

    // Update set of blocks which do not belong to any work region.
    for (BasicBlock *BB : W->blocks())
      OrphanBlocks.erase(BB);

    // The number of teams and threads can be changed by teams, parallel and
    // target constructs. Other work regions inherit values from the parent
    // region.
    if (W->getIsTeams()) {
      W2NumTeams[W] = GetConstantValue(W->getNumTeams());
      W2NumThreads[W] = GetConstantValue(W->getThreadLimit());
      continue;
    }
    if (W->getIsPar()) {
      W2NumTeams[W] = GetParentValue(W, W2NumTeams);
      W2NumThreads[W] = GetConstantValue(W->getNumThreads());
      continue;
    }
    if (W->getIsTarget()) {
      W2NumTeams[W] = None;
      W2NumThreads[W] = None;
      continue;
    }
    W2NumTeams[W] = GetParentValue(W, W2NumTeams);
    W2NumThreads[W] = GetParentValue(W, W2NumThreads);
  }

  // Adds [Lo, Hi) range metadata to the given call instruction. If upper
  // bound is not specified then max positive value of the call's result type
  // is used.
  auto AddRangeMD = [](CallBase *Call, int64_t Lo, Optional<APInt> Hi) {
    auto *Ty = cast<IntegerType>(Call->getType());
    APInt Max = Hi ? *Hi : APInt::getSignedMaxValue(Ty->getBitWidth());
    MDNode *RangeMD =
        MDBuilder(Call->getContext())
            .createRange(ConstantInt::get(Ty, Lo),
                         ConstantInt::get(Ty, Max.getSExtValue()));
    Call->setMetadata(LLVMContext::MD_range, RangeMD);
  };

  // Annotate OpenMP API calls in the given basic block using provided number of
  // teams and threads in the block.
  auto AnnotateCalls = [this, &AddRangeMD](BasicBlock *BB,
                                           Optional<APInt> NumTeams,
                                           Optional<APInt> NumThreads) {
    bool Changed = false;
    for (Instruction &I : *BB) {
      auto *Call = dyn_cast<CallBase>(&I);
      if (!Call || Call->getMetadata(LLVMContext::MD_range))
        continue;

      Function *Callee = Call->getCalledFunction();
      if (!Callee)
        continue;

      LibFunc TheLibFunc;
      if (!TLI->getLibFunc(*Callee, TheLibFunc) || !TLI->has(TheLibFunc))
        continue;

      switch (TheLibFunc) {
      case LibFunc_omp_get_num_teams:
        AddRangeMD(Call, 1, NumTeams ? *NumTeams + 1u : Optional<APInt>{None});
        Changed = true;
        break;
      case LibFunc_omp_get_team_num:
        AddRangeMD(Call, 0, NumTeams);
        Changed = true;
        break;
      case LibFunc_omp_get_num_threads:
        AddRangeMD(Call, 1,
                   NumThreads ? *NumThreads + 1u : Optional<APInt>{None});
        Changed = true;
        break;
      case LibFunc_omp_get_thread_num:
        AddRangeMD(Call, 0, NumThreads);
        Changed = true;
        break;
      default:
        break;
      }
    }
    return Changed;
  };

  // Walk work regions inner to outer and annotate OpenMP calls in work region's
  // blocks.
  bool Changed = false;
  for (WRegionNode *W : WRegionList) {
    for (BasicBlock *BB : W->blocks())
      Changed |= AnnotateCalls(BB, W2NumTeams[W], W2NumThreads[W]);
    W->resetBBSet();
  }

  // And finally annotate calls in function's blocks outside of any work region.
  for (BasicBlock *BB : OrphanBlocks)
    Changed |= AnnotateCalls(BB, None, None);

  return Changed;
}
#endif // INTEL_COLLAB
