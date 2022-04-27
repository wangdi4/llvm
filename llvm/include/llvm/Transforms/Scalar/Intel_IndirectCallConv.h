//===- Intel_IndirectCallConv.cpp - Indirect call Conv transformation -===//
////
//// Copyright (C) 2016-2022 Intel Corporation. All rights reserved.
////
//// The information and source code contained herein is the exclusive
//// property of Intel Corporation and may not be disclosed, examined
//// or reproduced in whole or in part without explicit written authorization
//// from the company.
////
////===-------------------------------------------------------------------===//
////
//// This pass performs indirect calls to direct calls conversion if possible
//// using points-to info. See implementation file for more details.
////
////===-------------------------------------------------------------------===//
//

#ifndef LLVM_TRANSFORMS_SCALAR_INTEL_INDIRECT_CALL_CONV_H
#define LLVM_TRANSFORMS_SCALAR_INTEL_INDIRECT_CALL_CONV_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class IndirectCallConvPass : public PassInfoMixin<IndirectCallConvPass> {
  bool UseAndersen;
#if INTEL_FEATURE_SW_DTRANS
  bool UseDTrans;
#endif // INTEL_FEATURE_SW_DTRANS

public:
#if INTEL_FEATURE_SW_DTRANS
  explicit IndirectCallConvPass(bool UseAndersen = false,
                                bool UseDTrans = false)
      : UseAndersen(UseAndersen), UseDTrans(UseDTrans) {}
#else // INTEL_FEATURE_SW_DTRANS
  explicit IndirectCallConvPass(bool UseAndersen = false)
      : UseAndersen(UseAndersen) {}
#endif // INTEL_FEATURE_SW_DTRANS
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_INTEL_INDIRECT_CALL_CONV_H
