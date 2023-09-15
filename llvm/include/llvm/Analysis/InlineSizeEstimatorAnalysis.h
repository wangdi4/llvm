//===- InlineSizeEstimatorAnalysis.h - ML size estimator --------*- C++ -*-===//
//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
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
//

#ifndef LLVM_ANALYSIS_INLINESIZEESTIMATORANALYSIS_H
#define LLVM_ANALYSIS_INLINESIZEESTIMATORANALYSIS_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class Function;

class TFModelEvaluator;
class InlineSizeEstimatorAnalysis
    : public AnalysisInfoMixin<InlineSizeEstimatorAnalysis> {
public:
  InlineSizeEstimatorAnalysis();
  InlineSizeEstimatorAnalysis(InlineSizeEstimatorAnalysis &&);
#if INTEL_CUSTOMIZATION
  InlineSizeEstimatorAnalysis &
  operator=(const InlineSizeEstimatorAnalysis &) = delete;
  InlineSizeEstimatorAnalysis &
  operator=(InlineSizeEstimatorAnalysis &&) = delete;
#endif // INTEL_CUSTOMIZATION
  ~InlineSizeEstimatorAnalysis();

  static AnalysisKey Key;
  using Result = std::optional<size_t>;
  Result run(const Function &F, FunctionAnalysisManager &FAM);
  static bool isEvaluatorRequested();

private:
  std::unique_ptr<TFModelEvaluator> Evaluator;
};

class InlineSizeEstimatorAnalysisPrinterPass
    : public PassInfoMixin<InlineSizeEstimatorAnalysisPrinterPass> {
  raw_ostream &OS;

public:
  explicit InlineSizeEstimatorAnalysisPrinterPass(raw_ostream &OS) : OS(OS) {}

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};
} // namespace llvm
#endif // LLVM_ANALYSIS_INLINESIZEESTIMATORANALYSIS_H
