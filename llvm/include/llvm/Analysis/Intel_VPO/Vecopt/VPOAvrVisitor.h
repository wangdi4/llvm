//===-- VPOAvrVisitor.h -----------------------------------------*- C++ -*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the visitor template class for visiting AVR nodes.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_VISITOR_H
#define LLVM_ANALYSIS_VPO_AVR_VISITOR_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvr.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrFunction.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrLoop.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrStmt.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrStmtIR.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrIf.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrSwitch.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrGenerate.h"
#include "llvm/Support/Compiler.h"

namespace llvm {
namespace vpo {

typedef AVRContainerTy::iterator AvrItr;

/// \brief This class is used to visit AVR nodes.
///
/// The forward/backward traversal works even if the current iterator is
/// invalidated (removed/replaced) as the next/prev iterator is saved so it
/// should work for most transformations. Specialized traversals might be needed
/// otherwise.
///
/// The public wrapper functions are in VPOAvrUtils.h.
///
/// Visitor (template class AV) needs to implement:
///
/// 1) Various visit(AVRType*) functions.
/// 2) Various postVisit(AVRType*) functions for node types which
///    can contain other nodes. These are only needed for recursive walks and
///    are called after we finish visiting the children of the node.
/// 3) bool isDone() for early termination of the traversal.
/// 4) bool skipRecursion(AVR *Node) for skipping recursion on Node. This is
///    checked after visit() has been called on Node.
///
/// Sample visitor class:
///
/// struct Visitor {
///   AVR *SkipNode;
///
///   void visit(AVRFunction* AFunction) { errs() << "visited function!\n"; }
///   void postVisit(AVRFunction* AFunction) { }
///
///   void visit(AVRLoop* Loop) { SkipNode = Loop; }
///   void postVisit(AVRLoop* Loop) { }
///
///   void visit(AVRIf* If) { errs() << "visited if!\n" }
///   void postVisit(AVRIf* If) { }
///   void visit(AVRWrn* AWrn ) { errs() << "visited wrn!\n" }
///   void postVisit(AVRWrn* AWrn) { }
///   void visit(AVRLabel* Label) { errs() << "visited label!\n" }
///   void visit(AVRAssign* Assign) { errs() << "visited assign!\n" }
///   void visit(AVRPhi* Phi) { errs() << "visited phi!\n" }
///   void visit(AVRCall* Call) { errs() << "visited call!\n" }
///   void visit(AVRBranch* BR) { errs() << "visited branch!\n" }
///   void visit(AVRBackEdge* BE) { errs() << "visited back edge!\n" }
///   void visit(AVREntry* Entry) { errs() << "visited entry!\n" }
///   void visit(AVRReturn* Return) { errs() << "visited return!\n" }
///   void visit(AVRExpr* Expr) { errs() << "visited expr!\n" }
///
///   bool isDone() { return false; }
///
///   bool skipRecursion (AVR *Node) { return Node == SkipNode; }
/// };
///
// The visit/postVisit functions can be specialized to the IR/HIR
/// implementations, for example:
///
///   void visit(AVRAssign* Assign) { errs() << "visited assign!\n" }
///   void visit(AVRAssignIR* AssignIR) { errs() << "visited assign IR!\n" }
///   void visit(AVRAssignHIR* AssignHIR) { errs() << "visited assign HIR!\n" }
///
/// It is also possible to implement generic(catch-all) visit() functions for
/// AVRs and specialize them for specific type, if desired. For example, if
/// an optimization only cares about loops, it can implement the visitor class
/// as follows:
///
/// struct Visitor {
///   void visit(AVRLoop* Loop) { // implementation here }
///   void postVisit(AVRLoop* Loop) { // implementation here }
///
///   void visit(AVR* Node) { } // Empty catch-all function for others
///   void postVisit(AVR* Node) { } // Empty catch-all function for others
///
///   bool isDone() { return false; }
///
///   bool skipRecursion (AVR* Node) { return false; }
/// };
///
template <typename AV> class AVRVisitor {

private:
  AV &Visitor;

  friend class AVRUtils;

  /// \brief A template function for calling Visitor.visit() on one of a list
  /// of AVR types (the first to match). If the parameter of type T can be
  /// dyn_cast'ed to type I then visit is called on I. Otherwise we recurse on
  /// the next type in the list.
  template <typename T, typename I, typename... Tail>
  inline void callVisit(T *Avr) {
    if (I *AvrI = dyn_cast<I>(Avr))
      Visitor.visit(AvrI);
    else
      callVisit<T, Tail...>(Avr);
  }

  /// \brief A template function for calling Visitor.visit() on some AVR type.
  template <typename T> inline void callVisit(T *Avr) { Visitor.visit(Avr); }

  /// \brief A template function for calling Visitor.postVisit() on one of a
  /// list of AVR types (the first to match). If the parameter of type T can be
  /// dyn_cast'ed to type I then visit is called on I. Otherwise we recurse on
  /// the next type in the list.
  template <typename T, typename I, typename... Tail>
  inline void callPostVisit(T *Avr) {
    if (I *AvrI = dyn_cast<I>(Avr))
      Visitor.postVisit(AvrI);
    else
      callPostVisit<T, Tail...>(Avr);
  }

  /// \brief A template function for calling Visitor.postVisit() on some AVR
  /// type.
  template <typename T> inline void callPostVisit(T *Avr) {
    Visitor.postVisit(Avr);
  }

public:
  AVRVisitor(AV &V) : Visitor(V) {}

  /// \brief Contains the core logic to visit nodes and recurse further.
  /// Returns true to indicate that early termination has occurred.
  /// Recursive parameter denotes if we want to visit inside the AvrNodes
  /// such as AVRIf and AVRLoops. RecursiveInsideLoops parameter denotes
  /// whether we want to visit inside the loops or not and this parameter
  /// is only useful if Recursive parameter is true.
  bool visit(AVR *Node, bool Recursive, bool RecurseInsideLoops, bool Forward);

  /// \brief Visits AVRs in the forward direction in the range [begin, end).
  /// Returns true to indicate that early termination has occurred.
  bool forwardVisit(AvrItr Begin, AvrItr End, bool Recursive,
                    bool RecurseInsideLoops);

  /// \brief Visits AVRs in the backward direction in the range [begin, end).
  /// Returns true to indicate that early termination has occurred.
  /// RecursiveInsideLoops parameter denotes whether we want to visit inside the
  /// loops or not and this parameter
  /// is only useful if Recursive parameter is true.
  bool backwardVisit(AvrItr Begin, AvrItr End, bool Recursive,
                     bool RecurseInsideLoops);

  /// \brief Visits all AVRs in the abstract layer in forward direction.
  void forwardVisitAll(AVRGenerateBase *AG);

  /// \brief Visits all AVRs in the abstract layer in backward direction.
  void backwardVisitAll(AVRGenerateBase *AG);
};

template <typename AV>
bool AVRVisitor<AV>::forwardVisit(AvrItr Begin, AvrItr End, bool Recursive,
                                  bool RecurseInsideLoops) {

  for (auto Itr = Begin, Next = Itr, AvrEnd = End; Itr != AvrEnd; Itr = Next) {

    ++Next;

    if (visit(&*Itr, Recursive, RecurseInsideLoops, true))
      return true;
  }

  return false;
}

template <typename AV>
bool AVRVisitor<AV>::backwardVisit(AvrItr Begin, AvrItr End, bool Recursive,
                                   bool RecurseInsideLoops) {

  AVRContainerTy::reverse_iterator RI(End), RE(Begin);

  /// Change direction and iterate backwards
  for (auto RNext = RI; RI != RE; RI = RNext) {

    ++RNext;

    if (visit(&(*(RI)), Recursive, RecurseInsideLoops, false))
      return true;
  }

  return false;
}

template <typename AV>
void AVRVisitor<AV>::forwardVisitAll(AVRGenerateBase *AG) {
  forwardVisit(AG->begin(), AG->end(), true, true);
}

template <typename AV>
void AVRVisitor<AV>::backwardVisitAll(AVRGenerateBase *AG) {
  backwardVisit(AG->begin(), AG->end(), true, true);
}

template <typename AV>
bool AVRVisitor<AV>::visit(AVR *Node, bool Recursive, bool RecurseInsideLoops,
                           bool Forward) {

  bool Ret = false;

  if (AVRFunction *AFunc = dyn_cast<AVRFunction>(Node)) {

    callVisit<AVRFunction>(AFunc);
    if (Recursive && !Visitor.skipRecursion(Node) && !Visitor.isDone()) {

      Ret = Forward ? forwardVisit(AFunc->child_begin(), AFunc->child_end(),
                                   RecurseInsideLoops, true)
                    : backwardVisit(AFunc->child_begin(), AFunc->child_end(),
                                    RecurseInsideLoops, true);

      if (Ret)
        return true;

      callPostVisit<AVRFunction>(AFunc);
    }
  } else if (AVRLoop *ALoop = dyn_cast<AVRLoop>(Node)) {

    callVisit<AVRLoop>(ALoop);
    if (Recursive && !Visitor.skipRecursion(Node) && !Visitor.isDone()) {

      Ret = Forward ? forwardVisit(ALoop->child_begin(), ALoop->child_end(),
                                   RecurseInsideLoops, true)
                    : backwardVisit(ALoop->child_begin(), ALoop->child_end(),
                                    RecurseInsideLoops, true);

      if (Ret)
        return true;

      callPostVisit<AVRLoop>(ALoop);
    }
  } else if (AVRWrn *AWrn = dyn_cast<AVRWrn>(Node)) {

    callVisit<AVRWrn>(AWrn);
    if (Recursive && !Visitor.skipRecursion(Node) && !Visitor.isDone()) {

      Ret = Forward ? forwardVisit(AWrn->child_begin(), AWrn->child_end(),
                                   RecurseInsideLoops, true)
                    : backwardVisit(AWrn->child_begin(), AWrn->child_end(),
                                    RecurseInsideLoops, true);

      if (Ret)
        return true;

      callPostVisit<AVRWrn>(AWrn);
    }
  } else if (AVRIf *AIf = dyn_cast<AVRIf>(Node)) {
    callVisit<AVRIf, AVRIfIR, AVRIfHIR>(AIf);

    if (Recursive && !Visitor.skipRecursion(Node) && !Visitor.isDone()) {
      Ret = Forward ? forwardVisit(AIf->then_begin(), AIf->then_end(),
                                   RecurseInsideLoops, true)
                    : backwardVisit(AIf->else_begin(), AIf->else_end(),
                                    RecurseInsideLoops, true);

      if (Ret)
        return true;

      Ret = Forward ? forwardVisit(AIf->else_begin(), AIf->else_end(),
                                   RecurseInsideLoops, true)
                    : backwardVisit(AIf->then_begin(), AIf->then_end(),
                                    RecurseInsideLoops, true);

      if (Ret) {
        return true;
      }

      callPostVisit<AVRIf, AVRIfIR, AVRIfHIR>(AIf);
    }
  } else if (AVRSwitch *ASwitch = dyn_cast<AVRSwitch>(Node)) {
    Visitor.visit(ASwitch);
    if (Recursive && !Visitor.skipRecursion(Node) && !Visitor.isDone()) {

      if (Forward) {
        for (unsigned Itr = 1, End = ASwitch->getNumCases(); Itr <= End;
             ++Itr) {
          if (forwardVisit(ASwitch->case_child_begin(Itr),
                           ASwitch->case_child_end(Itr), RecurseInsideLoops,
                           true)) {
            return true;
          }
        }
        if (forwardVisit(ASwitch->default_case_child_begin(),
                         ASwitch->default_case_child_end(), RecurseInsideLoops,
                         true)) {
          return true;
        }
      } else {
        if (backwardVisit(ASwitch->default_case_child_begin(),
                          ASwitch->default_case_child_end(), RecurseInsideLoops,
                          true)) {
          return true;
        }
        for (unsigned Itr = ASwitch->getNumCases(), End = 1; Itr >= End;
             --Itr) {
          if (backwardVisit(ASwitch->case_child_begin(Itr),
                            ASwitch->case_child_end(Itr), RecurseInsideLoops,
                            true)) {
            return true;
          }
        }
      }
    }
    callPostVisit<AVRSwitch, AVRSwitchIR, AVRSwitchHIR>(ASwitch);
  } else if (AVRAssign *AAssign = dyn_cast<AVRAssign>(Node)) {
    callVisit<AVRAssign, AVRAssignIR, AVRAssignHIR>(AAssign);

    if (!Recursive || Visitor.skipRecursion(Node) || Visitor.isDone())
      return true;

    AVR *First = Forward ? AAssign->getRHS() : AAssign->getLHS();
    AVR *Second = Forward ? AAssign->getLHS() : AAssign->getRHS();

    if (First != nullptr)
      Ret = visit(First, Recursive, RecurseInsideLoops, Forward);

    if (Ret)
      return true;

    if (Second != nullptr)
      Ret = visit(Second, Recursive, RecurseInsideLoops, Forward);

    if (Ret)
      return true;

    callPostVisit<AVRAssign, AVRAssignIR, AVRAssignHIR>(AAssign);
  } else if (AVRExpression *AExpr = dyn_cast<AVRExpression>(Node)) {
    callVisit<AVRExpression, AVRExpressionIR, AVRExpressionHIR>(AExpr);

    if (!Recursive || Visitor.skipRecursion(Node) || Visitor.isDone())
      return true;

    unsigned NumOperands = AExpr->getNumOperands();
    for (unsigned OpIt = 1; OpIt <= NumOperands; ++OpIt) {
      unsigned OpIndex = Forward ? OpIt - 1 : NumOperands - OpIt;
      AVR *Operand = AExpr->getOperand(OpIndex);
      Ret = visit(Operand, Recursive, RecurseInsideLoops, Forward);
      if (Ret)
        return true;
    }

    callPostVisit<AVRExpression, AVRExpressionIR, AVRExpressionHIR>(AExpr);
  } else if (AVRValue *AValue = dyn_cast<AVRValue>(Node)) {
    callVisit<AVRValue, AVRValueIR, AVRValueHIR>(AValue);
  } else if (AVRLabel *ALabel = dyn_cast<AVRLabel>(Node)) {
    callVisit<AVRLabel, AVRLabelIR, AVRLabelHIR>(ALabel);
  } else if (AVRPhi *APhi = dyn_cast<AVRPhi>(Node)) {
    callVisit<AVRPhi, AVRPhiIR>(APhi);
  } else if (AVRCall *ACall = dyn_cast<AVRCall>(Node)) {
    callVisit<AVRCall, AVRCallIR>(ACall);
  } else if (AVRBranch *ABranch = dyn_cast<AVRBranch>(Node)) {
    callVisit<AVRBranch, AVRBranchIR, AVRBranchHIR>(ABranch);
  } else if (AVRBackEdge *ABE = dyn_cast<AVRBackEdge>(Node)) {
    callVisit<AVRBackEdge, AVRBackEdgeIR>(ABE);
  } else if (AVREntry *AEntry = dyn_cast<AVREntry>(Node)) {
    callVisit<AVREntry, AVREntryIR>(AEntry);
  } else if (AVRReturn *AReturn = dyn_cast<AVRReturn>(Node)) {
    callVisit<AVRReturn, AVRReturnIR>(AReturn);
  } else if (AVRCompare *ACompare = dyn_cast<AVRCompare>(Node)) {
    callVisit<AVRCompare, AVRCompareIR>(ACompare);
  } else if (AVRSelect *ASelect = dyn_cast<AVRSelect>(Node)) {
    callVisit<AVRSelect, AVRSelectIR>(ASelect);
  } else if (AVRNOP *ANop = dyn_cast<AVRNOP>(Node)) {
    callVisit<AVRNOP>(ANop);
  } else if (AVRUnreachable *AUnreach = dyn_cast<AVRUnreachable>(Node)) {
    callVisit<AVRUnreachable, AVRUnreachableIR, AVRUnreachableHIR>(AUnreach);
  } else if (isa<AVR>(Node)) {
    llvm_unreachable("Malformed AVR pointer!");
  } else {
    Node->dump();
    llvm_unreachable("Unknown AVR type!");
  }

  return Ret;
}

} // End VPO Vectorizer
} // End LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_AVR_VISITOR_H
