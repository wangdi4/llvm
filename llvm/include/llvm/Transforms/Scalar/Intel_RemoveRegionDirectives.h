//==--- Intel_RemoveRegionDirectives.h - Header file of RemoveRegionDirectives pass -*- C++ -*---==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef __REMOVE_REGION_DIRECTIVES_H__
#define __REMOVE_REGION_DIRECTIVES_H__

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"

namespace llvm {

/// RemoveRegionDirectives class removes llvm.directive.region.entry
class RemoveRegionDirectivesPass :
  public PassInfoMixin<RemoveRegionDirectivesPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &);
};
} // end namespace llvm

#endif // __REMOVE_REGION_DIRECTIVES_H__
