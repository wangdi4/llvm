//===- RegionIdentification.cpp - Identifies HIR Regions ------------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HIR Region Identification pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"

#include "llvm/ADT/Statistic.h"

#include "llvm/Support/Debug.h"

#include "llvm/IR/IntrinsicInst.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/IR/Intel_LoopIR/IRRegion.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_LoopAnalysis/RegionIdentification.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-region-identification"

static cl::opt<unsigned> RegionNumThreshold(
    "region-number-threshold", cl::init(0), cl::Hidden,
    cl::desc("Threshold for number of regions to create HIR for, 0 means no"
             " threshold"));

STATISTIC(RegionCount, "Number of regions created");

INITIALIZE_PASS_BEGIN(RegionIdentification, "hir-region-identification",
                      "HIR Region Identification", false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTree)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_END(RegionIdentification, "hir-region-identification",
                    "HIR Region Identification", false, true)

char RegionIdentification::ID = 0;

FunctionPass *llvm::createRegionIdentificationPass() {
  return new RegionIdentification();
}

RegionIdentification::RegionIdentification() : FunctionPass(ID) {
  initializeRegionIdentificationPass(*PassRegistry::getPassRegistry());
}

void RegionIdentification::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<PostDominatorTree>();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<ScalarEvolutionWrapperPass>();
}

const GEPOperator *
RegionIdentification::getBaseGEPOp(const GEPOperator *GEPOp) const {

  while (auto TempGEPOp = dyn_cast<GEPOperator>(GEPOp->getPointerOperand())) {
    GEPOp = TempGEPOp;
  }

  return GEPOp;
}

Type *RegionIdentification::getPrimaryElementType(Type *PtrTy) const {
  assert(isa<PointerType>(PtrTy) && "Unexpected type!");

  Type *ElTy = cast<PointerType>(PtrTy)->getElementType();

  // Recurse into array types, if any.
  for (; ArrayType *ArrTy = dyn_cast<ArrayType>(ElTy);
       ElTy = ArrTy->getElementType()) {
  }

  return ElTy;
}

bool RegionIdentification::isHeaderPhi(const PHINode *Phi) const {
  auto ParentBB = Phi->getParent();

  auto Lp = LI->getLoopFor(ParentBB);

  if (!Lp) {
    return false;
  }

  if (Lp->getHeader() == ParentBB) {
    assert((Phi->getNumIncomingValues() == 2) &&
           "Unexpected number of operands for header phi!");
    return true;
  }

  return false;
}

bool RegionIdentification::isSupported(Type *Ty) const {
  assert(Ty && "Type is null!");

  for (; SequentialType *SeqTy = dyn_cast<SequentialType>(Ty);) {
    if (SeqTy->isVectorTy()) {
      DEBUG(dbgs()
            << "LOOPOPT_OPTREPORT: vector types currently not supported.\n");
      return false;
    }
    Ty = SeqTy->getElementType();
  }

  if (Ty->isStructTy() || Ty->isFunctionTy()) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: structure/function pointer types "
                    "currently not supported.\n");
    return false;
  }

  auto IntType = dyn_cast<IntegerType>(Ty);
  // Integer type greater than 64 bits not supported.This is mainly to throttle
  // 128 bit integers.
  if (IntType && (IntType->getPrimitiveSizeInBits() > 64)) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: integer types greater than 64 bits "
                    "currently not supported.\n");
    return false;
  }

  return true;
}

bool RegionIdentification::containsUnsupportedTy(
    const Instruction *Inst) const {

  if (auto GEPOp = dyn_cast<GEPOperator>(Inst)) {
    GEPOp = getBaseGEPOp(GEPOp);

    if (!isSupported(GEPOp->getSourceElementType())) {
      return true;
    }
  }

  unsigned NumOp = Inst->getNumOperands();

  // Skip checking the last operand of the call instruction which is the call
  // itself. It has a function pointer type which we do not support right now
  // but we do not want to throttle simple function calls.
  if (isa<CallInst>(Inst)) {
    --NumOp;
  }

  // Check instruction operands
  for (unsigned I = 0; I < NumOp; ++I) {

    if (const GEPOperator *GEPOp = dyn_cast<GEPOperator>(Inst->getOperand(I))) {
      GEPOp = getBaseGEPOp(GEPOp);

      if (!isSupported(GEPOp->getSourceElementType())) {
        return true;
      }

    } else if (!isSupported(Inst->getOperand(I)->getType())) {
      return true;
    }
  }

  return false;
}

const PHINode *
RegionIdentification::findIVDefInHeader(const Loop &Lp,
                                        const Instruction *Inst) const {

  // Is this a phi node in the loop header?
  if (Inst->getParent() == Lp.getHeader()) {
    if (auto Phi = dyn_cast<PHINode>(Inst)) {
      return Phi;
    }
  }

  for (auto I = Inst->op_begin(), E = Inst->op_end(); I != E; ++I) {
    if (auto OPInst = dyn_cast<Instruction>(I)) {
      // Instruction lies outside the loop.
      if (!Lp.contains(LI->getLoopFor(OPInst->getParent()))) {
        continue;
      }

      auto IVNode = findIVDefInHeader(Lp, OPInst);

      if (IVNode) {
        return IVNode;
      }
    }
  }

  return nullptr;
}

bool RegionIdentification::isSelfGenerable(const Loop &Lp,
                                           unsigned LoopnestDepth) const {

  // At least one of this loop's subloops reach MaxLoopNestLevel so we cannot
  // generate this loop.
  if (LoopnestDepth > MaxLoopNestLevel) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loopnest is more than "
                 << MaxLoopNestLevel << " deep.\n");
    return false;
  }

  // Loop is not in a handleable form.
  if (!Lp.isLoopSimplifyForm()) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loop structure is not handleable.\n");
    return false;
  }

  // Don't handle multi-exit loops for now.
  if (!Lp.getExitingBlock()) {
    DEBUG(dbgs()
          << "LOOPOPT_OPTREPORT: Multi-exit loops currently not supported.\n");
    return false;
  }

  // Skip loop with vectorize/unroll pragmas for now so that tests checking for
  // these are not affected.
  if (Lp.getLoopID()) {
    DEBUG(
        dbgs()
        << "LOOPOPT_OPTREPORT: Loops with pragmas currently not supported.\n");
    return false;
  }

  // Don't handle unknown loops for now.
  if (!SE->hasLoopInvariantBackedgeTakenCount(&Lp)) {
    DEBUG(dbgs()
          << "LOOPOPT_OPTREPORT: Unknown loops currently not supported.\n");
    return false;
  }

  // SCEV doesn't seem to set type of (ptr1 - ptr2) to integer in some cases
  // which causes issues in HIR.
  // TODO: look into SCEV analysis logic.
  if (SE->getBackedgeTakenCount(&Lp)->getType()->isPointerTy()) {
    DEBUG(dbgs()
          << "LOOPOPT_OPTREPORT: Pointer type trip count not supported.\n");
    return false;
  }

  auto LatchBB = Lp.getLoopLatch();

  // We cannot build lexical links if dominator/post-dominator info is absent.
  // This can be due to unreachable/infinite loops.
  if (!DT->getNode(LatchBB) || !PDT->getNode(LatchBB)) {
    DEBUG(dbgs()
          << "LOOPOPT_OPTREPORT: Unreachable/Infinite loops not supported.\n");
    return false;
  }

  // Check that the loop backedge is a conditional branch.
  auto Term = LatchBB->getTerminator();
  auto BrInst = dyn_cast<BranchInst>(Term);

  if (!BrInst) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Non-branch instrcutions in loop latch "
                    "currently not supported.\n");
    return false;
  }

  if (BrInst->isUnconditional()) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Unconditional branch instrcutions in "
                    "loop latch currently not supported.\n");
    return false;
  }

  const Value *LatchVal = BrInst->getCondition();

  auto LatchCmpInst = dyn_cast<Instruction>(LatchVal);

  if (!LatchCmpInst) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Non-instruction latch condition "
                    "currently not supported.\n");
    return false;
  }

  auto IVNode = findIVDefInHeader(Lp, LatchCmpInst);

  if (!IVNode) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Could not find loop IV.\n");
    return false;
  }

  // Check instructions inside the loop.
  for (auto I = Lp.block_begin(), E = Lp.block_end(); I != E; ++I) {

    // Skip this bblock as it has been checked by an inner loop.
    if (!Lp.empty() && LI->getLoopFor(*I) != (&Lp)) {
      continue;
    }

    if ((*I)->isLandingPad()) {
      DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Exception handling currently not "
                      "supported.\n");
      return false;
    }

    Term = (*I)->getTerminator();

    if (isa<IndirectBrInst>(Term)) {
      DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Indirect branches currently not "
                      "supported.\n");
      return false;
    }

    if (isa<InvokeInst>(Term) || isa<ResumeInst>(Term)) {
      DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Exception handling currently not "
                      "supported.\n");
      return false;
    }

    for (auto InstIt = (*I)->begin(), EndIt = (*I)->end(); InstIt != EndIt;
         ++InstIt) {

      if (InstIt->isAtomic()) {
        DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Atomic instructions are currently "
                        "not supported.\n");
        return false;
      }

      if (InstIt->getType()->isVectorTy()) {
        DEBUG(dbgs()
              << "LOOPOPT_OPTREPORT: Vector types currently not supported.\n");
        return false;
      }

      if (containsUnsupportedTy(&*InstIt)) {
        return false;
      }
    }
  }

  return true;
}

bool RegionIdentification::isSIMDLoop(const Loop &Lp) const {
  BasicBlock *PreheaderBB = Lp.getLoopPreheader();

  // Check if the first instruction is a SIMD directive.
  auto FirstInst = PreheaderBB->begin();
  auto IntrinInst = dyn_cast<IntrinsicInst>(FirstInst);

  if (!IntrinInst) {
    return false;
  }

  Value *Op = IntrinInst->getOperand(0);
  MetadataAsValue *MDVal = dyn_cast<MetadataAsValue>(Op);

  if (!MDVal) {
    return false;
  }

  auto MD = dyn_cast<MDNode>(MDVal->getMetadata());

  if (!MD || (MD->getNumOperands() != 1)) {
    return false;
  }

  MDString *MDStr = dyn_cast<MDString>(MD->getOperand(0));

  // TODO: Replace string literal with the defined value when it is available.
  if (!MDStr || !(MDStr->getString().equals("DIR.OMP.SIMD"))) {
    return false;
  }

  return true;
}

void RegionIdentification::createRegion(const Loop &Lp) {

  if (RegionNumThreshold && (RegionCount == RegionNumThreshold)) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Region throttled due to region number "
                    "threshold.\n");
    return;
  }

  IRRegion::RegionBBlocksTy BBlocks(Lp.getBlocks().begin(),
                                    Lp.getBlocks().end());
  BasicBlock *EntryBB = nullptr;

  if (isSIMDLoop(Lp)) {
    // Include preheader in the region as it contains SIMD directives.
    EntryBB = Lp.getLoopPreheader();
    BBlocks.insert(EntryBB);
  } else {
    EntryBB = Lp.getHeader();
  }

  IRRegion *Reg = new IRRegion(EntryBB, BBlocks);
  IRRegions.push_back(Reg);
  RegionCount++;
}

bool RegionIdentification::formRegionForLoop(const Loop &Lp,
                                             unsigned *LoopnestDepth) {
  SmallVector<Loop *, 8> GenerableLoops;
  bool Generable = true;

  *LoopnestDepth = 0;

  // Check which sub loops are generable.
  for (auto I = Lp.begin(), E = Lp.end(); I != E; ++I) {
    unsigned SubLoopnestDepth;

    if (formRegionForLoop(**I, &SubLoopnestDepth)) {
      GenerableLoops.push_back(*I);

      // Set maximum sub-loopnest depth
      *LoopnestDepth = std::max(*LoopnestDepth, SubLoopnestDepth);
    } else {
      Generable = false;
    }
  }

  // Check whether Lp is generable.
  if (Generable && !isSelfGenerable(Lp, ++(*LoopnestDepth))) {
    Generable = false;
  }

  // Lp itself is not generable so create regions for generable sub loops.
  if (!Generable) {
    // TODO: add logic to merge fuseable loops. This might also require
    // recognition of ztt and splitting basic blocks which needs to be done
    // in a transformation pass.
    for (auto I = GenerableLoops.begin(), E = GenerableLoops.end(); I != E;
         ++I) {
      createRegion(**I);
    }
  }

  return Generable;
}

void RegionIdentification::formRegions() {
  // LoopInfo::iterator visits loops in reverse program order so we need to use
  // reverse_iterator here.
  for (LoopInfo::reverse_iterator I = LI->rbegin(), E = LI->rend(); I != E;
       ++I) {
    unsigned Depth;
    if (formRegionForLoop(**I, &Depth)) {
      createRegion(**I);
    }
  }
}

bool RegionIdentification::runOnFunction(Function &F) {
  if (skipOptnoneFunction(F)) {
    return false;
  }

  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  PDT = &getAnalysis<PostDominatorTree>();
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();

  formRegions();

  return false;
}

void RegionIdentification::releaseMemory() {
  IRRegion::destroyAll();
  IRRegions.clear();
}

void RegionIdentification::print(raw_ostream &OS, const Module *M) const {

  for (auto I = IRRegions.begin(), E = IRRegions.end(); I != E; ++I) {
    OS << "\nRegion " << I - IRRegions.begin() + 1 << "\n";
    (*I)->print(OS, 3);
    OS << "\n";
  }
}

void RegionIdentification::verifyAnalysis() const {
  /// TODO: implement later
}
