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
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

#include "llvm/PassAnalysisSupport.h"

#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"

#include "llvm/Transforms/Utils/Intel_GeneralUtils.h"
#include "llvm/Transforms/Utils/Intel_IntrinsicUtils.h"

#include <algorithm>
#include <set>

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

  if (NeedTID) {
    TidPtr = new AllocaInst(Int32Ty, "tid.addr", AI);
    TidPtr->setAlignment(4);

    CallInst *RI;
    RI = VPOParoptUtils::findKmpcGlobalThreadNumCall(&F->getEntryBlock());
    if (!RI) {
      RI = VPOParoptUtils::genKmpcGlobalThreadNumCall(F, AI, IdentTy);
      RI->insertBefore(AI);
    }
    StoreInst *Tmp0 = new StoreInst(RI, TidPtr, false, 
                                    F->getEntryBlock().getTerminator());
    Tmp0->setAlignment(4);
  }

  if (NeedBID && (Mode & OmpPar) && (Mode & ParTrans)) {
    BidPtr = new AllocaInst(Int32Ty, "bid.addr", AI);
    BidPtr->setAlignment(4);

    ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);
    StoreInst *Tmp1 = new StoreInst(ValueZero, BidPtr, false, 
                                    F->getEntryBlock().getTerminator());
    Tmp1->setAlignment(4);
  }

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

    if (W->getIsOmpLoop() && W->getLoop()==nullptr) {
      // The WRN is a loop-type construct, but the loop is missing, most likely
      // because it has been optimized away. We skip the code transforms for
      // this WRN, and simply remove its directives.
      RemoveDirectives = true;
    }
    else switch (W->getWRegionKindID()) {

      // 1. Constructs that need to perform outlining:
      //      Parallel [for|sections], task, taskloop, etc.

      case WRegionNode::WRNParallel:
      {
        DEBUG(dbgs() << "\n WRNParallel - Transformation \n\n");

        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          // Privatization is enabled for both Prepare and Transform passes
          Changed = genPrivatizationCode(W);
          Changed |= genFirstPrivatizationCode(W);
          Changed |= genReductionCode(W);
          Changed |= genMultiThreadedCode(W);
          RemoveDirectives = true;
        }
        break;
      }
      case WRegionNode::WRNParallelSections:
      case WRegionNode::WRNParallelLoop:
      {
        DEBUG(dbgs() << "\n WRNParallelLoop - Transformation \n\n");

        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          AllocaInst *IsLastVal = nullptr;
          Changed = genLoopSchedulingCode(W, IsLastVal);
          // Privatization is enabled for both Prepare and Transform passes
          Changed |= genPrivatizationCode(W);
          Changed |= genLastPrivatizationCode(W, IsLastVal);
          Changed |= genFirstPrivatizationCode(W);
          Changed |= genReductionCode(W);
          Changed |= genMultiThreadedCode(W);
          RemoveDirectives = true;
        }
        break;
      }
      case WRegionNode::WRNTask:
        // Task constructs need to perform outlining
        break;

      case WRegionNode::WRNTaskloop:
        DEBUG(dbgs() << "\n WRNTaskloop - Transformation \n\n");
        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          StructType *KmpTaskTTWithPrivatesTy;
          StructType *KmpSharedTy;
          Value *LBPtr, *UBPtr, *STPtr;
          Changed = genTaskLoopInitCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy,
                                        LBPtr, UBPtr, STPtr);
          Changed |= genPrivatizationCode(W);
          Changed |= genFirstPrivatizationCode(W);
          Changed |= genSharedCodeForTaskLoop(W);
          Changed |= genRedCodeForTaskLoop(W);
          Changed |= genTaskLoopCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy,
                                     LBPtr, UBPtr, STPtr);
          RemoveDirectives = true;
        }
        break;

      // 2. Constructs that do not need to perform outlining. E.g., simd,
      //    taskgroup, atomic, for, sections, etc.

      case WRegionNode::WRNTaskgroup:
        break;

      case WRegionNode::WRNVecLoop:
      {
        // Privatization is enabled for SIMD Transform passes
        DEBUG(dbgs() << "\n WRNSimdLoop - Transformation \n\n");

        if ((Mode & OmpVec) && (Mode & ParTrans)) {
           Changed = genPrivatizationCode(W);
           // keep SIMD directives; will be processed by the Vectorizer
           RemoveDirectives = false;
           RemovePrivateClauses = false;
        }
        break;
      }
      case WRegionNode::WRNAtomic:
      { 
        DEBUG(dbgs() << "\n WRNAtomic - Transformation \n\n");
        if (Mode & ParPrepare) {
          Changed = VPOParoptAtomics::handleAtomic(dyn_cast<WRNAtomicNode>(W),
                                                   IdentTy, TidPtr);
          RemoveDirectives = true;
        }
        break;
      }
      case WRegionNode::WRNSections:
      case WRegionNode::WRNWksLoop:
      {
        DEBUG(dbgs() << "\n WRNWksLoop - Transformation \n\n");

        if ((Mode & OmpPar) && (Mode & ParTrans)) {
          AllocaInst *IsLastVal = nullptr;
          Changed = genLoopSchedulingCode(W, IsLastVal);
          Changed |= genPrivatizationCode(W);
          Changed |= genLastPrivatizationCode(W, IsLastVal);
          Changed |= genFirstPrivatizationCode(W);
          Changed |= genReductionCode(W);
          RemoveDirectives = true;
        }

        break;
      }
      case WRegionNode::WRNSingle:
      { DEBUG(dbgs() << "\n WRNSingle - Transformation \n\n");
        if (Mode & ParPrepare) {
          Changed = genSingleThreadCode(W);
          RemoveDirectives = true;
        }
        break;
      }
      case WRegionNode::WRNMaster:
      { DEBUG(dbgs() << "\n WRNMaster - Transformation \n\n");
        if (Mode & ParPrepare) {
          Changed = genMasterThreadCode(W);
          RemoveDirectives = true;
        }
        break;
      }
      case WRegionNode::WRNCritical:
      {
        DEBUG(dbgs() << "\n WRNCritical - Transformation \n\n");
        if (Mode & ParPrepare) {
          Changed = genCriticalCode(dyn_cast<WRNCriticalNode>(W));
          RemoveDirectives = true;
        }
        break;
      } 
      case WRegionNode::WRNOrdered:
      {
        DEBUG(dbgs() << "\n WRNOrdered - Transformation \n\n");
        if (Mode & ParPrepare) {
          Changed = genOrderedThreadCode(W);
          RemoveDirectives = true;
        }
        break;
      }
      case WRegionNode::WRNBarrier:
      case WRegionNode::WRNCancel:
      case WRegionNode::WRNFlush:
        break;
      default: break;
    }

    // Remove calls to directive intrinsics since the LLVM back end does not
    // know how to translate them.
    if (RemoveDirectives) {
      bool DirRemoved = VPOUtils::stripDirectives(W);
      assert(DirRemoved && "Directive intrinsics not removed for WRN.\n");
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
  case ReductionItem::WRNReductionSum:
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
  PN->addIncoming(ConstantInt::getFalse(C), EntryBB);
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
  case ReductionItem::WRNReductionSum:
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
  if (!AllocaTy->isSingleValueType() ||
      !DL.isLegalInteger(DL.getTypeSizeInBits(ScalarTy)) ||
      DL.getTypeSizeInBits(ScalarTy) % 8 != 0)
    VPOParoptUtils::genMemcpy(AI, FprivI->getOrig(), DL, AI->getAlignment(),
                              InsertPt->getParent());
  else {
    LoadInst *Load = Builder.CreateLoad(FprivI->getOrig());
    Builder.CreateStore(Load, AI);
  }
}

// Generate the lastprivate update code.
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
void VPOParoptTransform::genLprivFini(LastprivateItem *LprivI,
                                      Instruction *InsertPt) {
  AllocaInst *AI = dyn_cast<AllocaInst>(LprivI->getNew());
  Type *AllocaTy = AI->getAllocatedType();
  Type *ScalarTy = AllocaTy->getScalarType();
  const DataLayout &DL = InsertPt->getModule()->getDataLayout();

  IRBuilder<> Builder(InsertPt);
  if (!AllocaTy->isSingleValueType() ||
      !DL.isLegalInteger(DL.getTypeSizeInBits(ScalarTy)) ||
      DL.getTypeSizeInBits(ScalarTy) % 8 != 0) {
    VPOParoptUtils::genMemcpy(LprivI->getOrig(), AI, DL, AI->getAlignment(),
                              InsertPt->getParent());
  } else {
    LoadInst *Load = Builder.CreateLoad(AI);
    Builder.CreateStore(Load, LprivI->getOrig());
  }
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
  BasicBlock *PrivExitBB = SplitBlock(
      ExitBlock, dyn_cast<Instruction>(&*ExitBlock->begin()), DT, LI);
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
    VPOParoptUtils::genKmpcCriticalSection(
        W, IdentTy, TidPtr, dyn_cast<Instruction>(&*RedUpdateEntryBB->begin()),
        dyn_cast<Instruction>(&*W->getExitBBlock()->begin()), "");
    W->resetBBSet(); // Invalidate BBSet after transformations
    Changed = true;
  }
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genReductionCode\n");
  return Changed;
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
    NewPrivInst = new AllocaInst(ElemTy, nullptr, GV->getName());
    NewPrivInst->insertBefore(InsertPt);
    SmallVector<Instruction *, 8> RewriteCons;
    for (auto IB = GV->user_begin(), IE = GV->user_end(); IB != IE; ++IB) {
      if (ConstantExpr *CE = dyn_cast<ConstantExpr>(*IB))
        for (Use &U : CE->uses()) {
          User *UR = U.getUser();
          Instruction *I = dyn_cast<Instruction>(UR);
          if (W->contains(I->getParent()))
            RewriteCons.push_back(I);
        }
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

// Annotate the alias scope data for the references of the
// firstprivate, lastprivate, private shared, reduction variables
void VPOParoptTransform::annotateInstWithNoAlias(Instruction *ItemInst,
                                                 Item *IT) {
  LLVMContext &Context = F->getContext();
  if (IT->getAliasScope())
    ItemInst->setMetadata(
        LLVMContext::MD_alias_scope,
        MDNode::concatenate(ItemInst->getMetadata(LLVMContext::MD_alias_scope),
                            MDNode::get(Context, IT->getAliasScope())));

  if (IT->getNoAlias())
    ItemInst->setMetadata(
        LLVMContext::MD_noalias,
        MDNode::concatenate(ItemInst->getMetadata(LLVMContext::MD_noalias),
                            IT->getNoAlias()));
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
    if (VPOAnalysisUtils::isIntelDirectiveOrClause(UI)) {
      LLVMContext &C = F->getContext();
      // The operand in the clause needs to be replaced to maintain the SSA
      // version consistency.
      UI->replaceUsesOfWith(PrivValue,
                            ConstantPointerNull::get(Type::getInt8PtrTy(C)));
      continue;
    }
    UI->replaceUsesOfWith(PrivValue, NewPrivInst);
    if (isa<LoadInst>(UI) || isa<StoreInst>(UI))
      annotateInstWithNoAlias(UI, IT);
    // DEBUG(dbgs() << "New Instruction uses PrivItem: " << *UI << "\n");
  }
}

bool VPOParoptTransform::genFirstPrivatizationCode(WRegionNode *W) {

  bool Changed = false;

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genFirstPrivatizationCode\n");

  assert(W->isBBSetEmpty() &&
         "genFirstPrivatizationCode: BBSET should start empty");

  assert(W->hasFirstprivate() &&
         "genFirstPrivatizationCode: WRN doesn't take a firstprivate var");

  FirstprivateClause &FprivClause = W->getFpriv();
  if (!FprivClause.empty()) {
    W->populateBBSet();
    BasicBlock *EntryBB = W->getEntryBBlock();
    BasicBlock *PrivInitEntryBB = nullptr;
    Value *NewPrivInst = nullptr;
    bool ForTaskLoop = W->getWRegionKindID() == WRegionNode::WRNTaskloop;

    for (FirstprivateItem *FprivI : FprivClause.items()) {
      Value *Orig = FprivI->getOrig();
/*
      assert((isa<GlobalVariable>(Orig) || isa<AllocaInst>(Orig)) &&
             "genFirstPrivatizationCode: Unexpected firstprivate variable");
*/
      if (W->hasLastprivate()) {
        LastprivateClause &LprivClause = W->getLpriv();
        auto LprivI = LprivClause.findOrig(Orig);
        if (!LprivI) {
          if (!ForTaskLoop)
            NewPrivInst = genPrivatizationAlloca(W, Orig, &EntryBB->front(),
                                                 ".fpriv");
          else
            NewPrivInst = FprivI->getNew();

          genPrivatizationReplacement(W, Orig, NewPrivInst, FprivI);
          FprivI->setNew(NewPrivInst);
        } else
          FprivI->setNew(LprivI->getNew());
      } else {
        if (!ForTaskLoop)
          NewPrivInst =
              genPrivatizationAlloca(W, Orig, &EntryBB->front(), ".fpriv");
        else
          NewPrivInst = FprivI->getNew();
        genPrivatizationReplacement(W, Orig, NewPrivInst, FprivI);
        FprivI->setNew(NewPrivInst);
      }

      if (!ForTaskLoop) {
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
                                                  AllocaInst *IsLastVal) {
  bool Changed = false;

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genLastPrivatizationCode\n");

  assert(W->isBBSetEmpty() &&
         "genLastPrivatizationCode: BBSET should start empty");

  assert(W->hasLastprivate() &&
         "genLastPrivatizationCode: WRN doesn't take a lastprivate var");

  LastprivateClause &LprivClause = W->getLpriv();
  if (!LprivClause.empty()) {
    W->populateBBSet();
    bool ForTaskLoop = W->getWRegionKindID() == WRegionNode::WRNTaskloop;
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
      if (!ForTaskLoop)
        NewPrivInst =
            genPrivatizationAlloca(W, Orig, &EntryBB->front(), ".lpriv");
      else
        NewPrivInst = LprivI->getNew();
      genPrivatizationReplacement(W, Orig, NewPrivInst, LprivI);
      LprivI->setNew(NewPrivInst);
      genLprivFini(LprivI, BeginBB->getTerminator());
    }
    Changed = true;
    W->resetBBSet(); // Invalidate BBSet
  }

  DEBUG(dbgs() << "\nExit VPOParoptTransform::genLastPrivatizationCode\n");
  return Changed;
}

bool VPOParoptTransform::genPrivatizationCode(WRegionNode *W) {

  bool Changed = false;

  BasicBlock *EntryBB = W->getEntryBBlock();

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genPrivatizationCode\n");

  // Process all PrivateItems in the private clause
  PrivateClause &PrivClause = W->getPriv();
  if (!PrivClause.empty()) {

    assert(W->isBBSetEmpty() &&
           "genPrivatizationCode: BBSET should start empty");
    W->populateBBSet();

    bool ForTaskLoop = W->getWRegionKindID() == WRegionNode::WRNTaskloop;

    // Walk through each PrivateItem list in the private clause to perform 
    // privatization for each Value item
    for (PrivateItem *PrivI : PrivClause.items()) {
      Value *Orig = PrivI->getOrig();

      if (isa<GlobalVariable>(Orig) || isa<AllocaInst>(Orig)) {
        Value *NewPrivInst = nullptr;
        if (!ForTaskLoop) {
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
          Instruction *AllocaInsertPt = &EntryBB->front();

          NewPrivInst =
              genPrivatizationAlloca(W, Orig, AllocaInsertPt, ".priv");
        } else
          NewPrivInst = PrivI->getNew();
        genPrivatizationReplacement(W, Orig, NewPrivInst, PrivI);

        PrivI->setNew(NewPrivInst);
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
        Loop *L = W->getLoop();
        SE->forgetLoop(L);
    }
  }
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genPrivatizationCode\n");
  return Changed;
}

bool VPOParoptTransform::genLoopSchedulingCode(WRegionNode *W, 
                                               AllocaInst *&IsLastVal) {
  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genLoopSchedulingCode\n");

  assert(W->getIsOmpLoop() && "genLoopSchedulingCode: not a loop-type WRN");

  Loop *L = W->getLoop();

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

  // 
  // This is initial implementation of parallel loop scheduling to get 
  // a simple loop to work end-to-end.
  // 
  // TBD: handle all loop forms: Top test loop, bottom test loop, with 
  // PHI and without PHI nodes as SCEV bails out for many cases
  //
  LLVMContext &C = F->getContext();
  IntegerType *Int32Ty = Type::getInt32Ty(C);

  Type *LoopIndexType =
          WRegionUtils::getOmpCanonicalInductionVariable(L)->
          getIncomingValue(0)->getType();

  IntegerType *IndValTy = cast<IntegerType>(LoopIndexType);
  assert(IndValTy->getIntegerBitWidth() >= 32 &&
         "Omp loop index type width is equal or greater than 32 bit");

  Value *InitVal = WRegionUtils::getOmpLoopLowerBound(L);

  Instruction *InsertPt = dyn_cast<Instruction>(
    L->getLoopPreheader()->getTerminator());

  LoadInst *LoadTid = new LoadInst(TidPtr, "my.tid", InsertPt);
  LoadTid->setAlignment(4);

  IsLastVal =
      new AllocaInst(Int32Ty, "is.last", W->getEntryBBlock()->getTerminator());
  IsLastVal->setAlignment(4);
  AllocaInst *LowerBnd = new AllocaInst(IndValTy, "lower.bnd", InsertPt);
  LowerBnd->setAlignment(4);

  AllocaInst *UpperBnd = new AllocaInst(IndValTy, "upper.bnd", InsertPt);
  UpperBnd->setAlignment(4);

  AllocaInst *Stride = new AllocaInst(IndValTy, "stride", InsertPt);
  Stride->setAlignment(4);

  // UpperD is for distribute loop
  AllocaInst *UpperD = new AllocaInst(IndValTy, "upperD", InsertPt);
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

  StoreInst *Tmp4 = new StoreInst(ValueZero, IsLastVal, false, InsertPt);
  Tmp4->setAlignment(4);

  ICmpInst* LoopBottomTest = WRegionUtils::getOmpLoopBottomTest(L);

  bool IsUnsigned = LoopBottomTest->isUnsigned();
  int Size = LowerBnd->getType()
                     ->getPointerElementType()->getIntegerBitWidth();

  CallInst* KmpcInitCI;
  CallInst* KmpcFiniCI;
  CallInst* KmpcNextCI;

  Value *ChunkVal = (SchedKind == WRNScheduleStaticEven) ? 
                                  ValueOne : W->getSchedule().getChunkExpr();

  DEBUG(dbgs() << "--- Schedule Chunk Value: " << *ChunkVal << "\n\n");

  if (SchedKind == WRNScheduleStaticEven || SchedKind == WRNScheduleStatic) { 
    // Generate __kmpc__for_static_init_4{u}/8{u} Call Instruction
    KmpcInitCI = VPOParoptUtils::genKmpcStaticInit(W, IdentTy,
                               LoadTid, SchedType, IsLastVal, LowerBnd, 
                               UpperBnd, Stride, StrideVal, ValueOne, 
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
  PN->removeIncomingValue(L->getLoopPreheader());
  PN->addIncoming(LoadLB, L->getLoopPreheader());

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

  TerminatorInst *NewTermInst = BranchInst::Create(PreHdrInst->getSuccessor(0),
                                                   LoopExitBB, CompInst);
  ReplaceInstWithInst(InsertPt, NewTermInst);

  BasicBlock *LoopRegionExitBB = nullptr;

  if (LoopExitBB != W->getExitBBlock()) {
    InsertPt = dyn_cast<Instruction>(&*LoopExitBB->rbegin());
  }
  else {
    InsertPt = dyn_cast<Instruction>(&*LoopExitBB->begin());
    LoopRegionExitBB = SplitBlock(LoopExitBB, InsertPt, DT, LI);
    LoopRegionExitBB->setName("loop.region.exit");

    if (DT) 
      DT->changeImmediateDominator(LoopRegionExitBB, LoopExitBB);

    // After split LoopExitBB block, InsertPt is null, so we get
    // branch instruction
    InsertPt = dyn_cast<Instruction>(&*LoopExitBB->rbegin());

    W->setExitBBlock(LoopRegionExitBB);
  }

  if (SchedKind == WRNScheduleStaticEven) {

    BasicBlock *StaticInitBB = KmpcInitCI->getParent();

    KmpcFiniCI = VPOParoptUtils::genKmpcStaticFini(W, 
                                        IdentTy, LoadTid, InsertPt);
    KmpcFiniCI->setCallingConv(CallingConv::C);

    if (DT) 
      DT->changeImmediateDominator(LoopExitBB, StaticInitBB);
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

// Collects the users instructions for the instructions 
// in the set TidAndBidInstructions.
void VPOParoptTransform::collectInstructionUsesInRegion(WRegionNode *W) {
  for (Instruction *I : TidAndBidInstructions) 
    for (auto IB = I->user_begin(), IE = I->user_end();
         IB != IE; IB++) {
      if (Instruction *User = dyn_cast<Instruction>(*IB)) {
        if (W->contains(User->getParent()))
          IdMap[I].push_back(User);
      }
      else 
        llvm_unreachable("Expect the user is instruction.");
    }
}

// Collects the bid alloca instructions used by the outline functions.
void VPOParoptTransform::collectTidAndBidInstructionsForBB(BasicBlock *BB) {
  for (auto &I : *BB) {
    if (auto *CI = dyn_cast<CallInst>(&I)) {
      if (CI->hasFnAttr("mt-func")) {
        AllocaInst *AI = dyn_cast<AllocaInst>(CI->getArgOperand(1));
        assert(AI && "Expect alloca instruction for bid.");
        TidAndBidInstructions.insert(AI);
      }
    }
  }
}

// Generates the new tid/bid alloca instructions at the entry of the
// region and replaces the uses of tid/bid with the new value.
void VPOParoptTransform::codeExtractorPrepareTransform(WRegionNode *W,
                                                       bool IsTid) {
  AllocaInst *NewAI = nullptr;
  CallInst *NewTid = nullptr;
  BasicBlock *WREntryBB = W->getEntryBBlock();
  Instruction *InsertBefore = nullptr;

  for (Instruction &I : *WREntryBB) {
    if (!VPOAnalysisUtils::isIntelDirectiveOrClause(&I)) {
      InsertBefore = &I;
      break;
    }
  }

  for (auto &I : IdMap) {
    for (auto &J : I.second) {
      if (IsTid && NewTid == nullptr) {
        NewTid = VPOParoptUtils::genKmpcGlobalThreadNumCall(F, 
                                &*(WREntryBB->getFirstInsertionPt()),
                                nullptr);
        NewTid->insertBefore(InsertBefore);
      }
      if (!NewAI) {
        if (isa<AllocaInst>(I.first)) {
          NewAI = new AllocaInst(Type::getInt32Ty(F->getContext()),
                                 IsTid ? "new.tid.addr" : "new.bid.addr",
                                 InsertBefore);
          NewAI->setAlignment(4);

          ConstantInt *ValueZero;
          if (IsTid == false) 
            ValueZero = ConstantInt::get(Type::getInt32Ty(F->getContext()), 0);

          Value *StValue;
          if (IsTid) 
            StValue = NewTid;
          else
            StValue = ValueZero;
          new StoreInst(StValue, NewAI, false, InsertBefore);
        }
      }
      if (isa<AllocaInst>(I.first))
        J->replaceUsesOfWith(I.first, NewAI);
      else if (IsTid)
        J->replaceUsesOfWith(I.first, NewTid);
    }
  }
}

// Generates the tid call instruction at the entry of WRegion
// if the IR in the WRegion uses the tid value at the entry of
// the function. It also generates the bid alloca instruction in 
// the region if the region has outlined function.
// Here is one example compiled at -O3.
//
// Before:
// entry:
//   %tid.addr = alloca i32, align 4
//   %tid.val = tail call i32 
//    @__kmpc_global_thread_num({ i32, i32, i32, i32, i8* }* @.kmpc_loc.0.0)
// 
// for.body3:
//   %j.0 = phi i32 [ %add12, %for.body3 ], [ %add, %for.body3.preheader ]
//     call void @__kmpc_critical({ i32, i32, i32, i32, i8* }* 
//      @.kmpc_loc.0.0.2, i32 %tid.val, [8 x i32]* @.gomp_critical_user_.var)
//
// After:
// entry:
//   %tid.addr = alloca i32, align 4
//   %tid.val = tail call i32 
//    @__kmpc_global_thread_num({ i32, i32, i32, i32, i8* }* @.kmpc_loc.0.0)
//
// DIR.OMP.PARALLEL.1:
//   call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL")
//   call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
//   %tid.val38 = tail call i32 @__kmpc_global_thread_num(
//    { i32, i32, i32, i32, i8* }* @.kmpc_loc.0.0.6)
//   br label %DIR.QUAL.LIST.END.2
//
// for.body3:
//   %j.0 = phi i32 [ %add12, %for.body3 ], [ %add, %for.body3.preheader ]
//   call void @__kmpc_critical({ i32, i32, i32, i32, i8* }* 
//    @.kmpc_loc.0.0.2, i32 %tid.val38, [8 x i32]* @.gomp_critical_user_.var)
//
// Here is another example compiled at -O0.
//
// Before:
// entry:
//   %tid.addr23 = alloca i32, align 4
//   %tid.addr = alloca i32, align 4
//   %tid.val = tail call i32 @__kmpc_global_thread_num(
//     { i32, i32, i32, i32, i8* }* @.kmpc_loc.0.0)
//   store i32 %tid.val, i32* %tid.addr, align 4
//   store i32 %tid.val, i32* %tid.addr23, align 4
//
// for.body3:
//   %my.tid = load i32, i32* %tid.addr, align 4
//   call void @__kmpc_critical({ i32, i32, i32, i32, i8* }* 
//    @.kmpc_loc.0.0.2, i32 %my.tid, [8 x i32]* @.gomp_critical_user_.var)
//
// After:
//
// for.body:
//   call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL")
//   call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
//   %tid.val24 = tail call i32 @__kmpc_global_thread_num(
//     { i32, i32, i32, i32, i8* }* @.kmpc_loc.0.0.6)
//   %new.tid.addr = alloca i32, align 4
//   store i32 %tid.val24, i32* %new.tid.addr
//   br label %DIR.QUAL.LIST.END.221
//
// for.body3: 
//   %my.tid = load i32, i32* %new.tid.addr, align 4
//   call void @__kmpc_critical({ i32, i32, i32, i32, i8* }* 
//     @.kmpc_loc.0.0.2, i32 %my.tid, [8 x i32]* @.gomp_critical_user_.var)
//   br label %DIR.QUAL.LIST.END.5
// 
void VPOParoptTransform::codeExtractorPrepare(WRegionNode *W) {
  // Generates the bid alloca instruction in the region 
  // if the region has outlined function.
  TidAndBidInstructions.clear();
  IdMap.clear();

  assert(!(W->isBBSetEmpty()) && "codeExtractorPrepare: WRN has empty BBSet");

  for (auto IB = W->bbset_begin(); IB != W->bbset_end(); IB++) 
    collectTidAndBidInstructionsForBB(*IB);

  collectInstructionUsesInRegion(W);

  // The flag false indicates it is for bid case. Otherwise
  // it is for tid case.
  codeExtractorPrepareTransform(W, false);

  // Generates the tid call instruction at the entry of WRegion
  // if the IR in the WRegion uses the tid value at the entry of
  // the function.
  Function *F = W->getEntryBBlock()->getParent();
  BasicBlock *EntryBB = &F->getEntryBlock();
  CallInst *Tid = VPOParoptUtils::findKmpcGlobalThreadNumCall(EntryBB);
  if (!Tid) return;

  BasicBlock *WREntryBB = W->getEntryBBlock();
  CallInst *NewTid = VPOParoptUtils::findKmpcGlobalThreadNumCall(WREntryBB);
  if (NewTid) return;

  TidAndBidInstructions.clear();
  IdMap.clear();

  getAllocFromTid(Tid);
  TidAndBidInstructions.insert(Tid);

  collectInstructionUsesInRegion(W);
  // The flag false indicates it is for bid case. Otherwise
  // it is for tid case.
  codeExtractorPrepareTransform(W, true);

}

// Cleans up the generated __kmpc_global_thread_num() in the
// outlined function. Replaces the use of the __kmpc_global_thread_num()
// with the first argument of the outlined function.
// 
// Here is one example compiled at -O3.
// 
// Before:
// DIR.OMP.PARALLEL.1:                               ; preds = %newFuncRoot
//   call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL")
//   call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
//   %tid.val38 = tail call i32 @__kmpc_global_thread_num(
//     { i32, i32, i32, i32, i8* }* @.kmpc_loc.0.0.6)
//   br label %DIR.QUAL.LIST.END.2
//
// for.body3:
//   %j.0 = phi i32 [ %add12, %for.body3 ], [ %add, %for.body3.preheader ]
//   call void @__kmpc_critical({ i32, i32, i32, i32, i8* }* 
//     @.kmpc_loc.0.0.2, i32 %tid.val38, [8 x i32]* @.gomp_critical_user_.var)
//
//  After:
//  DIR.OMP.PARALLEL.1:                               ; preds = %newFuncRoot
//    %0 = load i32, i32* %tid
//    call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL")
//    call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
//    br label %DIR.QUAL.LIST.END.2
//
//  for.body3:
//   %j.0 = phi i32 [ %add12, %for.body3 ], [ %add, %for.body3.preheader ]
//   call void @__kmpc_critical({ i32, i32, i32, i32, i8* }* 
//     @.kmpc_loc.0.0.2, i32 %0, [8 x i32]* @.gomp_critical_user_.var)
//
// Another example is compiled at -O0.
//
// Before:
// for.body:                                         ; preds = %newFuncRoot
//   call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL")
//   call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
//   %tid.val24 = tail call i32 @__kmpc_global_thread_num(
//     { i32, i32, i32, i32, i8* }* @.kmpc_loc.0.0.6)
//   %new.tid.addr = alloca i32, align 4
//   store i32 %tid.val24, i32* %new.tid.addr
//
// for.body3:
//   %my.tid = load i32, i32* %new.tid.addr, align 4
//   call void @__kmpc_critical({ i32, i32, i32, i32, i8* }* 
//     @.kmpc_loc.0.0.2, i32 %my.tid, [8 x i32]* @.gomp_critical_user_.var)
//
// After:
//
// for.body3:
//   %my.tid = load i32, i32* %tid, align 4
//   call void @__kmpc_critical({ i32, i32, i32, i32, i8* }* 
//     @.kmpc_loc.0.0.2, i32 %my.tid, [8 x i32]* @.gomp_critical_user_.var)
//   br label %DIR.QUAL.LIST.END.5
//
void VPOParoptTransform::finiCodeExtractorPrepare(Function *F,
                                                  bool ForTaskLoop) {
  if (ForTaskLoop == false) {
    // Cleans the genererated bid alloca instruction in the
    // outline function.
    TidAndBidInstructions.clear();

    for (Function::iterator B = F->begin(), Be = F->end(); B != Be; ++B)
      collectTidAndBidInstructionsForBB(&*B);

    finiCodeExtractorPrepareTransform(F, false, nullptr, ForTaskLoop);
  }
  // Cleans up the generated __kmpc_global_thread_num() in the
  // outlined function. Replaces the use of the __kmpc_global_thread_num()
  // with the first argument of the outlined function.
  CallInst *Tid = nullptr;
  BasicBlock *NextBB = &F->getEntryBlock();
  do {
    Tid = VPOParoptUtils::findKmpcGlobalThreadNumCall(NextBB);
    if (Tid)
      break;
    if (std::distance(succ_begin(NextBB), succ_end(NextBB)) == 1)
      NextBB = *(succ_begin(NextBB));
    else
      break;
    if (std::distance(pred_begin(NextBB), pred_end(NextBB)) != 1)
      break;
  } while (!Tid);

  if (!Tid)
    return;

  TidAndBidInstructions.clear();

  if (ForTaskLoop == false)
    getAllocFromTid(Tid);
  TidAndBidInstructions.insert(Tid);

  finiCodeExtractorPrepareTransform(F, true, NextBB, ForTaskLoop);
}

// Replaces the use of tid/bid with the outlined function arguments.
void VPOParoptTransform::finiCodeExtractorPrepareTransform(Function *F,
                                                           bool IsTid,
                                                           BasicBlock *NextBB,
                                                           bool ForTaskLoop) {
  Value *NewAI = nullptr;
  Value *NewLoad = nullptr;
  for (Instruction *I : TidAndBidInstructions) {
    if (isa<AllocaInst>(I)) {
      if(!NewAI) {
        auto IT = F->arg_begin();
        if (!IsTid)
          IT++;
        NewAI = &*(IT);
      }
    } else if (IsTid) {
      if (ForTaskLoop == false) {
        IRBuilder<> Builder(NextBB);
        Builder.SetInsertPoint(NextBB, NextBB->getFirstInsertionPt());
        NewLoad = Builder.CreateLoad(&*(F->arg_begin()));
      } else {
        auto IT = F->arg_begin();
        NewLoad = &*(IT);
      }
    }
    if (isa<AllocaInst>(I))
      I->replaceAllUsesWith(NewAI);
    else if (IsTid)
      I->replaceAllUsesWith(NewLoad);
  }
  
  for (Instruction *I : TidAndBidInstructions) 
    I->eraseFromParent();
}

bool VPOParoptTransform::genMultiThreadedCode(WRegionNode *W) {
  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genMultiThreadedCode\n");
  assert(W->isBBSetEmpty() &&
         "genMultiThreadedCode: BBSET should start empty");

  W->populateBBSet();

  codeExtractorPrepare(W);

  bool Changed = false;

  // brief extract a W-Region to generate a function
  CodeExtractor CE(makeArrayRef(W->bbset_begin(), W->bbset_end()), DT, false);

  assert(CE.isEligible());

  // Set up Fn Attr for the new function
  if (Function *NewF = CE.extractCodeRegion()) {

    // Set up the Calling Convention used by OpenMP Runtime Library
    CallingConv::ID CC = CallingConv::C;

    DT->verifyDomTree();

    // Adjust the calling convention for both the function and the
    // call site.
    NewF->setCallingConv(CC);

    assert(NewF->hasOneUse() && "New function should have one use");
    User *U = NewF->user_back();

    CallInst *NewCall = cast<CallInst>(U);
    NewCall->setCallingConv(CC);

    CallSite CS(NewCall);

    unsigned int TidArgNo = 0;
    bool IsTidArg = false;

    for (auto I = CS.arg_begin(), E = CS.arg_end(); I != E; ++I) {
      if (*I == TidPtr)  {
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
    MTFnArgs.push_back(TidPtr);
    MTFnArgs.push_back(BidPtr);
    genThreadedEntryActualParmList(W, MTFnArgs);

    finiCodeExtractorPrepare(MTFn);

    DEBUG(dbgs() << " New Call to MTFn: " << *NewCall << "\n"); 
    // Pass all the same arguments of the extracted function.
    for (auto I = CS.arg_begin(), E = CS.arg_end(); I != E; ++I) {
      if (*I != TidPtr)  {
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

    ConstantInt *ValueOne = ConstantInt::get(Type::getInt32Ty(C), 1);

    TerminatorInst *TermInst = ForkTestBB->getTerminator();

    ICmpInst* CondInst = new ICmpInst(TermInst, ICmpInst::ICMP_EQ, 
                                      ForkTestCI, ValueOne, "");

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
      LoadInst *Tid = new LoadInst(TidPtr, "my.tid", ForkCI);
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
  
  Function *ForkCallFn = M->getFunction("__kmpc_fork_call");

  if (!ForkCallFn) {
    ForkCallFn = Function::Create(FnTy, GlobalValue::ExternalLinkage,
                                  "__kmpc_fork_call", M);
    ForkCallFn->setCallingConv(CallingConv::C);
  }

  AttributeSet ForkCallFnAttr;
  SmallVector<AttributeSet, 4> Attrs;

  AttributeSet FnAttrSet;
  AttrBuilder B;
  FnAttrSet = AttributeSet::get(C, ~0U, B);

  Attrs.push_back(FnAttrSet);
  ForkCallFnAttr = AttributeSet::get(C, Attrs);

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
void VPOParoptTransform::genThreadedEntryActualParmList(WRegionNode *W,
                                          std::vector<Value *>& MTFnArgs) {
  if (!W->hasCopyin())
    return;
  CopyinClause &CP = W->getCopyin();
  for (auto C : CP.items())
    MTFnArgs.push_back(C->getOrig());
}

// Generates the formal parameters in the outlined function for 
// copyin variables. It can be extended for other variables including
// firstprivate, shared, etc. 
void VPOParoptTransform::genThreadedEntryFormalParmList(WRegionNode *W,
                                          std::vector<Type *>& ParamsTy) {
  if (!W->hasCopyin())
    return;
  CopyinClause &CP = W->getCopyin();
  for (auto C : CP.items()) 
    ParamsTy.push_back(C->getOrig()->getType());
}

// Fix the name of copyin formal parameters for outlined function.
void VPOParoptTransform::fixThreadedEntryFormalParmName(WRegionNode *W,
                                                        Function *NFn) {
  if (!W->hasCopyin())
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
  if (!W->hasCopyin())
    return;
  CopyinClause &CP = W->getCopyin();
  if (!CP.empty()) {
    Function::arg_iterator NewArgI = NFn->arg_begin();
    ++NewArgI;
    ++NewArgI;
    const DataLayout DL=NFn->getParent()->getDataLayout();
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
        Value *TpvArg = 
                 Builder.CreatePtrToInt(&*NewArgI,Builder.getIntPtrTy(DL));
        Value *OldTpv = 
                 Builder.CreatePtrToInt(C->getOrig(),Builder.getIntPtrTy(DL));

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
        NFn->getEntryBlock().getTerminator()
            ->getSuccessor(1)->setName("copyin.not.master.end");
      }
      VPOParoptUtils::genMemcpy(
          C->getOrig(), &*NewArgI, DL,
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

  Instruction *InsertPt = dyn_cast<Instruction>(&*EntryBB->rbegin());

  // Generate __kmpc_master Call Instruction
  CallInst* MasterCI = VPOParoptUtils::genKmpcMasterOrEndMasterCall(W, 
                         IdentTy, TidPtr, InsertPt, true);
  MasterCI->insertBefore(InsertPt);

  //DEBUG(dbgs() << " MasterCI: " << *MasterCI << "\n\n");

  Instruction *InsertEndPt = dyn_cast<Instruction>(&*ExitBB->rbegin());

  // Generate __kmpc_end_master Call Instruction
  CallInst* EndMasterCI = VPOParoptUtils::genKmpcMasterOrEndMasterCall(W, 
                            IdentTy, TidPtr, InsertEndPt, false);
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
bool VPOParoptTransform::genSingleThreadCode(WRegionNode *W) {
  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genSingleThreadCode\n");
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *ExitBB = W->getExitBBlock();

  Instruction *InsertPt = dyn_cast<Instruction>(&*EntryBB->rbegin());

  // Generate __kmpc_single Call Instruction
  CallInst* SingleCI = VPOParoptUtils::genKmpcSingleOrEndSingleCall(W,
                         IdentTy, TidPtr, InsertPt, true);
  SingleCI->insertBefore(InsertPt);

  Instruction *InsertEndPt = dyn_cast<Instruction>(&*ExitBB->rbegin());

  // Generate __kmpc_end_single Call Instruction
  CallInst* EndSingleCI = VPOParoptUtils::genKmpcSingleOrEndSingleCall(W,
                            IdentTy, TidPtr, InsertEndPt, false);
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

  Instruction *InsertPt = dyn_cast<Instruction>(&*EntryBB->rbegin());

  // Generate __kmpc_ordered Call Instruction
  CallInst* OrderedCI = VPOParoptUtils::genKmpcOrderedOrEndOrderedCall(W,
                          IdentTy, TidPtr, InsertPt, true);
  OrderedCI->insertBefore(InsertPt);

  Instruction *InsertEndPt = dyn_cast<Instruction>(&*ExitBB->rbegin());

  // Generate __kmpc_end_ordered Call Instruction
  CallInst* EndOrderedCI = VPOParoptUtils::genKmpcOrderedOrEndOrderedCall(W,
                             IdentTy, TidPtr, InsertEndPt, false);
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
  assert(TidPtr != nullptr && "TidPtr is null.");

  assert(CriticalNode->isBBSetEmpty() &&
         "genCriticalCode: BBSET should start empty");

  // genKmpcCriticalSection() needs BBSet for error checking only;
  // In the future consider getting rid of this call to populateBBSet.
  CriticalNode->populateBBSet();

  StringRef LockNameSuffix = CriticalNode->getUserLockName();

  bool CriticalCallsInserted =
      LockNameSuffix.empty()
          ? VPOParoptUtils::genKmpcCriticalSection(CriticalNode, IdentTy,
                                                   TidPtr)
          : VPOParoptUtils::genKmpcCriticalSection(CriticalNode, IdentTy,
                                                   TidPtr, LockNameSuffix);

  DEBUG(dbgs() << __FUNCTION__ << ": Handling of Critical Node: "
               << (CriticalCallsInserted ? "Successful" : "Failed") << ".\n");

  assert(CriticalCallsInserted && "Failed to create critical section. \n");

  CriticalNode->resetBBSet(); // Invalidate BBSet
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genCriticalCode\n");
  return CriticalCallsInserted;
}
