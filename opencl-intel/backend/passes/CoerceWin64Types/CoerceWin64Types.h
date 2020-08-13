// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
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

#ifndef __COERCE_WIN64_TYPES_H__
#define __COERCE_WIN64_TYPES_H__

#include "llvm/Pass.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

namespace intel {
using namespace llvm;

class CoerceWin64Types : public ModulePass {
public:
  static char ID;

  CoerceWin64Types();

  llvm::StringRef getPassName() const override { return "CoerceWin64Types"; }

  bool runOnModule(Module &M) override;
private:
  bool runOnFunction(Function *F);
  Module *m_pModule;
  const DataLayout *m_pDataLayout;
  DenseMap<Function *, Function *> m_FunctionMap;
};

}
#endif // __COERCE_WIN64_TYPES_H__
