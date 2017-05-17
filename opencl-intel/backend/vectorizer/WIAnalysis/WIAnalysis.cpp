/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "WIAnalysis.h"
#include "Mangler.h"
#include "Predicator.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "CompilationUtils.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/PostDominators.h"

#include <string>
#include <stack>

namespace intel {

char WIAnalysis::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(WIAnalysis, "WIAnalysis", "WIAnalysis provides work item dependency info", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(SoaAllocaAnalysis)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(WIAnalysis, "WIAnalysis", "WIAnalysis provides work item dependency info", false, false)



static cl::opt<bool>
PrintWiaCheck("print-wia-check", cl::init(false), cl::Hidden,
  cl::desc("Debug wia-check analysis"));

const unsigned int WIAnalysis::MinIndexBitwidthToPreserve = 16;

/// Define shorter names for dependencies, for clarity of the conversion maps
#define UNI WIAnalysis::UNIFORM
#define SEQ WIAnalysis::CONSECUTIVE
#define PTR WIAnalysis::PTR_CONSECUTIVE
#define STR WIAnalysis::STRIDED
#define RND WIAnalysis::RANDOM

/// Dependency maps (define output dependency according to 2 input deps)

static const WIAnalysis::WIDependancy
add_conversion[WIAnalysis::NumDeps][WIAnalysis::NumDeps] = {
  /*          UNI, SEQ, PTR, STR, RND */
  /* UNI */  {UNI, SEQ, PTR, STR, RND},
  /* SEQ */  {SEQ, STR, STR, STR, RND},
  /* PTR */  {PTR, STR, STR, STR, RND},
  /* STR */  {STR, STR, STR, STR, RND},
  /* RND */  {RND, RND, RND, RND, RND}
};

static const WIAnalysis::WIDependancy
sub_conversion[WIAnalysis::NumDeps][WIAnalysis::NumDeps] = {
  /*          UNI, SEQ, PTR, STR, RND */
  /* UNI */  {UNI, STR, RND, RND, RND},
  /* SEQ */  {SEQ, RND, RND, RND, RND},
  /* PTR */  {PTR, RND, RND, RND, RND},
  /* STR */  {STR, RND, RND, RND, RND},
  /* RND */  {RND, RND, RND, RND, RND}
};


static const WIAnalysis::WIDependancy
mul_conversion[WIAnalysis::NumDeps][WIAnalysis::NumDeps] = {
  /*          UNI, SEQ, PTR, STR, RND */
  /* UNI */  {UNI, STR, STR, STR, RND},
  /* SEQ */  {STR, RND, RND, RND, RND},
  /* PTR */  {STR, RND, RND, RND, RND},
  /* STR */  {STR, RND, RND, RND, RND},
  /* RND */  {RND, RND, RND, RND, RND}
};

static const WIAnalysis::WIDependancy
select_conversion[WIAnalysis::NumDeps][WIAnalysis::NumDeps] = {
  /*          UNI, SEQ, PTR, STR, RND */
  /* UNI */  {UNI, STR, STR, STR, RND},
  /* SEQ */  {STR, SEQ, STR, STR, RND},
  /* PTR */  {STR, STR, PTR, STR, RND},
  /* STR */  {STR, STR, STR, STR, RND},
  /* RND */  {RND, RND, RND, RND, RND}
};

static const WIAnalysis::WIDependancy
gep_conversion[WIAnalysis::NumDeps][WIAnalysis::NumDeps] = {
  /* ptr\index UNI, SEQ, PTR, STR, RND */
  /* UNI */  {UNI, PTR, RND, RND, RND},
  /* SEQ */  {RND, RND, RND, RND, RND},
  /* PTR */  {PTR, RND, RND, RND, RND},
  /* STR */  {RND, RND, RND, RND, RND},
  /* RND */  {RND, RND, RND, RND, RND}
};

static const WIAnalysis::WIDependancy
gep_conversion_for_indirection[WIAnalysis::NumDeps][WIAnalysis::NumDeps] = {
  /* ptr\index UNI, SEQ, PTR, STR, RND */
  /* UNI */  {UNI, PTR, RND, RND, RND},
  /* SEQ */  {RND, RND, RND, RND, RND},
  /* PTR */  {STR, RND, RND, RND, RND},
  /* STR */  {RND, RND, RND, RND, RND},
  /* RND */  {RND, RND, RND, RND, RND}
};

WIAnalysis::WIAnalysis() : FunctionPass(ID), m_rtServices(NULL) {
    initializeWIAnalysisPass(*llvm::PassRegistry::getPassRegistry());
    m_vectorizedDim = 0;
}

WIAnalysis::WIAnalysis(unsigned int vectorizationDimension) :
  FunctionPass(ID), m_rtServices(NULL) {
  initializeWIAnalysisPass(*llvm::PassRegistry::getPassRegistry());
  m_vectorizedDim = vectorizationDimension;
}


bool WIAnalysis::runOnFunction(Function &F) {
  m_rtServices = getAnalysis<BuiltinLibInfo>().getRuntimeServices();
   V_ASSERT(m_rtServices && "Runtime services were not initialized!");

  if (! m_rtServices->orderedWI()) {
    return false;
  }

  // Obtain SoaAllocaAnalysis of the function
  m_soaAllocaAnalysis = &getAnalysis<SoaAllocaAnalysis>();
  V_ASSERT(m_soaAllocaAnalysis && "Unable to get pass");

  m_DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  assert(m_DT && "Unable to get DominatorTreeWrapperPass pass");
  m_PDT = &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
  assert(m_PDT && "Unable to get PostDominatorTreeWrapperPass pass");
  m_LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  assert(m_LI && "Unable to get LoopInfo pass");

  m_deps.clear();
  m_changed1.clear();
  m_changed2.clear();
  m_pChangedNew = &m_changed1;
  m_pChangedOld = &m_changed2;

  m_SchedulingConstraints.clear();

  // Compute the  first iteration of the WI-dep according to ordering
  // instructions this ordering is generally good (as it usually correlates
  // well with dominance).
  inst_iterator it = inst_begin(F);
  inst_iterator  e = inst_end(F);
  for (; it != e; ++it) {
    calculate_dep(&*it);
  }

  // Recursively check if WI-dep changes and if so recalculates
  // the WI-dep and marks the users for re-checking.
  // This procedure is guaranteed to converge since WI-dep can only
  // become less uniform (uniform->consecutive->ptr->stride->random).
  updateDeps();

  if(PrintWiaCheck) {
    outs() << F.getName().str() << "\n";
    for (it = inst_begin(F); it != e; ++it) {
      Instruction *I = &*it;
      outs()<<"WI-RunOnFunction " <<m_deps[I] <<" "<<*I <<" " << "\n";
    }
  }

  // Concatenate predicated regions, when possible, to guarantee that
  // predicated regions appear one after the other will still appear one after the other after linearization.
  // This is done in order to avoid cases where the linearizer accidently adds a non-conditional
  // branch from a predicated to a non-predicated node.
  // The concatenation is done to support non nested divergent branches that appear one after the other in the
  // same nesting level.
  // This code cannot get into an infinite loop because an influence region of a divergent branch span from the branch to
  // its immediate post dominator.
  for (SchdConstMap::iterator itr = m_SchedulingConstraints.begin();
           itr != m_SchedulingConstraints.end();
           ++itr) {
    std::vector<BasicBlock*> & dst = itr->second;
    // As long as the post dom's terminator starts a maximal divergent region
    while (m_SchedulingConstraints.count(dst.back())) {
      std::vector<BasicBlock*> src = m_SchedulingConstraints.find(dst.back())->second;
      // we add the influence region blocks started at the post dom to the dst
      dst.insert(dst.end(), src.begin(), src.end());
    }
  }

  return false;
}

void WIAnalysis::updateDeps() {

  // As long as we have values to update
  while(!m_pChangedNew->empty()) {
    // swap between changedSet pointers - recheck the newChanged(now old)
    std::swap(m_pChangedNew, m_pChangedOld);
    // clear the newChanged set so it will be filled with the users of
    // instruction which their WI-dep canged during the current iteration
    m_pChangedNew->clear();
    // update all changed values
    std::set<const Value*>::iterator it = m_pChangedOld->begin();
    std::set<const Value*>::iterator e = m_pChangedOld->end();
    for(; it != e; ++it) {
      // remove first instruction
      // calculate its new dependencey value
      calculate_dep(*it);
    }
  }
}

WIAnalysis::WIDependancy WIAnalysis::whichDepend(const Value* val){
  V_PRINT("WIA","Asking about "<<*val<<"\n");
  if (! m_rtServices->orderedWI()) {
    V_PRINT("WIA","Random!!\n");
    if(PrintWiaCheck) {
        outs()<<"whichDepend function "<< "WIA" <<"Random!!"<<"4"<< "\n";
    }

    return WIAnalysis::RANDOM;
  }
  V_ASSERT(m_pChangedNew->empty() && "set should be empty before query");
  V_ASSERT(val && "Bad value");
  if (m_deps.find(val) == m_deps.end()) {
    bool isInst = isa<Instruction>(val);

    if (isInst) return WIAnalysis::RANDOM;
    return WIAnalysis::UNIFORM;
  }
  V_PRINT("WIA","It is "<<m_deps[val]<<"\n");
  if(PrintWiaCheck) {
    outs()<<"whichDepend function "<< "WIA " <<m_deps[val] <<" "<<*val <<" " << "\n";
  }
  return m_deps[val];
}

void WIAnalysis::setDepend(const Instruction* from, const Instruction* to) {
  assert(from && to && "Bad instruction");
  assert(m_deps.find(from) != m_deps.end() && "Can't find from in m_deps");

  const BasicBlock * fromBB = from->getParent();
  const BasicBlock * toBB = to->getParent();

  if (m_divBlocks.count(fromBB))
    m_divBlocks.insert(toBB);

  if (m_divPhiBlocks.count(fromBB))
    m_divPhiBlocks.insert(toBB);

  m_deps[to] = m_deps[from];
}

bool WIAnalysis::isDivergentBlock(BasicBlock *BB) {
  return m_divBlocks.count(BB);
}

bool WIAnalysis::isDivergentPhiBlocks(BasicBlock *Phi) {
  return m_divPhiBlocks.count(Phi);
}

SchdConstMap & WIAnalysis::getSchedulingConstraints() {
  return m_SchedulingConstraints;
}

void WIAnalysis::invalidateDepend(const Value* val){
  if (m_deps.find(val) != m_deps.end()) {
    m_deps.erase(val);
  }
}

WIAnalysis::WIDependancy WIAnalysis::getDependency(const Value *val) {

  if (m_deps.find(val) == m_deps.end()) {
    m_deps[val] = WIAnalysis::UNIFORM;
  }
  return m_deps[val];
}

bool WIAnalysis::hasDependency(const Value *val) {

  if (!isa<Instruction>(val)) return true;
  return m_deps.count(val);
}

void WIAnalysis::calculate_dep(const Value* val) {
  V_ASSERT(val && "Bad value");

  // Not an instruction, must be a constant or an argument
  // Could this vector type be of a constant which
  // is not uniform ?
  assert(isa<Instruction>(val) && "Could we reach here with non instruction value?");

  const Instruction* inst = dyn_cast<Instruction>(val);
  V_ASSERT(inst && "This Value is not an Instruction");

  // We only calculate dependency on unset instructions if all their operands
  // were already given dependency. This is good for compile time since these
  // instructions will be visited again after the operands dependency is set.
  // An exception are phi nodes since they can be the ancestor of themselves in
  // the def-use chain. Note that in this case we force the phi to have the
  // pre header value already calculated.
  if (!hasDependency(inst)) {
    unsigned int unsetOpNum = 0;
    for(unsigned i=0; i<inst->getNumOperands(); ++i) {
      if (!hasDependency(inst->getOperand(i))) unsetOpNum++;
    }
    if (isa<PHINode>(inst)) {
      // We do not calculate PhiNode with all incoming values unset
      if(unsetOpNum == inst->getNumOperands()) return;
    }
    else {
      // We do not calculate non-PhiNode instruction that have unset operands
      if(unsetOpNum > 0) return;
    }
  }

  // Our initial value
  bool hasOriginal = hasDependency(inst);
  WIDependancy orig = hasOriginal ? getDependency(inst) : WIAnalysis::UNIFORM;
  WIDependancy dep = orig;

  if (orig == WIAnalysis::RANDOM)
    return;

  // LLVM does not have compile time polymorphisms
  // TODO: to make things faster we may want to sort the list below according
  // to the order of their probability of appearance.
  if      (const BinaryOperator *BI = dyn_cast<BinaryOperator>(inst))         dep = calculate_dep(BI);
  else if (const CallInst *CI = dyn_cast<CallInst>(inst))                     dep = calculate_dep(CI);
  else if (isa<CmpInst>(inst))                                                dep = calculate_dep_simple(inst);
  else if (isa<ExtractElementInst>(inst))                                     dep = calculate_dep_simple(inst);
  else if (const GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(inst))  dep = calculate_dep(GEP);
  else if (isa<InsertElementInst>(inst))                                      dep = calculate_dep_simple(inst);
  else if (isa<InsertValueInst>(inst))                                        dep = calculate_dep_simple(inst);
  else if (const PHINode *Phi = dyn_cast<PHINode>(inst))                      dep = calculate_dep(Phi);
  else if (isa<ShuffleVectorInst>(inst))                                      dep = calculate_dep_simple(inst);
  else if (isa<StoreInst>(inst))                                              dep = calculate_dep_simple(inst);
  else if (const TerminatorInst *TI = dyn_cast<TerminatorInst>(inst))         dep = calculate_dep(TI);
  else if (const SelectInst *SI = dyn_cast<SelectInst>(inst))                 dep = calculate_dep(SI);
  else if (const AllocaInst *AI = dyn_cast<AllocaInst>(inst))                 dep = calculate_dep(AI);
  else if (const CastInst *CI = dyn_cast<CastInst>(inst))                     dep = calculate_dep(CI);
  else if (isa<ExtractValueInst>(inst))                                       dep = calculate_dep_simple(inst);
  else if (isa<LoadInst>(inst))                                               dep = calculate_dep_simple(inst);
  else if (const VAArgInst *VAI = dyn_cast<VAArgInst>(inst))                  dep = calculate_dep(VAI);

  updateDepMap(inst, dep);
}

// Find divergent partial joins
void WIAnalysis::findDivergePartialJoins(const TerminatorInst *inst) {
  assert(inst && "inst cannot be null");
  assert(dyn_cast<BranchInst>(inst) && dyn_cast<BranchInst>(inst)->isConditional() && "branch has to be a conditional branch");
  assert(inst->getNumSuccessors() == 2 && "supports only for conditional branches with two successors");

  for (SmallPtrSet<BasicBlock*, 4>::iterator blkItr = m_partialJoins.begin();
       blkItr != m_partialJoins.end();
       ++blkItr) {

    BasicBlock * partialJoin = *blkItr;

    DenseSet<BasicBlock*> leftSet, rightSet;
    std::stack<BasicBlock*> workSet;


    // If this partial join does not contain phi nodes then go to the next one
    BasicBlock::iterator firstInst = partialJoin->begin();
    if (!isa<PHINode>(dyn_cast<Instruction>(firstInst)))
      continue;

    for (int i=0; i < 2; ++i) { // inst->getNumSuccessors() == 2

      if (inst->getSuccessor(i) != partialJoin) {
        workSet.push(inst->getSuccessor(i));

        while (!workSet.empty()) {
          BasicBlock *curBlk = workSet.top();
          workSet.pop();

          DenseSet<BasicBlock*> & blkSet = (i == 0) ? leftSet : rightSet;

          blkSet.insert(curBlk);

          for (succ_iterator SI = succ_begin(curBlk), E = succ_end(curBlk); SI != E; ++SI) {
            BasicBlock *succBlk = (*SI);
            if (succBlk != partialJoin && !blkSet.count(succBlk)) {
              workSet.push(succBlk);
            }
          }
        }
      }
    }

    bool reachRight = 0, reachLeft = 0;
    for (pred_iterator itr = pred_begin(partialJoin); itr != pred_end(partialJoin); ++itr) {
      BasicBlock * pred = *itr;

      bool isRight = rightSet.count(pred);
      bool isLeft = leftSet.count(pred);

      // If we saw a path from the left succ of cbr to a predecessor
      // and now we see a path from the right succ to a different one.
      // Or the other way around ...
      if ((isRight && reachLeft) || (isLeft && reachRight)) {
        m_divergePartialJoins.insert(partialJoin);
        break;
      }

      reachRight |= isRight;
      reachLeft  |= isLeft;
    }
  }
}

static bool phiHasEqualIncomingValues(PHINode* phi) {
  for (unsigned int i = 1; i < phi->getNumIncomingValues(); i++) {
    if (phi->getIncomingValue(0) != phi->getIncomingValue(i)) {
      return false;
    }
  }
  return true;
}

// Mark each phi node in join or a partial join as divergent
void WIAnalysis::markDependentPhiRandom() {

  // full join
  // Note that a branch can have null immediate post-dominator
  // when a function has multiple exits in llvm-ir
  if (m_fullJoin) {
    m_divPhiBlocks.insert(m_fullJoin);
    for (BasicBlock::iterator instItr = m_fullJoin->begin();
        instItr != m_fullJoin->end();
        ++instItr) {

      if (PHINode* phi = dyn_cast<PHINode>(instItr)) {
         if (!phiHasEqualIncomingValues(phi)) {
           updateDepMap(phi, WIAnalysis::RANDOM);
         }
      }
      else {
        break;
      }
    }
  }

  // partial joins
  for (SmallPtrSet<BasicBlock*, 4>::iterator blkItr = m_divergePartialJoins.begin();
       blkItr != m_divergePartialJoins.end();
       ++blkItr) {
    m_divPhiBlocks.insert((*blkItr));
    for (BasicBlock::iterator instItr = (*blkItr)->begin();
        instItr != (*blkItr)->end();
        ++instItr) {
      if (PHINode* phi = dyn_cast<PHINode>(instItr)) {
         if (!phiHasEqualIncomingValues(phi)) {
           updateDepMap(phi, WIAnalysis::RANDOM);
         }
      }
      else {
        break;
      }
    }
  }
}

void WIAnalysis::updateCfDependency(const TerminatorInst *inst) {
  BasicBlock *blk = (BasicBlock *)(inst->getParent());

  // If the root block is marked as divergent then we should not add
  // scheduling constraints for this region because it is part of a larger region
  // that is going to be predicated.
  // If we will add every predicated region then we might get a conflict at the linearizer
  // that caused by commoning.
  bool shouldUpdateConstraints = !isDivergentBlock(blk);

  calcInfoForBranch(inst);

  findDivergePartialJoins(inst);

  // Mark each phi node in a join or a partial join as divergent
  markDependentPhiRandom();

  // walk through all the instructions in the influence-region
  for(DenseSet<BasicBlock*>::iterator blkItr = m_influenceRegion.begin();
    blkItr != m_influenceRegion.end();
    ++blkItr) {
    BasicBlock *defBlk = *blkItr;


    // A node in the influence region of a divergent branch may not have
    // an incoming edge from a non-divergent block (unless its the immediate post
    // dominator which contains a random terminator).
    // Therefore, in case such an edge exists we need to mark the terminator of
    // the immediate dominator of the edge successor as random.
    // The only exception that this is allowed is in loops and because loops has
    // preheaders with non-conditional branches then the following will work
    // for these as well.
    for (pred_iterator itr = pred_begin(defBlk); itr != pred_end(defBlk); ++itr) {
      if (!isDivergentBlock(*itr) && (*itr != blk)) {
        // Because defBlk is divergent and *itr is not then the idom of defBlk
        // should also be a dom of *itr and therefore, such a dominator exists
        assert(m_DT->getNode(defBlk) && m_DT->getNode(defBlk)->getIDom() && "dominator cannot be null");
        BasicBlock *immDom = m_DT->getNode(defBlk)->getIDom()->getBlock();
        assert(immDom && "immDom cannot be null");

        TerminatorInst* term = immDom->getTerminator();
        BranchInst* br = dyn_cast<BranchInst>(term);
        assert(br && "br cannot be null");

        // allones and allzeroes branches are uniform.
        bool branchIsAllOnes = (Predicator::getAllOnesBranch(br->getParent()) != NULL);
        CallInst* callInst =
          br->isConditional() ? dyn_cast<CallInst>(br->getCondition()) : NULL;
        bool branchIsAllZeroes = callInst &&
          callInst->getCalledFunction() &&
          Mangler::isAllZero(callInst->getCalledFunction()->getName());

        if (br->isConditional() && !branchIsAllOnes && !branchIsAllZeroes) {
          updateDepMap(term, WIAnalysis::RANDOM);
          // This region is going to be part of a larger region that is going
          // to be predicated
          shouldUpdateConstraints = false;
        }

        break;
      }
    }

    for (BasicBlock::iterator I = defBlk->begin(), E = defBlk->end(); I != E; ++I) {
      Instruction *defInst = &*I;

      // If defInst is random then its randomness will propagate to its usages
      // in the regular way and no control flow info propagation is needed
      if (hasDependency(defInst) && getDependency(defInst) == WIAnalysis::RANDOM) {
        continue;
      }

      // look at the users
      for (Value::user_iterator useItr = defInst->user_begin();
          useItr != defInst->user_end();
          ++useItr) {

        Instruction *userInst = dyn_cast<Instruction>(*useItr);
        if (!userInst) {
          continue;
        }

        BasicBlock *useBlk = userInst->getParent();
        if (useBlk == defBlk) {
          // local def-use, not related to control-dependence
          continue; // check the next use
        }

        if (useBlk == m_fullJoin ||
            m_partialJoins.count(useBlk)) {

          // We can check whether the (partial) join is a loop exit and change the algorithm
          // This might increase accuracy in case there are gotos but seems like
          // redundant computation for our case.
          // For now we'll mark a usage in every join/partial join as random
          // We might change it in the future.

          updateDepMap(userInst, WIAnalysis::RANDOM);

        }
        else {
          // Mark each usage not in the influence region as random
          if (! m_influenceRegion.count(useBlk)){
            updateDepMap(userInst, WIAnalysis::RANDOM);
          }
        }
      }
    }
  }

  if (!shouldUpdateConstraints) {
    m_SchedulingConstraints.erase(blk);
  }

  m_influenceRegion.clear();
}

void WIAnalysis::updateDepMap(const Instruction *inst, WIAnalysis::WIDependancy dep)
{
  // If the value was changed
  if (!hasDependency(inst) || dep!=getDependency(inst)) {
    // Save the new value of this instruction
    m_deps[inst] = dep;
    // Register for update all of the dependent values of this updated
    // instruction.
    Value::const_user_iterator useItr = inst->user_begin();
    Value::const_user_iterator useEnd  = inst->user_end();
    for (; useItr != useEnd; ++useItr) {
      m_pChangedNew->insert(*useItr);
    }

    // divergent branch, trigger updates due to control-dependence
    const TerminatorInst *term = dyn_cast<TerminatorInst>(inst);
    if (term && dep != WIAnalysis::UNIFORM) {
      const BranchInst* br = dyn_cast<BranchInst>(term);
      if (br && br->isConditional()) {
        m_divBranchesQueue.push(br);
        // Due to data structures sharing, every divergent branch should
        // be handled separately. Therefore, we use a queue to guarantee that
        // newly random branches, discovered during branch divergent propagation,
        // are propagated only on termination of the previous divergent branch
        // propagation.
        if (m_divBranchesQueue.size() == 1) {
          do {
            br = m_divBranchesQueue.front();
            updateCfDependency(br);
            m_divBranchesQueue.pop();
          } while(m_divBranchesQueue.size() != 0);
        }
      }
    }
  }
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep_simple(const Instruction *I) {
  // simply check that all operands are uniform, if so return uniform, else random
  const unsigned nOps = I->getNumOperands();
  for (unsigned i=0; i<nOps; ++i) {
    const Value *op = I->getOperand(i);
    WIAnalysis::WIDependancy dep = getDependency(op);
    if (dep != WIAnalysis::UNIFORM)
      return WIAnalysis::RANDOM;
  }
  return WIAnalysis::UNIFORM;
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const BinaryOperator* inst) {
  // Calculate the dependency type for each of the operands
  Value *op0 = inst->getOperand(0);
  Value *op1 = inst->getOperand(1);

  WIAnalysis::WIDependancy dep0 = getDependency(op0);
  WIAnalysis::WIDependancy dep1 = getDependency(op1);

  // For whatever binary operation,
  // uniform returns uniform
  if ( WIAnalysis::UNIFORM == dep0 && WIAnalysis::UNIFORM == dep1) {
    return WIAnalysis::UNIFORM;
  }

  // FIXME:: assumes that the X value does not cross the +/- border - risky !!!
  // The pattern (and (X, C)), where C preserves the lower k bits of the value,
  // is often used for truncating of numbers in 64bit. We assume that the index
  // properties are not hurt by this.
  if (inst->getOpcode() == Instruction::And) {
    ConstantInt *C0 = dyn_cast<ConstantInt>(inst->getOperand(0));
    ConstantInt *C1 = dyn_cast<ConstantInt>(inst->getOperand(1));
    // Use any of the constants. Instcombine places constants on Op1
    // so try Op1 first.
    if (C1 || C0) {
      ConstantInt *C = C1 ? C1 : C0;
      WIAnalysis::WIDependancy dep = C1 ? dep0 : dep1;
      // Cannot look at bit pattern of huge integers.
      if (C->getBitWidth() < 65) {
        uint64_t val = C->getZExtValue();
        uint64_t ptr_mask = (1<<MinIndexBitwidthToPreserve)-1;
        // Zero all bits above the lower k bits that we are interested in
        val &= (ptr_mask);
        // Make sure that all of the remaining bits are active
        if (val == ptr_mask) { return dep; }
      }
    }
  }

  // FIXME:: assumes that the X value does not cross the +/- border - risky !!!
  // The pattern (ashr (shl X, C)C) is used for truncating of numbers in 64bit
  // The constant C must leave at least 32bits of the original number
  if (inst->getOpcode() == Instruction::AShr) {
    BinaryOperator* SHL = dyn_cast<BinaryOperator>(inst->getOperand(0));
    // We also allow add of uniform value between the ashr and shl instructions
    // since instcombine creates this pattern when adding a constant.
    // The shl forces all low bits to be zero, so there can be no carry to the
    // high bits due to the addition. Addition with uniform preservs WI-dep.
    if (SHL && SHL->getOpcode() == Instruction::Add) {
      Value *addedVal = SHL->getOperand(1);
      if (getDependency(addedVal) == WIAnalysis::UNIFORM) {
        SHL = dyn_cast<BinaryOperator>(SHL->getOperand(0));
      }
    }

    if (SHL && SHL->getOpcode() == Instruction::Shl) {
      ConstantInt * c_ashr = dyn_cast<ConstantInt>(inst->getOperand(1));
      ConstantInt * c_shl  = dyn_cast<ConstantInt>(SHL->getOperand(1));
      const IntegerType *AshrTy = cast<IntegerType>(inst->getType());
      if (c_ashr && c_shl && c_ashr->getZExtValue() == c_shl->getZExtValue()) {
        // If wordWidth - shift_width >= 32 bits
        if ((AshrTy->getBitWidth() - c_shl->getZExtValue()) >= MinIndexBitwidthToPreserve ) {
          // return the dep of the original X
          return getDependency(SHL->getOperand(0));
        }
      }
    }
  }

  switch (inst->getOpcode()) {
    // Addition simply adds the stride value, except for ptr_consecutive
    // which is promoted to strided.
    // Another exception is when we subtract the tid: 1 - X which turns the
    // tid order to random.
  case Instruction::Add:
  case Instruction::FAdd:
    return add_conversion[dep0][dep1];
  case Instruction::Sub:
  case Instruction::FSub:
    return sub_conversion[dep0][dep1];

  case Instruction::Mul:
  case Instruction::FMul:
  case Instruction::Shl:
    if ( WIAnalysis::UNIFORM == dep0 || WIAnalysis::UNIFORM == dep1) {
      // If one of the sides is uniform, then we can adopt
      // the other side (stride*uniform is still stride).
      // stride size is K, where K is the uniform input.
      // An exception to this is ptr_consecutive, which is
      // promoted to strided.
      return mul_conversion[dep0][dep1];
    }
  default:
    //TODO: Support more arithmetic if needed
    return WIAnalysis::RANDOM;
  }
  return WIAnalysis::RANDOM;
}

using namespace Intel::OpenCL::DeviceBackend;

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const CallInst* inst) {
  //TODO: This function requires much more work, to be correct:
  //   2) Some functions (dot_prod, cross_prod) provide "measurable"
  //   behavior (Uniform->strided).
  //   This information should also be obtained from RuntimeServices somehow.

  // Check if call is TID-generator
  bool err, isTidGen;
  unsigned dim = 0;
  isTidGen = m_rtServices->isTIDGenerator(inst, &err, &dim);
  // We do not vectorize TID with variable dimension
  V_ASSERT((!err) && "TIDGen inst receives non-constant input. Cannot vectorize!");
  // All WI's are consecutive along the zero dimension
  if (isTidGen && dim == m_vectorizedDim) return WIAnalysis::CONSECUTIVE;

  // Check if function is declared inside "this" module
  if (!inst->getCalledFunction()->isDeclaration()) {
    // For functions defined in this module - it is unsafe to assume anything
    return WIAnalysis::RANDOM;
  }

  // Check if the function is in the table of functions
  Function *origFunc = inst->getCalledFunction();
  std::string origFuncName = origFunc->getName().str();

  if (CompilationUtils::isWorkGroupBuiltin(origFuncName)) {
    // WG functions must be packetized (although their results may be uniform)
    return WIAnalysis::RANDOM;
  }

  // Check if the function is in the table of functions
  std::string scalarFuncName = origFuncName;

  // If it is a fake builtin then we might need to demangle
  // it before demangling the fake part.
  if (Mangler::isFakeBuiltin(scalarFuncName)) {
    std::string fakeFuncName = scalarFuncName;
    if(Mangler::isMangledCall(fakeFuncName)) {
        fakeFuncName = Mangler::demangle(fakeFuncName);
    }
    scalarFuncName = Mangler::demangle_fake_builtin(fakeFuncName);
  }

  bool isMangled = Mangler::isMangledCall(scalarFuncName);
  bool MaskedMemOp = (Mangler::isMangledLoad(scalarFuncName) ||
                      Mangler::isMangledStore(scalarFuncName));

  // First remove any name-mangling (for example, masking), from the function name
  if (isMangled) {
    scalarFuncName = Mangler::demangle(scalarFuncName);
  }

  // Check with the runtime whether we can say that the output of the call
  // is uniform in case all it's operands are uniform.
  // Note that for openCL the runtime will say it is true for: get_gloabl_id,
  // get_local_id, since on dimension 0 the isTIDGenerator should answer true,
  // and we will say the value is Consecutive. So in here we cover dimensions 1,2
  // which are uniform.
  bool UniformByOps = m_rtServices->hasNoSideEffect(scalarFuncName);

  // Look for the function in the builtin functions hash
  if (!MaskedMemOp && !isTidGen && !UniformByOps) {
    return WIAnalysis::RANDOM;
  }

  // Iterate over all input dependencies. If all are uniform - propagate it.
  // otherwise - return RANDOM
  unsigned numParams = inst->getNumArgOperands();

  bool isAllUniform = true;
  for (unsigned i = 0; i < numParams; ++i)
  {
    // Operand 0 is the function's name
    Value* op = inst->getArgOperand(i);
    WIDependancy dep = getDependency(op);
    if (WIAnalysis::UNIFORM != dep) {
      isAllUniform = false;
      break; // Uniformity check failed. no need to continue
    }
  }

  // An allones and allzeroes branches are uniform branch.
  if (Mangler::isAllOne(origFuncName) || Mangler::isAllZero(origFuncName)) {
    return WIAnalysis::UNIFORM;
  }

  if (isAllUniform) return WIAnalysis::UNIFORM;
  return WIAnalysis::RANDOM;
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const GetElementPtrInst* inst) {
  // running over the all indices arguments except for the last
  // here we assume the pointer is the first operand
  unsigned num = inst->getNumIndices();
  for (unsigned i=1; i < num; ++i) {
    const Value* op = inst->getOperand(i);
    WIAnalysis::WIDependancy dep = getDependency(op);
    if (dep != WIAnalysis::UNIFORM) {
      return WIAnalysis::RANDOM;
    }
  }
  const Value* opPtr = inst->getOperand(0);
  WIAnalysis::WIDependancy depPtr = getDependency(opPtr);

  const Value* lastInd = inst->getOperand(num);
  WIAnalysis::WIDependancy lastIndDep = getDependency(lastInd);

  // SOA Alloca related pointer will be turned into SOA format.
  // Thus, it is allowed to assume: PTR + UNI = PTR
  const bool  isIndirectGep = opPtr->getType() != inst->getType() &&
    !m_soaAllocaAnalysis->isSoaAllocaScalarRelated(opPtr);

  if (isIndirectGep)
    return gep_conversion_for_indirection[depPtr][lastIndDep];
  else
    return gep_conversion[depPtr][lastIndDep];
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const PHINode* inst) {
  unsigned num = inst->getNumIncomingValues();
  std::vector<WIDependancy> dep;
  for (unsigned i=0; i < num; ++i) {
    // For phi we ignore unset incoming values, so cases
    // like loop with consecutive variable that is increased
    // by uniform will be considered consecutive.
    Value* op = inst->getIncomingValue(i);
    if (hasDependency(op)) dep.push_back(getDependency(op));
  }
  assert(dep.size() > 0 && "We should not reach here with All incoming values are unset");

  WIDependancy totalDep = dep[0];
  for (unsigned i=1; i < dep.size(); ++i)
  {
    totalDep = select_conversion[totalDep][dep[i]];
  }

  return totalDep;
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const TerminatorInst* inst) {
  // Instruction has no return value
  // Just need to know if this inst is uniform or not
  // because we may want to avoid predication if the control flows
  // in the function are uniform...
  switch (inst->getOpcode())
  {
  case Instruction::Br:
    {
      const BranchInst * brInst = cast<BranchInst>(inst);
      if (brInst->isConditional())
      {
        // Conditional branch is uniform, if its condition is uniform
        Value* op = brInst->getCondition();
        WIAnalysis::WIDependancy dep = getDependency(op);
        if ( WIAnalysis::UNIFORM == dep ) {
          return WIAnalysis::UNIFORM;
        }
        return WIAnalysis::RANDOM;
      }
      // Unconditional branch is non TID-dependent
      return WIAnalysis::UNIFORM;
    }
  //Return instructions are unconditional
  case Instruction::Ret:
    return WIAnalysis::UNIFORM;
  case Instruction::IndirectBr:
    // TODO: Define the dependency requirements of indirectBr
  case Instruction::Switch:
    // TODO: Should this depend only on the condition, like branch?
  default:
    return WIAnalysis::RANDOM;
  }
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const SelectInst* inst) {
  Value* op0 = inst->getOperand(0); // mask
  WIAnalysis::WIDependancy dep0 = getDependency(op0);
  if (WIAnalysis::UNIFORM == dep0) {
    Value* op1 = inst->getOperand(1);
    Value* op2 = inst->getOperand(2);
    WIAnalysis::WIDependancy dep1 =getDependency(op1);
    WIAnalysis::WIDependancy dep2 =getDependency(op2);
    // In case of constant scalar select we can choose according to the mask.
    if (ConstantInt *C = dyn_cast<ConstantInt>(op0)) {
      uint64_t val = C->getZExtValue();
      if (val) return dep1;
      else return dep2;
    }
    // Select the "weaker" dep, but if only one dep is ptr_consecutive,
    // it must be promoted to strided ( as this data may
    // propagate to Load/Store instructions.
    return select_conversion[dep1][dep2];
  }
  // In case the mask is non-uniform the select outcome can be a combination
  // so we don't know nothing about it.
  return WIAnalysis::RANDOM;
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const AllocaInst* inst) {
  // Check if alloca instruction can be converted to SOA-alloca
  if( m_soaAllocaAnalysis->isSoaAllocaScalarRelated(inst) ) {
    return WIAnalysis::PTR_CONSECUTIVE;
  }
  return WIAnalysis::RANDOM;
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const CastInst* inst) {
  Value* op0 = inst->getOperand(0);
  WIAnalysis::WIDependancy dep0 = getDependency(op0);

  // independent remains independent
  if (WIAnalysis::UNIFORM == dep0) return dep0;

  switch (inst->getOpcode())
  {
  case Instruction::SExt:
  case Instruction::FPTrunc:
  case Instruction::FPExt:
  case Instruction::PtrToInt:
  case Instruction::IntToPtr:
  case Instruction::AddrSpaceCast: // [LLVM 3.6 UPGRADE] TODO: make sure this line is functionally correct
  case Instruction::UIToFP:
  case Instruction::FPToUI:
  case Instruction::FPToSI:
  case Instruction::SIToFP:
      return dep0;
  case Instruction::BitCast:
  case Instruction::ZExt:
    return WIAnalysis::RANDOM;
  // FIXME:: assumes that the value does not cross the +/- border - risky !!!!
  case Instruction::Trunc: {
    const Type *destType = inst->getDestTy();
    const IntegerType *intType = dyn_cast<IntegerType>(destType);
    if (intType && (intType->getBitWidth() >= MinIndexBitwidthToPreserve)) {
      return dep0;
    }
    return WIAnalysis::RANDOM;
  }
  default:
    V_ASSERT(false && "no such opcode");
    // never get here
    return WIAnalysis::RANDOM;
  }
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const VAArgInst* inst) {
  V_ASSERT(false && "Are we supporting this ??");
  return WIAnalysis::RANDOM;
}


// Calculating the influence region, partial joins, and block uniformity
void WIAnalysis::calcInfoForBranch(const TerminatorInst *inst)
{
  assert(inst && "inst cannot be null");
  assert(dyn_cast<BranchInst>(inst) && dyn_cast<BranchInst>(inst)->isConditional() && "branch has to be a conditional branch");
  assert(inst->getNumSuccessors() == 2 && "supports only for conditional branches with two successors");

  DomTreeNode * postDomNode = m_PDT->getNode((BasicBlock*)(inst->getParent()));

 // If we are in an infinite loop then there is no post-dominant
 // In this case, we mark everything reachable from the divergent branch as its influence region (conservative)
  if (postDomNode)  {
    // Because inst is a conditional branch then it is not the last basic block
    // and therefore getIDom does not return null
    assert(postDomNode->getIDom() != 0 && "Post dominator cannot be null");
    m_fullJoin = postDomNode->getIDom()->getBlock();
  }
  else {
    m_fullJoin = 0;
  }

  bool updatedFullJoin = true;

  std::vector<BasicBlock*> schedConstraints;

  // iterate until we do not need to recalculate the full join
  while (updatedFullJoin) {

    updatedFullJoin = false;

    m_influenceRegion.clear();
    m_partialJoins.clear();
    m_divergePartialJoins.clear();
    schedConstraints.clear();

    // adding the root of the predicated region for the scheduling constraints
    schedConstraints.push_back((BasicBlock*) inst->getParent());

    Loop *fullJoinLoop = m_LI->getLoopFor(m_fullJoin);
    SmallPtrSet<BasicBlock *, 4> fullJoinLoopLatches;

    if (fullJoinLoop) {
      for (pred_iterator itr = pred_begin(fullJoinLoop->getHeader());
          itr != pred_end(fullJoinLoop->getHeader());
          ++itr) {
        if (fullJoinLoop->contains(*itr))
          fullJoinLoopLatches.insert(*itr);
      }
    }

    DenseSet<BasicBlock*> leftSet, rightSet;
    std::stack<BasicBlock*> workSet;

    for (int i=0; i < 2; ++i) { // inst->getNumSuccessors() == 2

      if (inst->getSuccessor(i) != m_fullJoin) {

        workSet.push(inst->getSuccessor(i));

        while (!workSet.empty()) {
          BasicBlock *curBlk = workSet.top();
          workSet.pop();

          m_divBlocks.insert(curBlk); // mark block as divergent

          Loop * curLoop = m_LI->getLoopFor(curBlk);

          // If full join is in the curLoop and we reached the latch node of this loop
          // Then we should abort, update the full join by finding its first post-dominator which
          // is outside curLoop and then recalculate new info for this branch.
          if ((fullJoinLoop) && (fullJoinLoop == curLoop)
               && fullJoinLoopLatches.count(curBlk)) {
            updatedFullJoin = true;
            break;
          }

          if (i == 1 && leftSet.count(curBlk))
            m_partialJoins.insert(curBlk);

          DenseSet<BasicBlock*> & blkSet = (i == 0) ? leftSet : rightSet;

          blkSet.insert(curBlk);

          if (! m_influenceRegion.count(curBlk)) {
            m_influenceRegion.insert(curBlk);
            schedConstraints.push_back(curBlk);
          }

          for (succ_iterator SI = succ_begin(curBlk), E = succ_end(curBlk); SI != E; ++SI) {
            BasicBlock *succBlk = (*SI);
            if (succBlk != m_fullJoin && !blkSet.count(succBlk)) {
              workSet.push(succBlk);
            }
          }
        }
      }

      if (updatedFullJoin)
        break;
    }

    // If we need to update fullJoin it means that during computing the influence region
    // we have reached the latch node of the loop where the full join is located in.
    // In this case, in order to be sound, we should move the full join to the full join's
    // first post-dominator outside its loop and start the computation from the beginning.
    // This process can also be calculated incrementally by continue the calculation from
    // the current full join.

    if (updatedFullJoin) {
      BasicBlock * nextFullJoin = m_fullJoin;
      Loop *nextFullJoinLoop = 0;

      // find the first full join's post-dominator outside the post dominator's loop
      do {
        DomTreeNode * postDomNode = m_PDT->getNode((BasicBlock*)nextFullJoin);
        // if updatedFullJoin is true then we are not in an infinite loop and therefore, getNode
        // does not return null
        assert(postDomNode && "getNode should not return null");
        // If the post dom is inside the loop then it cannot be the last block and therefore,
        // getIDom does not return null
        assert(postDomNode->getIDom() && "getIDom should not return null");
        nextFullJoin = postDomNode->getIDom()->getBlock();
        nextFullJoinLoop = m_LI->getLoopFor(nextFullJoin);
      } while (nextFullJoinLoop == fullJoinLoop);

      m_fullJoin = nextFullJoin;
      fullJoinLoop = nextFullJoinLoop;
    }
  }

  schedConstraints.push_back(m_fullJoin);
  m_SchedulingConstraints[*(schedConstraints.begin())] = schedConstraints;

}

void WIAnalysis::print(raw_ostream &OS, const Module *M) const {
  if ( !M ) {
    OS << "No Module!\n";
    return;
  }
  //Print Module
  OS << *M;

  //Run on all instructions and print their  WIdependency
  OS << "\nWI related Values\n";
  for ( Module::const_iterator fi = M->begin(), fe = M->end(); fi != fe; ++fi ) {
    if (fi->isDeclaration()) continue;
    OS << fi->getName().str() << ":\n";
    for ( const auto &I : instructions(*fi)) {
      const Instruction* pInst = &I;
      //void type instructions has no value (i.e. no name) don't print them!
      if ( pInst->getType()->isVoidTy() ) continue;
      DenseMap<const Value*, WIDependancy>::const_iterator itDep = m_deps.find(pInst);
      if( itDep == m_deps.end()) {
        assert(false && "Expect all instructions to have WI dependency at this point");
        continue;
      }
      OS << pInst->getName().str() << " : ";
      switch (itDep->second) {
      case UNIFORM:
        OS << "UNI";
        break;
      case CONSECUTIVE:
        OS << "SEQ";
        break;
      case PTR_CONSECUTIVE:
        OS << "PTR";
        break;
      case STRIDED:
        OS << "SRT";
        break;
      case RANDOM:
        OS << "RND";
        break;
      default:
        assert(false && "Unknown WI dependency");
        OS << "unknown";
      }
      OS << "\n";
    }
  }
}

} // namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createWIAnalysisPass() {
    return new intel::WIAnalysis();
  }
}
