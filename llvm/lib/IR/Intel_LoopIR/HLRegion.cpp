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

HLRegion::HLRegion(RegionIdentification::RegionBBlocksTy &OrigBBs,
                   BasicBlock *EntryBB, BasicBlock *ExitBB)
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
    auto TempPredI = PredI;

    assert((OrigBBlocks.find(*PredI) == OrigBBlocks.end()) &&
           "Both region predecessors lie inside the reigon!");
    assert((++TempPredI == pred_end(EntryBBlock)) &&
           "Region has more than two predecessors!");
  }

  return *PredI;
}

BasicBlock *HLRegion::getSuccBBlock() const {
  auto SuccI = succ_begin(ExitBBlock);

  /// In some cases the exit bblock is also the loop latch, so the successor
  /// can be the loop header. We need to skip it, if that is the case.
  if (OrigBBlocks.find(*SuccI) != OrigBBlocks.end()) {
    SuccI++;
    auto TempSuccI = SuccI;

    assert((OrigBBlocks.find(*SuccI) == OrigBBlocks.end()) &&
           "Both region successors lie inside the reigon!");
    assert((++TempSuccI == succ_end(ExitBBlock)) &&
           "Region has more than two successors!");
  }

  return *SuccI;
}

HLNode *HLRegion::getFirstChild() {
  if (hasChildren()) {
    return child_begin();
  }

  return nullptr;
}

HLNode *HLRegion::getLastChild() {
  if (hasChildren()) {
    return std::prev(child_end());
  }

  return nullptr;
}
