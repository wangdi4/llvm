//===-------------------IntelMFReplacement.cpp ----------------------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// Replace arithmetic instructions like udiv, idiv, urem and srem with vector
// functions. In the future, also do a similar replacement for 'sincos' function
// with the right scalar SVML equivalent. This would simplify the vector
// codegen.
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/User.h"
#include "llvm/Transforms/Vectorize.h"
#include <unordered_map>

using namespace llvm;

#define DEBUG_TYPE "mf-replace"

static cl::opt<bool> DisableMFReplacement(
    "disable-mf-replacement", cl::init(false), cl::Hidden,
    cl::desc("Disable replacement of math-instruction like u/i-div and i/s-rem "
             "with scalar SVML function."));

STATISTIC(NumInstConverted, "Number of instructions converted");

struct MathLibraryFunctionsReplacementPass
    : PassInfoMixin<MathLibraryFunctionsReplacementPass> {
  // Run the pass over the function.
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

static bool isOptimizableOperation(Instruction *Inst) {
  Value *Divisor = Inst->getOperand(1);

  // This change is valid only for non-vectors and 32-bit integers.
  if (!Divisor->getType()->isIntegerTy(32))
    return false;

  // Do not replace functions with constant divisors as they may be replaced
  // with shifts in the future
  if (Constant *C = dyn_cast<Constant>(Divisor)) {
    const APInt &ConstIntVal = cast<ConstantInt>(C)->getValue();
    if (ConstIntVal.isPowerOf2() || (-ConstIntVal).isPowerOf2())
      return false;
  }
  return true;
}

namespace llvm {

class MathLibraryFunctionsReplacement {
private:
  SmallVector<Instruction *, 4> DivRemInsts;
  void collectMathInstructions(Function &F);
  bool transformDivRem(Module *M);
  bool transformDivRem(Instruction *I, Module *M);

  using InstOpcode = unsigned;
  using DivRemFnMap = std::unordered_map<InstOpcode, const char *>;

public:
  static DivRemFnMap DivRemInstFnMap;
  bool run(Function &F) {
    bool Changed = false;
    collectMathInstructions(F);
    transformDivRem(F.getParent());
    return Changed;
  }
};

MathLibraryFunctionsReplacement::DivRemFnMap
    MathLibraryFunctionsReplacement::DivRemInstFnMap = {
        {Instruction::UDiv, "_Z4udivjj"},
        {Instruction::SDiv, "_Z4idivii"},
        {Instruction::URem, "_Z4uremjj"},
        {Instruction::SRem, "_Z4iremii"}};

void MathLibraryFunctionsReplacement::collectMathInstructions(Function &F) {
  for (BasicBlock &Block : F) {
    for (Instruction &Inst : Block) {
      Instruction *I = &Inst;
      if (DivRemInstFnMap.count(I->getOpcode()) && isOptimizableOperation(I))
        DivRemInsts.push_back(I);
    }
  }
}

bool MathLibraryFunctionsReplacement::transformDivRem(Module *M) {
  for (Instruction *Inst : DivRemInsts) {
    Type *InstTy = Inst->getType();

    Value *Dividend = Inst->getOperand(0);
    Value *Divisor = Inst->getOperand(1);

    assert(Divisor->getType()->isIntegerTy() &&
           "Unexpected divisor element type");

    SmallVector<Type *, 2> OperandTys;

    // Create the function-type and signature
    OperandTys.push_back(Dividend->getType());
    OperandTys.push_back(Divisor->getType());

    FunctionType *FnSignature = FunctionType::get(InstTy, OperandTys, false);
    Function *FnDecl = cast<Function>(
        M->getOrInsertFunction(DivRemInstFnMap[Inst->getOpcode()], FnSignature)
            .getCallee());

    // Create the call
    SmallVector<Value *, 2> Args;
    Args.push_back(Dividend);
    Args.push_back(Divisor);

    CallInst *NewCall = CallInst::Create(
        FnDecl, Args, DivRemInstFnMap[Inst->getOpcode()], Inst);
    NewCall->setDebugLoc(Inst->getDebugLoc());

    // Replace original instruction with call
    Inst->replaceAllUsesWith(NewCall);
    Inst->eraseFromParent();
    NumInstConverted++;
  }
  return NumInstConverted > 0;
}

class MathLibraryFunctionsReplacementLegacyPass : public FunctionPass {
public:
  static char ID;

  MathLibraryFunctionsReplacementLegacyPass() : FunctionPass(ID) {
    initializeMathLibraryFunctionsReplacementLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    // Return without any change if instruction to function replacement is
    // disabled.
    if (DisableMFReplacement)
      return false;

    if (skipFunction(F))
      return false;

    MathLibraryFunctionsReplacement G;
    NumInstConverted = 0;
    return G.run(F);
  }
};

} // end namespace llvm

char MathLibraryFunctionsReplacementLegacyPass::ID = 0;

PreservedAnalyses
MathLibraryFunctionsReplacementPass::run(Function &F,
                                         FunctionAnalysisManager &AM) {
  MathLibraryFunctionsReplacement G;
  if (!G.run(F))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

INITIALIZE_PASS_BEGIN(
    MathLibraryFunctionsReplacementLegacyPass,
    "replace-with-math-library-functions",
    "Replace known math operations with optimized library functions", false,
    false)
INITIALIZE_PASS_END(
    MathLibraryFunctionsReplacementLegacyPass,
    "replace-with-math-library-functions",
    "Replace known math operations with optimized library functions", false,
    false)

Pass *llvm::createMathLibraryFunctionsReplacementPass() {
  return new MathLibraryFunctionsReplacementLegacyPass();
}
