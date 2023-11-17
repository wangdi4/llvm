//===-- X86InstrCMovSetCC.td - Conditional Move and SetCC --*- tablegen -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
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
// This file describes the X86 conditional move and set on condition
// instructions.
//
//===----------------------------------------------------------------------===//


#if INTEL_CUSTOMIZATION
class cmovcc_rr<bits<8> opcode, X86TypeInfo info>
  : ITy<opcode, MRMSrcRegCC, info, (outs info.RegClass:$dst),
        (ins info.RegClass:$src1, info.RegClass:$src2, ccode:$cond),
        "cmov${cond}", "{$src2, $dst|$dst, $src2}",
        [(set info.RegClass:$dst,
         (X86cmov info.RegClass:$src1,
                  info.RegClass:$src2, timm:$cond, EFLAGS))]> {
  let Opcode = opcode;
}

class cmovcc_rr_ND<bits<8> opcode, X86TypeInfo info>
  : ITy_EVEX<opcode, MRMSrcRegCC, info, (outs info.RegClass:$dst),
             (ins info.RegClass:$src1, info.RegClass:$src2, ccode:$cond),
             "cmov${cond}", "{$src2, $src1, $dst|$dst, $src1, $src2}",
             [(set info.RegClass:$dst,
              (X86cmov info.RegClass:$src1, info.RegClass:$src2,
                       timm:$cond, EFLAGS))]> {
  let Opcode = opcode;
}

class cmovcc_rm<bits<8> opcode, X86TypeInfo info>
  : ITy<opcode, MRMSrcMemCC, info, (outs info.RegClass:$dst),
        (ins info.RegClass:$src1, info.MemOperand:$src2, ccode:$cond),
        "cmov${cond}", "{$src2, $dst|$dst, $src2}",
        [(set info.RegClass:$dst,
         (X86cmov info.RegClass:$src1, (info.LoadNode addr:$src2),
                  timm:$cond, EFLAGS))]> {
  let Opcode = opcode;
}

class cmovcc_rm_ND<bits<8> opcode, X86TypeInfo info>
  : ITy_EVEX<opcode, MRMSrcMemCC, info, (outs info.RegClass:$dst),
             (ins info.RegClass:$src1, info.MemOperand:$src2, ccode:$cond),
             "cmov${cond}", "{$src2, $src1, $dst|$dst, $src1, $src2}",
             [(set info.RegClass:$dst,
              (X86cmov info.RegClass:$src1, (info.LoadNode addr:$src2),
                       timm:$cond, EFLAGS))]> {
  let Opcode = opcode;
}

multiclass cmovcc_base<bits<8> opcode, X86TypeInfo info> {
let isCommutable = 1, SchedRW = [WriteCMOV] in {
let Predicates = [HasCMOV, NoNDD], Constraints = "$src1 = $dst" in
  def rr : cmovcc_rr<opcode, info>, TB;
let Predicates = [HasCMOV, HasNDD, In64BitMode] in
  def rr_ND : cmovcc_rr_ND<opcode, info>, EVEX_B4V;
}

let SchedRW = [WriteCMOV.Folded, WriteCMOV.ReadAfterFold] in {
let Predicates = [HasCMOV, NoNDD], Constraints = "$src1 = $dst" in
  def rm : cmovcc_rm<opcode, info>, TB;
let Predicates = [HasCMOV, HasNDD, In64BitMode] in
  def rm_ND : cmovcc_rm_ND<opcode, info>, EVEX_B4V;
}
}

let Uses = [EFLAGS], isCodeGenOnly = 1, ForceDisassemble = 1 in {
  defm CMOV16 : cmovcc_base<0x40, Xi16>;
  defm CMOV32 : cmovcc_base<0x40, Xi32>;
  defm CMOV64 : cmovcc_base<0x40, Xi64>;
}

class cfcmovcc_rr<bits<8> opcode, X86TypeInfo info>
  : ITy_EVEX<opcode, MRMSrcRegCC, info, (outs info.RegClass:$dst),
             (ins info.RegClass:$src1, ccode:$cond),
             "cfcmov${cond}", "{$src1, $dst|$dst, $src1}",
             [(set info.RegClass:$dst,
              (X86cmov 0, info.RegClass:$src1, timm:$cond, EFLAGS))]> {
  let Opcode = opcode;
}

class cfcmovcc_rr_REV<bits<8> opcode, X86TypeInfo info>
  : ITy_EVEX<opcode, MRMDestRegCC, info, (outs info.RegClass:$dst),
             (ins info.RegClass:$src1, ccode:$cond),
             "cfcmov${cond}", "{$src1, $dst|$dst, $src1}", []> {
  let Opcode = opcode;
}

class cfcmovcc_rr_ND<bits<8> opcode, X86TypeInfo info>
  : ITy_EVEX<opcode, MRMSrcRegCC, info, (outs info.RegClass:$dst),
             (ins info.RegClass:$src1, info.RegClass:$src2, ccode:$cond),
             "cfcmov${cond}", "{$src2, $src1, $dst|$dst, $src1, $src2}", []> {
  let Opcode = opcode;
}

class cfcmovcc_rm<bits<8> opcode, X86TypeInfo info>
  : ITy_EVEX<opcode, MRMSrcMemCC, info, (outs info.RegClass:$dst),
             (ins info.MemOperand:$src1, ccode:$cond),
             "cfcmov${cond}", "{$src1, $dst|$dst, $src1}", []> {
  let Opcode = opcode;
}

class cfcmovcc_mr<bits<8> opcode, X86TypeInfo info>
  : ITy_EVEX<opcode, MRMDestMemCC, info, (outs info.MemOperand:$dst),
             (ins info.RegClass:$src1, ccode:$cond),
             "cfcmov${cond}", "{$src1, $dst|$dst, $src1}", []> {
  let Opcode = opcode;
}

class cfcmovcc_rm_ND<bits<8> opcode, X86TypeInfo info>
  : ITy_EVEX<opcode, MRMSrcMemCC, info, (outs info.RegClass:$dst),
             (ins info.RegClass:$src1, info.MemOperand:$src2, ccode:$cond),
             "cfcmov${cond}", "{$src2, $src1, $dst|$dst, $src1, $src2}", []> {
  let Opcode = opcode;
}

multiclass cfcmovcc_base<bits<8> opcode, X86TypeInfo info> {
let isCommutable = 1, SchedRW = [WriteCMOV] in {
let Predicates = [HasCMOV, HasCF, In64BitMode] in {
  def rr : cfcmovcc_rr<opcode, info>;
  def rr_REV : cfcmovcc_rr_REV<opcode, info>, EVEX_NF;
}
let Predicates = [HasCMOV, HasCF, HasNDD, In64BitMode] in
  def rr_ND : cfcmovcc_rr_ND<opcode, info>, EVEX_B4V, EVEX_NF;
}

let SchedRW = [WriteCMOV.Folded, WriteCMOV.ReadAfterFold] in {
let Predicates = [HasCMOV, HasCF, In64BitMode] in {
  let mayLoad = 1 in
  def rm : cfcmovcc_rm<opcode, info>;
  let mayStore = 1 in
  def mr : cfcmovcc_mr<opcode, info>, EVEX_NF;
}
let Predicates = [HasCMOV, HasCF, HasNDD, In64BitMode], mayLoad = 1 in
  def rm_ND : cfcmovcc_rm_ND<opcode, info>, EVEX_B4V, EVEX_NF;
}
}

let Uses = [EFLAGS], isCodeGenOnly = 1, ForceDisassemble = 1 in {
  defm CFCMOV16 : cfcmovcc_base<0x40, Xi16>;
  defm CFCMOV32 : cfcmovcc_base<0x40, Xi32>;
  defm CFCMOV64 : cfcmovcc_base<0x40, Xi64>;
}
#endif // INTEL_CUSTOMIZATION

def inv_cond_XFORM : SDNodeXForm<imm, [{
  X86::CondCode CC = static_cast<X86::CondCode>(N->getZExtValue());
  return CurDAG->getTargetConstant(X86::GetOppositeBranchCondition(CC),
                                   SDLoc(N), MVT::i8);
}]>;

// Conditional moves with folded loads with operands swapped and conditions
// inverted.
#if INTEL_CUSTOMIZATION
let Predicates = [HasCMOV, NoNDD] in {
#endif // INTEL_CUSTOMIZATION
  def : Pat<(X86cmov (loadi16 addr:$src1), GR16:$src2, timm:$cond, EFLAGS),
            (CMOV16rm GR16:$src2, addr:$src1, (inv_cond_XFORM timm:$cond))>;
  def : Pat<(X86cmov (loadi32 addr:$src1), GR32:$src2, timm:$cond, EFLAGS),
            (CMOV32rm GR32:$src2, addr:$src1, (inv_cond_XFORM timm:$cond))>;
  def : Pat<(X86cmov (loadi64 addr:$src1), GR64:$src2, timm:$cond, EFLAGS),
            (CMOV64rm GR64:$src2, addr:$src1, (inv_cond_XFORM timm:$cond))>;
}

#if INTEL_CUSTOMIZATION
let Predicates = [HasCMOV, HasNDD] in {
  def : Pat<(X86cmov (loadi16 addr:$src1), GR16:$src2, timm:$cond, EFLAGS),
            (CMOV16rm_ND GR16:$src2, addr:$src1, (inv_cond_XFORM timm:$cond))>;
  def : Pat<(X86cmov (loadi32 addr:$src1), GR32:$src2, timm:$cond, EFLAGS),
            (CMOV32rm_ND GR32:$src2, addr:$src1, (inv_cond_XFORM timm:$cond))>;
  def : Pat<(X86cmov (loadi64 addr:$src1), GR64:$src2, timm:$cond, EFLAGS),
            (CMOV64rm_ND GR64:$src2, addr:$src1, (inv_cond_XFORM timm:$cond))>;
}
let Predicates = [HasCMOV, HasCF] in {
  def : Pat<(X86cmov GR16:$src1, 0, timm:$cond, EFLAGS),
            (CFCMOV16rr GR16:$src1, (inv_cond_XFORM timm:$cond))>;
  def : Pat<(X86cmov GR32:$src1, 0, timm:$cond, EFLAGS),
            (CFCMOV32rr GR32:$src1, (inv_cond_XFORM timm:$cond))>;
  def : Pat<(X86cmov GR64:$src1, 0, timm:$cond, EFLAGS),
            (CFCMOV64rr GR64:$src1, (inv_cond_XFORM timm:$cond))>;
}
#endif // INTEL_CUSTOMIZATION
// SetCC instructions.
let Uses = [EFLAGS], isCodeGenOnly = 1, ForceDisassemble = 1 in {
  def SETCCr : I<0x90, MRMXrCC, (outs GR8:$dst), (ins ccode:$cond),
                "set${cond}\t$dst",
                [(set GR8:$dst, (X86setcc timm:$cond, EFLAGS))]>,
                TB, Sched<[WriteSETCC]>;
  def SETCCm : I<0x90, MRMXmCC, (outs), (ins i8mem:$dst, ccode:$cond),
                "set${cond}\t$dst",
                [(store (X86setcc timm:$cond, EFLAGS), addr:$dst)]>,
                TB, Sched<[WriteSETCCStore]>;
} // Uses = [EFLAGS]

// SALC is an undocumented instruction. Information for this instruction can be found
// here http://www.rcollins.org/secrets/opcodes/SALC.html
// Set AL if carry. 
let Uses = [EFLAGS], Defs = [AL], SchedRW = [WriteALU] in {
  def SALC : I<0xD6, RawFrm, (outs), (ins), "salc", []>, Requires<[Not64BitMode]>;
}
