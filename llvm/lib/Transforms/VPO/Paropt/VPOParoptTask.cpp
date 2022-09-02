#if INTEL_COLLAB
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
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
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"
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

    for (ReductionItem *RedI : RedClause.items()) {

      Value *Orig = RedI->getOrig();

      if (!isa<GlobalVariable>(Orig) && !isa<AllocaInst>(Orig))
          continue;

      Instruction *InsertPt = GeneralUtils::nextUniqueInstruction(
          cast<Instruction>(RedI->getNew()));
      auto *NewPrivInst = getClauseItemReplacementValue(RedI, InsertPt);
      genPrivatizationReplacement(W, Orig, NewPrivInst);
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
    if (auto *Dtor = FI->getDestructor())
      genPrivatizationInitOrFini(FI, Dtor, FK_Dtor, DestrGEP, nullptr, RetInst,
                                 &DT);
  }

  return DestThunk;
}

// Generate the code to update the last privates for taskloop.
void VPOParoptTransform::genLprivFiniForTaskLoop(LastprivateItem *LprivI,
                                                 Instruction *InsertPt) {
  Type *ItemTy = nullptr;
  Value *NumElements = nullptr;
  std::tie(ItemTy, NumElements, std::ignore) =
      VPOParoptUtils::getItemInfo(LprivI);

  Value *Src = LprivI->getNew();
  Value *Dst = LprivI->getOrigGEP();
  if (LprivI->getIsByRef())
    Dst = new LoadInst(Src->getType(), Dst, "", InsertPt);

#if INTEL_CUSTOMIZATION
  if (LprivI->getIsF90DopeVector()) {
    VPOParoptUtils::genF90DVLastprivateCopyCall(Src, Dst, InsertPt);
    return;
  }

#endif // INTEL_CUSTOMIZATION
  const DataLayout &DL = InsertPt->getModule()->getDataLayout();

  IRBuilder<> Builder(InsertPt);

  if (LprivI->getIsVla()) {
    MaybeAlign Align(DL.getABITypeAlignment(ItemTy));
    Builder.CreateMemCpy(Dst, Align, Src, Align,
                         LprivI->getNewThunkBufferSize());
  } else if (!VPOUtils::canBeRegisterized(ItemTy, DL) ||
             NumElements) {
    uint64_t Size = DL.getTypeAllocSize(ItemTy);
    assert((!NumElements || isa<ConstantInt>(NumElements)) &&
           "Lastprivate item should have been classified as VLA.");
    VPOUtils::genMemcpy(Dst, Src, Size, NumElements,
                        DL.getABITypeAlignment(ItemTy), Builder);
  } else {
    LoadInst *Load = Builder.CreateLoad(ItemTy, Src);
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
// typedef struct kmp_taskred_input {
//   void *reduce_shar; // shared between tasks item to reduce into
//   void *reduce_orig; // original reduction item used for initialization
//   size_t reduce_size; // size of data item
//   // three compiler-generated routines (init, fini are optional):
//   void *reduce_init; // data initialization routine (two parameters)
//   void *reduce_fini; // data finalization routine
//   void *reduce_comb; // data combiner routine
//   kmp_taskred_flags_t flags; // flags for additional info from compiler
// } kmp_taskred_input_t;
//
// Or if Mode is OmpTbb (uses old API):
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

  if (Mode & OmpTbb)
    KmpTaskTRedTy = VPOParoptUtils::getOrCreateStructType(
        F, "__struct.kmp_task_t_red_item",
        {Int8PtrTy, Int64Ty, Int8PtrTy, Int8PtrTy, Int8PtrTy, Int32Ty});
  else
    KmpTaskTRedTy = VPOParoptUtils::getOrCreateStructType(
        F, "__struct.kmp_taskred_input_t",
        {Int8PtrTy, Int8PtrTy, Int64Ty, Int8PtrTy, Int8PtrTy, Int8PtrTy,
         Int32Ty});
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
//            kmp_routine_entry_t destr;   // was kmp_cmplrdata_t data1;
//            IntPtrTy            priority // was kmp_cmplrdata_t data2;
//   For taskloops additional fields:
//            kmp_uint64          lb;
//            kmp_uint64          ub;
//            kmp_int64           st;
//            kmp_int32           liter;
//          };
// In kmp.h, kmp_cmplrdata_t is defined as the union of kmp_routine_entry_t
// (for destr thunk) and kmp_int32 (for priority). In Paropt we simplify the
// implementation by not using a union. Instead, we change it like this:
//            kmp_cmplrdata_t data1   becomes   kmp_routine_entry_t destr
//            kmp_cmplrdata_t data2   becomes   IntPtrTy            priority
// Both kmp_routine_entry_t and IntPtr have the same size (= pointer size).
void VPOParoptTransform::genKmpTaskTRecordDecl() {
  if (KmpTaskTTy)
    return;

  LLVMContext &C = F->getContext();
  IntegerType *Int32Ty = Type::getInt32Ty(C);
  IntegerType *Int64Ty = Type::getInt64Ty(C);
  Type *IntPtrTy = GeneralUtils::getSizeTTy(F); // i32/i64 matching ptr size

  Type *KmpTaskTyArgs[] = {Type::getInt8PtrTy(C),
                           KmpRoutineEntryPtrTy,
                           Int32Ty,
                           KmpRoutineEntryPtrTy,
                           IntPtrTy,
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
    WRegionNode *W, StructType *&KmpSharedTy, StructType *&KmpPrivatesTy,
    Instruction *InsertBefore) {
  LLVMContext &C = F->getContext();
  SmallVector<Type *, 4> KmpTaksTWithPrivatesTyArgs;
  KmpTaksTWithPrivatesTyArgs.push_back(KmpTaskTTy);
  SmallVector<Type *, 4> PrivateThunkTypes;
  SmallVector<Type *, 4> SharedThunkTypes;
  IRBuilder<> Builder(InsertBefore);
  Instruction *InsertBeforeForSize = W->getVlaAllocaInsertPt();
  IRBuilder<> SizeBuilder(C);
  if (InsertBeforeForSize)
    SizeBuilder.SetInsertPoint(InsertBeforeForSize);
  Type *SizeTTy = GeneralUtils::getSizeTTy(InsertBefore->getFunction());
  unsigned SizeTBitWidth = SizeTTy->getIntegerBitWidth();

  unsigned PrivateCount = 0;
  unsigned SharedCount = 0;
#if INTEL_CUSTOMIZATION

  auto reserveExtraFieldsForF90DVInPrivateThunk = [&](Item *I) {
    if (!I->getIsF90DopeVector())
      return;

    assert(InsertBeforeForSize != nullptr &&
           "genKmpTaskTWithPrivatesRecordDecl: Expect a vla array size "
           "insertion point for F90 Dope Vector");

    // For F90 DVs, similar to VLAs, we allocate three spaces in the
    // privates thunk, corresponding to the local copy of the DV, array size
    // in bytes, and offset to the actual array in the task thunk buffer.
    PrivateCount += 2;
    PrivateThunkTypes.push_back(SizeTTy);
    PrivateThunkTypes.push_back(SizeTTy);

    StringRef NamePrefix = I->getOrig()->getName();
    Value *ArraySizeInBytes =
        VPOParoptUtils::genF90DVSizeCall(I->getOrig(), InsertBeforeForSize);
    ArraySizeInBytes->setName(NamePrefix + ".array.size.in.bytes");
    I->setThunkBufferSize(ArraySizeInBytes);
  };
#endif // INTEL_CUSTOMIZATION

  auto reserveFieldsForItemInPrivateThunk = [&](Item *I) {
    Type *ElementTy = nullptr;
    Value *NumElements = nullptr;
    std::tie(ElementTy, NumElements, std::ignore) =
        VPOParoptUtils::getItemInfo(I);
    if (!NumElements) {
      PrivateThunkTypes.push_back(ElementTy);
      I->setPrivateThunkIdx(PrivateCount++);
#if INTEL_CUSTOMIZATION
      reserveExtraFieldsForF90DVInPrivateThunk(I);
#endif // INTEL_CUSTOMIZATION
      return;
    }

#if INTEL_CUSTOMIZATION
    assert(!I->getIsF90DopeVector() && "Unexpected: array of dope vectors");

#endif // INTEL_CUSTOMIZATION
    if (isa<ConstantInt>(NumElements)) {
      PrivateThunkTypes.push_back(ArrayType::get(
          ElementTy, cast<ConstantInt>(NumElements)->getZExtValue()));
      I->setPrivateThunkIdx(PrivateCount++);
      return;
    }

    StringRef NamePrefix = I->getOrig()->getName();
    I->setIsVla(true);

    assert(InsertBeforeForSize != nullptr &&
           "genKmpTaskTWithPrivatesRecordDecl: Expect a vla array size "
           "insertion point");

    Value *ElementSize = SizeBuilder.getIntN(
        SizeTBitWidth, ElementTy->getScalarSizeInBits() / 8);
    Value *ArraySizeInBytes =
        SizeBuilder.CreateMul(Builder.CreateBitCast(NumElements, SizeTTy),
                              ElementSize, NamePrefix + ".array.size.in.bytes");
    I->setThunkBufferSize(ArraySizeInBytes);

    // For VLAs, we allocate three spaces in the privates thunk. For the pointer
    // corresponding to the local copy of the item, array size, and offset for
    // the actual array which is allocated in a buffer at the end of the task
    // thunk.
    I->setPrivateThunkIdx(PrivateCount);
    PrivateCount += 3;
    PrivateThunkTypes.push_back(PointerType::getUnqual(ElementTy));
    PrivateThunkTypes.push_back(SizeTTy);
    PrivateThunkTypes.push_back(SizeTTy);
  };

  for (FirstprivateItem *FprivI : W->getFpriv().items())
    reserveFieldsForItemInPrivateThunk(FprivI);

  if (W->canHaveLastprivate()) {
    for (LastprivateItem *LprivI : W->getLpriv().items()) {
      if (FirstprivateItem *FprivI = LprivI->getInFirstprivate()) {
        LprivI->setPrivateThunkIdx(FprivI->getPrivateThunkIdx());
        LprivI->setIsVla(FprivI->getIsVla());
        LprivI->setThunkBufferSize(FprivI->getThunkBufferSize());
      } else
        reserveFieldsForItemInPrivateThunk(LprivI);

      auto PT = dyn_cast<PointerType>(LprivI->getOrig()->getType());
      assert(PT && "genKmpTaskTWithPrivatesRecordDecl: Expect last private "
                   "pointer argument");
      SharedThunkTypes.push_back(PT);
      LprivI->setSharedThunkIdx(SharedCount++);
    }
  }

  for (PrivateItem *PrivI : W->getPriv().items())
    reserveFieldsForItemInPrivateThunk(PrivI);

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

  AllocaInst *LBPtr, *UBPtr, *STPtr;

  return genTaskLoopInitCode(W, KmpTaskTTWithPrivatesTy, KmpSharedTy, LBPtr,
                             UBPtr, STPtr, LastIterGep, false);
}

// If an item has buffer space allocated for it at end of the thunk, make its
// New field point to that buffer space.
void VPOParoptTransform::linkPrivateItemToBufferAtEndOfThunkIfApplicable(
    Item *I, StructType *KmpPrivatesTy, Value *PrivatesGep,
    Value *TaskTWithPrivates, IRBuilder<> &Builder) {
  if (!I->getIsVla())
#if INTEL_CUSTOMIZATION
    if (!I->getIsF90DopeVector())
#endif // INTEL_CUSTOMIZATION
    return;

  StringRef OrigName = I->getOrig()->getName();
  Value *Zero = Builder.getInt32(0);

  int NewVIdx = I->getPrivateThunkIdx();
  Value *NewVGep = Builder.CreateInBoundsGEP(KmpPrivatesTy, PrivatesGep,
                                             {Zero, Builder.getInt32(NewVIdx)},
                                             OrigName + ".gep");

  // Create an if-then branch to check whether there is any space allocated
  // for NewV's data.
  Value *NewVDataSizeGep = Builder.CreateInBoundsGEP(
      KmpPrivatesTy, PrivatesGep, {Zero, Builder.getInt32(NewVIdx + 1)},
      OrigName + ".data.size.gep");
  Value *DataSize = Builder.CreateLoad(
      cast<GEPOperator>(NewVDataSizeGep)->getResultElementType(),
      NewVDataSizeGep, OrigName + ".data.size");

  Value *ZeroSize =
      Builder.getIntN(DataSize->getType()->getIntegerBitWidth(), 0);
  Value *IsSizeNonZero =
      Builder.CreateICmpNE(DataSize, ZeroSize, "is.size.non.zero");

  Instruction *BranchPt = &*Builder.GetInsertPoint();

  Instruction *ThenTerm = SplitBlockAndInsertIfThen(
      IsSizeNonZero, BranchPt, false,
      MDBuilder(Builder.getContext()).createBranchWeights(4, 1), DT, LI);
  BasicBlock *ThenBB = ThenTerm->getParent();
  ThenBB->setName("size.is.non.zero.then");

  Builder.SetInsertPoint(ThenTerm);

  // Now, inside the if-then branch, link the allocated data to the "new" field.
  Value *NewVDataOffsetGep = Builder.CreateInBoundsGEP(
      KmpPrivatesTy, PrivatesGep, {Zero, Builder.getInt32(NewVIdx + 2)},
      OrigName + ".data.offset.gep");
  Value *NewVDataOffset = Builder.CreateLoad(
      cast<GEPOperator>(NewVDataOffsetGep)->getResultElementType(),
      NewVDataOffsetGep, OrigName + ".data.offset");

  Type *Int8Ty = Builder.getInt8Ty();
  Type *Int8PtrTy = Builder.getInt8PtrTy();
  Value *TaskThunkBasePtr = Builder.CreateBitCast(TaskTWithPrivates, Int8PtrTy,
                                                  ".taskt.withprivates.base");
  Value *NewVData = Builder.CreateGEP(Int8Ty, TaskThunkBasePtr, NewVDataOffset,
                                      OrigName + ".priv.data");

  Builder.CreateStore(NewVData, Builder.CreateBitCast(
                                    NewVGep, PointerType::getUnqual(Int8PtrTy),
                                    OrigName + ".priv.gep.cast"));

  Builder.SetInsertPoint(BranchPt);
}

// Generate the code to replace the variables in the task loop with
// the thunk field dereferences
bool VPOParoptTransform::genTaskLoopInitCode(
    WRegionNode *W, StructType *&KmpTaskTTWithPrivatesTy,
    StructType *&KmpSharedTy, AllocaInst *&LBPtr, AllocaInst *&UBPtr,
    AllocaInst *&STPtr, Value *&LastIterGep, bool isLoop) {

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
  Instruction *InsertBefore = VPOParoptUtils::getInsertionPtForAllocas(W, F);
  KmpTaskTTWithPrivatesTy = genKmpTaskTWithPrivatesRecordDecl(
      W, KmpSharedTy, KmpPrivatesTy, InsertBefore);

  IRBuilder<> Builder(InsertBefore);
  Value *Zero = Builder.getInt32(0);

  Instruction *DummyTaskTWithPrivates = Builder.CreateAlloca(
      KmpTaskTTWithPrivatesTy, nullptr, "taskt.withprivates");

  if (isTargetSPIRV()) {
    // Pointers passed to the target function must be in generic space.
    DummyTaskTWithPrivates = cast<Instruction>(Builder.CreateAddrSpaceCast(
        DummyTaskTWithPrivates,
        KmpTaskTTWithPrivatesTy->getPointerTo(vpo::ADDRESS_SPACE_GENERIC)));
  }

  Builder.SetInsertPoint(W->getEntryBBlock()->getTerminator());
  Value *BaseTaskTGep =
      Builder.CreateInBoundsGEP(KmpTaskTTWithPrivatesTy, DummyTaskTWithPrivates,
                                {Zero, Zero}, ".taskt.base");
  // Value *PartIdGep = Builder.CreateInBoundsGEP(
  //     KmpTaskTTy, BaseTaskTGep, {Zero, Builder.getInt32(2)}, ".part.id");

  Value *SharedGep =
      Builder.CreateInBoundsGEP(KmpTaskTTy, BaseTaskTGep, {Zero, Zero});
  Value *SharedLoad = Builder.CreateLoad(
      cast<GEPOperator>(SharedGep)->getResultElementType(), SharedGep);
  Value *SharedCast = Builder.CreateBitCast(
      SharedLoad, PointerType::getUnqual(KmpSharedTy), ".shareds");

  Value *PrivatesGep =
      Builder.CreateInBoundsGEP(KmpTaskTTWithPrivatesTy, DummyTaskTWithPrivates,
                                {Zero, Builder.getInt32(1)}, ".privates");

  Value *LowerBoundGep = Builder.CreateInBoundsGEP(
      KmpTaskTTy, BaseTaskTGep, {Zero, Builder.getInt32(5)}, ".lb.gep");
  Value *LowerBoundLd = Builder.CreateLoad(
      cast<GEPOperator>(LowerBoundGep)->getResultElementType(),
      LowerBoundGep, ".lb");

  Value *UpperBoundGep = Builder.CreateInBoundsGEP(
      KmpTaskTTy, BaseTaskTGep, {Zero, Builder.getInt32(6)}, ".ub.gep");
  Value *UpperBoundLd = Builder.CreateLoad(
      cast<GEPOperator>(UpperBoundGep)->getResultElementType(),
      UpperBoundGep, ".ub");

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

  // Update the New and NewThunkBufferSize fields for the clause item.
  auto setNewForItemFromPrivateThunk = [&Builder, &KmpPrivatesTy, &PrivatesGep,
                                        &Zero](Item *I) {
    StringRef OrigName = I->getOrig()->getName();

    int NewVIdx = I->getPrivateThunkIdx();
    Value *NewVGep = Builder.CreateInBoundsGEP(
        KmpPrivatesTy, PrivatesGep, {Zero, Builder.getInt32(NewVIdx)},
        OrigName + ".gep");
    if (!I->getIsVla()) {
      I->setNew(NewVGep);
      return;
    }
    I->setNew(Builder.CreateLoad(
        cast<GEPOperator>(NewVGep)->getResultElementType(), NewVGep, OrigName));

    Value *NewVDataSizeGep = Builder.CreateInBoundsGEP(
        KmpPrivatesTy, PrivatesGep, {Zero, Builder.getInt32(NewVIdx + 1)},
        OrigName + ".data.size.gep");
    I->setNewThunkBufferSize(Builder.CreateLoad(
        cast<GEPOperator>(NewVDataSizeGep)->getResultElementType(),
        NewVDataSizeGep, OrigName + ".data.size"));
  };

  for (PrivateItem *PrivI : W->getPriv().items()) {
    linkPrivateItemToBufferAtEndOfThunkIfApplicable(
        PrivI, KmpPrivatesTy, PrivatesGep, DummyTaskTWithPrivates, Builder);
    setNewForItemFromPrivateThunk(PrivI);
  }

  for (FirstprivateItem *FprivI : W->getFpriv().items()) {
    linkPrivateItemToBufferAtEndOfThunkIfApplicable(
        FprivI, KmpPrivatesTy, PrivatesGep, DummyTaskTWithPrivates, Builder);
    setNewForItemFromPrivateThunk(FprivI);
  }

  if (W->canHaveLastprivate()) {
    for (LastprivateItem *LprivI : W->getLpriv().items()) {
      if (FirstprivateItem *FprivI = LprivI->getInFirstprivate()) {
        LprivI->setNew(FprivI->getNew());
        LprivI->setNewThunkBufferSize(FprivI->getNewThunkBufferSize());
      } else {
        linkPrivateItemToBufferAtEndOfThunkIfApplicable(
            LprivI, KmpPrivatesTy, PrivatesGep, DummyTaskTWithPrivates, Builder);
        setNewForItemFromPrivateThunk(LprivI);
      }

      StringRef OrigName = LprivI->getOrig()->getName();
      Value *ThunkSharedGep = Builder.CreateInBoundsGEP(
          KmpSharedTy, SharedCast,
          {Zero, Builder.getInt32(LprivI->getSharedThunkIdx())},
          OrigName + ".shr.gep");
      Value *ThunkSharedVal = Builder.CreateLoad(
          cast<GEPOperator>(ThunkSharedGep)->getResultElementType(),
          ThunkSharedGep, OrigName + ".shr");
      // Parm is used to record the address of last private in the compiler
      // shared variables in the thunk.
      LprivI->setOrigGEP(ThunkSharedVal);
    }
  }

  // Utility to generate the new reference from the call KmpcRedGetNthData
  // for the reduction variables.
  auto GenRefForRedVarsInTask = [&] (ReductionClause &RedClause,
                                     IRBuilder<> &Builder) {
    if (!RedClause.empty()) {
      for (ReductionItem *RedI : RedClause.items()) {
        computeArraySectionTypeOffsetSize(W, *RedI, &*Builder.GetInsertPoint());

        StringRef OrigName = RedI->getOrig()->getName();
        Value *ThunkSharedGep = Builder.CreateInBoundsGEP(
            KmpSharedTy, SharedCast,
            {Zero, Builder.getInt32(RedI->getSharedThunkIdx())},
            OrigName + ".shr.gep");
        Value *ThunkSharedVal = Builder.CreateLoad(
            cast<GEPOperator>(ThunkSharedGep)->getResultElementType(),
            ThunkSharedGep, OrigName + ".shr");

        // The __kmpc_task_reduction_get_th_data call needs the pointer to the
        // actual data being reduced, after any pointer dereferences or offset
        // additions.
        Type *ElementType = nullptr;
        std::tie(ElementType, std::ignore, std::ignore) =
            VPOParoptUtils::getItemInfo(RedI);

        if (RedI->getIsByRef())
          ThunkSharedVal = Builder.CreateLoad(
              ElementType->getPointerTo(
                  isTargetSPIRV() ? vpo::ADDRESS_SPACE_GENERIC : 0),
              ThunkSharedVal, OrigName + ".shr.deref");

        if (RedI->getIsArraySection()) {
          const ArraySectionInfo &ArrSecInfo = RedI->getArraySectionInfo();
          ThunkSharedVal = genBasePlusOffsetGEPForArraySection(
              ThunkSharedVal, ArrSecInfo, &*Builder.GetInsertPoint());
        }

        Value *RedRes = VPOParoptUtils::genKmpcRedGetNthData(
            W, TidPtrHolder, ThunkSharedVal, &*Builder.GetInsertPoint(),
            Mode & OmpTbb);
        RedRes->setName(OrigName + ".red");

        Type *RedNewTy = PointerType::getUnqual(ElementType);
        Value *RedResCast =
            Builder.CreateBitCast(RedRes, RedNewTy, OrigName + ".red.cast");
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
      Value *ThunkSharedVal = Builder.CreateLoad(
          cast<GEPOperator>(ThunkSharedGep)->getResultElementType(),
          ThunkSharedGep, OrigName + ".shr");
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

// Initialize the shared data area inside the thunk for task/taskloop.
// Fields 0, 3, and 4 in kmp_task_t may be initialized in this routine:
// struct kmp_task_t {
//   (idx=0)  void *              Shareds;
//   (idx=1)  kmp_routine_entry_t Routine;
//   (idx=2)  kmp_int32           PartId;
//   (idx=3)  kmp_routine_entry_t DestrThunk;
//   (idx=4)  IntPtrTy            Priority;
void VPOParoptTransform::copySharedStructToTaskThunk(
    WRegionNode *W, AllocaInst *Src, Value *Dst, StructType *KmpSharedTy,
    StructType *KmpTaskTTWithPrivatesTy, Function *DestrThunk,
    Instruction *InsertPt) {

  if (KmpSharedTy->getNumElements() == 0 && !DestrThunk && !W->getPriority())
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

    Value *LI = Builder.CreateLoad(
        cast<GEPOperator>(SharedTyGep)->getResultElementType(), SharedTyGep,
        ".shareds");

    LLVMContext &C = F->getContext();
    Value *SrcCast = Builder.CreateBitCast(Src, Type::getInt8PtrTy(C));

    Value *Size;

    const DataLayout DL = F->getParent()->getDataLayout();
    if (DL.getIntPtrType(Builder.getInt8PtrTy())->getIntegerBitWidth() == 64)
      Size = Builder.getInt64(
          DL.getTypeAllocSize(Src->getAllocatedType()));
    else
      Size = Builder.getInt32(
          DL.getTypeAllocSize(Src->getAllocatedType()));

    MaybeAlign Align(DL.getABITypeAlignment(Src->getAllocatedType()));
    Builder.CreateMemCpy(LI, Align, SrcCast, Align, Size);
  }

  if (DestrThunk) {
    Value *DestrGep = Builder.CreateInBoundsGEP(
        KmpTaskTTy, TaskTTyGep, {Zero, Builder.getInt32(3)}, ".destr.gep");
    Builder.CreateStore(DestrThunk, DestrGep);
  }

  if (W->getPriority()) {
    Value *PriorityGep = Builder.CreateInBoundsGEP(
        KmpTaskTTy, TaskTTyGep, {Zero, Builder.getInt32(4)}, ".priority.gep");
    Type *IntPtrTy = GeneralUtils::getSizeTTy(F);
    Value *Cast = Builder.CreateZExtOrBitCast(W->getPriority(), IntPtrTy,
                                              ".priority.cast");
    Builder.CreateStore(Cast, PriorityGep);
  }
}

Value *VPOParoptTransform::computeExtraBufferSpaceNeededAfterEndOfTaskThunk(
    WRegionNode *W, int TaskThunkWithPrivatesSize, Instruction *InsertBefore) {
  assert(TaskThunkWithPrivatesSize > 0 && "Unexpected size of task thunk.");

  IRBuilder<> Builder(InsertBefore);
  unsigned SizeTBitWidth = GeneralUtils::getSizeTTy(InsertBefore->getFunction())
                               ->getIntegerBitWidth();
  Value *CurrentOffset =
      Builder.getIntN(SizeTBitWidth, TaskThunkWithPrivatesSize);

  auto computeOffsetForItem = [&CurrentOffset, &Builder](Item *I) {
    if (!I->getIsVla())
#if INTEL_CUSTOMIZATION
      if (!I->getIsF90DopeVector())
#endif // INTEL_CUSTOMIZATION
      return;

    StringRef NamePrefix = I->getOrig()->getName();
    CurrentOffset->setName(NamePrefix + ".array.offset");
    I->setThunkBufferOffset(CurrentOffset);

    // next item's offset = current offset + current item's data size
    CurrentOffset = Builder.CreateAdd(CurrentOffset, I->getThunkBufferSize());
  };

  for (PrivateItem *PrivI : W->getPriv().items())
    computeOffsetForItem(PrivI);

  for (FirstprivateItem *FprivI : W->getFpriv().items())
    computeOffsetForItem(FprivI);

  if (W->canHaveLastprivate()) {
    for (LastprivateItem *LprivI : W->getLpriv().items())
      if (FirstprivateItem *FprivI = LprivI->getInFirstprivate())
        LprivI->setThunkBufferOffset(FprivI->getThunkBufferOffset());
      else
        computeOffsetForItem(LprivI);
  }
  CurrentOffset->setName("sizeof.taskt.with.privates.and.buffer");
  return CurrentOffset;
}

void VPOParoptTransform::saveVLASizeAndOffsetToPrivatesThunk(
    WRegionNode *W, Value *KmpPrivatesGep, StructType *KmpPrivatesTy,
    Instruction *InsertBefore) {

  IRBuilder<> Builder(InsertBefore);
  Value *Zero = Builder.getInt32(0);

  auto saveSizeAndOffsetForItem = [&](Item *I) {
    if (!I->getIsVla())
#if INTEL_CUSTOMIZATION
      if (!I->getIsF90DopeVector())
#endif // INTEL_CUSTOMIZATION
      return;

    StringRef OrigName = I->getOrig()->getName();
    int NewVIdx = I->getPrivateThunkIdx();

    // The privates thunk contains NewV, size and offset in three
    // consecutive locations beginning from the PrivateThunkIdx. Here we save
    // size and offset to the corresponding locations.
    Value *NewVDataSizeGep = Builder.CreateInBoundsGEP(
        KmpPrivatesTy, KmpPrivatesGep, {Zero, Builder.getInt32(NewVIdx + 1)},
        OrigName + ".priv.data.size.gep");
    Builder.CreateStore(I->getThunkBufferSize(), NewVDataSizeGep);

    Value *NewVDataOffsetGep = Builder.CreateInBoundsGEP(
        KmpPrivatesTy, KmpPrivatesGep, {Zero, Builder.getInt32(NewVIdx + 2)},
        OrigName + ".priv.data.offset.gep");
    Builder.CreateStore(I->getThunkBufferOffset(), NewVDataOffsetGep);
  };

  for (PrivateItem *PrivI : W->getPriv().items())
    saveSizeAndOffsetForItem(PrivI);

  for (FirstprivateItem *FprivI : W->getFpriv().items())
    saveSizeAndOffsetForItem(FprivI);

  if (W->canHaveLastprivate())
    for (LastprivateItem *LprivI : W->getLpriv().items()) {
      if (LprivI->getInFirstprivate())
        continue;
      saveSizeAndOffsetForItem(LprivI);
    }
}

Value *
VPOParoptTransform::genPrivatesGepForTask(Value *KmpTaskTTWithPrivates,
                                          StructType *KmpTaskTTWithPrivatesTy,
                                          Instruction *InsertBefore) {
  IRBuilder<> Builder(InsertBefore);
  Value *Zero = Builder.getInt32(0);
  Value *Cast = Builder.CreateBitCast(
      KmpTaskTTWithPrivates, PointerType::getUnqual(KmpTaskTTWithPrivatesTy),
      ".taskt.with.privates");

  return Builder.CreateInBoundsGEP(KmpTaskTTWithPrivatesTy, Cast,
                                   {Zero, Builder.getInt32(1)}, ".privates");
}

void VPOParoptTransform::genFprivInitForTask(WRegionNode *W,
                                             Value *KmpTaskTTWithPrivates,
                                             Value *KmpPrivatesGEP,
                                             StructType *KmpPrivatesTy,
                                             Instruction *InsertBefore) {

  FirstprivateClause &FprivClause = W->getFpriv();
  if (FprivClause.empty())
    return;

  IRBuilder<> Builder(InsertBefore);
  const DataLayout &DL = InsertBefore->getModule()->getDataLayout();

  for (FirstprivateItem *FprivI : FprivClause.items()) {

    assert(!FprivI->getThunkBufferOffset() ||
           !FprivI->getIsByRef() && "Unexpected Byref + VLA operand.");

    Value *OrigV = FprivI->getOrig();
    StringRef NamePrefix = OrigV->getName();

    // CodeExtractor may have moved the firstprivate value's definition into
    // the extractee, if it was totally unused in the parent.
    if ((isa<Instruction>(OrigV) && cast<Instruction>(OrigV)->getFunction() !=
                                        InsertBefore->getFunction()) ||
        (isa<Argument>(OrigV) &&
         cast<Argument>(OrigV)->getParent() != InsertBefore->getFunction())) {
      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Skipping firstprivate init for '";
                 OrigV->printAsOperand(dbgs());
                 dbgs() << "' as it's no longer in the current function.\n");
      continue;
    }

    if (FprivI->getIsVla()) {
      Type *Int8Ty = Builder.getInt8Ty();
      Type *Int8PtrTy = Builder.getInt8PtrTy();
      Value *TaskThunkBasePtr = Builder.CreateBitCast(
          KmpTaskTTWithPrivates, Int8PtrTy, ".taskt.with.privates.base");

      Value *NewData = Builder.CreateGEP(Int8Ty, TaskThunkBasePtr,
                                         FprivI->getThunkBufferOffset(),
                                         NamePrefix + ".priv.data");

      Value *OrigCast =
          Builder.CreateBitCast(OrigV, Int8PtrTy, NamePrefix + ".cast");

      Type *ItemTy = nullptr;
      std::tie(ItemTy, std::ignore, std::ignore) =
          VPOParoptUtils::getItemInfo(FprivI);

      MaybeAlign Align(DL.getABITypeAlignment(ItemTy));

      Builder.CreateMemCpy(NewData, Align, OrigCast, Align,
                           FprivI->getThunkBufferSize());
      continue;
    }

    Value *NewVGep = Builder.CreateInBoundsGEP(
        KmpPrivatesTy, KmpPrivatesGEP,
        {Builder.getInt32(0), Builder.getInt32(FprivI->getPrivateThunkIdx())},
        NamePrefix + ".priv.gep");
#if INTEL_CUSTOMIZATION

    if (FprivI->getIsF90DopeVector()) {
      // We first link the private dope vector to the data buffer allocated for
      // it, so that we can call f90_firstprivate_copy on it.
      linkPrivateItemToBufferAtEndOfThunkIfApplicable(
          FprivI, KmpPrivatesTy, KmpPrivatesGEP, KmpTaskTTWithPrivates,
          Builder);

      VPOParoptUtils::genF90DVFirstprivateCopyCall(NewVGep, OrigV,
                                                   InsertBefore);
      continue;
    }
#endif // INTEL_CUSTOMIZATION

    genCopyByAddr(FprivI, NewVGep, OrigV, InsertBefore,
                  FprivI->getCopyConstructor(), FprivI->getIsByRef());
  }
}

// Save the loop lower upper bound, upper bound and stride for the use
// by the call __kmpc_taskloop_5
void VPOParoptTransform::genLoopInitCodeForTaskLoop(WRegionNode *W,
                                                    AllocaInst *&LBPtr,
                                                    AllocaInst *&UBPtr,
                                                    AllocaInst *&STPtr) {
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

// Generate the outline function to be used as the reduce_init callback.
// The callback function has two arguments by default (for
// kmpc_taskred_init):
//   void reduce_init(&omp_priv, &omp_orig)
//
// For OmpTbb Mode (which uses the older task_reduction_init library function),
// the callback has one argument:
//   void reduce_init(&omp_priv)
Function *VPOParoptTransform::genTaskLoopRedInitFunc(WRegionNode *W,
                                                     ReductionItem *RedI) {
  LLVMContext &C = F->getContext();
  Module *M = F->getParent();

  Type *ElementType = nullptr;
  std::tie(ElementType, std::ignore, std::ignore) =
      VPOParoptUtils::getItemInfo(RedI);
  Type *ElementPtrType = PointerType::getUnqual(ElementType);

  SmallVector<Type *, 2> TaskLoopRedInitParams{ElementPtrType};
  if (!(Mode & OmpTbb))
    TaskLoopRedInitParams.push_back(ElementPtrType);

  FunctionType *TaskLoopRedInitFnTy =
      FunctionType::get(Type::getVoidTy(C), TaskLoopRedInitParams, false);

  Function *FnTaskLoopRedInit = Function::Create(
      TaskLoopRedInitFnTy, GlobalValue::InternalLinkage,
      F->getName() + "_task_red_init_" + Twine(W->getNumber()), M);
  FnTaskLoopRedInit->setCallingConv(CallingConv::C);

  Value *PrivArg = FnTaskLoopRedInit->getArg(0);
  Value *OrigArg = Mode & OmpTbb ? nullptr : FnTaskLoopRedInit->getArg(1);

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
  RedI->setTaskRedInitOrigArg(OrigArg);
  genReductionInit(W, RedI, EntryBB->getTerminator(), &DT);

  NewRedInst->replaceAllUsesWith(PrivArg);

  return FnTaskLoopRedInit;
}

// Generate the outline function for the reduction update
Function *VPOParoptTransform::genTaskLoopRedCombFunc(WRegionNode *W,
                                                     ReductionItem *RedI) {
  LLVMContext &C = F->getContext();
  Module *M = F->getParent();

  Type *ElementType = nullptr;
  std::tie(ElementType, std::ignore, std::ignore) =
      VPOParoptUtils::getItemInfo(RedI);

  Type *TaskLoopRedInitParams[] = {PointerType::getUnqual(ElementType),
                                   PointerType::getUnqual(ElementType)};
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

  genReductionFini(
      W, RedI, DstArg, EntryBB->getTerminator(), &DT,
      true); // There is no need for extra dereference/offset computation on the
             // destination arg in the reduction combiner for tasks.

  NewRedInst->replaceAllUsesWith(SrcArg);

  cast<Instruction>(NewRedInst)->eraseFromParent();

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
    genCopyByAddr(FI, DstGEP, SrcGEP, RetInst, FI->getCopyConstructor(),
                  FI->getIsByRef());
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
    Value *Orig = DepI->getOrig();
    Value *Size = nullptr;
    Value *BasePtr = Orig;
    Type *IntPtrTy = Builder.getIntPtrTy(DL);

    // If the dep value is output-only with no uses, outlining may have
    // wrapped it. Make a replacement alloca.
    if (auto *AI = dyn_cast<AllocaInst>(Orig))
      if (AI->getFunction() != InsertBefore->getFunction()) {
        Orig = Builder.CreateAlloca(Orig->getType(), nullptr, AI->getName());
        BasePtr = Orig;
      }

    // Paropt does not support code generation for non-contiguous sections.
    // When FE generates TYPED form of the clause it passes all the required
    // information in the clause itself:
    //   "(ptr %base, <element type specifier>,
    //     i64 %number.of.elements, i64 %offset.in.elements)"
    //
    // For TYPED clauses the array section info is populated during parsing,
    // so we do not need to call computeArraySectionTypeOffsetSize() here.
    if (!DepI->getIsTyped())
      computeArraySectionTypeOffsetSize(W, Orig, DepI->getArraySectionInfo(),
                                        DepI->getIsByRef(), InsertBefore);

    Value *BaseTaskTDependGep = Builder.CreateInBoundsGEP(
        KmpTaskTDependVecTy, DummyTaskTDependVec,
        {Builder.getInt32(0), Builder.getInt32(Count++)}, ".dep.struct");

    if (DepI->getIsArraySection()) {
      const ArraySectionInfo &ArrSecInfo = DepI->getArraySectionInfo();
      BasePtr =
          genBasePlusOffsetGEPForArraySection(Orig, ArrSecInfo, InsertBefore);
      Value *NumElements = ArrSecInfo.getSize();
      Value *ElementSize = Builder.getIntN(
          DL.getPointerSizeInBits(),
          DL.getTypeSizeInBits(ArrSecInfo.getElementType()) / 8);
      Size = Builder.CreateMul(NumElements, ElementSize,
                               Orig->getName() + ".size.in.bytes");
    } else {
      if (DepI->getIsTyped()) {
        Size = Builder.getIntN(
            DL.getPointerSizeInBits(),
            DL.getTypeAllocSize(DepI->getOrigItemElementTypeFromIR()));
        if (Value *NumElements = DepI->getNumElements()) {
          NumElements = Builder.CreateZExtOrTrunc(NumElements, Size->getType());
          Size = Builder.CreateMul(Size, NumElements);
        }
      } else {
        // OPAQUEPOINTER: this should be unreachable with opaque pointers.
        if (!cast<PointerType>(Orig->getType())->isOpaque())
          Size = Builder.getIntN(
              DL.getPointerSizeInBits(),
              DL.getTypeAllocSize(
                  Orig->getType()->getNonOpaquePointerElementType()));
        else
          llvm_unreachable("use DEPEND:TYPED with opaque pointers.");
      }
    }

    Value *Gep = Builder.CreateInBoundsGEP(
        KmpTaskDependInfoTy, BaseTaskTDependGep,
        {Builder.getInt32(0), Builder.getInt32(0)}, ".dep.base.ptr");
    Builder.CreateStore(Builder.CreatePtrToInt(BasePtr, IntPtrTy), Gep);

    Gep = Builder.CreateInBoundsGEP(KmpTaskDependInfoTy, BaseTaskTDependGep,
                                    {Builder.getInt32(0), Builder.getInt32(1)},
                                    ".dep.num.bytes");
    Builder.CreateStore(Size, Gep);

    Gep = Builder.CreateInBoundsGEP(KmpTaskDependInfoTy, BaseTaskTDependGep,
                                    {Builder.getInt32(0), Builder.getInt32(2)},
                                    ".dep.flags");
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
  unsigned Idx = 0;
  for (ReductionItem *RedI : RedClause.items()) {

    // For non-taskgroups, computeArraySectionTypeOffsetSize is called as part
    // of genTaskInitCode/genTaskLoopInitCode.
    if (isa<WRNTaskgroupNode>(W) && RedI->getIsArraySection())
      computeArraySectionTypeOffsetSize(W, *RedI, InsertBefore);

    StringRef NamePrefix = RedI->getOrig()->getName();

    Value *BaseTaskTRedGep = Builder.CreateInBoundsGEP(
        KmpTaskTTRedRecTy, DummyTaskTRedRec, {Zero, Builder.getInt32(Count++)},
        NamePrefix + ".red.struct");

    Value *Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                           {Zero, Builder.getInt32(Idx++)},
                                           NamePrefix + ".red.item");

    // The reduction item struct needs to store the starting address of the
    // actual data to be reduced (after any offset computation, or  dereference
    // for by-refs etc.)
    Value *RedIBase = RedI->getOrig();
    Type *ElementType = nullptr;
    Value *NumElements = nullptr;
    std::tie(ElementType, NumElements, std::ignore) =
        VPOParoptUtils::getItemInfo(RedI);

    if (RedI->getIsByRef())
      RedIBase = Builder.CreateLoad(
          ElementType->getPointerTo(
              isTargetSPIRV() ? vpo::ADDRESS_SPACE_GENERIC : 0),
          RedIBase, NamePrefix + Twine(".orig.deref"));

    if (RedI->getIsArraySection()) {
      const ArraySectionInfo &ArrSecInfo = RedI->getArraySectionInfo();
      RedIBase = genBasePlusOffsetGEPForArraySection(RedIBase, ArrSecInfo,
                                                     InsertBefore);
    }
    Builder.CreateStore(Builder.CreateBitCast(RedIBase, Type::getInt8PtrTy(C)),
                        Gep);

    if (!(Mode & OmpTbb)) {
      // The new task reduction API (kmpc_taskred_init) allows keeping a pointer
      // to the original reduction item in the second field, in case it is
      // needed in a call to the reduction init function for UDR.
      Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                      {Zero, Builder.getInt32(Idx++)},
                                      NamePrefix + ".red.orig");
      Builder.CreateStore(
          Builder.CreateBitCast(RedIBase, Type::getInt8PtrTy(C)), Gep);
    }

    Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                    {Zero, Builder.getInt32(Idx++)},
                                    NamePrefix + ".red.size");

    Value *ElementSize = Builder.getInt64(DL.getTypeAllocSize(ElementType));

    Value *Size = nullptr;
    Size = NumElements ? Builder.CreateMul(ElementSize, NumElements,
                                           NamePrefix + ".red.size")
                       : ElementSize;
    Builder.CreateStore(Size, Gep);

    Function *RedInitFunc = genTaskLoopRedInitFunc(W, RedI);
    Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                    {Zero, Builder.getInt32(Idx++)},
                                    NamePrefix + ".red.init");
    Builder.CreateStore(
        Builder.CreateBitCast(RedInitFunc, Type::getInt8PtrTy(C)), Gep);

    Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                    {Zero, Builder.getInt32(Idx++)},
                                    NamePrefix + ".red.fini");
    Builder.CreateStore(ConstantPointerNull::get(Type::getInt8PtrTy(C)), Gep);

    Function *RedCombFunc = genTaskLoopRedCombFunc(W, RedI);
    Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                    {Zero, Builder.getInt32(Idx++)},
                                    NamePrefix + ".red.comb");
    Builder.CreateStore(
        Builder.CreateBitCast(RedCombFunc, Type::getInt8PtrTy(C)), Gep);

    Gep = Builder.CreateInBoundsGEP(KmpTaskTRedTy, BaseTaskTRedGep,
                                    {Zero, Builder.getInt32(Idx++)},
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
  Value *DepArray = W->getDepArray();

  if (DepArray) {
    Value *NumDeps = W->getDepArrayNumDeps();
    assert(NumDeps && "Corrupt DEPARRAY IR");
    assert(DepClause.empty() && "Cannot have both DEPEND and DEPARRAY IR");
    resetValueInOmpClauseGeneric(W, DepArray);
    if (!isa<ConstantInt>(NumDeps)) // usually constant if DEPOBJ is not used
      resetValueInOmpClauseGeneric(W, NumDeps);
    return;
  }

  if (DepClause.empty())
    return;
  for (DependItem *DepI : DepClause.items()) {
    resetValueInOmpClauseGeneric(W, DepI->getOrig());
    if (DepI->getIsArraySection()) {
      const auto &ArraySectionDims =
          DepI->getArraySectionInfo().getArraySectionDims();

      for (const auto &Dim : ArraySectionDims) {
        resetValueInOmpClauseGeneric(W, std::get<0>(Dim));
        resetValueInOmpClauseGeneric(W, std::get<1>(Dim));
        resetValueInOmpClauseGeneric(W, std::get<2>(Dim));
      }
    }
  }
}

// The wrapper routine to generate the call __kmpc_omp_task_with_deps
void VPOParoptTransform::genTaskDeps(WRegionNode *W, StructType *IdentTy,
                                     Value *TidPtr, Value *TaskAlloc,
                                     AllocaInst *DummyTaskTDependRec,
                                     Instruction *InsertPt, bool IsTaskWait) {
  Value *Dep = W->getDepArray(); // pointer to the beginning of the dep array
  Value *NumDeps = nullptr;  // number of elements in the dep array

  // The QUAL.OMP.DEPEND and the QUAL.OMP.DEPARRAY forms cannot be mixed
  // in the same DIR.OMP.TASK.

  if (Dep) {
    // The QUAL.OMP.DEPARRAY form is used.
    NumDeps = W->getDepArrayNumDeps();
    assert(NumDeps && "Corrupt DEPARRAY IR: missing NumDeps");
    assert(!DummyTaskTDependRec && "Cannot have both DEPEND and DEPARRAY IR");
  } else {
    // The QUAL.OMP.DEPEND form is used.
    // TODO: remove this codepath when both FEs emit
    //       the QUAL.OMP.DEPARRAY form by default.
    assert(DummyTaskTDependRec && "Missing depend info array");

    IRBuilder<> Builder(InsertPt);
    Value *BaseTaskTDependGep = Builder.CreateInBoundsGEP(
        DummyTaskTDependRec->getAllocatedType(), DummyTaskTDependRec,
        {Builder.getInt32(0), Builder.getInt32(0)});
    LLVMContext &C = F->getContext();
    Dep = Builder.CreateBitCast(BaseTaskTDependGep, Type::getInt8PtrTy(C));
    DependClause const &DepClause = W->getDepend();
    NumDeps = Builder.getInt32(DepClause.size());
  }

  if (!IsTaskWait)
    VPOParoptUtils::genKmpcTaskWithDeps(W, IdentTy, TidPtr, TaskAlloc, Dep,
                                        NumDeps, InsertPt);
  else
    VPOParoptUtils::genKmpcTaskWaitDeps(W, IdentTy, TidPtr, Dep, NumDeps,
                                        InsertPt);
}

// Generate the call __kmpc_omp_task_alloc, __kmpc_taskloop_5 or
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
                                            AllocaInst *LBPtr,
                                            AllocaInst *UBPtr,
                                            AllocaInst *STPtr, bool isLoop) {

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTaskGenericCode\n");

  assert(W->getIsTask() && "genTaskGenericCode() expected a task or taskloop");

  W->populateBBSet();

  resetValueInOmpClauseGeneric(W, W->getIf());
  resetValueInOmpClauseGeneric(W, W->getFinal());
  resetValueInOmpClauseGeneric(W, W->getPriority());
  resetValueInTaskDependClause(W);
  if (isa<WRNTaskloopNode>(W)) {
    resetValueInOmpClauseGeneric(W, W->getNumTasks());
    resetValueInOmpClauseGeneric(W, W->getGrainsize());
  }

  AllocaInst *SharedAggrStruct = genAndPopulateTaskSharedStruct(W, KmpSharedTy);

  // Set up Fn Attr for the new function
  Function *NewF = VPOParoptUtils::genOutlineFunction(*W, DT, AC);

  CallInst *NewCall = cast<CallInst>(NewF->user_back());

  // TidArgNo parameter is unused, if IsTidArg is false.
  Function *MTFn = finalizeExtractedMTFunction(W, NewF, false, -1U, false);

#if INTEL_CUSTOMIZATION
  // Uncomment after the JIRA CMPLRLLVM-21925 is fixed.
  // assert(MTFn->arg_size() <= 2 &&
  //       "Outlined function for TaskLoop cannot have more than 2 arguments.");
#endif // INTEL_CUSTOMIZATION

  std::vector<Value *> MTFnArgs;

  LLVMContext &C = NewF->getContext();
  IntegerType *Int32Ty = Type::getInt32Ty(C);
  ConstantInt *ValueZero = ConstantInt::getSigned(Int32Ty, 0);
  MTFnArgs.push_back(ValueZero);
  genThreadedEntryActualParmList(W, MTFnArgs);

  for (auto I = NewCall->arg_begin(), E = NewCall->arg_end(); I != E; ++I) {
    MTFnArgs.push_back((*I));
  }
  CallInst *MTFnCI =
      CallInst::Create(MTFn->getFunctionType(), MTFn, MTFnArgs, "", NewCall);
  MTFnCI->setCallingConv(NewCall->getCallingConv());

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

  Value *TotalTaskTTWithPrivatesSize =
      computeExtraBufferSpaceNeededAfterEndOfTaskThunk(
          W, KmpTaskTTWithPrivatesTySz, NewCall);

  Function *DestrThunk = genTaskDestructorThunk(W, KmpTaskTTWithPrivatesTy);
  if (DestrThunk)
    W->setTaskFlag(W->getTaskFlag() | WRNTaskFlag::DtorThunk);     // 0x08

  if (W->getPriority())
    W->setTaskFlag(W->getTaskFlag() | WRNTaskFlag::PriorityUsed);  // 0x20

  CallInst *TaskAllocCI = VPOParoptUtils::genKmpcTaskAlloc(
      W, IdentTy, TidPtrHolder, DT, TotalTaskTTWithPrivatesSize, KmpSharedTySz,
      KmpRoutineEntryPtrTy, MTFnCI->getCalledFunction(), NewCall,
      Mode & OmpTbb);
  TaskAllocCI->setName(".task.alloc");

  copySharedStructToTaskThunk(W, SharedAggrStruct, TaskAllocCI, KmpSharedTy,
                              KmpTaskTTWithPrivatesTy, DestrThunk, NewCall);

  StructType *KmpPrivatesTy =
      dyn_cast<StructType>(KmpTaskTTWithPrivatesTy->getElementType(1));
  Value *PrivatesGep =
      genPrivatesGepForTask(TaskAllocCI, KmpTaskTTWithPrivatesTy, NewCall);

  saveVLASizeAndOffsetToPrivatesThunk(W, PrivatesGep, KmpPrivatesTy, NewCall);
#if INTEL_CUSTOMIZATION
  VPOParoptUtils::genF90DVInitForItemsInTaskPrivatesThunk(
      W, PrivatesGep, KmpPrivatesTy, NewCall);
#endif // INTEL_CUSTOMIZATION
  genFprivInitForTask(W, TaskAllocCI, PrivatesGep, KmpPrivatesTy, NewCall);

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
      if (!DummyTaskTDependRec && !W->getDepArray())
        VPOParoptUtils::genKmpcTask(W, IdentTy, TidPtrHolder, TaskAllocCI,
                                    NewCall);
      else
        genTaskDeps(W, IdentTy, TidPtrHolder, TaskAllocCI,
                    DummyTaskTDependRec, NewCall, false);
    } else {

      Instruction *ThenTerm, *ElseTerm;

      VPOParoptUtils::buildCFGForIfClause(Cmp, ThenTerm, ElseTerm, NewCall, DT);
      IRBuilder<> ElseBuilder(ElseTerm);
      if (!DummyTaskTDependRec && !W->getDepArray())
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
      MTFnArgs.push_back(ElseBuilder.CreateLoad(Int32Ty, TidPtrHolder));
      if (isTargetSPIRV())
        MTFnArgs.push_back(ElseBuilder.CreateAddrSpaceCast(
            TaskAllocCI, PointerType::get(KmpTaskTTWithPrivatesTy,
                                          vpo::ADDRESS_SPACE_GENERIC)));
      else
        MTFnArgs.push_back(ElseBuilder.CreateBitCast(
            TaskAllocCI, PointerType::getUnqual(KmpTaskTTWithPrivatesTy)));
      CallInst *SeqCI = CallInst::Create(MTFn->getFunctionType(), MTFn,
                                         MTFnArgs, "", ElseTerm);
      SeqCI->setCallingConv(NewCall->getCallingConv());
      SeqCI->takeName(NewCall);
      SeqCI->setDebugLoc(NewCall->getDebugLoc());
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
  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTaskWaitCode\n");

  DependClause const &DepClause = W->getDepend();

  if (!DepClause.empty() || W->getDepArray()) {
    Instruction *InsertPt = W->getEntryBBlock()->getTerminator();
    AllocaInst *DummyTaskTDependRec = genDependInitForTask(W, InsertPt);
    genTaskDeps(W, IdentTy, TidPtrHolder, /*TaskAlloc=*/nullptr,
                DummyTaskTDependRec, InsertPt, true);
  } else {
    VPOParoptUtils::genKmpcTaskWait(W, IdentTy, TidPtrHolder,
                                    W->getEntryBBlock()->getTerminator());
  }
  return true;
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
  VPOParoptUtils::addFuncletOperandBundle(TaskgroupCI, W->getDT());

  Instruction *InsertEndPt = ExitBB->getTerminator();

  CallInst *EndTaskgroupCI = VPOParoptUtils::genKmpcEndTaskgroupCall(
      W, IdentTy, TidPtrHolder, InsertEndPt);
  EndTaskgroupCI->insertBefore(InsertEndPt);
  VPOParoptUtils::addFuncletOperandBundle(EndTaskgroupCI, W->getDT());

  W->resetBBSet();
  return true;
}
#endif // INTEL_COLLAB
