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
///
/// \file
/// This file defines the VPlanCostModel class that is used for all cost
/// estimations performed in the VPlan-based vectorizer. The class provides both
/// the interfaces to calculate different costs (e.g., a single VPInstruction or
/// the whole VPlan) and the dedicated printing methods used exclusively for
/// testing purposes.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODEL_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODEL_H

#include "IntelVPlanCallVecDecisions.h"
#include "IntelVPlanCostModelHeuristics.h"
#include "IntelVPlanTTIWrapper.h"
#include "IntelVPlanVLSAnalysis.h"
#include "llvm/Analysis/Intel_OptVLS.h"

namespace llvm {
class DataLayout;
class TargetTransformInfo;
class Type;
class Value;
class raw_ostream;

namespace vpo {
class VPlan;
class VPBasicBlock;
class VPInstruction;

#if INTEL_CUSTOMIZATION
class VPlanVLSCostModel : public OVLSCostModel {
public:
  explicit VPlanVLSCostModel(unsigned VF,
                             const TargetTransformInfo &TTI, LLVMContext &Cntx)
      : OVLSCostModel(TTI, Cntx), VF(VF) {}
  /// Generic function to get a cost of OVLSInstruction. Internally it has
  /// dispatch functionality to return cost for OVLSShuffle
  virtual uint64_t getInstructionCost(const OVLSInstruction *I) const final;

  /// Return cost of a gather or scatter instruction.
  virtual uint64_t getGatherScatterOpCost(const OVLSMemref &Memref) const final;

protected:
  unsigned VF;
};
#endif // INTEL_CUSTOMIZATION

class VPlanCostModel {
#if INTEL_CUSTOMIZATION
  // To access getMemInstValueType.
  friend class VPlanVLSAnalysisHIR;
  // To access Plan, VF and others.
  friend class VPlanCostModelHeuristics::HeuristicBase;
  // To access VPTTI, getCost(), DL and others.
  friend class VPlanCostModelHeuristics::HeuristicSpillFill;
  friend class VPlanCostModelHeuristics::HeuristicGatherScatter;
  friend class VPlanCostModelHeuristics::HeuristicPsadbw;
#endif // INTEL_CUSTOMIZATION
public:
#if INTEL_CUSTOMIZATION
  VPlanCostModel(const VPlan *Plan, const unsigned VF,
                 const TargetTransformInfo *TTI,
                 const TargetLibraryInfo *TLI,
                 const DataLayout *DL,
                 VPlanVLSAnalysis *VLSA = nullptr)
    : Plan(Plan), VF(VF), TLI(TLI), DL(DL), VLSA(VLSA) {
    VPTTI = std::make_unique<VPlanTTIWrapper>(*TTI);

    // CallVecDecisions analysis invocation.
    VPlanCallVecDecisions CallVecDecisions(*const_cast<VPlan *>(Plan));
    // Pass native TTI into CallVecDecisions analysis.
    CallVecDecisions.run(VF, TLI, &VPTTI->getTTI());

    // Compute SVA results for current VPlan in order to compute cost accurately
    // in CM.
    const_cast<VPlan *>(Plan)->runSVA(VF, TLI);

    // Fill up HeuristicsPipeline with heuristics in the order they should be
    // applied.
    if (VF != 1)
      HeuristicsPipeline.push_back(
        std::make_unique<VPlanCostModelHeuristics::HeuristicSLP>(this));
    HeuristicsPipeline.push_back(
      std::make_unique<VPlanCostModelHeuristics::HeuristicSpillFill>(this));
  }
#else
  VPlanCostModel(const VPlan *Plan, const unsigned VF,
                 const TargetTransformInfo *TTI,
                 const TargetLibraryInfo *TLI,
                 const DataLayout *DL)
    : Plan(Plan), VF(VF), TLI(TLI), DL(DL) {
    VPTTI = std::make_unique<VPlanTTIWrapper>(*TTI);
  }
#endif // INTEL_CUSTOMIZATION
  unsigned getCost();
  virtual unsigned getLoadStoreCost(
    const VPInstruction *LoadOrStore, Align Alignment, unsigned VF);
  virtual unsigned getBlockRangeCost(const VPBasicBlock *Begin,
                                     const VPBasicBlock *End);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS, const std::string &Header);
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  virtual ~VPlanCostModel() {}

  static constexpr unsigned UnknownCost = -1u;

protected:
  const VPlan *Plan;
  unsigned VF;
  std::unique_ptr<VPlanTTIWrapper> VPTTI;
  const TargetLibraryInfo *TLI;
  const DataLayout *DL;
  // Keeps Heuristics queue in order they are applied.
  SmallVector<std::unique_ptr<VPlanCostModelHeuristics::HeuristicBase>, 8>
    HeuristicsPipeline;
#if INTEL_CUSTOMIZATION
  VPlanVLSAnalysis *VLSA;
#endif // INTEL_CUSTOMIZATION

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  std::string getCostNumberString(unsigned Cost) const {
    if (Cost == UnknownCost)
      return std::string("Unknown");
    return std::to_string(Cost);
  };
  // Returns string of VPInst attributes for CM debug dump.
  virtual std::string getAttrString(const VPInstruction *VPInst) const;
  // Printed in debug dumps.  Helps to distiguish base Cost Model dumps vs
  // inherited Cost Model dump.
  virtual std::string getHeaderPrefix() const {
    return "";
  };
  void printForVPInstruction(raw_ostream &OS, const VPInstruction *VPInst);
  void printForVPBasicBlock(raw_ostream &OS, const VPBasicBlock *VPBlock);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Consolidates the code that gets the cost of one operand or two operands
  // arithmetics instructions.  For one operand case Op2 is expected to be
  // null.  Op1 is never expected to be null.
  virtual unsigned getArithmeticInstructionCost(const unsigned Opcode,
                                                const VPValue *Op1,
                                                const VPValue *Op2,
                                                const Type *ScalarTy,
                                                const unsigned VF);
  virtual unsigned getCost(const VPInstruction *VPInst);
  unsigned getCostForVF(const VPInstruction *VPInst, unsigned VF);
  unsigned getCost(const VPBasicBlock *VPBB);
  // Return TTI contribution to the whole cost.
  unsigned getTTICost();
  virtual unsigned getLoadStoreCost(const VPInstruction *VPInst, unsigned VF);
  // Calculates the sum of the cost of extracting VF elements of Ty type
  // or the cost of inserting VF elements of Ty type into a vector.
  unsigned getInsertExtractElementsCost(unsigned Opcode,
                                        Type *Ty, unsigned VF);
  virtual unsigned getIntrinsicInstrCost(
    Intrinsic::ID ID, const CallBase &CB, unsigned VF,
    VPCallInstruction::CallVecScenariosTy VS);

  // These utilities are private for the class instead of being defined as
  // static functions because they need access to underlying Inst/HIRData in
  // VPInstruction via the friends relation between VPlanCostModel and
  // VPInstruction.
  //
  // Also, they won't be necessary if we had VPType for each VPValue.
  static Type *getMemInstValueType(const VPInstruction *VPInst);
  static unsigned getMemInstAddressSpace(const VPInstruction *VPInst);
  static Value *getGEP(const VPInstruction *VPInst);

  /// \Returns the alignment of the load/store \p VPInst.
  ///
  /// This method guarantees to never return zero by returning default alignment
  /// for the base type in case of zero alignment in the underlying IR, so this
  /// method can freely be used even for widening of the \p VPInst.
  unsigned getMemInstAlignment(const VPInstruction *VPInst) const;

  /// \Returns True iff \p VPInst is Unit Strided load or store.
  /// When load/store is strided NegativeStride is set to true if the stride is
  /// negative (-1 in number of elements) or to false otherwise.
  bool isUnitStrideLoadStore(const VPInstruction *VPInst,
                             bool &NegativeStride) const;

  // The utility checks whether the Cost Model can assume that 32-bit indexes
  // will be used instead of 64-bit indexes for gather/scatter HW instructions.
  unsigned getLoadStoreIndexSize(const VPInstruction *VPInst) const;

  // The method returns range with all Heuristics applicable to the current
  // VPlan.
  decltype(auto) heuristics() const {
    auto AsHeuristicBaseRef =
      [](decltype(*HeuristicsPipeline.begin()) P) ->
      const VPlanCostModelHeuristics::HeuristicBase & {
        return *P;
      };
    return map_range(HeuristicsPipeline, AsHeuristicBaseRef);
  }

private:
  // Get intrinsic corresponding to provided call that is expected to be
  // vectorized using SVML version. This is purely meant for internal cost
  // computation purposes and not for general purpose functionality (unlike
  // llvm::getIntrinsicForCallSite).
  // TODO: This is a temporary solution to avoid CM from choosing inefficient
  // VFs, complete solution would be to introduce a general scheme in TTI to
  // provide costs for different SVML calls. Check JIRA : CMPLRLLVM-23527.
  Intrinsic::ID getIntrinsicForSVMLCall(const VPCallInstruction *VPCall) const;

  // Apply all heuristics scheduled in the pipeline for the current VPlan
  // and return modified input Cost.
  unsigned applyHeuristics(unsigned Cost);
};

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODEL_H
