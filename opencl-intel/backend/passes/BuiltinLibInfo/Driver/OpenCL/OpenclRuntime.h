// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __OpenclRuntime_H_
#define __OpenclRuntime_H_

#include "RuntimeServices.h"
#include <map>
#include <string>
#include <set>

namespace intel {

/// @brief
///  Runtime services for Intel's Opencl SDK
///  These are services for runtime-specific information. Such services include
///  detection of Thread-ID creation instructions, and Scalar/Vector
///  mapping of builtin
///  functions.

class OpenclRuntime : public RuntimeServices {
public:

  /// @brief Constructor which get arbitraty table as input
  OpenclRuntime(SmallVector<Module*, 2> runtimeModuleList,
                const char **scalarSelects);

  /// @brief Destructor
  virtual ~OpenclRuntime() {}

  /// @brief Find a function in the runtime's built-in functions
  /// @param Name Function name to look for
  virtual Function *findInRuntimeModule(StringRef Name) const;

  /// @brief Search for a builtin function (used by scalarizer abd packetizer)
  /// @param inp_name Function name to look for
  virtual std::auto_ptr<VectorizerFunction>
  findBuiltinFunction(StringRef inp_name) const;

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

  /// @brief returns true the function needs to be replaced with fake function
  ///   used by OCLBuiltinPreVectorizationPass - currently disabled in Volcano   
  /// @param funcName Function name to check
  virtual bool needPreVectorizationFakeFunction(const std::string &funcName) const = 0;

  /// @brief returns true the function is a scalar select builtin used by
  ///  OCLBuiltinPreVectorizationPass - currently disabled in Volcano
  /// @param funcName Function name to check
  virtual bool isScalarSelect(const std::string &funcName) const;

  /// @brief returns true the function is writeImage which need scalarizing of
  ///  input used by OCLBuiltinPreVectorizationPass - currently disabled in
  ///  Volcano
  /// @param funcName Function name to check
  virtual bool isWriteImage(const std::string &funcName) const = 0;

  /// @brief returns true the function is fake writeImage which produced in
  /// Pre-Scalarization used AppleWiDepPrePacketizationPass
  /// @param funcName Function name to check
  virtual bool isFakeWriteImage(const std::string &funcName) const = 0;

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

  /// @brief Check wether the function needs 'VPlan' style masking,
  ///  meaning it has i32 mask as the last argument.
  /// @param the function name that is a subject to check
  bool needsVPlanStyleMask(StringRef) const override;

  virtual bool isReturnByPtrBuiltin(const std::string &func_name) const;

  virtual unsigned getNumJitDimensions() const;

  /// @brief retrun name of builtin to retrieve base global id for this work group.
  virtual const char *getBaseGIDName() const;

  /// @brief returns true if this name of atomic built-in.
  bool isAtomicBuiltin(const std::string &func_name) const;

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

  /// @brief returns true iff this is name of transposed_read_image.
  /// @param funcName Function name to check
  virtual bool isTransposedReadImg(const std::string &func_name) const = 0;

  /// @brief returns true iff this is name of transposed_write_image.
  /// @param funcName Function name to check
  virtual bool isTransposedWriteImg(const std::string &func_name) const = 0;

  /// @brief returns the read stream function from the runtime module.
  /// @param isPointer64Bit true if pointer size is 64bit, false otherwise.
  virtual Function *getReadStream(bool isPointer64Bit) const = 0;

  /// @brief returns the write stream function from the runtime module.
  /// @param isPointer64Bit true if pointer size is 64bit, false otherwise.
  virtual Function *getWriteStream(bool isPointer64Bit) const = 0;

  // @brief return true if this name of stream built-in.
  virtual bool isStreamFunc(const std::string &funcName) const = 0;
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

  /// @brief returns true iff whenever there is a vector argument to 
  ///        a vectorizeable scalar built-in it should be spread for 
  ///        the packertized version 
  ///        foo(<2 x float> %a) --> foo4(<4 x float> %a.x, <4 x float> %a.y)
  virtual bool alwaysSpreadVectorParams() const {return true;}

  /// @brief returns true iff whenever there is a vector return
  ///        of a vectorizeable scalar built-in it should be concatenated
  ///        for the packetized version:
  ///        <2 x float> foo(...) --> <8 x float> foo4(...)
  bool needsConcatenatedVectorReturn(StringRef) const override;

  /// @brief returns true iff whenever there is a vector argument to
  ///        a vectorizeable scalar built-in it should be concatenated
  ///        for the packetized version:
  ///        foo(<2 x float> %a) --> foo4(<8 x float>)
  bool needsConcatenatedVectorParams(StringRef) const override;

  OpenclRuntime(); // Do not implement

  /// @brief initiate the scalar selects set from array of names
  virtual void initScalarSelectSet(const char **scalarSelects);

  /// @brief hold names of scalar select builtins
  std::set<std::string> m_scalarSelectSet;

  /// @brief Pointer to runtime modules list
  /// (module with implementation of built-in functions)
  SmallVector<Module*, 2> m_runtimeModulesList;

  /// @brief Hold the requested packetization width
  /// (currently same one for all funcs)
  unsigned m_packetizationWidth;

private:
  
  
  /// @brief holds mapping bettwen dot builtin name and operand width
  std::map<std::string, unsigned> m_dotOpWidth;
  
  /// @brief initate the dot map
  void initDotMap();
};

} // Namespace

#endif // __OpenclRuntime_H_
