//===- IndirectCallPromotionAnalysis.h - Indirect call analysis -*- C++ -*-===//
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
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// Interface to identify indirect call promotion candidates.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INDIRECTCALLPROMOTIONANALYSIS_H
#define LLVM_ANALYSIS_INDIRECTCALLPROMOTIONANALYSIS_H

#include "llvm/ProfileData/InstrProf.h"

namespace llvm {

class Instruction;

// Class for identifying profitable indirect call promotion candidates when
// the indirect-call value profile metadata is available.
class ICallPromotionAnalysis {
private:
  // Allocate space to read the profile annotation.
  std::unique_ptr<InstrProfValueData[]> ValueDataArray;

  // Count is the call count for the direct-call target.
  // TotalCount is the total call count for the indirect-call callsite.
  // RemainingCount is the TotalCount minus promoted-direct-call count.
  // Return true we should promote this indirect-call target.
#if INTEL_CUSTOMIZATION
  // When 'IngoreTotalPercent' is set to 'true', only require the Count to meet
  // threshold requirment for the remaining count percentage, and not the total
  // count percentage. This is used when considering the final candidate to
  // enable that target to be converted even if the percentage is lower than the
  // total percentage, since all targets with a higher percentage have already
  // been converted. For example:
  //   Target1: 97.5%
  //   Target2: 2.5%
  // Normally, we would avoid converting Target2, but since there are no other
  // candidates, we might as well convert that one as well, so that all targets
  // will be direct.
  bool isPromotionProfitable(uint64_t Count, uint64_t TotalCount,
                             uint64_t RemainingCount, bool IngoreTotalPercent);
#endif // INTEL_CUSTOMIZATION

  // Returns the number of profitable candidates to promote for the
  // current ValueDataArray and the given \p Inst.
  uint32_t getProfitablePromotionCandidates(const Instruction *Inst,
                                            uint32_t NumVals,
                                            uint64_t TotalCount);

  // Noncopyable
  ICallPromotionAnalysis(const ICallPromotionAnalysis &other) = delete;
  ICallPromotionAnalysis &
  operator=(const ICallPromotionAnalysis &other) = delete;

public:
  ICallPromotionAnalysis();

  /// Returns reference to array of InstrProfValueData for the given
  /// instruction \p I.
  ///
  /// The \p NumVals, \p TotalCount and \p NumCandidates
  /// are set to the number of values in the array, the total profile count
  /// of the indirect call \p I, and the number of profitable candidates
  /// in the given array (which is sorted in reverse order of profitability).
  ///
  /// The returned array space is owned by this class, and overwritten on
  /// subsequent calls.
  ArrayRef<InstrProfValueData>
  getPromotionCandidatesForInstruction(const Instruction *I, uint32_t &NumVals,
                                       uint64_t &TotalCount,
                                       uint32_t &NumCandidates);
};

} // end namespace llvm

#endif
