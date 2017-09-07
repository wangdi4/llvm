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
bool VPOParoptTransform::genRedCodeForTaskGeneric(WRegionNode *W) {

  bool Changed = false;

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genRedCodeForTaskGeneric\n");

  ReductionClause &RedClause = W->getRed();
  if (!RedClause.empty()) {

    assert(W->isBBSetEmpty() &&
           "genRedCodeForTaskGeneric: BBSET should start empty");
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
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genRedCodeForTaskGeneric\n");
  return Changed;
}

// Generate the code to update the last privates for taskloop.
void VPOParoptTransform::genLprivFiniForTaskLoop(Value *Dst, Value *Src,
                                                 Instruction *InsertPt) {
  GetElementPtrInst *Gep = dyn_cast<GetElementPtrInst>(Src);
  Type *ScalarTy = Gep->getResultElementType();
  const DataLayout &DL = InsertPt->getModule()->getDataLayout();

  IRBuilder<> Builder(InsertPt);
  if (!DL.isLegalInteger(DL.getTypeSizeInBits(ScalarTy)) ||
      DL.getTypeSizeInBits(ScalarTy) % 8 != 0) {
    VPOParoptUtils::genMemcpy(Dst, Src, DL, DL.getABITypeAlignment(ScalarTy),
                              InsertPt->getParent());
  } else {
    LoadInst *Load = Builder.CreateLoad(Src);
    Builder.CreateStore(Load, Dst);
  }
}

// Replace the shared variable reference with the thunk field
// derefernce
bool VPOParoptTransform::genSharedCodeForTaskGeneric(WRegionNode *W) {

  bool Changed = false;

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genSharedCodeForTaskGeneric\n");

  SharedClause &ShaClause = W->getShared();
  if (!ShaClause.empty()) {

    assert(W->isBBSetEmpty() &&
           "genSharedCodeForTaskGeneric: BBSET should start empty");
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
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genSharedCodeForTaskGeneric\n");
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

// internal structure for dependInfo
//
// struct kmp_depend_info {
//   size_t arg_addr;
//   size_t arg_size;
//   char   depend_type;
// };
void VPOParoptTransform::genKmpTaskDependInfo() {
  if (KmpTaskDependInfoTy)
    return;

  LLVMContext &C = F->getContext();

  IntegerType *IntTy;
  const DataLayout &DL = F->getParent()->getDataLayout();

  if (DL.getIntPtrType(Type::getInt8PtrTy(C))->getIntegerBitWidth() == 64)
    IntTy = Type::getInt64Ty(C);
  else
    IntTy = Type::getInt32Ty(C);

  Type *KmpTaskDependTyArgs[] = {IntTy, IntTy, Type::getInt8Ty(C)};

  KmpTaskDependInfoTy = StructType::create(C, KmpTaskDependTyArgs,
                                           "struct.kmp_depend_info", false);
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
                   "pointer argument");
      KmpPrivatesIndices.push_back(PT->getElementType());
      SharedIndices.push_back(PT->getElementType());
      FprivI->setThunkIdx(Count++);
    }
  }

  if (W->hasLastprivate()) {
    LastprivateClause &LprivClause = W->getLpriv();
    if (!LprivClause.empty()) {
      for (LastprivateItem *LprivI : LprivClause.items()) {
        Value *Orig = LprivI->getOrig();
        auto PT = dyn_cast<PointerType>(Orig->getType());
        assert(PT && "genKmpTaskTWithPrivatesRecordDecl: Expect last private "
                     "pointer argument");
        KmpPrivatesIndices.push_back(PT->getElementType());
        SharedIndices.push_back(PT);
        LprivI->setThunkIdx(Count++);
      }
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
          "genKmpTaskTWithPrivatesRecordDecl: Expect private pointer argument");
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
                   "pointer argument");
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
                   "pointer argument");
      SharedIndices.push_back(PT);
      ShaI->setThunkIdx(Count++);
    }
  }

  KmpPrivatesTy = StructType::create(
      C, makeArrayRef(KmpPrivatesIndices.begin(), KmpPrivatesIndices.end()),
      "struct.kmp_privates.t", false);

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

bool VPOParoptTransform::genTaskInitCode(WRegionNode *W,
                                         StructType *&KmpTaskTTWithPrivatesTy,
                                         StructType *&KmpSharedTy,
                                         Value *&LastIterGep) {

  Value *LBPtr, *UBPtr, *STPtr;

  return genTaskLoopInitCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy, LBPtr,
                             UBPtr, STPtr, LastIterGep, false);
}

// Generate the code to replace the variables in the task loop with
// the thunk field dereferences
bool VPOParoptTransform::genTaskLoopInitCode(
    WRegionNode *W, StructType *&KmpTaskTTWithPrivatesTy,
    StructType *&KmpSharedTy, Value *&LBPtr, Value *&UBPtr, Value *&STPtr,
    Value *&LastIterGep, bool isLoop) {

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTaskLoopInitCode\n");

  Loop *L;
  if (isLoop) {
    L = W->getLoop();
    assert(L && "genTaskLoopInitCode: Loop not found");
    genLoopInitCodeForTaskLoop(W, LBPtr, UBPtr, STPtr);
  }

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
  */

  Indices.clear();
  Indices.push_back(Builder.getInt32(0));
  Indices.push_back(Builder.getInt32(8));
  LastIterGep = Builder.CreateInBoundsGEP(KmpTaskTTy, BaseTaskTGep, Indices);

  if (isLoop) {
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
  }
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

  if (W->hasLastprivate()) {
    LastprivateClause &LprivClause = W->getLpriv();
    if (!LprivClause.empty()) {
      for (LastprivateItem *LprivI : LprivClause.items()) {
        Indices.clear();
        Indices.push_back(Builder.getInt32(0));
        Indices.push_back(Builder.getInt32(LprivI->getThunkIdx()));
        Value *ThunkPrivatesGep =
            Builder.CreateInBoundsGEP(KmpPrivatesTy, PrivatesGep, Indices);
        Value *ThunkSharedGep =
            Builder.CreateInBoundsGEP(KmpSharedTy, SharedCast, Indices);
        Value *ThunkSharedVal = Builder.CreateLoad(ThunkSharedGep);
        // Parm is used to record the address of last private in the compiler
        // shared variables in the thunk.
        LprivI->setNew(ThunkPrivatesGep);
        LprivI->setParm(ThunkSharedVal);
      }
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

  W->resetBBSet();
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genTaskLoopInitCode\n");
  return true;
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

  if (W->hasLastprivate()) {
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
      TaskLoopRedInitFnTy, GlobalValue::InternalLinkage,
      F->getName() + "_task_red_init_" + Twine(W->getNumber()), M);
  FnTaskLoopRedInit->setCallingConv(CallingConv::C);

  Value *Arg = &*FnTaskLoopRedInit->arg_begin();

  BasicBlock *EntryBB = BasicBlock::Create(C, "entry", FnTaskLoopRedInit);

  // Since the function RedInit is created from scratch, the compiler
  // needs to declare a new DominatorTree.
  DominatorTree DT;
  DT.recalculate(*FnTaskLoopRedInit);

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
      TaskLoopRedInitFnTy, GlobalValue::InternalLinkage,
      F->getName() + "_task_red_comb_" + Twine(W->getNumber()), M);
  FnTaskLoopRedComb->setCallingConv(CallingConv::C);

  auto I = FnTaskLoopRedComb->arg_begin();
  Value *DstArg = &*I;
  I++;
  Value *SrcArg = &*I;

  BasicBlock *EntryBB = BasicBlock::Create(C, "entry", FnTaskLoopRedComb);

  DominatorTree DT;
  DT.recalculate(*FnTaskLoopRedComb);

  IRBuilder<> Builder(EntryBB);
  Builder.CreateRetVoid();

  Value *NewRedInst = RedI->getNew();

  genReductionFini(RedI, DstArg, EntryBB->getTerminator());

  NewRedInst->replaceAllUsesWith(SrcArg);

  cast<AllocaInst>(NewRedInst)->eraseFromParent();

  return FnTaskLoopRedComb;
}

// Generate the function for the last private so that the runtime can call it to
// set the last iteration flag.
Function *
VPOParoptTransform::genLastPrivateTaskDup(WRegionNode *W,
                                          StructType *KmpTaskTTWithPrivatesTy) {

  LastprivateClause &LprivClause = W->getLpriv();
  if (LprivClause.empty())
    return nullptr;

  LLVMContext &C = F->getContext();
  Module *M = F->getParent();

  Type *TaskDupParams[] = {PointerType::getUnqual(KmpTaskTTWithPrivatesTy),
                           PointerType::getUnqual(KmpTaskTTWithPrivatesTy),
                           Type::getInt32Ty(C)};
  FunctionType *TaskDupFnTy =
      FunctionType::get(Type::getVoidTy(C), TaskDupParams, false);

  Function *FnTaskDup =
      Function::Create(TaskDupFnTy, GlobalValue::InternalLinkage,
                       F->getName() + "_task_dup_" + Twine(W->getNumber()), M);
  FnTaskDup->setCallingConv(CallingConv::C);

  auto I = FnTaskDup->arg_begin();
  Value *Arg1 = &*I;
  I++;
  I++;
  Value *Arg3 = &*I;

  BasicBlock *EntryBB = BasicBlock::Create(C, "entry", FnTaskDup);

  DominatorTree DT;
  DT.recalculate(*FnTaskDup);

  IRBuilder<> Builder(EntryBB);

  SmallVector<Value *, 4> Indices;
  Indices.push_back(Builder.getInt32(0));
  Indices.push_back(Builder.getInt32(0));
  Value *BaseTaskTGep =
      Builder.CreateInBoundsGEP(KmpTaskTTWithPrivatesTy, Arg1, Indices);

  StructType *KmpTaskTTy =
      dyn_cast<StructType>(KmpTaskTTWithPrivatesTy->getElementType(0));

  Indices.clear();
  Indices.push_back(Builder.getInt32(0));
  Indices.push_back(Builder.getInt32(8));
  Value *LastIterGep =
      Builder.CreateInBoundsGEP(KmpTaskTTy, BaseTaskTGep, Indices);

  Builder.CreateStore(Arg3, LastIterGep);
  Builder.CreateRetVoid();

  return FnTaskDup;
}

// Generate the initialization code for task depend clauses. For each task
// depend clause, the compiler initializes the pointer, the size and the
// dependce type in the struct task_dep_info_type.
AllocaInst *
VPOParoptTransform::genDependInitForTask(WRegionNode *W,
                                         Instruction *InsertBefore) {
  if (!W->hasDepend())
    return nullptr;

  SmallVector<Type *, 4> KmpTaskTDependVecTyArgs;

  DependClause DepClause = W->getDepend();
  if (DepClause.empty())
    return nullptr;

  genKmpTaskDependInfo();

  LLVMContext &C = F->getContext();

  for (int I = 0; I < DepClause.size(); I++)
    KmpTaskTDependVecTyArgs.push_back(KmpTaskDependInfoTy);

  StructType *KmpTaskTDependVecTy =
      StructType::create(C,
                         makeArrayRef(KmpTaskTDependVecTyArgs.begin(),
                                      KmpTaskTDependVecTyArgs.end()),
                         "struct.kmp_task_depend_vec", false);

  IRBuilder<> Builder(InsertBefore);
  AllocaInst *DummyTaskTDependVec =
      Builder.CreateAlloca(KmpTaskTDependVecTy, nullptr, "task.depend.vec");

  const DataLayout DL = F->getParent()->getDataLayout();
  unsigned Count = 0;
  for (DependItem *DepI : DepClause.items()) {

    Value *BaseTaskTDependGep = Builder.CreateInBoundsGEP(
        KmpTaskTDependVecTy, DummyTaskTDependVec,
        {Builder.getInt32(0), Builder.getInt32(Count++)});

    Value *Gep =
        Builder.CreateInBoundsGEP(KmpTaskDependInfoTy, BaseTaskTDependGep,
                                  {Builder.getInt32(0), Builder.getInt32(0)});
    Builder.CreateStore(
        Builder.CreatePtrToInt(DepI->getOrig(),
                               DL.getIntPtrType(DepI->getOrig()->getType())),
        Gep);

    Gep = Builder.CreateInBoundsGEP(KmpTaskDependInfoTy, BaseTaskTDependGep,
                                    {Builder.getInt32(0), Builder.getInt32(1)});
    Builder.CreateStore(
        (DL.getIntPtrType(Builder.getInt8PtrTy())->getIntegerBitWidth() == 64)
            ? Builder.getInt64(DL.getTypeAllocSize(
                  DepI->getOrig()->getType()->getPointerElementType()))
            : Builder.getInt32(DL.getTypeAllocSize(
                  DepI->getOrig()->getType()->getPointerElementType())),
        Gep);

    Gep = Builder.CreateInBoundsGEP(KmpTaskDependInfoTy, BaseTaskTDependGep,
                                    {Builder.getInt32(0), Builder.getInt32(2)});
    Builder.CreateStore(Builder.getInt8(DepI->getIsIn() ? 0x1 : 0x3), Gep);
  }

  return DummyTaskTDependVec;
}
// Generate the call __kmpc_task_reduction_init and the corresponding
// preparation.
void VPOParoptTransform::genRedInitForTaskLoop(WRegionNode *W,
                                               Instruction *InsertBefore) {

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genRedInitForTaskLoop\n");

  genTaskTRedType();

  SmallVector<Type *, 4> KmpTaskTRedRecTyArgs;

  ReductionClause &RedClause = W->getRed();
  if (RedClause.empty())
    return;
  LLVMContext &C = F->getContext();

  for (int I = 0; I < RedClause.size(); I++)
    KmpTaskTRedRecTyArgs.push_back(KmpTaskTRedTy);

  StructType *KmpTaskTTRedRecTy = StructType::create(
      C, makeArrayRef(KmpTaskTRedRecTyArgs.begin(), KmpTaskTRedRecTyArgs.end()),
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

bool VPOParoptTransform::genTaskCode(WRegionNode *W,
                                     StructType *KmpTaskTTWithPrivatesTy,
                                     StructType *KmpSharedTy) {
  return genTaskGenericCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy, nullptr,
                            nullptr, nullptr, false);
}

// Set the the arguements in the if clause to be empty.
void VPOParoptTransform::resetValueInTaskIfClause(WRegionNode *W) {
  Value *V = W->getIf();
  if (!V)
    return;
  resetValueInIntelClauseGeneric(W, V);
}

// Set the the arguments in the depend clause to be empty.
void VPOParoptTransform::resetValueInTaskDependClause(WRegionNode *W) {
  if (!W->hasDepend())
    return;

  DependClause DepClause = W->getDepend();
  if (DepClause.empty())
    return;
  for (DependItem *DepI : DepClause.items()) {
    resetValueInIntelClauseGeneric(W, DepI->getOrig());
  }
}

// Set the the arguements in the Intel compiler generated clause to be empty.
void VPOParoptTransform::resetValueInIntelClauseGeneric(WRegionNode *W,
                                                        Value *V) {
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

// The wrapper routine to generate the call __kmpc_omp_task_with_deps
void VPOParoptTransform::genTaskDeps(WRegionNode *W, StructType *IdentTy,
                                     Value *TidPtr, Value *TaskAlloc,
                                     AllocaInst *DummyTaskTDependRec,
                                     Instruction *InsertPt, bool IsTaskWait) {
  IRBuilder<> Builder(InsertPt);

  Value *BaseTaskTDependGep = Builder.CreateInBoundsGEP(
      DummyTaskTDependRec->getAllocatedType(), DummyTaskTDependRec,
      {Builder.getInt32(0), Builder.getInt32(0)});
  LLVMContext &C = F->getContext();
  Value *Dep = Builder.CreateBitCast(BaseTaskTDependGep, Type::getInt8PtrTy(C));
  if (!IsTaskWait)
    VPOParoptUtils::genKmpcTaskWithDeps(W, IdentTy, TidPtr, TaskAlloc, Dep,
                                        W->getDepend().size(), InsertPt);
  else
    VPOParoptUtils::genKmpcTaskWaitDeps(W, IdentTy, TidPtr, Dep,
                                        W->getDepend().size(), InsertPt);
}

// Generate the call __kmpc_omp_task_alloc, __kmpc_taskloop or
// __kmpc_omp_task and the corresponding outlined function
//
// Based on the presence/absence of the IF and the DEPEND clauses,
// there are four different codegen for tasks:
//
// 1. "IF(expr)" absent, "DEPEND" absent:
//
//    __kmpc_omp_task(&loc, GTID, thunk_temp)
//
//
// 2. "IF(expr)" absent, "DEPEND" present:
//
//     __kmpc_omp_task_with_deps(&loc,GTID,thunk_temp, n,dep_list, 0,0)
//
//
// 3. "IF(expr)" present, "DEPEND" absent:
//
//  if (expr != 0) {
//    __kmpc_omp_task(&loc, GTID, thunk_temp)
//  } else {
//    __kmpc_omp_task_begin_if0(&loc, GTID, thunk_temp)
//    call task_outline_func(GTID, thunk_temp)
//    __kmpc_omp_task_complete_if0(&loc, GTID, thunk_temp)
//  }
//
//
// 4. "IF(expr)" present, "DEPEND" present:
//
//  if (expr != 0) {
//    __kmpc_omp_task_with_deps(&loc,GTID,thunk_temp, n,dep_list, 0,0)
//  } else {
//    __kmpc_omp_wait_deps(&loc,GTID, n,dep_list, 0,0)
//    __kmpc_omp_task_begin_if0(&loc, GTID, thunk_temp)
//    call task_outline_func(GTID, thunk_temp)
//    __kmpc_omp_task_complete_if0(&loc, GTID, thunk_temp)
//  }
//
bool VPOParoptTransform::genTaskGenericCode(WRegionNode *W,
                                            StructType *KmpTaskTTWithPrivatesTy,
                                            StructType *KmpSharedTy,
                                            Value *LBPtr, Value *UBPtr,
                                            Value *STPtr, bool isLoop) {

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTaskGenericCode\n");

  assert(W->isBBSetEmpty() && "genTaskGenericCode: BBSET should start empty");

  W->populateBBSet();

  codeExtractorPrepare(W);

  resetValueInTaskIfClause(W);

  resetValueInTaskDependClause(W);

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


    if (!NewCall->use_empty())
      NewCall->replaceAllUsesWith(MTFnCI);

    // Keep the orginal extraced function name after finalization
    MTFnCI->takeName(NewCall);

    genRedInitForTaskLoop(W, NewCall);

    AllocaInst *DummyTaskTDependRec = genDependInitForTask(W, NewCall);

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

    IRBuilder<> Builder(NewCall);

    Value *VIf = W->getIf();
    Value *Cmp = nullptr;
    if (VIf)
      Cmp = Builder.CreateICmpNE(VIf, ConstantInt::get(VIf->getType(), 0));
    if (isLoop) {
      VPOParoptUtils::genKmpcTaskLoop(
          W, IdentTy, TidPtr, TaskAllocCI, Cmp, LBPtr, UBPtr, STPtr,
          KmpTaskTTWithPrivatesTy, NewCall, Mode & OmpTbb,
          genLastPrivateTaskDup(W, KmpTaskTTWithPrivatesTy));
    } else {
      if (!VIf) {
        if (!DummyTaskTDependRec)
          VPOParoptUtils::genKmpcTask(W, IdentTy, TidPtr, TaskAllocCI, NewCall);
        else
          genTaskDeps(W, IdentTy, TidPtr, TaskAllocCI, DummyTaskTDependRec,
                      NewCall, false);
      } else {
        BasicBlock *SplitBeforeBB = NewCall->getParent();
        TerminatorInst *ThenTerm, *ElseTerm;
        SplitBlockAndInsertIfThenElse(Cmp, NewCall, &ThenTerm, &ElseTerm);
        ThenTerm->getParent()->setName("task_if.then");
        ElseTerm->getParent()->setName("task_if.else");
        NewCall->getParent()->setName("task_if.end");

        DT->addNewBlock(ThenTerm->getParent(), SplitBeforeBB);
        DT->addNewBlock(ElseTerm->getParent(), SplitBeforeBB);
        DT->addNewBlock(NewCall->getParent(), SplitBeforeBB);

        DT->changeImmediateDominator(ThenTerm->getParent(), SplitBeforeBB);
        DT->changeImmediateDominator(ElseTerm->getParent(), SplitBeforeBB);
        DT->changeImmediateDominator(NewCall->getParent(), SplitBeforeBB);
        BasicBlock *NextBB = NewCall->getParent()->getSingleSuccessor();
        DT->changeImmediateDominator(NextBB, NewCall->getParent());

        IRBuilder<> ElseBuilder(ElseTerm);
        if (!DummyTaskTDependRec)
          VPOParoptUtils::VPOParoptUtils::genKmpcTask(W, IdentTy, TidPtr,
                                                      TaskAllocCI, ThenTerm);
        else
          genTaskDeps(W, IdentTy, TidPtr, TaskAllocCI, DummyTaskTDependRec,
                      ThenTerm, false);

        genTaskDeps(W, IdentTy, TidPtr, TaskAllocCI, DummyTaskTDependRec,
                    ElseTerm, true);
        VPOParoptUtils::genKmpcTaskBeginIf0(W, IdentTy, TidPtr, TaskAllocCI,
                                            ElseTerm);
        MTFnArgs.clear();
        MTFnArgs.push_back(ElseBuilder.CreateLoad(TidPtr));
        MTFnArgs.push_back(ElseBuilder.CreateBitCast(
            TaskAllocCI, PointerType::getUnqual(KmpTaskTTWithPrivatesTy)));
        CallInst *SeqCI = CallInst::Create(MTFn, MTFnArgs, "", ElseTerm);
        SeqCI->setCallingConv(CS.getCallingConv());
        SeqCI->takeName(NewCall);
        VPOParoptUtils::genKmpcTaskCompleteIf0(W, IdentTy, TidPtr, TaskAllocCI,
                                               ElseTerm);
      }
    }

    NewCall->eraseFromParent();
    NewF->eraseFromParent();
    MTFnCI->eraseFromParent();

    W->resetBBSet(); // Invalidate BBSet after transformations

    Changed = true;
  }
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genTaskGenericCode\n");
  return Changed;
}

bool VPOParoptTransform::genTaskWaitCode(WRegionNode *W) {
  VPOParoptUtils::genKmpcTaskWait(W, IdentTy, TidPtr,
                                  W->getEntryBBlock()->getTerminator());
  return true;
}
