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

#ifndef __RESOLVE_VARIABLE_TID_CALL__
#define __RESOLVE_VARIABLE_TID_CALL__

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Pass.h"

#include <string>

using namespace llvm;

namespace intel {

class ResolveVariableTIDCall : public ModulePass {

public:
  static char ID;

  ResolveVariableTIDCall();

  StringRef getPassName() const override { return "ResolveVariableTIDCall"; }

  bool runOnModule(Module &M) override;

private:
  Module *M;

  Value *ConstZero;

  bool resolveVariableTIDCall(
      const std::string &TIDName,
      function_ref<Value *(unsigned, IRBuilderBase &Builder)> insertTID);
};
} // namespace intel

#endif
