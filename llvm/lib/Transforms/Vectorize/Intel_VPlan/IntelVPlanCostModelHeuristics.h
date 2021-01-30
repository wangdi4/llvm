//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines VPlanCostModelHeuristics namespace and classes within
/// this namespace that are used for cost estimations performed by VPlan Cost
/// Model.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODELHEURISTICS_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODELHEURISTICS_H

#include "IntelVPlanUtils.h"

namespace llvm {

namespace vpo {

class VPlanTTICostModel;

namespace VPlanCostModelHeuristics {
// The base class to inherit from the Cost Model heuristics that we apply on
// whole VPlan.
class HeuristicBase {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  std::string Name;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

protected:
  VPlanTTICostModel *CM;
  // Some utility stuff that is referenced within heuristics quite frequently.
  const VPlanVector *Plan;
  unsigned VF;
  unsigned UnknownCost;
  HeuristicBase(VPlanTTICostModel *CM, std::string Name);

public:
  virtual ~HeuristicBase() = default;
  // Applies the heuristic on the current VPlan to calculate new adjusted Cost.
  // This is the only required for each heuristics method to implement as it is
  // invoked for each heuristic from within VPlan Cost Model.
  virtual unsigned applyOnPlan(unsigned Cost) const = 0;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  std::string getName() const {
    return Name;
  }
  // Returns attribute string to dump additionally to each VPInstruction
  // in CM debug dumps.
  virtual std::string getAttrString(const VPInstruction *VPInst) const {
    return "";
  }
  // Returns attribute string to dump additionally to each VPBasicBlock
  // in CM debug dumps.
  virtual std::string getAttrString(const VPBasicBlock *VPBlock) const {
    return "";
  }
  // Implements debug dump output for Heuristic entities.
  // Base implementation dumps heuristic name only.  Specifications are free
  // to dump more details that are specific to each heuristic.
  virtual void dump() const {
    dbgs() << "Heuristic name: " << getName() << '\n';
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// Heurstic that calculates the cost of VPlan vectorization when VPlan
// vectorization potentially blocks SLP vectorization.
//
// TODO:
// *SLP* stuff and related utilities below is temporal solution to workaround
// the problem of blocking SLP vectorizer by VPlan vectorization. Removing SLP
// heuristics altogether is a long term plan.  We need other methods of
// interaction between SLP and VPlan vectorizers or merge them into a single
// vectorizer.
class HeuristicSLP : public HeuristicBase {
  // Static utilities below are used in this heuristics and it is undesirable
  // to reuse them elsewhere.
  //
  // Returns RegDDRef of type Memref in case VPInst has it associated.
  // Returns nullptr otherwise.
  static const loopopt::RegDDRef* getHIRMemref(const VPInstruction *VPInst);

  // The set of constant controlling search of SLP pattern in VPlan.
  // Current implementation searches stores and loads to/from adjacent memory.
  // In particular there should be VPlanSLPLoadPatternSize loads AND
  // VPlanSLPStorePatternSize stores into consequent memory cells.
  //
  // SPL doesn't neccesary need VPlanSLPLoadPatternSize consecutive in memory
  // loads but we do this check instead of building data flow/dependency graph
  // that SLP builds.
  //
  // Also to limit compile time impact during search we assume that all memrefs
  // that make SLP pattern are in some relatively small window of indexes in
  // the vector of all memrefs.  The window size is controlled by
  // VPlanSLPSearchWindowSize.
  //
  static const unsigned VPlanSLPSearchWindowSize = 16;
  static const unsigned VPlanSLPLoadPatternSize = 4;
  static const unsigned VPlanSLPStorePatternSize = 2;
  // Searches for part of SLP pattern in input vector of HIR memrefs. In
  // particular the utility checks if there is PatternSize memrefs in
  // HIRMemrefs that access memory consequently.  The utility does not
  // distiguish loads or stores in input list.  It is caller responsibility to
  // form the list properly.  Returns true if found or false otherwise.
  static bool findSLPHIRPattern(
    SmallVectorImpl<const loopopt::RegDDRef*> &HIRMemrefs,
    unsigned PatternSize);
  // Forms vector of VPlanSLPSearchWindowSize elements out of HIRMemrefs input
  // vector and apply search on smaller vector with help of findSLPHIRPattern.
  static bool ProcessSLPHIRMemrefs(
    SmallVectorImpl<const loopopt::RegDDRef*> const &HIRMemrefs,
    unsigned PatternSize);
public:
  HeuristicSLP(VPlanTTICostModel *CM) : HeuristicBase(CM, "SLP breaking") {};
  unsigned applyOnPlan(unsigned Cost) const final;
};

// Heurstic that searches for Search Loop idioms within VPlan.
// Returns the cost of vectorizing SearchLoop if such loop is detected or
// unmodified Cost otherwise.
class HeuristicSearchLoop : public HeuristicBase {
public:
  HeuristicSearchLoop(VPlanTTICostModel *CM) :
    HeuristicBase(CM, "SearchLoop Idiom") {};
  unsigned applyOnPlan(unsigned Cost) const final;
};

// Heurstic that calculates Spill/Fill cost.
// Implements basic register pressure calculation pass.
// Calculates the pressure of Vector Instructions only if VectorInsts is
// true. Calculates scalar instructions pressure otherwise.
class HeuristicSpillFill : public HeuristicBase {
  // LiveValues map contains the liveness of the given instruction multiplied
  // by its legalization factor.  The map contains LiveOut values for the
  // block on input of private applyOnPlan(Block, LiveValues, VectorRegsPressure)
  // and the map is updated by this method and contains LiveIn values after the
  // call.
  using LiveValuesTy = DenseMap<const VPInstruction*, int>;
  unsigned operator()(const VPBasicBlock *VPBB,
                      LiveValuesTy &LiveValues,
                      bool VectorRegsPressure) const;
public:
  HeuristicSpillFill(VPlanTTICostModel *CM) :
    HeuristicBase(CM, "Spill/Fill") {};
  unsigned applyOnPlan(unsigned Cost) const final;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  std::string getAttrString(const VPBasicBlock *VPBB) const final {
    LiveValuesTy LiveValues;
    std::string ReturnStr;
    unsigned ScalSpillFillCost = (*this)(VPBB, LiveValues, false);
    if (ScalSpillFillCost > 0)
      ReturnStr = "Block Scalar spill/fill approximate cost (not included "
        "into total cost): " + std::to_string(ScalSpillFillCost);

    if (VF > 1) {
      LiveValues.clear();
      unsigned VecSpillFillCost = (*this)(VPBB, LiveValues, true);
      if (VecSpillFillCost > 0) {
        if (ReturnStr != "")
          ReturnStr += '\n';
        ReturnStr += "Block Vector spill/fill approximate cost (not "
          "included into total cost): " + std::to_string(VecSpillFillCost);
      }
    }
    return ReturnStr;
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// Heuristic that calculates extra cost due to Gather/Scatter.
// HeuristicGatherScatter's interfaces calculate the sum of TTI costs of
// load and store instructions that are not unit load or store (i.e.
// most likely are implemented with gather or scatter HW instructions
// or just serialized).
class HeuristicGatherScatter : public HeuristicBase {
  unsigned operator()(const VPInstruction *VPInst) const;
  unsigned operator()(const VPBasicBlock *VPBlock) const;
public:
  HeuristicGatherScatter(VPlanTTICostModel *CM) :
    HeuristicBase(CM, "Gather/Scatter") {};
  unsigned applyOnPlan(unsigned Cost) const final;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  std::string getAttrString(const VPInstruction *VPInst) const final {
    if ((*this)(VPInst) > 0)
      return "GS";
    return HeuristicBase::getAttrString(VPInst);
  }
  std::string getAttrString(const VPBasicBlock *VPBB) const final {
    unsigned GatherScatterCost = (*this)(VPBB);
    if (GatherScatterCost > 0)
      return
        "total cost includes GS Cost: " + std::to_string(GatherScatterCost);
    return HeuristicBase::getAttrString(VPBB);
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// Heurstic that detects PSADBW patterns and adjusts the cost appropriately.
// It returns the Cost minus the sum of costs of all PSADWB patterns in VPlan
// or zero if the sum is greater.
// Also populates PsadbwPatternInsts with pattern instructions.
// TODO:
// The main method should return Cost of psadbw instruction instead of zero.
class HeuristicPsadbw : public HeuristicBase {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // PsadbwPatternInsts holds all instructions that are part of any PSADBW
  // pattern.  Used by dumping facilities only.
  mutable SmallPtrSet<const VPInstruction*, 32> PsadbwPatternInsts;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  // Checks for PSADWB pattern starting SelInst and updates
  // CurrPsadbwPatternInsts argument with instructions forming PSADWB pattern
  // based on SelInst.
  //
  // Returns true if pattern is found, false otherwise.
  bool checkPsadwbPattern(
    const VPInstruction *SelInst,
    SmallPtrSetImpl<const VPInstruction*> &CurrPsadbwPatternInsts) const;
public:
  HeuristicPsadbw(VPlanTTICostModel *CM) :
    HeuristicBase(CM, "psadbw pattern") {};
  // TODO:
  // The method should return Cost of psadbw instruction instead of zero.
  unsigned applyOnPlan(unsigned Cost) const final;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  std::string getAttrString(const VPInstruction *VPInst) const final {
    if (PsadbwPatternInsts.count(VPInst) > 0)
      return "PSADBW";
    return HeuristicBase::getAttrString(VPInst);
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

} // namespace VPlanCostModelHeuristics

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODELHEURISTICS_H
