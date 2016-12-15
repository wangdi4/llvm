//===-- CSASelectionDAGInfo.h - CSA SelectionDAG Info -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the CSA subclass for SelectionDAGTargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSASELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_CSA_CSASELECTIONDAGINFO_H

#include "llvm/CodeGen/SelectionDAGTargetInfo.h"

namespace llvm {

class CSATargetMachine;

class CSASelectionDAGInfo : public SelectionDAGTargetInfo {
public:
  explicit CSASelectionDAGInfo() = default;
};

}

#endif
