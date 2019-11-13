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
#include "IntelVPLoopAnalysis.h"
#include "IntelVPlan.h"
#include "IntelVPlanLoopInfo.h"
#include "IntelVPlanSyncDependenceAnalysis.h"
#include "IntelVPlanUtils.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vplan-divergence-analysis"

static cl::opt<bool>
    VPlanDAIgnoreOverflow("vplan-da-ignore-integer-overflow", cl::init(false),
                          cl::Hidden,
                          cl::desc("Allow VPlan's divergence analysis to "
                                   "ignore integer overflow checks."));
extern cl::opt<bool> EnableVPValueCodegen;

#define Uni VPVectorShape::Uni
#define Seq VPVectorShape::Seq
#define Ptr VPVectorShape::Ptr
#define Str VPVectorShape::Str
#define Rnd VPVectorShape::Rnd
#define Undef VPVectorShape::Undef

const VPVectorShape::VPShapeDescriptor
AddConversion[VPVectorShape::NumDescs][VPVectorShape::NumDescs] = {
  /*           Uni,   Seq,   Ptr,   Str,   Rnd,   Undef */
  /* Uni   */ {Uni,   Seq,   Ptr,   Str,   Rnd,   Undef},
  /* Seq   */ {Seq,   Str,   Str,   Str,   Rnd,   Undef},
  /* Ptr   */ {Ptr,   Str,   Str,   Str,   Rnd,   Undef},
  /* Str   */ {Str,   Str,   Str,   Str,   Rnd,   Undef},
  /* Rnd   */ {Rnd,   Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Undef */ {Undef, Undef, Undef, Undef, Undef, Undef}
};

const VPVectorShape::VPShapeDescriptor
SubConversion[VPVectorShape::NumDescs][VPVectorShape::NumDescs] = {
  /*           Uni,   Seq,   Ptr,   Str,   Rnd,   Undef */
  /* Uni   */ {Uni,   Str,   Rnd,   Rnd,   Rnd,   Undef},
  /* Seq   */ {Seq,   Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Ptr   */ {Ptr,   Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Str   */ {Str,   Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Rnd   */ {Rnd,   Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Undef */ {Undef, Undef, Undef, Undef, Undef, Undef}
};

const VPVectorShape::VPShapeDescriptor
MulConversion[VPVectorShape::NumDescs][VPVectorShape::NumDescs] = {
  /*            Uni,   Seq,   Ptr,   Str,   Rnd,   Undef */
  /* Uni   */  {Uni,   Str,   Str,   Str,   Rnd,   Undef},
  /* Seq   */  {Str,   Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Ptr   */  {Str,   Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Str   */  {Str,   Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Rnd   */  {Rnd,   Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Undef */  {Undef, Undef, Undef, Undef, Undef, Undef}
};

const VPVectorShape::VPShapeDescriptor
GepConversion[VPVectorShape::NumDescs][VPVectorShape::NumDescs] = {
  /* ptr\index   Uni,   Seq,   Ptr,   Str,   Rnd,   Undef */
  /* Uni   */   {Uni,   Ptr,   Rnd,   Str,   Rnd,   Undef},
  /* Seq   */   {Str,   Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Ptr   */   {Str,   Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Str   */   {Rnd,   Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Rnd   */   {Rnd,   Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Undef */   {Undef, Undef, Undef, Undef, Undef, Undef}
};

const VPVectorShape::VPShapeDescriptor
SelectConversion[VPVectorShape::NumDescs][VPVectorShape::NumDescs] = {
  /*            Uni,   Seq,   Ptr,   Str,   Rnd,   Undef */
  /* Uni   */  {Uni,   Str,   Str,   Str,   Rnd,   Undef},
  /* Seq   */  {Str,   Seq,   Str,   Str,   Rnd,   Undef},
  /* Ptr   */  {Str,   Str,   Ptr,   Str,   Rnd,   Undef},
  /* Str   */  {Str,   Str,   Str,   Str,   Rnd,   Undef},
  /* Rnd   */  {Rnd,   Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Undef */  {Undef, Undef, Undef, Undef, Undef, Undef}
};

// Undefine the defines used in table initialization. Code below is
// expected to use values from VPVectorShape directly.
#undef Uni
#undef Seq
#undef Ptr
#undef Str
#undef Rnd
#undef Undef

void VPlanDivergenceAnalysis::markDivergent(const VPValue &DivVal) {
  // Community version also checks to see if DivVal is a function argument.
  // For VPlan, function arguments are ExternalDefs, so check that here instead.
  assert(!isAlwaysUniform(DivVal) && "cannot be a divergent");
  DivergentValues.insert(&DivVal);
}

// Mark DivVal as a value that is non-divergent.
void VPlanDivergenceAnalysis::markNonDivergent(const VPValue *DivVal) {
  assert(DivVal &&
         "Cannot have non-null Value for clearing divergence property.");
  DivergentValues.erase(DivVal);
}

void VPlanDivergenceAnalysis::addUniformOverride(const VPValue &UniVal) {
  UniformOverrides.insert(&UniVal);
}

#if INTEL_CUSTOMIZATION
// TODO: temporary until a getPhis() interface is added to VPlan and the
// Phis are added during HCFG construction.
static void getPhis(const VPBlockBase *Block,
                    SmallVectorImpl<const VPInstruction *> &Phis) {
  const VPBasicBlock *PhiBlock = cast<VPBasicBlock>(Block);
  for (const VPInstruction &VPInst : PhiBlock->vpinstructions()) {
    unsigned OpCode = VPInst.getOpcode();
    if (OpCode == Instruction::PHI)
      Phis.push_back(&VPInst);
  }
}

static bool hasDeterministicResult(const VPInstruction &I) {
  // As of now only a call instruction known to possibly have non-deterministoic
  // result.
  if (I.getOpcode() != Instruction::Call)
    return true;

  if (I.getType()->isVoidTy())
    return true;

  // Check whether a call instruction may have a side effect as indicator of
  // possible non-determinism. It is actually a bit pessimistic approach since
  // non-determinism is only a subset of possible side effects.
  return !I.mayHaveSideEffects();
}
#endif // INTEL_CUSTOMIZATION

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

  if (!hasDeterministicResult(I))
    return true;
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
    for (auto &I : cast<VPBasicBlock>(UserBlock)->vpinstructions()) {
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
                                           bool PushAll) { // INTEL

#if INTEL_CUSTOMIZATION
  SmallVector<const VPInstruction *, 2> PhiNodes;
  // Once again, no phis() interface in VPlan
  getPhis(&Block, PhiNodes);
  for (const auto *Phi : PhiNodes) {
    if (isDivergent(*Phi) && !PushAll)
#endif
      continue;
    Worklist.push_back(Phi);
  }
}

void VPlanDivergenceAnalysis::pushUsers(const VPValue &V,
                                        bool PushAll) { // INTEL
  for (const auto *User : V.users()) {
    const auto *UserInst = dyn_cast<const VPInstruction>(User);
    if (!UserInst)
      continue;

    if (isDivergent(*UserInst) && !PushAll) // INTEL
      continue;

    // only compute divergent/shapes inside loop
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

#if INTEL_CUSTOMIZATION
// Main source of community code divergence here is that we don't represent
// branch instructions explicitly in VPlan yet. Thus, we have to use the
// condition of the branch to determine if the branch is divergent. Not a big
// deal, but we should be able to easily match the community code once VPlan
// is updated.
#endif
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

#if INTEL_CUSTOMIZATION
bool VPlanDivergenceAnalysis::pushMissingOperands(const VPInstruction &I) {
  bool MissingOp = false;
  for (const auto &Op : I.operands()) {
    if (getVectorShape(Op)->isUndefined()) {
      auto *OpInst = dyn_cast<VPInstruction>(Op);
      if (OpInst) {
        Worklist.push_back(OpInst);
        MissingOp = true;
      }
    }
  }
  return MissingOp;
}
#endif

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
        VPVectorShape *NewShape = getRandomVectorShape();
        updateVectorShape(&I, NewShape);
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

#if INTEL_CUSTOMIZATION
    VPVectorShape *NewShape = nullptr;

    bool IsPhiNode = (I.getOpcode() == Instruction::PHI);
    bool ShapeUpdated = false;
    if (IsPhiNode) {
      pushMissingOperands(I);
      NewShape = computeVectorShape(&I);
      ShapeUpdated = updateVectorShape(&I, NewShape);
    } else if (pushMissingOperands(I)) {
      continue;
    } else {
      NewShape = computeVectorShape(&I);
      ShapeUpdated = updateVectorShape(&I, NewShape);
    }

    if (ShapeUpdated)
      pushUsers(I, true);
#endif // INTEL_CUSTOMIZATION

    // update divergence of I due to divergent operands
    bool DivergentUpd = false;
    // community code divergence is the check on opcode instead of doing
    // dyn_cast<PHINode>.
    if (IsPhiNode)
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

#if INTEL_CUSTOMIZATION
bool VPlanDivergenceAnalysis::isUniformLoopEntity(const VPValue *V) const {
  if (isa<VPExternalDef>(V) &&
      !(RegionLoopEntities->getReduction(V) ||
        RegionLoopEntities->getInduction(V) ||
        RegionLoopEntities->getPrivate(V)))
    return true;
  return false;
}
#endif

bool VPlanDivergenceAnalysis::isAlwaysUniform(const VPValue &V) const {
  if (DivergentLoopEntities.count(&V))
    return false;

  if (UniformOverrides.find(&V) != UniformOverrides.end() ||
      isa<VPMetadataAsValue>(V) || isa<VPConstant>(V) || isa<VPExternalDef>(V))
    return true;

  // TODO: We have a choice on how to handle functions such as get_global_id().
  // Currently, OCL VecClone is already treating such calls as linear and
  // hoisting them outside of the inserted loop. As such, it adds the
  // appropriate stride to any users. Thus, here we treat these calls as uniform
  // because the added instructions by OCL VecClone to calculate stride will
  // cause DA to propagate the correct stride as is. The call to
  // isUniformLoopEntity has been commented out because of this since support
  // has now been added to make these calls VPInduction objects. If this line is
  // uncommented, the incorrect stride will be propagated. Later, we can always
  // allow DA to use this call instead of VecClone treating the calls as linear.
  // Either way is correct.
  //
  // if (isUniformLoopEntity(&V))
  //   return true;

  auto *VPInst = dyn_cast<VPInstruction>(&V);

  if (!VPInst || VPInst->getOpcode() != Instruction::Call)
    return false;

  auto *CalledFunc =
      dyn_cast<VPConstant>(VPInst->getOperand(VPInst->getNumOperands() - 1));

  if (!CalledFunc)
    return false;

  auto *Func = dyn_cast<Function>(CalledFunc->VPValue::getUnderlyingValue());
  if (!Func)
    return false;

  if (Func->hasFnAttribute("opencl-vec-uniform-return"))
    return true;

  return false;
}

bool VPlanDivergenceAnalysis::isDivergent(const VPValue &V) const {
  return DivergentValues.find(&V) != DivergentValues.end();
}

#if INTEL_CUSTOMIZATION
VPVectorShape *VPlanDivergenceAnalysis::getVectorShape(const VPValue *V) const {
  auto ShapeIter = VectorShapes.find(V);
  if (ShapeIter != VectorShapes.end())
    return ShapeIter->second.get();
  return UndefShape.get();
}

bool VPlanDivergenceAnalysis::shapesAreDifferent(VPVectorShape *OldShape,
                                                 VPVectorShape *NewShape) {
  // For the first-time this function is called in context of private-entities,
  // the OldShape is a nullptr.
  if ((!OldShape && NewShape) || (OldShape && !NewShape) ||
      (OldShape && NewShape &&
       (OldShape->getShapeDescriptor() != NewShape->getShapeDescriptor() ||
        (OldShape->hasKnownStride() && NewShape->hasKnownStride() &&
         OldShape->getStrideVal() != NewShape->getStrideVal())))) {
    return true;
  }
  return false;
}

bool VPlanDivergenceAnalysis::updateVectorShape(const VPValue *V,
                                                VPVectorShape *Shape) {
  VPVectorShape *OldShape = getVectorShape(V);

  // Has shape changed in any way?
  if (shapesAreDifferent(OldShape, Shape)) {
    std::unique_ptr<VPVectorShape> &ShapePtr = VectorShapes[V];
    if (ShapePtr)
      VectorShapes[V].reset(Shape);
    else {
      ShapePtr.reset(Shape);
      VectorShapes[V] = std::move(ShapePtr);
    }
    return true;
  }
  return false;
}

VPVectorShape* VPlanDivergenceAnalysis::getUniformVectorShape() {
  return new VPVectorShape(VPVectorShape::Uni, getConstantInt(0));
}

VPVectorShape* VPlanDivergenceAnalysis::getRandomVectorShape() {
  return new VPVectorShape(VPVectorShape::Rnd);
}

VPVectorShape *
VPlanDivergenceAnalysis::getSequentialVectorShape(uint64_t Stride) {
  return new VPVectorShape(VPVectorShape::Seq, getConstantInt(Stride));
}

VPVectorShape *VPlanDivergenceAnalysis::getStridedVectorShape(uint64_t Stride) {
  return new VPVectorShape(VPVectorShape::Str, getConstantInt(Stride));
}

void VPlanDivergenceAnalysis::setVectorShapesForUniforms(const VPLoop *VPLp) {
  ReversePostOrderTraversal<VPBlockBase *> RPOT(VPLp->getHeader());
  for (VPBlockBase *Block : make_range(RPOT.begin(), RPOT.end())) {
    if (auto VPBB = dyn_cast<VPBasicBlock>(Block)) {
      for (auto &VPInst : VPBB->vpinstructions()) {
        if (!isDivergent(VPInst) && getVectorShape(&VPInst)->isUndefined()) {
          VPVectorShape *NewShape = getUniformVectorShape();
          updateVectorShape(&VPInst, NewShape);
        }
      }
    }
  }
}

void VPlanDivergenceAnalysis::verifyVectorShapes(const VPLoop *VPLp) {
  ReversePostOrderTraversal<VPBlockBase *> RPOT(VPLp->getHeader());
  for (VPBlockBase *Block : make_range(RPOT.begin(), RPOT.end())) {
    if (auto VPBB = dyn_cast<VPBasicBlock>(Block)) {
      for (auto &VPInst : VPBB->vpinstructions()) {
        VPVectorShape *Shape = getVectorShape(&VPInst);
        assert(!Shape->isUndefined() && "Shape has not been defined");
        if (!isDivergent(VPInst) && !Shape->isUniform())
          llvm_unreachable("Uniform inst shape not defined as uniform");
        if (isDivergent(VPInst) && Shape->isUniform())
          llvm_unreachable("Divergent inst shape defined as uniform");
      }
    }
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#if INTEL_CUSTOMIZATION
// print function differs from the community version because VPlan is VPLoop
// based and not Module based (function DA).
void VPlanDivergenceAnalysis::print(raw_ostream &OS, const VPLoop *VPLp) {

  LLVM_DEBUG(dbgs() << "\nPrinting Divergence info for " << *VPLp << "\n");
  ReversePostOrderTraversal<VPBlockBase *> RPOT(VPLp->getHeader());
  for (VPBlockBase *Block : make_range(RPOT.begin(), RPOT.end())) {
    if (auto VPBB = dyn_cast<VPBasicBlock>(Block)) {
      LLVM_DEBUG(dbgs() << "Basic Block: " << VPBB->getName() << "\n");
      for (auto &VPInst : VPBB->vpinstructions()) {
        if (isDivergent(VPInst))
          LLVM_DEBUG(OS << "Divergent: ");
        else
          LLVM_DEBUG(OS << "Uniform: ");
        LLVM_DEBUG(getVectorShape(&VPInst)->print(OS); OS << ' ');
        LLVM_DEBUG(VPInst.dump(OS));
      }
      LLVM_DEBUG(dbgs() << "\n");
    }
  }
}
#endif // INTEL_CUSTOMIZATION
#endif // !NDEBUG || LLVM_ENABLE_DUMP

VPConstant* VPlanDivergenceAnalysis::getConstantInt(int64_t Val) {
  LLVMContext &C = *Plan->getLLVMContext();
  ConstantInt *CInt = ConstantInt::get(Type::getInt64Ty(C), Val);
  VPConstant *VPCInt = new VPConstant(CInt);
  return VPCInt;
}

bool VPlanDivergenceAnalysis::getConstantIntVal(VPValue *V, uint64_t &IntVal) {
  if (V && isa<VPConstant>(V)) {
    Constant *C = cast<Constant>(V->getUnderlyingValue());
    if (ConstantInt *CInt = dyn_cast<ConstantInt>(C)) {
      IntVal = CInt->getSExtValue();
      return true;
    }
  }
  return false;
}

VPVectorShape* VPlanDivergenceAnalysis::computeVectorShapeForBinaryInst(
    const VPInstruction *I) {

  VPValue *Op0 = I->getOperand(0);
  VPValue *Op1 = I->getOperand(1);

  LLVMContext &C = *Plan->getLLVMContext();

  if (getVectorShape(Op0)->isUniform() && getVectorShape(Op1)->isUniform())
    return getUniformVectorShape();

  // Put VPConstants on the right-hand side of the expression for commutative
  // instructions. This can later be used to calculate a new known stride for I
  // given that the left-hand side has a known stride.
  if (!isa<VPConstant>(Op1) && Instruction::isCommutative(I->getOpcode()))
    std::swap(Op0, Op1);

  VPVectorShape *Shape0 = getVectorShape(Op0);
  VPVectorShape *Shape1 = getVectorShape(Op1);
  VPVectorShape::VPShapeDescriptor Desc0 = Shape0->getShapeDescriptor();
  VPVectorShape::VPShapeDescriptor Desc1 = Shape1->getShapeDescriptor();

  switch (I->getOpcode()) {
    case Instruction::Mul:
    case Instruction::FMul: {
      // A constant integer multiplied by a known stride results in another
      // known stride that has been scaled.
      VPValue *NewStride = nullptr;
      uint64_t Op1IntVal;
      uint64_t Op0StrideIntVal;
      bool Op1IsInt = getConstantIntVal(Op1, Op1IntVal);
      bool Shape0StrideIsInt = getConstantIntVal(Shape0->getStride(),
                                                 Op0StrideIntVal);
      if (Op1IsInt && Shape0StrideIsInt) {
        uint64_t NewStrideVal = Op1IntVal * Op0StrideIntVal;
        ConstantInt *NewStrideInt = ConstantInt::get(Type::getInt64Ty(C),
                                                     NewStrideVal);
        NewStride = new VPConstant(NewStrideInt);
      }

      VPVectorShape::VPShapeDescriptor NewDesc;
      // Not sure if we can assume mul by 0/1 is optimized since we can
      // vectorize at O0/O1.
      if (Op1IsInt && Op1IntVal == 0)
        NewDesc = VPVectorShape::Uni;
      else if (Op1IsInt && Op1IntVal == 1)
        NewDesc = Shape0->getShapeDescriptor();
      else
        NewDesc = MulConversion[Desc0][Desc1];
      return new VPVectorShape(NewDesc, NewStride);
    }
    case Instruction::Add:
    case Instruction::FAdd: {
      VPValue *NewStride = nullptr;
      uint64_t Op0StrideIntVal;
      uint64_t Op1StrideIntVal;
      bool Op0StrideIsInt = getConstantIntVal(Shape0->getStride(),
                                              Op0StrideIntVal);
      bool Op1StrideIsInt = getConstantIntVal(Shape1->getStride(),
                                              Op1StrideIntVal);
      if (Op0StrideIsInt && Op1StrideIsInt) {
        uint64_t NewStrideVal = Op0StrideIntVal + Op1StrideIntVal;
        ConstantInt *NewStrideInt = ConstantInt::get(Type::getInt64Ty(C),
                                                     NewStrideVal);
        NewStride = new VPConstant(NewStrideInt);
      }
      VPVectorShape::VPShapeDescriptor NewDesc = AddConversion[Desc0][Desc1];
      return new VPVectorShape(NewDesc, NewStride);
    }
    case Instruction::Sub:
    case Instruction::FSub: {
      VPValue *NewStride = nullptr;
      uint64_t Op0StrideIntVal;
      uint64_t Op1StrideIntVal;
      bool Op0StrideIsInt = getConstantIntVal(Shape0->getStride(),
                                              Op0StrideIntVal);
      bool Op1StrideIsInt = getConstantIntVal(Shape1->getStride(),
                                              Op1StrideIntVal);
      if (Op0StrideIsInt && Op1StrideIsInt) {
        uint64_t NewStrideVal = Op0StrideIntVal - Op1StrideIntVal;
        ConstantInt *NewStrideInt = ConstantInt::get(Type::getInt64Ty(C),
                                                     NewStrideVal);
        NewStride = new VPConstant(NewStrideInt);
      }
      VPVectorShape::VPShapeDescriptor NewDesc = SubConversion[Desc0][Desc1];
      return new VPVectorShape(NewDesc, NewStride);
    }
    case Instruction::And: {
      if (VPlanDAIgnoreOverflow) {
        // AND operation with UINT_MAX indicates an integer overflow check and
        // clamping. Propagate the shape of the operand being checked for
        // overflow.
        if (auto *ConstOp1 = dyn_cast<VPConstant>(Op1)) {
          if (ConstOp1->isConstantInt() && ConstOp1->getZExtValue() == UINT_MAX)
            return new VPVectorShape(Desc0, Shape0->getStride());
        }
      }
      LLVM_FALLTHROUGH;
    }
    default:
      return getRandomVectorShape();
  }
}

VPVectorShape* VPlanDivergenceAnalysis::computeVectorShapeForCastInst(
    const VPInstruction *I) {

  VPValue *Op0 = I->getOperand(0);
  VPVectorShape *Shape0 = getVectorShape(Op0);
  if (Shape0->isUniform())
    return getUniformVectorShape();

  switch (I->getOpcode()) {
    case Instruction::SExt:
    case Instruction::PtrToInt:
    case Instruction::IntToPtr:
    case Instruction::SIToFP:
    case Instruction::UIToFP:
    case Instruction::FPToUI:
    case Instruction::FPToSI:
    case Instruction::AddrSpaceCast:
      return new VPVectorShape(Shape0->getShapeDescriptor(),
                               Shape0->getStride());
    case Instruction::BitCast: {
      PointerType *SrcPtrTy =
          dyn_cast<PointerType>(I->getOperand(0)->getType());
      if (SrcPtrTy || (I->getOperand(0)->getType() == I->getType())) {
        // Case 1: %Y = bitcast i32* %x to sint*          ; yields sint*:%x
        // Case 2: %Y = bitcast i32* %x to <3 x i32>*     ; yields <3 x i32>*:%x
        // Case 3: %Z = bitcast i32 %x to i32             ; yields i32: %x
        // Case 3, is commonly seen when doing codegen along HIR-path, where
        // as part of decomposition, temporary copy-assigments are generated.
        return new VPVectorShape(Shape0->getShapeDescriptor(),
                                 Shape0->getStride());
      }
      // For the following cases,
      // Case 1: %Z = bitcast <2 x int> %V to i64;        ; yields i64: %V
      // Case 2: %Z = bitcast <2 x i32*> %V to <2 x i64*> ; yields <2 x i64*>
      // Case 3: %X = bitcast i8 255 to i8                ; yields i8 :-1
      // Case 4: %BC = bitcast i64 %V to <2 x i32>        ; yields <2 x i32> :-1
      // there is a 'value'-cast. The returned shape has to be random.
      return getRandomVectorShape();
    }
    default:
      return getRandomVectorShape();
  }
}

VPVectorShape* VPlanDivergenceAnalysis::computeVectorShapeForGepInst(
    const VPInstruction *I) {

  const VPValue* PtrOp = I->getOperand(0);
  VPVectorShape *PtrShape = getVectorShape(PtrOp);
  unsigned NumOperands = I->getNumOperands();

  // If any of the gep indices, except the last, are not uniform, then return
  // random shape.
  for (unsigned i = 1; i < NumOperands - 1; i++) {
    const VPValue *Op = I->getOperand(i);
    VPVectorShape *OpShape = getVectorShape(Op);
    if (!OpShape->isUniform())
      return getRandomVectorShape();
  }

  const VPValue* LastIdx = I->getOperand(NumOperands - 1);
  VPVectorShape *IdxShape = getVectorShape(LastIdx);

  VPConstant *NewStride = nullptr;

  VPVectorShape::VPShapeDescriptor PtrShapeDesc =
      PtrShape->getShapeDescriptor();

  VPVectorShape::VPShapeDescriptor IdxShapeDesc =
      IdxShape->getShapeDescriptor();

  VPVectorShape::VPShapeDescriptor NewDesc =
      GepConversion[PtrShapeDesc][IdxShapeDesc];

  // If shape is not random, then a new stride (in bytes) can be calculated for
  // the gep. Gep stride is always in bytes.
  if (NewDesc != VPVectorShape::Rnd) {
    // BaseType of Gep should be a pointer type referring to a non-aggregate
    // type (i.e., scalar or vector type). For example, this should hold true
    // for multi-dim arrays.
    // Examples: float* -> float,
    //           [3000 x [3000 x i32]]* -> i32,
    //           <4 x i32>* -> <4 x i32>
    Type *PointedToTy = cast<PointerType>(I->getType())->getElementType();
    uint64_t PointedToTySize =
        Plan->getDataLayout()->getTypeSizeInBits(PointedToTy) >> 3;
    // For known strides:
    // 1) Uniform gep should result in 0 stride (i.e., pointer and idx are
    //    uniform).
    // 2) When strides are known for both pointer and idx, new stride should
    //    also be known.
    // Otherwise, if either stride is unknown, this should result in an unknown
    // stride (nullptr).
    if (PtrShape->hasKnownStride() && IdxShape->hasKnownStride()) {
      VPValue *PtrStride = PtrShape->getStride();
      VPValue *IdxStride = IdxShape->getStride();
      // It's possible that the ptr value is represented as an integer that has
      // a non-ptr stride. i.e., shape is unit stride or strided and not
      // represented in bytes. And, the gep index is uniform. To get the stride
      // in bytes for these cases, we can simply swap the shapes of the ptr and
      // index.
      if (IdxShape->isUniform() && PtrShape->isAnyStridedNonPtr())
        std::swap(PtrStride, IdxStride);
      uint64_t PtrStrideVal =
          cast<ConstantInt>(PtrStride->getUnderlyingValue())->getSExtValue();
      uint64_t IdxStrideVal =
          cast<ConstantInt>(IdxStride->getUnderlyingValue())->getSExtValue();
      NewStride = getConstantInt(PtrStrideVal + PointedToTySize * IdxStrideVal);

      // See if we can refine a strided pointer to a unit-strided pointer by
      // checking if new stride value is the same as size of pointedto type.
      const APInt &NewStrideVal =
          cast<ConstantInt>(NewStride->getUnderlyingValue())->getValue();
      uint64_t NewStrideValAbs = NewStrideVal.abs().getZExtValue();
      if (NewDesc == VPVectorShape::Str && PointedToTySize == NewStrideValAbs)
        NewDesc = VPVectorShape::Ptr;
    }
  }
  return new VPVectorShape(NewDesc, NewStride);
}

VPVectorShape* VPlanDivergenceAnalysis::computeVectorShapeForPhiNode(
    const VPPHINode *Phi) {

  // If any incoming value shape is temporalily divergent (uniform in loop,
  // divergent out-side loop), then the phi shape is random.
  for (unsigned i = 0; i < Phi->getNumIncomingValues(); i++) {
    VPValue *IncomingVal = Phi->getIncomingValue(i);
    if (isTemporalDivergent(*Phi->getParent(), *IncomingVal))
      return getRandomVectorShape();
  }

  // Incoming value shapes could be uniform, but the parent of the phi node may
  // be reached through a divergent branch. If so, the phi is divergent and
  // return random shape.
  if (isJoinDivergent(*Phi->getParent()))
    return getRandomVectorShape();

  // Compute shape for phi node.
  SmallVector<VPVectorShape*, 2> Shapes;
  VPVectorShape *NewShape = new VPVectorShape(VPVectorShape::Undef);
  Shapes.push_back(NewShape);
  for (unsigned i = 0; i < Phi->getNumIncomingValues(); i++) {
    VPValue *IncomingVal = Phi->getIncomingValue(i);
    VPVectorShape *IncomingShape = getVectorShape(IncomingVal);
    NewShape = VPVectorShape::joinShapes(NewShape, IncomingShape);
    Shapes.push_back(NewShape);
  }

  // Delete all shapes except the last one. These were all just temporary
  // leading up to the final shape computation.
  for (unsigned i = 0; i < Shapes.size() - 1; i++)
    delete Shapes[i];

  // Prevent undefined phi nodes from causing an infinite loop. If divergence is
  // ever propagated to the phi, then isJoinDivergent() becomes true and a new
  // shape will be computed. The infinite loop is possible for phis that stay
  // uniform because they can have cyclic dependencies on other uniform
  // VPInstructions. Thus, pushMissingOperands() will continuously push the
  // undefined operands of the phi, which will in turn push the phi node again,
  // causing the infinite loop.
  if (NewShape->isUndefined() && !isJoinDivergent(*Phi->getParent()))
    NewShape = getUniformVectorShape();

  return NewShape;
}

VPVectorShape* VPlanDivergenceAnalysis::computeVectorShapeForLoadInst(
    const VPInstruction *I) {

  VPValue *PtrOp = I->getOperand(0);
  VPVectorShape *PtrShape = getVectorShape(PtrOp);
  // Uniform DummyShape is used here instead of nullptr to avoid joinShapes from
  // returning the same shape as PtrShape. This would occur if PtrShape is
  // strided, but strided Ptr will most likely not return strided values. Thus,
  // this will force joinShapes to return either uniform or random.
  VPVectorShape *DummyShape = getUniformVectorShape();
  VPVectorShape *NewShape = VPVectorShape::joinShapes(DummyShape, PtrShape);
  delete DummyShape;
  return NewShape;
}

VPVectorShape* VPlanDivergenceAnalysis::computeVectorShapeForStoreInst(
    const VPInstruction *I) {

  VPValue *ValOp = I->getOperand(0);
  VPValue *PtrOp = I->getOperand(1);
  VPVectorShape *ValShape = getVectorShape(ValOp);
  VPVectorShape *PtrShape = getVectorShape(PtrOp);
  return VPVectorShape::joinShapes(ValShape, PtrShape);
}

VPVectorShape* VPlanDivergenceAnalysis::computeVectorShapeForCmpInst(
    const VPCmpInst *I) {

  // For now, shape optimizations for compare instructions are kept basic. For
  // integer comparisons, we can introduce other uniformity optimizations based
  // on KnownBits, but those have been excluded for now.
  VPValue *Op0 = I->getOperand(0);
  VPValue *Op1 = I->getOperand(1);
  VPVectorShape *Shape0 = getVectorShape(Op0);
  VPVectorShape *Shape1 = getVectorShape(Op1);

  if (Shape0->isUniform() && Shape1->isUniform())
    return getUniformVectorShape();

  return getRandomVectorShape();
}

VPVectorShape* VPlanDivergenceAnalysis::computeVectorShapeForInsertExtractInst(
    const VPInstruction *I) {

  VPValue *VectorOp = I->getOperand(0);
  VPValue *IdxOp = I->getOperand(1);
  VPVectorShape *VectorOpShape = getVectorShape(VectorOp);
  VPVectorShape *IdxOpShape = getVectorShape(IdxOp);
  return VPVectorShape::joinShapes(VectorOpShape, IdxOpShape);
}

VPVectorShape* VPlanDivergenceAnalysis::computeVectorShapeForSelectInst(
    const VPInstruction *I) {

  VPValue *Mask = I->getOperand(0);
  VPVectorShape *MaskShape = getVectorShape(Mask);
  if (MaskShape->isUniform()) {
    VPValue *Op1 = I->getOperand(1);
    VPValue *Op2 = I->getOperand(2);
    VPVectorShape *Shape1 = getVectorShape(Op1);
    VPVectorShape *Shape2 = getVectorShape(Op2);
    VPVectorShape::VPShapeDescriptor Shape1Desc = Shape1->getShapeDescriptor();
    VPVectorShape::VPShapeDescriptor Shape2Desc = Shape2->getShapeDescriptor();
    uint64_t MaskConstIntVal;
    bool MaskIsConstInt = getConstantIntVal(Mask, MaskConstIntVal);
    if (isa<VPConstant>(Mask) && MaskIsConstInt) {
      if (MaskConstIntVal)
        // mask = 1, shape is inherited from Op1
        return new VPVectorShape(Shape1Desc, Shape1->getStride());
      else
        // mask == 0, shape is inherited from Op2
        return new VPVectorShape(Shape2Desc, Shape2->getStride());
    }

    VPVectorShape::VPShapeDescriptor NewDesc =
        SelectConversion[Shape1Desc][Shape2Desc];

    VPValue *NewStride = nullptr;
    if (NewDesc == VPVectorShape::Uni)
      NewStride = getConstantInt(0);
    else if (VPVectorShape::isAnyStrided(NewDesc)) {
      // For selects generating strided VectorShape the stride information can
      // be propagted from the operands being blended, only if they have the
      // same stride.
      if (VPVectorShape::shapesHaveSameStride(Shape1, Shape2))
        NewStride = Shape1->getStride();
    }

    return new VPVectorShape(NewDesc, NewStride);
  }

  // Non-uniform mask results in selection of values from Op1 and Op2.
  return getRandomVectorShape();
}

VPVectorShape* VPlanDivergenceAnalysis::computeVectorShapeForCallInst(
    const VPInstruction *I) {

  // Will need to add support for functions with known shapes; e.g., something
  // similiar to get_global_id. However, get_global_id takes a single integer
  // constant argument. Because of that, divergence propagation will not see
  // this instruction because divergence properties will not be passed to calls
  // with all arguments known to always be uniform. Thus, shape propgatation for
  // these types of functions will be handled in intializeKnownShapes. But,
  // there could be other instructions that behave the same way that can be
  // handled here.

  if (!hasDeterministicResult(*I))
    return getRandomVectorShape();

  bool AllOpsUniform = true;
  unsigned NumOps = I->getNumOperands() - 1;
  for (unsigned i = 0; i < NumOps; i++) {
    VPValue* Op = I->getOperand(i);
    if (!getVectorShape(Op)->isUniform()) {
      AllOpsUniform = false;
      break;
    }
  }

  if (AllOpsUniform)
    return getUniformVectorShape();

  return getRandomVectorShape();
}

VPVectorShape* VPlanDivergenceAnalysis::computeVectorShape(
    const VPInstruction *I) {

  // Note: It is assumed at this point that shapes for all operands of I have
  // been defined, with the possible exception of phi nodes. Shape computation
  // for phi nodes can still happen without all operands being defined. See
  // computeImpl() and computeVectorShapeForPhiNode().

  if (isa<VPPHINode>(I))
    return computeVectorShapeForPhiNode(cast<VPPHINode>(I));

  VPVectorShape *NewShape = nullptr;
  unsigned Opcode = I->getOpcode();

  if (Instruction::isBinaryOp(Opcode))
    NewShape = computeVectorShapeForBinaryInst(I);
  else if (Instruction::isCast(Opcode))
    NewShape = computeVectorShapeForCastInst(I);
  else if (Opcode == Instruction::GetElementPtr)
    NewShape = computeVectorShapeForGepInst(I);
  else if (Opcode == Instruction::Load)
    NewShape = computeVectorShapeForLoadInst(I);
  else if (Opcode == Instruction::Store)
    NewShape = computeVectorShapeForStoreInst(I);
  else if (Opcode == Instruction::ICmp || Opcode == Instruction::FCmp)
    NewShape = computeVectorShapeForCmpInst(cast<VPCmpInst>(I));
  else if (Opcode == Instruction::InsertElement ||
           Opcode == Instruction::ExtractElement)
    NewShape = computeVectorShapeForInsertExtractInst(I);
  else if (Opcode == Instruction::Select)
    NewShape = computeVectorShapeForSelectInst(I);
  else if (Opcode == Instruction::Call)
    NewShape = computeVectorShapeForCallInst(I);
  else {
    LLVM_DEBUG(dbgs() << "Instruction not supported: " << *I);
    NewShape = getRandomVectorShape();
    //llvm_unreachable("Instruction not supported\n");
  }

  return NewShape;
}

void VPlanDivergenceAnalysis::initializeShapes(
    SmallVectorImpl<const VPInstruction*> &PhiNodes) {

  // Initialize all outer loop phi induction nodes using the appropriate
  // strides. We could also initialize VPExternalDefs to uniform here, but
  // we can do that only the fly in computeVectorShapes() to avoid an
  // additional iteration over the instructions in the VPlan.
  for (const auto *Phi : PhiNodes) {
    VPVectorShape *NewShape = nullptr;
    if (const VPInduction *Ind = RegionLoopEntities->getInduction(Phi)) {
      const VPValue *Step = Ind->getStep();
      // Default StepInt is 0 to account for variable step IV cases.
      int StepInt = 0;
      if (auto *StepConst = dyn_cast<VPConstant>(Step))
        if (StepConst->isConstantInt())
          StepInt = StepConst->getZExtValue();

      // IV's vector shape is determined based on its step value. For variable
      // step IVs, we choose Strided (unknown stride value).
      VPVectorShape::VPShapeDescriptor IVShape = (StepInt == 1 || StepInt == -1)
                                                     ? VPVectorShape::Seq
                                                     : VPVectorShape::Str;

      NewShape = new VPVectorShape(IVShape, const_cast<VPValue *>(Step));
    } else
      // To be conservative, we mark phi nodes with random shape unless we
      // know the phi is an induction. This matches the divergent property
      // set for outer loop phi nodes just before this step. See beginning
      // of compute().
      NewShape = getRandomVectorShape();

    updateVectorShape(Phi, NewShape);
  }

  ReversePostOrderTraversal<VPBlockBase *> RPOT(RegionLoop->getHeader());
  for (VPBlockBase *Block : make_range(RPOT.begin(), RPOT.end())) {
    if (auto VPBB = dyn_cast<VPBasicBlock>(Block)) {
      for (auto &VPInst : VPBB->vpinstructions()) {
        // If any operands of the instruction are always known to be
        // uniform, set the shape as uniform. This will also cover cases
        // where some instructions have all uniform operands that are
        // external definitions and for which shapes will not be defined
        // during divergence propagation. I.e., shape propagation happens
        // through def/use chains and these instructions are not using
        // other values defined within the region. Thus, these instructions
        // will never be "seen" while propagating divergence and shape
        // information and would result in shapes being undefined for them.
        unsigned NumOps = VPInst.getNumOperands();
        for (unsigned i = 0; i < NumOps; i++) {
          const VPValue* Op = VPInst.getOperand(i);
          if (isAlwaysUniform(*Op)) {
            VPVectorShape *NewShape = getUniformVectorShape();
            updateVectorShape(Op, NewShape);
          }
        }
      }
    }
  }
}
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
template <typename EntitiesRange>
void VPlanDivergenceAnalysis::markEntitiesAsDivergent(
    const EntitiesRange &Range) {
  // Mark the entity, i.e., the memory pointer as divergent.
  // For the private-entities, We also mark the aliases, which are outside the
  // loop, as divergent.
  for (const auto *RawEntityPtr : Range) {
    // Continue if there is no AllocaInst corresponding to the given entity.
    if (!RawEntityPtr->getIsMemOnly())
      continue;

    auto *AllocaInst =
        RegionLoopEntities->getMemoryDescriptor(RawEntityPtr)->getMemoryPtr();
    LLVM_DEBUG(dbgs() << "Memory entity = " << *AllocaInst << "\n");
    // Mark the Alloca instruction as divergent.
    DivergentLoopEntities.insert(AllocaInst);
    markDivergent(*AllocaInst);
    // Alloca is of a pointer type. Get the pointee size and set a tentative
    // shape.
    assert(isa<PointerType>(AllocaInst->getType()) &&
           "Expected private to be of a pointer-type");
    Type *PointeeTy =
        cast<PointerType>(AllocaInst->getType())->getPointerElementType();
    // We set the stride in terms of bytes.
    uint64_t Stride = Plan->getDataLayout()->getTypeSizeInBits(PointeeTy) >> 3;
    updateVectorShape(AllocaInst, getStridedVectorShape(Stride));

    // Currently, we only deal with aliases for loop-privates. Array-reductions
    // can potentially have aliases, but when the FE is capable of handling it,
    // we intend to take care of those in earlier passes. This code,
    // accordingly, might have to change.
    auto *PrivEntity = dyn_cast<VPPrivate>(RawEntityPtr);
    if (!PrivEntity)
      continue;

    for (const auto &AliasPair : PrivEntity->aliases()) {
      auto *AliasExternalDef = AliasPair.first;
      DivergentLoopEntities.insert(AliasExternalDef);
      markDivergent(*AliasExternalDef);
      updateVectorShape(AliasExternalDef, getRandomVectorShape());
    }
  }
}

#endif // INTEL_CUSTOMIZATION
void VPlanDivergenceAnalysis::compute(VPlan *P, VPLoop *CandidateLoop,
                                      VPLoopInfo *VPLInfo,
                                      VPDominatorTree &VPDomTree,
                                      VPPostDominatorTree &VPPostDomTree,
                                      bool IsLCSSA) {

  Plan = P;
  RegionLoop = CandidateLoop;
  RegionLoopEntities = Plan->getOrCreateLoopEntities(CandidateLoop);
  VPLI = VPLInfo;
  DT = &VPDomTree;
  IsLCSSAForm = IsLCSSA;
  SDA = new SyncDependenceAnalysis(CandidateLoop->getHeader(), VPDomTree,
                                   VPPostDomTree, *VPLInfo);

#if INTEL_CUSTOMIZATION
  // community code divergence due to no phis() interface
  SmallVector<const VPInstruction *, 2> PhiNodes;
  getPhis(CandidateLoop->getHeader(), PhiNodes);
  for (const auto *Phi : PhiNodes) {
#endif
    markDivergent(*Phi);
  }

#if INTEL_CUSTOMIZATION
  // Mark the relevant Loop-entities as divergent.

  // The flag returned by 'isLoopEntitiesImportDone()', when \ false, enforces
  // DA to consider VPExternalDef memory pointers corresponding to loop-entities
  // like inductions, reductions, and privates as 'divergent' for correct
  // propagation of privates' divergence. (The privatization process of those
  // entities replaces them with private memory definitions.) When the flag is \
  // true, all VPExternalDefs are treated as 'uniform' by DA.

  if (EnableVPValueCodegen && !P->isLoopEntitiesPrivatizationDone()) {

    // Mark private entities as divergent.
    markEntitiesAsDivergent(RegionLoopEntities->vpprivates());

    // Mark reduction entities as divergent.
    markEntitiesAsDivergent(RegionLoopEntities->vpreductions());

    // Mark induction entities as divergent.
    markEntitiesAsDivergent(RegionLoopEntities->vpinductions());
  }
#endif

  // Collect instructions that may possibly have non-deterministic result.
  for (auto *B : CandidateLoop->getBlocks())
    if (auto *VPBB = dyn_cast<VPBasicBlock>(B))
      for (const auto &VPInst : VPBB->vpinstructions())
        if (!hasDeterministicResult(VPInst))
          Worklist.push_back(&VPInst);

  // After the scalar remainder loop is extracted, the loop exit condition will
  // be uniform. Source of code divergence here away from community - no
  // getTerminator() interface.
  VPBlockBase *ExitingBlock = CandidateLoop->getExitingBlock();
  if (ExitingBlock) {
    auto LoopExitCond = ExitingBlock->getCondBit();
    assert(LoopExitCond && "Loop exit condition not found");
    if (LoopExitCond) {
      addUniformOverride(*LoopExitCond);
      VPVectorShape *NewShape = getUniformVectorShape();
      updateVectorShape(LoopExitCond, NewShape);
    }
  }

#if INTEL_CUSTOMIZATION
  // Propagate linearity - start at vector loop candidate header phi nodes.
  UndefShape = std::make_unique<VPVectorShape>(VPVectorShape::Undef);
  initializeShapes(PhiNodes);
#endif

  computeImpl();

#if INTEL_CUSTOMIZATION
  // Mark all remaining uniform instructions as having a uniform shape. This
  // isn't necessarily required because we can determine uniformity without
  // checking the vector shape information, but this will ensure a shape is
  // set for all instructions for consistency.
  setVectorShapesForUniforms(CandidateLoop);
  //verifyVectorShapes(CandidateLoop);
#endif

#if INTEL_CUSTOMIZATION
  // Mark the Loop-entities which we had marked as divergent, as uniform again.
  if (EnableVPValueCodegen && !P->isLoopEntitiesPrivatizationDone()) {
    for (auto *EntityPtr : DivergentLoopEntities) {
      markNonDivergent(EntityPtr);
      updateVectorShape(EntityPtr, getUniformVectorShape());
    }
    DivergentLoopEntities.clear();
  }
#endif

  LLVM_DEBUG(print(dbgs(), CandidateLoop));
}

// Note: community version contains a LoopDivergencePrinter class that creates
// a SyncDependenceAnalysis object and a LoopDivergenceAnalysis object. The
// constructor for the LoopDivergenceAnalysis object then calls compute() to
// begin DA execution. For VPlan, all of this is done as part of HCFG
// construction. Thus, those classes are not included in this file.
#endif //INTEL_COLLAB
