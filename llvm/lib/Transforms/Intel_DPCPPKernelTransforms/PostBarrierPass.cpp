//==--- PostBarrierPass.cpp - Resolve builtins created by Barrier - C++ -*--==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/PostBarrierPass.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelBarrierUtils.h"

#define DEBUG_TYPE "dpcpp-kernel-post-barrier"

using namespace llvm;

INITIALIZE_PASS(
    PostBarrier, DEBUG_TYPE,
    "DPCPP Post Barrier Pass - Resolve builtins created by Barrier pass", false,
    true)

namespace llvm {

char PostBarrier::ID = 0;

PostBarrier::PostBarrier() : ModulePass(ID) {}

bool PostBarrier::runOnModule(Module &M) {
  bool Changed = false;

  Changed |= replaceGetLocalSizeCall(M);
  Changed |= resolveSpecialBufferCall(M);

  return Changed;
}

bool PostBarrier::replaceGetLocalSizeCall(Module &M) {
  Function *GetLocalSize = M.getFunction("__builtin_get_local_size");

  if (!GetLocalSize) {
    LLVM_DEBUG(dbgs() << "No __builtin_get_local_size calls present!\n");
    return false;
  }

  std::map<Instruction *, Value *> LocalSizeValueMap;
  for (auto *U : GetLocalSize->users()) {
    CallInst *CI = dyn_cast<CallInst>(U);
    // Usage can be a global variable.
    if (!CI)
      continue;

    assert((CI->getNumArgOperands() == 1) &&
           "__builtin_get_local_size has to have an argument!");
    ConstantInt *C = dyn_cast<ConstantInt>(CI->getArgOperand(0));
    assert(C && "tid arg must be constant");
    unsigned Dim = C->getValue().getZExtValue();
    assert(Dim < 3 && "tid is not in range");

    Function *Kernel = CI->getFunction();
    assert((Kernel->hasFnAttribute("sycl_kernel") ||
            Kernel->hasFnAttribute("scalar_kernel")) &&
           "This has to be a scalar or vector kernel!");

    // 3+0 arg is local size 0, 3+1 is 1, 3+2 is dim 2.
    LocalSizeValueMap[CI] = Kernel->getArg(3 + Dim);
  }

  for (auto &KV : LocalSizeValueMap) {
    LLVM_DEBUG(dbgs() << "Replace \n"; KV.first->dump(); dbgs() << " with \n";
               KV.second->dump());
    KV.first->replaceAllUsesWith(KV.second);
    KV.first->eraseFromParent();
  }

  return !LocalSizeValueMap.empty();
}

bool PostBarrier::resolveSpecialBufferCall(Module &M) {
  Function *GetSpecialBuffer = M.getFunction(GET_SPECIAL_BUFFER);

  // In case there's no calls, exit.
  if (!GetSpecialBuffer) {
    LLVM_DEBUG(dbgs() << "KernelBarrier: No " GET_SPECIAL_BUFFER
                         " calls present\n");
    return false;
  }

  std::map<Instruction *, Value *> SpecialBufferMap;
  for (auto *U : GetSpecialBuffer->users()) {
    CallInst *CI = dyn_cast<CallInst>(U);
    // Usage can be a global variable.
    if (!CI)
      continue;

    Function *Kernel = CI->getFunction();
    assert((Kernel->hasFnAttribute("sycl_kernel") ||
            Kernel->hasFnAttribute("scalar_kernel")) &&
           "This has to be a scalar or vector kernel!");

    IRBuilder<> Builder(&*(CI->getParent()->begin()));
    assert(Kernel->hasFnAttribute("dpcpp-kernel-barrier-buffer-size") &&
           "Kernel must have dpcpp-kernel-barrier-buffer-size attr!");
    unsigned BufferSize;
    bool Res =
        to_integer(Kernel->getFnAttribute("dpcpp-kernel-barrier-buffer-size")
                       .getValueAsString(),
                   BufferSize);
    (void)Res;
    assert(Res && "KernelBarrier: dpcpp-kernel-barrier-buffer-size has to have "
                  "a numeric value!");

    const DataLayout &DL = M.getDataLayout();
    unsigned int SizeT = DL.getPointerSizeInBits(0);
    Type *SizeTTy = IntegerType::get(M.getContext(), SizeT);
    Value *BarrierBufferSize = ConstantInt::get(SizeTTy, BufferSize);
    for (unsigned Dim = 0; Dim < 3; ++Dim)
      BarrierBufferSize =
          Builder.CreateMul(BarrierBufferSize, Kernel->getArg(3 + Dim));
    BarrierBufferSize->setName("BarrierBufferSize");
    const auto AllocaAddrSpace = DL.getAllocaAddrSpace();
    AllocaInst *BarrierBuffer = new AllocaInst(
        Type::getInt8Ty(M.getContext()), AllocaAddrSpace, BarrierBufferSize,
        Align(128)); // 128 taken from CPU
    Builder.Insert(BarrierBuffer);
    SpecialBufferMap[CI] = BarrierBuffer;
  }

  for (auto &KV : SpecialBufferMap) {
    LLVM_DEBUG(dbgs() << "Replace \n"; KV.first->dump(); dbgs() << " with \n";
               KV.second->dump(););
    KV.first->replaceAllUsesWith(KV.second);
    KV.first->eraseFromParent();
  }

  return !SpecialBufferMap.empty();
}

ModulePass *createPostBarrierPass() { return new llvm::PostBarrier(); }

} // namespace llvm
