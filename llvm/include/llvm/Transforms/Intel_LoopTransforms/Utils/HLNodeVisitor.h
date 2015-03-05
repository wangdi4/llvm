//===---------- HLNodeVisitor.h - Visitor class for HIR ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the visitor class for HIR.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HLNODEVISITOR_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HLNODEVISITOR_H

#include "llvm/Support/Compiler.h"
#include "llvm/IR/Intel_LoopIR/HLNode.h"
#include "llvm/IR/Intel_LoopIR/HLRegion.h"
#include "llvm/IR/Intel_LoopIR/HLSwitch.h"
#include "llvm/IR/Intel_LoopIR/HLLabel.h"
#include "llvm/IR/Intel_LoopIR/HLGoto.h"
#include "llvm/IR/Intel_LoopIR/HLInst.h"
#include "llvm/IR/Intel_LoopIR/HLIf.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"

namespace llvm {

namespace loopopt {

/// \brief This class is used to visit HIR nodes.
///
/// The forward/backward traversal works even if the current iterator is
/// invalidated (removed/replaced) as the next/prev iterator is saved so it
/// should work for most transformations. Specialized traversals might be needed
/// otherwise.
///
/// The public wrapper functions are in HLNodeUtils.h.
//
/// Visitor (template class HV) needs to implement:
///
/// 1) Various visit( HLNodeType* ) functions.
/// 2) Various postVisit( HLNodeType* ) functions for node types which
///    can contain other nodes. These are only needed for recursive walks and
///    are called after we finish visiting the children of the node.
/// 3) bool isDone() for early termination of the traversal.
///
/// Sample visitor class:
///
/// struct Visitor {
///   void visit(HLRegion* Region) { errs() << "visited region!\n"; }
///   void postVisit(HLRegion* Region) { }
///   void visit(HLLoop* Loop) { errs() << "visited loop!\n"; }
///   void postVisit(HLLoop* Loop) { }
///   void visit(HLIf* If) { errs() << "visited if!\n" }
///   void postVisit(HLIf* If) { }
///   void visit(HlSwitch* Switch) { errs() << "visited switch!\n" }
///   void visit(HLLabel* Label) { errs() << "visited label!\n" }
///   void visit(HLGoto* Goto) { errs() << "visited goto!\n" }
///   void visit(HLInst* Inst) { errs() << "visited instruction!\n" }
///   bool isDone() { return false; }
/// };
///
template <typename HV> class HLNodeVisitor {
private:
  HV *Visitor;

  friend class HLNodeUtils;

  HLNodeVisitor(HV *V) : Visitor(V) {}

  /// \brief Contains the core logic to visit nodes and recurse further.
  /// Returns true to indicate that early termination has occured.
  bool visit(HLNode *Node, bool Recursive, bool Forward);

  /// \brief Visits HLNodes in the forward direction in the range [begin, end).
  /// Returns true to indicate that early termination has occured.
  bool forwardVisit(HLContainerTy::iterator Begin, HLContainerTy::iterator End,
                    bool Recursive);

  /// \brief Visits HLNodes in the backward direction in the range [begin, end).
  /// Returns true to indicate that early termination has occured.
  bool backwardVisit(HLContainerTy::iterator Begin, HLContainerTy::iterator End,
                     bool Recursive);

  /// \brief Visits all HLNodes in the HIR in forward direction.
  void forwardVisitAll();
  /// \brief Visits all HLNodes in the HIR in backward direction.
  void backwardVisitAll();
};

template <typename HV>
bool HLNodeVisitor<HV>::forwardVisit(HLContainerTy::iterator Begin,
                                     HLContainerTy::iterator End,
                                     bool Recursive) {

  for (auto I = Begin, Next = I, E = End; I != E; I = Next) {

    Next++;

    if (visit(I, Recursive, true)) {
      return true;
    }
  }

  return false;
}

template <typename HV>
bool HLNodeVisitor<HV>::backwardVisit(HLContainerTy::iterator Begin,
                                      HLContainerTy::iterator End,
                                      bool Recursive) {

  HLContainerTy::reverse_iterator RI(End), RE(Begin);

  /// Change direction and iterate backwards
  for (auto RNext = RI; RI != RE; RI = RNext) {

    RNext++;

    if (visit(&(*(RI)), Recursive, false)) {
      return true;
    }
  }

  return false;
}

template <typename HV> void HLNodeVisitor<HV>::forwardVisitAll() {
  forwardVisit(HLRegions.begin(), HLRegions.end(), true);
}

template <typename HV> void HLNodeVisitor<HV>::backwardVisitAll() {
  backwardVisit(HLRegions.begin(), HLRegions.end(), true);
}

template <typename HV>
bool HLNodeVisitor<HV>::visit(HLNode *Node, bool Recursive, bool Forward) {

  bool Ret;

  if (isa<HLRegion>(Node)) {
    HLRegion *Reg = cast<HLRegion>(Node);

    Visitor->visit(Reg);

    if (Recursive && !Visitor->isDone()) {
      Ret = Forward ? forwardVisit(Reg->child_begin(), Reg->child_end(), true)
                    : backwardVisit(Reg->child_begin(), Reg->child_end(), true);

      if (Ret) {
        return true;
      }

      Visitor->postVisit(Reg);
    }
  } else if (isa<HLSwitch>(Node)) {
    Visitor->visit(cast<HLSwitch>(Node));
  } else if (isa<HLLabel>(Node)) {
    Visitor->visit(cast<HLLabel>(Node));
  } else if (isa<HLGoto>(Node)) {
    Visitor->visit(cast<HLGoto>(Node));
  } else if (isa<HLInst>(Node)) {
    Visitor->visit(cast<HLInst>(Node));
  } else if (isa<HLIf>(Node)) {
    HLIf *If = cast<HLIf>(Node);

    Visitor->visit(If);

    if (Recursive && !Visitor->isDone()) {
      Ret = Forward ? forwardVisit(If->then_begin(), If->then_end(), true)
                    : backwardVisit(If->else_begin(), If->else_end(), true);

      if (Ret) {
        return true;
      }

      Ret = Forward ? forwardVisit(If->else_begin(), If->else_end(), true)
                    : backwardVisit(If->then_begin(), If->then_end(), true);

      if (Ret) {
        return true;
      }

      Visitor->postVisit(If);
    }
  } else if (isa<HLLoop>(Node)) {
    HLLoop *Loop = cast<HLLoop>(Node);

    Visitor->visit(Loop);

    if (Recursive && !Visitor->isDone()) {
      Ret = Forward ? forwardVisit(Loop->pre_begin(), Loop->pre_end(), true)
                    : backwardVisit(Loop->post_begin(), Loop->post_end(), true);

      if (Ret) {
        return true;
      }

      Ret = Forward
                ? forwardVisit(Loop->child_begin(), Loop->child_end(), true)
                : backwardVisit(Loop->child_begin(), Loop->child_end(), true);

      if (Ret) {
        return true;
      }

      Ret = Forward ? forwardVisit(Loop->post_begin(), Loop->post_end(), true)
                    : backwardVisit(Loop->pre_begin(), Loop->pre_end(), true);

      if (Ret) {
        return true;
      }

      Visitor->postVisit(Loop);
    }
  } else {
    llvm_unreachable("Unknown HLNode type!");
  }

  /// Visitor indicated that the traversal is done
  if (Visitor->isDone()) {
    return true;
  }

  return false;
}

} // End namespace loopopt

} // End namespace llvm

#endif
