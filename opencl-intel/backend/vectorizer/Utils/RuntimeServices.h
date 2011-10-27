/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __RuntimeServices_H_
#define __RuntimeServices_H_

#include "llvm/Module.h"
#include "llvm/Instructions.h"

#define HASH_BUILTIN_MISSING "_"

using namespace llvm;

namespace intel {

/// @brief
///  Interface for runtime services.
///  These are services for runtime-specific information. Such services include
///  detection of Thread-ID creation instructions, and Scalar/Vector mapping of builtin
///  functions.
/// @Author Sion Berkowits
class RuntimeServices {
public:

  RuntimeServices() {}
  virtual ~RuntimeServices() {}

  /// @brief Set the singleton runtime services object
  static void set(RuntimeServices *obj);

  /// @brief Get pointer to the singleton runtime services object
  static RuntimeServices *get();

  /// @brief Find a function in the runtime's built-in functions
  /// @param Name Function name to look for
  virtual Function *findInRuntimeModule(StringRef Name) const = 0;

  /// @brief Structure returned for each function name query
  typedef struct hashEntry {
    const char *funcs[6]; // Name of functions, sorted as (1,2,4,8,16,3)
    unsigned isScalarizable;
    unsigned isPacketizable;
  } hashEntry;

  typedef std::pair<const hashEntry*, unsigned> funcEntry;

  /// @brief Search for a builtin function (used by scalarizer abd packetizer)
  /// @param inp_name Function name to look for
  virtual funcEntry findBuiltinFunction(std::string &inp_name) const = 0;

  /// @brief Check if specified instruction is an ID generator
  /// @param inst The instruction
  /// @param err Returns TRUE, if unable to determine ID generation
  /// @param dim Dimention of TIDGenerator
  virtual bool isTIDGenerator(const Instruction *inst, bool *err, unsigned *dim) const = 0;

  /// @brief Check if function is a synchronization built-in
  /// @param inp_name Function name to look for
  virtual bool isSyncFunc(const std::string &func_name) const = 0;

  /// @brief Check if function is a known uniform function such as get_group_size
  /// @param inp_name Function name to look for
  virtual bool isKnownUniformFunc(std::string &func_name) const = 0;

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
  virtual bool hasNoSideEffect(std::string &func_name) const = 0;

  /// @brief returns true if the function is a masked version that support
  ///  i1 vector as first parameter
  /// @param func_name Function name to check
  /// @return true if function is masked version
  virtual bool isMaskedFunctionCall(std::string &func_name) const = 0;


private:
  static RuntimeServices *m_singleton;
};

} // Namespace

#endif // __RuntimeServices_H_
