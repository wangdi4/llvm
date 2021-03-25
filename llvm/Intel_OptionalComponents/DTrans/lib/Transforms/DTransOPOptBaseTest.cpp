//===-----------------DTransOptBaseOpaquePtrTest.cpp-----------------------===//
// Test pass for DTransOPOptBase functionality
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a test pass that exercises the basic functionality of
// the DTransOPOptBase class.
//
//===----------------------------------------------------------------------===//

// This file is only used for opt testing, do not include it as part of the
// product build.
#if !INTEL_PRODUCT_RELEASE
#include "Intel_DTrans/Transforms/DTransOPOptBaseTest.h"

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOPOptBase.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace dtransOP;

// This option is used to supply a comma separated list of structure types that
// should be renamed as part of the DTransOPOptBaseTestPass class test to verify
// dependent objects get transformed appropriately.
static cl::opt<std::string>
    DTransOPOptBaseOpaquePtrTestTypeList("dtransop-optbasetest-typelist",
                                         cl::ReallyHidden);

namespace {
class DTransOPOptBaseTestWrapper : public ModulePass {
private:
  DTransOPOptBaseTestPass Impl;

public:
  static char ID;

  DTransOPOptBaseTestWrapper() : ModulePass(ID) {
    initializeDTransOPOptBaseTestWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    DTransSafetyAnalyzerWrapper &DTAnalysisWrapper =
        getAnalysis<DTransSafetyAnalyzerWrapper>();
    DTransSafetyInfo &DTInfo = DTAnalysisWrapper.getDTransSafetyInfo(M);
    return Impl.runImpl(M, &DTInfo);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransSafetyAnalyzerWrapper>();
  }
};

// This class tests and demonstrates usage of the DTransOptBase class.
class DTransOptBaseTest : public DTransOPOptBase {
public:
  DTransOptBaseTest(DTransTypeManager &TM) : DTransOPOptBase(TM) {}

  // TODO: Implement the methods that the base class requires from the derived
  // class.
};

} // end anonymous namespace

char DTransOPOptBaseTestWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransOPOptBaseTestWrapper, "dtransop-optbasetest",
                      "DTrans base class tester for opaque pointers", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(DTransSafetyAnalyzerWrapper)
INITIALIZE_PASS_END(DTransOPOptBaseTestWrapper, "dtransop-optbasetest",
                    "DTrans base class tester for opaque pointers", false,
                    false)

ModulePass *llvm::createDTransOPOptBaseTestWrapperPass() {
  return new DTransOPOptBaseTestWrapper();
}

// Implementation for new pass manager
PreservedAnalyses
dtransOP::DTransOPOptBaseTestPass::run(Module &M, ModuleAnalysisManager &AM) {
  DTransSafetyInfo *DTInfo = &AM.getResult<DTransSafetyAnalyzer>(M);
  bool Changed = runImpl(M, DTInfo);
  if (!Changed)
    return PreservedAnalyses::all();

  // This pass is only used for testing the base class, so there is no need to
  // preserve anything when the IR changes.
  return PreservedAnalyses::none();
}

bool dtransOP::DTransOPOptBaseTestPass::runImpl(Module &M,
                                                DTransSafetyInfo *DTInfo) {
  DTransOptBaseTest Transformer(DTInfo->getTypeManager());
  return Transformer.run(M);
}

#endif // !INTEL_PRODUCT_RELEASE
