/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "BarrierUtils.h"
#include "CompilationUtils.h"
#include "MetaDataApi.h"

#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instruction.h"
#include "llvm/Instructions.h"
#include "llvm/Support/CFG.h"
#include "llvm/Version.h"

#include <set>
using namespace Intel::OpenCL::DeviceBackend;

namespace intel {


  BarrierUtils::BarrierUtils() : m_pModule(0) {
    clean();
  }

  void BarrierUtils::init(Module *pModule) {
    assert( pModule && "Trying to initialize BarrierUtils with NULL module" );
    m_pModule = pModule;

    //Get size of size_t in bits from the module
    m_uiSizeT = pModule->getPointerSize()*32;

    clean();
  }

  void BarrierUtils::clean() {
    m_localMemFenceValue = 0;
    m_barrierFunc = 0;
    m_dummyBarrierFunc = 0;
    m_getCurrWIFunc = 0;
    m_getSpecialBufferFunc = 0;
    m_getIterationCountFunc = 0;
    m_getNewLIDFunc = 0;
    m_getNewGIDFunc = 0;
    m_getGIDFunc = 0;

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
    BasicBlock *pPrevBB = NULL;
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
    return getSynchronizeType(pBB->begin());
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

  TFunctionVector& BarrierUtils::getAllFunctionsWithSynchronization() {
    //Initialize m_syncInstructions
    getAllSynchronizeInstructuons();

    //Clear old collected data!
    m_syncFunctions.clear();

    for ( TInstructionVector::iterator ii = m_syncInstructions.begin(),
      ie = m_syncInstructions.end(); ii != ie; ++ii ) {
        m_syncFunctions.push_back((*ii)->getParent()->getParent());
    }
    return m_syncFunctions;
  }

  TFunctionVector& BarrierUtils::getAllKernelFunctions() {
    //Clear old collected data!
    m_kernelFunctions.clear();
    Intel::MetaDataUtils mdUtils(m_pModule);
    if ( !mdUtils.isKernelsHasValue() ) {
      //Module contains no kernels, thus it contains no kernels
      return m_kernelFunctions;
    }

    // Get the kernels using the barrier for work group loops.
    Intel::MetaDataUtils::KernelsList::const_iterator itr = mdUtils.begin_Kernels();
    Intel::MetaDataUtils::KernelsList::const_iterator end = mdUtils.end_Kernels();
    for (; itr != end; ++itr) {
      Function *pFunc = (*itr)->getFunction();
      Intel::KernelInfoMetaDataHandle kimd = mdUtils.getKernelsInfoItem(pFunc);
      //Need to check if NoBarrierPath Value exists, it is not guaranteed that
      //KernelAnalysisPass is running in all scenarios.
      if (kimd->isNoBarrierPathHasValue() && kimd->getNoBarrierPath()) {
        //Kernel that should not be handled in Barrier path, skip it.
        continue;
      }
      //Add kernel to the list
      //Currently no check if kernel already added to the list!
      m_kernelFunctions.push_back(pFunc);
    }
    return m_kernelFunctions;
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

  Instruction* BarrierUtils::createMemFence(BasicBlock *pAtEnd) {
    /*Type *pResult = Type::getVoidTy(m_pModule->getContext());
    std::vector<Type*> funcTyArgs;
    FunctionType *pFuncTy = FunctionType::get(pResult, funcTyArgs, false);
    Constant *pNewFunc = m_pModule->getOrInsertFunction("llvm.x86.sse2.mfence", pFuncTy);
    return CallInst::Create(pNewFunc, "", pAtEnd);*/
    return NULL;
  }

  Instruction* BarrierUtils::createGetCurrWI(Instruction *pInsertBefore){
    if ( !m_getCurrWIFunc ) {
      //get_curr_WI() function is not initialized yet
      //There should not be get_curr_WI function declaration in the module
      assert( !m_pModule->getFunction(GET_CURR_WI) &&
        "get_curr_WI() instruction is origanlity declared by the module!!!" );

      //Create one
      Type *pResult = PointerType::get(
      IntegerType::get(m_pModule->getContext(), m_uiSizeT), CURR_WI_ADDR_SPACE);
      std::vector<Type*> funcTyArgs;
      m_getCurrWIFunc = createFunctionDeclaration(
        GET_CURR_WI, pResult, funcTyArgs);
      SetFunctionAttributeReadNone(m_getCurrWIFunc);
    }
    return CallInst::Create(m_getCurrWIFunc, "CurrWI", pInsertBefore);
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

  Instruction* BarrierUtils::createGetIterCount(Instruction *pInsertBefore){
    if ( !m_getIterationCountFunc ) {
      //get_iter_count() function is not initialized yet
      //There should not be get_iter_count function declaration in the module
      assert( !m_pModule->getFunction(GET_ITERATION_COUNT) &&
        "get_iter_count() instruction is origanlity declared by the module!!!" );

      //Create one
      Type *pResult = IntegerType::get(m_pModule->getContext(), m_uiSizeT);
      std::vector<Type*> funcTyArgs;
      m_getIterationCountFunc = createFunctionDeclaration(
        GET_ITERATION_COUNT, pResult, funcTyArgs);
      SetFunctionAttributeReadNone(m_getIterationCountFunc);
    }
    return CallInst::Create(m_getIterationCountFunc, "IterCount", pInsertBefore);
  }

  TInstructionVector& BarrierUtils::getAllGetLocalId() {
    if ( !m_bLIDInitialized ) {
      m_getLIDInstructions.clear();
      Function *pFunc = m_pModule->getFunction(CompilationUtils::mangledGetLID());
      if ( pFunc ) {
        for ( Value::use_iterator ui = pFunc->use_begin(),
          ue = pFunc->use_end(); ui != ue; ++ui ) {
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
        for ( Value::use_iterator ui = pFunc->use_begin(),
          ue = pFunc->use_end(); ui != ue; ++ui ) {
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

  Instruction* BarrierUtils::createNewGetLocalId(Value *pArg1, Value *pArg2, Instruction *pInsertBefore) {
    if ( !m_getNewLIDFunc ) {
      //get_new_local_id() function is not initialized yet
      //There should not be get_new_local_id function declaration in the module
      assert( !m_pModule->getFunction(GET_NEW_LID_NAME) &&
        "get_new_local_id() instruction is origanlity declared by the module!!!" );

      //Create one
      Type *pResult = IntegerType::get(m_pModule->getContext(), m_uiSizeT);
      std::vector<Type*> funcTyArgs;
      funcTyArgs.push_back(IntegerType::get(m_pModule->getContext(), 32));
      funcTyArgs.push_back(/*PointerType::get(*/IntegerType::get(m_pModule->getContext(), m_uiSizeT)/*,0)*/);
      m_getNewLIDFunc =
        createFunctionDeclaration(GET_NEW_LID_NAME, pResult, funcTyArgs);
      SetFunctionAttributeReadNone(m_getNewLIDFunc);
    }
    Value *args[2] = {pArg1, pArg2};
    return CallInst::Create(m_getNewLIDFunc, ArrayRef<Value*>(args, 2), "newLID", pInsertBefore);
  }

  Instruction* BarrierUtils::createNewGetGlobalId(Value *pArg1, Value *pArg2, Instruction *pInsertBefore) {
    if ( !m_getNewGIDFunc ) {
      //get_new_global_id() function is not initialized yet
      //There should not be get_new_global_id function declaration in the module
      assert( !m_pModule->getFunction(GET_NEW_GID_NAME) &&
        "get_new_local_id() instruction is originally declared by the module!!!" );

      //Create one
      Type *pResult = IntegerType::get(m_pModule->getContext(), m_uiSizeT);
      std::vector<Type*> funcTyArgs;
      funcTyArgs.push_back(IntegerType::get(m_pModule->getContext(), 32));
      funcTyArgs.push_back(/*PointerType::get(*/IntegerType::get(m_pModule->getContext(), m_uiSizeT)/*,0)*/);
      m_getNewGIDFunc =
        createFunctionDeclaration(GET_NEW_GID_NAME, pResult, funcTyArgs);
      SetFunctionAttributeReadNone(m_getNewGIDFunc);
    }
    Value *args[2] = {pArg1, pArg2};
    return CallInst::Create(m_getNewGIDFunc, ArrayRef<Value*>(args, 2), "newGID", pInsertBefore);
  }

  Instruction* BarrierUtils::createGetGlobalId(unsigned dim, Instruction* pInsertBefore) {
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
    return CallInst::Create(m_getGIDFunc, const_dim, "GID", pInsertBefore);
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
          for ( Value::use_iterator ui = pCalledFunc->use_begin(),
            ue = pCalledFunc->use_end(); ui != ue; ++ui ) {
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
    for ( Value::use_iterator ui = pFunc->use_begin(),
      ue = pFunc->use_end() ; ui != ue; ++ui ) {
        CallInst *pCall = dyn_cast<CallInst>(*ui);
        assert(pCall && "Something other than CallInst is using function!");
        //Add the call instruction into uses set
        usesSet.insert(pCall);
    }
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
#if (LLVM_VERSION == 3200) || (LLVM_VERSION == 3425)
    AttrListPtr barrier_Func_PAL;
#else
    AttributeSet barrier_Func_PAL;
#endif
    pNewFunc->setAttributes(barrier_Func_PAL);

    assert( pNewFunc && "Failed to create new function declaration" );

    return pNewFunc;
  }

  void BarrierUtils::SetFunctionAttributeReadNone(Function* pFunc) {
#if LLVM_VERSION == 3200
    AttrListPtr func_factorial_PAL;
    SmallVector<AttributeWithIndex, 4> Attrs;
    AttributeWithIndex PAWI;
    PAWI.Index = 4294967295U;
    AttrBuilder attBuilder;
    attBuilder.addAttribute(Attributes::None).addAttribute(Attributes::NoUnwind).addAttribute(Attributes::ReadNone) /* .addAttribute(Attribute::UWTable) */;
    PAWI.Attrs = Attributes::get(pFunc->getContext(), attBuilder);
    Attrs.push_back(PAWI);
    func_factorial_PAL = AttrListPtr::get(pFunc->getContext(), Attrs);
#elif LLVM_VERSION == 3425
    AttrListPtr func_factorial_PAL;
    SmallVector<AttributeWithIndex, 4> Attrs;
    AttributeWithIndex PAWI;
    PAWI.Index = 4294967295U;
    PAWI.Attrs = Attribute::None  | Attribute::NoUnwind | Attribute::ReadNone/* | Attribute::UWTable*/;
    Attrs.push_back(PAWI);
    func_factorial_PAL = AttrListPtr::get(Attrs);
#else
    AttributeSet func_factorial_PAL;
    AttrBuilder attBuilder;
    attBuilder.addAttribute(Attribute::None).addAttribute(Attribute::NoUnwind).addAttribute(Attribute::ReadNone) /* .addAttribute(Attribute::UWTable) */;
    func_factorial_PAL = AttributeSet::get(pFunc->getContext(), ~0, attBuilder);
#endif
    pFunc->setAttributes(func_factorial_PAL);
  }
} // namespace intel

