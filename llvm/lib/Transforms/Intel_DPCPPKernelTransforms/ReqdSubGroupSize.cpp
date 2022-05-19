//==------- ReqdSubGroupSize.cpp - ReqdSubGroupSize pass - C++ -*-----------==//
//
// Copyright (C) 2018-2022 Intel Corporation. All rights reserved.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ReqdSubGroupSize.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#define DEBUG_TYPE "dpcpp-kernel-reqd-sub-group-size"

using namespace llvm;
using namespace DPCPPKernelMetadataAPI;

cl::opt<std::string>
    OptReqdSubGroupSizes("dpcpp-reqd-sub-group-size", cl::init(""), cl::Hidden,
                         cl::desc("Per-kernel required subgroup"
                                  "size. Comma separated list of"
                                  " name(num)"));

namespace llvm {
/// for legacy pass manager
class ReqdSubGroupSizeLegacy : public ModulePass {
  ReqdSubGroupSizePass Impl;

public:
  ReqdSubGroupSizeLegacy() : ModulePass(ID) {
    initializeReqdSubGroupSizeLegacyPass(*PassRegistry::getPassRegistry());
  }

  static char ID;

  StringRef getPassName() const override {
    return "Intel DPCPP Kernel ReqdSubGroupSize Pass";
  }

  bool runOnModule(Module &M) override { return Impl.runImpl(M); }
};
} // namespace llvm

char ReqdSubGroupSizeLegacy::ID = 0;

INITIALIZE_PASS(ReqdSubGroupSizeLegacy, DEBUG_TYPE, "ReqdSubGroupSize pass",
                true /* CFG unchanged */, false /* transform pass */)

PreservedAnalyses ReqdSubGroupSizePass::run(Module &M,
                                            ModuleAnalysisManager &) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

static bool isValidReqdSubGroupSize(size_t ReqdSubGroupSize) {
  if (ReqdSubGroupSize == 0 || ReqdSubGroupSize == 4 || ReqdSubGroupSize == 8 ||
      ReqdSubGroupSize == 16 || ReqdSubGroupSize == 32 ||
      ReqdSubGroupSize == 64)
    return true;
  return false;
}

bool ReqdSubGroupSizePass::runImpl(Module &M) {
  bool Changed = false;
  if (OptReqdSubGroupSizes.size() == 0)
    return Changed;
  LLVM_DEBUG(dbgs() << OptReqdSubGroupSizes.ArgStr << " : "
                    << OptReqdSubGroupSizes << "\n");

  // Split name1(n1),name2(n2),name3(n3)... into
  //    name1(n1)
  //    name2(n2)
  //    name3(n3)
  //    ...
  StringRef Sizes(OptReqdSubGroupSizes);
  SmallVector<StringRef, 3> VSizes;
  Sizes.split(VSizes, ',', -1, false /* KeepEmpty */);

  // Match up each Kernel against each name(num)
  auto Kernels = KernelList(*&M).getList();
  for (Function *F : Kernels) {
    auto KMD = KernelMetadataAPI(F);
    StringRef FName(F->getName());
    auto FNameLen = FName.size();

    // Process each SubGrpSize specifier represented in "name(num)"
    for (auto &SubGrpSize : VSizes) {
      auto Len = SubGrpSize.size();
      if (!SubGrpSize.startswith(FName))
        continue; // Name should match
      if (SubGrpSize.rfind('(') != FNameLen || SubGrpSize.find(')') != Len - 1)
        continue; // ( and ) should be found in the correct locations.
      auto SubStr =
          SubGrpSize.substr(FNameLen + 1, Len - FNameLen - 2); // "num"
      size_t ReqdSubGrpSize = 0;
      if (SubStr.getAsInteger(10 /* radix */, ReqdSubGrpSize))
        continue;
      // Process valid values only.
      if (!isValidReqdSubGroupSize(ReqdSubGrpSize))
        continue;

      LLVM_DEBUG(dbgs() << "Set required subgroup size: " << ReqdSubGrpSize
                        << " for " << F->getName() << " function.\n");
      // Set required sub group size to the kernel.
      KMD.setReqdIntelSGSize(ReqdSubGrpSize);
      Changed = true;
    }
  }
  return Changed;
}

ModulePass *llvm::createReqdSubGroupSizeLegacyPass() {
  return new llvm::ReqdSubGroupSizeLegacy();
}
