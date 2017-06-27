//==---- SPIRMaterializer.cpp - SPIR 1.2 materializer ----------*- C++ -*---=
//
// Copyright (C) 2012-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------===
/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "SPIRMaterializer.h"

#include "NameMangleAPI.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/GlobalStatus.h"

#include <algorithm>

using namespace llvm;

namespace intel {

static void updateMetadata(llvm::Module &M) {
  llvm::NamedMDNode *Kernels = M.getNamedMetadata("opencl.kernels");
  if (!Kernels) return;
  for (auto *Kernel : Kernels->operands()) {
    auto I = Kernel->op_begin();
    auto Func = llvm::mdconst::dyn_extract<llvm::Function>(*I);
    for (++I; I != Kernel->op_end(); ++I) {
      auto *KernelAttr = cast<MDNode>(*I);
      auto AttrIt = KernelAttr->op_begin();
      MDString *AttrName = cast<MDString>(*AttrIt);
      std::vector<Metadata*> Operands;
      for (++AttrIt; AttrIt != KernelAttr->op_end(); ++AttrIt)
        Operands.push_back(*AttrIt);
      if (AttrName->getString() == "vec_type_hint" ||
          AttrName->getString() == "work_group_size_hint" ||
          AttrName->getString() == "reqd_work_group_size" ||
          AttrName->getString() == "max_work_group_size" ||
          AttrName->getString() == "intel_reqd_sub_group_size" ||
          AttrName->getString() == "kernel_arg_addr_space" ||
          AttrName->getString() == "kernel_arg_access_qual" ||
          AttrName->getString() == "kernel_arg_type" ||
          AttrName->getString() == "kernel_arg_base_type" ||
          AttrName->getString() == "kernel_arg_name" ||
          AttrName->getString() == "kernel_arg_type_qual")
        Func->setMetadata(AttrName->getString(),
                          llvm::MDNode::get(M.getContext(), Operands));
    }
  }
}

static std::string updateImageTypeName(StringRef Name, StringRef Acc) {
  std::string AccessQual = Acc.str();
  std::string Res = Name.str();

  assert(Res.find("_t") && "Invalid image type name");
  Res.insert(Res.find("_t") + 1, AccessQual);

  return Res;
}

enum SPIRAddressSpace {
  SPIRAS_Private,
  SPIRAS_Global,
  SPIRAS_Constant,
  SPIRAS_Local,
  SPIRAS_Generic,
  SPIRAS_Count,
};

static PointerType *getOrCreateOpaquePtrType(Module *M,
  const std::string &Name) {
  auto OpaqueType = M->getTypeByName(Name);
  if (!OpaqueType)
    OpaqueType = StructType::create(M->getContext(), Name);
  return PointerType::get(OpaqueType, SPIRAS_Global);
}

#ifndef NDEBUG
static bool isPointerToOpaqueStructType(llvm::Type *Ty) {
  if (auto PT = dyn_cast<PointerType>(Ty))
    if (auto ST = dyn_cast<StructType>(PT->getElementType()))
      if (ST->isOpaque())
        return true;
  return false;
}
#endif // NDEBUG

static reflection::TypePrimitiveEnum getPrimitiveType(Type *T) {
  assert(isPointerToOpaqueStructType(T) && "Invalid type");
  auto Name = T->getPointerElementType()->getStructName();
#define CASE(X, Y) StartsWith("opencl.image" #X, reflection::PRIMITIVE_IMAGE_##Y)
  return StringSwitch<reflection::TypePrimitiveEnum>(Name)
    .CASE(1d_ro_t, 1D_RO_T)
    .CASE(1d_wo_t, 1D_WO_T)
    .CASE(1d_rw_t, 1D_RW_T)
    .CASE(2d_ro_t, 2D_RO_T)
    .CASE(2d_wo_t, 2D_WO_T)
    .CASE(2d_rw_t, 2D_RW_T)
    .CASE(3d_ro_t, 3D_RO_T)
    .CASE(3d_wo_t, 3D_WO_T)
    .CASE(3d_rw_t, 3D_RW_T)
    .CASE(1d_array_ro_t, 1D_ARRAY_RO_T)
    .CASE(1d_array_wo_t, 1D_ARRAY_WO_T)
    .CASE(1d_array_rw_t, 1D_ARRAY_RW_T)
    .CASE(1d_buffer_ro_t, 1D_BUFFER_RO_T)
    .CASE(1d_buffer_wo_t, 1D_BUFFER_WO_T)
    .CASE(1d_buffer_rw_t, 1D_BUFFER_RW_T)
    .CASE(2d_array_depth_ro_t, 2D_ARRAY_DEPTH_RO_T)
    .CASE(2d_array_depth_wo_t, 2D_ARRAY_DEPTH_WO_T)
    .CASE(2d_array_depth_rw_t, 2D_ARRAY_DEPTH_RW_T)
    .CASE(2d_array_depth_ro_t, 2D_ARRAY_DEPTH_RO_T)
    .CASE(2d_array_depth_wo_t, 2D_ARRAY_DEPTH_WO_T)
    .CASE(2d_array_depth_rw_t, 2D_ARRAY_DEPTH_RW_T)
    .CASE(2d_array_ro_t, 2D_ARRAY_RO_T)
    .CASE(2d_array_wo_t, 2D_ARRAY_WO_T)
    .CASE(2d_array_rw_t, 2D_ARRAY_RW_T)
    .CASE(2d_depth_ro_t, 2D_DEPTH_RO_T)
    .CASE(2d_depth_wo_t, 2D_DEPTH_WO_T)
    .CASE(2d_depth_rw_t, 2D_DEPTH_RW_T)
    .Default(reflection::PRIMITIVE_NONE);
}

static void
changeImageCall(llvm::CallInst *CI,
                llvm::SmallVectorImpl<Instruction *> &InstToRemove) {
  auto *F = CI->getCalledFunction();
  if (!F)
    return;

  StringRef FName = F->getName();
  if (!isMangledName(FName.data()))
    return;

  // Update image type names with image access qualifiers
  if (FName.find("image") == std::string::npos)
    return;

  auto FD = demangle(FName.data());
  auto AccQ = StringSwitch<std::string>(FD.name)
                  .Case("write_imagef", "wo_")
                  .Case("write_imagei", "wo_")
                  .Case("write_imageui", "wo_")
                  .Default("ro_");
  auto ImgArg = CI->getArgOperand(0);
  auto ImgArgTy = ImgArg->getType();
  assert(isPointerToOpaqueStructType(ImgArgTy) && "Expect image type argument");
  auto STName = ImgArgTy->getPointerElementType()->getStructName();
  assert(STName.startswith("opencl.image") && "Expect image type argument");
  if (STName.find("_ro_t") != std::string::npos ||
      STName.find("_wo_t") != std::string::npos ||
      STName.find("_rw_t") != std::string::npos)
    return;
  std::vector<Value *> Args;
  std::vector<Type *> ArgTys;
  ArgTys.push_back(getOrCreateOpaquePtrType(CI->getParent()->getModule(),
                                            updateImageTypeName(STName, AccQ)));
  Args.push_back(
      BitCastInst::CreatePointerCast(CI->getArgOperand(0), ArgTys[0], "", CI));
  for (unsigned i = 1; i < CI->getNumArgOperands(); ++i) {
    Args.push_back(CI->getArgOperand(i));
    ArgTys.push_back(CI->getArgOperand(i)->getType());
  }
  auto *FT = FunctionType::get(F->getReturnType(), ArgTys, F->isVarArg());
  dyn_cast<reflection::PrimitiveType>((reflection::ParamType *)FD.parameters[0])
      ->setPrimitive(getPrimitiveType(ArgTys[0]));
  auto NewName = mangle(FD);

  // Check if a new function is already added to the module.
  auto NewF = F->getParent()->getFunction(NewName);
  if (!NewF) {
    // Create function with updated name
    NewF = Function::Create(FT, F->getLinkage(), NewName);
    NewF->copyAttributesFrom(F);

    F->getParent()->getFunctionList().insert(F->getIterator(), NewF);
  }

  CallInst *New = CallInst::Create(NewF, Args, "", CI);
  New->setCallingConv(CI->getCallingConv());
  New->setAttributes(NewF->getAttributes());
  if (CI->isTailCall())
    New->setTailCall();
  New->setDebugLoc(CI->getDebugLoc());

  // Replace old call instruction with updated one
  CI->replaceAllUsesWith(New);
  InstToRemove.push_back(CI);
}

static void changeAddrSpaceCastCall(llvm::CallInst *CI) {
  auto *F = CI->getCalledFunction();
  if (!F)
    return;

  StringRef FName = F->getName();
  if (!isMangledName(FName.data()))
    return;

  // Updates address space qualifier function names with unmangled ones
  if (FName.find("to_global") != std::string::npos ||
      FName.find("to_local") != std::string::npos ||
      FName.find("to_private") != std::string::npos) {
    reflection::FunctionDescriptor FD = demangle(FName.data());
    F->setName("__" + FD.name);
  }
}

// Basic block functors, to be applied on each block in the module.
// 1. Replaces calling conventions in calling sites.
// 2. Translates SPIR 1.2 built-in names to OpenCL CPU RT built-in names.
static void MaterializeBBlock(llvm::BasicBlock &BB) {
  llvm::SmallVector<Instruction *, 4> InstToRemove;

  for (auto &I : BB) {
    if (llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(&I)) {
      changeImageCall(CI, InstToRemove);
      changeAddrSpaceCastCall(CI);
    }
  }

  // Remove unused instructions
  for (auto &I : InstToRemove) {
    assert(I->use_empty() && "Cannot erase used instructions");
    I->eraseFromParent();
  }
}

// Function functor, to be applied for every function in the module.
// 1. Delegates call to basic-block functors.
// 2. Replaces calling conventions of function declarations.
static void MaterializeFunction(llvm::Function &F) {
  std::for_each(F.begin(), F.end(), MaterializeBBlock);
}

int MaterializeSPIR(llvm::Module &M) {
  updateMetadata(M);

  // Take care of calling conventions
  std::for_each(M.begin(), M.end(), MaterializeFunction);

  return 0;
}

}
