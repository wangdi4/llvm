//===---DTransOptBaseTest.h - Test pass for DTransOptBase functionality----===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the a test pass that exercises the basic functionality of
// the DTransOptBase class.
//
//===----------------------------------------------------------------------===//

#if !INTEL_INCLUDE_DTRANS
#error DTransOptBaseTest.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_OPTIONALCOMPONENTS_INTEL_DTRANS_TRANSFORMS_DTRANSOPTBASETEST_H
#define INTEL_OPTIONALCOMPONENTS_INTEL_DTRANS_TRANSFORMS_DTRANSOPTBASETEST_H

// This file is only used for opt testing, do not include it as part of the
// product build.
#if !INTEL_PRODUCT_RELEASE

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class DTransAnalysisInfo;
class TargetLibraryInfo;
class WholeProgramInfo;

namespace dtrans {

// This class is used for testing the base class functionality of the DTrans
// type rewriter used to verify that dependent types, variables, and function
// prototypes get properly rewritten. This pass is for LIT testing only, and
// should be included in any compiler pass pipelines.
class OptBaseTestPass : public PassInfoMixin<dtrans::OptBaseTestPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool
  runImpl(Module &M, DTransAnalysisInfo *DTInfo,
          std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
          WholeProgramInfo &WPInfo);
};

} // namespace dtrans

ModulePass *createDTransOptBaseTestWrapperPass();

} // namespace llvm

#endif // !INTEL_PRODUCT_RELEASE
#endif // INTEL_OPTIONALCOMPONENTS_INTEL_DTRANS_TRANSFORMS_DTRANSOPTBASETEST_H
