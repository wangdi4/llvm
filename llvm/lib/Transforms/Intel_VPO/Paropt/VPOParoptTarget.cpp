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
  resetValueInIfClause(W);
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

    W->resetBBSet(); // Invalidate BBSet after transformations

    Changed = true;
  }
  DEBUG(dbgs() << "\nExit VPOParoptTransform::genTargetOffloadingCode\n");
  return Changed;
}

// Return the size_t type for 32/bit architecture
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

// Generate the initialization code for the directive omp target.
CallInst *VPOParoptTransform::genTargetInitCode(WRegionNode *W, CallInst *Call,
                                                Instruction *InsertPt) {
  IRBuilder<> Builder(InsertPt);
  TargetDataInfo Info;

  MapClause MpClause = W->getMap();
  Info.NumberOfPtrs = MpClause.size();

  if (Info.NumberOfPtrs) {

    AllocaInst *TgBasePointersArray = Builder.CreateAlloca(
        ArrayType::get(Builder.getInt8PtrTy(), Info.NumberOfPtrs), nullptr,
        ".offload_baseptrs");

    AllocaInst *TgPointersArray = Builder.CreateAlloca(
        ArrayType::get(Builder.getInt8PtrTy(), Info.NumberOfPtrs), nullptr,
        ".offload_ptrs");

    SmallVector<llvm::Constant *, 16> ConstSizes;

    const DataLayout DL = F->getParent()->getDataLayout();
    for (MapItem *MpI : MpClause.items()) {
      Type *T = MpI->getOrig()->getType()->getPointerElementType();
      ConstSizes.push_back(
          ConstantInt::get(getSizeTTy(), DL.getTypeAllocSize(T)));
    }
    auto *SizesArrayInit = ConstantArray::get(
        ArrayType::get(getSizeTTy(), ConstSizes.size()), ConstSizes);

    GlobalVariable *SizesArrayGbl = new GlobalVariable(
        *(F->getParent()), SizesArrayInit->getType(), true,
        GlobalValue::PrivateLinkage, SizesArrayInit, ".offload_sizes", nullptr);
    SizesArrayGbl->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

    SmallVector<uint32_t, 16> MapTypes;
    getMapTypes(W, MapTypes);
    Constant *MapTypesArrayInit =
        ConstantDataArray::get(Builder.getContext(), MapTypes);
    auto *MapTypesArrayGbl =
        new GlobalVariable(*(F->getParent()), MapTypesArrayInit->getType(),
                           true, GlobalValue::PrivateLinkage, MapTypesArrayInit,
                           ".offload_maptypes", nullptr);
    MapTypesArrayGbl->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

    Info.BasePointersArray = TgBasePointersArray;
    Info.PointersArray = TgPointersArray;
    Info.SizesArray = SizesArrayGbl;
    Info.MapTypesArray = MapTypesArrayGbl;

    genOffloadArraysInit(W, &Info, Call, InsertPt);
  }

  genOffloadArraysArgument(&Info, InsertPt);

  GlobalVariable *OffloadRegionId = getOMPOffloadRegionId();

  return VPOParoptUtils::genTgtTarget(
      W, OffloadRegionId, Info.NumberOfPtrs, Info.BasePointersArray,
      Info.PointersArray, Info.SizesArray, Info.MapTypesArray, InsertPt);
}

// Return the map type for the data clause.
void VPOParoptTransform::getMapTypes(WRegionNode *W,
                                     SmallVectorImpl<uint32_t> &MapTypes) {
  MapClause MpClause = W->getMap();
  for (MapItem *MpI : MpClause.items())
    MapTypes.push_back(MpI->getMapKind() | 0x20);
}

// Pass the data to the array of base pointer as well as  array of
// section pointers.
void VPOParoptTransform::genOffloadArraysInit(WRegionNode *W,
                                              TargetDataInfo *Info,
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
        Info->BasePointersArray, 0, Cnt);
    Builder.CreateStore(BPVal, BP);

    Value *P = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Builder.getInt8PtrTy(), Info->NumberOfPtrs),
        Info->PointersArray, 0, Cnt);
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
  if (TgtOffloadRegionId)
    return TgtOffloadRegionId;

  LLVMContext &C = F->getContext();
  TgtOffloadRegionId = new GlobalVariable(
      *(F->getParent()), Type::getInt8Ty(C), true, GlobalValue::PrivateLinkage,
      Constant::getNullValue(Type::getInt8Ty(C)), ".omp_offload.region_id");

  return TgtOffloadRegionId;
}

// Generate the pointers pointing to the array of base pointer, the
// array of section pointers, the array of sizes, the array of map types.
void VPOParoptTransform::genOffloadArraysArgument(TargetDataInfo *Info,
                                                  Instruction *InsertPt) {
  IRBuilder<> Builder(InsertPt);

  if (Info->NumberOfPtrs) {
    Info->BasePointersArray = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Builder.getInt8PtrTy(), Info->NumberOfPtrs),
        Info->BasePointersArray, 0, 0);
    Info->PointersArray = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Builder.getInt8PtrTy(), Info->NumberOfPtrs),
        Info->PointersArray, 0, 0);
    Info->SizesArray = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(getSizeTTy(), Info->NumberOfPtrs), Info->SizesArray, 0,
        0);
    Info->MapTypesArray = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Type::getInt32Ty(F->getContext()), Info->NumberOfPtrs),
        Info->MapTypesArray, 0, 0);
  } else {
    Info->BasePointersArray = ConstantPointerNull::get(Builder.getInt8PtrTy());
    Info->PointersArray = ConstantPointerNull::get(Builder.getInt8PtrTy());
    Info->SizesArray =
        ConstantPointerNull::get(PointerType::getUnqual(getSizeTTy()));
    Info->MapTypesArray = ConstantPointerNull::get(
        PointerType::getUnqual(Type::getInt32Ty(F->getContext())));
  }
}
// struct __tgt_offload_entry{
//   void      *addr;       // Pointer to the offload entry info.
//                          // (function or global)
//   char      *name;       // Name of the function or global.
//   size_t     size;       // Size of the entry info (0 if it a function).
//   int32_t    flags;      // Flags associated with the entry, e.g. 'link'.
//   int32_t    reserved;   // Reserved, to use by the runtime library.
// };
//
StructType *VPOParoptTransform::getTgtOffloadEntryQTy() {
  if (TgtOffloadEntryTy)
    return TgtOffloadEntryTy;

  LLVMContext &C = F->getContext();

  Type *TyArgs[] = { Type::getInt8PtrTy(C), Type::getInt8PtrTy(C), getSizeTTy(),
                     Type::getInt32Ty(C),   Type::getInt32Ty(C) };
  TgtOffloadEntryTy =
      StructType::create(C, TyArgs, "struct.__tgt_offload_entry", false);
  return TgtOffloadEntryTy;
}

// struct __tgt_device_image{
//   void   *ImageStart;       // Pointer to the target code start.
//   void   *ImageEnd;         // Pointer to the target code end.
//     We also add the host entries to the device image, as it may be useful
//     for the target runtime to have access to that information.
//   __tgt_offload_entry  *EntriesBegin;   // Begin of the table with all
//                                         // the entries.
//   __tgt_offload_entry  *EntriesEnd;     // End of the table with all the
//                                         // entries (non inclusive).
// };
//
StructType *VPOParoptTransform::getTgtDeviceImageQTy() {
  if (TgtDeviceImageQTy)
    return TgtDeviceImageQTy;
  LLVMContext &C = F->getContext();
  Type *TyArgs[] = { Type::getInt8PtrTy(C),
                     Type::getInt8PtrTy(C),
                     PointerType::getUnqual(getTgtOffloadEntryQTy()),
                     PointerType::getUnqual(getTgtOffloadEntryQTy()) };
  TgtDeviceImageQTy =
      StructType::create(C, TyArgs, "struct.__tgt_device_image", false);
  return TgtDeviceImageQTy;
}
// struct __tgt_bin_desc{
//   int32_t              NumDevices;      // Number of devices supported.
//   __tgt_device_image   *DeviceImages;   // Arrays of device images
//                                         // (one per device).
//   __tgt_offload_entry  *EntriesBegin;   // Begin of the table with all the
//                                         // entries.
//   __tgt_offload_entry  *EntriesEnd;     // End of the table with all the
//                                         // entries (non inclusive).
// };
StructType *VPOParoptTransform::getTgtBinaryDescriptorQTy() {
  if (TgtBinaryDescriptorQTy)
    return TgtBinaryDescriptorQTy;

  LLVMContext &C = F->getContext();
  Type *TyArgs[] = { Type::getInt32Ty(C),
                     PointerType::getUnqual(getTgtDeviceImageQTy()),
                     PointerType::getUnqual(getTgtOffloadEntryQTy()),
                     PointerType::getUnqual(getTgtOffloadEntryQTy()) };
  TgtBinaryDescriptorQTy =
      StructType::create(C, TyArgs, "struct.__tgt_bin_desc", false);
  return TgtBinaryDescriptorQTy;
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
      ConstantStruct::get(getTgtOffloadEntryQTy(), EntryInitBuffer);

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
  auto OffloadEntryTy = getTgtOffloadEntryQTy();
  GlobalVariable *HostEntriesBegin = new GlobalVariable(
      *M, OffloadEntryTy, /*isConstant=*/true, GlobalValue::ExternalLinkage,
      /*Initializer=*/nullptr, ".omp_offloading.entries_begin");
  GlobalVariable *HostEntriesEnd = new GlobalVariable(
      *M, OffloadEntryTy, /*isConstant=*/true,
      llvm::GlobalValue::ExternalLinkage, /*Initializer=*/nullptr,
      ".omp_offloading.entries_end");
  StringRef T = "x86_64-mic";
  auto *ImgBegin = new GlobalVariable(
      *M, Type::getInt8Ty(C), /*isConstant=*/true, GlobalValue::ExternalLinkage,
      /*Initializer=*/nullptr, Twine(".omp_offloading.img_start.") + Twine(T));
  auto *ImgEnd = new GlobalVariable(
      *M, Type::getInt8Ty(C), /*isConstant=*/true, GlobalValue::ExternalLinkage,
      /*Initializer=*/nullptr, Twine(".omp_offloading.img_end.") + Twine(T));

  SmallVector<Constant *, 16> DevInitBuffer;
  DevInitBuffer.push_back(ImgBegin);
  DevInitBuffer.push_back(ImgEnd);
  DevInitBuffer.push_back(HostEntriesBegin);
  DevInitBuffer.push_back(HostEntriesEnd);

  Constant *DevInit =
      ConstantStruct::get(getTgtDeviceImageQTy(), DevInitBuffer);

  DevInitBuffer.clear();
  DevInitBuffer.push_back(DevInit);
  Constant *DevArrayInit = ConstantArray::get(
      ArrayType::get(getTgtDeviceImageQTy(), 1), DevInitBuffer);

  GlobalVariable *DeviceImages =
      new GlobalVariable(*M, DevArrayInit->getType(),
                         /*isConstant=*/true, GlobalValue::InternalLinkage,
                         DevArrayInit, ".omp_offloading.device_images");
  DeviceImages->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

  Constant *Index[] = { Constant::getNullValue(Type::getInt32Ty(C)),
                        Constant::getNullValue(Type::getInt32Ty(C)) };

  SmallVector<Constant *, 16> DescInitBuffer;
  DescInitBuffer.push_back(ConstantInt::get(Type::getInt32Ty(C), 1));
  DescInitBuffer.push_back(ConstantExpr::getGetElementPtr(
      DeviceImages->getValueType(), DeviceImages, Index));
  DescInitBuffer.push_back(HostEntriesBegin);
  DescInitBuffer.push_back(HostEntriesEnd);

  Constant *DescInit =
      ConstantStruct::get(getTgtBinaryDescriptorQTy(), DescInitBuffer);
  GlobalVariable *Desc =
      new GlobalVariable(*M, DescInit->getType(),
                         /*isConstant=*/true, GlobalValue::InternalLinkage,
                         DescInit, ".omp_offloading.descriptor");
  Function *TgtDescUnregFn = createTgtDescUnRegisterLib(W, Desc);
  createTgtDescRegisterLib(W, TgtDescUnregFn, Desc);
}

// Create the function .omp_offloading.descriptor_unreg.
Function *VPOParoptTransform::createTgtDescUnRegisterLib(WRegionNode *W,
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
  VPOParoptUtils::genTgtUnregisterLib(W, Desc, EntryBB->getTerminator());
  Fn->setSection(".text.startup");

  return Fn;
}

// Create the function .omp_offloading.descriptor_reg
Function *VPOParoptTransform::createTgtDescRegisterLib(WRegionNode *W,
                                                       Function *TgtDescUnregFn,
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
  VPOParoptUtils::genTgtRegisterLib(W, Desc, EntryBB->getTerminator());
  VPOParoptUtils::genCxaAtExit(W, TgtDescUnregFn, Desc, getDsoHandle(),
                               EntryBB->getTerminator());

  Fn->setSection(".text.startup");
  Fn->addFnAttr("offload.ctor", "true");
  return Fn;
}

// If the map data is global variable, Create the stack variable and
// replace the the global variable with the stack variable.
bool VPOParoptTransform::genMapPrivationCode(WRegionNode *W) {
  MapClause MpClause = W->getMap();
  BasicBlock *EntryBB = &(F->getEntryBlock());
  BasicBlock *ExitBB = W->getExitBBlock();
  BasicBlock *NextExitBB = SplitBlock(ExitBB, ExitBB->getTerminator(), DT, LI);
  W->populateBBSet();

  bool Changed = false;
  for (MapItem *MpI : MpClause.items()) {
    Value *Orig = MpI->getOrig();
    if (GlobalVariable *G = dyn_cast<GlobalVariable>(Orig)) {
      G->setTargetDeclare(true);
      auto NewPrivInst = genPrivatizationAlloca(
          W, Orig, EntryBB->getFirstNonPHI(), ".priv.mp");
      genPrivatizationReplacement(W, Orig, NewPrivInst, MpI);
      LoadInst *Load = new LoadInst(Orig);
      Load->insertAfter(cast<Instruction>(NewPrivInst));
      StoreInst *Store = new StoreInst(Load, NewPrivInst);
      Store->insertAfter(Load);
      IRBuilder<> Builder(NextExitBB->getTerminator());
      Builder.CreateStore(Builder.CreateLoad(NewPrivInst), Orig);
      Changed = true;
    }
  }
  return Changed;
}
