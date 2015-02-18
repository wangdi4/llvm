//===--- HLDDNode.cpp - Implements the HLDDNode class ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLDDNode class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"

#include "llvm/IR/Intel_LoopIR/DDRef.h"
#include "llvm/IR/Intel_LoopIR/HLNode.h"
#include "llvm/IR/Intel_LoopIR/HLDDNode.h"

using namespace llvm;
using namespace llvm::loopopt;

unsigned HLDDNode::GlobalNum(0);

HLDDNode::HLDDNode(unsigned SCID, HLNode* Par)
  : HLNode(SCID, Par), TopSortNum(0) {

  setNextNumber();
}

HLDDNode::HLDDNode(const HLDDNode &HLDDNodeObj)
  : HLNode(HLDDNodeObj), TopSortNum(0) {

  /// Loop over DDRefs
  for (const_ddref_iterator Iter = HLDDNodeObj.ddref_begin(),
       IterEnd = HLDDNodeObj.ddref_end(); Iter != IterEnd; ++Iter) {
    DDRef *NewDDRef = (*Iter)->clone();
    /// TODO: Check if HLNode is appropriately set in push_back call
    /// NewDDRef->setHLNode(this);
    DDRefs.push_back(NewDDRef);
  }

  setNextNumber();
}

void HLDDNode::setNextNumber() {
  Number = GlobalNum++;
}
