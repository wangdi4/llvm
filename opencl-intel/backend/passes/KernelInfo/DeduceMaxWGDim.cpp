/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

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

    SmallVector<CallInst*, 8> TIDCalls;
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
