//===- CSAGraphSplitter.h - Remove graph's cross dependencies ---*- C++ -*-===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Declaration of a pass which eliminates cross dependencies between different
/// CSA graphs.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CSA_TRANSFORMS_SCALAR_CSA_GRAPH_SPLITTER_H
#define LLVM_CSA_TRANSFORMS_SCALAR_CSA_GRAPH_SPLITTER_H

#include "llvm/IR/PassManager.h"

namespace llvm {

struct CSAGraphSplitterPass : public PassInfoMixin<CSAGraphSplitterPass> {
  CSAGraphSplitterPass() {}
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_CSA_TRANSFORMS_SCALAR_CSA_GRAPH_SPLITTER_H