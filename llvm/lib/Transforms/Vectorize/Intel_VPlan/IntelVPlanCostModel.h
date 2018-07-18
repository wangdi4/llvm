//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2018 Intel Corporation. All rights reserved.
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

namespace llvm {
class DataLayout;
class TargetTransformInfo;
class Type;
class Value;
class raw_ostream;

namespace vpo {
class VPlan;
class VPBasicBlock;
class VPBlockBase;
class VPInstruction;
#if INTEL_CUSTOMIZATION
class VPlanVLSAnalysis;
#endif // INTEL_CUSTOMIZATION

class VPlanCostModel {
#if INTEL_CUSTOMIZATION
  // To access getMemInstValueType.
  friend class VPlanVLSAnalysisHIR;
#endif // INTEL_CUSTOMIZATION
public:
#if INTEL_CUSTOMIZATION
  VPlanCostModel(const VPlan *Plan, const unsigned VF,
                 const TargetTransformInfo *TTI, const DataLayout *DL,
                 VPlanVLSAnalysis *VLSA)
      : Plan(Plan), VF(VF), TTI(TTI), DL(DL), VLSA(VLSA) {}
#else
  VPlanCostModel(const VPlan *Plan, const unsigned VF,
                 const TargetTransformInfo *TTI, const DataLayout *DL)
      : Plan(Plan), VF(VF), TTI(TTI), DL(DL) {}
#endif // INTEL_CUSTOMIZATION
  virtual unsigned getCost(const VPInstruction *VPInst) const;
  virtual unsigned getCost(const VPBasicBlock *VPBB) const;
  virtual unsigned getCost() const;
  void print(raw_ostream &OS) const;
  virtual ~VPlanCostModel() {}

protected:
  const VPlan *Plan;
  unsigned VF;
  const TargetTransformInfo *TTI;
  const DataLayout *DL;
#if INTEL_CUSTOMIZATION
  VPlanVLSAnalysis *VLSA;
#endif // INTEL_CUSTOMIZATION

  static constexpr unsigned UnknownCost = static_cast<unsigned>(-1);

  void printForVPBlockBase(raw_ostream &OS, const VPBlockBase *VPBlock) const;
  virtual unsigned getCost(const VPBlockBase *VPBlock) const;

  // These utilities are private for the class instead of being defined as
  // static functions because they need access to underlying Inst/HIRData in
  // VPInstruction via the friends relation between VPlanCostModel and
  // VPInstruction.
  //
  // Also, they won't be necessary if we had VPType for each VPValue.
  static Type *getMemInstValueType(const VPInstruction *VPInst);
  static unsigned getMemInstAddressSpace(const VPInstruction *VPInst);
  static Type *getVectorizedType(const Type *BaseTy, unsigned VF);
  static Value *getGEP(const VPInstruction *VPInst);

  /// \Returns the alignment of the load/store \p VPInst.
  ///
  /// This method guarantees to never return zero by returning default alignment
  /// for the base type in case of zero alignment in the underlying IR, so this
  /// method can freely be used even for widening of the \p VPInst.
  unsigned getMemInstAlignment(const VPInstruction *VPInst) const;
};

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODEL_H
