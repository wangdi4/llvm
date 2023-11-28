//===------- HIRArraySectionAnalysis.h -----------------------*- C++ -*----===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_HIRARRAYSECTIONANALYSIS_H
#define LLVM_HIRARRAYSECTIONANALYSIS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRAnalysisPass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/TargetTransformInfo.h"

namespace llvm {

namespace loopopt {

class ArraySectionInfo {
public:
  enum UDFlagTy : unsigned char {
    UNKNOWN = 0,
    USE = 1,
    DEF = 2
  };

private:
  UDFlagTy UDFlag = UDFlagTy::UNKNOWN;

  // Note: Lowers and Uppers are always have the same size.
  SmallVector<CanonExpr *, 4> Lowers;
  SmallVector<CanonExpr *, 4> Uppers;

  // Keep all unique index expressions per dimension.
  SmallVector<SmallVector<CanonExpr *, 1>, 4> Indices;

public:
  ArraySectionInfo(){};
  ArraySectionInfo(unsigned NumDimensions) {
    assert(NumDimensions != 0);
    Lowers.resize(NumDimensions);
    Uppers.resize(NumDimensions);
    Indices.resize(NumDimensions);
  }

  ArraySectionInfo(const ArraySectionInfo &Info) = delete;
  ArraySectionInfo &operator=(const ArraySectionInfo &Info) = delete;

  ArraySectionInfo clone() const;

  ArraySectionInfo(ArraySectionInfo &&) = default;
  ArraySectionInfo &operator=(ArraySectionInfo &&) = default;

  void clear() {
    Lowers.clear();
    Uppers.clear();
    Indices.clear();
    UDFlag = UDFlagTy::UNKNOWN;
  }

  bool equals(const ArraySectionInfo &Info) const;

  unsigned getNumDimensions() const { return Lowers.size(); }

  ArrayRef<const CanonExpr *> lowers() const { return Lowers; }
  MutableArrayRef<CanonExpr *> lowers() { return Lowers; }

  ArrayRef<const CanonExpr *> uppers() const { return Uppers; }
  MutableArrayRef<CanonExpr *> uppers() { return Uppers; }

  ArrayRef<const CanonExpr *> indices(unsigned Dim) const {
    return Indices[Dim];
  }

  SmallVectorImpl<CanonExpr *> &indices(unsigned Dim) { return Indices[Dim]; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS) const;

  LLVM_DUMP_METHOD void dump() const {
    formatted_raw_ostream FOS(dbgs());
    print(FOS);
    dbgs() << "\n";
  }
#endif

  UDFlagTy getUseDefFlags() const { return UDFlag; }
  bool isUse() const { return UDFlag & UDFlagTy::USE; }
  bool isDef() const { return UDFlag & UDFlagTy::DEF; }
  void setUse() { UDFlag = static_cast<UDFlagTy>(UDFlag | UDFlagTy::USE); }
  void setDef() { UDFlag = static_cast<UDFlagTy>(UDFlag | UDFlagTy::DEF); }
};

class ArraySectionAnalysisResult {
  DenseMap<unsigned, ArraySectionInfo> ArraySections;
  SmallVector<unsigned, 16> KnownBaseIndices;

public:
  const ArraySectionInfo *get(unsigned BaseIndex) const {
    auto I = ArraySections.find(BaseIndex);
    return I != ArraySections.end() ? &I->second : nullptr;
  }

  ArraySectionInfo *get(unsigned BaseIndex) {
    auto I = ArraySections.find(BaseIndex);
    return I != ArraySections.end() ? &I->second : nullptr;
  }

  ArraySectionInfo &create(unsigned BaseIndex) {
    assert(ArraySections.count(BaseIndex) == 0 && "Index already exist");
    KnownBaseIndices.push_back(BaseIndex);
    return ArraySections[BaseIndex];
  }

  bool empty() const { return knownBaseIndices().empty(); }

  ArrayRef<unsigned> knownBaseIndices() const { return KnownBaseIndices; }
  void merge(const ArraySectionAnalysisResult &InResult,
             const HLLoop *Loop = nullptr);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(formatted_raw_ostream &OS, const HLLoop *Lp = nullptr) const;

  LLVM_DUMP_METHOD void dump() const {
    formatted_raw_ostream FOS(dbgs());
    print(FOS);
    dbgs() << "\n";
  }
#endif
};

class HIRArraySectionAnalysis : public HIRAnalysis {
  HIRDDAnalysis &DDA;

  SmallDenseMap<const HLLoop *, std::unique_ptr<ArraySectionAnalysisResult>>
      Cache;

public:
  HIRArraySectionAnalysis(HIRFramework &HIRF, HIRDDAnalysis &DDA);
  HIRArraySectionAnalysis(const HIRArraySectionAnalysis &) = delete;
  HIRArraySectionAnalysis(HIRArraySectionAnalysis &&Arg) = default;

  const ArraySectionAnalysisResult &getOrCompute(const HLLoop *Loop);

  void markLoopBodyModified(const HLLoop *Lp) override;
  void markLoopBoundsModified(const HLLoop *Lp) override;

  void print(formatted_raw_ostream &OS, const HLLoop *Lp) override;
};

class HIRArraySectionAnalysisWrapperPass : public FunctionPass {
  std::unique_ptr<HIRArraySectionAnalysis> ASA;

public:
  static char ID;
  HIRArraySectionAnalysisWrapperPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void releaseMemory() override { ASA.reset(); }

  void print(raw_ostream &OS, const Module * = nullptr) const override {
    getASA().printAnalysis(OS);
  }

  HIRArraySectionAnalysis &getASA() { return *ASA; }
  const HIRArraySectionAnalysis &getASA() const { return *ASA; }
};

class HIRArraySectionAnalysisPass
    : public AnalysisInfoMixin<HIRArraySectionAnalysisPass> {
  friend struct AnalysisInfoMixin<HIRArraySectionAnalysisPass>;

  static AnalysisKey Key;

public:
  using Result = HIRArraySectionAnalysis;

  HIRArraySectionAnalysis run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_HIRARRAYSECTIONANALYSIS_H
