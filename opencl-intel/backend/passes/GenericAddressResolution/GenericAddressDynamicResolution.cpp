/*=================================================================================
Copyright (c) 2013, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "GenericAddressDynamicResolution.h"

#include <OCLPassSupport.h>
#include <NameMangleAPI.h>
#include <llvm/Constants.h>
#include <llvm/InstrTypes.h>
#include <llvm/Intrinsics.h>
#include <llvm/IntrinsicInst.h>
#include <llvm/GlobalValue.h>
#include <llvm/Support/InstIterator.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/ValueMap.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <set>
#include <assert.h>


using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend::Passes::GenericAddressSpace;

extern "C" {
  /// @brief  Creates new GenericAddressDynamicResolution module pass
  /// @returns new GenericAddressDynamicResolution module pass
  llvm::ModulePass *createGenericAddressDynamicResolutionPass() {
    return new intel::GenericAddressDynamicResolution();
  }
}

namespace intel {

  char GenericAddressDynamicResolution::ID = 0;

  OCL_INITIALIZE_PASS(GenericAddressDynamicResolution, "generic-addr-dynamic-resolution", "Resolves generic address space pointers to named ones", false, false)

  GenericAddressDynamicResolution::GenericAddressDynamicResolution() : ModulePass(ID) {
  }

  GenericAddressDynamicResolution::~GenericAddressDynamicResolution() {
  }

  bool GenericAddressDynamicResolution::runOnModule(Module &M) {
    bool changed = false;
    m_pModule = &M;
    m_pLLVMContext = &M.getContext();
    m_functionsToHandle.clear();
    m_GASFunctions.clear();

    // Sort all functions in call-graph order
    sortFunctionsInCGOrder(m_pModule, m_functionsToHandle, false);

    // Iterate through functions sorted in the function list 
    for (TFunctionList::iterator func_it = m_functionsToHandle.begin(), 
                                 func_it_end = m_functionsToHandle.end(); 
                                 func_it != func_it_end; func_it++) {
      m_GASPointers.clear();
      m_GASPointerDefs.clear();
      m_GASDefSpaces.clear();
      m_GASAllocaDefs.clear();
      m_BiPoints.clear();
      m_IntrinPoints.clear();
      m_AddrQualifierBICalls.clear();
      m_NonKernelCalls.clear();

      Function *pFunc = *func_it;
      // Collect per-function GAS pointers
      bool needsGASDefsFromArgs = analyzeGASPointers(pFunc);
      // Inject tracking of GAS pointers' address space into the IR
      changed |= resolveGASPointers(pFunc);
      // Resolved usages of GAS pointers to 'named' address space 
      changed |= resolveGASUsages(pFunc);
      // Clone function if its signature has changed
      if (needsGASDefsFromArgs) {
        // Replace the function with its clone
        *func_it = cloneResolvedFunction(pFunc);
        // Remove original function
        pFunc->eraseFromParent();
      }
    }

    return changed;
  }

  bool GenericAddressDynamicResolution::analyzeGASPointers(Function *pFunc) {

    // Collect calls with GAS pointers in the function (together with their immediate defs')
    for (inst_iterator inst_it = inst_begin(pFunc), 
                       inst_it_end = inst_end(pFunc); 
                       inst_it != inst_it_end; inst_it++) {

      Instruction *pInstr = &(*inst_it);

      // Filter-out all instructions but calls
      CallInst *pCallInstr = dyn_cast<CallInst>(pInstr);
      if (!pCallInstr) {
        continue;
      }

      // Analyze call instruction
      const Function *pCallee = pCallInstr->getCalledFunction();
      assert(pCallee && "Call instruction doesn't have a callee!");
      if (isAddressQualifierBI(pCallee)) {
        // Address Qualifier BI function call - start BOTTOM-UP data flow
        // analysis is search for definition point of its (single) parameter
        const Value *pSrcVal = pCallInstr->getArgOperand(0);
        const PointerType *pSrcType = dyn_cast<PointerType>(pSrcVal->getType());
        if (!pSrcType || !IS_ADDR_SPACE_GENERIC(pSrcType->getAddressSpace())) {
          assert(0 && "Address Qualifier BI function must have GAS pointer as a parameter!");
        }
        addGASPointer(pSrcVal);
        m_AddrQualifierBICalls.push_back(pCallInstr);
      } else if (isGenericAddrBI(pCallee)) {
        // BI call accepting GAS pointer arguments(s) - if needed, schedule for conversion 
        // to named-space parameters
        analyzeBIorIntrinsicCall(pCallInstr, CallBuiltIn);
      } else if (pCallee->isIntrinsic()) {
        // Intrinsic call - if needed, schedule for conversion to named-space parameters
        analyzeBIorIntrinsicCall(pCallInstr, CallIntrinsic);
      } else if (!pCallee->isDeclaration()) {
        // A function defined in the module - if needed, schedule for extension by 
        // named-space parameters
        TGASFuncMap::const_iterator func_it = m_GASFunctions.find(pCallee);
        if (func_it != m_GASFunctions.end()) {
          // Callee function has GAS pointer arguments which must be resolved -
          // start BOTTOM-UP data flow analysis is search for definition points
          // of its parameters whose corresponding arguments are reported as
          // "must be resolved"
          const SmallVector<unsigned, 8> &GASargIndices = func_it->second;
          // Scanning "must be resolved" arguments
          for (unsigned idx = 0; idx < GASargIndices.size(); idx++) {
            const Value *pSrcVal = pCallInstr->getArgOperand(GASargIndices[idx]);
            const PointerType *pSrcType = dyn_cast<PointerType>(pSrcVal->getType());
            if (pSrcType && !IS_ADDR_SPACE_GENERIC(pSrcType->getAddressSpace())) {
              assert(0 && "Function GAS argument collection is broken!");
            }
            addGASPointer(pSrcVal);
          }
          m_NonKernelCalls.push_back(pCallInstr);
        }
      } else {
        // This is a call to a function which is only declared in the module: do nothing
        continue;
      }
    }

    // Continue BOTTOM-UP traversal of the data flow in BFS order - until all 
    // definition points will be reached
    for (TPointerList::iterator ptr_it = m_GASPointers.begin(),
                                ptr_end = m_GASPointers.end(); 
                                ptr_it != ptr_end; ptr_it++) {
      // We can add new def/use mappings during traversal because they are collected
      // into list - whose iterator is safe after insertion
      traverseGASValue(*ptr_it, *ptr_it);
    }

    // Check whether this function's GAS pointer definition depends on argument(s)
    return m_GASFunctions.find(pFunc) != m_GASFunctions.end();
  }

  void GenericAddressDynamicResolution::addGASPointer(const Value *pPtrVal) {
    // Check whether this value is already scheduled for traversal,
    // and if not - add it to the collection 
    TPointerDefMap::iterator def_it = m_GASPointerDefs.find(pPtrVal);
    if (def_it == m_GASPointerDefs.end()) {
      m_GASPointers.push_back(pPtrVal);
      m_GASPointerDefs.insert(TPointerDefPair(pPtrVal, (Value*)NULL));
    }
  }

  void GenericAddressDynamicResolution::analyzeBIorIntrinsicCall(CallInst *pCallInstr, FuncCallType category) {

    assert((category == CallBuiltIn || category == CallIntrinsic) && "Unexpected function category!");
    // Address space enforcement: highest priority is 'global' for BI and 'private' for intrinsic
    // [we would like to have 'private' for all, however not all BIs have prototype with 'private']
    OCLAddressSpace::spaces hiPriSpace = (category == CallBuiltIn)? 
                                                    OCLAddressSpace::Global :
                                                    OCLAddressSpace::Private; 
    // Check for pointer arguments
    bool isGASParam = false;
    OCLAddressSpace::spaces foundSpace = OCLAddressSpace::Generic;
    for (unsigned idx = 0; idx < pCallInstr->getNumOperands(); idx++) {
      if (const PointerType *pSrcType = 
              dyn_cast<PointerType>(pCallInstr->getArgOperand(idx)->getType())) {
        // Check for pointer address space
        OCLAddressSpace::spaces space = 
                  (OCLAddressSpace::spaces) pSrcType->getAddressSpace();
        if (IS_ADDR_SPACE_GENERIC(space)) {
          // GAS pointer found - mark this call for future resolution
          isGASParam = true;
        } else {
          // Named-space pointer found - remember it (unless we already found
          // highest-priority space)
          if (foundSpace != hiPriSpace) {
            foundSpace = space;
          }
        }
      }
    }
    if (IS_ADDR_SPACE_GENERIC(foundSpace)) {
      // Set default to the highest priority target space
      foundSpace = hiPriSpace;
    }
    if (isGASParam) {
      // GAS pointer discovered - this function call should be scheduled for conversion
      // into its named-space pointer overload
      if (category == CallBuiltIn) {
        m_BiPoints.push_back(TBiIntrinPair(pCallInstr, foundSpace));
      } else {
        m_IntrinPoints.push_back(TBiIntrinPair(pCallInstr, foundSpace));
      }
    }
  }

  void GenericAddressDynamicResolution::traverseGASValue(const Value *pPtrVal, const Value *pCurVal) {

    // We traverse a pointer value according to its type: 
    //   1) argument, 
    //   2) instruction,
    //   3) pointer constant expression,
    //   4) null pointer
    if (const Argument *pArg = dyn_cast<Argument>(pCurVal)) {
      // If the value of interest is an argument - we collect it as definition.
      // The resolution will come from an additional argument (one per value of interest).
      collectDefPoint(pPtrVal, pCurVal);
      // If the argument is an integer or a pointer of generic type: prepare
      // the empty "vector of indices" (to be filled during resolution of arguments
      // which are definition points of GAS pointers)
      PointerType *pPtrType = dyn_cast<PointerType>(pArg->getType());
      bool toBeDefinedByCaller = !pPtrType || IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace());
      if (toBeDefinedByCaller) {
        TGASFuncMap::iterator func_it = m_GASFunctions.find(pArg->getParent());
        if (func_it == m_GASFunctions.end()) {
          SmallVector<unsigned, 8> indices;
          m_GASFunctions.insert(TGASFuncPair(pArg->getParent(), indices));
        }
      }
      // The argument is a terminal - nowhere to propagate
    } else if (const Instruction *pInstr = dyn_cast<Instruction>(pCurVal)) {
      // If the value of interest is an instruction - analyze it according to its type
      switch (pInstr->getOpcode()) {
        case Instruction::GetElementPtr :
        case Instruction::PtrToInt      :
        case Instruction::BitCast       : {
          // For unary instruction which may define address space of the value of
          // interest - either collect the definition, or traverse BOTTOM-UP
          const Value *pOperand = pInstr->getOperand(0);
          if (const PointerType *pPtrType = dyn_cast<PointerType>(pOperand->getType())) {
            // Bitcast/GEP/Ptr2Int from pointer - check if it has named address space
            if (!IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace())) {
              // We identified a definition point - collect it
              collectDefPoint(pPtrVal, pCurVal);
              break;
            }
          }
          // It is not a definition point - continue with BOTTOM-UP traversal
          traverseGASValue(pPtrVal, pOperand);
          break;
        }
        case Instruction::Select :
        case Instruction::PHI    : {
          // For instruction which is ambiguous w.r.t. pointer propagation:
          // consider it as a definition point for current GAS pointer, while 
          // scheduling its inputs as new GAS pointers for BOTTOM-UP traversal
          collectDefPoint(pPtrVal, pCurVal);
          unsigned firstIdx = (pInstr->getOpcode() == Instruction::PHI)? 0 : 1;
          for (unsigned idx = firstIdx; idx < pInstr->getNumOperands(); idx++) {
            addGASPointer(pInstr->getOperand(idx));
          }
          break;
        }
        case Instruction::Load : {
          // Load - keep track on all stores to the same address
          const AllocaInst *pAllocaPtr = dyn_cast<AllocaInst>(pInstr->getOperand(0));
          assert(pAllocaPtr && "GAS pointer can be loaded from value of Alloca only!");
          // Consider this instruction as a definition point for current GAS pointer
          collectDefPoint(pPtrVal, pCurVal);
          // Check whether the input pointer was already encountered
          std::pair<TAllocaMap::iterator,bool> res = 
                         m_GASAllocaDefs.insert(TAllocaPair(pAllocaPtr, (AllocaInst*) NULL));
          if (res.second) {
            // The input pointer wasn't encountered yet - schedule all inputs of Stores to
            // the same 'Alloca' slot for BOTTOM-UP traversal
            for (Value::const_use_iterator use_it = pAllocaPtr->use_begin(), 
                                           use_end = pAllocaPtr->use_end();
                                           use_it != use_end; use_it++) {
              const Instruction *pUse = dyn_cast<Instruction>(*use_it);
              assert(pUse && "Alloca uses must be instructions!");
              if (pUse->getOpcode() == Instruction::Store) {
                 addGASPointer(pUse->getOperand(0));
              }
            }
          }
          break;
        }
        case Instruction::IntToPtr :
          // Int-to-ptr - traverse further BOTTOM-UP, until PtrToInt will be encountered
          traverseGASValue(pPtrVal, pInstr->getOperand(0));
          break;
        default:
          // For binary operations - try both operands as a target of PtrToInt
          if (pInstr->isBinaryOp()) {
            traverseGASValue(pPtrVal, pInstr->getOperand(0));
            traverseGASValue(pPtrVal, pInstr->getOperand(1));
            break;
          }
          assert(0 && "Unexpected instruction using GAS pointer or its bitcast!");
          break;
      }
    } else if (const ConstantExpr *pCE = dyn_cast<ConstantExpr>(pCurVal)) {
      // If the value of interest is a constant expression - this is where
      // the address space may be defined
      const PointerType *pPtrType = dyn_cast<PointerType>(pCE->getType());
      if (pPtrType && !IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace())) {
        // Expression creates GAS pointer: collect this definition point
        collectDefPoint(pPtrVal, pCurVal);
      } else {
        Constant *pSpaceVal = checkAndResolveConstantExpression(pCE);
        assert(pSpaceVal && pSpaceVal->getType()->isIntegerTy() && 
               "Constant expression traversal failure");
        if (cast<ConstantInt>(pSpaceVal)->getZExtValue() != 
                                            (unsigned) OCLAddressSpace::Generic) {
          // Only named pointer value is of relevance
          collectDefPoint(pPtrVal, pCurVal);
        }
      }
    } else if (dyn_cast<ConstantPointerNull>(pCurVal)) {
      // For NULL value - record it for further resolution to its address space
      collectDefPoint(pPtrVal, pCurVal);
    } else {
      // This is a leaf value which is neither constant expression, nor NULL pointer
      // It may emerge upon two cases:
      //  a) "Dead-end" integer value traversal (see BinaryOp handling above).
      //  b) Uninitialized GAS pointer, or integer value not built out of a pointer.
      // In both cases we do nothing:
      //  - Case (a) is covered by traversal of another binary operand.
      //  - Case (b) will be covered by resolution of a GAS pointer which remained unresolved.
    }
  }

  void GenericAddressDynamicResolution::collectDefPoint(const Value *pPtrVal, const Value *pDefPoint) {
    TPointerDefMap::iterator def_it = m_GASPointerDefs.find(pPtrVal);
    assert(def_it != m_GASPointerDefs.end() && "Collection of GAS pointers is broken!");
    def_it->second = const_cast<Value*>(pDefPoint);
    m_GASDefSpaces.insert(TDefSpacePair(pDefPoint, (Value*)NULL));
  }

  Value *GenericAddressDynamicResolution::getAddrSpaceFor(const Value *pVal) {
    TPointerDefMap::iterator def_it = m_GASPointerDefs.find(pVal);
    assert(def_it != m_GASPointerDefs.end() && "GAS pointer collection is broken!");
    TDefSpaceMap::iterator space_it = m_GASDefSpaces.find(def_it->second);
    if (space_it == m_GASDefSpaces.end()) {
      return NULL;
    }
    return space_it->second;
  }

  bool GenericAddressDynamicResolution::resolveGASPointers(Function *pFunc) {

    bool changed = false;

    // At first - allocate 'Alloca' slots (if needed) for addr-space qualifier of the 
    // pointer values in 'alloca' slots
    for (TAllocaMap::iterator alloca_it = m_GASAllocaDefs.begin(),
                              alloca_end = m_GASAllocaDefs.end();
                              alloca_it != alloca_end; alloca_it++) {
      alloca_it->second = new AllocaInst(IntegerType::get(*m_pLLVMContext, 32),
                                         GAS_INSTRUCTION_NAME, pFunc->begin()->begin());
      assocDebugLocWith(alloca_it->second, pFunc->begin()->begin());
      changed = true;
    }

    // Then - generate LLVM virtual registers for addr-space qualifiers. Such variable
    // will reflect changes in addr-space qualifier of respective GAS pointer, so that
    // its currect value can be used by an Address Space Qualifier BI for querry about
    // addr-space qualifier of that pointer in run-time.

    // Starting from arguments - we need their addr-space variables (if any) in the same
    // order as they appear in the function's signature, otherwise we would follow the
    // common algorithm further below.
    SmallVector<unsigned, 8> *pIndices = NULL;
    TGASFuncMap::iterator func_it = m_GASFunctions.find(pFunc);
    if (func_it != m_GASFunctions.end()) {
      pIndices = &func_it->second;
      assert(pIndices->size() == 0 && "Function argument indices collection is broken!");
    }
    SmallVector<TDefSpaceMap::iterator, 8> extraArgs;

    unsigned idx = 0;
    for (Function::arg_iterator arg_it = pFunc->arg_begin(), 
                                arg_end = pFunc->arg_end(); 
                                arg_it != arg_end; arg_it++, idx++) {
      TDefSpaceMap::iterator space_it = m_GASDefSpaces.find(arg_it);
      if (space_it != m_GASDefSpaces.end()) {
        assert(!space_it->second && "GAS Pointer collection is broken!");
        // This argument serves as a definition point - check if it's addr-space
        // is expected to be defined by function's caller(s).
        // The criteria is: the argument is an integer or a pointer of generic type
        PointerType *pPtrType = dyn_cast<PointerType>(arg_it->getType());
        bool toBeDefinedByCaller = !pPtrType || IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace());
        if (toBeDefinedByCaller) {
          assert(pIndices && "Function argument indices collection is broken!");
          // For defined-by-caller argument - add GAS argument index to
          // existing per-function "vector of indices"
          pIndices->push_back(idx);
          // ... and collect the argument's definition for further resolution
          extraArgs.push_back(space_it);
        } else {
          // For argument which defines addr-space - resolve the definition point
          // with corresponding constant value of addr-space enumerator
          space_it->second = ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext), 
                                              pPtrType->getAddressSpace());
          changed = true;
        }
      }
    }
    // Finally - create new address space qualifier arguments (if any) for the current function.
    // They will serve as containers for addr-space qualifiers of GAS pointers.
    for (unsigned idx = 0; idx < extraArgs.size(); idx++) {
      extraArgs[idx]->second = 
            new Argument(IntegerType::get(*m_pLLVMContext, 32), GAS_ARG_NAME, pFunc);
      changed = true;
    }

    // For all definition points other than arguments
    while (true) {
      bool isDeferred = false;
      // Iterate through the collection of GAS pointers (TOP-DOWN) and associate them
      // and their definition values with generated values of corresponding 
      // addr-space qualifier LLVM virtual registers
      for (TPointerList::reverse_iterator ptr_it = m_GASPointers.rbegin(), 
                                          ptr_end = m_GASPointers.rend();
                                          ptr_it != ptr_end; ptr_it++) {
        TPointerDefMap::iterator def_it = m_GASPointerDefs.find(*ptr_it);
        assert(def_it != m_GASPointerDefs.end() && "GAS pointer collection is broken!");

        if (tryQuickResolveSpaceDefinition(def_it, pFunc)) {
          changed = true;
          continue;
        }

        const Value *pDefValue = def_it->second;
        TDefSpaceMap::iterator space_it = m_GASDefSpaces.find(pDefValue);
        assert(space_it != m_GASDefSpaces.end() && "GAS pointer collection is broken!");
        // We handle a pointer definition value according to its type: 
        //   1) instruction,
        //   2) pointer constant expression,
        //   3) null pointer
        if (const Instruction *pInstr = dyn_cast<Instruction>(pDefValue)) {
          // If the value of interest is an instruction - create new address space
          // qualifier value according to its type
          switch (pInstr->getOpcode()) {
            case Instruction::GetElementPtr :
            case Instruction::PtrToInt      :
            case Instruction::BitCast       : {
              const PointerType *pPtrType = dyn_cast<PointerType>(
                                                    pInstr->getOperand(0)->getType());
              assert(pPtrType && !IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace()) &&
                     "GAS pointer collection is broken!");
              // Instruction creates GAS pointer: create address space qualifier constant
              space_it->second = ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext), 
                                                  pPtrType->getAddressSpace());
              changed = true;
              break;
            }
            case Instruction::PHI    :
            case Instruction::Select :
              // PHI/Select - check that all inputs are already resolved 
              // and if yes - generate PHI/Select out of addr-space qualifiers
              if (resolvePhiOrSelectInstr(space_it)) {
                changed = true;
              } else {
                isDeferred = true;
              }
              break;
            case Instruction::Load : {
              // Load: create Load instruction from 'alloca' slot for address space 
              // qualifier value
              TAllocaMap::iterator alloca_it = m_GASAllocaDefs.find(
                                                  cast<AllocaInst>(pInstr->getOperand(0)));
              assert(alloca_it != m_GASAllocaDefs.end() && "Alloca collection is broken!");
              AllocaInst *pNewAllocaInstr = alloca_it->second;
              assert(pNewAllocaInstr && "Alloca collection is broken!");
              Instruction *pNewInstr = new LoadInst(pNewAllocaInstr, GAS_INSTRUCTION_NAME);
              pNewInstr->insertAfter(const_cast<Instruction*>(pInstr));
              assocDebugLocWith(pNewInstr, pInstr);
              space_it->second = pNewInstr;
              changed = true;
              break;
            }
            default:
              assert(0 && "GAS pointer collection is broken!");
              break;
          }
        } else if (const ConstantExpr *pCE = dyn_cast<ConstantExpr>(pDefValue)) {
          // Constant expression: create constant (expression) with address space
          // qualifier value of the expression
          const PointerType *pPtrType = dyn_cast<PointerType>(pCE->getType());
          if (pPtrType && !IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace())) {
            // Expression creates GAS pointer: create address space qualifier constant
            space_it->second = ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext), 
                                                pPtrType->getAddressSpace());
          } else {
            // Dig into the expression is search for the pointer address space
            space_it->second = checkAndResolveConstantExpression(pCE);
            assert(space_it->second && "Constant expression traversal failure");
          }
          changed = true;
        } else if (const ConstantPointerNull *pNull = dyn_cast<ConstantPointerNull>(pDefValue)) {
          // Null pointer: create address space qualifier value of the pointer address space
          space_it->second = ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext),
                                              pNull->getType()->getAddressSpace());
          changed = true;
        } else {
          assert(0 && "Unreachable point - unhandled unresolved GAS pointer");
        }
      }
      if (!isDeferred) {
        // All GAS pointers are associated with address space qualifier LLVM virtual register
        break;
      }
    }

    // Resolve all Store instructions which depend on 'Alloca' GAS pointers used by 
    // Address Space Qualifier BIs
    changed |= resolveStoreInstructions();    

    return changed; 
  }

  bool GenericAddressDynamicResolution::resolveGASUsages(Function *pFunc) {

    bool changed = false;

    // Resolve Calls to Addr Space Qualifier BIs
    for (SmallVector<CallInst*, 16>::iterator call_it = m_AddrQualifierBICalls.begin(),
                                              call_end = m_AddrQualifierBICalls.end();
                                              call_it != call_end; call_it++) {
      // Inlining the function call to instructions which calculate the function result
      resolveAddrSpaceQualifierBICall(*call_it);
      changed = true;
    }
    // Resolve BI function calls with GAS pointers
    for (TBiIntrinVector::iterator call_it = m_BiPoints.begin(),
                                   call_end = m_BiPoints.end();
                                   call_it != call_end; call_it++) {
      // Enforcing named address space to the function call
      resolveBIorIntrinsicCall(call_it->first, CallBuiltIn, call_it->second);
      changed = true;
    }
    // Resolve Intrinsic function calls with GAS pointers
    for (TBiIntrinVector::iterator call_it = m_IntrinPoints.begin(),
                                   call_end = m_IntrinPoints.end();
                                   call_it != call_end; call_it++) {
      // Enforcing named address space to the function call
      resolveBIorIntrinsicCall(call_it->first, CallIntrinsic, call_it->second);
      changed = true;
    }
    // Resolve non-kernel function calls with GAS pointers
    for (SmallVector<CallInst*, 16>::iterator call_it = m_NonKernelCalls.begin(),
                                              call_end = m_NonKernelCalls.end();
                                              call_it != call_end; call_it++) {
      // Replacing original function call with the one extended with addr-space qualifier parameters
      resolveNonKernelFunctionCall(*call_it);
      changed = true;
    }
    return changed;
  }

} // namespace intel
