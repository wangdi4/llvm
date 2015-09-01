//===----- WRegionNodeUtils.cpp - W-Region Node Utils class -----*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
//   This file implements the the WRegionNode Utils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"

#define DEBUG_TYPE "WRegionUtils"

using namespace llvm;
using namespace vpo;

/// \brief Create a specialized WRN based on the DirString.
/// If the string corrensponds to a BEGIN directive, then create
/// a WRN node of WRegionNodeKind corresponding to the directive,
/// and return a pointer to it. Otherwise; return nullptr.
WRegionNode *WRegionUtils::createWRegion(
  StringRef DirString,
  BasicBlock *EntryBB
)
{
  WRegionNode *W = nullptr;

  if (DirString == "dir.parallel") {
    W = new WRNParallelNode();
  }
  else if (DirString == "dir.simd") {
    W = new WRNVecLoopNode();
  }
  // TODO: complete the list for all WRegionNodeKinds

  if (W)
    W->setEntryBBlock(EntryBB);

  return W;
}


bool WRegionUtils::isEndDirective(
  StringRef DirString
)
{
  if ((DirString == "dir.end.parallel") ||
      (DirString == "dir.simd.end") ||     // This should be removed after fix in SIMDCloning 
      (DirString == "dir.end.simd")) {
    // TODO: complete the list for all WRegionNodeKinds
    return true;
  }
  return false;
}

void WRegionUtils::handleDirQual(
  IntrinsicInst *Intrin, 
  WRegionNode *W
) 
{
  // TODO: implement
  return;
}

void WRegionUtils::handleDirQualOpnd(
  IntrinsicInst *Intrin, 
  WRegionNode *W
) 
{
  // TODO: implement
  return;
}

void WRegionUtils::handleDirQualOpndList(
  IntrinsicInst *Intrin, 
  WRegionNode *W
) 
{
  // TODO: implement
  return;
}

// Insertion Utilities
void WRegionUtils::insertFirstChild(
  WRegionNode *Parent, 
  WrnIter wrn
) 
{
  insertWRegionNode(Parent, nullptr, wrn, FirstChild);
  return;
}

void WRegionUtils::insertLastChild(
  WRegionNode *Parent, 
  WrnIter wrn
) 
{
  insertWRegionNode(Parent, nullptr, wrn, LastChild);
  return;
}

void WRegionUtils::insertAfter(
  WrnIter  pos, 
  WRegionNode *wrn
) 
{
  assert(pos && "Insert Position is Null");
  insertWRegionNode(pos->getParent(), pos, wrn, Append);
}

void WRegionUtils::insertBefore(
  WrnIter  pos, 
  WRegionNode *wrn
) 
{
  assert(pos && "Insert Position is Null");
  insertWRegionNode(pos->getParent(), pos, wrn, Prepend);
}

void WRegionUtils::insertWRegionNode(
  WRegionNode  *Parent, 
  WrnIter  Pos, 
  WrnIter  W, 
  OpType   Op
) 
{
  assert(Parent && "Parent is Null");

  WRContainerTy &WRContainer = Parent->getChildren();

  WrnIter InsertionPoint;

  switch (Op) {
      case FirstChild:
        InsertionPoint = Parent->getFirstChild();
        break;
      case LastChild:
        InsertionPoint = Parent->getLastChild();
        break;
      case Append:
        InsertionPoint = std::next(Pos);
        break;
      case Prepend:
        InsertionPoint = Pos;
        break;
      default:
        llvm_unreachable("VPO: Unknown WRegionNode Insertion Operation Type");
  }

  W->setParent(Parent);
  WRContainer.insert(InsertionPoint, W);

  return;
}

void WRegionUtils::setLoopInfo(
  WRNVecLoopNode *WRNLoop,
  LoopInfo* LI
)
{
  WRNLoop->setLoopInfo(LI);
}
