/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __OpenclRuntime_H_
#define __OpenclRuntime_H_

#include "RuntimeServices.h"
#include "Functions.h"
#include <map>
#include <string>
#include <set>



namespace intel {

extern VFH::hashEntry OCLEntryDB[];
extern const char *volacanoScalarSelect[];

/// @brief
///  Runtime services for Intel's Opencl SDK
///  These are services for runtime-specific information. Such services include
///  detection of Thread-ID creation instructions, and Scalar/Vector
///  mapping of builtin
///  functions.
/// @Author Sion Berkowits

class OpenclRuntime : public RuntimeServices {
public:

  /// @brief Constructor which get arbitraty table as input
  OpenclRuntime(const Module *runtimeModule,
                VFH::hashEntry *DB = OCLEntryDB,
                const char **scalarSelects = volacanoScalarSelect);

  /// @brief Destructor
  ~OpenclRuntime() {}

  /// @brief Find a function in the runtime's built-in functions
  /// @param Name Function name to look for
  virtual Function *findInRuntimeModule(StringRef Name) const;

  /// @brief Search for a builtin function (used by scalarizer abd packetizer)
  /// @param inp_name Function name to look for
  virtual funcEntry findBuiltinFunction(std::string &inp_name) const;

  /// @brief OpenCL is an ordered programming language. WIAnalysis is needed
  /// @return True
  virtual bool orderedWI() const;

  /// @brief Check if specified Instruction is an ID generator
  /// @param inst The instruction
  /// @param err Returns TRUE, if unable to determine ID generation
  /// @param dim Dimention of TIDGenerator
  virtual bool isTIDGenerator(const Instruction *inst, bool *err, unsigned *dim) const;

  /// @brief Check the desired packetization width
  /// @return vector width for packetizing the function
  virtual unsigned getPacketizationWidth() const;

  /// @brief Sets the desired packetization width  
  /// @param width vector width for packetizing the function
  virtual void setPacketizationWidth(unsigned width);

  /// @brief Check if function is a synchronization built-in
  /// @param inp_name Function name to look for
  virtual bool isSyncFunc(const std::string &func_name) const;

  /// @brief Check if function is a known uniform function such as get_group_size
  /// @param inp_name Function name to look for
  virtual bool isKnownUniformFunc(std::string &func_name) const;

  /// @brief returns true if the function has no side effects
  ///  this means it can be safely vectorized regardless if it is being masked
  /// @param func_name Function name to check
  /// @return true if function has no side effects
  virtual bool hasNoSideEffect(std::string &func_name) const;

  /// @brief returns true the is a special function that needs resolving used
  ///  by OCLSpecialCaseResolver
  /// @param funcName Function name to check
  virtual bool needSpecialCaseResolving(std::string &funcName) const;

  /// @brief returns true the function needs to be replaced with fake function
  ///   used by OCLBuiltinPreVectorizationPass - currently disabled in Volcano   
  /// @param funcName Function name to check
  virtual bool needPreVectorizationFakeFunction(std::string &funcName) const;

  /// @brief returns true the function is a scalar select builtin used by
  ///  OCLBuiltinPreVectorizationPass - currently disabled in Volcano
  /// @param funcName Function name to check
  virtual bool isScalarSelect(std::string &funcName) const;

  /// @brief returns true the function is writeImage which need scalarizing of
  ///  input used by OCLBuiltinPreVectorizationPass - currently disabled in
  ///  Volcano
  /// @param funcName Function name to check
  virtual bool isWriteImage(std::string &funcName) const;

  /// @brief returns true the function is dot product that should be inlined
  ///  used OCLBuiltinPreVectorizationPass - works only for Volcano
  /// @param funcName Function name to check
  /// @return the width of the input parameters incase of dot, 0 otherwise
  virtual unsigned isInlineDot(std::string &funcName) const; 

  /// @brief returns true if the function is a masked version that support 
  ///  i1 vector as first parameter
  /// @param func_name Function name to check
  /// @return true if function is masked version
  virtual bool isMaskedFunctionCall(std::string &func_name) const;

protected:
  
  OpenclRuntime(); // Do not implement

  /// @brief initiate the scalar selects set from array of names
  virtual void initScalarSelectSet(const char **scalarSelects);

  /// @brief hold names of scalar select builtins
  std::set<std::string> m_scalarSelectSet;

  /// @brief Fills the OCL functions DB with DX buildins
  /// @param s VFH storage
  void initDB(VFH &s);

  /// @brief Pointer to runtime module
  /// (module with implementation of built-in functions)
  const Module *m_runtimeModule;

  /// @brief Hold the requested packetization width
  //(currently same one for all funcs)
  unsigned m_packetizationWidth;

  /// @brief Pointer to OpenCL wrappers hash object
  VFH m_vfh;

private:

  /// @brief holds mapping bettwen dot builtin name and operand width
  std::map<std::string, unsigned> m_dotOpWidth;

  /// @brief initate the dot map 
  void initDotMap();
};

/// @brief OpenCL-specific function names
#define GET_GID_NAME  "get_global_id"
#define GET_LID_NAME  "get_local_id"
#define GET_LOCAL_SIZE "get_local_size"
#define BARRIER_FUNC_NAME    "barrier"
#define WG_FUNCS_NAME_PREFIX  "__async"

} // Namespace

#endif // __OpenclRuntime_H_
