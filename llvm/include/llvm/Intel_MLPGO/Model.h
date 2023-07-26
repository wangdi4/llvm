//===- Model.h - Model ------------------------------------------*- C++ -*-===//
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

#ifndef LLVM_MLPGO_INTEL_MODEL_H
#define LLVM_MLPGO_INTEL_MODEL_H

#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDesc.h"

namespace machine_learning_engine {
class BranchPredictionModel;
}

namespace llvm {
namespace mlpgo {

// Machine learning backend wrapping.
// After instantiating this object,
// the model status must be manually checked through ok() function.
// throw: std::runtime_error
//        llvm::mlpgo::UndefinedEnvironmentVariable
class Model {
public:
  Model();

  ~Model();

  bool ok();

  std::vector<unsigned int> inference(const MLBrFeatureVec &FeatureVec) const;

  std::vector<unsigned int> calProbability(std::vector<float> Probs) const;

protected:
  machine_learning_engine::BranchPredictionModel *mlengineBackend;
};

} // namespace mlpgo
} // namespace llvm

#endif // LLVM_MLPGO_INTEL_MODEL_H
