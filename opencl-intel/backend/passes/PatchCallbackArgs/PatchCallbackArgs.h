// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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
