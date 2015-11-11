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
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"

#define DEBUG_TYPE "WRegionUtils"

using namespace llvm;
using namespace vpo;

/// \brief Create a specialized WRN based on the DirString.
/// If the string corrensponds to a BEGIN directive, then create
/// a WRN node of WRegionNodeKind corresponding to the directive,
/// and return a pointer to it. Otherwise; return nullptr.
WRegionNode *WRegionUtils::createWRegion(
  StringRef DirString,
  BasicBlock *EntryBB,
  LoopInfo *LI
)
{
  WRegionNode *W = nullptr;
  int DirID = VPOUtils::getDirectiveID(DirString);

  switch(DirID) {
    // TODO: complete the list for all WRegionNodeKinds
    case DIR_OMP_PARALLEL:
      W = new WRNParallelNode(EntryBB);
      break;
    case DIR_OMP_SIMD:
      W = new WRNVecLoopNode(EntryBB, LI);
      break;
  }
  return W;
}

bool WRegionUtils::isBeginDirective(
  StringRef DirString
)
{
  int DirID = VPOUtils::getDirectiveID(DirString);
  return WRegionUtils::isBeginDirective(DirID);
}

bool WRegionUtils::isBeginDirective(
  int DirID
)
{
  switch(DirID) {
    case DIR_OMP_PARALLEL:
    case DIR_OMP_PARALLEL_LOOP:
    case DIR_OMP_LOOP_SIMD:
    case DIR_OMP_PARALLEL_LOOP_SIMD:
    case DIR_OMP_SECTIONS:
    case DIR_OMP_PARALLEL_SECTIONS:
    case DIR_OMP_WORKSHARE:
    case DIR_OMP_PARALLEL_WORKSHARE:
    case DIR_OMP_SINGLE:
    case DIR_OMP_TASK:
    case DIR_OMP_MASTER:
    case DIR_OMP_CRITICAL:
    case DIR_OMP_ATOMIC:
    case DIR_OMP_ORDERED:
    case DIR_OMP_SIMD:
    case DIR_OMP_TASKLOOP:
    case DIR_OMP_TASKLOOP_SIMD:
    case DIR_OMP_TARGET:
    case DIR_OMP_TARGET_DATA:
    case DIR_OMP_TARGET_UPDATE:
    case DIR_OMP_TEAMS:
    case DIR_OMP_TEAMS_DISTRIBUTE:
    case DIR_OMP_TEAMS_SIMD:
    case DIR_OMP_TEAMS_DISTRIBUTE_SIMD:
    case DIR_OMP_DISTRIBUTE:
    case DIR_OMP_DISTRIBUTE_PARLOOP:
    case DIR_OMP_DISTRIBUTE_SIMD:
    case DIR_OMP_DISTRIBUTE_PARLOOP_SIMD:
    case DIR_OMP_TARGET_TEAMS:
    case DIR_OMP_TEAMS_DISTRIBUTE_PARLOOP:
    case DIR_OMP_TEAMS_DISTRIBUTE_PARLOOP_SIMD:
    case DIR_OMP_TARGET_TEAMS_DISTRIBUTE:
    case DIR_OMP_TARGET_TEAMS_DISTRIBUTE_PARLOOP:
    case DIR_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD:
    case DIR_OMP_TARGET_TEAMS_DISTRIBUTE_PARLOOP_SIMD:
      return true;
  }
  return false;
}


bool WRegionUtils::isEndDirective(
  StringRef DirString
)
{
  int DirID = VPOUtils::getDirectiveID(DirString);
  return WRegionUtils::isEndDirective(DirID);
}

bool WRegionUtils::isEndDirective(
  int DirID
)
{
  switch(DirID) {
    case DIR_OMP_END_PARALLEL:
    case DIR_OMP_END_PARALLEL_LOOP:
    case DIR_OMP_END_LOOP_SIMD:
    case DIR_OMP_END_PARALLEL_LOOP_SIMD:
    case DIR_OMP_END_SECTIONS:
    case DIR_OMP_END_PARALLEL_SECTIONS:
    case DIR_OMP_END_WORKSHARE:
    case DIR_OMP_END_PARALLEL_WORKSHARE:
    case DIR_OMP_END_SINGLE:
    case DIR_OMP_END_TASK:
    case DIR_OMP_END_MASTER:
    case DIR_OMP_END_CRITICAL:
    case DIR_OMP_END_ATOMIC:
    case DIR_OMP_END_ORDERED:
    case DIR_OMP_END_SIMD:
    case DIR_OMP_END_TASKLOOP:
    case DIR_OMP_END_TASKLOOP_SIMD:
    case DIR_OMP_END_TARGET:
    case DIR_OMP_END_TARGET_DATA:
    case DIR_OMP_END_TARGET_UPDATE:
    case DIR_OMP_END_TEAMS:
    case DIR_OMP_END_TEAMS_DISTRIBUTE:
    case DIR_OMP_END_TEAMS_SIMD:
    case DIR_OMP_END_TEAMS_DISTRIBUTE_SIMD:
    case DIR_OMP_END_DISTRIBUTE:
    case DIR_OMP_END_DISTRIBUTE_PARLOOP:
    case DIR_OMP_END_DISTRIBUTE_SIMD:
    case DIR_OMP_END_DISTRIBUTE_PARLOOP_SIMD:
    case DIR_OMP_END_TARGET_TEAMS:
    case DIR_OMP_END_TEAMS_DISTRIBUTE_PARLOOP:
    case DIR_OMP_END_TEAMS_DISTRIBUTE_PARLOOP_SIMD:
    case DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE:
    case DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARLOOP:
    case DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_SIMD:
    case DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARLOOP_SIMD:
      return true;
  }
  return false;
}

bool WRegionUtils::isBeginOrEndDirective(
  StringRef DirString
)
{
  int DirID = VPOUtils::getDirectiveID(DirString);
  return WRegionUtils::isBeginOrEndDirective(DirID);
}

bool WRegionUtils::isBeginOrEndDirective(
  int DirID
)
{
  return WRegionUtils::isBeginDirective(DirID) ||
         WRegionUtils::isEndDirective(DirID);
}

bool WRegionUtils::isSoloDirective(
  StringRef DirString
)
{
  int DirID = VPOUtils::getDirectiveID(DirString);
  return WRegionUtils::isSoloDirective(DirID);
}

bool WRegionUtils::isSoloDirective(
  int DirID
)
{
  switch(DirID) {
    case DIR_OMP_SECTION:
    case DIR_OMP_BARRIER:
    case DIR_OMP_TASKWAIT:
    case DIR_OMP_TASKYIELD:
    case DIR_OMP_FLUSH:
    case DIR_OMP_TARGET_ENTER_DATA:
    case DIR_OMP_TARGET_EXIT_DATA:
    case DIR_OMP_CANCEL:
    case DIR_OMP_CANCELLATION_POINT:
      return true;
  }
  return false;
}

bool WRegionUtils::isListEndDirective(
  StringRef DirString
)
{
  int DirID = VPOUtils::getDirectiveID(DirString);
  return WRegionUtils::isListEndDirective(DirID);
}

bool WRegionUtils::isListEndDirective(
  int DirID
)
{
  return DirID == DIR_QUAL_LIST_END;
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
