//===-- VPLoopInfo.h --------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines VPLoopInfo analysis and VPLoop class. VPLoopInfo is a
/// specialization of LoopInfoBase for VPBlockBase. VPLoops is a specialization
/// of LoopBase that is used to hold loop metadata from VPLoopInfo. Further
/// information can be found in VectorizationPlanner.rst.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLAN_VPLOOPINFO_H 
#define LLVM_TRANSFORMS_VECTORIZE_VPLAN_VPLOOPINFO_H

#include "llvm/Analysis/LoopInfoImpl.h"

namespace llvm {
class VPBlockBase;
namespace vpo {

/// A VPLoop holds analysis information for every loop detected by VPLoopInfo.
/// It is an instantiation of LoopBase.
class VPLoop : public LoopBase<VPBlockBase, VPLoop> {
private:
  friend class LoopInfoBase<VPBlockBase, VPLoop>;
  explicit VPLoop(VPBlockBase *VPB) : LoopBase<VPBlockBase, VPLoop>(VPB) {}
};

/// VPLoopInfo provides analysis of natural loop for VPBlockBase-based
/// Hierarchical CFG. It is a specialization of LoopInfoBase class.
typedef LoopInfoBase<VPBlockBase, VPLoop> VPLoopInfo;

} // End VPO Vectorizer Namespace
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_VPLAN_VPLOOPINFO_H 

