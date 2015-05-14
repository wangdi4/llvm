//===-- WRegionNode.cpp - Implements the WRegionNode class ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the WRegionNode class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"

using namespace llvm;
using namespace llvm::vpo;

WRContainerTy llvm::vpo::WRegions;

std::set<WRegionNode *> WRegionNode::Objs;
unsigned WRegionNode::UniqueNum(0);

WRegionNode::WRegionNode(unsigned SCID)
    : SubClassID(SCID), Parent(nullptr) {
  Objs.insert(this);
  setNextNumber();
}

WRegionNode::WRegionNode(const WRegionNode &WRegionNodeObj)
    : SubClassID(WRegionNodeObj.SubClassID), Parent(nullptr) {
  Objs.insert(this);
  setNextNumber();
}

void WRegionNode::destroy() {
  Objs.erase(this);
  delete this;
}

void WRegionNode::destroyAll() {

  for (auto &I : Objs) {
    delete I;
  }

  Objs.clear();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void WRegionNode::dump() const {
  formatted_raw_ostream OS(dbgs());
  print(OS, 0);
}
#endif

void WRegionNode::indent(formatted_raw_ostream &OS, unsigned Depth) const {

  static std::string WRegionIndentString(100, ' ');
  static std::string SpaceString(IndentWidth, ' ');

  WRegionIndentString.clear();

  /// Placeholder until we can get source location.
  if (!isa<WRegion>(this)) {
    OS << "<" << Number << ">";
  }
  OS.PadToColumn(10);

  auto Parent = getParent();

  while ((Depth > 0) && Parent) {
    WRegionIndentString = SpaceString + WRegionIndentString;
#if 0
    if (isa<WRegion>(Parent)) {
      WRegionIndentString = "|" + WRegionIndentString;
    }
#endif
    Depth--;
    Parent = Parent->getParent();
  }

  OS.indent(IndentWidth * Depth);
  OS << WRegionIndentString;
}

void WRegionNode::setNextNumber() { Number = UniqueNum++; }

WRegion *WRegionNode::getParentRegion() const {
  assert(!isa<WRegion>(this) && "Top Level WRegion cannot not have a parent!");

  WRegionNode *P = getParent();

  while (P && !isa<WRegion>(P)) {
    P = P->getParent();
  }
  return cast_or_null<WRegion>(P);
}
