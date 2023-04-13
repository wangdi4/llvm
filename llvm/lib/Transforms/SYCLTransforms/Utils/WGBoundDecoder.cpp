//===- WGBoundDecoder.cpp - Workgroup boundary decoding utilities --*- C++-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/Utils/WGBoundDecoder.h"
#include "llvm/ADT/Twine.h"

namespace llvm {
namespace WGBoundDecoder {

static const StringRef WG_BOUND_PREFIX = "WG.boundaries.";

unsigned getIndexOfInitGidAtDim(unsigned Dim) { return Dim * 2 + 1; }

unsigned getIndexOfSizeAtDim(unsigned Dim) { return Dim * 2 + 2; }

unsigned getUniformIndex() { return 0; }

unsigned getNumWGBoundArrayEntries(unsigned NumDim) { return NumDim * 2 + 1; }

std::string encodeWGBound(StringRef FName) {
  return (Twine(WG_BOUND_PREFIX) + Twine(FName)).str();
}

bool isWGBoundFunction(StringRef FName) {
  return FName.startswith(WG_BOUND_PREFIX);
}

} // namespace WGBoundDecoder
} // namespace llvm
