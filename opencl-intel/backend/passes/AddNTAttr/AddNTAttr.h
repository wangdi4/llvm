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

#ifndef __ADDNTATTR_H__
#define __ADDNTATTR_H__

#include "llvm/Analysis/Passes.h"
#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

namespace intel {

class AddNTAttr : public FunctionPass {
public:
  static char ID;

  AddNTAttr();

  StringRef getPassName() const override {
    return "AddNTAttr";
  }

  bool runOnFunction(Function &func) override;

  void getAnalysisUsage(AnalysisUsage &aU) const override;

private:
  Function *m_F;

  bool setNTAttr(StoreInst*sI);
};

} // namespace intel

#endif // __ADDNTATTR_H__
