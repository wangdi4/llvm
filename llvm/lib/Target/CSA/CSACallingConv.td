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

// For register assignments generally, see LPURegisterInfo.td

// LPU 64-bit C return-value convention using registers
def RetCC_Reg_LPU : CallingConv<[
  CCIfType<[i1,i8,i16,i32,i64,f16,f32,f64], CCAssignToReg<[R0, R1]>>,
  CCAssignToStack<8, 8>
]>;

// LPU 64-bit C Calling convention using registers
def CC_Reg_LPU : CallingConv<[
  CCIfType<[i1,i8,i16,i32,i64,f16,f32,f64],
        CCAssignToReg<[R2, R3, R4, R5, R6, R7, R8, R9,
                       R10, R11, R12, R13, R14, R15, R16, R17]>>,
  CCAssignToStack<8, 8>
]>;

// LPU 64-bit C Calling convention for varargs - all memory
def CC_Reg_VarArg_LPU : CallingConv<[
  CCAssignToStack<8, 8>
]>;

/*
Need to re-think LIC handling

// LPU 64-bit C return-value convention using LICs
def RetCC_LIC_LPU : CallingConv<[
  CCIfType<[i32,i64,f32,f64], CCAssignToReg<[C2, C3]>>,
  CCAssignToStack<8, 8>
]>;

// LPU 64-bit C Calling convention using LICs
def CC_LIC_LPU : CallingConv<[
  CCIfType<[i32,i64,f32,f64],
        CCAssignToReg<[C4, C5, C6, C7, C8, C9, C10, C11,
                       C12, C13, C14, C15, C16, C17, C18, C19]>>,
  CCAssignToStack<8, 8>
]>;
*/
