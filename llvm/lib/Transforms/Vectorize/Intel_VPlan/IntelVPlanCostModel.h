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
  explicit VPlanVLSCostModel(const VPlanCostModel &VPCM,
                             const TargetTransformInfo &TTI, LLVMContext &Cntx)
      : OVLSCostModel(TTI, Cntx), VPCM(VPCM) {}
  /// Generic function to get a cost of OVLSInstruction. Internally it has
  /// dispatch functionality to return cost for OVLSShuffle
  virtual uint64_t getInstructionCost(const OVLSInstruction *I) const final;

  /// Return cost of a gather or scatter instruction.
  virtual uint64_t getGatherScatterOpCost(const OVLSMemref &Memref) const final;

protected:
  const VPlanCostModel &VPCM;
};
#endif // INTEL_CUSTOMIZATION

class VPlanCostModel {
#if INTEL_CUSTOMIZATION
  // To access getMemInstValueType.
  friend class VPlanVLSAnalysisHIR;
  friend class VPlanVLSCostModel;
#endif // INTEL_CUSTOMIZATION
public:
#if INTEL_CUSTOMIZATION
  VPlanCostModel(const VPlan *Plan, const unsigned VF,
                 const TargetTransformInfo *TTI,
                 const TargetLibraryInfo *TLI,
                 const DataLayout *DL,
                 VPlanVLSAnalysis *VLSA)
    : Plan(Plan), VF(VF), TLI(TLI), DL(DL), VLSA(VLSA) {
    VPTTI = std::make_unique<VPlanTTIWrapper>(*TTI);
    if (VLSA)
      // FIXME: Really ugly to get LLVMContext from VLSA, which may not
      // even exist, but so far there's no other simple way to pass it here.
      // Unlike LLVM IR VPBB has no LLVMContext, because Type is not yet
      // implemented
      VLSCM = std::make_shared<VPlanVLSCostModel>(*this, *TTI, VLSA->getContext());

    // CallVecDecisions analysis invocation.
    VPlanCallVecDecisions CallVecDecisions(*const_cast<VPlan *>(Plan));
    // Pass native TTI into CallVecDecisions analysis.
    CallVecDecisions.run(VF, TLI, &VPTTI->getTTI());

    // Compute SVA results for current VPlan in order to compute cost accurately
    // in CM.
    const_cast<VPlan *>(Plan)->runSVA(VF, TLI);
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
  virtual unsigned getCost();
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
#if INTEL_CUSTOMIZATION
  VPlanVLSAnalysis *VLSA;
  std::shared_ptr<VPlanVLSCostModel> VLSCM;
#endif // INTEL_CUSTOMIZATION

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  std::string getCostNumberString(unsigned Cost) const {
    if (Cost == UnknownCost)
      return std::string("Unknown");
    return std::to_string(Cost);
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
  virtual unsigned getCostForVF(const VPInstruction *VPInst, unsigned VF);
  virtual unsigned getCost(const VPBasicBlock *VPBB);
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

  // TODO: Remove SLP framework.
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
};

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODEL_H
