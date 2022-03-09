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

#define DEBUG_TYPE "vpo-paropt-utils"

using namespace llvm;
using namespace llvm::vpo;

CallInst *VPOParoptUtils::genF90DVSizeCall(Value *DV,
                                           Instruction *InsertBefore) {
  IRBuilder<> Builder(InsertBefore);

  auto *DVCast = Builder.CreateBitCast(DV, Builder.getInt8PtrTy());
  CallInst *DataSize =
      genCall(InsertBefore->getModule(), "_f90_dope_vector_size",
              Builder.getInt64Ty(), {DVCast});
  DataSize->insertBefore(InsertBefore);
  return DataSize;
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
      genCall(InsertBefore->getModule(), "_f90_dope_vector_init",
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
  assert(isa<PointerType>(SrcV->getType()) && "Orig value is not a pointer");
  assert(
      isa<StructType>(SrcV->getType()->getPointerElementType()) &&
      "Clause item is expected to be a struct for F90 DVs.");

  if (AllowOverrideInsertPt && !GeneralUtils::isOMPItemGlobalVAR(DstV))
    InsertPt = (cast<Instruction>(DstV))->getParent()->getTerminator();

  IRBuilder<> Builder(InsertPt);

  auto &DL = InsertPt->getModule()->getDataLayout();
  MaybeAlign OrigAlignment = SrcV->getPointerAlignment(DL);
  CallInst *DataSize = genF90DVInitCall(SrcV, DstV, InsertPt, IsTargetSPIRV);
  setFuncCallingConv(DataSize, DataSize->getModule());

  Instruction *AllocBuilderInsertPt = &*Builder.GetInsertPoint();

  // TODO: OPAQUEPOINTER: We need another way to get the DV struct/element type.
  auto *DVType = cast<StructType>(SrcV->getType()->getPointerElementType());
  Type *ElementTy = DVType->getElementType(0)->getPointerElementType();

  // We need to compute the number of elements in the dope vector.  DataSize is
  // the size in bytes of the data "array" for the dope vector. To get number of
  // elements, divide by the size of each element in bytes.
  auto *NumElements = Builder.CreateUDiv(
      DataSize,
      Builder.getIntN(DataSize->getType()->getPrimitiveSizeInBits(),
                      DL.getTypeSizeInBits(ElementTy) / 8),
      NamePrefix + ".num_elements");
  I->setF90DVNumElements(NumElements);

  // Create a branch to guard memory allocation for local copy of DV's data.
  // if (dv_size != 0) {
  //  ... then allocate space for local copy
  // }
  if (CheckOrigAllocationBeforeAllocatingNew) {
    Value *ZeroSize =
        Builder.getIntN(DataSize->getType()->getIntegerBitWidth(), 0);
    Value *IsOrigAllocated =
        Builder.CreateICmpNE(DataSize, ZeroSize, "is.allocated");

    Instruction *BranchPt = &*Builder.GetInsertPoint();
    I->setF90DVDataAllocationPoint(BranchPt);

    Instruction *ThenTerm = SplitBlockAndInsertIfThen(
        IsOrigAllocated, BranchPt, false,
        MDBuilder(Builder.getContext()).createBranchWeights(4, 1), DT, LI);
    BasicBlock *ThenBB = ThenTerm->getParent();
    ThenBB->setName("allocated.then");
    AllocBuilderInsertPt = ThenTerm;
  }

  IRBuilder<> AllocBuilder(AllocBuilderInsertPt);
  // Get base address from the dope vector.
  auto *Zero = AllocBuilder.getInt32(0);
  auto *Addr0GEP = AllocBuilder.CreateInBoundsGEP(DVType, DstV, {Zero, Zero},
                                                  NamePrefix + ".addr0");
  Value *PointeeData = genPrivatizationAlloca(
      ElementTy, NumElements, OrigAlignment, &*AllocBuilder.GetInsertPoint(),
      IsTargetSPIRV, NamePrefix + ".data");
  auto *StoreVal = AllocBuilder.CreatePointerBitCastOrAddrSpaceCast(
      PointeeData, cast<GEPOperator>(Addr0GEP)->getResultElementType());
  AllocBuilder.CreateStore(StoreVal, Addr0GEP);

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

  // Get base address from the dope vector.
  auto *Zero = Builder.getInt32(0);
  auto *DVType = cast<StructType>(NewV->getType()->getPointerElementType());
  auto *Addr0GEP = Builder.CreateInBoundsGEP(DVType, NewV, {Zero, Zero},
                                             NamePrefix + ".addr0");
  DestArrayBeginOut = Builder.CreateLoad(
      Addr0GEP->getType()->getPointerElementType(), Addr0GEP,
      NamePrefix + ".data");
  DestElementTyOut = DestArrayBeginOut->getType()->getPointerElementType();

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

  auto *DVType = cast<StructType>(DestVal->getType()->getPointerElementType());
  auto *Zero = Builder.getInt32(0);
  auto *Addr0GEP = Builder.CreateInBoundsGEP(DVType, DestVal, {Zero, Zero},
                                             NamePrefix + ".addr0");
  DestArrayBeginOut = Builder.CreateLoad(
      Addr0GEP->getType()->getPointerElementType(), Addr0GEP,
      NamePrefix + ".data");
}
#endif // INTEL_CUSTOMIZATION
