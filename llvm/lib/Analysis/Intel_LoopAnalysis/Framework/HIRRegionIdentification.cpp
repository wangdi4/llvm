//===- HIRRegionIdentification.cpp - Identifies HIR Regions ---------------===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
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

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/Statistic.h"

#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_OptReport/LoopOptReport.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/IntrinsicInst.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRRegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_XmainOptLevelPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/TargetLibraryInfo.h"

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

static cl::opt<bool> DisableFusionRegions(
    "disable-hir-create-fusion-regions", cl::init(true), cl::Hidden,
    cl::desc("Disable HIR to create regions for multiple loops"
             "suitable for loop fusion"));

STATISTIC(RegionCount, "Number of regions created");

AnalysisKey HIRRegionIdentificationAnalysis::Key;

HIRRegionIdentification
HIRRegionIdentificationAnalysis::run(Function &F, FunctionAnalysisManager &AM) {
  // All the real work is done in the constructor for the
  // HIRRegionIdentification.
  return HIRRegionIdentification(
      F, AM.getResult<LoopAnalysis>(F), AM.getResult<DominatorTreeAnalysis>(F),
      AM.getResult<PostDominatorTreeAnalysis>(F),
      AM.getResult<ScalarEvolutionAnalysis>(F),
      AM.getResult<TargetLibraryAnalysis>(F),
      AM.getResult<XmainOptLevelAnalysis>(F).getOptLevel());
}

INITIALIZE_PASS_BEGIN(HIRRegionIdentificationWrapperPass,
                      "hir-region-identification", "HIR Region Identification",
                      false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(XmainOptLevelWrapperPass)
INITIALIZE_PASS_END(HIRRegionIdentificationWrapperPass,
                    "hir-region-identification", "HIR Region Identification",
                    false, true)

char HIRRegionIdentificationWrapperPass::ID = 0;

FunctionPass *llvm::createHIRRegionIdentificationWrapperPass() {
  return new HIRRegionIdentificationWrapperPass();
}

bool HIRRegionIdentificationWrapperPass::runOnFunction(Function &F) {
  // All the real work is done in the constructor for the
  // HIRRegionIdentification.
  RI.reset(new HIRRegionIdentification(
      F, getAnalysis<LoopInfoWrapperPass>().getLoopInfo(),
      getAnalysis<DominatorTreeWrapperPass>().getDomTree(),
      getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree(),
      getAnalysis<ScalarEvolutionWrapperPass>().getSE(),
      getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(),
      getAnalysis<XmainOptLevelWrapperPass>().getOptLevel()));

  return true;
}

void HIRRegionIdentificationWrapperPass::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<PostDominatorTreeWrapperPass>();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<ScalarEvolutionWrapperPass>();
  AU.addRequiredTransitive<TargetLibraryInfoWrapperPass>();
  AU.addRequiredTransitive<XmainOptLevelWrapperPass>();
}

HIRRegionIdentificationWrapperPass::HIRRegionIdentificationWrapperPass()
    : FunctionPass(ID) {
  initializeHIRRegionIdentificationWrapperPassPass(
      *PassRegistry::getPassRegistry());
}

/// Returns true if this bblock contains simd begin/end directive. \p BeginDir
/// flag indicates whether to look for begin or end directive.
static bool isSIMDOrParDirective(const Instruction *Inst, bool BeginDir) {
  auto IntrinInst = dyn_cast<IntrinsicInst>(Inst);

  if (!IntrinInst) {
    return false;
  }

  // TODO: Replace old simd directives by new region entry directives once VPO
  // support is added.
  if (vpo::VPOAnalysisUtils::isIntelDirective(IntrinInst->getIntrinsicID())) {
    StringRef DirStr = vpo::VPOAnalysisUtils::getDirectiveMetadataString(
        const_cast<IntrinsicInst *>(IntrinInst));

    int DirID = vpo::VPOAnalysisUtils::getDirectiveID(DirStr);

    return BeginDir ? (DirID == DIR_OMP_SIMD) : (DirID == DIR_OMP_END_SIMD);

  } else if (IntrinInst->hasOperandBundles()) {
    StringRef TagName = IntrinInst->getOperandBundleAt(0).getTagName();

    return BeginDir ? TagName.equals("DIR.OMP.PARALLEL.LOOP")
                    : TagName.equals("DIR.OMP.END.PARALLEL.LOOP");
  }

  return false;
}

/// Returns true if this bblock contains simd begin/end directive. \p BeginDir
/// flag indicates whether to look for begin or end directive.
static bool containsSIMDOrParDirective(const BasicBlock *BB, bool BeginDir) {
  for (auto &Inst : *BB) {
    if (isSIMDOrParDirective(&Inst, BeginDir)) {
      return true;
    }
  }

  return false;
}

/// Traces a chain of single predecessor/successor bblocks starting from \p BB
/// and looks for simd begin/end directive. Returns the bblock containing the
/// directive.
static BasicBlock *findSIMDOrParDirective(BasicBlock *BB, bool BeginDir) {

  for (; BB != nullptr;) {
    if (containsSIMDOrParDirective(BB, BeginDir)) {
      return BB;
    }
    BB = BeginDir ? BB->getSinglePredecessor() : BB->getSingleSuccessor();
  }

  return nullptr;
}

/// Inserts chain of bblocks from BeginBB to EndBB inclusive, to RegBBlocks.
static void addBBlocks(const BasicBlock *BeginBB, const BasicBlock *EndBB,
                       bool ShouldWalkPredecessors,
                       IRRegion::RegionBBlocksTy &RegBBlocks) {

  for (auto TempBB = BeginBB;; TempBB = ShouldWalkPredecessors
                                            ? TempBB->getSinglePredecessor()
                                            : TempBB->getSingleSuccessor()) {
    RegBBlocks.push_back(TempBB);

    if (TempBB == EndBB) {
      break;
    }
  }
}

/// Returns true if Lp is a SIMD or parallel loop. If RegBBlocks is non-null, it
/// adds simd loop predecess/successor bblocks to it. Entry/Exit bblocks for the
/// simd loop region are returned via \p EntryBB and \p ExitBB.
static bool isSIMDOrParLoop(const Loop &Lp,
                            IRRegion::RegionBBlocksTy *RegBBlocks = nullptr,
                            BasicBlock **RegEntryBB = nullptr,
                            BasicBlock **RegExitBB = nullptr) {

  BasicBlock *ExitBB = Lp.getExitBlock();

  // TODO: Multi-exit SIMD loops
  if (!ExitBB) {
    return false;
  }

  BasicBlock *PreheaderBB = Lp.getLoopPreheader();
  BasicBlock *BeginBB = findSIMDOrParDirective(PreheaderBB, true);

  if (!BeginBB) {
    return false;
  }

  BasicBlock *EndBB = findSIMDOrParDirective(ExitBB, false);

  assert(EndBB && "Could not find SIMD END Directive!");

  if (RegBBlocks) {
    addBBlocks(PreheaderBB, BeginBB, true, *RegBBlocks);
    addBBlocks(ExitBB, EndBB, false, *RegBBlocks);
  }

  if (RegEntryBB) {
    *RegEntryBB = BeginBB;
  }

  if (RegExitBB) {
    *RegExitBB = EndBB;
  }

  return true;
}

void HIRRegionIdentification::computeLoopSpansForFusion(
    const SmallVectorImpl<const Loop *> &Loops,
    SmallVectorImpl<LoopSpanTy> &Spans) {
  // TODO: Skip loops with Unknown Memory Access

  // Form the multiple top-level loop regions for fusion.
  // Consider only DO loops.
  unsigned Span = 0;
  LoopSpanTy CurSpan;
  for (int I = 0, E = Loops.size(); I < E;
       I += Span, Spans.push_back(CurSpan)) {
    Span = 1;
    CurSpan = {};

    auto *Lp1 = Loops[I];
    CurSpan.first.push_back(Lp1);

    if (!Lp1) {
      continue;
    }

    const Loop *Lp1Parent = Lp1->getParentLoop();

    if (isSIMDOrParLoop(*Lp1)) {
      continue;
    }

    auto *Lp1EB = Lp1->getExitBlock();
    if (!Lp1EB) {
      continue;
    }

    const SCEV *Lp1TC = SE.getBackedgeTakenCount(Lp1);
    if (isa<SCEVCouldNotCompute>(Lp1TC)) {
      continue;
    }

    for (int J = I + 1; J < E; ++J) {
      auto *Lp2 = Loops[J];
      if (!Lp2) {
        break;
      }

      if (Lp2->getParentLoop() != Lp1Parent) {
        break;
      }

      if (isSIMDOrParLoop(*Lp2)) {
        break;
      }

      if (!Lp2->getExitBlock()) {
        break;
      }

      const SCEV *Lp2TC = SE.getBackedgeTakenCount(Lp2);
      if (isa<SCEVCouldNotCompute>(Lp2TC)) {
        break;
      }

      if (Lp1TC->getType() != Lp2TC->getType()) {
        break;
      }

      const auto *Diff = dyn_cast<SCEVConstant>(SE.getMinusSCEV(Lp1TC, Lp2TC));
      if (!Diff || Diff->getAPInt().abs().ugt(MaxFusionTripCountDiff)) {
        break;
      }

      if (!DT.dominates(Lp1EB, Lp2->getHeader())) {
        break;
      }

      if (!PDT.dominates(Lp2->getHeader(), Lp1EB)) {
        break;
      }

      // Collect intermediate BBs between last loop in the current span and Lp2.
      LoopSpanTy::second_type IntermediateBBs;
      if (!collectIntermediateBBs(CurSpan.first.back(), Lp2, IntermediateBBs)) {
        break;
      }

      Span++;
      CurSpan.first.push_back(Lp2);
      CurSpan.second.insert(IntermediateBBs.begin(), IntermediateBBs.end());
    }
  }
}

namespace {

template <typename NodeRef, unsigned SmallSize>
struct IBBIteratorSet : public SmallPtrSet<NodeRef, SmallSize> {
  using BaseSet = SmallPtrSet<NodeRef, SmallSize>;
  using iterator = typename BaseSet::iterator;

  SmallPtrSetImpl<NodeRef> &BBs;
  bool CycleFound;

  IBBIteratorSet(SmallPtrSetImpl<NodeRef> &BBs) : BBs(BBs), CycleFound(false) {}

  std::pair<iterator, bool> insert(NodeRef N) {
    auto Pair = BaseSet::insert(N);
    if (!Pair.second) {
      CycleFound = true;
    }

    return Pair;
  }

  void completed(NodeRef N) {
    if (BBs.count(N)) {
      // Have to mark selected BBs as non-visited to make iterator possible to
      // visit them again using different path.
      BaseSet::erase(N);
    }
  }
};

} // namespace

bool HIRRegionIdentification::collectIntermediateBBs(
    const Loop *Loop1, const Loop *Loop2,
    SmallPtrSetImpl<const BasicBlock *> &BBs) {

  BasicBlock *ExitBB = Loop1->getExitBlock();
  SmallPtrSet<const Loop *, 4> LoopsSet{Loop1, Loop2};

  // Explore a depth-first paths from a loop exit blocks to loop headers.
  IBBIteratorSet<const BasicBlock *, 8> Visited(BBs);
  for (auto BBI = df_ext_begin(ExitBB, Visited),
            BBE = df_ext_end(ExitBB, Visited);
       BBI != BBE;) {

    if (Visited.CycleFound) {
      // Irreducible CFG detected.
      return false;
    }

    // Save the path if reached the loop header or a block that proven to
    // reach the loop header.
    if (BBs.count(*BBI) || (*BBI == Loop2->getHeader())) {
      // Do not include last BB in the path.
      for (int N = BBI.getPathLength() - 2; N >= 0; --N) {
        if (!BBs.insert(BBI.getPath(N)).second) {
          break;
        }
      }

      Visited.completed(*BBI);
      BBI.skipChildren();
      continue;
    }

    ++BBI;
  }

  if (BBs.size() > MaxIntermediateBBsForFusion) {
    return false;
  }

  for (auto *BB : BBs) {
    // Check that all BBs are generable.
    if (!isGenerable(BB, nullptr)) {
      return false;
    }

    for (const Instruction &Inst : *BB) {
      // Check that there is no unknown memory access calls
      if (isa<CallInst>(Inst) &&
          HLInst::hasUnknownMemoryAccess(cast<CallInst>(&Inst))) {
        return false;
      }

      // Check that there are no region live-outs within BBs.
      if (!std::all_of(Inst.user_begin(), Inst.user_end(),
                       [this, &BBs, &LoopsSet](const Value *V) {
                         auto *UseBB = cast<Instruction>(V)->getParent();
                         return BBs.count(UseBB) ||
                                LoopsSet.count(LI.getLoopFor(UseBB));
                       })) {
        return false;
      }
    }

    // Check for multiple entries to region. BB predecessors should all be
    // either a part of BBs or LoopsSet.
    assert(std::all_of(pred_begin(BB), pred_end(BB),
                       [this, &BBs, &LoopsSet](const BasicBlock *PredBB) {
                         return BBs.count(PredBB) ||
                                LoopsSet.count(LI.getLoopFor(PredBB));
                       }) &&
           "Found unexpected region entry");
  }

  return true;
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

  auto Lp = LI.getLoopFor(ParentBB);

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
        LLVM_DEBUG(
            dbgs()
            << "LOOPOPT_OPTREPORT: vector types currently not supported.\n");
        return false;
      }
      Ty = SeqTy->getElementType();
    } else {
      Ty = Ty->getPointerElementType();
    }
  }

  auto IntType = dyn_cast<IntegerType>(Ty);
  // Integer type greater than 64 bits not supported.This is mainly to throttle
  // 128 bit integers.
  if (IntType && (IntType->getPrimitiveSizeInBits() > 64)) {
    LLVM_DEBUG(
        dbgs() << "LOOPOPT_OPTREPORT: integer types greater than 64 bits "
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
      if (!Lp.contains(LI.getLoopFor(OpInst->getParent()))) {
        continue;
      }

      // Skip backedges.
      // This can happen for outer unknown loops.
      if (DT.dominates(Inst, OpInst)) {
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

  const bool IsInnermostLoop;
  const bool IsUnknownLoop;
  bool IsSmallTripLoop;
  bool IsProfitable;

  const unsigned OptLevel;
  unsigned InstCount;             // Approximates number of instructions in HIR.
  unsigned UnstructuredJumpCount; // Approximates goto/label counts in HIR.
  unsigned IfCount;               // Approximates number of ifs in HIR.

  // TODO: use different values for O2/O3.
  const unsigned MaxInstThreshold = 200;
  const unsigned MaxIfThreshold = 7;
  const unsigned O2MaxIfNestThreshold = 2;
  const unsigned O3MaxIfNestThreshold = 3;
  const unsigned SmallTripThreshold = 16;

public:
  CostModelAnalyzer(const HIRRegionIdentification &RI, const Loop &Lp,
                    const SCEV *BECount);

  bool isProfitable() const { return IsProfitable; }

  void analyze();

  bool visitBasicBlock(const BasicBlock &BB);
  bool visitInstruction(const Instruction &Inst);
  bool visitLoadInst(const LoadInst &LI);
  bool visitStoreInst(const StoreInst &SI);
  bool visitCallInst(const CallInst &CI);
  bool visitBranchInst(const BranchInst &BI);
};

HIRRegionIdentification::CostModelAnalyzer::CostModelAnalyzer(
    const HIRRegionIdentification &RI, const Loop &Lp, const SCEV *BECount)
    : RI(RI), Lp(Lp), IsInnermostLoop(Lp.empty()),
      IsUnknownLoop(isa<SCEVCouldNotCompute>(BECount)), IsProfitable(true),
      OptLevel(RI.OptLevel), InstCount(0), UnstructuredJumpCount(0),
      IfCount(0) {

  HeaderDomNode = RI.DT.getNode(Lp.getHeader());

  auto ConstBECount = dyn_cast<SCEVConstant>(BECount);
  IsSmallTripLoop =
      (ConstBECount &&
       (ConstBECount->getValue()->getZExtValue() <= SmallTripThreshold));
}

void HIRRegionIdentification::CostModelAnalyzer::analyze() {

  // SIMD loops should not be throttled.
  if (isSIMDOrParLoop(Lp)) {
    IsProfitable = true;
    return;
  }

  // Only allow innermost multi-exit loops for now.
  if (!Lp.getExitingBlock() && !Lp.empty()) {
    LLVM_DEBUG(
        dbgs() << "LOOPOPT_OPTREPORT: outer multi-exit loop throttled for "
                  "compile time reasons.\n");
    IsProfitable = false;
    return;
  }

  // Only handle standalone single bblock unknown loops at O2. We allow bigger
  // standalone innermost loops at O3.
  // We don't do much for outer unknown loops except prefetching which isn't
  // ready yet. Innermost unknown loops embedded inside other loops are
  // throttled for compile time reasons.
  if (IsUnknownLoop && (((OptLevel < 3) && (Lp.getNumBlocks() != 1)) ||
                        (Lp.getLoopDepth() != 1))) {
    LLVM_DEBUG(
        dbgs() << "LOOPOPT_OPTREPORT: unknown loop throttled for compile "
                  "time reasons.\n");
    IsProfitable = false;
    return;
  }

  for (auto BB = Lp.block_begin(), E = Lp.block_end(); BB != E; ++BB) {

    // Skip bblocks which belong to inner loops.
    if (!IsInnermostLoop && (RI.LI.getLoopFor(*BB) != &Lp)) {
      continue;
    }

    if (!visitBasicBlock(**BB)) {
      IsProfitable = false;
      break;
    }
  }
}

bool HIRRegionIdentification::CostModelAnalyzer::visitBasicBlock(
    const BasicBlock &BB) {

  auto BBInstCount = BB.size();

  // Bail out early instead of analyzing each individual instruction.
  if ((BBInstCount + InstCount) > MaxInstThreshold) {
    LLVM_DEBUG(
        dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of too "
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
    if (RI.SE.isSCEVable(Inst.getType())) {
      auto SC = RI.SE.getSCEV(const_cast<Instruction *>(&Inst));
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
    LLVM_DEBUG(
        dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of too "
                  "many statements.\n");
  }

  return Ret;
}

bool HIRRegionIdentification::CostModelAnalyzer::visitLoadInst(
    const LoadInst &LI) {
  if (LI.isVolatile()) {
    LLVM_DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of "
                         "volatile load.\n");
    return false;
  }

  return visitInstruction(static_cast<const Instruction &>(LI));
}

bool HIRRegionIdentification::CostModelAnalyzer::visitStoreInst(
    const StoreInst &SI) {
  if (SI.isVolatile()) {
    LLVM_DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of "
                         "volatile store.\n");
    return false;
  }

  return visitInstruction(static_cast<const Instruction &>(SI));
}

bool HIRRegionIdentification::CostModelAnalyzer::visitCallInst(
    const CallInst &CI) {

  // Allow user calls in small trip innermost loops so they can be completely
  // unrolled.
  // Also allow them in innermost unknown loops at O3 and above. They may be
  // candidates for predicate optimization.
  if (!IsInnermostLoop ||
      (!IsSmallTripLoop && ((OptLevel < 3) || !IsUnknownLoop))) {
    if (!isa<IntrinsicInst>(CI)) {
      auto Func = CI.getCalledFunction();

      if (!Func || !RI.TLI.isFunctionVectorizable(Func->getName())) {
        LLVM_DEBUG(
            dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of "
                      "user calls.\n");
        return false;
      }
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
    LLVM_DEBUG(
        dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of too "
                  "many ifs.\n");
    return false;
  }

  unsigned IfNestCount = 0;
  auto DomNode = RI.DT.getNode(const_cast<BasicBlock *>(ParentBB));

  while (DomNode != HeaderDomNode) {
    assert(DomNode && "Dominator tree node of a loop bblock is null!");

    auto DomBlock = DomNode->getBlock();
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
    // Ignore dominators whose terminator is not a branch. It can be a switch,
    // for example.
    if (DomBlock->getSinglePredecessor() &&
        isa<BranchInst>(DomBlock->getTerminator())) {
      ++IfNestCount;
    }

    DomNode = DomNode->getIDom();
  }

  // Increase thresholds for small trip innermost loops so that we can unroll
  // them.
  bool UseO3Thresholds = (OptLevel > 2) || (IsInnermostLoop && IsSmallTripLoop);

  unsigned IfNestThreshold =
      (UseO3Thresholds ? O3MaxIfNestThreshold : O2MaxIfNestThreshold);

  if (UseO3Thresholds && IsInnermostLoop) {
    // Allow 1 more If nesting for O3 innermost loops.
    ++IfNestThreshold;
  }

  // Add 1 to include reaching header node.
  if ((IfNestCount + 1) > IfNestThreshold) {
    LLVM_DEBUG(
        dbgs() << "LOOPOPT_OPTREPORT: Loop throttled due to presence of too "
                  "many nested ifs.\n");
    return false;
  }

  // Skip goto check for multi-exit loops.
  if (!Lp.getExitingBlock()) {
    return true;
  }

  auto Succ0 = BI.getSuccessor(0);
  auto Succ1 = BI.getSuccessor(1);

  // Within the same loop, conditional branches not dominating its successor
  // and the successor not post-dominating the branch indicates presence of a
  // goto in HLLoop.
  if ((!RI.DT.dominates(ParentBB, Succ0) &&
       !RI.PDT.dominates(Succ0, ParentBB)) ||
      (!RI.DT.dominates(ParentBB, Succ1) &&
       !RI.PDT.dominates(Succ1, ParentBB))) {
    LLVM_DEBUG(
        dbgs()
        << "LOOPOPT_OPTREPORT: Loop throttled due to presence of goto.\n");
    return false;
  }

  return true;
}

bool HIRRegionIdentification::shouldThrottleLoop(const Loop &Lp,
                                                 const SCEV *BECount) const {

  if (!CostModelThrottling) {
    return false;
  }

  CostModelAnalyzer CMA(*this, Lp, BECount);
  CMA.analyze();

  return !CMA.isProfitable();
}

static MDString *getStringMetadata(MDNode *Node) {
  assert(Node->getNumOperands() > 0 &&
         "metadata should have at least one operand!");

  return dyn_cast<MDString>(Node->getOperand(0));
}

static bool isUnrollMetadata(MDNode *Node) {
  MDString *Str = getStringMetadata(Node);

  return Str && Str->getString().startswith("llvm.loop.unroll");
}

static bool isDistributeMetadata(MDNode *Node) {
  MDString *Str = getStringMetadata(Node);

  return Str && Str->getString().equals("llvm.loop.distribute.enable");
}

static bool isVectorizeMetadata(MDNode *Node) {
  MDString *Str = getStringMetadata(Node);

  return Str && Str->getString().startswith("llvm.loop.vectorize");
}

static bool isDebugMetadata(MDNode *Node) {
  return isa<DILocation>(Node) || isa<DINode>(Node);
}

static bool isLoopCountMetadata(MDNode *Node) {
  MDString *Str = getStringMetadata(Node);

  return Str && Str->getString().startswith("llvm.loop.intel.loopcount");
}

static bool isSupportedMetadata(MDNode *Node) {

  if (isDebugMetadata(Node) || isUnrollMetadata(Node) ||
      isDistributeMetadata(Node) || isVectorizeMetadata(Node) ||
      isLoopCountMetadata(Node) || LoopOptReport::isOptReportMetadata(Node)) {
    return true;
  }

  unsigned Ops = Node->getNumOperands();

  for (unsigned I = 0; I < Ops; ++I) {
    MDNode *OpNode = dyn_cast<MDNode>(Node->getOperand(I));
    if (OpNode == Node) {
      continue;
    }

    if (!OpNode || !isSupportedMetadata(OpNode)) {
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
    if (!DT.dominates(BB, PredBB) &&
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

namespace {
// Keeps track of BasicBlock on stack.
// Finds any back edge during DFS, Loop's back edges and edges going outside
// Loop are ignored.
//
// Essentially, checks that a Loop has irreducible CFG.
//
// See also Intel_VPlan/VPlanDriver.cpp isIrreducibleCFG()
class DFLoopTraverse
    : public SmallPtrSet<typename GraphTraits<const BasicBlock *>::NodeRef,
                         32> {
public:
  typedef typename GraphTraits<const BasicBlock *>::NodeRef NodeRef;
  typedef SmallPtrSet<NodeRef, 32> Container;

  DFLoopTraverse(const LoopInfo *LI = nullptr, const Loop *Lp = nullptr)
      : LI(LI), Lp(Lp) {}

  bool isOutgoing(NodeRef ToBBlock) const {
    return Lp && !Lp->contains(ToBBlock);
  }

  // See LoopBase<>::getNumBackEdges
  bool isLoopBackedge(Optional<NodeRef> From, NodeRef To) const {
    if (!From) {
      return false;
    }
    auto ToLoop = LI->getLoopFor(To);
    return ToLoop && ToLoop->getHeader() == To &&
           ToLoop->contains(From.getValue());
  }

private:
  DFLoopTraverse(const DFLoopTraverse &that) = delete;
  DFLoopTraverse(const DFLoopTraverse &&that) = delete;
  DFLoopTraverse &operator=(const DFLoopTraverse &) = delete;
  DFLoopTraverse &operator=(const DFLoopTraverse &&) = delete;

  const LoopInfo *LI;
  const Loop *Lp;
};
} // namespace

namespace llvm {
// Specialization of po_iterator_storage to implement
// finishPostorder/insertEdge.
//
// These methods compute CycleSeen using info from DFLoopTraverse
template <> class po_iterator_storage<DFLoopTraverse, false> {
public:
  typedef typename GraphTraits<const BasicBlock *>::NodeRef NodeRef;

  po_iterator_storage(DFLoopTraverse &DFLoopInfo)
      : CycleSeen(false), DFLoopInfo(DFLoopInfo) {}

  // Return true if edge destination should be visited.
  // Computes CycleSeen
  bool insertEdge(Optional<NodeRef> From, NodeRef To) {
    if (CycleSeen || DFLoopInfo.isOutgoing(To) ||
        DFLoopInfo.isLoopBackedge(From, To)) {
      return false;
    }

    // Seen first time ever
    if (Visited.insert(To).second) {
      // keep in sync with po_iterator's stack
      auto res = DFLoopInfo.insert(To);
      assert(res.second && "DFLoopInfo and DF traversal are out of sync");
      (void)res;
      return true;
    }

    if (DFLoopInfo.find(To) != DFLoopInfo.end()) {
      CycleSeen = true;
    }

    return false;
  }

  // Called after all children of BB have been visited.
  void finishPostorder(NodeRef BB) {
    // Keep in sync with po_iterator's stack.
    DFLoopInfo.erase(BB);
  }

  bool getFoundCycle() const { return CycleSeen; }

private:
  bool CycleSeen;
  // set of basic block seen in pre-order.
  DFLoopTraverse::Container Visited;
  // set of basic block in po_iterator's stack.
  // need quick check 'if basic block is on stack of depth-first traversal'
  DFLoopTraverse &DFLoopInfo;
};
} // namespace llvm

namespace {
bool isIrreducible(const LoopInfo *LI, const Loop *Lp,
                   const BasicBlock *EntryBlock = nullptr) {

  assert((Lp == nullptr) == (EntryBlock != nullptr) &&
         "Exactly one parameter should be non-null");

  typedef po_iterator<const BasicBlock *, DFLoopTraverse> Iter;

  DFLoopTraverse dfs(LI, Lp);
  auto Start = Lp ? Lp->getHeader() : EntryBlock;
  for (auto PoIter = Iter::begin(Start, dfs), PoEnd = Iter::end(Start, dfs);
       PoIter != PoEnd; ++PoIter) {
    if (PoIter.getFoundCycle()) {
      LLVM_DEBUG(
          dbgs() << "LOOPOPT_OPTREPORT: Irreducible CFG not supported.\n");
      return true;
    }
  }

  return false;
}
} // namespace

static bool hasUnsupportedOperandBundle(const CallInst *CI) {
  if (!CI->hasOperandBundles()) {
    return false;
  }

  OperandBundleUse BU = CI->getOperandBundleAt(0);

  StringRef TagName = BU.getTagName();

  return !TagName.equals("DIR.PRAGMA.DISTRIBUTE_POINT") &&
         !TagName.equals("DIR.PRAGMA.END.DISTRIBUTE_POINT") &&
         // TODO: can we switch to TagName.startwith("DIR.OMP") later?
         !TagName.equals("DIR.OMP.PARALLEL.LOOP") &&
         !TagName.equals("DIR.OMP.END.PARALLEL.LOOP");
}

bool HIRRegionIdentification::isGenerable(const BasicBlock *BB,
                                          const Loop *Lp) {
  auto FirstInst = BB->getFirstNonPHI();

  if (isa<LandingPadInst>(FirstInst) || isa<FuncletPadInst>(FirstInst)) {
    LLVM_DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Exception handling currently not "
                         "supported.\n");
    return false;
  }

  auto Term = BB->getTerminator();

  if (isa<IndirectBrInst>(Term)) {
    LLVM_DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Indirect branches currently not "
                         "supported.\n");
    return false;
  }

  if (isa<InvokeInst>(Term) || isa<ResumeInst>(Term) ||
      isa<CatchSwitchInst>(Term) || isa<CatchReturnInst>(Term) ||
      isa<CleanupReturnInst>(Term)) {
    LLVM_DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Exception handling currently not "
                         "supported.\n");
    return false;
  }

  if (Lp && !Lp->getExitingBlock()) {
    // If there are multiple switch successors that jump to the same bblock
    // outside the loop, we throttle this loop. This condition essentially means
    // that there are multiple edges between src and dest bblock. Currently, the
    // lveout handling mechanism in the framework assumes each goto represents a
    // unique (src, dest) bblock pair. To handle this case we will have to add
    // more information to goto's (like switch case value) and do some tracking
    // of values liveout for each case goto. This is pretty complicated and
    // hence being left as a TODO for now. It may be simpler to handle switches
    // where the same value is liveout from each edge but this is also being
    // left as a TODO for now.
    if (auto Switch = dyn_cast<SwitchInst>(Term)) {
      SmallPtrSet<BasicBlock *, 8> LoopExitBBs;

      for (unsigned I = 0, NumSuccs = Switch->getNumSuccessors(); I < NumSuccs;
           ++I) {
        auto SuccBB = Switch->getSuccessor(I);

        if (!Lp->contains(SuccBB)) {
          if (LoopExitBBs.count(SuccBB) && isa<PHINode>(SuccBB->begin())) {
            LLVM_DEBUG(dbgs()
                       << "LOOPOPT_OPTREPORT: Switch instruction with "
                          "multiple successors outside the loop currently "
                          "not supported.\n");
            return false;
          }
          LoopExitBBs.insert(SuccBB);
        }
      }
    }
  }

  // Skip the terminator instruction.
  for (auto Inst = BB->begin(), E = std::prev(BB->end()); Inst != E; ++Inst) {

    if (Inst->isAtomic()) {
      LLVM_DEBUG(
          dbgs() << "LOOPOPT_OPTREPORT: Atomic instructions are currently "
                    "not supported.\n");
      return false;
    }

    // TODO: think about HIR representation for
    // InsertValueInst/ExtractValueInst.
    if (isa<InsertValueInst>(Inst) || isa<ExtractValueInst>(Inst)) {
      LLVM_DEBUG(
          dbgs() << "LOOPOPT_OPTREPORT: InsertValueInst/ExtractValueInst "
                    "currently not supported.\n");
      return false;
    }

    if (Inst->getType()->isVectorTy()) {
      LLVM_DEBUG(
          dbgs()
          << "LOOPOPT_OPTREPORT: Vector types currently not supported.\n");
      return false;
    }

    if (auto CInst = dyn_cast<CallInst>(Inst)) {
      if (CInst->isInlineAsm()) {
        LLVM_DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Inline assembly currently not "
                             "supported.\n");
        return false;
      }

      if (hasUnsupportedOperandBundle(CInst)) {
        LLVM_DEBUG(
            dbgs() << "LOOPOPT_OPTREPORT: Unsupported operand bundle.\n");
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
    if (!IsInnermostLoop && LI.getLoopFor(*BB) != (&Lp)) {
      continue;
    }

    if (!isGenerable(*BB, &Lp)) {
      return false;
    }
  }

  if (isIrreducible(&LI, &Lp)) {
    return false;
  }

  return true;
}

const Loop *HIRRegionIdentification::getOutermostParentLoop(const Loop *Lp) {
  const Loop *ParLp, *TmpLp;

  ParLp = Lp;

  while ((TmpLp = ParLp->getParentLoop())) {
    ParLp = TmpLp;
  }

  return ParLp;
}

bool HIRRegionIdentification::isSelfGenerable(const Loop &Lp,
                                              unsigned LoopnestDepth,
                                              bool IsFunctionRegionMode) const {

  // At least one of this loop's subloops reach MaxLoopNestLevel so we cannot
  // generate this loop.
  if (LoopnestDepth > MaxLoopNestLevel) {
    LLVM_DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loopnest is more than "
                      << MaxLoopNestLevel << " deep.\n");
    return false;
  }

  // Loop is not in a handleable form.
  if (!Lp.isLoopSimplifyForm()) {
    LLVM_DEBUG(
        dbgs() << "LOOPOPT_OPTREPORT: Loop structure is not handleable.\n");
    return false;
  }

  // Skip loops with unsupported pragmas.
  MDNode *LoopID = Lp.getLoopID();
  if (!DisablePragmaBailOut && !isSIMDOrParLoop(Lp) && LoopID &&
      !isSupportedMetadata(LoopID)) {
    LLVM_DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loops has unsupported pragma.\n");
    return false;
  }

  auto LatchBB = Lp.getLoopLatch();

  // We cannot build lexical links if dominator/post-dominator info is absent.
  // This can be due to unreachable/infinite loops.
  if (!DT.getNode(LatchBB) || !PDT.getNode(LatchBB)) {
    LLVM_DEBUG(
        dbgs()
        << "LOOPOPT_OPTREPORT: Unreachable/Infinite loops not supported.\n");
    return false;
  }

  // Check that the loop backedge is a conditional branch.
  auto BrInst = dyn_cast<BranchInst>(LatchBB->getTerminator());

  if (!BrInst) {
    LLVM_DEBUG(
        dbgs() << "LOOPOPT_OPTREPORT: Non-branch instructions in loop latch "
                  "currently not supported.\n");
    return false;
  }

  if (BrInst->isUnconditional()) {
    LLVM_DEBUG(
        dbgs() << "LOOPOPT_OPTREPORT: Unconditional branch instructions in "
                  "loop latch currently not supported.\n");
    return false;
  }

  const Value *LatchVal = BrInst->getCondition();

  auto LatchCmpInst = dyn_cast<Instruction>(LatchVal);

  if (!LatchCmpInst) {
    LLVM_DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Non-instruction latch condition "
                         "currently not supported.\n");
    return false;
  }

  // Use the outermost LLVM loop to evaluate the trip count as we do not know
  // the outermost HIR parent loop. This is okay for the initial analysis. If
  // we are not able to compute trip count of the loop after suppressing some
  // parent loops, the loop will be handled as an unknown loop.
  auto BECount =
      SE.getBackedgeTakenCountForHIR(&Lp, getOutermostParentLoop(&Lp));

  auto ConstBECount = dyn_cast<SCEVConstant>(BECount);

  // This represents a trip count of 2^n while we can only handle a trip count
  // up to 2^n-1.
  if (ConstBECount && ConstBECount->getValue()->isMinusOne()) {
    LLVM_DEBUG(
        dbgs() << "LOOPOPT_OPTREPORT: Loops with trip count greater than the "
                  "IV range currently not supported.\n");
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
    LLVM_DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Could not find loop IV.\n");
    return false;
  }

  if (IVNode->getType()->getPrimitiveSizeInBits() == 1) {
    // The following loop with i1 type IV has a trip count of 2 which is
    // outside its range. This is a quirk of SSA. CG will generate an infinite
    // loop for this case if we let it through.
    // for.i:
    // %i.08.i = phi i1 [ true, %entry ], [ false, %for.i ]
    // br i1 %i.08.i, label %for.i, label %exit
    LLVM_DEBUG(
        dbgs() << "LOOPOPT_OPTREPORT: i1 type IV currently not handled.\n");
    return false;
  }

  // We skip cost model throttling for function level region.
  if (!IsFunctionRegionMode && shouldThrottleLoop(Lp, BECount)) {
    return false;
  }

  return true;
}

void HIRRegionIdentification::createRegion(
    const ArrayRef<const Loop *> &Loops,
    const SmallPtrSetImpl<const BasicBlock *> *IntermediateBlocks) {

  if (RegionNumThreshold && (RegionCount == RegionNumThreshold)) {
    LLVM_DEBUG(
        dbgs() << "LOOPOPT_OPTREPORT: Region throttled due to region number "
                  "threshold.\n");
    return;
  }

  IRRegion::RegionBBlocksTy BBlocks;

  if (IntermediateBlocks) {
    BBlocks.append(IntermediateBlocks->begin(), IntermediateBlocks->end());
  }

  BasicBlock *EntryBB = Loops.front()->getHeader();
  BasicBlock *ExitBB = nullptr;

  for (auto *Lp : Loops) {
    bool IsFirstLoop = (Lp == Loops.front());
    bool IsLastLoop = (Lp == Loops.back());

    isSIMDOrParLoop(*Lp, &BBlocks, IsFirstLoop ? &EntryBB : nullptr,
                    IsLastLoop ? &ExitBB : nullptr);

    BBlocks.append(Lp->getBlocks().begin(), Lp->getBlocks().end());
  }

  IRRegions.emplace_back(EntryBB, BBlocks);

  if (ExitBB) {
    IRRegions.back().setExitBBlock(ExitBB);
  }

  RegionCount++;
}

bool HIRRegionIdentification::isGenerableLoopnest(
    const Loop &Lp, unsigned &LoopnestDepth,
    SmallVectorImpl<const Loop *> &GenerableLoops) {
  SmallVector<const Loop *, 8> SubGenerableLoops;
  bool Generable = true;

  LoopnestDepth = 0;

  // Check which sub loops are generable.
  for (auto I = Lp.begin(), E = Lp.end(); I != E; ++I) {
    unsigned SubLoopnestDepth;

    if (isGenerableLoopnest(**I, SubLoopnestDepth, SubGenerableLoops)) {
      // Set maximum sub-loopnest depth
      LoopnestDepth = std::max(LoopnestDepth, SubLoopnestDepth);
    } else {
      Generable = false;
    }
  }

  // Check whether Lp is generable.
  if (Generable && !isSelfGenerable(Lp, ++LoopnestDepth, false)) {
    Generable = false;
  }

  if (Generable) {
    // Entire loopnest is generable. Add Lp in generable set.
    GenerableLoops.push_back(&Lp);
  } else {
    // Add sub loops of Lp in generable set.
    GenerableLoops.append(SubGenerableLoops.begin(), SubGenerableLoops.end());
  }

  return Generable;
}

void HIRRegionIdentification::formRegions() {
  SmallVector<const Loop *, 32> GenerableLoops;

  if (LI.empty()) {
    return;
  }

  // LoopInfo::iterator visits loops in reverse program order so we need to
  // use reverse_iterator here.
  for (LoopInfo::reverse_iterator I = LI.rbegin(), E = LI.rend(); I != E; ++I) {
    unsigned Depth;
    isGenerableLoopnest(**I, Depth, GenerableLoops);
  }

  if (DisableFusionRegions) {
    for (const Loop *Lp : GenerableLoops) {
      createRegion(Lp, nullptr);
    }

    return;
  }

  SmallVector<LoopSpanTy, 8> LoopSpans;
  computeLoopSpansForFusion(GenerableLoops, LoopSpans);

  for (auto &LoopSpan : LoopSpans) {
    // Create region for generable loop span.
    createRegion(LoopSpan.first, &LoopSpan.second);
  }
}

void HIRRegionIdentification::createFunctionLevelRegion(Function &Func) {
  if (RegionNumThreshold && (RegionCount == RegionNumThreshold)) {
    LLVM_DEBUG(
        dbgs() << "LOOPOPT_OPTREPORT: Region throttled due to region number "
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
    if (!isGenerable(&*BBIt, nullptr)) {
      return false;
    }
  }

  if (isIrreducible(&LI, nullptr, &Func.getEntryBlock())) {
    return false;
  }

  return true;
}

bool HIRRegionIdentification::canFormFunctionLevelRegion(Function &Func) {
  // Entry bblock is the first bblock of the region. We do not include it
  // inside the region because the dummy instructions created by HIR
  // transformations are inserted in the entry bblock. Our function level
  // region will start from the terminator instruction of the entry bblock.
  // This is to maintain the "single entry" property of the region.

  if (!areBBlocksGenerable(Func)) {
    return false;
  }

  SmallVector<Loop *, 16> AllLoops = LI.getLoopsInPreorder();

  for (auto Lp : AllLoops) {
    if (!isSelfGenerable(*Lp, Lp->getLoopDepth(), true)) {
      return false;
    }
  }

  return true;
}

bool HIRRegionIdentification::isLoopConcatenationCandidate(BasicBlock *BB) {
  // Check that-
  // 1) All loads either load an i8 type value or load an i32 type value from
  // the same alloca.
  // 2) All stores store an i32 type value and use the same alloca as the base
  // pointer.
  // 3) Rest of the instructions are all integer type.
  auto &Cnxt = BB->getContext();
  auto Int8Ty = Type::getInt8Ty(Cnxt);
  auto Int32Ty = Type::getInt32Ty(Cnxt);
  Value *Alloca = nullptr;
  unsigned NumAllocaLoads = 0, NumAllocaStores = 0;

  for (auto It = BB->begin(), E = --BB->end(); It != E; ++It) {
    auto &Inst = (*It);
    auto InstTy = Inst.getType();
    Value *Ptr = nullptr;

    if (auto LInst = dyn_cast<LoadInst>(&Inst)) {
      if (InstTy != Int8Ty) {
        if (InstTy != Int32Ty) {
          return false;
        }
        Ptr = LInst->getPointerOperand();
      }
      ++NumAllocaLoads;

    } else if (auto SInst = dyn_cast<StoreInst>(&Inst)) {
      if (SInst->getValueOperand()->getType() != Int32Ty) {
        return false;
      }
      Ptr = SInst->getPointerOperand();
      ++NumAllocaStores;

    } else if (!isa<PointerType>(InstTy) && !isa<IntegerType>(InstTy)) {
      return false;
    }

    if (Ptr) {
      auto GEP = dyn_cast<GetElementPtrInst>(Ptr);

      if (!GEP) {
        return false;
      }

      auto GEPPtr = GEP->getPointerOperand();

      if (Alloca) {
        if (GEPPtr != Alloca) {
          return false;
        }
      } else if (!isa<AllocaInst>(GEPPtr)) {
        return false;
      } else {
        Alloca = GEPPtr;
      }
    }
  }

  if ((NumAllocaLoads != 4) && (NumAllocaStores != 4)) {
    return false;
  }

  return true;
}

bool HIRRegionIdentification::isLoopConcatenationCandidate() const {
  // Restrict to O3 and above.
  if (OptLevel < 3) {
    return false;
  }

  // We are looking for 16, single bblock loops which have a backedge count of
  // 3.
  if (std::distance(LI.begin(), LI.end()) != 16 &&
      std::distance(LI.begin(), LI.end()) != 4) {
    return false;
  }

  // Perform the cheap bblock count check first.
  for (auto Lp : LI) {
    if (Lp->getNumBlocks() != 1) {
      return false;
    }
  }

  // Check backedge taken count.
  for (auto Lp : LI) {
    auto BECount = SE.getBackedgeTakenCountForHIR(Lp, Lp);
    auto ConstBECount = dyn_cast<SCEVConstant>(BECount);

    if (!ConstBECount || (ConstBECount->getValue()->getSExtValue() != 3)) {
      return false;
    }
  }

  // Perform more checks on the loop body to minimize chances of forming
  // function level region in other cases.
  for (auto Lp : LI) {
    if (!isLoopConcatenationCandidate(Lp->getHeader())) {
      return false;
    }
  }

  return true;
}

HIRRegionIdentification::HIRRegionIdentification(
    Function &F, LoopInfo &LI, DominatorTree &DT, PostDominatorTree &PDT,
    ScalarEvolution &SE, TargetLibraryInfo &TLI, unsigned OptLevel)
    : LI(LI), DT(DT), PDT(PDT), SE(SE), TLI(TLI), OptLevel(OptLevel) {
  runImpl(F);
}

void HIRRegionIdentification::runImpl(Function &F) {
  if (F.hasFnAttribute(Attribute::OptimizeNone)) {
    return;
  }

  if (CreateFunctionLevelRegion || isLoopConcatenationCandidate() ||
      F.hasFnAttribute("may_have_huge_local_malloc")) {
    if (canFormFunctionLevelRegion(F)) {
      createFunctionLevelRegion(F);
    }
  } else {
    formRegions();
  }
}

HIRRegionIdentification::HIRRegionIdentification(HIRRegionIdentification &&RI)
    : IRRegions(std::move(RI.IRRegions)), LI(RI.LI), DT(RI.DT), PDT(RI.PDT),
      SE(RI.SE), TLI(RI.TLI), OptLevel(RI.OptLevel) {}

void HIRRegionIdentification::print(raw_ostream &OS) const {

  for (auto I = IRRegions.begin(), E = IRRegions.end(); I != E; ++I) {
    OS << "\nRegion " << I - IRRegions.begin() + 1 << "\n";
    I->print(OS, 3);
    OS << "\n";
  }
}
