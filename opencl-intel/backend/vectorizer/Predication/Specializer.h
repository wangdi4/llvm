/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef SPECIALIZER_H
#define SPECIALIZER_H
#include "Linearizer.h"
#include "OCLBranchProbability.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

namespace intel {

/// @brief Function Specialization
///  This classes is used by the Predicator. It operates in two passes.
///  First, before the predication, it collects dominance info from the original
///  control flow. Second, after the linearization
///  the actual specialization happens.
class Predicator;
class FunctionSpecializer {
public:

  /// Holds the data needed for each bypass
  struct BypassInfo {
    BypassInfo(BasicBlock * head, BasicBlock * root) : m_head(head), m_root(root), m_postDom(0), m_foot(0) {}
    BypassInfo() : m_head(0), m_root(0), m_postDom(0), m_foot(0) {}

    BasicBlock * m_head;    // Single predecessor of the entry node
    BasicBlock * m_root;    // Entry node
    BasicBlock * m_postDom; // Exit node
    BasicBlock * m_foot;    // Single successor of the exit node

    std::set<BasicBlock*> m_skippedBlocks; // All the basic blocks that should be bypasses, including m_root and m_postDom
  };

  struct BypassInfoComparator
  {
    bool operator()(const BypassInfo &bi1, const BypassInfo &bi2) const
    {
      if (bi1.m_head != bi2.m_head) 
        return bi1.m_head < bi2.m_head;
      if (bi1.m_root != bi2.m_root) 
        return bi1.m_root < bi2.m_root;
      if (bi1.m_postDom != bi2.m_postDom) 
        return bi1.m_postDom < bi2.m_postDom;
      return bi1.m_foot < bi2.m_foot;
    }
  };

  /// Short term BB collection
  typedef std::vector<BasicBlock*> BBVector;

  /// @brief C'tor
  /// @param pass Predication pass
  /// @param func Function to specialize
  /// @param inMasks map of incoming masks for basic blocks
  /// @param inMaskPlace Location of in-mask calculation
  /// @param all_zero Function to check for all-zero of values
  /// @param PDT postdominator analysis
  /// @param DT dominator analysis
  FunctionSpecializer(Predicator* pred, Function* func, Function* all_zero,
                      PostDominatorTreeWrapperPass* PDT, DominatorTree*  DT,
                      LoopInfo *LI, WIAnalysis *WIA, OCLBranchProbability *OBP);

  /// @brief Finds a single edge to specialize. This uses
  ///  the control dominance of the block to check.
  ///  Of all the CD-ed children, It finds the one which post-dominates
  ///  all others.
  /// @param mojo BasicBlock which Control Dominates other blocks
  /// @param branch Initial jump point to consider
  /// @return Destination edge to specialize or NULL
  BasicBlock* findMostPDomControlDep(BasicBlock* mojo, BasicBlock* branch);
  /// @brief Checks for control dominance of two blocks.
  ///  See HPC for Parallel Compiting, by Michael Wolfe pg 71.
  /// @param x src CD
  /// @param y dst CD
  /// @return True if X control dominates Y.
  bool isControlDominance(BasicBlock* x, BasicBlock *y);
  /// @brief Can this function be specialized ?
  /// Function can't be specialized if it has endless loop in it.
  /// Namely, not all blocks are postdominated by a single return block
  /// @return True if can be specialized.
  bool CanSpecialize();
  /// @brief Collect the needed information for specialization
  ///  Mainly collects dominator frontier information
  void CollectDominanceInfo();
  /// @brief Perform the specialization of all functions
  /// if profitable
  void specializeFunction();
  /// @brief Generate a list of linearization scopes
  ///  which will be used for scheduling of basic blocks
  /// @param parent Registration of scheduling scopes is performed
  ///  this main scope.
  void registerSchedulingScopes(SchedulingScope& parent);

private:
  /// @brief Find the first basic block which
  /// terminates with a return inst
  /// @return Block contatning return or Null
  BasicBlock* getAnyReturnBlock();
  /// @brief A heuristic to decide under which conditions
  ///  do we need to specialize the control flow of this region
  /// @param bi bypass information
  /// @return True if specialization if profitable
  bool shouldSpecialize(const BypassInfo & bi) const;
  /// @brief For all regions, if the region contained the old block, add a new block
  /// @param block Old Block to search
  /// @param fresh New block to add
  void addNewToRegion(BasicBlock* block, BasicBlock* fresh);
  /// @brief Create a new basic block on the edge between Before to After. 
  ///  sd
  /// @param Before Place it After this
  /// @param After Place it Before this
  /// @param name Name to use
  BasicBlock* createIntermediateBlock(BasicBlock* before, BasicBlock* after, const std::string& name);
  /// @brief Perform the specialization of an edge, from src to dst
  /// @param bi bypass info
  void specializeEdge(BypassInfo & bi);
  /// @brief Collects the list of instructions for which we need to
  ///  add a PHINode
  /// @param src Edge source
  /// @param dst Edge destination
  /// @param body block
  /// @param to_add_phi saves the list of instructions to this map
  void findValuesToPhi(
    BypassInfo & bi,
    std::vector<std::pair<Instruction* , std::set<Instruction*> > > &to_add_phi);
  /// @brief Propagates masks of bypassed region to the 'footer' basic block
  /// @param mask_target mask of a skipped basic block
  /// @param header      header basic block of the bypass
  /// @param exitBlock   exit block of the region
  /// @param footer      footer basic block of the bypass
  void propagateMask( Value *mask_target, BasicBlock *header, BasicBlock *exitBlock, 
                      BasicBlock *footer);

  /// @brief obtains the masks needed to be zeroed in the region header.
  /// @param bi - bypass info.
  void ObtainMasksToZero(BypassInfo & bi);

  /// @brief zero masks that are computed inside the region but used outside.
  /// @param bi - Bypass information.
  /// @param src - region header.
  /// @param exit - region exit block.
  /// @param footer - region footer.
  void ZeroBypassedMasks(BypassInfo & bi, BasicBlock *src, BasicBlock *exit,
                         BasicBlock *footer);

  /// @brief Calculating bypass information for a region starting at root
  /// root is a successor of either a divergent branch or a branch located
  /// in a divergent block
  /// @param root - entry point for a potential region
  /// @return - returns true if root is a potential BB for bypass
  bool calculateBypassInfoPerBranch(BasicBlock * root);

  /// @brief Giving an entry and an exit point for a region
  /// this function finds all the nodes inside the region and
  /// update the skipped set
  /// @param info - bypass info containing entry and exit blocks
  /// for the region and a set for adding the region's blocks
  void getBypassRegion(BypassInfo & info);

  /// @brief Add auxiliary node to allow support of bypass
  /// with two outgoing edges
  /// @param info - bypass info
  void addAuxBBForSingleExitEdge(BypassInfo & info);

  /// @brief A heuristics for adding a bypass for a single basic block
  /// @param BB - the basic block nominated for a bypass
  /// @return - returns true if the bypass should be added
  bool addHeuristics(const BasicBlock *BB) const;

  /// @brief initialize the cost of some of the built-in function
  /// To be used by the heuristics that decides whether a bypass should be added
  /// above a single basic block
  void initializeBICost();

private:
  /// Predicator pass
  Predicator* m_pred;
  /// Function we specialize
  Function* m_func;
  /// Function which calculates the all_zero of masks
  Function* m_allzero;
  /// Post Dominator tree analysis for function
  PostDominatorTreeWrapperPass* m_PDT;
  /// Dominator tree analysis for function
  DominatorTree* m_DT;
  // LoopInfo
  LoopInfo *m_LI;
  // Work Item Analysis
  WIAnalysis *m_WIA;
  // Branch probability analysis - for bypasses addition
  OCLBranchProbability *m_OBP;
  /// Zero
  Value* m_zero;
  /// One
  Value* m_one;
  /// Region out masks to zero 
  typedef std::vector<std::pair<BasicBlock*, BasicBlock*> > BBPairVec;
  typedef std::map<BypassInfo, BBPairVec, BypassInfoComparator> MapRegToBBPairVec;
  MapRegToBBPairVec m_outMasksToZero;
  /// Region in masks to zero 
  std::map<BypassInfo, BasicBlock*, BypassInfoComparator> m_inMasksToZero;

  /// A map that maps from a function name to the number of instructions that this function is composed of.
  /// If a function is not in the map then the number of instruction is inf
  std::map<std::string, unsigned> m_nameToInstNum;

  /// A vector containing the info for all the potential bypasses
  std::vector<BypassInfo> m_bypassInfoContainer;
};

} // namespace

#endif /* SPECIALIZER_H */
