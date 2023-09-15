#if INTEL_FEATURE_SW_ADVANCED
//===----------- Intel_TileMVInlMarker.h ----------------------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// Multiversioning and marking for inlining of tiled functions
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_TILEMVINLMARKER_H
#define LLVM_TRANSFORMS_IPO_INTEL_TILEMVINLMARKER_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

///
/// Pass to perform multiversioning of a region for tiling and marking tiled
/// functions for inlining.
///
class TileMVInlMarkerPass : public PassInfoMixin<TileMVInlMarkerPass> {
public:
  TileMVInlMarkerPass(void);
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm
#endif // LLVM_TRANSFORMS_IPO_INTEL_TILEMVINLMARKER_H
#endif // INTEL_FEATURE_SW_ADVANCED
