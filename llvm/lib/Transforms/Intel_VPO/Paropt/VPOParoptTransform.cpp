#if INTEL_COLLAB
//===- VPOParoptTransform.cpp - Transformation of W-Region for threading --===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation. and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
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

#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptAtomics.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"

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
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"

#include "llvm/PassAnalysisSupport.h"

#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"

#include "llvm/Transforms/Utils/Intel_GeneralUtils.h"
#include "llvm/Transforms/Utils/Intel_IntrinsicUtils.h"
#include "llvm/Transforms/Utils/LoopRotationUtils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"

#include <algorithm>
#include <set>
#include <vector>

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-transform"

// TODO FIXME remove this after full implementation of HIR-based loop
// transformation for CSA
cl::opt<bool> UseOmpRegionsInLoopopt(
  "loopopt-use-omp-region", cl::init(false), cl::ReallyHidden,
  cl::desc("; TEMPORARY"));

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
void VPOParoptTransform::genLoopBoundUpdatePrep(WRegionNode *W,
                                                AllocaInst *&LowerBnd,
                                                AllocaInst *&UpperBnd) {
  Loop *L = W->getWRNLoopInfo().getLoop();
  assert(L->isLoopSimplifyForm() &&
         "genLoopBoundUpdatePrep: Expect the loop is in SimplifyForm.");
  ICmpInst *CmpI = WRegionUtils::getOmpLoopZeroTripTest(L, W->getEntryBBlock());
  if (CmpI)
    W->getWRNLoopInfo().setZTTBB(CmpI->getParent());

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

  if (InitVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    InitVal = Builder.CreateSExtOrTrunc(InitVal, IndValTy);

  Builder.CreateStore(InitVal, LowerBnd);

  Value *UpperBndVal = VPOParoptUtils::computeOmpUpperBound(W, InsertPt);

  if (UpperBndVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    UpperBndVal = Builder.CreateSExtOrTrunc(UpperBndVal, IndValTy);

  Builder.CreateStore(UpperBndVal, UpperBnd);
}

// Generate the OCL loop update code.
void VPOParoptTransform::genOCLLoopBoundUpdateCode(WRegionNode *W,
                                                   AllocaInst *LowerBnd,
                                                   AllocaInst *UpperBnd) {
  Loop *L = W->getWRNLoopInfo().getLoop();
  Instruction *InsertPt =
      cast<Instruction>(L->getLoopPreheader()->getTerminator());
  LLVMContext &C = F->getContext();
  IRBuilder<> Builder(InsertPt);
  ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);
  ConstantInt *ValueOne = ConstantInt::get(Type::getInt32Ty(C), 1);
  Value *Arg[] = {ValueZero};
  CallInst *LocalSize =
      VPOParoptUtils::genOCLGenericCall("_Z14get_local_sizej", Arg, InsertPt);
  Value *NumThreads = Builder.CreateSExtOrTrunc(LocalSize, Type::getInt32Ty(C));
  Value *LB = Builder.CreateLoad(LowerBnd);
  Value *UB = Builder.CreateLoad(UpperBnd);
  Value *ItSpace = Builder.CreateSub(UB, LB);

  WRNScheduleKind SchedKind = VPOParoptUtils::getLoopScheduleKind(W);

  Value *Chunk = nullptr;
  if (SchedKind == WRNScheduleStaticEven ||
      SchedKind == WRNScheduleOrderedStaticEven) {
    Value *ItSpaceRounded = Builder.CreateAdd(ItSpace, NumThreads);
    Chunk = Builder.CreateSDiv(ItSpaceRounded, NumThreads);
  } else
    Chunk = ValueOne;

  CallInst *LocalId =
      VPOParoptUtils::genOCLGenericCall("_Z12get_local_idj", Arg, InsertPt);
  Value *LocalIdCasted =
      Builder.CreateSExtOrTrunc(LocalId, Type::getInt32Ty(C));
  Value *LBDiff = Builder.CreateMul(LocalIdCasted, Chunk);
  LB = Builder.CreateAdd(LB, LBDiff);
  Builder.CreateStore(LB, LowerBnd);

  Value *Ch = Builder.CreateSub(Chunk, ValueOne);
  Value *NewUB = Builder.CreateAdd(LB, Ch);

  Value *Compare = Builder.CreateICmp(ICmpInst::ICMP_ULT, NewUB, UB);
  TerminatorInst *ThenTerm = SplitBlockAndInsertIfThen(
      Compare, InsertPt, false,
      MDBuilder(F->getContext()).createBranchWeights(99999, 100000), DT, LI);
  BasicBlock *ThenBB = ThenTerm->getParent();
  ThenBB->setName("then.bb.");
  IRBuilder<> BuilderThen(ThenBB);
  BuilderThen.SetInsertPoint(ThenBB->getTerminator());
  BuilderThen.CreateStore(NewUB, UpperBnd);
}

// Generate the OCL loop scheduling code.
void VPOParoptTransform::genOCLLoopPartitionCode(WRegionNode *W,
                                                 AllocaInst *LowerBnd,
                                                 AllocaInst *UpperBnd) {

  Loop *L = W->getWRNLoopInfo().getLoop();
  DenseMap<Value *, std::pair<Value *, BasicBlock *>> ValueToLiveinMap;
  SmallSetVector<Instruction *, 8> LiveOutVals;
  EquivalenceClasses<Value *> ECs;
  wrnUpdateSSAPreprocess(L, ValueToLiveinMap, LiveOutVals, ECs);

  Instruction *InsertPt =
      dyn_cast<Instruction>(L->getLoopPreheader()->getTerminator());
  IRBuilder<> Builder(InsertPt);

  LoadInst *LoadLB = Builder.CreateLoad(LowerBnd);
  LoadInst *LoadUB = Builder.CreateLoad(UpperBnd);

  BasicBlock *StaticInitBB = InsertPt->getParent();

  PHINode *PN = WRegionUtils::getOmpCanonicalInductionVariable(L);
  PN->removeIncomingValue(L->getLoopPreheader());
  PN->addIncoming(LoadLB, L->getLoopPreheader());

  BasicBlock *LoopExitBB = WRegionUtils::getOmpExitBlock(L);

  bool IsLeft;
  CmpInst::Predicate PD = VPOParoptUtils::computeOmpPredicate(
      WRegionUtils::getOmpPredicate(L, IsLeft));
  ICmpInst *CompInst;
  CompInst = new ICmpInst(InsertPt, PD, LoadLB, LoadUB, "");

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
  TerminatorInst *NewTermInst =
      BranchInst::Create(PreHdrInst->getSuccessor(0), LoopExitBB, CompInst);
  ReplaceInstWithInst(InsertPt, NewTermInst);

  if (DT)
    DT->changeImmediateDominator(LoopExitBB, StaticInitBB);

  wrnUpdateLiveOutVals(L, LoopRegionExitBB, LiveOutVals, ECs);
  rewriteUsesOfOutInstructions(ValueToLiveinMap, LiveOutVals, ECs);
}
// Generate the iteration space partitioning code based on OpenCL.
// Given a loop as follows.
//   for (i = lb; i <= ub; i++)
// The output of partitioning as below.
//   chunk_size = (ub - lb + get_local_size()) / get_local_size();
//   new_lb = lb + get_local_id * chunk_size;
//   new_ub = min(lb + chunk_size - 1, ub);
//   for (i = new_lb; i <= new_ub; i++)
// Here we assume the global_size is equal to local_size, which means
// there is only one workgroup.
//
bool VPOParoptTransform::genOCLParallelLoop(WRegionNode *W) {

  AllocaInst *LowerBnd, *UpperBnd;

  genLoopBoundUpdatePrep(W, LowerBnd, UpperBnd);

  genOCLLoopBoundUpdateCode(W, LowerBnd, UpperBnd);

  genOCLLoopPartitionCode(W, LowerBnd, UpperBnd);

  return true;
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

  processDeviceTriples();

  BasicBlock::iterator I = F->getEntryBlock().begin();

  // Setup Anchor Instuction Point
  Instruction *AI = &*I;

  //
  // Create the LOC structure. The format is based on OpenMP KMP library
  //
  // typedef struct {
  //   kmp_int32 reserved_1;   // might be used in Fortran
  //   kmp_int32 flags;        // also f.flags; KMP_IDENT_xxx flags;
  //                           // KMP_IDENT_KMPC identifies this union member
  //   kmp_int32 reserved_2;   // not really used in Fortran any more
  //   kmp_int32 reserved_3;   // source[4] in Fortran, do not use for C++
  //   char      *psource;
  // } ident_t;
  //
  // The bits that the flags field can hold are defined as KMP_IDENT_* before.
  //
  // Note: IdentTy needs to be an anonymous struct. This is because if we use
  // a named struct type, then different Types are created for each function
  // encountered. For example, consider a module with two functions `foo1()`
  // and `foo2()`. When handling foo1(), a named struct type `ident_t` would
  // be created and used for generating function declarations and calls for
  // KMPC routines such as `__kmpc_global_thread_num(ident_t*)`. When it comes
  // to handling `foo2()`, a new named IdentTy would be created, say
  // `ident_t.0`, but when trying to emit a call to `__kmpc_global_thread_num`,
  // there would be a type mismatch between the expected argument type in the
  // declaration (ident_t *) and actual type of the argument (ident_t.0 *).
  IdentTy = StructType::get(C, {Type::getInt32Ty(C),   // reserved_1
                                Type::getInt32Ty(C),   // flags
                                Type::getInt32Ty(C),   // reserved_2
                                Type::getInt32Ty(C),   // reserved_3
                                Type::getInt8PtrTy(C)} // *psource
                            );

  StringRef S = F->getName();

  if (!S.compare_lower(StringRef("@main"))) {
    CallInst *RI = VPOParoptUtils::genKmpcBeginCall(F, AI, IdentTy);
    RI->insertBefore(AI);

    for (BasicBlock &I : *F) {
      if (isa<ReturnInst>(I.getTerminator())) {
        Instruction *Inst = I.getTerminator();

        CallInst *RI = VPOParoptUtils::genKmpcEndCall(F, Inst, IdentTy);
        RI->insertBefore(Inst);
      }
    }
  }

  if (WI->WRGraphIsEmpty()) {
    LLVM_DEBUG(
        dbgs() << "\n... No WRegion Candidates for Parallelization ...\n\n");
    return RoutineChanged;
  }

  bool NeedTID, NeedBID;

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
  // Walk throught W-Region list, the outlining / lowering is performed from
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
    bool RemovePrivateClauses = false;

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
    if (isTargetCSA() && !isSupportedOnCSA(W))
      RemoveDirectives = true;
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
        if ((Mode & OmpPar) && (Mode & ParTrans))
          resetValueInNumTeamsAndThreadsClause(W);
      case WRegionNode::WRNParallel:
        debugPrintHeader(W, IsPrepare);
        if (Mode & ParPrepare) {
          genCodemotionFenceforAggrData(W);
          Changed |= propagateCancellationPointsToIR(W);
        }
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed = clearCodemotionFenceIntrinsic(W);
          Changed |= clearCancellationPointAllocasFromIR(W);
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
          if (!VPOAnalysisUtils::isTargetSPIRV(F->getParent()) ||
              !hasOffloadCompilation()) {
            improveAliasForOutlinedFunc(W);
            // Privatization is enabled for both Prepare and Transform passes
            Changed |= genPrivatizationCode(W);
            Changed |= genFirstPrivatizationCode(W);
            Changed |= genReductionCode(W);
            Changed |= genCancellationBranchingCode(W);
            Changed |= genDestructorCode(W);
            Changed |= genMultiThreadedCode(W);
          }
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNParallelSections:
      case WRegionNode::WRNParallelLoop:
      case WRegionNode::WRNDistributeParLoop:
        debugPrintHeader(W, IsPrepare);
        if (Mode & ParPrepare) {
          regularizeOMPLoop(W);
          genCodemotionFenceforAggrData(W);
          Changed |= propagateCancellationPointsToIR(W);
        }
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed = clearCodemotionFenceIntrinsic(W);
          Changed |= clearCancellationPointAllocasFromIR(W);
          Changed |= regularizeOMPLoop(W, false);
          improveAliasForOutlinedFunc(W);

          AllocaInst *IsLastVal = nullptr;
          BasicBlock *IfLastIterBB = nullptr;

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
          if (isTargetCSA()) {
            RemoveDirectives = true;

            if (W->getIsParSections())
              Changed |= genCSASections(W);
            else if (W->getIsParLoop()) {
              if (UseOmpRegionsInLoopopt)
                RemoveDirectives = false;
              else
                Changed |= genCSALoop(W);
            } else
              llvm_unreachable("Unexpected work region kind");
            break;
          }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          // The compiler does not need to generate the outlined function
          // for omp parallel for loop.
          if (VPOAnalysisUtils::isTargetSPIRV(F->getParent()) &&
              hasOffloadCompilation())
            Changed = genOCLParallelLoop(W);
          else {
            Changed |= genLoopSchedulingCode(W, IsLastVal);
            // Privatization is enabled for both Prepare and Transform passes
            Changed |= genPrivatizationCode(W);
            Changed |= genBarrierForFpLpAndLinears(W);
            Changed |= genLastIterationCheck(W, IsLastVal, IfLastIterBB);
            Changed |= genLinearCode(W, IfLastIterBB);
            Changed |= genLastPrivatizationCode(W, IfLastIterBB);
            Changed |= genFirstPrivatizationCode(W);
            Changed |= genReductionCode(W);
            Changed |= genCancellationBranchingCode(W);
            Changed |= genDestructorCode(W);
            Changed |= genMultiThreadedCode(W);
          }
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNTask:
        if (Mode & ParPrepare) {
          genCodemotionFenceforAggrData(W);
          Changed |= propagateCancellationPointsToIR(W);
        }
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed |= clearCancellationPointAllocasFromIR(W);
          debugPrintHeader(W, false);
          Changed = clearCodemotionFenceIntrinsic(W);
          improveAliasForOutlinedFunc(W);
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
          regularizeOMPLoop(W);
          genCodemotionFenceforAggrData(W);
        }
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed = clearCodemotionFenceIntrinsic(W);
          Changed |= regularizeOMPLoop(W, false);
          improveAliasForOutlinedFunc(W);
          StructType *KmpTaskTTWithPrivatesTy;
          StructType *KmpSharedTy;
          Value *LBPtr, *UBPtr, *STPtr, *LastIterGep;
          BasicBlock *IfLastIterBB = nullptr;
          Changed |=
              genTaskLoopInitCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy,
                                  LBPtr, UBPtr, STPtr, LastIterGep);
          Changed |= genPrivatizationCode(W);
          Changed |= genFirstPrivatizationCode(W);
          Changed |= genBarrierForFpLpAndLinears(W);
          Changed |= genLastIterationCheck(W, LastIterGep, IfLastIterBB);
          Changed |= genLastPrivatizationCode(W, IfLastIterBB);
          Changed |= genSharedCodeForTaskGeneric(W);
          Changed |= genRedCodeForTaskGeneric(W);
          Changed |= genTaskGenericCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy,
                                        LBPtr, UBPtr, STPtr);
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
        debugPrintHeader(W, IsPrepare);
        if (Mode & ParPrepare)
          genCodemotionFenceforAggrData(W);
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed = clearCodemotionFenceIntrinsic(W);
          improveAliasForOutlinedFunc(W);
          Changed |= genPrivatizationCode(W);
          Changed |= genGlobalPrivatizationCode(W);
          Changed |= genFirstPrivatizationCode(W);
          Changed |= genDevicePtrPrivationCode(W);
          Changed |= genTargetOffloadingCode(W);
          Changed |= finalizeGlobalPrivatizationCode(W);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNTargetEnterData:
      case WRegionNode::WRNTargetExitData:
        debugPrintHeader(W, IsPrepare);
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed |= genTargetOffloadingCode(W);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNTargetData:
      case WRegionNode::WRNTargetUpdate:
        debugPrintHeader(W, IsPrepare);
        if (Mode & ParPrepare)
          genCodemotionFenceforAggrData(W);
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed = clearCodemotionFenceIntrinsic(W);
          improveAliasForOutlinedFunc(W);
          Changed |= genGlobalPrivatizationCode(W);
          Changed |= genDevicePtrPrivationCode(W);
          Changed |= genTargetOffloadingCode(W);
          Changed |= finalizeGlobalPrivatizationCode(W);
          RemoveDirectives = true;
        }
        break;

      // 2. Below are constructs that do not need to perform outlining.
      //    E.g., simd, taskgroup, atomic, for, sections, etc.

      case WRegionNode::WRNTaskgroup:
        debugPrintHeader(W, IsPrepare);
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed = genTaskgroupRegion(W);
          RemoveDirectives = true;
        }
        break;

      case WRegionNode::WRNVecLoop:
        if (Mode & ParPrepare)
          regularizeOMPLoop(W);
        // Privatization is enabled for SIMD Transform passes
        if ((Mode & OmpVec) && (Mode & ParTrans)) {
          debugPrintHeader(W, false);
          Changed = regularizeOMPLoop(W, false);
          Changed |= genPrivatizationCode(W);
          // keep SIMD directives; will be processed by the Vectorizer
          RemoveDirectives = false;
          RemovePrivateClauses = false;
        }
        break;
      case WRegionNode::WRNAtomic:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, true);
          Changed = VPOParoptAtomics::handleAtomic(cast<WRNAtomicNode>(W),
                                                   IdentTy, TidPtrHolder);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNWksLoop:
      case WRegionNode::WRNSections:
      case WRegionNode::WRNDistribute:
        debugPrintHeader(W, IsPrepare);
        if (Mode & ParPrepare) {
          regularizeOMPLoop(W);
          genCodemotionFenceforAggrData(W);
          Changed |= propagateCancellationPointsToIR(W);
        }
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          AllocaInst *IsLastVal = nullptr;
          BasicBlock *IfLastIterBB = nullptr;
          Changed = clearCodemotionFenceIntrinsic(W);
          Changed |= clearCancellationPointAllocasFromIR(W);
          Changed |= regularizeOMPLoop(W, false);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
          if (isTargetCSA()) {
            if (W->getIsSections())
              Changed |= genCSASections(W);
            else if (W->getIsOmpLoop())
              Changed |= genCSALoop(W);
            else
              llvm_unreachable("Unexpected work region kind");
            RemoveDirectives = true;
            break;
          }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          Changed |= genLoopSchedulingCode(W, IsLastVal);
          Changed |= genPrivatizationCode(W);
          Changed |= genBarrierForFpLpAndLinears(W);
          Changed |= genLastIterationCheck(W, IsLastVal, IfLastIterBB);
          Changed |= genLinearCode(W, IfLastIterBB);
          Changed |= genLastPrivatizationCode(W, IfLastIterBB);
          Changed |= genFirstPrivatizationCode(W);
          if (!W->getIsDistribute()) {
            Changed |= genReductionCode(W);
            Changed |= genCancellationBranchingCode(W);
          }
          Changed |= genDestructorCode(W);
          if (!W->getIsDistribute() && !W->getNowait())
            Changed |= genBarrier(W, false);
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
            Changed |= genCSASingle(W);
            RemoveDirectives = true;
            break;
          }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          AllocaInst *IsSingleThread = nullptr;
          Changed = genSingleThreadCode(W, IsSingleThread);
          Changed |= genCopyPrivateCode(W, IsSingleThread);
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
            // This is a NOP on CSA.
            RemoveDirectives = true;
            break;
          }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          Changed = genMasterThreadCode(W);
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
            // This is a NOP on CSA.
            RemoveDirectives = true;
            break;
          }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          Changed = genBarrier(W, true);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNCancel:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, true);
          Changed = genCancelCode(dyn_cast<WRNCancelNode>(W));
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
      default:
        break;
      } // switch
    }

    // Remove calls to directive intrinsics since the LLVM back end does not
    // know how to translate them.
    if (RemoveDirectives) {
      bool DirRemoved = VPOUtils::stripDirectives(W);
      assert(DirRemoved && "Directive intrinsics not removed for WRN.\n");
      (void) DirRemoved;
    } else if (RemovePrivateClauses) {
      VPOUtils::stripPrivateClauses(W);
    }

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
//  br label %DIR.QUAL.LIST.END.2.exitStub
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
//    br label %DIR.QUAL.LIST.END.2.exitStub
//
Value* VPOParoptTransform::genReductionFiniForBoolOps(ReductionItem *RedI,
                                          Value *Rhs1, Value *Rhs2,
                                          Type *ScalarTy,
                                          IRBuilder<> &Builder,
                                          bool IsAnd) {
  LLVMContext &C = F->getContext();
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
Value *VPOParoptTransform::genReductionScalarFini(ReductionItem *RedI,
                                                  Value *Rhs1, Value *Rhs2,
                                                  Value *Lhs, Type *ScalarTy,
                                                  IRBuilder<> &Builder) {
  Value *Res = nullptr;

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
  StoreInst *Tmp0 = Builder.CreateStore(Res, Lhs);
  return Tmp0;
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
//   br label %DIR.QUAL.LIST.END.2.exitStub
//
void VPOParoptTransform::genReductionFini(ReductionItem *RedI, Value *OldV,
                                          Instruction *InsertPt,
                                          DominatorTree *DT) {
  assert(isa<AllocaInst>(RedI->getNew()) &&
         "genReductionFini: Expect non-empty alloca instruction.");
  AllocaInst *NewAI = cast<AllocaInst>(RedI->getNew());

  IRBuilder<> Builder(InsertPt);
  if (RedI->getIsArraySection() ||
      NewAI->getAllocatedType()->isArrayTy())
    genRedAggregateInitOrFini(RedI, NewAI, OldV, InsertPt, false, DT);
  else {
    assert(VPOUtils::canBeRegisterized(NewAI->getAllocatedType(),
                                       NewAI->getModule()->getDataLayout()) &&
           "genReductionFini: Expect incoming scalar type.");
    LoadInst *OldLoad = Builder.CreateLoad(OldV);
    LoadInst *NewLoad = Builder.CreateLoad(NewAI);
    Type *ScalarTy = NewAI->getAllocatedType()->getScalarType();

    genReductionScalarFini(RedI, OldLoad, NewLoad, OldV, ScalarTy, Builder);
  }
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
//   br label %DIR.QUAL.LIST.END.1
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
//   br label %DIR.QUAL.LIST.END.2.exitStub
//
void VPOParoptTransform::genRedAggregateInitOrFini(ReductionItem *RedI,
                                                   AllocaInst *AI, Value *OldV,
                                                   Instruction *InsertPt,
                                                   bool IsInit,
                                                   DominatorTree *DT) {

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
    LoadInst *OldLoad = Builder.CreateLoad(DestElementPHI);
    LoadInst *NewLoad = Builder.CreateLoad(SrcElementPHI);
    genReductionScalarFini(RedI, OldLoad, NewLoad, DestElementPHI,
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
  assert(isa<AllocaInst>(FprivI->getNew()) &&
         "genFprivInit: Expect non-empty alloca instruction");
  AllocaInst *AI = cast<AllocaInst>(FprivI->getNew());
  const DataLayout &DL = InsertPt->getModule()->getDataLayout();
  IRBuilder<> Builder(InsertPt);

  if (Function *Cctor = FprivI->getCopyConstructor())
    VPOParoptUtils::genCopyConstructorCall(Cctor, FprivI->getNew(),
                                           FprivI->getOrig(), InsertPt);
  else if (!VPOUtils::canBeRegisterized(AI->getAllocatedType(), DL))
    VPOUtils::genMemcpy(AI, FprivI->getOrig(), DL, AI->getAlignment(),
                        InsertPt);
  else {
    LoadInst *Load = Builder.CreateLoad(FprivI->getOrig());
    Builder.CreateStore(Load, AI);
  }
}

// Generate the lastprivate update code. The same mechanism is also applied
// for copyprivate.
// Here is one example for the lastprivate update for the array.
// num_type    a[100];
// #pragma omp parallel for schedule( static, 1 ) lastprivate( a )
// The output of the array update is as follows.
//
//    %a = alloca [100 x float]
//    br label %DIR.QUAL.LIST.END.1
//
//  for.end:                                          ; preds = %dispatch.latch,
//  %DIR.QUAL.LIST.END.1
//    %1 = bitcast [100 x float]* %a to i8*
//    call void @llvm.memcpy.p0i8.p0i8.i64(i8* bitcast ([100 x float]* @a to
//    i8*), i8* %1, i64 400, i32 0, i1 false)
//    br label %for.end.split
//
void VPOParoptTransform::genLprivFini(Value *NewV, Value *OldV,
                                      Instruction *InsertPt) {
  assert(isa<AllocaInst>(NewV) &&
         "genLprivFini: Expect non-empty alloca instruction.");
  AllocaInst *AI = cast<AllocaInst>(NewV);
  const DataLayout &DL = InsertPt->getModule()->getDataLayout();

  IRBuilder<> Builder(InsertPt);
  if (!VPOUtils::canBeRegisterized(AI->getAllocatedType(), DL))
    VPOUtils::genMemcpy(OldV, AI, DL, AI->getAlignment(),
                        InsertPt->getParent());
  else {
    LoadInst *Load = Builder.CreateLoad(AI);
    Builder.CreateStore(Load, OldV);
  }
}

// genLprivFini interface to support nonPOD with call to CopyAssign
void VPOParoptTransform::genLprivFini(LastprivateItem *LprivI,
                                      Instruction *InsertPt) {
  Value *NewV = LprivI->getNew();
  Value *OldV = LprivI->getOrig();
  if (Function *CpAssn = LprivI->getCopyAssign())
    VPOParoptUtils::genCopyAssignCall(CpAssn, OldV, NewV, InsertPt);
  else
    genLprivFini(NewV, OldV, InsertPt);
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
//    br label %DIR.QUAL.LIST.END.1
//
void VPOParoptTransform::genReductionInit(ReductionItem *RedI,
                                          Instruction *InsertPt,
                                          DominatorTree *DT) {
  assert(isa<AllocaInst>(RedI->getNew()) &&
         "genReductionInit: Expect non-empty alloca instruction");
  AllocaInst *AI = cast<AllocaInst>(RedI->getNew());
  Type *AllocaTy = AI->getAllocatedType();
  Type *ScalarTy = AllocaTy->getScalarType();

  IRBuilder<> Builder(InsertPt);
  if (RedI->getIsArraySection() ||
      AI->getAllocatedType()->isArrayTy())
    genRedAggregateInitOrFini(RedI, AI, nullptr, InsertPt, true, DT);
  else {
    assert(VPOUtils::canBeRegisterized(AI->getAllocatedType(),
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
                                               BasicBlock *&PrivEntryBB) {
  BasicBlock *ExitBlock = W->getExitBBlock();
  BasicBlock *PrivExitBB;
  if (W->getIsOmpLoop()) {
    // If the loop has ztt block, the compiler has to generate the lastprivate
    // update code at the exit block of the loop.
    BasicBlock *ZttBlock = W->getWRNLoopInfo().getZTTBB();

    if (ZttBlock) {
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

    BasicBlock *RedInitEntryBB = nullptr;
    BasicBlock *RedUpdateEntryBB = nullptr;
    createEmptyPrivFiniBB(W, RedUpdateEntryBB);

    for (ReductionItem *RedI : RedClause.items()) {
      Value *NewRedInst;
      Value *Orig = RedI->getOrig();

/*
      assert((isa<GlobalVariable>(Orig) || isa<AllocaInst>(Orig)) &&
             "genReductionCode: Unexpected reduction variable");
*/

      Instruction *InsertPt = &EntryBB->front();

      computeArraySecReductionTypeOffsetSize(*RedI, InsertPt);

      const ArraySectionInfo &ArrSecInfo = RedI->getArraySectionInfo();
      NewRedInst = genPrivatizationAlloca(W, Orig, InsertPt, ".red",
                                          ArrSecInfo.getElementType(),
                                          ArrSecInfo.getSize());
      RedI->setNew(NewRedInst);

      Value *ReplacementVal = getReductionItemReplacementValue(*RedI, InsertPt);
      genPrivatizationReplacement(W, Orig, ReplacementVal, RedI);

      createEmptyPrvInitBB(W, RedInitEntryBB);
      genReductionInit(RedI, RedInitEntryBB->getTerminator(), DT);

      BasicBlock *BeginBB;
      createEmptyPrivFiniBB(W, BeginBB);
      genReductionFini(RedI, RedI->getOrig(), BeginBB->getTerminator(), DT);

      LLVM_DEBUG(dbgs() << "genReductionCode: reduced " << *Orig << "\n");
    }

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
    createEmptyPrivFiniBB(W, EndBB);
    VPOParoptUtils::genKmpcCriticalSection(
        W, IdentTy, TidPtrHolder,
        dyn_cast<Instruction>(RedUpdateEntryBB->begin()),
        EndBB->getTerminator(), "");
    W->resetBBSet(); // Invalidate BBSet after transformations
    Changed = true;
  }
  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genReductionCode\n");
  return Changed;
}

// For array [section] reduction init loop, compute the base address of the
// destination array, number of elements, and destination element type.
void VPOParoptTransform::genAggrReductionInitDstInfo(
    const ReductionItem &RedI, AllocaInst *AI, Instruction *InsertPt,
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

  DestArrayBegin = Builder.CreateBitCast(DestArrayBegin,
                                         PointerType::getUnqual(DestElementTy));
}

// For array [section] reduction finalization loop, compute the base address
// of the source and destination arrays, number of elements, and the type of
// destination array elements.
void VPOParoptTransform::genAggrReductionFiniSrcDstInfo(
    const ReductionItem &RedI, AllocaInst *AI, Value *OldV,
    Instruction *InsertPt, IRBuilder<> &Builder, Value *&NumElements,
    Value *&SrcArrayBegin, Value *&DestArrayBegin, Type *&DestElementTy) {

  bool IsArraySection = RedI.getIsArraySection();
  Type *SrcElementTy = nullptr;

  if (!IsArraySection) {
    NumElements = VPOParoptUtils::genArrayLength(AI, OldV, InsertPt, Builder,
                                                 DestElementTy, DestArrayBegin);
    DestArrayBegin = Builder.CreateBitCast(
        DestArrayBegin, PointerType::getUnqual(DestElementTy));

    VPOParoptUtils::genArrayLength(AI, AI, InsertPt, Builder, SrcElementTy,
                                   SrcArrayBegin);
    SrcArrayBegin = Builder.CreateBitCast(SrcArrayBegin,
                                          PointerType::getUnqual(SrcElementTy));
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
  SrcArrayBegin = Builder.CreateBitCast(SrcArrayBegin,
                                        PointerType::getUnqual(SrcElementTy));

  // Generated IR for destination starting address for the above example:
  //
  //   %_yarrptr.load = load [3 x [4 x [5 x i32]]]*, @_yarrptr          ; (1)
  //   %_yarrptr.load.cast = bitcast %_yarrptr.load to i32*             ; (2)
  //   %_yarrptr.load.cast.plus.offset = gep %_yarrptr.load.cast, 211   ; (3)
  //
  //   %_yarrptr.load.cast.plus.offset is the final DestArrayBegin.

  DestArrayBegin = RedI.getOrig();
  bool ArraySectionBaseIsPtr = ArrSecInfo.getBaseIsPointer();
  if (ArraySectionBaseIsPtr)
    DestArrayBegin = Builder.CreateLoad(
        DestArrayBegin, DestArrayBegin->getName() + ".load"); //          (1)

  assert(DestArrayBegin && isa<PointerType>(DestArrayBegin->getType()) &&
         "Illegal Destination Array for reduction fini.");

  DestArrayBegin = Builder.CreateBitCast(
      DestArrayBegin, PointerType::getUnqual(DestElementTy),
      DestArrayBegin->getName() + ".cast"); //                            (2)
  DestArrayBegin =
      Builder.CreateGEP(DestArrayBegin, ArrSecInfo.getOffset(),
                        DestArrayBegin->getName() + ".plus.offset"); //   (3)
}

void VPOParoptTransform::computeArraySecReductionTypeOffsetSize(
    ReductionItem &RI, Instruction *InsertPt) {

  if (!RI.getIsArraySection())
    return;

  IRBuilder<> Builder(InsertPt);

  Value *Orig = RI.getOrig();
  Type *RITy = Orig->getType();
  Type *ElemTy = cast<PointerType>(RITy)->getElementType();

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
  ArraySectionInfo &ArrSecInfo = RI.getArraySectionInfo();
  const auto &ArraySectionDims = ArrSecInfo.getArraySectionDims();
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
                // onece, so we skip it in the last iteration.

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
VPOParoptTransform::getReductionItemReplacementValue(ReductionItem const &RedI,
                                                     Instruction *InsertPt) {

  Value *NewRedInst = RedI.getNew();
  if (!RedI.getIsArraySection())
    return NewRedInst;

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
  const ArraySectionInfo &ArrSecInfo = RedI.getArraySectionInfo();
  Value *Offset = ArrSecInfo.getOffset();
  Value *NegOffset = Builder.CreateNeg(Offset, "neg.offset");
  Value *NewMinusOffset = Builder.CreateGEP(
      NewRedInst, NegOffset, NewRedInst->getName() + ".minus.offset"); // (1)

  if (!ArrSecInfo.getBaseIsPointer())
    return NewMinusOffset;

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

// A utility to privatize the variables within the region.
AllocaInst *
VPOParoptTransform::genPrivatizationAlloca(WRegionNode *W, Value *PrivValue,
                                           Instruction *InsertPt,
                                           const StringRef VarNameSuff) {
  // LLVM_DEBUG(dbgs() << "Private Instruction Defs: " << *PrivInst << "\n");
  // Generate a new Alloca instruction as privatization action
  AllocaInst *NewPrivInst = nullptr;

  assert(!(W->isBBSetEmpty()) &&
         "genPrivatizationAlloca: WRN has empty BBSet");

  if (auto PrivInst = dyn_cast<AllocaInst>(PrivValue)) {
    NewPrivInst = (AllocaInst *)PrivInst->clone();

    // Add 'priv' suffix for the new alloca instruction
    if (PrivInst->hasName())
      NewPrivInst->setName(PrivInst->getName() + VarNameSuff);

    NewPrivInst->insertBefore(InsertPt);
  } else if (GlobalVariable *GV = dyn_cast<GlobalVariable>(PrivValue)){
    Type *ElemTy = GV->getValueType();
    const DataLayout &DL = F->getParent()->getDataLayout();
    NewPrivInst = new AllocaInst(ElemTy, DL.getAllocaAddrSpace(), nullptr,
                                 GV->getName());
    NewPrivInst->insertBefore(InsertPt);
  } else {
    assert((isa<Argument>(PrivValue) || isa<GetElementPtrInst>(PrivValue)) &&
           "genPrivatizationAlloca: unsupported private item");
    Type *ElemTy = cast<PointerType>(PrivValue->getType())->getElementType();
    const DataLayout &DL = F->getParent()->getDataLayout();
    NewPrivInst = new AllocaInst(ElemTy, DL.getAllocaAddrSpace(), nullptr,
                                 PrivValue->getName());
    NewPrivInst->insertBefore(InsertPt);
  }

  return NewPrivInst;
}

// A utility to privatize the variables within the region.
AllocaInst *VPOParoptTransform::genPrivatizationAlloca(
    WRegionNode *W, Value *PrivValue, Instruction *InsertPt,
    const StringRef VarNameSuff, Type *ArrSecElementType, Value *ArrSecSize) {

  if (!ArrSecSize)
    return genPrivatizationAlloca(W, PrivValue, InsertPt, VarNameSuff);

  Function *F = InsertPt->getParent()->getParent();
  const DataLayout &DL = F->getParent()->getDataLayout();
  IRBuilder<> Builder(InsertPt);

  assert(ArrSecElementType && "Null array section element type.");
  auto *NewPrivInst =
      Builder.CreateAlloca(ArrSecElementType, DL.getAllocaAddrSpace(),
                           ArrSecSize, PrivValue->getName() + VarNameSuff);
  return NewPrivInst;
}

// Replace the variable with the privatized variable
void VPOParoptTransform::genPrivatizationReplacement(WRegionNode *W,
                                                     Value *PrivValue,
                                                     Value *NewPrivValue,
                                                     Item *IT) {

  // Find instructions in W that use V
  SmallVector<Instruction *, 8> PrivUses;
  if (!WRegionUtils::findUsersInRegion(W, PrivValue, &PrivUses, false))
    return; // Found no applicable uses of PrivValue in W's body

  // Replace all USEs of each PrivValue with its NewPrivValue in the
  // W-Region (parallel loop/region/section ... etc.)
  while (!PrivUses.empty()) {
    Instruction *UI = PrivUses.pop_back_val();
    UI->replaceUsesOfWith(PrivValue, NewPrivValue);

    if (isa<GlobalVariable>(PrivValue)) {
      // If PrivValue is a global, its uses could be in ConstantExprs
      SmallVector<Instruction *, 2> NewInstArr;
      IntelGeneralUtils::breakExpressions(UI, &NewInstArr);
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
//           __kmpc_barrier(...)    ; inserted by genBarrierForFpLpAndLinears()
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
bool VPOParoptTransform::genLinearCode(WRegionNode *W,
                                       BasicBlock *LinearFiniBB) {

  assert(W->isBBSetEmpty() && "genLinearCode: BBSET should start empty");

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
  IRBuilder<> InitBuilder(LoopBodyBB->getFirstNonPHI());
  Value *Index = WRegionUtils::getOmpCanonicalInductionVariable(L);
  assert(Index && "genLinearCode: Null Loop index.");

  for (LinearItem *LinearI : LrClause.items()) {
    Value *Orig = LinearI->getOrig();

    // (A) Create private copy of the linear var to be used instead of the
    // original var inside the WRegion. (2)
    NewLinearVar = genPrivatizationAlloca(W, Orig, EntryBB->getFirstNonPHI(),
                                          ".linear");                   // (2)

    // Create a copy of the linear variable to capture its starting value (1)
    LinearStartVar =
        genPrivatizationAlloca(W, Orig, EntryBB->getFirstNonPHI(), ""); // (1)
    LinearStartVar->setName("linear.start");

    // Replace original var with the new var inside the region.
    genPrivatizationReplacement(W, Orig, NewLinearVar, LinearI);
    LinearI->setNew(NewLinearVar);

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
  W->resetBBSet(); // Invalidate BBSet

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genLinearCode\n");
  return true;
}

bool VPOParoptTransform::genFirstPrivatizationCode(WRegionNode *W) {

  bool Changed = false;

  LLVM_DEBUG(
      dbgs() << "\nEnter VPOParoptTransform::genFirstPrivatizationCode\n");

  assert(W->isBBSetEmpty() &&
         "genFirstPrivatizationCode: BBSET should start empty");

  assert(W->canHaveFirstprivate() &&
         "genFirstPrivatizationCode: WRN doesn't take a firstprivate var");

  FirstprivateClause &FprivClause = W->getFpriv();
  if (!FprivClause.empty()) {
    W->populateBBSet();
    BasicBlock *EntryBB = W->getEntryBBlock();
    BasicBlock *ExitBB = W->getExitBBlock();
    BasicBlock *PrivInitEntryBB = nullptr;
    Value *NewPrivInst = nullptr;
    bool ForTask = W->getIsTask();

    for (FirstprivateItem *FprivI : FprivClause.items()) {
      Value *Orig = FprivI->getOrig();
      //
      // assert((isa<GlobalVariable>(Orig) || isa<AllocaInst>(Orig)) &&
      //      "genFirstPrivatizationCode: Unexpected firstprivate variable");
      //
      LastprivateItem *LprivI = FprivI->getInLastprivate();

      if (!LprivI) {
        NewPrivInst = genPrivatizationAlloca(W, Orig, EntryBB->getFirstNonPHI(),
                                             ".fpriv");

        // By this it can uniformly handle the global/local firstprivate.
        // For the case of local firstprivate, the New is the same as the Orig.
        Value *ValueToReplace = W->getIsTarget() ? FprivI->getNew() : Orig;
        genPrivatizationReplacement(W, ValueToReplace, NewPrivInst, FprivI);

        // For a given firstprivate variable, if it also occurs in a map
        // clause with "from" attribute, the compiler needs to generate
        // the code to copy the value back to the target memory.
        if (ForTask || (W->getIsTarget() && FprivI->getInMap() &&
                        FprivI->getInMap()->getIsMapFrom())) {
          IRBuilder<> Builder(EntryBB->getTerminator());
          Builder.CreateStore(Builder.CreateLoad(FprivI->getNew()),
                              NewPrivInst);
          Builder.SetInsertPoint(ExitBB->getTerminator());
          Builder.CreateStore(Builder.CreateLoad(NewPrivInst),
                              FprivI->getNew());
        }

        FprivI->setNew(NewPrivInst);
      } else {
        FprivI->setNew(LprivI->getNew());
        LLVM_DEBUG(dbgs() << "\n  genFirstPrivatizationCode: (" << *Orig
                          << ") is also lastprivate\n");
      }

      if (!ForTask) {
        createEmptyPrvInitBB(W, PrivInitEntryBB);
        genFprivInit(FprivI, PrivInitEntryBB->getTerminator());
      }
      LLVM_DEBUG(dbgs() << "genFirstPrivatizationCode: firstprivatized "
                        << *Orig << "\n");
    }
    Changed = true;
    W->resetBBSet(); // Invalidate BBSet
  }

  LLVM_DEBUG(
      dbgs() << "\nExit VPOParoptTransform::genFirstPrivatizationCode\n");
  return Changed;
}

bool VPOParoptTransform::genLastPrivatizationCode(WRegionNode *W,
                                                  BasicBlock *IfLastIterBB) {
  bool Changed = false;

  LLVM_DEBUG(
      dbgs() << "\nEnter VPOParoptTransform::genLastPrivatizationCode\n");

  assert(W->isBBSetEmpty() &&
         "genLastPrivatizationCode: BBSET should start empty");

  if (!W->canHaveLastprivate())
    return Changed;

  LastprivateClause &LprivClause = W->getLpriv();
  if (!LprivClause.empty()) {

    W->populateBBSet();

    assert(IfLastIterBB && "genLastPrivatizationCode: Null IfLastIterBB.");

    bool ForTask = W->getWRegionKindID() == WRegionNode::WRNTaskloop ||
                   W->getWRegionKindID() == WRegionNode::WRNTask;
    BasicBlock *EntryBB = W->getEntryBBlock();

    for (LastprivateItem *LprivI : LprivClause.items()) {
      Value *Orig = LprivI->getOrig();
      /*
            assert((isa<GlobalVariable>(Orig) || isa<AllocaInst>(Orig)) &&
                   "genLastPrivatizationCode: Unexpected lastprivate variable");
      */
      Value *NewPrivInst;
      if (!ForTask)
        NewPrivInst =
            genPrivatizationAlloca(W, Orig, &EntryBB->front(), ".lpriv");
      else
        NewPrivInst = LprivI->getNew();
      genPrivatizationReplacement(W, Orig, NewPrivInst, LprivI);
      if (!ForTask) {
        LprivI->setNew(NewPrivInst);
        // Emit constructor call for lastprivate var if it is not also a
        // firstprivate (in which case the firsprivate init emits a cctor).
        if (LprivI->getInFirstprivate() == nullptr)
          VPOParoptUtils::genConstructorCall(LprivI->getConstructor(),
                                             NewPrivInst, NewPrivInst);
        genLprivFini(LprivI, IfLastIterBB->getTerminator());
      } else
        genLprivFiniForTaskLoop(LprivI->getParm(), LprivI->getNew(),
                                IfLastIterBB->getTerminator());
    }
    Changed = true;
    W->resetBBSet(); // Invalidate BBSet
  }

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genLastPrivatizationCode\n");
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
  return true;
}

//  Clean up the intrinsic @llvm.invariant.group.barrier and replace the use
//  of the intrinsic with the its operand.
//
//  After the compiler generates the function call
//  @llvm.invariant.group.barrier in the VPO Paropt Prepare pass, the Early
//  CSE pass moves the bitcast instruction across the OMP region. Before
//  the VPO Paropt pass, the compiler removes the intrinsic
//  @llvm.invariant.group.barrier and propagates the result of the intrinsic
//  to the user instructions. The compiler has to handle the bitcast
//  instruction outside the OMP region by cloning that bitcast instruction
//  and place it at the beginning of region entry.
//
//  *** IR Dump After VPO Paropt Prepare Pass ***
//    %2 = call token @llvm.directive.region.entry()
//    [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"([10 x i32]* %pvtPtr) ]
//    %3 = bitcast [10 x i32]* %pvtPtr to i8*
//    %4 = call i8* @llvm.invariant.group.barrier(i8* %3)
//
//  *** IR Dump After Early CSE ***
//    %0 = bitcast [10 x i32]* %pvtPtr to i8*
//    ...
//  DIR.OMP.PARALLEL.1:
//    %1 = call token @llvm.directive.region.entry()
//    [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"([10 x i32]* %pvtPtr) ]
//    %2 = call i8* @llvm.invariant.group.barrier(i8* %0)
//
//  *** IR Dump Before VPO Paropt Prepare Pass ***
//    %0 = bitcast [10 x i32]* %pvtPtr to i8*
//    ...
//  DIR.OMP.PARALLEL.1:
//    %1 = call token @llvm.directive.region.entry()
//    [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"([10 x i32]* %pvtPtr) ]
//    %2 = bitcast [10 x i32]* %pvtPtr to i8*
//    ....
//
bool VPOParoptTransform::clearCodemotionFenceIntrinsic(WRegionNode *W) {
  bool Changed = false;
  W->populateBBSet();
  SmallVector<Instruction*, 8> DelIns;

  for (auto IB = W->bbset_begin(); IB != W->bbset_end(); IB++)
    for (auto &I : **IB)
      if (CallInst *CI = isFenceCall(&I)) {
        Value *V = CI->getOperand(0);
        if (auto *BI = dyn_cast<BitCastInst>(V)) {
          if (!W->contains(BI->getParent()) &&
              WRegionUtils::usedInRegionEntryDirective(W, BI->getOperand(0))) {
            Instruction *Ext = BI->clone();
            Ext->insertBefore(CI);
            CI->setOperand(0, Ext);
            V = Ext;
          }
        } else if (auto *GEPI = dyn_cast<GetElementPtrInst>(V)) {
          if (!W->contains(GEPI->getParent()) &&
              WRegionUtils::usedInRegionEntryDirective(W,
                                                       GEPI->getOperand(0))) {
            Instruction *Ext = GEPI->clone();
            Ext->insertBefore(CI);
            CI->setOperand(0, Ext);
            V = Ext;
          }
        }
        I.replaceAllUsesWith(V);
        DelIns.push_back(&I);
        Changed = true;
      }
  while (!DelIns.empty()) {
    Instruction *I = DelIns.pop_back_val();
    I->eraseFromParent();
  }
  W->resetBBSet();

  return Changed;
}

// Replace the occurrences of V within the region with the return value of the
// intrinsic @llvm.invariant.group.barrier.
void VPOParoptTransform::replaceValueWithinRegion(WRegionNode *W, Value *V) {
  // LLVM_DEBUG(dbgs() << "replaceValueWithinRegion: " << *V << "\n");

  // Find instructions in W that use V
  SmallVector<Instruction *, 8> Users;
  if (!WRegionUtils::findUsersInRegion(W, V, &Users))
    return; // Found no applicable uses of V in W's body

  // Create a new @llvm.invariant.group.barrier for V
  BasicBlock *EntryBB = W->getEntryBBlock();
  IRBuilder<> Builder(EntryBB->getTerminator());
  Value *NewI = Builder.CreateLaunderInvariantGroup(V);

  // Replace uses of V with NewI
  for (Instruction * User : Users) {
    if (isFenceCall(User) != nullptr) {
      // Skip fence intrinsics. Consider this case of nested constructs
      // privatizing "u":
      //           TYPE u;  // some struct type
      //           #pragma omp parallel private (u)  // outer construct
      //           {
      //              u.a=1;
      //                #pragma omp for private (u)  // inner construct
      //                for(...) { ...; u.a=2; }
      //              print(u.a); // print 1, not 2
      //           }
      // First we create %1=fence(bitcast...@u...) for the inner construct,
      // and replace all uses of @u with %1 in the inner region. Then we create
      // %2=fence(bitcast...@u...) for the outer construct, and replace uses of
      // @u with %2. However, we must not replace %1=fence(bitcast...@u...)
      // into %1=fence(bitcast...%2...). Otherwise, the privatizaion in the
      // inner construct is lost.
      //
      // Note: this guard works for global u, but not local u. For a global u,
      // the bitcast is represented as a ConstantExpr which is an operand of
      // the fence call. However, if u is local, a separate bitcast instruction
      // is done outside of the fence:
      //
      //    %3 = bitcast...%u...
      //    %4 = fence(%3)
      //
      // so checking for the fence itself is not effective in this case.
      // To solve that, look at the next section of code dealing with BitCast.
      //
      // LLVM_DEBUG(dbgs() << "Skipping Fence: " << *User << "\n");
      continue;
    }

    // see comment above
    if (BitCastInst *BCI = dyn_cast<BitCastInst>(User)) {
      bool Skip = false;
      for (llvm::User* U : BCI->users())
        if (Instruction *I = dyn_cast<Instruction>(U))
          if (isFenceCall(I)) {
            Skip=true;
            break;
          }
      if (Skip) {
        // LLVM_DEBUG(dbgs() << "Skipping BitCast: " << *BCI << "\n");
        continue;
      }
    }

    // LLVM_DEBUG(dbgs() << "Before Replacement: " << *User << "\n");
    User->replaceUsesOfWith(V, NewI);
    // LLVM_DEBUG(dbgs() << "After Replacement: " << *User << "\n");

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
    IntelGeneralUtils::breakExpressions(User, &NewInstArr);
    for (Instruction *NewInstr : NewInstArr) {
      // LLVM_DEBUG(dbgs() << "Before Replacement: " << *NewInstr << "\n");
      NewInstr->replaceUsesOfWith(V, NewI);
      // LLVM_DEBUG(dbgs() << "After Replacement: " << *NewInstr << "\n");
    }
  }
}

// Generate the intrinsic @llvm.invariant.group.barrier for local/global
// variable I.
void VPOParoptTransform::genFenceIntrinsic(WRegionNode *W, Value *I) {

  if (AllocaInst *AI = dyn_cast<AllocaInst>(I)) {
    Type *AllocaTy = AI->getAllocatedType();

    if (!AllocaTy->isSingleValueType())
      replaceValueWithinRegion(W, I);

  } else if (GlobalVariable *GV = dyn_cast<GlobalVariable>(I)) {
    Type *Ty = GV->getValueType();
    if (!Ty->isSingleValueType())
      replaceValueWithinRegion(W, I);
  }
  else {
    Type *Ty = I->getType();
    PointerType *PtrTy = dyn_cast<PointerType>(Ty);
    if (!PtrTy)
      return;
    Ty = PtrTy->getElementType();
    if (!Ty->isSingleValueType())
      replaceValueWithinRegion(W, I);
  }
}

// Return true if the instuction is a call to
// @llvm.invariant.group.barrier
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
  TerminatorInst *TermInst = Backedge->getTerminator();
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
void VPOParoptTransform::fixOMPDoWhileLoop(WRegionNode *W) {

  Loop *L = W->getWRNLoopInfo().getLoop();

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
        TerminatorInst *TermInst = Inc->getParent()->getTerminator();
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
          } else
            llvm_unreachable("cannot fix omp do-while loop");
        } else
          llvm_unreachable("cannot fix omp do-while loop");
      }
    }
  }
  llvm_unreachable("cannot fix omp do-while loop");
}

// The OMP loop is converted into bottom test loop to facilitate the
// code generation of VPOParopt transform and vectorization. This
// regularization is required for the program which is compiled at -O0
// and above.
bool VPOParoptTransform::regularizeOMPLoop(WRegionNode *W, bool First) {
  if (!W->getWRNLoopInfo().getLoop())
    return false;

  W->populateBBSet();
  if (!First) {
    // For the case of #pragma omp parallel for simd, the clang only
    // needs to generate the bundle omp.iv for the parallel region.
    if (W->getWRNLoopInfo().getNormIVSize()==0) {
      W->resetBBSet();
      return false;
    }
    Loop *L = W->getWRNLoopInfo().getLoop();
    const DataLayout &DL = L->getHeader()->getModule()->getDataLayout();
    const SimplifyQuery SQ = {DL, TLI, DT, AC};
    LoopRotation(L, LI, TTI, AC, DT, SE, SQ, true, unsigned(-1), true);
    std::vector<AllocaInst *> Allocas;
    SmallVector<Value *, 2> LoopEssentialValues;
    if (W->getWRNLoopInfo().getNormIV())
      LoopEssentialValues.push_back(W->getWRNLoopInfo().getNormIV());

    if (W->getWRNLoopInfo().getNormUB())
      LoopEssentialValues.push_back(W->getWRNLoopInfo().getNormUB());

    for (auto V : LoopEssentialValues) {
      AllocaInst *AI = dyn_cast<AllocaInst>(V);
      assert(AI && "Expect alloca instruction for omp_iv or omp_ub");
      for (auto IB = V->user_begin(), IE = V->user_end(); IB != IE; ++IB) {
        if (LoadInst *LdInst = dyn_cast<LoadInst>(*IB))
          LdInst->setVolatile(false);
        if (StoreInst *StInst = dyn_cast<StoreInst>(*IB))
          StInst->setVolatile(false);
      }
      resetValueInIntelClauseGeneric(W, V);
      Allocas.push_back(AI);
    }

    PromoteMemToReg(Allocas, *DT, AC);
    fixOMPDoWhileLoop(W);
  } else {
    std::vector<AllocaInst *> Allocas;
    SmallVector<Value *, 2> LoopEssentialValues;
    if (W->getWRNLoopInfo().getNormIV())
      LoopEssentialValues.push_back(W->getWRNLoopInfo().getNormIV());

    if (W->getWRNLoopInfo().getNormUB())
      LoopEssentialValues.push_back(W->getWRNLoopInfo().getNormUB());

    for (auto V : LoopEssentialValues) {
      assert(dyn_cast<AllocaInst>(V) &&
             "Expect alloca instruction for omp_iv or omp_ub");
      for (auto IB = V->user_begin(), IE = V->user_end(); IB != IE; ++IB) {
        if (LoadInst *LdInst = dyn_cast<LoadInst>(*IB))
          LdInst->setVolatile(true);
        if (StoreInst *StInst = dyn_cast<StoreInst>(*IB))
          StInst->setVolatile(true);
      }
    }
  }
  W->resetBBSet();
  return true;

  llvm_unreachable("Expect the omp normalized iv to be a stack variable.");
}

// Generate the intrinsic @llvm.invariant.group.barrier to inhibit the cse
// for the gep instruction related to array/struture which is marked
// as private, firstprivate, lastprivate, reduction or shared.
void VPOParoptTransform::genCodemotionFenceforAggrData(WRegionNode *W) {
  W->populateBBSet();
  if (W->canHavePrivate()) {
    PrivateClause &PrivClause = W->getPriv();
    for (PrivateItem *PrivI : PrivClause.items())
      genFenceIntrinsic(W, PrivI->getOrig());
  }

  if (W->canHaveFirstprivate()) {
    FirstprivateClause &FprivClause = W->getFpriv();
    for (FirstprivateItem *FprivI : FprivClause.items())
      genFenceIntrinsic(W, FprivI->getOrig());
  }

  if (W->canHaveShared()) {
    SharedClause &ShaClause = W->getShared();
    for (SharedItem *ShaI : ShaClause.items())
      genFenceIntrinsic(W, ShaI->getOrig());
  }

  if (W->canHaveReduction()) {
    ReductionClause &RedClause = W->getRed();
    for (ReductionItem *RedI : RedClause.items())
      genFenceIntrinsic(W, RedI->getOrig());
  }

  if (W->canHaveLastprivate()) {
    LastprivateClause &LprivClause = W->getLpriv();
    for (LastprivateItem *LprivI : LprivClause.items())
      genFenceIntrinsic(W, LprivI->getOrig());
  }
}

bool VPOParoptTransform::genPrivatizationCode(WRegionNode *W) {

  bool Changed = false;

  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *ExitBB = W->getExitBBlock();

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genPrivatizationCode\n");

  // Process all PrivateItems in the private clause
  PrivateClause &PrivClause = W->getPriv();
  if (!PrivClause.empty()) {

    assert(W->isBBSetEmpty() &&
           "genPrivatizationCode: BBSET should start empty");
    W->populateBBSet();

    bool ForTask = W->getWRegionKindID() == WRegionNode::WRNTaskloop ||
                   W->getWRegionKindID() == WRegionNode::WRNTask;

    // Walk through each PrivateItem list in the private clause to perform
    // privatization for each Value item
    for (PrivateItem *PrivI : PrivClause.items()) {
      Value *Orig = PrivI->getOrig();

      if (isa<GlobalVariable>(Orig) ||
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
          // On CSA we also need to do privatization for function arguments
          // because parallel region are not getting outlined. Thus we have
          // create private instances for function arguments which are
          // annotated as private to avoid modification of the original
          // argument.
          (isa<Argument>(Orig) && isTargetCSA()) ||
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
          isa<AllocaInst>(Orig)) {
        Value *NewPrivInst;

        // Insert alloca for privatization right after the BEGIN directive.
        // Note: do not hoist the following AllocaInsertPt computation out of
        // this for-loop. AllocaInsertPt may be a clause directive that is
        // removed by genPrivatizationReplacement(), so we need to recompute
        // AllocaInsertPt at every iteration of this for-loop.

        // For now, back out this change to AllocaInsertPt until we figure
        // out why it causes an assert in VPOCodeGen::getVectorPrivateBase
        // when running run_gf_channels (gridfusion4.3_tuned_channels).
        //
        //   Instruction *AllocaInsertPt = EntryBB->front().getNextNode();

        Instruction *AllocaInsertPt = EntryBB->getFirstNonPHI();
        NewPrivInst = genPrivatizationAlloca(W, Orig, AllocaInsertPt, ".priv");
        genPrivatizationReplacement(W, Orig, NewPrivInst, PrivI);

        if (!ForTask) {
          PrivI->setNew(NewPrivInst);
          VPOParoptUtils::genConstructorCall(PrivI->getConstructor(),
                                             NewPrivInst, NewPrivInst);
        } else {
          AllocaInst *AI = dyn_cast<AllocaInst>(NewPrivInst);
          const DataLayout &DL = AI->getModule()->getDataLayout();
          // The compiler creates the stack space for the local vars. Thus
          // the data needs to be copied from the thunk to the local vars.
          if (!VPOUtils::canBeRegisterized(AI->getAllocatedType(), DL)) {
            VPOUtils::genMemcpy(AI, PrivI->getNew(), DL, AI->getAlignment(),
                                EntryBB);
            VPOUtils::genMemcpy(PrivI->getNew(), AI, DL, AI->getAlignment(),
                                ExitBB);
          } else {
            IRBuilder<> Builder(EntryBB->getTerminator());
            Builder.CreateStore(Builder.CreateLoad(PrivI->getNew()),
                                NewPrivInst);
            Builder.SetInsertPoint(ExitBB->getTerminator());
            Builder.CreateStore(Builder.CreateLoad(NewPrivInst),
                                PrivI->getNew());
          }
        }

        LLVM_DEBUG(dbgs() << "genPrivatizationCode: privatized " << *Orig
                          << "\n");
      } else
        LLVM_DEBUG(dbgs() << "genPrivatizationCode: " << *Orig
                          << " is already private.\n");
    }

    Changed = true;
    W->resetBBSet(); // Invalidate BBSet after transformations

    // After Privatization is done, the SCEV should be re-generated.
    // This should apply to all loop-type constructs; ie, WRNs whose
    // "IsOmpLoop" attribute is true.
    if (SE && W->getIsOmpLoop()) {
        Loop *L = W->getWRNLoopInfo().getLoop();
        SE->forgetLoop(L);
    }
  }
  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genPrivatizationCode\n");
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
    Value *V = nullptr, *OV;
    bool Match;
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
    PHINode *PN = dyn_cast<PHINode>(&I);
    unsigned NumPHIValues = PN->getNumIncomingValues();
    unsigned II;
    BasicBlock *InBB;
    Value *IV;
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
            buildECs(&L, dyn_cast<PHINode>(&I), ECs);
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

bool VPOParoptTransform::genLoopSchedulingCode(WRegionNode *W,
                                               AllocaInst *&IsLastVal) {
  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genLoopSchedulingCode\n");

  assert(W->getIsOmpLoop() && "genLoopSchedulingCode: not a loop-type WRN");

  Loop *L = W->getWRNLoopInfo().getLoop();

  assert(L && "genLoopSchedulingCode: Loop not found");

  LLVM_DEBUG(dbgs() << "--- Parallel For LoopInfo: \n" << *L);
  LLVM_DEBUG(dbgs() << "--- Loop Preheader: " << *(L->getLoopPreheader())
                    << "\n");
  LLVM_DEBUG(dbgs() << "--- Loop Header: " << *(L->getHeader()) << "\n");
  LLVM_DEBUG(dbgs() << "--- Loop Latch: " << *(L->getLoopLatch()) << "\n\n");

  bool IsDoacrossLoop =
       ((isa<WRNParallelLoopNode>(W) || isa<WRNWksLoopNode>(W)) &&
         W->getOrdered() > 0);

  bool IsDistParLoop = isa<WRNDistributeParLoopNode>(W);
  bool IsDistForLoop = isa<WRNDistributeNode>(W);

  bool IsDistChunkedParLoop = false;

  Value *DistChunkVal = NULL;

  WRNScheduleKind DistSchedKind;

  if (IsDistParLoop) {

    // Get dist_schedule kind and chunk information from W-Region node
    // Default: DistributeStaticEven.
    DistSchedKind = VPOParoptUtils::getDistLoopScheduleKind(W);

    if (DistSchedKind == WRNScheduleDistributeStatic) {
      DistChunkVal = W->getDistSchedule().getChunkExpr();
      IsDistChunkedParLoop = true;
    }
  }

#if 0
  LLVM_DEBUG(dbgs() << "---- Loop Induction: "
               << *(L->getCanonicalInductionVariable()) << "\n\n");
  L->dump();
#endif

  assert(L->isLoopSimplifyForm() && "should follow from addRequired<>");

  ICmpInst *CmpI =
    WRegionUtils::getOmpLoopZeroTripTest(L, W->getEntryBBlock());
  if (CmpI)
    W->getWRNLoopInfo().setZTTBB(CmpI->getParent());

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
  LLVMContext &C = F->getContext();
  IntegerType *Int32Ty = Type::getInt32Ty(C);
  const DataLayout &DL = F->getParent()->getDataLayout();

  Type *LoopIndexType =
          WRegionUtils::getOmpCanonicalInductionVariable(L)->
          getIncomingValue(0)->getType();

  IntegerType *IndValTy = cast<IntegerType>(LoopIndexType);
  assert(IndValTy->getIntegerBitWidth() >= 32 &&
         "Omp loop index type width is equal or greater than 32 bit");

  Value *InitVal = WRegionUtils::getOmpLoopLowerBound(L);
  assert(isa<Instruction>(L->getLoopPreheader()->getTerminator()) &&
         "genLoopSchedulingCode: Expect non-empty instruction.");
  Instruction *InsertPt = cast<Instruction>(
    L->getLoopPreheader()->getTerminator());

  LoadInst *LoadTid = new LoadInst(TidPtrHolder, "my.tid", InsertPt);
  LoadTid->setAlignment(4);

  // Inserting the alloca of %is.last at InsertPt (=loop preheader) is wrong,
  // as it may not dominate its use at loop exit, which is reachable from the
  // ZTTBB above the preheader:
  //
  //   DIR.QUAL.LIST.END.2:        ; The ZTT
  //     %5 = load i32, i32* %.omp.lb.fpriv, align 4, !tbaa !5
  //     %6 = load i32, i32* %.omp.ub.fpriv, align 4, !tbaa !5
  //     %cmp6 = icmp ugt i32 %5, %6
  //     br i1 %cmp6, label %omp.loop.exit, label %omp.inner.for.body.lr.ph
  //
  //   omp.inner.for.body.lr.ph:   ; The loop preheader
  //     %my.tid = load i32, i32* %new.tid.addr, align 4
  //     %is.last = alloca i32, align 4 ; **ERROR: Doesn't dominate use!
  //    ...
  //
  //   omp.loop.exit:  ; Reachable from the ZTT BB bypassing the preheader
  //     %11 = load i32, i32* %is.last
  //     %12 = icmp ne i32 %11, 0
  //     br i1 %12, label %lastprivate.then, label %lastprivate.done
  //
  // The right insertion point for the def of %is.last is W's EntryBB.
  IsLastVal = new AllocaInst(Int32Ty, DL.getAllocaAddrSpace(), "is.last",
                             &(W->getEntryBBlock()->front()));
  IsLastVal->setAlignment(4);

  AllocaInst *LowerBnd = new AllocaInst(IndValTy, DL.getAllocaAddrSpace(),
                                        "lower.bnd", InsertPt);
  LowerBnd->setAlignment(4);

  AllocaInst *UpperBnd = new AllocaInst(IndValTy, DL.getAllocaAddrSpace(),
                                        "upper.bnd", InsertPt);
  UpperBnd->setAlignment(4);

  AllocaInst *Stride = new AllocaInst(IndValTy, DL.getAllocaAddrSpace(),
                                      "stride", InsertPt);
  Stride->setAlignment(4);

  // UpperD is for distribtue loop
  AllocaInst *UpperD = new AllocaInst(IndValTy, DL.getAllocaAddrSpace(),
                                      "upperD", InsertPt);
  UpperD->setAlignment(4);

  // Constant Definitions
  ConstantInt *ValueZero = ConstantInt::getSigned(Int32Ty, 0);
  ConstantInt *ValueOne  = ConstantInt::get(IndValTy, 1);

  // Get Schedule kind and chunk information from W-Region node
  // Default: static_even.
  WRNScheduleKind SchedKind = VPOParoptUtils::getLoopScheduleKind(W);

  ConstantInt *SchedType = ConstantInt::getSigned(Int32Ty, SchedKind);

  AllocaInst *TeamIsLast;
  AllocaInst *TeamLowerBnd;
  AllocaInst *TeamUpperBnd;
  AllocaInst *TeamStride;
  AllocaInst *TeamUpperD;

  if (IsDistChunkedParLoop) {
    TeamIsLast = new AllocaInst(Int32Ty, DL.getAllocaAddrSpace(),
                                "team.is.last",
                                &(W->getEntryBBlock()->front()));
    TeamIsLast->setAlignment(4);

    TeamLowerBnd = new AllocaInst(IndValTy, DL.getAllocaAddrSpace(),
                                  "team.lower.bnd", InsertPt);
    TeamLowerBnd->setAlignment(4);

    TeamUpperBnd = new AllocaInst(IndValTy, DL.getAllocaAddrSpace(),
                                  "team.upper.bnd", InsertPt);
    TeamUpperBnd->setAlignment(4);

    TeamStride = new AllocaInst(IndValTy, DL.getAllocaAddrSpace(),
                                "team.stride", InsertPt);
    TeamStride->setAlignment(4);

    TeamUpperD = new AllocaInst(IndValTy, DL.getAllocaAddrSpace(),
                                "team.upperD", InsertPt);
    TeamUpperD->setAlignment(4);

    StoreInst *Tmp = new StoreInst(ValueZero, TeamIsLast);
    Tmp->insertAfter(TeamIsLast);
    Tmp->setAlignment(4);
  }

  IRBuilder<> B(InsertPt);
  if (InitVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    InitVal = B.CreateSExtOrTrunc(InitVal, IndValTy);

  StoreInst *Tmp0 = new StoreInst(InitVal, LowerBnd, false, InsertPt);
  Tmp0->setAlignment(4);

  Value *UpperBndVal = VPOParoptUtils::computeOmpUpperBound(W, InsertPt);
  assert(UpperBndVal &&
         "genLoopSchedulingCode: Expect non-empty loop upper bound");

  if (UpperBndVal->getType()->getIntegerBitWidth() !=
                              IndValTy->getIntegerBitWidth())
    UpperBndVal = B.CreateSExtOrTrunc(UpperBndVal, IndValTy);

  StoreInst *Tmp1 = new StoreInst(UpperBndVal, UpperBnd, false, InsertPt);
  Tmp1->setAlignment(4);

  bool IsNegStride;
  Value *StrideVal = WRegionUtils::getOmpLoopStride(L, IsNegStride);
  StrideVal = VPOParoptUtils::cloneInstructions(StrideVal, InsertPt);

  if (IsNegStride) {
    ConstantInt *Zero = ConstantInt::get(IndValTy, 0);
    StrideVal = B.CreateSub(Zero, StrideVal);
  }

  if (StrideVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    StrideVal = B.CreateSExtOrTrunc(StrideVal, IndValTy);

  StoreInst *Tmp2 = new StoreInst(StrideVal, Stride, false, InsertPt);
  Tmp2->setAlignment(4);

  StoreInst *Tmp3 = new StoreInst(UpperBndVal, UpperD, false, InsertPt);
  Tmp3->setAlignment(4);

  // Insert the initialization of %is.last right after its alloca
  StoreInst *Tmp4 = new StoreInst(ValueZero, IsLastVal);
  Tmp4->insertAfter(IsLastVal);
  Tmp4->setAlignment(4);

  ICmpInst* LoopBottomTest = WRegionUtils::getOmpLoopBottomTest(L);

  bool IsUnsigned = LoopBottomTest->isUnsigned();
  int Size = LowerBnd->getType()
                     ->getPointerElementType()->getIntegerBitWidth();

  CallInst* KmpcInitCI;
  CallInst* KmpcFiniCI;
  CallInst* KmpcNextCI;

  LoadInst *TeamLB;
  LoadInst *TeamUB;
  LoadInst *TeamST;
  LoadInst *TeamUD;

  CallInst* KmpcTeamInitCI;

  Value *ChunkVal = (SchedKind == WRNScheduleStaticEven ||
                     SchedKind == WRNScheduleOrderedStaticEven) ?
                                  ValueOne : W->getSchedule().getChunkExpr();


  LLVM_DEBUG(dbgs() << "--- Schedule Chunk Value: " << *ChunkVal << "\n\n");

  if (IsDistChunkedParLoop) {
    StoreInst *Tmp0 = new StoreInst(InitVal, TeamLowerBnd, false, InsertPt);
    Tmp0->setAlignment(4);

    StoreInst *Tmp1 = new StoreInst(UpperBndVal, TeamUpperBnd, false, InsertPt);
    Tmp1->setAlignment(4);

    StoreInst *Tmp2 = new StoreInst(StrideVal, TeamStride, false, InsertPt);
    Tmp2->setAlignment(4);

    StoreInst *Tmp3 = new StoreInst(UpperBndVal, TeamUpperD, false, InsertPt);
    Tmp3->setAlignment(4);

    // Generate __kmpc_team_static_init_4{u}/8{u} Call Instruction
    KmpcTeamInitCI = VPOParoptUtils::genKmpcTeamStaticInit(W, IdentTy,
                               LoadTid, TeamIsLast, TeamLowerBnd,
                               TeamUpperBnd, TeamStride, StrideVal,
                               DistChunkVal, Size, IsUnsigned, InsertPt);

    TeamLB = new LoadInst(TeamLowerBnd, "team.new.lb", InsertPt);
    TeamLB->setAlignment(4);

    TeamUB = new LoadInst(TeamUpperBnd, "team.new.ub", InsertPt);
    TeamUB->setAlignment(4);

    TeamST = new LoadInst(TeamStride, "team.new.st", InsertPt);
    TeamST->setAlignment(4);

    TeamUD = new LoadInst(TeamUpperBnd, "team.new.ud", InsertPt);
    TeamUD->setAlignment(4);

    Tmp0 = new StoreInst(TeamLB, LowerBnd, false, InsertPt);
    Tmp0->setAlignment(4);

    Tmp1 = new StoreInst(TeamUB, UpperBnd, false, InsertPt);
    Tmp1->setAlignment(4);

    Tmp2 = new StoreInst(TeamST, Stride, false, InsertPt);
    Tmp2->setAlignment(4);

    Tmp3 = new StoreInst(TeamUB, UpperD, false, InsertPt);
    Tmp3->setAlignment(4);

  }

  if (IsDistParLoop && !IsDistChunkedParLoop) {
    ConstantInt *DistSchedType = ConstantInt::getSigned(Int32Ty, DistSchedKind);
    // Generate __kmpc_dist_for_static_init_4{u}/8{u} Call Instruction
    KmpcInitCI = VPOParoptUtils::genKmpcStaticInit(W, IdentTy,
                               LoadTid, DistSchedType, IsLastVal, LowerBnd,
                               UpperBnd, UpperD, Stride, StrideVal,
                               DistChunkVal, Size, IsUnsigned, InsertPt);
  }
  else if (SchedKind == WRNScheduleStatic ||
           SchedKind == WRNScheduleStaticEven) {
    // Generate __kmpc_for_static_init_4{u}/8{u} Call Instruction
    KmpcInitCI = VPOParoptUtils::genKmpcStaticInit(W, IdentTy,
                               LoadTid, SchedType, IsLastVal, LowerBnd,
                               UpperBnd, UpperD, Stride, StrideVal, ChunkVal,
                               Size, IsUnsigned, InsertPt);
  }
  else {
    // Generate __kmpc_dispatch_init_4{u}/8{u} Call Instruction
    if (IsDistForLoop) {
      ConstantInt *DistSchedType =
                     ConstantInt::getSigned(Int32Ty, DistSchedKind);

      KmpcInitCI = VPOParoptUtils::genKmpcDispatchInit(W, IdentTy,
                               LoadTid, DistSchedType, IsLastVal,
                               InitVal, UpperBndVal, StrideVal,
                               ChunkVal, Size, IsUnsigned, InsertPt);
    }
    else
      KmpcInitCI = VPOParoptUtils::genKmpcDispatchInit(W, IdentTy,
                               LoadTid, SchedType, IsLastVal,
                               InitVal, UpperBndVal, StrideVal,
                               ChunkVal, Size, IsUnsigned, InsertPt);

    // Generate __kmpc_dispatch_next_4{u}/8{u} Call Instruction
    KmpcNextCI = VPOParoptUtils::genKmpcDispatchNext(W, IdentTy,
                               LoadTid, IsLastVal, LowerBnd,
                               UpperBnd, Stride, Size, IsUnsigned, InsertPt);
  }

  // Insert doacross_init call for ordered(n)
  if (IsDoacrossLoop)
    VPOParoptUtils::genKmpcDoacrossInit(W, IdentTy, LoadTid, KmpcInitCI,
                                        LowerBnd, UpperBnd, Stride);

  LoadInst *LoadLB = new LoadInst(LowerBnd, "lb.new", InsertPt);
  LoadLB->setAlignment(4);

  LoadInst *LoadUB = new LoadInst(UpperBnd, "ub.new", InsertPt);
  LoadUB->setAlignment(4);

  PHINode *PN = WRegionUtils::getOmpCanonicalInductionVariable(L);
  //  Value *InitBoundV = PN->getIncomingValueForBlock(L->getLoopPreheader());
  PN->removeIncomingValue(L->getLoopPreheader());
  PN->addIncoming(LoadLB, L->getLoopPreheader());

  //  replaceUseWithinRegion(W, InitBoundV, LoadLB);

  BasicBlock *LoopExitBB = WRegionUtils::getOmpExitBlock(L);

  bool IsLeft;
  CmpInst::Predicate PD = VPOParoptUtils::computeOmpPredicate(
                                   WRegionUtils::getOmpPredicate(L, IsLeft));
  ICmpInst* CompInst;
  CompInst = new ICmpInst(InsertPt, PD, LoadLB, LoadUB, "");

  VPOParoptUtils::updateOmpPredicateAndUpperBound(W, LoadUB, InsertPt);

  BranchInst* PreHdrInst = dyn_cast<BranchInst>(InsertPt);
  assert(PreHdrInst->getNumSuccessors() == 1 &&
         "Expect preheader BB has one exit!");

  BasicBlock *LoopRegionExitBB =
      SplitBlock(LoopExitBB, LoopExitBB->getFirstNonPHI(), DT, LI);
  LoopRegionExitBB->setName("loop.region.exit");

  if (LoopExitBB == W->getExitBBlock())
    W->setExitBBlock(LoopRegionExitBB);

  std::swap(LoopExitBB, LoopRegionExitBB);
  TerminatorInst *NewTermInst = BranchInst::Create(PreHdrInst->getSuccessor(0),
                                                   LoopExitBB, CompInst);
  ReplaceInstWithInst(InsertPt, NewTermInst);

  InsertPt = LoopExitBB->getTerminator();

  if (SchedKind == WRNScheduleStaticEven) {

    BasicBlock *StaticInitBB = KmpcInitCI->getParent();

    KmpcFiniCI = VPOParoptUtils::genKmpcStaticFini(W,
                                        IdentTy, LoadTid, InsertPt);
    KmpcFiniCI->setCallingConv(CallingConv::C);

    // Insert doacross_fini call for ordered(n)
    if (IsDoacrossLoop)
      VPOParoptUtils::genKmpcDoacrossFini(W, IdentTy, LoadTid, KmpcFiniCI);

    if (DT)
      DT->changeImmediateDominator(LoopExitBB, StaticInitBB);

    wrnUpdateLiveOutVals(L, LoopRegionExitBB, LiveOutVals, ECs);
    rewriteUsesOfOutInstructions(ValueToLiveinMap, LiveOutVals, ECs);

  }
  else if (SchedKind == WRNScheduleStatic) {

    //// LLVM_DEBUG(dbgs() << "Before Loop Scheduling : "
    ////              << *(LoopExitBB->getParent()) << "\n\n");

    BasicBlock *StaticInitBB = KmpcInitCI->getParent();

    KmpcFiniCI = VPOParoptUtils::genKmpcStaticFini(W,
                                        IdentTy, LoadTid, InsertPt);
    KmpcFiniCI->setCallingConv(CallingConv::C);

    // Insert doacross_fini call for ordered(n)
    if (IsDoacrossLoop)
      VPOParoptUtils::genKmpcDoacrossFini(W, IdentTy, LoadTid, KmpcFiniCI);

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

    // Generate dispatch header BBlock
    BasicBlock *DispatchHeaderBB = SplitBlock(StaticInitBB, LoadLB, DT, LI);
    DispatchHeaderBB->setName("dispatch.header");

    // Generate a upper bound load instruction at top of DispatchHeaderBB
    LoadInst *TmpUB = new LoadInst(UpperBnd, "ub.tmp", LoadLB);

    BasicBlock *DispatchBodyBB = SplitBlock(DispatchHeaderBB, LoadLB, DT, LI);
    DispatchBodyBB->setName("dispatch.body");

    TerminatorInst *TermInst = DispatchHeaderBB->getTerminator();

    ICmpInst* MinUB;

    if (IsLeft)
      MinUB = new ICmpInst(TermInst, PD, TmpUB, UpperBndVal, "ub.min");
    else
      MinUB = new ICmpInst(TermInst, PD, UpperBndVal, TmpUB, "ub.min");

    StoreInst *NewUB = new StoreInst(UpperBndVal, UpperBnd, false, TermInst);

    BasicBlock *DispatchMinUBB = SplitBlock(DispatchHeaderBB, NewUB, DT, LI);
    DispatchMinUBB->setName("dispatch.min.ub");

    TermInst = DispatchHeaderBB->getTerminator();

    // Generate branch for dispatch.cond for get MIN upper bound
    TerminatorInst *NewTermInst = BranchInst::Create(DispatchBodyBB,
                                                     DispatchMinUBB, MinUB);
    ReplaceInstWithInst(TermInst, NewTermInst);

    // Generate dispatch chunk increment BBlock
    BasicBlock *DispatchLatchBB = SplitBlock(LoopExitBB, KmpcFiniCI, DT, LI);

    TermInst = LoopExitBB->getTerminator();
    LoopExitBB->setName("dispatch.inc");

    // Load Stride value to st.new
    LoadInst *StrideVal = new LoadInst(Stride, "st.inc", TermInst);

    // Generate inc.lb.new = lb.new + st.new
    BinaryOperator *IncLB = BinaryOperator::CreateAdd(
                                            LoadLB, StrideVal, "lb.inc");
    IncLB->insertBefore(TermInst);

    // Generate inc.lb.new = lb.new + st.new
    BinaryOperator *IncUB = BinaryOperator::CreateAdd(
                                            LoadUB, StrideVal, "ub.inc");
    IncUB->insertBefore(TermInst);

    StoreInst *NewIncLB = new StoreInst(IncLB, LowerBnd, false, TermInst);
    NewIncLB->setAlignment(4);

    StoreInst *NewIncUB = new StoreInst(IncUB, UpperBnd, false, TermInst);
    NewIncUB->setAlignment(4);

    TermInst->setSuccessor(0, DispatchHeaderBB);

    DispatchLatchBB->setName("dispatch.latch");

    TermInst = DispatchBodyBB->getTerminator();
    TermInst->setSuccessor(1, DispatchLatchBB);

    if (DT) {
      DT->changeImmediateDominator(DispatchHeaderBB, StaticInitBB);

      DT->changeImmediateDominator(DispatchBodyBB, DispatchHeaderBB);
      DT->changeImmediateDominator(DispatchMinUBB, DispatchHeaderBB);

      DT->changeImmediateDominator(DispatchLatchBB, DispatchBodyBB);
    }

    Loop *OuterLoop = WRegionUtils::createLoop(L, L->getParentLoop(), LI);
    WRegionUtils::updateBBForLoop(DispatchHeaderBB, OuterLoop,
                                  L->getParentLoop(), LI);
    WRegionUtils::updateBBForLoop(DispatchMinUBB, OuterLoop, L->getParentLoop(),
                                  LI);
    WRegionUtils::updateBBForLoop(DispatchBodyBB, OuterLoop, L->getParentLoop(),
                                  LI);
    WRegionUtils::updateBBForLoop(LoopExitBB, OuterLoop, L->getParentLoop(),
                                  LI);
    WRegionUtils::updateBBForLoop(LoopRegionExitBB, OuterLoop,
                                  L->getParentLoop(), LI);
    OuterLoop->moveToHeader(DispatchHeaderBB);

    wrnUpdateLiveOutVals(OuterLoop, LoopRegionExitBB, LiveOutVals, ECs);
    wrnUpdateSSAPreprocessForOuterLoop(OuterLoop, ValueToLiveinMap, LiveOutVals,
                                       ECs);
    rewriteUsesOfOutInstructions(ValueToLiveinMap, LiveOutVals, ECs);

    //// LLVM_DEBUG(dbgs() << "After Loop Scheduling : "
    ////              << *(LoopExitBB->getParent()) << "\n\n");
  }
  else {
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
    //        |  |    |             |          |
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
    KmpcFiniCI = VPOParoptUtils::genKmpcDispatchFini(W,
                          IdentTy, LoadTid, Size, IsUnsigned, InsertPt);
    KmpcFiniCI->setCallingConv(CallingConv::C);

    BasicBlock *DispatchInitBB = KmpcNextCI->getParent();

    BasicBlock *DispatchHeaderBB = SplitBlock(DispatchInitBB,
                                              KmpcNextCI, DT, LI);
    DispatchHeaderBB->setName("dispatch.header" + Twine(W->getNumber()));

    BasicBlock *DispatchBodyBB = SplitBlock(DispatchHeaderBB, LoadLB, DT, LI);
    DispatchBodyBB->setName("dispatch.body" + Twine(W->getNumber()));

    TerminatorInst *TermInst = DispatchHeaderBB->getTerminator();

    ICmpInst* CondInst = new ICmpInst(TermInst, ICmpInst::ICMP_NE,
                               KmpcNextCI, ValueZero,
                              "dispatch.cond" + Twine(W->getNumber()));

    TerminatorInst *NewTermInst = BranchInst::Create(DispatchBodyBB,
                                                    LoopExitBB, CondInst);
    ReplaceInstWithInst(TermInst, NewTermInst);

    BasicBlock *DispatchFiniBB = SplitBlock(LoopExitBB, KmpcFiniCI, DT, LI);

    TermInst = LoopExitBB->getTerminator();
    TermInst->setSuccessor(0, DispatchHeaderBB);

    // Update Dispatch Header BB Branch instruction
    TermInst = DispatchHeaderBB->getTerminator();
    TermInst->setSuccessor(1, DispatchFiniBB);

    // Insert doacross_fini call for ordered(n)
    if (IsDoacrossLoop)
      VPOParoptUtils::genKmpcDoacrossFini(W, IdentTy, LoadTid, KmpcFiniCI);

    KmpcFiniCI->eraseFromParent();

    if (DT) {
      DT->changeImmediateDominator(DispatchHeaderBB, DispatchInitBB);
      DT->changeImmediateDominator(DispatchBodyBB, DispatchHeaderBB);

      //DT->changeImmediateDominator(DispatchFiniBB, DispatchHeaderBB);

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
    BasicBlock *TeamInitBB = KmpcTeamInitCI->getParent();
    BasicBlock *TeamExitBB = KmpcFiniCI->getParent();

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
    //   |                  par loop  <------_+            |
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
    LoadInst *TmpUD = new LoadInst(TeamUpperD, "team.ud.tmp", TeamLB);

    BasicBlock *TeamDispBodyBB = SplitBlock(TeamDispHeaderBB, TeamLB, DT, LI);
    TeamDispBodyBB->setName("team.dispatch.body");

    ICmpInst* MinUB;

    TerminatorInst *TermInst = TeamDispHeaderBB->getTerminator();

    if (IsLeft)
      MinUB = new ICmpInst(TermInst, PD, TmpUB, TmpUD, "team.ub.min");
    else
      MinUB = new ICmpInst(TermInst, PD, TmpUD, TmpUB, "team.ub.min");

    StoreInst *NewUB = new StoreInst(TmpUD, TeamUpperBnd, false, TermInst);

    BasicBlock *TeamDispMinUBB = SplitBlock(TeamDispHeaderBB, NewUB, DT, LI);
    TeamDispMinUBB->setName("team.dispatch.min.ub");

    TermInst = TeamDispHeaderBB->getTerminator();

    // Generate branch for team.dispatch.cond for get MIN upper bound
    TerminatorInst *NewTermInst = BranchInst::Create(TeamDispBodyBB,
                                                     TeamDispMinUBB, MinUB);
    ReplaceInstWithInst(TermInst, NewTermInst);

    BasicBlock *TeamInnerBodyBB = SplitBlock(TeamDispBodyBB, TeamST, DT, LI);
    TeamInnerBodyBB->setName("team.dispatch.inner.body");

    ICmpInst* TeamTopTest;

    TermInst = TeamDispBodyBB->getTerminator();

    if (IsLeft)
      TeamTopTest = new ICmpInst(TermInst, PD, TeamLB, TeamUB, "team.top.test");
    else
      TeamTopTest = new ICmpInst(TermInst, PD, TeamUB, TeamLB, "team.top.test");

    // Generate branch for team.dispatch.cond for get MIN upper bound
    TerminatorInst *TeamTopTestBI = BranchInst::Create(TeamInnerBodyBB,
                                                       TeamExitBB, TeamTopTest);
    ReplaceInstWithInst(TermInst, TeamTopTestBI);

    // Generate dispatch chunk increment BBlock
    BasicBlock *TeamDispLatchBB = SplitBlock(TeamExitBB,
                                             &(TeamExitBB->front()), DT, LI);

    TermInst = TeamExitBB->getTerminator();
    TeamExitBB->setName("team.dispatch.inc");

    // Generate team.inc.lb = team.new.lb + team.new.st
    BinaryOperator *IncLB = BinaryOperator::CreateAdd(
                                            TeamLB, TeamST, "team.inc.lb");
    IncLB->insertBefore(TermInst);

    // Generate team.inc.ub = team.new.lb + team.new.st
    BinaryOperator *IncUB = BinaryOperator::CreateAdd(
                                            TeamUB, TeamST, "team.inc.ub");
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
    WRegionUtils::updateBBForLoop(TeamDispHeaderBB, OuterLoop,
                                  L->getParentLoop(), LI);
    WRegionUtils::updateBBForLoop(TeamDispMinUBB, OuterLoop, L->getParentLoop(),
                                  LI);
    WRegionUtils::updateBBForLoop(TeamDispBodyBB, OuterLoop, L->getParentLoop(),
                                  LI);
    WRegionUtils::updateBBForLoop(TeamExitBB, OuterLoop, L->getParentLoop(),
                                  LI);
    WRegionUtils::updateBBForLoop(LoopRegionExitBB, OuterLoop,
                                  L->getParentLoop(), LI);
    OuterLoop->moveToHeader(TeamDispHeaderBB);

    wrnUpdateLiveOutVals(OuterLoop, LoopRegionExitBB, LiveOutVals, ECs);
    wrnUpdateSSAPreprocessForOuterLoop(OuterLoop, ValueToLiveinMap, LiveOutVals,
                                       ECs);
    rewriteUsesOfOutInstructions(ValueToLiveinMap, LiveOutVals, ECs);

    //// LLVM_DEBUG(dbgs() << "After distribute par Loop scheduling: "
    ////                   << *TeamInitBB->getParent() << "\n\n");
  }

  // There are new BBlocks generated, so we need to reset BBSet
  W->resetBBSet();
  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genLoopSchedulingCode\n");
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
  assert(W->isBBSetEmpty() &&
         "genMultiThreadedCode: BBSET should start empty");

  W->populateBBSet();

  bool Changed = false;

  // brief extract a W-Region to generate a function
  CodeExtractor CE(makeArrayRef(W->bbset_begin(), W->bbset_end()), DT, false,
                   nullptr, nullptr, false, true);

  assert(CE.isEligible());

  // Set up Fn Attr for the new function
  if (Function *NewF = CE.extractCodeRegion()) {

    // Set up the Calling Convention used by OpenMP Runtime Library
    CallingConv::ID CC = CallingConv::C;

    DT->verify(DominatorTree::VerificationLevel::Full);

    // Adjust the calling convention for both the function and the
    // call site.
    NewF->setCallingConv(CC);

    if (hasParentTarget(W))
      NewF->addFnAttr("target.declare", "true");

    assert(NewF->hasOneUse() && "New function should have one use");
    User *U = NewF->user_back();

    // Remove @llvm.dbg.declare, @llvm.dbg.value intrinsics from NewF
    // to prevent verification failures. This is due due to the
    // CodeExtractor not properly handling them at the moment.
    VPOUtils::stripDebugInfoInstrinsics(*NewF);

    CallInst *NewCall = cast<CallInst>(U);
    NewCall->setCallingConv(CC);

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

    if (IntelGeneralUtils::hasNextUniqueInstruction(MTFnCI)) {
      Instruction* NextI = IntelGeneralUtils::nextUniqueInstruction(MTFnCI);
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
    //         /    \
    //        /      \
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

    TerminatorInst *TermInst = ForkTestBB->getTerminator();

    Value *IfClauseValue = nullptr;

    if (!W->getIsTeams())
      IfClauseValue = W->getIf();

    ICmpInst* CondInst = nullptr;

    if (IfClauseValue) {
      Instruction *IfAndForkTestCI = BinaryOperator::CreateAnd(
                     IfClauseValue, ForkTestCI, "and.if.clause", TermInst);
      IfAndForkTestCI->setDebugLoc(TermInst->getDebugLoc());
      CondInst = new ICmpInst(TermInst, ICmpInst::ICMP_NE,
                              IfAndForkTestCI, ValueZero, "if.fork.test");
    }
    else
      CondInst = new ICmpInst(TermInst, ICmpInst::ICMP_NE,
                              ForkTestCI, ValueZero, "fork.test");

    TerminatorInst *NewTermInst = BranchInst::Create(ThenForkBB, ElseCallBB,
                                                     CondInst);
    ReplaceInstWithInst(TermInst, NewTermInst);

    TerminatorInst *NewForkBI = BranchInst::Create(
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

    W->resetBBSet(); // Invalidate BBSet after transformations

    Changed = true;
  }

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genMultiThreadedCode\n");
  return Changed;
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
      TerminatorInst *Term;
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
bool VPOParoptTransform::genMasterThreadCode(WRegionNode *W) {
  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genMasterThreadCode\n");
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *ExitBB = W->getExitBBlock();

  Instruction *InsertPt = EntryBB->getTerminator();

  // Generate __kmpc_master Call Instruction
  CallInst *MasterCI = VPOParoptUtils::genKmpcMasterOrEndMasterCall(
      W, IdentTy, TidPtrHolder, InsertPt, true);
  MasterCI->insertBefore(InsertPt);

  // LLVM_DEBUG(dbgs() << " MasterCI: " << *MasterCI << "\n\n");

  Instruction *InsertEndPt = ExitBB->getTerminator();

  // Generate __kmpc_end_master Call Instruction
  CallInst *EndMasterCI = VPOParoptUtils::genKmpcMasterOrEndMasterCall(
      W, IdentTy, TidPtrHolder, InsertEndPt, false);
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

  TerminatorInst *TermInst = MasterTestBB->getTerminator();

  ICmpInst* CondInst = new ICmpInst(TermInst, ICmpInst::ICMP_EQ,
                                    MasterCI, ValueOne, "");

  TerminatorInst *NewTermInst = BranchInst::Create(ThenMasterBB,
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

  TerminatorInst *TermInst = SingleTestBB->getTerminator();

  ICmpInst* CondInst = new ICmpInst(TermInst, ICmpInst::ICMP_EQ,
                                    SingleCI, ValueOne, "");

  TerminatorInst *NewTermInst = BranchInst::Create(ThenSingleBB,
                                                   EndSingleSuccBB, CondInst);
  ReplaceInstWithInst(TermInst, NewTermInst);

  DT->changeImmediateDominator(ThenSingleBB, SingleCI->getParent());
  DT->changeImmediateDominator(ThenSingleBB->getTerminator()->getSuccessor(0),
                               SingleCI->getParent());

  W->resetBBSet(); // Invalidate BBSet
  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genSingleThreadCode\n");
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
  if (W->getIsDepSource()) {

    // For doacross post, we send in the outer WRegion's loop index.
    auto *ParentNormIV = WParent->getWRNLoopInfo().getNormIV();
    assert(ParentNormIV && "Cannot find IV of outer construct.");

    // Generate __kmpc_doacross_post call
    CallInst *DoacrossPostCI = VPOParoptUtils::genDoacrossWaitOrPostCall(
        W, IdentTy, TidPtrHolder, InsertPt, ParentNormIV,
        true); // 'depend (source)'
    (void)DoacrossPostCI;
    assert(DoacrossPostCI && "Failed to emit doacross_post call.");
  }

  // Emit doacross wait call(s) for 'depend(sink:...)'
  for (DepSinkItem *DSI : W->getDepSink().items()) {
    // Generate __kmpc_doacross_wait call
    CallInst *DoacrossWaitCI = VPOParoptUtils::genDoacrossWaitOrPostCall(
        W, IdentTy, TidPtrHolder, InsertPt, DSI->getSinkExpr(),
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
  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genCriticalCode\n");
  assert(CriticalNode != nullptr && "Critical node is null.");

  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(TidPtrHolder != nullptr && "TidPtr is null.");

  assert(CriticalNode->isBBSetEmpty() &&
         "genCriticalCode: BBSET should start empty");

  // genKmpcCriticalSection() needs BBSet for error checking only;
  // In the future consider getting rid of this call to populateBBSet.
  CriticalNode->populateBBSet();

  StringRef LockNameSuffix = CriticalNode->getUserLockName();

  bool CriticalCallsInserted =
      LockNameSuffix.empty()
          ? VPOParoptUtils::genKmpcCriticalSection(CriticalNode, IdentTy,
                                                   TidPtrHolder)
          : VPOParoptUtils::genKmpcCriticalSection(
                CriticalNode, IdentTy, TidPtrHolder, LockNameSuffix);

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
bool VPOParoptTransform::genBarrierForFpLpAndLinears(WRegionNode *W) {

  // A barrier is needed for capturing the initial value of linear
  // variables.
  bool BarrierNeeded = W->canHaveLinear() && !((W->getLinear()).empty());

  // A barrier is also needed if a variable is marked as both firstprivate and
  // lastprivate.
  if (!BarrierNeeded && (W->canHaveLastprivate() && W->canHaveFirstprivate())) {
    LastprivateClause &LprivClause = W->getLpriv();
    for (LastprivateItem *LprivI : LprivClause.items()) {
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

  return genBarrier(W, false); // Implicit Barrier
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
bool VPOParoptTransform::genLastIterationCheck(WRegionNode *W, Value *IsLastVal,
                                               BasicBlock *&IfLastIterOut) {

  // No need to emit the branch if W doesn't have any linear or lastprivate var.
  if ((!W->canHaveLastprivate() || (W->getLpriv()).empty()) &&
      (!W->canHaveLinear() || (W->getLinear()).empty()))
    return false;

  assert(IsLastVal && "genLastIterationCheck: IsLastVal is null.");

  // First, create an empty predecessor BBlock for ExitBB of the WRegion.  (3)
  BasicBlock *ExitBBPredecessor = nullptr;
  createEmptyPrivFiniBB(W, ExitBBPredecessor);
  assert(ExitBBPredecessor && "genLoopLastIterationCheck: Couldn't create "
                              "empty BBlock before the exit BB.");

  // Next, we insert the branching code in the newly created BBlock.
  Instruction *BranchInsertPt = ExitBBPredecessor->getTerminator();
  IRBuilder<> Builder(BranchInsertPt);

  LoadInst *LastLoad = Builder.CreateLoad(IsLastVal);                   // (1)
  ConstantInt *ValueZero =
      ConstantInt::getSigned(Type::getInt32Ty(F->getContext()), 0);
  Value *LastCompare = Builder.CreateICmpNE(LastLoad, ValueZero);       // (2)

  TerminatorInst *Term = SplitBlockAndInsertIfThen(LastCompare, BranchInsertPt,
                                                   false, nullptr, DT, LI);
  Term->getParent()->setName("last.then");
  ExitBBPredecessor->getTerminator()->getSuccessor(1)->setName("last.done");

  IfLastIterOut = Term->getParent();

  LLVM_DEBUG(
      dbgs() << __FUNCTION__
             << ": Emitted if-then branch for checking last iteration.\n");
  return true;
}

// Insert a call to __kmpc_barrier() at the end of the construct
bool VPOParoptTransform::genBarrier(WRegionNode *W, bool IsExplicit) {

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genBarrier [explicit="
                    << IsExplicit << "]\n");

  // Create a new BB split from W's ExitBB to be used as InsertPt.
  // Reuse the util that does this for Reduction and Lastprivate fini code.
  BasicBlock *NewBB = nullptr;
  createEmptyPrivFiniBB(W, NewBB);
  Instruction *InsertPt = NewBB->getTerminator();

  VPOParoptUtils::genKmpcBarrier(W, TidPtrHolder, InsertPt, IdentTy,
                                 IsExplicit);

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genBarrier\n");
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
    // If the construct has an 'IF' clause, we need to generate code like:
    // if (if_expr != 0) {
    //   %1 = __kmpc_cancel[lationpoint](...);
    // }
    Function *F = EntryBB->getParent();
    LLVMContext &C = F->getContext();
    ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);

    auto *CondInst = new ICmpInst(InsertPt, ICmpInst::ICMP_NE, IfExpr,
                                  ValueZero, "cancel.if");

    Instruction *IfCancelThen =
        SplitBlockAndInsertIfThen(CondInst, InsertPt, false, nullptr, DT, LI);
    assert(IfCancelThen && "genCancelCode: Cannot split BB at Cancel If");

    InsertPt = IfCancelThen;
    LLVM_DEBUG(
        dbgs() << "genCancelCode: Emitted If-Then-Else for IF EXPR: if (";
        IfExpr->printAsOperand(dbgs());
        dbgs() << ") then <%x = __kmpc_cancel[lationpoint]>.\n");
  }

  CallInst *CancelCall = VPOParoptUtils::genKmpcCancelOrCancellationPointCall(
      W, IdentTy, TidPtrHolder, InsertPt, W->getCancelKind(),
      W->getIsCancellationPoint());

  (void)CancelCall;
  assert(CancelCall && "genCancelCode: Failed to emit call");

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genCancelCode\n");
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
      VPOAnalysisUtils::isIntelDirectiveOrClause(CI) &&
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

  // (1) Add the list of allocas where cancellation points' return values are
  // stored, as an operand bundle in the region.entry() directive.
  CI = VPOParoptUtils::addOperandBundlesInCall(
      CI, {{"QUAL.OMP.CANCELLATION.POINTS", CancellationPointAllocas}});

  LLVM_DEBUG(dbgs() << "propagateCancellationPointsToIR: Added "
                    << CancellationPoints.size()
                    << " Cancellation Points to: " << *CI << ".\n");
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

    resetValueInIntelClauseGeneric(W, CPAlloca);

    // The only uses of CPAlloca (1) should be in the intrinsic (2) and the
    // store (2).
    assert(CPAlloca->getNumUses() <= 2 &&
           "Unexpected number of uses for cancellation point alloca.");

    IntrinsicInst *RegionEntry = nullptr;                            // (2)
    StoreInst *CPStore = nullptr;                                    // (3)

    for (auto It = CPAlloca->user_begin(), IE = CPAlloca->user_end(); It != IE;
         ++It) {
      if (IntrinsicInst *Intrinsic = dyn_cast<IntrinsicInst>(*It))
        RegionEntry = Intrinsic;
      else if (StoreInst *Store = dyn_cast<StoreInst>(*It))
        CPStore = Store;
      else
        llvm_unreachable("Unexpected user of cancellation point alloca.");
    }

    assert(RegionEntry &&
           "Unable to find intrinsic using cancellation point alloca.");
    assert(VPOAnalysisUtils::isIntelDirectiveOrClause(RegionEntry) &&
           "Unexpected user of cancellation point alloca.");

    LLVMContext &C = F->getContext();

    // Remove the cancellation point alloca from the `region.entry` directive.
    //    region.entry(..., null, ...)                                  ; (2)
    RegionEntry->replaceUsesOfWith(
        CPAlloca, ConstantPointerNull::get(Type::getInt8PtrTy(C)));

    // Next, we delete (1) and (3). Now, CPStore may have been removed by some
    // dead-code elimination optimization. e.g.
    //
    //   if (expr) {
    //     %1 = __kmpc_cancel(...)
    //     store %1, %cp
    //   }
    //
    // 'expr' may be always false, and %1 and the store can be optimized away.
    if (CPStore)
      CPStore->eraseFromParent();

    CPAlloca->eraseFromParent();
    Changed = true;
  }

  LLVM_DEBUG(
      dbgs()
      << "\nExit VPOParoptTransform::clearCancellationPointAllocasFromIR\n");

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
  assert(W->isBBSetEmpty() &&
         "genCancellationBranchingCode: BBSET should start empty");
  W->populateBBSet();

  Function *F = W->getEntryBBlock()->getParent();
  LLVMContext &C = F->getContext();
  ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);
  bool Changed = false;

  // For a loop construct with static [even] scheduling,
  // __kmpc_static_fini(...) call should be made even if the construt is
  // cancelled.
  bool NeedStaticFiniCall =
      (W->getIsOmpLoop() &&
       (W->getIsSections() ||
        (VPOParoptUtils::getLoopScheduleKind(W) == WRNScheduleStaticEven ||
         VPOParoptUtils::getLoopScheduleKind(W) == WRNScheduleStatic)));

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
  bool NeedCancelBarrierForNonBarriers = isa<WRNParallelNode>(W);

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

    bool CancellationPointIsBarrier =
        (dyn_cast<CallInst>(CancellationPoint)
             ->getCalledFunction()
             ->getName() == "__kmpc_cancel_barrier");

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

    assert(IntelGeneralUtils::hasNextUniqueInstruction(CancellationPoint) &&
           "genCancellationBranchingCode: Cannot find successor of "
           "Cancellation Point");
    auto *NextI = IntelGeneralUtils::nextUniqueInstruction(CancellationPoint);

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
    TerminatorInst *TermInst = OrgBB->getTerminator();
    TerminatorInst *NewTermInst =
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
      auto *CancelExitBBDominator =
          DT->findNearestCommonDominator(CurrentCancelExitBB, OrgBB);

      DT->changeImmediateDominator(CurrentCancelExitBB, CancelExitBBDominator);
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
    }

    Changed = true;
  }

  W->resetBBSet(); // Invalidate BBSet after transformations
  LLVM_DEBUG(
      dbgs() << "\nExit VPOParoptTransform::genCancellationBranchingCode\n");

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
    resetValueInIntelClauseGeneric(W, I->getOrig());
  }
}

// Set the the arguments in the Intel compiler generated clause to be empty.
void VPOParoptTransform::resetValueInIntelClauseGeneric(WRegionNode *W,
                                                        Value *V) {
  if (!V)
    return;

  SmallVector<Instruction *, 8> IfUses;
  for (auto IB = V->user_begin(), IE = V->user_end(); IB != IE; ++IB) {
    if (Instruction *User = dyn_cast<Instruction>(*IB))
      if (W->contains(User->getParent()))
        IfUses.push_back(User);
  }

  while (!IfUses.empty()) {
    Instruction *UI = IfUses.pop_back_val();
    if (VPOAnalysisUtils::isIntelDirectiveOrClause(UI)) {
      LLVMContext &C = F->getContext();
      UI->replaceUsesOfWith(V, ConstantPointerNull::get(Type::getInt8PtrTy(C)));
      break;
    }
  }
}

// Generate the copyprivate code. Here is one example.
// #pragma omp single copyprivate ( a,b )
// LLVM IR output:
//     %copyprivate.agg.5 = alloca %struct.kmp_copy_privates.t, align 8
//     %14 = bitcast %struct.kmp_copy_privates.t* %copyprivate.agg.5 to i8**
//     store i8* %.0, i8** %14, align 8
//     %15 = getelementptr inbounds %struct.kmp_copy_privates.t,
//           %struct.kmp_copy_privates.t* %copyprivate.agg.5, i64 0, i32 1
//     store float* %b.fpriv, float** %15, align 8
//     %16 = load i32, i32* %tid, align 4
//     %17 = bitcast %struct.kmp_copy_privates.t* %copyprivate.agg.5 to i8*
//     call void @__kmpc_copyprivate({ i32, i32, i32, i32, i8* }*
//          nonnull @.kmpc_loc.0.0.16, i32 %16, i32 16, i8* nonnull %17,
//          i8* bitcast (void (%struct.kmp_copy_privates.t*,
//          %struct.kmp_copy_privates.t*)* @test_copy_priv_5 to i8*), i32 %13)
//          #11
//
bool VPOParoptTransform::genCopyPrivateCode(WRegionNode *W,
                                            AllocaInst *IsSingleThread) {
  bool Changed = false;
  CopyprivateClause &CprivClause = W->getCpriv();
  if (CprivClause.empty())
    return Changed;
  W->populateBBSet();
  Instruction *InsertPt = W->getExitBBlock()->getTerminator();
  IRBuilder<> Builder(InsertPt);

  SmallVector<Type *, 4> KmpCopyPrivatesVars;
  for (CopyprivateItem *CprivI : CprivClause.items()) {
    Value *Orig = CprivI->getOrig();
    KmpCopyPrivatesVars.push_back(Orig->getType());
  }

  LLVMContext &C = F->getContext();
  StructType *KmpCopyPrivateTy = StructType::get(
      C, makeArrayRef(KmpCopyPrivatesVars.begin(), KmpCopyPrivatesVars.end()),
      /* struct.kmp_copy_privates.t */false);

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
  W->resetBBSet();
  return Changed;
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
    Indices.clear();
    Indices.push_back(Builder.getInt32(0));
    Indices.push_back(Builder.getInt32(cnt++));
    Value *SrcGep =
        Builder.CreateInBoundsGEP(KmpCopyPrivateTy, SrcArg, Indices);
    Value *DstGep =
        Builder.CreateInBoundsGEP(KmpCopyPrivateTy, DstArg, Indices);
    LoadInst *SrcLoad = Builder.CreateLoad(SrcGep);
    LoadInst *DstLoad = Builder.CreateLoad(DstGep);
    Value *NewCopyPrivInst = genPrivatizationAlloca(
        W, CprivI->getOrig(), EntryBB->getTerminator(), ".cp.priv");
    genLprivFini(NewCopyPrivInst, DstLoad, EntryBB->getTerminator());
    NewCopyPrivInst->replaceAllUsesWith(SrcLoad);
    cast<AllocaInst>(NewCopyPrivInst)->eraseFromParent();
  }

  return FnCopyPriv;
}

// Add alias_scope and no_alias metadata to improve the alias
// results in the outlined function.
void VPOParoptTransform::improveAliasForOutlinedFunc(WRegionNode *W) {
  if (OptLevel < 2)
    return;
  W->populateBBSet();
  VPOUtils::genAliasSet(makeArrayRef(W->bbset_begin(), W->bbset_end()), AA,
                        &(F->getParent()->getDataLayout()));
  W->resetBBSet();
}
#endif // INTEL_COLLAB
