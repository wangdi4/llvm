// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
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


#include "VectorKernelDiscard.h"
#include "InitializePasses.h"
#include "OCLPassSupport.h"

#include "llvm/ADT/Triple.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#define DEBUG_TYPE "VectorKernelDiscard"

using namespace intel;
using namespace DPCPPKernelMetadataAPI;
using namespace llvm;

extern "C" {
Pass *createBuiltinLibInfoPass(ArrayRef<Module *>, std::string);
FunctionPass *
createWeightedInstCounter(bool, const Intel::OpenCL::Utils::CPUDetect *);
}

namespace intel {

static constexpr float SCALAR_COST_MUTIPLIER = .98f;

char VectorKernelDiscard::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(VectorKernelDiscard, DEBUG_TYPE,
                          "Discard vectorized kernel per cost model",
                          false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy)
OCL_INITIALIZE_PASS_END(VectorKernelDiscard, DEBUG_TYPE,
                        "Discard vectorized kernel per cost model",
                        false, false)

VectorKernelDiscard::VectorKernelDiscard(const OptimizerConfig *Config)
    : ModulePass(ID), Config(Config) {
  initializeVectorKernelDiscardPass(*PassRegistry::getPassRegistry());
}

WeightedInstCounter *VectorKernelDiscard::addPassesToCalculateCost(
    legacy::FunctionPassManager &FPM, TargetMachine *TM,
    TargetLibraryInfoImpl &TLI, ArrayRef<Module *> BuiltinModules,
    bool IsScalar) {
  FPM.add(createTargetTransformInfoWrapperPass(
          TM->getTargetIRAnalysis()));
  FPM.add(new TargetLibraryInfoWrapperPass(TLI));
  FPM.add(createBuiltinLibInfoAnalysisLegacyPass(BuiltinModules));
  FPM.add(createBuiltinLibInfoPass(BuiltinModules, ""));
  WeightedInstCounter *Counter =
      (WeightedInstCounter *)createWeightedInstCounter(
          IsScalar, Config->GetCpuId());
  FPM.add(Counter);
  return Counter;
}

bool VectorKernelDiscard::runOnModule(Module &M) {
  TargetMachine* TM = Config->GetTargetMachine();
  TargetLibraryInfoImpl TLI(Triple(M.getTargetTriple()));

  ArrayRef<Module *> BuiltinModules =
      getAnalysis<BuiltinLibInfoAnalysisLegacy>()
          .getResult()
          .getBuiltinModules();

  llvm::legacy::FunctionPassManager ScalarFPM(&M);
  llvm::legacy::FunctionPassManager VectorFPM(&M);

  bool Changed = false;

  auto *ScalarCounter =
      addPassesToCalculateCost(ScalarFPM, TM, TLI, BuiltinModules, true);
  auto *VectorCounter =
      addPassesToCalculateCost(VectorFPM, TM, TLI, BuiltinModules, false);

  ScalarFPM.doInitialization();
  VectorFPM.doInitialization();

  auto Kernels = KernelList(*&M).getList();
  for (Function *F : Kernels) {
    LLVM_DEBUG(dbgs() << "Analyzing kernel \"" << F->getName() << "\"\n");

    auto KIMD = KernelInternalMetadataAPI(F);
    bool KernelHasSubgroups = KIMD.KernelHasSubgroups.get();
    if (KernelHasSubgroups) {
      LLVM_DEBUG(
          dbgs() << "  Kernel has subgroups, so keep vectorized kernel.\n");
      continue;
    }

    auto KMD = KernelMetadataAPI(F);
    bool ForcedVec = (Config->GetTransposeSize() != TRANSPOSE_SIZE_NOT_SET) ||
                     KMD.hasVecLength();
    if (ForcedVec) {
      LLVM_DEBUG(
          dbgs() << "  Kernel VF is forced, so keep vectorized kernel.\n");
      continue;
    }

    Function *VecF = KIMD.VectorizedKernel.get();
    if (!VecF)
      continue;
    auto VKIMD = KernelInternalMetadataAPI(VecF);
    unsigned VF = VKIMD.VectorizedWidth.get();
    assert(VF && "Vector factor cannot be 0 here.");

    LLVM_DEBUG(dbgs() << "Got vectorized kernel \"" << VecF->getName() << "\".\n");

    ScalarFPM.run(*F);
    VectorFPM.run(*VecF);

    int ScalarCost = ScalarCounter->getWeight();
    int VectorCost = VectorCounter->getWeight() / VF;

    LLVM_DEBUG(dbgs() << "  ScalarCost: " << ScalarCost
                      << "  VectorCost: " << VectorCost << '\n');
    if (ScalarCost * SCALAR_COST_MUTIPLIER <= VectorCost) {
      LLVM_DEBUG(dbgs() << "  Discard vectorized kernel.\n");
      VecF->eraseFromParent();
      KIMD.VectorizedKernel.set(nullptr);
      Changed = true;
    } else {
      LLVM_DEBUG(dbgs() << "  Keep vectorized kernel.\n");
    }
  }

  ScalarFPM.doFinalization();
  VectorFPM.doFinalization();
  return Changed;
}

} // namespace intel

extern "C" ModulePass *createVectorKernelDiscardPass(
    const OptimizerConfig *Config) {
  return new intel::VectorKernelDiscard(Config);
}
