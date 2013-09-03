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

namespace intel {

  Constant *GenericAddressDynamicResolution::checkAndResolveConstantExpression(const ConstantExpr *pCE) {
    // Generate constant expression with address space qualifier value of the expression
    unsigned Opcode = pCE->getOpcode();
    switch (Opcode) {
      case Instruction::BitCast       :
      case Instruction::PtrToInt      :
      case Instruction::GetElementPtr : {
        // Unary operation which may have a pointer operand:
        const PointerType *pPtrType = dyn_cast<PointerType>(pCE->getOperand(0)->getType());
        if (pPtrType && !IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace())) {
          // If it is based on a named addr-space pointer - produce addr-space qualifier constant
          return ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext), 
                                  pPtrType->getAddressSpace());
        } else {
          // Otherwise - handle the constant (expression) operand
          return checkAndResolveConstantOperand(pCE->getOperand(0));
        }
        break;
      }
      case Instruction::IntToPtr :
        // IntToPtr - handle the constant (expression) operand
        return checkAndResolveConstantOperand(pCE->getOperand(0));
        break;
      case Instruction::Select :
        // Select: produce Select similar to this one, but out of address
        // space qualifier values, rather than from pointer values
        return ConstantExpr::getSelect(pCE->getOperand(0), 
                                       checkAndResolveConstantOperand(pCE->getOperand(1)),
                                       checkAndResolveConstantOperand(pCE->getOperand(2)));
        break;
      default:
        if (Instruction::isBinaryOp(Opcode)) {
          // Binary operation: produce binary operation similar to this one, but out of address
          // space qualifier values, rather than from pointer values
          return ConstantExpr::get(Opcode, checkAndResolveConstantOperand(pCE->getOperand(0)),
                                           checkAndResolveConstantOperand(pCE->getOperand(1)));
          break;
        }
        assert(0 && "Unexpected instruction with generic address space constant expression pointer");
        return NULL;
    }
  }

  Constant *GenericAddressDynamicResolution::checkAndResolveConstantOperand(const Constant *pOperand) {
    // Act upon the operand type:
    if (const ConstantExpr *pCE = dyn_cast<ConstantExpr>(pOperand)) {
      // Either dig into the operand if it's a constant expression
      return checkAndResolveConstantExpression(pCE);
    } else {
      // Or produce addr-space qualifier constant of generic type
      emitWarning("Uninitialized generic address space pointer", NULL,
                  m_pModule, m_pLLVMContext);
      return ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext),
                              OCLAddressSpace::Generic);
    }
  }

  bool GenericAddressDynamicResolution::tryQuickResolveSpaceDefinition(
                                     TPointerDefMap::iterator def_it, const Function *pFunc) {

    // If this GAS pointer remained unresolved (because it is uninitialized 
    // or is built out of integer which was never created out of pointer) - set 
    // its addr-space LLVM virtual register to "generic"
    // Note that we populate definition point with a dummy (integer zero) value,
    // because we won't have it either NULL or meaningful (zero integer value
    // NEVER would be a definition point for a pointer!)
    if (!def_it->second) {
      emitWarning("Uninitialized generic address space pointer", NULL,
                  m_pModule, m_pLLVMContext);
      def_it->second = ConstantInt::getNullValue(Type::getInt32Ty(*m_pLLVMContext));
      m_GASDefSpaces.insert(
            TDefSpacePair(def_it->second, 
                          ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext), 
                                           OCLAddressSpace::Generic)));
      return true;
    }

    // If this GAS pointer was already handled successfully - nothing to do with it
    TDefSpaceMap::const_iterator space_it = m_GASDefSpaces.find(def_it->second);
    assert(space_it != m_GASDefSpaces.end() && "GAS Pointer collection is broken!");
    if (space_it->second) {
      return true;
    }

    return false;
  }

  bool GenericAddressDynamicResolution::resolvePhiOrSelectInstr(TDefSpaceMap::iterator space_it) {

    Value *pVal = const_cast<Value*>(space_it->first);
    Instruction *pInstr = cast<Instruction>(pVal);

    // PHI/Select - check that all inputs are already resolved 
    // (due to unbalanced BFS the resolution may be deferred)
    bool isPHI = (pInstr->getOpcode() == Instruction::PHI);
    bool allInputsResolved = true;
    unsigned numInputs = pInstr->getNumOperands();
    unsigned firstIdx = isPHI? 0 : 1;
    for (unsigned idx = firstIdx; idx < numInputs; idx++) {
      if (!getAddrSpaceFor(pInstr->getOperand(idx))) {
        allInputsResolved = false;
        break;
      }
    }
    if (!allInputsResolved) {
      return false;
    }
    // All inputs are resolved: 
    // create PHI/Select instruction out of address space qualifier values
    Instruction *pNewInstr;
    if (isPHI) {
      PHINode *pPhiInstr = dyn_cast<PHINode>(pInstr);
      PHINode *pNewPHI = PHINode::Create(
                                    Type::getInt32Ty(*m_pLLVMContext),
                                    numInputs, GAS_INSTRUCTION_NAME);
      for (unsigned idx = 0; idx < numInputs; idx++) {
        pNewPHI->addIncoming(getAddrSpaceFor(
                                pPhiInstr->getIncomingValue(idx)),
                                pPhiInstr->getIncomingBlock(idx));
      }
      pNewInstr = pNewPHI;
    } else {
      SelectInst *pSelInstr = dyn_cast<SelectInst>(pInstr);
      pNewInstr = SelectInst::Create(pSelInstr->getCondition(), 
                                      getAddrSpaceFor(
                                          pSelInstr->getTrueValue()),
                                      getAddrSpaceFor(
                                          pSelInstr->getFalseValue()),
                                      GAS_INSTRUCTION_NAME);
    }
    pNewInstr->insertAfter(pInstr);
    assocDebugLocWith(pNewInstr, pInstr);
    space_it->second = pNewInstr;
    return true;
  }

  void GenericAddressDynamicResolution::resolveAddrSpaceQualifierBICall(CallInst *pCallInstr) {

    // Inlining the function call to instructions which calculate the function result
    // on the basis of addr-space qualifier LLVM virtual register for the GAS pointer
    // used by this call instruction
    Value *pSpaceVal = getAddrSpaceFor(pCallInstr->getArgOperand(0));
    assert(pSpaceVal && "GAS pointers should be resolved at this point!");

    // Fetching function to inline
    const Function *pCallee = pCallInstr->getCalledFunction();
    assert(pCallee && "Call instruction doesn't have a callee!");
    StringRef funcName = pCallee->getName();
    std::string tmp = funcName.str();
    const char *funcNameStr = tmp.c_str();
    funcName = isMangledName(funcNameStr)? stripName(funcNameStr) : funcName;

    Instruction *pRetVal = NULL;
    if (funcName == "get_fence") {
      // Inlining "get_fence" BI function:
      //    pCondGlobal   = icmp eq i32 <space> OCLAddressSpace::Global
      //    pCondLocal    = icmp eq i32 <space> OCLAddressSpace::Local
      //    pSelectGlobal = select i1 pCondGlobal, i32 OCLAddressSpace::Global, 0
      //    pRetVal       = select i1 pCondLocal, i32 OCLAddressSpace::Local, pSelectGlobal
      Instruction *pCondGlobal =
              new ICmpInst(pCallInstr, ICmpInst::ICMP_EQ, pSpaceVal,
                            ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext),
                                            OCLAddressSpace::Global),
                            GAS_INSTRUCTION_NAME);
      Instruction *pCondLocal =
              new ICmpInst(pCallInstr, ICmpInst::ICMP_EQ, pSpaceVal,
                            ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext),
                                            OCLAddressSpace::Local),
                            GAS_INSTRUCTION_NAME);
      Instruction *pSelectGlobal =
              SelectInst::Create(
                        pCondGlobal,
                        ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext),
                                          CLK_GLOBAL_MEM_FENCE),
                        ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext), 0),
                        GAS_INSTRUCTION_NAME, pCallInstr);
      pRetVal = SelectInst::Create(
                              pCondLocal,
                              ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext),
                                              CLK_LOCAL_MEM_FENCE),
                              pSelectGlobal, GAS_INSTRUCTION_NAME, pCallInstr);
      // Adjusting line-info of inlined code
      assocDebugLocWith(pCondGlobal, pCallInstr);
      assocDebugLocWith(pCondLocal, pCallInstr);
      assocDebugLocWith(pSelectGlobal, pCallInstr);
      assocDebugLocWith(pRetVal, pCallInstr);
    } else {
      // Inlining "is_<target-space>" BI functions:
      //       pRes = icmp eq i32 <space> <target-space>
      // Calculating <target-space>
      int targetSpace = OCLAddressSpace::Generic;
      if (funcName == "is_global") {
        targetSpace = OCLAddressSpace::Global;
      } else if (funcName == "is_local") {
        targetSpace = OCLAddressSpace::Local;
      } else if (funcName == "is_private") {
        targetSpace = OCLAddressSpace::Private;
      } else {
        assert(0 && "Unknown Address Space Qualifier BI function");
      }
      pRetVal = new ICmpInst(pCallInstr, ICmpInst::ICMP_EQ, pSpaceVal,
                              ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext),
                                              targetSpace),
                              GAS_INSTRUCTION_NAME);
      assocDebugLocWith(pRetVal, pCallInstr);
    }
    // Replacing original call with inlined code
    pCallInstr->replaceAllUsesWith(pRetVal);
    pCallInstr->eraseFromParent();
  }

  void GenericAddressDynamicResolution::resolveBIorIntrinsicCall(
          CallInst *pCallInstr, FuncCallType category, OCLAddressSpace::spaces targetSpace) {

    // Converting the function call to that with all pointer parameters promoted to
    // target address space type, together with induction of BitCast for every 
    // promoted parameter
    assert((category == CallBuiltIn || category == CallIntrinsic) && "Unexpected function category!");
    // GAS pointer cannot be returned from a BI or Intrinsic function. This is because we
    // cannot access its IR (as only function declaration is available in this module's IR).
    // Thus, we cannot instrument it with add-space qualifier book-keeping.
    const PointerType *pRetPtrType = dyn_cast<PointerType>(pCallInstr->getType());
    if (pRetPtrType && IS_ADDR_SPACE_GENERIC(pRetPtrType->getAddressSpace())) {
      assert(0 && "Generic address space pointer cannot be returned from a BI or Intrinsic function");
    }

    unsigned numArgs = pCallInstr->getNumArgOperands();
    Function *pCallee = pCallInstr->getCalledFunction();
    assert(pCallee && "Call instruction doesn't have a callee!");
    std::string funcName = pCallee->getName().str();
    assert((category != CallBuiltIn || isMangledName(funcName.c_str())) && "Overloaded BI name should be mangled!");

    // At first - produce argument type & parameter lists towards target address space
    SmallVector<Type*,  8>                  argTypes;
    SmallVector<Value*, 8>                  params;
    SmallVector<OCLAddressSpace::spaces, 8> resolvedSpaces;
    SmallVector<OCLAddressSpace::spaces, 8> originalSpaces;
    for (unsigned idx = 0; idx < numArgs; idx++) {
      Value *pArg = pCallInstr->getArgOperand(idx);
      // Check whether we should update this parameter
      PointerType *pPtrType = dyn_cast<PointerType>(pArg->getType());
      if (pPtrType && pPtrType->getAddressSpace() != (unsigned)targetSpace) {
        // Parameter is of pointer type and differs from targeted address space:
        // it and its type should be replaced
        OCLAddressSpace::spaces originalSpace = 
                      (OCLAddressSpace::spaces) pPtrType->getAddressSpace();
        pPtrType = PointerType::get(pPtrType->getElementType(), targetSpace); 
        argTypes.push_back(pPtrType);
        // Induce conversion from original parameter to that of target type
        BitCastInst *pInducedBitcast = new BitCastInst(pArg, pPtrType,
                                                       GAS_INSTRUCTION_NAME, pCallInstr);
        assocDebugLocWith(pInducedBitcast, pCallInstr);
        params.push_back(pInducedBitcast);
        resolvedSpaces.push_back(targetSpace);
        originalSpaces.push_back(originalSpace); 
      } else {
        argTypes.push_back(pArg->getType());
        params.push_back(pArg);
        resolvedSpaces.push_back(OCLAddressSpace::Generic);
        originalSpaces.push_back(OCLAddressSpace::Generic);
      }
    }

    // Then - get or create target function object
    Function *pNewFunc = NULL;
    if (category == CallIntrinsic) {
      SmallVector<Type*, 8> overloadableArgTypes;
      getIntrinsicOverload(pCallee, argTypes, overloadableArgTypes);
      pNewFunc = Intrinsic::getDeclaration(m_pModule, (Intrinsic::ID)pCallee->getIntrinsicID(),
                                           overloadableArgTypes);
    } else {
      std::string newFuncName = getResolvedMangledName(funcName, resolvedSpaces, &originalSpaces);
      FunctionType *pNewFuncType = FunctionType::get(pCallInstr->getType(), argTypes, false);
      pNewFunc = dyn_cast<Function>(m_pModule->getOrInsertFunction(newFuncName, pNewFuncType));
    }
    assert(pNewFunc && "Non-function object with the same signature identified in the module");
    pNewFunc->setAttributes(pCallee->getAttributes());
    pNewFunc->setLinkage(pCallee->getLinkage());
    pNewFunc->setCallingConv(pCallee->getCallingConv());

    // And finally - generate replacement for Call instruction    
    CallInst *pNewCall = CallInst::Create(pNewFunc, ArrayRef<Value*>(params), "", pCallInstr);
    assert(pNewCall && "Couldn't create resolved CALL instruction!");
    pNewCall->setAttributes(pCallInstr->getAttributes());
    pNewCall->setCallingConv(pCallInstr->getCallingConv());
    // Replacing the original call instruction
    assocDebugLocWith(pNewCall, pCallInstr);
    pCallInstr->replaceAllUsesWith(pNewCall);
    pCallInstr->eraseFromParent();

  }

  void GenericAddressDynamicResolution::resolveNonKernelFunctionCall(CallInst *pCallInstr) {

    // Replacing original function call with the one modified in the following way:
    // it has additional parameters (addr-space qualifier LLVM virtual registers) - one
    // for each GAS pointer used as the callee function argument and appearing in the 
    // function's "vector of indices".

    unsigned numArgs = pCallInstr->getNumArgOperands();
    Function *pCallee = pCallInstr->getCalledFunction();
    assert(pCallee && "Call instruction doesn't have a callee!");

    // Produce parameter list with appended addr-space qualifier values
    SmallVector<Value*, 8> params;
    TGASFuncMap::const_iterator func_it = m_GASFunctions.find(pCallee);
    assert(func_it != m_GASFunctions.end() && 
            "Callee function GAS argument collection is broken!");
    const SmallVector<unsigned, 8> &indices = func_it->second;
    for (unsigned idx = 0; idx < numArgs; idx++) {
      params.push_back(pCallInstr->getArgOperand(idx));
    }
    // Appending "must be resolved" arguments' addr-space qualifier values
    for (unsigned idx = 0; idx < indices.size(); idx++) {
      Value *pArg = pCallInstr->getArgOperand(indices[idx]);
      Type *pArgType = pArg->getType();
      Value *pSpaceVal = NULL;
      if (isGASPtrArray(pArgType)) {
        // This is a GAS array 'alloca'/argument - locate its addr-space qualifier value
        TAllocaUsageMap::const_iterator alloca_it = m_GASAllocaUsages.find(pArg);
        assert(alloca_it != m_GASAllocaUsages.end() && "GAS pointer array collection is broken!");
        Value *pGASArray = const_cast<Value*>(alloca_it->second);
        TAllocaSpaceMap::iterator def_it = m_GASAllocaDefs.find(pGASArray);
        assert(def_it != m_GASAllocaDefs.end() && "GAS pointer array collection is broken!");
        SpaceArrayDef spaceDef = def_it->second;
        assert(spaceDef.pSpace && "GAS pointer array collection is broken!");
        pSpaceVal = spaceDef.pSpace;
      } else {        
        pSpaceVal = getAddrSpaceFor(pArg);
        assert(pSpaceVal && "GAS pointers should be resolved at this point!");
      }
      params.push_back(pSpaceVal);
    }
    // Append argument pointing to addr-space qualifier of GAS pointer return value (if any)
    if (PointerType *pPtrType = dyn_cast<PointerType>(pCallee->getReturnType())) {
      if (IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace())) {
        TDefSpaceMap::iterator space_it = m_GASDefSpaces.find(pCallInstr);
        if (space_it != m_GASDefSpaces.end()) {
          // The return value is in use in function, and thus it has definition - append it's qualifier
          LoadInst *pSpaceVal = dyn_cast<LoadInst>(space_it->second);
          assert(pSpaceVal && "Collection of GAS pointers is broken!");
          AllocaInst *pSpaceRetByValParam = dyn_cast<AllocaInst>(pSpaceVal->getPointerOperand());
          assert(pSpaceRetByValParam && "Function return value addr-space modifier param should be 'alloca'!");
          params.push_back(pSpaceRetByValParam);
        } else {
          // The return value is not used in function - append dummy parameter
          Instruction *pFuncStart = pCallInstr->getParent()->getParent()->getEntryBlock().begin();
          AllocaInst *pDummyParam = new AllocaInst(Type::getInt32Ty(*m_pLLVMContext),
                                                   GAS_ALLOCA_NAME, pFuncStart);
          assocDebugLocWith(pDummyParam, pFuncStart);
          params.push_back(pDummyParam);
        }
      }
    }

    // Generate replacement for Call instruction   
    CallInst *pNewCall = CallInst::Create(pCallee, ArrayRef<Value*>(params), "", pCallInstr);
    assert(pNewCall && "Couldn't create resolved CALL instruction!");
    pNewCall->setAttributes(pCallInstr->getAttributes());
    pNewCall->setCallingConv(pCallInstr->getCallingConv());
    // Replacing the original call instruction
    assocDebugLocWith(pNewCall, pCallInstr);
    pCallInstr->replaceAllUsesWith(pNewCall);
    pCallInstr->eraseFromParent();
  }

  Function *GenericAddressDynamicResolution::cloneResolvedFunction(Function *pFunc) {

    // At first - build function clone's name
    // We build mangled name out of function's original name and argument type list
    std::string funcName = pFunc->getName().str();
    bool isMangled = isMangledName(funcName.c_str());

    // Produce argument type list with appended addr-space qualifier values
    FunctionType *pFuncType = pFunc->getFunctionType();
    SmallVector<Type*, 8> argTypes;
    TGASFuncMap::iterator func_it = m_GASFunctions.find(pFunc);
    assert(func_it != m_GASFunctions.end() && 
            "Callee function GAS argument collection is broken!");
    SmallVector<unsigned, 8> indices = func_it->second;
    // Original argument type list
    for (unsigned idx = 0; idx < pFuncType->getNumParams(); idx++) {
      argTypes.push_back(pFuncType->getParamType(idx));
    }
    // Appending "must be resolved" arguments' addr-space qualifier values
    for (unsigned idx = 0; idx < indices.size(); idx++) {
      Type *pArgType = pFuncType->getParamType(indices[idx]);
      if (isGASPtrArray(pArgType)) {
        // Array of addr-space qualifier values - the type is <size-of-void*>*
        argTypes.push_back(m_PointerSlotType->getPointerTo());
      } else {
        // Scalar - the type is i32
        argTypes.push_back(Type::getInt32Ty(*m_pLLVMContext));
      }
    }
    // For function which returns GAS pointer - append pointer argument with 
    // addr-space qualifier for return value
    if (m_ReturnGAS.size()) {
      argTypes.push_back(Type::getInt32PtrTy(*m_pLLVMContext));
    }

    if (isMangled) {
      // If the function name is already mangled - enforce uniqueness of
      // mangled name by appending of a dummy pointer type to the argument type list
      funcName = stripName(funcName.c_str()).str();
      argTypes.push_back(Type::getInt32PtrTy(*m_pLLVMContext, OCLAddressSpace::Generic));
    }
    std::string newFuncName = getSpecializedFunctionName(funcName, argTypes);

    // Strip argument type list of dummy pointer type
    if (isMangled) {
      argTypes.pop_back();
    }

    // Then - build the clone      
    FunctionType *pNewFuncType = FunctionType::get(pFunc->getReturnType(), argTypes, false);
    Function *pNewFunc = dyn_cast<Function>(m_pModule->getOrInsertFunction(newFuncName, pNewFuncType));
    assert(pNewFunc && "Non-function object with the same signature identified in the module");
    pNewFunc->setAttributes(pFunc->getAttributes());
    pNewFunc->setLinkage(pFunc->getLinkage());
    pNewFunc->setCallingConv(pFunc->getCallingConv());

    // Clone original function into the new one
    ValueToValueMapTy VMap;
    for (Function::arg_iterator src_arg_it = pFunc->arg_begin(),
                                src_arg_end = pFunc->arg_end(),
                                new_arg_it = pNewFunc->arg_begin();
                                src_arg_it != src_arg_end; 
                                src_arg_it++, new_arg_it++) {
      new_arg_it->setName(src_arg_it->getName());
      // Map original formal arguments to resolved ones 
      VMap[src_arg_it] = new_arg_it;
    }
    SmallVector<ReturnInst*, 8> Returns;
    CloneFunctionInto(pNewFunc, pFunc, VMap, false, Returns);

    // Replace all references to the original function with those to the clone
    SmallVector<User*, 16> uses;
    for (Value::use_iterator use_it = pFunc->use_begin(),
                             use_end = pFunc->use_end(); 
                             use_it != use_end; use_it++) {
      // We handle all references to the function: both direct and indirect 
      User *pUse = dyn_cast<User>(*use_it);
      uses.push_back(pUse);
    }
    for (unsigned idx = 0; idx < uses.size(); idx++) {
      User *pUse = uses[idx];
      if (CallInst *pCallInstr = dyn_cast<CallInst>(pUse)) {
        // Explicit replacement of callee
        pCallInstr->setCalledFunction(pNewFunc);
      } else {
        // Replacement of usage 
        pUse->replaceUsesOfWith(pFunc, pNewFunc);
      }
    }
    assert(pFunc->hasNUses(0) && "Function should not have uses anymore!");

    // Replace original function in the map of "vector of indices" with the clone
    m_GASFunctions.erase(func_it);
    m_GASFunctions.insert(TGASFuncPair(pNewFunc, indices));

    return pNewFunc;
  }

  bool GenericAddressDynamicResolution::resolveLoadInstr(TDefSpaceMap::iterator space_it) {

    Value *pVal = const_cast<Value*>(space_it->first);
    LoadInst *pLoadInstr = cast<LoadInst>(pVal);

    TAllocaUsageMap::const_iterator alloca_it = m_GASAllocaUsages.find(pLoadInstr);
    assert(alloca_it != m_GASAllocaUsages.end() && "GAS pointer array collection is broken!");
    Value *pGASArray = const_cast<Value*>(alloca_it->second);
    TAllocaSpaceMap::iterator def_it = m_GASAllocaDefs.find(pGASArray);
    assert(def_it != m_GASAllocaDefs.end() && "GAS pointer array collection is broken!");
    SpaceArrayDef spaceDef = def_it->second;
    assert(spaceDef.pSpace && "GAS pointer array collection is broken!");

    Instruction *pNewInstr = NULL;
    AllocaInst *pAllocaInstr = dyn_cast<AllocaInst>(pGASArray);
    bool isScalarAlloc = pAllocaInstr && !pAllocaInstr->getAllocatedType()->isArrayTy() && 
                         !pAllocaInstr->isArrayAllocation();
    if (pLoadInstr->getPointerOperand() == pGASArray || isScalarAlloc) {
      // If this is about a scalar (using immediate 'alloca'/array-argument address as a pointer - with no GEP
      // in between): produce Load of space qualifier directly from 'alloca'/argument of space qualifier
      pNewInstr = new LoadInst(spaceDef.pSpace, GAS_INSTRUCTION_NAME);
      pNewInstr->insertAfter(pLoadInstr);
    } else {
      // Common case: produce Load from an array of addr-space qualifiers
      // Build source pointer to a slot within 'alloca'/argument of space qualifiers' array
      assert(spaceDef.pOffset && "GAS pointer array collection is broken!");
      Instruction *pSpaceSlotPtr = mapArrayPtrToSpacePtr(pLoadInstr->getPointerOperand(), 
                                                         spaceDef.pOffset, pLoadInstr);
      // Build instruction
      pNewInstr = new LoadInst(pSpaceSlotPtr, GAS_INSTRUCTION_NAME);
      // Insert the new instruction immediately after original Load
      pNewInstr->insertAfter(pSpaceSlotPtr);
    }
    assocDebugLocWith(pNewInstr, pLoadInstr);
    space_it->second = pNewInstr;
    return true;
  }

  bool GenericAddressDynamicResolution::resolveStoreInstructions() {

    bool changed = false;
    // Iterate through 'Alloca'/array-argument GAS pointers arrays and then through Store
    // instructions of each of them
    for (unsigned idx = 0; idx < m_GASAllocaVector.size(); idx++) {
      Value *pGASArray = m_GASAllocaVector[idx];
      assert(pGASArray && "GAS pointer array collection is broken!");
      TAllocaSpaceMap::iterator def_it = m_GASAllocaDefs.find(pGASArray);
      assert(def_it != m_GASAllocaDefs.end() &&
             "Collection of GAS pointer arrays is broken!");
      SpaceArrayDef spaceDef = def_it->second;
      assert(spaceDef.pSpace && "GAS pointer array collection is broken!");

      TAllocaStoresMap::const_iterator store_it = m_GASAllocaStores.find(pGASArray);
      if (store_it != m_GASAllocaStores.end()) {
        // There are Stores to this array
        for (unsigned idx = 0; idx < store_it->second.size(); idx++) {
          // Produce instruction for store of addr-space qualifier LLVM virtual
          // register for the GAS pointer used by this Store instruction
          StoreInst *pStoreInstr = store_it->second[idx];
          Value *pSpaceVal = getAddrSpaceFor(pStoreInstr->getValueOperand());
          assert(pSpaceVal && "GAS pointers should be resolved at this point!");
          Instruction *pNewInstr = NULL;
          AllocaInst *pAllocaInstr = dyn_cast<AllocaInst>(pGASArray);
          bool isScalarAlloc = pAllocaInstr && 
                               !pAllocaInstr->getAllocatedType()->isArrayTy() && 
                               !pAllocaInstr->isArrayAllocation();
          if (pStoreInstr->getPointerOperand() == pGASArray || isScalarAlloc) {
            // If this is about a scalar (using immediate 'alloca'/array-argument address as a pointer - with no GEP
            // in between): produce Store of space qualifier directly to 'alloca'/argument of space qualifier
            pNewInstr = new StoreInst(pSpaceVal, spaceDef.pSpace);
            pNewInstr->insertAfter(pStoreInstr);
          } else {
            // Common case: produce Store to an array of addr-space qualifiers
            // Build target pointer to a slot within 'alloca'/argument of space qualifiers' array
            assert(spaceDef.pOffset && "GAS pointer array collection is broken!");
            Instruction *pSpaceSlotPtr = mapArrayPtrToSpacePtr(
                                           pStoreInstr->getPointerOperand(), 
                                           spaceDef.pOffset, pStoreInstr);
            // Build instruction
            pNewInstr = new StoreInst(pSpaceVal, pSpaceSlotPtr);
            // Insert the new instruction immediately after original Store
            pNewInstr->insertAfter(pSpaceSlotPtr);
          }
          assocDebugLocWith(pNewInstr, pStoreInstr);
          changed = true;
        }
      }
    }
    return changed;
  }

  bool GenericAddressDynamicResolution::resolveRetInstructions() {
    bool changed = false;
    // Create additional pointer argument to the function, and assign it with addr-space
    // qualifiers for GAS pointers returned by Ret instructions
    assert(m_ReturnGAS.size() && "No return values to resolve!");
    Argument *pRetArgument = new Argument(
                                 Type::getInt32PtrTy(*m_pLLVMContext), GAS_ARG_NAME, 
                                 m_ReturnGAS[0]->getParent()->getParent());
    for (unsigned idx = 0; idx < m_ReturnGAS.size(); idx++) {
      ReturnInst *pRetInstr = m_ReturnGAS[idx];
      Value *pSpaceVal = getAddrSpaceFor(pRetInstr->getReturnValue());
      assert(pSpaceVal && "GAS pointers should be resolved at this point!");
      // Build instruction
      Instruction *pNewStoreInstr = new StoreInst(pSpaceVal, pRetArgument, pRetInstr);
      assocDebugLocWith(pNewStoreInstr, pRetInstr);
      changed = true;
    }

    return changed;
  }

  static inline unsigned calculateTypeSize(const Type *pType) {
    if (pType->isArrayTy()) {
      return pType->getArrayNumElements() * calculateTypeSize(pType->getArrayElementType());
    } else if (pType->isPointerTy()) {
      return 1;
    } else if (pType->isIntegerTy()) {
      return 1;
    } else {
      assert(0 && "Unexpected type of GAS pointer array element");
      return 0;
    }
  }

  GenericAddressDynamicResolution::SpaceArrayDef 
  GenericAddressDynamicResolution::getArrayToSpaceOffset(Value *pArrayPtr,
                                                         Instruction *pInsertAfter) {
    // Generate the following sequence of instructions (immediately after pInsertAfter):
    //                       EITHER
    // %allocaSize     = mul i32 <size-of-Array-type>, <size-of-pArrayPtr>
    // %arraySpace    = alloca <sizeof void*>, %allocaSize
    //                        OR
    // %arraySpace    = new argument of <sizeof void*>* type
    //                  AND THEN:
    // %arrayInt      = PtrToInt <alloca-type> <pArrayPtr> to i64
    // %arraySpaceInt = PtrToInt <typeof %arraySpace> <%arraySpace> to i64
    // %offset         = sub i64 %arraySpaceInt, %arrayInt
    Value *pArraySpace = NULL;
    Instruction* pOffsetCalcInsertPoint = NULL;
    if (AllocaInst *pAllocaInstr = dyn_cast<AllocaInst>(pArrayPtr)) {
      // size-of GAS pointer array's type
      Value *pSizeOfArrayType = 
          ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext), 
                           calculateTypeSize(pAllocaInstr->getAllocatedType()));
      // %allocaSize = mul i32 <size-of-Array-type>, <size-of-pArrayPtr>
      BinaryOperator *pAllocaSize = 
          BinaryOperator::CreateMul(pSizeOfArrayType,
                                    pAllocaInstr->getArraySize(),
                                    GAS_ALLOCA_NAME);
      pAllocaSize->insertAfter(pInsertAfter);
      assocDebugLocWith(pAllocaSize, pInsertAfter);
      // %allocaSpace = alloca <sizeof void*>, %allocaSize
      AllocaInst *pAllocaSpace = new AllocaInst(m_PointerSlotType, pAllocaSize,
                                                GAS_ALLOCA_NAME);
      pAllocaSpace->insertAfter(pAllocaSize);
      assocDebugLocWith(pAllocaSpace, pInsertAfter);
      pArraySpace = pAllocaSpace;
      // This is where the common offset calculation code will be inserted
      pOffsetCalcInsertPoint = pAllocaSpace;
    } else if (Argument *pArrayArg = dyn_cast<Argument>(pArrayPtr)) {
      // create new argument
      Argument *pArgSpace = new Argument(m_PointerSlotType->getPointerTo(), 
                                         GAS_ARG_NAME, pArrayArg->getParent());
      pArraySpace = pArgSpace;
      // This is where the common offset calculation code will be inserted
      pOffsetCalcInsertPoint = pInsertAfter;
    }

    // %arrayInt = PtrToInt <alloca-type> <pArrayPtr> to i64
    PtrToIntInst *pArrayInt = new PtrToIntInst(pArrayPtr,
                                               Type::getInt64Ty(*m_pLLVMContext),
                                               GAS_ALLOCA_NAME);
    pArrayInt->insertAfter(pOffsetCalcInsertPoint);
    assocDebugLocWith(pArrayInt, pInsertAfter);
    // %arraySpaceInt = PtrToInt <typeof %arraySpace> <%arraySpace> to i64
    PtrToIntInst *pArraySpaceInt = new PtrToIntInst(pArraySpace,
                                                    Type::getInt64Ty(*m_pLLVMContext),
                                                    GAS_ALLOCA_NAME);
    pArraySpaceInt->insertAfter(pArrayInt);
    assocDebugLocWith(pArraySpaceInt, pInsertAfter);
    // %offset = sub i64 %arraySpaceInt, %arrayInt
    BinaryOperator *pOffset =
                    BinaryOperator::CreateSub(pArraySpaceInt, pArrayInt,
                                              GAS_ALLOCA_NAME);
    pOffset->insertAfter(pArraySpaceInt);
    assocDebugLocWith(pOffset, pInsertAfter);

    SpaceArrayDef spaceArray;
    spaceArray.pSpace  = pArraySpace;
    spaceArray.pOffset = pOffset;
    return spaceArray;
  }

  Instruction *GenericAddressDynamicResolution::mapArrayPtrToSpacePtr(
                                                  Value *pArrayPtr, 
                                                  Instruction *pOffset, 
                                                  Instruction *pInsertAfter)    {
    // Generate the following sequence of instructions (immediately after pInsertAfter):
    // %arrayInt      = PtrToInt <array-type> <pArrayPtr> to i64
    // %arraySpaceInt = add i64 %arrayInt, <pOffset>
    // %spacePtr      = IntToPtr i64 %arraySpaceInt to i32*

    // Note that we generate i32* store/load pointer for BOTH 32/64-bit size of pointer.
    // This is because 32-bit is sufficient for addr-space qualifier, while the its slot
    // allocation still have to be equal in size with pointer size (for simplicity of
    // pointer mapping calculation).

    // %arrayInt = PtrToInt <array-type> <pArrayPtr> to i64
    PtrToIntInst *pArrayInt = new PtrToIntInst(pArrayPtr,
                                               Type::getInt64Ty(*m_pLLVMContext),
                                               GAS_ALLOCA_NAME);
    pArrayInt->insertAfter(pInsertAfter);
    assocDebugLocWith(pArrayInt, pInsertAfter);
    // %arraySpaceInt = add i64 %arrayInt, <pOffset>
    BinaryOperator *pArraySpaceInt = 
                    BinaryOperator::CreateAdd(pArrayInt,
                                              pOffset,
                                              GAS_ALLOCA_NAME);
    pArraySpaceInt->insertAfter(pArrayInt);
    assocDebugLocWith(pArraySpaceInt, pInsertAfter);
    // %spacePtr = IntToPtr i64 %arraySpaceInt to i32*
    IntToPtrInst *pSpacePtr = new IntToPtrInst(pArraySpaceInt,
                                               Type::getInt32PtrTy(*m_pLLVMContext),
                                               GAS_ALLOCA_NAME);
    pSpacePtr->insertAfter(pArraySpaceInt);
    assocDebugLocWith(pSpacePtr, pInsertAfter);

    return pSpacePtr;
  }

} // namespace intel
