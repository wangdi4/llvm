//===-- LPUSelectionDAGInfo.h - LPU SelectionDAG Info -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the LPU subclass for SelectionDAGTargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LPU_LPUSELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_LPU_LPUSELECTIONDAGINFO_H

#include "llvm/CodeGen/SelectionDAGTargetInfo.h"

namespace llvm {

class LPUTargetMachine;

class LPUSelectionDAGInfo : public SelectionDAGTargetInfo {
public:
  explicit LPUSelectionDAGInfo() = default;
};

}

#endif
