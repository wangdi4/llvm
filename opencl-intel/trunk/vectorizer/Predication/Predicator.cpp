#define DEBUG_TYPE "predicate"
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Support/CFG.h"
#include "llvm/Module.h"
#include "llvm/GlobalVariable.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "Specializer.h"
#include "Predicator.h"
#include "Linearizer.h"
#include "Mangler.h"
#include "Logger.h"

static cl::opt<bool>
EnableOptMasks("optmasks", cl::init(true), cl::Hidden,
  cl::desc("Optimize masks generation"));

// Invoked by runOnModule for every predicated function
STATISTIC(PredicatorCounter, "Counts number of functions visited");

namespace intel {

void Predicator::createAllOne(Module &M) {
  // Create function arguments
  std::vector<const Type*> allOneArgs;
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

  WIAnalysis* WIA = &getAnalysis<WIAnalysis>();
  V_ASSERT(WIA && "Unable to get pass");

  /// Place out-masks
  for (Function::iterator it = F.begin(), e  = F.end(); it != e ; ++it) {
    if (dyn_cast<ReturnInst>(it->getTerminator())) continue;
    WIAnalysis::WIDependancy dep = WIA->whichDepend(it->getTerminator());
    if (dep != WIAnalysis::UNIFORM) {
      V_PRINT("predicate", F.getName()<< "needs predication because of "<<it->getName()<<" \n");
      return true;
    }
  }

  V_PRINT("predicate", F.getName()<< "does not need predication \n");
  return false;
}

bool Predicator::runOnFunction(Function &F) {

  /// Create functions which we may use later
  createAllOne(*F.getParent());

  // Create constant values which we use often (one and zero)
  m_one = ConstantInt::get(
    F.getParent()->getContext(), APInt(1,1));
  m_zero = ConstantInt::get(
    F.getParent()->getContext(), APInt(1, 0));

  ++PredicatorCounter;

  // Only predicate if this function needs masking
  if (needPredication(F)) {
    predicateFunction(&F);
    return true;
  }
  // Did not change this function.
  return false;
}

bool Predicator::hasOutsideUsers(Instruction* inst, Loop* loop) {
  /// We do not select-prev values which are
  ///  not inside loops.
  if (!loop) return false;

  /// Do we have users outside of the current BB ?
  /// We check if we have users outside the loop.
  /// Note that we do not care if the user is in a different basic block because
  //  the entire loop will compress into a single BB.
  for (Value::use_iterator it = inst->use_begin(),
       e = inst->use_end(); it != e ; ++it) {
    // Is user instruction ?
    if (Instruction* user = dyn_cast<Instruction>(*it)) {
      // Is the parent of this instruction contained in this loop ?
      if (! loop->contains(user->getParent())) {
        V_PRINT("predicate", "has outside user "<<*inst<<"\n");
        return true;
      }
    }
  }
  // no outside userss
  return false;
}

Function* Predicator::createSelect(PHINode* phi, Value* mask) {
  V_ASSERT(phi->getNumIncomingValues() == 2 &&
           "Phi node must have only two incoming edges");

  // Create function arguments
  std::vector<const Type*> selectArgs;
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

  Instruction* last_user = BB->begin();
  // Find last user
  for (BasicBlock::iterator it = BB->getFirstNonPHI(),
       e=BB->end(); it != e; ++it) {
    // If this instruction is in our use-chain
    // or if this is a phi-node
    if (std::find(it->use_begin(), it->use_end(), inst) != it->use_end() ||
        dyn_cast<PHINode>(it)) {
      last_user = it;
    }
  }

  //TODO: replace with moveAfter when migrating to LLVM2.8 for all users
  inst->moveBefore(last_user);
  last_user->moveBefore(inst);
}

void Predicator::LinearizeBlock(
  BasicBlock* block, BasicBlock* next, BasicBlock* next_after_loop) {

  V_ASSERT(block && "Block must be valid");
  LoopInfo *LI = &getAnalysis<LoopInfo>();
  Loop* loop = LI->getLoopFor(block);

  TerminatorInst* term = block->getTerminator();
  V_ASSERT(term && "no terminator ?");
  unsigned term_successors = term->getNumSuccessors();

  // nothing to do for return block
  if (term_successors == 0) return;

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
      Value* loop_mask_p = m_loopMask[loop->getHeader()];
      Value* loop_mask   = new LoadInst(loop_mask_p, "loop_mask", block);
      V_ASSERT(m_allone && "Unable to find allone func");
      CallInst *call_allone =
        CallInst::Create(m_allone, loop_mask, "leave", block);

      term->eraseFromParent();
      BranchInst::Create(next_after_loop, header, call_allone, block);
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
      return;
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

    LoopInfo *LI = &getAnalysis<LoopInfo>();
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
  for (Function::iterator bb = F->begin(), bb_e = F->end(); bb != bb_e ; ++bb) {
    Loop* loop = LI->getLoopFor(bb);
    if (loop && scopes.find(loop) == scopes.end()) {
      // Add the new scopes to the scopes container. Later add them to the
      // parent scope which will delete them upon destruction.
      scopes[loop] = new SchedulingScope(loop->getHeader());
    }
  }

  // Add all basic blocks to scopes
  // for each BB
  for (Function::iterator BB = F->begin(), BBE = F->end(); BB != BBE ; ++BB) {
    parent.addBasicBlock(BB);
    // for each loop which we have registerd
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

  // add all loop sub-scopes to the main scops
  for (DenseMap<Loop*, SchedulingScope*>::iterator mapit = scopes.begin(),
       map_e = scopes.end(); mapit != map_e ; ++mapit ) {
    parent.addSubSchedulingScope(mapit->second);
  }
}

void Predicator::linearizeFunction(Function* F,
                                   FunctionSpecializer& specializer) {

  // Maps loop headers back to ths scops which represents them
  DenseMap<BasicBlock*, SchedulingScope*> headers;

  // global scope which contains the entire function
  // When this scope is destroied, all sub-scopes will be deleted.
  SchedulingScope main_scope(NULL);

  LoopInfo *LI = &getAnalysis<LoopInfo>();
  // register the scheduling constraints which are derived from loops
  registerLoopSchedulingScopes(main_scope, LI, F, headers);

  // add all linearization scopes to main-scope
  specializer.registerSchedulingScopes(main_scope);

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
    if (loop) {
      V_ASSERT(headers.find(loop->getHeader()) != headers.end() &&
               "Unable to find loop");

      SchedulingScope* scp = headers[loop->getHeader()];
      next_after_loop = scp->getFirstBlockAfter(schedule);
    }

    V_ASSERT(next_after_loop && "nothing comes after this loop");
    V_ASSERT(next      && "nothing comes after this block");
    // Perform the actual linearization of a single basic block
    // This will adjust the outgoing esged of each BB
    LinearizeBlock(current, next, next_after_loop);
  }
}


void Predicator::convertPhiToSelect(BasicBlock* BB) {

  LoopInfo *LI = &getAnalysis<LoopInfo>();
  V_ASSERT(LI && "Unable to get loop analysis");
  Loop* loop = LI->getLoopFor(BB);
  // We do not touch blocks which are loop heaers
  // is this the only condition ?
  if (loop && loop->getHeader() == BB) return;

  bool changed = true;
  // in here we loop over all instructions in a while
  // because the iterator does not allow us to change
  // the instruction while iterating
  while (changed) {
    changed = false;
    // for each instruction
    for(BasicBlock::iterator it = BB->begin(), e=BB->end(); it != e ; ++it) {
      // If it is a PHI node which we need to eliminate
      PHINode *phi = dyn_cast<PHINode>(it);
      // If we have reached the first non-phi, we can bail out
      if (!phi) return;

      V_ASSERT(phi->getNumIncomingValues() < 3
               && "Phi node must have only two or less incoming edges");
      // And we don't handle 1-entry phi node.
      if (phi->getNumIncomingValues() != 2) continue;

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
      SelectInst* select =
        SelectInst::Create(edge_mask, phi->getIncomingValue(0),
                           phi->getIncomingValue(1), "merge", edge_mask);

      // Put in a place which satisfies data dependencies
      moveAfterLastDependant(select);

      phi->replaceAllUsesWith(select);
      phi->eraseFromParent();
      // We may change instructions which we planned on prev-select-ing
      // in here we update the value which we want to prev-select
      std::replace(m_outsideUsers.begin(), m_outsideUsers.end(),
                   static_cast<Instruction*>(phi),
                   static_cast<Instruction*>(select));

      changed = true;
      break;
    } // for
  } // while
}
Function* Predicator::createPredicatedFunction(Instruction *inst,
                                               Value* pred,
                                               const std::string& name) {

  // If we have created this function in the past, return it
  if (m_externalFunections.find(name) != m_externalFunections.end()) {
    return m_externalFunections[name];
  }

  // Create function arguments
  std::vector<const Type*> args;
  /// first argument is predicator
  args.push_back(pred->getType());

  /// all other arguments are the arguments of the instruction
  if (CallInst *CI = dyn_cast<CallInst>(inst)){
    for (unsigned j = 0; j < CI->getNumArgOperands(); ++j) {
      args.push_back(CI->getArgOperand(j)->getType());
    }
  } else {
    for (User::op_iterator it = inst->op_begin(),
         e = inst->op_end(); it != e ; ++it) {
      args.push_back((*it)->getType());
    }
  }

  // Generate the function decleration. Return type and args.
  FunctionType* type = FunctionType::get( inst->getType(), args, false);

  // Declare function
  Module * currentModule = inst->getParent()->getParent()->getParent();
  Function* f = dyn_cast<Function>(
    currentModule->getOrInsertFunction(name, type));
  V_ASSERT(f && "Function type is incorrect, so dyn_cast failed");
  m_externalFunections[name] = f;
  return f;
}

std::string Predicator::getNameFromPointerType(const Type* tp) {
  V_ASSERT(tp->isPointerTy() && "Load argument must be of pointer type");
  const PointerType *pty = dyn_cast<PointerType>(tp);
  const Type *pointee = pty->getElementType();
  V_ASSERT( pointee && "Pointer must have a pointee");
  return pointee->getDescription();
}

Instruction* Predicator::predicateInstruction(Instruction *inst, Value* pred) {

  // Preplace Load with call to function
  if (LoadInst* load = dyn_cast<LoadInst>(inst)) {
    Function* func =
      createPredicatedFunction(load, pred, Mangler::getLoadName());

    // A single parameter (pointer)
    std::vector<Value*> params;
    params.push_back(pred);
    params.push_back(load->getOperand(0));

    CallInst* call =
      CallInst::Create(func, params.begin(), params.end(), "pLoad", inst);
    load->replaceAllUsesWith(call);
    load->eraseFromParent();
    return call;
  }

  // Preplace Store with call to function
  if (StoreInst* store = dyn_cast<StoreInst>(inst)) {
    //Get type name
    Function* func =
      createPredicatedFunction(store, pred, Mangler::getStoreName());

    // Parameters (value to store, address)
    std::vector<Value*> params;
    params.push_back(pred);
    params.push_back(store->getOperand(0));
    params.push_back(store->getOperand(1));
    CallInst* call =
      CallInst::Create(func, params.begin(), params.end(), "", inst);
    store->replaceAllUsesWith(call);
    store->eraseFromParent();
    return call;
  }

  // Replace function call with masked function call
  if (CallInst* call = dyn_cast<CallInst>(inst)) {
    //Get type name
    std::string desc = call->getCalledFunction()->getName();
    Function* func =
      createPredicatedFunction(call, pred, Mangler::mangle(desc));

    // copy predicator and original parameters list
    std::vector<Value*> params;
    params.push_back(pred);
    for (unsigned j = 0; j < call->getNumArgOperands(); ++j) {
      params.push_back(call->getArgOperand(j));
    }

    CallInst* pcall =
      CallInst::Create(func, params.begin(), params.end(), "", inst);
    call->replaceAllUsesWith(pcall);
    call->eraseFromParent();
    return pcall;
  }

  // Nothing to do for this instruction
  return NULL;
}

void Predicator::selectOutsideUsedInstructions(Instruction* inst) {
  Value* prev_ptr = new AllocaInst(
    inst->getType(),            // type
    inst->getName() + "_prev",  // name
    inst->getParent()->getParent()->getEntryBlock().begin()); // where

  // Get BB predicator
  //V_PRINT("predicate", "select "<<*F<<"\n");
  BasicBlock *BB = inst->getParent();
  V_ASSERT(m_inMask.find(BB) != m_inMask.end() && "BB has no in-mask");
  Value* pred = m_inMask[BB];

  /// Get loop header, latch
  LoopInfo *LI = &getAnalysis<LoopInfo>();
  V_ASSERT(LI && "Unable to get loop analysis");
  Loop* loop = LI->getLoopFor(BB);
  V_ASSERT(loop && "Unable to find loop, we only predicate in-loop values");

  V_ASSERT (m_inInst.find(BB) != m_inInst.end() &&
            "Where did we save the mask in this BB ?");

  // Load the predicate value and place the select
  // We will place them in the correct place in the next section
  Instruction* predicate  = new LoadInst(pred,"prediacte");
  Instruction* prev_value  = new LoadInst(prev_ptr, "prev_value");
  SelectInst* select = SelectInst::Create(predicate, inst, prev_value, "out_sel");
  Instruction* store  = new StoreInst(select,prev_ptr);

  // We are predicating a PHINode
  // we can't put the select before we save the mask!
  // We will have to put it after the in-mask is stored
  // however, in case of non-PHI-Node, we can put it before
  // the instruction
  if (dyn_cast<PHINode>(inst)) {
    Instruction* loc = m_inInst[BB];
    select->insertAfter(loc);
  } else {
    // make sure the instructions we created are after the original instruction
    select->insertAfter(inst);
  }
  predicate->insertBefore(select);
  prev_value->insertBefore(select);
  store->insertAfter(select);

  // Replace all of the users of the original instruction with the select
  // We only replace instructions which do not belong to the same loop.
  // Instructuins which are inside the loop will be predicated with local masks
  // instructions outside the loop need the special masking.
  std::vector<Value*> users(inst->use_begin(), inst->use_end());
  for (std::vector<Value*>::iterator it = users.begin(),
       e = users.end(); it != e; ++it) {
    // If the user is an instruction
    Instruction* user = dyn_cast<Instruction>(*it);
    V_ASSERT(user && "a non-instruction user");
    // If the user is in this loop, don't change it.
    if (!loop->contains(user->getParent()) && user != select) {
      // replace only the appropriate value
      V_ASSERT(select != user);
      (user)->replaceUsesOfWith(inst, select);
    }
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

    V_PRINT("predicate", "F-Predicating "<<**it<<"\n");
    // Load the mask
    Value* load_pred = new LoadInst(pred, "loda_pred", *it);
    // Use the value of the mask to predicate using a function call
    predicateInstruction(*it, load_pred);
  }// for

}

void Predicator::collectInstructionsToPredicate(BasicBlock *BB) {
  // For each BB
  // Obtain loop analysis (are we inside a loop ?)
  LoopInfo *LI = &getAnalysis<LoopInfo>();
  PostDominatorTree* PDT = &getAnalysis<PostDominatorTree>();

  V_ASSERT(LI && "Unable to get loop analysis");
  Loop* loop = LI->getLoopFor(BB);
  
  // incase a block post dominates the entry block and is not in a loop
  // then all the instructions within it will be executed once and only once
  // so no need for masking
  BasicBlock &entryBlock = BB->getParent()->getEntryBlock();
  bool shouldMask = !PDT->dominates(BB, &entryBlock) || loop;
  for(BasicBlock::iterator it = BB->begin(), e=BB->end(); it != e ; ++it) {
    // If this is a load/store/call, save it for later
    if ( (dyn_cast<LoadInst> (it) ||
         dyn_cast<StoreInst>(it) ||
         dyn_cast<CallInst>(it)) && shouldMask) {
      m_toPredicate.push_back(it);
    }

    // If this instruction is used outside its BB,
    // save it for later.
    if (loop && hasOutsideUsers(it, loop)) {
      m_outsideUsers.push_back(it);
    }
  }
}

void Predicator::maskDummyEntry(BasicBlock *BB) {
  /// Save this as the mask value for this BB
  m_inMask[BB] = new AllocaInst(
    IntegerType::get(BB->getParent()->getContext(), 1), // type
    BB->getName() + "_in_mask",                         // name
    BB->getParent()->getEntryBlock().begin());          // where


  /// Set exit mask to ALL basic blocks. (even if there is no exiting)
  m_exitMask[BB] = new AllocaInst(
    IntegerType::get(BB->getParent()->getContext(), 1), // type
    BB->getName() + "_exit_mask",                       // name
    BB->getParent()->getEntryBlock().begin());          // where

  /// Set loop mask for this loop
  LoopInfo *LI = &getAnalysis<LoopInfo>();
  Loop* loop = LI->getLoopFor(BB);

  // On preheader of loop, zero the value of edge exit mask
  // this mask stores the values which left on this edge
  // naturally, need to set it to zero in preheader, before loop.
  if (loop) {
    BasicBlock* preheader = loop->getLoopPreheader();
    V_ASSERT (preheader && "Loop must have a preheader");
    new StoreInst(m_zero , m_exitMask[BB], preheader->getTerminator());
  }

  /// Create a loop mask, index it acording to the loop header
  /// Only If previously, we didn't set a mask for this loop
  /// (and this is a loop)
  if (loop && m_loopMask.find(loop->getHeader()) == m_loopMask.end()) {
    m_loopMask[loop->getHeader()] =
      new AllocaInst(
        IntegerType::get(BB->getParent()->getContext(), 1),  // type
        loop->getHeader()->getName() + "_loop_mask",         // name
        BB->getParent()->getEntryBlock().begin());           // where
  }
}

void Predicator::maskOutgoing_useIncoming(BasicBlock *BB, BasicBlock* SrcBB) {

  TerminatorInst* term = BB->getTerminator();
  BranchInst* br = dyn_cast<BranchInst>(term);
  V_ASSERT(br && "Unable to handle non branch terminators");

  // Output same as input
  V_ASSERT(m_inMask.find(SrcBB) != m_inMask.end() && "BB has no in-mask");
  Value* incoming = m_inMask[SrcBB];
  m_outMask[std::make_pair(BB, br->getSuccessor(0))] = incoming;
}

void Predicator::maskOutgoing_loopexit(BasicBlock *BB) {
  V_ASSERT(m_inMask.find(BB) != m_inMask.end() && "BB has no in-mask");

  TerminatorInst* term = BB->getTerminator();
  BranchInst* br = dyn_cast<BranchInst>(term);
  V_ASSERT(br && "Unable to handle non branch terminators");

  // Below we handle conditional branches
  Value* cond = br->getCondition();
  BasicBlock *BBsucc0 = br->getSuccessor(0);

  LoopInfo *LI = &getAnalysis<LoopInfo>();
  V_ASSERT(LI && "Unable to get loop analysis");
  // Are the predecessor BBs in the same loop as me?
  Loop* LoopCurrentBB = LI->getLoopFor(BB);
  Loop* LoopForBB0  = LI->getLoopFor(BBsucc0);

  BasicBlock* BBexit;
  BasicBlock* BBlocal;
  bool exitFirst;

  /// Decide which edge is the exit edge
  // and which one is the loop-local edge.
  if (LoopForBB0 == LoopCurrentBB) {
    BBexit  =  br->getSuccessor(1);
    BBlocal =  br->getSuccessor(0);
    exitFirst = false;
  } else {
    BBexit  =  br->getSuccessor(0);
    BBlocal =  br->getSuccessor(1);
    exitFirst = true;
  }

  Value* entry_mask_p = m_inMask[BB];
  Value* exit_mask_p  = m_exitMask[BB];
  Value* entry_mask   = new LoadInst(entry_mask_p, "entry_mask", br);
  Value* exit_mask    = new LoadInst(exit_mask_p,  "exit_mask", br);
  
  /// ---- calculate the exit mask. Who has left the loop ?
  BinaryOperator* notCond =
    BinaryOperator::Create(Instruction::Xor, cond, m_one, "notCond", br);
  BinaryOperator* who_left_tr = BinaryOperator::Create(Instruction::And,
                                                       entry_mask, (exitFirst ? cond : notCond) , "who_left_tr", br);

  BinaryOperator* who_ever_left_edge = BinaryOperator::Create(Instruction::Or,
                                                              exit_mask, who_left_tr, "ever_left_loop", br);
  new StoreInst(who_ever_left_edge, exit_mask_p, br);

  /// Calculate the loop exit. Who had left the loop on all exits ?
  /// Update all parent loops
  while (LoopCurrentBB && !LoopCurrentBB->contains(BBexit)) {
    Value* loop_mask_p = m_loopMask[LoopCurrentBB->getHeader()];
    V_ASSERT(m_loopMask.find(LoopCurrentBB->getHeader()) != m_loopMask.end()
           && "BB has no loop-mask");
    Value* loop_mask   = new LoadInst(loop_mask_p, "loop_mask", br);
    // update global loop mask based on who had left the loop (calculated before)
    BinaryOperator* or1 = BinaryOperator::Create(
        Instruction::Or, loop_mask, who_left_tr, "loop_mask", br);
    new StoreInst(or1, loop_mask_p, br);
    LoopCurrentBB = LoopCurrentBB->getParentLoop();
  }

  /// Update the exit condition for the current loop
  /// Use the current 
  Loop* CurrentLoop = LI->getLoopFor(BB);
  Value* cloop_mask_p = m_loopMask[CurrentLoop->getHeader()];
  Value* cloop_mask   = new LoadInst(cloop_mask_p, "current_loop_mask", br);  
  BinaryOperator* cor1 = BinaryOperator::Create(Instruction::Or, cloop_mask, who_left_tr, "curr_loop_mask", br);

  /// ----  Create the exit condition. When to leave the loop
  V_ASSERT(m_allone && "Unable to find allone func");
  CallInst *call_allone = CallInst::Create(m_allone, cor1, "shouldexit", br);
  BinaryOperator* notExit = BinaryOperator::Create(
    Instruction::Xor, call_allone, m_one, "shouldexit_not", br);

  // Make sure that we set the right exit condition
  // If the first operand of the branch command is a jump
  // back to itself then settng it to 'should exit' would
  // create an inf loop. TODO: Another (more optimized) way to solve
  // this would be to flip the jump targets.
  if (exitFirst) {
    br->setCondition(call_allone);
  } else {
    br->setCondition(notExit);
  }
  
  /// ---- Create the masks for inside the loop
  Value* local_edge_mask_p = new AllocaInst(IntegerType::get(
      BB->getParent()->getContext(), 1),
    BB->getName() + "_to_" + BBlocal->getName()+"_mask",
    BB->getParent()->getEntryBlock().begin());

  BinaryOperator* and3 =
    BinaryOperator::Create(Instruction::And, entry_mask,
                           ((!exitFirst) ? cond : notCond) , "local_edge", br);

  new StoreInst(and3, local_edge_mask_p, br);

  /// Save outgoing edges
  m_outMask[std::make_pair(BB, BBexit)] = exit_mask_p;
  m_outMask[std::make_pair(BB, BBlocal)] = local_edge_mask_p;
}

void Predicator::maskOutgoing_fork(BasicBlock *BB) {

  TerminatorInst* term = BB->getTerminator();
  BranchInst* br = dyn_cast<BranchInst>(term);
  V_ASSERT(br && "Unable to handle non branch terminators");

  // Below we handle conditional brancs
  Value* cond = br->getCondition();
  BasicBlock *BBsucc0 = br->getSuccessor(0);
  BasicBlock *BBsucc1 = br->getSuccessor(1);

  ///
  /// In here we handle simple forking of two basic blocks to non exit blocks.
  //
  V_ASSERT(m_inMask.find(BB) != m_inMask.end() && "BB has no in-mask");
  Value* incoming = m_inMask[BB];
  Value  *l_incoming   = new LoadInst(incoming, "l_inc", br);

  /// One side takes the condition as is,
  /// the other uses the negation of the condition
  BinaryOperator* notCond = BinaryOperator::Create(
    Instruction::Xor, cond, m_one, "Mneg", br);

  /// We negate using XOR 1
  Value* mtrue  = new AllocaInst(
    IntegerType::get(BB->getParent()->getContext(), 1),
    BB->getName() + "_to_" + BBsucc0->getName()+"_mask",
    BB->getParent()->getEntryBlock().begin());

  Value* mfalse = new AllocaInst(
    IntegerType::get(BB->getParent()->getContext(), 1),
    BB->getName() + "_to_" + BBsucc1->getName()+"_mask",
    BB->getParent()->getEntryBlock().begin());

  BinaryOperator* MFalse  = BinaryOperator::Create(
    Instruction::And, l_incoming, notCond,
    BB->getName() + "_to_" + BBsucc1->getName(), br);
  BinaryOperator* MTrue   = BinaryOperator::Create(
    Instruction::And, l_incoming, cond,
    BB->getName() + "_to_" + BBsucc0->getName(), br);

  // Store the mask into the alloca
  new StoreInst(MFalse, mfalse, br);
  new StoreInst(MTrue, mtrue, br);
  /// Save outgoing edges
  m_outMask[std::make_pair(BB, BBsucc0 )] = mtrue;
  m_outMask[std::make_pair(BB, BBsucc1)] = mfalse;
}

void Predicator::collectOptimizedMasks(Function* F,
                                       PostDominatorTree* PDT,
                                       DominatorTree*  DT) {

  // get loop info
  LoopInfo *LI = &getAnalysis<LoopInfo>();
  // For all blocks in function
  for (Function::iterator x = F->begin(), x_e = F->end(); x != x_e ; ++x) {

    // If we are the header block in the loop
    // Incoming masks for this case are special. Ignore this case.
    Loop* loopX  = LI->getLoopFor(x);
    if (loopX && loopX->getHeader() == x) {
      continue;
    }

    for (Function::iterator y = F->begin(), y_e = F->end(); y != y_e ; ++y) {
      // If we are the header block in the loop
      // Incoming masks for this case are special. Ignore this case.
      Loop* loopY  = LI->getLoopFor(y);
      if (loopY && loopY->getHeader() == y) {
        continue;
      }

      if (loopY != loopX) continue;

      V_PRINT("predicate", x->getName()<<","<< y->getName()<<"share mask\n");
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
    } //x
  } //y

}

void Predicator::maskOutgoing(BasicBlock *BB) {
  //V_PRINT("predicate",
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

  // Implementation of unconditional branches is easy.
  if (br->isUnconditional()) {
    // If this outgoing mask is an optimized case
    return maskOutgoing_useIncoming(BB, BB);
  }
  // Below we handle conditional brancs
  BasicBlock *BBsucc0 = br->getSuccessor(0);
  BasicBlock *BBsucc1 = br->getSuccessor(1);

  /// We will need loop information to know if this BB
  /// has edges leaving the loop
  LoopInfo *LI = &getAnalysis<LoopInfo>();
  V_ASSERT(LI && "Unable to get analysis");
  // Are the predecessor BBs in the same loop as me?
  Loop* LoopCurrentBB = LI->getLoopFor(BB);
  Loop* LoopForBB0  = LI->getLoopFor(BBsucc0);
  Loop* LoopForBB1  = LI->getLoopFor(BBsucc1);
  ///
  /// In here we handle Loop exit
  ///
  if (LoopCurrentBB != LoopForBB0 || LoopCurrentBB != LoopForBB1) {
    maskOutgoing_loopexit(BB);
  } else {
    maskOutgoing_fork(BB);
  } // in-loop split
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

  // If not, and this is the entry block, there is nothing to do
  if (!pred) {
    ConstantInt* one = ConstantInt::get(
      BB->getParent()->getContext(), APInt(1, StringRef("1"), 10));

    Instruction* st = new StoreInst(one, old_in, BB->getFirstNonPHI());
    moveAfterLastDependant(st);
    m_inInst[BB] = st;
    return;
  }

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
  V_ASSERT(m_inMask.find(BB) != m_inMask.end() && "BB has no in-mask");
  Value* old_in = m_inMask[BB];

  // create phi node
  PHINode* new_phi = PHINode::Create(
    IntegerType::get(BB->getParent()->getContext(), 1),
    BB->getName() + "_Min", BB->begin());

  // For each incoming edge into the block
  for (pred_iterator it = pred_begin(BB), e = pred_end(BB); it != e; ++it) {
    BasicBlock* inBB = *it;
    CFGEdge edge = std::make_pair(inBB, BB);
    V_ASSERT(m_outMask.find(edge) != m_outMask.end() && "Edge has no out-mask");
    Value* in_mask_p = m_outMask[edge]; // edge from inBB to this BB
    Value* in_mask   = new LoadInst(in_mask_p, "in_mask", inBB->getTerminator());
    new_phi->addIncoming(in_mask, inBB);
  }

  Instruction* st = new StoreInst(new_phi, old_in, BB->getFirstNonPHI());
  m_inInst[BB] = st;

  /// On loop headers we also set the loop masks, which
  //indicates if we need to leave the loop.
  // In this mask:
  // 1 - already left the loop
  // 0 - waiting to leave the loop.
  // This means that we exit when mask is all-one.
  // The initial mask is All-one XOR incoming mask (from preheader)
  V_ASSERT(preheader && "No preheader ?");
  CFGEdge pedge = std::make_pair(preheader, BB);
  V_ASSERT(m_outMask.find(pedge) != m_outMask.end() &&
           "Preheader has no out-mask");
  Value* in_loop_mask_p = m_outMask[pedge]; // edge from preheader to this BB

  Value* in_loop_mask =
    new LoadInst(in_loop_mask_p, "in_lp_mask", preheader->getTerminator());
  BinaryOperator* neg_incoming_loop_mask =
    BinaryOperator::Create(Instruction::Xor,
                           in_loop_mask, m_one,
                           "negIncomingLoopMask",
                           preheader->getTerminator());

  V_ASSERT(m_loopMask.find(BB) != m_loopMask.end()&& "Unable to find loop mask");
  new StoreInst(neg_incoming_loop_mask,
                m_loopMask[BB], preheader->getTerminator());
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
  //V_PRINT("predicate",
  // "Masking Incoming BasicBlock:"<<BB->getName()<<"\n");

  if (std::distance(pred_begin(BB), pred_end(BB)) < 2) {
    // Get single pred or NULL
    maskIncoming_singlePred(BB, BB->getSinglePredecessor());
    return;
  }

  /// If this is not a simple case,
  /// we will need some loop info.
  LoopInfo *LI = &getAnalysis<LoopInfo>();
  Loop* loop  = LI->getLoopFor(BB);

  //
  // If we are the header block in the loop
  // This is where the backedge merges.
  //
  if (loop && loop->getHeader() == BB) {
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



void Predicator::predicateFunction(Function *F) {

  //
  // This is the entry point for the algorithm
  // It derives all of the steps of the predication.
  // This is a good start to read the code.
  //

  // This pass runs on multiple functions, clean
  // the data structures between each runts.
  m_inMask.clear();
  m_inInst.clear();
  m_outMask.clear();
  m_exitMask.clear();
  m_loopMask.clear();
  m_toPredicate.clear();
  m_outsideUsers.clear();
  m_optimizedMasks.clear();

  // Get Dominator and Post-Dominator analysis passes
  PostDominatorTree* PDT = &getAnalysis<PostDominatorTree>();
  DominatorTree* DT      = &getAnalysis<DominatorTree>();
  RegionInfo* RI         = &getAnalysis<RegionInfo>();
  FunctionSpecializer specializer(
    this, F, m_allzero, PDT, DT, RI);

  V_PRINT("predicate", "Predicating "<<F->getName()<<"\n");

  /// Before we begin the predication process we need to collect specialization
  /// information. This is formation is the dominator-frontier properties of
  /// the original graph.
  specializer.CollectDominanceInfo();

  // Collect efficient mask generation
  if (EnableOptMasks) {
    collectOptimizedMasks(F, PDT, DT);
  }

  V_PRINT("predicate", "Before:"<<*F<<"\n\n\n");

  // collect instructions to predicate and instructions
  // with outside users
  for( Function::iterator it = F->begin(), e  = F->end(); it != e ; ++it) {
    collectInstructionsToPredicate(it);
  }

  V_PRINT("predicate",
          "prev-select marked "<<
          m_outsideUsers.size()<<" instructions\n");


  LoopInfo *LI = &getAnalysis<LoopInfo>();
  V_ASSERT(LI && "Unable to get loop analysis");
  for( Function::iterator it = F->begin(), e  = F->end(); it != e ; ++it) {
    
    // Assert that each block has at most two preds
    V_ASSERT(std::distance(pred_begin(it), pred_end(it))<3 && "Phi canon failed");

    /// Verify that the loop is simplified
    Loop* loop = LI->getLoopFor(it);
    if (loop) {
        V_ASSERT(loop->isLoopSimplifyForm() && "Loop must be in normal form");
    }
  }

  /// Place dummy in-masks
  for( Function::iterator it = F->begin(), e  = F->end(); it != e ; ++it) {
    maskDummyEntry(it);
  }
  /// Place out-masks
  for (Function::iterator it = F->begin(), e  = F->end(); it != e ; ++it) {
    maskOutgoing(it);
  }
  /// replace dummy in-masks with real in-masks
  for (Function::iterator it = F->begin(), e  = F->end(); it != e ; ++it) {
    maskIncoming(it);
  }
  /// replace all PHINodes with select instruction
  for (Function::iterator it = F->begin(), e  = F->end(); it != e ; ++it) {
    convertPhiToSelect(it);
  }
  /// Place selects on loop-exit-users
  for (SmallInstVector::iterator
       it = m_outsideUsers.begin(), e=m_outsideUsers.end(); it != e; ++it) {
    selectOutsideUsedInstructions(*it);
  }
  /// 1.replace all side effect instructions with function calls
  /// 2.insert previous-select for instructions which are used outside
  /// the basic block
  predicateSideEffectInstructions();

  // Perform a linearization of the function
  linearizeFunction(F, specializer);

  V_PRINT("predicate", "Specialize:"<<*F<<"\n");

  V_ASSERT(!verifyFunction(*F) && "I broke this module");

  /// Insert bypass cfg (specialization)
  specializer.specializeFunction();

  V_PRINT("predicate", "Final:"<<*F<<"\n");
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

} // namespace intel


/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  FunctionPass* createPredicator() {
    return new intel::Predicator();
  }
}

/// Support for dynamic loading of modules under Linux
char intel::Predicator::ID = 0;
static RegisterPass<intel::Predicator>
CLIPredicate("predicate", "Predicate Function");

