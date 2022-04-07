// INTEL CONFIDENTIAL
//
// Copyright 2013-2018 Intel Corporation.
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

#include "PatchCallbackArgs.h"
#include "CompilationUtils.h"
#include "OCLAddressSpace.h"
#include "common_dev_limits.h"
#include "OCLPassSupport.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"

using namespace Intel::OpenCL::DeviceBackend;

extern "C" {
ModulePass *createPatchCallbackArgsPass(bool UseTLSGlobals) {
  return new intel::PatchCallbackArgs(UseTLSGlobals);
  }
}

namespace intel {

char PatchCallbackArgs::ID = 0;

OCL_INITIALIZE_PASS(PatchCallbackArgs, "patch-callback-args",
                    "Resolve OpenCL built-in calls to callbacks", false, false)

struct ImplicitArgAccessorFunc {
  const char * FuncName;
  // One of enum IMPLICIT_ARGS
  unsigned ArgId;
  // If the above is a structure, selects which field to access (currently only
  // applies to IA_WORK_GROUP_INFO)
  unsigned ArgSecondaryId;
};

ImplicitArgAccessorFunc ImplicitArgAccessorFuncList[] = {
  { "__get_block_to_kernel_mapper", ImplicitArgsUtils::IA_WORK_GROUP_INFO, NDInfo::BLOCK2KERNEL_MAPPER},
  { "__get_device_command_manager", ImplicitArgsUtils::IA_WORK_GROUP_INFO, NDInfo::RUNTIME_INTERFACE},
  { "__get_runtime_handle", ImplicitArgsUtils::IA_RUNTIME_HANDLE, NDInfo::LAST}
};

bool PatchCallbackArgs::runOnModule(Module &M) {
  ImplicitArgsAnalysisLegacy &IAA = getAnalysis<ImplicitArgsAnalysisLegacy>();
  ImplicitArgsInfo &IAInfo = IAA.getResult();
  bool Changed = false;
  SmallVector<CallInst*, 16> ToErase;
  for (unsigned I = 0, E = sizeof(ImplicitArgAccessorFuncList) /
                           sizeof(ImplicitArgAccessorFuncList[0]);
       I < E; ++I) {
    Function *CalledF = M.getFunction(ImplicitArgAccessorFuncList[I].FuncName);
    if (!CalledF)
      continue;
    assert(CalledF->isDeclaration() && "extern callback must be a declaration");
    for (Function::user_iterator UI = CalledF->user_begin(),
                                 UE = CalledF->user_end();
         UI != UE; ++UI) {
      CallInst *CI = dyn_cast<CallInst>(*UI);
      if (!CI)
        continue;
      Changed = true;
      Function *CallingF = CI->getParent()->getParent();
      ValuePair &ImplicitArgs = Func2ImplicitArgs[CallingF];
      if (!ImplicitArgs.first) {
        assert(!ImplicitArgs.second);
        // Need to create a cache entry for implicit arg values
        if (UseTLSGlobals) {
          ValuePair &ImplicitArgs = Func2ImplicitArgs[CallingF];
          Value *WorkInfo = CompilationUtils::getTLSGlobal(
              &M, ImplicitArgsUtils::IA_WORK_GROUP_INFO);
          Value *RuntimeHandle = CompilationUtils::getTLSGlobal(
              &M, ImplicitArgsUtils::IA_RUNTIME_HANDLE);
          assert(WorkInfo && "WorkInfo should be nullptr");
          assert(RuntimeHandle && "RuntimeHandle should not be null");
          IRBuilder<> B(
              dyn_cast<Instruction>(CallingF->getEntryBlock().begin()));
          ImplicitArgs.first = B.CreateLoad(
              WorkInfo->getType()->getPointerElementType(), WorkInfo);
          ImplicitArgs.second = B.CreateLoad(
              RuntimeHandle->getType()->getPointerElementType(), RuntimeHandle);
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
      switch (ImplicitArgAccessorFuncList[I].ArgId) {
      default:
        llvm_unreachable("Unhandled arguemnt ID");
      case ImplicitArgsUtils::IA_RUNTIME_HANDLE:
        Val = ImplicitArgs.second;
        break;
      case ImplicitArgsUtils::IA_WORK_GROUP_INFO: {
        unsigned NDInfoId = ImplicitArgAccessorFuncList[I].ArgSecondaryId;
        assert(NDInfoId == NDInfo::BLOCK2KERNEL_MAPPER ||
               NDInfoId == NDInfo::RUNTIME_INTERFACE);
        IRBuilder<> Builder(&*CallingF->getEntryBlock().begin());
        // When using tls insert after load instead
        if (UseTLSGlobals) {
          Builder.SetInsertPoint(
              dyn_cast<Instruction>(ImplicitArgs.first)->getNextNode());
        }
        Val = IAInfo.GenerateGetFromWorkInfo(NDInfoId, ImplicitArgs.first,
                                             Builder);
      } break;
      }
      if (Val->getType() != CI->getType())
        Val = CastInst::CreatePointerCast(Val, CI->getType(), "", CI);
      CI->replaceAllUsesWith(Val);
      ToErase.push_back(CI);
    }
  }
  for (unsigned I=0; I < ToErase.size(); ++I)
    ToErase[I]->eraseFromParent();
  return Changed;
}
}
