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

// Note: C0 is control input, C1 is control output

// LPU 64-bit C return-value convention.
def RetCC_LPU : CallingConv<[
  CCIfType<[i32,i64,f32,f64], CCAssignToReg<[C18, C19]>>,
  CCAssignToStack<8, 8>
]>;

// LPU 64-bit C Calling convention.
def CC_LPU : CallingConv<[
  CCIfType<[i32,i64,f32,f64],
        CCAssignToReg<[C2, C3, C4, C5, C6, C7, C8, C9,
                       C10, C11, C12, C13, C14, C15, C16, C17]>>,
  CCAssignToStack<8, 8>
]>;
