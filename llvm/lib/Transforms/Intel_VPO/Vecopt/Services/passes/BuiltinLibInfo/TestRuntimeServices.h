/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __TestRuntime_H_
#define __TestRuntime_H_

#include "RuntimeServices.h"
#include <map>
#include <string>
#include <set>

namespace intel {

/// @brief
///  Test runtime services for XMainVectorizer
///  These are services for runtime-specific information. Such services include
///  detection of Thread-ID creation instructions, and Scalar/Vector
///  mapping of builtin
///  functions.

class TestRuntime : public RuntimeServices {
public:

  /// @brief Constructor which get arbitraty table as input
  TestRuntime(const Module *runtimeModule);

  /// @brief Destructor
  virtual ~TestRuntime() {}

  /// @brief Find a function in the runtime's built-in functions
  /// @param Name Function name to look for
  virtual Function *findInRuntimeModule(StringRef Name) const;

  /// @brief Search for a builtin function (used by scalarizer abd packetizer)
  /// @param inp_name Function name to look for
  virtual std::unique_ptr<VectorizerFunction>
  findBuiltinFunction(StringRef inp_name) const;

  /// @brief WIAnalysis is needed
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

  /// @brief Checks if function is a 'faked function', (i.e., if the given name is
  //  has a definition in the builtin-runtime module, or is it just a
  //  synthesized name for internal usage.
  virtual bool isFakedFunction(StringRef fname) const;

  /// @brief returns true if the function has no side effects
  ///  this means it can be safely vectorized regardless if it is being masked
  /// @param func_name Function name to check
  /// @return true if function has no side effects
  virtual bool hasNoSideEffect(const std::string &func_name) const;

  /// @brief returns true the is a special function that needs resolving used
  ///  by OCLSpecialCaseResolver
  /// @param funcName Function name to check
  virtual bool needSpecialCaseResolving(const std::string &funcName) const;

  /// @brief returns true the function is a scalar select builtin used by
  ///  OCLBuiltinPreVectorizationPass - currently disabled in Volcano
  /// @param funcName Function name to check
  virtual bool isScalarSelect(const std::string &funcName) const;

  /// @brief returns true the function is dot product that should be inlined
  ///  used OCLBuiltinPreVectorizationPass - works only for Volcano
  /// @param funcName Function name to check
  /// @return the width of the input parameters incase of dot, 0 otherwise
  virtual unsigned isInlineDot(const std::string &funcName) const;

  /// @brief returns true if the function is a masked version that support 
  ///  i1 vector as first parameter
  /// @param func_name Function name to check
  /// @return true if function is masked version
  virtual bool isMaskedFunctionCall(const std::string &func_name) const;

  virtual bool isReturnByPtrBuiltin(const std::string &func_name) const;

  virtual unsigned getNumJitDimensions() const;

  /// @brief returns true if this name of atomic built-in.
  bool isAtomicBuiltin(const std::string &func_name) const;

  /// @brief returns true if this name of pipe built-in.
  bool isWorkItemPipeBuiltin(const std::string &func_name) const;

  /// @brief checks if funcName is mangled scalar min\max name.
  /// @param funcName input mangled name.
  /// @param isMin will be true if it is min builtin.
  /// @param isSigned will be true if it is signd min\max builtin.
  /// @returns true iff funcName is scalar min or max builtin.
  virtual bool isScalarMinMaxBuiltin(StringRef funcName, bool &isMin,
                                     bool &isSigned) const;

  /// @brief returns true if func_name is safe to speculative execute, and hence 
  ///        can be hoisted even if it is under control flow
  /// @param func_name name of the function.
  virtual bool isSafeToSpeculativeExecute(const std::string &func_name) const;

protected:

  /// @brief returns true if func_name is synchronization function with side effects.
  /// @param func_name name of the function.
  virtual bool isSyncWithSideEffect(const std::string &func_name) const;

  /// @brief returns true if func_name is synchronization function with no side effects.
  /// @param func_name name of the function.
  virtual bool isSyncWithNoSideEffect(const std::string &func_name) const;

  /// @brief returns true if func_name is a descriptor of image built-in.
  /// @param func_name name of the function.
  virtual bool isImageDescBuiltin(const std::string &func_name) const;

  /// @brief returns true if func_name is a known expensive call.
  /// @param func_name name of the function.
  virtual bool isExpensiveCall(const std::string &func_name) const;

  /// @brief returns true if func_name is a work item built-in.
  /// @param func_name name of the function.
  virtual bool isWorkItemBuiltin(const std::string &func_name) const;

  /// @brief returns true if func_name is a safe llvm initrinsic.
  /// @param func_name name of the function.
  virtual bool isSafeLLVMIntrinsic(const std::string &func_name) const;

  /// @brief returns true iff whenever the there is vector argument to 
  ///        a vectorizeable scalar built-in it should be spread for 
  ///        the packertized version 
  ///        foo(<2 x float> %a) --> foo4(<4 x float> %a.x, <4 x float> %a.y)
  virtual bool alwaysSpreadVectorParams() const {return true;}

  TestRuntime(); // Do not implement

  /// @brief Pointer to runtime module
  /// (module with implementation of built-in functions)
  const Module *m_runtimeModule;

  /// @brief Hold the requested packetization width
  //(currently same one for all funcs)
  unsigned m_packetizationWidth;

private:
  
  
  /// @brief holds mapping bettwen dot builtin name and operand width
  std::map<std::string, unsigned> m_dotOpWidth;
  
  /// @brief initate the dot map
  void initDotMap();
};

} // Namespace

#endif // __TestRuntime_H_
