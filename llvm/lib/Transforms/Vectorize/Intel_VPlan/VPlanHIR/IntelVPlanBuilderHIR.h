//===- IntelVPlanBuilderHIR.h - A utility for constructing VPInstructions -===//
//
//   Copyright (C) 2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file extends VPlanBuilder utility to create VPInstruction from HIR.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANBUILDER_HIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANBUILDER_HIR_H

#include "../IntelVPlanBuilder.h"

namespace llvm {
namespace vpo {

class VPBuilderHIR : public VPBuilder {
public:
  /// Create an N-ary operation with \p Opcode and \p Operands and set \p HInst
  /// as its VPInstructionData.
  VPValue *createNaryOp(unsigned Opcode, ArrayRef<VPValue *> Operands,
                        loopopt::HLDDNode *DDNode = nullptr) {
    VPInstruction *NewVPInst =
        cast<VPInstruction>(VPBuilder::createNaryOp(Opcode, Operands));
    if (DDNode)
      NewVPInst->HIR.setUnderlyingNode(DDNode);
    return NewVPInst;
  }
  VPValue *createNaryOp(unsigned Opcode,
                        std::initializer_list<VPValue *> Operands,
                        loopopt::HLDDNode *DDNode = nullptr) {
    return createNaryOp(Opcode, ArrayRef<VPValue *>(Operands), DDNode);
  }

  /// Create a VPInstruction with 'Add' opcode, \p LHS and \p RHS as operands
  /// and \p DDNode as its VPInstructionData.
  VPValue *createAdd(VPValue *LHS, VPValue *RHS, loopopt::HLDDNode *DDNode) {
    assert(DDNode && "DDNode can't be null.");
    auto *NewAdd = cast<VPInstruction>(VPBuilder::createAdd(LHS, RHS));
    NewAdd->HIR.setUnderlyingNode(DDNode);
    return NewAdd;
  }

  /// Create a VPCmpInst with \p LHS and \p RHS as operands, \p Pred as
  /// predicate and set \p DDNode as its VPInstructionData.
  VPCmpInst *createCmpInst(VPValue *LHS, VPValue *RHS, CmpInst::Predicate Pred,
                           loopopt::HLDDNode *DDNode) {
    assert(DDNode && "DDNode can't be null.");
    assert(LHS && RHS && "VPCmpInst's operands can't be null!");
    VPCmpInst *NewVPCmp = VPBuilder::createCmpInst(LHS, RHS, Pred);
    NewVPCmp->HIR.setUnderlyingNode(DDNode);
    return NewVPCmp;
  }

  /// Create a semi-phi operation with \p Operands as reaching definitions.
  VPValue *createSemiPhiOp(ArrayRef<VPValue *> Operands,
                           loopopt::HLDDNode *DDNode = nullptr) {
    // TODO: Enable assert, remove 'if' and invoke createPhi in super class for
    // semi-phis without underlying HIR when VPPhi representation is introduced.
    // assert(DDNode && "DDNode can't be null.");
    VPInstruction *NewSemiPhi =
        createInstruction(VPInstruction::SemiPhi, Operands);
    if (DDNode)
      NewSemiPhi->HIR.setUnderlyingNode(DDNode);
    return NewSemiPhi;
  }

  VPValue *createSemiPhiOp(std::initializer_list<VPValue *> Operands,
                           loopopt::HLDDNode *DDNode) {
    return createSemiPhiOp(ArrayRef<VPValue *>(Operands), DDNode);
  }

  // Construct VPBranchInst instruction from a \p Goto.
  VPBranchInst *createBr(loopopt::HLGoto *Goto) {
    assert(Goto && "HLGoto must be passed to construct VPBranchInst.");
    VPBranchInst *BranchInst = VPBuilder::createBr();
    BranchInst->HIR.setUnderlyingNode(Goto);
    BranchInst->HIR.setValid();
    return BranchInst;
  }

};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANBUILDER_HIR_H
