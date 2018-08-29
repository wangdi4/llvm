// INTEL CONFIDENTIAL
//
// Copyright 2018 Intel Corporation.
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
