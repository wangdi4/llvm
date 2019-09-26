//===-- IntelVPlanLoopIterator.h --------------------------------*- C++ -*-===//
//
//   Copyright (C) 2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
// This file defines GraphTraits to visit the VPBlockBases within a VPLoop.
//
// The idea and part of the implementation was borrowed from the LoopIterator.h.
// When upstreaming, we should probably unify/templatize that one instead.
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANLOOPITERATOR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANLOOPITERATOR_H

#include "IntelVPlan.h"
#include "IntelVPlanLoopInfo.h"

#include <utility>

namespace llvm {
namespace vpo {
struct VPLoopBodyTraitsImpl {
  using NodeRef = std::pair<const VPLoop *, const VPBlockBase *>;
  using GraphRef = const VPLoop *;

  using SuccIterator =
      decltype(std::declval<const VPBlockBase>().getSuccessors().begin());

  class WrappedSuccIterator
      : public iterator_adaptor_base<
            WrappedSuccIterator, SuccIterator,
            typename std::iterator_traits<SuccIterator>::iterator_category,
    NodeRef, ptrdiff_t, NodeRef *, NodeRef> {
    using Base = iterator_adaptor_base<
        WrappedSuccIterator, SuccIterator,
        typename std::iterator_traits<SuccIterator>::iterator_category,
      NodeRef, ptrdiff_t, NodeRef *, NodeRef>;
    const VPLoop *VPL;

  public:
    WrappedSuccIterator(SuccIterator Begin, const VPLoop *VPL)
        : Base(Begin), VPL(VPL) {}

    NodeRef operator*() const { return {VPL, *I}; }
  };

  static NodeRef getEntryNode(GraphRef G) { return {G, G->getHeader()}; }

  static decltype(auto) children(NodeRef Node) {
    return make_filter_range(
        make_range<WrappedSuccIterator>(
            {Node.second->getSuccessors().begin(), Node.first},
            {Node.second->getSuccessors().end(), Node.first}),
        [](NodeRef N) {
          const VPLoop *VPL = N.first;
          return N.second != VPL->getHeader() && VPL->contains(N.second);
        });
  }

  static decltype(auto) child_begin(NodeRef Node) {
    return children(Node).begin();
  }
  static decltype(auto) child_end(NodeRef Node) { return children(Node).end(); }
};

struct VPLoopBodyTraits : public VPLoopBodyTraitsImpl {
  // Can't do this inside VPLoopBodyTraitsImpl as it will be incomplete and
  // declval won't work.
  using ChildIteratorType = decltype(std::declval<VPLoopBodyTraitsImpl>()
                                         .children(std::declval<NodeRef>())
                                         .begin());
};
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANLOOPITERATOR_H
