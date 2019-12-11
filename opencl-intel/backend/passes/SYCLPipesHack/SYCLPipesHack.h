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
#ifndef __SYCLPIPESHACK_H__
#define __SYCLPIPESHACK_H__

#include <llvm/IR/Module.h>
#include <llvm/Pass.h>

namespace intel {
class SYCLPipesHack : public llvm::ModulePass {
public:
  static char ID;
  SYCLPipesHack();

  llvm::StringRef getPassName() const override {
    return "SYCLPipesHack";
  }

  bool runOnModule(llvm::Module &M) override;
  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
};
} // namespace intel


#endif
