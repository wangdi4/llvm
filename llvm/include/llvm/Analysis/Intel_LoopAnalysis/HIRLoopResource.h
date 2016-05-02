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
  enum LoopResourceBound {
    // Indicates if the loop has large number of memory operations.
    Memory,
    // Indicates if the loop has large number of floating point operations.
    FP,
    // Indicates if the loop has large number of integer operations.
    Int,
    // Indicates that loop bounding type is unknown.
    Unknown
  };

  // Integer operations
  unsigned IntOps;
  // Floating point operations
  unsigned FPOps;
  // Integer loads/reads
  unsigned IntMemReads;
  // Integer stores/writes
  unsigned IntMemWrites;
  // Floating point loads/reads.
  unsigned FPMemReads;
  // Floating point stores/writes.
  unsigned FPMemWrites;
  // Indicates the loop resource bound.
  LoopResourceBound Bound;

  // Costs based on type of operation.
  // TODO: Tune for specific targets later.
  static const unsigned IntCost = 1;
  static const unsigned FPCost = 2;
  static const unsigned MemCost = 2;

  /// Visitor to compute resource for the loop.
  struct LoopResourceVisitor;

  // Adds the loop resource LR to this resource information.
  LoopResourceInfo &operator+=(const LoopResourceInfo &LRI);

  // Classifies loop resource as Mem, FP, Int or Unknown bound.
  void classify();

public:
  LoopResourceInfo()
      : IntOps(0), FPOps(0), IntMemReads(0), IntMemWrites(0), FPMemReads(0),
        FPMemWrites(0), Bound(LoopResourceBound::Unknown) {}

  // Returns the number of memory reads.
  unsigned getNumMemReads() const { return IntMemReads + FPMemReads; }

  // Returns the number of memory writes.
  unsigned getNumMemWrites() const { return IntMemWrites + FPMemWrites; }

  // Returns the number of integer memory refs.
  unsigned getNumIntMemRefs() const { return IntMemReads + IntMemWrites; }

  // Returns the cost of integer memory refs.
  unsigned getIntMemRefsCost() const { return IntCost * getNumIntMemRefs(); }

  // Returns the number of FP memory refs.
  unsigned getNumFPMemRefs() const { return FPMemReads + FPMemWrites; }

  // Returns the cost of FP memory refs.
  unsigned getFPMemRefsCost() const { return FPCost * getNumFPMemRefs(); }

  // Returns the total number of memory references.
  unsigned getNumMemRefs() const {
    return getNumIntMemRefs() + getNumFPMemRefs();
  }

  // Returns the cost of all memory refs.
  unsigned getMemRefsCost() const {
    return getIntMemRefsCost() + getFPMemRefsCost();
  }

  // Returns the number of integer operations.
  unsigned getNumIntOps() const { return IntOps; }

  // Returns the cost of integer operations.
  unsigned getIntOpsCost() const { return IntCost * getNumIntOps(); }

  // Returns the number of FP operations.
  unsigned getNumFPOps() const { return FPOps; }

  // Returns the cost of FP operations.
  unsigned getFPOpsCost() const { return FPCost * getNumFPOps(); }

  // Returns the cost of all operations.
  unsigned getNumOps() const { return getNumIntOps() + getNumFPOps(); }

  // Returns the cost of integer operations.
  unsigned getOpsCost() const { return getIntOpsCost() + getFPOpsCost(); }

  // Returns true if loop resource is memory bound.
  bool isMemBound() const { return (Bound == LoopResourceBound::Memory); }

  // Returns true if loop resource is FP operations bound.
  bool isFPBound() const { return (Bound == LoopResourceBound::FP); }

  // Returns true if loop resource is integer operations bound.
  bool isIntBound() const { return (Bound == LoopResourceBound::Int); }

  // Returns true if loop resource bound cannot be determined.
  bool isUnkownBound() const { return (Bound == LoopResourceBound::Unknown); }

  // Computes loop resource for the passed in loop in the current object.
  void compute(HIRLoopResource &HLR, const HLLoop *Lp);

  // Prints the loop resource.
  void print(formatted_raw_ostream &OS, const HLLoop *Lp) const;
};

class HIRLoopResource final : public HIRAnalysisPass {
private:
  // Maintains the resource information in a map for the loops.
  DenseMap<const HLLoop *, LoopResourceInfo> ResourceMap;

  /// \brief Computes the loop resource by traversing the instructions
  /// inside the body, preheader and postexit.
  const LoopResourceInfo &computeLoopResource(const HLLoop *Loop);

protected:
  // Prints analyis results for loop.
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

  /// \brief Returns the loop resource of the specified loop. Transformations
  /// should call this method to access the loop resource.
  const LoopResourceInfo &getLoopResource(const HLLoop *Loop);

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HIRAnalysisPass *AP) {
    return AP->getHIRAnalysisID() == HIRAnalysisPass::HIRLoopResourceVal;
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
