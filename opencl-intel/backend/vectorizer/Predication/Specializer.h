#ifndef SPECIALIZER_H
#define SPECIALIZER_H
#include "Linearizer.h"
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/RegionInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

namespace intel {

/// @brief Function Specialization
///  This classes is used by the Predicator. It operates in two passes.
///  First, before the predication, it collects dominance info from the original
///  control flow. Second, after the linearization
///  the actual specialization happens.
/// @author Nadav Rotem
class Predicator;
class FunctionSpecializer {
public:
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
                      PostDominatorTree* PDT, DominatorTree*  DT,
                      RegionInfo* RI, LoopInfo *LI);
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
  ///  do we nneed to specialize the control flow of this region
  /// @param reg Region to specialize
  /// @return True if specialization if profitable
  bool shouldSpecialize(Region* reg);
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
  /// @param reg Region to specialize
  void specializeEdge(Region* reg);
  /// @brief Collects the list of blocks which are skipped in the
  // specialization
  /// @param Region to consider
  /// @param List of BB which are skipped
  void findSkippedBlocks(Region* reg, BBVector& skipped);
  /// @brief Collects the list of instructions for which we need to
  ///  add a PHINode
  /// @param src Edge source
  /// @param dst Edge destination
  /// @param body block
  /// @param to_add_phi saves the list of instructions to this map
  void findValuesToPhi(
    Region* reg,
    std::vector<std::pair<Instruction* , std::set<Instruction*> > > &to_add_phi);
  /// @brief Propagates masks of bypassed region to the 'footer' basic block
  /// @param mask_target mask of a skipped basic block
  /// @param header      header basic block of the bypass
  /// @param exitBlock   exit block of the region
  /// @param footer      footer basic block of the bypass
  void propagateMask( Value *mask_target, BasicBlock *header, BasicBlock *exitBlock, 
                      BasicBlock *footer);
  /// @brief Helper which returns 'true' if region has a successor or 'false' otherwise
  /// @param reg Region in question
  bool RegionHasSuccessor( Region * reg);

  /// @brief obtains the masks needed to be zeroed in the region header.
  /// @param reg - current region.
  /// @param exit - the block inside the region on the exit edge.
  /// @param foot - the block outside the region on the exit edge.
  void ObtainMasksToZero(Region *reg, BasicBlock *exit, BasicBlock *foot);

  /// @brief zero masks that are computed inside the region but used outside.
  /// @param reg - Current region.
  /// @param src - region header.
  /// @param exit - region exit block.
  /// @param footer - region footer.
  void ZeroBypassedMasks(Region *reg, BasicBlock *src, BasicBlock *exit,
                         BasicBlock *footer);

private:
  /// Predicator pass
  Predicator* m_pred;
  /// Function we specialize
  Function* m_func;
  /// Function which calculates the all_zero of masks
  Function* m_allzero;
  /// Post Dominator tree analysis for function
  PostDominatorTree* m_PDT;
  /// Dominator tree analysis for function
  DominatorTree* m_DT;
  // RegionInfo
  RegionInfo* m_RI;
  // LoopInfo
  LoopInfo *m_LI;
  /// Zero
  Value* m_zero;
  /// One
  Value* m_one;
  /// Region preheader
  std::map<Region*, BasicBlock*> m_heads;
  /// Region out masks to zero 
  typedef std::vector<std::pair<BasicBlock*, BasicBlock*> > BBPairVec;
  typedef std::map<Region*, BBPairVec > MapRegToBBPairVec;
  MapRegToBBPairVec m_outMasksToZero;
  /// Region in masks to zero 
  std::map<Region*, BasicBlock*> m_inMasksToZero;
  /// specialization regions
  std::vector<Region*> m_region_vector;
  /// All skipped blocks
  std::map<Region*, BBVector> m_skipped;
};


} // namespace

#endif /* SPECIALIZER_H */
