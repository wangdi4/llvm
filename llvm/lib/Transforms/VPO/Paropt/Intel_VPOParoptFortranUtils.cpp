#if INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Copyright (C) 2021 Intel Corporation
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
//==-- Intel_VPOParoptFortranUtils.cpp - VPO utils for Fortran -------------==//
//
// Authors:
// --------
// Abhinav Gaba (abhinav.gaba@intel.com)
//
//==------------------------------------------------------------------------==//
///
/// \file
/// This file provides a set of Fortran specific utilities for VPO Paropt
/// transformations to generate calls to various runtime APIs.
///
//==------------------------------------------------------------------------==//

#include "llvm/IR/MDBuilder.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Analysis/DomTreeUpdater.h"

#define DEBUG_TYPE "vpo-paropt-utils"

using namespace llvm;
using namespace llvm::vpo;

std::tuple<Type *, Type *> VPOParoptUtils::getF90DVItemInfo(const Item *I) {
  assert(I && "Null Clause Item.");
  assert(I->getIsF90DopeVector() && "Item is not an F90 DV.");

  Type *DVType = nullptr;
  std::tie(DVType, std::ignore, std::ignore) = VPOParoptUtils::getItemInfo(I);
  assert(isa<StructType>(DVType) && "DV Type is not a struct type.");

  assert(I->getIsTyped() && "Only typed F90_DVs are supported.");
  return {DVType, I->getPointeeElementTypeFromIR()};
}

Value *VPOParoptUtils::genF90DVSizeCall(Value *DV, Instruction *InsertBefore) {
  IRBuilder<> Builder(InsertBefore);

  auto *DVCast = Builder.CreateBitCast(DV, Builder.getInt8PtrTy());
  CallInst *DataSize =
      genCall(InsertBefore->getModule(), "_f90_dope_vector_size",
              Builder.getInt64Ty(), {DVCast});
  DataSize->insertBefore(InsertBefore);
  Value *ConstZeroValue = ConstantInt::get(DataSize->getType(), 0);
  Value *Compare = Builder.CreateICmpSLT(DataSize, ConstZeroValue);
  Value *AdjustedDataSize =
      Builder.CreateSelect(Compare, ConstZeroValue, DataSize);
  return AdjustedDataSize;
}

CallInst *VPOParoptUtils::genF90DVInitCall(Value *OrigDV, Value *NewDV,
                                           Instruction *InsertBefore,
                                           bool IsTargetSPIRV) {
  IRBuilder<> Builder(InsertBefore);

  Type *Int8PtrTy =
      Builder.getInt8PtrTy(IsTargetSPIRV ? vpo::ADDRESS_SPACE_GENERIC : 0);
  auto *NewDVCast =
      Builder.CreatePointerBitCastOrAddrSpaceCast(NewDV, Int8PtrTy);
  auto *OrigDVCast =
      Builder.CreatePointerBitCastOrAddrSpaceCast(OrigDV, Int8PtrTy);
  CallInst *DataSize =
      genCall(InsertBefore->getModule(), "_f90_dope_vector_init2",
              Builder.getInt64Ty(), {NewDVCast, OrigDVCast});
  DataSize->insertBefore(InsertBefore);
  DataSize->setName(".dv.init");
  return DataSize;
}

void VPOParoptUtils::genF90DVInitCode(
    Item *I, Instruction *InsertPt, DominatorTree *DT, LoopInfo *LI,
    bool IsTargetSPIRV, bool AllowOverrideInsertPt,
    bool CheckOrigAllocationBeforeAllocatingNew,
    bool StoreNumElementsToGlobal) {
  VPOParoptUtils::genF90DVInitCode(I, I->getOrig(), I->getNew(), InsertPt, DT,
                                   LI, IsTargetSPIRV, AllowOverrideInsertPt,
                                   CheckOrigAllocationBeforeAllocatingNew,
                                   StoreNumElementsToGlobal);
}

void VPOParoptUtils::genF90DVInitCode(
    Item *I, Value *SrcV, Value *DstV, Instruction *InsertPt, DominatorTree *DT,
    LoopInfo *LI, bool IsTargetSPIRV, bool AllowOverrideInsertPt,
    bool CheckOrigAllocationBeforeAllocatingNew,
    bool StoreNumElementsToGlobal) {
  assert(I->getIsF90DopeVector() && "Item is not an F90 dope vector.");

  StringRef NamePrefix = DstV->getName();
  assert(isa<PointerType>(SrcV->getType()) && "Src value is not a pointer");

  if (AllowOverrideInsertPt && !GeneralUtils::isOMPItemGlobalVAR(DstV))
    InsertPt = (cast<Instruction>(DstV))->getParent()->getTerminator();

  IRBuilder<> Builder(InsertPt);

  auto &DL = InsertPt->getModule()->getDataLayout();
  Align MinAlign = SrcV->getPointerAlignment(DL);
  MaybeAlign OrigAlignment = MinAlign > 1 ? MinAlign : (MaybeAlign)std::nullopt;
  CallInst *DataSize = genF90DVInitCall(SrcV, DstV, InsertPt, IsTargetSPIRV);
  setFuncCallingConv(DataSize, DataSize->getModule());

  Instruction *AllocBuilderInsertPt = &*Builder.GetInsertPoint();

  Type *DVType = nullptr;
  Type *DataElementTy = nullptr;
  std::tie(DVType, DataElementTy) = VPOParoptUtils::getF90DVItemInfo(I);

  // Create a branch to guard memory allocation for local copy of DV's data.
  // if (dv_size != 0) {
  //  ... then allocate space for local copy
  // }
  Value *ZeroSize =
      Builder.getIntN(DataSize->getType()->getIntegerBitWidth(), 0);
  BasicBlock *DVInitBB = InsertPt->getParent();
  Instruction *BranchPt = nullptr;
  if (CheckOrigAllocationBeforeAllocatingNew) {
    // Fortran allows allocated, zero length arrays, so DataSize == 0 indicates
    // that the array is allocated and has length of zero, but ParOpt task
    // code does not handle zero-length arrays, so pretend that DataSize == 0
    // means unallocated.
    Value *IsOrigAllocated =
        Builder.CreateICmpSGT(DataSize, ZeroSize, "is.allocated");

    BranchPt = &*Builder.GetInsertPoint();
    I->setF90DVDataAllocationPoint(BranchPt);

    DomTreeUpdater DTU(DT, DomTreeUpdater::UpdateStrategy::Lazy);
    Instruction *ThenTerm = SplitBlockAndInsertIfThen(
        IsOrigAllocated, BranchPt, false,
        MDBuilder(Builder.getContext()).createBranchWeights(4, 1), &DTU, LI);
    BasicBlock *ThenBB = ThenTerm->getParent();
    ThenBB->setName("allocated.then");
    AllocBuilderInsertPt = ThenTerm;
  }

  IRBuilder<> AllocBuilder(AllocBuilderInsertPt);
  // We need to compute the number of elements in the dope vector.  DataSize is
  // the size in bytes of the data "array" for the dope vector. To get number of
  // elements, divide by the size of each element in bytes.
  //
  // When the array is unallocated _f90_dope_vector_init2 returns -1.
  auto *NumElementsIfAllocated = AllocBuilder.CreateUDiv(
      DataSize,
      AllocBuilder.getIntN(DataSize->getType()->getPrimitiveSizeInBits(),
                           DL.getTypeSizeInBits(DataElementTy) / 8),
      NamePrefix + ".alloc.num_elements");

  // Get base address from the dope vector.
  auto *Zero = AllocBuilder.getInt32(0);
  auto *Addr0GEP = AllocBuilder.CreateInBoundsGEP(DVType, DstV, {Zero, Zero},
                                                  NamePrefix + ".addr0");
  Value *PointeeData = genPrivatizationAlloca(
      DataElementTy, NumElementsIfAllocated, OrigAlignment,
      &*AllocBuilder.GetInsertPoint(), IsTargetSPIRV, NamePrefix + ".data");

  auto *Addr0Ty = cast<StructType>(DVType)->getElementType(0);
  auto *StoreVal =
      AllocBuilder.CreatePointerBitCastOrAddrSpaceCast(PointeeData, Addr0Ty);
  AllocBuilder.CreateStore(StoreVal, Addr0GEP);

  // When the caller asked not to check if the array was already allocated,
  // pass back the NumElements without checking it
  llvm::Value *NumElements = NumElementsIfAllocated;
  if (CheckOrigAllocationBeforeAllocatingNew) {
    // When the caller does ask to check allocation, select between the correct
    // number of elements when the array is allocated or zero when not
    // allocated.
    //
    // num_elements = dv_size >= 0 ? num_elements_if_allocated : 0;
    //
    // The IR should look similar to:
    //
    //  do.dv.init:
    //    %.dv.init = call i64 @_f90_dope_vector_init2(i8* %0, i8* %1)
    //    %is.allocated = icmp sge i64 %.dv.init, 0
    //    br i1 %is.allocated, label %allocated.then, label %not.allocated
    //
    //  allocated.then:             ; preds = %do.dv.init
    //    %AR.alloc.num_elements = udiv i64 %.dv.init, 4
    //    %AR.addr0 =  ...
    //    %AR.data = alloca i32, i64 %AR.alloc.num_elements, align 4
    //    store i32* %AR.data, i32** %AR.addr0, align 8
    //    br label %not.allocated
    //
    //  not.allocated:              ; preds = %do.dv.init, %allocated.then
    //    %AR.num_elements = phi i64
    //          [ %AR.alloc.num_elements, %allocated.then ], [ 0, %do.dv.init ]
    //
    // The rest of ParOpt assumes that a length of 0 means the array is
    // unallocated, though Fortran does allow zero length arrays.  After the
    // dope vector is initialized here, treating the array as unallocated seems
    // not to cause problems later.
    //
    // If there are issues with privatization of zero length arrays, this is a
    // place to start debugging.

    IRBuilder<> TailBuilder(BranchPt);
    llvm::PHINode *NumElementsPHI = TailBuilder.CreatePHI(
        DataSize->getType(), 2, NamePrefix + ".num_elements");
    NumElementsPHI->addIncoming(NumElementsIfAllocated,
                                AllocBuilderInsertPt->getParent());
    NumElementsPHI->addIncoming(ZeroSize, DVInitBB);
    NumElements = NumElementsPHI;
  }
  I->setF90DVNumElements(NumElements);

  if (!StoreNumElementsToGlobal)
    return;

  GlobalVariable *GV = VPOParoptUtils::storeIntToThreadLocalGlobal(
      NumElements, &*Builder.GetInsertPoint(), "dv.num.elements");
  I->setF90DVNumElementsGV(GV);
}

void VPOParoptUtils::genF90DVInitForItemsInTaskPrivatesThunk(
    WRegionNode *W, Value *KmpPrivatesGEP, StructType *KmpPrivatesTy,
    Instruction *InsertBefore) {

  assert(W->getIsTask() && "WRegion is not a task.");

  IRBuilder<> Builder(InsertBefore);

  auto genF90DVInitCallForItem = [&](Item *I) {
    if (!I->getIsF90DopeVector())
      return;

    Value *OrigV = I->getOrig();
    StringRef NamePrefix = OrigV->getName();
    Value *NewVGep = Builder.CreateInBoundsGEP(
        KmpPrivatesTy, KmpPrivatesGEP,
        {Builder.getInt32(0), Builder.getInt32(I->getPrivateThunkIdx())},
        NamePrefix + ".priv.gep");
    genF90DVInitCall(OrigV, NewVGep, InsertBefore);
  };

  for (PrivateItem *PrivI : W->getPriv().items())
    genF90DVInitCallForItem(PrivI);

  for (FirstprivateItem *FprivI : W->getFpriv().items())
    genF90DVInitCallForItem(FprivI);

  if (W->canHaveLastprivate())
    for (LastprivateItem *LprivI : W->getLpriv().items()) {
      if (LprivI->getInFirstprivate())
        continue;
      genF90DVInitCallForItem(LprivI);
    }
}

void VPOParoptUtils::genF90DVFirstOrLastprivateCopyCallImpl(
    StringRef FnName, Value *NewV, Value *OrigV, Instruction *InsertBefore,
    bool IsTargetSPIRV) {

  IRBuilder<> Builder(InsertBefore);

  Type *Int8PtrTy =
      Builder.getInt8PtrTy(IsTargetSPIRV ? vpo::ADDRESS_SPACE_GENERIC : 0);
  auto *NewVCast = Builder.CreatePointerBitCastOrAddrSpaceCast(NewV, Int8PtrTy);
  auto *OrigVCast =
      Builder.CreatePointerBitCastOrAddrSpaceCast(OrigV, Int8PtrTy);
  CallInst *F90DVCopy = genCall(InsertBefore->getModule(), FnName,
                                Builder.getVoidTy(), {NewVCast, OrigVCast});
  F90DVCopy->insertBefore(InsertBefore);
  setFuncCallingConv(F90DVCopy, F90DVCopy->getModule());
}

void VPOParoptUtils::genF90DVFirstprivateCopyCall(Value *NewV, Value *OrigV,
                                                  Instruction *InsertBefore,
                                                  bool IsTargetSPIRV) {
  genF90DVFirstOrLastprivateCopyCallImpl("_f90_firstprivate_copy", NewV, OrigV,
                                         InsertBefore, IsTargetSPIRV);
}

void VPOParoptUtils::genF90DVLastprivateCopyCall(Value *NewV, Value *OrigV,
                                                 Instruction *InsertBefore,
                                                 bool IsTargetSPIRV) {
  genF90DVFirstOrLastprivateCopyCallImpl("_f90_lastprivate_copy", NewV, OrigV,
                                         InsertBefore, IsTargetSPIRV);
}

void VPOParoptUtils::genF90DVReductionInitDstInfo(const Item *I, Value *&NewV,
                                                  Value *&DestArrayBeginOut,
                                                  Type *&DestElementTyOut,
                                                  Value *&NumElementsOut,
                                                  Instruction *InsertBefore) {
  assert(I->getIsF90DopeVector() && "Item is not an F90 dope vector.");

  IRBuilder<> Builder(InsertBefore);
  StringRef NamePrefix = NewV->getName();

  Type *DVType = nullptr;
  Type *DataElementTy = nullptr;
  std::tie(DVType, DataElementTy) = VPOParoptUtils::getF90DVItemInfo(I);

  // Get base address from the dope vector.
  auto *Zero = Builder.getInt32(0);
  auto *Addr0GEP = Builder.CreateInBoundsGEP(DVType, NewV, {Zero, Zero},
                                             NamePrefix + ".addr0");
  auto *Addr0Ty = cast<StructType>(DVType)->getElementType(0);
  DestArrayBeginOut =
      Builder.CreateLoad(Addr0Ty, Addr0GEP, NamePrefix + ".data");
  DestElementTyOut = DataElementTy;

  Value *NumElementsFromI = I->getF90DVNumElements();
  GlobalVariable *NumElementsGV = I->getF90DVNumElementsGV();
  bool NumElementsFromIIsInSameFunctionAsInsertBefore =
      (cast<Instruction>(NumElementsFromI)->getFunction() ==
       InsertBefore->getFunction());

  if (!NumElementsGV || NumElementsFromIIsInSameFunctionAsInsertBefore) {
    NumElementsOut = NumElementsFromI;
    return;
  }

  Value *NumElementsLoadedFromGV =
      Builder.CreateLoad(NumElementsGV->getValueType(),
                         NumElementsGV, NumElementsGV->getName() + ".load");
  NumElementsOut = NumElementsLoadedFromGV;
}

void VPOParoptUtils::genF90DVReductionSrcDstInfo(
    const Item *I, Value *&SrcVal, Value *&DestVal, Value *&SrcArrayBeginOut,
    Value *&DestArrayBeginOut, Type *&DestElementTyOut, Value *&NumElementsOut,
    Instruction *InsertBefore) {
  assert(I->getIsF90DopeVector() && "Item is not an F90 dope vector.");

  // Destination on reduction init code (local array) is the source for the
  // finish code.
  VPOParoptUtils::genF90DVReductionInitDstInfo(I, SrcVal, SrcArrayBeginOut,
                                               DestElementTyOut, NumElementsOut,
                                               InsertBefore);

  IRBuilder<> Builder(InsertBefore);
  StringRef NamePrefix = DestVal->getName();

  Type *DVType = nullptr;
  std::tie(DVType, std::ignore) = VPOParoptUtils::getF90DVItemInfo(I);

  auto *Zero = Builder.getInt32(0);
  auto *Addr0GEP = Builder.CreateInBoundsGEP(DVType, DestVal, {Zero, Zero},
                                             NamePrefix + ".addr0");
  auto *Addr0Ty = cast<StructType>(DVType)->getElementType(0);
  DestArrayBeginOut =
      Builder.CreateLoad(Addr0Ty, Addr0GEP, NamePrefix + ".data");
}
#endif // INTEL_CUSTOMIZATION
