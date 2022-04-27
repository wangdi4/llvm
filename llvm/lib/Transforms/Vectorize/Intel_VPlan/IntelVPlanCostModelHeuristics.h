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

#include "llvm/Analysis/TargetTransformInfo.h"

#if INTEL_FEATURE_SW_ADVANCED

namespace llvm {
class OVLSGroup;

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
  HeuristicBase(VPlanTTICostModel *CM, std::string Name);

public:
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  std::string getName() const {
    return Name;
  }

  // Heuristic dumping facility.  By default does nothing, but actual heuristic
  // can redefine it at any Scope level (VPlan/VPBlock/VPInstruction). dump()
  // methods of each heuristics is invoked regardless of the scope level the
  // heuristics is declared for.
  template <typename ScopeTy>
  void dump(raw_ostream &OS, ScopeTy *Scope) const {}

  // Utilities for formatted print of cost increase/decrease due to Heuristics.
  // The output format depends on the Scope argument type. When ScopeTy is
  // VPlan the output is a full line with carriage return. If ScopeTy is
  // VPInstruction, the output line is short and w/o '\n' meaining that
  // several Heuristics are expected to report in a single line. Output format
  // for basic block is undefined and unused yet.
  void printCostChange(const VPInstructionCost &RefCost,
                       const VPInstructionCost &NewCost,
                       const VPlan *Scope, raw_ostream *OS) const;
  void printCostChange(const VPInstructionCost &RefCost,
                       const VPInstructionCost &NewCost,
                       const VPInstruction *Scope, raw_ostream *OS) const;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  // An heuristic can feature the VPlan level initialization routine which is
  // to be called explicitly by demand when CM is about to use the heuristic
  // to estimate the cost of whole VPlan. CM does not invoke initForVPlan when
  // it uses the heuristic to calculate the cost of VPBlock or VPInstruction.
  // This way we keep ctor light and don't spend time on possibly expensive
  // initialization in ctor when CM object is created to be used for
  // VPInstruction cost calculation only.
  // The default implementation do nothing.
  void initForVPlan() {}
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

  // Check that the given RednFinal VPInstruction is a reduction of the form
  //    double %vp51580 = reduction-final{fadd} double %vp48206 double ...
  // where %vp48206 is computed using the sequence
  //    double %vp23232 = phi
  //    ...
  //    double %vp1244 = load double* %vp44206
  //    ..
  //    i16 %vp48028 = load i16* %vp44246
  //    double %vp48064 = uitofp i16 %vp48028 to double
  //    double %vp48100 = fmul double %vp1244 double %vp48064
  //    double %vp48206 = fadd double %vp48100 double %vp23232
  //
  // The checks also include a negative stride one check for the $vp1244 load
  // and that %vp48028 load is a VLS optimization candidate. Loops that have
  // only reductions in the above form are considered better candidates for
  // SLP vectorization. We also restrict to exactly 4 such reductions and also
  // check that all above instructions are in the top loop header.
  bool checkForSLPRedn(const VPReductionFinal *RednFinal,
                       const VPBasicBlock *Header) const;

public:
  HeuristicSLP(VPlanTTICostModel *CM) : HeuristicBase(CM, "SLP breaking") {};
  void apply(const VPInstructionCost &TTICost, VPInstructionCost &Cost,
             const VPlanVector *Plan, raw_ostream *OS = nullptr) const;
};

// Heurstic that calculates Spill/Fill cost.
// Implements basic register pressure calculation pass.
// Calculates the pressure of Vector Instructions only if VectorInsts is
// true. Calculates scalar instructions pressure otherwise.
class HeuristicSpillFill : public HeuristicBase {
  // LiveValues map contains the liveness of the given instruction multiplied
  // by its legalization factor.  The map contains LiveOut values for the
  // block on input of private operator()(Block, LiveValues, VectorRegsPressure)
  // and the map is updated by this method and contains LiveIn values after the
  // call.
  using LiveValuesTy = DenseMap<const VPInstruction*, int>;
  VPInstructionCost operator()(const VPBasicBlock *VPBB,
                               LiveValuesTy &LiveValues,
                               bool VectorRegsPressure) const;
public:
  HeuristicSpillFill(VPlanTTICostModel *CM) :
    HeuristicBase(CM, "Spill/Fill") {};
  void apply(const VPInstructionCost &TTICost, VPInstructionCost &Cost,
             const VPlanVector *Plan, raw_ostream *OS = nullptr) const;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  using HeuristicBase::dump;
  void dump(raw_ostream &OS, const VPBasicBlock *VPBB) const;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// Heuristic that calculates extra cost due to Gather/Scatter.
// HeuristicGatherScatter's interfaces calculate the sum of TTI costs of
// load and store instructions that are not unit load or store (i.e.
// most likely are implemented with gather or scatter HW instructions
// or just serialized).
class HeuristicGatherScatter : public HeuristicBase {
  VPInstructionCost operator()(const VPInstruction *VPInst) const;
  VPInstructionCost operator()(const VPBasicBlock *VPBlock) const;
public:
  HeuristicGatherScatter(VPlanTTICostModel *CM) :
    HeuristicBase(CM, "Gather/Scatter") {};
  void apply(const VPInstructionCost &TTICost, VPInstructionCost &Cost,
             const VPlanVector *Plan, raw_ostream *OS = nullptr) const;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  using HeuristicBase::dump;
  void dump(raw_ostream &OS, const VPBasicBlock *VPBB) const;
  void dump(raw_ostream &OS, const VPInstruction *VPInst) const;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// Heurstic that detects PSADBW patterns and adjusts the cost appropriately.
// It returns the Cost minus the sum of costs of all PSADWB patterns in VPlan
// or zero if the sum is greater.
// Also populates PsadbwPatternInsts with pattern instructions.
// TODO:
// The main method should return Cost of psadbw instruction instead of zero.
class HeuristicPsadbw : public HeuristicBase {
  // SinglePatternInstsSet is the type for set holding VPInstructions that
  // belong to the same PSADBW pattern.
  using SinglePatternInstsSet = SmallPtrSet<const VPInstruction*, 32>;
  // PsadbwPatternInsts holds all instructions that are part of any PSADBW
  // pattern. It is a map of SinglePatternInstsSet's indexed by VPInstruction
  // which is carry out ADD instruction for the pattern.
  DenseMap<const VPInstruction*, SinglePatternInstsSet> PsadbwPatternInsts;

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
  void apply(const VPInstructionCost &TTICost, VPInstructionCost &Cost,
             const VPlanVector *Plan, raw_ostream *OS = nullptr) const;
  void initForVPlan();
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  using HeuristicBase::dump;
  void dump(raw_ostream &OS, const VPInstruction *VPInst) const;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// VPInstruction level heuristic that triggers on Integer DIV/REM instructions
// to take into account that the compiler uses corresponding SVML Vector
// entries to implement these operations.
class HeuristicSVMLIDivIRem : public HeuristicBase {
public:
  HeuristicSVMLIDivIRem(VPlanTTICostModel *CM) :
    HeuristicBase(CM, "IDiv/IRem") {};
  void apply(const VPInstructionCost &TTICost, VPInstructionCost &Cost,
             const VPInstruction *VPInst, raw_ostream *OS = nullptr) const;
};

// VPInstruction level heuristic that triggers on LOAD/STORE instructions
// and checks whether given instruction is a member of OVLS Group. The cost of
// such load/store can be lower if the group of loads/stores is emitted as
// vanilla loads/stores and shuffles rather than using gathers/scatters.
class HeuristicOVLSMember : public HeuristicBase {
  // ProcessedOVLSGroups holds the groups which Cost has already been taken into
  // account while traversing through VPlan during getCost().  This way we avoid
  // taking the same group price multiple times.
  // If Cost of OVLS group is better in terms of performance comparing to TTI
  // costs of instruction OVLS group would replace, then ProcessedOVLSGroups map
  // holds 'true' for this group.  Otherwise 'false' is stored in the map.
  using OVLSGroupMap = DenseMap<const OVLSGroup *, bool>;
  mutable OVLSGroupMap ProcessedOVLSGroups;

public:
  HeuristicOVLSMember(VPlanTTICostModel *CM) : HeuristicBase(CM, "OVLS") {};
  void apply(const VPInstructionCost &TTICost, VPInstructionCost &Cost,
             const VPInstruction *VPInst, raw_ostream *OS = nullptr) const;
};

} // namespace VPlanCostModelHeuristics

} // namespace vpo

} // namespace llvm
#endif // INTEL_FEATURE_SW_ADVANCED
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODELHEURISTICS_H
