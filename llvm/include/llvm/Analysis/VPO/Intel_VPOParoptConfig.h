//===- Intel_VPOParoptConfig.h - Paropt Config Storage ---------*- C++ -*--===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This is an immutable pass which stores the Paropt configuration
// information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_VPOPAROPTCONFIG_H
#define LLVM_ANALYSIS_INTEL_VPOPAROPTCONFIG_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/YAMLTraits.h"

namespace llvm {

class LLVMContext;

namespace vpo {

enum class RegisterAllocationMode { AUTO, SMALL, LARGE, DEFAULT };

struct KernelConfig {
  std::string Name;
  uint8_t SPMDSIMDWidth{0};
  uint64_t ThreadLimit{0};
  uint64_t NumTeams{0};
  RegisterAllocationMode RegisterAllocMode;
};

struct Config {
  std::vector<KernelConfig> KernelEntries;
};
} // namespace vpo

namespace yaml {
template <>
struct MappingTraits<vpo::KernelConfig> {
  static void mapping(IO &, vpo::KernelConfig &);
};

template <>
struct MappingTraits<vpo::Config> {
  static void mapping(IO &, vpo::Config &);
};
} // namespace yaml

class VPOParoptConfig {
  vpo::Config Config;

  Optional<const vpo::KernelConfig> getKernelConfig(
      StringRef Name) const;

public:
  VPOParoptConfig(LLVMContext &Context);

  bool invalidate(Module &M, const PreservedAnalyses &,
                  ModuleAnalysisManager::Invalidator &) {
    // Never invalidate the analysis.
    return false;
  }

  uint8_t getKernelSPMDSIMDWidth(StringRef Name) const;
  uint64_t getKernelThreadLimit(StringRef Name) const;
  uint64_t getKernelNumTeams(StringRef Name) const;
  vpo::RegisterAllocationMode getRegisterAllocMode(StringRef Name) const;
};

class VPOParoptConfigAnalysis :
    public AnalysisInfoMixin<VPOParoptConfigAnalysis> {
  friend AnalysisInfoMixin<VPOParoptConfigAnalysis>;
  static AnalysisKey Key;

public:
  typedef VPOParoptConfig Result;
  Result run(Module &M, ModuleAnalysisManager &AM);
};

class VPOParoptConfigWrapper : public ImmutablePass {
  std::unique_ptr<VPOParoptConfig> Result;

public:
  static char ID;

  VPOParoptConfigWrapper();

  const VPOParoptConfig &getResult() { return *Result; }

  bool doInitialization(Module &M) override;
  bool doFinalization(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

ImmutablePass *createVPOParoptConfigWrapperPass();
} // end namespace llvm

LLVM_YAML_IS_SEQUENCE_VECTOR(llvm::vpo::KernelConfig)
#endif // LLVM_ANALYSIS_INTEL_VPOPAROPTCONFIG_H
