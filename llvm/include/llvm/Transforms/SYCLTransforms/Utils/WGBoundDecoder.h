//===- WGBoundDecoder.h - Workgroup boundary decoding utilities ----*- C++-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_WG_BOUND_DECODER_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_WG_BOUND_DECODER_H

#include "llvm/ADT/StringRef.h"

namespace llvm {

/// Utilities to decode workgroup bound created by WGLoopBoundaries pass.
namespace WGBoundDecoder {

/// Returns the initial global id index for dimension Dim in the workgroup
/// boundaries array.
/// \param Dim dimension to query.
unsigned getIndexOfInitGidAtDim(unsigned Dim);

/// Returns the initial loop size index for dimension Dim in the workgroup
/// boundaries array.
/// \param Dim dimension to query.
unsigned getIndexOfSizeAtDim(unsigned Dim);

/// Returns the uniform early exit condition in the workgroup boundaries array.
unsigned getUniformIndex();

/// Returns the number of entries in the workgroup boundaries array.
/// \param NumDim number of dimensions.
unsigned getNumWGBoundArrayEntries(unsigned NumDim);

/// Returns the workgroup boundaries function name for the given function name.
/// \param FName name of kernel to get workgroup boundaries function name for.
std::string encodeWGBound(StringRef FName);

/// Returns true if this is a workgroup boundaries function name.
/// FName name to query.
bool isWGBoundFunction(StringRef Name);

} // namespace WGBoundDecoder
} // namespace llvm

#endif
