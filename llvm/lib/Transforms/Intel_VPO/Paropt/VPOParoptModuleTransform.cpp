#if INTEL_COLLAB
//===--- VPOParoptModuleTranform.cpp - Paropt Module Transforms --- C++ -*-===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation. and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Authors:
// --------
// Xinmin Tian (xinmin.tian@intel.com)
//
// Major Revisions:
// ----------------
// Dec 2015: Initial Implementation of MT-code generation (Xinmin Tian)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the ParOpt interface to perform module transformations
/// for OpenMP and Auto-parallelization
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptModuleTransform.h"

#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptTpv.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptTransform.h"

#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "VPOParopt"

// Perform paropt transformations for the module. Each module's function is
// transformed by a separate VPOParoptTransform instance which performs
// paropt transformations on a function level. Then, after tranforming all
// functions, create offload initialization code, emit offload entry table, and
// do necessary code cleanup (f.e. remove functions/globals which should not be
// generated for the target).
bool VPOParoptModuleTransform::doParoptTransforms(
    std::function<vpo::WRegionInfo &(Function &F)> WRegionInfoGetter) {

  bool Changed = false;

  processDeviceTriples();

  /// As new functions to be added, so we need to prepare the
  /// list of functions we want to work on in advance.
  std::vector<Function *> FnList;

  for (auto F = M.begin(), E = M.end(); F != E; ++F) {
    // TODO: need Front-End to set F->hasOpenMPDirective()
    if (F->isDeclaration()) // if(!F->hasOpenMPDirective()))
      continue;
    LLVM_DEBUG(dbgs() << "\n=== VPOParoptPass func: " << F->getName()
                      << " {\n");
    FnList.push_back(&*F);
  }

  // Iterate over all functions which OpenMP directives to perform Paropt
  // transformation and generate MT-code
  for (auto F : FnList) {

    LLVM_DEBUG(dbgs() << "\n=== VPOParoptPass Process func: " << F->getName()
                      << " {\n");

    // Walk the W-Region Graph top-down, and create W-Region List
    WRegionInfo &WI = WRegionInfoGetter(*F);
    WI.buildWRGraph(WRegionCollection::LLVMIR);

    if (WI.WRGraphIsEmpty()) {
      LLVM_DEBUG(dbgs() << "\nNo WRegion Candidates for Parallelization \n");
    }

    LLVM_DEBUG(WI.print(dbgs()));

    //
    // Set up a function pass manager so that we can run some cleanup
    // transforms on the LLVM IR after code gen.
    //
    // legacy::FunctionPassManager FPM(&M);

    LLVM_DEBUG(errs() << "VPOParoptPass: ");
    LLVM_DEBUG(errs().write_escaped(F->getName()) << '\n');

    LLVM_DEBUG(dbgs() << "\n=== VPOParoptPass before ParoptTransformer{\n");

    // AUTOPAR | OPENMP | SIMD | OFFLOAD
    VPOParoptTransform VP(this, F, &WI, WI.getDomTree(), WI.getLoopInfo(),
                          WI.getSE(), WI.getTargetTransformInfo(),
                          WI.getAssumptionCache(), WI.getTargetLibraryInfo(),
                          WI.getAliasAnalysis(), Mode, OptLevel,
                          SwitchToOffload);
    Changed = Changed | VP.paroptTransforms();

    LLVM_DEBUG(dbgs() << "\n}=== VPOParoptPass after ParoptTransformer\n");

    // Remove calls to directive intrinsics since the LLVM back end does not
    // know how to translate them.
    // VPOUtils::stripDirectives(*F);

    // It is possible that stripDirectives eliminates all instructions in a
    // basic block except for the branch instruction. Use CFG simplify to
    // eliminate them.
    // FPM.add(createCFGSimplificationPass());
    // FPM.run(*F);

    LLVM_DEBUG(dbgs() << "\n}=== VPOParopt end func: " << F->getName() << "\n");
  }

  if ((Mode & OmpPar) && (Mode & ParTrans))
    fixTidAndBidGlobals();

  if (!VPOAnalysisUtils::isTargetSPIRV(&M)) {
    // Generate offload initialization code.
    genOffloadingBinaryDescriptorRegistration();

    // Emit offload entries table.
    genOffloadEntries();
  }

  if (((Mode & OmpOffload) || SwitchToOffload) && (Mode & ParTrans)) {
    removeTargetUndeclaredGlobals();
    if (VPOAnalysisUtils::isTargetSPIRV(&M)) {
      // Add the metadata to indicate that the module is OpenCL C version.
      if (!M.getNamedMetadata("spirv.Source")) {
        SmallVector<Metadata *, 8> opSource = {
            ConstantAsMetadata::get(ConstantInt::get(Type::getInt32Ty(C), 3)),
            ConstantAsMetadata::get(
                ConstantInt::get(Type::getInt32Ty(C), 200000))};
        MDNode *srcMD = MDNode::get(C, opSource);
        M.getOrInsertNamedMetadata("spirv.Source")->addOperand(srcMD);
      }
    }
  }

  // Thread private legacy mode implementation
  if (Mode & OmpTpv) {
    VPOParoptTpvLegacyPass VPTL;
    ModuleAnalysisManager DummyMAM;
    PreservedAnalyses PA = VPTL.run(M, DummyMAM);
    Changed = Changed | !PA.areAllPreserved();
  }

  LLVM_DEBUG(dbgs() << "\n====== End VPO ParoptPass ======\n\n");
  return Changed;
}

// Collect the uses of the given global variable.
void VPOParoptModuleTransform::collectUsesOfGlobals(
    Constant *PtrHolder, SmallVectorImpl<Instruction *> &RewriteIns) {
  for (auto IB = PtrHolder->user_begin(), IE = PtrHolder->user_end(); IB != IE;
       IB++) {
    if (Instruction *User = dyn_cast<Instruction>(*IB))
      RewriteIns.push_back(User);
  }
}

// Transform the use of the tid global into __kmpc_global_thread_num or the
// the use of the first argument of the OMP outlined function. The use of
// bid global is transformed accordingly.
void VPOParoptModuleTransform::fixTidAndBidGlobals() {
  Constant *TidPtrHolder =
      M.getOrInsertGlobal("@tid.addr", Type::getInt32Ty(C));
  SmallVector<Instruction *, 8> RewriteIns;

  collectUsesOfGlobals(TidPtrHolder, RewriteIns);
  processUsesOfGlobals(TidPtrHolder, RewriteIns, true);

  RewriteIns.clear();
  Constant *BidPtrHolder =
      M.getOrInsertGlobal("@bid.addr", Type::getInt32Ty(C));
  collectUsesOfGlobals(BidPtrHolder, RewriteIns);
  processUsesOfGlobals(BidPtrHolder, RewriteIns, false);
}

// The utility to transform the tid/bid global variable.
void VPOParoptModuleTransform::processUsesOfGlobals(
    Constant *PtrHolder, SmallVectorImpl<Instruction *> &RewriteIns,
    bool IsTid) {

  while (!RewriteIns.empty()) {
    Instruction *User = RewriteIns.pop_back_val();

    Function *F = User->getParent()->getParent();
    if (F->getAttributes().hasAttribute(AttributeList::FunctionIndex,
                                        "mt-func")) {
      auto IT = F->arg_begin();
      if (!IsTid)
        IT++;
      User->replaceUsesOfWith(PtrHolder, &*IT);
    } else if (IsTid && F->getAttributes().hasAttribute(
                            AttributeList::FunctionIndex, "task-mt-func")) {
      BasicBlock *EntryBB = &F->getEntryBlock();
      IRBuilder<> Builder(EntryBB->getFirstNonPHI());
      AllocaInst *TidPtr =
          Builder.CreateAlloca(Type::getInt32Ty(C));
      Builder.CreateStore(&*(F->arg_begin()), TidPtr);
      User->replaceUsesOfWith(PtrHolder, TidPtr);
    } else {
      BasicBlock *EntryBB = &F->getEntryBlock();
      Instruction *Tid = nullptr;
      AllocaInst *TidPtr = nullptr;
      if (IsTid)
        Tid = VPOParoptUtils::findKmpcGlobalThreadNumCall(EntryBB);
      if (!Tid) {
        IRBuilder<> Builder(EntryBB->getFirstNonPHI());
        TidPtr = Builder.CreateAlloca(Type::getInt32Ty(C));
        if (IsTid) {
          Tid = VPOParoptUtils::genKmpcGlobalThreadNumCall(F, TidPtr, nullptr);
          Tid->insertBefore(EntryBB->getFirstNonPHI());
        }
        StoreInst *SI = nullptr;
        if (IsTid)
          SI = new StoreInst(Tid, TidPtr);
        else
          SI = new StoreInst(
              ConstantInt::get(Type::getInt32Ty(C), 0), TidPtr);
        SI->insertAfter(TidPtr);
      } else {
        for (auto IB = Tid->user_begin(), IE = Tid->user_end(); IB != IE;
             IB++) {
          auto User = dyn_cast<Instruction>(*IB);
          if (User && User->getParent() == Tid->getParent()) {
            StoreInst *SI = dyn_cast<StoreInst>(User);
            if (SI) {
              Value *V = SI->getPointerOperand();
              TidPtr = dyn_cast<AllocaInst>(V);
              break;
            }
          }
        }
      }

      if (TidPtr == nullptr) {
        IRBuilder<> Builder(EntryBB->getFirstNonPHI());
        TidPtr = Builder.CreateAlloca(Type::getInt32Ty(C));
        StoreInst *SI = new StoreInst(Tid, TidPtr);
        SI->insertAfter(Tid);
      }
      User->replaceUsesOfWith(PtrHolder, TidPtr);
    }
  }
}

// Remove routines and global variables which has no target declare
// attribute.
void VPOParoptModuleTransform::removeTargetUndeclaredGlobals() {
  std::vector<GlobalVariable *> DeadGlobalVars; // Keep track of dead globals
  for (GlobalVariable &GV : M.globals())
    if (!GV.isTargetDeclare()) {
      DeadGlobalVars.push_back(&GV); // Keep track of dead globals
      // TODO  The check of use_empty will be removed after the frontend
      // generates target_declare attribute for the variable GV.
      if (GV.use_empty() && GV.hasInitializer()) {
        Constant *Init = GV.getInitializer();
        GV.setInitializer(nullptr);
        if (!isa<GlobalValue>(Init) && !isa<ConstantData>(Init))
          Init->destroyConstant();
      }
    }

  std::vector<Function *> DeadFunctions;

  for (Function &F : M) {
    if (!F.getAttributes().hasAttribute(AttributeList::FunctionIndex,
                                        "target.declare")) {
      DeadFunctions.push_back(&F);
      if (!F.isDeclaration())
        F.deleteBody();
    } else {
#if INTEL_CUSTOMIZATION
      // This is a workaround for resolving the incompatibility issue
      // between the xmain compiler and IGC compiler. The IGC compiler
      // cannot accept the following instruction, which is generated
      // at compile time for helping infering the address space.
      //   %4 = bitcast [10000 x i32] addrspace(1)* %B to i8*
      // The instruction becomes dead after the inferring is done.
#endif // INTEL_CUSTOMIZATION
      for (BasicBlock &BB : F)
        for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E;) {
          Instruction *Inst = &*I++;
          if (isInstructionTriviallyDead(Inst))
            BB.getInstList().erase(Inst);
        }
    }
  }
  auto EraseUnusedGlobalValue = [&](GlobalValue *GV) {
    // TODO  The check of use_empty will be removed after the frontend
    // generates target_declare attribute for the variable GV.
    if (!GV->use_empty())
      return;
    GV->removeDeadConstantUsers();
    GV->eraseFromParent();
  };

  for (GlobalVariable *GV : DeadGlobalVars)
    EraseUnusedGlobalValue(GV);

  for (Function *F : DeadFunctions)
    EraseUnusedGlobalValue(F);
}

// Process the device information string into the triples.
void VPOParoptModuleTransform::processDeviceTriples() {
  auto TargetDevicesStr = M.getTargetDevices();
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

// Hold the struct type as follows.
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
StructType *VPOParoptModuleTransform::getTgOffloadEntryTy() {
  if (TgOffloadEntryTy)
    return TgOffloadEntryTy;

  Type *TyArgs[] = {Type::getInt8PtrTy(C), Type::getInt8PtrTy(C),
                    IntelGeneralUtils::getSizeTTy(&M), Type::getInt32Ty(C),
                    Type::getInt32Ty(C)};
  TgOffloadEntryTy =
      StructType::get(C, TyArgs, /* "struct.__tgt_offload_entry"*/false);
  return TgOffloadEntryTy;
}

// Hold the struct type as follows.
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
StructType *VPOParoptModuleTransform::getTgDeviceImageTy() {
  if (TgDeviceImageTy)
    return TgDeviceImageTy;

  Type *TyArgs[] = {Type::getInt8PtrTy(C), Type::getInt8PtrTy(C),
                    PointerType::getUnqual(getTgOffloadEntryTy()),
                    PointerType::getUnqual(getTgOffloadEntryTy())};
  TgDeviceImageTy =
      StructType::get(C, TyArgs, /* "struct.__tgt_device_image" */false);
  return TgDeviceImageTy;
}

// Hold the struct type as follows.
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
StructType *VPOParoptModuleTransform::getTgBinaryDescriptorTy() {
  if (TgBinaryDescriptorTy)
    return TgBinaryDescriptorTy;

  Type *TyArgs[] = {Type::getInt32Ty(C),
                    PointerType::getUnqual(getTgDeviceImageTy()),
                    PointerType::getUnqual(getTgOffloadEntryTy()),
                    PointerType::getUnqual(getTgOffloadEntryTy())};
  TgBinaryDescriptorTy =
      StructType::get(C, TyArgs, /* "struct.__tgt_bin_desc" */false);
  return TgBinaryDescriptorTy;
}

// Return/Create a variable that binds the atexit to this shared
// object.
GlobalVariable *VPOParoptModuleTransform::getDsoHandle() {
  if (DsoHandle)
    return DsoHandle;

  DsoHandle =
      new GlobalVariable(M, Type::getInt8Ty(C), false,
                         GlobalValue::ExternalLinkage, nullptr, "__dso_handle");

  DsoHandle->setVisibility(GlobalValue::HiddenVisibility);
  return DsoHandle;
}

// Register the offloading binary descriptors.
void VPOParoptModuleTransform::genOffloadingBinaryDescriptorRegistration() {
  if (hasOffloadCompilation() || TargetRegions.empty())
    return;

  auto OffloadEntryTy = getTgOffloadEntryTy();
  GlobalVariable *HostEntriesBegin = new GlobalVariable(
      M, OffloadEntryTy, /*isConstant=*/true, GlobalValue::ExternalLinkage,
      /*Initializer=*/nullptr, ".omp_offloading.entries_begin");
  GlobalVariable *HostEntriesEnd = new GlobalVariable(
      M, OffloadEntryTy, /*isConstant=*/true,
      llvm::GlobalValue::ExternalLinkage, /*Initializer=*/nullptr,
      ".omp_offloading.entries_end");

  SmallVector<Constant*, 8u> DeviceImagesInit;
  for (const auto &T : TgtDeviceTriples) {
    const auto &N = T.getTriple();

    auto *ImgBegin = new GlobalVariable(
      M, Type::getInt8Ty(C), /*isConstant=*/true,
      GlobalValue::ExternalWeakLinkage,
      /*Initializer=*/nullptr, Twine(".omp_offloading.img_start.") + Twine(N));
    auto *ImgEnd = new GlobalVariable(
      M, Type::getInt8Ty(C), /*isConstant=*/true,
      GlobalValue::ExternalWeakLinkage,
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
      new GlobalVariable(M, DevArrayInit->getType(),
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
      new GlobalVariable(M, DescInit->getType(),
                         /*isConstant=*/true, GlobalValue::InternalLinkage,
                         DescInit, ".omp_offloading.descriptor");
  Function *TgDescUnregFn = createTgDescUnregisterLib(Desc);
  Function *TgDescRegFn = createTgDescRegisterLib(TgDescUnregFn, Desc);

  // It is sufficient to call offload registration code once per unique
  // combination of target triples. Therefore create a comdat group for the
  // registration/unregistration functions and associated data. Registration
  // function name serves as a key for this comdat group (it includes sorted
  // offload target triple names).
  auto *ComdatKey = M.getOrInsertComdat(TgDescRegFn->getName());
  TgDescRegFn->setLinkage(GlobalValue::LinkOnceAnyLinkage);
  TgDescRegFn->setVisibility(GlobalValue::HiddenVisibility);
  TgDescRegFn->setComdat(ComdatKey);
  TgDescUnregFn->setComdat(ComdatKey);
  DeviceImages->setComdat(ComdatKey);
  Desc->setComdat(ComdatKey);
  appendToGlobalCtors(M, TgDescRegFn, 0, TgDescRegFn);
}

// Create the function .omp_offloading.descriptor_unreg.
Function *VPOParoptModuleTransform::createTgDescUnregisterLib(
    GlobalVariable *Desc) {
  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), false);

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
Function *VPOParoptModuleTransform::createTgDescRegisterLib(
    Function *TgDescUnregFn, GlobalVariable *Desc) {
  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), false);

  SmallVector<StringRef, 4u> DeviceNames(TgtDeviceTriples.size());
  transform(TgtDeviceTriples, DeviceNames.begin(),
            [](const Triple &T) -> const std::string& {
               return T.getTriple();
            });
  sort(DeviceNames.begin(), DeviceNames.end());
  SmallString<128u> FnName;
  {
    raw_svector_ostream OS(FnName);
    OS << ".omp_offloading.descriptor_reg";
    for (auto &T : DeviceNames)
      OS << "." << T;
  }

  Function *Fn = Function::Create(FnTy, GlobalValue::InternalLinkage,
                                  FnName, M);
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
  return Fn;
}

Constant* VPOParoptModuleTransform::registerTargetRegion(WRegionNode *W,
                                                         Constant *Func) {
  // Offload runtime needs an address which uniquely identifies the target
  // region. On the device side it has to be an address of the outlined
  // target region, but on the host side it can be anything which can uniquely
  // identify the region. Therefore, for the host compilation, we can just
  // create a variable which would serve as target region ID. Since we are not
  // taking address of the outlined function on the host side it can still be
  // inlined.
  Constant *ID = Func;
  if (!hasOffloadCompilation())
    ID = new GlobalVariable(M, Type::getInt8Ty(C), true,
                            GlobalValue::WeakAnyLinkage,
                            Constant::getNullValue(Type::getInt8Ty(C)),
                            Func->getName() + ".region_id");
  TargetRegions.push_back(std::make_pair(Func, ID));
  return ID;
}

// Create offloading entry for the provided entry ID and address.
void VPOParoptModuleTransform::genOffloadEntries() {
  if (TargetRegions.empty())
    return;

  Type *VoidStarTy = Type::getInt8PtrTy(C);
  Type *SizeTy = IntelGeneralUtils::getSizeTTy(&M);
  Type *Int32Ty = Type::getInt32Ty(C);

  for (auto &E : TargetRegions) {
    auto *Func = E.first;
    auto *ID = E.second;

    StringRef Name = Func->getName();

    Constant *StrInit = ConstantDataArray::getString(C, Name);

    GlobalVariable *Str = new GlobalVariable(
        M, StrInit->getType(), /*isConstant=*/true,
        GlobalValue::InternalLinkage, StrInit, ".omp_offloading.entry_name");
    Str->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
    Str->setTargetDeclare(true);

    SmallVector<Constant *, 5u> EntryInitBuffer;
    EntryInitBuffer.push_back(ConstantExpr::getBitCast(ID, VoidStarTy));
    EntryInitBuffer.push_back(ConstantExpr::getBitCast(Str, VoidStarTy));
    EntryInitBuffer.push_back(ConstantInt::get(SizeTy, 0));
    EntryInitBuffer.push_back(ConstantInt::get(Int32Ty, 0));
    EntryInitBuffer.push_back(ConstantInt::get(Int32Ty, 0));

    Constant *EntryInit =
        ConstantStruct::get(getTgOffloadEntryTy(), EntryInitBuffer);

    GlobalVariable *Entry =
        new GlobalVariable(M, EntryInit->getType(),
                           /*isConstant=*/true, GlobalValue::WeakAnyLinkage,
                           EntryInit, ".omp_offloading.entry." + Name);

    Entry->setTargetDeclare(true);
    Entry->setSection(".omp_offloading.entries");
  }
}
#endif // INTEL_COLLAB
