#if INTEL_COLLAB
//===------ VPOParoptTask.cpp - Transformation of WRegion for tasking -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// VPOParoptTask.cpp implements the omp task and taskloop feature.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"

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

#include "llvm/PassAnalysisSupport.h"

#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"

#include "llvm/Transforms/Utils/GeneralUtils.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-task"

// Replace the reduction variable reference with the dereference of
// the return pointer __kmpc_task_reduction_get_th_data. It handles
// both reduction and inreduction clause.
bool VPOParoptTransform::genRedCodeForTaskGeneric(WRegionNode *W) {

  bool Changed = false;

  LLVM_DEBUG(
      dbgs() << "\nEnter VPOParoptTransform::genRedCodeForTaskGeneric\n");

  // Utility to replace the reduction variable specified in
  // the reduction/inreduction clause with the return value of
  // function __kmpc_task_reduction_get_th_data.
  auto replaceReductionVarInTask = [&](WRegionNode *W,
                                       ReductionClause &RedClause) {
    W->populateBBSet();
    BasicBlock *EntryBB = W->getEntryBBlock();
    BasicBlock *ExitBB = W->getExitBBlock();

    for (ReductionItem *RedI : RedClause.items()) {

      Value *Orig = RedI->getOrig();

      if (isa<GlobalVariable>(Orig) || isa<AllocaInst>(Orig)) {
        Instruction *AllocaInsertPt = EntryBB->getFirstNonPHI();
        auto *NewPrivInst =
            genPrivatizationAlloca(RedI, AllocaInsertPt, ".red");
        genPrivatizationReplacement(W, Orig, NewPrivInst);

        IRBuilder<> Builder(EntryBB->getTerminator());
        VPOParoptUtils::genCopyByAddr(NewPrivInst, RedI->getNew(),
                                      EntryBB->getTerminator());
        VPOParoptUtils::genCopyByAddr(RedI->getNew(), NewPrivInst,
                                      ExitBB->getTerminator());
      }
    }
  };

  if (W->canHaveReduction()) {
    ReductionClause &RedClause = W->getRed();
    if (!RedClause.empty()) {
      replaceReductionVarInTask(W, RedClause);
      Changed = true;
      W->resetBBSet(); // Invalidate BBSet after transformations
    }
  }

  if (W->canHaveInReduction()) {
    ReductionClause &RedClause = W->getInRed();
    if (!RedClause.empty()) {
      replaceReductionVarInTask(W, RedClause);
      Changed = true;
      W->resetBBSet(); // Invalidate BBSet after transformations
    }
  }
  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genRedCodeForTaskGeneric\n");
  return Changed;
}

// Generate the destructor thunk for task firstprivate data.
// int32_t destr(int32_t gtid, char *taskThunk)
// This thunk is stored in the kmp_task_t structure which is passed to the
// task/taskloop creation call.
//
// This is required if a firstprivate object has a destructor, and:
// 1) The task executes a cancellation, *or*
// 2) It is a taskloop construct.
// Currently the thunk is passed to all tasks, so the firstprivates and their
// copies are always destructed from a destructor thunk.
Function *VPOParoptTransform::genTaskDestructorThunk(
    WRegionNode *W, StructType *KmpTaskTTWithPrivatesTy) {
  // First check if we need this thunk.
  if (!W->canHaveFirstprivate())
    return nullptr;
  bool hasDestr = false;
  for (FirstprivateItem *FI : W->getFpriv().items())
    if (FI->getDestructor()) {
      hasDestr = true;
      break;
    }
  if (!hasDestr)
    return nullptr;

  LLVMContext &C = F->getContext();
  Module *M = F->getParent();
  Type *DestThunkParams[] = {Type::getIntNTy(C, 32),
                             PointerType::getUnqual(Type::getIntNTy(C, 8))};
  FunctionType *DestThunkTy =
      FunctionType::get(Type::getIntNTy(C, 32), DestThunkParams, false);
  Function *DestThunk = Function::Create(
      DestThunkTy, GlobalValue::InternalLinkage,
      F->getName() + "_dtor_thunk_" + Twine(W->getNumber()), M);
  DestThunk->setCallingConv(CallingConv::C);
  BasicBlock *EntryBB = BasicBlock::Create(C, "entry", DestThunk);

  DominatorTree DT;
  DT.recalculate(*DestThunk);
  auto *ArgIter = DestThunk->arg_begin();
  // 2nd arg is the pointer to the task thunk.
  ArgIter++;
  IRBuilder<> Builder(EntryBB);

  Type *TaskThunkPtrType = PointerType::getUnqual(KmpTaskTTWithPrivatesTy);
  // The task thunk was passed in as char*. Cast it to the task thunk type.
  auto *TaskThunkPtr = Builder.CreateBitCast(&*ArgIter, TaskThunkPtrType);

  // Create the block terminator. This return value is unused.
  auto *RetInst = Builder.CreateRet(Builder.getInt32(0));

  // Insert the next sequences before the return.
  Builder.SetInsertPoint(RetInst);

  // Now call all the destructors on all firstprivate objects.
  SmallVector<Value *, 4> Indices;
  for (FirstprivateItem *FI : W->getFpriv().items()) {
    if (!FI->getDestructor())
      continue;
    // The firstprivate objects are stored whole, in the private area of the
    // task thunk.
    // %__struct.kmp_task_t_with_privates =
    //     type { %__struct.kmp_task_t, %__struct.kmp_privates.t }
    // %__struct.kmp_privates.t = type { %class.foo, %class.foo, i64, i32 }
    // Each FI is represented by a GEP from the start of the private area.
    // This was set up by firstprivate codegen.
    // We have a kmp_task_t_with_privates, so first GEP to kmp_privates_t
    // and then add the FI's GEP index.
    //
    // If firstprivate codegen is later changed to use local vars instead
    // of thunk references, we will need to find the indices a different way.
    assert(isa<GetElementPtrInst>(FI->getNew()) &&
      "Firstprivate item must have a GEP pointing to local storage");

    auto *FIGEP = cast<GetElementPtrInst>(FI->getNew());
    // a struct GEP should have 2 indices: 0 then the real index
    assert(FIGEP->getNumIndices() == 2);
    Indices.clear();
    // 0 == struct reference
    Indices.push_back(Builder.getInt32(0));
    // 1 == index of private area in kmp_task_t_with_privates
    Indices.push_back(Builder.getInt32(1));
    // GEP index from FI
    Indices.push_back(FIGEP->getOperand(2));
    Value *DestrGEP = Builder.CreateInBoundsGEP(KmpTaskTTWithPrivatesTy,
                                                TaskThunkPtr, Indices);
    // Call the destructor. Insert the call before the return inst.
    // The gencall will verify that the GEP type matches the destructor
    // parameter.
    auto *DestrCall =
        VPOParoptUtils::genDestructorCall(FI->getDestructor(), DestrGEP, RetInst);

    // Move the builder before the destructor call.
    Builder.SetInsertPoint(DestrCall);
  }

  return DestThunk;
}

// Generate the code to update the last privates for taskloop.
void VPOParoptTransform::genLprivFiniForTaskLoop(LastprivateItem *LprivI,
                                                 Instruction *InsertPt) {

  Value *Src = LprivI->getNew();
  Value *Dst = LprivI->getOrigGEP();
  if (LprivI->getIsByRef())
    Dst = new LoadInst(Dst, "", InsertPt);

  GetElementPtrInst *Gep = dyn_cast<GetElementPtrInst>(Src);
  Type *ScalarTy = Gep->getResultElementType();
  const DataLayout &DL = InsertPt->getModule()->getDataLayout();

  IRBuilder<> Builder(InsertPt);
  if (!VPOUtils::canBeRegisterized(ScalarTy, DL))
    VPOUtils::genMemcpy(Dst, Src, DL, DL.getABITypeAlignment(ScalarTy),
                        InsertPt->getParent());
  else {
    LoadInst *Load = Builder.CreateLoad(Src);
    Builder.CreateStore(Load, Dst);
  }
}

// Replace the shared variable reference with the thunk field
// derefernce
bool VPOParoptTransform::genSharedCodeForTaskGeneric(WRegionNode *W) {

  bool Changed = false;

  LLVM_DEBUG(
      dbgs() << "\nEnter VPOParoptTransform::genSharedCodeForTaskGeneric\n");

  SharedClause &ShaClause = W->getShared();
  if (!ShaClause.empty()) {

    W->populateBBSet();

    for (SharedItem *ShaI : ShaClause.items()) {

      Value *Orig = ShaI->getOrig();
      resetValueInOmpClauseGeneric(W, Orig);

      Value *NewPrivInst = nullptr;
      NewPrivInst = ShaI->getNew();
      genPrivatizationReplacement(W, Orig, NewPrivInst);
    }

    Changed = true;
  } // if

  LLVM_DEBUG(
      dbgs() << "\nExit VPOParoptTransform::genSharedCodeForTaskGeneric\n");

  W->resetBBSetIfChanged(Changed); // Clear BBSet if transformed
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
  PointerType *Int8PtrTy = Type::getInt8PtrTy(C);


  KmpTaskTRedTy = VPOParoptUtils::getOrCreateStructType(
      F, "__struct.kmp_task_t_red_item",
      {Int8PtrTy, Int64Ty, Int8PtrTy, Int8PtrTy, Int8PtrTy, Int32Ty});
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

  KmpTaskDependInfoTy = VPOParoptUtils::getOrCreateStructType(
      F, "__struct.kmp_depend_info", KmpTaskDependTyArgs);
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
  StructType *KmpCmplrdataTy = VPOParoptUtils::getOrCreateStructType(
      F, "__union.kmp_cmplrdata_t", KmpCmplrdataTyArgs);

  Type *KmpTaskTyArgs[] = {Type::getInt8PtrTy(C),
                           KmpRoutineEntryPtrTy,
                           Int32Ty,
                           KmpCmplrdataTy,
                           KmpCmplrdataTy,
                           Int64Ty,
                           Int64Ty,
                           Int64Ty,
                           Int32Ty};

  KmpTaskTTy = VPOParoptUtils::getOrCreateStructType(F, "__struct.kmp_task_t",
                                                     KmpTaskTyArgs);
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

  SmallVector<Type *, 4> PrivateThunkTypes;
  SmallVector<Type *, 4> SharedThunkTypes;

  unsigned PrivateCount = 0;
  unsigned SharedCount = 0;

  auto getDataTypeForItemInPrivateThunk = [](Item *I) -> Type * {
    Type *ElementTy = nullptr;
    Value *NumElements = nullptr;
    unsigned AddrSpace = 0;
    getItemInfo(I, ElementTy, NumElements, AddrSpace);
    if (!NumElements)
      return ElementTy;

    assert(isa<ConstantInt>(NumElements) &&
           "genKmpTaskTWithPrivatesRecordDecl: VLAs are not supported.");
    uint64_t NE = cast<ConstantInt>(NumElements)->getZExtValue();
    return ArrayType::get(ElementTy, NE);
  };

  FirstprivateClause &FprivClause = W->getFpriv();
  if (!FprivClause.empty()) {
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      Type *LocalDataType = getDataTypeForItemInPrivateThunk(FprivI);
      PrivateThunkTypes.push_back(LocalDataType);
      FprivI->setPrivateThunkIdx(PrivateCount++);
    }
  }

  if (W->canHaveLastprivate()) {
    LastprivateClause &LprivClause = W->getLpriv();
    if (!LprivClause.empty()) {
      for (LastprivateItem *LprivI : LprivClause.items()) {
        if (FirstprivateItem *FprivI = LprivI->getInFirstprivate())
          LprivI->setPrivateThunkIdx(FprivI->getPrivateThunkIdx());
        else {
          Type *LocalDataType = getDataTypeForItemInPrivateThunk(LprivI);
          PrivateThunkTypes.push_back(LocalDataType);
          LprivI->setPrivateThunkIdx(PrivateCount++);
        }

        auto PT = dyn_cast<PointerType>(LprivI->getOrig()->getType());
        assert(PT && "genKmpTaskTWithPrivatesRecordDecl: Expect last private "
                     "pointer argument");
        SharedThunkTypes.push_back(PT);
        LprivI->setSharedThunkIdx(SharedCount++);
      }
    }
  }

  PrivateClause &PrivClause = W->getPriv();
  if (!PrivClause.empty()) {
    for (PrivateItem *PrivI : PrivClause.items()) {
      Type *LocalDataType = getDataTypeForItemInPrivateThunk(PrivI);
      PrivateThunkTypes.push_back(LocalDataType);
      PrivI->setPrivateThunkIdx(PrivateCount++);
    }
  }

  // Utility to add fields in the thunk for the reduction variables.
  auto AddRedVarInThunk = [](ReductionClause &RedClause,
                              SmallVectorImpl<Type *> &SharedThunkTypes,
                              unsigned &SharedCount) {
    if (!RedClause.empty()) {
      for (ReductionItem *RedI : RedClause.items()) {
        Value *Orig = RedI->getOrig();
        auto PT = dyn_cast<PointerType>(Orig->getType());
        assert(PT && "genKmpTaskTWithPrivatesRecordDecl: Expect reduction "
                     "pointer argument");
        SharedThunkTypes.push_back(PT);
        RedI->setSharedThunkIdx(SharedCount++);
      }
    }
  };

  if (W->canHaveReduction()) {
    ReductionClause &RedClause = W->getRed();
    AddRedVarInThunk(RedClause, SharedThunkTypes, SharedCount);
  }

  if (W->canHaveInReduction()) {
    ReductionClause &RedClause = W->getInRed();
    AddRedVarInThunk(RedClause, SharedThunkTypes, SharedCount);
  }

  SharedClause &ShaClause = W->getShared();
  if (!ShaClause.empty()) {
    for (SharedItem *ShaI : ShaClause.items()) {
      Value *Orig = ShaI->getOrig();
      auto PT = dyn_cast<PointerType>(Orig->getType());
      assert(PT && "genKmpTaskTWithPrivatesRecordDecl: Expect shared "
                   "pointer argument");
      SharedThunkTypes.push_back(PT);
      ShaI->setSharedThunkIdx(SharedCount++);
    }
  }

  KmpPrivatesTy = StructType::create(C, PrivateThunkTypes,
                                     "__struct.kmp_privates.t", false);

  KmpSharedTy =
      StructType::create(C, SharedThunkTypes, "__struct.shared.t", false);

  KmpTaksTWithPrivatesTyArgs.push_back(KmpPrivatesTy);

  StructType *KmpTaskTTWithPrivatesTy =
      StructType::create(C,
                         makeArrayRef(KmpTaksTWithPrivatesTyArgs.begin(),
                                      KmpTaksTWithPrivatesTyArgs.end()),
                         "__struct.kmp_task_t_with_privates", false);

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Task thunk type for privates: '"
                    << *KmpPrivatesTy << "', shareds: '" << *KmpSharedTy
                    << "'.\n");
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

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTaskLoopInitCode\n");

  Loop *L;
  if (isLoop) {
    L = W->getWRNLoopInfo().getLoop();
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
  Value *Zero = Builder.getInt32(0);

  AllocaInst *DummyTaskTWithPrivates = Builder.CreateAlloca(
      KmpTaskTTWithPrivatesTy, nullptr, "taskt.withprivates");

  Builder.SetInsertPoint(W->getEntryBBlock()->getTerminator());
  Value *BaseTaskTGep = Builder.CreateInBoundsGEP(
      KmpTaskTTWithPrivatesTy, DummyTaskTWithPrivates, {Zero, Zero});
  // Value *PartIdGep = Builder.CreateInBoundsGEP(
  //     KmpTaskTTy, BaseTaskTGep, {Zero, Builder.getInt32(2)}, ".part.id");

  Value *SharedGep =
      Builder.CreateInBoundsGEP(KmpTaskTTy, BaseTaskTGep, {Zero, Zero});
  Value *SharedLoad = Builder.CreateLoad(SharedGep);
  Value *SharedCast = Builder.CreateBitCast(
      SharedLoad, PointerType::getUnqual(KmpSharedTy), ".shareds");

  Value *PrivatesGep =
      Builder.CreateInBoundsGEP(KmpTaskTTWithPrivatesTy, DummyTaskTWithPrivates,
                                {Zero, Builder.getInt32(1)}, ".privates");

  Value *LowerBoundGep = Builder.CreateInBoundsGEP(
      KmpTaskTTy, BaseTaskTGep, {Zero, Builder.getInt32(5)}, ".lb.gep");
  Value *LowerBoundLd = Builder.CreateLoad(LowerBoundGep, ".lb");

  Value *UpperBoundGep = Builder.CreateInBoundsGEP(
      KmpTaskTTy, BaseTaskTGep, {Zero, Builder.getInt32(6)}, ".ub.gep");
  Value *UpperBoundLd = Builder.CreateLoad(UpperBoundGep, ".ub");

  /*The stride does not need to change since the loop is normalized by the clang
  Value *StrideGep = Builder.CreateInBoundsGEP(
      KmpTaskTTy, BaseTaskTGep, {Zero, Builder.getInt32(7)}, ".stride.gep");
  Value *StrideLd = Builder.CreateLoad(StrideGep, ".stride");
  */

  LastIterGep = Builder.CreateInBoundsGEP(
      KmpTaskTTy, BaseTaskTGep, {Zero, Builder.getInt32(8)}, ".last.iter.gep");

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
    VPOParoptUtils::updateOmpPredicateAndUpperBound(W, 0, UpperBoundLd,
                                                    &*Builder.GetInsertPoint());
  }
  PrivateClause &PrivClause = W->getPriv();
  if (!PrivClause.empty()) {
    for (PrivateItem *PrivI : PrivClause.items()) {
      Value *ThunkPrivatesGep = Builder.CreateInBoundsGEP(
          KmpPrivatesTy, PrivatesGep,
          {Zero, Builder.getInt32(PrivI->getPrivateThunkIdx())},
          PrivI->getOrig()->getName() + ".gep");
      PrivI->setNew(ThunkPrivatesGep);
    }
  }

  FirstprivateClause &FprivClause = W->getFpriv();
  if (!FprivClause.empty()) {
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      Value *ThunkPrivatesGep = Builder.CreateInBoundsGEP(
          KmpPrivatesTy, PrivatesGep,
          {Zero, Builder.getInt32(FprivI->getPrivateThunkIdx())},
          FprivI->getOrig()->getName() + ".gep");
      FprivI->setNew(ThunkPrivatesGep);
    }
  }

  if (W->canHaveLastprivate()) {
    LastprivateClause &LprivClause = W->getLpriv();
    if (!LprivClause.empty()) {
      for (LastprivateItem *LprivI : LprivClause.items()) {
        StringRef OrigName = LprivI->getOrig()->getName();
        if (FirstprivateItem *FprivI = LprivI->getInFirstprivate())
          LprivI->setNew(FprivI->getNew());
        else {
          Value *ThunkPrivatesGep = Builder.CreateInBoundsGEP(
              KmpPrivatesTy, PrivatesGep,
              {Zero, Builder.getInt32(LprivI->getPrivateThunkIdx())},
              OrigName + ".gep");
          LprivI->setNew(ThunkPrivatesGep);
        }
        Value *ThunkSharedGep = Builder.CreateInBoundsGEP(
            KmpSharedTy, SharedCast,
            {Zero, Builder.getInt32(LprivI->getSharedThunkIdx())},
            OrigName + ".shr.gep");
        Value *ThunkSharedVal =
            Builder.CreateLoad(ThunkSharedGep, OrigName + ".shr");
        // Parm is used to record the address of last private in the compiler
        // shared variables in the thunk.
        LprivI->setOrigGEP(ThunkSharedVal);
      }
    }
  }

  // Utility to generate the new reference from the call KmpcRedGetNthData
  // for the reduction variables.
  auto GenRefForRedVarsInTask = [&] (ReductionClause &RedClause,
                                     IRBuilder<> &Builder) {
    if (!RedClause.empty()) {
      for (ReductionItem *RedI : RedClause.items()) {
        StringRef OrigName = RedI->getOrig()->getName();
        Value *ThunkSharedGep = Builder.CreateInBoundsGEP(
            KmpSharedTy, SharedCast,
            {Zero, Builder.getInt32(RedI->getSharedThunkIdx())},
            OrigName + ".shr.gep");
        Value *ThunkSharedVal =
            Builder.CreateLoad(ThunkSharedGep, OrigName + ".shr");
        Value *RedRes = VPOParoptUtils::genKmpcRedGetNthData(
            W, TidPtrHolder, ThunkSharedVal, &*Builder.GetInsertPoint(),
            Mode & OmpTbb);
        RedRes->setName(OrigName + ".red");
        Value *RedResCast = Builder.CreateBitCast(
            RedRes, RedI->getOrig()->getType(), OrigName + ".red.cast");
        RedI->setNew(RedResCast);
      }
    }
  };

  if (W->canHaveReduction()) {
    ReductionClause &RedClause = W->getRed();
    GenRefForRedVarsInTask(RedClause, Builder);
  }

  if (W->canHaveInReduction()) {
    ReductionClause &RedClause = W->getInRed();
    GenRefForRedVarsInTask(RedClause, Builder);
  }

  SharedClause &ShaClause = W->getShared();
  if (!ShaClause.empty()) {
    for (SharedItem *ShaI : ShaClause.items()) {
      StringRef OrigName = ShaI->getOrig()->getName();
      Value *ThunkSharedGep = Builder.CreateInBoundsGEP(
          KmpSharedTy, SharedCast,
          {Zero, Builder.getInt32(ShaI->getSharedThunkIdx())},
          OrigName + ".shr.gep");
      Value *ThunkSharedVal =
          Builder.CreateLoad(ThunkSharedGep, OrigName + ".shr");
      ShaI->setNew(ThunkSharedVal);
    }
  }

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genTaskLoopInitCode\n");

  W->resetBBSet(); // CFG changed; clear BBSet
  return true;
}

// Create a struct to contain all shared data for the task. This is allocated in
// the caller of the task, and is populated with pointers to shared variables,
// reduction variables, and lastprivate variables.
AllocaInst *
VPOParoptTransform::genAndPopulateTaskSharedStruct(WRegionNode *W,
                                                   StructType *KmpSharedTy) {

  BasicBlock *EntryPredecessor = W->getEntryBBlock()->getSinglePredecessor();
  assert(EntryPredecessor && "Single pre-task block not created.");

  // Split the predecessor BBlock before inserting code in it. This is
  // needed because the predecessor itself might be an entry/exit BBlock
  // of another WRegion, in which case inserting code in it would deform
  // that WRegion.
  EntryPredecessor = SplitBlock(EntryPredecessor,
                                &*(EntryPredecessor->getTerminator()), DT, LI);
  Instruction *InsertPt = EntryPredecessor->getTerminator();
  IRBuilder<> Builder(InsertPt);
  Value *Zero = Builder.getInt32(0);
  AllocaInst *TaskSharedBase =
      Builder.CreateAlloca(KmpSharedTy, nullptr, "taskt.shared.agg");

  if (W->canHaveLastprivate()) {
    LastprivateClause &LprivClause = W->getLpriv();
    if (!LprivClause.empty()) {
      for (LastprivateItem *LprivI : LprivClause.items()) {
        Value *Gep = Builder.CreateInBoundsGEP(
            KmpSharedTy, TaskSharedBase,
            {Zero, Builder.getInt32(LprivI->getSharedThunkIdx())},
            LprivI->getOrig()->getName() + ".shr.gep");
        // storing pointer value to task base
        Builder.CreateStore(LprivI->getOrig(), Gep);
      }
    }
  }

  // Utility to generate the gep instructions for the reduction variables.
  auto GenGepForRedVarsInTask = [&](ReductionClause &RedClause,
                                    IRBuilder<> &Builder) {
    if (!RedClause.empty()) {
      for (ReductionItem *RedI : RedClause.items()) {
        Value *Gep = Builder.CreateInBoundsGEP(
            KmpSharedTy, TaskSharedBase,
            {Zero, Builder.getInt32(RedI->getSharedThunkIdx())},
            RedI->getOrig()->getName() + ".shr.gep");
        Builder.CreateStore(RedI->getOrig(), Gep);
      }
    }
  };

  if (W->canHaveReduction()) {
    ReductionClause &RedClause = W->getRed();
    GenGepForRedVarsInTask(RedClause, Builder);
  }

  if (W->canHaveInReduction()) {
    ReductionClause &RedClause = W->getInRed();
    GenGepForRedVarsInTask(RedClause, Builder);
  }

  SharedClause &ShaClause = W->getShared();
  if (!ShaClause.empty()) {
    for (SharedItem *ShaI : ShaClause.items()) {
      Value *Gep = Builder.CreateInBoundsGEP(
          KmpSharedTy, TaskSharedBase,
          {Zero, Builder.getInt32(ShaI->getSharedThunkIdx())},
          ShaI->getOrig()->getName() + ".shr.gep");
      Builder.CreateStore(ShaI->getOrig(), Gep);
    }
  }

  return TaskSharedBase;
}

// Initialize the data in the shared data area inside the thunk.
// Also used for tasks.
void VPOParoptTransform::copySharedStructToTaskThunk(
    WRegionNode *W, AllocaInst *Src, Value *Dst, StructType *KmpSharedTy,
    StructType *KmpTaskTTWithPrivatesTy, Function *DestrThunk,
    Instruction *InsertPt) {

  if (KmpSharedTy->getNumElements() == 0 && !DestrThunk)
    return;

  IRBuilder<> Builder(InsertPt);
  Value *Zero = Builder.getInt32(0);

  Value *Cast = Builder.CreateBitCast(
      Dst, PointerType::getUnqual(KmpTaskTTWithPrivatesTy),
      ".taskt.with.privates");

  Value *TaskTTyGep = Builder.CreateInBoundsGEP(KmpTaskTTWithPrivatesTy, Cast,
                                                {Zero, Zero}, ".taskt");

  StructType *KmpTaskTTy =
      dyn_cast<StructType>(KmpTaskTTWithPrivatesTy->getElementType(0));

  if (KmpSharedTy->getNumElements()) {
    Value *SharedTyGep = Builder.CreateInBoundsGEP(KmpTaskTTy, TaskTTyGep,
                                                   {Zero, Zero}, ".sharedptr");

    Value *LI = Builder.CreateLoad(SharedTyGep, ".shareds");

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

    unsigned Align = DL.getABITypeAlignment(Src->getAllocatedType());
    Builder.CreateMemCpy(LI, Align, SrcCast, Align, Size);
  }

  if (!DestrThunk)
    return;

  Value *DestrGep = Builder.CreateInBoundsGEP(
      KmpTaskTTy, TaskTTyGep,
      {Zero, Builder.getInt32(3) /* cmplrdate_t */, Zero /* destructor */},
      ".destr.gep");
  Builder.CreateStore(DestrThunk, DestrGep);
}

void VPOParoptTransform::genFprivInitForTask(
    WRegionNode *W, Value *KmpTaskTTWithPrivates,
    StructType *KmpTaskTTWithPrivatesTy, Instruction *InsertPt) {

  FirstprivateClause &FprivClause = W->getFpriv();
  if (FprivClause.empty())
    return;

  IRBuilder<> Builder(InsertPt);
  Value *Zero = Builder.getInt32(0);
  Value *Cast = Builder.CreateBitCast(
      KmpTaskTTWithPrivates, PointerType::getUnqual(KmpTaskTTWithPrivatesTy),
      ".taskt.with.privates");

  Value *PrivatesGep = Builder.CreateInBoundsGEP(
      KmpTaskTTWithPrivatesTy, Cast, {Zero, Builder.getInt32(1)}, ".privates");

  StructType *KmpPrivatesTy =
      dyn_cast<StructType>(KmpTaskTTWithPrivatesTy->getElementType(1));

  for (FirstprivateItem *FprivI : FprivClause.items()) {
    Value *OrigV = FprivI->getOrig();
    StringRef NamePrefix = OrigV->getName();

    Value *NewVGep = Builder.CreateInBoundsGEP(
        KmpPrivatesTy, PrivatesGep,
        {Zero, Builder.getInt32(FprivI->getPrivateThunkIdx())},
        NamePrefix + ".priv.gep");

    VPOParoptUtils::genCopyByAddr(NewVGep, OrigV, InsertPt,
                                  FprivI->getCopyConstructor(),
                                  FprivI->getIsByRef());
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
  Loop *L = W->getWRNLoopInfo().getLoop();
  Type *IndValTy = WRegionUtils::getOmpCanonicalInductionVariable(L)
                       ->getIncomingValue(0)
                       ->getType();

  AllocaInst *LowerBnd = Builder.CreateAlloca(IndValTy, nullptr, "lower.bnd");
  Value *InitVal = WRegionUtils::getOmpLoopLowerBound(L);

  InitVal = VPOParoptUtils::cloneInstructions(InitVal, &*(EntryBB->begin()));
  assert(InitVal &&
          "genLoopInitCodeForTaskLoop: Expect non-empty loop lower bound");

  if (InitVal->getType()->getIntegerBitWidth() !=
      IndValTy->getIntegerBitWidth())
    InitVal = Builder.CreateSExtOrTrunc(InitVal, IndValTy);
  Builder.CreateStore(InitVal, LowerBnd);
  LBPtr = LowerBnd;

  AllocaInst *UpperBnd = Builder.CreateAlloca(IndValTy, nullptr, "upper.bnd");
  Value *UpperBndVal =
      VPOParoptUtils::computeOmpUpperBound(W, 0, EntryBB->getTerminator(),
                                           ".for.taskloop.init");

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
  assert(StrideVal &&
         "genLoopInitCodeForTaskLoop: Expect non-empty loop stride.");

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
      genPrivatizationAlloca(RedI, EntryBB->getFirstNonPHI(), ".red");

  RedI->setNew(NewRedInst);
  genReductionInit(W, RedI, EntryBB->getTerminator(), &DT);

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

  genReductionFini(W, RedI, DstArg, EntryBB->getTerminator(), &DT);

  NewRedInst->replaceAllUsesWith(SrcArg);

  cast<AllocaInst>(NewRedInst)->eraseFromParent();

  return FnTaskLoopRedComb;
}

// Generate the taskdup function for the first and last privates.
// void task_dup(%__struct.kmp_task_t_with_privates* dst,
//               %__struct.kmp_task_t_with_privates* src,
//               i32 last_iter_flag)
//
// Firstprivates: If a firstprivate var has a copy constructor, call the copy
// constructor to copy the data from src to dst.
// Lastprivates: Store the value of the last_iter_flag parameter, to the
// last_iter field of dst.
Function *
VPOParoptTransform::genFLPrivateTaskDup(WRegionNode *W,
                                        StructType *KmpTaskTTWithPrivatesTy) {
  // We only need the taskdup if there is a lastprivate, or a firstprivate
  // with a copy constructor.
  LastprivateClause &LprivClause = W->getLpriv();
  FirstprivateClause &FprivClause = W->getFpriv();
  if (LprivClause.empty()) {
    if (FprivClause.empty())
      return nullptr;
    bool hasCtor = false;
    for (FirstprivateItem *FI : FprivClause.items())
      if (FI->getCopyConstructor()) {
        hasCtor = true;
        break;
      }
    if (!hasCtor)
      return nullptr;
  }

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
  // Arg1: dst
  Value *Arg1 = &*I;
  I++;
  // Arg2: src
  Value *Arg2 = &*I;
  I++;
  // Arg3: last_iter flag
  Value *Arg3 = &*I;

  BasicBlock *EntryBB = BasicBlock::Create(C, "entry", FnTaskDup);

  DominatorTree DT;
  DT.recalculate(*FnTaskDup);

  IRBuilder<> Builder(EntryBB);

  SmallVector<Value *, 4> Indices;
  if (!LprivClause.empty()) {
    Indices.push_back(Builder.getInt32(0));
    Indices.push_back(Builder.getInt32(0));
    Value *BaseTaskTGep =
        Builder.CreateInBoundsGEP(KmpTaskTTWithPrivatesTy, Arg1, Indices);

    StructType *KmpTaskTTy =
        dyn_cast<StructType>(KmpTaskTTWithPrivatesTy->getElementType(0));

    Indices.clear();
    // Index of last_iter flag in dst.
    Indices.push_back(Builder.getInt32(0));
    Indices.push_back(Builder.getInt32(8));
    Value *LastIterGep =
        Builder.CreateInBoundsGEP(KmpTaskTTy, BaseTaskTGep, Indices);

    // Store last_iter flag value to dst.
    Builder.CreateStore(Arg3, LastIterGep);
  }
  auto *RetInst = Builder.CreateRetVoid();

  if (FprivClause.empty())
    return FnTaskDup;

  // Generate the copy constructor calls for the firstprivate items.
  Builder.SetInsertPoint(RetInst);
  for (FirstprivateItem *FI : FprivClause.items()) {
    if (!FI->getCopyConstructor())
      continue;

    // kmp_task_t_with_privates { kmp_task_t, kmp_privates_t }
    // kmp_privates_t { object_1, object_2, ... }
    // Assume each FI is represented by a GEP from kmp_privates_t.
    // area. We have a pointer to kmp_task_t_with_privates, so we will GEP
    // to kmp_privates_t and then add the FI's GEP index.
    //
    // If firstprivate codegen is later changed to use local vars instead
    // of thunk references, we will need to find the indices a different way.
    auto *FIGEP = cast<GetElementPtrInst>(FI->getNew());
    // a struct GEP should have 2 indices: 0 then the real index
    assert(FIGEP->getNumIndices() == 2);
    Indices.clear();
    // 0 == struct reference
    Indices.push_back(Builder.getInt32(0));
    // 1 == index of private area in kmp_task_t_with_privates
    Indices.push_back(Builder.getInt32(1));
    // GEP index from FI
    Indices.push_back(FIGEP->getOperand(2));
    Value *DstGEP =
        Builder.CreateInBoundsGEP(KmpTaskTTWithPrivatesTy, Arg1, Indices);
    Value *SrcGEP =
        Builder.CreateInBoundsGEP(KmpTaskTTWithPrivatesTy, Arg2, Indices);
    VPOParoptUtils::genCopyByAddr(DstGEP, SrcGEP, RetInst,
                                  FI->getCopyConstructor(), FI->getIsByRef());
    Builder.SetInsertPoint(RetInst);
  }
  return FnTaskDup;
}

// Generate the initialization code for task depend clauses. For each task
// depend clause, the compiler initializes the pointer, the size and the
// dependce type in the struct task_dep_info_type.
AllocaInst *
VPOParoptTransform::genDependInitForTask(WRegionNode *W,
                                         Instruction *InsertBefore) {
  if (!W->canHaveDepend())
    return nullptr;

  SmallVector<Type *, 4> KmpTaskTDependVecTyArgs;

  DependClause const &DepClause = W->getDepend();
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
                         "__struct.kmp_task_depend_vec", false);

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
void VPOParoptTransform::genRedInitForTask(WRegionNode *W,
                                           Instruction *InsertBefore) {

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genRedInitForTask\n");

  genTaskTRedType();

  SmallVector<Type *, 4> KmpTaskTRedRecTyArgs;

  if (!W->canHaveReduction())
    return; // in case this is a task instead of taskloop

  ReductionClause &RedClause = W->getRed();
  if (RedClause.empty())
    return;
  LLVMContext &C = F->getContext();

  for (int I = 0; I < RedClause.size(); I++)
    KmpTaskTRedRecTyArgs.push_back(KmpTaskTRedTy);

  StructType *KmpTaskTTRedRecTy = StructType::create(
      C, KmpTaskTRedRecTyArgs, "__struct.kmp_task_t_red_rec", false);

  IRBuilder<> Builder(InsertBefore);
  Value *Zero = Builder.getInt32(0);

  AllocaInst *DummyTaskTRedRec =
      Builder.CreateAlloca(KmpTaskTTRedRecTy, nullptr, "taskt.red.rec");

  const DataLayout DL = F->getParent()->getDataLayout();
  unsigned Count = 0;
  for (ReductionItem *RedI : RedClause.items()) {

    StringRef NamePrefix = RedI->getOrig()->getName();

    Value *BaseTaskTRedGep = Builder.CreateInBoundsGEP(
        KmpTaskTTRedRecTy, DummyTaskTRedRec, {Zero, Builder.getInt32(Count++)},
        NamePrefix + ".red.struct");

    Value *Gep = Builder.CreateInBoundsGEP(
        KmpTaskTRedTy, BaseTaskTRedGep, {Zero, Zero}, NamePrefix + ".red.item");
    Builder.CreateStore(
        Builder.CreateBitCast(RedI->getOrig(), Type::getInt8PtrTy(C)), Gep);

    Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                    {Zero, Builder.getInt32(1)},
                                    NamePrefix + ".red.size");
    Builder.CreateStore(
        Builder.getInt64(DL.getTypeAllocSize(
            RedI->getOrig()->getType()->getPointerElementType())),
        Gep);

    Function *RedInitFunc = genTaskLoopRedInitFunc(W, RedI);
    Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                    {Zero, Builder.getInt32(2)},
                                    NamePrefix + ".red.init");
    Builder.CreateStore(
        Builder.CreateBitCast(RedInitFunc, Type::getInt8PtrTy(C)), Gep);

    Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                    {Zero, Builder.getInt32(3)},
                                    NamePrefix + ".red.fini");
    Builder.CreateStore(ConstantPointerNull::get(Type::getInt8PtrTy(C)), Gep);

    Function *RedCombFunc = genTaskLoopRedCombFunc(W, RedI);
    Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                    {Zero, Builder.getInt32(4)},
                                    NamePrefix + ".red.comb");
    Builder.CreateStore(
        Builder.CreateBitCast(RedCombFunc, Type::getInt8PtrTy(C)), Gep);

    Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                    {Zero, Builder.getInt32(5)},
                                    NamePrefix + ".red.flags");
    Builder.CreateStore(Builder.getInt32(0), Gep);
  }
  VPOParoptUtils::genKmpcTaskReductionInit(
      W, TidPtrHolder, Count, DummyTaskTRedRec, &*Builder.GetInsertPoint(),
      Mode & OmpTbb);
  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genRedInitForTask\n");
}

bool VPOParoptTransform::genTaskCode(WRegionNode *W,
                                     StructType *KmpTaskTTWithPrivatesTy,
                                     StructType *KmpSharedTy) {
  return genTaskGenericCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy, nullptr,
                            nullptr, nullptr, false);
}

// Set the the arguments in the depend clause to be empty.
void VPOParoptTransform::resetValueInTaskDependClause(WRegionNode *W) {
  if (!W->canHaveDepend())
    return;

  DependClause const &DepClause = W->getDepend();
  if (DepClause.empty())
    return;
  for (DependItem *DepI : DepClause.items()) {
    resetValueInOmpClauseGeneric(W, DepI->getOrig());
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
  DependClause const &DepClause = W->getDepend();

  if (!IsTaskWait)
    VPOParoptUtils::genKmpcTaskWithDeps(W, IdentTy, TidPtr, TaskAlloc, Dep,
                                        DepClause.size(), InsertPt);
  else
    VPOParoptUtils::genKmpcTaskWaitDeps(W, IdentTy, TidPtr, Dep,
                                        DepClause.size(), InsertPt);
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

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTaskGenericCode\n");

  W->populateBBSet();

  resetValueInOmpClauseGeneric(W, W->getIf());

  resetValueInTaskDependClause(W);

  AllocaInst *SharedAggrStruct = genAndPopulateTaskSharedStruct(W, KmpSharedTy);

  // Set up Fn Attr for the new function
  Function *NewF = VPOParoptUtils::genOutlineFunction(*W, DT, AC);

  CallInst *NewCall = cast<CallInst>(NewF->user_back());
  CallSite CS(NewCall);

  // TidArgNo parameter is unused, if IsTidArg is false.
  Function *MTFn = finalizeExtractedMTFunction(W, NewF, false, -1U, false);

  std::vector<Value *> MTFnArgs;

  LLVMContext &C = NewF->getContext();
  IntegerType *Int32Ty = Type::getInt32Ty(C);
  ConstantInt *ValueZero = ConstantInt::getSigned(Int32Ty, 0);
  MTFnArgs.push_back(ValueZero);
  genThreadedEntryActualParmList(W, MTFnArgs);

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

  // Keep the orginal extracted function name after finalization
  MTFnCI->takeName(NewCall);

  genRedInitForTask(W, NewCall);

  AllocaInst *DummyTaskTDependRec = genDependInitForTask(W, NewCall);

  const DataLayout DL = NewF->getParent()->getDataLayout();
  int KmpTaskTTWithPrivatesTySz =
      DL.getTypeAllocSize(KmpTaskTTWithPrivatesTy);
  int KmpSharedTySz = DL.getTypeAllocSize(KmpSharedTy);
  assert(MTFnCI->getCalledFunction() &&
         "genTaskGenericCode: Expect non-empty function.");

  Function *DestrThunk = genTaskDestructorThunk(W, KmpTaskTTWithPrivatesTy);
  if (DestrThunk)
    W->setTaskFlag(W->getTaskFlag() | 0x8);

  CallInst *TaskAllocCI = VPOParoptUtils::genKmpcTaskAlloc(
      W, IdentTy, TidPtrHolder, KmpTaskTTWithPrivatesTySz, KmpSharedTySz,
      KmpRoutineEntryPtrTy, MTFnCI->getCalledFunction(), NewCall,
      Mode & OmpTbb);
  TaskAllocCI->setName(".task.alloc");

  copySharedStructToTaskThunk(W, SharedAggrStruct, TaskAllocCI, KmpSharedTy,
                              KmpTaskTTWithPrivatesTy, DestrThunk, NewCall);
  genFprivInitForTask(W, TaskAllocCI, KmpTaskTTWithPrivatesTy, NewCall);

  IRBuilder<> Builder(NewCall);

  Value *VIf = W->getIf();
  Value *Cmp = nullptr;
  if (VIf)
    Cmp = Builder.CreateICmpNE(VIf, ConstantInt::get(VIf->getType(), 0));
  if (isLoop) {
    VPOParoptUtils::genKmpcTaskLoop(
        W, IdentTy, TidPtrHolder, TaskAllocCI, Cmp, LBPtr, UBPtr, STPtr,
        KmpTaskTTWithPrivatesTy, NewCall, Mode & OmpTbb,
        genFLPrivateTaskDup(W, KmpTaskTTWithPrivatesTy));
  } else {
    if (!VIf) {
      if (!DummyTaskTDependRec)
        VPOParoptUtils::genKmpcTask(W, IdentTy, TidPtrHolder, TaskAllocCI,
                                    NewCall);
      else
        genTaskDeps(W, IdentTy, TidPtrHolder, TaskAllocCI,
                    DummyTaskTDependRec, NewCall, false);
    } else {

      Instruction *ThenTerm, *ElseTerm;

      buildCFGForIfClause(Cmp, ThenTerm, ElseTerm, NewCall);
      IRBuilder<> ElseBuilder(ElseTerm);
      if (!DummyTaskTDependRec)
        VPOParoptUtils::genKmpcTask(W, IdentTy, TidPtrHolder, TaskAllocCI,
                                    ThenTerm);
      else {
        genTaskDeps(W, IdentTy, TidPtrHolder, TaskAllocCI,
                    DummyTaskTDependRec, ThenTerm, false);
        genTaskDeps(W, IdentTy, TidPtrHolder, TaskAllocCI,
                    DummyTaskTDependRec, ElseTerm, true);
      }
      VPOParoptUtils::genKmpcTaskBeginIf0(W, IdentTy, TidPtrHolder,
                                          TaskAllocCI, ElseTerm);
      MTFnArgs.clear();
      MTFnArgs.push_back(ElseBuilder.CreateLoad(TidPtrHolder));
      MTFnArgs.push_back(ElseBuilder.CreateBitCast(
          TaskAllocCI, PointerType::getUnqual(KmpTaskTTWithPrivatesTy)));
      CallInst *SeqCI = CallInst::Create(MTFn, MTFnArgs, "", ElseTerm);
      SeqCI->setCallingConv(CS.getCallingConv());
      SeqCI->takeName(NewCall);
      VPOParoptUtils::genKmpcTaskCompleteIf0(W, IdentTy, TidPtrHolder,
                                             TaskAllocCI, ElseTerm);
    }
  }

  NewCall->eraseFromParent();
  NewF->eraseFromParent();
  MTFnCI->eraseFromParent();

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genTaskGenericCode\n");

  W->resetBBSet(); // Invalidate BBSet after transformations
  return true;
}

bool VPOParoptTransform::genTaskWaitCode(WRegionNode *W) {
  VPOParoptUtils::genKmpcTaskWait(W, IdentTy, TidPtrHolder,
                                  W->getEntryBBlock()->getTerminator());
  return true;
}

// build the CFG for if clause.
void VPOParoptTransform::buildCFGForIfClause(Value *Cmp,
                                             Instruction *&ThenTerm,
                                             Instruction *&ElseTerm,
                                             Instruction *InsertPt) {
  BasicBlock *SplitBeforeBB = InsertPt->getParent();
  SplitBlockAndInsertIfThenElse(Cmp, InsertPt, &ThenTerm, &ElseTerm);
  ThenTerm->getParent()->setName("if.then");
  ElseTerm->getParent()->setName("if.else");
  InsertPt->getParent()->setName("if.end");

  DT->addNewBlock(ThenTerm->getParent(), SplitBeforeBB);
  DT->addNewBlock(ElseTerm->getParent(), SplitBeforeBB);
  DT->addNewBlock(InsertPt->getParent(), SplitBeforeBB);

  DT->changeImmediateDominator(ThenTerm->getParent(), SplitBeforeBB);
  DT->changeImmediateDominator(ElseTerm->getParent(), SplitBeforeBB);
  BasicBlock *NextBB = InsertPt->getParent()->getSingleSuccessor();
  assert(NextBB && "Null Next BB.");
  if (NextBB->getUniquePredecessor())
    DT->changeImmediateDominator(NextBB, InsertPt->getParent());
}

// Generate code for OMP taskgroup construct.
//   #pragma omp taskgroup
bool VPOParoptTransform::genTaskgroupRegion(WRegionNode *W) {
  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTaskgroupRegion\n");

  W->populateBBSet();
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *ExitBB = W->getExitBBlock();

  Instruction *InsertPt = EntryBB->getTerminator();

  CallInst *TaskgroupCI =
      VPOParoptUtils::genKmpcTaskgroupCall(W, IdentTy, TidPtrHolder, InsertPt);
  TaskgroupCI->insertBefore(InsertPt);
  genRedInitForTask(W, InsertPt);

  Instruction *InsertEndPt = ExitBB->getTerminator();

  CallInst *EndTaskgroupCI = VPOParoptUtils::genKmpcEndTaskgroupCall(
      W, IdentTy, TidPtrHolder, InsertEndPt);
  EndTaskgroupCI->insertBefore(InsertEndPt);

  W->resetBBSet();
  return true;
}
#endif // INTEL_COLLAB
