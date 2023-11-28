//===- Target/X86/Intel_X86TransformToLibmSinCosCall.cpp -------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
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
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// This file defines the pass which transforms the following IRs:
/// call void @sincos(double %x, ptr %t1, ptr %t2)
/// ->
/// %call = call {double, double} @__libm_sse2_sincos(double %x)
/// %0 = extractvalue { double, double } %call, 0
/// %1 = extractvalue { double, double } %call, 1
/// store double %1 ptr %t1
/// store double %2 ptr %t2
//
//===----------------------------------------------------------------------===//
//

#include "X86.h"
#include "X86Subtarget.h"
#include "X86TargetMachine.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/InitializePasses.h"

using namespace llvm;

#define DEBUG_TYPE "transform-to-libm-sincos-call"

namespace {

static bool isGNUSinCosCall(CallInst *CI) {
  if (!CI->getCalledFunction())
    return false;
  if (CI->getCalledFunction()->getName() == "sincos") {
    if (CI->getNumOperands() != 4)
      return false;
    if (!CI->getOperand(0)->getType()->isDoubleTy())
      return false;
    if (!CI->getOperand(1)->getType()->isPointerTy())
      return false;
    if (!CI->getOperand(2)->getType()->isPointerTy())
      return false;
    if (!CI->getType()->isVoidTy())
      return false;
    return true;
  } else if (CI->getCalledFunction()->getName() == "sincosf") {
    if (CI->getNumOperands() != 4)
      return false;
    if (!CI->getOperand(0)->getType()->isFloatTy())
      return false;
    if (!CI->getOperand(1)->getType()->isPointerTy())
      return false;
    if (!CI->getOperand(2)->getType()->isPointerTy())
      return false;
    if (!CI->getType()->isVoidTy())
      return false;
    return true;
  }
  return false;
}

class X86TransformToLibmSinCosCallPass : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid.

  X86TransformToLibmSinCosCallPass() : FunctionPass(ID) {
    initializeX86TransformToLibmSinCosCallPassPass(
        *PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override;

private:
  bool getTargetInfo(const Function &F) {
    auto *TPC = getAnalysisIfAvailable<TargetPassConfig>();
    if (!TPC)
      return false;

    auto &TM = TPC->getTM<X86TargetMachine>();
    // FIXME: Replace IntelLibIRCAllowed with IntelLibMAllowed when the later
    // is also on by default.
    if (!TM.Options.IntelLibIRCAllowed)
      return false;
    if (!TM.Options.ApproxFuncFPMath)
      return false;

    ST = TM.getSubtargetImpl(F);
    return true;
  }

  // Process interested IR
  bool ProcessSinCosCall(CallInst *CI);

  const X86Subtarget *ST = nullptr;
};

} // end anonymous namespace

char X86TransformToLibmSinCosCallPass::ID = 0;

INITIALIZE_PASS_BEGIN(X86TransformToLibmSinCosCallPass, DEBUG_TYPE,
                      "transform GNU sincos call into libm version", false,
                      false)
INITIALIZE_PASS_END(X86TransformToLibmSinCosCallPass, DEBUG_TYPE,
                    "transform GNU sincos call into libm version", false, false)

FunctionPass *llvm::createX86TransformToLibmSinCosCallPass() {
  return new X86TransformToLibmSinCosCallPass();
}

bool X86TransformToLibmSinCosCallPass::ProcessSinCosCall(CallInst *CI) {
  IRBuilder<> Builder(CI);
  const char *Name = nullptr;
  Type *FloatTy = CI->getOperand(0)->getType();
  if (FloatTy->isDoubleTy())
    Name = "__libm_sse2_sincos";
  else if (FloatTy->isFloatTy())
    Name = "__libm_sse2_sincosf";
  else
    return false;
  auto *RetTy = StructType::create({FloatTy, FloatTy});
  FunctionCallee Func =
      CI->getModule()->getOrInsertFunction(Name, RetTy, FloatTy);
  Value *NewCall = Builder.CreateCall(Func, CI->getOperand(0));
  Value *Sin = Builder.CreateExtractValue(NewCall, 0, "sin");
  Value *Cos = Builder.CreateExtractValue(NewCall, 1, "cos");
  Builder.CreateStore(Sin, CI->getOperand(1));
  Builder.CreateStore(Cos, CI->getOperand(2));
  CI->eraseFromParent();
  return true;
}

bool X86TransformToLibmSinCosCallPass::runOnFunction(Function &F) {
  bool MadeChange = false;

  if (F.hasFnAttribute("no-builtins"))
    return false;

  if (!getTargetInfo(F))
    return false;

  // FIXME: When x86 32 bit calling conventions works return {float, float},
  // remove Subtarget.is64Bit().
  if (!ST->is64Bit())
    return false;

  if (!ST->hasSSE2())
    return false;

  // Put sincos calls into worklist
  SmallVector<Instruction *, 8> Worklist;
  for (BasicBlock *BB : depth_first(&F)) {
    for (BasicBlock::iterator BBI = BB->begin(), BBIE = BB->end(); BBI != BBIE;
         ++BBI) {
      auto *CI = dyn_cast<CallInst>(&*BBI);
      if (CI && isGNUSinCosCall(CI)) {
        Worklist.push_back(&*BBI);
      }
    }
  }
  // Process sincos calls in the worklist
  for (auto II : Worklist) {
    CallInst *CI = cast<CallInst>(&*II);
    MadeChange |= ProcessSinCosCall(CI);
  }
  return MadeChange;
}
