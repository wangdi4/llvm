//===------------- MemInitTrimDownOP.h - DTransMemInitTrimDownOPPass ------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Initial Memory Allocation Trim Down optimization
// pass for opaque pointers.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_MEMINITTRIMDOWNOP_H
#define INTEL_DTRANS_TRANSFORMS_MEMINITTRIMDOWNOP_H

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

#include "Intel_DTrans/Transforms/StructOfArraysOPInfoImpl.h"

namespace llvm {

namespace dtransOP {

/// Pass to perform Initial Memory Allocation Trim Down optimization.
class MemInitTrimDownOPPass : public PassInfoMixin<MemInitTrimDownOPPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  bool runImpl(Module &M, DTransSafetyInfo &Info, SOAGetTLITy GetTLI,
               WholeProgramInfo &WPInfo, SOADominatorTreeType &GetDT);
};

} // namespace dtransOP

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_MEMINITTRIMDOWNOP_H
