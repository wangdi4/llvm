// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "BarrierUtils.h"
#include "CompilationUtils.h"
#include "MetadataAPI.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"

#include <vector>

using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::MetadataAPI;

namespace intel {


  BarrierUtils::BarrierUtils() :
      m_pModule(0), m_uiSizeT(0), m_SizetTy(nullptr), m_I32Ty(nullptr) {
    clean();
  }

  void BarrierUtils::init(Module *pModule) {
    assert( pModule && "Trying to initialize BarrierUtils with NULL module" );
    m_pModule = pModule;

    //Get size of size_t in bits from the module
    clean();
    m_uiSizeT = pModule->getDataLayout().getPointerSizeInBits(0);
    assert(m_uiSizeT == 32 || m_uiSizeT == 64);
    m_I32Ty = Type::getInt32Ty(pModule->getContext());
    m_SizetTy = IntegerType::get(pModule->getContext(), m_uiSizeT);
  }

  void BarrierUtils::clean() {
    m_localMemFenceValue = 0;
    m_barrierFunc = 0;
    m_dummyBarrierFunc = 0;
    m_getSpecialBufferFunc = 0;
    m_getGIDFunc = 0;
    m_getBaseGIDFunc = 0;
    m_getLocalSizeFunc = 0;

    //No need to clear these values, each get clears them first
    //m_syncInstructions.clear();
    //m_syncBasicBlocks.clear();
    //m_syncFunctions.clear();
    //m_kernelFunctions.clear();

    m_bSyncDataInitialized = false;
    //No need to clear these values,
    //initializing indecator to false assure clear them first
    //m_barriers.clear();
    //m_dummyBarriers.clear();
    //m_fibers.clear();

    m_bLIDInitialized = false;
    m_bGIDInitialized = false;
    //No need to clear these values,
    //initializing indecator to false assure clear them first
    //m_getLIDInstructions.clear();
    //m_getGIDInstructions.clear();

    m_bNonInlinedCallsInitialized = false;
    //No need to clear these values,
    //initializing indecator to false assure clear them first
    //m_functionsWithNonInlinedCalls.clear();
  }

  BasicBlock* BarrierUtils::findBasicBlockOfUsageInst(Value *pVal, Instruction *pUserInst) {
    if( !isa<PHINode>(pUserInst) ) {
      //Not PHINode, return usage instruction basic block
      return pUserInst->getParent();
    }
    //Usage is a PHINode, find previous basic block according to pVal
    PHINode *pPhiNode = cast<PHINode>(pUserInst);
    BasicBlock *pPrevBB = nullptr;
    for ( pred_iterator bi = pred_begin(pPhiNode->getParent()),
      be = pred_end(pPhiNode->getParent()); bi != be; ++bi ) {
        BasicBlock *pBB = *bi;
        Value *pPHINodeVal = pPhiNode->getIncomingValueForBlock(pBB);
        if ( pPHINodeVal == pVal ) {
          //pBB is the previous basic block
          assert( !pPrevBB && "PHINode is using pVal twice!" );
          pPrevBB = pBB;
        }
    }
    assert( pPrevBB && "Failed to find previous basic block!" );
    return pPrevBB;
  }

  SYNC_TYPE BarrierUtils::getSynchronizeType(Instruction *pInst) {
    //Initialize sync data if it is not done yet
    initializeSyncData();

    if ( !isa<CallInst>(pInst) ) {
      //Not a call instruction, cannot be a synchronize instruction
      return SYNC_TYPE_NONE;
    }
    if ( m_barriers.count(pInst) ) {
      //It is a barrier instruction
      return SYNC_TYPE_BARRIER;
    }
    if ( m_dummyBarriers.count(pInst) ) {
      //It is a dummyBarrier instruction
      return SYNC_TYPE_DUMMY_BARRIER;
    }
    if ( m_fibers.count(pInst) ) {
      //It is a fiber instruction
      return SYNC_TYPE_FIBER;
    }
    return SYNC_TYPE_NONE;
  }

  SYNC_TYPE BarrierUtils::getSynchronizeType(BasicBlock *pBB) {
    return getSynchronizeType(&*pBB->begin());
  }

  TInstructionVector& BarrierUtils::getAllSynchronizeInstructuons() {
    //Initialize sync data if it is not done yet
    initializeSyncData();

    //Clear old collected data!
    m_syncInstructions.clear();

    //Insert all barrier instructions
    for ( TInstructionSet::iterator ii = m_barriers.begin(),
      ie = m_barriers.end(); ii != ie; ++ii ) {
        m_syncInstructions.push_back(*ii);
    }
    //Insert all dummyBarrier instructions
    for ( TInstructionSet::iterator ii = m_dummyBarriers.begin(),
      ie = m_dummyBarriers.end(); ii != ie; ++ii ) {
        m_syncInstructions.push_back(*ii);
    }
    //Insert all fiber instructions
    for ( TInstructionSet::iterator ii = m_fibers.begin(),
      ie = m_fibers.end(); ii != ie; ++ii ) {
        m_syncInstructions.push_back(*ii);
    }

    return m_syncInstructions;
  }

  TInstructionVector& BarrierUtils::getWGCallInstructions(CALL_BI_TYPE type) {

    //Clear old collected data
    m_WGcallInstructions.clear();

    // Scan external function definitions in the module
    for ( Module::iterator fi = m_pModule->begin(), fe = m_pModule->end(); fi != fe; ++fi ) {
      Function *pFunc = &*fi;
      if( !pFunc->isDeclaration() ) {
        //Built-in functions assumed to be declarations at this point.
        continue;
      }
      std::string funcName = pFunc->getName().str();
      if((CALL_BI_TYPE_WG == type && CompilationUtils::isWorkGroupBuiltin(funcName)) ||
         (CALL_BI_TYPE_WG_ASYNC_OR_PIPE == type && CompilationUtils::isWorkGroupAsyncOrPipeBuiltin(funcName, m_pModule))) {
        //Module contains declaration of a WG function built-in, fix its usages.
        Function::user_iterator ui = pFunc->user_begin();
        Function::user_iterator ue = pFunc->user_end();
        for ( ; ui != ue; ++ui ) {
          CallInst *pCallInst = dyn_cast<CallInst>(*ui);
          if( !pCallInst ) {
            assert(false && "usage of work-group built-in is not a call instruction!");
            continue;
          }
          //Found a call instruction to work-group built-in, collect it.
          m_WGcallInstructions.push_back(pCallInst);
        }
      }
    }

    return m_WGcallInstructions;
  }

  //TBasicBlockVector& BarrierUtils::getAllSynchronizeBasicBlocks() {
  //  //Initialize m_syncInstructions
  //  getAllSynchronizeInstructuons();

  //  //Clear old collected data!
  //  m_syncBasicBlocks.clear();

  //  for ( TInstructionVector::iterator ii = m_syncInstructions.begin(),
  //    ie = m_syncInstructions.end(); ii != ie; ++ii ) {
  //      m_syncBasicBlocks.push_back((*ii)->getParent());
  //  }
  //  return m_syncBasicBlocks;
  //}

  TFunctionSet& BarrierUtils::getAllFunctionsWithSynchronization() {
    //Initialize m_syncInstructions
    getAllSynchronizeInstructuons();

    //Clear old collected data!
    m_syncFunctions.clear();

    for ( TInstructionVector::iterator ii = m_syncInstructions.begin(),
      ie = m_syncInstructions.end(); ii != ie; ++ii ) {
        m_syncFunctions.insert((*ii)->getParent()->getParent());
    }
    return m_syncFunctions;
  }

  TFunctionVector BarrierUtils::getAllKernelsAndVectorizedCounterparts(
      const SmallVectorImpl<Function *> &KernelList) {
    TFunctionVector Result;

    for (auto *F : KernelList) {
      Result.push_back(F);
      auto VectorizedKernelMetadata =
          KernelInternalMetadataAPI(F).VectorizedKernel;
      if (VectorizedKernelMetadata.hasValue() && VectorizedKernelMetadata.get())
        Result.push_back(VectorizedKernelMetadata.get());
    }

    // rely on move ctor.
    return Result;
  }

  TFunctionVector& BarrierUtils::getAllKernelsWithBarrier() {
    auto Kernels = KernelList(m_pModule);

    //Clear old collected data!
    m_kernelFunctions.clear();
    if (Kernels.empty()) {
      return m_kernelFunctions;
    }

    // Get the kernels using the barrier for work group loops.
    for (auto pFunc : Kernels) {
      auto kimd = KernelInternalMetadataAPI(pFunc);
      //Need to check if NoBarrierPath Value exists, it is not guaranteed that
      //KernelAnalysisPass is running in all scenarios.
      if (kimd.NoBarrierPath.hasValue() && kimd.NoBarrierPath.get()) {
        //Kernel that should not be handled in Barrier path, skip it.
        continue;
      }
      //Add kernel to the list
      //Currently no check if kernel already added to the list!
      m_kernelFunctions.push_back(pFunc);
    }

    // collect functions to process
    auto TodoList = getAllKernelsAndVectorizedCounterparts(Kernels.getList());

    for (auto pFunc : TodoList) {
      auto kimd = KernelInternalMetadataAPI(pFunc);
      m_kernelVectorizationWidths[pFunc] =
          kimd.VectorizedWidth.hasValue() ? kimd.VectorizedWidth.get() : 1;
    }

    return m_kernelFunctions;
  }

  unsigned BarrierUtils::getKernelVectorizationWidth(const Function *F) const {
    TFunctionToUnsigned::const_iterator I = m_kernelVectorizationWidths.find(F);
    if (I == m_kernelVectorizationWidths.end()) return 1;
    return I->second;
  }

  Instruction* BarrierUtils::createBarrier(Instruction *pInsertBefore){
    if ( !m_barrierFunc ) {
      //Barrier function is not initialized yet
      //Check if there is a declaration in the module
      m_barrierFunc = m_pModule->getFunction(CompilationUtils::mangledBarrier());
    }
    if ( !m_barrierFunc ) {
      //Module has no barrier declaration
      //Create one
      Type *pResult = Type::getVoidTy(m_pModule->getContext());
      std::vector<Type*> funcTyArgs;
      funcTyArgs.push_back(IntegerType::get(m_pModule->getContext(), 32));
      m_barrierFunc =
        createFunctionDeclaration(CompilationUtils::mangledBarrier(), pResult, funcTyArgs);
      m_barrierFunc->setAttributes(m_barrierFunc->getAttributes().addAttribute(
        m_barrierFunc->getContext(), AttributeList::FunctionIndex, Attribute::NoDuplicate));
    }
    if ( !m_localMemFenceValue ) {
      //LocalMemFenceValue is not initialized yet
      //Create one
      Type *memFenceType = m_barrierFunc->getFunctionType()->getParamType(0);
      m_localMemFenceValue = ConstantInt::get(memFenceType, CLK_LOCAL_MEM_FENCE);
    }
    return CallInst::Create(m_barrierFunc, m_localMemFenceValue, "", pInsertBefore);
  }

  Instruction* BarrierUtils::createDummyBarrier(Instruction *pInsertBefore){
    if ( !m_dummyBarrierFunc ) {
      //Dummy Barrier function is not initialized yet
      //Check if there is a declaration in the module
      m_dummyBarrierFunc = m_pModule->getFunction(DUMMY_BARRIER_FUNC_NAME);
    }
    if ( !m_dummyBarrierFunc ) {
      //Module has no Dummy barrier declaration
      //Create one
      Type *pResult = Type::getVoidTy(m_pModule->getContext());
      std::vector<Type*> funcTyArgs;
      m_dummyBarrierFunc = createFunctionDeclaration(
        DUMMY_BARRIER_FUNC_NAME, pResult, funcTyArgs);
    }
    return CallInst::Create(m_dummyBarrierFunc, "", pInsertBefore);
  }

  bool BarrierUtils::isDummyBarrierCall(CallInst *pCallInstr) {
    assert(pCallInstr && "Instruction should not be NULL!");
    //Initialize sync data if it is not done yet
    initializeSyncData();
    return m_dummyBarriers.count(pCallInstr);
  }

  bool BarrierUtils::isBarrierCall(CallInst *pCallInstr) {
    assert(pCallInstr && "Instruction should not be NULL!");
    //Initialize sync data if it is not done yet
    initializeSyncData();
    return m_barriers.count(pCallInstr);
  }

  Instruction* BarrierUtils::createMemFence(IRBuilder<> &B) {
    /*Type *pResult = Type::getVoidTy(m_pModule->getContext());
    std::vector<Type*> funcTyArgs;
    FunctionType *pFuncTy = FunctionType::get(pResult, funcTyArgs, false);
    Constant *pNewFunc = m_pModule->getOrInsertFunction("llvm.x86.sse2.mfence", pFuncTy);
    return CallInst::Create(pNewFunc, "", pAtEnd);*/
    return nullptr;
  }

  Instruction* BarrierUtils::createGetSpecialBuffer(Instruction *pInsertBefore){
    if ( !m_getSpecialBufferFunc ) {
      //get_special_buffer() function is not initialized yet
      //There should not be get_special_buffer function declaration in the module
      assert( !m_pModule->getFunction(GET_SPECIAL_BUFFER) &&
        "get_special_buffer() instruction is origanlity declared by the module!!!" );

      //Create one
      Type *pResult = PointerType::get(
      IntegerType::get(m_pModule->getContext(), 8), SPECIAL_BUFFER_ADDR_SPACE);
      std::vector<Type*> funcTyArgs;
      m_getSpecialBufferFunc = createFunctionDeclaration(
        GET_SPECIAL_BUFFER, pResult, funcTyArgs);
      SetFunctionAttributeReadNone(m_getSpecialBufferFunc);
    }
    return CallInst::Create(m_getSpecialBufferFunc, "pSB", pInsertBefore);
  }

  TInstructionVector& BarrierUtils::getAllGetLocalId() {
    if ( !m_bLIDInitialized ) {
      m_getLIDInstructions.clear();
      Function *pFunc = m_pModule->getFunction(CompilationUtils::mangledGetLID());
      if ( pFunc ) {
        for ( Value::user_iterator ui = pFunc->user_begin(),
          ue = pFunc->user_end(); ui != ue; ++ui ) {
            CallInst *pInstCall = dyn_cast<CallInst>(*ui);
            assert( pInstCall &&
              "Something other than CallInst is using get_local_id function!" );
            m_getLIDInstructions.push_back(pInstCall);
        }
      }
      m_bLIDInitialized = true;
    }
    return m_getLIDInstructions;
  }

  TInstructionVector& BarrierUtils::getAllGetGlobalId() {
    if ( !m_bGIDInitialized ) {
      m_getGIDInstructions.clear();
      Function *pFunc = m_pModule->getFunction(CompilationUtils::mangledGetGID());
      if ( pFunc ) {
        for ( Value::user_iterator ui = pFunc->user_begin(),
          ue = pFunc->user_end(); ui != ue; ++ui ) {
            CallInst *pInstCall = dyn_cast<CallInst>(*ui);
            assert( pInstCall &&
              "Something other than CallInst is using get_globalal_id function!" );
            m_getGIDInstructions.push_back(pInstCall);
        }
      }
      m_bGIDInitialized = true;
    }
    return m_getGIDInstructions;
  }

  Instruction* BarrierUtils::createGetBaseGlobalId(Value* dim, Instruction* pInsertBefore) {
    const std::string FuncName = GET_BASE_GID;
    if ( !m_getBaseGIDFunc ) {
      // Get existing get_global_id function
      m_getBaseGIDFunc = m_pModule->getFunction(FuncName);
    }
    if (!m_getBaseGIDFunc) {
      //Create one
      Type *pResult = IntegerType::get(m_pModule->getContext(), m_uiSizeT);
      std::vector<Type*> funcTyArgs;
      funcTyArgs.push_back(IntegerType::get(m_pModule->getContext(), 32));
      m_getBaseGIDFunc = createFunctionDeclaration(FuncName, pResult, funcTyArgs);
      SetFunctionAttributeReadNone(m_getBaseGIDFunc);
    }
    return CallInst::Create(m_getBaseGIDFunc, dim,
                            AppendWithDimension("BaseGlobalId_", dim),
                            pInsertBefore);
  }

  Instruction* BarrierUtils::createGetGlobalId(unsigned dim, IRBuilder<> &B) {
    const std::string strGID = CompilationUtils::mangledGetGID();
    if ( !m_getGIDFunc ) {
      // Get existing get_global_id function
      m_getGIDFunc = m_pModule->getFunction(strGID);
    }
    if (!m_getGIDFunc) {
      //Create one
      Type *pResult = IntegerType::get(m_pModule->getContext(), m_uiSizeT);
      std::vector<Type*> funcTyArgs;
      funcTyArgs.push_back(IntegerType::get(m_pModule->getContext(), 32));
      m_getGIDFunc = createFunctionDeclaration(strGID, pResult, funcTyArgs);
      SetFunctionAttributeReadNone(m_getGIDFunc);
    }
    Type* uint_type = IntegerType::get(m_pModule->getContext(), 32);
    Value* const_dim = ConstantInt::get(uint_type, dim, false);
    return B.CreateCall(m_getGIDFunc, const_dim,
                            AppendWithDimension("GlobalID_", dim));
  }

  bool BarrierUtils::doesCallModuleFunction(Function *pFunc) {
    if ( !m_bNonInlinedCallsInitialized ) {
      m_functionsWithNonInlinedCalls.clear();
      //Collect all functions with non inlined calls, i.e.
      //functions that calls other functions from this module
      for ( Module::iterator fi = m_pModule->begin(),
        fe = m_pModule->end(); fi != fe; ++fi ) {
          Function *pCalledFunc = &*fi;
          if ( pCalledFunc->isDeclaration() ) {
            //It is not an internal function, only delaration
            continue;
          }
          for ( Value::user_iterator ui = pCalledFunc->user_begin(),
            ue = pCalledFunc->user_end(); ui != ue; ++ui ) {
              CallInst *pCallInst = dyn_cast<CallInst>(*ui);
              // usage of pFunc can be a global variable!
              if( !pCallInst ) {
                // usage of pFunc is not a CallInst
                continue;
              }
              Function *pCallingFunc = pCallInst->getParent()->getParent();
              if ( !m_functionsWithNonInlinedCalls.count(pCallingFunc) ) {
                m_functionsWithNonInlinedCalls.insert(pCallingFunc);
              }
          }
      }
      m_bNonInlinedCallsInitialized = true;
    }
    return m_functionsWithNonInlinedCalls.count(pFunc);
  }

  void BarrierUtils::initializeSyncData() {
    if( m_bSyncDataInitialized ) {
      //Sync data already initialized
      return;
    }

    //Clear old collected data!
    m_barriers.clear();
    m_dummyBarriers.clear();
    m_fibers.clear();

    //Find all calls to barrier()
    findAllUsesOfFunc(CompilationUtils::mangledBarrier(), m_barriers);
    //Find all calls to work_group_barrier()
    findAllUsesOfFunc(CompilationUtils::mangledWGBarrier(
      CompilationUtils::WG_BARRIER_NO_SCOPE), m_barriers);
    findAllUsesOfFunc(CompilationUtils::mangledWGBarrier(
      CompilationUtils::WG_BARRIER_WITH_SCOPE), m_barriers);
    //Find all calls to dummyBarrier()
    findAllUsesOfFunc(DUMMY_BARRIER_FUNC_NAME, m_dummyBarriers);
    //Find all calls to fiber()
    findAllUsesOfFunc(FIBER_FUNC_NAME, m_fibers);

    m_bSyncDataInitialized = true;
  }

  void BarrierUtils::findAllUsesOfFunc(const llvm::StringRef& name, TInstructionSet &usesSet) {
    //Check if given function name is declared in the module
    Function *pFunc = m_pModule->getFunction(name);
    if ( !pFunc ) {
      //Function is not declared
      return;
    }
    //Find all calls to given function name
    for ( Value::user_iterator ui = pFunc->user_begin(),
      ue = pFunc->user_end() ; ui != ue; ++ui ) {
        CallInst *pCall = dyn_cast<CallInst>(*ui);
        assert(pCall && "Something other than CallInst is using function!");
        //Add the call instruction into uses set
        usesSet.insert(pCall);
    }
  }

  Instruction *BarrierUtils::createGetLocalSize(unsigned dim,
                                                Instruction *pInsertBefore) {
    // Callee's declaration: size_t get_local_size(uint dimindx);
    const std::string strGID = CompilationUtils::mangledGetLocalSize();
    if ( !m_getLocalSizeFunc ) {
      // Get existing get_local_size function
      m_getLocalSizeFunc = m_pModule->getFunction(strGID);
    }
    if (!m_getLocalSizeFunc) {
      //Create one
      Type *pResult = m_SizetTy;
      std::vector<Type*> funcTyArgs;
      funcTyArgs.push_back(m_I32Ty);
      m_getLocalSizeFunc =
          createFunctionDeclaration(strGID, pResult, funcTyArgs);
      SetFunctionAttributeReadNone(m_getLocalSizeFunc);
    }
    Value* const_dim = ConstantInt::get(m_I32Ty, dim, false);
    return CallInst::Create(m_getLocalSizeFunc, const_dim,
                            AppendWithDimension("LocalSize_", dim),
                            pInsertBefore);
}

  Function* BarrierUtils::createFunctionDeclaration(const llvm::Twine& name, Type *pResult, std::vector<Type*>& funcTyArgs) {
    FunctionType *pFuncTy = FunctionType::get(
      /*Result=*/pResult,
      /*Params=*/funcTyArgs,
      /*isVarArg=*/false);

    assert( pFuncTy && "Failed to create new function type" );

    Function *pNewFunc = Function::Create(
      /*Type=*/pFuncTy,
      /*Linkage=*/GlobalValue::ExternalLinkage,
      /*Name=*/name, m_pModule); //(external, no body)
    pNewFunc->setCallingConv(CallingConv::C);

    assert( pNewFunc && "Failed to create new function declaration" );

    return pNewFunc;
  }

  void BarrierUtils::SetFunctionAttributeReadNone(Function* pFunc) {
    AttrBuilder attBuilder;
    attBuilder.addAttribute(Attribute::NoUnwind).addAttribute(Attribute::ReadNone) /* .addAttribute(Attribute::UWTable) */;
    auto func_factorial_PAL =
      AttributeList::get(pFunc->getContext(), AttributeList::FunctionIndex, attBuilder);
    pFunc->setAttributes(func_factorial_PAL);
  }
} // namespace intel

