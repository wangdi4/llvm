#if INTEL_COLLAB
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//   Source file:
//   ------------
//   VPOAnalysis.cpp -- VPO Analysis Passes initializers.
//
//===----------------------------------------------------------------------===//

#include "llvm/InitializePasses.h"

using namespace llvm;

void llvm::initializeVPOAnalysis(PassRegistry &Registry) {
  initializeWRegionCollectionWrapperPassPass(Registry);
  initializeWRegionInfoWrapperPassPass(Registry);
}
#endif // INTEL_COLLAB
