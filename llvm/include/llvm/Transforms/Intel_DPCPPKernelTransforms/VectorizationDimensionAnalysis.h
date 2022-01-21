//===- VectorizationDimensionAnalysis.h - Vectorize dim analysis -*- C++-*-===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_VECTORIZATION_DIMENSION_ANALYSIS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_VECTORIZATION_DIMENSION_ANALYSIS_H

#include "llvm/ADT/MapVector.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DevLimits.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/WorkItemAnalysis.h"

namespace llvm {

class RuntimService;

/// VectorizeDimInfo decides which dimension is best for vectorization. The
/// heuristic dramatically prefers to vectorize in dimension 0, and only switch
/// dimension if the chances of harming performance this way seems very small.
/// Also, it is forbidden to change dimension at all in some cases, including:
/// 1. KernelAnalysis pass determines barrier pass must be used (usually a sign
///   of a barrier).
/// 2. get_local_id or get_group_id are used.
/// 3. local memory is used.
/// (in cases 2 & 3, even though we may theoretically switch dimension, we may
/// not later unite several workgroups to run together, so we dare not switch
/// dimension, because it is likely there is not enough size available to
/// support vectorization in a dimension different than 0.)
class VectorizeDimInfo {
public:
  VectorizeDimInfo();

  /// Return true if vectorization dimension is chosen to be 0 based on fast
  /// checks, e.g. NoBarrierPath is false.
  bool preCheckDimZero(Function &F, const RuntimeService *RTS);

  /// Compute analysis.
  void compute(Function &F, WorkItemInfo &WIInfo);

  // Return the vectorization dimension.
  unsigned getVectorizeDim() const { return VectorizeDim; }

  // Return whether it is ok to unite workgroups.
  bool getCanUniteWorkGroups() const { return CanUniteWorkGroups; }

  void print(raw_ostream &OS) const;

private:
  /// Test whether the function uses a certain dimension. (uses = calls
  /// get_*_id(dim)) \param F the function to test. \param Dim the dimension to
  /// look for.
  bool hasDim(Function *F, unsigned Dim) const;

  const RuntimeService *RTService;

  /// The vectorization dimension.
  unsigned VectorizeDim;

  /// Whether it is ok to unite workgroups.
  bool CanUniteWorkGroups;

  /// Whether the dimension exists.
  bool DimExist[MAX_WORK_DIM];

  /// Whether the dimension is a valid possibility.
  bool DimValid[MAX_WORK_DIM];

  /// True if there is at least one block that prefers dimension X over 0.
  bool SwitchMotivation[MAX_WORK_DIM];

  /// How many BB's perfer dimension 1/2.
  int PreferredDim[MAX_WORK_DIM];

  unsigned TotalDims;

  /// Statistics
  unsigned NumGoodLoadStores[MAX_WORK_DIM];
  unsigned NumBadLoadStores[MAX_WORK_DIM];
};

/// Analysis pass that provides VectorizeDimInfo.
class VectorizationDimensionAnalysis
    : public AnalysisInfoMixin<VectorizationDimensionAnalysis> {
  friend AnalysisInfoMixin<VectorizationDimensionAnalysis>;
  static AnalysisKey Key;

public:
  using Result = MapVector<Function *, VectorizeDimInfo>;

  Result run(Module &M, ModuleAnalysisManager &AM);
};

/// Printer pass for VectorizationDimensionAnalysis.
class VectorizationDimensionAnalysisPrinter
    : public PassInfoMixin<VectorizationDimensionAnalysisPrinter> {
  raw_ostream &OS;

public:
  explicit VectorizationDimensionAnalysisPrinter(raw_ostream &OS) : OS(OS) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

class VectorizationDimensionAnalysisLegacy : public ModulePass {
public:
  static char ID;

  VectorizationDimensionAnalysisLegacy();

  bool runOnModule(Module &M) override;

  StringRef getPassName() const override {
    return "VectorizationDimensionAnalysisLegacy";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  void print(raw_ostream &OS, const Module *) const override;

  using Result = MapVector<Function *, VectorizeDimInfo>;
  Result &getResult() { return VDInfos; }
  const Result &getResult() const { return VDInfos; }

private:
  Result VDInfos;
};

} // namespace llvm
#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_VECTORIZATION_DIMENSION_ANALYSIS_H
