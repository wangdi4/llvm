#if INTEL_FEATURE_MARKERCOUNT
//==- Definition of the MarkerCountIntrinsicInserterPass class -*- C++-*-======//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===-----------------------------------------------------------------------===//
//
// This file declares the MarkerCountIntrinsicInserterPass class which is used
// to insert mid-end marker count.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INSTRUMENTATION_MARKERCOUNTINTRINSICINSERTER_H
#define LLVM_TRANSFORMS_INSTRUMENTATION_MARKERCOUNTINTRINSICINSERTER_H

#include "llvm/IR/PassManager.h"
#include <map>

namespace llvm {
class LoopInfo;
class MarkerCountIntrinsicInserterPass
    : public PassInfoMixin<MarkerCountIntrinsicInserterPass> {
private:
  std::map<std::string, unsigned> OverrideMarkerCount;
  unsigned MarkerCountKind;
  unsigned getMarkerCount(StringRef FunctionName) const;
  bool insertMarkerCountIntrinsic(Function &F, unsigned MCK, LoopInfo *LI);

public:
  MarkerCountIntrinsicInserterPass(unsigned MarkerCountKind = 0,
                                   StringRef OverrideMarkerCountFile = "");

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace llvm

#endif
#endif // INTEL_FEATURE_MARKERCOUNT
