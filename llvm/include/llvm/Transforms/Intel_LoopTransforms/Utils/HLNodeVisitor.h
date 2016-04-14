//===---------- HLNodeVisitor.h - Visitor class for HIR ---------*- C++ -*-===//
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
///
/// Visitor (template class HV derived from HLNodeVisitorBase)
/// needs to implement:
///
/// 1) Various visit( HLNodeType* ) functions.
/// 2) Various postVisit( HLNodeType* ) functions for node types which
///    can contain other nodes. These are only needed for recursive walks and
///    are called after we finish visiting the children of the node.
/// 3) Optional: bool isDone() for early termination of the traversal.
/// 4) Optional: bool skipRecursion(HLNode *Node) for skipping
///    recursion on Node. This is checked after visit() has been called on Node.
///
/// Sample visitor class:
///
/// struct Visitor final : public HLNodeVisitorBase {
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
///   bool isDone() override { return false; }
///
///   bool skipRecursion override (HLNode *Node) { return Node == SkipNode; }
/// };
///
/// It is also possible to implement generic(catch-all) visit() functions for
/// HLNodes and specialize them for specific type, if desired. For example, if
/// an optimization only cares about loops, it can implement the visitor class
/// as follows:
///
/// struct Visitor final : public HLNodeVisitorBase {
///   void visit(HLLoop* Loop) { // implementation here }
///   void postVisit(HLLoop* Loop) { // implementation here }
///
///   void visit(HLNode* Node) { } // Empty catch-all function for others
///   void postVisit(HLNode* Node) { } // Empty catch-all function for others
/// };
///
/// Recursive parameter denotes if we want to visit inside the HLNodes
/// such as HLIf and HLLoops. RecursiveInsideLoops parameter denotes
/// whether we want to visit inside the loops or not and this parameter
/// is only useful if Recursive parameter is true.

template <typename T>
using IsHLNodeTy =
    typename std::enable_if<std::is_base_of<HLNode, T>::value>::type;

struct HLNodeVisitorBase {
  virtual bool isDone() const { return false; }
  virtual bool skipRecursion(const HLNode *Node) const { return false; }
};

template <typename HV, bool Recursive = true, bool RecurseInsideLoops = true,
          bool Forward = true>
class HLNodeVisitor {

  // TODO: if C++14 would be available, std::is_final can be used
  static_assert(std::is_base_of<HLNodeVisitorBase, HV>::value,
                "HV must be a final derivative of HLNodeVisitorBase");

private:
  HV &Visitor;

  friend class HLNodeUtils;

  HLNodeVisitor(HV &V) : Visitor(V) {}

  /// \brief Contains the core logic to visit nodes and recurse further.
  /// Returns true to indicate that early termination has occurred.
  template <typename NodeTy, typename = IsHLNodeTy<NodeTy>>
  bool visit(NodeTy *Node) {
    bool Ret;

    if (auto Reg = dyn_cast<HLRegion>(Node)) {

      Visitor.visit(Reg);

      if (Recursive && !Visitor.skipRecursion(Node) && !Visitor.isDone()) {
        Ret = visitRange(Reg->child_begin(), Reg->child_end());
        if (Ret) {
          return true;
        }

        Visitor.postVisit(Reg);
      }
    } else if (auto If = dyn_cast<HLIf>(Node)) {

      Visitor.visit(If);

      if (Recursive && !Visitor.skipRecursion(Node) && !Visitor.isDone()) {
        Ret = Forward ? visitRange(If->then_begin(), If->then_end())
                      : visitRange(If->else_begin(), If->else_end());

        if (Ret) {
          return true;
        }

        Ret = Forward ? visitRange(If->else_begin(), If->else_end())
                      : visitRange(If->then_begin(), If->then_end());

        if (Ret) {
          return true;
        }

        Visitor.postVisit(If);
      }
    } else if (auto Loop = dyn_cast<HLLoop>(Node)) {

      Visitor.visit(Loop);

      if (Recursive && RecurseInsideLoops && !Visitor.skipRecursion(Node) &&
          !Visitor.isDone()) {
        Ret = Forward ? visitRange(Loop->pre_begin(), Loop->pre_end())
                      : visitRange(Loop->post_begin(), Loop->post_end());

        if (Ret) {
          return true;
        }

        Ret = visitRange(Loop->child_begin(), Loop->child_end());

        if (Ret) {
          return true;
        }

        Ret = Forward ? visitRange(Loop->post_begin(), Loop->post_end())
                      : visitRange(Loop->pre_begin(), Loop->pre_end());

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
            if (visitRange(Switch->case_child_begin(I),
                           Switch->case_child_end(I))) {
              return true;
            }
          }

          if (visitRange(Switch->default_case_child_begin(),
                         Switch->default_case_child_end())) {
            return true;
          }

        } else {

          if (visitRange(Switch->default_case_child_begin(),
                         Switch->default_case_child_end())) {
            return true;
          }

          for (unsigned I = Switch->getNumCases(), E = 0; I > E; --I) {
            if (visitRange(Switch->case_child_begin(I),
                           Switch->case_child_end(I))) {
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

  /// \brief Visits HLNodes in the specified direction in the range [begin,
  /// end).
  /// Returns true to indicate that early termination has occurred.
  template <typename NodeTy, template <typename> class It,
            typename = IsHLNodeTy<NodeTy>>
  bool visitRange(It<NodeTy> Begin, It<NodeTy> End) {
    if (Forward) {
      for (auto I = Begin, Next = I, E = End; I != E; I = Next) {

        ++Next;

        if (visit<NodeTy>(&(*I))) {
          return true;
        }
      }
    } else {
      std::reverse_iterator<decltype(Begin)> RI(End);
      std::reverse_iterator<decltype(End)> RE(Begin);

      for (auto I = RI, Next = I, E = RE; I != E; I = Next) {

        ++Next;

        if (visit<NodeTy>(&(*I))) {
          return true;
        }
      }
    }
    return false;
  }
};

///
/// \brief This class visits HLNodes, in inner loop first manner.
/// It first visits the loop body in "recurse inside loop" mode,
/// such that inner loops are processed first. Then, in the "not
/// recurse inside loop" mode to visit the current loop body only.
/// Visitor class's visit() function for loop node is used to process
/// the loop node itself before its loop body is processed in "not recurse"
/// mode, while the positVisit() is used to process the loop node as part
/// of the immediate parent loop body, after it's loop body is visited.
/// Nodes outside of outermost loops are not visited.
///
/// Suppose our HLNodes are the following.
///
/// R1
///   S1
///   IF1
///     S2
///   L1
///     IF2
///       S3
///       L2
///         S4
///       S5
///
/// The following is how the traversal happens (for simplicity, omitting
/// calls to visitRange*()).
/// 1) visitRecursiveInsideLoops(R1)
///   2) visitRecursiveInsideLoops(S1)
///   3) visitRecursiveInsideLoops(IF1)
///     4) visitRecursiveInsideLoops(S2)
///   5) visitRecursiveInsideLoops(L1)
///     5-1) visitRecursiveInsideLoops(IF2)
///       5-2) visitRecursiveInsideLoops(S3)
///       5-3) visitRecursiveInsideLoops(L2)
///         5-3-1) visitRecursiveInsideLoops(S4)
///       5-3-2) calls V.visit(L2)
///         5-3-3) visit(S4), which calls V.visit(S4)
///       5-4) visitRecursiveInsideLoops(S5)
///     5-5) calls V.visit(L1)
///       5-6) visit(IF2), which calls V.visit(IF2)
///         5-7) visit(S3), which calls V.visit(S3)
///         5-8) visit(L2), which calls V.postVisit(L2)  // as part of L1 body
///         5-9) visit(S5), which calls V.visit(S5)
///       5-10) calls V.postVisit(IF2) // after IF body visits.
///     5-11) calls V.postVisit(L1) // exisint from outermost loop
///
template <typename HV, bool Forward = true>
class HLInnerToOuterLoopVisitor {
private:
  HV &Visitor;

  friend class HLNodeUtils;

  HLInnerToOuterLoopVisitor(HV &V) : Visitor(V) {}

  /// \brief Contains the core logic to recurse as deep in the loop nest
  /// as possible first, and then on the way back visit the loop and loop body
  /// nodes. Returns true to indicate that early termination has occurred.
  template <typename NodeTy, typename = IsHLNodeTy<NodeTy> >
  bool visitRecurseInsideLoops(NodeTy *Node){
    if (auto Reg = dyn_cast<HLRegion>(Node)) {
      if (!Visitor.skipRecursion(Node) && !Visitor.isDone()) {
        if (visitRangeRecurseInsideLoops(Reg->child_begin(),Reg->child_end())) {
          return true;
        }
      }
    } else if (auto If = dyn_cast<HLIf>(Node)) {
      if (!Visitor.skipRecursion(Node) && !Visitor.isDone()) {
        auto Begin1 = Forward ? If->then_begin() : If->else_begin();
        auto Begin2 = Forward ? If->else_begin() : If->then_begin();
        auto End1   = Forward ? If->then_end()   : If->else_end();
        auto End2   = Forward ? If->else_end()   : If->then_end();
        if (visitRangeRecurseInsideLoops(Begin1, End1)) {
          return true;
        }
        if (visitRangeRecurseInsideLoops(Begin2, End2)) {
          return true;
        }
      }
    } else if (auto Loop = dyn_cast<HLLoop>(Node)) {
      // No need to visit preheader/postexit in this traversal mode.
      if (!Visitor.skipRecursion(Node) && !Visitor.isDone()) {
        // Visit the body in "recurse inside loop" mode first.
        // This is to hit all the loop nodes inside.
        if (visitRangeRecurseInsideLoops(Loop->child_begin(),
                                         Loop->child_end())) {
          return true;
        }
        Visitor.visit(Loop);  // Visit first as the parent of nodes whose
                              // parent loop is this one.
        if (!Visitor.isDone()) {
          return true;
        }
        // Call visit/postVisit function if the nodes' parent loop is this one.
        if (visitRange(Loop->child_begin(), Loop->child_end())) {
          return true;
        }
        // postVisit(Loop) for inner loops are performed as part of parent
        // loops' body node visit. For outermost loops, since there aren't
        // parent loops, postVisit() is performed here.
        if (!Loop->getParentLoop()) {
          Visitor.postVisit(Loop);
          if (!Visitor.isDone()) {
            return true;
          }
        }
      }
    } else if (auto Switch = dyn_cast<HLSwitch>(Node)) {
      if (!Visitor.skipRecursion(Node) && !Visitor.isDone()) {
        if (!Forward) {
          if (visitRangeRecurseInsideLoops(Switch->default_case_child_begin(),
                                           Switch->default_case_child_end()))
            return true;
        }
        for (unsigned I = 1, E = Switch->getNumCases(); I <= E; ++I) {
          unsigned I1 = Forward ? I : E - I + 1;
          if (visitRangeRecurseInsideLoops(Switch->case_child_begin(I1),
                                           Switch->case_child_end(I1)))
            return true;
        }
        if (Forward) {
          if (visitRangeRecurseInsideLoops(Switch->default_case_child_begin(),
                                           Switch->default_case_child_end()))
            return true;
        }
      }
    }
    return false;
  }

  /// \brief Contains the core logic to visit nodes and recurse further
  /// on HLIf and HLSwitch. Recursion inside loop is handled by
  /// visitRecurseInsideLoops(). Returns true to indicate that early
  /// termination has occurred.
  template <typename NodeTy, typename = IsHLNodeTy<NodeTy> >
  bool visit(NodeTy *Node){
    if (isa<HLRegion>(Node)) {
      llvm_unreachable("HLRegion node unexpected!");
    } else if (auto If = dyn_cast<HLIf>(Node)) {
      Visitor.visit(If);
      if (!Visitor.skipRecursion(Node) && !Visitor.isDone()) {
        auto Begin1 = Forward ? If->then_begin() : If->else_begin();
        auto Begin2 = Forward ? If->else_begin() : If->then_begin();
        auto End1   = Forward ? If->then_end()   : If->else_end();
        auto End2   = Forward ? If->else_end()   : If->then_end();
        if (visitRange(Begin1, End1)) {
          return true;
        }
        if (visitRange(Begin2, End2)) {
          return true;
        }
      }
      Visitor.postVisit(If);
    } else if (auto Loop = dyn_cast<HLLoop>(Node)) {
      auto Begin1 = Forward ? Loop->pre_begin()  : Loop->post_begin();
      auto Begin2 = Forward ? Loop->post_begin() : Loop->pre_begin();
      auto End1   = Forward ? Loop->pre_end()    : Loop->post_end();
      auto End2   = Forward ? Loop->post_end()   : Loop->pre_end();
      if (visitRange(Begin1, End1)) {
        return true;
      }
      Visitor.postVisit(Loop);  // visit the loop node as part of parent loop's
                                // body node, also after all it's body bodes are
                                // processed.
      if (!Visitor.isDone()) {
        return true;
      }
      if (visitRange(Begin2, End2)) {
        return true;
      }
    } else if (auto Switch = dyn_cast<HLSwitch>(Node)) {
      Visitor.visit(Switch);
      if (!Visitor.skipRecursion(Node) && !Visitor.isDone()) {
        if (!Forward) {
          if (visitRange(Switch->default_case_child_begin(),
                         Switch->default_case_child_end()))
            return true;
        }
        for (unsigned I = 1, E = Switch->getNumCases(); I <= E; ++I) {
          unsigned I1 = Forward ? I : E - I + 1;
          if (visitRange(Switch->case_child_begin(I1),
                         Switch->case_child_end(I1)))
            return true;
        }
        if (Forward) {
          if (visitRange(Switch->default_case_child_begin(),
                         Switch->default_case_child_end()))
            return true;
        }
      }
      Visitor.postVisit(Switch);
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

  /// \brief Visits HLNodes in the forward/backward direction in the range
  /// [begin, end). Returns true to indicate that early termination has
  /// occurred.
  template <typename NodeTy, typename = IsHLNodeTy<NodeTy> >
  bool visitRangeRecurseInsideLoops(ilist_iterator<NodeTy> Begin,
                                    ilist_iterator<NodeTy> End) {
    if (Forward) {
      for (auto I = Begin, Next = I, E = End; I != E; I = Next) {
        ++Next;
        if (visitRecurseInsideLoops(&(*I))) {
          return true;
        }
      }
    }
    else {
      std::reverse_iterator<decltype(Begin)> RI(End);
      std::reverse_iterator<decltype(End)> RE(Begin);
      for (auto I = RI, Next = I, E = RE; I != E; I = Next) {
        ++Next;
        if (visitRecurseInsideLoops(&(*I))) {
          return true;
        }
      }
    }
    return false;
  }

  /// \brief Visits HLNodes in the forward/backward direction in the range
  /// [begin, end). Returns true to indicate that early termination has
  /// occurred.
  template <typename NodeTy, typename = IsHLNodeTy<NodeTy> >
  bool visitRange(ilist_iterator<NodeTy> Begin, ilist_iterator<NodeTy> End) {
    if (Forward) {
      for (auto I = Begin, Next = I, E = End; I != E; I = Next) {
        ++Next;
        if (visit(&(*I))) {
          return true;
        }
      }
    }
    else {
      std::reverse_iterator<decltype(Begin)> RI(End);
      std::reverse_iterator<decltype(End)> RE(Begin);
      for (auto I = RI, Next = I, E = RE; I != E; I = Next) {
        ++Next;
        if (visit(&(*I))) {
          return true;
        }
      }
    }
    return false;
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
