#if INTEL_CUSTOMIZATION
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

void VPOParoptUtils::genF90DVInitCode(Item *I, Instruction *InsertPt,
                                      bool IsTargetSPIRV,
                                      bool AllowOverrideInsertPt) {
  assert(I->getIsF90DopeVector() && "Item is not an F90 dope vector.");

  Value *NewV = I->getNew();
  Value *OrigV = I->getOrig();
  StringRef NamePrefix = NewV->getName();
  assert(isa<PointerType>(OrigV->getType()) && "Orig value is not a pointer");
  assert(
      isa<StructType>(cast<PointerType>(OrigV->getType())->getElementType()) &&
      "Clause item is expected to be a struct for F90 DVs.");

  if (AllowOverrideInsertPt && !GeneralUtils::isOMPItemGlobalVAR(NewV))
    InsertPt = (cast<Instruction>(NewV))->getParent()->getTerminator();

  IRBuilder<> Builder(InsertPt);

  MaybeAlign OrigAlignment =
      OrigV->getPointerAlignment(InsertPt->getModule()->getDataLayout());
  CallInst *DataSize = genF90DVInitCall(OrigV, NewV, InsertPt, IsTargetSPIRV);
  setFuncCallingConv(DataSize, IsTargetSPIRV);

  // Get base address from the dope vector.
  auto *Zero = Builder.getInt32(0);
  auto *Addr0GEP =
      Builder.CreateInBoundsGEP(NewV, {Zero, Zero}, NamePrefix + ".addr0");
  Type *ElementTy =
      cast<PointerType>(cast<PointerType>(Addr0GEP->getType()->getScalarType())
                            ->getElementType())
          ->getElementType();
  Value *PointeeData = genPrivatizationAlloca(
      ElementTy, DataSize, OrigAlignment, InsertPt, IsTargetSPIRV,
      NamePrefix + ".data");
  auto *StoreVal =
      Builder.CreatePointerBitCastOrAddrSpaceCast(
          PointeeData,
          cast<GetElementPtrInst>(Addr0GEP)->getResultElementType());
  Builder.CreateStore(StoreVal, Addr0GEP);

  if (!isa<ReductionItem>(I))
    return;

  // For reduction, emit the init and fini loops, we need to compute the
  // number of elements in the dope vector.  DataSize is in size of the size in
  // bytes of the data "array" for the dope vector. To get number of elements,
  // divide by the size of each element in bytes.
  auto *NumElements = Builder.CreateUDiv(
      DataSize,
      Builder.getIntN(DataSize->getType()->getPrimitiveSizeInBits(),
                      ElementTy->getPrimitiveSizeInBits() / 8),
      NamePrefix + ".num_elements");
  I->setF90DVNumElements(NumElements);
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
  setFuncCallingConv(F90DVCopy, IsTargetSPIRV);
  F90DVCopy->insertBefore(InsertBefore);
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
  auto *Addr0GEP =
      Builder.CreateInBoundsGEP(NewV, {Zero, Zero}, NamePrefix + ".addr0");
  DestArrayBeginOut = Builder.CreateLoad(Addr0GEP, NamePrefix + ".data");
  DestElementTyOut =
      cast<PointerType>(DestArrayBeginOut->getType())->getElementType();

  NumElementsOut = I->getF90DVNumElements();
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

  auto *Zero = Builder.getInt32(0);
  auto *Addr0GEP =
      Builder.CreateInBoundsGEP(DestVal, {Zero, Zero}, NamePrefix + ".addr0");
  DestArrayBeginOut = Builder.CreateLoad(Addr0GEP, NamePrefix + ".data");
}
#endif // INTEL_CUSTOMIZATION
