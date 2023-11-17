//==-- Intel_X86InstrAPXCondition.td - APX conditional Instrs --*- tablegen -*-==//
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
//===----------------------------------------------------------------------===//
//
// This file describes the APX conditional instructions in the X86
// architecture.
//
//===----------------------------------------------------------------------===//

/// Special form for ITy_EVEX with two conditional operands
//  NOTE: The assembly string is not important here b/c it won't be in asm match tables.
class ITy_EVEX_C<bits<8> opcode, Format f, X86TypeInfo typeinfo, DAGOperand op1, DAGOperand op2,
          string mnemonic>
  : ITy_EVEX<opcode, f, typeinfo, (outs), (ins op1:$src1, op2:$src2, cflags:$dcf, ccode:$cond),
             mnemonic#"${cond}", "$dcf\t{$src2, $src1|$src1, $src2}" , []> {
  // Infer instruction prefixes from type info.
  let isCodeGenOnly = 1;
  let ForceDisassemble = 1;
  let Uses = [EFLAGS];
  let Defs = [EFLAGS];
  let hasTwoConditionalOps = 1;
}

class ITy_EVEX_C_Imm<bits<8> opcode, Format f, X86TypeInfo typeinfo,
                     DAGOperand op1, DAGOperand op2, string mnemonic>
  : ITy_EVEX_C<opcode, f, typeinfo, op1, op2, mnemonic> {
  let ImmT = typeinfo.ImmEncoding;
}

let Predicates = [In64BitMode] in {
let SchedRW = [WriteALU] in {
def CCMP8rr : ITy_EVEX_C<0x38, MRMDestReg, Xi8,  GR8,  GR8,  "ccmp">;
def CCMP16rr: ITy_EVEX_C<0x38, MRMDestReg, Xi16, GR16, GR16, "ccmp">;
def CCMP32rr: ITy_EVEX_C<0x38, MRMDestReg, Xi32, GR32, GR32, "ccmp">;
def CCMP64rr: ITy_EVEX_C<0x38, MRMDestReg, Xi64, GR64, GR64, "ccmp">;
def CCMP8rr_REV : ITy_EVEX_C<0x3a, MRMSrcReg, Xi8,  GR8,  GR8,  "ccmp">;
def CCMP16rr_REV: ITy_EVEX_C<0x3a, MRMSrcReg, Xi16, GR16, GR16, "ccmp">;
def CCMP32rr_REV: ITy_EVEX_C<0x3a, MRMSrcReg, Xi32, GR32, GR32, "ccmp">;
def CCMP64rr_REV: ITy_EVEX_C<0x3a, MRMSrcReg, Xi64, GR64, GR64, "ccmp">;
let ImmT = Imm8 in {
def CCMP16ri8: ITy_EVEX_C<0x83, MRM7r, Xi16, GR16, i16i8imm, "ccmp">;
def CCMP32ri8: ITy_EVEX_C<0x83, MRM7r, Xi32, GR32, i32i8imm, "ccmp">;
def CCMP64ri8: ITy_EVEX_C<0x83, MRM7r, Xi64, GR64, i64i8imm, "ccmp">;
let mayLoad = 1 in {
def CCMP16mi8: ITy_EVEX_C<0x83, MRM7m, Xi16, i16mem, i16i8imm, "ccmp">;
def CCMP32mi8: ITy_EVEX_C<0x83, MRM7m, Xi32, i32mem, i32i8imm, "ccmp">;
def CCMP64mi8: ITy_EVEX_C<0x83, MRM7m, Xi64, i64mem, i64i8imm, "ccmp">;
} // mayLoad = 1
}
def CCMP8ri : ITy_EVEX_C_Imm<0x80, MRM7r, Xi8,   GR8, i8imm,  "ccmp">;
def CCMP16ri: ITy_EVEX_C_Imm<0x80, MRM7r, Xi16, GR16, i16imm, "ccmp">;
def CCMP32ri: ITy_EVEX_C_Imm<0x80, MRM7r, Xi32, GR32, i32imm, "ccmp">;
def CCMP64ri32: ITy_EVEX_C_Imm<0x80, MRM7r, Xi64, GR64, i64i32imm, "ccmp">;
let mayLoad = 1 in {
def CCMP8mi : ITy_EVEX_C_Imm<0x80, MRM7m, Xi8,   i8mem, i8imm,  "ccmp">;
def CCMP16mi: ITy_EVEX_C_Imm<0x80, MRM7m, Xi16, i16mem, i16imm, "ccmp">;
def CCMP32mi: ITy_EVEX_C_Imm<0x80, MRM7m, Xi32, i32mem, i32imm, "ccmp">;
def CCMP64mi32: ITy_EVEX_C_Imm<0x80, MRM7m, Xi64, i64mem, i64i32imm, "ccmp">;
} // mayLoad = 1

let isCommutable = 1 in {
def CTEST8rr : ITy_EVEX_C<0x84, MRMDestReg, Xi8,  GR8,  GR8,  "ctest">;
def CTEST16rr: ITy_EVEX_C<0x84, MRMDestReg, Xi16, GR16, GR16, "ctest">;
def CTEST32rr: ITy_EVEX_C<0x84, MRMDestReg, Xi32, GR32, GR32, "ctest">;
def CTEST64rr: ITy_EVEX_C<0x84, MRMDestReg, Xi64, GR64, GR64, "ctest">;
}
def CTEST8ri : ITy_EVEX_C_Imm<0xF6, MRM0r, Xi8,   GR8, i8imm,  "ctest">;
def CTEST16ri: ITy_EVEX_C_Imm<0xF6, MRM0r, Xi16, GR16, i16imm, "ctest">;
def CTEST32ri: ITy_EVEX_C_Imm<0xF6, MRM0r, Xi32, GR32, i32imm, "ctest">;
def CTEST64ri32: ITy_EVEX_C_Imm<0xF6, MRM0r, Xi64, GR64, i64i32imm, "ctest">;
let mayLoad = 1 in {
def CTEST8mi : ITy_EVEX_C_Imm<0xF6, MRM0m, Xi8,   i8mem, i8imm,  "ctest">;
def CTEST16mi: ITy_EVEX_C_Imm<0xF6, MRM0m, Xi16, i16mem, i16imm, "ctest">;
def CTEST32mi: ITy_EVEX_C_Imm<0xF6, MRM0m, Xi32, i32mem, i32imm, "ctest">;
def CTEST64mi32: ITy_EVEX_C_Imm<0xF6, MRM0m, Xi64, i64mem, i64i32imm, "ctest">;
} // mayLoad = 1
} // SchedRW
let SchedRW = [WriteALU.Folded, WriteALU.ReadAfterFold], mayLoad = 1 in {
def CCMP8rm : ITy_EVEX_C<0x3a, MRMSrcMem, Xi8,  GR8,  i8mem,  "ccmp">;
def CCMP16rm: ITy_EVEX_C<0x3a, MRMSrcMem, Xi16, GR16, i16mem, "ccmp">;
def CCMP32rm: ITy_EVEX_C<0x3a, MRMSrcMem, Xi32, GR32, i32mem, "ccmp">;
def CCMP64rm: ITy_EVEX_C<0x3a, MRMSrcMem, Xi64, GR64, i64mem, "ccmp">;
} // SchedRW
let SchedRW = [WriteALU.Folded, ReadDefault, ReadDefault, ReadDefault,
               ReadDefault, ReadDefault, WriteALU.ReadAfterFold],
    mayLoad = 1 in {
def CCMP8mr : ITy_EVEX_C<0x38, MRMDestMem, Xi8,  i8mem,  GR8,  "ccmp">;
def CCMP16mr: ITy_EVEX_C<0x38, MRMDestMem, Xi16, i16mem, GR16, "ccmp">;
def CCMP32mr: ITy_EVEX_C<0x38, MRMDestMem, Xi32, i32mem, GR32, "ccmp">;
def CCMP64mr: ITy_EVEX_C<0x38, MRMDestMem, Xi64, i64mem, GR64, "ccmp">;
def CTEST8mr : ITy_EVEX_C<0x84, MRMDestMem, Xi8,  i8mem,  GR8,  "ctest">;
def CTEST16mr: ITy_EVEX_C<0x84, MRMDestMem, Xi16, i16mem, GR16, "ctest">;
def CTEST32mr: ITy_EVEX_C<0x84, MRMDestMem, Xi32, i32mem, GR32, "ctest">;
def CTEST64mr: ITy_EVEX_C<0x84, MRMDestMem, Xi64, i64mem, GR64, "ctest">;
} // SchedRW, mayLoad
} // Predicates
