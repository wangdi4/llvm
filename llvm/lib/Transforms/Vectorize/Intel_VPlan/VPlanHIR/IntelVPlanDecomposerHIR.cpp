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
  assert((HLp->isDo() || HLp->isDoMultiExit()) && HLp->isNormalized() &&
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

// Return true if \p Def is considered an external definition. An external
// definition is a definition that happens outside of the outermost HLLoop,
// including its preheader and exit. A special kind of operands that fits into
// this category is metadata operands.
bool VPDecomposerHIR::isExternalDef(DDRef *UseDDR) {
  // TODO: We are pushing outermost loop PH and Exit outside of the VPlan region
  // for now so this code won't be valid until we bring them back. return
  // !Def->getHLNodeUtils().contains(OutermostHLp, Def,
  //                                 true /*include preheader/exit*/);
  assert(UseDDR->isRval() && "DDRef must be an RValue!");

  // Check if UseDDR is metadata.
  if (UseDDR->isMetadata())
    return true;

  return OutermostHLp->isLiveIn(UseDDR->getSymbase());
}

// Return the number of reaching definitions for \p DDR. Reaching definitions
// are computed based on the edges of the DDGraph of the outermost loop that we
// are representing, plus one, if \p DDR is a live-in value of the outermost
// loop.
unsigned VPDecomposerHIR::getNumReachingDefinitions(DDRef *UseDDR) {
  assert(UseDDR->isRval() && "DDRef must be an RValue!");

  if (UseDDR->isMetadata())
    // Metadata is considered a external definition. I has a single definition
    // since a metadata operand doesn't have DD edges.
    return 1;

  assert((UseDDR->isSelfBlob() || isa<BlobDDRef>(UseDDR)) &&
         "Expected self blob or BlobDDRef!");

  auto BlobInEdges = DDG.incoming(UseDDR);
  return std::distance(BlobInEdges.begin(), BlobInEdges.end()) +
         OutermostHLp->isLiveIn(UseDDR->getSymbase());
}

// Return a pointer to the last VPInstruction of \p VPBB. Return nullptr if \p
// VPBB is empty.
static VPInstruction *getLastVPI(VPBasicBlock *VPBB) {
  assert((VPBB->empty() || isa<VPInstruction>(&VPBB->back())) &&
         "VPRecipes in HIR?");
  return VPBB->empty() ? nullptr : cast<VPInstruction>(&VPBB->back());
}

// Set \p MasterVPI as master VPInstruction of all the decomposed VPInstructions
// between \p LastVPIBeforeDec and the own MasterVPI. If LastVPIBeforeDec is
// null, the first decomposed VPInstruction is the first one in \p VPBB.
void VPDecomposerHIR::setMasterForDecomposedVPIs(
    VPInstruction *MasterVPI, VPInstruction *LastVPIBeforeDec,
    VPBasicBlock *VPBB) {
  assert(MasterVPI->getParent() == VPBB && "MasterVPI must be in VPBB.");
  assert((!LastVPIBeforeDec || LastVPIBeforeDec->getParent() == VPBB) &&
         "LastVPIBeforeDec must be in VPBB.");

  VPBasicBlock::iterator DecompVPIStart =
      LastVPIBeforeDec == nullptr
          ? VPBB->begin()
          : std::next(VPBasicBlock::iterator(LastVPIBeforeDec));
  VPBasicBlock::iterator DecompVPIEnd = VPBasicBlock::iterator(MasterVPI);
  for (auto &Recipe : make_range(DecompVPIStart, DecompVPIEnd)) {
    assert(isa<VPInstruction>(Recipe) && "VPRecipes in HIR?");
    auto *DecompVPI = cast<VPInstruction>(&Recipe);
    // DecompVPI is a new VPInstruction at this point. To be marked as
    // decomposed VPInstruction with the following 'setMaster'.
    assert(DecompVPI->isNew() && "Expected new VPInstruction!");
    DecompVPI->HIR.setMaster(MasterVPI);
  }
}

// Create VPInstruction for \p DDNode and insert it in VPBuilder's insertion
// point. If \p DDNode is an HLIf, create a VPCmpInst representing the
// (multi-)predicate comparisons. HLLoop are not expected.
VPInstruction *
VPDecomposerHIR::createVPInstruction(HLDDNode *DDNode,
                                     ArrayRef<VPValue *> VPOperands) {
  assert(DDNode && "Expected DDNode to create a VPInstruction.");
  assert(!isa<HLLoop>(DDNode) && "HLLoop shouldn't be processed here!");
  HLInst *HInst = dyn_cast<HLInst>(DDNode);
  assert((!HInst || HInst->getLLVMInstruction()) &&
         "Missing LLVM Instruction for HLInst.");

  // Create VPCmpInst for HLInst representing a CmpInst or HLIf.
  VPInstruction *NewVPInst;
  if ((HInst && isa<CmpInst>(HInst->getLLVMInstruction())) ||
      isa<HLIf>(DDNode)) {
    assert(VPOperands.size() == 2 && "Expected 2 operands in CmpInst.");
    NewVPInst = Builder.createCmpInst(VPOperands[0], VPOperands[1],
                                      getPredicateFromHIR(DDNode), DDNode);
  } else {
    // Generic VPInstruction.
    assert(HInst && HInst->getLLVMInstruction() &&
           "Expected HLInst with underlying LLVM IR.");
    NewVPInst = cast<VPInstruction>(Builder.createNaryOp(
        HInst->getLLVMInstruction()->getOpcode(), VPOperands, DDNode));
  }

  HLDef2VPValue[DDNode] = NewVPInst;
  return NewVPInst;
}

// Return a sequence of VPValues (VPOperands) that represents DDNode's operands
// in VPlan. In addition to the RegDDRef to VPValue translation, operands are
// sorted in the way VPlan expects them. Some operands, such as the LHS operand
// in some HIR instructions, are ignored because they are not explicitly
// represented as an operand in VPlan.
void VPDecomposerHIR::createVPOperandsForMasterVPInst(
    HLDDNode *DDNode, SmallVectorImpl<VPValue *> &VPOperands) {

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

    VPOperands.push_back(decomposeVPOperand(HIROp));
  }

  // Fix discrepancies in the order of operands between HLInst and
  // VPInstruction:
  //   - Store:
  //       HLInst: (%A)[i1] = %add;
  //       VPInstruction: store %add, %decompAi1
  if (IsStore)
    std::iter_swap(VPOperands.begin(), std::next(VPOperands.begin()));
}

// Create or retrieve an existing VPValue that represents the definition of the
// use \p UseDDR (R-value DDRef representing a use). Return an external
// definition if such definition happens outside of the outermost loop
// represented in VPlan. Otherwise, return a VPInstruction representing the
// definition.
void VPDecomposerHIR::createOrGetVPDefsForUse(
    DDRef *UseDDR, SmallVectorImpl<VPValue *> &VPDefs) {

  assert(UseDDR->isRval() && "DDRef must be an RValue!");

  // Process external definitions.
  // TODO: We are creating redundant external definitions. To be fixed with the
  // introduction of VPExternalDef class.
  if (isExternalDef(UseDDR)) {
    VPValue *VPExtDef = new VPValue();
    Plan->addExternalDef(VPExtDef);
    VPDefs.push_back(VPExtDef);
  }

  // Process definitions coming from incoming DD edges. At this point, all
  // the sources of the incoming edges of UseDDR must have an associated
  // VPInstruction modeling the definition.
  auto InEdges = DDG.incoming(UseDDR);
  for (const DDEdge *Edge : InEdges) {
    // Get the HLDDNode causing the definition.
    HLDDNode *DefNode = Edge->getSrc()->getHLDDNode();

    auto VPValIt = HLDef2VPValue.find(DefNode);
    assert(VPValIt != HLDef2VPValue.end() &&
           "Missing VPInstruction for HLDDNode!");
    VPDefs.push_back(VPValIt->second);
  }
}

void VPDecomposerHIR::createLoopIVAndIVStart(HLLoop *HLp, VPBasicBlock *LpPH) {
  assert((HLp->isDo() || HLp->isDoMultiExit()) && HLp->isNormalized() &&
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

  // Create induction phi only with IVStart. We will add IVNext in a separate
  // step. Insert it at the beginning of the loop header and map it to the loop
  // level. HLp is set as underlying HIR of the induction phi.
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
  // This is an example of the master and decomposed VPInstructions that are
  // generated for an HLLoop by this method:
  // HLLoop:
  //   <26> + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
  //   ...
  //   <26> + END LOOP
  //
  // Output:
  //   BB3: // Loop PH
  //    %vp43808 = sext %vp43744 // UB decomposition.
  //    %vp44016 = add %vp43808 i64 -1 // UB decomposition (master VPI)
  //   SUCCESSORS(1):BB4
  //
  //   BB4: // Loop H and Latch
  //    %vp40944 = semi-phi i64 0 %vp43488
  //    ...
  //    %vp43488 = add %vp40944 i64 1 // IVNext (master VPI)
  //    %vp44352 = icmp %vp43488 %vp44016 // BottomTest (master VPI)
  //   SUCCESSORS(2):BB4(%vp44352), BB5(!%vp44352)

  // Retrieve the inductive semi-phi (IV) generated for this HLLoop.
  assert(HLLp2IVSemiPhi.count(HLp) &&
         "Expected semi-phi VPInstruction for HLLoop.");
  VPInstruction *IndSemiPhi = HLLp2IVSemiPhi[HLp];

  // Create add VPInstruction for IV next. HLp is set as underlying HIR of the
  // created VPInstruction. Only normalized loops are expected so we use step 1.
  assert(HLp->getStrideCanonExpr()->isOne() &&
         "Expected positive unit-stride HLLoop.");
  Builder.setInsertPoint(LpLatch);
  auto *IVNext = cast<VPInstruction>(Builder.createAdd(
      IndSemiPhi,
      Plan->getVPConstant(ConstantInt::getSigned(HLp->getIVType(), 1)), HLp));

  // Add IVNext to induction semi-phi.
  IndSemiPhi->addOperand(IVNext);

  // Create VPValue for bottom test condition. If decomposition is needed:
  //   1) decompose UB operand. Decomposed VPInstructions are inserted into the
  //      loop PH since they are loop invariant,
  //   2) create master VPInstruction for the bottom test condition with IVNext
  //      and decomposed UB as operands, and,
  //   3) set the last created VPInstruction for UB as master VPInstruction for
  //      that UB group of decomposed VPInstructions.
  assert(HLp->getUpperDDRef() && "Expected a valid upper DDRef for HLLoop.");
  SmallVector<VPValue *, 2> VPOperands;
  VPValue *DecompUB;
  VPOperands.push_back(IVNext);
  { // #1. This scope is for Guard (RAII).
    VPBuilder::InsertPointGuard Guard(Builder);
    Builder.setInsertPoint(LpPH);
    DecompUB = decomposeVPOperand(HLp->getUpperDDRef());
    VPOperands.push_back(DecompUB);
  }

  // #2.
  auto *BottomTest = Builder.createCmpInst(VPOperands[0], VPOperands[1],
                                           getPredicateFromHIR(HLp), HLp);

  if (auto *DecompUBVPI = dyn_cast<VPInstruction>(DecompUB)) {
    // #3. Turn last decomposed VPInstruction of UB as master VPInstruction of
    // the decomposed group.
    DecompUBVPI->HIR.setUnderlyingDDN(HLp);

    // Set DecompUBVPI as master VPInstruction of any other decomposed
    // VPInstruction of UB.
    setMasterForDecomposedVPIs(DecompUBVPI, nullptr /*First VPI before decomp*/,
                               LpPH);
    DecompUBVPI->HIR.setValid();
  }

  // Set the underlying HIR of the new VPInstructions (and its potential
  // decomposed VPInstructions) to valid.
  IndSemiPhi->HIR.setValid();
  IVNext->HIR.setValid();
  BottomTest->HIR.setValid();
  return BottomTest;
}

// Add operands to VPInstructions representing phi nodes from the input IR.
// PhisToFix contains a pair with VPPhi and its associated sink DDRef. We get
// the source of each incoming edge of DDRef and set the VPValue associated to
// that source as operand of the VPPhi.
void VPDecomposerHIR::fixPhiNodes() {
  for (auto &VPPhiDDRPair : PhisToFix) {
    VPInstruction *VPPhi = VPPhiDDRPair.first;
    DDRef *UseDDR = VPPhiDDRPair.second;
    assert(VPPhi->getNumOperands() == 0 &&
           "Expected VPInstruction with no operands.");

    SmallVector<VPValue *, 4> VPOperands;
    createOrGetVPDefsForUse(UseDDR, VPOperands);
    assert(VPOperands.size() > 1 && "Expected multiple definitions for VPPhi!");
    for (auto *VPOp : VPOperands)
      VPPhi->addOperand(VPOp);

    // Set the master VPInstruction of this VPPhi as valid after the fix.
    VPPhi->HIR.getMaster()->HIR.setValid();
  }
}

VPInstruction *
VPDecomposerHIR::createVPInstructionsForDDNode(HLDDNode *DDNode,
                                               VPBasicBlock *InsPointVPBB) {
  LLVM_DEBUG(dbgs() << "Generating VPInstructions for "; DDNode->dump();
             dbgs() << "\n");

  // There should't be any VPValue for DDNode at this point. Otherwise, we
  // visited DDNode when we shouldn't, breaking the RPO traversal order.
  assert(!HLDef2VPValue.count(DDNode) && "DDNode shouldn't have been visited.");

  // Set the insertion point in the builder for the VPInstructions that we are
  // going to create for this DDNode.
  Builder.setInsertPoint(InsPointVPBB);
  // Keep last instruction before decomposition. We will need it to set the
  // master VPInstruction of all the decomposed VPInstructions created.
  VPInstruction *LastVPIBeforeDec = getLastVPI(InsPointVPBB);

  // Create and decompose the operands of the future new VPInstruction.
  // They will be inserted (obviously) before the new VPInstruction.
  SmallVector<VPValue *, 4> VPOperands;
  createVPOperandsForMasterVPInst(DDNode, VPOperands);

  // Create new VPInstruction with previous operands.
  VPInstruction *NewVPInst = createVPInstruction(DDNode, VPOperands);

  // Set NewVPInst as master VPInstruction of any decomposed VPInstruction
  // resulting from decomposing its operands.
  setMasterForDecomposedVPIs(NewVPInst, LastVPIBeforeDec, InsPointVPBB);

  // Set the underlying HIR of the new VPInstruction (and its potential
  // decomposed VPInstructions) to valid.
  NewVPInst->HIR.setValid();
  return NewVPInst;
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

  // If the RegDDRef is a self blob or metadata, we use the RegDDRef directly in
  // the following steps since there is no BlobDDRef associated to this Blob.
  // Otherwise, we retrieve and use the BlobDDRef.
  DDRef *DDR;
  if (RDDR.isSelfBlob() || RDDR.isMetadata())
    DDR = &RDDR;
  else {
    unsigned BlobIndex = RDDR.getBlobUtils().findBlob(Blob);
    assert(BlobIndex != InvalidBlobIndex && "SCEV is not a Blob");
    DDR = RDDR.getBlobDDRef(BlobIndex);
    assert(DDR != nullptr && "BlobDDRef not found!");
  }

  unsigned BlobNumReachDefs = Decomposer.getNumReachingDefinitions(DDR);
  assert(BlobNumReachDefs > 0 && "Blob without reaching definitions!");

  // Blob has reaching definitions. We need to retrieve (or create) the VPValues
  // associated to the sources DDRefs (definitions). If there are multiple
  // definitions, in addition, we introduce a semi-phi operation that "blends"
  // all the VPValue definitions.
  if (BlobNumReachDefs == 1) {
    // Single definition.
    SmallVector<VPValue *, 1> VPDefs;
    Decomposer.createOrGetVPDefsForUse(DDR, VPDefs);
    assert(VPDefs.size() == 1 && "Expected single definition.");
    return VPDefs.front();
  } else
    // The operands of the semi-phi are not set right now since some of them
    // might not have been created yet. They will be set by fixPhiNodes.
    return cast<VPInstruction>(
        Decomposer.Builder.createSemiPhiOp({} /*No operands*/));
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
