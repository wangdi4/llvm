/*=================================================================================
Copyright (c) 2013, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#define DEBUG_TYPE "resolve-wi-call"
#include "PatchCallbackArgs.h"
#include "CompilationUtils.h"
#include "OCLAddressSpace.h"
#include "common_dev_limits.h"
#include "OCLPassSupport.h"
#include "MetaDataApi.h"

#include "llvm/Support/InstIterator.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"

using namespace Intel::OpenCL::DeviceBackend;

extern "C" {
  ModulePass* createPatchCallbackArgsPass() {
    return new intel::PatchCallbackArgs();
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
  ImplicitArgsAnalysis &IAA = getAnalysis<ImplicitArgsAnalysis>();
  unsigned PointerSize = M.getDataLayout()->getPointerSizeInBits(0);
  IAA.initDuringRun(PointerSize);
  bool Changed = false;
  SmallVector<CallInst*, 16> ToErase;
  for (unsigned I = 0, E = sizeof(ImplicitArgAccessorFuncList) /
                           sizeof(ImplicitArgAccessorFuncList[0]);
       I < E; ++I) {
    Function *CalledF = M.getFunction(ImplicitArgAccessorFuncList[I].FuncName);
    if (!CalledF)
      continue;
    assert(CalledF->isDeclaration() && "extern callback must be a declaration");
    for (Function::use_iterator UI = CalledF->use_begin(),
                                UE = CalledF->use_end();
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
        Argument *WorkInfo; // Used to access CallbackContext
        Argument *RuntimeHandle; // Needed by some callbacks
        CompilationUtils::getImplicitArgs(CallingF, NULL, &WorkInfo, NULL, NULL,
                                          NULL, &RuntimeHandle);
        ImplicitArgs.first = WorkInfo;
        ImplicitArgs.second = RuntimeHandle;
      }
      assert(ImplicitArgs.second);
      Value *Val = 0;
      switch (ImplicitArgAccessorFuncList[I].ArgId) {
      default:
        assert(false && "Unhandled arguemnt ID");
      case ImplicitArgsUtils::IA_RUNTIME_HANDLE:
        Val = ImplicitArgs.second;
        break;
      case ImplicitArgsUtils::IA_WORK_GROUP_INFO: {
        unsigned NDInfoId = ImplicitArgAccessorFuncList[I].ArgSecondaryId;
        assert(NDInfoId == NDInfo::BLOCK2KERNEL_MAPPER ||
               NDInfoId == NDInfo::RUNTIME_INTERFACE);
        IRBuilder<> Builder(CallingF->getEntryBlock().begin());
        Val =
            IAA.GenerateGetFromWorkInfo(NDInfoId, ImplicitArgs.first, Builder);
      } break;
      }
      if (Val->getType() != CI->getType())
        Val = new BitCastInst(Val, CI->getType(), "", CI);
      CI->replaceAllUsesWith(Val);
      ToErase.push_back(CI);
    }
  }
  for (unsigned I=0; I < ToErase.size(); ++I)
    ToErase[I]->eraseFromParent();
  return Changed;
}
}
