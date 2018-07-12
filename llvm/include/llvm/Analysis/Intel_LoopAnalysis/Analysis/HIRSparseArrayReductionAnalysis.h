//===--------   HIRSparseArrayReductionAnalysis.h   -----------------------===//
//
// Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
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

#ifndef LLVM_ANALYSIS_INTEL_SPARSE_ARRAY_REDUCTION_H
#define LLVM_ANALYSIS_INTEL_SPARSE_ARRAY_REDUCTION_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRAnalysisPass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

#include <map>

namespace llvm {

class formatted_raw_ostream;
namespace loopopt {

class CanonExpr;
class HLInst;
class HLLoop;

typedef SmallVector<const HLInst *, 4> SparseArrayReductionChain;

struct SparseArrayReductionInfo {
  SparseArrayReductionChain Chain;
  unsigned Symbase;
  unsigned OpCode;

  SparseArrayReductionInfo(SparseArrayReductionChain &ReductionInsts,
                           unsigned Symbase, unsigned ReductionOpCode)
      : Chain(ReductionInsts), Symbase(Symbase), OpCode(ReductionOpCode) {}
};

typedef SmallVector<SparseArrayReductionInfo, 4> SparseArrayReductionChainList;

// This distance is set as 2 for 544.nab
// It can be increased for longer reduction chain
const unsigned SparserLoadDistance = 2;

class HIRSparseArrayReductionAnalysis : public HIRAnalysis {
  typedef DDRefGatherer<const RegDDRef, MemRefs> MemRefGatherer;
  typedef DDRefGrouping::RefGroupTy<const RegDDRef *> RefGroupTy;
  typedef DDRefGrouping::RefGroupVecTy<const RegDDRef *> RefGroupVecTy;

  HIRDDAnalysis &DDA;
  DDGraph DDG;

  // From Loop, look up all sets of Insts in a Sparse Array Reduction chain
  SmallDenseMap<const HLLoop *, SparseArrayReductionChainList, 16>
      SparseArrayReductionMap;

  // From Inst, Look up Index to Reduction Info (Chain, Symbase and Opcode).
  // There is no need to go through Loop,
  // Because there may be so many sparse array reductions in a loop.
  SmallDenseMap<const HLInst *, unsigned, 16> SparseArrayReductionInstMap;

  // Perform  Sparse Array Reducton Analysis
  void identifySparseArrayReductionChains(const HLLoop *Loop);

  // Find whether there a flow-edge from a load within Hop distance
  // Works with a recursive walk on Rval ddrefs (including Blob ddrefs)
  bool findLoadInstWithinNHops(const HLInst *SrcInst, unsigned NestingLevel,
                               unsigned Hop, bool *SingleLoadFound);

  // Given an instruction, find out whether this is a valid reduction statement
  // With an incoming and outgoing flow-edge to sparse array access
  bool isReductionStmt(const HLInst *Inst, unsigned *ReductionOpCode,
                       const RegDDRef *StoreRef);

  // Perform all the legality checks on the candidate memory references
  bool isLegallyValid(const RegDDRef *LoadRef, const RegDDRef *StoreRef,
                      const HLLoop *Loop, const BlobDDRef *NonLinearBRRef,
                      HLInst **ReductionInst, unsigned *ReductionOpCode);

  // Saves a newly identified chain in a map
  void setSparseArrayReductionChainList(
      SparseArrayReductionChain &ReductionInsts, const HLLoop *Loop,
      unsigned ReductionSymbase, unsigned ReductionOpCode);

  // Determines whether memory refs of a particular symbase is such that it
  // becomes a candidate for reduction
  // Process the input memory refs symbase and generate reduction chains
  void validateAndCreateSparseArrayReduction(const HLLoop *Loop,
                                             const RefGroupTy &RefVec);

public:
  HIRSparseArrayReductionAnalysis(HIRFramework &HIRF, HIRDDAnalysis &DDA);
  HIRSparseArrayReductionAnalysis(const HIRSparseArrayReductionAnalysis &) =
      delete;
  HIRSparseArrayReductionAnalysis(HIRSparseArrayReductionAnalysis &&Arg)
      : HIRAnalysis(Arg.HIRF), DDA(Arg.DDA),
        SparseArrayReductionMap(std::move(Arg.SparseArrayReductionMap)),
        SparseArrayReductionInstMap(
            std::move(Arg.SparseArrayReductionInstMap)) {}
  virtual ~HIRSparseArrayReductionAnalysis() {}

  // Compute SparseArrayReduction for all innermost loops
  void computeSparseArrayReductionChains(const HLLoop *Loop);

  // Get SparseArrayReduction of a Loop
  const SparseArrayReductionChainList &
  getSparseArrayReductionChain(const HLLoop *Loop);

  // Returns the total number of chains identified
  unsigned getNumSparseArrayReductionChains(const HLLoop *Loop) {
    SparseArrayReductionChainList &SARCL = SparseArrayReductionMap[Loop];
    return SARCL.size();
  }

  // Is Inst part of a Sparse Array Reduction. Indicate of Single Stmt when
  // argument supplied
  bool isSparseArrayReduction(const HLInst *Inst,
                              bool *IsSingleStmt = nullptr) const;

  // Get SparseArray Reduction Info of a Loop (Chain, Symbase and Opcode).
  // Returns null if the instruction is not part of a reduction
  const SparseArrayReductionInfo *
  getSparseArrayReductionInfo(const HLInst *Inst) const;

  // Checks if operand is a sparse array reduction operand
  // And returns related opcode
  bool isReductionRef(const RegDDRef *Ref, unsigned &ReductionOpCode);

  void print(formatted_raw_ostream &OS, const HLLoop *Loop) override;
  void print(formatted_raw_ostream &OS, const HLLoop *Loop,
             const SparseArrayReductionChainList *SR);
  void markLoopBodyModified(const HLLoop *L) override;
};

class HIRSparseArrayReductionAnalysisWrapperPass : public FunctionPass {
  std::unique_ptr<HIRSparseArrayReductionAnalysis> HSAR;

public:
  static char ID;
  HIRSparseArrayReductionAnalysisWrapperPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void releaseMemory() override;

  void print(raw_ostream &OS, const Module * = nullptr) const override {
    getHSAR().printAnalysis(OS);
  }

  HIRSparseArrayReductionAnalysis &getHSAR() { return *HSAR; }
  const HIRSparseArrayReductionAnalysis &getHSAR() const { return *HSAR; }
};

class HIRSparseArrayReductionAnalysisPass
    : public AnalysisInfoMixin<HIRSparseArrayReductionAnalysisPass> {
  friend struct AnalysisInfoMixin<HIRSparseArrayReductionAnalysisPass>;

  static AnalysisKey Key;

public:
  using Result = HIRSparseArrayReductionAnalysis;

  HIRSparseArrayReductionAnalysis run(Function &F, FunctionAnalysisManager &AM);
};

class HIRSparseArrayReductionAnalysisPrinterPass
    : public PassInfoMixin<HIRSparseArrayReductionAnalysisPrinterPass> {
  raw_ostream &OS;

public:
  explicit HIRSparseArrayReductionAnalysisPrinterPass(raw_ostream &OS)
      : OS(OS) {}

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    AM.getResult<HIRSparseArrayReductionAnalysisPass>(F).printAnalysis(OS);
    return PreservedAnalyses::all();
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
