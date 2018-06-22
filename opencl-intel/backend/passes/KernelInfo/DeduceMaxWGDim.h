// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __KERNEL_INFO_H__
#define __KERNEL_INFO_H__

#include "BuiltinLibInfo.h"
#include "CompilationUtils.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/LoopInfo.h"

#include <set>

using namespace llvm;

using namespace Intel::OpenCL::DeviceBackend;
namespace intel {

class OpenclRuntime;

class DeduceMaxWGDim : public ModulePass {
  std::set<Function *> ForbiddenFuncUsers;
  const OpenclRuntime *RT;
  const std::string MangledGetGID; 
  const std::string MangledGetLID; 

public:
  /// Pass identification, replacement for typeid
  static char ID;

  DeduceMaxWGDim()
      : ModulePass(ID), MangledGetGID(CompilationUtils::mangledGetGID()),
        MangledGetLID(CompilationUtils::mangledGetLID()) {}

  virtual llvm::StringRef getPassName() const { return "DeduceMaxWGDim"; }

  bool runOnModule(Module &M);
  bool runOnFunction(Function &F);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    // Only modifies metadata
    AU.setPreservesAll();
    AU.addRequired<BuiltinLibInfo>();
  }
};

} // namespace intel {

#endif // __KERNEL_INFO_H__

