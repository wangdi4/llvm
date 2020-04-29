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
#include "IntelVPSOAAnalysis.h"
#include "IntelVPlan.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanLoopInfo.h"
#include "IntelVPlanSyncDependenceAnalysis.h"
#include "IntelVPlanUtils.h"
#include "llvm/Support/CommandLine.h"
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
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::opt<bool>
    DumpDA("vplan-dump-da", cl::init(false), cl::Hidden,
           cl::desc("Print detailed DA dump after analysis is done."));
static cl::opt<bool> DumpPlanDA(
    "vplan-dump-plan-da", cl::init(false), cl::Hidden,
    cl::desc(
        "Print detailed DA dump for the entire VPlan after analysis is done."));
#define DA_FVERIFY_INIT true
#else
#define DA_FVERIFY_INIT false
#endif // !NDEBUG || LLVM_ENABLE_DUMP

static cl::opt<bool>
    VPlanVerifyDA("vplan-verify-da", cl::init(DA_FVERIFY_INIT), cl::Hidden,
                  cl::desc("Run sanity-check on VPlan divergence analysis"));

extern cl::opt<bool> EnableVPValueCodegen;

#define Uni    VPVectorShape::Uni
#define Seq    VPVectorShape::Seq
#define Str    VPVectorShape::Str
#define Rnd    VPVectorShape::Rnd
#define SOASeq VPVectorShape::SOASeq
#define SOAStr VPVectorShape::SOAStr
#define SOARnd VPVectorShape::SOARnd
#define Undef  VPVectorShape::Undef

const VPVectorShape::VPShapeDescriptor
AddConversion[VPVectorShape::NumDescs][VPVectorShape::NumDescs] = {
  /*           Uni,   Seq,   Str,   Rnd,   Undef */
  /* Uni   */ {Uni,   Seq,   Str,   Rnd,   Undef},
  /* Seq   */ {Seq,   Str,   Str,   Rnd,   Undef},
  /* Str   */ {Str,   Str,   Str,   Rnd,   Undef},
  /* Rnd   */ {Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Undef */ {Undef, Undef, Undef, Undef, Undef}
};

const VPVectorShape::VPShapeDescriptor
SubConversion[VPVectorShape::NumDescs][VPVectorShape::NumDescs] = {
  /*           Uni,   Seq,   Str,   Rnd,   Undef */
  /* Uni   */ {Uni,   Str,   Rnd,   Rnd,   Undef},
  /* Seq   */ {Seq,   Rnd,   Rnd,   Rnd,   Undef},
  /* Str   */ {Str,   Rnd,   Rnd,   Rnd,   Undef},
  /* Rnd   */ {Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Undef */ {Undef, Undef, Undef, Undef, Undef}
};

const VPVectorShape::VPShapeDescriptor
MulConversion[VPVectorShape::NumDescs][VPVectorShape::NumDescs] = {
  /*            Uni,   Seq,   Str,   Rnd,   Undef */
  /* Uni   */  {Uni,   Str,   Str,   Rnd,   Undef},
  /* Seq   */  {Str,   Rnd,   Rnd,   Rnd,   Undef},
  /* Str   */  {Str,   Rnd,   Rnd,   Rnd,   Undef},
  /* Rnd   */  {Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Undef */  {Undef, Undef, Undef, Undef, Undef}
};

const VPVectorShape::VPShapeDescriptor
GepConversion[VPVectorShape::NumDescs][VPVectorShape::NumDescs] = {
  /* ptr\index      Uni,      Seq,      Str,      Rnd,      Undef */
  /* Uni      */   {Uni,      Str,      Str,      Rnd,      Undef},
  /* Seq      */   {Str,      Rnd,      Rnd,      Rnd,      Undef},
  /* Str      */   {Str,      Rnd,      Rnd,      Rnd,      Undef},
  /* Rnd      */   {Rnd,      Rnd,      Rnd,      Rnd,      Undef},
  /* SOASeq   */   {SOASeq,   SOAStr,   SOARnd,   SOARnd,   Undef},
  /* SOAStr   */   {SOAStr,   SOAStr,   SOARnd,   SOARnd,   Undef},
  /* SOARnd   */   {SOARnd,   SOARnd,   SOARnd,   SOARnd,   Undef},
  /* Undef    */   {Undef,    Undef,    Undef,    Undef,    Undef}
};

const VPVectorShape::VPShapeDescriptor
SelectConversion[VPVectorShape::NumDescs][VPVectorShape::NumDescs] = {
  /*            Uni,   Seq,   Str,   Rnd,   Undef */
  /* Uni   */  {Uni,   Str,   Str,   Rnd,   Undef},
  /* Seq   */  {Str,   Seq,   Str,   Rnd,   Undef},
  /* Str   */  {Str,   Str,   Str,   Rnd,   Undef},
  /* Rnd   */  {Rnd,   Rnd,   Rnd,   Rnd,   Undef},
  /* Undef */  {Undef, Undef, Undef, Undef, Undef}
};

// Undefine the defines used in table initialization. Code below is
// expected to use values from VPVectorShape directly.
#undef Uni
#undef Seq
#undef Str
#undef Rnd
#undef SOASeq
#undef SOAStr
#undef SOARnd
#undef Undef

static void assertOperandsDefined(const VPInstruction &I,
                                  VPlanDivergenceAnalysis *DA) {
  assert(none_of(I.operands(),
                 [=](VPValue *Op) {
                   return DA->getVectorShape(Op).isUndefined();
                 }) &&
         "Undefined shape not expected!");
}

void VPlanDivergenceAnalysis::markDivergent(const VPValue &DivVal) {
  // Community version also checks to see if DivVal is a function argument.
  // For VPlan, function arguments are ExternalDefs, so check that here instead.
  assert(!isAlwaysUniform(DivVal) && "cannot be a divergent");
  if (getVectorShape(&DivVal).isAnyStrided())
    return;
  updateVectorShape(&DivVal, getRandomVectorShape());
}

// Mark UniVal as a value that is non-divergent.
void VPlanDivergenceAnalysis::markUniform(const VPValue &UniVal) {
  updateVectorShape(&UniVal, getUniformVectorShape());
}

unsigned VPlanDivergenceAnalysis::getTypeSizeInBytes(Type *Ty) const {
  assert(Ty && "Expected a non-null value for argument type.");
  return Plan->getDataLayout()->getTypeSizeInBits(Ty) >> 3;
}

bool VPlanDivergenceAnalysis::isUnitStridePtr(const VPValue *Ptr) const {
  bool IsNegOneStride;
  return isUnitStridePtr(Ptr, IsNegOneStride);
}

bool VPlanDivergenceAnalysis::isUnitStridePtr(const VPValue *Ptr,
                                              bool &IsNegOneStride) const {
  assert(isa<PointerType>(Ptr->getType()) &&
         "Expect argument of isUnitStridePtr to be of PointerType.");
  // Compute the pointee-size in bytes.
  unsigned PtrNumBytes =
      getTypeSizeInBytes(Ptr->getType()->getPointerElementType());

  auto VectorShape = getVectorShape(Ptr);

  // Compare stride value and pointee-size in bytes to appropriately set
  // IsNegOneStride and return true for unit stride case.
  if (VectorShape.isStrided() && VectorShape.hasKnownStride()) {
    auto StrideVal = VectorShape.getStrideVal();
    if (std::abs(StrideVal) == PtrNumBytes) {
      IsNegOneStride = StrideVal < 0;
      return true;
    }
  }

  IsNegOneStride = false;
  return false;
}

#if INTEL_CUSTOMIZATION
static bool hasDeterministicResult(const VPInstruction &I) {
  if (I.getType()->isVoidTy())
    return true;

  // Be conservative and assume any instruction with side effects can produce
  // non-deterministic value. An example of an instruction with side effects but
  // still producing deterministic value is a function like
  //
  //    define i32* @foo(i32* %ptr) {
  //      store i32 42, i32* %ptr
  //      ret i32* %ptr
  //    }
  //
  // However, we currently don't have an easy way to detect such determinism and
  // use conservative approach.
  return !I.mayHaveSideEffects();
}
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
#else
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
#endif // INTEL_CUSTOMIZATION

// Push the instruction onto the Worklist.
bool VPlanDivergenceAnalysis::pushToWorklist(const VPInstruction &I) {
  if (OnWorklist.insert(&I).second) {
    Worklist.push(&I);
    return true;
  }
  return false;
}

// Get the instruction from the Worklist.
const VPInstruction *VPlanDivergenceAnalysis::popFromWorklist() {
  if (Worklist.empty())
    return nullptr;

  const VPInstruction *I = Worklist.front();
  Worklist.pop();
  OnWorklist.erase(I);
  return I;
}

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
    const VPBasicBlock &ObservingBlock, const VPValue &Val) const {
  const auto *Inst = dyn_cast<const VPInstruction>(&Val);
  if (!Inst)
    return false;
  // check whether any divergent loop carrying @Val terminates before control
  // proceeds to @ObservingBlock
  const auto *VPLp = VPLI->getLoopFor(Inst->getParent());
  if (!VPLp || VPLp->contains(&ObservingBlock))
    return false;

  // FIXME this is imprecise (liveouts of uniform exits appear divergent,
  // eventhough they are uniform)
  if (!IsLCSSAForm) {
    // check whether any divergent loop carrying Val terminates before control
    // proceeds to ObservingBlock

    for (; VPLp && VPLp != RegionLoop && !VPLp->contains(&ObservingBlock);
         VPLp = VPLp->getParentLoop()) {
      if (DivergentLoops.find(VPLp) != DivergentLoops.end())
        return true;
    }
  } else {
    // All loop live-outs are funneled through LCSSA phis that sit on immediate
    // exit blocks. As such, only LCSSA phi nodes can observed temporal
    // divergence.
    return isDivergentLoopExit(ObservingBlock);
  }

  return false;
}

VPVectorShape
VPlanDivergenceAnalysis::getObservedShape(const VPBasicBlock &ObserverBlock,
                                          const VPValue &Val) {
  if (isTemporalDivergent(ObserverBlock, Val))
    return getRandomVectorShape();

  return getVectorShape(&Val);
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

bool VPlanDivergenceAnalysis::inRegion(const VPBasicBlock &BB) const {
  if (!RegionLoop)
    return true;

  return RegionLoop->contains(&BB);
}

// marks all users of loop-carried values of the loop headed by @LoopHeader as
// divergent
void VPlanDivergenceAnalysis::taintLoopLiveOuts(
    const VPBasicBlock &LoopHeader) {
  auto *DivLoop = VPLI->getLoopFor(&LoopHeader);
  assert(DivLoop && "loopHeader is not actually part of a loop");

  SmallVector<VPBasicBlock *, 8> TaintStack;
  DivLoop->getExitBlocks(TaintStack);

  // Otherwise potential users of loop-carried values could be anywhere in the
  // dominance region of @DivLoop (including its fringes for phi nodes)
  DenseSet<const VPBasicBlock *> Visited;
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
      // source.
      pushPHINodes(*UserBlock, true);
      continue;
    }

    // taint outside users of values carried by @DivLoop
    for (VPInstruction &I : *UserBlock) {
      if (isAlwaysUniform(I))
        continue;

      for (auto &Op : I.operands()) {
        auto *OpInst = dyn_cast<VPInstruction>(Op);
        if (!OpInst)
          continue;
        if (DivLoop->contains(OpInst->getParent())) {
          pushToWorklist(I);
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

void VPlanDivergenceAnalysis::pushPHINodes(const VPBasicBlock &Block,
                                           bool PushAll) { // INTEL

#if INTEL_CUSTOMIZATION
  for (const auto &Phi : Block.getVPPhis()) {
    if (isDivergent(Phi) && !PushAll)
#endif // INTEL_CUSTOMIZATION
      continue;
    pushToWorklist(Phi);
  }
}

void VPlanDivergenceAnalysis::pushUsers(const VPValue &V) {
  for (const auto *User : V.users()) {
    const auto *UserInst = dyn_cast<const VPInstruction>(User);
    if (!UserInst)
      continue;

    // only compute divergent/shapes inside loop
    if (!inRegion(*UserInst))
      continue;
    pushToWorklist(*UserInst);
  }
}

bool VPlanDivergenceAnalysis::propagateJoinDivergence(
    const VPBasicBlock &JoinBlock, const VPLoop *BranchLoop) {
  LLVM_DEBUG(dbgs() << "\tpropJoinDiv " << JoinBlock.getName() << "\n");

  // Ignore divergence outside the region.
  if (!inRegion(JoinBlock))
    return false;

  // Disjoint-paths divergent at \p joinBlock.
  if (!addJoinDivergentBlock(JoinBlock))
    return false;

  // Push non-divergent phi nodes in \p JoinBlock to the worklist.
  pushPHINodes(JoinBlock, false);

  // joinBlock is a divergent loop exit.
  if (BranchLoop && !BranchLoop->contains(&JoinBlock))
    return true;

  return false;
}

#if INTEL_CUSTOMIZATION
// Main source of community code divergence here is that we don't represent
// branch instructions explicitly in VPlan yet. Thus, we have to use the
// condition of the branch to determine if the branch is divergent. Not a big
// deal, but we should be able to easily match the community code once VPlan
// is updated.
#endif // INTEL_CUSTOMIZATION
void VPlanDivergenceAnalysis::propagateBranchDivergence(
    const VPBasicBlock *CondBlock) {
  LLVM_DEBUG(dbgs() << "propBranchDiv " << CondBlock->getName() << "\n");
  const auto *BranchLoop = VPLI->getLoopFor(CondBlock);

  // whether there is a divergent loop exit from @BranchLoop (if any)
  bool IsBranchLoopDivergent = false;

  // @BranchLoop is a divergent loop due to the divergent branch created by
  // @Cond.
  for (const auto *JoinBlock : SDA->joinBlocks(*CondBlock)) {
    if (propagateJoinDivergence(*JoinBlock, BranchLoop)) {
      addDivergentLoopExit(*JoinBlock);
      IsBranchLoopDivergent |= true;
    }

    if (IsBranchLoopDivergent) {
      if (!addDivergentLoops(*BranchLoop))
        return;
      propagateLoopDivergence(*BranchLoop);
    }
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

  // propagate divergence
  while (const VPInstruction *NextI = popFromWorklist()) {
    const VPInstruction &I = *NextI;

    // maintain uniformity of overrides
    if ((isAlwaysUniform(I)) && !getVectorShape(&I).isUndefined())
      continue;

    bool IsPhiOrTerminatorNode = I.getOpcode() == Instruction::PHI ||
                                 I.getOpcode() == Instruction::Br;
    bool ShapeUpdated = false;
    VPVectorShape NewShape;
    if (IsPhiOrTerminatorNode) {
      // Some operands might be undefined for header phis or terminators. That
      // is fine, the phi will be re-visited during backpropagation.
      NewShape = computeVectorShape(&I);
      ShapeUpdated |= updateVectorShape(&I, NewShape);
    } else {
      assertOperandsDefined(I, this);
      NewShape = computeVectorShape(&I);
      ShapeUpdated |= updateVectorShape(&I, NewShape);
    }

    if (ShapeUpdated) {
      pushUsers(I);

      // Propagate branch divergence from the block(s) containing the CondBit.
      // TODO: this code should be removed after the introduction of the
      // VPInstruction for branches.
      if (!NewShape.isUniform() && CondBit2BlockMap.count(&I)) {
        // propagate control divergence to affected instructions
        for (auto *Block : CondBit2BlockMap[&I])
          propagateBranchDivergence(Block);
      }
    }
  }
}

bool VPlanDivergenceAnalysis::isAlwaysUniform(const VPValue &V) const {
  if (isa<VPMetadataAsValue>(V) || isa<VPConstant>(V) || isa<VPExternalDef>(V))
    return true;

  // TODO: We have a choice on how to handle functions such as get_global_id().
  // Currently, OCL VecClone is already treating such calls as linear and
  // hoisting them outside of the inserted loop. As such, it adds the
  // appropriate stride to any users. Thus, here we treat these calls as uniform
  // because the added instructions by OCL VecClone to calculate stride will
  // cause DA to propagate the correct stride as is.

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
  if (isAlwaysUniform(V))
    return false;

  return !getVectorShape(&V).isUniform();
}

#if INTEL_CUSTOMIZATION
VPVectorShape VPlanDivergenceAnalysis::getVectorShape(const VPValue *V) const {
  auto *NonConstDA = const_cast<VPlanDivergenceAnalysis *>(this);
  if (isAlwaysUniform(*V))
    return NonConstDA->getUniformVectorShape();

  // FIXME: This needs an explicit vector IV.
  if (Plan->isBackedgeUniformityForced())
    if (VPBasicBlock *LoopLatch = RegionLoop->getLoopLatch())
      if (LoopLatch->getCondBit() == V)
        return NonConstDA->getUniformVectorShape();

  auto ShapeIter = VectorShapes.find(V);
  if (ShapeIter != VectorShapes.end())
    return ShapeIter->second;
  return VPVectorShape::getUndef();
}

bool VPlanDivergenceAnalysis::shapesAreDifferent(VPVectorShape OldShape,
                                                 VPVectorShape NewShape) {
  // For the first-time this function is called in context of private-entities,
  // the OldShape is undefined.

  // FIXME: That should be operator== modulo adjustment for undef being
  // different from another undef.
  if ((OldShape.isUndefined() && !NewShape.isUndefined()) ||
      (!OldShape.isUndefined() && NewShape.isUndefined()) ||
      (!OldShape.isUndefined() && !NewShape.isUndefined() &&
       (OldShape.getShapeDescriptor() != NewShape.getShapeDescriptor() ||
        (OldShape.hasKnownStride() && NewShape.hasKnownStride() &&
         OldShape.getStrideVal() != NewShape.getStrideVal())))) {
    return true;
  }
  return false;
}

bool VPlanDivergenceAnalysis::updateVectorShape(const VPValue *V,
                                                VPVectorShape Shape) {
  VPVectorShape OldShape = getVectorShape(V);

  // Has shape changed in any way?
  if (shapesAreDifferent(OldShape, Shape)) {
    VectorShapes[V] = Shape;
    return true;
  }
  return false;
}

VPVectorShape VPlanDivergenceAnalysis::getUniformVectorShape() {
  return {VPVectorShape::Uni, getConstantInt(0)};
}

VPVectorShape VPlanDivergenceAnalysis::getRandomVectorShape() {
  return {VPVectorShape::Rnd};
}

VPVectorShape
VPlanDivergenceAnalysis::getSequentialVectorShape(int64_t Stride) {
  return {VPVectorShape::Seq, getConstantInt(Stride)};
}

VPVectorShape VPlanDivergenceAnalysis::getStridedVectorShape(int64_t Stride) {
  return {VPVectorShape::Str, getConstantInt(Stride)};
}

/// Return true if the given variable has SOA Shape.
bool VPlanDivergenceAnalysis::isSOAShape(const VPValue *Val) const {
  assert(Val && "Expected a non-null value.");
  auto Shape = getVectorShape(Val).getShapeDescriptor();
  return Shape == VPVectorShape::SOASeq || Shape == VPVectorShape::SOAStr ||
         Shape == VPVectorShape::SOARnd;
}

// Returns a SOASequential vector shape with the given stride.
VPVectorShape
VPlanDivergenceAnalysis::getSOASequentialVectorShape(int64_t Stride) {
  return {VPVectorShape::SOASeq, getConstantInt(Stride)};
}

// Returns a SOARandom vector shape with the given stride.
VPVectorShape VPlanDivergenceAnalysis::getSOARandomVectorShape() {
  return {VPVectorShape::SOARnd};
}

// Verify the shape of each instruction in give Block \p VPBB.
void VPlanDivergenceAnalysis::verifyBasicBlock(const VPBasicBlock *VPBB) {
  for (auto &VPInst : *VPBB) {
    assert(!getVectorShape(&VPInst).isUndefined() &&
           "Shape has not been defined");
    (void)VPInst;
  }
}

// Verify that there are no undefined shapes after divergence analysis.
// Also ensure that divergent/uniform properties are consistent with vector
// shapes.
void VPlanDivergenceAnalysis::verifyVectorShapes() {
  for (VPBasicBlock *VPBB : depth_first(Plan->getEntryBlock()))
    verifyBasicBlock(VPBB);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// print function differs from the community version because VPlan is VPLoop
// based and not Module based (function DA).
void VPlanDivergenceAnalysis::print(raw_ostream &OS, const VPLoop *VPLp) {
  ReversePostOrderTraversal<VPBasicBlock *> RPOT(VPLp ? VPLp->getHeader()
                                                      : Plan->getEntryBlock());
  OS << "\nPrinting Divergence info for ";
  if (VPLp)
    OS << *VPLp;
  else
    OS << Plan->getName();
  OS << "\n";

  for (VPBasicBlock *VPBB : RPOT) {
    OS << "Basic Block: " << VPBB->getName() << "\n";
    for (auto &VPInst : *VPBB) {
      if (!PrintTerminatorInst && isa<VPBranchInst>(VPInst))
        continue;
      if (isDivergent(VPInst))
        OS << "Divergent: ";
      else
        OS << "Uniform: ";
      getVectorShape(&VPInst).print(OS);
      OS << ' ';
      VPInst.dump(OS);
    }
    OS << "\n";
  }
}

#endif // !NDEBUG || LLVM_ENABLE_DUMP

VPConstant* VPlanDivergenceAnalysis::getConstantInt(int64_t Val) {
  LLVMContext &C = *Plan->getLLVMContext();
  ConstantInt *CInt = ConstantInt::get(Type::getInt64Ty(C), Val);
  VPConstant *VPCInt = Plan->getVPConstant(CInt);
  return VPCInt;
}

bool VPlanDivergenceAnalysis::getConstantIntVal(VPValue *V, int64_t &IntVal) {
  if (V && isa<VPConstant>(V)) {
    Constant *C = cast<Constant>(V->getUnderlyingValue());
    if (ConstantInt *CInt = dyn_cast<ConstantInt>(C)) {
      IntVal = CInt->getSExtValue();
      return true;
    }
  }
  return false;
}

VPVectorShape VPlanDivergenceAnalysis::computeVectorShapeForUnaryInst(
    const VPInstruction *I) {
  VPValue *Op = I->getOperand(0);
  if (getObservedShape(*I->getParent(), *Op).isUniform())
    return getUniformVectorShape();
  return getRandomVectorShape();
}

VPVectorShape VPlanDivergenceAnalysis::computeVectorShapeForBinaryInst(
    const VPInstruction *I) {

  VPValue *Op0 = I->getOperand(0);
  VPValue *Op1 = I->getOperand(1);

  LLVMContext &C = *Plan->getLLVMContext();

  // Put VPConstants on the right-hand side of the expression for commutative
  // instructions. This can later be used to calculate a new known stride for I
  // given that the left-hand side has a known stride.
  if (!isa<VPConstant>(Op1) && Instruction::isCommutative(I->getOpcode()))
    std::swap(Op0, Op1);

  VPVectorShape Shape0 = getObservedShape(*I->getParent(), *Op0);
  VPVectorShape Shape1 = getObservedShape(*I->getParent(), *Op1);

  if (Shape0.isUniform() && Shape1.isUniform())
    return getUniformVectorShape();

  VPVectorShape::VPShapeDescriptor Desc0 = Shape0.getShapeDescriptor();
  VPVectorShape::VPShapeDescriptor Desc1 = Shape1.getShapeDescriptor();

  switch (I->getOpcode()) {
    case Instruction::Mul:
    case Instruction::FMul: {
      // A constant integer multiplied by a known stride results in another
      // known stride that has been scaled.
      VPValue *NewStride = nullptr;
      int64_t Op1IntVal;
      int64_t Op0StrideIntVal;
      bool Op1IsInt = getConstantIntVal(Op1, Op1IntVal);
      bool Shape0StrideIsInt = getConstantIntVal(Shape0.getStride(),
                                                 Op0StrideIntVal);
      if (Op1IsInt && Shape0StrideIsInt) {
        int64_t NewStrideVal = Op1IntVal * Op0StrideIntVal;
        ConstantInt *NewStrideInt = ConstantInt::get(Type::getInt64Ty(C),
                                                     NewStrideVal);
        NewStride = Plan->getVPConstant(NewStrideInt);
      }

      VPVectorShape::VPShapeDescriptor NewDesc;
      // Not sure if we can assume mul by 0/1 is optimized since we can
      // vectorize at O0/O1.
      if (Op1IsInt && Op1IntVal == 0)
        NewDesc = VPVectorShape::Uni;
      else if (Op1IsInt && Op1IntVal == 1)
        NewDesc = Shape0.getShapeDescriptor();
      else
        NewDesc = MulConversion[Desc0][Desc1];
      return {NewDesc, NewStride};
    }
    case Instruction::Add:
    case Instruction::FAdd: {
      VPValue *NewStride = nullptr;
      int64_t Op0StrideIntVal;
      int64_t Op1StrideIntVal;
      bool Op0StrideIsInt = getConstantIntVal(Shape0.getStride(),
                                              Op0StrideIntVal);
      bool Op1StrideIsInt = getConstantIntVal(Shape1.getStride(),
                                              Op1StrideIntVal);
      if (Op0StrideIsInt && Op1StrideIsInt) {
        uint64_t NewStrideVal = Op0StrideIntVal + Op1StrideIntVal;
        ConstantInt *NewStrideInt = ConstantInt::get(Type::getInt64Ty(C),
                                                     NewStrideVal);
        NewStride = Plan->getVPConstant(NewStrideInt);
      }
      VPVectorShape::VPShapeDescriptor NewDesc = AddConversion[Desc0][Desc1];
      return {NewDesc, NewStride};
    }
    case Instruction::Sub:
    case Instruction::FSub: {
      VPValue *NewStride = nullptr;
      int64_t Op0StrideIntVal;
      int64_t Op1StrideIntVal;
      bool Op0StrideIsInt =
          getConstantIntVal(Shape0.getStride(), Op0StrideIntVal);
      bool Op1StrideIsInt =
          getConstantIntVal(Shape1.getStride(), Op1StrideIntVal);
      if (Op0StrideIsInt && Op1StrideIsInt) {
        int64_t NewStrideVal = Op0StrideIntVal - Op1StrideIntVal;
        ConstantInt *NewStrideInt =
            ConstantInt::get(Type::getInt64Ty(C), NewStrideVal);
        NewStride = Plan->getVPConstant(NewStrideInt);
        VPVectorShape::VPShapeDescriptor NewDesc;
        switch (NewStrideVal) {
        case 0:
          NewDesc = VPVectorShape::Uni;
          break;
        case 1:
        case -1:
          NewDesc = VPVectorShape::Seq;
          break;
        default:
          NewDesc = VPVectorShape::Str;
          break;
        }
        return {NewDesc, NewStride};
      }
      VPVectorShape::VPShapeDescriptor NewDesc = SubConversion[Desc0][Desc1];
      return {NewDesc, NewStride};
    }
    case Instruction::And: {
      if (VPlanDAIgnoreOverflow) {
        // AND operation with UINT_MAX indicates an integer overflow check and
        // clamping. Propagate the shape of the operand being checked for
        // overflow.
        if (auto *ConstOp1 = dyn_cast<VPConstant>(Op1)) {
          if (ConstOp1->isConstantInt() && ConstOp1->getZExtValue() == UINT_MAX)
            return {Desc0, Shape0.getStride()};
        }
      }
      LLVM_FALLTHROUGH;
    }
    default:
      return getRandomVectorShape();
  }
}

VPVectorShape VPlanDivergenceAnalysis::computeVectorShapeForCastInst(
    const VPInstruction *I) {

  VPValue *Op0 = I->getOperand(0);
  VPVectorShape Shape0 = getObservedShape(*I->getParent(), *Op0);
  if (Shape0.isUniform())
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
      return Shape0;
    case Instruction::BitCast: {
      PointerType *SrcPtrTy =
          dyn_cast<PointerType>(I->getOperand(0)->getType());
      if (SrcPtrTy || (I->getOperand(0)->getType() == I->getType())) {
        // Case 1: %Y = bitcast i32* %x to sint*          ; yields sint*:%x
        // Case 2: %Y = bitcast i32* %x to <3 x i32>*     ; yields <3 x i32>*:%x
        // Case 3: %Z = bitcast i32 %x to i32             ; yields i32: %x
        // Case 3, is commonly seen when doing codegen along HIR-path, where
        // as part of decomposition, temporary copy-assigments are generated.
        return Shape0;
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

VPVectorShape
VPlanDivergenceAnalysis::computeVectorShapeForSOAGepInst(const VPInstruction *I) {
  const auto &VPBB = *I->getParent();
  VPValue *PtrOp = I->getOperand(0);
  VPVectorShape PtrShape = getObservedShape(VPBB, *PtrOp);

  unsigned NumOperands = I->getNumOperands();
  // If any of the gep indices, except the last, are not uniform, then return
  // random shape.
  for (unsigned i = 1; i < NumOperands - 1; i++) {
    const VPValue *Op = I->getOperand(i);
    VPVectorShape OpShape = getVectorShape(Op);
    if (!OpShape.isUniform())
      return getSOARandomVectorShape();
  }

  const VPValue *LastIdx = I->getOperand(NumOperands - 1);
  VPVectorShape IdxShape = getObservedShape(VPBB, *LastIdx);

  VPConstant *NewStride = nullptr;

  VPVectorShape::VPShapeDescriptor PtrShapeDesc =
      PtrShape.getShapeDescriptor();

  VPVectorShape::VPShapeDescriptor IdxShapeDesc =
      IdxShape.getShapeDescriptor();

  VPVectorShape::VPShapeDescriptor NewDesc =
      GepConversion[PtrShapeDesc][IdxShapeDesc];

  // If shape is not random, then a new stride (in bytes) can be calculated for
  // the gep. Gep stride is always in bytes.
  if (NewDesc != VPVectorShape::SOARnd) {
    Type *PointedToTy = cast<PointerType>(I->getType())->getElementType();
    uint64_t PointedToTySize = getTypeSizeInBytes(PointedToTy);
    // For known strides:
    // 1) Uniform gep on an array-private should result in strided-access with
    //   stride = PointedToTySize.
    // 2) Unit-Stride gep's result in strided access with stride = VF *
    // PointedToTySize. Since we do not know the value of VF, the stride is unknown.
    if (PtrShape.hasKnownStride() && IdxShape.hasKnownStride()) {
      VPValue *IdxStride = IdxShape.getStride();

      uint64_t IdxStrideVal =
          cast<ConstantInt>(IdxStride->getUnderlyingValue())->getSExtValue();

      if (IdxStrideVal == 0)
        NewStride = getConstantInt(PointedToTySize);
      else {
        // TODO: Set up symbolic stride and express it as multiple of
        // VF (CMPLRLLVM-19061).
        if (PtrShape.isSOAUnitStride())
          NewStride = getConstantInt(IdxStrideVal * PointedToTySize);
        else if (PtrShape.isSOAStrided())
          // Can be refined further and the correct stride set.
          NewStride = nullptr;
        NewDesc = VPVectorShape::SOAStr;
      }
    }
  }
  return {NewDesc, NewStride};
}


VPVectorShape
VPlanDivergenceAnalysis::computeVectorShapeForMemAddrInst(const VPInstruction *I) {

  const auto &VPBB = *I->getParent();
  VPValue *PtrOp = I->getOperand(0);

  // If this is a GEP on SOA variable, invoke the SOA-specific GEP-Shape
  // computation function.
  if (isSOAShape(PtrOp))
    return computeVectorShapeForSOAGepInst(I);

  VPVectorShape PtrShape = getObservedShape(VPBB, *PtrOp);
  unsigned NumOperands = I->getNumOperands();

  // If any of the gep indices, except the last, are not uniform, then return
  // random shape.
  for (unsigned i = 1; i < NumOperands - 1; i++) {
    const VPValue *Op = I->getOperand(i);
    VPVectorShape OpShape = getVectorShape(Op);
    if (!OpShape.isUniform())
      return getRandomVectorShape();
  }

  const VPValue *LastIdx = I->getOperand(NumOperands - 1);
  VPVectorShape IdxShape = getObservedShape(VPBB, *LastIdx);

  // Special processing for subscript instructions which could have struct
  // offsets in 0th dimension.
  if (auto *Subscript = dyn_cast<VPSubscriptInst>(I)) {
    ArrayRef<unsigned> ZeroDimOffsets = Subscript->getStructOffsets(0);
    if (!ZeroDimOffsets.empty() && !IdxShape.isUniform())
      // 0-th dimension index is divergent and we have struct offsets, do not
      // proceed.
      return getRandomVectorShape();
  }

  VPConstant *NewStride = nullptr;

  VPVectorShape::VPShapeDescriptor PtrShapeDesc =
      PtrShape.getShapeDescriptor();

  VPVectorShape::VPShapeDescriptor IdxShapeDesc =
      IdxShape.getShapeDescriptor();

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
    uint64_t PointedToTySize = getTypeSizeInBytes(PointedToTy);
    // For known strides:
    // 1) Uniform gep should result in 0 stride (i.e., pointer and idx are
    //    uniform).
    // 2) When strides are known for both pointer and idx, new stride should
    //    also be known.
    // Otherwise, if either stride is unknown, this should result in an unknown
    // stride (nullptr).
    if (PtrShape.hasKnownStride() && IdxShape.hasKnownStride()) {
      VPValue *PtrStride = PtrShape.getStride();
      VPValue *IdxStride = IdxShape.getStride();

      uint64_t PtrStrideVal =
          cast<ConstantInt>(PtrStride->getUnderlyingValue())->getSExtValue();
      uint64_t IdxStrideVal =
          cast<ConstantInt>(IdxStride->getUnderlyingValue())->getSExtValue();

      assert((PtrStrideVal == 0 || IdxStrideVal == 0) &&
             "Expect one of PtrStrideVal or IdxStrideVal to be 0.");

      NewStride = getConstantInt(PtrStrideVal + PointedToTySize * IdxStrideVal);

      // See if we can refine a strided pointer to a unit-strided pointer by
      // checking if new stride value is the same as size of pointedto type.
      const APInt &NewStrideVal =
          cast<ConstantInt>(NewStride->getUnderlyingValue())->getValue();
      uint64_t NewStrideValAbs = NewStrideVal.abs().getZExtValue();
      if (NewDesc == VPVectorShape::Str && PointedToTySize == NewStrideValAbs)
        NewDesc = VPVectorShape::Str;
    }
  }
  return {NewDesc, NewStride};
}

VPVectorShape
VPlanDivergenceAnalysis::computeVectorShapeForPhiNode(const VPPHINode *Phi) {
  // Incoming value shapes could be uniform, but the parent of the phi node may
  // be reached through a divergent branch. If so, the phi is divergent and
  // return random shape.
  if (!Phi->hasConstantOrUndefValue() && isJoinDivergent(*Phi->getParent()))
    return getRandomVectorShape();

  // Compute shape for phi node.
  SmallVector<VPVectorShape, 2> Shapes;
  VPVectorShape NewShape = VPVectorShape::getUndef();
  Shapes.push_back(NewShape);
  for (unsigned i = 0; i < Phi->getNumIncomingValues(); i++) {
    VPValue *IncomingVal = Phi->getIncomingValue(i);
    VPVectorShape IncomingShape =
        getObservedShape(*Phi->getParent(), *IncomingVal);
    NewShape = VPVectorShape::joinShapes(NewShape, IncomingShape);
    Shapes.push_back(NewShape);
  }

  return NewShape;
}

VPVectorShape VPlanDivergenceAnalysis::computeVectorShapeForLoadInst(
    const VPInstruction *I) {

  VPValue *PtrOp = I->getOperand(0);
  VPVectorShape PtrShape = getObservedShape(*I->getParent(), *PtrOp);
  if (Instruction *UI = I->getInstruction())
    if (cast<LoadInst>(UI)->isVolatile())
      return getRandomVectorShape();

  if (PtrShape.isUniform())
    // FIXME: volatile load from uniform memory should still be marked as
    // divergent/random-shaped.
    return PtrShape;

  return getRandomVectorShape();
}

VPVectorShape VPlanDivergenceAnalysis::computeVectorShapeForStoreInst(
    const VPInstruction *I) {

  const auto &VPBB = *I->getParent();
  VPValue *ValOp = I->getOperand(0);
  VPValue *PtrOp = I->getOperand(1);
  VPVectorShape ValShape = getObservedShape(VPBB, *ValOp);
  VPVectorShape PtrShape = getObservedShape(VPBB, *PtrOp);
  return VPVectorShape::joinShapes(ValShape, PtrShape);
}

VPVectorShape VPlanDivergenceAnalysis::computeVectorShapeForCmpInst(
    const VPCmpInst *I) {

  // For now, shape optimizations for compare instructions are kept basic. For
  // integer comparisons, we can introduce other uniformity optimizations based
  // on KnownBits, but those have been excluded for now.
  const auto &VPBB = *I->getParent();
  VPValue *Op0 = I->getOperand(0);
  VPValue *Op1 = I->getOperand(1);
  VPVectorShape Shape0 = getObservedShape(VPBB, *Op0);
  VPVectorShape Shape1 = getObservedShape(VPBB, *Op1);

  if (Shape0.isUniform() && Shape1.isUniform())
    return getUniformVectorShape();

  return getRandomVectorShape();
}

VPVectorShape VPlanDivergenceAnalysis::computeVectorShapeForInsertExtractInst(
    const VPInstruction *I) {

  VPValue *VectorOp = I->getOperand(0);
  VPValue *IdxOp = I->getOperand(1);
  VPVectorShape VectorOpShape = getVectorShape(VectorOp);
  VPVectorShape IdxOpShape = getVectorShape(IdxOp);
  return VPVectorShape::joinShapes(VectorOpShape, IdxOpShape);
}

VPVectorShape VPlanDivergenceAnalysis::computeVectorShapeForSelectInst(
    const VPInstruction *I) {

  VPValue *Mask = I->getOperand(0);
  VPVectorShape MaskShape = getVectorShape(Mask);
  if (MaskShape.isUniform()) {
    const auto &VPBB = *I->getParent();
    VPValue *Op1 = I->getOperand(1);
    VPValue *Op2 = I->getOperand(2);
    VPVectorShape Shape1 = getObservedShape(VPBB, *Op1);
    VPVectorShape Shape2 = getObservedShape(VPBB, *Op2);
    VPVectorShape::VPShapeDescriptor Shape1Desc = Shape1.getShapeDescriptor();
    VPVectorShape::VPShapeDescriptor Shape2Desc = Shape2.getShapeDescriptor();
    int64_t MaskConstIntVal;
    bool MaskIsConstInt = getConstantIntVal(Mask, MaskConstIntVal);
    if (isa<VPConstant>(Mask) && MaskIsConstInt) {
      if (MaskConstIntVal)
        // mask = 1, shape is inherited from Op1
        return Shape1;
      else
        // mask == 0, shape is inherited from Op2
        return Shape2;
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
        NewStride = Shape1.getStride();
    }

    return {NewDesc, NewStride};
  }

  // Non-uniform mask results in selection of values from Op1 and Op2.
  return getRandomVectorShape();
}

VPVectorShape VPlanDivergenceAnalysis::computeVectorShapeForCallInst(
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
  const auto &VPBB = *I->getParent();
  for (unsigned i = 0; i < NumOps; i++) {
    VPValue* Op = I->getOperand(i);
    if (!getObservedShape(VPBB, *Op).isUniform()) {
      AllOpsUniform = false;
      break;
    }
  }

  if (AllOpsUniform)
    return getUniformVectorShape();

  return getRandomVectorShape();
}

// Computes vector shape for AllocatePrivate instruction.
VPVectorShape VPlanDivergenceAnalysis::computeVectorShapeForAllocatePrivateInst(
    const VPAllocatePrivate *AI) {
  // Allocate-private is of a pointer type. Get the pointee size and set a
  // tentative shape.
  Type *PointeeTy = cast<PointerType>(AI->getType())->getPointerElementType();

  // Check for SOA-layout.
  if (AI->isSOALayout() && isa<ArrayType>(PointeeTy)) {
    uint64_t StrideVal =
        getTypeSizeInBytes(cast<ArrayType>(PointeeTy)->getElementType());
    return getSOASequentialVectorShape(StrideVal);
  }

  // We set the stride in terms of bytes.
  int64_t Stride = getTypeSizeInBytes(PointeeTy);
  updateVectorShape(AI, getStridedVectorShape(Stride));

  return getVectorShape(AI);
}

// Computes vector shape for induction-init instruction.
VPVectorShape VPlanDivergenceAnalysis::computeVectorShapeForInductionInit(
    const VPInductionInit *Init) {
  VPValue *Step = Init->getStep();
  auto *StepConst = dyn_cast<VPConstant>(Step);

  // If we do not have a constant step, return random shape.
  if (!StepConst) {
    // This could be a VPExternalDef (a non-constant value), i.e., a
    // variable step IV. We should set the shape to be random if we
    // cannot infer the step-size.
    assert(isa<VPExternalDef>(Step) &&
           "Expect the non-constant to be VPExternalDef.");
    return getRandomVectorShape();
  }

  // If we do not have an integer step, return random shape.
  if (!StepConst->isConstantInt())
    return getRandomVectorShape();

  int StepInt;
  // If this is a pointer induction, compute the step-size in terms of
  // bytes, using the size of the pointee.
  if (isa<PointerType>(Init->getType())) {
    unsigned TypeSizeInBytes;

    // Handle strides for pointer-inductions appropriately.
    // A 'uniform' pointer-shape indicates that we are dealing with non-private
    // data. Private-data, and its aliases (via casts and GEPs) will have
    // non-'uniform' shape and the stride would be the same as that of 'alloca'
    // given that we are dealing with these instructions in the loop-preheader.
    if (Init->getBinOpcode() == Instruction::GetElementPtr) {
      auto InitShape = getVectorShape(Init->getOperand(0));

      // We can have strided-shape with unknown-stride. Return random vector
      // shape in such scenario.
      if (!InitShape.hasKnownStride())
        return getRandomVectorShape();

      TypeSizeInBytes =
          InitShape.isUniform()
              ? getTypeSizeInBytes(Init->getType()->getPointerElementType())
              : InitShape.getStrideVal();
    } else
      TypeSizeInBytes =
          getTypeSizeInBytes(Init->getType()->getPointerElementType());

    StepInt = TypeSizeInBytes * StepConst->getZExtValue();
  } else
    StepInt = StepConst->getZExtValue();

  if (StepInt == 1 || StepInt == -1)
    // Step could be of i32 type. That is why we do not use
    // getSequentialVectorShape().
    return VPVectorShape{VPVectorShape::Seq, const_cast<VPValue *>(Step)};

  return getStridedVectorShape(StepInt);
}

VPVectorShape
VPlanDivergenceAnalysis::computeVectorShape(const VPInstruction *I) {

  // Note: It is assumed at this point that shapes for all operands of I have
  // been defined, with the possible exception of phi nodes. Shape computation
  // for phi nodes can still happen without all operands being defined. See
  // computeImpl() and computeVectorShapeForPhiNode().

  if (isa<VPPHINode>(I))
    return computeVectorShapeForPhiNode(cast<VPPHINode>(I));

  VPVectorShape NewShape = VPVectorShape::getUndef();
  unsigned Opcode = I->getOpcode();

  const auto &ParentBB = *I->getParent();

  if (Instruction::isBinaryOp(Opcode))
    NewShape = computeVectorShapeForBinaryInst(I);
  else if (Instruction::isCast(Opcode))
    NewShape = computeVectorShapeForCastInst(I);
  else if (Opcode == Instruction::GetElementPtr ||
           Opcode == VPInstruction::Subscript)
    NewShape = computeVectorShapeForMemAddrInst(I);
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
  else if (Opcode == Instruction::Br) {
    VPValue *Cond = cast<VPBranchInst>(I)->getCondition();
    NewShape = !Cond ? getUniformVectorShape()
                     : getObservedShape(ParentBB, *Cond);
  } else if (Instruction::isUnaryOp(Opcode))
    NewShape = computeVectorShapeForUnaryInst(I);
  else if (Opcode == VPInstruction::Not || Opcode == VPInstruction::Pred)
    NewShape = getObservedShape(ParentBB, *(I->getOperand(0)));
  else if (Opcode == VPInstruction::Abs)
    // Shape computation of abs instruction is same as unary instruction for
    // now. Revisit in future if this needs to change.
    NewShape = computeVectorShapeForUnaryInst(I);
  else if (Opcode == VPInstruction::AllZeroCheck)
    NewShape = getUniformVectorShape();
  else if (Opcode == VPInstruction::InductionInit)
    NewShape = computeVectorShapeForInductionInit(cast<VPInductionInit>(I));
  else if (Opcode == VPInstruction::InductionInitStep)
    NewShape = getUniformVectorShape();
  else if (Opcode == VPInstruction::InductionFinal)
    NewShape = getUniformVectorShape();
  else if (Opcode == VPInstruction::ReductionInit)
    NewShape = getRandomVectorShape();
  else if (Opcode == VPInstruction::ReductionFinal)
    NewShape = getUniformVectorShape();
  else if (Opcode == VPInstruction::AllocatePrivate)
    NewShape = computeVectorShapeForAllocatePrivateInst(
        cast<const VPAllocatePrivate>(I));
  else if (Opcode == VPInstruction::OrigTripCountCalculation)
    NewShape = getUniformVectorShape();
  else if (Opcode == VPInstruction::VectorTripCountCalculation)
    NewShape = getUniformVectorShape();
  else if (Opcode == VPInstruction::HIRCopy)
    NewShape = getObservedShape(ParentBB, *(I->getOperand(0)));
  else if (Opcode >= VPInstruction::SMax && Opcode <= VPInstruction::FMin) {
    LLVM_DEBUG(dbgs() << "MIN/MAX DA is overly conservative: " << *I);
    // FIXME: Compute divergence based on the operands.
    NewShape = getRandomVectorShape();
  } else {
    LLVM_DEBUG(dbgs() << "Instruction not supported: " << *I);
    NewShape = getRandomVectorShape();
    assert(Opcode <= Instruction::OtherOpsEnd &&
           "VPlan-specific VPInstruction not supported in DA!");
  }

  return NewShape;
}

void VPlanDivergenceAnalysis::improveStrideUsingIR() {
  for (const VPBasicBlock &VPBB : *Plan) {
    for (auto &VPInst : VPBB) {
      // We are only attempting to improve the stride information of load/store
      // pointer operand.
      const VPValue *PtrOp = getLoadStorePointerOperand(&VPInst);
      if (!PtrOp)
        continue;

      // Nothing further to do if pointer shape is already marked not random.
      if (!getVectorShape(PtrOp).isRandom())
        continue;

      // HIR allows instructions like t1 = a[i] + b[i]. We can reliably
      // determine the underlying memory reference only for master instructions.
      // For stores, it is the LVAL ref. For loads, it is the RVAL ref.
      if (!VPInst.HIR.isMaster())
        continue;

      const loopopt::HLNode *HNode = VPInst.HIR.getUnderlyingNode();
      assert(HNode && "Unexpected null underlying node for master");

      int64_t Stride;
      if (getStrideUsingHIR(*HNode, Stride)) {
        LLVM_DEBUG(dbgs() << "Improved stride information for: " << VPInst);
        updateVectorShape(PtrOp, getStridedVectorShape(Stride));
      }
    }
  }
}
#endif // INTEL_CUSTOMIZATION

void VPlanDivergenceAnalysis::compute(VPlan *P, VPLoop *CandidateLoop,
                                      VPLoopInfo *VPLInfo,
                                      VPDominatorTree &VPDomTree,
                                      VPPostDominatorTree &VPPostDomTree,
                                      bool IsLCSSA) {

  assert(!DARecomputationDisabled &&
         "DA should not be computed for this Cloned VPlan!");

  Plan = P;
  RegionLoop = CandidateLoop;
  VPLI = VPLInfo;
  DT = &VPDomTree;
  PDT = &VPPostDomTree;
  IsLCSSAForm = IsLCSSA;
  SDA = std::make_unique<SyncDependenceAnalysis>(
      CandidateLoop ? CandidateLoop->getHeader() : P->getEntryBlock(),
      VPDomTree, VPPostDomTree, *VPLInfo);
  // Push everything to the worklist.
  ReversePostOrderTraversal<VPBasicBlock *> RPOT(Plan->getEntryBlock());
  for (auto *BB : RPOT) {
    // Sometimes the CondBit exists in a different block than the instruction
    // forming the CondBit. To properly propagate branch divergence, what we
    // really need to know is which Block uses the instruction as the CondBit.
    // This code piggybacks off the RPO traversal here to prevent further cost
    // in computeImpl() and keeps a map of the Instruction to the block(s)
    // containing the CondBit that computeImpl() will use to properly propagate
    // branch divergence. CondBits using VPExternalDefs not included here, but
    // we should be doing the right thing in computeImpl() by not propagating
    // divergence for them anyway. Please see vplan_da_condbit_separate_block.ll
    // as an example.
    if (VPInstruction *CondBitInst =
            dyn_cast_or_null<VPInstruction>(BB->getCondBit()))
      CondBit2BlockMap[CondBitInst].push_back(BB);
    for (auto &Inst : *BB)
      pushToWorklist(Inst);
  }

  // Compute the shapes of instructions - iterate until fixed point is reached.
  computeImpl();

#if INTEL_CUSTOMIZATION

  // We verify the shapes of the instructions 'always' in the debug-build and if
  // the command-line switch is enabled.
  if (VPlanVerifyDA)
    verifyVectorShapes();

  improveStrideUsingIR();
#endif // INTEL_CUSTOMIZATION

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (DumpDA)
    print(dbgs(), RegionLoop);
  if (DumpPlanDA)
    print(dbgs());
#endif // !NDEBUG || LLVM_ENABLE_DUMP
}

// Recomputes the shapes of all the instructions in the \p Seeds set and
// triggers the recomputation of all dependent instructions.
// This function assumes that all the required information like VPlan,
// Dominator/Post-Dominator tree, etc. are unchanged from the previous
// invocation of the \p compute method.
void VPlanDivergenceAnalysis::recomputeShapes(
    SmallPtrSetImpl<VPInstruction *> &Seeds) {

  assert(!DARecomputationDisabled &&
         "DA should not be computed for this Cloned VPlan!");

  if (Seeds.empty())
    return;

  // Clear the Worklist.
  clearWorklist();

  // Compute the shapes of the VPAllocatePrivate seed-instructions and push
  // their users to the Worklist.
  for (auto *Priv : Seeds) {
    auto Shape = computeVectorShape(Priv);
    updateVectorShape(Priv, Shape);
    pushUsers(*Priv);
  }

  // Compute the shapes of instructions.
  computeImpl();

#if INTEL_CUSTOMIZATION

  // We verify the shapes of the instructions 'always' in the debug-build and if
  // the command-line switch is enabled.
  if (VPlanVerifyDA)
    verifyVectorShapes();

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (DumpDA)
    print(dbgs(), RegionLoop);
#endif // !NDEBUG || LLVM_ENABLE_DUMP
}

#endif // INTEL_CUSTOMIZATION

void VPlanDivergenceAnalysis::cloneVectorShapes(
    VPlan *ClonedVPlan, DenseMap<VPValue *, VPValue *> &OrigClonedValuesMap) {

  ClonedVPlan->getVPlanDA()->Plan = ClonedVPlan;

  for (const auto &Pair : OrigClonedValuesMap) {
    VPValue *OrigVal = Pair.first;
    VPValue *ClonedVal = Pair.second;

    if (isa<VPBasicBlock>(OrigVal))
      continue;

    VPVectorShape OrigShape = getVectorShape(OrigVal);
    VPVectorShape *NewClonedShape = OrigShape.clone();
    VPValue *OrigStride = OrigShape.getStride();
    auto It = OrigClonedValuesMap.find(OrigStride);
    VPValue *ClonedStride = nullptr;
    if (OrigStride != nullptr)
      ClonedStride =
          (It != OrigClonedValuesMap.end()) ? It->second : OrigStride;
    NewClonedShape->setStride(ClonedStride);
    ClonedVPlan->getVPlanDA()->updateVectorShape(ClonedVal, *NewClonedShape);
  }
}

// Note: community version contains a LoopDivergencePrinter class that creates
// a SyncDependenceAnalysis object and a LoopDivergenceAnalysis object. The
// constructor for the LoopDivergenceAnalysis object then calls compute() to
// begin DA execution. For VPlan, all of this is done as part of HCFG
// construction. Thus, those classes are not included in this file.
#endif //INTEL_COLLAB
