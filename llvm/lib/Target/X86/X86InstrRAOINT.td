//===---- X86InstrRAOINT.td -------------------------------*- tablegen -*--===//
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
// This file describes the instructions that make up the Intel RAO-INT
// instruction set.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// RAO-INT instructions

def SDTRAOBinaryArith : SDTypeProfile<0, 2, [SDTCisPtrTy<0>, SDTCisInt<1>]>;

def X86rao_add  : SDNode<"X86ISD::AADD", SDTRAOBinaryArith,
                         [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def X86rao_or   : SDNode<"X86ISD::AOR",  SDTRAOBinaryArith,
                         [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def X86rao_xor  : SDNode<"X86ISD::AXOR", SDTRAOBinaryArith,
                         [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def X86rao_and  : SDNode<"X86ISD::AAND", SDTRAOBinaryArith,
                         [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;

multiclass RAOINT_BASE<string OpcodeStr> {
#if INTEL_CUSTOMIZATION
  let Predicates = [HasRAOINT, NoEGPR] in
#endif // INTEL_CUSTOMIZATION
    def 32mr : I<0xfc, MRMDestMem, (outs), (ins i32mem:$dst, GR32:$src),
                 !strconcat("a", OpcodeStr, "{l}\t{$src, $dst|$dst, $src}"),
                 [(!cast<SDNode>("X86rao_" # OpcodeStr) addr:$dst, GR32:$src)]>,
               Sched<[WriteALURMW]>;

#if INTEL_CUSTOMIZATION
  let Predicates = [HasRAOINT, NoEGPR, In64BitMode] in
#endif // INTEL_CUSTOMIZATION
    def 64mr : I<0xfc, MRMDestMem, (outs), (ins i64mem:$dst, GR64:$src),
                 !strconcat("a", OpcodeStr, "{q}\t{$src, $dst|$dst, $src}"),
                 [(!cast<SDNode>("X86rao_" # OpcodeStr) addr:$dst, GR64:$src)]>,
               Sched<[WriteALURMW]>, REX_W;
}

#if INTEL_CUSTOMIZATION
multiclass RAOINT_BASE_EVEX<string OpcodeStr> {
let Predicates = [HasRAOINT, HasEGPR, In64BitMode] in {
  def 32mr_EVEX : ITy_EVEX<0xfc, MRMDestMem, Xi32, (outs), (ins i32mem:$dst, GR32:$src),
                    !strconcat("a", OpcodeStr), "{$src, $dst|$dst, $src}",
                    [(!cast<SDNode>("X86rao_" # OpcodeStr) addr:$dst, GR32:$src)], 0>,
                  Sched<[WriteALURMW]>;
  def 64mr_EVEX : ITy_EVEX<0xfc, MRMDestMem, Xi64, (outs), (ins i64mem:$dst, GR64:$src),
                    !strconcat("a", OpcodeStr), "{$src, $dst|$dst, $src}",
                    [(!cast<SDNode>("X86rao_" # OpcodeStr) addr:$dst, GR64:$src)], 0>,
                  Sched<[WriteALURMW]>;
}
}
#endif // INTEL_CUSTOMIZATION

defm AADD : RAOINT_BASE<"add">, T8PS;
defm AAND : RAOINT_BASE<"and">, T8PD;
defm AOR  : RAOINT_BASE<"or" >, T8XD;
defm AXOR : RAOINT_BASE<"xor">, T8XS;
#if INTEL_CUSTOMIZATION
defm AADD : RAOINT_BASE_EVEX<"add">, T_MAP4PS;
defm AAND : RAOINT_BASE_EVEX<"and">, T_MAP4PD;
defm AOR  : RAOINT_BASE_EVEX<"or" >, T_MAP4XD;
defm AXOR : RAOINT_BASE_EVEX<"xor">, T_MAP4XS;
#endif // INTEL_CUSTOMIZATION
