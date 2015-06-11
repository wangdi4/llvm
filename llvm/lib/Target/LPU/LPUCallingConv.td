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
  CCIfType<[i32,i64,f32,f64], CCAssignToReg<[RES0, RES1, RES2, RES3, RES4, RES5, RES6, RES7]>>,
  CCAssignToStack<8, 8>
]>;

// LPU 64-bit C Calling convention.
def CC_LPU : CallingConv<[
  CCIfType<[i32,i64,f32,f64],
        CCAssignToReg<[ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7,
                       ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15]>>,
  CCAssignToStack<8, 8>
]>;
