//===-- LPUSelectionDAGInfo.h - LPU SelectionDAG Info -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the LPU subclass for TargetSelectionDAGInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LPU_LPUSELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_LPU_LPUSELECTIONDAGINFO_H

#include "llvm/Target/TargetSelectionDAGInfo.h"

namespace llvm {

class LPUTargetMachine;

class LPUSelectionDAGInfo : public TargetSelectionDAGInfo {
public:
  explicit LPUSelectionDAGInfo(const DataLayout &DL);
  ~LPUSelectionDAGInfo();
};

}

#endif
