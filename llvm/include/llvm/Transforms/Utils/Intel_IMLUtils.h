//===-------- Intel_IMFUtils.h - Utilites for Intel Math Libraries --------===//
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
/// This file includes utility functions for building Intel Math Libraries
/// (IML), and lowering mathematical operations in applications to IML calls.
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_UTILS_INTEL_IMLUTILS_H
#define LLVM_TRANSFORMS_UTILS_INTEL_IMLUTILS_H

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Instructions.h"

namespace llvm {

class FunctionType;
class VectorType;

/// Determine whether \p CC is one of the SVML calling conventions.
bool isSVMLCallingConv(CallingConv::ID CC);

/// Returns the vector type on which a C/C++ SVML function works based on its
/// signature. For example, for <4 x double> (<4 x double>) it should return
/// <4 x double>. Returns nullptr if FT is not a valid SVML function type.
VectorType *getVectorTypeForCSVMLFunction(FunctionType *FT);

/// Determine which variant of SVML calling convention a function should use
/// according to its name and type. Returns std::nullopt if a variant can't
/// be chosen with the given input.
std::optional<CallingConv::ID>
getSVMLCallingConvByNameAndType(StringRef FnName, FunctionType *FT);

/// Convert a unified SVML calling convention to its corresponding variant among
/// legacy C/C++ SVML calling conventions. Used for calling functions from
/// ICC-built SVML library. This function should be removed along with legacy
/// SVML calling conventions, after we switch to new Xmain-built SVML libraries.
CallingConv::ID getLegacyCSVMLCallingConvFromUnified(CallingConv::ID);

/// Returns true if the function should use intel_features_init_cc.
bool shouldUseIntelFeaturesInitCallConv(StringRef FuncName);

/// Extract all IMF attributes of a call site. Returns an AttrBuilder because
/// it's meant to be temporary.
AttrBuilder getIMFAttributes(CallInst *CI);
}

#endif // LLVM_TRANSFORMS_UTILS_INTEL_IMLUTILS_H
