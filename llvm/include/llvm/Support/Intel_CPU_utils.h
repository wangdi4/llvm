//===---------------- Intel_CPU_utils.h - CPU utilities -*----------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// Intel CPU Utilities
//===----------------------------------------------------------------------===//
//
#ifndef LLVM_SUPPORT_INTEL_CPU_UTILS_H
#define LLVM_SUPPORT_INTEL_CPU_UTILS_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringSet.h"

namespace llvm {

namespace X86 {
typedef uint64_t ISAVecElementTy;
// Get the feature bit map with the feature strings.
std::array<ISAVecElementTy, 2>
getCpuFeatureBitmap(const ArrayRef<StringRef> CpuFeatures);

// Return true if the cpu feature is valid
bool isCpuFeatureValid(const StringRef CpuFeature);
} // namespace X86

} // namespace llvm

#endif // LLVM_SUPPORT_INTEL_CPU_UTILS_H
