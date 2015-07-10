//===----- WRegion.cpp - Implements the WRegion class ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the WRegion class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/CFG.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"

using namespace llvm;
using namespace llvm::vpo;

WRegion::WRegion(BasicBlock *EntryBB, BasicBlock *ExitBB, const WRegionBSetTy &BBs,
    const LoopInfo *LoopI) : WRegionNode(WRegionNode::VPO_PAR_REGION),
    EntryBBlock(EntryBB), ExitBBlock(ExitBB), BBlockSet(BBs), LI(LoopI)
{}

WRegion *WRegion::clone() const {
  llvm_unreachable("Do not support W-Region cloning.");
  return nullptr;
}

void WRegion::print(formatted_raw_ostream &OS, unsigned Depth) const {
  indent(OS, Depth);

  OS << "BEGIN W-REGION\n";

  for (auto I = wrn_child_begin(), E = wrn_child_end(); I != E; I++) {
    I->print(OS, Depth + 1);
  }

  indent(OS, Depth);

  OS << "END REGION\n";
}

BasicBlock *WRegion::getPredBBlock() const {
  auto PredI = pred_begin(EntryBBlock);

  /// In some cases the entry bblock is also the loop header, so the predecessor
  /// can be the loop latch. We need to skip it, if that is the case.
  if (BBlockSet.find(*PredI) != BBlockSet.end()) {
    PredI++;
    auto TempPredI = PredI;

    assert((BBlockSet.find(*PredI) == BBlockSet.end()) &&
           "Both region predecessors lie inside the W-Region!");
    assert((++TempPredI == pred_end(EntryBBlock)) &&
           "Region has more than two predecessors!");
  }

  return *PredI;
}

BasicBlock *WRegion::getSuccBBlock() const {
  auto SuccI = succ_begin(ExitBBlock);

  /// In some cases the exit bblock is also the loop latch, so the successor
  /// can be the loop header. We need to skip it, if that is the case.
  if (BBlockSet.find(*SuccI) != BBlockSet.end()) {
    SuccI++;
    auto TempSuccI = SuccI;

    assert((BBlockSet.find(*SuccI) == BBlockSet.end()) &&
           "Both region successors lie inside the reigon!");
    assert((++TempSuccI == succ_end(ExitBBlock)) &&
           "W-Region has more than two successors!");
  }

  return *SuccI;
}

WRegionNode *WRegion::getFirstChild() {
  if (hasChildren()) {
    return wrn_child_begin();
  }

  return nullptr;
}

WRegionNode *WRegion::getLastChild() {
  if (hasChildren()) {
    return std::prev(wrn_child_end());
  }

  return nullptr;
}
