//===---------------- CommuteCond.h - CommuteCondPass -------------------===//
//
// Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
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

#if !INTEL_INCLUDE_DTRANS
#error CommuteCond.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_COMMUTECOND_H
#define INTEL_DTRANS_TRANSFORMS_COMMUTECOND_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class WholeProgramInfo;
class DTransAnalysisInfo;

namespace dtrans {

class CommuteCondPass : public PassInfoMixin<dtrans::CommuteCondPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool runImpl(Module &M, DTransAnalysisInfo &Info, WholeProgramInfo &WPInfo);
};

} // end namespace dtrans

ModulePass *createDTransCommuteCondWrapperPass();

} // end namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_COMMUTECOND_H
