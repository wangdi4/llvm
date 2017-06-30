/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "GroupBuiltinPass.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "CompilationUtils.h"
#include "NameMangleAPI.h"
#include "FunctionDescriptor.h"
#include "ParameterType.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"

#include <CL/cl.h>

using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

  typedef std::pair<Function*,Instruction*> TFuncEntryPair;
  typedef std::map<Function*,Instruction*> TFuncEntryMap;

  char GroupBuiltin::ID = 0;

  OCL_INITIALIZE_PASS_BEGIN(GroupBuiltin, "B-GroupBuiltins", "Barrier Pass - Handle WorkGroup BI calls", false, true)
  OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
  OCL_INITIALIZE_PASS_END(GroupBuiltin, "B-GroupBuiltins", "Barrier Pass - Handle WorkGroup BI calls", false, true)

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
          pInitVal = ConstantInt::get(pInt32Type, CL_INT_MIN);
          break;
        case reflection::PRIMITIVE_UINT:
          pInitVal = ConstantInt::get(pInt32Type, 0);
          break;
        case reflection::PRIMITIVE_LONG:
          pInitVal = ConstantInt::get(pInt64Type, CL_LONG_MIN);
          break;
        case reflection::PRIMITIVE_ULONG:
          pInitVal = ConstantInt::get(pInt64Type, 0);
          break;
        case reflection::PRIMITIVE_HALF:
          pInitVal = ConstantFP::get(*m_pLLVMContext, APFloat::getInf(APFloat::IEEEhalf(), true));
          break;
        case reflection::PRIMITIVE_FLOAT:
          pInitVal = ConstantFP::get(*m_pLLVMContext, APFloat::getInf(APFloat::IEEEsingle(), true));
          break;
        case reflection::PRIMITIVE_DOUBLE:
          pInitVal = ConstantFP::get(*m_pLLVMContext, APFloat::getInf(APFloat::IEEEdouble(), true));
          break;
        default:
          assert(0 && "Unsupported WG argument type");
          break;
      }
    } else if (CompilationUtils::isWorkGroupMin(funcName)) {
      // Initial value for work_group_..._min: MAX
      switch (dataEnum) {
        case reflection::PRIMITIVE_INT:
          pInitVal = ConstantInt::get(pInt32Type, CL_INT_MAX);
          break;
        case reflection::PRIMITIVE_UINT:
          pInitVal = ConstantInt::get(pInt32Type, CL_UINT_MAX);
          break;
        case reflection::PRIMITIVE_LONG:
          pInitVal = ConstantInt::get(pInt64Type, CL_LONG_MAX);
          break;
        case reflection::PRIMITIVE_ULONG:
          pInitVal = ConstantInt::get(pInt64Type, CL_ULONG_MAX);
          break;
        case reflection::PRIMITIVE_HALF:
          pInitVal = ConstantFP::get(*m_pLLVMContext, APFloat::getInf(APFloat::IEEEhalf(), false));
          break;
        case reflection::PRIMITIVE_FLOAT:
          pInitVal = ConstantFP::get(*m_pLLVMContext, APFloat::getInf(APFloat::IEEEsingle(), false));
          break;
        case reflection::PRIMITIVE_DOUBLE:
          pInitVal = ConstantFP::get(*m_pLLVMContext, APFloat::getInf(APFloat::IEEEdouble(), false));
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
        case reflection::PRIMITIVE_HALF:
          pInitVal = ConstantFP::get(*m_pLLVMContext, APFloat::getZero(APFloat::IEEEhalf()));
          break;
        case reflection::PRIMITIVE_FLOAT:
          pInitVal = ConstantFP::get(*m_pLLVMContext, APFloat::getZero(APFloat::IEEEsingle()));
          break;
        case reflection::PRIMITIVE_DOUBLE:
          pInitVal = ConstantFP::get(*m_pLLVMContext, APFloat::getZero(APFloat::IEEEdouble()));
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

  Function* GroupBuiltin::FindFunctionInModule(const std::string& funcName) {
    for (SmallVector<Module*, 2>::iterator it = m_builtinModuleList.begin();
        it != m_builtinModuleList.end(); ++it) {
      assert(*it != NULL && "Encountered NULL ptr in m_builtinModuleList");
      Function* pRetFunction = (*it)->getFunction(funcName);
      if (pRetFunction)
        return pRetFunction;
    }
    return NULL;
  }

  bool GroupBuiltin::runOnModule(Module &M) {

    m_pModule = &M;
    m_pLLVMContext = &M.getContext();
    unsigned pointerSizeInBits = M.getDataLayout().getPointerSizeInBits(0);
    assert((32 == pointerSizeInBits  || 64 == pointerSizeInBits) &&
           "Unsupported pointer size");
    m_pSizeT = IntegerType::get(*m_pLLVMContext, pointerSizeInBits);
    m_builtinModuleList = getAnalysis<BuiltinLibInfo>().getBuiltinModules();
    assert(!m_builtinModuleList.empty() && "Builtin module were not initialized!");

    //Initialize barrier utils class with current module
    m_util.init(&M);

    // Handle async and pipe work-group built-ins
    TInstructionVector callWGSimpleFunc = m_util.getWGCallInstructions(CALL_BI_TYPE_WG_ASYNC_OR_PIPE);
    for (unsigned idx = 0; idx < callWGSimpleFunc.size(); idx++) {
      CallInst *pWGSimpleCallInst = cast<CallInst>(callWGSimpleFunc[idx]);
      // Add Barrier before async function call instruction
      m_util.createBarrier(pWGSimpleCallInst);
      // Add dummyBarrier after async function call instruction
      Instruction *pDummyBarrierCall = m_util.createDummyBarrier();
      pDummyBarrierCall->insertAfter(pWGSimpleCallInst);
    }

    // Handle WorkGroup built-ins
    TInstructionVector callWgFunc = m_util.getWGCallInstructions(CALL_BI_TYPE_WG);
    TFunctionSet visitedFunctions;
    for (unsigned idx = 0; idx < callWgFunc.size(); idx++) {
      CallInst *pWgCallInst = cast<CallInst>(callWgFunc[idx]);
      // Replace call to WG-wide function with that to per-WI function
      // (which accumulates the result):

      // Collect info about caller function (where the call resides)
      Function *pFunc = pWgCallInst->getParent()->getParent();
      Instruction *pFirstInstr = &*pFunc->getEntryBlock().begin();
      // Mark the function as visited.
      if (visitedFunctions.insert(pFunc)) {
        // This is the first time we visit this function.
        //
        // This pass creates alloca instructions and initialize them as part
        // of the solution to resolve a work group builtin.
        // These allocas are uniform for all work items in a group.
        // However, barrier pass handles all allocas as non-uniform.
        // So, in order to prevent that, we add a marker "dummyBarrier" at begin
        // of the function, and make sure all alloca and initialization instructions
        // are added before this marker. Need to add the marker only once.
        Instruction *pDummyBarrierCall = m_util.createDummyBarrier(pFirstInstr);
        // Update the first instruction to be the marker. All alloca & initialize
        // instructions will be created before this first instruction.
        pFirstInstr = pDummyBarrierCall;
      }

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

      //   a. At first - produce parameter list for the new callee
      SmallVector<Value*, 2> params;
      if (!CompilationUtils::isWorkGroupBroadCast(funcName)) {
        //    For all WG function, but broadcasts - keep original arguments intact
        for (unsigned idx = 0; idx < numArgs; idx++) {
          Value *pArg = pWgCallInst->getArgOperand(idx);
          params.push_back(pArg);
        }
      } else {
        //    For broadcasts - replace "local_id" argument with LINEAR local_id,
        //    followed by LINEAR local ID of the WI
        //     --- original 'gentype' argument
        Value *pArg = pWgCallInst->getArgOperand(0);
        params.push_back(pArg);
        //     --- linear form of local_id parameter
        pArg = calculateLinearID(pWgCallInst);
        params.push_back(pArg);
       //     --- linear local ID of the WI
        pArg = getLinearID(pWgCallInst);
        params.push_back(pArg);
      }
      //      Append pointer to return value accumulator
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
      //      --- get the new function declaration out of built-in module list.
      Function *LibFunc = FindFunctionInModule(newFuncName);
      assert(LibFunc && "WG builtin is not supported in built-in module");
      Function *pNewFunc = dyn_cast<Function>(
        CompilationUtils::importFunctionDecl(m_pModule, LibFunc));
      assert(pNewFunc && "Non-function object with the same signature "
                         "identified in the module");


      // 4. Prepare the call with that to function with extended parameter list
      CallInst *pNewCall = CallInst::Create(pNewFunc, ArrayRef<Value*>(params), "CallWGForItem", pWgCallInst);
      assert(pNewCall && "Couldn't create CALL instruction!");
      pNewCall->setAttributes(pWgCallInst->getAttributes());
      pNewCall->setCallingConv(pWgCallInst->getCallingConv());
      if (pWgCallInst->getDebugLoc()) {
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
        //    --- parameter list
        SmallVector<Value*, 8> params;
        params.push_back(pLoadResult);
        //    --- remangle with unique name (derived from original WG function name)
        //        [Note that the signature is as of original WG function]
        std::string finalizeFuncName = CompilationUtils::appendWorkGroupFinalizePrefix(funcName);
        //    --- create function
        //    --- get the new function declaration out of built-in modules list.
        Function *LibFunc = FindFunctionInModule(finalizeFuncName);
        assert(LibFunc && "WG builtin is not supported in built-in module");
        Function *pFinalizeFunc = dyn_cast<Function>(
          CompilationUtils::importFunctionDecl(m_pModule, LibFunc));
        assert(pFinalizeFunc && "Non-function object with the same signature identified in the module");

        // c. Create call to finalization function object
        CallInst *pFinalizeCall = CallInst::Create(pFinalizeFunc, ArrayRef<Value*>(params), "CallFinalizeWG", pWgCallInst);
        assert(pFinalizeCall && "Couldn't create CALL instruction!");

        pNewCall = pFinalizeCall;
      }

      // 7. re-initialize the alloca (in case we are in a loop) and create dummy barrier
      //    immediately AFTER it to assure we initialize once per work-group
      (void) new StoreInst(pInitValue, pResult, pWgCallInst);
      (void) m_util.createDummyBarrier(pWgCallInst);

      // 8. Discard old function call
      pWgCallInst->replaceAllUsesWith(pNewCall);
      pWgCallInst->eraseFromParent();
    }

    return !callWGSimpleFunc.empty() || !callWgFunc.empty();
  }


} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createGroupBuiltinPass() {
    return new intel::GroupBuiltin();
  }
}
