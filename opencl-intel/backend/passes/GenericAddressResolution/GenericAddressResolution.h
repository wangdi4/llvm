/*=================================================================================
Copyright (c) 2013, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __GENERIC_ADDRESS_RESOLUTION_H__
#define __GENERIC_ADDRESS_RESOLUTION_H__

#include <OCLAddressSpace.h>
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/Instruction.h>
#include <list>
#include <string>

#if defined(__APPLE__)
  #include <OpenCL/cl.h>
#else
  #include <CL/cl.h>
#endif

#define CLK_LOCAL_MEM_FENCE (CL_LOCAL)
#define CLK_GLOBAL_MEM_FENCE (CL_GLOBAL)

#define MD_GAS_COUNT    "opencl.compiler.2_0.gen_addr_space_pointer_counter"
#define MD_GAS_WARNINGS "opencl.compiler.2_0.gen_addr_space_pointer_warnings"


using namespace Intel::OpenCL::DeviceBackend::Utils;

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Passes { namespace GenericAddressSpace {
  using namespace llvm;

  /// @brief Function call classification
  typedef enum {
    CallBuiltIn = 0,
    CallIntrinsic,
    CallNonKernel
  } FuncCallType;

  typedef std::list<Function*> TFunctionList;

  /// @brief Checks if a function is one of Address Space Qualifier BIs
  /// @param pFunc - function to be checked 
  /// @returns true if the check is successful or false otherwise
  bool isAddressQualifierBI(const Function *pFunc);
 
  /// @brief Checks if a function is one of BIs accepting generic addr space pointer
  /// @param pFunc - function to be checked 
  /// @returns true if the check is successful or false otherwise
  bool isGenericAddrBI(const Function *pFunc);

  /// @brief Sorts all functions defined in the module in the call-graph order
  /// @param pModule        - module whose functions are to be sorted
  /// @param orderedList    - sorted list produced by the function
  /// @param isTopDownOrder - true for top-down order, or false for bottom-up order
  void sortFunctionsInCGOrder(Module *pModule, TFunctionList &orderedList, bool isTopDownOrder);

  /// @brief  Helper for transition of debug info from original instruction to new instruction
  /// @param  pNew - new instruction
  /// @param  pOld - original instruction
  void setDebugLocBy(Instruction *pNew, const Instruction *pOld);

  /// @brief  Helper for conversion of original mangled function name to that with
  /// @brief  resolved pointer address space types
  /// @param  origMangledName - original mangled name of the function
  /// @param  resolvedSpaces  - vector of resolved space types (one per parameter)
  ///         (for non-pointer or generic pointer - OCLAddressSpace::Generic)
  /// @returns mangled name modified upon resolved address space types 
  std::string getResolvedMangledName(std::string origMangledName,
                                     SmallVector<OCLAddressSpace::spaces, 8> resolvedSpaces);

  /// @brief  Helper for composition of unique mangled function name for a specialized
  /// @brief  non-kernel function (out from the function original name and its pointer arguments)
  /// @param  functionName - name of the function
  /// @param  argTypes     - vector of the function's argument types (one per argument)
  /// @returns mangled name built upon provided name and types 
  std::string getSpecializedFunctionName(std::string functionName,
                                         SmallVector<Type*, 8> argTypes);

  /// @brief  Helper for composition of overloaded intrinsic argument signature 
  /// @brief  out of actual parameters
  /// @param  pFunc      - intrinsic function
  /// @param  paramTypes - actual parameters
  /// @param  argTypes   - arguments for overloaded signature
  /// @returns mangled name built upon provided name and types 
  void getIntrinsicOverload(Function *pFunc, SmallVector<Type*, 8> paramTypes, 
                                             SmallVector<Type*, 8> &argTypes);

} // namespace GenericAddressSpace
} // namespace Passes
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif

