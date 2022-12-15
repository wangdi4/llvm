//===- IntelVPAssumptionCache.h ---------------------------------*- C++ -*-===//
//
//   Copyright (C) 2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VP_ASSUMPTION_CACHE_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VP_ASSUMPTION_CACHE_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/AssumptionCache.h"

namespace llvm {
class AssumeInst;
class AssumptionCache;
class Value;

namespace vpo {
class VPCallInstruction;
class VPValue;
class VPValueMapper;

/// A VPValue-based cache of \@llvm.assume calls within the VPlan, similar to
/// LLVM's AssumptionCache.
///
/// This cache mirrors the implementation of AssumptionCache in VPlan IR,
/// allowing for similar functionality without relying on the underlying
/// LLVM-IR. The intent is to allow for look up of LLVM assumptions using
/// VPValues without needing an underlying LLVM Value, which may not be present.
///
/// NOTE: this is just a wrapper around AssumptionCache for now.
class VPAssumptionCache {
public:
  VPAssumptionCache(AssumptionCache &AC) : AssumptionCacheLLVM(AC) {}

  /// Clone this cache.
  VPAssumptionCache clone() const { return VPAssumptionCache(*getLLVMCache()); }

  /// Get the inner LLVM cache.
  AssumptionCache *getLLVMCache() const { return &AssumptionCacheLLVM; }

private:
  /// A handle to the inner LLVM cache -- should only be used in the LLVM-IR
  /// path, or when importing assumptions in the front-end.
  AssumptionCache &AssumptionCacheLLVM;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VP_ASSUMPTION_CACHE_H
