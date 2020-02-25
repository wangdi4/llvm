//===---------- ForEach.h - Visitor helper class for HIR --------*- C++ -*-===//
//
// Copyright (C) 2016-2020 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"

namespace llvm {

namespace loopopt {

class HLNode;

namespace detail {

template <typename T, typename Func>
struct ForEachVisitorBaseTraits {
  typedef T NodeType;

  static_assert(
      std::is_base_of<HLNode, typename std::remove_const<T>::type>::value,
      "Undefined node type, should be derived from HLNode");


  static void visit(NodeType *Node, Func F) { F(Node); }
  static bool skipRecursion(const HLNode *Node) { return false; }
};

template <typename T, typename Func>
struct ForEachVisitorTraits : ForEachVisitorBaseTraits<T, Func> {};

template <typename Func>
struct ForEachVisitorTraits<HLRegion, Func>
    : ForEachVisitorBaseTraits<HLRegion, Func> {
  static bool skipRecursion(const HLNode *Node) { return true; }
};

template <typename NodeTy, typename Func>
struct ForEachRegDDRefVisitorTraits
    : public ForEachVisitorBaseTraits<NodeTy, Func> {
  static void visit(NodeTy *Node, Func F) {
    for (auto *Ref : make_range(Node->ddref_begin(), Node->ddref_end())) {
      F(Ref);
    }
  }
};

template <typename Func>
struct ForEachVisitorTraits<RegDDRef, Func>
    : ForEachRegDDRefVisitorTraits<HLDDNode, Func> {};

template <typename Func>
struct ForEachVisitorTraits<const RegDDRef, Func>
    : ForEachRegDDRefVisitorTraits<const HLDDNode, Func> {};

/// Internal visitor that is used by ForEach, ForPostEach classes.
/// It will call \p Func() on each visit(T*)/postVisit(T*) depending on the
/// \p IsPostVisitor.
template <typename T, typename Func, bool IsPostVisitor>
struct ForEachVisitor final : public HLNodeVisitorBase {
  Func F;
  ForEachVisitor(Func F) : F(F) {}

  void visit(typename ForEachVisitorTraits<T, Func>::NodeType *Node) {
    if (!IsPostVisitor) {
      ForEachVisitorTraits<T, Func>::visit(Node, F);
    }
  }

  void postVisit(typename ForEachVisitorTraits<T, Func>::NodeType *Node) {
    if (IsPostVisitor) {
      ForEachVisitorTraits<T, Func>::visit(Node, F);
    }
  }

  void postVisit(const HLNode *) {}
  void visit(const HLNode *) {}

  bool skipRecursion(const HLNode *Node) const {
    return ForEachVisitorTraits<T, Func>::skipRecursion(Node);
  }
};

}

template <typename T, bool IsPostVisitor>
struct ForEachImpl {
  template <bool Recursive = true, typename Iter, typename Func>
  static void visitRange(Iter Begin, Iter End, Func F) {
    detail::ForEachVisitor<T, Func, IsPostVisitor> Visitor(F);
    HLNodeUtils::visitRange<Recursive>(Visitor, Begin, End);
  }

  template <bool Recursive = true, typename NodeTy, typename Func>
  static void visit(NodeTy Node, Func F) {
    detail::ForEachVisitor<T, Func, IsPostVisitor> Visitor(F);
    HLNodeUtils::visit<Recursive>(Visitor, Node);
  }
};

/// The ForEach<T> class could be used to iterate over the HIR nodes of fixed
/// type \p T in-order using functional approach.
template <typename T> using ForEach = ForEachImpl<T, false>;

/// The ForPostEach<T> class could be used to iterate over the HIR nodes of
/// fixed type \p T post-order using functional approach.
template <typename T> using ForPostEach = ForEachImpl<T, true>;
}
}

#endif
