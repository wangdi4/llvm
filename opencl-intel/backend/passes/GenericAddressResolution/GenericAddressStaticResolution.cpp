/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "GenericAddressStaticResolution.h"

#include <OCLPassSupport.h>
#include <MetaDataApi.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/ValueMap.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Support/Debug.h>
#include <assert.h>

#define DEBUG_TYPE "GenericAddressStaticResolution"

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend::Passes::GenericAddressSpace;

extern "C" {
  /// @brief Creates new GenericAddressStaticResolution module pass
  /// @returns new GenericAddressStaticResolution module pass
  llvm::ModulePass *createGenericAddressStaticResolutionPass() {
    return new intel::GenericAddressStaticResolution();
  }
}

namespace intel {

  char GenericAddressStaticResolution::ID = 0;

  OCL_INITIALIZE_PASS(GenericAddressStaticResolution, "generic-addr-static-resolution", "Resolves generic address space pointers to named ones", false, false)

  GenericAddressStaticResolution::GenericAddressStaticResolution() : ModulePass(ID) {
  }

  GenericAddressStaticResolution::~GenericAddressStaticResolution() {
  }

  bool GenericAddressStaticResolution::runOnModule(Module &M) {
    bool changed = false;
    m_pModule = &M;
    m_pLLVMContext = &M.getContext();
    m_failCount = 0;
    m_functionsToHandle.clear();

    // Sort all functions in call-graph order
    sortFunctionsInCGOrder(m_pModule, m_functionsToHandle, true);

    // Iterate through functions sorted in the function list
    for (TFunctionList::iterator func_it = m_functionsToHandle.begin(),
                                 func_it_end = m_functionsToHandle.end();
                                 func_it != func_it_end; func_it++) {
      m_GASPointers.clear();
      m_GASEstimate.clear();
      m_replaceMap.clear();
      m_replaceVector.clear();
      // Prepare per-function elements of the collection
      analyzeGASPointers(func_it);
      // Static resolution of the collected instructions
      changed |= resolveGASPointers(func_it);
    }

    // Create metadata about remaining GAS pointers
    Intel::MetaDataUtils mdUtils(m_pModule);
    if (mdUtils.empty_ModuleInfoList()) {
      mdUtils.addModuleInfoListItem(Intel::ModuleInfoMetaDataHandle(Intel::ModuleInfoMetaData::get()));
    }
    Intel::ModuleInfoMetaDataHandle handle = mdUtils.getModuleInfoListItem(0);
    handle->setGAScounter(m_failCount);
    mdUtils.save(*m_pLLVMContext);

    return changed;
  }

  void GenericAddressStaticResolution::analyzeGASPointers(TFunctionList::const_iterator curFuncIt) {

    for (inst_iterator inst_it = inst_begin(*curFuncIt),
         inst_it_end = inst_end(*curFuncIt);
          inst_it != inst_it_end;) {

      Instruction *pInstr = &(*inst_it++);

      if (IntToPtrInst* pIntToPtr = dyn_cast<IntToPtrInst>(pInstr)) {
        if (PtrToIntInst* pPtrToInt = dyn_cast<PtrToIntInst>(pIntToPtr->getOperand(0))) {
          Type* dstTy = pIntToPtr->getDestTy();
          CastInst* castInst = CastInst::CreatePointerCast(pPtrToInt->getOperand(0), dstTy, "", pPtrToInt);
          pIntToPtr->replaceAllUsesWith(castInst);
          pIntToPtr->eraseFromParent();
          if(pPtrToInt->user_empty())
            pPtrToInt->eraseFromParent();
          continue;
        }
      }
      for (unsigned idx = 0; idx < pInstr->getNumOperands(); idx++) {
        if (ConstantExpr *pPtrToInt = dyn_cast<ConstantExpr>(pInstr->getOperand(idx)))
          if (Instruction::IntToPtr == pPtrToInt->getOpcode())
            if (ConstantExpr* pIntToPtr = dyn_cast<ConstantExpr>(pPtrToInt->getOperand(0)))
              if (Instruction::PtrToInt == pIntToPtr->getOpcode()){
                Type *dstType = pPtrToInt->getType();
                Constant* newConstant = ConstantExpr::getPointerCast(pIntToPtr->getOperand(0), dstType);
                pPtrToInt->replaceAllUsesWith(newConstant);
              }
      }
    }


    // Collect GAS pointers initializations in the function (together with their uses' tree)
    for (inst_iterator inst_it = inst_begin(*curFuncIt),
                       inst_it_end = inst_end(*curFuncIt);
                       inst_it != inst_it_end; inst_it++) {

      Instruction *pInstr = &(*inst_it);

      // Filter-out unsupported cases of structs of GAS pointers
      const AllocaInst *pAlloca = dyn_cast<const AllocaInst>(pInstr);
      if (pAlloca && isAllocaStructGASPointer(pAlloca->getAllocatedType(), false)) {
        // let the dynamic resolution handle it...
        DEBUG( dbgs() << "No support for structs of generic address space pointers: "; pAlloca->dump(); );
        continue;
      }

      // At first, we check the most frequent initialization cases:
      //     <named>-to-<generic> address space conversion of pointer value by BitCast and GEP
      const PointerType *pPtrType = dyn_cast<const PointerType>(pInstr->getType());
      if (pPtrType && IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace())) {
        unsigned opCode = pInstr->getOpcode();
        if (opCode == Instruction::BitCast || opCode == Instruction::AddrSpaceCast || opCode == Instruction::GetElementPtr) {
          const PointerType *pSrcPtrType = dyn_cast<PointerType>(pInstr->getOperand(0)->getType());
          if (pSrcPtrType && isSinglePtr(pSrcPtrType) &&
              !IS_ADDR_SPACE_GENERIC(pSrcPtrType->getAddressSpace())) {
            // If this is a conversion from named pointer type to GAS pointer:
            // store GAS pointer info into the collection (together with its uses - recursively)
            DEBUG( dbgs() << "Added GAS pointer: "; pInstr->dump(); );
            addGASInstr(pInstr, (OCLAddressSpace::spaces) pSrcPtrType->getAddressSpace());
            continue;
          }
        }
      }

      // Then - look for constant expression producing generic addr-space pointer value
      // out of named one (inside a ConstantExpr operand)
      for (unsigned idx = 0; idx < pInstr->getNumOperands(); idx++) {
        if (handleGASConstantExprIfNeeded(pInstr->getOperand(idx), pInstr)) {
          break;
        }
      }
      // We don't handle 'IntToPtr' case (another initialization case) here because we cannot
      // guess about its named space origin. We will reach it during propagation of
      // corresponding integer value (generated by an 'PtrToInt' instruction)
    }

    // Now collect use trees of GAS pointer initializations
    for (TPointerList::iterator ptr_it = m_GASPointers.begin();
                                ptr_it != m_GASPointers.end(); ptr_it++) {

      Instruction *pInstr = *ptr_it;
      TPointerMap::const_iterator estimate = m_GASEstimate.find(pInstr);
      assert(estimate != m_GASEstimate.end() && "GAS Collection is broken!");
      // We can add new pointers during propagation because they are collected
      // into list - whose iterator is safe after insertion
      propagateSpace(pInstr, estimate->second);
    }
  }

  bool GenericAddressStaticResolution::isAllocaStructGASPointer(
                                            const Type *pType, bool isStructDetected) {
    if (pType->isStructTy()) {
      // Look into the structure fields for arrays, structs and primitive types of GAS pointers
      for (unsigned idx = 0; idx < pType->getStructNumElements(); idx++) {
        const Type *pElemType = pType->getStructElementType(idx);
        if (pElemType->isStructTy() && isAllocaStructGASPointer(pElemType, true)) {
          return true;
        } else if (pElemType->isArrayTy()) {
          const Type *pArrayElemType = pElemType->getArrayElementType();
          if (pArrayElemType->isAggregateType() && isAllocaStructGASPointer(pArrayElemType, true)) {
            return true;
          } else if (pArrayElemType->isPointerTy() &&
                     IS_ADDR_SPACE_GENERIC(
                       cast<const PointerType>(pArrayElemType)->getAddressSpace())) {
            return true;
          }
        } else if (pElemType->isPointerTy() &&
                   IS_ADDR_SPACE_GENERIC(cast<const PointerType>(pElemType)->getAddressSpace())) {
          return true;
        }
      }
    } else if (pType->isArrayTy()) {
      // Look into the array element for structs of GAS pointers
      const Type *pArrayElemType = pType->getArrayElementType();
      if (pArrayElemType->isAggregateType() &&
          isAllocaStructGASPointer(pArrayElemType, isStructDetected)) {
        return true;
      } else if (isStructDetected && pArrayElemType->isPointerTy() &&
                IS_ADDR_SPACE_GENERIC(
                    cast<const PointerType>(pArrayElemType)->getAddressSpace())) {
        return true;
      }
    } else if (isStructDetected && pType->isPointerTy() &&
               IS_ADDR_SPACE_GENERIC(
                   cast<const PointerType>(pType)->getAddressSpace())) {
      return true;
    }
    return false;
  }

  void GenericAddressStaticResolution::propagateSpace(Instruction *pInstr,
                                                      OCLAddressSpace::spaces space) {
    bool toPropagate = false;
    switch (pInstr->getOpcode()) {
      // Instructions which don't generate new pointer - no propagation
      case Instruction::Load          :
      case Instruction::Store         :
      case Instruction::AtomicCmpXchg :
      case Instruction::AtomicRMW     :
      case Instruction::ICmp          :
      case Instruction::Ret           :
        break;
      // Call may propagate GAS pointer, however this is not expected to be
      // frequent, and we leave it for dynamic resolution. This is because the
      // analysis should be delegated to callee, and then we will need yet more
      // rounds over all functions to resolve new dependencies:
      //  - "named" pointer result value may produce new "named" arguments to another callee,
      //  - the latter may produce yet another callee instance,
      //  - the latter may produce yet another "named" return value and so on.
      // In worst case, amount of rounds will reach amount of functions with GAS pointer
      // return value. Compile-time vs. optimization gain trade-off can be negative here.
      case Instruction::Call : {
        const PointerType *pPtrType = dyn_cast<const PointerType>(pInstr->getType());
        if (pPtrType && IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace())) {
          // For GAS pointer return value - enforce it as generic
          space = OCLAddressSpace::Generic;
          toPropagate = true;
        }
        break;
      }
      // Ptr2Int generates integer which may become a pointer back later:
      // propagate to uses
      case Instruction::PtrToInt:
        toPropagate = true;
        break;
      // Instructions which generate new GAS pointer or integer-out-of-GAS-pointer:
      // propagate to uses
      case Instruction::PHI    :
      case Instruction::Select :
        toPropagate = true;
        break;
      // GEP/Int2Ptr may or may not generate new GAS pointer - analyze further
      case Instruction::IntToPtr      :
      case Instruction::GetElementPtr : {
        // Filter-out GEP/Int2Ptr which doesn't propagate GAS pointer
        const PointerType *pPtrType = cast<const PointerType>(pInstr->getType());
        if (IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace())) {
          toPropagate = true;
        }
        break;
      }
      // Bitcast may or may not generate new GAS pointer - analyze further
      case Instruction::AddrSpaceCast :
      case Instruction::BitCast       : {
        // Filter-out Bitcasts which don't propagate GAS pointer
        const PointerType *pPtrType = dyn_cast<const PointerType>(pInstr->getType());
        if (pPtrType && IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace())) {
          toPropagate = true;
        }
        break;
      }
      default:
        if (pInstr->isBinaryOp()) {
          // Integer which is produced out of pointer can be propagated by binary operation
          toPropagate = true;
          break;
        }
        // No other instruction is expected (as bitwise and numerical conversions of
        // integer produced out of pointer are not expected)
        assert(0 && "Unexpected instruction with generic address space pointer");
        break;
    }
    if (toPropagate) {
      for (Value::user_iterator user_it = pInstr->user_begin(),
                                user_end = pInstr->user_end();
                                user_it != user_end; user_it++) {
        Instruction *pUserInst = dyn_cast<Instruction>(*user_it);
        assert(pUserInst && "All uses of instruction should be instructions!");
        addGASInstr(pUserInst, space);
      }
    }
  }

  void GenericAddressStaticResolution::addGASInstr(Instruction *pInstr, OCLAddressSpace::spaces space) {
    // Special case: call to LLVM intrinsic which is not overloadable.
    // In such case we should preserve GAS pointer as is.
    if (IntrinsicInst *pInstrinInstr = dyn_cast<IntrinsicInst>(pInstr)) {
      if (!Intrinsic::isOverloaded(pInstrinInstr->getIntrinsicID())) {
        space = OCLAddressSpace::Generic;
      }
    }
    TPointerMap::iterator ptr_it = m_GASEstimate.find(pInstr);
    if (ptr_it == m_GASEstimate.end()) {
      // For first-seen instruction - record it
      m_GASPointers.push_back(pInstr);
      if (pInstr->getOpcode() == Instruction::Ret) {
        // Enforce GAS return value on Ret - on order to be symmetrical to Call
        space = OCLAddressSpace::Generic;
      }
      if (pInstr->getOpcode() == Instruction::Store) {
        // If Store instruction encountered due to its VALUE operand (rather than
        // ADDRESS operand) - we should not resolve it. That is because not all
        // uses of corresponding 'alloca' GAS pointer may be resolvable.
        const PointerType *pValuePtrType =
                      dyn_cast<const PointerType>(pInstr->getOperand(0)->getType());
        if (pValuePtrType && IS_ADDR_SPACE_GENERIC(pValuePtrType->getAddressSpace())) {
          // The Store is due to GAS pointer value operand - enforce it as GAS case
          assert(!IS_ADDR_SPACE_GENERIC
                  (cast<PointerType>
                      (pInstr->getOperand(1)->getType())->getAddressSpace()) &&
                 "GAS pointer cannot be stored to memory pointed-to by GAS pointer!");
          space = OCLAddressSpace::Generic;
        }
      }
      m_GASEstimate.insert(TPointerInfo(pInstr, space));
      return;
    }
    // If we reached already traversed node - validate its type
    if (ptr_it->second == space) {
      // Original addr space is confirmed - nothing to do
      return;
    }
    // Filter-out call instruction, in which multiple addr spaces are allowed
    if (pInstr->getOpcode() == Instruction::Call) {
      Function *pCallee = cast<CallInst>(pInstr)->getCalledFunction();
      if (!pCallee || !isAddressQualifierBI(pCallee)) {
        return;
      }
    }
    // In the case of conflicting types - revert named space to generic
    if (!IS_ADDR_SPACE_GENERIC(space)) {
      // Account for failure
      m_failCount++;
    }
    if (ptr_it->second != OCLAddressSpace::Generic) {
      // In the case of conflict - enforce GAS type and ...
      ptr_it->second = OCLAddressSpace::Generic;
      // ... proceed to uses in order to revert them to generic space as well
      propagateSpace(pInstr, ptr_it->second);
    }
  }

  bool GenericAddressStaticResolution::handleGASConstantExprIfNeeded(Value *pOperand, Instruction *pInstr) {
    // Check that the operand produces GAS pointer
    PointerType *pPtrType = dyn_cast<PointerType>(pOperand->getType());
    if (pPtrType && IS_ADDR_SPACE_GENERIC(pPtrType->getAddressSpace())) {
      // Check for constant expression
      if (ConstantExpr *pCE = dyn_cast<ConstantExpr>(pOperand)) {
        // Check operands of the constant expression
        for (unsigned idx = 0; idx < pCE->getNumOperands(); idx++) {
          Value *pOpVal = pCE->getOperand(idx);
          if (PointerType *pOpPtrType = dyn_cast<PointerType>(pOpVal->getType())) {
            // We're looking only for pointer operands of the expression
            OCLAddressSpace::spaces opPtrSpace = (OCLAddressSpace::spaces) pOpPtrType->getAddressSpace();
            if (IS_ADDR_SPACE_GENERIC(opPtrSpace)) {
              // If the pointer is GAS - look for a named addr-space pointer behind him
              return handleGASConstantExprIfNeeded(pOpVal, pInstr);
            } else {
              // If the pointer is named - add the instruction to the collection
              addGASInstr(pInstr, opPtrSpace);
              return true;
            }
          }
        }
      }
    }
    return false;
  }

  bool GenericAddressStaticResolution::resolveGASPointers(TFunctionList::iterator curFuncIt) {

    bool changed = false;
    // Iterate through the collection of GAS pointers and try to resolve them statically
    // to named address space pointer
    for (TPointerList::iterator ptr_it = m_GASPointers.begin(),
                                ptr_end = m_GASPointers.end();
                                ptr_it != ptr_end; ptr_it++) {
      Instruction *pInstr = *ptr_it;
      TPointerMap::const_iterator map_it = m_GASEstimate.find(pInstr);
      assert(map_it != m_GASEstimate.end() && "GAS pointer collection is broken!");
      OCLAddressSpace::spaces space = map_it->second;
      // Ignore instructions which cannot be resolved
      if (IS_ADDR_SPACE_GENERIC(space)) {
        continue;
      }

      // Resolve GAS pointers from collection:

      // 1. Prepare replacements with named addr space pointers
      switch (pInstr->getOpcode()) {
        case Instruction::IntToPtr      :
        case Instruction::AddrSpaceCast :
        case Instruction::BitCast       :
        case Instruction::GetElementPtr :
          changed |= resolveInstructionConvert(pInstr, space);
          break;
        case Instruction::Load          :
        case Instruction::Store         :
        case Instruction::AtomicCmpXchg :
        case Instruction::AtomicRMW     :
        case Instruction::PtrToInt      :
          changed |= resolveInstructionOnePointer(pInstr, space);
          break;
        case Instruction::PHI :
          changed |= resolveInstructionPhiNode(cast<PHINode>(pInstr), space);
          break;
        case Instruction::Select :
        case Instruction::ICmp   :
          changed |= resolveInstructionTwoPointers(pInstr, space);
          break;
        case Instruction::Call :
          changed |= resolveInstructionCall(cast<CallInst>(pInstr), curFuncIt);
          break;
        default:
          if (pInstr->isBinaryOp()) {
            // Nothing to resolve for binary operation - it served only as transit
            // node for propagation of integer-out-of-GAS-pointer
            break;
          }
          assert(0 && "Unexpected instruction with generic address space pointer");
          break;
      }
    }

    // 2. Integrate replacements into the function body
    for (TReplaceVector::const_reverse_iterator repl_it = m_replaceVector.rbegin(),
                                                repl_end = m_replaceVector.rend();
                                                repl_it != repl_end; repl_it++) {

      Instruction *pOldInstr = repl_it->first;
      Value *pNewVal = repl_it->second;

      // Replace uses of original instruction with those of new value
      switch (pOldInstr->getOpcode()) {
        case Instruction::Load          :
        case Instruction::Store         :
        case Instruction::AtomicCmpXchg :
        case Instruction::AtomicRMW     :
        case Instruction::PtrToInt      :
        case Instruction::ICmp          :
        case Instruction::Call          :
          // For instruction which doesn't produce a pointer: replace uses with new value
          pOldInstr->replaceAllUsesWith(pNewVal);
          break;
        case Instruction::AddrSpaceCast :
        case Instruction::BitCast       :
        case Instruction::IntToPtr      :
        case Instruction::GetElementPtr : {
          // For Int2Ptr/Bitcast/GEP instruction which ORIGINALLY produced NAMED addr-space pointer
          // or integer: replace uses with new value
          PointerType *pDestType = dyn_cast<PointerType>(pOldInstr->getType());
          if (!pDestType || !IS_ADDR_SPACE_GENERIC(pDestType->getAddressSpace())) {
            pOldInstr->replaceAllUsesWith(pNewVal);
          } else {
            // Clean-up is need because BFS tree of GAS data flow is not guaranteed
            // to be balanced, and yet may have cycles
            pOldInstr->replaceAllUsesWith(Constant::getNullValue(pOldInstr->getType()));
          }
          break;
        }
        default:
          // For instruction which produces a pointer (less bitcast/GEP special case above):
          // its use is already set during address space resolution, however
          // clean-up is yet needed because BFS tree of GAS data flow is not guaranteed
          // to be balanced, and yet may have cycles
          pOldInstr->replaceAllUsesWith(Constant::getNullValue(pOldInstr->getType()));
          break;
      }
      // Fix-up debug info for new instruction
      if (Instruction *pNewInstr = dyn_cast<Instruction>(pNewVal)) {
        assocDebugLocWith(pNewInstr, pOldInstr);
      }
      // Remove original instruction
      pOldInstr->eraseFromParent();
    }

    return changed;
  }

} // namespace intel
