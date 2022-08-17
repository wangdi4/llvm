//===------- HIRTempCleanup.cpp - Implements Temp Cleanup pass ------------===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
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

#include "llvm/Transforms/Intel_LoopTransforms/HIRTempCleanupPass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
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

  // Keeps track of single definition of the rval which makes the temp
  // unsubstitutible. This only applies to liveout copies. In some cases, we can
  // reorder instructions and still substitute the temp.
  HLInst *SingleRvalDefInst;

  HLLoop *DefLoop;

  // Stores single use site of load temps.
  // Stores the lexically last use of liveout copy that was substituted. This is
  // needed for correctly updating loop liveout set.
  RegDDRef *UseRef;

  // Stores liveout copy's uses in inner loops. They can only be substituted
  // once we determine that the rval has not been redefined in the inner loop.
  SmallVector<RegDDRef *, 8> InnerLoopUses;

  // Indicates whether this temp can be legally substituted. Temps are usually
  // first made non-substitutable and then marked invalid. This allows for more
  // substitution as uses of temp before it was marked non-substitutble can
  // still be processed. For example a single use load temp whose use was
  // discovered before it was marked non-substitutable can still be eliminated.
  // TODO: Add a better decription of substitutable/valid for both load and
  // liveout copy temps.
  bool IsSubstitutable;

  // Indicates discarded temps. This is so that we don't have to erase them from
  // the container.
  bool IsValid;

public:
  TempInfo(HLInst *DefInst)
      : DefInst(DefInst), SingleRvalDefInst(nullptr),
        DefLoop(DefInst->getLexicalParentLoop()), UseRef(nullptr),
        IsSubstitutable(true), IsValid(true) {}

  HLInst *getDefInst() const { return DefInst; }

  HLInst *getSingleRvalDefInst() const { return SingleRvalDefInst; }
  bool hasSingleRvalDefInst() const { return getSingleRvalDefInst(); }

  bool setSingleRvalDefInst(HLInst *RvalDefInst);

  HLLoop *getDefLoop() const { return DefLoop; }

  const RegDDRef *getDDRef() const { return DefInst->getLvalDDRef(); }
  const RegDDRef *getRvalDDRef() const { return DefInst->getRvalDDRef(); }

  bool isLoad() const { return isa<LoadInst>(DefInst->getLLVMInstruction()); }

  bool isSubstitutable() const { return IsSubstitutable; }
  void markNonSubstitutable() { IsSubstitutable = false; }

  bool isValid() const { return IsValid; }
  void markInvalid() { IsValid = false; }

  void markNonSubstitutableOrInvalid(HLInst *InvalidatingInst) {
    assert(isLoad() && "Attempt to access use ref for copy temp!");

    if (UseRef && (UseRef->getHLDDNode()->getTopSortNum() >
                   InvalidatingInst->getTopSortNum())) {
      // Use of load temp was in a liveout copy temp's RvalDefInst and was
      // reordered so the old use is no longer valid.
      markInvalid();
    } else {
      markNonSubstitutable();
    }
  }

  RegDDRef *getUseRef() const {
    assert(isLoad() && "Attempt to access use ref for copy temp!");
    return UseRef;
  }

  void setUseRef(RegDDRef *Ref) {
    assert(isLoad() && "Attempt to set use ref for copy temp!");
    UseRef = Ref;
  }

  /// Returns true if substitution is invalidated due to reordering of rval's
  /// def inst.
  bool isInvalidatedByReordering(SmallPtrSetImpl<HLInst *> &MovedRvalDefs,
                                 HLDDNode *UseNode);

  // Substitutes temp in the stored use ref.
  void substituteInUseRef(SmallPtrSetImpl<HLInst *> &MovedRvalDefs);

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

  // Replaces liveout temp by its copy in the Node represented by \p UseRef. It
  // is also replaced in the lval temp ref of the node, if applicable.
  void substituteInUseNode(RegDDRef *UseRef);

  void addInnerLoopUse(RegDDRef *Use) {
    assert(!isLoad() && "Attempt to add inner loop uses for load temp!");
    InnerLoopUses.push_back(Use);
  }

  // Substitutes or invalidates temp for the stored inner loop uses. \p
  // InvalidatingLoop indicates the loopnest where temp is invalid.
  void processInnerLoopUses(HLLoop *InvalidatingLoop);

  // Tries to move \p UseNode before single rval definition of temp so temp can
  // be substituted. Returns true/false based on whether it is successful in
  // moving the node.
  bool movedUseBeforeRvalDef(HLDDNode *UseNode);

  // Tries to move single rval definition of temp after \p UseNode so temp can
  // be substituted. Returns true/false based on whether it is successful in
  // moving the node.
  bool movedRvalDefAfterUse(HLDDNode *UseNode);
};

bool TempInfo::setSingleRvalDefInst(HLInst *RvalDefInst) {
  assert(!isLoad() && "Attempt to add rval definition for load temp!");

  // Temp is already non-substitutible.
  if (!isSubstitutable()) {
    return false;
  }

  if (SingleRvalDefInst) {
    // Give up on reordering instructions if we found more than one definition
    // or rval.
    SingleRvalDefInst = nullptr;
    return false;
  }

  auto *ParLp = getDefLoop();

  // Do not handle loop liveout temps.
  if (ParLp->isLiveOut(getSymbase())) {
    return false;
  }

  // Restrict to innermost loops for now.
  if (!ParLp->isInnermost() || (RvalDefInst->getLexicalParentLoop() != ParLp)) {
    return false;
  }

  SingleRvalDefInst = RvalDefInst;
  return true;
}

// Skip the substitution if the reordering of rval's def inst prevents
// substitution.
// This is a separate check because visitor misses checking rval def insts
// in their new location in HIR if they are reordered.
//
// For example, in the case below t2 load temp is substitutable before
// reordering but not afterwards-
//
// t1.out = t1
// t1 = t1 + 1
// t2 = A[t1.out]
// t3 = t2
//
// t2 = A[t1]
// t1 = t1 + 1
// t3 = t2
bool TempInfo::isInvalidatedByReordering(
    SmallPtrSetImpl<HLInst *> &MovedRvalDefs, HLDDNode *UseNode) {

  if (MovedRvalDefs.empty()) {
    return false;
  }

  unsigned DefTSNum = DefInst->getTopSortNum();
  unsigned UseTSNum = UseNode->getTopSortNum();

  for (auto *RvalDef : MovedRvalDefs) {
    unsigned RvalDefTSNum = RvalDef->getTopSortNum();

    if (RvalDefTSNum > DefTSNum && RvalDefTSNum < UseTSNum) {
      unsigned LvalBlobIndex = RvalDef->getLvalBlobIndex();

      if (DefInst->usesTempBlob(LvalBlobIndex)) {
        return true;
      }
    }
  }

  return false;
}

void TempInfo::substituteInUseRef(SmallPtrSetImpl<HLInst *> &MovedRvalDefs) {
  assert(isLoad() && "Attempt to access use ref for copy temp!");

  if (auto UseRef = getUseRef()) {
    auto UseNode = UseRef->getHLDDNode();
    auto UseInst = dyn_cast<HLInst>(UseNode);
    RegDDRef *LvalRef = nullptr;
    unsigned Index = getBlobIndex();

    if (isInvalidatedByReordering(MovedRvalDefs, UseNode)) {
      return;
    }

    // If use is in the copy, replace use inst by def inst by changing its lval.
    // This is so that we don't add memref to copy insts.
    if (UseInst && UseInst->isCopyInst()) {
      LvalRef = UseInst->removeLvalDDRef();
      DefInst->replaceOperandDDRef(DefInst->getLvalDDRef(), LvalRef);
      HLNodeUtils::moveBefore(UseInst, DefInst);
      HLNodeUtils::remove(UseInst);

    } else {
      UseNode->replaceOperandDDRef(UseRef, DefInst->removeRvalDDRef());
      HLNodeUtils::remove(DefInst);
      LvalRef = UseNode->getLvalDDRef();
    }

    // Rval blob could have been propagated to lval temp by parser. Since we now
    // have a load in the rval, we need to make lval as a self blob. For
    // example, consider this case-
    // t1 = A[i];
    // t2 = t1;   // livein copy,
    // Here t2's canonical form is 1 * t1. After substitution the instruction
    // becomes t2 = A[i]. t2 can no longer be in terms of t1. It should be
    // marked as a self-blob.
    if (LvalRef && LvalRef->isTerminalRef() && LvalRef->usesTempBlob(Index)) {
      LvalRef->makeSelfBlob();
    }

  } else {
    // No uses, we can simply remove the instruction.
    HLNodeUtils::remove(DefInst);
  }
}

void TempInfo::substituteInUseNode(RegDDRef *UseRef) {
  assert(!isLoad() && "Invalid for load temps!");
  assert((!UseRef->isTerminalRef() || !UseRef->isLval()) &&
         "terminal lval ref not expected!");

  unsigned LvalBlobIndex = getBlobIndex();
  unsigned RvalBlobIndex = getRvalBlobIndex();

  auto Ret = UseRef->replaceTempBlob(LvalBlobIndex, RvalBlobIndex);
  (void)Ret;
  assert(Ret && "Temp blob was not replaced!");

  // Sometimes blob can get cancelled with substitution.
  // Refer to blob-cancellation-with-substitution.ll
  UseRef->makeConsistent();

  auto *UseNode = UseRef->getHLDDNode();
  auto *LvalRef = UseNode->getLvalDDRef();

  // Blob could have been propagated to the temp lval by parser. Replace
  // it there as well.
  if (LvalRef && LvalRef->isTerminalRef()) {
    unsigned UseLvalBlobIndex =
        LvalRef->getBlobUtils().findTempBlobIndex(LvalRef->getSymbase());

    // Do not allow recursive definitions like the following as they don't make
    // sense:
    //
    // t1 = smax(t1, t2);
    // <LVAL-REG>: NON-LINEAR i32 smax(t1, t2)
    //
    // Instead we make t1 a self blob.
    if (UseLvalBlobIndex == RvalBlobIndex) {
      LvalRef->makeSelfBlob();

    } else {
      LvalRef->replaceTempBlob(LvalBlobIndex, RvalBlobIndex);
      LvalRef->makeConsistent();
    }
  }

  // Replace lval symbase by rval symbase as livein.
  auto *DefLoop = getDefLoop();
  auto *UseLoop = isa<HLLoop>(UseNode) ? cast<HLLoop>(UseNode)
                                       : UseRef->getLexicalParentLoop();
  unsigned RvalSymbase = getRvalSymbase();
  unsigned LvalSymbase = getSymbase();

  auto LCALoop = HLNodeUtils::getLowestCommonAncestorLoop(DefLoop, UseLoop);

  while (UseLoop != LCALoop) {
    UseLoop->addLiveInTemp(RvalSymbase);
    UseLoop->removeLiveInTemp(LvalSymbase);
    UseLoop = UseLoop->getParentLoop();
  }
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
      substituteInUseNode(UseRef);
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

    HLLoop *DefLoop = getDefLoop();

    for (auto UseRef : InnerLoopUses) {

      auto *UseNode = UseRef->getHLDDNode();
      HLLoop *UseLoop = isa<HLLoop>(UseNode) ? cast<HLLoop>(UseNode)
                                             : UseNode->getLexicalParentLoop();

      if (HLNodeUtils::getLowestCommonAncestorLoop(InvalidatingLoop, UseLoop) ==
          DefLoop) {
        substituteInUseNode(UseRef);
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

static bool areInSameParent(HLInst *Inst1, HLInst *Inst2) {
  auto *Parent = Inst1->getParent();
  if (Parent != Inst2->getParent()) {
    return false;
  }

  // Only handle cases where both insts are in the same case of If/Switch
  // parent.
  if (auto *IfParent = dyn_cast<HLIf>(Parent)) {
    return IfParent->isThenChild(Inst1) == IfParent->isThenChild(Inst2);
  }

  if (auto *SwitchParent = dyn_cast<HLSwitch>(Parent)) {
    return SwitchParent->getChildCaseNum(Inst1) ==
           SwitchParent->getChildCaseNum(Inst2);
  }
  return true;
}

static bool canMoveNode(HLDDNode *Node) {
  auto *Inst = dyn_cast<HLInst>(Node);

  if (!Inst || Inst->isCallInst()) {
    return false;
  }

  // Do not move memrefs.
  for (auto *Ref : make_range(Inst->op_ddref_begin(), Inst->op_ddref_end())) {
    if (Ref->isMemRef()) {
      return false;
    }
  }

  return true;
}

bool TempInfo::movedRvalDefAfterUse(HLDDNode *UseNode) {
  assert(hasSingleRvalDefInst() && "Temp does not contain rval def!");

  auto *RvalDefInst = getSingleRvalDefInst();

  auto *UseInst = dyn_cast<HLInst>(UseNode);

  if (!UseInst) {
    return false;
  }

  if (!canMoveNode(RvalDefInst)) {
    return false;
  }

  if (!areInSameParent(RvalDefInst, UseInst)) {
    return false;
  }

  // It is possible that RvalDefInst was already moved before UseInst
  // while processing another RvalDefInst.
  // If so, we can trivially return true here.
  if (UseInst->getTopSortNum() < RvalDefInst->getTopSortNum()) {
    return true;
  }

  unsigned DefInstLvalBlobIndex = RvalDefInst->getLvalBlobIndex();

  // Check if intermediate nodes prevent reordering.
  for (auto *NextNode = RvalDefInst->getNextNode(),
            *EndNode = UseInst->getNextNode();
       NextNode != EndNode; NextNode = NextNode->getNextNode()) {

    auto *Inst = dyn_cast<HLInst>(NextNode);

    // Do not move across non-insts.
    if (!Inst) {
      return false;
    }

    // Illegal to move if lval of RvalDefInst is used by an intermediate inst.
    if (Inst->usesTempBlob(DefInstLvalBlobIndex)) {
      return false;
    }

    unsigned LvalBlobIndex = Inst->getLvalBlobIndex();

    // Illegal to move if lval of an intermediate inst is used by RvalDefInst.
    if (RvalDefInst->usesTempBlob(LvalBlobIndex)) {
      return false;
    }
  }

  HLNodeUtils::moveAfter(UseInst, RvalDefInst);
  return true;
}

bool TempInfo::movedUseBeforeRvalDef(HLDDNode *UseNode) {
  assert(hasSingleRvalDefInst() && "Temp does not contain rval def!");

  if (!canMoveNode(UseNode)) {
    return false;
  }

  auto *UseInst = cast<HLInst>(UseNode);
  auto *RvalDefInst = getSingleRvalDefInst();

  if (!areInSameParent(RvalDefInst, UseInst)) {
    return false;
  }

  // It is possible that UseInst was already moved before RvalDefInst
  // while processing another use of temp in UseInst.
  // If so, we can trivially return true here.
  if (UseInst->getTopSortNum() < RvalDefInst->getTopSortNum()) {
    return true;
  }

  unsigned UseLvalBlobIndex = UseInst->getLvalBlobIndex();

  // Check if intermediate nodes prevent reordering.
  for (auto *PrevNode = UseInst->getPrevNode(),
            *EndNode = RvalDefInst->getPrevNode();
       PrevNode != EndNode; PrevNode = PrevNode->getPrevNode()) {

    auto *Inst = dyn_cast<HLInst>(PrevNode);

    // Do not move across non-insts.
    if (!Inst) {
      return false;
    }

    // Illegal to move if lval of UseInst is used by an intermediate inst.
    if (Inst->usesTempBlob(UseLvalBlobIndex)) {
      return false;
    }

    unsigned LvalBlobIndex = Inst->getLvalBlobIndex();

    // Illegal to move if lval of an intermediate inst is used by UseInst.
    if (UseInst->usesTempBlob(LvalBlobIndex)) {
      return false;
    }
  }

  HLNodeUtils::moveBefore(RvalDefInst, UseInst);
  return true;
}

// Visitor class which populates/updates/substitutes temps.
class TempSubstituter final : public HLNodeVisitorBase {
  HIRFramework *HIRF;
  SmallPtrSet<HLInst *, 8> MovedRvalDefs;
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

  /// Processes a use found for liveout temp \p Temp.
  void processLiveoutTempUse(TempInfo &Temp, RegDDRef *UseRef);

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
class HIRTempCleanupLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRTempCleanupLegacyPass() : HIRTransformPass(ID) {
    initializeHIRTempCleanupLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
  }
};
} // namespace

char HIRTempCleanupLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRTempCleanupLegacyPass, "hir-temp-cleanup",
                      "HIR Temp Cleanup", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_END(HIRTempCleanupLegacyPass, "hir-temp-cleanup",
                    "HIR Temp Cleanup", false, false)

FunctionPass *llvm::createHIRTempCleanupPass() {
  return new HIRTempCleanupLegacyPass();
}

void TempSubstituter::processLiveoutTempUse(TempInfo &Temp, RegDDRef *UseRef) {

  if (!Temp.isSubstitutable()) {
    Temp.markInvalid();
    return;
  }

  auto *Node = UseRef->getHLDDNode();
  if (Temp.hasSingleRvalDefInst()) {
    // If use cannot be moved before single rval definition of temp, we can
    // try if single rval definition of temp can be moved after use.
    // If reordering is unsuccessful, make the temp invalid.
    if (!Temp.movedUseBeforeRvalDef(Node)) {
      bool Moved = Temp.movedRvalDefAfterUse(Node);

      if (Moved) {
        MovedRvalDefs.insert(Temp.getSingleRvalDefInst());

      } else {
        Temp.markInvalid();
        return;
      }
    }

  } else {
    HLLoop *ParentLoop;

    if (isa<HLLoop>(Node) ||
        ((ParentLoop = Node->getLexicalParentLoop()) &&
         !Node->getHLNodeUtils().contains(ParentLoop, Temp.getDefInst()))) {
      // Inner loop uses are handled when either the temp is marked as
      // non-substitutable or after traversing the region.
      Temp.addInnerLoopUse(UseRef);
      return;
    }
  }

  Temp.substituteInUseNode(UseRef);

  // Store as last use ref to update liveouts correctly.
  Temp.setLastUseRef(UseRef);
}

void TempSubstituter::visit(HLDDNode *Node) {

  for (auto RefIt = Node->ddref_begin(), E = Node->ddref_end(); RefIt != E;
       ++RefIt) {
    auto Ref = *RefIt;

    // Ignore uses in temp lvals for bookkeeping purposes. We can modify them
    // along with the rval.
    if (Ref->isLval() && Ref->isTerminalRef()) {
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
        // Loaded pointer values may also be substituted the same as
        // self blobs if they represent a self value not an expression.
        //   %16 = (%4)[4 * i1];
        //   &((%16)[0]) -> (%4)[4 * i1]
        //
        // Temp may be referring to stride of the SelfAddressOf ref so we need
        // to check the base pointer index to be sure.
        bool IsSelfPointer =
            Ref->isSelfAddressOf() && (Ref->getBasePtrBlobIndex() == TempIndex);

        bool UseIsIndirectCallPtr = false;
        if (auto *Inst = dyn_cast<HLInst>(Node)) {
          if (Inst->isIndirectCallInst() &&
              (Ref == Inst->getIndirectCallPtr())) {
            UseIsIndirectCallPtr = true;
          }
        }
        // Cannot subtitute load if-
        // 1) There is more than one use, OR
        // 2) Temp is not substitutable, OR
        // 3) The use is not at the top level (embedded inside the canon expr
        // tree), OR
        // 4) Node is a HLLoop, OR
        // 5) The use is inside a different loop, OR
        // 6) Use is the function pointer operand of indirect call.
        if (Temp.getUseRef() || !Temp.isSubstitutable() ||
            !(IsSelfBlob || IsSelfPointer) || isa<HLLoop>(Node) ||
            (Node->getLexicalParentLoop() != Temp.getDefLoop()) ||
            UseIsIndirectCallPtr) {
          Temp.markInvalid();

        } else {
          Temp.setUseRef(Ref);
        }
      } else {
        processLiveoutTempUse(Temp, Ref);
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

  if (Inst->mayWriteToMemory()) {
    InvalidateLoads = true;
  }

  unsigned LvalBlobIndex = HInst->getLvalBlobIndex();

  if (!InvalidateLoads && (LvalBlobIndex == InvalidBlobIndex)) {
    return;
  }

  for (auto &Temp : CandidateTemps) {

    if (!Temp.isValid()) {
      continue;
    }

    if (InvalidateLoads && Temp.isLoad()) {
      Temp.markNonSubstitutableOrInvalid(HInst);
      continue;
    }

    if ((LvalBlobIndex != InvalidBlobIndex) &&
        Temp.getRvalDDRef()->usesTempBlob(LvalBlobIndex)) {

      if (Temp.isLoad()) {
        Temp.markNonSubstitutableOrInvalid(HInst);

        // This call gives us the chance to substitute by reordering
        // instructions.
      } else if (!Temp.setSingleRvalDefInst(HInst)) {

        Temp.markNonSubstitutable();
        Temp.processInnerLoopUses(HInst->getLexicalParentLoop());

        // Add rval temp as loop liveout based on performed substitutions.
        auto TempRef = Temp.getDDRef();

        if (auto LastUseLoop = Temp.getLastUseLoop()) {
          auto TempLoop = Temp.getDefLoop();
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
  if (!RvalRef->isSelfBlob() && !RvalRef->isSelfAddressOf()) {
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

  if (auto *ParLoop = HInst->getParentLoop()) {
    // Bail out on loops with distribute point as we do not track whether we are
    // crossing a distribute point due to substitution. For example-
    //
    // t1 = A[i];
    // t2 = C[i];  <distribute_point>
    //
    // B[i] = t1;  << Cannot substitute here.
    //             << We need to perform scalar expansion for t1.
    if (ParLoop->hasDistributePoint()) {
      return false;
    }
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

    auto Parent = Temp.getDefInst()->getParent();

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
      Temp.substituteInUseRef(MovedRvalDefs);

    } else {

      Temp.processInnerLoopUses(nullptr);

      unsigned Symbase = Temp.getSymbase();
      unsigned NewSymbase = Temp.getRvalSymbase();
      HLLoop *ParentLoop = Temp.getDefLoop();
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

      // Temp is deemed unnecessary.
      HLNodeUtils::remove(Temp.getDefInst());
    }

    if (isNodeEmpty(Parent)) {
      HasEmptyNodes = true;
    }
  }

  CandidateTemps.clear();
}

void TempSubstituter::substituteTemps(HLRegion *Reg) {
  MovedRvalDefs.clear();

  HLNodeUtils::visitRange(*this, Reg->child_begin(), Reg->child_end());
  eliminateSubstitutedTemps(Reg);

  // Parents can become recursively empty when we remove nodes so it is better
  // to scan the whole region.
  if (HasEmptyNodes) {
    HLNodeUtils::removeEmptyNodes(Reg, false);
  }

  // Restore flags.
  SIMDDirSeen = false;
  HasEmptyNodes = false;
}

static void runTempCleanup(HIRFramework &HIRF) {
  if (DisableHIRTempCleanup) {
    LLVM_DEBUG(dbgs() << "HIR Temp Cleanup Disabled \n");
    return;
  }

  TempSubstituter TS(&HIRF);

  for (auto RegIt = HIRF.hir_begin(), End = HIRF.hir_end(); RegIt != End;
       ++RegIt) {
    TS.substituteTemps(cast<HLRegion>(&*RegIt));
  }
}

bool HIRTempCleanupLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    LLVM_DEBUG(dbgs() << "HIR Temp Cleanup Disabled \n");
    return false;
  }

  runTempCleanup(getAnalysis<HIRFrameworkWrapperPass>().getHIR());
  return false;
}

PreservedAnalyses HIRTempCleanupPass::runImpl(llvm::Function &F,
                                              llvm::FunctionAnalysisManager &AM,
                                              HIRFramework &HIRF) {
  runTempCleanup(HIRF);
  return PreservedAnalyses::all();
}
