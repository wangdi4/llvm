//===- BranchPredictionModel.h  - Branch prediction model -------*- C++ -*-===//
//
// Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_MLPGO_INTEL_BRANCH_PREDICTION_MODEL_H
#define LLVM_LIB_MLPGO_INTEL_BRANCH_PREDICTION_MODEL_H

#include "ModelBase.h"
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDesc.h"

#include <string>

namespace machine_learning_engine {

#define NOPRED -1

class BranchPredictionModel : public MlModelBase {
public:
  std::string getModelFileName() override;
  MlModelStatus predict(double *prediction,
                        const llvm::mlpgo::MLBrFeatureVec &FeatureVec);
};

} // namespace machine_learning_engine

#endif // LLVM_LIB_MLPGO_INTEL_BRANCH_PREDICTION_MODEL_H
