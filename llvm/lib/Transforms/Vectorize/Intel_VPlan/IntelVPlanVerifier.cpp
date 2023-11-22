//===-- IntelVPlanVerifier.cpp --------------------------------------------===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
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

#include "IntelVPlanVerifier.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanUtils.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "vplan-verifier"

// This is a helper macro to provide more info than just an assert when
// the verifier detects a failure. This is a macro rather than a function
// to allow us to print the expression that caused the assertion to fail,
// since a function argument would print only the name.
// Note: The do {} while(0) is to prevent issues when ifs are used without
// enclosing curly braces.
#ifndef NDEBUG
#define ASSERT_VPVALUE(Check, V, Msg)                                          \
  do {                                                                         \
    if (!(Check)) {                                                            \
      dbgs() << "VPlan verifier check failed for value:\n";                    \
      if (V)                                                                   \
        (V)->dump();                                                           \
      assert((Check) && (Msg));                                                \
    }                                                                          \
  } while (0)
#define ASSERT_VPBB(Check, BB, Msg)                                            \
  do {                                                                         \
    if (!(Check)) {                                                            \
      dbgs() << "VPlan verifier check failed for VPBasicBlock:\n";             \
      if (BB)                                                                  \
        dbgs() << (BB)->getName();                                             \
      assert((Check) && (Msg));                                                \
    }                                                                          \
  } while (0)
#else
#define ASSERT_VPVALUE(Check, V, Msg) ((void)nullptr)
#define ASSERT_VPBB(Check, BB, Msg) ((void)nullptr)
#endif // ifndef NDEBUG
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

// Check the block's instructions to make sure that all phis and blends are at
// the beginning of the block, and not mixed in with regular instructions
void VPlanVerifier::verifyPhiBlendPlacement(const VPBasicBlock *Block) const {
  auto It = Block->begin();
  while (It != Block->end() && (isa<VPPHINode>(It) || isa<VPBlendInst>(It)))
    It++;

  while (It != Block->end()) {
    ASSERT_VPVALUE(!isa<VPPHINode>(It) && !isa<VPBlendInst>(It), &(*It),
                   "Phi/Blend instruction not at the beginning of the block");
    It++;
  }
}

// Verify that an instruction dominates all of its uses
void VPlanVerifier::verifySSA(const VPInstruction *VPI,
                              const VPDominatorTree *DT) const {
  if (!DT)
    return;

  for (VPUser *User : VPI->users()) {
    if (const VPInstruction *VPU = dyn_cast<VPInstruction>(User)) {
      if (!isa<VPPHINode>(VPU) && !isa<VPPHINode>(VPI) && VPI != VPU) {
        ASSERT_VPVALUE(DT->dominates(VPI, VPU), VPI,
                       "Instruction does not dominate all uses");
      }
    }
  }
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

void VPlanVerifier::verifyCFGExternals(const VPlan *Plan) {
  Plan->getExternals().verifyVPConstants();
  Plan->getExternals().verifyVPExternalDefs();
  Plan->getExternals().verifyVPExternalDefsHIR();
  Plan->getExternals().verifyVPMetadataAsValues();
  Plan->getExternals().verifyVPExternalUses();
}

// Static interface to verify a VPlan
// Calls into verifyVPlan after constructing a verifier object
void VPlanVerifier::verify(const VPlanVector *Plan, const Loop *Lp,
                           unsigned int CheckFlags) {
  assert(Plan && "Plan must be defined");
  assert(Plan->getDataLayout() && "Plan data layout must be defined");
  VPlanVerifier Verifier(Lp, *Plan->getDataLayout());
  Verifier.verifyVPlan(Plan, CheckFlags);
}

void VPlanVerifier::verifyDA(const VPlan *Plan) const {
  if (auto *DABase = Plan->getVPlanDA()) {
    if (auto *DA = dyn_cast<VPlanDivergenceAnalysis>(DABase)) {
      DA->verifyVectorShapes();
    }
  }
}

void VPlanVerifier::verifyDAShape(const VPInstruction *Inst) const {
  auto *Block = Inst->getParent();
  if (Block->getParent()) {
    if (auto *DAB = Block->getParent()->getVPlanDA()) {
      if (auto *DA = dyn_cast<VPlanDivergenceAnalysis>(DAB)) {
        if (Inst->getOpcode() <= Instruction::OtherOpsEnd) {
          auto OldShape = DA->getVectorShape(*Inst);
          auto NewShape = DA->computeVectorShape(Inst);
          ASSERT_VPVALUE(!DA->shapesAreDifferent(NewShape, OldShape), Inst,
                         "Recalculated shape for DA is different");
          (void)NewShape;
          (void)OldShape;
        }
      }
    }
  }
}

// Verify the LiveIn/Out lists attached to the VPlan
void VPlanVerifier::verifyLiveInOut(const VPlanVector *Plan) const {
  const VPExternalValues &Externals = Plan->getExternals();
  unsigned Idx = 0;
  auto NumExtUses = std::distance(Plan->getExternals().externalUses().begin(),
                                  Plan->getExternals().externalUses().end());
  for (const VPLiveOutValue *LiveOut : Plan->liveOutValues()) {
    assert(Idx < NumExtUses &&
           "Live out index exceeds number of external uses");
    assert(LiveOut && "Null live out in plan list");
    unsigned MergeId = LiveOut->getMergeId();
    assert(Idx == MergeId && "Live out index and merge ID do not match!");
    assert(Externals.getVPExternalUse(MergeId) &&
           "No matching VPExternalUse for live out");
    Idx++;
    (void)MergeId;
  }

  Idx = 0;
  for (const VPLiveInValue *LiveIn : Plan->liveInValues()) {
    assert(Idx < NumExtUses &&
           "Live out index exceeds number of external uses");
    // Not all external uses are live-in, but the size of LiveIns is maintained
    // to allow indexing based off of MergeId. So, skip any null entries
    if (LiveIn) {
      unsigned MergeId = LiveIn->getMergeId();
      assert(Idx == MergeId && "Live in index and merge ID do not match!");
      assert(Externals.getVPExternalUse(MergeId) &&
             "No matching VPExternalUse for live in");
      (void)MergeId;
    }
    Idx++;
  }
  (void)Idx;
  (void)Externals;
  (void)NumExtUses;
}

void VPlanVerifier::verifyHeaderExitPredicates(const VPLoop *Lp) const {
  auto *Header = Lp->getHeader();
  assert(Header);

  auto *HeaderPredicate = Header->getBlockPredicate();
  SmallVector<VPBasicBlock *, 4> ExitingBlocks;
  Lp->getExitingBlocks(ExitingBlocks);
  for (auto *Exit : ExitingBlocks) {
    auto *ExitPredicate = Exit->getBlockPredicate();
    ASSERT_VPBB((HeaderPredicate && ExitPredicate) ||
                    (!HeaderPredicate && !ExitPredicate),
                Header,
                "Loop header and exit must both be predicated or neither");

    if (HeaderPredicate)
      ASSERT_VPVALUE(
          HeaderPredicate->getOperand(0) == ExitPredicate->getOperand(0),
          HeaderPredicate,
          "Header and exit block of loop do not have the same predicate");
    (void)ExitPredicate;
  }
  (void)HeaderPredicate;
}

// Public interface to verify the loop and its loop info.
void VPlanVerifier::verifyVPlan(const VPlanVector *Plan,
                                unsigned int CheckFlags) {

  VPLInfo = Plan->getVPLoopInfo();
  Flags = CheckFlags;

  if (DisableVerification)
    return;

  LLVM_DEBUG(dbgs() << "Verifying loop nest.\n");

  if (!shouldSkipExternals()) {
    verifyCFGExternals(Plan);
    verifyLiveInOut(Plan);
  }

  unsigned BBNum = 0;
  (void)BBNum;
  for (const VPBasicBlock *VPBB : depth_first(&Plan->getEntryBlock())) {
    verifyBlock(VPBB);
    ++BBNum;
  }

  assert(Plan->size() == BBNum && "Plan has wrong size!");
#ifndef NDEBUG
  // Verify dom/postdom tree
  if (Plan->getDT())
    assert(Plan->getDT()->verify() && "VPlan Dominator Tree failed to verify");
  if (Plan->getPDT())
    assert(Plan->getPDT()->verify() &&
           "VPlan Post-Dominator Tree failed to verify");
#endif

  // Skipped in cases where the loop info isn't updated to reflect
  // transformations that have been performed
  if (!VPLInfo || shouldSkipLoopInfo())
    return;

  VPLoop *TopLoop = *VPLInfo->begin();
  for (auto *CurVPLoop : post_order(TopLoop)) {
    CurVPLoop->verifyLoop();

    verifyHeaderExitPredicates(CurVPLoop);
  }

  // The number of subloops can change in the VPLoop as a result of VPEntity
  // insertions, so we may not always want to check the number of loops
  if (TheLoop && !shouldSkipNumLoops()) {
    verifyNumLoops();
  }

  // Check that every inner loop has only one exit block
  // Not guaranteed until loop exit merging is done
  if (!shouldSkipInnerMultiExit()) {
    for (const VPLoop *SL : TopLoop->getSubLoops()) {
      // getExitingBlock is null if there are multiple exit blocks
      ASSERT_VPVALUE(SL->getExitingBlock(), SL,
                     "Inner loop has multiple exits");
      (void)SL;
    }
  }
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
  ASSERT_VPVALUE(Op1Ty && Op2Ty, IC,
                 "The operands of ICmp operation should have a valid type.");
  ASSERT_VPVALUE(Op1Ty == Op2Ty, IC,
                 "Both operands to ICmp instruction are not of the same type!");
  // Check that the operands are the right type
  ASSERT_VPVALUE((Op1Ty->isIntOrIntVectorTy() || Op1Ty->isPtrOrPtrVectorTy()),
                 IC, "Invalid operand types for ICmp instruction");
  // Check that the predicate is valid.
  ASSERT_VPVALUE(CmpInst::isIntPredicate(cast<VPCmpInst>(IC)->getPredicate()),
                 IC, "Invalid predicate in ICmp instruction!");
  (void)Op1Ty;
  (void)Op2Ty;
}

void VPlanVerifier::verifyFCmpInst(const VPInstruction *FC) const {
  assert(FC->getOpcode() == Instruction::FCmp);
  // Check that the operands are the same type
  Type *Op1Ty = FC->getOperand(0)->getType();
  Type *Op2Ty = FC->getOperand(1)->getType();
  ASSERT_VPVALUE(Op1Ty && Op2Ty, FC,
                 "The operands of FCmp operation should have a valid type.");
  ASSERT_VPVALUE(Op1Ty == Op2Ty, FC,
                 "Both operands to FCmp instruction are not of the same type!");
  // Check that the operands are the right type
  ASSERT_VPVALUE(Op1Ty->isFPOrFPVectorTy(), FC,
                 "Invalid operand types for FCmp instruction");
  // Check that the predicate is valid.
  ASSERT_VPVALUE(CmpInst::isFPPredicate(cast<VPCmpInst>(FC)->getPredicate()),
                 FC, "Invalid predicate in FCmp instruction!");
  (void)Op1Ty;
  (void)Op2Ty;
}

void VPlanVerifier::verifyTruncInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::Trunc);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();

  ASSERT_VPVALUE(SrcTy->isIntOrIntVectorTy(), I,
                 "Trunc only operates on integer");
  ASSERT_VPVALUE(DstTy->isIntOrIntVectorTy(), I,
                 "Trunc only produces an integer");
  ASSERT_VPVALUE(SrcTy->isVectorTy() == DstTy->isVectorTy(), I,
                 "trunc source and destination must both be vector or scalar");
  if (SrcTy->isVectorTy()) {
    ASSERT_VPVALUE(cast<VectorType>(SrcTy)->getElementCount() ==
                       cast<VectorType>(DstTy)->getElementCount(),
                   I, "Trunc source and dest vector length mismatch");
  }
  ASSERT_VPVALUE(SrcTy->getScalarSizeInBits() > DstTy->getScalarSizeInBits(), I,
                 "DstTy too big for Trunc");

  (void)SrcTy;
  (void)DstTy;
}

void VPlanVerifier::verifyZExtInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::ZExt);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();

  ASSERT_VPVALUE(SrcTy->isIntOrIntVectorTy(), I,
                 "ZExt only operates on integer");
  ASSERT_VPVALUE(DstTy->isIntOrIntVectorTy(), I,
                 "ZExt only produces an integer");
  ASSERT_VPVALUE(SrcTy->isVectorTy() == DstTy->isVectorTy(), I,
                 "zext source and destination must both be vector or scalar");
  if (SrcTy->isVectorTy()) {
    ASSERT_VPVALUE(cast<VectorType>(SrcTy)->getElementCount() ==
                       cast<VectorType>(DstTy)->getElementCount(),
                   I, "ZExt source and dest vector length mismatch");
  }

  ASSERT_VPVALUE(SrcTy->getScalarSizeInBits() < DstTy->getScalarSizeInBits(), I,
                 "Type too small for ZExt");

  (void)SrcTy;
  (void)DstTy;
}

void VPlanVerifier::verifySExtInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::SExt);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();

  ASSERT_VPVALUE(SrcTy->isIntOrIntVectorTy(), I,
                 "SExt only operates on integer");
  ASSERT_VPVALUE(DstTy->isIntOrIntVectorTy(), I,
                 "SExt only produces an integer");
  ASSERT_VPVALUE(SrcTy->isVectorTy() == DstTy->isVectorTy(), I,
                 "sext source and destination must both be vector or scalar");
  if (SrcTy->isVectorTy()) {
    ASSERT_VPVALUE(cast<VectorType>(SrcTy)->getElementCount() ==
                       cast<VectorType>(DstTy)->getElementCount(),
                   I, "SExt source and dest vector length mismatch");
  }
  ASSERT_VPVALUE(SrcTy->getScalarSizeInBits() < DstTy->getScalarSizeInBits(), I,
                 "Type too small for SExt");

  (void)SrcTy;
  (void)DstTy;
}

void VPlanVerifier::verifyFPTruncInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::FPTrunc);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();
  ASSERT_VPVALUE(SrcTy->isFPOrFPVectorTy(), I, "FPTrunc only operates on FP");
  ASSERT_VPVALUE(DstTy->isFPOrFPVectorTy(), I, "FPTrunc only produces an FP");
  ASSERT_VPVALUE(
      SrcTy->isVectorTy() == DstTy->isVectorTy(), I,
      "fptrunc source and destination must both be vector or scalar");
  if (SrcTy->isVectorTy()) {
    ASSERT_VPVALUE(cast<VectorType>(SrcTy)->getElementCount() ==
                       cast<VectorType>(DstTy)->getElementCount(),
                   I, "FPTrunc source and dest vector length mismatch");
  }
  ASSERT_VPVALUE(SrcTy->getScalarSizeInBits() > DstTy->getScalarSizeInBits(), I,
                 "DstTy too big for FPTrunc");

  (void)SrcTy;
  (void)DstTy;
}

void VPlanVerifier::verifyFPExtInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::FPExt);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();

  ASSERT_VPVALUE(SrcTy->isFPOrFPVectorTy(), I, "FPExt only operates on FP");
  ASSERT_VPVALUE(DstTy->isFPOrFPVectorTy(), I, "FPExt only produces an FP");
  ASSERT_VPVALUE(SrcTy->isVectorTy() == DstTy->isVectorTy(), I,
                 "fpext source and destination must both be vector or scalar");
  if (SrcTy->isVectorTy()) {
    ASSERT_VPVALUE(cast<VectorType>(SrcTy)->getElementCount() ==
                       cast<VectorType>(DstTy)->getElementCount(),
                   I, "FPExt source and dest vector length mismatch");
  }
  ASSERT_VPVALUE(SrcTy->getScalarSizeInBits() < DstTy->getScalarSizeInBits(), I,
                 "DstTy too small for FPExt");

  (void)SrcTy;
  (void)DstTy;
}

void VPlanVerifier::verifyFPToUIInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::FPToUI);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();

  ASSERT_VPVALUE(SrcTy->isFPOrFPVectorTy(), I,
                 "FPToUI source must be FP or FP vector");
  ASSERT_VPVALUE(DstTy->isIntOrIntVectorTy(), I,
                 "FPToUI result must be integer or integer vector");

  auto *SrcVecTy = dyn_cast<FixedVectorType>(SrcTy);
  auto *DstVecTy = dyn_cast<FixedVectorType>(DstTy);
  if (SrcVecTy && DstVecTy)
    ASSERT_VPVALUE(SrcVecTy->getNumElements() == DstVecTy->getNumElements(), I,
                   "FPToUI source and dest vector length mismatch");
  else
    ASSERT_VPVALUE(!SrcTy->isVectorTy() && !DstTy->isVectorTy(), I,
                   "FPToUI source and dest must both be vector or scalar");
}

void VPlanVerifier::verifyFPToSIInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::FPToSI);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();
  ASSERT_VPVALUE(SrcTy->isFPOrFPVectorTy(), I,
                 "FPToSI source must be FP or FP vector");
  ASSERT_VPVALUE(DstTy->isIntOrIntVectorTy(), I,
                 "FPToSI result must be integer or integer vector");

  auto *SrcVecTy = dyn_cast<FixedVectorType>(SrcTy);
  auto *DstVecTy = dyn_cast<FixedVectorType>(DstTy);
  if (SrcVecTy && DstVecTy)
    ASSERT_VPVALUE(SrcVecTy->getNumElements() == DstVecTy->getNumElements(), I,
                   "FPToSI source and dest vector length mismatch");
  else
    ASSERT_VPVALUE(!SrcTy->isVectorTy() && !DstTy->isVectorTy(), I,
                   "FPToSI source and dest must both be vector or scalar");
}

void VPlanVerifier::verifyUIToFPInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::UIToFP);
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();
  ASSERT_VPVALUE(SrcTy->isIntOrIntVectorTy(), I,
                 "UIToFP source must be integer or integer vector");
  ASSERT_VPVALUE(DstTy->isFPOrFPVectorTy(), I,
                 "UIToFP result must be FP or FP vector");

  auto *SrcVecTy = dyn_cast<FixedVectorType>(SrcTy);
  auto *DstVecTy = dyn_cast<FixedVectorType>(DstTy);
  if (SrcVecTy && DstVecTy)
    ASSERT_VPVALUE(SrcVecTy->getNumElements() == DstVecTy->getNumElements(), I,
                   "UIToFP source and dest vector length mismatch");
  else
    ASSERT_VPVALUE(!SrcTy->isVectorTy() && !DstTy->isVectorTy(), I,
                   "UIToFP source and dest must both be vector or scalar");
}

void VPlanVerifier::verifySIToFPInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::SIToFP);
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();
  ASSERT_VPVALUE(SrcTy->isIntOrIntVectorTy(), I,
                 "SIToFP source must be integer or integer vector");
  ASSERT_VPVALUE(DstTy->isFPOrFPVectorTy(), I,
                 "SIToFP result must be FP or FP vector");

  auto *SrcVecTy = dyn_cast<FixedVectorType>(SrcTy);
  auto *DstVecTy = dyn_cast<FixedVectorType>(DstTy);
  if (SrcVecTy && DstVecTy)
    ASSERT_VPVALUE(SrcVecTy->getNumElements() == DstVecTy->getNumElements(), I,
                   "SIToFP source and dest vector length mismatch");
  else
    ASSERT_VPVALUE(!SrcTy->isVectorTy() && !DstTy->isVectorTy(), I,
                   "SIToFP source and dest must both be vector or scalar");
}

void VPlanVerifier::verifyIntToPtrInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::IntToPtr);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();
  ASSERT_VPVALUE(SrcTy->isIntOrIntVectorTy(), I,
                 "IntToPtr source must be integral");
  ASSERT_VPVALUE(DstTy->isPtrOrPtrVectorTy(), I,
                 "IntToPtr result must be pointer");

  if (auto *Ptr = dyn_cast<PointerType>(DstTy->getScalarType()))
    ASSERT_VPVALUE(!DL.isNonIntegralPointerType(Ptr), I,
                   "inttoptr not supported for non-integral pointers");

  auto *SrcVecTy = dyn_cast<FixedVectorType>(SrcTy);
  auto *DstVecTy = dyn_cast<FixedVectorType>(DstTy);
  if (SrcVecTy && DstVecTy)
    ASSERT_VPVALUE(SrcVecTy->getNumElements() == DstVecTy->getNumElements(), I,
                   "IntToPtr vector length mismatch");
  else
    ASSERT_VPVALUE(!SrcTy->isVectorTy() && !DstTy->isVectorTy(), I,
                   "IntToPtr type mismatch");
}

void VPlanVerifier::verifyPtrToIntInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::PtrToInt);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();
  ASSERT_VPVALUE(SrcTy->isPtrOrPtrVectorTy(), I,
                 "PtrToInt source must be pointer");
  ASSERT_VPVALUE(DstTy->isIntOrIntVectorTy(), I,
                 "PtrToInt result must be integral");
  ASSERT_VPVALUE(SrcTy->isVectorTy() == DstTy->isVectorTy(), I,
                 "PtrToInt type mismatch");

  if (auto *Ptr = dyn_cast<PointerType>(DstTy->getScalarType()))
    ASSERT_VPVALUE(!DL.isNonIntegralPointerType(Ptr), I,
                   "PtrToInt not supported for non-integral pointers");

  auto *SrcVecTy = dyn_cast<FixedVectorType>(SrcTy);
  auto *DstVecTy = dyn_cast<FixedVectorType>(DstTy);
  if (SrcVecTy && DstVecTy)
    ASSERT_VPVALUE(SrcVecTy->getNumElements() == DstVecTy->getNumElements(), I,
                   "PtrToInt vector length mismatch");
  else
    ASSERT_VPVALUE(!SrcTy->isVectorTy() && !DstTy->isVectorTy(), I,
                   "PtrToInt type mismatch");
}

void VPlanVerifier::verifyBitCastInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::BitCast);
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();

  ASSERT_VPVALUE((SrcTy && DstTy), I, "Invalid Src or Dst type");
  auto *SrcPtrTy = dyn_cast<PointerType>(SrcTy->getScalarType());
  auto *DstPtrTy = dyn_cast<PointerType>(DstTy->getScalarType());

  ASSERT_VPVALUE(!SrcPtrTy == !DstPtrTy, I,
                 "BitCast implies a no-op cast of type only. No bits change");

  if (SrcPtrTy)
    ASSERT_VPVALUE(SrcPtrTy->getAddressSpace() == DstPtrTy->getAddressSpace(),
                   I, "Bitcast: pointer address spaces must match");
  else
    ASSERT_VPVALUE(SrcTy->getPrimitiveSizeInBits() ==
                       DstTy->getPrimitiveSizeInBits(),
                   I, "Source and Dstination bit widths should be identical.");
  (void)DstPtrTy;

  // A vector of pointers must have the same number of elements.
  auto *SrcVecTy = dyn_cast<FixedVectorType>(SrcTy);
  auto *DstVecTy = dyn_cast<FixedVectorType>(DstTy);
  if (SrcVecTy && DstVecTy)
    ASSERT_VPVALUE(
        SrcVecTy->getNumElements() * SrcVecTy->getScalarSizeInBits() ==
            DstVecTy->getNumElements() * DstVecTy->getScalarSizeInBits(),
        I, "BitCast: A vector of pointers must have the same total size.");
}

void VPlanVerifier::verifyBinaryOperator(const VPInstruction *BI) const {
  ASSERT_VPVALUE(
      BI->getOperand(0)->getType() == BI->getOperand(1)->getType(), BI,
      "Both operands to a binary operator are not of the same type!");
  switch (BI->getOpcode()) {
  case Instruction::Add:
  case Instruction::Sub:
  case Instruction::Mul:
  case Instruction::SDiv:
  case Instruction::UDiv:
  case Instruction::SRem:
  case Instruction::URem:
    ASSERT_VPVALUE(
        BI->getType()->isIntOrIntVectorTy(), BI,
        "Integer arithmetic operators only work with integral types!");
    ASSERT_VPVALUE(BI->getType() == BI->getOperand(0)->getType(), BI,
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
    ASSERT_VPVALUE(BI->getType()->isFPOrFPVectorTy(), BI,
                   "Floating-point arithmetic operators only work with "
                   "floating-point types!");
    ASSERT_VPVALUE(BI->getType() == BI->getOperand(0)->getType(), BI,
                   "Floating-point arithmetic operators must have same type "
                   "for operands and result!");
    break;
  // Check that logical operators are only used with integral operands.
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor:
    ASSERT_VPVALUE(BI->getType()->isIntOrIntVectorTy(), BI,
                   "Logical operators only work with integral types!");
    ASSERT_VPVALUE(
        BI->getType() == BI->getOperand(0)->getType(), BI,
        "Logical operators must have same type for operands and result!");
    break;
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
    ASSERT_VPVALUE(BI->getType()->isIntOrIntVectorTy(), BI,
                   "Shifts only work with integral types!");
    ASSERT_VPVALUE(BI->getType() == BI->getOperand(0)->getType(), BI,
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
    ASSERT_VPVALUE(1, VPInst,
                   "The type of the operands of Load/Store are not correct");
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
    ASSERT_VPVALUE(Op, Op, "Null operand found!");
    // We expect that for each Op->U edge there is a matching U->Op edge.
    ASSERT_VPVALUE(Op->getNumUsersTo(U) == U->getNumOperandsFrom(Op), Op,
                   "Op->U and U->Op do not match!");
  }
}

void VPlanVerifier::verifyUsers(const VPValue *Def) {
  for (const VPUser *U : Def->users()) {
    (void)U;
    // We expect that for each Def->U edge there is a matching U->Def edge.
    ASSERT_VPVALUE(Def->getNumUsersTo(U) == U->getNumOperandsFrom(Def), U,
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
  ASSERT_VPVALUE(
      Phi->getNumIncomingValues() == Phi->getParent()->getNumPredecessors(),
      Phi, "Number of incoming values doesn't match with number of preds");

  const auto &PBlocks = Phi->blocks();
  for (auto *Block : Phi->getParent()->getPredecessors()) {
    ASSERT_VPVALUE(llvm::find(PBlocks, Block) != PBlocks.end(), Phi,
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
  ASSERT_VPVALUE(isa<PointerType>(TargetTy) || isa<VectorType>(TargetTy), GEP,
                 "GEP base pointer is not a vector or a vector of pointers.");
  (void)TargetTy;

  // Check that each index of GEP is integer or vector of integer type
  for (auto OpIt = GEP->op_begin() + 1; OpIt != GEP->op_end(); ++OpIt) {
    ASSERT_VPVALUE((*OpIt)->getType()->isIntOrIntVectorTy(), GEP,
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
  ASSERT_VPVALUE(PtrTy->isPtrOrPtrVectorTy(), Subscript,
                 "SubscriptInst base ptr is not pointer type.");
  (void)PtrTy;

  ASSERT_VPVALUE(Subscript->getNumOperands() == 3 * NumDims + 1, Subscript,
                 "SubscriptInst has invalid number of operands.");

  for (int Dim = NumDims - 1; Dim >= 0; --Dim) {
    auto DimInfo = Subscript->dim(Dim);
    unsigned Rank = DimInfo.Rank;
    VPValue *Lower = DimInfo.LowerBound;
    VPValue *Stride = DimInfo.StrideInBytes;
    VPValue *Index = DimInfo.Index;

    ASSERT_VPVALUE(
        Rank <= 32, Subscript,
        "Rank cannot be greater than 32, max possible number of dimensions.");

    VPValue *IntArgs[] = {Lower, Stride, Index};
    ASSERT_VPVALUE(
        all_of(IntArgs,
               [](VPValue *V) { return V->getType()->isIntOrIntVectorTy(); }),
        Subscript, "SubscriptInst lower/stride/index must be integers.");
    (void)IntArgs;
    (void)Rank;
  }
}

void VPlanVerifier::verifyAbsInst(const VPInstruction *I) const {
  assert(I->getOpcode() == VPInstruction::Abs);

  // Abs instruction has one operand.
  ASSERT_VPVALUE(I->getNumOperands() == 1, I,
                 "Abs instruction should have 1 operand");

  // Operand and instruction types should match.
  Type *OpTy = I->getOperand(0)->getType();
  Type *InstTy = I->getType();
  ASSERT_VPVALUE(OpTy == InstTy, I, "Unexpected operand/inst type mismatch");

  ASSERT_VPVALUE(OpTy->isIntOrIntVectorTy(), I,
                 "Abs only operates on integers");

  (void)OpTy;
  (void)InstTy;
}

// Verify information of \p Inst nested in \p Block.
void VPlanVerifier::verifyInstruction(const VPInstruction *Inst,
                                      const VPBasicBlock *Block) const {
  // Generic checks of instructions
  ASSERT_VPVALUE(Inst->getType() != nullptr, Inst,
                 "VPInstruction cannot have a nullptr base-type");
  ASSERT_VPVALUE(Inst->getParent() == Block, Inst,
                 "Incorrect VPBB parent for a VPInstruction");
  ASSERT_VPVALUE(
      (Inst->getOpcode() != Instruction::PHI || isa<VPPHINode>(Inst)), Inst,
      "Phi VPInstructions should be represented with VPHINode!");

  // Check that the return value of the instruction is either void or a legal
  // value type.
  ASSERT_VPVALUE(
      (Inst->getType()->isVoidTy() || Inst->getType()->isFirstClassType()),
      Inst, "Instruction returns a non-scalar type!");
  verifyOperands(Inst);
  verifyUsers(Inst);
  verifySpecificInstruction(Inst);
  if (auto *Plan = dyn_cast<VPlanVector>(Block->getParent())) {
    verifySSA(Inst, Plan->getDT());

    if (!shouldSkipDA()) {
      auto *DA = Plan->getVPlanDA();
      if (!DA)
        return;

      ASSERT_VPVALUE(!DA->getVectorShape(*Inst).isUndefined(), Inst,
                     "Shape has not been defined for instruction");

      if (!shouldSkipDAShapes()) {
        verifyDAShape(Inst);
      }
    }
  }
}

// Verify if the block is correctly connected with other basic blocks in the
// loop.
void VPlanVerifier::verifyBlock(const VPBasicBlock *VPBB) const {

  for (const auto &Inst : *VPBB)
    verifyInstruction(&Inst, VPBB);

  // Check that all phis/blends are at the beginning of the block
  verifyPhiBlendPlacement(VPBB);

  // Check block's ConditionBit
  if (VPBB->getNumSuccessors() > 1)
    ASSERT_VPBB(VPBB->getCondBit() && VPBB->getNumSuccessors() == 2, VPBB,
                "Missing condition bit.");
  else
    ASSERT_VPBB(!VPBB->getCondBit() && VPBB->getNumSuccessors() < 2, VPBB,
                "Unexpected condition bit.");

  // Check if there is a bidirectional link between block and its successors.
  ASSERT_VPBB(all_of(VPBB->getSuccessors(),
                     [VPBB](VPBasicBlock *Succ) {
                       return any_of(
                           Succ->getPredecessors(),
                           [VPBB](VPBasicBlock *Pred) { return Pred == VPBB; });
                     }),
              VPBB,
              "There is not a bidirectional link between the current block and "
              "its successors.");

  // There must be only one instance of the successors in block's
  // successor list.
  ASSERT_VPBB(all_of(VPBB->getSuccessors(),
                     [VPBB](VPBasicBlock *Succ) {
                       return std::count(VPBB->getSuccessors().begin(),
                                         VPBB->getSuccessors().end(),
                                         Succ) == 1;
                     }),
              VPBB, "Multiple instances of the same successor.");

  // Check if there is a bidirectional link between block and its
  // predecessors.
  ASSERT_VPBB(all_of(VPBB->getPredecessors(),
                     [VPBB](VPBasicBlock *Pred) {
                       return any_of(
                           Pred->getSuccessors(),
                           [VPBB](VPBasicBlock *Succ) { return Succ == VPBB; });
                     }),
              VPBB,
              "There is not a bidirectional link between the current block and "
              "its predecessors.");

  // There must be only one instance of the predecessors in block's
  // predecessor list.
  ASSERT_VPBB(all_of(VPBB->getPredecessors(),
                     [VPBB](VPBasicBlock *Pred) {
                       return std::count(VPBB->getPredecessors().begin(),
                                         VPBB->getPredecessors().end(),
                                         Pred) == 1;
                     }),
              VPBB, "Multiple instances of the same predecessor.");
}
