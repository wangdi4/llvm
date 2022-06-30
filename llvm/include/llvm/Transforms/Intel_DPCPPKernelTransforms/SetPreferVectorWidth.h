//===------ SetPreferVectorWidth.h - Set prefer vector width ----*- C++ -*-===//
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

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SET_PREFER_VECTOR_WIDTH_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SET_PREFER_VECTOR_WIDTH_H

#include "llvm/IR/Intel_VectorVariant.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class SetPreferVectorWidthPass
    : public PassInfoMixin<SetPreferVectorWidthPass> {
public:
  SetPreferVectorWidthPass(VectorVariant::ISAClass ISA = VectorVariant::XMM);

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &FAM);

  // Glue for old PM.
  bool runImpl(Module &M);

private:
  VectorVariant::ISAClass ISA;
};

} // namespace llvm
#endif
