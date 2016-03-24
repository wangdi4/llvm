//===-------- Intel_IntrinsicUtils.h - Class definition -*- C++ -*---------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// Set of utilities that operate on intrinsics introduced by using OpenMP and
/// SIMD directives. They are also used by auto parallelization/vectorization
/// to generate parallel/vector code.
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORM_UTILS_INTEL_INTRINSICUTILS_H
#define LLVM_TRANSFORM_UTILS_INTEL_INTRINSICUTILS_H

namespace llvm {

/// \brief This class provides a set of utility functions that operate on
/// intrinsics introduced by OpenMP/SIMD directives.
class IntelIntrinsicUtils {

public:

  /// \brief Private utility function used to encode a StringRef as a
  /// Metadata Value. This Value is then used by the createDirective*
  /// functions as the parameter representing the type of directive.
  static Value* createMetadataAsValueFromString(Module &M, StringRef Str);

  /// \brief Return a call to the llvm.intel.directive intrinsic.
  static CallInst* createDirectiveCall(Module &M, StringRef DirectiveStr);

  /// \brief Return a call to the llvm.intel.directive.qual intrinsic.
  static CallInst* createDirectiveQualCall(Module &M, StringRef DirectiveStr);

  /// \brief Return a call to the llvm.intel.directive.qual.opnd intrinsic.
  static CallInst* createDirectiveQualOpndCall(Module &M,
                                               StringRef DirectiveStr,
                                               Value *Val);

  /// \brief Return a call to the llvm.intel.directive.qual.opndlist
  /// intrinsic.
  static CallInst* createDirectiveQualOpndListCall(
      Module &M,
      StringRef DirectiveStr,
      SmallVector<Value*, 4>& VarCallArgs);

  /// \brief Returns strings corresponding to OpenMP directives.
  static StringRef getDirectiveString(int Id);

  /// \brief Returns strings corresponding to OpenMP clauses.
  static StringRef getClauseString(int Id);

  /// \brief Returns MetadataAsValue corresponding to OpenMP directives.
  static MetadataAsValue *createDirectiveMetadataAsValue(Module &M, int Id) {
    StringRef Str(getDirectiveString(Id));
    return cast<MetadataAsValue>(createMetadataAsValueFromString(M, Str));
  }

  /// \brief Returns MetadataAsValue corresponding to OpenMP clauses.
  static MetadataAsValue *createClauseMetadataAsValue(Module &M, int Id) {
    StringRef Str(getClauseString(Id));
    return cast<MetadataAsValue>(createMetadataAsValueFromString(M, Str));
  }
};

} // end llvm namespace

#endif // LLVM_TRANSFORM_UTILS_INTEL_INTRINSICUTILS_H
