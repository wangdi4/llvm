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
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRParser.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"

#define DEBUG_TYPE "vplan-decomposer"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::loopopt;

// \brief RAII object that stores the current master VPInstruction under
// decomposition in VPDecomposerHIR and restores it when the object is
// destroyed.
class llvm::vpo::MasterVPIGuard {
  VPDecomposerHIR &Decomposer;
  VPInstruction *MasterVPI;

public:
  MasterVPIGuard(VPDecomposerHIR &Dec, VPInstruction *NewMasterVPI)
      : Decomposer(Dec), MasterVPI(Dec.MasterVPI) {
    Decomposer.MasterVPI = NewMasterVPI;
  }

  MasterVPIGuard(const MasterVPIGuard &) = delete;
  MasterVPIGuard &operator=(const MasterVPIGuard &) = delete;

  ~MasterVPIGuard() { Decomposer.MasterVPI = MasterVPI; }
};

// Creates a decomposed VPInstruction that combines \p LHS and \p RHS VPValues
// using \p OpCode as operator and \p MasterVPI as master VPInstruction. If \p
// LHS or \p RHS is null, it returns the non-null VPValue.
VPValue *VPDecomposerHIR::combineDecompDefs(VPValue *LHS, VPValue *RHS,
                                            Type *Ty, unsigned OpCode) {
  assert((LHS != nullptr || RHS != nullptr) && "LHS and RHS cannot be nullptr");

  if (LHS == nullptr)
    return RHS;
  if (RHS == nullptr)
    return LHS;

  auto *NewVPI = cast<VPInstruction>(Builder.createNaryOp(OpCode, {LHS, RHS}));
  NewVPI->HIR.setMaster(MasterVPI);
  return NewVPI;
}

// Create a VPConstant for an integer coefficient.
VPConstant *VPDecomposerHIR::decomposeCoeff(int64_t Coeff, Type *Ty) {
  assert((Ty->isIntegerTy() || Ty->isPointerTy()) &&
         "Expected integer or pointer type for coefficient.");
  // Null value for pointer types needs special treatment
  if (Coeff == 0 && Ty->isPointerTy())
    return Plan->getVPConstant(Constant::getNullValue(Ty));

  return Plan->getVPConstant(ConstantInt::getSigned(Ty, Coeff));
}

// Create a VPInstruction with \p Src as source operand, \p ConvOpCode as
// conversion opcode (ZExt, SExt or Trunc) and \p DestType as destination type.
// TODO: \p DestType is not used at this point since VPInstruction doesn't have
// type representation.
VPInstruction *VPDecomposerHIR::decomposeConversion(VPValue *Src,
                                                    unsigned ConvOpCode,
                                                    Type *DestType) {
  assert((ConvOpCode == Instruction::ZExt || ConvOpCode == Instruction::SExt ||
          ConvOpCode == Instruction::Trunc) &&
         "Unexpected conversion OpCode.");

  // TODO: We need to set the conversion type (DestType)!
  auto *NewConv = cast<VPInstruction>(Builder.createNaryOp(ConvOpCode, {Src}));
  NewConv->HIR.setMaster(MasterVPI);
  return NewConv;
}

// Create a VPInstruction for the conversion of \p CE, if any, using \p Src as a
// source operand. Return \p Src if \p CE doesn't have conversion.
VPValue *VPDecomposerHIR::decomposeCanonExprConv(CanonExpr *CE, VPValue *Src) {
  if (CE->getDestType() == CE->getSrcType())
    return Src;

  if (CE->isZExt())
    return decomposeConversion(Src, Instruction::ZExt, CE->getDestType());
  if (CE->isSExt())
    return decomposeConversion(Src, Instruction::SExt, CE->getDestType());
  if (CE->isTrunc())
    return decomposeConversion(Src, Instruction::Trunc, CE->getDestType());

  llvm_unreachable("Unsupported conversion in VPlan decomposer!");
}

// Decompose a blob given its \p BlobIdx and \p BlobCoeff. Return the last
// VPValue resulting from its decomposition.
VPValue *VPDecomposerHIR::decomposeBlob(RegDDRef *RDDR, unsigned BlobIdx,
                                        int64_t BlobCoeff) {
  BlobTy Blob = RDDR->getBlobUtils().getBlob(BlobIdx);

  // Decompose Blob.
  VPBlobDecompVisitor BlobDecomp(*RDDR, *this);
  VPValue *DecompBlob = BlobDecomp.visit(Blob);

  if (BlobCoeff != 1) {
    // Create VPInstruction for Coeff * blob.
    Type *BlobTy = Blob->getType();
    Type *CoeffType;

    if (BlobTy->isPointerTy()) {
      // If blob has pointer type, coeff must be integer.
      // If coeff != 1 and blob type is pointer, only -1 coeff is allowed for
      // now.
      assert((BlobCoeff == -1) &&
             "Unexpected blob coefficient for pointer type.");

      unsigned PointerSize =
          RDDR->getDDRefUtils().getDataLayout().getPointerTypeSizeInBits(
              BlobTy);
      switch (PointerSize) {
      case 64:
        CoeffType = Type::getInt64Ty(BlobTy->getContext());
        break;
      case 32:
        CoeffType = Type::getInt32Ty(BlobTy->getContext());
        break;
      default:
        llvm_unreachable("Unexpected pointer size.");
      }
    } else
      CoeffType = Blob->getType();

    VPValue *BlobCoeffValue = decomposeCoeff(BlobCoeff, CoeffType);
    DecompBlob = combineDecompDefs(DecompBlob, BlobCoeffValue, Blob->getType(),
                                   Instruction::Mul);
  }

  return DecompBlob;
}

// Return the HLLoop given an IV level by traversing HLLoops starting from the
// HLLoop parent of \p DDR.
static HLLoop *getHLLoopForLevel(DDRef *DDR, unsigned IVLevel) {
  HLLoop *ParentLp = DDR->getParentLoop();
  assert(ParentLp && "Expected parent HLLoop for DDRef!");

  while (ParentLp->getNestingLevel() != IVLevel) {
    ParentLp = ParentLp->getParentLoop();
    assert(ParentLp && "HLLoop not found for IV level!");
  }

  return ParentLp;
}

// Decompose the IV sub-expression of a CanonExpr. Return the last VPValue
// resulting from its decomposition.
VPValue *VPDecomposerHIR::decomposeIV(RegDDRef *RDDR, CanonExpr *CE,
                                      unsigned IVLevel, Type *Ty) {
  int64_t IVConstCoeff;
  unsigned IVBlobIndex;
  CE->getIVCoeff(IVLevel, &IVBlobIndex, &IVConstCoeff);

  VPValue *DecompIV = nullptr;

  if (IVBlobIndex != InvalidBlobIndex)
    // Create VPInstruction for blob * IV.
    // The blob coefficient 1 because the IV coefficient is IVConstCoeff and
    // it's processed later.
    DecompIV = decomposeBlob(RDDR, IVBlobIndex, 1 /*BlobCoeff*/);

  if (IVConstCoeff != 1) {
    VPValue *DecompIVCoeff = decomposeCoeff(IVConstCoeff, Ty);
    DecompIV = combineDecompDefs(DecompIVCoeff, DecompIV, Ty, Instruction::Mul);
  }

  VPValue *VPIndVar = HLLp2IVSemiPhi[getHLLoopForLevel(RDDR, IVLevel)];
  if (!VPIndVar) {
    // If there is no semi-phi in the map, it means that the IV is an external
    // definition.
    // TODO: We could be creating redundant external definitions here because
    // this external definition cannot be mapped to an HLInst. Add check at the
    // beginning of this function to return an existing external definition in
    // the VPlan pool.
    VPIndVar = new VPValue();
    Plan->addExternalDef(VPIndVar);
  }

  DecompIV = combineDecompDefs(DecompIV, VPIndVar, Ty, Instruction::Mul);
  return DecompIV;
}

// Decompose a CanonExpr. Return the last VPValue resulting from its
// decomposition.
VPValue *VPDecomposerHIR::decomposeCanonExpr(RegDDRef *RDDR, CanonExpr *CE) {
  LLVM_DEBUG(dbgs() << "  Decomposing CanonExpr: "; CE->dump(); dbgs() << "\n");
  VPValue *DecompDef = nullptr;

  // Decompose blobs.
  for (auto BlobIt = CE->blob_begin(); BlobIt != CE->blob_end(); ++BlobIt) {
    unsigned BlobIdx = CE->getBlobIndex(BlobIt);
    assert(BlobIdx != InvalidBlobIndex && "Invalid blob index!");

    int64_t BlobCoeff = CE->getBlobCoeff(BlobIdx);
    assert(BlobCoeff != 0 && "Invalid blob coefficient!");

    VPValue *DecompBlob = decomposeBlob(RDDR, BlobIdx, BlobCoeff);
    DecompDef = combineDecompDefs(DecompDef, DecompBlob, CE->getSrcType(),
                                  Instruction::Add);
  }

  // Decompose IV expression.
  for (auto IVIt = CE->iv_begin(), E = CE->iv_end(); IVIt != E; ++IVIt) {
    int64_t IVConstCoeff = CE->getIVConstCoeff(IVIt);

    if (IVConstCoeff != 0) {
      VPValue *DecompIV =
          decomposeIV(RDDR, CE, CE->getLevel(IVIt), CE->getSrcType());
      DecompDef = combineDecompDefs(DecompDef, DecompIV, CE->getSrcType(),
                                    Instruction::Add);
    }
  }

  // Decompose constant additive. If it's 0, we ignore it when the CE has more
  // components (e.g., X + 0). Otherwise, CE is representing the constant 0 and
  // we have to generate a VPValue for it.
  int64_t AddCoeff = CE->getConstant();
  if (AddCoeff != 0 || !DecompDef) {
    VPValue *DecompCoeff = decomposeCoeff(AddCoeff, CE->getSrcType());
    DecompDef = combineDecompDefs(DecompDef, DecompCoeff, CE->getSrcType(),
                                  Instruction::Add);
  }

  // Decompose denominator.
  int64_t Denominator = CE->getDenominator();
  if (Denominator != 1) {
    VPValue *DecompDenom = decomposeCoeff(Denominator, CE->getSrcType());
    DecompDef = combineDecompDefs(DecompDef, DecompDenom, CE->getSrcType(),
                                  CE->isUnsignedDiv() ? Instruction::UDiv
                                                      : Instruction::SDiv);
  }

  // Decompose conversions.
  DecompDef = decomposeCanonExprConv(CE, DecompDef);

  assert(DecompDef && "CanonExpr has not been decomposed");
  return DecompDef;
}

// Decompose the RegDDRef operand of an HLDDNode. Return the last VPValue
// resulting from its decomposition.
VPValue *VPDecomposerHIR::decomposeVPOperand(RegDDRef *RDDR) {
  assert(RDDR && "Expected a valid RegDDRef.");
  if (RDDR->isTerminalRef())
    return decomposeCanonExpr(RDDR, RDDR->getSingleCanonExpr());

  // Memory ops
  return new VPValue(); // TODO: decomposeMemoryOp(AVal);
}

// Utility function that returns a CmpInst::Predicate for a given DDNode. The
// return value is in the context of the *plain* CFG construction:
//   1) HLInst representing a CmpInst -> CmpInst's opcode.
//   2) Single-predicate HLIf -> HLIf single predicate.
//   3) Multi-predicate HLIf -> BAD_ICMP_PREDICATE (to be fixed during
//      decomposition).
//      TODO: Multi-predicate HLIfs should be properly decomposed and its logic
//      shouldn't be necessary.
//   4) HLLoop -> ICMP_SLE or ICMP_ULE (bottom test).
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
// complex HIR constructs:
//     1) HLIf: The VPCmpInst represents the multi-predicate comparisons and
//        contains all the operands involved in the comparisons.
//     2) HLLoop: The VPCmpInst represents the bottom test comparison. It
//        contains the HLLoop lower-bound, the upper-bound and the step.
// Please, note that the previous semantics are only valid during *plain* CFG
// constructions and may become invalid after HIR decomposition. Do not reuse
// this code for other purposes.
VPInstruction *VPDecomposerHIR::createNoOperandVPInst(HLDDNode *DDNode,
                                                      bool InsertVPInst) {
  assert(DDNode && "Expected DDNode to create a VPInstruction.");

  // Clear the insertion point if we don't have to insert the new VPInstruction.
  VPBuilder::InsertPointGuard Guard(Builder);
  if (!InsertVPInst)
    Builder.clearInsertionPoint();

  HLInst *HInst = dyn_cast<HLInst>(DDNode);
  assert((!HInst || HInst->getLLVMInstruction()) &&
         "Missing LLVM Instruction for HLInst.");

  // Create VPCmpInst for HLInst representing a CmpInst, HLIfs and HLLoops
  // (bottom test).
  if ((HInst && isa<CmpInst>(HInst->getLLVMInstruction())) ||
      isa<HLIf>(DDNode) || isa<HLLoop>(DDNode)) {
    VPCmpInst *NewCmp = Builder.createCmpInst(
        nullptr, nullptr, getPredicateFromHIR(DDNode), DDNode);
    HLDef2VPValue[DDNode] = NewCmp;
    return NewCmp;
  }

  // Generic VPInstruction.
  assert(HInst && HInst->getLLVMInstruction() &&
         "Expected HLInst with underlying LLVM IR.");
  auto *NewVPInst = cast<VPInstruction>(Builder.createNaryOp(
      HInst->getLLVMInstruction()->getOpcode(), {}, DDNode));
  HLDef2VPValue[DDNode] = NewVPInst;
  return NewVPInst;
}

// Return a sequence of VPValues (VPValueOps) that represents DDNode's operands
// in VPlan. In addition to the RegDDRef to VPValue translation, operands are
// sorted in the way VPlan expects them. Some operands, such as the LHS operand
// in some HIR instructions, are ignored because they are not explicitly
// represented as an operand in VPlan.
void VPDecomposerHIR::createVPOperandsForMasterVPInst(
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

    VPValueOps.push_back(decomposeVPOperand(HIROp));
  }

  // Fix discrepancies in the order of operands between HLInst and
  // VPInstruction:
  //     - Store: dest = store src -> store src dest
  if (IsStore)
    std::iter_swap(VPValueOps.begin(), std::next(VPValueOps.begin()));
}

void VPDecomposerHIR::createLoopIVAndIVStart(HLLoop *HLp, VPBasicBlock *LpPH) {
  assert(HLp->isDo() && HLp->isNormalized() &&
         "Only normalized single-exit DO loops are supported for now.");
  assert(LpPH->getSingleSuccessor() &&
         isa<VPBasicBlock>(LpPH->getSingleSuccessor()) &&
         "Loop PH must have one successor VPBasicBlock.");
  VPBasicBlock *LpH = cast<VPBasicBlock>(LpPH->getSingleSuccessor());

  // Create IV start (0). Only normalized loops are expected.
  CanonExpr *LowerCE = HLp->getLowerCanonExpr();
  assert(LowerCE->isZero() && "Expected normalized IV.");
  assert(LowerCE->getDestType() == HLp->getIVType() &&
         "Lower bound and IV type doesn't match.");
  VPConstant *IVStart = Plan->getVPConstant(
      ConstantInt::getSigned(LowerCE->getDestType(), LowerCE->getConstant()));
  // TODO: Attach HLp as underlying HIR of the IV Start and set HIR to valid.

  // Create induction phi only with IVStart. We will add IVNext in a separate
  // step. Insert it at the beginning of the loop header and map it to the loop
  // level.
  VPBuilder::InsertPointGuard Guard(Builder);
  Builder.setInsertPoint(LpH, LpH->begin());
  VPInstruction *IndSemiPhi =
      cast<VPInstruction>(Builder.createSemiPhiOp({IVStart}, HLp));
  assert(!HLLp2IVSemiPhi.count(HLp) && "HLLoop has multiple IVs?");
  HLLp2IVSemiPhi[HLp] = IndSemiPhi;
  IndSemiPhi->HIR.setValid();
}

VPValue *VPDecomposerHIR::createLoopIVNextAndBottomTest(HLLoop *HLp,
                                                        VPBasicBlock *LpPH,
                                                        VPBasicBlock *LpLatch) {
  // Retrieve the inductive semi-phi (IV) generated for this HLLoop.
  assert(HLLp2IVSemiPhi.count(HLp) &&
         "Expected semi-phi VPInstruction for HLLoop.");
  VPInstruction *IndSemiPhi = HLLp2IVSemiPhi[HLp];

  // Create IV next. Only normalized loops are expected so we use step 1.
  assert(HLp->getStrideCanonExpr()->isOne() &&
         "Expected positive unit-stride HLLoop.");
  Builder.setInsertPoint(LpLatch);
  auto *IVNext = cast<VPInstruction>(Builder.createAdd(
      IndSemiPhi,
      Plan->getVPConstant(ConstantInt::getSigned(HLp->getIVType(), 1)), HLp));

  // Add IVNext to induction semi-phi.
  IndSemiPhi->addOperand(IVNext);

  // Create VPInstruction for bottom test condition. We 1) create the
  // VPInstruction for the test condition without operands, 2) decompose UB
  // operand setting the VPInstruction as master VPInstruction and 3) add
  // operands to the VPInstruction. The VPInstructions resulting from the UB
  // decomposition are inserted into the loop PH since they are loop invariant.
  assert(HLp->getUpperDDRef() && "Expected a valid upper DDRef for HLLoop.");
  auto *BottomTest = createNoOperandVPInst(HLp, LpLatch);
  BottomTest->addOperand(IVNext);
  VPBuilder::InsertPointGuard Guard(Builder);
  Builder.setInsertPoint(LpPH);
  MasterVPIGuard MasterGuard(*this, BottomTest);
  BottomTest->addOperand(decomposeVPOperand(HLp->getUpperDDRef()));

  // Set the underlying HIR of the new VPInstructions to valid.
  IndSemiPhi->HIR.setValid();
  IVNext->HIR.setValid();
  BottomTest->HIR.setValid();
  return BottomTest;
}

VPInstruction *
VPDecomposerHIR::createVPInstructions(HLDDNode *DDNode,
                                      VPBasicBlock *InsPointVPBB) {
  LLVM_DEBUG(dbgs() << "Generating VPInstructions for "; DDNode->dump();
             dbgs() << "\n");

  // Set the insertion point in the builder for the VPInstructions that we are
  // going to create for this DDNode.
  Builder.setInsertPoint(InsPointVPBB);

  auto VPValIt = HLDef2VPValue.find(DDNode);
  VPInstruction *NewVPInst;

  if (VPValIt != HLDef2VPValue.end()) {
    // DDNode is a definition with a user that has been previously visited. We
    // have to set its operands properly and insert it into the VPBasicBlock.
    NewVPInst = cast<VPInstruction>(VPValIt->second);
    Builder.insert(NewVPInst);
  } else
    // Create new VPInstruction without operands. Operands are created later. We
    // need to follow this order because this VPInstruction needs to be set as
    // master VPInstruction of its operands.
    NewVPInst = createNoOperandVPInst(DDNode, true /*Insert VPInst*/);

  // Create and decompose the operands of the new VPInstruction. We insert the
  // operands before the new VPInstruction the belong to. We use the new
  // VPInstruction as master VPInstruction of the decomposed operands.
  VPBuilder::InsertPointGuard BuilderGuard(Builder);
  Builder.setInsertPoint(NewVPInst);
  MasterVPIGuard MasterGuard(*this, NewVPInst);

  SmallVector<VPValue *, 4> VPValueOps;
  createVPOperandsForMasterVPInst(DDNode, VPValueOps);

  // Set VPInstruction's operands.
  for (VPValue *Operand : VPValueOps) {
    NewVPInst->addOperand(Operand);
  }

  NewVPInst->HIR.setValid();
  return NewVPInst;
}

// Create or retrieve an existing VPValue for the definition of \p Edge. Return
// an external definition if the \p Edge definition is outside of the outermost
// loop in VPlan. Otherwise, return a VPInstruction.
VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::createOrGetVPDefFor(const DDEdge *Edge) {
  // Get the HLDDNode causing the definition.
  HLDDNode *DefNode = Edge->getSrc()->getHLDDNode();
  auto VPValIt = Decomposer.HLDef2VPValue.find(DefNode);

  if (VPValIt != Decomposer.HLDef2VPValue.end())
    // Return the VPValue associated to the HLDDNode definition created
    // previously.
    return VPValIt->second;

  if (!DefNode->getHLNodeUtils().contains(Decomposer.OutermostHLp, DefNode,
                                          true /*include preheader/exit*/)) {
    // The definition is outside of the outermost loop. Create an external
    // definition.
    VPValue *VPExtDef = new VPValue();
    Decomposer.Plan->addExternalDef(VPExtDef);
    Decomposer.HLDef2VPValue[DefNode] = VPExtDef;
    return VPExtDef;
  }

  // HLDDNode definition hasn't been visited yet. Create VPInstruction without
  // operands and do not insert it in the VPBasicBlock. This VPInstruction will
  // be fixed and inserted when the HLDDNode definition is processed in
  // createVPInstructions.
  return Decomposer.createNoOperandVPInst(DefNode, false /*Insert VPIntr*/);
}

// Create a VPValue for a non-integer constant \p Blob. A non-integer constant
// blob can be a floating point or an undef.
VPConstant *VPDecomposerHIR::VPBlobDecompVisitor::decomposeNonIntConstBlob(
    const SCEVUnknown *Blob) {
  BlobUtils &BlUtils = RDDR.getBlobUtils();
  assert(BlUtils.isConstantDataBlob(Blob) && "Expected a ConstantDataBlob.");
  (void)BlUtils;

  ConstantFP *FPConst;
  if (BlUtils.isConstantFPBlob(Blob, &FPConst))
    return Decomposer.Plan->getVPConstant(FPConst);

  if (BlUtils.isUndefBlob(Blob))
    return Decomposer.Plan->getVPConstant(UndefValue::get(Blob->getType()));

  llvm_unreachable("Unsupported non-integer HIR Constant.");
}

// Create a VPValue for a standalone blob given its SCEV. A standalone blob is
// unitary and doesn't need decomposition.
VPValue *VPDecomposerHIR::VPBlobDecompVisitor::decomposeStandAloneBlob(
    const SCEVUnknown *Blob) {

  if (RDDR.getBlobUtils().isConstantDataBlob(Blob))
    // Decompose constant blobs that are not integer values.
    return decomposeNonIntConstBlob(Blob);

  // If the RegDDRef is a self blob, we use it directly to retrieve the DDG
  // edges of Blob because there is no BlobDDRef. Otherwise, we retrieve and use
  // the BlobDDRef.
  DDRef *DDR;
  if (RDDR.isSelfBlob())
    DDR = &RDDR;
  else {
    unsigned BlobIndex = RDDR.getBlobUtils().findBlob(Blob);
    assert(BlobIndex != InvalidBlobIndex && "SCEV is not a Blob");
    DDR = RDDR.getBlobDDRef(BlobIndex);
    assert(DDR != nullptr && "BlobDDRef not found!");
  }

  auto BlobInEdges = Decomposer.DDG.incoming(DDR);
  if (BlobInEdges.begin() != BlobInEdges.end()) {
    // Blob has incoming DD edges. We need to retrieve (or create) the VPValues
    // associated to the DD sources (definitions). If there are multiple
    // definitions, in addition, we introduce a semi-phi operation that "blends"
    // all the VPValue definitions.

    if (std::next(BlobInEdges.begin()) == BlobInEdges.end())
      // Single definition.
      return createOrGetVPDefFor(*BlobInEdges.begin());

    // Multiple definitions.
    SmallVector<VPValue *, 4> BlobVPDefs;
    for (const DDEdge *Edge : BlobInEdges)
      BlobVPDefs.push_back(createOrGetVPDefFor(Edge));

    auto *NewSemiPhi =
        cast<VPInstruction>(Decomposer.Builder.createSemiPhiOp(BlobVPDefs));
    NewSemiPhi->HIR.setMaster(Decomposer.MasterVPI);
    return NewSemiPhi;
  }

  // Blob has no incoming DD edges. This means that it is an external definition
  // that is not even part of the DD graph.
  // TODO: We could be creating redundant external definitions here because this
  // external definition cannot be mapped to an HLInst. Add check at the
  // beginning of this function to return an existing external definition in the
  // VPlan pool.
  VPValue *VPExtDef = new VPValue();
  Decomposer.Plan->addExternalDef(VPExtDef);
  return VPExtDef;
}

// Helper function to decomposed an SCEVNAryExpr using the same \p OpCode to
// combine all the sub-expressions.
VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::decomposeNAryOp(const SCEVNAryExpr *Blob,
                                                      unsigned OpCode) {
  VPValue *DecompDef = nullptr;
  for (auto *SCOp : Blob->operands()) {
    VPValue *VPOp = visit(SCOp);
    DecompDef =
        Decomposer.combineDecompDefs(VPOp, DecompDef, Blob->getType(), OpCode);
  }

  return DecompDef;
}

VPValue *VPDecomposerHIR::VPBlobDecompVisitor::visitConstant(
    const SCEVConstant *Constant) {
  return Decomposer.Plan->getVPConstant(Constant->getValue());
}

VPValue *VPDecomposerHIR::VPBlobDecompVisitor::visitTruncateExpr(
    const SCEVTruncateExpr *Expr) {
  VPValue *Src = visit(Expr->getOperand());
  return Decomposer.decomposeConversion(Src, Instruction::Trunc,
                                        Expr->getType());
}

VPValue *VPDecomposerHIR::VPBlobDecompVisitor::visitZeroExtendExpr(
    const SCEVZeroExtendExpr *Expr) {
  VPValue *Src = visit(Expr->getOperand());
  return Decomposer.decomposeConversion(Src, Instruction::ZExt,
                                        Expr->getType());
}

VPValue *VPDecomposerHIR::VPBlobDecompVisitor::visitSignExtendExpr(
    const SCEVSignExtendExpr *Expr) {
  VPValue *Src = visit(Expr->getOperand());
  return Decomposer.decomposeConversion(Src, Instruction::SExt,
                                        Expr->getType());
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitAddExpr(const SCEVAddExpr *Expr) {
  return decomposeNAryOp(Expr, Instruction::Add);
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitMulExpr(const SCEVMulExpr *Expr) {
  return decomposeNAryOp(Expr, Instruction::Mul);
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitUDivExpr(const SCEVUDivExpr *Expr) {
  VPValue *DivLHS = visit(Expr->getLHS());
  VPValue *DivRHS = visit(Expr->getRHS());
  return Decomposer.combineDecompDefs(DivLHS, DivRHS, Expr->getType(),
                                      Instruction::UDiv);
}

VPValue *VPDecomposerHIR::VPBlobDecompVisitor::visitAddRecExpr(
    const SCEVAddRecExpr *Expr) {
  llvm_unreachable("Expected add-recs to be broken by canon-expr.");
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitSMaxExpr(const SCEVSMaxExpr *Expr) {
  return decomposeNAryOp(Expr, VPInstruction::SMax);
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitUMaxExpr(const SCEVUMaxExpr *Expr) {
  return decomposeNAryOp(Expr, VPInstruction::UMax);
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitUnknown(const SCEVUnknown *Expr) {
  return decomposeStandAloneBlob(Expr);
}

VPValue *VPDecomposerHIR::VPBlobDecompVisitor::visitCouldNotCompute(
    const SCEVCouldNotCompute *Expr) {
  llvm_unreachable("Attempt to use a SCEVCouldNotCompute object.");
}
