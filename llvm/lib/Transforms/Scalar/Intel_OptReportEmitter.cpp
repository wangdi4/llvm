//===------- OptReportEmitter.cpp - Prints Optimization reports -----------===//
//
// Copyright (C) 2017-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements Optimization Report emitter.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_OptReportEmitter.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FormattedStream.h"

#include "llvm/Analysis/Intel_OptReport/OptReport.h"
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/Intel_OptReport/OptReportPrintUtils.h"

#if INTEL_FEATURE_CSA
#include "llvm/ADT/Triple.h"
#endif // INTEL_FEATURE_CSA

#define DEBUG_TYPE "intel-ir-optreport-emitter"

using namespace llvm;
using namespace llvm::OptReportUtils;

static cl::opt<bool> DisableIROptReportEmitter(
    "disable-intel-ir-optreport-emitter", cl::init(false), cl::Hidden,
    cl::desc("Disable IR optimization report emitter"));

namespace {
struct OptReportEmitter {

  OptReportEmitter() {}

  void printOptReportRecursive(const Loop *L, unsigned Depth,
                               formatted_raw_ostream &FOS);

#if INTEL_FEATURE_CSA
  bool isCSALibMathFunction(const Function &F);

  bool isCSALibCFunction(const Function &F);
#endif // INTEL_FEATURE_CSA

  bool run(Function &F, LoopInfo &LI);
};

void OptReportEmitter::printOptReportRecursive(const Loop *L, unsigned Depth,
                                               formatted_raw_ostream &FOS) {
  OptReport OR = OptReport::findOptReportInLoopID(L->getLoopID());

  printNodeHeaderAndOrigin(FOS, Depth, OR, L->getStartLoc());

  if (OR) {
    printOptReport(FOS, Depth + 1, OR);
  }

  for (const Loop *CL : L->getSubLoops())
    printOptReportRecursive(CL, Depth + 1, FOS);

  printNodeFooter(FOS, Depth, OR);

  if (OR && OR.nextSibling())
    printEnclosedOptReport(FOS, Depth, OR.nextSibling());
}

#if INTEL_FEATURE_CSA
bool OptReportEmitter::isCSALibMathFunction(const Function &F) {
  // The functions listed below are defined in the CSA SDK file:
  // base/<DATE>/libcsa/libcsamath.bc
  //
  // Note: A better approach than checking for the functions by name
  // would be to use a custom function attribute, which would disable
  // opt-report for a function. This attribute can be used for all
  // "library" functions, instead of listing them here.

  if (F.getName() == "ceil") return true;
  if (F.getName() == "ceilf") return true;
  if (F.getName() == "dp_sqrt_RD") return true;
  if (F.getName() == "dp_sqrt_RN") return true;
  if (F.getName() == "dp_sqrt_RU") return true;
  if (F.getName() == "dp_sqrt_RZ") return true;
  if (F.getName() == "exp2f") return true;
  if (F.getName() == "exp2") return true;
  if (F.getName() == "expf") return true;
  if (F.getName() == "exp") return true;
  if (F.getName() == "floor") return true;
  if (F.getName() == "floorf") return true;
  if (F.getName() == "log10f") return true;
  if (F.getName() == "log10") return true;
  if (F.getName() == "log2f") return true;
  if (F.getName() == "log2") return true;
  if (F.getName() == "log") return true;
  if (F.getName() == "logf") return true;
  if (F.getName() == "powf") return true;
  if (F.getName() == "pow") return true;
  if (F.getName() == "round") return true;
  if (F.getName() == "roundf") return true;
  if (F.getName() == "sin") return true;
  if (F.getName() == "sinf") return true;
  if (F.getName() == "cos") return true;
  if (F.getName() == "cosf") return true;
  if (F.getName() == "trunc") return true;
  if (F.getName() == "truncf") return true;
  return false;
}

bool OptReportEmitter::isCSALibCFunction(const Function &F) {
  // The functions listed below are defined in the CSA SDK file:
  // base/<DATE>/libcsa/libcsac.bc
  //
  // Note: A better approach than checking for the functions by name
  // would be to use a custom function attribute, which would disable
  // opt-report for a function. This attribute can be used for all
  // "library" functions, instead of listing them here.

  if (F.getName() == "CsaMemInitialize") return true;
  if (F.getName() == "CsaMemFree") return true;
  if (F.getName() == "CsaMemAlloc") return true;
  return false;
}
#endif // INTEL_FEATURE_CSA

bool OptReportEmitter::run(Function &F, LoopInfo &LI) {
  if (DisableIROptReportEmitter)
    return false;

#if INTEL_FEATURE_CSA
  bool IsCsaTarget = Triple(F.getParent()->getTargetTriple()).getArch() ==
                     Triple::ArchType::csa;

  if (IsCsaTarget)
    if (isCSALibMathFunction(F) || isCSALibCFunction(F))
      return false;
#endif // INTEL_FEATURE_CSA

  formatted_raw_ostream &OS = OptReportOptions::getOutputStream();
  OS << "Global optimization report for : " << F.getName() << "\n";

  // First check that there are attached reports to the function itself.
  OptReport FunOR = OptReportTraits<Function>::getOptReport(F);
  if (FunOR)
    printEnclosedOptReport(OS, 0, FunOR.firstChild());

  // Traversal through all loops of the program in lexicographical order.
  // Due to the specifics of loop build algorithm, it is achieved via reverse
  // iteration.
  for (LoopInfo::reverse_iterator I = LI.rbegin(), E = LI.rend(); I != E; ++I)
    printOptReportRecursive(*I, 0, OS);

  OS << "================================================================="
        "\n\n";

  return false;
}

struct OptReportEmitterLegacyPass : public FunctionPass {
  static char ID;

  OptReportEmitterLegacyPass() : FunctionPass(ID) {
    initializeOptReportEmitterLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;

  // We don't modify the program, so we preserve all analyses.
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

void OptReportEmitterLegacyPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<LoopInfoWrapperPass>();
}

bool OptReportEmitterLegacyPass::runOnFunction(Function &F) {
  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  return OptReportEmitter().run(F, LI);
}

} // namespace

PreservedAnalyses OptReportEmitterPass::run(Function &F,
                                            FunctionAnalysisManager &AM) {
  OptReportEmitter Emitter;
  Emitter.run(F, AM.getResult<LoopAnalysis>(F));
  return PreservedAnalyses::all();
}

char OptReportEmitterLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(OptReportEmitterLegacyPass, DEBUG_TYPE,
                      "The pass emits optimization reports", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(OptReportEmitterLegacyPass, DEBUG_TYPE,
                    "The pass emits optimization reports", false, false)

namespace llvm {

FunctionPass *createOptReportEmitterLegacyPass() {
  return new OptReportEmitterLegacyPass();
}
} // namespace llvm
