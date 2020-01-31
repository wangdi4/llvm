//===-- X86TargetInfo.cpp - X86 Target Implementation ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/X86TargetInfo.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheX86_32Target() {
  static Target TheX86_32Target;
  return TheX86_32Target;
}
Target &llvm::getTheX86_64Target() {
  static Target TheX86_64Target;
  return TheX86_64Target;
}
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ICECODE
Target &llvm::getTheX86_IceCodeTarget() {
  static Target TheX86_IceCodeTarget;
  return TheX86_IceCodeTarget;
}
#endif  // INTEL_FEATURE_ICECODE
#endif  // INTEL_CUSTOMIZATION

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeX86TargetInfo() {
  RegisterTarget<Triple::x86, /*HasJIT=*/true> X(
      getTheX86_32Target(), "x86", "32-bit X86: Pentium-Pro and above", "X86");

  RegisterTarget<Triple::x86_64, /*HasJIT=*/true> Y(
      getTheX86_64Target(), "x86-64", "64-bit X86: EM64T and AMD64", "X86");

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ICECODE
  RegisterTarget<Triple::x86_icecode, /*HasJIT=*/true> Z(
      getTheX86_IceCodeTarget(), "x86-icecode", "64-bit X86: IceCode", "X86");
#endif  // INTEL_FEATURE_ICECODE
#endif  // INTEL_CUSTOMIZATION
}
