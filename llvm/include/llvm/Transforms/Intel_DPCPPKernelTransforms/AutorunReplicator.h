//===-- AutorunReplicator.h - Autorun Replicator ----------------*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation
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
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_AUTORUN_REPLICATOR_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_AUTORUN_REPLICATOR_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class AutorunReplicatorPass : public PassInfoMixin<AutorunReplicatorPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M);

private:
  struct ComputeID {
    ComputeID(int X, int Y, int Z) : XDimValue(X), YDimValue(Y), ZDimValue(Z) {}

    int XDimValue = -1;
    int YDimValue = -1;
    int ZDimValue = -1;
  };

  bool createReplicas(Function *F, SmallVectorImpl<Function *> &Replicas);

  void resolveGetComputeID(CallInst *GetComputeIDCall);

  using ComputeIDsMapTy = DenseMap<Function *, ComputeID>;

  ComputeIDsMapTy ComputeIDs;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_AUTORUN_REPLICATOR_H
