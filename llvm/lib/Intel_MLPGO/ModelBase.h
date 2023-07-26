//===- ModelBase.h - Model Base ---------------------------------*- C++ -*-===//
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

#pragma once

#ifndef NO_ONNXRUNTIME
// We are deliberately not providing 32-bit target ONNX libraries because
// majority of our products is 64-bit only. In case a 32-bit product
// attempts to use ONNX errors will be reported, which should be gracefully
// handled by the caller.
#if !defined(__x86_64__) && !defined(_WIN64)
#define NO_ONNXRUNTIME
#endif
#endif // !NO_ONNXRUNTIME

// If ONNX runtime is not supported still define our infrastructure to avoid
// guarding multiple places throughout the compiler with NO_ONNXRUNTIME
// macro. We will just return runtime errors if methods are used in
// unsupported environments.
//
#ifdef NO_ONNXRUNTIME
// Just mute the global static ORT initialization that is on by default.
#define ORT_API_MANUAL_INIT
#endif // NO_ONNXRUNTIME

#include <onnxruntime_cxx_api.h>

namespace machine_learning_engine {

/**
 * All ML models API functions that implement a significant action, use this
 * enum as a return value We do not use the exception mechanism, to comply with
 * users environment and coding convention
 */
enum class MlModelStatus {
  OK = 0,
  INPUT_ERROR = 1,
  ORT_ERROR = 2,
  GENERAL_ERROR = 3,
};

/**
 * ML models base class. It is a mix of API definition and specific inference
 * framework implementation (onnx). Each new model introduced is a new class
 * that should:
 * - inherite this class
 * - implement the pure virtual functions of this class
 * - define model specific prediction enum and features struct
 * - define and implement a 'predict' function which recieves the specific
 * features and returns specific prediction
 */
class MlModelBase {

protected:
  /**
   * These parameters are required by onnx inference execution ( run() ),
   * however we aquire them in onnx initialization phase ( initialize() ).
   * Model initialization happens once, while inference happens multiple times,
   * and both are called at different times.
   * Thus we need to store these parameters as members of the class,
   * as a way to pass them from initialization time to inference time.
   * These members are protected and not private to let inheriting test class to
   * access them
   */
  std::shared_ptr<Ort::Session> m_session = NULL;
  std::shared_ptr<Ort::Env> m_ortEnv = NULL;

  /**
   * This function is responsible for the model inference.
   * It assumes onnx file was loaded and all the model metadata is available.
   * It uses onnx API and the loaded onnx runtime object (done at
   * initialization) to pass the input features and return the output
   * predictions
   */
  MlModelStatus runModel(std::vector<float> input, std::vector<float> &output);

public:
  /**
   * D'tor is responsible for releasing any allocated resources for onnx
   * inference ideally all the logic is implemented in uninitialize()
   */
  virtual ~MlModelBase();

  /**
   * This is a pure virtual function to enforce inheriting classes to implement
   * it. It is implemented by inheriting classes because only they know their
   * specific file name.
   */
  virtual std::string getModelFileName() = 0;

  /**
   * This is a pure virtual function to enforce inheriting classes to implement
   * it. Inheriting model users should use this to print model version to their
   * log. Currently specific model (inheriting class) and the core onnx
   * inference (this base class) have a unique single version without a
   * separation between the evolution of the two.
   */
  std::string getVersion();

  /**
   * This function is responsible for loading the onnx file, reading the model
   * metadata and storing all objects relevant for inference
   */
  MlModelStatus initialize(const ORTCHAR_T *modelFilePath);

  /**
   * This function is responsible for loading the onnx file from the memory
   * provided, reading the model metadata and storing all objects relevant for
   * inference
   */
  MlModelStatus initialize(const void *modelData, size_t const modelDataLength);
};

} // namespace machine_learning_engine
