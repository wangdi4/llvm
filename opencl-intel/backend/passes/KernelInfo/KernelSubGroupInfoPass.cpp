// INTEL CONFIDENTIAL
//
// Copyright 2012-2019 Intel Corporation.
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

#include "KernelSubGroupInfoPass.h"
#include "CompilationUtils.h"
#include "MetadataAPI.h"
#include "OCLPassSupport.h"

#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"

using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::MetadataAPI;

namespace intel {

char KernelSubGroupInfo::ID = 0;

OCL_INITIALIZE_PASS(KernelSubGroupInfo, "kernel-sub-group-info",
                    "mark kernels with subgroups", false, false)

bool KernelSubGroupInfo::runOnFunction(Function &F) const {
  auto kimd = KernelInternalMetadataAPI(&F);
  bool HasSubGroups = containsSubGroups(&F);
  kimd.KernelHasSubgroups.set(HasSubGroups);
  return HasSubGroups;
}

bool KernelSubGroupInfo::containsSubGroups(Function *pFunc) const {
  CallGraphNode *Node = (*CG)[pFunc];
  for (auto It = df_begin(Node), End = df_end(Node); It != End;) {
    Function *Fn = It->getFunction();
    if (Fn && Fn->isDeclaration()) {
      if (Fn->getName().contains("sub_group"))
        return true;
      It = It.skipChildren();
    } else {
      It++;
    }
  }
  return false;
}

bool KernelSubGroupInfo::runOnModule(Module &M) {
  // Get all kernels
  CompilationUtils::FunctionSet kernelsFunctionSet;
  CompilationUtils::getAllKernels(kernelsFunctionSet, &M);

  CG = &getAnalysis<CallGraphWrapperPass>().getCallGraph();

  bool Changed = false;
  // Run on all scalar functions for handling and handle them
  for (auto *F : kernelsFunctionSet) {
    Changed |= runOnFunction(*F);
  }

  return Changed;
}

} // namespace intel

extern "C" {
ModulePass *createKernelSubGroupInfoPass() {
  return new intel::KernelSubGroupInfo();
}
}
