//===- HIRPropagateCastedIV.cpp - Implements PropagateCastedIV class
//------------===//
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
// This pass propagate casted IV in the memory references.
//
// We are transforming this-
//
//  Do i1 = 0, N
//    %t2 = zext.i32.i64(i1 + 2);
//    % t = (%t1)[t2 + t3];
//  END Do
//
// To-
//
//  %ptr = &(%t1)[t3];
//  Do i1 = 0, N
//    %t = (%ptr)[i1 + 2];
//  END DO
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRPropagateCastedIV.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/ADT/SmallSet.h"

#include "HIRPropagateCastedIVImpl.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#define OPT_SWITCH "hir-propagate-casted-iv"
#define OPT_DESC "HIR Propagate Casted IV"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

struct CollectMemRefs final : public HLNodeVisitorBase {
  HLLoop *Lp;
  SmallVectorImpl<RegDDRef *> &MemRefs;
  unsigned &CandidateBlobIndex;
  RegDDRef *&CandidateRef;
  bool &CanDeleteCandidateInst;
  HLNode *LastChild;

public:
  CollectMemRefs(HLLoop *Lp, SmallVectorImpl<RegDDRef *> &MemRefs,
                 unsigned &CandidateBlobIndex, RegDDRef *&CandidateRef,
                 bool &CanDeleteCandidateInst)
      : Lp(Lp), MemRefs(MemRefs), CandidateBlobIndex(CandidateBlobIndex),
        CandidateRef(CandidateRef),
        CanDeleteCandidateInst(CanDeleteCandidateInst),
        LastChild(Lp->getLastChild()) {
    assert(Lp && "Loop is null!\n");
  }

  void visit(HLDDNode *Node);
  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node){};

  void getCandidateBlobIndex(HLDDNode *Node);
};

void CollectMemRefs::visit(HLDDNode *Node) {
  if (CandidateBlobIndex == InvalidBlobIndex) {
    getCandidateBlobIndex(Node);
    return;
  }

  bool FoundFirstUseRef = false;

  for (auto It = Node->ddref_begin(), ItE = Node->ddref_end(); It != ItE;
       ++It) {
    RegDDRef *Ref = (*It);

    if (!Ref->usesTempBlob(CandidateBlobIndex)) {
      continue;
    }

    if (!Ref->isMemRef() || Ref->isFake() ||
        Ref->getBitCastDestType() != nullptr) {
      CanDeleteCandidateInst = false;
      continue;
    }

    if (!Ref->isSingleCanonExpr()) {
      CanDeleteCandidateInst = false;
      continue;
    }

    bool IsValid = true;

    for (auto BIt = Ref->blob_begin(), End = Ref->blob_end(); BIt != End;
         ++BIt) {
      auto Blob = *BIt;
      if (Blob->getBlobIndex() != CandidateBlobIndex && Blob->isNonLinear()) {
        IsValid = false;
        break;
      }
    }

    if (!IsValid) {
      CanDeleteCandidateInst = false;
      continue;
    }

    CanonExpr *CE = Ref->getSingleCanonExpr();

    unsigned LoopLevel = Lp->getNestingLevel();

    if (CE->hasIV(LoopLevel)) {
      CanDeleteCandidateInst = false;
      continue;
    }

    if (!CE->containsStandAloneBlob(CandidateBlobIndex, false)) {
      CanDeleteCandidateInst = false;
      continue;
    }

    if (!FoundFirstUseRef) {
      if (Node != LastChild && !HLNodeUtils::dominates(Node, LastChild)) {
        CanDeleteCandidateInst = false;
        return;
      }
      FoundFirstUseRef = true;
    }

    MemRefs.push_back(Ref);
  }
}

void CollectMemRefs::getCandidateBlobIndex(HLDDNode *Node) {
  RegDDRef *LRef = Node->getLvalDDRef();

  if (!LRef || !LRef->isTerminalRef()) {
    return;
  }

  unsigned SB = LRef->getSymbase();

  if (Lp->isLiveIn(SB)) {
    return;
  }

  CanonExpr *CE = LRef->getSingleCanonExpr();
  unsigned LoopLevel = Lp->getNestingLevel();

  if (!CE->hasIV(LoopLevel) || CE->getSrcType() == CE->getDestType()) {
    return;
  }

  if (Node != LastChild && !HLNodeUtils::dominates(Node, LastChild)) {
    return;
  }

  CandidateBlobIndex = LRef->getBlobUtils().findTempBlobIndex(SB);

  // Temp does not have blob index if there are no uses inside the region
  if (CandidateBlobIndex == InvalidBlobIndex) {
    return;
  }

  if (Lp->isLiveOut(SB)) {
    CanDeleteCandidateInst = false;
  }

  CandidateRef = LRef;
}

bool HIRPropagateCastedIV::doCollection(HLLoop *Lp,
                                        SmallVectorImpl<RegDDRef *> &MemRefs,
                                        unsigned &CandidateBlobIndex,
                                        RegDDRef *&CandidateRef,
                                        bool &CanDeleteCandidateInst) {
  CollectMemRefs Collector(Lp, MemRefs, CandidateBlobIndex, CandidateRef,
                           CanDeleteCandidateInst);
  HLNodeUtils::visitRange(Collector, Lp->getFirstChild(), Lp->getLastChild());
  return !MemRefs.empty();
}

// In the test case-
//    BEGIN REGION { }
//       + DO i1 = 0, -1 * %t108 + %t23 + -1, 1   <DO_MULTI_EXIT_LOOP>
//           |   %t115 = zext.i32.i64(i1 + %t108);
//           |   %t118 = (%t27)[%t112 + %t115];
//           |   %t119 = (%t27)[i1 + %t108];
//           |   if (%t118 != %t119)
//           |   {
//           |      goto t124;
//           |   }
//        + END LOOP
//    END REGION
//
// Candidate-
// %t115 = zext.i32.i64(i1 + %t108);
//
// UseRef-
// (%t27)[%t112 + %t115];
//
// We are forming this in loop preheader-
// %ptr = &((%t27)[%t112]);
bool HIRPropagateCastedIV::propagateCastedIV(HLLoop *Lp) {
  SmallVector<RegDDRef *, 16> MemRefs;
  unsigned CandidateBlobIndex = InvalidBlobIndex;
  RegDDRef *CandidateRef = nullptr;
  bool CanDeleteCandidateInst = true;

  if (!doCollection(Lp, MemRefs, CandidateBlobIndex, CandidateRef,
                    CanDeleteCandidateInst)) {
    return false;
  }

  unsigned LoopLevel = Lp->getNestingLevel();

  auto &HNU = HIRF.getHLNodeUtils();
  auto &DRU = HNU.getDDRefUtils();
  CanonExpr *CandidateCE = CandidateRef->getSingleCanonExpr();
  const SmallVector<const RegDDRef *, 1> Aux = {CandidateRef};

  for (auto UseRef : MemRefs) {
    ArrayRef<unsigned> OffsetsRef = UseRef->getTrailingStructOffsets(1);
    SmallVector<unsigned, 8> Offsets(OffsetsRef.begin(), OffsetsRef.end());

    HLDDNode *UseNode = UseRef->getHLDDNode();
    unsigned OpNum = UseNode->getOperandNum(UseRef);
    UseNode->removeOperandDDRef(OpNum);

    UseRef->setAddressOf(true);
    UseRef->setInBounds(false);

    UseRef->getSingleCanonExpr()->removeBlob(CandidateBlobIndex);
    UseRef->removeTrailingStructOffsets(1);
    UseRef->makeConsistent({}, LoopLevel - 1);

    // Insert the new inst as a last node into Loop's Prehdr
    HLInst *PrehdrInst = HNU.createCopyInst(UseRef, "ptr");
    HLNodeUtils::insertAsLastPreheaderNode(Lp, PrehdrInst);

    // Mark as Loop's LiveIn Temp
    RegDDRef *LvalRef = PrehdrInst->getLvalDDRef();
    Lp->addLiveInTemp(LvalRef->getSymbase());

    // Modified ref example-
    // %t118 = (%ptr)[i1 + %t108]
    RegDDRef *NewRef = DRU.createMemRef(LvalRef->getSelfBlobIndex(),
                                        LoopLevel - 1, UseRef->getSymbase());

    NewRef->addDimension(CandidateCE->clone(), Offsets);
    NewRef->makeConsistent(Aux, LoopLevel);
    UseNode->setOperandDDRef(NewRef, OpNum);
  }

  if (CanDeleteCandidateInst) {
    HLNodeUtils::remove(CandidateRef->getHLDDNode());
  }

  // Mark the loop and its parent loop/region have been changed
  HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(Lp);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<HIRLoopStatistics>(Lp);

  return true;
}

bool HIRPropagateCastedIV::run() {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << "HIR Propagate Casted IV Disabled \n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIR Propagate Casted IV on Function : "
                    << HIRF.getFunction().getName() << "\n");

  // Gather all inner-most Loop Candidates
  SmallVector<HLLoop *, 64> CandidateLoops;

  HLNodeUtils &HNU = HIRF.getHLNodeUtils();
  HNU.gatherInnermostLoops(CandidateLoops);

  if (CandidateLoops.empty()) {
    LLVM_DEBUG(dbgs() << HIRF.getFunction().getName()
                      << "() has no inner-most loop\n ");
    return false;
  }
  // LLVM_DEBUG(dbgs() << " # Innermost Loops: " << CandidateLoops.size() <<
  // "\n");

  bool Result = false;

  for (auto &Lp : CandidateLoops) {
    Result = propagateCastedIV(Lp) || Result;
  }

  return Result;
}

PreservedAnalyses
HIRPropagateCastedIVPass::run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &AM) {
  HIRPropagateCastedIV(AM.getResult<HIRFrameworkAnalysis>(F)).run();
  return PreservedAnalyses::all();
}

class HIRPropagateCastedIVLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRPropagateCastedIVLegacyPass() : HIRTransformPass(ID) {
    initializeHIRPropagateCastedIVLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRPropagateCastedIV(getAnalysis<HIRFrameworkWrapperPass>().getHIR())
        .run();
  }
};

char HIRPropagateCastedIVLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRPropagateCastedIVLegacyPass, OPT_SWITCH, OPT_DESC,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_END(HIRPropagateCastedIVLegacyPass, OPT_SWITCH, OPT_DESC, false,
                    false)

FunctionPass *llvm::createHIRPropagateCastedIVPass() {
  return new HIRPropagateCastedIVLegacyPass();
}
