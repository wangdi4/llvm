//===----- RemoveDTransTypeMetadata.h - Remove DTrans type metadata -------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error RemoveDTransTypeMetadata.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_REMOVEDTRANSTYPEMETADATA_H
#define INTEL_DTRANS_TRANSFORMS_REMOVEDTRANSTYPEMETADATA_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
namespace dtransOP {

// This pass will remove all DTrans type metadate from the IR:
// - Structure descriptors from the DTrans metadata.
// - DTrans type metadata attachments on Functions and Instructions.
// - DTrans type attributes on Functions.
class RemoveAllDTransTypeMetadataPass
    : public PassInfoMixin<RemoveAllDTransTypeMetadataPass> {

public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  bool runImpl(Module &M);
};

// This pass will remove structure descriptors from the DTrans
// metadata for structure types that are not referenced in the IR,
// except by the DTrans type metadata. This enables removal of
// those structure types from the IR.
class RemoveDeadDTransTypeMetadataPass
    : public PassInfoMixin<RemoveDeadDTransTypeMetadataPass> {

public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  bool runImpl(Module &M);
};

} // end namespace dtransOP

ModulePass *createRemoveAllDTransTypeMetadataWrapperPass();
ModulePass *createRemoveDeadDTransTypeMetadataWrapperPass();

} // end namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_REMOVEDTRANSTYPEMETADATA_H
