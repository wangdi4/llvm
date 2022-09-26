//===- IntelVPlanScalVecAnalysis.h ----------------------------------------===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
/// \file
///  This file provides VPlanScalVecAnalysis which is used to track and keep
///  information about expected code generation type for each VPInstruction
///  and its corresponding operands: scalar and/or vector.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLANSCALVECANALYSIS_H
#define LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLANSCALVECANALYSIS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"

#include <bitset>

namespace llvm {

class OperandBitsTy;
class raw_ostream;
class TargetLibraryInfo;

namespace vpo {

class VPlanVector;
class VPInstruction;
class VPPHINode;
class VPBasicBlock;

/// Base class for ScalVec Analysis. Defines interfaces and implements those
/// ones that are common for Scalar ScalVec and Vector ScalVec analyses.
class VPlanScalVecAnalysisBase {
protected:
  // Enum for different types of ScalVec analyses.
  enum class SVAType {
    Scalar,
    Vector,
  };

  // VPlan for which SVA results are computed for.
  VPlanVector *Plan;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS, const VPInstruction *VPI);
  void print(raw_ostream &OS, const VPBasicBlock *VPBB);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  VPlanScalVecAnalysisBase(SVAType Type) : Type(Type) {}

public:
  SVAType getSVAType() const { return Type; }

  virtual ~VPlanScalVecAnalysisBase() = default;

  // Getter interfaces for querying SVA results at instruction-level.
  virtual bool instNeedsVectorCode(const VPInstruction *Inst) const = 0;

  virtual bool instNeedsFirstScalarCode(const VPInstruction *Inst) const = 0;

  virtual bool instNeedsLastScalarCode(const VPInstruction *Inst) const = 0;

  // Getter interfaces for querying SVA results at operand-level.
  virtual bool operandNeedsVectorCode(const VPInstruction *Inst,
                                      unsigned OpIdx) const = 0;

  virtual bool operandNeedsFirstScalarCode(const VPInstruction *Inst,
                                           unsigned OpIdx) const = 0;

  virtual bool operandNeedsLastScalarCode(const VPInstruction *Inst,
                                          unsigned OpIdx) const = 0;

  // Getter interfaces for broadcast and extract patterns.
  virtual bool instNeedsBroadcast(const VPInstruction *Inst) const = 0;

  virtual bool instNeedsExtractFromFirstActiveLane(
    const VPInstruction *Inst) const = 0;

  virtual bool instNeedsExtractFromLastActiveLane(
    const VPInstruction *Inst) const = 0;

  // Getter interfaces for querying SVA results at return value level.
  virtual bool retValNeedsVectorCode(const VPInstruction *Inst) const = 0;

  virtual bool retValNeedsFirstScalarCode(const VPInstruction *Inst) const = 0;

  virtual bool retValNeedsLastScalarCode(const VPInstruction *Inst) const = 0;

  // TODO: Public setter/add interfaces are not needed as of now since bits are
  // computed internally within SVA. Introduce them if needed in the future.

  /// Central function to determine and propagate Scalar/Vector nature of
  /// VPInstructions for a given VPlan. The algorithm does a PO traversal on the
  /// CFG and works on use->def chain to propagate the nature of VPInstructions
  /// and its corresponding operands. Information about divergence property of a
  /// VPValue is also used from DA for decisions about an instruction's nature.
  /// Results of CallVecDecisions analysis are used to compute more accurate
  /// results for Call instructions and their argument operands.
  virtual void compute(VPlanVector *P) = 0;

  virtual void clear(void) = 0;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS);
  /// Print SVA result for given VPInstruction.
  virtual void printSVAKindForInst(raw_ostream &OS,
                                   const VPInstruction *VPI) const = 0;
  /// Print SVA result for given operand of VPInstruction.
  virtual void printSVAKindForOperand(raw_ostream &OS, const VPInstruction *VPI,
                                      unsigned OpIdx) const = 0;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

private:
  SVAType Type;
};

/// This class is responsible to find and keep whether a VPInstruction and its
/// operands must be widened or it can be kept scalar for VF > 1 cases. For
/// example consider a series of uniform instructions that can be computed as
/// scalar and broadcasted in vector context.
class VPlanScalVecAnalysis final : public VPlanScalVecAnalysisBase {
public:
  explicit VPlanScalVecAnalysis() :
    VPlanScalVecAnalysisBase(SVAType::Vector) {};
  ~VPlanScalVecAnalysis() = default;

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(VPlanScalVecAnalysisBase const *V) {
    return V->getSVAType() == SVAType::Vector;
  }

  // Getter interfaces for querying SVA results at instruction-level.
  bool instNeedsVectorCode(const VPInstruction *Inst) const override {
    return getSVABitsForInst(Inst).test(static_cast<unsigned>(SVAKind::Vector));
  }

  bool instNeedsFirstScalarCode(const VPInstruction *Inst) const override {
    return getSVABitsForInst(Inst).test(
        static_cast<unsigned>(SVAKind::FirstScalar));
  }

  bool instNeedsLastScalarCode(const VPInstruction *Inst) const override {
    return getSVABitsForInst(Inst).test(
        static_cast<unsigned>(SVAKind::LastScalar));
  }

  // Getter interfaces for querying SVA results at operand-level.
  bool operandNeedsVectorCode(const VPInstruction *Inst,
                              unsigned OpIdx) const override {
    return getSVABitsForOperand(Inst, OpIdx)
        .test(static_cast<unsigned>(SVAKind::Vector));
  }

  bool operandNeedsFirstScalarCode(const VPInstruction *Inst,
                                   unsigned OpIdx) const override {
    return getSVABitsForOperand(Inst, OpIdx)
        .test(static_cast<unsigned>(SVAKind::FirstScalar));
  }

  bool operandNeedsLastScalarCode(const VPInstruction *Inst,
                                  unsigned OpIdx) const override {
    return getSVABitsForOperand(Inst, OpIdx)
        .test(static_cast<unsigned>(SVAKind::LastScalar));
  }

  // Getter interfaces for broadcast and extract patterns.
  bool instNeedsBroadcast(const VPInstruction *Inst) const override;

  bool instNeedsExtractFromFirstActiveLane(
    const VPInstruction *Inst) const override;

  bool instNeedsExtractFromLastActiveLane(
    const VPInstruction *Inst) const override;

  // Getter interfaces for querying SVA results at return value level.
  bool retValNeedsVectorCode(const VPInstruction *Inst) const override {
    return getSVABitsForReturnValue(Inst).test(
      static_cast<unsigned>(SVAKind::Vector));
  }

  bool retValNeedsFirstScalarCode(const VPInstruction *Inst) const override {
    return getSVABitsForReturnValue(Inst).test(
      static_cast<unsigned>(SVAKind::FirstScalar));
  }

  bool retValNeedsLastScalarCode(const VPInstruction *Inst) const override {
    return getSVABitsForReturnValue(Inst).test(static_cast<unsigned>(
      SVAKind::LastScalar));
  }

  void compute(VPlanVector *P) override;

  void clear(void) override { VPlanSVAResults.clear(); }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// Print SVA result for given VPInstruction.
  void printSVAKindForInst(raw_ostream &OS,
                           const VPInstruction *VPI) const override;
  /// Print SVA result for given operand of VPInstruction.
  void printSVAKindForOperand(raw_ostream &OS, const VPInstruction *VPI,
                              unsigned OpIdx) const override;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

private:
  enum class SVAKind {
    // FirstScalar kind means that given value (instruction or operand) is
    // required in scalar context for the first active lane. CG must generate
    // scalar instruction for the value using appropriate first active lane
    // values. (NOTE : For a uniform value first active lane would imply a copy
    // of the scalar version of the instruction.)
    FirstScalar = 0,
    // LastScalar kind means that given value is required in scalar context
    // for the last active lane. CG must generate scalar instruction for the
    // value using appropriate last active lane values.
    LastScalar = 1,
    // Vector kind means that given value is vector in nature and CG must
    // widen the value. In case if value is uniform, its scalar value will be
    // broadcasted.
    Vector = 2,
    // Last possible SVAKind. Useful to count number of bits needed for tracking
    // in SVA.
    LastSVAKind = Vector
  };

  // Bitset to represent and track status of each SVAKind for an instruction or
  // its operands.
  using SVABits = std::bitset<static_cast<unsigned>(SVAKind::LastSVAKind) + 1>;
  using OperandBitsTy = SmallVector<SVABits, 4>;

  // Wrapper struct to encapsulate different types of SVABits that are tracked
  // for any instruction in VPlan.
  struct VPInstSVABits {
    // Bits to represent nature of return value of an instruction. NOTE: For
    // most cases these bits will be empty, which implies that return value of
    // instruction has the same nature as instruction. Exceptions would be
    // reduction finalization, some calls and all-zero check.
    SVABits RetValBits;
    // Bits to represent nature of the instruction.
    SVABits InstBits;
    // Bits to represent nature of operands needed for current instruction.
    OperandBitsTy OperandBits;
  };

  // Central data structure to track results of analysis.
  SmallDenseMap<const VPInstruction *, VPInstSVABits> VPlanSVAResults;

  // Container to track loop header PHIs that are skipped during forward
  // propagation. Such PHIs occur when they do not have any processed users i.e.
  // used by other unprocessed loop header PHIs.
  DenseSet<const VPPHINode *> SkippedPHIs;

  /// Utility to reverse-map bitset index to SVAKind.
  SVAKind getSVAKindForBit(unsigned BitIdx) {
    switch (BitIdx) {
    case 0:
      return SVAKind::FirstScalar;
    case 1:
      return SVAKind::LastScalar;
    case 2:
      return SVAKind::Vector;
    default:
      llvm_unreachable("Invalid SVA bitset index.");
    }
  }

  /// Utility to get current SVA bits set for a VPInstruction in the tracking
  /// table. Returns None if instruction does not have an entry in the table or
  /// is not processed i.e. no bits set.
  Optional<SVABits> findSVABitsForInst(const VPInstruction *Inst) const {
    auto InstIt = VPlanSVAResults.find(Inst);
    if (InstIt != VPlanSVAResults.end()) {
      if (InstIt->second.InstBits.none())
        return None;

      return InstIt->second.InstBits;
    }

    return None;
  }

  /// Utility to get current SVA bits set for given operand of VPInstruction in
  /// the tracking table. Returns None if instruction or the requested operand
  /// does not have an entry in the table or is not processed i.e. no bits set.
  Optional<SVABits> findSVABitsForOperand(const VPInstruction *Inst,
                                          unsigned OpIdx) const {
    auto InstIt = VPlanSVAResults.find(Inst);
    if (InstIt == VPlanSVAResults.end())
      return None;

    auto OperandBits = InstIt->second.OperandBits[OpIdx];
    // Results not computed/recorded for operand.
    if (OperandBits.none())
      return None;

    return OperandBits;
  }

  /// Utility to get current SVA bits set for return value of given
  /// VPInstruction in the tracking table. Returns None if instruction does not
  /// have an entry in the table or is not processed i.e. no bits set.
  Optional<SVABits> findSVABitsForReturnValue(const VPInstruction *Inst) const {
    auto InstIt = VPlanSVAResults.find(Inst);
    if (InstIt == VPlanSVAResults.end())
      return None;

    SVABits InstRetValBits = InstIt->second.RetValBits;
    // Generic case where return value bits is empty, meaning it has same nature
    // as instruction.
    if (InstRetValBits.none())
      return findSVABitsForInst(Inst);

    return InstRetValBits;
  }

  SVABits getSVABitsForInst(const VPInstruction *Inst) const {
    Optional<SVABits> InstBits = findSVABitsForInst(Inst);
    assert(InstBits && InstBits.value().any() &&
           "None of the SVA bits are set for VPInstruction.");
    return InstBits.value();
  }

  SVABits getSVABitsForOperand(const VPInstruction *Inst,
                               unsigned OpIdx) const {
    Optional<SVABits> OperandBits = findSVABitsForOperand(Inst, OpIdx);
    assert(OperandBits && OperandBits.value().any() &&
           "None of the SVA bits are set for operand of VPInstruction.");
    return OperandBits.value();
  }

  SVABits
  getAllSetBitsFromUsers(const VPInstruction *Inst,
                         SVABits DefaultBits = SVABits(4) /* Vector only */);

  SVABits getSVABitsForReturnValue(const VPInstruction *Inst) const {
    auto InstIt = VPlanSVAResults.find(Inst);
    assert(InstIt != VPlanSVAResults.end() &&
           "Instruction not processed, no entry in results.");
    SVABits InstRetValBits = InstIt->second.RetValBits;

    // Generic case where return value bits is empty, meaning it has same nature
    // as instruction.
    if (InstRetValBits.none())
      return getSVABitsForInst(Inst);

    return InstRetValBits;
  }

  /// Setter functions to set a specific bit or group of bits.

  /// Set SVA result for given instruction.
  void setSVAKindForInst(const VPInstruction *Inst, const SVAKind Kind) {
    VPlanSVAResults[Inst].InstBits.set(static_cast<unsigned>(Kind));
  }
  void setSVABitsForInst(const VPInstruction *Inst, SVABits &SetBits) {
    VPlanSVAResults[Inst].InstBits = SetBits;
  }

  /// Set SVA result for specific operand of given instruction.
  void setSVAKindForOperand(const VPInstruction *Inst, unsigned OpIdx,
                            const SVAKind Kind);
  void setSVABitsForOperand(const VPInstruction *Inst, unsigned OpIdx,
                            SVABits &SetBits);

  /// Set the same SVA result for all the operands of a given VPInstruction.
  void setSVAKindForAllOperands(const VPInstruction *Inst, const SVAKind Kind);
  void setSVABitsForAllOperands(const VPInstruction *Inst, SVABits &SetBits);

  /// Set SVA result for return value of given VPInstruction.
  void setSVAKindForReturnValue(const VPInstruction *Inst, const SVAKind Kind) {
    VPlanSVAResults[Inst].RetValBits.set(static_cast<unsigned>(Kind));
  }

  /// Utilities to update an instruction or operand SVABits by or-ing with a
  /// group of bits.

  void orSVABitsForInst(const VPInstruction *Inst, SVABits &OrBits) {
    VPlanSVAResults[Inst].InstBits |= OrBits;
  }
  void orSVABitsForOperand(const VPInstruction *Inst, unsigned OpIdx,
                           SVABits &OrBits);
  void orSVABitsForAllOperands(const VPInstruction *Inst, SVABits &OrBits);

  /// Analyze and compute SVA properties for given VPInstruction. Following 3
  /// cases are possible for an instruction -
  /// 1. Specially processed
  /// 2. Determine SVA nature using DA
  /// 3. Compute and refine SVA nature based on user-site bits
  void compute(const VPInstruction *VPInst);

  /// Specialized method to back propagate SVA bits for a recurrent PHI node.
  /// Since these PHIs can represent back-edge or recurrence property, a chain
  /// of propagation might be needed. Consider the example below -
  //
  //  loop.body:
  //    %iv.phi = [ %init, PH ], [ %iv.update, Body ]
  //    %div.use = add %iv.phi, %div.value
  //    ...
  //    %iv.update = add %iv.phi, %iv.step
  //
  //  Initially SVA visits %iv.update and finds it to be Uniform and hence
  //  "FirstScalar" attribute is set for %iv.phi. Next when %div.use is visited,
  //  since it's Divergent in nature, attribute of %iv.phi becomes
  //  "FirstScalarVector". This new attribute needs to be re-propagated to
  //  %iv.update and its operands. Currently recurrent PHIs are expected to be
  //  seen only at loop headers.
  void backPropagateSVABitsForRecurrentPHI(const VPPHINode *Phi,
                                           SVABits &SetBits);

  /// Compute SVA properties for instructions that need special processing. Such
  /// instructions are analyzed using more information and their SVA nature is
  /// not blindly propagated to operands. This utility returns false if given
  /// instruction is not processed specially.
  bool computeSpecialInstruction(const VPInstruction *Inst);

  /// Helper utility to check if given instruction is processed specially in
  /// SVA.
  bool isSVASpecialProcessedInst(const VPInstruction *Inst);
};

/// This class implementation for so called Scalar ScalVec analysis, which
/// results are enquired for VF == 1 only. We don't really do any analysis
/// for VF = 1 but return trivially known results.
/// The class is used to unify access to VPlanScalVecAnalysisBase's methods
/// for all VFs.
class VPlanScalVecAnalysisScalar final : public VPlanScalVecAnalysisBase {
public:
  explicit VPlanScalVecAnalysisScalar() :
    VPlanScalVecAnalysisBase(SVAType::Scalar) {};
  ~VPlanScalVecAnalysisScalar() = default;

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(VPlanScalVecAnalysisBase const *V) {
    return V->getSVAType() == SVAType::Scalar;
  }

  // Getter interfaces for querying SVA results at instruction-level.
  bool instNeedsVectorCode(const VPInstruction *Inst) const override {
    return false;
  }

  bool instNeedsFirstScalarCode(const VPInstruction *Inst) const override {
    return true;
  }

  bool instNeedsLastScalarCode(const VPInstruction *Inst) const override {
    return false;
  }

  // Getter interfaces for querying SVA results at operand-level.
  bool operandNeedsVectorCode(const VPInstruction *Inst,
                              unsigned OpIdx) const override {
    return false;
  }

  bool operandNeedsFirstScalarCode(const VPInstruction *Inst,
                                   unsigned OpIdx) const override {
    return true;
  }

  bool operandNeedsLastScalarCode(const VPInstruction *Inst,
                                  unsigned OpIdx) const override {
    return false;
  }

  // Getter interfaces for broadcast and extract patterns.
  bool instNeedsBroadcast(const VPInstruction *Inst) const override {
    return false;
  }

  bool instNeedsExtractFromFirstActiveLane(
    const VPInstruction *Inst) const override {
    return false;
  }

  bool instNeedsExtractFromLastActiveLane(
    const VPInstruction *Inst) const override {
    return false;
  }

  // Getter interfaces for querying SVA results at return value level.
  bool retValNeedsVectorCode(const VPInstruction *Inst) const override {
    return false;
  }

  bool retValNeedsFirstScalarCode(const VPInstruction *Inst) const override {
    return true;
  }

  bool retValNeedsLastScalarCode(const VPInstruction *Inst) const override {
    return false;
  }

  void compute(VPlanVector *P) override {}

  void clear(void) override {}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// Print SVA result for given VPInstruction.
  void printSVAKindForInst(raw_ostream &OS,
                           const VPInstruction *VPI) const override;
  /// Print SVA result for given operand of VPInstruction.
  void printSVAKindForOperand(raw_ostream &OS, const VPInstruction *VPI,
                              unsigned OpIdx) const override;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

private:
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLANSCALVECANALYSIS_H
