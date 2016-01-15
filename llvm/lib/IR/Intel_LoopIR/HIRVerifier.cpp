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

#include "llvm/IR/Intel_LoopIR/HIRVerifier.h"
#include "llvm/IR/Intel_LoopIR/HLNode.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#define DEBUG_TYPE "hir-verify"

using namespace llvm;
using namespace llvm::loopopt;

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

public:
  static void verifyNode(const HLNode *N, bool Recursive = true);
  static void verifyAll();

  HIRVerifierImpl()
      : TopSortNum(0), InnermostLoop(nullptr), CurrentLoop(nullptr) {}

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
  }

  void visit(const HLRegion *R) {
    TopSortNum = 0;
    visit(static_cast<const HLNode *>(R));
  }

  void postVisit(const HLNode *Node) {}

  void visit(const HLLoop *L) {
    // Innermost flag verification begin
    CurrentLoop = L;

    assert(InnermostLoop == nullptr && "Found a loop inside innermost loop");
    if (L->isInnermost()) {
      InnermostLoop = L;
    }
    // Innermost flag verification end

    visit(static_cast<const HLNode *>(L));
  }

  void postVisit(const HLLoop *L) {
    // Innermost flag verification begin
    if (InnermostLoop) {
      assert(L == InnermostLoop && "Unexpected HLLoop");
      InnermostLoop = nullptr;
    }
    // Check if the node is leaf HLLoop. Only for the leaf HLLoop visit(HLLoop*)
    // and postVisit(HLLoop*) would be called subsequently.
    if (L == CurrentLoop) {
      assert(L->isInnermost() && "No innermost loop found");
    }
    // Innermost flag verification end

    postVisit(static_cast<const HLNode *>(L));
  }
};
}
}

template <bool Recursive> void HIRVerifier::verifyNode(const HLNode *N) {
  HIRVerifierImpl V;
  HLNodeUtils::visit<Recursive>(V, N);
}

void HIRVerifier::verifyAll() {
  HIRVerifierImpl V;
  HLNodeUtils::visitAll(V);
}
