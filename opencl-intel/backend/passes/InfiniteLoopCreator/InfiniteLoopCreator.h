// INTEL CONFIDENTIAL
//
// Copyright 2017-2018 Intel Corporation.
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

#ifndef __INFINITE_LOOPS_CREATOR_H__
#define __INFINITE_LOOPS_CREATOR_H__

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

namespace intel {

class InfiniteLoopCreator : public llvm::ModulePass {
public:
  static char ID;

  InfiniteLoopCreator();

  llvm::StringRef getPassName() const override { return "InfiniteLoopCreator"; }

  bool runOnModule(llvm::Module &M) override;

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;

private:
  bool runOnFunction(llvm::Function *F);
};

} // namespace intel

#endif // __INFINITE_LOOPS_CREATOR_H__
