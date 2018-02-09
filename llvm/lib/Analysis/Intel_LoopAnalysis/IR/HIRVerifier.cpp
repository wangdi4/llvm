//===---- HIRVerifier.cpp - Verifies internal structure of HLNodes --------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HIR verifier that checks internal
// structure of HLNodes and attached DDRefs
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"
#include "llvm/ADT/BitVector.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HIRVerifier.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNode.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#define DEBUG_TYPE "hir-verify"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    HIRQuality("hir-verify-quality",
               cl::desc("Verify quality of HIR (default=false), whether "
                        "attributes are not too conservative"),
               cl::init(false));

static cl::opt<bool>
    HIRCFDefLevel("hir-verify-cf-def-level",
                  cl::desc("Verify consistency of control-flow-related def-level "
                           "attribute (default=false)"),
                  cl::init(false));

namespace {

// 1. Checks that linear uses do not contradict non-linear definitions
//
// The following code will result in assertion.
// DO i1 = 0, N
//    DO i2 = 0, N
//      ... = %0 (linear@0)
//    END DO
//    %0 (non-linear) = ...
// END DO
//
// The check assumes that HLIf, HLSwitch and loops' ZTT does not affect flow of
// information from kill to use. Control flow is used for checks.
//
// 2. Checks that linear uses have not too conservative def@level
class UseKillInfo {
public:
  UseKillInfo(unsigned MaxSymbase) {
    for (unsigned Ind = 1; Ind <= MaxLoopNestLevel; ++Ind) {
      TempLinearUses[Ind - 1].resize(MaxSymbase);
      TempDefLevel[Ind - 1].resize(MaxSymbase);
      TempKills[Ind - 1].resize(MaxSymbase);
    }
  }

  // ToLevel is lexical level where temp reference occurs.
  void addUses(unsigned DefLevel, unsigned ToLevel, unsigned TempSymbase) {
    for (unsigned Ind = DefLevel + 1; Ind <= ToLevel; ++Ind) {
      TempLinearUses[Ind - 1].set(TempSymbase - 1);
    }
    if (DefLevel > 0) {
      TempDefLevel[DefLevel - 1].set(TempSymbase - 1);
    }
  }

  void addKills(unsigned Level, unsigned TempSymbase) {
    for (unsigned Ind = 1; Ind <= Level; ++Ind) {
      TempKills[Ind - 1].set(TempSymbase - 1);
    }
  }

  void postCheckLoopAndReport(const HLLoop *Loop) {
    auto Level = Loop->getNestingLevel();
    // Consistency check
    bool Consistent = true;
    if (HIRCFDefLevel) {
      // DO i1 ..
      //   DO i2 = ..
      //      DO i3 =
      //        .. = %t(def@1), addUses(1, 3, ..)
      //      END DO
      //      %t = .. addKills(2, ...)
      //   END DO // %t inconsistency found here
      // END DO
      auto &Uses = TempLinearUses[Level - 1]; // Update in-place
      // Set-theoretic TempLinearUses[Level - 1] & TempKills[Level - 1]
      Uses &= TempKills[Level - 1];

      Consistent = Uses.none();
      if (!Consistent) {
        printSymbases("Inconsistent SBs:", Uses);
      }
    }
    // Conservative check
    bool Conservative = false;
    if (HIRQuality) {
      // DO i1 ..
      //   DO i2 = ..
      //      DO i3 =
      //        .. = %t(def@2), addUses(2, 3, ..)
      //      END DO
      //   END DO // %t conservative found here, no kills at level 2
      //   %t = .. addKills(1, ...)
      // END DO
      auto &Uses =
          TempDefLevel[Level - 1]; // Update in-place TempDefLevel and TempKills
      TempKills[Level - 1].flip();
      // Set-theoretic TempDefLevel[Level - 1] & ~TempKills[Level - 1]
      Uses &= TempKills[Level - 1];

      Conservative = Uses.any();
      if (Conservative) {
        printSymbases("Conservative SBs:", Uses);
      }
    }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (!Consistent || Conservative)
      Loop->dump(true);
#endif

    assert(Consistent && "def@level is inconsistent");
    assert(!Conservative && "def@level is conservative");

    resetForLevel(Level);
  }

  void postCheckRegion() {
    for (unsigned Ind = 1; Ind <= MaxLoopNestLevel; ++Ind) {
      assert(TempLinearUses[Ind - 1].none() && "Traversal logic is incorrect");
      assert(TempDefLevel[Ind - 1].none() && "Traversal logic is incorrect");
      assert(TempKills[Ind - 1].none() && "Traversal logic is incorrect");
    }
  }

private:
  UseKillInfo() = delete;
  UseKillInfo(const UseKillInfo &) = delete;
  UseKillInfo &operator=(const UseKillInfo &) = delete;

  /// Which temps are considered linear at a given level
  BitVector TempLinearUses[MaxLoopNestLevel];
  /// TempDefLevel[L - 1] keeps temps uses, which are marked as def@level == L
  BitVector TempDefLevel[MaxLoopNestLevel];
  /// Which temps are modified at a given level
  BitVector TempKills[MaxLoopNestLevel];

  void resetForLevel(unsigned Level) {
    TempLinearUses[Level - 1].reset();
    TempKills[Level - 1].reset();
    TempDefLevel[Level - 1].reset();
  }
  static void printSymbases(const char *Prefix, const BitVector &BV) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    dbgs() << Prefix << " ";
    bool First = true;
    for (auto It = BV.find_first(); It != -1; It = BV.find_next(It)) {
      dbgs() << (First ? "" : ", ") << It + 1;
      First = false;
    }
    dbgs() << "\n";
#endif
  }
};
}

namespace llvm {
namespace loopopt {

class HIRVerifierImpl final : public HLNodeVisitorBase {
  unsigned TopSortNum;

  // These fields InnermostLoop and CurrentLoop are used to verify correctness
  // of the innermost flag of HLLoop objects.
  // There two cases to check:
  //  1. There is a loop inside an innermost loop,
  //  2. Leaf HLLoop always should be marked as innermost.
  // In the forward traversal (visit) we check #1
  // In the backward traversal (postVisit) we check #2.
  const HLLoop *InnermostLoop;
  const HLLoop *CurrentLoop;

  // The set is used to catch duplicated HLNodeNumbers.
  std::set<unsigned> HLNodeNumbers;

  // The class to check scalar non-linear kill/uses
  UseKillInfo BlobUsesKills;

public:
  HIRVerifierImpl(unsigned MaxScalarSymbase)
      : TopSortNum(0), InnermostLoop(nullptr), CurrentLoop(nullptr),
        BlobUsesKills(MaxScalarSymbase) {}

  void visit(const HLNode *Node) {
    unsigned CurrentTopSortNum = Node->getTopSortNum();
    if (Node->getParent()) {
      assert(Node->getParent()->getMaxTopSortNum() >= CurrentTopSortNum &&
             "Parent LexicalLastTopSortNum should "
             "be bigger than every TopSortNum");
    }

    assert(CurrentTopSortNum > TopSortNum &&
           "TopSortNum should be strictly monotonic");
    TopSortNum = CurrentTopSortNum;

    Node->verify();

    unsigned Number = Node->getNumber();
    assert(HLNodeNumbers.count(Number) == 0 && "Node number duplicate found!");
    HLNodeNumbers.insert(Number);
  }

  void visit(const HLDDNode *DDNode) {
    auto NodeLevel = DDNode->getNodeLevel();
    for (const DDRef *Ref :
         llvm::make_range(DDNode->all_dd_begin(), DDNode->all_dd_end())) {

      if (!Ref->isTerminalRef()) {
        continue;
      }

      bool IsLval = Ref->isLval();
      if (!Ref->isSelfBlob() && !IsLval) {
        continue;
      }

      auto Symbase = Ref->getSymbase();
      const auto *CE = Ref->getSingleCanonExpr();
      if (IsLval) {
        BlobUsesKills.addKills(NodeLevel, Symbase);
      } else {
        BlobUsesKills.addUses(CE->isNonLinear() ? NodeLevel
                                                : CE->getDefinedAtLevel(),
                              NodeLevel, Symbase);
      }
    }
    visit(static_cast<const HLNode *>(DDNode));
  }

  void visit(const HLRegion *Region) {
    TopSortNum = 0;
    visit(static_cast<const HLNode *>(Region));
  }

  void visit(const HLLoop *Loop) {
    // Innermost flag verification begin
    CurrentLoop = Loop;

    assert(InnermostLoop == nullptr && "Found a loop inside innermost loop");
    if (Loop->isInnermost()) {
      InnermostLoop = Loop;
    }
    // Innermost flag verification end

    visit(static_cast<const HLDDNode *>(Loop));
  }

  void postVisit(const HLNode *Node) {}

  void postVisit(const HLRegion *Region) {
    BlobUsesKills.postCheckRegion();
    postVisit(static_cast<const HLNode *>(Region));
  }

  void postVisit(const HLLoop *Loop) {
    // Innermost flag verification begin
    if (InnermostLoop) {
      assert(Loop == InnermostLoop && "Unexpected HLLoop");
      InnermostLoop = nullptr;
    }
    // Check if the node is leaf HLLoop. Only for the leaf HLLoop visit(HLLoop*)
    // and postVisit(HLLoop*) would be called subsequently.
    if (Loop == CurrentLoop) {
      assert(Loop->isInnermost() && "No innermost loop found");
    }
    // Innermost flag verification end

    BlobUsesKills.postCheckLoopAndReport(Loop);

    postVisit(static_cast<const HLNode *>(Loop));
  }
};
}
}

void HIRVerifier::verifyAll(const HIRFramework &HIRF) {
  HIRVerifierImpl V(HIRF.getMaxSymbase());
  auto Marker = HIRF.getHLNodeUtils().getMarkerNode();

  (void)Marker;
  assert((!Marker || !Marker->isAttached()) &&
         "Marker node is attached to HIR!");

  HLNodeUtils::visitRange(V, HIRF.hir_begin(), HIRF.hir_end());
}
