//===-- X86InstrFoldTables.h - X86 Instruction Folding Tables ---*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the interface to query the X86 memory folding tables.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_X86INSTRFOLDTABLES_H
#define LLVM_LIB_TARGET_X86_X86INSTRFOLDTABLES_H

#include <cstdint>
#include "llvm/Support/X86FoldTablesUtils.h"

namespace llvm {

// This struct is used for both the folding and unfold tables. They KeyOp
// is used to determine the sorting order.
struct X86MemoryFoldTableEntry {
  uint16_t KeyOp;
  uint16_t DstOp;
  uint16_t Flags;

  bool operator<(const X86MemoryFoldTableEntry &RHS) const {
    return KeyOp < RHS.KeyOp;
  }
  bool operator==(const X86MemoryFoldTableEntry &RHS) const {
    return KeyOp == RHS.KeyOp;
  }
  friend bool operator<(const X86MemoryFoldTableEntry &TE, unsigned Opcode) {
    return TE.KeyOp < Opcode;
  }
};

// Look up the memory folding table entry for folding a load and a store into
// operand 0.
const X86MemoryFoldTableEntry *lookupTwoAddrFoldTable(unsigned RegOp);

// Look up the memory folding table entry for folding a load or store with
// operand OpNum.
const X86MemoryFoldTableEntry *lookupFoldTable(unsigned RegOp, unsigned OpNum);

#if INTEL_CUSTOMIZATION
// Look up the broadcast folding table entry for folding a broadcast with
// operand OpNum.
const X86MemoryFoldTableEntry *lookupBroadcastFoldTable(unsigned RegOp,
                                                        unsigned OpNum);
#endif

// Look up the memory unfolding table entry for this instruction.
const X86MemoryFoldTableEntry *lookupUnfoldTable(unsigned MemOp);

} // namespace llvm

#endif
