//===-- PatchCallbackArgs.cpp - Resolve builtin call to callbacks ---------===//
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

#include "llvm/Transforms/SYCLTransforms/PatchCallbackArgs.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ImplicitArgsUtils.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-patch-callback-args"

struct ImplicitArgAccessorFunc {
  const char *FuncName;
  // One of enum IMPLICIT_ARGS
  unsigned ArgId;
  // If the above is a structure, selects which field to access (currently only
  // applies to IA_WORK_GROUP_INFO)
  unsigned ArgSecondaryId;
};

static ImplicitArgAccessorFunc ImplicitArgAccessorFuncList[] = {
    {"__get_block_to_kernel_mapper", ImplicitArgsUtils::IA_WORK_GROUP_INFO,
     NDInfo::BLOCK2KERNEL_MAPPER},
    {"__get_device_command_manager", ImplicitArgsUtils::IA_WORK_GROUP_INFO,
     NDInfo::RUNTIME_INTERFACE},
    {"__get_runtime_handle", ImplicitArgsUtils::IA_RUNTIME_HANDLE,
     NDInfo::LAST}};

PreservedAnalyses PatchCallbackArgsPass::run(Module &M,
                                             ModuleAnalysisManager &MAM) {
  ImplicitArgsInfo *IAInfo = &MAM.getResult<ImplicitArgsAnalysis>(M);
  if (!runImpl(M, IAInfo))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<ImplicitArgsAnalysis>();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

bool PatchCallbackArgsPass::runImpl(Module &M, ImplicitArgsInfo *IAInfo) {
  bool Changed = false;
  bool HasTLSGlobals = CompilationUtils::hasTLSGlobals(M);
  SmallVector<CallInst *, 16> ToErase;
  for (const ImplicitArgAccessorFunc &IAAF : ImplicitArgAccessorFuncList) {
    Function *CalledF = M.getFunction(IAAF.FuncName);
    if (!CalledF)
      continue;
    assert(CalledF->isDeclaration() && "extern callback must be a declaration");
    for (auto *UI : CalledF->users()) {
      CallInst *CI = dyn_cast<CallInst>(UI);
      if (!CI)
        continue;
      Changed = true;
      Function *CallingF = CI->getFunction();
      ValuePair &ImplicitArgs = FuncToImplicitArgs[CallingF];
      if (!ImplicitArgs.first) {
        assert(!ImplicitArgs.second);
        // Need to create a cache entry for implicit arg values
        if (HasTLSGlobals) {
          ValuePair &ImplicitArgs = FuncToImplicitArgs[CallingF];
          GlobalValue *WorkInfo = CompilationUtils::getTLSGlobal(
              &M, ImplicitArgsUtils::IA_WORK_GROUP_INFO);
          GlobalValue *RuntimeHandle = CompilationUtils::getTLSGlobal(
              &M, ImplicitArgsUtils::IA_RUNTIME_HANDLE);
          assert(WorkInfo && "WorkInfo should be nullptr");
          assert(RuntimeHandle && "RuntimeHandle should not be null");
          IRBuilder<> B(&*CallingF->getEntryBlock().begin());
          ImplicitArgs.first = B.CreateLoad(WorkInfo->getValueType(), WorkInfo);
          ImplicitArgs.second =
              B.CreateLoad(RuntimeHandle->getValueType(), RuntimeHandle);
        } else {
          Value *WorkInfo;      // Used to access CallbackContext
          Value *RuntimeHandle; // Needed by some callbacks
          CompilationUtils::getImplicitArgs(CallingF, nullptr, &WorkInfo,
                                            nullptr, nullptr, nullptr,
                                            &RuntimeHandle);
          ImplicitArgs.first = WorkInfo;
          ImplicitArgs.second = RuntimeHandle;
        }
      }
      assert(ImplicitArgs.second);
      Value *Val = 0;
      switch (IAAF.ArgId) {
      default:
        llvm_unreachable("Unhandled arguemnt ID");
      case ImplicitArgsUtils::IA_RUNTIME_HANDLE:
        Val = ImplicitArgs.second;
        break;
      case ImplicitArgsUtils::IA_WORK_GROUP_INFO: {
        unsigned NDInfoId = IAAF.ArgSecondaryId;
        assert(NDInfoId == NDInfo::BLOCK2KERNEL_MAPPER ||
               NDInfoId == NDInfo::RUNTIME_INTERFACE);
        IRBuilder<> Builder(&*CallingF->getEntryBlock().begin());
        // When using tls insert after load instead
        if (HasTLSGlobals) {
          Builder.SetInsertPoint(
              cast<Instruction>(ImplicitArgs.first)->getNextNode());
        }
        Val = IAInfo->GenerateGetFromWorkInfo(NDInfoId, ImplicitArgs.first,
                                              Builder);
      } break;
      }
      if (Val->getType() != CI->getType())
        Val = CastInst::CreatePointerCast(Val, CI->getType(), "", CI);
      CI->replaceAllUsesWith(Val);
      ToErase.push_back(CI);
    }
  }
  for (auto *CI : ToErase)
    CI->eraseFromParent();
  return Changed;
}
