//===------- HIRTempCleanup.cpp - Implements Temp Cleanup pass ------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// This pass cleans up unnecessary temps from the HIR. These include uselss
// liveout copies created by the framework and single use load instructions.
//
// Liveout copy example-
//
// Before cleanup-
//
// DO i1
//   t1.out = t1
//   A[i] = t1.out
//   t1 = B[i]
// END DO
//
// After cleanup-
//
// DO i1
//   A[i] = t1
//   t1 = B[i]
// END DO
//
//
// Load temp example-
//
// Before cleanup-
//
// DO i1
//   t = A[i1]
//   B[i1] = t
// END DO
//
// After cleanup-
//
// DO i1
//   B[i1] = A[i1]
// END DO
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "hir-temp-cleanup"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    DisableHIRTempCleanup("disable-hir-temp-cleanup", cl::init(false),
                          cl::Hidden, cl::desc("Disable HIR temp cleanup"));

namespace {

/// Structure representing temp candidates which can be substituted.
struct TempInfo {
private:
  HLInst *DefInst;
  // Stores single use site of load temps.
  RegDDRef *UseRef;
  // Indicates whether this temp can be legally substituted.
  bool IsSubstitutable;
  // Indicates discarded temps. This is so that we don't have to erase them from
  // the container.
  bool IsValid;

public:
  TempInfo(HLInst *DefInst)
      : DefInst(DefInst), UseRef(nullptr), IsSubstitutable(true),
        IsValid(true) {}

  HLInst *getDefInst() const { return DefInst; }

  const RegDDRef *getDDRef() const { return DefInst->getLvalDDRef(); }
  const RegDDRef *getRvalDDRef() const { return DefInst->getRvalDDRef(); }

  bool isLoad() const { return isa<LoadInst>(DefInst->getLLVMInstruction()); }

  bool isSubstitutable() const { return IsSubstitutable; }
  void markNonSubstitutable() { IsSubstitutable = false; }

  bool isValid() const { return IsValid; }
  void markInvalid() { IsValid = false; }

  RegDDRef *getUseRef() {
    assert(isLoad() && "Attempt to access use ref for copy temp!");
    return UseRef;
  }

  void setUseRef(RegDDRef *Ref) {
    assert(isLoad() && "Attempt to set use ref for copy temp!");
    UseRef = Ref;
  }

  unsigned getBlobIndex() const { return getDDRef()->getSelfBlobIndex(); }
  unsigned getRvalBlobIndex() const {
    assert(!isLoad() && "Attempt to access load temp's rval blob index!");
    auto RvalRef = getRvalDDRef();
    return RvalRef->isAddressOf() ? RvalRef->getBaseCE()->getSingleBlobIndex()
                                  : RvalRef->getSelfBlobIndex();
  }

  unsigned getSymbase() const { return getDDRef()->getSymbase(); }
  unsigned getRvalSymbase() const {
    assert(!isLoad() && "Attempt to access load temp's rval symbase!");
    return BlobUtils::getTempBlobSymbase(getRvalBlobIndex());
  }
};

// Visitor class which populates/updates/substitutes temps.
class TempSubstituter final : public HLNodeVisitorBase {
  HIRFramework *HIRF;
  SmallVector<TempInfo, 32> CandidateTemps;
  const HLNode *SkipNode;
  bool SIMDDirSeen;

public:
  TempSubstituter(HIRFramework *HIRF)
      : HIRF(HIRF), SkipNode(nullptr), SIMDDirSeen(false) {}

  /// Adds/updates temp candidates.
  void visit(HLInst *Inst);
  /// Processes node by performing substitution and/or invalidating candidate
  /// temps.
  void visit(HLDDNode *Node);

  // Be conservative in presence of complicated control flow.
  // TODO: Is it worth refining using domination check?
  void visit(HLGoto *Goto) { CandidateTemps.clear(); };
  void visit(HLLabel *Label) { CandidateTemps.clear(); };
  void visit(HLSwitch *Switch) {
    CandidateTemps.clear();
    SkipNode = Switch;
  }

  void visit(HLNode *Node) { llvm_unreachable("Unexpected HLNode type!"); }
  void postVisit(HLNode *Node) {}

  bool skipRecursion(const HLNode *Node) const override {
    return Node == SkipNode;
  }

  /// Returns true if the instruction is either of the form t1 = t2 (where both
  /// lval/rval are self blobs) or t1 = &t2[0] (for pointer types).
  bool isLiveoutCopy(HLInst *HInst) const;

  /// Returns true if the instruction is of the form t1 = A[i] and t1 is not
  /// loop/region liveout.
  bool isLoad(HLInst *HInst) const;

  /// Makes temp candidates non-substitutable based on whether HInst invalidates
  /// their definition. For example, a liveout copy t1 = t2 becomes
  /// non-substitutable when t2 is redefined. Load temps also become
  /// non-substitutable on encountering an instruction which can write to
  /// memory.
  void updateTempCandidates(HLInst *HInst);

  /// Eliminates temps which have been successfully substituted.
  void eliminateSubstitutedTemps(HLRegion *Reg);

  /// Main driver function to find and substitutes unnecessary temps.
  void substituteTemps(HLRegion *Reg);
};

// Wrapper pass.
class HIRTempCleanup : public HIRTransformPass {
public:
  static char ID;

  HIRTempCleanup() : HIRTransformPass(ID) {
    initializeHIRTempCleanupPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFramework>();
  }
};
}

char HIRTempCleanup::ID = 0;
INITIALIZE_PASS_BEGIN(HIRTempCleanup, "hir-temp-cleanup", "HIR Temp Cleanup",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_END(HIRTempCleanup, "hir-temp-cleanup", "HIR Temp Cleanup",
                    false, false)

FunctionPass *llvm::createHIRTempCleanupPass() { return new HIRTempCleanup(); }

void TempSubstituter::visit(HLDDNode *Node) {
  RegDDRef *NodeLvalRef = nullptr;

  for (auto RefIt = Node->ddref_begin(), E = Node->ddref_end(); RefIt != E;
       ++RefIt) {
    auto Ref = *RefIt;

    // Ignore uses in temp lvals for bookkeeping purposes. We can modify them
    // along with the rval.
    if (Ref->isLval() && Ref->isTerminalRef()) {
      NodeLvalRef = Ref;
      continue;
    }

    for (auto &Temp : CandidateTemps) {
      if (!Temp.isValid()) {
        continue;
      }

      unsigned TempIndex = Temp.getBlobIndex();

      bool IsSelfBlob = false;
      if (!Ref->usesTempBlob(TempIndex, &IsSelfBlob)) {
        continue;
      }

      if (Temp.isLoad()) {
        // Cannot subtitute load if-
        // 1) There is more than one use, OR
        // 2) Temp is not substitutable, OR
        // 3) The use is not at the top level (embedded inside the canon expr
        // tree), OR
        // 4) Node is a HLLoop.
        // 5) The use is inside a different loop.
        if (Temp.getUseRef() || !Temp.isSubstitutable() || !IsSelfBlob ||
            isa<HLLoop>(Node) || (Node->getLexicalParentLoop() !=
                                  Temp.getDefInst()->getLexicalParentLoop())) {
          Temp.markInvalid();

        } else {
          Temp.setUseRef(Ref);
        }
      } else {
        HLLoop *ParentLoop;

        if (!Temp.isSubstitutable() ||
            ((ParentLoop = Node->getParentLoop()) &&
             // TODO: Extend to handle inner loop uses.
             !HLNodeUtils::contains(ParentLoop, Temp.getDefInst()))) {
          Temp.markInvalid();

        } else {
          unsigned NewIndex = Temp.getRvalBlobIndex();
          auto Ret = Ref->replaceTempBlob(TempIndex, NewIndex);

          (void)Ret;
          assert(Ret && "Temp blob was not replaced!");

          // Blob could have been propagated to the temp lval by parser. Replace
          // it there as well.
          if (NodeLvalRef) {
            NodeLvalRef->replaceTempBlob(TempIndex, NewIndex);
          }
        }
      }
      // Self blob has been fully processed.
      if (IsSelfBlob) {
        break;
      }
    }
  }
}

void TempSubstituter::updateTempCandidates(HLInst *HInst) {

  bool InvalidateLoads = false;
  auto Inst = HInst->getLLVMInstruction();
  unsigned LvalBlobIndex = InvalidBlobIndex;

  if (Inst->mayWriteToMemory()) {
    InvalidateLoads = true;
  }

  RegDDRef *LvalRef = HInst->getLvalDDRef();

  if (LvalRef && LvalRef->isTerminalRef()) {
    LvalBlobIndex = LvalRef->isSelfBlob()
                        ? LvalRef->getSelfBlobIndex()
                        : BlobUtils::findTempBlobIndex(LvalRef->getSymbase());
  }

  if (!InvalidateLoads && (LvalBlobIndex == InvalidBlobIndex)) {
    return;
  }

  for (auto &Temp : CandidateTemps) {

    if (!Temp.isValid()) {
      continue;
    }

    if (InvalidateLoads && Temp.isLoad()) {
      Temp.markNonSubstitutable();
      continue;
    }

    if ((LvalBlobIndex != InvalidBlobIndex) &&
        Temp.getRvalDDRef()->usesTempBlob(LvalBlobIndex)) {
      Temp.markNonSubstitutable();

      auto CurLoop = HInst->getLexicalParentLoop();
      assert(CurLoop && "SCC value found outside loop!");

      // Update loop liveouts.
      if (!Temp.isLoad()) {
        auto CurTempLoop = Temp.getDefInst()->getLexicalParentLoop();
        auto LCALoop =
            HLNodeUtils::getLowestCommonAncestorLoop(CurLoop, CurTempLoop);

        auto TempRef = Temp.getDDRef();
        auto OldSymbase = Temp.getSymbase();
        auto NewSymbase = Temp.getRvalSymbase();

        while ((CurTempLoop != LCALoop) && CurTempLoop->isLiveOut(OldSymbase)) {
          CurTempLoop->replaceLiveOutTemp(OldSymbase, NewSymbase);
        }

        // Temp is used outside region so it cannot be eliminated.
        if (TempRef->isLiveOutOfRegion()) {
          Temp.markInvalid();
        }
      }
    }
  }
}

bool TempSubstituter::isLiveoutCopy(HLInst *HInst) const {
  if (!HIRF->isLiveoutCopy(HInst)) {
    return false;
  }
  assert(HInst->getLexicalParentLoop() &&
         "Liveout copy doesn't have a parent loop!");

  assert(HInst->getLvalDDRef()->isSelfBlob() &&
         "Lval of liveout copy is not a self blob!");

  RegDDRef *RvalRef = HInst->getRvalDDRef();
  if (!RvalRef->isSelfBlob() &&
      !(RvalRef->isAddressOf() && (RvalRef->getNumDimensions() == 1) &&
        (*RvalRef->canon_begin())->isZero())) {
    return false;
  }

  return true;
}

bool TempSubstituter::isLoad(HLInst *HInst) const {
  if (!isa<LoadInst>(HInst->getLLVMInstruction())) {
    return false;
  }

  RegDDRef *LvalRef = HInst->getLvalDDRef();

  if ((HInst->getLexicalParentLoop() && LvalRef->isLiveOutOfParentLoop()) ||
      LvalRef->isLiveOutOfRegion()) {
    return false;
  }

  return true;
}

void TempSubstituter::visit(HLInst *HInst) {
  if (HInst->isSIMDDirective()) {
    SIMDDirSeen = true;
  }

  // 1. Visit the DDRefs of the instruction for substitution opportunity.
  visit(cast<HLDDNode>(HInst));

  // 2. Update existing candidates.
  updateTempCandidates(HInst);

  // 3. Add new candidate.
  if (isLiveoutCopy(HInst) || isLoad(HInst)) {
    CandidateTemps.push_back(TempInfo(HInst));
  }
}

void TempSubstituter::eliminateSubstitutedTemps(HLRegion *Reg) {
  for (auto &Temp : CandidateTemps) {
    if (!Temp.isValid()) {
      continue;
    }

    if (Temp.isLoad()) {
      // Suppress temp substitution for regions with simd loops as explicit
      // reduction recognition fails otherwise. This is a workaround until
      // we can change explicit reduction recognition to work without relying
      // on underlying LLVM IR. Since temp cleanup is run currently at start
      // of HIR framework pass, we only bail out for explicit SIMD cases.
      if (SIMDDirSeen) {
        continue;
      }

      // Load temp is subtituted once we have traversed the entire region and
      // determined that it has a single use.
      auto UseRef = Temp.getUseRef();

      if (UseRef) {
        auto UseNode = UseRef->getHLDDNode();

        UseNode->replaceOperandDDRef(UseRef,
                                     Temp.getDefInst()->removeRvalDDRef());

        auto LvalRef = UseNode->getLvalDDRef();

        // Rval blob could have been propagated to lval temp by parser. Since we
        // now have a load in the rval, we need to make lval as a self blob. For
        // example, consider this case-
        // t1 = A[i];
        // t2 = t1   // livein copy,
        // Here t2's canonical form is 1 * t1. After substitution the
        // instruction becomes t2 = A[i]. t2 can no longer be in terms of t1. It
        // should be marked as a self-blob.
        if (LvalRef && LvalRef->isTerminalRef() &&
            LvalRef->usesTempBlob(Temp.getBlobIndex())) {
          LvalRef->makeSelfBlob();
        }
      }
    } else {
      if (Temp.isSubstitutable()) {
        // Update region/loop liveouts.
        unsigned OldSymbase = Temp.getSymbase();
        unsigned NewSymbase = Temp.getRvalSymbase();

        HLLoop *ParentLoop = Temp.getDefInst()->getLexicalParentLoop();

        while (ParentLoop && ParentLoop->isLiveOut(OldSymbase)) {
          ParentLoop->replaceLiveOutTemp(OldSymbase, NewSymbase);
          ParentLoop = ParentLoop->getParentLoop();
        }

        if (Reg->isLiveOut(OldSymbase)) {
          Reg->replaceLiveOutTemp(OldSymbase, NewSymbase);
        }
      }
    }

    // Temp is deemed unnecessary.
    HLNodeUtils::remove(Temp.getDefInst());
  }

  CandidateTemps.clear();
}

void TempSubstituter::substituteTemps(HLRegion *Reg) {
  HLNodeUtils::visitRange(*this, Reg->child_begin(), Reg->child_end());
  eliminateSubstitutedTemps(Reg);
}

bool HIRTempCleanup::runOnFunction(Function &F) {
  if (DisableHIRTempCleanup || skipFunction(F)) {
    DEBUG(dbgs() << "HIR Temp Cleanup Disabled \n");
    return false;
  }

  auto HIRF = &getAnalysis<HIRFramework>();
  TempSubstituter TS(HIRF);

  for (auto RegIt = HIRF->hir_begin(), End = HIRF->hir_end(); RegIt != End;
       ++RegIt) {
    TS.substituteTemps(cast<HLRegion>(&*RegIt));
  }

  return false;
}
