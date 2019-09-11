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

#include "DeduceMaxWGDim.h"
#include "CompilationUtils.h"
#include "MetadataAPI.h"
#include "LoopUtils/LoopUtils.h"
#include "OpenclRuntime.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"

using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::MetadataAPI;

namespace intel {

  char DeduceMaxWGDim::ID = 0;

  OCL_INITIALIZE_PASS_BEGIN(DeduceMaxWGDim, "deduce-max-dim", "Deduce the maximum WG dimemsion that needs to be executed", false, false)
  OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
  OCL_INITIALIZE_PASS_END(DeduceMaxWGDim, "deduce-max-dim", "Deduce the maximum WG dimemsion that needs to be executed", false, false)

  bool DeduceMaxWGDim::runOnFunction(Function &F) {
    // If kernel calls any forbidden functions, fail
    if (ForbiddenFuncUsers.count(&F)) {
      return false;
    }

    // If we have subgroups then at least one vector iteration is expected,
    // it can't be achieved without a loop.
    bool SubGroupPath = KernelInternalMetadataAPI(&F).KernelHasSubgroups.get();
    if (SubGroupPath)
      return false;

    SmallVector<CallInst*, 8> TIDCalls;
    // TODO: handle other built-ins like get_group_id(dim), etc.
    LoopUtils::getAllCallInFunc(MangledGetGID, &F, TIDCalls);
    LoopUtils::getAllCallInFunc(MangledGetLID, &F, TIDCalls);
    int MaxDim = -1;
    for (unsigned I = 0, E = TIDCalls.size(); I != E; ++I) {
      ConstantInt *Dim = dyn_cast<ConstantInt>(TIDCalls[I]->getOperand(0));
      // If dimension argument is not a constant, fail
      if (!Dim)
        return false;
      MaxDim = std::max(MaxDim, int(Dim->getZExtValue()));
    }

    // No point in saying that kernel needs 3D
    if (MaxDim == 2)
      return false;

    KernelInternalMetadataAPI(&F).MaxWGDimensions.set(MaxDim + 1);

    return true;
  }

  bool DeduceMaxWGDim::runOnModule(Module& M) {
    RT = static_cast<OpenclRuntime *>(
        getAnalysis<BuiltinLibInfo>().getRuntimeServices());
    ForbiddenFuncUsers.clear();
    LoopUtils::fillAtomicBuiltinUsers(M, RT, ForbiddenFuncUsers);
    LoopUtils::fillInternalFuncUsers(M, RT, ForbiddenFuncUsers);

    LoopUtils::fillWorkItemPipeBuiltinUsers(M, RT, ForbiddenFuncUsers);
    LoopUtils::fillPrintfs(M, RT, ForbiddenFuncUsers);

    // Get all kernels
    CompilationUtils::FunctionSet kernelsFunctionSet;
    CompilationUtils::getAllKernels(kernelsFunctionSet, &M);

    // Run on all scalar functions
    bool Changed = false;
    for ( CompilationUtils::FunctionSet::iterator fi = kernelsFunctionSet.begin(),
      fe = kernelsFunctionSet.end(); fi != fe; ++fi ) {
        Function *pFunc = cast<Function>(*fi);
        Changed = runOnFunction(*pFunc) || Changed;
    }

    return Changed;
  }

} // namespace intel {

extern "C" {
  ModulePass* createDeduceMaxWGDimPass() {
    return new intel::DeduceMaxWGDim();
  }
}
