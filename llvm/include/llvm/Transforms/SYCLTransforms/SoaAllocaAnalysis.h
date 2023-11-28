//===- SoaAllocaAnalysis.h - SOA alloca analysis ----------------*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_SOA_ALLOCA_ANALYSIS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_SOA_ALLOCA_ANALYSIS_H

#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class AllocaInst;

/// SoaAllocaInfo provides values depending on alloca instruction that can be
/// converted to SOA alloca.
class SoaAllocaInfo {
public:
  explicit SoaAllocaInfo(const Function &F);

  void print(raw_ostream &OS) const;

  /// Return true if given value is derived from SOA alloca.
  /// \param V input value.
  bool isSoaAllocaRelated(const Value *V) const;

  /// Return true if given value is derived from SOA alloca with scalar base
  /// type.
  /// \param V input value.
  bool isSoaAllocaScalarRelated(const Value *V) const;

  /// Return true if given value is derived from SOA alloca with vector base
  /// type.
  /// \param V input value.
  bool isSoaAllocaVectorRelated(const Value *V) const;

  /// Return true if given value is Alloca or GEP instructions that are
  /// derived from SOA-alloca instruction.
  /// \param V input value.
  bool isSoaAllocaRelatedPointer(const Value *V) const;

  /// Return width of given value. Assumed it is derived from SOA alloca
  /// with vector base type.
  /// \param V input value.
  unsigned getSoaAllocaVectorWidth(const Value *V) const;

private:
  //// Find all values that are derived from SOA alloca.
  void compute();

  /// Return true if given alloca instruction is supported.
  /// \param AI alloca instruction.
  /// \param IsVectorBasedType true iff alloca has vector base type.
  /// \param ArrayNestedLevel number of nested array in alloca type.
  /// \param[out] Visited list of instructions derived from supported alloca.
  bool isSupportedAlloca(const AllocaInst *AI, bool IsVectorBasedType,
                         unsigned ArrayNestedLevel,
                         SmallSet<const Value *, 32> &Visited);

  /// Return true if given CallInst is supported memset.
  /// \param CI - call instruction.
  bool isSupportedMemset(const CallInst *CI);

  /// The function that the SoaAllocaInfo is for.
  const Function &F;

  /// Instrctions derived from SOA alloca are mapped to base vector type length,
  /// 0 for scalar base type.
  DenseMap<const Value *, unsigned> AllocaSOA;
};

/// Analysis pass which detects values which depend on alloca instruction
/// that can be converted to SOA alloca.
class SoaAllocaAnalysis : public AnalysisInfoMixin<SoaAllocaAnalysis> {
  friend AnalysisInfoMixin<SoaAllocaAnalysis>;
  static AnalysisKey Key;

public:
  using Result = SoaAllocaInfo;

  Result run(Function &F, FunctionAnalysisManager &FAM);
};

/// Printer pass for SoaAllocaAnalysis.
class SoaAllocaAnalysisPrinter
    : public PassInfoMixin<SoaAllocaAnalysisPrinter> {
  raw_ostream &OS;

public:
  explicit SoaAllocaAnalysisPrinter(raw_ostream &OS) : OS(OS) {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace llvm
#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_SOA_ALLOCA_ANALYSIS_H
