//===----------------------------------------------------------------------===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOVectorCandidate.cpp -- Implements the vector loop candidate object
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOVectorCandidate.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/Passes.h"

#define DEBUG_TYPE "vector-candidate"

using namespace llvm;
using namespace llvm::vpo;

VectorCandidate::VectorCandidate(WRNVecLoopNode *WNode) : WRNode(WNode),
  HIRNode(nullptr), IsHIRCandidate(false)
{
  setEntryBBlock(WNode->getEntryBBlock());
  setExitBBlock(WNode->getExitBBlock());
}

VectorCandidate::VectorCandidate(HLNode *HirNode) : WRNode(nullptr),
  HIRNode(HirNode), IsHIRCandidate(true)
{
  // TODO: HIR Vector Candidate creation implementation.
}

BasicBlock *VectorCandidate::getPredBBlock() const 
{
  /*
  auto PredI = pred_begin(EntryBBlock);

  // In some cases the entry bblock is also the loop header, so the predecessor
  // can be the loop latch. We need to skip it, if that is the case.
  if (BBlocks.count(*PredI)) {
    PredI++;
    auto TempPredI = PredI;

    assert(!BBlocks.count(*PredI) &&
           "Both region predecessors lie inside the reigon!");
    assert((++TempPredI == pred_end(EntryBBlock)) &&
           "Region has more than two predecessors!");
  }

  return *PredI;
  */
  return nullptr;
}


BasicBlock *VectorCandidate::getSuccBBlock() const 
{
  /*
  auto SuccI = succ_begin(ExitBBlock);

  /// In some cases the exit bblock is also the loop latch, so the successor
  /// can be the loop header. We need to skip it, if that is the case.
  if (BBlocks.count(*SuccI)) {
    SuccI++;
    auto TempSuccI = SuccI;

    assert(!BBlocks.count(*SuccI) &&
           "Both region successors lie inside the reigon!");
    assert((++TempSuccI == succ_end(ExitBBlock)) &&
           "Region has more than two successors!");
  }

  return *SuccI;
  */
  return nullptr;
}

