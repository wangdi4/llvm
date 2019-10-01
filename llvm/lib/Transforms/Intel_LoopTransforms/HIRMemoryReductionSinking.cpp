//===-------------------- HIRMemoryReductionSinking.cpp -------------------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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
#include "llvm/Transforms/Intel_LoopTransforms/HIRMemoryReductionSinking.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#define OPT_SWITCH "hir-memory-reduction-sinking"
#define OPT_DESC "HIR Memory Reduction Sinking"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

namespace {

class HIRMemoryReductionSinkingLegacyPass : public HIRTransformPass {
public:
  static char ID;
  HIRMemoryReductionSinkingLegacyPass() : HIRTransformPass(ID) {
    initializeHIRMemoryReductionSinkingLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.setPreservesAll();
  }
};

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
  RegDDRef *LoadRef;
  RegDDRef *StoreRef;

  MemoryReductionInfo(unsigned Opcode, RegDDRef *LoadRef, RegDDRef *StoreRef)
      : Opcode(Opcode), LoadRef(LoadRef), StoreRef(StoreRef) {}
};

class HIRMemoryReductionSinking {
  HIRLoopStatistics &HLS;
  HIRDDAnalysis &HDDA;

  SmallVector<MemoryReductionInfo, 16> MemoryReductions;
  SmallVector<MemoryReductionInfo, 8> InvariantMemoryReductions;

public:
  HIRMemoryReductionSinking(HIRLoopStatistics &HLS, HIRDDAnalysis &HDDA)
      : HLS(HLS), HDDA(HDDA) {}
  bool run(HLLoop *Lp);

  bool collectMemoryReductions(HLLoop *Lp);

  bool validateReductionTemp(DDGraph DDG);
  bool validateMemoryReductions(const HLLoop *Lp);

  void sinkInvariantReductions(HLLoop *Lp);
};

} // namespace

char HIRMemoryReductionSinkingLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRMemoryReductionSinkingLegacyPass, OPT_SWITCH, OPT_DESC,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRMemoryReductionSinkingLegacyPass, OPT_SWITCH, OPT_DESC,
                    false, false)

FunctionPass *llvm::createHIRMemoryReductionSinkingPass() {
  return new HIRMemoryReductionSinkingLegacyPass();
}

static HLInst *getReductionStore(RegDDRef *LoadRef, HLInst *LoadInst) {
  // We only handle consecutive load and store instructions when looking for
  // reductions.
  // TODO: Extend logic to handle non-consecutive loads and stores.
  auto *SInst = dyn_cast_or_null<HLInst>(LoadInst->getNextNode());

  if (!SInst || !isa<StoreInst>(SInst->getLLVMInstruction())) {
    return nullptr;
  }

  auto *StoreRef = SInst->getLvalDDRef();
  if (!DDRefUtils::areEqual(LoadRef, StoreRef)) {
    return nullptr;
  }

  auto *StoreRvalRef = SInst->getRvalDDRef();

  if (!StoreRvalRef->isTerminalRef()) {
    return nullptr;
  }

  unsigned TempIndex = LoadInst->getLvalDDRef()->getSelfBlobIndex();
  if (!StoreRvalRef->getSingleCanonExpr()->containsStandAloneBlob(TempIndex,
                                                                  false)) {
    return nullptr;
  }

  return SInst;
}

bool HIRMemoryReductionSinking::collectMemoryReductions(HLLoop *Lp) {
  unsigned Level = Lp->getNestingLevel();
  auto *FirstChild = Lp->getFirstChild();

  // Goes through top-level loop nodes and collects candidates which structually
  // look like memory reductions. DD based legality is checked later.
  for (auto &Node : make_range(Lp->child_begin(), Lp->child_end())) {
    auto *HInst = dyn_cast<HLInst>(&Node);

    if (!HInst) {
      continue;
    }

    auto *LLVMInst = HInst->getLLVMInstruction();
    unsigned Opcode;
    RegDDRef *LoadRef = nullptr;
    RegDDRef *AlternateLoadRef = nullptr;
    HLInst *StoreInst = nullptr;

    if (isa<LoadInst>(LLVMInst)) {
      // Looking for this pattern for integer types-
      //    %0 = A[5];
      //    A[5] = %0 + t;
      LoadRef = HInst->getRvalDDRef();

      if (!LoadRef->getDestType()->isIntegerTy()) {
        continue;
      }

      Opcode = Instruction::Add;

    } else if (isa<BinaryOperator>(LLVMInst) && HInst->isReductionOp(&Opcode)) {
      // Looking for this pattern-
      //   %add = A[5]  +  %t;
      //   A[5] = %add;
      auto *FPOp = dyn_cast<FPMathOperator>(LLVMInst);

      if (FPOp && !FPOp->isFast()) {
        continue;
      }

      LoadRef = HInst->getOperandDDRef(1);

      // Try both memref operands of commutative operations.
      if (!LoadRef->isMemRef() && Instruction::isCommutative(Opcode)) {
        AlternateLoadRef = HInst->getOperandDDRef(2);

        if (!AlternateLoadRef->isMemRef()) {
          AlternateLoadRef = nullptr;
        }
      }

    } else {
      continue;
    }

    StoreInst = getReductionStore(LoadRef, HInst);

    if (!StoreInst && AlternateLoadRef) {
      LoadRef = AlternateLoadRef;
      StoreInst = getReductionStore(LoadRef, HInst);
    }

    if (!StoreInst || !HLNodeUtils::postDominates(StoreInst, FirstChild)) {
      continue;
    }

    if (LoadRef->isStructurallyInvariantAtLevel(Level)) {
      InvariantMemoryReductions.emplace_back(Opcode, LoadRef,
                                             StoreInst->getLvalDDRef());
    } else {
      MemoryReductions.emplace_back(Opcode, LoadRef, StoreInst->getLvalDDRef());
    }
  }

  return !InvariantMemoryReductions.empty();
}

bool HIRMemoryReductionSinking::validateReductionTemp(DDGraph DDG) {
  // Checks whether the lval temp of the reduction is used elsewhere in the
  // loop. If so, invalidates the reduction.

  auto ReductionTempEscapes = [&](MemoryReductionInfo &RednInfo) {
    auto *LvalTempRef = RednInfo.LoadRef->getHLDDNode()->getLvalDDRef();
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

  MemoryReductions.erase(std::remove_if(MemoryReductions.begin(),
                                        MemoryReductions.end(),
                                        ReductionTempEscapes),
                         MemoryReductions.end());

  return !InvariantMemoryReductions.empty();
}

static bool
isValidReductionRef(const DDRef *Ref, unsigned Opcode,
                    const SmallVectorImpl<MemoryReductionInfo> &MemReductions) {
  bool CheckLval = Ref->isLval();

  for (auto &OtherRedn : MemReductions) {
    if (Opcode != OtherRedn.Opcode) {
      continue;
    }

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

bool HIRMemoryReductionSinking::validateMemoryReductions(const HLLoop *Lp) {
  DDGraph DDG = HDDA.getGraph(Lp);

  if (!validateReductionTemp(DDG)) {
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

      // Give up if there is a possibiliy of partial overlap between dependent
      // refs. Partial overlap is not possible between refs of same type, same
      // alignment and alignment >= size.
      if ((StoreTy != SinkRef->getDestType()) ||
          (StoreAlignment != cast<RegDDRef>(SinkRef)->getAlignment()) ||
          (StoreAlignment < StoreSize)) {
        Escapes = true;
        break;
      }

      if (!isValidReductionRef(SinkRef, InvRedn.Opcode, MemoryReductions) &&
          !isValidReductionRef(SinkRef, InvRedn.Opcode,
                               InvariantMemoryReductions)) {
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

  return !InvariantMemoryReductions.empty();
}

static RegDDRef *createReductionInitializer(HLLoop *Lp, unsigned Opcode,
                                            Type *Ty) {
  // Creates reduction initialization and inserts it in the loop preheader-
  // %tmp = <identity constant>

  auto &HNU = Lp->getHLNodeUtils();
  auto *Const = HLInst::getRecurrenceIdentity(Opcode, Ty);

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

void HIRMemoryReductionSinking::sinkInvariantReductions(HLLoop *Lp) {
  auto &HNU = Lp->getHLNodeUtils();
  unsigned Level = Lp->getNestingLevel();

  // Sink collected reduction in reverse order to keep lexical order same in
  // loop postexit.
  for (auto RedIt = InvariantMemoryReductions.rbegin(),
            E = InvariantMemoryReductions.rend();
       RedIt != E; ++RedIt) {
    auto *LoadRef = RedIt->LoadRef;
    auto *LoadInst = cast<HLInst>(LoadRef->getHLDDNode());

    auto *StoreRef = RedIt->StoreRef;
    auto *StoreInst = StoreRef->getHLDDNode();

    unsigned Opcode = RedIt->Opcode;

    auto *RednTemp =
        createReductionInitializer(Lp, Opcode, LoadRef->getDestType());

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
      NewStoreRval->makeConsistent({RednTemp, LoadTemp});
    }

    auto *TmpRednInst = HNU.createBinaryHLInst(
        Opcode, RednTemp->clone(), RednOpRef, "redn", RednTemp->clone(), BinOp);

    HLNodeUtils::insertBefore(LoadInst, TmpRednInst);

    // Move original reduction instructions to postexit.
    HLNodeUtils::moveAsFirstPostexitNode(Lp, StoreInst);
    HLNodeUtils::moveAsFirstPostexitNode(Lp, LoadInst);

    // Linear (invariant) memrefs can become non-linear in post-exit as they are
    // now in outer loop scope.
    LoadRef->makeConsistent({}, Level - 1);
    StoreRef->makeConsistent({}, Level - 1);
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

  const LoopStatistics &LS = HLS.getSelfLoopStatistics(Lp);

  if (LS.hasCallsWithUnsafeSideEffects()) {
    return false;
  }

  MemoryReductions.clear();
  InvariantMemoryReductions.clear();

  if (!collectMemoryReductions(Lp)) {
    return false;
  }

  if (!validateMemoryReductions(Lp)) {
    return false;
  }

  HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(Lp);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<HIRLoopStatistics>(Lp);

  sinkInvariantReductions(Lp);
  return true;
}

static bool runMemoryReductionSinking(HIRFramework &HIRF,
                                      HIRLoopStatistics &HLS,
                                      HIRDDAnalysis &HDDA) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << "HIR Memory Reduction Sinking Disabled \n");
    return false;
  }

  SmallVector<HLLoop *, 64> CandidateLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(CandidateLoops);

  HIRMemoryReductionSinking MRS(HLS, HDDA);

  bool Result = false;

  for (auto *Lp : CandidateLoops) {
    Result = MRS.run(Lp) || Result;
  }

  return Result;
}

bool HIRMemoryReductionSinkingLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    return false;
  }

  bool Result = runMemoryReductionSinking(
      getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
      getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS(),
      getAnalysis<HIRDDAnalysisWrapperPass>().getDDA());
  return Result;
}

PreservedAnalyses
HIRMemoryReductionSinkingPass::run(llvm::Function &F,
                                   llvm::FunctionAnalysisManager &AM) {
  runMemoryReductionSinking(AM.getResult<HIRFrameworkAnalysis>(F),
                            AM.getResult<HIRLoopStatisticsAnalysis>(F),
                            AM.getResult<HIRDDAnalysisPass>(F));
  return PreservedAnalyses::all();
}
