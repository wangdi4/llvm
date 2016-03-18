//===----------------------------------------------------------------------===//
//
//   Copyright (C) 2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOTransform.cpp -- VPO Transform Passes initializers.
//
//===----------------------------------------------------------------------===//

#include "llvm/InitializePasses.h"

using namespace llvm;

void llvm::initializeIntel_VPOTransforms(PassRegistry &Registry) {
  initializeVPODriverPass(Registry);
  initializeVPODriverHIRPass(Registry);
  initializeVPODirectiveCleanupPass(Registry);

  initializeVPOParoptPass(Registry);

  initializeVPOCFGRestructuringPass(Registry);
}

