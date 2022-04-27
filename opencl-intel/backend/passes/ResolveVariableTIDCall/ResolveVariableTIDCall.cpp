// Copyright 2020-2021 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "ResolveVariableTIDCall.h"

#include "CompilationUtils.h"
#include "OCLPassSupport.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/KernelBarrierUtils.h"

#define DEBUG_TYPE "resolve-variable-tid-call"

using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

char ResolveVariableTIDCall::ID = 0;

OCL_INITIALIZE_PASS(ResolveVariableTIDCall, "resolve-variable-tid-call",
                    "Resolve Variable TID Calls", false, false)

ResolveVariableTIDCall::ResolveVariableTIDCall()
    : ModulePass(ID), M(nullptr), ConstZero(nullptr) {}

// This transformation can remove the limitation for vectorizer and sub-group
// emulation when there is any variable TID call in the kernel.
bool ResolveVariableTIDCall::resolveVariableTIDCall(
    const std::string &TIDName,
    function_ref<Value *(unsigned, IRBuilderBase &Builder)> insertTID) {
  SmallVector<CallInst *, 4> VariableTIDCalls;
  auto *Fn = M->getFunction(TIDName);
  if (!Fn)
    return false;
  for (auto *U : Fn->users()) {
    auto *CI = dyn_cast<CallInst>(U);
    if (!CI)
      continue;
    if (isa<ConstantInt>(CI->getArgOperand(0)))
      continue;
    LLVM_DEBUG(dbgs() << "Found Variable TID Call:" << *CI << " in Function: "
                      << CI->getFunction()->getName() << "\n");
    VariableTIDCalls.push_back(CI);
  }

  if (VariableTIDCalls.empty())
    return false;

  auto &Context = M->getContext();

  for (auto *CI : VariableTIDCalls) {
    auto *F = CI->getFunction();
    auto *Arg = CI->getArgOperand(0);

    // Create BasicBlocks for every dimension.
    auto *BBDim0 = CI->getParent();
    auto *BBDimOOB = BBDim0->splitBasicBlock(CI, "tid.dim.oob");
    auto *BBAfter = BBDimOOB->splitBasicBlock(CI, "tid.dim.res");
    auto *BBDim1 = BasicBlock::Create(Context, "tid.dim1.", F, BBDimOOB);
    auto *BBDim2 = BasicBlock::Create(Context, "tid.dim2.", F, BBDimOOB);

    BBDim0->getTerminator()->eraseFromParent();

    BasicBlock *BBDims[] = {BBDim0, BBDim1, BBDim2, BBDimOOB};

    IRBuilder<> Builder(CI);
    auto *TIDPHI = Builder.CreatePHI(CI->getType(), 4);
    TIDPHI->addIncoming(ConstZero, BBDimOOB);

    // Create TID Calls.
    for (unsigned Dim = 0; Dim < 3; ++Dim) {
      Builder.SetInsertPoint(BBDims[Dim]);
      auto *TID = insertTID(Dim, Builder);
      auto *Cond = Builder.CreateICmpEQ(Arg, Builder.getInt32(Dim));
      Builder.CreateCondBr(Cond, BBAfter, BBDims[Dim + 1]);
      TIDPHI->addIncoming(TID, BBDims[Dim]);
    }

    CI->replaceAllUsesWith(TIDPHI);
    CI->eraseFromParent();
  }

  return true;
}

bool ResolveVariableTIDCall::runOnModule(Module &M) {
  this->M = &M;

  BarrierUtils Utils;
  Utils.init(&M);
  ConstZero = ConstantInt::get(Utils.getSizetTy(), 0);

  bool Changed = false;
  Changed |=
      resolveVariableTIDCall(CompilationUtils::mangledGetLID(),
                             [&Utils](unsigned Dim, IRBuilderBase &Builder) {
                               return Utils.createGetLocalId(Dim, Builder);
                             });
  Changed |=
      resolveVariableTIDCall(CompilationUtils::mangledGetGID(),
                             [&Utils](unsigned Dim, IRBuilderBase &Builder) {
                               return Utils.createGetGlobalId(Dim, Builder);
                             });
  return Changed;
}

} // namespace intel

extern "C" {
llvm::Pass *createResolveVariableTIDCallPass() {
  return new intel::ResolveVariableTIDCall();
}
}
