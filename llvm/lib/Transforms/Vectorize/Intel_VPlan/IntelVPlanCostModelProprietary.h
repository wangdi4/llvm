//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODELPROPRIETARY_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODELPROPRIETARY_H

#include "IntelVPlanCostModel.h"
#include "IntelVPlanVLSAnalysis.h"

namespace llvm {

namespace vpo {

class VPlanCostModelProprietary : public VPlanCostModel {
public:
  explicit VPlanCostModelProprietary(const VPlan *Plan, unsigned VF,
                                     const TargetTransformInfo *TTI,
                                     const TargetLibraryInfo *TLI,
                                     const DataLayout *DL,
                                     VPlanVLSAnalysis *VLSA)
    : VPlanCostModel(Plan, VF, TTI, TLI, DL, VLSA) {
    VLSA->getOVLSMemrefs(Plan, VF);
  }

  virtual unsigned getCost() final;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS, const std::string &Header);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  ~VPlanCostModelProprietary() {}

private:
  virtual unsigned getCost(const VPInstruction *VPInst) final;
  virtual unsigned getCost(const VPBasicBlock *VPBB) final;
  virtual unsigned getLoadStoreCost(const VPInstruction *VPInst) {
    return getLoadStoreCost(VPInst, false /* Don't use VLS cost by default */);
  }
  unsigned getLoadStoreCost(const VPInstruction *VPInst,
                            const bool UseVLSCost);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printForVPInstruction(
    raw_ostream &OS, const VPInstruction *VPInst);
  void printForVPBasicBlock(
    raw_ostream &OS, const VPBasicBlock *VPBlock);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Implements basic register pressure calculation pass.
  // Bothers vector registers only currently.
  // LiveValues map contains the liveness of the given instruction multiplied
  // by its legalization factor.  The map contains LiveOut values for the block
  // on input of getSpillFillCost(Block, LiveValues) and the map is updated
  // by getSpillFillCost() and contains LiveIn values after the call.
  unsigned getSpillFillCost(
    const VPBasicBlock *VPBlock,
    DenseMap<const VPInstruction*, int /* legalization factor */> &LiveValues);
  unsigned getSpillFillCost(void);

  // Consolidates proprietary code that gets the cost of one operand or two
  // operands arithmetics instructions.
  virtual unsigned getArithmeticInstructionCost(const unsigned Opcode,
                                                const VPValue *Op1,
                                                const VPValue *Op2,
                                                const Type *ScalarTy,
                                                const unsigned VF) final;

  // *SLP* and related utilities below is temporal solution to workaround
  // the problem of blocking SLP vectorizer by VPlan vectorization.
  // Eventually SLP is expected to become a part of VPlan and/or share
  // implementation with VPlan.
  //
  // Since the static utilities are used in CM and it is undesirable to reuse
  // them elsewhere they are placed in CM class.
  //
  // Returns RegDDRef of type Memref in case VPInst has it associated.
  // Returns nullptr otherwise.
  static const RegDDRef* getHIRMemref(const VPInstruction *VPInst);

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
    SmallVectorImpl<const RegDDRef*> &HIRMemrefs, unsigned PatternSize);
  // Forms vector of VPlanSLPSearchWindowSize elements out of HIRMemrefs input
  // vector and apply search on smaller vector with help of findSLPHIRPattern.
  static bool ProcessSLPHIRMemrefs(
    SmallVectorImpl<const RegDDRef*> const &HIRMemrefs, unsigned PatternSize);

  // Returns true if there is an extra price for VPlan vectorization as it
  // potentially blocks SLP vectorization.
  //
  // Here is the logic of detection of basic SLP candidates.
  //
  // CheckForSLPExtraCost() gathers all store and load memrefs for each basic
  // block into separate vectors that are passes them individually to
  // ProcessSLPHIRMemrefs() for certain size pattern detection.
  //
  // ProcessSLPHIRMemrefs() extracts VPlanSLPSearchWindowSize instructions from
  // input vector starting at offset = 0 and ending at offset =
  // sizeof(input vector) - VPlanSLPSearchWindowSize.  ProcessSLPHIRMemrefs
  // forms a new vector out of extracted elements and pass it to
  // findSLPHIRPattern for analysis.
  //
  // findSLPHIRPattern pops the top element of input vector and analyzes
  // whether or not the rest of elements in input vector make sequential
  // memory access around the top element (i.e. access elements sequentially
  // before and/or after the memory that the top element accesses).
  // If pattern of sequential is not found the top element is discarded and
  // findSLPHIRPattern continues recursive search on reduced input vector until
  // the pattern is found or the size of input vector becomes less than
  // requested pattern size.
  //
  bool CheckForSLPExtraCost() const;

  // ProcessedOVLSGroups holds the groups which Cost has already been taken into
  // account while traversing through VPlan during getCost().  This way we avoid
  // taking the same group price multiple times.
  // If Cost of OVLS group is better in terms of performance comparing to TTI
  // costs of intruction OVLS group would replace, then ProcessedOVLSGroups map
  // holds 'true' for this group.  Otherwise 'false' is stored in the map.
  using OVLSGroupMap = DenseMap<const OVLSGroup *, bool>;
  OVLSGroupMap ProcessedOVLSGroups;

  // PsadbwPatternInsts holds all instructions that are part of any PSADBW
  // pattern.  Used by dumping facilities only.
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // PsadbwPatternInsts holds all instructions that are part of any PSADBW
  // pattern.  Used by dumping facilities only.
  SmallPtrSet<const VPInstruction*, 32> PsadbwPatternInsts;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Checks for PSADWB pattern starting SelInst and updates
  // CurrPsadbwPatternInsts argument with instructions forming PSADWB pattern
  // based on SelInst.
  //
  // Returns true if pattern is found, false otherwise.
  bool checkPsadwbPattern(
    const VPInstruction *SelInst,
    SmallPtrSetImpl<const VPInstruction*> &CurrPsadbwPatternInsts);

  // Return the sum of costs of all PSADWB patterns in VPlan.
  // Also populates PsadbwPatternInsts with pattern instructions.
  unsigned getPsadwbPatternCost();

  // getGatherScatterCost() interfaces calculate the sum of TTI costs of
  // load and store instructions that are not unit load or store (i.e.
  // most likely are implemented with gather or scatter HW instructions
  // or just serialized).
  //
  // Note that g/s cost is collected with a separate pass though VPlan, not
  // inside getCost(VPInstruction) routine in a dedicated accumulator, because
  // getCost(VPInstruction) can be called multiple times for the same
  // VPInstruction from various heuristics and we don't want getCost() to have
  // a side effect of updating acc.
  //
  // As a cost of such approach we need to keep details of walk through VPlan
  // in sync with walk though VPlan in main getCost() routines.  Such, the same
  // weights should be applied on blocks when block frequency info is deployed.
  unsigned getGatherScatterCost(const VPInstruction *VPInst);
  unsigned getGatherScatterCost(const VPBasicBlock *VPBlock);
  unsigned getGatherScatterCost();

  // FIXME: This is a temporary workaround until proper cost modeling is
  // implemented.
  //
  // To bail out if too many i1 operations are inside the loop as that (most
  // probably) represents complicated CFG and we need to use Basic Block
  // Frequency info to correctly calculate the cost. Until it's done, just
  // report high vector cost for loops with too many i1 instructions.
  unsigned NumberOfBoolComputations = 0;

  /// \Returns True iff \p VPInst is Unit Strided load or store.
  /// When load/store is strided NegativeStride is set to true if the stride is
  /// negative (-1 in number of elements) or to false otherwise.
  virtual bool isUnitStrideLoadStore(
    const VPInstruction *VPInst, bool &NegativeStride) const final;
};

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODELPROPRIETARY_H
