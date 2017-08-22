//=- Nios2RegisterBank.td - Describe the Nios2 Banks -----*- tablegen -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

/// General Purpose Registers: W, X.
def GPRRegBank : RegisterBank<"GPR", [CPURegs]>;
