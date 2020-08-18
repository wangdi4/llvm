//===------------------- DTransImmutableAnalysis.h -----------------------===//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This stores 'immutable' part of DTransAnalysis pass.
//
//===----------------------------------------------------------------------===//

#if !INTEL_INCLUDE_DTRANS
#error DTransImmutableAnalysis.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef LLVM_DTRANS_ANALYSIS_DTRANSIMMUTABLE_H
#define LLVM_DTRANS_ANALYSIS_DTRANSIMMUTABLE_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {

class DTransImmutableInfo {

  struct FieldInfo {
    SmallVector<Constant *, 2> LikelyValues;
    SmallVector<Constant *, 2> LikelyIndirectArrayValues;

    // If the current field is an array with constant integers, then store the
    // entries (pair.first) and its constant values (pair.second).
    SmallVector<std::pair<Constant *, Constant*>, 2> ConstantEntriesInArray;

    FieldInfo() {}
  };

  struct StructInfo {
    SmallVector<FieldInfo, 16> Fields;

    StructInfo(StructType *StructTy) {
      assert(StructTy && "StructTy is nullptr");
      Fields.resize(StructTy->getNumElements());
    }
  };

  DenseMap<StructType *, StructInfo *> StructInfoMap;

public:
  DTransImmutableInfo() {}

  ~DTransImmutableInfo() {
    for (auto &Info : StructInfoMap) {
      delete Info.second;
    }
  }

  /// Adds likely constant values for field \p FieldNum of struct \p StructTy.
  void
  addStructFieldInfo(StructType *StructTy, unsigned FieldNum,
      const SetVector<Constant *> &LikelyValues,
      const SetVector<Constant *> &LikelyIndirectArrayValues,
      const SetVector< std::pair<Constant*, Constant*> > &ConsEntriesInArray);

  /// Returns likely set of constant values for \p FieldNum of struct \p
  /// StructTy. Returns null if no info exists.
  const SmallVectorImpl<llvm::Constant *> *
  getLikelyConstantValues(StructType *StructTy, unsigned FieldNum);

  /// Returns likely set of constant values for indirect array \p FieldNum of
  /// struct \p StructTy. Returns null if no info exists.
  const SmallVectorImpl<llvm::Constant *> *
  getLikelyIndirectArrayConstantValues(StructType *StructTy, unsigned FieldNum);

  /// Given the \p FieldNum, which is an array in structure \p StructTy, return
  /// a vector of std::pair that represents the constant entries in this array.
  /// Each pair represents an entry in the array (first) and its constant value
  /// (second).
  const SmallVectorImpl< std::pair<Constant *, Constant*> > *
  getConstantEntriesFromArray(StructType *StructTy, unsigned FieldNum);

  /// Results cannot be invalidated.
  bool invalidate(Module &, const PreservedAnalyses &,
                  ModuleAnalysisManager::Invalidator &) {
    return false;
  }

  /// Results cannot be invalidated.
  bool invalidate(Function &, const PreservedAnalyses &,
                  FunctionAnalysisManager::Invalidator &) {
    return false;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
};

class DTransImmutableAnalysis
    : public AnalysisInfoMixin<DTransImmutableAnalysis> {
  friend AnalysisInfoMixin<DTransImmutableAnalysis>;
  static AnalysisKey Key;

public:
  typedef DTransImmutableInfo Result;
  Result run(Module &M, ModuleAnalysisManager &AM);
  Result run(Function &F, FunctionAnalysisManager &AM);
};

class DTransImmutableAnalysisWrapper : public ImmutablePass {
  DTransImmutableInfo Impl;

public:
  static char ID;

  DTransImmutableAnalysisWrapper();

  DTransImmutableInfo &getResult() { return Impl; }
};

ImmutablePass *createDTransImmutableAnalysisWrapperPass();

} // namespace llvm

#endif
