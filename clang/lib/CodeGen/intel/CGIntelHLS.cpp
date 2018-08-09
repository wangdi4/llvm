//==--- CodeGenHLS.cpp - HLS-specific Codegen ------------------*- C++ -*---==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "CodeGenFunction.h"

using namespace clang;
using namespace CodeGen;

void CodeGenFunction::EmitHLSComponentMetadata(const FunctionDecl *FD,
                                               llvm::Function *Fn) {
  if (!FD->hasAttr<ComponentAttr>())
    return;

  // ComponentMD contains {name, return type}
  SmallVector<llvm::Metadata *, 10> ComponentMD;
  SmallVector<llvm::Metadata *, 10> ArgTypeMD;
  SmallVector<llvm::Metadata *, 10> ImplTypeMD;
  SmallVector<llvm::Metadata *, 10> StableMD;
  SmallVector<llvm::Metadata *, 10> CosimNameMD;
  SmallVector<llvm::Metadata *, 10> MemoryMD;

  llvm::LLVMContext &Ctx = getLLVMContext();
  llvm::IntegerType *Int32Ty = llvm::Type::getInt32Ty(Ctx);

  // Function name (redundant, but consistent with previous version)
  ComponentMD.push_back(llvm::MDString::get(Ctx, Fn->getName()));

  llvm::Type *retType = Fn->getReturnType();
  if (retType->isVoidTy()) {
    ComponentMD.push_back(llvm::MDNode::get(Ctx, {}));
  } else {
    ComponentMD.push_back(
        llvm::ValueAsMetadata::get(llvm::UndefValue::get(retType)));
  }
  Fn->setMetadata("ihc_component", llvm::MDNode::get(Ctx, ComponentMD));

  for (const ParmVarDecl *PVD : FD->parameters()) {
    const Type *ParamType = PVD->getType().getCanonicalType().getTypePtr();

    StringRef ArgType = "default";
    if (PVD->getAttr<SlaveMemoryArgumentAttr>())
      ArgType = "mm_slave";
    else if (ParamType->isPointerType() || ParamType->isReferenceType())
      ArgType = "pointer";
    ArgTypeMD.push_back(llvm::MDString::get(Ctx, ArgType));

    StringRef ImplType = "wire";
    if (const auto *AIA = PVD->getAttr<ArgumentInterfaceAttr>())
      ImplType =
          ArgumentInterfaceAttr::ConvertArgumentInterfaceTypeToStr(
              AIA->getType());
    ImplTypeMD.push_back(llvm::MDString::get(Ctx, ImplType));

    int Stable = 0;
    if (PVD->hasAttr<StableArgumentAttr>())
      Stable = 1;
    StableMD.push_back(llvm::ConstantAsMetadata::get(
            llvm::ConstantInt::get(Int32Ty, Stable)));

    CosimNameMD.push_back(llvm::MDString::get(Ctx, PVD->getName()));

    SmallString<256> AnnotStr;
    CGM.generateHLSAnnotation(PVD, AnnotStr);
    MemoryMD.push_back(llvm::MDString::get(Ctx, AnnotStr));
  }
  if (FD->getNumParams()) {
    Fn->setMetadata("arg_type", llvm::MDNode::get(Ctx, ArgTypeMD));
    Fn->setMetadata("impl_type", llvm::MDNode::get(Ctx, ImplTypeMD));
    Fn->setMetadata("stable", llvm::MDNode::get(Ctx, StableMD));
    Fn->setMetadata("cosim_name", llvm::MDNode::get(Ctx, CosimNameMD));
    Fn->setMetadata("memory", llvm::MDNode::get(Ctx, MemoryMD));
  }

  const auto *CIA = FD->getAttr<ComponentInterfaceAttr>();
  assert(CIA && "Component should have a ComponentInterfaceAttr");
  StringRef CompIntString =
      ComponentInterfaceAttr::ConvertComponentInterfaceTypeToStr(
          CIA->getType());
  Fn->setMetadata(
      "component_interface",
      llvm::MDNode::get(Ctx, {llvm::MDString::get(Ctx, CompIntString)}));

  int IsStallFreeReturn = FD->hasAttr<StallFreeReturnAttr>() ? 1 : 0;
  llvm::ConstantInt *SFRC = llvm::ConstantInt::get(Int32Ty, IsStallFreeReturn);
  Fn->setMetadata("stall_free_return",
                  llvm::MDNode::get(Ctx, llvm::ConstantAsMetadata::get(SFRC)));

  int IsUseSingleClock = FD->hasAttr<UseSingleClockAttr>() ? 1 : 0;
  llvm::ConstantInt *USCC = llvm::ConstantInt::get(Int32Ty, IsUseSingleClock);
  Fn->setMetadata("use_single_clock",
                  llvm::MDNode::get(Ctx, llvm::ConstantAsMetadata::get(USCC)));
}

void CodeGenFunction::EmitOpenCLHLSComponentMetadata(const FunctionDecl *FD,
                                                     llvm::Function *Fn) {
  llvm::LLVMContext &Context = getLLVMContext();

  // MDNode for the local_mem_size attribute.
  SmallVector<llvm::Metadata*, 8> ArgLocalMemSizeAttr;
  bool IsLocalMemSizeAttr = false;
  for (unsigned I = 0, E = FD->getNumParams(); I != E; ++I) {
    const ParmVarDecl *parm = FD->getParamDecl(I);

    auto *LocalMemSizeAttr = parm->getAttr<OpenCLLocalMemSizeAttr>();
    if (LocalMemSizeAttr)
      IsLocalMemSizeAttr = true;
    ArgLocalMemSizeAttr.push_back(llvm::ConstantAsMetadata::get(
        (LocalMemSizeAttr)
            ? Builder.getInt32(LocalMemSizeAttr->getLocalMemSize())
            : Builder.getInt32(0)));
  }

  if (IsLocalMemSizeAttr)
    Fn->setMetadata("local_mem_size",
                    llvm::MDNode::get(Context, ArgLocalMemSizeAttr));

  if (const auto *MCA = FD->getAttr<MaxConcurrencyAttr>()) {
    llvm::APSInt MCAInt = MCA->getMax()->EvaluateKnownConstInt(getContext());
    Fn->setMetadata("max_concurrency",
                    llvm::MDNode::get(Context, llvm::ConstantAsMetadata::get(
                                                   Builder.getInt(MCAInt))));
  }

  if (FD->hasAttr<StallFreeAttr>()) {
    llvm::Metadata *attrMDArgs[] = {
        llvm::ConstantAsMetadata::get(Builder.getTrue())};
    Fn->setMetadata("stall_free", llvm::MDNode::get(Context, attrMDArgs));
  }

  if (const auto *A = FD->getAttr<SchedulerPipeliningEffortPctAttr>()) {
    llvm::APSInt SPEPInt =
        A->getSchedulerPipeliningEffortPct()->EvaluateKnownConstInt(
            getContext());
    Fn->setMetadata("scheduler_pipelining_effort_pct",
                    llvm::MDNode::get(Context, llvm::ConstantAsMetadata::get(
                                                   Builder.getInt(SPEPInt))));
  }
}
