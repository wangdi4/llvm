//===- InstCombine.h - InstCombine pass -------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
///
/// This file provides the primary interface to the instcombine pass. This pass
/// is suitable for use in the new pass manager. For a pass that works with the
/// legacy pass manager, use \c createInstructionCombiningPass().
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INSTCOMBINE_INSTCOMBINE_H
#define LLVM_TRANSFORMS_INSTCOMBINE_INSTCOMBINE_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/InstCombine/InstCombineWorklist.h"

namespace llvm {

class InstCombinePass : public PassInfoMixin<InstCombinePass> {
  InstCombineWorklist Worklist;
  bool ExpensiveCombines;
  bool TypeLoweringOpts; // INTEL

public:
  static StringRef name() { return "InstCombinePass"; }

  explicit InstCombinePass(bool ExpensiveCombines = true,     // INTEL
                           bool TypeLoweringOpts = true)      // INTEL
      : ExpensiveCombines(ExpensiveCombines),                 // INTEL
        TypeLoweringOpts(TypeLoweringOpts) {}                 // INTEL

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

/// The legacy pass manager's instcombine pass.
///
/// This is a basic whole-function wrapper around the instcombine utility. It
/// will try to combine all instructions in the function.
class InstructionCombiningPass : public FunctionPass {
  InstCombineWorklist Worklist;
  const bool ExpensiveCombines;
  const bool TypeLoweringOpts; // INTEL

public:
  static char ID; // Pass identification, replacement for typeid

<<<<<<< HEAD
  InstructionCombiningPass(bool ExpensiveCombines = true,       // INTEL
                           bool TypeLoweringOpts = true)        // INTEL
      : FunctionPass(ID), ExpensiveCombines(ExpensiveCombines), // INTEL
        TypeLoweringOpts(TypeLoweringOpts) {                    // INTEL
    initializeInstructionCombiningPassPass(*PassRegistry::getPassRegistry());
  }
=======
  InstructionCombiningPass(bool ExpensiveCombines = true);
>>>>>>> 05da2fe52162c80dfa18aedf70cf73cb11201811

  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnFunction(Function &F) override;
};

//===----------------------------------------------------------------------===//
//
// InstructionCombining - Combine instructions to form fewer, simple
// instructions. This pass does not modify the CFG, and has a tendency to make
// instructions dead, so a subsequent DCE pass is useful.
//
// This pass combines things like:
//    %Y = add int 1, %X
//    %Z = add int 1, %Y
// into:
//    %Z = add int 2, %X
//
#if INTEL_CUSTOMIZATION
FunctionPass *createInstructionCombiningPass(bool ExpensiveCombines = true,
                                             bool TypeLoweringOpts = true);
#endif // INTEL_CUSTOMIZATION
}

#endif
