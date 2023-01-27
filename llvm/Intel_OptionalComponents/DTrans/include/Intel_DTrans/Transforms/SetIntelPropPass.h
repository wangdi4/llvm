//===--------------- SetIntelPropPass.h - SetIntelPropPass-----------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the SetIntelPropPass which sets the "Intel Proprietary"
// module flag.
//

#ifndef INTEL_DTRANS_TRANSFORMS_SETINTELPROPPASS_H
#define INTEL_DTRANS_TRANSFORMS_SETINTELPROPPASS_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

namespace dtrans {

// This pass sets the "Intel Proprietary" module flag
class SetIntelPropPass : public PassInfoMixin<dtrans::SetIntelPropPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // namespace dtrans

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_SETINTELPROPPASS_H
