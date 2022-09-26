//===---- HLNodeIterator.h - GraphTraits Instantiation ----------*- C++ -*-===//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// GraphTraits specializations for HLNode
// to use df_iterator.
//
// using HLPreOrderIterator = df_iterator<loopopt::HLNode *>;
//
// HLPreOrderIterator is a direct implementation of df_iterator.
// Thus, it iterates a subtree(graph) starting from the given root node.
// Notice parameters to begin() and end() functions should be pointers to
// HLNode. HLPreOrderIterator::end(Parameter)'s Parameter does not have any
// effect because internally, Parameter is never used. For readability,
// parameter to HLPreOrderIterator::end() can be set to the same
// parameter to that of begin() function or nullptr.
//
// Example usages:
// for (HLPreOrderIterator
//     It = df_iterator<HLNode*>::begin(&*HIRF.hir_begin()),
//     EIt = df_iterator<HLNode *>::end(nullptr); It != EIt; ++It) {
//              It->dump();
// }
//
// if (std::any_of(HLPreOrderIterator::begin(&*HIRF.hir_begin()),
//                 HLPreOrderIterator::end(&*HIRF.hir_begin()))) {
//        do something
// }
//
// HLRangeIterator is a wraper around HLPreOrderIterator to provide the
// functionality of visitRange(BeginIter, EndIter);
//
// Example usages:
// for (HLRangeIterator It = HLRangeIterator(HIRF.hir_begin()),
//                      EIt = HLRangeIterator(HIRF.hir_end());
//      It != EIt; ++It) {
//       do something
//     }
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HLNODEITER_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HLNODEITER_H

#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNode.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

namespace llvm {

namespace loopopt {
class ChildNodeIterator {
public:
  using iteration_category = std::forward_iterator_tag;
  using value_type = HLNode *;
  using difference_type = ptrdiff_t;
  using pointer = HLNode *;
  using reference = HLNode *;

  using ilist_iter = loopopt::HLContainerTy::iterator;

private:
  ilist_iter InnerIter;
  HLNode *getNode() const { return &(*InnerIter); }

  /// Denotes a status when a loop is visited as a child.
  /// Normal : the loop is visted first, thus before its preheader.
  /// Pre : loop's preheader will be visited and if the loop is visted (again)
  ///       it is after its preheader is visited
  /// Post: loop is already visited and it is a turn for its postexit.
  enum StateTy { Pre, Normal, Post };
  StateTy State;

  /// Loop that are being visited. Notice this will have the pointer to a loop,
  /// even when its preheader and postexit are visited.
  HLLoop *LoopInTransit;

  /// Reset the status because a loop including preheader/postexit is done.
  void goOutOfLoop() {
    LoopInTransit = nullptr;
    State = Normal;
  }

public:
  ChildNodeIterator(const ilist_iter &ChildIter)
      : InnerIter(ChildIter), State(Normal), LoopInTransit(nullptr) {}

  ChildNodeIterator() {}

  ChildNodeIterator &operator++() {
    if (State == Normal) {
      if (HLLoop *Node = dyn_cast<HLLoop>(getNode())) {
        if (Node->pre_begin() != Node->pre_end()) {
          InnerIter = Node->pre_begin();
        }
        assert(!LoopInTransit);
        // Even it pre-header doesn't exist, mark that we are taking care of
        // loop
        LoopInTransit = Node;
        State = Pre;
      } else {
        // Non-loop (thus, no-preheader, no-postexit)
        assert(!isa<HLLoop>(getNode()));
        ++InnerIter;
      }
    } else if (State == Pre) {
      // Non-loop never get here.
      assert(LoopInTransit);
      if (HLLoop *Lp = dyn_cast<HLLoop>(getNode())) {
        // Loop is revisited after pre-header
        // Now is the turn for post-exit
        assert(Lp == LoopInTransit);
        if (Lp->post_begin() != Lp->post_end()) {
          InnerIter = Lp->post_begin();
          State = Post;
        } else {
          ++InnerIter;
          goOutOfLoop();
        }
      } else {
        // go through pre-header;
        ++InnerIter;
        if (InnerIter == LoopInTransit->pre_end()) {
          InnerIter = LoopInTransit->getIterator();
        }
      }
    } else {
      // State == Post
      // Non-loop never get here.
      assert(LoopInTransit);
      assert(InnerIter != LoopInTransit->post_end());
      // go through post-exit
      ++InnerIter;
      if (InnerIter == LoopInTransit->post_end()) {
        InnerIter = LoopInTransit->getIterator();
        ++InnerIter;
        goOutOfLoop();
      }
    }
    return (*this);
  }

  ChildNodeIterator operator++(int) {
    ChildNodeIterator RetVal = *this;
    ++(*this);
    return RetVal;
  }

  bool operator==(const ChildNodeIterator &Other) const {
    return this->InnerIter == Other.InnerIter;
  }

  bool operator!=(const ChildNodeIterator &Other) const {
    return !(operator==(Other));
  }

  // Suppose HLLoop was given as the starting point of df_iterator.
  // Specializing opertor*() cannot help in making pre_header of the loop
  // is visited before the loop itself by df_itertor.
  // As shown below, the operator implementation of df_iterator
  // the stack's top is returned right away. operator*() of
  // ChildNodeIterator is not called before returning the top of the stack.
  // By df_iterator<loopopt::HLNode*>::begin(HLLoop* l), HLLoop* l is
  // immediately put into VisitStack's top.
  // From df_itarator
  //     const NodeRef &operator*() const { return VisitStack.back().first; }
  //     NodeRef operator->() const { return **this; }
  //
  // In other words, Node set by df_iterator<..>::begin(Node) does not
  // affected by ChildNodeIterator at all.
  // Thus, currently,
  // if HLLoop loop is given as the root of df_iterator(), that loop's
  // pre-header/post-exit are not visited.
  HLNode *operator*() const {
    // df_iterator<> does post-inc.
    //
    // NodeRef = *(*Opt)++;
    // For the first reference of loop as a child of a parent,
    // operator*() should return the pre-header, if any.
    if (State == Normal) {
      if (HLLoop *Node = dyn_cast<HLLoop>(getNode())) {
        if (Node->pre_begin() != Node->pre_end()) {
          return &(*Node->pre_begin());
        }
      }
    }
    return getNode();
  }
};

class NonLoopChildBeginEnd {
private:
  using ChildIteratorType = loopopt::ChildNodeIterator;
  using NodeRef = loopopt::HLNode *;

public:
  // child_begin for non-loop
  static ChildIteratorType non_loop_child_begin(NodeRef N) {

    assert(!isa<HLLoop>(N));

    if (HLRegion *Reg = dyn_cast<HLRegion>(N)) {
      return ChildIteratorType(Reg->child_begin());
    } else if (HLIf *If = dyn_cast<HLIf>(N)) {
      return ChildIteratorType(If->child_begin());
    } else if (HLSwitch *Switch = dyn_cast<HLSwitch>(N)) {
      return ChildIteratorType(Switch->child_begin());
    } else {
      // HLInst, HLGoto, HLLabel - no children
      return ChildIteratorType();
    }
  }

  static ChildIteratorType non_loop_child_end(NodeRef N) {

    assert(!isa<HLLoop>(N));

    if (HLRegion *Reg = dyn_cast<HLRegion>(N)) {
      return ChildIteratorType(Reg->child_end());
    } else if (HLIf *If = dyn_cast<HLIf>(N)) {
      return ChildIteratorType(If->child_end());
    } else if (HLSwitch *Switch = dyn_cast<HLSwitch>(N)) {
      return ChildIteratorType(Switch->child_end());
    } else {
      // HLInst, HLGoto, HLLabel - no children
      return ChildIteratorType();
    }
  }
};

} // namespace loopopt

template <> struct GraphTraits<loopopt::HLNode *> {
  using NodeRef = loopopt::HLNode *;

  using ChildIteratorType = loopopt::ChildNodeIterator;

  using HLLoop = loopopt::HLLoop;
  using HLRegion = loopopt::HLRegion;
  using HLIf = loopopt::HLIf;
  using HLSwitch = loopopt::HLSwitch;

  static NodeRef getEntryNode(loopopt::HLNode *N) { return N; }

  static ChildIteratorType child_begin(NodeRef N) {
    if (HLLoop *Lp = dyn_cast<HLLoop>(N)) {
      // Note that preheader is visited before loopbody
      // but after loop itself.
      return ChildIteratorType(Lp->child_begin());
    } else {
      return loopopt::NonLoopChildBeginEnd::non_loop_child_begin(N);
    }
  }

  static ChildIteratorType child_end(NodeRef N) {
    if (HLLoop *Lp = dyn_cast<HLLoop>(N)) {
      return ChildIteratorType(Lp->child_end());
    } else {
      return loopopt::NonLoopChildBeginEnd::non_loop_child_end(N);
    }
  }
};

namespace loopopt {
namespace skipinnermostbody {

// GraphTraits that skipping recursing into the body of the innermost loop.
// Other than that, the functionality is the same as the one above.
// Separate GraphTraits in another namespace is neede because of following
// reason.
// I would like to use the same GraphType and NodeRef type with
// the one above but make it work slightly differently.
// No additional template argument to GraphType is allowed
// to utilize df_iterator. But I can have different GraphTraits.
// Thus, a new GraphTraits in the new namespace.
// Notice that template specialization has to be happen
// within the same namespace.
template <typename GraphType> struct GraphTraits {
  // using NodeRef = typename GraphType::UnknownGraphTypeError;
};

template <> struct GraphTraits<loopopt::HLNode *> {
  using NodeRef = loopopt::HLNode *;

  using ChildIteratorType = loopopt::ChildNodeIterator;

  using HLLoop = loopopt::HLLoop;
  using HLRegion = loopopt::HLRegion;
  using HLIf = loopopt::HLIf;
  using HLSwitch = loopopt::HLSwitch;

  static NodeRef getEntryNode(loopopt::HLNode *N) { return N; }

  static ChildIteratorType child_begin(NodeRef N) {
    if (HLLoop *Lp = dyn_cast<HLLoop>(N)) {
      // Note that preheader is visited before loopbody
      // but after loop itself.
      if (!Lp->isInnermost())
        return ChildIteratorType(Lp->child_begin());
      else
        return ChildIteratorType();
    } else {
      return loopopt::NonLoopChildBeginEnd::non_loop_child_begin(N);
    }
  }

  static ChildIteratorType child_end(NodeRef N) {
    if (HLLoop *Lp = dyn_cast<HLLoop>(N)) {
      if (!Lp->isInnermost())
        return ChildIteratorType(Lp->child_end());
      else
        return ChildIteratorType();
    } else {
      return loopopt::NonLoopChildBeginEnd::non_loop_child_end(N);
    }
  }
};

} // namespace skipinnermostbody
} // namespace loopopt

// Use it if a HLNode's all subtree needs to be visited.
// GraphTraits<HLNode *> specialization in llvm namespace is used.
using HLPreOrderIterator = df_iterator<loopopt::HLNode *>;

// GraphTraits<HLNode *> specialization in
// llvm::loopopt::skipinnermostbody namespace is used.
using SkipInnermostGraphTraits =
    loopopt::skipinnermostbody::GraphTraits<loopopt::HLNode *>;
using HLPreOrderIteratorSkipInnermostBody =
    df_iterator<loopopt::HLNode *,
                df_iterator_default_set<SkipInnermostGraphTraits::NodeRef>,
                false, SkipInnermostGraphTraits>;

namespace loopopt {

/// HLRangeIterator
/// A wraper iterator class to implement the functionality of
/// visitRange(Begin, End). Nodes are visited in DFS's preorder.
/// df_iterator starts from a Node as a root of a tree(graph) and
/// ends when all reacheable Nodes are visited.
/// This iterator goes through all nodes in DFS preorder starting from
/// Begin to Prev(End).
/// Begin and End are sequentially connected through ilist.
/// End is the past the last, the last node reachable from Begin
/// by going forward through ilist.
/// The logic is the same as visitRange(Begin, End)
template <typename HLPreOrderIteratorClass>
class HLRangeIteratorImpl {
public:
  using iteration_category = std::forward_iterator_tag;
  using value_type = loopopt::HLContainerTy::iterator;
  using difference_type = std::ptrdiff_t;
  using pointer = loopopt::HLContainerTy::iterator;
  using reference = HLNode *;

private:
  using ilist_iter = loopopt::HLContainerTy::iterator;
  using NodeRef = loopopt::HLNode *;
  using df_iter = HLPreOrderIteratorClass;

private:
  /// Tracking HLContainerTy::iterator.
  /// Subtree where dfs search is done for each.
  ilist_iter TreeRootIter;

  /// Trakcing DFS pre order point.
  mutable df_iter DfIter;

  /// To denote the end of a DFS search. Empty visit stack.
  df_iter DfIterEnd;

  /// Tracking the status of iterator.
  /// True means HLRangeIterator is before referenced for the first time.
  /// This is a workaround for the lack of isSentinel() capability on HLNode.
  /// If Sentinel is enabled for HLNode, this can be removed and
  /// isSentinel() can be directly used during construction.
  mutable bool IsFirst;

  // Populate visit stack of df_iterator before any operator works if
  // the stack is empty.
  // Ideally, this can be done in the constructor, but we can't do so
  // because our HLContainterTy is Sentinel-un-enabled simple_ilist.
  void populateDFStackIfEmpty() const {
    if (IsFirst) {
      DfIter = df_iter::begin(&*TreeRootIter);
      IsFirst = false;
    }
  }

public:
  HLRangeIteratorImpl(const ilist_iter &I)
      : TreeRootIter(I),
        // At first, we keep df_iterator's stack empty.
        // This is because, we do not know if given I is Sentinel or not.
        // HLNode is simple_ilist, which by default, Sentinel is not enabled.
        // Thus, isSentinel() is not callable.
        // However, if given I is a Sentinel, e.g. hir_end(), dereferencing
        // I causes a seg fault.
        // df_iterator<>::end() construct a df_iterator<> with empty stack.
        // Stack is populated first in operator*() and &operatpr++().
        DfIter(df_iter::end(nullptr)), DfIterEnd(df_iter::end(nullptr)),
        IsFirst(true) {}

  NodeRef operator*() const {
    populateDFStackIfEmpty();
    return *DfIter;
  }

  HLRangeIteratorImpl &operator++() {
    // Check here should be present in case &operator++() is called
    // before other referencing operator like operator*().
    // E.G. std::any_of calls &operator++() first
    populateDFStackIfEmpty();

    ++DfIter;
    if (DfIter == DfIterEnd) {
      ++TreeRootIter;
      // IsFirst should be rest to reflect starting of a new TreeRoot.
      IsFirst = true;
    }
    return *this;
  }

  bool operator==(const HLRangeIteratorImpl &Other) const {
    return TreeRootIter == Other.TreeRootIter && DfIter == Other.DfIter &&
           DfIterEnd == Other.DfIterEnd;
  }

  bool operator!=(const HLRangeIteratorImpl &Other) const {
    return !(*this == Other);
  }

  HLRangeIteratorImpl operator++(int) {
    HLRangeIteratorImpl RetVal = *this;
    ++(*this);
    return RetVal;
  }
};

// template <typename T> using ForEach = ForEachImpl<T, false>;
using HLRangeIterator = HLRangeIteratorImpl<HLPreOrderIterator>;
using HLRangeSkipInnermostIterator =
    HLRangeIteratorImpl<HLPreOrderIteratorSkipInnermostBody>;

} // namespace loopopt
} // namespace llvm

#endif
