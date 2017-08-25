/*=================================================================================
Copyright (c) 2017, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "FMASplitter.h"
#include "OCLPassSupport.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"

extern "C" {
  llvm::ModulePass* createFMASplitterPass() {
    return new intel::FMASplitter();
  }
}

namespace intel {

char FMASplitter::ID = 0;

OCL_INITIALIZE_PASS(FMASplitter, "fma-splitter",
                    "Split fmuladd to fmul + fadd", false, false)

bool FMASplitter::runOnModule(llvm::Module &M) {
  bool bChanged = false;
  for (auto &F : M) {
    bChanged |= handleFMAIntrinsics(F);
  }

  return bChanged;
}

bool FMASplitter::handleFMAIntrinsics(Function &F) {

  std::vector<Instruction *> toDelete;
  for (auto &I : instructions(F)) {
    if (auto *Intrin = dyn_cast<IntrinsicInst>(&I)) {

      if (Intrin->getIntrinsicID() ==  Intrinsic::fmuladd) {
        Value* Mul0 = Intrin->getArgOperand(0);
        Value* Mul1 = Intrin->getArgOperand(1);
        Value* Add0 = Intrin->getArgOperand(2);
        Instruction* MulResult =
            BinaryOperator::CreateFMul(Mul0, Mul1, "splitfma", Intrin);
        Instruction* AddResult =
            BinaryOperator::CreateFAdd(MulResult, Add0, "splitfma", Intrin);
        Intrin->replaceAllUsesWith(AddResult);
        toDelete.push_back(Intrin);
      }
    }
  }

  for (auto I : toDelete) {
    I->eraseFromParent();
  }

  return !toDelete.empty();
}

} // namespace intel {