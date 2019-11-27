//===------------------- DTransImmutableAnalysis.h -----------------------===//
//
// Copyright (C) 2019-2019 Intel Corporation. All rights reserved.
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
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"

namespace llvm {

class DTransImmutableInfo {

  struct FieldInfo {
    SmallVector<Constant *, 2> LikelyValues;
    SmallVector<Constant *, 2> LikelyIndirectArrayValues;

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
                     SmallPtrSetImpl<Constant *> &LikelyValues,
                     SmallPtrSetImpl<Constant *> &LikelyIndirectArrayValues);

  /// Returns likely set of constant values for \p FieldNum of struct \p
  /// StructTy. Returns null if no info exists.
  const SmallVectorImpl<llvm::Constant *> *
  getLikelyConstantValues(StructType *StructTy, unsigned FieldNum);

  /// Returns likely set of constant values for indirect array \p FieldNum of
  /// struct \p StructTy. Returns null if no info exists.
  const SmallVectorImpl<llvm::Constant *> *
  getLikelyIndirectArrayConstantValues(StructType *StructTy, unsigned FieldNum);

  /// Results cannot be invalidated.
  bool invalidate(Module &, const PreservedAnalyses &,
                  ModuleAnalysisManager::Invalidator &) {
    return false;
  }

  void print(raw_ostream &OS) const;
};

class DTransImmutableAnalysis
    : public AnalysisInfoMixin<DTransImmutableAnalysis> {
  friend AnalysisInfoMixin<DTransImmutableAnalysis>;
  static AnalysisKey Key;

public:
  typedef DTransImmutableInfo Result;
  Result run(Module &M, ModuleAnalysisManager &AM);
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
