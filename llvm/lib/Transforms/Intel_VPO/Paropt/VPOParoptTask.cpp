//===- VPOParoptTask.cpp - Transformation of W-Region for threading --===//
//
// Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation. and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// VPOParoptTask.cpp implements the omp taskloop feature.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"

#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"

#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

#include "llvm/PassAnalysisSupport.h"

#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"

#include "llvm/Transforms/Utils/Intel_GeneralUtils.h"
#include "llvm/Transforms/Utils/Intel_IntrinsicUtils.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-task"

// Replace the reduction variable reference with the dereference of
// the return pointer __kmpc_task_reduction_get_th_data
bool VPOParoptTransform::genRedCodeForTaskLoop(WRegionNode *W) {

  bool Changed = false;

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genRedCodeForTaskLoop\n");

  ReductionClause &RedClause = W->getRed();
  if (!RedClause.empty()) {

    assert(W->isBBSetEmpty() &&
           "genRedCodeForTaskLoop: BBSET should start empty");
    W->populateBBSet();
    BasicBlock *EntryBB = W->getEntryBBlock();
    BasicBlock *ExitBB = W->getExitBBlock();

    for (ReductionItem *RedI : RedClause.items()) {

      Value *Orig = RedI->getOrig();

      if (isa<GlobalVariable>(Orig) || isa<AllocaInst>(Orig)) {
        Instruction *AllocaInsertPt = EntryBB->getFirstNonPHI();
        Value *NewPrivInst =
            genPrivatizationAlloca(W, Orig, AllocaInsertPt, ".red");
        genPrivatizationReplacement(W, Orig, NewPrivInst, RedI);
        IRBuilder<> Builder(EntryBB->getTerminator());
        Builder.CreateStore(Builder.CreateLoad(RedI->getNew()), NewPrivInst);
        Builder.SetInsertPoint(ExitBB->getTerminator());
        Builder.CreateStore(Builder.CreateLoad(NewPrivInst), RedI->getNew());
      }
    }

    Changed = true;
    W->resetBBSet(); // Invalidate BBSet after transformations
  }
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genRedCodeForTaskLoop\n");
  return Changed;
}

// Replace the shared variable reference with the thunk field
// derefernce
bool VPOParoptTransform::genSharedCodeForTaskLoop(WRegionNode *W) {

  bool Changed = false;

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genSharedCodeForTaskLoop\n");

  SharedClause &ShaClause = W->getShared();
  if (!ShaClause.empty()) {

    assert(W->isBBSetEmpty() &&
           "genSharedCodeForTaskLoop: BBSET should start empty");
    W->populateBBSet();

    for (SharedItem *ShaI : ShaClause.items()) {

      Value *Orig = ShaI->getOrig();

      if (isa<GlobalVariable>(Orig) || isa<AllocaInst>(Orig)) {
        Value *NewPrivInst = nullptr;
        NewPrivInst = ShaI->getNew();
        genPrivatizationReplacement(W, Orig, NewPrivInst, ShaI);
      }
    }

    Changed = true;
    W->resetBBSet(); // Invalidate BBSet after transformations
  }
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genSharedCodeForTaskLoop\n");
  return Changed;
}

// Build typedef kmp_int32 (* kmp_routine_entry_t)(kmp_int32, void *);
void VPOParoptTransform::genKmpRoutineEntryT() {
  if (!KmpRoutineEntryPtrTy) {
    LLVMContext &C = F->getContext();
    IntegerType *Int32Ty = Type::getInt32Ty(C);
    Type *KmpRoutineEntryTyArgs[] = {Int32Ty, Type::getInt8PtrTy(C)};
    FunctionType *KmpRoutineEntryTy =
        FunctionType::get(Int32Ty, KmpRoutineEntryTyArgs, false);
    KmpRoutineEntryPtrTy = PointerType::getUnqual(KmpRoutineEntryTy);
  }
}

// internal structure for reduction data item related info
//
// typedef struct kmp_task_red_input {
//    void       *reduce_shar; // shared reduction item
//    size_t      reduce_size; // size of data item
//    void       *reduce_init; // data initialization routine
//    void       *reduce_fini; // data finalization routine
//    void       *reduce_comb; // data combiner routine
//    kmp_task_red_flags_t flags; // flags for additional info from compiler
// } kmp_task_red_input_t;
//
void VPOParoptTransform::genTaskTRedType() {
  if (KmpTaskTRedTy)
    return;

  LLVMContext &C = F->getContext();
  IntegerType *Int32Ty = Type::getInt32Ty(C);
  IntegerType *Int64Ty = Type::getInt64Ty(C);

  Type *TaskTRedTyArgs[] = {Type::getInt8PtrTy(C), Int64Ty,
                            Type::getInt8PtrTy(C), Type::getInt8PtrTy(C),
                            Type::getInt8PtrTy(C), Int32Ty};
  KmpTaskTRedTy = StructType::create(C, TaskTRedTyArgs,
                                     "struct.kmp_task_t_red_item", false);
}

// Build struct kmp_task_t {
//            void *              shareds;
//            kmp_routine_entry_t routine;
//            kmp_int32           part_id;
//            kmp_cmplrdata_t data1;
//            kmp_cmplrdata_t data2;
//   For taskloops additional fields:
//            kmp_uint64          lb;
//            kmp_uint64          ub;
//            kmp_int64           st;
//            kmp_int32           liter;
//          };
//
void VPOParoptTransform::genKmpTaskTRecordDecl() {
  if (KmpTaskTTy)
    return;

  LLVMContext &C = F->getContext();
  IntegerType *Int32Ty = Type::getInt32Ty(C);
  IntegerType *Int64Ty = Type::getInt64Ty(C);

  Type *KmpCmplrdataTyArgs[] = {KmpRoutineEntryPtrTy};
  StructType *KmpCmplrdataTy =
      StructType::create(C, KmpCmplrdataTyArgs, "union.kmp_cmplrdata_t", false);

  Type *KmpTaskTyArgs[] = {Type::getInt8PtrTy(C),
                           KmpRoutineEntryPtrTy,
                           Int32Ty,
                           KmpCmplrdataTy,
                           KmpCmplrdataTy,
                           Int64Ty,
                           Int64Ty,
                           Int64Ty,
                           Int32Ty};

  KmpTaskTTy = StructType::create(C, KmpTaskTyArgs, "struct.kmp_task_t", false);
}

// Generate the struct type kmpc_task_t as well as its private data
// area. One example is as follows.
// %struct.kmp_task_t_with_privates = type { %struct.kmp_task_t,
// %struct..kmp_privates.t }
// %struct.kmp_task_t = type { i8*, i32 (i32, i8*)*, i32,
// %union.kmp_cmplrdata_t, %union.kmp_cmplrdata_t, i64, i64, i64, i32}
// %struct..kmp_privates.t = type { i64, i64, i32 }
StructType *VPOParoptTransform::genKmpTaskTWithPrivatesRecordDecl(
    WRegionNode *W, StructType *&KmpSharedTy, StructType *&KmpPrivatesTy) {
  LLVMContext &C = F->getContext();
  SmallVector<Type *, 4> KmpTaksTWithPrivatesTyArgs;
  KmpTaksTWithPrivatesTyArgs.push_back(KmpTaskTTy);

  SmallVector<Type *, 4> KmpPrivatesIndices;
  SmallVector<Type *, 4> SharedIndices;

  unsigned Count = 0;

  FirstprivateClause &FprivClause = W->getFpriv();
  if (!FprivClause.empty()) {
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      Value *Orig = FprivI->getOrig();
      auto PT = dyn_cast<PointerType>(Orig->getType());
      assert(PT && "genKmpTaskTWithPrivatesRecordDecl: Expect first private "
                   "pionter argument");
      KmpPrivatesIndices.push_back(PT->getElementType());
      SharedIndices.push_back(PT->getElementType());
      FprivI->setThunkIdx(Count++);
    }
  }

  LastprivateClause &LprivClause = W->getLpriv();
  if (!LprivClause.empty()) {
    for (LastprivateItem *LprivI : LprivClause.items()) {
      Value *Orig = LprivI->getOrig();
      auto PT = dyn_cast<PointerType>(Orig->getType());
      assert(PT && "genKmpTaskTWithPrivatesRecordDecl: Expect last private "
                   "pionter argument");
      KmpPrivatesIndices.push_back(PT->getElementType());
      SharedIndices.push_back(PT);
      LprivI->setThunkIdx(Count++);
    }
  }

  unsigned SaveCount = Count;
  PrivateClause &PrivClause = W->getPriv();
  if (!PrivClause.empty()) {
    for (PrivateItem *PrivI : PrivClause.items()) {
      Value *Orig = PrivI->getOrig();
      auto PT = dyn_cast<PointerType>(Orig->getType());
      assert(
          PT &&
          "genKmpTaskTWithPrivatesRecordDecl: Expect private pionter argument");
      KmpPrivatesIndices.push_back(PT->getElementType());
      PrivI->setThunkIdx(Count++);
    }
  }

  Count = SaveCount;

  ReductionClause &RedClause = W->getRed();
  if (!RedClause.empty()) {
    for (ReductionItem *RedI : RedClause.items()) {
      Value *Orig = RedI->getOrig();
      auto PT = dyn_cast<PointerType>(Orig->getType());
      assert(PT && "genKmpTaskTWithPrivatesRecordDecl: Expect reduction "
                   "pionter argument");
      SharedIndices.push_back(PT);
      RedI->setThunkIdx(Count++);
    }
  }

  SharedClause &ShaClause = W->getShared();
  if (!ShaClause.empty()) {
    for (SharedItem *ShaI : ShaClause.items()) {
      Value *Orig = ShaI->getOrig();
      auto PT = dyn_cast<PointerType>(Orig->getType());
      assert(PT && "genKmpTaskTWithPrivatesRecordDecl: Expect shared "
                   "pionter argument");
      SharedIndices.push_back(PT);
      ShaI->setThunkIdx(Count++);
    }
  }

  KmpPrivatesTy = StructType::create(
      C, makeArrayRef(KmpPrivatesIndices.begin(), KmpPrivatesIndices.end()),
      "struct..kmp_privates.t", false);

  KmpSharedTy = StructType::create(
      C, makeArrayRef(SharedIndices.begin(), SharedIndices.end()),
      "struct.shared.t", false);

  KmpTaksTWithPrivatesTyArgs.push_back(KmpPrivatesTy);

  StructType *KmpTaskTTWithPrivatesTy =
      StructType::create(C, makeArrayRef(KmpTaksTWithPrivatesTyArgs.begin(),
                                         KmpTaksTWithPrivatesTyArgs.end()),
                         "struct.kmp_task_t_with_privates", false);

  return KmpTaskTTWithPrivatesTy;
}

// Generate the code to replace the variables in the task loop with
// the thunk field dereferences
bool VPOParoptTransform::genTaskLoopInitCode(
    WRegionNode *W, StructType *&KmpTaskTTWithPrivatesTy,
    StructType *&KmpSharedTy, Value *&LBPtr, Value *&UBPtr, Value *&STPtr) {

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTaskLoopInitCode\n");

  Loop *L = W->getLoop();
  assert(L && "genTaskLoopInitCode: Loop not found");
  genLoopInitCodeForTaskLoop(W, LBPtr, UBPtr, STPtr);

  // Build type kmp_task_t
  genKmpRoutineEntryT();
  genKmpTaskTRecordDecl();
  KmpSharedTy = nullptr;
  StructType *KmpPrivatesTy = nullptr;
  KmpTaskTTWithPrivatesTy =
      genKmpTaskTWithPrivatesRecordDecl(W, KmpSharedTy, KmpPrivatesTy);

  IRBuilder<> Builder(F->getEntryBlock().getTerminator());
  AllocaInst *DummyTaskTWithPrivates = Builder.CreateAlloca(
      KmpTaskTTWithPrivatesTy, nullptr, "taskt.withprivates");

  Builder.SetInsertPoint(W->getEntryBBlock()->getTerminator());
  SmallVector<Value *, 4> Indices;
  Indices.push_back(Builder.getInt32(0));
  Indices.push_back(Builder.getInt32(0));
  Value *BaseTaskTGep = Builder.CreateInBoundsGEP(
      KmpTaskTTWithPrivatesTy, DummyTaskTWithPrivates, Indices);
  /*
    Indices.clear();
    Indices.push_back(Builder.getInt32(0));
    Indices.push_back(Builder.getInt32(2));
    Value *PartIdGep =
        Builder.CreateInBoundsGEP(KmpTaskTTy, BaseTaskTGep, Indices);
  */
  Indices.clear();
  Indices.push_back(Builder.getInt32(0));
  Indices.push_back(Builder.getInt32(0));
  Value *SharedGep =
      Builder.CreateInBoundsGEP(KmpTaskTTy, BaseTaskTGep, Indices);
  Value *SharedLoad = Builder.CreateLoad(SharedGep);
  Value *SharedCast =
      Builder.CreateBitCast(SharedLoad, PointerType::getUnqual(KmpSharedTy));

  Indices.clear();
  Indices.push_back(Builder.getInt32(0));
  Indices.push_back(Builder.getInt32(1));
  Value *PrivatesGep = Builder.CreateInBoundsGEP(
      KmpTaskTTWithPrivatesTy, DummyTaskTWithPrivates, Indices);

  Indices.clear();
  Indices.push_back(Builder.getInt32(0));
  Indices.push_back(Builder.getInt32(5));
  Value *LowerBoundGep =
      Builder.CreateInBoundsGEP(KmpTaskTTy, BaseTaskTGep, Indices);
  Value *LowerBoundLd = Builder.CreateLoad(LowerBoundGep);

  Indices.clear();
  Indices.push_back(Builder.getInt32(0));
  Indices.push_back(Builder.getInt32(6));
  Value *UpperBoundGep =
      Builder.CreateInBoundsGEP(KmpTaskTTy, BaseTaskTGep, Indices);
  Value *UpperBoundLd = Builder.CreateLoad(UpperBoundGep);

  /*The stride does not need to change since the loop is normalized by the clang
  Indices.clear();
  Indices.push_back(Builder.getInt32(0));
  Indices.push_back(Builder.getInt32(7));
  Value *StrideGep =
      Builder.CreateInBoundsGEP(KmpTaskTTy, BaseTaskTGep, Indices);
  Value *StrideLd = Builder.CreateLoad(StrideGep);

  Indices.clear();
  Indices.push_back(Builder.getInt32(0));
  Indices.push_back(Builder.getInt32(8));
  Value *LastIterGep =
      Builder.CreateInBoundsGEP(KmpTaskTTy, BaseTaskTGep, Indices);
  Value *LastIterLd = Builder.CreateLoad(LastIterGep);
  */
  Type *IndValTy = WRegionUtils::getOmpCanonicalInductionVariable(L)
                       ->getIncomingValue(0)
                       ->getType();

  PHINode *PN = WRegionUtils::getOmpCanonicalInductionVariable(L);
  PN->removeIncomingValue(L->getLoopPreheader());

  if (LowerBoundLd->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    LowerBoundLd = Builder.CreateSExtOrTrunc(LowerBoundLd, IndValTy);

  PN->addIncoming(LowerBoundLd, L->getLoopPreheader());

  if (UpperBoundLd->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    UpperBoundLd = Builder.CreateSExtOrTrunc(UpperBoundLd, IndValTy);
  VPOParoptUtils::updateOmpPredicateAndUpperBound(W, UpperBoundLd,
                                                  &*Builder.GetInsertPoint());
  PrivateClause &PrivClause = W->getPriv();
  if (!PrivClause.empty()) {
    for (PrivateItem *PrivI : PrivClause.items()) {
      Indices.clear();
      Indices.push_back(Builder.getInt32(0));
      Indices.push_back(Builder.getInt32(PrivI->getThunkIdx()));
      Value *ThunkPrivatesGep =
          Builder.CreateInBoundsGEP(KmpPrivatesTy, PrivatesGep, Indices);
      PrivI->setNew(ThunkPrivatesGep);
    }
  }

  FirstprivateClause &FprivClause = W->getFpriv();
  if (!FprivClause.empty()) {
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      Indices.clear();
      Indices.push_back(Builder.getInt32(0));
      Indices.push_back(Builder.getInt32(FprivI->getThunkIdx()));
      Value *ThunkPrivatesGep =
          Builder.CreateInBoundsGEP(KmpPrivatesTy, PrivatesGep, Indices);
      FprivI->setNew(ThunkPrivatesGep);
    }
  }

  LastprivateClause &LprivClause = W->getLpriv();
  if (!LprivClause.empty()) {
    for (LastprivateItem *LprivI : LprivClause.items()) {
      Indices.clear();
      Indices.push_back(Builder.getInt32(0));
      Indices.push_back(Builder.getInt32(LprivI->getThunkIdx()));
      Value *ThunkPrivatesGep =
          Builder.CreateInBoundsGEP(KmpPrivatesTy, PrivatesGep, Indices);
      LprivI->setNew(ThunkPrivatesGep);
    }
  }

  ReductionClause &RedClause = W->getRed();
  if (!RedClause.empty()) {
    for (ReductionItem *RedI : RedClause.items()) {
      Indices.clear();
      Indices.push_back(Builder.getInt32(0));
      Indices.push_back(Builder.getInt32(RedI->getThunkIdx()));
      Value *ThunkSharedGep =
          Builder.CreateInBoundsGEP(KmpSharedTy, SharedCast, Indices);
      Value *ThunkSharedVal = Builder.CreateLoad(ThunkSharedGep);
      Value *RedRes = VPOParoptUtils::genKmpcRedGetNthData(
          W, TidPtr, ThunkSharedVal, &*Builder.GetInsertPoint(), Mode & OmpTbb);
      RedI->setNew(Builder.CreateBitCast(RedRes, RedI->getOrig()->getType()));
    }
  }

  SharedClause &ShaClause = W->getShared();
  if (!ShaClause.empty()) {
    for (SharedItem *ShaI : ShaClause.items()) {
      Indices.clear();
      Indices.push_back(Builder.getInt32(0));
      Indices.push_back(Builder.getInt32(ShaI->getThunkIdx()));
      Value *ThunkSharedGep =
          Builder.CreateInBoundsGEP(KmpSharedTy, SharedCast, Indices);
      ShaI->setNew(Builder.CreateLoad(ThunkSharedGep));
    }
  }

  prepareNoAliasMetadataInTaskLoop(W);

  W->resetBBSet();
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genTaskLoopInitCode\n");
  return true;
}

// Prepare the scope alias metadata for the references of the
// firstprivate, lastprivate, private shared, reduction variables
void VPOParoptTransform::prepareNoAliasMetadataInTaskLoop(WRegionNode *W) {
  LLVMContext &Context = F->getContext();
  MDBuilder MDB(Context);
  MDNode *Domain = MDB.createAnonymousAliasScopeDomain("ParoptDomain");
  SmallVector<Metadata *, 4> AllScopes;
  SmallVector<Metadata *, 4> NonAliasingScopes;

  PrivateClause &PrivClause = W->getPriv();
  if (!PrivClause.empty()) {
    for (PrivateItem *PrivI : PrivClause.items()) {
      auto MD = MDB.createAnonymousAliasScope(Domain);
      PrivI->setAliasScope(MD);
      AllScopes.push_back(MD);
    }
  }

  FirstprivateClause &FprivClause = W->getFpriv();
  if (!FprivClause.empty()) {
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      auto MD = MDB.createAnonymousAliasScope(Domain);
      FprivI->setAliasScope(MD);
      AllScopes.push_back(MD);
    }
  }

  LastprivateClause &LprivClause = W->getLpriv();
  if (!LprivClause.empty()) {
    for (LastprivateItem *LprivI : LprivClause.items()) {
      auto MD = MDB.createAnonymousAliasScope(Domain);
      LprivI->setAliasScope(MD);
      AllScopes.push_back(MD);
    }
  }

  ReductionClause &RedClause = W->getRed();
  if (!RedClause.empty()) {
    for (ReductionItem *RedI : RedClause.items()) {
      auto MD = MDB.createAnonymousAliasScope(Domain);
      RedI->setAliasScope(MD);
      AllScopes.push_back(MD);
    }
  }

  SharedClause &ShaClause = W->getShared();
  if (!ShaClause.empty()) {
    for (SharedItem *ShaI : ShaClause.items()) {
      auto MD = MDB.createAnonymousAliasScope(Domain);
      ShaI->setAliasScope(MD);
      AllScopes.push_back(MD);
    }
  }

  PrivClause = W->getPriv();
  if (!PrivClause.empty()) {
    for (PrivateItem *PrivI : PrivClause.items()) {
      NonAliasingScopes.clear();
      for (auto *MD : AllScopes) {
        if (MD != PrivI->getAliasScope())
          NonAliasingScopes.push_back(MD);
      }
      PrivI->setNoAlias(MDNode::get(Context, NonAliasingScopes));
    }
  }

  FprivClause = W->getFpriv();
  if (!FprivClause.empty()) {
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      NonAliasingScopes.clear();
      for (auto *MD : AllScopes) {
        if (MD != FprivI->getAliasScope())
          NonAliasingScopes.push_back(MD);
      }
      FprivI->setNoAlias(MDNode::get(Context, NonAliasingScopes));
    }
  }

  LprivClause = W->getLpriv();
  if (!LprivClause.empty()) {
    for (LastprivateItem *LprivI : LprivClause.items()) {
      NonAliasingScopes.clear();
      for (auto *MD : AllScopes) {
        if (MD != LprivI->getAliasScope())
          NonAliasingScopes.push_back(MD);
      }
      LprivI->setNoAlias(MDNode::get(Context, NonAliasingScopes));
    }
  }

  RedClause = W->getRed();
  if (!RedClause.empty()) {
    for (ReductionItem *RedI : RedClause.items()) {
      NonAliasingScopes.clear();
      for (auto *MD : AllScopes) {
        if (MD != RedI->getAliasScope())
          NonAliasingScopes.push_back(MD);
      }
      RedI->setNoAlias(MDNode::get(Context, NonAliasingScopes));
    }
  }

  ShaClause = W->getShared();
  if (!ShaClause.empty()) {
    for (SharedItem *ShaI : ShaClause.items()) {
      NonAliasingScopes.clear();
      for (auto *MD : AllScopes) {
        if (MD != ShaI->getAliasScope())
          NonAliasingScopes.push_back(MD);
      }
      ShaI->setNoAlias(MDNode::get(Context, NonAliasingScopes));
    }
  }
}

// Set up the mapping between the variables (firstprivate,
// lastprivate, reduction and shared) and the counterparts in the thunk.
AllocaInst *VPOParoptTransform::genTaskPrivateMapping(WRegionNode *W,
                                                      Instruction *InsertPt,
                                                      StructType *KmpSharedTy) {
  SmallVector<Value *, 4> Indices;

  IRBuilder<> Builder(InsertPt);
  AllocaInst *TaskSharedBase =
      Builder.CreateAlloca(KmpSharedTy, nullptr, "taskt.shared.agg");

  FirstprivateClause &FprivClause = W->getFpriv();
  if (!FprivClause.empty()) {
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      Indices.clear();
      Indices.push_back(Builder.getInt32(0));
      Indices.push_back(Builder.getInt32(FprivI->getThunkIdx()));
      Value *Gep =
          Builder.CreateInBoundsGEP(KmpSharedTy, TaskSharedBase, Indices);
      LoadInst *Load = Builder.CreateLoad(FprivI->getOrig());
      Builder.CreateStore(Load, Gep);
    }
  }

  LastprivateClause &LprivClause = W->getLpriv();
  if (!LprivClause.empty()) {
    for (LastprivateItem *LprivI : LprivClause.items()) {
      Indices.clear();
      Indices.push_back(Builder.getInt32(0));
      Indices.push_back(Builder.getInt32(LprivI->getThunkIdx()));
      Value *Gep =
          Builder.CreateInBoundsGEP(KmpSharedTy, TaskSharedBase, Indices);
      Builder.CreateStore(LprivI->getOrig(), Gep);
    }
  }

  ReductionClause &RedClause = W->getRed();
  if (!RedClause.empty()) {
    for (ReductionItem *RedI : RedClause.items()) {
      Indices.clear();
      Indices.push_back(Builder.getInt32(0));
      Indices.push_back(Builder.getInt32(RedI->getThunkIdx()));
      Value *Gep =
          Builder.CreateInBoundsGEP(KmpSharedTy, TaskSharedBase, Indices);
      Builder.CreateStore(RedI->getOrig(), Gep);
    }
  }

  SharedClause &ShaClause = W->getShared();
  if (!ShaClause.empty()) {
    for (SharedItem *ShaI : ShaClause.items()) {
      Indices.clear();
      Indices.push_back(Builder.getInt32(0));
      Indices.push_back(Builder.getInt32(ShaI->getThunkIdx()));
      Value *Gep =
          Builder.CreateInBoundsGEP(KmpSharedTy, TaskSharedBase, Indices);
      Builder.CreateStore(ShaI->getOrig(), Gep);
    }
  }

  return TaskSharedBase;
}

// Initialize the data in the shared data area inside the thunk
void VPOParoptTransform::genSharedInitForTaskLoop(
    WRegionNode *W, AllocaInst *Src, Value *Dst, StructType *KmpSharedTy,
    StructType *KmpTaskTTWithPrivatesTy, Instruction *InsertPt) {
  IRBuilder<> Builder(InsertPt);

  Value *Cast = Builder.CreateBitCast(
      Dst, PointerType::getUnqual(KmpTaskTTWithPrivatesTy));

  SmallVector<Value *, 4> Indices;
  Indices.push_back(Builder.getInt32(0));
  Indices.push_back(Builder.getInt32(0));
  Value *TaskTTyGep =
      Builder.CreateInBoundsGEP(KmpTaskTTWithPrivatesTy, Cast, Indices);

  StructType *KmpTaskTTy =
      dyn_cast<StructType>(KmpTaskTTWithPrivatesTy->getElementType(0));

  Value *SharedTyGep =
      Builder.CreateInBoundsGEP(KmpTaskTTy, TaskTTyGep, Indices);

  Value *LI = Builder.CreateLoad(SharedTyGep);

  LLVMContext &C = F->getContext();
  Value *SrcCast = Builder.CreateBitCast(Src, Type::getInt8PtrTy(C));

  Value *Size;

  const DataLayout DL = F->getParent()->getDataLayout();
  if (DL.getIntPtrType(Builder.getInt8PtrTy())->getIntegerBitWidth() == 64)
    Size = Builder.getInt64(
        DL.getTypeAllocSize(Src->getType()->getPointerElementType()));
  else
    Size = Builder.getInt32(
        DL.getTypeAllocSize(Src->getType()->getPointerElementType()));

  Builder.CreateMemCpy(LI, SrcCast, Size,
                       DL.getABITypeAlignment(Src->getAllocatedType()));

  Indices.clear();
  Indices.push_back(Builder.getInt32(0));
  Indices.push_back(Builder.getInt32(1));
  Value *PrivatesGep =
      Builder.CreateInBoundsGEP(KmpTaskTTWithPrivatesTy, Cast, Indices);

  Value *SharedBase =
      Builder.CreateBitCast(LI, PointerType::getUnqual(KmpSharedTy));

  StructType *KmpPrivatesTy =
      dyn_cast<StructType>(KmpTaskTTWithPrivatesTy->getElementType(1));

  FirstprivateClause &FprivClause = W->getFpriv();
  if (!FprivClause.empty()) {
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      int TIdx = FprivI->getThunkIdx();

      Indices.clear();
      Indices.push_back(Builder.getInt32(0));
      Indices.push_back(Builder.getInt32(TIdx));
      Value *PrivateGep =
          Builder.CreateInBoundsGEP(KmpPrivatesTy, PrivatesGep, Indices);
      Value *SharedGep =
          Builder.CreateInBoundsGEP(KmpSharedTy, SharedBase, Indices);
      if (DL.getIntPtrType(Builder.getInt8PtrTy())->getIntegerBitWidth() == 64)
        Size = Builder.getInt64(DL.getTypeAllocSize(
            FprivI->getOrig()->getType()->getPointerElementType()));
      else
        Size = Builder.getInt32(DL.getTypeAllocSize(
            FprivI->getOrig()->getType()->getPointerElementType()));

      Value *S = Builder.CreateBitCast(
          SharedGep, PointerType::getUnqual(Type::getInt8PtrTy(C)));
      Value *D = Builder.CreateBitCast(
          PrivateGep, PointerType::getUnqual(Type::getInt8PtrTy(C)));
      Builder.CreateMemCpy(
          D, S, Size, DL.getABITypeAlignment(FprivI->getOrig()->getType()));
    }
  }
}

// Save the loop lower upper bound, upper bound and stride for the use
// by the call __kmpc_taskloop
void VPOParoptTransform::genLoopInitCodeForTaskLoop(WRegionNode *W,
                                                    Value *&LBPtr,
                                                    Value *&UBPtr,
                                                    Value *&STPtr) {
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *NewEntryBB = SplitBlock(EntryBB, &*(EntryBB->begin()), DT, LI);
  W->setEntryBBlock(NewEntryBB);
  IRBuilder<> Builder(EntryBB->getTerminator());
  Loop *L = W->getLoop();
  Type *IndValTy = WRegionUtils::getOmpCanonicalInductionVariable(L)
                       ->getIncomingValue(0)
                       ->getType();

  AllocaInst *LowerBnd = Builder.CreateAlloca(IndValTy, nullptr, "lower.bnd");
  Value *InitVal = WRegionUtils::getOmpLoopLowerBound(L);

  InitVal = VPOParoptUtils::cloneInstructions(InitVal, &*(EntryBB->begin()));

  if (InitVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    InitVal = Builder.CreateSExtOrTrunc(InitVal, IndValTy);
  Builder.CreateStore(InitVal, LowerBnd);
  LBPtr = LowerBnd;

  AllocaInst *UpperBnd = Builder.CreateAlloca(IndValTy, nullptr, "upper.bnd");
  Value *UpperBndVal =
      VPOParoptUtils::computeOmpUpperBound(W, EntryBB->getTerminator());

  if (UpperBndVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    UpperBndVal = Builder.CreateSExtOrTrunc(UpperBndVal, IndValTy);
  Builder.CreateStore(UpperBndVal, UpperBnd);
  UBPtr = UpperBnd;

  AllocaInst *Stride = Builder.CreateAlloca(IndValTy, nullptr, "stride");
  bool IsNegStride;
  Value *StrideVal = WRegionUtils::getOmpLoopStride(L, IsNegStride);
  StrideVal =
      VPOParoptUtils::cloneInstructions(StrideVal, &*(EntryBB->begin()));

  if (StrideVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    StrideVal = Builder.CreateSExtOrTrunc(StrideVal, IndValTy);

  Builder.CreateStore(StrideVal, Stride);
  STPtr = Stride;
}

// Generate the outline function of reduction initilaization
Function *VPOParoptTransform::genTaskLoopRedInitFunc(WRegionNode *W,
                                                     ReductionItem *RedI) {
  LLVMContext &C = F->getContext();
  Value *Orig = RedI->getOrig();
  Module *M = F->getParent();

  Type *TaskLoopRedInitParams[] = {Orig->getType()};
  FunctionType *TaskLoopRedInitFnTy =
      FunctionType::get(Type::getVoidTy(C), TaskLoopRedInitParams, false);

  Function *FnTaskLoopRedInit = Function::Create(
      TaskLoopRedInitFnTy, GlobalValue::ExternalLinkage,
      F->getName() + "_task_red_init_" + Twine(W->getNumber()), M);
  FnTaskLoopRedInit->setCallingConv(CallingConv::C);

  Value *Arg = &*FnTaskLoopRedInit->arg_begin();

  BasicBlock *EntryBB = BasicBlock::Create(C, "entry", FnTaskLoopRedInit);

  if (DT)
    DT->recalculate(*FnTaskLoopRedInit);

  IRBuilder<> Builder(EntryBB);
  Builder.CreateRetVoid();
  Value *NewRedInst =
      genPrivatizationAlloca(W, Orig, &EntryBB->front(), ".red");

  RedI->setNew(NewRedInst);
  genReductionInit(RedI, EntryBB->getTerminator());

  NewRedInst->replaceAllUsesWith(Arg);

  return FnTaskLoopRedInit;
}

// Generate the outline function for the reduction update
Function *VPOParoptTransform::genTaskLoopRedCombFunc(WRegionNode *W,
                                                     ReductionItem *RedI) {
  LLVMContext &C = F->getContext();
  Value *Orig = RedI->getOrig();
  Module *M = F->getParent();

  Type *TaskLoopRedInitParams[] = {Orig->getType(), Orig->getType()};
  FunctionType *TaskLoopRedInitFnTy =
      FunctionType::get(Type::getVoidTy(C), TaskLoopRedInitParams, false);

  Function *FnTaskLoopRedComb = Function::Create(
      TaskLoopRedInitFnTy, GlobalValue::ExternalLinkage,
      F->getName() + "_task_red_comb_" + Twine(W->getNumber()), M);
  FnTaskLoopRedComb->setCallingConv(CallingConv::C);

  auto I = FnTaskLoopRedComb->arg_begin();
  Value *DstArg = &*I;
  I++;
  Value *SrcArg = &*I;

  BasicBlock *EntryBB = BasicBlock::Create(C, "entry", FnTaskLoopRedComb);

  if (DT)
    DT->recalculate(*FnTaskLoopRedComb);

  IRBuilder<> Builder(EntryBB);
  Builder.CreateRetVoid();

  Value *NewRedInst = RedI->getNew();

  genReductionFini(RedI, DstArg, EntryBB->getTerminator());

  NewRedInst->replaceAllUsesWith(SrcArg);

  cast<AllocaInst>(NewRedInst)->eraseFromParent();

  return FnTaskLoopRedComb;
}

// Generate the call __kmpc_task_reduction_init and the corresponding
// preparation.
void VPOParoptTransform::genRedInitForTaskLoop(WRegionNode *W,
                                               Instruction *InsertBefore) {

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genRedInitForTaskLoop\n");

  genTaskTRedType();

  SmallVector<Type *, 4> KmpTaksTRedRecTyArgs;

  ReductionClause &RedClause = W->getRed();
  if (RedClause.empty())
    return;
  LLVMContext &C = F->getContext();

  for (int I = 0; I < RedClause.size(); I++)
    KmpTaksTRedRecTyArgs.push_back(KmpTaskTRedTy);

  StructType *KmpTaskTTRedRecTy = StructType::create(
      C, makeArrayRef(KmpTaksTRedRecTyArgs.begin(), KmpTaksTRedRecTyArgs.end()),
      "struct.kmp_task_t_red_rec", false);

  IRBuilder<> Builder(InsertBefore);
  AllocaInst *DummyTaskTRedRec =
      Builder.CreateAlloca(KmpTaskTTRedRecTy, nullptr, "taskt.red.rec");

  const DataLayout DL = F->getParent()->getDataLayout();
  unsigned Count = 0;
  for (ReductionItem *RedI : RedClause.items()) {

    Value *BaseTaskTRedGep = Builder.CreateInBoundsGEP(
        KmpTaskTTRedRecTy, DummyTaskTRedRec,
        {Builder.getInt32(0), Builder.getInt32(Count++)});

    Value *Gep =
        Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                  {Builder.getInt32(0), Builder.getInt32(0)});
    Builder.CreateStore(
        Builder.CreateBitCast(RedI->getOrig(), Type::getInt8PtrTy(C)), Gep);

    Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                    {Builder.getInt32(0), Builder.getInt32(1)});
    Builder.CreateStore(
        Builder.getInt64(DL.getTypeAllocSize(
            RedI->getOrig()->getType()->getPointerElementType())),
        Gep);

    Function *RedInitFunc = genTaskLoopRedInitFunc(W, RedI);
    Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                    {Builder.getInt32(0), Builder.getInt32(2)});
    Builder.CreateStore(
        Builder.CreateBitCast(RedInitFunc, Type::getInt8PtrTy(C)), Gep);

    Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                    {Builder.getInt32(0), Builder.getInt32(3)});
    Builder.CreateStore(ConstantPointerNull::get(Type::getInt8PtrTy(C)), Gep);

    Function *RedCombFunc = genTaskLoopRedCombFunc(W, RedI);
    Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                    {Builder.getInt32(0), Builder.getInt32(4)});
    Builder.CreateStore(
        Builder.CreateBitCast(RedCombFunc, Type::getInt8PtrTy(C)), Gep);

    Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                    {Builder.getInt32(0), Builder.getInt32(5)});
    Builder.CreateStore(Builder.getInt32(0), Gep);
  }
  VPOParoptUtils::genKmpcTaskReductionInit(W, TidPtr, Count, DummyTaskTRedRec,
                                           &*Builder.GetInsertPoint(),
                                           Mode & OmpTbb);
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genRedInitForTaskLoop\n");
}

// Generate the call __kmpc_omp_task_alloc, __kmpc_taskloop and the
// corresponding outlined function
bool VPOParoptTransform::genTaskLoopCode(WRegionNode *W,
                                         StructType *KmpTaskTTWithPrivatesTy,
                                         StructType *KmpSharedTy, Value *LBPtr,
                                         Value *UBPtr, Value *STPtr) {

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTaskLoopCode\n");

  assert(W->isBBSetEmpty() && "genTaskLoopCode: BBSET should start empty");

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

    for (auto I = CS.arg_begin(), E = CS.arg_end(); I != E; ++I) {
      ++TidArgNo;
    }

    Function *MTFn =
        finalizeExtractedMTFunction(W, NewF, false, TidArgNo, false);

    std::vector<Value *> MTFnArgs;

    LLVMContext &C = NewF->getContext();
    IntegerType *Int32Ty = Type::getInt32Ty(C);
    ConstantInt *ValueZero = ConstantInt::getSigned(Int32Ty, 0);
    MTFnArgs.push_back(ValueZero);
    genThreadedEntryActualParmList(W, MTFnArgs);

    finiCodeExtractorPrepare(MTFn, true);
    for (auto I = CS.arg_begin(), E = CS.arg_end(); I != E; ++I) {
      MTFnArgs.push_back((*I));
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

    genRedInitForTaskLoop(W, NewCall);

    AllocaInst *PrivateBase = genTaskPrivateMapping(W, NewCall, KmpSharedTy);
    const DataLayout DL = NewF->getParent()->getDataLayout();
    int KmpTaskTTWithPrivatesTySz =
        DL.getTypeAllocSize(KmpTaskTTWithPrivatesTy);
    int KmpSharedTySz = DL.getTypeAllocSize(KmpSharedTy);
    CallInst *TaskAllocCI = VPOParoptUtils::genKmpcTaskAlloc(
        W, IdentTy, TidPtr, KmpTaskTTWithPrivatesTySz, KmpSharedTySz,
        KmpRoutineEntryPtrTy, MTFnCI->getCalledFunction(), NewCall,
        Mode & OmpTbb);

    genSharedInitForTaskLoop(W, PrivateBase, TaskAllocCI, KmpSharedTy,
                             KmpTaskTTWithPrivatesTy, NewCall);

    VPOParoptUtils::genKmpcTaskLoop(W, IdentTy, TidPtr, TaskAllocCI, LBPtr,
                                    UBPtr, STPtr, KmpTaskTTWithPrivatesTy,
                                    NewCall, Mode & OmpTbb);

    NewCall->eraseFromParent();

    NewF->eraseFromParent();

    MTFnCI->eraseFromParent();

    W->resetBBSet(); // Invalidate BBSet after transformations

    Changed = true;
  }
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genTaskLoopCode\n");
  return Changed;
}
