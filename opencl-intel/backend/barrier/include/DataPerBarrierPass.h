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

#ifndef __DATA_PER_BARRIER_PASS_H__
#define __DATA_PER_BARRIER_PASS_H__

#include "BarrierUtils.h"

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"

// Forward declarations
namespace llvm {
  class Function;
  class BasicBlock;
  class Instruction;
}

using namespace llvm;

namespace intel {

  /// @brief DataPerBarrier pass is a analysis module pass used to collect
  /// data on barrier/dummy barrier instructions
  class DataPerBarrier : public ModulePass {
  public:
    typedef struct  {
      TInstructionSet    m_relatedBarriers;
    } SBarrierRelated;
    typedef struct {
      unsigned int m_id;
      SYNC_TYPE    m_type;
    } SBarrierData;
    typedef MapVector<BasicBlock*, TBasicBlockSet> TBasicBlock2BasicBlocksSetMap;
    typedef MapVector<Function*, TInstructionSet> TInstructionSetPerFunctionMap;
    typedef MapVector<Instruction*, SBarrierRelated> TBarrier2BarriersSetMap;
    typedef MapVector<Instruction*, SBarrierData> TDataPerBarrierMap;

  public:
    static char ID;

    /// @brief C'tor
    DataPerBarrier();

    /// @brief D'tor
    ~DataPerBarrier() {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const override {
      return "Intel OpenCL DataPerBarrier";
    }

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    virtual bool runOnModule(Module &M) override {
      //Initialize barrier utils class with current module
      m_util.init(&M);
      InitSynchronizeData();
      for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
        runOnFunction(*fi);
      }
      return false;
    }

    /// @brief Inform about usage/mofication/dependency of this pass
    virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
      // Analysis pass preserve all
      AU.setPreservesAll();
    }

    /// @brief print data collected by the pass on the given module
    /// @param OS stream to print the info regarding the module into
    /// @param M pointer to the Module
    void print(raw_ostream &OS, const Module *M = 0) const override;

    /// @brief return sync instruction calls of given function
    /// @param pFunc pointer to Function
    /// @returns container of all sync instructions call in pFunc
    TInstructionSet& getSyncInstructions(Function *pFunc) {
      assert(m_syncsPerFuncMap.count(pFunc) && "Function has no sync data!");
      return m_syncsPerFuncMap[pFunc];
    }

    /// @brief return unique id for given sync instruction
    /// @param pInst pointer to sync instruction
    /// @returns unique id for given sync instruction
    unsigned int getUniqueID(Instruction *pInst) {
      assert( m_dataPerBarrierMap.count(pInst)
        && "instruction has no sync data!" );
      return m_dataPerBarrierMap[pInst].m_id;
    }

    /// @brief return type of given sync instruction
    /// @param pInst pointer to sync instruction
    /// @returns unique id for given sync instruction
    SYNC_TYPE getSyncType(Instruction *pInst) {
      assert( m_dataPerBarrierMap.count(pInst)
        && "instruction has no sync data!" );
      return m_dataPerBarrierMap[pInst].m_type;
    }

    /// @brief Check if predecessors of given basic block is available.
    /// @return true if available, false otherwise.
    bool hasPredecessors(BasicBlock *pBB) {
      return m_predecessorsMap.count(pBB);
    }

    /// @brief return Predecessors of given basic block
    /// @param pBB pointer to basic block
    /// @returns basic blocks set of Predecessors
    TBasicBlockSet& getPredecessors(BasicBlock *pBB) {
      assert( m_predecessorsMap.count(pBB)
        && "basic block has no predecessor data!" );
      return m_predecessorsMap[pBB];
    }

    /// @brief return Successors of given basic block
    /// @param pBB pointer to basic block
    /// @returns basic blocks set of Successors
    TBasicBlockSet& getSuccessors(BasicBlock *pBB) {
      assert( m_successorsMap.count(pBB)
        && "basic block has no successor data!" );
      return m_successorsMap[pBB];
    }

    /// @brief return Barrier Predecessors of given sync instruction
    /// @param pInst pointer to sync instruction
    /// @returns basic blocks set of Barrier Predecessors
    SBarrierRelated& getBarrierPredecessors(Instruction *pInst) {
      assert( m_barrierPredecessorsMap.count(pInst)
        && "sync instruction has no barrier predecessor data!" );
      return m_barrierPredecessorsMap[pInst];
    }

    /// @brief return true if given function contains synchronize instruction
    /// @param pFunc pointer to Function
    /// @returns true if and only if given function contains synchronize instruction
    bool hasSyncInstruction(Function *pFunc) {
      if( !m_syncsPerFuncMap.count(pFunc)) return false;
      return (m_syncsPerFuncMap[pFunc].size() > 0);
    }

  private:
    /// @brief execute pass on given function
    /// @param F function to optimize
    /// @returns True if function was modified
    bool runOnFunction(Function &F);

    /// @brief Initialize data of synchronize instruction in processed module
    void InitSynchronizeData();

    /// @brief calculate Predecessors of given basic block
    /// @param pBB pointer to basic block
    void FindPredecessors(BasicBlock *pBB);

    /// @brief calculate Successors of given basic block
    /// @param pBB pointer to basic block
    void FindSuccessors(BasicBlock *pBB);

    /// @brief calculate Barrier Predecessors of given sync instruction
    /// @param pInst pointer to sync instruction
    void FindBarrierPredecessors(Instruction *pInst);

  private:
    /// This is barrier utility class
    BarrierUtils m_util;

    // Analysis Data for pass user
    TInstructionSetPerFunctionMap m_syncsPerFuncMap;
    TDataPerBarrierMap            m_dataPerBarrierMap;
    TBasicBlock2BasicBlocksSetMap m_predecessorsMap;
    TBasicBlock2BasicBlocksSetMap m_successorsMap;
    TBarrier2BarriersSetMap       m_barrierPredecessorsMap;

  };

} // namespace intel

#endif // __DATA_PER_BARRIER_PASS_H__

