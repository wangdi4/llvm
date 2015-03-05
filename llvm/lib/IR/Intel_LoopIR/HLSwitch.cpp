//===---- HLSwitch.cpp - Implements the HLSwitch class --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLSwitch class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"

#include "llvm/IR/Intel_LoopIR/HLSwitch.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

HLSwitch::HLSwitch() : HLDDNode(HLNode::HLSwitchVal) {}

HLSwitch::HLSwitch(const HLSwitch &HLSwitchObj) : HLDDNode(HLSwitchObj) {
  unsigned Count = 0;

  /// Clone DDRefs
  for (auto I = HLSwitchObj.ddref_begin(), E = HLSwitchObj.ddref_end(); I != E;
       I++, Count++) {
    setOperandDDRef((*I)->clone(), Count);
  }
}

HLSwitch *HLSwitch::clone() const {

  /// Call the Copy Constructor
  HLSwitch *NewHLSwitch = new HLSwitch(*this);

  return NewHLSwitch;
}
