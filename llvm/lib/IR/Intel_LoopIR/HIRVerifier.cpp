//===---- HIRVerifier.cpp - Verifies internal structure of HLNodes --------===//
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

public:
  static void verifyNode(const HLNode *N, bool Recursive = true);
  static void verifyAll();

  HIRVerifierImpl() : TopSortNum(0) {}

  void visit(const HLNode *Node) {
    unsigned CurrentTopSortNum = Node->getTopSortNum();
    if (Node->getParent()) {
      assert(Node->getParent()->getLexicalLastTopSortNum() >=
                 CurrentTopSortNum &&
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
