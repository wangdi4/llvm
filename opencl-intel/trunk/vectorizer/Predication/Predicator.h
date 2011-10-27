#ifndef __PREDICATOR_H_
#define __PREDICATOR_H_
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Transforms/Scalar.h"

#include "WIAnalysis.h"
#include "PhiCanon.h"
#include "Linearizer.h"
#include "Specializer.h"

using namespace llvm;

namespace intel {
/// @brief The predicator pass
///  This pass insert masks before each of the instructions which has side
///  effects. It linearizes the flow control to allow the execution of several
///  threads by running a single path. See readme.txt for more details.
///  After this pass, many allocas are left in the code. It is essential to run
///  the following transformation (optimization passes): mem2rem, cfg_simplify,
///  dce
/// @author Nadav Rotem
class Predicator : public FunctionPass {

public:
  static char ID; // Pass identification, replacement for typeid
  Predicator() : FunctionPass(ID) {}

private:
  /// Declare the type of our small isntruction vector
  typedef SmallVector<Instruction*, 8> SmallInstVector;
  /// Declare the type for our incoming-value-to-source-edge data structure
  typedef DenseMap<std::pair<BasicBlock*, Value*>, BasicBlock*> IncomingEdgeMap;
  /// For marking an edge between two blocks
  typedef std::pair<BasicBlock*, BasicBlock*> CFGEdge;

  /*! \name Create Helper Function
   *  \{ */
  /// @brief Create the function which checks if all predicates in vector
  /// are zero
  /// @param M Module to place func
  void createAllOne(Module &M);
  /// @brief Create a function call from a general instruction
  /// @param inst Instruction to model
  /// @param pred predicator to consider
  /// @param name C-name for function
  /// @return newly generated function
  Function* createPredicatedFunction(Instruction *inst, Value* pred,
                                     const std::string& name);
  /// @brief Create Select function from PHINodes.
  /// Does not change BasicBlock. Adds the new function to the module.
  /// @param phi Phi to model
  /// @param mask Mask for predicating the select
  /// @return new function.
  Function* createSelect(PHINode* phi, Value* mask);
  /// @brief Move the newly created Instruction after its last dependency
  /// @param inst
  void moveAfterLastDependant(Instruction* inst);
  /*! \} */

  /*! \name Linearize
   * \{ */
  /// @brief register scheduling constraints which are derived from
  ///  loops in our function
  /// @param parent Where to register the constraints
  /// @param LI Loop info about F
  /// @param F Function to scan
  /// @param headers map between the loop headers and the constraints
  ///   which we found
  void registerLoopSchedulingScopes(SchedulingScope& parent, LoopInfo* LI,
                                    Function* F, DenseMap<BasicBlock*,
                                    SchedulingScope*>& headers);
  /// @brief Perform Linearization phase on a function
  /// @param F function to flatten
  /// @param specializer Function specializer for scheduling constraints
  void linearizeFunction(Function* F, FunctionSpecializer& specializer);
  /// @brief after reordering basic blocks, fix PhiNode entry
  /// to reflect new src of incoming values
  /// @param tofix BasicBlock whos PHI we modify
  /// @param src This BB points to the tofix basic block
  void LinearizeFixPhiNode(BasicBlock* tofix, BasicBlock* src);
  /// @brief Perform the actual ordering of a single basic block
  ///  handles adding branch instructions. Handles cases such as loop exit,
  ///  latches, etc.
  /// @param block Block to organize.
  /// @param next If we are in a streight sequence, go to next
  /// @param next_after_loop If we are in a loop, this BB comes after
  ///  loop (exit block)
  void LinearizeBlock(BasicBlock* block, BasicBlock* next,
                      BasicBlock* next_after_loop);
  /*! \} */

  /*! \name Information helpers
   * \{ */
  /// @brief Checks if instruction has users outide of this basic block.
  /// @param inst Instruction to check
  /// @param loop Loop containing the instructions. May be null.
  /// @return true if has outside users.
  bool hasOutsideUsers(Instruction* inst, Loop* loop);
  /*! \} */

  /*! \name Transformations on Basic Block
   * \{ */
  /// @brief Return name of pointee in string representation
  /// @param tp pointer to consider
  /// @return name of type
  std::string getNameFromPointerType(const Type* tp);
  /// @brief Create a functions call which represents a predicated instructions
  //  of the given type.
  /// @param inst Instruction to predicate
  /// @param pred The predication value.
  /// @return Pointer to new function call.
  Instruction* predicateInstruction(Instruction *inst, Value* pred);
  /// @brief Turn all instructions with side-effects to predicated
  ///  function calls
  void predicateSideEffectInstructions();
  /// @brief Replace PHInodes of in-loop merges with selec nodes.
  // this does not replace PHI nodes which are due to loop PHIs.
  /// @param BB BasicBlock to manipulate
  void convertPhiToSelect(BasicBlock* BB);
  /// @brief Insert selection with previous for instruction
  /// which are used outside the basic block.
  /// @param inst inst to manipulate
  void selectOutsideUsedInstructions(Instruction* inst);
  /// @brief The role of this function is to
  /// record instructions of two types:
  /// 1. Store a pointer to all of the
  /// instructions which we need to replace with
  /// a predicated function call.
  /// 2. Stores Store a pointer to all instructions with
  /// outside dependencies for predication with select.
  /// @param BB Block to traverse
  void collectInstructionsToPredicate(BasicBlock *BB);
  /*! \} */

  /*! \name Create Outgoing masks
   * \{ */
  /// @brief Optimized masks are masks which we don't need to calculate because
  /// properties of the graph indicate that the mask had already been
  /// calculated. Regions with a single entry edge and a single exit edge must
  /// have the same input mask and output mask. This function collects these
  /// edges.
  /// @param F Function to process
  /// @param PDT Post dominator info
  /// @param DT Dominator info
  void collectOptimizedMasks(Function* F,
                             PostDominatorTree* PDT,
                             DominatorTree*  DT);
  /// @brief Place outgoing masks on all out-going edged.
  /// @param BB BB to predicate
  void maskOutgoing(BasicBlock *BB);
  /// @brief Place the outgoing masks in the case the BB
  /// is a simple BB which is terminated by a simple unconditional
  /// branch
  /// @param BB Where to place the mask
  /// @param SrcBB Where to take the incoming mask from
  void maskOutgoing_useIncoming(BasicBlock *BB, BasicBlock* SrcBB);
  /// @brief Place the outgoing masks. Use outcoming mask of SrcBB
  /// branch
  /// @param BB Where to place the mask
  /// @param SrcBB Where to take the incoming mask from
  void maskOutgoing_useOutcoming(BasicBlock *BB, BasicBlock* SrcBB);
  /// @brief Place the outgoing masks in the case the BB
  /// is a conditional branch within a loop.
  /// @param BB
  void maskOutgoing_fork(BasicBlock *BB);
  /// @brief Place the outgoing masks in the case the BB
  /// is a conditional branch leaving a loop.
  /// @param BB
  void maskOutgoing_loopexit(BasicBlock *BB);
  /*! \} */

  /*! \name Create Incoming masks
   * \{ */
  /// @brief Place a dummy in-mask.
  /// We shall replace this mask later, after the out-mask are placed.
  /// @param BB BB to predicate
  void maskDummyEntry(BasicBlock *BB);
  /// @brief Place incoming masks.
  /// This replaces the dummy masks with real masks.
  /// @param BB BB to manhandle
  void maskIncoming(BasicBlock *BB);
  /// @brief Place incoming mask which takes after the in-mask of pred
  /// @param BB Place in-mask in here
  /// @param pred Take the in-mask of this
  void maskIncoming_optimized(BasicBlock *BB, BasicBlock* pred);
  /// @brief Place incoming masks on
  /// BB with a single predecessor
  /// @param BB
  /// @param pred Pred May be null in case of entry block
  void maskIncoming_singlePred(BasicBlock *BB, BasicBlock* pred);
  /// @brief Place incoming masks on BB which
  /// are loop headers
  /// @param BB
  /// @param preheader Loop Preheader for this BB
  void maskIncoming_loopHeader(BasicBlock *BB, BasicBlock* preheader);
  /// @brief Place incoming masks on BB which
  /// are in-loop cfg or simple merge.
  /// @param BB
  void maskIncoming_simpleMerge(BasicBlock *BB);
  /*! \} */


  /*! \name Module derivers and entry functions
   * \{ */
  /// @brief Predicate a function
  /// @param F Function to predicate
  void predicateFunction(Function *F);
  /// @brief Checks if this function needs to be predicates
  ///   or if control flow is same for all iterations.
  /// @param F function to manipulate
  bool needPredication(Function &F);
  /*! \} */

public:
  /*! \name LLVM Interface
   * \{ */
  /// @brief LLVM module pass interface
  /// @param F function to predicate
  /// @return true if modified
  virtual bool runOnFunction(Function &F);
  /// @brief requests analysis from LLVM system
  /// @param AU
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    // We need to canonicalize PHIs
    //AU.addRequired<PhiCanon>();
    // We need loop info
    AU.addRequired<LoopInfo>();
    // We need dominance frontier to estimate
    // the dominance for specialization
    AU.addRequired<DominanceFrontier>();
    AU.addRequired<DominatorTree>();
    AU.addRequired<PostDominatorTree>();
    AU.addRequired<RegionInfo>();
    // we preserve nothing. really.
    AU.addRequired<WIAnalysis>();
  }
  /*! \} */

   /*! \name Predicator analysis interface
   * \{ */
  /// @brief requests a mask for edge in graph
  /// @param A src Block
  /// @param B dst Block
  Value* getEdgeMask(BasicBlock* A, BasicBlock* B);
  /// @brief requests the in-mask for edge in graph
  /// @param A src Block
  /// @param B dst Block
  Value* getInMask(BasicBlock* block);
  /*! \} */

private:
  /// Constant holding the value one.
  ConstantInt* m_one;
  /// Constant holding the value zero.
  ConstantInt* m_zero;
  /// Incoming predicators for basic blocks
  DenseMap<BasicBlock*, Value*> m_inMask;
  /// Saves the last instruction which was used
  ///  to save the in-mask for the basic block
  DenseMap<BasicBlock*, Instruction*> m_inInst;
  /// Outgoing predicators for edges between basic blocks
  DenseMap<CFGEdge, Value*> m_outMask;
  /// Masks for edges exiting the loop
  DenseMap<BasicBlock*, Value*> m_exitMask;
  /// Masks for active threads inside the loop
  DenseMap<BasicBlock*, Value*> m_loopMask;
  /// Instructions to predicate (load/store/calls, etc)
  SmallInstVector m_toPredicate;
  /// Instructions which has outside users (for selection)
  SmallInstVector m_outsideUsers;
  /// The function which checks if a vector of predicates is zero
  Function* m_allzero;
  /// The function which checks if a vector of predicates is one
  Function* m_allone;
  /// Maps the created functions to their names
  std::map<std::string, Function*> m_externalFunections;
  /// Maps output masks for blocks with input masks of other blocks.
  DenseMap<BasicBlock*, BasicBlock*> m_optimizedMasks;
};

}
#endif //define __PREDICATOR_H_
