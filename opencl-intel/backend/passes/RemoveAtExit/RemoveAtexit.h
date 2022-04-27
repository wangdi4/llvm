// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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

#ifndef __REMOVE_ATEXIT_H__
#define __REMOVE_ATEXIT_H__

#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

namespace intel {
using namespace llvm;

class RemoveAtExit : public ModulePass {
public:
  static char ID;

  RemoveAtExit();

  llvm::StringRef getPassName() const override { return "RemoveAtExit"; }

  bool runOnModule(Module &M) override;

protected:


  bool runOnFunction(Function *F);
};
}
#endif // __REMOVE_ATEXIT_H__
