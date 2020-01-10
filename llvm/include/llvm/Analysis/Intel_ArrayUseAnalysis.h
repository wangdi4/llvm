//===- Intel_ArrayUseAnalysis.h - Array Usage Analysis ----------*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This analysis is a specialized analysis that determines what indexes of an
// array may be accessed by subsequent portions of the code.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_ARRAYUSEANALYSIS_H
#define LLVM_ANALYSIS_INTEL_ARRAYUSEANALYSIS_H

#include "llvm/IR/PassManager.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Pass.h"

namespace llvm {
class ArrayUse;
class Instruction;
class LoopInfo;
class ScalarEvolution;
class SCEV;
class Value;

/// ArrayRangeInfo represents an inclusive range of values. Unlike the
/// ConstantRange class, the high endpoint is included in the range.
/// Additionally, the possible values are represented as SCEV values instead of
/// constants.
///
/// The range values are not represented with full precision: they are limited
/// to values of the kind [a, b]. If a computation cannot be represented as a
/// single range, then the returned result would be the smallest range that
/// contains both ranges.
class ArrayRangeInfo final {
  const SCEV *Low, *High;
  /// The kind of the range: EMPTY refers to one that is completely empty,
  /// FULL to one that is completely full, and RANGE is to something that is in
  /// between.
  enum KindEnum { EMPTY, FULL, RANGE } Kind;
  ArrayRangeInfo(KindEnum Kind, const SCEV *Low = nullptr,
                 const SCEV *High = nullptr)
  : Low(Low), High(High), Kind(Kind) {}

public:
  /// Default constructor (for use as DenseMap values)--starts as empty.
  ArrayRangeInfo() : ArrayRangeInfo(EMPTY) {}
  static ArrayRangeInfo empty() { return ArrayRangeInfo(EMPTY); }
  static ArrayRangeInfo full() { return ArrayRangeInfo(FULL); }
  static ArrayRangeInfo range(const SCEV *Low, const SCEV *High) {
    return ArrayRangeInfo(RANGE, Low, High);
  }
  bool isEmpty() const { return Kind == EMPTY; }
  bool isFull() const { return Kind == FULL; }
  const SCEV *getLowIndex() const { return Low; }
  const SCEV *getHighIndex() const { return High; }

  /// Return a range info that is the union of this and another range.
  ArrayRangeInfo unionWith(const ArrayRangeInfo &, ScalarEvolution &) const;
  /// Return a range info that contains values in this set but not the other
  /// set.
  ArrayRangeInfo difference(const ArrayRangeInfo &, ScalarEvolution &) const;
  bool operator==(const ArrayRangeInfo &Other) const;
  bool operator!=(const ArrayRangeInfo &O) const { return !(*this == O); }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS) const;
  friend raw_ostream &operator<<(raw_ostream &OS, const ArrayRangeInfo &Range) {
    Range.print(OS);
    return OS;
  }
#endif
};

/// Array liveness information for a single array.
class ArrayUseInfo final {
  class RangeDataflow;

  Value *Source;
  const SCEV *Size;
  std::unique_ptr<RangeDataflow> DataflowResults;

  friend class ArrayUse;

  std::unique_ptr<RangeDataflow> computeDataflow(ScalarEvolution &SE,
    const ArrayUse &AU);

public:
  ArrayUseInfo(Value *Source, const SCEV *Size);
  ~ArrayUseInfo();
  static std::unique_ptr<ArrayUseInfo> make(Value *Source, ScalarEvolution &SE);

  /// Get the size of the array, as a SCEV value.
  const SCEV *getSize() const { return Size; }

  /// Get the instruction or global that allocates this array.
  Value *getAllocation() const { return Source; }

  /// Get the range of the array used after any given point in the program.
  ArrayRangeInfo getRangeUseAfterPoint(Instruction *I) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS) const;
#endif
};

/// An caching analysis to determine array range liveness information at
/// various points in the program.
class ArrayUse {
  typedef ValueMap<const Value *, std::unique_ptr<ArrayUseInfo>> ArrayMapTy;
  Function &F;
  LoopInfo &LI;
  ScalarEvolution &SE;
  std::unique_ptr<ArrayMapTy> ArrayUseMap;

  ArrayRangeInfo getRangeForSCEV(const SCEV *, const SCEV *Size) const;

public:
  ArrayUse(Function &F, LoopInfo &LI, ScalarEvolution &SE);

  /// Get the range that is used by a particular instruction. The instruction
  /// must be a load, store, or call instruction.
  ArrayRangeInfo getRangeUse(Instruction &I) const;

  /// Get the \c ArrayUseInfo that the given value is a part of.
  ArrayUseInfo *getSourceArray(Value *V) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// Print the analysis for debugging.
  void print(raw_ostream &OS) const;
#endif

  void reset() { ArrayUseMap.reset(); }
};

/// Analysis pass that exposes the \c ArrayUse for a function.
class ArrayUseAnalysis
    : public AnalysisInfoMixin<ArrayUseAnalysis> {
  friend AnalysisInfoMixin<ArrayUseAnalysis>;

  static AnalysisKey Key;

public:
  using Result = ArrayUse;

  ArrayUse run(Function &F, FunctionAnalysisManager &AM);
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// Printer pass for the \c ArrayUseAnalysis results.
class ArrayUsePrinterPass
    : public PassInfoMixin<ArrayUsePrinterPass> {
  raw_ostream &OS;

public:
  explicit ArrayUsePrinterPass(raw_ostream &OS) : OS(OS) {}

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};
#endif

class ArrayUseWrapperPass : public FunctionPass {
  std::unique_ptr<ArrayUse> AU;

public:
  static char ID;

  ArrayUseWrapperPass();

  ArrayUse &getArrayUse() { return *AU; }
  const ArrayUse &getArrayUse() const { return *AU; }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS, const Module * = nullptr) const override;
#endif
};

} // namespace llvm

#endif // LLVM_ANALYSIS_INTEL_ARRAYUSEANALYSIS_H
