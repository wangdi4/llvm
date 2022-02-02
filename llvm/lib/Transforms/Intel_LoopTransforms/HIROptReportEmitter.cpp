//===---- HIROptReportEmitter.cpp - Prints Loop Optimization reports ------==//
//
// Copyright (C) 2018-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements HIR optimization report emitter.
//
// Even though, the normal emission of loop optimization reports happens
// later-on after HIR passes, it is still beneficial to have this functionality
// here, at HIR stage, at least for debug purposes. This is because in HIR loops
// are first class citizens and the hierarchical structure there is much better
// preserved and defined.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIROptReportEmitterPass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/Intel_OptReport/OptReportPrintUtils.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::OptReportUtils;

#define DEBUG_TYPE "hir-optreport-emitter"

static cl::opt<bool> DisableHIROptReportEmitter(
    "disable-hir-optreport-emitter", cl::Hidden, cl::init(false),
    cl::desc("Disable HIR optimization report emitter"));

struct HIROptReportEmitter {
  bool run(Function &F, HIRFramework &HIRF);
};

struct HIROptReportEmitVisitor final : public HLNodeVisitorBase {
  formatted_raw_ostream &FOS;
  unsigned Depth;

  HIROptReportEmitVisitor(formatted_raw_ostream &FOS) : FOS(FOS), Depth(0) {}

  bool skipRecursion(const HLNode *Node) {
    const HLLoop *L = dyn_cast<HLLoop>(Node);

    // if skipRecursion returns true, the postVisit is not being
    // called for the loop, so we have to call it manually here.
    if (L && L->isInnermost()) {
      postVisit(L);
      return true;
    }
    return false;
  }

  void visit(const HLRegion *Reg) {
    OptReport OR = Reg->getOptReport();
    if (OR && OR.firstChild())
      printEnclosedOptReport(FOS, Depth, OR.firstChild());
  }

  void visit(const HLLoop *Lp) {
    OptReport OR = Lp->getOptReport();

    printNodeHeaderAndOrigin(FOS, Depth, OR, Lp->getDebugLoc());

    ++Depth;
    if (OR)
      printOptReport(FOS, Depth, OR);
  }

  void postVisit(const HLLoop *Lp) {
    OptReport OR = Lp->getOptReport();

    --Depth;
    printNodeFooter(FOS, Depth, OR);

    if (OR && OR.nextSibling())
      printEnclosedOptReport(FOS, Depth, OR.nextSibling());
  }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
};

bool HIROptReportEmitter::run(Function &F, HIRFramework &HIRF) {
  if (DisableHIROptReportEmitter)
    return false;

  formatted_raw_ostream &OS = OptReportOptions::getOutputStream();
  OS << "Report from: HIR Loop optimizations framework for : " << F.getName()
     << "\n";

  HIROptReportEmitVisitor ORV(OS);
  HIRF.getHLNodeUtils().visitAll(ORV);

  OS << "=================================================================\n\n";
  return false;
}

class HIROptReportEmitterWrapperPass : public HIRTransformPass {
public:
  static char ID;
  HIROptReportEmitterWrapperPass();

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

char HIROptReportEmitterWrapperPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIROptReportEmitterWrapperPass, "hir-optreport-emitter",
                      "HIR optimization report emitter", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_END(HIROptReportEmitterWrapperPass, "hir-optreport-emitter",
                    "HIR optimization report emitter", false, false)

FunctionPass *llvm::createHIROptReportEmitterWrapperPass() {
  return new HIROptReportEmitterWrapperPass();
}

HIROptReportEmitterWrapperPass::HIROptReportEmitterWrapperPass()
    : HIRTransformPass(ID) {
  initializeHIROptReportEmitterWrapperPassPass(
      *PassRegistry::getPassRegistry());
}

void HIROptReportEmitterWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<HIRFrameworkWrapperPass>();
  AU.setPreservesAll();
}

bool HIROptReportEmitterWrapperPass::runOnFunction(Function &F) {
  if (DisableHIROptReportEmitter)
    return false;

  HIRFramework &HIRF = getAnalysis<HIRFrameworkWrapperPass>().getHIR();

  HIROptReportEmitter Emitter;
  Emitter.run(F, HIRF);

  return false;
}

PreservedAnalyses HIROptReportEmitterPass::runImpl(Function &F,
                                                   FunctionAnalysisManager &AM,
                                                   HIRFramework &HIRF) {
  if (DisableHIROptReportEmitter)
    return PreservedAnalyses::all();

  HIROptReportEmitter Emitter;
  Emitter.run(F, HIRF);

  return PreservedAnalyses::all();
}
