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
  OCL_INITIALIZE_PASS_DEPENDENCY(DataPerInternalFunction);
  OCL_INITIALIZE_PASS_END(Barrier, "B-Barrier", "Barrier Pass - Handle special values & replace barrier/fiber with internal loop over WIs", false, true)

  Barrier::Barrier(bool isNativeDebug) : ModulePass(ID), m_isNativeDBG(isNativeDebug) {
    initializeBarrierPass(*llvm::PassRegistry::getPassRegistry());
  }

  bool Barrier::runOnModule(Module &M) {
    //Get Analysis data
    m_pDataPerBarrier = &getAnalysis<DataPerBarrier>();
    m_pDataPerValue = &getAnalysis<DataPerValue>();
    m_pDataPerInternalFunction = &getAnalysis<DataPerInternalFunction>();

    //Initialize barrier utils class with current module
    m_util.init(&M);

    m_pContext = &M.getContext();
    //Initialize the side of size_t
    m_uiSizeT = M.getPointerSize()*32;
    m_sizeTType = IntegerType::get(*m_pContext, m_uiSizeT);

    //Update Map with structure stride size for each kernel
    updateStructureStride(M);

    //Run over module functions and for such with synchronize instruction:
    // 1. Handle Values from Group-A, Group-B.1 and Group-B.2
    // 2. Hanlde synchronize instructions
    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
      runOnFunction(*fi);
    }

    //Fix get_local_id() and get_global_id() function calls
    fixGetWIIdFunctions(M);

    //Fix non inlined internal functions that need special handling
    fixNonInlinedInternalFunctions(M);
    return true;
  }

  bool Barrier::runOnFunction(Function &F) {
    if (F.isDeclaration())
      return false;

    //Check if function has no synchronize instructions!
    if ( !m_pDataPerBarrier->hasSyncInstruction(&F) ) {
      //Function has no barrier: do nothing!
      return false;
    }
    m_pSyncInstructions = &m_pDataPerBarrier->getSyncInstructions(&F);

    m_pSpecialValues = &m_pDataPerValue->getValuesToHandle(&F);
    m_pAllocaValues = &m_pDataPerValue->getAllocaValuesToHandle(&F);
    m_pCrossBarrierValues = &m_pDataPerValue->getUniformValuesToHandle(&F);

    unsigned int structureSize = m_pDataPerValue->getStrideSize(&F);
    m_pStructureSizeValue = ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, structureSize));

    //Clear container for new iteration on new function
    m_toRemoveInstructions.clear();
    m_preSyncLoopHeader.clear();

    if ( m_pDataPerBarrier->hasFiberInstruction(&F) ) {
      //TODO: handle this case
      assert( false && "handle case when having fiber instructions!");
      return true;
    }

    //There is no fiber instructions
    //Create new BB at the begining of the function for declarations
    F.begin()->splitBasicBlock(F.begin()->begin(), "FirstBB");
    //Initialize the argument values
    //This is needed for optimize pCurrWI calculation
    bool hasNoInternalCalls = !m_util.doesCallModuleFunction(&F);
    initArgumentValues(F.begin()->begin(), hasNoInternalCalls);
    //Initialize the currWI pointer of this function
    m_pCurrWIperFunction[&F] = m_pCurrWIValue;
    //Initialize the CurrSB pointer of this function
    m_pCurrSBPerFunction[&F] = m_pCurrSBValue;
    //Initialize the WIIterationCount pointer of this function
    m_pIterationCountPerFunction[&F] = m_pWIIterationCountValue;

    //Fix special values
    fixSpecialValues();

    //Do not fix alloca values in order for DWARF based debugging to work.
    if (!m_isNativeDBG)
        //Fix alloca values
        fixAllocaValues();

    //Fix cross barrier uniform values
    fixCrossBarrierValues(F.begin()->begin());

    //Replace sync instructions with internal loop over WI ID
    replaceSyncInstructions();

    //Remove all instructions in m_toRemoveInstructions
    eraseAllToRemoveInstructions();
    return true;
  }

  void Barrier::useStackAsWorkspace(Instruction* insertBefore, Instruction* insertBeforeEnd) {
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
                offset, pAllocaInst->getType(), insertBefore);

            // create copy to work item buffer (from stack)
            builder.SetInsertPoint(insertBefore);
            builder.CreateMemCpy(pAddrInSpecialBufferCopyOut,
                            pAllocaInst, pSizeToCopy, pAllocaInst->getAlignment(), false);
          }

          if (insertBeforeEnd) {
            pAddrInSpecialBufferCopyIn = getAddressInSpecialBuffer(
                offset, pAllocaInst->getType(), insertBeforeEnd);

            // create copy to stack (from work item buffer)
            builder.SetInsertPoint(insertBeforeEnd);
            builder.CreateMemCpy(pAllocaInst, pAddrInSpecialBufferCopyIn,
                            pSizeToCopy, pAllocaInst->getAlignment(), false);
          }
        } else {
          if (insertBefore) {
            pAddrInSpecialBufferCopyOut = getAddressInSpecialBuffer(
                offset, pAllocaInst->getType(), insertBefore);
            // create copy to work item buffer (from stack)
            LoadInst *pLDInstCopyOut = new LoadInst(pAllocaInst, "CopyOut", insertBefore);
            new StoreInst(pLDInstCopyOut, pAddrInSpecialBufferCopyOut, insertBefore);
          }

          if (insertBeforeEnd) {
            pAddrInSpecialBufferCopyIn = getAddressInSpecialBuffer(
                offset, pAllocaInst->getType(), insertBeforeEnd);

            // create copy to stack (from work item buffer)
            LoadInst *pLDInstCopyIn = new LoadInst(pAddrInSpecialBufferCopyIn, "CopyIn", insertBeforeEnd);
            new StoreInst(pLDInstCopyIn, pAllocaInst, insertBeforeEnd);
          }
        }
    }
  }

  void Barrier::fixAllocaValues() {
    TInstructionVector userInsts;
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
          userInsts.push_back(pUserInst);
      }
      //Run over all saved uses instructions and handle them
      for ( TInstructionVector::iterator ui = userInsts.begin(),
        ue = userInsts.end(); ui != ue; ) {
          Instruction *pUserInst = *ui;
          Instruction *pInsertBefore = getInstructionToInsertBefore(pAllocaInst, pUserInst, false);
          assert( pInsertBefore && "pInsertBefore is NULL, update getInstructionToInsertBefore()!" );
          //Calculate the pointer of the current alloca in the special buffer
          Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(
            offset, pAllocaInst->getType(), pInsertBefore);
          //Replace the alloca with the new address in special buffer
          ++ui; // Need to increment iterator before replace it!
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
      PointerType *pType = PointerType::get(pTypeInSP, SPECIAL_BUFFER_ADDR_SPACE);
      //Handle Special buffer only if it is not a call instruction.
      //Special buffer value of call instruction will be handled in the callee.
      if( !( isa<CallInst>(pInst) &&
        m_pDataPerInternalFunction->needToBeFixed(cast<CallInst>(pInst)) ) ) {
          //Calculate the pointer of the current special in the special buffer
          Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(offset, pType, pNextInst);
          Instruction *pInstToStore = !oneBitBaseType ? pInst :
            CastInst::CreateZExtOrBitCast(pInst, pTypeInSP, "ZEXT-i1Toi32", pNextInst);;

          //Add Store instruction after the value instruction
          new StoreInst(pInstToStore, pAddrInSpecialBuffer, pNextInst);
      }

      TInstructionVector userInsts;
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
          userInsts.push_back(pUserInst);
      }
      //Run over all saved user instructions and handle by adding
      //load instruction before each value use
      for ( TInstructionVector::iterator ui = userInsts.begin(),
        ue = userInsts.end(); ui != ue; ) {
          Instruction *pUserInst = *ui;
          Instruction *pInsertBefore = getInstructionToInsertBefore(pInst, pUserInst, true);
          if ( !pInsertBefore ) {
            //as no barrier in the middle, no need to load & replace the origin value
            ++ui; // Need to increment iterator!
            continue;
          }
          //Calculate the pointer of the current special in the special buffer
          Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(offset, pType, pInsertBefore);
          Value *pLoadedValue = new LoadInst(pAddrInSpecialBuffer, "loadedValue", pInsertBefore);
          Value *pRealValue = !oneBitBaseType ? pLoadedValue :
            CastInst::CreateTruncOrBitCast(pLoadedValue, pInst->getType(), "Trunc-i1Toi32", pInsertBefore);
          //Replace the use of old value with the new loaded value from special buffer
          ++ui; // Need to increment iterator before replace it!
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
      new StoreInst(pInst, pAllocaInst, pNextInst);

      TInstructionVector userInsts;
      //Save all uses of pInst and add them to a container before start handling them!
      for ( Instruction::use_iterator ui = pInst->use_begin(),
        ue = pInst->use_end(); ui != ue; ++ui ) {
          Instruction *pUserInst = dyn_cast<Instruction>(*ui);
          assert( pUserInst && "uses of special instruction is not an instruction!" );
          if ( pInst->getParent() == pUserInst->getParent() && !isa<PHINode>(pUserInst) ) {
            //This use of pInst is at the same basic block (no barrier cross so far)
            continue;
          }
          userInsts.push_back(pUserInst);
      }
      //Run over all saved user instructions and handle by adding
      //load instruction before each value use
      for ( TInstructionVector::iterator ui = userInsts.begin(),
        ue = userInsts.end(); ui != ue; ) {
          Instruction *pUserInst = *ui;
          Instruction *pInsertBefore = getInstructionToInsertBefore(pInst, pUserInst, true);
          if ( !pInsertBefore ) {
            //as no barrier in the middle, no need to load & replace the origin value
            ++ui; // Need to increment iterator!
            continue;
          }
          //Calculate the pointer of the current special in the special buffer
          Value *pLoadedValue = new LoadInst(pAllocaInst, "loadedValue", pInsertBefore);
          //Replace the use of old value with the new loaded value from special buffer
          ++ui; // Need to increment iterator before replace it!
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
            m_pCurrWIValue, pPreSyncBB->begin());

          new StoreInst(ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, 0)),
            m_pCurrSBValue, pPreSyncBB->begin());

          new StoreInst(ConstantInt::get(Type::getInt32Ty(*m_pContext), APInt(32, id)),
            m_pCurrBarrierValue, pPreSyncBB->begin());
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
        LoadInst *pLoadedCurrWI = new LoadInst(m_pCurrWIValue, "loadedCurrWI", pPreSyncBB);
        ICmpInst *pCheckWIIter = new ICmpInst(*pPreSyncBB, ICmpInst::ICMP_ULT,
          pLoadedCurrWI, m_pWIIterationCountValue, "check.WI.iter");
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
        //Value *pLoadedCurrWI = new LoadInst(m_pCurrWIValue, "loadedCurrWI", pThenBB);
        Value *pUpdatedCurrWI = BinaryOperator::CreateNUWAdd(pLoadedCurrWI,
          ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, 1)), "CurrWI++", pThenBB);
        new StoreInst(pUpdatedCurrWI, m_pCurrWIValue, pThenBB);

        //Create "CurrSBBase+=Stride" code
        Value *pLoadedCurrSB = new LoadInst(m_pCurrSBValue, "loadedCurrSB", pThenBB);
        Value *pUpdatedCurrSB = BinaryOperator::CreateNUWAdd(pLoadedCurrSB,
          m_pStructureSizeValue, "loadedCurrSB+Stride", pThenBB);
        new StoreInst(pUpdatedCurrSB, m_pCurrSBValue, pThenBB);

        Instruction *pFirstSyncInst = *pSyncPreds->begin();
        if ( numCases == 1 ) {
          //Only one case, no need for switch, create unconditional jump
          BranchInst::Create(pFirstSyncInst->getParent(), pThenBB);
        } else {
          //More than one case, create a switch
          //The first sync instruction is chosen to be the switch Default case
          Value *pLoadedCurrBarrier = new LoadInst(m_pCurrBarrierValue, "loadedCurrBarrier", pThenBB);
          SwitchInst *pSwitch = SwitchInst::Create(pLoadedCurrBarrier, pFirstSyncInst->getParent(), numCases-1, pThenBB);
          for ( TInstructionVector::iterator ii = pSyncPreds->begin(),
            ie = pSyncPreds->end(); (++ii) != ie; ) {
              Instruction *pSyncInst = *ii;
              unsigned int predId = m_pDataPerBarrier->getUniqueID(pSyncInst);
              pSwitch->addCase(ConstantInt::get(*m_pContext, APInt(32, predId)), pSyncInst->getParent());
          }
        }

        //C. Create initialization to currWI, currSB and currBarrier in pElseBB
        // currWI = 0
        // currSB = 0
        // currBarrier = id
        //And connect the pElseBB to the pSyncBB with unconditional jump
        new StoreInst(ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, 0)),
          m_pCurrWIValue, pElseBB);
        new StoreInst(ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, 0)),
          m_pCurrSBValue, pElseBB);
        new StoreInst(ConstantInt::get(Type::getInt32Ty(*m_pContext), APInt(32, id)),
          m_pCurrBarrierValue, pElseBB);
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
        BranchInst::Create(pSyncBB, pElseBB);

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

  void Barrier::initArgumentValues(Instruction *pInsertBefore, bool hasNoInternalCalls) {
    //Add currBarrier alloca
    m_pCurrBarrierValue = new AllocaInst(Type::getInt32Ty(*m_pContext),
      "currBarrier", pInsertBefore);

    //Will hold the index in special buffer and will be increased by stride size
    m_pCurrSBValue = new AllocaInst(m_sizeTType, "CurrSBIndex", pInsertBefore);

    //get_curr_wi()
    if (hasNoInternalCalls ) {
      m_pCurrWIValue = new AllocaInst(m_sizeTType, "CurrWI", pInsertBefore);
    } else {
      m_pCurrWIValue = m_util.createGetCurrWI(pInsertBefore);
    }

    //get_special_buffer()
    m_pSpecialBufferValue = m_util.createGetSpecialBuffer(pInsertBefore);

    //get_iter_count()
    m_pWIIterationCountValue = m_util.createGetIterCount(pInsertBefore);

    if (m_isNativeDBG) {
      // Move alloca instructions for locals/parameters for debugging purposes
      for (TValueVector::iterator vi = m_pAllocaValues->begin(), ve = m_pAllocaValues->end();
           vi != ve; ++vi ) {
        AllocaInst *pAllocaInst = cast<AllocaInst>(*vi);
        pAllocaInst->moveBefore(pInsertBefore);
      }
    }
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
    unsigned int offset, PointerType *pType, Instruction *pInsertBefore){
      Value *pOffsetVal = ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, offset));
      //If hit this assert then need to handle PHINode!
      assert( !isa<PHINode>(pInsertBefore) && "cannot add instructions before a PHI node!" );
      //Calculate the pointer of the given offset for currWI in the special buffer
      Value *Idxs[1];
      Value *pLoadedCurrSB = new LoadInst(m_pCurrSBValue, "loadedCurrSB", pInsertBefore);
      Idxs[0] = BinaryOperator::CreateNUWAdd(pLoadedCurrSB,
        pOffsetVal, "&(pSB[currWI].offset)", pInsertBefore);
      Value *pAddrInSpecialBuffer = GetElementPtrInst::CreateInBounds(
        m_pSpecialBufferValue, ArrayRef<Value*>(Idxs), "&pSB[currWI].offset", pInsertBefore);
      //Bitcast pointer according to alloca type!
      pAddrInSpecialBuffer = BitCastInst::CreatePointerCast(
        pAddrInSpecialBuffer, pType, "CastToValueType", pInsertBefore);
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
        if( !m_pCurrWIperFunction.count(pFunc) ) {
          m_pCurrWIperFunction[pFunc] = m_util.createGetCurrWI(pFunc->begin()->begin());
        }
        //Replace get_local_id(arg) with get_new_local_id(arg, *pCurrWI)
        Value *args1 = pOldCall->getOperand(0);
        Value *args2 = new LoadInst(m_pCurrWIperFunction[pFunc], "currWI", pOldCall);
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
        if( !m_pCurrWIperFunction.count(pFunc) ) {
          m_pCurrWIperFunction[pFunc] = m_util.createGetCurrWI(pFunc->begin()->begin());
        }
        //Replace get_global_id(arg) with get_new_global_id(arg, *pCurrWI)
        Value *args1 = pOldCall->getOperand(0);
        Value *args2 = new LoadInst(m_pCurrWIperFunction[pFunc], "currWI", pOldCall);
        Instruction *pNewCall = m_util.createNewGetGlobalId(args1, args2, pOldCall);
        pOldCall->replaceAllUsesWith(pNewCall);
        m_toRemoveInstructions.push_back(pOldCall);
    }

    //Remove all instructions in m_toRemoveInstructions
    eraseAllToRemoveInstructions();

    return true;
  }

  bool Barrier::fixNonInlinedInternalFunctions(Module &M) {
    TFunctionVector &functionsTofix =
      m_pDataPerInternalFunction->getOrderedFunctionsToFix();
    //Run over all functions to be fixed in the right order (from leaf to root)
    for ( TFunctionVector::iterator fi = functionsTofix.begin(),
      fe = functionsTofix.end(); fi != fe; ++fi ) {
        Function *pFuncToFix = *fi;
        //Create fixed version of the function with extra offset parameters
        Function *pNewFunc = createFixFunctionVersion(pFuncToFix);

        //clear container for new iteration on new function
        m_toRemoveInstructions.clear();
        //Run over old uses of pFuncToFix and replace with call to pNewFunc
        for ( Value::use_iterator ui = pFuncToFix->use_begin(),
          ue = pFuncToFix->use_end(); ui != ue; ++ui ) {
            CallInst *pCallInst = cast<CallInst>(*ui);
            //Create call instruction to new function version
            if ( createFixedCallInstruction(pCallInst, pFuncToFix, pNewFunc) ) {
              //If new call instruction created we can remove the old one
              m_toRemoveInstructions.push_back(pCallInst);
            }
        }

        if (m_isNativeDBG) {
            // Since pFuncToFix is being replaced, also update the associated debug metadata
            NamedMDNode *pllvmDebugCU = pFuncToFix->getParent()->getNamedMetadata("llvm.dbg.cu");
            if (pllvmDebugCU) {
                for(int ui = 0, ue = pllvmDebugCU->getNumOperands(); ui < ue; ui++) {
                  MDNode* pMetadata = pllvmDebugCU->getOperand(ui);
                  replaceMDUsesOfFunc(pMetadata, pFuncToFix, pNewFunc);
                }
            }
        }
        //Remove all instructions in m_toRemoveInstructions
        eraseAllToRemoveInstructions();
    }
    return true;
  }

  void Barrier::replaceMDUsesOfFunc(MDNode* pMetadata, Function* pFunc, Function* pNewFunc) {
    m_pFunc = pFunc;
    m_pNewF = pNewFunc;
    replaceMDUsesOfFunc(pMetadata);
  }

  void Barrier::replaceMDUsesOfFunc(MDNode* pMetadata) {
    SmallVector<Value *, 16> values;
    for (int i = 0, e = pMetadata->getNumOperands(); i < e; ++i) {
      Value *elem = pMetadata->getOperand(i);
      if (elem) {
        if (MDNode *Node = dyn_cast<MDNode>(elem))
            replaceMDUsesOfFunc(Node);

        // Elem needs to be set again otherwise changes will be undone.
        elem = pMetadata->getOperand(i);
        if (m_pFunc == dyn_cast<Function>(elem))
          elem = m_pNewF;
      }
      values.push_back(elem);
    }
    MDNode* pNewMetadata = MDNode::get(*m_pContext, ArrayRef<Value*>(values));
    // TODO: Why may pMetadata and pNewMetadata be the same value ?
    if (pMetadata != pNewMetadata)
      pMetadata->replaceAllUsesWith(pNewMetadata);
  }

  Function* Barrier::createFixFunctionVersion(Function *pFuncToFix) {
    std::vector<llvm::Type *> newArgsVec;

    //Add original parameter types
    for ( Function::ArgumentListType::iterator ai = pFuncToFix->getArgumentList().begin(),
      ae = pFuncToFix->getArgumentList().end(); ai != ae; ++ai ) {
        newArgsVec.push_back(ai->getType());
    }

    //Add extra parameter of size_t type for offsets in special buffer
    unsigned int numOfArgs = pFuncToFix->getFunctionType()->getNumParams();
    bool hasReturnValue = !pFuncToFix->getFunctionType()->getReturnType()->isVoidTy();
    //Keep one last argument for return value
    unsigned int numOfArgsWithReturnValue = hasReturnValue ? numOfArgs+1 : numOfArgs;
    for ( unsigned int i = 0; i < numOfArgsWithReturnValue; ++i ) {
      //Having a function that need to be fixed means it is a non-inline function.
      //A call instruction to such function has implicit barrier before it and
      //dummpyBarrier after it, thus if value passed to the function call depends
      //on work-item id then it must be in special buffer.
      if ( m_pDataPerInternalFunction->isInSpecialBuffer(pFuncToFix, i) ) {
        newArgsVec.push_back(m_sizeTType);
      }
    }

    FunctionType *FTy = FunctionType::get(pFuncToFix->getReturnType(), newArgsVec, false);

    Function *pNewFunc = Function::Create(FTy, pFuncToFix->getLinkage(), pFuncToFix->getName()+"_New");
    pNewFunc->setCallingConv(pFuncToFix->getCallingConv());

    llvm::ValueToValueMapTy ValueMap;
    TValueVector newFuncArgs;

    //Copy parameter names of original function to new function
    //Map between original parameters of original with of new function
    Function::arg_iterator dai = pNewFunc->arg_begin();
    for ( Function::const_arg_iterator sai = pFuncToFix->arg_begin(),
      sae = pFuncToFix->arg_end(); sai != sae; ++sai, ++dai) {
        dai->setName(sai->getName());
        ValueMap[sai] = dai;
        newFuncArgs.push_back(&*dai);
    }

    //Give new parameters of new function a proper name "offset"
    for ( unsigned int i = 0; i < numOfArgsWithReturnValue; ++i ) {
      if ( m_pDataPerInternalFunction->isInSpecialBuffer(pFuncToFix, i) ) {
        dai->setName("offset");
        newFuncArgs.push_back(&*dai);
        dai++;
      }
    }
    SmallVector<ReturnInst*, 8> Returns;
    CloneFunctionInto(pNewFunc, pFuncToFix, ValueMap, true, Returns, "", NULL);

    pFuncToFix->getParent()->getFunctionList().push_back(pNewFunc);

    // Identify the CurrWI/CurrSB/WIIterCount in the new function using the clone mapping.
    m_pCurrWIValue = ValueMap[m_pCurrWIperFunction[pFuncToFix]];
    m_pCurrSBValue = ValueMap[m_pCurrSBPerFunction[pFuncToFix]];
    m_pWIIterationCountValue = ValueMap[m_pIterationCountPerFunction[pFuncToFix]];
    assert( m_pCurrWIValue && "Did not find the \"CurrWI\" instruction" );
    assert( m_pCurrSBValue && "Did not find the \"CurrSBIndex\" instruction" );
    assert( m_pWIIterationCountValue && "Did not find the \"IterCount\" instruction" );

    //Add get_special_buffer()
    m_pSpecialBufferValue = m_util.createGetSpecialBuffer(pNewFunc->begin()->begin());

    m_pBadOffsetValue = ConstantInt::get(
      m_sizeTType, DataPerInternalFunction::m_badOffset);
    //Use offsets instead of original parameters
    unsigned int currNewIndex = numOfArgs;
    for ( unsigned int i = 0; i < numOfArgs; ++i ) {
      if ( m_pDataPerInternalFunction->isInSpecialBuffer(pFuncToFix, i) ) {
        Value *pOriginalArg = newFuncArgs[i];
        Value *pOffsetArg = newFuncArgs[currNewIndex];
        bool alwaysInSB =
          m_pDataPerInternalFunction->alwaysInSpecialBuffer(pFuncToFix, i);
        fixArgumentUsage(pOriginalArg, pOffsetArg, alwaysInSB);
        currNewIndex++;
      }
    }
    if ( hasReturnValue &&
      //The last argument for return value
      m_pDataPerInternalFunction->isInSpecialBuffer(pFuncToFix, numOfArgs) ) {
      Value *pOffsetArg = newFuncArgs[currNewIndex];

      bool alwaysInSB =
        m_pDataPerInternalFunction->alwaysInSpecialBuffer(pFuncToFix, numOfArgs);
      //Shut the warning up
      (void)alwaysInSB;

      std::vector<BasicBlock*> pVecBB;
      for ( Function::iterator bi = pNewFunc->begin(), be = pNewFunc->end(); bi != be; ++bi ) {
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
        assert(alwaysInSB && "Some of the functions callers has no offset for return value!!");
        Value *pRetVal = pRetInst->getOperand(0);
        if ( Instruction *pInst = dyn_cast<Instruction>(pRetVal) ) {
          //Find next instruction so we can create new instruction before it
          Instruction *pNextInst = &*(++BasicBlock::iterator(pInst));
          if ( isa<PHINode>(pNextInst) ) {
            //pNextInst is a PHINode, find first non PHINode to add instructions before it
            pNextInst = pNextInst->getParent()->getFirstNonPHI();
          }
          fixReturnValue(pRetVal, pOffsetArg, pNextInst);
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
          //      pSB[pCurrSBValue+pOffsetArg] = pRetVal
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

          //Value *pLoadedCurrWI = new LoadInst(m_pCurrWIValue, "loadedCurrWI", pThenBB);
          Value *pLoadedCurrWI = new LoadInst(m_pCurrWIValue, "loadedCurrWI", pLoopBB);
          ICmpInst *pCheckWIIter = new ICmpInst(*pLoopBB, ICmpInst::ICMP_ULT,
            pLoadedCurrWI, m_pWIIterationCountValue, "check.WI.iter");

          Value *pUpdatedCurrWI = BinaryOperator::CreateNUWAdd(pLoadedCurrWI,
            ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, 1)), "CurrWI++", pLoopBB);
          new StoreInst(pUpdatedCurrWI, m_pCurrWIValue, pLoopBB);

          //Create "CurrSBBase+=Stride" code
          Value *pLoadedCurrSB = new LoadInst(m_pCurrSBValue, "loadedCurrSB", pLoopBB);
          Value *pUpdatedCurrSB = BinaryOperator::CreateNUWAdd(pLoadedCurrSB,
            m_pStructureSizeValue, "loadedCurrSB+Stride", pLoopBB);
          new StoreInst(pUpdatedCurrSB, m_pCurrSBValue, pLoopBB);

          BranchInst::Create(pLoopBB, pNewRetBB, pCheckWIIter, pLoopBB);

          Instruction *pNextInst = pLoopBB->getFirstNonPHI();
          fixReturnValue(pRetVal, pOffsetArg, pNextInst);
        }
      }
    }
    return pNewFunc;
  }

  void Barrier::fixArgumentUsage(Value *pOriginalArg, Value *pOffsetArg, bool alwaysInSB) {
    assert( (!m_pDataPerValue->isOneBitElementType(pOriginalArg) ||
      !isa<VectorType>(pOriginalArg->getType())) && "pOriginalArg with base type i1!");
    for ( Value::use_iterator ui = pOriginalArg->use_begin(),
      ue = pOriginalArg->use_end(); ui != ue; ) {
        Instruction *pUserInst = dyn_cast<Instruction>(*ui);
        assert( pUserInst && "Something other than Instruction is using function argument!" );
        Instruction *pInsertBefore = pUserInst;
        if ( isa<PHINode>(pUserInst) ) {
          BasicBlock *pPrevBB = BarrierUtils::findBasicBlockOfUsageInst(pOriginalArg, pUserInst);
          pInsertBefore = pPrevBB->getTerminator();
        }
        if ( alwaysInSB ) {
          //In this case we will always get a valid offset and need to load the argument
          //from the special buffer using the offset corresponding argument
          Value *Idxs[1];
          Value *pLoadedCurrSB = new LoadInst(m_pCurrSBValue, "loadedCurrSB", pInsertBefore);
          Idxs[0] = BinaryOperator::CreateNUWAdd(pLoadedCurrSB,
            pOffsetArg, "&(pSB[currWI].offset)", pInsertBefore);
          Value *pAddrInSpecialBuffer = GetElementPtrInst::CreateInBounds(
            m_pSpecialBufferValue, ArrayRef<Value*>(Idxs), "&pSB[currWI].offset", pInsertBefore);
          //Bitcast pointer according to alloca type!
          PointerType *pType = PointerType::get(pOriginalArg->getType(), SPECIAL_BUFFER_ADDR_SPACE);
          pAddrInSpecialBuffer = BitCastInst::CreatePointerCast(
            pAddrInSpecialBuffer, pType, "CastToArgType", pInsertBefore);
          Value *pLoadedValue = new LoadInst(pAddrInSpecialBuffer, "loadedValue", pInsertBefore);
          ++ui; // Need to increment iterator before replace it!
          pUserInst->replaceUsesOfWith(pOriginalArg, pLoadedValue);
          continue;
        }
        //Need to check when offset is bad Offset in order to use right parameter!
        BasicBlock *pConditionBB = pInsertBefore->getParent();
        BasicBlock *pPHINodeBB =
          pConditionBB->splitBasicBlock(BasicBlock::iterator(pInsertBefore), "");
        //Create then and else basic-blocks
        BasicBlock *pThenBB = BasicBlock::Create(
          *m_pContext, "thenBB", pConditionBB->getParent(), pPHINodeBB);

        //A. change the pConditionBB basic block as follow
        //A(1). remove the unconditional jump instruction
        pConditionBB->getTerminator()->eraseFromParent();

        //A(2). add the entry tail code
        // if(offset != badOffset) {pThenBB} else {pPHINodeBB}
        ICmpInst *checkBadOffset = new ICmpInst(*pConditionBB, ICmpInst::ICMP_NE,
          pOffsetArg, m_pBadOffsetValue, "check.bad.offset");
        BranchInst::Create(pThenBB, pPHINodeBB, checkBadOffset, pConditionBB);

        //B. Create load from special buffer using offset in pThenBB
        Value *Idxs[1];
        Value *pLoadedCurrSB = new LoadInst(m_pCurrSBValue, "loadedCurrSB", pThenBB);
        Idxs[0] = BinaryOperator::CreateNUWAdd(pLoadedCurrSB,
          pOffsetArg, "&(pSB[currWI].offset)", pThenBB);
        Value *pAddrInSpecialBuffer = GetElementPtrInst::CreateInBounds(
          m_pSpecialBufferValue, ArrayRef<Value*>(Idxs), "&pSB[currWI].offset", pThenBB);
        //Bitcast pointer according to alloca type!
        PointerType *pType = PointerType::get(pOriginalArg->getType(), SPECIAL_BUFFER_ADDR_SPACE);
        pAddrInSpecialBuffer = BitCastInst::CreatePointerCast(
          pAddrInSpecialBuffer, pType, "CastToArgType", pThenBB);
        Value *pLoadedValue = new LoadInst(pAddrInSpecialBuffer, "loadedValue", pThenBB);
        //Create unconditional jump instruction from pThenBB to pPHINodeBB
        BranchInst::Create(pPHINodeBB, pThenBB);

        //C. Create PHINode at the begining of the pPHINodeBB to represent the
        //   Valid parameter and then replace the original parameter with this value
        PHINode* pValidArg = PHINode::Create(pOriginalArg->getType(), 2, "", pPHINodeBB->begin());
        pValidArg->addIncoming(pLoadedValue, pThenBB);
        pValidArg->addIncoming(pOriginalArg, pConditionBB);
        ++ui; // Need to increment iterator before replace it!
        pUserInst->replaceUsesOfWith(pOriginalArg, pValidArg);
    }
  }

  void Barrier::fixReturnValue(Value *pRetVal, Value *pOffsetArg, Instruction* pNextInst) {
    assert( (!m_pDataPerValue->isOneBitElementType(pRetVal) ||
      !isa<VectorType>(pRetVal->getType())) && "pRetVal with base type i1!");
    //pRetVal might be a result of calling other function itself
    //in such case no need to handle it here as it will be saved
    //to the special buffer by the called function itself.
    if ( !( isa<CallInst>(pRetVal) &&
      m_pDataPerInternalFunction->needToBeFixed(cast<CallInst>(pRetVal)) ) ) {
        //Calculate the pointer of the current special in the special buffer
        Value *Idxs[1];
        Value *pLoadedCurrSB = new LoadInst(m_pCurrSBValue, "loadedCurrSB", pNextInst);
        Idxs[0] = BinaryOperator::CreateNUWAdd(pLoadedCurrSB,
          pOffsetArg, "&(pSB[currWI].offset)", pNextInst);
        Value *pAddrInSpecialBuffer = GetElementPtrInst::CreateInBounds(
          m_pSpecialBufferValue, ArrayRef<Value*>(Idxs), "&pSB[currWI].offset", pNextInst);
        //Bitcast pointer according to alloca type!
        PointerType *pType = PointerType::get(pRetVal->getType(), SPECIAL_BUFFER_ADDR_SPACE);
        pAddrInSpecialBuffer = BitCastInst::CreatePointerCast(
          pAddrInSpecialBuffer, pType, "CastToArgType", pNextInst);
        //Add Store instruction after the value instruction
        new StoreInst(pRetVal, pAddrInSpecialBuffer, pNextInst);
    }
  }

  bool Barrier::createFixedCallInstruction(
    CallInst *pOriginalCall, Function *pOriginalFunc, Function *pNewFunc) {
    if ( !m_pDataPerInternalFunction->needToBeFixed(pOriginalCall) ) {
      //No need to fix this call instruction
      return false;
    }

    SmallVector<Value*, 16> params;
    // Create new call instruction with extended parameters
    params.clear();
    unsigned int numOfArgs = pOriginalCall->getNumArgOperands();
    bool hasReturnValue = !(pOriginalCall->getType()->isVoidTy());
    // Keep one last argument for return value
    unsigned int numOfArgsWithReturnValue = hasReturnValue ? numOfArgs+1 : numOfArgs;
    for ( unsigned int i = 0; i < numOfArgs; ++i ) {
      params.push_back(pOriginalCall->getArgOperand(i));
    }
    for ( unsigned int i = 0; i < numOfArgsWithReturnValue; ++i ) {
      if ( m_pDataPerInternalFunction->isInSpecialBuffer(pOriginalFunc, i) ) {
        unsigned int offset =
          m_pDataPerInternalFunction->getOffset(pOriginalCall, i);
        Value *pOffsetVal = ConstantInt::get(m_sizeTType, offset);
        params.push_back(pOffsetVal);
      }
    }

    CallInst *pNewCall = CallInst::Create(
      pNewFunc, ArrayRef<Value*>(params), "", pOriginalCall);
    pNewCall->setCallingConv(pOriginalCall->getCallingConv());

    pOriginalCall->replaceAllUsesWith(pNewCall);
    return true;
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
