// INTEL CONFIDENTIAL
//
// Copyright 2012-2022 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __CHOOSE_VECTORIZATION_DIMENSION_H_
#define __CHOOSE_VECTORIZATION_DIMENSION_H_

#include "Logger.h"
#include "OclTune.h"

#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DevLimits.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/WorkItemAnalysis.h"

using namespace llvm;

namespace intel {
/// @brief ChooseVectorizationDimension pass is used to decide
/// which dimension is best for vectorization. The heuristic dramatically
/// prefers to vectorize in dimension 0, and only switch dimension
/// if the chances of harming performance this way seems very small.
/// also, in some cases it is forbidden to change dimensions at all.
/// these cases include:
/// 1. KernelAnalysis pass determined barrier pass must be used (usually a sign of a barrier)
/// 2. get_local_id or get_group_id are used.
/// 3. local memory is used.
/// (in cases 2 & 3, even though we may theoretically switch dimensions, we may not
/// later unite several workgroups to run together, so we dare not switch dimensions,
/// because it is likely there is not enough size avaialble to support vectorization
/// in a dimension different than 0.)
class ChooseVectorizationDimensionImpl {
public:

  ChooseVectorizationDimensionImpl();

  /// Return true if vectorization dimension is chosen to be 0 based on fast
  /// checks, e.g. NoBarrierPath is false.
  bool preCheckDimZero(Function &F, RuntimeService *RTS);

  bool run(Function &F, WorkItemInfo &WIInfo);

  /// @brief Function for querying the vectorization dimension.
  unsigned getVectorizationDim() const {
    return m_vectorizationDim;
  }

  /// @brief Function for querying whether it is ok to unite workgroups.
  bool getCanUniteWorkgroups() const {
    return m_canUniteWorkgroups;
  }

private:
  /// @brief writes the conclusions into the metadata.
  /// @param dim the dimension to write.
  /// @param canUniteWorkGroups the decision whether or not it is legal to unite work groups.
  void setFinalDecision(int dim, bool canUniteWorkGroups);

  /// @brief tests whether the function uses a certain dimension.
  /// (uses = calls get_***_id(dim))
  /// @param F the function to test.
  /// @param Dim the dimension to look for.
  /// @param RTS runtime service.
  bool hasDim(Function *F, unsigned int Dim);

  /// Runtime service.
  RuntimeService *RTService;

  /// The vectorized dimension
  unsigned int m_vectorizationDim;

  /// whether it is ok to unite workgroups.
  bool m_canUniteWorkgroups;

  /// Whether the dimension exists.
  bool DimExist[MAX_WORK_DIM];
  /// Whether the dimension is a valid possibility.
  bool DimValid[MAX_WORK_DIM];
  // True if there is at least one block that prefers dimension x over 0.
  bool SwitchMotivation[MAX_WORK_DIM];
  /// How many BB's perfer dimension 1/2.
  int PreferredDim[MAX_WORK_DIM];

  int TotalDims;

#ifndef INTEL_PRODUCT_RELEASE
  // Statistics:
  Statistic::ActiveStatsT m_kernelStats;
  Statistic Dim_Zero_Good_Store_Loads;
  Statistic Dim_Zero_Bad_Store_Loads;
  Statistic Dim_One_Good_Store_Loads;
  Statistic Dim_One_Bad_Store_Loads;
  Statistic Dim_Two_Good_Store_Loads;
  Statistic Dim_Two_Bad_Store_Loads;
  Statistic Chosen_Vectorization_Dim;
#endif // INTEL_PRODUCT_RELEASE
};

// Function pass, used by Volcano
class ChooseVectorizationDimension : public FunctionPass {
public:
  static char ID;

  ChooseVectorizationDimension() : FunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<BuiltinLibInfoAnalysisLegacy>();
    AU.addRequired<WorkItemAnalysisLegacy>();
  }

  llvm::StringRef getPassName() const override {
    return "ChooseVectorizationDimension";
  }

  bool runOnFunction(Function &F) override {
    auto *RTService = getAnalysis<BuiltinLibInfoAnalysisLegacy>()
                          .getResult()
                          .getRuntimeService();
    if (!Impl.preCheckDimZero(F, RTService)) {
      WorkItemInfo &WIInfo = getAnalysis<WorkItemAnalysisLegacy>().getResult();
      Impl.run(F, WIInfo);
    }
    return false;
  }

  /// @brief Function for querying the vectorization dimension.
  unsigned getVectorizationDim() const { return Impl.getVectorizationDim(); }

  /// @brief Function for querying whether it is ok to unite workgroups.
  bool getCanUniteWorkgroups() const { return Impl.getCanUniteWorkgroups(); }

private:
  ChooseVectorizationDimensionImpl Impl;
};

// Module pass, used by VPlan
class ChooseVectorizationDimensionModulePass : public ModulePass {
public:
  static char ID;

  ChooseVectorizationDimensionModulePass() :
    ModulePass(ID)
  {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<BuiltinLibInfoAnalysisLegacy>();
    AU.addRequired<WorkItemAnalysisLegacy>();
  }

  llvm::StringRef getPassName() const override {
    return "ChooseVectorizationDimensionModulePass";
  }

  bool runOnModule(Module &M) override;

  /// @brief Function for querying the vectorization dimension.
  unsigned getVectorizationDim(Function *F) const {
    assert(ChosenVecDims.count(F) && "Not a kernel function?");
    return ChosenVecDims.find(F)->second;
  }

  /// @brief Function for querying whether it is ok to unite workgroups.
  bool getCanUniteWorkgroups(Function *F) const {
    assert(CanUniteWorkgroups.count(F) && "Not a kernel function?");
    return CanUniteWorkgroups.find(F)->second;
  }

private:
  SmallDenseMap<Function *, unsigned> ChosenVecDims;
  SmallDenseMap<Function *, bool> CanUniteWorkgroups;
};

}
#endif //define __CHOOSE_VECTORIZATION_DIMENSION_H_
