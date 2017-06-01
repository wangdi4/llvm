/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "GenericAddressStaticResolution.h"

#include <CompilationUtils.h>
#include <OCLPassSupport.h>
#include <NameMangleAPI.h>
#include <FunctionDescriptor.h>
#include <ParameterType.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Operator.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/ValueMap.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Transforms/Utils/ValueMapper.h>
#include <assert.h>


using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend::Passes::GenericAddressSpace;

namespace intel {

  Value *GenericAddressStaticResolution::getReplacementForInstr(Instruction *pInstr) {

    TReplaceMap::const_iterator mapping = m_replaceMap.find(pInstr);
    if (mapping == m_replaceMap.end()) {
      // Source GAS pointer is unresolved - nothing to do here
      return NULL;
    } else {
      return mapping->second;
    }
  }

  Value *GenericAddressStaticResolution::getResolvedOperand(Value *pOperand,
                                                            OCLAddressSpace::spaces space) {
    // At first - try to find a replacement for the operand
    Value *pResolvedValue = getReplacementForInstr(dyn_cast<Instruction>(pOperand));
    if (pResolvedValue) {
      return pResolvedValue;
    }
    // Then - check for ConstantExpr case for resolution
    pResolvedValue = resolveConstantExpression(pOperand, space);
    if (pResolvedValue) {
      return pResolvedValue;
    }
    return NULL;
  }

  void GenericAddressStaticResolution::fixUpPointerUsages(Instruction *pNewInstr, Instruction *pOldInstr) {
    SmallVector<Instruction*,16> uses;
    // Iterate through usages of original instruction
    for (Value::user_iterator user_it = pOldInstr->user_begin(),
                              user_end = pOldInstr->user_end();
                              user_it != user_end; user_it++) {
      Instruction *pUserInst = dyn_cast<Instruction>(*user_it);
      assert(pUserInst && "All uses of instruction should be instructions!");
      uses.push_back(pUserInst);
    }
    for (unsigned useIdx = 0; useIdx < uses.size(); useIdx++) {
      Instruction *pUse = uses[useIdx];
      // Check whether we should update this usage
      TPointerMap::const_iterator ptr_it = m_GASEstimate.find(pUse);
      if (ptr_it != m_GASEstimate.end()) {
        // Check whether we have to induce bitcast to GAS pointer
        if (IS_ADDR_SPACE_GENERIC(ptr_it->second)) {
          assert(pNewInstr && "pNewInstr is nullptr");
          const PointerType *pSrcType = cast<PointerType>(pNewInstr->getType());
          // Induce conversion from named type to generic one
          CastInst *pInducedBitcast = CastInst::CreatePointerCast(pNewInstr,
                                                PointerType::get(
                                                    pSrcType->getElementType(),
                                                    OCLAddressSpace::Generic),
                                                    pOldInstr->getName(), pOldInstr);
          // Replace uses of original instruction with bitcast
          pUse->replaceUsesOfWith(pOldInstr, pInducedBitcast);
        } else {
          // Check whether the use's replacement is already created
          // (i.e., there are several GAS pointers as operands)
          if (Value *repVal = getReplacementForInstr(pUse)) {
            Instruction *repInstr = dyn_cast<Instruction>(repVal);
            assert(repInstr && "Replacement info is broken!");
            // Update the use replacement if it is: PHI, Select, Icmp, Call
            switch (repInstr->getOpcode()) {
              case Instruction::PHI : {
                // For PHI - just add new edge
                PHINode *PhiInstr = cast<PHINode>(repInstr);
                PhiInstr->addIncoming(pNewInstr, pNewInstr->getParent());
                break;
              }
              case Instruction::Select :
              case Instruction::ICmp   :
              case Instruction::Call   : {
                // For Select, Icmp, Call - update corresponding operand(s)
                assert(pUse->getNumOperands() == repInstr->getNumOperands() &&
                       "Replacement info is broken!");
                for (unsigned idx = 0; idx < pUse->getNumOperands(); idx++) {
                  if (pUse->getOperand(idx) == pOldInstr) {
                    repInstr->setOperand(idx, pNewInstr);
                  }
                }
                break;
              }
              default:
                assert(0 && "Unexpected instruction with generic address space pointer");
                break;
            }
          }   // check multi-pointer operand case
        }     // check induced bitcast case
      }       // check if this usage need to be updated
    }         // iteration through usages
  }

  bool GenericAddressStaticResolution::resolveInstructionConvert(Instruction *pInstr, OCLAddressSpace::spaces space) {

    // Fetch source value
    Value *pPrevValue = pInstr->getOperand(0);
    const PointerType *pSrcType = dyn_cast<PointerType>(pPrevValue->getType());
    PointerType *pDestType = dyn_cast<PointerType>(pInstr->getType());

    if (pSrcType && pDestType) {
      assert((IS_ADDR_SPACE_GENERIC(pSrcType->getAddressSpace()) ||
              IS_ADDR_SPACE_GENERIC(pDestType->getAddressSpace())) &&
             "Cannot reach this point with named-to-named conversion!");
    }

    Instruction *pNewInstr = NULL;
    if (pDestType && IS_ADDR_SPACE_GENERIC(pDestType->getAddressSpace())) {
      // Original conversion is to GAS pointer:
      //   - produce BitCast/GEP for <named>-to-<named> out of <named/generic>-to-<generic>
      //                     OR
      //   - produce Int2Ptr for <int>-to<named> out of <int>-to<generic>

      // At first - try to find a resolved value for source operand
      if (Value *pNewPrevValue = getResolvedOperand(pPrevValue, space)) {
        pPrevValue = pNewPrevValue;
      }
      // Then - generate the instruction
      switch (pInstr->getOpcode()) {
        case Instruction::IntToPtr: {
          pNewInstr = new IntToPtrInst(pPrevValue,
                                       PointerType::get(pDestType->getElementType(), space),
                                       pInstr->getName(), pInstr);
          break;
        }
        case Instruction::AddrSpaceCast :
        case Instruction::BitCast       : {
          pNewInstr = CastInst::CreatePointerCast(pPrevValue,
                                      PointerType::get(pDestType->getElementType(), space),
                                      pInstr->getName(), pInstr);

          break;
        }
        case Instruction::GetElementPtr : {
          GetElementPtrInst *pGepInstr = cast<GetElementPtrInst>(pInstr);
          SmallVector<Value*, 8> idxList;
          for (GetElementPtrInst::const_op_iterator idx_it = pGepInstr->idx_begin(),
                                                    idx_end = pGepInstr->idx_end();
                                                    idx_it != idx_end; idx_it++) {
            idxList.push_back(*idx_it);
          }
          // [LLVM 3.8 UPGRADE] ToDo: Replace nullptr for pointer type with actual type
          // (not using type from pointer as this functionality is planned to be removed.
          GetElementPtrInst *pNewGEP =
                  GetElementPtrInst::Create(nullptr, pPrevValue, idxList,
                                            pInstr->getName(), pInstr);
          pNewGEP->setIsInBounds(pGepInstr->isInBounds());
          pNewInstr = pNewGEP;
          break;
        }
        default:
          assert(0 && "Unexpected instruction with generic address space pointer target");
          break;
      }
      // Fix-up pointer usages
      fixUpPointerUsages(pNewInstr, pInstr);
    } else if (pSrcType && IS_ADDR_SPACE_GENERIC(pSrcType->getAddressSpace())) {
      // Original bitcast is from generic to named: check validity of the conversion
      //                                            and produce named-to-named anyway
      // Original bitcast to from generic to integer: produce named-to-integer

      // At first - find a resolution for source value
      assert((pInstr->getOpcode() == Instruction::BitCast || pInstr->getOpcode() == Instruction::AddrSpaceCast) &&
             "Only Bitcast/AddrSpaceCast can convert pointer with named space to another named space or integer");
      pPrevValue = getResolvedOperand(pPrevValue, space);
      assert(pPrevValue && "Cannot reach this point without resolved value for GAS pointer!");
      // Then - generate the instruction
      const PointerType *pNewType = dyn_cast<PointerType>(pPrevValue->getType());
      assert(pNewType && "Replacement for pointer should be a pointer!");
      if (pDestType && pNewType->getAddressSpace() != pDestType->getAddressSpace()) {
        // Conflict between addr-space types of source and destination - report
        emitWarning("Illegal conversion from generic address space pointer",
                     pInstr, m_pModule, m_pLLVMContext);
      }
      pNewInstr = CastInst::CreatePointerCast(pPrevValue, pInstr->getType(), pInstr->getName(), pInstr);
    } else if (!pDestType || !IS_ADDR_SPACE_GENERIC(pDestType->getAddressSpace())) {
      // The only reasons for that case are:
      // - BitCast from named addr-space pointer to non-pointer
      // - Int2Ptr to named addr-space
      // In both case we just need to generate new instruction on the basis of replaced operand

      // At first - find a resolution for source value
      pPrevValue = getResolvedOperand(pPrevValue, space);
      assert(pPrevValue && "Cannot reach this point without resolved value for GAS pointer!");
      // Then - generate the instruction
      switch (pInstr->getOpcode()) {
        case Instruction::AddrSpaceCast :
        case Instruction::BitCast       :
          pNewInstr = CastInst::CreatePointerCast(pPrevValue, pInstr->getType(), pInstr->getName(), pInstr);
          break;
        case Instruction::IntToPtr:
          pNewInstr = new IntToPtrInst(pPrevValue, pInstr->getType(), pInstr->getName(), pInstr);
          break;
        default:
          assert(0 && "Unexpected instruction with named address space pointer or integer target!");
          break;
      }
    } else {
      assert(0 && "Unexpected convert instruction!");
    }

    // Schedule the original instruction for replacement
    m_replaceMap.insert(TMapPair(pInstr, pNewInstr));
    m_replaceVector.push_back(TMapPair(pInstr, pNewInstr));
    return true;
  }

  bool GenericAddressStaticResolution::resolveInstructionOnePointer(Instruction *pInstr, OCLAddressSpace::spaces space) {

    unsigned ptrOperandIdx = pInstr->getOpcode() == Instruction::Store? 1 : 0;
    const PointerType *pSrcType = dyn_cast<PointerType>(pInstr->getOperand(ptrOperandIdx)->getType());
    if (!pSrcType || !IS_ADDR_SPACE_GENERIC(pSrcType->getAddressSpace())) {
      assert(0 && "Only GAS pointer is expected here!");
    }

    // At first - finding preceding instruction (original and its resolution)
    Value *pPrevValue = pInstr->getOperand(ptrOperandIdx);
    Value *pNewValue = getResolvedOperand(pPrevValue, space);
    assert(pNewValue && "Cannot reach this point without resolved value for GAS pointer!");
    // Then - generate the instruction
    Instruction *pNewInstr = NULL;
    switch (pInstr->getOpcode()) {
      case Instruction::Load : {
        LoadInst *pLoadInstr = cast<LoadInst>(pInstr);
        LoadInst *pNewLoad = new LoadInst(pNewValue, pLoadInstr->getName(),
                                    pLoadInstr->isVolatile(),  pLoadInstr->getAlignment(),
                                    pLoadInstr->getOrdering(), pLoadInstr->getSynchScope(),
                                    pLoadInstr);
        pNewInstr = pNewLoad;
        break;
      }
      case Instruction::Store : {
        StoreInst *pStoreInstr = cast<StoreInst>(pInstr);
        StoreInst *pNewStore = new StoreInst(pStoreInstr->getValueOperand(), pNewValue,
                                    pStoreInstr->isVolatile(),  pStoreInstr->getAlignment(),
                                    pStoreInstr->getOrdering(), pStoreInstr->getSynchScope(),
                                    pStoreInstr);
        pNewInstr = pNewStore;
        break;
      }
      case Instruction::AtomicCmpXchg : {
        AtomicCmpXchgInst *pCmpXchg = cast<AtomicCmpXchgInst>(pInstr);
        AtomicCmpXchgInst *pNewCmpXchg = new AtomicCmpXchgInst(
                                          pNewValue, pCmpXchg->getCompareOperand(), pCmpXchg->getNewValOperand(),
                                          pCmpXchg->getSuccessOrdering(), pCmpXchg->getFailureOrdering(),
                                          pCmpXchg->getSynchScope(), pCmpXchg);
        pNewCmpXchg->setVolatile(pCmpXchg->isVolatile());
        pNewInstr = pNewCmpXchg;
        break;
      }
      case Instruction::AtomicRMW : {
        AtomicRMWInst *pAtomicRMW = cast<AtomicRMWInst>(pInstr);
        AtomicRMWInst *pNewAtomicRMW = new AtomicRMWInst(
                                        pAtomicRMW->getOperation(), pNewValue,
                                        pAtomicRMW->getValOperand(),pAtomicRMW->getOrdering(),
                                        pAtomicRMW->getSynchScope(), pAtomicRMW);
        pNewAtomicRMW->setVolatile(pAtomicRMW->isVolatile());
        pNewInstr = pNewAtomicRMW;
        break;
      }
      case Instruction::PtrToInt : {
        pNewInstr = new PtrToIntInst(pNewValue, pInstr->getType(), pInstr->getName(), pInstr);
        break;
      }
      default:
        assert(0 && "Unexpected instruction with generic address space pointer");
        break;
    }
    // Schedule the original instruction for replacement
    m_replaceMap.insert(TMapPair(pInstr, pNewInstr));
    m_replaceVector.push_back(TMapPair(pInstr, pNewInstr));
    return true;
  }

  bool GenericAddressStaticResolution::resolveInstructionPhiNode(PHINode *pPhiInstr, OCLAddressSpace::spaces space) {

    const PointerType *pDestType = dyn_cast<PointerType>(pPhiInstr->getType());
    if (!pDestType) {
      // Nothing to resolve for integer PHI - it served only as transit
      // node for propagation of integer-out-of-GAS-pointer
      return false;
    }
    assert(IS_ADDR_SPACE_GENERIC(pDestType->getAddressSpace()) &&
           "Only GAS pointer is expected here!");

    // At first - create empty PHI node of required type
    unsigned numIncoming = pPhiInstr->getNumIncomingValues();
    PHINode *pNewPHI = PHINode::Create(
                          PointerType::get(pDestType->getElementType(), space),
                          numIncoming, pPhiInstr->getName(), pPhiInstr);
    // Then - add all incoming edges which can be resolved
    for (unsigned idx = 0; idx < numIncoming; idx++) {
      Value *pPrevValue = pPhiInstr->getIncomingValue(idx);
      if (Value *pNewVal = getResolvedOperand(pPrevValue, space)) {
        pNewPHI->addIncoming(pNewVal, pPhiInstr->getIncomingBlock(idx));
      }
    }
    assert(pPhiInstr->getNumIncomingValues() &&
           "Cannot reach this point without resolved value for at least one incoming GAS pointer!");
    // Fix-up pointer usages
    fixUpPointerUsages(pNewPHI, pPhiInstr);
    // Schedule the original instruction for replacement
    m_replaceMap.insert(TMapPair(pPhiInstr, pNewPHI));
    m_replaceVector.push_back(TMapPair(pPhiInstr, pNewPHI));
    return true;
  }

  bool GenericAddressStaticResolution::resolveInstructionTwoPointers(Instruction *pInstr, OCLAddressSpace::spaces space) {

    unsigned firstIdx = pInstr->getOpcode() == Instruction::ICmp? 0 : 1;
    const PointerType *pSrcType = dyn_cast<PointerType>(pInstr->getOperand(firstIdx)->getType());
    if (!pSrcType) {
      // Nothing to resolve for integer Select/ICmp - it served only as transit
      // node for propagation of integer-out-of-GAS-pointer
      return false;
    }

    assert(IS_ADDR_SPACE_GENERIC(pSrcType->getAddressSpace()) &&
           "Only GAS pointer is expected here!");

    // At first - check available resolution(s) and complement them by NULL pointer if required
    Constant *pNull = ConstantPointerNull::get(PointerType::get(pSrcType->getElementType(), space));
    Value *pVal[2] = {pNull, pNull};
    unsigned count = 0;
    for (unsigned idx = 0; idx < 2; idx++) {
      Value *pPrevValue = pInstr->getOperand(firstIdx + idx);
      if (Value *pNewVal = getResolvedOperand(pPrevValue, space)) {
        pVal[idx] = pNewVal;
        count++;
      }
    }
    assert(count &&
           "Cannot reach this point without resolved value for at least one incoming GAS pointer!");
    // Then - generate the instruction on basis of input values above
    Instruction *pNewInstr = NULL;
    switch (pInstr->getOpcode()) {
      case Instruction::Select : {
        SelectInst *pSelectInstr = cast<SelectInst>(pInstr);
        SelectInst *pNewSelect = SelectInst::Create(pSelectInstr->getCondition(),
                                                    pVal[0], pVal[1], pSelectInstr->getName(),
                                                    pSelectInstr);
        pNewInstr = pNewSelect;
        // Fix-up pointer usages
        fixUpPointerUsages(pNewSelect, pInstr);
        break;
      }
      case Instruction::ICmp : {
        ICmpInst *pIcmpInstr = cast<ICmpInst>(pInstr);
        ICmpInst *pNewICmp = new ICmpInst(pInstr, pIcmpInstr->getPredicate(), pVal[0], pVal[1],
                                          pInstr->getName());
        pNewInstr = pNewICmp;
        break;
      }
      default:
        assert(0 && "Unexpected instruction with generic address space pointer");
        break;
    }
    // Schedule the original instruction for replacement
    m_replaceMap.insert(TMapPair(pInstr, pNewInstr));
    m_replaceVector.push_back(TMapPair(pInstr, pNewInstr));
    return true;
  }

  bool GenericAddressStaticResolution::resolveInstructionCall(CallInst *pCallInstr, TFunctionList::iterator curFuncIt) {

    // Act separately for different kinds of callee
    const Function *pCallee = pCallInstr->getCalledFunction();
    if (!pCallee) {
      // Nothing to do with indirect call
      return false;
    }
    // Analyze callee of direct call
    if (isAddressQualifierBI(pCallee)) {
      // Replace Address Space Qualifier BI call with constant value
      foldAddressQualifierCall(pCallInstr);
      return true;
    } else if (isGenericAddrBI(pCallee)) {
      // Replace call to BI using generic address space, with corresponding BI
      // using named address space
      if (resolveFunctionCall(pCallInstr, CallBuiltIn)) {
        return true;
      }
    } else if (pCallee->isIntrinsic()) {
      // Replace call to intrinsic using generic address space, with that using
      // named address space
      if (resolveFunctionCall(pCallInstr, CallIntrinsic)) {
        return true;
      }
    } else if (!pCallee->isDeclaration()) {
      // This is a call to a non-kernel function - specialize it
      // and account the specialized version of non-kernel function for
      // GAS pointers which it may have
      Function *specializedCallee = resolveFunctionCall(pCallInstr, CallNonKernel);
      if (specializedCallee) {
        // Insert the function to the function-list (just after the function being processed)
        m_functionsToHandle.insert(++curFuncIt, specializedCallee);
        return true;
      }
    } else {
      // This is a call to a non-BI function which is only declared in the module
      assert(0 && "Discovered a non-BI function which is not defined in the module!");
      // We cannot process this call: leave it unresolved
      return false;
    }
    return false;
  }

  void GenericAddressStaticResolution::foldAddressQualifierCall(CallInst *pCallInstr) {

    // Fetching origin's named address space type
    TPointerMap::const_iterator ptr_it = m_GASEstimate.find(pCallInstr);
    assert(ptr_it != m_GASEstimate.end() && !IS_ADDR_SPACE_GENERIC(ptr_it->second) &&
      "Parameter of Address Space Qualifier function should be of named-space pointer type");
    OCLAddressSpace::spaces assignedSpace = ptr_it->second;

    // Folded bitcast/constant should be produced out of resolved value!
    Value *pPrevVal = pCallInstr->getArgOperand(0);
    Value *pNewVal = getResolvedOperand(pPrevVal, assignedSpace);
    if (!pNewVal) {
      assert(0 && "Resolution must be possible at this point!");
    }
    assert(dyn_cast<PointerType>(pNewVal->getType()) &&
           "Parameter of Address Space Qualifier function should be a pointer!");

    Value *pFolded = getFoldedAddrSpaceCall(pCallInstr, pNewVal);

    // Schedule the original instruction for replacement
    m_replaceMap.insert(TMapPair(pCallInstr, pFolded));
    m_replaceVector.push_back(TMapPair(pCallInstr, pFolded));
  }

  Function *GenericAddressStaticResolution::resolveFunctionCall(CallInst *pCallInstr, FuncCallType category) {

    unsigned numArgs = pCallInstr->getNumArgOperands();
    Function *pCallee = pCallInstr->getCalledFunction();
    assert(pCallee && "Call instruction doesn't have a callee!");
    std::string funcName = pCallee->getName().str();
    using namespace Intel::OpenCL::DeviceBackend;
    if (CompilationUtils::isPipeBuiltin(funcName)) return nullptr;
    assert((category != CallBuiltIn || isMangledName(funcName.c_str())) &&
           "Overloaded BI name should be mangled!");

    // At first - produce argument & parameter lists upon resolved values
    SmallVector<Type*,  8>                  argTypes;
    SmallVector<Value*, 8>                  params;
    SmallVector<OCLAddressSpace::spaces, 8> spaces;
    bool isGasPointersOnly = true;
    for (unsigned idx = 0; idx < numArgs; idx++) {
      Value *pArg = pCallInstr->getArgOperand(idx);
      // Check whether we should update this parameter
      TPointerMap::const_iterator ptr_it = m_GASEstimate.find(dyn_cast<Instruction>(pArg));
      if (ptr_it == m_GASEstimate.end() || IS_ADDR_SPACE_GENERIC(ptr_it->second)) {
        // The parameter hasn't changed - copy from original
        argTypes.push_back(pArg->getType());
        params.push_back(pArg);
        spaces.push_back(OCLAddressSpace::Generic);
      } else {
        isGasPointersOnly = false;
        OCLAddressSpace::spaces assignedSpace = ptr_it->second;
        //   ---- Replace: -------
        // 1. Parameter type
        PointerType *pPtrType = dyn_cast<PointerType>(pArg->getType());
        if (pPtrType) {
          // Parameter is of pointer type - replace with its named-space version
          pPtrType = PointerType::get(pPtrType->getElementType(), assignedSpace);
          argTypes.push_back(pPtrType);
          spaces.push_back(assignedSpace);
        } else {
          // Otherwise - copy the type from original
          argTypes.push_back(pArg->getType());
          spaces.push_back(OCLAddressSpace::Generic);
        }
        // 2. Parameter value
        if (Value *pArgVal = getResolvedOperand(pArg, assignedSpace)) {
          // If resolved value is available - add it
          params.push_back(pArgVal);
        } else {
          // Otherwise - add dummy value of argument type
          if (pPtrType) {
            params.push_back(ConstantPointerNull::get(pPtrType));
          } else {
            params.push_back(pArg);
          }
        }
      }
    }
    assert(argTypes.size() == numArgs && "Argument types list is broken!");

    // Function call didn't change - do nothing
    if (isGasPointersOnly) {
      return NULL;
    }

    // Alert if LLVM intrinsic function has pointer parameters of heterogeneous address space
    if (category == CallIntrinsic) {
      bool isFirst     = true;
      bool hasMismatch = false;
      OCLAddressSpace::spaces foundSpace = OCLAddressSpace::Generic;
      for (unsigned idx = 0; idx < argTypes.size(); idx++) {
        if (const PointerType *pPtrType = dyn_cast<PointerType>(argTypes[idx])) {
          OCLAddressSpace::spaces curSpace =
                            (OCLAddressSpace::spaces) pPtrType->getAddressSpace();
          // Only named addr-space may be conflicting
          if (!IS_ADDR_SPACE_GENERIC(curSpace)) {
            if (isFirst) {
              foundSpace = curSpace;
              isFirst = false;
            } else if (curSpace != foundSpace) {
              hasMismatch = true;
            }
          }
        }
      }
      if (hasMismatch) {
        // Conflict between add-space names of different parameters - report
        emitWarning("Built-in or Intrinsic call with parameters of different address spaces",
                    pCallInstr, m_pModule, m_pLLVMContext);
      }
    }

    // Generate new function name:
    std::string newFuncName;
    if (category == CallBuiltIn) {
      // Either upon re-mangling of BI with resolved GAS parameter(s)
      newFuncName = getResolvedMangledName(funcName, spaces);
    } else if (category == CallNonKernel) {
      // Or upon custom rule: <original-name> + <pointer-arg-mangling>
      newFuncName = getSpecializedFunctionName(funcName, argTypes);
    }

    // Get or create function object
    Function *pNewFunc = NULL;
    if (category == CallIntrinsic) {
      SmallVector<Type*, 8> overloadableArgTypes;
      getIntrinsicOverload(pCallee, argTypes, overloadableArgTypes);
      pNewFunc = Intrinsic::getDeclaration(m_pModule, (Intrinsic::ID)pCallee->getIntrinsicID(),
                                           overloadableArgTypes);
    } else {
      FunctionType *pNewFuncType = FunctionType::get(pCallInstr->getType(), argTypes, false);
      pNewFunc = dyn_cast<Function>(m_pModule->getOrInsertFunction(newFuncName, pNewFuncType));
    }
    assert(pNewFunc && "Non-function object with the same signature identified in the module");
    pNewFunc->setAttributes(pCallee->getAttributes());
    pNewFunc->setLinkage(pCallee->getLinkage());
    pNewFunc->setCallingConv(pCallee->getCallingConv());

    // Generate function body of specialized non-kernel function
    if (category == CallNonKernel && pNewFunc->isDeclaration()) {
      // Clone original function into the new one
      ValueToValueMapTy VMap;
      for (Function::arg_iterator src_arg_it = pCallee->arg_begin(),
                                  src_arg_end = pCallee->arg_end(),
                                  new_arg_it = pNewFunc->arg_begin();
                                  src_arg_it != src_arg_end;
                                  src_arg_it++, new_arg_it++) {
        new_arg_it->setName(src_arg_it->getName());
        // Map original formal arguments to resolved ones
        VMap[&*src_arg_it] = &*new_arg_it;
      }
      SmallVector<ReturnInst*, 8> Returns;
      CloneFunctionInto(pNewFunc, pCallee, VMap, true, Returns);
      // Now - induce bitcasts between resolved arguments and GAS pointers
      // (still in use within the clone)
      for (Function::arg_iterator src_arg_it = pCallee->arg_begin(),
                                  src_arg_end = pCallee->arg_end(),
                                  new_arg_it = pNewFunc->arg_begin();
                                  src_arg_it != src_arg_end;
                                  src_arg_it++, new_arg_it++) {
        // Process only resolved arguments
        if (src_arg_it->getType() == new_arg_it->getType()) {
          continue;
        }
        PointerType *pOrigPtrType = dyn_cast<PointerType>(src_arg_it->getType());
        assert(pOrigPtrType && IS_ADDR_SPACE_GENERIC(pOrigPtrType->getAddressSpace()) &&
               "Argument of original function should be a GAS pointer!");
        // Produce and insert bitcast
        CastInst *pNewBitCast = CastInst::CreatePointerCast(&*new_arg_it, pOrigPtrType,
                                                   new_arg_it->getName(),
                                                   &*pNewFunc->getEntryBlock().begin());
        // Replace usages of the argument with those of bitcast
        SmallVector<Instruction*,16> uses;
        for (Value::user_iterator user_it = new_arg_it->user_begin(),
                                  user_end = new_arg_it->user_end();
                                  user_it != user_end; user_it++) {
          Instruction *pUserInst = dyn_cast<Instruction>(*user_it);
          if (pUserInst && pUserInst != pNewBitCast) {
            uses.push_back(pUserInst);
          }
        }
        for (unsigned idx = 0; idx < uses.size(); idx++) {
          uses[idx]->replaceUsesOfWith(&*new_arg_it, pNewBitCast);
        }
      }     // iteration through arguments
    }       // new function cloning

    // Generate replacement for Call instruction
    CallInst *pNewCall = CallInst::Create(pNewFunc, ArrayRef<Value*>(params),
                                          pCallInstr->getName(), pCallInstr);
    assert(pNewCall && "Couldn't create resolved CALL instruction!");
    pNewCall->setAttributes(pCallInstr->getAttributes());
    pNewCall->setCallingConv(pCallInstr->getCallingConv());

    // Schedule the original instruction for replacement
    m_replaceMap.insert(TMapPair(pCallInstr, pNewCall));
    m_replaceVector.push_back(TMapPair(pCallInstr, pNewCall));

    return pNewFunc;
  }

  Value *GenericAddressStaticResolution::resolveConstantExpression(Value *pVal, OCLAddressSpace::spaces space) {
    // Check that the operand produces a pointer
    if (PointerType *pPtrType = dyn_cast<PointerType>(pVal->getType())) {
      // If pointer is already of named addr-space type - nothing to do
      if (!IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace())) {
        return pVal;
      }
      // Check for constant expression
      if (ConstantExpr *pCE = dyn_cast<ConstantExpr>(pVal)) {
        // Generate new constant expression with resolved pointer's address space
        switch (pCE->getOpcode()) {
          case Instruction::IntToPtr :
            // IntToPtr case: enforce target addr space type on the result
            return ConstantExpr::getIntToPtr(pCE->getOperand(0),
                                        PointerType::get(pPtrType->getElementType(), space));
            break;
          case Instruction::AddrSpaceCast :
          case Instruction::BitCast       :
            // Bitcast case: modify BitCast expression towards target addr space type
            return ConstantExpr::getPointerCast(pCE->getOperand(0),
                                            PointerType::get(pPtrType->getElementType(), space));
            break;
          case Instruction::GetElementPtr : {
              // GEP case: modify Pointer operand to target addr space type and the constant result type
              SmallVector<Constant*, 8> operands;
              operands.push_back(cast<Constant>(resolveConstantExpression(pCE->getOperand(0), space)));
              for (unsigned idx = 1; idx < pCE->getNumOperands(); idx++) {
                operands.push_back(pCE->getOperand(idx));
              }
              auto *GEPO = cast<GEPOperator>(pCE);
              return ConstantExpr::getGetElementPtr(
                GEPO->getSourceElementType(), operands[0], ((ArrayRef<Constant*>)operands).slice(1),
                GEPO->isInBounds());
            }
            break;
          case Instruction::Select : {
              // Select case: modify Pointer operand to target addr space type and the constant result type
              SmallVector<Constant*, 8> operands;
              operands.push_back(pCE->getOperand(0));
              for (unsigned idx = 1; idx < pCE->getNumOperands(); idx++) {
                operands.push_back(cast<Constant>(resolveConstantExpression(pCE->getOperand(idx), space)));
              }
              return pCE->getWithOperands(operands,
                                          PointerType::get(pPtrType->getElementType(), space));
            }
            break;
          default:
            // A binary or bitwise expression cannot be reached here, because we enter the constant expression
            // with pointer value, and then stop on IntToPtr and Bitcast (who are the only ones which could
            // lead to integer type involved)
            assert(0 && "Unexpected instruction with generic address space constant expression pointer");
            return NULL;
        }
      } else {
        return NULL;
      }
    } else {
      return NULL;
    }
  }

} // namespace intel
