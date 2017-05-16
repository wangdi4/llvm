/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#define DEBUG_TYPE "predicate"
#include "Predicator.h"
#include "CompilationUtils.h"
#include "VectorizerUtils.h"
#include "Specializer.h"
#include "Linearizer.h"
#include "Mangler.h"
#include "Logger.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "OCLAddressSpace.h"

#include "llvm/Pass.h"
#include "llvm/InitializePasses.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <iostream>

static cl::opt<bool>
EnableOptMasks("optmasks", cl::init(true), cl::Hidden,
  cl::desc("Optimize masks generation"));

static cl::opt<bool>
PreserveUCF("presucf", cl::init(true), cl::Hidden,
  cl::desc("Partially preserve uniform control flow inside divergent region"));

static cl::opt<bool>
EnableAllOnes("all-ones", cl::init(true), cl::Hidden,
  cl::desc("Insert all-ones bypasses"));

// Invoked by runOnModule for every predicated function
STATISTIC(PredicatorCounter, "Counts number of functions visited");

namespace intel {

/// Support for dynamic loading of modules under Linux
char Predicator::ID = 0;

const int MAX_NUMBER_OF_BLOCKS_IN_AN_ALLONES_BYPASS = 6;

OCL_INITIALIZE_PASS_BEGIN(Predicator, "predicate", "Predicate Function", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(DominanceFrontierWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(WIAnalysis)
OCL_INITIALIZE_PASS_DEPENDENCY(OCLBranchProbability)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(Predicator, "predicate", "Predicate Function", false, false)

Predicator::Predicator() :
  FunctionPass(ID),
  m_maskedLoadCtr(0),
  m_maskedStoreCtr(0),
  m_maskedCallCtr(0),
  OCLSTAT_INIT(Predicated_Uniform_Store_Or_Loads,
    "store or loads with uniform address and value but with masks, that are not zerobypassed and thus predicated"
    , m_kernelStats),
  OCLSTAT_INIT(AllOnes_Bypasses,
    "total number of allones bypasses inserted",
    m_kernelStats),
  OCLSTAT_INIT(AllOnes_Bypasses_Due_To_Non_Consecutive_Store_Load,
    "number of allones bypasses for blocks that do not contain consecutive store or load instructions",
    m_kernelStats),
  OCLSTAT_INIT(Predicated,
    "one if the function is predicated, zero otherwise",
    m_kernelStats),
  OCLSTAT_INIT(Unpredicated_Uniform_Store_Load,
    "instructions being unpredicated because they are uniform store/load",
    m_kernelStats),
  OCLSTAT_INIT(Unpredicated_Cosecutive_Local_Memory_Load,
    "instructions being unpredicated because they are consecuitve local memory load",
    m_kernelStats),
  OCLSTAT_INIT(Predicated_Consecutive_Local_Memory_Load,
    "load instructions of local memory that could potentially be unmasked, but currently aren't",
    m_kernelStats),
  OCLSTAT_INIT(Preserved_Uniform_Conrol_Flow_Regions,
    "preserved Uniform Control Flow regions inside Divergent Control Flow regions",
    m_kernelStats),
  OCLSTAT_INIT(Edge_Not_Being_Specialized_Because_EdgeHot,
    "edges that were chosen not to be specialized because of the EdgeHot heuristic",
    m_kernelStats),
  OCLSTAT_INIT(Edge_Not_Being_Specialized_Because_EdgeHot_At_Least_50Insts,
    "edges that were chosen not to be specialized because of the EdgeHot heuristic, AND the bypassed region consists of at least 50 insts",
    m_kernelStats),
  OCLSTAT_INIT(Edge_Not_Being_Specialized_Because_Should_Not_Specialize,
    "edges that were chosen not to be specialzied only because of the ShouldSpecialize heuristic",
    m_kernelStats),
  OCLSTAT_INIT(Edge_Not_Being_Specialized_Break_Inside_A_Loop,
    "edges that were not specialized because they are (probably) break instructions inside a loop that has a conditional latch",
    m_kernelStats),
  OCLSTAT_INIT(Zero_Bypasses,
    "total number of zero bypasses inserted",
    m_kernelStats)
{
  initializePredicatorPass(*llvm::PassRegistry::getPassRegistry());
  m_rtServices = NULL;
}

bool Predicator::doFunctionArgumentsContainLocalMem(Function* F) {
  for (Function::ArgumentListType::iterator it = F->getArgumentList().begin(),
    e = F->getArgumentList().end(); it!= e; ++it) {
    if (PointerType* ptrType = dyn_cast<PointerType>(it->getType())) {
      if (ptrType->getAddressSpace() == Intel::OpenCL::DeviceBackend::Utils::OCLAddressSpace::Local) {
        return true;
      }
    }
  }
  return false;
}

void Predicator::createAllOne(Module &M) {
  // Create function arguments
  std::vector<Type*> allOneArgs;
  allOneArgs.push_back(IntegerType::get(M.getContext(), 1));
  FunctionType* allOneType = FunctionType::get(
    IntegerType::get(M.getContext(), 1), allOneArgs, false);
  // Declare function
  Function* func_allOne = dyn_cast<Function>(M.getOrInsertFunction(
      Mangler::name_allOne, allOneType));
  V_ASSERT(func_allOne && "Function type is incorrect, so dyn_cast failed");

  // Save for later
  m_allone = func_allOne;

  // Declare function
  Function* func_allZero = dyn_cast<Function>(M.getOrInsertFunction(
      Mangler::name_allZero, allOneType));
  V_ASSERT(func_allZero && "Function type is incorrect, so dyn_cast failed");

  // Save for later
  m_allzero = func_allZero;
}

bool Predicator::needPredication(Function &F) {

  /// Place out-masks
  for (Function::iterator it = F.begin(), e  = F.end(); it != e ; ++it) {
    if (dyn_cast<ReturnInst>(it->getTerminator())) continue;
    WIAnalysis::WIDependancy dep = m_WIA->whichDepend(it->getTerminator());
    if (dep != WIAnalysis::UNIFORM) {
      V_PRINT(predicate, F.getName()<< "needs predication because of "<<it->getName()<<" \n");
      return true;
    }
  }

  V_PRINT(predicate, F.getName()<< "does not need predication \n");
  return false;
}

bool Predicator::runOnFunction(Function &F) {

  m_rtServices = getAnalysis<BuiltinLibInfo>().getRuntimeServices();
  V_ASSERT(m_rtServices && "Runtime services were not initialized!");

  /// Work item analysis pointer
  m_WIA = &getAnalysis<WIAnalysis>();
  V_ASSERT(m_WIA && "Unable to get work item analysis pointer pass");

  /// Create functions which we may use later
  createAllOne(*F.getParent());

  // Create constant values which we use often (one and zero)
  m_one = ConstantInt::get(
    F.getParent()->getContext(), APInt(1,1));
  m_zero = ConstantInt::get(
    F.getParent()->getContext(), APInt(1, 0));

  ++PredicatorCounter;
  Predicated = 0; // statistics
  // Predication is needed only in the presence of divergent branch
  if (needPredication(F)) {
    Predicated++; // statistics
    predicateFunction(&F);
    intel::Statistic::pushFunctionStats(m_kernelStats, F, DEBUG_TYPE);
    return true;
  }
  // Did not change this function.
  intel::Statistic::pushFunctionStats(m_kernelStats, F, DEBUG_TYPE);
  return false;
}

bool Predicator::hasOutsideRandomUsers(Instruction* inst, Loop* loop) {
  /// We do not select-prev values which are
  ///  not inside loops.
  if (!loop) return false;

  /// Do we have users outside of the current BB ?
  /// We check if we have users outside the loop.
  /// Note that we do not care if the user is in a different basic block because
  /// the entire loop will compress into a single BB.
  for (Value::user_iterator it = inst->user_begin(),
       e = inst->user_end(); it != e ; ++it) {
    // Is user instruction ?
    if (Instruction* user = dyn_cast<Instruction>(*it)) {
      // if the user is non-random, then either it is inside the loop,
      // or all the exits of the loop are uniform. In any case,
      // the user can continue to use the original value after predication.
      if (m_WIA->whichDepend(user) != WIAnalysis::RANDOM) {
        continue;
      }
      // Is the parent of this instruction contained in this loop ?
      if (! loop->contains(user->getParent())) {
        V_PRINT(predicate, "has outside user "<<*inst<<"\n");
        return true;
      }
    }
  }
  // no outside users
  return false;
}

Function* Predicator::createSelect(PHINode* phi, Value* mask) {
  V_ASSERT(phi->getNumIncomingValues() == 2 &&
           "Phi node must have only two incoming edges");

  // Create function arguments
  std::vector<Type*> selectArgs;
  selectArgs.push_back(IntegerType::get(
      phi->getParent()->getParent()->getContext(), 1));
  selectArgs.push_back(phi->getIncomingValue(0)->getType());
  selectArgs.push_back(phi->getIncomingValue(1)->getType());

  FunctionType* selectType = FunctionType::get(
    phi->getIncomingValue(0)->getType(), selectArgs, false);

  // Declare function
  Module* currentModule = phi->getParent()->getParent()->getParent();
  Function* func_select = dyn_cast<Function>(
    currentModule->getOrInsertFunction("select", selectType));
  V_ASSERT(func_select && "Function type is incorrect, so dyn_cast failed");

  return func_select;
}

void Predicator::moveAfterLastDependant(Instruction* inst) {
  V_ASSERT(inst && "Unable to schedule null instruction");
  BasicBlock* BB = inst->getParent();
  V_ASSERT(BB && "No parent ?");

  Instruction* last_user = &*BB->begin();
  // Find last user
  for (BasicBlock::iterator it = BasicBlock::iterator(BB->getFirstNonPHI()),
       e=BB->end(); it != e; ++it) {
    Instruction* I = &*it;
    // If this instruction is in our use-chain
    // or if this is a phi-node
    if (std::find(I->user_begin(), I->user_end(), inst) != I->user_end() ||
        dyn_cast<PHINode>(I)) {
      last_user = I;
    }
  }

  //TODO: replace with moveAfter when migrating to LLVM2.8 for all users
  inst->moveBefore(last_user);
  last_user->moveBefore(inst);
}

void Predicator::LinearizeBlock(BasicBlock* block, BasicBlock* next,
                                Loop* loop, BasicBlock* next_after_loop) {

  V_ASSERT(block && "Block must be valid");

  TerminatorInst* term = block->getTerminator();
  V_ASSERT(term && "no terminator ?");
  unsigned term_successors = term->getNumSuccessors();

  // nothing to do for return block
  if (term_successors == 0) return;

  // The control flow of uniform branches in a non divergent blocks should remain
  // as it is
  if (m_WIA->whichDepend(term) == WIAnalysis::UNIFORM && !m_WIA->isDivergentBlock(block))
    return;

  if (!loop) {
    // case where not a loop, a simple branch below
    term->eraseFromParent();
    BranchInst::Create(next, block);
    // we may enter a loop, fix its phi node
    return LinearizeFixPhiNode(next, block);
  }

  //
  // Below we handle loop blocks
  //
  BasicBlock* header = loop->getHeader();

  // unconditional
  if (1 == term_successors) {
    BasicBlock* succ0 = term->getSuccessor(0);
    //If it has no backedge , the block's branch is replaced by an
    //unconditional branch to the next block in the list.
    if (succ0 != header) {
      term->setSuccessor(0, next);
      return  LinearizeFixPhiNode(next, block);
    }
    //If the block only has a backedge, its branch
    //is replaced by a conditional branch with edges to the header and the next
    //block in the list with the exit mask of the block as the branch condition.
    if (succ0 == header) {
      Value* loop_mask_p = m_inMask[loop->getHeader()];
      Value* loop_mask   = new LoadInst(loop_mask_p, "loop_mask", block);
      V_ASSERT(m_allzero && "Unable to find allzero func");
      CallInst *call_allzero =
        CallInst::Create(m_allzero, loop_mask, "leave", block);

      term->eraseFromParent();
      BranchInst::Create(next_after_loop, header, call_allzero, block);
      return LinearizeFixPhiNode(next_after_loop, block);
    }
  }

  // conditional
  if (2 == term_successors) {
    BasicBlock* succ0 = term->getSuccessor(0);
    BasicBlock* succ1 = term->getSuccessor(1);
    //If it has no backedge , the block's branch is replaced by an
    //unconditional branch to the next block in the list.
    if (succ0 != header && succ1 != header) {
      term->eraseFromParent();
      BranchInst::Create(next, block);
      return LinearizeFixPhiNode(next, block);
    }
    // If the block has a conditional branch with a backedge, the other edge
    // that leaves the loop is replaced by an edge to the next block in the
    // list.
    if (succ0 == header) {
      V_ASSERT(!loop->contains(succ1) && "br cant have backedge + internal edge");
      term->setSuccessor(1, next);
    }
    if (succ1 == header) {
      V_ASSERT(!loop->contains(succ0) && "br cant have backedge + internal edge");
      term->setSuccessor(0, next);
    }
    return LinearizeFixPhiNode(next, block);
  }//2 succ

  V_ASSERT(false && "bad number of successors");
}

void Predicator::LinearizeFixPhiNode(BasicBlock* tofix, BasicBlock* src) {

  // For each instruction (which may be PHI)
  for (BasicBlock::iterator it = tofix->begin(),
       e = tofix->end(); it != e; ++it) {

    PHINode* phi = dyn_cast<PHINode>(it);

    // once we hit first non-phi, we can bail out
    // since all phis are at the beginning of basic block
    if (!phi) return;

    LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    Loop* loop = LI->getLoopFor(tofix);

    // For each of the incoming edges to the phi
    unsigned num_entries = phi->getNumIncomingValues();
    for (unsigned i=0; i< num_entries ; ++i) {
      BasicBlock* incoming = phi->getIncomingBlock(i);
      V_ASSERT(incoming && "bad incoming block");
      // if this is not a latch edge, fix it
      if (! (loop && loop->getLoopLatch() == incoming)) {
        phi->setIncomingBlock(i, src);
      }
    }// each phi entry
  }// for each phi
}

void Predicator::registerLoopSchedulingScopes(SchedulingScope& parent,
                                              LoopInfo* LI, Function* F,
                                              DenseMap<BasicBlock*,
                                              SchedulingScope*>& headers) {

  DenseMap<Loop*, SchedulingScope*> scopes;

  // Create a scope for each loop
  for (Function::iterator bbit = F->begin(), bb_e = F->end(); bbit != bb_e ; ++bbit) {
    BasicBlock* bb = &*bbit;
    Loop* loop = LI->getLoopFor(bb);
    // Check if loop is scheduled as UCF region
    if (loop && !isUCFInter(loop->getHeader()) &&
       scopes.find(loop) == scopes.end()) {
      // Add the new scopes to the scopes container. Later add them to the
      // parent scope which will delete them upon destruction.
      scopes[loop] = new SchedulingScope(loop->getHeader());
    }
  }

  // Add all basic blocks to scopes
  // for each BB
  for (Function::iterator BBIt = F->begin(), BBE = F->end(); BBIt != BBE ; ++BBIt) {
    BasicBlock* BB = &*BBIt;
    parent.addBasicBlock(BB);
    // for each loop which we have registered
    for (DenseMap<Loop*, SchedulingScope*>::iterator mapit =
         scopes.begin(), map_e = scopes.end(); mapit != map_e ; ++mapit ) {
      Loop* loop = mapit->first;
      SchedulingScope* scope = mapit->second;

      if (loop && loop->contains(BB)) {
        scope->addBasicBlock(BB);
        headers[loop->getHeader()] = scope;
      }

    }//for
  }

  // add all loop sub-scopes to the main scopes
  for (DenseMap<Loop*, SchedulingScope*>::iterator mapit = scopes.begin(),
       map_e = scopes.end(); mapit != map_e ; ++mapit ) {
    parent.addSubSchedulingScope(mapit->second);
  }
}

void Predicator::addDivergentBranchesSchedConstraints(SchedulingScope& main_scope) {

  // Getting the scheduling constraints calculated during WIA
  SchdConstMap & predSched = m_WIA->getSchedulingConstraints();

  // Going over the constraints and add these as scheduling scopes
  for (SchdConstMap::iterator itr = predSched.begin();
       itr != predSched.end();
       ++itr) {
    assert(itr->second.size() > 1 && "Constraint size should be larger than 1.");

    SchedulingScope *scp = new SchedulingScope(NULL);
    for (std::vector<BasicBlock*>::iterator bbIt = itr->second.begin();
         bbIt !=  itr->second.end();
         ++bbIt) {
      // If a function has two return statements then the immediate post dom of a branch might be null
      if (! (*bbIt)) continue;

      scp->addBasicBlock(*bbIt);
    }

    // register the bypass info restrictions with parent
    main_scope.addSubSchedulingScope(scp);
  }
}

void Predicator::linearizeFunction(Function* F,
                                   FunctionSpecializer& specializer) {

  // Maps loop headers back to the scopes which represents them
  DenseMap<BasicBlock*, SchedulingScope*> headers;

  // global scope which contains the entire function
  // When this scope is destroyed, all sub-scopes will be deleted.
  SchedulingScope main_scope(NULL, true);

  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  // register the scheduling constraints which are derived from loops
  registerLoopSchedulingScopes(main_scope, LI, F, headers);

  // add all linearization scopes to main-scope
  specializer.registerSchedulingScopes(main_scope);

  // add scheduling constraints for the predicated regions
  addDivergentBranchesSchedConstraints(main_scope);

  // register UCF scheduling constraints
  registerUCFSchedulingScopes(main_scope);

  // Perform the actual scheduling of the blocks
  std::vector<BasicBlock*> schedule;
  main_scope.schedule(schedule);

  // Order the basic blocks according to the calculated scheduling
  for (unsigned int i=0 ; i< schedule.size()-1; ++i) {
    BasicBlock* current = schedule.at(i);
    BasicBlock* next = schedule.at(i+1);
    Loop* loop = LI->getLoopFor(current);
    BasicBlock* next_after_loop = next;

    // If we are in a loop, we need to find which basic block
    // comes right after the loop
    if (loop && headers.count(loop->getHeader()) != 0) {
      SchedulingScope* scp = headers[loop->getHeader()];
      next_after_loop = scp->getFirstBlockAfter(schedule);
    } else {
      // This means BB isn't inside a loop or the entire loop is inside UCF region
      // which should not be linearized
      loop = NULL;
    }
    // Preserve control flow inside UCF regions
    if(isUCFInter(current) || isUCFEntry(current))
      continue;

    V_ASSERT(next_after_loop && "nothing comes after this loop");
    V_ASSERT(next      && "nothing comes after this block");
    // Perform the actual linearization of a single basic block
    // This will adjust the outgoing edged of each BB
    LinearizeBlock(current, next, loop, next_after_loop);
  }
}

Value* Predicator::getPhiCond(PHINode* phi, bool& switchValuesOrder) {
  BasicBlock* BB = phi->getParent();
  DomTreeNode* node = m_DT->getNode(BB);
  V_ASSERT(node && "DomTreeNode is nullptr");
  DomTreeNode* idom = node->getIDom();
  if (!idom) {
    V_ASSERT(false && "cannot find immediate dominator");
    return NULL;
  }
  BasicBlock* idomBB = idom->getBlock();
  if (!idomBB) {
    V_ASSERT(false && "cannot find immedaite dominator");
    return NULL;
  }
  // note that the terminator of idomBB may have changed
  // since we started to run the predicator, so we use
  // m_branchesInfo to find the original data.
  if (m_branchesInfo.find(idomBB) == m_branchesInfo.end())
    return NULL;
  Value *cond = m_branchesInfo[idomBB].m_cond;

  // find which side of the branch belongs to which incoming phi value.
  BasicBlock* succ0 = m_branchesInfo[idomBB].m_succ0;
  BasicBlock* succ1 = m_branchesInfo[idomBB].m_succ1;
  bool firstToFirst = (succ0 == phi->getIncomingBlock(0)) || (isReachableInsideIteration(succ0, phi->getIncomingBlock(0))) ||
    (succ0 == BB && phi->getIncomingBlock(0) == idomBB);
  bool firstToSecond = (succ0 == phi->getIncomingBlock(1)) || (isReachableInsideIteration(succ0, phi->getIncomingBlock(1))) ||
    (succ0 == BB && phi->getIncomingBlock(1) == idomBB);
  bool secondToFirst = (succ1 == phi->getIncomingBlock(0)) || (isReachableInsideIteration(succ1, phi->getIncomingBlock(0))) ||
    (succ1 == BB && phi->getIncomingBlock(0) == idomBB);
  bool secondToSecond = (succ1 == phi->getIncomingBlock(1)) || (isReachableInsideIteration(succ1, phi->getIncomingBlock(1))) ||
    (succ1 == BB && phi->getIncomingBlock(1) == idomBB);
  if (firstToFirst && secondToSecond && !firstToSecond && !secondToFirst) {
    // first successor leads to first incoming phi value, second to second.
    switchValuesOrder = false;
    return cond;
  }
  if (firstToSecond && secondToFirst && !firstToFirst && !secondToSecond) {
    // second successor leads to first incoming phi value, first to second.
    switchValuesOrder = true;
    return cond;
  }
  // couldn't determine which side of the condition leads to which value
  // of the phi.
  return NULL;

}

void Predicator::convertPhiToSelect(BasicBlock* BB) {

  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  V_ASSERT(LI && "Unable to get loop analysis");
  Loop* loop = LI->getLoopFor(BB);
  // We do not touch blocks which are loop headers
  // is this the only condition ?
  if (loop && loop->getHeader() == BB) return;

  // Cache all PHI entries which can be converted to select
  SmallInstVector PhiInstrVector;
  for (BasicBlock::iterator it = BB->begin(); it != BB->end(); it++) {
    // for each instruction
    // If it is a PHI node which we need to eliminate
    PHINode *phi = dyn_cast<PHINode>(it);
    // If we have reached the first non-phi, the caching is over
    if (!phi) break;

    V_ASSERT(phi->getNumIncomingValues() < 3
             && "Phi node must have only two or less incoming edges");
    // Collect only 2-entry phi nodes.
    if (phi->getNumIncomingValues() == 2) {
      PhiInstrVector.push_back(&*it);
    }
  }

  // Convert all PHI nodes collected
  for (SmallInstVector::iterator it = PhiInstrVector.begin();
                                 it != PhiInstrVector.end(); it++) {
    PHINode *phi = dyn_cast<PHINode>(*it);
    V_ASSERT(
      m_outMask.find(std::make_pair(phi->getIncomingBlock(0), BB)) !=
      m_outMask.end());

    V_ASSERT(m_inInst.find(BB) != m_inInst.end() && "No in-mask");
    Instruction* place = m_inInst[BB];

    Value* edge_mask_p =
      m_outMask[std::make_pair(phi->getIncomingBlock(0), BB)];

    Instruction* edge_mask = new LoadInst(
      edge_mask_p, "emask", place);

    place->moveBefore(edge_mask);

    // create select instruction
    Value* selectCond = edge_mask;
    unsigned int firstValIndex = 0;
    unsigned int secondValIndex = 1;
    bool switchValuesOrder;
    // if we are able to locate the original condition to be used for the select,
    // it is better to use it and not the mask, for performance reasons.
    // it is especially critical if the phi is uniform,
    // because then the select can remain uniform for the packetizer,
    // while the edge_mask is by definition always random.
    if (Value* cond = getPhiCond(phi, switchValuesOrder)) {
      // found the phi condition, use it for the select.
      selectCond = cond;
      if (switchValuesOrder) {
        firstValIndex = 1;
        secondValIndex = 0;
      }
      // maybe we are creating the first random outside user for cond?
      // if so, better to just use the mask.
      if (Instruction* condInst = dyn_cast<Instruction>(cond)) { // cond is instruction &&
        Loop* condLoop = m_LI->getLoopFor(condInst->getParent());
        if (condLoop &&                                          // cond inside loop &&
          m_WIA->whichDepend(phi) == WIAnalysis::RANDOM  &&    // the phi is RANDOM &&
          !m_outsideUsers.count(condInst) && // cond has no other users &&
          !condLoop->contains(phi->getParent())) { // phi is not inside the same loop
          // just use the mask.
          selectCond = edge_mask;
          firstValIndex = 0;
          secondValIndex = 1;
        }
      }
    }
    SelectInst* select =
      SelectInst::Create(selectCond, phi->getIncomingValue(firstValIndex),
                         phi->getIncomingValue(secondValIndex), "merge", edge_mask);
    m_WIA->setDepend(phi, select);
    VectorizerUtils::SetDebugLocBy(select, phi);

    // Put in a place which satisfies data dependencies.
    // Note, selectOutsideUsedInstructions assumes all non-phi instructions
    // (such as select instructions) are placed after storing the in_mask
    // of the BB. Thus, we move the instrcution after the edge_mask,
    // even if doesn't use it.
    select->setOperand(0, edge_mask);
    moveAfterLastDependant(select);
    select->setOperand(0, selectCond);

    // If this phi is a condition for a branch we need to replace it in
    // m_branchesInfo with the newly created select.
    for (Value::user_iterator it = phi->user_begin(),
	 end = phi->user_end();
	 it != end; ++it) {
      BranchInst* br = dyn_cast<BranchInst>(*it);
      if (!br || !br->isConditional()) {
	continue;
      }
      BasicBlock* usingBB = br->getParent();
      m_branchesInfo[usingBB].m_cond = select;
    }

    phi->replaceAllUsesWith(select);
    phi->eraseFromParent();
    // We may change instructions which we planned on prev-select-ing
    // in here we update the value which we want to prev-select
    if (m_outsideUsers.count(phi)) {
      m_outsideUsers.remove(phi);
      m_outsideUsers.insert(select);
    }
  }
}

Function* Predicator::createPredicatedFunction(Instruction *inst,
                                               Value* pred,
                                               const std::string& name) {
  // If we have created this function in the past, return it
  if (m_externalFunections.find(name) != m_externalFunections.end()){
    return m_externalFunections[name];
  }

  // Create function arguments
  std::vector<Type*> args;
  /// first argument is predicator
  args.push_back(pred->getType());

  /// all other arguments are the arguments of the instruction
  if (CallInst *CI = dyn_cast<CallInst>(inst)){
    for (unsigned j = 0; j < CI->getNumArgOperands(); ++j){
      args.push_back(CI->getArgOperand(j)->getType());
    }
  } else {
    for (User::op_iterator it = inst->op_begin(),
         e = inst->op_end(); it != e ; ++it) {
      args.push_back((*it)->getType());
    }
  }

  // Generate the function declaration. Return type and args.
  FunctionType* type = FunctionType::get( inst->getType(), args, false);

  // Declare function
  Module * currentModule = inst->getParent()->getParent()->getParent();
  Function* f = dyn_cast<Function>(
    currentModule->getOrInsertFunction(name, type));
  V_ASSERT(f && "Function type is incorrect, so dyn_cast failed");
  m_externalFunections[name] = f;
  return f;
}

bool Predicator::isLocalMemoryConsecutiveLoad(Instruction* inst) {
  if (LoadInst* load = dyn_cast<LoadInst>(inst)) {
    if (load->getPointerAddressSpace() == Intel::OpenCL::DeviceBackend::Utils::OCLAddressSpace::Local) {
      if (m_WIA->whichDepend(load->getPointerOperand()) == WIAnalysis::PTR_CONSECUTIVE)
        return true;
    }
  }
  return false;
}

bool Predicator::keepOriginalInstructionAsWell(Instruction* original) {
  if ((isa<StoreInst>(original) || isa<LoadInst>(original)) &&
    m_WIA->whichDepend(original) == WIAnalysis::UNIFORM) {
      Predicated_Uniform_Store_Or_Loads++; // statistics
      return true; // to later unpredicate a uniform store or load
                   // if it is being allzero-bypassed.
  }
  // Local memory inside the kernel is created on the stack,
  // on a padded buffer. Thus such loads can safely be done without mask,
  // if the address is consecutive and the mask is not empty.
  if (isLocalMemoryConsecutiveLoad(original)) {
    Predicated_Consecutive_Local_Memory_Load++; // statistics
    if (!m_hasLocalMemoryArgs) {
      return true;
    }
  }
  if (m_valuableAllOnesBlocks.count(original->getParent())) {
    return true; // for all-ones optimization
  }
  return false;
}

void Predicator::replaceInstructionByPredicatedOne(Instruction* original,
                                                   Instruction* predicated) {
  VectorizerUtils::SetDebugLocBy(predicated, original);
  // need to keep m_predicatedSelect dictionary updated.
  if (m_valuableAllOnesBlocks.count(original->getParent())) {
    for (Value::user_iterator it = original->user_begin(),
       e = original->user_end(); it != e ; ++it) {
         Instruction* inst = dyn_cast<Instruction>(*it);
         if (inst && m_predicatedSelects.count(inst) &&
                       m_predicatedSelects[inst] == original)   {
           m_predicatedSelects[inst] = predicated;
         }
    }
  }

  original->replaceAllUsesWith(predicated);
  // if we are going to duplicate this block and create
  // an allones version, we want to keep for now the original
  // instruction as well, and we will remove it to the duplicated block later.
  if (keepOriginalInstructionAsWell(original))
    m_predicatedToOriginalInst[predicated]=original;
  else
    original->eraseFromParent();
}

Instruction* Predicator::predicateInstruction(Instruction *inst, Value* pred) {
  // Preplace Load with call to function
  if (LoadInst* load = dyn_cast<LoadInst>(inst)) {
    Function* func =
      createPredicatedFunction(load, pred, Mangler::getLoadName(load->getAlignment()));

    // A single parameter (pointer)
    std::vector<Value*> params;
    params.push_back(pred);
    params.push_back(load->getOperand(0));

    CallInst* call =
      CallInst::Create(func, ArrayRef<Value*>(params), "pLoad", inst);
    replaceInstructionByPredicatedOne(load, call);
    return call;
  }

  // Preplace Store with call to function
  if (StoreInst* store = dyn_cast<StoreInst>(inst)) {
    //Get type name
    Function* func =
        createPredicatedFunction(store, pred, Mangler::getStoreName(store->getAlignment()));

    // Parameters (value to store, address)
    std::vector<Value*> params;
    params.push_back(pred);
    params.push_back(store->getOperand(0));
    params.push_back(store->getOperand(1));
    CallInst* call =
      CallInst::Create(func, ArrayRef<Value*>(params), "", inst);
    replaceInstructionByPredicatedOne(store, call);
    return call;
  }

  // Replace function call with masked function call
  if (CallInst* call = dyn_cast<CallInst>(inst)) {
    std::string desc = call->getCalledFunction()->getName().str();
    std::string maskedName = Mangler::mangle(desc);
    //if the predicated is a faked one, we need to create it artificially.
    //Otherwise, we simply import it from the builtin module.
    Function* func;
    if (m_rtServices->isFakedFunction(maskedName))
      func = createPredicatedFunction(call, pred, maskedName);
    else {
      Function* pMaskedfunc = m_rtServices->findInRuntimeModule(maskedName);
      V_ASSERT(pMaskedfunc && "function not found in runtime module");
      Type* pMaskTy = pMaskedfunc->getFunctionType()->getParamType(0);
      pred = CastInst::CreateSExtOrBitCast(pred, pMaskTy, "", call);
      Module* pCurrentModule = call->getParent()->getParent()->getParent();

      using namespace Intel::OpenCL::DeviceBackend;
      func = cast<Function>(CompilationUtils::importFunctionDecl(pCurrentModule,
                                                                pMaskedfunc));
    }
    const FunctionType* pFuncTy = func->getFunctionType();
    std::vector<Value*> params;
    // insert the mask as the first actual parameter
    params.push_back(pred);
    // copy predicator and original parameters list
    for (unsigned j = 0; j < call->getNumArgOperands(); ++j)
      params.push_back(call->getArgOperand(j));
    //we might need to cast struct pointers, if they are not defined in this
    //module
    V_ASSERT(params.size() == pFuncTy->getNumParams() &&
      "parameter number mismatch");
    for (unsigned i=1 ; i<params.size() ; ++i){
      Value* pParami = params[i];
      const Type* pParamTy = pParami->getType();
      if (pParamTy->isPointerTy() && cast<PointerType>(pParamTy)->getElementType()->isStructTy()){
        Type* pTargetTy = pFuncTy->getParamType(i);
        params[i] = VectorizerUtils::getCastedArgIfNeeded(pParami, pTargetTy, call);
      }
    }
    CallInst* pcall =
      CallInst::Create(func, ArrayRef<Value*>(params), "", call);
    //Update new call instruction with calling convention and attributes
    pcall->setCallingConv(call->getCallingConv());
    AttributeSet as;
    AttributeSet callAttr = call->getAttributes();
    for (unsigned int i=0; i < call->getNumArgOperands(); ++i) {
      //Parameter attributes starts with index 1-NumOfParams
      unsigned int idx = i+1;
      //pcall starts with mask argument, skip it when setting original argument attributes.
      as.addAttributes(func->getContext(), 1 + idx, callAttr.getParamAttributes(idx));
    }
    //set function attributes of pcall
    as.addAttributes(func->getContext(), AttributeSet::FunctionIndex, callAttr.getFnAttributes());
    //set return value attributes of pcall
    as.addAttributes(func->getContext(), AttributeSet::ReturnIndex, callAttr.getRetAttributes());
    pcall->setAttributes(as);
    replaceInstructionByPredicatedOne(call, pcall);
    return pcall;
  }

  // Nothing to do for this instruction
  return NULL;
}

static bool isInstructionABeforeB(BasicBlock* BB, Instruction* A, Instruction* B) {
  for (BasicBlock::iterator it = BB->begin(), e = BB->end();
    it != e; ++it) {
    if (&*it == A)
      return true;
    if (&*it == B)
      return false;
  }
  return false;
}

void Predicator::selectOutsideUsedInstructions(Instruction* inst) {

  // Should be done only for divergent blocks
  if (! m_WIA->isDivergentBlock(inst->getParent())) {
    return;
  }

  Value* prev_ptr = new AllocaInst(
    inst->getType(),            // type
    inst->getName() + "_prev",  // name
    &*inst->getParent()->getParent()->getEntryBlock().begin()); // where

  // Get BB predicator
  //V_PRINT(predicate, "select "<<*F<<"\n");
  BasicBlock *BB = inst->getParent();
  V_ASSERT(m_inMask.find(BB) != m_inMask.end() && "BB has no in-mask");
  Value* pred = m_inMask[BB];

  /// Get loop header, latch
  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  V_ASSERT(LI && "Unable to get loop analysis");
  Loop* loop = LI->getLoopFor(BB);
  V_ASSERT(loop && "Unable to find loop, we only predicate in-loop values");

  V_ASSERT (m_inInst.find(BB) != m_inInst.end() &&
            "Where did we save the mask in this BB ?");

  // Load the predicate value and place the select
  // We will place them in the correct place in the next section
  Instruction* predicate  = new LoadInst(pred,"predicate");
  Instruction* prev_value  = new LoadInst(prev_ptr, "prev_value");
  SelectInst* select = SelectInst::Create(predicate, inst, prev_value, "out_sel");
  Instruction* store  = new StoreInst(select,prev_ptr);
  VectorizerUtils::SetDebugLocBy(select, inst);

  // We are predicating a PHINode
  // we can't put the select before we save the mask!
  // We will have to put it after the in-mask is stored
  // however, in case of non-PHI-Node, we can put it before
  // the instruction
  if (dyn_cast<PHINode>(inst)) {
    Instruction* loc = m_inInst[BB];
    // m_inInst can be a terminator, this happens when basic block contains only PhiNodes.
    if(loc->isTerminator()) {
      select->insertBefore(loc);
    } else {
      select->insertAfter(loc);
    }
  } else {
    V_ASSERT(m_inInst.count(BB) && isInstructionABeforeB(BB, m_inInst[BB], inst) &&
      "bug! creating a select before the mask is ready!");
    // make sure the instructions we created are after the original instruction
    select->insertAfter(inst);
  }
  predicate->insertBefore(select);
  prev_value->insertBefore(select);
  store->insertAfter(select);

  // Replace all of the users of the original instruction with the select
  // We only replace instructions which do not belong to the same loop.
  // Instructions which are inside the loop will be predicated with local masks
  // instructions outside the loop need the special masking.
  std::vector<Value*> users(inst->user_begin(), inst->user_end());
  for (Value * userVal : users) {
    // If the user is an instruction
    Instruction* user = dyn_cast<Instruction>(userVal);
    V_ASSERT(user && "a non-instruction user");
    // if the user is non-random, then all the exits of the loop are
    // uniform, and it can safely continue to use the original inst.
    if (m_WIA->whichDepend(user) != WIAnalysis::RANDOM) {
      continue;
    }
    // If the user is in this loop, don't change it.
    if (!loop->contains(user->getParent()) && user != select) {
      // replace only the appropriate value
      V_ASSERT(select != user);
      (user)->replaceUsesOfWith(inst, select);
    }
  }

  // if we are going to create an allone bypass for the basic block
  // that contains inst, we want to map the select into inst.
  // the reason is that in the allone bypass, we can use
  // inst instead of the select instrcution.
  if (m_valuableAllOnesBlocks.count(inst->getParent())) {
    m_predicatedSelects[select] = inst;
  }
}

void Predicator::predicateSideEffectInstructions() {
  // For each instruction which we intended to predicate
  for(SmallInstVector::iterator
      it = m_toPredicate.begin(),
      e = m_toPredicate.end(); it != e ; ++it) {

    BasicBlock* BB = (*it)->getParent();
    V_ASSERT(m_inMask.find(BB) != m_inMask.end() && "BB has no in-mask");
    Value* pred = m_inMask[BB];
    V_ASSERT(pred && "Unable to find predicate");

    V_PRINT(predicate, "F-Predicating "<<**it<<"\n");
    // Load the mask
    Value* load_pred = new LoadInst(pred, "loda_pred", *it);
    // Use the value of the mask to predicate using a function call
    predicateInstruction(*it, load_pred);
  }// for

}

void Predicator::collectInstructionsToPredicate(BasicBlock *BB) {
  // For each BB
  // Obtain loop analysis (are we inside a loop ?)
  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  PostDominatorTreeWrapperPass &PDTPass = getAnalysis<PostDominatorTreeWrapperPass>();
  PostDominatorTree *PDT = &PDTPass.getPostDomTree();

  V_ASSERT(LI && "Unable to get loop analysis");
  Loop* loop = LI->getLoopFor(BB);

  // In case a block post dominates the entry block and is not in a loop
  // then all the instructions within it will be executed once and only once
  // so no need for masking
  BasicBlock &entryBlock = BB->getParent()->getEntryBlock();
  bool shouldMask = !PDT->dominates(BB, &entryBlock) || loop;
  for(BasicBlock::iterator IIt = BB->begin(), e=BB->end(); IIt != e ; ++IIt) {
    Instruction* I = &*IIt;
    // If this is a load/store/call, save it for later
    V_STAT(if (dyn_cast<LoadInst> (I)) m_maskedLoadCtr++;)
    V_STAT(if (dyn_cast<StoreInst> (I)) m_maskedStoreCtr++;)
    V_STAT(if (dyn_cast<CallInst> (I)) m_maskedCallCtr++;)

    if (shouldMask) {
      if ( isa<LoadInst> (I) || isa<StoreInst>(I) ) {
        m_toPredicate.push_back(I);
      }
      else if (CallInst *CI = dyn_cast<CallInst>(I)) {
        std::string funcname = CI->getCalledFunction()->getName().str();
        if (!m_rtServices->hasNoSideEffect(funcname))  {
          m_toPredicate.push_back(I);
        }
      }
    }

    // If this instruction is used outside its BB and the loop is not inside UCF region,
    // save it for later.
    if (loop && !isUCFInter(loop->getHeader()) && hasOutsideRandomUsers(I, loop)) {
      m_outsideUsers.insert(I);
    }
  }
}

void Predicator::maskDummyEntry(BasicBlock *BB) {
  /// Save this as the mask value for this BB
  m_inMask[BB] = new AllocaInst(
    IntegerType::get(BB->getParent()->getContext(), 1),   // type
    BB->getName() + "_in_mask",                           // name
    &*BB->getParent()->getEntryBlock().begin());          // where
}

// Use the incoming mask as the outgoing mask
// Can be either done for a non-conditional branch or for a uniform branch
// in a non-divergent block
void Predicator::maskOutgoing_useIncoming(BasicBlock *BB, BasicBlock* SrcBB) {

  TerminatorInst* term = BB->getTerminator();
  BranchInst* br = dyn_cast<BranchInst>(term);
  V_ASSERT(br && "Unable to handle non branch terminators");

  // Output same as input
  V_ASSERT(m_inMask.find(SrcBB) != m_inMask.end() && "BB has no in-mask");
  Value* incoming = m_inMask[SrcBB];

  for (unsigned i=0; i < br->getNumSuccessors(); ++i)
    m_outMask[std::make_pair(BB, br->getSuccessor(i))] = incoming;
}

bool Predicator::isAlwaysFollowedBy(Loop *L, BasicBlock* exitBlock) {
  BasicBlock* loopHeader = L->getHeader();
  if (loopHeader == exitBlock)
    return true;
  std::vector<BasicBlock *> unTracedBlocks;
  std::set<BasicBlock *> seenBlocks;
  unTracedBlocks.push_back(loopHeader);
  while (!unTracedBlocks.empty())
  {
    BasicBlock* curr = unTracedBlocks.back();
    unTracedBlocks.pop_back();

    TerminatorInst* term = curr->getTerminator();
    BranchInst* br = dyn_cast<BranchInst>(term);
    // if we reached a return instruction not via the exitBlock:
    if (!br)
      continue;

    for (unsigned int i = 0; i < br->getNumSuccessors(); i++)
    {
      BasicBlock* succ = br->getSuccessor(i);
      // if we reached the header again:
      if (succ == loopHeader)
        return false;

      // don't trace through exit block.
      if (succ == exitBlock)
        continue;

      // trace only through blocks inside the loop we haven't seen yet.
      if (!seenBlocks.count(succ) && L->contains(succ))
      {
        seenBlocks.insert(succ);
        unTracedBlocks.push_back(succ);
      }
    }

  }
  // no path from the loop header back to itself without passing through the
  // exit block
  return true;
}

bool Predicator::isReachableInsideIteration(BasicBlock* src, BasicBlock* dst) {
  if (src == dst)
    return true;
  std::vector<BasicBlock *> unTracedBlocks;
  std::set<BasicBlock *> seenBlocks;
  unTracedBlocks.push_back(src);
  while (!unTracedBlocks.empty())
  {
    BasicBlock* curr = unTracedBlocks.back();
    unTracedBlocks.pop_back();

    TerminatorInst* term = curr->getTerminator();
    BranchInst* br = dyn_cast<BranchInst>(term);
    // if we reached a return instruction not via the exitBlock:
    if (!br)
      continue;

    for (unsigned int i = 0; i < br->getNumSuccessors(); i++)
    {
      BasicBlock* succ = br->getSuccessor(i);
      // if this is a backedge, don't trace through it.
      if (Loop* loop = m_LI->getLoopFor(curr)) {
        if (succ == loop->getHeader()) {
          continue;
        }
      }

      // if we found a path from source to dest:
      if (succ == dst)
        return true;

      // trace only through blocks we haven't seen yet.
      if (!seenBlocks.count(succ))
      {
        seenBlocks.insert(succ);
        unTracedBlocks.push_back(succ);
      }
    }

  }
  // couldn't reach destination
  return false;
}

void Predicator::maskOutgoing_loopexit(BasicBlock *BB) {
  V_ASSERT(m_inMask.find(BB) != m_inMask.end() && "BB has no in-mask");

  TerminatorInst* term = BB->getTerminator();
  BranchInst* br = dyn_cast<BranchInst>(term);
  V_ASSERT(br && br->isConditional() && "expected conditional branch");

  // Get the loop for the basic block.
  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  Loop *L = LI->getLoopFor(BB);
  V_ASSERT(L && L->isLoopExiting(BB) && "expected exiting block");

  // Below we handle conditional branches
  Value *cond = br->getCondition();
  Value *notCond = BinaryOperator::CreateNot(cond, "notCond", br);

  /// Decide which edge is the exit edge and which one is the loop-local edge.
  BasicBlock *BBsucc1 = br->getSuccessor(1);
  bool exitFirst = L->contains(BBsucc1);
  unsigned localIndex = exitFirst;
  BasicBlock* BBexit =  br->getSuccessor(1-localIndex);
  BasicBlock* BBlocal = br->getSuccessor(localIndex);
  Value *localCond = exitFirst ? notCond : cond;
  Value *exitCond  = exitFirst ? cond : notCond;

  // Get incoming mask for the block.
  Value* entry_mask_p = m_inMask[BB];
  Value* entry_mask   = new LoadInst(entry_mask_p, "entry_mask", br);

  /// Handles the out mask for the edge inside the loop.
  Value* local_edge_mask_p = new AllocaInst(
    IntegerType::get(BB->getParent()->getContext(), 1),
    BB->getName() + "_to_" + BBlocal->getName()+"_mask",
    &*BB->getParent()->getEntryBlock().begin());
  BinaryOperator* localOutMask = BinaryOperator::Create(Instruction::And,
      entry_mask, localCond , "local_edge", br);
  new StoreInst(localOutMask, local_edge_mask_p, br);
  m_outMask[std::make_pair(BB, BBlocal)] = local_edge_mask_p;

  /// Handles out mask for the exit edge.
  Loop* outestLoop = L;
  while (outestLoop->getParentLoop() && !outestLoop->getParentLoop()->contains(BBexit)) {
    outestLoop = outestLoop->getParentLoop();
  }
  BasicBlock *preHeader = outestLoop->getLoopPreheader();
  Value* who_left_tr = BinaryOperator::Create(Instruction::And,
                                     entry_mask, exitCond , "who_left_tr", br);
  if (L->getExitingBlock()) {
    V_ASSERT(BB == L->getExitingBlock() &&
        "incase there is only one exiting block it should be the current");
    // Incase there is only one exiting block can use the incoming mask
    // of the preheader since all work items entering the loop will exit
    // through this edge.
    V_ASSERT(L == outestLoop &&
      "same block can't be the single exit of a loop, and also exit a higher level loop");
    V_ASSERT(preHeader && m_inMask.count(preHeader) && "no in mask for preheader");
    m_outMask[std::make_pair(BB, BBexit)] = m_inMask[L->getLoopPreheader()];
  } else {
    // The out mask holds whether the current work item ever left the loop
    // using this edge. we zero the exit edge mask when entering the loop (in
    // preheader) and on each iteration we or the mask with the exit condition.
    Value *exit_edge_mask_p = new AllocaInst(
      IntegerType::get(BB->getParent()->getContext(), 1),
      BB->getName() + "_to_" + BBexit->getName()+"_mask",
      &*BB->getParent()->getEntryBlock().begin());
    // Zero the exit edge mask before entering the loop.
    new StoreInst(m_zero, exit_edge_mask_p, preHeader->getTerminator());
    Value* exit_edge_mask = new LoadInst(exit_edge_mask_p,  "exit_mask", br);
    BinaryOperator* who_ever_left_edge = BinaryOperator::Create(Instruction::Or,
                exit_edge_mask, who_left_tr, "ever_left_loop", br);
    new StoreInst(who_ever_left_edge, exit_edge_mask_p, br);
    m_outMask[std::make_pair(BB, BBexit)] = exit_edge_mask_p;
  }

  /// Update loop masks for all the loops that BB is exiting (possible parent loops).
  Value *curLoopMask = NULL;
  bool mostInnerLoop = true;
  do {
    V_ASSERT(m_inMask.count(L->getHeader()) && "header has no in-mask");
    // If this block (BB) is not nested, then its in mask is
    // the same as the loop mask, and the new loop mask
    // is simply the local edge value. (and inMask, localCond).
    Value *newLoopMask = localOutMask;
    Value* loopMask_p = m_inMask[L->getHeader()];
    if (!mostInnerLoop || !isAlwaysFollowedBy(L, BB)) {
      // This exit block is nested, and thus we need to update the loop
      // mask with negation of exit edge.
      Value* who_left_tr_not =
          BinaryOperator::CreateNot(who_left_tr, "who_left_tr_not", br);
      Value* loopMask   = new LoadInst(loopMask_p, "loop_mask", br);
      newLoopMask = BinaryOperator::Create(
      Instruction::And, loopMask, who_left_tr_not, "loop_mask", br);
    }

    if (!curLoopMask) curLoopMask = newLoopMask;
    new StoreInst(newLoopMask, loopMask_p, br);
    L = L->getParentLoop();
    mostInnerLoop = false;
  } while (L && !L->contains(BBexit));

  L = LI->getLoopFor(BB);

  V_ASSERT(L && "Loop L is nullptr");

  // If there is more than on exit block or
  // the branch is not uniform or
  // the branch is nested
  if (!L->getExitingBlock() ||
      m_WIA->whichDepend(br) != WIAnalysis::UNIFORM ||
      !isAlwaysFollowedBy(L, BB)) {
    /// ----  Create the exit condition. When to leave the loop
    V_ASSERT(m_allzero && "Unable to find allzero func");
    CallInst *call_allzero =
        CallInst::Create(m_allzero, curLoopMask, "shouldexit", br);

    // Make sure the exit block is the first successor.
    BranchInst::Create(BBexit, BBlocal, call_allzero, br);
    br->eraseFromParent();
  }
}

void Predicator::maskOutgoing_fork(BasicBlock *BB) {

  TerminatorInst* term = BB->getTerminator();
  BranchInst* br = dyn_cast<BranchInst>(term);
  V_ASSERT(br && "Unable to handle non branch terminators");

  // Below we handle conditional branches
  Value* cond = br->getCondition();
  BasicBlock *BBsucc0 = br->getSuccessor(0);
  BasicBlock *BBsucc1 = br->getSuccessor(1);

  ///
  /// In here we handle simple forking of two basic blocks to non exit blocks.
  //

  /// One side takes the condition as is,
  /// the other uses the negation of the condition
  BinaryOperator* notCond = BinaryOperator::Create(
    Instruction::Xor, cond, m_one, "Mneg", br);

  /// We negate using XOR 1
  Value* mtrue  = new AllocaInst(
    IntegerType::get(BB->getParent()->getContext(), 1),
    BB->getName() + "_to_" + BBsucc0->getName()+"_mask",
    &*BB->getParent()->getEntryBlock().begin());

  Value* mfalse = new AllocaInst(
    IntegerType::get(BB->getParent()->getContext(), 1),
    BB->getName() + "_to_" + BBsucc1->getName()+"_mask",
    &*BB->getParent()->getEntryBlock().begin());

  Value* MFalse, *MTrue;

  if (m_WIA->isDivergentBlock(BB)) {
    V_ASSERT(m_inMask.find(BB) != m_inMask.end() && "BB has no in-mask");
    Value* incoming = m_inMask[BB];
    Value  *l_incoming   = new LoadInst(incoming, "l_inc", br);

    MFalse = BinaryOperator::Create(
      Instruction::And, l_incoming, notCond,
      BB->getName() + "_to_" + BBsucc1->getName(), br);
    MTrue   = BinaryOperator::Create(
      Instruction::And, l_incoming, cond,
      BB->getName() + "_to_" + BBsucc0->getName(), br);
  }
  else {
    MFalse = notCond;
    MTrue = cond;
  }

  // Store the mask into the alloca
  new StoreInst(MFalse, mfalse, br);
  new StoreInst(MTrue, mtrue, br);
  /// Save outgoing edges
  m_outMask[std::make_pair(BB, BBsucc0)] = mtrue;
  m_outMask[std::make_pair(BB, BBsucc1)] = mfalse;
}

void Predicator::collectBranchesInfo(Function* F) {
  for (Function::iterator it = F->begin(), e = F->end(); it != e; ++ it) {
    BasicBlock* BB = &*it;
    TerminatorInst* term = BB->getTerminator();
    BranchInst* br = dyn_cast<BranchInst>(term);
    if (!br || !br->isConditional()) {
      continue;
    }
    Value* cond = br->getCondition();
    BasicBlock* succ0 = br->getSuccessor(0);
    BasicBlock* succ1 = br->getSuccessor(1);
    m_branchesInfo[BB] = BranchInfo(succ0, succ1, cond);
  }
}

void Predicator::collectOptimizedMasks(Function* F,
                                       PostDominatorTree* PDT,
                                       DominatorTree*  DT) {

  // get loop info
  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  // For all blocks in function
  for (Function::iterator x_iter = F->begin(), x_e = F->end(); x_iter != x_e ; ++x_iter) {
    BasicBlock* x = &*x_iter;
    // If we are the header block in the loop
    // Incoming masks for this case are special. Ignore this case.
    Loop* loopX  = LI->getLoopFor(x);
    if (loopX && loopX->getHeader() == x) {
      continue;
    }
    // Skip UCF interior and exit BBs because they all use the UCF entry's mask
    if(isUCFInter(x) || isUCFExit(x)) continue;

    for (Function::iterator y_iter = F->begin(), y_e = F->end(); y_iter != y_e ; ++y_iter) {
      BasicBlock* y = &*y_iter;
      // If we are the header block in the loop
      // Incoming masks for this case are special. Ignore this case.
      Loop* loopY  = LI->getLoopFor(y);
      if (loopY && loopY->getHeader() == y) {
        continue;
      }

      if (loopY != loopX) continue;

      V_PRINT(predicate, x->getName()<<","<< y->getName()<<"share mask\n");
      // Ignore same-block relations
      if (x == y) continue;

      // found an optimization chance
      if (PDT->dominates(x,y) && DT->dominates(y,x)) {
        // If we never set an edge for X
        if (m_optimizedMasks.find(x) == m_optimizedMasks.end()) {
          // Set optimized mask for x.
          m_optimizedMasks[x] = y;
        } else {
          // both this mask and previous mask are okay
          // We need to select the earliest mask.
          // we select the better candidate by looking at dom info
          if (DT->dominates(y, m_optimizedMasks[x])) {
            m_optimizedMasks[x] = y;
          }
        }
      }
    } //x_iter
  } //y_iter

}


bool Predicator::isUCFEntry(BasicBlock* BB) const {
  return m_ucfEntry2Exit.find(BB) != m_ucfEntry2Exit.end();
}

bool Predicator::isUCFInter(BasicBlock* BB) const {
  return m_ucfInter2Entry.find(BB) != m_ucfInter2Entry.end();
}

bool Predicator::isUCFExit(BasicBlock* BB) const {
  return m_ucfExit2Entry.find(BB) != m_ucfExit2Entry.end();
}

BasicBlock * Predicator::getUCFExit(BasicBlock* BB) {
  // Return this BB if it is an exit
  if(isUCFExit(BB))
    return BB;
  // Otherwise try to find UCF entry
  BasicBlock * ucfEntryBB = getUCFEntry(BB);
  if(!ucfEntryBB) return NULL;
  // And map it to UCF exit
  BasicBlock * ucfExitBB = m_ucfEntry2Exit[ucfEntryBB];
  V_ASSERT(ucfExitBB && "corrupted UCF data");
  return ucfExitBB;
}

BasicBlock * Predicator::getUCFEntry(BasicBlock* BB) {
  // Return this BB if it is an entry
  if(m_ucfEntry2Exit.count(BB))
    return BB;
  // Check if this is an interior BB
  std::map<BasicBlock*, BasicBlock*>::iterator findInterIt = m_ucfInter2Entry.find(BB);
  if(findInterIt != m_ucfInter2Entry.end())
    return findInterIt->second;
  // Check if this is an exit BB
  std::map<BasicBlock*, BasicBlock*>::iterator findExitIt = m_ucfExit2Entry.find(BB);
  if(findExitIt != m_ucfExit2Entry.end())
    return findExitIt->second;

  return NULL;
}

static bool isInsideEntryLoop(Loop const * entryLoop, Loop const * interLoop) {
  if(entryLoop == NULL)
    return true;

  while(interLoop) {
    if(entryLoop == interLoop)
      return true;
    else
      interLoop = interLoop->getParentLoop();
  }
  return false;
}

bool Predicator::isUCFRegion(BasicBlock * const entryBB, BasicBlock * const exitBB, LoopInfo * LI) {
  // Go over this region and check if there are non-uniform conditional branches or any of BBs is
  // outside of a loop containing the entry BB.
  Loop const * entryLoop = LI->getLoopFor(entryBB);
  std::set<BasicBlock *> visited;
  visited.insert(exitBB);
  std::queue<BasicBlock *> workqueue;
  workqueue.push(entryBB);

  while(!workqueue.empty()) {
    BasicBlock * currBB = workqueue.front();
    workqueue.pop();
    visited.insert(currBB);

    BranchInst * br = dyn_cast<BranchInst>(currBB->getTerminator());
    V_ASSERT((br || currBB->getTerminator()->getNumSuccessors() < 2) && "unexpected BB terminator");

    if(br && br->isConditional() && m_WIA->whichDepend(br) != WIAnalysis::UNIFORM) {
      return false;
    }
    else if(!isInsideEntryLoop(entryLoop, LI->getLoopFor(currBB))) {
      return false;
    }

    for(succ_iterator succBB = succ_begin(currBB), succEndBB = succ_end(currBB);
          succBB != succEndBB; ++succBB) {
      if(visited.count(*succBB) == 0) workqueue.push(*succBB);
    }
  }
  return true;
}

void Predicator::collectUCFRegions(Function* F, LoopInfo * LI,
                                   PostDominatorTree * PDT,
                                   DominatorTree * DT) {
  // Go over divergent regions identified by WIAnalysys and collect largest UCF regions
  // nested inside divergent regions.
  SchdConstMap & predSched = m_WIA->getSchedulingConstraints();

  for(SchdConstMap::iterator divRegIt = predSched.begin(), divRegEnd = predSched.end();
        divRegIt != divRegEnd; ++divRegIt) {
    // First collect smallest Single Entry Single Exit regions
    // The BB at the front of the scheduling constraints received from WIAnalysis is an entry BB
    // to the main divergent control flow region and the BB at the back is the full join BB (an exit)
    for(std::vector<BasicBlock*>::iterator bbIt = divRegIt->second.begin() + 1, bbEnd = divRegIt->second.end() - 1;
          bbIt != bbEnd; ++bbIt) {
      BasicBlock * entryBB = *bbIt;
      // Skip NULL references to BBs which might be received from the WIAnalysis under some specific
      // circumstances
      if(NULL == entryBB) continue;

      Loop* loop = LI->getLoopFor(entryBB);
      // For simplicity handle loops with one exit block only
      if(loop && loop->getHeader() == entryBB && loop->getUniqueExitBlock()) {
        BasicBlock * preHeaderBB = loop->getLoopPreheader();
        BasicBlock * loopExitBB = loop->getUniqueExitBlock();
        if(LI->getLoopFor(preHeaderBB) == LI->getLoopFor(loopExitBB) && !isUCFEntry(preHeaderBB) &&
           DT->dominates(preHeaderBB, loopExitBB) && PDT->dominates(loopExitBB, preHeaderBB) &&
           isUCFRegion(preHeaderBB, loopExitBB, LI)) {
          m_ucfEntry2Exit[preHeaderBB] = loopExitBB;
          m_ucfExit2Entry[loopExitBB] = preHeaderBB;
          continue;
        }
      }

      // Check if this BB has uniform conditional branch
      BranchInst * br = dyn_cast<BranchInst>(entryBB->getTerminator());
      if(!br || br->isUnconditional() || m_WIA->whichDepend(br) != WIAnalysis::UNIFORM) continue;

      DomTreeNode * postDomNode = PDT->getNode(entryBB);
      V_ASSERT(postDomNode && "postDomNode is nullptr");
      DomTreeNode * iPostDom = postDomNode->getIDom();
      // Skip infinite loops and return BBs
      if(!iPostDom) continue;
      BasicBlock * exitBB = iPostDom->getBlock();
      // Check the entry dominates its exit BB and they are both located inside the same loop
      if(!DT->dominates(entryBB, exitBB) ||
          LI->getLoopFor(entryBB) != LI->getLoopFor(exitBB)) continue;

      /* Check if there is an additional BB(B on picture) that creates loop
       * with entryBB and has path to exit thru the entryBB only.
       *
       *  |---------|
       *  |  entry  |
       *  |---------|
       *      ||
       *      \/
       *  |---------|<=========|---------|
       *  | entryBB |          |    B    |
       *  |---------|=========>|---------|
       *      ||                   /\
       *      \/                   ||
       *  |---------|              ||
       *  | exitBB  |              ||
       *  |---------|              ||
       *      ||                   ||
       *      \/                   ||
       *  |---------|              ||
       *  |    A    |==============||
       *  |---------|
       *      ||
       *      \/
       *  |---------|
       *  |   exit  |
       *  |---------|
      */
      bool isEntryDomedBySuccs = false;
      for(succ_iterator succIt = succ_begin(entryBB),
            succEnd = succ_end(entryBB); succIt != succEnd; ++succIt) {
        if(PDT->dominates(entryBB, *succIt)) {
          isEntryDomedBySuccs = true;
          break;
        }
      }

      if(isEntryDomedBySuccs) continue;

      // Register new UCF region.
      if(isUCFRegion(entryBB, exitBB, LI)) {
        // There must be unique pairs of entry -> exit BBs which might be invalidated by
        // the loop handling above
        if(isUCFExit(exitBB)) {
          // So, if the exit is already registered
          BasicBlock * existingEntry = m_ucfExit2Entry[exitBB];
          V_ASSERT(existingEntry && "entry BB had to be registered along with exit BB");
          // And the already registered entry BB dominates a new entry then drop the new
          // region which is smaller
          if (DT->dominates(existingEntry, entryBB)) {
            continue;
          }
          // Otherwise remove the existing entry and register the new region which is bigger
          else {
            m_ucfEntry2Exit.erase(existingEntry);
          }
        }

        m_ucfEntry2Exit[entryBB] = exitBB;
        m_ucfExit2Entry[exitBB] = entryBB;
      }
    }
  }

  // Concatenate consecutive UCF regions into one region
  for(std::map<BasicBlock*, BasicBlock*>::iterator regionIt = m_ucfEntry2Exit.begin(), regionEnd = m_ucfEntry2Exit.end();
        regionIt != regionEnd;) {
    std::map<BasicBlock*, BasicBlock*>::iterator currRegIt = regionIt++;
    // Check if an exit BB is an entry to another subregion
    BasicBlock * currEntryBB = currRegIt->first;
    BasicBlock * currExitBB = currRegIt->second;

    // Go backward while there is unique predecessor and this predecessor has single successor
    BasicBlock * prevExitBB = currEntryBB;
    while(!isUCFExit(prevExitBB)) {
      BasicBlock * predBB = prevExitBB->getUniquePredecessor();
      if(!predBB || predBB->getTerminator()->getNumSuccessors() != 1) {
        prevExitBB = NULL;
        break;
      }
      prevExitBB = predBB;
    }
    // Check if there is a preceeding UCF region
    if(prevExitBB) {
      BasicBlock * prevEntryBB = m_ucfExit2Entry[prevExitBB];
      V_ASSERT(prevEntryBB && "UCF region data is corrupted");
      // Update new region info
      m_ucfEntry2Exit[prevEntryBB] = currExitBB;
      m_ucfExit2Entry[currExitBB] = prevEntryBB;
      // And erase the outdated (and invalid) data
      m_ucfEntry2Exit.erase(currEntryBB);
      m_ucfExit2Entry.erase(prevExitBB);
    }
  }

  // Collect scheduling constraints and interior BBs of UCF regions
  for(std::map<BasicBlock*, BasicBlock*>::iterator regionIt = m_ucfExit2Entry.begin(), regionEnd = m_ucfExit2Entry.end();
        regionIt != regionEnd;) {
    std::map<BasicBlock*, BasicBlock*>::iterator currRegIt = regionIt++;

    BasicBlock * entryBB = currRegIt->second;
    BasicBlock * exitBB = currRegIt->first;

    std::vector<BasicBlock *> & schRegion = m_ucfSchedulingConstraints[entryBB];
    V_ASSERT(schRegion.empty() && "an initialized scheduling UCF region must be empty");

    std::set<BasicBlock *> visited;
    std::queue<BasicBlock *> workqueue;
    // Go over all BBs starting from an entry BB till its exit BB
    workqueue.push(entryBB);
    visited.insert(exitBB);

    while(!workqueue.empty()) {
      BasicBlock * currBB = workqueue.front();
      workqueue.pop();
      // If there is BB inside the current region which is an entry to anther UCF region
      // then just delete the nested UCF region.
      if(currBB != entryBB && isUCFEntry(currBB)) {
        BasicBlock * const nestedEntryBB = currBB;
        BasicBlock * const nestedExitBB = getUCFExit(currBB);
        V_ASSERT(nestedExitBB && "the exit must be tied till here with the entry");
        // First, check if the iterator isn't going to be invalidated
        if(m_ucfExit2Entry.end() != regionIt &&
           m_ucfExit2Entry.find(nestedExitBB) == regionIt)
          ++regionIt;
        // Clear out nested UCF region
        m_ucfEntry2Exit.erase(nestedEntryBB);
        m_ucfExit2Entry.erase(nestedExitBB);
        m_ucfSchedulingConstraints.erase(nestedEntryBB);
      }
      // Note here the entry BB is temporary became interior BB for itself (at the very first iteration)
      schRegion.push_back(currBB);
      // In case the nested region was handled before its outer region the nested data is overwritten
      m_ucfInter2Entry[currBB] = entryBB;

      for(succ_iterator succIt = succ_begin(currBB), succEnd = succ_end(currBB);
            succIt != succEnd; ++succIt) {
        BasicBlock * interBB = *succIt;
        // Do not cross the exit BB of this region and do not follow backedges
        // inside UCF region.
        if(!visited.insert(interBB).second)
          continue;
        workqueue.push(interBB);
      }
    }
    // Remove the entry BB from the interior set of BBs
    m_ucfInter2Entry.erase(entryBB);
    // The exit BB is always at the back of the scheduling constraints container
    // and the entry BB is at the front
    schRegion.push_back(exitBB);
    V_ASSERT(schRegion.front() == entryBB && "the entry BB of UCF must go first");
  }

  if(m_ucfSchedulingConstraints.size() > 0) {
    Preserved_Uniform_Conrol_Flow_Regions = m_ucfSchedulingConstraints.size();
  }
}

void Predicator::registerUCFSchedulingScopes(SchedulingScope& main_scope) {
  // Go over the UCF constraints and add these as scheduling scopes
  for (SchdConstMap::iterator itr = m_ucfSchedulingConstraints.begin();
       itr != m_ucfSchedulingConstraints.end(); ++itr) {
    V_ASSERT(itr->second.size() >= 2 && "UCF scope must contain at least two BBs, entry and exit");

    SchedulingScope *scp = new SchedulingScope(NULL, true);
    for (std::vector<BasicBlock*>::iterator bbIt = itr->second.begin(), itrEnd = itr->second.end();
         bbIt != itrEnd; ++bbIt) {
      scp->addBasicBlock(*bbIt);
    }
    // Add this UCF scope to the main scope.
    main_scope.addSubSchedulingScope(scp);
  }
}

void Predicator::maskOutgoing(BasicBlock *BB) {
  //V_PRINT(predicate,
  //"Masking Outgoing BasicBlock:"<<BB->getName()<<"\n");

  /// No outgoing edges - nothing to do.
  TerminatorInst* term = BB->getTerminator();
  if (dyn_cast<ReturnInst>(term)) {
    // Nothing to do for return inst.
    return;
  }

  /// From this point, we only handle Branch instructions
  BranchInst* br = dyn_cast<BranchInst>(term);
  V_ASSERT(br && "Unable to handle non return/branch terminators");

  // Implementation of unconditional branches or uniform branches in
  // non divergent blocks or in UCF regions can be easily done
  if (br->isUnconditional() ||
    (m_WIA->whichDepend(br) == WIAnalysis::UNIFORM && !m_WIA->isDivergentBlock(BB)) ||
    isUCFEntry(BB) || isUCFInter(BB)) {
    // If this outgoing mask is an optimized case
    return maskOutgoing_useIncoming(BB, BB);
  }

  /// We will need loop information to know if this BB
  /// has edges leaving the loop
  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  V_ASSERT(LI && "Unable to get analysis");
  // Are the predecessor BBs in the same loop as me?
  Loop* L = LI->getLoopFor(BB);

  if (L && L->isLoopExiting(BB)) {
    /// In here we handle Loop exit
    maskOutgoing_loopexit(BB);
  } else {
    // in-loop split
    maskOutgoing_fork(BB);
  }
}

void Predicator::maskIncoming_optimized(BasicBlock *BB, BasicBlock* pred) {
  /// Where to save the mask
  V_ASSERT(m_inMask.find(BB) != m_inMask.end() && "BB has no in-mask");
  Value* old_in = m_inMask[BB];

  /// The incoming mask of pred
  V_ASSERT(m_inMask.find(pred) != m_inMask.end() && "BB has no in-mask");
  Value* pred_in = m_inMask[pred];

  Instruction* place = BB->getFirstNonPHI();
  LoadInst* ld = new LoadInst(pred_in, "opt", place);
  Instruction* st = new StoreInst(ld, old_in, place);
  m_inInst[BB] = st;
}

void Predicator::maskIncoming_singlePred(BasicBlock *BB, BasicBlock* pred) {
  /// Find the old dummy mask
  V_ASSERT(m_inMask.find(BB) != m_inMask.end() && "BB has no in-mask");
  Value* old_in = m_inMask[BB];

  /// Else, there is only one way to get here.
  /// We shall use this mask as in-mask.
  CFGEdge edge = std::make_pair(pred, BB);
  V_ASSERT(m_outMask.find(edge) != m_outMask.end() && "Edge has no out-mask");
  Value* new_mask = m_outMask[edge]; // edge from inBB to this BB
  V_ASSERT(new_mask && "New mask is NULL");

  Instruction* place = BB->getFirstNonPHI();
  LoadInst* ld = new LoadInst(new_mask, "l_newmask", place);
  Instruction* st = new StoreInst(ld, old_in, place);
  m_inInst[BB] = st;
}

void Predicator::maskIncoming_loopHeader(BasicBlock *BB, BasicBlock* preheader){
  /// Find the old dummy mask
  V_ASSERT(m_inMask.count(BB) && m_inMask.count(preheader) && "no in masks");
  Instruction *loc = preheader->getTerminator();
  Value *preHeadMask =  new LoadInst(m_inMask[preheader], "prehead_mask", loc);
  new StoreInst(preHeadMask, m_inMask[BB], loc);

  // updating of the in mask when exiting the loop is done when dealing with
  // out_masks on edges.
  m_inInst[BB] = BB->getFirstNonPHI();
}

void Predicator::maskIncoming_simpleMerge(BasicBlock *BB) {
  V_ASSERT(std::distance(pred_begin(BB), pred_end(BB)) < 3 &&
           "More than two preds");

  // Nothing to do if this is entry block
  if (pred_begin(BB) == pred_end(BB)) return;

  ///
  /// We are a regular merge BB, like after IF
  ///
  // find the edge which directs to us. Use its mask.
  CFGEdge edge = std::make_pair(*pred_begin(BB), BB);
  V_ASSERT(m_outMask.find(edge) != m_outMask.end() && "Edge has no out-mask");

  // edge from inBB to this BB
  Value* in_mask = m_outMask[edge];
  // Place to insert all new instructions
  Instruction* place = BB->getFirstNonPHI();
  Value  *l_in_mask   = new LoadInst(in_mask, "l_in_mask", place);

  /// Create a big Or of all incoming values.
  for (pred_iterator it = pred_begin(BB), e = pred_end(BB); it != e; ++it) {
    CFGEdge edge = std::make_pair(*it, BB);
    V_ASSERT(m_outMask.find(edge) != m_outMask.end() && "Edge has no out-mask");
    // edge from inBB to this BB
    Value* mask = m_outMask[edge];
    Value  *l_mask   = new LoadInst(mask, "l_mask", place);
    l_in_mask  = BinaryOperator::Create(
      Instruction::Or, l_in_mask, l_mask, BB->getName() + "_Min", place);
  }
  Instruction* st = new StoreInst(l_in_mask, m_inMask[BB], place);
  m_inInst[BB] = st;
}

void Predicator::maskIncoming(BasicBlock *BB) {
  //V_PRINT(predicate,
  // "Masking Incoming BasicBlock:"<<BB->getName()<<"\n");

  // If this is the function entry block set its mask to all ones
  BasicBlock * funcEntryBB = &BB->getParent()->getEntryBlock();
  if (funcEntryBB == BB) {
    ConstantInt* one = ConstantInt::get(
      BB->getParent()->getContext(), APInt(1, StringRef("1"), 10));

    Instruction* st = new StoreInst(one, m_inMask[BB], BB->getFirstNonPHI());
    moveAfterLastDependant(st);
    m_inInst[BB] = st;
    return;
  }

  // If this BB is not divergent use function entry BB's mask
  if(!m_WIA->isDivergentBlock(BB)) {
    maskIncoming_optimized(BB, funcEntryBB);
  }

  // If this is an interior or exit UCF BB then use the
  // UCF entry mask
  if(isUCFInter(BB)) {
    maskIncoming_optimized(BB, m_ucfInter2Entry[BB]);
    return;
  }
  if(isUCFExit(BB)) {
    maskIncoming_optimized(BB, m_ucfExit2Entry[BB]);
    return;
  }

  if (std::distance(pred_begin(BB), pred_end(BB)) < 2) {
    // Get single pred or NULL
    maskIncoming_singlePred(BB, BB->getSinglePredecessor());
    return;
  }

  /// If this is not a simple case,
  /// we will need some loop info.
  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  Loop* loop  = LI->getLoopFor(BB);

  //
  // If we are the header block in the loop
  // This is where the backedge merges.
  if (loop && loop->getHeader() == BB) {
    V_ASSERT(!isUCFExit(BB) && "loop header must not be UCF exit");
    maskIncoming_loopHeader(BB, loop->getLoopPreheader());
    return;
  }

  // If we can optimize the generation of this mask
  if (m_optimizedMasks.find(BB) != m_optimizedMasks.end()) {
    BasicBlock* par = m_optimizedMasks[BB];
    // We can't set the incoming mask based on the
    // outgoing of the same block.
    V_ASSERT(BB != par && "Can't use our own mask");
    return maskIncoming_optimized(BB, par);
  }
  maskIncoming_simpleMerge(BB);
}


bool Predicator::checkCanonicalForm(Function *F, LoopInfo *LI) {
  for( Function::iterator it = F->begin(), e  = F->end(); it != e ; ++it) {
    BasicBlock* BB = &*it;
    // Assert that each block has at most two preds
    V_ASSERT(std::distance(pred_begin(BB), pred_end(BB))<3 && "Phi canon failed");

    /// Verify that the loop is simplified
    Loop* loop = LI->getLoopFor(BB);
    if (loop) {
        V_ASSERT(loop->isLoopSimplifyForm() && "Loop must be in normal form");
    }
  }
  return true;
}

void Predicator::markLoopsThatBeginsWithFullMaskAsZeroBypassed() {
  PostDominatorTree* PDT = &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  for (LoopInfo::iterator it = LI->begin(), e = LI->end();
    it != e; ++ it) { // for each loop
    // if header is divergent but preheader is uniform
    if (m_WIA->isDivergentBlock((*it)->getHeader()) &&
      !m_WIA->isDivergentBlock((*it)->getLoopPreheader())) {
      BasicBlock* header = (*it)->getHeader();
      for (std::vector<BasicBlock*>::const_iterator
        it2 = (*it)->getBlocks().begin(),
        e2 = (*it)->getBlocks().end();
        it2 != e2; ++it2) {
        if (PDT->dominates(*it2, header)) {
          // block will never be executed with a zero mask:
          blockIsBeingZeroBypassed(*it2);
        }
      }
    }
  }
}

void Predicator::predicateFunction(Function *F) {

  //
  // This is the entry point for the algorithm
  // It derives all of the steps of the predication.
  // This is a good start to read the code.
  //

  // This pass runs on multiple functions, clean
  // the data structures between each runs.
  m_inMask.clear();
  m_inInst.clear();
  m_outMask.clear();
  m_toPredicate.clear();
  m_outsideUsers.clear();
  m_optimizedMasks.clear();
  m_valuableAllOnesBlocks.clear();
  m_predicatedToOriginalInst.clear();
  m_predicatedSelects.clear();
  m_branchesInfo.clear();
  m_ucfEntry2Exit.clear();
  m_ucfInter2Entry.clear();
  m_ucfExit2Entry.clear();
  m_ucfSchedulingConstraints.clear();

  // Get Dominator and Post-Dominator analysis passes
  PostDominatorTree* PDT = &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
  DominatorTree* DT      = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  m_DT = DT; // save the dominator tree.
  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  m_LI = LI;
  m_hasLocalMemoryArgs = doFunctionArgumentsContainLocalMem(F);
  V_ASSERT(LI && "Unable to get loop analysis");
  OCLBranchProbability *OBP = &getAnalysis<OCLBranchProbability>();
  assert (OBP && "OpenCL Branch Probability is not available");

  FunctionSpecializer specializer(
    this, F, m_allzero, PDT, DT, LI, m_WIA, OBP);

  V_PRINT(predicate, "Predicating "<<F->getName()<<"\n");

  // collect branches info before the branches are changed.
  collectBranchesInfo(F);

  // Collect UCF regions inside divergent regions.
  // Must be run before specializer.CollectDominanceInfo because it
  // uses the collected information.
  if(PreserveUCF) {
    collectUCFRegions(F, LI, PDT, DT);
  }

  /// Before we begin the predication process we need to collect specialization
  /// information. This is formation is the dominator-frontier properties of
  /// the original graph.
  specializer.CollectDominanceInfo();

  // calculates the allones heuristics. That is, which block should be
  // bypassed by an allones bypass.
  calculateHeuristic(F);

  // Collect efficient mask generation
  if (EnableOptMasks) {
    collectOptimizedMasks(F, PDT, DT);
  }

  V_PRINT(predicate, "Before:"<<*F<<"\n\n\n");

  // collect instructions to predicate and instructions
  // with outside users
  V_STAT(
  V_PRINT(vectorizer_stat, "Predicator Statistics on function "<<F->getName()<<":\n");
  V_PRINT(vectorizer_stat, "======================================================\n");
  m_maskedLoadCtr = 0;
  m_maskedStoreCtr = 0;
  m_maskedCallCtr = 0;
  )

  for( Function::iterator it = F->begin(), e  = F->end(); it != e ; ++it) {
    BasicBlock* BB = &*it;
    if (m_WIA->isDivergentBlock(BB))
      collectInstructionsToPredicate(BB);
  }

  V_STAT(
  V_PRINT(vectorizer_stat, "Discovered "<<m_maskedLoadCtr<<" masked load instructions\n");
  V_PRINT(vectorizer_stat, "Discovered "<<m_maskedStoreCtr<<" masked store instructions\n");
  V_PRINT(vectorizer_stat, "Discovered "<<m_maskedCallCtr<<" masked call instructions\n");
  )

  V_PRINT(predicate,
          "prev-select marked "<<
          m_outsideUsers.size()<<" instructions\n");

  V_ASSERT(checkCanonicalForm(F, LI) && "Function for predication has to be in canonical form");

  /// Place dummy in-masks
  for( Function::iterator it = F->begin(), e  = F->end(); it != e ; ++it) {
    maskDummyEntry(&*it);
  }
  /// Place out-masks
  for (Function::iterator it = F->begin(), e  = F->end(); it != e ; ++it) {
    maskOutgoing(&*it);
  }
  /// Replace dummy in-masks with real in-masks
  for (Function::iterator it = F->begin(), e  = F->end(); it != e ; ++it) {
    maskIncoming(&*it);
  }
  /// Replace all the divergent PHINodes and the PHINodes in
  /// divergent blocks with select instructions
  for (Function::iterator it = F->begin(), e  = F->end(); it != e ; ++it) {
    BasicBlock* BB = &*it;
    // Skip UCF regions except it's entry BB
    if(isUCFInter(BB) || isUCFExit(BB))
      continue;
    if (m_WIA->isDivergentBlock(BB) || m_WIA->isDivergentPhiBlocks(BB))
      convertPhiToSelect(BB);
  }
  /// Place selects on loop-exit-users
  for (SetVector<Instruction*>::iterator
       it = m_outsideUsers.begin(), e=m_outsideUsers.end(); it != e; ++it) {
    selectOutsideUsedInstructions(*it);
  }

  /// 1.replace all side effect instructions with function calls
  /// 2.insert previous-select for instructions which are used outside
  /// the basic block
  predicateSideEffectInstructions();

  // Perform a linearization of the function
  linearizeFunction(F, specializer);

  V_PRINT(predicate, "Specialize:"<<*F<<"\n");

  V_ASSERT(!verifyFunction(*F) && "I broke this module");

  /// Insert all zero bypasses (specialization)
  specializer.specializeFunction();

  markLoopsThatBeginsWithFullMaskAsZeroBypassed();

  // Note it is important to check all-ones only after checking for all-zeroes.
  // If the mask is all-zeroes, then checking for all-ones, then for all-zeroes, and then
  // doing nothing more (=bypassing) will be twice is costly than testing
  // for all-zero first (2 conditional branches instead of 1).
  // If the mask is all-ones, then there is more code to be executed
  // other than the testing of the masks
  // (this code probably contains loads/stores, otherwise the heuristic would not
  // place an all-one bypass), and thus the extra condition is relatively cheap.

  /// Insert all one bypasses
  insertAllOnesBypasses();

  // clear original instructions that were previously kept but eventually unused/
  // (if instruction was kept because it is a uniform load or store, but
  // the block was not zero-bypassed and not allones-bypassed, the instruction
  // should be cleared now. There shouldn't be any such instructions using
  // the current allones heuristics, but in case we ever choose to change it,
  // or that we turn the allones optimization off, this is needed.
  clearRemainingOriginalInstructions();


  V_ASSERT(!verifyFunction(*F) && "I broke this module");

  V_PRINT(predicate, "Final:"<<*F<<"\n");
}

Value* Predicator::getEdgeMask(BasicBlock* A, BasicBlock* B) {
  CFGEdge edge = std::make_pair(A, B);
  if (m_outMask.find(edge) != m_outMask.end()) return m_outMask[edge];
  return NULL;
}

Value* Predicator::getInMask(BasicBlock* block) {
  if (m_inMask.find(block) != m_inMask.end()) return m_inMask[block];
  return NULL;
}

// Used to unpredicate a masked store or load instructions
// with uniform arguments, after it has turned out this block
// is being bypassed by an allzero check. Assumes that the instruction
// is present in m_predicatedToOriginalInst.
// @param inst The instruction to unpredicate.
void Predicator::unpredicateInstruction(Instruction* call) {
  V_ASSERT(isa<CallInst>(call) && "expected a call instrcution");
  V_ASSERT(m_predicatedToOriginalInst.count(call) &&
    "missing predicated to original entry");

  // simply use original instruction.
  Instruction* original = m_predicatedToOriginalInst[call];

   // need to keep m_predicatedSelect dictionary updated.
  for (Value::user_iterator it = call->user_begin(),
    e = call->user_end(); it != e ; ++it) {
    Instruction* user = dyn_cast<Instruction>(*it);
    if (user && m_predicatedSelects.count(user) &&
                  m_predicatedSelects[user] == call)   {
      m_predicatedSelects[user] = original;
    }
  }

  m_predicatedToOriginalInst.erase(call);
  call->replaceAllUsesWith(original);
  call->eraseFromParent();
}

bool Predicator::isMaskedUniformStoreOrLoad(Instruction* inst) {
  CallInst* call = dyn_cast<CallInst>(inst);
  if (!call || !(call->getCalledFunction())) {
    return false;
  }

  if (Mangler::isMangledLoad(call->getCalledFunction()->getName())) {
    V_ASSERT(call->getNumArgOperands() == 2 && "expected 2 arguments");
    if (m_WIA->whichDepend(call->getArgOperand(1)) == WIAnalysis::UNIFORM) {
       V_ASSERT(m_predicatedToOriginalInst.count(call) &&
        "missing predicated to original entry");
      V_ASSERT(isa<LoadInst>(m_predicatedToOriginalInst[call]) && "expected a load");
      // just to be on the safe side:
      if (!isa<LoadInst>(m_predicatedToOriginalInst[call])) {
        return false;
      }
      return true;
    }
    return false;
  }
  if (Mangler::isMangledStore(call->getCalledFunction()->getName())) {
    V_ASSERT(call->getNumArgOperands() == 3 && "expected 3 arguments");
    if (m_WIA->whichDepend(call->getArgOperand(1)) == WIAnalysis::UNIFORM &&
      m_WIA->whichDepend(call->getArgOperand(2)) == WIAnalysis::UNIFORM) {
      V_ASSERT(m_predicatedToOriginalInst.count(call) &&
        "missing predicated to original entry");
      V_ASSERT(isa<StoreInst>(m_predicatedToOriginalInst[call]) && "expected a store");
       // just to be on the safe side:
      if (!isa<StoreInst>(m_predicatedToOriginalInst[call])) {
        return false;
      }
      return true;
    }
  }
  return false;
}

// if a block is being bypassed, we can "unpredicate"
// any load/store instructions with uniform arguments.
// (that is, the only non-uniform argument is the mask).
void Predicator::blockIsBeingZeroBypassed(BasicBlock* BB) {
  // need to duplicate instructions in order to safely iterate over them.
  // (going to remove instructions later)
  std::vector<Instruction*> inBB;
  for (BasicBlock::iterator bbi=BB->begin(), bbe=BB->end();
    bbi != bbe; ++ bbi) {
    Instruction* I = &*bbi;
    if (m_predicatedToOriginalInst.count(I)) {
      inBB.push_back(I);
    }
  }

  for (std::vector<Instruction*>::iterator it = inBB.begin(), e = inBB.end();
    it != e; ++ it) {
    V_ASSERT(m_predicatedToOriginalInst.count(*it) && "missing original inst");
    if (isMaskedUniformStoreOrLoad(*it)) {
      Predicated_Uniform_Store_Or_Loads--; // statistics
      Unpredicated_Uniform_Store_Load++; // statistics
      unpredicateInstruction(*it);
    }
    // Local memory inside the kernel is
    // created on the stack, in a padded buffer. thus such loads can
    // safely be done without mask, if the address is consecutive
    // and the mask is not empty.
    else if (!m_hasLocalMemoryArgs &&
      isLocalMemoryConsecutiveLoad(m_predicatedToOriginalInst[*it])) {
      Unpredicated_Cosecutive_Local_Memory_Load++; // statistics
      Predicated_Consecutive_Local_Memory_Load--; // statistics
      unpredicateInstruction(*it);
    }
  }
}

void Predicator::insertAllOnesBypassesUCFRegion(BasicBlock * const ucfEntryBB) {
  BasicBlock * const ucfExitBB = getUCFExit(ucfEntryBB);
  V_ASSERT(m_inMask.count(ucfEntryBB) && "missing in mask");
  Value* ucfMask = m_inMask[ucfEntryBB];

  ValueToValueMapTy clonesMap;
  // Choose an instruction which the first predicated instruction
  // in this BB or it's terminator. This is crutial for the correct mask handling.
  Instruction * firstPredOrTerm = NULL;
  for(BasicBlock::iterator bbIt = ucfEntryBB->begin(), bbEnd = ucfEntryBB->end();
        bbIt != bbEnd; ++bbIt) {
    firstPredOrTerm = &*bbIt;
    if(m_predicatedToOriginalInst.count(firstPredOrTerm)) break;
  }
  V_ASSERT(firstPredOrTerm && "it is a branch or the 1st predicated inst. in this BB");
  // Split the UCF entry so that all PHI nodes are stay intact. Other instructions go
  // into the second half (which is a brand new BB)
  BasicBlock* allOnesBeginBB = ucfEntryBB;
  // Note that the allOnesBeginBB is actually is UCF entry
  BasicBlock * postEntryBB = SplitBlock(allOnesBeginBB, firstPredOrTerm, m_DT, m_LI);
  m_ucfInter2Entry[postEntryBB] = ucfEntryBB;
  // Split exit BB at the terminator instruction.
  BasicBlock* allOnesEndBB = SplitBlock(ucfExitBB, ucfExitBB->getTerminator(), m_DT, m_LI);
  m_ucfEntry2Exit[ucfEntryBB] = allOnesEndBB;
  m_ucfExit2Entry[allOnesEndBB] = ucfEntryBB;

  // Collect users outside the original UCF region including the allOnesEndBB
  // plus the used values itself
  std::set<Instruction *> origUsedValues;
  SmallVector<Instruction *, 16> outsideUsers;
  std::set<BasicBlock *> originalUCF;
  originalUCF.insert(postEntryBB);
  originalUCF.insert(ucfExitBB);

  SmallVector<BasicBlock *, 8> workqueue;
  workqueue.push_back(postEntryBB);
  workqueue.push_back(ucfExitBB);
  while(workqueue.size() > 0) {
    BasicBlock * ucfOrigBB = workqueue.back();
    workqueue.pop_back();
    // Look for instructions used outside of the original UCF including the new entry and exit BBs
    // (namely allOnesBeginBB and allOnesEndBB) and remember the users and the used values
    for (BasicBlock::iterator ii = ucfOrigBB->begin(), ei = ucfOrigBB->end(); ii != ei; ++ii) {
      Instruction* I = &*ii;
      for(Value::user_iterator useIt = I->user_begin(); useIt != I->user_end(); ++useIt) {
        Instruction * userInst = dyn_cast<Instruction>(*useIt);
        V_ASSERT(userInst && "userInst is not an Instruction");
        BasicBlock * userBB = userInst->getParent();
        if(userInst &&
           (userBB == allOnesEndBB  || userBB == allOnesBeginBB || getUCFEntry(userBB) != ucfEntryBB)) {
          // Found an outside user
          outsideUsers.push_back(userInst);
          origUsedValues.insert(I);
        }
      }
    }

    if(ucfOrigBB == ucfExitBB) continue;
    for (succ_iterator succ = succ_begin(ucfOrigBB), end = succ_end(ucfOrigBB);
        succ != end; ++succ) {
      if(originalUCF.insert(*succ).second) {
        workqueue.push_back(*succ);
      }
    }
  }

  // Clone the entire region
  SmallVector<BasicBlock *, 8> clones;
  for(std::set<BasicBlock *>::iterator ucfIt = originalUCF.begin(), ucfEnd = originalUCF.end();
        ucfIt != ucfEnd; ++ucfIt) {
    BasicBlock * ucfOrigBB = *ucfIt;
    BasicBlock * cloneBB =
      CloneBasicBlock(ucfOrigBB, clonesMap, ".ucf_allones", ucfOrigBB->getParent());
    clonesMap[ucfOrigBB] = cloneBB;
    clones.push_back(cloneBB);
  }

  // Update references inside the clones.
  for(SmallVector<BasicBlock *, 8>::iterator bbIt = clones.begin(), bbEnd = clones.end();
        bbIt != bbEnd; ++bbIt) {
    BasicBlock * cloneBB = *bbIt;
    for (BasicBlock::iterator ii = cloneBB->begin(); ii != cloneBB->end(); ++ii)
      RemapInstruction(&*ii, clonesMap, RF_IgnoreMissingLocals);
  }

  // Create conditional branch to the original and cloned UCF entry BBs
  allOnesBeginBB->getTerminator()->eraseFromParent();
  LoadInst* loadMask = new LoadInst(ucfMask, "ucf_region_mask", allOnesBeginBB);
  CallInst* isAllOnesTest = CallInst::Create(m_allone, loadMask, "isAllOnes", allOnesBeginBB);
  BasicBlock * postEntryCloneBB = cast<BasicBlock>(clonesMap[postEntryBB]);
  BranchInst::Create(postEntryCloneBB, postEntryBB, isAllOnesTest, allOnesBeginBB);

  // Create a phi node for each value used outside of this UCF region (including the allones end BB's terminator)
  // and populate original value to new PHI value mapping
  ValueToValueMapTy outsidersMap;
  for(std::set<Instruction *>::iterator origUsedIt = origUsedValues.begin(), usedEnd = origUsedValues.end();
        origUsedIt != usedEnd; ++origUsedIt) {
    Value * orig = *origUsedIt;
    Value * clone = clonesMap[orig];
    // Create new PHI node
    PHINode * phi = PHINode::Create(orig->getType(), 2, orig->getName() + "ucf_allones", allOnesEndBB->getTerminator());
    phi->addIncoming(orig, ucfExitBB);
    phi->addIncoming(clone, cast<BasicBlock>(clonesMap[ucfExitBB]));
    // And add the mapping
    outsidersMap[orig] = phi;
  }
  outsidersMap[ucfExitBB] = allOnesEndBB;
  // Update refereneces in the outside users
  for(SmallVector<Instruction *, 16>::iterator userIt = outsideUsers.begin(), userEnd = outsideUsers.end();
        userIt != userEnd; ++userIt) {
    RemapInstruction(*userIt, outsidersMap, RF_IgnoreMissingLocals);
  }
  // Unpredicate the cloned UCF region
  for(std::set<BasicBlock *>::iterator ucfIt = originalUCF.begin(), ucfEnd = originalUCF.end();
        ucfIt != ucfEnd; ++ucfIt) {
    BasicBlock * ucfOrigBB = *ucfIt;
    for (BasicBlock::iterator ii = ucfOrigBB->begin(), ei = ucfOrigBB->end(); ii != ei; ++ii) {
      Instruction * const origPred = &*ii;
      if(m_predicatedToOriginalInst.count(origPred)) {
        Instruction *  const origUnpred =  m_predicatedToOriginalInst[origPred];
        V_ASSERT(origUnpred && "broken reference in m_predicatedToOriginalInst");
        Instruction * clonePred = cast<Instruction>(clonesMap[origPred]);
        Instruction * const cloneUnpred = cast<Instruction>(clonesMap[origUnpred]);
        V_ASSERT(clonePred && cloneUnpred && "clones mapping is broken");
        clonePred->replaceAllUsesWith(cloneUnpred);
        clonePred->eraseFromParent();
      }
    }
  }

  return;
}

void Predicator::insertAllOnesBypassesSingleBlockLoopCase(BasicBlock* original) {
  // If this is a divergent loop we are going to replace the original block
  // with the following structure.
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

  // 1. Create Blocks
 BasicBlock* entry = BasicBlock::Create(original->getContext(),
    "allones_entry_"+original->getName(), original->getParent(), original);

 BasicBlock* allOnes = BasicBlock::Create(original->getContext(),
    "allones_"+original->getName(), original->getParent(), original);

 BasicBlock* testAllZeroes = BasicBlock::Create(original->getContext(),
    "test_all_zeroes_"+original->getName(), original->getParent(), original);

 BasicBlock* entry2 = BasicBlock::Create(original->getContext(),
    "predicated_entry_"+original->getName(), original->getParent(), original);

  V_ASSERT(original->getNextNode() && "expecting a next node");
  BasicBlock* exit = BasicBlock::Create(original->getContext(),
    "post_"+original->getName(), original->getParent(), original->getNextNode());

  // 2. Change pre-header of 'original' to point to 'entry' instead.
  std::vector<BasicBlock*> preds(pred_begin(original),pred_end(original));
  for (std::vector<BasicBlock*>::iterator  pred = preds.begin(), e2 = preds.end();
    pred != e2; ++pred) {
      if (*pred == original)
        continue;
      TerminatorInst* term = (*pred)->getTerminator();
      BranchInst* br = dyn_cast<BranchInst>(term);
      V_ASSERT(br && "expected branch");
      for (unsigned int i = 0; i < br->getNumSuccessors(); ++i) {
        if (br->getSuccessor(i) == original) {
          br->setSuccessor(i, entry);
        }
      }
  }

  // 3. At the end of the 'entry' block, test the mask for allones,
  //    and decide accordingly whether to go to allone block
  //    (which is not predicated), or to entry2, leading to original.
  V_ASSERT(m_inMask.count(original) && "missing in mask");
  Value* mask = m_inMask[original];
  LoadInst* loadMask = new LoadInst(mask, "block_mask", entry);
  CallInst* isAllOnes = CallInst::Create(m_allone, loadMask, "isAllOnes", entry);
  BranchInst::Create(allOnes, entry2, isAllOnes, entry);

  // 4. fill allones block with duplicate instructions.
  std::map<Instruction*,Instruction*> originalToAllOnesInst;
  for (BasicBlock::iterator ii = original->begin(), e2 = original->end();
    ii != e2; ++ii) {
      Instruction* inst = &*ii;
      Instruction* clone;
      if (m_predicatedSelects.count(inst)) {
        // no need to duplicate the select, since mask is allones.
        // simply use the value.
        V_ASSERT(originalToAllOnesInst.count(m_predicatedSelects[inst]) &&
          "original inst should have been cloned before reaching the select");
        originalToAllOnesInst[inst] = originalToAllOnesInst[m_predicatedSelects[inst]];
        continue;
      }
      else if (m_predicatedToOriginalInst.count(inst)) {
        // predicated instruction to remain in original.
        // non predicated instruction to be moved into allones.
        Instruction* originalNonPredicatedInst = m_predicatedToOriginalInst[inst];
        clone = originalNonPredicatedInst->clone();
        clone->takeName(originalNonPredicatedInst);
        // playing with fire here: removing the originalNonPredicatedInst
        // while iterating the parent. This should be Ok, as we have not yet reached it.
        originalNonPredicatedInst->eraseFromParent();
        m_predicatedToOriginalInst.erase(inst);
      }
      else {
        // the original instruction remains in the original block,
        // and a copy of it goes into the allones block.
        clone = inst->clone();
      }
      allOnes->getInstList().push_back(clone);

      // handle inblock instruction arguments for the allones block.
      originalToAllOnesInst[inst]=clone;
      for (unsigned int i = 0; i < clone->getNumOperands(); ++i) {
        Value* op = clone->getOperand(i);
        Instruction* opInst = dyn_cast<Instruction>(op);
        if (opInst) {
          if (opInst->getParent() == original) {
            if (originalToAllOnesInst.count(opInst)) {
              clone->replaceUsesOfWith(opInst, originalToAllOnesInst[opInst]);
            }
          }
        }
      }
  }

  // 5. fix phi nodes in allones block.
  for (BasicBlock::iterator ii = allOnes->begin(), e = allOnes->end();
    ii != e; ++ii) {
      PHINode* phi = dyn_cast<PHINode>(ii);
      if (!phi) {
        break;
      }
      V_ASSERT(phi->getNumIncomingValues() == 2 && "expected 2 incoming vals");
      for (unsigned int i = 0; i < phi->getNumIncomingValues(); i++) {
        if (phi->getIncomingBlock(i) == original) {
          phi->setIncomingBlock(i, allOnes);
          Value* val = phi->getIncomingValue(i);
          Instruction* valInst = dyn_cast<Instruction>(val);
          if (!valInst || valInst->getParent() != original) {
            continue;
          }
          V_ASSERT(originalToAllOnesInst.count(valInst) && "missing allones instruction");

          phi->setIncomingValue(i, originalToAllOnesInst[valInst]);
        }
        else {
          phi->setIncomingBlock(i, entry);
        }
      }
  }


  // 6. replace exit condition in the allones block.
  // if the condition is allzero, it should be replaced with allones.
  // but the condition might also be uniform, in which case
  // the condition itself remains unchanged,
  // only the successors must be replaced.
  bool isAllZeroCondition = true; // start by assuming non-uniform.
  TerminatorInst* allOnesTerminator = allOnes->getTerminator();
  BranchInst* allOnesBr = dyn_cast<BranchInst>(allOnesTerminator);
  V_ASSERT(allOnesBr && allOnesBr->isConditional() && "expected conditional branch");
  Value* condition = allOnesBr->getCondition();
  CallInst* callCond = dyn_cast<CallInst>(condition);
  if (!callCond || callCond->getCalledFunction() != m_allzero) {
    // if not an allzero, it must be a uniform condition.
    isAllZeroCondition = false;
    V_ASSERT(m_WIA->whichDepend(original->getTerminator()) == WIAnalysis::UNIFORM
      && "condition must be uniform if not allZero");

    for (unsigned int i = 0; i < allOnesBr->getNumSuccessors(); i++) {
      if (allOnesBr->getSuccessor(i) == original) {
        allOnesBr->setSuccessor(i, allOnes);
      }
      else {
        allOnesBr->setSuccessor(i, testAllZeroes);
      }
    }
  }
  else { // condition is allzero. Replace with allones.
    allOnes->getTerminator()->eraseFromParent();
    V_ASSERT(callCond->getNumUses() == 0 && "expected no users");
    callCond->setCalledFunction(m_allone);
    BranchInst::Create(allOnes, testAllZeroes, callCond, allOnes);
  }

  // 7. fill test all zero
  if (isAllZeroCondition) {
    CallInst* allZeroCall = CallInst::Create(m_allzero, callCond->getArgOperand(0), "isAllZero", testAllZeroes);
    BranchInst::Create(exit, entry2, allZeroCall, testAllZeroes);
  }
  else { // if loop condition is uniform, no need to test for allzeroes
    BranchInst::Create(exit, entry2, m_one, testAllZeroes);
  }

  // 8. Create PHI-Nodes inside entry2 for values that are
  // used in phi-nodes inside original.
  // also fix phi-nodes inside original.
  for (BasicBlock::iterator ii = original->begin(), e = original->end();
    ii != e; ++ii) {
      PHINode* phi = dyn_cast<PHINode>(ii);
      if (!phi) {
        break;
      }
      V_ASSERT(phi->getNumIncomingValues() == 2 && "expected 2 incoming vals");
      for (unsigned int i = 0; i < phi->getNumIncomingValues(); i++) {
        if (phi->getIncomingBlock(i) == original) {
          Value* val = phi->getIncomingValue(i);
          Instruction* valInst = dyn_cast<Instruction>(val);
          Value* newVal;
          if (valInst && originalToAllOnesInst.count(valInst)) {
            newVal = originalToAllOnesInst[valInst];
          }
          else {
            newVal = val;
          }

          PHINode* newPhi = PHINode::Create(phi->getType(), 2, "pred_phi_"+phi->getName(), entry2);
          unsigned int otherIndex = 1-i;
          newPhi->addIncoming(newVal, testAllZeroes);
          newPhi->addIncoming(phi->getIncomingValue(otherIndex), entry);

          phi->setIncomingValue(otherIndex, newPhi);
          phi->setIncomingBlock(otherIndex, entry2);
          break;
        }
      }
  }

  // 9. Put terminator in entry2
  BranchInst::Create(original, entry2);

  // 10. insert phi-nodes at exit to choose between the allones or predicated values.
  for (BasicBlock::iterator ii = original->begin(), e2 = original->end();
    ii != e2; ++ii) {
      Instruction* I = &*ii;
      PHINode* predicationPhi = NULL;
      // changing users, so we need to iterate on copy.
      std::vector<Value*> users(I->user_begin(), I->user_end());
      for (Value * user : users) {
          Instruction* userInst = dyn_cast<Instruction>(user);
          if (userInst) { // if user is an instruction
            if (userInst->getParent() != original) { // user is outside this block
              // the user should use a phi instead.
              if (!predicationPhi) {
                V_ASSERT(originalToAllOnesInst.count(I)
                  && "missing allones version");
                predicationPhi = PHINode::Create(I->getType(), 2,
                  ii->getName()+"_predication_phi", exit);

                predicationPhi->addIncoming(I,original);
                predicationPhi->addIncoming(originalToAllOnesInst[I],testAllZeroes);
              }
              userInst->replaceUsesOfWith(I,predicationPhi);
            }
          }
      }
  }

  // 11. change terminator  in original and put terminator in exit.
  BasicBlock* originalSuccessor = NULL;
  TerminatorInst* originalTerm = original->getTerminator();
  BranchInst* originalBr = dyn_cast<BranchInst>(originalTerm);
  V_ASSERT(originalBr && originalBr->getNumSuccessors() == 2 && "expected conditional branch");
  for (unsigned int i = 0; i < originalBr->getNumSuccessors(); i++) {
    if (originalBr->getSuccessor(i) != original) {
      originalSuccessor = originalBr->getSuccessor(i);
      originalBr->setSuccessor(i, exit);
      break;
    }
  }
  V_ASSERT(originalSuccessor && "missing original succcessor");
  BranchInst::Create(originalSuccessor, exit);



  // 12. change phi-nodes at the successors of exit (which now follow exit
  //    instead of original).
  // there is actually just one successor: originalSuccessor.
  for (BasicBlock::iterator bbi = originalSuccessor->begin(),
    e3 = originalSuccessor->end(); bbi != e3; ++bbi) {
      PHINode* phi = dyn_cast<PHINode>(bbi);
      if (!phi) {
        break;
      }

      for (unsigned int i = 0; i < phi->getNumIncomingValues(); i++) {
        if (phi->getIncomingBlock(i) == original) {
          phi->setIncomingBlock(i, exit);
        }
      }
  }
}


static bool isSingleBlockLoop(BasicBlock* BB) {
  for (pred_iterator it = pred_begin(BB), e = pred_end(BB);
    it != e; ++it) {
      if (*it == BB) {
        return true;
      }
  }
  return false;
}

void Predicator::insertAllOnesBypasses() {
  std::set<BasicBlock *> ucfVisited;

  for (std::set<BasicBlock*>::iterator it = m_valuableAllOnesBlocks.begin(),
    e = m_valuableAllOnesBlocks.end(); it != e; ++it) {
      // for each block to be bypassed
      BasicBlock* original = *it;

      // preliminaries:
      // find last predicated instruction in the block.
      Instruction* lastPredicatedInst = NULL;
      for (BasicBlock::iterator ii = original->begin(), e2 = original->end();
        ii != e2; ++ii) {
          Instruction* I = &*ii;
          if (m_predicatedToOriginalInst.count(I)) {
            lastPredicatedInst = I;
          }
      }
      if (!lastPredicatedInst) {
        // no point duplicating this block if it has no predicated instructions.
        continue;
      }

      // Check if UCF region has been handled already.
      BasicBlock* ucfEntryBB = getUCFEntry(original);
      if(ucfEntryBB && !ucfVisited.insert(ucfEntryBB).second) {
        // This UCF regions has been handled already
        continue;
      }

      // statistics:://////////////////////////////////////
      AllOnes_Bypasses++;
      // if the bypass is due to a consecutive store/load,
      // the counter is decremented earlier.
      // this is better than counting them directly,
      // because non-consecutive store/loads may
      // not lead to actual bypasses at the end due to
      // unpredication of uniform store/loads.
      AllOnes_Bypasses_Due_To_Non_Consecutive_Store_Load++;
      /////////////////////////////////////////////////////

      if(ucfEntryBB) {
        // In case this BB is inside UCF region the modification is pretty strigthforward
        // because the UCF region has a single entry and a single exit. For the simplicity
        // the whole SESE region is unpdredicated.
        insertAllOnesBypassesUCFRegion(ucfEntryBB);
        continue;
      }

      if (isSingleBlockLoop(original)) {
        // we treat loops made of a single block as a special case
        // (which is more efficient)
        insertAllOnesBypassesSingleBlockLoopCase(original);
        continue;
      }

      // we are going to replace the original block, with a structure of four blocks:
      //              entry
      //             .     .
      //            .       .
      //       original    allones
      //            .       .
      //             .     .
      //              exit
      // We will do the following:
      // 1. Create the entry, exit, and allones BBs.
      // 2. Change predecessors of 'original' to point to 'entry' instead.
      // 3. Move and duplicate instructions:
      //     a. instructions before the first predicated instruction of original
      //        are moved to entry.
      //     b. instructions between the first and last predicated instruction of
      //        are cloned to appear both in the original BB and in the
      //        allones BB. the allones BB uses non-predicated instructions
      //        instead of the predicated ones.
      //     c. instructions after the last predicated instruction of original
      //        are moved to exit.
      // 4. Put terminators in original and allones leading into exit.
      // 5. insert phi-nodes at exit to choose between the allones or predicated values.
      // 6. At the end of the 'entry' block, test the mask for allones,
      //    and decide accordingly whether to go to allone block
      //    (which is not predicated), or to original (which is predicated).
      // 7. change phinodes at the successors of exit (which now follow exit
      //    instead of original)

      // 1. Create the entry, exit, and allones BBs.
      BasicBlock*  allOnes = BasicBlock::Create(original->getContext(),
        original->getName() + "_allOnes", original->getParent(), original);

      BasicBlock* entry = BasicBlock::Create(original->getContext(),
        "pre_"+original->getName(), original->getParent(), allOnes);
      V_ASSERT(original->getNextNode() && "expecting a next node");
      BasicBlock* exit = BasicBlock::Create(original->getContext(),
        "post_"+original->getName(), original->getParent(), original->getNextNode());

      // 2. Change predecessors of 'original' to point to 'entry' instead.
      // note that a block can be his own predecessor, so need
      // to make a copy of the preds and run on the copy.
      std::vector<BasicBlock*> preds(pred_begin(original),pred_end(original));
      for (std::vector<BasicBlock*>::iterator  pred = preds.begin(), e2 = preds.end();
        pred != e2; ++pred) {
          TerminatorInst* term = (*pred)->getTerminator();
          BranchInst* br = dyn_cast<BranchInst>(term);
          V_ASSERT(br && "expected branch");
          for (unsigned int i = 0; i < br->getNumSuccessors(); ++i) {
            if (br->getSuccessor(i) == original) {
              br->setSuccessor(i, entry);
            }
          }
      }


      // 3. Move and duplicate instructions
      std::vector<Instruction*> toRemove;
      std::map<Instruction*,Instruction*> originalToAllOnesInst;
      bool beforeFirstPredicatedInstruction = true;
      bool afterLastPredicatedInstruction = false;
      for (BasicBlock::iterator ii = original->begin(), e2 = original->end();
        ii != e2; ++ii) {
          Instruction * I = &*ii;
          if (beforeFirstPredicatedInstruction &&
            m_predicatedToOriginalInst.count(I)) {
            beforeFirstPredicatedInstruction = false;
          }
          Instruction* inst = I;
          Instruction* clone;
          if (beforeFirstPredicatedInstruction) {
            // move into entry.
            clone = inst->clone();
            clone->takeName(inst);
            entry->getInstList().push_back(clone);
            inst->replaceAllUsesWith(clone);
            toRemove.push_back(inst);
            continue;
          }
          else if (afterLastPredicatedInstruction) {
            // move into exit
            clone = inst->clone();
            clone->takeName(inst);
            exit->getInstList().push_back(clone);
            inst->replaceAllUsesWith(clone);
            toRemove.push_back(inst);
            continue;
          }
          else if (m_predicatedToOriginalInst.count(inst)) {
            // predicated instruction to remain in original.
            // non predicated instruction to be moved into allones.
            Instruction* originalNonPredicatedInst = m_predicatedToOriginalInst[inst];
            clone = originalNonPredicatedInst->clone();
            clone->takeName(originalNonPredicatedInst);
            // playing with fire here: removing the originalNonPredicatedInst
            // while iterating the parent. This should be Ok, as we have not yet reached it.
            originalNonPredicatedInst->eraseFromParent();
            m_predicatedToOriginalInst.erase(inst);
            if (inst == lastPredicatedInst) {
              afterLastPredicatedInstruction = true;
            }
          }
          else {
            // the original instruction remains in the original block,
            // and a copy of it goes into the allones block.
            clone = inst->clone();
          }
          allOnes->getInstList().push_back(clone);

          // handle inblock instruction arguments for the allones block.
          originalToAllOnesInst[inst]=clone;
          for (unsigned int i = 0; i < clone->getNumOperands(); ++i) {
            Value* op = clone->getOperand(i);
            Instruction* opInst = dyn_cast<Instruction>(op);
            if (opInst) {
              if (opInst->getParent() == original) {
                V_ASSERT(originalToAllOnesInst.count(opInst) && "missing allones");
                clone->replaceUsesOfWith(opInst, originalToAllOnesInst[opInst]);
              }
            }
          }
      }
      V_ASSERT(!beforeFirstPredicatedInstruction && afterLastPredicatedInstruction
        && "loop didn't finished as expected");

      for (std::vector<Instruction*>::iterator
        toRem = toRemove.begin(), e2 = toRemove.end(); toRem != e2; ++toRem) {
          (*toRem)->eraseFromParent();
      }

      // 4. Put terminators in original and allones leading into exit.
      BranchInst::Create(exit, allOnes);
      BranchInst::Create(exit, original);

      // 5. insert phi-nodes at exit to choose between the allones or predicated values.
      Instruction* firstInExitBlock = &*exit->begin();
      for (BasicBlock::iterator ii = original->begin(), e2 = original->end();
        ii != e2; ++ii) {
          Instruction* I = &*ii;
          PHINode* predicationPhi = NULL;
          // changing users, so we need to iterate on copy.
          std::vector<Value*> users(I->user_begin(), I->user_end());
          for (Value * user : users) {
              Instruction* userInst = dyn_cast<Instruction>(user);
              if (userInst) { // if user is an instruction
                if (userInst->getParent() != original) { // user is outside this block
                  // the user should use a phi instead.
                  if (!predicationPhi) {
                    V_ASSERT(originalToAllOnesInst.count(I)
                             && "missing allones version");
                    predicationPhi = PHINode::Create(I->getType(), 2,
                      I->getName()+"_predication_phi", firstInExitBlock);

                    predicationPhi->addIncoming(I,original);
                    predicationPhi->addIncoming(originalToAllOnesInst[I],allOnes);
                  }
                  userInst->replaceUsesOfWith(I,predicationPhi);
                }
              }
          }
      }

      // 6. At the end of the 'entry' block, test the mask for allones,
      //    and decide accordingly whether to go to allone block
      //    (which is not predicated), or to original (which is predicated).
      V_ASSERT(m_inMask.count(original) && "missing in mask");
      Value* mask = m_inMask[original];
      LoadInst* loadMask = new LoadInst(mask, "block_mask", entry);
      CallInst* isAllOnes = CallInst::Create(m_allone, loadMask, "isAllOnes", entry);
      BranchInst::Create(allOnes, original, isAllOnes, entry);

      // 7. change phi-nodes at the successors of exit (which now follow exit
      //    instead of original)
      for (succ_iterator succ = succ_begin(exit), e2 = succ_end(exit);
        succ != e2; ++succ) {
          for (BasicBlock::iterator bbi = succ->begin(), e3 = succ->end();
            bbi != e3; ++bbi) {
              PHINode* phi = dyn_cast<PHINode>(bbi);
              if (!phi) {
                break;
              }

              for (unsigned int i = 0; i < phi->getNumIncomingValues(); i++) {
                if (phi->getIncomingBlock(i) == original) {
                  phi->setIncomingBlock(i, exit);
                }
              }

          }
      }

    }
}

void Predicator::clearRemainingOriginalInstructions() {
  for (std::map<Instruction*,Instruction*>::iterator
    it = m_predicatedToOriginalInst.begin(),
    e = m_predicatedToOriginalInst.end();
    it != e; ++ it) {
    it->second->eraseFromParent();
  }
}

// returns the terminator of BB if its a branch conditional on allones.
// otherwise returns NULL.
BranchInst* Predicator::getAllOnesBranch(BasicBlock* BB) {
  TerminatorInst* term = BB->getTerminator();
  V_ASSERT(term && "terminator cannot be null");
  BranchInst* br = dyn_cast<BranchInst>(term);
  if (!br) return NULL;

  if (br->isConditional()) {
    Value* cond = br->getCondition();
    CallInst* condCall = dyn_cast<CallInst>(cond);
    if (condCall && condCall->getCalledFunction()) {
      StringRef name = condCall->getCalledFunction()->getName();
      if (Mangler::isAllOne(name))
        return br;
    }
  }
  return NULL;
}

// recursively (by checking type of predecessors) gets the allones block type.
static Predicator::AllOnesBlockType getAllOnesBlockTypeRec(BasicBlock* BB, int recursionLevel) {
  // allones blocks doesn't contain loops other than single block loops.
  // if recursion level is high, then we are in a loop, so
  // this is not an allones blcok.
  if (recursionLevel > MAX_NUMBER_OF_BLOCKS_IN_AN_ALLONES_BYPASS+1) {
    return Predicator::NONE;
  }
  bool isBlockALoop = isSingleBlockLoop(BB);
  BranchInst* allOnesBranch = Predicator::getAllOnesBranch(BB);
  // 1. first handle blocks that end with an allones branch.
  if (allOnesBranch) {
    if (isBlockALoop) {
      // note a block could also be SINGLE_BLOCK_LOOP_ALLONES
      // without terminating in an allones branch. (could be a uniform branch)
      return Predicator::SINGLE_BLOCK_LOOP_ALLONES;
    }
    else for (unsigned int i = 0; i < allOnesBranch->getNumSuccessors(); i++) {
      if (isSingleBlockLoop(allOnesBranch->getSuccessor(i))) {
        return Predicator::SINGLE_BLOCK_LOOP_ENTRY;
      }
    }
    return Predicator::ENTRY;
  }

  // 2. then handle blocks that are a self-loop.
  if (isBlockALoop) {
    for (pred_iterator it = pred_begin(BB), e = pred_end(BB); it != e; ++it) {
      if (*it != BB) {
        Predicator::AllOnesBlockType predType = getAllOnesBlockTypeRec(*it, recursionLevel+1);
        if (predType == Predicator::SINGLE_BLOCK_LOOP_ENTRY_TO_ORIGINAL) {
          return Predicator::SINGLE_BLOCK_LOOP_ORIGINAL;
        }
        else if (predType == Predicator::SINGLE_BLOCK_LOOP_ENTRY) {
          return Predicator::SINGLE_BLOCK_LOOP_ALLONES;
        }
      }
    }
    return Predicator::NONE;
  }

  // 3. then handle all the rest.
  for (pred_iterator it = pred_begin(BB), e = pred_end(BB); it != e; ++it) {
    V_ASSERT(*it != BB && "BB shouldn't be a self-loop!");
    Predicator::AllOnesBlockType predType = getAllOnesBlockTypeRec(*it, recursionLevel+1);
    if (predType == Predicator::NONE) {
      return Predicator::NONE;
    }
    else if (predType == Predicator::ENTRY) {
      // either all ones or original.
      BranchInst* entryBranch = Predicator::getAllOnesBranch(*it);
      V_ASSERT(entryBranch && "expected a valid allones branch in entry");
      V_ASSERT(entryBranch->getNumOperands() >= 3 &&
        "not enough operands for allones");
      if (entryBranch->getOperand(2) == BB) return Predicator::ALLONES;
      V_ASSERT((entryBranch->getOperand(1) == BB) && "should be this block");
      return Predicator::ORIGINAL;
    }
    else if (predType == Predicator::ORIGINAL ||
      predType == Predicator::ALLONES) {
        return Predicator::EXIT;
    }
    else if (predType == Predicator::EXIT) {
      return Predicator::NONE;
    }
    else if (predType == Predicator::SINGLE_BLOCK_LOOP_ENTRY) {
      // could not be SINGLE_BLOCK_LOOP_ALLONES, because that
      // is already handled when handling self-loop blocks.
      return Predicator::SINGLE_BLOCK_LOOP_ENTRY_TO_ORIGINAL;
    }
    else if (predType == Predicator::SINGLE_BLOCK_LOOP_ALLONES) {
      // could not be SINGLE_BLOCK_LOOP_ALLONES, because that
      // is already handled when handling self-loop blocks.
      return Predicator::SINGLE_BLOCK_LOOP_TEST_ALLZEROES;
    }
    else if (predType == Predicator::SINGLE_BLOCK_LOOP_ORIGINAL) {
      // could not be SINGLE_BLOCK_LOOP_ORIGINAL, because that
      // is already handled when handling self-loop blocks.
      return  Predicator::SINGLE_BLOCK_LOOP_EXIT;
    }
    else if (predType == Predicator::SINGLE_BLOCK_LOOP_EXIT) {
      return Predicator::NONE;
    }
    V_ASSERT(predType !=
     Predicator::SINGLE_BLOCK_LOOP_ENTRY_TO_ORIGINAL &&
     "original should have been caught at self-loop blocks");

    // if pred is Predicator::SINGLE_BLOCK_LOOP_TEST_ALLZEROES,
    // then two options for type, we will find out using the other predecessor.
  }
  return Predicator::NONE;
}

Predicator::AllOnesBlockType Predicator::getAllOnesBlockType(BasicBlock* BB) {
  return getAllOnesBlockTypeRec(BB, 0);
}

// assumes it gets a SINGLE_BLOCK_LOOP_ORIGINAL block,
// and returns its SIGLE_BLOCK_LOOP_ALLONES twin.
BasicBlock* Predicator::getAllOnesSingleLoopBlock(BasicBlock* originalSingleLoop) {
  V_ASSERT(getAllOnesBlockType(originalSingleLoop) ==
    Predicator::SINGLE_BLOCK_LOOP_ORIGINAL &&
    "expected original single block loop");

  pred_iterator pred_it = pred_begin(originalSingleLoop);
  V_ASSERT(pred_it != pred_end(originalSingleLoop) &&
    "expected to find entry to original");
  BasicBlock* entryToOriginal = *pred_it;
  V_ASSERT(Predicator::getAllOnesBlockType(entryToOriginal) ==
    Predicator::SINGLE_BLOCK_LOOP_ENTRY_TO_ORIGINAL &&
    "block type misfit");

  for (pred_iterator pred_it2 = pred_begin(entryToOriginal),
    pred_e = pred_end(entryToOriginal);
    pred_it2 != pred_e; ++pred_it2) {
      BasicBlock* pred2 = *pred_it2;
      if (Predicator::getAllOnesBlockType(pred2) ==
        Predicator::SINGLE_BLOCK_LOOP_ENTRY) {
          // go over successors of the entry.
          for (succ_iterator succ_it = succ_begin(pred2),
            succ_e = succ_end(pred2);
            succ_it != succ_e; ++succ_it) {
              if (getAllOnesBlockType(*succ_it)
                == Predicator::SINGLE_BLOCK_LOOP_ALLONES) {
                  return *succ_it;
              }
          }
      }
  }

  V_ASSERT(false && "couldn't find single block loop allones");
  return originalSingleLoop;
}

// assumes it gets an ORIGINAL,
// and returns its ENTRY predecessor.
BasicBlock* Predicator::getEntryBlockFromOriginal(BasicBlock* original) {
  V_ASSERT(getAllOnesBlockType(original) ==
    Predicator::ORIGINAL && "expected original block type");

  pred_iterator pred_it = pred_begin(original);
  V_ASSERT(pred_it != pred_end(original) && "expected to find entry");
  BasicBlock* entry = *pred_it;
  V_ASSERT(getAllOnesBlockType(entry) ==
    Predicator::ENTRY && "block type misfit");

  return entry;
}

// assumes it gets a SINGLE_BLOCK_LOOP_ORIGINAL block,
// and returns the corresponding SINGLE_BLOCK_LOOP_ENTRY.
BasicBlock* Predicator::getEntryBlockFromLoopOriginal(BasicBlock* loopOriginal) {
  V_ASSERT(getAllOnesBlockType(loopOriginal) ==
    Predicator::SINGLE_BLOCK_LOOP_ORIGINAL &&
    "expected original single block loop");

  pred_iterator pred_it = pred_begin(loopOriginal);
  V_ASSERT(pred_it != pred_end(loopOriginal) && "expected to find entry to original");
  BasicBlock* entryToOriginal = *pred_it;
  V_ASSERT(getAllOnesBlockType(entryToOriginal) ==
    Predicator::SINGLE_BLOCK_LOOP_ENTRY_TO_ORIGINAL &&
    "block type misfit");

  for (pred_iterator pred_it2 = pred_begin(entryToOriginal),
    pred_e = pred_end(entryToOriginal);
    pred_it2 != pred_e; ++pred_it2) {
      BasicBlock* pred2 = *pred_it2;
      if (getAllOnesBlockType(pred2) ==
        Predicator::SINGLE_BLOCK_LOOP_ENTRY) {
          return pred2;
      }
  }
  V_ASSERT(false && "failed to find entry");
  return loopOriginal;
}



bool Predicator::blockHasLoadStore(BasicBlock* BB) {
  bool hasNonConsecutiveStoreLoad = false;
  for (BasicBlock::iterator it = BB->begin(), e = BB->end();
    it != e; ++it) { // for each instruction
    // (this test can be much simpler, but we also
    // want to gather statistics about
    // number of allones bypasses due to non-consecutive
    // store/loads.
    if (LoadInst* load = dyn_cast<LoadInst>(it)) {
      OCLSTAT_GATHER_CHECK(
        Value* operand = load->getPointerOperand();
        WIAnalysis::WIDependancy dep = m_WIA->whichDepend(operand);
        if (dep == WIAnalysis::PTR_CONSECUTIVE) {
          AllOnes_Bypasses_Due_To_Non_Consecutive_Store_Load--; // statistics
          return true;
        }
        else {
          hasNonConsecutiveStoreLoad = true;
        }
        continue;
      );
      return true;
    }
    if (StoreInst* store = dyn_cast<StoreInst>(it)) {
      OCLSTAT_GATHER_CHECK(
        Value* operand = store->getPointerOperand();
        WIAnalysis::WIDependancy dep = m_WIA->whichDepend(operand);
        if (dep == WIAnalysis::PTR_CONSECUTIVE) {
          AllOnes_Bypasses_Due_To_Non_Consecutive_Store_Load--; // statistics
          return true;
        }
        else {
          hasNonConsecutiveStoreLoad = true;
        }
        continue;
      );
      return true;
    }
  }

  return hasNonConsecutiveStoreLoad;
}

void Predicator::calculateHeuristic(Function* F) {
#ifndef NDEBUG
  // if desired, turn off allones by not marking any blocks to be bypassed.
  if (getenv("all-ones-off") || getenv("all_ones_off")) {
    return;
  }
#endif
  if (!EnableAllOnes)
    return;
  // the heuristics is a simple check if a divergent block has loads / stores.
  // this proved to be better than several more sophisticated alternatives.
  for (Function::iterator it = F->begin(), e = F->end(); it != e; ++it) {
    BasicBlock* BB = &*it;
    if (m_WIA->isDivergentBlock(BB) && blockHasLoadStore(BB)) {
      m_valuableAllOnesBlocks.insert(BB);
    }
  }
}




} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  FunctionPass* createPredicator() {
    return new intel::Predicator();
  }
}
