// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

//===- AnalysisWrappers.cpp - Wrappers around non-pass analyses -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines pass wrappers around LLVM analyses that don't make sense to
// be passes.  It provides a nice standard pass interface to these classes so
// that they can be printed out by analyze.
//
// These classes are separated out of analyze.cpp so that it is more clear which
// code is the integral part of the analyze tool, and which part of the code is
// just making it so more passes are available.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {
/// ExternalFunctionsPassedConstants - This pass prints out call sites to
/// external functions that are called with constant arguments.  This can be
/// useful when looking for standard library functions we should constant fold
/// or handle in alias analyses.
struct ExternalFunctionsPassedConstants : public ModulePass {
  static char ID; // Pass ID, replacement for typeid
  ExternalFunctionsPassedConstants() : ModulePass(ID) {}
  bool runOnModule(Module &M) override {
    for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
      if (!I->isDeclaration())
        continue;

      bool PrintedFn = false;
      for (User *U : I->users()) {
        Instruction *UI = dyn_cast<Instruction>(U);
        if (!UI)
          continue;

        CallBase *CB = dyn_cast<CallBase>(UI);
        if (!CB)
          continue;

        for (auto AI = CB->arg_begin(), E = CB->arg_end(); AI != E; ++AI) {
          if (!isa<Constant>(*AI))
            continue;

          if (!PrintedFn) {
            errs() << "Function '" << I->getName() << "':\n";
            PrintedFn = true;
          }
          errs() << *UI;
          break;
        }
      }
    }

    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};
} // namespace

char ExternalFunctionsPassedConstants::ID = 0;
static RegisterPass<ExternalFunctionsPassedConstants>
    P1("print-externalfnconstants",
       "Print external fn callsites passed constants");
