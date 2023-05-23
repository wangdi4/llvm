#if INTEL_FEATURE_ISA_APX_F
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
             mnemonic#"${cond}", "$dcf\t{$src2, $src1|$src1, $src2}" , []>, EVEX_B {
  // Infer instruction prefixes from type info.
  let isCodeGenOnly = 1;
  let ForceDisassemble = 1;
  let Uses = [EFLAGS];
  let Defs = [EFLAGS];
  let hasTwoConditionalOps = 1;
}

let Predicates = [In64BitMode] in {
def CCMP32rr: ITy_EVEX_C<0x38, MRMDestReg, Xi32, GR32, GR32, "ccmp">;
def CCMP64rr: ITy_EVEX_C<0x38, MRMDestReg, Xi64, GR64, GR64, "ccmp">;
}
#endif // INTEL_FEATURE_ISA_APX_F
