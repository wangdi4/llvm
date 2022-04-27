/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2021 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may not
 * use, modify, copy, publish, distribute, disclose or transmit this software or
 * the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/* end INTEL_CUSTOMIZATION */
//===-- X86InstPrinterCommon.cpp - X86 assembly instruction printing ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file includes code common for rendering MCInst instances as AT&T-style
// and Intel-style assembly.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_MCTARGETDESC_X86INSTPRINTERCOMMON_H
#define LLVM_LIB_TARGET_X86_MCTARGETDESC_X86INSTPRINTERCOMMON_H

#include "llvm/MC/MCInstPrinter.h"

namespace llvm {

class X86InstPrinterCommon : public MCInstPrinter {
public:
  using MCInstPrinter::MCInstPrinter;

  virtual void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O) = 0;
  void printCondCode(const MCInst *MI, unsigned Op, raw_ostream &OS);
  void printSSEAVXCC(const MCInst *MI, unsigned Op, raw_ostream &OS);
  void printVPCOMMnemonic(const MCInst *MI, raw_ostream &OS);
  void printVPCMPMnemonic(const MCInst *MI, raw_ostream &OS);
  void printCMPMnemonic(const MCInst *MI, bool IsVCmp, raw_ostream &OS);
  void printRoundingControl(const MCInst *MI, unsigned Op, raw_ostream &O);
  void printPCRelImm(const MCInst *MI, uint64_t Address, unsigned OpNo,
                     raw_ostream &O);

protected:
  void printInstFlags(const MCInst *MI, raw_ostream &O,
                      const MCSubtargetInfo &STI);
  void printOptionalSegReg(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printVKPair(const MCInst *MI, unsigned OpNo, raw_ostream &OS);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AMX_LNC
  void printTILEPair(const MCInst *MI, unsigned OpNo, raw_ostream &OS);
#endif // INTEL_FEATURE_ISA_AMX_LNC
#if INTEL_FEATURE_ISA_AMX_TRANSPOSE2
  void printTILEQuad(const MCInst *MI, unsigned OpNo, raw_ostream &OS);
#endif // INTEL_FEATURE_ISA_AMX_TRANSPOSE2
#endif // INTEL_CUSTOMIZATION
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_X86_MCTARGETDESC_X86INSTPRINTERCOMMON_H
