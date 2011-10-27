/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/
#include "BarrierUtils.h"

#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instruction.h"
#include "llvm/Instructions.h"
#include "llvm/Support/CFG.h"

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
    PHINode *pPhiNode =  dyn_cast<PHINode>(pUserInst);
    BasicBlock *pPrevBB = NULL;
    for ( pred_iterator bi = pred_begin(pPhiNode->getParent()),
      be = pred_end(pPhiNode->getParent()); bi != be; ++bi ) {
        BasicBlock *pBB = dyn_cast<BasicBlock>(*bi);
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
    //Check for some common module errors, before actually diving in
    NamedMDNode *pOpenCLMetadata = m_pModule->getNamedMetadata("opencl.kernels");
    if ( !pOpenCLMetadata ) {
      //Module contains no MetaData, thus it contains no kernels
      return m_kernelFunctions;
    }

    unsigned int numOfKernels = pOpenCLMetadata->getNumOperands();
    if ( numOfKernels == 0 ) {
      //Module contains no kernels
      return m_kernelFunctions;
    }

    //List all kernels in module
    for ( unsigned int i = 0, e = numOfKernels; i != e; ++i ) {
      MDNode *elt = pOpenCLMetadata->getOperand(i);
      Value *field0 = elt->getOperand(0)->stripPointerCasts();
      if ( Function *pKernelFunc = dyn_cast<Function>(field0) ) {
        //Add kernel to the list
        //Currently no check if kernel already added to the list!
        m_kernelFunctions.push_back(pKernelFunc);

        //Check if there is a vectorized version of this kernel
        std::string vectorizedKernelName = 
          std::string(VECTORIZED_KERNEL_PREFIX) + pKernelFunc->getName().data();
        Function *pVectorizedKernelFunc = m_pModule->getFunction(vectorizedKernelName);
        if ( pVectorizedKernelFunc ) {
          //Add vectorized kernel function to list
          m_kernelFunctions.push_back(pVectorizedKernelFunc);
        }
      }
    }
    return m_kernelFunctions;
  }

  Instruction* BarrierUtils::createBarrier(Instruction *pInsertBefore){
    if ( !m_barrierFunc ) {
      //Barrier function is not initialized yet
      //Check if there is a declaration in the module
      m_barrierFunc = m_pModule->getFunction(BARRIER_FUNC_NAME);
    }
    if ( !m_barrierFunc ) {
      //Module has no barrier declaration
      //Create one
      const Type *pResult = Type::getVoidTy(m_pModule->getContext());
      std::vector<const Type*> funcTyArgs;
      funcTyArgs.push_back(IntegerType::get(m_pModule->getContext(), m_uiSizeT));
      m_barrierFunc = 
        createFunctionDeclaration(BARRIER_FUNC_NAME, pResult, funcTyArgs);
    }
    if ( !m_localMemFenceValue ) {
      //LocalMemFenceValue is not initialized yet
      //Create one
      m_localMemFenceValue = ConstantInt::get(
        m_pModule->getContext(), APInt(m_uiSizeT, CLK_LOCAL_MEM_FENCE));
    }
    return CallInst::Create(m_barrierFunc, m_localMemFenceValue, "", pInsertBefore);
  }

  Instruction* BarrierUtils::createDummyBarrier(Instruction *pInsertBefore){
    if ( !m_dummyBarrierFunc ) {
      //Dummy Barrier function is not initialized yet
      //There should not be dummyBarrier function declaration in the module
      assert( !m_pModule->getFunction(DUMMY_BARRIER_FUNC_NAME) && 
        "dummyBarrier() instruction is origanlity declared by the module!!!" );

      //Create one
      const Type *pResult = Type::getVoidTy(m_pModule->getContext());
      std::vector<const Type*> funcTyArgs;
      m_dummyBarrierFunc = createFunctionDeclaration(
        DUMMY_BARRIER_FUNC_NAME, pResult, funcTyArgs);
    }
    return CallInst::Create(m_dummyBarrierFunc, "", pInsertBefore);
  }

  Instruction* BarrierUtils::createMemFence(BasicBlock *pAtEnd) {
    const Type *pResult = Type::getVoidTy(m_pModule->getContext());
    std::vector<const Type*> funcTyArgs;
    FunctionType *pFuncTy = FunctionType::get(pResult, funcTyArgs, false);
    Constant *pNewFunc = m_pModule->getOrInsertFunction("llvm.x86.sse2.mfence", pFuncTy);
    return CallInst::Create(pNewFunc, "", pAtEnd);
  }

  Instruction* BarrierUtils::createGetCurrWI(Instruction *pInsertBefore){
    if ( !m_getCurrWIFunc ) {
      //get_curr_WI() function is not initialized yet
      //There should not be get_curr_WI function declaration in the module
      assert( !m_pModule->getFunction(GET_CURR_WI) && 
        "get_curr_WI() instruction is origanlity declared by the module!!!" );

      //Create one
      const Type *pResult = PointerType::get(
        IntegerType::get(m_pModule->getContext(), m_uiSizeT), CURR_WI_ADDR_SPACE);
      std::vector<const Type*> funcTyArgs;
      m_getCurrWIFunc = createFunctionDeclaration(
        GET_CURR_WI, pResult, funcTyArgs);
    }
    return CallInst::Create(m_getCurrWIFunc, "pCurrWI", pInsertBefore);
  }

  Instruction* BarrierUtils::createGetSpecialBuffer(Instruction *pInsertBefore){
    if ( !m_getSpecialBufferFunc ) {
      //get_special_buffer() function is not initialized yet
      //There should not be get_special_buffer function declaration in the module
      assert( !m_pModule->getFunction(GET_SPECIAL_BUFFER) && 
        "get_special_buffer() instruction is origanlity declared by the module!!!" );

      //Create one
      const Type *pResult = PointerType::get(
        IntegerType::get(m_pModule->getContext(), 8), SPECIAL_BUFFER_ADDR_SPACE);
      std::vector<const Type*> funcTyArgs;
      m_getSpecialBufferFunc = createFunctionDeclaration(
        GET_SPECIAL_BUFFER, pResult, funcTyArgs);
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
      const Type *pResult = IntegerType::get(m_pModule->getContext(), m_uiSizeT);
      std::vector<const Type*> funcTyArgs;
      m_getIterationCountFunc = createFunctionDeclaration(
        GET_ITERATION_COUNT, pResult, funcTyArgs);
    }
    return CallInst::Create(m_getIterationCountFunc, GET_ITER_COUNT, pInsertBefore);
  }

  TInstructionVector& BarrierUtils::getAllGetLocalId() {
    if ( !m_bLIDInitialized ) {
      m_getLIDInstructions.clear();
      Function *pFunc = m_pModule->getFunction(GET_LID_NAME);
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
      Function *pFunc = m_pModule->getFunction(GET_GID_NAME);
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
      const Type *pResult = IntegerType::get(m_pModule->getContext(), m_uiSizeT);
      std::vector<const Type*> funcTyArgs;
      funcTyArgs.push_back(IntegerType::get(m_pModule->getContext(), 32));
      funcTyArgs.push_back(/*PointerType::get(*/IntegerType::get(m_pModule->getContext(), m_uiSizeT)/*,0)*/);
      m_getNewLIDFunc = 
        createFunctionDeclaration(GET_NEW_LID_NAME, pResult, funcTyArgs);
    }
    Value *args[2] = {pArg1, pArg2};
    return CallInst::Create(m_getNewLIDFunc, args, args+2, "newLID", pInsertBefore);
  }

  Instruction* BarrierUtils::createNewGetGlobalId(Value *pArg1, Value *pArg2, Instruction *pInsertBefore) {
    if ( !m_getNewGIDFunc ) {
      //get_new_global_id() function is not initialized yet
      //There should not be get_new_global_id function declaration in the module
      assert( !m_pModule->getFunction(GET_NEW_GID_NAME) && 
        "get_new_local_id() instruction is origanlity declared by the module!!!" );

      //Create one
      const Type *pResult = IntegerType::get(m_pModule->getContext(), m_uiSizeT);
      std::vector<const Type*> funcTyArgs;
      funcTyArgs.push_back(IntegerType::get(m_pModule->getContext(), 32));
      funcTyArgs.push_back(/*PointerType::get(*/IntegerType::get(m_pModule->getContext(), m_uiSizeT)/*,0)*/);
      m_getNewGIDFunc = 
        createFunctionDeclaration(GET_NEW_GID_NAME, pResult, funcTyArgs);
    }
    Value *args[2] = {pArg1, pArg2};
    return CallInst::Create(m_getNewGIDFunc, args, args+2, "newGID", pInsertBefore);
  }

  bool BarrierUtils::doesCallModuleFunction(Function *pFunc) {
    if ( !m_bNonInlinedCallsInitialized ) {
      m_functionsWithNonInlinedCalls.clear();
      //Collect all functions with non inlined calls, i.e.
      //functions that calls other functions from this module
      for ( Module::iterator fi = m_pModule->begin(),
        fe = m_pModule->end(); fi != fe; ++fi ) {
          Function *pCalledFunc = dyn_cast<Function>(&*fi);
          if ( pCalledFunc->isDeclaration() ) {
            //It is not an internal function, only delaration
            continue;
          }
          for ( Value::use_iterator ui = pCalledFunc->use_begin(),
            ue = pCalledFunc->use_end(); ui != ue; ++ui ) {
              CallInst *pCallInst = dyn_cast<CallInst>(*ui);
              assert( pCallInst && "Something other than CallInst is using a function!" );
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
    //Find all calls to barrier()
    findAllUsesOfFunc(BARRIER_FUNC_NAME, m_barriers);
    //Find all calls to dummyBarrier()
    findAllUsesOfFunc(DUMMY_BARRIER_FUNC_NAME, m_dummyBarriers);
    //Find all calls to fiber()
    findAllUsesOfFunc(FIBER_FUNC_NAME, m_fibers);

    m_bSyncDataInitialized = true;
  }

  void BarrierUtils::findAllUsesOfFunc(const llvm::StringRef& name, TInstructionSet &usesSet) {
    //Clear old collected data!
    usesSet.clear();

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

  Function* BarrierUtils::createFunctionDeclaration(const llvm::Twine& name, const Type *pResult, std::vector<const Type*>& funcTyArgs) {
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
    AttrListPtr barrier_Func_PAL;
    pNewFunc->setAttributes(barrier_Func_PAL);

    assert( pNewFunc && "Failed to create new function declaration" );
    return pNewFunc;
  }
} // namespace intel

