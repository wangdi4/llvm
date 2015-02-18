//===------------ HLIf.cpp - Implements the HLIf class --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLIf class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_LoopIR/HLIf.h"

using namespace llvm;
using namespace llvm::loopopt;

HLIf::HLIf(HLNode* Par)
  : HLDDNode(HLNode::HLIfVal, Par) { }

HLIf::HLIf(const HLIf &HLIfObj)
  : HLDDNode(HLIfObj), Preds(HLIfObj.Preds),
    Conjunctions(HLIfObj.Conjunctions) {

  /// Loop over Then children and Else children
  for (const_then_iterator ThenIter = HLIfObj.then_begin(),
       ThenIterEnd = HLIfObj.then_end(); ThenIter != ThenIterEnd; ++ThenIter) {
    HLNode *NewHLNode = ThenIter->clone();
    /// TODO: Check if setParent is handled in push_back
    /// NewHLNode->setParent(this);
    ThenChildren.push_back(NewHLNode);
  }

  for (const_then_iterator ElseIter = HLIfObj.else_begin(),
       ElseIterEnd = HLIfObj.else_end();
       ElseIter != ElseIterEnd; ++ElseIter) {
    HLNode *NewHLNode = ElseIter->clone();
    /// TODO: Check if setParent is handled in push_back
    /// NewHLNode->setParent(this);
    ElseChildren.push_back(NewHLNode);
  }
}

HLIf* HLIf::clone() const {

  /// Call the Copy Constructor
  HLIf *NewHLIf = new HLIf(*this);

  return NewHLIf;
}

