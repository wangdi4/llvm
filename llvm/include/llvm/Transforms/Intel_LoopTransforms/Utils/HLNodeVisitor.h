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
/// The traversal works even if the current iterator is invalidated 
/// (removed/replaced) as the next iterator is saved so it should work for most
/// transformations. Specialized traversals might be needed otherwise.
//
/// Visitor (template class HV) needs to implement:
///
/// 1) Various visit[Element]() functions like visitRegion(), visitLoop etc.
/// 2) [optional] bool isDone() for early termination of the traversal.
///  
template<typename HV>
class HLNodeVisitor {
private:
  HV* Visitor;

  friend class HLNodeUtils;

  HLNodeVisitor(HV* V) : Visitor(V) { }

  /// \brief Visits HLNodes in the forward direction in the range [begin, end).
  void forwardVisit(HLContainerTy::iterator Begin, HLContainerTy::iterator End,
    bool Recursive = true);
  /// \brief Visits HLNodes in the backward direction in the range [begin, end).
  void backwardVisit(HLContainerTy::iterator Begin, HLContainerTy::iterator End,
     bool Recursive = true);

  /// \brief Visits all HLNodes in the HIR in forward direction.
  void forwardVisitAll();
  /// \brief Visits all HLNodes in the HIR in backward direction.
  void backwardVisitAll();

};

} // End namespace loopopt

} // End namespace llvm

#endif
