//===-- HIRParVecAnalysis.cpp ---------------------------------------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/HIRParVecAnalysis.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/IR/Intel_LoopIR/Diag.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#include "llvm/Analysis/LoopInfo.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "parvec-analysis"

static cl::opt<bool>
    Diag("hir-enable-parvec-diag", cl::init(false), cl::Hidden,
         cl::desc("Enable non-vectorization/non-parallelization diagnostics "
                  "from ParVec analyzer"));

namespace {

/// \brief Visitor class to determine parallelizabilty/vectorizability of loops
/// in the nest. In order to make structural analysis work before DD edge walk,
/// be sure to run this visitor class in InnerToOuter walker.
class ParVecVisitor final : public HLNodeVisitorBase {
private:
  /// Mode - curent analysis mode running, auto-par or auto-vec.
  ParVecInfo::AnalysisMode Mode;
  /// InfoMap - Map associating HLLoops with corresponding par vec info.
  DenseMap<HLLoop *, ParVecInfo *> &InfoMap;
  /// TLI - Target library info analysis.
  TargetLibraryInfo *TLI;
  /// DDA - Data dependency analysis handle.
  HIRDDAnalysis *DDA;
  /// SRA - Safe reduction analysis handle
  HIRSafeReductionAnalysis *SRA;

public:
  ParVecVisitor(ParVecInfo::AnalysisMode Mode, TargetLibraryInfo *TLI,
                HIRDDAnalysis *DDA, HIRSafeReductionAnalysis *SRA,
                DenseMap<HLLoop *, ParVecInfo *> &InfoMap)
      : Mode(Mode), InfoMap(InfoMap), TLI(TLI), DDA(DDA), SRA(SRA) {}
  /// \brief Determine parallelizability/vectorizability of the loop
  void postVisit(HLLoop *Loop);
  /// \brief Report instructions that are not suitable for auto-parallelization
  /// and/or auto-vectorization.
  void visit(HLInst *Node);

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

  /// \brief Analyze one DDEdge for the source node.
  void analyze(const RegDDRef *SrcRef, const DDEdge *Edge);

  /// \brief Analyze whether the src/sink flow dependence can be ignored
  /// due to safe reduction.
  bool isSafeReductionFlowDep(const RegDDRef *SrcRef, const RegDDRef *SinkRef);

public:
  DDWalk(TargetLibraryInfo &TLI, HIRDDAnalysis &DDA,
         HIRSafeReductionAnalysis &SRA, HLLoop *CandidateLoop, ParVecInfo *Info)
      : TLI(TLI), DDA(DDA), SRA(SRA), DDG(DDA.getGraph(CandidateLoop, false)),
        CandidateLoop(CandidateLoop), Info(Info), ComputedSafeRedn(false) {}

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

/// \brief Visitor class to invalidate the cached ParVec analysis results.
class ParVecForgetVisitor final : public HLNodeVisitorBase {
  DenseMap<HLLoop *, ParVecInfo *> &InfoMap;

public:
  ParVecForgetVisitor(DenseMap<HLLoop *, ParVecInfo *> &theMap)
      : InfoMap(theMap) {}
  /// \brief Invalidate the cached result.
  void visit(HLLoop *Loop) {
    delete InfoMap[Loop];
    InfoMap[Loop] = nullptr;
  }

  /// \brief catch-all visit().
  void visit(HLNode *Node) {}
  /// \brief catch-all postVisit().
  void postVisit(HLNode *Node) {}
};

/// \brief Visitor class to print the cached ParVec analysis results.
class ParVecPrintVisitor final : public HLNodeVisitorBase {
  const DenseMap<HLLoop *, ParVecInfo *> &InfoMap;
  raw_ostream &OS;

public:
  ParVecPrintVisitor(const DenseMap<HLLoop *, ParVecInfo *> &theMap,
                     raw_ostream &theOS)
      : InfoMap(theMap), OS(theOS) {}
  /// \brief Print for one loop.
  void visit(HLLoop *Loop) {
    auto Info = InfoMap.lookup(Loop);
    if (Info)
      Info->print(OS);
  }

  /// \brief catch-all visit().
  void visit(HLNode *Node) {}
  /// \brief catch-all postVisit().
  void postVisit(HLNode *Node) {}
};

} // unnamed namespace

FunctionPass *llvm::createHIRParVecAnalysisPass() {
  return new HIRParVecAnalysis();
}
char HIRParVecAnalysis::ID = 0;
INITIALIZE_PASS_BEGIN(HIRParVecAnalysis, "hir-parvec-analysis",
                      "HIR Parallel/Vector Candidate Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysis)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysis)
INITIALIZE_PASS_END(HIRParVecAnalysis, "hir-parvec-analysis",
                    "HIR Parallel/Vector Candidate Analysis", false, true)

void ParVecVisitor::postVisit(HLLoop *Loop) {
  // Analyze parallelizability/vectorizability if not cached.
  ParVecInfo::get(Mode, InfoMap, TLI, DDA, SRA, Loop);
}

void ParVecVisitor::visit(HLInst *Node) {
  // Identify HLInst that is not suitable for auto-parallelization
  // and/or auto-vectorization.
  auto LIRInst = Node->getLLVMInstruction();
  ParVecInfo::LoopType Type = ParVecInfo::Analyzing;

  if (isa<InvokeInst>(LIRInst) || isa<LandingPadInst>(LIRInst)) {
    Type = ParVecInfo::EH;

  } else if (auto Call = dyn_cast<CallInst>(LIRInst)) {
    auto Func = Call->getCalledFunction();

    if (!Func || !TLI->isFunctionVectorizable(Func->getName())) {
      Type = ParVecInfo::UNKNOWN_CALL;
    }
  }

  if (Type != ParVecInfo::Analyzing) {
    auto Loop = Node->getParentLoop();
    DebugLoc Loc = LIRInst->getDebugLoc();
    while (Loop) {
      ParVecInfo::set(Mode, InfoMap, Loop, Mode, Type, Loc);
      Loop = Loop->getParentLoop();
    }
  }
}

void HIRParVecAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<TargetLibraryInfoWrapperPass>();
  AU.addRequiredTransitive<HIRFramework>();
  AU.addRequiredTransitive<HIRDDAnalysis>();
  AU.addRequiredTransitive<HIRSafeReductionAnalysis>();
}

bool HIRParVecAnalysis::runOnFunction(Function &F) {
  if (isSIMDEnabledFunction(F)) {
    return false;
  }

  Enabled = true;
  TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
  HIRF = &getAnalysis<HIRFramework>();
  DDA = &getAnalysis<HIRDDAnalysis>();
  SRA = &getAnalysis<HIRSafeReductionAnalysis>();

  // ParVecAnalysis runs in on-demand mode. runOnFunction is almost no-op.
  // In the debug mode, run actual analysis in ParallelVector mode, print
  // the result, and releas memory as if nothing happened. "opt -analyze"
  // doesn't print anything.
  DEBUG(analyze(ParVecInfo::ParallelVector));
  DEBUG(print(dbgs()));
  DEBUG(releaseMemory());

  return false;
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
  HIRF->getHLNodeUtils().visitAllInnerToOuter(Vis);
}

void HIRParVecAnalysis::analyze(ParVecInfo::AnalysisMode Mode,
                                HLRegion *Region) {
  if (!Enabled) {
    return;
  }
  ParVecVisitor Vis(Mode, TLI, DDA, SRA, InfoMap);
  HIRF->getHLNodeUtils().visitInnerToOuter(Vis, Region);
}

void HIRParVecAnalysis::analyze(ParVecInfo::AnalysisMode Mode, HLLoop *Loop) {
  if (!Enabled) {
    return;
  }
  ParVecVisitor Vis(Mode, TLI, DDA, SRA, InfoMap);
  HIRF->getHLNodeUtils().visitInnerToOuter(Vis, Loop);
}

void HIRParVecAnalysis::releaseMemory() {
  for (auto Iter = InfoMap.begin(), End = InfoMap.end(); Iter != End; Iter++) {
    delete Iter->second;
    Iter->second = nullptr;
  }
  InfoMap.clear();
}

void HIRParVecAnalysis::forget(HLRegion *Region) {
  ParVecForgetVisitor Vis(InfoMap);
  HIRF->getHLNodeUtils().visit(Vis, Region);
}

void HIRParVecAnalysis::forget(HLLoop *Loop, bool Nest) {
  if (!Nest) {
    delete InfoMap[Loop];
    InfoMap[Loop] = nullptr;
    return;
  }
  ParVecForgetVisitor Vis(InfoMap);
  HIRF->getHLNodeUtils().visit(Vis, Loop);
}

void HIRParVecAnalysis::print(raw_ostream &OS, const Module *M) const {
  ParVecPrintVisitor Vis(InfoMap, OS);
  HIRF->getHLNodeUtils().visitAll(Vis);
}

bool HIRParVecAnalysis::isSIMDEnabledFunction(Function &Func) {
  // TODO: ABI related stuff should become part of TargetTransformInfo.
  return Func.getName().startswith("_ZGV");
}

bool DDWalk::isSafeReductionFlowDep(const RegDDRef *SrcRef,
                                    const RegDDRef *SinkRef) {
  assert(SrcRef && "SrcRef cannot be null!");

  if (!SinkRef) {
    return false;
  }

  HLNode *WriteNode = SrcRef->getHLDDNode();
  auto Inst = dyn_cast<HLInst>(WriteNode);

  if (!Inst) {
    return false;
  }

  if (!ComputedSafeRedn) {
    SRA.computeSafeReductionChains(CandidateLoop);
    ComputedSafeRedn = true;
  }

  auto SRI = SRA.getSafeRedInfo(Inst);

  // The vectorizer currently cannot handle min/max reductions, they are
  // therefore suppressed
  if (SRI) {
    if (SRI->OpCode == Instruction::Select) {
      return false;
    } else {
      return true;
    }
  }

  return false;
}

void DDWalk::analyze(const RegDDRef *SrcRef, const DDEdge *Edge) {
  DEBUG(Edge->dump());

  unsigned NestLevel = CandidateLoop->getNestingLevel();
  if (!Edge->preventsParallelization(NestLevel)) {
    DEBUG(dbgs() << "\tis safe to vectorize/parallelize\n");
    return;
  }
  if (Info->isVectorMode()) {
    if (!Edge->preventsVectorization(NestLevel)) {
      DEBUG(dbgs() << "\tis safe to vectorize but unsafe to parallelize\n");
      // TODO: Set ParType/ParLoc. Call emitDiag().
      return;
    }
  }

  if (SrcRef && SrcRef->isTerminalRef() &&
      !CandidateLoop->isLiveIn(SrcRef->getSymbase())) {
    DEBUG(dbgs() << "\tis safe to vectorize/parallelize (private)\n");
    return;
  }

  DDRef *SinkRef = Edge->getSink();

  if (Edge->isFLOWdep() &&
      isSafeReductionFlowDep(SrcRef, dyn_cast<RegDDRef>(SinkRef))) {
    DEBUG(dbgs() << "\tis safe to vectorize/parallelize (safe reduction)\n");
    return;
  }

  // Is this really useful if refineDV() doesn't recompute?
  if (Edge->isRefinableDepAtLevel(NestLevel)) {
    DirectionVector DV;
    DistanceVector DistV;

    bool IsIndep = false;
    if (DDA.refineDV(Edge->getSrc(), SinkRef, NestLevel, 1, DV, DistV,
                     &IsIndep)) {
      // TODO: Set Type/Loc. Call emitDiag().
      DEBUG(dbgs() << "\tis unsafe to vectorize/parallelize");
    } else {
      // TODO: Set Type/Loc. Call emitDiag().
      DEBUG(dbgs() << "\tis unsafe to vectorize/parallelize");
    }
    DEBUG(dbgs() << " @ Level " << NestLevel << "\n");
  } else {
    DEBUG(dbgs() << "\tis unsafe to vectorize/parallelize\n");
  }
  Info->setVecType(ParVecInfo::FE_DIAG_PAROPT_VEC_VECTOR_DEPENDENCE);
  Info->setParType(ParVecInfo::FE_DIAG_PAROPT_VEC_VECTOR_DEPENDENCE);
}

void DDWalk::visit(HLDDNode *Node) {

  if (auto Inst = dyn_cast<HLInst>(Node)) {
    if (auto Call = dyn_cast<CallInst>(Inst->getLLVMInstruction())) {
      auto Func = Call->getCalledFunction();

      if (!Func || !TLI.isFunctionVectorizable(Func->getName())) {
        Info->setVecType(ParVecInfo::UNKNOWN_CALL);
        Info->setParType(ParVecInfo::UNKNOWN_CALL);
        return;
      }
    }
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
  setLoc(HLoop->getLLVMLoop()->getStartLoc());
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

void ParVecInfo::analyze(HLLoop *Loop, TargetLibraryInfo *TLI,
                         HIRDDAnalysis *DDA, HIRSafeReductionAnalysis *SRA) {
  // DD Analysis is expensive. Be sure to run structural analysis first,
  // i.e., before coming here.
  if (isVectorMode() && Loop->isSIMD()) {
    setVecType(SIMD);
    return; // no diag needed
  }

  if (Mode == VectorForVectorizerInnermost && !Loop->isInnermost()) {
    setVecType(FE_DIAG_VEC_NOT_INNERMOST);
    emitDiag();
    return;
  }

  if (!Loop->isDo()) {
    setVecType(NON_DO_LOOP);
    emitDiag();
    return;
  }

  if (!isDone()) {
    cleanEdges();
    DDWalk DDW(*TLI, *DDA, *SRA, Loop, this); // Legality checker.
    Loop->getHLNodeUtils().visit(DDW, Loop); // This can change isDone() status.
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
    auto LoopLoc = HLoop->getLLVMLoop()->getStartLoc();
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
    // TODO: Par reason strings.
    OS << " Par:[" << LoopTypeString[ParType] << "]\n";
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
