//===- HIRRegionIdentification.cpp - Identifies HIR Regions ---------------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/IntrinsicInst.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRRegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_OptReport/LoopOptReport.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/Intel_XmainOptLevelPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/TargetLibraryInfo.h"

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

static cl::opt<unsigned> LoopMaterializationBBSize(
    "hir-loop-materialization-bb-size", cl::init(50), cl::Hidden,
    cl::desc("Threshold for number of instructions allowed in the basic block "
             "which may be a loop materialization candidate"));

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
      getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F),
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

  if (!IntrinInst || !IntrinInst->hasOperandBundles()) {
    return false;
  }

  StringRef TagName = IntrinInst->getOperandBundleAt(0).getTagName();

  return BeginDir ? (TagName.equals("DIR.OMP.PARALLEL.LOOP") ||
                     TagName.equals("DIR.OMP.SIMD"))
                  : (TagName.equals("DIR.OMP.END.PARALLEL.LOOP") ||
                     TagName.equals("DIR.OMP.END.SIMD"));

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
      // Check that there are no unknown aliasing calls
      if (isa<CallInst>(Inst) &&
          HLInst::hasUnknownAliasing(cast<CallInst>(&Inst))) {
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

static void printOptReportRemark(const Loop *Lp, const Twine &Remark) {
  // There can be many messages from bblocks checked for loop materialization
  // which can drown out loop related ones. Skipping them for now.
  if (!Lp) {
    return;
  }

  LLVM_DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loop ");
  LLVM_DEBUG(Lp->getHeader()->printAsOperand(dbgs(), false));

  if (const DebugLoc Loc = Lp->getStartLoc()) {
    LLVM_DEBUG(dbgs() << " at "
                      << "(" << Loc.getLine() << "," << Loc.getCol() << ")");
  }

  LLVM_DEBUG(dbgs() << ": " << Remark << "\n");
}

bool HIRRegionIdentification::isSupported(Type *Ty, bool IsGEPRelated,
                                          const Loop *Lp) {
  assert(Ty && "Type is null!");

  while (isa<SequentialType>(Ty) || isa<PointerType>(Ty)) {
    if (auto SeqTy = dyn_cast<SequentialType>(Ty)) {
      if (IsGEPRelated && SeqTy->isVectorTy()) {
        printOptReportRemark(
            Lp, "GEP related vector types currently not supported.");
        return false;
      }
      Ty = SeqTy->getElementType();
    } else {
      Ty = Ty->getPointerElementType();
    }
  }

  auto IntType = dyn_cast<IntegerType>(Ty);
  // Integer type greater than 64 bits not supported. This is mainly to throttle
  // 128 bit integers.
  if (IntType && (IntType->getPrimitiveSizeInBits() > 64)) {
    printOptReportRemark(
        Lp, "Integer types greater than 64 bits currently not supported.");
    return false;
  }

  return true;
}

bool HIRRegionIdentification::containsUnsupportedTy(
    const GEPOrSubsOperator *GEPOrSubs, const Loop *Lp) {

  if (auto *SubInst = dyn_cast<SubscriptInst>(GEPOrSubs)) {
    // Subscript intrinsic can contain vector types.
    return !isSupported(SubInst->getPointerOperandType(), true, Lp) ||
           !isSupported(SubInst->getIndex()->getType(), true, Lp) ||
           !isSupported(SubInst->getStride()->getType(), true, Lp);
  }

  auto *GEPOp = cast<GEPOperator>(GEPOrSubs);

  for (unsigned I = 0, NumOp = GEPOp->getNumOperands(); I < NumOp; ++I) {
    if (!isSupported(GEPOp->getOperand(I)->getType(), true, Lp)) {
      return true;
    }
  }

  // We need to check 'indexed' types as well as they can contain vector types
  // even if indices don't. This is possible when indexing structures which
  // contain vector elements.
  for (auto I = gep_type_begin(GEPOp), E = gep_type_end(GEPOp); I != E; ++I) {
    if (!isSupported(I.getIndexedType(), true, Lp)) {
      return true;
    }
  }

  return false;
}

bool HIRRegionIdentification::containsUnsupportedTy(const Instruction *Inst,
                                                    const Loop *Lp) {

  if (auto GEPOp = dyn_cast<GEPOrSubsOperator>(Inst)) {
    return containsUnsupportedTy(GEPOp, Lp);
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
    if (!isSupported(Inst->getOperand(I)->getType(), false, Lp)) {
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
  const unsigned O3MaxIfNestThreshold = 6;
  const unsigned SmallTripThreshold = 16;

public:
  CostModelAnalyzer(const HIRRegionIdentification &RI, const Loop &Lp,
                    const SCEV *BECount, bool &ThrottleParentLoop);

  bool isProfitable() const { return IsProfitable; }

  void analyze();

  bool visitBasicBlock(const BasicBlock &BB);
  bool visitInstruction(const Instruction &Inst);
  bool visitLoadInst(const LoadInst &LI);
  bool visitStoreInst(const StoreInst &SI);
  bool visitCallInst(const CallInst &CI);
  bool visitBranchInst(const BranchInst &BI);

  void printStats() const;
};

HIRRegionIdentification::CostModelAnalyzer::CostModelAnalyzer(
    const HIRRegionIdentification &RI, const Loop &Lp, const SCEV *BECount,
    bool &ThrottleParentLoop)
    : RI(RI), Lp(Lp), IsInnermostLoop(Lp.empty()),
      IsUnknownLoop(isa<SCEVCouldNotCompute>(BECount)), IsProfitable(true),
      OptLevel(RI.OptLevel), InstCount(0), UnstructuredJumpCount(0),
      IfCount(0) {

  HeaderDomNode = RI.DT.getNode(Lp.getHeader());

  auto ConstBECount = dyn_cast<SCEVConstant>(BECount);
  IsSmallTripLoop =
      (ConstBECount &&
       (ConstBECount->getValue()->getZExtValue() <= SmallTripThreshold));

  // Suppress parent loops of unknown loops.
  ThrottleParentLoop = IsUnknownLoop;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void HIRRegionIdentification::CostModelAnalyzer::printStats() const {
  dbgs() << "Loop instruction count: " << InstCount << "\n";
  dbgs() << "Loop goto count: " << UnstructuredJumpCount << "\n";
  dbgs() << "Loop if count: " << IfCount << "\n";
}
#endif

void HIRRegionIdentification::CostModelAnalyzer::analyze() {

  // SIMD loops should not be throttled.
  if (isSIMDOrParLoop(Lp)) {
    IsProfitable = true;
    return;
  }

  // Only allow innermost multi-exit loops for now.
  if (!Lp.getExitingBlock() && !Lp.empty()) {
    printOptReportRemark(
        &Lp, "Outer multi-exit loop throttled for compile time reasons.");
    IsProfitable = false;
    return;
  }

  // Only handle standalone single bblock unknown loops at O2. We allow bigger
  // standalone innermost loops at O3.
  // We don't do much for outer unknown loops except prefetching which isn't
  // ready yet. Innermost unknown loops embedded inside other loops are
  // throttled for compile time reasons.
  if (IsUnknownLoop &&
      (((OptLevel < 3) && (Lp.getNumBlocks() != 1)) || !Lp.empty())) {
    printOptReportRemark(
        &Lp, "Non-countable loop throttled for compile time reasons.");
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
  if ((BBInstCount + InstCount) > 10 * MaxInstThreshold) {
    printOptReportRemark(&Lp,
                         "Throttled due to presence of too many statements.");
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
  // Subscript instructions are like GEPs and hence most likely eliminated in
  // HIR. This check can be removed once we add support for them in
  // ScalarEvolution.
  if (!isa<CmpInst>(Inst) && !isa<SubscriptInst>(Inst) &&
      !isa<DbgInfoIntrinsic>(Inst)) {

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
    printOptReportRemark(&Lp,
                         "Throttled due to presence of too many statements.");
  }

  return Ret;
}

bool HIRRegionIdentification::CostModelAnalyzer::visitLoadInst(
    const LoadInst &LI) {
  if (LI.isVolatile()) {
    printOptReportRemark(&Lp, "Throttled due to presence of volatile load.");
    return false;
  }

  return visitInstruction(static_cast<const Instruction &>(LI));
}

bool HIRRegionIdentification::CostModelAnalyzer::visitStoreInst(
    const StoreInst &SI) {
  if (SI.isVolatile()) {
    printOptReportRemark(&Lp, "Throttled due to presence of volatile store.");
    return false;
  }

  return visitInstruction(static_cast<const Instruction &>(SI));
}

bool HIRRegionIdentification::CostModelAnalyzer::visitCallInst(
    const CallInst &CI) {

  // Allow user calls in small trip innermost loops so they can be completely
  // unrolled.
  // Also allow them in innermost loops at O3 and above. They may be candidates
  // for predicate optimization.
  // TODO: consider removing this logic allowing user calls at O3 for all loops.
  if (IsInnermostLoop && (IsSmallTripLoop || OptLevel > 2)) {
    return visitInstruction(static_cast<const Instruction &>(CI));
  }

  // Allow intrinsic calls.
  if (isa<IntrinsicInst>(CI)) {
    return visitInstruction(static_cast<const Instruction &>(CI));
  }

  auto *Func = CI.getCalledFunction();
  if (Func) {

    LibFunc LF;
    // Allow library and vectorizable calls.
    if ((RI.TLI.getLibFunc(Func->getName(), LF) && RI.TLI.has(LF)) ||
        RI.TLI.isFunctionVectorizable(Func->getName())) {
      return visitInstruction(static_cast<const Instruction &>(CI));
    }
  }

  printOptReportRemark(&Lp, "Throttled due to presence of user calls.");
  return false;
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
    printOptReportRemark(&Lp, "Throttled due to presence of too many ifs.");
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
    printOptReportRemark(
        &Lp, "Loop throttled due to presence of too many nested ifs.");
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
    printOptReportRemark(&Lp, "Loop throttled due to presence of goto.");
    return false;
  }

  return true;
}

static bool isStructFieldLoadCast(const Value *Val) {
  // We are looking for this pattern-
  //
  // %ptr = GEP struct* p, index, <struct offset>
  // %ld = load %ptr
  // val = uitofp %ld
  auto *CastInst = dyn_cast<UIToFPInst>(Val);

  if (!CastInst) {
    return false;
  }

  auto *LInst = dyn_cast<LoadInst>(CastInst->getOperand(0));

  if (!LInst) {
    return false;
  }

  auto *GEP = dyn_cast<GetElementPtrInst>(LInst->getPointerOperand());

  if (!GEP || GEP->getNumOperands() != 3) {
    return false;
  }

  auto *BasePtr = GEP->getPointerOperand();

  if (!BasePtr->getType()->getPointerElementType()->isStructTy()) {
    return false;
  }

  return true;
}

static bool isConvolutionReduction(const PHINode *HeaderPhi,
                                   const Value **CommonFMulOperand) {
  // We are looking for this pattern-
  // loopheader:
  //   %redn = phi double [ %init, %preheader ] [ %add, %latch ]
  //   %ptr = GEP struct* p, index, <struct offset>
  //   %ld = load %ptr
  //   %cast = uitofp %ld
  //   %mul = fmul %cast, %common-op
  //   %add = fadd %redn, %mul

  if (!HeaderPhi->getType()->isDoubleTy()) {
    return false;
  }

  unsigned NumUses = HeaderPhi->getNumUses();

  if (NumUses > 2 || NumUses == 0) {
    return false;
  }

  const Instruction *FAddInst = nullptr;

  if (NumUses == 1) {
    FAddInst = cast<Instruction>(*HeaderPhi->user_begin());
  } else {
    auto *FirstUser = cast<Instruction>(*HeaderPhi->user_begin());
    auto *SecondUser = cast<Instruction>(*(std::next(HeaderPhi->user_begin())));

    if (isa<DbgInfoIntrinsic>(FirstUser)) {
      FAddInst = SecondUser;
    } else if (!isa<DbgInfoIntrinsic>(SecondUser)) {
      return false;
    } else {
      FAddInst = FirstUser;
    }
  }

  if (FAddInst->getOpcode() != Instruction::FAdd) {
    return false;
  }

  if (FAddInst->getParent() != HeaderPhi->getParent()) {
    return false;
  }

  if (HeaderPhi->getOperand(0) != FAddInst &&
      HeaderPhi->getOperand(1) != FAddInst) {
    return false;
  }

  auto *Op0 = FAddInst->getOperand(0);
  auto *FMulInst =
      dyn_cast<Instruction>((Op0 == HeaderPhi) ? FAddInst->getOperand(1) : Op0);

  if (!FMulInst || FMulInst->getOpcode() != Instruction::FMul) {
    return false;
  }

  auto *MulOp0 = FMulInst->getOperand(0);
  auto *MulOp1 = FMulInst->getOperand(1);
  const Value *CommonOperand = nullptr;

  if (isStructFieldLoadCast(MulOp0)) {
    CommonOperand = MulOp1;
  } else if (isStructFieldLoadCast(MulOp1)) {
    CommonOperand = MulOp0;
  } else {
    return false;
  }

  if (!*CommonFMulOperand) {
    *CommonFMulOperand = CommonOperand;
  } else if (CommonOperand != *CommonFMulOperand) {
    return false;
  }

  return true;
}

bool isInnermostConvolutionLoop(const Loop &Lp) {
  if (!Lp.empty() || !Lp.getExitingBlock()) {
    return false;
  }

  unsigned ReductionCount = 0;
  const Value *CommonFMulOperand = nullptr;

  for (auto &Phi : Lp.getHeader()->phis()) {
    if (isConvolutionReduction(&Phi, &CommonFMulOperand)) {
      ++ReductionCount;
    }
  }

  return ReductionCount >= 3;
}

static bool isMiddleConvolutionLoop(const Loop &Lp) {
  // Allow one innermost loop.
  if (std::distance(Lp.begin(), Lp.end()) != 1) {
    return false;
  }

  if (!Lp.getExitingBlock()) {
    return false;
  }

  if (!isInnermostConvolutionLoop(**Lp.begin())) {
    return false;
  }

  return true;
}

static bool isOuterConvolutionLoop(const Loop &Lp, const SCEV *BECount) {

  // We are looking for an outer single-exit countable loop.
  if (Lp.empty() || !Lp.getExitingBlock() ||
      (BECount && isa<SCEVCouldNotCompute>(BECount))) {
    return false;
  }

  // Allow two inner loops.
  if (std::distance(Lp.begin(), Lp.end()) != 2) {
    return false;
  }

  if (!isMiddleConvolutionLoop(**Lp.begin())) {
    return false;
  }

  if (!isMiddleConvolutionLoop(**(Lp.begin() + 1))) {
    return false;
  }

  return true;
}

static bool isOutermostConvolutionLoop(const Loop &Lp) {

  if (Lp.empty() || !Lp.getExitingBlock()) {
    return false;
  }

  if (std::distance(Lp.begin(), Lp.end()) != 1) {
    return false;
  }

  if (!isOuterConvolutionLoop(**Lp.begin(), nullptr)) {
    return false;
  }

  return true;
}

static bool containsInvariantSwitchInInnermostLoop(const Loop *Lp,
                                                   const Loop *InnermostLp,
                                                   PostDominatorTree &PDT) {

  // Look for invariant switch in innermost child loop of Lp.
  for (auto *BB :
       make_range(InnermostLp->block_begin(), InnermostLp->block_end())) {
    auto *Term = BB->getTerminator();

    auto *Switch = dyn_cast<SwitchInst>(Term);

    if (!Switch) {
      continue;
    }

    auto *CondInst = dyn_cast<Instruction>(Switch->getCondition());

    // Check loop invariance of switch condition w.r.t current loop 'Lp'.
    if (CondInst && Lp->contains(CondInst->getParent())) {
      continue;
    }

    // Check post-domination of switch w.r.t innermost loop's header. This is a
    // profitability check for high possibility of 'unswitching' of switch.
    if (!PDT.dominates(BB, InnermostLp->getHeader())) {
      continue;
    }

    return true;
  }

  return false;
}

static bool containsInvariantSwitchInInnermostLoop(const Loop *Lp,
                                                   const SCEV *BECount,
                                                   PostDominatorTree &PDT) {
  // Only allow countable loops.
  if (isa<SCEVCouldNotCompute>(BECount)) {
    return false;
  }

  const Loop *InnermostLp = Lp;
  // Recurse into single child loop to get to innermost loop.
  while (!InnermostLp->empty()) {
    if (std::distance(InnermostLp->begin(), InnermostLp->end()) != 1) {
      return false;
    }

    InnermostLp = *(InnermostLp->begin());
  }

  return containsInvariantSwitchInInnermostLoop(Lp, InnermostLp, PDT);
}

bool HIRRegionIdentification::shouldThrottleLoop(
    const Loop &Lp, const SCEV *BECount, bool &ThrottleParentLoop) const {

  if (!CostModelThrottling) {
    return false;
  }

  if (OptLevel > 2) {
    if (isOuterConvolutionLoop(Lp, BECount) || isOutermostConvolutionLoop(Lp)) {
      return false;
    }

    if (containsInvariantSwitchInInnermostLoop(&Lp, BECount, PDT)) {
      return false;
    }
  }

  CostModelAnalyzer CMA(*this, Lp, BECount, ThrottleParentLoop);
  CMA.analyze();

  LLVM_DEBUG(CMA.printStats());

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

static bool isFusionMetadata(MDNode *Node) {
  MDString *Str = getStringMetadata(Node);

  return Str && Str->getString().startswith("llvm.loop.fusion");
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
      isLoopCountMetadata(Node) || LoopOptReport::isOptReportMetadata(Node) ||
      isFusionMetadata(Node)) {
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
      printOptReportRemark(Lp, "Irreducible CFG not supported.");
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
    printOptReportRemark(Lp, "Exception handling currently not supported.");
    return false;
  }

  auto Term = BB->getTerminator();

  if (isa<IndirectBrInst>(Term)) {
    printOptReportRemark(Lp, "Indirect branches currently not supported.");
    return false;
  }

  if (isa<InvokeInst>(Term) || isa<ResumeInst>(Term) ||
      isa<CatchSwitchInst>(Term) || isa<CatchReturnInst>(Term) ||
      isa<CleanupReturnInst>(Term)) {
    printOptReportRemark(Lp, "Exception handling currently not supported.");
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
            printOptReportRemark(
                Lp, "Switch instruction with "
                    "multiple successors outside the loop currently "
                    "not supported.");
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
      printOptReportRemark(Lp,
                           "Atomic instructions are currently not supported.");
      return false;
    }

    // TODO: think about HIR representation for
    // InsertValueInst/ExtractValueInst.
    if (isa<InsertValueInst>(Inst) || isa<ExtractValueInst>(Inst)) {
      printOptReportRemark(
          Lp, "InsertValueInst/ExtractValueInst currently not supported.");
      return false;
    }

    if (auto CInst = dyn_cast<CallInst>(Inst)) {
      if (CInst->isInlineAsm()) {
        printOptReportRemark(Lp, "Inline assembly currently not supported.");
        return false;
      }

      if (hasUnsupportedOperandBundle(CInst)) {
        printOptReportRemark(Lp, "Unsupported operand bundle.");
        return false;
      }
    }

    if (containsUnsupportedTy(&*Inst, Lp)) {
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
                                              bool IsFunctionRegionMode,
                                              bool &ThrottleParentLoop) const {

  // At least one of this loop's subloops reach MaxLoopNestLevel so we cannot
  // generate this loop.
  if (LoopnestDepth > MaxLoopNestLevel) {
    printOptReportRemark(&Lp, "Loopnest is more than " +
                                  Twine(MaxLoopNestLevel) + " deep.");
    return false;
  }

  // Loop is not in a handleable form.
  if (!Lp.isLoopSimplifyForm()) {
    printOptReportRemark(&Lp, "Loop structure is not handleable.");
    return false;
  }

  // Skip loops with unsupported pragmas.
  MDNode *LoopID = Lp.getLoopID();
  if (!DisablePragmaBailOut && !isSIMDOrParLoop(Lp) && LoopID &&
      !isSupportedMetadata(LoopID)) {
    printOptReportRemark(&Lp, "Loop has unsupported pragma.");
    return false;
  }

  auto LatchBB = Lp.getLoopLatch();

  // We cannot build lexical links if dominator/post-dominator info is absent.
  // This can be due to unreachable/infinite loops.
  if (!DT.getNode(LatchBB) || !PDT.getNode(LatchBB)) {
    printOptReportRemark(&Lp, "Unreachable/Infinite loop not supported.");
    return false;
  }

  // Check that the loop backedge is a conditional branch.
  auto BrInst = dyn_cast<BranchInst>(LatchBB->getTerminator());

  if (!BrInst) {
    printOptReportRemark(
        &Lp, "Non-branch instructions in loop latch currently not supported.");
    return false;
  }

  if (BrInst->isUnconditional()) {
    printOptReportRemark(&Lp, "Unconditional branch instructions in loop latch "
                              "currently not supported.");
    return false;
  }

  const Value *LatchVal = BrInst->getCondition();

  auto LatchCmpInst = dyn_cast<Instruction>(LatchVal);

  if (!LatchCmpInst) {
    printOptReportRemark(
        &Lp, "Non-instruction latch condition currently not supported.");
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
    printOptReportRemark(&Lp, "Loop with trip count greater than the IV range "
                              "currently not supported.");
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
    // TODO: remove this restriction
    printOptReportRemark(&Lp, "Loop without IV not supported.");
    return false;
  }

  if (IVNode->getType()->getPrimitiveSizeInBits() == 1) {
    // The following loop with i1 type IV has a trip count of 2 which is
    // outside its range. This is a quirk of SSA. CG will generate an infinite
    // loop for this case if we let it through.
    // for.i:
    // %i.08.i = phi i1 [ true, %entry ], [ false, %for.i ]
    // br i1 %i.08.i, label %for.i, label %exit
    printOptReportRemark(&Lp, "i1 type IV currently not handled.");
    return false;
  }

  // We skip cost model throttling for function level region.
  if (!IsFunctionRegionMode &&
      shouldThrottleLoop(Lp, BECount, ThrottleParentLoop)) {
    return false;
  }

  return true;
}

void HIRRegionIdentification::createRegion(
    const ArrayRef<const Loop *> &Loops,
    const SmallPtrSetImpl<const BasicBlock *> *IntermediateBlocks) {

  if (RegionNumThreshold && (RegionCount == RegionNumThreshold)) {
    printOptReportRemark(Loops.front(),
                         "Region throttled due to region number threshold.");
    return;
  }

  IRRegion::RegionBBlocksTy BBlocks;
  IRRegion::RegionBBlocksTy NonLoopBBlocks;

  if (IntermediateBlocks) {
    NonLoopBBlocks.append(IntermediateBlocks->begin(),
                          IntermediateBlocks->end());
  }

  BasicBlock *EntryBB = Loops.front()->getHeader();
  BasicBlock *ExitBB = nullptr;

  for (auto *Lp : Loops) {
    bool IsFirstLoop = (Lp == Loops.front());
    bool IsLastLoop = (Lp == Loops.back());

    isSIMDOrParLoop(*Lp, &NonLoopBBlocks, IsFirstLoop ? &EntryBB : nullptr,
                    IsLastLoop ? &ExitBB : nullptr);

    BBlocks.append(Lp->getBlocks().begin(), Lp->getBlocks().end());
  }

  BBlocks.append(NonLoopBBlocks.begin(), NonLoopBBlocks.end());

  IRRegions.emplace_back(EntryBB,
                         ExitBB ? ExitBB : Loops.back()->getLoopLatch(),
                         BBlocks, NonLoopBBlocks);

  // Keep track of all bblocks part of regular regions.
  AllLoopBasedRegionsBBlocks.insert(BBlocks.begin(), BBlocks.end());

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

  bool ThrottleParentLoop = false;
  // Check whether Lp is generable.
  if (Generable &&
      !isSelfGenerable(Lp, ++LoopnestDepth, false, ThrottleParentLoop)) {
    Generable = false;
  }

  if (Generable) {
    // Entire loopnest is generable. Add Lp in generable set.
    GenerableLoops.push_back(&Lp);
  } else {
    // Add sub loops of Lp in generable set.
    GenerableLoops.append(SubGenerableLoops.begin(), SubGenerableLoops.end());
  }

  return (!ThrottleParentLoop && Generable);
}

static const Instruction *getSingleUserInSameBBlock(const Instruction *Inst,
                                                    const BasicBlock *BB) {

  auto IsValidUser = [BB](const Instruction *Inst) {
    if (Inst->getParent() != BB) {
      return false;
    }
    if (isa<CallInst>(Inst)) {
      return false;
    }
    return true;
  };

  if (Inst->hasOneUse()) {
    auto *UserInst = cast<Instruction>(*Inst->user_begin());

    if (!IsValidUser(UserInst)) {
      return nullptr;
    }

    return UserInst;
  }

  // This is to handle cases like this-
  // t1 = A[0]
  // t2 = t1 * t1
  if (!Inst->hasNUses(2)) {
    return nullptr;
  }

  auto *FirstUserInst = cast<Instruction>(*Inst->user_begin());
  auto *SecondUserInst = cast<Instruction>(*(std::next(Inst->user_begin())));

  if ((FirstUserInst != SecondUserInst) || !IsValidUser(FirstUserInst)) {
    return nullptr;
  }

  return FirstUserInst;
}

static bool haveExpectedDistance(const Value *Val1, const Value *Val2,
                                 ScalarEvolution &SE,
                                 uint64_t ExpectedDistance) {
  auto *SC1 = SE.getSCEV(const_cast<Value *>(Val1));
  auto *SC2 = SE.getSCEV(const_cast<Value *>(Val2));

  auto *ConstSCEV = dyn_cast<SCEVConstant>(SE.getMinusSCEV(SC1, SC2));

  if (ConstSCEV &&
      (ConstSCEV->getValue()->getZExtValue() == ExpectedDistance)) {
    return true;
  }

  return false;
}

static bool foundMatchingLoads(
    const LoadInst *LInst,
    SmallVectorImpl<std::pair<const LoadInst *, const Instruction *>>
        &CandidateLoads,
    ScalarEvolution &SE, const DataLayout &DL) {
  // Looks for two loads with following properties-
  // 1) Have single user in same bblock.
  // 2) Opcode and type of single user is the same.
  // 3) If single user is a store, they access consecutive locations. Otherwise,
  //    we check the same properties on second level users.
  // 4) Loads access consecutive locations.

  auto *ParentBB = LInst->getParent();
  auto *User = getSingleUserInSameBBlock(LInst, ParentBB);

  if (!User) {
    return false;
  }

  auto FoundMatchingUsers = [&SE, &DL](const Instruction *User1,
                                       const Instruction *User2) {
    if (User1->getOpcode() != User2->getOpcode()) {
      return false;
    }

    if (auto *StoreUser1 = dyn_cast<StoreInst>(User1)) {
      auto *StoreUser2 = cast<StoreInst>(User2);

      auto *Ptr1 = StoreUser1->getPointerOperand();
      uint64_t AllocSize =
          DL.getTypeAllocSize(Ptr1->getType()->getPointerElementType());

      if (!haveExpectedDistance(Ptr1, StoreUser2->getPointerOperand(), SE,
                                AllocSize)) {
        return false;
      }
    } else if (User1->getType() != User2->getType()) {
      // Even though opcodes match, the type may be different for cast
      // instructions.
      return false;
    }

    return true;
  };

  auto *Ptr = LInst->getPointerOperand();
  uint64_t LoadSize = DL.getTypeAllocSize(LInst->getType());

  for (auto &PrevEntry : CandidateLoads) {

    auto *PrevLoad = PrevEntry.first;
    if (LInst->getType() != PrevLoad->getType()) {
      continue;
    }

    auto *PrevUser = PrevEntry.second;
    if (!FoundMatchingUsers(User, PrevUser)) {
      continue;
    }

    if (!isa<StoreInst>(User) && (User != PrevUser)) {
      auto *SecondLevelUser = getSingleUserInSameBBlock(User, ParentBB);
      auto *SecondLevelPrevUser = getSingleUserInSameBBlock(PrevUser, ParentBB);

      if (!SecondLevelUser || !SecondLevelPrevUser ||
          !FoundMatchingUsers(SecondLevelUser, SecondLevelPrevUser)) {
        continue;
      }
    }

    auto *PrevPtr = PrevLoad->getPointerOperand();

    if (haveExpectedDistance(Ptr, PrevPtr, SE, LoadSize)) {
      return true;
    }
  }

  CandidateLoads.emplace_back(LInst, User);

  return false;
}

static bool
foundMatchingStores(const StoreInst *SInst,
                    SmallVectorImpl<const StoreInst *> &CandidateStores,
                    ScalarEvolution &SE, const DataLayout &DL) {
  // Looks for two stores with following properties-
  // 1) Stores access consecutive locations.
  // 2) Store values are the same-
  //    A[0] = t1;
  //    A[1] = t1;
  // Rest of the cases are mostly handled by load path.
  //
  // Leaving following case as a TODO as just matching for constants results
  // in lot of cases with arbitrary constants. Stricter matching requires
  // more logic.
  //
  // A[0] = 0;
  // A[1] = 1;

  auto *Ptr = SInst->getPointerOperand();
  auto *StoreVal = SInst->getValueOperand();

  uint64_t AllocSize =
      DL.getTypeAllocSize(Ptr->getType()->getPointerElementType());

  for (auto *PrevStore : CandidateStores) {
    if (StoreVal != PrevStore->getValueOperand()) {
      continue;
    }

    auto *PrevPtr = PrevStore->getPointerOperand();
    if (Ptr->getType() != PrevPtr->getType()) {
      continue;
    }

    if (haveExpectedDistance(Ptr, PrevPtr, SE, AllocSize)) {
      return true;
    }
  }

  CandidateStores.push_back(SInst);

  return false;
}

static bool isLoopMaterializationCandidate(const BasicBlock &BB,
                                           ScalarEvolution &SE) {

  // Skip big bblocks.
  if (BB.size() > LoopMaterializationBBSize) {
    return false;
  }

  SmallVector<std::pair<const LoadInst *, const Instruction *>, 8>
      CandidateLoads;
  SmallVector<const StoreInst *, 8> CandidateStores;

  auto &DL = BB.getParent()->getParent()->getDataLayout();

  for (auto &Inst : BB) {
    if (auto *LInst = dyn_cast<LoadInst>(&Inst)) {
      if (LInst->isVolatile()) {
        return false;
      }

      if (foundMatchingLoads(LInst, CandidateLoads, SE, DL)) {
        return true;
      }

    } else if (auto *SInst = dyn_cast<StoreInst>(&Inst)) {
      if (SInst->isVolatile()) {
        return false;
      }

      if (foundMatchingStores(SInst, CandidateStores, SE, DL)) {
        return true;
      }

    } else if (isa<CallInst>(Inst)) {
      // Ignore candidates across call insts.
      CandidateLoads.clear();
      CandidateStores.clear();
    }
  }

  return false;
}

HIRRegionIdentification::iterator
HIRRegionIdentification::getLexicalInsertionPos(const BasicBlock *BB) {
  // Lexical position is determined using following 'reachability' logic-
  //
  // 1) Returns the first region whose entry block is reachable from BB. In this
  // case, BB comes lexically before this region.
  //
  // Or
  //
  // 2) Returns the second region amongst two consecutive regions such that
  // first region's entry block can reach BB but second region's entry block
  // cannot. In this case BB comes lexically after first region but has no
  // 'reachability' relationship with the next region. For example-
  //
  // if () {
  //   Lp1
  //   BB
  // } else {
  //   Lp2
  // }
  //
  // Or
  //
  // 3) Returns end iterator.
  //

  SmallPtrSet<const BasicBlock *, 1> FromBBs, EmptyEndBBs;
  FromBBs.insert(BB);
  auto RegIt = IRRegions.begin();

  for (auto PrevRegIt = IRRegions.end(), E = IRRegions.end(); RegIt != E;
       PrevRegIt = RegIt, ++RegIt) {
    auto *RegEntryBB = RegIt->getEntryBBlock();

    // Satisfied condition 1) mentioned above.
    if (isReachableFrom(RegEntryBB, EmptyEndBBs, FromBBs)) {
      break;
    }

    if (PrevRegIt == E) {
      continue;
    }

    SmallPtrSet<const BasicBlock *, 1> PrevRegEntry, RegEntry;
    PrevRegEntry.insert(PrevRegIt->getEntryBBlock());
    RegEntry.insert(RegEntryBB);

    // We can end search if we reach current region from previous region.
    if (isReachableFrom(BB, RegEntry, PrevRegEntry)) {

      // Satisfied condition 2) mentioned above.
      if (!isReachableFrom(BB, EmptyEndBBs, RegEntry)) {
        break;
      }
    }
  }

  return RegIt;
}

void HIRRegionIdentification::formRegionsForLoopMaterialization(
    Function &Func) {

  for (auto &BB : Func) {
    if (AllLoopBasedRegionsBBlocks.count(&BB)) {
      continue;
    }

    if (!isGenerable(&BB, nullptr)) {
      continue;
    }

    if (!DT.isReachableFromEntry(&BB)) {
      continue;
    }

    if (!isLoopMaterializationCandidate(BB, SE)) {
      continue;
    }

    // Region number threshold setup is half-broken in the presence of regions
    // for loop materialization as they are created after regular regions even
    // though lexically they may fall between regular regions.
    if (RegionNumThreshold && (RegionCount == RegionNumThreshold)) {
      printOptReportRemark(nullptr,
                           "Region throttled due to region number threshold.");
      return;
    }

    auto InsertPos = getLexicalInsertionPos(&BB);

    IRRegion::RegionBBlocksTy BBs, NonLoopBBs;
    BBs.push_back(&BB);
    NonLoopBBs.push_back(&BB);

    IRRegion TempRegion(&BB, &BB, BBs, NonLoopBBs, true);

    // SmallVector does not have emplace().
    IRRegions.insert(InsertPos, std::move(TempRegion));

    RegionCount++;
  }
}

void HIRRegionIdentification::formRegions(Function &Func) {
  SmallVector<const Loop *, 32> GenerableLoops;

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

  } else {
    SmallVector<LoopSpanTy, 8> LoopSpans;
    computeLoopSpansForFusion(GenerableLoops, LoopSpans);

    for (auto &LoopSpan : LoopSpans) {
      // Create region for generable loop span.
      createRegion(LoopSpan.first, &LoopSpan.second);
    }
  }

  formRegionsForLoopMaterialization(Func);
}

void HIRRegionIdentification::createFunctionLevelRegion(Function &Func) {
  if (RegionNumThreshold && (RegionCount == RegionNumThreshold)) {
    printOptReportRemark(nullptr,
                         "Region throttled due to region number threshold.");
    return;
  }

  IRRegion::RegionBBlocksTy BBlocks;

  for (auto BBIt = ++Func.begin(), E = Func.end(); BBIt != E; ++BBIt) {
    BBlocks.push_back(&*BBIt);
  }

  IRRegion::RegionBBlocksTy NonLoopBBlocks;
  IRRegions.emplace_back(&Func.getEntryBlock(), nullptr, BBlocks,
                         NonLoopBBlocks, false, true);

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

  bool ThrottleParentLoop;
  for (auto Lp : AllLoops) {
    if (!isSelfGenerable(*Lp, Lp->getLoopDepth(), true, ThrottleParentLoop)) {
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
    formRegions(F);
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
