//==--- PipeIOTransformation.h - Replace pipe built-ins to io -*- C++ -*---==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef __PIPE_IO_TRANSFORMATION_H__
#define __PIPE_IO_TRANSFORMATION_H__

#include <llvm/IR/Module.h>

namespace intel {

class PipeIOTransformation : public llvm::ModulePass {
public:
  static char ID;
  PipeIOTransformation();

  virtual llvm::StringRef getPassName() const override {
    return "PipeIOTransformation";
  }

  bool runOnModule(llvm::Module &M) override;

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
};

} // namespace intel

#endif // __PIPE_IO_TRANSFORMATION_H__
