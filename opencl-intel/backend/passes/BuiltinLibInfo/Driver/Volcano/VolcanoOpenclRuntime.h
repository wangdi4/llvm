// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

#ifndef __VolcanoOpenclRuntime_H_
#define __VolcanoOpenclRuntime_H_

#include "OpenclRuntime.h"

namespace intel {

extern const char *volacanoScalarSelect[];

class VolcanoOpenclRuntime : public OpenclRuntime {
public:
  /// @brief Constructor
  VolcanoOpenclRuntime(ArrayRef<Module *> runtimeModuleList);

  /// @brief Destructor
  ~VolcanoOpenclRuntime() {}

  VolcanoOpenclRuntime(); // Do not implement

  /// @brief returns true the function needs to be replaced with fake function
  ///   used by OCLBuiltinPreVectorizationPass - currently disabled in Volcano
  /// @param funcName Function name to check
  virtual bool
  needPreVectorizationFakeFunction(const std::string &funcName) const override;

  /// @brief returns true the function is writeImage which need scalarizing of
  ///  input used by OCLBuiltinPreVectorizationPass - currently disabled in
  ///  Volcano
  /// @param funcName Function name to check
  virtual bool isWriteImage(const std::string &funcName) const override;

  /// @brief returns true the function is fake writeImage which produced in
  /// Pre-Scalarization used AppleWiDepPrePacketizationPass
  /// @param funcName Function name to check
  virtual bool isFakeWriteImage(const std::string &funcName) const override;

  /// @brief returns true iff this is name of transposed_read_image.
  /// @param funcName Function name to check
  virtual bool isTransposedReadImg(const std::string &func_name) const override;

  /// @brief returns true iff this is name of transposed_write_image.
  /// @param funcName Function name to check
  virtual bool
  isTransposedWriteImg(const std::string &func_name) const override;

  /// @brief returns the read stream function from the runtime module.
  /// @param isPointer64Bit true if pointer size is 64bit, false otherwise.
  virtual Function *getReadStream(bool isPointer64Bit) const override;

  /// @brief returns the write stream function from the runtime module.
  /// @param isPointer64Bit true if pointer size is 64bit, false otherwise.
  virtual Function *getWriteStream(bool isPointer64Bit) const override;

  // @brief return true if this name of stream built-in.
  virtual bool isStreamFunc(const std::string &funcName) const override;
};

} // namespace intel

#endif
