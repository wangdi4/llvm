// INTEL CONFIDENTIAL
//
// Copyright 2012-2020 Intel Corporation.
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
#include "BuiltinLibInfo.h"
#include "WIAnalysis.h"
#include "OclTune.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"

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
class ChooseVectorizationDimension : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid
  ChooseVectorizationDimension();

  /// @brief Provides name of pass
  llvm::StringRef getPassName() const override {
    return "ChooseVectorizationDimension";
  }

  /*! \name LLVM Interface
   * \{ */
  /// @brief LLVM function pass interface
  /// @param F function to test
  /// @return true if modified (always false)
  bool runOnFunction(Function &F) override;
  /// @brief requests analysis from LLVM system
  /// @param AU
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<BuiltinLibInfo>();
  }

  /// @brief Function for querying the vectorization dimension
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
  /// @param dim the dimension to look for.
  bool hasDim(Function* F, unsigned int dim);

  /// Runtime services pointer
  RuntimeServices * m_rtServices;

  /// The vectorized dimension
  unsigned int m_vectorizationDim;

  /// whether it is ok to unite workgroups.
  bool m_canUniteWorkgroups;

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

}
#endif //define __CHOOSE_VECTORIZATION_DIMENSION_H_
