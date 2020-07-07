// INTEL_FEATURE_ISA_TDX //===------------*- tablegen -*-------------------===//
//===- Intel_X86InstrTDX.td - TDX Instruction Set Extension -*- tablegen -*===//
//
//      Copyright (c) 2018-2019 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
//===----------------------------------------------------------------------===//
//
// This file describes the instructions that make up the Intel TDX instruction
// set.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// TDX instructions

// 64-bit only instructions
let SchedRW = [WriteSystem], Predicates = [In64BitMode] in {
// SEAMCALL - Call to SEAM VMX-root Operation Module
def SEAMCALL : I<0x01, MRM_CF, (outs), (ins),
             "seamcall", []>, PD;

// SEAMRET - Return to Legacy VMX-root Operation
def SEAMRET : I<0x01, MRM_CD, (outs), (ins),
             "seamret", []>, TB;

// SEAMOPS - SEAM Operations
def SEAMOPS : I<0x01, MRM_CE, (outs), (ins),
             "seamops", []>, TB;

} // SchedRW

// common instructions
let SchedRW = [WriteSystem] in {
// TDCALL - Call SEAM Module Functions
def TDCALL : I<0x01, MRM_CC, (outs), (ins),
             "tdcall", []>, TB;

} // SchedRW
// end INTEL_FEATURE_ISA_TDX