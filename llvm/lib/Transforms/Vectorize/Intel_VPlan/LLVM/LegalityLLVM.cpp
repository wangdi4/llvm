//===-LegalityLLVM.cpp-------------------------------------------*- C++ -*-===//
//
//   INTEL CONFIDENTIAL
//
//   Copyright (C) 2019 Intel Corporation
//
//   This software and the related documents are Intel copyrighted materials,
//   and your use of them is governed by the express license under which they
//   were provided to you ("License").  Unless the License provides otherwise,
//   you may not use, modify, copy, publish, distribute, disclose or treansmit
//   this software or the related documents without Intel's prior written
//   permission.
//
//   This software and the related documents are provided as is, with no
//   express or implied warranties, other than those that are expressly
//   stated in the License.
//
//===----------------------------------------------------------------------===//
///
///   \file LegalityLLVM.cpp
///   VPlan vectorizer's LLVM IR legality analysis.
///
///   Split from Legality.cpp on 2023-09-27.
///
//===----------------------------------------------------------------------===//

#include "LegalityLLVM.h"
#include "../IntelVPlan.h"
#include "../IntelVPlanUtils.h"
#include "llvm/IR/PatternMatch.h"

#define DEBUG_TYPE "VPlanLegality"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool>
    UseSimdChannels("use-simd-channels", cl::init(true), cl::Hidden,
                    cl::desc("use simd versions of read/write pipe functions"));

// Currently only for testing purposes, can be enabled when we start to support
// vectorizing nested SIMD directives
static cl::opt<bool>
    EnableNestedRegions("vplan-enable-nested-simd", cl::init(false), cl::Hidden,
                        cl::desc("Allow nesting of different SIMD regions"));

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
      if (isa<AddrSpaceCastInst>(U)) {
        WorkList.push_back(U);
        continue;
      }
      if (isa<GetElementPtrInst>(U))
        WorkList.push_back(U);

      if ((isa<PHINode>(U) || isa<SelectInst>(U)) && Visited.insert(U).second)
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

/// Check that the instruction's type doesn't preclude vectorization.
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

/// This converts the input type \p Ty to an integer type that's wide
/// enough to avoid overflow when computing the loop's trip count.
/// \p Ty is expected to be either a pointer or an integral type.
static Type *convertToSufficientlyWideIntegerType(const DataLayout &DL,
                                                  Type *Ty) {
  if (Ty->isPointerTy())
    return DL.getIntPtrType(Ty);

  // It is possible that char's or short's overflow when we ask for the loop's
  // trip count, work around this by changing the type size.
  if (Ty->getScalarSizeInBits() < 32)
    return Type::getInt32Ty(Ty->getContext());

  return Ty;
}

/// Return the wider of two types.  Note that short integer types are
/// widened to 32 bits before the comparison.
static Type *getWiderType(const DataLayout &DL, Type *Ty0, Type *Ty1) {
  Ty0 = convertToSufficientlyWideIntegerType(DL, Ty0);
  Ty1 = convertToSufficientlyWideIntegerType(DL, Ty1);
  if (Ty0->getScalarSizeInBits() > Ty1->getScalarSizeInBits())
    return Ty0;
  return Ty1;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void LegalityLLVM::dump(raw_ostream &OS) const {
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
bool LegalityLLVM::doesReductionUsePhiNodes(Value *RedVarPtr,
                                            PHINode *&LoopHeaderPhiNode,
                                            Value *&StartV) {
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
          // Tracked in CMPLRLLVM-52295.
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

bool LegalityLLVM::isInMemoryReductionPattern(Value *RedVarPtr,
                                              Instruction *&CallOrStore) {
  SmallVector<Value *, 4> Users;
  CallOrStore = nullptr;
  CallInst *Call = nullptr;
  // Collect relevant users of the reduction variable or any of its memory
  // aliases (casts).
  collectAllRelevantUsers(RedVarPtr->stripPointerCasts(), Users);
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

bool LegalityLLVM::isEndDirective(Instruction *I) const {
  return VPOAnalysisUtils::isEndDirective(I) &&
         VPOAnalysisUtils::getDirectiveID(I) == DIR_OMP_SIMD;
}

// Utility to analyze all instructions between the SIMD clause and the loop to
// identify any aliasing variables to the explicit SIMD descriptors. We traverse
// the CFG backwards, starting from Loop pre-header to the BB where SIMD clause
// is found.
void LegalityLLVM::collectPreLoopDescrAliases() {
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
void LegalityLLVM::collectPostExitLoopDescrAliases() {
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

// Check the safety of aliasing of loop-privates outside of the loop.
// We want to scan the block RegionEntry : loop-preheader and checks if we have
// a store of the  pointer to any memory locations. If that is the case, we
// treat this loop as unsafe for vectorization.
bool LegalityLLVM::isAliasingSafe(DominatorTree &DT,
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

void LegalityLLVM::parseMinMaxReduction(
    Value *RedVarPtr, RecurKind Kind,
    std::optional<InscanReductionKind> InscanRedKind, Type *Ty) {

  // Analyzing some possible scenarios:
  // (1)
  //  for.body:
  //  **REDUCTION PHI** -
  //  %LoopHeaderPhiNode = phi i32[%.pre, %PreHeader], [%MinMaxResultInst,
  //  %for.inc] %cmp1 = icmp sgt i32 %LoopHeaderPhiNode, %Val br i1 %cmp1, label
  //  %if.then, label %for.inc
  //
  //  if.then:
  //   STORE i32 %Val, i32* %min, align 4
  //   br label %for.inc
  //
  //  for.inc:
  //   % MinMaxResultInst = PHI i32[%Val, %if.then], [%LoopHeaderPhiNode,
  //   %for.body]
  //   ..
  //   br i1 %exitcond, label %for.end, label %for.body

  // (2)
  //  for.body:
  //  **REDUCTION PHI** -
  //  %LoopHeaderPhiNode = phi i32 [%.pre, %PreHeader], [%MinMaxResultInst,
  //  %for.inc] %cmp1 = icmp sgt i32 %LoopHeaderPhiNode, %Val %MinMaxResultInst
  //  = select i1 %cmp1, i32 %Val, i32 %LoopHeaderPhiNode

  // (3)
  //  for.body:
  //  **REDUCTION PHI** -
  //  %LoopHeaderPhiNode = phi float [%.pre, %PreHeader], [%MinMaxResultInst,
  //  %for.inc] %MinMaxResultInst = call @llvm.minnum.f32(float %Val, float
  //  %LoopHeaderPhiNode)

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
              match(
                  U,
                  m_CombineOr(
                      m_CombineOr(m_CombineOr(m_Intrinsic<Intrinsic::smin>(),
                                              m_Intrinsic<Intrinsic::smax>()),
                                  m_CombineOr(m_Intrinsic<Intrinsic::umin>(),
                                              m_Intrinsic<Intrinsic::umax>())),
                      m_CombineOr(
                          m_CombineOr(m_Intrinsic<Intrinsic::maxnum>(),
                                      m_Intrinsic<Intrinsic::minnum>()),
                          m_CombineOr(m_Intrinsic<Intrinsic::maximum>(),
                                      m_Intrinsic<Intrinsic::minimum>())))));
    });
    assert(It != LoopHeaderPhiNode->user_end() && "Reduction op not found!");
    MinMaxResultInst = cast<Instruction>(*It);

    if (match(MinMaxResultInst, m_Intrinsic<Intrinsic::maximum>()))
      Kind = RecurKind::FMaximum;
    else if (match(MinMaxResultInst, m_Intrinsic<Intrinsic::minimum>()))
      Kind = RecurKind::FMinimum;

    SmallPtrSet<Instruction *, 4> CastInsts;
    FastMathFlags FMF = FastMathFlags::getFast();
    RecurrenceDescriptor RD(StartV, MinMaxResultInst,
                            nullptr /*IntermediateStore*/, Kind, FMF, nullptr,
                            StartV->getType(), true /*Signed*/,
                            false /*Ordered*/, CastInsts, -1U);
    ExplicitReductions[LoopHeaderPhiNode] = {
        RD, RedVarPtr, std::nullopt /*InscanReductionKind*/};
  } else if (isInMemoryReductionPattern(RedVarPtr, ReductionUse)) {
    // Min/max opcodes are not expected for complex type reductions.
    InMemoryReductions[RedVarPtr] = {Kind, InscanRedKind, ReductionUse,
                                     false /*IsComplex*/, Ty};
  }
}

void LegalityLLVM::parseBinOpReduction(
    Value *RedVarPtr, RecurKind Kind,
    std::optional<InscanReductionKind> InscanRedKind, bool IsComplex,
    Type *Ty) {

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
    RecurrenceDescriptor RD(StartV, Combiner, nullptr /*IntermediateStore*/,
                            Kind, FMF, nullptr, ReductionPhi->getType(),
                            true /*Signed*/, false /*Ordered*/, CastInsts, -1U);
    ExplicitReductions[ReductionPhi] = {RD, RedVarPtr, InscanRedKind};
  } else if ((UseMemory = isInMemoryReductionPattern(RedVarPtr, ReductionUse)))
    InMemoryReductions[RedVarPtr] = {Kind, InscanRedKind, ReductionUse,
                                     IsComplex, Ty};

  if (!UsePhi && !UseMemory)
    LLVM_DEBUG(dbgs() << "LV: Explicit reduction pattern is not recognized ");
}

void LegalityLLVM::addReduction(
    Value *RedVarPtr, Type *Ty, RecurKind Kind,
    std::optional<InscanReductionKind> InscanRedKind, bool IsComplex) {
  assert(isa<PointerType>(RedVarPtr->getType()) &&
         "Expected reduction variable to be a pointer type");

  // TODO: Support min/max scan reductions as well.  Tracked in
  // CMPLRLLVM-52293.
  if (RecurrenceDescriptorData::isMinMaxRecurrenceKind(Kind)) {
    parseMinMaxReduction(RedVarPtr, Kind, InscanRedKind, Ty);
    return;
  }

  parseBinOpReduction(RedVarPtr, Kind, InscanRedKind, IsComplex, Ty);
}

bool LegalityLLVM::isExplicitReductionPhi(PHINode *Phi) {
  return ExplicitReductions.count(Phi);
}

bool LegalityLLVM::isPHIOkayForVectorization(PHINode *Phi, BasicBlock *BB,
                                             const WRNVecLoopNode *WRLp,
                                             BasicBlock *Header) {
  // If this PHINode is not in the header block, then we know that we
  // can convert it to select during if-conversion. No need to check if
  // the PHIs in this block are induction or reduction variables.
  if (BB != Header) {
    // Check that this instruction has no outside users or is an
    // identified reduction value with an outside user.
    if (!hasOutsideLoopUser(TheLoop, Phi, AllowedExit))
      return true;

    if (any_of(Phi->users(), [&](User *U) {
          return isa<PHINode>(U) && ExplicitReductions.count(cast<PHINode>(U));
        }))
      // Used in reduction scheme.
      return true;

    if (checkAndAddAliasForSimdLastPrivate(Phi))
      return true;

    if (checkUncondLastPrivOperands<PHINode>(
            Header, Phi, [&](PHINode *Phi) { return Inductions.count(Phi); }))
      return true;

    return bailoutWithDebug(
        OptReportVerbosity::Medium, OptRemarkID::VecFailUnknownLiveOut,
        INTERNAL("Loop contains a live-out value that could not be "
                 "identified as an induction or reduction."),
        WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                      : AuxRemarkID::Loop);
  }

  // We only allow if-converted PHIs with exactly two incoming values.
  if (Phi->getNumIncomingValues() != 2)
    return bailoutWithDebug(
        OptReportVerbosity::Medium, OptRemarkID::VecFailComplexControlFlow,
        INTERNAL("Loop contains a recurrent computation without "
                 "exactly two predecessors."),
        WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                      : AuxRemarkID::Loop,
        std::string(" 5.0"));

  if (isExplicitReductionPhi(Phi))
    return true;

  RecurrenceDescriptor RedDes;
  if (RecurrenceDescriptor::isReductionPHI(Phi, TheLoop, RedDes)) {
    AllowedExit.insert(RedDes.getLoopExitInstr());
    Reductions[Phi] = RedDes;
    return true;
  }

  InductionDescriptor ID;
  if (InductionDescriptor::isInductionPHI(Phi, TheLoop, PSE, ID, false,
                                          false /* OnlyConstPtrStep */) &&
      // The induction auto-recognizer can produce an induction in a
      // form that our framework doesn't handle when a variable-step
      // add recurrence is bypassed by a uniform compare from the
      // header to the latch.  In such cases when the comparison is
      // less or greater, scalar evolution correctly identifies the
      // step recurrence as one of the SCEVMinMaxExpr types.  The
      // current framework doesn't know what to do with this.  Since
      // it only happens for variable-step inductions, the problem
      // isn't observed in the community vectorizer.  It's best for
      // us to bail out in such cases.  See CMPLRLLVM-44206.
      !isa<SCEVMinMaxExpr>(ID.getStep())) {
    addInductionPhi(Phi, ID, AllowedExit);
    return true;
  }

  if (checkAndAddAliasForSimdLastPrivate(Phi))
    return true;

  LLVM_DEBUG(dbgs() << "LV: Found an unidentified PHI." << *Phi << "\n");
  return bailoutWithDebug(
      OptReportVerbosity::Medium, OptRemarkID::VecFailUnknownRecurrence,
      INTERNAL("Loop contains a recurrent computation that could not "
               "be identified as an induction or reduction."),
      WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                    : AuxRemarkID::Loop);
}

bool LegalityLLVM::isCallOkayForVectorization(CallInst *Call,
                                              const WRNVecLoopNode *WRLp) {
  Function *F = Call->getCalledFunction();
  if (!F)
    return true;

  if (vpo::VPOAnalysisUtils::isBeginDirective(Call)) {
    // Memory motion guarding directives are inserted for UDRs and will be
    // removed by VPlan framework.
    if (vpo::VPOAnalysisUtils::getDirectiveID(Call) == DIR_VPO_GUARD_MEM_MOTION)
      return true;

    if (EnableNestedRegions)
      return true;

    // Most probably DIR.OMP.ORDERED, which we have to support in future.
    // But even any other directive is unexpected here, so be safe.
    bool OmpOrd = VPOAnalysisUtils::getDirectiveID(Call) == DIR_OMP_ORDERED;
    LLVM_DEBUG(dbgs() << "LV: For call " << *Call << ":\n");
    if (OmpOrd)
      return bailout(
          OptReportVerbosity::Medium, OptRemarkID::VecFailGenericBailout,
          OptReportAuxDiag::getMsg(AuxRemarkID::OmpSimdOrderedUnsupported));
    else if (NestedSimdStrategy == NestedSimdStrategies::BailOut)
      return bailoutWithDebug(
          OptReportVerbosity::Medium, OptRemarkID::VecFailNestedSimdRegion,
          INTERNAL("Unsupported nested OpenMP (simd) loop or region."),
          WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                        : AuxRemarkID::Loop);
  }

  // Bail out if we need to scalarize the read/write pipe OpenCL calls. We
  // have to do this because there are no users of these calls directly since
  // the results are written through a ptr argument. Thus, the vectorizer is
  // unable to correctly materialize the necessary scalars into a vector
  // through the VectorLoopValueMap. See getOrCreateVectorValue().
  if ((isOpenCLReadChannel(F->getName()) ||
       isOpenCLWriteChannel(F->getName())) &&
      !UseSimdChannels) {
    return bailout(OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
                   INTERNAL("OpenCL read/write channel is not enabled."));
  }

  return true;
}

bool LegalityLLVM::canVectorize(DominatorTree &DT, const WRNVecLoopNode *WRLp) {
  IsSimdLoop = WRLp;
  clearBailoutRemark();

  // Import explicit data from WRLoop.
  // Decision about loop vectorization is based on this data.
  if (!EnterExplicitData(WRLp)) {
    assert(BR.BailoutRemark && "EnterExplicitData didn't set bailout data!");
    return false;
  }

  if (IsSimdLoop) {
    collectPreLoopDescrAliases();
    collectPostExitLoopDescrAliases();
  }

  if (TheLoop->getNumBackEdges() != 1 || !TheLoop->getExitingBlock())
    return bailoutWithDebug(
        OptReportVerbosity::Medium, OptRemarkID::VecFailComplexControlFlow,
        INTERNAL("Loop control flow is not understood by vectorizer"),
        WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                      : AuxRemarkID::Loop,
        std::string(" 5.0"));

  // We only handle bottom-tested loops, i.e. loop in which the condition is
  // checked at the end of each iteration. With that we can assume that all
  // instructions in the loop are executed the same number of times.
  if (TheLoop->getExitingBlock() != TheLoop->getLoopLatch())
    return bailoutWithDebug(
        OptReportVerbosity::Medium, OptRemarkID::VecFailComplexControlFlow,
        INTERNAL("Loop control flow is not understood by vectorizer"),
        WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                      : AuxRemarkID::Loop,
        std::string(" 5.0"));

  // ScalarEvolution needs to be able to find the exit count.
  const SCEV *ExitCount = PSE.getBackedgeTakenCount();
  if (ExitCount == PSE.getSE()->getCouldNotCompute())
    return bailoutWithDebug(
        OptReportVerbosity::High, OptRemarkID::VecFailUnknownInductionVariable,
        INTERNAL("LV: SCEV could not compute the loop iteration count."),
        WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                      : AuxRemarkID::Loop,
        std::string(" 5.0"));

  // Check if aliasing of privates is safe outside of the loop.
  CallInst *RegionEntry =
      WRLp ? cast<CallInst>(WRLp->getEntryDirective()) : nullptr;

  if (!isAliasingSafe(DT, RegionEntry))
    return bailout(OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
                   INTERNAL("Aliasing of privates outside the loop can't be "
                            "determined to be safe."));

  LoadInst *VolatileLoad = nullptr;
  BasicBlock *Header = TheLoop->getHeader();
  // For each block in the loop.
  for (BasicBlock *BB : TheLoop->blocks()) {
    // Scan the instructions in the block and look for hazards.
    for (Instruction &I : *BB) {

      if (!isSupportedInstructionType(I))
        return bailoutWithDebug(
            OptReportVerbosity::Medium, OptRemarkID::VecFailBadType,
            INTERNAL("Instruction contains unsupported data type"),
            WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                          : AuxRemarkID::Loop);

      // Look for a Windows atomic load idiom; for example:
      //   %ld = load volatile i64, ptr %g, align 8
      //   fence syncscope("singlethread") seq_cst
      // Together these do the Windows equivalent of "load atomic".
      // For now we bail out rather than try to correctly scalarize
      // these if the loop is otherwise vectorizable.
      if (auto *Load = dyn_cast<LoadInst>(&I))
        if (Load->isVolatile())
          VolatileLoad = Load;

      if (auto *Fence = dyn_cast<FenceInst>(&I)) {
        if (VolatileLoad &&
            Fence->getOrdering() == AtomicOrdering::SequentiallyConsistent &&
            Fence->getSyncScopeID() == SyncScope::SingleThread)
          return bailoutWithDebug(
              OptReportVerbosity::Medium, OptRemarkID::VecWindowsAtomic,
              INTERNAL("Loop contains a volatile load and fence."),
              WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                            : AuxRemarkID::Loop,
              "load");
      }

      if (auto *Phi = dyn_cast<PHINode>(&I)) {
        if (!isPHIOkayForVectorization(Phi, BB, WRLp, Header))
          return false;
        continue;
      }

      if (auto Call = dyn_cast<CallInst>(&I)) {
        if (!isCallOkayForVectorization(Call, WRLp))
          return false;
        continue;
      }
    }
  }
  if (!Induction && Inductions.empty())
    return bailoutWithDebug(
        OptReportVerbosity::High, OptRemarkID::VecFailUnknownInductionVariable,
        INTERNAL("LV: Did not find one integer induction var."),
        WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                      : AuxRemarkID::Loop,
        std::string(" 5.0"));

  return true;
}

bool LegalityLLVM::isLiveOut(const Instruction *I) const {
  if (!TheLoop->contains(I))
    return false;
  return (llvm::any_of(I->users(), [this](const User *U) {
    return !TheLoop->contains(cast<Instruction>(U));
  }));
}

const Instruction *
LegalityLLVM::getLiveOutPhiOperand(const PHINode *Phi) const {
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
bool LegalityLLVM::checkAndAddAliasForSimdLastPrivate(const PHINode *Phi) {
  if (!IsSimdLoop)
    return false;
  bool IsHeaderPhi = Phi->getParent() == TheLoop->getHeader();
  const Instruction *LiveOut = Phi;
  const BasicBlock *PreheaderBB = TheLoop->getLoopPreheader();

  auto CheckPhiPreheaderOperand = [PreheaderBB, this](const Instruction *LOut,
                                                      const PHINode *Phi) {
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

PrivDescr<Value> *
LegalityLLVM::findPrivateOrAlias(const Value *Candidate) const {
  if (Privates.count(Candidate))
    return Privates.find(Candidate)->second.get();
  for (auto Priv : privates())
    if (Priv->findAlias(Candidate))
      return const_cast<PrivDescrTy *>(Priv);
  return nullptr;
}

void LegalityLLVM::updatePrivateExitInst(PrivDescrTy *Priv,
                                         const Instruction *ExitI) {
  if (!Priv->getUpdateInstructions().empty()) {
    assert(ExitI == Priv->getUpdateInstructions()[0] &&
           "second liveout for private");
  }
  Priv->addUpdateInstruction(ExitI);
}

void LegalityLLVM::addInductionPhi(PHINode *Phi, const InductionDescriptor &ID,
                                   SmallPtrSetImpl<Value *> &AllowedExit) {

  Inductions[Phi] = ID;

  Type *PhiTy = Phi->getType();
  const DataLayout &DL = Phi->getModule()->getDataLayout();

  // Get the widest type.
  if (!PhiTy->isFloatingPointTy()) {
    if (!WidestIndTy)
      WidestIndTy = convertToSufficientlyWideIntegerType(DL, PhiTy);
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

bool LegalityLLVM::isLoopPrivate(Value *V) const {
  return Privates.count(getPtrThruCast<BitCastInst>(V)) ||
         isInMemoryReduction(V);
}

bool LegalityLLVM::isInMemoryReduction(Value *V) const {
  V = getPtrThruCast<BitCastInst>(V);
  return isa<PointerType>(V->getType()) && InMemoryReductions.count(V);
}

bool LegalityLLVM::isInductionVariable(const Value *V) {
  Value *In0 = const_cast<Value *>(V);
  PHINode *PN = dyn_cast_or_null<PHINode>(In0);
  if (!PN)
    return false;
  return Inductions.count(PN);
}
