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
#include "IntelVPlanUtils.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"

#define DEBUG_TYPE "vpo-ir-loop-vectorize-legality"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool, true> ForceComplexTyReductionVecOpt(
    "vplan-force-complex-type-reduction-vectorization",
    cl::location(ForceComplexTyReductionVec), cl::Hidden,
    cl::desc("Force vectorization of reduction involving complex type."));

static cl::opt<bool, true> ForceInscanReductionVecOpt(
    "vplan-force-inscan-reduction-vectorization",
    cl::location(ForceInscanReductionVec), cl::Hidden,
    cl::desc("Force vectorization of inscan reduction."));

static cl::opt<bool>
    UseSimdChannels("use-simd-channels", cl::init(true), cl::Hidden,
                    cl::desc("use simd versions of read/write pipe functions"));

namespace llvm {
namespace vpo {
bool ForceComplexTyReductionVec = false;
bool ForceInscanReductionVec = false;
} // namespace vpo
} // namespace llvm

/// The function collects Load, Store, and non-intrinsic call instructions that
/// access the reduction variable \p RedVarPtr.
static void collectAllRelevantUsers(Value *RedVarPtr,
                                    SmallVectorImpl<Value *> &Users) {
  SmallVector<Value *, 4> WorkList;
  SmallSet<User *, 4> Visited;

  WorkList.push_back(RedVarPtr);
  while (!WorkList.empty()) {
    auto *CurrVal = WorkList.pop_back_val();

    for (auto U : CurrVal->users()) {
      if (isa<LoadInst>(U) || isa<StoreInst>(U)) {
        Users.push_back(U);
        continue;
      }
      if (auto CI = dyn_cast<CallInst>(U)) {
        if (!CI->getCalledFunction() || !CI->getCalledFunction()->isIntrinsic())
          Users.push_back(U);
        continue;
      }
      if (isa<BitCastInst>(U)) {
        WorkList.push_back(U);
        continue;
      }
      if (isa<AddrSpaceCastInst>(U))
        WorkList.push_back(U);
    }
  }
}

/// Return \p true if \p Ty or any of the member-type of \p Ty is a scalable
/// type.
static bool isOrHasScalableTy(Type *InTy) {
  SetVector<Type *> WL;
  DenseSet<Type *> AnalyzedTypes;
  WL.insert(InTy);

  while (!WL.empty()) {
    Type *Ty = WL.pop_back_val();

    if (AnalyzedTypes.count(Ty))
      continue;

    if (isa<ScalableVectorType>(Ty))
      return true;

    if (auto *CastTy = dyn_cast<PointerType>(Ty))
      if (!CastTy->isOpaque())
        WL.insert(CastTy->getPointerElementType());

    if (auto *CastTy = dyn_cast<VectorType>(Ty))
      WL.insert(CastTy->getElementType());

    if (auto *CastTy = dyn_cast<ArrayType>(Ty))
      WL.insert(CastTy->getElementType());

    if (auto *StructTy = dyn_cast<StructType>(Ty))
      for (auto *ElemTy : StructTy->elements())
        WL.insert(ElemTy);

    AnalyzedTypes.insert(Ty);
  }

  return false;
}

static bool isSupportedInstructionType(const Instruction &I) {
  Type *Ty = I.getType();

  if (isOrHasScalableTy(Ty)) {
    // For debug builds, assert that the incoming IR does not have scalable
    // vector type.
    assert(false && "VPlan does not support IR with ScalableVectorType.");
    // For release builds, just bail out.
    return false;
  }

  auto *VecTy = dyn_cast<VectorType>(Ty);
  if (!VecTy)
    return true;

  bool InstrOperandsAreNonScalableType =
      none_of(I.operands(),
              [](const Value *V) { return isOrHasScalableTy(V->getType()); });

  return VecTy->getElementType()->isSingleValueType() &&
         InstrOperandsAreNonScalableType;
}

/// Check that the instruction has outside loop users and is not an identified
/// reduction variable.
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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPOVectorizationLegality::dump(raw_ostream &OS) const {
  OS << "VPOLegality Descriptor Lists\n";
  OS << "\n\nVPOLegality PrivateList:\n";
  for (auto const &Pvt : Privates) {
    Pvt.second->print(OS);
  }
  OS << "\n\nVPOLegality UDRList:\n";
  for (auto const &UDR : UserDefinedReductions) {
    UDR->print(OS);
  }
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

/// Analyze reduction pattern for variable \p RedVarPtr and return true if we
/// have Phi nodes inside. If yes, return the Phi node in \p LoopHeaderPhiNode
/// and the initializer in \p StartV.
// Analyzed scenarios include:
// (1) -- Reduction Phi nodes, the new value is in reg
// StartV = Load
//  ** Inside loop body **
//  **REDUCTION PHI** -
// Current = phi (StartV, NewVal)
// %NewVal = add nsw i32 %NextVal, %Current
// eof loop
// use %NewVal
//
// (2) -- Reduction uses register, but also stores to memory inside loop.
// store i32 StartV, i32* %Sum
// ** Inside loop body **
// %Current = phi (StartV, NewVal)
// %NewVal = add i32 %Current, %NextVal
// store i32 %NewVal, i32* %Sum
// eof loop
// use %NewVal
bool VPOVectorizationLegality::doesReductionUsePhiNodes(
    Value *RedVarPtr, PHINode *&LoopHeaderPhiNode, Value *&StartV) {
  auto usedInOnlyOneHeaderPhiNode = [this](Value *V) {
    PHINode *Phi = nullptr;
    for (auto U : V->users())
      if (isa<PHINode>(U) &&
          cast<PHINode>(U)->getParent() == TheLoop->getHeader()) {
        if (Phi) // More than one Phi node
          return (PHINode *)nullptr;
        Phi = cast<PHINode>(U);
      }
    return Phi;
  };
  auto isReductionStartPhiNode = [this](PHINode *Phi, Instruction *UpdateInst) {
    // Reduction phi is expected to be in loop header.
    if (Phi->getParent() != TheLoop->getHeader())
      return false;

    // Header phis can have only two incoming values. For a reduction phi, one
    // is expected to be UpdateInst and the other a loop invariant
    // initialization value.
    assert(Phi->getNumIncomingValues() == 2 &&
           "Header Phis expected to have two incoming values.");
    Value *InVal0 = Phi->getIncomingValue(0);
    Value *InVal1 = Phi->getIncomingValue(1);

    if (UpdateInst && InVal0 != UpdateInst && InVal1 != UpdateInst)
      return false;

    if (!TheLoop->isLoopInvariant(InVal0) && !TheLoop->isLoopInvariant(InVal1))
      return false;

    // All conditions satisfied for reduction phi.
    return true;
  };
  SmallVector<Value *, 4> Users;
  collectAllRelevantUsers(RedVarPtr, Users);
  for (auto U : Users) {
    if (auto LI = dyn_cast<LoadInst>(U)) {
      if (TheLoop->isLoopInvariant(LI)) { // Scenario (1)
        LoopHeaderPhiNode = usedInOnlyOneHeaderPhiNode(U);
        if (LoopHeaderPhiNode &&
            isReductionStartPhiNode(LoopHeaderPhiNode,
                                    nullptr /*UpdateInst*/)) {
          StartV = LI;
          break;
        }
      }
      // Nothing more for scenario (1).
      continue;
    }
    if (auto SI = dyn_cast<StoreInst>(U)) {
      if (TheLoop->contains(SI->getParent())) { // Scenario (2)
        // The updated value of reduction is being stored inside loop, try to
        // check if reduction also has a PHI node to perform the reduction in
        // register.
        Value *UpdateV = SI->getOperand(0);
        if (!isa<Instruction>(UpdateV))
          continue;

        Instruction *UpdateInst = cast<Instruction>(UpdateV);
        for (auto Op : UpdateInst->operand_values()) {
          // TODO: Handle more complex cases like masked reductions which will
          // have multiple updating instructions. Try to templatize
          // ReductionDescr::tryToCompleteByVPlan from VPLoopAnalysis.
          if (!isa<PHINode>(Op))
            continue;

          PHINode *UpdatePhiOp = cast<PHINode>(Op);
          // We expect to have only one PHI for the reduction in loop header.
          // Confirm this by checking loop header PHI users of the UpdateInst.
          if (UpdatePhiOp == usedInOnlyOneHeaderPhiNode(UpdateInst) &&
              isReductionStartPhiNode(UpdatePhiOp, UpdateInst)) {
            LoopHeaderPhiNode = UpdatePhiOp;
            StartV = LoopHeaderPhiNode->getOperand(0) == UpdateV
                         ? LoopHeaderPhiNode->getOperand(1)
                         : LoopHeaderPhiNode->getOperand(0);
          }
        }
        if (StartV && LoopHeaderPhiNode)
          break;
      }
    }
  }
  return (StartV && LoopHeaderPhiNode);
}

bool VPOVectorizationLegality::isInMemoryReductionPattern(
    Value *RedVarPtr, Instruction *&CallOrStore) {
  SmallVector<Value *, 4> Users;
  CallOrStore = nullptr;
  CallInst *Call = nullptr;
  collectAllRelevantUsers(RedVarPtr, Users);
  for (auto U : Users) {
    if (auto SI = dyn_cast<StoreInst>(U))
      if (!TheLoop->isLoopInvariant(SI)) {
        CallOrStore = SI;
        return true;
      }
    if (auto CI = dyn_cast<CallInst>(U))
     Call = CI;
  }
  if (Call) {
    CallOrStore = Call;
    return true;
  }
  return false;
}

bool VPOVectorizationLegality::isEndDirective(Instruction *I) const {
  return VPOAnalysisUtils::isEndDirective(I) &&
         VPOAnalysisUtils::getDirectiveID(I) == DIR_OMP_SIMD;
}

// Utility to analyze all instructions between the SIMD clause and the loop to
// identify any aliasing variables to the explicit SIMD descriptors. We traverse
// the CFG backwards, starting from Loop pre-header to the BB where SIMD clause
// is found.
void VPOVectorizationLegality::collectPreLoopDescrAliases() {
  BasicBlock *LpPH = TheLoop->getLoopPreheader();

  if (!LpPH)
    return;

  for (auto *CurBB = LpPH; CurBB; CurBB = CurBB->getSinglePredecessor()) {
    for (auto &I : reverse((*CurBB))) {
      if (isEndDirective(&I))
        return;
      if (!isa<LoadInst>(&I))
        continue;
      LLVM_DEBUG(dbgs() << "VPOLegal: LoadInst: "; I.dump());
      Value *LoadPtrOp = cast<LoadInst>(&I)->getPointerOperand();
      if (Privates.count(LoadPtrOp)) {
        std::unique_ptr<PrivDescrTy> &Descr = Privates.find(LoadPtrOp)->second;

        LLVM_DEBUG(dbgs() << "Found an alias for a SIMD descriptor ref ";
                   LoadPtrOp->dump());
        Descr->addAlias(&I, std::make_unique<DescrValueTy>(&I));
      }
    }
  }
}

// Utility to analyze all instructions in loop post-exit to identify any
// aliasing variables to the explicit SIMD descriptor. We traverse CFG starting
// from loop exit BB to the BB where END.SIMD clause is found.
void VPOVectorizationLegality::collectPostExitLoopDescrAliases() {
  BasicBlock *LpEx = TheLoop->getExitBlock();

  if (!LpEx)
    return;

  for (auto *CurBB = LpEx; CurBB; CurBB = CurBB->getSingleSuccessor()) {
    for (auto &I : *CurBB) {
      if (isEndDirective(&I))
        return;
      auto *SI = dyn_cast<StoreInst>(&I);
      if (!SI)
        continue;
      LLVM_DEBUG(dbgs() << "VPOLegal: StoreInst: "; SI->dump());
      Value *StorePtrOp = SI->getPointerOperand();
      if (!Privates.count(StorePtrOp))
        continue;

      std::unique_ptr<PrivDescrTy> &Descr = Privates.find(StorePtrOp)->second;
      auto *StoreOp = dyn_cast<Instruction>(SI->getValueOperand());
      if (!StoreOp)
        continue;
      if (!TheLoop->contains(StoreOp)) {
        auto *Phi = dyn_cast<PHINode>(StoreOp);
        if (!Phi) // non-lcssa?
          continue;
        StoreOp = getLiveOutPhiOperand(Phi);
        if (!StoreOp)
          continue;
      }
      LLVM_DEBUG(dbgs() << "Found an alias for a SIMD descriptor ref ";
                 StoreOp->dump());
      Descr->addAlias(StoreOp, std::make_unique<DescrValueTy>(StoreOp));
    }
  }
}

template <typename LoopEntitiesRange>
bool VPOVectorizationLegality::isEntityAliasingSafe(
    const LoopEntitiesRange &LERange,
    function_ref<bool(const Instruction *)> IsAliasInRelevantScope) {
  for (auto *En : LERange) {
    SetVector<const Value *> WL;
    WL.insert(En);
    while (!WL.empty()) {
      auto *HeadI = WL.pop_back_val();
      for (auto *Use : HeadI->users()) {
        const auto *UseInst = cast<Instruction>(Use);

        // We only want to analyze the blocks between the region-entry and the
        // loop-block (typically just simd.loop.preheader). This means we won't
        // loop on cycle-causing PHIs.
        if (!IsAliasInRelevantScope(UseInst))
          continue;

        // If this is a store of private pointer or any of its alias to an
        // external memory, treat the loop as unsafe for vectorization and
        // return false.
        if (const auto *SI = dyn_cast<StoreInst>(UseInst))
          if (SI->getValueOperand() == HeadI)
            return false;
        if (isTrivialPointerAliasingInst(UseInst))
          WL.insert(UseInst);
      }
    }
  }
  return true;
}

// Check the safety of aliasing of loop-privates outside of the loop.
// We want to scan the block RegionEntry : loop-preheader and checks if we have
// a store of the  pointer to any memory locations. If that is the case, we
// treat this loop as unsafe for vectorization.
bool VPOVectorizationLegality::isAliasingSafe(DominatorTree &DT,
                                              const CallInst *RegionEntry) {
  // We would not have a RegionEntry in case of auto-vectorization or when we
  // are using -vplan-build-vect-candidates. In that scenario, we do not want
  // this check to be done and depend on the AA analysis for safety.
  if (!RegionEntry)
    return true;

  // Check that the aliasing instruction is present in one of the blocks between
  // the region-entry and the loop-header.
  auto IsInstInRelevantScope = [&](const Instruction *I) {
    return DT.dominates(RegionEntry, I) &&
           DT.dominates(I, TheLoop->getHeader());
  };

  return isEntityAliasingSafe(privateVals(), IsInstInRelevantScope) &&
         isEntityAliasingSafe(explicitReductionVals(), IsInstInRelevantScope) &&
         isEntityAliasingSafe(inMemoryReductionVals(), IsInstInRelevantScope) &&
         isEntityAliasingSafe(linearVals(), IsInstInRelevantScope);
}

void VPOVectorizationLegality::parseMinMaxReduction(Value *RedVarPtr,
                                                    RecurKind Kind) {

  // Analyzing some possible scenarios:
  // (1)
  //  for.body:
  //  **REDUCTION PHI** -
  //  %LoopHeaderPhiNode = phi i32[%.pre, %PreHeader], [%MinMaxResultInst, %for.inc]
  //  %cmp1 = icmp sgt i32 %LoopHeaderPhiNode, %Val
  //  br i1 %cmp1, label %if.then, label %for.inc
  //
  //  if.then:
  //   STORE i32 %Val, i32* %min, align 4
  //   br label %for.inc
  //
  //  for.inc:
  //   % MinMaxResultInst = PHI i32[%Val, %if.then], [%LoopHeaderPhiNode, %for.body]
  //   ..
  //   br i1 %exitcond, label %for.end, label %for.body

  // (2)
  //  for.body:
  //  **REDUCTION PHI** -
  //  %LoopHeaderPhiNode = phi i32 [%.pre, %PreHeader], [%MinMaxResultInst, %for.inc]
  //  %cmp1 = icmp sgt i32 %LoopHeaderPhiNode, %Val
  //  %MinMaxResultInst = select i1 %cmp1, i32 %Val, i32 %LoopHeaderPhiNode

  // (3)
  //  for.body:
  //  **REDUCTION PHI** -
  //  %LoopHeaderPhiNode = phi float [%.pre, %PreHeader], [%MinMaxResultInst, %for.inc]
  //  %MinMaxResultInst = call @llvm.minnum.f32(float %Val, float %LoopHeaderPhiNode)

  // (4)
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
  Instruction *MinMaxResultInst = nullptr;
  Value *StartV = nullptr;
  Instruction *ReductionUse = nullptr;
  if (doesReductionUsePhiNodes(RedVarPtr, LoopHeaderPhiNode, StartV)) {
    using namespace PatternMatch;
    auto It = find_if(LoopHeaderPhiNode->users(), [this](auto *U) {
      return !TheLoop->isLoopInvariant(U) &&
             (isa<PHINode>(U) || isa<SelectInst>(U) ||
              match(U, m_CombineOr(m_Intrinsic<Intrinsic::maxnum>(),
                                   m_Intrinsic<Intrinsic::minnum>())));
    });
    if (It != LoopHeaderPhiNode->user_end())
      MinMaxResultInst = cast<Instruction>(*It);

    SmallPtrSet<Instruction *, 4> CastInsts;
    FastMathFlags FMF = FastMathFlags::getFast();
    RecurrenceDescriptor RD(StartV, MinMaxResultInst,
                            nullptr /*IntermediateStore*/, Kind, FMF, nullptr,
                            StartV->getType(), true /*Signed*/,
                            false /*Ordered*/, CastInsts, -1U);
    ExplicitReductions[LoopHeaderPhiNode] = {RD, RedVarPtr,
                                             None /*InscanReductionKind*/};
  } else if (isInMemoryReductionPattern(RedVarPtr, ReductionUse))
    InMemoryReductions[RedVarPtr] =
      {Kind, None /*InscanReductionKind*/, ReductionUse};
}

void VPOVectorizationLegality::parseBinOpReduction(
    Value *RedVarPtr, RecurKind Kind,
    Optional<InscanReductionKind> InscanRedKind) {

  // Analyzing 3 possible scenarios:
  // (1) -- Reduction Phi nodes, the new value is in reg
  // StartV = Load
  //  ** Inside loop body **
  //  **REDUCTION PHI** -
  // Current = phi (StartV, NewVal)
  // %NewVal = add nsw i32 %NextVal, %Current
  // eof loop
  // use %NewVal
  //
  // (2) -- Reduction uses register, but also stores to memory inside loop.
  // store i32 StartV, i32* %Sum
  // ** Inside loop body **
  // %Current = phi (StartV, NewVal)
  // %NewVal = add i32 %Current, %NextVal
  // store i32 %NewVal, i32* %Sum
  // eof loop
  // use %NewVal
  //
  // (3) -- The new value is always in memory
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
  Instruction *ReductionUse = nullptr;
  if ((UsePhi = doesReductionUsePhiNodes(RedVarPtr, ReductionPhi, StartV))) {
    Value *CombinerV = (ReductionPhi->getIncomingValue(0) == StartV)
                           ? ReductionPhi->getIncomingValue(1)
                           : ReductionPhi->getIncomingValue(0);
    Instruction *Combiner = cast<Instruction>(CombinerV);
    SmallPtrSet<Instruction *, 4> CastInsts;
    FastMathFlags FMF = FastMathFlags::getFast();
    if (InscanRedKind) {
      UseMemory = isInMemoryReductionPattern(RedVarPtr, ReductionUse);
      // TODO: make this assert to be a proper bailout in prod.
      assert(UseMemory &&
             "Fully registerized inscan reduction is not supported!");
      (void)UseMemory;
    }
    RecurrenceDescriptor RD(StartV, Combiner, nullptr /*IntermediateStore*/,
                            Kind, FMF, nullptr, ReductionPhi->getType(),
                            true /*Signed*/, false /*Ordered*/, CastInsts, -1U);
    ExplicitReductions[ReductionPhi] = {RD, RedVarPtr, InscanRedKind};
  } else if ((UseMemory = isInMemoryReductionPattern(RedVarPtr, ReductionUse)))
    InMemoryReductions[RedVarPtr] = {Kind, InscanRedKind, ReductionUse};

  if (!UsePhi && !UseMemory)
    LLVM_DEBUG(dbgs() << "LV: Explicit reduction pattern is not recognized ");
}

void VPOVectorizationLegality::addReduction(
    Value *RedVarPtr, RecurKind Kind,
    Optional<InscanReductionKind> InscanRedKind) {
  assert(isa<PointerType>(RedVarPtr->getType()) &&
         "Expected reduction variable to be a pointer type");

  // TODO: Support min/max scan reductions as well.
  if (RecurrenceDescriptorData::isMinMaxRecurrenceKind(Kind)) {
    parseMinMaxReduction(RedVarPtr, Kind);
    return;
  }

  parseBinOpReduction(RedVarPtr, Kind, InscanRedKind);
}

bool VPOVectorizationLegality::isExplicitReductionPhi(PHINode *Phi) {
  return ExplicitReductions.count(Phi);
}

bool VPOVectorizationLegality::bailout(BailoutReason Code) {
  LLVM_DEBUG(dbgs() << getBailoutReasonStr(Code));
  return false;
}

bool VPOVectorizationLegality::canVectorize(DominatorTree &DT,
                                            const WRNVecLoopNode *WRLp) {
    IsSimdLoop = WRLp;

  // Import explicit data from WRLoop.
  // Decision about loop vectorization is based on this data.
  if (!EnterExplicitData(WRLp))
    return false;

  if (IsSimdLoop) {
    collectPreLoopDescrAliases();
    collectPostExitLoopDescrAliases();
  }

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

  // Check if aliasing of privates is safe outside of the loop.
  CallInst *RegionEntry =
      WRLp ? cast<CallInst>(WRLp->getEntryDirective()) : nullptr;

  if (!isAliasingSafe(DT, RegionEntry)) {
    LLVM_DEBUG(dbgs() << "LV: Safety of aliasing of privates outside of the "
                         "loop cannot be accertained. \n");
    return false;
  }

  BasicBlock *Header = TheLoop->getHeader();
  // For each block in the loop.
  for (BasicBlock *BB : TheLoop->blocks()) {
    // Scan the instructions in the block and look for hazards.
    for (Instruction &I : *BB) {
      if (!isSupportedInstructionType(I))
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

          if (any_of(Phi->users(), [&](User *U) {
                return isa<PHINode>(U) &&
                       ExplicitReductions.count(cast<PHINode>(U));
              }))
            // Used in reduction scheme.
            continue;

          if (checkAndAddAliasForSimdLastPrivate(Phi))
            continue;

          LLVM_DEBUG(dbgs() << "LV: PHI value could not be identified as"
                            << " an induction or reduction." << *Phi << "\n");
          return false;
        }

        // We only allow if-converted PHIs with exactly two incoming values.
        if (Phi->getNumIncomingValues() != 2) {
          LLVM_DEBUG(dbgs() << "LV: Found an invalid PHI.\n");
          return false;
        }

        if (isExplicitReductionPhi(Phi))
          continue;

        RecurrenceDescriptor RedDes;
        if (RecurrenceDescriptor::isReductionPHI(Phi, TheLoop, RedDes)) {
          AllowedExit.insert(RedDes.getLoopExitInstr());
          Reductions[Phi] = RedDes;
          continue;
        }

        InductionDescriptor ID;
        if (InductionDescriptor::isInductionPHI(Phi, TheLoop, PSE, ID)) {
          addInductionPhi(Phi, ID, AllowedExit);
          continue;
        }

        if (checkAndAddAliasForSimdLastPrivate(Phi))
          continue;

        LLVM_DEBUG(dbgs() << "LV: Found an unidentified PHI." << *Phi << "\n");
        return false;
      } // end of PHI handling

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

        if (vpo::VPOAnalysisUtils::isBeginDirective(Call)) {
          // Memory motion guarding directives are inserted for UDRs and will be
          // removed by VPlan framework.
          if (vpo::VPOAnalysisUtils::getDirectiveID(Call) ==
              DIR_VPO_GUARD_MEM_MOTION)
            continue;

          // Most probably DIR.OMP.ORDERED, which we have to support in future.
          // But even any other directive is unexpected here, so be safe.
          LLVM_DEBUG(dbgs()
                     << (VPOAnalysisUtils::getDirectiveID(Call) ==
                                 DIR_OMP_ORDERED
                             ? "LV: Unimplemented omp simd ordered support."
                             : "LV: Unsupported nested region directive.")
                     << *Call << "\n");
          return false;
        }

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
  return true;
}

bool VPOVectorizationLegality::isLiveOut(const Instruction *I) const {
  if (!TheLoop->contains(I))
    return false;
  return (llvm::any_of(I->users(), [this](const User *U) {
    return !TheLoop->contains(cast<Instruction>(U));
  }));
}

const Instruction *
VPOVectorizationLegality::getLiveOutPhiOperand(const PHINode *Phi) const {
  if (isLiveOut(Phi))
    return Phi;
  auto Iter = llvm::find_if(Phi->operands(), [&](const Value *Oper) {
    if (const Instruction *I = dyn_cast<Instruction>(Oper))
      return isLiveOut(I);
    return false;
  });
  return Iter == Phi->op_end() ? nullptr : cast<Instruction>(*Iter);
}

// The routine can be called for two cases of phi, when one is in the loop
// header or for liveout phi.
//
// We detect two potential cases for private aliases here.
//
// 1) LoopPreheader:
//        %alias = load %private
//        ...
//    LoopHeader:
//        %priv_phi = phi [%alias, %LoopPreheader], ...
//
// 2) LoopPreheader:
//        ...
//    LoopHeader:
//        %priv_phi = phi [%some_const, %LoopPreheader], [%liveout_phi, %Latch]
//        ...
//    Body:
//        %priv = something
//        ...
//    Latch (or any other block)
//        %liveout_phi = phi [%priv_phi, %LoopHeader], [%priv, %Body]
//        ...
//    LoopExit:
//        %lcssa_phi = phi [%liveout_phi, %Latch]
//        store %lcssa.phi, %private
//
// 3) LoopPreheader:
//        %alias = load %private
//        ...
//    LoopHeader:
//        %priv_phi = phi [%alias, %loop_preheader], [%priv_exit, %latch]
//        ...
//    Latch:
//       %priv_exit = some_inst
//       ...
//    outside_loop:
//       ; no store to %private, just using %priv_exit
//
// In the first case, if the load from private is used in the phi then phi is
// alias for private.
// In the second case, we can have a check for both phis, from loop header and
// liveout phi from latch. The loop header phi is checked when the check in the
// first case does not work, e.g. the incoming value is constant.
// In the third case, if the load from private is used in the phi then phi is
// alias for private. The live out value is not stored into private but we can
// check the use in the header phi and propagate alias-ness from that phi.
//
// No other data dependency checks are done because we do this for simd loops
// only.
bool VPOVectorizationLegality::checkAndAddAliasForSimdLastPrivate(
    const PHINode *Phi) {
  if (!IsSimdLoop)
    return false;
  bool IsHeaderPhi = Phi->getParent() == TheLoop->getHeader();
  const Instruction *LiveOut = Phi;
  const BasicBlock *PreheaderBB = TheLoop->getLoopPreheader();

  auto CheckPhiPreheaderOperand =
      [PreheaderBB, this](const Instruction *LOut, const PHINode *Phi) {
        const Value *PHIncomingVal = Phi->getIncomingValueForBlock(PreheaderBB);
        if (auto *Priv = findPrivateOrAlias(PHIncomingVal)) {
          updatePrivateExitInst(Priv, LOut);
          return true;
        }
        return false;
      };

  if (IsHeaderPhi) {
    LiveOut = getLiveOutPhiOperand(Phi);
    if (!LiveOut)
      return false;
    if (CheckPhiPreheaderOperand(LiveOut, Phi))
      return true;
    if (!isa<PHINode>(LiveOut))
      return false;
  } else if (!isLiveOut(Phi))
    return false;

  // Liveout [phi] not in loop header.
  if (auto Priv = findPrivateOrAlias(LiveOut)) {
    updatePrivateExitInst(Priv, LiveOut);
    return true;
  }
  // Check whether the LiveOut is used by a header phi and that phi is alias for
  // a lastprivate. The case 3) above.
  auto HeaderUserIt = llvm::find_if(
      LiveOut->users(), [LoopHdr = TheLoop->getHeader()](const User *U) {
        auto HdrPhi = dyn_cast<PHINode>(U);
        return HdrPhi && HdrPhi->getParent() == LoopHdr;
      });
  if (HeaderUserIt != LiveOut->user_end()) {
    return CheckPhiPreheaderOperand(LiveOut, cast<PHINode>(*HeaderUserIt));
  }

  return false;
}

PrivDescr<Value>*
VPOVectorizationLegality::findPrivateOrAlias(const Value *Candidate) const {
  if (Privates.count(Candidate))
    return Privates.find(Candidate)->second.get();
  for (auto Priv : privates())
    if (Priv->findAlias(Candidate))
      return const_cast<PrivDescrTy*>(Priv);
  return nullptr;
}

void VPOVectorizationLegality::updatePrivateExitInst(PrivDescrTy *Priv,
                                                     const Instruction *ExitI) {
  if (!Priv->getUpdateInstructions().empty()) {
    assert(ExitI == Priv->getUpdateInstructions()[0] &&
           "second liveout for private");
  }
  Priv->addUpdateInstruction(ExitI);
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

  using namespace PatternMatch;
  // Int inductions are special because we only allow one IV.
  if (ID.getKind() == InductionDescriptor::IK_IntInduction &&
      ID.getConstIntStepValue() && match(ID.getConstIntStepValue(), m_One()) &&
      match(ID.getStartValue(), m_Zero())) {

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

bool VPOVectorizationLegality::isLoopPrivate(Value *V) const {
  return Privates.count(getPtrThruCast<BitCastInst>(V)) ||
         isInMemoryReduction(V);
}

bool VPOVectorizationLegality::isInMemoryReduction(Value *V) const {
  V = getPtrThruCast<BitCastInst>(V);
  return isa<PointerType>(V->getType()) && InMemoryReductions.count(V);
}

bool VPOVectorizationLegality::isInductionVariable(const Value *V) {
  Value *In0 = const_cast<Value *>(V);
  PHINode *PN = dyn_cast_or_null<PHINode>(In0);
  if (!PN)
    return false;
  return Inductions.count(PN);
}

