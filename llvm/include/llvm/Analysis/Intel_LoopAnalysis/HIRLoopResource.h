//===--------- HIRLoopResource.h - Provides Loop Resource --------*- C++-*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#include "llvm/Pass.h"

#include "llvm/ADT/DenseMap.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRAnalysisPass.h"

namespace llvm {

class formatted_raw_ostream;

namespace loopopt {

class CanonExpr;
class HLInst;
class HLLoop;
class HIRLoopResource;

// Captures all the loop resource information.
struct LoopResourceInfo {
private:
  /// Indicates whether loop cost is dominated by memory, FP or int operations.
  enum LoopResourceBound { Memory, FP, Int, Unknown };

  unsigned IntOps;
  unsigned FPOps;
  unsigned IntMemReads;
  unsigned IntMemWrites;
  unsigned FPMemReads;
  unsigned FPMemWrites;
  LoopResourceBound Bound;

  /// Costs based on type of operation.
  // TODO: Tune for specific targets later.
  static const unsigned IntCost = 1;
  static const unsigned FPCost = 2;
  static const unsigned MemCost = 2;

public:
  LoopResourceInfo()
      : IntOps(0), FPOps(0), IntMemReads(0), IntMemWrites(0), FPMemReads(0),
        FPMemWrites(0), Bound(LoopResourceBound::Unknown) {}

  LoopResourceInfo(const LoopResourceInfo &LRI)
      : IntOps(LRI.IntOps), FPOps(LRI.FPOps), IntMemReads(LRI.IntMemReads),
        IntMemWrites(LRI.IntMemWrites), FPMemReads(LRI.FPMemReads),
        FPMemWrites(LRI.FPMemWrites), Bound(LRI.Bound) {}

  /// Visitor to compute resource.
  struct LoopResourceVisitor;

  /// Returns the number of memory reads.
  unsigned getNumMemReads() const { return IntMemReads + FPMemReads; }

  /// Returns the number of memory writes.
  unsigned getNumMemWrites() const { return IntMemWrites + FPMemWrites; }

  /// Returns the number of integer memory operations.
  unsigned getNumIntMemOps() const { return IntMemReads + IntMemWrites; }

  /// Returns the cost of integer memory operations.
  unsigned getIntMemOpsCost() const { return IntCost * getNumIntMemOps(); }

  /// Returns the number of FP memory operations.
  unsigned getNumFPMemOps() const { return FPMemReads + FPMemWrites; }

  /// Returns the cost of FP memory operations.
  unsigned getFPMemOpsCost() const { return FPCost * getNumFPMemOps(); }

  /// Returns the total number of memory references.
  unsigned getNumMemOps() const { return getNumIntMemOps() + getNumFPMemOps(); }

  /// Returns the cost of all memory operations.
  unsigned getMemOpsCost() const {
    return getIntMemOpsCost() + getFPMemOpsCost();
  }

  /// Returns the number of integer operations.
  unsigned getNumIntOps() const { return IntOps; }

  /// Returns the cost of integer operations.
  unsigned getIntOpsCost() const { return IntCost * getNumIntOps(); }

  /// Returns the number of FP operations.
  unsigned getNumFPOps() const { return FPOps; }

  /// Returns the cost of FP operations.
  unsigned getFPOpsCost() const { return FPCost * getNumFPOps(); }

  /// Returns the cost of integer and FP operations.
  unsigned getNumIntAndFPOps() const { return getNumIntOps() + getNumFPOps(); }

  /// Returns the cost of integer and FP operations.
  unsigned getIntAndFPOpsCost() const {
    return getIntOpsCost() + getFPOpsCost();
  }

  /// Returns true if loop resource is memory bound.
  bool isMemBound() const { return (Bound == LoopResourceBound::Memory); }

  /// Returns true if loop resource is FP operations bound.
  bool isFPBound() const { return (Bound == LoopResourceBound::FP); }

  /// Returns true if loop resource is integer operations bound.
  bool isIntBound() const { return (Bound == LoopResourceBound::Int); }

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

  /// Prints the loop resource.
  void print(formatted_raw_ostream &OS, const HLLoop *Lp) const;
};

class HIRLoopResource final : public HIRAnalysisPass {
private:
  /// Maintains self resource information for loops.
  DenseMap<const HLLoop *, LoopResourceInfo> SelfResourceMap;

  /// Maintains total resource information for loops.
  DenseMap<const HLLoop *, LoopResourceInfo> TotalResourceMap;

  /// \brief Computes self and/or total loop resource of \p Lp. If SelfOnly mode
  /// is set, we only compute the self resource.
  const LoopResourceInfo &computeLoopResource(const HLLoop *Lp, bool SelfOnly);

protected:
  /// Prints analyis results for loop.
  virtual void print(formatted_raw_ostream &OS, const HLLoop *Lp) override;

public:
  HIRLoopResource()
      : HIRAnalysisPass(ID, HIRAnalysisPass::HIRLoopResourceVal) {}
  static char ID;

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const;

  void releaseMemory() override;

  /// \brief This method will mark the loop and all its parent loops as
  /// modified. If loop changes, resources of the loop and all its parents loops
  /// needs to recomputed.
  void markLoopBodyModified(const HLLoop *Loop) override;

  /// \brief Returns the loop resource of the specified loop. This excludes loop
  /// resource of children loops.
  const LoopResourceInfo &getSelfLoopResource(const HLLoop *Loop);

  /// \brief Returns the loop resource of the specified loop including children
  /// loops.
  /// NOTE: Children loop's resouce is added assuming a trip count of one. No
  /// multiplier is involved.
  const LoopResourceInfo &getTotalLoopResource(const HLLoop *Loop);

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HIRAnalysisPass *AP) {
    return AP->getHIRAnalysisID() == HIRAnalysisPass::HIRLoopResourceVal;
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
