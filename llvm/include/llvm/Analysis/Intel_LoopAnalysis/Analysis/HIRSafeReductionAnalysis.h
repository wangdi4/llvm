//===--------   HIRSafeReductionAnalysis.h    -----------------------------===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_SAFEREDUCTION_H
#define LLVM_ANALYSIS_INTEL_SAFEREDUCTION_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRAnalysisPass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

#include <map>

namespace llvm {

class formatted_raw_ostream;

namespace loopopt {

class CanonExpr;
class HLInst;
class HLLoop;

typedef SmallVector<const HLInst *, 4> SafeRedChain;

struct SafeRedInfo {
  SafeRedChain Chain;
  unsigned Symbase;
  unsigned OpCode;
  bool HasUnsafeAlgebra;

  SafeRedInfo(SafeRedChain &RedInsts, unsigned Symbase, unsigned RedOpCode,
              bool HasUnsafeAlgebra)
      : Chain(RedInsts), Symbase(Symbase), OpCode(RedOpCode),
        HasUnsafeAlgebra(HasUnsafeAlgebra) {}

  bool hasUnsafeAlgebra(void) const { return HasUnsafeAlgebra; }
};

typedef SmallVector<SafeRedInfo, 4> SafeRedInfoList;

class HIRSafeReductionAnalysis : public HIRAnalysis {
  HIRDDAnalysis &DDA;

  unsigned FirstRvalSB;
  const HLNode *FirstChild;
  // From Loop, look up all sets of Insts in a Safe Reduction chain
  SmallDenseMap<const HLLoop *, SafeRedInfoList, 16> SafeReductionMap;
  // From Inst, Look up  Index to Reduction Info (Chain, Symbase and Opcode).
  // There is no need to go through Loop,
  // because there are not many safe reductions in a function.
  SmallDenseMap<const HLInst *, unsigned, 16> SafeReductionInstMap;

  bool findFirstRedStmt(const HLLoop *Loop, const HLInst *Inst,
                        bool *SingleStmtReduction, unsigned *FirstSB,
                        unsigned *ReductionOpCode, DDGraph DDG);

  void setSafeRedChainList(SafeRedChain &RedInsts, const HLLoop *Loop,
                           unsigned RedSymbase, unsigned RedOpCode);

  void identifySafeReductionChain(const HLLoop *Loop, DDGraph DDG);
  bool isValidSR(const RegDDRef *LRef, const HLLoop *Loop, HLInst **SinkInst,
                 DDRef **SinkDDRef, unsigned ReductionOpCode, DDGraph DDG);

  // Perform  SafeReducton Analysis
  void identifySafeReduction(const HLLoop *Loop);
  // Checks if a temp is legal to be used for Reduction
  //  e.g s =  10 * s + ..  is not legal
  bool isRedTemp(CanonExpr *CE, unsigned BlobIndex);

public:
  HIRSafeReductionAnalysis(HIRFramework &HIRF, HIRDDAnalysis &DDA);
  HIRSafeReductionAnalysis(const HIRSafeReductionAnalysis &) = delete;
  HIRSafeReductionAnalysis(HIRSafeReductionAnalysis &&Arg)
      : HIRAnalysis(Arg.HIRF), DDA(Arg.DDA),
        SafeReductionMap(std::move(Arg.SafeReductionMap)),
        SafeReductionInstMap(std::move(Arg.SafeReductionInstMap)) {}
  virtual ~HIRSafeReductionAnalysis() {}

  // Compute SafeReduction for all innermost loops
  void computeSafeReductionChains(const HLLoop *Loop);

  // Get SafeReduction of a Loop
  const SafeRedInfoList &getSafeRedInfoList(const HLLoop *Loop);

  // Is Inst part of a Safe Reduction. Indicate of Single Stmt when
  // argument supplied
  bool isSafeReduction(const HLInst *Inst, bool *IsSingleStmt = nullptr,
                       bool *HasUnsafeAlgebra = nullptr) const;

  // Get Safe Reduction Info of a Loop (Chain, Symbase and Opcode). Returns null
  // if the instruction is not part of a reduction
  const SafeRedInfo *getSafeRedInfo(const HLInst *Inst) const;

  // Checks if operand is a safe reduction operand and returns related opcode
  bool isReductionRef(const RegDDRef *Ref, unsigned &RedOpCode);

  void printAnalysis(raw_ostream &OS) const override;
  void print(formatted_raw_ostream &OS, const HLLoop *Loop) override;
  void print(formatted_raw_ostream &OS, const HLLoop *Loop,
             const SafeRedInfoList *SR);

  void markLoopBodyModified(const HLLoop *L) override;
};

class HIRSafeReductionAnalysisWrapperPass : public FunctionPass {
  std::unique_ptr<HIRSafeReductionAnalysis> HSR;

public:
  static char ID;
  HIRSafeReductionAnalysisWrapperPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void releaseMemory() override;

  void print(raw_ostream &OS, const Module * = nullptr) const override {
    getHSR().printAnalysis(OS);
  }

  HIRSafeReductionAnalysis &getHSR() { return *HSR; }
  const HIRSafeReductionAnalysis &getHSR() const { return *HSR; }
};

class HIRSafeReductionAnalysisPass
    : public AnalysisInfoMixin<HIRSafeReductionAnalysisPass> {
  friend struct AnalysisInfoMixin<HIRSafeReductionAnalysisPass>;

  static AnalysisKey Key;

public:
  using Result = HIRSafeReductionAnalysis;

  HIRSafeReductionAnalysis run(Function &F, FunctionAnalysisManager &AM);
};

} // End namespace loopopt

} // End namespace llvm

#endif
