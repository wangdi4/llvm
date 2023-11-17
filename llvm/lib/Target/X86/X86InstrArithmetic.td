//===-- X86InstrArithmetic.td - Integer Arithmetic Instrs --*- tablegen -*-===//
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
// This file describes the integer arithmetic instructions in the X86
// architecture.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// LEA - Load Effective Address
let SchedRW = [WriteLEA] in {
let hasSideEffects = 0 in
def LEA16r   : I<0x8D, MRMSrcMem,
                 (outs GR16:$dst), (ins anymem:$src),
                 "lea{w}\t{$src|$dst}, {$dst|$src}", []>, OpSize16;
let isReMaterializable = 1 in
def LEA32r   : I<0x8D, MRMSrcMem,
                 (outs GR32:$dst), (ins anymem:$src),
                 "lea{l}\t{$src|$dst}, {$dst|$src}",
                 [(set GR32:$dst, lea32addr:$src)]>,
                 OpSize32, Requires<[Not64BitMode]>;

def LEA64_32r : I<0x8D, MRMSrcMem,
                  (outs GR32:$dst), (ins lea64_32mem:$src),
                  "lea{l}\t{$src|$dst}, {$dst|$src}",
                  [(set GR32:$dst, lea64_32addr:$src)]>,
                  OpSize32, Requires<[In64BitMode]>;

let isReMaterializable = 1 in
def LEA64r   : RI<0x8D, MRMSrcMem, (outs GR64:$dst), (ins lea64mem:$src),
                  "lea{q}\t{$src|$dst}, {$dst|$src}",
                  [(set GR64:$dst, lea64addr:$src)]>;
} // SchedRW

// Pseudo instruction for lea that prevent optimizer from eliminating
// the instruction.
let SchedRW = [WriteLEA], isPseudo = true, hasSideEffects = 1 in {
def PLEA32r   : PseudoI<(outs GR32:$dst), (ins anymem:$src), []>;
def PLEA64r   : PseudoI<(outs GR64:$dst), (ins anymem:$src), []>;
}

//===----------------------------------------------------------------------===//
//  Fixed-Register Multiplication and Division Instructions.
//

// SchedModel info for instruction that loads one value and gets the second
// (and possibly third) value from a register.
// This is used for instructions that put the memory operands before other
// uses.
class SchedLoadReg<X86FoldableSchedWrite Sched> : Sched<[Sched.Folded,
  // Memory operand.
  ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault,
  // Register reads (implicit or explicit).
  Sched.ReadAfterFold, Sched.ReadAfterFold]>;

// BinOpRR - Binary instructions with inputs "reg, reg".
class BinOpRR<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
              dag outlist, X86FoldableSchedWrite sched, list<dag> pattern>
  : ITy<opcode, MRMDestReg, typeinfo, outlist,
        (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2),
        mnemonic, "{$src2, $src1|$src1, $src2}", pattern>,
    Sched<[sched]>;

// BinOpRR_F - Binary instructions with inputs "reg, reg", where the pattern
// has just a EFLAGS as a result.
class BinOpRR_F<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                SDPatternOperator opnode>
  : BinOpRR<opcode, mnemonic, typeinfo, (outs), WriteALU,
            [(set EFLAGS,
                  (opnode typeinfo.RegClass:$src1, typeinfo.RegClass:$src2))]>;

// BinOpRR_RF - Binary instructions with inputs "reg, reg", where the pattern
// has both a regclass and EFLAGS as a result.
class BinOpRR_RF<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                 SDNode opnode>
  : BinOpRR<opcode, mnemonic, typeinfo, (outs typeinfo.RegClass:$dst), WriteALU,
            [(set typeinfo.RegClass:$dst, EFLAGS,
                  (opnode typeinfo.RegClass:$src1, typeinfo.RegClass:$src2))]>;

// BinOpRR_RFF - Binary instructions with inputs "reg, reg", where the pattern
// has both a regclass and EFLAGS as a result, and has EFLAGS as input.
class BinOpRR_RFF<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                  SDNode opnode>
  : BinOpRR<opcode, mnemonic, typeinfo, (outs typeinfo.RegClass:$dst), WriteADC,
            [(set typeinfo.RegClass:$dst, EFLAGS,
                  (opnode typeinfo.RegClass:$src1, typeinfo.RegClass:$src2,
                          EFLAGS))]>;

// BinOpRR_Rev - Binary instructions with inputs "reg, reg"(reversed encoding).
class BinOpRR_Rev<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                  X86FoldableSchedWrite sched = WriteALU>
  : ITy<opcode, MRMSrcReg, typeinfo,
        (outs typeinfo.RegClass:$dst),
        (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2),
        mnemonic, "{$src2, $dst|$dst, $src2}", []>,
    Sched<[sched]> {
  // The disassembler should know about this, but not the asmparser.
  let isCodeGenOnly = 1;
  let ForceDisassemble = 1;
  let hasSideEffects = 0;
}

// BinOpRR_RFF_Rev - Binary instructions with inputs "reg, reg"(reversed
// encoding), with sched = WriteADC.
class BinOpRR_RFF_Rev<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo>
  : BinOpRR_Rev<opcode, mnemonic, typeinfo, WriteADC>;

// BinOpRR_F_Rev - Binary instructions with inputs "reg, reg"(reversed
// encoding), without outlist dag.
class BinOpRR_F_Rev<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo>
  : ITy<opcode, MRMSrcReg, typeinfo, (outs),
        (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2),
        mnemonic, "{$src2, $src1|$src1, $src2}", []>,
    Sched<[WriteALU]> {
  // The disassembler should know about this, but not the asmparser.
  let isCodeGenOnly = 1;
  let ForceDisassemble = 1;
  let hasSideEffects = 0;
}

// BinOpRM - Binary instructions with inputs "reg, [mem]".
class BinOpRM<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
              dag outlist, X86FoldableSchedWrite sched, list<dag> pattern>
  : ITy<opcode, MRMSrcMem, typeinfo, outlist,
        (ins typeinfo.RegClass:$src1, typeinfo.MemOperand:$src2),
        mnemonic, "{$src2, $src1|$src1, $src2}", pattern>,
    Sched<[sched.Folded, sched.ReadAfterFold]>;

// BinOpRM_ImplicitUse - Binary instructions with inputs "reg, [mem]".
// There is an implicit register read at the end of the operand sequence.
class BinOpRM_ImplicitUse<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                          dag outlist, X86FoldableSchedWrite sched, list<dag> pattern>
  : ITy<opcode, MRMSrcMem, typeinfo, outlist,
        (ins typeinfo.RegClass:$src1, typeinfo.MemOperand:$src2),
        mnemonic, "{$src2, $src1|$src1, $src2}", pattern>,
    Sched<[sched.Folded, sched.ReadAfterFold,
           // base, scale, index, offset, segment.
           ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault,
           // implicit register read.
           sched.ReadAfterFold]>;

// BinOpRM_F - Binary instructions with inputs "reg, [mem]", where the pattern
// has just a EFLAGS as a result.
class BinOpRM_F<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                SDNode opnode>
  : BinOpRM<opcode, mnemonic, typeinfo, (outs), WriteALU,
            [(set EFLAGS,
            (opnode typeinfo.RegClass:$src1, (typeinfo.LoadNode addr:$src2)))]>;

// BinOpRM_RF - Binary instructions with inputs "reg, [mem]", where the pattern
// has both a regclass and EFLAGS as a result.
class BinOpRM_RF<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                 SDNode opnode>
  : BinOpRM<opcode, mnemonic, typeinfo, (outs typeinfo.RegClass:$dst), WriteALU,
            [(set typeinfo.RegClass:$dst, EFLAGS,
            (opnode typeinfo.RegClass:$src1, (typeinfo.LoadNode addr:$src2)))]>;

// BinOpRM_RFF - Binary instructions with inputs "reg, [mem]", where the pattern
// has both a regclass and EFLAGS as a result, and has EFLAGS as input.
class BinOpRM_RFF<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                  SDNode opnode>
  : BinOpRM_ImplicitUse<opcode, mnemonic, typeinfo,
                        (outs typeinfo.RegClass:$dst), WriteADC,
                        [(set typeinfo.RegClass:$dst, EFLAGS,
                         (opnode typeinfo.RegClass:$src1,
                         (typeinfo.LoadNode addr:$src2), EFLAGS))]>;

// BinOpRI - Binary instructions with inputs "reg, imm".
class BinOpRI<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
              Format f, dag outlist, X86FoldableSchedWrite sched, list<dag> pattern>
  : ITy<opcode, f, typeinfo, outlist,
        (ins typeinfo.RegClass:$src1, typeinfo.ImmOperand:$src2),
        mnemonic, "{$src2, $src1|$src1, $src2}", pattern>,
    Sched<[sched]> {
  let ImmT = typeinfo.ImmEncoding;
}

// BinOpRI_F - Binary instructions with inputs "reg, imm", where the pattern
// has EFLAGS as a result.
class BinOpRI_F<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                SDPatternOperator opnode, Format f>
  : BinOpRI<opcode, mnemonic, typeinfo, f, (outs), WriteALU,
            [(set EFLAGS,
                (opnode typeinfo.RegClass:$src1, typeinfo.ImmOperator:$src2))]>;

// BinOpRI_RF - Binary instructions with inputs "reg, imm", where the pattern
// has both a regclass and EFLAGS as a result.
class BinOpRI_RF<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                 SDNode opnode, Format f>
  : BinOpRI<opcode, mnemonic, typeinfo, f, (outs typeinfo.RegClass:$dst), WriteALU,
            [(set typeinfo.RegClass:$dst, EFLAGS,
                (opnode typeinfo.RegClass:$src1, typeinfo.ImmOperator:$src2))]>;

// BinOpRI_RFF - Binary instructions with inputs "reg, imm", where the pattern
// has both a regclass and EFLAGS as a result, and has EFLAGS as input.
class BinOpRI_RFF<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                  SDNode opnode, Format f>
  : BinOpRI<opcode, mnemonic, typeinfo, f, (outs typeinfo.RegClass:$dst), WriteADC,
            [(set typeinfo.RegClass:$dst, EFLAGS,
                (opnode typeinfo.RegClass:$src1, typeinfo.ImmOperator:$src2,
                        EFLAGS))]>;

// BinOpRI8 - Binary instructions with inputs "reg, imm8".
class BinOpRI8<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
               Format f, dag outlist, X86FoldableSchedWrite sched, list<dag> pattern>
  : ITy<opcode, f, typeinfo, outlist,
        (ins typeinfo.RegClass:$src1, typeinfo.Imm8Operand:$src2),
        mnemonic, "{$src2, $src1|$src1, $src2}", pattern>,
    Sched<[sched]> {
  let ImmT = Imm8; // Always 8-bit immediate.
}

// BinOpRI8_F - Binary instructions with inputs "reg, imm8".
class BinOpRI8_F<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo, Format f>
  : BinOpRI8<opcode, mnemonic, typeinfo, f, (outs), WriteALU, []>;

// BinOpRI8_RF - Binary instructions with inputs "reg, imm8".
class BinOpRI8_RF<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo, Format f>
  : BinOpRI8<opcode, mnemonic, typeinfo, f, (outs typeinfo.RegClass:$dst), WriteALU, []>;

// BinOpRI8_RFF - Binary instructions with inputs "reg, imm8".
class BinOpRI8_RFF<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo, Format f>
  : BinOpRI8<opcode, mnemonic, typeinfo, f, (outs typeinfo.RegClass:$dst), WriteADC, []>;

// BinOpMR - Binary instructions with inputs "[mem], reg".
class BinOpMR<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
              list<dag> pattern>
  : ITy<opcode, MRMDestMem, typeinfo,
        (outs), (ins typeinfo.MemOperand:$dst, typeinfo.RegClass:$src),
        mnemonic, "{$src, $dst|$dst, $src}", pattern>;

// BinOpMR_RMW - Binary instructions with inputs "[mem], reg", where the pattern
// implicitly use EFLAGS.
class BinOpMR_RMW<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                  SDNode opnode>
  : BinOpMR<opcode, mnemonic, typeinfo,
            [(store (opnode (load addr:$dst), typeinfo.RegClass:$src), addr:$dst),
             (implicit EFLAGS)]>,
    Sched<[WriteALURMW,
           // base, scale, index, offset, segment
           ReadDefault, ReadDefault, ReadDefault,
           ReadDefault, ReadDefault,
           WriteALU.ReadAfterFold]>;  // reg

// BinOpMR_RMW_FF - Binary instructions with inputs "[mem], reg", where the
// pattern sets EFLAGS and implicitly uses EFLAGS.
class BinOpMR_RMW_FF<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                     SDNode opnode>
  : BinOpMR<opcode, mnemonic, typeinfo,
            [(store (opnode (load addr:$dst), typeinfo.RegClass:$src, EFLAGS),
                    addr:$dst),
             (implicit EFLAGS)]>,
    Sched<[WriteADCRMW,
          // base, scale, index, offset, segment
          ReadDefault, ReadDefault, ReadDefault,
          ReadDefault, ReadDefault,
          WriteALU.ReadAfterFold,    // reg
          WriteALU.ReadAfterFold]>;  // EFLAGS

// BinOpMR_F - Binary instructions with inputs "[mem], reg", where the pattern
// has EFLAGS as a result.
class BinOpMR_F<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                SDPatternOperator opnode>
  : BinOpMR<opcode, mnemonic, typeinfo,
            [(set EFLAGS, (opnode (typeinfo.LoadNode addr:$dst),
                                   typeinfo.RegClass:$src))]>,
    Sched<[WriteALU.Folded, ReadDefault, ReadDefault, ReadDefault,
            ReadDefault, ReadDefault, WriteALU.ReadAfterFold]>;

// BinOpMI - Binary instructions with inputs "[mem], imm".
class BinOpMI<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
              Format f, list<dag> pattern>
  : ITy<opcode, f, typeinfo,
        (outs), (ins typeinfo.MemOperand:$dst, typeinfo.ImmOperand:$src),
        mnemonic, "{$src, $dst|$dst, $src}", pattern> {
  let ImmT = typeinfo.ImmEncoding;
}

// BinOpMI_RMW - Binary instructions with inputs "[mem], imm", where the
// pattern implicitly use EFLAGS.
class BinOpMI_RMW<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                  SDNode opnode, Format f>
  : BinOpMI<opcode, mnemonic, typeinfo, f,
            [(store (opnode (typeinfo.VT (load addr:$dst)),
                            typeinfo.ImmOperator:$src), addr:$dst),
             (implicit EFLAGS)]>,
    Sched<[WriteALURMW]>;

// BinOpMI_RMW_FF - Binary instructions with inputs "[mem], imm", where the
// pattern sets EFLAGS and implicitly uses EFLAGS.
class BinOpMI_RMW_FF<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                     SDNode opnode, Format f>
  : BinOpMI<opcode, mnemonic, typeinfo, f,
            [(store (opnode (typeinfo.VT (load addr:$dst)),
                             typeinfo.ImmOperator:$src, EFLAGS), addr:$dst),
                             (implicit EFLAGS)]>,
    Sched<[WriteADCRMW]>;

// BinOpMI_F - Binary instructions with inputs "[mem], imm", where the pattern
// has EFLAGS as a result.
class BinOpMI_F<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                SDPatternOperator opnode, Format f>
  : BinOpMI<opcode, mnemonic, typeinfo, f,
            [(set EFLAGS, (opnode (typeinfo.LoadNode addr:$dst),
                                  typeinfo.ImmOperator:$src))]>,
    Sched<[WriteALU.Folded]>;

// BinOpMI8 - Binary instructions with inputs "[mem], imm8".
class BinOpMI8<string mnemonic, X86TypeInfo typeinfo,
               Format f, list<dag> pattern>
  : ITy<0x82, f, typeinfo,
        (outs), (ins typeinfo.MemOperand:$dst, typeinfo.Imm8Operand:$src),
        mnemonic, "{$src, $dst|$dst, $src}", pattern> {
  let ImmT = Imm8; // Always 8-bit immediate.
}

// BinOpMI8_RMW - Binary instructions with inputs "[mem], imm8".
class BinOpMI8_RMW<string mnemonic, X86TypeInfo typeinfo, Format f>
  : BinOpMI8<mnemonic, typeinfo, f, []>, Sched<[WriteALURMW]>;

// BinOpMI8_RMW_FF - Binary instructions with inputs "[mem], imm8".
class BinOpMI8_RMW_FF<string mnemonic, X86TypeInfo typeinfo, Format f>
  : BinOpMI8<mnemonic, typeinfo, f, []>, Sched<[WriteADCRMW]>;

// BinOpMI8_F - Binary instructions with inputs "[mem], imm8"
class BinOpMI8_F<string mnemonic, X86TypeInfo typeinfo, Format f>
  : BinOpMI8<mnemonic, typeinfo, f, []>, Sched<[WriteALU.Folded]>;

// BinOpAI - Binary instructions with input imm, that implicitly use A reg and
// implicitly define Areg and EFLAGS.
class BinOpAI<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
              Register areg, string operands, X86FoldableSchedWrite sched = WriteALU>
  : ITy<opcode, RawFrm, typeinfo,
        (outs), (ins typeinfo.ImmOperand:$src),
        mnemonic, operands, []>,
    Sched<[sched]> {
  let ImmT = typeinfo.ImmEncoding;
  let Uses = [areg];
  let Defs = [areg, EFLAGS];
  let hasSideEffects = 0;
}

// BinOpAI_RFF - Binary instructions with input imm, that implicitly use and
// define Areg and EFLAGS.
class BinOpAI_RFF<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                  Register areg, string operands>
  : BinOpAI<opcode, mnemonic, typeinfo, areg, operands, WriteADC> {
  let Uses = [areg, EFLAGS];
}

// BinOpAI_F - Binary instructions with input imm, that implicitly use A reg and
// implicitly define EFLAGS.
class BinOpAI_F<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                Register areg, string operands>
  : BinOpAI<opcode, mnemonic, typeinfo, areg, operands> {
  let Defs = [EFLAGS];
}


#if INTEL_CUSTOMIZATION
// BinOpRI8_RF_EVEX - Binary instructions with inputs "reg, imm8", evex
// version.
class BinOpRI8_RF_EVEX<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                       Format f, X86FoldableSchedWrite sched,
                       list<dag> pattern = []>
  : ITy_EVEX<opcode, f, typeinfo, (outs typeinfo.RegClass:$dst),
             (ins typeinfo.RegClass:$src1, typeinfo.Imm8Operand:$src2),
              mnemonic, "{$src2, $src1|$src1, $src2}", pattern>,
    Sched<[sched]> {
  let ImmT = Imm8; // Always 8-bit immediate.
}

// BinOpMR_RMW_EVEX - Binary instructions with inputs "[mem], reg", evex/nf
// version.
class BinOpMR_RMW_EVEX<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo>
  : ITy_EVEX<opcode, MRMDestMem, typeinfo, (outs),
             (ins typeinfo.MemOperand:$dst, typeinfo.RegClass:$src),
             mnemonic, "{$src, $dst|$dst, $src}", []>,
    Sched<[WriteALURMW,
           // base, scale, index, offset, segment
           ReadDefault, ReadDefault, ReadDefault,
           ReadDefault, ReadDefault,
           WriteALU.ReadAfterFold]>;  // reg;

// BinOpMR_ND - Binary instructions with inputs "[mem], reg", ndd version.
class BinOpMR_ND<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                 list<dag> pattern>
  : ITy_EVEX<opcode, MRMDestMem, typeinfo, (outs typeinfo.RegClass:$dst),
             (ins typeinfo.MemOperand:$src1, typeinfo.RegClass:$src2),
             mnemonic, "{$src2, $src1, $dst|$dst, $src1, $src2}", pattern>,
    Sched<[WriteALU.Folded, WriteALU.ReadAfterFold]>;  // reg;

// BinOpRI_RF_EVEX - Binary instructions with inputs "reg, imm", evex version.
class BinOpRI_RF_EVEX<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                      Format f, list<dag> pattern = [],
                      X86FoldableSchedWrite sched = WriteALU>
  : ITy_EVEX<opcode, f, typeinfo, (outs typeinfo.RegClass:$dst),
             (ins typeinfo.RegClass:$src1, typeinfo.ImmOperand:$src2),
             mnemonic, "{$src2, $src1|$src1, $src2}", pattern>,
    Sched<[sched]> {
  let ImmT = typeinfo.ImmEncoding;
}

// BinOpRI_RF_ND - Binary instructions with inputs "reg, imm", ndd version.
class BinOpRI_RF_ND<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                    Format f, list<dag> patterns = [],
                    X86FoldableSchedWrite sched = WriteALU>
  : ITy_EVEX<opcode, f, typeinfo, (outs typeinfo.RegClass:$dst),
             (ins typeinfo.RegClass:$src1, typeinfo.ImmOperand:$src2),
             mnemonic, "{$src2, $src1, $dst|$dst, $src1, $src2}",
             patterns>,
    Sched<[sched]>{
  let ImmT = typeinfo.ImmEncoding;
}

// BinOpRR_RF_EVEX - Binary instructions with inputs "reg, reg", evex encoding.
class BinOpRR_RF_EVEX<bits<8> opcode, string mnemonic, X86TypeInfo info>
  : ITy_EVEX<opcode, MRMDestReg, info, (outs info.RegClass:$dst),
             (ins info.RegClass:$src1, info.RegClass:$src2),
             mnemonic, "{$src2, $src1|$src1, $src2}", []>,
    Sched<[WriteALU]>;

// BinOpRR_RF_ND - Binary instructions with inputs "reg, reg", ndd version.
class BinOpRR_RF_ND<bits<8> opcode, string mnemonic, X86TypeInfo info,
                    list<dag> pattern = []>
  : ITy_EVEX<opcode, MRMDestReg, info, (outs info.RegClass:$dst),
             (ins info.RegClass:$src1, info.RegClass:$src2), mnemonic,
             "{$src2, $src1, $dst|$dst, $src1, $src2}", pattern>,
    Sched<[WriteALU]>;

// BinOpRR_RF_Common - Binary instructions with inputs "reg, reg", where the
// pattern has both a regclass and EFLAGS as a result, contains legacy and
// APX version.
multiclass BinOpRR_RF_Common<bits<8> opcode, string mnemonic,
                             X86TypeInfo typeinfo, SDNode opnodeflag,
                             bit CommutableRR, bit ConvertibleToThreeAddressRR>
{
  let isCommutable = CommutableRR in {
    let Constraints = "$src1 = $dst" in {
        let isConvertibleToThreeAddress = ConvertibleToThreeAddressRR in {
          let Defs = [EFLAGS] in {
          let Predicates = [NoNDD] in
            def NAME: BinOpRR_RF<opcode, mnemonic, typeinfo, opnodeflag>;
          let Predicates = [In64BitMode] in
            def _EVEX: BinOpRR_RF_EVEX<opcode, mnemonic, typeinfo>,
                       EmitVEXOrEVEXPrefix;
          } // Defs = [EFLAGS]
          let Predicates = [In64BitMode] in
            def _NF:BinOpRR_RF_EVEX<opcode, mnemonic, typeinfo>, EVEX_NF;
  }}
  let Defs = [EFLAGS], Predicates = [HasNDD, In64BitMode] in {
      def _ND:  BinOpRR_RF_ND<opcode, mnemonic, typeinfo,
                              [(set typeinfo.RegClass:$dst, EFLAGS,
                               (opnodeflag typeinfo.RegClass:$src1,
                                           typeinfo.RegClass:$src2))]>,
              EVEX_B4V;
  }
  let Predicates = [HasNDD, In64BitMode] in
      def _ND_NF: BinOpRR_RF_ND<opcode, mnemonic, typeinfo>, EVEX_B4V, EVEX_NF;
  }
}

// BinOpRR_Rev_EVEX - Binary instructions with inputs "reg, reg"
// (reversed encoding), evex version.
class BinOpRR_Rev_EVEX<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                       X86FoldableSchedWrite sched = WriteALU>
  : ITy_EVEX<opcode, MRMSrcReg, typeinfo, (outs typeinfo.RegClass:$dst),
             (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2),
             mnemonic, "{$src2, $dst|$dst, $src2}", []>,
    Sched<[sched]> {
  let isCodeGenOnly = 1;
  let ForceDisassemble = 1;
  let hasSideEffects = 0;
}

// BinOpRR_Rev_ND - Binary instructions with inputs "reg, reg"
// (reversed encoding), ndd version.
class BinOpRR_Rev_ND<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                     X86FoldableSchedWrite sched = WriteALU>
  : ITy_EVEX<opcode, MRMSrcReg, typeinfo, (outs typeinfo.RegClass:$dst),
             (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2),
             mnemonic, "{$src2, $src1, $dst|$dst, $src1, $src2}", []>,
    Sched<[sched]> {
  let isCodeGenOnly = 1;
  let ForceDisassemble = 1;
  let hasSideEffects = 0;
}

// BinOpRR_Rev_Common - Binary instructions with inputs "reg, reg"
// (reversed encoding), contains legacy and APX version.
multiclass BinOpRR_Rev_Common<bits<8> opcode, string mnemonic,
                              X86TypeInfo typeinfo>{
    let Constraints = "$src1 = $dst" in {
      let Defs = [EFLAGS] in {
        let Predicates = [NoNDD] in
        def _REV: BinOpRR_Rev<opcode, mnemonic, typeinfo>;
        let Predicates = [In64BitMode] in
        def _REV_EVEX: BinOpRR_Rev_EVEX<opcode, mnemonic, typeinfo>,
                       EmitVEXOrEVEXPrefix;
      }
        let Predicates = [In64BitMode] in
        def _NF_REV: BinOpRR_Rev_EVEX<opcode, mnemonic, typeinfo>, EVEX_NF;
  }
  let Defs = [EFLAGS], Predicates = [HasNDD, In64BitMode] in {
      def _ND_REV : BinOpRR_Rev_ND<opcode, mnemonic, typeinfo>, EVEX_B4V;
  }
  let Predicates = [HasNDD, In64BitMode] in
      def _ND_NF_REV : BinOpRR_Rev_ND<opcode, mnemonic, typeinfo>,
          EVEX_B4V, EVEX_NF;
}

// BinOpRM_RF_EVEX - Instructions like "add reg, [mem]", evex encoding.
class BinOpRM_RF_EVEX<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo>
  : ITy_EVEX<opcode, MRMSrcMem, typeinfo, (outs typeinfo.RegClass:$dst),
             (ins typeinfo.RegClass:$src1, typeinfo.MemOperand:$src2),
             mnemonic, "{$src2, $src1|$src1, $src2}", []>,
    Sched<[WriteALU.Folded, WriteALU.ReadAfterFold]>;

// BinOpRM_RF_ND - Instructions like "add reg, [mem]", ndd version.
class BinOpRM_RF_ND<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                    list<dag> pattern>
  : ITy_EVEX<opcode, MRMSrcMem, typeinfo, (outs typeinfo.RegClass:$dst),
             (ins typeinfo.RegClass:$src1, typeinfo.MemOperand:$src2),
             mnemonic, "{$src2, $src1, $dst|$dst, $src1, $src2}", pattern>,
    Sched<[WriteALU.Folded, WriteALU.ReadAfterFold]>;

// BinOpRM_RF_Common - Instructions like "add reg, [mem]", contains
// legacy and APX version.
multiclass BinOpRM_RF_Common<bits<8> opcode, string mnemonic,
                             X86TypeInfo typeinfo, SDNode opnodeflag>{
  let Constraints = "$src1 = $dst" in {
  let Defs = [EFLAGS] in {
    let Predicates = [NoNDD] in
      def NAME: BinOpRM_RF<opcode, mnemonic, typeinfo, opnodeflag>;
    let Predicates = [In64BitMode], mayLoad = 1 in
      def _EVEX: BinOpRM_RF_EVEX<opcode, mnemonic, typeinfo>,
                 EmitVEXOrEVEXPrefix;
  }
  let Predicates = [In64BitMode], mayLoad = 1 in
      def _NF: BinOpRM_RF_EVEX<opcode, mnemonic, typeinfo>, EVEX_NF;
  }
  let Defs = [EFLAGS], Predicates = [HasNDD, In64BitMode] in {
  def _ND : BinOpRM_RF_ND<opcode, mnemonic, typeinfo,
                          [(set typeinfo.RegClass:$dst, EFLAGS,
                           (opnodeflag typeinfo.RegClass:$src1,
                           (typeinfo.LoadNode addr:$src2)))]>,
            EVEX_B4V;
  }
  let Predicates = [HasNDD, In64BitMode], mayLoad = 1 in
  def _ND_NF : BinOpRM_RF_ND<opcode, mnemonic, typeinfo, []>,
               EVEX_B4V, EVEX_NF;
}

// BinOpRI8_RF_ND - Instructions like "add reg, imm8", ndd version.
class BinOpRI8_RF_ND<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                     Format f, list<dag> pattern,
                     X86FoldableSchedWrite sched = WriteALU>
  : ITy_EVEX<opcode, f, typeinfo, (outs typeinfo.RegClass:$dst),
             (ins typeinfo.RegClass:$src1, typeinfo.Imm8Operand:$src2),
             mnemonic, "{$src2, $src1, $dst|$dst, $src1, $src2}", pattern>,
    Sched<[sched]> {
  let ImmT = Imm8; // Always 8-bit immediate.
}

// BinOpRI8_RF_Common - Instructions like "add reg, imm8", contains legacy
// and APX version.
multiclass BinOpRI8_RF_Common<bits<8> opcode, string mnemonic,
                              X86TypeInfo typeinfo, Format f>{
  let Constraints = "$src1 = $dst" in {
    let Defs = [EFLAGS] in {
      let Predicates = [NoNDD] in
      def NAME: BinOpRI8_RF<opcode, mnemonic, typeinfo, f>;
      let Predicates = [In64BitMode] in
      def _EVEX: BinOpRI8_RF_EVEX<opcode, mnemonic, typeinfo, f, WriteALU>,
                 EmitVEXOrEVEXPrefix;
    }
    let Predicates = [In64BitMode] in
      def _NF: BinOpRI8_RF_EVEX<opcode, mnemonic, typeinfo, f, WriteALU>,
               EVEX_NF;
  }
  let Defs = [EFLAGS], Predicates = [HasNDD, In64BitMode] in {
    def _ND : BinOpRI8_RF_ND<opcode, mnemonic, typeinfo, f, []>,
              EVEX_B4V;
  }
  let Predicates = [HasNDD, In64BitMode] in
    def _ND_NF : BinOpRI8_RF_ND<opcode, mnemonic, typeinfo, f, []>,
                 EVEX_B4V, EVEX_NF;
}

// BinOpRI_RF_Common - Binary instructions with inputs "reg, imm", where the
// pattern has both a regclass and EFLAGS as a result, contains legacy and
// APX version.
multiclass BinOpRI_RF_Common<bits<8> opcode, string mnemonic,
                             X86TypeInfo typeinfo, SDNode opnodeflag,
                             Format f>{
  let Constraints = "$src1 = $dst" in {
    let Defs = [EFLAGS] in {
      let Predicates = [NoNDD] in
      def NAME: BinOpRI_RF<opcode, mnemonic, typeinfo, opnodeflag, f>;
      let Predicates = [In64BitMode] in
      def _EVEX : BinOpRI_RF_EVEX<opcode, mnemonic, typeinfo, f>,
                  EmitVEXOrEVEXPrefix;
    }
    let Predicates = [In64BitMode] in
      def _NF: BinOpRI_RF_EVEX<opcode, mnemonic, typeinfo, f>, EVEX_NF;
  }
  let Defs = [EFLAGS], Predicates = [HasNDD, In64BitMode] in {
    def _ND : BinOpRI_RF_ND<opcode, mnemonic, typeinfo, f,
                            [(set typeinfo.RegClass:$dst, EFLAGS,
                             (opnodeflag typeinfo.RegClass:$src1,
                                         typeinfo.ImmOperator:$src2))]>,
              EVEX_B4V;
  }
  let Predicates = [HasNDD, In64BitMode] in
    def _ND_NF : BinOpRI_RF_ND<opcode, mnemonic, typeinfo, f>,
                 EVEX_B4V, EVEX_NF;
}

// BinOpMI8_EVEX - Binary instructions with inputs "[mem], imm8", evex version.
class BinOpMI8_EVEX<string mnemonic, X86TypeInfo typeinfo, dag outlist,
                    string asm, Format f, list<dag> pattern>
  : ITy_EVEX<0x82, f, typeinfo, outlist,
             (ins typeinfo.MemOperand:$src1, typeinfo.Imm8Operand:$src2),
             mnemonic, asm, pattern> {
  let ImmT = Imm8; // Always 8-bit immediate.
}

// BinOpMI8_RMW_EVEX - Binary instructions with inputs "[mem], imm8",
// evex version.
class BinOpMI8_RMW_EVEX<string mnemonic, X86TypeInfo typeinfo, Format f>
  : BinOpMI8_EVEX<mnemonic, typeinfo, (outs),
                  "{$src2, $src1|$src1, $src2}", f, []>,
    Sched<[WriteALURMW]>;

// BinOpMI8_ND - Binary instructions with inputs "[mem], imm8", ndd version.
class BinOpMI8_ND<string mnemonic, X86TypeInfo typeinfo, Format f,
                  list<dag> pattern>
  : BinOpMI8_EVEX<mnemonic, typeinfo, (outs typeinfo.RegClass:$dst),
                  "{$src2, $src1, $dst|$dst, $src1, $src2}", f, pattern>,
    Sched<[WriteALU]>;

// BinOpMI8_RMW_Common - Binary instructions with inputs "[mem], imm8", where
// the pattern implicitly use EFLAGS, contains legacy and APX version.
multiclass BinOpMI8_RMW_Common<string mnemonic, X86TypeInfo typeinfo,
                               Format f, Predicate pred = TruePredicate> {
let Defs = [EFLAGS] in {
  let Predicates = [pred], mayStore = 1 in
    def NAME : BinOpMI8_RMW<mnemonic, typeinfo, f>;
  let Predicates = [In64BitMode], mayStore = 1 in
    def _EVEX : BinOpMI8_RMW_EVEX<mnemonic, typeinfo, f>,
                EmitVEXOrEVEXPrefix;
  let Predicates = [HasNDD, In64BitMode] in {
    def _ND : BinOpMI8_ND<mnemonic, typeinfo, f, []>, EVEX_B4V;
  }
}
  let Predicates = [In64BitMode], mayStore = 1 in
    def _NF : BinOpMI8_RMW_EVEX<mnemonic, typeinfo, f>, EVEX_NF;
  let Predicates = [HasNDD, In64BitMode] in
    def _ND_NF : BinOpMI8_ND<mnemonic, typeinfo, f, []>, EVEX_B4V, EVEX_NF;
}

// BinOpMI_EVEX - Binary instructions with inputs "[mem], imm", evex version.
class BinOpMI_EVEX<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                   dag outlist, string asm, Format f, list<dag> pattern>
  : ITy_EVEX<opcode, f, typeinfo, outlist,
             (ins typeinfo.MemOperand:$src1, typeinfo.ImmOperand:$src2),
             mnemonic, asm, pattern> {
  let ImmT = typeinfo.ImmEncoding;
}

// BinOpMI_RMW_EVEX - Binary instructions with inputs "add [mem], imm",
// evex version.
class BinOpMI_RMW_EVEX<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                       Format f>
  : BinOpMI_EVEX<opcode, mnemonic, typeinfo, (outs),
                 "{$src2, $src1|$src1, $src2}", f, []>,
    Sched<[WriteALURMW]>;

// BinOpMI_ND - Binary instructions with inputs "add [mem], imm", ndd version.
class BinOpMI_ND<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                 Format f, list<dag> pattern>
  : BinOpMI_EVEX<opcode, mnemonic, typeinfo, (outs typeinfo.RegClass:$dst),
                 "{$src2, $src1, $dst|$dst, $src1, $src2}", f, pattern>,
    Sched<[WriteALU]>;

// BinOpMI_Common - Binary instructions with inputs "add [mem], imm", where
// the pattern implicitly use EFLAGS, contains legacy and APX version.
multiclass BinOpMI_Common<bits<8> opcode, string mnemonic,
                          X86TypeInfo typeinfo, SDNode opnode, Format f,
                          Predicate pred = TruePredicate>{
  // Instructions like "add [mem], imm".
  let Defs = [EFLAGS] in {
  let Predicates = [pred], mayStore = 1 in {
    def NAME: BinOpMI_RMW<opcode, mnemonic, typeinfo, opnode, f>;
  }
  let Predicates = [In64BitMode], mayStore = 1 in
    def _EVEX : BinOpMI_RMW_EVEX<opcode, mnemonic, typeinfo, f>,
                EmitVEXOrEVEXPrefix;
  // Instructions like "add reg, [mem], imm".
  let Predicates = [HasNDD, In64BitMode] in {
    def _ND : BinOpMI_ND<opcode, mnemonic, typeinfo, f,
                         [(set typeinfo.RegClass:$dst,
                          (opnode (typeinfo.VT (load addr:$src1)),
                           typeinfo.ImmOperator:$src2)), (implicit EFLAGS)]>,
              EVEX_B4V;
  }
  }
  let Predicates = [In64BitMode], mayStore = 1 in
  // Instructions like "{nf} add [mem], imm".
    def _NF: BinOpMI_RMW_EVEX<opcode, mnemonic, typeinfo, f>, EVEX_NF;
  let Predicates = [HasNDD, In64BitMode] in
  // Instructions like "{nf} add reg, [mem], imm".
    def _ND_NF : BinOpMI_ND<opcode, mnemonic, typeinfo, f, []>,
                 EVEX_B4V, EVEX_NF;
}

// BinOpMR_Common - Binary instructions with inputs "[mem], reg", contains
// legacy and APX version.
multiclass BinOpMR_Common<bits<8> opcode, string mnemonic,
                          X86TypeInfo typeinfo, SDNode opnode>{
  // Instructions like "add [mem], reg".
  let mayStore = 1 in {
  let Defs = [EFLAGS] in {
    def NAME: BinOpMR_RMW<opcode, mnemonic, typeinfo, opnode>;
    let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
    def _EVEX: BinOpMR_RMW_EVEX<opcode, mnemonic, typeinfo>,
               EmitVEXOrEVEXPrefix;
  }
  // Instructions like "{nf} add [mem], reg".
  let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
    def _NF: BinOpMR_RMW_EVEX<opcode, mnemonic, typeinfo>, EVEX_NF;
  }
  // Instructions like "add reg, [mem], reg".
  let Defs = [EFLAGS], Predicates = [HasNDD, In64BitMode] in {
  def _ND : BinOpMR_ND<opcode, mnemonic, typeinfo,
                       [(set typeinfo.RegClass:$dst,
                        (opnode (load addr:$src1),
                         typeinfo.RegClass:$src2)), (implicit EFLAGS)]>,
            EVEX_B4V;
  }
  let Predicates = [HasNDD, In64BitMode], mayLoad = 1 in
  // Instructions like "{nf} add reg, [mem], reg".
  def _ND_NF : BinOpMR_ND<opcode, mnemonic, typeinfo, []>,
               EVEX_B4V, EVEX_NF;
}
#endif // INTEL_CUSTOMIZATION

//  UnaryOpM - Unary instructions with a memory operand.
class UnaryOpM<bits<8> opcode, Format f, string mnemonic, X86TypeInfo info,
               list<dag> pattern>
  : ITy<opcode, f, info, (outs), (ins info.MemOperand:$dst), mnemonic,
        "$dst", pattern>;

//  UnaryOpR - Unary instructions with a register.
class UnaryOpR<bits<8> opcode, Format f, string mnemonic, X86TypeInfo info,
               list<dag> pattern>
  : ITy<opcode, f, info, (outs info.RegClass:$dst),
        (ins info.RegClass:$src1), mnemonic, "$dst", pattern>;

#if INTEL_CUSTOMIZATION
//  UnaryOpM - Unary instructions with a memory operand, EVEX encoding.
class UnaryOpM_EVEX<bits<8> opcode, Format f, string mnemonic,
                    X86TypeInfo info, list<dag> pattern>
  : ITy_EVEX<opcode, f, info, (outs), (ins info.MemOperand:$dst), mnemonic,
             "$dst", pattern>;

//  UnaryOpR - Unary instructions with a register, EVEX encoding.
class UnaryOpR_EVEX<bits<8> opcode, Format f, string mnemonic,
                    X86TypeInfo info, list<dag> pattern>
  : ITy_EVEX<opcode, f, info, (outs info.RegClass:$dst),
             (ins info.RegClass:$src1), mnemonic, "$dst", pattern>;

//  UnaryOpM_NDD - Unary instructions with a memory operand, NDD version.
class UnaryOpM_NDD<bits<8> opcode, Format f, string mnemonic, X86TypeInfo info,
                   list<dag> pattern>
  : ITy_EVEX<opcode, f, info, (outs info.RegClass:$dst),
             (ins info.MemOperand:$src1), mnemonic,
             "{$src1, $dst|$dst, $src1}", pattern>;

//  UnaryOpR_NDD - Unary instructions with a register, NDD version.
class UnaryOpR_NDD<bits<8> opcode, Format f, string mnemonic, X86TypeInfo info,
                   list<dag> pattern>
  : ITy_EVEX<opcode, f, info, (outs info.RegClass:$dst),
             (ins info.RegClass:$src1),
             mnemonic, "{$src1, $dst|$dst, $src1}", pattern>;

multiclass INCDECR<Format f, string mnemonic, X86TypeInfo info,
                   SDPatternOperator node>{
let isConvertibleToThreeAddress = 1, CodeSize = 2 in {
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst", Predicates = [NoNDD] in
  def NAME : UnaryOpR<0xFE, f, mnemonic, info,
                      [(set info.RegClass:$dst, EFLAGS,
                       (node info.RegClass:$src1, 1))]>;
let Constraints = "$src1 = $dst", Predicates = [In64BitMode] in
  def _EVEX : UnaryOpR_EVEX<0xFE, f, mnemonic, info, []>, EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : UnaryOpR_NDD<0xFE, f, mnemonic, info,
                         [(set info.RegClass:$dst, EFLAGS,
                          (node info.RegClass:$src1, 1))]>, EVEX_B4V;
}
let Constraints = "$src1 = $dst", Predicates = [In64BitMode] in
  def _NF : UnaryOpR_EVEX<0xFE, f, mnemonic, info, []>, EVEX_NF;
let Predicates = [HasNDD, In64BitMode] in
  def _ND_NF : UnaryOpR_NDD<0xFE, f, mnemonic, info, []>,
               EVEX_B4V, EVEX_NF;
}
}

//  INCDECM - Instructions like "inc [mem]".
multiclass INCDECM<Format f, string mnemonic, X86TypeInfo info, int num>{
let CodeSize = 2 in {
let Defs = [EFLAGS] in{
let Predicates = !if(info.HasREX_W, [UseIncDec, In64BitMode], [UseIncDec]) in
  def NAME : UnaryOpM<0xFE, f, mnemonic, info,
                      [(store (add (info.LoadNode addr:$dst), num), addr:$dst),
                       (implicit EFLAGS)]>;
let Predicates = [UseIncDec, In64BitMode], mayLoad = 1, mayStore = 1 in
  def _EVEX : UnaryOpM_EVEX<0xFE, f, mnemonic, info, []>, EmitVEXOrEVEXPrefix;
let Predicates = [UseIncDec, HasNDD, In64BitMode] in
  def _ND : UnaryOpM_NDD<0xFE, f, mnemonic, info,
                         [(set info.RegClass:$dst,
                          (add (info.LoadNode addr:$src1), num)),
                          (implicit EFLAGS)]>, EVEX_B4V;
}
let Predicates = [UseIncDec, In64BitMode], mayLoad = 1, mayStore = 1 in
  def _NF : UnaryOpM_EVEX<0xFE, f, mnemonic, info, []>, EVEX_NF;
let Predicates = [UseIncDec, HasNDD, In64BitMode], mayLoad = 1 in
  def _ND_NF : UnaryOpM_NDD<0xFE, f, mnemonic, info, []>,
               EVEX_B4V, EVEX_NF;
}
}
#endif // INTEL_CUSTOMIZATION

//  INCDECR_ALT - Instructions like "inc reg" short forms.
class INCDECR_ALT<bits<8> opcode, string mnemonic, X86TypeInfo info>
  : UnaryOpR<opcode, AddRegFrm, mnemonic, info, []>{
  let Predicates = [Not64BitMode];
  let Opcode = opcode;
}

#if INTEL_CUSTOMIZATION
//  MulOpR - Instructions like "mul reg".
multiclass MulOpR<bits<8> opcode, Format f, string mnemonic, X86TypeInfo info,
                  Register reg1, Register reg2, X86FoldableSchedWrite sched,
                  list<dag> pattern>{
let Defs = !if(!eq(info.InstrSuffix, "b"), [reg1, EFLAGS, reg2],
                                           [reg1, reg2, EFLAGS]),
    Uses = [reg1] in{
  def NAME : ITy<opcode, f, info, (outs), (ins info.RegClass:$src), mnemonic,
                 "$src", pattern>,
             Sched<[sched]>;
let Predicates = [In64BitMode] in
  def _EVEX : ITy_EVEX<opcode, f, info, (outs), (ins info.RegClass:$src),
                       mnemonic, "$src", []>,
              Sched<[sched]>, EmitVEXOrEVEXPrefix;
}
let Defs = [reg1, reg2], Uses = [reg1], Predicates = [In64BitMode] in
  def _NF : ITy_EVEX<opcode, f, info, (outs), (ins info.RegClass:$src),
                     mnemonic, "$src", []>,
            Sched<[sched]>, EVEX_NF;
}

//  MulOpM - Instructions like "mul [mem]".
multiclass MulOpM<bits<8> opcode, Format f, string mnemonic, X86TypeInfo info,
                  Register reg1, Register reg2, X86FoldableSchedWrite sched,
                  list<dag> pattern>{
let Defs = !if(!eq(info.InstrSuffix, "b"), [reg1, EFLAGS, reg2],
                                           [reg1, reg2, EFLAGS]),
    Uses = [reg1], mayLoad = 1 in {
let Predicates = !if(info.HasREX_W, [In64BitMode], [TruePredicate]) in
  def NAME : ITy<opcode, f, info, (outs), (ins info.MemOperand:$src), mnemonic,
                 "$src", pattern>,
             SchedLoadReg<sched>;
let Predicates = [In64BitMode], mayLoad = 1 in
  def _EVEX : ITy_EVEX<opcode, f, info, (outs), (ins info.MemOperand:$src),
                       mnemonic, "$src", []>,
              SchedLoadReg<sched>, EmitVEXOrEVEXPrefix;
}
let Defs = [reg1, reg2], Uses = [reg1], Predicates = [In64BitMode],
    mayLoad = 1 in
  def _NF : ITy_EVEX<opcode, f, info, (outs), (ins info.MemOperand:$src),
                     mnemonic, "$src", []>,
            SchedLoadReg<sched>, EVEX_NF;
}

//  DivOpR - Instructions like "div reg".
multiclass DivOpR<bits<8> opcode, Format f, string mnemonic, X86TypeInfo info,
                  Register reg1, Register reg2, list<Register> uses,
                  X86FoldableSchedWrite sched, list<dag> pattern>{
let Defs = [reg1, reg2, EFLAGS], Uses = uses in {
  def NAME : ITy<opcode, f, info, (outs), (ins info.RegClass:$src), mnemonic,
                 "$src", pattern>,
             Sched<[sched]>;
let Predicates = [In64BitMode] in
  def _EVEX : ITy_EVEX<opcode, f, info, (outs), (ins info.RegClass:$src),
                       mnemonic, "$src", []>,
              Sched<[sched]>, EmitVEXOrEVEXPrefix;
}
let Defs = [reg1, reg2], Uses = uses, Predicates = [In64BitMode] in
  def _NF : ITy_EVEX<opcode, f, info, (outs), (ins info.RegClass:$src),
                     mnemonic, "$src", []>,
            Sched<[sched]>, EVEX_NF;
}

//  DivOpM - Instructions like "div [mem]".
multiclass DivOpM<bits<8> opcode, Format f, string mnemonic, X86TypeInfo info,
                  Register reg1, Register reg2, list<Register> uses,
                  X86FoldableSchedWrite sched, list<dag> pattern>{
let Defs = [reg1, reg2, EFLAGS], Uses = uses in {
let Predicates = !if(info.HasREX_W, [In64BitMode], [TruePredicate]) in
  def NAME : ITy<opcode, f, info, (outs), (ins info.MemOperand:$src), mnemonic,
                 "$src", pattern>,
             SchedLoadReg<sched>;
let Predicates = [In64BitMode] in
  def _EVEX : ITy_EVEX<opcode, f, info, (outs), (ins info.MemOperand:$src),
                       mnemonic, "$src", []>,
              SchedLoadReg<sched>, EmitVEXOrEVEXPrefix;
}
let Defs = [reg1, reg2], Uses = uses, Predicates = [In64BitMode] in
  def _NF : ITy_EVEX<opcode, f, info, (outs), (ins info.MemOperand:$src),
                     mnemonic, "$src", []>,
            SchedLoadReg<sched>, EVEX_NF;
}

//  NegOpR - Instructions like "neg reg".
multiclass NegOpR<bits<8> opcode, string mnemonic, X86TypeInfo info>{
let CodeSize = 2 in {
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst", Predicates = [NoNDD] in
  def NAME : UnaryOpR<opcode, MRM3r, mnemonic, info,
                      [(set info.RegClass:$dst, (ineg info.RegClass:$src1)),
                       (implicit EFLAGS)]>;
let Constraints = "$src1 = $dst", Predicates = [In64BitMode] in
  def _EVEX : UnaryOpR_EVEX<opcode, MRM3r, mnemonic, info, []>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : UnaryOpR_NDD<opcode, MRM3r, mnemonic, info,
                              [(set info.RegClass:$dst,
                               (ineg info.RegClass:$src1)), (implicit EFLAGS)]>,
            EVEX_B4V;
}
let Constraints = "$src1 = $dst", Predicates = [In64BitMode] in
  def _NF : UnaryOpR_EVEX<opcode, MRM3r, mnemonic, info, []>,
            EVEX_NF;
let Predicates = [HasNDD, In64BitMode] in
  def _ND_NF : UnaryOpR_NDD<opcode, MRM3r, mnemonic, info, []>,
               EVEX_B4V, EVEX_NF;
}
}

//  NotOpR - Instructions like "not reg".
multiclass NotOpR<bits<8> opcode, string mnemonic, X86TypeInfo info>{
let CodeSize = 2 in {
let Constraints = "$src1 = $dst", Predicates = [NoNDD] in
  def NAME : UnaryOpR<opcode, MRM2r, mnemonic, info,
                      [(set info.RegClass:$dst, (not info.RegClass:$src1))]>;
let Constraints = "$src1 = $dst", Predicates = [In64BitMode] in
  def _EVEX : UnaryOpR_EVEX<opcode, MRM2r, mnemonic, info, []>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : UnaryOpR_NDD<opcode, MRM2r, mnemonic, info,
                          [(set info.RegClass:$dst,
                           (not info.RegClass:$src1))]>,
            EVEX_B4V;
}
}

//  NegOpM - Instructions like "neg [mem]".
multiclass NegOpM<bits<8> opcode, string mnemonic, X86TypeInfo info>{
let CodeSize = 2 in {
let Defs = [EFLAGS] in {
let Predicates = !if(info.HasREX_W, [In64BitMode], [TruePredicate]) in
  def NAME : UnaryOpM<opcode, MRM3m, mnemonic, info,
                      [(store (ineg (info.LoadNode addr:$dst)), addr:$dst),
                       (implicit EFLAGS)]>;
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _EVEX : UnaryOpM_EVEX<opcode, MRM3m, mnemonic, info, []>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : UnaryOpM_NDD<opcode, MRM3m, mnemonic, info,
                         [(set info.RegClass:$dst,
                          (ineg (info.LoadNode addr:$src1))),
                                (implicit EFLAGS)]>,
            EVEX_B4V;
}
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _NF : UnaryOpM_EVEX<opcode, MRM3m, mnemonic, info, []>, EVEX_NF;
let Predicates = [HasNDD, In64BitMode], mayLoad = 1 in
  def _ND_NF : UnaryOpM_NDD<opcode, MRM3m, mnemonic, info, []>,
               EVEX_B4V, EVEX_NF;
}
}

//  NotOpM - Instructions like "not [mem]".
multiclass NotOpM<bits<8> opcode, string mnemonic, X86TypeInfo info>{
let CodeSize = 2 in {
let Predicates = !if(info.HasREX_W, [In64BitMode], [TruePredicate]) in
  def NAME : UnaryOpM<opcode, MRM2m, mnemonic, info,
                      [(store (not (info.LoadNode addr:$dst)), addr:$dst)]>;
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _EVEX : UnaryOpM_EVEX<opcode, MRM2m, mnemonic, info, []>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : UnaryOpM_NDD<opcode, MRM2m, mnemonic, info,
                         [(set info.RegClass:$dst,
                          (not (info.LoadNode addr:$src1)))]>, EVEX_B4V;
}
}
#endif // INTEL_CUSTOMIZATION

// BinOpRR_C - Binary instructions with inputs "reg, reg", which used mainly
// with Constraints = "$src1 = $dst".
class BinOpRR_C<bits<8> opcode, Format f, string mnemonic, X86TypeInfo info,
                list<dag> pattern>
  : ITy<opcode, f, info, (outs info.RegClass:$dst),
        (ins info.RegClass:$src1, info.RegClass:$src2),
        mnemonic, "{$src2, $dst|$dst, $src2}", pattern>;

// BinOpRM_C - Binary instructions with inputs "reg, [mem]", which used mainly
// with Constraints = "$src1 = $dst".
class BinOpRM_C<bits<8> opcode, Format f, string mnemonic, X86TypeInfo info,
                list<dag> pattern>
  : ITy<opcode, f, info, (outs info.RegClass:$dst),
        (ins info.RegClass:$src1, info.MemOperand:$src2),
        mnemonic, "{$src2, $dst|$dst, $src2}", pattern>;

#if INTEL_CUSTOMIZATION
// BinOpRR_EVEX - Binary instructions with inputs "reg, reg", which used EVEX
// encoding.
class BinOpRR_EVEX<bits<8> opcode, Format f, string mnemonic, X86TypeInfo info,
                   list<dag> pattern>
  : ITy_EVEX<opcode, f, info, (outs info.RegClass:$dst),
             (ins info.RegClass:$src1, info.RegClass:$src2),
             mnemonic, "{$src2, $dst|$dst, $src2}", pattern>;

// BinOpRM_EVEX - Binary instructions with inputs "reg, [mem]", which used EVEX
// encoding.
class BinOpRM_EVEX<bits<8> opcode, Format f, string mnemonic, X86TypeInfo info,
                   list<dag> pattern>
  : ITy_EVEX<opcode, f, info, (outs info.RegClass:$dst),
             (ins info.RegClass:$src1, info.MemOperand:$src2),
             mnemonic, "{$src2, $dst|$dst, $src2}", pattern>;

// BinOpRR_NDD - Binary instructions with inputs "reg, reg",  NDD version.
class BinOpRR_NDD<bits<8> opcode, Format f, string mnemonic, X86TypeInfo info,
                  list<dag> pattern>
  : ITy_EVEX<opcode, f, info, (outs info.RegClass:$dst),
             (ins info.RegClass:$src1, info.RegClass:$src2),
              mnemonic, "{$src2, $src1, $dst|$dst, $src1, $src2}", pattern>;

// BinOpRM_NDD - Binary instructions with inputs "reg, [mem]", NDD version.
class BinOpRM_NDD<bits<8> opcode, Format f, string mnemonic, X86TypeInfo info,
                  list<dag> pattern>
  : ITy_EVEX<opcode, f, info, (outs info.RegClass:$dst),
             (ins info.RegClass:$src1, info.MemOperand:$src2),
              mnemonic, "{$src2, $src1, $dst|$dst, $src1, $src2}", pattern>;

multiclass IMulOpRR<bits<8> opcode, string mnemonic, X86TypeInfo info,
                    X86FoldableSchedWrite sched>{
let isCommutable = 1 in {
let Defs = [EFLAGS] in {
  let Constraints = "$src1 = $dst", Predicates = [NoNDD] in
  def NAME : BinOpRR_C<opcode, MRMSrcReg, mnemonic, info,
                       [(set info.RegClass:$dst, EFLAGS,
                        (X86smul_flag info.RegClass:$src1,
                        info.RegClass:$src2))]>,
             Sched<[sched]>, TB;
  let Constraints = "$src1 = $dst", Predicates = [In64BitMode] in
  def _EVEX : BinOpRR_EVEX<opcode, MRMSrcReg, mnemonic, info, []>,
              Sched<[sched]>, EmitVEXOrEVEXPrefix;
  let Predicates = [HasNDD, In64BitMode] in
  def _ND : BinOpRR_NDD<opcode, MRMSrcReg, mnemonic, info,
                        [(set info.RegClass:$dst, EFLAGS,
                         (X86smul_flag info.RegClass:$src1,
                          info.RegClass:$src2))]>,
            Sched<[sched]>, EVEX_B4V;
}
  let Constraints = "$src1 = $dst", Predicates = [In64BitMode] in
  def _NF : BinOpRR_EVEX<opcode, MRMSrcReg, mnemonic, info, []>,
            Sched<[sched]>, EVEX_NF;
  let Predicates = [HasNDD, In64BitMode] in
  def _ND_NF : BinOpRR_NDD<opcode, MRMSrcReg, mnemonic, info, []>,
               Sched<[sched]>, EVEX_B4V, EVEX_NF;
}
}

// IMulOpRM - Instructions like "imul reg, reg, [mem]".
multiclass IMulOpRM<bits<8> opcode, string mnemonic, X86TypeInfo info,
                    X86FoldableSchedWrite sched>{
let Defs = [EFLAGS] in{
  let Constraints = "$src1 = $dst", Predicates = [NoNDD] in
  def NAME : BinOpRM_C<opcode, MRMSrcMem, mnemonic, info,
                       [(set info.RegClass:$dst, EFLAGS,
                        (X86smul_flag info.RegClass:$src1,
                        (info.LoadNode addr:$src2)))]>,
             Sched<[sched.Folded, sched.ReadAfterFold]>, TB;
  let Constraints = "$src1 = $dst", Predicates = [In64BitMode], mayLoad = 1 in
  def _EVEX : BinOpRM_EVEX<opcode, MRMSrcMem, mnemonic, info, []>,
              Sched<[sched.Folded, sched.ReadAfterFold]>,
              EmitVEXOrEVEXPrefix;
  let Predicates = [HasNDD, In64BitMode] in
  def _ND : BinOpRM_NDD<opcode, MRMSrcMem, mnemonic, info,
                        [(set info.RegClass:$dst, EFLAGS,
                         (X86smul_flag info.RegClass:$src1,
                         (info.LoadNode addr:$src2)))]>,
            Sched<[sched.Folded, sched.ReadAfterFold]>, EVEX_B4V;
}
  let Constraints = "$src1 = $dst", Predicates = [In64BitMode], mayLoad = 1 in
  def _NF : BinOpRM_EVEX<opcode, MRMSrcMem, mnemonic, info, []>,
            Sched<[sched.Folded, sched.ReadAfterFold]>, EVEX_NF;
  let Predicates = [HasNDD, In64BitMode], mayLoad = 1 in
  def _ND_NF : BinOpRM_NDD<opcode, MRMSrcMem, mnemonic, info, []>,
               Sched<[sched.Folded, sched.ReadAfterFold]>, EVEX_B4V, EVEX_NF;
}

// IMulOpRRI8 - Instructions like "imul reg, reg, i8".
multiclass IMulOpRRI8<bits<8> opcode, string mnemonic, X86TypeInfo info,
                      X86FoldableSchedWrite sched>{
let hasSideEffects = 0, ImmT = Imm8 in {
let Defs = [EFLAGS] in {
  def NAME : ITy<opcode, MRMSrcReg, info, (outs info.RegClass:$dst),
                 (ins info.RegClass:$src1,
                  info.Imm8Operand:$src2), mnemonic,
                 "{$src2, $src1, $dst|$dst, $src1, $src2}", []>,
             Sched<[sched]>;
let Predicates = [In64BitMode] in
  def _EVEX : ITy_EVEX<opcode, MRMSrcReg, info, (outs info.RegClass:$dst),
                       (ins info.RegClass:$src1,
                       info.Imm8Operand:$src2), mnemonic,
                       "{$src2, $src1, $dst|$dst, $src1, $src2}", []>,
              Sched<[sched]>, EmitVEXOrEVEXPrefix;
}
let Predicates = [In64BitMode] in
  def _NF : ITy_EVEX<opcode, MRMSrcReg, info, (outs info.RegClass:$dst),
                     (ins info.RegClass:$src1,
                      info.Imm8Operand:$src2), mnemonic,
                     "{$src2, $src1, $dst|$dst, $src1, $src2}", []>,
            Sched<[sched]>, EVEX_NF;
}
}

// IMulOpRRI - Instructions like "imul reg, reg, i16/i32/i64".
multiclass IMulOpRRI<bits<8> opcode, string mnemonic, X86TypeInfo info,
                     X86FoldableSchedWrite sched>{
let ImmT = info.ImmEncoding in {
let Defs = [EFLAGS] in {
  def NAME : ITy<opcode, MRMSrcReg, info, (outs info.RegClass:$dst),
                 (ins info.RegClass:$src1,
                  info.ImmOperand:$src2), mnemonic,
                 "{$src2, $src1, $dst|$dst, $src1, $src2}",
                 [(set info.RegClass:$dst, EFLAGS,
                  (X86smul_flag info.RegClass:$src1,
                                info.ImmNoSuOperator:$src2))]>,
             Sched<[sched]>;
let Predicates = [In64BitMode] in
  def _EVEX : ITy_EVEX<opcode, MRMSrcReg, info, (outs info.RegClass:$dst),
                       (ins info.RegClass:$src1,
                        info.ImmOperand:$src2), mnemonic,
                       "{$src2, $src1, $dst|$dst, $src1, $src2}", []>,
              Sched<[sched]>, EmitVEXOrEVEXPrefix;
}
let Predicates = [In64BitMode] in
  def _NF : ITy_EVEX<opcode, MRMSrcReg, info, (outs info.RegClass:$dst),
                     (ins info.RegClass:$src1,
                      info.ImmOperand:$src2), mnemonic,
                      "{$src2, $src1, $dst|$dst, $src1, $src2}", []>,
            Sched<[sched]>, EVEX_NF;
}
}

// IMulOpRMI8 - Instructions like "imul reg, [mem], i8".
multiclass IMulOpRMI8<bits<8> opcode, string mnemonic, X86TypeInfo info,
                      X86FoldableSchedWrite sched>{
let hasSideEffects = 0, mayLoad = 1, ImmT = Imm8 in {
let Defs = [EFLAGS] in {
  def NAME : ITy<opcode, MRMSrcMem, info, (outs info.RegClass:$dst),
                 (ins info.MemOperand:$src1,
                  info.Imm8Operand:$src2), mnemonic,
                 "{$src2, $src1, $dst|$dst, $src1, $src2}", []>,
             Sched<[sched.Folded]>;
let Predicates = [In64BitMode], mayLoad = 1 in
  def _EVEX : ITy_EVEX<opcode, MRMSrcMem, info, (outs info.RegClass:$dst),
                       (ins info.MemOperand:$src1,
                       info.Imm8Operand:$src2), mnemonic,
                       "{$src2, $src1, $dst|$dst, $src1, $src2}", []>,
              Sched<[sched.Folded]>, EmitVEXOrEVEXPrefix;
}
let Predicates = [In64BitMode], mayLoad = 1 in
  def _NF : ITy_EVEX<opcode, MRMSrcMem, info, (outs info.RegClass:$dst),
                     (ins info.MemOperand:$src1,
                      info.Imm8Operand:$src2), mnemonic,
                     "{$src2, $src1, $dst|$dst, $src1, $src2}", []>,
            Sched<[sched.Folded]>, EVEX_NF;
}
}

// IMulOpRMI - Instructions like "imul reg, reg, i16/i32/i64".
multiclass IMulOpRMI<bits<8> opcode, string mnemonic, X86TypeInfo info,
                     X86FoldableSchedWrite sched>{
let ImmT = info.ImmEncoding in {
let Defs = [EFLAGS] in {
  def NAME : ITy<opcode, MRMSrcMem, info, (outs info.RegClass:$dst),
                 (ins info.MemOperand:$src1,
                  info.ImmOperand:$src2), mnemonic,
                 "{$src2, $src1, $dst|$dst, $src1, $src2}",
                 [(set info.RegClass:$dst, EFLAGS,
                  (X86smul_flag (info.LoadNode addr:$src1),
                  info.ImmNoSuOperator:$src2))]>,
             Sched<[sched.Folded]>;
let Predicates = [In64BitMode], mayLoad = 1 in
  def _EVEX : ITy_EVEX<opcode, MRMSrcMem, info, (outs info.RegClass:$dst),
                       (ins info.MemOperand:$src1,
                        info.ImmOperand:$src2), mnemonic,
                       "{$src2, $src1, $dst|$dst, $src1, $src2}", []>,
              Sched<[sched.Folded]>, EmitVEXOrEVEXPrefix;
}
let Predicates = [In64BitMode], mayLoad = 1 in
  def _NF : ITy_EVEX<opcode, MRMSrcMem, info, (outs info.RegClass:$dst),
                     (ins info.MemOperand:$src1,
                      info.ImmOperand:$src2), mnemonic,
                     "{$src2, $src1, $dst|$dst, $src1, $src2}", []>,
            Sched<[sched.Folded]>, EVEX_NF;
}
}
#endif // INTEL_CUSTOMIZATION

def X86add_flag_nocf : PatFrag<(ops node:$lhs, node:$rhs),
                               (X86add_flag node:$lhs, node:$rhs), [{
  return hasNoCarryFlagUses(SDValue(N, 1));
}]>;

def X86sub_flag_nocf : PatFrag<(ops node:$lhs, node:$rhs),
                               (X86sub_flag node:$lhs, node:$rhs), [{
  // Only use DEC if the result is used.
  return !SDValue(N, 0).use_empty() && hasNoCarryFlagUses(SDValue(N, 1));
}]>;

#if INTEL_CUSTOMIZATION
let Defs = [EFLAGS] in {
let Constraints = "$src1 = $dst", SchedRW = [WriteALU] in {
// Short forms only valid in 32-bit mode. Selected during MCInst lowering.
let CodeSize = 1, hasSideEffects = 0 in {
def INC16r_alt : INCDECR_ALT<0x40, "inc", Xi16>;
def INC32r_alt : INCDECR_ALT<0x40, "inc", Xi32>;

// Short forms only valid in 32-bit mode. Selected during MCInst lowering.
def DEC16r_alt : INCDECR_ALT<0x48, "dec", Xi16>;
def DEC32r_alt : INCDECR_ALT<0x48, "dec", Xi32>;
} // CodeSize = 1, hasSideEffects = 0
} // Constraints = "$src1 = $dst", SchedRW
} // Defs = [EFLAGS]

let SchedRW = [WriteALU] in {
  // Can xform into LEA.
defm INC8r  : INCDECR<MRM0r, "inc", Xi8, X86add_flag_nocf>;
defm INC16r : INCDECR<MRM0r, "inc", Xi16, X86add_flag_nocf>;
defm INC32r : INCDECR<MRM0r, "inc", Xi32, X86add_flag_nocf>;
defm INC64r : INCDECR<MRM0r, "inc", Xi64, X86add_flag_nocf>;
  // Can xform into LEA.
defm DEC8r  : INCDECR<MRM1r, "dec", Xi8, X86sub_flag_nocf>;
defm DEC16r : INCDECR<MRM1r, "dec", Xi16, X86sub_flag_nocf>;
defm DEC32r : INCDECR<MRM1r, "dec", Xi32, X86sub_flag_nocf>;
defm DEC64r : INCDECR<MRM1r, "dec", Xi64, X86sub_flag_nocf>;
} // SchedRW

let SchedRW = [WriteALURMW] in {
  defm INC8m  : INCDECM<MRM0m, "inc", Xi8, 1>;
  defm INC16m : INCDECM<MRM0m, "inc", Xi16, 1>;
  defm INC32m : INCDECM<MRM0m, "inc", Xi32, 1>;
  defm INC64m : INCDECM<MRM0m, "inc", Xi64, 1>;

  defm DEC8m  : INCDECM<MRM1m, "dec", Xi8, -1>;
  defm DEC16m : INCDECM<MRM1m, "dec", Xi16, -1>;
  defm DEC32m : INCDECM<MRM1m, "dec", Xi32, -1>;
  defm DEC64m : INCDECM<MRM1m, "dec", Xi64, -1>;
} // SchedRW

// Extra precision multiplication

// AL is really implied by AX, but the registers in Defs must match the
// SDNode results (i8, i32).
// AL,AH = AL*GR8
defm MUL8r  : MulOpR<0xF6, MRM4r, "mul", Xi8, AL, AX, WriteIMul8,
               // FIXME: Used for 8-bit mul, ignore result upper 8 bits.
               // This probably ought to be moved to a def : Pat<> if the
               // syntax can be accepted.
               [(set AL, (mul AL, GR8:$src)), (implicit EFLAGS)]>;
// AX,DX = AX*GR16
let  hasSideEffects = 0 in{
defm MUL16r : MulOpR<0xF7, MRM4r, "mul", Xi16, AX, DX, WriteIMul16, []>;
// EAX,EDX = EAX*GR32
defm MUL32r : MulOpR<0xF7, MRM4r, "mul", Xi32, EAX, EDX, WriteIMul32,
               [/*(set EAX, EDX, EFLAGS, (X86umul_flag EAX, GR32:$src))*/]>;
// RAX,RDX = RAX*GR64
defm MUL64r : MulOpR<0xF7, MRM4r, "mul", Xi64, RAX, RDX, WriteIMul64,
                [/*(set RAX, RDX, EFLAGS, (X86umul_flag RAX, GR64:$src))*/]>;
// AL,AH = AL*[mem8]
}

defm MUL8m  : MulOpM<0xF6, MRM4m, "mul", Xi8, AL, AX, WriteIMul8,
               // FIXME: Used for 8-bit mul, ignore result upper 8 bits.
               // This probably ought to be moved to a def : Pat<> if the
               // syntax can be accepted.
               [(set AL, (mul AL, (loadi8 addr:$src))),
                (implicit EFLAGS)]>;
// AX,DX = AX*[mem16]
let mayLoad = 1, hasSideEffects = 0 in {
defm MUL16m : MulOpM<0xF7, MRM4m, "mul", Xi16, AX, DX, WriteIMul16, []>;
// EAX,EDX = EAX*[mem32]
defm MUL32m : MulOpM<0xF7, MRM4m, "mul", Xi32, EAX, EDX, WriteIMul32, []>;
// RAX,RDX = RAX*[mem64]
defm MUL64m : MulOpM<0xF7, MRM4m, "mul", Xi64, RAX, RDX, WriteIMul64, []>;
}

let hasSideEffects = 0 in {
// AL,AH = AL*GR8
defm IMUL8r  : MulOpR<0xF6, MRM5r, "imul", Xi8, AL, AX, WriteIMul8, []>;
// AX,DX = AX*GR16
defm IMUL16r : MulOpR<0xF7, MRM5r, "imul", Xi16, AX, DX, WriteIMul16, []>;
// EAX,EDX = EAX*GR32
defm IMUL32r : MulOpR<0xF7, MRM5r, "imul", Xi32, EAX, EDX, WriteIMul32, []>;
// RAX,RDX = RAX*GR64
defm IMUL64r : MulOpR<0xF7, MRM5r, "imul", Xi64, RAX, RDX, WriteIMul64, []>;

let mayLoad = 1 in {
// AL,AH = AL*[mem8]
defm IMUL8m  : MulOpM<0xF6, MRM5m, "imul", Xi8, AL, AX, WriteIMul8, []>;
// AX,DX = AX*[mem16]
defm IMUL16m : MulOpM<0xF7, MRM5m, "imul", Xi16, AX, DX, WriteIMul16, []>;
// EAX,EDX = EAX*[mem32]
defm IMUL32m : MulOpM<0xF7, MRM5m, "imul", Xi32, EAX, EDX, WriteIMul32, []>;
// RAX,RDX = RAX*[mem64]
defm IMUL64m : MulOpM<0xF7, MRM5m, "imul", Xi64, RAX, RDX, WriteIMul64, []>;
}

// X = IMUL Y, Z --> X = IMUL Z, Y
// Register-Register Signed Integer Multiply
defm IMUL16rr : IMulOpRR<0xAF, "imul", Xi16, WriteIMul16Reg>;
defm IMUL32rr : IMulOpRR<0xAF, "imul", Xi32, WriteIMul32Reg>;
defm IMUL64rr : IMulOpRR<0xAF, "imul", Xi64, WriteIMul64Reg>;

// Register-Memory Signed Integer Multiply
defm IMUL16rm : IMulOpRM<0xAF, "imul", Xi16, WriteIMul16Reg>;
defm IMUL32rm : IMulOpRM<0xAF, "imul", Xi32, WriteIMul32Reg>;
defm IMUL64rm : IMulOpRM<0xAF, "imul", Xi64, WriteIMul64Reg>;
}

// Surprisingly enough, these are not two address instructions!
// NOTE: These are order specific, we want the ri8 forms to be listed
// first so that they are slightly preferred to the ri forms.

// Register-Integer Signed Integer Multiply
// GR16 = GR16*I8
defm IMUL16rri8 : IMulOpRRI8<0x6B, "imul", Xi16, WriteIMul16Imm>;
// GR16 = GR16*I16
defm IMUL16rri  : IMulOpRRI<0x69, "imul", Xi16, WriteIMul16Imm>;
// GR32 = GR32*I32
defm IMUL32rri  : IMulOpRRI<0x69, "imul", Xi32, WriteIMul32Imm>;
// GR32 = GR32*I8
defm IMUL32rri8 : IMulOpRRI8<0x6B, "imul", Xi32, WriteIMul32Imm>;
// GR64 = GR64*I8
defm IMUL64rri8 : IMulOpRRI8<0x6B, "imul", Xi64, WriteIMul64Imm>;
// GR64 = GR64*I32
defm IMUL64rri32 : IMulOpRRI<0x69, "imul", Xi64, WriteIMul64Imm>;

// Memory-Integer Signed Integer Multiply
// GR16 = [mem16]*I8
defm IMUL16rmi8 : IMulOpRMI8<0x6B, "imul", Xi16, WriteIMul16Imm>;
// GR16 = [mem16]*I16
defm IMUL16rmi  : IMulOpRMI<0x69, "imul", Xi16, WriteIMul16Imm>;
// GR32 = [mem32]*I8
defm IMUL32rmi8 : IMulOpRMI8<0x6B, "imul", Xi32, WriteIMul32Imm>;
// GR32 = [mem32]*I32
defm IMUL32rmi  : IMulOpRMI<0x69, "imul", Xi32, WriteIMul32Imm>;
// GR64 = [mem64]*I8
defm IMUL64rmi8 : IMulOpRMI8<0x6B, "imul", Xi64, WriteIMul64Imm>;
// GR64 = [mem64]*I32
defm IMUL64rmi32 : IMulOpRMI<0x69, "imul", Xi64, WriteIMul64Imm>;

// unsigned division/remainder
let hasSideEffects = 1 in { // so that we don't speculatively execute
// AX/r8 = AL,AH
defm DIV8r  : DivOpR<0xF6, MRM6r, "div", Xi8, AL, AH, [AX], WriteDiv8, []>;
// DX:AX/r16 = AX,DX
defm DIV16r : DivOpR<0xF7, MRM6r, "div", Xi16, AX, DX, [AX, DX], WriteDiv16, []>;
// EDX:EAX/r32 = EAX,EDX
defm DIV32r : DivOpR<0xF7, MRM6r, "div", Xi32, EAX, EDX, [EAX, EDX], WriteDiv32, []>;
// RDX:RAX/r64 = RAX,RDX
defm DIV64r : DivOpR<0xF7, MRM6r, "div", Xi64, RAX, RDX, [RAX, RDX], WriteDiv64, []>;

let mayLoad = 1 in {
// AX/[mem8] = AL,AH
defm DIV8m  : DivOpM<0xF6, MRM6m, "div", Xi8, AL, AH, [AX], WriteDiv8, []>;
// DX:AX/[mem16] = AX,DX
defm DIV16m : DivOpM<0xF7, MRM6m, "div", Xi16, AX, DX, [AX, DX], WriteDiv16, []>;
// EDX:EAX/[mem32] = EAX,EDX
defm DIV32m : DivOpM<0xF7, MRM6m, "div", Xi32, EAX, EDX, [EAX, EDX], WriteDiv32, []>;
// RDX:RAX/[mem64] = RAX,RDX
defm DIV64m : DivOpM<0xF7, MRM6m, "div", Xi64, RAX, RDX, [RAX, RDX], WriteDiv64, []>;
}

// Signed division/remainder.
// AX/r8 = AL,AH
defm IDIV8r : DivOpR<0xF6, MRM7r, "idiv", Xi8, AL, AH, [AX], WriteIDiv8, []>;
// DX:AX/r16 = AX,DX
defm IDIV16r: DivOpR<0xF7, MRM7r, "idiv", Xi16, AX, DX, [AX, DX], WriteIDiv16, []>;
// EDX:EAX/r32 = EAX,EDX
defm IDIV32r: DivOpR<0xF7, MRM7r, "idiv", Xi32, EAX, EDX, [EAX, EDX], WriteIDiv32, []>;
// RDX:RAX/r64 = RAX,RDX
defm IDIV64r: DivOpR<0xF7, MRM7r, "idiv", Xi64, RAX, RDX, [RAX, RDX], WriteIDiv64, []>;

let mayLoad = 1 in {
// AX/[mem8] = AL,AH
defm IDIV8m : DivOpM<0xF6, MRM7m, "idiv", Xi8, AL, AH, [AX], WriteIDiv8, []>;
// DX:AX/[mem16] = AX,DX
defm IDIV16m: DivOpM<0xF7, MRM7m, "idiv", Xi16, AX, DX, [AX, DX], WriteIDiv16, []>;
// EDX:EAX/[mem32] = EAX,EDX
defm IDIV32m: DivOpM<0xF7, MRM7m, "idiv", Xi32, EAX, EDX, [EAX, EDX], WriteIDiv32, []>;
// RDX:RAX/[mem64] = RAX,RDX
defm IDIV64m: DivOpM<0xF7, MRM7m, "idiv", Xi64, RAX, RDX, [RAX, RDX], WriteIDiv64, []>;
}
} // hasSideEffects = 0

//===----------------------------------------------------------------------===//
//  Two address Instructions.
//

// unary instructions
let SchedRW = [WriteALU] in {
defm NEG8r  : NegOpR<0xF6, "neg", Xi8>;
defm NEG16r : NegOpR<0xF7, "neg", Xi16>;
defm NEG32r : NegOpR<0xF7, "neg", Xi32>;
defm NEG64r : NegOpR<0xF7, "neg", Xi64>;
} // SchedRW

// Read-modify-write negate.
let SchedRW = [WriteALURMW] in {
defm NEG8m  : NegOpM<0xF6, "neg", Xi8>;
defm NEG16m : NegOpM<0xF7, "neg", Xi16>;
defm NEG32m : NegOpM<0xF7, "neg", Xi32>;
defm NEG64m : NegOpM<0xF7, "neg", Xi64>;
} // SchedRW

// Note: NOT does not set EFLAGS!
let SchedRW = [WriteALU] in {
defm NOT8r  : NotOpR<0xF6, "not", Xi8>;
defm NOT16r : NotOpR<0xF7, "not", Xi16>;
defm NOT32r : NotOpR<0xF7, "not", Xi32>;
defm NOT64r : NotOpR<0xF7, "not", Xi64>;
} // SchedRW

let SchedRW = [WriteALURMW] in {
defm NOT8m  : NotOpM<0xF6, "not", Xi8>;
defm NOT16m : NotOpM<0xF7, "not", Xi16>;
defm NOT32m : NotOpM<0xF7, "not", Xi32>;
defm NOT64m : NotOpM<0xF7, "not", Xi64>;
} // SchedRW

/// ArithBinOp_R - This is an arithmetic binary operator where the pattern is
/// defined with "(set GPR:$dst, (...".
///
/// It would be nice to get rid of the second and third argument here, but
/// tblgen can't handle dependent type references aggressively enough: PR8330
///
/// This is a multiclass contains legacy and APX version.
multiclass ArithBinOp_RF<bits<8> BaseOpc, bits<8> BaseOpc2, bits<8> BaseOpc4,
                         string mnemonic, Format RegMRM, Format MemMRM,
                         SDNode opnodeflag, SDNode opnode, bit CommutableRR,
                         bit ConvertibleToThreeAddress,
                         bit ConvertibleToThreeAddressRR> {
  defm NAME#8rr  : BinOpRR_RF_Common<BaseOpc, mnemonic, Xi8,  opnodeflag,
                                     CommutableRR, ConvertibleToThreeAddressRR>;
  defm NAME#16rr : BinOpRR_RF_Common<BaseOpc, mnemonic, Xi16, opnodeflag,
                                     CommutableRR, ConvertibleToThreeAddressRR>;
  defm NAME#32rr : BinOpRR_RF_Common<BaseOpc, mnemonic, Xi32, opnodeflag,
                                     CommutableRR, ConvertibleToThreeAddressRR>;
  defm NAME#64rr : BinOpRR_RF_Common<BaseOpc, mnemonic, Xi64, opnodeflag,
                                     CommutableRR, ConvertibleToThreeAddressRR>;

  defm NAME#8rr  : BinOpRR_Rev_Common<BaseOpc2, mnemonic, Xi8>;
  defm NAME#16rr : BinOpRR_Rev_Common<BaseOpc2, mnemonic, Xi16>;
  defm NAME#32rr : BinOpRR_Rev_Common<BaseOpc2, mnemonic, Xi32>;
  defm NAME#64rr : BinOpRR_Rev_Common<BaseOpc2, mnemonic, Xi64>;

  defm NAME#8rm   : BinOpRM_RF_Common<BaseOpc2, mnemonic, Xi8 , opnodeflag>;
  defm NAME#16rm  : BinOpRM_RF_Common<BaseOpc2, mnemonic, Xi16, opnodeflag>;
  defm NAME#32rm  : BinOpRM_RF_Common<BaseOpc2, mnemonic, Xi32, opnodeflag>;
  defm NAME#64rm  : BinOpRM_RF_Common<BaseOpc2, mnemonic, Xi64, opnodeflag>;

  // NOTE: These are order specific, we want the ri8 forms to be listed
  // first so that they are slightly preferred to the ri forms.
  let isConvertibleToThreeAddress = ConvertibleToThreeAddress, hasSideEffects= 0 in {
    defm NAME#16ri8 : BinOpRI8_RF_Common<0x82, mnemonic, Xi16, RegMRM>;
    defm NAME#32ri8 : BinOpRI8_RF_Common<0x82, mnemonic, Xi32, RegMRM>;
    defm NAME#64ri8 : BinOpRI8_RF_Common<0x82, mnemonic, Xi64, RegMRM>;
    defm NAME#8ri   : BinOpRI_RF_Common<0x80, mnemonic, Xi8 , opnodeflag,
                                        RegMRM>;
    defm NAME#16ri  : BinOpRI_RF_Common<0x80, mnemonic, Xi16, opnodeflag,
                                        RegMRM>;
    defm NAME#32ri  : BinOpRI_RF_Common<0x80, mnemonic, Xi32, opnodeflag,
                                        RegMRM>;
    defm NAME#64ri32: BinOpRI_RF_Common<0x80, mnemonic, Xi64, opnodeflag,
                                        RegMRM>;
  }

  let mayLoad = 1 in {
    defm NAME#8mr    : BinOpMR_Common<BaseOpc, mnemonic, Xi8 , opnode>;
    defm NAME#16mr   : BinOpMR_Common<BaseOpc, mnemonic, Xi16, opnode>;
    defm NAME#32mr   : BinOpMR_Common<BaseOpc, mnemonic, Xi32, opnode>;
    defm NAME#64mr   : BinOpMR_Common<BaseOpc, mnemonic, Xi64, opnode>;
  }

  // NOTE: These are order specific, we want the mi8 forms to be listed
  // first so that they are slightly preferred to the mi forms.
let mayLoad = 1, hasSideEffects = 0 in {
  defm NAME#16mi8  : BinOpMI8_RMW_Common<mnemonic, Xi16, MemMRM>;
  defm NAME#32mi8  : BinOpMI8_RMW_Common<mnemonic, Xi32, MemMRM>;
  defm NAME#64mi8  : BinOpMI8_RMW_Common<mnemonic, Xi64, MemMRM,
                                         In64BitMode>;

  defm NAME#8mi    : BinOpMI_Common<0x80, mnemonic, Xi8 , opnode, MemMRM>;
  defm NAME#16mi   : BinOpMI_Common<0x80, mnemonic, Xi16, opnode, MemMRM>;
  defm NAME#32mi   : BinOpMI_Common<0x80, mnemonic, Xi32, opnode, MemMRM>;
  defm NAME#64mi32 : BinOpMI_Common<0x80, mnemonic, Xi64, opnode, MemMRM,
                                    In64BitMode>;
  // These are for the disassembler since 0x82 opcode behaves like 0x80, but
  // not in 64-bit mode.
  let mayStore = 1, Predicates = [Not64BitMode], isCodeGenOnly = 1,
      ForceDisassemble = 1, Defs = [EFLAGS] in {
    let Constraints = "$src1 = $dst" in
      def NAME#8ri8 : BinOpRI8_RF<0x82, mnemonic, Xi8, RegMRM>;
    let mayLoad = 1, mayStore = 1 in
      def NAME#8mi8 : BinOpMI8_RMW<mnemonic, Xi8, MemMRM>;
  }
}

  def NAME#8i8   : BinOpAI<BaseOpc4, mnemonic, Xi8 , AL,
                           "{$src, %al|al, $src}">;
  def NAME#16i16 : BinOpAI<BaseOpc4, mnemonic, Xi16, AX,
                           "{$src, %ax|ax, $src}">;
  def NAME#32i32 : BinOpAI<BaseOpc4, mnemonic, Xi32, EAX,
                           "{$src, %eax|eax, $src}">;
  def NAME#64i32 : BinOpAI<BaseOpc4, mnemonic, Xi64, RAX,
                           "{$src, %rax|rax, $src}">;
}

// BinOpRR_RFF_ND - Binary instructions with inputs "reg, reg", where the
// pattern has both a regclass and EFLAGS as a result, and has EFLAGS as input,
// ndd version.
class BinOpRR_RFF_ND<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                     SDNode opnode>
  : BinOpRR_NDD<opcode, MRMDestReg, mnemonic, typeinfo,
                [(set typeinfo.RegClass:$dst, EFLAGS,
                 (opnode typeinfo.RegClass:$src1,
                         typeinfo.RegClass:$src2, EFLAGS))]>,
    Sched<[WriteADC]>;

// BinOpRR_RFF_common - Binary instructions with inputs "reg, reg", where the
// pattern has both a regclass and EFLAGS as a result, and has EFLAGS as input,
// contains legacy and APX version.
multiclass BinOpRR_RFF_common<bits<8> opcode, string mnemonic,
                              X86TypeInfo typeinfo, SDNode node>{
let Constraints = "$src1 = $dst" in {
  let Predicates = [NoNDD] in
  def NAME : BinOpRR_RFF<opcode, mnemonic, typeinfo, node>;
  let Predicates = [In64BitMode] in
  def _EVEX : BinOpRR_EVEX<opcode, MRMDestReg, mnemonic, typeinfo,[]>,
              EmitVEXOrEVEXPrefix, Sched<[WriteADC]>;
}
  let Predicates = [HasNDD, In64BitMode] in
  def _ND : BinOpRR_RFF_ND<opcode, mnemonic, typeinfo, node>, EVEX_B4V;
}

// BinOpRR_RFF_Rev_common - Binary instructions with inputs "reg, reg"(reversed
// encoding), with sched = WriteADC, contains legacy and APX version.
multiclass BinOpRR_RFF_Rev_common<bits<8> opcode, string mnemonic,
                                  X86TypeInfo typeinfo>{
let Constraints = "$src1 = $dst" in {
  let Predicates = [NoNDD] in
  def _REV : BinOpRR_RFF_Rev<opcode, mnemonic, typeinfo>;
  let Predicates = [In64BitMode] in
  def _EVEX_REV : BinOpRR_Rev_EVEX<opcode, mnemonic, typeinfo, WriteADC>,
                  EmitVEXOrEVEXPrefix;
}
  let Predicates = [HasNDD, In64BitMode] in
  def _ND_REV : BinOpRR_Rev_ND<opcode, mnemonic, typeinfo, WriteADC>,
                EVEX_B4V;
}

// BinOpRM_ImplicitUse_EVEX - Binary instructions with inputs "reg, [mem]".
class BinOpRM_ImplicitUse_EVEX<bits<8> opcode, string mnemonic,
                               X86TypeInfo typeinfo,
                               X86FoldableSchedWrite sched, string asm,
                               list<dag> pattern>
  : ITy_EVEX<opcode, MRMSrcMem, typeinfo, (outs typeinfo.RegClass:$dst),
             (ins typeinfo.RegClass:$src1, typeinfo.MemOperand:$src2),
             mnemonic, asm, pattern>,
    Sched<[sched.Folded, sched.ReadAfterFold,
           // base, scale, index, offset, segment.
           ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault,
           // implicit register read.
           sched.ReadAfterFold]>;

// BinOpRM_RFF_EVEX - Binary instructions with inputs "reg, [mem]",
// evex encoding.
class BinOpRM_RFF_EVEX<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo>
  : BinOpRM_ImplicitUse_EVEX<opcode, mnemonic, typeinfo, WriteADC,
                             "{$src2, $src1|$src1, $src2}", []>;

// BinOpRM_RFF_ND - Binary instructions with inputs "reg, reg, [mem]", where
// the pattern has both a regclass and EFLAGS as a result, and has EFLAGS as
// input, ndd version.
class BinOpRM_RFF_ND<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                     SDNode opnode>
  : BinOpRM_ImplicitUse_EVEX<opcode, mnemonic, typeinfo, WriteADC,
                             "{$src2, $src1, $dst|$dst, $src1, $src2}",
                             [(set typeinfo.RegClass:$dst, EFLAGS,
                              (opnode typeinfo.RegClass:$src1,
                              (typeinfo.LoadNode addr:$src2), EFLAGS))]>;

// BinOpRM_RFF_Common - Binary instructions with inputs "reg, [mem]", where
// the pattern has both a regclass and EFLAGS as a result, and has EFLAGS as
// input, contains legacy and APX version.
multiclass BinOpRM_RFF_Common<bits<8> opcode, string mnemonic,
                              X86TypeInfo typeinfo, SDNode opnode>{
let Constraints = "$src1 = $dst" in {
  let Predicates = [NoNDD] in
  def NAME : BinOpRM_RFF<opcode, mnemonic, typeinfo, opnode>;
  let Predicates = [In64BitMode], mayLoad = 1 in
  def _EVEX : BinOpRM_RFF_EVEX<opcode, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
}
  let Predicates = [HasNDD, In64BitMode] in
  def _ND : BinOpRM_RFF_ND<opcode, mnemonic, typeinfo, opnode>, EVEX_B4V;
}

// BinOpRI_RFF_EVEX - Binary instructions with inputs "reg, imm",
// evex encoding.
class BinOpRI_RFF_EVEX<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                       Format f>
  : BinOpRI_RF_EVEX<opcode, mnemonic, typeinfo, f, [], WriteADC>;

// BinOpRI_RFF_ND - Binary instructions with inputs "reg, reg, imm", where the
// pattern has both a regclass and EFLAGS as a result, and has EFLAGS as input,
// ndd version.
class BinOpRI_RFF_ND<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                     SDNode opnode, Format f>
  : BinOpRI_RF_ND<opcode, mnemonic, typeinfo, f,
                  [(set typeinfo.RegClass:$dst, EFLAGS,
                   (opnode typeinfo.RegClass:$src1,
                           typeinfo.ImmOperator:$src2, EFLAGS))], WriteADC>;

// BinOpRI_RFF_Common - Binary instructions with inputs "reg, imm", where the
// pattern has both a regclass and EFLAGS as a result, and has EFLAGS as input,
// contains legacy and APX version.
multiclass BinOpRI_RFF_Common<bits<8> opcode, string mnemonic,
                              X86TypeInfo typeinfo, SDNode opnode, Format f>{
let Constraints = "$src1 = $dst" in {
  let Predicates = [NoNDD] in
  def NAME : BinOpRI_RFF<opcode, mnemonic, typeinfo, opnode, f>;
  let Predicates = [In64BitMode] in
  def _EVEX : BinOpRI_RFF_EVEX<opcode, mnemonic, typeinfo, f>,
              EmitVEXOrEVEXPrefix;
}
  let Predicates = [HasNDD, In64BitMode] in
  def _ND : BinOpRI_RFF_ND<opcode, mnemonic, typeinfo, opnode, f>, EVEX_B4V;
}

// BinOpRI8_RFF_EVEX - Binary instructions with inputs "reg, imm8",
// evex encoding.
class BinOpRI8_RFF_EVEX<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                        Format f>
  : BinOpRI8_RF_EVEX<opcode, mnemonic, typeinfo, f, WriteADC, []>;

// BinOpRI8_RFF_ND - Binary instructions with inputs "reg, imm8", where the
// pattern has both a regclass and EFLAGS as a result, and has EFLAGS as input,
// ndd version.
class BinOpRI8_RFF_ND<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                      Format f>
  : BinOpRI8_RF_ND<opcode, mnemonic, typeinfo, f, [], WriteADC>;

// BinOpRI8_RFF_Common - Binary instructions with inputs "reg, imm8", where the
// pattern has both a regclass and EFLAGS as a result, and has EFLAGS as input,
// contains legacy and APX version.
multiclass BinOpRI8_RFF_Common<bits<8> opcode, string mnemonic,
                               X86TypeInfo typeinfo, Format f>{
let Constraints = "$src1 = $dst" in {
  let Predicates = [NoNDD] in
  def NAME : BinOpRI8_RFF<opcode, mnemonic, typeinfo, f>;
  let Predicates = [In64BitMode] in
  def _EVEX : BinOpRI8_RFF_EVEX<opcode, mnemonic, typeinfo, f>,
              EmitVEXOrEVEXPrefix;
}
  let Predicates = [HasNDD, In64BitMode] in
  def _ND : BinOpRI8_RFF_ND<opcode, mnemonic, typeinfo, f>, EVEX_B4V;
}

// BinOpMR_EVEX - Binary instructions with inputs "[mem], reg", evex encoding.
class BinOpMR_EVEX<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                   dag outlist, string asm, list<dag> pattern>
  : ITy_EVEX<opcode, MRMDestMem, typeinfo, outlist,
             (ins typeinfo.MemOperand:$src1, typeinfo.RegClass:$src2),
             mnemonic, asm, pattern>;

// BinOpMR_RMW_FF_EVEX - Binary instructions with inputs "[mem], reg",
// evex encoding.
class BinOpMR_RMW_FF_EVEX<bits<8> opcode, string mnemonic,
                          X86TypeInfo typeinfo>
  : BinOpMR_EVEX<opcode, mnemonic, typeinfo, (outs),
                 "{$src2, $src1|$src1, $src2}", []>,
    Sched<[WriteADCRMW,
          // base, scale, index, offset, segment
          ReadDefault, ReadDefault, ReadDefault,
          ReadDefault, ReadDefault,
          WriteALU.ReadAfterFold,    // reg
          WriteALU.ReadAfterFold]>;  // EFLAGS

// BinOpMR_FF_ND - Binary instructions with inputs "reg, [mem], reg", where
// the pattern sets EFLAGS and implicitly uses EFLAGS, ndd version.
class BinOpMR_FF_ND<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                    SDNode opnode>
  : BinOpMR_EVEX<opcode, mnemonic, typeinfo, (outs typeinfo.RegClass:$dst),
                 "{$src2, $src1, $dst|$dst, $src1, $src2}",
                 [(set typeinfo.RegClass:$dst, (opnode (load addr:$src1),
                       typeinfo.RegClass:$src2, EFLAGS)), (implicit EFLAGS)]>,
    Sched<[WriteADC.Folded, WriteADC.ReadAfterFold,
           // base, scale, index, offset, segment.
           ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault,
           // implicit register read.
           WriteADC.ReadAfterFold]>;  // EFLAGS

// BinOpMR_RMW_FF_Common - Binary instructions with inputs "[mem], reg", where
// the pattern sets EFLAGS and implicitly uses EFLAGS, contains legacy and
// APX version.
multiclass BinOpMR_RMW_FF_Common<bits<8> opcode, string mnemonic,
                                 X86TypeInfo typeinfo, SDNode opnode>{
let Predicates = [NoNDD] in
  def NAME : BinOpMR_RMW_FF<opcode, mnemonic, typeinfo, opnode>;
let Predicates = [In64BitMode], mayLoad = 1, mayStore = 1 in
  def _EVEX : BinOpMR_RMW_FF_EVEX<opcode, mnemonic, typeinfo>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : BinOpMR_FF_ND<opcode, mnemonic, typeinfo, opnode>, EVEX_B4V;
}

// BinOpMI8_RMW_FF_EVEX - Binary instructions with inputs "[mem], imm8",
// evex encoding.
class BinOpMI8_RMW_FF_EVEX<string mnemonic, X86TypeInfo typeinfo, Format f>
  : BinOpMI8_EVEX<mnemonic, typeinfo, (outs),
                  "{$src2, $src1|$src1, $src2}", f, []>,
    Sched<[WriteADCRMW]>;

// BinOpMI8_FF_ND - Binary instructions with inputs "reg, [mem], imm8",
// where the pattern sets EFLAGS and implicitly uses EFLAGS, ndd version.
class BinOpMI8_FF_ND<string mnemonic, X86TypeInfo typeinfo, Format f>
  : BinOpMI8_EVEX<mnemonic, typeinfo, (outs typeinfo.RegClass:$dst),
                  "{$src2, $src1, $dst|$dst, $src1, $src2}", f, []>,
    Sched<[WriteADC]>;

// BinOpMI8_RMW_FF_Common - Binary instructions with inputs "[mem], imm8",
// where the pattern sets EFLAGS and implicitly uses EFLAGS, contains legacy
// and APX version.
multiclass BinOpMI8_RMW_FF_Common<string mnemonic, X86TypeInfo typeinfo,
                                  Format f, Predicate pred = TruePredicate> {
let Predicates = [pred], mayStore = 1 in
  def NAME : BinOpMI8_RMW_FF<mnemonic, typeinfo, f>;
let Predicates = [In64BitMode], mayStore = 1 in
  def _EVEX : BinOpMI8_RMW_FF_EVEX<mnemonic, typeinfo, f>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : BinOpMI8_FF_ND<mnemonic, typeinfo, f>, EVEX_B4V;
}

// BinOpMI_RMW_FF_EVEX - Binary instructions with inputs "[mem], imm",
// evex encoding.
class BinOpMI_RMW_FF_EVEX<bits<8> opcode, string mnemonic,
                          X86TypeInfo typeinfo, Format f>
  : BinOpMI_EVEX<opcode, mnemonic, typeinfo, (outs),
                 "{$src2, $src1|$src1, $src2}", f, []>,
    Sched<[WriteADCRMW]>;

// BinOpMI_FF_ND - Binary instructions with inputs "[mem], imm", where
// the pattern sets EFLAGS and implicitly uses EFLAGS, ndd version.
class BinOpMI_FF_ND<bits<8> opcode, string mnemonic, X86TypeInfo typeinfo,
                    SDNode opnode, Format f>
  : BinOpMI_EVEX<opcode, mnemonic, typeinfo, (outs typeinfo.RegClass:$dst),
                 "{$src2, $src1, $dst|$dst, $src1, $src2}", f,
                 [(set typeinfo.RegClass:$dst,
                  (opnode (typeinfo.VT (load addr:$src1)),
                   typeinfo.ImmOperator:$src2, EFLAGS)), (implicit EFLAGS)]>,
    Sched<[WriteADC]>;

// BinOpMI_RMW_FF_Common - Binary instructions with inputs "[mem], imm", where
// the pattern sets EFLAGS and implicitly uses EFLAGS, contains legacy
// and APX version.
multiclass BinOpMI_RMW_FF_Common<bits<8> opcode, string mnemonic,
                                 X86TypeInfo typeinfo, SDNode opnode,
                                 Format f, Predicate pred = TruePredicate>{
let Predicates = [pred], mayStore = 1 in
  def NAME : BinOpMI_RMW_FF<opcode, mnemonic, typeinfo, opnode, f>;
let Predicates = [In64BitMode], mayStore = 1 in
  def _EVEX : BinOpMI_RMW_FF_EVEX<opcode, mnemonic, typeinfo, f>,
              EmitVEXOrEVEXPrefix;
let Predicates = [HasNDD, In64BitMode] in
  def _ND : BinOpMI_FF_ND<opcode, mnemonic, typeinfo, opnode, f>, EVEX_B4V;
}

/// ArithBinOp_RFF - This is an arithmetic binary operator where the pattern is
/// defined with "(set GPR:$dst, EFLAGS, (node LHS, RHS, EFLAGS))" like ADC and
/// SBB.
///
/// It would be nice to get rid of the second and third argument here, but
/// tblgen can't handle dependent type references aggressively enough: PR8330
///
/// It is a general multiclass which contains legacy and APX version.
multiclass ArithBinOp_RFF<bits<8> BaseOpc, bits<8> BaseOpc2, bits<8> BaseOpc4,
                          string mnemonic, Format RegMRM, Format MemMRM,
                          SDNode opnode, bit CommutableRR,
                          bit ConvertibleToThreeAddress> {
  let Uses = [EFLAGS], Defs = [EFLAGS] in {
      let isCommutable = CommutableRR in {
        defm NAME#8rr  : BinOpRR_RFF_common<BaseOpc, mnemonic, Xi8 , opnode>;
        let isConvertibleToThreeAddress = ConvertibleToThreeAddress in {
          defm NAME#16rr : BinOpRR_RFF_common<BaseOpc, mnemonic, Xi16, opnode>;
          defm NAME#32rr : BinOpRR_RFF_common<BaseOpc, mnemonic, Xi32, opnode>;
          defm NAME#64rr : BinOpRR_RFF_common<BaseOpc, mnemonic, Xi64, opnode>;
        } // isConvertibleToThreeAddress
      } // isCommutable

      defm NAME#8rr  : BinOpRR_RFF_Rev_common<BaseOpc2, mnemonic, Xi8>;
      defm NAME#16rr : BinOpRR_RFF_Rev_common<BaseOpc2, mnemonic, Xi16>;
      defm NAME#32rr : BinOpRR_RFF_Rev_common<BaseOpc2, mnemonic, Xi32>;
      defm NAME#64rr : BinOpRR_RFF_Rev_common<BaseOpc2, mnemonic, Xi64>;

      defm NAME#8rm   : BinOpRM_RFF_Common<BaseOpc2, mnemonic, Xi8 , opnode>;
      defm NAME#16rm  : BinOpRM_RFF_Common<BaseOpc2, mnemonic, Xi16, opnode>;
      defm NAME#32rm  : BinOpRM_RFF_Common<BaseOpc2, mnemonic, Xi32, opnode>;
      defm NAME#64rm  : BinOpRM_RFF_Common<BaseOpc2, mnemonic, Xi64, opnode>;

      defm NAME#8ri   : BinOpRI_RFF_Common<0x80, mnemonic, Xi8 , opnode,
                                           RegMRM>;

      let isConvertibleToThreeAddress = ConvertibleToThreeAddress, hasSideEffects = 0 in {
        // NOTE: These are order specific, we want the ri8 forms to be listed
        // first so that they are slightly preferred to the ri forms.
        defm NAME#16ri8 : BinOpRI8_RFF_Common<0x82, mnemonic, Xi16, RegMRM>;
        defm NAME#32ri8 : BinOpRI8_RFF_Common<0x82, mnemonic, Xi32, RegMRM>;
        defm NAME#64ri8 : BinOpRI8_RFF_Common<0x82, mnemonic, Xi64, RegMRM>;

        defm NAME#16ri  : BinOpRI_RFF_Common<0x80, mnemonic, Xi16, opnode,
                                             RegMRM>;
        defm NAME#32ri  : BinOpRI_RFF_Common<0x80, mnemonic, Xi32, opnode,
                                             RegMRM>;
        defm NAME#64ri32: BinOpRI_RFF_Common<0x80, mnemonic, Xi64, opnode,
                                             RegMRM>;
      }

    defm NAME#8mr    : BinOpMR_RMW_FF_Common<BaseOpc, mnemonic, Xi8 , opnode>;
    defm NAME#16mr   : BinOpMR_RMW_FF_Common<BaseOpc, mnemonic, Xi16, opnode>;
    defm NAME#32mr   : BinOpMR_RMW_FF_Common<BaseOpc, mnemonic, Xi32, opnode>;
    defm NAME#64mr   : BinOpMR_RMW_FF_Common<BaseOpc, mnemonic, Xi64, opnode>;
    let mayLoad = 1, hasSideEffects = 0 in {
    // NOTE: These are order specific, we want the mi8 forms to be listed
    // first so that they are slightly preferred to the mi forms.
    defm NAME#16mi8  : BinOpMI8_RMW_FF_Common<mnemonic, Xi16, MemMRM>;
    defm NAME#32mi8  : BinOpMI8_RMW_FF_Common<mnemonic, Xi32, MemMRM>;
    defm NAME#64mi8  : BinOpMI8_RMW_FF_Common<mnemonic, Xi64, MemMRM,
                                              In64BitMode>;

    defm NAME#8mi    : BinOpMI_RMW_FF_Common<0x80, mnemonic, Xi8 , opnode,
                                             MemMRM>;
    defm NAME#16mi   : BinOpMI_RMW_FF_Common<0x80, mnemonic, Xi16, opnode,
                                             MemMRM>;
    defm NAME#32mi   : BinOpMI_RMW_FF_Common<0x80, mnemonic, Xi32, opnode,
                                             MemMRM>;
    defm NAME#64mi32 : BinOpMI_RMW_FF_Common<0x80, mnemonic, Xi64, opnode,
                                             MemMRM, In64BitMode>;
    }

    // These are for the disassembler since 0x82 opcode behaves like 0x80, but
    // not in 64-bit mode.
    let Predicates = [Not64BitMode], isCodeGenOnly = 1, ForceDisassemble = 1,
        hasSideEffects = 0 in {
      let Constraints = "$src1 = $dst" in
        def NAME#8ri8 : BinOpRI8_RFF<0x82, mnemonic, Xi8, RegMRM>;
      let mayLoad = 1, mayStore = 1 in
        def NAME#8mi8 : BinOpMI8_RMW_FF<mnemonic, Xi8, MemMRM>;
    }
  } // Uses = [EFLAGS], Defs = [EFLAGS]

  def NAME#8i8   : BinOpAI_RFF<BaseOpc4, mnemonic, Xi8 , AL,
                               "{$src, %al|al, $src}">;
  def NAME#16i16 : BinOpAI_RFF<BaseOpc4, mnemonic, Xi16, AX,
                               "{$src, %ax|ax, $src}">;
  def NAME#32i32 : BinOpAI_RFF<BaseOpc4, mnemonic, Xi32, EAX,
                               "{$src, %eax|eax, $src}">;
  def NAME#64i32 : BinOpAI_RFF<BaseOpc4, mnemonic, Xi64, RAX,
                               "{$src, %rax|rax, $src}">;
}
#endif // INTEL_CUSTOMIZATION

/// ArithBinOp_F - This is an arithmetic binary operator where the pattern is
/// defined with "(set EFLAGS, (...".  It would be really nice to find a way
/// to factor this with the other ArithBinOp_*.
///
multiclass ArithBinOp_F<bits<8> BaseOpc, bits<8> BaseOpc2, bits<8> BaseOpc4,
                        string mnemonic, Format RegMRM, Format MemMRM,
                        SDNode opnode,
                        bit CommutableRR, bit ConvertibleToThreeAddress> {
  let Defs = [EFLAGS] in {
    let isCommutable = CommutableRR in {
      def NAME#8rr  : BinOpRR_F<BaseOpc, mnemonic, Xi8 , opnode>;
      let isConvertibleToThreeAddress = ConvertibleToThreeAddress in {
        def NAME#16rr : BinOpRR_F<BaseOpc, mnemonic, Xi16, opnode>;
        def NAME#32rr : BinOpRR_F<BaseOpc, mnemonic, Xi32, opnode>;
        def NAME#64rr : BinOpRR_F<BaseOpc, mnemonic, Xi64, opnode>;
      }
    } // isCommutable

    def NAME#8rr_REV  : BinOpRR_F_Rev<BaseOpc2, mnemonic, Xi8>;
    def NAME#16rr_REV : BinOpRR_F_Rev<BaseOpc2, mnemonic, Xi16>;
    def NAME#32rr_REV : BinOpRR_F_Rev<BaseOpc2, mnemonic, Xi32>;
    def NAME#64rr_REV : BinOpRR_F_Rev<BaseOpc2, mnemonic, Xi64>;

    def NAME#8rm   : BinOpRM_F<BaseOpc2, mnemonic, Xi8 , opnode>;
    def NAME#16rm  : BinOpRM_F<BaseOpc2, mnemonic, Xi16, opnode>;
    def NAME#32rm  : BinOpRM_F<BaseOpc2, mnemonic, Xi32, opnode>;
    def NAME#64rm  : BinOpRM_F<BaseOpc2, mnemonic, Xi64, opnode>;

    def NAME#8ri   : BinOpRI_F<0x80, mnemonic, Xi8 , opnode, RegMRM>;

    let isConvertibleToThreeAddress = ConvertibleToThreeAddress, hasSideEffects = 0 in {
      // NOTE: These are order specific, we want the ri8 forms to be listed
      // first so that they are slightly preferred to the ri forms.
      def NAME#16ri8 : BinOpRI8_F<0x82, mnemonic, Xi16, RegMRM>;
      def NAME#32ri8 : BinOpRI8_F<0x82, mnemonic, Xi32, RegMRM>;
      def NAME#64ri8 : BinOpRI8_F<0x82, mnemonic, Xi64, RegMRM>;

      def NAME#16ri  : BinOpRI_F<0x80, mnemonic, Xi16, opnode, RegMRM>;
      def NAME#32ri  : BinOpRI_F<0x80, mnemonic, Xi32, opnode, RegMRM>;
      def NAME#64ri32: BinOpRI_F<0x80, mnemonic, Xi64, opnode, RegMRM>;
    }

    def NAME#8mr    : BinOpMR_F<BaseOpc, mnemonic, Xi8 , opnode>;
    def NAME#16mr   : BinOpMR_F<BaseOpc, mnemonic, Xi16, opnode>;
    def NAME#32mr   : BinOpMR_F<BaseOpc, mnemonic, Xi32, opnode>;
    def NAME#64mr   : BinOpMR_F<BaseOpc, mnemonic, Xi64, opnode>;

    // NOTE: These are order specific, we want the mi8 forms to be listed
    // first so that they are slightly preferred to the mi forms.
    let mayLoad = 1, hasSideEffects = 0 in {
      def NAME#16mi8  : BinOpMI8_F<mnemonic, Xi16, MemMRM>;
      def NAME#32mi8  : BinOpMI8_F<mnemonic, Xi32, MemMRM>;
      let Predicates = [In64BitMode] in
      def NAME#64mi8  : BinOpMI8_F<mnemonic, Xi64, MemMRM>;

      def NAME#8mi    : BinOpMI_F<0x80, mnemonic, Xi8 , opnode, MemMRM>;
      def NAME#16mi   : BinOpMI_F<0x80, mnemonic, Xi16, opnode, MemMRM>;
      def NAME#32mi   : BinOpMI_F<0x80, mnemonic, Xi32, opnode, MemMRM>;
      let Predicates = [In64BitMode] in
      def NAME#64mi32 : BinOpMI_F<0x80, mnemonic, Xi64, opnode, MemMRM>;
    }

    // These are for the disassembler since 0x82 opcode behaves like 0x80, but
    // not in 64-bit mode.
    let Predicates = [Not64BitMode], isCodeGenOnly = 1, ForceDisassemble = 1,
        hasSideEffects = 0 in {
      def NAME#8ri8 : BinOpRI8_F<0x82, mnemonic, Xi8, RegMRM>;
      let mayLoad = 1 in
        def NAME#8mi8 : BinOpMI8_F<mnemonic, Xi8, MemMRM>;
    }
  } // Defs = [EFLAGS]

  def NAME#8i8   : BinOpAI_F<BaseOpc4, mnemonic, Xi8 , AL,
                             "{$src, %al|al, $src}">;
  def NAME#16i16 : BinOpAI_F<BaseOpc4, mnemonic, Xi16, AX,
                             "{$src, %ax|ax, $src}">;
  def NAME#32i32 : BinOpAI_F<BaseOpc4, mnemonic, Xi32, EAX,
                             "{$src, %eax|eax, $src}">;
  def NAME#64i32 : BinOpAI_F<BaseOpc4, mnemonic, Xi64, RAX,
                             "{$src, %rax|rax, $src}">;
}

defm AND : ArithBinOp_RF<0x20, 0x22, 0x24, "and", MRM4r, MRM4m,
                         X86and_flag, and, 1, 0, 0>;
defm OR  : ArithBinOp_RF<0x08, 0x0A, 0x0C, "or", MRM1r, MRM1m,
                         X86or_flag, or, 1, 0, 0>;
defm XOR : ArithBinOp_RF<0x30, 0x32, 0x34, "xor", MRM6r, MRM6m,
                         X86xor_flag, xor, 1, 0, 0>;
defm ADD : ArithBinOp_RF<0x00, 0x02, 0x04, "add", MRM0r, MRM0m,
                         X86add_flag, add, 1, 1, 1>;
let isCompare = 1 in {
defm SUB : ArithBinOp_RF<0x28, 0x2A, 0x2C, "sub", MRM5r, MRM5m,
                         X86sub_flag, sub, 0, 1, 0>;
}

// Version of XOR8rr_NOREX that use GR8_NOREX. This is used by the handling of
// __builtin_parity where the last step xors an h-register with an l-register.
let isCodeGenOnly = 1, hasSideEffects = 0, Constraints = "$src1 = $dst",
    Defs = [EFLAGS], isCommutable = 1 in
def XOR8rr_NOREX : I<0x30, MRMDestReg, (outs GR8_NOREX:$dst),
                     (ins GR8_NOREX:$src1, GR8_NOREX:$src2),
                     "xor{b}\t{$src2, $dst|$dst, $src2}", []>,
                     Sched<[WriteALU]>;

// Arithmetic.
defm ADC : ArithBinOp_RFF<0x10, 0x12, 0x14, "adc", MRM2r, MRM2m, X86adc_flag,
                          1, 0>;
defm SBB : ArithBinOp_RFF<0x18, 0x1A, 0x1C, "sbb", MRM3r, MRM3m, X86sbb_flag,
                          0, 0>;

let isCompare = 1 in {
defm CMP : ArithBinOp_F<0x38, 0x3A, 0x3C, "cmp", MRM7r, MRM7m, X86cmp, 0, 0>;
}

// Patterns to recognize loads on the LHS of an ADC. We can't make X86adc_flag
// commutable since it has EFLAGs as an input.
def : Pat<(X86adc_flag (loadi8 addr:$src2), GR8:$src1, EFLAGS),
          (ADC8rm GR8:$src1, addr:$src2)>;
def : Pat<(X86adc_flag (loadi16 addr:$src2), GR16:$src1, EFLAGS),
          (ADC16rm GR16:$src1, addr:$src2)>;
def : Pat<(X86adc_flag (loadi32 addr:$src2), GR32:$src1, EFLAGS),
          (ADC32rm GR32:$src1, addr:$src2)>;
def : Pat<(X86adc_flag (loadi64 addr:$src2), GR64:$src1, EFLAGS),
          (ADC64rm GR64:$src1, addr:$src2)>;

// Patterns to recognize RMW ADC with loads in operand 1.
def : Pat<(store (X86adc_flag GR8:$src, (loadi8 addr:$dst), EFLAGS),
                 addr:$dst),
          (ADC8mr addr:$dst, GR8:$src)>;
def : Pat<(store (X86adc_flag GR16:$src, (loadi16 addr:$dst), EFLAGS),
                 addr:$dst),
          (ADC16mr addr:$dst, GR16:$src)>;
def : Pat<(store (X86adc_flag GR32:$src, (loadi32 addr:$dst), EFLAGS),
                 addr:$dst),
          (ADC32mr addr:$dst, GR32:$src)>;
def : Pat<(store (X86adc_flag GR64:$src, (loadi64 addr:$dst), EFLAGS),
                 addr:$dst),
          (ADC64mr addr:$dst, GR64:$src)>;

// Patterns for basic arithmetic ops with relocImm for the immediate field.
multiclass ArithBinOp_RF_relocImm_Pats<SDNode OpNodeFlag, SDNode OpNode> {
#if INTEL_CUSTOMIZATION
let Predicates = [NoNDD] in {
#endif // INTEL_CUSTOMIZATION
  def : Pat<(OpNodeFlag GR8:$src1, relocImm8_su:$src2),
            (!cast<Instruction>(NAME#"8ri") GR8:$src1, relocImm8_su:$src2)>;
  def : Pat<(OpNodeFlag GR16:$src1, relocImm16_su:$src2),
            (!cast<Instruction>(NAME#"16ri") GR16:$src1, relocImm16_su:$src2)>;
  def : Pat<(OpNodeFlag GR32:$src1, relocImm32_su:$src2),
            (!cast<Instruction>(NAME#"32ri") GR32:$src1, relocImm32_su:$src2)>;
  def : Pat<(OpNodeFlag GR64:$src1, i64relocImmSExt32_su:$src2),
            (!cast<Instruction>(NAME#"64ri32") GR64:$src1, i64relocImmSExt32_su:$src2)>;

  def : Pat<(store (OpNode (load addr:$dst), relocImm8_su:$src), addr:$dst),
            (!cast<Instruction>(NAME#"8mi") addr:$dst, relocImm8_su:$src)>;
  def : Pat<(store (OpNode (load addr:$dst), relocImm16_su:$src), addr:$dst),
            (!cast<Instruction>(NAME#"16mi") addr:$dst, relocImm16_su:$src)>;
  def : Pat<(store (OpNode (load addr:$dst), relocImm32_su:$src), addr:$dst),
            (!cast<Instruction>(NAME#"32mi") addr:$dst, relocImm32_su:$src)>;
  def : Pat<(store (OpNode (load addr:$dst), i64relocImmSExt32_su:$src), addr:$dst),
            (!cast<Instruction>(NAME#"64mi32") addr:$dst, i64relocImmSExt32_su:$src)>;
#if INTEL_CUSTOMIZATION
}
let Predicates = [HasNDD, In64BitMode] in {
  def : Pat<(OpNodeFlag GR8:$src1, relocImm8_su:$src2),
            (!cast<Instruction>(NAME#"8ri_ND") GR8:$src1, relocImm8_su:$src2)>;
  def : Pat<(OpNodeFlag GR16:$src1, i16relocImmSExt8_su:$src2),
            (!cast<Instruction>(NAME#"16ri8_ND") GR16:$src1, i16relocImmSExt8_su:$src2)>;
  def : Pat<(OpNodeFlag GR16:$src1, relocImm16_su:$src2),
            (!cast<Instruction>(NAME#"16ri_ND") GR16:$src1, relocImm16_su:$src2)>;
  def : Pat<(OpNodeFlag GR32:$src1, i32relocImmSExt8_su:$src2),
            (!cast<Instruction>(NAME#"32ri8_ND") GR32:$src1, i32relocImmSExt8_su:$src2)>;
  def : Pat<(OpNodeFlag GR32:$src1, relocImm32_su:$src2),
            (!cast<Instruction>(NAME#"32ri_ND") GR32:$src1, relocImm32_su:$src2)>;
  def : Pat<(OpNodeFlag GR64:$src1, i64relocImmSExt8_su:$src2),
            (!cast<Instruction>(NAME#"64ri8_ND") GR64:$src1, i64relocImmSExt8_su:$src2)>;
  def : Pat<(OpNodeFlag GR64:$src1, i64relocImmSExt32_su:$src2),
            (!cast<Instruction>(NAME#"64ri32_ND") GR64:$src1, i64relocImmSExt32_su:$src2)>;

  def : Pat<(OpNode (load addr:$dst), relocImm8_su:$src),
            (!cast<Instruction>(NAME#"8mi_ND") addr:$dst, relocImm8_su:$src)>;
  def : Pat<(OpNode (load addr:$dst), i16relocImmSExt8_su:$src),
            (!cast<Instruction>(NAME#"16mi8_ND") addr:$dst, i16relocImmSExt8_su:$src)>;
  def : Pat<(OpNode (load addr:$dst), relocImm16_su:$src),
            (!cast<Instruction>(NAME#"16mi_ND") addr:$dst, relocImm16_su:$src)>;
  def : Pat<(OpNode (load addr:$dst), i32relocImmSExt8_su:$src),
            (!cast<Instruction>(NAME#"32mi8_ND") addr:$dst, i32relocImmSExt8_su:$src)>;
  def : Pat<(OpNode (load addr:$dst), relocImm32_su:$src),
            (!cast<Instruction>(NAME#"32mi_ND") addr:$dst, relocImm32_su:$src)>;
  def : Pat<(OpNode (load addr:$dst), i64relocImmSExt8_su:$src),
            (!cast<Instruction>(NAME#"64mi8_ND") addr:$dst, i64relocImmSExt8_su:$src)>;
  def : Pat<(OpNode (load addr:$dst), i64relocImmSExt32_su:$src),
            (!cast<Instruction>(NAME#"64mi32_ND") addr:$dst, i64relocImmSExt32_su:$src)>;
}
#endif // INTEL_CUSTOMIZATION
}

multiclass ArithBinOp_RFF_relocImm_Pats<SDNode OpNodeFlag> {
#if INTEL_CUSTOMIZATION
let Predicates = [NoNDD] in {
#endif // INTEL_CUSTOMIZATION
  def : Pat<(OpNodeFlag GR8:$src1, relocImm8_su:$src2, EFLAGS),
            (!cast<Instruction>(NAME#"8ri") GR8:$src1, relocImm8_su:$src2)>;
  def : Pat<(OpNodeFlag GR16:$src1, relocImm16_su:$src2, EFLAGS),
            (!cast<Instruction>(NAME#"16ri") GR16:$src1, relocImm16_su:$src2)>;
  def : Pat<(OpNodeFlag GR32:$src1, relocImm32_su:$src2, EFLAGS),
            (!cast<Instruction>(NAME#"32ri") GR32:$src1, relocImm32_su:$src2)>;
  def : Pat<(OpNodeFlag GR64:$src1, i64relocImmSExt32_su:$src2, EFLAGS),
            (!cast<Instruction>(NAME#"64ri32") GR64:$src1, i64relocImmSExt32_su:$src2)>;

  def : Pat<(store (OpNodeFlag (load addr:$dst), relocImm8_su:$src, EFLAGS), addr:$dst),
            (!cast<Instruction>(NAME#"8mi") addr:$dst, relocImm8_su:$src)>;
  def : Pat<(store (OpNodeFlag (load addr:$dst), relocImm16_su:$src, EFLAGS), addr:$dst),
            (!cast<Instruction>(NAME#"16mi") addr:$dst, relocImm16_su:$src)>;
  def : Pat<(store (OpNodeFlag (load addr:$dst), relocImm32_su:$src, EFLAGS), addr:$dst),
            (!cast<Instruction>(NAME#"32mi") addr:$dst, relocImm32_su:$src)>;
  def : Pat<(store (OpNodeFlag (load addr:$dst), i64relocImmSExt32_su:$src, EFLAGS), addr:$dst),
            (!cast<Instruction>(NAME#"64mi32") addr:$dst, i64relocImmSExt32_su:$src)>;
#if INTEL_CUSTOMIZATION
}
let Predicates = [HasNDD, In64BitMode] in {
  def : Pat<(OpNodeFlag GR8:$src1, relocImm8_su:$src2, EFLAGS),
            (!cast<Instruction>(NAME#"8ri_ND") GR8:$src1, relocImm8_su:$src2)>;
  def : Pat<(OpNodeFlag GR16:$src1, i16relocImmSExt8_su:$src2, EFLAGS),
            (!cast<Instruction>(NAME#"16ri8_ND") GR16:$src1, i16relocImmSExt8_su:$src2)>;
  def : Pat<(OpNodeFlag GR16:$src1, relocImm16_su:$src2, EFLAGS),
            (!cast<Instruction>(NAME#"16ri_ND") GR16:$src1, relocImm16_su:$src2)>;
  def : Pat<(OpNodeFlag GR32:$src1, i32relocImmSExt8_su:$src2, EFLAGS),
            (!cast<Instruction>(NAME#"32ri8_ND") GR32:$src1, i32relocImmSExt8_su:$src2)>;
  def : Pat<(OpNodeFlag GR32:$src1, relocImm32_su:$src2, EFLAGS),
            (!cast<Instruction>(NAME#"32ri_ND") GR32:$src1, relocImm32_su:$src2)>;
  def : Pat<(OpNodeFlag GR64:$src1, i64relocImmSExt8_su:$src2, EFLAGS),
            (!cast<Instruction>(NAME#"64ri8_ND") GR64:$src1, i64relocImmSExt8_su:$src2)>;
  def : Pat<(OpNodeFlag GR64:$src1, i64relocImmSExt32_su:$src2, EFLAGS),
            (!cast<Instruction>(NAME#"64ri32_ND") GR64:$src1, i64relocImmSExt32_su:$src2)>;

  def : Pat<(OpNodeFlag (load addr:$dst), relocImm8_su:$src, EFLAGS),
            (!cast<Instruction>(NAME#"8mi_ND") addr:$dst, relocImm8_su:$src)>;
  def : Pat<(OpNodeFlag (load addr:$dst), i16relocImmSExt8_su:$src, EFLAGS),
            (!cast<Instruction>(NAME#"16mi8_ND") addr:$dst, i16relocImmSExt8_su:$src)>;
  def : Pat<(OpNodeFlag (load addr:$dst), relocImm16_su:$src, EFLAGS),
            (!cast<Instruction>(NAME#"16mi_ND") addr:$dst, relocImm16_su:$src)>;
  def : Pat<(OpNodeFlag (load addr:$dst), i32relocImmSExt8_su:$src, EFLAGS),
            (!cast<Instruction>(NAME#"32mi8_ND") addr:$dst, i32relocImmSExt8_su:$src)>;
  def : Pat<(OpNodeFlag (load addr:$dst), relocImm32_su:$src, EFLAGS),
            (!cast<Instruction>(NAME#"32mi_ND") addr:$dst, relocImm32_su:$src)>;
  def : Pat<(OpNodeFlag (load addr:$dst), i64relocImmSExt8_su:$src, EFLAGS),
            (!cast<Instruction>(NAME#"64mi8_ND") addr:$dst, i64relocImmSExt8_su:$src)>;
  def : Pat<(OpNodeFlag (load addr:$dst), i64relocImmSExt32_su:$src, EFLAGS),
            (!cast<Instruction>(NAME#"64mi32_ND") addr:$dst, i64relocImmSExt32_su:$src)>;
}
#endif // INTEL_CUSTOMIZATION
}

multiclass ArithBinOp_F_relocImm_Pats<SDNode OpNodeFlag> {
  def : Pat<(OpNodeFlag GR8:$src1, relocImm8_su:$src2),
            (!cast<Instruction>(NAME#"8ri") GR8:$src1, relocImm8_su:$src2)>;
  def : Pat<(OpNodeFlag GR16:$src1, relocImm16_su:$src2),
            (!cast<Instruction>(NAME#"16ri") GR16:$src1, relocImm16_su:$src2)>;
  def : Pat<(OpNodeFlag GR32:$src1, relocImm32_su:$src2),
            (!cast<Instruction>(NAME#"32ri") GR32:$src1, relocImm32_su:$src2)>;
  def : Pat<(OpNodeFlag GR64:$src1, i64relocImmSExt32_su:$src2),
            (!cast<Instruction>(NAME#"64ri32") GR64:$src1, i64relocImmSExt32_su:$src2)>;

  def : Pat<(OpNodeFlag (loadi8 addr:$src1), relocImm8_su:$src2),
            (!cast<Instruction>(NAME#"8mi") addr:$src1, relocImm8_su:$src2)>;
  def : Pat<(OpNodeFlag (loadi16 addr:$src1), relocImm16_su:$src2),
            (!cast<Instruction>(NAME#"16mi") addr:$src1, relocImm16_su:$src2)>;
  def : Pat<(OpNodeFlag (loadi32 addr:$src1), relocImm32_su:$src2),
            (!cast<Instruction>(NAME#"32mi") addr:$src1, relocImm32_su:$src2)>;
  def : Pat<(OpNodeFlag (loadi64 addr:$src1), i64relocImmSExt32_su:$src2),
            (!cast<Instruction>(NAME#"64mi32") addr:$src1, i64relocImmSExt32_su:$src2)>;
}

defm AND : ArithBinOp_RF_relocImm_Pats<X86and_flag, and>;
defm OR  : ArithBinOp_RF_relocImm_Pats<X86or_flag, or>;
defm XOR : ArithBinOp_RF_relocImm_Pats<X86xor_flag, xor>;
defm ADD : ArithBinOp_RF_relocImm_Pats<X86add_flag, add>;
defm SUB : ArithBinOp_RF_relocImm_Pats<X86sub_flag, sub>;

defm ADC : ArithBinOp_RFF_relocImm_Pats<X86adc_flag>;
defm SBB : ArithBinOp_RFF_relocImm_Pats<X86sbb_flag>;

defm CMP : ArithBinOp_F_relocImm_Pats<X86cmp>;

// ADC is commutable, but we can't indicate that to tablegen. So manually
// reverse the operands.
def : Pat<(X86adc_flag GR8:$src1, relocImm8_su:$src2, EFLAGS),
          (ADC8ri relocImm8_su:$src2, GR8:$src1)>;
def : Pat<(X86adc_flag i16relocImmSExt8_su:$src2, GR16:$src1, EFLAGS),
          (ADC16ri8 GR16:$src1, i16relocImmSExt8_su:$src2)>;
def : Pat<(X86adc_flag relocImm16_su:$src2, GR16:$src1, EFLAGS),
          (ADC16ri GR16:$src1, relocImm16_su:$src2)>;
def : Pat<(X86adc_flag i32relocImmSExt8_su:$src2, GR32:$src1, EFLAGS),
          (ADC32ri8 GR32:$src1, i32relocImmSExt8_su:$src2)>;
def : Pat<(X86adc_flag relocImm32_su:$src2, GR32:$src1, EFLAGS),
          (ADC32ri GR32:$src1, relocImm32_su:$src2)>;
def : Pat<(X86adc_flag i64relocImmSExt8_su:$src2, GR64:$src1, EFLAGS),
          (ADC64ri8 GR64:$src1, i64relocImmSExt8_su:$src2)>;
def : Pat<(X86adc_flag i64relocImmSExt32_su:$src2, GR64:$src1, EFLAGS),
          (ADC64ri32 GR64:$src1, i64relocImmSExt32_su:$src2)>;

def : Pat<(store (X86adc_flag relocImm8_su:$src, (load addr:$dst), EFLAGS), addr:$dst),
          (ADC8mi addr:$dst, relocImm8_su:$src)>;
def : Pat<(store (X86adc_flag i16relocImmSExt8_su:$src, (load addr:$dst), EFLAGS), addr:$dst),
          (ADC16mi8 addr:$dst, i16relocImmSExt8_su:$src)>;
def : Pat<(store (X86adc_flag relocImm16_su:$src, (load addr:$dst), EFLAGS), addr:$dst),
          (ADC16mi addr:$dst, relocImm16_su:$src)>;
def : Pat<(store (X86adc_flag i32relocImmSExt8_su:$src, (load addr:$dst), EFLAGS), addr:$dst),
          (ADC32mi8 addr:$dst, i32relocImmSExt8_su:$src)>;
def : Pat<(store (X86adc_flag relocImm32_su:$src, (load addr:$dst), EFLAGS), addr:$dst),
          (ADC32mi addr:$dst, relocImm32_su:$src)>;
def : Pat<(store (X86adc_flag i64relocImmSExt8_su:$src, (load addr:$dst), EFLAGS), addr:$dst),
          (ADC64mi8 addr:$dst, i64relocImmSExt8_su:$src)>;
def : Pat<(store (X86adc_flag i64relocImmSExt32_su:$src, (load addr:$dst), EFLAGS), addr:$dst),
          (ADC64mi32 addr:$dst, i64relocImmSExt32_su:$src)>;

//===----------------------------------------------------------------------===//
// Semantically, test instructions are similar like AND, except they don't
// generate a result.  From an encoding perspective, they are very different:
// they don't have all the usual imm8 and REV forms, and are encoded into a
// different space.
def X86testpat : PatFrag<(ops node:$lhs, node:$rhs),
                         (X86cmp (and_su node:$lhs, node:$rhs), 0)>;

let isCompare = 1 in {
  let Defs = [EFLAGS] in {
    let isCommutable = 1 in {
      // Avoid selecting these and instead use a test+and. Post processing will
      // combine them. This gives bunch of other patterns that start with
      // and a chance to match.
      def TEST8rr  : BinOpRR_F<0x84, "test", Xi8 , null_frag>;
      def TEST16rr : BinOpRR_F<0x84, "test", Xi16, null_frag>;
      def TEST32rr : BinOpRR_F<0x84, "test", Xi32, null_frag>;
      def TEST64rr : BinOpRR_F<0x84, "test", Xi64, null_frag>;
    } // isCommutable

    let hasSideEffects = 0, mayLoad = 1 in {
    def TEST8mr    : BinOpMR_F<0x84, "test", Xi8 , null_frag>;
    def TEST16mr   : BinOpMR_F<0x84, "test", Xi16, null_frag>;
    def TEST32mr   : BinOpMR_F<0x84, "test", Xi32, null_frag>;
    def TEST64mr   : BinOpMR_F<0x84, "test", Xi64, null_frag>;
    }

    def TEST8ri    : BinOpRI_F<0xF6, "test", Xi8 , X86testpat, MRM0r>;
    def TEST16ri   : BinOpRI_F<0xF6, "test", Xi16, X86testpat, MRM0r>;
    def TEST32ri   : BinOpRI_F<0xF6, "test", Xi32, X86testpat, MRM0r>;
    def TEST64ri32 : BinOpRI_F<0xF6, "test", Xi64, X86testpat, MRM0r>;

    def TEST8mi    : BinOpMI_F<0xF6, "test", Xi8 , X86testpat, MRM0m>;
    def TEST16mi   : BinOpMI_F<0xF6, "test", Xi16, X86testpat, MRM0m>;
    def TEST32mi   : BinOpMI_F<0xF6, "test", Xi32, X86testpat, MRM0m>;
    let Predicates = [In64BitMode] in
    def TEST64mi32 : BinOpMI_F<0xF6, "test", Xi64, X86testpat, MRM0m>;
  } // Defs = [EFLAGS]

  def TEST8i8    : BinOpAI_F<0xA8, "test", Xi8 , AL,
                             "{$src, %al|al, $src}">;
  def TEST16i16  : BinOpAI_F<0xA8, "test", Xi16, AX,
                             "{$src, %ax|ax, $src}">;
  def TEST32i32  : BinOpAI_F<0xA8, "test", Xi32, EAX,
                             "{$src, %eax|eax, $src}">;
  def TEST64i32  : BinOpAI_F<0xA8, "test", Xi64, RAX,
                             "{$src, %rax|rax, $src}">;
} // isCompare

// Patterns to match a relocImm into the immediate field.
def : Pat<(X86testpat GR8:$src1, relocImm8_su:$src2),
          (TEST8ri GR8:$src1, relocImm8_su:$src2)>;
def : Pat<(X86testpat GR16:$src1, relocImm16_su:$src2),
          (TEST16ri GR16:$src1, relocImm16_su:$src2)>;
def : Pat<(X86testpat GR32:$src1, relocImm32_su:$src2),
          (TEST32ri GR32:$src1, relocImm32_su:$src2)>;
def : Pat<(X86testpat GR64:$src1, i64relocImmSExt32_su:$src2),
          (TEST64ri32 GR64:$src1, i64relocImmSExt32_su:$src2)>;

def : Pat<(X86testpat (loadi8 addr:$src1), relocImm8_su:$src2),
          (TEST8mi addr:$src1, relocImm8_su:$src2)>;
def : Pat<(X86testpat (loadi16 addr:$src1), relocImm16_su:$src2),
          (TEST16mi addr:$src1, relocImm16_su:$src2)>;
def : Pat<(X86testpat (loadi32 addr:$src1), relocImm32_su:$src2),
          (TEST32mi addr:$src1, relocImm32_su:$src2)>;
def : Pat<(X86testpat (loadi64 addr:$src1), i64relocImmSExt32_su:$src2),
          (TEST64mi32 addr:$src1, i64relocImmSExt32_su:$src2)>;

//===----------------------------------------------------------------------===//
// ANDN Instruction
//
multiclass bmi_andn<string mnemonic, RegisterClass RC, X86MemOperand x86memop,
                    PatFrag ld_frag, X86FoldableSchedWrite sched> {
  def rr : I<0xF2, MRMSrcReg, (outs RC:$dst), (ins RC:$src1, RC:$src2),
            !strconcat(mnemonic, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"),
            [(set RC:$dst, EFLAGS, (X86and_flag (not RC:$src1), RC:$src2))]>,
            Sched<[sched]>;
  def rm : I<0xF2, MRMSrcMem, (outs RC:$dst), (ins RC:$src1, x86memop:$src2),
            !strconcat(mnemonic, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"),
            [(set RC:$dst, EFLAGS,
             (X86and_flag (not RC:$src1), (ld_frag addr:$src2)))]>,
           Sched<[sched.Folded, sched.ReadAfterFold]>;
}

// Complexity is reduced to give and with immediate a chance to match first.
let Predicates = [HasBMI], Defs = [EFLAGS], AddedComplexity = -6 in {
  defm ANDN32 : bmi_andn<"andn{l}", GR32, i32mem, loadi32, WriteALU>, T8PS, VEX_4V;
  defm ANDN64 : bmi_andn<"andn{q}", GR64, i64mem, loadi64, WriteALU>, T8PS, VEX_4V, REX_W;
}

let Predicates = [HasBMI], AddedComplexity = -6 in {
  def : Pat<(and (not GR32:$src1), GR32:$src2),
            (ANDN32rr GR32:$src1, GR32:$src2)>;
  def : Pat<(and (not GR64:$src1), GR64:$src2),
            (ANDN64rr GR64:$src1, GR64:$src2)>;
  def : Pat<(and (not GR32:$src1), (loadi32 addr:$src2)),
            (ANDN32rm GR32:$src1, addr:$src2)>;
  def : Pat<(and (not GR64:$src1), (loadi64 addr:$src2)),
            (ANDN64rm GR64:$src1, addr:$src2)>;
}

//===----------------------------------------------------------------------===//
// MULX Instruction
//
#if INTEL_CUSTOMIZATION
multiclass bmi_mulx<string mnemonic, RegisterClass RC, X86MemOperand x86memop,
                    X86FoldableSchedWrite sched> {
let hasSideEffects = 0 in {
let Predicates = [HasBMI2, NoEGPR] in {
  def rr : I<0xF6, MRMSrcReg, (outs RC:$dst1, RC:$dst2), (ins RC:$src),
             !strconcat(mnemonic, "\t{$src, $dst2, $dst1|$dst1, $dst2, $src}"),
             []>, T8XD, VEX_4V, Sched<[WriteIMulH, sched]>;

  let mayLoad = 1 in
  def rm : I<0xF6, MRMSrcMem, (outs RC:$dst1, RC:$dst2), (ins x86memop:$src),
             !strconcat(mnemonic, "\t{$src, $dst2, $dst1|$dst1, $dst2, $src}"),
             []>, T8XD, VEX_4V,
             Sched<[WriteIMulHLd, sched.Folded,
                    // Memory operand.
                    ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault,
                    // Implicit read of EDX/RDX
                    sched.ReadAfterFold]>;

  // Pseudo instructions to be used when the low result isn't used. The
  // instruction is defined to keep the high if both destinations are the same.
  def Hrr : PseudoI<(outs RC:$dst), (ins RC:$src),
                    []>, Sched<[sched]>;

  let mayLoad = 1 in
  def Hrm : PseudoI<(outs RC:$dst), (ins x86memop:$src),
                    []>, Sched<[sched.Folded]>;
}
let Predicates = [HasBMI2, HasEGPR, In64BitMode] in
  def rr#_EVEX : I<0xF6, MRMSrcReg, (outs RC:$dst1, RC:$dst2), (ins RC:$src),
                   !strconcat(mnemonic, "\t{$src, $dst2, $dst1|$dst1, $dst2, $src}"),
                   []>, T8XD, EVEX_4V, EmitVEXOrEVEXPrefix, Sched<[WriteIMulH, sched]>;
let Predicates = [HasBMI2, HasEGPR, In64BitMode], mayLoad = 1 in
  def rm#_EVEX : I<0xF6, MRMSrcMem, (outs RC:$dst1, RC:$dst2), (ins x86memop:$src),
                   !strconcat(mnemonic, "\t{$src, $dst2, $dst1|$dst1, $dst2, $src}"),
                   []>, T8XD, EVEX_4V, EmitVEXOrEVEXPrefix,
                 Sched<[WriteIMulHLd, sched.Folded,
                        // Memory operand.
                        ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault,
                        // Implicit read of EDX/RDX
                        sched.ReadAfterFold]>;
}
}

let Uses = [EDX] in
  defm MULX32 : bmi_mulx<"mulx{l}", GR32, i32mem, WriteMULX32>;
let Uses = [RDX] in
  defm MULX64 : bmi_mulx<"mulx{q}", GR64, i64mem, WriteMULX64>, REX_W;
#endif // INTEL_CUSTOMIZATION

//===----------------------------------------------------------------------===//
// ADCX and ADOX Instructions
//
// We don't have patterns for these as there is no advantage over ADC for
// most code.
class ADCOXOpRR <bits<8> opcode, string mnemonic, X86TypeInfo info>
  : BinOpRR_C<opcode, MRMSrcReg, mnemonic, info, []>{
  let Opcode = opcode;
  let OpSize = OpSizeFixed;
}

class ADCOXOpRM <bits<8> opcode, string mnemonic, X86TypeInfo info>
  : BinOpRM_C<opcode, MRMSrcMem, mnemonic, info, []>{
  let Opcode = opcode;
  let OpSize = OpSizeFixed;
}

#if INTEL_CUSTOMIZATION
multiclass ADCOXOpRR_EVEX<bits<8> opcode, string mnemonic, X86TypeInfo info> {
let Opcode = opcode, OpSize = OpSizeFixed in {
let Constraints = "$src1 = $dst", Predicates = [HasEGPR, In64BitMode] in
  def _EVEX : BinOpRR_EVEX<0x66, MRMSrcReg, mnemonic, info, []>;
let Predicates=[HasADX, HasNDD, In64BitMode] in
  def _ND : BinOpRR_NDD<0x66, MRMSrcReg, mnemonic, info, []>, EVEX_B4V;
}
}

multiclass ADCOXOpRM_EVEX<bits<8> opcode, string mnemonic, X86TypeInfo info> {
let Opcode = opcode, OpSize = OpSizeFixed in {
let Constraints = "$src1 = $dst", Predicates = [HasEGPR, In64BitMode] in
  def _EVEX : BinOpRM_EVEX<0x66, MRMSrcMem, mnemonic, info, []>;
let Predicates = [HasADX, HasNDD, In64BitMode] in
  def _ND : BinOpRM_NDD<0x66, MRMSrcMem, mnemonic, info, []>, EVEX_B4V;
}
}
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
let Defs = [EFLAGS], Uses = [EFLAGS], hasSideEffects = 0 in {
  let SchedRW = [WriteADC], isCommutable = 1 in {
  let Predicates = [HasADX], Constraints = "$src1 = $dst" in {
#endif // INTEL_CUSTOMIZATION
  def ADCX32rr : ADCOXOpRR<0xF6, "adcx", Xi32>, T8PD;
  def ADCX64rr : ADCOXOpRR<0xF6, "adcx", Xi64>, T8PD;

  def ADOX32rr : ADCOXOpRR<0xF6, "adox", Xi32>, T8XS;
  def ADOX64rr : ADCOXOpRR<0xF6, "adox", Xi64>, T8XS;
#if INTEL_CUSTOMIZATION
  }
#endif // INTEL_CUSTOMIZATION
#if INTEL_CUSTOMIZATION
  let OpPrefix = PD in {
  defm ADCX32rr : ADCOXOpRR_EVEX<0x66, "adcx", Xi32>;
  defm ADCX64rr : ADCOXOpRR_EVEX<0x66, "adcx", Xi64>;
  }
  let OpPrefix = XS in {
  defm ADOX32rr : ADCOXOpRR_EVEX<0x66, "adox", Xi32>;
  defm ADOX64rr : ADCOXOpRR_EVEX<0x66, "adox", Xi64>;
  }
#endif // INTEL_CUSTOMIZATION
  } // SchedRW

  let mayLoad = 1,
      SchedRW = [WriteADC.Folded, WriteADC.ReadAfterFold,
                 // Memory operand.
                 ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault,
                 // Implicit read of EFLAGS
                 WriteADC.ReadAfterFold] in {
#if INTEL_CUSTOMIZATION
  let Predicates = [HasADX], Constraints = "$src1 = $dst" in {
#endif // INTEL_CUSTOMIZATION
  def ADCX32rm : ADCOXOpRM<0xF6, "adcx", Xi32>, T8PD;
  def ADCX64rm : ADCOXOpRM<0xF6, "adcx", Xi64>, T8PD;

  def ADOX32rm : ADCOXOpRM<0xF6, "adox", Xi32>, T8XS;
  def ADOX64rm : ADCOXOpRM<0xF6, "adox", Xi64>, T8XS;
#if INTEL_CUSTOMIZATION
  }
#endif // INTEL_CUSTOMIZATION
#if INTEL_CUSTOMIZATION
  let OpPrefix = PD in {
  defm ADCX32rm : ADCOXOpRM_EVEX<0x66, "adcx", Xi32>;
  defm ADCX64rm : ADCOXOpRM_EVEX<0x66, "adcx", Xi64>;
  }

  let OpPrefix = XS in {
  defm ADOX32rm : ADCOXOpRM_EVEX<0x66, "adox", Xi32>;
  defm ADOX64rm : ADCOXOpRM_EVEX<0x66, "adox", Xi64>;
  }
#endif // INTEL_CUSTOMIZATION
  } // mayLoad, SchedRW
}
