//===-- InferArgumentAlias.cpp - Infer function argument alias ------------===//
//
// Copyright (C) 2022 Intel Corporation
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
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/InferArgumentAlias.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/LoopUtils.h"

using namespace llvm;
using namespace CompilationUtils;
using namespace LoopUtils;

// isFunctionMayBeCalled - Returns true if the function may be called from
// with the module
static bool isFunctionMayBeCalled(const Function &F) {
  // If there may be Clang Blocks present in the module, pessimistically
  // assume this function is a callee
  if (hasOcl20Support(*F.getParent()))
    return true;
  for (auto *UI : F.users())
    if (isa<CallInst>(*UI))
      return true;
  return false;
}

// setNoAlias - Sets the NoAlias attribute to one or all arguments in 'V'. If
// ReturnAfterFirst is true, sets the NoAlias attribute to the first argument
// in V without this attribute, otherwise sets all arguments in V with the
// attribute. 'NoAlias' is passed in for efficiency.
static void setNoAlias(SmallVectorImpl<Argument *> &V, bool ReturnAfterFirst) {
  for (auto *AI : V) {
    if (!AI->hasNoAliasAttr()) {
      AI->addAttr(Attribute::NoAlias);
      if (ReturnAfterFirst)
        return;
    }
  }
}

static FuncSet getSyncUsersFuncs(Module &M) {
  // Get all synchronize built-ins declared in module
  FuncSet WGSyncBuiltins = getAllSyncBuiltinsDecls(M, /*IsWG*/ true);
  FuncSet SGSyncBuiltins = getAllSyncBuiltinsDecls(M, /*IsWG*/ false);

  FuncSet SyncBuiltins{WGSyncBuiltins.begin(), WGSyncBuiltins.end()};
  SyncBuiltins.insert(SGSyncBuiltins.begin(), SGSyncBuiltins.end());

  FuncSet SyncUsers;
  fillFuncUsersSet(SyncBuiltins, SyncUsers);
  return SyncUsers;
}

// AddNoAliasAttrs - Attempts to add the 'NoAlias' attribute to function
// arguments
static bool AddNoAliasAttrs(Function &F, bool isKernel, FuncSet &SyncUsers) {
  // Rules:
  // 1. If exists a kernel function, F, s.t. F is not called by other functions,
  // then any local memory argument of F is safe to be marked as NoAlias
  // 2. If the kernel is called by other function, treat local memory
  // arguments the same as other address spaces using the following rules
  // TODO: for now rules #1-2 are disabled in OpenCL 2.0 mode until we
  // understand how are they affected by dynamic NDRange
  // 3. Given there are N arguments from address space A and N-1 of these
  // arguments are marked as NoAlias, then mark the remaining argument as
  // NoAlias (holds true also for N=1)
  // 4. Constant and Global address spaces are considered to be same, since two
  // buffers may be initialized from the same host memory
  // 5. If there exist any arguments of the generic address space, we must
  // pessistically assume they will collide with other pointer arguments
  if (F.isDeclaration())
    return false;

  // The pointer may be written by other threads.
  if (SyncUsers.count(&F))
    return false;

  // Keeps the function's pointer arguments in address space buckets
  SmallVector<Argument *, 16> Args[ADDRESS_SPACE_LAST_STATIC + 1];
  // Keeps the number of function args with NoAlias per addrspace
  unsigned NumArgsNoAlias[ADDRESS_SPACE_LAST_STATIC + 1] = {0};
  // Analyze the function arguments
  for (auto &AI : F.args()) {
    if (!AI.getType()->isPointerTy())
      continue;
    PointerType *T = cast<PointerType>(AI.getType());
    unsigned AS = T->getAddressSpace();
    if (AS == ADDRESS_SPACE_GENERIC) {
      if (!AI.hasNoAliasAttr())
        return false;
      continue;
    }
    // Global and Constant address spaces will be sharing same bucket. See
    // comments above.
    if (AS == ADDRESS_SPACE_CONSTANT)
      AS = ADDRESS_SPACE_GLOBAL;
    Args[AS].push_back(&AI);
    if (AI.hasNoAliasAttr())
      NumArgsNoAlias[AS]++;
  }
  // Modify the arguments
  bool Changed = false;
  for (unsigned AS = 0; AS < ADDRESS_SPACE_LAST_STATIC + 1; ++AS) {
    if (Args[AS].size() && Args[AS].size() - 1 == NumArgsNoAlias[AS]) {
      setNoAlias(Args[AS], true);
      Changed = true;
    }
  }
  // local addrspace has its additional special rules as listed above
  if (!Args[ADDRESS_SPACE_LOCAL].size())
    return Changed;
  // If kernel function and is not called by any other functions, can set all
  // local args to NoAlias
  if (isKernel && !isFunctionMayBeCalled(F)) {
    setNoAlias(Args[ADDRESS_SPACE_LOCAL], false);
    Changed = true;
  }
  return Changed;
}

static bool runOnFunction(Function &F, bool isKernel, FuncSet &SyncUsers) {
  return AddNoAliasAttrs(F, isKernel, SyncUsers);
}

bool InferArgumentAliasPass::runImpl(Module &M) {
  FuncSet Kernels = getAllKernels(M);
  FuncSet SyncUsers = getSyncUsersFuncs(M);

  bool Changed = false;
  for (auto &F : M)
    Changed |= runOnFunction(F, Kernels.count(&F), SyncUsers);
  return Changed;
}

PreservedAnalyses InferArgumentAliasPass::run(Module &M,
                                               ModuleAnalysisManager &MAM) {
  return runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
