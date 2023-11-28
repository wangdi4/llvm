//===-- CSASelectionDAGInfo.h - CSA SelectionDAG Info -----------*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
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

} // namespace llvm

#endif
