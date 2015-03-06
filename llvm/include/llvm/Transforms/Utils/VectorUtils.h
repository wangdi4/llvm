//===- llvm/Transforms/Utils/VectorUtils.h - Vector utilities -*- C++ -*-=====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines some vectorizer utilities.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_VECTORUTILS_H
#define LLVM_TRANSFORMS_UTILS_VECTORUTILS_H

#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Support/Debug.h"

#define LV_NAME "loop-vectorize"
#define DEBUG_TYPE LV_NAME

namespace llvm {

/// \brief Identify if the intrinsic is trivially vectorizable.
///
/// This method returns true if the intrinsic's argument types are all
/// scalars for the scalar form of the intrinsic and all vectors for
/// the vector form of the intrinsic.
static inline bool isTriviallyVectorizable(Intrinsic::ID ID) {
  switch (ID) {
  case Intrinsic::sqrt:
  case Intrinsic::sin:
  case Intrinsic::cos:
  case Intrinsic::exp:
  case Intrinsic::exp2:
  case Intrinsic::log:
  case Intrinsic::log10:
  case Intrinsic::log2:
  case Intrinsic::fabs:
  case Intrinsic::minnum:
  case Intrinsic::maxnum:
  case Intrinsic::copysign:
  case Intrinsic::floor:
  case Intrinsic::ceil:
  case Intrinsic::trunc:
  case Intrinsic::rint:
  case Intrinsic::nearbyint:
  case Intrinsic::round:
  case Intrinsic::bswap:
  case Intrinsic::ctpop:
  case Intrinsic::pow:
  case Intrinsic::fma:
  case Intrinsic::fmuladd:
  case Intrinsic::ctlz:
  case Intrinsic::cttz:
  case Intrinsic::powi:
    return true;
  default:
    return false;
  }
}

static bool hasVectorInstrinsicScalarOpd(Intrinsic::ID ID,
                                         unsigned ScalarOpdIdx) {
  switch (ID) {
    case Intrinsic::ctlz:
    case Intrinsic::cttz:
    case Intrinsic::powi:
      return (ScalarOpdIdx == 1);
    case Intrinsic::nrand48:
    case Intrinsic::erand48:
    case Intrinsic::jrand48:
      return (ScalarOpdIdx == 0);
    default:
      return false;
  }
}

static Intrinsic::ID checkUnaryFloatSignature(const CallInst &I,
                                              Intrinsic::ID ValidIntrinsicID) {
  if (I.getNumArgOperands() != 1 ||
      !I.getArgOperand(0)->getType()->isFloatingPointTy() ||
      I.getType() != I.getArgOperand(0)->getType() ||
      !I.onlyReadsMemory())
    return Intrinsic::not_intrinsic;

  return ValidIntrinsicID;
}

static Intrinsic::ID checkUnaryFloatReturningIntSignature(
                        const CallInst &I,
                        Intrinsic::ID ValidIntrinsicID)
{
  if (I.getNumArgOperands() != 1 || !I.getType()->isIntegerTy() ||
      !I.getArgOperand(0)->getType()->isFloatingPointTy() ||
      !I.onlyReadsMemory())
    return Intrinsic::not_intrinsic;

  return ValidIntrinsicID;
}

static Intrinsic::ID checkBinaryFloatSignature(
                        const CallInst &I,
                        Intrinsic::ID ValidIntrinsicID)
{
  if (I.getNumArgOperands() != 2 ||
      !I.getArgOperand(0)->getType()->isFloatingPointTy() ||
      !I.getArgOperand(1)->getType()->isFloatingPointTy() ||
      I.getType() != I.getArgOperand(0)->getType() ||
      I.getType() != I.getArgOperand(1)->getType() ||
      !I.onlyReadsMemory())
    return Intrinsic::not_intrinsic;

  return ValidIntrinsicID;
}

static Intrinsic::ID checkVoidReturningDoubleSignature(
                        const CallInst &I,
                        Intrinsic::ID ValidIntrinsicID)
{
  if (I.getNumArgOperands() != 0 || !I.getType()->isDoubleTy() ||
      !I.onlyReadsMemory()) {
    return Intrinsic::not_intrinsic;
  }

  return ValidIntrinsicID;
}

static Intrinsic::ID checkVoidReturningInt64Signature(
                        const CallInst &I,
                        Intrinsic::ID ValidIntrinsicID)
{
  if (I.getNumArgOperands() != 0 || !I.getType()->isIntegerTy(64) ||
      !I.onlyReadsMemory()) {
    return Intrinsic::not_intrinsic;
  }

  return ValidIntrinsicID;
}

static Intrinsic::ID checkPtrReturningInt64Signature(
                        const CallInst &I,
                        Intrinsic::ID ValidIntrinsicID)
{
  if (I.getNumArgOperands() != 1 || !I.getType()->isIntegerTy(64) ||
      !I.getArgOperand(0)->getType()->isPointerTy() ||
      !I.onlyReadsMemory()) {
    return Intrinsic::not_intrinsic;
  }

  return ValidIntrinsicID;
}

static Intrinsic::ID checkPtrReturningDoubleSignature(
                        const CallInst &I,
                        Intrinsic::ID ValidIntrinsicID)
{
  if (I.getNumArgOperands() != 1 || !I.getType()->isDoubleTy() ||
      !I.getArgOperand(0)->getType()->isPointerTy() ||
      !I.onlyReadsMemory()) {
    return Intrinsic::not_intrinsic;
  }

  return ValidIntrinsicID;
}

static Intrinsic::ID checkFloatIntReturningFloatSignature(
                        const CallInst &I,
                        Intrinsic::ID ValidIntrinsicID)
{
  if (I.getNumArgOperands() != 2 || !I.getType()->isFloatingPointTy() ||
      !I.getArgOperand(0)->getType()->isFloatingPointTy() ||
      !I.getArgOperand(1)->getType()->isIntegerTy(32) ||
      !I.onlyReadsMemory()) {
    return Intrinsic::not_intrinsic;
  }

  return ValidIntrinsicID;
}

static Intrinsic::ID checkFloatBinaryFloatPtrReturningVoid(
                        const CallInst &I,
                        Intrinsic::ID ValidIntrinsicID)
{
  Type *argOneType   = I.getArgOperand(0)->getType();
  Type *argTwoType   = I.getArgOperand(1)->getType();
  Type *argThreeType = I.getArgOperand(2)->getType();

  if (I.getNumArgOperands() != 3 || !I.getType()->isVoidTy() ||
      !argOneType->isFloatingPointTy() ||
      !argTwoType->isPointerTy() ||
      !argTwoType->getPointerElementType()->isFloatingPointTy() ||
      !argThreeType->isPointerTy() ||
      !argThreeType->getPointerElementType()->isFloatingPointTy() ||
      !I.onlyReadsMemory()) {
    return Intrinsic::not_intrinsic;
  }

  return ValidIntrinsicID;
}

static Intrinsic::ID
getIntrinsicIDForCall(CallInst *CI, const TargetLibraryInfo *TLI) {
  // If we have an intrinsic call, check if it is trivially vectorizable.
  if (IntrinsicInst *II = dyn_cast<IntrinsicInst>(CI)) {
    Intrinsic::ID ID = II->getIntrinsicID();
    if (isTriviallyVectorizable(ID) || ID == Intrinsic::lifetime_start ||
        ID == Intrinsic::lifetime_end || ID == Intrinsic::assume)
      return ID;
    else
      return Intrinsic::not_intrinsic;
  }

  if (!TLI)
    return Intrinsic::not_intrinsic;

  LibFunc::Func Func;
  Function *F = CI->getCalledFunction();
  // We're going to make assumptions on the semantics of the functions, check
  // that the target knows that it's available in this environment and it does
  // not have local linkage.
  if (!F || F->hasLocalLinkage() || !TLI->getLibFunc(F->getName(), Func))
    return Intrinsic::not_intrinsic;

  // Otherwise check if we have a call to a function that can be turned into a
  // vector intrinsic.
  // TODO: all cases where the ReadNone attribute is set explicitly need to be
  // retested when we figure out how to get all of the intel-specific math
  // library routines included correctly. Attributes should not have to be set
  // explicitly like this since the standard C ones work just fine as is.
  switch (Func) {
  default:
    break;
  case LibFunc::sin:
  case LibFunc::sinf:
  case LibFunc::sinl:
    return checkUnaryFloatSignature(*CI, Intrinsic::sin);
  case LibFunc::cos:
  case LibFunc::cosf:
  case LibFunc::cosl:
    return checkUnaryFloatSignature(*CI, Intrinsic::cos);
  case LibFunc::exp:
  case LibFunc::expf:
  case LibFunc::expl:
  case LibFunc::dunder_exp_finite:
  case LibFunc::dunder_expf_finite:
    return checkUnaryFloatSignature(*CI, Intrinsic::exp);
  case LibFunc::exp2:
  case LibFunc::exp2f:
  case LibFunc::exp2l:
  case LibFunc::dunder_exp2_finite:
  case LibFunc::dunder_exp2f_finite:
    return checkUnaryFloatSignature(*CI, Intrinsic::exp2);
  case LibFunc::log:
  case LibFunc::logf:
  case LibFunc::logl:
  case LibFunc::dunder_log_finite:
  case LibFunc::dunder_logf_finite:
    return checkUnaryFloatSignature(*CI, Intrinsic::log);
  case LibFunc::log10:
  case LibFunc::log10f:
  case LibFunc::log10l:
  case LibFunc::dunder_log10_finite:
  case LibFunc::dunder_log10f_finite:
    return checkUnaryFloatSignature(*CI, Intrinsic::log10);
  case LibFunc::log2:
  case LibFunc::log2f:
  case LibFunc::log2l:
  case LibFunc::dunder_log2_finite:
  case LibFunc::dunder_log2f_finite:
    return checkUnaryFloatSignature(*CI, Intrinsic::log2);
  case LibFunc::fabs: // Not currently in ICC
  case LibFunc::fabsf:
  case LibFunc::fabsl:
    return checkUnaryFloatSignature(*CI, Intrinsic::fabs);
  case LibFunc::fmin:
  case LibFunc::fminf:
  case LibFunc::fminl:
    return checkBinaryFloatSignature(*CI, Intrinsic::minnum);
  case LibFunc::fmax:
  case LibFunc::fmaxf:
  case LibFunc::fmaxl:
    return checkBinaryFloatSignature(*CI, Intrinsic::maxnum);
  case LibFunc::copysign: // Not currently in ICC
  case LibFunc::copysignf:
  case LibFunc::copysignl:
    return checkBinaryFloatSignature(*CI, Intrinsic::copysign);
  case LibFunc::floor:
  case LibFunc::floorf:
  case LibFunc::floorl:
    return checkUnaryFloatSignature(*CI, Intrinsic::floor);
  case LibFunc::ceil:
  case LibFunc::ceilf:
  case LibFunc::ceill:
    return checkUnaryFloatSignature(*CI, Intrinsic::ceil);
  case LibFunc::trunc:
  case LibFunc::truncf:
  case LibFunc::truncl:
    return checkUnaryFloatSignature(*CI, Intrinsic::trunc);
  case LibFunc::rint:
  case LibFunc::rintf:
  case LibFunc::rintl:
    return checkUnaryFloatSignature(*CI, Intrinsic::rint);
  case LibFunc::nearbyint:
  case LibFunc::nearbyintf:
  case LibFunc::nearbyintl:
    return checkUnaryFloatSignature(*CI, Intrinsic::nearbyint);
  case LibFunc::round:
  case LibFunc::roundf:
  case LibFunc::roundl:
    return checkUnaryFloatSignature(*CI, Intrinsic::round);
  case LibFunc::pow:
  case LibFunc::powf:
  case LibFunc::powl:
    return checkBinaryFloatSignature(*CI, Intrinsic::pow);
  case LibFunc::acosh:
  case LibFunc::acoshf:
  case LibFunc::dunder_acosh_finite:
  case LibFunc::dunder_acoshf_finite:
    return checkUnaryFloatSignature(*CI, Intrinsic::acosh);
  case LibFunc::asinh:
  case LibFunc::asinhf:
    return checkUnaryFloatSignature(*CI, Intrinsic::asinh);
  case LibFunc::atanh:
  case LibFunc::atanhf:
  case LibFunc::dunder_atanh_finite:
  case LibFunc::dunder_atanhf_finite:
    return checkUnaryFloatSignature(*CI, Intrinsic::atanh);
  case LibFunc::atan:
  case LibFunc::atanf:
    return checkUnaryFloatSignature(*CI, Intrinsic::atan);
  case LibFunc::erfc:
  case LibFunc::erfcf:
    return checkUnaryFloatSignature(*CI, Intrinsic::erfc);
  case LibFunc::expm1:
  case LibFunc::expm1f:
  case LibFunc::expm1l:
    return checkUnaryFloatSignature(*CI, Intrinsic::expm1);
  // May need to separate these into separate intrinsics. One for __isnan,
  // one for _isnan, and one for isnan.
  case LibFunc::dunder_isnanf:
  case LibFunc::dunder_isnan:
  case LibFunc::under_isnan:
  case LibFunc::under_isnanf:
  case LibFunc::isnan:
  case LibFunc::isnanf:
    CI->setDoesNotAccessMemory();
    return checkUnaryFloatReturningIntSignature(*CI, Intrinsic::isnan);
  case LibFunc::under_isinf:
  case LibFunc::under_isinff:
  case LibFunc::dunder_isinf:
  case LibFunc::dunder_isinff:
  case LibFunc::isinf:
  case LibFunc::isinff:
    CI->setDoesNotAccessMemory();
    return checkUnaryFloatReturningIntSignature(*CI, Intrinsic::isinf);
  case LibFunc::dunder_finite:
  case LibFunc::dunder_finitef:
    return checkUnaryFloatReturningIntSignature(*CI, Intrinsic::finite);
  case LibFunc::nextafter:
  case LibFunc::nextafterf:
    return checkBinaryFloatSignature(*CI, Intrinsic::nextafter);
  case LibFunc::drand48:
    // drand48 is only externed from stdlib.h. In LLVM, weak symbols will
    // not carry the ReadNone function attribute. This must be set explicitly
    // so that the type checking function does not fail.
    CI->setDoesNotAccessMemory();
    return checkVoidReturningDoubleSignature(*CI, Intrinsic::drand48);
  case LibFunc::lrand48:
    CI->setDoesNotAccessMemory();
    return checkVoidReturningInt64Signature(*CI, Intrinsic::lrand48);
  case LibFunc::cdfnorminv:
  case LibFunc::cdfnorminvf:
    CI->setDoesNotAccessMemory();
    return checkUnaryFloatSignature(*CI, Intrinsic::cdfnorminv);
  case LibFunc::tan:
  case LibFunc::tanf:
    return checkUnaryFloatSignature(*CI, Intrinsic::tan);
  case LibFunc::tand:
  case LibFunc::tandf:
    CI->setDoesNotAccessMemory();
    return checkUnaryFloatSignature(*CI, Intrinsic::tand);
  case LibFunc::asin:
  case LibFunc::asinf:
  case LibFunc::dunder_asin_finite:
  case LibFunc::dunder_asinf_finite:
    return checkUnaryFloatSignature(*CI, Intrinsic::asin);
  case LibFunc::mrand48:
    CI->setDoesNotAccessMemory();
    return checkVoidReturningInt64Signature(*CI, Intrinsic::mrand48);
  case LibFunc::remainder:
  case LibFunc::remainderf:
  case LibFunc::dunder_remainder_finite:
  case LibFunc::dunder_remainderf_finite:
    return checkBinaryFloatSignature(*CI, Intrinsic::remainder);
  case LibFunc::nrand48:
    CI->setDoesNotAccessMemory();
    return checkPtrReturningInt64Signature(*CI, Intrinsic::nrand48);
  case LibFunc::erand48:
    CI->setDoesNotAccessMemory();
    return checkPtrReturningDoubleSignature(*CI, Intrinsic::erand48);
  case LibFunc::hypot:
  case LibFunc::hypotf:
  case LibFunc::dunder_hypot_finite:
  case LibFunc::dunder_hypotf_finite:
    return checkBinaryFloatSignature(*CI, Intrinsic::hypot);
  case LibFunc::erfinv:
  case LibFunc::erfinvf:
    CI->setDoesNotAccessMemory();
    return checkUnaryFloatSignature(*CI, Intrinsic::erfinv);
  case LibFunc::log1p:
  case LibFunc::log1pf:
    return checkUnaryFloatSignature(*CI, Intrinsic::log1p);
  case LibFunc::tanh:
  case LibFunc::tanhf:
    return checkUnaryFloatSignature(*CI, Intrinsic::tanh);
  case LibFunc::sind:
  case LibFunc::sindf:
    CI->setDoesNotAccessMemory();
    return checkUnaryFloatSignature(*CI, Intrinsic::sind);
  case LibFunc::fdim:
  case LibFunc::fdimf:
    return checkBinaryFloatSignature(*CI, Intrinsic::fdim);
  case LibFunc::atan2:
  case LibFunc::atan2f:
  case LibFunc::dunder_atan2_finite:
  case LibFunc::dunder_atan2f_finite:
    return checkBinaryFloatSignature(*CI, Intrinsic::atan2);
  case LibFunc::cosh:
  case LibFunc::coshf:
  case LibFunc::dunder_cosh_finite:
  case LibFunc::dunder_coshf_finite:
    return checkUnaryFloatSignature(*CI, Intrinsic::cosh);
  case LibFunc::ldexp:
  case LibFunc::ldexpf:
  case LibFunc::scalbn:
  case LibFunc::scalbnf:
    return checkFloatIntReturningFloatSignature(*CI, Intrinsic::scalbn);
  case LibFunc::invsqrt:
  case LibFunc::invsqrtf:
    CI->setDoesNotAccessMemory();
    return checkUnaryFloatSignature(*CI, Intrinsic::invsqrt);
  case LibFunc::erfcinv:
  case LibFunc::erfcinvf:
    CI->setDoesNotAccessMemory();
    return checkUnaryFloatSignature(*CI, Intrinsic::erfcinv);
  case LibFunc::cbrt:
  case LibFunc::cbrtf:
    return checkUnaryFloatSignature(*CI, Intrinsic::cbrt);
  case LibFunc::sinh:
  case LibFunc::sinhf:
  case LibFunc::dunder_sinh_finite:
  case LibFunc::dunder_sinhf_finite:
    return checkUnaryFloatSignature(*CI, Intrinsic::sinh);
  case LibFunc::exp10:
  case LibFunc::exp10f:
    CI->setDoesNotAccessMemory();
    return checkUnaryFloatSignature(*CI, Intrinsic::exp10);
  case LibFunc::cosd:
  case LibFunc::cosdf:
    CI->setDoesNotAccessMemory();
    return checkUnaryFloatSignature(*CI, Intrinsic::cosd);
  case LibFunc::erf:
  case LibFunc::erff:
    return checkUnaryFloatSignature(*CI, Intrinsic::erf);
  case LibFunc::sincos:
  case LibFunc::sincosf:
    CI->setDoesNotAccessMemory();
    return checkFloatBinaryFloatPtrReturningVoid(*CI, Intrinsic::sincos);
  case LibFunc::jrand48:
    CI->setDoesNotAccessMemory();
    return checkPtrReturningInt64Signature(*CI, Intrinsic::jrand48);
  case LibFunc::acos:
  case LibFunc::acosf:
  case LibFunc::dunder_acos_finite:
  case LibFunc::dunder_acosf_finite:
    return checkUnaryFloatSignature(*CI, Intrinsic::acos);
  case LibFunc::ilogb:
  case LibFunc::ilogbf:
    return checkUnaryFloatReturningIntSignature(*CI, Intrinsic::ilogb);
  case LibFunc::fmod:
  case LibFunc::fmodf:
  case LibFunc::dunder_fmod_finite:
  case LibFunc::dunder_fmodf_finite:
    return checkBinaryFloatSignature(*CI, Intrinsic::fmod);
  case LibFunc::cdfnorm:
  case LibFunc::cdfnormf:
    CI->setDoesNotAccessMemory();
    return checkUnaryFloatSignature(*CI, Intrinsic::cdfnorm);
  case LibFunc::logb:
  case LibFunc::logbf:
    return checkUnaryFloatSignature(*CI, Intrinsic::logb);
  }

  return Intrinsic::not_intrinsic;
}

} // llvm namespace

#endif
