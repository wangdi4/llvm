/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __PATCH_CALLBACKS_H__
#define __PATCH_CALLBACKS_H__

#include "ImplicitArgsUtils.h"
#include "ImplicitArgsAnalysis/ImplicitArgsAnalysis.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/DenseMap.h"

using namespace llvm;

namespace intel {

// PatchCallbackArgs - Patches calls to external callbacks with arguemnts that
// are retrieved from the function's implicit arguemnts.
class PatchCallbackArgs : public ModulePass {
  // Maps a function to its pair of values holding the callback context and
  // the Runtime Handle, if such exists
  typedef std::pair<Value *, Value *> ValuePair;
  typedef DenseMap<Function *, ValuePair> Func2ValuePair;
  Func2ValuePair Func2ImplicitArgs;
public:
  static char ID;
  PatchCallbackArgs() : ModulePass(ID) {}
  virtual llvm::StringRef getPassName() const { return "PatchCallbackArgs"; }
  bool runOnModule(Module &M);
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<ImplicitArgsAnalysis>();
  }
};
}
#endif //__PATCH_CALLBACKS_H__
