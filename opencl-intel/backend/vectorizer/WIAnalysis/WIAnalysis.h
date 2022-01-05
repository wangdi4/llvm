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

#ifndef __WIANALYSIS_H_
#define __WIANALYSIS_H_

#include "BuiltinLibInfo.h"
#include "Logger.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SoaAllocaAnalysis.h"

#include <map>
#include <queue>

typedef std::map<BasicBlock*, std::vector<BasicBlock*> > SchdConstMap;

using namespace llvm;

namespace intel {

/// @brief Work Item Analysis class used to provide information on
///  individual instructions. The analysis class detects values which
///  depend in work-item and describe their dependency.
///  The algorithm used is recursive and new instructions are updated
///  according to their operands (which are already calculated).
class WIAnalysis : public FunctionPass {
public:
    static char ID; // Pass identification, replacement for typeid
    WIAnalysis();
    /// @breif special C'tor used by ChooseVectorizationDimension.
    /// This forces the vectorization dimension to be the given value,
    /// regardless of the metadata.
    WIAnalysis(unsigned int vectorizationDimension);

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const override {
      return "WIAnalysis";
    }

    /// @brief LLVM Function pass entry
    /// @param F Function to transform
    /// @return True if changed
    virtual bool runOnFunction(Function &F) override;

    /// @brief Update dependency relations between all values
    void updateDeps();

    /// @brief describes the type of dependency on the work item
    enum WIDependancy {
      UNIFORM         = 0, /// All elements in vector are constant
      CONSECUTIVE     = 1, /// Elements are consecutive
      PTR_CONSECUTIVE = 2, /// Elements are pointers which are consecutive
      STRIDED         = 3, /// Elements are in strides
      RANDOM          = 4, /// Unknown or non consecutive order
      NumDeps         = 5  /// Overall amount of dependencies
    };

    /// The WIAnalysis follows pointer arithmetic
    ///  and Index arithmetic when calculating dependency
    ///  properties. If a part of the index is lost due to
    ///  a transformation, it is acceptable.
    ///  This constant decides how many bits need to be
    ///  preserved before we give up on the analysis.
    static const unsigned int MinIndexBitwidthToPreserve;

    /// @brief Returns the type of dependency the instruction has on
    /// the work-item
    /// @param val Value to test
    /// @return Dependency kind
    WIDependancy whichDepend(const Value* val);

    /// @brief Set the type of dependency an instruction should have
    /// and update the data structures accordingly
    /// @param from - the source instruction
    /// @param to - the target instruction
    void setDepend(const Instruction* from, const Instruction* to);

    /// @brief Returns whether BB is a divergent block
    /// @param BB the block
    /// @return true for divergent and false otherwise
    bool isDivergentBlock(BasicBlock *BB);

    /// @brief Returns whether the Phi's in BB are divergent
    /// @param BB the basic block
    /// @return true for divergent and false otherwise
    bool isDivergentPhiBlocks(BasicBlock *BB);

    /// @brief Returns a map with scheduling contraints
    /// @return a map with scheduling contraints
    SchdConstMap & getSchedulingConstraints();

    /// @brief Inform analysis that instruction was invalidated
    /// as pointer may later be reused
    /// @param val Value to invalidate
    void invalidateDepend(const Value* val);

private:
    /*! \name Dependency Calculation Functions
     *  \{ */
    /// @brief Calculate the dependency type for the instruction
    /// @param inst Instruction to inspect
    /// @return Type of dependency.
    void calculate_dep(const Value* val);
    WIDependancy calculate_dep(const BinaryOperator* inst);
    WIDependancy calculate_dep(const UnaryOperator* inst);
    WIDependancy calculate_dep(const CallInst* inst);
    WIDependancy calculate_dep(const GetElementPtrInst* inst);
    WIDependancy calculate_dep(const PHINode* inst);
    WIDependancy calculate_dep_terminator(const Instruction* inst);
    WIDependancy calculate_dep(const SelectInst* inst);
    WIDependancy calculate_dep(const AllocaInst* inst);
    WIDependancy calculate_dep(const CastInst* inst);
    WIDependancy calculate_dep(const VAArgInst* inst);
    /*! \} */

    /// @brief do the trivial checking WI-dep
    /// @param I instruction to check
    /// @return Dependency type. Returns Uniform if all operands are
    ///         Uniform, Random othewise
    WIDependancy calculate_dep_simple(const Instruction *I);

    /// @brief update the WI-dep from a divergent branch
    /// @param the divergent branch and the dependency
    /// @return no explicit return, however, affected instructions
    ///         are added to m_pChangedNew
    void updateDepMap(const Instruction *inst, WIAnalysis::WIDependancy dep);

    /// @brief look for partial joins reachable from two different successo
    /// s.t. the path from each successor accesses the partial join from a 
    /// predecessor
    /// @return
    void findDivergePartialJoins(const Instruction *inst);

    /// @brief mark all the Phi nodes in full/partial joins as random
    /// @return
    void markDependentPhiRandom();

    /// @brief the main function to handle control flow divergence
    /// @param the divergent branch
    /// @return
    void updateCfDependency(const Instruction *inst);

    /// @brief Provide known dependency type for requested value
    /// @param val Value to examine
    /// @return Dependency type. Returns Uniform for unknown type
    WIDependancy getDependency(const Value *val);

    /// @brief return true if there is calculated dependency type for requested value
    /// @param val Value to examine
    /// @return true if value has dependency type, false otherwise.
    bool hasDependency(const Value *val);

    // @brief Calculates the influence region, divergent loops, divergent blocks, and partial joins for
    // the branch inst. PDT is given for post dominance info
    // @param divergent branch
    // @return the full join
    void calcInfoForBranch(const Instruction *inst);

    /// @brief  LLVM Interface
    /// @param AU Analysis
    virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
      // Analysis pass preserve all
      AU.setPreservesAll();
      AU.addRequired<SoaAllocaAnalysisLegacy>();
      AU.addRequired<DominatorTreeWrapperPass>();
      AU.addRequired<PostDominatorTreeWrapperPass>();
      AU.addRequired<LoopInfoWrapperPass>();
      AU.addRequired<BuiltinLibInfo>();
    }

    /// @brief print data collected by the pass on the given module
    /// @param OS stream to print the info regarding the module into
    /// @param M pointer to the Module
    void print(raw_ostream &OS, const Module *M) const override;

  private:
    // @brief pointer to Soa alloca analysis performed for this function
    SoaAllocaInfo *m_soaAllocaInfo;
    /// Stores an updated list of all dependencies
    DenseMap<const Value*, WIDependancy> m_deps;
    /// Runtime services pointer
    RuntimeServices * m_rtServices;
    /// Iteratively one set holds the changed from the previous iteration and
    /// the other holds the new changed values from the current iteration.
    std::set<const Value*> m_changed1;
    std::set<const Value*> m_changed2;
    /// ptr to m_changed1, m_changed2
    std::set<const Value*> *m_pChangedOld;
    std::set<const Value*> *m_pChangedNew;

   /// Analyses needed by the control flow divergence
   /// propagation
    DominatorTree *m_DT;
    PostDominatorTree *m_PDT;
    LoopInfo *m_LI;

    //// Fields for the control flow divergence propagation

    //// block info - these are general for the kernel and not branch specific

    // stores the divergent blocks - ones that have an input mask
    DenseSet<const BasicBlock*> m_divBlocks;

    // stores the divergent phi block nodes - ones that has divergent output due to the control flow
    DenseSet<const BasicBlock*> m_divPhiBlocks;

    // holds the divergent branches waiting to propagate their divergency
    std::queue<const BranchInst*> m_divBranchesQueue;

    //// branch specific info

    // Immediate post-dominator
    // In case where the immediate post-dominator of a branch is
    // inside a loop, the latch node is not an exiting node, and
    // the latch node is in the influence region.
    // Then m_fullJoin is moved to be the first post-dominator
    // outside the loop

    BasicBlock *m_fullJoin;

    // influence region - blocks that exist in a path from cbr to fullJoin
    DenseSet<BasicBlock*> m_influenceRegion;

    // blocks in influenceRegion that are reachable from cbr by two different successors
    SmallPtrSet<BasicBlock*, 4> m_partialJoins;

    // blocks in influenceRegion that are reachable from cbr by its two successors 
    // and there exists a path from each successors that accesses the block from a different predecessor
    SmallPtrSet<BasicBlock*, 4> m_divergePartialJoins;

    // A map that maps a block terminating with a divergent branch to a vector containing the divergent branch basic block
    // together with its influence region and its immediate post-dominator.
    // Later on we use it to add scheduling constraints for the linearizer
    SchdConstMap m_SchedulingConstraints;

    // the dimension over which we vectorize (usually 0).
    unsigned int m_vectorizedDim;
  };
} // namespace


#endif //define __WIANALYSIS_H_
