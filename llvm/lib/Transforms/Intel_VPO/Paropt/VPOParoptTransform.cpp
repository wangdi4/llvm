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
  DEBUG(dbgs() << "\nSTART: Gather WRegion Node List\n");

  NeedTID = NeedBID = false;
  VPOWRegionVisitor Visitor(WRegionList, NeedTID, NeedBID);
  WRegionUtils::forwardVisit(Visitor, WI->getWRGraph());

  DEBUG(dbgs() << "\nEND: Gather WRegion Node List\n");
  return;
}

static void debugPrintHeader(WRegionNode *W, bool IsPrepare) {
  if (IsPrepare)
    DEBUG(dbgs() << "\n\n === VPOParopt Prepare: ");
  else
    DEBUG(dbgs() << "\n\n === VPOParopt Transform: ");

  DEBUG(dbgs() << W->getName().upper() << " construct\n\n");
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
    DEBUG(dbgs() << "\n... No WRegion Candidates for Parallelization ...\n\n");
    return RoutineChanged;
  }

  bool NeedTID, NeedBID;

  // Collects the list of WRNs into WRegionList, and sets NeedTID and NeedBID
  // to true/false depending on whether it finds a WRN that needs the TID or
  // BID, respectively.
  gatherWRegionNodeList(NeedTID, NeedBID);

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
          genCodemotionFenceforAggrData(W);
          Changed |= propagateCancellationPointsToIR(W);
        }
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed = clearCodemotionFenceIntrinsic(W);
          // Privatization is enabled for both Prepare and Transform passes
          Changed |= genPrivatizationCode(W);
          Changed |= genFirstPrivatizationCode(W);
          Changed |= genReductionCode(W);
          Changed |= genCancellationBranchingCode(W);
          Changed |= genDestructorCode(W);
          Changed |= genMultiThreadedCode(W);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNParallelSections:
      case WRegionNode::WRNParallelLoop:
        debugPrintHeader(W, IsPrepare);
        if (Mode & ParPrepare) {
          regularizeOMPLoop(W);
          genCodemotionFenceforAggrData(W);
          Changed |= propagateCancellationPointsToIR(W);
        }
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          Changed = clearCodemotionFenceIntrinsic(W);
          Changed |= regularizeOMPLoop(W, false);
          AllocaInst *IsLastVal = nullptr;
          Changed |= genLoopSchedulingCode(W, IsLastVal);
          // Privatization is enabled for both Prepare and Transform passes
          Changed |= genPrivatizationCode(W);
          Changed |= genLastPrivatizationCode(W, IsLastVal);
          Changed |= genFirstPrivatizationCode(W);
          Changed |= genReductionCode(W);
          Changed |= genCancellationBranchingCode(W);
          Changed |= genDestructorCode(W);
          Changed |= genMultiThreadedCode(W);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNTask:
        if (Mode & ParPrepare) {
          Changed |= propagateCancellationPointsToIR(W);
        }
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          debugPrintHeader(W, false);
          StructType *KmpTaskTTWithPrivatesTy;
          StructType *KmpSharedTy;
          Value *LastIterGep;
          Changed = genTaskInitCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy,
                                    LastIterGep);
          Changed |= genPrivatizationCode(W);
          Changed |= genFirstPrivatizationCode(W);
          Changed |= genLastPrivatizationCode(W, LastIterGep);
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
          StructType *KmpTaskTTWithPrivatesTy;
          StructType *KmpSharedTy;
          Value *LBPtr, *UBPtr, *STPtr, *LastIterGep;
          Changed |=
              genTaskLoopInitCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy,
                                  LBPtr, UBPtr, STPtr, LastIterGep);
          Changed |= genPrivatizationCode(W);
          Changed |= genFirstPrivatizationCode(W);
          Changed |= genLastPrivatizationCode(W, LastIterGep);
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
        if (Mode & ParPrepare) {
          Changed = genTaskgroupRegion(W);
          RemoveDirectives = true;
        }
        break;

      case WRegionNode::WRNVecLoop:
        // Privatization is enabled for SIMD Transform passes
        if ((Mode & OmpVec) && (Mode & ParTrans)) {
          debugPrintHeader(W, false);
          Changed = genPrivatizationCode(W);
          // keep SIMD directives; will be processed by the Vectorizer
          RemoveDirectives = false;
          RemovePrivateClauses = false;
        }
        break;
      case WRegionNode::WRNAtomic:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, true);
          Changed = VPOParoptAtomics::handleAtomic(dyn_cast<WRNAtomicNode>(W),
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
          Changed = clearCodemotionFenceIntrinsic(W);
          Changed |= regularizeOMPLoop(W, false);
          Changed |= genLoopSchedulingCode(W, IsLastVal);
          Changed |= genPrivatizationCode(W);
          Changed |= genLastPrivatizationCode(W, IsLastVal);
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
          Changed = genMasterThreadCode(W);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNCritical:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, true);
          Changed = genCriticalCode(dyn_cast<WRNCriticalNode>(W));
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNOrdered:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, true);
          Changed = genOrderedThreadCode(W);
          RemoveDirectives = true;
        }
        break;
      case WRegionNode::WRNBarrier:
        if (Mode & ParPrepare) {
          debugPrintHeader(W, true);
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
      DEBUG(dbgs() << "   === WRN #" << W->getNumber() << " transformed.\n\n");
    }
    else
      DEBUG(dbgs() << "   === WRN #" << W->getNumber()
                                                   << " NOT transformed.\n\n");
  }

  for (WRegionNode *R : WRegionList)
    delete R;
  WRegionList.clear();
  return RoutineChanged;
}

Value *VPOParoptTransform::genReductionMinMaxInit(ReductionItem *RedI,
                                                  Type *Ty, bool IsMax) {
  Value *V;

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
  Value *V;
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
  Value *IsGT; // compares Rhs1 > Rhs2

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
  Value *Res;

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
                                          Instruction *InsertPt) {
  AllocaInst *NewAI = dyn_cast<AllocaInst>(RedI->getNew());
  Type *AllocaTy = NewAI->getAllocatedType();
  Type *ScalarTy = AllocaTy->getScalarType();
  const DataLayout &DL = InsertPt->getModule()->getDataLayout();

  IRBuilder<> Builder(InsertPt);
  if (!AllocaTy->isSingleValueType() ||
      !DL.isLegalInteger(DL.getTypeSizeInBits(ScalarTy)) ||
      DL.getTypeSizeInBits(ScalarTy) % 8 != 0) {
    genRedAggregateInitOrFini(RedI, NewAI, OldV, InsertPt, false);
  } else {
    LoadInst *OldLoad = Builder.CreateLoad(OldV);
    LoadInst *NewLoad = Builder.CreateLoad(NewAI);
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
                                                   bool IsInit) {

  IRBuilder<> Builder(InsertPt);
  auto EntryBB = Builder.GetInsertBlock();

  Type *DestElementTy = nullptr;
  Value *DestArrayBegin = nullptr;

  auto NumElements = VPOParoptUtils::genArrayLength(
      AI, IsInit ? AI : OldV, InsertPt, Builder, DestElementTy, DestArrayBegin);
  auto DestAddr = Builder.CreateBitCast(DestArrayBegin,
                                        PointerType::getUnqual(DestElementTy));

  Type *SrcElementTy = nullptr;
  Value *SrcArrayBegin = nullptr;
  Value *SrcAddr = nullptr;

  if (!IsInit) {
    VPOParoptUtils::genArrayLength(AI, AI, InsertPt, Builder, SrcElementTy,
                                   SrcArrayBegin);
    SrcAddr = Builder.CreateBitCast(SrcArrayBegin,
                                    PointerType::getUnqual(SrcElementTy));
  }

  auto DestBegin = DestAddr;
  auto SrcBegin = SrcAddr;

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

  AllocaInst *AI = dyn_cast<AllocaInst>(FprivI->getNew());
  Type *AllocaTy = AI->getAllocatedType();
  Type *ScalarTy = AllocaTy->getScalarType();
  const DataLayout &DL = InsertPt->getModule()->getDataLayout();

  IRBuilder<> Builder(InsertPt);
  if (Function *Cctor = FprivI->getCopyConstructor())
    VPOParoptUtils::genCopyConstructorCall(Cctor, FprivI->getNew(),
                                           FprivI->getOrig(), InsertPt);
  else if (!AllocaTy->isSingleValueType() ||
      !DL.isLegalInteger(DL.getTypeSizeInBits(ScalarTy)) ||
      DL.getTypeSizeInBits(ScalarTy) % 8 != 0 || AI->isArrayAllocation())
    VPOParoptUtils::genMemcpy(AI, FprivI->getOrig(), DL, AI->getAlignment(),
                              InsertPt->getParent());
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
  AllocaInst *AI = dyn_cast<AllocaInst>(NewV);
  Type *AllocaTy = AI->getAllocatedType();
  Type *ScalarTy = AllocaTy->getScalarType();
  const DataLayout &DL = InsertPt->getModule()->getDataLayout();

  IRBuilder<> Builder(InsertPt);
  if (!AllocaTy->isSingleValueType() ||
      !DL.isLegalInteger(DL.getTypeSizeInBits(ScalarTy)) ||
      DL.getTypeSizeInBits(ScalarTy) % 8 != 0) {
    VPOParoptUtils::genMemcpy(OldV, AI, DL, AI->getAlignment(),
                              InsertPt->getParent());
  } else {
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
                                          Instruction *InsertPt) {

  AllocaInst *AI = dyn_cast<AllocaInst>(RedI->getNew());
  Type *AllocaTy = AI->getAllocatedType();
  Type *ScalarTy = AllocaTy->getScalarType();
  const DataLayout &DL = InsertPt->getModule()->getDataLayout();

  IRBuilder<> Builder(InsertPt);
  if (!AllocaTy->isSingleValueType() ||
      !DL.isLegalInteger(DL.getTypeSizeInBits(ScalarTy)) ||
      DL.getTypeSizeInBits(ScalarTy) % 8 != 0) {
    genRedAggregateInitOrFini(RedI, AI, nullptr, InsertPt, true);
  } else {
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

      BasicBlock *LoopExitBB;
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

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genReductionCode\n");

  ReductionClause &RedClause = W->getRed();
  if (!RedClause.empty()) {

    assert(W->isBBSetEmpty() && "genReductionCode: BBSET should start empty");
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
      NewRedInst = genPrivatizationAlloca(W, Orig, &EntryBB->front(), ".red");
      genPrivatizationReplacement(W, Orig, NewRedInst, RedI);
      RedI->setNew(NewRedInst);
      createEmptyPrvInitBB(W, RedInitEntryBB);
      genReductionInit(RedI, RedInitEntryBB->getTerminator());
      BasicBlock *BeginBB;
      createEmptyPrivFiniBB(W, BeginBB);
      genReductionFini(RedI, RedI->getOrig(), BeginBB->getTerminator());
      DEBUG(dbgs() << "genReductionCode: reduced " << *Orig << "\n");
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
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genReductionCode\n");
  return Changed;
}

// Collect the instructions of global variable uses recursively to
// handle the case of nested constant expressions.
void VPOParoptTransform::collectGlobalUseInsnsRecursively(
    WRegionNode *W, SmallVectorImpl<Instruction *> &RewriteCons,
    ConstantExpr *CE) {
  for (Use &U : CE->uses()) {
    User *UR = U.getUser();
    if (Instruction *I = dyn_cast<Instruction>(UR)) {
      if (W->contains(I->getParent()))
       RewriteCons.push_back(I);
    } else if (ConstantExpr *C = dyn_cast<ConstantExpr>(UR))
    collectGlobalUseInsnsRecursively(W, RewriteCons, C);
  }
}

// A utility to privatize the variables within the region.
Value *
VPOParoptTransform::genPrivatizationAlloca(WRegionNode *W, Value *PrivValue,
                                           Instruction *InsertPt,
                                           const StringRef VarNameSuff) {
  // DEBUG(dbgs() << "Private Instruction Defs: " << *PrivInst << "\n");
  // Generate a new Alloca instruction as privatization action
  AllocaInst *NewPrivInst;

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
    SmallVector<Instruction *, 8> RewriteCons;
    for (auto IB = GV->user_begin(), IE = GV->user_end(); IB != IE; ++IB) {
      if (ConstantExpr *CE = dyn_cast<ConstantExpr>(*IB))
        collectGlobalUseInsnsRecursively(W, RewriteCons, CE);
    }
    while (!RewriteCons.empty()) {
      Instruction *I = RewriteCons.pop_back_val();
      IntelGeneralUtils::breakExpressions(I);
    }
  } else {
    // TODO: Privatize Value that is neither global nor alloca
    DEBUG(dbgs() << "\ngenPrivatizationAlloca: TODO: Handle Arguments.\n");
    llvm_unreachable("genPrivatizationAlloca: unsupported private item");
  }

  return NewPrivInst;
}

// Replace the variable with the privatized variable
void VPOParoptTransform::genPrivatizationReplacement(WRegionNode *W,
                                                     Value *PrivValue,
                                                     Value *NewPrivInst,
                                                     Item *IT) {
  SmallVector<Instruction *, 8> PrivUses;
  for (auto IB = PrivValue->user_begin(), IE = PrivValue->user_end(); IB != IE;
       ++IB) {
    if (Instruction *User = dyn_cast<Instruction>(*IB))
      if (W->contains(User->getParent()))
        PrivUses.push_back(User);
  }
  // Replace all USEs of each PrivItem with its new PrivItem in the
  // W-Region (parallel loop/region/section ... etc.)
  while (!PrivUses.empty()) {
    Instruction *UI = PrivUses.pop_back_val();
    UI->replaceUsesOfWith(PrivValue, NewPrivInst);
    // DEBUG(dbgs() << "New Instruction uses PrivItem: " << *UI << "\n");
  }
}

bool VPOParoptTransform::genFirstPrivatizationCode(WRegionNode *W) {

  bool Changed = false;

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genFirstPrivatizationCode\n");

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
/*
      assert((isa<GlobalVariable>(Orig) || isa<AllocaInst>(Orig)) &&
             "genFirstPrivatizationCode: Unexpected firstprivate variable");
*/
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
        DEBUG(dbgs() << "\n  genFirstPrivatizationCode: (" << *Orig
                     << ") is also lastprivate\n");
      }

      if (!ForTask) {
        createEmptyPrvInitBB(W, PrivInitEntryBB);
        genFprivInit(FprivI, PrivInitEntryBB->getTerminator());
      }
      DEBUG(dbgs() << "genFirstPrivatizationCode: firstprivatized "
                   << *Orig << "\n");
    }
    Changed = true;
    W->resetBBSet(); // Invalidate BBSet
  }

  DEBUG(dbgs() << "\nExit VPOParoptTransform::genFirstPrivatizationCode\n");
  return Changed;
}

bool VPOParoptTransform::genLastPrivatizationCode(WRegionNode *W,
                                                  Value *IsLastVal) {
  bool Changed = false;

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genLastPrivatizationCode\n");

  assert(W->isBBSetEmpty() &&
         "genLastPrivatizationCode: BBSET should start empty");

  if (!W->canHaveLastprivate())
    return Changed;

  LastprivateClause &LprivClause = W->getLpriv();
  if (!LprivClause.empty()) {
    W->populateBBSet();
    FirstprivateItem *FprivI = nullptr;
    if (W->canHaveFirstprivate()) {
      for (LastprivateItem *LprivI : LprivClause.items()) {
        FprivI = LprivI->getInFirstprivate();
        if (FprivI)
          break;
      }
    }
    // If a variable is marked as firstprivate and lastprivate, the
    // compiler has to generate the barrier. Please note that it is
    // unnecessary to generate the barrier if the firstprivate is
    // scalar and it is passed by value.
    if (FprivI)
      genBarrier(W, false); // implicit barrier

    bool ForTask = W->getWRegionKindID() == WRegionNode::WRNTaskloop ||
                   W->getWRegionKindID() == WRegionNode::WRNTask;
    BasicBlock *EntryBB = W->getEntryBBlock();
    BasicBlock *BeginBB = nullptr;
    createEmptyPrivFiniBB(W, BeginBB);
    IRBuilder<> Builder(BeginBB);
    Builder.SetInsertPoint(BeginBB->getTerminator());

    assert(IsLastVal && "genLastPrivatizationCode: IsLastVal not initialized");
    LoadInst *LastLoad = Builder.CreateLoad(IsLastVal);
    ConstantInt *ValueZero =
        ConstantInt::getSigned(Type::getInt32Ty(F->getContext()), 0);
    Value *LastCompare = Builder.CreateICmpNE(LastLoad, ValueZero);
    TerminatorInst *Term = SplitBlockAndInsertIfThen(
        LastCompare, BeginBB->getTerminator(), false, nullptr, DT, LI);
    Term->getParent()->setName("lastprivate.then");
    BeginBB->getTerminator()->getSuccessor(1)->setName("lastprivate.done");
    BeginBB = Term->getParent();

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
        genLprivFini(LprivI, BeginBB->getTerminator());
      } else
        genLprivFiniForTaskLoop(LprivI->getParm(), LprivI->getNew(),
                                BeginBB->getTerminator());
    }
    Changed = true;
    W->resetBBSet(); // Invalidate BBSet
  }

  DEBUG(dbgs() << "\nExit VPOParoptTransform::genLastPrivatizationCode\n");
  return Changed;
}

// Generate destructor calls for [first|last]private variables
bool VPOParoptTransform::genDestructorCode(WRegionNode *W) {
  if (!WRegionUtils::needsDestructors(W)) {
    DEBUG(dbgs() << "\nVPOParoptTransform::genDestructorCode: No dtors\n");
    return false;
  }

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genDestructorCode\n");

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

  DEBUG(dbgs() << "\nExit VPOParoptTransform::genDestructorCode\n");
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
  // DEBUG(dbgs() << "replaceValueWithinRegion: " << *V << "\n");

  // Find instructions in W that use V
  SmallVector<Instruction *, 8> Users;
  if (!WRegionUtils::findUsersInRegion(W, V, &Users))
    return; // Found no applicable uses of V in W's body

  // Create a new @llvm.invariant.group.barrier for V
  BasicBlock *EntryBB = W->getEntryBBlock();
  IRBuilder<> Builder(EntryBB->getTerminator());
  Value *NewI = Builder.CreateInvariantGroupBarrier(V);

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
      // DEBUG(dbgs() << "Skipping Fence: " << *User << "\n");
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
        // DEBUG(dbgs() << "Skipping BitCast: " << *BCI << "\n");
        continue;
      }
    }

    // DEBUG(dbgs() << "Before Replacement: " << *User << "\n");
    User->replaceUsesOfWith(V, NewI);
    // DEBUG(dbgs() << "After Replacement: " << *User << "\n");

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
      // DEBUG(dbgs() << "Before Replacement: " << *NewInstr << "\n");
      NewInstr->replaceUsesOfWith(V, NewI);
      // DEBUG(dbgs() << "After Replacement: " << *NewInstr << "\n");
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
}

// Return true if the instuction is a call to
// @llvm.invariant.group.barrier
CallInst*  VPOParoptTransform::isFenceCall(Instruction *I) {
  if (CallInst *CI = dyn_cast<CallInst>(I))
    if (CI->getCalledFunction() && CI->getCalledFunction()->getIntrinsicID() ==
                                       Intrinsic::invariant_group_barrier)
      return CI;
  return nullptr;
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

  if (WRegionUtils::isDoWhileLoop(L))
    fixOmpDoWhileLoopImpl(L);
  else if (WRegionUtils::isWhileLoop(L))
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
        } else if (Pred == CmpInst::ICMP_SGT) {
          Value *Operand = CondInst->getOperand(0);
          if (Operand == Inc) {
            CondInst->setPredicate(CmpInst::ICMP_SLE);
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

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genPrivatizationCode\n");

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

      if (isa<GlobalVariable>(Orig) || isa<AllocaInst>(Orig)) {
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
          IRBuilder<> Builder(EntryBB->getTerminator());
          Builder.CreateStore(Builder.CreateLoad(PrivI->getNew()), NewPrivInst);
          Builder.SetInsertPoint(ExitBB->getTerminator());
          Builder.CreateStore(Builder.CreateLoad(NewPrivInst), PrivI->getNew());
        }

        DEBUG(dbgs() << "genPrivatizationCode: privatized " << *Orig << "\n");
      } else
        DEBUG(dbgs() << "genPrivatizationCode: " << *Orig
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
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genPrivatizationCode\n");
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
    Value *V, *OV;
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
    LiveOutVals.insert(&I);
    buildECs(&L, dyn_cast<PHINode>(&I), ECs);
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
    PHINode *PN = dyn_cast<PHINode>(ExitVal);
    if (!PN)
      continue;
    FirstLoopExitBB = PN->getParent();
    SSA.Initialize(ExitVal->getType(), ExitVal->getName());

    BasicBlock *OrigPreheader = nullptr;
    Value *OrigPreHeaderVal = nullptr;

    for (auto M : ValueToLiveinMap) {
      if (ECs.findLeader(M.first) == ECs.findLeader(PN)) {
        OrigPreheader = M.second.second;
        OrigPreHeaderVal = M.second.first;
        SSA.AddAvailableValue(M.second.second, M.second.first);
        break;
      }
    }
    SSA.AddAvailableValue(PN->getParent(), PN);
    assert(OrigPreheader && OrigPreHeaderVal &&
           "rewriteUsesOfOutInstructions: live in value is missing\n");
    for (Value::use_iterator UI = ExitVal->use_begin(), UE = ExitVal->use_end();
         UI != UE;) {
      Use &U = *UI;
      ++UI;

      Instruction *UserInst = cast<Instruction>(U.getUser());
      if (!isa<PHINode>(UserInst)) {
        BasicBlock *UserBB = UserInst->getParent();

        if (UserBB == FirstLoopExitBB)
          continue;

        if (UserBB == OrigPreheader) {
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
  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genLoopSchedulingCode\n");

  assert(W->getIsOmpLoop() && "genLoopSchedulingCode: not a loop-type WRN");

  Loop *L = W->getWRNLoopInfo().getLoop();

  assert(L && "genLoopSchedulingCode: Loop not found");

  DEBUG(dbgs() << "--- Parallel For LoopInfo: \n" << *L);
  DEBUG(dbgs() << "--- Loop Preheader: " << *(L->getLoopPreheader()) << "\n");
  DEBUG(dbgs() << "--- Loop Header: " << *(L->getHeader()) << "\n");
  DEBUG(dbgs() << "--- Loop Latch: " << *(L->getLoopLatch()) << "\n\n");

#if 0
  DEBUG(dbgs() << "---- Loop Induction: "
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

  Instruction *InsertPt = dyn_cast<Instruction>(
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

  IRBuilder<> B(InsertPt);
  if (InitVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    InitVal = B.CreateSExtOrTrunc(InitVal, IndValTy);

  StoreInst *Tmp0 = new StoreInst(InitVal, LowerBnd, false, InsertPt);
  Tmp0->setAlignment(4);

  Value *UpperBndVal = VPOParoptUtils::computeOmpUpperBound(W, InsertPt);

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

  Value *ChunkVal = (SchedKind == WRNScheduleStaticEven ||
                     SchedKind == WRNScheduleOrderedStaticEven) ?
                                  ValueOne : W->getSchedule().getChunkExpr();

  DEBUG(dbgs() << "--- Schedule Chunk Value: " << *ChunkVal << "\n\n");

  if (SchedKind == WRNScheduleStaticEven || SchedKind == WRNScheduleStatic) {
    // Generate __kmpc__for_static_init_4{u}/8{u} Call Instruction
    KmpcInitCI = VPOParoptUtils::genKmpcStaticInit(W, IdentTy,
                               LoadTid, SchedType, IsLastVal, LowerBnd,
                               UpperBnd, Stride, StrideVal, ChunkVal,
                               Size, IsUnsigned, InsertPt);
  }
  else {
    // Generate __kmpc_dispatch_init_4{u}/8{u} Call Instruction
    KmpcInitCI = VPOParoptUtils::genKmpcDispatchInit(W, IdentTy,
                               LoadTid, SchedType, InitVal, UpperBndVal,
                               StrideVal, ChunkVal, Size, IsUnsigned, InsertPt);

    // Generate __kmpc_dispatch_next_4{u}/8{u} Call Instruction
    KmpcNextCI = VPOParoptUtils::genKmpcDispatchNext(W, IdentTy,
                               LoadTid, IsLastVal, LowerBnd,
                               UpperBnd, Stride, Size, IsUnsigned, InsertPt);
  }

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

    if (DT)
      DT->changeImmediateDominator(LoopExitBB, StaticInitBB);

    wrnUpdateLiveOutVals(L, LoopRegionExitBB, LiveOutVals, ECs);
    rewriteUsesOfOutInstructions(ValueToLiveinMap, LiveOutVals, ECs);

  }
  else if (SchedKind == WRNScheduleStatic) {

    //// DEBUG(dbgs() << "Before Loop Scheduling : "
    ////              << *(LoopExitBB->getParent()) << "\n\n");

    BasicBlock *StaticInitBB = KmpcInitCI->getParent();

    KmpcFiniCI = VPOParoptUtils::genKmpcStaticFini(W,
                                        IdentTy, LoadTid, InsertPt);
    KmpcFiniCI->setCallingConv(CallingConv::C);

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

    //// DEBUG(dbgs() << "After Loop Scheduling : "
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

  // There are new BBlocks generated, so we need to reset BBSet
  W->resetBBSet();
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genLoopSchedulingCode\n");
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
  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genMultiThreadedCode\n");
  assert(W->isBBSetEmpty() &&
         "genMultiThreadedCode: BBSET should start empty");

  W->populateBBSet();

  bool Changed = false;

  // brief extract a W-Region to generate a function
  CodeExtractor CE(makeArrayRef(W->bbset_begin(), W->bbset_end()), DT, false);

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

    CallInst *NewCall = cast<CallInst>(U);
    NewCall->setCallingConv(CC);

    CallSite CS(NewCall);

    unsigned int TidArgNo = 0;
    bool IsTidArg = false;

    for (auto I = CS.arg_begin(), E = CS.arg_end(); I != E; ++I) {
      if (*I == TidPtrHolder) {
        IsTidArg = true;
        DEBUG(dbgs() << " NewF Tid Argument: " << *(*I) << "\n");
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

    DEBUG(dbgs() << " New Call to MTFn: " << *NewCall << "\n");
    // Pass all the same arguments of the extracted function.
    for (auto I = CS.arg_begin(), E = CS.arg_end(); I != E; ++I) {
      if (*I != TidPtrHolder) {
        DEBUG(dbgs() << " NewF Arguments: " << *(*I) << "\n");
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

    Value* IfClauseValue = W->getIf();

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
    Value *NumThreads = W->getNumThreads();

    if (NumThreads) {
      LoadInst *Tid = new LoadInst(TidPtrHolder, "my.tid", ForkCI);
      Tid->setAlignment(4);
      VPOParoptUtils::genKmpcPushNumThreads(W,
                                            IdentTy, Tid, NumThreads, ForkCI);
    }

    // Remove the serial call to MTFn function from the program, reducing
    // the use-count of MTFn
    // MTFnCI->eraseFromParent();

    W->resetBBSet(); // Invalidate BBSet after transformations

    Changed = true;
  }

  DEBUG(dbgs() << "\nExit VPOParoptTransform::genMultiThreadedCode\n");
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
      VPOParoptUtils::genMemcpy(
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
  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genMasterThreadCode\n");
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *ExitBB = W->getExitBBlock();

  Instruction *InsertPt = EntryBB->getTerminator();

  // Generate __kmpc_master Call Instruction
  CallInst *MasterCI = VPOParoptUtils::genKmpcMasterOrEndMasterCall(
      W, IdentTy, TidPtrHolder, InsertPt, true);
  MasterCI->insertBefore(InsertPt);

  //DEBUG(dbgs() << " MasterCI: " << *MasterCI << "\n\n");

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
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genMasterThreadCode\n");
  return true; // Changed
}

// Generate code for single/end single construct and update LLVM control-flow
// and dominator tree accordingly
bool VPOParoptTransform::genSingleThreadCode(WRegionNode *W,
                                             AllocaInst *&IsSingleThread) {
  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genSingleThreadCode\n");
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
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genSingleThreadCode\n");
  return true;  // Changed
}

// Generate code for ordered/end ordered construct for preserving ordered
// region execution order
bool VPOParoptTransform::genOrderedThreadCode(WRegionNode *W) {
  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genOrderedThreadCode\n");
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

  //BasicBlock *OrderedBB = OrderedCI->getParent();
  //DEBUG(dbgs() << " Ordered Entry BBlock: " << *OrderedBB << "\n\n");

  //BasicBlock *EndOrderedBB = EndOrderedCI->getParent();
  //DEBUG(dbgs() << " Ordered Exit BBlock: " << *EndOrderedBB << "\n\n");

  W->resetBBSet(); // Invalidate BBSet
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genOrderedThreadCode\n");
  return true;  // Changed
}

// Generates code for the OpenMP critical construct.
bool VPOParoptTransform::genCriticalCode(WRNCriticalNode *CriticalNode) {
  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genCriticalCode\n");
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

  DEBUG(dbgs() << __FUNCTION__ << ": Handling of Critical Node: "
               << (CriticalCallsInserted ? "Successful" : "Failed") << ".\n");

  assert(CriticalCallsInserted && "Failed to create critical section. \n");

  CriticalNode->resetBBSet(); // Invalidate BBSet
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genCriticalCode\n");
  return CriticalCallsInserted;
}

// Insert a call to __kmpc_barrier() at the end of the construct
bool VPOParoptTransform::genBarrier(WRegionNode *W, bool IsExplicit) {

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genBarrier [explicit="
               << IsExplicit << "]\n");

  // Create a new BB split from W's ExitBB to be used as InsertPt.
  // Reuse the util that does this for Reduction and Lastprivate fini code.
  BasicBlock *NewBB = nullptr;
  createEmptyPrivFiniBB(W, NewBB);
  Instruction *InsertPt = NewBB->getTerminator();

  VPOParoptUtils::genKmpcBarrier(W, TidPtrHolder, InsertPt, IdentTy,
                                 IsExplicit);

  DEBUG(dbgs() << "\nExit VPOParoptTransform::genBarrier\n");
  return true;
}

// Create a __kmpc_flush() call and insert it into W's EntryBB
bool VPOParoptTransform::genFlush(WRegionNode *W) {

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genFlush\n");

  BasicBlock *EntryBB = W->getEntryBBlock();
  Instruction *InsertPt = EntryBB->getTerminator();
  VPOParoptUtils::genKmpcFlush(W, IdentTy, InsertPt);

  DEBUG(dbgs() << "\nExit VPOParoptTransform::genFlush\n");
  return true;
}

// Insert a call to __kmpc_cancel/__kmpc_cancellation_point at the end of the
// construct
bool VPOParoptTransform::genCancelCode(WRNCancelNode *W) {

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genCancelCode\n");

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
    DEBUG(dbgs() << "genCancelCode: Emitted If-Then-Else for IF EXPR: if (";
          IfExpr->printAsOperand(dbgs());
          dbgs() << ") then <%x = __kmpc_cancel[lationpoint]>.\n");
  }

  CallInst *CancelCall = VPOParoptUtils::genKmpcCancelOrCancellationPointCall(
      W, IdentTy, TidPtrHolder, InsertPt, W->getCancelKind(),
      W->getIsCancellationPoint());

  (void)CancelCall;
  assert(CancelCall && "genCancelCode: Failed to emit call");

  DEBUG(dbgs() << "\nExit VPOParoptTransform::genCancelCode\n");
  return true;
}

// Propagate all cancellation points from the body of W, to the 'region.exit'
// directive for the region.
bool VPOParoptTransform::propagateCancellationPointsToIR(WRegionNode *W) {

  if (!W->canHaveCancellationPoints())
    return false;

  auto &CancellationPoints = W->getCancellationPoints();
  if (CancellationPoints.empty())
    return false;

  // Find the end.region() directive intrinsic.
  Instruction *Inst = W->getExitBBlock()->getFirstNonPHI();
  CallInst *CI = dyn_cast<CallInst>(Inst);

  assert(CI && "propagateCancellationPointsToIR: Exit BBlocks's first "
               "non-PHI Instruction is not a Call");
  assert(
      VPOAnalysisUtils::isIntelDirectiveOrClause(CI) &&
      "propagateCancellationPointsToIR: Cannot find region.exit() directive");

  // OpndBundles take Values, so need to cast the SmallVector of Instructions to
  // SmallVector of Values.
  SmallVector<Value *, 2> CancellationPointsAsValues(CancellationPoints.begin(),
                                                     CancellationPoints.end());

  // Add the list of cancellation points as an operand bundle in the
  // end.region() directive.
  CI = VPOParoptUtils::addOperandBundlesInCall(
      CI, {{"QUAL.OMP.CANCELLATION.POINTS", CancellationPointsAsValues}});

  DEBUG(dbgs() << "propagateCancellationPointsToIR: Added "
               << CancellationPoints.size() << " Cancellation Points to: "
               << *CI << ".\n");
  return true;

  // TODO: Add PHIs to avoid the issue of "Instruction does not dominate all uses".
  // That is seen if we use opt and dump IR after vpo-paropt-prepare and before
  // vpo-paropt.
}

// Insert If-Then-Else branches from each Cancellation Point in W, to
// jump to the end of W if the Cancellation Point is non-zero.
bool VPOParoptTransform::genCancellationBranchingCode(WRegionNode *W) {

  if (!W->canHaveCancellationPoints())
    return false;

  auto &CancellationPoints = W->getCancellationPoints();
  if (CancellationPoints.empty())
    return false;

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genCancellationBranchingCode\n");
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
  DEBUG(dbgs() << "genCancellationBranchingCode: Created CancelExitBB: [";
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
    //    | region.exit(... %x)   |
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

    DEBUG(auto &OS = dbgs();
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
    //    | region.exit(..., %x)          |
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

      DEBUG(dbgs() << "genCancellationBranchingCode: Created predecessor of "
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

      DEBUG(dbgs() << "genCancellationBranchingCode: Created BB for "
                      "non-barrier cancellation points: [";
            CancelExitBBForNonBarriers->printAsOperand(dbgs());
            dbgs() << "] containing '__kmpc_cancel_barrier' call.\n");
    }

    // Finally, remove the cancellation point from the `end.region` directive.
    //    +---------------+---------------+
    //    | region.exit(..., null)        |
    //    +-------------------------------+
    resetValueInIntelClauseGeneric(W, CancellationPoint);
    Changed = true;
  }

  W->resetBBSet(); // Invalidate BBSet after transformations
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genCancellationBranchingCode\n");

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
      if (W->contains(User->getParent()) ||
          W->getExitBBlock() == User->getParent())
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

  PointerType *PtrTy = dyn_cast<PointerType>(CopyPrivateBase->getType());
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
