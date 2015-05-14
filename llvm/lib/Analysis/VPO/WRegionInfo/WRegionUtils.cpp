//===- WRegionNodeUtils.cpp - Implements W-Region Node Utils class --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the the WRegionNode Utils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "WRegionUtils"

using namespace llvm;
using namespace vpo;

// W-Region Graph Creatation Utilities
WRegion *WRegionUtils::createWRegion(
  BasicBlock *EntryBB, 
  BasicBlock *ExitBB, WRegionBSetTy &BBs
) 
{
  return new WRegion(EntryBB, ExitBB, BBs);
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
  // TODO: Add VpoParLoop, VpoParSections support.

  if (isa<WRegion>(Parent)) {
    auto wrn = dyn_cast<WRegion>(Parent);
#if 1
    WRContainerTy &WRContainer = wrn->Children;
    WrnIter pos;

    switch (Op) {
      case FirstChild:
        pos = wrn->wrn_child_begin();
        break;
      case LastChild:
        pos = wrn->wrn_child_end();
        break;
      case Append:
        pos = std::next(Pos);
        break;
      case Prepend:
        pos = Pos;
        break;
      default:
        llvm_unreachable("VPO: Unknown WRegionNode Insertion Operation Type");
    }
    W->setParent(Parent);
    WRContainer.insert(pos, W);
#endif
  }
  else {
    // TODO: Missing Support
    DEBUG(dbgs() << "Missing Full Insertion Support\n");
  }
  return;
}

