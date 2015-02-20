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
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

HLLoop::HLLoop(HLIf* ZttIf, bool IsDoWh, unsigned NumEx)
  : HLDDNode(HLNode::HLLoopVal), Ztt(ZttIf), IsDoWhile(IsDoWh)
  , NumExits(NumEx), NestingLevel(0), IsInnermost(true) { 

  ChildBegin = Children.end();
  PostexitBegin = Children.end();
}

HLLoop::HLLoop(const HLLoop &HLLoopObj)
  : HLDDNode(HLLoopObj), IsDoWhile(HLLoopObj.IsDoWhile)
  , NumExits(HLLoopObj.NumExits), NestingLevel(0)
  , IsInnermost(HLLoopObj.IsInnermost) {

  ChildBegin = Children.end();
  PostexitBegin = Children.end();

  /// Clone the Ztt
  Ztt = nullptr;
  if (HLLoopObj.hasZtt()) {
    Ztt = HLLoopObj.Ztt->clone();
  }

  /// Loop over children, preheader and postexit
  for (const_pre_iterator PreIter = HLLoopObj.pre_begin(),
       PreIterEnd = HLLoopObj.pre_end(); PreIter != PreIterEnd; ++PreIter) {
    HLNode *NewHLNode = PreIter->clone();
    HLNodeUtils::insertAsLastPreheaderNode(this, NewHLNode);
  }

  for (const_child_iterator ChildIter = HLLoopObj.child_begin(),
       ChildIterEnd = HLLoopObj.child_end();
       ChildIter != ChildIterEnd; ++ChildIter) {
    HLNode *NewHLNode = ChildIter->clone();
    HLNodeUtils::insertAsLastChild(this, NewHLNode);
  }

  for (const_post_iterator PostIter = HLLoopObj.post_begin(),
       PostIterEnd = HLLoopObj.post_end();
       PostIter != PostIterEnd; ++PostIter) {
    HLNode *NewHLNode = PostIter->clone();
    HLNodeUtils::insertAsLastPostexitNode(this, NewHLNode);
  }
}

HLLoop* HLLoop::clone() const {

  /// Call the Copy Constructor
  HLLoop *NewHLLoop = new HLLoop(*this);

  return NewHLLoop;
}

