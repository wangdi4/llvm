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

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/IR/Intel_LoopIR/Diag.h"

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
  /// DDA - Data dependency analysis handle.
  HIRDDAnalysis *DDA;

public:
  ParVecVisitor(ParVecInfo::AnalysisMode Mode, HIRDDAnalysis *DDA,
                DenseMap<HLLoop *, ParVecInfo *> &InfoMap)
      : Mode(Mode), InfoMap(InfoMap), DDA(DDA) {}
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
  /// DDG - Data dependency graph.
  DDGraph DDG;
  /// CandidateLoop - current candidate loop.
  HLLoop *CandidateLoop;
  /// Info - Par vec analysis info.
  ParVecInfo *Info;

  /// \brief Analyze one DDEdge for the source node.
  void analyze(const DDEdge *Edge);
  /// \brief Analyze whether the src/sink DDRefs represents privatizable
  /// terminals.
  bool isSimplePrivateTerminal(const RegDDRef *SrcRef, const RegDDRef *SinkRef);
public:
  DDWalk(DDGraph DDG, HLLoop *CandidateLoop, ParVecInfo *Info)
      : DDG(DDG), CandidateLoop(CandidateLoop), Info(Info) {}
  /// \brief Visit all outgoing DDEdges for the given node.
  void visit(HLDDNode *Node);

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

FunctionPass *llvm::createHIRParVecAnalysisPass() { return new HIRParVecAnalysis(); }
char HIRParVecAnalysis::ID = 0;
INITIALIZE_PASS_BEGIN(HIRParVecAnalysis, "hir-parvec-analysis",
                      "HIR Parallel/Vector Candidate Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysis)
INITIALIZE_PASS_END(HIRParVecAnalysis, "hir-parvec-analysis",
                    "HIR Parallel/Vector Candidate Analysis", false, true)

void ParVecVisitor::postVisit(HLLoop *Loop) {
  // Analyze parallelizability/vectorizability if not cached.
  ParVecInfo::get(Mode, InfoMap, DDA, Loop);
}

void ParVecVisitor::visit(HLInst *Node) {
  // Identify HLInst that is not suitable for auto-parallelization
  // and/or auto-vectorization.
  auto LIRInst = Node->getLLVMInstruction();
  if (isa<InvokeInst>(LIRInst) || isa<LandingPadInst>(LIRInst)) {
    auto Loop = Node->getParentLoop();
    DebugLoc Loc = LIRInst->getDebugLoc();
    while (Loop) {
      ParVecInfo::set(Mode, InfoMap, Loop, Mode, ParVecInfo::EH, Loc);
      Loop = Loop->getParentLoop();
    }
  }
}

void HIRParVecAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<HIRDDAnalysis>();
}

bool HIRParVecAnalysis::runOnFunction(Function &F) {
  if (isSIMDEnabledFunction(F)) {
    return false;
  }

  Enabled = true;
  DDA = &getAnalysis<HIRDDAnalysis>();

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
  auto Info = ParVecInfo::get(Mode, InfoMap, DDA, Loop);
  return Info;
}

void HIRParVecAnalysis::analyze(ParVecInfo::AnalysisMode Mode) {
  if (!Enabled) {
    return;
  }
  ParVecVisitor Vis(Mode, DDA, InfoMap);
  HLNodeUtils::visitAllInnerToOuter(Vis);
}

void HIRParVecAnalysis::analyze(ParVecInfo::AnalysisMode Mode,
                                HLRegion *Region) {
  if (!Enabled) {
    return;
  }
  ParVecVisitor Vis(Mode, DDA, InfoMap);
  HLNodeUtils::visitInnerToOuter(Vis, Region);
}

void HIRParVecAnalysis::analyze(ParVecInfo::AnalysisMode Mode, HLLoop *Loop) {
  if (!Enabled) {
    return;
  }
  ParVecVisitor Vis(Mode, DDA, InfoMap);
  HLNodeUtils::visitInnerToOuter(Vis, Loop);
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
  HLNodeUtils::visit(Vis, Region);
}

void HIRParVecAnalysis::forget(HLLoop *Loop, bool Nest) {
  if (!Nest) {
    delete InfoMap[Loop];
    InfoMap[Loop] = nullptr;
    return;
  }
  ParVecForgetVisitor Vis(InfoMap);
  HLNodeUtils::visit(Vis, Loop);
}

void HIRParVecAnalysis::print(raw_ostream &OS, const Module *M) const {
  ParVecPrintVisitor Vis(InfoMap, OS);
  HLNodeUtils::visitAll(Vis);
}

bool HIRParVecAnalysis::isSIMDEnabledFunction(Function &Func) {
  // TODO: ABI related stuff should become part of TargetTransformInfo.
  return Func.getName().startswith("_ZGV");
}

bool DDWalk::isSimplePrivateTerminal(const RegDDRef *SrcRef,
                                     const RegDDRef *SinkRef) {
  // This function deals with terminals.
  if (!(SrcRef && SinkRef && SrcRef->isTerminalRef())) {
    return false;
  }

  HLNode *WriteNode     = SrcRef->isLval() ? SrcRef->getHLDDNode()
                                           : SinkRef->getHLDDNode();
  HLNode *TheOtherNode  = SrcRef->isLval() ? SinkRef->getHLDDNode()
                                           : SrcRef->getHLDDNode();

  if (SrcRef->isLval() && SinkRef->isLval()) {
    // Terminal output dependence is fine, as long as it is not live out
    // (possible dead code) or last value can be correctly computed.
    // If there is a use of the value inside the loop, FLOW or ANTI dependence
    // should exist and it would prevent vectorization/parallelization if
    // not privatizable in that context.
    return true;
  }
  else if (HLNodeUtils::strictlyDominates(WriteNode, TheOtherNode)) {
    // Def strictly dominates Use. Privatizable.
    return true;
  }

  return false;
}

void DDWalk::analyze(const DDEdge *Edge) {
  DEBUG(Edge->dump());

  DDRef *DDref = Edge->getSink();
  if (!HLNodeUtils::contains(CandidateLoop, DDref->getHLDDNode())) {
    DEBUG(dbgs() << "\tis safe to vectorize/parallelize (Sink not in loop)\n");
    if (Edge->isFLOWdep()) {
      // TODO: produce info for liveout information
    }
    return;
  }

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

  if (isSimplePrivateTerminal(dyn_cast<RegDDRef>(Edge->getSrc()),
                              dyn_cast<RegDDRef>(DDref))){
    DEBUG(dbgs() << "\tis safe to vectorize/parallelize (private)\n");
    return;
  }

  // Is this really useful if refineDV() doesn't recompute?
  if (Edge->isRefinableDepAtLevel(NestLevel)) {
    DVectorTy DV;
    bool IsIndep = false;
    if (refineDV(Edge->getSrc(), DDref, NestLevel, 1, DV, &IsIndep)) {
      // TODO: Set Type/Loc. Call emitDiag().
      DEBUG(dbgs() << "\tis unsafe to vectorize/parallelize");
    } else {
      // TODO: Set Type/Loc. Call emitDiag().
      DEBUG(dbgs() << "\tis unsafe to vectorize/parallelize");
    }
    DEBUG(dbgs() << " @ Level " << NestLevel << "\n");
  }
  else {
    DEBUG(dbgs() << "\tis unsafe to vectorize/parallelize\n");
  }
  Info->setVecType(ParVecInfo::FE_DIAG_PAROPT_VEC_VECTOR_DEPENDENCE);
  Info->setParType(ParVecInfo::FE_DIAG_PAROPT_VEC_VECTOR_DEPENDENCE);
}

void DDWalk::visit(HLDDNode *Node) {
  // For all DDREFs
  for (auto Itr = Node->ddref_begin(), End = Node->ddref_end(); Itr != End;
       ++Itr) {
    if ((*Itr)->isConstant()) {
      continue;
    }
    // For all outgoing edges.
    for (auto II = DDG.outgoing_edges_begin(*Itr),
              EE = DDG.outgoing_edges_end(*Itr);
         II != EE; ++II) {
      const DDEdge *Edge = &(*II);
      analyze(Edge);
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

void ParVecInfo::analyze(HLLoop *Loop, HIRDDAnalysis *DDA) {
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
  if (!isDone()) {
    auto DDG = DDA->getGraph(Loop, false);
    cleanEdges();
    DDWalk DDW(DDG, Loop, this);   // Legality checker.
    HLNodeUtils::visit(DDW, Loop); // This can change isDone() status.
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
    if (VecType <= SIMD){
      OS << LoopTypeString[VecType];
    }
    else {
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
    "loop has SIMD directive"
};

