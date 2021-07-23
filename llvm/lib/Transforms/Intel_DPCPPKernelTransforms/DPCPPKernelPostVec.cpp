//=--------------------- DPCPPKernelPostVec.cpp -*-C++-*---------------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// DPCPPKernelPostVec checks if a cloned kernel is not vectorized. If not, it
/// is removed.
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelPostVec.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#define DEBUG_TYPE "DPCPPKernelPostVec"
#define SV_NAME "dpcpp-kernel-postvec"

using namespace llvm;
using namespace llvm::vpo;

char DPCPPKernelPostVec::ID = 0;
static const char lv_name[] = "DPCPPKernelPostVec";

INITIALIZE_PASS(DPCPPKernelPostVec, SV_NAME, lv_name, false /* modifies CFG */,
                false /* transform pass */)

namespace llvm {
DPCPPKernelPostVec::DPCPPKernelPostVec() : ModulePass(ID) {
  initializeDPCPPKernelPostVecPass(*PassRegistry::getPassRegistry());
}

// Checks if the kernel has directives. If not, then the kernel was vectorized.
bool DPCPPKernelPostVec::isKernelVectorized(Function *Clone) {
  for (Instruction &I : instructions(Clone))
    if (VPOAnalysisUtils::isOpenMPDirective(&I))
      return false;
  return true;
}

bool DPCPPKernelPostVec::runOnModule(Module &M) {
  auto Kernels = DPCPPKernelCompilationUtils::getKernels(M);
  if (Kernels.empty()) {
    LLVM_DEBUG(dbgs() << "No kernels found!\n";);
    return false;
  }

  bool ModifiedModule = false;
  for (Function *F : Kernels) {
    // Remove "recommended-vector-length" attribute.
    DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(F);
    KIMD.RecommendedVL.set(0);

    Function *ClonedKernel = KIMD.VectorizedKernel.get();
    if (ClonedKernel && !isKernelVectorized(ClonedKernel)) {
      // Unset the metadata of the original kernel.
      KIMD.VectorizedKernel.set(nullptr);
      // If the kernel is not vectorized, then the cloned kernel is removed.
      ClonedKernel->eraseFromParent();
      ModifiedModule = true;
    }
  }

  return ModifiedModule;
}

ModulePass *createDPCPPKernelPostVecPass(void) {
  return new DPCPPKernelPostVec();
}
} // namespace llvm
