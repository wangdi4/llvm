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

// VPlanTTICostModel defines interface and its implementation, that is to be
// used by Cost Model heuristics.  Also all Heuristics independent code goes
// into VPlanTTICostModel.
//
// We don't create objects of this type, a general Cost Model type object
// rather passed to Heuristics.
class VPlanTTICostModel {
public:
  static constexpr unsigned UnknownCost = -1u;
  unsigned getTTICost(const VPInstruction *VPInst);
  // getLoadStoreCost(LoadOrStore, Alignment, VF) interface returns the Cost
  // of Load/Store VPInstruction given VF and Alignment on input.
  unsigned getLoadStoreCost(
    const VPInstruction *LoadOrStore, Align Alignment, unsigned VF);
  // The Cost of Load/Store using underlying IR/HIR Inst Alignment.
  unsigned getLoadStoreCost(const VPInstruction *VPInst, unsigned VF);

  const VPlanVector * const Plan;
  const unsigned VF;
  const TargetLibraryInfo * const TLI;
  const DataLayout * const DL;
  const VPlanTTIWrapper VPTTI;

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

  /// \Returns true if VPInst is part of an optimized VLS group.
  bool isOptimizedVLSGroupMember(const VPInstruction *VPInst) const {
    return getOptimizedVLSGroupData(VPInst, VLSA, Plan).hasValue();
  }

protected:
#if INTEL_CUSTOMIZATION
  VPlanVLSAnalysis *VLSA;
#endif // INTEL_CUSTOMIZATION

  VPlanTTICostModel(const VPlanVector *Plan, const unsigned VF,
                    const TargetTransformInfo *TTI,
                    const TargetLibraryInfo *TLI, const DataLayout *DL,
                    VPlanVLSAnalysis *VLSA)
      : Plan(Plan), VF(VF), TLI(TLI), DL(DL), VPTTI(*TTI, *DL), VLSA(VLSA) {

    // CallVecDecisions analysis invocation.
    VPlanCallVecDecisions CallVecDecisions(*const_cast<VPlanVector *>(Plan));

    // Pass native TTI into CallVecDecisions analysis.
    CallVecDecisions.runForVF(VF, TLI, &VPTTI.getTTI());

    // Compute SVA results for current VPlan in order to compute cost
    // accurately in CM.
    const_cast<VPlanVector *>(Plan)->runSVA();
  }

  // We prefer protected dtor over virtual one as there is no plan to
  // manipulate with objects through VPlanTTICostModel type handler.
  ~VPlanTTICostModel() {};

  // Consolidates the code that gets the cost of one operand or two operands
  // arithmetics instructions.  For one operand case Op2 is expected to be
  // null.  Op1 is never expected to be null.
  //
  // TODO:
  // This method should not be virtual once VPInst-level heuristics are
  // introduced and special handling logic is moved from this routine
  // to standalone heuristic.
  virtual unsigned getArithmeticInstructionCost(const unsigned Opcode,
                                                const VPValue *Op1,
                                                const VPValue *Op2,
                                                const Type *ScalarTy,
                                                const unsigned VF);

private:
  // These utilities are private for the class instead of being defined as
  // static functions because they need access to underlying Inst/HIRData in
  // VPInstruction via the friends relation VPInstruction.
  //
  // Also, they won't be necessary if we had VPType for each VPValue.
  static Type *getMemInstValueType(const VPInstruction *VPInst);
  static unsigned getMemInstAddressSpace(const VPInstruction *VPInst);

  // The utility checks whether the Cost Model can assume that 32-bit indexes
  // will be used instead of 64-bit indexes for gather/scatter HW instructions.
  unsigned getLoadStoreIndexSize(const VPInstruction *VPInst) const;

  // Calculates the sum of the cost of extracting VF elements of Ty type
  // or the cost of inserting VF elements of Ty type into a vector.
  unsigned getInsertExtractElementsCost(unsigned Opcode,
                                        Type *Ty, unsigned VF);

  // Get intrinsic corresponding to provided call that is expected to be
  // vectorized using SVML version. This is purely meant for internal cost
  // computation purposes and not for general purpose functionality (unlike
  // llvm::getIntrinsicForCallSite).
  // TODO: This is a temporary solution to avoid CM from choosing inefficient
  // VFs, complete solution would be to introduce a general scheme in TTI to
  // provide costs for different SVML calls. Check JIRA : CMPLRLLVM-23527.
  Intrinsic::ID getIntrinsicForSVMLCall(const VPCallInstruction *VPCall) const;

  // Get Cost for Intrinsic (ID) call.
  unsigned getIntrinsicInstrCost(
    Intrinsic::ID ID, const VPCallInstruction *VPCall, unsigned VF);

  // Returns TTI Cost in VPInst arbitrary VF.
  unsigned getTTICostForVF(const VPInstruction *VPInst, unsigned VF);
};

// Class HeuristicsList is designed to hold Heuristics objects. It is a
// template class and should be specialized with Heuristics types.  An object
// of HeuristicsList<Scope, HTy1, ... , HTyn> creates Heuristics objects of
// specified types HTy1 ... HTyn on the specified Scope, which in turn can be
// VPlan, VPBlock or VPInstruction.
//
// If a heuristics is created on scope 'Scope' it means that such heuristic is
// applied on that scope (i.e. for VPlan/VPBlock/VPInstruction).
// HeuristicsList implements facility to apply all heuristics it contains.
template <typename Scope, typename... Ts> class HeuristicsList;

// Recursive declaration for arbitrary number of Heuristics types on input.
template <typename Scope, typename HTy, typename... HTys>
class HeuristicsList<Scope, HTy, HTys...>
    : public HeuristicsList<Scope, HTys...> {
  using Base = HeuristicsList<Scope, HTys...>;
  HTy H;
public:
  HeuristicsList() = delete;
  HeuristicsList(VPlanTTICostModel *CM) : Base(CM), H(CM) {};
  void apply(unsigned TTICost, unsigned &Cost,
             Scope *S, raw_ostream *OS = nullptr) const {
    H.apply(TTICost, Cost, S, OS);
    // Once any heuristics in the pipeline returns Unknown cost
    // return it immediately.
    if (Cost == VPlanTTICostModel::UnknownCost)
      return;
    this->Base::apply(TTICost, Cost, S, OS);
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Note that ScopeTy and Scope are different types.
  template <typename ScopeTy>
  void dump(raw_ostream &OS, ScopeTy *S) const {
    H.dump(OS, S);
    this->Base::dump(OS, S);
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// Specialization for empty Heuristics types list.
template <typename Scope>
class HeuristicsList<Scope> {
public:
  HeuristicsList() = delete;
  HeuristicsList(VPlanTTICostModel *CM) {}
  // There is no heuristics to apply, thus just be transparent.
  void apply(unsigned TTICost, unsigned &Cost,
             Scope *S, raw_ostream *OS = nullptr) const {}
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // No heuristics to emit dump.
  template <typename ScopeTy>
  void dump(raw_ostream &OS, ScopeTy *S) const {}
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// TODO: VPlanCostModel class is temporal, for transition to template based
// Cost Model types.
class VPlanCostModel : public VPlanTTICostModel {
public:
  VPlanCostModel(const VPlanVector *Plan, const unsigned VF,
                 const TargetTransformInfo *TTI, const TargetLibraryInfo *TLI,
                 const DataLayout *DL, VPlanVLSAnalysis *VLSA = nullptr)
    : VPlanTTICostModel(Plan, VF, TTI, TLI, DL, VLSA),
      VPAA(*Plan->getVPSE(), *Plan->getVPVT(), VF),
      HeuristicsPipeline(this) {}
  // Get Cost for VPlan with specified peeling.
  unsigned getCost(VPlanPeelingVariant *PeelingVariant = nullptr);
  unsigned getMemInstAlignment(const VPInstruction *VPInst) const;
  virtual unsigned getLoadStoreCost(
    const VPInstruction *LoadOrStore, Align Alignment, unsigned VF) {
    return VPlanTTICostModel::getLoadStoreCost(LoadOrStore, Alignment, VF);
  }
  virtual unsigned getBlockRangeCost(const VPBasicBlock *Begin,
                                     const VPBasicBlock *End);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS, const std::string &Header);
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  virtual ~VPlanCostModel() {}

protected:
  VPlanPeelingVariant* DefaultPeelingVariant = nullptr;
  VPlanAlignmentAnalysis VPAA;

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

  virtual unsigned getArithmeticInstructionCost(const unsigned Opcode,
                                                const VPValue *Op1,
                                                const VPValue *Op2,
                                                const Type *ScalarTy,
                                                const unsigned VF) override {
    return VPlanTTICostModel::getArithmeticInstructionCost(
      Opcode, Op1, Op2, ScalarTy, VF);
  }
  virtual unsigned getCost(const VPInstruction *VPInst);
  unsigned getCost(const VPBasicBlock *VPBB);
  // Return TTI contribution to the whole cost.
  unsigned getTTICost();

  virtual unsigned getLoadStoreCost(const VPInstruction *VPInst, unsigned VF) {
    return VPlanTTICostModel::getLoadStoreCost(VPInst, VF);
  }

  // Temporal virtual methods to invoke apply facilities on HeuristicsPipeline.
  virtual void applyHeuristicsPipeline(
    unsigned TTICost, unsigned &Cost,
    const VPlanVector *Plan, raw_ostream *OS = nullptr) const {
    HeuristicsPipeline.apply(TTICost, Cost, Plan, OS);
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Temporal virtual methods to invoke dump facilities on HeuristicsPipeline.
  virtual void dumpHeuristicsPipeline(raw_ostream &OS,
                                      const VPlanVector *Plan) const {
    HeuristicsPipeline.dump(OS, Plan);
  }
  virtual void dumpHeuristicsPipeline(raw_ostream &OS,
                                      const VPBasicBlock *VPBB) const {
    HeuristicsPipeline.dump(OS, VPBB);
  }
  virtual void dumpHeuristicsPipeline(raw_ostream &OS,
                                      const VPInstruction *VPInst) const {
    HeuristicsPipeline.dump(OS, VPInst);
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
private:
  // Apply all heuristics scheduled in the pipeline for the current VPlan
  // and return modified input Cost.
  unsigned applyHeuristics(unsigned Cost);

  // Heuristics list type specific to base cost model.
  HeuristicsList<const VPlanVector,
    VPlanCostModelHeuristics::HeuristicSLP,
    VPlanCostModelHeuristics::HeuristicSpillFill> HeuristicsPipeline;
};

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODEL_H
