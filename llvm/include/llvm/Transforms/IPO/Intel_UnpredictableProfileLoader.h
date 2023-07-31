//===-- Intel_UnpredictableProfileLoader.h - Unpredictable Profile Loader -===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_UNPREDICTABLEPROFILELOADER_H
#define LLVM_TRANSFORMS_IPO_INTEL_UNPREDICTABLEPROFILELOADER_H

#include "llvm/IR/PassManager.h"
#include "llvm/ProfileData/SampleProfReader.h"

namespace llvm {

class Module;

struct UnpredictableProfileLoaderPass
    : PassInfoMixin<UnpredictableProfileLoaderPass> {
  UnpredictableProfileLoaderPass(StringRef FrequencyProfileFile);
  UnpredictableProfileLoaderPass();
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
  std::unique_ptr<SampleProfileReader> FreqReader, MispReader;
  bool loadSampleProfile(Module &M);
  bool addUpredictableMetadata(Module &F);
  bool addUpredictableMetadata(Function &F);
  ErrorOr<double> getMispredictRatio(const FunctionSamples *FreqSamples,
                                     const FunctionSamples *MispSamples,
                                     const Instruction *I);
  const std::string FrequencyProfileFile;
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_UNPREDICTABLEPROFILELOADER_H
