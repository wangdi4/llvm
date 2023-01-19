//===- AddNTAttr.h - Add NT Attr -------------------------------*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_ADD_NT_ATTR_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_ADD_NT_ATTR_H

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
class AddNTAttrPass : public PassInfoMixin<AddNTAttrPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Function &F, AAResults &AaRet);

private:
  bool setNTAttr(StoreInst *SI);

  Function *F = nullptr;
};
} // namespace llvm
#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_ADD_NT_ATTR_H
