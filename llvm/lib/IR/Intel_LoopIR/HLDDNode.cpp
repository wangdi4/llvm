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

#include "llvm/IR/Intel_LoopIR/HLNode.h"
#include "llvm/IR/Intel_LoopIR/HLDDNode.h"

using namespace llvm;
using namespace llvm::loopopt;

unsigned HLDDNode::GlobalNum(0);

HLDDNode::HLDDNode(unsigned SCID, HLNode* Par)
  : HLNode(SCID, Par), TopSortNum(0) {

  Number = GlobalNum++;
}

