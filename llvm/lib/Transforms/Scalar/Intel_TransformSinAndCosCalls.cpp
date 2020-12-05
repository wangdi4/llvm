//===- Intel_TransformSinAndCosCalls.cpp - Transform sin and cos Calls -===//
//
// Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===-------------------------------------------------------------------===//
//
// This pass transforms calls to sin and cos to calls to sinpi, cospi, or
// sincospi.
//
//===-------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/Intel_TransformSinAndCosCalls.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/MathExtras.h"
#include <stdio.h>

using namespace llvm;
using namespace llvm::PatternMatch;
using namespace intel_transform_sin_cos_calls;

#define DEBUG_TYPE "transform-sin-cos-calls"

STATISTIC(NumTransformedSinAndCosCalls, "Number of transformed sin and cos calls");

/*
 * This transformation is disabled by default. To enable, compile with:
 * -mllvm -enable-transform-sin-cos
 */
static cl::opt<bool> EnablePass("enable-transform-sin-cos",
                                cl::init(false), cl::Hidden,
                                cl::desc("Enable transform sin/cos to sinpi/cospi"));

static bool isSinf(CallInst *CI, TargetLibraryInfo &TLI) {
  Function *Callee = CI->getCalledFunction();
  LibFunc LF;

  /* Check for sinf library call. */
  if (Callee && TLI.getLibFunc(*Callee, LF)) {
    if (LF == LibFunc_sinf) {
      return true;
    }
  }

  /* Check for sin intrinsic call. */
  if (auto *Intrin = dyn_cast<IntrinsicInst>(CI)) {
    if (Intrin->getIntrinsicID() == Intrinsic::sin) {
      Value *Arg = CI->getArgOperand(0);
      auto *ArgType = Arg->getType();
      if (ArgType->isFloatTy())
        return true;
    }
  }

  return false;
}

static bool isCosf(CallInst *CI, TargetLibraryInfo &TLI) {
  Function *Callee = CI->getCalledFunction();
  LibFunc LF;

  /* Check for cosf library call. */
  if (Callee && TLI.getLibFunc(*Callee, LF)) {
    if (LF == LibFunc_cosf) {
      return true;
    }
  }

  /* Check for cos intrinsic call. */
  if (auto *Intrin = dyn_cast<IntrinsicInst>(CI)) {
    if (Intrin->getIntrinsicID() == Intrinsic::cos) {
      Value *Arg = CI->getArgOperand(0);
      auto *ArgType = Arg->getType();
      if (ArgType->isFloatTy())
        return true;
    }
  }

  return false;
}

static bool callHasHighImfPrecisionAttr(CallInst *CI) {
  if (CI->hasFnAttr("imf-precision")) {
    Attribute Attr = CI->getFnAttr("imf-precision");
    StringRef Val = Attr.getValueAsString();
    return (Val == "high");
  }
  return false;
}

/*
 * This function does the following transformation:
 * sin(expr * constant) -> sinpi(expr * (constant/pi))
 * cos(expr * constant) -> cospi(expr * (constant/pi))
 *
 * sin (cos) is converted to sinpi (cospi), if the following three
 * conditions hold:
 * . argument is of the form (expr * constant) or (constant * expr),
 * . -ffast-math is specified, and
 * . -imf-precision=low or medium
 *
 * To compute the new argument for sinpi (or cospi), we divide the
 * constant by pi in higher precision, then round to the required
 * precision. So the new argument is (expr * (constant/pi)).
 *
 * For example, suppose we have:
 *   float arg, expr, x, y;
 *   arg = expr * 8.0;
 *   x = sinf(arg);
 *   y = x + arg;
 *
 * Before this transformation, the IR looks like this:
 *   %arg = fmul float %expr, 8.0
 *   %x = call float @sinf(float %arg)
 *   %y = fadd float %x, %arg
 *
 * After this transformation, the IR looks like this:
 *   %arg = fmul float %expr, 8.0
 *   %arg.overpi = fmul float %expr, 2.5465
 *   %x = call float @sinpif(float %arg.overpi)
 *   %y = fadd float %x, %arg
 *
 * Note that in this transformation, we generate a new argument
 * (8.0/pi) and change the existing sinf call to sinpif call,
 * retaining all existing attributes on the call.
 */
static bool convertToSinpiOrCospi(CallInst *CI, TargetLibraryInfo &TLI) {
  Value *Arg = CI->getArgOperand(0);
  auto *ArgType = Arg->getType();
  Value *Expr;
  ConstantFP *Const;

  if (CI->getNumArgOperands() != 1)
    return false;

  // Argument must be of the form (expr * constant) or *constant * expr).
  if (match(Arg, m_FMul(m_Value(Expr), m_ConstantFP(Const)))) {
    Module *M          = CI->getParent()->getModule();
    Function *origFunc = CI->getCalledFunction();
    FunctionCallee Callee;

    // Compute new constant.
    double ConstOverPi = Const->getValueAPF().convertToFloat()
      / llvm::numbers::pi;
    Value *newConst = ConstantFP::get(Const->getType(), ConstOverPi);

    IRBuilder<> Builder(CI);
    Value *FMul = Builder.CreateFMulFMF(Expr, newConst,
      dyn_cast<Instruction>(Arg), Arg->getName() + ".overpi");

    // Update orignal sin/cos call instruction.
    if (isSinf(CI, TLI)) {
      Callee = M->getOrInsertFunction("sinpif", origFunc->getAttributes(),
          ArgType, ArgType);
    }
    else {
      Callee = M->getOrInsertFunction("cospif", origFunc->getAttributes(),
          ArgType, ArgType);
    }

    CI->setCalledFunction(Callee);
    CI->setArgOperand(0, FMul);
    return true;
  }

  return false;
}

/*
 * Entry point for transforming sin and cos calls.  Returns true if we
 * need to invalidate analysis.
 */
bool TransformSinAndCosCalls::run() {
  if (! EnablePass)
    return false;

  bool Changed = false;

  for (BasicBlock &Block : F) {
    for (Instruction &Inst : Block) {
      Instruction *I = &Inst;
      bool ConvertedInst = false;

      if (auto *CI = dyn_cast<CallInst>(I)) {
        // TODO: Double
        if (isSinf(CI, TLI) || isCosf(CI, TLI)) {
          if (CI->isFast() && (! callHasHighImfPrecisionAttr(CI))) {
            ConvertedInst = convertToSinpiOrCospi(CI, TLI);
            if (ConvertedInst) {
              NumTransformedSinAndCosCalls++;
              Changed = true;
            }
          }
        }
      }
    }
  }

  return Changed;
}

/*
 * New Pass Manager
 */

bool TransformSinAndCosCallsPass::runImpl(Function &F, TargetLibraryInfo &TLI) {
  return TransformSinAndCosCalls(F, TLI).run();
}

PreservedAnalyses TransformSinAndCosCallsPass::run(Function &F,
                                               FunctionAnalysisManager &AM) {
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);

  bool Changed = runImpl(F, TLI);
  if (Changed) {
    return PreservedAnalyses::none();
  }
  return PreservedAnalyses::all();
}

/*
 * Legacy (Old) Pass Manager
 */

char TransformSinAndCosCallsLegacyPass::ID = 0;

TransformSinAndCosCallsLegacyPass::TransformSinAndCosCallsLegacyPass() : FunctionPass(ID) {
  initializeTransformSinAndCosCallsLegacyPassPass(*PassRegistry::getPassRegistry());
}

bool TransformSinAndCosCallsLegacyPass::runOnFunction(Function &F) {
  auto &TLI = getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
  NumTransformedSinAndCosCalls = 0;

  return Impl.runImpl(F, TLI);
}

INITIALIZE_PASS_BEGIN(TransformSinAndCosCallsLegacyPass, DEBUG_TYPE,
                "Transform sin and cos calls", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(TransformSinAndCosCallsLegacyPass, DEBUG_TYPE,
                "Transform sin and cos calls", false, false)

/*
 * Public interface to the pass. Currently uses legacy (old) pass
 * manager.
 */

// Public interface to the pass
FunctionPass *llvm::createTransformSinAndCosCallsPass() {
  return new TransformSinAndCosCallsLegacyPass();
}
