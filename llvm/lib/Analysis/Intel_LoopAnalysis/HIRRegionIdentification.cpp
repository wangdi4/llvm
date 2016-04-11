//===- HIRRegionIdentification.cpp - Identifies HIR Regions ---------------===//
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

#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/IntrinsicInst.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/IR/Intel_LoopIR/IRRegion.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRRegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-region-identification"

static cl::opt<unsigned> RegionNumThreshold(
    "hir-region-number-threshold", cl::init(0), cl::Hidden,
    cl::desc("Threshold for number of regions to create HIR for, 0 means no"
             " threshold"));

static cl::opt<bool> CostModelThrottling(
    "hir-cost-model-throttling", cl::init(true), cl::Hidden,
    cl::desc("Throttles loops deemed non-profitable by the cost model"));

STATISTIC(RegionCount, "Number of regions created");

INITIALIZE_PASS_BEGIN(HIRRegionIdentification, "hir-region-identification",
                      "HIR Region Identification", false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_END(HIRRegionIdentification, "hir-region-identification",
                    "HIR Region Identification", false, true)

char HIRRegionIdentification::ID = 0;

FunctionPass *llvm::createHIRRegionIdentificationPass() {
  return new HIRRegionIdentification();
}

HIRRegionIdentification::HIRRegionIdentification() : FunctionPass(ID) {
  initializeHIRRegionIdentificationPass(*PassRegistry::getPassRegistry());
}

void HIRRegionIdentification::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<PostDominatorTreeWrapperPass>();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<ScalarEvolutionWrapperPass>();
}

const GEPOperator *
HIRRegionIdentification::getBaseGEPOp(const GEPOperator *GEPOp) const {

  while (auto TempGEPOp = dyn_cast<GEPOperator>(GEPOp->getPointerOperand())) {
    GEPOp = TempGEPOp;
  }

  return GEPOp;
}

Type *HIRRegionIdentification::getPrimaryElementType(Type *PtrTy) const {
  assert(isa<PointerType>(PtrTy) && "Unexpected type!");

  Type *ElTy = cast<PointerType>(PtrTy)->getElementType();

  // Recurse into array types, if any.
  for (; ArrayType *ArrTy = dyn_cast<ArrayType>(ElTy);
       ElTy = ArrTy->getElementType()) {
  }

  return ElTy;
}

bool HIRRegionIdentification::isHeaderPhi(const PHINode *Phi) const {
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

bool HIRRegionIdentification::isSupported(Type *Ty) const {
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

bool HIRRegionIdentification::containsUnsupportedTy(
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
HIRRegionIdentification::findIVDefInHeader(const Loop &Lp,
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

class HIRRegionIdentification::CostModelAnalyzer
    : public InstVisitor<CostModelAnalyzer, bool> {
  const HIRRegionIdentification &RI;
  const Loop &Lp;
  DomTreeNode *HeaderDomNode;
  bool IsProfitable;
  unsigned InstCount;             // Approximates number of instructions in HIR.
  unsigned UnstructuredJumpCount; // Approximates goto/label counts in HIR.
  unsigned IfCount;               // Approximates number of ifs in HIR.

  // TODO: use different values for O2/O3.
  const unsigned MaxInstThreshold = 200;
  const unsigned MaxIfThreshold = 7;
  const unsigned MaxIfNestThreshold = 2;

public:
  CostModelAnalyzer(const HIRRegionIdentification &RI, const Loop &Lp)
      : RI(RI), Lp(Lp), IsProfitable(true), InstCount(0),
        UnstructuredJumpCount(0), IfCount(0) {
    HeaderDomNode = RI.DT->getNode(Lp.getHeader());
  }

  bool isProfitable() const { return IsProfitable; }

  void analyze() {
    for (auto BB = Lp.block_begin(), E = Lp.block_end(); BB != E; ++BB) {
      if (!visitBasicBlock(**BB)) {
        IsProfitable = false;
        break;
      }
    }
  }

  bool visitBasicBlock(const BasicBlock &BB);
  bool visitInstruction(const Instruction &Inst);
  bool visitCallInst(const CallInst &CI);
  bool visitBranchInst(const BranchInst &BI);
};

bool HIRRegionIdentification::CostModelAnalyzer::visitBasicBlock(
    const BasicBlock &BB) {
  for (auto &Inst : BB) {
    if (!visit(const_cast<Instruction &>(Inst))) {
      return false;
    }
  }

  return true;
}

bool HIRRegionIdentification::CostModelAnalyzer::visitInstruction(
    const Instruction &Inst) {
  // Compares are most likely eliminated in HIR.
  if (!isa<CmpInst>(Inst)) {

    // The following checks are to ignore linear instructions.
    if (RI.SE->isSCEVable(Inst.getType())) {
      auto SC = RI.SE->getSCEV(const_cast<Instruction *>(&Inst));
      auto AddRec = dyn_cast<SCEVAddRecExpr>(SC);

      if (!AddRec || !AddRec->isAffine()) {
        auto Phi = dyn_cast<PHINode>(&Inst);

        if (Phi) {
          // Non-linear phis will be deconstructed using copy stmts for each
          // operand.
          InstCount += Phi->getNumIncomingValues();
        } else {
          ++InstCount;
        }
      }
    } else {
      ++InstCount;
    }
  }

  bool Ret = (InstCount <= MaxInstThreshold);

  if (!Ret) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of too "
                    "many statements.\n");
  }

  return Ret;
}

bool HIRRegionIdentification::CostModelAnalyzer::visitCallInst(
    const CallInst &CI) {
  if (!isa<IntrinsicInst>(CI)) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of user "
                    "calls.\n");
    return false;
  }

  return visitInstruction(static_cast<const Instruction &>(CI));
}

bool HIRRegionIdentification::CostModelAnalyzer::visitBranchInst(
    const BranchInst &BI) {
  if (BI.isUnconditional()) {
    return visitInstruction(static_cast<const Instruction &>(BI));
  }

  if (++IfCount > MaxIfThreshold) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of too "
                    "many ifs.\n");
    return false;
  }

  unsigned IfNestCount = 0;
  auto ParentBB = BI.getParent();
  auto DomNode = RI.DT->getNode(const_cast<BasicBlock *>(ParentBB));

  while (DomNode != HeaderDomNode) {
    assert(DomNode && "Dominator tree node of a loop bblock is null!");
    ++IfNestCount;
    DomNode = DomNode->getIDom();
  }

  if (IfNestCount > MaxIfNestThreshold) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of too "
                    "many nested ifs.\n");
    return false;
  }

  // Complex CFG checks do not apply to headers/latches.
  if ((ParentBB == Lp.getHeader()) || (ParentBB == Lp.getLoopLatch())) {
    return true;
  }

  auto Succ0 = BI.getSuccessor(0);
  auto Succ1 = BI.getSuccessor(1);

  // Within the same loop, conditional branches not dominating its successor and
  // the successor not post-dominating the branch indicates presence of a goto
  // in HLLoop.
  if (((RI.LI->getLoopFor(Succ0) == &Lp) && !RI.DT->dominates(ParentBB, Succ0) &&
       !RI.PDT->dominates(Succ0, ParentBB)) ||
      ((RI.LI->getLoopFor(Succ1) == &Lp) && !RI.DT->dominates(ParentBB, Succ1) &&
       !RI.PDT->dominates(Succ1, ParentBB))) {
    DEBUG(dbgs()
          << "LOOPOPT_OPTREPORT: Loop throttled due to presence of goto.\n");
    return false;
  }

  return true;
}

bool HIRRegionIdentification::shouldThrottleLoop(const Loop &Lp) const {

  if (!CostModelThrottling) {
    return false;
  }

  // Restrict checks to innermost loops for now. This can be expanded later.
  if (!Lp.empty()) {
    return false;
  }

  CostModelAnalyzer CMA(*this, Lp);
  CMA.analyze();

  return !CMA.isProfitable();
}

bool HIRRegionIdentification::isSelfGenerable(const Loop &Lp,
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
  // these are not affected. Allow SIMD loops.
  if (!isSIMDLoop(Lp) && Lp.getLoopID()) {
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

  if (IVNode->getType()->getPrimitiveSizeInBits() == 1) {
    // The following loop with i1 type IV has a trip count of 2 which is outside
    // its range. This is a quirk of SSA. CG will generate an infinite loop for
    // this case if we let it through.
    // for.i:
    // %i.08.i = phi i1 [ true, %entry ], [ false, %for.i ]
    // br i1 %i.08.i, label %for.i, label %exit
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: i1 type IV currently not handled.\n");
    return false;
  }

  // TODO: move into a separate function.
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

  if (shouldThrottleLoop(Lp)) {
    return false;
  }

  return true;
}

bool HIRRegionIdentification::isSIMDLoop(const Loop &Lp) const {
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

void HIRRegionIdentification::createRegion(const Loop &Lp) {

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

bool HIRRegionIdentification::formRegionForLoop(const Loop &Lp,
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

void HIRRegionIdentification::formRegions() {
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

bool HIRRegionIdentification::runOnFunction(Function &F) {
  if (skipOptnoneFunction(F)) {
    return false;
  }

  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  PDT = &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();

  formRegions();

  return false;
}

void HIRRegionIdentification::releaseMemory() {
  IRRegion::destroyAll();
  IRRegions.clear();
}

void HIRRegionIdentification::print(raw_ostream &OS, const Module *M) const {

  for (auto I = IRRegions.begin(), E = IRRegions.end(); I != E; ++I) {
    OS << "\nRegion " << I - IRRegions.begin() + 1 << "\n";
    (*I)->print(OS, 3);
    OS << "\n";
  }
}

void HIRRegionIdentification::verifyAnalysis() const {
  /// TODO: implement later
}
