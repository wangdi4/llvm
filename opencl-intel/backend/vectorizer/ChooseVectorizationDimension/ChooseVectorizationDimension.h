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
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VectorizationDimensionAnalysis.h"

using namespace llvm;

namespace intel {

// Function pass, used by Volcano
class ChooseVectorizationDimension : public FunctionPass {
public:
  static char ID;

  ChooseVectorizationDimension();

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  llvm::StringRef getPassName() const override {
    return "ChooseVectorizationDimension";
  }

  bool runOnFunction(Function &F) override;

  /// @brief Function for querying the vectorization dimension.
  unsigned getVectorizationDim() const { return VDInfo.getVectorizeDim(); }

  /// @brief Function for querying whether it is ok to unite workgroups.
  bool getCanUniteWorkgroups() const { return VDInfo.getCanUniteWorkGroups(); }

private:
  VectorizeDimInfo VDInfo;
};

} // namespace intel
#endif // define __CHOOSE_VECTORIZATION_DIMENSION_H_
