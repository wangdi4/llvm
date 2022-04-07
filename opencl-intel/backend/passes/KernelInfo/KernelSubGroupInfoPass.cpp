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
#include "OCLPassSupport.h"

#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace Intel::OpenCL::DeviceBackend;
using namespace DPCPPKernelMetadataAPI;

namespace intel {

char KernelSubGroupInfo::ID = 0;

OCL_INITIALIZE_PASS(KernelSubGroupInfo, "kernel-sub-group-info",
                    "mark functions and kernels with subgroups", false, false)

bool KernelSubGroupInfo::runOnFunction(Function &F) const {
  if (!F.isDeclaration() && containsSubGroups(&F)) {
    F.addFnAttr(CompilationUtils::ATTR_HAS_SUBGROUPS);
    return true;
  }
  return false;
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
  CG = &getAnalysis<CallGraphWrapperPass>().getCallGraph();

  bool Changed = false;

  for (auto &F : M)
    Changed |= runOnFunction(F);

  // Get all kernels.
  CompilationUtils::FunctionSet kernelsFunctionSet;
  CompilationUtils::getAllKernels(kernelsFunctionSet, &M);

  // Note: We should always set this metadata because it's used to indicate
  // we are under native SG mode.
  for (auto *F : kernelsFunctionSet) {
    auto kimd = KernelInternalMetadataAPI(F);
    kimd.KernelHasSubgroups.set(
        F->hasFnAttribute(CompilationUtils::ATTR_HAS_SUBGROUPS));
  }

  return Changed || !kernelsFunctionSet.empty();
}

} // namespace intel

extern "C" {
ModulePass *createKernelSubGroupInfoPass() {
  return new intel::KernelSubGroupInfo();
}
}
