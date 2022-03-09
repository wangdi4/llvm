#if INTEL_COLLAB
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
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
#if INTEL_CUSTOMIZATION
  initializeVPOParoptConfigWrapperPass(Registry);
#endif // INTEL_CUSTOMIZATION
}
#endif // INTEL_COLLAB
