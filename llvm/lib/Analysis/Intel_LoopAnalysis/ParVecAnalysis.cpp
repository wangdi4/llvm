//===--- ParVecAnalysis.cpp - Provides Parallel/Vector Candidate Analysis ---===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Parallel/Vector Candidate Analysis pass.
// It identifies auto-parallelization/auto-vectorization candidate
// loops. For auto-parallelization, this pass will also decide whether
// auto-parallelization should happent to the loop.
//
// Available options:
//   -hir-enable-parvec-diag Enable non-vectorization/non-parallelization
//                           diagnostics from ParVec analyzer itself, for
//                           debugging purposes. Normal reporting happens
//                           w/o this flag when analysis is invoked
//                           in ForThreadizer/ForVectorizer modes.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/ParVecAnalysis.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/IR/Intel_LoopIR/Diag.h"

#include "llvm/Analysis/LoopInfo.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "parvec-analysis"

static cl::opt<bool>
    Diag("hir-enable-parvec-diag", cl::init(false), cl::Hidden,
            cl::desc("Enable non-vectorization/non-parallelization diagnostics from ParVec analyzer"));

namespace {

/// \brief Visitor class to determine parallelizabilty/vectorizability of loops
/// in the nest. In order to make structural analysis work before DD edge walk,
/// be sure to run this visitor class in InnerToOuter walker.
class ParVecVisitor final : public HLNodeVisitorBase {
private:
  ParVecInfo::AnalysisMode Mode;
  DenseMap<HLLoop*, ParVecInfo*> &InfoMap;
  DDAnalysis *DDA;
public:
  ParVecVisitor(ParVecInfo::AnalysisMode Mode, DDAnalysis *DDA,
                DenseMap<HLLoop*, ParVecInfo*> &InfoMap)
    : Mode(Mode), InfoMap(InfoMap), DDA(DDA) {}
  /// \brief Determine parallelizability/vectorizability of the loop
  void postVisit(HLLoop* Loop);
  /// \brief Report instructions that are not suitable for auto-parallelization
  /// and/or auto-vectorization.
  void visit(HLInst* Node);

  // TODO: Add structural analysis for non-rectangular loop nest.
  // TODO: Add structural analysis for HLSwitch.

  /// \brief catch-all visit().
  void visit(HLNode* Node) { };
  /// \brief catch-all postVisit().
  void postVisit(HLNode* Node) { }
};

/// \brief Visitor class to determine parallelizability/vectorizability of
/// the given loop with the given DDG.
class DDWalk final : public HLNodeVisitorBase {
  DDGraph DDG;
  HLLoop *CandidateLoop;
  ParVecInfo *Info;
public:
  DDWalk(DDGraph DDG, HLLoop *CandidateLoop, ParVecInfo *Info)
    : DDG(DDG), CandidateLoop(CandidateLoop), Info(Info) { }
  /// \brief Visit all outgoing DDEdges for the given node.
  void visit(HLDDNode *Node);
  /// \brief Analyze one DDEdge for the source node.
  void analyze(const DDEdge *Edge);

  /// \brief catch-all visit().
  void visit(HLNode *Node) { }
  /// \brief catch-all postVisit().
  void postVisit(HLNode *Node) { }
};

/// \brief Visitor class to invalidate the cached ParVec analysis results.
class ParVecForgetVisitor final : public HLNodeVisitorBase {
  DenseMap<HLLoop*, ParVecInfo*> &InfoMap;
public:
  ParVecForgetVisitor(DenseMap<HLLoop*, ParVecInfo*> &theMap)
    : InfoMap(theMap) {}
  /// \brief Invalidate the cached result.
  void visit(HLLoop* Loop) { delete InfoMap[Loop]; InfoMap[Loop] = nullptr; }

  /// \brief catch-all visit().
  void visit(HLNode* Node) { }
  /// \brief catch-all postVisit().
  void postVisit(HLNode* Node) { }
};

/// \brief Visitor class to print the cached ParVec analysis results.
class ParVecPrintVisitor final : public HLNodeVisitorBase {
  const DenseMap<HLLoop*, ParVecInfo*> &InfoMap;
  raw_ostream &OS;
public:
  ParVecPrintVisitor(const DenseMap<HLLoop*, ParVecInfo*> &theMap,
                     raw_ostream &theOS)
    : InfoMap(theMap), OS(theOS) {}
  /// \brief Print for one loop.
  void visit(HLLoop* Loop) {
    auto Info = InfoMap.lookup(Loop);
    if (Info) {
      Info->print(OS);
    }
  }

  /// \brief catch-all visit().
  void visit(HLNode* Node) { }
  /// \brief catch-all postVisit().
  void postVisit(HLNode* Node) { }
};

} // unnamed namespace

FunctionPass *llvm::createParVecAnalysisPass() { return new ParVecAnalysis(); }
char ParVecAnalysis::ID = 0;
INITIALIZE_PASS_BEGIN(ParVecAnalysis, "hir-parvec-analyze",
                      "Parallel/Vector Candidate Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(DDAnalysis)
INITIALIZE_PASS_END(ParVecAnalysis, "hir-parvec-analyze",
                    "Parallel/Vector Candidate Analysis", false, true)

void ParVecVisitor::postVisit(HLLoop *Loop) {
  // Analyze parallelizability/vectorizability if not cached.
  ParVecInfo::get(Mode, InfoMap, DDA, Loop);
}

void ParVecVisitor::visit(HLInst *Node) {
  // Identify HLInst that is not suitable for auto-parallelization
  // and/or auto-vectorization.
  auto LIRInst = Node->getLLVMInstruction();
  if (isa<InvokeInst>(LIRInst) ||
      isa<LandingPadInst>(LIRInst)){
    auto Loop = Node->getParentLoop();
    DebugLoc Loc = LIRInst->getDebugLoc();
    while (Loop) {
      ParVecInfo::set(Mode, InfoMap, Loop, Mode, ParVecInfo::EH, Loc);
      Loop = Loop->getParentLoop();
    }
  }
}

void ParVecAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<DDAnalysis>();
}

bool ParVecAnalysis::runOnFunction(Function &F) {
  if (isSIMDEnabledFunction(F)) {
    return false;
  }

  Enabled = true;
  DDA = &getAnalysis<DDAnalysis>();

  // ParVecAnalysis runs in on-demand mode. runOnFunction is almost no-op.
  // In the debug mode, run actual analysis in ParallelVector mode, print
  // the result, and releas memory as if nothing happened. "opt -analyze"
  // doesn't print anything.
  DEBUG(analyze(ParVecInfo::ParallelVector));
  DEBUG(print(dbgs()));
  DEBUG(releaseMemory());

  return false;
}

ParVecInfo *ParVecAnalysis::getInfo(ParVecInfo::AnalysisMode Mode, HLLoop *Loop) {
  if (!Enabled) {
    return nullptr;
  }
  auto Info = ParVecInfo::get(Mode, InfoMap, DDA, Loop);
  return Info;
}

void ParVecAnalysis::analyze(ParVecInfo::AnalysisMode Mode) {
  if (!Enabled)
    return;
  ParVecVisitor V(Mode, DDA, InfoMap);
  HLNodeUtils::visitAllInnerToOuter(V);
}

void ParVecAnalysis::analyze(ParVecInfo::AnalysisMode Mode, HLRegion *Region) {
  if (!Enabled)
    return;
  ParVecVisitor V(Mode, DDA, InfoMap);
  HLNodeUtils::visitInnerToOuter(V, Region);
}

void ParVecAnalysis::analyze(ParVecInfo::AnalysisMode Mode, HLLoop *Loop) {
  if (!Enabled)
    return;
  ParVecVisitor V(Mode, DDA, InfoMap);
  HLNodeUtils::visitInnerToOuter(V, Loop);
}

void ParVecAnalysis::releaseMemory() {
  auto end   = InfoMap.end();
  for (auto iter = InfoMap.begin(); iter != end; iter++) {
    delete iter->second;
    iter->second = nullptr;
  }
  InfoMap.clear();
}

void ParVecAnalysis::forget(HLRegion *Region) {
  ParVecForgetVisitor V(InfoMap);
  HLNodeUtils::visit(V, Region);
}

void ParVecAnalysis::forget(HLLoop *Loop, bool Nest) {
  if (!Nest) {
    delete InfoMap[Loop];
    InfoMap[Loop] = nullptr;
    return;
  }
  ParVecForgetVisitor V(InfoMap);
  HLNodeUtils::visit(V, Loop);
}

void ParVecAnalysis::print(raw_ostream &OS, const Module *M) const {
  ParVecPrintVisitor V(InfoMap, OS);
  HLNodeUtils::visitAll(V);
}

bool ParVecAnalysis::isSIMDEnabledFunction(Function &F) {
  // TODO: ABI related stuff should become part of TargetTransformInfo.
  return F.getName().startswith("_ZGV");
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

  // Is this really useful if refineDV() doesn't recompute?
  if (Edge->isRefinableDepAtLevel(NestLevel)) {
    DVectorTy DV;
    bool IsIndep = false;
    if (refineDV(Edge->getSrc(), DDref, NestLevel, 1, DV, &IsIndep)) {
      // TODO: Set Type/Loc. Call emitDiag().
      DEBUG(dbgs() << "\tis unsafe to vectorize/parallelize");
    }
    else {
      // TODO: Set Type/Loc. Call emitDiag().
      DEBUG(dbgs() << "\tis unsafe to vectorize/parallelize");
    }
    DEBUG(dbgs() << " @ Level " << NestLevel);
  }
}

void DDWalk::visit(HLDDNode *Node) {
  // For all DDREFs
  for (auto I = Node->ddref_begin(), E = Node->ddref_end(); I != E; ++I) {
    if ((*I)->isConstant()) {
      continue;
    }
    // For all outgoing edges.
    for (auto II = DDG.outgoing_edges_begin(*I),
         EE = DDG.outgoing_edges_end(*I);                                
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

void ParVecInfo::emitDiag(){
  // Until real diagnostic reporting is used, depend on this internal flag.
  if (!Diag) {
    return;
  }
  if (!isEmitMode()) {
    return;
  }
  // Replace this with real diagnostic reporting.
  print(errs(), false);
}

void ParVecInfo::analyze(HLLoop *Loop, DDAnalysis *DDA){
  // DD Analysis is expensive. Be sure to run structural analysis first,
  // i.e., before coming here.
  if (isVectorMode() && Loop->isSIMD()) {
    setVecType(SIMD);
    return; // no diag needed
  }
  if (Mode == VectorForVectorizerInnermost &&
      !Loop->isInnermost()) {
    setVecType(FE_DIAG_VEC_NOT_INNERMOST);
    emitDiag();
    return;
  }
  if (!isDone()) {
    auto DDG = DDA->getGraph(Loop, false);
    cleanEdges();
    DDWalk DDW(DDG, Loop, this);   // Legality checker.
    HLNodeUtils::visit(DDW, Loop); // This can trigger isDone().
  }
  if (!isDone()) {
    if (isParallelMode() && ParType == Analyzing)
      setParType(ParOkay);
    if (isVectorMode() && VecType == Analyzing)
      setVecType(VecOkay);
  }
  if (!(Mode == ParallelForThreadizer && ParType == ParOkay)) {
    // For non-parallelization, analysis is all done. Returning.
    return; // emitDiag() should be already called.
  }
  // TODO: Continue to Parallelization profitability analysis
}

void ParVecInfo::print(raw_ostream &OS, bool WithLoop){
  if (WithLoop) {
    printIndent(OS, false);
    OS << "LoopNode(" << HLoop->getNumber() << ") @ ";
    auto LoopLoc = HLoop->getLLVMLoop()->getStartLoc();
    if (LoopLoc) { LoopLoc.print(OS); }
    OS << "\n";
  }
  if (isParallelMode()) {
    if (WithLoop) {
      printIndent(OS, true);
    }
    if (ParLoc) { ParLoc.print(OS); }
    // TODO: Par reason strings.
    OS << " Par:[" << LoopTypeString[ParType] << "]\n";
  }
  if (isVectorMode()) {
    if (WithLoop) {
      printIndent(OS, true);
    }
    if (VecLoc) { VecLoc.print(OS); }
    OS << " ";
    if (VecType <= SIMD){ OS << LoopTypeString[VecType]; }
    else OS << "#" << VecType << ": "
	    << OptReportDiag::getMsg(VecType);
    OS << "\n";
  }
}

void ParVecInfo::printIndent(raw_ostream &OS, bool ZeroBase){
  assert(HLoop && "must be non-NULL\n");
  auto NestLevel = HLoop->getNestingLevel();
  for (unsigned i=ZeroBase ? 0 : 1;i<NestLevel;i++){
    OS << "  ";
  }
}

const std::string ParVecInfo::LoopTypeString[4] = {
  "analyzing", "loop is parallelizable", "loop is vectorizable",
  "loop has SIMD directive"
};

