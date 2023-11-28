//===- SimplifyCFGOptions.h - Control structure for SimplifyCFG -*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// A set of parameters used to control the transforms in the SimplifyCFG pass.
// Options may change depending on the position in the optimization pipeline.
// For example, canonical form that includes switches and branches may later be
// replaced by lookup tables and selects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_SIMPLIFYCFGOPTIONS_H
#define LLVM_TRANSFORMS_UTILS_SIMPLIFYCFGOPTIONS_H

namespace llvm {

class AssumptionCache;

struct SimplifyCFGOptions {
  int BonusInstThreshold = 1;
  bool ForwardSwitchCondToPhi = false;
  bool ConvertSwitchRangeToICmp = false;
  bool ConvertSwitchToLookupTable = false;
  bool NeedCanonicalLoop = true;
  bool HoistCommonInsts = false;
  bool SinkCommonInsts = false;
  bool SimplifyCondBranch = true;
  bool SpeculateBlocks = true;
  bool InvalidateAndersRes = false; // INTEL

  AssumptionCache *AC = nullptr;

  // Support 'builder' pattern to set members by name at construction time.
  SimplifyCFGOptions &bonusInstThreshold(int I) {
    BonusInstThreshold = I;
    return *this;
  }
  SimplifyCFGOptions &forwardSwitchCondToPhi(bool B) {
    ForwardSwitchCondToPhi = B;
    return *this;
  }
  SimplifyCFGOptions &convertSwitchRangeToICmp(bool B) {
    ConvertSwitchRangeToICmp = B;
    return *this;
  }
  SimplifyCFGOptions &convertSwitchToLookupTable(bool B) {
    ConvertSwitchToLookupTable = B;
    return *this;
  }
  SimplifyCFGOptions &needCanonicalLoops(bool B) {
    NeedCanonicalLoop = B;
    return *this;
  }
  SimplifyCFGOptions &hoistCommonInsts(bool B) {
    HoistCommonInsts = B;
    return *this;
  }
  SimplifyCFGOptions &sinkCommonInsts(bool B) {
    SinkCommonInsts = B;
    return *this;
  }
  SimplifyCFGOptions &setAssumptionCache(AssumptionCache *Cache) {
    AC = Cache;
    return *this;
  }
  SimplifyCFGOptions &setSimplifyCondBranch(bool B) {
    SimplifyCondBranch = B;
    return *this;
  }

  SimplifyCFGOptions &speculateBlocks(bool B) {
    SpeculateBlocks = B;
    return *this;
  }
#if INTEL_CUSTOMIZATION
  SimplifyCFGOptions &invalidateAndersRes(bool B) {
    InvalidateAndersRes = B;
    return *this;
  }
#endif // INTEL_CUSTOMIZATION
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_UTILS_SIMPLIFYCFGOPTIONS_H
