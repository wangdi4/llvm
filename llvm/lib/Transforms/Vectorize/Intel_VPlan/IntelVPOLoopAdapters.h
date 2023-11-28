//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOLoopAdapter is an adapter that provides a unified interface for LLVM-IR
//   Loop and HIR HLLoop. This adapter is very convenient for the implementation
//   of the common part of algorithms that have to deal with Loop's and
//   HLLoop's. Hopefully, once we demonstrate that having the same interface for
//   Loop and HLLoop is convenient to prevent code replication, we could ask HIR
//   team for actual changes in the interface of HLLoop and remove this adapter.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPOLOOPADAPTERS_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPOLOOPADAPTERS_H

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"

using namespace loopopt;

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Namespace

template <class LoopType> class HIRLoopAdapter {

private:
  LoopType *Lp;

public:
  HIRLoopAdapter(LoopType *L) { Lp = L; }

  bool isInnermost() const {
    llvm_unreachable("LoopType template argument is not supported.");
  }
};

template <> bool HIRLoopAdapter<Loop>::isInnermost() const {
  return Lp->getSubLoops().empty();
}

template <> bool HIRLoopAdapter<HLLoop>::isInnermost() const {
  return Lp->isInnermost();
}

} // End VPO Namespace
} // End LLVM Namespace
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPOLOOPADAPTERS_H
