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

  /// Map HLLoop to the semi-phi instruction representing its IV in VPlan.
  SmallDenseMap<loopopt::HLLoop *, VPInstruction *> HLLp2IVSemiPhi;

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

  /// Create a semi-phi VPInstruction representing the \p HLp IV and a VPValue
  /// for the IV Start. The semi-phi is inserted in the loop header VPBasicBlock
  /// (successor of \p LpPH).
  // TODO: The IV Start will be inserted in the loop pre-header (\p LpPH). We
  // only support constant IV Starts that do not require to be inserted in any
  // VPBasicBlock.
  void createLoopIVAndIVStart(loopopt::HLLoop *HLp, VPBasicBlock *LpPH);

  /// Create the VPValue representation for the \p HLp bottom test and IV Next
  /// and insert them in \p LpLatch.
  VPValue *createLoopIVNextAndBottomTest(loopopt::HLLoop *HLp, VPBasicBlock *LpLatch);
};

} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANDECOMPOSERHIR_H
