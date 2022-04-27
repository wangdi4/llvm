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
#include "IntelVPlanHCFGBuilderHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

namespace llvm {
namespace loopopt {
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
public:
  class VPInductionHIR {
    // VPInstruction that incerements the induction.
    VPInstruction *UpdateInstr;
    // Step of induction.
    VPValue *Step;
    // Incoming (starting) value of induction. Can be null, in that case the
    // initial value can be obtained from the phi that uses the UpdateInstr.
    VPValue *Start;
    // Constant lower/upper bound of induction; nullptr otherwise.
    VPValue *StartVal = nullptr;
    VPValue *EndVal = nullptr;

  public:
    explicit VPInductionHIR(VPInstruction *Instr, VPValue *Stride,
                            VPValue *Incoming, VPValue *StartVal,
                            VPValue *EndVal)
        : UpdateInstr(Instr), Step(Stride), Start(Incoming),
          StartVal(StartVal), EndVal(EndVal) {}

    VPInductionHIR() = delete;

    VPInstruction *getUpdateInstr() { return UpdateInstr; }
    VPValue *getStep() { return Step; }
    VPValue *getStart() { return Start; }
    VPValue *getStartVal() { return StartVal; }
    VPValue *getEndVal() { return EndVal; }
  };
  typedef SmallVector<std::unique_ptr<VPInductionHIR>, 2> VPInductionHIRList;
  typedef DenseMap<const loopopt::HLLoop *, std::unique_ptr<VPInductionHIRList>>
      VPLoopInductionsHIRMap;

  // Get the load of VConflict idiom.
  VPInstruction *getVPLoadConflict(DDRef *LoadRef) {
    return RefToVPLoadConflict[LoadRef];
  }

  // Get the conflict index of VConflict idiom.
  VPValue *getVPConflictIndex(DDRef *LoadRef) {
    return RefToConflictingIndex[LoadRef];
  }

private:
  /// The VPlan we are working on.
  VPlanVector *Plan;

  /// Outermost HLLoop in this VPlan.
  const loopopt::HLLoop *OutermostHLp;

  /// HIR DDGraph that contains DD information for the incoming loop nest.
  const loopopt::DDGraph &DDG;

  /// Map HLInst's and their respective VPValue's representing their definition.
  DenseMap<loopopt::HLDDNode *, VPValue *> HLDef2VPValue;

  /// VPInstruction builder for HIR.
  VPBuilderHIR Builder;

  /// Map HLLoop to the VPPHI instruction representing its IV in VPlan.
  SmallDenseMap<loopopt::HLLoop *, VPPHINode *> HLLp2IVPhi;

  // Holds lists of induction descriptors, grouped by HLLoop.
  VPLoopInductionsHIRMap Inductions;

  // HIR legality object
  HIRVectorizationLegality &HIRLegality;

  // Maps Load RegDDRef to the generated load instruction that corresponds
  // to VConflict idiom.
  SmallDenseMap<DDRef *, VPInstruction *> RefToVPLoadConflict;

  // Maps Load RegDDRef to the conflicting index of VConflict idiom.
  SmallDenseMap<DDRef *, VPValue *> RefToConflictingIndex;

  // A map to track empty VPPhi nodes that are added during decomposition. We
  // know that there can be only one unique PHI node per VPBasicBlock for a
  // given Symbase (corresponding to the sink DDRef). We are also storing the
  // sink DDRef that triggered the placement of this VPPhi node, which will be
  // used to create ExternalUses of this PHI if needed.

  // Key for the PhisToFix map is <VPBasicBlock, Symbase>
  using PhiFixMapKey = std::pair<VPBasicBlock *, unsigned>;
  // The value mapped to the key is the VPPHINode and the ambiguous sink DDRef
  using PhiFixMapValue = std::pair<VPPHINode *, loopopt::DDRef *>;
  MapVector<PhiFixMapKey, PhiFixMapValue> PhisToFix;

  // Internal helper to obtain the ambiguous sink DDRef corresponding to a
  // tracked symbase for which empty PHIs were created.
  loopopt::DDRef *getDDRefForTrackedSymbase(unsigned Sym) const {
    for (auto KeyVal : PhisToFix) {
      if (Sym == KeyVal.first.second)
        return KeyVal.second.second;
    }

    // Symbase is not being tracked for fixing PHIs.
    return nullptr;
  }

  //////////////// Data structures for fixPhiNodePass //////////////////

  // a. Set of unique Symbases for which empty PHI nodes were added
  SetVector<unsigned> TrackedSymbases;
  // b. Preserve a map to track which VPPhiNode corresponds to which Symbase
  // This is needed for quick lookup, instead of iterating over PhisToFix map
  MapVector<VPPHINode *, unsigned> PhiToSymbaseMap;
  // c. Set of visited VPBBs for the traversal
  SmallPtrSet<VPBasicBlock *, 16> PhiNodePassVisited;
  // d. Map to lookup the base types of the PHI nodes that were added for given
  // tracked Symbase
  DenseMap<unsigned, Type *> TrackedSymTypes;

  /// Data package used by fixPhiNodePass
  struct PhiNodePassData {
    // Map to track the incoming VPValue of a Symbase when traversing from
    // predecessor block (VPBBPred) to current block (VPBB). Symbase is the key
    // and its corresponding VPValue is the value.
    using VPValMap = std::map<unsigned, VPValue *>;

    PhiNodePassData(VPBasicBlock *B, VPBasicBlock *P, VPValMap V)
        : VPBB(B), VPBBPred(P), VPValues(V) {}

    VPBasicBlock *VPBB;
    VPBasicBlock *VPBBPred;
    VPValMap VPValues;
  };

  //////////////////////////////////////////////////////////////////////

  // Execute the PHI node fixing algorithm by visiting the VPBasicBlock \p VPBB
  // from its predecessor \p Pred, with the current state of VPValues for the
  // tracked Symbases stored in \p IncomingVPVals. \p Worklist is used to add
  // new VPBBs that need to be processed after VPBB.
  void fixPhiNodePass(VPBasicBlock *VPBB, VPBasicBlock *Pred,
                      PhiNodePassData::VPValMap &IncomingVPVals,
                      SmallVectorImpl<PhiNodePassData> &Worklist);

  void computeLiveInBlocks(unsigned Symbase,
                           const SmallPtrSetImpl<VPBasicBlock *> &DefBlocks,
                           const SmallPtrSetImpl<VPBasicBlock *> &UsingBlocks,
                           SmallPtrSetImpl<VPBasicBlock *> &LiveInBlocks);

  void addIDFPhiNodes();

  // Methods to create VPInstructions out of an HLNode.
  bool isExternalDef(const loopopt::DDRef *UseDDR);
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
  void getOrCreateVPDefsForUse(loopopt::DDRef *UseDDR,
                               SmallVectorImpl<VPValue *> &VPDefs);

  // Private helper method to create CmpInsts in VPlan using given HLPredicate
  // and operands.
  VPCmpInst *createCmpInst(const HLPredicate &P, VPValue *LHS, VPValue *RHS) {
    VPCmpInst *Inst = Builder.createCmpInst(P.Kind, LHS, RHS);
    Inst->setDebugLocation(P.DbgLoc);
    if (CmpInst::isFPPredicate(P.Kind))
      Inst->setFastMathFlags(P.FMF);
    return Inst;
  }

  // Helper method to retrieve or create an empty PHI node for an ambiguous use
  // of given DDRef. The PHI is created at the beginning of provided
  // VPBasicBlock, and then added to relevant fixPhiNodes data structures.
  VPPHINode *getOrCreateEmptyPhiForDDRef(Type *PhiTy, VPBasicBlock *VPBB,
                                         DDRef *DDR);

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
    VPValue *visitPtrToIntExpr(const SCEVPtrToIntExpr *);
    VPValue *visitAddExpr(const SCEVAddExpr *Expr);
    VPValue *visitMulExpr(const SCEVMulExpr *Expr);
    VPValue *visitUDivExpr(const SCEVUDivExpr *Expr);
    VPValue *visitAddRecExpr(const SCEVAddRecExpr *Expr);
    VPValue *visitSMaxExpr(const SCEVSMaxExpr *Expr);
    VPValue *visitUMaxExpr(const SCEVUMaxExpr *Expr);
    VPValue *visitSMinExpr(const SCEVSMinExpr *Expr);
    VPValue *visitUMinExpr(const SCEVUMinExpr *Expr);
    VPValue *visitSequentialUMinExpr(const SCEVSequentialUMinExpr *Expr);
    VPValue *visitUnknown(const SCEVUnknown *Expr);
    VPValue *visitCouldNotCompute(const SCEVCouldNotCompute *Expr);
  };
  friend class VPBlobDecompVisitor;

public:
  VPDecomposerHIR(VPlanVector *P, const loopopt::HLLoop *OHLp,
                  const loopopt::DDGraph &DDG,
                  HIRVectorizationLegality &HIRLegality)
      : Plan(P), OutermostHLp(OHLp), DDG(DDG), HIRLegality(HIRLegality){};

  /// Create VPInstructions for the incoming \p Node and insert them into \p
  /// InsPointVPBB. \p Node will be decomposed into several VPInstructions if
  /// it's too complex to be represented using a single VPInstruction. Return
  /// the last VPInstruction of the Def/Use chain created.
  VPInstruction *createVPInstructionsForNode(loopopt::HLNode *Node,
                                             VPBasicBlock *InsPointVPBB);

  /// Create a VPPHINode representing the \p HLp IV and a VPValue
  /// for the IV Start. The PHI is inserted in the loop header VPBasicBlock
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

  /// Create instructions that compute the Ztt check for the given \p HLp.
  /// The instruction that gives the final result to be used for bypassing
  /// the loop is returned.
  VPInstruction *createLoopZtt(HLLoop *HLp, VPBasicBlock *ZttBlock);

  /// Create empty PHIs for temps that are live-out in VPlan's exit block. This
  /// ensures that correct merge PHIs are emitted for ambiguous use of a DDRef
  /// outside the loop being vectorized. Consider the example below -
  //
  // %t1 = ...
  // DO LOOP
  //   if (cond)
  //     %t1 = ...
  // END LOOP
  //   ... = %t1
  //
  // For above input HIR, a PHI is needed in loop latch VPBB to merge ambiguous
  // definitions of %t1, even though %t1 has no uses inside the loop.
  void createExitPhisForExternalUses(VPBasicBlock *ExitBB);

  /// Add operands to empty VPPHINodes added during decomposition.
  void fixPhiNodes();

  // Fix operands of VPExternalUses created for live-out temps during
  // decomposition.
  void fixExternalUses();

  VPValue *getVPValueForNode(const loopopt::HLNode *Node);

  /// Return requested VPConstant, for components that don't have VPlan
  /// reference.
  VPConstant *getVPValueForConst(Constant *CVal) const {
    return Plan->getVPConstant(CVal);
  }

  VPExternalDef *getVPExternalDefForDDRef(const loopopt::DDRef *Ref) {
    assert(isExternalDef(Ref) &&
           "DDRef is not externally defined for the loop.");
    return Plan->getVPExternalDefForDDRef(Ref);
  }

  VPExternalDef *getVPExternalDefForSIMDDescr(const loopopt::DDRef *Ref) {
    bool IsSIMDDescr = HIRLegality.getReductionDescr(Ref) != nullptr ||
                       HIRLegality.getLinearDescr(Ref) != nullptr ||
                       HIRLegality.getPrivateDescr(Ref) != nullptr ||
                       HIRLegality.getPrivateDescrNonPOD(Ref) != nullptr;
    assert(IsSIMDDescr && "DDRef is not a SIMD entity descriptor.");
    (void)IsSIMDDescr;
    return Plan->getVPExternalDefForDDRef(Ref);
  }

  /// Return induction list.
  VPInductionHIRList &getInductions(const loopopt::HLLoop *L) {
    return *(Inductions[L]);
  }

  // Construct VPBranchInst instruction from a \p HGoto.
  VPBranchInst *createVPBranchInstruction(VPBasicBlock *InsPointVPBB,
                                          VPBasicBlock *Succ,
                                          loopopt::HLGoto *HGoto) {
    assert(HGoto && "HLGoto must be passed to construct VPBranchInst.");
    InsPointVPBB->setTerminator(Succ);

    VPBranchInst *BranchInst = InsPointVPBB->getTerminator();
    BranchInst->setDebugLocation(HGoto->getDebugLoc());
    BranchInst->HIR().setUnderlyingNode(HGoto);
    BranchInst->HIR().setValid();

    return BranchInst;
  }
};

} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANDECOMPOSERHIR_H
