//===------- SIMDIntrinsicChecker.cpp -------------------------------------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements SIMDIntrinsicChecker class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/SIMDIntrinsicChecker.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"

#define DEBUG_TYPE "hir-simd-intrinsic-checker"
using namespace llvm;
using namespace loopopt;

// TODO: expand - or use vpo's def/impl
StringSet<> SIMDIntrinsicChecker::HandleableOpBundleNames = {
    "DIR.OMP.SIMD", "QUAL.OMP.LINEAR:IV", "QUAL.OMP.REDUCTION.ADD",
    "QUAL.OMP.NORMALIZED.IV", "QUAL.OMP.NORMALIZED.UB"};

SIMDIntrinsicChecker::SIMDIntrinsicChecker(const HLInst *DirSIMD,
                                           const HLLoop *Loop)
    : DirSIMD(DirSIMD), DirSIMDExit(nullptr), Loop(Loop), IsHandleable(false) {
  if (DirSIMD && Loop)
    IsHandleable = parseSIMDEntry();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void SIMDIntrinsicChecker::dump() const {

  if (!IsHandleable)
    dbgs() << "SIMD intrin is not currently handleable.\n";

  if (DirSIMD) {
    dbgs() << "SIMD Entry intrinsic instrunction: ";
    DirSIMD->dump();
    dbgs() << "\n";
  }

  if (DirSIMDExit) {
    dbgs() << "SIMD Exi intrinsic instruction: ";
    DirSIMDExit->dump();
    dbgs() << "\n";
  }

  if (!RedPreLoopInsts.empty()) {
    dbgs() << "Pre-loop insts related to OMP REDUCTION:\n";
    for (const HLInst *I : RedPreLoopInsts) {
      I->dump();
      dbgs() << "\n";
    }
  } else {
    dbgs() << "No OMP REDUCTION related pre-loop insts.\n";
  }

  if (!RedPostLoopInsts.empty()) {
    dbgs() << "Post-loop insts related to OMP REDUCTION:\n";
    for (const HLInst *I : RedPostLoopInsts) {
      I->dump();
      dbgs() << "\n";
    }
  } else {
    dbgs() << "No OMP REDUCTION related post-loop insts.\n";
  }
}
#endif

SmallVector<const HLNode *, 4> SIMDIntrinsicChecker::collectPreLoop() {
  SmallVector<const HLNode *, 4> PreLoopNodes;

  // Collect PreLoop
  // From DirSIMD's next  to Loop's prev
  auto SIMDEntryInst = DirSIMD;
  bool IsInPreheader =
      llvm::any_of(Loop->preheaderNodes(), [SIMDEntryInst](const HLNode &N) {
        return &N == SIMDEntryInst;
      });

  // DirSIMD->isInPreheader(Loop) Cannot be used. isInPreheader(*)
  // assumes DirSIMD's parentLoop is Loop. Otherwise, it fails on the
  // asserttion.
  if (IsInPreheader) {
    auto PreHeaderRange = make_range(DirSIMD->getIterator(), Loop->pre_end());
    auto NodeRange =
        map_range(PreHeaderRange, [](const HLNode &N) { return &N; });
    PreLoopNodes.append(NodeRange.begin(), NodeRange.end());
  } else {
    // Dir SIMD Entry is in the pre-loop. Add all from the next node to Loop
    // and all preheaders if any.
    auto PreNodes = map_range(
        make_range(DirSIMD->getNextNode()->getIterator(), Loop->getIterator()),
        [](const HLNode &N) { return &N; });
    PreLoopNodes.append(PreNodes.begin(), PreNodes.end());
    auto PreHeaderRange =
        map_range(Loop->preheaderNodes(), [](const HLNode &N) { return &N; });
    PreLoopNodes.append(PreHeaderRange.begin(), PreHeaderRange.end());
  }

  return PreLoopNodes;
}

SmallVector<const HLNode *, 4> SIMDIntrinsicChecker::collectPostLoop() {
  SmallVector<const HLNode *, 4> PostLoopNodes;

  // PostLoop
  bool EndFound = false;
  for (const HLNode &Node : Loop->postExitNodes()) {
    if (&Node == DirSIMDExit) {
      EndFound = true;
      break;
    }
    PostLoopNodes.push_back(&Node);
  }
  // Collect nodes between the HLLoop node and the end-SIMD clause directive.
  if (!EndFound) {
    const HLNode *CurNode = Loop->getNextNode();
    while (CurNode && CurNode != DirSIMDExit) {
      PostLoopNodes.push_back(CurNode);
      CurNode = CurNode->getNextNode();
    }
    assert(CurNode && "can't find region end");
  }

  return PostLoopNodes;
}

bool SIMDIntrinsicChecker::isHandleableOpBundle(StringRef TagName) {
  return HandleableOpBundleNames.contains(TagName);
}

// VPOAnalysisUtils.h provides \p ClauseSpecifier, which can be used as
//  StringRef ClauseString = BU.getTagName();
//  ClauseSpecifier ClauseInfo(ClauseString);
//  isReductionClause(ClauseInfo.ClauseID);
// \p ClauseSpecifier has a number of internal structure. Using it here
// seems overkill for our purpose. Here, we use a simple alternative.
bool SIMDIntrinsicChecker::isReductionOpBundle(StringRef TagName) {
  // e.g. QUAL.OMP.REDUCTION.ADD
  return TagName.startswith("QUAL.OMP.REDUCTION.");
}

bool SIMDIntrinsicChecker::parseOperands() {

  for (unsigned I = 0, E = DirSIMD->getNumOperandBundles(); I < E; ++I) {
    StringRef TagName = DirSIMD->getOperandBundleAt(I).getTagName();

    if (!isHandleableOpBundle(TagName))
      return false;

    if (!isReductionOpBundle(TagName))
      continue;

    LLVM_DEBUG(dbgs() << "TagName: " << TagName << "\n");

    for (const RegDDRef *Ref : make_range(DirSIMD->bundle_op_ddref_begin(I),
                                          DirSIMD->bundle_op_ddref_end(I))) {
      // insert into set
      // Currently, only AddressOf() refs are handled.
      if (!Ref->isAddressOf())
        return false;
      ReductionRefs.insert(Ref);
    }
  }

  LLVM_DEBUG(
      if (ReductionRefs.empty()) { dbgs() << "No reduction refs\n"; } else {
        dbgs() << "ReductionRefs in SIMD Entry: ";
        for (auto *Ref : ReductionRefs) {
          Ref->dump();
          dbgs() << "\n";
        }
      });

  return true;
}

bool SIMDIntrinsicChecker::hasMatchingReductionRefs(const RegDDRef *Ref) {
  if (Ref->isMemRef()) {
    return std::any_of(ReductionRefs.begin(), ReductionRefs.end(),
                       [Ref](const RegDDRef *RedRef) {
                         return RedRef->isAddressOf() &&
                                DDRefUtils::areEqualWithoutAddressOf(RedRef,
                                                                     Ref);
                       });

  } else if (Ref->isTerminalRef()) {
    return llvm::any_of(ReductionRefs, [Ref](const RegDDRef *RedRef) {
      return DDRefUtils::areEqual(RedRef, Ref);
    });
  }

  return false;
}

const HLInst *SIMDIntrinsicChecker::findSIMDExit(const HLLoop *Loop) {
  // PostExit
  const HLNode *Node = Loop->getFirstPostexitNode();
  while (Node) {
    if (const HLInst *Inst = dyn_cast<HLInst>(Node))
      if (Inst->isSIMDEndDirective())
        return Inst;
    Node = Node->getNextNode();
  }

  // PostLoop
  Node = Loop->getNextNode();
  while (Node) {
    if (const HLInst *Inst = dyn_cast<HLInst>(Node))
      if (Inst->isSIMDEndDirective())
        return Inst;
    Node = Node->getNextNode();
  }

  return nullptr;
}

bool SIMDIntrinsicChecker::parseSIMDEntry() {

  // Parse OperandBundles of SIMDEntry
  if (!parseOperands())
    return false;

  DirSIMDExit = findSIMDExit(Loop);
  if (!DirSIMDExit)
    return false;

  // Collect PreLoop/PreHeader & PostLoop/PostExit insts
  // Same logic as in HIRVectorizationLegality::findAliasDDRefs
  SmallVector<const HLNode *, 4> PreLoopNodes = collectPreLoop();
  SmallVector<const HLNode *, 4> PostLoopNodes = collectPostLoop();

  // From PreLoop, collect reduction related insts, if any.
  // E.g. %t1 = %a[0]
  //      %t2 = %t1
  // Refs %a[0] & %t1 & %t2 will be elements of ReductionRefs.
  // Insts (1) & (2) will be collected in RedPreLoopInsts.
  for (const HLNode *Node : PreLoopNodes) {
    const HLInst *HInst = dyn_cast<HLInst>(Node);
    if (!HInst)
      continue;
    const RegDDRef *Rval = HInst->getRvalDDRef();
    if (!Rval)
      continue;

    if (hasMatchingReductionRefs(Rval)) {
      assert(HInst->getLvalDDRef());
      ReductionRefs.insert(HInst->getLvalDDRef());
      RedPreLoopInsts.push_back(HInst);
    }
  }

  // From PostLoop, collect reduction related insts, if any.
  // E.g. %a[0] = %t1
  //      %b[0] = %t2
  // Where OMP.SIMD.REDUCTION.*(&(%a[0])), OMP.SIMD.REDUCTION.*(&(%b[0]))
  for (const HLNode *Node : PostLoopNodes) {
    const HLInst *HInst = dyn_cast<HLInst>(Node);
    if (!HInst)
      continue;
    const RegDDRef *Lval = HInst->getLvalDDRef();
    if (!Lval)
      continue;

    if (hasMatchingReductionRefs(Lval)) {
      assert(HInst->getRvalDDRef());
      RedPostLoopInsts.push_back(HInst);
    }
  }

  return true;
}

// Everything is in pre/post loop, nothing in preheader or postexit.
bool SIMDIntrinsicChecker::areAllInPreAndPostLoop() const {
  unsigned MinLoopTopSortNum = Loop->getMinTopSortNum();
  unsigned MaxLoopTopSortNum = Loop->getMaxTopSortNum();
  return DirSIMD->getTopSortNum() < MinLoopTopSortNum &&
         DirSIMDExit->getTopSortNum() > MaxLoopTopSortNum &&
         (RedPreLoopInsts.empty() ||
          RedPreLoopInsts.back()->getTopSortNum() < Loop->getMinTopSortNum()) &&
         (RedPostLoopInsts.empty() ||
          RedPostLoopInsts.front()->getTopSortNum() > Loop->getMaxTopSortNum());
}
