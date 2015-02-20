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
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

HLIf::HLIf()
  : HLDDNode(HLNode::HLIfVal) { 
  ElseBegin = Children.end();
}

HLIf::HLIf(const HLIf &HLIfObj)
  : HLDDNode(HLIfObj), Preds(HLIfObj.Preds) 
  , Conjunctions(HLIfObj.Conjunctions) {

  ElseBegin = Children.end();

  /// Loop over Then children and Else children
  for (const_then_iterator ThenIter = HLIfObj.then_begin(),
       ThenIterEnd = HLIfObj.then_end(); ThenIter != ThenIterEnd; ++ThenIter) {
    HLNode *NewHLNode = ThenIter->clone();
    HLNodeUtils::insertAsLastIfChild(this, NewHLNode);
  }

  for (const_then_iterator ElseIter = HLIfObj.else_begin(),
       ElseIterEnd = HLIfObj.else_end();
       ElseIter != ElseIterEnd; ++ElseIter) {
    HLNode *NewHLNode = ElseIter->clone();
    HLNodeUtils::insertAsLastIfChild(this, NewHLNode, false);
  }
}

HLIf* HLIf::clone() const {

  /// Call the Copy Constructor
  HLIf *NewHLIf = new HLIf(*this);

  return NewHLIf;
}

