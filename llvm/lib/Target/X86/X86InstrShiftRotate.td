//===-- X86InstrShiftRotate.td - Shift and Rotate Instrs ---*- tablegen -*-===//
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
// This file describes the shift and rotate instructions.
//
//===----------------------------------------------------------------------===//

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_APX_F
// The class below named I_* means it's an Instruction inherited by ITy,
// The chars behind underline represent their inputs and outputs, use a '_' to
// connect. In it, R means Register, M means mem, I means Imm, so I_R_R means
// this class denotes an instruction with one output reg and one input reg.
class I_R_R<bits<8> opcode, Format f, X86TypeInfo typeinfo, string mnemonic,
            string operands, list<dag> patterns>
  : ITy<opcode, f, typeinfo, (outs typeinfo.RegClass:$dst),
        (ins typeinfo.RegClass:$src1), mnemonic, operands, patterns>;
class I_R_R_EVEX<bits<8> opcode, Format f, X86TypeInfo typeinfo, string mnemonic,
                 string operands, list<dag> patterns>
  : ITy_EVEX<opcode, f, typeinfo, (outs typeinfo.RegClass:$dst),
            (ins typeinfo.RegClass:$src1), mnemonic, operands, patterns>;

class I_R_RI<bits<8> opcode, Format f, X86TypeInfo typeinfo, string mnemonic,
             string operands, list<dag> patterns>
  : ITy<opcode, f, typeinfo, (outs typeinfo.RegClass:$dst),
        (ins typeinfo.RegClass:$src1, u8imm:$src2),
        mnemonic, operands, patterns>;
class I_R_RI_EVEX<bits<8> opcode, Format f, X86TypeInfo typeinfo, string mnemonic,
                  string operands, list<dag> patterns>
  : ITy_EVEX<opcode, f, typeinfo, (outs typeinfo.RegClass:$dst),
        (ins typeinfo.RegClass:$src1, u8imm:$src2),
        mnemonic, operands, patterns>;

class I_N_M<bits<8> opcode, Format f, X86TypeInfo typeinfo, string mnemonic,
            string operands, list<dag> patterns>
  : ITy<opcode, f, typeinfo, (outs), (ins typeinfo.MemOperand:$dst),
        mnemonic, operands, patterns>;
class I_N_M_EVEX<bits<8> opcode, Format f, X86TypeInfo typeinfo,
                 string mnemonic, string operands, list<dag> patterns>
  : ITy_EVEX<opcode, f, typeinfo, (outs), (ins typeinfo.MemOperand:$dst),
             mnemonic, operands, patterns>;

class I_R_M_EVEX<bits<8> opcode, Format f, X86TypeInfo typeinfo,
                 string mnemonic, string operands, list<dag> patterns>
  : ITy_EVEX<opcode, f, typeinfo, (outs typeinfo.RegClass:$dst),
             (ins typeinfo.MemOperand:$src1), mnemonic, operands, patterns>;

class I_N_MI<bits<8> opcode, Format f, X86TypeInfo typeinfo, string mnemonic,
             string operands, list<dag> patterns>
  : ITy<opcode, f, typeinfo, (outs),
        (ins typeinfo.MemOperand:$dst, u8imm:$src),
        mnemonic, operands, patterns>;
class I_N_MI_EVEX<bits<8> opcode, Format f, X86TypeInfo typeinfo,
                  string mnemonic, string operands, list<dag> patterns>
  : ITy_EVEX<opcode, f, typeinfo, (outs),
             (ins typeinfo.MemOperand:$dst, u8imm:$src),
             mnemonic, operands, patterns>;

class I_R_MI_EVEX<bits<8> opcode, Format f, X86TypeInfo typeinfo,
                  string mnemonic, string operands, list<dag> patterns>
  : ITy_EVEX<opcode, f, typeinfo, (outs typeinfo.RegClass:$dst),
             (ins typeinfo.MemOperand:$src1, u8imm:$src2),
             mnemonic, operands, patterns>;

class I_R_RR<bits<8> opcode, Format f, X86TypeInfo typeinfo, string mnemonic,
             string operands, list<dag> patterns>
  : ITy<opcode, f, typeinfo, (outs typeinfo.RegClass:$dst),
        (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2), mnemonic,
        operands, patterns>;
class I_R_RR_EVEX<bits<8> opcode, Format f, X86TypeInfo typeinfo,
                  string mnemonic, string operands, list<dag> patterns>
  : ITy_EVEX<opcode, f, typeinfo, (outs typeinfo.RegClass:$dst),
             (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2), mnemonic,
             operands, patterns>;

class I_R_RRI<bits<8> opcode, Format f, X86TypeInfo typeinfo, string mnemonic,
              string operands, list<dag> patterns>
  : ITy<opcode, f, typeinfo, (outs typeinfo.RegClass:$dst),
        (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2, u8imm:$src3),
        mnemonic, operands, patterns>;
class I_R_RRI_EVEX<bits<8> opcode, Format f, X86TypeInfo typeinfo,
                   string mnemonic, string operands, list<dag> patterns>
  : ITy_EVEX<opcode, f, typeinfo, (outs typeinfo.RegClass:$dst),
             (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2, u8imm:$src3),
              mnemonic, operands, patterns>;

class I_N_MR<bits<8> opcode, Format f, X86TypeInfo typeinfo,
             string mnemonic, string operands, list<dag> patterns>
  : ITy<opcode, f, typeinfo, (outs),
        (ins typeinfo.MemOperand:$dst, typeinfo.RegClass:$src2),
        mnemonic, operands, patterns>;
class I_N_MR_EVEX<bits<8> opcode, Format f, X86TypeInfo typeinfo,
             string mnemonic, string operands, list<dag> patterns>
  : ITy_EVEX<opcode, f, typeinfo, (outs),
             (ins typeinfo.MemOperand:$dst, typeinfo.RegClass:$src2),
              mnemonic, operands, patterns>;

class I_R_MR_EVEX<bits<8> opcode, Format f, X86TypeInfo typeinfo,
                  string mnemonic, string operands, list<dag> patterns>
  : ITy_EVEX<opcode, f, typeinfo, (outs typeinfo.RegClass:$dst),
             (ins typeinfo.MemOperand:$src1, typeinfo.RegClass:$src2),
              mnemonic, operands, patterns>;

class I_N_MRI<bits<8> opcode, Format f, X86TypeInfo typeinfo,
              string mnemonic, string operands, list<dag> patterns>
  : ITy<opcode, f, typeinfo, (outs),
        (ins typeinfo.MemOperand:$dst, typeinfo.RegClass:$src2, u8imm:$src3),
        mnemonic, operands, patterns>;
class I_N_MRI_EVEX<bits<8> opcode, Format f, X86TypeInfo typeinfo,
                   string mnemonic, string operands, list<dag> patterns>
  : ITy_EVEX<opcode, f, typeinfo, (outs),
             (ins typeinfo.MemOperand:$dst,
              typeinfo.RegClass:$src2, u8imm:$src3),
             mnemonic, operands, patterns>;

class I_R_MRI_EVEX<bits<8> opcode, Format f, X86TypeInfo typeinfo,
                   string mnemonic, string operands, list<dag> patterns>
  : ITy_EVEX<opcode, f, typeinfo, (outs typeinfo.RegClass:$dst),
             (ins typeinfo.MemOperand:$src1,
              typeinfo.RegClass:$src2, u8imm:$src3),
             mnemonic, operands, patterns>;

class shift_rCL<bits<8> opcode, Format f, string mnemonic, SDNode node,
                X86TypeInfo typeinfo>
  : I_R_R<opcode, f, typeinfo, mnemonic,"{%cl, $dst|$dst, cl}",
          [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, CL))]>;

class shift_rCL_NDD<bits<8> opcode, Format f, string mnemonic, SDNode node,
                    X86TypeInfo typeinfo>
  : I_R_R_EVEX<opcode, f, typeinfo, mnemonic,
               "{%cl, $src1, $dst|$dst, $src1, cl}",
               [(set typeinfo.RegClass:$dst,
                (node typeinfo.RegClass:$src1, CL))]>;

class shift_rCL_NoPat<bits<8> opcode, Format f, string mnemonic,
                      X86TypeInfo typeinfo>
  : I_R_R_EVEX<opcode, f, typeinfo, mnemonic, "{%cl, $dst|$dst, cl}", []>;

class shift_rCL_NDD_NoPat<bits<8> opcode, Format f, string mnemonic,
                          X86TypeInfo typeinfo>
  : I_R_R_EVEX<opcode, f, typeinfo, mnemonic,
               "{%cl, $src1, $dst|$dst, $src1, cl}", []>;

multiclass mshift_rCL<bits<8> opcode, Format f, string mnemonic, SDNode node,
                      X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst" in {
let Predicates = [NoNDD] in
  def NAME : shift_rCL<opcode, f, mnemonic, node, typeinfo>;
let Predicates = [In64BitMode] in
  def _EVEX : shift_rCL_NoPat<opcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
}
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shift_rCL_NDD<opcode, f, mnemonic, node, typeinfo>, EVEX_B4V;
}
let Constraints = "$src1 = $dst", Predicates = [In64BitMode] in
  def _NF : shift_rCL_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_NF;
let Predicates = [HasNDD, In64BitMode] in
  def _ND_NF : shift_rCL_NDD_NoPat<opcode, f, mnemonic, typeinfo>,
               EVEX_B4V, EVEX_NF;
}

class shift_ri<bits<8> opcode, Format f, string mnemonic, SDNode node,
               X86TypeInfo typeinfo>
  : I_R_RI<opcode, f, typeinfo, mnemonic, "{$src2, $dst|$dst, $src2}",
           [(set typeinfo.RegClass:$dst,
            (node typeinfo.RegClass:$src1, (i8 imm:$src2)))]> {
  let ImmT = Imm8;
}

class shift_ri_NDD<bits<8> opcode, Format f, string mnemonic, SDNode node,
                   X86TypeInfo typeinfo>
  : I_R_RI_EVEX<opcode, f, typeinfo, mnemonic,
                "{$src2, $src1, $dst|$dst, $src1, $src2}",
                [(set typeinfo.RegClass:$dst,
                 (node typeinfo.RegClass:$src1, (i8 imm:$src2)))]> {
  let ImmT = Imm8;
}

class shift_ri_NoPat<bits<8> opcode, Format f, string mnemonic,
                     X86TypeInfo typeinfo>
  : I_R_RI_EVEX<opcode, f, typeinfo, mnemonic, "{$src2, $dst|$dst, $src2}", []> {
  let ImmT = Imm8;
}

class shift_ri_NDD_NoPat<bits<8> opcode, Format f, string mnemonic,
                         X86TypeInfo typeinfo>
  : I_R_RI_EVEX<opcode, f, typeinfo, mnemonic,
                "{$src2, $src1, $dst|$dst, $src1, $src2}", []> {
  let ImmT = Imm8;
}

multiclass mshift_ri<bits<8> opcode, Format f, string mnemonic, SDNode node,
                      X86TypeInfo typeinfo> {
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst" in {
let Predicates = [NoNDD] in
  def NAME : shift_ri<opcode, f, mnemonic, node, typeinfo>;
let Predicates = [In64BitMode] in
  def _EVEX : shift_ri_NoPat<opcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
}
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shift_ri_NDD<opcode, f, mnemonic, node, typeinfo>,
            EVEX_B4V;
}
let Constraints = "$src1 = $dst", Predicates = [In64BitMode] in
  def _NF : shift_ri_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_NF;
let Predicates = [HasNDD, In64BitMode] in
  def _ND_NF : shift_ri_NDD_NoPat<opcode, f, mnemonic, typeinfo>,
               EVEX_B4V, EVEX_NF;
}

class shiftl_r1<bits<8> opcode, Format f, X86TypeInfo typeinfo>
  : I_R_R_EVEX<opcode, f, typeinfo, "shl", "$dst", []>;

class shiftl_r1_NDD<bits<8> opcode, Format f, X86TypeInfo typeinfo>
  : I_R_R_EVEX<opcode, f, typeinfo, "shl", "{$src1, $dst|$dst, $src1}", []>;

multiclass mshiftl_r1<bits<8> opcode, Format f, X86TypeInfo typeinfo> {
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst" in {
  def NAME: I_R_R<opcode, f, typeinfo, "shl", "$dst", []>;
let Predicates = [In64BitMode] in
  def _EVEX : shiftl_r1<opcode, f, typeinfo>, EmitVEXOrEVEXPrefix;
}
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shiftl_r1_NDD<opcode, f, typeinfo>, EVEX_B4V;
}
let Constraints = "$src1 = $dst", Predicates = [In64BitMode] in
  def _NF : shiftl_r1<opcode, f, typeinfo>, EVEX_NF;
let Predicates = [HasNDD, In64BitMode] in
  def _ND_NF : shiftl_r1_NDD<opcode, f, typeinfo>, EVEX_B4V, EVEX_NF;
}

let hasSideEffects = 0 in {
let Uses = [CL], SchedRW = [WriteShiftCL] in {
defm SHL8rCL  : mshift_rCL<0xD2, MRM4r, "shl", shl, Xi8>;
defm SHL16rCL : mshift_rCL<0xD3, MRM4r, "shl", shl, Xi16>;
defm SHL32rCL : mshift_rCL<0xD3, MRM4r, "shl", shl, Xi32>;
defm SHL64rCL : mshift_rCL<0xD3, MRM4r, "shl", shl, Xi64>;
}

let SchedRW = [WriteShift], isConvertibleToThreeAddress = 1 in {
defm SHL8ri   : mshift_ri<0xC0, MRM4r, "shl", shl, Xi8>;
defm SHL16ri  : mshift_ri<0xC1, MRM4r, "shl", shl, Xi16>;
defm SHL32ri  : mshift_ri<0xC1, MRM4r, "shl", shl, Xi32>;
defm SHL64ri  : mshift_ri<0xC1, MRM4r, "shl", shl, Xi64>;
}

let SchedRW = [WriteShift] in {
defm SHL8r1   : mshiftl_r1<0xD0, MRM4r, Xi8>;
defm SHL16r1  : mshiftl_r1<0xD1, MRM4r, Xi16>;
defm SHL32r1  : mshiftl_r1<0xD1, MRM4r, Xi32>;
defm SHL64r1  : mshiftl_r1<0xD1, MRM4r, Xi64>;
}
}

// FIXME: Why do we need an explicit "Uses = [CL]" when the instr has a pattern
// using CL?

class shift_mCL<bits<8> opcode, Format f, string mnemonic, SDNode node,
                X86TypeInfo typeinfo>
  : I_N_M<opcode, f, typeinfo, mnemonic, "{%cl, $dst|$dst, cl}",
          [(store (node (typeinfo.LoadNode addr:$dst), CL), addr:$dst)]>;

class shift_mCL_NDD<bits<8> opcode, Format f, string mnemonic, SDNode node,
                    X86TypeInfo typeinfo>
  : I_R_M_EVEX<opcode, f, typeinfo, mnemonic,
               "{%cl, $src1, $dst|$dst, $src1, cl}",
               [(set typeinfo.RegClass:$dst,
                (node (typeinfo.LoadNode addr:$src1), CL))]>;

class shift_mCL_NoPat<bits<8> opcode, Format f, string mnemonic,
                      X86TypeInfo typeinfo>
  : I_N_M_EVEX<opcode, f, typeinfo, mnemonic, "{%cl, $dst|$dst, cl}", []>;

class shift_mCL_NDD_NoPat<bits<8> opcode, Format f, string mnemonic,
                          X86TypeInfo typeinfo>
  : I_R_M_EVEX<opcode, f, typeinfo, mnemonic,
               "{%cl, $src1, $dst|$dst, $src1, cl}", []>;

multiclass mshift_mCL<bits<8> opcode, Format f, string mnemonic, SDNode node,
                      X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Predicates = !if(typeinfo.HasREX_W, [NoNDD, In64BitMode],
                                        [NoNDD]) in
  def NAME : shift_mCL<opcode, f, mnemonic, node, typeinfo>;
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _EVEX : shift_mCL_NoPat<opcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shift_mCL_NDD<opcode, f, mnemonic, node, typeinfo>, EVEX_B4V;
}
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _NF : shift_mCL_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_NF;
let Predicates = [HasNDD, In64BitMode], mayLoad = 1 in
  def _ND_NF : shift_mCL_NDD_NoPat<opcode, f, mnemonic, typeinfo>,
               EVEX_B4V, EVEX_NF;
}

let Uses = [CL], SchedRW = [WriteShiftCLLd, WriteRMW], hasSideEffects = 0 in {
defm SHL8mCL  : mshift_mCL<0xD2, MRM4m, "shl", shl, Xi8>;
defm SHL16mCL : mshift_mCL<0xD3, MRM4m, "shl", shl, Xi16>;
defm SHL32mCL : mshift_mCL<0xD3, MRM4m, "shl", shl, Xi32>;
defm SHL64mCL : mshift_mCL<0xD3, MRM4m, "shl", shl, Xi64>;
}

class shift_mi<bits<8> opcode, Format f, string mnemonic, SDNode node,
               X86TypeInfo typeinfo>
  : I_N_MI<opcode, f, typeinfo, mnemonic, "{$src, $dst|$dst, $src}",
           [(store (node (typeinfo.LoadNode addr:$dst),
                            (i8 imm:$src)), addr:$dst)]>{
  let ImmT = Imm8;
}

class shift_mi_NDD<bits<8> opcode, Format f, string mnemonic, SDNode node,
                   X86TypeInfo typeinfo>
  : I_R_MI_EVEX<opcode, f, typeinfo, mnemonic,
                "{$src2, $src1, $dst|$dst, $src1, $src2}",
                [(set typeinfo.RegClass:$dst,
                 (node (typeinfo.LoadNode addr:$src1), (i8 imm:$src2)))]>{
  let ImmT = Imm8;
}

class shift_mi_NoPat_legacy<bits<8> opcode, Format f, string mnemonic,
                            X86TypeInfo typeinfo>
  : I_N_MI<opcode, f, typeinfo, mnemonic, "{$src, $dst|$dst, $src}", []>{
  let ImmT = Imm8;
}

class shift_mi_NoPat<bits<8> opcode, Format f, string mnemonic,
                     X86TypeInfo typeinfo>
  : I_N_MI_EVEX<opcode, f, typeinfo, mnemonic, "{$src, $dst|$dst, $src}", []>{
  let ImmT = Imm8;
}

class shift_mi_NDD_NoPat<bits<8> opcode, Format f, string mnemonic,
                         X86TypeInfo typeinfo>
  : I_R_MI_EVEX<opcode, f, typeinfo, mnemonic,
                "{$src2, $src1, $dst|$dst, $src1, $src2}", []>{
  let ImmT = Imm8;
}

multiclass mshift_mi<bits<8> opcode, Format f, string mnemonic, SDNode node,
                     X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Predicates = !if(typeinfo.HasREX_W, [NoNDD, In64BitMode],
                                        [NoNDD]) in
  def NAME : shift_mi<opcode, f, mnemonic, node, typeinfo>;
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _EVEX : shift_mi_NoPat<opcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shift_mi_NDD<opcode, f, mnemonic, node, typeinfo>, EVEX_B4V;
}
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _NF : shift_mi_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_NF;
let Predicates = [HasNDD, In64BitMode], mayLoad = 1 in
  def _ND_NF : shift_mi_NDD_NoPat<opcode, f, mnemonic, typeinfo>,
               EVEX_B4V, EVEX_NF;
}

class shift_m1_NoPat<bits<8> opcode, Format f, string mnemonic,
                     X86TypeInfo typeinfo>
  : I_N_M_EVEX<opcode, f, typeinfo, mnemonic, "$dst", []>;

class shift_m1_NDD_NoPat<bits<8> opcode, Format f, string mnemonic,
                         X86TypeInfo typeinfo>
  : I_R_M_EVEX<opcode, f, typeinfo, mnemonic, "{$src1, $dst|$dst, $src1}", []>;

multiclass mshift_m1<bits<8> opcode, Format f, string mnemonic,
                     X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Predicates = !if(typeinfo.HasREX_W, [NoNDD, In64BitMode], [NoNDD]),
    mayLoad = 1, mayStore = 1 in
  def NAME : I_N_M<opcode, f, typeinfo, mnemonic, "$dst", []>;
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _EVEX : shift_m1_NoPat<opcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode], mayLoad = 1 in
  def _ND : shift_m1_NDD_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_B4V;
}
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _NF : shift_m1_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_NF;
let Predicates = [HasNDD, In64BitMode], mayLoad = 1 in
  def _ND_NF : shift_m1_NDD_NoPat<opcode, f, mnemonic, typeinfo>,
               EVEX_B4V, EVEX_NF;
}

let SchedRW = [WriteShiftLd, WriteRMW], hasSideEffects = 0 in {
defm SHL8mi   : mshift_mi<0xC0, MRM4m, "shl", shl, Xi8>;
defm SHL16mi  : mshift_mi<0xC1, MRM4m, "shl", shl, Xi16>;
defm SHL32mi  : mshift_mi<0xC1, MRM4m, "shl", shl, Xi32>;
defm SHL64mi  : mshift_mi<0xC1, MRM4m, "shl", shl, Xi64>;

defm SHL8m1   : mshift_m1<0xD0, MRM4m, "shl", Xi8>;
defm SHL16m1  : mshift_m1<0xD1, MRM4m, "shl", Xi16>;
defm SHL32m1  : mshift_m1<0xD1, MRM4m, "shl", Xi32>;
defm SHL64m1  : mshift_m1<0xD1, MRM4m, "shl", Xi64>;
}

//=======================SHR==========================
class shift_r1<bits<8> opcode, Format f, string mnemonic, X86TypeInfo typeinfo>
  : I_R_R<opcode, f, typeinfo, mnemonic, "$dst", []>;

class shift_r1_NoPat<bits<8> opcode, Format f, string mnemonic,
                     X86TypeInfo typeinfo>
  : I_R_R_EVEX<opcode, f, typeinfo, mnemonic, "$dst", []>;

class shift_r1_NDD_NoPat<bits<8> opcode, Format f, string mnemonic,
                         X86TypeInfo typeinfo>
  : I_R_R_EVEX<opcode, f, typeinfo, mnemonic, "{$src1, $dst|$dst, $src1}", []>;

multiclass mshift_r1<bits<8> opcode, Format f, string mnemonic,
                     X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst" in {
let Predicates = [NoNDD] in
  def NAME : shift_r1<opcode, f, mnemonic, typeinfo>;
let Predicates = [In64BitMode] in
  def _EVEX : shift_r1_NoPat<opcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
}
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shift_r1_NDD_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_B4V;
}
let Constraints = "$src1 = $dst", Predicates = [In64BitMode] in
  def _NF : shift_r1_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_NF;
let Predicates = [HasNDD, In64BitMode] in
  def _ND_NF : shift_r1_NDD_NoPat<opcode, f, mnemonic, typeinfo>,
               EVEX_B4V, EVEX_NF;
}

let hasSideEffects = 0 in {
let Uses = [CL], SchedRW = [WriteShiftCL] in {
defm SHR8rCL  : mshift_rCL<0xD2, MRM5r, "shr", srl, Xi8>;
defm SHR16rCL : mshift_rCL<0xD3, MRM5r, "shr", srl, Xi16>;
defm SHR32rCL : mshift_rCL<0xD3, MRM5r, "shr", srl, Xi32>;
defm SHR64rCL : mshift_rCL<0xD3, MRM5r, "shr", srl, Xi64>;
} // Uses = [CL], SchedRW

let SchedRW = [WriteShift] in {
defm SHR8ri   : mshift_ri<0xC0, MRM5r, "shr", srl, Xi8>;
defm SHR16ri  : mshift_ri<0xC1, MRM5r, "shr", srl, Xi16>;
defm SHR32ri  : mshift_ri<0xC1, MRM5r, "shr", srl, Xi32>;
defm SHR64ri  : mshift_ri<0xC1, MRM5r, "shr", srl, Xi64>;

defm SHR8r1   : mshift_r1<0xD0, MRM5r, "shr", Xi8>;
defm SHR16r1  : mshift_r1<0xD1, MRM5r, "shr", Xi16>;
defm SHR32r1  : mshift_r1<0xD1, MRM5r, "shr", Xi32>;
defm SHR64r1  : mshift_r1<0xD1, MRM5r, "shr", Xi64>;
} // SchedRW

let Uses = [CL], SchedRW = [WriteShiftCLLd, WriteRMW] in {
defm SHR8mCL  : mshift_mCL<0xD2, MRM5m, "shr", srl, Xi8>;
defm SHR16mCL : mshift_mCL<0xD3, MRM5m, "shr", srl, Xi16>;
defm SHR32mCL : mshift_mCL<0xD3, MRM5m, "shr", srl, Xi32>;
defm SHR64mCL : mshift_mCL<0xD3, MRM5m, "shr", srl, Xi64>;
}

let SchedRW = [WriteShiftLd, WriteRMW] in {
defm SHR8mi   : mshift_mi<0xC0, MRM5m, "shr", srl, Xi8>;
defm SHR16mi  : mshift_mi<0xC1, MRM5m, "shr", srl, Xi16>;
defm SHR32mi  : mshift_mi<0xC1, MRM5m, "shr", srl, Xi32>;
defm SHR64mi  : mshift_mi<0xC1, MRM5m, "shr", srl, Xi64>;

defm SHR8m1   : mshift_m1<0xD0, MRM5m, "shr", Xi8>;
defm SHR16m1  : mshift_m1<0xD1, MRM5m, "shr", Xi16>;
defm SHR32m1  : mshift_m1<0xD1, MRM5m, "shr", Xi32>;
defm SHR64m1  : mshift_m1<0xD1, MRM5m, "shr", Xi64>;
} // SchedRW

//====================SAR=======================
let Uses = [CL], SchedRW = [WriteShiftCL] in {
defm SAR8rCL  : mshift_rCL<0xD2, MRM7r, "sar", sra, Xi8>;
defm SAR16rCL : mshift_rCL<0xD3, MRM7r, "sar", sra, Xi16>;
defm SAR32rCL : mshift_rCL<0xD3, MRM7r, "sar", sra, Xi32>;
defm SAR64rCL : mshift_rCL<0xD3, MRM7r, "sar", sra, Xi64>;
} // Uses = [CL], SchedRW

let SchedRW = [WriteShift]in {
defm SAR8ri   : mshift_ri<0xC0, MRM7r, "sar", sra, Xi8>;
defm SAR16ri  : mshift_ri<0xC1, MRM7r, "sar", sra, Xi16>;
defm SAR32ri  : mshift_ri<0xC1, MRM7r, "sar", sra, Xi32>;
defm SAR64ri  : mshift_ri<0xC1, MRM7r, "sar", sra, Xi64>;
} // SchedRW

let SchedRW = [WriteShift] in {
defm SAR8r1   : mshift_r1<0xD0, MRM7r, "sar", Xi8>;
defm SAR16r1  : mshift_r1<0xD1, MRM7r, "sar", Xi16>;
defm SAR32r1  : mshift_r1<0xD1, MRM7r, "sar", Xi32>;
defm SAR64r1  : mshift_r1<0xD1, MRM7r, "sar", Xi64>;
}

let Uses = [CL], SchedRW = [WriteShiftCLLd, WriteRMW] in {
defm SAR8mCL  : mshift_mCL<0xD2, MRM7m, "sar", sra, Xi8>;
defm SAR16mCL : mshift_mCL<0xD3, MRM7m, "sar", sra, Xi16>;
defm SAR32mCL : mshift_mCL<0xD3, MRM7m, "sar", sra, Xi32>;
defm SAR64mCL : mshift_mCL<0xD3, MRM7m, "sar", sra, Xi64>;
}

let SchedRW = [WriteShiftLd, WriteRMW] in {
defm SAR8mi   : mshift_mi<0xC0, MRM7m, "sar", sra, Xi8>;
defm SAR16mi  : mshift_mi<0xC1, MRM7m, "sar", sra, Xi16>;
defm SAR32mi  : mshift_mi<0xC1, MRM7m, "sar", sra, Xi32>;
defm SAR64mi  : mshift_mi<0xC1, MRM7m, "sar", sra, Xi64>;

// Shift by 1
defm SAR8m1   : mshift_m1<0xD0, MRM7m, "sar", Xi8>;
defm SAR16m1  : mshift_m1<0xD1, MRM7m, "sar", Xi16>;
defm SAR32m1  : mshift_m1<0xD1, MRM7m, "sar", Xi32>;
defm SAR64m1  : mshift_m1<0xD1, MRM7m, "sar", Xi64>;
} // SchedRW
}

//===----------------------------------------------------------------------===//
// Rotate instructions
//===----------------------------------------------------------------------===//

multiclass mshift_rCL_NoNF<bits<8> opcode, Format f, string mnemonic,
                           X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst" in {
let Predicates = [NoNDD] in
  def NAME : I_R_R<opcode, f, typeinfo, mnemonic, "{%cl, $dst|$dst, cl}", []>;
let Predicates = [In64BitMode] in
  def _EVEX : shift_rCL_NoPat<opcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
}
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shift_rCL_NDD_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_B4V;
}
}

multiclass mshift_ri_NoNF<bits<8> opcode, Format f, string mnemonic,
                          X86TypeInfo typeinfo>{
let ImmT = Imm8 in {
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst" in {
let Predicates = [NoNDD] in
  def NAME : I_R_RI<opcode, f, typeinfo, mnemonic,
                    "{$src2, $dst|$dst, $src2}", []>;
let Predicates = [In64BitMode] in
  def _EVEX : shift_ri_NoPat<opcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
}
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shift_ri_NDD_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_B4V;
}
}
}

multiclass mshift_r1_NoNF<bits<8> opcode, Format f, string mnemonic,
                          X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst" in {
let Predicates = [NoNDD] in
  def NAME : I_R_R<opcode, f, typeinfo, mnemonic, "$dst", []>;
let Predicates = [In64BitMode] in
  def _EVEX : shift_r1_NoPat<opcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
}
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shift_r1_NDD_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_B4V;
}
}

multiclass mshift_mCL_NoNF<bits<8> opcode, Format f, string mnemonic,
                           X86TypeInfo typeinfo>{
let Defs = [EFLAGS], mayLoad = 1 in {
let Predicates = !if(typeinfo.HasREX_W,
                     [NoNDD, In64BitMode], [NoNDD]), mayStore = 1 in
  def NAME : I_N_M<opcode, f, typeinfo, mnemonic, "{%cl, $dst|$dst, cl}", []>;
let Predicates = [In64BitMode], mayStore = 1 in
  def _EVEX : shift_mCL_NoPat<opcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shift_mCL_NDD_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_B4V;
}
}

multiclass mshift_mi_NoNF<bits<8> opcode, Format f, string mnemonic,
                          X86TypeInfo typeinfo>{
let Defs = [EFLAGS], mayLoad = 1 in {
let Predicates = !if(typeinfo.HasREX_W,
                     [NoNDD, In64BitMode], [NoNDD]), mayStore = 1 in
  def NAME : shift_mi_NoPat_legacy<opcode, f, mnemonic, typeinfo>;
let Predicates = [In64BitMode], mayStore = 1 in
  def _EVEX : shift_mi_NoPat<opcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shift_mi_NDD_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_B4V;
}
}

multiclass mshift_m1_NoNF<bits<8> opcode, Format f, string mnemonic,
                          X86TypeInfo typeinfo>{
let Defs = [EFLAGS], mayLoad = 1 in {
let Predicates = !if(typeinfo.HasREX_W,
                     [NoNDD, In64BitMode], [NoNDD]), mayStore = 1 in
  def NAME : I_N_M<opcode, f, typeinfo, mnemonic, "$dst", []>;
let Predicates = [In64BitMode], mayStore = 1 in
  def _EVEX : shift_m1_NoPat<opcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shift_m1_NDD_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_B4V;
}
}

let hasSideEffects = 0 in {
let Uses = [CL, EFLAGS], SchedRW = [WriteRotateCL] in {
defm RCL8rCL  : mshift_rCL_NoNF<0xD2, MRM2r, "rcl", Xi8>;
defm RCL16rCL : mshift_rCL_NoNF<0xD3, MRM2r, "rcl", Xi16>;
defm RCL32rCL : mshift_rCL_NoNF<0xD3, MRM2r, "rcl", Xi32>;
defm RCL64rCL : mshift_rCL_NoNF<0xD3, MRM2r, "rcl", Xi64>;
}

let Uses = [EFLAGS], SchedRW = [WriteRotate] in {
defm RCL8ri   : mshift_ri_NoNF<0xC0, MRM2r, "rcl", Xi8>;
defm RCL16ri  : mshift_ri_NoNF<0xC1, MRM2r, "rcl", Xi16>;
defm RCL32ri  : mshift_ri_NoNF<0xC1, MRM2r, "rcl", Xi32>;
defm RCL64ri  : mshift_ri_NoNF<0xC1, MRM2r, "rcl", Xi64>;
}

let Uses = [EFLAGS], SchedRW = [WriteRotate] in {
defm RCL8r1   : mshift_r1_NoNF<0xD0, MRM2r, "rcl", Xi8>;
defm RCL16r1  : mshift_r1_NoNF<0xD1, MRM2r, "rcl", Xi16>;
defm RCL32r1  : mshift_r1_NoNF<0xD1, MRM2r, "rcl", Xi32>;
defm RCL64r1  : mshift_r1_NoNF<0xD1, MRM2r, "rcl", Xi64>;
}

let Uses = [CL, EFLAGS], SchedRW = [WriteRotateCLLd, WriteRMW] in {
defm RCL8mCL  : mshift_mCL_NoNF<0xD2, MRM2m, "rcl", Xi8>;
defm RCL16mCL : mshift_mCL_NoNF<0xD3, MRM2m, "rcl", Xi16>;
defm RCL32mCL : mshift_mCL_NoNF<0xD3, MRM2m, "rcl", Xi32>;
defm RCL64mCL : mshift_mCL_NoNF<0xD3, MRM2m, "rcl", Xi64>;
}

let Uses = [EFLAGS], SchedRW = [WriteRotateLd, WriteRMW] in {
defm RCL8mi   : mshift_mi_NoNF<0xC0, MRM2m, "rcl", Xi8>;
defm RCL16mi  : mshift_mi_NoNF<0xC1, MRM2m, "rcl", Xi16>;
defm RCL32mi  : mshift_mi_NoNF<0xC1, MRM2m, "rcl", Xi32>;
defm RCL64mi  : mshift_mi_NoNF<0xC1, MRM2m, "rcl", Xi64>;

// Shift by 1
defm RCL8m1   : mshift_m1_NoNF<0xD0, MRM2m, "rcl", Xi8>;
defm RCL16m1  : mshift_m1_NoNF<0xD1, MRM2m, "rcl", Xi16>;
defm RCL32m1  : mshift_m1_NoNF<0xD1, MRM2m, "rcl", Xi32>;
defm RCL64m1  : mshift_m1_NoNF<0xD1, MRM2m, "rcl", Xi64>;
} // SchedRW

// =====================RCR=========================

let Uses = [CL, EFLAGS], SchedRW = [WriteRotateCL] in {
defm RCR8rCL  : mshift_rCL_NoNF<0xD2, MRM3r, "rcr", Xi8>;
defm RCR16rCL : mshift_rCL_NoNF<0xD3, MRM3r, "rcr", Xi16>;
defm RCR32rCL : mshift_rCL_NoNF<0xD3, MRM3r, "rcr", Xi32>;
defm RCR64rCL : mshift_rCL_NoNF<0xD3, MRM3r, "rcr", Xi64>;
} // Uses = [CL, EFLAGS], SchedRW

let Uses = [EFLAGS], SchedRW = [WriteRotate]in {
defm RCR8ri   : mshift_ri_NoNF<0xC0, MRM3r, "rcr", Xi8>;
defm RCR16ri  : mshift_ri_NoNF<0xC1, MRM3r, "rcr", Xi16>;
defm RCR32ri  : mshift_ri_NoNF<0xC1, MRM3r, "rcr", Xi32>;
defm RCR64ri  : mshift_ri_NoNF<0xC1, MRM3r, "rcr", Xi64>;
}

let Uses = [EFLAGS], SchedRW = [WriteRotate] in {
defm RCR8r1   : mshift_r1_NoNF<0xD0, MRM3r, "rcr", Xi8>;
defm RCR16r1  : mshift_r1_NoNF<0xD1, MRM3r, "rcr", Xi16>;
defm RCR32r1  : mshift_r1_NoNF<0xD1, MRM3r, "rcr", Xi32>;
defm RCR64r1  : mshift_r1_NoNF<0xD1, MRM3r, "rcr", Xi64>;
}

let Uses = [CL, EFLAGS], SchedRW = [WriteRotateCLLd, WriteRMW] in {
defm RCR8mCL  : mshift_mCL_NoNF<0xD2, MRM3m, "rcr", Xi8>;
defm RCR16mCL : mshift_mCL_NoNF<0xD3, MRM3m, "rcr", Xi16>;
defm RCR32mCL : mshift_mCL_NoNF<0xD3, MRM3m, "rcr", Xi32>;
defm RCR64mCL : mshift_mCL_NoNF<0xD3, MRM3m, "rcr", Xi64>;
}

let Uses = [EFLAGS], SchedRW = [WriteRotateLd, WriteRMW] in {
defm RCR8mi   : mshift_mi_NoNF<0xC0, MRM3m, "rcr", Xi8>;
defm RCR16mi  : mshift_mi_NoNF<0xC1, MRM3m, "rcr", Xi16>;
defm RCR32mi  : mshift_mi_NoNF<0xC1, MRM3m, "rcr", Xi32>;
defm RCR64mi  : mshift_mi_NoNF<0xC1, MRM3m, "rcr", Xi64>;

defm RCR8m1   : mshift_m1_NoNF<0xD0, MRM3m, "rcr", Xi8>;
defm RCR16m1  : mshift_m1_NoNF<0xD1, MRM3m, "rcr", Xi16>;
defm RCR32m1  : mshift_m1_NoNF<0xD1, MRM3m, "rcr", Xi32>;
defm RCR64m1  : mshift_m1_NoNF<0xD1, MRM3m, "rcr", Xi64>;
}
} // hasSideEffects = 0

// ================== ROL =======================
let Uses = [CL], SchedRW = [WriteRotateCL] in {
defm ROL8rCL  : mshift_rCL<0xD2, MRM0r, "rol", rotl, Xi8>;
defm ROL16rCL : mshift_rCL<0xD3, MRM0r, "rol", rotl, Xi16>;
defm ROL32rCL : mshift_rCL<0xD3, MRM0r, "rol", rotl, Xi32>;
defm ROL64rCL : mshift_rCL<0xD3, MRM0r, "rol", rotl, Xi64>;
}

let SchedRW = [WriteRotate]in {
defm ROL8ri   : mshift_ri<0xC0, MRM0r, "rol", rotl, Xi8>;
defm ROL16ri  : mshift_ri<0xC1, MRM0r, "rol", rotl, Xi16>;
defm ROL32ri  : mshift_ri<0xC1, MRM0r, "rol", rotl, Xi32>;
defm ROL64ri  : mshift_ri<0xC1, MRM0r, "rol", rotl, Xi64>;
}

let SchedRW = [WriteRotate] in {
defm ROL8r1   : mshift_r1<0xD0, MRM0r, "rol", Xi8>;
defm ROL16r1  : mshift_r1<0xD1, MRM0r, "rol", Xi16>;
defm ROL32r1  : mshift_r1<0xD1, MRM0r, "rol", Xi32>;
defm ROL64r1  : mshift_r1<0xD1, MRM0r, "rol", Xi64>;
}

let Uses = [CL], SchedRW = [WriteRotateCLLd, WriteRMW] in {
defm ROL8mCL  : mshift_mCL<0xD2, MRM0m, "rol", rotl, Xi8>;
defm ROL16mCL : mshift_mCL<0xD3, MRM0m, "rol", rotl, Xi16>;
defm ROL32mCL : mshift_mCL<0xD3, MRM0m, "rol", rotl, Xi32>;
defm ROL64mCL : mshift_mCL<0xD3, MRM0m, "rol", rotl, Xi64>;
}

let SchedRW = [WriteRotateLd, WriteRMW] in {
defm ROL8mi   : mshift_mi<0xC0, MRM0m, "rol", rotl, Xi8>;
defm ROL16mi  : mshift_mi<0xC1, MRM0m, "rol", rotl, Xi16>;
defm ROL32mi  : mshift_mi<0xC1, MRM0m, "rol", rotl, Xi32>;
defm ROL64mi  : mshift_mi<0xC1, MRM0m, "rol", rotl, Xi64>;

defm ROL8m1   : mshift_m1<0xD0, MRM0m, "rol", Xi8>;
defm ROL16m1  : mshift_m1<0xD1, MRM0m, "rol", Xi16>;
defm ROL32m1  : mshift_m1<0xD1, MRM0m, "rol", Xi32>;
defm ROL64m1  : mshift_m1<0xD1, MRM0m, "rol", Xi64>;
}

// ================== ROR =======================
let Uses = [CL], SchedRW = [WriteRotateCL] in {
defm ROR8rCL  : mshift_rCL<0xD2, MRM1r, "ror", rotr, Xi8>;
defm ROR16rCL : mshift_rCL<0xD3, MRM1r, "ror", rotr, Xi16>;
defm ROR32rCL : mshift_rCL<0xD3, MRM1r, "ror", rotr, Xi32>;
defm ROR64rCL : mshift_rCL<0xD3, MRM1r, "ror", rotr, Xi64>;
}

let SchedRW = [WriteRotate]in {
defm ROR8ri   : mshift_ri<0xC0, MRM1r, "ror", rotr, Xi8>;
defm ROR16ri  : mshift_ri<0xC1, MRM1r, "ror", rotr, Xi16>;
defm ROR32ri  : mshift_ri<0xC1, MRM1r, "ror", rotr, Xi32>;
defm ROR64ri  : mshift_ri<0xC1, MRM1r, "ror", rotr, Xi64>;
}

let SchedRW = [WriteRotate] in {
defm ROR8r1   : mshift_r1<0xD0, MRM1r, "ror", Xi8>;
defm ROR16r1  : mshift_r1<0xD1, MRM1r, "ror", Xi16>;
defm ROR32r1  : mshift_r1<0xD1, MRM1r, "ror", Xi32>;
defm ROR64r1  : mshift_r1<0xD1, MRM1r, "ror", Xi64>;
}

let Uses = [CL], SchedRW = [WriteRotateCLLd, WriteRMW] in {
defm ROR8mCL  : mshift_mCL<0xD2, MRM1m, "ror", rotr, Xi8>;
defm ROR16mCL : mshift_mCL<0xD3, MRM1m, "ror", rotr, Xi16>;
defm ROR32mCL : mshift_mCL<0xD3, MRM1m, "ror", rotr, Xi32>;
defm ROR64mCL : mshift_mCL<0xD3, MRM1m, "ror", rotr, Xi64>;
}

let SchedRW = [WriteRotateLd, WriteRMW] in {
defm ROR8mi   : mshift_mi<0xC0, MRM1m, "ror", rotr, Xi8>;
defm ROR16mi  : mshift_mi<0xC1, MRM1m, "ror", rotr, Xi16>;
defm ROR32mi  : mshift_mi<0xC1, MRM1m, "ror", rotr, Xi32>;
defm ROR64mi  : mshift_mi<0xC1, MRM1m, "ror", rotr, Xi64>;

defm ROR8m1   : mshift_m1<0xD0, MRM1m, "ror", Xi8>;
defm ROR16m1  : mshift_m1<0xD1, MRM1m, "ror", Xi16>;
defm ROR32m1  : mshift_m1<0xD1, MRM1m, "ror", Xi32>;
defm ROR64m1  : mshift_m1<0xD1, MRM1m, "ror", Xi64>;
}

//===----------------------------------------------------------------------===//
// Double shift instructions (generalizations of rotate)
//===----------------------------------------------------------------------===//
class shiftLD_rrCL<bits<8> opcode, Format f, string mnemonic, SDNode node,
                   X86TypeInfo typeinfo>
  : I_R_RR<opcode, f, typeinfo, mnemonic,
           "{%cl, $src2, $dst|$dst, $src2, cl}",
           [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1,
                                         typeinfo.RegClass:$src2, CL))]>,
    TB{
  let Opcode = opcode;
}

class shiftLD_rrCL_NDD<bits<8> opcode, Format f, string mnemonic, SDNode node,
                       X86TypeInfo typeinfo>
  : I_R_RR_EVEX<opcode, f, typeinfo, mnemonic,
                "{%cl, $src2, $src1, $dst|$dst, $src1, $src2, cl}",
                [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1,
                                              typeinfo.RegClass:$src2, CL))]>{
  let Opcode = opcode;
}

class shiftLD_rrCL_NoPat<bits<8> opcode, Format f, string mnemonic,
                         X86TypeInfo typeinfo>
  : I_R_RR_EVEX<opcode, f, typeinfo, mnemonic,
                "{%cl, $src2, $dst|$dst, $src2, cl}", []>{
  let Opcode = opcode;
}

class shiftLD_rrCL_NDD_NoPat<bits<8> opcode, Format f, string mnemonic,
                             X86TypeInfo typeinfo>
  : I_R_RR_EVEX<opcode, f, typeinfo, mnemonic,
                "{%cl, $src2, $src1, $dst|$dst, $src1, $src2, cl}", []>{
  let Opcode = opcode;
}

multiclass mshiftLD_rrCL<bits<8> opcode, Format f, string mnemonic,
                         SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst" in {
let Predicates = [NoNDD] in
  def NAME : shiftLD_rrCL<opcode, f, mnemonic, node, typeinfo>;
let Predicates = [In64BitMode] in
  def _EVEX : shiftLD_rrCL_NoPat<opcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
}
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shiftLD_rrCL_NDD<opcode, f, mnemonic, node, typeinfo>, EVEX_B4V;
}
let Constraints = "$src1 = $dst", Predicates = [In64BitMode] in
  def _NF : shiftLD_rrCL_NoPat<opcode, f, mnemonic, typeinfo>,
            EVEX_NF;
let Predicates = [HasNDD, In64BitMode] in
  def _ND_NF : shiftLD_rrCL_NDD_NoPat<opcode, f, mnemonic, typeinfo>,
               EVEX_B4V, EVEX_NF;
}

class shiftRD_rrCL<bits<8> opcode, Format f, string mnemonic, SDNode node,
                   X86TypeInfo typeinfo>
  : I_R_RR<opcode, f, typeinfo, mnemonic,
           "{%cl, $src2, $dst|$dst, $src2, cl}",
           [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src2,
                                            typeinfo.RegClass:$src1, CL))]>,
    TB{
  let Opcode = opcode;
}

class shiftRD_rrCL_NDD<bits<8> opcode, Format f, string mnemonic, SDNode node,
                       X86TypeInfo typeinfo>
  : I_R_RR_EVEX<opcode, f, typeinfo, mnemonic,
                "{%cl, $src2, $src1, $dst|$dst, $src1, $src2, cl}",
                [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src2,
                                              typeinfo.RegClass:$src1, CL))]>{
  let Opcode = opcode;
}

class shiftRD_rrCL_NoPat<bits<8> opcode, Format f, string mnemonic,
                         X86TypeInfo typeinfo>
  : I_R_RR_EVEX<opcode, f, typeinfo, mnemonic,
                "{%cl, $src2, $dst|$dst, $src2, cl}", []>{
  let Opcode = opcode;
}

class shiftRD_rrCL_NDD_NoPat<bits<8> opcode, Format f, string mnemonic,
                            X86TypeInfo typeinfo>
  : I_R_RR_EVEX<opcode, f, typeinfo, mnemonic,
                "{%cl, $src2, $src1, $dst|$dst, $src1, $src2, cl}", []>{
  let Opcode = opcode;
}

multiclass mshiftRD_rrCL<bits<8> opcode, Format f, string mnemonic,
                         SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst" in {
let Predicates = [NoNDD] in
  def NAME : shiftRD_rrCL<opcode, f, mnemonic, node, typeinfo>;
let Predicates = [In64BitMode] in
  def _EVEX : shiftRD_rrCL_NoPat<opcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
}
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shiftRD_rrCL_NDD<opcode, f, mnemonic, node, typeinfo>, EVEX_B4V;
}
let Constraints = "$src1 = $dst", Predicates = [In64BitMode] in
  def _NF : shiftRD_rrCL_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_NF;
let Predicates = [HasNDD, In64BitMode] in
  def _ND_NF : shiftRD_rrCL_NDD_NoPat<opcode, f, mnemonic, typeinfo>,
               EVEX_B4V, EVEX_NF;
}

let Uses = [CL], SchedRW = [WriteSHDrrcl] in {
defm SHLD16rrCL : mshiftLD_rrCL<0xA5, MRMDestReg, "shld", X86fshl, Xi16>;
defm SHLD32rrCL : mshiftLD_rrCL<0xA5, MRMDestReg, "shld", fshl, Xi32>;
defm SHLD64rrCL : mshiftLD_rrCL<0xA5, MRMDestReg, "shld", fshl, Xi64>;

defm SHRD16rrCL : mshiftRD_rrCL<0xAD, MRMDestReg, "shrd", X86fshr, Xi16>;
defm SHRD32rrCL : mshiftRD_rrCL<0xAD, MRMDestReg, "shrd", fshr, Xi32>;
defm SHRD64rrCL : mshiftRD_rrCL<0xAD, MRMDestReg, "shrd", fshr, Xi64>;
}

class shiftLD_rri8<bits<8> opcode, Format f, string mnemonic, SDNode node,
                   X86TypeInfo typeinfo>
  : I_R_RRI<opcode, f, typeinfo, mnemonic,
            "{$src3, $src2, $dst|$dst, $src2, $src3}",
            [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1,
                                typeinfo.RegClass:$src2, (i8 imm:$src3)))]>,
    TB{
  let Opcode = opcode;
  let ImmT = Imm8;
}

class shiftLD_rri8_NDD<bits<8> opcode, Format f, string mnemonic, SDNode node,
                       X86TypeInfo typeinfo>
  : I_R_RRI_EVEX<opcode, f, typeinfo, mnemonic,
                 "{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1,
                                typeinfo.RegClass:$src2, (i8 imm:$src3)))]>{
  let Opcode = opcode;
  let ImmT = Imm8;
}

class shiftLD_rri8_NoPat<bits<8> opcode, Format f, string mnemonic,
                         X86TypeInfo typeinfo>
  : I_R_RRI_EVEX<opcode, f, typeinfo, mnemonic,
                 "{$src3, $src2, $dst|$dst, $src2, $src3}", []>{
  let Opcode = opcode;
  let ImmT = Imm8;
}

class shiftLD_rri8_NDD_NoPat<bits<8> opcode, Format f, string mnemonic,
                             X86TypeInfo typeinfo>
  : I_R_RRI_EVEX<opcode, f, typeinfo, mnemonic,
                 "{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}", []>{
  let Opcode = opcode;
  let ImmT = Imm8;
}

multiclass mshiftLD_rri8<bits<8> opcode, bits<8> newcode, Format f,
                         string mnemonic, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst" in {
let Predicates = [NoNDD] in
  def NAME : shiftLD_rri8<opcode, f, mnemonic, node, typeinfo>;
let Predicates = [In64BitMode] in
  def _EVEX : shiftLD_rri8_NoPat<newcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
}
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shiftLD_rri8_NDD<newcode, f, mnemonic, node, typeinfo>, EVEX_B4V;
}
let Constraints = "$src1 = $dst", Predicates = [In64BitMode] in
  def _NF : shiftLD_rri8_NoPat<newcode, f, mnemonic, typeinfo>, EVEX_NF;
let Predicates = [HasNDD, In64BitMode] in
  def _ND_NF : shiftLD_rri8_NDD_NoPat<newcode, f, mnemonic, typeinfo>,
               EVEX_B4V, EVEX_NF;
}

class shiftRD_rri8<bits<8> opcode, Format f, string mnemonic, SDNode node,
                   X86TypeInfo typeinfo>
  : I_R_RRI<opcode, f, typeinfo, mnemonic,
            "{$src3, $src2, $dst|$dst, $src2, $src3}",
            [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src2,
                                typeinfo.RegClass:$src1, (i8 imm:$src3)))]>,
    TB{
  let Opcode = opcode;
  let ImmT = Imm8;
}

class shiftRD_rri8_NDD<bits<8> opcode, Format f, string mnemonic, SDNode node,
                       X86TypeInfo typeinfo>
  : I_R_RRI_EVEX<opcode, f, typeinfo, mnemonic,
                 "{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src2,
                                typeinfo.RegClass:$src1, (i8 imm:$src3)))]>{
  let Opcode = opcode;
  let ImmT = Imm8;
}

class shiftRD_rri8_NoPat<bits<8> opcode, Format f, string mnemonic,
                         X86TypeInfo typeinfo>
  : I_R_RRI_EVEX<opcode, f, typeinfo, mnemonic,
                 "{$src3, $src2, $dst|$dst, $src2, $src3}", []>{
  let Opcode = opcode;
  let ImmT = Imm8;
}

class shiftRD_rri8_NDD_NoPat<bits<8> opcode, Format f, string mnemonic,
                             X86TypeInfo typeinfo>
  : I_R_RRI_EVEX<opcode, f, typeinfo, mnemonic,
                 "{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}", []>{
  let Opcode = opcode;
  let ImmT = Imm8;
}

multiclass mshiftRD_rri8<bits<8> opcode, bits<8> newcode, Format f,
                         string mnemonic, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst" in {
let Predicates = [NoNDD] in
  def NAME : shiftRD_rri8<opcode, f, mnemonic, node, typeinfo>;
let Predicates = [In64BitMode] in
  def _EVEX : shiftRD_rri8_NoPat<newcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
}
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shiftRD_rri8_NDD<newcode, f, mnemonic, node, typeinfo>, EVEX_B4V;
}
let Constraints = "$src1 = $dst", Predicates = [In64BitMode] in
  def _NF : shiftRD_rri8_NoPat<newcode, f, mnemonic, typeinfo>, EVEX_NF;
let Predicates = [HasNDD, In64BitMode] in
  def _ND_NF : shiftRD_rri8_NDD_NoPat<newcode, f, mnemonic, typeinfo>,
               EVEX_B4V, EVEX_NF;
}

let isCommutable = 1, SchedRW = [WriteSHDrri] in {
defm SHLD16rri8 : mshiftLD_rri8<0xA4, 0x24, MRMDestReg, "shld", X86fshl, Xi16>;
defm SHLD32rri8 : mshiftLD_rri8<0xA4, 0x24, MRMDestReg, "shld", fshl, Xi32>;
defm SHLD64rri8 : mshiftLD_rri8<0xA4, 0x24, MRMDestReg, "shld", fshl, Xi64>;

defm SHRD16rri8 : mshiftRD_rri8<0xAC, 0x2C, MRMDestReg, "shrd", X86fshr, Xi16>;
defm SHRD32rri8 : mshiftRD_rri8<0xAC, 0x2C, MRMDestReg, "shrd", fshr, Xi32>;
defm SHRD64rri8 : mshiftRD_rri8<0xAC, 0x2C, MRMDestReg, "shrd", fshr, Xi64>;
}

class shiftLD_mrCL<bits<8> opcode, Format f, string mnemonic, SDNode node,
                   X86TypeInfo typeinfo>
  : I_N_MR<opcode, f, typeinfo, mnemonic,
           "{%cl, $src2, $dst|$dst, $src2, cl}",
           [(store (node (typeinfo.LoadNode addr:$dst),
                    typeinfo.RegClass:$src2, CL), addr:$dst)]>,
    TB{
  let Opcode = opcode;
}

class shiftLD_mrCL_NDD<bits<8> opcode, Format f, string mnemonic, SDNode node,
                       X86TypeInfo typeinfo>
  : I_R_MR_EVEX<opcode, f, typeinfo, mnemonic,
                "{%cl, $src2, $src1, $dst|$dst, $src1, $src2, cl}",
                [(set typeinfo.RegClass:$dst,
                 (node (typeinfo.LoadNode addr:$src1),
                        typeinfo.RegClass:$src2, CL))]>{
  let Opcode = opcode;
}

class shiftLD_mrCL_NoPat<bits<8> opcode, Format f, string mnemonic,
                         X86TypeInfo typeinfo>
  : I_N_MR_EVEX<opcode, f, typeinfo, mnemonic,
                "{%cl, $src2, $dst|$dst, $src2, cl}", []>{
  let Opcode = opcode;
}

class shiftLD_mrCL_NDD_NoPat<bits<8> opcode, Format f, string mnemonic,
                             X86TypeInfo typeinfo>
  : I_R_MR_EVEX<opcode, f, typeinfo, mnemonic,
                "{%cl, $src2, $src1, $dst|$dst, $src1, $src2, cl}", []>{
  let Opcode = opcode;
}

multiclass mshiftLD_mrCL<bits<8> opcode, Format f, string mnemonic,
                         SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Predicates = [NoNDD] in
  def NAME : shiftLD_mrCL<opcode, f, mnemonic, node, typeinfo>;
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _EVEX : shiftLD_mrCL_NoPat<opcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shiftLD_mrCL_NDD<opcode, f, mnemonic, node, typeinfo>, EVEX_B4V;
}
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _NF : shiftLD_mrCL_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_NF;
let Predicates = [HasNDD, In64BitMode], mayLoad = 1 in
  def _ND_NF : shiftLD_mrCL_NDD_NoPat<opcode, f, mnemonic, typeinfo>,
               EVEX_B4V, EVEX_NF;
}

class shiftRD_mrCL<bits<8> opcode, Format f, string mnemonic, SDNode node,
                   X86TypeInfo typeinfo>
  : I_N_MR<opcode, f, typeinfo, mnemonic,
           "{%cl, $src2, $dst|$dst, $src2, cl}",
           [(store (node typeinfo.RegClass:$src2,
                   (typeinfo.LoadNode addr:$dst), CL), addr:$dst)]>,
    TB{
  let Opcode = opcode;
}

class shiftRD_mrCL_NoPat<bits<8> opcode, Format f, string mnemonic,
                         X86TypeInfo typeinfo>
  : I_N_MR_EVEX<opcode, f, typeinfo, mnemonic,
                "{%cl, $src2, $dst|$dst, $src2, cl}", []>{
  let Opcode = opcode;
}

class shiftRD_mrCL_NDD<bits<8> opcode, Format f, string mnemonic, SDNode node,
                       X86TypeInfo typeinfo>
  : I_R_MR_EVEX<opcode, f, typeinfo, mnemonic,
                "{%cl, $src2, $src1, $dst|$dst, $src1, $src2, cl}",
                [(set typeinfo.RegClass:$dst,
                 (node typeinfo.RegClass:$src2,
                 (typeinfo.LoadNode addr:$src1), CL))]>{
  let Opcode = opcode;
}

class shiftRD_mrCL_NDD_NoPat<bits<8> opcode, Format f, string mnemonic,
                             X86TypeInfo typeinfo>
  : I_R_MR_EVEX<opcode, f, typeinfo, mnemonic,
                "{%cl, $src2, $src1, $dst|$dst, $src1, $src2, cl}", []>{
  let Opcode = opcode;
}

multiclass mshiftRD_mrCL<bits<8> opcode, Format f, string mnemonic,
                         SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Predicates = [NoNDD] in
  def NAME : shiftRD_mrCL<opcode, f, mnemonic, node, typeinfo>;
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _EVEX : shiftRD_mrCL_NoPat<opcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shiftRD_mrCL_NDD<opcode, f, mnemonic, node, typeinfo>, EVEX_B4V;
}
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _NF : shiftRD_mrCL_NoPat<opcode, f, mnemonic, typeinfo>, EVEX_NF;
let Predicates = [HasNDD, In64BitMode], mayLoad = 1 in
  def _ND_NF : shiftRD_mrCL_NDD_NoPat<opcode, f, mnemonic, typeinfo>,
               EVEX_B4V, EVEX_NF;
}

let Uses = [CL], SchedRW = [WriteSHDmrcl] in {
defm SHLD16mrCL : mshiftLD_mrCL<0xA5, MRMDestMem, "shld", X86fshl, Xi16>;
defm SHLD32mrCL : mshiftLD_mrCL<0xA5, MRMDestMem, "shld", fshl, Xi32>;
defm SHLD64mrCL : mshiftLD_mrCL<0xA5, MRMDestMem, "shld", fshl, Xi64>;

defm SHRD16mrCL : mshiftRD_mrCL<0xAD, MRMDestMem, "shrd", X86fshr, Xi16>;
defm SHRD32mrCL : mshiftRD_mrCL<0xAD, MRMDestMem, "shrd", fshr, Xi32>;
defm SHRD64mrCL : mshiftRD_mrCL<0xAD, MRMDestMem, "shrd", fshr, Xi64>;
}

class shiftLD_mri8<bits<8> opcode, Format f, string mnemonic, SDNode node,
                   X86TypeInfo typeinfo>
  : I_N_MRI<opcode, f, typeinfo, mnemonic,
            "{$src3, $src2, $dst|$dst, $src2, $src3}",
            [(store (node (typeinfo.LoadNode addr:$dst),
                  typeinfo.RegClass:$src2, (i8 imm:$src3)), addr:$dst)]>,
    TB{
  let Opcode = opcode;
  let ImmT = Imm8;
}

class shiftLD_mri8_NDD<bits<8> opcode, Format f, string mnemonic, SDNode node,
                       X86TypeInfo typeinfo>
  : I_R_MRI_EVEX<opcode, f, typeinfo, mnemonic,
                 "{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                 [(set typeinfo.RegClass:$dst,
                  (node (typeinfo.LoadNode addr:$src1),
                    typeinfo.RegClass:$src2, (i8 imm:$src3)))]>{
  let Opcode = opcode;
  let ImmT = Imm8;
}

class shiftLD_mri8_NoPat<bits<8> opcode, Format f, string mnemonic,
                         X86TypeInfo typeinfo>
  : I_N_MRI_EVEX<opcode, f, typeinfo, mnemonic,
                 "{$src3, $src2, $dst|$dst, $src2, $src3}", []>{
  let Opcode = opcode;
  let ImmT = Imm8;
}

class shiftLD_mri8_NDD_NoPat<bits<8> opcode, Format f, string mnemonic,
                             X86TypeInfo typeinfo>
  : I_R_MRI_EVEX<opcode, f, typeinfo, mnemonic,
                 "{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}", []>{
  let Opcode = opcode;
  let ImmT = Imm8;
}

multiclass mshiftLD_mri8<bits<8> opcode, bits<8> newcode, Format f,
                         string mnemonic, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Predicates = [NoNDD] in
  def NAME : shiftLD_mri8<opcode, f, mnemonic, node, typeinfo>;
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _EVEX : shiftLD_mri8_NoPat<newcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shiftLD_mri8_NDD<newcode, f, mnemonic, node, typeinfo>, EVEX_B4V;
}
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _NF : shiftLD_mri8_NoPat<newcode, f, mnemonic, typeinfo>, EVEX_NF;
let Predicates = [HasNDD, In64BitMode], mayLoad = 1 in
  def _ND_NF : shiftLD_mri8_NDD_NoPat<newcode, f, mnemonic, typeinfo>,
               EVEX_B4V, EVEX_NF;
}

class shiftRD_mri8<bits<8> opcode, Format f, string mnemonic, SDNode node,
                   X86TypeInfo typeinfo>
  : I_N_MRI<opcode, f, typeinfo, mnemonic,
            "{$src3, $src2, $dst|$dst, $src2, $src3}",
            [(store (node typeinfo.RegClass:$src2,
                  (typeinfo.LoadNode addr:$dst), (i8 imm:$src3)), addr:$dst)]>,
    TB{
  let Opcode = opcode;
  let ImmT = Imm8;
}

class shiftRD_mri8_NDD<bits<8> opcode, Format f, string mnemonic, SDNode node,
                       X86TypeInfo typeinfo>
  : I_R_MRI_EVEX<opcode, f, typeinfo, mnemonic,
                 "{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                 [(set typeinfo.RegClass:$dst,
                  (node typeinfo.RegClass:$src2,
                  (typeinfo.LoadNode addr:$src1), (i8 imm:$src3)))]>{
  let Opcode = opcode;
  let ImmT = Imm8;
}

class shiftRD_mri8_NoPat<bits<8> opcode, Format f, string mnemonic,
                         X86TypeInfo typeinfo>
  : I_N_MRI_EVEX<opcode, f, typeinfo, mnemonic,
                 "{$src3, $src2, $dst|$dst, $src2, $src3}", []>{
  let Opcode = opcode;
  let ImmT = Imm8;
}

class shiftRD_mri8_NDD_NoPat<bits<8> opcode, Format f, string mnemonic,
                             X86TypeInfo typeinfo>
  : I_R_MRI_EVEX<opcode, f, typeinfo, mnemonic,
                 "{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}", []>{
  let Opcode = opcode;
  let ImmT = Imm8;
}

multiclass mshiftRD_mri8<bits<8> opcode, bits<8> newcode, Format f,
                         string mnemonic, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Predicates = [NoNDD] in
  def NAME : shiftRD_mri8<opcode, f, mnemonic, node, typeinfo>;
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _EVEX : shiftRD_mri8_NoPat<newcode, f, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : shiftRD_mri8_NDD<newcode, f, mnemonic, node, typeinfo>, EVEX_B4V;
}
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _NF : shiftRD_mri8_NoPat<newcode, f, mnemonic, typeinfo>, EVEX_NF;
let Predicates = [HasNDD, In64BitMode], mayLoad = 1 in
  def _ND_NF : shiftRD_mri8_NDD_NoPat<newcode, f, mnemonic, typeinfo>,
               EVEX_B4V, EVEX_NF;
}

let SchedRW = [WriteSHDmri] in {
defm SHLD16mri8 : mshiftLD_mri8<0xA4, 0x24, MRMDestMem, "shld", X86fshl, Xi16>;
defm SHLD32mri8 : mshiftLD_mri8<0xA4, 0x24, MRMDestMem, "shld", fshl, Xi32>;
defm SHLD64mri8 : mshiftLD_mri8<0xA4, 0x24, MRMDestMem, "shld", fshl, Xi64>;

defm SHRD16mri8 : mshiftRD_mri8<0xAC, 0x2C, MRMDestMem, "shrd", X86fshr, Xi16>;
defm SHRD32mri8 : mshiftRD_mri8<0xAC, 0x2C, MRMDestMem, "shrd", fshr, Xi32>;
defm SHRD64mri8 : mshiftRD_mri8<0xAC, 0x2C, MRMDestMem, "shrd", fshr, Xi64>;
}

#else // INTEL_FEATURE_ISA_APX_F
// FIXME: Someone needs to smear multipattern goodness all over this file.

let Defs = [EFLAGS], hasSideEffects = 0 in {

let Constraints = "$src1 = $dst" in {
let Uses = [CL], SchedRW = [WriteShiftCL] in {
def SHL8rCL  : I<0xD2, MRM4r, (outs GR8 :$dst), (ins GR8 :$src1),
                 "shl{b}\t{%cl, $dst|$dst, cl}",
                 [(set GR8:$dst, (shl GR8:$src1, CL))]>;
def SHL16rCL : I<0xD3, MRM4r, (outs GR16:$dst), (ins GR16:$src1),
                 "shl{w}\t{%cl, $dst|$dst, cl}",
                 [(set GR16:$dst, (shl GR16:$src1, CL))]>, OpSize16;
def SHL32rCL : I<0xD3, MRM4r, (outs GR32:$dst), (ins GR32:$src1),
                 "shl{l}\t{%cl, $dst|$dst, cl}",
                 [(set GR32:$dst, (shl GR32:$src1, CL))]>, OpSize32;
def SHL64rCL : RI<0xD3, MRM4r, (outs GR64:$dst), (ins GR64:$src1),
                  "shl{q}\t{%cl, $dst|$dst, cl}",
                  [(set GR64:$dst, (shl GR64:$src1, CL))]>;
} // Uses = [CL], SchedRW

let SchedRW = [WriteShift] in {
let isConvertibleToThreeAddress = 1 in {   // Can transform into LEA.
def SHL8ri   : Ii8<0xC0, MRM4r, (outs GR8 :$dst), (ins GR8 :$src1, u8imm:$src2),
                   "shl{b}\t{$src2, $dst|$dst, $src2}",
                   [(set GR8:$dst, (shl GR8:$src1, (i8 imm:$src2)))]>;

def SHL16ri  : Ii8<0xC1, MRM4r, (outs GR16:$dst), (ins GR16:$src1, u8imm:$src2),
                   "shl{w}\t{$src2, $dst|$dst, $src2}",
                   [(set GR16:$dst, (shl GR16:$src1, (i8 imm:$src2)))]>,
                   OpSize16;
def SHL32ri  : Ii8<0xC1, MRM4r, (outs GR32:$dst), (ins GR32:$src1, u8imm:$src2),
                   "shl{l}\t{$src2, $dst|$dst, $src2}",
                   [(set GR32:$dst, (shl GR32:$src1, (i8 imm:$src2)))]>,
                   OpSize32;
def SHL64ri  : RIi8<0xC1, MRM4r, (outs GR64:$dst),
                    (ins GR64:$src1, u8imm:$src2),
                    "shl{q}\t{$src2, $dst|$dst, $src2}",
                    [(set GR64:$dst, (shl GR64:$src1, (i8 imm:$src2)))]>;
} // isConvertibleToThreeAddress = 1

def SHL8r1   : I<0xD0, MRM4r, (outs GR8:$dst), (ins GR8:$src1),
                 "shl{b}\t$dst", []>;
def SHL16r1  : I<0xD1, MRM4r, (outs GR16:$dst), (ins GR16:$src1),
                 "shl{w}\t$dst", []>, OpSize16;
def SHL32r1  : I<0xD1, MRM4r, (outs GR32:$dst), (ins GR32:$src1),
                 "shl{l}\t$dst", []>, OpSize32;
def SHL64r1  : RI<0xD1, MRM4r, (outs GR64:$dst), (ins GR64:$src1),
                 "shl{q}\t$dst", []>;
} // SchedRW
} // Constraints = "$src = $dst"

// FIXME: Why do we need an explicit "Uses = [CL]" when the instr has a pattern
// using CL?
let Uses = [CL], SchedRW = [WriteShiftCLLd, WriteRMW] in {
def SHL8mCL  : I<0xD2, MRM4m, (outs), (ins i8mem :$dst),
                 "shl{b}\t{%cl, $dst|$dst, cl}",
                 [(store (shl (loadi8 addr:$dst), CL), addr:$dst)]>;
def SHL16mCL : I<0xD3, MRM4m, (outs), (ins i16mem:$dst),
                 "shl{w}\t{%cl, $dst|$dst, cl}",
                 [(store (shl (loadi16 addr:$dst), CL), addr:$dst)]>,
                 OpSize16;
def SHL32mCL : I<0xD3, MRM4m, (outs), (ins i32mem:$dst),
                 "shl{l}\t{%cl, $dst|$dst, cl}",
                 [(store (shl (loadi32 addr:$dst), CL), addr:$dst)]>,
                 OpSize32;
def SHL64mCL : RI<0xD3, MRM4m, (outs), (ins i64mem:$dst),
                  "shl{q}\t{%cl, $dst|$dst, cl}",
                  [(store (shl (loadi64 addr:$dst), CL), addr:$dst)]>,
                  Requires<[In64BitMode]>;
} // Uses, SchedRW

let SchedRW = [WriteShiftLd, WriteRMW], mayLoad = 1, mayStore = 1 in {
def SHL8mi   : Ii8<0xC0, MRM4m, (outs), (ins i8mem :$dst, u8imm:$src),
                   "shl{b}\t{$src, $dst|$dst, $src}",
                [(store (shl (loadi8 addr:$dst), (i8 imm:$src)), addr:$dst)]>;
def SHL16mi  : Ii8<0xC1, MRM4m, (outs), (ins i16mem:$dst, u8imm:$src),
                   "shl{w}\t{$src, $dst|$dst, $src}",
               [(store (shl (loadi16 addr:$dst), (i8 imm:$src)), addr:$dst)]>,
               OpSize16;
def SHL32mi  : Ii8<0xC1, MRM4m, (outs), (ins i32mem:$dst, u8imm:$src),
                   "shl{l}\t{$src, $dst|$dst, $src}",
               [(store (shl (loadi32 addr:$dst), (i8 imm:$src)), addr:$dst)]>,
               OpSize32;
def SHL64mi : RIi8<0xC1, MRM4m, (outs), (ins i64mem:$dst, u8imm:$src),
                  "shl{q}\t{$src, $dst|$dst, $src}",
                  [(store (shl (loadi64 addr:$dst), (i8 imm:$src)), addr:$dst)]>,
                  Requires<[In64BitMode]>;

// Shift by 1
def SHL8m1   : I<0xD0, MRM4m, (outs), (ins i8mem :$dst),
                 "shl{b}\t$dst", []>;
def SHL16m1  : I<0xD1, MRM4m, (outs), (ins i16mem:$dst),
                 "shl{w}\t$dst", []>, OpSize16;
def SHL32m1  : I<0xD1, MRM4m, (outs), (ins i32mem:$dst),
                 "shl{l}\t$dst", []>, OpSize32;
def SHL64m1 : RI<0xD1, MRM4m, (outs), (ins i64mem:$dst),
                  "shl{q}\t$dst", []>, Requires<[In64BitMode]>;
} // SchedRW, mayLoad, mayStore

let Constraints = "$src1 = $dst" in {
let Uses = [CL], SchedRW = [WriteShiftCL] in {
def SHR8rCL  : I<0xD2, MRM5r, (outs GR8 :$dst), (ins GR8 :$src1),
                 "shr{b}\t{%cl, $dst|$dst, cl}",
                 [(set GR8:$dst, (srl GR8:$src1, CL))]>;
def SHR16rCL : I<0xD3, MRM5r, (outs GR16:$dst), (ins GR16:$src1),
                 "shr{w}\t{%cl, $dst|$dst, cl}",
                 [(set GR16:$dst, (srl GR16:$src1, CL))]>, OpSize16;
def SHR32rCL : I<0xD3, MRM5r, (outs GR32:$dst), (ins GR32:$src1),
                 "shr{l}\t{%cl, $dst|$dst, cl}",
                 [(set GR32:$dst, (srl GR32:$src1, CL))]>, OpSize32;
def SHR64rCL : RI<0xD3, MRM5r, (outs GR64:$dst), (ins GR64:$src1),
                  "shr{q}\t{%cl, $dst|$dst, cl}",
                  [(set GR64:$dst, (srl GR64:$src1, CL))]>;
} // Uses, SchedRW

let SchedRW = [WriteShift] in {
def SHR8ri   : Ii8<0xC0, MRM5r, (outs GR8:$dst), (ins GR8:$src1, u8imm:$src2),
                   "shr{b}\t{$src2, $dst|$dst, $src2}",
                   [(set GR8:$dst, (srl GR8:$src1, (i8 imm:$src2)))]>;
def SHR16ri  : Ii8<0xC1, MRM5r, (outs GR16:$dst), (ins GR16:$src1, u8imm:$src2),
                   "shr{w}\t{$src2, $dst|$dst, $src2}",
                   [(set GR16:$dst, (srl GR16:$src1, (i8 imm:$src2)))]>,
                   OpSize16;
def SHR32ri  : Ii8<0xC1, MRM5r, (outs GR32:$dst), (ins GR32:$src1, u8imm:$src2),
                   "shr{l}\t{$src2, $dst|$dst, $src2}",
                   [(set GR32:$dst, (srl GR32:$src1, (i8 imm:$src2)))]>,
                   OpSize32;
def SHR64ri : RIi8<0xC1, MRM5r, (outs GR64:$dst), (ins GR64:$src1, u8imm:$src2),
                  "shr{q}\t{$src2, $dst|$dst, $src2}",
                  [(set GR64:$dst, (srl GR64:$src1, (i8 imm:$src2)))]>;

// Shift right by 1
def SHR8r1   : I<0xD0, MRM5r, (outs GR8:$dst), (ins GR8:$src1),
                 "shr{b}\t$dst", []>;
def SHR16r1  : I<0xD1, MRM5r, (outs GR16:$dst), (ins GR16:$src1),
                 "shr{w}\t$dst", []>, OpSize16;
def SHR32r1  : I<0xD1, MRM5r, (outs GR32:$dst), (ins GR32:$src1),
                 "shr{l}\t$dst", []>, OpSize32;
def SHR64r1  : RI<0xD1, MRM5r, (outs GR64:$dst), (ins GR64:$src1),
                 "shr{q}\t$dst", []>;
} // SchedRW
} // Constraints = "$src = $dst"


let Uses = [CL], SchedRW = [WriteShiftCLLd, WriteRMW] in {
def SHR8mCL  : I<0xD2, MRM5m, (outs), (ins i8mem :$dst),
                 "shr{b}\t{%cl, $dst|$dst, cl}",
                 [(store (srl (loadi8 addr:$dst), CL), addr:$dst)]>;
def SHR16mCL : I<0xD3, MRM5m, (outs), (ins i16mem:$dst),
                 "shr{w}\t{%cl, $dst|$dst, cl}",
                 [(store (srl (loadi16 addr:$dst), CL), addr:$dst)]>,
                 OpSize16;
def SHR32mCL : I<0xD3, MRM5m, (outs), (ins i32mem:$dst),
                 "shr{l}\t{%cl, $dst|$dst, cl}",
                 [(store (srl (loadi32 addr:$dst), CL), addr:$dst)]>,
                 OpSize32;
def SHR64mCL : RI<0xD3, MRM5m, (outs), (ins i64mem:$dst),
                  "shr{q}\t{%cl, $dst|$dst, cl}",
                  [(store (srl (loadi64 addr:$dst), CL), addr:$dst)]>,
                  Requires<[In64BitMode]>;
} // Uses, SchedRW

let SchedRW = [WriteShiftLd, WriteRMW], mayLoad = 1, mayStore = 1 in {
def SHR8mi   : Ii8<0xC0, MRM5m, (outs), (ins i8mem :$dst, u8imm:$src),
                   "shr{b}\t{$src, $dst|$dst, $src}",
                [(store (srl (loadi8 addr:$dst), (i8 imm:$src)), addr:$dst)]>;
def SHR16mi  : Ii8<0xC1, MRM5m, (outs), (ins i16mem:$dst, u8imm:$src),
                   "shr{w}\t{$src, $dst|$dst, $src}",
               [(store (srl (loadi16 addr:$dst), (i8 imm:$src)), addr:$dst)]>,
               OpSize16;
def SHR32mi  : Ii8<0xC1, MRM5m, (outs), (ins i32mem:$dst, u8imm:$src),
                   "shr{l}\t{$src, $dst|$dst, $src}",
               [(store (srl (loadi32 addr:$dst), (i8 imm:$src)), addr:$dst)]>,
               OpSize32;
def SHR64mi : RIi8<0xC1, MRM5m, (outs), (ins i64mem:$dst, u8imm:$src),
                  "shr{q}\t{$src, $dst|$dst, $src}",
                 [(store (srl (loadi64 addr:$dst), (i8 imm:$src)), addr:$dst)]>,
                 Requires<[In64BitMode]>;

// Shift by 1
def SHR8m1   : I<0xD0, MRM5m, (outs), (ins i8mem :$dst),
                 "shr{b}\t$dst", []>;
def SHR16m1  : I<0xD1, MRM5m, (outs), (ins i16mem:$dst),
                 "shr{w}\t$dst", []>, OpSize16;
def SHR32m1  : I<0xD1, MRM5m, (outs), (ins i32mem:$dst),
                 "shr{l}\t$dst", []>, OpSize32;
def SHR64m1 : RI<0xD1, MRM5m, (outs), (ins i64mem:$dst),
                  "shr{q}\t$dst", []>, Requires<[In64BitMode]>;
} // SchedRW, mayLoad, mayStore


let Constraints = "$src1 = $dst" in {
let Uses = [CL], SchedRW = [WriteShiftCL] in {
def SAR8rCL  : I<0xD2, MRM7r, (outs GR8 :$dst), (ins GR8 :$src1),
                 "sar{b}\t{%cl, $dst|$dst, cl}",
                 [(set GR8:$dst, (sra GR8:$src1, CL))]>;
def SAR16rCL : I<0xD3, MRM7r, (outs GR16:$dst), (ins GR16:$src1),
                 "sar{w}\t{%cl, $dst|$dst, cl}",
                 [(set GR16:$dst, (sra GR16:$src1, CL))]>,
                 OpSize16;
def SAR32rCL : I<0xD3, MRM7r, (outs GR32:$dst), (ins GR32:$src1),
                 "sar{l}\t{%cl, $dst|$dst, cl}",
                 [(set GR32:$dst, (sra GR32:$src1, CL))]>,
                 OpSize32;
def SAR64rCL : RI<0xD3, MRM7r, (outs GR64:$dst), (ins GR64:$src1),
                 "sar{q}\t{%cl, $dst|$dst, cl}",
                 [(set GR64:$dst, (sra GR64:$src1, CL))]>;
} // Uses, SchedRW

let SchedRW = [WriteShift] in {
def SAR8ri   : Ii8<0xC0, MRM7r, (outs GR8 :$dst), (ins GR8 :$src1, u8imm:$src2),
                   "sar{b}\t{$src2, $dst|$dst, $src2}",
                   [(set GR8:$dst, (sra GR8:$src1, (i8 imm:$src2)))]>;
def SAR16ri  : Ii8<0xC1, MRM7r, (outs GR16:$dst), (ins GR16:$src1, u8imm:$src2),
                   "sar{w}\t{$src2, $dst|$dst, $src2}",
                   [(set GR16:$dst, (sra GR16:$src1, (i8 imm:$src2)))]>,
                   OpSize16;
def SAR32ri  : Ii8<0xC1, MRM7r, (outs GR32:$dst), (ins GR32:$src1, u8imm:$src2),
                   "sar{l}\t{$src2, $dst|$dst, $src2}",
                   [(set GR32:$dst, (sra GR32:$src1, (i8 imm:$src2)))]>,
                   OpSize32;
def SAR64ri  : RIi8<0xC1, MRM7r, (outs GR64:$dst),
                    (ins GR64:$src1, u8imm:$src2),
                    "sar{q}\t{$src2, $dst|$dst, $src2}",
                    [(set GR64:$dst, (sra GR64:$src1, (i8 imm:$src2)))]>;

// Shift by 1
def SAR8r1   : I<0xD0, MRM7r, (outs GR8 :$dst), (ins GR8 :$src1),
                 "sar{b}\t$dst", []>;
def SAR16r1  : I<0xD1, MRM7r, (outs GR16:$dst), (ins GR16:$src1),
                 "sar{w}\t$dst", []>, OpSize16;
def SAR32r1  : I<0xD1, MRM7r, (outs GR32:$dst), (ins GR32:$src1),
                 "sar{l}\t$dst", []>, OpSize32;
def SAR64r1  : RI<0xD1, MRM7r, (outs GR64:$dst), (ins GR64:$src1),
                  "sar{q}\t$dst", []>;
} // SchedRW
} // Constraints = "$src = $dst"


let Uses = [CL], SchedRW = [WriteShiftCLLd, WriteRMW] in {
def SAR8mCL  : I<0xD2, MRM7m, (outs), (ins i8mem :$dst),
                 "sar{b}\t{%cl, $dst|$dst, cl}",
                 [(store (sra (loadi8 addr:$dst), CL), addr:$dst)]>;
def SAR16mCL : I<0xD3, MRM7m, (outs), (ins i16mem:$dst),
                 "sar{w}\t{%cl, $dst|$dst, cl}",
                 [(store (sra (loadi16 addr:$dst), CL), addr:$dst)]>,
                 OpSize16;
def SAR32mCL : I<0xD3, MRM7m, (outs), (ins i32mem:$dst),
                 "sar{l}\t{%cl, $dst|$dst, cl}",
                 [(store (sra (loadi32 addr:$dst), CL), addr:$dst)]>,
                 OpSize32;
def SAR64mCL : RI<0xD3, MRM7m, (outs), (ins i64mem:$dst),
                 "sar{q}\t{%cl, $dst|$dst, cl}",
                 [(store (sra (loadi64 addr:$dst), CL), addr:$dst)]>,
                 Requires<[In64BitMode]>;
} // Uses, SchedRW

let SchedRW = [WriteShiftLd, WriteRMW], mayLoad = 1, mayStore = 1 in {
def SAR8mi   : Ii8<0xC0, MRM7m, (outs), (ins i8mem :$dst, u8imm:$src),
                   "sar{b}\t{$src, $dst|$dst, $src}",
                [(store (sra (loadi8 addr:$dst), (i8 imm:$src)), addr:$dst)]>;
def SAR16mi  : Ii8<0xC1, MRM7m, (outs), (ins i16mem:$dst, u8imm:$src),
                   "sar{w}\t{$src, $dst|$dst, $src}",
               [(store (sra (loadi16 addr:$dst), (i8 imm:$src)), addr:$dst)]>,
               OpSize16;
def SAR32mi  : Ii8<0xC1, MRM7m, (outs), (ins i32mem:$dst, u8imm:$src),
                   "sar{l}\t{$src, $dst|$dst, $src}",
               [(store (sra (loadi32 addr:$dst), (i8 imm:$src)), addr:$dst)]>,
               OpSize32;
def SAR64mi  : RIi8<0xC1, MRM7m, (outs), (ins i64mem:$dst, u8imm:$src),
                    "sar{q}\t{$src, $dst|$dst, $src}",
                 [(store (sra (loadi64 addr:$dst), (i8 imm:$src)), addr:$dst)]>,
                 Requires<[In64BitMode]>;

// Shift by 1
def SAR8m1   : I<0xD0, MRM7m, (outs), (ins i8mem :$dst),
                 "sar{b}\t$dst", []>;
def SAR16m1  : I<0xD1, MRM7m, (outs), (ins i16mem:$dst),
                 "sar{w}\t$dst", []>, OpSize16;
def SAR32m1  : I<0xD1, MRM7m, (outs), (ins i32mem:$dst),
                 "sar{l}\t$dst", []>, OpSize32;
def SAR64m1 : RI<0xD1, MRM7m, (outs), (ins i64mem:$dst),
                  "sar{q}\t$dst", []>, Requires<[In64BitMode]>;
} // SchedRW

//===----------------------------------------------------------------------===//
// Rotate instructions
//===----------------------------------------------------------------------===//

let Constraints = "$src1 = $dst" in {

let Uses = [CL, EFLAGS], SchedRW = [WriteRotateCL] in {
def RCL8rCL : I<0xD2, MRM2r, (outs GR8:$dst), (ins GR8:$src1),
                "rcl{b}\t{%cl, $dst|$dst, cl}", []>;
def RCL16rCL : I<0xD3, MRM2r, (outs GR16:$dst), (ins GR16:$src1),
                 "rcl{w}\t{%cl, $dst|$dst, cl}", []>, OpSize16;
def RCL32rCL : I<0xD3, MRM2r, (outs GR32:$dst), (ins GR32:$src1),
                 "rcl{l}\t{%cl, $dst|$dst, cl}", []>, OpSize32;
def RCL64rCL : RI<0xD3, MRM2r, (outs GR64:$dst), (ins GR64:$src1),
                  "rcl{q}\t{%cl, $dst|$dst, cl}", []>;
} // Uses = [CL, EFLAGS], SchedRW

let Uses = [EFLAGS], SchedRW = [WriteRotate] in {
def RCL8r1 : I<0xD0, MRM2r, (outs GR8:$dst), (ins GR8:$src1),
               "rcl{b}\t$dst", []>;
def RCL8ri : Ii8<0xC0, MRM2r, (outs GR8:$dst), (ins GR8:$src1, u8imm:$cnt),
                 "rcl{b}\t{$cnt, $dst|$dst, $cnt}", []>;
def RCL16r1 : I<0xD1, MRM2r, (outs GR16:$dst), (ins GR16:$src1),
                "rcl{w}\t$dst", []>, OpSize16;
def RCL16ri : Ii8<0xC1, MRM2r, (outs GR16:$dst), (ins GR16:$src1, u8imm:$cnt),
                  "rcl{w}\t{$cnt, $dst|$dst, $cnt}", []>, OpSize16;
def RCL32r1 : I<0xD1, MRM2r, (outs GR32:$dst), (ins GR32:$src1),
                "rcl{l}\t$dst", []>, OpSize32;
def RCL32ri : Ii8<0xC1, MRM2r, (outs GR32:$dst), (ins GR32:$src1, u8imm:$cnt),
                  "rcl{l}\t{$cnt, $dst|$dst, $cnt}", []>, OpSize32;
def RCL64r1 : RI<0xD1, MRM2r, (outs GR64:$dst), (ins GR64:$src1),
                 "rcl{q}\t$dst", []>;
def RCL64ri : RIi8<0xC1, MRM2r, (outs GR64:$dst), (ins GR64:$src1, u8imm:$cnt),
                   "rcl{q}\t{$cnt, $dst|$dst, $cnt}", []>;
} // Uses = [EFLAGS], SchedRW

let Uses = [CL, EFLAGS], SchedRW = [WriteRotateCL] in {
def RCR8rCL : I<0xD2, MRM3r, (outs GR8:$dst), (ins GR8:$src1),
                "rcr{b}\t{%cl, $dst|$dst, cl}", []>;
def RCR16rCL : I<0xD3, MRM3r, (outs GR16:$dst), (ins GR16:$src1),
                 "rcr{w}\t{%cl, $dst|$dst, cl}", []>, OpSize16;
def RCR32rCL : I<0xD3, MRM3r, (outs GR32:$dst), (ins GR32:$src1),
                 "rcr{l}\t{%cl, $dst|$dst, cl}", []>, OpSize32;
def RCR64rCL : RI<0xD3, MRM3r, (outs GR64:$dst), (ins GR64:$src1),
                  "rcr{q}\t{%cl, $dst|$dst, cl}", []>;
} // Uses = [CL, EFLAGS], SchedRW

let Uses = [EFLAGS], SchedRW = [WriteRotate] in {
def RCR8r1 : I<0xD0, MRM3r, (outs GR8:$dst), (ins GR8:$src1),
               "rcr{b}\t$dst", []>;
def RCR8ri : Ii8<0xC0, MRM3r, (outs GR8:$dst), (ins GR8:$src1, u8imm:$cnt),
                 "rcr{b}\t{$cnt, $dst|$dst, $cnt}", []>;
def RCR16r1 : I<0xD1, MRM3r, (outs GR16:$dst), (ins GR16:$src1),
                "rcr{w}\t$dst", []>, OpSize16;
def RCR16ri : Ii8<0xC1, MRM3r, (outs GR16:$dst), (ins GR16:$src1, u8imm:$cnt),
                  "rcr{w}\t{$cnt, $dst|$dst, $cnt}", []>, OpSize16;
def RCR32r1 : I<0xD1, MRM3r, (outs GR32:$dst), (ins GR32:$src1),
                "rcr{l}\t$dst", []>, OpSize32;
def RCR32ri : Ii8<0xC1, MRM3r, (outs GR32:$dst), (ins GR32:$src1, u8imm:$cnt),
                  "rcr{l}\t{$cnt, $dst|$dst, $cnt}", []>, OpSize32;
def RCR64r1 : RI<0xD1, MRM3r, (outs GR64:$dst), (ins GR64:$src1),
                 "rcr{q}\t$dst", []>;
def RCR64ri : RIi8<0xC1, MRM3r, (outs GR64:$dst), (ins GR64:$src1, u8imm:$cnt),
                   "rcr{q}\t{$cnt, $dst|$dst, $cnt}", []>;
} // Uses = [EFLAGS], SchedRW
} // Constraints = "$src = $dst"

let mayLoad = 1, mayStore = 1 in {
let Uses = [EFLAGS], SchedRW = [WriteRotateLd, WriteRMW] in {
def RCL8m1 : I<0xD0, MRM2m, (outs), (ins i8mem:$dst),
               "rcl{b}\t$dst", []>;
def RCL8mi : Ii8<0xC0, MRM2m, (outs), (ins i8mem:$dst, u8imm:$cnt),
                 "rcl{b}\t{$cnt, $dst|$dst, $cnt}", []>;
def RCL16m1 : I<0xD1, MRM2m, (outs), (ins i16mem:$dst),
                "rcl{w}\t$dst", []>, OpSize16;
def RCL16mi : Ii8<0xC1, MRM2m, (outs), (ins i16mem:$dst, u8imm:$cnt),
                  "rcl{w}\t{$cnt, $dst|$dst, $cnt}", []>, OpSize16;
def RCL32m1 : I<0xD1, MRM2m, (outs), (ins i32mem:$dst),
                "rcl{l}\t$dst", []>, OpSize32;
def RCL32mi : Ii8<0xC1, MRM2m, (outs), (ins i32mem:$dst, u8imm:$cnt),
                  "rcl{l}\t{$cnt, $dst|$dst, $cnt}", []>, OpSize32;
def RCL64m1 : RI<0xD1, MRM2m, (outs), (ins i64mem:$dst),
                 "rcl{q}\t$dst", []>, Requires<[In64BitMode]>;
def RCL64mi : RIi8<0xC1, MRM2m, (outs), (ins i64mem:$dst, u8imm:$cnt),
                   "rcl{q}\t{$cnt, $dst|$dst, $cnt}", []>,
                   Requires<[In64BitMode]>;

def RCR8m1 : I<0xD0, MRM3m, (outs), (ins i8mem:$dst),
               "rcr{b}\t$dst", []>;
def RCR8mi : Ii8<0xC0, MRM3m, (outs), (ins i8mem:$dst, u8imm:$cnt),
                 "rcr{b}\t{$cnt, $dst|$dst, $cnt}", []>;
def RCR16m1 : I<0xD1, MRM3m, (outs), (ins i16mem:$dst),
                "rcr{w}\t$dst", []>, OpSize16;
def RCR16mi : Ii8<0xC1, MRM3m, (outs), (ins i16mem:$dst, u8imm:$cnt),
                  "rcr{w}\t{$cnt, $dst|$dst, $cnt}", []>, OpSize16;
def RCR32m1 : I<0xD1, MRM3m, (outs), (ins i32mem:$dst),
                "rcr{l}\t$dst", []>, OpSize32;
def RCR32mi : Ii8<0xC1, MRM3m, (outs), (ins i32mem:$dst, u8imm:$cnt),
                  "rcr{l}\t{$cnt, $dst|$dst, $cnt}", []>, OpSize32;
def RCR64m1 : RI<0xD1, MRM3m, (outs), (ins i64mem:$dst),
                 "rcr{q}\t$dst", []>, Requires<[In64BitMode]>;
def RCR64mi : RIi8<0xC1, MRM3m, (outs), (ins i64mem:$dst, u8imm:$cnt),
                   "rcr{q}\t{$cnt, $dst|$dst, $cnt}", []>,
                   Requires<[In64BitMode]>;
} // Uses = [EFLAGS], SchedRW

let Uses = [CL, EFLAGS], SchedRW = [WriteRotateCLLd, WriteRMW] in {
def RCL8mCL : I<0xD2, MRM2m, (outs), (ins i8mem:$dst),
                "rcl{b}\t{%cl, $dst|$dst, cl}", []>;
def RCL16mCL : I<0xD3, MRM2m, (outs), (ins i16mem:$dst),
                 "rcl{w}\t{%cl, $dst|$dst, cl}", []>, OpSize16;
def RCL32mCL : I<0xD3, MRM2m, (outs), (ins i32mem:$dst),
                 "rcl{l}\t{%cl, $dst|$dst, cl}", []>, OpSize32;
def RCL64mCL : RI<0xD3, MRM2m, (outs), (ins i64mem:$dst),
                  "rcl{q}\t{%cl, $dst|$dst, cl}", []>,
                  Requires<[In64BitMode]>;

def RCR8mCL : I<0xD2, MRM3m, (outs), (ins i8mem:$dst),
                "rcr{b}\t{%cl, $dst|$dst, cl}", []>;
def RCR16mCL : I<0xD3, MRM3m, (outs), (ins i16mem:$dst),
                 "rcr{w}\t{%cl, $dst|$dst, cl}", []>, OpSize16;
def RCR32mCL : I<0xD3, MRM3m, (outs), (ins i32mem:$dst),
                 "rcr{l}\t{%cl, $dst|$dst, cl}", []>, OpSize32;
def RCR64mCL : RI<0xD3, MRM3m, (outs), (ins i64mem:$dst),
                  "rcr{q}\t{%cl, $dst|$dst, cl}", []>,
                  Requires<[In64BitMode]>;
} // Uses = [CL, EFLAGS], SchedRW
} // mayLoad, mayStore

let Constraints = "$src1 = $dst" in {
// FIXME: provide shorter instructions when imm8 == 1
let Uses = [CL], SchedRW = [WriteRotateCL] in {
def ROL8rCL  : I<0xD2, MRM0r, (outs GR8 :$dst), (ins GR8 :$src1),
                 "rol{b}\t{%cl, $dst|$dst, cl}",
                 [(set GR8:$dst, (rotl GR8:$src1, CL))]>;
def ROL16rCL : I<0xD3, MRM0r, (outs GR16:$dst), (ins GR16:$src1),
                 "rol{w}\t{%cl, $dst|$dst, cl}",
                 [(set GR16:$dst, (rotl GR16:$src1, CL))]>, OpSize16;
def ROL32rCL : I<0xD3, MRM0r, (outs GR32:$dst), (ins GR32:$src1),
                 "rol{l}\t{%cl, $dst|$dst, cl}",
                 [(set GR32:$dst, (rotl GR32:$src1, CL))]>, OpSize32;
def ROL64rCL : RI<0xD3, MRM0r, (outs GR64:$dst), (ins GR64:$src1),
                  "rol{q}\t{%cl, $dst|$dst, cl}",
                  [(set GR64:$dst, (rotl GR64:$src1, CL))]>;
} // Uses, SchedRW

let SchedRW = [WriteRotate] in {
def ROL8ri   : Ii8<0xC0, MRM0r, (outs GR8 :$dst), (ins GR8 :$src1, u8imm:$src2),
                   "rol{b}\t{$src2, $dst|$dst, $src2}",
                   [(set GR8:$dst, (rotl GR8:$src1, (i8 imm:$src2)))]>;
def ROL16ri  : Ii8<0xC1, MRM0r, (outs GR16:$dst), (ins GR16:$src1, u8imm:$src2),
                   "rol{w}\t{$src2, $dst|$dst, $src2}",
                   [(set GR16:$dst, (rotl GR16:$src1, (i8 imm:$src2)))]>,
                   OpSize16;
def ROL32ri  : Ii8<0xC1, MRM0r, (outs GR32:$dst), (ins GR32:$src1, u8imm:$src2),
                   "rol{l}\t{$src2, $dst|$dst, $src2}",
                   [(set GR32:$dst, (rotl GR32:$src1, (i8 imm:$src2)))]>,
                   OpSize32;
def ROL64ri  : RIi8<0xC1, MRM0r, (outs GR64:$dst),
                    (ins GR64:$src1, u8imm:$src2),
                    "rol{q}\t{$src2, $dst|$dst, $src2}",
                    [(set GR64:$dst, (rotl GR64:$src1, (i8 imm:$src2)))]>;

// Rotate by 1
def ROL8r1   : I<0xD0, MRM0r, (outs GR8 :$dst), (ins GR8 :$src1),
                 "rol{b}\t$dst", []>;
def ROL16r1  : I<0xD1, MRM0r, (outs GR16:$dst), (ins GR16:$src1),
                 "rol{w}\t$dst", []>, OpSize16;
def ROL32r1  : I<0xD1, MRM0r, (outs GR32:$dst), (ins GR32:$src1),
                 "rol{l}\t$dst", []>, OpSize32;
def ROL64r1  : RI<0xD1, MRM0r, (outs GR64:$dst), (ins GR64:$src1),
                  "rol{q}\t$dst", []>;
} // SchedRW
} // Constraints = "$src = $dst"

let Uses = [CL], SchedRW = [WriteRotateCLLd, WriteRMW] in {
def ROL8mCL  : I<0xD2, MRM0m, (outs), (ins i8mem :$dst),
                 "rol{b}\t{%cl, $dst|$dst, cl}",
                 [(store (rotl (loadi8 addr:$dst), CL), addr:$dst)]>;
def ROL16mCL : I<0xD3, MRM0m, (outs), (ins i16mem:$dst),
                 "rol{w}\t{%cl, $dst|$dst, cl}",
                 [(store (rotl (loadi16 addr:$dst), CL), addr:$dst)]>, OpSize16;
def ROL32mCL : I<0xD3, MRM0m, (outs), (ins i32mem:$dst),
                 "rol{l}\t{%cl, $dst|$dst, cl}",
                 [(store (rotl (loadi32 addr:$dst), CL), addr:$dst)]>, OpSize32;
def ROL64mCL :  RI<0xD3, MRM0m, (outs), (ins i64mem:$dst),
                   "rol{q}\t{%cl, $dst|$dst, cl}",
                   [(store (rotl (loadi64 addr:$dst), CL), addr:$dst)]>,
                   Requires<[In64BitMode]>;
} // Uses, SchedRW

let SchedRW = [WriteRotateLd, WriteRMW], mayLoad = 1, mayStore = 1 in {
def ROL8mi   : Ii8<0xC0, MRM0m, (outs), (ins i8mem :$dst, u8imm:$src1),
                   "rol{b}\t{$src1, $dst|$dst, $src1}",
               [(store (rotl (loadi8 addr:$dst), (i8 imm:$src1)), addr:$dst)]>;
def ROL16mi  : Ii8<0xC1, MRM0m, (outs), (ins i16mem:$dst, u8imm:$src1),
                   "rol{w}\t{$src1, $dst|$dst, $src1}",
              [(store (rotl (loadi16 addr:$dst), (i8 imm:$src1)), addr:$dst)]>,
              OpSize16;
def ROL32mi  : Ii8<0xC1, MRM0m, (outs), (ins i32mem:$dst, u8imm:$src1),
                   "rol{l}\t{$src1, $dst|$dst, $src1}",
              [(store (rotl (loadi32 addr:$dst), (i8 imm:$src1)), addr:$dst)]>,
              OpSize32;
def ROL64mi  : RIi8<0xC1, MRM0m, (outs), (ins i64mem:$dst, u8imm:$src1),
                    "rol{q}\t{$src1, $dst|$dst, $src1}",
                [(store (rotl (loadi64 addr:$dst), (i8 imm:$src1)), addr:$dst)]>,
                Requires<[In64BitMode]>;

// Rotate by 1
def ROL8m1   : I<0xD0, MRM0m, (outs), (ins i8mem :$dst),
                 "rol{b}\t$dst", []>;
def ROL16m1  : I<0xD1, MRM0m, (outs), (ins i16mem:$dst),
                 "rol{w}\t$dst", []>, OpSize16;
def ROL32m1  : I<0xD1, MRM0m, (outs), (ins i32mem:$dst),
                 "rol{l}\t$dst", []>, OpSize32;
def ROL64m1  : RI<0xD1, MRM0m, (outs), (ins i64mem:$dst),
                 "rol{q}\t$dst", []>, Requires<[In64BitMode]>;
} // SchedRW, mayLoad, mayStore

let Constraints = "$src1 = $dst" in {
let Uses = [CL], SchedRW = [WriteRotateCL] in {
def ROR8rCL  : I<0xD2, MRM1r, (outs GR8 :$dst), (ins GR8 :$src1),
                 "ror{b}\t{%cl, $dst|$dst, cl}",
                 [(set GR8:$dst, (rotr GR8:$src1, CL))]>;
def ROR16rCL : I<0xD3, MRM1r, (outs GR16:$dst), (ins GR16:$src1),
                 "ror{w}\t{%cl, $dst|$dst, cl}",
                 [(set GR16:$dst, (rotr GR16:$src1, CL))]>, OpSize16;
def ROR32rCL : I<0xD3, MRM1r, (outs GR32:$dst), (ins GR32:$src1),
                 "ror{l}\t{%cl, $dst|$dst, cl}",
                 [(set GR32:$dst, (rotr GR32:$src1, CL))]>, OpSize32;
def ROR64rCL : RI<0xD3, MRM1r, (outs GR64:$dst), (ins GR64:$src1),
                  "ror{q}\t{%cl, $dst|$dst, cl}",
                  [(set GR64:$dst, (rotr GR64:$src1, CL))]>;
}

let SchedRW = [WriteRotate] in {
def ROR8ri   : Ii8<0xC0, MRM1r, (outs GR8 :$dst), (ins GR8 :$src1, u8imm:$src2),
                   "ror{b}\t{$src2, $dst|$dst, $src2}",
                   [(set GR8:$dst, (rotr GR8:$src1, (i8 imm:$src2)))]>;
def ROR16ri  : Ii8<0xC1, MRM1r, (outs GR16:$dst), (ins GR16:$src1, u8imm:$src2),
                   "ror{w}\t{$src2, $dst|$dst, $src2}",
                   [(set GR16:$dst, (rotr GR16:$src1, (i8 imm:$src2)))]>,
                   OpSize16;
def ROR32ri  : Ii8<0xC1, MRM1r, (outs GR32:$dst), (ins GR32:$src1, u8imm:$src2),
                   "ror{l}\t{$src2, $dst|$dst, $src2}",
                   [(set GR32:$dst, (rotr GR32:$src1, (i8 imm:$src2)))]>,
                   OpSize32;
def ROR64ri  : RIi8<0xC1, MRM1r, (outs GR64:$dst),
                    (ins GR64:$src1, u8imm:$src2),
                    "ror{q}\t{$src2, $dst|$dst, $src2}",
                    [(set GR64:$dst, (rotr GR64:$src1, (i8 imm:$src2)))]>;

// Rotate by 1
def ROR8r1   : I<0xD0, MRM1r, (outs GR8 :$dst), (ins GR8 :$src1),
                 "ror{b}\t$dst", []>;
def ROR16r1  : I<0xD1, MRM1r, (outs GR16:$dst), (ins GR16:$src1),
                 "ror{w}\t$dst", []>, OpSize16;
def ROR32r1  : I<0xD1, MRM1r, (outs GR32:$dst), (ins GR32:$src1),
                 "ror{l}\t$dst", []>, OpSize32;
def ROR64r1  : RI<0xD1, MRM1r, (outs GR64:$dst), (ins GR64:$src1),
                  "ror{q}\t$dst", []>;
} // SchedRW
} // Constraints = "$src = $dst", SchedRW

let Uses = [CL], SchedRW = [WriteRotateCLLd, WriteRMW] in {
def ROR8mCL  : I<0xD2, MRM1m, (outs), (ins i8mem :$dst),
                 "ror{b}\t{%cl, $dst|$dst, cl}",
                 [(store (rotr (loadi8 addr:$dst), CL), addr:$dst)]>;
def ROR16mCL : I<0xD3, MRM1m, (outs), (ins i16mem:$dst),
                 "ror{w}\t{%cl, $dst|$dst, cl}",
                 [(store (rotr (loadi16 addr:$dst), CL), addr:$dst)]>, OpSize16;
def ROR32mCL : I<0xD3, MRM1m, (outs), (ins i32mem:$dst),
                 "ror{l}\t{%cl, $dst|$dst, cl}",
                 [(store (rotr (loadi32 addr:$dst), CL), addr:$dst)]>, OpSize32;
def ROR64mCL : RI<0xD3, MRM1m, (outs), (ins i64mem:$dst),
                  "ror{q}\t{%cl, $dst|$dst, cl}",
                  [(store (rotr (loadi64 addr:$dst), CL), addr:$dst)]>,
                  Requires<[In64BitMode]>;
} // Uses, SchedRW

let SchedRW = [WriteRotateLd, WriteRMW], mayLoad = 1, mayStore =1 in {
def ROR8mi   : Ii8<0xC0, MRM1m, (outs), (ins i8mem :$dst, u8imm:$src),
                   "ror{b}\t{$src, $dst|$dst, $src}",
                   [(store (rotr (loadi8 addr:$dst), (i8 imm:$src)), addr:$dst)]>;
def ROR16mi  : Ii8<0xC1, MRM1m, (outs), (ins i16mem:$dst, u8imm:$src),
                   "ror{w}\t{$src, $dst|$dst, $src}",
                   [(store (rotr (loadi16 addr:$dst), (i8 imm:$src)), addr:$dst)]>,
                   OpSize16;
def ROR32mi  : Ii8<0xC1, MRM1m, (outs), (ins i32mem:$dst, u8imm:$src),
                   "ror{l}\t{$src, $dst|$dst, $src}",
                   [(store (rotr (loadi32 addr:$dst), (i8 imm:$src)), addr:$dst)]>,
                   OpSize32;
def ROR64mi  : RIi8<0xC1, MRM1m, (outs), (ins i64mem:$dst, u8imm:$src),
                    "ror{q}\t{$src, $dst|$dst, $src}",
                    [(store (rotr (loadi64 addr:$dst), (i8 imm:$src)), addr:$dst)]>,
                    Requires<[In64BitMode]>;

// Rotate by 1
def ROR8m1   : I<0xD0, MRM1m, (outs), (ins i8mem :$dst),
                 "ror{b}\t$dst", []>;
def ROR16m1  : I<0xD1, MRM1m, (outs), (ins i16mem:$dst),
                 "ror{w}\t$dst", []>, OpSize16;
def ROR32m1  : I<0xD1, MRM1m, (outs), (ins i32mem:$dst),
                 "ror{l}\t$dst", []>,
                 OpSize32;
def ROR64m1  : RI<0xD1, MRM1m, (outs), (ins i64mem:$dst),
                 "ror{q}\t$dst", []>, Requires<[In64BitMode]>;
} // SchedRW, mayLoad, mayStore


//===----------------------------------------------------------------------===//
// Double shift instructions (generalizations of rotate)
//===----------------------------------------------------------------------===//

let Constraints = "$src1 = $dst" in {

let Uses = [CL], SchedRW = [WriteSHDrrcl] in {
def SHLD16rrCL : I<0xA5, MRMDestReg, (outs GR16:$dst),
                   (ins GR16:$src1, GR16:$src2),
                   "shld{w}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                   [(set GR16:$dst, (X86fshl GR16:$src1, GR16:$src2, CL))]>,
                   TB, OpSize16;
def SHRD16rrCL : I<0xAD, MRMDestReg, (outs GR16:$dst),
                   (ins GR16:$src1, GR16:$src2),
                   "shrd{w}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                   [(set GR16:$dst, (X86fshr GR16:$src2, GR16:$src1, CL))]>,
                   TB, OpSize16;
def SHLD32rrCL : I<0xA5, MRMDestReg, (outs GR32:$dst),
                   (ins GR32:$src1, GR32:$src2),
                   "shld{l}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                   [(set GR32:$dst, (fshl GR32:$src1, GR32:$src2, CL))]>,
                   TB, OpSize32;
def SHRD32rrCL : I<0xAD, MRMDestReg, (outs GR32:$dst),
                   (ins GR32:$src1, GR32:$src2),
                   "shrd{l}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                   [(set GR32:$dst, (fshr GR32:$src2, GR32:$src1, CL))]>,
                   TB, OpSize32;
def SHLD64rrCL : RI<0xA5, MRMDestReg, (outs GR64:$dst),
                    (ins GR64:$src1, GR64:$src2),
                    "shld{q}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                    [(set GR64:$dst, (fshl GR64:$src1, GR64:$src2, CL))]>,
                    TB;
def SHRD64rrCL : RI<0xAD, MRMDestReg, (outs GR64:$dst),
                    (ins GR64:$src1, GR64:$src2),
                    "shrd{q}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                    [(set GR64:$dst, (fshr GR64:$src2, GR64:$src1, CL))]>,
                    TB;
} // Uses, SchedRW

let isCommutable = 1, SchedRW = [WriteSHDrri] in {  // These instructions commute to each other.
def SHLD16rri8 : Ii8<0xA4, MRMDestReg,
                     (outs GR16:$dst),
                     (ins GR16:$src1, GR16:$src2, u8imm:$src3),
                     "shld{w}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                     [(set GR16:$dst, (X86fshl GR16:$src1, GR16:$src2,
                                      (i8 imm:$src3)))]>,
                     TB, OpSize16;
def SHRD16rri8 : Ii8<0xAC, MRMDestReg,
                     (outs GR16:$dst),
                     (ins GR16:$src1, GR16:$src2, u8imm:$src3),
                     "shrd{w}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                     [(set GR16:$dst, (X86fshr GR16:$src2, GR16:$src1,
                                      (i8 imm:$src3)))]>,
                     TB, OpSize16;
def SHLD32rri8 : Ii8<0xA4, MRMDestReg,
                     (outs GR32:$dst),
                     (ins GR32:$src1, GR32:$src2, u8imm:$src3),
                     "shld{l}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                     [(set GR32:$dst, (fshl GR32:$src1, GR32:$src2,
                                      (i8 imm:$src3)))]>,
                 TB, OpSize32;
def SHRD32rri8 : Ii8<0xAC, MRMDestReg,
                     (outs GR32:$dst),
                     (ins GR32:$src1, GR32:$src2, u8imm:$src3),
                     "shrd{l}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                     [(set GR32:$dst, (fshr GR32:$src2, GR32:$src1,
                                      (i8 imm:$src3)))]>,
                 TB, OpSize32;
def SHLD64rri8 : RIi8<0xA4, MRMDestReg,
                      (outs GR64:$dst),
                      (ins GR64:$src1, GR64:$src2, u8imm:$src3),
                      "shld{q}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                      [(set GR64:$dst, (fshl GR64:$src1, GR64:$src2,
                                       (i8 imm:$src3)))]>,
                 TB;
def SHRD64rri8 : RIi8<0xAC, MRMDestReg,
                      (outs GR64:$dst),
                      (ins GR64:$src1, GR64:$src2, u8imm:$src3),
                      "shrd{q}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                      [(set GR64:$dst, (fshr GR64:$src2, GR64:$src1,
                                       (i8 imm:$src3)))]>,
                 TB;
} // SchedRW
} // Constraints = "$src = $dst"

let Uses = [CL], SchedRW = [WriteSHDmrcl] in {
def SHLD16mrCL : I<0xA5, MRMDestMem, (outs), (ins i16mem:$dst, GR16:$src2),
                   "shld{w}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                   [(store (X86fshl (loadi16 addr:$dst), GR16:$src2, CL),
                                    addr:$dst)]>, TB, OpSize16;
def SHRD16mrCL : I<0xAD, MRMDestMem, (outs), (ins i16mem:$dst, GR16:$src2),
                  "shrd{w}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                  [(store (X86fshr GR16:$src2, (loadi16 addr:$dst), CL),
                                   addr:$dst)]>, TB, OpSize16;

def SHLD32mrCL : I<0xA5, MRMDestMem, (outs), (ins i32mem:$dst, GR32:$src2),
                   "shld{l}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                   [(store (fshl (loadi32 addr:$dst), GR32:$src2, CL),
                     addr:$dst)]>, TB, OpSize32;
def SHRD32mrCL : I<0xAD, MRMDestMem, (outs), (ins i32mem:$dst, GR32:$src2),
                  "shrd{l}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                  [(store (fshr GR32:$src2, (loadi32 addr:$dst), CL),
                                addr:$dst)]>, TB, OpSize32;

def SHLD64mrCL : RI<0xA5, MRMDestMem, (outs), (ins i64mem:$dst, GR64:$src2),
                    "shld{q}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                    [(store (fshl (loadi64 addr:$dst), GR64:$src2, CL),
                                  addr:$dst)]>, TB;
def SHRD64mrCL : RI<0xAD, MRMDestMem, (outs), (ins i64mem:$dst, GR64:$src2),
                    "shrd{q}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                    [(store (fshr GR64:$src2, (loadi64 addr:$dst), CL),
                                  addr:$dst)]>, TB;
} // Uses, SchedRW

let SchedRW = [WriteSHDmri] in {
def SHLD16mri8 : Ii8<0xA4, MRMDestMem,
                    (outs), (ins i16mem:$dst, GR16:$src2, u8imm:$src3),
                    "shld{w}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                    [(store (X86fshl (loadi16 addr:$dst), GR16:$src2,
                                     (i8 imm:$src3)), addr:$dst)]>,
                    TB, OpSize16;
def SHRD16mri8 : Ii8<0xAC, MRMDestMem,
                     (outs), (ins i16mem:$dst, GR16:$src2, u8imm:$src3),
                     "shrd{w}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                    [(store (X86fshr GR16:$src2, (loadi16 addr:$dst),
                                     (i8 imm:$src3)), addr:$dst)]>,
                     TB, OpSize16;

def SHLD32mri8 : Ii8<0xA4, MRMDestMem,
                    (outs), (ins i32mem:$dst, GR32:$src2, u8imm:$src3),
                    "shld{l}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                    [(store (fshl (loadi32 addr:$dst), GR32:$src2,
                                  (i8 imm:$src3)), addr:$dst)]>,
                    TB, OpSize32;
def SHRD32mri8 : Ii8<0xAC, MRMDestMem,
                     (outs), (ins i32mem:$dst, GR32:$src2, u8imm:$src3),
                     "shrd{l}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                     [(store (fshr GR32:$src2, (loadi32 addr:$dst),
                                   (i8 imm:$src3)), addr:$dst)]>,
                     TB, OpSize32;

def SHLD64mri8 : RIi8<0xA4, MRMDestMem,
                      (outs), (ins i64mem:$dst, GR64:$src2, u8imm:$src3),
                      "shld{q}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                      [(store (fshl (loadi64 addr:$dst), GR64:$src2,
                                    (i8 imm:$src3)), addr:$dst)]>,
                 TB;
def SHRD64mri8 : RIi8<0xAC, MRMDestMem,
                      (outs), (ins i64mem:$dst, GR64:$src2, u8imm:$src3),
                      "shrd{q}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                      [(store (fshr GR64:$src2, (loadi64 addr:$dst),
                                    (i8 imm:$src3)), addr:$dst)]>,
                 TB;
} // SchedRW

} // Defs = [EFLAGS], hasSideEffects
#endif // INTEL_FEATURE_ISA_APX_F
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_APX_F
multiclass opposite_rotate_r1_pattern<SDNode FromNode, I ToNode,
                                      bits<8> offset, RegisterClass RC>{
let Predicates = [NoNDD] in
def : Pat<(FromNode RC:$src1,  (i8 offset)),  (ToNode  RC:$src1)>;
let Predicates = [HasNDD, In64BitMode] in
def : Pat<(FromNode RC:$src1,  (i8 offset)),
          (!cast<I>(ToNode#_ND)  RC:$src1)>;
}


multiclass opposite_rotate_m1_pattern<SDNode FromNode, I ToNode, int offset,
                                      PatFrag LoadNode>{
let Predicates = [NoNDD] in
def : Pat<(store (FromNode (LoadNode addr:$dst), (i8 offset)), addr:$dst),
          (ToNode addr:$dst)>;
let Predicates = [HasNDD, In64BitMode] in
def : Pat<(FromNode (LoadNode addr:$src1), (i8 offset)),
          (!cast<I>(ToNode#_ND) addr:$src1)>;
}

defm : opposite_rotate_r1_pattern<rotl, ROR8r1 , 7,  GR8>;
defm : opposite_rotate_r1_pattern<rotl, ROR16r1, 15, GR16>;
defm : opposite_rotate_r1_pattern<rotl, ROR32r1, 31, GR32>;
defm : opposite_rotate_r1_pattern<rotl, ROR64r1, 63, GR64>;
defm : opposite_rotate_r1_pattern<rotr, ROL8r1 , 7,  GR8>;
defm : opposite_rotate_r1_pattern<rotr, ROL16r1, 15, GR16>;
defm : opposite_rotate_r1_pattern<rotr, ROL32r1, 31, GR32>;
defm : opposite_rotate_r1_pattern<rotr, ROL64r1, 63, GR64>;

defm : opposite_rotate_m1_pattern<rotl, ROR8m1, 7,  loadi8>;
defm : opposite_rotate_m1_pattern<rotl, ROR16m1, 15, loadi16>;
defm : opposite_rotate_m1_pattern<rotl, ROR32m1, 31, loadi32>;
defm : opposite_rotate_m1_pattern<rotl, ROR64m1, 63, loadi64>;
defm : opposite_rotate_m1_pattern<rotr, ROL8m1, 7,  loadi8>;
defm : opposite_rotate_m1_pattern<rotr, ROL16m1, 15, loadi16>;
defm : opposite_rotate_m1_pattern<rotr, ROL32m1, 31, loadi32>;
defm : opposite_rotate_m1_pattern<rotr, ROL64m1, 63, loadi64>;
#else // INTEL_FEATURE_ISA_APX_F
// Use the opposite rotate if allows us to use the rotate by 1 instruction.
def : Pat<(rotl GR8:$src1,  (i8 7)),  (ROR8r1  GR8:$src1)>;
def : Pat<(rotl GR16:$src1, (i8 15)), (ROR16r1 GR16:$src1)>;
def : Pat<(rotl GR32:$src1, (i8 31)), (ROR32r1 GR32:$src1)>;
def : Pat<(rotl GR64:$src1, (i8 63)), (ROR64r1 GR64:$src1)>;
def : Pat<(rotr GR8:$src1,  (i8 7)),  (ROL8r1  GR8:$src1)>;
def : Pat<(rotr GR16:$src1, (i8 15)), (ROL16r1 GR16:$src1)>;
def : Pat<(rotr GR32:$src1, (i8 31)), (ROL32r1 GR32:$src1)>;
def : Pat<(rotr GR64:$src1, (i8 63)), (ROL64r1 GR64:$src1)>;

def : Pat<(store (rotl (loadi8 addr:$dst), (i8 7)), addr:$dst),
          (ROR8m1 addr:$dst)>;
def : Pat<(store (rotl (loadi16 addr:$dst), (i8 15)), addr:$dst),
          (ROR16m1 addr:$dst)>;
def : Pat<(store (rotl (loadi32 addr:$dst), (i8 31)), addr:$dst),
          (ROR32m1 addr:$dst)>;
def : Pat<(store (rotl (loadi64 addr:$dst), (i8 63)), addr:$dst),
          (ROR64m1 addr:$dst)>, Requires<[In64BitMode]>;

def : Pat<(store (rotr (loadi8 addr:$dst), (i8 7)), addr:$dst),
          (ROL8m1 addr:$dst)>;
def : Pat<(store (rotr (loadi16 addr:$dst), (i8 15)), addr:$dst),
          (ROL16m1 addr:$dst)>;
def : Pat<(store (rotr (loadi32 addr:$dst), (i8 31)), addr:$dst),
          (ROL32m1 addr:$dst)>;
def : Pat<(store (rotr (loadi64 addr:$dst), (i8 63)), addr:$dst),
          (ROL64m1 addr:$dst)>, Requires<[In64BitMode]>;
#endif // INTEL_FEATURE_ISA_APX_F
#endif // INTEL_CUSTOMIZATION

// Sandy Bridge and newer Intel processors support faster rotates using
// SHLD to avoid a partial flag update on the normal rotate instructions.
// Use a pseudo so that TwoInstructionPass and register allocation will see
// this as unary instruction.
let Predicates = [HasFastSHLDRotate], AddedComplexity = 5,
    Defs = [EFLAGS], isPseudo = 1, SchedRW = [WriteSHDrri],
    Constraints = "$src1 = $dst" in {
  def SHLDROT32ri  : I<0, Pseudo, (outs GR32:$dst),
                       (ins GR32:$src1, u8imm:$shamt), "",
                     [(set GR32:$dst, (rotl GR32:$src1, (i8 imm:$shamt)))]>;
  def SHLDROT64ri  : I<0, Pseudo, (outs GR64:$dst),
                       (ins GR64:$src1, u8imm:$shamt), "",
                     [(set GR64:$dst, (rotl GR64:$src1, (i8 imm:$shamt)))]>;

  def SHRDROT32ri  : I<0, Pseudo, (outs GR32:$dst),
                       (ins GR32:$src1, u8imm:$shamt), "",
                     [(set GR32:$dst, (rotr GR32:$src1, (i8 imm:$shamt)))]>;
  def SHRDROT64ri  : I<0, Pseudo, (outs GR64:$dst),
                       (ins GR64:$src1, u8imm:$shamt), "",
                     [(set GR64:$dst, (rotr GR64:$src1, (i8 imm:$shamt)))]>;
}

def ROT32L2R_imm8  : SDNodeXForm<imm, [{
  // Convert a ROTL shamt to a ROTR shamt on 32-bit integer.
  return getI8Imm(32 - N->getZExtValue(), SDLoc(N));
}]>;

def ROT64L2R_imm8  : SDNodeXForm<imm, [{
  // Convert a ROTL shamt to a ROTR shamt on 64-bit integer.
  return getI8Imm(64 - N->getZExtValue(), SDLoc(N));
}]>;

// NOTE: We use WriteShift for these rotates as they avoid the stalls
// of many of the older x86 rotate instructions.
multiclass bmi_rotate<string asm, RegisterClass RC, X86MemOperand x86memop> {
let hasSideEffects = 0 in {
  def ri : Ii8<0xF0, MRMSrcReg, (outs RC:$dst), (ins RC:$src1, u8imm:$src2),
               !strconcat(asm, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"),
               []>, TAXD, VEX, Sched<[WriteShift]>;
  let mayLoad = 1 in
  def mi : Ii8<0xF0, MRMSrcMem, (outs RC:$dst),
               (ins x86memop:$src1, u8imm:$src2),
               !strconcat(asm, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"),
               []>, TAXD, VEX, Sched<[WriteShiftLd]>;
}
}

multiclass bmi_shift<string asm, RegisterClass RC, X86MemOperand x86memop> {
let hasSideEffects = 0 in {
  def rr : I<0xF7, MRMSrcReg4VOp3, (outs RC:$dst), (ins RC:$src1, RC:$src2),
             !strconcat(asm, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"), []>,
             VEX, Sched<[WriteShift]>;
  let mayLoad = 1 in
  def rm : I<0xF7, MRMSrcMem4VOp3,
             (outs RC:$dst), (ins x86memop:$src1, RC:$src2),
             !strconcat(asm, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"), []>,
             VEX, Sched<[WriteShift.Folded,
                         // x86memop:$src1
                         ReadDefault, ReadDefault, ReadDefault, ReadDefault,
                         ReadDefault,
                         // RC:$src2
                         WriteShift.ReadAfterFold]>;
}
}

let Predicates = [HasBMI2] in {
  defm RORX32 : bmi_rotate<"rorx{l}", GR32, i32mem>;
  defm RORX64 : bmi_rotate<"rorx{q}", GR64, i64mem>, REX_W;
  defm SARX32 : bmi_shift<"sarx{l}", GR32, i32mem>, T8XS;
  defm SARX64 : bmi_shift<"sarx{q}", GR64, i64mem>, T8XS, REX_W;
  defm SHRX32 : bmi_shift<"shrx{l}", GR32, i32mem>, T8XD;
  defm SHRX64 : bmi_shift<"shrx{q}", GR64, i64mem>, T8XD, REX_W;
  defm SHLX32 : bmi_shift<"shlx{l}", GR32, i32mem>, T8PD;
  defm SHLX64 : bmi_shift<"shlx{q}", GR64, i64mem>, T8PD, REX_W;

  // Prefer RORX which is non-destructive and doesn't update EFLAGS.
  let AddedComplexity = 10 in {
    def : Pat<(rotr GR32:$src, (i8 imm:$shamt)),
              (RORX32ri GR32:$src, imm:$shamt)>;
    def : Pat<(rotr GR64:$src, (i8 imm:$shamt)),
              (RORX64ri GR64:$src, imm:$shamt)>;
    def : Pat<(rotl GR32:$src, (i8 imm:$shamt)),
              (RORX32ri GR32:$src, (ROT32L2R_imm8 imm:$shamt))>;
    def : Pat<(rotl GR64:$src, (i8 imm:$shamt)),
              (RORX64ri GR64:$src, (ROT64L2R_imm8 imm:$shamt))>;
  }

  def : Pat<(rotr (loadi32 addr:$src), (i8 imm:$shamt)),
            (RORX32mi addr:$src, imm:$shamt)>;
  def : Pat<(rotr (loadi64 addr:$src), (i8 imm:$shamt)),
            (RORX64mi addr:$src, imm:$shamt)>;

  def : Pat<(rotl (loadi32 addr:$src), (i8 imm:$shamt)),
            (RORX32mi addr:$src, (ROT32L2R_imm8 imm:$shamt))>;
  def : Pat<(rotl (loadi64 addr:$src), (i8 imm:$shamt)),
            (RORX64mi addr:$src, (ROT64L2R_imm8 imm:$shamt))>;

  // Prefer SARX/SHRX/SHLX over SAR/SHR/SHL with variable shift BUT not
  // immediate shift, i.e. the following code is considered better
  //
  //  mov %edi, %esi
  //  shl $imm, %esi
  //  ... %edi, ...
  //
  // than
  //
  //  movb $imm, %sil
  //  shlx %sil, %edi, %esi
  //  ... %edi, ...
  //
  let AddedComplexity = 1 in {
    def : Pat<(sra GR32:$src1, GR8:$src2),
              (SARX32rr GR32:$src1,
                        (INSERT_SUBREG
                          (i32 (IMPLICIT_DEF)), GR8:$src2, sub_8bit))>;
    def : Pat<(sra GR64:$src1, GR8:$src2),
              (SARX64rr GR64:$src1,
                        (INSERT_SUBREG
                          (i64 (IMPLICIT_DEF)), GR8:$src2, sub_8bit))>;

    def : Pat<(srl GR32:$src1, GR8:$src2),
              (SHRX32rr GR32:$src1,
                        (INSERT_SUBREG
                          (i32 (IMPLICIT_DEF)), GR8:$src2, sub_8bit))>;
    def : Pat<(srl GR64:$src1, GR8:$src2),
              (SHRX64rr GR64:$src1,
                        (INSERT_SUBREG
                          (i64 (IMPLICIT_DEF)), GR8:$src2, sub_8bit))>;

    def : Pat<(shl GR32:$src1, GR8:$src2),
              (SHLX32rr GR32:$src1,
                        (INSERT_SUBREG
                          (i32 (IMPLICIT_DEF)), GR8:$src2, sub_8bit))>;
    def : Pat<(shl GR64:$src1, GR8:$src2),
              (SHLX64rr GR64:$src1,
                        (INSERT_SUBREG
                          (i64 (IMPLICIT_DEF)), GR8:$src2, sub_8bit))>;
  }

  // We prefer to use
  //  mov (%ecx), %esi
  //  shl $imm, $esi
  //
  // over
  //
  //  movb $imm, %al
  //  shlx %al, (%ecx), %esi
  //
  // This priority is enforced by IsProfitableToFoldLoad.
  def : Pat<(sra (loadi32 addr:$src1), GR8:$src2),
            (SARX32rm addr:$src1,
                      (INSERT_SUBREG
                        (i32 (IMPLICIT_DEF)), GR8:$src2, sub_8bit))>;
  def : Pat<(sra (loadi64 addr:$src1), GR8:$src2),
            (SARX64rm addr:$src1,
                      (INSERT_SUBREG
                        (i64 (IMPLICIT_DEF)), GR8:$src2, sub_8bit))>;

  def : Pat<(srl (loadi32 addr:$src1), GR8:$src2),
            (SHRX32rm addr:$src1,
                      (INSERT_SUBREG
                        (i32 (IMPLICIT_DEF)), GR8:$src2, sub_8bit))>;
  def : Pat<(srl (loadi64 addr:$src1), GR8:$src2),
            (SHRX64rm addr:$src1,
                      (INSERT_SUBREG
                        (i64 (IMPLICIT_DEF)), GR8:$src2, sub_8bit))>;

  def : Pat<(shl (loadi32 addr:$src1), GR8:$src2),
            (SHLX32rm addr:$src1,
                      (INSERT_SUBREG
                        (i32 (IMPLICIT_DEF)), GR8:$src2, sub_8bit))>;
  def : Pat<(shl (loadi64 addr:$src1), GR8:$src2),
            (SHLX64rm addr:$src1,
                      (INSERT_SUBREG
                        (i64 (IMPLICIT_DEF)), GR8:$src2, sub_8bit))>;
}

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_APX_F
multiclass SHXXri_pattern<SDNode FromNode, string ToNode, X86TypeInfo info>{
let Predicates = [NoNDD] in
  def : Pat<(FromNode info.RegClass:$src1, (i8 relocImm:$src2)),
            (!cast<Instruction>(ToNode) info.RegClass:$src1,
                                              relocImm:$src2)>;
let Predicates = [HasNDD, In64BitMode] in
  def : Pat<(FromNode info.RegClass:$src1, (i8 relocImm:$src2)),
            (!cast<Instruction>(ToNode#_ND) info.RegClass:$src1,
                                                  relocImm:$src2)>;
}
defm : SHXXri_pattern<rotl, "ROL8ri", Xi8>;
defm : SHXXri_pattern<rotl, "ROL16ri", Xi16>;
defm : SHXXri_pattern<rotl, "ROL32ri", Xi32>;
defm : SHXXri_pattern<rotl, "ROL64ri", Xi64>;

defm : SHXXri_pattern<rotr, "ROR8ri", Xi8>;
defm : SHXXri_pattern<rotr, "ROR16ri", Xi16>;
defm : SHXXri_pattern<rotr, "ROR32ri", Xi32>;
defm : SHXXri_pattern<rotr, "ROR64ri", Xi64>;
#else // INTEL_FEATURE_ISA_APX_F
def : Pat<(rotl GR8:$src1, (i8 relocImm:$src2)),
          (ROL8ri GR8:$src1, relocImm:$src2)>;
def : Pat<(rotl GR16:$src1, (i8 relocImm:$src2)),
          (ROL16ri GR16:$src1, relocImm:$src2)>;
def : Pat<(rotl GR32:$src1, (i8 relocImm:$src2)),
          (ROL32ri GR32:$src1, relocImm:$src2)>;
def : Pat<(rotl GR64:$src1, (i8 relocImm:$src2)),
          (ROL64ri GR64:$src1, relocImm:$src2)>;

def : Pat<(rotr GR8:$src1, (i8 relocImm:$src2)),
          (ROR8ri GR8:$src1, relocImm:$src2)>;
def : Pat<(rotr GR16:$src1, (i8 relocImm:$src2)),
          (ROR16ri GR16:$src1, relocImm:$src2)>;
def : Pat<(rotr GR32:$src1, (i8 relocImm:$src2)),
          (ROR32ri GR32:$src1, relocImm:$src2)>;
def : Pat<(rotr GR64:$src1, (i8 relocImm:$src2)),
          (ROR64ri GR64:$src1, relocImm:$src2)>;
#endif // INTEL_FEATURE_ISA_APX_F
#endif // INTEL_CUSTOMIZATION
