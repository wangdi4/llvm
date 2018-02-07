//===- VPOParoptTarget.cpp - Transformation of W-Region for offloading --===//
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
/// VPOParoptTarget.cpp implements the omp target feature.
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

#define DEBUG_TYPE "vpo-paropt-target"

// Generate the code for the directive omp target
bool VPOParoptTransform::genTargetOffloadingCode(WRegionNode *W) {

  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTargetOffloadingCode\n");

  W->populateBBSet();

  codeExtractorPrepare(W);
  resetValueInIntelClauseGeneric(W, W->getIf());
  resetValueInIsDevicePtrClause(W);
  resetValueInPrivateClause(W);

  bool Changed = false;

  // extract a W-Region to generate a function
  CodeExtractor CE(makeArrayRef(W->bbset_begin(), W->bbset_end()), DT, false);

  assert(CE.isEligible());

  // Set up Fn Attr for the new function
  if (Function *NewF = CE.extractCodeRegion()) {

    // Set up the Calling Convention used by OpenMP Runtime Library
    CallingConv::ID CC = CallingConv::C;
    NewF->addFnAttr("target.declare", "true");

    DT->verifyDomTree();

    // Adjust the calling convention for both the function and the
    // call site.
    NewF->setCallingConv(CC);

    assert(NewF->hasOneUse() && "New function should have one use");
    User *U = NewF->user_back();

    CallInst *NewCall = cast<CallInst>(U);
    NewCall->setCallingConv(CC);

    IRBuilder<> Builder(F->getEntryBlock().getTerminator());
    AllocaInst *OffloadError = Builder.CreateAlloca(
        Type::getInt32Ty(F->getContext()), nullptr, ".run_host_version");

    Value *VIf = W->getIf();
    CallInst *Call;
    Instruction *InsertPt = NewCall;

    if (VIf) {
      Value *Cmp =
          Builder.CreateICmpNE(VIf, ConstantInt::get(VIf->getType(), 0));
      TerminatorInst *ThenTerm, *ElseTerm;
      buildCFGForIfClause(Cmp, ThenTerm, ElseTerm, InsertPt);
      InsertPt = ThenTerm;
      Call = genTargetInitCode(W, NewCall, InsertPt);
      Builder.SetInsertPoint(ElseTerm);
      Builder.CreateStore(
          ConstantInt::getSigned(Type::getInt32Ty(F->getContext()), -1),
          OffloadError);
    } else
      Call = genTargetInitCode(W, NewCall, InsertPt);

    if (isa<WRNTargetNode>(W)) {
      Builder.SetInsertPoint(InsertPt);
      Builder.CreateStore(Call, OffloadError);

      Builder.SetInsertPoint(NewCall);
      LoadInst *LastLoad = Builder.CreateLoad(OffloadError);
      ConstantInt *ValueZero =
          ConstantInt::getSigned(Type::getInt32Ty(F->getContext()), 0);
      Value *ErrorCompare = Builder.CreateICmpNE(LastLoad, ValueZero);
      TerminatorInst *Term = SplitBlockAndInsertIfThen(ErrorCompare, NewCall,
                                                       false, nullptr, DT, LI);
      Term->getParent()->setName("omp_offload.failed");
      LastLoad->getParent()->getTerminator()->getSuccessor(1)->setName(
          "omp_offload.cont");
      NewCall->removeFromParent();
      NewCall->insertBefore(Term->getParent()->getTerminator());

      genRegistrationFunction(W, NewF);
    } else if (isa<WRNTargetDataNode>(W)) {
      NewCall->removeFromParent();
      NewCall->insertAfter(Call);
    }

    W->resetBBSet(); // Invalidate BBSet after transformations

    Changed = true;
  }
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genTargetOffloadingCode\n");
  return Changed;
}

// Reset the expression value in IsDevicePtr clause to be empty.
void VPOParoptTransform::resetValueInIsDevicePtrClause(WRegionNode *W) {
  if (!W->canHaveIsDevicePtr())
    return;

  IsDevicePtrClause IDevicePtrClause = W->getIsDevicePtr();
  if (IDevicePtrClause.empty())
    return;

  for (auto *I : IDevicePtrClause.items()) {
    resetValueInIntelClauseGeneric(W, I->getOrig());
  }
}

// Return the size_t type for 32/64 bit architecture
Type *VPOParoptTransform::getSizeTTy() {
  LLVMContext &C = F->getContext();

  IntegerType *IntTy;
  const DataLayout &DL = F->getParent()->getDataLayout();

  if (DL.getIntPtrType(Type::getInt8PtrTy(C))->getIntegerBitWidth() == 64)
    IntTy = Type::getInt64Ty(C);
  else
    IntTy = Type::getInt32Ty(C);
  return IntTy;
}

// Returns the corresponding flag for a given map clause modifier.
unsigned VPOParoptTransform::getMapTypeFlag(MapItem *MapI, bool IsFirstExprFlag,
                                            bool IsFirstComponentFlag) {
  unsigned Res = 0u;

  if (MapI->getIsMapTofrom())
    Res = TGT_MAP_TO | TGT_MAP_FROM;
  else if (MapI->getIsMapTo() || MapI->getInFirstprivate())
    Res = TGT_MAP_TO;
  else if (MapI->getIsMapFrom())
    Res = TGT_MAP_FROM;
  else if (MapI->getIsMapDelete())
    Res = TGT_MAP_DELETE;

  // WRNMapAlloc and WRNMapRelease are the default behavior in the runtime.

  if (MapI->getIsMapAlways())
    Res |= TGT_MAP_ALWAYS;

  if (IsFirstExprFlag)
    Res |= TGT_MAP_IS_PTR;
  if (IsFirstComponentFlag)
    Res |= TGT_MAP_FIRST_REF;

  return Res;
}

// Generate the sizes and map type flags for the given map type, map
// modifier and the expression V.
void VPOParoptTransform::GenTgtInformationForPtrs(
    WRegionNode *W, Value *V, SmallVectorImpl<Constant *> &ConstSizes,
    SmallVectorImpl<uint32_t> &MapTypes) {
  const DataLayout DL = F->getParent()->getDataLayout();

  bool IsFirstExprFlag = true;
  bool IsFirstComponentFlag = true;

  MapClause MpClause = W->getMap();
  for (MapItem *MapI : MpClause.items()) {
    if (MapI->getNew() != V)
      continue;
    Type *T = MapI->getOrig()->getType()->getPointerElementType();
    ConstSizes.push_back(
        ConstantInt::get(getSizeTTy(), DL.getTypeAllocSize(T)));
    MapTypes.push_back(
        getMapTypeFlag(MapI, !IsFirstExprFlag, IsFirstComponentFlag));
    IsFirstExprFlag = false;
  }

  if (W->canHaveFirstprivate()) {
    FirstprivateClause &FprivClause = W->getFpriv();
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      if (FprivI->getOrig() != V)
        continue;
      if (FprivI->getInMap())
        continue;
      Type *T = FprivI->getOrig()->getType()->getPointerElementType();
      ConstSizes.push_back(
          ConstantInt::get(getSizeTTy(), DL.getTypeAllocSize(T)));
      MapTypes.push_back(TGT_MAP_TO);
    }
  }

  if (W->canHaveIsDevicePtr()) {
    IsDevicePtrClause IDevicePtrClause = W->getIsDevicePtr();
    for (IsDevicePtrItem *IsDevicePtrI : IDevicePtrClause.items()) {
      if (IsDevicePtrI->getNew() != V)
        continue;
      Type *T = getSizeTTy();
      ConstSizes.push_back(ConstantInt::get(T, DL.getTypeAllocSize(T)));
      MapTypes.push_back(TGT_MAP_PRIVATE_VAL | TGT_MAP_FIRST_REF);
    }
  }
}

// Generate the initialization code for the directive omp target.
CallInst *VPOParoptTransform::genTargetInitCode(WRegionNode *W, CallInst *Call,
                                                Instruction *InsertPt) {
  DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTargetInitCode\n");
  IRBuilder<> Builder(F->getEntryBlock().getFirstNonPHI());
  TgDataInfo Info;

  MapClause MpClause = W->getMap();
  Info.NumberOfPtrs = Call->getNumArgOperands();

  if (Info.NumberOfPtrs) {

    AllocaInst *TgBasePointersArray = Builder.CreateAlloca(
        ArrayType::get(Builder.getInt8PtrTy(), Info.NumberOfPtrs), nullptr,
        ".offload_baseptrs");

    AllocaInst *TgPointersArray = Builder.CreateAlloca(
        ArrayType::get(Builder.getInt8PtrTy(), Info.NumberOfPtrs), nullptr,
        ".offload_ptrs");

    SmallVector<Constant *, 16> ConstSizes;
    SmallVector<uint32_t, 16> MapTypes;

    for (unsigned II = 0; II < Call->getNumArgOperands(); ++II) {
      Value *BPVal = Call->getArgOperand(II);
      GenTgtInformationForPtrs(W, BPVal, ConstSizes, MapTypes);
    }

    auto *SizesArrayInit = ConstantArray::get(
        ArrayType::get(getSizeTTy(), ConstSizes.size()), ConstSizes);

    GlobalVariable *SizesArrayGbl = new GlobalVariable(
        *(F->getParent()), SizesArrayInit->getType(), true,
        GlobalValue::PrivateLinkage, SizesArrayInit, ".offload_sizes", nullptr);
    SizesArrayGbl->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

    Constant *MapTypesArrayInit =
        ConstantDataArray::get(Builder.getContext(), MapTypes);
    auto *MapTypesArrayGbl =
        new GlobalVariable(*(F->getParent()), MapTypesArrayInit->getType(),
                           true, GlobalValue::PrivateLinkage, MapTypesArrayInit,
                           ".offload_maptypes", nullptr);
    MapTypesArrayGbl->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

    Info.BaseDataPtrs = TgBasePointersArray;
    Info.DataPtrs = TgPointersArray;
    Info.DataSizes = SizesArrayGbl;
    Info.DataMapTypes = MapTypesArrayGbl;

    genOffloadArraysInit(W, &Info, Call, InsertPt);
  }

  genOffloadArraysArgument(&Info, InsertPt);

  GlobalVariable *OffloadRegionId = getOMPOffloadRegionId();

  CallInst *TgtCall;
  if (isa<WRNTargetNode>(W))
    TgtCall = VPOParoptUtils::genTgtTarget(
        W, OffloadRegionId, Info.NumberOfPtrs, Info.ResBaseDataPtrs,
        Info.ResDataPtrs, Info.ResDataSizes, Info.ResDataMapTypes, InsertPt);
  else if (isa<WRNTargetDataNode>(W)) {
    TgtCall = VPOParoptUtils::genTgtTargetDataBegin(
        W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
        Info.ResDataSizes, Info.ResDataMapTypes, InsertPt);
    genOffloadArraysArgument(&Info, Call);
    VPOParoptUtils::genTgtTargetDataEnd(
        W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
        Info.ResDataSizes, Info.ResDataMapTypes, Call);
  }

  DEBUG(dbgs() << "\nExit VPOParoptTransform::genTargetInitCode\n");
  return TgtCall;
}

// Pass the data to the array of base pointer as well as  array of
// section pointers.
void VPOParoptTransform::genOffloadArraysInit(WRegionNode *W, TgDataInfo *Info,
                                              CallInst *Call,
                                              Instruction *InsertPt) {
  IRBuilder<> Builder(InsertPt);
  int Cnt = 0;

  for (unsigned II = 0; II < Call->getNumArgOperands(); ++II) {
    Value *BPVal = Call->getArgOperand(II);
    if (BPVal->getType()->isPointerTy())
      BPVal = Builder.CreateBitCast(BPVal, Builder.getInt8PtrTy());
    else
      BPVal = Builder.CreateIntToPtr(BPVal, Builder.getInt8PtrTy());
    Value *BP = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Builder.getInt8PtrTy(), Info->NumberOfPtrs),
        Info->BaseDataPtrs, 0, Cnt);
    Builder.CreateStore(BPVal, BP);

    Value *P = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Builder.getInt8PtrTy(), Info->NumberOfPtrs),
        Info->DataPtrs, 0, Cnt);
    Builder.CreateStore(BPVal, P);
    Cnt++;
  }
}

// Return/Create a variable that binds the atexit to this shared
// object.
GlobalVariable *VPOParoptTransform::getDsoHandle() {
  if (DsoHandle)
    return DsoHandle;
  LLVMContext &C = F->getContext();
  Module *M = F->getParent();

  DsoHandle =
      new GlobalVariable(*M, Type::getInt8Ty(C), false,
                         GlobalValue::ExternalLinkage, nullptr, "__dso_handle");

  DsoHandle->setVisibility(GlobalValue::HiddenVisibility);
  return DsoHandle;
}

// Return/Create the target region ID used by the runtime library to
// identify the current target region.
GlobalVariable *VPOParoptTransform::getOMPOffloadRegionId() {
  if (TgOffloadRegionId)
    return TgOffloadRegionId;

  LLVMContext &C = F->getContext();
  TgOffloadRegionId = new GlobalVariable(
      *(F->getParent()), Type::getInt8Ty(C), true, GlobalValue::PrivateLinkage,
      Constant::getNullValue(Type::getInt8Ty(C)), ".omp_offload.region_id");

  return TgOffloadRegionId;
}

// Generate the pointers pointing to the array of base pointer, the
// array of section pointers, the array of sizes, the array of map types.
void VPOParoptTransform::genOffloadArraysArgument(TgDataInfo *Info,
                                                  Instruction *InsertPt) {
  IRBuilder<> Builder(InsertPt);

  if (Info->NumberOfPtrs) {
    Info->ResBaseDataPtrs = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Builder.getInt8PtrTy(), Info->NumberOfPtrs),
        Info->BaseDataPtrs, 0, 0);
    Info->ResDataPtrs = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Builder.getInt8PtrTy(), Info->NumberOfPtrs),
        Info->DataPtrs, 0, 0);
    Info->ResDataSizes = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(getSizeTTy(), Info->NumberOfPtrs), Info->DataSizes, 0,
        0);
    Info->ResDataMapTypes = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Type::getInt32Ty(F->getContext()), Info->NumberOfPtrs),
        Info->DataMapTypes, 0, 0);
  } else {
    Info->ResBaseDataPtrs = ConstantPointerNull::get(Builder.getInt8PtrTy());
    Info->ResDataPtrs = ConstantPointerNull::get(Builder.getInt8PtrTy());
    Info->ResDataSizes =
        ConstantPointerNull::get(PointerType::getUnqual(getSizeTTy()));
    Info->ResDataMapTypes = ConstantPointerNull::get(
        PointerType::getUnqual(Type::getInt32Ty(F->getContext())));
  }
}

// \brief Hold the struct type as follows.
//    struct __tgt_offload_entry {
//      void      *addr;       // The address of a global variable
//                             // or entry point in the host.
//      char      *name;       // Name of the symbol referring to the
//                             // global variable or entry point.
//      size_t     size;       // Size in bytes of the global variable or
//                             // zero if it is entry point.
//      int32_t    flags;      // Flags of the entry.
//      int32_t    reserved;   // Reserved by the runtime library.
// };
StructType *VPOParoptTransform::getTgOffloadEntryTy() {
  if (TgOffloadEntryTy)
    return TgOffloadEntryTy;

  LLVMContext &C = F->getContext();

  Type *TyArgs[] = { Type::getInt8PtrTy(C), Type::getInt8PtrTy(C), getSizeTTy(),
                     Type::getInt32Ty(C),   Type::getInt32Ty(C) };
  TgOffloadEntryTy =
      StructType::create(C, TyArgs, "struct.__tgt_offload_entry", false);
  return TgOffloadEntryTy;
}

// \brief Hold the struct type as follows.
// struct __tgt_device_image{
//   void   *ImageStart;       // The address of the beginning of the
//                             // target code.
//   void   *ImageEnd;         // The address of the end of the target
//                             // code.
//   __tgt_offload_entry  *EntriesBegin;  // The first element of an array
//                                        // containing the globals and
//                                        // target entry points.
//   __tgt_offload_entry  *EntriesEnd;    // The last element of an array
//                                        // containing the globals and
//                                        // target entry points.
// };
StructType *VPOParoptTransform::getTgDeviceImageTy() {
  if (TgDeviceImageTy)
    return TgDeviceImageTy;
  LLVMContext &C = F->getContext();
  Type *TyArgs[] = {Type::getInt8PtrTy(C), Type::getInt8PtrTy(C),
                    PointerType::getUnqual(getTgOffloadEntryTy()),
                    PointerType::getUnqual(getTgOffloadEntryTy())};
  TgDeviceImageTy =
      StructType::create(C, TyArgs, "struct.__tgt_device_image", false);
  return TgDeviceImageTy;
}

// \brief Hold the struct type as follows.
// struct __tgt_bin_desc{
//   uint32_t              NumDevices;     // Number of device types i
//                                         // supported.
//   __tgt_device_image   *DeviceImages;   // A pointer to an array of
//                                         // NumDevices elements.
//   __tgt_offload_entry  *EntriesBegin;   // The first element of an array
//                                         // containing the globals and
//                                         // target entry points.
//   __tgt_offload_entry  *EntriesEnd;     // The last element of an array
//                                         // containing the globals and
//                                         // target entry points.
// };
//
StructType *VPOParoptTransform::getTgBinaryDescriptorTy() {
  if (TgBinaryDescriptorTy)
    return TgBinaryDescriptorTy;

  LLVMContext &C = F->getContext();
  Type *TyArgs[] = {Type::getInt32Ty(C),
                    PointerType::getUnqual(getTgDeviceImageTy()),
                    PointerType::getUnqual(getTgOffloadEntryTy()),
                    PointerType::getUnqual(getTgOffloadEntryTy())};
  TgBinaryDescriptorTy =
      StructType::create(C, TyArgs, "struct.__tgt_bin_desc", false);
  return TgBinaryDescriptorTy;
}

// Create offloading entry for the provided entry ID and address.
void VPOParoptTransform::genOffloadEntry(Constant *ID, Constant *Addr) {
  StringRef Name = Addr->getName();
  LLVMContext &C = F->getContext();
  Module *M = F->getParent();

  Constant *AddrPtr = ConstantExpr::getBitCast(ID, Type::getInt8PtrTy(C));

  Constant *StrPtrInit = ConstantDataArray::getString(C, Name);

  GlobalVariable *Str = new GlobalVariable(
      *M, StrPtrInit->getType(), /*isConstant=*/true,
      GlobalValue::InternalLinkage, StrPtrInit, ".omp_offloading.entry_name");
  Str->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
  Str->setTargetDeclare(true);
  Constant *StrPtr = llvm::ConstantExpr::getBitCast(Str, Type::getInt8PtrTy(C));

  SmallVector<Constant *, 16> EntryInitBuffer;
  EntryInitBuffer.push_back(AddrPtr);
  EntryInitBuffer.push_back(StrPtr);
  EntryInitBuffer.push_back(ConstantInt::get(getSizeTTy(), 0));
  EntryInitBuffer.push_back(ConstantInt::get(Type::getInt32Ty(C), 0));
  EntryInitBuffer.push_back(ConstantInt::get(Type::getInt32Ty(C), 0));

  Constant *EntryInit =
      ConstantStruct::get(getTgOffloadEntryTy(), EntryInitBuffer);

  GlobalVariable *Entry =
      new GlobalVariable(*M, EntryInit->getType(),
                         /*isConstant=*/true, GlobalValue::ExternalLinkage,
                         EntryInit, ".omp_offloading.entry");

  Entry->setTargetDeclare(true);
  Entry->setSection(".omp_offloading.entries");
}

// Register the offloading descriptors as well the offloading binary
// descriptors.
void VPOParoptTransform::genRegistrationFunction(WRegionNode *W, Function *Fn) {
  genOffloadEntriesAndInfoMetadata(W, Fn);
  genOffloadingBinaryDescriptorRegistration(W);
}

// Register the offloading descriptors.
void
VPOParoptTransform::genOffloadEntriesAndInfoMetadata(WRegionNode *W,
                                                     Function *OutlinedFn) {
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  M->getOrInsertNamedMetadata("omp_offload.info");
  if (Mode & OmpOffload) {
    Constant *OutlinedFnID =
        ConstantExpr::getBitCast(OutlinedFn, Type::getInt8PtrTy(C));
    OutlinedFn->setLinkage(GlobalValue::ExternalLinkage);
    genOffloadEntry(OutlinedFnID, OutlinedFn);
  } else
    genOffloadEntry(getOMPOffloadRegionId(), OutlinedFn);
}

// Register the offloading binary descriptors.
void
VPOParoptTransform::genOffloadingBinaryDescriptorRegistration(WRegionNode *W) {
  if (Mode & OmpOffload)
    return;
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  auto OffloadEntryTy = getTgOffloadEntryTy();
  GlobalVariable *HostEntriesBegin = new GlobalVariable(
      *M, OffloadEntryTy, /*isConstant=*/true, GlobalValue::ExternalLinkage,
      /*Initializer=*/nullptr, ".omp_offloading.entries_begin");
  GlobalVariable *HostEntriesEnd = new GlobalVariable(
      *M, OffloadEntryTy, /*isConstant=*/true,
      llvm::GlobalValue::ExternalLinkage, /*Initializer=*/nullptr,
      ".omp_offloading.entries_end");

  SmallVector<Constant*, 16> DeviceImagesInit;
  for (const auto &T : TgtDeviceTriples) {
    const auto &N = T.getTriple();

    auto *ImgBegin = new GlobalVariable(
      *M, Type::getInt8Ty(C), /*isConstant=*/true, GlobalValue::ExternalLinkage,
      /*Initializer=*/nullptr, Twine(".omp_offloading.img_start.") + Twine(N));
    auto *ImgEnd = new GlobalVariable(
      *M, Type::getInt8Ty(C), /*isConstant=*/true, GlobalValue::ExternalLinkage,
      /*Initializer=*/nullptr, Twine(".omp_offloading.img_end.") + Twine(N));

    SmallVector<Constant*, 4> DevInitBuffer;
    DevInitBuffer.push_back(ImgBegin);
    DevInitBuffer.push_back(ImgEnd);
    DevInitBuffer.push_back(HostEntriesBegin);
    DevInitBuffer.push_back(HostEntriesEnd);

    Constant *DevInit = ConstantStruct::get(getTgDeviceImageTy(), DevInitBuffer);
    DeviceImagesInit.push_back(DevInit);
  }

  Constant *DevArrayInit = ConstantArray::get(
      ArrayType::get(getTgDeviceImageTy(), DeviceImagesInit.size()),
      DeviceImagesInit);

  GlobalVariable *DeviceImages =
      new GlobalVariable(*M, DevArrayInit->getType(),
                         /*isConstant=*/true, GlobalValue::InternalLinkage,
                         DevArrayInit, ".omp_offloading.device_images");
  DeviceImages->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

  Constant *Index[] = { Constant::getNullValue(Type::getInt32Ty(C)),
                        Constant::getNullValue(Type::getInt32Ty(C)) };

  SmallVector<Constant *, 16> DescInitBuffer;
  DescInitBuffer.push_back(
      ConstantInt::get(Type::getInt32Ty(C), TgtDeviceTriples.size()));
  DescInitBuffer.push_back(ConstantExpr::getGetElementPtr(
      DeviceImages->getValueType(), DeviceImages, Index));
  DescInitBuffer.push_back(HostEntriesBegin);
  DescInitBuffer.push_back(HostEntriesEnd);

  Constant *DescInit =
      ConstantStruct::get(getTgBinaryDescriptorTy(), DescInitBuffer);
  GlobalVariable *Desc =
      new GlobalVariable(*M, DescInit->getType(),
                         /*isConstant=*/true, GlobalValue::InternalLinkage,
                         DescInit, ".omp_offloading.descriptor");
  Function *TgDescUnregFn = createTgDescUnregisterLib(W, Desc);
  createTgDescRegisterLib(W, TgDescUnregFn, Desc);
}

// Create the function .omp_offloading.descriptor_unreg.
Function *VPOParoptTransform::createTgDescUnregisterLib(WRegionNode *W,
                                                        GlobalVariable *Desc) {
  LLVMContext &C = F->getContext();
  Module *M = F->getParent();

  Type *Params[] = { Type::getInt8PtrTy(C) };
  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), Params, false);

  Function *Fn = Function::Create(FnTy, GlobalValue::InternalLinkage,
                                  ".omp_offloading.descriptor_unreg", M);
  Fn->setCallingConv(CallingConv::C);

  BasicBlock *EntryBB = BasicBlock::Create(C, "entry", Fn);

  DominatorTree DT;
  DT.recalculate(*Fn);

  IRBuilder<> Builder(EntryBB);

  Builder.CreateRetVoid();
  VPOParoptUtils::genTgtUnregisterLib(Desc, EntryBB->getTerminator());
  Fn->setSection(".text.startup");

  return Fn;
}

// Create the function .omp_offloading.descriptor_reg
Function *VPOParoptTransform::createTgDescRegisterLib(WRegionNode *W,
                                                      Function *TgDescUnregFn,
                                                      GlobalVariable *Desc) {
  LLVMContext &C = F->getContext();
  Module *M = F->getParent();

  Type *Params[] = { Type::getInt8PtrTy(C) };
  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), Params, false);

  Function *Fn = Function::Create(FnTy, GlobalValue::InternalLinkage,
                                  ".omp_offloading.descriptor_reg", M);
  Fn->setCallingConv(CallingConv::C);

  BasicBlock *EntryBB = BasicBlock::Create(C, "entry", Fn);

  DominatorTree DT;
  DT.recalculate(*Fn);

  IRBuilder<> Builder(EntryBB);

  Builder.CreateRetVoid();

  VPOParoptUtils::genTgtRegisterLib(Desc, EntryBB->getTerminator());
  VPOParoptUtils::genCxaAtExit(TgDescUnregFn, Desc, getDsoHandle(),
                               EntryBB->getTerminator());

  Fn->setSection(".text.startup");
  Fn->addFnAttr("offload.ctor", "true");
  return Fn;
}

// The utility to generate the stack variable to pass the value of
// global variable.
Value *VPOParoptTransform::genGlobalPrivatizationImpl(WRegionNode *W,
                                                      GlobalVariable *G,
                                                      BasicBlock *EntryBB,
                                                      BasicBlock *NextExitBB,
                                                      Item *IT) {
  G->setTargetDeclare(true);
  auto NewPrivInst =
      genPrivatizationAlloca(W, G, EntryBB->getFirstNonPHI(), ".priv.mp");
  genPrivatizationReplacement(W, G, NewPrivInst, IT);
  LoadInst *Load = new LoadInst(G);
  Load->insertAfter(cast<Instruction>(NewPrivInst));
  StoreInst *Store = new StoreInst(Load, NewPrivInst);
  Store->insertAfter(Load);
  IRBuilder<> Builder(NextExitBB->getTerminator());
  Builder.CreateStore(Builder.CreateLoad(NewPrivInst), G);
  return NewPrivInst;
}

// If the incoming data is global variable, Create the stack variable and
// replace the the global variable with the stack variable.
bool VPOParoptTransform::genGlobalPrivatizationCode(WRegionNode *W) {
  MapClause MpClause = W->getMap();
  BasicBlock *EntryBB = &(F->getEntryBlock());
  BasicBlock *ExitBB = W->getExitBBlock();
  BasicBlock *NextExitBB = SplitBlock(ExitBB, ExitBB->getTerminator(), DT, LI);
  W->populateBBSet();

  bool Changed = false;
  for (MapItem *MapI : MpClause.items()) {
    Value *Orig = MapI->getOrig();
    if (GlobalVariable *G = dyn_cast<GlobalVariable>(Orig)) {

      Value *NewPrivInst =
          genGlobalPrivatizationImpl(W, G, EntryBB, NextExitBB, MapI);

      MapI->setNew(NewPrivInst);
      Changed = true;
    } else
      // The New is set to be the same as Orig for local firstprivate so that
      // global/local firstprivate can be processed in a unified way in the
      // later OMP code generation.
      MapI->setNew(Orig);
  }

  if (W->canHaveFirstprivate()) {
    FirstprivateClause &FprivClause = W->getFpriv();
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      Value *Orig = FprivI->getOrig();
      MapItem *MapI = FprivI->getInMap();
      if (MapI) {
        FprivI->setNew(MapI->getNew());
        continue;
      }
      if (GlobalVariable *G = dyn_cast<GlobalVariable>(Orig)) {

        Value *NewPrivInst =
            genGlobalPrivatizationImpl(W, G, EntryBB, NextExitBB, FprivI);

        FprivI->setNew(NewPrivInst);
        Changed = true;
      } else
        FprivI->setNew(Orig);
    }
  }

  if (Changed)
    W->resetBBSet();
  return Changed;
}

// Pass the value of the DevicePtr to the outlined function.
bool VPOParoptTransform::genDevicePtrPrivationCode(WRegionNode *W) {
  bool Changed = false;
  if (!W->canHaveIsDevicePtr())
    return Changed;
  IsDevicePtrClause IDevicePtrClause = W->getIsDevicePtr();
  if (!IDevicePtrClause.empty()) {
    W->populateBBSet();
    BasicBlock *EntryBB = W->getEntryBBlock();
    BasicBlock *PrivInitEntryBB = nullptr;
    Changed = true;
    IRBuilder<> Builder(W->getPredBBlock()->getTerminator());
    for (IsDevicePtrItem *IsDevicePtrI : IDevicePtrClause.items()) {
      Value *Orig = IsDevicePtrI->getOrig();
      Value *NewPrivInst = genPrivatizationAlloca(
          W, Orig, EntryBB->getFirstNonPHI(), ".isdeviceptr");
      genPrivatizationReplacement(W, Orig, NewPrivInst, IsDevicePtrI);
      IsDevicePtrI->setNew(NewPrivInst);
      createEmptyPrvInitBB(W, PrivInitEntryBB);
      Builder.SetInsertPoint(W->getPredBBlock()->getTerminator());
      LoadInst *Load = Builder.CreateLoad(IsDevicePtrI->getOrig());
      Builder.SetInsertPoint(PrivInitEntryBB->getTerminator());
      Builder.CreateStore(Load, NewPrivInst);
      IsDevicePtrI->setNew(Load);
    }
  }
  return Changed;
}

// Process the device information string into the triples.
void VPOParoptTransform::processDeviceTriples() {
  auto TargetDevicesStr = F->getParent()->getTargetDevices();
  std::string::size_type Pos = 0;
  while (true) {
    std::string::size_type Next = TargetDevicesStr.find(',', Pos);
    Triple TT(TargetDevicesStr.substr(Pos, Next - Pos));
    TgtDeviceTriples.push_back(TT);
    if (Next == std::string::npos)
      break;
    Pos = Next + 1;
  }
}
