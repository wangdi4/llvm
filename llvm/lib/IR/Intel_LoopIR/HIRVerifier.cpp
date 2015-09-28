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

#include "llvm/IR/Intel_LoopIR/HIRVerifier.h"
#include "llvm/IR/Intel_LoopIR/HLNode.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

namespace llvm {
namespace loopopt {

class HIRVerifierImpl final : public HLNodeVisitorBase {
public:
  static void verifyNode(const HLNode *N, bool Recursive = true);
  static void verifyAll();

  void visit(const HLNode *Node) { Node->verify(); }
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
