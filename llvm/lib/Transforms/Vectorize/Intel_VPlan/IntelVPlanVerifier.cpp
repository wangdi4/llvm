//===-- IntelVPlanVerifier.cpp --------------------------------------------===//
//
//   Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#define DEBUG_TYPE "vplan-verifier"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool>
    DisableHCFGVerification("vplan-disable-verification", cl::init(false),
                            cl::desc("Disable VPlan H-CFG verification"));

// Verify that Block is contained in the right VPLoop.
void VPlanVerifier::verifyContainerLoop(
    const VPBlockBase *Block, const VPLoopRegion *ParentLoopR,
    const SmallPtrSetImpl<const VPBlockBase *> &ExternalBlocks) const {
  const VPLoop *ContainerLoop = nullptr;

  if (ParentLoopR) {
    if (ExternalBlocks.count(Block))
      // If Block is outside of the loop cyle, Block shouldn't be contained in
      // this loop but in ParentRegion's parent loop (if any).
      ContainerLoop = VPLInfo->getLoopFor(Block->getParent());
    else
      // Block should be contained in parent loop.
      ContainerLoop = ParentLoopR->getVPLoop();
  }

  if (ContainerLoop)
    assert(ContainerLoop->contains(Block) &&
           "Block is not contained in the right loop");
  else
    // Check that the loop is not contained in any loop.
    assert(!VPLInfo->getLoopFor(Block) &&
           "Block should not be contained in any VPLoop");
}

// Verify VPLoop information in \p LoopRegion.
void VPlanVerifier::verifyVPLoopInfo(const VPLoopRegion *LoopRegion) const {

  const VPLoop *Loop = LoopRegion->getVPLoop();
  assert(Loop && "Missing VPLoop for VPLoopRegion");

  const VPBlockBase *Preheader = LoopRegion->getEntry();
  assert(Preheader && Preheader == Loop->getLoopPreheader() &&
         "Wrong loop preheader");
  const VPBlockBase *Header = Preheader->getSingleSuccessor();
  assert(Header && "Loop preheader must have a single successor");
  assert(Header == Loop->getHeader() && "Wrong loop header");

  assert((VPLInfo->getLoopFor(Header) == Loop) &&
         "Unexpected loop from loop header");

  (void) Loop;
  (void) Header;
}

/// Collect VPBlockBases that are outside of the loop cycle of the loop in \p
/// ParentLoopR. Return \p ExternalBlock with the external blocks. If \p
/// ExternalBlock is not empty, it is cleared before collecting the new external
/// blocks.
static void CollectLoopExternalBlocks(
    const VPLoopRegion *ParentLoopR, const VPLoopInfo *VPLInfo,
    SmallPtrSetImpl<const VPBlockBase *> &ExternalBlocks) {
  ExternalBlocks.clear();

  // Add loop PH to ExternalBlocks.
  ExternalBlocks.insert(ParentLoopR->getEntry());

  // Add VPBlockBases after loop exitings to ExternalBlocks.
  SmallVector<VPBlockBase *, 4> LoopExits;
  ParentLoopR->getVPLoop()->getUniqueExitBlocks(LoopExits);
  for (const auto *LpExit : LoopExits)
    for (const auto *Block :
         make_range(df_iterator<const VPBlockBase *>::begin(LpExit),
                    df_iterator<const VPBlockBase *>::end(LpExit))) {
      // TODO: Regions outside the loop cycle are not expected. Implement them
      // when necessary.
      assert(!isa<VPRegionBlock>(Block) && "Regions not supported!");
      ExternalBlocks.insert(Block);
    }
}

// Verify information of LoopRegions nested in \p Region.
void VPlanVerifier::verifyLoopRegions(
    const VPRegionBlock *TopRegion) const {

  // Hold blocks outside of the loop cycle of ParentLoopR.
  SmallPtrSet<const VPBlockBase *, 8> ExternalBlocks;

  // VerifyLoopRegions implementation.
  typedef std::function<void(const VPRegionBlock *, const VPLoopRegion *)> RT;
  RT verifyLoopRegionsImp = [&](const VPRegionBlock *Region,
                                const VPLoopRegion *ParentLoopR) {

    assert(Region && "Region cannot be null");

    if (const auto *LoopR = dyn_cast<VPLoopRegion>(Region)) {
      // If Region is a LoopRegion, Region is the new ParentLoopR.
      ParentLoopR = LoopR;
      CollectLoopExternalBlocks(ParentLoopR, VPLInfo, ExternalBlocks);

      // Checks for underlying-IR-specific information.
      verifyIRSpecificLoopRegion(LoopR);
    }

    // Visit Region's CFG
    for (const VPBlockBase *VPB :
         make_range(df_iterator<const VPRegionBlock *>::begin(Region),
                    df_iterator<const VPRegionBlock *>::end(Region))) {
      // If subregion is a loop region, use it as ParentLoop in the visit
      if (const auto *LoopRegion = dyn_cast<VPLoopRegion>(VPB)) {
        verifyVPLoopInfo(LoopRegion);
      }

      verifyContainerLoop(VPB, ParentLoopR, ExternalBlocks);
    }

    // Visit SubRegion
    for (const VPBlockBase *VPB :
         make_range(df_iterator<const VPRegionBlock *>::begin(Region),
                    df_iterator<const VPRegionBlock *>::end(Region))) {

      if (const auto *SR = dyn_cast<VPRegionBlock>(VPB)) {
        verifyLoopRegionsImp(SR, ParentLoopR);
      }
    }
  };

  verifyLoopRegionsImp(TopRegion, nullptr /*ParentLoopRegion*/);
}

// Count the number of VPLoopRegion's nested in \p Region.
static unsigned countLoopRegionsInRegion(const VPRegionBlock *Region) {

  unsigned NumLoops = 0;

  for (const VPBlockBase *VPB :
       make_range(df_iterator<const VPBlockBase *>::begin(Region->getEntry()),
                  df_iterator<const VPBlockBase *>::end(Region->getExit()))) {

    if (isa<VPLoopRegion>(VPB))
      ++NumLoops;

    // Count nested VPLoops
    if (const VPRegionBlock *VPR = dyn_cast<VPRegionBlock>(VPB))
      NumLoops += countLoopRegionsInRegion(VPR);
  }

  return NumLoops;
}

// Count the number of VPLoop's in \p Lp, including itself.
template <class LoopT> static unsigned countLoopsInLoop(const LoopT *Lp) {

  const std::vector<LoopT *> &SubLoops = Lp->getSubLoops();
  unsigned NumLoops = 1;

  for (const LoopT *SL : SubLoops)
    NumLoops += countLoopsInLoop(SL);

  return NumLoops;
}

// Verify that TopRegion contains the same number of loops (VPLoopRegion) as
// VPLoopInfo and LoopInfo.
void VPlanVerifier::verifyNumLoops(const VPRegionBlock *TopRegion) const {

  // Compare number of loops in H-CFG with loops in VPLoopInfo and LoopInfo
  unsigned NumLoopsInCFG = countLoopRegionsInRegion(TopRegion);

  assert(VPLInfo->size() && "More than one top loop is not expected");
  unsigned NumLoopsInVPLoopInfo = countLoopsInLoop<VPLoop>(*VPLInfo->begin());
  unsigned NumLoopsInIR = countLoopsInUnderlyingIR();

  assert(NumLoopsInCFG == NumLoopsInVPLoopInfo &&
         NumLoopsInVPLoopInfo == NumLoopsInIR &&
         "Number of loops in H-CFG, VPLoopInfo and underlying IR don't match");

  (void)NumLoopsInCFG;
  (void)NumLoopsInVPLoopInfo;
  (void)NumLoopsInIR;
}

// Main class to verify loop information.
void VPlanVerifier::verifyLoops(const VPRegionBlock *TopRegion) const {
  if (all_of(depth_first(TopRegion),
             [](const VPBlockBase *BB) { return isa<VPBasicBlock>(BB); }))
    // Flattened CFG, skip loop regions verification.
    return;
  verifyNumLoops(TopRegion);
  verifyLoopRegions(TopRegion);
}

// Main function for VPRegionBlock verification.
void VPlanVerifier::verifyRegions(const VPRegionBlock *Region) const {

  const VPBlockBase *Entry = Region->getEntry();
  const VPBlockBase *Exit = Region->getExit();

  // At this point, we don't expect Entry or Exit to be another region
  assert(isa<VPBasicBlock>(Entry) && "Region entry is not a VPBasicBlock");
  assert(isa<VPBasicBlock>(Exit) && "Region exit is not a VPBasicBlock");

  // Entry and Exit shouldn't have any predecessor/successor, respectively
  assert(Entry->getNumPredecessors() == 0 && "Region entry has predecessors");
  assert(Exit->getNumSuccessors() == 0 && "Region exit has successors");

  // We are not creating all possible SESE regions. At this point, Entry must
  // have more than two successors and Exit more than two predecessors. This
  // doesn't apply to VPLoopRegion's or TopRegion.
  if (Region->getParent() != nullptr /*TopRegion*/ &&
      !isa<VPLoopRegion>(Region)) {
    assert(Entry->getNumSuccessors() > 1 &&
           "Region entry must have more than one successors");
    assert(Exit->getNumPredecessors() > 1 &&
           "Region exit must have more than one predecessors");
  }
  (void) Entry;
  (void) Exit;

  // Traverse Region's blocks
  unsigned NumBlocks = 0;
  for (const VPBlockBase *VPB :
       make_range(df_iterator<const VPBlockBase *>::begin(Region->getEntry()),
                  df_iterator<const VPBlockBase *>::end(Region->getExit()))) {
    // Compute Region's size
    ++NumBlocks;

    // Check block's parent
    assert(VPB->getParent() == Region && "VPBlockBase has wrong parent");

    // Check block's ConditionBit
    if (VPB->getNumSuccessors() > 1)
      assert(VPB->getCondBit() && "Missing condition bit!");
    else
      assert(!VPB->getCondBit() && "Unexpected condition bit!");

    // Check block's successors
    const auto &Successors = VPB->getSuccessors();
    for (const VPBlockBase *Succ : Successors) {
      // There must be only one instance of the successor in block's successor
      // list. TODO: This won't work for switch statements
      assert(std::count(Successors.begin(), Successors.end(), Succ) == 1 &&
             "Multiple instances of the same successor");

      // There must be a bidirectional link between block and successor
      const auto &SuccPreds = Succ->getPredecessors();
      assert(std::find(SuccPreds.begin(), SuccPreds.end(), VPB) !=
                 SuccPreds.end() &&
             "Missing predecessor link");
      (void) SuccPreds;
    }

    // Check block's predecessors
    const auto &Predecessors = VPB->getPredecessors();
    for (const VPBlockBase *Pred : Predecessors) {

      // Block and predecessor must be inside the same region
      assert(Pred->getParent() == VPB->getParent() &&
             "Predecessor is not in the same region");

      // There must be only one instance of the predecessor in block's
      // predecessor list. TODO: This won't work for switch statements
      assert(std::count(Predecessors.begin(), Predecessors.end(), Pred) == 1 &&
             "Multiple instances of the same predecessor");

      // There must be a bidirectional link between block and predecessor
      const auto &PredSuccs = Pred->getSuccessors();
      assert(std::find(PredSuccs.begin(), PredSuccs.end(), VPB) !=
                 PredSuccs.end() &&
             "Missing successor link");
      (void)PredSuccs;
    }
    verifyBlock(VPB, Region);

    // Recurse here so that we won't need to do the traversal twice.
    if (const auto *SubRegion = dyn_cast<VPRegionBlock>(VPB))
      verifyRegions(SubRegion);
  }

  assert(NumBlocks == Region->getSize() && "Region has a wrong size");
}

#if INTEL_CUSTOMIZATION
void VPlanVerifier::verifyHCFGContext(const VPlan *Plan) {
  Plan->verifyVPConstants();
  Plan->verifyVPExternalDefs();
  Plan->verifyVPExternalDefsHIR();
  Plan->verifyVPMetadataAsValues();
}
#endif

// Public interface to verify the hierarchical CFG.
void VPlanVerifier::verifyHierarchicalCFG(
#if INTEL_CUSTOMIZATION
    const VPlan *Plan,
#endif
    const VPRegionBlock *TopRegion) const {

  if (DisableHCFGVerification)
    return;

  LLVM_DEBUG(dbgs() << "Verifying Hierarchical CFG.\n");

#if INTEL_CUSTOMIZATION
  verifyHCFGContext(Plan);
#endif
  if (VPLInfo)
    verifyLoops(TopRegion);

  verifyRegions(TopRegion);
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
  bool SrcVec = SrcTy->isVectorTy();
  bool DstVec = DstTy->isVectorTy();

  assert(SrcVec == DstVec &&
         "FPToUI source and dest must both be vector or scalar");
  assert(SrcTy->isFPOrFPVectorTy() && "FPToUI source must be FP or FP vector");
  assert(DstTy->isIntOrIntVectorTy() &&
         "FPToUI result must be integer or integer vector");

  if (SrcVec && DstVec)
    assert(cast<VectorType>(SrcTy)->getNumElements() ==
               cast<VectorType>(DstTy)->getNumElements() &&
           "FPToUI source and dest vector length mismatch");
}

void VPlanVerifier::verifyFPToSIInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::FPToSI);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();
  bool SrcVec = SrcTy->isVectorTy();
  bool DstVec = DstTy->isVectorTy();

  assert(SrcVec == DstVec &&
         "FPToSI source and dest must both be vector or scalar");
  assert(SrcTy->isFPOrFPVectorTy() && "FPToSI source must be FP or FP vector");
  assert(DstTy->isIntOrIntVectorTy() &&
         "FPToSI result must be integer or integer vector");

  if (SrcVec && DstVec)
    assert(cast<VectorType>(SrcTy)->getNumElements() ==
               cast<VectorType>(DstTy)->getNumElements() &&
           "FPToSI source and dest vector length mismatch");

}

void VPlanVerifier::verifyUIToFPInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::UIToFP);
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();
  bool SrcVec = SrcTy->isVectorTy();
  bool DstVec = DstTy->isVectorTy();

  assert(SrcVec == DstVec &&
         "UIToFP source and dest must both be vector or scalar");
  assert(SrcTy->isIntOrIntVectorTy() &&
         "UIToFP source must be integer or integer vector");
  assert(DstTy->isFPOrFPVectorTy() && "UIToFP result must be FP or FP vector");

  if (SrcVec && DstVec)
    assert(cast<VectorType>(SrcTy)->getNumElements() ==
               cast<VectorType>(DstTy)->getNumElements() &&
           "UIToFP source and dest vector length mismatch");

}

void VPlanVerifier::verifySIToFPInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::SIToFP);
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();
  bool SrcVec = SrcTy->isVectorTy();
  bool DstVec = DstTy->isVectorTy();

  assert(SrcVec == DstVec &&
         "SIToFP source and dest must both be vector or scalar");
  assert(SrcTy->isIntOrIntVectorTy() &&
         "SIToFP source must be integer or integer vector");
  assert(DstTy->isFPOrFPVectorTy() && "SIToFP result must be FP or FP vector");

  if (SrcVec && DstVec)
    assert(cast<VectorType>(SrcTy)->getNumElements() ==
               cast<VectorType>(DstTy)->getNumElements() &&
           "SIToFP source and dest vector length mismatch");
}

void VPlanVerifier::verifyIntToPtrInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::IntToPtr);
  // Get the source and destination types
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();

  assert(SrcTy->isIntOrIntVectorTy() && "IntToPtr source must be an integral");
  assert(DstTy->isPtrOrPtrVectorTy() && "IntToPtr result must be a pointer");
  assert(SrcTy->isVectorTy() == DstTy->isVectorTy() &&
         "IntToPtr type mismatch");

  auto *Ptr = dyn_cast<PointerType>(DstTy->getScalarType());
  if (Ptr)
    assert(!DL.isNonIntegralPointerType(Ptr) &&
           "inttoptr not supported for non-integral pointers");

  if (SrcTy->isVectorTy()) {
    assert(cast<VectorType>(SrcTy)->getNumElements() ==
               cast<VectorType>(DstTy)->getNumElements() &&
           "IntToPtr Vector width mismatch");
  }

  (void)DstTy;
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

  auto *Ptr = dyn_cast<PointerType>(DstTy->getScalarType());
  if (Ptr)
    assert(!DL.isNonIntegralPointerType(Ptr) &&
           "ptrtoint not supported for non-integral pointers");

  if (SrcTy->isVectorTy()) {
    assert(cast<VectorType>(SrcTy)->getNumElements() ==
               cast<VectorType>(DstTy)->getNumElements() &&
           "PtrToInt Vector width mismatch");
  }
}

void VPlanVerifier::verifyBitCastInst(const VPInstruction *I) const {
  assert(I->getOpcode() == Instruction::BitCast);
  Type *SrcTy = I->getOperand(0)->getType();
  Type *DstTy = I->getType();

  assert((SrcTy && DstTy) && "Invalid Src or Dst type");
  PointerType *SrcPtrTy = dyn_cast<PointerType>(SrcTy->getScalarType());
  PointerType *DstPtrTy = dyn_cast<PointerType>(DstTy->getScalarType());

  assert((!SrcPtrTy == !DstPtrTy) &&
         "BitCast implies a no-op cast of type only. No bits change");

  if (!SrcPtrTy)
    assert(
        (SrcTy->getPrimitiveSizeInBits() == DstTy->getPrimitiveSizeInBits()) &&
        "Source and Dstination bit widths should be identical.");

  if (SrcPtrTy)
    assert(SrcPtrTy->getAddressSpace() == DstPtrTy->getAddressSpace() &&
           "Bitcast: pointer address spaces must match");

  // A vector of pointers must have the same number of elements.
  auto *SrcVecTy = dyn_cast<VectorType>(SrcTy);
  auto *DstVecTy = dyn_cast<VectorType>(DstTy);

  if (SrcVecTy && DstVecTy) {
    assert(
        SrcVecTy->getNumElements() == DstVecTy->getNumElements() &&
        "BitCast: A vector of pointers must have the same number of elements.");
  }

  (void)DstPtrTy;
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
  case Instruction::Store:
    assert(1 && "The type of the operands of Load/Store are not correct");
    break;
  case Instruction::GetElementPtr:
    verifyGEPInstruction(cast<VPGEPInstruction>(VPInst));
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
    // A VPRegion can be set as a predecessor thus we use getExitBasicBlock().
    assert(llvm::find(PBlocks, Block->getExitBasicBlock())!= PBlocks.end() &&
           "A predecessor is not incoming VPBB for VPPHINode");
    (void)Block;
  }
  (void)PBlocks;
}

// Verify operand types of the \p GEP instruction. Also check that the
// OperandIsStructOffset tracker is consistent with number of operands in \p
// GEP.
void VPlanVerifier::verifyGEPInstruction(const VPGEPInstruction *GEP) const {
  // Check base pointer VPValue type. The first operand of the GEP will be the
  // base pointer.
  Type *TargetTy = GEP->getOperand(0)->getType();
  assert(isa<PointerType>(TargetTy) &&
         "GEP base pointer is not a vector or a vector of pointers.");
  (void)TargetTy;

  // Consistency check between operands and OperandIsStructOffset
  assert(GEP->OperandIsStructOffset.size() == GEP->getNumOperands() &&
         "Number of operands and struct offset tracker sizes don't match.");

  // Check that the base pointer and first index operand of GEP is not a struct
  // offset
  assert(
      !GEP->OperandIsStructOffset[0] && !GEP->OperandIsStructOffset[1] &&
      "Base pointer and first index operand of GEP cannot be a struct offset.");

  // Check that each index of GEP is integer or vector of integer type
  for (auto OpIt = GEP->op_begin() + 1; OpIt != GEP->op_end(); ++OpIt) {
    assert((*OpIt)->getType()->isIntOrIntVectorTy() &&
           "GEP indexes must be integers.");
    (void)OpIt;
  }
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

// Verify information of Blocks nested in \p Region.
void VPlanVerifier::verifyBlock(const VPBlockBase *Block,
                                const VPRegionBlock *Region) const {
  if (const auto *BB = dyn_cast<const VPBasicBlock>(Block)) {
    for (const auto &Inst : BB->vpinstructions())
      verifyInstruction(&Inst, BB);
  }
}
