/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "GroupBuiltinPass.h"
#include "OCLPassSupport.h"
#include "CompilationUtils.h"
#include <NameMangleAPI.h>
#include <FunctionDescriptor.h>
#include <ParameterType.h>

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"

#include <cfloat>
#include <climits>

using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

  typedef std::pair<Function*,Instruction*> TFuncEntryPair;
  typedef std::map<Function*,Instruction*> TFuncEntryMap;

  char GroupBuiltin::ID = 0;

  OCL_INITIALIZE_PASS(GroupBuiltin, "B-GroupBuiltins", "Barrier Pass - Handle WorkGroup BI calls", false, true)

  GroupBuiltin::GroupBuiltin() : ModulePass(ID) {}


  Constant *GroupBuiltin::getInitializationValue(Function *pFunc) {

    // Collect parameters for initialization value selection 
    Type *pRetType = pFunc->getFunctionType()->getReturnType();
    unsigned dataWidth = pRetType->isVectorTy()? pRetType->getVectorNumElements() : 1;
    std::string funcName = pFunc->getName().str();
    assert(isMangledName(funcName.c_str()) && "WG BI function name is expected to be mangled!");
    reflection::FunctionDescriptor fd = demangle(funcName.c_str());
    reflection::RefParamType pParam = fd.parameters[0];
    if (reflection::VectorType *pVecParam = 
              reflection::dyn_cast<reflection::VectorType>(pParam)) {
      pParam = pVecParam->getScalarType();
    }
    reflection::PrimitiveType *pPrimitiveParam = reflection::dyn_cast<reflection::PrimitiveType>(pParam);
    assert(pPrimitiveParam && "WG function parameter should be either primitive or vector of primitives");
    reflection::TypePrimitiveEnum dataEnum = pPrimitiveParam->getPrimitive();

    Constant *pInitVal = NULL;
    Type *pInt32Type = Type::getInt32Ty(*m_pLLVMContext);
    Type *pInt64Type = Type::getInt64Ty(*m_pLLVMContext);
    Type *pFloatType = Type::getFloatTy(*m_pLLVMContext);
    Type *pDoubleType = Type::getDoubleTy(*m_pLLVMContext);
    // Act according to the function's logic
    if (CompilationUtils::isWorkGroupAll(funcName)) {
      // Initial value for work_group_all: 0x1
      switch (dataEnum) {
        case reflection::PRIMITIVE_INT:
          pInitVal = ConstantInt::get(pInt32Type, 1);
          break;
        default:
          assert(0 && "Unsupported WG argument type");
          break;
      }
    } else if (CompilationUtils::isWorkGroupMax(funcName)) {
      // Initial value for work_group_..._max: MIN
      switch (dataEnum) {
        case reflection::PRIMITIVE_INT:
          pInitVal = ConstantInt::get(pInt32Type, INT_MIN);
          break;
        case reflection::PRIMITIVE_UINT:
          pInitVal = ConstantInt::get(pInt32Type, 0);
          break;
        case reflection::PRIMITIVE_LONG:
          pInitVal = ConstantInt::get(pInt64Type, LONG_MIN);
          break;
        case reflection::PRIMITIVE_ULONG:
          pInitVal = ConstantInt::get(pInt64Type, 0);
          break;
        case reflection::PRIMITIVE_FLOAT:
          pInitVal = ConstantFP::get(pFloatType, FLT_MIN);
          break;
        case reflection::PRIMITIVE_DOUBLE:
          pInitVal = ConstantFP::get(pDoubleType, DBL_MIN);
          break;
        default:
          assert(0 && "Unsupported WG argument type");
          break;
      }
    } else if (CompilationUtils::isWorkGroupMin(funcName)) {
      // Initial value for work_group_..._min: MAX
      switch (dataEnum) {
        case reflection::PRIMITIVE_INT:
          pInitVal = ConstantInt::get(pInt32Type, INT_MAX);
          break;
        case reflection::PRIMITIVE_UINT:
          pInitVal = ConstantInt::get(pInt32Type, UINT_MAX);
          break;
        case reflection::PRIMITIVE_LONG:
          pInitVal = ConstantInt::get(pInt64Type, LONG_MAX);
          break;
        case reflection::PRIMITIVE_ULONG:
          pInitVal = ConstantInt::get(pInt64Type, ULONG_MAX);
          break;
        case reflection::PRIMITIVE_FLOAT:
          pInitVal = ConstantFP::get(pFloatType, FLT_MAX);
          break;
        case reflection::PRIMITIVE_DOUBLE:
          pInitVal = ConstantFP::get(pDoubleType, DBL_MAX);
          break;
        default:
          assert(0 && "Unsupported WG argument type");
          break;
      }
    } else {
      // Initial value for the rest of WG functions: zero
      switch (dataEnum) {
        case reflection::PRIMITIVE_INT:
        case reflection::PRIMITIVE_UINT:
          pInitVal = ConstantInt::get(pInt32Type, 0);
          break;
        case reflection::PRIMITIVE_LONG:
        case reflection::PRIMITIVE_ULONG:
          pInitVal = ConstantInt::get(pInt64Type, 0);
          break;
        case reflection::PRIMITIVE_FLOAT:
          pInitVal = ConstantFP::get(pFloatType, 0.0);
          break;
        case reflection::PRIMITIVE_DOUBLE:
          pInitVal = ConstantFP::get(pDoubleType, 0.0);
          break;
        default:
          assert(0 && "Unsupported WG argument type");
          break;
      }
    }

    // Broadcast the constant to vector if relevant
    if (dataWidth > 1) {
      pInitVal = ConstantVector::getSplat(dataWidth, pInitVal);
    }

    return pInitVal;
  }


  CallInst *GroupBuiltin::getWICall(Instruction *pBefore, std::string funcName, unsigned dimIdx) {
    // Arguments and parameters
    SmallVector<Type*,  8> argTypes;
    SmallVector<Value*, 8> params;
    Type *pInt32Type = Type::getInt32Ty(*m_pLLVMContext);
    argTypes.push_back(pInt32Type);
    params.push_back(ConstantInt::get(pInt32Type, dimIdx));
    // Function object
    FunctionType *pFuncType = FunctionType::get(m_pSizeT, argTypes, false);
    Function *pFunc = dyn_cast<Function>(m_pModule->getOrInsertFunction(funcName, pFuncType));
    assert(pFunc && "Non-function object with the same signature identified in the module");
    // Function call
    CallInst *pCall = CallInst::Create(pFunc, ArrayRef<Value*>(params), "WIcall", pBefore);
    assert(pCall && "Couldn't create CALL instruction!");
    return pCall;
  }

  // Generates sequence for LinearID calculation for 2D workgroup
  static Instruction *calculate2DimLinearID(Instruction *pBefore, Value *pLocalID_0, 
                                            Value *pLocalSize_0, Value *pLocalID_1) {
    // Calculate 2-dimensional LinearID: local_id(1)*get_local_size(0)+local_id(0)
    BinaryOperator *pMul = 
          BinaryOperator::CreateMul(pLocalSize_0, pLocalID_1, "", pBefore);
    return BinaryOperator::CreateAdd(pMul, pLocalID_0, "getLinearId2D", pBefore);
  }

  // Generates sequence for LinearID calculation for 3D workgroup
  static Instruction *calculate3DimLinearID(Instruction *pBefore, Value *pLinearID_2Dim, 
                                            Value *pLocalSize_0, Value *pLocalSize_1, 
                                            Value *pLocalID_2) {
    // Calculate 3-dimensional LinearID: 
    //    <2-dimensional LinearID> + local_id(2)*get_local_size(0)*get_local_size(1)
    BinaryOperator *pMul1 = 
        BinaryOperator::CreateMul(pLocalSize_1, pLocalID_2, "", pBefore);
    BinaryOperator *pMul2 = 
        BinaryOperator::CreateMul(pLocalSize_0, pMul1, "", pBefore);
    return BinaryOperator::CreateAdd(pMul2, pLinearID_2Dim, "getLinearId3D", pBefore);
  }


  Instruction *GroupBuiltin::getLinearID(CallInst *pWgCallInstr) {
    unsigned nDim = pWgCallInstr->getNumArgOperands() - 1;
    // For 1-dim workgroup - return get_local_id(0)
    Instruction *pRetVal = getWICall(pWgCallInstr, CompilationUtils::mangledGetLID(), 0);
    if (nDim > 1) {
      // For multi-dimensional - start from 2-dim calculation
      CallInst *pLocalSize_0 = getWICall(pWgCallInstr, CompilationUtils::mangledGetLocalSize(), 0);
      CallInst *pLocalID_1   = getWICall(pWgCallInstr, CompilationUtils::mangledGetLID(), 1);
      pRetVal = calculate2DimLinearID(pWgCallInstr, pRetVal, pLocalSize_0, pLocalID_1);
      if (nDim > 2) {
        // For 3-dim - account for dimension#2
        CallInst *pLocalSize_1 = getWICall(pWgCallInstr, CompilationUtils::mangledGetLocalSize(), 1);
        CallInst *pLocalID_2   = getWICall(pWgCallInstr, CompilationUtils::mangledGetLID(), 2);
        pRetVal = calculate3DimLinearID(pWgCallInstr, pRetVal, pLocalSize_0, pLocalSize_1, pLocalID_2);
      }
    }
    return pRetVal;
  }

  Value *GroupBuiltin::calculateLinearID(CallInst *pWgCallInstr) {
    unsigned nDim = pWgCallInstr->getNumArgOperands() - 1;
    // For single-dimensional we return local_id parameter as is
    Value *pRetVal = pWgCallInstr->getArgOperand(1);
    if (nDim > 1) {
      // For multi-dimensional - start from 2-dim calculation
      CallInst *pLocalSize_0 = getWICall(pWgCallInstr, CompilationUtils::mangledGetLocalSize(), 0);
      pRetVal = calculate2DimLinearID(pWgCallInstr, pRetVal, pLocalSize_0, pWgCallInstr->getArgOperand(2));
      if (nDim > 2) {
        // For 3-dim - account for dimension#2
        CallInst *pLocalSize_1 = getWICall(pWgCallInstr, CompilationUtils::mangledGetLocalSize(), 1);
        pRetVal = calculate3DimLinearID(pWgCallInstr, pRetVal, pLocalSize_0, pLocalSize_1, 
                                        pWgCallInstr->getArgOperand(3));
      }
    }
    return pRetVal;
  }

  bool GroupBuiltin::runOnModule(Module &M) {

    m_pModule = &M;
    m_pLLVMContext = &M.getContext();
    m_pSizeT = (m_pModule->getPointerSize() == Module::Pointer64)? 
                                          Type::getInt64Ty(*m_pLLVMContext): 
                                          Type::getInt32Ty(*m_pLLVMContext);

    //Initialize barrier utils class with current module
    m_util.init(&M);

    // Handle async built-ins
    TInstructionVector callAsyncFunc = m_util.getWGCallInstructions(CALL_BI_TYPE_ASYNC);
    for (unsigned idx = 0; idx < callAsyncFunc.size(); idx++) {
      CallInst *pAsyncCallInst = cast<CallInst>(callAsyncFunc[idx]);
      // Add Barrier before async function call instruction
      m_util.createBarrier(pAsyncCallInst);
      // Add dummyBarrier after async function call instruction
      Instruction *pDummyBarrierCall = m_util.createDummyBarrier();
      pDummyBarrierCall->insertAfter(pAsyncCallInst);
    }

    // Handle WorkGroup built-ins
    TInstructionVector callWgFunc = m_util.getWGCallInstructions(CALL_BI_TYPE_WG);
    TFuncEntryMap funcFirstInst;
    for (unsigned idx = 0; idx < callWgFunc.size(); idx++) {
      CallInst *pWgCallInst = cast<CallInst>(callWgFunc[idx]);
      // Replace call to WG-wide function with that to per-WI function 
      // (which accumulates the result):

      // Collect info about caller function (where the call resides)
      Function *pFunc = pWgCallInst->getParent()->getParent();
      Instruction *pFirstInstr = pFunc->getEntryBlock().begin();
      // We keep function's 1st instruction BEFORE any allocas & initializations
      // will be inserted above it, the rest of "inserts" of caller function
      // won't succeed because of existing entry for the same function
      funcFirstInst.insert(TFuncEntryPair(pFunc, pFirstInstr));

      // Info about this function call
      unsigned numArgs = pWgCallInst->getNumArgOperands();
      Function *pCallee = pWgCallInst->getCalledFunction();
      std::string funcName = pCallee->getName().str();
      Type *pRetType = pWgCallInst->getType();

      // 1. Add alloca for the accumulated result at the function start...
      AllocaInst *pResult = new AllocaInst(pRetType, "AllocaWGResult", pFirstInstr);

      // 2. ... and initialize the result according to function name and argument type
      Value *pInitValue = getInitializationValue(pCallee);
      (void) new StoreInst(pInitValue, pResult, pFirstInstr);

      // 3. Extend function parameter list with pointer to of the accumulated result

      //   a. At first - produce argument type & parameter lists for the new callee
      SmallVector<Type*,  2> argTypes;
      SmallVector<Value*, 2> params;
      if (!CompilationUtils::isWorkGroupBroadCast(funcName)) {
        //    For all WG function, but broadcasts - keep original arguments intact
        for (unsigned idx = 0; idx < numArgs; idx++) {
          Value *pArg = pWgCallInst->getArgOperand(idx);
          argTypes.push_back(pArg->getType());
          params.push_back(pArg);
        }
      } else {
        //    For broadcasts - replace "local_id" argument with LINEAR local_id,
        //    followed by LINEAR local ID of the WI
        //     --- original 'gentype' argument
        Value *pArg = pWgCallInst->getArgOperand(0);
        argTypes.push_back(pArg->getType());
        params.push_back(pArg);
        //     --- linear form of local_id parameter
        pArg = calculateLinearID(pWgCallInst);
        argTypes.push_back(m_pSizeT);
        params.push_back(pArg);
       //     --- linear local ID of the WI
        pArg = getLinearID(pWgCallInst);
        argTypes.push_back(m_pSizeT);
        params.push_back(pArg);
      }
      //      Append pointer to return value accumulator
      argTypes.push_back(pResult->getType());
      params.push_back(pResult);

      //   b. Remangle the function name upon appended accumulated result's pointer
      assert(isMangledName(funcName.c_str()) && "WG BI function name is expected to be mangled!");
      reflection::FunctionDescriptor fd = demangle(funcName.c_str());
      if (CompilationUtils::isWorkGroupBroadCast(funcName)) {
        //    Adjust parameter list to changes in broadcast WG signature (recycling 'size_t' from parameter#1):
        if (numArgs == 2) {
          //     --- complete lacking (for 1D broadcast)
          fd.parameters.push_back(fd.parameters[1]);
        } else if (numArgs == 4) {
          //     --- remove redundant (for 3D broadcast)
          fd.parameters.pop_back();
        }
      }
      //      We're utilizing 'gentype' parameter attribute from the 1st argument, because for a WG
      //      function its return type is ALWAYS a pointer to the type of 1st parameter. The better way 
      //      would be to use a constructor of type argument out of LLVM type, however there is no 
      //      such in NameMangleAPI. We mangle a __private pointer to that 'gentype'.
      reflection::PointerType *pGenT = new reflection::PointerType(fd.parameters[0]);
      pGenT->addAttribute(reflection::ATTR_PRIVATE);
      fd.parameters.push_back(reflection::RefParamType(pGenT));
      std::string newFuncName = mangle(fd);

      //   c. Create function declaration object (unless the module contains it already)
      FunctionType *pNewFuncType = FunctionType::get(pRetType, argTypes, false);
      Function *pNewFunc = dyn_cast<Function>(M.getOrInsertFunction(newFuncName, pNewFuncType));
      assert(pNewFunc && "Non-function object with the same signature identified in the module");
      pNewFunc->setAttributes(pCallee->getAttributes());
      pNewFunc->setLinkage(pCallee->getLinkage());
      pNewFunc->setCallingConv(pCallee->getCallingConv());

      // 4. Prepare the call with that to function with extended parameter list
      CallInst *pNewCall = CallInst::Create(pNewFunc, ArrayRef<Value*>(params), "CallWGForItem", pWgCallInst);
      assert(pNewCall && "Couldn't create CALL instruction!");
      pNewCall->setAttributes(pWgCallInst->getAttributes());
      pNewCall->setCallingConv(pWgCallInst->getCallingConv());
      if (!pWgCallInst->getDebugLoc().isUnknown()) {
        pNewCall->setDebugLoc(pWgCallInst->getDebugLoc());
      }

      // 5. Create barrier() call immediately AFTER per-WI call
      (void) m_util.createBarrier(pWgCallInst);

      // 6. For uniform & vectorized WG function (less broadcast WG) - finalize the result
      if (CompilationUtils::isWorkGroupUniform(funcName)    && 
          !CompilationUtils::isWorkGroupBroadCast(funcName) &&
          pRetType->isVectorTy()) {

        // a. Load 'alloca' value of the accumulated result
        LoadInst *pLoadResult = new LoadInst(pResult, "LoadWGFinalResult", pWgCallInst);

        // b. Create finalization function object:
        //    --- argument and parameter lists
        SmallVector<Type*,  8> argTypes;
        SmallVector<Value*, 8> params;
        argTypes.push_back(pRetType);
        params.push_back(pLoadResult);
        //    --- remangle with unique name (derived from original WG function name)
        //        [Note that the signature is as of original WG function]
        reflection::FunctionDescriptor fd = demangle(funcName.c_str());
        fd.name = FINALIZE_WG_FUNCTION_PREFIX + fd.name;
        std::string finalizeFuncName = mangle(fd);
        //    --- create function
        FunctionType *pFinalizeFuncType = FunctionType::get(pRetType, argTypes, false);
        Function *pFinalizeFunc = dyn_cast<Function>(M.getOrInsertFunction(finalizeFuncName, pFinalizeFuncType));
        assert(pFinalizeFunc && "Non-function object with the same signature identified in the module");

        // c. Create call to finalization function object
        CallInst *pFinalizeCall = CallInst::Create(pFinalizeFunc, ArrayRef<Value*>(params), "CallFinalizeWG", pWgCallInst);
        assert(pFinalizeCall && "Couldn't create CALL instruction!");

        // d. Create dummy barrier immediately AFTER finalization call
        (void) m_util.createDummyBarrier(pWgCallInst);

        pNewCall = pFinalizeCall;
      }

      // 7. Discard old function call
      pWgCallInst->replaceAllUsesWith(pNewCall);
      pWgCallInst->eraseFromParent();

    }

    // Add dummyBarrier after allocas & initializations of WG function return value accumulators
    for (TFuncEntryMap::iterator func_it = funcFirstInst.begin(), 
                                 func_it_end = funcFirstInst.end();
                                 func_it != func_it_end; func_it++) {
      Instruction *pDummyBarrierCall = m_util.createDummyBarrier();
      pDummyBarrierCall->insertBefore(func_it->second);
    }

    return !callAsyncFunc.empty() || !callWgFunc.empty();
  }


} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createGroupBuiltinPass() {
    return new intel::GroupBuiltin();
  }
}
