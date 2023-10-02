//==== DTransNormalizeOP.cpp - Normalize IR for the DTransSafetyAnalyzer ====//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//
// This file defines the pass that normalizes the IR to reduce instances where
// safety flags will be set by the DTransSafetyAnalyzer.
//
// 1: Conversion of element zero accesses to be GEP based.
// Example:
// From:
//   %struct.test = type { i32 }
//   %ptr_to_struct.test = alloca %struct.test
//   %x = load i32, ptr %ptr_to_struct.test
// To:
//   %ptr_to_struct.test = alloca %struct.test
//   %dtnorm = getelementptr %struct.test, ptr %ptr_to_struct.test, i64 0, i32 0
//   %x = load i32, ptr %dtnorm
// Example:
// From:
//   %struct.Y = type { [7 x ptr] }
//   %struct.X = type { i64, %struct.Y }
//   %x1 = getelementptr inbounds %struct.X, ptr %arg, i64 0, i32 1
//   %y2 = getelementptr inbounds [7 x ptr], ptr %w, i64 0, i64 0
//   %63 = phi ptr [ %x1, %B0 ], [ %y2, %B1 ]
// To:
//   %x1 = getelementptr inbounds %struct.X, ptr %arg, i64 0, i32 1
//   %dtnorm = getelementptr inbounds [7 x ptr], ptr %x1, i64 0, i32 0
//   %y2 = getelementptr inbounds [7 x ptr], ptr %w, i64 0, i64 0
//   %63 = phi ptr [ %dtnorm, %B0 ], [ %y2, %B1 ]
// 2: This pass could be extended in the future to support other cases, such as
//    replacing byte-flattened GEPs.
//===---------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/DTransNormalizeOP.h"

#include "Intel_DTrans/Analysis/DTransAllocCollector.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

using namespace llvm;
using namespace dtransOP;

#define DEBUG_TYPE "dtrans-normalizeop"

class DTransNormalizeImpl {
public:
  DTransNormalizeImpl(Module &M, PtrTypeAnalyzer &PtrAnalyzer,
                      TypeMetadataReader &MDReader, DTransAllocCollector &DTAC,
                      DTransNormalizeOPPass::GetTLIFn GetTLI)
      : M(M), PtrAnalyzer(PtrAnalyzer), MDReader(MDReader), DTAC(DTAC),
        GetTLI(GetTLI) {}

  bool run() {

    LLVMContext &Ctx = M.getContext();
    Zero32 = ConstantInt::get(llvm::Type::getInt32Ty(Ctx), 0);
    ZeroPtrSizedInt = ConstantInt::get(
        llvm::Type::getIntNTy(Ctx, M.getDataLayout().getPointerSizeInBits()),
        0);

    for (auto &F : M) {
      for (inst_iterator It = inst_begin(F), E = inst_end(F); It != E; ++It) {
        Instruction *I = &*It;
        if (isa<LoadInst>(I) || isa<StoreInst>(I)) {
          bool NormalizedPtr = checkPointer(I, getLoadStorePointerOperand(I));
          if (!NormalizedPtr)
            checkStoredValue(I);
        } else if (auto PHIN = dyn_cast<PHINode>(I)) {
          checkPHI(PHIN);
        } else if (auto *Ret = dyn_cast<ReturnInst>(I)) {
          checkReturnInst(Ret);
        } else if (auto *GEPI = dyn_cast<GetElementPtrInst>(I)) {
          checkGEPInst(GEPI);
        } else if (auto *CB = dyn_cast<CallBase>(I)) {
          checkCallBase(CB);
        } else if (auto *Int2Ptr = dyn_cast<IntToPtrInst>(I)) {
          checkInt2Ptr(Int2Ptr);
        }
      }
    }

    if (InstructionsToGepify.empty() && PHIsToGepify.empty() &&
        ReturnsToGepify.empty() && PHIReturnsGepify.empty() &&
        GEPsToGepify.empty() && CallBaseArgsGepify.empty() &&
        StoreValueGepify.empty() && AddGepify.empty())
      return false;

    for (auto &KV : InstructionsToGepify)
      gepify(std::get<0>(KV), std::get<1>(KV), std::get<2>(KV));

    for (auto &KV : PHIsToGepify)
      gepifyPHI(KV.first, KV.second);

    for (auto &KV : ReturnsToGepify)
      gepifyReturn(KV.first, KV.second);

    for (auto &KV : PHIReturnsGepify)
      gepifyPHIReturn(KV.first, KV.second);

    for (auto &KV : GEPsToGepify)
      gepifyGEPI(KV.first, KV.second);

    for (auto &KV : CallBaseArgsGepify)
      gepifyCallBaseArgs(KV.first, KV.second);

    for (auto &KV : StoreValueGepify)
      gepifyStoreValue(KV.first, KV.second);

    for (auto &KV : AddGepify)
      gepifyAdd(std::get<0>(KV), std::get<1>(KV), std::get<2>(KV),
                std::get<3>(KV), std::get<4>(KV), std::get<5>(KV));

    return true;
  }

private:
  Module &M;
  PtrTypeAnalyzer &PtrAnalyzer;
  TypeMetadataReader &MDReader;
  DTransAllocCollector &DTAC;
  DTransNormalizeOPPass::GetTLIFn GetTLI;

  static bool isCompilerConstantData(Value *V) { return isa<ConstantData>(V); }

  bool checkPointer(Instruction *I, Value *Ptr) {

    // Check if (GEP, 0, 0) is really needed and return true if not needed.
    auto IsNormalizationNotNeeded = [&](Value *Ptr) {
      // Let us assume %arg is a pointer to ValueVectorOf. Checks if
      // (GEP, 0, 0) is needed for "%i".
      // %ValueVectorOf = type { i8, i32, i32, ptr, ptr }
      //
      // define void @foo(ptr %arg) {
      //   %i = getelementptr %ValueVectorOf, ptr %arg, i64 0, i32 0
      //   store i8 0, ptr %i, align 8
      //   ...
      // }

      if (auto *GEPI = dyn_cast<GetElementPtrInst>(Ptr)) {
        if (!GEPI->hasAllZeroIndices())
          return false;
        if (auto *Arg = dyn_cast<Argument>(GEPI->getPointerOperand())) {
          unsigned ArgNo = Arg->getArgNo();
          DTransType *ArgDeclTy = getArgumentTypeFromDType(
              MDReader.getDTransTypeFromMD(Arg->getParent()), ArgNo);
          DTransType *ArgDeclStTy = getPointeeDTransStructTy(ArgDeclTy);
          if (ArgDeclStTy &&
              ArgDeclStTy->getLLVMType() == GEPI->getSourceElementType() &&
              !cast<DTransStructType>(ArgDeclStTy)
                   ->getFieldType(0)
                   ->isAggregateType())
            return true;
        }
      }
      return false;
    };

    if (isCompilerConstantData(Ptr))
      return false;

    ValueTypeInfo *PtrInfo = PtrAnalyzer.getValueTypeInfo(Ptr);
    assert(PtrInfo &&
           "PtrTypeAnalyzer failed to construct ValueTypeInfo for load "
           "pointer operand");

    if (PtrInfo->getUnhandled() || PtrInfo->getDependsOnUnhandled())
      return false;

    PtrTypeAnalyzer::ElementZeroInfo Info =
        PtrAnalyzer.getElementZeroPointer(I);
    DTransType *Ty = Info.Ty;
    if (!Ty)
      return false;

    if ((Ty->isStructTy() &&
         cast<DTransStructType>(Ty)->getNumContainedElements() == 0) ||
        (Ty->isArrayTy() &&
         cast<DTransArrayType>(Ty)->getNumContainedElements() == 0))
      return false;

    if (IsNormalizationNotNeeded(Ptr))
      return false;

    InstructionsToGepify.insert({I, Info.Ty, Info.Depth});
    return true;
  }

  void gepify(Instruction *I, DTransType *Ty, unsigned int Depth) {
    assert(isa<LoadInst>(I) || isa<StoreInst>(I));
    Value *Ptr = getLoadStorePointerOperand(I);
    SmallVector<Value *, 2> IdxList;
    IdxList.push_back(ZeroPtrSizedInt);
    for (unsigned int Count = 0; Count < Depth; ++Count)
      IdxList.push_back(Zero32);
    auto *GEP =
        GetElementPtrInst::Create(Ty->getLLVMType(), Ptr, IdxList, "dtnorm", I);
    if (auto *LI = dyn_cast<LoadInst>(I)) {
      LLVM_DEBUG(dbgs() << "Replacing pointer operand in: " << *LI
                        << "\nWith: " << *GEP);
      LI->replaceUsesOfWith(LI->getPointerOperand(), GEP);
    } else if (auto *SI = dyn_cast<StoreInst>(I)) {
      if (SI->getValueOperand() == SI->getPointerOperand())
        return;
      LLVM_DEBUG(dbgs() << "Replacing pointer operand in: " << *SI
                        << "\nWith: " << *GEP);
      SI->replaceUsesOfWith(SI->getPointerOperand(), GEP);
    } else {
      llvm_unreachable("Only Load/Store currently allowed");
    }
  }

  GetElementPtrInst *createGEPToAccessZeroElement(Value *Ptr, DTransType *DTy) {
    SmallVector<Value *, 2> IdxList;
    IdxList.push_back(ZeroPtrSizedInt);
    IdxList.push_back(Zero32);
    GetElementPtrInst *NewGEP = NewGEPInsts[Ptr];
    if (!NewGEP) {
      Instruction *NextI;
      if (isa<CallBase>(Ptr)) {
        NextI = cast<CallBase>(Ptr)->getNextNonDebugInstruction();
      } else if (isa<PHINode>(Ptr)) {
        NextI = cast<PHINode>(Ptr)->getParent()->getFirstNonPHI();
      } else {
        assert(isa<Argument>(Ptr) && "Expected Argument");
        NextI = cast<Argument>(Ptr)
                    ->getParent()
                    ->getEntryBlock()
                    .getFirstNonPHIOrDbg();
      }
      NewGEP = GetElementPtrInst::Create(DTy->getLLVMType(), Ptr, IdxList,
                                         "dtnorm", NextI);
      NewGEPInsts[Ptr] = NewGEP;
    }
    return NewGEP;
  }

  void gepifyPHIReturn(
      PHINode *PHI,
      SetVector<std::pair<unsigned, DTransType *>> &ValueToDTransTyMap) {
    for (auto &Entry : ValueToDTransTyMap) {
      Value *Ptr = PHI->getIncomingValue(Entry.first);
      GetElementPtrInst *NewGEP =
          createGEPToAccessZeroElement(Ptr, Entry.second);
      LLVM_DEBUG(dbgs() << "Replacing incoming value in: " << *PHI
                        << "\nWith: " << *NewGEP << "\n");
      PHI->replaceUsesOfWith(Ptr, NewGEP);
    }
  }

  void gepifyCallBaseArgs(
      CallBase *CB,
      SetVector<std::pair<unsigned, DTransType *>> &ValueToDTransTyMap) {
    for (auto &Entry : ValueToDTransTyMap) {
      Value *Ptr = CB->getArgOperand(Entry.first);
      GetElementPtrInst *NewGEP =
          createGEPToAccessZeroElement(Ptr, Entry.second);
      LLVM_DEBUG(dbgs() << "Replacing incoming value in: " << *CB
                        << "\nWith: " << *NewGEP << "\n");
      CB->setArgOperand(Entry.first, NewGEP);
    }
  }

  void gepifyReturn(ReturnInst *RetI, DTransType *DTy) {
    Value *Ptr = RetI->getReturnValue();
    GetElementPtrInst *NewGEP = createGEPToAccessZeroElement(Ptr, DTy);
    RetI->replaceUsesOfWith(Ptr, NewGEP);
    LLVM_DEBUG(dbgs() << "Replacing return value in: " << *RetI
                      << "\nWith: " << *NewGEP << "\n");
  }

  void gepifyGEPI(GetElementPtrInst *GEPI, DTransType *DTy) {
    Value *Ptr = GEPI->getPointerOperand();
    GetElementPtrInst *NewGEP = createGEPToAccessZeroElement(Ptr, DTy);
    GEPI->replaceUsesOfWith(Ptr, NewGEP);
    LLVM_DEBUG(dbgs() << "Replacing pointer operand in: " << *GEPI
                      << "\nWith: " << *NewGEP << "\n");
  }

  // A New (GEP, 0, 0) is created for each use of allocation in different
  // store instruction and will not try to reuse by saving them in NewGEPInsts
  // only when value operand of store is InvokeInst.
  void gepifyStoreValue(StoreInst *SI, DTransType *DTy) {
    Value *Val = SI->getValueOperand();
    GetElementPtrInst *NewGEP;
    if (isa<InvokeInst>(Val)) {
      SmallVector<Value *, 2> IdxList;
      IdxList.push_back(ZeroPtrSizedInt);
      IdxList.push_back(Zero32);

      NewGEP = GetElementPtrInst::Create(
          DTy->getLLVMType(), SI->getValueOperand(), IdxList, "dtnorm", SI);
    } else {
      NewGEP = createGEPToAccessZeroElement(Val, DTy);
    }

    SI->setOperand(0, NewGEP);
    LLVM_DEBUG(dbgs() << "Replacing return value in: " << *SI
                      << "\nWith: " << *NewGEP << "\n");
  }

  void checkPHI(PHINode *PHIN) {

    auto GetCandidateType = [this](Value *V) -> Type * {
      if (auto PHI = dyn_cast<PHINode>(V))
        return PHIType.lookup(PHI);
      if (auto GEPI = dyn_cast<GetElementPtrInst>(V))
        return GEPI->getSourceElementType();
      return nullptr;
    };

    auto SimpleStructGEPI = [](Value *V) -> Type * {
      auto GEPI = dyn_cast<GetElementPtrInst>(V);
      if (!GEPI || GEPI->getNumIndices() != 2)
        return nullptr;
      auto CI0 = dyn_cast<ConstantInt>(GEPI->getOperand(1));
      if (!CI0 || !CI0->isZero())
        return nullptr;
      auto CI1 = dyn_cast<ConstantInt>(GEPI->getOperand(2));
      if (!CI1)
        return nullptr;
      auto Ty = dyn_cast<StructType>(GEPI->getSourceElementType());
      if (!Ty)
        return nullptr;
      auto STy = dyn_cast<StructType>(Ty->getTypeAtIndex(CI1->getZExtValue()));
      if (!STy || STy->getNumElements() == 0)
        return nullptr;
      return STy->getTypeAtIndex((unsigned)0);
    };

    auto TestAndInsert = [this, SimpleStructGEPI](PHINode *PHIN, unsigned Num,
                                                  Value *GEPI0,
                                                  Type *Ty) -> bool {
      if (auto ATy0 = dyn_cast_or_null<ArrayType>(SimpleStructGEPI(GEPI0)))
        if (auto ATy1 = dyn_cast<ArrayType>(Ty))
          if (ATy0 == ATy1) {
            PHIsToGepify.insert({PHIN, Num});
            PHIType.insert({PHIN, Ty});
            return true;
          }
      return false;
    };

    if (checkPHIReturn(PHIN))
      return;

    if (PHIN->getNumIncomingValues() != 2)
      return;
    auto V0 = PHIN->getIncomingValue(0);
    auto V1 = PHIN->getIncomingValue(1);
    auto Ty0 = GetCandidateType(V0);
    auto Ty1 = GetCandidateType(V1);
    if (!Ty0 || !Ty1)
      return;
    if (Ty0 == Ty1) {
      PHIType.insert({PHIN, Ty0});
      return;
    }
    if (TestAndInsert(PHIN, 0, V0, Ty1))
      return;
    if (TestAndInsert(PHIN, 1, V1, Ty0))
      return;
  }

  // If return type of "F" is pointer to struct, this function returns
  // type of the struct.
  Type *getFunctionReturnPointeeTy(Function *F) {
    DTransType *DType = MDReader.getDTransTypeFromMD(F);
    if (!DType)
      return nullptr;
    auto *FnType = cast<DTransFunctionType>(DType);
    DTransType *DRetTy = FnType->getReturnType();
    if (!DRetTy->isPointerTy())
      return nullptr;
    auto DTStTy = dyn_cast<DTransStructType>(DRetTy->getPointerElementType());
    if (!DTStTy)
      return nullptr;
    return DTStTy->getLLVMType();
  }

  // For given AllocPtrInfo, checks that only DomTy and its sub-object are
  // aliases and returns sub-object alias type. Ex: For example, given the
  // types:
  //   %struct.derived = type { %struct.base }
  //
  // Let us assume input alias set of of AllocPtrInfo:
  //  %struct.derived*, %struct.base*, i8*
  //
  // When DomTy is %struct.derived*, this function returns %struct.base*.
  DTransType *getSubObjAliasTy(ValueTypeInfo &AllocPtrInfo, DTransType *DomTy) {

    // Returns true of DTy is pointer to integer type.
    auto IsIntPointer = [](DTransType *DTy) {
      DTransType *Base = DTy;
      while (Base->isPointerTy())
        Base = Base->getPointerElementType();
      if (Base->isIntegerTy())
        return true;
      return false;
    };

    DTransType *SubObjStTy = nullptr;
    // Check if DomTy is pointer to struct.
    if (!DomTy->isPointerTy())
      return nullptr;
    auto DomStTy = dyn_cast<DTransStructType>(DomTy->getPointerElementType());
    if (!DomStTy)
      return nullptr;
    if (!DomStTy->indexValid(0u))
      return nullptr;

    for (auto *AliasTy :
         AllocPtrInfo.getPointerTypeAliasSet(ValueTypeInfo::VAT_Use)) {
      if (!AliasTy->isPointerTy())
        return nullptr;
      DTransType *BTy = AliasTy->getPointerElementType();
      // Skip pointer to non-aggregate types.
      if (IsIntPointer(BTy))
        continue;
      if (AliasTy == DomTy)
        continue;
      // Check if BTy is sub-object of DomTy at offset 0.
      if (!isSubObject(DomStTy, BTy))
        return nullptr;
      if (SubObjStTy)
        return nullptr;
      SubObjStTy = AliasTy;
    }
    return SubObjStTy;
  }

  // Returns true if %Val is an allocation call and "UsedTy" doesn't match with
  // dominant aggregate type of %Val.
  //
  // Ex: For example, given the types:
  //   %struct.derived = type { %struct.base }
  //
  // Let us assume alias set of %Val is %struct.derived* and %struct.base*.
  //
  //  %Val = AllocCall();
  //   ...
  //  %p = getelementptr %struct.base, ptr %Val, 0, 2
  //
  // Return true for this example as %Val is used as %struct.base.
  //
  bool isNormalizedGEPNeeded(Value *Val, Type *UsedTy, DTransType **DomStTyPtr,
                             bool AllowInvoke = false) {
    auto Call = dyn_cast<CallBase>(Val);
    if (!Call)
      return false;

    if (!isa<CallInst>(Call) && !(AllowInvoke && isa<InvokeInst>(Call)))
      return false;

    ValueTypeInfo *PtrInfo = PtrAnalyzer.getValueTypeInfo(Val);
    if (!PtrInfo)
      return false;
    if (PtrInfo->getUnhandled() || PtrInfo->getDependsOnUnhandled())
      return false;

    const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
    dtrans::AllocKind AKind = DTAC.getAllocFnKind(Call, TLI);
    if (AKind == dtrans::AK_NotAlloc)
      return false;
    DTransType *DomTy = PtrAnalyzer.getDominantAggregateUsageType(*PtrInfo);
    if (!DomTy)
      return false;
    // Normalization is not needed if UsedTy is same as DomTy.
    DTransType *DomStTy = DomTy->getPointerElementType();
    if (DomStTy->getLLVMType() == UsedTy)
      return false;
    DTransType *ATy = getSubObjAliasTy(*PtrInfo, DomTy);
    // Check if SubObj type is used.
    if (!ATy || ATy->getPointerElementType()->getLLVMType() != UsedTy)
      return false;
    *DomStTyPtr = DomStTy;
    return true;
  }

  // Check if (GEP, 0, 0) is needed if return value is memory allocation
  // pointer. Example: From:
  //   %struct.derived = type { %struct.base }
  //   @foo() {
  //      %i = tail call ptr @Alloc(i64 0, ptr null)
  //      call void @Ctor1(ptr %i)
  //      ret ptr %i
  //   }
  //   Let us assume aliases of %i are %struct.base* and %struct.derived* and
  //   return type of foo is %struct.base*.
  //
  // To:
  //   %struct.derived = type { %struct.base }
  //   @foo() {
  //      %i = tail call ptr @Alloc(i64 0, ptr null)
  //      call void @Ctor1(ptr %i)
  //      %dtnorm = getelementptr %%struct.derived, ptr %i, i64 0, i32 0
  //      ret ptr %dtnorm
  //   }
  //
  void checkReturnInst(ReturnInst *Ret) {
    Value *RetVal = Ret->getReturnValue();
    if (!RetVal || isCompilerConstantData(RetVal))
      return;
    Type *StTy = getFunctionReturnPointeeTy(Ret->getFunction());
    if (!StTy)
      return;

    DTransType *DomStTy;
    if (!isNormalizedGEPNeeded(RetVal, StTy, &DomStTy))
      return;
    ReturnsToGepify.insert({Ret, DomStTy});
  }

  // Get dominant type for PHI if there is one. PHI is used
  // as "UsedStTy".
  DTransType *getPHIDominantType(PHINode *PHI, Type *UsedStTy) {
    bool AllocFound = false;
    DTransType *PHIValTy = nullptr;
    DTransType *DomTy = nullptr;
    for (unsigned I = 0; I < PHI->getNumIncomingValues(); I++) {
      Value *Val = PHI->getIncomingValue(I);
      if (isCompilerConstantData(Val))
        continue;
      if (isNormalizedGEPNeeded(Val, UsedStTy, &DomTy)) {
        // Allow only single allocation.
        if (AllocFound)
          return nullptr;
        AllocFound = true;
      } else {
        // For non-allocation values, makes sure there is only
        // one alias type.
        ValueTypeInfo *Info = PtrAnalyzer.getValueTypeInfo(Val);
        if (!Info)
          return nullptr;
        if (Info->getUnhandled() || Info->getDependsOnUnhandled())
          return nullptr;
        auto &UseAliases = Info->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use);
        if (UseAliases.size() != 1)
          return nullptr;
        DTransType *AliasTy = *UseAliases.begin();
        if (!AliasTy->isPointerTy())
          return nullptr;
        DomTy = AliasTy->getPointerElementType();
        if (!isa<DTransStructType>(DomTy))
          return nullptr;
      }

      // Makes sure types of all incoming values are same.
      if (!PHIValTy)
        PHIValTy = DomTy;
      else if (PHIValTy != DomTy)
        return nullptr;
    }
    if (!AllocFound)
      return nullptr;
    return PHIValTy;
  }

  // Check if (GEP, 0, 0) is needed if return value is PHI where all incoming
  // value of PHI are memory allocation calls.
  //
  // Example:
  // From:
  //   %struct.derived = type { %struct.base }
  //   @foo() {
  //     %i = tail call ptr @Alloc(i64 0, ptr null)
  //     invoke void @Ctor1(ptr %i)
  //     ...
  //     %i5 = phi ptr [ %i, %bb1 ], [ null, %bb ]
  //     ret ptr %i5
  //   }
  //   Let us assume aliases of %i are %struct.base* and %struct.derived* and
  //   return type of foo is %struct.base*.
  //
  // To:
  //   %struct.derived = type { %struct.base }
  //   @foo() {
  //     %i = tail call ptr @Alloc(i64 0, ptr null)
  //     %dtnorm = getelementptr %struct.derived, ptr %i, i64 0, i32 0
  //     invoke void @Ctor1(ptr %i)
  //     ...
  //     %i5 = phi ptr [ %dtnorm, %bb1 ], [ null, %bb ]
  //     ret ptr %i5
  //   }
  bool checkPHIReturn(PHINode *PHIN) {
    if (!PHIN->hasOneUse())
      return false;
    auto *RetI = dyn_cast<ReturnInst>(*PHIN->user_begin());
    if (!RetI)
      return false;
    ValueTypeInfo *PtrInfo = PtrAnalyzer.getValueTypeInfo(PHIN);
    if (!PtrInfo)
      return false;
    if (PtrInfo->getUnhandled() || PtrInfo->getDependsOnUnhandled())
      return false;

    Type *StTy = getFunctionReturnPointeeTy(PHIN->getFunction());
    if (!StTy)
      return false;

    SetVector<std::pair<unsigned, DTransType *>> ValueToDomTyMap;
    SmallPtrSet<CallBase *, 8> AllocCallsSet;

    for (unsigned I = 0; I < PHIN->getNumIncomingValues(); I++) {
      Value *Val = PHIN->getIncomingValue(I);
      if (isCompilerConstantData(Val))
        continue;
      DTransType *DomTy;
      if (!isNormalizedGEPNeeded(Val, StTy, &DomTy))
        return false;

      ValueToDomTyMap.insert({I, DomTy});
      AllocCallsSet.insert(cast<CallBase>(Val));
    }
    // Makes sure allocation calls are not duplicated.
    if (ValueToDomTyMap.size() == 0 ||
        AllocCallsSet.size() != ValueToDomTyMap.size())
      return false;
    auto &ValueToDomSet = PHIReturnsGepify[PHIN];
    for (const auto &Entry : ValueToDomTyMap)
      ValueToDomSet.insert(Entry);
    return true;
  }

  // Check if (GEP, 0, 0) is needed if return value is PHI where all incoming
  // value of PHI are memory allocation calls.
  //
  // Example 1:
  // From:
  //   %struct.derived = type { %struct.base }
  //   @foo(ptr %arg) {
  //     %i = getelementptr inbounds %"DOMParser", ptr %arg, i64 0, i32 1
  //     %i1 = tail call ptr @Alloc(i64 40, ptr null)
  //     %i2 = getelementptr inbounds %struct.base, ptr %i1, i64 0, i32 1
  //     ...
  //   }
  //   Let us assume aliases of %i are %struct.base* and %struct.derived*.
  //
  // To:
  //   %struct.derived = type { %struct.base }
  //   @foo(ptr %arg) {
  //     %i = getelementptr inbounds %"DOMParser", ptr %arg, i64 0, i32 1
  //     %i1 = tail call ptr @Alloc(i64 40, ptr null)
  //     %dtnorm = getelementptr %struct.derived, ptr %i1, i64 0, i32 0
  //     %i2 = getelementptr inbounds %struct.base, ptr %dtnorm, i64 0, i32 1
  //     ...
  //   }
  //
  // Example 2:
  // From:
  //   %struct.derived = type { %struct.base }
  //   @foo(ptr %arg) {
  //   bb1:
  //     %i = getelementptr inbounds %Parser, ptr %arg, i64 0, i32 1
  //     %i2 = load ptr, ptr %i, align 8
  //     br i1 false, label %bb3, label %bb5
  //
  //   bb3:
  //     %i4 = tail call ptr @Alloc(i64 40, ptr null)
  //     store ptr %i4, ptr %i, align 8
  //     br label %bb5
  //
  //   bb5:
  //     %i6 = phi ptr [ %i4, %bb3 ], [ %i2, %bb1 ]
  //     %i7 = getelementptr inbounds %struct.base, ptr %i6, i64 0, i32 2
  //       ...
  //   }
  //   Let us assume aliases of %i are %struct.base* and %struct.derived*.
  //
  // To:
  //   %struct.derived = type { %struct.base }
  //   @foo(ptr %arg) {
  //   bb1:
  //     %i = getelementptr inbounds %Parser, ptr %arg, i64 0, i32 1
  //     %i2 = load ptr, ptr %i, align 8
  //     br i1 false, label %bb3, label %bb5
  //
  //   bb3:
  //     %i4 = tail call ptr @Alloc(i64 40, ptr null)
  //     store ptr %i4, ptr %i, align 8
  //     br label %bb5
  //
  //   bb5:
  //     %i6 = phi ptr [ %i4, %bb3 ], [ %i2, %bb1 ]
  //     %dtnorm = getelementptr %struct.derived, ptr %i6, i64 0, i32 0
  //     %i7 = getelementptr inbounds %struct.base, ptr %dtnorm, i64 0, i32 2
  //     ...
  //   }
  void checkGEPInst(GetElementPtrInst *GEPI) {
    DTransType *DomTy;
    if (GEPI->getNumIndices() != 2)
      return;
    Value *Val = GEPI->getPointerOperand();
    Type *StTy = GEPI->getSourceElementType();
    if (!isa<StructType>(StTy))
      return;
    if (isNormalizedGEPNeeded(Val, StTy, &DomTy)) {
      GEPsToGepify.insert({GEPI, DomTy});
      return;
    }
    auto *PHI = dyn_cast<PHINode>(Val);
    if (!PHI)
      return;

    DomTy = getPHIDominantType(PHI, StTy);
    if (!DomTy)
      return;
    GEPsToGepify.insert({GEPI, DomTy});
  }

  // Returns pointee if DTy points to a struct.
  DTransType *getPointeeDTransStructTy(DTransType *DTy) {
    if (!DTy || !DTy->isPointerTy())
      return nullptr;
    DTransType *PTy = DTy->getPointerElementType();
    if (!isa<DTransStructType>(PTy))
      return nullptr;
    return PTy;
  }

  // Get argument type from DType if possible.
  DTransType *getArgumentTypeFromDType(DTransType *DType, unsigned Idx) {
    if (!DType)
      return nullptr;
    auto *FnType = dyn_cast<DTransFunctionType>(DType);
    if (!FnType || Idx >= FnType->getNumArgs() || FnType->isVarArg())
      return nullptr;
    return FnType->getArgType(Idx);
  }

  // Get argument type of Callee if possible.
  DTransType *getCalleeArgumentType(CallBase *CB, unsigned Idx) {
    Function *F = dtrans::getCalledFunction(*CB);
    if (F && (F->isDeclaration() || Idx >= F->arg_size()))
      return nullptr;

    DTransType *DType = nullptr;
    if (CB->isIndirectCall() || CB->getMetadata("_Intel.Devirt.Call"))
      DType = MDReader.getDTransTypeFromMD(CB);
    else if (F)
      DType = MDReader.getDTransTypeFromMD(F);
    return getArgumentTypeFromDType(DType, Idx);
  }

  // Returns true if SUbTy is sub-object of DomTy at offset 0.
  bool isSubObject(DTransType *DomTy, DTransType *SubTy) {
    auto *DomStTy = dyn_cast<DTransStructType>(DomTy);
    auto *SubSty = dyn_cast<DTransStructType>(SubTy);
    if (!DomStTy || !SubSty || DomStTy->getNumFields() == 0)
      return false;
    DTransType *ElementZeroTy = DomStTy->getFieldType(0);
    auto *ElementZeroStTy = dyn_cast<DTransStructType>(ElementZeroTy);
    if (!ElementZeroStTy || !SubSty->compare(*ElementZeroStTy))
      return false;
    return true;
  }

  // Check if (GEP, 0, 0) is needed for arguments of calls.
  //
  // Example 1:
  // From:
  //   %derived = type { %base }
  //   @foo(ptr %arg) {
  //     %i1 = tail call ptr @bar(ptr %arg)
  //     ...
  //   }
  //   Let us assume type of 1st argument of foo is %derived* and type of
  //   1st argument of bar is base*.
  //
  // To:
  //   %derived = type { %base }
  //   @foo(ptr %arg) {
  //     %denorm = getelementptr inbounds %derived, ptr %arg, i64 0, i32 0
  //     %i1 = tail call ptr @bar(ptr %denorm)
  //     ...
  //   }
  //
  // Example 2:
  // From:
  //   %derived = type { %base }
  //   @foo(ptr %arg) {
  //     %i1 = tail call ptr @Alloc(i64 40, ptr null)
  //     ...
  //     tail call ptr @bar(ptr %i1)
  //     ...
  //   }
  //   Let us assume aliases of %i1 are %base* and %derived* and
  //   type of 1st argument of bar is %base*.
  //
  // To:
  //   %derived = type { %base }
  //   @foo(ptr %arg) {
  //     %i1 = tail call ptr @Alloc(i64 40, ptr null)
  //     %denorm = getelementptr inbounds %derived, ptr %i1, i64 0, i32 0
  //     ...
  //     tail call ptr @bar(ptr %denorm)
  //     ...
  //   }
  //
  // Example 3:
  // From:
  //   %struct.derived = type { %struct.base }
  //   @foo(ptr %arg) {
  //   bb1:
  //     %i = getelementptr inbounds %Parser, ptr %arg, i64 0, i32 1
  //     %i2 = load ptr, ptr %i, align 8
  //     br i1 false, label %bb3, label %bb5
  //
  //   bb3:
  //     %i4 = tail call ptr @Alloc(i64 40, ptr null)
  //     store ptr %i4, ptr %i, align 8
  //     br label %bb5
  //
  //   bb5:
  //     %i6 = phi ptr [ %i4, %bb3 ], [ %i2, %bb1 ]
  //     bar(ptr %i6)
  //       ...
  //   }
  //   Let us assume aliases of %i6 are %struct.base* and %struct.derived* and
  //   argument type of bar is %struct.base**.
  //
  // To:
  //   %struct.derived = type { %struct.base }
  //   @foo(ptr %arg) {
  //   bb1:
  //     %i = getelementptr inbounds %Parser, ptr %arg, i64 0, i32 1
  //     %i2 = load ptr, ptr %i, align 8
  //     br i1 false, label %bb3, label %bb5
  //
  //   bb3:
  //     %i4 = tail call ptr @Alloc(i64 40, ptr null)
  //     store ptr %i4, ptr %i, align 8
  //     br label %bb5
  //
  //   bb5:
  //     %i6 = phi ptr [ %i4, %bb3 ], [ %i2, %bb1 ]
  //     %dtnorm = getelementptr %struct.derived, ptr %i6, i64 0, i32 0
  //     bar(ptr %dtnorm)
  //     ...
  //   }
  void checkCallBase(CallBase *CB) {
    SetVector<std::pair<unsigned, DTransType *>> ValueToDomTyMap;
    unsigned NumArg = CB->arg_size();
    Function *F = CB->getFunction();
    for (unsigned AI = 0; AI < NumArg; ++AI) {
      Value *ArgVal = CB->getArgOperand(AI);
      if (!ArgVal->getType()->isPointerTy())
        continue;
      DTransType *ArgUsedTy = getCalleeArgumentType(CB, AI);
      DTransType *ArgUsedStTy = getPointeeDTransStructTy(ArgUsedTy);
      if (!ArgUsedStTy)
        continue;
      DTransType *DomStTy;
      if (auto *Arg = dyn_cast<Argument>(ArgVal)) {
        unsigned ArgNo = Arg->getArgNo();
        DTransType *ArgDeclTy =
            getArgumentTypeFromDType(MDReader.getDTransTypeFromMD(F), ArgNo);
        DTransType *ArgDeclStTy = getPointeeDTransStructTy(ArgDeclTy);
        if (!ArgDeclStTy)
          continue;
        if (!isSubObject(ArgDeclStTy, ArgUsedStTy))
          continue;

        ValueToDomTyMap.insert({AI, ArgDeclStTy});
      } else if (isNormalizedGEPNeeded(ArgVal, ArgUsedStTy->getLLVMType(),
                                       &DomStTy)) {
        ValueToDomTyMap.insert({AI, DomStTy});
      } else if (auto *PHI = dyn_cast<PHINode>(ArgVal)) {
        DTransType *ExpectedTy =
            getPHIDominantType(PHI, ArgUsedStTy->getLLVMType());
        if (!ExpectedTy)
          continue;
        ValueToDomTyMap.insert({AI, ExpectedTy});
      }
    }
    if (ValueToDomTyMap.size() == 0)
      return;
    auto &ValueToDomSet = CallBaseArgsGepify[CB];
    for (const auto &Entry : ValueToDomTyMap)
      ValueToDomSet.insert(Entry);
  }

  // Check (GEP, 0, 0) is needed if stored value is memory allocation
  // pointer.
  // Example 1:
  // From:
  //   %struct.derived = type { %struct.base }
  //   %struct.test = { i32, %struct.base* }
  //   @foo(ptr %a) {
  //      %i = tail call ptr @Alloc(i64 16, ptr null)
  //      call void @Ctor1(ptr %i)
  //      ...
  //      %p = getelementptr inbounds %struct.test, ptr %a, i64 0, i32 1
  //      store ptr %i, ptr %p
  //   }
  //   Let us assume aliases of %i are %struct.base* and %struct.derived* and
  //   %p is alias to %struct.base* only.
  //
  // To:
  //   %struct.derived = type { %struct.base }
  //   %struct.test = { i32, %struct.base* }
  //   @foo(ptr %a) {
  //      %i = tail call ptr @Alloc(i64 16, ptr null)
  //      call void @Ctor1(ptr %i)
  //      ...
  //      %p = getelementptr inbounds %struct.test, ptr %a, i64 0, i32 1
  //      %dtnorm = getelementptr %%struct.derived, ptr %i, i64 0, i32 0
  //      store ptr %dtnorm, ptr %p
  //   }
  //
  // Example 2:
  // From:
  //   %struct.derived = type { %struct.base }
  //   @foo(ptr %arg) {
  //   bb1:
  //     %i = getelementptr inbounds %Parser, ptr %arg, i64 0, i32 1
  //     %i2 = load ptr, ptr %i, align 8
  //     br i1 false, label %bb3, label %bb5
  //
  //   bb3:
  //     %i4 = tail call ptr @Alloc(i64 40, ptr null)
  //     store ptr %i4, ptr %i, align 8
  //     br label %bb5
  //
  //   bb5:
  //     %i6 = phi ptr [ %i4, %bb3 ], [ %i2, %bb1 ]
  //     store ptr %i6, %p
  //       ...
  //   }
  //   Let us assume aliases of %i6 are %struct.base* and %struct.derived* and
  //   alias of %p is %struct.base**.
  //
  // To:
  //   %struct.derived = type { %struct.base }
  //   @foo(ptr %arg) {
  //   bb1:
  //     %i = getelementptr inbounds %Parser, ptr %arg, i64 0, i32 1
  //     %i2 = load ptr, ptr %i, align 8
  //     br i1 false, label %bb3, label %bb5
  //
  //   bb3:
  //     %i4 = tail call ptr @Alloc(i64 40, ptr null)
  //     store ptr %i4, ptr %i, align 8
  //     br label %bb5
  //
  //   bb5:
  //     %i6 = phi ptr [ %i4, %bb3 ], [ %i2, %bb1 ]
  //     %dtnorm = getelementptr %struct.derived, ptr %i6, i64 0, i32 0
  //     store ptr %dtnorm, %p
  //     ...
  //   }
  void checkStoredValue(Instruction *I) {
    auto *SI = dyn_cast<StoreInst>(I);
    if (!SI)
      return;
    Value *Ptr = SI->getPointerOperand();
    Value *Val = SI->getValueOperand();
    if (isCompilerConstantData(Ptr) || isCompilerConstantData(Val))
      return;
    // Check if pointer operand has single alias type that points to a struct.
    ValueTypeInfo *Info = PtrAnalyzer.getValueTypeInfo(Ptr);
    if (!Info || Info->getUnhandled() || Info->getDependsOnUnhandled())
      return;
    auto &UseAliases = Info->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use);
    if (UseAliases.size() != 1)
      return;
    DTransType *AliasTy = *UseAliases.begin();
    if (!AliasTy->isPointerTy())
      return;
    DTransType *PtrStTy = AliasTy->getPointerElementType();
    if (!PtrStTy->isPointerTy())
      return;
    DTransType *StTy = PtrStTy->getPointerElementType();
    if (!isa<DTransStructType>(StTy))
      return;

    // Check (GEP, 0, 0) is needed for value operand.
    DTransType *DomStTy;
    if (auto *PHI = dyn_cast<PHINode>(Val)) {
      DomStTy = getPHIDominantType(PHI, StTy->getLLVMType());
      if (!DomStTy)
        return;
    } else {
      if (!isNormalizedGEPNeeded(Val, StTy->getLLVMType(), &DomStTy,
                                 /* AllowInvoke */ true))
        return;
    }

    StoreValueGepify[SI] = DomStTy;
  }

  void gepifyPHI(PHINode *PHIN, unsigned Num) {
    auto GEPI = dyn_cast<GetElementPtrInst>(PHIN->getIncomingValue(Num));
    if (!GEPI)
      return;
    Type *Ty = PHIType.lookup(PHIN);
    if (!Ty)
      return;

    SmallVector<Value *, 2> IdxList;
    IdxList.push_back(ZeroPtrSizedInt);
    IdxList.push_back(Zero32);
    auto *GEP = GetElementPtrInst::Create(Ty, GEPI, IdxList, "dtnorm",
                                          GEPI->getNextNonDebugInstruction());
    PHIN->replaceUsesOfWith(PHIN->getIncomingValue(Num), GEP);
  }

  // Pointer type analyzer and safety analyzer don't handle ADD
  // but GEP instruction for pointer, this function tries to convert ADD
  // instruction to GEP instruction when there are 2 same type' pointers,
  // and one unknown type's pointer:
  // Check if the unknown type's pointer is in a positive position first.
  // If yes, then the other 2 same type's pointers make up subtraction, and
  // can be evenly divided by their type. So we finally can convert
  // "inttoptr(undef-b+c)" to "GEP type, ptr undef, (c-b)/type_size".
  //
  // Example:
  // From:
  //   %alloc_ptr = tail call ptr @realloc(ptr noundef %net_arcs, i64 1024)
  //   %alloc = ptrtoint ptr %alloc_ptr to i64
  //   %a = ptrtoint ptr %a_ptr to i64
  //   %b = ptrtoint ptr %b_ptr to i64
  //   %off = sub i64 %alloc, %a
  //   %c = add i64 %off, %b
  //   %c_ptr = inttoptr i64 %c to ptr
  // To:
  //   %alloc_ptr = tail call ptr @realloc(ptr noundef %net_arcs, i64 1024)
  //   %alloc = ptrtoint ptr %alloc_ptr to i64
  //   %a = ptrtoint ptr %a_ptr to i64
  //   %b = ptrtoint ptr %b_ptr to i64
  //   %off = sub i64 %b, %a
  //   %idx = sdiv i64 %off, 64
  //   %ptr = getelementptr %arc, ptr %alloc_ptr, i64 %idx
  void checkInt2Ptr(IntToPtrInst *Int2Ptr) {
    auto IsLegalPtr = [&](PtrToIntInst *PtrToInt, DTransType *&Ty) {
      Value *Ptr = PtrToInt->getOperand(0);
      ValueTypeInfo *PtrInfo = PtrAnalyzer.getValueTypeInfo(Ptr);
      if (!PtrInfo)
        return false;
      if (PtrInfo->getUnhandled() || PtrInfo->getDependsOnUnhandled())
        return false;

      auto &UseAliases =
          PtrInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use);
      if (UseAliases.size() != 1)
        return false;

      Ty = *UseAliases.begin();
      if (!Ty->isPointerTy())
        return false;

      return Ty->getPointerElementType()->isStructTy();
    };

    auto *Add = dyn_cast<BinaryOperator>(Int2Ptr->getOperand(0));
    if (!Add || Add->getOpcode() != Instruction::Add || !Add->hasOneUse())
      return;

    int SubIdx = 0;
    // Find the first available sub instruction.
    auto *Sub = dyn_cast<BinaryOperator>(Add->getOperand(SubIdx));
    if (!Sub || Sub->getOpcode() != Instruction::Sub || !Sub->hasOneUse()) {
      SubIdx = 1;
      Sub = dyn_cast<BinaryOperator>(Add->getOperand(SubIdx));
      if (!Sub || Sub->getOpcode() != Instruction::Sub || !Sub->hasOneUse())
        return;
    }

    // Get the two positive pointers, one comes from 'add' instruction,
    // another comes from 'sub's first operand.
    PtrToIntInst *PosPtr[2] = {nullptr};
    PosPtr[0] = dyn_cast<PtrToIntInst>(Add->getOperand(1 - SubIdx));
    if (!PosPtr[0] || !PosPtr[0]->hasOneUse())
      return;

    PosPtr[1] = dyn_cast<PtrToIntInst>(Sub->getOperand(0));
    if (!PosPtr[1] || !PosPtr[1]->hasOneUse())
      return;

    // Get the negative pointer.
    auto *NegPtr = dyn_cast<PtrToIntInst>(Sub->getOperand(1));
    if (!NegPtr || !NegPtr->hasOneUse())
      return;

    DTransType *NegPtrTy = nullptr;
    if (!IsLegalPtr(NegPtr, NegPtrTy))
      return;

    int PosPtrFailIdx = -1;
    int PosPtrPassIdx = -1;
    DTransType *PosPtrTy[2] = {nullptr};

    // For the two positive pointers, we should make sure at least one of them's
    // type is equal to the negative pointer.
    for (int I = 0; I < 2; ++I) {
      if (!IsLegalPtr(PosPtr[I], PosPtrTy[I])) {
        PosPtrFailIdx = I;
        continue;
      }

      if (PosPtrTy[I] != NegPtrTy) {
        PosPtrFailIdx = I;
        continue;
      }

      PosPtrPassIdx = I;
    }

    // If one positive pointer's type is equal to the negative pointer,
    // make sure the other one is an alloc instruction.
    if (PosPtrFailIdx != -1 && PosPtrPassIdx != -1) {
      auto Call = dyn_cast<CallInst>(PosPtr[PosPtrFailIdx]->getOperand(0));
      if (!Call)
        return;

      const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
      dtrans::AllocKind AKind = DTAC.getAllocFnKind(Call, TLI);
      if (AKind == dtrans::AK_NotAlloc)
        return;
    } else {
      return;
    }

    const DataLayout &DL = M.getDataLayout();
    auto PosPtrLLVMTy =
        PosPtrTy[PosPtrPassIdx]->getPointerElementType()->getLLVMType();
    TypeSize PotentialScale = DL.getTypeAllocSize(PosPtrLLVMTy);
    if (PotentialScale.isScalable())
      return;

    AddGepify.push_back({Int2Ptr, Add, NegPtr, PosPtrLLVMTy,
                         PosPtr[PosPtrPassIdx], PosPtr[1 - PosPtrPassIdx]});
  }

  void gepifyAdd(Instruction *Int2Ptr, Instruction *Add, PtrToIntInst *NegPtr,
                 Type *PosPtrTy, PtrToIntInst *PosPtr0, PtrToIntInst *PosPtr1) {
    const DataLayout &DL = M.getDataLayout();
    TypeSize PotentialScale = DL.getTypeAllocSize(PosPtrTy);

    auto *NewSub = BinaryOperator::CreateSub(PosPtr0, NegPtr, "off", Int2Ptr);
    auto *TySize =
        ConstantInt::get(NewSub->getType(), PotentialScale.getFixedValue());
    auto *NewDiv = BinaryOperator::CreateSDiv(NewSub, TySize, "idx", Int2Ptr);

    SmallVector<Value *, 2> IdxList;
    IdxList.push_back(NewDiv);
    auto *NewGEP = GetElementPtrInst::Create(PosPtrTy, PosPtr1->getOperand(0),
                                             IdxList, "ptr", Int2Ptr);
    Int2Ptr->replaceAllUsesWith(NewGEP);

    Int2Ptr->eraseFromParent();
    Add->eraseFromParent();
  }

  ConstantInt *Zero32 = nullptr;
  ConstantInt *ZeroPtrSizedInt = nullptr;

  SetVector<std::tuple<Instruction *, DTransType *, unsigned int>>
      InstructionsToGepify;
  SmallMapVector<PHINode *, Type *, 8> PHIType;
  SetVector<std::pair<PHINode *, unsigned>> PHIsToGepify;
  SetVector<std::pair<ReturnInst *, DTransType *>> ReturnsToGepify;
  SmallMapVector<PHINode *, SetVector<std::pair<unsigned, DTransType *>>, 8>
      PHIReturnsGepify;
  SetVector<std::pair<GetElementPtrInst *, DTransType *>> GEPsToGepify;
  SmallMapVector<CallBase *, SetVector<std::pair<unsigned, DTransType *>>, 8>
      CallBaseArgsGepify;
  SmallMapVector<StoreInst *, DTransType *, 8> StoreValueGepify;
  SmallVector<std::tuple<Instruction *, Instruction *, PtrToIntInst *, Type *,
                         PtrToIntInst *, PtrToIntInst *>>
      AddGepify;

  // This is used to reuse GEP instruction if it is already one.
  SmallMapVector<Value *, GetElementPtrInst *, 8> NewGEPInsts;
};

PreservedAnalyses
dtransOP::DTransNormalizeOPPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  bool Changed = runImpl(M, WPInfo, GetTLI);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<TargetLibraryAnalysis>();
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

bool dtransOP::DTransNormalizeOPPass::runImpl(Module &M,
                                              WholeProgramInfo &WPInfo,
                                              GetTLIFn GetTLI) {
  if (!WPInfo.isWholeProgramSafe())
    return false;

  LLVMContext &Ctx = M.getContext();
  const DataLayout &DL = M.getDataLayout();
  DTransTypeManager TM(M.getContext());
  TypeMetadataReader MDReader(TM);
  if (!MDReader.initialize(M)) {
    LLVM_DEBUG(dbgs() << "DTransSafetyInfo: Type metadata reader did not find "
                         "structure type metadata\n");
    return false;
  }

  DTransAllocCollector DTAC(MDReader, GetTLI);
  DTAC.populateAllocDeallocTable(M);
  PtrTypeAnalyzer PtrAnalyzer(Ctx, TM, MDReader, DL, GetTLI);
  PtrAnalyzer.run(M);

  DTransNormalizeImpl Impl(M, PtrAnalyzer, MDReader, DTAC, GetTLI);
  return Impl.run();
}
