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
// NOTE: This pass should be the first pass in HIR. It has implicit assumptions
// about the temps it tries to clean up.
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
  // Stores the lexically last use of liveout copy that was substituted. This is
  // needed for correctly updating loop liveout set.
  RegDDRef *UseRef;

  // Stores liveout copy's uses in inner loops. They can only be substituted
  // once we determine that the rval has not been redefined in the inner loop.
  SmallVector<RegDDRef *, 8> InnerLoopUses;

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

  HLLoop *getLoop() const { return getDefInst()->getLexicalParentLoop(); }

  const RegDDRef *getDDRef() const { return DefInst->getLvalDDRef(); }
  const RegDDRef *getRvalDDRef() const { return DefInst->getRvalDDRef(); }

  bool isLoad() const { return isa<LoadInst>(DefInst->getLLVMInstruction()); }

  bool isSubstitutable() const { return IsSubstitutable; }
  void markNonSubstitutable() { IsSubstitutable = false; }

  bool isValid() const { return IsValid; }
  void markInvalid() { IsValid = false; }

  RegDDRef *getUseRef() const {
    assert(isLoad() && "Attempt to access use ref for copy temp!");
    return UseRef;
  }

  void setUseRef(RegDDRef *Ref) {
    assert(isLoad() && "Attempt to set use ref for copy temp!");
    UseRef = Ref;
  }

  // Substitutes temp in the stored use ref.
  void substituteInUseRef();

  RegDDRef *getLastUseRef() const {
    assert(!isLoad() && "Attempt to access last use of load temp!");
    return UseRef;
  }

  void setLastUseRef(RegDDRef *Ref) {
    assert(!isLoad() && "Attempt to set last use of load temp!");
    UseRef = Ref;
  }

  HLLoop *getLastUseLoop() const {
    return getLastUseRef() ? getLastUseRef()->getLexicalParentLoop() : nullptr;
  }

  // Replaces liveout temp by it copy. Returns true if substitution was
  // performed.
  bool substituteInRef(RegDDRef *Ref);

  void addInnerLoopUse(RegDDRef *Use) {
    assert(!isLoad() && "Attempt to add inner loop uses for load temp!");
    InnerLoopUses.push_back(Use);
  }

  // Substitutes or invalidates temp for the stored inner loop uses. \p
  // InvalidatingLoop indicates the loopnest where temp is invalid.
  void processInnerLoopUses(HLLoop *InvalidatingLoop);

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
    return DefInst->getBlobUtils().getTempBlobSymbase(getRvalBlobIndex());
  }
};

void TempInfo::substituteInUseRef() {
  assert(isLoad() && "Attempt to access use ref for copy temp!");

  if (auto UseRef = getUseRef()) {
    auto UseNode = UseRef->getHLDDNode();

    UseNode->replaceOperandDDRef(UseRef, getDefInst()->removeRvalDDRef());

    auto LvalRef = UseNode->getLvalDDRef();

    // Rval blob could have been propagated to lval temp by parser. Since we now
    // have a load in the rval, we need to make lval as a self blob. For
    // example, consider this case-
    // t1 = A[i];
    // t2 = t1;   // livein copy,
    // Here t2's canonical form is 1 * t1. After substitution the instruction
    // becomes t2 = A[i]. t2 can no longer be in terms of t1. It should be
    // marked as a self-blob.
    if (LvalRef && LvalRef->isTerminalRef() &&
        LvalRef->usesTempBlob(getBlobIndex())) {
      LvalRef->makeSelfBlob();
    }
  }
}

bool TempInfo::substituteInRef(RegDDRef *Ref) {
  assert(!isLoad() && "Invalid for load temps!");
  return Ref->replaceTempBlob(getBlobIndex(), getRvalBlobIndex());
}

void TempInfo::processInnerLoopUses(HLLoop *InvalidatingLoop) {
  assert(!isLoad() && "Attempt to add inner loop uses for load temp!");

  if (InnerLoopUses.empty()) {
    return;
  }

  RegDDRef *CurLastUse = getLastUseRef();
  RegDDRef *LastInnerUse = nullptr;

  if (isSubstitutable() || !InvalidatingLoop) {
    for (auto UseRef : InnerLoopUses) {
      substituteInRef(UseRef);
    }

    LastInnerUse = InnerLoopUses.back();

  } else {
    // The inner loop use can be substituted if the lowest common ancestor loop
    // of the use loop and InvalidatingLoop is the same as temp's loop.
    //
    // For example, in the following loopnest redefinition of t1 in i3 loop
    // invalidates both inner loop use 2 and 3 but inner loop use 1 can still be
    // substituted.
    // DO i1
    //   t.out = t1
    //   DO i2
    //      = t.out // inner use 1
    //   END DO
    //
    //   DO i2
    //      = t.out // inner use 2
    //     DO i3
    //         = t.out // inner use 3
    //      t1 =  // redefinition of t1 (t.out's rval)
    //     END DO
    //   END DO
    // END DO

    auto TempLoop = getLoop();

    for (auto UseRef : InnerLoopUses) {
      auto LCALoop = HLNodeUtils::getLowestCommonAncestorLoop(
          InvalidatingLoop, UseRef->getLexicalParentLoop());

      if (LCALoop == TempLoop) {
        substituteInRef(UseRef);
        LastInnerUse = UseRef;

      } else {
        markInvalid();
      }
    }
  }

  // Update the lexical last use.
  if (LastInnerUse &&
      (!CurLastUse || (LastInnerUse->getHLDDNode()->getTopSortNum() >
                       CurLastUse->getHLDDNode()->getTopSortNum()))) {
    setLastUseRef(LastInnerUse);
  }

  InnerLoopUses.clear();
}

// Visitor class which populates/updates/substitutes temps.
class TempSubstituter final : public HLNodeVisitorBase {
  HIRFramework *HIRF;
  SmallVector<TempInfo, 32> CandidateTemps;
  bool SIMDDirSeen;
  bool HasEmptyNodes;

private:
  // Returns true if the parent node is empty due to node removal.
  bool isNodeEmpty(HLNode *Parent) const;

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

public:
  TempSubstituter(HIRFramework *HIRF)
      : HIRF(HIRF), SIMDDirSeen(false), HasEmptyNodes(false) {}

  /// Adds/updates temp candidates.
  void visit(HLInst *Inst);
  /// Processes node by performing substitution and/or invalidating candidate
  /// temps.
  void visit(HLDDNode *Node);

  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}

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
            isa<HLLoop>(Node) ||
            (Node->getLexicalParentLoop() != Temp.getLoop())) {
          Temp.markInvalid();

        } else {
          Temp.setUseRef(Ref);
        }
      } else {

        if (!Temp.isSubstitutable()) {
          Temp.markInvalid();

        } else {
          HLLoop *ParentLoop;

          if ((ParentLoop = Node->getParentLoop()) &&
              !Node->getHLNodeUtils().contains(ParentLoop, Temp.getDefInst())) {
            // Inner loop uses are handled when either the temp is marked as
            // non-substitutable or after traversing the region.
            Temp.addInnerLoopUse(Ref);
            continue;
          }

          auto Ret = Temp.substituteInRef(Ref);
          (void)Ret;
          assert(Ret && "Temp blob was not replaced!");

          // Store as last use ref to update liveouts correctly.
          Temp.setLastUseRef(Ref);

          // Blob could have been propagated to the temp lval by parser. Replace
          // it there as well.
          if (NodeLvalRef) {
            Temp.substituteInRef(NodeLvalRef);
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
    LvalBlobIndex =
        LvalRef->isSelfBlob()
            ? LvalRef->getSelfBlobIndex()
            : HInst->getBlobUtils().findTempBlobIndex(LvalRef->getSymbase());
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

      if (!Temp.isLoad()) {

        Temp.processInnerLoopUses(HInst->getLexicalParentLoop());

        // Add rval temp as loop liveout based on performed substitutions.
        auto TempRef = Temp.getDDRef();

        if (auto LastUseLoop = Temp.getLastUseLoop()) {
          auto TempLoop = Temp.getLoop();
          auto LCALoop =
              HLNodeUtils::getLowestCommonAncestorLoop(LastUseLoop, TempLoop);

          auto NewSymbase = Temp.getRvalSymbase();

          while (TempLoop != LCALoop) {
            TempLoop->addLiveOutTemp(NewSymbase);
            TempLoop = TempLoop->getParentLoop();
          }
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

  if (HInst->getRvalDDRef()->isVolatile()) {
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

bool TempSubstituter::isNodeEmpty(HLNode *Node) const {
  if (auto Loop = dyn_cast<HLLoop>(Node)) {
    return !Loop->hasChildren();

  } else if (auto If = dyn_cast<HLIf>(Node)) {
    return (!If->hasThenChildren() && !If->hasElseChildren());
  }

  // No-op for empty region.
  // I don't think switches can become empty due to temp removal.
  return false;
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

      // Load temp is substituted once we have traversed the entire region and
      // determined that it has a single use.
      Temp.substituteInUseRef();

    } else {

      Temp.processInnerLoopUses(nullptr);

      unsigned Symbase = Temp.getSymbase();
      unsigned NewSymbase = Temp.getRvalSymbase();
      HLLoop *ParentLoop = Temp.getLoop();
      HLLoop *LCALoop = nullptr;
      bool SkipLiveouts = false;

      // Temp may have been substituted in some places. We need to replace it in
      // loop liveouts by its rval symbase.
      if (Reg->isLiveOut(Symbase)) {
        assert(Temp.isSubstitutable() && "Temp is live out of region and "
                                         "non-subtitutable but wat not marked "
                                         "invalid!");
        // LCALoop is null in this path as the region liveout temp should be
        // replaced as loop liveout in all the parent loops.
        Reg->replaceLiveOutTemp(Symbase, NewSymbase);

      } else if (!Temp.getLastUseRef()) {
        // Under rare cases, it is possible for SSA deconstruction to create
        // copies which do not have any uses. We should skip liveout processing
        // for them.
        SkipLiveouts = true;

      } else if (auto LastUseLoop = Temp.getLastUseLoop()) {
        // If the last use was outside the outermost loop, LastUseLoop will be
        // null.
        LCALoop =
            HLNodeUtils::getLowestCommonAncestorLoop(LastUseLoop, ParentLoop);
      }

      if (!SkipLiveouts) {
        while (ParentLoop != LCALoop) {
          ParentLoop->replaceLiveOutTemp(Symbase, NewSymbase);
          ParentLoop = ParentLoop->getParentLoop();
        }
      }
    }

    auto Parent = Temp.getDefInst()->getParent();

    // Temp is deemed unnecessary.
    Reg->getHLNodeUtils().remove(Temp.getDefInst());

    if (isNodeEmpty(Parent)) {
      HasEmptyNodes = true;
    }
  }

  CandidateTemps.clear();
}

void TempSubstituter::substituteTemps(HLRegion *Reg) {
  HLNodeUtils::visitRange(*this, Reg->child_begin(), Reg->child_end());
  eliminateSubstitutedTemps(Reg);

  // Parents can become recursively empty when we remove nodes so it is better
  // to scan the whole region.
  if (HasEmptyNodes) {
    HLNodeUtils::removeEmptyNodes(Reg);
  }

  // Restore flags.
  SIMDDirSeen = false;
  HasEmptyNodes = false;
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
