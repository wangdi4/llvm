//===-------------------- HIRMemoryReductionSinking.cpp -------------------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file sinks invariant memory reductions to loop's postexit.
//
// For example:
//
// DO i1
//   A[i1] = A[i1] + t1;
//   A[5] = A[5] + t2;
// END DO
//
// ===>
//
//  tr = 0;
// DO i1
//   A[i1] = A[i1] + t1;
//   tr = tr + t2;
// END DO
//  A[5] = A[5] + tr;
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRMemoryReductionSinkingPass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#define OPT_SWITCH "hir-memory-reduction-sinking"
#define OPT_DESC "HIR Memory Reduction Sinking"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

// Turn off reductions inside conditions
static cl::opt<bool> DisableConditionalReductions(
    "disable-" OPT_SWITCH "-conditional-reductions", cl::init(false),
    cl::Hidden,
    cl::desc("Disable " OPT_DESC " for reductions inside conditions"));

namespace {

// The memory reduction is described by a load ref and a store ref.
// There are two kinds of patterns-
// 1) Load inst followed by store inst with terminal ref-
//    %0 = A[5];
//    A[5] = %0 + t;
//
// This happens for integer types.
//
// 2) Reduction operation containing the load followed by store inst-
//   %add = A[5]  +  %t;
//   A[5] = %add;
//
// This happens for fp types and some integer types.
//
// Note: We are not handling load and store in the same instruction.
// This can only happen if a HIR transformation creates such an instruction.
// A[5] = A[5] + t;
struct MemoryReductionInfo {
  unsigned Opcode;
  FastMathFlags FMF;
  RegDDRef *LoadRef;
  RegDDRef *StoreRef;

  MemoryReductionInfo(unsigned Opcode, FastMathFlags FMF, RegDDRef *LoadRef,
                      RegDDRef *StoreRef)
      : Opcode(Opcode), FMF(FMF), LoadRef(LoadRef), StoreRef(StoreRef) {}
};

class HIRMemoryReductionSinking {
  HIRLoopStatistics &HLS;
  HIRDDAnalysis &HDDA;

  // If true, then we are going to apply the transformation for the reductions
  // inside conditions.
  bool AllowConditionalReductionSinking;

  SmallVector<MemoryReductionInfo, 16> MemoryReductions;
  SmallVector<MemoryReductionInfo, 8> InvariantMemoryReductions;
  SmallVector<MemoryReductionInfo, 8> ConditionalInvariantMemoryReductions;

public:
  HIRMemoryReductionSinking(HIRLoopStatistics &HLS, HIRDDAnalysis &HDDA,
                            bool AllowConditionalReductionSinking)
      : HLS(HLS), HDDA(HDDA),
        AllowConditionalReductionSinking(AllowConditionalReductionSinking) {}
  bool run(HLLoop *Lp);

  bool validateReductionTemp(DDGraph DDG, const HLLoop *Lp);
  bool validateMemoryReductions(const HLLoop *Lp);
  bool isValidReductionRef(const DDRef *Ref, unsigned Opcode);
  void sinkInvariantReductions(HLLoop *Lp);

private:
  void clearReductions() {
    MemoryReductions.clear();
    InvariantMemoryReductions.clear();
    ConditionalInvariantMemoryReductions.clear();
  }

  bool haveCandidateReductions() {
    return !InvariantMemoryReductions.empty() ||
           !ConditionalInvariantMemoryReductions.empty();
  }

  // Return true if the user or the pass manager disables the transformation
  // for reductions inside condition.
  bool isConditionalReductionSinkingDisabled() {
    return (DisableConditionalReductions || !AllowConditionalReductionSinking);
  }
};

// This visitor is used to traverse through the innermost loop and identify
// the possible reductions that will be sinked.
class CandidatesCollector final : public HLNodeVisitorBase {
private:
  SmallVector<MemoryReductionInfo, 16> &MemoryReductions;
  SmallVector<MemoryReductionInfo, 8> &InvariantMemoryReductions;
  SmallVector<MemoryReductionInfo, 8> &ConditionalInvariantMemoryReductions;
  HLLoop *ParentLoop;

public:
  CandidatesCollector(
      SmallVector<MemoryReductionInfo, 16> &MemoryReductions,
      SmallVector<MemoryReductionInfo, 8> &InvariantMemoryReductions,
      SmallVector<MemoryReductionInfo, 8> &ConditionalInvariantMemoryReductions,
      HLLoop *Loop)
      : MemoryReductions(MemoryReductions),
        InvariantMemoryReductions(InvariantMemoryReductions),
        ConditionalInvariantMemoryReductions(
            ConditionalInvariantMemoryReductions),
        ParentLoop(Loop) {}

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  void visit(HLInst *HInst);
};

// Helper class used for handling the ORing process of a reduction If.
class ReductionIfInfo {
private:
  // HLIf that will be placed outside the loop to finalize the reduction
  HLIf *ReductionIf;

  // True if the reduction If is ready for combine multiple reductions inside
  // the same condition.
  bool ReductionIfCanBeORed;

public:
  ReductionIfInfo() : ReductionIf(nullptr), ReductionIfCanBeORed(false) {}

  void resetReductionIfInfo(HLIf *NewReductionIf) {
    ReductionIf = NewReductionIf;
    ReductionIfCanBeORed = false;
  }

  HLIf *getReductionIf() const { return ReductionIf; }

  void updateReductionIf(RegDDRef *LoadRef, HLInst *InitInst);

private:
  void prepareReductionIfForORing();
};

} // namespace

static HLInst *getReductionStore(HLInst *LoadInst, bool CheckSelfBlobRval) {
  // We only handle consecutive load and store instructions when looking for
  // reductions.
  // TODO: Extend logic to handle non-consecutive loads and stores.
  auto *SInst = dyn_cast_or_null<HLInst>(LoadInst->getNextNode());

  if (!SInst || !isa<StoreInst>(SInst->getLLVMInstruction())) {
    return nullptr;
  }

  auto *StoreRvalRef = SInst->getRvalDDRef();

  if (!StoreRvalRef->isTerminalRef()) {
    return nullptr;
  }

  unsigned TempIndex = LoadInst->getLvalDDRef()->getSelfBlobIndex();
  if (CheckSelfBlobRval) {
    if (!StoreRvalRef->isSelfBlob() ||
        StoreRvalRef->getSelfBlobIndex() != TempIndex) {
      return nullptr;
    }
  } else {
    if (!StoreRvalRef->getSingleCanonExpr()->containsStandAloneBlob(TempIndex,
                                                                    false)) {
      return nullptr;
    }
  }

  return SInst;
}

// Goes through top-level loop nodes and collects candidates which structually
// look like memory reductions. DD based legality is checked later.
void CandidatesCollector::visit(HLInst *HInst) {
  unsigned Level = ParentLoop->getNestingLevel();
  auto *FirstChild = ParentLoop->getFirstChild();

  bool IsFirstPattern = true;

  auto *LLVMInst = HInst->getLLVMInstruction();
  unsigned Opcode;
  FastMathFlags FMF;
  RegDDRef *LoadRef = nullptr;
  RegDDRef *AlternateLoadRef = nullptr;
  HLInst *StoreInst = nullptr;

  if (isa<LoadInst>(LLVMInst)) {
    // Looking for this pattern for integer types-
    // (first pattern)
    //    %0 = A[5];
    //    A[5] = %0 + t;
    LoadRef = HInst->getRvalDDRef();

    if (!LoadRef->getDestType()->isIntegerTy()) {
      return;
    }

    Opcode = Instruction::Add;

  } else if (isa<BinaryOperator>(LLVMInst) && HInst->isReductionOp(&Opcode)) {
    // Looking for this pattern-
    // (second pattern)
    //   %add = A[5]  +  %t;
    //   A[5] = %add;
    auto *FPOp = dyn_cast<FPMathOperator>(LLVMInst);

    if (FPOp) {
      if (!FPOp->hasAllowReassoc()) {
        return;
      }
      FMF = FPOp->getFastMathFlags();
    }

    LoadRef = HInst->getOperandDDRef(1);
    if (Instruction::isCommutative(Opcode))
      AlternateLoadRef = HInst->getOperandDDRef(2);

    if (!LoadRef->isMemRef() &&
        (!AlternateLoadRef || !AlternateLoadRef->isMemRef()))
      return;

    IsFirstPattern = false;

  } else {
    return;
  }

  StoreInst = getReductionStore(HInst, !IsFirstPattern);
  if (!StoreInst)
    return;

  auto *StoreRef = StoreInst->getLvalDDRef();

  if (!LoadRef->isMemRef() || !DDRefUtils::areEqual(LoadRef, StoreRef)) {
    if (AlternateLoadRef && AlternateLoadRef->isMemRef() &&
        DDRefUtils::areEqual(AlternateLoadRef, StoreRef))
      LoadRef = AlternateLoadRef;
    else
      return;
  }

  if (LoadRef->isStructurallyInvariantAtLevel(Level)) {
    auto &InvariantReductions =
        HLNodeUtils::postDominates(StoreInst, FirstChild)
            ? InvariantMemoryReductions
            : ConditionalInvariantMemoryReductions;
    InvariantReductions.emplace_back(Opcode, FMF, LoadRef,
                                     StoreInst->getLvalDDRef());
  } else if (!ParentLoop->isLiveOut(HInst->getLvalDDRef()->getSymbase())) {
    // MemoryReductions tracks the non-invariant reductions that could
    // alias to other invariant reductions, which we are trying to sink.
    // Example:
    // %0 = (@A)[0][i1];  <-- Memory Reduction candidate
    // (@A)[0][i1] = %0 + 2;
    // %1 = (@A)[0][5];   <-- InvariantMemoryReduction candidate
    // (@A)[0][5] = %1 + 3;

    // We only save memory reduction if the Lval Temp is not liveout.
    // Sinking an aliasing invariant memory reduction does not preserve
    // liveout for this reduction. Later validation checks will bailout.
    MemoryReductions.emplace_back(Opcode, FMF, LoadRef,
                                  StoreInst->getLvalDDRef());
  }
}

bool HIRMemoryReductionSinking::validateReductionTemp(DDGraph DDG,
                                                      const HLLoop *Lp) {
  // Checks whether the lval temp of the reduction is used elsewhere in the
  // loop. If so, invalidates the reduction.

  auto ReductionTempEscapes = [&](MemoryReductionInfo &RednInfo) {
    auto *LvalTempRef = RednInfo.LoadRef->getHLDDNode()->getLvalDDRef();
    // TODO: handle liveout temps depending on reduction pattern.
    if (Lp->isLiveOut(LvalTempRef->getSymbase())) {
      return true;
    }

    auto *UseNode = RednInfo.StoreRef->getHLDDNode();

    bool Escapes = false;
    for (auto *Edge : DDG.outgoing(LvalTempRef)) {
      if (!Edge->isFlow()) {
        continue;
      }

      if (Edge->getSink()->getHLDDNode() != UseNode) {
        Escapes = true;
        break;
      }
    }

    return Escapes;
  };

  InvariantMemoryReductions.erase(
      std::remove_if(InvariantMemoryReductions.begin(),
                     InvariantMemoryReductions.end(), ReductionTempEscapes),
      InvariantMemoryReductions.end());

  ConditionalInvariantMemoryReductions.erase(
      std::remove_if(ConditionalInvariantMemoryReductions.begin(),
                     ConditionalInvariantMemoryReductions.end(),
                     ReductionTempEscapes),
      ConditionalInvariantMemoryReductions.end());

  MemoryReductions.erase(std::remove_if(MemoryReductions.begin(),
                                        MemoryReductions.end(),
                                        ReductionTempEscapes),
                         MemoryReductions.end());

  return haveCandidateReductions();
}

// This function checks if the input opcode is a subtraction, then return the
// opcode for addition, else return the same. It is used to associate the
// reductions that combine additions and subtractions.
static unsigned getAssociatedOpcode(unsigned Opcode) {
  if (Opcode == Instruction::FSub) {
    return Instruction::FAdd;
  }

  if (Opcode == Instruction::Sub) {
    return Instruction::Add;
  }

  return Opcode;
}

static bool
isValidReductionRef(const DDRef *Ref, unsigned Opcode,
                    const SmallVectorImpl<MemoryReductionInfo> &MemReductions) {
  bool CheckLval = Ref->isLval();

  for (auto &OtherRedn : MemReductions) {
    if (getAssociatedOpcode(Opcode) !=
        getAssociatedOpcode(OtherRedn.Opcode))
      continue;

    if (CheckLval) {
      if (Ref == OtherRedn.StoreRef) {
        return true;
      }
    } else if (Ref == OtherRedn.LoadRef) {
      return true;
    }
  }

  return false;
}

bool HIRMemoryReductionSinking::isValidReductionRef(const DDRef *Ref,
                                                    unsigned Opcode) {
  return ::isValidReductionRef(Ref, Opcode, MemoryReductions) ||
         ::isValidReductionRef(Ref, Opcode, InvariantMemoryReductions) ||
         ::isValidReductionRef(Ref, Opcode,
                               ConditionalInvariantMemoryReductions);
}

bool HIRMemoryReductionSinking::validateMemoryReductions(const HLLoop *Lp) {
  DDGraph DDG = HDDA.getGraph(Lp);
  LLVM_DEBUG(dbgs() << "[MRS] Loop DDG:\n");
  LLVM_DEBUG(DDG.dump());

  if (!validateReductionTemp(DDG, Lp)) {
    return false;
  }

  // Check whether the store of invariant memory reduction has edges to
  // non-reduction memrefs inside the loop. If so, we invalidate the reduction.
  auto InvReductionStoreEscapes = [&](MemoryReductionInfo &InvRedn) {
    auto *StoreRef = InvRedn.StoreRef;

    bool Escapes = false;
    auto *StoreTy = StoreRef->getDestType();
    auto StoreSize = StoreRef->getDestTypeSizeInBytes();
    auto StoreAlignment = StoreRef->getAlignment();

    for (auto *Edge : DDG.outgoing(StoreRef)) {
      auto *SinkRef = Edge->getSink();

      if (SinkRef == StoreRef || SinkRef == InvRedn.LoadRef) {
        continue;
      }

      auto *SinkRegRef = cast<RegDDRef>(SinkRef);
      // Give up if there is a possibiliy of partial overlap between dependent
      // refs. Partial overlap is not possible between refs of same type, same
      // alignment, and alignment >= size if the base and shape aren't equal.
      if ((StoreTy != SinkRef->getDestType()) ||
          (StoreAlignment != SinkRegRef->getAlignment()) ||
          (StoreAlignment < StoreSize &&
           !DDRefUtils::haveEqualBaseAndShapeAndOffsets(SinkRegRef, StoreRef,
                                                        false))) {
        Escapes = true;
        break;
      }

      if (!isValidReductionRef(SinkRef, InvRedn.Opcode)) {
        Escapes = true;
        break;
      }
    }

    return Escapes;
  };

  InvariantMemoryReductions.erase(
      std::remove_if(InvariantMemoryReductions.begin(),
                     InvariantMemoryReductions.end(), InvReductionStoreEscapes),
      InvariantMemoryReductions.end());

  // If the optimization is disabled then clear the data collected for
  // invariant memory reductions inside conditions. We don't disable the
  // collection earlier because the data collected from these reductions can be
  // useful for the reductions identified outside the conditions
  // (InvariantMemoryReductions).
  if (isConditionalReductionSinkingDisabled())
    ConditionalInvariantMemoryReductions.clear();
  else
    ConditionalInvariantMemoryReductions.erase(
        std::remove_if(ConditionalInvariantMemoryReductions.begin(),
                       ConditionalInvariantMemoryReductions.end(),
                       InvReductionStoreEscapes),
        ConditionalInvariantMemoryReductions.end());

  return haveCandidateReductions();
}

static RegDDRef *createReductionInitializer(HLLoop *Lp, unsigned Opcode,
                                            FastMathFlags FMF, Type *Ty) {
  // Creates reduction initialization and inserts it in the loop preheader-
  // %tmp = <identity constant>

  auto &HNU = Lp->getHLNodeUtils();
  auto *Const = HLInst::getRecurrenceIdentity(Opcode, Ty, FMF);

  RegDDRef *InitRef = nullptr;

  if (auto *IntConst = dyn_cast<ConstantInt>(Const)) {
    int64_t Val = IntConst->getSExtValue();
    InitRef = HNU.getDDRefUtils().createConstDDRef(Ty, Val);
  } else {
    InitRef = HNU.getDDRefUtils().createConstDDRef(Const);
  }

  auto *InitInst = HNU.createCopyInst(InitRef, "tmp");

  HLNodeUtils::insertAsLastPreheaderNode(Lp, InitInst);

  auto *RednTemp = InitInst->getLvalDDRef();
  unsigned Symbase = RednTemp->getSymbase();

  // Add new reduction temp as livein and liveout to loop.
  Lp->addLiveInTemp(Symbase);
  Lp->addLiveOutTemp(Symbase);

  return RednTemp;
}

// This function uses the information in the input MemoryReductionInfo to
// create a reduction temp and replaces reduction load/store inside the loop
// with the temp.
static HLInst *createReductionTemp(HLLoop *Lp, MemoryReductionInfo &InvRedn) {
  auto &HNU = Lp->getHLNodeUtils();
  unsigned Level = Lp->getNestingLevel();

  auto *LoadRef = InvRedn.LoadRef;
  auto *LoadInst = cast<HLInst>(LoadRef->getHLDDNode());

  auto *StoreRef = InvRedn.StoreRef;
  auto *StoreInst = StoreRef->getHLDDNode();

  unsigned Opcode = getAssociatedOpcode(InvRedn.Opcode);

  auto *RednTemp = createReductionInitializer(Lp, Opcode, InvRedn.FMF,
                                              LoadRef->getDestType());

  auto *BinOp = dyn_cast<BinaryOperator>(LoadInst->getLLVMInstruction());
  RegDDRef *RednOpRef = nullptr;

  // Replace reduction operand by new temp in the original reduction
  // instruction. The original reduction operand will be used in temp
  // reduction.
  if (BinOp) {
    // Handling this pattern-
    //   %add = A[5]  +  B[i];
    //   A[5] = %add;
    //
    //   ==>
    //
    //   %add = A[5]  +  %tmp
    //   A[5] = %add;
    if (LoadRef == LoadInst->getOperandDDRef(1)) {
      RednOpRef = LoadInst->removeOperandDDRef(2);
      LoadInst->setOperandDDRef(RednTemp->clone(), 2);
    } else {
      RednOpRef = LoadInst->removeOperandDDRef(1);
      LoadInst->setOperandDDRef(RednTemp->clone(), 1);
    }
  } else {
    // Handling this pattern (integer types only)-
    //   %0 = A[5];
    //   A[5] = %0 + t;
    //
    //   ==>
    //
    //   %0 = A[5];
    //   A[5] = %0 + %tmp;
    RednOpRef = StoreInst->removeOperandDDRef(1);
    auto *LoadTemp = LoadInst->getLvalDDRef();
    unsigned TempIndex = LoadTemp->getSelfBlobIndex();
    RednOpRef->getSingleCanonExpr()->removeBlob(TempIndex);
    RednOpRef->makeConsistent({}, Level);

    auto *NewStoreRval = RednTemp->clone();
    NewStoreRval->getSingleCanonExpr()->addBlob(TempIndex, 1);
    StoreInst->setOperandDDRef(NewStoreRval, 1);
    NewStoreRval->makeConsistent({RednTemp, LoadTemp}, Level - 1);
  }

  auto *TmpRednInst = HNU.createBinaryHLInst(
      Opcode, RednTemp->clone(), RednOpRef, "redn", RednTemp->clone(), BinOp);

  HLNodeUtils::insertBefore(LoadInst, TmpRednInst);

  return cast<HLInst>(RednTemp->getHLDDNode());
}

// This function updates the optimization remarks after the transformation.
static void addOptimizationsRemark(HLLoop *Lp, MemoryReductionInfo &InvRedn) {

  OptReportBuilder &ORBuilder =
      Lp->getHLNodeUtils().getHIRFramework().getORBuilder();
  auto *StoreRef = InvRedn.StoreRef;

  unsigned StoreLineNum = 0;

  if (StoreRef->getDebugLoc()) {
    StoreLineNum = StoreRef->getDebugLoc().getLine();
  }

  // Load/Store of reduction at line %d sinked after loop
  ORBuilder(*Lp).addRemark(OptReportVerbosity::Low,
                           OptRemarkID::MemoryReductionSinking, StoreLineNum);
}

// Linear (invariant) memrefs can become non-linear in post-exit as they are
// now in outer loop scope.
static void makeLoadAndStoreRefsConsistent(HLLoop *Lp,
                                           MemoryReductionInfo &InvRedn) {
  unsigned Level = Lp->getNestingLevel();
  InvRedn.LoadRef->makeConsistent({}, Level - 1);
  InvRedn.StoreRef->makeConsistent({}, Level - 1);
}

// Create a new If condition that will be used to encapsulate the load and
// store of the reduction. This node will be created if we found that the
// reduction is inside an If. For example:
//
//   BEGIN REGION { }
//         + DO i1 = 0, 99, 1   <DO_LOOP>
//         |   if ((%b)[i1] > 10)
//         |   {
//         |      %1 = (%a)[5];
//         |      (%a)[5] = %1 + 2;
//         |   }
//         + END LOOP
//   END REGION
//
// Will be converted as follows:
//
//   BEGIN REGION { modified }
//         %tmp = 0;
//
//         + DO i1 = 0, 99, 1   <DO_LOOP>
//         |   if ((%b)[i1] > 10)
//         |   {
//         |      %tmp = %tmp  +  2;
//         |   }
//         + END LOOP
//
//         if (%tmp != 0)
//         {
//            %1 = (%a)[5];
//            (%a)[5] = %1 + %tmp;
//         }
//   END REGION
//
// This function creates the If used to compare if %tmp is not equal to 0.
static HLIf *createConditionalReductionIf(RegDDRef *LoadRef, HLInst *InitInst) {

  auto *RednTemp = InitInst->getLvalDDRef();
  auto *TempInitVal = InitInst->getRvalDDRef();

  auto &HNU = InitInst->getHLNodeUtils();

  // TODO: We need to make sure if the floating point comparison is ordered or
  // unordered when the LoadRef's type is a floating point (CMPLRLLVM-51654).
  auto PredTy = LoadRef->getDestType()->isIntegerTy() ? PredicateTy::ICMP_NE
                                                      : PredicateTy::FCMP_UNE;
  auto *LHS = RednTemp->clone();
  auto *RHS = TempInitVal->clone();
  auto *NewIf = HNU.createHLIf(PredTy, LHS, RHS);

  return NewIf;
}

// Helper function that will be used to preprare the reduction If to be ORed.
// For example, assume the following case:
//
//   BEGIN REGION { }
//         + DO i1 = 0, 99, 1   <DO_LOOP>
//         |   if ((%b)[i1] > 10)
//         |   {
//         |      %1 = (%a)[5];
//         |      (%a)[5] = %1 + 2;
//         |      %2 = (%c)[7];
//         |      (%c)[7] = %2 + 4;
//         |      %3 = (%d)[10];
//         |      (%d)[10] = %3 + 8;
//         |   }
//         + END LOOP
//   END REGION
//
// The first reduction that will be handled is the reduction between %3 and
// (%d)[10]. The loop will look as follows after the first transformation:
//
//   BEGIN REGION { }
//         %tmp = 0;
//         + DO i1 = 0, 99, 1   <DO_LOOP>
//         |   if ((%b)[i1] > 10)
//         |   {
//         |      %1 = (%a)[5];
//         |      (%a)[5] = %1 + 2;
//         |      %2 = (%c)[7];
//         |      (%c)[7] = %2 + 4;
//         |      %tmp = %tmp  +  8;
//         |   }
//         + END LOOP
//
//         if (%tmp != 0)
//         {
//            %3 = (%d)[10];
//            (%d)[10] = %3 + %tmp;
//         }
//   END REGION
//
// We need to change the predicate of the reduction If in order to include the
// reduction between %2 and (%c)[7] in it. This function will cleanup the
// reduction If as follows:
//
//         %cmp = %tmp != 0;
//         if (%cmp != 0)
//         {
//            %3 = (%d)[10];
//            (%d)[10] = %3 + %tmp;
//         }
//
// The new compare (%cmp) will be used now for ORing the condition inside the
// reduction If (see updateReductionIf comments for more details).
void ReductionIfInfo::prepareReductionIfForORing() {
  assert(!ReductionIfCanBeORed &&
         "Reduction If has been prepared for ORing already");

  auto &HNU = ReductionIf->getHLNodeUtils();
  auto PredBegin = ReductionIf->pred_begin();
  auto &Pred = *PredBegin;

  auto *CmpLHS = ReductionIf->removeLHSPredicateOperandDDRef(PredBegin);
  auto *CmpRHS = ReductionIf->removeRHSPredicateOperandDDRef(PredBegin);
  auto *Cmp = HNU.createCmp(Pred, CmpLHS, CmpRHS);
  HLNodeUtils::insertBefore(ReductionIf, Cmp);

  // The predicate inside the If condition will be the result of a compare
  // now, therefore it should be compared against an integer 0.
  auto *LHS = Cmp->getLvalDDRef()->clone();
  auto *RHSZero = Cmp->getDDRefUtils().createNullDDRef(LHS->getDestType());
  ReductionIf->replacePredicate(PredBegin, PredicateTy::ICMP_NE);

  ReductionIf->setLHSPredicateOperandDDRef(LHS, PredBegin);
  ReductionIf->setRHSPredicateOperandDDRef(RHSZero, PredBegin);

  ReductionIfCanBeORed = true;
}

// Update the reduction If to include more that one reduction inside the
// condition. For example, assume the following loop
//
//   BEGIN REGION { }
//         + DO i1 = 0, 99, 1   <DO_LOOP>
//         |   if ((%b)[i1] > 10)
//         |   {
//         |      %1 = (%a)[5];
//         |      (%a)[5] = %1 + 2;
//         |      %2 = (%c)[7];
//         |      (%c)[7] = %2 + 4;
//         |      %3 = (%d)[10];
//         |      (%d)[10] = %3 + 8;
//         |   }
//         + END LOOP
//   END REGION
//
// The reduction between %3 and (%d)[10] will create a new reduction If, but
// it needs to be updated to include the reduction between %2 and (%c)[7]. This
// reduction If needs be prepared for ORing (see prepareReductionIfForORing for
// more details) first, which results in the following:
//
//     %cmp = %tmp != 0;
//     if (%cmp != 0)
//     {
//        %3 = (%d)[10];
//        (%d)[10] = %3 + %tmp;
//     }
//
// Then, a new compare instruction will be created that checks if the temp for
// storing the result for %2 is not 0. An OR will be created to check if the
// previous left DDRef of the compare inside the reduction If (%cmp) is true
// or if the new compare instruction is true. The new OR will replace this
// DDRef in the reduction If. The new condition will look as follows:
//
//     %cmp = %tmp != 0;
//     %cmp8 = %tmp6 != 0;
//     %or = %cmp  |  %cmp8;
//     if (%or != 0)
//     {
//        %2 = (%c)[7];
//        (%c)[7] = %2 + %tmp6;
//        %3 = (%d)[10];
//        (%d)[10] = %3 + %tmp;
//     }
//
// The final result will look as follows at the end of the transformation:
//
//   BEGIN REGION { modified }
//            %tmp = 0;
//            %tmp6 = 0;
//            %tmp9 = 0;
//         + DO i1 = 0, 99, 1   <DO_LOOP>
//         |   if ((%b)[i1] > 10)
//         |   {
//         |      %tmp9 = %tmp9  +  2;
//         |      %tmp6 = %tmp6  +  4;
//         |      %tmp = %tmp  +  8;
//         |   }
//         + END LOOP
//         %cmp = %tmp != 0;
//         %cmp8 = %tmp6 != 0;
//         %or = %cmp  |  %cmp8;
//         %cmp11 = %tmp9 != 0;
//         %or12 = %or  |  %cmp11;
//         if (%or12 != 0)
//         {
//            %1 = (%a)[5];
//            (%a)[5] = %1 + %tmp9;
//            %2 = (%c)[7];
//            (%c)[7] = %2 + %tmp6;
//            %3 = (%d)[10];
//            (%d)[10] = %3 + %tmp;
//         }
//   END REGION
//
void ReductionIfInfo::updateReductionIf(RegDDRef *LoadRef, HLInst *InitInst) {

  // Prepare the condition for ORing if it is the first time that is going to
  // be ORed
  if (!ReductionIfCanBeORed)
    prepareReductionIfForORing();

  auto &HNU = ReductionIf->getHLNodeUtils();

  // Create the new compare instruction (%tmp2 != 0)
  auto PredTy = LoadRef->getDestType()->isIntegerTy() ? PredicateTy::ICMP_NE
                                                      : PredicateTy::FCMP_UNE;

  auto *RednTemp = InitInst->getLvalDDRef();
  auto *TempInitVal = InitInst->getRvalDDRef();

  // Create the new compare and insert it
  auto *LHS = RednTemp->clone();
  auto *RHS = TempInitVal->clone();
  auto *NewCmp = HNU.createCmp(PredTy, LHS, RHS);
  HLNodeUtils::insertBefore(ReductionIf, NewCmp);

  // Create the new OR with the new operands
  auto PredBegin = ReductionIf->pred_begin();
  RegDDRef *ORLHS = ReductionIf->removeLHSPredicateOperandDDRef(PredBegin);
  RegDDRef *ORRHS = NewCmp->getLvalDDRef()->clone();
  auto *ORInst = HNU.createOr(ORLHS, ORRHS);
  HLNodeUtils::insertBefore(ReductionIf, ORInst);

  // Replace the LHS of the predicate with the new OR created
  ReductionIf->setLHSPredicateOperandDDRef(ORInst->getLvalDDRef()->clone(),
                                           PredBegin);
}

// Return true if the current reduction If can be reused. This condition can be
// reused if the two input reductions are always executed together inside the
// same condition branch/case.
static bool reductionIfCanBeReused(HLDDNode *PrevLoadInst,
                                   HLDDNode *CurrLoadInst) {
  return HLNodeUtils::dominates(PrevLoadInst, CurrLoadInst) &&
         HLNodeUtils::postDominates(CurrLoadInst, PrevLoadInst);
}

void HIRMemoryReductionSinking::sinkInvariantReductions(HLLoop *Lp) {

  if (!ConditionalInvariantMemoryReductions.empty())
    Lp->extractZttPreheaderAndPostexit();

  // Sink collected reduction in reverse order to keep lexical order same in
  // loop postexit.
  for (auto &InvRedn : make_range(InvariantMemoryReductions.rbegin(),
                                  InvariantMemoryReductions.rend())) {
    createReductionTemp(Lp, InvRedn);
    auto *LoadInst = cast<HLInst>(InvRedn.LoadRef->getHLDDNode());
    auto *StoreInst = cast<HLInst>(InvRedn.StoreRef->getHLDDNode());

    HLNodeUtils::moveAsFirstPostexitNode(Lp, StoreInst);
    HLNodeUtils::moveAsFirstPostexitNode(Lp, LoadInst);

    makeLoadAndStoreRefsConsistent(Lp, InvRedn);
    addOptimizationsRemark(Lp, InvRedn);
  }

  ReductionIfInfo RedIfInfo;
  bool ReductionCanBeReused = false;
  for (auto RedIt = ConditionalInvariantMemoryReductions.rbegin(),
            E = ConditionalInvariantMemoryReductions.rend();
       RedIt != E; ++RedIt) {

    HLInst *InitInst = createReductionTemp(Lp, *RedIt);

    auto *LoadRef = RedIt->LoadRef;
    auto *LoadInst = LoadRef->getHLDDNode();
    auto *StoreInst = RedIt->StoreRef->getHLDDNode();

    // Add the current reduction under the same reduction If, or create a new
    // one, using the result precomputed.
    if (!ReductionCanBeReused) {
      HLIf *NewRedIf = createConditionalReductionIf(LoadRef, InitInst);
      RedIfInfo.resetReductionIfInfo(NewRedIf);
      HLNodeUtils::insertAfter(Lp, NewRedIf);
    } else {
      RedIfInfo.updateReductionIf(LoadRef, InitInst);
    }

    // Precompute if the previous reduction can be added in the reduction if
    // used by the current reduction. We consider the next entry in the
    // traversal as the previous instruction since InvariantMemoryReductions is
    // being traversed in reverse. This needs to be done before moving the
    // current load outside of the loop since the check depends on the
    // HLNodeUtils domination analyses.
    ReductionCanBeReused = false;
    if ((RedIt + 1) != ConditionalInvariantMemoryReductions.rend()) {
      auto *PrevLoadRef = (RedIt + 1)->LoadRef;
      auto *PrevLoadInst = PrevLoadRef->getHLDDNode();
      ReductionCanBeReused = reductionIfCanBeReused(PrevLoadInst, LoadInst);
    }

    HLIf *RedIf = RedIfInfo.getReductionIf();

    HLNodeUtils::moveAsFirstThenChild(RedIf, StoreInst);
    HLNodeUtils::moveAsFirstThenChild(RedIf, LoadInst);

    makeLoadAndStoreRefsConsistent(Lp, *RedIt);
    addOptimizationsRemark(Lp, *RedIt);
  }

  Lp->getParentRegion()->setGenCode();
}

bool HIRMemoryReductionSinking::run(HLLoop *Lp) {

  if (Lp->getNumExits() > 1) {
    return false;
  }

  if (Lp->isSIMD()) {
    return false;
  }

  const LoopStatistics &LS = HLS.getSelfStatistics(Lp);

  if (LS.hasCallsWithUnsafeSideEffects()) {
    return false;
  }

  clearReductions();

  CandidatesCollector Candidates(MemoryReductions, InvariantMemoryReductions,
                                 ConditionalInvariantMemoryReductions, Lp);

  HLNodeUtils::visitRange(Candidates, Lp->child_begin(), Lp->child_end());

  if (!haveCandidateReductions())
    return false;

  if (!validateMemoryReductions(Lp)) {
    return false;
  }

  sinkInvariantReductions(Lp);

  // We don't invalidate the loop statistics analysis if new conditions weren't
  // generated.
  HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(Lp);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(Lp);

  return true;
}

static bool runMemoryReductionSinking(HIRFramework &HIRF,
                                      HIRLoopStatistics &HLS,
                                      HIRDDAnalysis &HDDA,
                                      bool AllowConditionalReductionSinking) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << "HIR Memory Reduction Sinking Disabled \n");
    return false;
  }

  SmallVector<HLLoop *, 64> CandidateLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(CandidateLoops);

  HIRMemoryReductionSinking MRS(HLS, HDDA, AllowConditionalReductionSinking);

  bool Result = false;

  for (auto *Lp : CandidateLoops) {
    Result = MRS.run(Lp) || Result;
  }

  return Result;
}

PreservedAnalyses HIRMemoryReductionSinkingPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  ModifiedHIR = runMemoryReductionSinking(
      HIRF, AM.getResult<HIRLoopStatisticsAnalysis>(F),
      AM.getResult<HIRDDAnalysisPass>(F), AllowConditionalReductionSinking);
  return PreservedAnalyses::all();
}
