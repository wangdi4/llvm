#if INTEL_COLLAB
//===----------------- IntelVPlanDivergenceAnalysis.cpp -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements VPlan Divergence Analysis. Divergence analysis computes
// information that describes how values change with respect to each vector
// lane for a loop that may be vectorized using a specific VF. Values that are
// uniform are the same for each lane. Values that are or can be different for
// each lane are said to be divergent. Data divergence is discovered in a couple
// of ways:
//
// 1) Values can be predefined to be uniform/divergent, e.g., loop induction
//    variable for vector loop (divergent), definitions external to the VPlan
//    (uniform), etc.
//
// 2) Control flow divergence (e.g., conditional branching) causes data
//    divergence because this can result in different execution paths for each
//    vector lane, resulting in multiple definitions reaching a use.
//
// The VPlan Divergence Analysis algorithm identifies uniformity/divergence for
// each VPValue defined within the VPlan based upon both criteria.
//
// This work is based very closely on the DA work done at Saarland University
// for the Region Vectorizer, led by Simon Moll, and that is now being used
// within LLVM for GPGPU kernel divergence analysis. The attempt here was to
// port that algorithm to VPlan, with the eventual goal of templatizing it for
// use over LLVM and VPlan CFGs.
//
// See the following for reference:
//
//   https://github.com/cdl-saarland/vplan-rv
//   https://github.com/cdl-saarland/rv
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanDivergenceAnalysis.h"
#include "IntelVPlan.h"
#include "IntelVPlanLoopInfo.h"
#include "IntelVPlanSyncDependenceAnalysis.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vplan-divergence-analysis"

void VPlanDivergenceAnalysis::markDivergent(const VPValue &DivVal) {
  // Community version also checks to see if DivVal is a function argument.
  // For VPlan, function arguments are ExternalDefs, so check that here instead.
  assert(isa<VPInstruction>(DivVal) || isa<VPExternalDef>(DivVal));
  assert(!isAlwaysUniform(DivVal) && "cannot be a divergent");
  DivergentValues.insert(&DivVal);
}

void VPlanDivergenceAnalysis::addUniformOverride(const VPValue &UniVal) {
  UniformOverrides.insert(&UniVal);
}

// TODO: temporary until a getPhis() interface is added to VPlan and the
// Phis are added during HCFG construction.
static void getPhis(const VPBlockBase *Block,
                    SmallVectorImpl<const VPInstruction *> &Phis) {
  const VPBasicBlock *PhiBlock = cast<VPBasicBlock>(Block);
  for (const VPRecipeBase &Recipe : PhiBlock->getRecipes()) {
    if (const VPInstruction *VPInst = dyn_cast<VPInstruction>(&Recipe)) {
      unsigned OpCode = VPInst->getOpcode();
      if (OpCode == Instruction::PHI)
        Phis.push_back(VPInst);
    }
  }
}

#if !INTEL_CUSTOMIZATION
// This is used in the community version because br instructions are explicit.
// We will do the same for VPlan once supported.
bool DivergenceAnalysis::updateTerminator(const TerminatorInst &Term) const {

  if (Term.getNumSuccessors() <= 1)
    return false;

  if (auto *BranchTerm = dyn_cast<BranchInst>(&Term)) {
    assert(BranchTerm->isConditional());
    return isDivergent(*BranchTerm->getCondition());
  }

  if (auto *SwitchTerm = dyn_cast<SwitchInst>(&Term))
    return isDivergent(*SwitchTerm->getCondition());

  if (isa<InvokeInst>(Term))
    return false; // ignore abnormal executions through landingpad

  llvm_unreachable("unexpected terminator");
}
#endif

bool VPlanDivergenceAnalysis::updateNormalInstruction(
    const VPInstruction &I) const {
  // TODO function calls with side effects, etc
  for (const auto &Op : I.operands()) {
    if (isDivergent(*Op))
      return true;
  }
  return false;
}

bool VPlanDivergenceAnalysis::isTemporalDivergent(
    const VPBlockBase &ObservingBlock, const VPValue &Val) const {
  const auto *Inst = dyn_cast<const VPInstruction>(&Val);
  if (!Inst)
    return false;
  // check whether any divergent loop carrying @Val terminates before control
  // proceeds to @ObservingBlock
  const auto *VPLp = VPLI->getLoopFor(Inst->getParent());
  assert(VPLp && "Phi does not have parent loop?");
  for (; VPLp && VPLp != RegionLoop && !VPLp->contains(&ObservingBlock);
       VPLp = VPLp->getParentLoop()) {
    if (DivergentLoops.find(VPLp) != DivergentLoops.end())
      return true;
  }

  return false;
}

bool VPlanDivergenceAnalysis::updatePHINode(const VPInstruction &Phi) const {
  // joining divergent disjoint path in @Phi parent block
  // Currently, we don't have a way to determine whether or not a PHI
  // node always merges together the same value (i.e., call to
  // hasConstantOrUndefValue()).
  if (isJoinDivergent(*Phi.getParent()))
    return true;

  // An incoming value could be divergent by itself.
  // Otherwise, an incoming value could be uniform within the loop
  // that carries its definition but it may appear divergent
  // from outside the loop. This happens when divergent loop exits
  // drop definitions of that uniform value in different iterations.
  //
  // for (int i = 0; i < n; ++i) { // 'i' is uniform inside the loop
  //   if (i % thread_id == 0) break;    // divergent loop exit
  // }
  // int divI = i;                 // divI is divergent
  //
  // Source of code divergence with community is that Phi nodes in
  // VPlan do not have getNumIncomingValues(), getIncomingValue()
  // interfaces.
  for (const auto *Operand : Phi.operands()) {
    if (isDivergent(*Operand) ||
        isTemporalDivergent(*Phi.getParent(), *Operand))
      return true;
  }
  return false;
}

bool VPlanDivergenceAnalysis::inRegion(const VPInstruction &I) const {
  return I.getParent() && inRegion(*I.getParent());
}

bool VPlanDivergenceAnalysis::inRegion(const VPBlockBase &BB) const {
  return RegionLoop->contains(&BB);
}

// marks all users of loop-carried values of the loop headed by @LoopHeader as
// divergent
void VPlanDivergenceAnalysis::taintLoopLiveOuts(const VPBlockBase &LoopHeader) {
  auto *DivLoop = VPLI->getLoopFor(&LoopHeader);
  assert(DivLoop && "loopHeader is not actually part of a loop");

  SmallVector<VPBlockBase *, 8> TaintStack;
  DivLoop->getExitBlocks(TaintStack);

  // Otherwise potential users of loop-carried values could be anywhere in the
  // dominance region of @DivLoop (including its fringes for phi nodes)
  DenseSet<const VPBlockBase *> Visited;
  for (auto *Block : TaintStack)
    Visited.insert(Block);
  Visited.insert(&LoopHeader);

  while (!TaintStack.empty()) {
    auto *UserBlock = TaintStack.back();
    TaintStack.pop_back();

    // don't spread divergence beyond the region
    if (!inRegion(*UserBlock))
      continue;

    assert(!DivLoop->contains(UserBlock) &&
           "irreducible control flow detected");

    // phi nodes at the fringes of the dominance region
    if (!DT->dominates(&LoopHeader, UserBlock)) {
      // all PHI nodes of @userBlock become divergent
      // source of divergence with the community due to VPlan not having a
      // phis() interface for a block. This is the reason for the getPhis()
      // function.
      pushPHINodes(*UserBlock, true);
      continue;
    }

    // taint outside users of values carried by @DivLoop
    // community code divergence here is with how instructions are iterated over
    // due to recipes. Community code simply does 'for (auto &I : *UserBlock)'.
    for (auto &Recipe : *cast<VPBasicBlock>(UserBlock)) {
      VPInstruction &I = cast<VPInstruction>(Recipe);
      if (isAlwaysUniform(I))
        continue;
      if (isDivergent(I))
        continue;

      for (auto &Op : I.operands()) {
        auto *OpInst = dyn_cast<VPInstruction>(Op);
        if (!OpInst)
          continue;
        if (DivLoop->contains(cast<VPBlockBase>(OpInst->getParent()))) {
          markDivergent(I);
          pushUsers(I);
          break;
        }
      }
    }

    // visit all blocks in the dominance region
    // In VPlan, VPBasicBlocks are not inherited from BasicBlock, so the
    // successors() interface is not available.
    for (auto *SuccBlock : UserBlock->getSuccessors()) {
      if (!Visited.insert(SuccBlock).second)
        continue;
      TaintStack.push_back(SuccBlock);
    }
  }
}

void VPlanDivergenceAnalysis::pushPHINodes(const VPBlockBase &Block,
                                           bool PushAll) {

  SmallVector<const VPInstruction *, 2> PhiNodes;
  // Once again, no phis() interface in VPlan
  getPhis(&Block, PhiNodes);
  for (const auto *Phi : PhiNodes) {
    if (isDivergent(*Phi) && !PushAll)
      continue;
    Worklist.push_back(Phi);
  }
}

void VPlanDivergenceAnalysis::pushUsers(const VPValue &V) {
  for (const auto *User : V.users()) {
    const auto *UserInst = dyn_cast<const VPInstruction>(User);
    if (!UserInst)
      continue;

    if (isDivergent(*UserInst))
      continue;

    // only compute divergent inside loop
    if (!inRegion(*UserInst))
      continue;
    Worklist.push_back(UserInst);
  }
}

bool VPlanDivergenceAnalysis::propagateJoinDivergence(
    const VPBlockBase &JoinBlock, const VPLoop *BranchLoop) {
  LLVM_DEBUG(dbgs() << "\tpropJoinDiv " << JoinBlock.getName() << "\n");

  // ignore divergence outside the region
  if (!inRegion(JoinBlock))
    return false;

  // push non-divergent phi nodes in @JoinBlock to the worklist
  pushPHINodes(JoinBlock, false);

  // @joinBlock is a divergent loop exit
  if (BranchLoop && !BranchLoop->contains(&JoinBlock))
    return true;

  // disjoint-paths divergent at @joinBlock
  markBlockJoinDivergent(JoinBlock);
  return false;
}

// Main source of community code divergence here is that we don't represent
// branch instructions explicitly in VPlan yet. Thus, we have to use the
// condition of the branch to determine if the branch is divergent. Not a big
// deal, but we should be able to easily match the community code once VPlan
// is updated.
void VPlanDivergenceAnalysis::propagateBranchDivergence(const VPValue &Cond) {
  const VPInstruction *CondInst = cast<VPInstruction>(&Cond);
  LLVM_DEBUG(dbgs() << "propBranchDiv " << CondInst->getParent()->getName()
                    << "\n");

  markDivergent(Cond);

  const auto *BranchLoop = VPLI->getLoopFor(CondInst->getParent());

  // whether there is a divergent loop exit from @BranchLoop (if any)
  bool IsBranchLoopDivergent = false;

  // iterate over all blocks reachable by disjoint from @Cond within the loop
  // also iterates over loop exits that become divergent due to @Cond.
  for (const auto *JoinBlock :
       SDA->joinBlocks(*cast<VPBlockBase>(CondInst->getParent()))) {
    IsBranchLoopDivergent |= propagateJoinDivergence(*JoinBlock, BranchLoop);
  }

  // @BranchLoop is a divergent loop due to the divergent branch created by
  // @Cond.
  if (IsBranchLoopDivergent) {
    assert(BranchLoop && "parent loop not found for divergent branch");
    if (!DivergentLoops.insert(BranchLoop).second)
      return;
    propagateLoopDivergence(*BranchLoop);
  }
}

void VPlanDivergenceAnalysis::propagateLoopDivergence(
    const VPLoop &ExitingLoop) {
  // VPlan VPLoop inherits from LoopBase, which does not have a getName method.
  // We can easily implement one to match community version.
  LLVM_DEBUG(dbgs() << "propLoopDiv " << ExitingLoop << "\n");

  // don't propagate beyond region
  if (!inRegion(*ExitingLoop.getHeader()))
    return;

  const auto *BranchLoop = ExitingLoop.getParentLoop();

  // Uses of loop-carried values could occur anywhere
  // within the dominance region of the definition. All loop-carried
  // definitions are dominated by the loop header (reducible control).
  // Thus all users have to be in the dominance region of the loop header,
  // except PHI nodes that can also live at the fringes of the dom region
  // (incoming defining value).
  if (!IsLCSSAForm)
    taintLoopLiveOuts(*ExitingLoop.getHeader());

  // whether there is a divergent loop exit from @BranchLoop (if any)
  bool IsBranchLoopDivergent = false;

  // iterate over all blocks reachable by disjoint paths from exits of
  // @ExitingLoop also iterates over loop exits (of @BranchLoop) that in turn
  // become divergent.
  for (const auto *JoinBlock : SDA->joinBlocks(ExitingLoop))
    IsBranchLoopDivergent |= propagateJoinDivergence(*JoinBlock, BranchLoop);

  // @BranchLoop is divergent due to divergent loop exit in @ExitingLoop
  if (IsBranchLoopDivergent) {
    assert(BranchLoop && "parent loop not found for loop with divergent exit");
    if (!DivergentLoops.insert(BranchLoop).second)
      return;
    propagateLoopDivergence(*BranchLoop);
  }
}

void VPlanDivergenceAnalysis::computeImpl() {
  for (auto *DivVal : DivergentValues)
    pushUsers(*DivVal);

  // propagate divergence
  while (!Worklist.empty()) {
    const VPInstruction &I = *Worklist.back();
    Worklist.pop_back();

    // maintain uniformity of overrides
    if (isAlwaysUniform(I))
      continue;

    bool WasDivergent = isDivergent(I);
    if (WasDivergent)
      continue;

#if INTEL_CUSTOMIZATION
    // Branch instructions are not explicitly represented in VPlan, so check
    // to see if the current instruction is the same as the VPCondBit of the
    // parent block. If it is, then this tells us that we have a conditional
    // branch and BranchDependenceAnalysis can be used to tell us which phi
    // instructions are divergent. Note: the condition bit can be something
    // other than a cmp instruction.
    const VPValue *Cond = I.getParent()->getCondBit();
    if (&I == Cond) {
      if (updateNormalInstruction(I)) {
        // propagate control divergence to affected instructions
        propagateBranchDivergence(*Cond);
        continue;
      }
    }
#else
    // propagate divergence caused by terminator
    if (isa<TerminatorInst>(I)) {
      auto &Term = cast<TerminatorInst>(I);
      if (updateTerminator(Term)) {
        // propagate control divergence to affected instructions
        propagateBranchDivergence(Term);
        continue;
      }
    }
#endif // INTEL_CUSTOMIZATION

    // update divergence of I due to divergent operands
    bool DivergentUpd = false;
    // community code divergence is the check on opcode instead of doing
    // dyn_cast<PHINode>.
    unsigned OpCode = I.getOpcode();
    if (OpCode == Instruction::PHI)
      DivergentUpd = updatePHINode(I);
    else
      DivergentUpd = updateNormalInstruction(I);

    // propagate value divergence to users
    if (DivergentUpd) {
      markDivergent(I);
      pushUsers(I);
    }
  }
}

bool VPlanDivergenceAnalysis::isAlwaysUniform(const VPValue &V) const {
  return (UniformOverrides.find(&V) != UniformOverrides.end() ||
          isa<VPExternalDef>(V) || isa<VPMetadataAsValue>(V));
}

bool VPlanDivergenceAnalysis::isDivergent(const VPValue &V) const {
  return DivergentValues.find(&V) != DivergentValues.end();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// print function differs from the community version because VPlan is VPLoop
// based and not Module based (function DA).
void VPlanDivergenceAnalysis::print(raw_ostream &OS, const VPLoop *VPLp) const {

  LLVM_DEBUG(dbgs() << "\nPrinting Divergence info for " << *VPLp << "\n");
  ReversePostOrderTraversal<VPBlockBase *> RPOT(VPLp->getHeader());
  for (VPBlockBase *Block : make_range(RPOT.begin(), RPOT.end())) {
    if (auto VPBB = dyn_cast<VPBasicBlock>(Block)) {
      LLVM_DEBUG(dbgs() << "Basic Block: " << VPBB->getName() << "\n");
      for (auto &Recipe : VPBB->getRecipes()) {
        if (const VPInstruction *VPInst = dyn_cast<VPInstruction>(&Recipe)) {
          if (const VPValue *V = dyn_cast<VPValue>(VPInst)) {
            if (DivergentValues.find(V) != DivergentValues.end())
              LLVM_DEBUG(OS << "Divergent: "; V->dump(OS));
            else
              LLVM_DEBUG(OS << "Uniform: "; V->dump(OS));
          }
        }
      }
      LLVM_DEBUG(dbgs() << "\n");
    }
  }
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

void VPlanDivergenceAnalysis::compute(VPLoop *CandidateLoop,
                                      VPLoopInfo *VPLInfo,
                                      VPDominatorTree &VPDomTree,
                                      VPPostDominatorTree &VPPostDomTree,
                                      bool IsLCSSA) {

  RegionLoop = CandidateLoop;
  VPLI = VPLInfo;
  DT = &VPDomTree;
  IsLCSSAForm = IsLCSSA;
  SDA = new SyncDependenceAnalysis(CandidateLoop->getHeader(), VPDomTree,
                                   VPPostDomTree, *VPLInfo);

  // community code divergence due to no phis() interface
  SmallVector<const VPInstruction *, 2> PhiNodes;
  getPhis(CandidateLoop->getHeader(), PhiNodes);
  for (const auto *Phi : PhiNodes) {
    markDivergent(*Phi);
  }

  // after the scalar remainder loop is extracted, the loop exit condition will
  // be uniform
  // Source of code divergence here away from community - no getTerminator()
  VPBlockBase *ExitingBlock = CandidateLoop->getExitingBlock();
  if (ExitingBlock) {
    auto LoopExitCond = ExitingBlock->getCondBit();
    assert(LoopExitCond && "Loop exit condition not found");
    if (LoopExitCond)
      addUniformOverride(*LoopExitCond);
  }

  computeImpl();

  LLVM_DEBUG(print(dbgs(), CandidateLoop));
}

// Note: community version contains a LoopDivergencePrinter class that creates
// a SyncDependenceAnalysis object and a LoopDivergenceAnalysis object. The
// constructor for the LoopDivergenceAnalysis object then calls compute() to
// begin DA execution. For VPlan, all of this is done as part of HCFG
// construction. Thus, those classes are not included in this file.
#endif //INTEL_COLLAB
