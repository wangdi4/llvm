//===- SYCLAliasAnalysis.h - addrspace based alias analysis ----*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//
//
// SYCLAliasAnalysis pass provides alias result based on the fact that pointers
// in different address spaces are not aliased.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_SYCLALIASANALYSIS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_SYCLALIASANALYSIS_H

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/ValueHandle.h"

namespace llvm {

/// An AAResult providing alias queries.
class SYCLAAResult : public AAResultBase {

public:
  SYCLAAResult();

  /// Handle invalidation events in the new pass manager.
  bool invalidate(Function &F, const PreservedAnalyses &PA,
                  FunctionAnalysisManager::Invalidator &Inv);

  AliasResult alias(const MemoryLocation &LocA, const MemoryLocation &LocB,
                    AAQueryInfo &, const Instruction *CtxI = nullptr);

  ModRefInfo getModRefInfoMask(const MemoryLocation &Loc, AAQueryInfo &AAQI,
                              bool IgnoreLocals = false);

  void deleteValue(Value *V);
  void copyValue(Value *From, Value *To);
  void addEscapingUse(Use &U);

private:
  // AACallbackVH - A CallbackVH to arrange for SYCLAliasAnalysis to be
  // notified whenever a Value is deleted.
  class AACallbackVH final : public CallbackVH {
    SYCLAAResult *AAR;
    void deleted() override;
    void allUsesReplacedWith(Value *New) override;

  public:
    AACallbackVH(Value *V, SYCLAAResult *AAR = nullptr);
  };

  // Helper class to hold the result of address space resolution
  class ResolveResult {
  public:
    ResolveResult() = delete;
    ResolveResult(const ResolveResult &) = default;
    ResolveResult(bool Resolved, unsigned int AS)
        : Resolved(Resolved), AS(AS) {}

    bool isResolved() { return Resolved; }
    unsigned int getAddressSpace() { return AS; }
    bool operator==(const ResolveResult &Other) {
      return (Resolved == Other.Resolved) && (AS == Other.AS);
    }

  private:
    bool Resolved;
    unsigned int AS;
  };

  /// typedef for ValueMap.
  typedef DenseMap<AACallbackVH, ResolveResult, DenseMapInfo<Value *>>
      ValueMapType;

  /// Cache of the values we have analyzed so far.
  ValueMapType ValueMap;

  /// Go over used values and usages and loop for a cast to a named address
  /// space. If there are no conversions from/to int and only one namespace
  /// different from default (__private) is found resolve all values found
  /// on the way to this address space.
  /// 1st arg: pointer value to resolve
  /// 2nd arg: if true force the resolving once more instead of using
  ///          cached results.
  ResolveResult resolveAddressSpace(const Value *V, bool Force);

  typedef SmallPtrSet<const Value *, 16> SmallValueSet;
  ResolveResult cacheResult(SmallValueSet &Values, ResolveResult RR);

  void rauwValue(Value *OldVal, Value *NewVal);

  int DisjointASs;
};

/// Analysis pass that provides alias result. The result is never invalidated.
class SYCLAliasAnalysis : public AnalysisInfoMixin<SYCLAliasAnalysis> {
  friend AnalysisInfoMixin<SYCLAliasAnalysis>;
  static AnalysisKey Key;

public:
  using Result = SYCLAAResult;

  Result run(Function &, FunctionAnalysisManager &);
};

} // namespace llvm
#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_SYCLALIASANALYSIS_H
