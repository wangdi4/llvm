//===--- HIRIfReversal.h - Declaration of HIRIfReversal Pass -------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===--------------------------------------------------------------------===//
//
// Note:
// This file contains HIRIfReversal pass's class declaration.
// This allows a complete HIRIfReversal class declaration available to the
// HIRLoopTransformUtils.cpp file,so that it can instantiate a complete
// HIRIfReversal object to use inside the independent HIR Loop Transform
// Utility.
//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_IFREVERSALIMPL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_IFREVERSALIMPL_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {
namespace loopopt {

struct DirectionVector;
class HIRFramework;
class HIRDDAnalysis;
class HIRLoopStatistics;

class HIRIfReversal {
  HIRFramework &HIRF;
  HIRDDAnalysis &HDDA;
  HIRLoopStatistics &HLS;

public:
  HIRIfReversal(HIRFramework &HIRF, HIRDDAnalysis &HDDA, HIRLoopStatistics &HLS)
      : HIRF(HIRF), HDDA(HDDA), HLS(HLS) {}

  bool run();

  bool findProfitableCandidates(const HLLoop *Loop, SmallSet<HLIf *, 2> &IfSet);
};
} // namespace loopopt
} // namespace llvm

#endif
