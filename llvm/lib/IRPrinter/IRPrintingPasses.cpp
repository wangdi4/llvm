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
                                 bool ShouldPreserveUseListOrder,
                                 bool EmitSummaryIndex)
    : OS(OS), Banner(Banner),
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
      ShouldPreserveUseListOrder(ShouldPreserveUseListOrder),
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
      EmitSummaryIndex(EmitSummaryIndex) {}

PreservedAnalyses PrintModulePass::run(Module &M, ModuleAnalysisManager &AM) {
<<<<<<< HEAD
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
=======
  // RemoveDIs: there's no textual representation of the DPValue debug-info,
  // convert to dbg.values before writing out.
  bool ShouldConvert = M.IsNewDbgInfoFormat;
  if (ShouldConvert)
    M.convertFromNewDbgValues();

>>>>>>> 10a9e7442c4c4e09ee94f6e1d67e2ba7396bf7cb
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
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL

  ModuleSummaryIndex *Index =
      EmitSummaryIndex ? &(AM.getResult<ModuleSummaryIndexAnalysis>(M))
                       : nullptr;
  if (Index) {
    if (Index->modulePaths().empty())
      Index->addModule("");
    Index->print(OS);
  }

  if (ShouldConvert)
    M.convertToNewDbgValues();

  return PreservedAnalyses::all();
}

PrintFunctionPass::PrintFunctionPass() : OS(dbgs()) {}
PrintFunctionPass::PrintFunctionPass(raw_ostream &OS, const std::string &Banner)
    : OS(OS), Banner(Banner) {}

PreservedAnalyses PrintFunctionPass::run(Function &F,
                                         FunctionAnalysisManager &) {
<<<<<<< HEAD
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
=======
  // RemoveDIs: there's no textual representation of the DPValue debug-info,
  // convert to dbg.values before writing out.
  bool ShouldConvert = F.IsNewDbgInfoFormat;
  if (ShouldConvert)
    F.convertFromNewDbgValues();

>>>>>>> 10a9e7442c4c4e09ee94f6e1d67e2ba7396bf7cb
  if (isFunctionInPrintList(F.getName())) {
    if (forcePrintModuleIR())
      OS << Banner << " (function: " << F.getName() << ")\n" << *F.getParent();
    else
      OS << Banner << '\n' << static_cast<Value &>(F);
  }
<<<<<<< HEAD
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
=======

  if (ShouldConvert)
    F.convertToNewDbgValues();

>>>>>>> 10a9e7442c4c4e09ee94f6e1d67e2ba7396bf7cb
  return PreservedAnalyses::all();
}
