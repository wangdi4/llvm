//===---------------------------DTransOPOptBaseTest.h-----------------------===//
// Test pass for DTransOptBase functionality
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the a test pass that exercises the basic functionality of
// the DTransOPOptBase class.
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error DTransOPOptBaseTest.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_DTRANSOPOPTBASETEST_H
#define INTEL_DTRANS_TRANSFORMS_DTRANSOPOPTBASETEST_H

// This file is only used for opt testing, do not include it as part of the
// product build.
#if !INTEL_PRODUCT_RELEASE

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
namespace llvm {

class TargetLibraryInfo;

namespace dtransOP {
class DTransSafetyInfo;

// This class is used for testing the base class functionality of the DTrans
// type rewriter used to verify that dependent types, variables, and function
// prototypes get properly rewritten. This pass is for LIT testing only, and
// should be included in any compiler pass pipelines.
class DTransOPOptBaseTestPass : public PassInfoMixin<DTransOPOptBaseTestPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  bool runImpl(Module &M, DTransSafetyInfo *DTInfo);
};

} // end namespace dtransOP

} // end namespace llvm

#endif // !INTEL_PRODUCT_RELEASE
#endif // INTEL_DTRANS_TRANSFORMS_DTRANSOPOPTBASETEST_H
