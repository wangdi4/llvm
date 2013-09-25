/*=================================================================================
Copyright (c) 2012, Intel Corporation
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
    m_PointerSlotType = 
      (m_pModule->getPointerSize() == Module::Pointer64)? 
                                          Type::getInt64Ty(*m_pLLVMContext): 
                                          Type::getInt32Ty(*m_pLLVMContext);
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
      m_GASAllocaUsages.clear();
      m_GASAllocaDefs.clear();
      m_GASAllocaVector.clear();
      m_GASAllocaStores.clear();
      m_BiPoints.clear();
      m_IntrinPoints.clear();
      m_AddrQualifierBICalls.clear();
      m_NonKernelCalls.clear();
      m_ReturnGAS.clear();

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

    // Collect usage tree of Loads/Stores for GAS pointer array arguments
    for (Function::arg_iterator arg_it = pFunc->arg_begin(),
                                arg_end = pFunc->arg_end();
                                arg_it != arg_end; arg_it++) {
      Type *pArgType = arg_it->getType();
      if (isGASPtrArray(pArgType)) {
        collectLoadAndStoreUsagesFor(arg_it, arg_it);
      }
    }

    // Collect calls with GAS pointers in the function (together with their immediate defs')
    for (inst_iterator inst_it = inst_begin(pFunc), 
                       inst_it_end = inst_end(pFunc); 
                       inst_it != inst_it_end; inst_it++) {

      Instruction *pInstr = &(*inst_it);

      if (pInstr->getOpcode() == Instruction::Ret) {
        // Ret instruction - check return value (if any) for GAS pointer
        ReturnInst *pRetInstr = cast<ReturnInst>(pInstr);
        if (Value *pRetVal = pRetInstr->getReturnValue()) {
          // Note that we assume that GAS pointer return value will always
          // feed its value to a Address Qualifier BI function call, however 
          // this case is not expected to be frequent. That assumption significantly
          // simplifies the analysis: otherwise the analysis should be delegated 
          // to caller, and then we will need yet more rounds over all functions
          // in order to resolve new (circular) dependencies:
          //  - caller will detect that callee's return GAS return value is of interest,
          //  - callee analysis will look for "named" origin(s) of return value,
          //  - that analysis may add new space-value argument(s) to caller-to-callee function call
          //  - call parameter(s) which correspond(s) to argument(s) should be analyzed for "named" origin(s)
          //  - on the way we can find dependency on pointer result value of yet another callee, 
          //  - the latter may produce yet new space-value argument(s) to be resolved 
          //  - the latter may produce yet another dependency on return value and so on.
          // In worst case, amount of rounds will reach amount of functions with GAS pointer
          // return value. Compile-time vs. optimization gain trade-off can be negative here.
          const PointerType *pPtrType = dyn_cast<PointerType>(pRetVal->getType());
          if (pPtrType && IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace())) {
            // Returns GAS pointer: start BOTTOM-UP data flow analysis in search
            // for its definition point
            addGASPointer(pRetVal);
            m_ReturnGAS.push_back(pRetInstr);
          }
        }
        continue;
      }

      const AllocaInst *pAlloca = dyn_cast<const AllocaInst>(pInstr);
      if (pAlloca && isGASPointer(pAlloca->getAllocatedType())) {
        // Collect usage tree of Loads/Stores for this 'alloca'
        collectLoadAndStoreUsagesFor(pAlloca, pAlloca);
        continue;
      }

      // Filter-out all other instructions but calls
      CallInst *pCallInstr = dyn_cast<CallInst>(pInstr);
      if (!pCallInstr) {
        continue;
      }

      // Analyze call instruction
      const Function *pCallee = pCallInstr->getCalledFunction();
      assert(pCallee && "Call instruction doesn't have a callee!");
      if (isAddressQualifierBI(pCallee)) {
        // Address Qualifier BI function call - start BOTTOM-UP data flow
        // analysis in search for definition point of its (single) parameter
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
          // start BOTTOM-UP data flow analysis in search for definition points
          // of its parameters whose corresponding arguments are reported as
          // "must be resolved"
          const SmallVector<unsigned, 8> &GASargIndices = func_it->second;
          // Scanning "must be resolved" arguments
          for (unsigned idx = 0; idx < GASargIndices.size(); idx++) {
            Value *pSrcVal = pCallInstr->getArgOperand(GASargIndices[idx]);
            Type *pArgType = pSrcVal->getType();
            const PointerType *pSrcType = dyn_cast<PointerType>(pArgType);
            if ((!pSrcType || IS_ADDR_SPACE_GENERIC(pSrcType->getAddressSpace())) && 
                                                                !isGASPtrArray(pArgType)) {
              // We don't consider GAS array parameter as a value to be traced bottom-up...
              addGASPointer(pSrcVal);
            } else {
              // ... instead - we add it to the collection of must-be-resolved
              // GAS pointer arrays and schedule its Stores for traversal
              // Check whether this array area was already encountered
              SpaceArrayDef spaceArray = {NULL, NULL};
              TAllocaUsageMap::const_iterator alloca_it = m_GASAllocaUsages.find(pSrcVal);
              assert(alloca_it != m_GASAllocaUsages.end() && "GAS pointer array collection is broken!");
              Value *pGASArray = const_cast<Value*>(alloca_it->second);
              std::pair<TAllocaSpaceMap::iterator,bool> res =
                      m_GASAllocaDefs.insert(TAllocaSpacePair(pGASArray, spaceArray));
              if (res.second) {
                // This array area wasn't encountered yet - schedule all inputs of Stores to
                // the same array area for BOTTOM-UP traversal
                TAllocaStoresMap::const_iterator stores_it = m_GASAllocaStores.find(pGASArray);
                if (stores_it != m_GASAllocaStores.end()) {
                  const std::vector<StoreInst*> &stores = stores_it->second;
                  for (unsigned idx = 0; idx < stores.size(); idx++) {
                    addGASPointer(stores[idx]->getValueOperand());
                  }             
                }
                // Also remember this Alloca/argument for future deterministic iterations
                m_GASAllocaVector.push_back(pGASArray);
              }
            }
          }
          m_NonKernelCalls.push_back(pCallInstr);
        } else {
          // Remember the function call if it returns GAS pointer
          PointerType *pPtrType = dyn_cast<PointerType>(pCallee->getReturnType());
          if (pPtrType && IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace())) {
            m_NonKernelCalls.push_back(pCallInstr);
          }
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
    return m_GASFunctions.find(pFunc) != m_GASFunctions.end() || m_ReturnGAS.size();
  }

  bool GenericAddressDynamicResolution::isGASPointer(const Type *pType) {
    // There is an assumption that no structs with GAS pointers or their
    // aggregates at this point (validated by static GAS pointer resolution pass).
    // Note that integer arrays with pointer-size elements are also of interest,
    // because they may serve for storing of pointers.
    if (pType->isArrayTy()) {
      // Look into the array element for GAS pointer
      const Type *pArrayElemType = pType->getArrayElementType();
      if (pArrayElemType->isArrayTy() && isGASPointer(pArrayElemType)) {
        return true;
      } else if (pArrayElemType == m_PointerSlotType ||
                 (pArrayElemType->isPointerTy() && 
                  IS_ADDR_SPACE_GENERIC(
                    cast<const PointerType>(pArrayElemType)->getAddressSpace()))) {
        return true;
      }
    } else if (pType == m_PointerSlotType ||
               (pType->isPointerTy() &&
                IS_ADDR_SPACE_GENERIC(
                   cast<const PointerType>(pType)->getAddressSpace()))) {
      return true;
    }
    return false;
  }

  bool GenericAddressDynamicResolution::isGASPtrArray(const Type *pType) {
    return (pType->isArrayTy() && isGASPointer(pType->getArrayElementType())) ||
           (pType->isPointerTy() && !isSinglePtr(pType) && 
                                isGASPointer(pType->getPointerElementType()));
  }

  void GenericAddressDynamicResolution::collectLoadAndStoreUsagesFor(
                                            const Value *pArray, 
                                            const Value *pCurVal) {
    for (Value::const_use_iterator use_it = pCurVal->use_begin(),
                                   use_end = pCurVal->use_end();
                                   use_it != use_end; use_it++) {

      const Instruction *pUse = dyn_cast<Instruction>(*use_it);
      assert(pUse && "Array uses must be instructions!");
      switch (pUse->getOpcode()) {
        case Instruction::Load  :
        case Instruction::Store :
          if (m_GASAllocaUsages.find(pUse) == m_GASAllocaUsages.end()) {
            // First-time encountered Load/Store instruction - keep its mapping to Alloca/array-argument
            m_GASAllocaUsages.insert(TAllocaUsagePair(pUse, pArray));
            if (pUse->getOpcode() == Instruction::Store) {
              // For Store instruction - keep mapping from Alloca/array-argument to it
              TAllocaStoresMap::iterator store_it = m_GASAllocaStores.find(pArray); 
              if (store_it == m_GASAllocaStores.end()) {
                std::vector<StoreInst*> stores;
                std::pair<TAllocaStoresMap::iterator, bool> res = m_GASAllocaStores.insert(TAllocaStoresPair(pArray, stores));
                store_it = res.first;
              }
              const StoreInst *pStore = cast<StoreInst>(pUse);
              store_it->second.push_back(const_cast<StoreInst*>(pStore));
            }
          }
          // Nothing to DFS after Load and Store
          break;
        case Instruction::Call :
          // Call is also a terminal usage (when parameter is a GAS pointer array)
          // We should map GAS pointer array parameter to its GAS pointer array 'alloca'/argument
          if (m_GASAllocaUsages.find(pCurVal) == m_GASAllocaUsages.end()) {
            // First-time encountered array usage instruction - keep its mapping to Alloca/array-argument
            m_GASAllocaUsages.insert(TAllocaUsagePair(pCurVal, pArray));
          }
          // Nothing to DFS after Call
          break;
        case Instruction::GetElementPtr :
        case Instruction::IntToPtr      :
        case Instruction::PtrToInt      :
        case Instruction::BitCast       :
          // For GEP/Bitcast/Int2Ptr/ptr2int instruction - continue to search
          collectLoadAndStoreUsagesFor(pArray, pUse);
          break;
        default:
          if (pUse->isBinaryOp()) {
            // For binary operation upon pointer's integer holder - continue to search
            collectLoadAndStoreUsagesFor(pArray, pUse);
          }
          // We expect GAS pointer arrays to be represented by 'alloca', rather than 
          // by LLVM array aggregate value or by vector, hence we don't expect 'extractvalue' 
          // and 'insertvalue' instructions.
          // For other usages (e.g., ICmp) there is nothing to DFS further.
          break;
      }
    }
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
    bool isGASParam  = false;
    bool hasMismatch = false;
    bool isFirst     = true;
    OCLAddressSpace::spaces foundSpace = OCLAddressSpace::Generic;
    for (unsigned idx = 0; idx < pCallInstr->getNumArgOperands(); idx++) {
      if (const PointerType *pSrcType = 
              dyn_cast<PointerType>(pCallInstr->getArgOperand(idx)->getType())) {
        // Check for pointer address space
        OCLAddressSpace::spaces curSpace = 
                  (OCLAddressSpace::spaces) pSrcType->getAddressSpace();
        if (IS_ADDR_SPACE_GENERIC(curSpace)) {
          // GAS pointer found - mark this call for future resolution
          isGASParam = true;
        } else {
          // Named-space pointer found - remember it (unless we already found
          // highest-priority space)
          if (isFirst) {
            foundSpace = curSpace;
            isFirst = false;
          } else if (curSpace != foundSpace) {
            hasMismatch = true;
            if (foundSpace != hiPriSpace) {
              foundSpace = curSpace;
            }
          }
        }
      }
    }
    if (IS_ADDR_SPACE_GENERIC(foundSpace)) {
      // Set default to the highest priority target space
      foundSpace = hiPriSpace;
    }
    if (isGASParam || hasMismatch) {
      // GAS pointer discovered or there is a mismatch between 'named' address 
      // space types of different parameters - this function call should be scheduled
      // for conversion into its named-space pointer overload
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
      // Note that GAS pointer arrays are effectively filtered-out as well, because they cannot be
      // generic (they are generated by 'alloca' and thus are of Private space)
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
          const Type *pOperandType = pOperand->getType();
          if (const PointerType *pPtrType = dyn_cast<PointerType>(pOperandType)) {
            assert(isSinglePtr(pOperandType) && 
                   "Bitcast/GEP/Ptr2Int from GAS 'alloca'/argument array value cannot be reached during bottom-up traversal!");
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
          TAllocaUsageMap::const_iterator alloca_it = m_GASAllocaUsages.find(pInstr);
          assert(alloca_it != m_GASAllocaUsages.end() && 
                 "GAS pointer can be originated from value of Alloca/argument only!");
          // Consider this instruction as a definition point for current GAS pointer
          collectDefPoint(pPtrVal, pCurVal);
          Value *pArrayPtr = const_cast<Value*>(alloca_it->second);
          // Check whether this array area was already encountered
          SpaceArrayDef spaceArray = {NULL, NULL};
          std::pair<TAllocaSpaceMap::iterator,bool> res =
                  m_GASAllocaDefs.insert(TAllocaSpacePair(pArrayPtr, spaceArray));
          if (res.second) {
            // This array area wasn't encountered yet - schedule all inputs of Stores to
            // the same array area for BOTTOM-UP traversal
            TAllocaStoresMap::const_iterator stores_it = m_GASAllocaStores.find(pArrayPtr);
            if (stores_it != m_GASAllocaStores.end()) {
              const std::vector<StoreInst*> &stores = stores_it->second;
              for (unsigned idx = 0; idx < stores.size(); idx++) {
                addGASPointer(stores[idx]->getValueOperand());
              }             
            }
            // Also remember this Alloca/argument for future deterministic iterations
            m_GASAllocaVector.push_back(pArrayPtr);
          }
          break;
        }
        case Instruction::Call : {
          // Call instruction can be reached only if its GAS pointer result value is in use
          PointerType *pPtrType = dyn_cast<PointerType>(pInstr->getType());
          if (pPtrType) {
            // We can pick that point as definition point only if it is a pointer
            assert(IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace()) && 
                   "A pointer should have generic address space at this point!");
            collectDefPoint(pPtrVal, pCurVal);
          }
          // If the function returns integer rather than the pointer - the corresponding
          // value will be left undefined, and will be resolved to 'Generic' space in
          // the course of resolution. This is a trade-off vs. time-consuming handling
          // of circular dependencies between caller(s) and callee(s). We assume that
          // Pointer<->Integer conversions are used for pointer arithmetic only, and not
          // for purpose of storing & conveying pointer values across function boundaries.
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
    def_it->second = pDefPoint;
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

    // At first - generate addr-space qualifiers' arrays from the
    // corresponding 'Alloca' GAS pointer arrays
    for (unsigned idx = 0; idx < m_GASAllocaVector.size(); idx++) {
      Value *pArray= m_GASAllocaVector[idx];
      if (AllocaInst *pAlloca = dyn_cast<AllocaInst>(pArray)) {
        // For GAS pointer array/scalar represented by 'alloca' - generate addr-space 
        // qualifier 'alloca' array/scalar and calculate its offset (for array case only!) 
        TAllocaSpaceMap::iterator alloca_it = m_GASAllocaDefs.find(pAlloca);
        assert(alloca_it != m_GASAllocaDefs.end() && 
               "Collection of GAS pointer arrays is broken!");
        if (!pAlloca->getAllocatedType()->isArrayTy() && !pAlloca->isArrayAllocation()) {
          // Optimization for 'alloca' scalar case - we can omit offset generation
          Instruction *pFuncStart = pFunc->getEntryBlock().begin();
          AllocaInst *pSpaceArray = new AllocaInst(m_PointerSlotType, GAS_ALLOCA_NAME, pFuncStart);
          alloca_it->second.pSpace = pSpaceArray;
          assocDebugLocWith(pSpaceArray, pFuncStart);
          // We leave offset value as NULL
        } else {
          // Common case - generate addr-space qualifier 'alloca' array and calculate its offset
          alloca_it->second = getArrayToSpaceOffset(pAlloca, pAlloca);
        }
        changed = true;
      } else if (dyn_cast<Argument>(pArray)) {
        // For array - just prepare to further GAS pointer argument resolution
        TGASFuncMap::iterator func_it = m_GASFunctions.find(pFunc);
        if (func_it == m_GASFunctions.end()) {
          SmallVector<unsigned, 8> indices;
          m_GASFunctions.insert(TGASFuncPair(pFunc, indices));
        }
      } else {
        assert(0 && "Only 'alloca' or array argument may produce GAS pointer array!");
      }
    }

    // Then - generate LLVM virtual registers for addr-space qualifiers. Such
    // variable will reflect changes in addr-space qualifier of respective GAS pointer,
    // so that its current value can be used by an Address Space Qualifier BI for
    // querry about addr-space qualifier of that pointer in run-time.

    // Starting from arguments - we need their addr-space variables (if any) in the same
    // order as they appear in the function's signature, otherwise we would follow the
    // common algorithm further below.
    SmallVector<unsigned, 8> *pIndices = NULL;
    TGASFuncMap::iterator func_it = m_GASFunctions.find(pFunc);
    if (func_it != m_GASFunctions.end()) {
      pIndices = &func_it->second;
      assert(pIndices->size() == 0 && "Function argument indices collection is broken!");
    }
    SmallVector<Argument*, 8> funcArgs;
    for (Function::arg_iterator arg_it = pFunc->arg_begin(),
                                arg_end = pFunc->arg_end();
                                arg_it != arg_end; arg_it++) {
      funcArgs.push_back(arg_it);
    }

    for (unsigned idx = 0; idx < funcArgs.size(); idx++) {
      Argument *pArg = funcArgs[idx];
      TDefSpaceMap::iterator space_it = m_GASDefSpaces.find(pArg);
      if (space_it != m_GASDefSpaces.end()) {
        assert(!space_it->second && "GAS Pointer collection is broken!");
        // This argument serves as a definition point - check if it's addr-space
        // is expected to be defined by function's caller(s).
        // The criteria is: the argument is an integer or a pointer of generic type
        PointerType *pPtrType = dyn_cast<PointerType>(pArg->getType());
        bool toBeDefinedByCaller = !pPtrType || IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace());
        // Note that GAS pointer arrays are effectively filtered-out as well, because they cannot be
        // generic (they are generated by 'alloca' and thus are of Private space).
        if (toBeDefinedByCaller) {
          assert(pIndices && "Function argument indices collection is broken!");
          // For defined-by-caller argument - add GAS argument index to
          // existing per-function "vector of indices" ...
          pIndices->push_back(idx);
          // ... and create new address space qualifier argument for the current function.
          // It will serve as a container for addr-space qualifier of GAS pointer.
          space_it->second = 
                   new Argument(Type::getInt32Ty(*m_pLLVMContext), GAS_ARG_NAME, pFunc);
        } else {
          // For argument which defines addr-space - resolve the definition point
          // with corresponding constant value of addr-space enumerator
          space_it->second = ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext), 
                                              pPtrType->getAddressSpace());
        }
        changed = true;
        continue;
      }
      TAllocaSpaceMap::iterator alloca_it = m_GASAllocaDefs.find(pArg);
      if (alloca_it != m_GASAllocaDefs.end()) {
        // If argument represents GAS pointer array - generate addr-space 
        // qualifier array argument and calculate its offset
        assert(pIndices && "Function argument indices collection is broken!");
        // For defined-by-caller argument - add GAS argument index to
        // existing per-function "vector of indices" ...
        pIndices->push_back(idx);
        // ... and create new address space qualifier argument for the current function.
        // It will serve as a container for addr-space qualifier of GAS pointer array.
        alloca_it->second = getArrayToSpaceOffset(pArg, pFunc->getEntryBlock().begin());
      }
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
            case Instruction::Load :
              // Load: create Load instruction from 'alloca' or array argument slot for
              // address space qualifier value
              changed |= resolveLoadInstr(space_it);
              break;
            case Instruction::Call : {
              // Call with GAS pointer return value: resolve to 'alloca' slot which will
              // be passed (by pointer) to the callee, and then assigned inside the callee
              Instruction *pFuncStart = pFunc->getEntryBlock().begin();
              Instruction *pAlocaInstr = new AllocaInst(Type::getInt32Ty(*m_pLLVMContext),
                                                        GAS_ALLOCA_NAME, pFuncStart);
              assocDebugLocWith(pAlocaInstr, pFuncStart);
              // We yet have to generate Load from that alloca slot
              Instruction *pNewInstr = new LoadInst(pAlocaInstr, GAS_INSTRUCTION_NAME);
              // Insert the new instruction immediately after Call
              pNewInstr->insertAfter(const_cast<Instruction*>(pInstr));
              assocDebugLocWith(pNewInstr, pInstr);
              // We intentionally assume Load to be addr-space modifier value (rather than 
              // the function parameter with addr-space qualifier itself), because this
              // is the value which will be used for resolution of downstream usages of GAS.
              // Still, resolution of the function call itself will use the pointer operand
              // of Load as a parameter.
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

    return changed;
  }

  bool GenericAddressDynamicResolution::resolveGASUsages(Function *pFunc) {

    bool changed = false;

    // Resolve all Store instructions which depend on 'Alloca'/argument GAS pointers 
    // used by Address Space Qualifier BIs
    if (m_GASAllocaStores.size()) {
      changed |= resolveStoreInstructions();
    }

    // Resolve Calls to Addr Space Qualifier BIs
    for (unsigned idx = 0; idx < m_AddrQualifierBICalls.size(); idx++) {
      // Inlining the function call to instructions which calculate the function result
      resolveAddrSpaceQualifierBICall(m_AddrQualifierBICalls[idx]);
      changed = true;
    }
    // Resolve BI function calls with GAS pointers
    for (unsigned idx = 0; idx < m_BiPoints.size(); idx++) {
     // Enforcing named address space to the function call
      resolveBIorIntrinsicCall(m_BiPoints[idx].first, CallBuiltIn, m_BiPoints[idx].second);
      changed = true;
    }
    // Resolve Intrinsic function calls with GAS pointers
    for (unsigned idx = 0; idx < m_IntrinPoints.size(); idx++) {
      // Enforcing named address space to the function call
      resolveBIorIntrinsicCall(m_IntrinPoints[idx].first, CallIntrinsic, m_IntrinPoints[idx].second);
      changed = true;
    }
    // Resolve non-kernel function calls with GAS pointers
    for (unsigned idx = 0; idx < m_NonKernelCalls.size(); idx++) {
      // Replacing original function call with the one extended with addr-space qualifier parameters
      resolveNonKernelFunctionCall(m_NonKernelCalls[idx]);
      changed = true;
    }
    // Resolve Ret instructions with GAS pointer return values
    if (m_ReturnGAS.size()) {
      changed |= resolveRetInstructions();
    }

    return changed;
  }

} // namespace intel
