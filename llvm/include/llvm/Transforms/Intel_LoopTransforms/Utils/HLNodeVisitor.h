//===---------- HLNodeVisitor.h - Visitor class for HIR ---------*- C++ -*-===//
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
// This file defines the visitor class for HIR.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HLNODEVISITOR_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HLNODEVISITOR_H

#include "llvm/Support/Compiler.h"

#include "llvm/IR/Intel_LoopIR/HLRegion.h"
#include "llvm/IR/Intel_LoopIR/HLSwitch.h"
#include "llvm/IR/Intel_LoopIR/HLLabel.h"
#include "llvm/IR/Intel_LoopIR/HLGoto.h"
#include "llvm/IR/Intel_LoopIR/HLInst.h"
#include "llvm/IR/Intel_LoopIR/HLIf.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"

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
/// 4) bool skipRecursion(HLNode *Node) for skipping recursion on Node. This is
///    checked after visit() has been called on Node.
///
/// Sample visitor class:
///
/// struct Visitor {
///   HLNode *SkipNode;
///
///   void visit(HLRegion* Region) { errs() << "visited region!\n"; }
///   void postVisit(HLRegion* Region) { }
///
///   void visit(HLLoop* Loop) { SkipNode = Loop; }
///   void postVisit(HLLoop* Loop) { }
///
///   void visit(HLIf* If) { errs() << "visited if!\n" }
///   void postVisit(HLIf* If) { }
///   void visit(HLSwitch* Switch) { errs() << "visited switch!\n" }
///   void postVisit(HLSwitch* Switch) { }
///   void visit(HLLabel* Label) { errs() << "visited label!\n" }
///   void visit(HLGoto* Goto) { errs() << "visited goto!\n" }
///   void visit(HLInst* Inst) { errs() << "visited instruction!\n" }
///
///   bool isDone() { return false; }
///
///   bool skipRecursion (HLNode *Node) { return Node == SkipNode; }
/// };
///
/// It is also possible to implement generic(catch-all) visit() functions for
/// HLNodes and specialize them for specific type, if desired. For example, if
/// an optimization only cares about loops, it can implement the visitor class
/// as follows:
///
/// struct Visitor {
///   void visit(HLLoop* Loop) { // implementation here }
///   void postVisit(HLLoop* Loop) { // implementation here }
///
///   void visit(HLNode* Node) { } // Empty catch-all function for others
///   void postVisit(HLNode* Node) { } // Empty catch-all function for others
///
///   bool isDone() { return false; }
///
///   bool skipRecursion (HLNode *Node) { return false; }
/// };
///
template <typename HV> class HLNodeVisitor {
private:
  HV &Visitor;

  friend class HLNodeUtils;

  HLNodeVisitor(HV &V) : Visitor(V) {}

  /// \brief Contains the core logic to visit nodes and recurse further.
  /// Returns true to indicate that early termination has occurred.
  /// Recursive parameter denotes if we want to visit inside the HLNodes
  /// such as HLIf and HLLoops. RecursiveInsideLoops parameter denotes
  /// whether we want to visit inside the loops or not and this parameter
  /// is only useful if Recursive parameter is true.
  bool visit(HLNode *Node, bool Recursive, bool RecurseInsideLoops,
             bool Forward);

  /// \brief Visits HLNodes in the forward direction in the range [begin, end).
  /// Returns true to indicate that early termination has occurred.
  bool forwardVisit(HLContainerTy::iterator Begin, HLContainerTy::iterator End,
                    bool Recursive, bool RecurseInsideLoops);

  /// \brief Visits HLNodes in the backward direction in the range [begin, end).
  /// Returns true to indicate that early termination has occurred.
  /// RecursiveInsideLoops parameter denotes whether we want to visit inside the
  /// loops or not and this parameter
  /// is only useful if Recursive parameter is true.
  bool backwardVisit(HLContainerTy::iterator Begin, HLContainerTy::iterator End,
                     bool Recursive, bool RecurseInsideLoops);

  /// \brief Visits all HLNodes in the HIR in forward direction.
  void forwardVisitAll(HIRParser *HIRP);
  /// \brief Visits all HLNodes in the HIR in backward direction.
  void backwardVisitAll(HIRParser *HIRP);
};

template <typename HV>
bool HLNodeVisitor<HV>::forwardVisit(HLContainerTy::iterator Begin,
                                     HLContainerTy::iterator End,
                                     bool Recursive, bool RecurseInsideLoops) {

  for (auto I = Begin, Next = I, E = End; I != E; I = Next) {

    ++Next;

    if (visit(I, Recursive, RecurseInsideLoops, true)) {
      return true;
    }
  }

  return false;
}

template <typename HV>
bool HLNodeVisitor<HV>::backwardVisit(HLContainerTy::iterator Begin,
                                      HLContainerTy::iterator End,
                                      bool Recursive, bool RecurseInsideLoops) {

  HLContainerTy::reverse_iterator RI(End), RE(Begin);

  /// Change direction and iterate backwards
  for (auto RNext = RI; RI != RE; RI = RNext) {

    ++RNext;

    if (visit(&(*(RI)), Recursive, RecurseInsideLoops, false)) {
      return true;
    }
  }

  return false;
}

template <typename HV>
void HLNodeVisitor<HV>::forwardVisitAll(HIRParser *HIRP) {
  forwardVisit(HIRP->hir_begin(), HIRP->hir_end(), true, true);
}

template <typename HV>
void HLNodeVisitor<HV>::backwardVisitAll(HIRParser *HIRP) {
  backwardVisit(HIRP->hir_begin(), HIRP->hir_end(), true, true);
}

template <typename HV>
bool HLNodeVisitor<HV>::visit(HLNode *Node, bool Recursive,
                              bool RecurseInsideLoops, bool Forward) {

  bool Ret;

  if (auto Reg = dyn_cast<HLRegion>(Node)) {

    Visitor.visit(Reg);

    if (Recursive && !Visitor.skipRecursion(Node) && !Visitor.isDone()) {
      Ret = Forward ? forwardVisit(Reg->child_begin(), Reg->child_end(),
                                   RecurseInsideLoops, true)
                    : backwardVisit(Reg->child_begin(), Reg->child_end(),
                                    RecurseInsideLoops, true);

      if (Ret) {
        return true;
      }

      Visitor.postVisit(Reg);
    }
  } else if (auto If = dyn_cast<HLIf>(Node)) {

    Visitor.visit(If);

    if (Recursive && !Visitor.skipRecursion(Node) && !Visitor.isDone()) {
      Ret = Forward ? forwardVisit(If->then_begin(), If->then_end(),
                                   RecurseInsideLoops, true)
                    : backwardVisit(If->else_begin(), If->else_end(),
                                    RecurseInsideLoops, true);

      if (Ret) {
        return true;
      }

      Ret = Forward ? forwardVisit(If->else_begin(), If->else_end(),
                                   RecurseInsideLoops, true)
                    : backwardVisit(If->then_begin(), If->then_end(),
                                    RecurseInsideLoops, true);

      if (Ret) {
        return true;
      }

      Visitor.postVisit(If);
    }
  } else if (auto Loop = dyn_cast<HLLoop>(Node)) {

    Visitor.visit(Loop);

    if (Recursive && RecurseInsideLoops && !Visitor.skipRecursion(Node) &&
        !Visitor.isDone()) {
      Ret =
          Forward
              ? forwardVisit(Loop->pre_begin(), Loop->pre_end(), true, true)
              : backwardVisit(Loop->post_begin(), Loop->post_end(), true, true);

      if (Ret) {
        return true;
      }

      Ret = Forward ? forwardVisit(Loop->child_begin(), Loop->child_end(), true,
                                   true)
                    : backwardVisit(Loop->child_begin(), Loop->child_end(),
                                    true, true);

      if (Ret) {
        return true;
      }

      Ret = Forward
                ? forwardVisit(Loop->post_begin(), Loop->post_end(), true, true)
                : backwardVisit(Loop->pre_begin(), Loop->pre_end(), true, true);

      if (Ret) {
        return true;
      }

      Visitor.postVisit(Loop);
    }
  } else if (auto Switch = dyn_cast<HLSwitch>(Node)) {

    Visitor.visit(Switch);

    if (Recursive && !Visitor.skipRecursion(Node) && !Visitor.isDone()) {

      if (Forward) {
        for (unsigned I = 1, E = Switch->getNumCases(); I <= E; ++I) {
          if (forwardVisit(Switch->case_child_begin(I),
                           Switch->case_child_end(I), RecurseInsideLoops,
                           true)) {
            return true;
          }
        }

        if (forwardVisit(Switch->default_case_child_begin(),
                         Switch->default_case_child_end(), RecurseInsideLoops,
                         true)) {
          return true;
        }

      } else {

        if (backwardVisit(Switch->default_case_child_begin(),
                          Switch->default_case_child_end(), RecurseInsideLoops,
                          true)) {
          return true;
        }

        for (unsigned I = Switch->getNumCases(), E = 0; I > E; --I) {
          if (backwardVisit(Switch->case_child_begin(I),
                            Switch->case_child_end(I), RecurseInsideLoops,
                            true)) {
            return true;
          }
        }
      }

      Visitor.postVisit(Switch);
    }
  } else if (auto Label = dyn_cast<HLLabel>(Node)) {
    Visitor.visit(Label);
  } else if (auto Goto = dyn_cast<HLGoto>(Node)) {
    Visitor.visit(Goto);
  } else if (auto Inst = dyn_cast<HLInst>(Node)) {
    Visitor.visit(Inst);
  } else {
    llvm_unreachable("Unknown HLNode type!");
  }

  /// Visitor indicated that the traversal is done
  if (Visitor.isDone()) {
    return true;
  }

  return false;
}

} // End namespace loopopt

} // End namespace llvm

#endif
