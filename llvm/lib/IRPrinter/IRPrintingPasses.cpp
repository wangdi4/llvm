//===--- IRPrintingPasses.cpp - Module and Function printing passes -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// PrintModulePass and PrintFunctionPass implementations.
//
//===----------------------------------------------------------------------===//

#include "llvm/IRPrinter/IRPrintingPasses.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/ModuleSummaryAnalysis.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PrintPasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

PrintModulePass::PrintModulePass() : OS(dbgs()) {}
PrintModulePass::PrintModulePass(raw_ostream &OS, const std::string &Banner,
<<<<<<< HEAD
                                 bool ShouldPreserveUseListOrder)
    : OS(OS), Banner(Banner)
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
      ,
      ShouldPreserveUseListOrder(ShouldPreserveUseListOrder)
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
{}

PreservedAnalyses PrintModulePass::run(Module &M, ModuleAnalysisManager &) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
=======
                                 bool ShouldPreserveUseListOrder,
                                 bool EmitSummaryIndex)
    : OS(OS), Banner(Banner),
      ShouldPreserveUseListOrder(ShouldPreserveUseListOrder),
      EmitSummaryIndex(EmitSummaryIndex) {}

PreservedAnalyses PrintModulePass::run(Module &M, ModuleAnalysisManager &AM) {
>>>>>>> f102fe73044062542f022182381c17172a999db2
  if (llvm::isFunctionInPrintList("*")) {
    if (!Banner.empty())
      OS << Banner << "\n";
    M.print(OS, nullptr, ShouldPreserveUseListOrder);
  } else {
    bool BannerPrinted = false;
    for (const auto &F : M.functions()) {
      if (llvm::isFunctionInPrintList(F.getName())) {
        if (!BannerPrinted && !Banner.empty()) {
          OS << Banner << "\n";
          BannerPrinted = true;
        }
        F.print(OS);
      }
    }
  }
<<<<<<< HEAD
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
=======

  ModuleSummaryIndex *Index =
      EmitSummaryIndex ? &(AM.getResult<ModuleSummaryIndexAnalysis>(M))
                       : nullptr;
  if (Index) {
    if (Index->modulePaths().empty())
      Index->addModule("", 0);
    Index->print(OS);
  }

>>>>>>> f102fe73044062542f022182381c17172a999db2
  return PreservedAnalyses::all();
}

PrintFunctionPass::PrintFunctionPass() : OS(dbgs()) {}
PrintFunctionPass::PrintFunctionPass(raw_ostream &OS, const std::string &Banner)
    : OS(OS), Banner(Banner) {}

PreservedAnalyses PrintFunctionPass::run(Function &F,
                                         FunctionAnalysisManager &) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
  if (isFunctionInPrintList(F.getName())) {
    if (forcePrintModuleIR())
      OS << Banner << " (function: " << F.getName() << ")\n" << *F.getParent();
    else
      OS << Banner << '\n' << static_cast<Value &>(F);
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
  return PreservedAnalyses::all();
}
