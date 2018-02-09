//==----- FMASplitter.cpp - Pass to split FMA intrinsic -*- C++ -*----------==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Pass.h"

using namespace llvm;

namespace {

/// @brief  FMASplitter is responsible for splitting llvm.fmuladd to
//  "fmul + fadd" pair.
class FMASplitter : public FunctionPass {
public:
  /// Pass identification, replacement for typeid
  static char ID;

  FMASplitter() : FunctionPass(ID) {}

  virtual llvm::StringRef getPassName() const { return "FMASplitter"; }

  virtual bool runOnFunction(Function &F) {
    if (skipFunction(F)) {
      return false;
    }

    std::vector<Instruction *> toDelete;
    for (auto &I : instructions(F)) {
      if (auto *Intrin = dyn_cast<IntrinsicInst>(&I)) {
        if (Intrin->getIntrinsicID() == Intrinsic::fmuladd) {
          Value *Mul0 = Intrin->getArgOperand(0);
          Value *Mul1 = Intrin->getArgOperand(1);
          Value *Add0 = Intrin->getArgOperand(2);
          Instruction *MulResult =
              BinaryOperator::CreateFMul(Mul0, Mul1, "splitfma", Intrin);
          Instruction *AddResult =
              BinaryOperator::CreateFAdd(MulResult, Add0, "splitfma", Intrin);
          Intrin->replaceAllUsesWith(AddResult);
          toDelete.push_back(Intrin);
        }
      }
    }

    for (auto I : toDelete)
      I->eraseFromParent();

    return !toDelete.empty();
  }
};
}

INITIALIZE_PASS(FMASplitter, "fma-splitter", "Split fmuladd to fmul + fadd",
                false, false)
char FMASplitter::ID = 0;

llvm::FunctionPass *createFMASplitterPass() { return new FMASplitter(); }

