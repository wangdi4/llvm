//===-- CSAMCAsmInfo.h - CSA asm properties --------------------*- C++ -*--===//
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
// This file contains the declaration of the CSAMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_MCTARGETDESC_CSAMCASMINFO_H
#define LLVM_LIB_TARGET_CSA_MCTARGETDESC_CSAMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
class Triple;

class CSAMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit CSAMCAsmInfo(const Triple &T, const MCTargetOptions &Options);
};

} // namespace llvm

#endif
