/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __GENERIC_ADDRESS_STATIC_RESOLUTION_H__
#define __GENERIC_ADDRESS_STATIC_RESOLUTION_H__

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


using namespace Intel::OpenCL::DeviceBackend::Utils;
using namespace Intel::OpenCL::DeviceBackend::Passes;

extern "C" {
  /// Returns an instance of the GenericAddressDynamicResolution pass,
  /// which will be added to a PassManager and run on a Module.
  llvm::ModulePass *createGenericAddressStaticResolutionPass();
}

namespace intel {

  using namespace llvm;

  /// @brief  GenericAddressStaticResolution class resolves generic address space
  /// @brief  pointers to a named address space: local, global or private.
  class GenericAddressStaticResolution : public ModulePass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief  Constructor
    GenericAddressStaticResolution();

    /// @brief  Destructor
    ~GenericAddressStaticResolution();

    /// @brief  Provides name of pass
    virtual StringRef getPassName() const {
      return "GenericAddressStaticResolution";
    }

    /// @brief  LLVM Module pass entry
    /// @param  M Module to transform
    /// @returns  true if changed
    bool runOnModule(Module &M);

    /// @brief  LLVM Interface
    /// @param  AU Analysis
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      // We could declare 'PromotePass' here (as mem2reg pass is a
      // mandatory pre-requisite), however it is not exposed by LLVM,
      // hence we suppose that Optimizer flow will invoke its factory
      // (createPromoteMemoryToRegisterPass) prior to 'GenericAddressStaticResolution'
      // pass. TODO: to be re-evaluated with future LLVM releases
    }

  private:

    /// @brief  The llvm module this pass needs to update
    Module      *m_pModule;

    /// @brief  The llvm context
    LLVMContext *m_pLLVMContext;

    /// @brief  GAS Pointer Collection Data Model for the following accesses:
    /// @brief    1. Iteration through GAS pointers' instructions in a function
    typedef std::list<Instruction*>                          TPointerList;
    /// @brief    2. Quick access to a GAS pointer instruction info
    /// @brief       (mapping from instruction to its estimated named space pointer result type)
    typedef std::pair<Instruction*, OCLAddressSpace::spaces> TPointerInfo;
    typedef std::map<Instruction*, OCLAddressSpace::spaces>  TPointerMap;

    /// @brief  Collection of function and pointer info
    /// @brief  1. Function list in call-graph order
    GenericAddressSpace::TFunctionList m_functionsToHandle;
    /// @brief  2. GAS pointers' instructions in data flow order (for one function only!)
    TPointerList                       m_GASPointers;
    /// @brief  3. GAS pointers' instructions' info for quick access (for one function only!)
    TPointerMap                        m_GASEstimate;

    /// @brief  Instruction Replacement Info Data Model for the following accesses:
    /// @brief  (mapping from old instruction /first element/ to new value /second element/)
    typedef std::pair<Instruction*, Value*> TMapPair;
    /// @brief  1. Fast access to replacement info (old-to-new)
    typedef std::map<Instruction*, Value*>  TReplaceMap;
    /// @brief  2. Deterministic iteration through replacement info
    typedef std::vector<TMapPair>           TReplaceVector;

    /// @brief  <old-instruction>-to-<new-value> info
    /// @brief  1. Search for mapping
    TReplaceMap    m_replaceMap;
    /// @brief  2. Iterate through mappings
    TReplaceVector m_replaceVector;

    /// @brief  counter of pointers which the pass was unable to resolve
    unsigned m_failCount;

  private:

    /// @brief  Prepares collection of GAS pointers' usages from a function and
    /// @brief  estimates their named address space
    /// @param  curFuncIt - iterator to function to traverse
    void analyzeGASPointers(GenericAddressSpace::TFunctionList::const_iterator curFuncIt);

    /// @brief  Resolves GAS pointers to named address space
    /// @param  curFuncIt - iterator to function to resolve
    /// @returns  true if any pointer was statically resolved, or false otherwise
    bool resolveGASPointers(GenericAddressSpace::TFunctionList::iterator curFuncIt);

    //  Helpers for GAS pointers' collection processing
    // -------------------------------------------------

    /// @brief  Helper for adding of generic pointer instruction
    /// @brief  to the collection (together with its use tree)
    /// @param  pInstr - instruction to add
    /// @param  space  - target named space
    void addGASInstr(Instruction *pInstr, OCLAddressSpace::spaces space);

    /// @brief  Helper for propagation of address space type to instruction's uses
    /// @param  pInstr - instruction of interest
    /// @param  space  - space type to propagate
    void propagateSpace(Instruction *pInstr, OCLAddressSpace::spaces space);

    /// @brief  Checks whether an operand is a constant expression which produces
    /// @brief  GAS pointer out of named address space pointer, and adds instruction
    /// @brief  to the collection if this is the case
    /// @param  pOperand - operand to check
    /// @param  pInstr   - instruction to which this operand belongs (directly, or via
    ///                    encompassing constant expression)
    /// @returns  true if the check is successful (and the instruction is added to
    ///           the collection), or false otherwise
    bool handleGASConstantExprIfNeeded(Value *pOperand, Instruction *pInstr);

    // Generic-to-named address space resolvers for different instructions
    // --------------------------------------------------------------------

    /// @brief  GAS resolvers for different instructions
    /// @param  pInstr - instruction to resolve
    /// @param  space  - target named space
    /// @param  funcIt - iterator to function which is under resolution
    /// @returns  true if the IR was modified, or false otherwise
    bool resolveInstructionConvert(Instruction *pInstr, OCLAddressSpace::spaces space);
    bool resolveInstructionOnePointer(Instruction *pInstr, OCLAddressSpace::spaces space);
    bool resolveInstructionTwoPointers(Instruction *pInstr, OCLAddressSpace::spaces space);
    bool resolveInstructionPhiNode(PHINode *pPhiInstr, OCLAddressSpace::spaces space);
    bool resolveInstructionCall(CallInst *pCallInstr, GenericAddressSpace::TFunctionList::iterator curFuncIt);

    // Helpers for special cases
    // --------------------------

    /// @brief  Helper for folding of an "Address Space Qualifier" BI call
    /// @brief  into constant value it should produce
    /// @param  pCallInst - call instruction
    void foldAddressQualifierCall(CallInst *pCallInstr);

    /// @brief  Helper for resolution of a function call from generic to named addr space
    /// @param  pCallInst - call instruction
    /// @param  type      - category of the callee type: BI, intrinsic, or non-kernel
    /// @returns  pointer to specialized function's object or NULL if no new function was created
    Function *resolveFunctionCall(CallInst *pCallInstr, GenericAddressSpace::FuncCallType category);

    /// @brief  Helper for resolution of GAS pointer constant expression
    /// @brief  to the constant's named address space
    /// @param  pVal  - value to resolve
    /// @param  space - target named space
    /// @returns  resolved value if it was possible, or NULL otherwise
    Value *resolveConstantExpression(Value *pVal, OCLAddressSpace::spaces space);

    /// @brief  Helper for retrieval of replacement value for original instruction (from the map)
    /// @param  pInstr - original instruction
    /// @returns  pointer to replacement value or NULL if no replacement is identified
    Value *getReplacementForInstr(Instruction *pInstr);

    /// @brief  Helper for production of resolved value for instruction operand
    /// @param  pVal  - instruction operand
    /// @param  space - target named space
    /// @returns  pointer to resolved value or NULL if there is no resolved value available
    Value *getResolvedOperand(Value *pOperand, OCLAddressSpace::spaces space);

    /// @brief  Helper for fix-up of usages of new instruction pointer result:
    /// @brief    a) For incompatible named-vs-GAS pointers - induce bitcast
    /// @brief    b) If use is already-created PHI/Select/Icmp/Call - update corresponding input
    /// @param  pNewInstr - new instruction, whose usages should be fixed
    /// @param  pOldInstr - old instruction which will be replaced by new one
    void fixUpPointerUsages(Instruction *pNewInstr, Instruction *pOldInstr);

    // Checkers for special cases
    // ---------------------------

    /// @brief  Checks whether given type is an alloca of struct with GAS pointer
    /// @brief  pType            - type to check
    /// @brief  isStructDetected - true if we already identified 'struct' in
    ///         the course of recursion, or false otherwise
    /// @returns  true/false according to result of the check
    bool isAllocaStructGASPointer(const Type *pType, bool isStructDetected);

  };

} // namespace intel

#endif
