//===- LoopAccessAnalysisPrinter.cpp - Loop Access Analysis Printer --------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/LoopAccessAnalysisPrinter.h"
#ifndef INTEL_CUSTOMIZATION
#include "llvm/ADT/PriorityWorklist.h"
#endif // INTEL_CUSTOMIZATION
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#ifndef INTEL_CUSTOMIZATION
#include "llvm/Transforms/Utils/LoopUtils.h"
#endif // INTEL_CUSTOMIZATION

using namespace llvm;

#define DEBUG_TYPE "loop-accesses"

#ifdef INTEL_CUSTOMIZATION
PreservedAnalyses
LoopAccessInfoPrinterPass::run(Loop &L, LoopAnalysisManager &AM,
                               LoopStandardAnalysisResults &AR, LPMUpdater &) {
  Function &F = *L.getHeader()->getParent();
  auto &LAI = AM.getResult<LoopAccessAnalysis>(L, AR);
  OS << "Loop access info in function '" << F.getName() << "':\n";
  OS.indent(2) << L.getHeader()->getName() << ":\n";
  LAI.print(OS, 4);
  return PreservedAnalyses::all();
}

#else
PreservedAnalyses LoopAccessInfoPrinterPass::run(Function &F,
                                                 FunctionAnalysisManager &AM) {
  auto &LAIs = AM.getResult<LoopAccessAnalysis>(F);
  auto &LI = AM.getResult<LoopAnalysis>(F);
  OS << "Loop access info in function '" << F.getName() << "':\n";

  SmallPriorityWorklist<Loop *, 4> Worklist;
  appendLoopsToWorklist(LI, Worklist);
  while (!Worklist.empty()) {
    Loop *L = Worklist.pop_back_val();
    OS.indent(2) << L->getHeader()->getName() << ":\n";
    LAIs.getInfo(*L).print(OS, 4);
  }
  return PreservedAnalyses::all();
}
#endif // INTEL_CUSTOMIZATION
