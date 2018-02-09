//===-- VPlanDecomposeHIR.h -------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2018 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
/// TODO: Port AVR Doc.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANDECOMPOSERHIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANDECOMPOSERHIR_H

#include "IntelVPlanBuilderHIR.h"

namespace llvm {
namespace loopopt {
class HLLoop;
class CanonExpr;
class DDGraph;
class DDEdge;
} // namespace loopopt

namespace vpo {

class VPDecomposerHIR {
private:
  /// The VPlan we are working on.
  VPlan *Plan;

  /// HIR DDGraph that contains DD information for the incoming loop nest.
  const loopopt::DDGraph &DDG;

  /// Map HLInst's and their respective VPValue's representing their definition.
  DenseMap<loopopt::HLDDNode *, VPValue *> HLDef2VPValue;

  /// VPInstruction builder for HIR.
  VPBuilderHIR Builder;

  /// Map the HIR IV level to the semi-phi instruction representing that IV in
  /// VPlan.
  SmallDenseMap<int, VPInstruction *> IVLevel2SemiPhi;

  // Methods to create VPInstructions out of an HLDDNode.
  VPInstruction *createNoOperandVPInst(loopopt::HLDDNode *DDNode,
                                       bool InsertVPInst);
  VPValue *createOrGetVPDefFrom(const loopopt::DDEdge *Edge);
  VPValue *createOrGetVPOperand(loopopt::RegDDRef *HIROp);
  void buildVPOpsForDDNode(loopopt::HLDDNode *HInst,
                           SmallVectorImpl<VPValue *> &VPValueOps);

public:
  VPDecomposerHIR(VPlan *P, const loopopt::DDGraph &DDG) : Plan(P), DDG(DDG){};

  /// Create VPInstructions for the incoming \p DDNode and insert them into \p
  /// InsPointVPBB. \p DDNode will be decomposed into several VPInstructions if
  /// it's too complex to be represented using a single VPInstruction. Return
  /// the last VPInstruction of the Def/Use chain created.
  VPInstruction *createVPInstructions(loopopt::HLDDNode *DDNode,
                                      VPBasicBlock *InsPointVPBB);
};

} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANDECOMPOSERHIR_H
