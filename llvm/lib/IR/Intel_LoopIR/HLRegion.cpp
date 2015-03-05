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
#include "llvm/IR/CFG.h"
#include "llvm/IR/Intel_LoopIR/HLRegion.h"

using namespace llvm;
using namespace llvm::loopopt;

HLRegion::HLRegion(std::set<BasicBlock *> &OrigBBs, BasicBlock *EntryBB,
                   BasicBlock *ExitBB)
    : HLNode(HLNode::HLRegionVal), OrigBBlocks(OrigBBs), EntryBBlock(EntryBB),
      ExitBBlock(ExitBB) {}

HLRegion *HLRegion::clone() const {

  llvm_unreachable("Do not support HLRegion cloning.");

  return nullptr;
}

BasicBlock *HLRegion::getPredBBlock() const {
  auto PredI = pred_begin(EntryBBlock);

  /// In some cases the entry bblock is also the loop header, so the predecessor
  /// can be the loop latch. We need to skip it, if that is the case.
  if (OrigBBlocks.find(*PredI) != OrigBBlocks.end()) {
    PredI++;
  }

  return *PredI;
}

BasicBlock *HLRegion::getSuccBBlock() const {
  return *(succ_begin(ExitBBlock));
}
