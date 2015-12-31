//===------------------------------------------------------------*- C++ -*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//   Source file:
//   ------------
//   VPOAvrVistor.h -- Defines the vistor template class for visiting AVR nodes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_VISTOR_H
#define LLVM_ANALYSIS_VPO_AVR_VISTOR_H

#include "llvm/Support/Compiler.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvr.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrFunction.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrLoop.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrStmt.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrStmtIR.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrIf.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrGenerate.h"

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

 public:

  AVRVisitor(AV &V) : Visitor(V) {}

  /// \brief Contains the core logic to visit nodes and recurse further.
  /// Returns true to indicate that early termination has occurred.
  /// Recursive parameter denotes if we want to visit inside the AvrNodes
  /// such as AVRIf and AVRLoops. RecursiveInsideLoops parameter denotes
  /// whether we want to visit inside the loops or not and this parameter
  /// is only useful if Recursive parameter is true.
  bool visit(AVR *Node, bool Recursive, bool RecurseInsideLoops,
             bool Forward);

  /// \brief Visits AVRs in the forward direction in the range [begin, end).
  /// Returns true to indicate that early termination has occurred.
  bool forwardVisit(AvrItr Begin, AvrItr End, bool Recursive, bool RecurseInsideLoops);

  /// \brief Visits AVRs in the backward direction in the range [begin, end).
  /// Returns true to indicate that early termination has occurred.
  /// RecursiveInsideLoops parameter denotes whether we want to visit inside the
  /// loops or not and this parameter
  /// is only useful if Recursive parameter is true.
  bool backwardVisit(AvrItr Begin, AvrItr End, bool Recursive, bool RecurseInsideLoops);

  /// \brief Visits all AVRs in the abstract layer in forward direction.
  void forwardVisitAll(AVRGenerate *AG);

  /// \brief Visits all AVRs in the abstract layer in backward direction.
  void backwardVisitAll(AVRGenerate *AG);
};


template <typename AV>
bool AVRVisitor<AV>::forwardVisit(AvrItr Begin, AvrItr End, bool Recursive,
                                  bool RecurseInsideLoops) {

  for (auto I = Begin, Next = I, E = End; I != E; I = Next) {

    ++Next;

    if (visit(I, Recursive, RecurseInsideLoops, true)) {
      return true;
    }
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

    if (visit(&(*(RI)), Recursive, RecurseInsideLoops, false)) {
      return true;
    }
  }

  return false;
}

template <typename AV>
void AVRVisitor<AV>::forwardVisitAll(AVRGenerate *AG) {
  forwardVisit(AG->begin(), AG->end(), true, true);
}

template <typename AV>
void AVRVisitor<AV>::backwardVisitAll(AVRGenerate *AG) {
  backwardVisit(AG->begin(), AG->end(), true, true);
}

template <typename AV>
bool AVRVisitor<AV>::visit(AVR *Node, bool Recursive, bool RecurseInsideLoops,
                           bool Forward) {

  bool Ret = false; 

  if (AVRFunction *AFunc = dyn_cast<AVRFunction>(Node)) {

    Visitor.visit(AFunc);
    if (Recursive && !Visitor.skipRecursion(Node) && !Visitor.isDone()) {

      Ret = Forward ? forwardVisit(AFunc->child_begin(), AFunc->child_end(),
				   RecurseInsideLoops, true)
        : backwardVisit(AFunc->child_begin(), AFunc->child_end(),
                        RecurseInsideLoops, true);

      if (Ret) {
        return true;
      }

      Visitor.postVisit(AFunc);
    }
  }
  else if (AVRLoop *ALoop = dyn_cast<AVRLoop>(Node)) {

    Visitor.visit(ALoop);
    if (Recursive && !Visitor.skipRecursion(Node) && !Visitor.isDone()) {

      Ret = Forward ? forwardVisit(ALoop->child_begin(), ALoop->child_end(),
				   RecurseInsideLoops, true)
        : backwardVisit(ALoop->child_begin(), ALoop->child_end(),
                        RecurseInsideLoops, true);

      if (Ret) {
        return true;
      }

      Visitor.postVisit(ALoop);
    }
  }
  else if (AVRWrn *AWrn = dyn_cast<AVRWrn>(Node)){

    Visitor.visit(AWrn);
    if (Recursive && !Visitor.skipRecursion(Node) && !Visitor.isDone()) {

      Ret = Forward ? forwardVisit(AWrn->child_begin(), AWrn->child_end(),
				   RecurseInsideLoops, true)
        : backwardVisit(AWrn->child_begin(), AWrn->child_end(),
                        RecurseInsideLoops, true);

      if (Ret) {
        return true;
      }

      Visitor.postVisit(AWrn);
    }
  }
  else if (AVRIf *AIf = dyn_cast<AVRIf>(Node)) {
    Visitor.visit(AIf);

    if (Recursive && !Visitor.skipRecursion(Node) && !Visitor.isDone()) {
      Ret = Forward ? forwardVisit(AIf->then_begin(), AIf->then_end(),
                                   RecurseInsideLoops, true)
	: backwardVisit(AIf->else_begin(), AIf->else_end(),
			RecurseInsideLoops, true);

      if (Ret) {
        return true;
      }

      Ret = Forward ? forwardVisit(AIf->else_begin(), AIf->else_end(),
                                   RecurseInsideLoops, true)
	: backwardVisit(AIf->then_begin(), AIf->then_end(),
			RecurseInsideLoops, true);

      if (Ret) {
        return true;
      }

      Visitor.postVisit(AIf);
    }
  }
  else if (AVRAssign *AAssign = dyn_cast<AVRAssign>(Node)) {
    Visitor.visit(AAssign);
  }
  else if (AVRLabel *ALabel = dyn_cast<AVRLabel>(Node)) {
    Visitor.visit(ALabel);
  }
  else if (AVRPhi *APhi = dyn_cast<AVRPhi>(Node)) {
    Visitor.visit(APhi);
  }
  else if (AVRCall *ACall = dyn_cast<AVRCall>(Node)) {
    Visitor.visit(ACall);
  }
  else if (AVRBranch *ABranch = dyn_cast<AVRBranch>(Node)) {
    Visitor.visit(ABranch);
  }
  else if (AVRBackEdge *ABE = dyn_cast<AVRBackEdge>(Node)) {
    Visitor.visit(ABE);
  }
  else if (AVREntry *AEntry = dyn_cast<AVREntry>(Node)) {
    Visitor.visit(AEntry);
  }
  else if (AVRReturn *AReturn = dyn_cast<AVRReturn>(Node)) {
    Visitor.visit(AReturn);
  }
  else if (AVRCompare *ACompare = dyn_cast<AVRCompare>(Node)){
    Visitor.visit(ACompare);
  }
  else if (AVRSelect *ASelect = dyn_cast<AVRSelect>(Node)){
    Visitor.visit(ASelect);
  }
  else {
    llvm_unreachable("Unknown AVR type!");
  }

  return Ret;
}

} // End VPO Vectorizer
} // End LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_AVR_VISTOR_H
