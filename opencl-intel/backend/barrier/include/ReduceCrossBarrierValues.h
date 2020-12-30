//===------- ReduceCrossBarrierValues.h - Class definition -*- C++*--------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// See comments in the corresponding .cpp file for details.
///
// ===--------------------------------------------------------------------=== //
#ifndef REDUCE_CROSS_BARRIER_VALUES_H
#define REDUCE_CROSS_BARRIER_VALUES_H

#include "BuiltinLibInfo.h"
#include "DataPerValuePass.h"
#include "OCLPassSupport.h"
#include "WIRelatedValuePass.h"

#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/IR/Dominators.h"

using namespace llvm;

namespace intel {

class ReduceCrossBarrierValues : public FunctionPass {
public:
  ReduceCrossBarrierValues();

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<BuiltinLibInfo>();
    AU.addRequired<DataPerBarrier>();
    AU.addRequired<DataPerValue>();
    AU.addRequired<DominanceFrontierWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<WIRelatedValue>();
    AU.addPreserved<BuiltinLibInfo>();
    AU.addPreserved<DataPerBarrier>();
    AU.addPreserved<DominanceFrontierWrapperPass>();
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addPreserved<WIRelatedValue>();
  }

  bool runOnFunction(Function &) override;

  static char ID;
};

} // namespace intel
#endif // REDUCE_CROSS_BARRIER_VALUES_H
