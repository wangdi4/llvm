//===- VPlanBuilderHIR.h - A VPlan utility for constructing VPInstructions ===//
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

#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLAN_BUILDER_HIR_H
#define LLVM_TRANSFORMS_VECTORIZE_VPLAN_BUILDER_HIR_H

#include "VPlan/VPlanInstructionData.h"
#include "VPlanBuilder.h"

namespace llvm {
namespace vpo {

class VPBuilderHIR : public VPBuilder {
public:
  /// Create an N-ary operation with \p Opcode and \p Operands and set \p HInst
  /// as its VPInstructionData.
  VPValue *createNaryOp(unsigned Opcode, ArrayRef<VPValue *> Operands,
                        HLDDNode *DDNode) {
    VPInstruction *NewVPInst = createInstruction(Opcode, Operands);
    NewVPInst->setHIRData(new VPInstructionDataHIR(DDNode));
    return NewVPInst;
  }
  VPValue *createNaryOp(unsigned Opcode,
                        std::initializer_list<VPValue *> Operands,
                        HLDDNode *DDNode) {
    return createNaryOp(Opcode, ArrayRef<VPValue *>(Operands), DDNode);
  }

  /// Create a semi-phi operation with \p Operands as reaching definitions.
  VPValue *createSemiPhiOp(ArrayRef<VPValue *> Operands) {
    return createInstruction(VPInstruction::SemiPhi, Operands);
  }
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_VPLAN_BUILDER_HIR_H
