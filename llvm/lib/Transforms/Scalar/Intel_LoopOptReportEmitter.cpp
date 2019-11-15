//===- LoopOptReportEmitter.cpp - Prints Loop Optimization reports -------===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements Loop Optimization Report emitter.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_LoopOptReportEmitter.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/FormattedStream.h"

#include "llvm/Analysis/Intel_OptReport/LoopOptReport.h"
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/LoopOptReportPrintUtils.h"

#define DEBUG_TYPE "intel-ir-optreport-emitter"

using namespace llvm;
using namespace llvm::OptReportUtils;

static cl::opt<bool> DisableIROptReportEmitter(
    "disable-intel-ir-optreport-emitter", cl::init(false), cl::Hidden,
    cl::desc("Disable IR optimization report emitter"));

namespace {
struct LoopOptReportEmitter {

  LoopOptReportEmitter() {}

  void printLoopOptReportRecursive(const Loop *L, unsigned Depth,
                                   formatted_raw_ostream &FOS);

  bool run(Function &F, LoopInfo &LI);
};

void LoopOptReportEmitter::printLoopOptReportRecursive(
    const Loop *L, unsigned Depth, formatted_raw_ostream &FOS) {
  LoopOptReport OptReport =
      LoopOptReport::findOptReportInLoopID(L->getLoopID());

  printLoopHeaderAndOrigin(FOS, Depth, OptReport, L->getStartLoc());

  if (OptReport) {
    printOptReport(FOS, Depth + 1, OptReport);
  }

  for (const Loop *CL : L->getSubLoops())
    printLoopOptReportRecursive(CL, Depth + 1, FOS);

  printLoopFooter(FOS, Depth);

  if (OptReport && OptReport.nextSibling())
    printEnclosedOptReport(FOS, Depth, OptReport.nextSibling());
}

bool LoopOptReportEmitter::run(Function &F, LoopInfo &LI) {
  if (DisableIROptReportEmitter)
    return false;

  formatted_raw_ostream OS(dbgs());
  OS << "Global loop optimization report for : " << F.getName() << "\n";

  // First check that there are attached reports to the function itself.
  LoopOptReport FunOR = LoopOptReportTraits<Function>::getOptReport(F);
  if (FunOR)
    printEnclosedOptReport(OS, 0, FunOR.firstChild());

  // Traversal through all loops of the program in lexicographical order.
  // Due to the specifics of loop build algorithm, it is achieved via reverse
  // iteration.
  for (LoopInfo::reverse_iterator I = LI.rbegin(), E = LI.rend(); I != E; ++I)
    printLoopOptReportRecursive(*I, 0, OS);

  OS << "================================================================="
        "\n\n";

  return false;
}

struct LoopOptReportEmitterLegacyPass : public FunctionPass {
  static char ID;

  LoopOptReportEmitterLegacyPass() : FunctionPass(ID) {
    initializeLoopOptReportEmitterLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;

  // We don't modify the program, so we preserve all analyses.
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

void LoopOptReportEmitterLegacyPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<LoopInfoWrapperPass>();
}

bool LoopOptReportEmitterLegacyPass::runOnFunction(Function &F) {
  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  return LoopOptReportEmitter().run(F, LI);
}

} // namespace

PreservedAnalyses LoopOptReportEmitterPass::run(Function &F,
                                                FunctionAnalysisManager &AM) {
  LoopOptReportEmitter Emitter;
  Emitter.run(F, AM.getResult<LoopAnalysis>(F));
  return PreservedAnalyses::all();
}

char LoopOptReportEmitterLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(LoopOptReportEmitterLegacyPass, DEBUG_TYPE,
                      "The pass emits optimization reports", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(LoopOptReportEmitterLegacyPass, DEBUG_TYPE,
                    "The pass emits optimization reports", false, false)

namespace llvm {

FunctionPass *createLoopOptReportEmitterLegacyPass() {
  return new LoopOptReportEmitterLegacyPass();
}
} // namespace llvm
