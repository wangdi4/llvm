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

#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/IntrinsicInst.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/TargetLibraryInfo.h"

#include "llvm/IR/Intel_LoopIR/CanonExpr.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRRegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"

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

static cl::opt<bool> DisablePragmaBailOut(
    "disable-hir-pragma-bailout", cl::init(false), cl::Hidden,
    cl::desc("Disable HIR bailout for non unroll/vectorizer loop metadata"));

static cl::opt<bool> CreateFunctionLevelRegion(
    "hir-create-function-level-region", cl::init(false), cl::Hidden,
    cl::desc("force HIR to create a single function level region instead of "
             "creating regions for individual loopnests"));

STATISTIC(RegionCount, "Number of regions created");

INITIALIZE_PASS_BEGIN(HIRRegionIdentification, "hir-region-identification",
                      "HIR Region Identification", false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
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
  AU.addRequiredTransitive<TargetLibraryInfoWrapperPass>();
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

bool HIRRegionIdentification::isSupported(Type *Ty) {
  assert(Ty && "Type is null!");

  while (isa<SequentialType>(Ty) || isa<PointerType>(Ty)) {
    if (auto SeqTy = dyn_cast<SequentialType>(Ty)) {
      if (SeqTy->isVectorTy()) {
        DEBUG(dbgs()
              << "LOOPOPT_OPTREPORT: vector types currently not supported.\n");
        return false;
      }
      Ty = SeqTy->getElementType();
    } else {
      Ty = Ty->getPointerElementType();
    }
  }

  if (Ty->isFunctionTy()) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: function pointer types currently not "
                    "supported.\n");
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

bool HIRRegionIdentification::containsUnsupportedTy(const GEPOperator *GEPOp) {
  SmallVector<Value *, 8> Operands;

  auto BaseTy =
      cast<PointerType>(GEPOp->getPointerOperandType())->getElementType();

  if (!isSupported(BaseTy)) {
    return true;
  }

  unsigned NumOp = GEPOp->getNumOperands() - 1;
  Operands.push_back(const_cast<Value *>(GEPOp->getOperand(1)));

  for (unsigned I = 2; I <= NumOp; ++I) {
    Operands.push_back(const_cast<Value *>(GEPOp->getOperand(I)));

    auto OpTy = GetElementPtrInst::getIndexedType(BaseTy, Operands);

    if (!isSupported(OpTy)) {
      return true;
    }
  }

  return false;
}

bool HIRRegionIdentification::containsUnsupportedTy(const Instruction *Inst) {

  if (auto GEPOp = dyn_cast<GEPOperator>(Inst)) {
    return containsUnsupportedTy(GEPOp);
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
    if (!isSupported(Inst->getOperand(I)->getType())) {
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
    if (auto OpInst = dyn_cast<Instruction>(I)) {

      // Instruction lies outside the loop.
      if (!Lp.contains(LI->getLoopFor(OpInst->getParent()))) {
        continue;
      }

      // Skip backedges.
      // This can happen for outer unknown loops.
      if (DT->dominates(Inst, OpInst)) {
        continue;
      }

      auto IVNode = findIVDefInHeader(Lp, OpInst);

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
    bool IsInnermostLoop = Lp.empty();

    for (auto BB = Lp.block_begin(), E = Lp.block_end(); BB != E; ++BB) {

      // Skip bblocks which belong to inner loops.
      if (!IsInnermostLoop && (RI.LI->getLoopFor(*BB) != &Lp)) {
        continue;
      }

      if (!visitBasicBlock(**BB)) {
        IsProfitable = false;
        break;
      }
    }
  }

  bool visitBasicBlock(const BasicBlock &BB);
  bool visitInstruction(const Instruction &Inst);
  bool visitLoadInst(const LoadInst &LI);
  bool visitStoreInst(const StoreInst &SI);
  bool visitCallInst(const CallInst &CI);
  bool visitBranchInst(const BranchInst &BI);
};

bool HIRRegionIdentification::CostModelAnalyzer::visitBasicBlock(
    const BasicBlock &BB) {

  auto BBInstCount = BB.size();

  // Bail out early instead of analyzing each individual instruction.
  if ((BBInstCount + InstCount) > MaxInstThreshold) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of too "
                    "many statements.\n");
    return false;
  }

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

bool HIRRegionIdentification::CostModelAnalyzer::visitLoadInst(
    const LoadInst &LI) {
  if (LI.isVolatile()) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of "
                    "volatile load.\n");
    return false;
  }

  return visitInstruction(static_cast<const Instruction &>(LI));
}

bool HIRRegionIdentification::CostModelAnalyzer::visitStoreInst(
    const StoreInst &SI) {
  if (SI.isVolatile()) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of "
                    "volatile store.\n");
    return false;
  }

  return visitInstruction(static_cast<const Instruction &>(SI));
}

bool HIRRegionIdentification::CostModelAnalyzer::visitCallInst(
    const CallInst &CI) {

  if (!isa<IntrinsicInst>(CI)) {
    auto Func = CI.getCalledFunction();

    if (!Func || !RI.TLI->isFunctionVectorizable(Func->getName())) {
      DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of "
                      "user calls.\n");
      return false;
    }
  }

  return visitInstruction(static_cast<const Instruction &>(CI));
}

bool HIRRegionIdentification::CostModelAnalyzer::visitBranchInst(
    const BranchInst &BI) {
  if (BI.isUnconditional()) {
    return visitInstruction(static_cast<const Instruction &>(BI));
  }

  auto ParentBB = BI.getParent();

  // Complex CFG checks do not apply to headers/latches.
  if ((ParentBB == Lp.getHeader()) || (ParentBB == Lp.getLoopLatch())) {
    return true;
  }

  if (++IfCount > MaxIfThreshold) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of too "
                    "many ifs.\n");
    return false;
  }

  unsigned IfNestCount = 0;
  auto DomNode = RI.DT->getNode(const_cast<BasicBlock *>(ParentBB));

  while (DomNode != HeaderDomNode) {
    assert(DomNode && "Dominator tree node of a loop bblock is null!");

    // Consider this a nested if scenario only if the dominator has a single
    // predecessor otherwise sibling ifs may be counted as nested due to
    // merge/join bblocks.
    // Nested ifs look like this-
    // if () {
    //   if () {
    //   }
    // }
    //
    // As opposed to sibling ifs-
    //
    // if () {
    // } else {
    // }
    //     <-- The merge point is a dominator of the sibling if.
    // if() {
    // }
    //
    if (DomNode->getBlock()->getSinglePredecessor()) {
      ++IfNestCount;
    }

    DomNode = DomNode->getIDom();
  }

  // Add 1 to include reaching header node.
  if ((IfNestCount + 1) > MaxIfNestThreshold) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of too "
                    "many nested ifs.\n");
    return false;
  }

  auto Succ0 = BI.getSuccessor(0);
  auto Succ1 = BI.getSuccessor(1);

  // Within the same loop, conditional branches not dominating its successor and
  // the successor not post-dominating the branch indicates presence of a goto
  // in HLLoop.
  if (((RI.LI->getLoopFor(Succ0) == &Lp) &&
       !RI.DT->dominates(ParentBB, Succ0) &&
       !RI.PDT->dominates(Succ0, ParentBB)) ||
      ((RI.LI->getLoopFor(Succ1) == &Lp) &&
       !RI.DT->dominates(ParentBB, Succ1) &&
       !RI.PDT->dominates(Succ1, ParentBB))) {
    DEBUG(dbgs()
          << "LOOPOPT_OPTREPORT: Loop throttled due to presence of goto.\n");
    return false;
  }

  return true;
}

bool HIRRegionIdentification::shouldThrottleLoop(const Loop &Lp,
                                                 bool IsUnknown) const {

  if (!CostModelThrottling) {
    return false;
  }

  // SIMD loops should not be throttled.
  if (isSIMDLoop(Lp)) {
    return false;
  }

  // Only handle standalone single bblock unknown loops for now. We don't do
  // much for outer unknown loops except prefetching which isn't ready yet.
  // Inner unknown loops are throttled for compile time reasons.
  if (IsUnknown && ((Lp.getNumBlocks() != 1) || (Lp.getLoopDepth() != 1))) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: unknown loop throttled for compile "
                    "time reasons.\n");
    return true;
  }

  CostModelAnalyzer CMA(*this, Lp);
  CMA.analyze();

  return !CMA.isProfitable();
}

bool HIRRegionIdentification::isDebugMetadataOnly(MDNode *Node) {
  unsigned Ops = Node->getNumOperands();
  if (Ops == 1) {
    return isa<DILocation>(Node) || isa<DINode>(Node);
  }

  for (unsigned I = 0; I < Ops; ++I) {
    MDNode *OpNode = dyn_cast<MDNode>(Node->getOperand(I));
    if (OpNode == Node) {
      continue;
    }

    if (!OpNode || !isDebugMetadataOnly(OpNode)) {
      return false;
    }
  }

  return true;
}

bool HIRRegionIdentification::isReachableFromImpl(
    const BasicBlock *BB, const SmallPtrSetImpl<const BasicBlock *> &EndBBs,
    const SmallPtrSetImpl<const BasicBlock *> &FromBBs,
    SmallPtrSetImpl<const BasicBlock *> &VisitedBBs) const {

  if (FromBBs.count(BB)) {
    return true;
  }

  if (EndBBs.count(BB)) {
    return false;
  }

  if (VisitedBBs.count(BB)) {
    return false;
  } else {
    VisitedBBs.insert(BB);
  }

  for (auto Pred = pred_begin(BB), E = pred_end(BB); Pred != E; ++Pred) {
    auto PredBB = *Pred;

    // Skip recursing into backedges.
    if (!DT->dominates(BB, PredBB) &&
        isReachableFromImpl(PredBB, EndBBs, FromBBs, VisitedBBs)) {
      return true;
    }
  }

  return false;
}

bool HIRRegionIdentification::isReachableFrom(
    const BasicBlock *BB, const SmallPtrSetImpl<const BasicBlock *> &EndBBs,
    const SmallPtrSetImpl<const BasicBlock *> &FromBBs) const {
  SmallPtrSet<const BasicBlock *, 32> VisitedBBs;

  return isReachableFromImpl(BB, EndBBs, FromBBs, VisitedBBs);
}

bool HIRRegionIdentification::containsCycle(const BasicBlock *BB,
                                            const Loop *Lp) const {
  SmallVector<BasicBlock *, 8> DomChildren;

  auto Node = DT->getNode(const_cast<BasicBlock *>(BB));

  // Collect dominator children in the same loop.
  for (auto &I : (*Node)) {
    auto BB = I->getBlock();

    if (!Lp || (LI->getLoopFor(BB) == Lp)) {
      DomChildren.push_back(BB);
    }
  }

  unsigned Size = DomChildren.size();

  if (Size < 2) {
    return false;
  }

  SmallPtrSet<const BasicBlock *, 1> EndBBs;
  SmallPtrSet<const BasicBlock *, 1> FromBBs;
  EndBBs.insert(BB);

  // For each pair of dominator children, check if they can reach each other
  // without going through the dominator.
  for (unsigned I = 0; I < Size - 1; ++I) {
    auto ChildBB1 = DomChildren[I];

    for (unsigned J = I + 1; J < Size; ++J) {
      auto ChildBB2 = DomChildren[J];

      FromBBs.clear();
      FromBBs.insert(ChildBB2);

      if (!isReachableFrom(ChildBB1, EndBBs, FromBBs)) {
        continue;
      }

      FromBBs.clear();
      FromBBs.insert(ChildBB1);

      if (isReachableFrom(ChildBB2, EndBBs, FromBBs)) {
        return true;
      }
    }
  }

  return false;
}

bool HIRRegionIdentification::isGenerable(const BasicBlock *BB) {
  auto FirstInst = BB->getFirstNonPHI();

  if (isa<LandingPadInst>(FirstInst) || isa<FuncletPadInst>(FirstInst)) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Exception handling currently not "
                    "supported.\n");
    return false;
  }

  auto Term = BB->getTerminator();

  if (isa<IndirectBrInst>(Term)) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Indirect branches currently not "
                    "supported.\n");
    return false;
  }

  if (isa<InvokeInst>(Term) || isa<ResumeInst>(Term) ||
      isa<CatchSwitchInst>(Term) || isa<CatchReturnInst>(Term) ||
      isa<CleanupReturnInst>(Term)) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Exception handling currently not "
                    "supported.\n");
    return false;
  }

  // Skip the terminator instruction.
  for (auto Inst = BB->begin(), E = std::prev(BB->end()); Inst != E; ++Inst) {

    if (Inst->isAtomic()) {
      DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Atomic instructions are currently "
                      "not supported.\n");
      return false;
    }

    // TODO: think about HIR representation for
    // InsertValueInst/ExtractValueInst.
    if (isa<InsertValueInst>(Inst) || isa<ExtractValueInst>(Inst)) {
      DEBUG(dbgs() << "LOOPOPT_OPTREPORT: InsertValueInst/ExtractValueInst "
                      "currently not supported.\n");
      return false;
    }

    if (Inst->getType()->isVectorTy()) {
      DEBUG(dbgs()
            << "LOOPOPT_OPTREPORT: Vector types currently not supported.\n");
      return false;
    }

    if (auto CInst = dyn_cast<CallInst>(Inst)) {
      if (CInst->isInlineAsm()) {
        DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Inline assembly currently not "
                        "supported.\n");
        return false;
      }
    }

    if (containsUnsupportedTy(&*Inst)) {
      return false;
    }
  }

  return true;
}

bool HIRRegionIdentification::areBBlocksGenerable(const Loop &Lp) const {
  bool IsInnermostLoop = Lp.empty();

  // Check instructions inside the loop.
  for (auto BB = Lp.block_begin(), E = Lp.block_end(); BB != E; ++BB) {

    // Skip this bblock as it has been checked by an inner loop.
    if (!IsInnermostLoop && LI->getLoopFor(*BB) != (&Lp)) {
      continue;
    }

    if (!isGenerable(*BB)) {
      return false;
    }

    // TODO: Is there a more efficient way to check this?
    if (containsCycle(*BB, &Lp)) {
      DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Irreducible CFG not supported.\n");
      return false;
    }
  }

  return true;
}

bool HIRRegionIdentification::isSelfGenerable(const Loop &Lp,
                                              unsigned LoopnestDepth,
                                              bool IsFunctionRegionMode) const {

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
  // these are not affected. Allow SIMD loops and dbg metadata.
  MDNode *LoopID = Lp.getLoopID();
  if (!DisablePragmaBailOut && !isSIMDLoop(Lp) && LoopID &&
      !isDebugMetadataOnly(LoopID)) {
    DEBUG(
        dbgs()
        << "LOOPOPT_OPTREPORT: Loops with pragmas currently not supported.\n");
    return false;
  }

  auto BECount = SE->getBackedgeTakenCount(&Lp);

  auto UndefBECount = dyn_cast<SCEVUnknown>(BECount);

  if (UndefBECount && isa<UndefValue>(UndefBECount->getValue())) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loops with undef backedge taken count "
                    "currently not supported.\n");
    return false;
  }

  auto ConstBECount = dyn_cast<SCEVConstant>(BECount);

  // This represents a trip count of 2^n while we can only handle a trip count
  // up to 2^n-1.
  if (ConstBECount && ConstBECount->getValue()->isMinusOne()) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loops with trip count greater than the "
                    "IV range currently not supported.\n");
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
  auto BrInst = dyn_cast<BranchInst>(LatchBB->getTerminator());

  if (!BrInst) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Non-branch instructions in loop latch "
                    "currently not supported.\n");
    return false;
  }

  if (BrInst->isUnconditional()) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Unconditional branch instructions in "
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

  // Check whether the loop contains irreducible CFG before calling
  // findIVDefInHeader() otherwise it may loop infinitely.
  // We skip the bblock check for function region mode as it is done at the
  // function level by the caller.
  if (!IsFunctionRegionMode && !areBBlocksGenerable(Lp)) {
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

  // We skip cost model throttling for function level region.
  if (!IsFunctionRegionMode &&
      shouldThrottleLoop(Lp, isa<SCEVCouldNotCompute>(BECount))) {
    return false;
  }

  return true;
}

bool HIRRegionIdentification::isSIMDDirective(const Instruction *Inst,
                                              bool BeginDir) {
  auto IntrinInst = dyn_cast<IntrinsicInst>(Inst);

  if (!IntrinInst) {
    return false;
  }

  if (!vpo::VPOAnalysisUtils::isIntelDirective(IntrinInst->getIntrinsicID())) {
    return false;
  }

  StringRef DirStr = vpo::VPOAnalysisUtils::getDirectiveMetadataString(
      const_cast<IntrinsicInst *>(IntrinInst));

  int DirID = vpo::VPOAnalysisUtils::getDirectiveID(DirStr);

  return BeginDir ? (DirID == DIR_OMP_SIMD) : (DirID == DIR_OMP_END_SIMD);
}

bool HIRRegionIdentification::containsSIMDDirective(const BasicBlock *BB,
                                                    bool BeginDir) {
  for (auto &Inst : *BB) {
    if (isSIMDDirective(&Inst, BeginDir)) {
      return true;
    }
  }

  return false;
}

BasicBlock *HIRRegionIdentification::findSIMDDirective(BasicBlock *BB,
                                                       bool BeginDir) {

  for (; BB != nullptr;) {
    if (containsSIMDDirective(BB, BeginDir)) {
      return BB;
    }
    BB = BeginDir ? BB->getSinglePredecessor() : BB->getSingleSuccessor();
  }

  return nullptr;
}

void HIRRegionIdentification::addBBlocks(
    const BasicBlock *BeginBB, const BasicBlock *EndBB,
    IRRegion::RegionBBlocksTy &RegBBlocks) const {

  for (auto TempBB = BeginBB;; TempBB = TempBB->getSingleSuccessor()) {
    RegBBlocks.push_back(TempBB);

    if (TempBB == EndBB) {
      break;
    }
  }
}

bool HIRRegionIdentification::isSIMDLoop(const Loop &Lp,
                                         IRRegion::RegionBBlocksTy *RegBBlocks,
                                         BasicBlock **RegEntryBB,
                                         BasicBlock **RegExitBB) const {

  BasicBlock *ExitBB = Lp.getExitBlock();

  if (!ExitBB) {
    return false;
  }

  BasicBlock *PreheaderBB = Lp.getLoopPreheader();
  BasicBlock *BeginBB = findSIMDDirective(PreheaderBB, true);

  if (!BeginBB) {
    return false;
  }

  BasicBlock *EndBB = findSIMDDirective(ExitBB, false);

  assert(EndBB && "Could not find SIMD END Directive!");

  if (RegBBlocks) {
    addBBlocks(BeginBB, PreheaderBB, *RegBBlocks);
    addBBlocks(ExitBB, EndBB, *RegBBlocks);
  }

  if (RegEntryBB) {
    *RegEntryBB = BeginBB;
  }

  if (RegExitBB) {
    *RegExitBB = EndBB;
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

  BasicBlock *EntryBB = nullptr, *ExitBB = nullptr;

  if (!isSIMDLoop(Lp, &BBlocks, &EntryBB, &ExitBB)) {
    EntryBB = Lp.getHeader();
  }

  IRRegions.emplace_back(EntryBB, BBlocks);

  if (ExitBB) {
    IRRegions.back().setExitBBlock(ExitBB);
  }

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
  if (Generable && !isSelfGenerable(Lp, ++(*LoopnestDepth), false)) {
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

void HIRRegionIdentification::createFunctionLevelRegion(Function &Func) {
  if (RegionNumThreshold && (RegionCount == RegionNumThreshold)) {
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Region throttled due to region number "
                    "threshold.\n");
    return;
  }

  IRRegion::RegionBBlocksTy BBlocks;

  for (auto BBIt = ++Func.begin(), E = Func.end(); BBIt != E; ++BBIt) {
    BBlocks.push_back(&*BBIt);
  }

  IRRegions.emplace_back(&Func.getEntryBlock(), BBlocks, true);

  RegionCount++;
}

bool HIRRegionIdentification::areBBlocksGenerable(Function &Func) const {

  for (auto BBIt = ++Func.begin(), E = Func.end(); BBIt != E; ++BBIt) {
    if (!isGenerable(&*BBIt)) {
      return false;
    }

    if (containsCycle(&*BBIt, nullptr)) {
      DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Irreducible CFG not supported.\n");
      return false;
    }
  }

  return true;
}

bool HIRRegionIdentification::canFormFunctionLevelRegion(Function &Func) {
  // Entry bblock is the first bblock of the region. We do not include it inside
  // the region because the dummy instructions created by HIR transformations
  // are inserted in the entry bblock. Our function level region will start from
  // the terminator instruction of the entry bblock. This is to maintain the
  // "single entry" property of the region.

  if (!areBBlocksGenerable(Func)) {
    return false;
  }

  SmallVector<Loop *, 4> AllLoops = LI->getLoopsInPreorder();

  for (auto Lp : AllLoops) {
    if (!isSelfGenerable(*Lp, Lp->getLoopDepth(), true)) {
      return false;
    }
  }

  return true;
}

bool HIRRegionIdentification::runOnFunction(Function &Func) {
  if (Func.hasFnAttribute(Attribute::OptimizeNone)) {
    return false;
  }

  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  PDT = &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();

  if (CreateFunctionLevelRegion) {
    if (canFormFunctionLevelRegion(Func)) {
      createFunctionLevelRegion(Func);
    }
  } else {
    formRegions();
  }

  return false;
}

void HIRRegionIdentification::releaseMemory() { IRRegions.clear(); }

void HIRRegionIdentification::print(raw_ostream &OS, const Module *M) const {

  for (auto I = IRRegions.begin(), E = IRRegions.end(); I != E; ++I) {
    OS << "\nRegion " << I - IRRegions.begin() + 1 << "\n";
    I->print(OS, 3);
    OS << "\n";
  }
}

void HIRRegionIdentification::verifyAnalysis() const {
  /// TODO: implement later
}
