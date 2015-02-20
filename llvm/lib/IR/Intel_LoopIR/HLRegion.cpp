//===--- HLRegion.cpp - Implements the HLRegion class ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLRegion class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/ErrorHandling.h"

#include "llvm/IR/Intel_LoopIR/HLRegion.h"

using namespace llvm;
using namespace llvm::loopopt;

HLRegion::HLRegion(std::set< BasicBlock* >& OrigBBs,
  BasicBlock* PredBB, BasicBlock* SuccBB)
  : HLNode(HLNode::HLRegionVal), OrigBBlocks(OrigBBs)
  , PredBBlock(PredBB), SuccBBlock(SuccBB) { }


HLRegion* HLRegion::clone() const {

  llvm_unreachable("Do not support HLRegion cloning.");

  return nullptr;
}


