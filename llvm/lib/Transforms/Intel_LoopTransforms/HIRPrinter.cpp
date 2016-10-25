//===------------- HIRPrinter.cpp - Prints HIR ----------------------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HIR printer pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"

#include "llvm/IR/Function.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
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
  HIRPrinter(raw_ostream &OS, const std::string &Banner)
      : FunctionPass(ID), OS(OS), Banner(Banner) {}

  bool runOnFunction(Function &F) override {
    auto &HIRF = getAnalysis<HIRFramework>();

    OS << Banner << "\n";
    OS << "Function: " << F.getName() << "\n";

    HIRF.print(OS);

    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequired<HIRFramework>();
  }
};
}

INITIALIZE_PASS(HIRPrinter, "hir-printer", "HIR Print", false, false)
char HIRPrinter::ID = 0;

FunctionPass *llvm::createHIRPrinterPass(raw_ostream &OS,
                                         const std::string &Banner) {
  return new HIRPrinter(OS, Banner);
}
