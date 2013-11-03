/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "GenericAddressDynamicResolution.h"

#include <OCLPassSupport.h>
#include <NameMangleAPI.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/Support/InstIterator.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/ValueMap.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <set>
#include <assert.h>


using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend::Passes::GenericAddressSpace;

namespace intel {


  void GenericAddressDynamicResolution::resolveAddrSpaceQualifierBICall(CallInst *pCallInstr) {

    // Source parameter
    Value *pPtr = pCallInstr->getArgOperand(0);
    assert(dyn_cast<PointerType>(pPtr->getType()) && 
           "Parameter of Address Space Qualifier function should be a pointer!");

    Value *pFolded = getFoldedAddrSpaceCall(pCallInstr, pPtr);
    // Adjusting line-info of inlined code
    if (Instruction *pFoldedInstr = dyn_cast<Instruction>(pFolded)) {
      assocDebugLocWith(pFoldedInstr, pCallInstr);
    }

    // Replacing original call with inlined code
    pCallInstr->replaceAllUsesWith(pFolded);
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
                                                       "AddrSpace", pCallInstr);
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

} // namespace intel
