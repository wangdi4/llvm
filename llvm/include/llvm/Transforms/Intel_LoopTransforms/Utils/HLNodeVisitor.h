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

namespace llvm {

namespace loopopt {

/// \brief This class is used to visit HIR nodes.
/// 
/// The forward/backward traversal works even if the current iterator is 
/// invalidated (removed/replaced) as the next/prev iterator is saved so it 
/// should work for most transformations. Specialized traversals might be needed
/// otherwise.
///
/// Visitor (template class HV) needs to implement:
///
/// 1) Various visit[Element]() functions like visitRegion(), visitLoop etc.
/// 2) bool isDone() for early termination of the traversal.
///  
template<typename HV>
class HLNodeVisitor {
private:
  HV* Visitor;

  friend class HLNodeUtils;

  HLNodeVisitor(HV* V) : Visitor(V) { }

  /// \brief Contains the core logic to visit nodes and recurse further.
  /// Returns true to indicate that early termination has occured.
  bool visit(HLContainerTy::iterator It, bool Recursive, bool Forward);

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

} // End namespace loopopt

} // End namespace llvm

#endif
