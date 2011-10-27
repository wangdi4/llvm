/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/

#include "DataPerValuePass.h"
#include "BarrierUtils.h"

#include "llvm/Instructions.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/raw_ostream.h"

namespace intel {
  char DataPerValue::ID = 0;

  DataPerValue::DataPerValue() : ModulePass(ID), m_pTD(0) {}

  bool DataPerValue::runOnModule(Module &M) {
    //Get Analysis data
    m_pDataPerBarrier = &getAnalysis<DataPerBarrier>();
    m_pWIRelatedValue = &getAnalysis<WIRelatedValue>();

    m_currentOffset = 0;
    m_maxAlignment = 0;
    //Create instance of TargetData for this module
    m_pTD = new TargetData(&M);
    assert( m_pTD && "Failed to create new instance of TargetData!" );

    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
      runOnFunction(*fi);
    }
    //Delete the instance of TargetData
    delete m_pTD;

    //Check that stide size is aligned with max alignment
    if ( m_maxAlignment != 0 && (m_currentOffset % m_maxAlignment) != 0 ) {
      m_currentOffset = (m_currentOffset + m_maxAlignment) & (~(m_maxAlignment-1));
    }
    m_bufferTotalSize = m_currentOffset;
    return false;
  }

  bool DataPerValue::runOnFunction(Function &F) {
    m_pSyncInstructions = &m_pDataPerBarrier->getSyncInstructions(&F);

    //Check if function has no synchronize instruction!
    if ( !m_pDataPerBarrier->hasSyncInstruction(&F) ) {
      //Function has no synchronize instruction: do nothing!
      return false;
    }

    //run over all the values of the function and Cluster into 3 groups
    //Group-A   : Alloca instructions
    //Group-B.1 : Values crossed barriers and the value is
    //            related to WI-Id or initialized inside a loop
    //Group-B.2 : Value crossed barrier but does not suit Group-B.2
    for ( inst_iterator ii = inst_begin(F), ie = inst_end(F); ii != ie; ++ii ) {
      Instruction *pInst = dyn_cast<Instruction>(&*ii);
      if ( isa<AllocaInst>(pInst) ) {
        //It is an alloca value, add it to Group_A container
        m_allocaValuesPerFuncMap[&F].push_back(pInst);
        continue;
      }
      switch ( isSpecialValue(pInst, m_pWIRelatedValue->isWIRelated(pInst)) ) {
        case SPECIAL_VALUE_TYPE_B1:
          //It is an special value, add it to special value container
          m_specialValuesPerFuncMap[&F].push_back(pInst);
          break;
        case SPECIAL_VALUE_TYPE_B2:
          //It is an uniform value that usage cross a barrier
          //add it to cross barrier value container
          m_crossBarrierValuesPerFuncMap[&F].push_back(pInst);
          break;
        case SPECIAL_VALUE_TYPE_NONE:
          //No need to handle this value
          break;
        default:
          assert( false && "Unknown special value type!" );
      }
    }

    calculateOffsets(F);

    return false;
  }

  DataPerValue::SPECIAL_VALUE_TYPE DataPerValue::isSpecialValue(Value *pVal, bool isWIRelated) {
    //Value "v" is special (cross barrier) if there is
    //one barrier instruction "i" and one value usage "u" such that:
    //BB(v) in BB(i)->predecessors and BB(u) in B(i)->successors
    Instruction *pInst = dyn_cast<Instruction>(pVal);
    assert( pInst && "trying check if non-instruction is a special value!" );
    BasicBlock *pValBB = pInst->getParent();

    //Value that is not dependent on WI-Id and initialized outside a loop
    //can not be in Group-B.1. If it cross a barrier it will be in Group-B.2
    bool isNotGroupB1Type = !isWIRelated && 
      !m_pDataPerBarrier->getPredecessors(pValBB).count(pValBB);

    //By default we assume value is not special till prove otherwise
    SPECIAL_VALUE_TYPE retType = SPECIAL_VALUE_TYPE_NONE;

    //Run over all usages of pVal and check if one crosses a barrier
    for ( Value::use_iterator ui = pVal->use_begin(), ue = pVal->use_end(); ui != ue; ++ui ) {
      Instruction *pInstUsage = dyn_cast<Instruction>(*ui);
      assert( pInstUsage && "usage of pVal is non-instruction!" );
      BasicBlock *pValUsageBB = pInstUsage->getParent();

      if ( pValUsageBB == pValBB && !isa<PHINode>(pInstUsage) ) {
        //pVal and pInstUsage has same basic block and pVal apears before pInstUsage
        //Sync instruction exists only at begin of basic block, thus these values
        //are not crossed by sync instruction, check next usage of pVal
        continue;
      }

      if ( isa<ReturnInst>(pInstUsage) ) {
        //Return value is saved on special buffer by 
        //BarrierPass::fixNonInlinedInternalFunction
        //should not consider it special value at this point!
        continue;
      }

      //Run over all sync instructions
      TInstructionSet::iterator ii = m_pSyncInstructions->begin();
      TInstructionSet::iterator ie = m_pSyncInstructions->end();
      for ( ; ii != ie; ++ii ) {
        Instruction *pSyncInst = dyn_cast<Instruction>(*ii);
        BasicBlock *pSyncBB = pSyncInst->getParent();
        if ( pSyncBB->getParent() != pValBB->getParent() ) {
          assert(false && "can we reach sync instructions from other functions?!" );
          //This sync instructions is from another function
          continue;
        }
        if ( m_pDataPerBarrier->getPredecessors(pSyncBB).count(pValBB) &&
          m_pDataPerBarrier->getSuccessors(pSyncBB).count(pValUsageBB) ) {
            //Found value usage "u" and barrier "i" such that
            //BB(v) in BB(i)->predecessors and BB(u) in B(i)->successors

            if ( isWIRelated ) {
              //pVal depends on work item id and crosses a barrier
              return SPECIAL_VALUE_TYPE_B1;
            }

            if( m_pDataPerBarrier->getPredecessors(pSyncBB).count(pSyncBB) ) {
              //pSyncBB is a predecessor of itself
              //means synchronize instruction is inside a loop

              BasicBlock *pPrevBB = BarrierUtils::findBasicBlockOfUsageInst(pInst, pInstUsage);
              if ( isCrossedByBarrier(pPrevBB, pValBB) ) {
                //pVal does not depend on work item id but it is crossed by loop barrier
                return ( isNotGroupB1Type )? SPECIAL_VALUE_TYPE_B2 : SPECIAL_VALUE_TYPE_B1;
              }
              //Current usage of pVal are not crossed by barrier,
              //skip other barriers and check the next usage of pVal
              break;
            }
            //pVal does not depend on work item id and it is crossed by a non loop barrier
            //Upgrade retType to be special value of group-B.2
            if ( isNotGroupB1Type ) {
              //We can return at this point as we know that isNotGroupB1Type == true!
              return SPECIAL_VALUE_TYPE_B2;
            }
            //But still need to check if it cross other loop barriers
            retType = SPECIAL_VALUE_TYPE_B2;
        }
        assert( ! (m_pDataPerBarrier->getPredecessors(pSyncBB).count(pValUsageBB) &&
          m_pDataPerBarrier->getSuccessors(pSyncBB).count(pValBB) ) &&
          "can usage come before value?! (Handle such case)" );
      }
    }
    return retType;
  }

  bool DataPerValue::isCrossedByBarrier(BasicBlock *pValUsageBB, BasicBlock *pValBB) {
    DataPerBarrier::TBasicBlocksSet perdecessors;
    std::vector<BasicBlock*> basicBlocksToHandle;

    perdecessors.clear();
    basicBlocksToHandle.push_back(pValUsageBB);

    while ( !basicBlocksToHandle.empty() ) {
      BasicBlock *pBBToHandle = basicBlocksToHandle.back();
      basicBlocksToHandle.pop_back();
      Instruction *pFirstInst = dyn_cast<Instruction>(pBBToHandle->begin());
      if ( m_pSyncInstructions->count(pFirstInst) ) {
        //Found a barrier
        return true;
      }
      for (pred_iterator i = pred_begin(pBBToHandle), e = pred_end(pBBToHandle); i != e; ++i) {
        BasicBlock *pred_bb = dyn_cast<BasicBlock>(*i);
        if ( pred_bb == pValBB ) {
          //Reached pValBB stop recursive at this direction!
          continue;
        }
        if ( perdecessors.count(pred_bb) ) {
          //pred_bb was already added to predecessors
          continue;
        }
        //This is a new predecessor add it to the predecessors container
        perdecessors.insert(pred_bb);
        //Also add it to the basicBlocksToHandle to calculate its predecessors
        basicBlocksToHandle.push_back(pred_bb);
      }
    }
    return false;
  }

  void DataPerValue::calculateOffsets(Function &F) {

    TValueVector &specialValues = m_specialValuesPerFuncMap[&F];

    //Run over all special values in function
    for ( TValueVector::iterator vi = specialValues.begin(),
      ve = specialValues.end(); vi != ve; ++vi ) {
        Value *pVal = dyn_cast<Value>(*vi);
        //Get Offset of special value type
        m_valueToOffsetMap[pVal] = getValueOffset(pVal->getType(), 0);
    }

    TValueVector &allocaValues = m_allocaValuesPerFuncMap[&F];

    //Run over all special values in function
    for ( TValueVector::iterator vi = allocaValues.begin(),
      ve = allocaValues.end(); vi != ve; ++vi ) {
        Value *pVal = dyn_cast<Value>(*vi);
        AllocaInst *pAllocaInst = dyn_cast<AllocaInst>(pVal);
        //Get Offset of alloca instruction contained type
        m_valueToOffsetMap[pVal] = getValueOffset(
          pVal->getType()->getContainedType(0), pAllocaInst->getAlignment());
    }
  }

  unsigned int DataPerValue::getValueOffset(const Type *pType, unsigned int allocaAlignment) {
    //TODO: check what is better to use for alignment?
    //unsigned int alignment = m_pTD->getABITypeAlignment(pType);
    unsigned int alignment = (allocaAlignment) ?
      allocaAlignment : m_pTD->getPrefTypeAlignment(pType);
    assert( alignment && "alignment is 0" );
    unsigned int sizeInBits = m_pTD->getTypeSizeInBits(pType);
    if ( sizeInBits == 1 ) {
      //We allocate one whole byte when the value size is a single bit
      sizeInBits = 8;
    }
    unsigned int sizeInBytes = sizeInBits / 8;
    assert( sizeInBytes && "sizeInBytes is 0" );

    //Update max alignment
    if ( alignment > m_maxAlignment ) {
      m_maxAlignment = alignment;
    }

    if ( (m_currentOffset % alignment) != 0 ) {
      //Offset is not aligned on value size
      assert( ((alignment & (alignment-1)) == 0) && "alignment is not power of 2!" );
      //TODO: check what to do with the following assert - it fails on
      //      test_basic.exe kernel_memory_alignment_private
      //assert( (alignment <= 32) && "alignment is bigger than 32 bytes (should we align to more than 32 bytes?)" );
      m_currentOffset = (m_currentOffset + alignment) & (~(alignment-1));
    }
    assert( (m_currentOffset % alignment) == 0 && "Offset is not aligned on value size!" );
    //Found offset of given type
    unsigned int offset = m_currentOffset;
    //Increment current available offset with pVal size
    m_currentOffset += sizeInBytes;

    return offset;
  }

  void DataPerValue::print(raw_ostream &OS, const Module *M) const {
    if ( !M ) {
      OS << "No Module!\n";
      return;
    }
    //Print Module
    OS << *M;

    //Run on all alloca values
    OS << "\nGroup-A Values\n";
    TValuesPerFunctionMap::const_iterator fi = m_allocaValuesPerFuncMap.begin();
    TValuesPerFunctionMap::const_iterator fe = m_allocaValuesPerFuncMap.end();
    for ( ; fi != fe; ++fi ) {
      Function *pFunc = dyn_cast<Function>(fi->first);
      //Print function name
      OS << pFunc->getNameStr() << "\n";
      const TValueVector &vv = fi->second;
      for ( TValueVector::const_iterator vi = vv.begin(), ve = vv.end();  vi != ve; ++vi ) {
        Value *pValue = dyn_cast<Value>(*vi);
        //Print alloca value name
        OS  << "\t" << pValue->getNameStr() << "\n";
      }
    }

    //Run on all special values
    OS << "\nGroup-B.1 Values\n";
    fi = m_specialValuesPerFuncMap.begin();
    fe = m_specialValuesPerFuncMap.end();
    for ( ; fi != fe; ++fi ) {
      Function *pFunc = dyn_cast<Function>(fi->first);
      //Print function name
      OS << pFunc->getNameStr() << "\n";
      const TValueVector &vv = fi->second;
      for ( TValueVector::const_iterator vi = vv.begin(), ve = vv.end();  vi != ve; ++vi ) {
        Value *pValue = dyn_cast<Value>(*vi);
        //Print special value name
        OS  << "\t" << pValue->getNameStr() << "\n";
      }
    }

    //Run on all cross barrier unifrom values
    OS << "\nGroup-B.2 Values\n";
    fi = m_crossBarrierValuesPerFuncMap.begin();
    fe = m_crossBarrierValuesPerFuncMap.end();
    for ( ; fi != fe; ++fi ) {
      Function *pFunc = dyn_cast<Function>(fi->first);
      //Print function name
      OS << pFunc->getNameStr() << "\n";
      const TValueVector &vv = fi->second;
      for ( TValueVector::const_iterator vi = vv.begin(), ve = vv.end();  vi != ve; ++vi ) {
        Value *pValue = dyn_cast<Value>(*vi);
        //Print cross barrier uniform value name
        OS  << "\t" << pValue->getNameStr() << "\n";
      }
    }
  }

  //Register this pass...
  static RegisterPass<DataPerValue> DPV("d-p-v",
    "Collect Data per Value", false, true);


} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createDataPerValuePass() {
    return new intel::DataPerValue();
  }
}