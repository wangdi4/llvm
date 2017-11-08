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

  // ComponentMD contains {name, return type, argument metadata...}
  SmallVector<llvm::Metadata *, 10> ComponentMD;
  // AttrMD contains the function-level attributes.
  SmallVector<llvm::Metadata *, 10> AttrMD;

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

  for (const ParmVarDecl *PVD : FD->parameters()) {
    const Type *ParamType = PVD->getType().getCanonicalType().getTypePtr();

    std::pair<StringRef, StringRef> ArgType = {"arg_type", "default"};
    std::pair<StringRef, StringRef> ImplType = {"impl_type", "wire"};
    std::pair<StringRef, int> Stable = {"stable", 0};
    std::pair<StringRef, StringRef> CosimName = {"cosim_name", PVD->getName()};

    if (const auto *AIA = PVD->getAttr<ArgumentInterfaceAttr>())
      ImplType.second =
          ArgumentInterfaceAttr::ConvertArgumentInterfaceTypeToStr(
              AIA->getType());

    if (PVD->hasAttr<StableArgumentAttr>())
      Stable.second = 1;

    if (PVD->getAttr<SlaveMemoryArgumentAttr>())
      ArgType.second = "mm_slave";
    else if (ParamType->isPointerType() || ParamType->isReferenceType())
      ArgType.second = "pointer";

    SmallVector<llvm::Metadata *, 10> MDs = {
        llvm::MDString::get(Ctx, ArgType.first),
        llvm::MDString::get(Ctx, ArgType.second),
        llvm::MDString::get(Ctx, ImplType.first),
        llvm::MDString::get(Ctx, ImplType.second),
        llvm::MDString::get(Ctx, Stable.first),
        llvm::ConstantAsMetadata::get(
            llvm::ConstantInt::get(Int32Ty, Stable.second)),
        llvm::MDString::get(Ctx, CosimName.first),
        llvm::MDString::get(Ctx, CosimName.second)};
    if (const auto *LMSA = PVD->getAttr<OpenCLLocalMemSizeAttr>()) {
      MDs.push_back(llvm::MDString::get(Ctx, "local_mem_size"));
      MDs.push_back(llvm::ConstantAsMetadata::get(
          llvm::ConstantInt::get(Int32Ty, LMSA->getLocalMemSize())));
    }
    ComponentMD.push_back(llvm::MDNode::get(Ctx, MDs));
  }

  AttrMD.push_back(llvm::MDString::get(Ctx, "cosim_name"));
  AttrMD.push_back(llvm::MDString::get(Ctx, Fn->getName()));

  AttrMD.push_back(llvm::MDString::get(Ctx, "component_interface"));
  const auto *CIA = FD->getAttr<ComponentInterfaceAttr>();
  StringRef CompIntString =
      ComponentInterfaceAttr::ConvertComponentInterfaceTypeToStr(
          CIA->getType());
  AttrMD.push_back(llvm::MDString::get(Ctx, CompIntString));

  AttrMD.push_back(llvm::MDString::get(Ctx, "stall_free_return"));
  int IsStallFreeReturn = FD->hasAttr<StallFreeReturnAttr>() ? 1 : 0;
  AttrMD.push_back(llvm::ConstantAsMetadata::get(
      llvm::ConstantInt::get(Int32Ty, IsStallFreeReturn)));

  if (const auto *MCA = FD->getAttr<MaxConcurrencyAttr>()) {
    AttrMD.push_back(llvm::MDString::get(Ctx, "max_concurrency"));
    AttrMD.push_back(llvm::ConstantAsMetadata::get(
        llvm::ConstantInt::get(Int32Ty, MCA->getMax())));
  }

  Fn->setMetadata("ihc_component", llvm::MDNode::get(Ctx, ComponentMD));
  Fn->setMetadata("ihc_attrs", llvm::MDNode::get(Ctx, AttrMD));
}
