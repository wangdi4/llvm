//===- TypeAlignment.h - Type alignment utilities----------------*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_TYPE_ALIGNMRENT_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_TYPE_ALIGNMRENT_H

#include "llvm/Transforms/SYCLTransforms/KernelArgType.h"

namespace llvm {

/// Provide alignment and size information for KernelArgument and general
/// alignment utilities (aligning offsets and pointers).
class TypeAlignment {
public:
  /// Returns the size of the given argument.
  /// \param arg the argument for which to return its size.
  static size_t getSize(const KernelArgument &arg);

  /// Returns the alignment of the given argument.
  /// \param arg the argument for which to return its alignment.
  static size_t getAlignment(const KernelArgument &arg);

  /// Returns offest aligned based on the given alignment.
  /// \param alignment the alignment.
  /// \param offset the offset to align.
  static size_t align(size_t alignment, size_t offset);

  /// Returns pointer aligned based on the given alignment.
  /// \param alignment the alignment.
  /// \param pointer the pointer to align.
  static char *align(size_t alignment, const char *ptr);

public:
  // Represents the maximum alignment.
  static const size_t MAX_ALIGNMENT = DEV_MAXIMUM_ALIGN;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_TYPE_ALIGNMRENT_H
