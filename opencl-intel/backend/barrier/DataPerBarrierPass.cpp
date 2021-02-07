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

#include "DataPerBarrierPass.h"
#include "OCLPassSupport.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/raw_ostream.h"

namespace intel {
  char DataPerBarrier::ID = 0;

  OCL_INITIALIZE_PASS(DataPerBarrier, "B-BarrierAnalysis", "Barrier Pass - Collect Data per Barrier", false, true)

  DataPerBarrier::DataPerBarrier() : ModulePass(ID) {}

  void DataPerBarrier::InitSynchronizeData() {
    //Internal Data used to calculate user Analysis Data
    unsigned int currentAvailableID = 0;

    //Find all synchronize instructions
    TInstructionVector &syncInstructions =
        m_util.getAllSynchronizeInstructions();

    for (TInstructionVector::iterator ii = syncInstructions.begin(),
      ie = syncInstructions.end(); ii != ie; ++ii ) {

        Instruction *pInst = dyn_cast<Instruction>(*ii);
        assert( pInst && "Sync list contains something other than instruction!" );

        SYNC_TYPE type = m_util.getSynchronizeType(pInst);
        assert( SYNC_TYPE_NONE != type && "Sync list contains non sync instruction!" );

        //Save id and type of current sync instruction
        m_dataPerBarrierMap[pInst].m_id = currentAvailableID++;
        m_dataPerBarrierMap[pInst].m_type = type;

        Function *pFunc = pInst->getParent()->getParent();
        //Classify the sync instruction according to its parent function
        m_syncsPerFuncMap[pFunc].insert(pInst);
    }
  }

  bool DataPerBarrier::runOnFunction(Function &F) {
    if ( m_syncsPerFuncMap.count(&F) == 0 ) {
      //Function has no barrir/dummy barrier instruction, simply return.
      return false;
    }

    //Calculate predecessors for all basic blocks
    for ( Function::iterator bi = F.begin(), be = F.end(); bi != be; ++bi ) {
      BasicBlock *pBB = &*bi;
      FindPredecessors(pBB);
    }

    //Calculate successors and sync predecessors only for sync basic blocks
    TInstructionSet::iterator i = m_syncsPerFuncMap[&F].begin();
    TInstructionSet::iterator e = m_syncsPerFuncMap[&F].end();
    for ( ; i != e; ++i ) {
      Instruction *pSyncInst = *i;
      BasicBlock *pBB = pSyncInst->getParent();
      FindSuccessors(pBB);
      FindBarrierPredecessors(pSyncInst);
    }
    return false;
  }

  void DataPerBarrier::FindPredecessors(BasicBlock *pBB) {
    TBasicBlockSet &predecessors = m_predecessorsMap[pBB];
    std::vector<BasicBlock*> basicBlocksToHandle;

    predecessors.clear();
    basicBlocksToHandle.push_back(pBB);

    while ( !basicBlocksToHandle.empty() ) {
      BasicBlock *pBBToHandle = basicBlocksToHandle.back();
      basicBlocksToHandle.pop_back();
      for (pred_iterator i = pred_begin(pBBToHandle), e = pred_end(pBBToHandle); i != e; ++i) {
        BasicBlock *pred_bb = *i;
        if ( predecessors.count(pred_bb) ) {
          //pred_bb was already added to predecessors
          continue;
        }
        //This is a new predecessor add it to the predecessors container
        predecessors.insert(pred_bb);
        //Also add it to the basicBlocksToHandle to calculate its predecessors
        basicBlocksToHandle.push_back(pred_bb);
      }
    }
  }

  void DataPerBarrier::FindSuccessors(BasicBlock *pBB) {
    TBasicBlockSet &successors = m_successorsMap[pBB];
    std::vector<BasicBlock*> basicBlocksToHandle;

    successors.clear();
    basicBlocksToHandle.push_back(pBB);
    //Barrier basic block is always a successor basic block of itself!
    successors.insert(pBB);

    while ( !basicBlocksToHandle.empty() ) {
      BasicBlock *pBBToHandle = basicBlocksToHandle.back();
      basicBlocksToHandle.pop_back();
      for (succ_iterator i = succ_begin(pBBToHandle), e = succ_end(pBBToHandle); i != e; ++i) {
        BasicBlock *succ_bb = *i;
        if ( successors.count(succ_bb) ) {
          //pred_bb was already added to successors
          continue;
        }
        //This is a new successor add it to the successors container
        successors.insert(succ_bb);
        //Also add it to the basicBlocksToHandle to calculate its successors
        basicBlocksToHandle.push_back(succ_bb);
      }
    }
  }

  void DataPerBarrier::FindBarrierPredecessors(Instruction *pInst) {
    BasicBlock *pBB = pInst->getParent();
    Function *pFunc = pBB->getParent();
    TInstructionSet &barrierBBSet = m_syncsPerFuncMap[pFunc];
    SBarrierRelated &barrierRelated = m_barrierPredecessorsMap[pInst];
    TInstructionSet &barrierPerdecessors = barrierRelated.m_relatedBarriers;
    std::vector<BasicBlock*> basicBlocksToHandle;
    TBasicBlockSet basicBlocksAddedForHandle;

    barrierPerdecessors.clear();
    basicBlocksToHandle.push_back(pBB);

    while ( !basicBlocksToHandle.empty() ) {
      BasicBlock *pBBToHandle = basicBlocksToHandle.back();
      basicBlocksToHandle.pop_back();
      for (pred_iterator i = pred_begin(pBBToHandle), e = pred_end(pBBToHandle); i != e; ++i) {
        BasicBlock *pred_bb = *i;
        if ( basicBlocksAddedForHandle.count(pred_bb) ) {
          //pred_bb was already handled
          continue;
        }
        //This is a new predecessor
        basicBlocksAddedForHandle.insert(pred_bb);
        Instruction *pInst = &*(pred_bb->begin());
        if ( barrierBBSet.count(pInst) ) {
          //This predecessor basic block conatins a barrier
          barrierPerdecessors.insert(pInst);
        } else {
          //Add it to the basicBlocksToHandle to calculate its predecessors
          basicBlocksToHandle.push_back(pred_bb);
        }
      }
    }
  }

  void DataPerBarrier::print(raw_ostream &OS, const Module *M) const {
    if ( !M ) {
      OS << "No Module!\n";
      return;
    }
    //Print Module
    OS << *M;

    //Run on all barrier basic blocks
    OS << "\nsynchronize basic blocks\n";
    TInstructionSetPerFunctionMap::const_iterator fi = m_syncsPerFuncMap.begin();
    TInstructionSetPerFunctionMap::const_iterator fe = m_syncsPerFuncMap.end();
    for ( ; fi != fe; ++fi ) {
      Function *pFunc = fi->first;
      //Print function name
      OS << "+" <<pFunc->getName() << "\n";
      const TInstructionSet &iiSet = fi->second;
      for ( TInstructionSet::const_iterator ii = iiSet.begin(), ie = iiSet.end();  ii != ie; ++ii ) {
        Instruction *pSyncInst = *ii;
        BasicBlock *pBB = pSyncInst->getParent();
        //Print basic block name
        OS << "\t-" << pBB->getName() << "\n";
      }
      OS << "*" << "\n";
    }

    //Run on all Predecessors
    OS << "\nbasic blocks predecessors\n";
    TBasicBlock2BasicBlocksSetMap::const_iterator bbi = m_predecessorsMap.begin();
    TBasicBlock2BasicBlocksSetMap::const_iterator bbe = m_predecessorsMap.end();
    for ( ; bbi != bbe; ++bbi ) {
      BasicBlock *pBBB = bbi->first;
      //Print barrier basic block name
      OS << "+" << pBBB->getName() << "\n";
      const TBasicBlockSet &bbSet = bbi->second;
      for ( TBasicBlockSet::const_iterator bi = bbSet.begin(), be = bbSet.end();  bi != be; ++bi ) {
        BasicBlock *pBB = *bi;
        //Print predecessor basic block name
        OS << "\t-" << pBB->getName() << "\n";
      }
      OS << "*" << "\n";
    }

    //Run on all Successors
    OS << "\nsynchronize basic blocks successors\n";
    bbi = m_successorsMap.begin();
    bbe = m_successorsMap.end();
    for ( ; bbi != bbe; ++bbi ) {
      BasicBlock *pBBB = bbi->first;
      //Print barrier basic block name
      OS<< "+" << pBBB->getName() << "\n";
      const TBasicBlockSet &bbSet = bbi->second;
      for ( TBasicBlockSet::const_iterator bi = bbSet.begin(), be = bbSet.end();  bi != be; ++bi ) {
        BasicBlock *pBB = *bi;
        //Print successor basic block name
        OS << "\t-" << pBB->getName() << "\n";
      }
      OS << "*" << "\n";
    }

    //Run on all Barrier Predecessors
    OS << "\nsynchronize basic blocks barrier predecessors\n";
    TBarrier2BarriersSetMap::const_iterator iii = m_barrierPredecessorsMap.begin();
    TBarrier2BarriersSetMap::const_iterator iie = m_barrierPredecessorsMap.end();
    for ( ; iii != iie; ++iii ) {
      Instruction *pInst = iii->first;
      BasicBlock *pBBB = pInst->getParent();
      //Print barrier basic block name
      OS << "+" << pBBB->getName() << "\n";
      const TInstructionSet &iiVec = iii->second.m_relatedBarriers;
      for ( TInstructionSet::const_iterator ii = iiVec.begin(), ie = iiVec.end();  ii != ie; ++ii ) {
        Instruction *pInstPred = *ii;
        BasicBlock *pBB = pInstPred->getParent();
        //Print barrier predecessor basic block name
        OS << "\t-" << pBB->getName() << "\n";
      }
      OS << "*" << "\n";
    }

    OS << "DONE\n\n";
  }


} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createDataPerBarrierPass() {
    return new intel::DataPerBarrier();
  }
}
