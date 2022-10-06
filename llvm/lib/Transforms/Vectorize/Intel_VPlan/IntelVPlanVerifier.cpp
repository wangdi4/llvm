//===-- IntelVPlanVerifier.cpp --------------------------------------------===//
//
//   Copyright (C) 2015-2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines VPlanVerifier class that is used to verify that several
// aspects of a VPlan are correct.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanUtils.h"
#include "IntelVPlanVerifier.h"
#include "IntelVPlanDominatorTree.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "vplan-verifier"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool>
    DisableVerification("vplan-disable-verification", cl::init(false),
                        cl::desc("Disable VPlan H-CFG verification"));

// Count the number of VPLoop's in \p Lp, including itself.
template <class LoopT> static unsigned countLoopsInLoop(const LoopT *Lp) {

  const std::vector<LoopT *> &SubLoops = Lp->getSubLoops();
  unsigned NumLoops = 1;

  for (const LoopT *SL : SubLoops)
    NumLoops += countLoopsInLoop(SL);

  return NumLoops;
}

// Verify that Plan contains the same number of loops (VPLoopRegion) as
// VPLoopInfo and LoopInfo.
void VPlanVerifier::verifyNumLoops(void) const {
  // Compare number of loops in VPLoopInfo with loops in LoopInfo.
  assert(VPLInfo->size() && "More than one top loop is not expected");
  unsigned NumLoopsInVPLoopInfo = countLoopsInLoop<VPLoop>(*VPLInfo->begin());
  unsigned NumLoopsInIR = countLoopsInUnderlyingIR();
  assert(NumLoopsInVPLoopInfo == NumLoopsInIR &&
         "Number of loops in VPLoopInfo and underlying IR don't match");
  (void)NumLoopsInVPLoopInfo;
  (void)NumLoopsInIR;
}

#if INTEL_CUSTOMIZATION
void VPlanVerifier::verifyCFGExternals(const VPlan *Plan) {
  Plan->getExternals().verifyVPConstants();
  Plan->getExternals().verifyVPExternalDefs();
  Plan->getExternals().verifyVPExternalDefsHIR();
  Plan->getExternals().verifyVPMetadataAsValues();
}
#endif

// Public interface to verify the loop and its loop info.
void VPlanVerifier::verifyLoops(
#if INTEL_CUSTOMIZATION
    const VPlanVector *Plan,
#endif
    const VPDominatorTree &VPDomTree, VPLoopInfo *VPLI) {

  VPLInfo = VPLI;

  if (DisableVerification)
    return;

  LLVM_DEBUG(dbgs() << "Verifying loop nest.\n");

#if INTEL_CUSTOMIZATION
  verifyCFGExternals(Plan);
#endif

  unsigned BBNum = 0;
  (void)BBNum;
  for (const VPBasicBlock *VPBB : depth_first(&Plan->getEntryBlock())) {
    verifyBlock(VPBB);
    ++BBNum;
  }

  assert(Plan->size() == BBNum && "Plan has wrong size!");

  if (!VPLInfo)
    return;

  // TODO: Verify domination and postdomination trees.
  VPLInfo->verify(VPDomTree);
  VPLoop *TopLoop = *VPLInfo->begin();
  for (auto *CurVPLoop : post_order(TopLoop)) {
    CurVPLoop->verifyLoop();
  }

  if (TheLoop)
    verifyNumLoops();
}

unsigned VPlanVerifier::countLoopsInUnderlyingIR() const {
  assert(TheLoop && "TheLoop can't be null.");
  return countLoopsInLoop<Loop>(TheLoop);
}

void VPlanVerifier::verifyICmpInst(const VPInstruction *IC) const {
  assert(IC->getOpcode() == Instruction::ICmp);
  // Check that the operands are the same type
  Type *Op1Ty = IC->getOperand(0)->getType();
  Type *Op2Ty = IC->getOperand(1)->getType();
  assert(Op1Ty && Op2Ty &&
         "The operands of ICmp operation should have a valid type.");
  assert(Op1Ty == Op2Ty &&
         "Both operands to ICmp instruction are not of the same type!");
  // Check that the operands are the right type
  assert((Op1Ty->isIntOrIntVectorTy() || Op1Ty->isPtrOrPtrVectorTy()) &&
         "Invalid operand types for ICmp instruction");
  // Check that the predicate is valid.
  assert(CmpInst::isIntPredicate(cast<VPCmpInst>(IC)->getPredicate()) &&
         "Invalid predicate in ICmp instruction!");
  (void)Op1Ty;
  (void)Op2Ty;
}

void VPlanVerifier::verifyFCmpInst(const VPInstruction *FC) const {
  assert(FC->getOpcode() == Instruction::FCmp);
  // Check that the operands are the same type
  Type *Op1Ty = FC->getOperand(0)->getType();
  Type *Op2Ty = FC->getOperand(1)->getType();
  assert(Op1Ty && Op2Ty &&
         "The operands of FCmp operation should have a valid type.");
  assert(Op1Ty == Op2Ty &&
         "Both operands to FCmp instruction are not of the same type!");
  // Check that the operands are the right type
  assert(Op1Ty->isFPOrFPVectorTy() &&
         "Invalid operand types for FCmp instruction");
  // Check that the predicate is valid.
  assert(CmpInst::isFPPredicate(cast<VPCmpInst>(FC)->getPredicate()) &&
         "Invalid predicate in FCmp instruction!");
  (void)Op1Ty;
  (void)Op2Ty;
}

void VPlanVerifier::verifyTruncInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::Trunc);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();

  assert(SrcTy->isIntOrIntVectorTy() && "Trunc only operates on integer");
  assert(DstTy->isIntOrIntVectorTy() && "Trunc only produces integer");
  assert(SrcTy->isVectorTy() == DstTy->isVectorTy() &&
         "trunc source and destination must both be a vector or neither");
  assert(SrcTy->getScalarSizeInBits() > DstTy->getScalarSizeInBits() &&
         "DstTy too big for Trunc");

  (void)SrcTy;
  (void)DstTy;
}

void VPlanVerifier::verifyZExtInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::ZExt);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();

  assert(SrcTy->isIntOrIntVectorTy() && "ZExt only operates on integer");
  assert(DstTy->isIntOrIntVectorTy() && "ZExt only produces an integer");
  assert(SrcTy->isVectorTy() == DstTy->isVectorTy() &&
         "zext source and destination must both be a vector or neither");

  assert(SrcTy->getScalarSizeInBits() < DstTy->getScalarSizeInBits() &&
         "Type too small for ZExt");

  (void)SrcTy;
  (void)DstTy;
}

void VPlanVerifier::verifySExtInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::SExt);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();

  assert(SrcTy->isIntOrIntVectorTy() && "SExt only operates on integer");
  assert(DstTy->isIntOrIntVectorTy() && "SExt only produces an integer");
  assert(SrcTy->isVectorTy() == DstTy->isVectorTy() &&
         "sext source and destination must both be a vector or neither");
  assert(SrcTy->getScalarSizeInBits() < DstTy->getScalarSizeInBits() &&
         "Type too small for SExt");

  (void)SrcTy;
  (void)DstTy;
}

void VPlanVerifier::verifyFPTruncInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::FPTrunc);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();
  assert(SrcTy->isFPOrFPVectorTy() && "FPTrunc only operates on FP");
  assert(DstTy->isFPOrFPVectorTy() && "FPTrunc only produces an FP");
  assert(SrcTy->isVectorTy() == DstTy->isVectorTy() &&
         "fptrunc source and destination must both be a vector or neither");
  assert(SrcTy->getScalarSizeInBits() > DstTy->getScalarSizeInBits() &&
         "DstTy too big for FPTrunc");

  (void)SrcTy;
  (void)DstTy;
}

void VPlanVerifier::verifyFPExtInst(const VPInstruction *I) const {
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();

  assert(SrcTy->isFPOrFPVectorTy() && "FPExt only operates on FP");
  assert(DstTy->isFPOrFPVectorTy() && "FPExt only produces an FP");
  assert(SrcTy->isVectorTy() == DstTy->isVectorTy() &&
         "fpext source and destination must both be a vector or neither");
  assert(SrcTy->getScalarSizeInBits() < DstTy->getScalarSizeInBits() &&
         "DstTy too small for FPExt");

  (void)SrcTy;
  (void)DstTy;
}

void VPlanVerifier::verifyFPToUIInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::FPToUI);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();

  assert(SrcTy->isFPOrFPVectorTy() && "FPToUI source must be FP or FP vector");
  assert(DstTy->isIntOrIntVectorTy() &&
         "FPToUI result must be integer or integer vector");

  auto *SrcVecTy = dyn_cast<FixedVectorType>(SrcTy);
  auto *DstVecTy = dyn_cast<FixedVectorType>(DstTy);
  if (SrcVecTy && DstVecTy)
    assert(SrcVecTy->getNumElements() == DstVecTy->getNumElements() &&
           "FPToUI source and dest vector length mismatch");
  else
    assert(!SrcTy->isVectorTy() && !DstTy->isVectorTy() &&
           "FPToUI source and dest must both be vector or scalar");
}

void VPlanVerifier::verifyFPToSIInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::FPToSI);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();
  assert(SrcTy->isFPOrFPVectorTy() && "FPToSI source must be FP or FP vector");
  assert(DstTy->isIntOrIntVectorTy() &&
         "FPToSI result must be integer or integer vector");

  auto *SrcVecTy = dyn_cast<FixedVectorType>(SrcTy);
  auto *DstVecTy = dyn_cast<FixedVectorType>(DstTy);
  if (SrcVecTy && DstVecTy)
    assert(SrcVecTy->getNumElements() == DstVecTy->getNumElements() &&
           "FPToSI source and dest vector length mismatch");
  else
    assert(!SrcTy->isVectorTy() && !DstTy->isVectorTy() &&
           "FPToSI source and dest must both be vector or scalar");
}

void VPlanVerifier::verifyUIToFPInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::UIToFP);
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();
  assert(SrcTy->isIntOrIntVectorTy() &&
         "UIToFP source must be integer or integer vector");
  assert(DstTy->isFPOrFPVectorTy() && "UIToFP result must be FP or FP vector");

  auto *SrcVecTy = dyn_cast<FixedVectorType>(SrcTy);
  auto *DstVecTy = dyn_cast<FixedVectorType>(DstTy);
  if (SrcVecTy && DstVecTy)
    assert(SrcVecTy->getNumElements() == DstVecTy->getNumElements() &&
           "UIToFP source and dest vector length mismatch");
  else
    assert(!SrcTy->isVectorTy() && !DstTy->isVectorTy() &&
           "UIToFP source and dest must both be vector or scalar");
}

void VPlanVerifier::verifySIToFPInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::SIToFP);
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();
  assert(SrcTy->isIntOrIntVectorTy() &&
         "SIToFP source must be integer or integer vector");
  assert(DstTy->isFPOrFPVectorTy() && "SIToFP result must be FP or FP vector");

  auto *SrcVecTy = dyn_cast<FixedVectorType>(SrcTy);
  auto *DstVecTy = dyn_cast<FixedVectorType>(DstTy);
  if (SrcVecTy && DstVecTy)
    assert(SrcVecTy->getNumElements() == DstVecTy->getNumElements() &&
           "SIToFP source and dest vector length mismatch");
  else
    assert(!SrcTy->isVectorTy() && !DstTy->isVectorTy() &&
           "SIToFP source and dest must both be vector or scalar");
}

void VPlanVerifier::verifyIntToPtrInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::IntToPtr);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();
  assert(SrcTy->isIntOrIntVectorTy() && "IntToPtr source must be an integral");
  assert(DstTy->isPtrOrPtrVectorTy() && "IntToPtr result must be a pointer");

  if (auto *Ptr = dyn_cast<PointerType>(DstTy->getScalarType()))
    assert(!DL.isNonIntegralPointerType(Ptr) &&
           "inttoptr not supported for non-integral pointers");

  auto *SrcVecTy = dyn_cast<FixedVectorType>(SrcTy);
  auto *DstVecTy = dyn_cast<FixedVectorType>(DstTy);
  if (SrcVecTy && DstVecTy)
    assert(SrcVecTy->getNumElements() == DstVecTy->getNumElements() &&
           "IntToPtr Vector width mismatch");
  else
    assert(!SrcTy->isVectorTy() && !DstTy->isVectorTy() &&
           "IntToPtr type mismatch");
}

void VPlanVerifier::verifyPtrToIntInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::PtrToInt);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();
  assert(SrcTy->isPtrOrPtrVectorTy() && "PtrToInt source must be pointer");
  assert(DstTy->isIntOrIntVectorTy() && "PtrToInt result must be integral");
  assert(SrcTy->isVectorTy() == DstTy->isVectorTy() &&
         "PtrToInt type mismatch");

  if (auto *Ptr = dyn_cast<PointerType>(DstTy->getScalarType()))
    assert(!DL.isNonIntegralPointerType(Ptr) &&
           "PtrToInt not supported for non-integral pointers");

  auto *SrcVecTy = dyn_cast<FixedVectorType>(SrcTy);
  auto *DstVecTy = dyn_cast<FixedVectorType>(DstTy);
  if (SrcVecTy && DstVecTy)
    assert(SrcVecTy->getNumElements() == DstVecTy->getNumElements() &&
           "PtrToInt Vector width mismatch");
  else
    assert(!SrcTy->isVectorTy() && !DstTy->isVectorTy() &&
           "PtrToInt type mismatch");
}

void VPlanVerifier::verifyBitCastInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::BitCast);
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();

  assert((SrcTy && DstTy) && "Invalid Src or Dst type");
  auto *SrcPtrTy = dyn_cast<PointerType>(SrcTy->getScalarType());
  auto *DstPtrTy = dyn_cast<PointerType>(DstTy->getScalarType());

  assert(!SrcPtrTy == !DstPtrTy &&
         "BitCast implies a no-op cast of type only. No bits change");

  if (SrcPtrTy)
    assert(SrcPtrTy->getAddressSpace() == DstPtrTy->getAddressSpace() &&
           "Bitcast: pointer address spaces must match");
  else
    assert(SrcTy->getPrimitiveSizeInBits() == DstTy->getPrimitiveSizeInBits() &&
           "Source and Dstination bit widths should be identical.");
  (void)DstPtrTy;

  // A vector of pointers must have the same number of elements.
  auto *SrcVecTy = dyn_cast<FixedVectorType>(SrcTy);
  auto *DstVecTy = dyn_cast<FixedVectorType>(DstTy);

  if (SrcVecTy && DstVecTy)
    assert(
        SrcVecTy->getNumElements() == DstVecTy->getNumElements() &&
        "BitCast: A vector of pointers must have the same number of elements.");
}

void VPlanVerifier::verifyBinaryOperator(const VPInstruction *BI) const {
  assert(BI->getOperand(0)->getType() == BI->getOperand(1)->getType() &&
         "Both operands to a binary operator are not of the same type!");
  switch (BI->getOpcode()) {
  case Instruction::Add:
  case Instruction::Sub:
  case Instruction::Mul:
  case Instruction::SDiv:
  case Instruction::UDiv:
  case Instruction::SRem:
  case Instruction::URem:
    assert(BI->getType()->isIntOrIntVectorTy() &&
           "Integer arithmetic operators only work with integral types!");
    assert(BI->getType() == BI->getOperand(0)->getType() &&
           "Integer arithmetic operators must have same type "
           "for operands and result!");
    break;
  // Check that floating-point arithmetic operators are only used with
  // floating-point operands.
  case Instruction::FAdd:
  case Instruction::FSub:
  case Instruction::FMul:
  case Instruction::FDiv:
  case Instruction::FRem:
    assert(BI->getType()->isFPOrFPVectorTy() &&
           "Floating-point arithmetic operators only work with "
           "floating-point types!");
    assert(BI->getType() == BI->getOperand(0)->getType() &&
           "Floating-point arithmetic operators must have same type "
           "for operands and result!");
    break;
  // Check that logical operators are only used with integral operands.
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor:
    assert(BI->getType()->isIntOrIntVectorTy() &&
           "Logical operators only work with integral types!");
    assert(BI->getType() == BI->getOperand(0)->getType() &&
           "Logical operators must have same type for operands and result!");
    break;
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
    assert(BI->getType()->isIntOrIntVectorTy() &&
           "Shifts only work with integral types!");
    assert(BI->getType() == BI->getOperand(0)->getType() &&
           "Shift return type must be same as operands!");
    break;
  default:
    llvm_unreachable("Unknown BinaryOperator opcode!");
  }
}

// Verify information of \p Inst and the sanity of its operands.
void VPlanVerifier::verifySpecificInstruction(
    const VPInstruction *VPInst) const {
  if (Instruction::isBinaryOp(VPInst->getOpcode())) {
    verifyBinaryOperator(VPInst);
  }
  // TODO: XMain source currently does not have isUnaryOps(), even though it is
  // present in community LLVM. We might want to do some extra verification for
  // instruction like Instruction::FNeg
  unsigned Opcode = VPInst->getOpcode();
  switch (Opcode) {
  case Instruction::Load:
  case VPInstruction::ExpandLoad:
  case VPInstruction::ExpandLoadNonu:
  case Instruction::Store:
  case VPInstruction::CompressStore:
  case VPInstruction::CompressStoreNonu:
    assert(1 && "The type of the operands of Load/Store are not correct");
    break;
  case Instruction::GetElementPtr:
    verifyGEPInstruction(cast<VPGEPInstruction>(VPInst));
    break;
  case VPInstruction::Subscript:
    verifySubscriptInst(cast<VPSubscriptInst>(VPInst));
    break;
  case Instruction::ICmp:
    verifyICmpInst(VPInst);
    break;
  case Instruction::FCmp:
    verifyFCmpInst(VPInst);
    break;
  case Instruction::SExt:
    verifySExtInst(VPInst);
    break;
  case Instruction::ZExt:
    verifyZExtInst(VPInst);
    break;
  case Instruction::FPExt:
    verifyFPExtInst(VPInst);
    break;
  case Instruction::Trunc:
    verifyTruncInst(VPInst);
    break;
  case Instruction::FPTrunc:
    verifyFPTruncInst(VPInst);
    break;
  case Instruction::BitCast:
    verifyBitCastInst(VPInst);
    break;
  case Instruction::IntToPtr:
    verifyIntToPtrInst(VPInst);
    break;
  case Instruction::PtrToInt:
    verifyPtrToIntInst(VPInst);
    break;
  case Instruction::UIToFP:
    verifyUIToFPInst(VPInst);
    break;
  case Instruction::SIToFP:
    verifySIToFPInst(VPInst);
    break;
  case Instruction::FPToSI:
    verifyFPToSIInst(VPInst);
    break;
  case Instruction::FPToUI:
    verifyFPToUIInst(VPInst);
    break;
  case Instruction::PHI:
    verifyPHINode(cast<VPPHINode>(VPInst));
    break;
  case VPInstruction::Abs:
    verifyAbsInst(VPInst);
    break;
  default:
    // TODO: There are more LLVM instructions than the ones that we handle here.
    // E.g., 'br' or 'call'. Once we have VPlan support for all, we can
    // introduce a hard assertion here. Till that point, we just do with a
    // 'break'.
    break;
  }
}

void VPlanVerifier::verifyOperands(const VPUser *U) {
  for (const VPValue *Op : U->operands()) {
    (void)Op;
    assert(Op && "Null operand found!");
    // We expect that for each Op->U edge there is a matching U->Op edge.
    assert(Op->getNumUsersTo(U) == U->getNumOperandsFrom(Op) &&
           "Op->U and U->Op do not match!");
  }
}

void VPlanVerifier::verifyUsers(const VPValue *Def) {
  for (const VPUser *U : Def->users()) {
    (void)U;
    // We expect that for each Def->U edge there is a matching U->Def edge.
    assert(Def->getNumUsersTo(U) == U->getNumOperandsFrom(Def) &&
           "Def->U and U->Def do not match!");
    // TODO: Add more exhaustive checks for call instruction, with Function as
    // one of the arguments
  }
}

// Verify that number of incoming values matches to number of predecessors
// of the block where PHI node is located. Also verify that each incoming block
// is found in the predecessor list of \p Phi node's parent VPBB.
void VPlanVerifier::verifyPHINode(const VPPHINode *Phi) const {
  assert(Phi->getOpcode() == Instruction::PHI);
  assert(Phi->getNumIncomingValues() ==
             Phi->getParent()->getNumPredecessors() &&
         "Number of incoming values doesn't match with number of preds");

  const auto &PBlocks = Phi->blocks();
  for (auto *Block : Phi->getParent()->getPredecessors()) {
    assert(llvm::find(PBlocks, Block) != PBlocks.end() &&
           "A predecessor is not incoming VPBB for VPPHINode");
    (void)Block;
  }
  (void)PBlocks;
}

// Verify operand types of the \p GEP instruction.
void VPlanVerifier::verifyGEPInstruction(const VPGEPInstruction *GEP) const {
  // Check base pointer VPValue type. The first operand of the GEP will be the
  // base pointer.
  Type *TargetTy = GEP->getOperand(0)->getType();
  assert(isa<PointerType>(TargetTy) &&
         "GEP base pointer is not a vector or a vector of pointers.");
  (void)TargetTy;

  // Check that each index of GEP is integer or vector of integer type
  for (auto OpIt = GEP->op_begin() + 1; OpIt != GEP->op_end(); ++OpIt) {
    assert((*OpIt)->getType()->isIntOrIntVectorTy() &&
           "GEP indexes must be integers.");
    (void)OpIt;
  }
}

// Verify operands and type consistency of the given VPSubscriptInst
// instruction. Verification is not done for result type since VPSubscriptInst
// can represent combined multi-dimensional access (unlike
// llvm.intel.subscript), in which case resulting type would not match base
// pointer type.
void VPlanVerifier::verifySubscriptInst(
    const VPSubscriptInst *Subscript) const {
  VPValue *Ptr = Subscript->getPointerOperand();
  unsigned NumDims = Subscript->getNumDimensions();

  Type *PtrTy = Ptr->getType();
  assert(PtrTy->isPtrOrPtrVectorTy() &&
         "SubscriptInst base ptr is not pointer type.");
  (void)PtrTy;

  assert(Subscript->getNumOperands() == 3 * NumDims + 1 &&
         "SubscriptInst has invalid number of operands.");

  for (int Dim = NumDims - 1; Dim >= 0; --Dim) {
    auto DimInfo = Subscript->dim(Dim);
    unsigned Rank = DimInfo.Rank;
    VPValue *Lower = DimInfo.LowerBound;
    VPValue *Stride = DimInfo.StrideInBytes;
    VPValue *Index = DimInfo.Index;

    assert(
        Rank <= 32 &&
        "Rank cannot be greater than 32, max possible number of dimensions.");

    VPValue *IntArgs[] = {Lower, Stride, Index};
    assert(
        all_of(IntArgs,
               [](VPValue *V) { return V->getType()->isIntOrIntVectorTy(); }) &&
        "SubscriptInst lower/stride/index must be integers.");
    (void)IntArgs;
    (void)Rank;
  }
}

void VPlanVerifier::verifyAbsInst(const VPInstruction *I) const {
  assert(I->getOpcode() == VPInstruction::Abs);

  // Abs instruction has one operand.
  assert(I->getNumOperands() == 1 && "Abs instruction should have 1 operand");

  // Operand and instruction types should match.
  Type *OpTy = I->getOperand(0)->getType();
  Type *InstTy = I->getType();
  assert(OpTy == InstTy && "Unexpected operand/inst type mismatch");

  assert(OpTy->isIntOrIntVectorTy() && "Abs only operates on integers");

  (void)OpTy;
  (void)InstTy;
}

// Verify information of \p Inst nested in \p Block.
void VPlanVerifier::verifyInstruction(const VPInstruction *Inst,
                                      const VPBasicBlock *Block) const {
  // Generic checks of instructions
  assert(Inst->getType() != nullptr &&
         "VPInstruction cannot have a nullptr base-type");
  assert(Inst->getParent() == Block &&
         "Incorrect VPBB parent for a VPInstruction");
  assert((Inst->getOpcode() != Instruction::PHI || isa<VPPHINode>(Inst)) &&
         "Phi VPInstructions should be represented with VPHINode!");

  // Check that the return value of the instruction is either void or a legal
  // value type.
  assert((Inst->getType()->isVoidTy() || Inst->getType()->isFirstClassType()) &&
         "Instruction returns a non-scalar type!");
  verifyOperands(Inst);
  verifyUsers(Inst);
  verifySpecificInstruction(Inst);
}

// Verify if the block is correctly connected with other basic blocks in the
// loop.
void VPlanVerifier::verifyBlock(const VPBasicBlock *VPBB) const {

  for (const auto &Inst : *VPBB)
    verifyInstruction(&Inst, VPBB);

  // Check block's ConditionBit
  if (VPBB->getNumSuccessors() > 1)
    assert(VPBB->getCondBit() && VPBB->getNumSuccessors() == 2 &&
           "Missing condition bit.");
  else
    assert(!VPBB->getCondBit() && VPBB->getNumSuccessors() < 2 &&
           "Unexpected condition bit.");

  // Check if there is a bidirectional link between block and its successors.
  assert(all_of(VPBB->getSuccessors(),
                [VPBB](VPBasicBlock *Succ) {
                  return any_of(
                      Succ->getPredecessors(),
                      [VPBB](VPBasicBlock *Pred) { return Pred == VPBB; });
                }) &&
         "There is not a bidirectional link between the current block and "
         "its successors.");

  // There must be only one instance of the successors in block's
  // successor list.
  assert(all_of(VPBB->getSuccessors(),
                 [VPBB](VPBasicBlock *Succ) {
                   return std::count(VPBB->getSuccessors().begin(),
                                     VPBB->getSuccessors().end(), Succ) == 1;
                 }) &&
         "Multiple instances of the same successor.");

  // Check if there is a bidirectional link between block and its
  // predecessors.
  assert(all_of(VPBB->getPredecessors(),
                [VPBB](VPBasicBlock *Pred) {
                  return any_of(
                      Pred->getSuccessors(),
                      [VPBB](VPBasicBlock *Succ) { return Succ == VPBB; });
                }) &&
         "There is not a bidirectional link between the current block and "
         "its predecessors.");

  // There must be only one instance of the predecessors in block's
  // predecessor list.
  assert(all_of(VPBB->getPredecessors(),
                [VPBB](VPBasicBlock *Pred) {
                  return std::count(VPBB->getPredecessors().begin(),
                                    VPBB->getPredecessors().end(), Pred) == 1;
                }) &&
         "Multiple instances of the same predecessor.");
}
