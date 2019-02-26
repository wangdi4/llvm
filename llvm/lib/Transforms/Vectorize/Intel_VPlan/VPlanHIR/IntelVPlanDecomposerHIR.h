//===-- VPlanDecomposeHIR.h -------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

namespace llvm {
namespace loopopt {
class HLLoop;
class CanonExpr;
class DDGraph;
class DDEdge;
class DDRef;
class HLIf;
} // namespace loopopt

namespace vpo {

// Main class to create VPInstructions out of HLNodes during the VPlan plain
// CFG construction.
class VPDecomposerHIR {
private:
  /// The VPlan we are working on.
  VPlan *Plan;

  /// Outermost HLLoop in this VPlan.
  const loopopt::HLLoop *OutermostHLp;

  /// HIR DDGraph that contains DD information for the incoming loop nest.
  const loopopt::DDGraph &DDG;

  /// Map HLInst's and their respective VPValue's representing their definition.
  DenseMap<loopopt::HLDDNode *, VPValue *> HLDef2VPValue;

  /// VPInstruction builder for HIR.
  VPBuilderHIR Builder;

  /// Map HLLoop to the semi-phi instruction representing its IV in VPlan.
  SmallDenseMap<loopopt::HLLoop *, VPInstruction *> HLLp2IVSemiPhi;

  // A map to track empty VPPhi nodes that are added during decomposition. We
  // know that there can be only one unique PHI node per VPBasicBlock for a
  // given Symbase (corresponding to the sink DDRef). Currently we are also
  // storing the sink DDRef that triggered the placement of this VPPhi node, but
  // this will be removed in the future when the new algorithm to fix empty PHI
  // nodes is implemented.
  // TODO: Remove DDRef after implementing new VPPhi node fixing algorithm

  // Key for the PhisToFix map is <Symbase, VPBlockID>
  using PhiFixMapKey = std::pair<unsigned, unsigned>;
  // The value mapped to the key is the semi-phi instruction and the ambiguous
  // sink DDRef
  using PhiFixMapValue = std::pair<VPInstruction *, loopopt::DDRef *>;
  DenseMap<PhiFixMapKey, PhiFixMapValue> PhisToFix;

  // Methods to create VPInstructions out of an HLNode.
  bool isExternalDef(loopopt::DDRef *UseDDR);
  unsigned getNumReachingDefinitions(loopopt::DDRef *UseDDR);
  void setMasterForDecomposedVPIs(VPInstruction *MasterVPI,
                                  VPInstruction *LastVPIBeforeDec,
                                  VPBasicBlock *VPBB);
  VPInstruction *createVPInstruction(loopopt::HLNode *Node,
                                     ArrayRef<VPValue *> VPOperands);
  VPInstruction *createVPInstsForHLIf(loopopt::HLIf *HIf,
                                      ArrayRef<VPValue *> VPOperands);
  void createVPOperandsForMasterVPInst(loopopt::HLNode *Node,
                                       SmallVectorImpl<VPValue *> &VPOperands);
  void createOrGetVPDefsForUse(loopopt::DDRef *UseDDR,
                               SmallVectorImpl<VPValue *> &VPDefs);

  // Methods to decompose complex HIR.
  VPValue *combineDecompDefs(VPValue *LHS, VPValue *RHS, Type *Ty,
                             unsigned OpCode);
  VPConstant *decomposeCoeff(int64_t Coeff, Type *Ty);
  VPInstruction *decomposeConversion(VPValue *Src, unsigned ConvOpCode,
                                     Type *DestType);
  VPValue *decomposeCanonExprConv(loopopt::CanonExpr *CE, VPValue *Src);
  VPValue *decomposeBlobImplicitConv(VPValue *Src, Type *DestTy);
  VPValue *decomposeBlob(loopopt::RegDDRef *RDDR, unsigned BlobIdx,
                         int64_t BlobCoeff);
  VPValue *decomposeIV(loopopt::RegDDRef *RDDR, loopopt::CanonExpr *CE,
                       unsigned IVLevel, Type *Ty);
  VPValue *decomposeCanonExpr(loopopt::RegDDRef *RDDR, loopopt::CanonExpr *CE);
  VPValue *decomposeMemoryOp(loopopt::RegDDRef *RDDR);
  VPValue *decomposeVPOperand(loopopt::RegDDRef *RDDR);

  /// This class implements the decomposition of a blob in a RegDDRef. The
  /// decomposition is based on the SCEV representation of the blob.
  class VPBlobDecompVisitor
      : public SCEVVisitor<VPBlobDecompVisitor, VPValue *> {
  private:
    // RegDDRef of the blobs we are decomposing.
    loopopt::RegDDRef &RDDR;

    // Hold the global state of the decomposition and shared methods also used
    // to decomposed RegDDRefs and CanonExprs.
    VPDecomposerHIR &Decomposer;

    // Helper functions.
    VPValue *createOrGetVPDefFor(const loopopt::DDEdge *Edge);
    VPConstant *decomposeNonIntConstBlob(const SCEVUnknown *Blob);
    VPValue *decomposeStandAloneBlob(const SCEVUnknown *Blob);
    VPValue *decomposeNAryOp(const SCEVNAryExpr *Blob, unsigned OpCode);

  public:
    VPBlobDecompVisitor(loopopt::RegDDRef &R, VPDecomposerHIR &Dec)
        : RDDR(R), Decomposer(Dec) {}

    // Visitor methods.
    VPValue *visitConstant(const SCEVConstant *Constant);
    VPValue *visitTruncateExpr(const SCEVTruncateExpr *Expr);
    VPValue *visitZeroExtendExpr(const SCEVZeroExtendExpr *Expr);
    VPValue *visitSignExtendExpr(const SCEVSignExtendExpr *Expr);
    VPValue *visitAddExpr(const SCEVAddExpr *Expr);
    VPValue *visitMulExpr(const SCEVMulExpr *Expr);
    VPValue *visitUDivExpr(const SCEVUDivExpr *Expr);
    VPValue *visitAddRecExpr(const SCEVAddRecExpr *Expr);
    VPValue *visitSMaxExpr(const SCEVSMaxExpr *Expr);
    VPValue *visitUMaxExpr(const SCEVUMaxExpr *Expr);
    VPValue *visitUnknown(const SCEVUnknown *Expr);
    VPValue *visitCouldNotCompute(const SCEVCouldNotCompute *Expr);
  };
  friend class VPBlobDecompVisitor;

public:
  VPDecomposerHIR(VPlan *P, const loopopt::HLLoop *OHLp,
                  const loopopt::DDGraph &DDG)
      : Plan(P), OutermostHLp(OHLp), DDG(DDG){};

  /// Create VPInstructions for the incoming \p Node and insert them into \p
  /// InsPointVPBB. \p Node will be decomposed into several VPInstructions if
  /// it's too complex to be represented using a single VPInstruction. Return
  /// the last VPInstruction of the Def/Use chain created.
  VPInstruction *createVPInstructionsForNode(loopopt::HLNode *Node,
                                             VPBasicBlock *InsPointVPBB);

  /// Create a semi-phi VPInstruction representing the \p HLp IV and a VPValue
  /// for the IV Start. The semi-phi is inserted in the loop header VPBasicBlock
  /// (successor of \p LpPH).
  // TODO: The IV Start will be inserted in the loop pre-header (\p LpPH). We
  // only support constant IV Starts that do not require to be inserted in any
  // VPBasicBlock.
  void createLoopIVAndIVStart(loopopt::HLLoop *HLp, VPBasicBlock *LpPH);

  /// Create the VPValue representation for the \p HLp bottom test and and IV
  /// Next and insert them in \p LpLatch. The decomposed VPInstructions of the
  /// \p HLp upper bound are inserted into \p LpPH since they are loop
  /// invariant.
  VPValue *createLoopIVNextAndBottomTest(loopopt::HLLoop *HLp,
                                         VPBasicBlock *LpPH,
                                         VPBasicBlock *LpLatch);

  /// Add operands to VPInstructions representing semi-phi operation resulting
  /// from decomposition.
  void fixPhiNodes();
};

} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANDECOMPOSERHIR_H
