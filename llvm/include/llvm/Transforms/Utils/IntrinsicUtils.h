#if INTEL_COLLAB // -*- C++ -*-
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------- IntrinsicUtils.h - Class definition -*- C++ -*------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// Utilities to support llvm.directive.region.entry/exit intrinsics,
/// which are used to represent compiler directives in the IR.
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORM_UTILS_INTRINSICUTILS_H
#define LLVM_TRANSFORM_UTILS_INTRINSICUTILS_H

#include "llvm/IR/Instructions.h"

namespace llvm {

/// This class provides a set of utility functions that operate on
/// the llvm.directive.region.entery/exit intrinsics
class IntrinsicUtils {

public:
#if INTEL_CUSTOMIZATION
  /// Return a call to the llvm.directive.region.entry() intrinsic for
  /// SIMD.
  static CallInst *createSimdDirectiveBegin(
      Module &M,
      SmallDenseMap<StringRef, SmallVector<Value *, 4>> &DirectiveStrMap);

  /// Return a call to the llvm.directive.region.exit() intrinsic for
  /// SIMD.
  static CallInst *createSimdDirectiveEnd(Module &M, CallInst *DirCall);

  /// Private utility function used to encode a StringRef as a
  /// Metadata Value. This Value is then used by the createDirective*
  /// functions as the parameter representing the type of directive.
  static Value *createMetadataAsValueFromString(Module &M, StringRef Str);

  /// Returns MetadataAsValue corresponding to OpenMP directives.
  static MetadataAsValue *createDirectiveMetadataAsValue(Module &M, int Id) {
    StringRef Str(getDirectiveString(Id));
    return cast<MetadataAsValue>(createMetadataAsValueFromString(M, Str));
  }

  /// Returns MetadataAsValue corresponding to OpenMP clauses.
  static MetadataAsValue *createClauseMetadataAsValue(Module &M, int Id) {
    StringRef Str(getClauseString(Id));
    return cast<MetadataAsValue>(createMetadataAsValueFromString(M, Str));
  }

  /// Check if V is used in OMP private clause inside SIMD region.
  static bool isValueUsedBySimdPrivateClause(const Instruction *I,
                                             const Value *V);
#endif // INTEL_CUSTOMIZATION

  /// Given an enum \p Id for a directive, return its corresponding string
  static StringRef getDirectiveString(int Id);

  /// Given an enum \p Id for a clause, return its corresponding string
  static StringRef getClauseString(int Id);

  /// Return true if the instruction is a directive represented
  /// as a llvm.directive.region.entry/exit intrinsic call
  static bool isDirective(Instruction *I);

  /// Creates a clone of \p CI without any operand bundles that match
  /// by \p Predicate. Replaces all uses of the original \p CI
  /// with the new Instruction created.
  /// \returns newly created CallInst, if it created one, \p CI otherwise
  /// (if no matching bundle on \p CI).
  static CallInst *removeOperandBundlesFromCall(
      CallInst *CI,
      function_ref<bool(const OperandBundleDef &Bundle)> Predicate);

#if INTEL_CUSTOMIZATION
  /// Creates a clone of \p CI without without private clauses for \p V.
  /// Replaces all uses of the original \p CI with the new Instruction created.
  /// \returns the created CallInst, if it created one, \p CI otherwise (when
  /// no private clauses for \p V found).
  static CallInst *removePrivateClauseForValue(CallInst *CI, const Value *V);
#endif // INTEL_CUSTOMIZATION
};

} // namespace llvm

#endif // LLVM_TRANSFORM_UTILS_INTRINSICUTILS_H
#endif // INTEL_COLLAB
