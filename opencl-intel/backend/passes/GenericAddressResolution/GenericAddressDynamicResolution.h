/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __GENERIC_ADDRESS_DYNAMIC_RESOLUTION_H__
#define __GENERIC_ADDRESS_DYNAMIC_RESOLUTION_H__

#include "GenericAddressResolution.h"
#include <llvm/Pass.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/User.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Utils/PromoteMemToReg.h>
#include <llvm/ADT/SmallVector.h>
#include <map>
#include <vector>
#include <string>
#include <list>


using namespace Intel::OpenCL::DeviceBackend::Utils;
using namespace Intel::OpenCL::DeviceBackend::Passes;

extern "C" {
  /// Returns an instance of the GenericAddressDynamicResolution pass,
  /// which will be added to a PassManager and run on a Module.
  llvm::ModulePass *createGenericAddressDynamicResolutionPass();
}

namespace intel {

  using namespace llvm;

  /// @brief  GenericAddressDynamicResolution class resolves generic address space 
  /// @brief  pointers to a named address space (local, global or private) towards
  /// @brief  the following objectives: 
  /// @brief    1) Calculation of Address Space Qualifier BI functions' result
  /// @brief    2) Substitution of BI function call with GAS pointer parameters with that
  /// @brief       of 'named' pointer parameters
  /// @brief    3) Substitution of intrinsic function call with GAS pointer parameters 
  /// @brief       with that of 'named' parameters
  /// @brief  All code modifications are done for unified memory case only
  class GenericAddressDynamicResolution : public ModulePass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief  Constructor
    GenericAddressDynamicResolution();

    /// @brief  Destructor
    ~GenericAddressDynamicResolution();

    /// @brief  Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "GenericAddressDynamicResolution";
    }

    /// @brief  LLVM Module pass entry
    /// @param  M Module to transform
    /// @returns  true if changed
    bool runOnModule(Module &M);

    /// @brief  LLVM Interface
    /// @param  AU Analysis
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      // Performance-wise, we could reduce overhead of run-time
      // identification if we had 'FunctionInliningPass' as a
      // pre-requisite. However, that would break Debug mode
      // compilation of a LLVM module. Another pre-requisite
      // is mem2reg pass, however it will surely run as a part
      // of standard LLVM function optimization passes.
    }

  private:

    /// @brief  The llvm module this pass needs to update
    Module      *m_pModule;

    /// @brief  Function list in callee-to-caller (i.e., reversed call-graph) order
    GenericAddressSpace::TFunctionList m_functionsToHandle;

    /// @brief   Iteration through BI and Intrinsic calls with GAS parameters.
    /// @brief   Associates a function call /first element/ with named address 
    /// @brief   space type suggested for resolution /second element/.
    typedef std::pair<CallInst*, OCLAddressSpace::spaces> TBiIntrinPair;
    typedef SmallVector<TBiIntrinPair, 16>                TBiIntrinVector;

    /// @brief  Function call points info
    /// @brief   1. BI calls with GAS pointer parameters (for one function only!)
    TBiIntrinVector m_BiPoints;
    /// @brief   2. Intrinsic calls with GAS pointer parameters (for one function only!)
    TBiIntrinVector m_IntrinPoints;

    /// @brief  Calls to Address Qualifier BI
    SmallVector<CallInst*, 16>   m_AddrQualifierBICalls;

  private:

    /// @brief  Prepares collection of GAS pointers
    /// @brief  and identifies their usage points
    /// @param  pFunc - function to traverse
    void analyzeGASPointers(Function *pFunc);

    /// @brief  Resolves usages of GAS pointers to 'named' address space
    /// @param  pFunc - function to resolve
    /// @returns  true if any call was resolved, or false otherwise
    bool resolveGASUsages(Function *pFunc);

    //  Helpers for GAS pointers' collection processing
    // -------------------------------------------------------

    /// @brief  Helper for analysis of function call which 
    /// @brief  doesn't need to find a definition point
    /// @param  pCallInstr - instruction to analyze
    /// @param  category   - function category: BI or intrinsic
    void analyzeBIorIntrinsicCall(CallInst *pCallInstr, GenericAddressSpace::FuncCallType category);

    // Generic-to-named address space resolvers for different instructions using GAS pointers
    // --------------------------------------------------------------------------------------

    /// @brief  GAS pointer resolvers for call instructions
    /// @param  pInstr      - instruction to resolve
    /// @param  category    - function category: BI or intrinsic
    /// @param  targetSpace - target named space
    void resolveAddrSpaceQualifierBICall(CallInst *pCallInstr);
    void resolveBIorIntrinsicCall(CallInst *pCallInstr, GenericAddressSpace::FuncCallType category, OCLAddressSpace::spaces targetSpace);

  };

} // namespace intel

#endif
