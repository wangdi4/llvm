#if INTEL_COLLAB
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//   Source file:
//   ------------
//   VPOTransform.cpp -- VPO Transform Passes initializers.
//
//===----------------------------------------------------------------------===//

#include "llvm/InitializePasses.h"

using namespace llvm;

void llvm::initializeVPOTransforms(PassRegistry &Registry) {
#if INTEL_CUSTOMIZATION
  initializeVPODirectiveCleanupPass(Registry);
#endif // INTEL_CUSTOMIZATION
  initializeVPOParoptPreparePass(Registry);
  initializeVPOParoptPass(Registry);
  initializeVPOParoptTpvPass(Registry);

  initializeVPORestoreOperandsPass(Registry);
  initializeVPOCFGRestructuringPass(Registry);
}

#endif // INTEL_COLLAB
