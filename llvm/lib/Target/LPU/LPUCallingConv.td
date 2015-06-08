//===- LPUCallingConv.td - Calling Conventions LPU ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the LPU architecture.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Return Value Calling Conventions
//===----------------------------------------------------------------------===//

// LPU 64-bit C return-value convention.
def RetCC_LPU : CallingConv<[
  CCIfType<[i32,i64,f32,f64], CCAssignToReg<[R4, R5, R6, R7]>>,

  // Alternatively, they are assigned to the stack in 8-byte aligned units.
  CCAssignToStack<8, 8>
]>;

// LPU 64-bit C Calling convention.
def CC_LPU : CallingConv<[
  // All arguments get passed in integer registers if there is space.
  CCIfType<[i32,i64,f32,f64], CCAssignToReg<[R0, R1, R2, R3]>>,

  // Alternatively, they are assigned to the stack in 8-byte aligned units.
  CCAssignToStack<8, 8>
]>;
