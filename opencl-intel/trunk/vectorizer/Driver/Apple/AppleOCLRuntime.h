/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __APPLE_OCL_RUNTIME_H_
#define __APPLE_OCL_RUNTIME_H_

#include "OpenclRuntime.h"

namespace intel {

extern VFH::hashEntry AppleOCLEntryDB[];

/// @brief
///  Runtime services for Apple Opencl SDK
///  These are services for runtime-specific information. Such services include
///  detection of Thread-ID creation instructions, and Scalar/Vector
///  mapping of builtin
///  functions.
/// @Author Ran Chachick
class AppleOpenclRuntime : public OpenclRuntime {
public:
  
  /// @brief Constructor
  AppleOpenclRuntime(const Module *runtimeModule);
  
  /// @brief Destructor
  ~AppleOpenclRuntime() {}

  /// @brief returns true the function needs to be replaced with fake function
  ///   used by OCLBuiltinPreVectorizationPass
  /// @param funcName Function name to check
  /// @return true if builin needs special case resolving
  virtual bool needPreVectorizationFakeFunction(std::string &funcName) const;

  /// @brief returns true the function is writeImage which need scalarizing of 
  ///  input used by OCLBuiltinPreVectorizationPass
  /// @param funcName Function name to check
  virtual bool isWriteImage(std::string &funcName) const;

  /// @brief returns true the function is fake writeImage which produced in
  /// Pre-Scalarization used AppleWiDepPrePacketizationPass - unique to apple
  /// @param funcName Function name to check
  virtual bool isFakeWriteImage(std::string &funcName) const;

  /// @brief returns true the function is dot product that should be inlined
  ///  used OCLBuiltinPreVectorizationPass - disabled for Apple
  /// @param funcName Function name to check
  /// @return the width of the input parameters incase of dot, 0 otherwise
  virtual unsigned isInlineDot(std::string &funcName) const;

  /// @brief returns true if the function has no side effects
  ///  this means it can be safely vectorized regardless if it is being masked 
  /// @param func_name Function name to check
  /// @return true if function has no side effects
  virtual bool hasNoSideEffect(std::string &func_name) const;


private:

  /// @breif hold names of builtins that require replacement with fake function
  std::set<std::string> m_needPreVectorizationSet;

  
  AppleOpenclRuntime(); // Do not implement
};



} // Namespace

#endif // __APPLE_OCL_RUNTIME_H_
