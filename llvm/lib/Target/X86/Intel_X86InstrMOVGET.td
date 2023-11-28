#if INTEL_FEATURE_ISA_GPR_MOVGET
//===---------------------*- tablegen -*-----------------------------------===//
//=- Intel_X86InstrMOVGET.td - MOVGET Instruction Set Extension -*- tablegen -*=//
//
//      Copyright (c) 2018 Intel Corporation.
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
// This file describes the instructions that make up the Intel MOVGET instruction
// set.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// MOVGET instructions

let Predicates = [HasGPRMOVGET] in
  def MOVGET32rm : I<0xfa, MRMSrcMem, (outs GR32:$dst), (ins i32mem:$src),
                    "movget{l}\t{$src, $dst|$dst, $src}",
                    [(set GR32:$dst, (int_x86_movget32 addr:$src))]>,
                    T8XS, OpSize32;
let Predicates = [HasGPRMOVGET, In64BitMode] in
  def MOVGET64rm : I<0xfa, MRMSrcMem, (outs GR64:$dst), (ins i64mem:$src),
                    "movget{q}\t{$src, $dst|$dst, $src}",
                    [(set GR64:$dst, (int_x86_movget64 addr:$src))]>,
                    T8XS, REX_W;
#endif // INTEL_FEATURE_ISA_GPR_MOVGET

#if INTEL_FEATURE_ISA_MOVGET64B
let Predicates = [HasMOVGET64B] in
  def MOVGET64B32rm : I<0xfa, MRMSrcMem, (outs GR32:$dst), (ins i512mem:$src),
                      "movget64b{l}\t{$src, $dst|$dst, $src}",
                      [(set GR32:$dst, (int_x86_movget64b32 addr:$src))]>,
                      T8PS, OpSize32;
let Predicates = [HasMOVGET64B, In64BitMode] in
  def MOVGET64B64rm : I<0xfa, MRMSrcMem, (outs GR64:$dst), (ins i512mem:$src),
                      "movget64b{q}\t{$src, $dst|$dst, $src}",
                      [(set GR64:$dst, (int_x86_movget64b64 addr:$src))]>,
                      T8PS, REX_W;
#endif // INTEL_FEATURE_ISA_MOVGET64B
