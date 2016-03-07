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

template <typename HV,
          bool InnerToOuter,
          bool Forward = true>
class HLLoopVisitor {
private:
  HV &Visitor;

  friend class HLNodeUtils;

  HLLoopVisitor(HV &V) : Visitor(V) {}

  /// \brief Contains the core logic to visit nodes and recurse further.
  /// Returns true to indicate that early termination has occurred.
  /// Recursive parameter denotes if we want to visit inside the HLNodes
  /// such as HLIf and HLLoops. RecursiveInsideLoops parameter denotes
  /// whether we want to visit inside the loops or not and this parameter
  /// is only useful if Recursive parameter is true.
  template <bool RecurseInsideLoops,
            typename NodeTy, typename = IsHLNodeTy<NodeTy> >
  bool visit(NodeTy *Node){
    bool Ret;

    if (auto Reg = dyn_cast<HLRegion>(Node)) {
      Visitor.visit(Reg);
      if (!Visitor.skipRecursion(Node) && !Visitor.isDone()) {
        Ret = visit<RecurseInsideLoops>(Reg->child_begin(),
                                                Reg->child_end());
        if (Ret) {
          return true;
        }
      }
      Visitor.postVisit(Reg);
    } else if (auto If = dyn_cast<HLIf>(Node)) {
      if (!RecurseInsideLoops) Visitor.visit(If);
      if (!Visitor.skipRecursion(Node) && !Visitor.isDone()) {
        auto Begin1 = Forward ? If->then_begin() : If->else_begin();
        auto Begin2 = Forward ? If->else_begin() : If->then_begin();
        auto End1   = Forward ? If->then_end()   : If->else_end();
        auto End2   = Forward ? If->else_end()   : If->then_end();

        Ret = visit<RecurseInsideLoops>(Begin1, End1);
        if (Ret) {
          return true;
        }

        Ret = visit<RecurseInsideLoops>(Begin2, End2);
        if (Ret) {
          return true;
        }
      }
      if (!RecurseInsideLoops) Visitor.postVisit(If);
    } else if (auto Loop = dyn_cast<HLLoop>(Node)) {
      auto Begin1 = Forward ? Loop->pre_begin()  : Loop->post_begin();
      auto Begin2 = Forward ? Loop->post_begin() : Loop->pre_begin();
      auto End1   = Forward ? Loop->pre_end()    : Loop->post_end();
      auto End2   = Forward ? Loop->post_end()   : Loop->pre_end();

      Ret = visit<RecurseInsideLoops>(Begin1, End1);
      if (Ret) {
        return true;
      }

      if (!InnerToOuter && !RecurseInsideLoops) Visitor.visit(Loop);

      if (RecurseInsideLoops && !Visitor.skipRecursion(Node) &&
          !Visitor.isDone()) {
        Ret = visit<InnerToOuter>(Loop->child_begin(),
                                          Loop->child_end());
        if (Ret) {
          return true;
        }
        if (InnerToOuter) Visitor.visit(Loop);
        Ret = visit<!InnerToOuter>(Loop->child_begin(),
                                           Loop->child_end());
        if (Ret) {
          return true;
        }
        Visitor.postVisit(Loop);
      }
      Ret = visit<RecurseInsideLoops>(Begin2, End2);
      if (Ret) {
        return true;
      }
    } else if (auto Switch = dyn_cast<HLSwitch>(Node)) {
      if (!RecurseInsideLoops) Visitor.visit(Switch);
      if (!Visitor.skipRecursion(Node) && !Visitor.isDone()) {
        if (!Forward) {
          if (visit<RecurseInsideLoops>(Switch->default_case_child_begin(),
                    Switch->default_case_child_end()))
            return true;
        }
        for (unsigned I = 1, E = Switch->getNumCases(); I <= E; ++I) {
          unsigned I1 = Forward ? I : E - I + 1;
          if (visit<RecurseInsideLoops>(Switch->case_child_begin(I1),
                    Switch->case_child_end(I1)))
            return true;
        }
        if (Forward) {
          if (visit<RecurseInsideLoops>(Switch->default_case_child_begin(),
                    Switch->default_case_child_end()))
            return true;
        }
      }
      if (!RecurseInsideLoops) Visitor.postVisit(Switch);
    } else if (auto Label = dyn_cast<HLLabel>(Node)) {
      if (!RecurseInsideLoops) Visitor.visit(Label);
    } else if (auto Goto = dyn_cast<HLGoto>(Node)) {
      if (!RecurseInsideLoops) Visitor.visit(Goto);
    } else if (auto Inst = dyn_cast<HLInst>(Node)) {
      if (!RecurseInsideLoops) Visitor.visit(Inst);
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
  /// occurred. RecursiveInsideLoops parameter denotes whether we want to
  /// visit inside the loops or not.
  template <bool RecurseInsideLoops,
            typename NodeTy, typename = IsHLNodeTy<NodeTy> >
  bool visit(ilist_iterator<NodeTy> Begin, ilist_iterator<NodeTy> End) {
    if (Forward) {
      for (auto I = Begin, Next = I, E = End; I != E; I = Next) {
        ++Next;
        if (visit<RecurseInsideLoops>(&(*I))) {
          return true;
        }
      }
    }
    else {
      std::reverse_iterator<decltype(Begin)> RI(End);
      std::reverse_iterator<decltype(End)> RE(Begin);

      for (auto I = RI, Next = I, E = RE; I != E; I = Next) {

        ++Next;

        if (visit<RecurseInsideLoops>(&(*I))) {
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
