//===-- HIRParVecAnalysis.cpp ---------------------------------------------===//
//
// Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the Parallel/Vector Candidate Analysis pass. It
/// identifies auto-parallelization/auto-vectorization candidate loops. For
/// auto-parallelization, this pass will also decide whether
/// auto-parallelization should happen to the loop.
///
/// Available options:
///   -hir-enable-parvec-diag Enable non-vectorization/non-parallelization
///                           diagnostics from ParVec analyzer itself, for
///                           debugging purposes. Normal reporting happens
///                           w/o this flag when analysis is invoked
///                           in ForThreadizer/ForVectorizer modes.
///
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRParVecAnalysis.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_OptReport/Diag.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/VectorUtils.h"

#include "llvm/InitializePasses.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "parvec-analysis"

static cl::opt<bool>
    Diag("hir-enable-parvec-diag", cl::init(false), cl::Hidden,
         cl::desc("Enable non-vectorization/non-parallelization diagnostics "
                  "from ParVec analyzer"));

cl::opt<bool>
    MinMaxIndexEnabled("enable-mmindex", cl::init(true), cl::Hidden,
                       cl::desc("Enable min/max+index idiom recognition"));

cl::opt<bool> VConflictIdiomEnabled("enable-vconflict-idiom", cl::init(false),
                                    cl::Hidden,
                                    cl::desc("Enable vconflict idiom"));

static cl::opt<bool> DisableNonMonotonicIndexes(
    "disable-nonlinear-mmindex", cl::init(true), cl::Hidden,
    cl::desc("Disable min/max+index idiom recognition for non-linear indexes"));

namespace {

/// \brief Visitor class to determine parallelizabilty/vectorizability of loops
/// in the nest.
class ParVecVisitor final : public HLNodeVisitorBase {
private:
  /// Mode - curent analysis mode running, auto-par or auto-vec.
  ParVecInfo::AnalysisMode Mode;
  /// InfoMap - Map associating HLLoops with corresponding par vec info.
  HIRParVecInfoMapType &InfoMap;
  /// TLI - Target library info analysis.
  TargetLibraryInfo *TLI;
  /// DDA - Data dependency analysis handle.
  HIRDDAnalysis *DDA;
  /// SRA - Safe reduction analysis handle
  HIRSafeReductionAnalysis *SRA;

public:
  ParVecVisitor(ParVecInfo::AnalysisMode Mode, TargetLibraryInfo *TLI,
                HIRDDAnalysis *DDA, HIRSafeReductionAnalysis *SRA,
                HIRParVecInfoMapType &InfoMap)
      : Mode(Mode), InfoMap(InfoMap), TLI(TLI), DDA(DDA), SRA(SRA) {}
  /// \brief Determine parallelizability/vectorizability of the loop
  void postVisit(HLLoop *Loop);

  // TODO: Add structural analysis for non-rectangular loop nest.
  // TODO: Add structural analysis for HLSwitch.

  /// \brief catch-all visit().
  void visit(HLNode *Node){};
  /// \brief catch-all postVisit().
  void postVisit(HLNode *Node) {}
};

/// \brief Visitor class to determine parallelizability/vectorizability of
/// the given loop with the given DDG.
class DDWalk final : public HLNodeVisitorBase {
  /// TLI - Target Library Info
  TargetLibraryInfo &TLI;

  /// DDA - Data dependence analysis
  HIRDDAnalysis &DDA;

  /// SRA - Safe reduction analysis
  HIRSafeReductionAnalysis &SRA;

  /// DDG - Data dependency graph.
  DDGraph DDG;
  /// CandidateLoop - current candidate loop.
  HLLoop *CandidateLoop;
  /// Info - Par vec analysis info.
  ParVecInfo *Info;
  /// Indicates whether we have computed safe reduction chain for the loop.
  bool ComputedSafeRedn;

  const HIRVectorIdioms &IdiomList;

  /// \brief Analyze one DDEdge for the source node.
  void analyze(const RegDDRef *SrcRef, const DDEdge *Edge);

  /// \brief Analyze whether the flow dependence edge can be ignored
  /// due to safe reduction.
  bool isSafeReductionFlowDep(const DDEdge *Edge);

public:
  DDWalk(TargetLibraryInfo &TLI, HIRDDAnalysis &DDA,
         HIRSafeReductionAnalysis &SRA, HLLoop *CandidateLoop, ParVecInfo *Info,
         const HIRVectorIdioms &IList)
      : TLI(TLI), DDA(DDA), SRA(SRA), DDG(DDA.getGraph(CandidateLoop)),
        CandidateLoop(CandidateLoop), Info(Info), ComputedSafeRedn(false),
        IdiomList(IList) {}

  /// \brief Visit all outgoing DDEdges for the given node.
  void visit(HLDDNode *Node);

  /// Mark loop containing switches as non-vectorizable until VPO is able to
  /// handle them.
  void visit(HLSwitch *Switch) {
    Info->setVecType(ParVecInfo::SWITCH_STMT);
    Info->setParType(ParVecInfo::SWITCH_STMT);
  }

  /// \brief catch-all visit().
  void visit(HLNode *Node) {}
  /// \brief catch-all postVisit().
  void postVisit(HLNode *Node) {}
};

/// \brief Visitor class to print the cached ParVec analysis results.
class ParVecPrintVisitor final : public HLNodeVisitorBase {
  const HIRParVecInfoMapType &InfoMap;
  raw_ostream &OS;

public:
  ParVecPrintVisitor(const HIRParVecInfoMapType &theMap, raw_ostream &theOS)
      : InfoMap(theMap), OS(theOS) {}
  /// \brief Print for one loop.
  void visit(HLLoop *Loop) {
    auto Info = InfoMap.find(Loop);
    if (Info != InfoMap.end())
      Info->second->print(OS);
  }

  /// \brief catch-all visit().
  void visit(HLNode *Node) {}
  /// \brief catch-all postVisit().
  void postVisit(HLNode *Node) {}
};

// Dumps bail-out messages when idiom recognition fails.
struct MatchFail {
  MatchFail(const char *IdiomName)
#ifndef NDEBUG
    : IdiomName(IdiomName)
#endif
  {
  }
  bool operator()(Twine Reason) {
    LLVM_DEBUG(dbgs() << '[' << IdiomName << "] Skipped: " << Reason << '\n');
    return false;
  }

private:
#ifndef NDEBUG
  const char *IdiomName;
#endif
};

} // unnamed namespace

void ParVecVisitor::postVisit(HLLoop *Loop) {
  // Analyze parallelizability/vectorizability if not cached.
  ParVecInfo::get(Mode, InfoMap, TLI, DDA, SRA, Loop);
}

FunctionPass *llvm::createHIRParVecAnalysisPass() {
  return new HIRParVecAnalysisWrapperPass();
}

char HIRParVecAnalysisWrapperPass::ID = 0;

HIRParVecAnalysisWrapperPass::HIRParVecAnalysisWrapperPass()
    : FunctionPass(ID) {
  initializeHIRParVecAnalysisWrapperPassPass(*PassRegistry::getPassRegistry());
}

INITIALIZE_PASS_BEGIN(HIRParVecAnalysisWrapperPass, "hir-parvec-analysis",
                      "HIR Parallel/Vector Candidate Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRParVecAnalysisWrapperPass, "hir-parvec-analysis",
                    "HIR Parallel/Vector Candidate Analysis", false, true)

void HIRParVecAnalysisWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<TargetLibraryInfoWrapperPass>();
  AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
  AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
  AU.addRequiredTransitive<HIRSafeReductionAnalysisWrapperPass>();
}

bool HIRParVecAnalysisWrapperPass::runOnFunction(Function &F) {
  if (HIRParVecAnalysis::isSIMDEnabledFunction(F)) {
    HPVA.reset(
        new HIRParVecAnalysis(false, nullptr, nullptr, nullptr, nullptr));
    return false;
  }

  auto TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
  auto HIRF = &getAnalysis<HIRFrameworkWrapperPass>().getHIR();
  auto DDA = &getAnalysis<HIRDDAnalysisWrapperPass>().getDDA();
  auto SRA = &getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR();

  // ParVecAnalysis runs in on-demand mode. runOnFunction is almost no-op.
  // In the debug mode, run actual analysis in ParallelVector mode, print
  // the result, and releas memory as if nothing happened. "opt -analyze"
  // doesn't print anything.
  LLVM_DEBUG(HPVA.reset(new HIRParVecAnalysis(true, TLI, HIRF, DDA, SRA)));
  LLVM_DEBUG(HPVA->analyze(ParVecInfo::ParallelVector));
  LLVM_DEBUG(HPVA->printAnalysis(dbgs()));

  HPVA.reset(new HIRParVecAnalysis(true, TLI, HIRF, DDA, SRA));

  return false;
}

AnalysisKey HIRParVecAnalysisPass::Key;

HIRParVecAnalysis HIRParVecAnalysisPass::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  if (HIRParVecAnalysis::isSIMDEnabledFunction(F))
    return HIRParVecAnalysis(false, nullptr, nullptr, nullptr, nullptr);

  auto TLI = &AM.getResult<TargetLibraryAnalysis>(F);
  auto HIRF = &AM.getResult<HIRFrameworkAnalysis>(F);
  auto DDA = &AM.getResult<HIRDDAnalysisPass>(F);
  auto SRA = &AM.getResult<HIRSafeReductionAnalysisPass>(F);

  return HIRParVecAnalysis(true, TLI, HIRF, DDA, SRA);
}

const ParVecInfo *HIRParVecAnalysis::getInfo(ParVecInfo::AnalysisMode Mode,
                                             HLLoop *Loop) {
  if (!Enabled) {
    return nullptr;
  }
  auto Info = ParVecInfo::get(Mode, InfoMap, TLI, DDA, SRA, Loop);
  return Info;
}

void HIRParVecAnalysis::analyze(ParVecInfo::AnalysisMode Mode) {
  if (!Enabled) {
    return;
  }
  ParVecVisitor Vis(Mode, TLI, DDA, SRA, InfoMap);
  HIRF.getHLNodeUtils().visitAll(Vis);
}

void HIRParVecAnalysis::analyze(ParVecInfo::AnalysisMode Mode,
                                HLRegion *Region) {
  if (!Enabled) {
    return;
  }
  ParVecVisitor Vis(Mode, TLI, DDA, SRA, InfoMap);
  HLNodeUtils::visit(Vis, Region);
}

void HIRParVecAnalysis::analyze(ParVecInfo::AnalysisMode Mode, HLLoop *Loop) {
  if (!Enabled) {
    return;
  }
  ParVecVisitor Vis(Mode, TLI, DDA, SRA, InfoMap);
  HLNodeUtils::visit(Vis, Loop);
}

void HIRParVecAnalysis::markLoopBodyModified(const HLLoop *Lp) {
  assert(Lp && " Loop parameter is null.");
  for (; Lp; Lp = Lp->getParentLoop())
    InfoMap.erase(Lp);
}

void HIRParVecAnalysis::printAnalysis(raw_ostream &OS) const {
  ParVecPrintVisitor Vis(InfoMap, OS);
  HIRF.getHLNodeUtils().visitAll(Vis);
}

bool HIRParVecAnalysis::isSIMDEnabledFunction(Function &Func) {
  // TODO: ABI related stuff should become part of TargetTransformInfo.
  return Func.getName().startswith("_ZGV");
}

bool DDWalk::isSafeReductionFlowDep(const DDEdge *Edge) {
  assert(Edge->isFlow() && "Flow edge expected!");

  DDRef *SrcRef = Edge->getSrc();

  const HLDDNode *WriteNode = SrcRef->getHLDDNode();
  auto Inst = dyn_cast<HLInst>(WriteNode);

  if (!Inst) {
    return false;
  }

  if (!ComputedSafeRedn) {
    SRA.computeSafeReductionChains(CandidateLoop);
    ComputedSafeRedn = true;
  }

  auto SRI = SRA.getSafeRedInfo(Inst);

  if (SRI) {
    // TODO: add support.
    return !SRI->HasUnsafeAlgebra;
  }

  return IdiomList.isIdiom(Inst) == HIRVectorIdioms::MinOrMax ||
         IdiomList.isIdiom(Inst) == HIRVectorIdioms::MMFirstLastIdx ||
         IdiomList.isIdiom(Inst) == HIRVectorIdioms::MMFirstLastVal;
}

void DDWalk::analyze(const RegDDRef *SrcRef, const DDEdge *Edge) {

  LLVM_DEBUG(Edge->dump());

  unsigned NestLevel = CandidateLoop->getNestingLevel();
  if (!Edge->preventsParallelization(NestLevel)) {
    LLVM_DEBUG(dbgs() << "\tis safe to vectorize/parallelize\n");
    return;
  }
  if (Info->isVectorMode()) {
    if (!Edge->preventsVectorization(NestLevel)) {
      LLVM_DEBUG(
          dbgs() << "\tis safe to vectorize but unsafe to parallelize\n");
      // TODO: Set ParType/ParLoc. Call emitDiag().
      return;
    }
  }

  if (SrcRef->isTerminalRef() &&
      !CandidateLoop->isLiveIn(SrcRef->getSymbase())) {
    LLVM_DEBUG(dbgs() << "\tis safe to vectorize/parallelize (private)\n");
    return;
  }

  if (Edge->isFlow() && isSafeReductionFlowDep(Edge)) {
    LLVM_DEBUG(
        dbgs() << "\tis safe to vectorize/parallelize (safe reduction)\n");
    return;
  }

  if (Info->isVectorMode() && DDA.isRefinableDepAtLevel(Edge, NestLevel)) {
    DDRef *SinkRef = Edge->getSink();

    // Compute the deepest common level.
    unsigned DeepestLevel;
    if (CandidateLoop->isInnermost()) {
      // If CandidateLoop is innermost then we know that its level is also the
      // deepest common level.
      DeepestLevel = NestLevel;
    } else {
      // Otherwise, we may potentially need to reason about two loop nests.
      HLLoop *SrcLoop = SrcRef->getHLDDNode()->getLexicalParentLoop();
      HLLoop *DstLoop = SinkRef->getHLDDNode()->getLexicalParentLoop();
      HLLoop *LCALoop =
          HLNodeUtils::getLowestCommonAncestorLoop(SrcLoop, DstLoop);
      assert(LCALoop && "Refinable deps are expected to share a common loop");
      DeepestLevel = LCALoop->getNestingLevel();
    }
    assert(DeepestLevel && "Computed invalid DeepestLevel for refinement");

    // Input DV set to test for innermost loop vectorization
    // For outer loop vectorization, modification is neeeded here or elsewhere
    auto RefinedDep =
        DDA.refineDV(SrcRef, SinkRef, NestLevel, DeepestLevel, false);
    if (RefinedDep.isIndependent()) {
      LLVM_DEBUG(dbgs() << "\tis safe to vectorize (indep)\n");
      return;
    }
    if (RefinedDep.isRefined()) {
      // RefineDV will not flip the direction
      // the result DV is from source to sink
      // Just need to check for DV. Other conditions are covered by
      // the call to preventsVectorization above
      DirectionVector DirV = RefinedDep.getDV();
      LLVM_DEBUG(DirV.print(dbgs()));
      if (!DirV.isCrossIterDepAtLevel(NestLevel)) {
        LLVM_DEBUG(
            dbgs() << "\tis DV improved by RefineDD: Safe to vectorize\n");
        return;
      }
    }
    LLVM_DEBUG(dbgs() << "\tis unsafe to vectorize\n");
  } else {
    LLVM_DEBUG(dbgs() << "\tDV is not refinable - unsafe to vectorize\n");
  }

  Info->setVecType(ParVecInfo::FE_DIAG_PAROPT_VEC_VECTOR_DEPENDENCE);
  Info->setParType(ParVecInfo::FE_DIAG_PAROPT_VEC_VECTOR_DEPENDENCE);
}

void DDWalk::visit(HLDDNode *Node) {

  // Identify HLInst that is not suitable for auto-parallelization
  // and/or auto-vectorization.
  if (auto Inst = dyn_cast<HLInst>(Node)) {

    auto LIRInst = Inst->getLLVMInstruction();
    DebugLoc Loc = LIRInst->getDebugLoc();

    if (isa<InvokeInst>(LIRInst) || isa<LandingPadInst>(LIRInst)) {
      Info->setVecType(ParVecInfo::EH);
      Info->setParType(ParVecInfo::EH);
      Info->setLoc(Loc);
      return;
    } else if (auto *Call = Inst->getCallInst()) {
      auto Func = Call->getCalledFunction();

      bool IsVectorizable;

      if (Func) {
        if (Func->isIntrinsic()) {
          auto IntrinsicId = Func->getIntrinsicID();
          // @llvm.assume intrinsic is not "trivially" vectorizable because it
          // does not have a vector variant, but it does not prevent
          // vectorization in any way.
          // TODO: lifetime_start/lifetime_end.
          // TODO: Check if ephemeral values (ones whose use-chain ends only in
          //       calls to @llvm.assumes) should be handled too here.
          // TODO: Update cost model so that ephemeral values are ignored.
          IsVectorizable = isTriviallyVectorizable(IntrinsicId) ||
                           IntrinsicId == Intrinsic::assume;
        } else
          IsVectorizable = TLI.isFunctionVectorizable(Func->getName());
      } else {
        IsVectorizable = false;
      }

      if (!IsVectorizable) {
        Info->setVecType(ParVecInfo::UNKNOWN_CALL);
        Info->setParType(ParVecInfo::UNKNOWN_CALL);
        Info->setLoc(Loc);
        return;
      }
    }

    // If the node is marked as idiom then we don't need to check dd edges.
    if (IdiomList.isIdiom(Inst) != HIRVectorIdioms::NoIdiom)
      return;
  }

  // For all DDREFs
  for (auto Itr = Node->ddref_begin(), End = Node->ddref_end(); Itr != End;
       ++Itr) {
    auto Ref = *Itr;

    auto II = DDG.outgoing_edges_begin(Ref), EE = DDG.outgoing_edges_end(Ref);

    // TODO - Check if we really need to analyze edges for non-livein temps
    assert((Ref->isLval() || !Ref->isConstant() || II == EE) &&
           "Constant DDREF is not expected to have any DD edges");

    // For all outgoing edges.
    for (; II != EE; ++II) {
      const DDEdge *Edge = *II;
      analyze(Ref, Edge);
    }
  }
}

ParVecInfo::ParVecInfo(AnalysisMode Mode, HLLoop *HLoop)
    : HLoop(HLoop), Mode(Mode), ParType(Analyzing), VecType(Analyzing),
      InnerUnknownLoop(nullptr), Switch(nullptr) {
  setLoc(HLoop->getDebugLoc());
}

void ParVecInfo::emitDiag() {
  // Until real diagnostic reporting is used, depend on this internal flag.
  if (!Diag)
    return;

  if (!isEmitMode())
    return;

  // Replace this with real diagnostic reporting.
  print(errs(), false);
}

// Return true if Loop is either a SIMD loop or part of a SIMD loop.
static bool loopInSIMD(HLLoop *Loop) {
  while (Loop) {
    if (Loop->isSIMD())
      return true;
    Loop = Loop->getParentLoop();
  }
  return false;
}

void ParVecInfo::analyze(HLLoop *Loop, TargetLibraryInfo *TLI,
                         HIRDDAnalysis *DDA, HIRSafeReductionAnalysis *SRA) {
  if (Loop->hasCompleteUnrollEnablingPragma()) {
    // Bail out of vectorization if complete unroll requested.
    setVecType(UNROLL_PRAGMA_LOOP);
    emitDiag();
    return;
  }

  // If the loop is marked with novector pragma, set disabling reason and
  // return.
  if (isVectorMode() && Loop->hasVectorizeDisablingPragma()) {
    setVecType(NOVECTOR_PRAGMA_LOOP);
    emitDiag();
    return;
  }

  // DD Analysis is expensive. Be sure to run structural analysis first,
  // i.e., before coming here.
  if (isVectorMode() && loopInSIMD(Loop)) {
    setVecType(SIMD);
    return; // no diag needed
  }

  if (Mode == VectorForVectorizerInnermost && !Loop->isInnermost()) {
    setVecType(FE_DIAG_VEC_NOT_INNERMOST);
    emitDiag();
    return;
  }

  if (!Loop->isDo() && !Loop->isDoMultiExit()) {
    setVecType(NON_DO_LOOP);
    emitDiag();
    return;
  }

  if (!Loop->isNormalized()) {
    setVecType(NON_NORMALIZED_LOOP);
    emitDiag();
    return;
  }

  if (!isDone()) {
    cleanEdges();
    HIRVectorIdioms IList;
    if (isVectorMode()) {
      HIRVectorIdiomAnalysis IdAnalysis;
      IdAnalysis.gatherIdioms(IList, DDA->getGraph(Loop), *SRA, Loop);
    }
    DDWalk DDW(*TLI, *DDA, *SRA, Loop, this, IList); // Legality checker.
    // This ignores preheader/postexit blocks in legality check.
    // This can change isDone() status.
    HLNodeUtils::visitRange(DDW, Loop->child_begin(), Loop->child_end());
  }
  if (isDone()) {
    // Necessary analysis is all complete.
    emitDiag();
    return;
  }

  // DDG check is finished. If not bailing out prior to this point,
  // loop is legal to vectorize, parallelize, or both.
  if (isParallelMode() && ParType == Analyzing) {
    setParType(ParOkay);
  }
  if (isVectorMode() && VecType == Analyzing) {
    setVecType(VecOkay);
  }

  // For non-parallelization, analysis is all done. Returning.
  if (!(Mode == ParallelForThreadizer && ParType == ParOkay)) {
    return; // emitDiag() should be already called.
  }

  // TODO: Continue to Parallelization profitability analysis
}

void ParVecInfo::print(raw_ostream &OS, bool WithLoop) const {
  if (WithLoop) {
    printIndent(OS, false);
    OS << "LoopNode(" << HLoop->getNumber() << ") @ ";
    auto LoopLoc = HLoop->getDebugLoc();
    if (LoopLoc) {
      LoopLoc.print(OS);
    }
    OS << "\n";
  }
  if (isParallelMode()) {
    if (WithLoop) {
      printIndent(OS, true);
    }
    if (ParLoc) {
      ParLoc.print(OS);
    }
    OS << " ";
    // TODO: Par reason strings.
    if (ParType <= SIMD) {
      OS << " Par:[" << LoopTypeString[ParType] << "]\n";
    } else {
      OS << "#" << ParType << ": " << OptReportDiag::getMsg(ParType);
    }
  }
  if (isVectorMode()) {
    if (WithLoop) {
      printIndent(OS, true);
    }
    if (VecLoc) {
      VecLoc.print(OS);
    }
    OS << " ";
    if (VecType <= SIMD) {
      OS << LoopTypeString[VecType];
    } else {
      OS << "#" << VecType << ": " << OptReportDiag::getMsg(VecType);
    }
    OS << "\n";
  }
}

void ParVecInfo::printIndent(raw_ostream &OS, bool ZeroBase) const {
  assert(HLoop && "must be non-NULL\n");
  auto NestLevel = HLoop->getNestingLevel();
  for (unsigned Itr = ZeroBase ? 0 : 1; Itr < NestLevel; Itr++) {
    OS << "  ";
  }
}

const std::string ParVecInfo::LoopTypeString[4] = {
    "analyzing", "loop is parallelizable", "loop is vectorizable",
    "loop has SIMD directive"};

class HIRIdiomAnalyzer final : public HLNodeVisitorBase {
  const DDGraph &DDG;
  HIRSafeReductionAnalysis &SRAnalysis;

  /// Output, list of idioms
  HIRVectorIdioms &IdiomList;
  /// Current loop to analyse.
  HLLoop *Loop;

public:
  HIRIdiomAnalyzer(HIRVectorIdioms &IList, const DDGraph &DDG,
                   HIRSafeReductionAnalysis &SRA, HLLoop *Loop)
      : DDG(DDG), SRAnalysis(SRA), IdiomList(IList), Loop(Loop) {
    SRAnalysis.computeSafeReductionChains(Loop);
  }

  /// \brief Visit all outgoing DDEdges for the given node.
  void visit(HLDDNode *Node);
  void visit(HLSwitch *Switch) {}
  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}
  bool tryMinMaxIdiom(HLDDNode *Node);
  bool tryVConflictIdiom(HLDDNode *Node);
};

// Checks if the given Node is the beginning of Min/Max idiom. If the search
// succeeds, then the Node and its linked instructions are added in IdiomList.
bool HIRIdiomAnalyzer::tryMinMaxIdiom(HLDDNode *Node) {
  auto *MinMaxInst = dyn_cast<HLInst>(Node);
  if (!MinMaxInst)
    return false;

  // First hunt for min/max pattern
  if (!MinMaxInst->isMinOrMax())
    return false;

  // If the instruction is safe reduction then it can't be idiom.
  if (SRAnalysis.getSafeRedInfo(MinMaxInst))
    return false;

  // Get operands and predicate
  const RegDDRef *Operand1 = MinMaxInst->getOperandDDRef(1);
  const RegDDRef *Operand2 = MinMaxInst->getOperandDDRef(2);
  const RegDDRef *Operand3 = MinMaxInst->getOperandDDRef(3);
  const RegDDRef *Operand4 = MinMaxInst->getOperandDDRef(4);
  const RegDDRef *Lval = MinMaxInst->getLvalDDRef();

  // Temporary disable FP data types for primary minmax. For FP data type
  // we need some additional checks to be generated (eg for NAN) which is
  // not implemented yet.
  if (!Lval->getDestType()->isIntegerTy())
    return false;

  PredicateTy Pred = MinMaxInst->getPredicate();

  if (!Lval->isTerminalRef())
    return false;

  bool LvalEqualsThirdOp = DDRefUtils::areEqual(Lval, Operand3);

  // Lval should be involved in minmax idiom.
  if (!LvalEqualsThirdOp && !DDRefUtils::areEqual(Lval, Operand4))
    return false;

  // Supposing the MinMaxInst is in the form
  //  mm = (mm OP b) ? mm : b;
  // For all outgoing edges, check whether sink stmt is in the form
  //  t = (mm OP b) ? t : c;
  // Where t is arbitrary temp, (mm OP b) is the same as in MinMaxInst and
  // the select is in the same order as in MinMaxInst i.e. the condtion is
  // not flipped. That means, 't' in the rhs is in the same position as mm in
  // rhs of MinMaxInst.
  //
  // Actual example where mm is %best.014 and t is %tmp.015.
  // + DO i1 = 0, sext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
  // |   %0 = (@ordering)[0][i1];
  // |   %tmp.015 = (%0 > %best.014) ? i1 : %tmp.015;
  // |   %best.014 = (%0 > %best.014) ? %0 : %best.014;
  // + END LOOP
  //
  //

  LLVM_DEBUG(dbgs() << "[MinMax+Index] Looking at candidate:";
             MinMaxInst->dump());

  MatchFail Mismatch("MinMax+Index");
  DenseMap<HLInst *, HIRVectorIdioms::IdiomId> LinkedInstr;

  for (DDEdge *E : DDG.outgoing(Lval)) {
    if (E->isOutput()) {
      if (E->getSink() != Lval)
        return Mismatch("dependency");
    } else {
      assert(E->isFlow() && "Flow edge expected");

      auto *SinkRef = E->getSink();

      auto *SinkNode = SinkRef->getHLDDNode();

      // Ignore flow edges to same node.
      if (SinkNode == Node)
        continue;

      LLVM_DEBUG(dbgs() << "[MinMax+Index] Depends on:"; SinkNode->dump());

      // Node should be at top level.
      if (SinkNode->getParent() != Loop)
        return Mismatch("in another loop");

      // Only backward edges are allowed.
      if (SinkNode->getTopSortNum() > Node->getTopSortNum())
        return Mismatch("incorrect nodes order");

      auto *Select = dyn_cast<HLInst>(SinkNode);
      if (!Select || !isa<SelectInst>(Select->getLLVMInstruction()))
        return Mismatch("dependency on non-select node");

      auto *SelectLval = Select->getLvalDDRef();
      if (!SelectLval->isTerminalRef())
        return Mismatch("dependency on non-terminal");

      auto *SelectOp1 = Select->getOperandDDRef(1);
      auto *SelectOp2 = Select->getOperandDDRef(2);

      // To make sure that SinkRef is the ref that occurs in the comparison, not
      // some blob embedded in the 3rd or 4th operand.
      if (SinkRef != SelectOp1 && SinkRef != SelectOp2)
        return Mismatch("dependency on comparison operand");

      // This verifies that the two comparison operations are identical.
      if ((Select->getPredicate() != Pred) ||
          !DDRefUtils::areEqual(SelectOp1, Operand1) ||
          !DDRefUtils::areEqual(SelectOp2, Operand2))
        return Mismatch("dependency on not the same comparison");

      HIRVectorIdioms::IdiomId IdiomKind = HIRVectorIdioms::NoIdiom;
      const RegDDRef *Rhs = LvalEqualsThirdOp ? Select->getOperandDDRef(4)
                                              : Select->getOperandDDRef(3);
      StringRef Msg;
      if (!Rhs->isLinear()) {
        IdiomKind = HIRVectorIdioms::MMFirstLastVal;
        Msg = "\n";
      }
      else {
        IdiomKind = HIRVectorIdioms::MMFirstLastIdx;
        // Check that we have assignment of a pure linear value.
        // That means: exclude non-terminals, conversions, and values with
        // division (like i1/4).
        // In the example below the value to check is 'i1'.
        // + DO i1 = 0, sext.i32.i64(%m) + -1, 1 <DO_LOOP> <MAX_TC_EST = 1000>
        // |   %0 = (@ordering)[0][i1];
        // |   %tmp.015 = (%0 > %best.014) ? i1 : %tmp.015;
        // |   %best.014 = (%0 > %best.014) ? %0 : %best.014;
        // + END LOOP
        if (!Rhs->isTerminalRef()) {
          IdiomKind = HIRVectorIdioms::MMFirstLastVal;
          Msg = "(nonterm)\n";
        } else if (Rhs->getSrcType() != Rhs->getDestType()) {
          // TODO: Currently, linears are usually promoted to 64-bit even in
          // source code they are 32-bit. Then in IR we have their truncation
          // 64->32 bit before using them. Our DA does not have capability to
          // promote vector shape through that truncation so we have to bail
          // out here. We need to improve linearity analysis in DA.
          //
          IdiomKind = HIRVectorIdioms::MMFirstLastVal;
          Msg = "(conversion)\n";
        } else if (Rhs->getSingleCanonExpr()->getDenominator() != 1) {
          // Loopopt can declare as linear e.g. i1/3. But we need a monotonic
          // sequence so bail out on any denominator.
          IdiomKind = HIRVectorIdioms::MMFirstLastVal;
          Msg = "(denom)\n";
        } else if (Rhs->getSingleCanonExpr()->isInvariantAtLevel(
                       Loop->getNestingLevel())) {
          // Can be invariant at that level.
          IdiomKind = HIRVectorIdioms::MMFirstLastVal;
          Msg = "(invariant)\n";
        }
      }
      if (DisableNonMonotonicIndexes &&
          IdiomKind != HIRVectorIdioms::MMFirstLastIdx)
        return Mismatch(Twine("nonlinear rhs disabled ") + Msg);

      // This verifies that temp occurs in the rval in the same position as the
      // minmax temp.
      if (LvalEqualsThirdOp) {
        if (!DDRefUtils::areEqual(SelectLval, Select->getOperandDDRef(3)))
          return Mismatch("dependency on not-same-order select");
      } else if (!DDRefUtils::areEqual(SelectLval, Select->getOperandDDRef(4)))
        return Mismatch("dependency on not-same-order select");

      // Need to check whether SinkLVal does not have other dependencies in
      // the loop.
      for (DDEdge *SinkLvalEdge : DDG.outgoing(SelectLval)) {
        auto SinkOtherNode = SinkLvalEdge->getSink()->getHLDDNode();
        if (SinkOtherNode != SinkNode && SinkOtherNode->getParent() == Loop)
          // TODO: Check wether we can allow dependencies between any of
          // gathered instructions (moving this check to out of the loop).
          return Mismatch("other dependency");
      }

      // Add HInst as recognized temp.
      LinkedInstr.insert({Select, IdiomKind});
    }
  }
  if (!LinkedInstr.empty()) {
    // Add Node as idiom.
    IdiomList.addIdiom(MinMaxInst, HIRVectorIdioms::MinOrMax);
    for (auto Linked : LinkedInstr) {
      IdiomList.addLinked(MinMaxInst, Linked.first /* Instruction */,
                          Linked.second /* IdiomKind */);
    }
    LLVM_DEBUG(dbgs() << "[MinMax+Index] Accepted\n");
    return true;
  }
  return false;
}

// The "vconflict idiom" refers to the user code with vector data dependencies
// that can be resolved using vconflict instruction. Particularly, dependencies
// due to possible overlapped indexes in statements like
// a[index] = a[index] OP some_value
// The example of incoming HIR is as follows
// <0>          BEGIN REGION { }
// <15>               + DO i1 = 0, 1023, 1   <DO_LOOP> <nounroll>
// <3>                |   %0 = (%B)[i1];
// <7>                |   %add = (%A)[%0]  +  1.000000e+00;
// <8>                |   (%A)[%0] = %add;
// <15>               + END LOOP
// <0>          END REGION
// In this routine, we detect possible dependencies that can be resolved using
// vconflict. I.e. we check each store if it has memory dependency only on the
// load from the same address.
// There are three possible ways to generate code for vconflict idiom depending
// on OP and some_value classification from above. We don't consider/recognize
// those different kinds of idiom here, the classification is done in VPlan.
bool HIRIdiomAnalyzer::tryVConflictIdiom(HLDDNode *CurNode) {
  auto *StoreInst = dyn_cast<HLInst>(CurNode);
  if (!StoreInst)
    return false;

  MatchFail Mismatch("VConflict Idiom");
  RegDDRef *StoreMemDDRef = StoreInst->getLvalDDRef();
  if (!StoreMemDDRef || !StoreMemDDRef->isMemRef())
    return false;

  LLVM_DEBUG(dbgs() << "[VConflict Idiom] Looking at store candidate:";
             StoreInst->dump());

  // The store address of the above example has two incoming edges:
  // 7:8 (%A)[%0] --> (%A)[%0] ANTI (*) (?)
  // 8:8 (%A)[%0] --> (%A)[%0] OUTPUT (*) (?)
  int AntiDepCnt = 0;
  for (DDEdge *E : DDG.incoming(StoreMemDDRef)) {
    if (E->isOutput()) {
      if (E->getSrc() != StoreMemDDRef)
        return Mismatch(
            "The output dependency is expected to be self-dependency.\n");
    } else {
      DDRef *SrcRef = E->getSrc();
      HLDDNode *SrcNode = SrcRef->getHLDDNode();
      LLVM_DEBUG(dbgs() << "[VConflict Idiom] Depends(WAR) on:";
                 SrcNode->dump());

      if (!E->isAnti())
        return Mismatch("Expected anti-dependency.");

      if (AntiDepCnt >= 1)
        return Mismatch("Too many dependencies.");

      AntiDepCnt++;
      // TODO: Update VConflict search to work with multi-dimensional arrays.
      // For now, we just bail-out.
      if (StoreMemDDRef->getNumDimensions() > 1)
        return Mismatch("Multidimensional arrays are not supported.");

      // Check if both nodes are at top level.
      if (SrcNode->getParent() != Loop)
        return Mismatch("Source is in another loop.");

      // Only backward edges are allowed.
      if (SrcNode->getTopSortNum() > CurNode->getTopSortNum())
        return Mismatch("Nodes are not in the right order.");

      // Check if both source and sink nodes have the same memory reference.
      if (SrcRef->isRval() && DDRefUtils::areEqual(SrcRef, StoreMemDDRef))
        continue;

      return Mismatch("Wrong memory dependency.");
    }
  }

  if (AntiDepCnt == 0 || AntiDepCnt > 1)
    return Mismatch("Store address should have one anti-dependency.");

  LLVM_DEBUG(dbgs() << "[VConflict Idiom] Detected!\n");
  IdiomList.addIdiom(StoreInst, HIRVectorIdioms::VConflict);
  return true;
}

// The routine looks for some idiom patterns. If any is recognized, the Node is
// added into IdiomList tagged with what kind of idiom was recognized.
void HIRIdiomAnalyzer::visit(HLDDNode *Node) {

  if (MinMaxIndexEnabled && tryMinMaxIdiom(Node))
    return;

  if (VConflictIdiomEnabled && tryVConflictIdiom(Node))
    return;

  return;
}

void HIRVectorIdiomAnalysis::gatherIdioms(HIRVectorIdioms &IList,
                                          const DDGraph &DDG,
                                          HIRSafeReductionAnalysis &SRA,
                                          HLLoop *Loop) {
  if (MinMaxIndexEnabled || VConflictIdiomEnabled) {
    HIRIdiomAnalyzer IdiomAnalyzer(IList, DDG, SRA, Loop);
    Loop->getHLNodeUtils().visit(IdiomAnalyzer, Loop);
    LLVM_DEBUG(IList.dump());
  } else
    LLVM_DEBUG(dbgs() << "Any idiom recognition is disabled\n");
}

extern void llvm::loopopt::deleteHIRVectorIdioms(HIRVectorIdioms *p) {
  delete p;
}
