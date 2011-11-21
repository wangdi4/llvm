#include "Specializer.h"
#include "Predicator.h"
#include "Linearizer.h"
#include "Logger.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/RegionIterator.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/InlineCost.h"

static cl::opt<unsigned>
SpecializeThreshold("specialize-threshold", cl::init(10), cl::Hidden,
  cl::desc("The cut-off point for specialization of a single basic block"));

static cl::opt<bool>
EnableSpecialization("specialize", cl::init(true), cl::Hidden,
  cl::desc("Enable specialization"));

static cl::opt<bool>
SpecOnlyAfterBranch("specialize-only-after-branch", cl::init(false), cl::Hidden,
  cl::desc("Specialize blocks only after a branch"));

namespace intel {

FunctionSpecializer::FunctionSpecializer(Predicator* pred, Function* func,
                                         Function* all_zero,
                                         PostDominatorTree* PDT,
                                         DominatorTree*  DT,
                                         RegionInfo *RI):
  m_pred(pred), m_func(func),
  m_allzero(all_zero), m_PDT(PDT), m_DT(DT), m_RI(RI),
  m_zero(ConstantInt::get(m_func->getParent()->getContext(), APInt(1, 0))) {  }

bool FunctionSpecializer::shouldSpecialize(Region *reg) {

  // In case we are specializing a single basic block
  V_ASSERT(m_skipped.find(reg) != m_skipped.end());
  V_ASSERT(m_skipped[reg].size() > 0);
  if (m_skipped[reg].size() < 2) {
    CodeMetrics Metrics;
    Metrics.analyzeBasicBlock(reg->getEntry());
    // Check if we are within the threashold
    if (Metrics.NumInsts < SpecializeThreshold) return false;
  }

  return true;
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
  V_ASSERT(ret && "unable to find return block");

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

void FunctionSpecializer::CollectDominanceInfo() {

  // If we did not enable specialization, do not collect any jump edges
  if (! EnableSpecialization) return;
  // We can't specialize code which has endless loop (not all blocks are
  // dominated by return block)
  if (! CanSpecialize()) return;

  for (Function::iterator bb = m_func->begin(), bbe = m_func->end(); bb != bbe ; ++bb) {
    Region *reg = m_RI->getRegionFor(bb);
    V_ASSERT(reg && "getRegionFor faile!");    
    if (reg->getExit() && reg->getEntry() && reg->isSimple()) {

    // Register region once
    if (m_region_lookup.find(reg) != m_region_lookup.end()) continue;

    // Region must have a single exit (exit block not part of region)
    if (!reg->getExit()->getSinglePredecessor()) continue;

    // Has single pred ? (yes, it is a simple region)
    BasicBlock *entry = reg->getEntry();
    // Loop over all preds to the first block in entry.
    // Some of the preds may be backedges. Look for edges from outside
    // the region. We need to have one.
    llvm::pred_iterator it = pred_begin(entry);
    llvm::pred_iterator e  = pred_end(entry);
    BasicBlock* head = NULL;     
    for (; it!=e; ++it) {
      if (reg->contains(*it)) continue;
      if (head) { V_ASSERT(false && "Multiple outside entries ? must have only two preds!"); }
      head = *it;
    }
    
    V_ASSERT(head && "no head!");
    if (!head) continue;

    BranchInst *br = dyn_cast<BranchInst>(head->getTerminator());
    V_ASSERT(br && "Must have branch inst");
    // Only specialize after condition
    if (SpecOnlyAfterBranch && (! br->isConditional())) continue;

    // Register the region
    m_region_lookup.insert(reg);
    m_region_list.push_back(reg);
    m_heads[reg] = head;
    BBVector skipped;
    findSkippedBlocks(reg, skipped);
    m_skipped[reg] = skipped;
    }// simple reg
  }
}

void FunctionSpecializer::specializeFunction() {

  for (std::vector<Region*>::iterator it = m_region_list.begin(),
      e = m_region_list.end(); it != e; ++it) {
      if (shouldSpecialize(*it)) {
        specializeEdge(*it);
        V_ASSERT(!verifyFunction(*m_func) && "I broke this module");
      }
  }

}

void FunctionSpecializer::registerSchedulingScopes(SchedulingScope& parent) {
  // for all regions
  for (std::vector<Region*>::iterator rit = m_region_list.begin(),
      re = m_region_list.end(); rit != re; ++rit) {
    Region* reg = *rit;
    // if we specialze this region
    if (! shouldSpecialize(reg)) continue;
    // create a new region
    V_ASSERT(m_skipped.find(reg) != m_skipped.end());
    BBVector skipped = m_skipped[reg];
    SchedulingScope *scp = new SchedulingScope(NULL);
    // insert all basic blocks
    for(BBVector::iterator bit = skipped.begin(), 
        be = skipped.end(); bit != be; ++bit) {
      scp->addBasicBlock(*bit);
    }
    // Make sure exit location is scheduled right after
    scp->addBasicBlock(reg->getExit());
    // register region with parent
    parent.addSubSchedulingScope(scp);
  }// regions
}

void FunctionSpecializer::addNewToRegion(BasicBlock* old, BasicBlock* fresh) {
  V_ASSERT(old && fresh);
  for (std::map<Region*, BBVector>::iterator it=m_skipped.begin(), e=m_skipped.end();it!=e;++it) {
    // if has old, add the new
    if (std::find(it->second.begin(), it->second.end(), old) != it->second.end()) {
      it->second.push_back(fresh); 
    }
  }
}

BasicBlock* FunctionSpecializer::createIntermediateBlock(
  BasicBlock* before, BasicBlock* after, const std::string& name) {
  V_ASSERT(std::find(pred_begin(after),pred_end(after), before) != pred_end(after) && "right order");
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

void FunctionSpecializer::specializeEdge(Region* reg) {

  V_ASSERT(!verifyFunction(*m_func) && "I broke this module");
  
  V_ASSERT(m_heads.find(reg) != m_heads.end() && "no head!");
  BasicBlock* head = m_heads[reg];
  BasicBlock* src = createIntermediateBlock(head, reg->getEntry(), "header");

  Value* mask = m_pred->getEdgeMask(head ,reg->getEntry());
  V_ASSERT(mask && "unable to find mask");

  BasicBlock* exitBlock = reg->getExit()->getSinglePredecessor();
  V_ASSERT(exitBlock);
  BasicBlock* footer = createIntermediateBlock(exitBlock, reg->getExit(), "footer");

  BBVector skipped = m_skipped[reg];
  
  // Get the list of PHINodes which we need to insert (for skipped instructions)
  std::vector<std::pair<Instruction* , std::set<Instruction*> > > vals;
  findValuesToPhi(reg,  vals);
  
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
  BranchInst::Create(footer, reg->getEntry(), call_allzero, src);
  branch->eraseFromParent();

  // Create the PHINodes for the split values
  for (std::vector<std::pair<Instruction* , std::set<Instruction*> > >::iterator it =
       vals.begin(), e = vals.end(); e != it ; ++it) {
    Instruction* inst = it->first;
    // create phi node
    V_ASSERT(!inst->getType()->isVoidTy() &&
           "Instruction must not have void type");
    PHINode* new_phi = PHINode::Create(
      inst->getType(), inst->getName() + "_spec", footer->begin());
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

  // In some cases, the selector of the errside-user would want to use
  // the mask from the skipped blocks. We need to zero this mask because
  // it is skipped (and known to be zero). We will use a phinode.
  V_ASSERT(m_skipped.find(reg) != m_skipped.end());
  for(BBVector::iterator bit = skipped.begin(), 
      be = skipped.end(); bit != be; ++bit) { 

    // If this is a 'masked block' (not a new block)
    Value* mask_target = m_pred->getInMask(*bit);

    if (mask_target) {
      Value* non_bypass_mask =
        new LoadInst(mask_target, mask_target->getName() + "_non_bypass",
      exitBlock->getTerminator());

      PHINode* new_phi =
        PHINode::Create(non_bypass_mask->getType(),
        mask_target->getName() + "_maskspec", footer->begin());

      new_phi->addIncoming(non_bypass_mask, exitBlock);
      new_phi->addIncoming(m_zero, src);
      new StoreInst(new_phi, mask_target, footer->getFirstNonPHI());
    }
  }

  // New start and end nodes are a part of this region
  addNewToRegion(reg->getEntry(), src);
  addNewToRegion(reg->getEntry(), footer);

  V_ASSERT(!verifyFunction(*m_func) && "I broke this module");
}
  
void FunctionSpecializer::findSkippedBlocks(Region* reg, BBVector& skipped) {
  
  V_ASSERT(reg && "bad region");
  
  for (Function::iterator it = m_func->begin(), e = m_func->end(); it != e ; ++it) {
    if (reg->contains(it)) { skipped.push_back(it); }
  } 
  skipped.push_back(reg->getEntry());
}

void FunctionSpecializer::findValuesToPhi(
  Region* reg, std::vector<std::pair<Instruction* , std::set<Instruction*> > > &to_add_phi) {

  V_ASSERT(m_skipped.find(reg) != m_skipped.end());
  BBVector skipped = m_skipped[reg];
  // Find values to protect behind a PHI
  // For each skipped BB
  for (BBVector::iterator BB = skipped.begin(), BBe = skipped.end(); BB != BBe; ++BB) {
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
          if (std::find(skipped.begin(), skipped.end(), iii->getParent()) == skipped.end())  { deps.insert(iii);  }
        }
      }
      if (!deps.empty()) {
        to_add_phi.push_back(std::pair<Instruction* , std::set<Instruction*> >(inst,deps));
      }
    } // for each bb
  } // for each bb
}

} // namesapce
