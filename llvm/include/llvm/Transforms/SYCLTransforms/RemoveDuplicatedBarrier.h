//==--- RemoveDuplicatedBarrier.h - RemoveDuplicatedBarrier pass - C++ -*---==//
//
// Copyright 2012 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_REMOVE_DUPLICATED_BARRIER_PASS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_REMOVE_DUPLICATED_BARRIER_PASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierUtils.h"

namespace llvm {
/// @brief RemoveDuplicatedBarrier pass is a module pass used to prevent
/// barrier/dummyBarrier instructions from appearing in sequence,
/// i.e. if two or more such instructions appears in sequence, keep only one
/// and remove the rest according to the following rules:
/// dummyBarrier-barrier(global) : do nothing
/// dummyBarrier-Any : remove Any
/// barrier-barrier : remove the one with local argument if exists or any
class RemoveDuplicatedBarrierPass
    : public PassInfoMixin<RemoveDuplicatedBarrierPass> {
public:
  bool runImpl(Module &M);
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

private:
  BarrierUtils BarrierUtil;
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_REMOVE_DUPLICATED_BARRIER_PASS_H
