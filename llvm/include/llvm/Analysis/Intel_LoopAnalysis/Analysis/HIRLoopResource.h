//===--------- HIRLoopResource.h - Provides Loop Resource --------*- C++-*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This analysis provides loop resource information for a given loop.
// The analysis stores information of resource computation and invalidates it,
// whenever the loop is modified. This property avoids recomputation across
// different passes.

// The loop resource contains the number of memory operations such as reads and
// writes and operations such as floating point or integer. This information
// is useful for transformations. However, the computation of loop resource
// is not exactly precise since it does not visit the predicates inside the if
// condition. The rationale is that many of these operations may be moved
// outside the loop or be eliminated by common subexpression elimination.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_RESOURCE_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_RESOURCE_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/FormattedStream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRAnalysisPass.h"

namespace llvm {

class formatted_raw_ostream;
class TargetTransformInfo;
class Instruction;
class Loop;
class LoopInfo;

namespace loopopt {

class CanonExpr;
class HLInst;
class HLLoop;
class HIRLoopResource;

// Captures all the loop resource information.
struct LoopResourceInfo {
private:
  /// Indicates whether loop cost is dominated by memory, FP or int operations.
  enum LoopResourceBound { Memory, FP, Int, Branch, Unknown };

  unsigned IntOps;
  unsigned IntOpsCost;
  unsigned FPOps;
  unsigned FPOpsCost;
  unsigned BranchOps;
  unsigned BranchOpsCost;
  unsigned IntMemReads;
  unsigned IntMemWrites;
  unsigned FPMemReads;
  unsigned FPMemWrites;
  LoopResourceBound Bound;

public:
  LoopResourceInfo()
      : IntOps(0), IntOpsCost(0), FPOps(0), FPOpsCost(0), BranchOps(0),
        BranchOpsCost(0), IntMemReads(0), IntMemWrites(0), FPMemReads(0),
        FPMemWrites(0), Bound(LoopResourceBound::Unknown) {}

  LoopResourceInfo(const LoopResourceInfo &LRI)
      : IntOps(LRI.IntOps), IntOpsCost(LRI.IntOpsCost), FPOps(LRI.FPOps),
        FPOpsCost(LRI.FPOpsCost), BranchOps(LRI.BranchOps),
        BranchOpsCost(LRI.BranchOpsCost), IntMemReads(LRI.IntMemReads),
        IntMemWrites(LRI.IntMemWrites), FPMemReads(LRI.FPMemReads),
        FPMemWrites(LRI.FPMemWrites), Bound(LRI.Bound) {}

  /// Costs metrics of operations.
  enum OperationCost { FreeOp = 0, BasicOp = 1, ExpensiveOp = 2, MemOp = 4 };

  /// Visitor to compute resource.
  struct LoopResourceVisitor;

  /// Returns the number of memory reads.
  unsigned getNumMemReads() const { return IntMemReads + FPMemReads; }

  /// Returns the number of memory writes.
  unsigned getNumMemWrites() const { return IntMemWrites + FPMemWrites; }

  /// Returns the number of integer memory operations.
  unsigned getNumIntMemOps() const { return IntMemReads + IntMemWrites; }

  /// Returns the cost of integer memory operations.
  unsigned getIntMemOpsCost() const {
    return OperationCost::MemOp * getNumIntMemOps();
  }

  /// Returns the number of FP memory operations.
  unsigned getNumFPMemOps() const { return FPMemReads + FPMemWrites; }

  /// Returns the cost of FP memory operations.
  unsigned getFPMemOpsCost() const {
    return OperationCost::MemOp * getNumFPMemOps();
  }

  /// Returns the number of branching operations.
  unsigned getNumBranchOps() const { return BranchOps; }

  /// Returns the cost of branching (misprediction) operations.
  unsigned getBranchOpsCost() const { return BranchOpsCost; }

  /// Returns the total number of memory references.
  unsigned getNumMemOps() const { return getNumIntMemOps() + getNumFPMemOps(); }

  /// Returns the cost of all memory operations.
  unsigned getMemOpsCost() const {
    return getIntMemOpsCost() + getFPMemOpsCost();
  }

  /// Returns the number of integer operations.
  unsigned getNumIntOps() const { return IntOps; }

  /// Returns the cost of integer operations.
  unsigned getIntOpsCost() const { return IntOpsCost; }

  /// Returns the number of FP operations.
  unsigned getNumFPOps() const { return FPOps; }

  /// Returns the cost of FP operations.
  unsigned getFPOpsCost() const { return FPOpsCost; }

  /// Returns the cost of integer and FP operations.
  unsigned getNumIntAndFPOps() const { return getNumIntOps() + getNumFPOps(); }

  /// Returns the cost of integer and FP operations.
  unsigned getIntAndFPOpsCost() const {
    return getIntOpsCost() + getFPOpsCost();
  }

  /// Returns the cost of integer, FP and memory operations.
  unsigned getTotalCost() const {
    return getIntAndFPOpsCost() + getBranchOpsCost() + getMemOpsCost();
  }

  /// Returns true if loop resource is memory bound.
  bool isMemBound() const { return (Bound == LoopResourceBound::Memory); }

  /// Returns true if loop resource is FP operations bound.
  bool isFPBound() const { return (Bound == LoopResourceBound::FP); }

  /// Returns true if loop resource is integer operations bound.
  bool isIntBound() const { return (Bound == LoopResourceBound::Int); }

  /// Returns true if loop resource is branching operations bound.
  bool isBranchBound() const { return (Bound == LoopResourceBound::Branch); }

  /// Returns true if loop resource bound cannot be determined.
  bool isUnknownBound() const { return (Bound == LoopResourceBound::Unknown); }

  /// Classifies loop resource as Mem, FP, Int or Unknown bound.
  void classify();

  /// Computes loop resource for the passed in loop in the current object.
  void compute(HIRLoopResource &HLR, const HLLoop *Lp);

  /// Adds the loop resource LRI to this resource information.
  LoopResourceInfo &operator+=(const LoopResourceInfo &LRI);

  /// Multiplies loop resource.
  LoopResourceInfo &operator*=(unsigned Multiplier);

  /// Adds \p Num integer operations each with a cost of \p Cost.
  void addIntOps(unsigned Cost, unsigned Num = 1) {
    IntOps += Num;
    IntOpsCost += (Cost * Num);
  }

  /// Adds \p Num FP operations each with a cost of \p Cost.
  void addFPOps(unsigned Cost, unsigned Num = 1) {
    FPOps += Num;
    FPOpsCost += (Cost * Num);
  }

  /// Adds 1 branch operation with a cost of \p Cost.
  void addBranchOps(unsigned Cost) {
    ++BranchOps;
    BranchOpsCost += Cost;
  }

  /// Prints the loop resource.
  void print(formatted_raw_ostream &OS, const HLLoop *Lp) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(const HLLoop *Lp) const {
    formatted_raw_ostream FOS(dbgs());
    print(FOS, Lp);
  }
#endif
};

class HIRLoopResource : public HIRAnalysis {
private:
  const LoopInfo &LI;
  const TargetTransformInfo &TTI;

  /// Maintains self resource information for loops.
  DenseMap<const HLLoop *, LoopResourceInfo> SelfResourceMap;

  /// Maintains total resource information for loops.
  DenseMap<const HLLoop *, LoopResourceInfo> TotalResourceMap;

  /// Returns the cost of an LLVM instruction.
  unsigned getOperationCost(const Instruction &Inst) const;

protected:
  /// Prints analyis results for loop.
  virtual void print(formatted_raw_ostream &OS, const HLLoop *Lp) override;

public:
  HIRLoopResource(HIRFramework &HIRF, LoopInfo &LI, TargetTransformInfo &TTI)
      : HIRAnalysis(HIRF), LI(LI), TTI(TTI) {}
  HIRLoopResource(HIRLoopResource &&Arg)
      : HIRAnalysis(std::move(Arg)), LI(Arg.LI), TTI(Arg.TTI),
        SelfResourceMap(std::move(Arg.SelfResourceMap)),
        TotalResourceMap(std::move(Arg.TotalResourceMap)) {}
  HIRLoopResource(const HIRLoopResource &) = delete;

  /// Returns pointer to TargetTransformInfo analysis.
  const TargetTransformInfo &getTTI() const { return TTI; }

  /// \brief This method will mark the loop and all its parent loops as
  /// modified. If loop changes, resources of the loop and all its parents loops
  /// needs to be recomputed.
  void markLoopBodyModified(const HLLoop *Loop) override;

  /// \brief Returns the loop resource of the specified loop. This excludes loop
  /// resource of children loops.
  const LoopResourceInfo &getSelfLoopResource(const HLLoop *Loop);

  /// \brief Returns the loop resource of the specified loop including children
  /// loops.
  /// NOTE: Children loop's resource is added assuming a trip count of one. No
  /// multiplier is involved.
  const LoopResourceInfo &getTotalLoopResource(const HLLoop *Loop);

  /// Returns the cost of a LLVM loop excluding sub-loops. The cost is computed
  /// using the same metrics as LoopResourceInfo.
  unsigned getLLVMLoopCost(const Loop &Lp);
};

class HIRLoopResourceWrapperPass : public FunctionPass {
  std::unique_ptr<HIRLoopResource> HLR;

public:
  static char ID;
  HIRLoopResourceWrapperPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void releaseMemory() override;

  void print(raw_ostream &OS, const Module * = nullptr) const override {
    getHLR().printAnalysis(OS);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const {
    formatted_raw_ostream FOS(dbgs());
    print(FOS);
  }
#endif

  HIRLoopResource &getHLR() { return *HLR; }
  const HIRLoopResource &getHLR() const { return *HLR; }
};

class HIRLoopResourceAnalysis
    : public AnalysisInfoMixin<HIRLoopResourceAnalysis> {
  friend struct AnalysisInfoMixin<HIRLoopResourceAnalysis>;

  static AnalysisKey Key;

public:
  using Result = HIRLoopResource;

  HIRLoopResource run(Function &F, FunctionAnalysisManager &AM);
};

} // End namespace loopopt

} // End namespace llvm

#endif
