//===---------- ForEach.h - Visitor helper class for HIR --------*- C++ -*-===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// If you don't need a complex logic for iterating over HIR and you are concern
// about only one type of HLNodes, the ForEach<T> helper could be useful.
//
// Here is an example:
//
// * To iterate over every HLIf within a region R:
//
//   ForEach<HLIf>::visitRange(R->child_begin(), R->child_end(),
//     [](HLIf *If) {
//       If->dump();
//   });
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_FOREACH_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_FOREACH_H

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

namespace llvm {

namespace loopopt {

class HLNode;

namespace internal {

/// Internal visitor that is used by ForEach, ForPostEach classes.
/// It will call \p Func() on each visit(T*)/postVisit(T*) depending on the
/// \p IsPostVisitor.
template <typename T, typename Func, bool IsPostVisitor>
struct ForEachVisitor final : public HLNodeVisitorBase {
  Func F;
  ForEachVisitor(Func F) : F(F) {}

  void visit(T *Node) {
    if (!IsPostVisitor) {
      F(Node);
    }
  }

  void postVisit(T *Node) {
    if (IsPostVisitor) {
      F(Node);
    }
  }

  void postVisit(const HLNode *) {}
  void visit(const HLNode *) {}
};

}

/// The ForEach<T> class could be used to iterate over the HIR nodes of fixed
/// type \p T post-order using functional approach.
template <typename T> struct ForPostEach {
  template <typename Iter, typename Func>
  static void visitRange(Iter Begin, Iter End, Func F) {
    internal::ForEachVisitor<T, Func, true> Visitor(F);
    HLNodeUtils::visitRange(Visitor, Begin, End);
  }
};

/// The ForEach<T> class could be used to iterate over the HIR nodes of fixed
/// type \p T in-order using functional approach.
template <typename T> struct ForEach {
  template <typename Iter, typename Func>
  static void visitRange(Iter Begin, Iter End, Func F) {
    internal::ForEachVisitor<T, Func, false> Visitor(F);
    HLNodeUtils::visitRange(Visitor, Begin, End);
  }
};

}

}

#endif
