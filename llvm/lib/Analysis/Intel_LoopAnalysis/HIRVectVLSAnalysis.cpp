//===---- HIRVLAAnalysis.cpp - Computes VLS Analysis ---------------------===//
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
/// Implement a test VLS-client to exercise the functionality of the
/// HIRVLSClient routines.
///
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"

#include "llvm/IR/Type.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRVLSClient.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRVectVLSAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-vect-vls-analysis"

// Primarily used for debugging purpose.
// We do not print any information except DEBUG mode.
static cl::opt<bool> debugHIRVectVLS(
    "hir-debug-vect-vls", cl::init(false), cl::Hidden,
    cl::desc("Pre-computes vls information for all loops inside function."));

FunctionPass *llvm::createHIRVectVLSAnalysisPass() {
  return new HIRVectVLSAnalysis();
}

char HIRVectVLSAnalysis::ID = 0;

INITIALIZE_PASS_BEGIN(HIRVectVLSAnalysis, "hir-vect-vls-analysis",
                      "HIR Vect VLS Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(DDAnalysis)
INITIALIZE_PASS_END(HIRVectVLSAnalysis, "hir-vect-vls-analysis",
                    "HIR Vect VLS Analysis", false, true)

void HIRVectVLSAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {

  AU.setPreservesAll();
  AU.addRequired<HIRFramework>();
  AU.addRequired<DDAnalysis>();
}

void HIRVectVLSAnalysis::releaseMemory() {}

void HIRVectVLSAnalysis::print(raw_ostream &OS, const Module *M) const {
  // TODO: Go over all innerloops in function and print the groups of
  // neighboring Memrefs that were found, maybe calling
  // OptVLS::dumpOVLSGroupVector(OS, *OVLSGrps);
  // TODO: For this we need to record all the Groups in a mapping from Loops to
  // Groups.
}

void HIRVectVLSAnalysis::markLoopBodyModified(const HLLoop *L) {

  assert(L && " Loop parameter is null.");
  // Mark loop as modified.
  // TODO. Currently no information is recorded so nothing to invalidate.
}

// Test the VLS-Client utilities directly, without going through the VLS
// engine (without creating VLSMemrefs).
void HIRVectVLSAnalysis::testVLSMemrefAnalysis(VectVLSContext *VectContext,
                                               LoopMemrefsVector &RefVec) {

  unsigned Level = VectContext->getLoopLevel();
  DEBUG(dbgs() << "\nVLS: Examining level " << Level);

  for (auto VecIt = RefVec.begin(), End = RefVec.end(); VecIt != End; ++VecIt) {

    RegDDRef *Ref = *VecIt;
    DEBUG(dbgs() << "\nExamine Ref "; Ref->dump());

    // Is it Strided at Level?
    CanonExpr *Stride = Ref->getStrideAtLevel(Level);
    bool strided = false;
    int64_t Val;
    if (Stride) {
      //      DEBUG(dbgs() << "\n  Stride at Level is "; Stride->dump(1));
      if (Stride->isIntConstant(&Val)) {
        if (Val) {
          strided = true;
          DEBUG(dbgs() << "   Strided Access at Level " << Level
                       << ". Constant Stride = " << Val << ".\n");
        }
        //        else
        //          DEBUG(dbgs() << "   Zero Stride.\n");
      } else {
        strided = true;
        DEBUG(dbgs() << "   Strided Access at Level " << Level
                     << ". Non Constant Stride.\n");
      }
    }

    if (!strided) {
      // TODO: Is it an indexed mem access....?
      //      DEBUG(dbgs() << "\n  Access not Strided.\n");
    }

    // Check relations between pairs of ddrefs
    for (auto VecIt2 = RefVec.begin() + 1, End = RefVec.end(); VecIt2 != End;
         ++VecIt2) {
      RegDDRef *Ref2 = *VecIt2;
      int64_t Distance;
      DEBUG(dbgs() << "\n   Compare with Ref "; Ref2->dump());

      // Compute Distances
      if (DDRefUtils::getConstDistance(Ref, Ref2, &Distance)) {
        DEBUG(dbgs() << "     Distance  = " << Distance << ".\n");
      } else {
        DEBUG(dbgs() << "     Could not compute Distance.\n");
      }

      // CanMoveTo?
      if (HIRVLSClientMemref::canAccessWith(Ref, Ref2, VectContext)) {
        DEBUG(dbgs() << "     Can move to location of ref2\n");
      } else {
        DEBUG(dbgs() << "     Illegal to move to location of ref2\n");
      }
    }
  }
}

// The ClientMemrefs we create here hold information that is relevant under
// VectContext, e.g. the current loop level that is being considered for
// vectorization. We initially create the ClientMemrefs without setting the
// NumberOfElements (this will be set later per VF).
void HIRVectVLSAnalysis::getVLSMemrefs(VectVLSContext *VectContext,
                                       LoopMemrefsVector &RefVec,
                                       OVLSMemrefVector &Mrfs) {

  for (auto &Ref : RefVec) {
    // DEBUG(dbgs() << "\nExamine Ref "; Ref->dump());

    // FIXME: These 'new' operations in the loop are expensive and unnecessary.
    // The OVLSMemrefVector type should be changed to store objects rather than
    // pointers.
    HIRVLSClientMemref *Mrf = new HIRVLSClientMemref(Ref, VectContext);
    if (!Mrf->setStridedAccess()) {
      // TODO: Try indexed
      // FORNOW: Access Remains unknown.
    }
    Mrfs.push_back((OVLSMemref *)Mrf);
  }
}

void HIRVectVLSAnalysis::computeVLSGroups(const OVLSMemrefVector &Memrefs,
                                          VectVLSContext *VectContext,
                                          OVLSGroupVector &Grps) {

  unsigned GroupSize = MAX_VECTOR_LENGTH; // CHECKME
  unsigned Level = VectContext->getLoopLevel();
  unsigned VF = VectContext->getVectFactor();
  DEBUG(dbgs() << "\nVLS: Examining level " << Level);
  DEBUG(dbgs() << " with NumElements(VF) " << VF << "\n");

  // FIXME: Need to decide if we want NumElements (currently a member field
  // of the base class OVLSMemref) to be a property of the vectorization context
  // (that is, same NumElements (=VF) to all Memrefs under this context) rather
  // than a property of an individual Memref. This would require changing the
  // Base class, allowing clients to override getNumElements.
  // For now leaving it as is. When we add SLP-aware support, we may
  // want the flexibility to use different numElements than VF (?).
  for (unsigned I = 0, Size = Memrefs.size(); I < Size; ++I) {
    OVLSMemref *Mrf = Memrefs[I];
    HIRVLSClientMemref *CLMrf = (HIRVLSClientMemref *)(Mrf);
    assert(CLMrf != NULL);
    CLMrf->setNumElements(VF);
  }

  OptVLSInterface::getGroups(Memrefs, Grps, GroupSize);
}

class VectVLSDDRefVisitor final : public HLNodeVisitorBase {
private:
  unsigned LoopLevel;
  LoopMemrefsVector &LoopMemrefs;

public:
  VectVLSDDRefVisitor(unsigned LoopLevel, LoopMemrefsVector &LoopMemrefs)
      : LoopLevel(LoopLevel), LoopMemrefs(LoopMemrefs) {}

  void visit(const HLInst *I);
  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
};

// Gather memrefs from Node and add them to LoopMemrefs.
void VectVLSDDRefVisitor::visit(const HLInst *Inst) {

  // Using op_ddref iterator because we do not want to optimize fake refs.
  for (auto I = Inst->op_ddref_begin(), E = Inst->op_ddref_end(); I != E; ++I) {
    RegDDRef *RegRef = *I;
    if (!RegRef->isMemRef() ||
        HIRVLSClientMemref::isInvariantAtLevel(RegRef, LoopLevel)) {
      continue;
    }
    LoopMemrefs.push_back((RegRef));
  }
}

// Only used when debugHIRVectVLS is on.
// Just stress testing roughly emulating the vectorizer usage case.
void HIRVectVLSAnalysis::analyzeVLSInLoop(const HLLoop *Loop) {

  unsigned Level = Loop->getNestingLevel();
  HLLoop *Loop2 = const_cast<HLLoop *>(Loop);
  DDGraph DDG = DDA->getGraph(Loop2, false);
  // DEBUG(DDG.dump());

  // 1. Gather MemRefs in Loop
  LoopMemrefsVector LoopMemrefs;
  VectVLSDDRefVisitor V(Level, LoopMemrefs);
  HLNodeUtils::visit(V, Loop);

  // 2. Identify VLS groups -- groups of neighbouring LoopMemrefs.
  // Do this for each candidate Vectorization Factor (VF). The VF affects
  // which dependence distances we can ignore when we examine which memrefs can
  // be colocated in the same group during the grouping process.
  // It also affects the NumElements member of all the Memrefs.
  // TODO: Consideration of dependence distance no yet implemeneted.
  VectVLSContext VectContext(DDG, Level);
#if 0
  testVLSMemrefAnalysis(&VectContext, LoopMemrefs);
#endif
  OVLSMemrefVector LoopVLSMrfs;
  getVLSMemrefs(&VectContext, LoopMemrefs, LoopVLSMrfs);

  // To roughly emulate the vectorizer driver we would want to apply the
  // following for each candidate VF. Here we use only one VF.
  // for (int VF = 2; VF < MAX_GROUP_SIZE; VF *= 2) {
  {
    int VF = 4;
    VectContext.setCurrentVF(VF);

    OVLSGroupVector Grps;
    computeVLSGroups(LoopVLSMrfs, &VectContext, Grps);

    // Do something with Grps

    for (OVLSGroup *Grp : Grps) {
      delete Grp;
    }
  }

  for (OVLSMemref *Memref : LoopVLSMrfs) {
    delete Memref;
  }
}

// Only used when debugHIRVectVLS is on.
// Just stress testing roughly emulating the vectorizer usage case.
// FORNOW: Gather the innermost loops as candidates.
// TODO: Do this for vectorization candidate loops.
// TODO: Replace with InnerToOuter Walk of Vectorization candidate loop Nests?
void HIRVectVLSAnalysis::analyze() {

  SmallVector<const HLLoop *, 64> CandidateLoops;
  HLNodeUtils::gatherInnermostLoops(CandidateLoops);
  for (auto &Lp : CandidateLoops) {
    analyzeVLSInLoop(Lp);
  }
}

// Performs a basic setup without actually running the VLS analysis.
bool HIRVectVLSAnalysis::runOnFunction(Function &F) {

  DDA = &getAnalysis<DDAnalysis>();

  if (debugHIRVectVLS) {
    analyze();
  }

  return false;
}
