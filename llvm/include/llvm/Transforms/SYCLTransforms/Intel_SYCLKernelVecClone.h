//===----------- Intel_SYCLKernelVecClone.h - Class definition -*- C++-*--===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file defines the SYCLKernelVecClone pass class.
///
// ===--------------------------------------------------------------------=== //
#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_VEC_CLONE_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_VEC_CLONE_H

#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/SYCLTransforms/Intel_VectorizationDimensionAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Utils/Intel_VecClone.h"

namespace llvm {

enum GlobalWorkSizeLT2GState : uint8_t { GWS_FALSE, GWS_TRUE, GWS_AUTO };

class SYCLKernelVecCloneImpl : public VecCloneImpl {
public:
  SYCLKernelVecCloneImpl(ArrayRef<VectItem> VectInfos, VFISAKind ISA);

  void setIsOCL(bool IsOCL) { this->IsOCL = IsOCL; }

  void setVectorizationDimensionMap(
      const VectorizationDimensionAnalysis::Result *VDMap) {
    this->VDMap = VDMap;
  }

private:
  // Configuration options
  ArrayRef<VectItem> VectInfos;
  VFISAKind ISA;
  bool IsOCL = false;

  SYCLKernelMetadataAPI::KernelList::KernelVectorTy Kernels;
  const VectorizationDimensionAnalysis::Result *VDMap;
  DenseMap<Function *, SmallVector<CallInst *, 8>> FuncToTIDBuiltinCalls;

  // Kernel-related code transformation: 1. updates all the uses of TID calls
  // OpenCL example:
  // with TID + new induction variable. TIDs are updated in this way because
  // they are treated as linears. 2. hoists the TID call out of the loop and
  // updates the users. If we kept the get_global_id()/get_local_id() inside the
  // loop, then we will need a vector verison of it. Instead, we simply move it
  // outside of the loop and we generate TID+1,TID+2,TID+3 etc. Hoisting the TID
  // calls outside of the for-loop might create additional load/stores for some
  // kernels with barriers.
  void handleLanguageSpecifics(Function &F, PHINode *Phi, Function *Clone,
                               BasicBlock *EntryBlock, const VFInfo &Variant,
                               const ValueToValueMapTy &VMap) override;

  // Prepare OpenCL kernel for VecClone (emits vector-variant attributes).
  void languageSpecificInitializations(Module &M) override;
};

class SYCLKernelVecClonePass : public PassInfoMixin<SYCLKernelVecClonePass> {
private:
  SYCLKernelVecCloneImpl Impl;

public:
  explicit SYCLKernelVecClonePass(ArrayRef<VectItem> VectInfos = {},
                                  VFISAKind ISA = VFISAKind::SSE);

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  static bool isRequired() { return true; }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_VEC_CLONE_H
