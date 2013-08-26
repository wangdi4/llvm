/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "Specializer.h"
#include "Predicator.h"
#include "Linearizer.h"
#include "Mangler.h"
#include "Logger.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/RegionIterator.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Constants.h"

#include <stack>

static cl::opt<unsigned>
SpecializeThreshold("specialize-threshold", cl::init(15), cl::Hidden,
  cl::desc("The cut-off point for specialization of a single basic block"));

static cl::opt<bool>
EnableSpecialization("specialize", cl::init(true), cl::Hidden,
  cl::desc("Enable specialization"));

namespace intel {

FunctionSpecializer::FunctionSpecializer(Predicator* pred, Function* func,
                                         Function* all_zero,
                                         PostDominatorTree* PDT,
                                         DominatorTree*  DT,
                                         LoopInfo *LI,
                                         WIAnalysis *WIA,
                                         OCLBranchProbability *OBP):
  m_pred(pred), m_func(func),
  m_allzero(all_zero), m_PDT(PDT), m_DT(DT), m_LI(LI), m_WIA(WIA), m_OBP(OBP),
  m_zero(ConstantInt::get(m_func->getParent()->getContext(), APInt(1, 0))),
  m_one(ConstantInt::get(m_func->getParent()->getContext(), APInt(1, 1)))
{
  initializeBICost();
}

void FunctionSpecializer::initializeBICost() {
  m_nameToInstNum["max"] = 1;
  m_nameToInstNum["fabs"] = 1;
}

bool FunctionSpecializer::addHeuristics(const BasicBlock *BB) const {
  // Collect instruction amount metrics
  CodeMetrics Metrics;
  Metrics.analyzeBasicBlock(BB);

  unsigned numInst = Metrics.NumInsts;

  // Check whether there is a function call
  for (BasicBlock::const_iterator it = BB->begin(); it != BB->end(); it++) {
    if (isa<InvokeInst>(it))
      return true;

    if (const CallInst* CI = dyn_cast<CallInst>(it)) {
      Function* calledFunc = CI->getCalledFunction();

      assert(calledFunc && "Called function should not be null");

      std::string name = Mangler::demangle(calledFunc->getName().str(), false);
      std::map<std::string, unsigned>::const_iterator itr = m_nameToInstNum.find(name);

      // If the called instruction's penalty is known
      if (itr != m_nameToInstNum.end()) {
        numInst += itr->second;
      }
      else {
        return true;
      }
    }
  }

  return (numInst >= SpecializeThreshold);
}

bool FunctionSpecializer::shouldSpecialize(const BypassInfo & bi) const {

  // In case we are specializing a single basic block
  V_ASSERT(bi.m_skippedBlocks.size() > 0);
  if (bi.m_skippedBlocks.size() < 2 && !addHeuristics(bi.m_root)) {
    return false;
  }

  // Specialize only if the root has a a mask
  return m_WIA->isDivergentBlock(bi.m_root);
}

BasicBlock* FunctionSpecializer::getAnyReturnBlock() {
  // find return block
  for (Function::iterator x = m_func->begin(),
       xe = m_func->end(); x != xe ; ++x) {
    Instruction* term = x->getTerminator();
    if (dyn_cast<ReturnInst>(term)) {
      return x;
    }
  }
  return NULL;
}

bool FunctionSpecializer::CanSpecialize() {

  BasicBlock* ret = getAnyReturnBlock();
  if (!ret) {
    return false;
  }

  for (Function::iterator y = m_func->begin(),
       ye = m_func->end(); y != ye ; ++y) {
    //  Is Ret the 'all post dominator'
    // (return block which dominates all) ?
    if (!m_PDT->dominates(ret, y))  {
      return false;
    }
  }
  // we can specialize!
  return true;
}

void FunctionSpecializer::ObtainMasksToZero(BypassInfo & bi) {
  V_ASSERT(bi.m_postDom && bi.m_foot && "NULL argumnets?");
  std::vector<std::pair<BasicBlock*, BasicBlock*> > outMasks;
  // We need to make sure the outmask on the region exit edge is zero if 
  // we by pass the region.
  outMasks.push_back (std::make_pair(bi.m_postDom, bi.m_foot));

  // It can be that we bypass the preheader of a loop but we will not bypass
  // the loop itself when the exit edge is between the loop preheader and the 
  // loop header. The preheader initializes the loop\exit masks of the loop 
  // so we need to enforce them being 0 in case the preheader is skipped (which
  // means the loop should be masked out, and the exit edges are zero also).
  Loop *footLoop = m_LI->getLoopFor(bi.m_foot);
  if (footLoop && footLoop->getLoopPreheader() == bi.m_postDom) {
    V_ASSERT(footLoop->getHeader() == bi.m_foot &&
          "only edge entering the loop should be from preheader to header");
    // Adding the in mask of the loop header.
    m_inMasksToZero[bi] = bi.m_foot;

    // Adding the out masks of the loop exit edges.
    SmallVector<BasicBlock *, 4> exitingBlocks;
    footLoop->getExitingBlocks(exitingBlocks);
    for (unsigned i=0; i<exitingBlocks.size(); ++i) {
      BasicBlock *exitingBB = exitingBlocks[i];
      for (succ_iterator succIt = succ_begin(exitingBB),
          succE = succ_end(exitingBB); succIt != succE; ++succIt) {
        if (!footLoop->contains(*succIt)) {
          outMasks.push_back(std::make_pair(exitingBB, *succIt));
        }
      }
    }
  }
  m_outMasksToZero[bi] = outMasks;
}

void FunctionSpecializer::addAuxBBForSingleExitEdge(BypassInfo & info) {
    BasicBlock* new_block = BasicBlock::Create(info.m_postDom->getContext(), "bypassAuxExitBB", info.m_postDom->getParent(), info.m_foot);

    // Add the new block to the loop info analysis
    Loop *loop = m_LI->getLoopFor(info.m_postDom);
    if (loop)
      loop->addBasicBlockToLoop(new_block, m_LI->getBase());

    TerminatorInst* term = info.m_postDom->getTerminator();
    assert (isa<BranchInst>(term) && "term should be a branch instruction");
    BranchInst *br = cast<BranchInst>(term);

    // Move the branch from the post dominator to the new block
    Value* cond = br->getCondition();
    BasicBlock *BBsucc0 = br->getSuccessor(0);
    BasicBlock *BBsucc1 = br->getSuccessor(1);

    BranchInst * newBr = BranchInst::Create(BBsucc0, BBsucc1, cond, new_block);

    // Update the WI analysis with the new branch info
    m_WIA->setDepend(br, newBr);

    // Update the Phi nodes of info.m_postDom's original successors
    for (unsigned i=0; i < 2; ++i) {
      for (BasicBlock::iterator itr = term->getSuccessor(i)->begin();
          itr != term->getSuccessor(i)->end();
          ++itr) {
        if (PHINode* phi = dyn_cast<PHINode>(itr)) {
          for (unsigned i=0; i < phi->getNumIncomingValues(); ++i) {
            if (phi->getIncomingBlock(i) == info.m_postDom)
              phi->setIncomingBlock(i, new_block);
          }
        }
        else
          break;
      }
    }

    // Remove the original branch from the post-dominator
    term->eraseFromParent();

    // Update the post dominator branch to jump to the new block
    BranchInst::Create(new_block, info.m_postDom);

    // Update dominance info
    m_DT->addNewBlock(new_block, info.m_postDom);

    // Update the foot node in the bypass info
    info.m_foot = new_block;

    // Update other bypass infos for adding this new basic block
    for (std::vector<BypassInfo>::iterator itr = m_bypassInfoContainer.begin();
        itr != m_bypassInfoContainer.end();
        ++itr) {
      if (&(*itr) != &info) {
        // Update the skipped list if needed
        if (itr->m_skippedBlocks.count(info.m_postDom)){
          itr->m_skippedBlocks.insert(new_block);
        }
        // Update head if needed
        if (info.m_head == info.m_postDom)
          info.m_head = new_block;
      }
    }

    // Update the scheduling constrains for the predicated regions 
    SchdConstMap & predSched = m_WIA->getSchedulingConstraints();
    for (SchdConstMap::iterator itr = predSched.begin();
           itr != predSched.end();
           ++itr) {

      std::vector<BasicBlock*>::iterator bbItr = std::find(itr->second.begin(), itr->second.end(), info.m_postDom);

      // if the post dom is part of a scheduling constraint then add the new block
      // to the scheduling constraint. The new block is added right after the post dom to maintain topological order
      if (bbItr != itr->second.end())
        itr->second.insert(++bbItr, new_block);
    }
}

void FunctionSpecializer::getBypassRegion(BypassInfo & info) {

  std::stack<BasicBlock*> workSet;

  // make sure we do not continue after the last dominated post dominator
  info.m_skippedBlocks.insert(info.m_postDom);

  // starting the work from the root
  workSet.push(info.m_root);

  while (!workSet.empty()) {
    BasicBlock *curBlk = workSet.top();
    workSet.pop();
    info.m_skippedBlocks.insert(curBlk); // mark block as skipped

    for (succ_iterator SI = succ_begin(curBlk), E = succ_end(curBlk); SI != E; ++SI) {
      BasicBlock *succBlk = (*SI);
      if (!info.m_skippedBlocks.count(succBlk)) {
        workSet.push(succBlk);
      }
    }
  }
}

bool FunctionSpecializer::calculateBypassInfoPerBranch(BasicBlock *root) {

  assert(root && "root shouldn't be null");

  BasicBlock * head = root->getSinglePredecessor();

  // A single entry is needed
  if (!head)
    return false;

  // find the first post-dom of root that also dominated by root

  m_bypassInfoContainer.push_back(BypassInfo(head, root));
  BypassInfo & info = m_bypassInfoContainer.back();

  BasicBlock * postDom = root;

  // Basically, what we want to do now is to take the last post dominator
  // that is both dominated by root and in the same loop as root
  // (of course no loop means the same loop)

  while(1) {
    DomTreeNode * currentNode = m_PDT->getNode(postDom);
    DomTreeNode * postDomNode = 0;

    if (currentNode) // Can be 0 in case of infinite loops
      postDomNode = currentNode->getIDom();

    if (postDomNode) { // Can be 0 in case of last basic block
      postDom = postDomNode->getBlock();
    }
    else {
      if (! info.m_postDom) { // root is either an exit block or inside an infinite loop
        m_bypassInfoContainer.pop_back();
        return false;
      }
      else
        break;
    }

    // The postDom should be dominated by root and in the same loop
    if (m_DT->dominates(root, postDom)) {
      if (m_LI->getLoopFor(root) == m_LI->getLoopFor(postDom)) {
        info.m_postDom = postDom;
      }
    }
    else
      break;
  }

  if (! info.m_postDom) {   // Bypass a single basic block
    info.m_postDom = info.m_root;
    info.m_skippedBlocks.insert(info.m_postDom);
  }
  else {
    getBypassRegion(info);
  }

  assert (info.m_postDom && "Post dominator is expected");

  llvm::succ_iterator succItr = succ_begin(info.m_postDom);
  llvm::succ_iterator succEnd = succ_end(info.m_postDom);

  // Return in case that m_postDom is an exit node
  if (succItr == succEnd) {
    m_bypassInfoContainer.pop_back();
    return false;
  }

  info.m_foot = *succItr;
  // if postDom has two successors
  if (++succItr != succEnd) {
    // if we plan to add a bypass then we need to add an auxiliary BB in this case
    if (m_DT->dominates(info.m_postDom, info.m_foot) && m_DT->dominates(info.m_postDom, *succItr) &&
        (info.m_root != info.m_postDom || addHeuristics(info.m_root))) {
      addAuxBBForSingleExitEdge(info);
    }
    else {
      m_bypassInfoContainer.pop_back();
      return false;
    }
  }
  assert (info.m_foot && "m_foot was not defined");

  return true;
}

void FunctionSpecializer::CollectDominanceInfo() {

  // If we did not enable specialization, do not collect any jump edges
  if (! EnableSpecialization) return;
  // We can't specialize code which has endless loop (not all blocks are
  // dominated by return block)
  if (! CanSpecialize()) return;

  for (Function::iterator bb = m_func->begin(), bbe = m_func->end(); bb != bbe ; ++bb) {

    TerminatorInst* term = bb->getTerminator();
    BranchInst* br = dyn_cast<BranchInst>(term);

    // we calculate regions either for successors of divergent branches or
    // for divergent blocks
    if(!br || ! br->isConditional() ||
      (!m_WIA->isDivergentBlock(bb) && m_WIA->whichDepend(term) == WIAnalysis::UNIFORM))
      continue;

    for (unsigned i=0; i < br->getNumSuccessors(); ++i) {
      if (! m_DT->dominates(bb, br->getSuccessor(i)) ||
          m_OBP->isEdgeHot(bb, br->getSuccessor(i))  ||
          ! calculateBypassInfoPerBranch(br->getSuccessor(i)))
        continue;

      BypassInfo & info = m_bypassInfoContainer.back();

      ObtainMasksToZero(info);
    }
  }
}

void FunctionSpecializer::specializeFunction() {
  for (std::vector<BypassInfo>::iterator itr = m_bypassInfoContainer.begin();
      itr != m_bypassInfoContainer.end();
      ++itr) {
    if (shouldSpecialize(*itr)) {
      specializeEdge(*itr);
      V_ASSERT(!verifyFunction(*m_func) && "I broke this module");
    }
  }
}

void FunctionSpecializer::registerSchedulingScopes(SchedulingScope& parent) {
  for (std::vector<BypassInfo>::iterator itr = m_bypassInfoContainer.begin();
        itr != m_bypassInfoContainer.end();
        ++itr) {
    BypassInfo & bi = *itr;
    // if we specialize
    if (! shouldSpecialize(bi)) continue;
    // bypasses over a single BB does not cause to additional linearization constraints
    if (bi.m_skippedBlocks.size() == 1) continue;

    // create a scheduling scope
    SchedulingScope *scp = new SchedulingScope(NULL);
    // insert all basic blocks restrictions from the bypass info
    for (std::set<BasicBlock*>::iterator itr = bi.m_skippedBlocks.begin();
        itr != bi.m_skippedBlocks.end();
        ++itr) {
      scp->addBasicBlock(*itr);
    }
    // register the bypass info restrictions with parent
    parent.addSubSchedulingScope(scp);
  }
}

void FunctionSpecializer::addNewToRegion(BasicBlock* old, BasicBlock* fresh) {
  V_ASSERT(old && fresh);

  for (std::vector<BypassInfo>::iterator itr =m_bypassInfoContainer.begin();
      itr != m_bypassInfoContainer.end();
      ++itr) {
    // if has old, add the new
    if (itr->m_skippedBlocks.count(old)) {
      itr->m_skippedBlocks.insert(fresh);
    }
  }
}


BasicBlock* FunctionSpecializer::createIntermediateBlock(
  BasicBlock* before, BasicBlock* after, const std::string& name) {

  // new block
  BasicBlock* new_block = BasicBlock::Create(before->getContext(), name, after->getParent(), after);
  // fix before
  TerminatorInst* term = before->getTerminator();
  BranchInst *br = dyn_cast<BranchInst>(term);
  V_ASSERT(br);

  bool changed = false;
  for (unsigned i=0; i<br->getNumSuccessors(); ++i) {
    if (br->getSuccessor(i) == after) { changed = true; br->setSuccessor(i, new_block); }
  }
  V_ASSERT(changed);

  // Fix after block
  for (BasicBlock::iterator it = after->begin(),e=after->end(); it != e; ++it) {
    if (PHINode* phi = dyn_cast<PHINode>(it)) {
      for (unsigned i=0; i< phi->getNumIncomingValues();++i) {
        if (phi->getIncomingBlock(i) == before) phi->setIncomingBlock(i, new_block);
      }
    } else break;
  }

  // change new_block to point to after
  BranchInst::Create(after,new_block);
  V_ASSERT(new_block->getSinglePredecessor());
  return new_block;
}

void FunctionSpecializer::ZeroBypassedMasks(BypassInfo & bi, BasicBlock *src,
                                    BasicBlock *exit, BasicBlock *footer) {
  // Some mask are initialized or computed inside the region but are used
  // outside the region. These edges, blocks of these masks were collected 
  // in the collectDominanceInfo stage. we use phi node that collect the 
  // value computed inside the region or zero if the region is bypassed, and
  // set the mask in the region footer.
  std::map<BypassInfo, BasicBlock*, BypassInfoComparator>::iterator inIt = m_inMasksToZero.find(bi);
  if (inIt != m_inMasksToZero.end()) {
    BasicBlock *BB = inIt->second;
    Value *maskP = m_pred->getInMask(BB);
    V_ASSERT(maskP != NULL && "BB has no in-mask");
    propagateMask(maskP, src, exit, footer);
  }
  MapRegToBBPairVec::iterator outIt = m_outMasksToZero.find(bi);
  if (outIt != m_outMasksToZero.end()) {
    BBPairVec &edges = outIt->second;
    for (unsigned i=0; i<edges.size(); ++i) {
      Value *maskP = m_pred->getEdgeMask(edges[i].first, edges[i].second);
      V_ASSERT(maskP != NULL && "BB has no in-mask");
      propagateMask(maskP, src, exit, footer);
    }
  }
}

void FunctionSpecializer::specializeEdge(BypassInfo & bi) {

  V_ASSERT(!verifyFunction(*m_func) && "I broke this module");
  
  V_ASSERT(bi.m_head && "no head!");

  // Refining the incoming edge
  BasicBlock* head = bi.m_head;
  BasicBlock* entry = bi.m_root;
  BasicBlock* before = entry->getSinglePredecessor();
  // if there are two edges entering the region ('loop' case) - take edge coming
  // from outside
  if (!before) {
    for (llvm::pred_iterator it = pred_begin(entry); it != pred_end(entry); ++it) {
      if (bi.m_skippedBlocks.find(*it) != bi.m_skippedBlocks.end()) continue;
      if (before) {
        // BUGBUG: sometimes linearizer's change of CFG leads to situation
        // when dominance info collected earlier is no valid anymore (e.g., some
        // region is being extended, or new dominance produced). In that case
        // edge analysis vs. region containment info doesn't hold
        before = NULL;
        break;
      }
      before = *it;
    }
  }
  if (!before) return;

  // Refining the outgoing edge
  BasicBlock* exitBlock = bi.m_postDom;
  BasicBlock* after = NULL;
  // if there are two edges leaving the region ('loop' case) - take edge going
  // outside of the loop
  for (llvm::succ_iterator it = succ_begin(exitBlock); it != succ_end(exitBlock); ++it) {
    if (bi.m_skippedBlocks.find(*it) != bi.m_skippedBlocks.end()) continue;
    if (after) {
      // BUGBUG: sometimes linearizer's change of CFG leads to situation
      // when dominance info collected earlier is no valid anymore (e.g., some
      // region is being extended, or new dominance produced). In that case
      // edge analysis vs. region containment info doesn't hold
      after = NULL;
      break;
    }
    after = *it;
  }

  if (!after) return;

  // Creating launching and landing pads for bypass
  BasicBlock* src = createIntermediateBlock(before, entry, "header");

  Value* mask = m_pred->getEdgeMask(head ,bi.m_root);
  V_ASSERT(mask && "unable to find mask");

  BasicBlock* footer = createIntermediateBlock(exitBlock, after, "footer");

  // Get the list of PHINodes which we need to insert (for skipped instructions)
  std::vector<std::pair<Instruction* , std::set<Instruction*> > > vals;
  findValuesToPhi(bi,  vals);
  
  // Create the function call to 'is zero' to test if we need
  // to jump over this section
  V_ASSERT( mask && "Unable to find incoming mask for block");
  Value *incoming = mask;
  Value *l_incoming = new LoadInst(incoming, "", src->getTerminator());
  CallInst *call_allzero =
    CallInst::Create(m_allzero, l_incoming, "jumpover", src->getTerminator());
  // replace previous branch instruction with conditional jump
  BranchInst *branch = dyn_cast<BranchInst>(src->getTerminator());
  V_ASSERT(branch && "This terminator is not a branch!");
  BranchInst::Create(footer, bi.m_root, call_allzero, src);
  branch->eraseFromParent();

  // Create the PHINodes for the split values
  for (std::vector<std::pair<Instruction* , std::set<Instruction*> > >::iterator it =
       vals.begin(), e = vals.end(); e != it ; ++it) {
    Instruction* inst = it->first;
    // create phi node
    V_ASSERT(!inst->getType()->isVoidTy() &&
           "Instruction must not have void type");
    PHINode* new_phi = PHINode::Create(
      inst->getType(), 2, inst->getName() + "_spec", footer->begin());
    UndefValue* undef = UndefValue::get(inst->getType());

    // Use PHI value outside of skipped region
    for(std::set<Instruction*>::iterator deps =
        it->second.begin(), deps_e = it->second.end();
        deps != deps_e ; ++deps ) {
      (*deps)->replaceUsesOfWith(inst, new_phi);
    }

    new_phi->addIncoming(inst, exitBlock);
    new_phi->addIncoming(undef, src);
  }

  // Zero masks that are used outside the region but are computed
  // inside the region.
  ZeroBypassedMasks(bi, src, exitBlock, footer);

  // New start and end nodes are a part of this region
  addNewToRegion(bi.m_root, src);
  addNewToRegion(bi.m_root, footer);

  V_ASSERT(!verifyFunction(*m_func) && "I broke this module");

}

void FunctionSpecializer::propagateMask(Value *mask_target, BasicBlock *header,
                                   BasicBlock *exitBlock, BasicBlock *footer) {
  V_ASSERT(mask_target && header && exitBlock && footer &&
           mask_target->getType() == PointerType::get(m_zero->getType(),0));
  Value* non_bypass_mask =
    new LoadInst(mask_target, mask_target->getName() + "_non_bypass",
                 exitBlock->getTerminator());

  PHINode* new_phi =
    PHINode::Create(non_bypass_mask->getType(), 2,
                    mask_target->getName() + "_maskspec", footer->begin());

  new_phi->addIncoming(non_bypass_mask, exitBlock);
  new_phi->addIncoming(m_zero, header);
  new StoreInst(new_phi, mask_target, footer->getFirstNonPHI());
}

void FunctionSpecializer::findValuesToPhi(
  BypassInfo & bi, std::vector<std::pair<Instruction* , std::set<Instruction*> > > &to_add_phi) {

  // Find values to protect behind a PHI
  // For each skipped BB
  for (std::set<BasicBlock*>::iterator BB = bi.m_skippedBlocks.begin();
      BB != bi.m_skippedBlocks.end(); ++BB) {
    // for each inst
    for (BasicBlock::iterator inst = (*BB)->begin(), inst_e = (*BB)->end(); inst != inst_e; ++inst) {
      // For each skipped instruction
      std::set<Instruction*> deps;
      // for each user
      if (inst->getType()->isVoidTy()) continue;
      // for each of the users of this inst
      for (Value::use_iterator us = inst->use_begin(),
           us_e = inst->use_end(); us != us_e ; ++us) {
        if (Instruction* iii = dyn_cast<Instruction>(*us)) {
          // if the user of this instruction is not in skipped region
          // if this is a new BB
          if (! bi.m_skippedBlocks.count( iii->getParent()))  { deps.insert(iii);  }
        }
      }
      if (!deps.empty()) {
        to_add_phi.push_back(std::pair<Instruction* , std::set<Instruction*> >(inst,deps));
      }
    } // for each bb
  } // for each bb
}

} // namespace
