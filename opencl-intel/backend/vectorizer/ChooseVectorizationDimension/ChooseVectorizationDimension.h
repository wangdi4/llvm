/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
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

  enum PreferredOption {
    First = 0,
    Second = 1,
    Neutral = 2
  };

public:
  static char ID; // Pass identification, replacement for typeid
  ChooseVectorizationDimension();

  /// @brief Provides name of pass
  virtual llvm::StringRef getPassName() const {
    return "ChooseVectorizationDimension";
  }

  /*! \name LLVM Interface
   * \{ */
  /// @brief LLVM function pass interface
  /// @param F function to test
  /// @return true if modified (always false)
  virtual bool runOnFunction(Function &F);
  /// @brief requests analysis from LLVM system
  /// @param AU
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<BuiltinLibInfo>();
  }

  /// @brief sets the scalar version of the function it is going to be run on.
  void setScalarFunc(Function* func);

   /// @brief Function for querying the vectorization dimension
   unsigned int getVectorizationDim();

   /// @brief Function for querying whether it is ok to unite workgroups.
   bool getCanUniteWorkgroups();

private:
  /// @brief writes the conclusions into the metadata.
  /// @param dim the dimension to write.
  /// @param canUniteWorkGroups the decision whether or not it is legal to unite work groups.
  void setFinalDecision(int dim, bool canUniteWorkGroups);

  /// @brief checks whether it is even allowed to switch dimensions.
  /// specifically: a) KernelAnalysis pass determined no barrier
  /// b) no get_local_id and get_group_id c) no access to local memory.
  /// @param F function to test.
  bool canSwitchDimensions(Function* F);

  /// @brief tests whether the function uses a certain dimension.
  /// (uses = calls get_***_id(dim))
  /// @param F the function to test.
  /// @param dim the dimension to look for.
  bool hasDim(Function* F, unsigned int dim);

  /// @brief counts how many good load stores and bad load stores
  /// are in the given BasicBlock, if using the provided WIAnalysis.
  /// Good load stores: uniform load/stores and consecutive scalar load/stores.
  /// Bad load stores: random/strided load stores and also vector consecutive load/stores.
  /// @param wi the WIAnlaysis to ask for the dependency of the pointer param.
  /// @param BB the basic block to count its instructions
  /// @param goodLoadStores write the number of good load/stores into this param.
  /// @param badLoadStores write the number of bad load/stores into this param.
  /// @param dim the dimension we are counting. Only needed for statistics.
  void countLoadStores(WIAnalysis* wi, BasicBlock* BB,
    int& goodLoadStores, int& badLoadStores, int dim);

  /// @brief compares this basic block under for two different dimensions,
  /// and outputs which dimension is better for vectorization considering
  /// this specific basic block. Answer is: first/second/neutral.
  /// When choosing between the two options, prefer the one with a higher
  /// number of good load/stores. If number is identical, prefer
  /// an option under which the basic block is uniform rather than divergent.
  /// if uniformity/divergency is also identical, return neutral.
  /// @param isDivergentInFirst is the block divergent in if vectorizing by the first dimension contested.
  /// @param isDivergentInSecond is the block divergent in if vectorizing by the second dimension contested.
  /// @param goodLoadStoresFirst number of good load/stores for the first dimension contested.
  /// @param goodLoadStoresSecond number of good load/stores for the second dimension contested.
  PreferredOption getPreferredOption(bool isDivergentInFirst, bool isDivergentInSecond,
    int goodLoadStoresFirst, int goodLoadStoresSecond);

  /// Runtime services pointer
  RuntimeServices * m_rtServices;

  /// the scalar func.
  Function* m_scalarFunc;

  /// The vectorized dimension
  unsigned int m_vectorizationDim;

  /// whether it is ok to unite workgroups.
  bool m_canUniteWorkgroups;

  // Statistics:
  Statistic::ActiveStatsT m_kernelStats;
  Statistic Dim_Zero_Good_Store_Loads;
  Statistic Dim_Zero_Bad_Store_Loads;
  Statistic Dim_One_Good_Store_Loads;
  Statistic Dim_One_Bad_Store_Loads;
  Statistic Dim_Two_Good_Store_Loads;
  Statistic Dim_Two_Bad_Store_Loads;
  Statistic Chosen_Vectorization_Dim;
};

}
#endif //define __CHOOSE_VECTORIZATION_DIMENSION_H_
