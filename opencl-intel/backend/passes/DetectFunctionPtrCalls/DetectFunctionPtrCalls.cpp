/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#define DEBUG_TYPE "detectfuncptrcalls"

#include "DetectFunctionPtrCalls.h"
#include "MetadataAPI.h"
#include "OCLPassSupport.h"

#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>

using namespace Intel;
using namespace llvm;
using namespace Intel::MetadataAPI;

extern "C" {
Pass *createDetectFuncPtrCalls() { return new intel::DetectFuncPtrCalls(); }
}

namespace intel {

char DetectFuncPtrCalls::ID = 0;
OCL_INITIALIZE_PASS(DetectFuncPtrCalls, "detectfuncptrcall",
                    "Detect Function Pointer Calls", false, false)

DetectFuncPtrCalls::DetectFuncPtrCalls()
    : ModulePass(ID), m_DetectedFuncPtrCall(false) {}

bool DetectFuncPtrCalls::runOnModule(Module &M) {

  m_DetectedFuncPtrCall = false;
  m_funcWithFuncPtrCall.clear();

  for (auto &Func : M) {
    if (DetectInFunction(Func)) {
      FunctionMetadataAPI(&Func).FuncPtrCall.set(true);
      m_DetectedFuncPtrCall = true;
    }
  }

  // The Metadata was changed, but it should not have
  // impact significant enough to consider something changed.
  return false;
}

// print out results
void DetectFuncPtrCalls::print(raw_ostream &O, const Module *M) const {
  if (m_DetectedFuncPtrCall) {
    O << "DetectFuncPtrCalls: Found function pointer calls.\n";
    O << "DetectFuncPtrCalls: Functions with unresolved pointer calls:\n";

    for (auto &Func : *const_cast<Module *>(M)) {
      auto funcPtrCallInfo = FunctionMetadataAPI(&Func).FuncPtrCall;
      if (funcPtrCallInfo.hasValue() && funcPtrCallInfo.get()) {
        O << Func.getName() << ".\n";
      }
    }
  } else
    O << "DetectFuncPtrCalls: not found function pointer calls.\n";
}

bool DetectFuncPtrCalls::DetectInFunction(Function &F) {
  for (auto &I : instructions(F)) {
    if (const CallInst *CI = dyn_cast<CallInst>(&I)) {
      if (nullptr == CI->getCalledFunction()) {
        return true;
      }
    }
  }

  return false;
}

} // namespace intel
