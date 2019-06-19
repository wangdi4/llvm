#if INTEL_COLLAB // -*- C++ -*-
//===----------- IntrinsicUtils.h - Class definition -*- C++ -*------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// Set of utilities that operate on intrinsics introduced by using OpenMP and
/// SIMD directives. They are also used by auto parallelization/vectorization
/// to generate parallel/vector code.
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORM_UTILS_INTRINSICUTILS_H
#define LLVM_TRANSFORM_UTILS_INTRINSICUTILS_H

namespace llvm {

/// \brief This class provides a set of utility functions that operate on
/// intrinsics introduced by OpenMP/SIMD directives.
class IntrinsicUtils {

public:
#if INTEL_CUSTOMIZATION
  /// \brief Return a call to the llvm.directive.region.entry() intrinsic for
  /// SIMD.
  static CallInst *createSimdDirectiveBegin(
      Module &M,
      SmallDenseMap<StringRef, SmallVector<Value *, 4>> &DirectiveStrMap);

  /// \brief Return a call to the llvm.directive.region.exit() intrinsic for
  /// SIMD.
  static CallInst *createSimdDirectiveEnd(Module &M, CallInst *DirCall);

  /// \brief Private utility function used to encode a StringRef as a
  /// Metadata Value. This Value is then used by the createDirective*
  /// functions as the parameter representing the type of directive.
  static Value *createMetadataAsValueFromString(Module &M, StringRef Str);

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
#endif // INTEL_CUSTOMIZATION

  /// \brief Returns strings corresponding to OpenMP directives.
  static StringRef getDirectiveString(int Id);

  /// \brief Returns strings corresponding to OpenMP clauses.
  static StringRef getClauseString(int Id);

  /// \brief Return true if the instruction is an OpenMP directive
  /// represented as a directive.region.entry/exit intrinsic call
  static bool isOpenMPDirective(Instruction *I);
};

} // namespace llvm

#endif // LLVM_TRANSFORM_UTILS_INTRINSICUTILS_H
#endif // INTEL_COLLAB
