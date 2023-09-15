//===---------------- CommuteCond.h - CommuteCondPass -------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans CommuteCond  optimization.
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error CommuteCond.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_COMMUTECOND_H
#define INTEL_DTRANS_TRANSFORMS_COMMUTECOND_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class WholeProgramInfo;

namespace dtransOP {
class DTransSafetyInfo;

class CommuteCondOPPass : public PassInfoMixin<CommuteCondOPPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  bool runImpl(Module &M, DTransSafetyInfo &Info, WholeProgramInfo &WPInfo);
};

} // end namespace dtransOP

} // end namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_COMMUTECOND_H
