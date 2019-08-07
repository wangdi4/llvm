//=---- OCLPostVect.cpp -*-C++-*----=//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// OCLPostVect checks if a cloned kernel is not vectorized. If not, it is
/// removed.
// ===--------------------------------------------------------------------=== //
#include "OCLPostVect.h"
#include "InitializePasses.h"
#include "InstCounter.h"
#include "MetadataAPI.h"
#include "OCLVecClone.h"

#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LegacyPassManager.h"

#define DEBUG_TYPE "OCLPostVect"
#define SV_NAME "ocl-postvect"

using namespace llvm;
using namespace llvm::vpo;
using namespace Intel::MetadataAPI;

extern "C" FunctionPass *createWeightedInstCounter(bool, Intel::CPUId);
extern "C" Pass *
createBuiltinLibInfoPass(llvm::SmallVector<llvm::Module *, 2> pRtlModuleList,
                         std::string type);

namespace intel {

char OCLPostVect::ID = 0;
static const char lv_name[] = "OCLPostVect";
OCL_INITIALIZE_PASS_BEGIN(OCLPostVect, SV_NAME, lv_name,
                          false /* modifies CFG */, false /* transform pass */)
OCL_INITIALIZE_PASS_END(OCLPostVect, SV_NAME, lv_name,
                        false /* modififies CFG */, false /* transform pass */)

OCLPostVect::OCLPostVect() : ModulePass(ID) {}

void OCLPostVect::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<BuiltinLibInfo>();
}

// Checks if the kernel has directives. If not, then the kernel was vectorized.
bool OCLPostVect::isKernelVectorized(Function *Clone) {
  for (BasicBlock &Block : *Clone)
    for (Instruction &I : Block)
      if (VPOAnalysisUtils::isOpenMPDirective(&I))
        return false;
  return true;
}

bool OCLPostVect::runOnModule(Module &M) {
  auto Kernels = KernelList(*&M).getList();
  bool ModifiedModule = false;
  for (Function *F : Kernels) {
    auto FMD = KernelInternalMetadataAPI(F);
    Function *ClonedKernel = FMD.VectorizedKernel.get();
    if (ClonedKernel &&
        (!isKernelVectorized(ClonedKernel) ||
        (!KernelMetadataAPI(F).hasVecLength() &&
             !isKernelVectorizationProfitable(M, F, ClonedKernel)))) {
      // unset the metadata of the original kernel wh
      MDValueGlobalObjectStrategy::unset(F, "vectorized_kernel");
      // If the kernel is not vectorized, then the cloned kernel is removed.
      ClonedKernel->eraseFromParent();
      ModifiedModule = true;
    }
  }
  return ModifiedModule;
}

bool OCLPostVect::isKernelVectorizationProfitable(Module &M, Function *F,
                                                  Function *ClonedKernel) {
  legacy::FunctionPassManager FPM(&M);
  WeightedInstCounter *Counter =
      (WeightedInstCounter *)createWeightedInstCounter(true, Intel::CPUId());
  FPM.add(createBuiltinLibInfoPass(
      getAnalysis<BuiltinLibInfo>().getBuiltinModules(), ""));
  FPM.add(Counter);

  FPM.run(*F);
  float SCost = Counter->getWeight();
  FPM.run(*ClonedKernel);
  float VCost = Counter->getWeight();
  float Ratio = VCost / SCost;

  IsaEncodingValue ISAEncoding = SSE42;
  if (CPUIsaEncodingOverride.getNumOccurrences())
    ISAEncoding = CPUIsaEncodingOverride.getValue();
  else if (Intel::CPUId().HasAVX512Core())
    ISAEncoding = IsaEncodingValue::AVX512Core;
  else if (Intel::CPUId().HasAVX2())
    ISAEncoding = IsaEncodingValue::AVX2;

  int VectorLength = 4;
  if (ISAEncoding == IsaEncodingValue::AVX512Core)
    VectorLength = 16;
  else if (ISAEncoding == IsaEncodingValue::AVX2)
    VectorLength = 8;

  return Ratio < WeightedInstCounter::RATIO_MULTIPLIER * VectorLength;
}

} // namespace intel

extern "C" Pass *createOCLPostVectPass(void) {
  return new intel::OCLPostVect();
}
