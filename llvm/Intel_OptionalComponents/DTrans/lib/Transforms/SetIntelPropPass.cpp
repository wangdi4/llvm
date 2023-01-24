//===---------------------- SetIntelPropPass.cpp --------------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass does nothing except set the "Intel Proprietary" module flag.
// There is nothing DTrans-specific about this pass except that DTrans is
// currently the only place we want to set this flag. If it needs to be set
// elsewhere later, this pass can be moved and renamed.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/SetIntelPropPass.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "set-intel-prop"

PreservedAnalyses dtrans::SetIntelPropPass::run(Module &M,
                                                ModuleAnalysisManager &AM) {
  // This isn't guarded under "#if INTEL_PRODUCT_RELEASE" because we test
  // this pass and the associated check by running this pass from a LIT test.
  // The code that adds this pass to the pass manager should be under an
  // INTEL_PRODUCT_RELEASE check
  LLVM_DEBUG(dbgs() << "Setting the Intel Proprietary module flag\n");
  M.setIntelProprietary();
  return PreservedAnalyses::all();
}
