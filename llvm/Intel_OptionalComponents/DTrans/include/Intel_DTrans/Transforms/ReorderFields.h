//===--------------- ReorderFields.h - DTransReorderFieldsPass ------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans reorder fields optimization pass.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_REORDERFIELDS_H
#define INTEL_DTRANS_TRANSFORMS_REORDERFIELDS_H

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

namespace dtrans {

/// Pass to perform DTrans optimizations.
class ReorderFieldsPass : public PassInfoMixin<dtrans::ReorderFieldsPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool runImpl(Module &M, DTransAnalysisInfo &Info);

private:
  SafetyData ReorderFieldsSafetyConditions;

  // The pointers in this vector are owned by the DTransAnalysisInfo.
  SmallVector<dtrans::StructInfo *, 4> CandidateTypes;

  bool gatherCandidateTypes(DTransAnalysisInfo &DTInfo, const DataLayout &DL);
  bool isCandidateType(StructType *StructT, const DataLayout &DL);
  bool isCandidateTypeHasEnoughPadding(StructType *StructT,
                                       const DataLayout &DL);
};

} // namespace dtrans

ModulePass *createDTransReorderFieldsWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_REORDERFIELDS_H
