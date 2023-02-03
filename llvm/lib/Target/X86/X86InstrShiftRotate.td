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
multiclass shift_rCL<bits<8> opcode, Format f, string mnemonic, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst", Predicates = [TruePredicate] in
  def NAME  : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{%cl, $dst|$dst, cl}", 
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, CL))]>;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in 
  def NAME#_ND : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{%cl, $src1, $dst|$dst, $src1, cl}", 
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, CL))]>;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{%cl, $dst|$dst, cl}", 
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, CL))]>;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{%cl, $src1, $dst|$dst, $src1, cl}", 
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, CL))]>;
}

multiclass shiftl_ri<bits<8> opcode, Format f, string mnemonic, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst", Predicates = [TruePredicate], isConvertibleToThreeAddress = 1 in
  def NAME  : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1, u8imm:$src2),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src2, $dst|$dst, $src2}", 
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, (i8 imm:$src2)))]>;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in 
  def NAME#_ND : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1, u8imm:$src2),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src2, $src1, $dst|$dst, $src1, $src2}", 
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, (i8 imm:$src2)))]>;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1, u8imm:$src2),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src2, $dst|$dst, $src2}", 
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, (i8 imm:$src2)))]>;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1, u8imm:$src2),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src2, $src1, $dst|$dst, $src1, $src2}", 
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, (i8 imm:$src2)))]>;
}

multiclass shift_ri<bits<8> opcode, Format f, string mnemonic, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst", Predicates = [TruePredicate] in
  def NAME  : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1, u8imm:$src2),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src2, $dst|$dst, $src2}", 
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, (i8 imm:$src2)))]>;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in 
  def NAME#_ND : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1, u8imm:$src2),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src2, $src1, $dst|$dst, $src1, $src2}", 
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, (i8 imm:$src2)))]>;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1, u8imm:$src2),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src2, $dst|$dst, $src2}", 
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, (i8 imm:$src2)))]>;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1, u8imm:$src2),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src2, $src1, $dst|$dst, $src1, $src2}", 
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, (i8 imm:$src2)))]>;
}

multiclass shiftl_r1<bits<8> opcode, Format f, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst", Predicates = [TruePredicate] in
  def NAME  : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 "shl{"#typeinfo.InstrSuffix#"}\t$dst", []>;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in 
  def NAME#_ND : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 "shl{"#typeinfo.InstrSuffix#"}\t{$src1, $dst|$dst, $src1}", []>;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 "{nf}\tshl{"#typeinfo.InstrSuffix#"}\t$dst", []>;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 "{nf}\tshl{"#typeinfo.InstrSuffix#"}\t{$src1, $dst|$dst, $src1}", []>;
}

let Uses = [CL], SchedRW = [WriteShiftCL] in {
defm SHL8rCL  : shift_rCL<0xD2, MRM4r, "shl", shl, Xi8>;
defm SHL16rCL : shift_rCL<0xD3, MRM4r, "shl", shl, Xi16>, OpSize16;
defm SHL32rCL : shift_rCL<0xD3, MRM4r, "shl", shl, Xi32>, OpSize32;
defm SHL64rCL : shift_rCL<0xD3, MRM4r, "shl", shl, Xi64>, REX_W;
} // Uses = [CL], SchedRW

let SchedRW = [WriteShift]in {
defm SHL8ri   : shiftl_ri<0xC0, MRM4r, "shl", shl, Xi8>;
defm SHL16ri  : shiftl_ri<0xC1, MRM4r, "shl", shl, Xi16>, OpSize16;
defm SHL32ri  : shiftl_ri<0xC1, MRM4r, "shl", shl, Xi32>, OpSize32;
defm SHL64ri  : shiftl_ri<0xC1, MRM4r, "shl", shl, Xi64>, REX_W;
} // SchedRW

// NOTE: We don't include patterns for shifts of a register by one, because
// 'add reg,reg' is cheaper (and we have a Pat pattern for shift-by-one).
let hasSideEffects = 0, SchedRW = [WriteShift] in {
defm SHL8r1   : shiftl_r1<0xD0, MRM4r, Xi8>;
defm SHL16r1  : shiftl_r1<0xD1, MRM4r, Xi16>, OpSize16;
defm SHL32r1  : shiftl_r1<0xD1, MRM4r, Xi32>, OpSize32;
defm SHL64r1  : shiftl_r1<0xD1, MRM4r, Xi64>, REX_W;
} // hasSideEffects = 0

// FIXME: Why do we need an explicit "Uses = [CL]" when the instr has a pattern
// using CL?

multiclass shift_mCL<bits<8> opcode, Format f, string mnemonic, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
  def NAME  : I<opcode, f, (outs), (ins typeinfo.MemOperand :$dst),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{%cl, $dst|$dst, cl}", 
                 [(store (node (typeinfo.LoadNode addr:$dst), CL), addr:$dst)]>, 
                 Requires<!if(typeinfo.HasREX_W,[In64BitMode],[])>;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in 
  def NAME#_ND : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                (ins typeinfo.MemOperand :$src1),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{%cl, $src1, $dst|$dst, $src1, cl}", 
                 [(set typeinfo.RegClass:$dst, 
                 (node (typeinfo.LoadNode addr:$src1), CL))]>;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF  : I<opcode, f, (outs), (ins typeinfo.MemOperand :$dst),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{%cl, $dst|$dst, cl}", 
                 [(store (node (typeinfo.LoadNode addr:$dst), CL), addr:$dst)]>, 
                 Requires<!if(typeinfo.HasREX_W,[In64BitMode],[])>;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.MemOperand :$src1),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{%cl, $src1, $dst|$dst, $src1, cl}", 
                 [(set typeinfo.RegClass:$dst, 
                 (node (typeinfo.LoadNode addr:$src1), CL))]>;
}

let Uses = [CL], SchedRW = [WriteShiftCLLd, WriteRMW] in {
defm SHL8mCL  : shift_mCL<0xD2, MRM4m, "shl", shl, Xi8>;
defm SHL16mCL : shift_mCL<0xD3, MRM4m, "shl", shl, Xi16>, OpSize16;
defm SHL32mCL : shift_mCL<0xD3, MRM4m, "shl", shl, Xi32>, OpSize32;
defm SHL64mCL : shift_mCL<0xD3, MRM4m, "shl", shl, Xi64>, REX_W;
}

multiclass shift_mi<bits<8> opcode, Format f, string mnemonic, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Predicates = [TruePredicate] in
  def NAME  : Ii8<opcode, f, (outs), (ins typeinfo.MemOperand :$dst, u8imm:$src),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src, $dst|$dst, $src}", 
                 [(store (node (typeinfo.LoadNode addr:$dst), (i8 imm:$src)), addr:$dst)]>, 
                 Requires<!if(typeinfo.HasREX_W,[In64BitMode],[])>;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in 
  def NAME#_ND : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                (ins typeinfo.MemOperand :$src1, u8imm:$src2),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src2, $src1, $dst|$dst, $src1, $src2}", 
                 [(set typeinfo.RegClass:$dst, 
                 (node (typeinfo.LoadNode addr:$src1), (i8 imm:$src2)))]>;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF  : Ii8<opcode, f, (outs), (ins typeinfo.MemOperand :$dst, u8imm:$src),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src, $dst|$dst, $src}", 
                 [(store (node (typeinfo.LoadNode addr:$dst), (i8 imm:$src)), addr:$dst)]>, 
                 Requires<!if(typeinfo.HasREX_W,[In64BitMode],[])>;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.MemOperand :$src1, u8imm:$src2),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src2, $src1, $dst|$dst, $src1, $src2}", 
                 [(set typeinfo.RegClass:$dst, 
                 (node (typeinfo.LoadNode addr:$src1), (i8 imm:$src2)))]>;
}

multiclass shift_m1<bits<8> opcode, Format f, string mnemonic, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Predicates = [TruePredicate] in
  def NAME  : I<opcode, f, (outs), (ins typeinfo.MemOperand :$dst),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t$dst", 
                 [(store (node (typeinfo.LoadNode addr:$dst), (i8 1)), addr:$dst)]>, 
                 Requires<!if(typeinfo.HasREX_W,[In64BitMode],[])>;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in 
  def NAME#_ND : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                (ins typeinfo.MemOperand :$src1),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src1, $dst|$dst, $src1}", 
                 [(set typeinfo.RegClass:$dst, 
                 (node (typeinfo.LoadNode addr:$src1), (i8 1)))]>;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.MemOperand :$src1),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src1, $dst|$dst, $src1}", 
                 [(set typeinfo.RegClass:$dst, 
                 (node (typeinfo.LoadNode addr:$src1), (i8 1)))]>;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.MemOperand :$src1),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src1, $dst|$dst, $src1}", 
                 [(set typeinfo.RegClass:$dst, 
                 (node (typeinfo.LoadNode addr:$src1), (i8 1)))]>;
}

let SchedRW = [WriteShiftLd, WriteRMW] in {
defm SHL8mi   : shift_mi<0xC0, MRM4m, "shl", shl, Xi8>;
defm SHL16mi  : shift_mi<0xC1, MRM4m, "shl", shl, Xi16>, OpSize16;
defm SHL32mi  : shift_mi<0xC1, MRM4m, "shl", shl, Xi32>, OpSize32;
defm SHL64mi  : shift_mi<0xC1, MRM4m, "shl", shl, Xi64>, REX_W;

// Shift by 1
defm SHL8m1   : shift_m1<0xD0, MRM4m, "shl", shl, Xi8>;
defm SHL16m1  : shift_m1<0xD1, MRM4m, "shl", shl, Xi16>, OpSize16;
defm SHL32m1  : shift_m1<0xD1, MRM4m, "shl", shl, Xi32>, OpSize32;
defm SHL64m1  : shift_m1<0xD1, MRM4m, "shl", shl, Xi64>, REX_W;
} // SchedRW

//=======================SHR==========================
multiclass shift_r1<bits<8> opcode, Format f, string mnemonic, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst", Predicates = [TruePredicate] in
  def NAME  : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t$dst", 
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, (i8 1)))]>;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in 
  def NAME#_ND : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src1, $dst|$dst, $src1}", 
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, (i8 1)))]>;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t$dst", 
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, (i8 1)))]>;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src1, $dst|$dst, $src1}", 
                 [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, (i8 1)))]>;
}

let Uses = [CL], SchedRW = [WriteShiftCL] in {
defm SHR8rCL  : shift_rCL<0xD2, MRM5r, "shr", srl, Xi8>;
defm SHR16rCL : shift_rCL<0xD3, MRM5r, "shr", srl, Xi16>, OpSize16;
defm SHR32rCL : shift_rCL<0xD3, MRM5r, "shr", srl, Xi32>, OpSize32;
defm SHR64rCL : shift_rCL<0xD3, MRM5r, "shr", srl, Xi64>, REX_W;
} // Uses = [CL], SchedRW

let SchedRW = [WriteShift]in {
defm SHR8ri   : shift_ri<0xC0, MRM5r, "shr", srl, Xi8>;
defm SHR16ri  : shift_ri<0xC1, MRM5r, "shr", srl, Xi16>, OpSize16;
defm SHR32ri  : shift_ri<0xC1, MRM5r, "shr", srl, Xi32>, OpSize32;
defm SHR64ri  : shift_ri<0xC1, MRM5r, "shr", srl, Xi64>, REX_W;

defm SHR8r1   : shift_r1<0xD0, MRM5r, "shr", srl, Xi8>;
defm SHR16r1  : shift_r1<0xD1, MRM5r, "shr", srl, Xi16>, OpSize16;
defm SHR32r1  : shift_r1<0xD1, MRM5r, "shr", srl, Xi32>, OpSize32;
defm SHR64r1  : shift_r1<0xD1, MRM5r, "shr", srl, Xi64>, REX_W;
} // SchedRW

let Uses = [CL], SchedRW = [WriteShiftCLLd, WriteRMW] in {
defm SHR8mCL  : shift_mCL<0xD2, MRM5m, "shr", srl, Xi8>;
defm SHR16mCL : shift_mCL<0xD3, MRM5m, "shr", srl, Xi16>, OpSize16;
defm SHR32mCL : shift_mCL<0xD3, MRM5m, "shr", srl, Xi32>, OpSize32;
defm SHR64mCL : shift_mCL<0xD3, MRM5m, "shr", srl, Xi64>, REX_W;
}

let SchedRW = [WriteShiftLd, WriteRMW] in {
defm SHR8mi   : shift_mi<0xC0, MRM5m, "shr", srl, Xi8>;
defm SHR16mi  : shift_mi<0xC1, MRM5m, "shr", srl, Xi16>, OpSize16;
defm SHR32mi  : shift_mi<0xC1, MRM5m, "shr", srl, Xi32>, OpSize32;
defm SHR64mi  : shift_mi<0xC1, MRM5m, "shr", srl, Xi64>, REX_W;

// Shift by 1
defm SHR8m1   : shift_m1<0xD0, MRM5m, "shr", srl, Xi8>;
defm SHR16m1  : shift_m1<0xD1, MRM5m, "shr", srl, Xi16>, OpSize16;
defm SHR32m1  : shift_m1<0xD1, MRM5m, "shr", srl, Xi32>, OpSize32;
defm SHR64m1  : shift_m1<0xD1, MRM5m, "shr", srl, Xi64>, REX_W;
} // SchedRW

//====================SAR=======================
let Uses = [CL], SchedRW = [WriteShiftCL] in {
defm SAR8rCL  : shift_rCL<0xD2, MRM7r, "sar", sra, Xi8>;
defm SAR16rCL : shift_rCL<0xD3, MRM7r, "sar", sra, Xi16>, OpSize16;
defm SAR32rCL : shift_rCL<0xD3, MRM7r, "sar", sra, Xi32>, OpSize32;
defm SAR64rCL : shift_rCL<0xD3, MRM7r, "sar", sra, Xi64>, REX_W;
} // Uses = [CL], SchedRW

let SchedRW = [WriteShift]in {
defm SAR8ri   : shift_ri<0xC0, MRM7r, "sar", sra, Xi8>;
defm SAR16ri  : shift_ri<0xC1, MRM7r, "sar", sra, Xi16>, OpSize16;
defm SAR32ri  : shift_ri<0xC1, MRM7r, "sar", sra, Xi32>, OpSize32;
defm SAR64ri  : shift_ri<0xC1, MRM7r, "sar", sra, Xi64>, REX_W;
} // SchedRW

// NOTE: We don't include patterns for shifts of a register by one, because
// 'add reg,reg' is cheaper (and we have a Pat pattern for shift-by-one).
let SchedRW = [WriteShift] in {
defm SAR8r1   : shift_r1<0xD0, MRM7r, "sar", sra, Xi8>;
defm SAR16r1  : shift_r1<0xD1, MRM7r, "sar", sra, Xi16>, OpSize16;
defm SAR32r1  : shift_r1<0xD1, MRM7r, "sar", sra, Xi32>, OpSize32;
defm SAR64r1  : shift_r1<0xD1, MRM7r, "sar", sra, Xi64>, REX_W;
} // hasSideEffects = 0

let Uses = [CL], SchedRW = [WriteShiftCLLd, WriteRMW] in {
defm SAR8mCL  : shift_mCL<0xD2, MRM7m, "sar", sra, Xi8>;
defm SAR16mCL : shift_mCL<0xD3, MRM7m, "sar", sra, Xi16>, OpSize16;
defm SAR32mCL : shift_mCL<0xD3, MRM7m, "sar", sra, Xi32>, OpSize32;
defm SAR64mCL : shift_mCL<0xD3, MRM7m, "sar", sra, Xi64>, REX_W;
}

let SchedRW = [WriteShiftLd, WriteRMW] in {
defm SAR8mi   : shift_mi<0xC0, MRM7m, "sar", sra, Xi8>;
defm SAR16mi  : shift_mi<0xC1, MRM7m, "sar", sra, Xi16>, OpSize16;
defm SAR32mi  : shift_mi<0xC1, MRM7m, "sar", sra, Xi32>, OpSize32;
defm SAR64mi  : shift_mi<0xC1, MRM7m, "sar", sra, Xi64>, REX_W;

// Shift by 1
defm SAR8m1   : shift_m1<0xD0, MRM7m, "sar", sra, Xi8>;
defm SAR16m1  : shift_m1<0xD1, MRM7m, "sar", sra, Xi16>, OpSize16;
defm SAR32m1  : shift_m1<0xD1, MRM7m, "sar", sra, Xi32>, OpSize32;
defm SAR64m1  : shift_m1<0xD1, MRM7m, "sar", sra, Xi64>, REX_W;
} // SchedRW

//===----------------------------------------------------------------------===//
// Rotate instructions
//===----------------------------------------------------------------------===//

multiclass shift_rCL_NoPat<bits<8> opcode, Format f, string mnemonic, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst", Predicates = [TruePredicate] in
  def NAME  : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{%cl, $dst|$dst, cl}", []>;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in 
  def NAME#_ND : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{%cl, $src1, $dst|$dst, $src1, cl}", []>;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{%cl, $dst|$dst, cl}", []>;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{%cl, $src1, $dst|$dst, $src1, cl}", []>;
}

multiclass shift_ri_NoPat<bits<8> opcode, Format f, string mnemonic, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst", Predicates = [TruePredicate] in
  def NAME  : Ii8<opcode, f, (outs typeinfo.RegClass:$dst),  
                  (ins typeinfo.RegClass:$src1, u8imm:$cnt),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$cnt, $dst|$dst, $cnt}", []>;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in 
  def NAME#_ND : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1, u8imm:$cnt),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$cnt, $src1, $dst|$dst, $src1, $cnt}", []>;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1, u8imm:$cnt),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$cnt, $dst|$dst, $cnt}", []>;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1, u8imm:$cnt),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$cnt, $src1, $dst|$dst, $src1, $cnt}", []>;
}

multiclass shift_r1_NoPat<bits<8> opcode, Format f, string mnemonic, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst", Predicates = [TruePredicate] in
  def NAME  : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t$dst", []>;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in 
  def NAME#_ND : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src1, $dst|$dst, $src1}", []>;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t$dst", []>;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.RegClass:$src1),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src1, $dst|$dst, $src1}", []>;
}

multiclass shift_mCL_NoPat<bits<8> opcode, Format f, string mnemonic, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Predicates = [TruePredicate] in
  def NAME  : I<opcode, f, (outs), (ins typeinfo.MemOperand :$dst),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{%cl, $dst|$dst, cl}", 
                 []>, Requires<!if(typeinfo.HasREX_W,[In64BitMode],[])>;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in 
  def NAME#_ND : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                (ins typeinfo.MemOperand :$src1),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{%cl, $src1, $dst|$dst, $src1, cl}", []>;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF  : I<opcode, f, (outs), (ins typeinfo.MemOperand :$dst),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{%cl, $dst|$dst, cl}", 
                 []>, Requires<!if(typeinfo.HasREX_W,[In64BitMode],[])>;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.MemOperand :$src1),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{%cl, $src1, $dst|$dst, $src1, cl}", []>;
}

multiclass shift_mi_NoPat<bits<8> opcode, Format f, string mnemonic, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Predicates = [TruePredicate] in
  def NAME  : Ii8<opcode, f, (outs), (ins typeinfo.MemOperand :$dst, u8imm:$cnt),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$cnt, $dst|$dst, $cnt}", 
                 []>, Requires<!if(typeinfo.HasREX_W,[In64BitMode],[])>;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in 
  def NAME#_ND : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                (ins typeinfo.MemOperand :$src1, u8imm:$cnt),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$cnt, $src1, $dst|$dst, $src1, $cnt}", []>;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF  : Ii8<opcode, f, (outs), (ins typeinfo.MemOperand :$dst, u8imm:$cnt),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$cnt, $dst|$dst, $cnt}", 
                 []>, Requires<!if(typeinfo.HasREX_W,[In64BitMode],[])>;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.MemOperand :$src1, u8imm:$cnt),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$cnt, $src1, $dst|$dst, $src1, $cnt}", []>;
}

multiclass shift_m1_NoPat<bits<8> opcode, Format f, string mnemonic, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Predicates = [TruePredicate] in
  def NAME  : I<opcode, f, (outs), (ins typeinfo.MemOperand :$dst),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t$dst", []>, 
                 Requires<!if(typeinfo.HasREX_W,[In64BitMode],[])>;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in 
  def NAME#_ND : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.MemOperand :$src1),
                 mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src1, $dst|$dst, $src1}", []>;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF  : I<opcode, f, (outs), (ins typeinfo.MemOperand :$dst),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t$dst", []>, 
                 Requires<!if(typeinfo.HasREX_W,[In64BitMode],[])>;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                  (ins typeinfo.MemOperand :$src1),
                 "{nf}\t"#mnemonic#"{"#typeinfo.InstrSuffix#"}\t{$src1, $dst|$dst, $src1}", []>;
}

let hasSideEffects = 0 in {
let Uses = [CL, EFLAGS], SchedRW = [WriteRotateCL] in {
defm RCL8rCL  : shift_rCL_NoPat<0xD2, MRM2r, "rcl", Xi8>;
defm RCL16rCL : shift_rCL_NoPat<0xD3, MRM2r, "rcl", Xi16>, OpSize16;
defm RCL32rCL : shift_rCL_NoPat<0xD3, MRM2r, "rcl", Xi32>, OpSize32;
defm RCL64rCL : shift_rCL_NoPat<0xD3, MRM2r, "rcl", Xi64>, REX_W;
} // Uses = [CL, EFLAGS], SchedRW

let Uses = [EFLAGS], SchedRW = [WriteRotate]in {
defm RCL8ri   : shift_ri_NoPat<0xC0, MRM2r, "rcl", Xi8>;
defm RCL16ri  : shift_ri_NoPat<0xC1, MRM2r, "rcl", Xi16>, OpSize16;
defm RCL32ri  : shift_ri_NoPat<0xC1, MRM2r, "rcl", Xi32>, OpSize32;
defm RCL64ri  : shift_ri_NoPat<0xC1, MRM2r, "rcl", Xi64>, REX_W;
} // SchedRW

let Uses = [EFLAGS], SchedRW = [WriteRotate] in {
defm RCL8r1   : shift_r1_NoPat<0xD0, MRM2r, "rcl", Xi8>;
defm RCL16r1  : shift_r1_NoPat<0xD1, MRM2r, "rcl", Xi16>, OpSize16;
defm RCL32r1  : shift_r1_NoPat<0xD1, MRM2r, "rcl", Xi32>, OpSize32;
defm RCL64r1  : shift_r1_NoPat<0xD1, MRM2r, "rcl", Xi64>, REX_W;
} // hasSideEffects = 0

let mayStore = 1 in {
let Uses = [CL, EFLAGS], SchedRW = [WriteRotateCLLd, WriteRMW] in {
defm RCL8mCL  : shift_mCL_NoPat<0xD2, MRM2m, "rcl", Xi8>;
defm RCL16mCL : shift_mCL_NoPat<0xD3, MRM2m, "rcl", Xi16>, OpSize16;
defm RCL32mCL : shift_mCL_NoPat<0xD3, MRM2m, "rcl", Xi32>, OpSize32;
defm RCL64mCL : shift_mCL_NoPat<0xD3, MRM2m, "rcl", Xi64>, REX_W;
}

let Uses = [EFLAGS], SchedRW = [WriteRotateLd, WriteRMW] in {
defm RCL8mi   : shift_mi_NoPat<0xC0, MRM2m, "rcl", Xi8>;
defm RCL16mi  : shift_mi_NoPat<0xC1, MRM2m, "rcl", Xi16>, OpSize16;
defm RCL32mi  : shift_mi_NoPat<0xC1, MRM2m, "rcl", Xi32>, OpSize32;
defm RCL64mi  : shift_mi_NoPat<0xC1, MRM2m, "rcl", Xi64>, REX_W;

// Shift by 1
defm RCL8m1   : shift_m1_NoPat<0xD0, MRM2m, "rcl", Xi8>;
defm RCL16m1  : shift_m1_NoPat<0xD1, MRM2m, "rcl", Xi16>, OpSize16;
defm RCL32m1  : shift_m1_NoPat<0xD1, MRM2m, "rcl", Xi32>, OpSize32;
defm RCL64m1  : shift_m1_NoPat<0xD1, MRM2m, "rcl", Xi64>, REX_W;
} // SchedRW
} // mayStore

// =====================RCR=========================

let Uses = [CL, EFLAGS], SchedRW = [WriteRotateCL] in {
defm RCR8rCL  : shift_rCL_NoPat<0xD2, MRM3r, "rcr", Xi8>;
defm RCR16rCL : shift_rCL_NoPat<0xD3, MRM3r, "rcr", Xi16>, OpSize16;
defm RCR32rCL : shift_rCL_NoPat<0xD3, MRM3r, "rcr", Xi32>, OpSize32;
defm RCR64rCL : shift_rCL_NoPat<0xD3, MRM3r, "rcr", Xi64>, REX_W;
} // Uses = [CL, EFLAGS], SchedRW

let Uses = [EFLAGS], SchedRW = [WriteRotate]in {
defm RCR8ri   : shift_ri_NoPat<0xC0, MRM3r, "rcr", Xi8>;
defm RCR16ri  : shift_ri_NoPat<0xC1, MRM3r, "rcr", Xi16>, OpSize16;
defm RCR32ri  : shift_ri_NoPat<0xC1, MRM3r, "rcr", Xi32>, OpSize32;
defm RCR64ri  : shift_ri_NoPat<0xC1, MRM3r, "rcr", Xi64>, REX_W;
} // SchedRW

let Uses = [EFLAGS], SchedRW = [WriteRotate] in {
defm RCR8r1   : shift_r1_NoPat<0xD0, MRM3r, "rcr", Xi8>;
defm RCR16r1  : shift_r1_NoPat<0xD1, MRM3r, "rcr", Xi16>, OpSize16;
defm RCR32r1  : shift_r1_NoPat<0xD1, MRM3r, "rcr", Xi32>, OpSize32;
defm RCR64r1  : shift_r1_NoPat<0xD1, MRM3r, "rcr", Xi64>, REX_W;
} // hasSideEffects = 0

let mayStore = 1 in {
let Uses = [CL, EFLAGS], SchedRW = [WriteRotateCLLd, WriteRMW] in {
defm RCR8mCL  : shift_mCL_NoPat<0xD2, MRM3m, "rcr", Xi8>;
defm RCR16mCL : shift_mCL_NoPat<0xD3, MRM3m, "rcr", Xi16>, OpSize16;
defm RCR32mCL : shift_mCL_NoPat<0xD3, MRM3m, "rcr", Xi32>, OpSize32;
defm RCR64mCL : shift_mCL_NoPat<0xD3, MRM3m, "rcr", Xi64>, REX_W;
}

let Uses = [EFLAGS], SchedRW = [WriteRotateLd, WriteRMW] in {
defm RCR8mi   : shift_mi_NoPat<0xC0, MRM3m, "rcr", Xi8>;
defm RCR16mi  : shift_mi_NoPat<0xC1, MRM3m, "rcr", Xi16>, OpSize16;
defm RCR32mi  : shift_mi_NoPat<0xC1, MRM3m, "rcr", Xi32>, OpSize32;
defm RCR64mi  : shift_mi_NoPat<0xC1, MRM3m, "rcr", Xi64>, REX_W;

// Shift by 1
defm RCR8m1   : shift_m1_NoPat<0xD0, MRM3m, "rcr", Xi8>;
defm RCR16m1  : shift_m1_NoPat<0xD1, MRM3m, "rcr", Xi16>, OpSize16;
defm RCR32m1  : shift_m1_NoPat<0xD1, MRM3m, "rcr", Xi32>, OpSize32;
defm RCR64m1  : shift_m1_NoPat<0xD1, MRM3m, "rcr", Xi64>, REX_W;
} // SchedRW
} // mayStore
} // hasSideEffects = 0

// ================== ROL =======================
let Uses = [CL], SchedRW = [WriteRotateCL] in {
defm ROL8rCL  : shift_rCL<0xD2, MRM0r, "rol", rotl, Xi8>;
defm ROL16rCL : shift_rCL<0xD3, MRM0r, "rol", rotl, Xi16>, OpSize16;
defm ROL32rCL : shift_rCL<0xD3, MRM0r, "rol", rotl, Xi32>, OpSize32;
defm ROL64rCL : shift_rCL<0xD3, MRM0r, "rol", rotl, Xi64>, REX_W;
} // Uses = [CL], SchedRW

let SchedRW = [WriteRotate]in {
defm ROL8ri   : shift_ri<0xC0, MRM0r, "rol", rotl, Xi8>;
defm ROL16ri  : shift_ri<0xC1, MRM0r, "rol", rotl, Xi16>, OpSize16;
defm ROL32ri  : shift_ri<0xC1, MRM0r, "rol", rotl, Xi32>, OpSize32;
defm ROL64ri  : shift_ri<0xC1, MRM0r, "rol", rotl, Xi64>, REX_W;
} // SchedRW

let SchedRW = [WriteRotate] in {
defm ROL8r1   : shift_r1<0xD0, MRM0r, "rol", rotl, Xi8>;
defm ROL16r1  : shift_r1<0xD1, MRM0r, "rol", rotl, Xi16>, OpSize16;
defm ROL32r1  : shift_r1<0xD1, MRM0r, "rol", rotl, Xi32>, OpSize32;
defm ROL64r1  : shift_r1<0xD1, MRM0r, "rol", rotl, Xi64>, REX_W;
} // hasSideEffects = 0

let Uses = [CL], SchedRW = [WriteRotateCLLd, WriteRMW] in {
defm ROL8mCL  : shift_mCL<0xD2, MRM0m, "rol", rotl, Xi8>;
defm ROL16mCL : shift_mCL<0xD3, MRM0m, "rol", rotl, Xi16>, OpSize16;
defm ROL32mCL : shift_mCL<0xD3, MRM0m, "rol", rotl, Xi32>, OpSize32;
defm ROL64mCL : shift_mCL<0xD3, MRM0m, "rol", rotl, Xi64>, REX_W;
}

let SchedRW = [WriteRotateLd, WriteRMW] in {
defm ROL8mi   : shift_mi<0xC0, MRM0m, "rol", rotl, Xi8>;
defm ROL16mi  : shift_mi<0xC1, MRM0m, "rol", rotl, Xi16>, OpSize16;
defm ROL32mi  : shift_mi<0xC1, MRM0m, "rol", rotl, Xi32>, OpSize32;
defm ROL64mi  : shift_mi<0xC1, MRM0m, "rol", rotl, Xi64>, REX_W;

// Shift by 1
defm ROL8m1   : shift_m1<0xD0, MRM0m, "rol", rotl, Xi8>;
defm ROL16m1  : shift_m1<0xD1, MRM0m, "rol", rotl, Xi16>, OpSize16;
defm ROL32m1  : shift_m1<0xD1, MRM0m, "rol", rotl, Xi32>, OpSize32;
defm ROL64m1  : shift_m1<0xD1, MRM0m, "rol", rotl, Xi64>, REX_W;
} // SchedRW

// ================== ROL =======================
let Uses = [CL], SchedRW = [WriteRotateCL] in {
defm ROR8rCL  : shift_rCL<0xD2, MRM1r, "ror", rotr, Xi8>;
defm ROR16rCL : shift_rCL<0xD3, MRM1r, "ror", rotr, Xi16>, OpSize16;
defm ROR32rCL : shift_rCL<0xD3, MRM1r, "ror", rotr, Xi32>, OpSize32;
defm ROR64rCL : shift_rCL<0xD3, MRM1r, "ror", rotr, Xi64>, REX_W;
} // Uses = [CL], SchedRW

let SchedRW = [WriteRotate]in {
defm ROR8ri   : shift_ri<0xC0, MRM1r, "ror", rotr, Xi8>;
defm ROR16ri  : shift_ri<0xC1, MRM1r, "ror", rotr, Xi16>, OpSize16;
defm ROR32ri  : shift_ri<0xC1, MRM1r, "ror", rotr, Xi32>, OpSize32;
defm ROR64ri  : shift_ri<0xC1, MRM1r, "ror", rotr, Xi64>, REX_W;
} // SchedRW

let SchedRW = [WriteRotate] in {
defm ROR8r1   : shift_r1<0xD0, MRM1r, "ror", rotr, Xi8>;
defm ROR16r1  : shift_r1<0xD1, MRM1r, "ror", rotr, Xi16>, OpSize16;
defm ROR32r1  : shift_r1<0xD1, MRM1r, "ror", rotr, Xi32>, OpSize32;
defm ROR64r1  : shift_r1<0xD1, MRM1r, "ror", rotr, Xi64>, REX_W;
} // hasSideEffects = 0

let Uses = [CL], SchedRW = [WriteRotateCLLd, WriteRMW] in {
defm ROR8mCL  : shift_mCL<0xD2, MRM1m, "ror", rotr, Xi8>;
defm ROR16mCL : shift_mCL<0xD3, MRM1m, "ror", rotr, Xi16>, OpSize16;
defm ROR32mCL : shift_mCL<0xD3, MRM1m, "ror", rotr, Xi32>, OpSize32;
defm ROR64mCL : shift_mCL<0xD3, MRM1m, "ror", rotr, Xi64>, REX_W;
}

let SchedRW = [WriteRotateLd, WriteRMW] in {
defm ROR8mi   : shift_mi<0xC0, MRM1m, "ror", rotr, Xi8>;
defm ROR16mi  : shift_mi<0xC1, MRM1m, "ror", rotr, Xi16>, OpSize16;
defm ROR32mi  : shift_mi<0xC1, MRM1m, "ror", rotr, Xi32>, OpSize32;
defm ROR64mi  : shift_mi<0xC1, MRM1m, "ror", rotr, Xi64>, REX_W;

// Shift by 1
defm ROR8m1   : shift_m1<0xD0, MRM1m, "ror", rotr, Xi8>;
defm ROR16m1  : shift_m1<0xD1, MRM1m, "ror", rotr, Xi16>, OpSize16;
defm ROR32m1  : shift_m1<0xD1, MRM1m, "ror", rotr, Xi32>, OpSize32;
defm ROR64m1  : shift_m1<0xD1, MRM1m, "ror", rotr, Xi64>, REX_W;
} // SchedRW

//===----------------------------------------------------------------------===//
// Double shift instructions (generalizations of rotate)
//===----------------------------------------------------------------------===//

multiclass shiftLD_rrCL<bits<8> opcode, Format f, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst", Predicates = [TruePredicate] in
  def NAME : I<opcode, f, (outs typeinfo.RegClass:$dst),
              (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2),
              "shld{"#typeinfo.InstrSuffix#"}\t{%cl, $src2, $dst|$dst, $src2, cl}",
              [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, 
                                              typeinfo.RegClass:$src2, CL))]>, TB;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in
  def NAME#_ND : I<opcode, f, (outs typeinfo.RegClass:$dst),
                  (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2),
                  "shld{"#typeinfo.InstrSuffix#"}\t{%cl, $src2, $src1, $dst|$dst, $src1, $src2, cl}",
                  [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, 
                                                  typeinfo.RegClass:$src2, CL))]>, TB;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : I<opcode, f, (outs typeinfo.RegClass:$dst),
                  (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2),
                  "{nf}\tshld{"#typeinfo.InstrSuffix#"}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                  [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, 
                                                  typeinfo.RegClass:$src2, CL))]>, TB;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : I<opcode, f, (outs typeinfo.RegClass:$dst),
                  (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2),
                  "{nf}\tshld{"#typeinfo.InstrSuffix#"}\t{%cl, $src2, $src1, $dst|$dst, $src1, $src2, cl}",
                  [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, 
                                                  typeinfo.RegClass:$src2, CL))]>, TB;
}

multiclass shiftRD_rrCL<bits<8> opcode, Format f, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst", Predicates = [TruePredicate] in
  def NAME : I<opcode, f, (outs typeinfo.RegClass:$dst),
              (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2),
              "shrd{"#typeinfo.InstrSuffix#"}\t{%cl, $src2, $dst|$dst, $src2, cl}",
              [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src2, 
                                              typeinfo.RegClass:$src1, CL))]>, TB;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in
  def NAME#_ND : I<opcode, f, (outs typeinfo.RegClass:$dst),
                  (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2),
                  "shrd{"#typeinfo.InstrSuffix#"}\t{%cl, $src2, $src1, $dst|$dst, $src1, $src2, cl}",
                  [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src2, 
                                                  typeinfo.RegClass:$src1, CL))]>, TB;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : I<opcode, f, (outs typeinfo.RegClass:$dst),
                  (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2),
                  "{nf}\tshrd{"#typeinfo.InstrSuffix#"}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                  [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src2, 
                                                  typeinfo.RegClass:$src1, CL))]>, TB;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : I<opcode, f, (outs typeinfo.RegClass:$dst),
                  (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2),
                  "{nf}\tshrd{"#typeinfo.InstrSuffix#"}\t{%cl, $src2, $src1, $dst|$dst, $src1, $src2, cl}",
                  [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src2, 
                                                  typeinfo.RegClass:$src1, CL))]>, TB;
}

let Uses = [CL], SchedRW = [WriteSHDrrcl] in {
defm SHLD16rrCL : shiftLD_rrCL<0xA5, MRMDestReg, X86fshl, Xi16>, OpSize16;
defm SHLD32rrCL : shiftLD_rrCL<0xA5, MRMDestReg, fshl, Xi32>, OpSize32;
defm SHLD64rrCL : shiftLD_rrCL<0xA5, MRMDestReg, fshl, Xi64>, REX_W;

defm SHRD16rrCL : shiftRD_rrCL<0xAD, MRMDestReg, X86fshr, Xi16>, OpSize16;
defm SHRD32rrCL : shiftRD_rrCL<0xAD, MRMDestReg, fshr, Xi32>, OpSize32;
defm SHRD64rrCL : shiftRD_rrCL<0xAD, MRMDestReg, fshr, Xi64>, REX_W;
}

multiclass shiftLD_rri8<bits<8> opcode, Format f, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst", Predicates = [TruePredicate] in
  def NAME : Ii8<opcode, f, (outs typeinfo.RegClass:$dst),
              (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2, u8imm:$src3),
              "shld{"#typeinfo.InstrSuffix#"}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
              [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, 
                                              typeinfo.RegClass:$src2, (i8 imm:$src3)))]>, TB;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in
  def NAME#_ND : Ii8<opcode, f, (outs typeinfo.RegClass:$dst),
                  (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2, u8imm:$src3),
                  "shld{"#typeinfo.InstrSuffix#"}\t{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                  [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, 
                                                  typeinfo.RegClass:$src2, (i8 imm:$src3)))]>, TB;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : Ii8<opcode, f, (outs typeinfo.RegClass:$dst),
                  (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2, u8imm:$src3),
                  "{nf}\tshld{"#typeinfo.InstrSuffix#"}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                  [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, 
                                                  typeinfo.RegClass:$src2, (i8 imm:$src3)))]>, TB;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : Ii8<opcode, f, (outs typeinfo.RegClass:$dst),
                  (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2, u8imm:$src3),
                  "{nf}\tshld{"#typeinfo.InstrSuffix#"}\t{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                  [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src1, 
                                                  typeinfo.RegClass:$src2, (i8 imm:$src3)))]>, TB;
}

multiclass shiftRD_rri8<bits<8> opcode, Format f, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst", Predicates = [TruePredicate] in
  def NAME : Ii8<opcode, f, (outs typeinfo.RegClass:$dst),
              (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2, u8imm:$src3),
              "shrd{"#typeinfo.InstrSuffix#"}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
              [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src2, 
                                              typeinfo.RegClass:$src1, (i8 imm:$src3)))]>, TB;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in
  def NAME#_ND : Ii8<opcode, f, (outs typeinfo.RegClass:$dst),
                  (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2, u8imm:$src3),
                  "shrd{"#typeinfo.InstrSuffix#"}\t{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                  [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src2, 
                                                  typeinfo.RegClass:$src1, (i8 imm:$src3)))]>, TB;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : Ii8<opcode, f, (outs typeinfo.RegClass:$dst),
                  (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2, u8imm:$src3),
                  "{nf}\tshrd{"#typeinfo.InstrSuffix#"}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                  [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src2, 
                                                  typeinfo.RegClass:$src1, (i8 imm:$src3)))]>, TB;
let Predicates = [FalsePredicateNDD, FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : Ii8<opcode, f, (outs typeinfo.RegClass:$dst),
                  (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2, u8imm:$src3),
                  "{nf}\tshrd{"#typeinfo.InstrSuffix#"}\t{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                  [(set typeinfo.RegClass:$dst, (node typeinfo.RegClass:$src2, 
                                                  typeinfo.RegClass:$src1, (i8 imm:$src3)))]>, TB;
}

let isCommutable = 1, SchedRW = [WriteSHDrri] in {
defm SHLD16rri8 : shiftLD_rri8<0xA4, MRMDestReg, X86fshl, Xi16>, OpSize16;
defm SHLD32rri8 : shiftLD_rri8<0xA4, MRMDestReg, fshl, Xi32>, OpSize32;
defm SHLD64rri8 : shiftLD_rri8<0xA4, MRMDestReg, fshl, Xi64>, REX_W;

defm SHRD16rri8 : shiftRD_rri8<0xAC, MRMDestReg, X86fshr, Xi16>, OpSize16;
defm SHRD32rri8 : shiftRD_rri8<0xAC, MRMDestReg, fshr, Xi32>, OpSize32;
defm SHRD64rri8 : shiftRD_rri8<0xAC, MRMDestReg, fshr, Xi64>, REX_W;
}

multiclass shiftLD_mrCL<bits<8> opcode, Format f, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Predicates = [TruePredicate] in
  def NAME : I<opcode, f, (outs), (ins typeinfo.MemOperand:$dst, 
                                            typeinfo.RegClass:$src2),
                "shld{"#typeinfo.InstrSuffix#"}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                [(store (node (typeinfo.LoadNode addr:$dst), 
                typeinfo.RegClass:$src2, CL), addr:$dst)]>, TB;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in
  def NAME#_ND : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                (ins typeinfo.MemOperand:$src1, 
                            typeinfo.RegClass:$src2),
                "shld{"#typeinfo.InstrSuffix#"}\t{%cl, $src2, $src1, $dst|$dst, $src1, $src2, cl}",
                [(set typeinfo.RegClass:$dst, 
                (node (typeinfo.LoadNode addr:$src1), 
                typeinfo.RegClass:$src2, CL))]>, TB;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : I<opcode, f, (outs), (ins typeinfo.MemOperand:$dst, 
                                        typeinfo.RegClass:$src2),
                "{nf}\tshld{"#typeinfo.InstrSuffix#"}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                [(store (node (typeinfo.LoadNode addr:$dst), 
                typeinfo.RegClass:$src2, CL), addr:$dst)]>, TB;
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                    (ins typeinfo.MemOperand:$src1, 
                                typeinfo.RegClass:$src2),
                "{nf}\tshld{"#typeinfo.InstrSuffix#"}\t{%cl, $src2, $src1, $dst|$dst, $src1, $src2, cl}",
                [(set typeinfo.RegClass:$dst, 
                (node (typeinfo.LoadNode addr:$src1), 
                typeinfo.RegClass:$src2, CL))]>, TB;
}

multiclass shiftRD_mrCL<bits<8> opcode, Format f, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Predicates = [TruePredicate] in
  def NAME : I<opcode, f, (outs), (ins typeinfo.MemOperand:$dst, 
                                            typeinfo.RegClass:$src2),
                "shrd{"#typeinfo.InstrSuffix#"}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                [(store (node typeinfo.RegClass:$src2, 
                (typeinfo.LoadNode addr:$dst), CL), addr:$dst)]>, TB;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in
  def NAME#_ND : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                (ins typeinfo.MemOperand:$src1, 
                            typeinfo.RegClass:$src2),
                "shrd{"#typeinfo.InstrSuffix#"}\t{%cl, $src2, $src1, $dst|$dst, $src1, $src2, cl}",
                [(set typeinfo.RegClass:$dst, 
                (node typeinfo.RegClass:$src2, 
                (typeinfo.LoadNode addr:$src1), CL))]>, TB;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : I<opcode, f, (outs), (ins typeinfo.MemOperand:$dst, 
                                                typeinfo.RegClass:$src2),
                "{nf}\tshrd{"#typeinfo.InstrSuffix#"}\t{%cl, $src2, $dst|$dst, $src2, cl}",
                [(store (node typeinfo.RegClass:$src2, 
                (typeinfo.LoadNode addr:$dst), CL), addr:$dst)]>, TB;
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : I<opcode, f, (outs typeinfo.RegClass:$dst), 
                    (ins typeinfo.MemOperand:$src1, 
                                typeinfo.RegClass:$src2),
                    "{nf}\tshrd{"#typeinfo.InstrSuffix#"}\t{%cl, $src2, $src1, $dst|$dst, $src1, $src2, cl}",
                    [(set typeinfo.RegClass:$dst, 
                    (node typeinfo.RegClass:$src2, 
                    (typeinfo.LoadNode addr:$src1), CL))]>, TB;
}

let Uses = [CL], SchedRW = [WriteSHDmrcl] in {
defm SHLD16mrCL : shiftLD_mrCL<0xA5, MRMDestMem, X86fshl, Xi16>, OpSize16;
defm SHLD32mrCL : shiftLD_mrCL<0xA5, MRMDestMem, fshl, Xi32>, OpSize32;
defm SHLD64mrCL : shiftLD_mrCL<0xA5, MRMDestMem, fshl, Xi64>, REX_W;

defm SHRD16mrCL : shiftRD_mrCL<0xAD, MRMDestMem, X86fshr, Xi16>, OpSize16;
defm SHRD32mrCL : shiftRD_mrCL<0xAD, MRMDestMem, fshr, Xi32>, OpSize32;
defm SHRD64mrCL : shiftRD_mrCL<0xAD, MRMDestMem, fshr, Xi64>, REX_W;
}

multiclass shiftLD_mri8<bits<8> opcode, Format f, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Predicates = [TruePredicate] in
  def NAME : Ii8<opcode, f, (outs), (ins typeinfo.MemOperand:$dst, 
                                  typeinfo.RegClass:$src2, u8imm:$src3),
                "shld{"#typeinfo.InstrSuffix#"}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                [(store (node (typeinfo.LoadNode addr:$dst), 
                typeinfo.RegClass:$src2, (i8 imm:$src3)), addr:$dst)]>, TB;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in
  def NAME#_ND : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                (ins typeinfo.MemOperand:$src1, 
                    typeinfo.RegClass:$src2, u8imm:$src3),
                "shld{"#typeinfo.InstrSuffix#"}\t{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                [(set typeinfo.RegClass:$dst, 
                (node (typeinfo.LoadNode addr:$src1), 
                typeinfo.RegClass:$src2, (i8 imm:$src3)))]>, TB;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : Ii8<opcode, f, (outs), (ins typeinfo.MemOperand:$dst, 
                                      typeinfo.RegClass:$src2, u8imm:$src3),
                "{nf}\tshld{"#typeinfo.InstrSuffix#"}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                [(store (node (typeinfo.LoadNode addr:$dst), 
                  typeinfo.RegClass:$src2, (i8 imm:$src3)), addr:$dst)]>, TB;
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                    (ins typeinfo.MemOperand:$src1, 
                      typeinfo.RegClass:$src2, u8imm:$src3),
                    "{nf}\tshld{"#typeinfo.InstrSuffix#"}\t{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                    [(set typeinfo.RegClass:$dst, 
                    (node (typeinfo.LoadNode addr:$src1), 
                    typeinfo.RegClass:$src2, (i8 imm:$src3)))]>, TB;
}

multiclass shiftRD_mri8<bits<8> opcode, Format f, SDNode node, X86TypeInfo typeinfo>{
let Defs = [EFLAGS] in {
let Predicates = [TruePredicate] in
  def NAME : Ii8<opcode, f, (outs), (ins typeinfo.MemOperand:$dst, 
                                typeinfo.RegClass:$src2, u8imm:$src3),
                "shrd{"#typeinfo.InstrSuffix#"}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                [(store (node typeinfo.RegClass:$src2, 
                (typeinfo.LoadNode addr:$dst), (i8 imm:$src3)), addr:$dst)]>, TB;
let Predicates = [FalsePredicateNDD], isCodeGenOnly = 1 in
  def NAME#_ND : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                (ins typeinfo.MemOperand:$src1, 
                    typeinfo.RegClass:$src2, u8imm:$src3),
                "shrd{"#typeinfo.InstrSuffix#"}\t{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                [(set typeinfo.RegClass:$dst, 
                  (node typeinfo.RegClass:$src2, 
                  (typeinfo.LoadNode addr:$src1), (i8 imm:$src3)))]>, TB;
}
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_NF : Ii8<opcode, f, (outs), (ins typeinfo.MemOperand:$dst, 
                                    typeinfo.RegClass:$src2, u8imm:$src3),
                "{nf}\tshrd{"#typeinfo.InstrSuffix#"}\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                [(store (node typeinfo.RegClass:$src2, 
                (typeinfo.LoadNode addr:$dst), (i8 imm:$src3)), addr:$dst)]>, TB;
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in
  def NAME#_ND_NF : Ii8<opcode, f, (outs typeinfo.RegClass:$dst), 
                    (ins typeinfo.MemOperand:$src1, 
                      typeinfo.RegClass:$src2, u8imm:$src3),
                    "{nf}\tshrd{"#typeinfo.InstrSuffix#"}\t{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                    [(set typeinfo.RegClass:$dst, 
                    (node typeinfo.RegClass:$src2, 
                    (typeinfo.LoadNode addr:$src1), (i8 imm:$src3)))]>, TB;
}

let SchedRW = [WriteSHDmri] in {
defm SHLD16mri8 : shiftLD_mri8<0xA4, MRMDestMem, X86fshl, Xi16>, OpSize16;
defm SHLD32mri8 : shiftLD_mri8<0xA4, MRMDestMem, fshl, Xi32>, OpSize32;
defm SHLD64mri8 : shiftLD_mri8<0xA4, MRMDestMem, fshl, Xi64>, REX_W;

defm SHRD16mri8 : shiftRD_mri8<0xAC, MRMDestMem, X86fshr, Xi16>, OpSize16;
defm SHRD32mri8 : shiftRD_mri8<0xAC, MRMDestMem, fshr, Xi32>, OpSize32;
defm SHRD64mri8 : shiftRD_mri8<0xAC, MRMDestMem, fshr, Xi64>, REX_W;
}

#else // INTEL_FEATURE_ISA_APX_F
// FIXME: Someone needs to smear multipattern goodness all over this file.

let Defs = [EFLAGS] in {

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

// NOTE: We don't include patterns for shifts of a register by one, because
// 'add reg,reg' is cheaper (and we have a Pat pattern for shift-by-one).
let hasSideEffects = 0 in {
def SHL8r1   : I<0xD0, MRM4r, (outs GR8:$dst), (ins GR8:$src1),
                 "shl{b}\t$dst", []>;
def SHL16r1  : I<0xD1, MRM4r, (outs GR16:$dst), (ins GR16:$src1),
                 "shl{w}\t$dst", []>, OpSize16;
def SHL32r1  : I<0xD1, MRM4r, (outs GR32:$dst), (ins GR32:$src1),
                 "shl{l}\t$dst", []>, OpSize32;
def SHL64r1  : RI<0xD1, MRM4r, (outs GR64:$dst), (ins GR64:$src1),
                 "shl{q}\t$dst", []>;
} // hasSideEffects = 0
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

let SchedRW = [WriteShiftLd, WriteRMW] in {
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
                 "shl{b}\t$dst",
                [(store (shl (loadi8 addr:$dst), (i8 1)), addr:$dst)]>;
def SHL16m1  : I<0xD1, MRM4m, (outs), (ins i16mem:$dst),
                 "shl{w}\t$dst",
                 [(store (shl (loadi16 addr:$dst), (i8 1)), addr:$dst)]>,
                 OpSize16;
def SHL32m1  : I<0xD1, MRM4m, (outs), (ins i32mem:$dst),
                 "shl{l}\t$dst",
                 [(store (shl (loadi32 addr:$dst), (i8 1)), addr:$dst)]>,
                 OpSize32;
def SHL64m1 : RI<0xD1, MRM4m, (outs), (ins i64mem:$dst),
                  "shl{q}\t$dst",
                 [(store (shl (loadi64 addr:$dst), (i8 1)), addr:$dst)]>,
                 Requires<[In64BitMode]>;
} // SchedRW

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
                 "shr{b}\t$dst",
                 [(set GR8:$dst, (srl GR8:$src1, (i8 1)))]>;
def SHR16r1  : I<0xD1, MRM5r, (outs GR16:$dst), (ins GR16:$src1),
                 "shr{w}\t$dst",
                 [(set GR16:$dst, (srl GR16:$src1, (i8 1)))]>, OpSize16;
def SHR32r1  : I<0xD1, MRM5r, (outs GR32:$dst), (ins GR32:$src1),
                 "shr{l}\t$dst",
                 [(set GR32:$dst, (srl GR32:$src1, (i8 1)))]>, OpSize32;
def SHR64r1  : RI<0xD1, MRM5r, (outs GR64:$dst), (ins GR64:$src1),
                 "shr{q}\t$dst",
                 [(set GR64:$dst, (srl GR64:$src1, (i8 1)))]>;
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

let SchedRW = [WriteShiftLd, WriteRMW] in {
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
                 "shr{b}\t$dst",
                 [(store (srl (loadi8 addr:$dst), (i8 1)), addr:$dst)]>;
def SHR16m1  : I<0xD1, MRM5m, (outs), (ins i16mem:$dst),
                 "shr{w}\t$dst",
                 [(store (srl (loadi16 addr:$dst), (i8 1)), addr:$dst)]>,
                 OpSize16;
def SHR32m1  : I<0xD1, MRM5m, (outs), (ins i32mem:$dst),
                 "shr{l}\t$dst",
                 [(store (srl (loadi32 addr:$dst), (i8 1)), addr:$dst)]>,
                 OpSize32;
def SHR64m1 : RI<0xD1, MRM5m, (outs), (ins i64mem:$dst),
                  "shr{q}\t$dst",
                 [(store (srl (loadi64 addr:$dst), (i8 1)), addr:$dst)]>,
                 Requires<[In64BitMode]>;
} // SchedRW

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
                 "sar{b}\t$dst",
                 [(set GR8:$dst, (sra GR8:$src1, (i8 1)))]>;
def SAR16r1  : I<0xD1, MRM7r, (outs GR16:$dst), (ins GR16:$src1),
                 "sar{w}\t$dst",
                 [(set GR16:$dst, (sra GR16:$src1, (i8 1)))]>, OpSize16;
def SAR32r1  : I<0xD1, MRM7r, (outs GR32:$dst), (ins GR32:$src1),
                 "sar{l}\t$dst",
                 [(set GR32:$dst, (sra GR32:$src1, (i8 1)))]>, OpSize32;
def SAR64r1  : RI<0xD1, MRM7r, (outs GR64:$dst), (ins GR64:$src1),
                  "sar{q}\t$dst",
                  [(set GR64:$dst, (sra GR64:$src1, (i8 1)))]>;
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

let SchedRW = [WriteShiftLd, WriteRMW] in {
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
                 "sar{b}\t$dst",
                [(store (sra (loadi8 addr:$dst), (i8 1)), addr:$dst)]>;
def SAR16m1  : I<0xD1, MRM7m, (outs), (ins i16mem:$dst),
                 "sar{w}\t$dst",
               [(store (sra (loadi16 addr:$dst), (i8 1)), addr:$dst)]>,
               OpSize16;
def SAR32m1  : I<0xD1, MRM7m, (outs), (ins i32mem:$dst),
                 "sar{l}\t$dst",
               [(store (sra (loadi32 addr:$dst), (i8 1)), addr:$dst)]>,
               OpSize32;
def SAR64m1 : RI<0xD1, MRM7m, (outs), (ins i64mem:$dst),
                  "sar{q}\t$dst",
                 [(store (sra (loadi64 addr:$dst), (i8 1)), addr:$dst)]>,
                 Requires<[In64BitMode]>;
} // SchedRW

//===----------------------------------------------------------------------===//
// Rotate instructions
//===----------------------------------------------------------------------===//

let hasSideEffects = 0 in {
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

let mayStore = 1 in {
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
} // mayStore
} // hasSideEffects = 0

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
                 "rol{b}\t$dst",
                 [(set GR8:$dst, (rotl GR8:$src1, (i8 1)))]>;
def ROL16r1  : I<0xD1, MRM0r, (outs GR16:$dst), (ins GR16:$src1),
                 "rol{w}\t$dst",
                 [(set GR16:$dst, (rotl GR16:$src1, (i8 1)))]>, OpSize16;
def ROL32r1  : I<0xD1, MRM0r, (outs GR32:$dst), (ins GR32:$src1),
                 "rol{l}\t$dst",
                 [(set GR32:$dst, (rotl GR32:$src1, (i8 1)))]>, OpSize32;
def ROL64r1  : RI<0xD1, MRM0r, (outs GR64:$dst), (ins GR64:$src1),
                  "rol{q}\t$dst",
                  [(set GR64:$dst, (rotl GR64:$src1, (i8 1)))]>;
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

let SchedRW = [WriteRotateLd, WriteRMW] in {
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
                 "rol{b}\t$dst",
                 [(store (rotl (loadi8 addr:$dst), (i8 1)), addr:$dst)]>;
def ROL16m1  : I<0xD1, MRM0m, (outs), (ins i16mem:$dst),
                 "rol{w}\t$dst",
                 [(store (rotl (loadi16 addr:$dst), (i8 1)), addr:$dst)]>,
                 OpSize16;
def ROL32m1  : I<0xD1, MRM0m, (outs), (ins i32mem:$dst),
                 "rol{l}\t$dst",
                 [(store (rotl (loadi32 addr:$dst), (i8 1)), addr:$dst)]>,
                 OpSize32;
def ROL64m1  : RI<0xD1, MRM0m, (outs), (ins i64mem:$dst),
                 "rol{q}\t$dst",
                 [(store (rotl (loadi64 addr:$dst), (i8 1)), addr:$dst)]>,
                 Requires<[In64BitMode]>;
} // SchedRW

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
                 "ror{b}\t$dst",
                 [(set GR8:$dst, (rotr GR8:$src1, (i8 1)))]>;
def ROR16r1  : I<0xD1, MRM1r, (outs GR16:$dst), (ins GR16:$src1),
                 "ror{w}\t$dst",
                 [(set GR16:$dst, (rotr GR16:$src1, (i8 1)))]>, OpSize16;
def ROR32r1  : I<0xD1, MRM1r, (outs GR32:$dst), (ins GR32:$src1),
                 "ror{l}\t$dst",
                 [(set GR32:$dst, (rotr GR32:$src1, (i8 1)))]>, OpSize32;
def ROR64r1  : RI<0xD1, MRM1r, (outs GR64:$dst), (ins GR64:$src1),
                  "ror{q}\t$dst",
                  [(set GR64:$dst, (rotr GR64:$src1, (i8 1)))]>;
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

let SchedRW = [WriteRotateLd, WriteRMW] in {
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
                 "ror{b}\t$dst",
                 [(store (rotr (loadi8 addr:$dst), (i8 1)), addr:$dst)]>;
def ROR16m1  : I<0xD1, MRM1m, (outs), (ins i16mem:$dst),
                 "ror{w}\t$dst",
                 [(store (rotr (loadi16 addr:$dst), (i8 1)), addr:$dst)]>,
                 OpSize16;
def ROR32m1  : I<0xD1, MRM1m, (outs), (ins i32mem:$dst),
                 "ror{l}\t$dst",
                 [(store (rotr (loadi32 addr:$dst), (i8 1)), addr:$dst)]>,
                 OpSize32;
def ROR64m1  : RI<0xD1, MRM1m, (outs), (ins i64mem:$dst),
                 "ror{q}\t$dst",
                 [(store (rotr (loadi64 addr:$dst), (i8 1)), addr:$dst)]>,
                 Requires<[In64BitMode]>;
} // SchedRW


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

} // Defs = [EFLAGS]
#endif // INTEL_FEATURE_ISA_APX_F
#endif // INTEL_CUSTOMIZATION

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

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_APX_F
// shiftX do not have NDD version
let Predicates = [FalsePredicateNF], isCodeGenOnly = 1 in {
  defm RORX32_NF : bmi_rotate<"rorx{l}", GR32, i32mem>;
  defm RORX64_NF : bmi_rotate<"rorx{q}", GR64, i64mem>, VEX_W1X;
  defm SARX32_NF : bmi_shift<"sarx{l}", GR32, i32mem>, T8XS;
  defm SARX64_NF : bmi_shift<"sarx{q}", GR64, i64mem>, T8XS, VEX_W1X;
  defm SHRX32_NF : bmi_shift<"shrx{l}", GR32, i32mem>, T8XD;
  defm SHRX64_NF : bmi_shift<"shrx{q}", GR64, i64mem>, T8XD, VEX_W1X;
  defm SHLX32_NF : bmi_shift<"shlx{l}", GR32, i32mem>, T8PD;
  defm SHLX64_NF : bmi_shift<"shlx{q}", GR64, i64mem>, T8PD, VEX_W1X;
}
#endif // INTEL_FEATURE_ISA_APX_F
#endif // INTEL_CUSTOMIZATION

let Predicates = [HasBMI2] in {
  defm RORX32 : bmi_rotate<"rorx{l}", GR32, i32mem>;
  defm RORX64 : bmi_rotate<"rorx{q}", GR64, i64mem>, VEX_W;
  defm SARX32 : bmi_shift<"sarx{l}", GR32, i32mem>, T8XS;
  defm SARX64 : bmi_shift<"sarx{q}", GR64, i64mem>, T8XS, VEX_W;
  defm SHRX32 : bmi_shift<"shrx{l}", GR32, i32mem>, T8XD;
  defm SHRX64 : bmi_shift<"shrx{q}", GR64, i64mem>, T8XD, VEX_W;
  defm SHLX32 : bmi_shift<"shlx{l}", GR32, i32mem>, T8PD;
  defm SHLX64 : bmi_shift<"shlx{q}", GR64, i64mem>, T8PD, VEX_W;

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
