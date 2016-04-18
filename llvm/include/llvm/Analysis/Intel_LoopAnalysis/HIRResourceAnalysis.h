//===----------- LoopResource.h - Provides Loop Resource ---------*- C++-*-===//
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

#include "llvm/Analysis/Intel_LoopAnalysis/HIRAnalysisPass.h"

#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/raw_ostream.h"

#include <map>

namespace llvm {

namespace loopopt {

class CanonExpr;
class HLInst;
class HLLoop;

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

// Captures all the loop resource information.
struct LoopResourceInfo {

  // Integer operations
  uint64_t IntOps;
  // Floating point operations
  uint64_t FPOps;
  // Integer loads/reads
  uint64_t IntMemReads;
  // Integer stores/writes
  uint64_t IntMemWrites;
  // Floating point loads/reads.
  uint64_t FPMemReads;
  // Floating point stores/writes.
  uint64_t FPMemWrites;
  // Indicates the loop resource bound .
  LoopResourceBound Bound;

  LoopResourceInfo()
      : IntOps(0), FPOps(0), IntMemReads(0), IntMemWrites(0), FPMemReads(0),
        FPMemWrites(0), Bound(LoopResourceBound::Unknown) {}

  void clear() {
    IntOps = FPOps = IntMemReads = IntMemWrites = FPMemReads = FPMemWrites = 0;
    Bound = LoopResourceBound::Unknown;
  }

  // Adds the loop resource LR to this resource information.
  void add(LoopResourceInfo &LR) {
    IntOps += LR.IntOps;
    FPOps += LR.FPOps;
    IntMemReads += LR.IntMemReads;
    IntMemWrites += LR.IntMemWrites;
    FPMemReads += LR.FPMemReads;
    FPMemWrites += LR.FPMemWrites;
  }

  // Multiplies the current resource with specified multiplier.
  // This is useful when trip count needs to be multiplied.
  void multiply(int64_t Multiplier) {
	assert((Multiplier!=0) && " Multiplier is zero.");
    IntOps *= Multiplier;
    FPOps *= Multiplier;
    IntMemReads *= Multiplier;
    IntMemWrites *= Multiplier;
    FPMemReads *= Multiplier;
    FPMemWrites *= Multiplier;
  }

  // Returns the total number of memory references.
  uint64_t getNumMemoryRefs() const {
    return (IntMemReads + IntMemWrites + FPMemReads + FPMemWrites);
  }

  // Returns the total number of operations.
  uint64_t getNumOps() const { return (IntOps + FPOps); }

  void dump() const {
#ifndef NDEBUG
    formatted_raw_ostream OS(dbgs());
    print(OS);
#endif
  }

  // Prints the loop resource.
  void print(formatted_raw_ostream &OS) const {
#ifndef NDEBUG
    OS << "Integer Memory Reads: " << IntMemReads << "\n";
    OS << "Integer Memory Writes: " << IntMemWrites << "\n";
    OS << "Integer Operations: " << IntOps << "\n";
    OS << "Floating Point Reads: " << FPMemReads << "\n";
    OS << "Floating Point Writes: " << FPMemWrites << "\n";
    OS << "Floating Point Operations: " << FPOps << "\n";
    switch (Bound) {
    case LoopResourceBound::Memory:
      OS << "Memory Bound \n";
      break;
    case LoopResourceBound::Int:
      OS << "Integer Bound \n";
      break;
    case LoopResourceBound::FP:
      OS << "Floating Point Bound \n";
      break;
    case LoopResourceBound::Unknown:
      OS << "Unknown Bound \n";
      break;
    default:
      llvm_unreachable(" Loop should be classified in a bound.");
    }
#endif
  }
};

class HIRResourceAnalysis final : public HIRAnalysisPass {

private:
  // Symbolic constant to denote unknown 'N' trip count.
  const unsigned SymbolicConst = 10;

  // Costs based on type of operation.
  // This information can be tuned if we specific target information.
  const unsigned IntOpCost = 1;
  const unsigned FPOpCost = 2;
  const unsigned IntMemReadCost = 2;
  const unsigned IntMemWriteCost = 2;
  const unsigned FPMemReadCost = 4;
  const unsigned FPMemWriteCost = 4;

  // Maintains the resource information in a map for the loops.
  SmallDenseMap<const HLLoop *, LoopResourceInfo *, 16> ResourceMap;

  // First argument is the HLLoop and second argument
  // tells if the loop was modified or not. True indicates it was
  // modified and False indicates no change inside this loop.
  // When there is no change, all children have valid resource information.
  SmallDenseMap<const HLLoop *, bool, 64> LoopModificationMap;

  /// Visitor to traverse the instructions inside the loop.
  struct LoopVisitor;

  /// \brief Adds the child loop resource to the parent.
  void addLoopResource(const HLLoop *ParentLoop, const HLLoop *ChildLoop);

  /// \brief Classifies the loop to determine if it is memory bound
  /// or integer bound.
  void classifyLoopResource(LoopResourceInfo *LRInfo);

  /// \brief Computes the loop resource by traversing the instructions
  /// inside the body, preheader and postexit.
  void computeLoopResource(const HLLoop *Loop);

  /// \brief Returns the trip count of the loop. If the trip is large
  /// or unknown, we return a constant symbolic value.
  int64_t getTripCount(const HLLoop *Loop);

  /// \brief Returns true if Ty is int type.
  bool isIntTy(const Type *Ty);

  /// \brief Returns true if Ty is floating point type.
  bool isFloatTy(const Type *Ty);

  /// \brief Returns true if the type is supported by resource analysis.
  bool isTypeSupported(const Type *Ty);

  /// \brief Returns true if the loop is modified or the loop doesn't
  /// exist in the map.
  bool isLoopModified(const HLLoop *Loop) const;

  /// \brief Counts the operations inside the canon expr.
  void processCanonExpr(const CanonExpr *CE, LoopResourceInfo *LRInfo);

  /// \brief Processes the instruction and updates the loop resource
  /// appropriately.
  void processInstruction(const HLInst *HLInst, LoopResourceInfo *LRInfo);

  /// \brief Clears the resource map if it exists, otherwise it creates
  /// a new allocation.
  LoopResourceInfo *resetResourceMap(const HLLoop *Loop);

  /// \brief This method is used for verification purpose only during lit
  /// tests.
  void verifyAllLoops();

public:
  HIRResourceAnalysis()
      : HIRAnalysisPass(ID, HIRAnalysisPass::HIRResourceVal) {}
  static char ID;

  bool runOnFunction(Function &F) override;

  void print(raw_ostream &OS, const Module * = nullptr) const override;

  /// \brief Prints out the LoopResource Information.
  void printLoopResourceInfo(raw_ostream &OS, const HLLoop *L) const;

  void getAnalysisUsage(AnalysisUsage &AU) const;

  void releaseMemory() override;

  /// \brief This method will mark the loop and all its parent loops as
  /// modified. If loop changes, resources of the loop and all its parents loops
  /// needs to recomputed.
  void markLoopBodyModified(const HLLoop *Loop) override;

  /// TODO: Add this method to other analysis and test it.
  /// \brief This method will remove all the loop resource information from
  /// internal data structures.
  /// void forgetLoop(const HLLoop *Loop);

  /// \brief Returns the loop resource of the specified loop. Transformations
  /// should call this method to access the loop resource.
  const LoopResourceInfo *getLoopResource(const HLLoop *Loop);

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HIRAnalysisPass *AP) {
    return AP->getHIRAnalysisID() == HIRAnalysisPass::HIRResourceVal;
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
