//==--- InfiniteLoopCreator.h - InfiniteLoopCreator pass       -*- C++ -*---==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef __INFINITE_LOOPS_CREATOR_H__
#define __INFINITE_LOOPS_CREATOR_H__

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

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
