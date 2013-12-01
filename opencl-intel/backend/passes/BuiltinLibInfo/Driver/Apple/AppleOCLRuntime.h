/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __APPLE_OCL_RUNTIME_H_
#define __APPLE_OCL_RUNTIME_H_

#include "OpenclRuntime.h"

namespace intel {

/// @brief
///  Runtime services for Apple Opencl SDK
///  These are services for runtime-specific information. Such services include
///  detection of Thread-ID creation instructions, and Scalar/Vector
///  mapping of builtin
///  functions.
class AppleOpenclRuntime : public OpenclRuntime {
public:
  
  /// @brief Constructor
  AppleOpenclRuntime(const Module *runtimeModule);
  
  /// @brief Destructor
  virtual ~AppleOpenclRuntime();

  /// @brief Find a function in the runtime's built-in functions
  /// @param Name Function name to look for
  virtual Function *findInRuntimeModule(StringRef Name) const;

  /// @brief returns true the function needs to be replaced with fake function
  ///   used by OCLBuiltinPreVectorizationPass
  /// @param funcName Function name to check
  /// @return true if builin needs special case resolving
  virtual bool needPreVectorizationFakeFunction(const std::string &funcName) const;

  /// @brief returns true the function is writeImage which need scalarizing of 
  ///  input used by OCLBuiltinPreVectorizationPass
  /// @param funcName Function name to check
  virtual bool isWriteImage(const std::string &funcName) const;

  /// @brief returns true the function is fake writeImage which produced in
  /// Pre-Scalarization used AppleWiDepPrePacketizationPass - unique to apple
  /// @param funcName Function name to check
  virtual bool isFakeWriteImage(const std::string &funcName) const;

  /// @brief returns true iff this is name of transposed_read_image.
  /// @param funcName Function name to check
  virtual bool isTransposedReadImg(const std::string &func_name) const;

  /// @brief returns true iff this is name of transposed_write_image.
  /// @param funcName Function name to check
  virtual bool isTransposedWriteImg(const std::string &func_name) const;

  /// @brief returns the read stream function from the runtime module.
  /// @param isPointer64Bit true if pointer size is 64bit, false otherwise.
  virtual Function *getReadStream(bool isPointer64Bit) const;

  /// @brief returns the write stream function from the runtime module.
  /// @param isPointer64Bit true if pointer size is 64bit, false otherwise.
  virtual Function *getWriteStream(bool isPointer64Bit) const;

  // @brief return true if this name of stream built-in.
  virtual bool isStreamFunc(const std::string &funcName) const;

  /// @brief returns true iff whenever the there is vector argument to 
  ///        a vectorizeable scalar built-in it should be spread for 
  ///        the packertized version 
  ///        foo(<2 float> %a) --> foo4(<4 x float> %a.x, <4 xfloat> %a.y)
  virtual bool alwaysSpreadVectorParams() const {return true;};

private:

  /// @breif hold names of builtins that require replacement with fake function
  std::set<std::string> m_needPreVectorizationSet;
  
  AppleOpenclRuntime(); // Do not implement

  Module *m_innerRTModule;

  std::auto_ptr<VectorizerFunction> m_readImageEntry;

  std::auto_ptr<VectorizerFunction> m_writeImageEntry;
};



} // Namespace

#endif // __APPLE_OCL_RUNTIME_H_
