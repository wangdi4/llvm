//===-------------AnnotatorCleaner.h - AnnotatorCleanerPass----------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans Annotation Cleanup (metadata/annotation
// intrinsics) pass which removes annotations created by the DTransAnnotator
// class.
//
#if !INTEL_INCLUDE_DTRANS
#error AnnotatorCleaner.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_ANNOTATORCLEANER_H
#define INTEL_DTRANS_TRANSFORMS_ANNOTATORCLEANER_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class WholeProgramInfo;

namespace dtrans {

// This pass removes the DTrans specific annotations created by the
// DTransAnnotator class. Removes both metadata or llvm.ptr.annotation intrinsic
// calls
class AnnotatorCleanerPass
    : public PassInfoMixin<dtrans::AnnotatorCleanerPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool runImpl(Module &M, WholeProgramInfo &WPInfo);

private:
  // Removes the DTrans specific annotations from the Function. Returns
  // 'true' if changes were made.
  bool cleanFunction(Function &F);
};

} // namespace dtrans

ModulePass *createDTransAnnotatorCleanerWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_ANNOTATORCLEANER_H
