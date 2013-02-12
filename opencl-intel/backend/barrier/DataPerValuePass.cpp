/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "DataPerValuePass.h"
#include "BarrierUtils.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "llvm/Instructions.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/raw_ostream.h"

namespace intel {
  char DataPerValue::ID = 0;

  OCL_INITIALIZE_PASS_BEGIN(DataPerValue, "B-ValueAnalysis", "Barrier Pass - Collect Data per Value", false, true)
  OCL_INITIALIZE_PASS_DEPENDENCY(DataPerBarrier)
  OCL_INITIALIZE_PASS_DEPENDENCY(WIRelatedValue)
  OCL_INITIALIZE_PASS_END(DataPerValue, "B-ValueAnalysis", "Barrier Pass - Collect Data per Value", false, true)

  DataPerValue::DataPerValue()
    : ModulePass(ID), m_pTD(0)
  {
    initializeDataPerValuePass(*llvm::PassRegistry::getPassRegistry());
  }

  bool DataPerValue::runOnModule(Module &M) {
    //Get Analysis data
    m_pDataPerBarrier = &getAnalysis<DataPerBarrier>();
    m_pWIRelatedValue = &getAnalysis<WIRelatedValue>();

    //Initialize barrier utils class with current module
    m_util.init(&M);

    // obtain TagetData of the module
    m_pTD = getAnalysisIfAvailable<TargetData>();
    assert( m_pTD && "Failed to obtain instance of TargetData!" );

    // Find and sort all connected function into disjointed groups
    CalculateConnectedGraph(M);

    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
      runOnFunction(*fi);
    }

    //Check that stide size is aligned with max alignment
    for ( TEntryToBufferDataMap::iterator di = m_entryToBufferDataMap.begin(),
      de = m_entryToBufferDataMap.end(); di != de; ++di ) {
        unsigned int maxAlignment = di->second.m_maxAlignment;
        unsigned int currentOffset = di->second.m_currentOffset;
      if ( maxAlignment != 0 && (currentOffset % maxAlignment) != 0 ) {
        currentOffset = (currentOffset + maxAlignment) & (~(maxAlignment-1));
      }
      di->second.m_bufferTotalSize = currentOffset;
    }
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
      CallInst *pCallInst = dyn_cast<CallInst>(pInst);
      if ( pCallInst ) {
        Function *pCalledFunc = pCallInst->getCalledFunction();
        if ( pCalledFunc && !pCalledFunc->getReturnType()->isVoidTy() &&
          m_pDataPerBarrier->hasSyncInstruction(pCalledFunc) ) {
            // Call instructions to functions that contains barriers
            // need to be stored in the special buffer.
            assert(m_pWIRelatedValue->isWIRelated(pInst)
              && "Must be work-item realted value!");
            m_specialValuesPerFuncMap[&F].push_back(pInst);
            continue;
        }
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

            if ( isWIRelated && !m_pDataPerBarrier->getPredecessors(pSyncBB).count(pSyncBB) ) {
              //pVal depends on work item id and crosses a barrier that is not in a loop
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
    if ( pValUsageBB == pValBB ) {
      //This can happen when pValUsage is a PHINode
      return false;
    }

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
    unsigned int entry = m_functionToEntryMap[&F];
    SpecialBufferData& bufferData = m_entryToBufferDataMap[entry];

    //Run over all special values in function
    for ( TValueVector::iterator vi = specialValues.begin(),
      ve = specialValues.end(); vi != ve; ++vi ) {
        Value *pVal = dyn_cast<Value>(*vi);
        //Get Offset of special value type
        m_valueToOffsetMap[pVal] = getValueOffset(pVal, pVal->getType(), 0, bufferData);
    }

    TValueVector &allocaValues = m_allocaValuesPerFuncMap[&F];

    //Run over all special values in function
    for ( TValueVector::iterator vi = allocaValues.begin(),
      ve = allocaValues.end(); vi != ve; ++vi ) {
        Value *pVal = dyn_cast<Value>(*vi);
        AllocaInst *pAllocaInst = dyn_cast<AllocaInst>(pVal);
        //Get Offset of alloca instruction contained type
        m_valueToOffsetMap[pVal] = getValueOffset(pVal,
          pVal->getType()->getContainedType(0), pAllocaInst->getAlignment(), bufferData);
    }
  }

  unsigned int DataPerValue::getValueOffset(
    Value* pVal, Type *pType, unsigned int allocaAlignment, SpecialBufferData& bufferData) {

    unsigned int sizeFactor = 1;
    unsigned int alignmentFactor = 1;
    Type *pElementType = pType;
    VectorType* pVecType = dyn_cast<VectorType>(pType);
    if ( pVecType ) {
      pElementType = pVecType->getElementType();
    }
    assert(!isa<VectorType>(pElementType) && "element type of a vector is another vector!");
    if ( m_pTD->getTypeSizeInBits(pElementType) == 1 ) {
      //we have a Value with base type i1
      m_oneBitElementValues.insert(pVal);
      //We will extend i1 to i32 before storing to special buffer.
      sizeFactor = 32; // In bits
      alignmentFactor = 4; // In bytes
      assert(m_pTD->getPrefTypeAlignment(pType) ==
        (pVecType ? pVecType->getNumElements() : 1) &&
        "assumes alignment of vector of i1 type equals to vector length");
    }
    //TODO: check what is better to use for alignment?
    //unsigned int alignment = m_pTD->getABITypeAlignment(pType);
    unsigned int alignment = ((allocaAlignment) ?
      allocaAlignment : m_pTD->getPrefTypeAlignment(pType)) * alignmentFactor;
    assert( alignment && "alignment is 0" );
    unsigned int sizeInBits = m_pTD->getTypeSizeInBits(pType) * sizeFactor;

    unsigned int sizeInBytes = sizeInBits / 8;
    assert( sizeInBytes && "sizeInBytes is 0" );

    //Update max alignment
    if ( alignment > bufferData.m_maxAlignment ) {
      bufferData.m_maxAlignment = alignment;
    }

    if ( (bufferData.m_currentOffset % alignment) != 0 ) {
      //Offset is not aligned on value size
      assert( ((alignment & (alignment-1)) == 0) && "alignment is not power of 2!" );
      //TODO: check what to do with the following assert - it fails on
      //      test_basic.exe kernel_memory_alignment_private
      //assert( (alignment <= 32) && "alignment is bigger than 32 bytes (should we align to more than 32 bytes?)" );
      bufferData.m_currentOffset = (bufferData.m_currentOffset + alignment) & (~(alignment-1));
    }
    assert( (bufferData.m_currentOffset % alignment) == 0 && "Offset is not aligned on value size!" );
    //Found offset of given type
    unsigned int offset = bufferData.m_currentOffset;
    //Increment current available offset with pVal size
    bufferData.m_currentOffset += sizeInBytes;

    return offset;
  }

  void DataPerValue::CalculateConnectedGraph(Module &M) {
    unsigned int currEntry = 0;

    //Run on all functions in module
    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
      Function* pFunc = dyn_cast<Function>(fi);
      if ( pFunc->isDeclaration() ) {
        //Skip non defined functions
        continue;
      }
      if ( m_functionToEntryMap.count(pFunc) ) {
        //pFunc already has an entry number,
        //replace all appears of it with the current entry number.
        FixEntryMap(m_functionToEntryMap[pFunc], currEntry);
      } else {
        //pFunc has no entry number yet, give it the current entry number
        m_functionToEntryMap[pFunc] = currEntry;
      }
      for ( Value::use_iterator ui = pFunc->use_begin(),
        ue = pFunc->use_end(); ui != ue; ++ui ) {
          CallInst *pCallInst = dyn_cast<CallInst>(*ui);
          // usage of pFunc can be a global variable!
          if( !pCallInst ) {
            // usage of pFunc is not a CallInst
            continue;
          }
          Function *pCallerFunc = pCallInst->getParent()->getParent();
          if ( m_functionToEntryMap.count(pCallerFunc) ) {
            //pCallerFunc already has an entry number,
            //replace all appears of it with the current entry number.
            FixEntryMap(m_functionToEntryMap[pCallerFunc], currEntry);
          } else {
            //pCallerFunc has no entry number yet, give it the current entry number
            m_functionToEntryMap[pCallerFunc] = currEntry;
          }
      }
      currEntry++;
    }
  }

  void DataPerValue::FixEntryMap(unsigned int from, unsigned int to) {
    if ( from == to ) {
      //No need to fix anything
      return;
    }
    //Replace all apears of value "from" with value "to"
    for ( TFunctionToEntryMap::iterator ei = m_functionToEntryMap.begin(),
      ee = m_functionToEntryMap.end(); ei != ee; ++ ei ) {
        if ( ei->second == from ) {
          ei->second = to;
        }
    }
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
      const TValueVector &vv = fi->second;
      if ( vv.empty() ) {
        // Function has no values of Group-A
        continue;
      }
      //Print function name
      OS << "+" << pFunc->getName() << "\n";
      for ( TValueVector::const_iterator vi = vv.begin(), ve = vv.end();  vi != ve; ++vi ) {
        Value *pValue = dyn_cast<Value>(*vi);
        //Print alloca value name
        OS << "\t-" << pValue->getName() << "\t(" << m_valueToOffsetMap.find(pValue)->second << ")\n";
      }
      OS << "*" << "\n";
    }

    //Run on all special values
    OS << "\nGroup-B.1 Values\n";
    fi = m_specialValuesPerFuncMap.begin();
    fe = m_specialValuesPerFuncMap.end();
    for ( ; fi != fe; ++fi ) {
      Function *pFunc = dyn_cast<Function>(fi->first);
      const TValueVector &vv = fi->second;
      if ( vv.empty() ) {
        // Function has no values of Group-B.1
        continue;
      }
      //Print function name
      OS << "+" << pFunc->getName() << "\n";
      for ( TValueVector::const_iterator vi = vv.begin(), ve = vv.end();  vi != ve; ++vi ) {
        Value *pValue = dyn_cast<Value>(*vi);
        //Print special value name
        OS << "\t-" << pValue->getName() << "\t(" << m_valueToOffsetMap.find(pValue)->second << ")\n";
      }
      OS << "*" << "\n";
    }

    //Run on all cross barrier unifrom values
    OS << "\nGroup-B.2 Values\n";
    fi = m_crossBarrierValuesPerFuncMap.begin();
    fe = m_crossBarrierValuesPerFuncMap.end();
    for ( ; fi != fe; ++fi ) {
      Function *pFunc = dyn_cast<Function>(fi->first);
      const TValueVector &vv = fi->second;
      if ( vv.empty() ) {
        // Function has no values of Group-B.2
        continue;
      }
      //Print function name
      OS << "+" << pFunc->getName() << "\n";
      for ( TValueVector::const_iterator vi = vv.begin(), ve = vv.end();  vi != ve; ++vi ) {
        Value *pValue = dyn_cast<Value>(*vi);
        //Print cross barrier uniform value name
        OS << "\t-" << pValue->getName() << "\n";
      }
      OS << "*" << "\n";
    }

    OS << "Buffer Total Size: ";
    for ( TFunctionToEntryMap::const_iterator ei = m_functionToEntryMap.begin(),
      ee = m_functionToEntryMap.end(); ei != ee; ++ei ) {
        //Print function name & its entry
        OS << "+" << ei->first->getName() << " : [" << ei->second << "]\n";
    }
    for ( TEntryToBufferDataMap::const_iterator di = m_entryToBufferDataMap.begin(),
      de = m_entryToBufferDataMap.end(); di != de; ++di ) {
      //Print entry & its structure stride
      OS << "entry(" << di->first << ") : (" << di->second.m_bufferTotalSize << ")\n";
    }

    OS << "DONE\n";
  }


} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createDataPerValuePass() {
    return new intel::DataPerValue();
  }
}