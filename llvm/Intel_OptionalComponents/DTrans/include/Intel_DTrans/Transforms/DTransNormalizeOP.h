//====- DTransNormalizeOP.h - Normalize IR for the DTransSafetyAnalyzer---====//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//
// This file defines the pass that normalizes the IR to reduce instances where
// safety flags will be set by the DTransSafetyAnalyzer
//===---------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error DTransNormalizeOP.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_DTRANSNORMALIZEOP_H
#define INTEL_DTRANS_TRANSFORMS_DTRANSNORMALIZEOP_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class Module;
class TargetLibraryInfo;
class WholeProgramInfo;

namespace dtransOP {

class DTransNormalizeOPPass : public PassInfoMixin<DTransNormalizeOPPass> {
public:
  using GetTLIFn = std::function<const TargetLibraryInfo &(const Function &)>;

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
  bool runImpl(Module &M, WholeProgramInfo &WPInfo, GetTLIFn GetTLI);
};

} // namespace dtransOP

ModulePass *createDTransNormalizeOPWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_DTRANSNORMALIZEOP_H
