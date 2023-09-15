//===- HIRUndoSinkingForPerfectLoopnest.cpp Implements
// UndoSinkingForPerfectLoopnest
// class -===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass recognizes the pattern to unsink the instructions that were not
// used in loop interchange and loop blocking.
//
// We are transforming this-
//
//  DO i1 = 0, 999
//    DO i2 = 0, 999, 1   <DO_LOOP>
//      (%c)[0][i1][i2] = 0.000000e+00;
//    ENDDO
//  ENDDO
//
//  DO i1 = 0, 999
//    DO i2 = 0, 999, 1   <DO_LOOP>
//      DO i3 = 0, 999, 1   <DO_LOOP>
//        %add = c[i1][i2];
//        %mul = (%a)[0][i1][i3]  *  (%b)[0][i3][i2];
//        %add = %add  +  %mul;
//       (%c)[0][i1][i2] = %add;
//      END LOOP
//   END LOOP
// END LOOP
//
// To-
//
//  DO i1 = 0, 999
//    DO i2 = 0, 999, 1   <DO_LOOP>
//      (%c)[0][i1][i2] = 0.000000e+00;
//    ENDDO
//  ENDDO
//
//  DO i1 = 0, 999
//    DO i2 = 0, 999, 1   <DO_LOOP>
//      %add = 0.000000e+00;
//      DO i3 = 0, 999, 1   <DO_LOOP>
//        %add = c[i1][i2];
//        %mul = (%a)[0][i1][i3]  *  (%b)[0][i3][i2];
//        %add = %add  +  %mul;
//      END LOOP
//      (%c)[0][i1][i2] = %add;
//   END LOOP
// END LOOP
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRUndoSinkingForPerfectLoopnestPass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"

#define OPT_SWITCH "hir-undo-sinking-for-perfect-loopnest"
#define OPT_DESC "HIR Undo Sinking For Perfect Loopnest"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

namespace {

class HIRUndoSinkingForPerfectLoopnest {
  HIRFramework &HIRF;
  HIRDDAnalysis &DDA;
  struct MatchingStoreFinder;

public:
  HIRUndoSinkingForPerfectLoopnest(HIRFramework &HIRF, HIRDDAnalysis &DDA)
      : HIRF(HIRF), DDA(DDA) {}

  bool run();

private:
  HLLoop *getPrevSiblingLoop(HLNode *Node, RegDDRef *LoadRef,
                             bool &ShouldBailOut);

  HLLoop *findCandidateSiblingLoop(HLLoop *Lp, RegDDRef *LoadRef);
};
} // namespace

static bool haveSameLoopBounds(HLInst *CopyInst, HLInst *SinkedLoadInst) {
  HLLoop *MatchingStoreLp = CopyInst->getLexicalParentLoop();
  unsigned StoreNodeLevel = MatchingStoreLp->getNestingLevel();
  HLLoop *SinkedLoadLp = SinkedLoadInst->getLexicalParentLoop();
  unsigned LoadNodeLevel = SinkedLoadLp->getNestingLevel();

  if (StoreNodeLevel != LoadNodeLevel - 1) {
    return false;
  }

  SinkedLoadLp = SinkedLoadLp->getParentLoop();
  auto LCALoop =
      HLNodeUtils::getLowestCommonAncestorLoop(MatchingStoreLp, SinkedLoadLp);

  while (SinkedLoadLp != LCALoop) {
    RegDDRef *LowerDDRef = SinkedLoadLp->getLowerDDRef();
    RegDDRef *SiblingLowerDDRef = MatchingStoreLp->getLowerDDRef();
    RegDDRef *UpperDDRef = SinkedLoadLp->getUpperDDRef();
    RegDDRef *SiblingUpperDDRef = MatchingStoreLp->getUpperDDRef();
    RegDDRef *StrideDDRef = SinkedLoadLp->getStrideDDRef();
    RegDDRef *SiblingStrideDDRef = MatchingStoreLp->getStrideDDRef();

    if (!DDRefUtils::areEqual(LowerDDRef, SiblingLowerDDRef) ||
        !DDRefUtils::areEqual(UpperDDRef, SiblingUpperDDRef) ||
        !DDRefUtils::areEqual(StrideDDRef, SiblingStrideDDRef)) {
      return false;
    }

    SinkedLoadLp = SinkedLoadLp->getParentLoop();
    MatchingStoreLp = MatchingStoreLp->getParentLoop();
  }

  return true;
}

struct HIRUndoSinkingForPerfectLoopnest::MatchingStoreFinder final
    : public HLNodeVisitorBase {
  HLInst *SinkedLoadInst;
  HLInst *&CopyInst;
  bool IsDone;

  MatchingStoreFinder(HLInst *SinkedLoadInst, HLInst *&CopyInst)
      : SinkedLoadInst(SinkedLoadInst), CopyInst(CopyInst), IsDone(false) {}

  void visit(HLDDNode *Node) {
    HLInst *HInst = dyn_cast<HLInst>(Node);

    if (!HInst) {
      return;
    }

    auto Inst = HInst->getLLVMInstruction();

    if (HInst->isCallInst()) {
      IsDone = true;
      return;
    }

    auto *LvalRef = HInst->getLvalDDRef();
    auto *RvalRef = HInst->getRvalDDRef();

    if (!LvalRef) {
      return;
    }

    if (!RvalRef) {
      return;
    }

    if (!RvalRef->isConstant()) {
      return;
    }

    if (!isa<StoreInst>(Inst) ||
        !DDRefUtils::areEqual(LvalRef, SinkedLoadInst->getRvalDDRef())) {
      if (LvalRef->getSymbase() ==
          SinkedLoadInst->getRvalDDRef()->getSymbase()) {
        IsDone = true;
      }
      return;
    }

    if (!HLNodeUtils::strictlyDominates(Node, SinkedLoadInst)) {
      IsDone = true;
      return;
    }

    if (!haveSameLoopBounds(HInst, SinkedLoadInst)) {
      IsDone = true;
      return;
    }

    auto &HNU = Node->getHLNodeUtils();
    RegDDRef *RHS = RvalRef->clone();
    RegDDRef *LHS = SinkedLoadInst->getLvalDDRef()->clone();
    CopyInst = HNU.createCopyInst(RHS, "copy", LHS);
    IsDone = true;
  }

  void visit(HLGoto *Goto) { IsDone = true; }
  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}
  bool isDone() const { return IsDone; }
};

HLLoop *HIRUndoSinkingForPerfectLoopnest::getPrevSiblingLoop(
    HLNode *Node, RegDDRef *LoadRef, bool &ShouldBailOut) {

  HLLoop *SiblingLp = nullptr;

  while ((Node = Node->getPrevNode())) {
    if ((SiblingLp = dyn_cast<HLLoop>(Node))) {
      break;
    } else if (auto *Inst = dyn_cast<HLInst>(Node)) {
      if (Inst->isCallInst()) {
        ShouldBailOut = true;
      }

      if (auto *LvalRef = Inst->getLvalDDRef()) {
        if (LvalRef->getSymbase() == LoadRef->getSymbase()) {
          ShouldBailOut = true;
        }
      }
    } else {
      ShouldBailOut = true;
    }

    if (ShouldBailOut) {
      break;
    }
  }
  return SiblingLp;
}

HLLoop *
HIRUndoSinkingForPerfectLoopnest::findCandidateSiblingLoop(HLLoop *Lp,
                                                           RegDDRef *LoadRef) {
  HLLoop *SiblingLp = nullptr;
  bool ShouldBailOut = false;

  do {
    SiblingLp = getPrevSiblingLoop(Lp, LoadRef, ShouldBailOut);

    if (ShouldBailOut) {
      return nullptr;
    }

    if (!SiblingLp) {
      Lp = Lp->getParentLoop();
    } else {
      break;
    }
  } while (Lp != nullptr);

  return SiblingLp;
}

static void setLinear(DDRef *TmpRef, unsigned LoopLevel) {
  TmpRef->getSingleCanonExpr()->setDefinedAtLevel(LoopLevel - 1);

  if (auto BlobRef = dyn_cast<BlobDDRef>(TmpRef)) {
    BlobRef->getParentDDRef()->updateDefLevel();
  } else {
    assert(cast<RegDDRef>(TmpRef)->isTerminalRef() &&
           "Expecting a terminal ref");
  }
}

static void insertInstToPreheader(HLLoop *Lp, HLInst *Inst,
                                  HLInst *&PreheaderInst) {
  if (!PreheaderInst) {
    HLNodeUtils::insertAsLastPreheaderNode(Lp, Inst);
  } else {
    HLNodeUtils::insertBefore(PreheaderInst, Inst);
  }

  PreheaderInst = Inst;
}

bool HIRUndoSinkingForPerfectLoopnest::run() {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << "HIR UndoSinking For Perfect Loopnest Disabled \n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIR UndoSinking For Perfect Loopnest on Function : "
                    << HIRF.getFunction().getName() << "\n");

  // Gather all inner-most loop and its parent loop as Candidates
  SmallVector<HLLoop *, 64> InnermostLoops;

  HLNodeUtils &HNU = HIRF.getHLNodeUtils();
  HNU.gatherInnermostLoops(InnermostLoops);

  bool Result = false;
  for (auto *Lp : InnermostLoops) {
    if (!Lp->isUndoSinkingCandidate()) {
      continue;
    }

    SmallSet<unsigned, 8> StoreRvalSBs;
    HLInst *PreheaderInst = nullptr;
    DDGraph DDG = DDA.getGraph(Lp);
    unsigned LoopLevel = Lp->getNestingLevel();

    for (auto I = Lp->child_rbegin(), Next = I, E = Lp->child_rend(); I != E;
         I = Next) {
      Next++;

      HLInst *HInst = dyn_cast<HLInst>(&*I);

      if (!HInst) {
        continue;
      }

      if (!HInst->isSinked()) {
        continue;
      }

      auto Inst = HInst->getLLVMInstruction();

      LLVM_DEBUG(dbgs() << "\tUndoSinking for Inst:\n"; HInst->dump(););

      if (isa<LoadInst>(Inst)) {
        // Create a copy inst when a matching store is found
        HLInst *CopyInst = nullptr;

        HLLoop *SiblingLp = findCandidateSiblingLoop(Lp, HInst->getRvalDDRef());

        if (SiblingLp) {
          MatchingStoreFinder USV(HInst, CopyInst);
          HLNodeUtils::visitRange<true, true, false>(
              USV, SiblingLp->child_begin(), SiblingLp->child_end());
        }

        RegDDRef *LRef = HInst->getLvalDDRef();
        HLNodeUtils::remove(HInst);

        if (CopyInst) {
          insertInstToPreheader(Lp, CopyInst, PreheaderInst);
          CopyInst->getRvalDDRef()->makeConsistent({}, LoopLevel - 1);
        } else {
          insertInstToPreheader(Lp, HInst, PreheaderInst);
          HInst->setIsSinked(false);
        }

        Lp->addLiveInTemp(LRef->getSymbase());

        if (!StoreRvalSBs.count(LRef->getSymbase())) {
          for (const DDEdge *Edge : DDG.outgoing(LRef)) {
            if (Edge->isFlow()) {
              DDRef *DDRefSink = Edge->getSink();

              if (!HLNodeUtils::contains(Lp, DDRefSink->getHLDDNode())) {
                continue;
              }

              setLinear(DDRefSink, LoopLevel);
            }
          }
        }
      } else {
        assert(isa<StoreInst>(Inst) &&
               "Only load/store are expected to be sinked.");

        StoreRvalSBs.insert(HInst->getRvalDDRef()->getSymbase());

        HLNodeUtils::moveAsFirstPostexitNode(Lp, HInst);

        for (auto *Ref :
             make_range(HInst->op_ddref_begin(), HInst->op_ddref_end())) {
          Lp->addLiveOutTemp(Ref, true /* OnlyNonLinear */);
        }

        HInst->getLvalDDRef()->makeConsistent({}, LoopLevel - 1);
        HInst->setIsSinked(false);
      }
      Result = true;
    }

    if (Result) {
      HIRInvalidationUtils::invalidateBody(Lp);
      HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<HIRLoopStatistics>(
          Lp);
    }
  }

  return Result;
}

PreservedAnalyses HIRUndoSinkingForPerfectLoopnestPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  ModifiedHIR =
      HIRUndoSinkingForPerfectLoopnest(HIRF, AM.getResult<HIRDDAnalysisPass>(F))
          .run();
  return PreservedAnalyses::all();
}

class HIRUndoSinkingForPerfectLoopnestLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRUndoSinkingForPerfectLoopnestLegacyPass() : HIRTransformPass(ID) {
    initializeHIRUndoSinkingForPerfectLoopnestLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRUndoSinkingForPerfectLoopnest(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
               getAnalysis<HIRDDAnalysisWrapperPass>().getDDA())
        .run();
  }
};

char HIRUndoSinkingForPerfectLoopnestLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRUndoSinkingForPerfectLoopnestLegacyPass, OPT_SWITCH,
                      OPT_DESC, false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRUndoSinkingForPerfectLoopnestLegacyPass, OPT_SWITCH,
                    OPT_DESC, false, false)

FunctionPass *llvm::createHIRUndoSinkingForPerfectLoopnestPass() {
  return new HIRUndoSinkingForPerfectLoopnestLegacyPass();
}
