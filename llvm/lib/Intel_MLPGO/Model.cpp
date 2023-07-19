//===- Model.cpp - Model ----------------------------------------*- C++ -*-===//
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

#include "llvm/Intel_MLPGO/Model.h"
#include "BranchPredictionModel.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <cmath>
#include <vector>

#include "onnx_model.inc"

#define DEBUG_TYPE "mlpgo"

#define PMAX (0x80000000)

#define TOKENPASTE(x, y) x##y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)

namespace llvm {
namespace mlpgo {

static unsigned char *getOnnxModel() { return &ONNX_MODEL[0]; }
static unsigned int getOnnxModelSize() { return TOKENPASTE2(ONNX_MODEL, _len); }

Model::Model() : mlengineBackend(nullptr) {
  mlengineBackend = new machine_learning_engine::BranchPredictionModel();

#ifndef INTEL_PRODUCT_RELEASE
  if (const char *modelFilePath = getenv("INTEL_MLPGO_MODEL_PATH")) {
    // get model specific onnx file name
    // construct the global path to the model file
    std::string FullPath(modelFilePath);
    FullPath += mlengineBackend->getModelFileName();

    // initialize model
#ifdef _WIN32
    std::wstring FullPathWide(FullPath.begin(), FullPath.end());
    machine_learning_engine::MlModelStatus bpModelInitializeStatus =
        mlengineBackend->initialize(FullPathWide.c_str());
#else
    machine_learning_engine::MlModelStatus bpModelInitializeStatus =
        mlengineBackend->initialize(FullPath.c_str());
#endif

    // validate initialization status
    if (bpModelInitializeStatus != machine_learning_engine::MlModelStatus::OK) {
      LLVM_DEBUG(llvm::dbgs() << "Failed to initializae ML model. Error: "
                              << (int)bpModelInitializeStatus << "\n");
      mlengineBackend = nullptr;
      return;
    }
    // print model version to log
    LLVM_DEBUG(llvm::dbgs() << "ML model version: "
                            << mlengineBackend->getVersion() << "\n");
    return;
  }
#endif // INTEL_PRODUCT_RELEASE

  machine_learning_engine::MlModelStatus bpModelInitializeStatus =
      mlengineBackend->initialize(getOnnxModel(), getOnnxModelSize());
  if (bpModelInitializeStatus != machine_learning_engine::MlModelStatus::OK) {
    mlengineBackend = nullptr;
  }
  LLVM_DEBUG(llvm::dbgs() << "ML model version: "
                          << mlengineBackend->getVersion() << "\n");
}

// Return the model status.
bool Model::ok() { return nullptr != mlengineBackend; }

Model::~Model() {
  if (this->mlengineBackend) {
    delete mlengineBackend;
  }
}

std::vector<unsigned> Model::calProbability(std::vector<float> Probs) const {

  for (float &Prob : Probs) {
    if (Prob > 1) {
      LLVM_DEBUG(llvm::dbgs() << "MLPGOModel: invalid probability (p > 1)\n");
      Prob = 1;
    } else if (Prob < 0) {
      LLVM_DEBUG(llvm::dbgs() << "MLPGOModel: invalid probability (p < 0)\n");
      Prob = 0;
    }
  }

  uint64_t totalProbability = 0;
  std::vector<unsigned> scaledProbs(Probs.size(), 0);
  for (unsigned i = 0; i < scaledProbs.size(); ++i) {
    scaledProbs[i] = std::round(Probs[i] * PMAX);
    totalProbability += scaledProbs[i];
  }

  std::vector<unsigned> FinalPrediction(Probs.size());
  for (unsigned i = 0; i < Probs.size(); ++i) {
    unsigned llvmBranchProbability =
        totalProbability == 0
            ? PMAX / (int)(Probs.size())
            : std::round(((double)scaledProbs[i] / totalProbability) * PMAX);
    FinalPrediction[i] = llvmBranchProbability;
  }

  return FinalPrediction;
}

std::vector<unsigned int>
Model::inference(const MLBrFeatureVec &FeatureVec) const {
  assert(mlengineBackend);
  std::vector<float> Prediction(FeatureVec.getNumOfSucc());
  if (FeatureVec.getNumOfSucc() == 2) {
    double bpModelPrediction = NOPRED;
    // run model
    machine_learning_engine::MlModelStatus bpModelPredictStatus =
        mlengineBackend->predict(&bpModelPrediction, FeatureVec);
    // validate model action
    if (bpModelPredictStatus != machine_learning_engine::MlModelStatus::OK)
      return {};

    Prediction[0] = bpModelPrediction;
    Prediction[1] = 1 - bpModelPrediction;
  }

  return calProbability(Prediction);
}

} // namespace mlpgo
} // namespace llvm
