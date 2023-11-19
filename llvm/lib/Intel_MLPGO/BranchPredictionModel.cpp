//===- BranchPredictionModel.cpp - Branch Prediction Model ------*- C++ -*-===//
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

#include "BranchPredictionModel.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDesc.h"

#include <cassert>
#include <cmath>

#define DEBUG_TYPE "mlpgo"

namespace machine_learning_engine {

std::string BranchPredictionModel::getModelFileName() {
  return "icx_bp_0.1.1.onnx";
}
MlModelStatus
BranchPredictionModel::predict(double *prediction,
                               const llvm::mlpgo::MLBrFeatureVec &FeatureVec) {
  try {
    *prediction = NOPRED;

    assert(FeatureVec.getNumOfSucc() == 2);
    const llvm::mlpgo::BrSrcBBFeturesT &SrcBBFeatures =
        FeatureVec.getSrcBBFeatures();

    // Note: it is crucial that the features will be placed in the vector in the
    // exact order expected by the model
    std::vector<float> input;

    input.insert(input.end(), {
#define BR_SRC_BB_FEATURE(Type, Name) (float)SrcBBFeatures.Name,
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDesc.def"
#undef BR_SRC_BB_FEATURE
                              });
    LLVM_DEBUG(llvm::dbgs() << "Input vector is:\n");
#define BR_SRC_BB_FEATURE(Type, Name)                                          \
  LLVM_DEBUG(llvm::dbgs() << SrcBBFeatures.Name << " ");
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDesc.def"
#undef BR_SRC_BB_FEATURE

    for (size_t SuccIdx = 0; SuccIdx < FeatureVec.getNumOfSucc(); ++SuccIdx) {
      const llvm::mlpgo::BrSuccFeaturesT &SuccFeatures =
          FeatureVec.getSuccFeatures(SuccIdx);

      input.insert(input.end(), {
#define BR_SUCC_BB_FEATURE(Type, Name) (float)SuccFeatures.Name,
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDesc.def"
#undef BR_SUCC_BB_FEATURE
                                });
#define BR_SUCC_BB_FEATURE(Type, Name)                                         \
  LLVM_DEBUG(llvm::dbgs() << SuccFeatures.Name << " ");
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDesc.def"
#undef BR_SUCC_BB_FEATURE
    }

    LLVM_DEBUG(llvm::dbgs() << "\n");

    std::vector<float> output;

    // TODO: validate output size is as expected. probably initialize() should
    // keep output size read from onnx as member of this class

    MlModelStatus MLStatus = this->runModel(input, output);
    if (MLStatus != MlModelStatus::OK)
      return MLStatus;

    LLVM_DEBUG(llvm::dbgs() << "Branch Prediction Model result is: "
                            << std::to_string(output[0]) << "\n");

    *prediction = output[0];

    return MlModelStatus::OK;
  }

  catch (...) {
    LLVM_DEBUG(llvm::dbgs() << "Failed to execute the model\n");
    return MlModelStatus::GENERAL_ERROR;
  }
}

} // namespace machine_learning_engine
