/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __GENERIC_ADDRESS_RESOLUTION_H__
#define __GENERIC_ADDRESS_RESOLUTION_H__

#include <OCLAddressSpace.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <list>
#include <string>

#include <CL/cl.h>

#define CLK_LOCAL_MEM_FENCE (CL_LOCAL)
#define CLK_GLOBAL_MEM_FENCE (CL_GLOBAL)

using namespace Intel::OpenCL::DeviceBackend::Utils;

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Passes { namespace GenericAddressSpace {
  using namespace llvm;

  /// @brief  Function call classification
  typedef enum {
    CallBuiltIn = 0,
    CallIntrinsic,
    CallNonKernel
  } FuncCallType;

  typedef std::list<Function*> TFunctionList;

  /// @brief  Checks if a function is one of Address Space Qualifier BIs
  /// @param  pFunc - function to be checked
  /// @returns  true if the check is successful or false otherwise
  bool isAddressQualifierBI(const Function *pFunc);

  /// @brief  Checks if a function is one of BIs accepting generic addr space pointer
  /// @param  pFunc - function to be checked
  /// @returns  true if the check is successful or false otherwise
  bool isGenericAddrBI(const Function *pFunc);

  /// @brief  Sorts all functions defined in the module in the call-graph order
  /// @param  pModule        - module whose functions are to be sorted
  /// @param  orderedList    - sorted list produced by the function
  /// @param  isTopDownOrder - true for top-down order, or false for bottom-up order
  void sortFunctionsInCGOrder(Module *pModule, TFunctionList &orderedList,
                              bool isTopDownOrder);

  /// @brief  Helper for transition of debug info from original instruction to
  /// new instruction
  /// @param  pNew - new instruction
  /// @param  pOld - original instruction
  void assocDebugLocWith(Instruction *pNew, const Instruction *pOld);

  /// @brief  Helper for conversion of original mangled function name to that with
  /// @brief  resolved pointer address space types
  /// @param  origMangledName - original mangled name of the function
  /// @param  resolvedSpaces  - vector of resolved space types (one per parameter)
  ///         (for non-pointer or generic pointer - OCLAddressSpace::Generic)
  /// @returns  mangled name modified upon resolved address space types
  std::string getResolvedMangledName(
      std::string origMangledName,
      const SmallVector<OCLAddressSpace::spaces, 8> &resolvedSpaces,
      const SmallVector<OCLAddressSpace::spaces, 8> *originalSpaces = 0);

  /// @brief  Helper for composition of unique mangled function name for a specialized
  /// @brief  non-kernel function (out from the function original name and its pointer arguments)
  /// @param  functionName - name of the function
  /// @param  argTypes     - vector of the function's argument types (one per argument)
  /// @returns  mangled name built upon provided name and types
  std::string getSpecializedFunctionName(std::string functionName,
                                         const SmallVector<Type*, 8> &argTypes);

  /// @brief  Helper for composition of overloaded intrinsic argument signature
  /// @brief  out of the intrinsic function arguments
  /// @param  pFunc                - intrinsic function
  /// @param  argTypes             - list of argument types of the function
  /// @param  overloadableArgTypes - list of argument types for overloaded signature
  /// @returns list of argument types for overloaded signature (in overloadableArgTypes)
  void getIntrinsicOverload(Function *pFunc,
                            const SmallVector<Type *, 8> &argTypes,
                            SmallVector<Type *, 8> &overloadableArgTypes);

  /// @brief  Helper for printing-out of a warning and appending corresponding
  /// @brief  line number to metadata
  /// @param  warning  - warning to print
  /// @param  pInstr   - instruction of the warning, or NULL
  /// @param  pModule  - module whose metadata should be created/appended
  /// @param  pContext - current context
  void emitWarning(std::string warning, Instruction *pInstr, Module *pModule,
                   LLVMContext *pContext);

  /// @brief  Helper for check of pointer array case
  /// @param  pPtrType - pointer to check
  /// @returns  true if this is a pointer to scalar, or false otherwise
  bool isSinglePtr(const Type *pPtrType);

  /// @brief  Helper for generation of CPU-specific code/constant instead of
  /// @brief  call to Address Space Qualifier BI function
  /// @param  pCallInstr - original call to Address Space Qualifier BI function
  /// @param  pSrcPtr    - input pointer to the call (already resolved if needed)
  /// @returns value of generated code/constant
  Value *getFoldedAddrSpaceCall(CallInst *pCallInstr, Value *pSrcPtr);


} // namespace GenericAddressSpace
} // namespace Passes
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif

