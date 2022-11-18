//===--- IRPrintingPasses.cpp - Module and Function printing passes -------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// PrintModulePass and PrintFunctionPass implementations for the legacy pass
// manager.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PrintPasses.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

<<<<<<< HEAD
PrintModulePass::PrintModulePass() : OS(dbgs()) {}
PrintModulePass::PrintModulePass(raw_ostream &OS, const std::string &Banner,
                                 bool ShouldPreserveUseListOrder)
    : OS(OS), Banner(Banner)
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
      ,
      ShouldPreserveUseListOrder(ShouldPreserveUseListOrder)
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
{
}

PreservedAnalyses PrintModulePass::run(Module &M, ModuleAnalysisManager &) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
  if (llvm::isFunctionInPrintList("*")) {
    if (!Banner.empty())
      OS << Banner << "\n";
    M.print(OS, nullptr, ShouldPreserveUseListOrder);
  }
  else {
    bool BannerPrinted = false;
    for(const auto &F : M.functions()) {
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

=======
>>>>>>> 7059a6c32cfad8f272fad47265e3890cd7a1a7e1
namespace {

class PrintModulePassWrapper : public ModulePass {
  raw_ostream &OS;
  std::string Banner;
  bool ShouldPreserveUseListOrder;

public:
  static char ID;
  PrintModulePassWrapper() : ModulePass(ID), OS(dbgs()) {}
  PrintModulePassWrapper(raw_ostream &OS, const std::string &Banner,
                         bool ShouldPreserveUseListOrder)
      : ModulePass(ID), OS(OS), Banner(Banner),
        ShouldPreserveUseListOrder(ShouldPreserveUseListOrder) {}

  bool runOnModule(Module &M) override {
<<<<<<< HEAD
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
    ModuleAnalysisManager DummyMAM;
    P.run(M, DummyMAM);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
=======
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
>>>>>>> 7059a6c32cfad8f272fad47265e3890cd7a1a7e1
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  StringRef getPassName() const override { return "Print Module IR"; }
};

class PrintFunctionPassWrapper : public FunctionPass {
  raw_ostream &OS;
  std::string Banner;

public:
  static char ID;
  PrintFunctionPassWrapper() : FunctionPass(ID), OS(dbgs()) {}
  PrintFunctionPassWrapper(raw_ostream &OS, const std::string &Banner)
      : FunctionPass(ID), OS(OS), Banner(Banner) {}

  // This pass just prints a banner followed by the function as it's processed.
  bool runOnFunction(Function &F) override {
<<<<<<< HEAD
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
    FunctionAnalysisManager DummyFAM;
    P.run(F, DummyFAM);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
=======
    if (isFunctionInPrintList(F.getName())) {
      if (forcePrintModuleIR())
        OS << Banner << " (function: " << F.getName() << ")\n"
           << *F.getParent();
      else
        OS << Banner << '\n' << static_cast<Value &>(F);
    }
>>>>>>> 7059a6c32cfad8f272fad47265e3890cd7a1a7e1
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  StringRef getPassName() const override { return "Print Function IR"; }
};

} // namespace

char PrintModulePassWrapper::ID = 0;
INITIALIZE_PASS(PrintModulePassWrapper, "print-module",
                "Print module to stderr", false, true)
char PrintFunctionPassWrapper::ID = 0;
INITIALIZE_PASS(PrintFunctionPassWrapper, "print-function",
                "Print function to stderr", false, true)

ModulePass *llvm::createPrintModulePass(llvm::raw_ostream &OS,
                                        const std::string &Banner,
                                        bool ShouldPreserveUseListOrder) {
  return new PrintModulePassWrapper(OS, Banner, ShouldPreserveUseListOrder);
}

FunctionPass *llvm::createPrintFunctionPass(llvm::raw_ostream &OS,
                                            const std::string &Banner) {
  return new PrintFunctionPassWrapper(OS, Banner);
}

bool llvm::isIRPrintingPass(Pass *P) {
  const char *PID = (const char *)P->getPassID();

  return (PID == &PrintModulePassWrapper::ID) ||
         (PID == &PrintFunctionPassWrapper::ID);
}
