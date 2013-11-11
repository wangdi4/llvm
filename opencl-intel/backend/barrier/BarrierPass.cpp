/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "BarrierPass.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "MetaDataApi.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Transforms/Utils/Cloning.h"

namespace intel {

  char Barrier::ID = 0;

  OCL_INITIALIZE_PASS_BEGIN(Barrier, "B-Barrier", "Barrier Pass - Handle special values & replace barrier/fiber with internal loop over WIs", false, true)
  OCL_INITIALIZE_PASS_DEPENDENCY(DataPerBarrier)
  OCL_INITIALIZE_PASS_DEPENDENCY(DataPerValue)
  OCL_INITIALIZE_PASS_END(Barrier, "B-Barrier", "Barrier Pass - Handle special values & replace barrier/fiber with internal loop over WIs", false, true)

  Barrier::Barrier(bool isNativeDebug) : ModulePass(ID), m_isNativeDBG(isNativeDebug) {
    initializeBarrierPass(*llvm::PassRegistry::getPassRegistry());
  }

  bool Barrier::runOnModule(Module &M) {
    //Get Analysis data
    m_pDataPerBarrier = &getAnalysis<DataPerBarrier>();
    m_pDataPerValue = &getAnalysis<DataPerValue>();

    //Initialize barrier utils class with current module
    m_util.init(&M);

    m_pContext = &M.getContext();
    //Initialize the side of size_t
    m_uiSizeT = M.getPointerSize()*32;
    m_sizeTType = IntegerType::get(*m_pContext, m_uiSizeT);

    //Update Map with structure stride size for each kernel
    updateStructureStride(M);

    //Find all functions that call synchronize instructions
    TFunctionSet& functionsWithSync = m_util.getAllFunctionsWithSynchronization();
    //Collect data for each function with synchronize instruction
    for ( TFunctionSet::iterator fi = functionsWithSync.begin(),
        fe = functionsWithSync.end(); fi != fe; ++fi ) {
      Function* pFunc = *fi;

      //Check if function has no synchronize instructions!
      assert ( m_pDataPerBarrier->hasSyncInstruction(pFunc) &&
        "Cannot reach here with function that has no barrier");

      //Create new BB at the begining of the function for declarations
      pFunc->begin()->splitBasicBlock(pFunc->getEntryBlock().begin(), "FirstBB");
      //Initialize the argument values
      //This is needed for optimize pCurrWI calculation
      bool hasNoInternalCalls = !m_util.doesCallModuleFunction(pFunc);
      createBarrierKeyValues(pFunc, hasNoInternalCalls);
    }

    //Fix non inlined internal functions that need special handling
    //Run over functions with synchronize instruction:
    // 1. Handle non-inline functions
    // 2. Handle call instructions to non-inline functions
    for ( TFunctionSet::iterator fi = functionsWithSync.begin(),
        fe = functionsWithSync.end(); fi != fe; ++fi ) {
      Function* pFuncToFix = *fi;
      //Create fixed version of the function with extra offset parameters
      fixNonInlineFunction(pFuncToFix);

      //Run over old uses of pFuncToFix and replace with call to pNewFunc
      for ( Value::use_iterator ui = pFuncToFix->use_begin(),
          ue = pFuncToFix->use_end(); ui != ue; ++ui ) {
        CallInst *pCallInst = dyn_cast<CallInst>(*ui);
        if ( !pCallInst ) continue;
        //Handle call instruction operands and return value, if needed.
        fixCallInstruction(pCallInst);
      }
    }

    //Run over functions with synchronize instruction:
    // 1. Handle Values from Group-A, Group-B.1 and Group-B.2
    // 2. Hanlde synchronize instructions
    for ( TFunctionSet::iterator fi = functionsWithSync.begin(),
        fe = functionsWithSync.end(); fi != fe; ++fi ) {
      Function* pFuncToFix = *fi;
      runOnFunction(*pFuncToFix);
    }

    //Fix get_local_id() and get_global_id() function calls
    fixGetWIIdFunctions(M);

    return true;
  }

  bool Barrier::runOnFunction(Function &F) {

    assert(!m_pDataPerBarrier->hasFiberInstruction(&F) && "handle case when having fiber instructions!");

    //Get key values for this functions.
    getBarrierKeyValues(&F);

    m_pSyncInstructions = &m_pDataPerBarrier->getSyncInstructions(&F);

    m_pSpecialValues = &m_pDataPerValue->getValuesToHandle(&F);
    m_pAllocaValues = &m_pDataPerValue->getAllocaValuesToHandle(&F);
    m_pCrossBarrierValues = &m_pDataPerValue->getUniformValuesToHandle(&F);

    Instruction* pInsertBefore = F.getEntryBlock().begin();
    if (m_isNativeDBG) {
      // Move alloca instructions for locals/parameters for debugging purposes
      for (TValueVector::iterator vi = m_pAllocaValues->begin(), ve = m_pAllocaValues->end();
           vi != ve; ++vi ) {
        AllocaInst *pAllocaInst = cast<AllocaInst>(*vi);
        pAllocaInst->moveBefore(pInsertBefore);
      }
    }

    //Clear container for new iteration on new function
    m_toRemoveInstructions.clear();
    m_preSyncLoopHeader.clear();

    //Fix special values
    fixSpecialValues();

    //Do not fix alloca values in order for DWARF based debugging to work.
    if (!m_isNativeDBG) {
      //Fix alloca values
      fixAllocaValues();
    }

    //Fix cross barrier uniform values
    fixCrossBarrierValues(F.begin()->begin());

    //Replace sync instructions with internal loop over WI ID
    replaceSyncInstructions();

    //Remove all instructions in m_toRemoveInstructions
    eraseAllToRemoveInstructions();
    return true;
  }

  void Barrier::useStackAsWorkspace(Instruction* insertBefore, Instruction* insertBeforeEnd) {
    //TODO: do we need to set DebugLoc for these instruction?
    //      These are debug instruction, so we assume there
    //      are not binded to any source instruction.
    IRBuilder<> builder(*m_pContext);

    for (TValueVector::iterator vi = m_pAllocaValues->begin(),
      ve = m_pAllocaValues->end(); vi != ve; ++vi ) {
        AllocaInst *pAllocaInst = dyn_cast<AllocaInst>(*vi);
        assert( pAllocaInst && "container of alloca values has non AllocaInst value!" );
        assert( !m_pDataPerValue->isOneBitElementType(pAllocaInst) && "AllocaInst with base type i1!" );
        //Get offset of alloca value in special buffer
        unsigned int offset = m_pDataPerValue->getOffset(pAllocaInst);

        Value *pAddrInSpecialBufferCopyOut;
        Value *pAddrInSpecialBufferCopyIn;
        Type *pAllocaType = pAllocaInst->getAllocatedType();
        if (pAllocaType->isStructTy() || pAllocaType->isArrayTy()) {
          Constant *pSizeToCopy = ConstantExpr::getSizeOf(pAllocaType);
          if (insertBefore) {
            pAddrInSpecialBufferCopyOut = getAddressInSpecialBuffer(
                offset, pAllocaInst->getType(), insertBefore, NULL);

            // create copy to work item buffer (from stack)
            builder.SetInsertPoint(insertBefore);
            builder.CreateMemCpy(pAddrInSpecialBufferCopyOut,
                            pAllocaInst, pSizeToCopy, pAllocaInst->getAlignment(), false);
          }

          if (insertBeforeEnd) {
            pAddrInSpecialBufferCopyIn = getAddressInSpecialBuffer(
                offset, pAllocaInst->getType(), insertBeforeEnd, NULL);

            // create copy to stack (from work item buffer)
            builder.SetInsertPoint(insertBeforeEnd);
            builder.CreateMemCpy(pAllocaInst, pAddrInSpecialBufferCopyIn,
                            pSizeToCopy, pAllocaInst->getAlignment(), false);
          }
        } else {
          if (insertBefore) {
            pAddrInSpecialBufferCopyOut = getAddressInSpecialBuffer(
                offset, pAllocaInst->getType(), insertBefore, NULL);
            // create copy to work item buffer (from stack)
            LoadInst *pLDInstCopyOut = new LoadInst(pAllocaInst, "CopyOut", insertBefore);
            new StoreInst(pLDInstCopyOut, pAddrInSpecialBufferCopyOut, insertBefore);
          }

          if (insertBeforeEnd) {
            pAddrInSpecialBufferCopyIn = getAddressInSpecialBuffer(
                offset, pAllocaInst->getType(), insertBeforeEnd, NULL);

            // create copy to stack (from work item buffer)
            LoadInst *pLDInstCopyIn = new LoadInst(pAddrInSpecialBufferCopyIn, "CopyIn", insertBeforeEnd);
            new StoreInst(pLDInstCopyIn, pAllocaInst, insertBeforeEnd);
          }
        }
    }
  }

  void Barrier::fixAllocaValues() {
    TInstructionSet userInsts;
    TValueVector::iterator vi = m_pAllocaValues->begin();
    TValueVector::iterator ve = m_pAllocaValues->end();
    for ( ; vi != ve; ++vi ) {
      AllocaInst *pAllocaInst = dyn_cast<AllocaInst>(*vi);
      assert( pAllocaInst && "container of alloca values has non AllocaInst value!" );
      assert( !m_pDataPerValue->isOneBitElementType(pAllocaInst) && "AllocaInst with base type i1!" );
      //Get offset of alloca value in special buffer
      unsigned int offset = m_pDataPerValue->getOffset(pAllocaInst);
      userInsts.clear();
      //Save all user instructions in a container before start handling them!
      for ( Instruction::use_iterator ui = pAllocaInst->use_begin(),
        ue = pAllocaInst->use_end(); ui != ue; ++ui ) {
          Instruction *pUserInst = dyn_cast<Instruction>(*ui);
          assert( pUserInst && "uses of alloca instruction is not an instruction!" );
          userInsts.insert(pUserInst);
      }
      //Run over all saved uses instructions and handle them
      for ( TInstructionSet::iterator ui = userInsts.begin(),
        ue = userInsts.end(); ui != ue; ++ui ) {
          Instruction *pUserInst = *ui;
          Instruction *pInsertBefore = getInstructionToInsertBefore(pAllocaInst, pUserInst, false);
          assert( pInsertBefore && "pInsertBefore is NULL, update getInstructionToInsertBefore()!" );
          const DebugLoc& DB = pUserInst->getDebugLoc();
          //Calculate the pointer of the current alloca in the special buffer
          Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(
            offset, pAllocaInst->getType(), pInsertBefore, &DB);
          //Replace the alloca with the new address in special buffer
          pUserInst->replaceUsesOfWith(pAllocaInst, pAddrInSpecialBuffer);
      }
      m_toRemoveInstructions.push_back(pAllocaInst);
    }
  }

  void Barrier::fixSpecialValues() {
    TValueVector::iterator vi = m_pSpecialValues->begin();
    TValueVector::iterator ve = m_pSpecialValues->end();
    for ( ; vi != ve; ++vi ) {
      Instruction *pInst = dyn_cast<Instruction>(*vi);
      assert( pInst && "container of special values has non Instruction value!" );

      const DebugLoc& DB = pInst->getDebugLoc();
      //This will hold the real type of this value in the special buffer
      Type *pTypeInSP = pInst->getType();
      bool oneBitBaseType = m_pDataPerValue->isOneBitElementType(pInst);
      if ( oneBitBaseType ) {
        // base type is i1 need to ZEXT/TRUNC to/from i32
        VectorType *pVecType = dyn_cast<VectorType>(pInst->getType());
        if( pVecType ) {
          pTypeInSP = VectorType::get(IntegerType::get(*m_pContext, 32), pVecType->getNumElements());
        } else {
          pTypeInSP = IntegerType::get(*m_pContext, 32);
        }
      }

      //Get offset of special value in special buffer
      unsigned int offset = m_pDataPerValue->getOffset(pInst);
      //Find next instruction so we can create new instruction before it
      Instruction *pNextInst = &*(++BasicBlock::iterator(pInst));
      if ( isa<PHINode>(pNextInst) ) {
        //pNextInst is a PHINode, find first non PHINode to add instructions before it
        pNextInst = pNextInst->getParent()->getFirstNonPHI();
      }
      //Get PointerType of value type
      PointerType *pType = pTypeInSP->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
      //Handle Special buffer only if it is not a call instruction.
      //Special buffer value of call instruction will be handled in the callee.
      CallInst* pCallInst = dyn_cast<CallInst>(pInst);
      if( !( pCallInst && m_pDataPerValue->hasOffset(pCallInst->getCalledFunction()) ) ) {
        //Calculate the pointer of the current special in the special buffer
        Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(offset, pType, pNextInst, &DB);
        Instruction *pInstToStore = !oneBitBaseType ? pInst :
          CastInst::CreateZExtOrBitCast(pInst, pTypeInSP, "ZEXT-i1Toi32", pNextInst);
        //Need to set DebugLoc for the case is oneBitBaseType. It won't hart to set
        //Same DebugLoc for the other case, as DB = pInst->getDebugLoc();
        pInstToStore->setDebugLoc(DB);
        //Add Store instruction after the value instruction
        StoreInst* pStoreInst = new StoreInst(pInstToStore, pAddrInSpecialBuffer, pNextInst);
        pStoreInst->setDebugLoc(DB);
      }

      TInstructionSet userInsts;
      //Save all uses of pInst and add them to a container before start handling them!
      for ( Instruction::use_iterator ui = pInst->use_begin(),
        ue = pInst->use_end(); ui != ue; ++ui ) {
          Instruction *pUserInst = dyn_cast<Instruction>(*ui);
          assert( pUserInst && "uses of special instruction is not an instruction!" );
          if ( pInst->getParent() == pUserInst->getParent() ) {
            //This use of pInst is at the same basic block (no barrier cross so far)
            //assert( !isa<PHINode>(pUserInst) && "user instruction is a PHINode and appears befre pInst in BB" );
            if ( !isa<PHINode>(pUserInst) ) {
              continue;
            }
          }
          if ( isa<ReturnInst>(pUserInst) ) {
            // We don't want to return the value from the Special buffer we will load it later by the caller
            continue;
          }
          userInsts.insert(pUserInst);
      }
      //Run over all saved user instructions and handle by adding
      //load instruction before each value use
      for ( TInstructionSet::iterator ui = userInsts.begin(),
        ue = userInsts.end(); ui != ue; ++ui ) {
          Instruction *pUserInst = *ui;
          Instruction *pInsertBefore = getInstructionToInsertBefore(pInst, pUserInst, true);
          if ( !pInsertBefore ) {
            //as no barrier in the middle, no need to load & replace the origin value
            continue;
          }
          const DebugLoc& DB = pUserInst->getDebugLoc();
          //Calculate the pointer of the current special in the special buffer
          Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(offset, pType, pInsertBefore, &DB);
          Instruction *pLoadedValue = new LoadInst(pAddrInSpecialBuffer, "loadedValue", pInsertBefore);
          Instruction *pRealValue = !oneBitBaseType ? pLoadedValue :
            CastInst::CreateTruncOrBitCast(pLoadedValue, pInst->getType(), "Trunc-i1Toi32", pInsertBefore);
          pLoadedValue->setDebugLoc(DB);
          pRealValue->setDebugLoc(DB);
          //Replace the use of old value with the new loaded value from special buffer
          pUserInst->replaceUsesOfWith(pInst, pRealValue);
      }
    }
  }

  void Barrier::fixCrossBarrierValues(Instruction *pInsertBefore) {
    TValueVector::iterator vi = m_pCrossBarrierValues->begin();
    TValueVector::iterator ve = m_pCrossBarrierValues->end();
    for ( ; vi != ve; ++vi ) {
      Instruction *pInst = dyn_cast<Instruction>(*vi);
      assert( pInst && "container of special values has non Instruction value!" );
      //Find next instruction so we can create new instruction before it
      Instruction *pNextInst = &*(++BasicBlock::iterator(pInst));
      if ( isa<PHINode>(pNextInst) ) {
        //pNextInst is a PHINode, find first non PHINode to add instructions before it
        pNextInst = pNextInst->getParent()->getFirstNonPHI();
      }
      //Create alloca of value type at begining of function
      AllocaInst *pAllocaInst = new AllocaInst(pInst->getType(), pInst->getName(), pInsertBefore);
      //Add Store instruction after the value instruction
      StoreInst* pStoreInst = new StoreInst(pInst, pAllocaInst, pNextInst);
      pStoreInst->setDebugLoc(pInst->getDebugLoc());

      TInstructionSet userInsts;
      //Save all uses of pInst and add them to a container before start handling them!
      for ( Instruction::use_iterator ui = pInst->use_begin(),
        ue = pInst->use_end(); ui != ue; ++ui ) {
          Instruction *pUserInst = dyn_cast<Instruction>(*ui);
          assert( pUserInst && "uses of special instruction is not an instruction!" );
          if ( pInst->getParent() == pUserInst->getParent() && !isa<PHINode>(pUserInst) ) {
            //This use of pInst is at the same basic block (no barrier cross so far)
            continue;
          }
          userInsts.insert(pUserInst);
      }
      //Run over all saved user instructions and handle by adding
      //load instruction before each value use
      for ( TInstructionSet::iterator ui = userInsts.begin(),
        ue = userInsts.end(); ui != ue; ++ui ) {
          Instruction *pUserInst = *ui;
          Instruction *pInsertBefore = getInstructionToInsertBefore(pInst, pUserInst, true);
          if ( !pInsertBefore ) {
            //as no barrier in the middle, no need to load & replace the origin value
            continue;
          }
          //Calculate the pointer of the current special in the special buffer
          Instruction *pLoadedValue = new LoadInst(pAllocaInst, "loadedValue", pInsertBefore);
          pLoadedValue->setDebugLoc(pUserInst->getDebugLoc());
          //Replace the use of old value with the new loaded value from special buffer
          pUserInst->replaceUsesOfWith(pInst, pLoadedValue);
      }
    }
  }

  void Barrier::replaceSyncInstructions() {
    //Run over all sync instructions and split its basic-block
    //in order to create an empty basic-block previous to the sync basic block
    for ( TInstructionSet::iterator ii = m_pSyncInstructions->begin(),
      ie = m_pSyncInstructions->end(); ii != ie; ++ii ) {
        Instruction *pInst = dyn_cast<Instruction>(*ii);
        assert( pInst && "sync instruction container contains non instruction!" );
        BasicBlock* pLoopHeaderBB = pInst->getParent();
        BasicBlock* pLoopEntryBB =
          pInst->getParent()->splitBasicBlock(BasicBlock::iterator(pInst), "SyncBB");
        m_preSyncLoopHeader[pLoopEntryBB] = pLoopHeaderBB;
        m_toRemoveInstructions.push_back(pInst);
    }
    for ( TInstructionSet::iterator ii = m_pSyncInstructions->begin(),
      ie = m_pSyncInstructions->end(); ii != ie; ++ii ) {
        Instruction *pInst = *ii;
        unsigned int id = m_pDataPerBarrier->getUniqueID(pInst);
        SYNC_TYPE type = m_pDataPerBarrier->getSyncType(pInst);
        BasicBlock *pSyncBB = pInst->getParent();
        BasicBlock *pPreSyncBB = m_preSyncLoopHeader[pSyncBB];
        assert( pPreSyncBB && "pSyncBB assumed to have sync loop header basic block!" );
        if ( SYNC_TYPE_DUMMY_BARRIER == type ) {
          //This is a dummy barrier replace with the following
          // currWI = 0
          // currSB = 0
          // currBarrier = id
          new StoreInst(ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, 0)),
            m_currBarrierKeyValues->m_pCurrWIValue, pPreSyncBB->begin());

          new StoreInst(ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, 0)),
            m_currBarrierKeyValues->m_pCurrSBValue, pPreSyncBB->begin());

          new StoreInst(ConstantInt::get(Type::getInt32Ty(*m_pContext), APInt(32, id)),
            m_currBarrierKeyValues->m_pCurrBarrierValue, pPreSyncBB->begin());
          continue;
        }
        //This is a barrier/fiber instruction replace with the following code
        // if (currWI < WIIterationCount) {
        //   currWI++;
        //   switch (currBarrier) {
        //     case i: goto barrier_i;
        //   }
        // } else {
        //   currWI = 0;
        //   currBarrier = id
        // }

        //Create then and else basic-blocks
        BasicBlock *pThenBB = BasicBlock::Create(
          *m_pContext, "thenBB", pPreSyncBB->getParent(), pSyncBB);
        BasicBlock *pElseBB = BasicBlock::Create(
          *m_pContext, "elseBB", pPreSyncBB->getParent(), pSyncBB);

        //A. change the preSync basic block as follow
        //A(1). remove the unconditional jump instruction
        pPreSyncBB->getTerminator()->eraseFromParent();

        //A(2). add the entry tail code
        // if(currWI < WIIterationCount) {pThenBB} else {pElseBB}
        LoadInst *pLoadedCurrWI = new LoadInst(m_currBarrierKeyValues->m_pCurrWIValue, "loadedCurrWI", pPreSyncBB);
        ICmpInst *pCheckWIIter = new ICmpInst(*pPreSyncBB, ICmpInst::ICMP_ULT,
          pLoadedCurrWI, m_currBarrierKeyValues->m_pWIIterationCountValue, "check.WI.iter");
        BranchInst *pBrInst = BranchInst::Create(pThenBB, pElseBB, pCheckWIIter, pPreSyncBB);
        // Set the barrier() debug metadata to these newly added instructions
        const DebugLoc& DB = pInst->getDebugLoc();
        pLoadedCurrWI->setDebugLoc(DB);
        pCheckWIIter->setDebugLoc(DB);
        pBrInst->setDebugLoc(DB);

        //B. Create currWI++ and switch instruction in pThenBB
        DataPerBarrier::SBarrierRelated *pRelated = &m_pDataPerBarrier->getBarrierPredecessors(pInst);
        assert( !pRelated->m_hasFiberRelated && "We reach here only if function has no fiber!" );
        TInstructionVector *pSyncPreds = &pRelated->m_relatedBarriers;
        unsigned int numCases = pSyncPreds->size();
        assert( 0 != numCases && "Barrier cannot be without predecessors!" );

        //Create "CurrWI++" code
        Instruction *pUpdatedCurrWI = BinaryOperator::CreateNUWAdd(pLoadedCurrWI,
          ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, 1)), "CurrWI++", pThenBB);
        StoreInst* pStoredCurrWI = new StoreInst(pUpdatedCurrWI, m_currBarrierKeyValues->m_pCurrWIValue, pThenBB);
        // Set the barrier() debug metadata to these newly added instructions
        pUpdatedCurrWI->setDebugLoc(DB);
        pStoredCurrWI->setDebugLoc(DB);

        //Create "CurrSBBase+=Stride" code
        Instruction *pLoadedCurrSB = new LoadInst(m_currBarrierKeyValues->m_pCurrSBValue, "loadedCurrSB", pThenBB);
        Instruction *pUpdatedCurrSB = BinaryOperator::CreateNUWAdd(pLoadedCurrSB,
          m_currBarrierKeyValues->m_pStructureSizeValue, "loadedCurrSB+Stride", pThenBB);
        StoreInst* pStoredCurrSB = new StoreInst(pUpdatedCurrSB, m_currBarrierKeyValues->m_pCurrSBValue, pThenBB);
        // Set the barrier() debug metadata to these newly added instructions
        pLoadedCurrSB->setDebugLoc(DB);
        pUpdatedCurrSB->setDebugLoc(DB);
        pStoredCurrSB->setDebugLoc(DB);

        Instruction *pFirstSyncInst = *pSyncPreds->begin();
        if ( numCases == 1 ) {
          //Only one case, no need for switch, create unconditional jump
          Instruction* pBrInst = BranchInst::Create(pFirstSyncInst->getParent(), pThenBB);
          // Set the barrier() debug metadata to these newly added instructions
          pBrInst->setDebugLoc(DB);
        } else {
          //More than one case, create a switch
          //The first sync instruction is chosen to be the switch Default case
          Instruction *pLoadedCurrBarrier = new LoadInst(m_currBarrierKeyValues->m_pCurrBarrierValue, "loadedCurrBarrier", pThenBB);
          SwitchInst *pSwitch = SwitchInst::Create(pLoadedCurrBarrier, pFirstSyncInst->getParent(), numCases-1, pThenBB);
          for ( TInstructionVector::iterator ii = pSyncPreds->begin(),
            ie = pSyncPreds->end(); (++ii) != ie; ) {
              Instruction *pSyncInst = *ii;
              unsigned int predId = m_pDataPerBarrier->getUniqueID(pSyncInst);
              pSwitch->addCase(ConstantInt::get(*m_pContext, APInt(32, predId)), pSyncInst->getParent());
          }
          // Set the barrier() debug metadata to these newly added instructions
          pLoadedCurrBarrier->setDebugLoc(DB);
          pSwitch->setDebugLoc(DB);
        }

        //C. Create initialization to currWI, currSB and currBarrier in pElseBB
        // currWI = 0
        // currSB = 0
        // currBarrier = id
        //And connect the pElseBB to the pSyncBB with unconditional jump
        StoreInst* pElseStoredCurrWI = new StoreInst(ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, 0)),
          m_currBarrierKeyValues->m_pCurrWIValue, pElseBB);
        StoreInst* pElseStoredCurrSB = new StoreInst(ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, 0)),
          m_currBarrierKeyValues->m_pCurrSBValue, pElseBB);
        StoreInst* pElseStoredCurrBarrier = new StoreInst(ConstantInt::get(Type::getInt32Ty(*m_pContext), APInt(32, id)),
          m_currBarrierKeyValues->m_pCurrBarrierValue, pElseBB);
        assert(isa<CallInst>(pInst) &&
               "Barrier instruction is not a CallInst!");
        CallInst *pBarrier = cast<CallInst>(pInst);
        Value *pArg1 = pBarrier->getOperand(0);
        assert( pArg1 && "Barrier instruction has no first argument!" );
        assert(isa<ConstantInt>(pArg1) &&
               "Barrier first argument (memory fence) must be const!");
        ConstantInt *pMemFence = cast<ConstantInt>(pArg1);
        if ( pMemFence->getZExtValue() & CLK_GLOBAL_MEM_FENCE ) {
          //barrier(global): add mem_fence instruction!
          m_util.createMemFence(pElseBB);
        }
        Instruction* pElseBrInst = BranchInst::Create(pSyncBB, pElseBB);
        // Set the barrier() debug metadata to these newly added instructions
        pElseStoredCurrWI->setDebugLoc(DB);
        pElseStoredCurrSB->setDebugLoc(DB);
        pElseStoredCurrBarrier->setDebugLoc(DB);
        pElseBrInst->setDebugLoc(DB);

        // Only if we are debugging, copy data into the stack from work item buffer
        // for execution and copy data out when finished. This allows for proper
        // DWARF based debugging.
        if (m_isNativeDBG) {
          // Use the then and else blocks to copy work item data into and out of the stack for each work item
          Instruction &pThenFront = pThenBB->front();
          Instruction &pElseFront = pElseBB->front();
          useStackAsWorkspace(&pThenFront, &(pThenBB->back()));
          useStackAsWorkspace(&pElseFront, &(pElseBB->back()));

          // I add the function DebugCopy as a marker so it can be handled later in LocalBuffers pass.
          // LocalBuffers pass is responsible for implementing __local variables correctly in OpenCL
          // (ie. as work-group globals and not thread globals). I insert them in these marked blocks
          // so that I know when I need to copy from the local buffer into the thread local (global).
          // This is also how I know where the beginning of each work item iteration is (in the presence
          // of barriers) which is where the copying occurs.

          // Maybe there is a better way, I'm not sure. The problem I found is LocalBuffers finds all
          // uses of a __local variable and updates the references to a local buffer memory location
          // rather then the thread specific global for which the __local variable symbol is defined.
          // So any changes to __local variables would have to be delayed until this pass or LocalBuffers
          // would have to behave very differently.

          // There is also a copy that occurs from the local buffer into the global variable after each
          // use of the __local variable so that the thread specific global stays updated. This is
          // independent of the function markers. This is done in LocalBuffers pass.

          // This only allows for reading of __local variables and not setting.

          Type *pResult = Type::getVoidTy(*m_pContext);
          Module *M = pThenBB->getParent()->getParent();
          Constant *pFunc = M->getOrInsertFunction("DebugCopy.", pResult, NULL);
          CallInst::Create(pFunc, "", &pThenFront);
          CallInst::Create(pFunc, "", &pElseFront);
        }
    }
  }

  void Barrier::createBarrierKeyValues(Function* pFunc, bool hasNoInternalCalls) {
    SBarrierKeyValues* pBarrierKeyValues = &m_pBarrierKeyValuesPerFunction[pFunc];

    Instruction* pInsertBefore = pFunc->getEntryBlock().begin();
    //Add currBarrier alloca
    pBarrierKeyValues->m_pCurrBarrierValue = new AllocaInst(Type::getInt32Ty(*m_pContext),
      "currBarrier", pInsertBefore);

    //Will hold the index in special buffer and will be increased by stride size
    pBarrierKeyValues->m_pCurrSBValue = new AllocaInst(m_sizeTType, "CurrSBIndex", pInsertBefore);

    //get_curr_wi()
    if (hasNoInternalCalls ) {
      pBarrierKeyValues->m_pCurrWIValue = new AllocaInst(m_sizeTType, "CurrWI", pInsertBefore);
    } else {
      pBarrierKeyValues->m_pCurrWIValue = m_util.createGetCurrWI(pInsertBefore);
    }

    //get_special_buffer()
    pBarrierKeyValues->m_pSpecialBufferValue = m_util.createGetSpecialBuffer(pInsertBefore);

    //get_iter_count()
    pBarrierKeyValues->m_pWIIterationCountValue = m_util.createGetIterCount(pInsertBefore);

    unsigned int structureSize = m_pDataPerValue->getStrideSize(pFunc);
    pBarrierKeyValues->m_pStructureSizeValue =
      ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, structureSize));
  }

  void Barrier::getBarrierKeyValues(Function* pFunc) {
    assert(m_pBarrierKeyValuesPerFunction.count(pFunc) &&
      "initiation of argument values is broken");
    m_currBarrierKeyValues = &m_pBarrierKeyValuesPerFunction[pFunc];
  }

  Instruction* Barrier::getInstructionToInsertBefore(Instruction *pInst, Instruction *pUserInst, bool expectNULL) {
    if ( !isa<PHINode>(pUserInst) ) {
      //pUserInst is not a PHINode, we can insert instruction before it.
      return pUserInst;
    }
    //pUserInst is a PHINode, find previous basic block
    BasicBlock *pPrevBB = BarrierUtils::findBasicBlockOfUsageInst(pInst, pUserInst);

    if ( expectNULL && pPrevBB == pInst->getParent() ) {
      //In such case no need to load & replace the origin value
      //as no barrier in the middle, return NULL to indecate that.
      return NULL;
    }
    return pPrevBB->getTerminator();
  }

  Value* Barrier::getAddressInSpecialBuffer(
    unsigned int offset, PointerType *pType, Instruction *pInsertBefore, const DebugLoc* DB){
      Value *pOffsetVal = ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, offset));
      //If hit this assert then need to handle PHINode!
      assert( !isa<PHINode>(pInsertBefore) && "cannot add instructions before a PHI node!" );
      //Calculate the pointer of the given offset for currWI in the special buffer
      Instruction *pLoadedCurrSB = new LoadInst(m_currBarrierKeyValues->m_pCurrSBValue, "loadedCurrSB", pInsertBefore);
      Instruction *pIndexInst = BinaryOperator::CreateNUWAdd(pLoadedCurrSB, pOffsetVal, "&(pSB[currWI].offset)", pInsertBefore);
      Value *Idxs[1];
      Idxs[0] = pIndexInst;
      Instruction *pAddrInSBinBytes = GetElementPtrInst::CreateInBounds(
        m_currBarrierKeyValues->m_pSpecialBufferValue, ArrayRef<Value*>(Idxs), "&pSB[currWI].offset", pInsertBefore);
      //Bitcast pointer according to alloca type!
      Instruction *pAddrInSpecialBuffer = BitCastInst::CreatePointerCast(
        pAddrInSBinBytes, pType, "CastToValueType", pInsertBefore);

      if( DB ) {
        //Set DebugLoc for all new added instructions.
        pLoadedCurrSB->setDebugLoc(*DB);
        pIndexInst->setDebugLoc(*DB);
        pAddrInSBinBytes->setDebugLoc(*DB);
        pAddrInSpecialBuffer->setDebugLoc(*DB);
      }
      return pAddrInSpecialBuffer;
  }

  bool Barrier::fixGetWIIdFunctions(Module &M) {
    //clear container for new iteration on new function
    m_toRemoveInstructions.clear();

    //Find all get_local_id instructions
    TInstructionVector& getLIDInstructions = m_util.getAllGetLocalId();
    for (TInstructionVector::iterator ii = getLIDInstructions.begin(),
      ie = getLIDInstructions.end(); ii != ie; ++ii ) {
        CallInst *pOldCall = dyn_cast<CallInst>(*ii);
        assert( pOldCall && "Something other than CallInst is using get_local_id function!" );
        //If no pCurrWI is not defined on this function yet, then define it
        Function *pFunc = pOldCall->getParent()->getParent();
        Value* pCurrWIValue = NULL;
        if( m_pBarrierKeyValuesPerFunction.count(pFunc) ) {
          pCurrWIValue = m_pBarrierKeyValuesPerFunction[pFunc].m_pCurrWIValue;
        }
        else {
          pCurrWIValue = m_util.createGetCurrWI(pFunc->begin()->begin());
        }
        //Replace get_local_id(arg) with get_new_local_id(arg, *pCurrWI)
        Value *args1 = pOldCall->getOperand(0);
        Value *args2 = new LoadInst(pCurrWIValue, "currWI", pOldCall);
        Instruction *pNewCall = m_util.createNewGetLocalId(args1, args2, pOldCall);
        pOldCall->replaceAllUsesWith(pNewCall);
        m_toRemoveInstructions.push_back(pOldCall);
    }

    //Find all get_local_id instructions
    TInstructionVector& getGIDInstructions = m_util.getAllGetGlobalId();
    for (TInstructionVector::iterator ii = getGIDInstructions.begin(),
      ie = getGIDInstructions.end(); ii != ie; ++ii ) {
        CallInst *pOldCall = dyn_cast<CallInst>(*ii);
        assert( pOldCall && "Something other than CallInst is using get_global_id function!" );
        //If no pCurrWI is not defined on this function yet, then define it
        Function *pFunc = pOldCall->getParent()->getParent();
        Value* pCurrWIValue = NULL;
        if( m_pBarrierKeyValuesPerFunction.count(pFunc) ) {
          pCurrWIValue = m_pBarrierKeyValuesPerFunction[pFunc].m_pCurrWIValue;
        }
        else {
          pCurrWIValue = m_util.createGetCurrWI(pFunc->begin()->begin());
        }
        //Replace get_global_id(arg) with get_new_global_id(arg, *pCurrWI)
        Value *args1 = pOldCall->getOperand(0);
        Value *args2 = new LoadInst(pCurrWIValue, "currWI", pOldCall);
        Instruction *pNewCall = m_util.createNewGetGlobalId(args1, args2, pOldCall);
        pOldCall->replaceAllUsesWith(pNewCall);
        m_toRemoveInstructions.push_back(pOldCall);
    }

    //Remove all instructions in m_toRemoveInstructions
    eraseAllToRemoveInstructions();

    return true;
  }

  void Barrier::fixNonInlineFunction(Function *pFuncToFix) {
    //TODO: do we need to set DebugLoc for these instructions?
    //Get key values for this functions.
    getBarrierKeyValues(pFuncToFix);

    unsigned int numOfArgs = pFuncToFix->getFunctionType()->getNumParams();
    //Use offsets instead of original parameters
    Function::arg_iterator argIter = pFuncToFix->arg_begin();
    for ( unsigned int i = 0; i < numOfArgs; ++i, ++argIter ) {
      Value *pArgVal = &*argIter;
      if ( m_pDataPerValue->hasOffset(pArgVal) ) {
        unsigned int offset = m_pDataPerValue->getOffset(pArgVal);
        fixArgumentUsage(pArgVal, offset);
      }
    }
    if ( m_pDataPerValue->hasOffset(pFuncToFix) ) {
      unsigned int offset = m_pDataPerValue->getOffset(pFuncToFix);

      std::vector<BasicBlock*> pVecBB;
      for ( Function::iterator bi = pFuncToFix->begin(), be = pFuncToFix->end(); bi != be; ++bi ) {
        BasicBlock *pBB = &*bi;
        pVecBB.push_back(pBB);
      }
      //Run over all basic blocks of the new function and handle return terminators
      for ( std::vector<BasicBlock*>::iterator bi = pVecBB.begin(), be = pVecBB.end(); bi != be; ++bi ) {
        BasicBlock *pBB = *bi;
        ReturnInst *pRetInst = dyn_cast<ReturnInst>(pBB->getTerminator());
        if ( !pRetInst ) {
          //It is not return instruction terminator, check next basic block
          continue;
        }
        Value *pRetVal = pRetInst->getOperand(0);
        if ( Instruction *pInst = dyn_cast<Instruction>(pRetVal) ) {
          //Find next instruction so we can create new instruction before it
          Instruction *pNextInst = &*(++BasicBlock::iterator(pInst));
          if ( isa<PHINode>(pNextInst) ) {
            //pNextInst is a PHINode, find first non PHINode to add instructions before it
            pNextInst = pNextInst->getParent()->getFirstNonPHI();
          }
          fixReturnValue(pRetVal, offset, pNextInst);
        } else {
          //In this case the return value is not an instruction and
          //it cannot be assumed that it is inside the barrier loop.
          //Thus, need to create a new barrier loop that store this value
          //in the special buffer, that is why we needed to find the values:
          // m_pCurrSBValue, m_pCurrWIValue, m_pWIIterationCountValue
          //Before:
          //  BB:
          //      ret pRetVal
          //After:
          //  BB:
          //      br loopBB
          //  loopBB:
          //      pSB[pCurrSBValue+offset] = pRetVal
          //      cond currWI < IterCount
          //      currWI++
          //      pCurrSBValue += Stride
          //      br cond, loopBB, RetBB
          //  RetBB:
          //      ret pRetVal
          BasicBlock *pNewRetBB =
            pBB->splitBasicBlock(BasicBlock::iterator(pRetInst), "RetBB");
          BasicBlock *pLoopBB = BasicBlock::Create(
            *m_pContext, "loopBB", pBB->getParent(), pNewRetBB);

          pBB->getTerminator()->eraseFromParent();
          BranchInst::Create(pLoopBB, pBB);

          Value *pLoadedCurrWI = new LoadInst(m_currBarrierKeyValues->m_pCurrWIValue, "loadedCurrWI", pLoopBB);
          ICmpInst *pCheckWIIter = new ICmpInst(*pLoopBB, ICmpInst::ICMP_ULT,
            pLoadedCurrWI, m_currBarrierKeyValues->m_pWIIterationCountValue, "check.WI.iter");

          Value *pUpdatedCurrWI = BinaryOperator::CreateNUWAdd(pLoadedCurrWI,
            ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, 1)), "CurrWI++", pLoopBB);
          new StoreInst(pUpdatedCurrWI, m_currBarrierKeyValues->m_pCurrWIValue, pLoopBB);

          //Create "CurrSBBase+=Stride" code
          Value *pLoadedCurrSB = new LoadInst(m_currBarrierKeyValues->m_pCurrSBValue, "loadedCurrSB", pLoopBB);
          Value *pUpdatedCurrSB = BinaryOperator::CreateNUWAdd(pLoadedCurrSB,
            m_currBarrierKeyValues->m_pStructureSizeValue, "loadedCurrSB+Stride", pLoopBB);
          new StoreInst(pUpdatedCurrSB, m_currBarrierKeyValues->m_pCurrSBValue, pLoopBB);

          BranchInst::Create(pLoopBB, pNewRetBB, pCheckWIIter, pLoopBB);

          Instruction *pNextInst = pLoopBB->getFirstNonPHI();
          fixReturnValue(pRetVal, offset, pNextInst);
        }
      }
    }
  }

  void Barrier::fixArgumentUsage(Value *pOriginalArg, unsigned int offsetArg) {
    //TODO: do we need to set DebugLoc for these instructions?
    assert( (!m_pDataPerValue->isOneBitElementType(pOriginalArg) ||
      !isa<VectorType>(pOriginalArg->getType())) && "pOriginalArg with base type i1!");
    TInstructionSet userInsts;
    for ( Value::use_iterator ui = pOriginalArg->use_begin(),
      ue = pOriginalArg->use_end(); ui != ue; ++ui) {
        Instruction *pUserInst = dyn_cast<Instruction>(*ui);
        userInsts.insert(pUserInst);
    }
    for ( TInstructionSet::iterator ui = userInsts.begin(),
      ue = userInsts.end(); ui != ue; ++ui) {
        Instruction *pUserInst = *ui;
        assert( pUserInst && "Something other than Instruction is using function argument!" );
        Instruction *pInsertBefore = pUserInst;
        if ( isa<PHINode>(pUserInst) ) {
          BasicBlock *pPrevBB = BarrierUtils::findBasicBlockOfUsageInst(pOriginalArg, pUserInst);
          pInsertBefore = pPrevBB->getTerminator();
        }
        //In this case we will always get a valid offset and need to load the argument
        //from the special buffer using the offset corresponding argument
        PointerType *pType = pOriginalArg->getType()->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
        Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(offsetArg, pType, pInsertBefore, NULL);
        Value *pLoadedValue = new LoadInst(pAddrInSpecialBuffer, "loadedValue", pInsertBefore);
        pUserInst->replaceUsesOfWith(pOriginalArg, pLoadedValue);
    }
  }

  void Barrier::fixReturnValue(Value *pRetVal, unsigned int offsetRet, Instruction* pInsertBefore) {
    //TODO: do we need to set DebugLoc for these instructions?
    assert( (!m_pDataPerValue->isOneBitElementType(pRetVal) ||
      !isa<VectorType>(pRetVal->getType())) && "pRetVal with base type i1!");
    //pRetVal might be a result of calling other function itself
    //in such case no need to handle it here as it will be saved
    //to the special buffer by the called function itself.
    //if ( !( isa<CallInst>(pRetVal) &&
    //  m_pDataPerInternalFunction->needToBeFixed(cast<CallInst>(pRetVal)) ) ) {
        //Calculate the pointer of the current special in the special buffer
        PointerType *pType = pRetVal->getType()->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
        Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(offsetRet, pType, pInsertBefore, NULL);
        //Add Store instruction after the value instruction
        new StoreInst(pRetVal, pAddrInSpecialBuffer, pInsertBefore);
    //}
  }

  void Barrier::fixCallInstruction(CallInst *pCallToFix) {
    Function* pCalledFunc = pCallToFix->getCalledFunction();
    assert(pCalledFunc && "Call instruction has no called function");
    Function* pFunc = pCallToFix->getParent()->getParent();

    //Get key values for this functions.
    getBarrierKeyValues(pFunc);

    const DebugLoc& DB = pCallToFix->getDebugLoc();
    Instruction* pInsertBefore = NULL;
    Function::arg_iterator argIter = pCalledFunc->arg_begin();
    for ( CallInst::const_op_iterator opi = pCallToFix->op_begin(),
      ope = pCallToFix->op_end(); opi != ope; ++opi, ++argIter) {
        if ( !m_pDataPerValue->hasOffset(&*argIter) ) continue;

        if ( !pInsertBefore ) {
          //Split sync instruction basic-block that contains the call instruction
          BasicBlock* pPreBB = pCallToFix->getParent();
          BasicBlock::iterator firstInst = pPreBB->begin();
          assert( m_pDataPerBarrier->getSyncInstructions(pFunc).count(&*firstInst) &&
            "assume first instruction to be sync instruction" );
          pPreBB->splitBasicBlock(firstInst, "SyncBB");
          pInsertBefore = pPreBB->getTerminator();
        }
        //Need to handle operand
        Value* pOpVal = *opi;
        unsigned int offset = m_pDataPerValue->getOffset(&*argIter);

        //Calculate the pointer of the current special in the special buffer
        PointerType *pType = pOpVal->getType()->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
        Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(offset, pType, pInsertBefore, &DB);
        //Add Store instruction before the synchronize instruction (in the pre basic block)
        StoreInst* pStoreInst = new StoreInst(pOpVal, pAddrInSpecialBuffer, pInsertBefore);
        pStoreInst->setDebugLoc(DB);
    }
    //Check if return value has usages
    if ( !pCallToFix->getNumUses() ) return;

    if ( !m_pDataPerValue->hasOffset(pCalledFunc) ) return;
    //Need to handle return value

    //Validate that next basic block is a synchronize basic block
    BasicBlock* pCallBB = pCallToFix->getParent();
    BranchInst* pBrInst = dyn_cast<BranchInst>(pCallBB->getTerminator());
    assert(pBrInst && pBrInst->getNumSuccessors() == 1 && "callInst BB has more than one successor");
    BasicBlock::iterator firstInst = pBrInst->getSuccessor(0)->begin();
    assert( m_pDataPerBarrier->getSyncInstructions(pFunc).count(&*firstInst) &&
            "assume first instruction to be sync instruction" );
    //Find next instruction so we can create new instruction before it
    Instruction *pNextInst = &*(++firstInst);

    unsigned int offset = m_pDataPerValue->getOffset(pCalledFunc);

    //Calculate the pointer of the current special in the special buffer
    PointerType *pType = pCallToFix->getType()->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
    Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(offset, pType, pNextInst, &DB);
    //Add Load instruction from special buffer at function offset
    LoadInst *pLoadedValue = new LoadInst(pAddrInSpecialBuffer, "loadedValue", pNextInst);
    pLoadedValue->setDebugLoc(DB);

    if ( m_pDataPerValue->hasOffset(pCallToFix) ) {
      //CallInst return value has an offset in the special buffer
      //Store the value to this offset.
      unsigned int offsetRet = m_pDataPerValue->getOffset(pCallToFix);

      //Calculate the pointer of the current special in the special buffer
      Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(offsetRet, pType, pNextInst, &DB);
      //Add Store instruction to special buffer at return value offset
      StoreInst* pStoreInst = new StoreInst(pLoadedValue, pAddrInSpecialBuffer, pNextInst);
      pStoreInst->setDebugLoc(DB);
    }
    else {
      pCallToFix->replaceAllUsesWith(pLoadedValue);
    }
  }

  void Barrier::eraseAllToRemoveInstructions() {
    //Remove all instructions in m_toRemoveInstructions
    for( TInstructionVector::iterator ii = m_toRemoveInstructions.begin(),
      ie = m_toRemoveInstructions.end(); ii != ie; ++ii ) {
        Instruction *pInst = dyn_cast<Instruction>(*ii);
        assert( pInst && "remove instruction container contains non instruction!" );
        pInst->eraseFromParent();
    }
  }

  void Barrier::updateStructureStride(Module & M) {
    Intel::MetaDataUtils mdUtils(&M);
    if ( !mdUtils.isKernelsInfoHasValue() ) {
      //Module contains no MetaData information, thus it contains no kernels
      return;
    }
    // Get the kernels using the barrier for work group loops.
    Intel::MetaDataUtils::KernelsInfoMap::const_iterator itr = mdUtils.begin_KernelsInfo();
    Intel::MetaDataUtils::KernelsInfoMap::const_iterator end = mdUtils.end_KernelsInfo();
    for (; itr != end; ++itr) {
      unsigned int strideSize = 0;
      Intel::KernelInfoMetaDataHandle kimd = itr->second;
      //Need to check if NoBarrierPath Value exists, it is not guaranteed that
      //KernelAnalysisPass is running in all scenarios.
      if (kimd->isNoBarrierPathHasValue() && kimd->getNoBarrierPath()) {
        //Kernel that should not be handled in Barrier path,
        //set barrier buffer stride to default.
        kimd->setBarrierBufferSize(strideSize);
        continue;
      }
      Function* pFunc = itr->first;
      assert( pFunc && "MetaData first operand is not of type Function!" );
      //Need to check if Vectorized Width Value exists, it is not guaranteed that
      //Vectorized is running in all scenarios.
      int vecWidth = kimd->isVectorizedWidthHasValue() ? kimd->getVectorizedWidth() : 1;
      strideSize = m_pDataPerValue->getStrideSize(pFunc);
      strideSize = (strideSize + vecWidth - 1) / vecWidth;
      kimd->setBarrierBufferSize(strideSize);
    }
    mdUtils.save(M.getContext());
  }


} // namespace intel


/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createBarrierPass(bool isNativeDebug) {
    return new intel::Barrier(isNativeDebug);
  }

  void getBarrierPassStrideSize(Pass *pPass, std::map<std::string, unsigned int>& bufferStrideMap) {
    ((intel::Barrier*)pPass)->getStrideMap(bufferStrideMap);
  }
}
