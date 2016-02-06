/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __RuntimeServices_H_
#define __RuntimeServices_H_

#include "VectorizerFunction.h"

#include "llvm/IR/Instructions.h"

using namespace llvm;

namespace intel {

/// @brief
///  Interface for runtime services.
///  These are services for runtime-specific information. Such services include
///  detection of Thread-ID creation instructions, and Scalar/Vector mapping of builtin
///  functions.
class RuntimeServices {
public:

  RuntimeServices() {}
  virtual ~RuntimeServices() {}

  /// @brief Find a function in the runtime's built-in functions
  /// @param Name Function name to look for
  virtual Function *findInRuntimeModule(StringRef Name) const = 0;

  /// @brief Search for a builtin function (used by scalarizer abd packetizer)
  /// @param inp_name Function name to look for
  virtual std::unique_ptr<VectorizerFunction>
  findBuiltinFunction(StringRef) const = 0;

  /// @brief Check if specified instruction is an ID generator
  /// @param inst The instruction
  /// @param err Returns TRUE, if unable to determine ID generation
  /// @param dim Dimention of TIDGenerator
  virtual bool isTIDGenerator(const Instruction *inst, bool *err, unsigned *dim) const = 0;

  /// @brief Check if function is a synchronization built-in
  /// @param inp_name Function name to look for
  virtual bool isSyncFunc(const std::string &func_name) const = 0;

  /// @brief Check if the shading language is ordered
  ///  OpenCL is ordered, DX is not because all items must
  ///  be randomly ordered.
  /// @return true if ordered.
  virtual bool orderedWI() const = 0;

  /// @brief Check the desired packetization width
  /// @return vector width for packetizing the function
  virtual unsigned getPacketizationWidth() const = 0;

  /// @brief Sets the desired packetization width
  /// @param width vector width for packetizing the function
  virtual void setPacketizationWidth(unsigned width) = 0;

  /// @brief returns true if the function has no side effects
  ///  this means it can be safely vectorized regardless if it is being masked
  /// @param func_name Function name to check
  /// @return true if function has no side effects
  virtual bool hasNoSideEffect(const std::string &func_name) const = 0;

  /// @brief returns true if func_name is a known expensive call.
  /// @param func_name name of the function.
  virtual bool isExpensiveCall(const std::string &func_name) const = 0;

  /// @brief returns true if the function is a masked version that support
  ///  i1 vector as first parameter
  /// @param func_name Function name to check
  /// @return true if function is masked version
  virtual bool isMaskedFunctionCall(const std::string &func_name) const = 0;

  /// @brief Check if function a 'faked function', (i.e., if the given name is
  //  has a definition in the builtin-runtime module, or is it just a
  //  synthesized name for internal usage.
  virtual bool isFakedFunction(StringRef) const = 0;

  /// @brief returns true iff whenever the there is vector argument to 
  ///        a vectorizeable scalar built-in it should be spread for 
  ///        the packertized version 
  ///        foo(<2 float> %a) --> foo4(<4 x float> %a.x, <4 xfloat> %a.y)
  virtual bool alwaysSpreadVectorParams() const = 0;

};

} // Namespace

#endif // __RuntimeServices_H_
