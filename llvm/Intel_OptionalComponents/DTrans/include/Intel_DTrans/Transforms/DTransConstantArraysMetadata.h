//===---- DTransConstantArraysMetada.h - DTransConstantArraysMetada  -----===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//
// This pass will check if a field in an structure is an array with constant
// entries. If so, then collect the constant values and store them inside a
// metadata. This metadata can be used by the loop optimizer, constant
// propagation, or any other transformation that depends on constant values.
//
// This file declares the DTrans constant arrays metadata pass.
//===---------------------------------------------------------------------===//

#if !INTEL_INCLUDE_DTRANS
#error DTransConstantArraysMetadata.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_CONSTANTARRAYSMETADATA_H
#define INTEL_DTRANS_TRANSFORMS_CONSTANTARRAYSMETADATA_H

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

namespace dtrans {

/// Pass to perform DTrans constant arrays metadata.
class ConstantArraysMetadataPass
    : public PassInfoMixin<dtrans::ConstantArraysMetadataPass> {

public:
  ConstantArraysMetadataPass(){};

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  void runImpl(Module &M, DTransAnalysisInfo &DTInfo, WholeProgramInfo &WPInfo);
};

} // namespace dtrans

ModulePass *createDTransConstantArraysMetadataWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_CONSTANTARRAYSMETADATA_H