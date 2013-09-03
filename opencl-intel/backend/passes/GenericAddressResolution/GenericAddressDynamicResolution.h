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
#include <llvm/Value.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Instructions.h>
#include <llvm/User.h>
#include <llvm/LLVMContext.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Utils/PromoteMemToReg.h>
#include <llvm/ADT/SmallVector.h>
#include <map>
#include <vector>
#include <string>
#include <list>


using namespace Intel::OpenCL::DeviceBackend::Utils;
using namespace Intel::OpenCL::DeviceBackend::Passes;

#define GAS_INSTRUCTION_NAME "AddrSpace"
#define GAS_ALLOCA_NAME      "AllocaSpace"
#define GAS_ARG_NAME         "ArgSpace"

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
  class GenericAddressDynamicResolution : public ModulePass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief  Constructor
    GenericAddressDynamicResolution();

    /// @brief  Destructor
    ~GenericAddressDynamicResolution();

    /// @brief  Provides name of pass
    virtual const char *getPassName() const {
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

    /// @brief  The llvm context
    LLVMContext *m_pLLVMContext;

    /// @brief  Type of pointer holder slot
    IntegerType *m_PointerSlotType;

    /// @brief  Data Model for GAS Pointers' Def-Use Collection.
    /// @brief  Note: The collection contains only GAS pointers which are eventually
    /// @brief        used by Address Space Qualifier BI calls.
    /// @brief   1. Iteration through GAS Pointers which are to be resolved
    typedef std::list<const Value*>                   TPointerList;
    /// @brief   2. Quick access to GAS pointer addr space definition point
    /// @brief      (mapping from pointer value /first element/ to its 
    /// @brief       definition point /second element/)
    typedef std::pair<const Value*, const Value*>           TPointerDefPair;
    typedef std::map<const Value*,  const Value*>           TPointerDefMap;
    /// @brief   3. Quick access to GAS pointer's addr-space qualifier variable
    /// @brief      (mapping between GAS pointer's definition point /first element/
    /// @brief       and addr-space qualifier variable for this GAS pointer /second element/)
    typedef std::pair<const Value*, Value*>           TDefSpacePair;
    typedef std::map<const Value*, Value*>            TDefSpaceMap;

    /// @brief  Collection of function and GAS pointer info
    /// @brief   1. Function list in callee-to-caller (i.e., reversed call-graph) order
    GenericAddressSpace::TFunctionList m_functionsToHandle;
    /// @brief   2. GAS pointers in reversed data flow order (for one function only!)
    TPointerList                       m_GASPointers;
    /// @brief   3. GAS pointers' definition points for quick access (for one function only!)
    TPointerDefMap                     m_GASPointerDefs;
    /// @brief   4. GAS pointers' addr-space qualifier variables for quick access (for one function only!)
    TDefSpaceMap                       m_GASDefSpaces;

    /// @brief  Data Model for Collection of GAS Pointer Array/Scalar Allocas
    /// @brief   1. Quick access to GAS pointer alloca/array-argument upon its Load/Store usage
    /// @brief      (mapping between GAS pointer's Load/Store instruction /first element/
    /// @brief       and GAS pointer array 'alloca' area or array argument /second element/)
    /// @brief      Special case: Call parameter usage may not be an instruction!
    typedef std::pair<const Value*, const Value*> TAllocaUsagePair;
    typedef std::map<const Value*, const Value*>  TAllocaUsageMap;
    /// @brief   2. Quick access to GAS pointer addr-space qualifiers' alloca area or array argument
    /// @brief      (mapping between GAS pointer's 'alloca' area or array argument /first element/
    /// @brief       and 'alloca' area or array argument assigned for addr-space qualifiers of the pointers
    /// @brief       together with offsets between these areas /second element/)
    typedef struct {   // Information about 'alloca' or array argument with addr-space qualifier:
      Value       *pSpace;  // Array/scalar 'alloca' / array-argument value of addr-space qualifier
      Instruction *pOffset; // Offset (in bytes) from the corresponding GAS
                            // pointer array (NULL for scalar)
    } SpaceArrayDef;
    typedef std::pair<Value*, SpaceArrayDef> TAllocaSpacePair;
    typedef std::map<Value*, SpaceArrayDef>  TAllocaSpaceMap;
    /// @brief   3. Quick access to Store instructions to 'alloca' scalar/array
    /// @brief      of GAS pointers or array argument (mapping between GAS pointer's
    /// @brief      'alloca' area or array argument /first element/ and vector of Store
    /// @brief      instructions to that area /second element/)
    typedef std::pair<const Value*, std::vector<StoreInst*> > TAllocaStoresPair;
    typedef std::map<const Value*, std::vector<StoreInst*> >  TAllocaStoresMap;

    /// @brief  Collection of info about GAS pointer array's and scalar's 'alloca' or argument areas
    /// @brief   1. Info about ALL GAS pointers' Alloca/argument usages for quick access
    /// @brief      (for one function only!)
    TAllocaUsageMap         m_GASAllocaUsages;
    /// @brief   2. Info about GAS pointer addr-space qualifiers' 'alloca' or array argument
    /// @brief      and its offset from corresponding GAS pointer array 'alloca' or array
    /// @brief      argument for quick access (for one function only!)
    /// @brief      Note: only GAS pointers used by Address Qualifier BIs are collected here
    TAllocaSpaceMap         m_GASAllocaDefs;
    /// @brief   3. GAS pointer 'allocas' of arrays/scalars or array arguments used by
    /// @brief      Address Qualifier BIs - in deterministic iteration order (for one function only!)
    SmallVector<Value*, 16> m_GASAllocaVector;
    /// @brief   4. Info about ALL GAS pointers' Store instructions for quick access
    /// @brief      (for one function only!)
    TAllocaStoresMap        m_GASAllocaStores;

    /// @brief  Function call collection data model for the following accesses:
    /// @brief   1. Iteration through BI and Intrinsic calls with GAS parameters.
    /// @brief      Associates a function call /first element/ with named address 
    /// @brief      space type suggested for resolution /second element/.
    typedef std::pair<CallInst*, OCLAddressSpace::spaces> TBiIntrinPair;
    typedef SmallVector<TBiIntrinPair, 16>                TBiIntrinVector;
    /// @brief   2. Fast access to function's arguments which should be 
    /// @brief      resolved. Associates function object /first element/ with 
    /// @brief      vector of indices of arguments whose addr-space should be
    /// @brief      resolved by caller /second element/. Only arguments used by
    /// @brief      Address Space Qualifier BI calls in the function will be
    /// @brief      accounted for in the "vector of indices".
    typedef std::pair<const Function*, SmallVector<unsigned, 8> > TGASFuncPair;
    typedef std::map<const Function*, SmallVector<unsigned, 8> >  TGASFuncMap;

    /// @brief  Function call points info
    /// @brief   1. BI calls with GAS pointer parameters (for one function only!)
    TBiIntrinVector m_BiPoints;
    /// @brief   2. Intrinsic calls with GAS pointer parameters (for one function only!)
    TBiIntrinVector m_IntrinPoints;
    /// @brief   3. Functions with unresolved GAS pointer arguments
    TGASFuncMap     m_GASFunctions;

    /// @brief  Collection of GAS pointers' usage points for fix-up
    /// @brief   1. Calls to Address Qualifier BI
    SmallVector<CallInst*, 16>   m_AddrQualifierBICalls;
    /// @brief   2. Calls to non-kernel functions
    SmallVector<CallInst*, 16>   m_NonKernelCalls;
    /// @brief   3. Return instructions with GAS pointers
    SmallVector<ReturnInst*, 16> m_ReturnGAS;

  private:

    /// @brief  Prepares collection of GAS pointers
    /// @brief  and identifies their definition points
    /// @param  pFunc - function to traverse
    /// @returns  true if the function's GAS pointer definition comes from
    ///           outside, or false otherwise
    bool analyzeGASPointers(Function *pFunc);

    /// @brief  Inject the tracking of GAS pointers' address space into the IR
    /// @param  pFunc - function to process
    /// @returns  true if any code was injected, or false otherwise
    bool resolveGASPointers(Function *pFunc);

    /// @brief  Resolves usages of GAS pointers to 'named' address space
    /// @param  pFunc - function to resolve
    /// @returns  true if any call was resolved, or false otherwise
    bool resolveGASUsages(Function *pFunc);

    /// @brief  Clones the function, so that its signature will be
    /// @brief  consistent with the modifications of function body
    /// @param  pFunc - function to clone
    /// @returns  pointer to cloned function
    Function *cloneResolvedFunction(Function *pFunc);

    //  Helpers for GAS pointers' collection processing
    // -------------------------------------------------------

    /// @brief  Helper for step-by-step BOTTOM-UP traversal of
    /// @brief  GAS pointer value towards definition point
    /// @param  pPtrVal - pointer whose definition point is looked for
    /// @param  pCurVal - currently traversed pointer
    void traverseGASValue(const Value *pPtrVal, const Value *pCurVal);

    /// @brief  Helper for analysis of function call which 
    /// @brief  doesn't need to find a definition point
    /// @param  pCallInstr - instruction to analyze
    /// @param  category   - function category: BI or intrinsic
    void analyzeBIorIntrinsicCall(CallInst *pCallInstr, GenericAddressSpace::FuncCallType category);

    /// @brief  Helper for addition of a GAS pointer to the collection
    /// @param  pPtrVal - GAS pointer whose definition point to be looked for
    void addGASPointer(const Value *pPtrVal);

    /// @brief  Helper for addition of the definition point for a GAS pointer
    /// @param  pPtrVal   - GAS pointer whose definition point is found
    /// @param  pDefPoint - Instruction or formal argument which produces named address space pointer
    void collectDefPoint(const Value *pPtrVal, const Value *pDefPoint);

    /// @brief  Helper for collection of Load and Store instructions from/to memory
    /// @brief  area specified by an 'alloca' instruction or array argument and depending
    /// @brief  on a specific instruction
    /// @param  pArrayPtr - 'alloca'/argument which defines the memory area used by Loads and Stores
    /// @param  pCurVal   - instruction whose usages are traversed (recursively) in search
    ///                     for depending Load and Store instructions
    void collectLoadAndStoreUsagesFor(const Value *pArrayPtr, const Value *pCurVal);

    /// @brief  Given a GAS pointer, looks for addr-space qualifier variable for it 
    /// @param  pVal - GAS pointer of interest
    /// @returns  pointer to the GAS pointer's addr-space qualifier variable
    Value *getAddrSpaceFor(const Value *pVal);

    //  Helpers for addr-space qualifier LLVM values' generation 
    // ----------------------------------------------------------

    /// @brief  Helper for generation of addr-space qualifiers' constant (expression)
    /// @brief  out of pointer constant expression
    /// @param  pCE - constant expression to handle
    /// @returns  addr-space qualifiers' constant (expression), or NULL
    Constant *checkAndResolveConstantExpression(const ConstantExpr *pCE);

    /// @brief  Helper for resolution of constant expression's operand.
    /// @brief  Produces addr-space qualifiers' constant (expression)
    /// @param  pOperand - constant (expression) operand to handle
    /// @returns  addr-space qualifiers' constant (expression), or NULL
    Constant *checkAndResolveConstantOperand(const Constant *pOperand);

    /// @brief  Helper for generation of 'alloca' or array argument of addr-space
    /// @brief  qualifiers for a GAS pointer array, and calculation of offset between
    /// @brief  both of them
    /// @param  pArrayPtr    - 'Alloca'/argument of GAS pointer array
    /// @param  pInsertAfter - where to insert new instructions
    /// @returns  values of the addr-space qualifiers' 'alloca'/argument and its offset 
    ///           from GAS pointers' array
    SpaceArrayDef getArrayToSpaceOffset(Value *pArrayPtr, Instruction *pInsertAfter);

    /// @brief  Helper for generation of pointer to slot in 'alloca'/argument array with
    /// @brief  addr-space qualifiers upon pointer to slot in corresponding
    /// @brief  GAS pointer array 'alloca'/argument
    /// @param  pArrayPtr    - 'Alloca' slot / array argument for Load/Store
    /// @param  pOffset      - offset from the array pointer to a pointer to
    ///                        corresponding array of addr-space qualifiers
    /// @param  pInsertAfter - where to insert new instructions
    /// @returns  value of pointer to addr-space qualifier slot
    Instruction *mapArrayPtrToSpacePtr(Value *pArrayPtr, Instruction *pOffset, 
                                       Instruction *pInsertAfter);

    // Generic-to-named address space resolvers for different instructions using GAS pointers
    // --------------------------------------------------------------------------------------

    /// @brief  Check whether the GAS pointer definition can be resolved w/o 
    /// @brief  referring to instruction
    /// @param  def_it - iterator pointing to the GAS pointer definition
    /// @param  pFunc  - function under resolution
    /// @returns  true if quick resolution succeeded, or false otherwise
    bool tryQuickResolveSpaceDefinition(TPointerDefMap::iterator def_it, const Function *pFunc);

    /// @brief  GAS pointer resolvers for LLVM instructions other than call 
    /// @param  space_it - iterator pointing to the GAS pointer definition
    /// @returns  true if resolution succeeded, or false if it is deferred
    bool resolveLoadInstr(TDefSpaceMap::iterator space_it);
    bool resolveStoreInstructions();
    bool resolveRetInstructions();
    bool resolvePhiOrSelectInstr(TDefSpaceMap::iterator space_it);

    /// @brief  GAS pointer resolvers for call instructions
    /// @param  pInstr      - instruction to resolve
    /// @param  category    - function category: BI or intrinsic
    /// @param  targetSpace - target named space
    void resolveAddrSpaceQualifierBICall(CallInst *pCallInstr);
    void resolveBIorIntrinsicCall(CallInst *pCallInstr, GenericAddressSpace::FuncCallType category, OCLAddressSpace::spaces targetSpace);
    void resolveNonKernelFunctionCall(CallInst *pCallInstr);

    // Checkers for special cases
    // ---------------------------

    /// @brief  Checks whether given type is a scalar/array with GAS pointer 
    /// @brief  pType - type to check
    /// @returns  true/false according to result of the check
    bool isGASPointer(const Type *pType);

    /// @brief  Helper for check of GAS pointer array case
    /// @param  pType - type to check
    /// @returns  true if this is a GAS pointer array, or false otherwise
    bool isGASPtrArray(const Type *pType);

  };

} // namespace intel

#endif
