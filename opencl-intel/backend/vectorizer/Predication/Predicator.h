/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __PREDICATOR_H_
#define __PREDICATOR_H_
#include "BuiltinLibInfo.h"
#include "WIAnalysis.h"
#include "PhiCanon.h"
#include "Linearizer.h"
#include "Specializer.h"
#include "OclTune.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

namespace intel {
/// @brief The predicator pass
///  This pass insert masks before each of the instructions which has side
///  effects. It linearizes the flow control to allow the execution of several
///  threads by running a single path. See readme.txt for more details.
///  After this pass, many allocas are left in the code. It is essential to run
///  the following transformation (optimization passes): mem2rem, cfg_simplify,
///  dce
class Predicator : public FunctionPass {

public:
  static char ID; // Pass identification, replacement for typeid
  Predicator();

  /// @brief Provides name of pass
  virtual StringRef getPassName() const {
    return "Predicator";
  }

  struct BranchInfo {
    BranchInfo(BasicBlock * succ0, BasicBlock * succ1, Value* cond) : m_succ0(succ0), m_succ1(succ1), m_cond(cond) {}
    BranchInfo() : m_succ0(0), m_succ1(0), m_cond(0) {}

    BasicBlock * m_succ0;
    BasicBlock * m_succ1;
    Value * m_cond;
  };

  /// type of the block inside the allones structure:
  // If not a single loop block, then one of:
  //
  //              entry
  //             .     .
  //            .       .
  //       original    allones
  //            .       .
  //             .     .
  //              exit
  //
  //
  // If a single loop block, then one of:
  //             entry
  //               .         .
  //all-ones -> all-ones      .
  //             . .           .
  //          test all zeroes  .
  //          .    .          .
  //         .   entry2      <- entry
  //         .     .
  //         .  original  <- original
  //          .    .   .
  //            . exit
   enum AllOnesBlockType {
      /// Block not related to an allone bypass.
      NONE  = 0,
      ENTRY = 1,
      ORIGINAL = 2,
      ALLONES = 3,
      EXIT = 4,
      SINGLE_BLOCK_LOOP_ENTRY = 5,
      SINGLE_BLOCK_LOOP_ALLONES = 6,
      SINGLE_BLOCK_LOOP_TEST_ALLZEROES = 7,
      SINGLE_BLOCK_LOOP_ENTRY_TO_ORIGINAL = 8,
      SINGLE_BLOCK_LOOP_ORIGINAL = 9,
      SINGLE_BLOCK_LOOP_EXIT = 10
  };

  /// @brief Return true if this is an entry BB to UCF region.
  bool isUCFEntry(BasicBlock* BB) const;
  /// @brief Return true if this is an interior UCF region BB.
  bool isUCFInter(BasicBlock* BB) const;
  /// @brierf Return true if this is an exit BB from UCF region.
  bool isUCFExit(BasicBlock* BB) const;
  /// @brierf Return exit BB of UCF region or NULL.
  BasicBlock * getUCFExit(BasicBlock* entryBB);
  /// @brierf Return entry BB of UCF region or NULL.
  BasicBlock * getUCFEntry(BasicBlock* entryBB);

private:
  /// Declare the type of our small instruction vector
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
  /// @brief returns true if the function arguments contain a pointer
  /// to local memory, false otherwise.
  /// @param F the function to test.
  bool doFunctionArgumentsContainLocalMem(Function* F);
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
  /// @brief add scheduling constraints for divergent regions
  /// @param main_scope - the scope that we should add the constraints to
  void addDivergentBranchesSchedConstraints(SchedulingScope& main_scope);
  /// @brief register Uniform Control Flow scheduling contraints
  /// @param main_scope - the scope that we should add the constraints to
  void registerUCFSchedulingScopes(SchedulingScope& main_scope);
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
  /// @param loop Loop this BB is inside
  /// @param next_after_loop If we are in a loop, this BB comes after
  ///  loop (exit block)
  void LinearizeBlock(BasicBlock* block, BasicBlock* next,
                      Loop* loop, BasicBlock* next_after_loop);
  /*! \} */

  /*! \name Information helpers
   * \{ */
  /// @brief Checks if instruction has random users outide of its loop.
  /// @param inst Instruction to check
  /// @param loop Loop containing the instructions. May be null.
  /// @return true if has outside users.
  bool hasOutsideRandomUsers(Instruction* inst, Loop* loop);
  /*! \} */

  /*! \name Transformations on Basic Block
   * \{ */
  /// @brief Return name of pointee in string representation
  /// @param tp pointer to consider
  /// @return name of type
  std::string getNameFromPointerType(Type* tp);
  /// @brief replace the original instruction by the predicated one,
  /// and saves the data for the allones bypasser if needed.
  /// @param original instruction that is to be replaced.
  /// @param predicated the predicated version of the same instruction.
  void replaceInstructionByPredicatedOne(Instruction* original,
                                         Instruction* predicated);
  /// @brief Create a functions call which represents a predicated instructions
  //  of the given type.
  /// @param inst Instruction to predicate
  /// @param pred The predication value.
  /// @return Pointer to new function call.
  Instruction* predicateInstruction(Instruction *inst, Value* pred);
  /// @brief Turn all instructions with side-effects to predicated
  ///  function calls
  void predicateSideEffectInstructions();
  /// @brief returns the condition of the branch that determines
  /// which value is selected in the phi-node.
  /// May fail to find the condition and return NULL.
  /// @param phi the phi-node to find the condition for.
  /// @param switchValuesOrder being set to true if the first incoming value of the
  /// phi should be taken when the condition is true. False if otherwise.
  Value* getPhiCond(PHINode* phi, bool& switchValuesOrder);
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
                             DominatorTree* DT);
  /// @brief Test if a given region do not have divergent control flow inside
  /// @param entryBB entry basic block
  /// @param exitBB exit basic block
  /// @return true if this is UCF region and false overwise
  bool isUCFRegion(BasicBlock * const entryBB, BasicBlock * const exitBB, LoopInfo * LI);
  /// @brief Uniform Control Flow subregion is a part of control flow graph inside
  /// divergent control flow region with uniform branches only in a whole SESE region.
  /// This property allows to preserve the control flow inside these regions instead of
  /// linearazing the entire divergent region.
  /// @param F Function to process
  /// @param LI Loop info about F
  /// @param PDT Post dominator info
  /// @param DT Dominator info
  void collectUCFRegions(Function* F, LoopInfo * LI,
                         PostDominatorTree * PDT,
                         DominatorTree *  DT);
  /// @brief for each block that ends with a divergent branch,
  /// saves the branchInfo into m_branchesInfo
  /// @param F Function to process
  void collectBranchesInfo(Function* F);
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

  /// @brief checks whether in every loop iteration the exit block is reached.
  /// @param loopHeader the header of the loop.
  /// @param exitBlock the block to be checked as reached in every iteration.
  bool isAlwaysFollowedBy(Loop *L, BasicBlock* exitBlock);
  /// @brief checks whether BasicBlock dst is reachable from
  /// BasicBlock source in the same loop iteration (without using any backedges).
  /// @param src source basic block
  /// @param dst destination basic block
  bool isReachableInsideIteration(BasicBlock* src, BasicBlock* dst);

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
  /// @brief replaces a predicated instruction with a non-predicated one.
  /// Assumes the predicated instruction
  /// is present in m_predicatedToOriginalInst.
  /// @param inst The instruction to unpredicate.
  void unpredicateInstruction(Instruction* inst);
  /// @brief returns true if the given instruction is a load
  /// of local memory.
  /// @param inst Instruction to test whether it is load of local memory.
  bool isLocalMemoryConsecutiveLoad(Instruction* inst);
  /// @brief check if the instruction is a masked store or load
  /// with uniform parameters. That is, the mask is the only non-unifrom
  /// parameter.
  /// @param inst The instruction to check.
  bool isMaskedUniformStoreOrLoad(Instruction* inst);
  /// @brief calculates the heuristic that decides
  /// which divergent branches should be allones bypassed.
  /// this method runs before predication. The results are stored inside
  /// m_heuristic decisions and used later.
  void calculateHeuristic(Function* F);
  /// @brief upon predicating an instruction, tests whether
  /// the original instruction should be kept as well, for later
  /// use inside the predicator.
  /// @param original the original (unpredicated) instruction.
  bool keepOriginalInstructionAsWell(Instruction* original);
  /// @brief erase original instructions that were kept
  /// alongside their predicated version, but were eventually unused.
  void clearRemainingOriginalInstructions();
  /// @beief Loops that begin with full masks (all items
  /// always reach them) but have divergent exit conditions,
  /// are de-facto zero-bypassed, because they are never run
  /// with empty (zero) masks. This method declares them as such,
  /// and thus enables further optimizations for these loops.
  void markLoopsThatBeginsWithFullMaskAsZeroBypassed();
  /// @brief true if the given block holds load and/or store instructions.
  bool blockHasLoadStore(BasicBlock* BB);
  /// @brief inserts allones bypasses into the code, in places where the heuristics
  /// suggested to do so.
  void insertAllOnesBypasses();
  /// @brief Create allOnes bypass for an entrire UCF region
  /// @param ucfEntryBB - UCF entry BB
  void insertAllOnesBypassesUCFRegion(BasicBlock * const ucfEntryBB);
  /// @brief In case that the block we want to test for allones is
  /// a loop consists of a single block, then we treat it differently
  /// (in order to be more efficient).
  /// @param original The single loop block.
  void insertAllOnesBypassesSingleBlockLoopCase(BasicBlock* original);

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

  /// @brief Checks if the input function is in canonical form
  /// @param F function to manipulate
  bool checkCanonicalForm(Function *F, LoopInfo *LI);

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
    AU.addRequired<LoopInfoWrapperPass>();
    // We need dominance frontier to estimate
    // the dominance for specialization
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<PostDominatorTreeWrapperPass>();
    // we preserve nothing. really.
    AU.addRequired<WIAnalysis>();
    // for bypasses usage
    AU.addRequired<OCLBranchProbability>();
    AU.addRequired<BuiltinLibInfo>();
  }
  /*! \} */

   /*! \name Predicator analysis interface
   * \{ */
  /// @brief requests a mask for edge in graph
  /// @param A src Block
  /// @param B dst Block
  Value* getEdgeMask(BasicBlock* A, BasicBlock* B);
  /// @brief requests the in-mask for edge in graph
  /// @param block src Block
  Value* getInMask(BasicBlock* block);
  /// @brief The specializer uses this method to inform
  /// the predicator that a block has been successfully bypassed
  /// by an allzero bypass, and thus, if executed
  /// can be assume to have a non-zero mask.
  /// @param BB The basic block that is bypassed.
  void blockIsBeingZeroBypassed(BasicBlock* BB);
  /*! \} */

  /// @brief expects a block that is a single loop block and that has an 'allone'
  /// single loop block twin. Returns the twin.
  /// @param originalSingleLoop A block that is both a header and a latch of a
  /// loop. This block should be predicated, but should also have an
  /// allones uniform twin.
  static BasicBlock* getAllOnesSingleLoopBlock(BasicBlock* originalSingleLoop);
  /// @brief expects a block that is of AllOnesBlockType::ORIGINAL type.
  /// returns the entry predecessor.
  /// @param original A predicated block that was bypassed with an allones
  /// bypass.
  static BasicBlock* getEntryBlockFromOriginal(BasicBlock* original);
  /// @brief expects a block that is of
  /// AllOnesBlockType::SINGLE_BLOCK_LOOP_ORIGINAL type.
  /// returns the corresponding SINGLE_BLOCK_LOOP_ENTRY.
  /// @param loopOriginal A block that is both a header and a latch of a
  /// loop. This block should be predicated, but should also have an
  /// allones uniform twin.
  static BasicBlock* getEntryBlockFromLoopOriginal(BasicBlock* loopOriginal);
  /// @brief If this block is part of an allones bypass,
  /// returns the specific AllOnesBlock type. Otherwise
  /// returns AllOnesBlock::NONE
  /// @param BB
  static AllOnesBlockType getAllOnesBlockType(BasicBlock* BB);
  /// @brief If this blocks ends with a conditional branch,
  /// and the condition is a call instruction to the allones function,
  /// then this method returns this conditional branch.
  /// otherwise returns NULL.
  /// @param BB
  static BranchInst* getAllOnesBranch(BasicBlock* BB);

private:
  /// Pointer to runtime service object
  const RuntimeServices * m_rtServices;
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
  /// Instructions to predicate (load/store/calls, etc)
  SmallInstVector m_toPredicate;
  /// Instructions which has outside users (for selection)
  SetVector<Instruction*> m_outsideUsers;
  /// The function which checks if a vector of predicates is zero
  Function* m_allzero;
  /// The function which checks if a vector of predicates is one
  Function* m_allone;
  /// Maps the created functions to their names
  std::map<std::string, Function*> m_externalFunections;
  /// Maps output masks for blocks with input masks of other blocks.
  DenseMap<BasicBlock*, BasicBlock*> m_optimizedMasks;
  /// Counter for masked load
  int m_maskedLoadCtr;
  /// Counter for masked store
  int m_maskedStoreCtr;
  /// Counter for masked call
  int m_maskedCallCtr;
  /// true if the predicated function gets arguments that are pointers to local mem.
  bool m_hasLocalMemoryArgs;
  // Work-item analysis pointer
  WIAnalysis* m_WIA;
  // Dominator tree pointer.
  DominatorTree* m_DT;
  // Loop info pointer
  LoopInfo* m_LI;

  /// blocks that the heuristic decides it is a good idea
  /// to test their mask for allones.
  std::set<BasicBlock*> m_valuableAllOnesBlocks;
  /// when predicating an instruction, if the original instruction
  /// is also going to be used for an allones block, create this mapping.
  std::map<Instruction*,Instruction*> m_predicatedToOriginalInst;
  /// when inserting a select instruction for instructions that are
  /// used outside loops, create this mapping from the select instruction
  /// into the first value that is used for the select (which is
  /// the needed value if the mask is allones.)
  std::map<Instruction*,Instruction*> m_predicatedSelects;
  /// For each basic block that ends with a divergent branch,
  /// hold the original branch info (before changes by the predicator).
  std::map<BasicBlock*, BranchInfo> m_branchesInfo;
  /// Entry basic blocks to UCF regions
  std::map<BasicBlock*, BasicBlock*> m_ucfEntry2Exit;
  /// Interior UCF BB with relation to the entry BBs.
  std::map<BasicBlock*, BasicBlock*> m_ucfInter2Entry;
  /// Exit basic blocks out of UCF regions with relation to the entry BBs
  std::map<BasicBlock*, BasicBlock*> m_ucfExit2Entry;
  /// UCF region scheduling constraints
  SchdConstMap m_ucfSchedulingConstraints;

  // Statistics:
  Statistic::ActiveStatsT m_kernelStats;
  Statistic Predicated_Uniform_Store_Or_Loads;
  Statistic AllOnes_Bypasses;
  Statistic AllOnes_Bypasses_Due_To_Non_Consecutive_Store_Load;
  Statistic Predicated;
  Statistic Unpredicated_Uniform_Store_Load;
  Statistic Unpredicated_Cosecutive_Local_Memory_Load;
  Statistic Predicated_Consecutive_Local_Memory_Load;
  Statistic Preserved_Uniform_Conrol_Flow_Regions;
  public: Statistic Edge_Not_Being_Specialized_Because_EdgeHot;
  public: Statistic Edge_Not_Being_Specialized_Because_EdgeHot_At_Least_50Insts;
  public: Statistic Edge_Not_Being_Specialized_Because_Should_Not_Specialize;
  public: Statistic Edge_Not_Being_Specialized_Break_Inside_A_Loop;
  public: Statistic Zero_Bypasses;
};

}
#endif //define __PREDICATOR_H_
