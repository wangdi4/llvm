//===- CSACallingConv.td - Calling Conventions CSA ---------*- tablegen -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the CSA architecture.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Return Value Calling Conventions
//===----------------------------------------------------------------------===//

// For register assignments generally, see CSARegisterInfo.td

// CSA 64-bit C return-value convention using registers
def RetCC_Reg_CSA : CallingConv<[
  CCIfType<[i1,i8,i16,i32,i64,f16,f32,f64,v8i8,v4i16,v4f16,v2f32],
           CCAssignToReg<[R0, R1]>>,
  CCAssignToStack<8, 8>
]>;

// CSA 64-bit C Calling convention using registers
def CC_Reg_CSA : CallingConv<[
  CCIfType<[i1,i8,i16,i32,i64,f16,f32,f64,v8i8,v4i16,v4f16,v2f32],
        CCAssignToReg<[R2, R3, R4, R5, R6, R7, R8, R9,
                       R10, R11, R12, R13, R14, R15, R16, R17]>>,
  CCAssignToStack<8, 8>
]>;

// CSA 64-bit C Calling convention for varargs - all memory
def CC_Reg_VarArg_CSA : CallingConv<[
  CCAssignToStack<8, 8>
]>;

