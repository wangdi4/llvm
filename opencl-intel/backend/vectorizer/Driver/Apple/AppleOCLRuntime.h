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
  virtual bool needPreVectorizationFakeFunction(const std::string &funcName) const;

  /// @brief returns true the function is writeImage which need scalarizing of 
  ///  input used by OCLBuiltinPreVectorizationPass
  /// @param funcName Function name to check
  virtual bool isWriteImage(const std::string &funcName) const;

  /// @brief returns true the function is fake writeImage which produced in
  /// Pre-Scalarization used AppleWiDepPrePacketizationPass - unique to apple
  /// @param funcName Function name to check
  virtual bool isFakeWriteImage(const std::string &funcName) const;

  /// @brief returns true the function is dot product that should be inlined
  ///  used OCLBuiltinPreVectorizationPass - disabled for Apple
  /// @param funcName Function name to check
  /// @return the width of the input parameters incase of dot, 0 otherwise
  virtual unsigned isInlineDot(const std::string &funcName) const;

  /// @brief retruns number of Jit dimensions.
  virtual unsigned getNumJitDimensions() const;

  // @brief retrun name of builtin to retrieve base global id for this work group.
  virtual const char *getBaseGIDName() const;

  /// @brief returns true iff this is name of transposed_read_image.
  bool isTransposedReadImg(const std::string &func_name) const;

  /// @brief returns true iff this is name of transposed_write_image.
  bool isTransposedWriteImg(const std::string &func_name) const;

  /// @brief returns the read stream function from the runtime module.
  Function *getReadStream() const;

  /// @brief returns the write stream function from the runtime module.
  Function *getWriteStream() const;

  /// @brief returns true iff whenever the there is vector argument to 
  ///        a vectorizeable scalar built-in it should be spread for 
  ///        the packertized version 
  ///        foo(<2 float> %a) --> foo4(<4 x float> %a.x, <4 xfloat> %a.y)
  virtual bool alwaysSpreadVectorParams() const {return false;};

private:

  /// @breif hold names of builtins that require replacement with fake function
  std::set<std::string> m_needPreVectorizationSet;

  
  AppleOpenclRuntime(); // Do not implement

  const hashEntry *m_readImageEntry;

  const hashEntry *m_writeImageEntry;
};



} // Namespace

#endif // __APPLE_OCL_RUNTIME_H_
