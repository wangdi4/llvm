//===-- VPlanDecomposeHIR.cpp ---------------------------------------------===//
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
/// TODO: Port AVR documentation.
///
//===----------------------------------------------------------------------===//

#include "IntelVPlanDecomposerHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDGraph.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"

#define DEBUG_TYPE "vplan-decomposer"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::loopopt;

// Utility function that returns a CmpInst::Predicate for a given DDNode. The
// return value is in the context of the *plain* CFG construction (see
// 'createNoOperandVPInst' for further information):
//   1) HLInst representing a CmpInst -> CmpInst's opcode.
//   2) Single-predicate HLIf -> HLIf single predicate.
//   3) Multi-predicate HLIf -> BAD_ICMP_PREDICATE (to be fixed during
//      decomposition).
//   4) HLLoop -> ICMP_SLE or ICMP_ULE (bottom test).
// Please, note that the previous semantics are only valid during *plain* CFG
// constructions and may become invalid after HIR decomposition. Do not reuse
// this code for other purposes.
static CmpInst::Predicate getPredicateFromHIR(HLDDNode *DDNode) {
  assert((isa<HLInst>(DDNode) || isa<HLIf>(DDNode) || isa<HLLoop>(DDNode)) &&
         "Expected HLInst, HLIf or HLLoop.");

  if (auto *HInst = dyn_cast<HLInst>(DDNode)) {
    assert(isa<CmpInst>(HInst->getLLVMInstruction()) && "Expected CmpInst.");
    return cast<CmpInst>(HInst->getLLVMInstruction())->getPredicate();
  }

  if (auto *HIf = dyn_cast<HLIf>(DDNode)) {
    assert(HIf->getNumPredicates() && "HLIf with no predicate?");
    CmpInst::Predicate FirstPred = HIf->pred_begin()->Kind;

    // Multiple predicates need decomposition. We mark this with a bad
    // predicate.
    if (HIf->getNumPredicates() > 1) {
      // TODO: HIR recently added support for a mix of INT/FP predicates so this
      // assert will become invalid soon. This might not impact this function
      // but decomposition should generate the right predicate for each
      // comparison.
      assert(CmpInst::isIntPredicate(FirstPred) &&
             "Multiple predicates only expected on integer types.");

      return CmpInst::BAD_ICMP_PREDICATE;
    }

    // Single predicate.
    return FirstPred;
  }

  // Get the predicate for the HLLoop bottom test condition.
  auto *HLp = cast<HLLoop>(DDNode);
  assert(HLp->isDo() && HLp->isNormalized() &&
         "Expected single-exit normalized DO HLLoop.");
  assert(HLp->getLowerCanonExpr()->getDestType()->isIntegerTy() &&
         HLp->getUpperCanonExpr()->getDestType()->isIntegerTy() &&
         "HLLoops only support integer IVs.");

  // HLLoop upper-bound is inclusive so we return the proper less-equal
  // predicate based on the sign bit of the comparison type.
  // TODO: Does HIR perform any normalization regarding sign/unsigned types?
  if (cast<IntegerType>(HLp->getLowerCanonExpr()->getDestType())->getSignBit())
    return CmpInst::ICMP_SLE;

  return CmpInst::ICMP_ULE;
}

// Create a VPInstruction with no operands for the incoming DDNode. If
// InsertVPInst is true, the new VPInstruction is inserted in the current
// VPBuilder's insertion point. Otherwise, it's not inserted.
// During *plain* CFG construction, we create VPCmpInst for the following
// complex HIR constructs that will be later fixed during decomposition:
//     1) Multi-predicate HLIf: The VPCmpInst represents the multi-predicate
//     comparisons and contains all the operands involved in the comparisons.
//     2) HLLoop: The VPCmpInst represents the bottom test comparison. It
//     contains the HLLoop lower-bound, the upper-bound and the step.
// Please, note that the previous semantics are only valid during *plain* CFG
// constructions and may become invalid after HIR decomposition. Do not reuse
// this code for other purposes.
VPInstruction *VPDecomposerHIR::createNoOperandVPInst(HLDDNode *DDNode,
                                                      bool InsertVPInst) {
  // Clear the insertion point if we don't have to insert the new VPInstruction.
  VPBuilder::InsertPointGuard Guard(Builder);
  if (!InsertVPInst)
    Builder.clearInsertionPoint();

  // TODO: all VPInstructions should have a DDNode. However, we are currently
  // representing external defs with a VPInstruction and we are not attaching
  // DDNode to it. We will fix this when we have a specific representation for
  // external defs.
  if (DDNode) {
    // Create a new VPInstruction from DDNode with no operands.
    HLInst *HInst = dyn_cast<HLInst>(DDNode);
    assert((!HInst || HInst->getLLVMInstruction()) &&
           "Missing LLVM Instruction for HLInst.");

    // VPCmpInst for HLInst representing a CmpInst, HLIfs (single and
    // multi-predicate) and HLLoops (bottom test).
    if ((HInst && isa<CmpInst>(HInst->getLLVMInstruction())) ||
        isa<HLIf>(DDNode) || isa<HLLoop>(DDNode))
      return Builder.createCmpInst(nullptr, nullptr,
                                   getPredicateFromHIR(DDNode), DDNode);

    // Generic VPInstruction
    assert(HInst && "Expected HLInst.");
    return cast<VPInstruction>(Builder.createNaryOp(
        HInst->getLLVMInstruction()->getOpcode(), {}, DDNode));
  }

  // Temporal representation for external uses.
  return cast<VPInstruction>(Builder.createNaryOp(0 /*Opcode*/, {}));
}

// Returns the VPValue that defines Edge's sink.
VPValue *VPDecomposerHIR::createOrGetVPDefFrom(const DDEdge *Edge) {
  // Get the HLDDNode causing the definition.
  HLDDNode *DefNode = Edge->getSrc()->getHLDDNode();
  auto VPValIt = HLDef2VPValue.find(DefNode);

  // Return the VPValue associated to the HLDDNode definition if it has been
  // visited previously.
  if (VPValIt != HLDef2VPValue.end())
    return VPValIt->second;

  // HLDDNode definition hasn't been visited yet. Create VPInstruction without
  // operands and do not insert it in the VPBasicBlock. This VPInstruciton will
  // be fixed and inserted when the HLDDNode definition is processed in
  // createVPInstructionsForRange.
  VPValue *NewVPInst = createNoOperandVPInst(DefNode, false /*Insert VPIntr*/);
  HLDef2VPValue[DefNode] = NewVPInst;
  return NewVPInst;
}

// Return the Constant representation of a constant RegDDRef.
static Constant *getConstantFromHIR(RegDDRef *RDDRef) {
  assert(RDDRef->getSingleCanonExpr() &&
         "Constant CanonExpr that is not Single CanonExpr?");
  CanonExpr *CExpr = RDDRef->getSingleCanonExpr();

  if (CExpr->isIntConstant()) {
    int64_t CECoeff = CExpr->getConstant();
    Type *CETy = CExpr->getDestType();

    // Null value for pointer types needs special treatment
    if (CECoeff == 0 && CETy->isPointerTy()) {
      return Constant::getNullValue(CETy);
    }
    return ConstantInt::getSigned(CETy, CECoeff);
  }

  ConstantFP *FPConst;
  if (CExpr->isFPConstant(&FPConst))
    return FPConst;

  if (CExpr->isNull())
    return ConstantPointerNull::get(cast<PointerType>(CExpr->getDestType()));

  llvm_unreachable("Unsupported HIR Constant.");
}

// Helper function that is used by 'buildVPOpsForDDNode' to create a new VPValue
// or retrieve an existing one for an HLDDNode's operand (HIROp). This function
// must only be used to create/retrieve VPValues for *HLDDNode's operands*
// and not to create regular VPInstruction's. For the latter, you should look at
// 'createOrFixVPInstr'.
VPValue *VPDecomposerHIR::createOrGetVPOperand(RegDDRef *HIROp) {
  // HIR treats an undef as a constant - TBD see if this is the right thing to
  // do. Treat an undef as a non-constant for now.
  if (HIROp->isConstant() && !HIROp->containsUndef()) {
    return Plan->getVPConstant(getConstantFromHIR(HIROp));
  }

  if (HIROp->isUnitaryBlob()) {
    // Operand represents a single temporal that doesn't need decomposition.
    // Conversions or single temporals with constant additive != 1 will not hit
    // here.
    auto OpInEdges = DDG.incoming(HIROp);

    // If operand has incoming DD edges, we need to retrieve (or create) the
    // VPValues associated to the DD sources (definition). If there are
    // multiple definitions, in addition, we introduce a semi-phi operation that
    // "blends" all the VPValue definitions.
    if (OpInEdges.begin() != OpInEdges.end()) {
      if (std::next(OpInEdges.begin()) == OpInEdges.end())
        // Single definition.
        return createOrGetVPDefFrom(*OpInEdges.begin());

      // Multiple definitions.
      SmallVector<VPValue *, 4> OpVPDefs;
      for (const DDEdge *Edge : OpInEdges) {
        OpVPDefs.push_back(createOrGetVPDefFrom(Edge));
      }

      return Builder.createSemiPhiOp(OpVPDefs);
    }

    // Operand has no incoming DD edges. This means that HIROp is a use whose
    // definition is outside VPlan.
    VPValue *NewVPVal =
        createNoOperandVPInst(nullptr /*No opcode*/, false /*Insert VPInst*/);
    Plan->addExternalDef(cast<VPInstruction>(NewVPVal));
    return NewVPVal;
  }

  // Operand is a complex RegDDRef that needs decomposition. As it may contain
  // different temps (uses), we cannot introduce them into the VPValue U-D
  // chain until decomposition happens. For that reason, we use a VPValue to
  // mark that operand needs to be fixed later.
  // FIXME: This memory is not being freed. It will be fixed when introducing
  // decomposition.
  return new VPValue();
}

// Return a sequence of VPValues (VPValueOps) that represents DDNode's operands
// in VPlan. In addition to the RegDDRef to VPValue translation, operands are
// sorted in the way VPlan expects them. Some operands, such as the LHS operand
// in some HIR instructions, are ignored because they are not explicitly
// represented as an operand in VPlan.
void VPDecomposerHIR::buildVPOpsForDDNode(
    HLDDNode *DDNode, SmallVectorImpl<VPValue *> &VPValueOps) {

  auto *HInst = dyn_cast<HLInst>(DDNode);
  bool IsStore =
      HInst && HInst->getLLVMInstruction()->getOpcode() == Instruction::Store;

  // Collect operands necessary to build a VPInstruction out of an HLInst and
  // translate them into VPValue's.
  for (RegDDRef *HIROp :
       make_range(DDNode->op_ddref_begin(), DDNode->op_ddref_end())) {
    // We skip LHS operands for most instructions.
    if (HIROp->isLval() && !IsStore)
      continue;

    VPValueOps.push_back(createOrGetVPOperand(HIROp));
  }

  // Fix discrepancies in the order of operands between HLInst and
  // VPInstruction:
  //     - Store: dest = store src -> store src dest
  if (IsStore)
    std::iter_swap(VPValueOps.begin(), std::next(VPValueOps.begin()));
}

// Main interface to decompose an HLDDNode into VPInstructions. The resulting
// VPInstructions are inserted at the end of \p InsertPoint VPBasicBlock and
// added to the HLDef2VPValue map.
VPInstruction *
VPDecomposerHIR::createVPInstructions(HLDDNode *DDNode,
                                      VPBasicBlock *InsPointVPBB) {
  LLVM_DEBUG(dbgs() << "Generating VPInstructions for "; DDNode->dump();
             dbgs() << "\n");

  // Set the insertion point in the builder for the VPInstructions that we are
  // going to create for this DDNode.
  Builder.setInsertPoint(InsPointVPBB);

  // Translate HIR operands into VPValue operands. This needs to happen before
  // creating the VPInstruction because it may introduce new VPInstructions for
  // operands (e.g., semi-phis).
  SmallVector<VPValue *, 4> VPValueOps;
  buildVPOpsForDDNode(DDNode, VPValueOps);

  auto VPValIt = HLDef2VPValue.find(DDNode);
  VPInstruction *NewVPInst;

  if (VPValIt != HLDef2VPValue.end()) {
    // DDNode is a definition with a user that has been previously visited. We
    // have to set its operands properly and insert it into the VPBasicBlock.
    NewVPInst = cast<VPInstruction>(VPValIt->second);
    Builder.insert(NewVPInst);
  } else {
    // Create new VPInstruction. We set operands later to factorize code in 'if'
    // and 'else' branches and reuse 'createNoOperandVPInst'.
    NewVPInst = createNoOperandVPInst(DDNode, true /*Insert VPInst*/);
    HLDef2VPValue[DDNode] = NewVPInst;
  }

  // Set VPInstruction's operands.
  for (VPValue *Operand : VPValueOps) {
    NewVPInst->addOperand(Operand);
  }

  return NewVPInst;
}
