//===- HIRSinkingForPerfectLoopnest.cpp Implements SinkingForPerfectLoopnest
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
// This pass recognizes the pattern to help to enable loop distribution.
//
// We are transforming this-
//
//    DO i1 = 0, 999
//      DO i2 = 0, 999
//         (%c)[0][i1][i2] = 0.000000e+00;
//         %add = 0.000000e+00;
//
//        DO i3 = 0, 999
//            %mul = (%a)[0][i1][i3]  *  (%b)[0][i3][i2];
//            %add = %add  +  %mul;
//        END LOOP

//        (%c)[0][i1][i2] = %add;
//    END LOOP
//    END LOOP
//
// To-
//
//    DO i1 = 0, 999
//    DO i2 = 0, 999, 1   <DO_LOOP>
//       (%c)[0][i1][i2] = 0.000000e+00;

//       DO i3 = 0, 999, 1   <DO_LOOP>
//         %add = c[i1][i2];
//         %mul = (%a)[0][i1][i3]  *  (%b)[0][i3][i2];
//         %add = %add  +  %mul;
//         (%c)[0][i1][i2] = %add;
//       END LOOP

//    END LOOP
//    END LOOP
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRSinkingForPerfectLoopnestPass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"

#define OPT_SWITCH "hir-sinking-for-perfect-loopnest"
#define OPT_DESC "HIR Sinking For Perfect Loopnest"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

namespace {

class HIRSinkingForPerfectLoopnest {
  HIRFramework &HIRF;
  HIRDDAnalysis &DDA;
  struct SinkingVisitor;

public:
  HIRSinkingForPerfectLoopnest(HIRFramework &HIRF, HIRDDAnalysis &DDA)
      : HIRF(HIRF), DDA(DDA) {}

  bool run();
};

} // namespace

// Gather all near perfect loop nests
struct HIRSinkingForPerfectLoopnest::SinkingVisitor final
    : public HLNodeVisitorBase {
  bool Modified = false;
  HIRDDAnalysis &DDA;
  HLNode *SkipNode;

  SinkingVisitor(HIRDDAnalysis &DDA) : DDA(DDA), SkipNode(nullptr) {}

  void visit(HLLoop *Loop) {
    if (Loop->isInnermost()) {
      SkipNode = Loop;
      return;
    }

    bool IsNearPerfectLoop = false;
    const HLLoop *InnermostLoop = nullptr;

    bool IsPerfectLoopNest = HLNodeUtils::isPerfectLoopNest(
        Loop, &InnermostLoop, false, &IsNearPerfectLoop);

    if (IsPerfectLoopNest) {
      SkipNode = Loop;
      return;
    }

    if (!IsNearPerfectLoop) {
      return;
    }

    SkipNode = Loop;

    bool ProfitableForInterchange =
        HLNodeUtils::hasNonUnitStrideRefs(InnermostLoop);

    DDGraph DDG = DDA.getGraph(Loop);
    if (!ProfitableForInterchange &&
        !containsProfitableEdge(Loop, InnermostLoop, DDG)) {
      LLVM_DEBUG(dbgs() << Loop->getNumber() << "- No Profitable Ref/Edge\n");
      return;
    }

    InterchangeIgnorableSymbasesTy IgnorableSymbases;
    if (DDUtils::enablePerfectLoopNest(const_cast<HLLoop *>(InnermostLoop), DDG,
                                       IgnorableSymbases, true)) {
      Modified = true;
      HIRInvalidationUtils::invalidateBody(InnermostLoop);
      HIRInvalidationUtils::invalidateBody(InnermostLoop->getParentLoop());
    }
  }

  // Assume incoming loopnest is near-perfect. Return true if a dependency
  // exists that crosses the innermost loop, in which case we will sink. E.g:
  //   DO i2
  //      %add = 0;
  //     DO i3
  //            %add = %add  +  %mul; <--- add has dep to outside the innerloop
  //     END i3
  //     (%c)[i1][i2] = %add;
  bool containsProfitableEdge(const HLLoop *OuterLoop, const HLLoop *InnerLoop,
                              DDGraph &DDG) const {
    const HLLoop *ParLoop = InnerLoop->getParentLoop();
    for (const HLNode &Node :
         make_range(ParLoop->child_begin(), ParLoop->child_end())) {
      auto *Inst = dyn_cast<HLInst>(&Node);
      if (!Inst) {
        continue;
      }

      for (const RegDDRef *Ref :
           make_range(Inst->op_ddref_begin(), Inst->op_ddref_end())) {
        if (Ref->isLval() && Ref->isSelfBlob() &&
            InnerLoop->isLiveIn(Ref->getSymbase())) {
          return true;
        }

        for (const auto *Edge : DDG.outgoing(Ref)) {
          if (Edge->getSink()->getParentLoop() == InnerLoop) {
            return true;
          }
        }
      }
    }

    return false;
  }

  bool sinked() { return Modified; }
  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}
  bool skipRecursion(const HLNode *Node) const { return Node == SkipNode; }
};

bool HIRSinkingForPerfectLoopnest::run() {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << "HIR Sinking For Perfect Loopnest Disabled \n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIR Sinking For Perfect Loopnest on Function : "
                    << HIRF.getFunction().getName() << "\n");

  SinkingVisitor SV(DDA);
  HIRF.getHLNodeUtils().visitAll(SV);

  return SV.sinked();
}

PreservedAnalyses HIRSinkingForPerfectLoopnestPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  ModifiedHIR =
      HIRSinkingForPerfectLoopnest(HIRF, AM.getResult<HIRDDAnalysisPass>(F))
          .run();
  return PreservedAnalyses::all();
}

class HIRSinkingForPerfectLoopnestLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRSinkingForPerfectLoopnestLegacyPass() : HIRTransformPass(ID) {
    initializeHIRSinkingForPerfectLoopnestLegacyPassPass(
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

    return HIRSinkingForPerfectLoopnest(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
               getAnalysis<HIRDDAnalysisWrapperPass>().getDDA())
        .run();
  }
};

char HIRSinkingForPerfectLoopnestLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRSinkingForPerfectLoopnestLegacyPass, OPT_SWITCH,
                      OPT_DESC, false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRSinkingForPerfectLoopnestLegacyPass, OPT_SWITCH,
                    OPT_DESC, false, false)

FunctionPass *llvm::createHIRSinkingForPerfectLoopnestPass() {
  return new HIRSinkingForPerfectLoopnestLegacyPass();
}
