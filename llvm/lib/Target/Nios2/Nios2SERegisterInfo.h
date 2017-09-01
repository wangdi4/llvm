//===-- Nios2SERegisterInfo.h - Nios232 Register Information ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Nios232/64 implementation of the TargetRegisterInfo
// class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NIOS2_NIOS2SEREGISTERINFO_H
#define LLVM_LIB_TARGET_NIOS2_NIOS2SEREGISTERINFO_H

#include "Nios2RegisterInfo.h"

namespace llvm {
class Nios2SEInstrInfo;

class Nios2SERegisterInfo : public Nios2RegisterInfo {
public:
  Nios2SERegisterInfo(const Nios2Subtarget &Subtarget);

  const TargetRegisterClass *intRegClass(unsigned Size) const override;
};

} // end namespace llvm

#endif
