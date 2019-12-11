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

#ifndef __DXRuntime_H_
#define __DXRuntime_H_

#include "RuntimeServices.h"
#include "Functions.h"

namespace intel {

extern hashEntry DXEntryDB[];

/// @brief Runtime services for DX
class DXRuntime : public RuntimeServices {
public:

  /// @brief Constructor
  DXRuntime(SmallVector<Module*, 2> runtimeModuleList, unsigned packetWidth);

  /// @brief Destructor
  ~DXRuntime() {}

  /// @brief Find a function in the runtime's built-in functions
  /// @param Name Function name to look for
  virtual Function *findInRuntimeModule(StringRef Name) const;

  /// @brief Search for a builtin function (used by scalarizer abd packetizer)
  /// @param inp_name Function name to look for
  virtual std::auto_ptr<VectorizerFunction>
  findBuiltinFunction(StringRef inp_name) const;

  /// @brief DX is not ordered. WIAnalysis is not needed
  ///  since everything is assumed to be random.
  /// @return True
  virtual bool orderedWI() const;

  /// @brief Check if specified Instruction is an ID generator
  /// @param inst The instruction
  /// @param err Returns TRUE, if unable to determine ID generation
  /// @param dim Dimention of TIDGenerator
  virtual bool isTIDGenerator(const Instruction *inst, bool *err, unsigned* dim) const;

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

  /// @brief returns true if func_name is a known expensive call.
  /// @param func_name name of the function.
  virtual bool isExpensiveCall(const std::string &func_name) const;

  /// @brief returns true if the function is a masked version that support 
  ///  i1 vector as first parameter
  /// @param func_name Function name to check
  /// @return true if function is masked version
  virtual bool isMaskedFunctionCall(const std::string &func_name) const;

  /// @brief Check wether the function needs 'VPlan' style masking,
  ///  meaning it has i32 mask as the last argument.
  /// @param the function name that is a subject to check
  bool needsVPlanStyleMask(StringRef) const override;

  /// @brief returns true iff whenever there is a vector argument to 
  ///        a vectorizeable scalar built-in it should be spread for 
  ///        the packertized version 
  ///        foo(<2 x float> %a) --> foo4(<4 x float> %a.x, <4 x float> %a.y)
  virtual bool alwaysSpreadVectorParams() const {return false;}

  /// @brief returns true iff whenever there is a vector return
  ///        of a vectorizeable scalar built-in it should be concatenated
  ///        for the packetized version:
  ///        <2 x float> foo(...) --> <8 x float> foo4(...)
  bool needsConcatenatedVectorReturn(StringRef) const override { return false; }

  /// @brief returns true iff whenever there is a vector argument to
  ///        a vectorizeable scalar built-in it should be concatenated
  ///        for the packetized version:
  ///        foo(<2 x float> %a) --> foo4(<8 x float>)
  bool needsConcatenatedVectorParams(StringRef) const override { return false; }

  /// @brief returns true iff spec guarantees that all work items hit
  ///        a memory function, so predication is redundant
  bool allowsUnpredicatedMemoryAccess(StringRef) const override { return false; }

private:
  DXRuntime(); // Do not implement

  /// @brief Fills the OCL functions DB with DX buildins
  /// @param s VFH storage
  void initDB(hashEntry* entry);

  /// @brief Pointer to runtime modules list
  /// (module with implementation of built-in functions)
  SmallVector<Module*, 2> m_runtimeModulesList;

  /// @brief Hold the requested packetization width
  /// (currently same one for all funcs)
  unsigned m_packetizationWidth;

  /// @brief Pointer to OpenCL wrappers hash object
  VFH m_vfh;
};

} // Namespace

#endif // __DXRuntime_H_
