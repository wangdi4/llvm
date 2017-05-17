/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

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

namespace Intel {
class MetaDataUtils;
}
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
  bool runOnFunction(Function &F, Intel::MetaDataUtils &MDU);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    // Only modifies metadata
    AU.setPreservesAll();
    AU.addRequired<BuiltinLibInfo>();
  }
};

} // namespace intel {

#endif // __KERNEL_INFO_H__

