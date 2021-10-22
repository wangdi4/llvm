//===------- Intel_IMFUtils.cpp - Utilites for Intel Math Libraries -------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// Implementation of IML utilities.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Utils/Intel_IMLUtils.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;

bool llvm::isSVMLCallingConv(CallingConv::ID CC) {
  return CC == CallingConv::SVML || CC == CallingConv::SVML_AVX ||
         CC == CallingConv::SVML_AVX512 || CC == CallingConv::SVML_Unified ||
         CC == CallingConv::SVML_Unified_256 ||
         CC == CallingConv::SVML_Unified_512;
}

VectorType *llvm::getVectorTypeForCSVMLFunction(FunctionType *FT) {
  Type *CallRetType = FT->getReturnType();
  auto *RetStructTy = dyn_cast_or_null<StructType>(CallRetType);
  // For structure return types, return the first structure element as
  // a vector type.
  if (RetStructTy && RetStructTy->getNumElements())
    return dyn_cast<VectorType>(RetStructTy->getElementType(0));
  else
    return dyn_cast<VectorType>(CallRetType);
}

static CallingConv::ID getCSVMLCallingConvByVectorSize(unsigned Size) {
  assert(isPowerOf2_32(Size) && Size <= 512 && "Invalid vector size");
  if (Size <= 128)
    return CallingConv::SVML_Unified;
  else if (Size == 256)
    return CallingConv::SVML_Unified_256;
  else if (Size == 512)
    return CallingConv::SVML_Unified_512;
  else
    llvm_unreachable("Invalid vector size");
}

// Function names in SVML for OCLRT follows a pattern like: __ocl_svml_l9_powr2
// The target ID we need to match is the 12th and 13th byte in the name.
constexpr const size_t OCL_TARGET_ID_START_POS = 11;
constexpr const size_t OCL_TARGET_ID_LENGTH = 2;
constexpr const size_t OCL_TARGET_ID_END_POS =
    OCL_TARGET_ID_START_POS + OCL_TARGET_ID_LENGTH;

/// Determine which variant of an SVML calling convention a function should use
/// according to its name and type. Because OCLRT uses JIT, the exact target
/// hardware is always known before compilation starts. So in SVML for OCLRT,
/// every function has a designated target without dynamic dispatch, which is
/// indicated by the function name, and the list of callee-saved registers is
/// determined by the maximum vector length supported by that target, not the
/// type of argument and return values.
static CallingConv::ID getOCLSVMLCallingConvByName(StringRef FnName) {
  // Mapping between target IDs and corresponding calling conventions
  static const llvm::StringMap<CallingConv::ID> TargetIDToCallConvMap({
      // IA32
      {"v8", CallingConv::SVML_Unified},
      {"n8", CallingConv::SVML_Unified},
      {"g9", CallingConv::SVML_Unified_256},
      {"s9", CallingConv::SVML_Unified_256},
      {"x0", CallingConv::SVML_Unified_512},
      {"x1", CallingConv::SVML_Unified_512},

      // Intel 64
      {"u8", CallingConv::SVML_Unified},
      {"h8", CallingConv::SVML_Unified},
      {"e9", CallingConv::SVML_Unified_256},
      {"l9", CallingConv::SVML_Unified_256},
      {"z0", CallingConv::SVML_Unified_512},
      {"z1", CallingConv::SVML_Unified_512},

      // KNL
      // TODO: KNL support has been discontinued in 2022.0. This entry only
      // exists for transition. After library build migration is done we should
      // remove the KNL part of SVML, as well as this entry.
      {"b3", CallingConv::SVML_Unified_512},
  });

  assert(FnName.startswith("__ocl_svml_") &&
         FnName.size() > OCL_TARGET_ID_END_POS &&
         FnName[OCL_TARGET_ID_END_POS] == '_' &&
         "Ivalid function name for OpenCL SVML function");

  StringRef TargetID =
      FnName.substr(OCL_TARGET_ID_START_POS, OCL_TARGET_ID_LENGTH);
  auto I = TargetIDToCallConvMap.find(TargetID);
  assert(I != TargetIDToCallConvMap.end() &&
         "Invalid target ID for OpenCL SVML function");
  return I->second;
}

CallingConv::ID llvm::getSVMLCallingConvByNameAndType(StringRef FnName,
                                                      FunctionType *FT) {
  if (FnName.startswith("__svml_"))
    return getCSVMLCallingConvByVectorSize(
        getVectorTypeForCSVMLFunction(FT)->getPrimitiveSizeInBits());

  if (FnName.startswith("__ocl_svml_") &&
      FnName.size() > OCL_TARGET_ID_END_POS &&
      FnName[OCL_TARGET_ID_END_POS] == '_')
    return getOCLSVMLCallingConvByName(FnName);

  llvm_unreachable("Invalid function name for SVML function");
}

CallingConv::ID llvm::getLegacyCSVMLCallingConvFromUnified(CallingConv::ID CC) {
  switch (CC) {
    case CallingConv::SVML_Unified:
      return CallingConv::SVML;
    case CallingConv::SVML_Unified_256:
      return CallingConv::SVML_AVX;
    case CallingConv::SVML_Unified_512:
      return CallingConv::SVML_AVX512;
    default:
      llvm_unreachable("Expect one of unified SVML calling conventions");
  }
}
