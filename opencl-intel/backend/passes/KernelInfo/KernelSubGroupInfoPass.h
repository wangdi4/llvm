//  INTEL CONFIDENTIAL
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

#ifndef __KERNEL_SUB_GROUP_INFO_H
#define __KERNEL_SUB_GROUP_INFO_H

#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

using namespace llvm;

namespace intel {

  class KernelSubGroupInfo : public ModulePass {
  public:

    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief Constructor
    KernelSubGroupInfo() : ModulePass(ID) {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const override {
      return "KernelSubGroupInfo";
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<CallGraphWrapperPass>();
      AU.setPreservesAll();
    }

    /// @brief performs KernelSubGroupInfo pass on the module
    bool runOnModule(Module &M) override;
  };

  class KernelSubGroupInfoPass : public PassInfoMixin<KernelSubGroupInfoPass> {
  public:
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
  };

} // namespace intel

#endif // __KERNEL_SUB_GROUP_INFO_H
