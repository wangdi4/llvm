/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/
#include "BarrierPass.h"

#include "llvm/Instructions.h"
#include "llvm/Support/CFG.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Support/InstIterator.h"

namespace intel {

  char Barrier::ID = 0;

  Barrier::Barrier() : ModulePass(ID) {}

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
    m_pSyncInstructions = &m_pDataPerBarrier->getSyncInstructions(&F);

    //Check if function has no synchronize instructions!
    if ( !m_pDataPerBarrier->hasSyncInstruction(&F) ) {
      //Function has no barrier: do nothing!
      return true;
    }

    m_pSpecialValues = &m_pDataPerValue->getValuesToHandle(&F);
    m_pAllocaValues = &m_pDataPerValue->getAllocaValuesToHandle(&F);
    m_pCrossBarrierValues = &m_pDataPerValue->getUniformValuesToHandle(&F);

    unsigned int structureSize = m_pDataPerValue->getTotalSize();
    m_pStructureSizeValue = ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, structureSize));

    //Clear container for new iteration on new function
    m_toRemoveInstructions.clear();

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

    //Fix special values
    fixSpecialValues();
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

  void Barrier::fixAllocaValues() {
    TInstructionVector userInsts;
    TValueVector::iterator vi = m_pAllocaValues->begin();
    TValueVector::iterator ve = m_pAllocaValues->end();
    for ( ; vi != ve; ++vi ) {
      AllocaInst *pAllocaInst = dyn_cast<AllocaInst>(*vi);
      assert( pAllocaInst && "container of alloca values has non AllocaInst value!" );
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
          Instruction *pUserInst = dyn_cast<Instruction>(*ui);
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
      //Get offset of special value in special buffer
      unsigned int offset = m_pDataPerValue->getOffset(pInst);
      //Find next instruction so we can create new instruction before it
      Instruction *pNextInst = dyn_cast<Instruction>(&*(++BasicBlock::iterator(pInst)));
      if ( isa<PHINode>(pNextInst) ) {
        //pNextInst is a PHINode, find first non PHINode to add instructions before it
        pNextInst = pNextInst->getParent()->getFirstNonPHI();
      }
      //Get PointerType of value type
      PointerType *pType = PointerType::get(pInst->getType(), SPECIAL_BUFFER_ADDR_SPACE);
      //Handle Special buffer only if it is not a call instruction.
      //Special buffer value of call instruction will be handled in the callee.
      if( !( isa<CallInst>(pInst) &&
        m_pDataPerInternalFunction->needToBeFixed(dyn_cast<CallInst>(pInst)) ) ) {
          //Calculate the pointer of the current special in the special buffer
          Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(offset, pType, pNextInst);
          //Add Store instruction after the value instruction
          new StoreInst(pInst, pAddrInSpecialBuffer, pNextInst);
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
          Instruction *pUserInst = dyn_cast<Instruction>(*ui);
          Instruction *pInsertBefore = getInstructionToInsertBefore(pInst, pUserInst, true);
          if ( !pInsertBefore ) {
            //as no barrier in the middle, no need to load & replace the origin value
            ++ui; // Need to increment iterator!
            continue;
          }
          //Calculate the pointer of the current special in the special buffer
          Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(offset, pType, pInsertBefore);
          Value *pLoadedValue = new LoadInst(pAddrInSpecialBuffer, "loadedValue", pInsertBefore);
          //Replace the use of old value with the new loaded value from special buffer
          ++ui; // Need to increment iterator before replace it!
          pUserInst->replaceUsesOfWith(pInst, pLoadedValue);
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
      Instruction *pNextInst = dyn_cast<Instruction>(&*(++BasicBlock::iterator(pInst)));
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
          Instruction *pUserInst = dyn_cast<Instruction>(*ui);
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
        pInst->getParent()->splitBasicBlock(BasicBlock::iterator(pInst), "SyncBB");
        m_toRemoveInstructions.push_back(pInst);
    }
    for ( TInstructionSet::iterator ii = m_pSyncInstructions->begin(),
      ie = m_pSyncInstructions->end(); ii != ie; ++ii ) {
        Instruction *pInst = dyn_cast<Instruction>(*ii);
        assert( pInst && "sync instruction container contains non instruction!" );
        unsigned int id = m_pDataPerBarrier->getUniqueID(pInst);
        SYNC_TYPE type = m_pDataPerBarrier->getSyncType(pInst);
        BasicBlock *pSyncBB = pInst->getParent();
        BasicBlock *pPreSyncBB = pSyncBB->getPrevNode();
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
        Value *pLoadedCurrWI = new LoadInst(m_pCurrWIValue, "loadedCurrWI", pPreSyncBB);
        ICmpInst *pCheckWIIter = new ICmpInst(*pPreSyncBB, ICmpInst::ICMP_ULT,
          pLoadedCurrWI, m_pWIIterationCountValue, "check.WI.iter");
        BranchInst::Create(pThenBB, pElseBB, pCheckWIIter, pPreSyncBB);

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

        Instruction *pFirstSyncInst = dyn_cast<Instruction>(*pSyncPreds->begin());
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
              Instruction *pSyncInst = dyn_cast<Instruction>(*ii);
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
        CallInst *pBarrier = dyn_cast<CallInst>(pInst);
        assert( pBarrier && "Barrier instruction is not a CallInst!" );
        Value *pArg1 = pBarrier->getOperand(0);
        assert( pArg1 && "Barrier instruction has no first argument!" );
        ConstantInt *pMemFence = dyn_cast<ConstantInt>(pArg1);
        assert( pMemFence && "Barrier instruction has non constant first argument (memory fence)!" );
        if ( pMemFence->getZExtValue() & CLK_GLOBAL_MEM_FENCE ) {
          //barrier(global): add mem_fence instruction!
          m_util.createMemFence(pElseBB);
        }
        BranchInst::Create(pSyncBB, pElseBB);
    }
  }

  void Barrier::initArgumentValues(Instruction *pInsertBefore, bool hasNoInternalCalls) {
    //Add currBarrier alloca
    m_pCurrBarrierValue = new AllocaInst(Type::getInt32Ty(*m_pContext),
      "currBarrier", pInsertBefore);

    //Will hold the index in special buffer and will be increased by stride size
    m_pCurrSBValue = new AllocaInst(m_sizeTType, CURR_SB_INDEX_ALLOCA, pInsertBefore);

    //get_curr_wi()
    if (hasNoInternalCalls ) {
      m_pCurrWIValue = new AllocaInst(m_sizeTType, CURR_WI_ALLOCA, pInsertBefore);
    } else {
      m_pCurrWIValue = m_util.createGetCurrWI(pInsertBefore);
    }

    //get_special_buffer()
    m_pSpecialBufferValue = m_util.createGetSpecialBuffer(pInsertBefore);

    //get_iter_count()
    m_pWIIterationCountValue = m_util.createGetIterCount(pInsertBefore);
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
    unsigned int offset, const PointerType *pType, Instruction *pInsertBefore){
      Value *pOffsetVal = ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, offset));
      //If hit this assert then need to handle PHINode!
      assert( !isa<PHINode>(pInsertBefore) && "cannot add instructions before a PHI node!" );
      //Calculate the pointer of the given offset for currWI in the special buffer
      Value *Idxs[1];
      Value *pLoadedCurrSB = new LoadInst(m_pCurrSBValue, "loadedCurrSB", pInsertBefore);
      Idxs[0] = BinaryOperator::CreateNUWAdd(pLoadedCurrSB,
        pOffsetVal, "&(pSB[currWI].offset)", pInsertBefore);
      Value *pAddrInSpecialBuffer = GetElementPtrInst::CreateInBounds(
        m_pSpecialBufferValue, Idxs, Idxs+1, "&pSB[currWI].offset", pInsertBefore);
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
        Function *pFuncToFix = dyn_cast<Function>(*fi);
        //Create fixed version of the function with extra offset parameters
        Function *pNewFunc = createFixFunctionVersion(pFuncToFix);

        //clear container for new iteration on new function
        m_toRemoveInstructions.clear();
        //Run over old uses of pFuncToFix and replace with call to pNewFunc
        for ( Value::use_iterator ui = pFuncToFix->use_begin(),
          ue = pFuncToFix->use_end(); ui != ue; ++ui ) {
            CallInst *pCallInst = dyn_cast<CallInst>(*ui);
            //Create call instruction to new function version
            if ( createFixedCallInstruction(pCallInst, pFuncToFix, pNewFunc) ) {
              //If new call instruction created we can remove the old one
              m_toRemoveInstructions.push_back(pCallInst);
            }
        }
        //Remove all instructions in m_toRemoveInstructions
        eraseAllToRemoveInstructions();
    }
    return true;
  }

  Function* Barrier::createFixFunctionVersion(Function *pFuncToFix) {
    std::vector<const llvm::Type *> newArgsVec;

    //Add original parameter types
    for ( Function::ArgumentListType::iterator ai = pFuncToFix->getArgumentList().begin(),
      ae = pFuncToFix->getArgumentList().end(); ai != ae; ++ai ) {
        newArgsVec.push_back(ai->getType());
    }

    //Add extra parameter of i32 type for offsets in special buffer
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

    ValueMap<const Value*, Value*> ValueMap;
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

    //Find "CurrSBIndex" Alloca instruction
    m_pCurrSBValue = 0;
    //TODO: can we just check at the first basic block?
    bool bCurrSBIndex = false;
    bool bCurrWI = false;
    bool bIterCount = false;
    for ( inst_iterator ii = inst_begin(pNewFunc), ie = inst_end(pNewFunc); ii != ie; ++ii ) {
      if ( AllocaInst *pAllocaInst = dyn_cast<AllocaInst>(&*ii) ) {
        if ( !pAllocaInst->getNameStr().compare(CURR_SB_INDEX_ALLOCA) ) {
          //Found the "CurrSBIndex." Alloca instruction
          m_pCurrSBValue = pAllocaInst;
          bCurrSBIndex = true;
        } else if ( !pAllocaInst->getNameStr().compare(CURR_WI_ALLOCA) ) {
          //Found the "CurrWI." Alloca instruction
          m_pCurrWIValue = pAllocaInst;
          bCurrWI = true;
        }
      } else if ( CallInst *pCallInst = dyn_cast<CallInst>(&*ii) ) {
        if ( !pCallInst->getNameStr().compare(GET_ITER_COUNT) ) {
          //Found the "IterCount." Alloca instruction
          m_pWIIterationCountValue = pCallInst;
          bIterCount = true;
        }
      }
      if ( bCurrSBIndex && bCurrWI && bIterCount ) {
        break;
      }
    }
    assert( m_pCurrSBValue && "Did not find the \"CurrSBIndex.\" Alloca instruction" );
    assert( m_pCurrWIValue && "Did not find the \"CurrWI.\" Alloca instruction" );
    assert( m_pWIIterationCountValue && "Did not find the \"IterCount.\" Call instruction" );
    //Add get_special_buffer()
    m_pSpecialBufferValue = m_util.createGetSpecialBuffer(pNewFunc->begin()->begin());

    m_pBadOffsetValue = ConstantInt::get(
      IntegerType::getInt32Ty(*m_pContext), DataPerInternalFunction::m_badOffset);
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
#ifdef _DEBUG
      bool alwaysInSB = 
#endif
        m_pDataPerInternalFunction->alwaysInSpecialBuffer(pFuncToFix, numOfArgs);
      std::vector<BasicBlock*> pVecBB;
      for ( Function::iterator bi = pNewFunc->begin(), be = pNewFunc->end(); bi != be; ++bi ) {
        BasicBlock *pBB = dyn_cast<BasicBlock>(&*bi);
        pVecBB.push_back(pBB);
      }
      //Run over all basic blocks of the new function and handle return terminators
      for ( std::vector<BasicBlock*>::iterator bi = pVecBB.begin(), be = pVecBB.end(); bi != be; ++bi ) {  
        BasicBlock *pBB = dyn_cast<BasicBlock>(*bi);
        ReturnInst *pRetInst = dyn_cast<ReturnInst>(pBB->getTerminator());
        if ( !pRetInst ) {
          //It is not return instruction terminator, check next basic block
          continue;
        }
        assert(alwaysInSB && "Some of the functions callers has no offset for return value!!");
        Value *pRetVal = pRetInst->getOperand(0);
        if ( Instruction *pInst = dyn_cast<Instruction>(pRetVal) ) {
          //Find next instruction so we can create new instruction before it
          Instruction *pNextInst = dyn_cast<Instruction>(&*(++BasicBlock::iterator(pInst)));
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
            m_pSpecialBufferValue, Idxs, Idxs+1, "&pSB[currWI].offset", pInsertBefore);
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
          m_pSpecialBufferValue, Idxs, Idxs+1, "&pSB[currWI].offset", pThenBB);
        //Bitcast pointer according to alloca type!
        PointerType *pType = PointerType::get(pOriginalArg->getType(), SPECIAL_BUFFER_ADDR_SPACE);
        pAddrInSpecialBuffer = BitCastInst::CreatePointerCast(
          pAddrInSpecialBuffer, pType, "CastToArgType", pThenBB);
        Value *pLoadedValue = new LoadInst(pAddrInSpecialBuffer, "loadedValue", pThenBB);
        //Create unconditional jump instruction from pThenBB to pPHINodeBB
        BranchInst::Create(pPHINodeBB, pThenBB);

        //C. Create PHINode at the begining of the pPHINodeBB to represent the
        //   Valid parameter and then replace the original parameter with this value
        PHINode* pValidArg = PHINode::Create(pOriginalArg->getType(), "", pPHINodeBB->begin());
    pValidArg->reserveOperandSpace(2);
    pValidArg->addIncoming(pLoadedValue, pThenBB);
    pValidArg->addIncoming(pOriginalArg, pConditionBB);
        ++ui; // Need to increment iterator before replace it!
        pUserInst->replaceUsesOfWith(pOriginalArg, pValidArg);
    }
  }

  void Barrier::fixReturnValue(Value *pRetVal, Value *pOffsetArg, Instruction* pNextInst) {
    //pRetVal might be a result of calling other function itself
    //in such case no need to handle it here as it will be saved
    //to the special buffer by the called function itself.
    if ( !( isa<CallInst>(pRetVal) &&
      m_pDataPerInternalFunction->needToBeFixed(dyn_cast<CallInst>(pRetVal)) ) ) {
        //Calculate the pointer of the current special in the special buffer
        Value *Idxs[1];
        Value *pLoadedCurrSB = new LoadInst(m_pCurrSBValue, "loadedCurrSB", pNextInst);
        Idxs[0] = BinaryOperator::CreateNUWAdd(pLoadedCurrSB,
          pOffsetArg, "&(pSB[currWI].offset)", pNextInst);
        Value *pAddrInSpecialBuffer = GetElementPtrInst::CreateInBounds(
          m_pSpecialBufferValue, Idxs, Idxs+1, "&pSB[currWI].offset", pNextInst);
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
      pNewFunc, params.begin(), params.end(), "", pOriginalCall);
    pNewCall->setCallingConv(pOriginalCall->getCallingConv());

    pOriginalCall->uncheckedReplaceAllUsesWith(pNewCall);
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

  //Register this pass...
  static RegisterPass<Barrier> BP("b-p",
    "Handle special values & replace barrier/fiber with internal loop over WIs",
    false, true);

} // namespace intel


/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createBarrierPass() {
    return new intel::Barrier();
  }

  unsigned int getBarrierPassStrideSize(Pass *pPass) {
    return ((intel::Barrier*)pPass)->getStrideSize();
  }
}