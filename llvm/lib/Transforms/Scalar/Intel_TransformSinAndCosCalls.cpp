//===- Intel_TransformSinAndCosCalls.cpp - Transform sin and cos Calls -===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===-------------------------------------------------------------------===//
//
// This pass transforms calls to sin and cos to calls to sinpi, cospi, or
// sincos.
//
//===-------------------------------------------------------------------===//

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/Intel_TransformSinAndCosCalls.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
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

// Pi used in double-precision computations
#ifndef M_PI
    #define M_PI    3.14159265358979323846264338327950288
#endif

// Pi used in quad-precision computations
APFloat PiQuad(APFloat::IEEEquad(),
               APInt(128, {0x8469'898c'c517'01b8, 0x4000'921f'b544'42d1}));

STATISTIC(NumTransformedSinAndCosCalls, "Number of transformed sin/cos calls");

static cl::opt<bool> EnablePass(
    "enable-transform-sin-cos",
    cl::init(true), cl::Hidden,
    cl::desc("Enable sin/cos transformations"));

static cl::opt<bool> EnableTransformDoubleSinCos(
    "enable-transform-sin-cos-double",
    cl::init(false), cl::Hidden,
    cl::desc("Enable transformation of double sin/cos to sinpi/cospi"));

/*
 * This function checks whether the call (CI) is call to the library
 * function LF, or a call to the intrinsic with IntrinID whose type is
 * indicated byIsFloatType.
 *
 * When checking for a library function, the LibFunc argument LF is
 * sufficient to fully identify the library function.

 * When checking for an intrinsic, IntrinID does not fully identify
 * the intrinsic. We also need to specify the datatype of the
 * intrinsic we are looking for. This is indicated by the argument
 * IsFloatType.
 *
 * Examples:
 *
 * Check whether CI calls sinf (the float version of sin):
 * bool IsSinf = isMathLibFunctionCall(UserCI, TLI, LibFunc_sinf,
 *                                     Intrinsic::sin, true);
 *
 * Check whether CI calls cos (the double version of cos):
 * bool IsCos = isMathLibFunctionCall(UserCI, TLI, LibFunc_cos,
 *                                    Intrinsic::cos, false);
 */
static bool isMathLibFunctionCall(CallInst *CI, TargetLibraryInfo &TLI,
                                  LibFunc LF, Intrinsic::ID IntrinID,
                                  bool IsFloatType) {
  Function *Callee = CI->getCalledFunction();
  LibFunc CalleeLF;

  /* Check for library call. */
  if (Callee && TLI.getLibFunc(*Callee, CalleeLF)) {
    if (CalleeLF == LF) {
      return true;
    }
  }

  /* Check for intrinsic call. */
  if (auto *Intrin = dyn_cast<IntrinsicInst>(CI)) {
    if (Intrin->getIntrinsicID() == IntrinID) {
      Value *Arg = CI->getArgOperand(0);
      auto *ArgType = Arg->getType();
      if (ArgType->isFloatTy() && IsFloatType)
        return true;
      else if (ArgType->isDoubleTy() && (! IsFloatType))
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
 * Return true if the sin/cos call instruction (CI) can be paired with
 * another cos/sin call.
 */
static bool isPairedSinCos(CallInst *CI,
                           DominatorTree &DT,
                           TargetLibraryInfo &TLI) {

  assert(CI->arg_size() == 1 && "Single argument call expected.");

  Value *Arg = CI->getOperand(0);
  bool IsSinf, IsCosf, IsSin, IsCos;

  IsSinf = isMathLibFunctionCall(CI, TLI, LibFunc_sinf, Intrinsic::sin, true);
  IsCosf = isMathLibFunctionCall(CI, TLI, LibFunc_cosf, Intrinsic::cos, true);
  IsSin = isMathLibFunctionCall(CI, TLI, LibFunc_sin, Intrinsic::sin, false);
  IsCos = isMathLibFunctionCall(CI, TLI, LibFunc_cos, Intrinsic::cos, false);


  // Search the uses of "Arg" and check sin and cos calls that
  // dominate or are dominated by the given call (CI).
  for (User *U : Arg->users()) {
    if (auto *UserCI = dyn_cast<CallInst>(U)) {
      bool IsSinfUser, IsCosfUser, IsSinUser, IsCosUser;
      IsSinfUser = isMathLibFunctionCall(UserCI, TLI, LibFunc_sinf,
                                         Intrinsic::sin, true);
      IsCosfUser = isMathLibFunctionCall(UserCI, TLI, LibFunc_cosf,
                                         Intrinsic::cos, true);
      IsSinUser  = isMathLibFunctionCall(UserCI, TLI, LibFunc_sin,
                                         Intrinsic::sin, false);
      IsCosUser  = isMathLibFunctionCall(UserCI, TLI, LibFunc_cos,
                                         Intrinsic::cos, false);

      if ((IsSinf && IsCosfUser) ||
          (IsCosf && IsSinfUser) ||
          (IsSin  && IsCosUser)  ||
          (IsCos  && IsSinUser))
        if (DT.dominates(UserCI, CI) || DT.dominates(CI, UserCI)) {
          return true;
        }
    }
  }

  return false;
}

/*
 * Return true if can convert sin to sinpi, and cos to cospi.
 * Otherwise, return false.
 */
static bool doConvertToSinpiOrCospi(CallInst *CI,
                                    DominatorTree &DT,
                                    LoopInfo &LI,
                                    TargetLibraryInfo &TLI) {

  assert(CI->arg_size() == 1 && "Single argument call expected.");

  // -ffast-math must be specified.
  if (! CI->isFast())
    return false;

  // -fimf-precision must be low or medium.
  if (callHasHighImfPrecisionAttr(CI))
    return false;

  // The call (CI) must be:
  // (a) float (sinf or cosf)
  // OR
  // (b) double (sin or cos) and the transformation is enabled for double.
  bool IsSinf, IsCosf, IsSin, IsCos;
  IsSinf = isMathLibFunctionCall(CI, TLI, LibFunc_sinf, Intrinsic::sin, true);
  IsCosf = isMathLibFunctionCall(CI, TLI, LibFunc_cosf, Intrinsic::cos, true);
  IsSin = isMathLibFunctionCall(CI, TLI, LibFunc_sin, Intrinsic::sin, false);
  IsCos = isMathLibFunctionCall(CI, TLI, LibFunc_cos, Intrinsic::cos, false);

  if (! (IsSinf ||
         IsCosf ||
         (IsSin && EnableTransformDoubleSinCos) ||
         (IsCos && EnableTransformDoubleSinCos)))
    return false;

  // If the call (CI) is enclosed by an OpenMP simd loop, then we do
  // the transformation to sinpi/cospi because the call will be
  // vectorized.
  Loop *EnclosingLoop = LI.getLoopFor(CI->getParent());
  if (EnclosingLoop && isOmpSIMDLoop(EnclosingLoop))
    return true;

  // The sin/cos call must not be paired with another cos/sin call.
  // If the call is paired, we don't do the transformation to
  // sinpi/cospi, but rather let the scalar sin/cos pair be merged
  // into scalar sincos, which is generally faster.
  if (! isPairedSinCos(CI, DT, TLI))
    return true;

  return false;
}

static Value * divideFloatConstByPi(ConstantFP *Const) {
  // Do computation of new constant in double precision.
  float ConstOverPi = (Const->getValueAPF().convertToFloat() / M_PI);
  return ConstantFP::get(Const->getType(), ConstOverPi);
}

static Value * divideDoubleConstByPi(ConstantFP *Const) {
  // Do computation of new constant in quad precision.
  APFloat ConstVal = Const->getValueAPF();
  bool Ignored;

  ConstVal.convert(APFloat::IEEEquad(), APFloat::rmNearestTiesToEven,
                   &Ignored);
  ConstVal = ConstVal / PiQuad;
  ConstVal.convert(APFloat::IEEEdouble(), APFloat::rmNearestTiesToEven,
                   &Ignored);

  return ConstantFP::get(Const->getType(), ConstVal);
}

/*
 * This function transforms sin to sinpi, and cos to cospi.
 *
 * The transformation is enabled if all the following three conditions hold:
 * (a) Argument is of the form (Expr * Const)
 *     OR
 *     Argument is of the form ((Expr * MulConst) + AddConst)
 * (b) -ffast-math is specified, AND
 * (c) -imf-precision=low or medium
 *
 * If arguments is of the form (Expr * Const), then the following
 * transformations are applied:
 * sin(Expr * Const) -> sinpi(Expr * (Const/Pi))
 * cos(Expr * Const) -> cospi(Expr * (Const/Pi))
 *
 * If argument is of the form ((Expr * MulConst) + AddConst), then
 * the following transformations are applied:
 * sin((Expr*MulConst) + AddConst) -> sinpi((Expr*MulConst/Pi) + AddConst/Pi)
 * cos((Expr*MulConst) + AddConst) -> cospi((Expr*MulConst/Pi) + AddConst/Pi)
 *
 * ------
 *
 * In case of float sinf/cosf, we compute the new constant(s) for
 * sinpif/cospif by dividing the constant(s) by Pi in double
 * precision, and then rounding to float.
 *
 * In case of double sin/cos, we compute the new constant(s) for
 * sinpi/cospi by dividing the constant(s) by Pi in quad
 * precision, then rounding to double.
 *
 * ------
 *
 * To illustrate the transformation in LLVM IR, suppose we have:
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
 * (8.0/Pi) and change the existing sinf call to sinpif call,
 * retaining all existing attributes on the call.
 *
 */
static bool convertToSinpiOrCospi(CallInst *CI, TargetLibraryInfo &TLI) {
  Value *Arg = CI->getArgOperand(0);
  auto *ArgType = Arg->getType();
  Value *Expr, *FSinCosArg;
  ConstantFP *Const, *MulConst, *AddConst;
  Value *NewConst, *NewMulConst, *NewAddConst;
  bool PatternMatch = false;

  if (CI->arg_size() != 1)
    return false;

  if ((! ArgType->isFloatTy()) && (! ArgType->isDoubleTy()))
    return false;

  if (match(Arg, m_FMul(m_Value(Expr), m_ConstantFP(Const)))) {
    // sin/cos argument is of the form: Arg = (Expr * Const)

    // Compute new constant.
    IRBuilder<> Builder(CI);

    if (ArgType->isFloatTy()) {
      NewConst = divideFloatConstByPi(Const);
    } else {
      NewConst = divideDoubleConstByPi(Const);
    }

    FSinCosArg = Builder.CreateFMulFMF(
        Expr, NewConst, dyn_cast<Instruction>(Arg), Arg->getName() + ".overpi");

    PatternMatch = true;

  } else if (match(Arg,
                   m_FAdd(
                       m_FMul(m_Value(Expr), m_ConstantFP(MulConst)),
                       m_ConstantFP(AddConst)))) {
    // sin/cos argument is of the form:
    // Mul = (MulConst * Expr)
    // Arg = Mul + AddConst

    // Compute new add and mul constants.
    IRBuilder<> Builder(CI);

    if (ArgType->isFloatTy()) {
      NewMulConst = divideFloatConstByPi(MulConst);
      NewAddConst = divideFloatConstByPi(AddConst);
    }
    else {
      NewMulConst = divideDoubleConstByPi(MulConst);
      NewAddConst = divideDoubleConstByPi(AddConst);
    }

    Value *OrigFMul = cast<Instruction>(Arg)->getOperand(0);
    Value *FMul =
        Builder.CreateFMulFMF(Expr, NewMulConst, cast<Instruction>(OrigFMul),
                              OrigFMul->getName() + ".overpi");
    FSinCosArg =
        Builder.CreateFAddFMF(FMul, NewAddConst, cast<Instruction>(Arg),
                              Arg->getName() + ".overpi");

    PatternMatch = true;
  }

  // Update original sinf/cosf/sin/cos call instruction.
  if (PatternMatch) {
    Module *M = CI->getParent()->getModule();
    Function *origFunc = CI->getCalledFunction();
    FunctionCallee Callee;

    if (isMathLibFunctionCall(CI, TLI, LibFunc_sinf, Intrinsic::sin, true))
      Callee = M->getOrInsertFunction("sinpif", origFunc->getAttributes(),
                                      ArgType, ArgType);
    else if (isMathLibFunctionCall(CI, TLI, LibFunc_cosf, Intrinsic::cos, true))
      Callee = M->getOrInsertFunction("cospif", origFunc->getAttributes(),
                                      ArgType, ArgType);
    else if (isMathLibFunctionCall(CI, TLI, LibFunc_sin,  Intrinsic::sin, false))
      Callee = M->getOrInsertFunction("sinpi", origFunc->getAttributes(),
                                      ArgType, ArgType);
    else
      Callee = M->getOrInsertFunction("cospi", origFunc->getAttributes(),
                                      ArgType, ArgType);

    CI->setCalledFunction(Callee);
    CI->setArgOperand(0, FSinCosArg);
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
        bool IsSinf, IsCosf, IsSin, IsCos;
        IsSinf = isMathLibFunctionCall(CI, TLI, LibFunc_sinf, Intrinsic::sin,
                                       true);
        IsCosf = isMathLibFunctionCall(CI, TLI, LibFunc_cosf, Intrinsic::cos,
                                       true);
        IsSin  = isMathLibFunctionCall(CI, TLI, LibFunc_sin,  Intrinsic::sin,
                                       false);
        IsCos  = isMathLibFunctionCall(CI, TLI, LibFunc_cos,  Intrinsic::cos,
                                       false);
        if (IsSinf || IsCosf || IsSin || IsCos) {
          if (doConvertToSinpiOrCospi(CI, DT, LI, TLI)) {
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

bool TransformSinAndCosCallsPass::runImpl(Function &F,
                                          DominatorTree &DT,
                                          LoopInfo &LI,
                                          TargetLibraryInfo &TLI) {
  return TransformSinAndCosCalls(F, DT, LI, TLI).run();
}

PreservedAnalyses TransformSinAndCosCallsPass::run(Function &F,
                                               FunctionAnalysisManager &AM) {
  auto &DT  = AM.getResult<DominatorTreeAnalysis>(F);
  auto &LI  = AM.getResult<LoopAnalysis>(F);
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);

  bool Changed = runImpl(F, DT, LI, TLI);
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
  auto &DT  = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  auto &LI  = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  auto &TLI = getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
  NumTransformedSinAndCosCalls = 0;

  return Impl.runImpl(F, DT, LI, TLI);
}

INITIALIZE_PASS_BEGIN(TransformSinAndCosCallsLegacyPass, DEBUG_TYPE,
                "Transform sin and cos calls", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(TransformSinAndCosCallsLegacyPass, DEBUG_TYPE,
                "Transform sin and cos calls", false, false)

/*
 * Public interface to the pass. Currently uses legacy (old) pass
 * manager.
 */

FunctionPass *llvm::createTransformSinAndCosCallsPass() {
  return new TransformSinAndCosCallsLegacyPass();
}
