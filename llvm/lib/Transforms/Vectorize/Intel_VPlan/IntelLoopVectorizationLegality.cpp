//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   IntelLoopVectorizationLegality.cpp -- LLVM IR loop vectorization legality
//   analysis.
//
//
//===----------------------------------------------------------------------===//

#include "IntelLoopVectorizationLegality.h"
#include "IntelVPlan.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool>
    UseSimdChannels("use-simd-channels", cl::init(true), cl::Hidden,
                    cl::desc("use simd versions of read/write pipe functions"));
extern cl::opt<bool> EnableVPValueCodegen;

#define DEBUG_TYPE "vpo-ir-loop-vectorize-legality"

/// The function collects Load and Store instruction that access the
/// reduction variable \p RedVarPtr.
static void collectAllRelevantUsers(Value *RedVarPtr,
                                    SmallVectorImpl<Value *> &Users) {
  for (auto U : RedVarPtr->users()) {
    if (isa<LoadInst>(U) || isa<StoreInst>(U))
      Users.push_back(U);
    else if (isa<BitCastInst>(U)) {
      Value *Ptr = getPtrThruCast<BitCastInst>(RedVarPtr);
      if (Ptr && Ptr != RedVarPtr)
        for (auto U : Ptr->users())
          if (isa<LoadInst>(U) || isa<StoreInst>(U))
            Users.push_back(U);
    }
  }
}

static bool checkCombinerOp(Value *CombinerV,
                            RecurrenceDescriptor::RecurrenceKind Kind) {
  switch (Kind) {
  case RecurrenceDescriptor::RK_FloatAdd:
    return isa<Instruction>(CombinerV) &&
           (cast<Instruction>(CombinerV)->getOpcode() == Instruction::FAdd ||
            cast<Instruction>(CombinerV)->getOpcode() == Instruction::FSub);
  case RecurrenceDescriptor::RK_IntegerAdd:
    return isa<Instruction>(CombinerV) &&
           (cast<Instruction>(CombinerV)->getOpcode() == Instruction::Add ||
            cast<Instruction>(CombinerV)->getOpcode() == Instruction::Sub);
  case RecurrenceDescriptor::RK_IntegerMult:
    return isa<Instruction>(CombinerV) &&
           cast<Instruction>(CombinerV)->getOpcode() == Instruction::Mul;
  case RecurrenceDescriptor::RK_FloatMult:
    return isa<Instruction>(CombinerV) &&
           cast<Instruction>(CombinerV)->getOpcode() == Instruction::FMul;
  case RecurrenceDescriptor::RK_IntegerAnd:
    return isa<Instruction>(CombinerV) &&
           cast<Instruction>(CombinerV)->getOpcode() == Instruction::And;
  case RecurrenceDescriptor::RK_IntegerOr:
    return isa<Instruction>(CombinerV) &&
           cast<Instruction>(CombinerV)->getOpcode() == Instruction::Or;
  case RecurrenceDescriptor::RK_IntegerXor:
    return isa<Instruction>(CombinerV) &&
           cast<Instruction>(CombinerV)->getOpcode() == Instruction::Xor;
  default:
    break;
  }
  return false;
}

static bool isSupportedInstructionType(Type *Ty) {
  return !Ty->isVectorTy() || Ty->getVectorElementType()->isSingleValueType();
}

/// \brief Check that the instruction has outside loop users and is not an
/// identified reduction variable.
static bool hasOutsideLoopUser(const Loop *TheLoop, Instruction *Inst,
                               SmallPtrSetImpl<Value *> &AllowedExit) {
  // Reduction and Induction instructions are allowed to have exit users. All
  // other instructions must not have external users.
  if (!AllowedExit.count(Inst))
    // Check that all of the users of the loop are inside the BB.
    for (User *U : Inst->users()) {
      Instruction *UI = cast<Instruction>(U);
      // This user may be a reduction exit value.
      if (!TheLoop->contains(UI)) {
        LLVM_DEBUG(dbgs() << "LV: Found an outside user for : " << *UI << '\n');
        return true;
      }
    }
  return false;
}

static bool isUsedInReductionScheme(
    PHINode *Phi,
    VPOVectorizationLegality::ExplicitReductionList &ReductionPhis) {
  return std::any_of(Phi->users().begin(), Phi->users().end(), [&](User *U) {
    return isa<PHINode>(U) && ReductionPhis.count(cast<PHINode>(U));
  });
}

static Type *convertPointerToIntegerType(const DataLayout &DL, Type *Ty) {
  if (Ty->isPointerTy())
    return DL.getIntPtrType(Ty);

  // It is possible that char's or short's overflow when we ask for the loop's
  // trip count, work around this by changing the type size.
  if (Ty->getScalarSizeInBits() < 32)
    return Type::getInt32Ty(Ty->getContext());

  return Ty;
}

static Type *getWiderType(const DataLayout &DL, Type *Ty0, Type *Ty1) {
  Ty0 = convertPointerToIntegerType(DL, Ty0);
  Ty1 = convertPointerToIntegerType(DL, Ty1);
  if (Ty0->getScalarSizeInBits() > Ty1->getScalarSizeInBits())
    return Ty0;
  return Ty1;
}

/// Analyze reduction pattern for variable \p RedVarPtr and return true if we
/// have Phi nodes inside. If yes, return the Phi node in \p LoopHeaderPhiNode
/// and the initializer in \p StartV.
bool VPOVectorizationLegality::doesReductionUsePhiNodes(
    Value *RedVarPtr, PHINode *&LoopHeaderPhiNode, Value *&StartV) {
  auto usedInOnlyOnePhiNode = [](Value *V) {
    PHINode *Phi = nullptr;
    for (auto U : V->users())
      if (isa<PHINode>(U)) {
        if (Phi) // More than one Phi node
          return (PHINode *)nullptr;
        Phi = cast<PHINode>(U);
      }
    return Phi;
  };
  SmallVector<Value *, 4> Users;
  collectAllRelevantUsers(RedVarPtr, Users);
  for (auto U : Users)
    if (auto LI = dyn_cast<LoadInst>(U))
      if (TheLoop->isLoopInvariant(LI)) { // Scenario (1)
        LoopHeaderPhiNode = usedInOnlyOnePhiNode(U);
        if (LoopHeaderPhiNode &&
            LoopHeaderPhiNode->getParent() == TheLoop->getHeader())
          StartV = LI;
      }
  return (StartV && LoopHeaderPhiNode);
}

/// Return true if the reduction variable \p RedVarPtr is stored inside the
/// loop.
bool VPOVectorizationLegality::isReductionVarStoredInsideTheLoop(
    Value *RedVarPtr) {
  SmallVector<Value *, 4> Users;
  collectAllRelevantUsers(RedVarPtr, Users);
  // I assume that one load or one store being found inside loop is enough
  // to say that we have them both. Since the reduction is explicit, deep
  // analysis for a possible inconsistency is not required.
  for (auto U : Users) {
    if (auto LI = dyn_cast<LoadInst>(U))
      if (!TheLoop->isLoopInvariant(LI))
        return true;
    if (auto SI = dyn_cast<StoreInst>(U))
      if (!TheLoop->isLoopInvariant(SI))
        return true;
  }
  return false;
}

void VPOVectorizationLegality::parseMinMaxReduction(
    AllocaInst *RedVarPtr, RecurrenceDescriptor::RecurrenceKind Kind,
    RecurrenceDescriptor::MinMaxRecurrenceKind Mrk) {

  // Analyzing 2 possible scenarios:
  // (1)
  //  for.body:
  //  **REDUCTION PHI** -
  //  %LoopHeaderPhiNode = phi i32[%.pre, %PreHeader], [%MinMaxResultPhi, %for.inc]
  //  %cmp1 = icmp sgt i32 %LoopHeaderPhiNode, %Val
  //  br i1 %cmp1, label %if.then, label %for.inc
  //
  //  if.then:
  //   STORE i32 %Val, i32* %min, align 4
  //   br label %for.inc
  //
  //  for.inc:
  //   % MinMaxResultPhi = PHI i32[%Val, %if.then], [%Tmp, %for.body]
  //   ..
  //   br i1 %exitcond, label %for.end, label %for.body

  // (2)
  //
  //  for.body:
  //  ** NO REDUCTION PHI **
  //  %Current = LOAD i32, i32* %Min, align 4
  //  %cmp1 = icmp sgt i32 %Val, %Current
  //  br i1 %cmp1, label %if.then, label %for.inc
  //  if.then:
  //   STORE i32 %Val, i32* %min, align 4
  //   br label %for.inc
  //
  //  for.inc:
  //   NO PHI
  //   ..
  //   br i1 %exitcond, label %for.end, label %for.body

  PHINode *LoopHeaderPhiNode = nullptr;
  Instruction *MinMaxResultPhi = nullptr;
  Value *StartV = nullptr;
  if (doesReductionUsePhiNodes(RedVarPtr, LoopHeaderPhiNode, StartV)) {
    for (auto PnUser : LoopHeaderPhiNode->users()) {
      if (TheLoop->isLoopInvariant(PnUser))
        continue;
      if (auto Phi = dyn_cast<PHINode>(PnUser))
        MinMaxResultPhi = Phi;
      else if (auto Select = dyn_cast<SelectInst>(PnUser))
        MinMaxResultPhi = Select;
      if (MinMaxResultPhi != nullptr)
        break;
    }
    SmallPtrSet<Instruction *, 4> CastInsts;
    FastMathFlags FMF = FastMathFlags::getFast();
    RecurrenceDescriptor RD(StartV, MinMaxResultPhi, Kind, FMF, Mrk, nullptr,
                            StartV->getType(), true, CastInsts);
    ExplicitReductions[LoopHeaderPhiNode] = {RD, RedVarPtr};
  }
  InMemoryReductions[RedVarPtr] = {Kind, Mrk};
}

void VPOVectorizationLegality::parseBinOpReduction(
    AllocaInst *RedVarPtr, RecurrenceDescriptor::RecurrenceKind Kind) {

  // Analyzing 2 possible scenarios:
  // (1) -- Reduction Phi nodes, the new value is in reg
  // StartV = Load
  //  ** Inside loop body **
  //  **REDUCTION PHI** -
  // Current = phi (StartV, NewVal)
  // %NewVal = add nsw i32 %NextVal, %Current
  // eof loop
  // use %NewVal
  //
  // (2) -- The new value is always in memory
  // ** Inside loop body **
  // %Current = LOAD i32, i32* %Sum, align 4
  //  %NewVal = add nsw i32 %NextVal, %Current
  //  STORE i32 %NewVal, i32* %Sum, align 4
  // eof loop
  // load i32* %Sum

  Value *StartV = nullptr;
  PHINode *ReductionPhi = nullptr;
  bool UsePhi = false;
  bool UseMemory = false;
  if ((UsePhi = doesReductionUsePhiNodes(RedVarPtr, ReductionPhi, StartV))) {
    Value *CombinerV = (ReductionPhi->getIncomingValue(0) == StartV)
                           ? ReductionPhi->getIncomingValue(1)
                           : ReductionPhi->getIncomingValue(0);
    if (!checkCombinerOp(CombinerV, Kind)) {
      LLVM_DEBUG(dbgs() << "LV: Combiner op does not match reduction type ");
      return;
    }
    Instruction *Combiner = cast<Instruction>(CombinerV);
    SmallPtrSet<Instruction *, 4> CastInsts;
    FastMathFlags FMF = FastMathFlags::getFast();
    RecurrenceDescriptor RD(StartV, Combiner, Kind, FMF,
                            RecurrenceDescriptor::MRK_Invalid, nullptr,
                            ReductionPhi->getType(), true, CastInsts);
    ExplicitReductions[ReductionPhi] = {RD, cast<AllocaInst>(RedVarPtr)};
  }
  if ((UseMemory = isReductionVarStoredInsideTheLoop(RedVarPtr)))
    InMemoryReductions[RedVarPtr] = {Kind, RecurrenceDescriptor::MRK_Invalid};

  if (!UsePhi && !UseMemory)
    LLVM_DEBUG(dbgs() << "LV: Explicit reduction pattern is not recognized ");
}

void VPOVectorizationLegality::parseExplicitReduction(
    Value *RedVarPtr, RecurrenceDescriptor::RecurrenceKind Kind,
    RecurrenceDescriptor::MinMaxRecurrenceKind Mrk) {
  assert(isa<AllocaInst>(RedVarPtr) &&
         "Expected Alloca instruction as a pointer to reduction variable");

  if (Mrk != RecurrenceDescriptor::MRK_Invalid)
    return parseMinMaxReduction(cast<AllocaInst>(RedVarPtr), Kind, Mrk);

  return parseBinOpReduction(cast<AllocaInst>(RedVarPtr), Kind);
}

bool VPOVectorizationLegality::isExplicitReductionPhi(PHINode *Phi) {
  return ExplicitReductions.count(Phi);
}

void VPOVectorizationLegality::addReductionMult(Value *V) {
  if (V->getType()->getPointerElementType()->isIntegerTy())
    parseExplicitReduction(V, RecurrenceDescriptor::RK_IntegerMult);
  else
    parseExplicitReduction(V, RecurrenceDescriptor::RK_FloatMult);
}

void VPOVectorizationLegality::addReductionAdd(Value *V) {
  if (V->getType()->getPointerElementType()->isIntegerTy())
    parseExplicitReduction(V, RecurrenceDescriptor::RK_IntegerAdd);
  else
    parseExplicitReduction(V, RecurrenceDescriptor::RK_FloatAdd);
}

void VPOVectorizationLegality::addReductionMin(Value *V, bool IsSigned) {
  if (V->getType()->getPointerElementType()->isIntegerTy()) {
    RecurrenceDescriptor::MinMaxRecurrenceKind Mrk =
        IsSigned ? RecurrenceDescriptor::MRK_SIntMin
                 : RecurrenceDescriptor::MRK_UIntMin;
    parseExplicitReduction(V, RecurrenceDescriptor::RK_IntegerMinMax, Mrk);
  } else
    parseExplicitReduction(V, RecurrenceDescriptor::RK_FloatMinMax,
                           RecurrenceDescriptor::MRK_FloatMin);
}

void VPOVectorizationLegality::addReductionMax(Value *V, bool IsSigned) {
  if (V->getType()->getPointerElementType()->isIntegerTy()) {
    RecurrenceDescriptor::MinMaxRecurrenceKind Mrk =
        IsSigned ? RecurrenceDescriptor::MRK_SIntMax
                 : RecurrenceDescriptor::MRK_UIntMax;
    parseExplicitReduction(V, RecurrenceDescriptor::RK_IntegerMinMax, Mrk);
  } else
    parseExplicitReduction(V, RecurrenceDescriptor::RK_FloatMinMax,
                           RecurrenceDescriptor::MRK_FloatMax);
}

bool VPOVectorizationLegality::canVectorize() {

  if (TheLoop->getNumBackEdges() != 1 || !TheLoop->getExitingBlock()) {
    LLVM_DEBUG(dbgs() << "loop control flow is not understood by vectorizer");
    return false;
  }
  // We only handle bottom-tested loops, i.e. loop in which the condition is
  // checked at the end of each iteration. With that we can assume that all
  // instructions in the loop are executed the same number of times.
  if (TheLoop->getExitingBlock() != TheLoop->getLoopLatch()) {
    LLVM_DEBUG(dbgs() << "loop control flow is not understood by vectorizer");
    return false;
  }
  // ScalarEvolution needs to be able to find the exit count.
  const SCEV *ExitCount = PSE.getBackedgeTakenCount();
  if (ExitCount == PSE.getSE()->getCouldNotCompute()) {
    LLVM_DEBUG(dbgs() << "LV: SCEV could not compute the loop exit count.\n");
    return false;
  }

  BasicBlock *Header = TheLoop->getHeader();
  // For each block in the loop.
  for (BasicBlock *BB : TheLoop->blocks()) {
    // Scan the instructions in the block and look for hazards.
    for (Instruction &I : *BB) {
      if (!isSupportedInstructionType(I.getType()))
        return false;
      if (auto *Phi = dyn_cast<PHINode>(&I)) {

        // If this PHINode is not in the header block, then we know that we
        // can convert it to select during if-conversion. No need to check if
        // the PHIs in this block are induction or reduction variables.
        if (BB != Header) {
          // Check that this instruction has no outside users or is an
          // identified reduction value with an outside user.
          if (!hasOutsideLoopUser(TheLoop, Phi, AllowedExit))
            continue;
          if (isUsedInReductionScheme(Phi, ExplicitReductions))
            continue;
          LLVM_DEBUG(dbgs() << "LV: PHI value could not be identified as"
                            << " an induction or reduction \n");
          return false;
        }

        // We only allow if-converted PHIs with exactly two incoming values.
        if (Phi->getNumIncomingValues() != 2) {
          LLVM_DEBUG(dbgs() << "LV: Found an invalid PHI.\n");
          return false;
        }

        if (isExplicitReductionPhi(Phi)) {
          if (EnableVPValueCodegen && !VPlanUseVPEntityInstructions) {
            LLVM_DEBUG(dbgs() << "VPVALCG: Not handling reductions without "
                                 "VPLoopEntities.\n");
            return false;
          }

          continue;
        }

        RecurrenceDescriptor RedDes;
        if (RecurrenceDescriptor::isReductionPHI(Phi, TheLoop, RedDes)) {
          if (EnableVPValueCodegen && !VPlanUseVPEntityInstructions) {
            LLVM_DEBUG(dbgs() << "VPVALCG: Not handling reductions without "
                                 "VPLoopEntities.\n");
            return false;
          }

          AllowedExit.insert(RedDes.getLoopExitInstr());
          Reductions[Phi] = RedDes;
          continue;
        }

        InductionDescriptor ID;
        if (InductionDescriptor::isInductionPHI(Phi, TheLoop, PSE, ID)) {
          addInductionPhi(Phi, ID, AllowedExit);
          continue;
        }

        LLVM_DEBUG(dbgs() << "LV: Found an unidentified PHI." << *Phi << "\n");
        return false;
      } // end of PHI handling

      // Check for handled shuffles
      if (auto ShufInst = dyn_cast<ShuffleVectorInst>(&I)) {
        if (getSplatValue(ShufInst) ||
            isa<ConstantAggregateZero>(ShufInst->getMask()))
          continue;

        LLVM_DEBUG(dbgs() << "LV: Unsupported shufflevector instruction."
                          << *ShufInst << "\n");
        return false;
      }

      // Bail out if we need to scalarize the read/write pipe OpenCL calls. We
      // have to do this because there are no users of these calls directly
      // since the results are written through a ptr argument. Thus, the
      // vectorizer is unable to correctly materialize the necessary scalars
      // into a vector through the VectorLoopValueMap. See
      // getOrCreateVectorValue().
      if (auto Call = dyn_cast<CallInst>(&I)) {
        Function *F = Call->getCalledFunction();
        if (!F)
          continue;

        if ((isOpenCLReadChannel(F->getName()) ||
             isOpenCLWriteChannel(F->getName())) &&
            !UseSimdChannels)
          return false;
      }
    }
  }
  if (!Induction && Inductions.empty()) {
    LLVM_DEBUG(dbgs() << "LV: Did not find one integer induction var.\n");
    return false;
  }

  collectLoopUniformsForAnyVF();
  return true;
}

void VPOVectorizationLegality::addInductionPhi(
    PHINode *Phi, const InductionDescriptor &ID,
    SmallPtrSetImpl<Value *> &AllowedExit) {

  Inductions[Phi] = ID;

  Type *PhiTy = Phi->getType();
  const DataLayout &DL = Phi->getModule()->getDataLayout();

  // Get the widest type.
  if (!PhiTy->isFloatingPointTy()) {
    if (!WidestIndTy)
      WidestIndTy = convertPointerToIntegerType(DL, PhiTy);
    else
      WidestIndTy = getWiderType(DL, PhiTy, WidestIndTy);
  }

  // Int inductions are special because we only allow one IV.
  if (ID.getKind() == InductionDescriptor::IK_IntInduction &&
      ID.getConstIntStepValue() && ID.getConstIntStepValue()->isOne() &&
      isa<Constant>(ID.getStartValue()) &&
      cast<Constant>(ID.getStartValue())->isNullValue()) {

    // Use the phi node with the widest type as induction. Use the last
    // one if there are multiple (no good reason for doing this other
    // than it is expedient). We've checked that it begins at zero and
    // steps by one, so this is a canonical induction variable.
    if (!Induction || PhiTy == WidestIndTy)
      Induction = Phi;
  }

  // Both the PHI node itself, and the "post-increment" value feeding
  // back into the PHI node may have external users.
  AllowedExit.insert(Phi);
  AllowedExit.insert(Phi->getIncomingValueForBlock(TheLoop->getLoopLatch()));

  LLVM_DEBUG(dbgs() << "LV: Found an induction variable.\n");
  return;
}

bool VPOVectorizationLegality::isLoopInvariant(Value *V) {
  // Each lane gets its own copy of the private value
  if (isLoopPrivate(V))
    return false;

  return LoopInvariants.count(V);
}

bool VPOVectorizationLegality::isLoopPrivate(Value *V) const {
  return Privates.count(getPtrThruCast<BitCastInst>(V)) ||
         isInMemoryReduction(V);
}

bool VPOVectorizationLegality::isLoopPrivateAggregate(Value *V) const {
  V = getPtrThruCast<BitCastInst>(V);
  V = getPtrThruCast<AddrSpaceCastInst>(V);
  if (isLoopPrivate(V)) {
    Type *PointeeTy = cast<PointerType>(V->getType())->getPointerElementType();
    return PointeeTy->isVectorTy() || PointeeTy->isAggregateType();
  }
  return false;
}

bool VPOVectorizationLegality::isInMemoryReduction(Value *V) const {
  V = getPtrThruCast<BitCastInst>(V);
  return isa<AllocaInst>(V) && InMemoryReductions.count(cast<AllocaInst>(V));
}

bool VPOVectorizationLegality::isLastPrivate(Value *V) const {
  return LastPrivates.count(getPtrThruCast<BitCastInst>(V)) != 0;
}

bool VPOVectorizationLegality::isCondLastPrivate(Value *V) const {
  return CondLastPrivates.count(getPtrThruCast<BitCastInst>(V));
}

bool VPOVectorizationLegality::isLinear(Value *Val, int *Step) {
  auto PtrThruBitCast = getPtrThruCast<BitCastInst>(Val);
  if (Linears.count(PtrThruBitCast)) {
    if (Step)
      *Step = Linears[PtrThruBitCast];
    return true;
  }

  return false;
}

bool VPOVectorizationLegality::isUnitStepLinear(Value *Val, int *Step,
                                                Value **NewScal) {
  if (UnitStepLinears.count(Val)) {
    auto NewValStep = UnitStepLinears[Val];
    if (Step)
      *Step = NewValStep.second;
    if (NewScal)
      *NewScal = NewValStep.first;

    return true;
  }

  return false;
}

int VPOVectorizationLegality::isConsecutivePtr(Value *Ptr) {
  // An in-memory loop private is handled according to the following rules based
  // on assumption of AOS-layout of variables,
  // ScalarTy --> <VF x ScalarTy> , Stride = 1
  // [NElts x Ty] --> [VF x [NElts x Ty]] , Stride = NElts
  // {Ty1, [NElts x Ty2]} --> [VF x {Ty1, [NElts x Ty2]}] , Stride = unknown

  if (isLoopPrivateAggregate(Ptr))
    return 0;
  else if (isLoopPrivate(Ptr))
    return 1;

  int Stride = 0;
  if (PtrStrides.count(Ptr))
    Stride = PtrStrides[Ptr];

  if (Stride == 1 || Stride == -1)
    return Stride;

  // See if we can use linear information to check if we have a consecutive
  // pointer
  auto *PtrTy = cast<PointerType>(Ptr->getType());
  if (PtrTy->getElementType()->isAggregateType())
    return 0;

  // We are looking for a GEP whose last operand is a unit step linear item
  if (!isa<GetElementPtrInst>(Ptr))
    return 0;

  auto Gep = cast<GetElementPtrInst>(Ptr);
  unsigned NumOperands = Gep->getNumOperands();
  Value *LastGepOper = Gep->getOperand(NumOperands - 1);

  // If the last operand is not a unit stride linear bail out
  int LinStep = 0;

  if (!isUnitStepLinear(LastGepOper, &LinStep))
    return 0;

  // If any of the Gep operands other than the last one is not loop invariant -
  // bail out
  for (unsigned Index = 0; Index < NumOperands - 1; ++Index) {
    auto Op = Gep->getOperand(Index);
    if (!isLoopInvariant(Op))
      return 0;
  }

  return LinStep;
}

bool VPOVectorizationLegality::isInductionVariable(const Value *V) {
  Value *In0 = const_cast<Value *>(V);
  PHINode *PN = dyn_cast_or_null<PHINode>(In0);
  if (!PN)
    return false;
  return Inductions.count(PN);
}

void VPOVectorizationLegality::collectLoopUniformsForAnyVF() {
  // We now know that the loop is vectorizable!
  // Collect instructions inside the loop that will remain uniform after
  // vectorization.

  // Global values, params and instructions outside of current loop are out of
  // scope.
  auto isOutOfScope = [&](Value *V) -> bool {
    Instruction *I = dyn_cast<Instruction>(V);
    return (!I || !TheLoop->contains(I));
  };

  SetVector<Instruction *> Worklist;
  BasicBlock *Latch = TheLoop->getLoopLatch();

  // Start with the conditional branch. If the branch condition is an
  // instruction contained in the loop that is only used by the branch, it is
  // uniform.
  auto *Cmp = dyn_cast<Instruction>(Latch->getTerminator()->getOperand(0));
  if (Cmp && TheLoop->contains(Cmp) && Cmp->hasOneUse()) {
    Worklist.insert(Cmp);
    LLVM_DEBUG(dbgs() << "LV: Found uniform instruction: " << *Cmp << "\n");
  }

  for (auto *BB : TheLoop->blocks())
    for (auto &I : *BB) {
#if 0
      auto isInnerLoopInduction = [&](PHINode *Phi, const Loop *&InnerL) -> bool {
        if (isInductionVariable(Phi))
          return false;

        if (!PSE.getSE()->isSCEVable(Phi->getType()))
          return false;

        const SCEV *PhiScev = PSE.getSCEV(Phi);
        if (auto AR = dyn_cast<SCEVAddRecExpr>(PhiScev)) {
          InnerL = AR->getLoop();
          return (InnerL != TheLoop && TheLoop->contains(InnerL));
        }
        return false;
      };

      // The code below incorrectly assumes that all inner loop inductions are
      // uniform. This is not true for divergent inner loops. This issue will
      // be fixed when we move to full VPValue based code generation combined
      // with use of DA analysis information.
      if (auto Phi = dyn_cast<PHINode>(&I)) {
        const Loop *InnerLoop = nullptr;
        if (isInnerLoopInduction(Phi, InnerLoop)) {
          Worklist.insert(Phi);
          LLVM_DEBUG(dbgs()
                     << "LV: Found uniform instruction: " << *Phi << "\n");
          BasicBlock *InnerLoopLatch = InnerLoop->getLoopLatch();
          BranchInst *Br = cast<BranchInst>(InnerLoopLatch->getTerminator());
          Worklist.insert(Br);
          auto *Cmp = dyn_cast<Instruction>(Br->getOperand(0));
          if (Cmp && InnerLoop->contains(Cmp) && Cmp->hasOneUse()) {
            Worklist.insert(Cmp);
            LLVM_DEBUG(dbgs()
                       << "LV: Found uniform instruction: " << *Cmp << "\n");
          }
          auto *IndUpdate =
            cast<Instruction>(Phi->getIncomingValueForBlock(InnerLoopLatch));
          LLVM_DEBUG(dbgs() << "LV: Found uniform instruction: " << *IndUpdate
                            << "\n");
          Worklist.insert(IndUpdate);
        }
      }
#endif
      if (auto Br = dyn_cast<BranchInst>(&I)) {
        if (!Br->isConditional())
          continue;
        Value *Cond = Br->getCondition();
        if (TheLoop->isLoopInvariant(Cond))
          Worklist.insert(Br);
      } else if (isa<GetElementPtrInst>(&I)) {
        // Collect invariant GEP operands
        for (Value *Op : I.operands()) {
          if (PSE.getSE()->isLoopInvariant(PSE.getSCEV(Op), TheLoop))
            LoopInvariants.insert(Op);
        }
      } else if (isa<SelectInst>(&I)) {
        // Collect invariant select conditions
        Value *Cond = I.getOperand(0);
        if (PSE.getSE()->isLoopInvariant(PSE.getSCEV(Cond), TheLoop))
          LoopInvariants.insert(Cond);
      }

      // Load with loop invariant pointer
      Value *Ptr = getLoadStorePointerOperand(&I);
      if (Ptr && !isLoopPrivate(Ptr)) {
        // Collect pointer stride information
        if (!PtrStrides.count(Ptr)) {
          const ValueToValueMap &Strides = ValueToValueMap();
          int Stride = getPtrStride(PSE, Ptr, TheLoop, Strides, false);
          PtrStrides[Ptr] = Stride;
        }
        const SCEV *PtrScevAtTheLoopScope =
            PSE.getSE()->getSCEVAtScope(Ptr, TheLoop);
        if (PSE.getSE()->isLoopInvariant(PtrScevAtTheLoopScope, TheLoop) &&
            isa<LoadInst>(&I))
          Worklist.insert(&I);
      }
    }
  // Expand Worklist in topological order: whenever a new instruction
  // is added , its users should be either already inside Worklist, or
  // out of scope. It ensures a uniform instruction will only be used
  // by uniform instructions or out of scope instructions.
  unsigned idx = 0;
  while (idx != Worklist.size()) {
    Instruction *I = Worklist[idx++];

    for (auto OV : I->operand_values()) {
      if (auto *OI = dyn_cast<Instruction>(OV)) {
        if (all_of(OI->users(), [&](User *U) -> bool {
              return isOutOfScope(U) || Worklist.count(cast<Instruction>(U));
            })) {
          Worklist.insert(OI);
          LLVM_DEBUG(dbgs()
                     << "LV: Found uniform instruction: " << *OI << "\n");
          if (all_of(OV->users(), [&](User *U) -> bool {
                return isOutOfScope(U) || Worklist.count(cast<Instruction>(U));
              })) {
            Worklist.insert(OI);
            LLVM_DEBUG(dbgs()
                       << "LV: Found uniform instruction: " << *OI << "\n");
          }
        }
      }
    }
  }

  UniformForAnyVF.insert(Worklist.begin(), Worklist.end());
}
