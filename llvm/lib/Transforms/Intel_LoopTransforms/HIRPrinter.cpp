//===------------- HIRPrinter.cpp - Prints HIR ----------*- C++ -*---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HIR printer pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRCreation.h"

#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-printer"

namespace {

class HIRPrinter : public FunctionPass {
  raw_ostream &OS;
  std::string Banner;

public:
  static char ID;
  HIRPrinter() : FunctionPass(ID), OS(dbgs()) {}
  HIRPrinter(raw_ostream &OS, const std::string &Banner) : FunctionPass(ID), 
            OS(OS), Banner(Banner) {}

  bool runOnFunction(Function &F) override {
    auto HIR = getAnalysisIfAvailable<HIRCreation>();

    OS << Banner << "\n";

    if (HIR) {
      HIR->print(OS, nullptr);
    }

    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
  }

};

}

INITIALIZE_PASS(HIRPrinter, "hir-printer", "HIR Print", false, false)
char HIRPrinter::ID = 0;

FunctionPass *llvm::createHIRPrinterPass(raw_ostream &OS, 
                                         const std::string &Banner) { 
  return new HIRPrinter(OS, Banner); 
}

