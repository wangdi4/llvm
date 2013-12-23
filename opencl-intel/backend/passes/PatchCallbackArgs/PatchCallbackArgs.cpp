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
#include "CallbackDesc.h"

#include "llvm/Support/InstIterator.h"
#include "llvm/Version.h"
#if LLVM_VERSION == 3425
#include "llvm/Target/TargetData.h"
#else
#include "llvm/DataLayout.h"
#endif
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IRBuilder.h"

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

bool PatchCallbackArgs::runOnModule(Module &M) {
  ImplicitArgsAnalysis &IAA = getAnalysis<ImplicitArgsAnalysis>();
  unsigned PointerSize = getAnalysis<DataLayout>().getPointerSizeInBits();
  IAA.initDuringRun(PointerSize);
  bool Changed = false;
  // Iterate over all 
  unsigned CallbackLookupCount = sizeof(CallbackLookup) / sizeof(CallbackLookup[0]);
  for (unsigned I = 0; I < CallbackLookupCount; ++I) {
    Function *CalledF = M.getFunction(CallbackLookup[I].CallbackName);
    if (!CalledF) continue;
    assert(CalledF->isDeclaration() && "extern callback must be a declaration");
    for (Function::use_iterator UI = CalledF->use_begin(),
                                UE = CalledF->use_end();
         UI != UE; ++UI) {
      CallInst *CI = dyn_cast<CallInst>(*UI);
      if (!CI) continue;
      Changed = true;
      Function *CallingF = CI->getParent()->getParent();
      ValuePair &ImplicitArgs = Func2ImplicitArgs[CallingF];
      if (!ImplicitArgs.first) {
        assert(!ImplicitArgs.second);
        // Need to create a cache entry for implicit arg values
        Argument *WorkInfo; // Used to access CallbackContext
        Argument *RuntimeHandle; // Needed by some callbacks
        CompilationUtils::getImplicitArgs(CallingF, NULL, &WorkInfo, NULL, NULL,
                                          NULL, NULL, &RuntimeHandle);
        IRBuilder<> Builder(CallingF->getEntryBlock().begin());
        ImplicitArgs.first = IAA.GenerateGetFromWorkInfo(
            NDInfo::RUNTIME_CALLBACKS, WorkInfo, Builder);
        ImplicitArgs.second = RuntimeHandle;
      }
      assert(ImplicitArgs.second);
      // Now we have the required values extracted from the implicit args
      // Patch the calls
      unsigned CallbackContextArgIdx = CI->getNumArgOperands() - 1;
      unsigned RuntimeHandleArgIdx = 0;
      if (CallbackLookup[I].NeedRuntimeHandleArg) {
        RuntimeHandleArgIdx = CallbackContextArgIdx--;
      }
      // Replace the Callback context argument
      Value *NewOp = ImplicitArgs.first;
      if (NewOp->getType() !=
          CI->getArgOperand(CallbackContextArgIdx)->getType()) {
        NewOp = new BitCastInst(
            NewOp, CI->getArgOperand(CallbackContextArgIdx)->getType(), "", CI);
      }
      // Replace the Runtime handle argument
      CI->setArgOperand(CallbackContextArgIdx, NewOp);
      if (RuntimeHandleArgIdx) {
        Value * NewOp = ImplicitArgs.second;
        if (NewOp->getType() !=
            CI->getArgOperand(RuntimeHandleArgIdx)->getType()) {
          NewOp = new BitCastInst(
              NewOp, CI->getArgOperand(RuntimeHandleArgIdx)->getType(), "", CI);
        }
        CI->setArgOperand(RuntimeHandleArgIdx, ImplicitArgs.second);
      }
    }
  }
  return Changed;
}
}
