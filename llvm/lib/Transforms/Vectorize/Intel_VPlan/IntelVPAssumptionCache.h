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
/// This cache sits on top of an LLVM AssumptionCache, and is intended to be
/// populated in the VPlan Frontend by importing assumptions for external
/// definitions from the underlying AssumptionCache, and registering assumptions
/// interally within the VPlan as they are encountered.
///
/// NOTE: Support for importing external assumptions and unregistering internal
/// assumptions is not yet implemented.
class VPAssumptionCache {
public:
  VPAssumptionCache(AssumptionCache &AC) : AssumptionCacheLLVM(AC) {}

  /// Clone this cache.
  VPAssumptionCache clone(const VPValueMapper &Mapper) const;

  /// Get the inner LLVM cache.
  AssumptionCache *getLLVMCache() const { return &AssumptionCacheLLVM; }

  /// An assume may be:
  ///  - AssumeInst (external to the plan), or
  ///  - VPCallInstruction (internal to the plan).
  using AssumeT = PointerUnion<const AssumeInst *, const VPCallInstruction *>;

  /// A result elem is the cache's element type: a pair consisting of a user
  /// (the Assume) and an index into that user to the operand bundle that
  /// affected a given value.
  struct ResultElem {
    AssumeT Assume;
    unsigned Index;
  };

  /// Retrieve the list of assumptions which affect this \p Value, if any exist.
  MutableArrayRef<ResultElem> assumptionsFor(const VPValue *Value) {
    auto It = AffectedValues.find(Value);
    if (It == AffectedValues.end())
      return MutableArrayRef<ResultElem>{};
    return It->second;
  }

  /// Return all assumptions which have been registered in the cache.
  MutableArrayRef<ResultElem> assumptions() { return Assumes; }

  /// Register the given \p Assume in the cache.
  void registerAssumption(const VPCallInstruction &Assume);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

private:
  /// If not yet cached, insert this \p Assume and \p Index into the list of
  /// cached assumes and add \p Val as an affected value.
  void insertAssume(const VPValue *Val, AssumeT Assume, unsigned Index);

private:
  /// A map of VPValues back to the assumes that affect them.
  DenseMap<const VPValue *, SmallVector<ResultElem, 1>> AffectedValues;

  /// The total list of assumptions registered in the cache.
  SmallVector<ResultElem, 2> Assumes;

  /// A handle to the inner LLVM cache -- should only be used in the LLVM-IR
  /// path, or when importing assumptions in the front-end.
  AssumptionCache &AssumptionCacheLLVM;
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
raw_ostream &operator<<(llvm::raw_ostream &OS,
                        const VPAssumptionCache::AssumeT &Assume);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

namespace detail {
template <typename To> struct ResultElemCastInfo {
  using From = llvm::vpo::VPAssumptionCache::ResultElem;

  static bool isPossible(const From &Val) {
    return Val.Assume.template is<const To *>();
  }
  static const To *doCast(const From &Val) {
    return Val.Assume.template get<const To *>();
  }
  static const To *doCastIfPossible(const From &Val) {
    return Val.Assume.template dyn_cast<const To *>();
  }
  static const To *castFailed() { return nullptr; }
};
} // namespace detail
} // namespace vpo

// Machinery (CastInfo specialization) to allow for 'isa/cast/dyn_cast' to Value
// and VPValue from ResultElem, e.g:
//
//   const AssumeInst *Assume =
//      dyn_cast<AssumeInst>(AC->assumptionsFor(V).front());
//
//   const VPCallInstruction *VPAssume =
//      cast<VPCallInstruction>(AC->assumptions().front());
//
template <typename To>
struct CastInfo<To, const llvm::vpo::VPAssumptionCache::ResultElem,
                std::enable_if_t<std::disjunction<
                    std::is_same<llvm::AssumeInst, To>,
                    std::is_same<llvm::vpo::VPCallInstruction, To>>::value>>
    : vpo::detail::ResultElemCastInfo<To> {};

template <typename To>
struct CastInfo<To, llvm::vpo::VPAssumptionCache::ResultElem,
                std::enable_if_t<std::disjunction<
                    std::is_same<llvm::AssumeInst, To>,
                    std::is_same<llvm::vpo::VPCallInstruction, To>>::value>>
    : vpo::detail::ResultElemCastInfo<To> {};
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VP_ASSUMPTION_CACHE_H
