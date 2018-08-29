//===- SPIRMetadataAdder.cpp - Add SPIR related module scope metadata -----===//
//
// Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "CGSPIRMetadataAdder.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/TypeFinder.h"
#include "llvm/Transforms/IPO.h"
#include <sstream>

using namespace llvm;
using namespace clang;
using namespace CodeGen;

static const char *ImageTypeNames[] = {"opencl.image1d_ro_t",
                                       "opencl.image1d_wo_t",
                                       "opencl.image1d_rw_t",
                                       "opencl.image1d_array_ro_t",
                                       "opencl.image1d_array_wo_t",
                                       "opencl.image1d_array_rw_t",
                                       "opencl.image1d_buffer_ro_t",
                                       "opencl.image1d_buffer_wo_t",
                                       "opencl.image1d_buffer_rw_t",
                                       "opencl.image2d_ro_t",
                                       "opencl.image2d_wo_t",
                                       "opencl.image2d_rw_t",
                                       "opencl.image2d_array_ro_t",
                                       "opencl.image2d_array_wo_t",
                                       "opencl.image2d_array_rw_t",
                                       "opencl.image2d_depth_ro_t",
                                       "opencl.image2d_depth_wo_t",
                                       "opencl.image2d_depth_rw_t",
                                       "opencl.image2d_array_depth_ro_t",
                                       "opencl.image2d_array_depth_wo_t",
                                       "opencl.image2d_array_depth_rw_t",
                                       "opencl.image2d_msaa_ro_t",
                                       "opencl.image2d_msaa_wo_t",
                                       "opencl.image2d_msaa_rw_t",
                                       "opencl.image2d_array_msaa_ro_t",
                                       "opencl.image2d_array_msaa_wo_t",
                                       "opencl.image2d_array_msaa_rw_t",
                                       "opencl.image2d_msaa_depth_ro_t",
                                       "opencl.image2d_msaa_depth_wo_t",
                                       "opencl.image2d_msaa_depth_rw_t",
                                       "opencl.image2d_array_msaa_depth_ro_t",
                                       "opencl.image2d_array_msaa_depth_wo_t",
                                       "opencl.image2d_array_msaa_depth_rw_t",
                                       "opencl.image3d_ro_t",
                                       "opencl.image3d_wo_t",
                                       "opencl.image3d_rw_t"};

static const char *ImageDepthTypeNames[] = {
    "opencl.image2d_depth_ro_t",       "opencl.image2d_depth_wo_t",
    "opencl.image2d_depth_rw_t",       "opencl.image2d_array_depth_ro_t",
    "opencl.image2d_array_depth_wo_t", "opencl.image2d_array_depth_rw_t"};

static const char *ImageMSAATypeNames[] = {
    "opencl.image2d_msaa_ro_t",
    "opencl.image2d_msaa_wo_t",
    "opencl.image2d_msaa_rw_t",
    "opencl.image2d_array_msaa_ro_t",
    "opencl.image2d_array_msaa_wo_t",
    "opencl.image2d_array_msaa_rw_t",
    "opencl.image2d_msaa_depth_ro_t",
    "opencl.image2d_msaa_depth_wo_t",
    "opencl.image2d_msaa_depth_rw_t",
    "opencl.image2d_array_msaa_depth_ro_t",
    "opencl.image2d_array_msaa_depth_wo_t",
    "opencl.image2d_array_msaa_depth_rw_t"};

struct OCLExtensionsTy {
#define OPENCLEXT(nm) unsigned _##nm : 1;
#include "clang/Basic/OpenCLExtensions.def"

OCLExtensionsTy() {
#define OPENCLEXT(nm) _##nm = 0;
#include "clang/Basic/OpenCLExtensions.def"
  }
};

typedef void (*func_call_handler)(CallInst *CI, OCLExtensionsTy &Exts);

void baseAtomics64(CallInst *CI, OCLExtensionsTy &Exts) {
  auto *FirstArgTy = dyn_cast<PointerType>(CI->getArgOperand(0)->getType());

  if (FirstArgTy && FirstArgTy->getPointerElementType()->isIntegerTy() &&
      FirstArgTy->getPointerElementType()->getScalarSizeInBits() == 64)
    Exts._cl_khr_int64_base_atomics = 1;
}

void extAtomics64(CallInst *CI, OCLExtensionsTy &Exts) {
  auto *FirstArgTy = dyn_cast<PointerType>(CI->getArgOperand(0)->getType());

  if (FirstArgTy && FirstArgTy->getPointerElementType()->isIntegerTy() &&
      FirstArgTy->getPointerElementType()->getScalarSizeInBits() == 64)
    Exts._cl_khr_int64_extended_atomics = 1;
}

void image3DWrite(CallInst *CI, OCLExtensionsTy &Exts) {
  auto *FirstArgTy = dyn_cast<PointerType>(CI->getArgOperand(0)->getType());

  if (FirstArgTy && FirstArgTy->getPointerElementType()->isStructTy() &&
      !FirstArgTy->getPointerElementType()->getStructName().compare(
          "opencl.image3d_t"))
    Exts._cl_khr_3d_image_writes = 1;
}

typedef struct {
  const char *FuncName;
  func_call_handler Handler;
} FuncCallHandlersTy;

static const FuncCallHandlersTy FuncCallHandlers[] = {
    {"_Z8atom_add", baseAtomics64},     {"_Z8atom_sub", baseAtomics64},
    {"_Z9atom_xchg", baseAtomics64},    {"_Z8atom_inc", baseAtomics64},
    {"_Z8atom_dec", baseAtomics64},     {"_Z12atom_cmpxchg", baseAtomics64},
    {"_Z8atom_min", extAtomics64},      {"_Z8atom_max", extAtomics64},
    {"_Z8atom_and", extAtomics64},      {"_Z7atom_or", extAtomics64},
    {"_Z8atom_xor", extAtomics64},      {"_Z12write_imagef", image3DWrite},
    {"_Z12write_imagei", image3DWrite}, {"_Z13write_imageui", image3DWrite}};

static bool searchTypeInType(llvm::Type *Ty1, llvm::Type *Ty2, bool IgnorePtrs);

static bool searchTypeInType(llvm::Type *Ty1, llvm::Type *Ty2, bool IgnorePtrs,
                             SmallSet<llvm::Type *, 16> &TypesList) {
  if (Ty1 == Ty2)
    return true;

  if (Ty1->isVectorTy())
    return searchTypeInType(Ty1->getVectorElementType(), Ty2, IgnorePtrs,
                            TypesList);

  if (Ty1->isArrayTy())
    return searchTypeInType(Ty1->getArrayElementType(), Ty2, IgnorePtrs,
                            TypesList);

  if (!IgnorePtrs && Ty1->isPointerTy()) {
    // prevent infinte loop (such as a struct that conatins a pointer to itself)
    if (TypesList.count(Ty1->getPointerElementType()) > 0)
      return false;

    return searchTypeInType(Ty1->getPointerElementType(), Ty2, IgnorePtrs,
                            TypesList);
  }

  if (Ty1->isStructTy()) {
    TypesList.insert(Ty1);
    auto *StrTy = dyn_cast<llvm::StructType>(Ty1);

    for (auto *ElemTy : StrTy->elements())
      if (searchTypeInType(ElemTy, Ty2, IgnorePtrs, TypesList))
        return true;
  }

  if (Ty1->isFunctionTy()) {
    TypesList.insert(Ty1);
    auto *FuncTy = dyn_cast<llvm::FunctionType>(Ty1);

    if (searchTypeInType(FuncTy->getReturnType(), Ty2, IgnorePtrs))
      return true;

    for (auto *PTy : FuncTy->params())
      if (searchTypeInType(PTy, Ty2, IgnorePtrs))
        return true;
  }

  return false;
}

static bool searchTypeInType(llvm::Type *Ty1, llvm::Type *Ty2,
                             bool IgnorePtrs) {
  SmallSet<llvm::Type *, 16> TypesList;
  return searchTypeInType(Ty1, Ty2, IgnorePtrs, TypesList);
}

static void functionAddSPIRMetadata(Function &F, bool &bUseDoubles,
                                    OCLExtensionsTy &sUsedExts);

void clang::CodeGen::addSPIRMetadata(Module &M, int OCLVersion,
                                     std::string SPIROptions) {
  Type *DoubleTy = Type::getDoubleTy(M.getContext());
  Type *HalfTy = Type::getHalfTy(M.getContext());

  OCLExtensionsTy UsedExts;

  bool UseDoubles = false;
  bool UseImages = false;

  for (auto &G : M.globals()) {
    if (searchTypeInType(G.getType(), DoubleTy, false))
      UseDoubles = true;
    if (searchTypeInType(G.getType(), HalfTy, true))
      UsedExts._cl_khr_fp16 = true;
  }

  // check if image types are defined
  for (size_t i = 0;
       i < sizeof(ImageTypeNames) / sizeof(ImageTypeNames[0]); i++) {
    if (M.getTypeByName(ImageTypeNames[i])) {
      UseImages = true;
      break;
    }
  }

  // check if depth image types are defined
  for (size_t i = 0;
       i < sizeof(ImageDepthTypeNames) / sizeof(ImageDepthTypeNames[0]); i++) {
    if (M.getTypeByName(ImageDepthTypeNames[i])) {
      UsedExts._cl_khr_depth_images = true;
      break;
    }
  }

  // check if msaa image types are defined
  for (size_t i = 0;
       i < sizeof(ImageMSAATypeNames) / sizeof(ImageMSAATypeNames[0]); i++) {
    if (M.getTypeByName(ImageMSAATypeNames[i])) {
      UsedExts._cl_khr_gl_msaa_sharing = true;
      break;
    }
  }

  // scan all functions
  for (auto &F : M.functions()) {
    functionAddSPIRMetadata(F, UseDoubles, UsedExts);
  }

  // Add used extensions
  SmallVector<llvm::Metadata *, 5> OCLExtElts;

#define OPENCLEXT(nm)                                                          \
  if (UsedExts._##nm)                                                         \
    OCLExtElts.push_back(llvm::MDString::get(M.getContext(), #nm));
#include "clang/Basic/OpenCLExtensions.def"

  NamedMDNode *OCLExtMD =
      M.getOrInsertNamedMetadata("opencl.used.extensions");

  OCLExtMD->addOperand(MDNode::get(M.getContext(), OCLExtElts));

  // Add used optional core features
  SmallVector<Metadata *, 5> OCLOptCoreElts;

  if (UseDoubles)
    OCLOptCoreElts.push_back(MDString::get(M.getContext(), "cl_doubles"));

  if (UseImages)
    OCLOptCoreElts.push_back(MDString::get(M.getContext(), "cl_images"));

  NamedMDNode *OptCoreMD =
      M.getOrInsertNamedMetadata("opencl.used.optional.core.features");
  OptCoreMD->addOperand(MDNode::get(M.getContext(), OCLOptCoreElts));

  // Add build options
  NamedMDNode *OCLCompOptsMD =
      M.getOrInsertNamedMetadata("opencl.compiler.options");
  SmallVector<llvm::Metadata *, 5> OCLBuildOptions;

  std::stringstream SOpts(SPIROptions);
  std::string SOpt;
  while (SOpts >> SOpt)
    OCLBuildOptions.push_back(MDString::get(M.getContext(), SOpt));
  OCLCompOptsMD->addOperand(MDNode::get(M.getContext(), OCLBuildOptions));
}

static void functionAddSPIRMetadata(Function &F, bool &UseDoubles,
                                    OCLExtensionsTy &UsedExts) {
  Type *DoubleTy = Type::getDoubleTy(F.getParent()->getContext());
  Type *HalfTy = Type::getHalfTy(F.getParent()->getContext());

  for (const auto &Arg : F.args()) {
    if (searchTypeInType(Arg.getType(), DoubleTy, false))
      UseDoubles = true;
    if (searchTypeInType(Arg.getType(), HalfTy, true))
      UsedExts._cl_khr_fp16 = true;
  }

  for (auto &I : instructions(F)) {
    if (searchTypeInType(I.getType(), DoubleTy, false))
      if (!(dyn_cast<FPExtInst>(&I)))
        UseDoubles = true;
    if (searchTypeInType(I.getType(), HalfTy, true))
      UsedExts._cl_khr_fp16 = true;

    for (auto *Op : I.operand_values()){
      if (searchTypeInType(Op->getType(), DoubleTy, false))
        if (!(isa<CallInst>(&I) &&
              cast<CallInst>(&I)->getCalledFunction() &&
              cast<CallInst>(&I)->getCalledFunction()->isVarArg()))
          UseDoubles = true;
      if (searchTypeInType(Op->getType(), HalfTy, true))
        UsedExts._cl_khr_fp16 = true;
    }

    auto *CI = dyn_cast<CallInst>(&I);
    if (CI && CI->getCalledFunction()) {
      StringRef FName = CI->getCalledFunction()->getName();

      for (size_t i = 0;
           i < sizeof(FuncCallHandlers) / sizeof(FuncCallHandlers[0]); i++) {
        if (FName.equals(FuncCallHandlers[i].FuncName))
          FuncCallHandlers[i].Handler(CI, UsedExts);
      }
    }
  }
}
