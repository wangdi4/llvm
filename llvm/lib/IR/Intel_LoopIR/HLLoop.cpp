//===-------- HLLoop.cpp - Implements the HLLoop class --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLLoop class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"

#include "llvm/IR/Intel_LoopIR/HLLoop.h"

using namespace llvm;
using namespace llvm::loopopt;

HLLoop::HLLoop(HLNode* Par, HLIf* ZttIf, bool isDoWh, unsigned NumEx)
  : HLDDNode(HLNode::HLLoopVal, Par), Ztt(ZttIf), isDoWhile(isDoWh)
  , NumExits(NumEx) { }

HLLoop::HLLoop(const HLLoop &HLLoopObj)
  : HLDDNode(HLLoopObj), isDoWhile(HLLoopObj.isDoWhile),
    NumExits(HLLoopObj.NumExits) {

  /// Clone the Ztt
  Ztt = nullptr;
  if (HLLoopObj.hasZtt())
    Ztt = HLLoopObj.Ztt->clone();

  /// Loop over children, preheader and postexit
  for (const_pre_iterator PreIter = HLLoopObj.pre_begin(),
       PreIterEnd = HLLoopObj.pre_end(); PreIter != PreIterEnd; ++PreIter) {
    HLNode *NewHLNode = PreIter->clone();
    Preheader.push_back(NewHLNode);
  }

  for (const_post_iterator PostIter = HLLoopObj.post_begin(),
       PostIterEnd = HLLoopObj.post_end();
       PostIter != PostIterEnd; ++PostIter) {
    HLNode *NewHLNode = PostIter->clone();
    Postexit.push_back(NewHLNode);
  }

  for (const_child_iterator ChildIter = HLLoopObj.child_begin(),
       ChildIterEnd = HLLoopObj.child_end();
       ChildIter != ChildIterEnd; ++ChildIter) {
    HLNode *NewHLNode = ChildIter->clone();
    Children.push_back(NewHLNode);
  }
}

HLLoop* HLLoop::clone() const {

  /// Check for 'this' as null
  assert(this && " HLLoop cannot be null");

  /// Call the Copy Constructor
  HLLoop *NewHLLoop = new HLLoop(*this);

  return NewHLLoop;
}

