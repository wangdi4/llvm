//===--- ExpandLargeDivRem.cpp - Expand large div/rem ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass expands div/rem instructions with a bitwidth above a threshold
// into a call to auto-generated functions.
// This is useful for targets like x86_64 that cannot lower divisions
// with more than 128 bits or targets like x86_32 that cannot lower divisions
// with more than 64 bits.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/ExpandLargeDivRem.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/GlobalsModRef.h"
<<<<<<< HEAD
=======
#include "llvm/Analysis/TargetTransformInfo.h"
>>>>>>> 4b4e6d8bada98270cb94509d9f04acc7d6f4a0a8
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Utils/IntegerDivision.h"

using namespace llvm;

static cl::opt<unsigned>
<<<<<<< HEAD
    ExpandDivRemBits("expand-div-rem-bits", cl::Hidden, cl::init(128),
                     cl::desc("div and rem instructions on integers with "
                              "more than <N> bits are expanded."));

static bool runImpl(Function &F) {
  SmallVector<BinaryOperator *, 4> Replace;
  bool Modified = false;

=======
    ExpandDivRemBits("expand-div-rem-bits", cl::Hidden,
                     cl::init(llvm::IntegerType::MAX_INT_BITS),
                     cl::desc("div and rem instructions on integers with "
                              "more than <N> bits are expanded."));

static bool isConstantPowerOfTwo(llvm::Value *V, bool SignedOp) {
  auto *C = dyn_cast<ConstantInt>(V);
  if (!C)
    return false;

  APInt Val = C->getValue();
  if (SignedOp && Val.isNegative())
    Val = -Val;
  return Val.isPowerOf2();
}

static bool isSigned(unsigned int Opcode) {
  return Opcode == Instruction::SDiv || Opcode == Instruction::SRem;
}

static bool runImpl(Function &F, const TargetTransformInfo &TTI) {
  SmallVector<BinaryOperator *, 4> Replace;
  bool Modified = false;

  unsigned MaxLegalDivRemBitWidth = TTI.maxLegalDivRemBitWidth();
  if (ExpandDivRemBits != llvm::IntegerType::MAX_INT_BITS)
    MaxLegalDivRemBitWidth = ExpandDivRemBits;

  if (MaxLegalDivRemBitWidth >= llvm::IntegerType::MAX_INT_BITS)
    return false;

>>>>>>> 4b4e6d8bada98270cb94509d9f04acc7d6f4a0a8
  for (auto &I : instructions(F)) {
    switch (I.getOpcode()) {
    case Instruction::UDiv:
    case Instruction::SDiv:
    case Instruction::URem:
    case Instruction::SRem: {
      // TODO: This doesn't handle vectors.
      auto *IntTy = dyn_cast<IntegerType>(I.getType());
<<<<<<< HEAD
      if (!IntTy || IntTy->getIntegerBitWidth() <= ExpandDivRemBits)
=======
      if (!IntTy || IntTy->getIntegerBitWidth() <= MaxLegalDivRemBitWidth)
        continue;

      // The backend has peephole optimizations for powers of two.
      if (isConstantPowerOfTwo(I.getOperand(1), isSigned(I.getOpcode())))
>>>>>>> 4b4e6d8bada98270cb94509d9f04acc7d6f4a0a8
        continue;

      Replace.push_back(&cast<BinaryOperator>(I));
      Modified = true;
      break;
    }
    default:
      break;
    }
  }

  if (Replace.empty())
    return false;

  while (!Replace.empty()) {
    BinaryOperator *I = Replace.pop_back_val();

    if (I->getOpcode() == Instruction::UDiv ||
        I->getOpcode() == Instruction::SDiv) {
      expandDivision(I);
    } else {
      expandRemainder(I);
    }
  }

  return Modified;
}

PreservedAnalyses ExpandLargeDivRemPass::run(Function &F,
                                             FunctionAnalysisManager &AM) {
<<<<<<< HEAD
  bool Changed = runImpl(F);
=======
  TargetTransformInfo &TTI = AM.getResult<TargetIRAnalysis>(F);
  bool Changed = runImpl(F, TTI);
>>>>>>> 4b4e6d8bada98270cb94509d9f04acc7d6f4a0a8

  if (Changed)
    return PreservedAnalyses::none();

  return PreservedAnalyses::all();
}

class ExpandLargeDivRemLegacyPass : public FunctionPass {
public:
  static char ID;

  ExpandLargeDivRemLegacyPass() : FunctionPass(ID) {
    initializeExpandLargeDivRemLegacyPassPass(*PassRegistry::getPassRegistry());
  }

<<<<<<< HEAD
  bool runOnFunction(Function &F) override { return runImpl(F); }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
=======
  bool runOnFunction(Function &F) override {
    auto &TTI = getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
    return runImpl(F, TTI);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetTransformInfoWrapperPass>();
>>>>>>> 4b4e6d8bada98270cb94509d9f04acc7d6f4a0a8
    AU.addPreserved<AAResultsWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
  }
};

char ExpandLargeDivRemLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(ExpandLargeDivRemLegacyPass, "expand-large-div-rem",
                      "Expand large div/rem", false, false)
INITIALIZE_PASS_END(ExpandLargeDivRemLegacyPass, "expand-large-div-rem",
                    "Expand large div/rem", false, false)

FunctionPass *llvm::createExpandLargeDivRemPass() {
  return new ExpandLargeDivRemLegacyPass();
}
