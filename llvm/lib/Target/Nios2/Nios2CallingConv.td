//===-- Nios2CallingConv.td - Calling Conventions for Nios2 --*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This describes the calling conventions for Nios2 architecture.
//===----------------------------------------------------------------------===//

/// CCIfSubtarget - Match if the current subtarget has a feature F.
class CCIfSubtarget<string F, CCAction A>:
  CCIf<!strconcat("State.getTarget().getSubtarget<Nios2Subtarget>().", F), A>;

def RetCC_Nios2EABI : CallingConv<[
  // i32 are returned in registers R3, R3
  CCIfType<[i32], CCAssignToReg<[R2, R3]>>,
  CCIfByVal<CCPassByVal<4, 4>>,
  // Stack parameter slots for i32 is 32-bit words and 4-byte aligned.
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

def RetCC_Nios2 : CallingConv<[
  CCDelegateTo<RetCC_Nios2EABI>
]>;

def CSR_O32 : CalleeSavedRegs<(add RA, FP, (sequence "R%u", 16, 23))>;

