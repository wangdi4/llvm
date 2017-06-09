//===-- Nios2SERegisterInfo.cpp - NIOS2 Register Information ------== -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the NIOS2 implementation of the TargetRegisterInfo
// class.
//
//===----------------------------------------------------------------------===//

#include "Nios2SERegisterInfo.h"

using namespace llvm;

#define DEBUG_TYPE "nios2-reg-info"

Nios2SERegisterInfo::Nios2SERegisterInfo(const Nios2Subtarget &ST)
  : Nios2RegisterInfo(ST) {}

const TargetRegisterClass *
Nios2SERegisterInfo::intRegClass(unsigned Size) const {
  return &Nios2::CPURegsRegClass;
}
