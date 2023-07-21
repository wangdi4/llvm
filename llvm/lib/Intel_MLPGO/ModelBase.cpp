//===- ModelBase.cpp - Model Base -------------------------------*- C++ -*-===//
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

#include "ModelBase.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <cassert>

#define DEBUG_TYPE "mlpgo"

namespace machine_learning_engine {

MlModelBase::~MlModelBase() {}

std::string MlModelBase::getVersion() {
  // onnx model is not loaded
  if (!m_session) {
    return std::string();
  }
  constexpr char ONNX_MODEL_VER_KEY[] = "mlpgo_model_version";

  Ort::AllocatorWithDefaultOptions allocator;
  std::string modelVer =
      m_session->GetModelMetadata()
          .LookupCustomMetadataMapAllocated(ONNX_MODEL_VER_KEY, allocator)
          .get();
  return modelVer;
}

MlModelStatus MlModelBase::initialize(const void *modelData,
                                      size_t modelDataLength) {
  try {
    assert(modelData && modelDataLength);

    // Sanity check that the loaded onnxruntime supports the version
    // we compiled to.
    if (!OrtGetApiBase()->GetApi(ORT_API_VERSION)) {
      LLVM_DEBUG(llvm::dbgs()
                 << "Ort failed to load API version " << ORT_API_VERSION << ". "
                 << "The loaded onnxruntime is version "
                 << OrtGetApiBase()->GetVersionString() << ". "
                 << "Check that the right onnxruntime was loaded.\n");
      return MlModelStatus::ORT_ERROR;
    }

    // setup onnxruntime environment
    // ort env is a single instance per process, need to keep this in mind for
    // future multi model support
    m_ortEnv = std::make_shared<Ort::Env>(ORT_LOGGING_LEVEL_FATAL, "cpp_onnx");
    m_ortEnv->DisableTelemetryEvents();

    // create session and store in this object for future use
    Ort::SessionOptions sessionOptions;
    m_session = std::make_shared<Ort::Session>(*m_ortEnv, modelData,
                                               modelDataLength, sessionOptions);
    // disable collection and sending of telemetry events to Microsoft

    return MlModelStatus::OK;
  } catch (Ort::Exception ortException) {
    LLVM_DEBUG(llvm::dbgs()
               << "Ort failed to load the model, exception error code: "
               << ortException.GetOrtErrorCode() << "; " << ortException.what()
               << "\n");

    return MlModelStatus::ORT_ERROR;
  } catch (...) {
    LLVM_DEBUG(llvm::dbgs() << "Failed to load the model\n");

    return MlModelStatus::GENERAL_ERROR;
  }
}

MlModelStatus MlModelBase::initialize(const ORTCHAR_T *modelFilePath) {
  try {
    // model file path validation
    if (modelFilePath == nullptr || *modelFilePath == '\0') {
      return MlModelStatus::INPUT_ERROR;
    }

    // setup onnxruntime environment
    // ort env is a single instance per process, need to keep this in mind for
    // future multi model support
    m_ortEnv = std::make_shared<Ort::Env>(ORT_LOGGING_LEVEL_FATAL, "cpp_onnx");
    m_ortEnv->DisableTelemetryEvents();

    // create session and store in this object for future use
    Ort::SessionOptions sessionOptions;
    m_session = std::make_shared<Ort::Session>(*m_ortEnv, modelFilePath,
                                               sessionOptions);

    return MlModelStatus::OK;
  } catch (Ort::Exception ortException) {
    LLVM_DEBUG(llvm::dbgs()
               << "Ort failed to load the model, exception error code: "
               << ortException.GetOrtErrorCode() << "; " << ortException.what()
               << "\n");

    return MlModelStatus::ORT_ERROR;
  } catch (...) {
    LLVM_DEBUG(llvm::dbgs() << "Failed to load the model\n");

    return MlModelStatus::GENERAL_ERROR;
  }
}

MlModelStatus MlModelBase::runModel(std::vector<float> input,
                                    std::vector<float> &output) {
  try {
    // validate we got the features
    if (input.empty()) {
      LLVM_DEBUG(llvm::dbgs()
                 << "Failed to execute the model, the input is empty\n");
      return MlModelStatus::INPUT_ERROR;
    }

    // alocate input tensor
    // currently we assume only 2 dimensions, the first (index 0) refers to
    // batch size we assume it is set to 1 in onnx file
    auto inputTensorInfo =
        m_session->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo();
    int64_t inputSize = input.size();
    std::vector<int64_t> inputDims = {1,
                                      inputSize}; // inputTensorInfo.GetShape();
    auto inputTensorSize = inputDims[1];

    // validate user features amount and model expected features amount match
    if ((size_t)inputTensorSize != input.size()) {
      LLVM_DEBUG(llvm::dbgs()
                 << "Failed to execute the model, expected input length: "
                 << inputTensorSize << ", given input length: " << input.size()
                 << "\n");
      return MlModelStatus::INPUT_ERROR;
    }

    // fill input tensor with given features
    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(
        OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

    std::vector<float> inputTensorValues(input);
    std::vector<Ort::Value> inputTensor;

    inputTensor.push_back(Ort::Value::CreateTensor<float>(
        memoryInfo, inputTensorValues.data(), inputTensorValues.size(),
        inputDims.data(), inputDims.size()));

    // classification and regression models have veriying number of outputs
    // classification has 2 outputs, one for classes and one for probabilities
    // regression has 1 output
    // we take the first input only, because we are planning for it to be a
    // single one in all cases we take the last output, because for regression
    // it is the only one and classification the last one is "probabilities"
    Ort::AllocatorWithDefaultOptions allocator;
    auto inputNameLocation = 0;
    auto outputNameLocation = m_session->GetOutputCount() - 1;
    auto inputName =
        m_session->GetInputNameAllocated(inputNameLocation, allocator);
    auto outputName =
        m_session->GetOutputNameAllocated(outputNameLocation, allocator);
    std::vector<const char *> inputNames{inputName.get()};
    std::vector<const char *> outputNames{outputName.get()};

    // run inference
    auto outputTensors = m_session->Run(
        Ort::RunOptions{nullptr}, inputNames.data(), inputTensor.data(),
        inputNames.size(), outputNames.data(), outputNames.size());

    // validate infernce result length matches the expected
    if (outputTensors.empty()) {
      LLVM_DEBUG(llvm::dbgs()
                 << "Failed to execute the model, got empty output\n");
      return MlModelStatus::INPUT_ERROR;
    }

    // retrieve inference results
    // we take outputValues index 1 because we need only one output and it is
    // the second one
    auto &ortValue = outputTensors[0];
    const auto *modelResults = ortValue.GetTensorMutableData<float>();
    const auto modelResultsSize =
        ortValue.GetTensorTypeAndShapeInfo().GetElementCount();

    // validate inference has result
    // the magic numbers here relate to the classification vs regression thingy
    // described above in the code
    auto expectedOutputSize = m_session->GetOutputTypeInfo(outputNameLocation)
                                  .GetTensorTypeAndShapeInfo()
                                  .GetShape()[1];
    if (modelResultsSize != (size_t)expectedOutputSize) {
      LLVM_DEBUG(llvm::dbgs()
                 << "Failed to execute the model, output size "
                 << modelResultsSize << ", does not match the expected "
                 << expectedOutputSize << "\n");
      return MlModelStatus::INPUT_ERROR;
    }

    // debug print of inference results
    for (size_t i = 0; i < modelResultsSize; ++i) {
      LLVM_DEBUG(llvm::dbgs() << "Model result " << i << " : "
                              << std::to_string(modelResults[i]) << "\n");
    }

    // copy model results to output
    for (size_t i = 0; i < modelResultsSize; ++i) {
      output.push_back(modelResults[i]);
    }

    return MlModelStatus::OK;
  } catch (Ort::Exception ortException) {
    LLVM_DEBUG(llvm::dbgs() << "Ort failed to execute the model:\n");
    LLVM_DEBUG(llvm::dbgs()
               << "exception message: " << ortException.what() << "\n");
    LLVM_DEBUG(llvm::dbgs() << "exception error code: "
                            << ortException.GetOrtErrorCode() << "\n");

    return MlModelStatus::ORT_ERROR;
  } catch (...) {
    LLVM_DEBUG(llvm::dbgs() << "Failed to execute the model");

    return MlModelStatus::GENERAL_ERROR;
  }
}

} // namespace machine_learning_engine
