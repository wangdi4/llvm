//===- CSAInstrInfo.td - CSA Instruction defs -----------------*- tblgen-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the CSA instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

include "CSAInstrFormats.td"

// Possible TODO:
// - Get rid of "f" variants for stores and other operations?  (e.g. ideally
//   there should be a single LD32, ST32, MERGE32, etc.)
// - Get rid of separate ordered loads/stores by having default operands?
//   (Also, should %ign be zero_reg defined in Target.td, and use that
//   as default for order operands for stores/loads?)
// - Possible to merge BinOp and CmpOp (something that can take both a PatFrag
//   and an SDNode?)
// - Use "foreacj" or some other mechanism to deal with 8/16/32/64 redundancy
//   in patterns? (but will likely eventually need opcode bitpatterns, so
//   that may not be feasible/desirable long term)
// - Merge ADDR_I/R and ADDR_RI/RR similar to RegImm?
// - Why are so many operands in CSAGenInstrInfo.inc unknown?

//===----------------------------------------------------------------------===//
// CSA Instruction Predicate Definitions
//===----------------------------------------------------------------------===//

def IsOrdered    : Predicate<"Subtarget.isOrdered()">;
def HasI0        : Predicate<"Subtarget.hasI0()">;
def HasI1        : Predicate<"Subtarget.hasI1()">;
def HasI8        : Predicate<"Subtarget.hasI8()">;
def HasI16       : Predicate<"Subtarget.hasI16()">;
def HasI32       : Predicate<"Subtarget.hasI32()">;
def HasI64       : Predicate<"Subtarget.hasI64()">;
def HasF16       : Predicate<"Subtarget.hasF16()">;
def HasF32       : Predicate<"Subtarget.hasF32()">;
def HasF64       : Predicate<"Subtarget.hasF64()">;
def HasSextL     : Predicate<"Subtarget.hasSextL()">;
def HasDispl     : Predicate<"Subtarget.hasDispl()">;
def HasIndex     : Predicate<"Subtarget.hasIndex()">;
def HasShAdd     : Predicate<"Subtarget.hasShAdd()">;
def HasBitOp     : Predicate<"Subtarget.hasBitOp()">;
def HasIDiv      : Predicate<"Subtarget.hasIDiv()">;
def HasFDiv      : Predicate<"Subtarget.hasFDiv()">;
def HasFMA       : Predicate<"Subtarget.hasFMA()">;
def HasRcpA      : Predicate<"Subtarget.hasRcpA()">;
def HasRSqrtA    : Predicate<"Subtarget.hasRSqrtA()">;
def HasSqrt      : Predicate<"Subtarget.hasSqrt()">;
def HasMath0     : Predicate<"Subtarget.hasMath0()">;

//===----------------------------------------------------------------------===//
// CSA Operand Definitions.
//===----------------------------------------------------------------------===//

def brtarget     : Operand<OtherVT>;
def calltarget   : Operand<i64>;

def UnitOpnd : Operand<i64> {
  let PrintMethod = "printUnitOperand";
}

// Not in Target.td with the other *imms
let OperandType = "OPERAND_IMMEDIATE" in {
def f16imm : Operand<f16>;
}

// TableGen expects special *RegImm* functions in CSAAsmParser due to this.
def RegImmAsmOperand : AsmOperandClass {
  let Name = "RegImm";
}

// Register or immedate
class RegImmOperand<ValueType vt> : Operand<vt>, ComplexPattern<vt,1,"SelectRegImm", [imm, fpimm]> {
    let OperandType = "OPERAND_REG_IMM";
    // We need to give TableGen some clues (along with some custom function in
    // CSAAsmParser) to deal with these particularly flexible operands.
    let ParserMatchClass = RegImmAsmOperand;
  }

// Rounding mode operands. These are literal integers underneath, but are
// printed and parsed as their symbolic names. The default numerically matches
// the simulator's default value in csa.h. (This is CSA::ROUND_NEAREST.)
// Note that RMODE operands are optional in the sense that the compiler doesn't
// need to add them to MachineInstrs. However, the parser needs them to be
// explicitly provided.
def RMODE : OperandWithDefaultOps <i64, (ops (i64 0))>
{
  let PrintMethod = "printRModeOperand";
}

// RCL (Reg/Chan/Literal) is the same as used in the simulator
def RCLi0  : RegImmOperand<i1>;
def RCLi1  : RegImmOperand<i1>;
def RCLi8  : RegImmOperand<i8>;
def RCLi16 : RegImmOperand<i16>;
def RCLf16 : RegImmOperand<f16>;
def RCLi32 : RegImmOperand<i32>;
def RCLf32 : RegImmOperand<f32>;
def RCLi64 : RegImmOperand<i64>;
def RCLf64 : RegImmOperand<f64>;

// Memory operands
class Addr<int numArgs, string funcName, dag opInfo> :
  Operand<i64>, ComplexPattern<i64, numArgs,
    funcName, [],
    [SDNPWantParent]> { let MIOperandInfo = opInfo; }

let PrintMethod = "printMemOperand" in {
def ADDR_RX : Addr<2, "SelectAddrRegIdx", (ops I64:$base, I64:$offset)>;
def ADDR_RI : Addr<2, "SelectAddrRegImm", (ops I64:$base, i64imm:$offset)>;
def ADDR_RR : Addr<2, "SelectAddrRegReg", (ops I64:$base, I64:$offset)>;
}
def ADDR_I : Addr<1, "SelectAddrImm", (ops i64imm:$imm)>;
def ADDR_R : Addr<1, "SelectAddrReg", (ops I64:$addr)>{
  // For some reason, the asm matcher is deducing that we must match the I64
  // with an Imm. Override, allowing either, at least for asm parsing.
  let ParserMatchClass = RegImmAsmOperand;
}

//===----------------------------------------------------------------------===//
// CSA Type associations
//===----------------------------------------------------------------------===//
// CSAOpInfo - information that describes CSA information about types used
// in operations.  For example, what register class and immediate to use.
//
class CSAOpInfo<ValueType vt, int opBitSize, string instrSuffix,
    RegisterClass rc, RegisterClass regRC, RegisterClass licRC,
    DAGOperand rcl, DAGOperand immOperand, list<Predicate> preds> {
  // VT - the value type itself
  ValueType VT = vt;

  // OpBitSize - the size of the operation.  (e.g.
  int OpBitSize = opBitSize;

  // InstrSuffix - used on instructions with this type.
  // e.g. i8->8, i64->64, f32->f32
  string InstrSuffix = instrSuffix;

  // RC - the generic (either actual register or lic) register class
  // associated with this type
  RegisterClass RC = rc;

  // RegRC - the actual "register" register class associated with this type
  RegisterClass RegRC = regRC;

  // LICRC - the LIC register class associated with this type
  RegisterClass LICRC = licRC;

  // RCL - Register (Reg or LIC) or Literal operand
  DAGOperand RCL = rcl;

  // L (ImmOperand) - the operand kind of an immediate of this type
  DAGOperand L = immOperand;

  // Preds - Predicate list associated with type (not yet used by patterns)
  list<Predicate> Preds = preds;
}

// CSA type associations
def Ti0  : CSAOpInfo< i1,  0,   "0",  I0,  RI0,  CI0,  RCLi0,  i1imm,  [HasI0]>;
def Ti1  : CSAOpInfo< i1,  1,   "1",  I1,  RI1,  CI1,  RCLi1,  i1imm,  [HasI1]>;
def Ti8  : CSAOpInfo< i8,  8,   "8",  I8,  RI8,  CI8,  RCLi8,  i8imm,  [HasI8]>;
def Ts8  : CSAOpInfo< i8,  8,  "s8",  I8,  RI8,  CI8,  RCLi8,  i8imm,  [HasI8]>;
def Tu8  : CSAOpInfo< i8,  8,  "u8",  I8,  RI8,  CI8,  RCLi8,  i8imm,  [HasI8]>;
def Ti16 : CSAOpInfo<i16, 16,  "16", I16, RI16, CI16, RCLi16, i16imm, [HasI16]>;
def Ts16 : CSAOpInfo<i16, 16, "s16", I16, RI16, CI16, RCLi16, i16imm, [HasI16]>;
def Tu16 : CSAOpInfo<i16, 16, "u16", I16, RI16, CI16, RCLi16, i16imm, [HasI16]>;
def Tf16 : CSAOpInfo<f16, 16, "f16", I16, RI16, CI16, RCLf16, f16imm, [HasF16]>;
def Ti32 : CSAOpInfo<i32, 32,  "32", I32, RI32, CI32, RCLi32, i32imm, [HasI32]>;
def Ts32 : CSAOpInfo<i32, 32, "s32", I32, RI32, CI32, RCLi32, i32imm, [HasI32]>;
def Tu32 : CSAOpInfo<i32, 32, "u32", I32, RI32, CI32, RCLi32, i32imm, [HasI32]>;
def Tf32 : CSAOpInfo<f32, 32, "f32", I32, RI32, CI32, RCLf32, f32imm, [HasF32]>;
def Ti64 : CSAOpInfo<i64, 64,  "64", I64, RI64, CI64, RCLi64, i64imm, [HasI64]>;
def Ts64 : CSAOpInfo<i64, 64, "s64", I64, RI64, CI64, RCLi64, i64imm, [HasI64]>;
def Tu64 : CSAOpInfo<i64, 64, "u64", I64, RI64, CI64, RCLi64, i64imm, [HasI64]>;
def Tf64 : CSAOpInfo<f64, 64, "f64", I64, RI64, CI64, RCLf64, f64imm, [HasF64]>;


def flog : SDNode<"ISD::FLOG", SDTFPUnaryOp>;
def fexp : SDNode<"ISD::FEXP", SDTFPUnaryOp>;

//===----------------------------------------------------------------------===//
// CSA profiles and nodes
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// CSA Instructions.
//===----------------------------------------------------------------------===//

def XPHI : Instruction {
  let Namespace = "CSA";
  let OutOperandList = (outs);
  let InOperandList = (ins variable_ops);
  let AsmString = "XPHI";
  let isCodeGenOnly = 1;
  let isPseudo = 1;
}

// These are target-independent nodes, but have target-specific formats.
def SDT_CSACall         : SDTypeProfile<0, 1, [SDTCisVT<0, iPTR>]>;
def SDT_CSACallSeqStart : SDCallSeqStart<[ SDTCisVT<0, i64> ]>;
def SDT_CSACallSeqEnd   : SDCallSeqEnd<[ SDTCisVT<0, i64>, SDTCisVT<1, i64> ]>;

def SDT_CSAWrapper      : SDTypeProfile<1, 1, [SDTCisSameAs<0,1>,
                                               SDTCisPtrTy<0>]>;

def CSARet : SDNode<"CSAISD::Ret", SDTNone,
    [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

// Call
def CSACall : SDNode<"CSAISD::Call", SDT_CSACall,
                      [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                      SDNPVariadic]>;

def CSATailCall : SDNode<"CSAISD::TailCall", SDT_CSACall,
                      [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_CSACallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END",   SDT_CSACallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def CSAWrapper : SDNode<"CSAISD::Wrapper", SDT_CSAWrapper>;

let Defs = [SP], Uses = [SP] in {
def ADJCALLSTACKDOWN : PseudoInstCSA<(outs), (ins i64imm:$amt),
                               "# ADJCALLSTACKDOWN $amt",
                               [(callseq_start timm:$amt)]>;
def ADJCALLSTACKUP : PseudoInstCSA<(outs), (ins i64imm:$amt1, i64imm:$amt2),
                            "# ADJCALLSTACKUP $amt1",
                            [(callseq_end timm:$amt1, timm:$amt2)]>;
}

class MovOp<GenericOp gen, CSAOpInfo oi,
            InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst),
    (ins oi.RC:$op1), // Reg or literal (e.g. either i32 or f32 for 32b)
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1"),
    [],
    !listconcat(preds,oi.Preds), itin> {
      let OpInfo = oi;
    }

class UnaryOp<GenericOp gen, SDNode opNode, CSAOpInfo oi,
              InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1"),
    [(set oi.RC:$dst, (opNode oi.RCL:$op1))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

// Separate pattern because not/ineg are PatFrags rather than SDNodes
class UnaryOpP<GenericOp gen, PatFrag opNode, CSAOpInfo oi,
               InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1"),
    [(set oi.RC:$dst, (opNode oi.RCL:$op1))],
    !listconcat(preds,gen.Preds,oi.Preds), itin> {
      let OpInfo = oi;
      let AddedComplexity = 1;  // prefer not/neg to other forms
    }

class SExtOp<GenericOp gen, CSAOpInfo oid, CSAOpInfo ois,
             InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs oid.RC:$dst),
    (ins ois.RCL:$op1, ois.RCL:$op2),
    !strconcat(gen.AsmString, oid.InstrSuffix, "\t$dst, $op1, $op2"),
    [],
    !listconcat(preds,gen.Preds,oid.Preds), itin>;

class CvtOp<GenericOp gen, SDNode opNode, CSAOpInfo oid, CSAOpInfo ois,
            InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs oid.RC:$dst),
    (ins ois.RCL:$src),
    !strconcat(gen.AsmString, oid.InstrSuffix, ois.InstrSuffix, "\t$dst, $src"),
    [(set oid.RC:$dst, (opNode ois.RCL:$src))],
    !listconcat(preds,gen.Preds,oid.Preds,ois.Preds), itin>;

// We do not bother trying to specify commutative, since there doesn't
// appear to be an advantage.
class BinOp<GenericOp gen, SDNode opNode, CSAOpInfo oi,
            InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1, oi.RCL:$op2),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1, $op2"),
    [(set oi.RC:$dst, (opNode oi.RCL:$op1, oi.RCL:$op2))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

// Same as BinOp, but with an optional rounding mode.
class BinOpR<GenericOp gen, SDNode opNode, CSAOpInfo oi,
            InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1, oi.RCL:$op2, RMODE:$rm),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1, $op2, $rm"),
    [(set oi.RC:$dst, (opNode oi.RCL:$op1, oi.RCL:$op2))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

// There is no SDNode for these. If we want them to be selected, we'll need
// custom selection in CSAISelLowering due to TableGen's inability to deal with
// multi-output selection.
class BinOpC<GenericOp gen, CSAOpInfo oi,
            InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst, I1:$cout),
    (ins oi.RCL:$op1, oi.RCL:$op2, Ti1.RCL:$cin),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $cout, $op1, $op2, $cin"),
    [],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

// Shift ops specifically have an i8 for the shift amount
class ShiftOp<GenericOp gen, SDNode opNode, CSAOpInfo oi,
              InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1, RCLi8:$op2),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1, $op2"),
    [(set oi.RC:$dst, (opNode oi.RCL:$op1, RCLi8:$op2))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

// CmpOp is identical to BinOp, except opNode is a PatFrag rather than
// an SDNode...
class CmpOp<GenericOp gen, PatFrag opNode, CSAOpInfo oi,
            InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs I1:$dst),
    (ins oi.RCL:$op1, oi.RCL:$op2),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1, $op2"),
    [(set I1:$dst, (opNode (oi.VT oi.RCL:$op1), (oi.VT oi.RCL:$op2)))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

// BSEL - Only allow constant 1st operand, so CSA network only needs to support
// 2 operand input boxes.
class BSelOp<GenericOp gen, CSAOpInfo oi,
             InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst),
    (ins oi.L:$sel, oi.RCL:$v0, oi.RCL:$v1),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $sel, $v0, $v1"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin>;

class ShAdd<GenericOp gen, SDNode inner, SDNode outer, CSAOpInfo oi,
            InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1, RCLi8:$op2, oi.RCL:$op3),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1, $op2, $op3"),
    [(set oi.RC:$dst, (outer (inner oi.RCL:$op1, RCLi8:$op2), oi.RCL:$op3))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

// 'fma' is a pattern already defined by LLVM.
// Create missing equivalents for fms and fmrs.
class TriOpFrag<dag res> : PatFrag<(ops node:$LHS, node:$MHS, node:$RHS), res>;
def fms : TriOpFrag<(fma node:$LHS, node:$MHS, (fneg node:$RHS))>;
def fmrs: TriOpFrag<(fma (fneg node:$LHS), node:$MHS, node:$RHS)>;

class FusedOp1<GenericOp gen, SDNode inner, SDNode outer, CSAOpInfo oi,
               InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1, oi.RCL:$op2, oi.RCL:$op3, RMODE:$rm),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1, $op2, $op3, $rm"),
    [(set oi.RC:$dst, (outer (inner oi.RCL:$op1, oi.RCL:$op2), oi.RCL:$op3))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

class FusedOp2<GenericOp gen, SDNode inner, SDNode outer, CSAOpInfo oi,
               InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1, oi.RCL:$op2, oi.RCL:$op3, RMODE:$rm),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op2, $op3, $op1, $rm"),
    [(set oi.RC:$dst, (outer oi.RCL:$op1, (inner oi.RCL:$op2, oi.RCL:$op3)))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

class TriOp1<GenericOp gen, SDPatternOperator node, CSAOpInfo oi,
               InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1, oi.RCL:$op2, oi.RCL:$op3, RMODE:$rm),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1, $op2, $op3, $rm"),
    [(set oi.RC:$dst, (node oi.RCL:$op1, oi.RCL:$op2, oi.RCL:$op3))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

class CopyOp<GenericOp gen, CSAOpInfo oi,
             InstrItinClass itin = IIVir, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$op0, oi.RC:$op1, oi.RC:$op2, oi.RC:$op3),
    (ins oi.RCL:$op4),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$op0, $op1, $op2, $op3, $op4"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin>;

class RAppOp<GenericOp gen, CSAOpInfo oi,
             InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst, I1:$pred),
    (ins  oi.RCL:$op2, oi.RCL:$op3),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $pred, $op2, $op3"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin>;

class RSqrtAppOp<GenericOp gen, CSAOpInfo oi,
             InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst, I1:$pred),
    (ins  oi.RCL:$op2),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $pred, $op2"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin>;

class MergeOp<string opStr, CSAOpInfo t,
              list<Predicate> preds, InstrItinClass itin> :
  FMTGEN<
    (outs t.RC:$dst),
    (ins I1:$sel, t.RCL:$v0, t.RCL:$v1),
    !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
    [(set t.VT:$dst, (select I1:$sel, (t.VT t.RCL:$v1), (t.VT t.RCL:$v0)))],
    preds, itin>;

class SwitchOp<GenericOp gen, CSAOpInfo oi,
               InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst0, oi.RC:$dst1),
    (ins I1:$sel, oi.RCL:$v),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst0, $dst1, $sel, $v"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin>;

class SwitchAnyOp<GenericOp gen, CSAOpInfo oi,
                  InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst0, oi.RC:$dst1, I1:$sel),
    (ins oi.RCL:$v),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst0, $dst1, $sel, $v"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin>;

class PickOp<GenericOp gen, CSAOpInfo oi,
             InstrItinClass itin, list<Predicate> preds = []>:
  FMTGEN<
    (outs oi.RC:$dst),
    (ins I1:$sel, oi.RCL:$v0, oi.RCL:$v1),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $sel, $v0, $v1"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin>;

class PickAnyOp<GenericOp gen, CSAOpInfo oi,
                InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$dst, I1:$sel),
    (ins oi.RCL:$v0, oi.RCL:$v1),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $sel, $v0, $v1"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin>;

// We thought that hasSideEffects would capture the internal state and stream
// output behavior, but it's really not the right thing.  From the standpoint
// of retained state, each time a SEQ or REPEAT is triggered, it has a fresh
// new state and is uneffected by previous "side effects".  Marking these
// operations as having side effects has not benefit and interferes with dead
// instruction detection.
class SeqCOp<GenericOp gen, CSAOpInfo oi,
             InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<  // TODO: something to reflect state
    (outs oi.RC:$val, I1:$pred, I1:$first, I1:$last),
    (ins oi.RCL:$base, oi.RCL:$count, oi.RCL:$stride),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$val, $pred, $first, $last, $base, $count, $stride"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin> { }

// See comment for SeqCOp, above, as to why hasSideEffects is not being set.
// TODO: something to reflect state
class SeqSOp<GenericOp gen, CSAOpInfo oi,
             InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$val, I1:$pred, I1:$first, I1:$last),
    (ins oi.RCL:$base, oi.RCL:$bound, oi.RCL:$stride),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$val, $pred, $first, $last, $base, $bound, $stride"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin> { }

// See comment for SeqCOp, above, as to why hasSideEffects is not being set.
class RepeatOp<GenericOp gen, CSAOpInfo oi,
               InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<  // TODO: something to reflect state
    (outs oi.RC:$out),
    (ins I1:$pred, oi.RCL:$in),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$out, $pred, $in"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin> { }

// See comment for SeqCOp, above, as to why hasSideEffects is not being set.
class StrideOp<GenericOp gen, CSAOpInfo oi,
               InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<  // TODO: something to reflect state
    (outs oi.RC:$out),
    (ins I1:$pred, oi.RCL:$base, oi.RCL:$stride),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$out, $pred, $base, $stride"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin> { }

class SReduceOp<GenericOp gen, CSAOpInfo oi,
                InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$result, oi.RC:$each),
    (ins oi.RCL:$init, oi.RCL:$inval, I1:$ctl),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$result, $each, $init, $inval, $ctl"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin> { }

class SReduceOpR<GenericOp gen, CSAOpInfo oi,
                InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$result, oi.RC:$each),
    (ins oi.RCL:$init, oi.RCL:$inval, I1:$ctl, RMODE:$rm),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$result, $each, $init, $inval, $ctl, $rm"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin> { }

class FMSReduceOp<GenericOp gen, CSAOpInfo oi,
                  InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<
    (outs oi.RC:$result, oi.RC:$each),
    (ins oi.RCL:$init, oi.RCL:$inval1, oi.RCL:$inval2, I1:$ctl, RMODE:$rm),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$result, $each, $init, $inval1, $inval2, $ctl, $rm"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin> { }

multiclass LdOp<string opStr, CSAOpInfo t,
           InstrItinClass itin = IILD> {
  let Predicates = [HasIndex], Itinerary = itin in {
    def X : FMTGEN< // indexed reg+reg
      (outs t.RC:$dst),
      (ins ADDR_RX:$addr),
      !strconcat(opStr, "x\t$dst, $addr"),
      [(set t.VT:$dst, (load ADDR_RX:$addr))]>;
  }

  let Predicates = [HasDispl], Itinerary = itin in {
    def D : FMTGEN< // normal literal displacement form (val+k)
      (outs t.RC:$dst),
      (ins ADDR_RI:$addr),
      !strconcat(opStr, "d\t$dst, $addr"),
      [(set t.VT:$dst, (load ADDR_RI:$addr))]>;
    def R : FMTGEN< // non-indexed reg+reg
      (outs t.RC:$dst),
      (ins ADDR_RR:$addr),
      !strconcat(opStr, "d\t$dst, $addr"),
      [(set t.VT:$dst, (load ADDR_RR:$addr))]>;
  }

  let Itinerary = itin in {
    def I : FMTGEN<  // e.g. ld of literal address - ld rx,sym
      (outs t.RC:$dst),
      (ins ADDR_I:$addr),
      !strconcat(opStr, "\t$dst, $addr"),
      [(set t.VT:$dst, (load ADDR_I:$addr))]>;
    def "" : FMTGEN< // normal basereg form
      (outs t.RC:$dst),
      (ins ADDR_R:$addr),
      !strconcat(opStr, "\t$dst, $addr"),
      [(set t.VT:$dst, (load ADDR_R:$addr))]>;
    }
}

// Ugly combinatorial issue between literal store data and addressing modes
multiclass StOp<string opStr, CSAOpInfo t,
           InstrItinClass itin = IIST> {
  let Predicates = [HasIndex], mayStore = 1, Itinerary = itin in {
    def X : FMTGEN<
      (outs),
      (ins ADDR_RX:$addr, t.RCL:$data),
      !strconcat(opStr, "x\t$addr, $data"),
      [(store (t.VT t.RCL:$data), ADDR_RX:$addr)]>;
  }

  let Predicates = [HasDispl], Itinerary = itin in {
    def D : FMTGEN<
      (outs),
      (ins ADDR_RI:$addr, t.RCL:$data),
      !strconcat(opStr, "d\t$addr, $data, %ign"),
      [(store (t.VT t.RCL:$data), ADDR_RI:$addr)]>;
    def R : FMTGEN<
      (outs),
      (ins ADDR_RR:$addr, t.RCL:$data),
      !strconcat(opStr, "d\t$addr, $data, %ign"),
      [(store (t.VT t.RCL:$data), ADDR_RR:$addr)]>;
  }

  let Itinerary = itin in {
    def "" : FMTGEN<
      (outs),
      (ins ADDR_R:$addr, t.RCL:$data),
      !strconcat(opStr, "\t$addr, $data, %ign"),
      [(store (t.VT t.RCL:$data), ADDR_R:$addr)]>;
  }
}

// TBD(jsukha): Even uglier hack to defined "ordered" memory
// instructions, i.e., the equivalent LD/ST instructions as above,
// except with two additional operands.
//
// One extra argument is an output($issued flag),
// and one extra argument is an input ($ready flag).
//
// There might be a nice way to change the definition
// of the existing load/store operations, to do the same thing,
// and/or define more macros to eliminate duplicated code between
// these two cases. But this hack is what I could figure out for now.

multiclass OLdOp<string opStr, CSAOpInfo t,
           InstrItinClass itin = IILD> {
  let Predicates = [HasIndex], Itinerary = itin in {
    def X : FMTGEN< // indexed reg+reg
      (outs t.RC:$dst, I0:$issued),
      (ins ADDR_RX:$addr, I0:$ready),
      !strconcat(opStr, "x\t$dst, $addr, $issued, $ready"),
      []>;
  }

  let Predicates = [HasDispl], Itinerary = itin in {
    def D : FMTGEN< // normal literal displacement form (val+k)
      (outs t.RC:$dst, I0:$issued),
      (ins ADDR_RI:$addr, I0:$ready),
      !strconcat(opStr, "d\t$dst, $addr, $issued, $ready"),
      []>;
    def R : FMTGEN< // non-indexed reg+reg
      (outs t.RC:$dst, I0:$issued),
      (ins ADDR_RR:$addr, I0:$ready),
      !strconcat(opStr, "d\t$dst, $addr, $issued, $ready"),
      []>;
  }

  let Itinerary = itin in {
    def I : FMTGEN<  // e.g. ld of literal address - ld rx,sym
      (outs t.RC:$dst, I0:$issued),
      (ins ADDR_I:$addr, I0:$ready),
      !strconcat(opStr, "\t$dst, $addr, $issued, $ready"),
      []>;
    def "" : FMTGEN< // normal basereg form
      (outs t.RC:$dst, I0:$issued),
      (ins ADDR_R:$addr, I0:$ready),
      !strconcat(opStr, "\t$dst, $addr, $issued, $ready"),
      []>;
    }
}

multiclass OStOp<string opStr, CSAOpInfo t,
           InstrItinClass itin = IIST> {
  let Predicates = [HasIndex], mayStore = 1, Itinerary = itin in {
    def X : FMTGEN<
      (outs I0:$issued),
      (ins ADDR_RX:$addr, t.RCL:$data, I0:$ready),
      !strconcat(opStr, "x\t$addr, $data, $issued, $ready"),
      []>;
  }

  let Predicates = [HasDispl], mayStore = 1, Itinerary = itin in {
    def D : FMTGEN<
      (outs I0:$issued),
      (ins ADDR_RI:$addr, t.RCL:$data, I0:$ready),
      !strconcat(opStr, "d\t$addr, $data, $issued, $ready"),
      []>;
    def R : FMTGEN<
      (outs I0:$issued),
      (ins ADDR_RR:$addr, t.RCL:$data, I0:$ready),
      !strconcat(opStr, "d\t$addr, $data, $issued, $ready"),
      []>;
  }

  let Itinerary = itin, mayStore = 1 in {
      def "" : FMTGEN<
        (outs I0:$issued),
        (ins ADDR_R:$addr, t.RCL:$data, I0:$ready),
        !strconcat(opStr, "\t$addr, $data, $issued, $ready"),
        []>;
  }
}

class MullohiOp<GenericOp gen, CSAOpInfo oi,
               InstrItinClass itin = IIALU, list<Predicate> preds = []> :
  FMTGEN <
    (outs oi.RC:$lo, oi.RC:$hi),
    (ins  oi.RCL:$op1, oi.RCL:$op2),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$lo, $hi, $op1, $op2"),
    [],
    !listconcat(preds,oi.Preds), itin>
    { let mayLoad = 0; let mayStore = 0; let hasSideEffects = 0; }

class FModOp<GenericOp gen, CSAOpInfo oi,
               InstrItinClass itin = IIALU, list<Predicate> preds = []> :
  FMTGEN <
    (outs oi.RC:$intp, oi.RC:$fracp),
    (ins  oi.RCL:$op1),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$intp, $fracp, $op1"),
    [],
    !listconcat(preds,oi.Preds), itin>
    { let mayLoad = 0; let mayStore = 0; let hasSideEffects = 0; }

class AtomicOp<GenericOp gen, SDPatternOperator opNode, CSAOpInfo oi,
               InstrItinClass itin = IIATM, list<Predicate> preds = []> :
  FMTGEN <
    (outs oi.RC:$dst),
    (ins ADDR_R:$addr, oi.RCL:$op3),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t%ign, $dst, $addr, $op3, %ign\t# NOTE: Generated as unordered."),
    [(set oi.VT:$dst, (opNode ADDR_R:$addr, (oi.VT oi.RCL:$op3)))],
    !listconcat(preds,oi.Preds), itin>
    { let mayLoad = 1; let mayStore = 1; let hasSideEffects = 1; }

class OAtomicOp<GenericOp gen, CSAOpInfo oi,
                InstrItinClass itin = IIATM, list<Predicate> preds = []> :
  FMTGEN <
    (outs oi.RC:$dst, I0:$issued),
    (ins ADDR_R:$addr, oi.RCL:$op3, I0:$ready),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$issued, $dst, $addr, $op3, $ready"),
    [], !listconcat(preds,oi.Preds), itin>
    { let mayLoad = 1; let mayStore = 1; let hasSideEffects = 1; }

class AtomicOpR<GenericOp gen, SDPatternOperator opNode, CSAOpInfo oi,
               InstrItinClass itin = IIATM, list<Predicate> preds = []> :
  FMTGEN <
    (outs oi.RC:$dst),
    (ins ADDR_R:$addr, oi.RCL:$op3, RMODE:$rm),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t%ign, $dst, $addr, $op3, %ign, $rm\t# NOTE: Generated as unordered."),
    [(set oi.VT:$dst, (opNode ADDR_R:$addr, (oi.VT oi.RCL:$op3)))],
    !listconcat(preds,oi.Preds), itin>
    { let mayLoad = 1; let mayStore = 1; let hasSideEffects = 1; }

class OAtomicOpR<GenericOp gen, CSAOpInfo oi,
                InstrItinClass itin = IIATM, list<Predicate> preds = []> :
  FMTGEN <
    (outs oi.RC:$dst, I0:$issued),
    (ins ADDR_R:$addr, oi.RCL:$op3, I0:$ready, RMODE:$rm),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$issued, $dst, $addr, $op3, $ready, $rm"),
    [], !listconcat(preds,oi.Preds), itin>
    { let mayLoad = 1; let mayStore = 1; let hasSideEffects = 1; }

class AtomicOp2<GenericOp gen, PatFrag opNode, CSAOpInfo oi,
                InstrItinClass itin = IIATM, list<Predicate> preds = []> :
  FMTGEN <
    (outs oi.RC:$dst),
    (ins ADDR_R:$addr, oi.RCL:$op3, oi.RCL:$op4),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t%ign, $dst, $addr, $op3, $op4, %ign\t# NOTE: Generated as unordered."),
    [(set oi.RC:$dst, (opNode ADDR_R:$addr, oi.RCL:$op4, oi.RCL:$op3))],
    !listconcat(preds,oi.Preds), itin>
    { let mayLoad = 1; let mayStore = 1; let hasSideEffects = 1; }

class OAtomicOp2<GenericOp gen, CSAOpInfo oi,
                 InstrItinClass itin = IIATM, list<Predicate> preds = []> :
  FMTGEN <
    (outs oi.RC:$dst, I0:$issued),
    (ins ADDR_R:$addr, oi.RCL:$op3, oi.RCL:$op4, I0:$ready),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$issued, $dst, $addr, $op3, $op4, $ready"),
    [], !listconcat(preds,oi.Preds), itin>
    { let mayLoad = 1; let mayStore = 1; let hasSideEffects = 1; }

let isBranch=1, isTerminator=1, Itinerary = IICtl in {
  // Branch true
  def BT : FMTGEN<(outs), (ins I1:$cond, brtarget:$target),
    "bt\t$cond, $target",
    [(brcond I1:$cond, bb:$target)]>;
  // Branch false
  def BF : FMTGEN<(outs), (ins I1:$cond, brtarget:$target),
    "bf\t$cond, $target",
    [(brcond (not I1:$cond), bb:$target)]>;
  // Unconditional branch
  let isBarrier=1 in {
    def BR : FMTGEN<(outs), (ins brtarget:$target),
      "br\t$target",
      [(br bb:$target)]>;
  }
}

def : Pat<(brcond (i1 (xor I1:$cond, -1)), bb:$target),
          (BF I1:$cond, bb:$target)>;
def : Pat<(brcond (i1 (setne I1:$cond, -1)), bb:$target),
          (BF I1:$cond, bb:$target)>;

let isReturn=1, isTerminator=1, isBarrier=1, Itinerary = IICtl in
def RET : FMTGEN<
    (outs),
    (ins),
    "ret\t%ra",         // implicit use of RA
    [(CSARet)]>;

let isBranch=1, isTerminator=1, isBarrier=1, Itinerary = IICtl in
def JMP : FMTGEN<
  (outs),
  (ins I64:$target),
  "jmp\t$target",
  [(brind I64:$target)]>;

let isCall=1, Itinerary = IICtl,
    Defs = [
      R0,  R1,  R2,  R3,  R4,  R5,  R6,  R7,
      R8,  R9,  R10, R11, R12, R13, R14, R15,
      R16, R17, R18, R19, R20, R21, R22, R23,
   // R24, R25, R26, R27, R28, R29, R30, R31,  // preserved
   // R32, R33, R34, R35, R36, R37, R38, R39,  // preserved
      R40, R41, R42, R43, R44, R45, R46, R47,
      R48, R49, R50, R51, R52, R53, R54, R55,
      R56, R57, R58, R59,/*FP, TP,  SP,*/RA  ] in {

  def JSR : FMTGEN<
    (outs),
    (ins I64:$target, variable_ops),
    "jsr\t%ra, $target",        // implicit use of RA
    []>;

  def JSRi : FMTGEN<
    (outs),
    (ins calltarget:$target, variable_ops),
    "jsr\t%ra, $target",        // implicit use of RA
    []>;

  let isTerminator=1, isReturn=1, isBarrier=1, hasExtraSrcRegAllocReq=1,
    isCodeGenOnly=1 in {
    def JTR : FMTGEN<
      (outs),
      (ins I64:$target, variable_ops),
      "jmp\t$target",
      []>;

    def JTRi : FMTGEN<
      (outs),
      (ins calltarget:$target, variable_ops),
      "jmp\t$target",
      []>;
  }

}

def : Pat<(CSACall tglobaladdr:$dst),      (JSRi tglobaladdr:$dst)>;
def : Pat<(CSACall texternalsym:$dst),     (JSRi texternalsym:$dst)>;
def : Pat<(CSACall imm:$dst),              (JSRi imm:$dst)>;
def : Pat<(CSACall I64:$dst),              (JSR I64:$dst)>;

// Tail call
def : Pat<(CSATailCall tglobaladdr:$dst),  (JTRi tglobaladdr:$dst)>;
def : Pat<(CSATailCall texternalsym:$dst), (JTRi texternalsym:$dst)>;
def : Pat<(CSATailCall imm:$dst),          (JTRi imm:$dst)>;
def : Pat<(CSATailCall I64:$dst),          (JTR I64:$dst)>;

def MOV : GenericOp<"mov">;
def MOV0      : MovOp<MOV, Ti0,  IIALU>;
def MOV1      : MovOp<MOV, Ti1,  IIALU>;
def MOV8      : MovOp<MOV, Ti8,  IIALU>;
def MOV16     : MovOp<MOV, Ti16, IIALU>;
def MOV32     : MovOp<MOV, Ti32, IIALU>;
def MOV64     : MovOp<MOV, Ti64, IIALU>;

def : Pat<(i64 (CSAWrapper tglobaladdr:$src)),   (MOV64 tglobaladdr:$src)>;
def : Pat<(i64 (CSAWrapper texternalsym:$src)),  (MOV64 texternalsym:$src)>;
def : Pat<(i64 (CSAWrapper tblockaddress:$src)), (MOV64 tblockaddress:$src)>;
def : Pat<(i64 (CSAWrapper tjumptable:$src)),    (MOV64 tjumptable:$src)>;

def NOT       : GenericOp<"not">;
def NOT1      : UnaryOpP<NOT,    not,    Ti1,  IIALU>;
def NOT8      : UnaryOpP<NOT,    not,    Ti8,  IIALU>;
def NOT16     : UnaryOpP<NOT,    not,    Ti16, IIALU>;
def NOT32     : UnaryOpP<NOT,    not,    Ti32, IIALU>;
def NOT64     : UnaryOpP<NOT,    not,    Ti64, IIALU>;

def NEG       : GenericOp<"neg">;
def NEG8      : UnaryOpP<NEG,    ineg,   Ti8,  IIALU>;
def NEG16     : UnaryOpP<NEG,    ineg,   Ti16, IIALU>;
def NEG32     : UnaryOpP<NEG,    ineg,   Ti32, IIALU>;
def NEG64     : UnaryOpP<NEG,    ineg,   Ti64, IIALU>;

def NEGF16    : UnaryOp< NEG,    fneg,   Tf16, IIALU>;
def NEGF32    : UnaryOp< NEG,    fneg,   Tf32, IIALU>;
def NEGF64    : UnaryOp< NEG,    fneg,   Tf64, IIALU>;

def ABS       : GenericOp<"abs">;
def ABSF16    : UnaryOp< ABS,    fabs,   Tf16, IIALU>;
def ABSF32    : UnaryOp< ABS,    fabs,   Tf32, IIALU>;
def ABSF64    : UnaryOp< ABS,    fabs,   Tf64, IIALU>;

def SQRT      : GenericOp<"sqrt", [HasSqrt]>;
def SQRTF16   : UnaryOp< SQRT,   fsqrt,  Tf16, IISqrtF16>;
def SQRTF32   : UnaryOp< SQRT,   fsqrt,  Tf32, IISqrtF32>;
def SQRTF64   : UnaryOp< SQRT,   fsqrt,  Tf64, IISqrtF64>;

def EXP2      : GenericOp<"exp2", [HasMath0]>;
def EXP2F16   : UnaryOp< EXP2,   fexp2,  Tf16, IIMathF16>;
def EXP2F32   : UnaryOp< EXP2,   fexp2,  Tf32, IIMathF32>;
def EXP2F64   : UnaryOp< EXP2,   fexp2,  Tf64, IIMathF64>;

def LOG2      : GenericOp<"log2", [HasMath0]>;
def LOG2F16   : UnaryOp< LOG2,   flog2,  Tf16, IIMathF16>;
def LOG2F32   : UnaryOp< LOG2,   flog2,  Tf32, IIMathF32>;
def LOG2F64   : UnaryOp< LOG2,   flog2,  Tf64, IIMathF64>;

def EXP       : GenericOp<"exp", [HasMath0]>;
def EXPF16    : UnaryOp< EXP,    fexp,   Tf16, IIMathF16>;
def EXPF32    : UnaryOp< EXP,    fexp,   Tf32, IIMathF32>;
def EXPF64    : UnaryOp< EXP,    fexp,   Tf64, IIMathF64>;

def LOG       : GenericOp<"log", [HasMath0]>;
def LOGF16    : UnaryOp< LOG,    flog,   Tf16, IIMathF16>;
def LOGF32    : UnaryOp< LOG,    flog,   Tf32, IIMathF32>;
def LOGF64    : UnaryOp< LOG,    flog,   Tf64, IIMathF64>;

def SIN       : GenericOp<"sin", [HasMath0]>;
def SINF16    : UnaryOp< SIN,    fsin,   Tf16, IIMathF16>;
def SINF32    : UnaryOp< SIN,    fsin,   Tf32, IIMathF32>;
def SINF64    : UnaryOp< SIN,    fsin,   Tf64, IIMathF64>;

def COS       : GenericOp<"cos", [HasMath0]>;
def COSF16    : UnaryOp< COS,    fcos,   Tf16, IIMathF16>;
def COSF32    : UnaryOp< COS,    fcos,   Tf32, IIMathF32>;
def COSF64    : UnaryOp< COS,    fcos,   Tf64, IIMathF64>;

def MOD       : GenericOp<"mod", [HasMath0]>;
def MODF32    : FModOp<  MOD,            Tf32, IIMathF32>;
def MODF64    : FModOp<  MOD,            Tf64, IIMathF64>;

//def SINCOS : GenericOp<"sincos", [HasMath0]>;
//def SINCOSF16 : UnaryOp<SINCOS,fsincos,Tf16, IIMathF16>;
//def SINCOSF32 : UnaryOp<SINCOS,fsincos,Tf32, IIMathF32>;
//def SINCOSF64 : UnaryOp<SINCOS,fsincos,Tf64, IIMathF64>;

// BitOps. Note that LLVM's SDNodes expect the output type to match the input
// type, while in the simulator all output types are 8-bit.
def CTPOP     : GenericOp<"ctpop", [HasBitOp]>;
def CTPOP8    : UnaryOp< CTPOP,  ctpop,  Ti8,  IIALU>;
def CTPOP16   : UnaryOp< CTPOP,  ctpop,  Ti16, IIALU>;
def CTPOP32   : UnaryOp< CTPOP,  ctpop,  Ti32, IIALU>;
def CTPOP64   : UnaryOp< CTPOP,  ctpop,  Ti64, IIALU>;

def CTLZ      : GenericOp<"ctlz", [HasBitOp]>;
def CTLZ8     : UnaryOp< CTLZ,   ctlz,   Ti8,  IIALU>;
def CTLZ16    : UnaryOp< CTLZ,   ctlz,   Ti16, IIALU>;
def CTLZ32    : UnaryOp< CTLZ,   ctlz,   Ti32, IIALU>;
def CTLZ64    : UnaryOp< CTLZ,   ctlz,   Ti64, IIALU>;

def CTTZ      : GenericOp<"cttz", [HasBitOp]>;
def CTTZ8     : UnaryOp< CTTZ,   cttz,   Ti8,  IIALU>;
def CTTZ16    : UnaryOp< CTTZ,   cttz,   Ti16, IIALU>;
def CTTZ32    : UnaryOp< CTTZ,   cttz,   Ti32, IIALU>;
def CTTZ64    : UnaryOp< CTTZ,   cttz,   Ti64, IIALU>;

// Helper fragment to find "parity". Clang's __builtin_parity(x) will result in
// (ctpop(x)&1) in IR, which will be transformed back to parityN(x) by this
// pattern fragment.
def parity : PatFrag<(ops node:$in), (and (ctpop node:$in), 1)>;
def PARITY    : GenericOp<"parity", [HasBitOp]>;
def PARITY8   : UnaryOpP<PARITY, parity, Ti8,  IIALU>;
def PARITY16  : UnaryOpP<PARITY, parity, Ti16, IIALU>;
def PARITY32  : UnaryOpP<PARITY, parity, Ti32, IIALU>;
def PARITY64  : UnaryOpP<PARITY, parity, Ti64, IIALU>;

def SEXT      : GenericOp<"sext">;
def SEXT8     : SExtOp<  SEXT,           Ti8,  Ti8,  IIALU>;
def SEXT16    : SExtOp<  SEXT,           Ti16, Ti16, IIALU>;
def SEXT32    : SExtOp<  SEXT,           Ti32, Ti32, IIALU>;
def SEXT64    : SExtOp<  SEXT,           Ti64, Ti64, IIALU>;

// first type in convert name is result, second is source type
def CVT       : GenericOp<"cvt">;
def CVTS32F32 : CvtOp<   CVT, fp_to_sint,Ts32, Tf32, IICvtIF>;
def CVTS32F64 : CvtOp<   CVT, fp_to_sint,Ts32, Tf64, IICvtIF>;

def CVTU32F32 : CvtOp<   CVT, fp_to_uint,Tu32, Tf32, IICvtIF>;
def CVTU32F64 : CvtOp<   CVT, fp_to_uint,Tu32, Tf64, IICvtIF>;

def CVTS64F32 : CvtOp<   CVT, fp_to_sint,Ts64, Tf32, IICvtIF>;
def CVTS64F64 : CvtOp<   CVT, fp_to_sint,Ts64, Tf64, IICvtIF>;

def CVTU64F32 : CvtOp<   CVT, fp_to_uint,Tu64, Tf32, IICvtIF>;
def CVTU64F64 : CvtOp<   CVT, fp_to_uint,Tu64, Tf64, IICvtIF>;

def CVTF32S32 : CvtOp<   CVT, sint_to_fp,Tf32, Ts32, IICvtFI>;
def CVTF32S64 : CvtOp<   CVT, sint_to_fp,Tf32, Ts64, IICvtFI>;
def CVTF32U32 : CvtOp<   CVT, uint_to_fp,Tf32, Tu32, IICvtFI>;
def CVTF32U64 : CvtOp<   CVT, uint_to_fp,Tf32, Tu64, IICvtFI>;

def CVTF64S32 : CvtOp<   CVT, sint_to_fp,Tf64, Ts32, IICvtFI>;
def CVTF64S64 : CvtOp<   CVT, sint_to_fp,Tf64, Ts64, IICvtFI>;
def CVTF64U32 : CvtOp<   CVT, uint_to_fp,Tf64, Tu32, IICvtFI>;
def CVTF64U64 : CvtOp<   CVT, uint_to_fp,Tf64, Tu64, IICvtFI>;

def CVTF32F64 : CvtOp<   CVT, fpround,    Tf32, Tf64, IICvtFF>;

def CVTF64F32 : CvtOp<   CVT, fpextend,   Tf64, Tf32, IICvtFF>;

def AND       : GenericOp<"and">;
def AND1      : BinOp<   AND,    and,    Ti1,  IIALU>;
def AND8      : BinOp<   AND,    and,    Ti8,  IIALU>;
def AND16     : BinOp<   AND,    and,    Ti16, IIALU>;
def AND32     : BinOp<   AND,    and,    Ti32, IIALU>;
def AND64     : BinOp<   AND,    and,    Ti64, IIALU>;

def OR        : GenericOp<"or">;
def OR1       : BinOp<   OR,     or,     Ti1,  IIALU>;
def OR8       : BinOp<   OR,     or,     Ti8,  IIALU>;
def OR16      : BinOp<   OR,     or,     Ti16, IIALU>;
def OR32      : BinOp<   OR,     or,     Ti32, IIALU>;
def OR64      : BinOp<   OR,     or,     Ti64, IIALU>;

def XOR       : GenericOp<"xor">;
def XOR1      : BinOp<   XOR,    xor,    Ti1,  IIALU>;
def XOR8      : BinOp<   XOR,    xor,    Ti8,  IIALU>;
def XOR16     : BinOp<   XOR,    xor,    Ti16, IIALU>;
def XOR32     : BinOp<   XOR,    xor,    Ti32, IIALU>;
def XOR64     : BinOp<   XOR,    xor,    Ti64, IIALU>;

def SLL       : GenericOp<"sll">;
def SLL8      : ShiftOp< SLL,    shl,    Ti8,  IIShft>;
def SLL16     : ShiftOp< SLL,    shl,    Ti16, IIShft>;
def SLL32     : ShiftOp< SLL,    shl,    Ti32, IIShft>;
def SLL64     : ShiftOp< SLL,    shl,    Ti64, IIShft>;

def SRL       : GenericOp<"srl">;
def SRL8      : ShiftOp< SRL,    srl,    Ti8,  IIShft>;
def SRL16     : ShiftOp< SRL,    srl,    Ti16, IIShft>;
def SRL32     : ShiftOp< SRL,    srl,    Ti32, IIShft>;
def SRL64     : ShiftOp< SRL,    srl,    Ti64, IIShft>;

def SRA       : GenericOp<"sra">;
def SRA8      : ShiftOp< SRA,    sra,    Ti8,  IIShft>;
def SRA16     : ShiftOp< SRA,    sra,    Ti16, IIShft>;
def SRA32     : ShiftOp< SRA,    sra,    Ti32, IIShft>;
def SRA64     : ShiftOp< SRA,    sra,    Ti64, IIShft>;

def : Pat<(i1 (add i1:$op1, i1:$op2)),       (XOR1 $op1,$op2)>;
def : Pat<(i1 (add i1:$op1, (i1 imm:$imm))), (XOR1 $op1,$imm)>;

def ADD       : GenericOp<"add">;
def ADD8      : BinOp<   ADD,    add,    Ti8,  IIALU>;
def ADD16     : BinOp<   ADD,    add,    Ti16, IIALU>;
def ADD32     : BinOp<   ADD,    add,    Ti32, IIALU>;
def ADD64     : BinOp<   ADD,    add,    Ti64, IIALU>;

def ADC       : GenericOp<"adc">;
def ADC8      : BinOpC<  ADC,            Ti8,  IIALU>;
def ADC16     : BinOpC<  ADC,            Ti16, IIALU>;
def ADC32     : BinOpC<  ADC,            Ti32, IIALU>;
def ADC64     : BinOpC<  ADC,            Ti64, IIALU>;

def ADDF16    : BinOpR<  ADD,    fadd,   Tf16, IIAddF16>;
def ADDF32    : BinOpR<  ADD,    fadd,   Tf32, IIAddF32>;
def ADDF64    : BinOpR<  ADD,    fadd,   Tf64, IIAddF64>;

def : Pat<(i1 (sub i1:$op1, i1:$op2)),       (XOR1 $op1,$op2)>;
def : Pat<(i1 (sub i1:$op1, (i1 imm:$imm))), (XOR1 $op1,$imm)>;
def SUB       : GenericOp<"sub">;
def SUB8      : BinOp<   SUB,    sub,    Ti8,  IIALU>;
def SUB16     : BinOp<   SUB,    sub,    Ti16, IIALU>;
def SUB32     : BinOp<   SUB,    sub,    Ti32, IIALU>;
def SUB64     : BinOp<   SUB,    sub,    Ti64, IIALU>;

def SBB       : GenericOp<"sbb">;
def SBB8      : BinOpC<  SBB,            Ti8,  IIALU>;
def SBB16     : BinOpC<  SBB,            Ti16, IIALU>;
def SBB32     : BinOpC<  SBB,            Ti32, IIALU>;
def SBB64     : BinOpC<  SBB,            Ti64, IIALU>;

def SUBF16    : BinOpR<  SUB,    fsub,   Tf16, IIAddF16>;
def SUBF32    : BinOpR<  SUB,    fsub,   Tf32, IIAddF32>;
def SUBF64    : BinOpR<  SUB,    fsub,   Tf64, IIAddF64>;

def MUL       : GenericOp<"mul">;
def MUL8      : BinOp<   MUL,    mul,    Ti8,  IIMulI8>;
def MUL16     : BinOp<   MUL,    mul,    Ti16, IIMulI16>;
def MUL32     : BinOp<   MUL,    mul,    Ti32, IIMulI32>;
def MUL64     : BinOp<   MUL,    mul,    Ti64, IIMulI64>;

def MULF16    : BinOpR<  MUL,    fmul,   Tf16, IIMulF16>;
def MULF32    : BinOpR<  MUL,    fmul,   Tf32, IIMulF32>;
def MULF64    : BinOpR<  MUL,    fmul,   Tf64, IIMulF64>;

def DIV       : GenericOp<"div">;
def DIVS8     : BinOp<   DIV,    sdiv,   Ts8,  IIDivI8,  [HasIDiv]>;
def DIVS16    : BinOp<   DIV,    sdiv,   Ts16, IIDivI16, [HasIDiv]>;
def DIVS32    : BinOp<   DIV,    sdiv,   Ts32, IIDivI32, [HasIDiv]>;
def DIVS64    : BinOp<   DIV,    sdiv,   Ts64, IIDivI64, [HasIDiv]>;

def DIVU8     : BinOp<   DIV,    udiv,   Tu8,  IIDivI8,  [HasIDiv]>;
def DIVU16    : BinOp<   DIV,    udiv,   Tu16, IIDivI16, [HasIDiv]>;
def DIVU32    : BinOp<   DIV,    udiv,   Tu32, IIDivI32, [HasIDiv]>;
def DIVU64    : BinOp<   DIV,    udiv,   Tu64, IIDivI64, [HasIDiv]>;

def DIVF16    : BinOpR<  DIV,    fdiv,   Tf16, IIDivF16, [HasFDiv]>;
def DIVF32    : BinOpR<  DIV,    fdiv,   Tf32, IIDivF32, [HasFDiv]>;
def DIVF64    : BinOpR<  DIV,    fdiv,   Tf64, IIDivF64, [HasFDiv]>;

def POW       : GenericOp<"pow", [HasMath0]>;
def POWF32    : BinOp<   POW,    fpow,   Tf32, IIMathF32>;
def POWF64    : BinOp<   POW,    fpow,   Tf64, IIMathF64>;

def CMPLT     : GenericOp<"cmplt">;
def CMPLTS8   : CmpOp<   CMPLT,  setlt,  Ts8,  IIALU>;
def CMPLTS16  : CmpOp<   CMPLT,  setlt,  Ts16, IIALU>;
def CMPLTS32  : CmpOp<   CMPLT,  setlt,  Ts32, IIALU>;
def CMPLTS64  : CmpOp<   CMPLT,  setlt,  Ts64, IIALU>;
def CMPLTF16  : CmpOp<   CMPLT,  setlt,  Tf16, IICmpF>;
def CMPLTF32  : CmpOp<   CMPLT,  setlt,  Tf32, IICmpF>;
def CMPLTF64  : CmpOp<   CMPLT,  setlt,  Tf64, IICmpF>;

def CMPLTU8   : CmpOp<   CMPLT,  setult, Tu8,  IIALU>;
def CMPLTU16  : CmpOp<   CMPLT,  setult, Tu16, IIALU>;
def CMPLTU32  : CmpOp<   CMPLT,  setult, Tu32, IIALU>;
def CMPLTU64  : CmpOp<   CMPLT,  setult, Tu64, IIALU>;

def CMPOLTF16 : CmpOp<   CMPLT,  setolt, Tf16, IICmpF>;
def CMPOLTF32 : CmpOp<   CMPLT,  setolt, Tf32, IICmpF>;
def CMPOLTF64 : CmpOp<   CMPLT,  setolt, Tf64, IICmpF>;

def CMPULTF16 : CmpOp<   CMPLT,  setult, Tf16, IICmpF>;
def CMPULTF32 : CmpOp<   CMPLT,  setult, Tf32, IICmpF>;
def CMPULTF64 : CmpOp<   CMPLT,  setult, Tf64, IICmpF>;

def CMPLE     : GenericOp<"cmple">;
def CMPLES8   : CmpOp<   CMPLE,  setle,  Ts8,  IIALU>;
def CMPLES16  : CmpOp<   CMPLE,  setle,  Ts16, IIALU>;
def CMPLES32  : CmpOp<   CMPLE,  setle,  Ts32, IIALU>;
def CMPLES64  : CmpOp<   CMPLE,  setle,  Ts64, IIALU>;
def CMPLEF16  : CmpOp<   CMPLE,  setle,  Tf16, IICmpF>;
def CMPLEF32  : CmpOp<   CMPLE,  setle,  Tf32, IICmpF>;
def CMPLEF64  : CmpOp<   CMPLE,  setle,  Tf64, IICmpF>;

def CMPLEU8   : CmpOp<   CMPLE,  setule, Tu8,  IIALU>;
def CMPLEU16  : CmpOp<   CMPLE,  setule, Tu16, IIALU>;
def CMPLEU32  : CmpOp<   CMPLE,  setule, Tu32, IIALU>;
def CMPLEU64  : CmpOp<   CMPLE,  setule, Tu64, IIALU>;

def CMPOLEF16 : CmpOp<   CMPLE,  setole, Tf16, IICmpF>;
def CMPOLEF32 : CmpOp<   CMPLE,  setole, Tf32, IICmpF>;
def CMPOLEF64 : CmpOp<   CMPLE,  setole, Tf64, IICmpF>;

def CMPULEF16 : CmpOp<   CMPLE,  setule, Tf16, IICmpF>;
def CMPULEF32 : CmpOp<   CMPLE,  setule, Tf32, IICmpF>;
def CMPULEF64 : CmpOp<   CMPLE,  setule, Tf64, IICmpF>;

// Note - there is no cmpeq1.  As long as operands are clean, cmpeq8
// suffices
def CMPEQ     : GenericOp<"cmpeq">;
def CMPEQ8    : CmpOp<   CMPEQ,  seteq,  Ti8,  IIALU>;
def CMPEQ16   : CmpOp<   CMPEQ,  seteq,  Ti16, IIALU>;
def CMPEQ32   : CmpOp<   CMPEQ,  seteq,  Ti32, IIALU>;
def CMPEQ64   : CmpOp<   CMPEQ,  seteq,  Ti64, IIALU>;
def CMPEQF16  : CmpOp<   CMPEQ,  seteq,  Tf16, IICmpF>;
def CMPEQF32  : CmpOp<   CMPEQ,  seteq,  Tf32, IICmpF>;
def CMPEQF64  : CmpOp<   CMPEQ,  seteq,  Tf64, IICmpF>;

def CMPOEQF16 : CmpOp<   CMPEQ,  setoeq, Tf16, IICmpF>;
def CMPOEQF32 : CmpOp<   CMPEQ,  setoeq, Tf32, IICmpF>;
def CMPOEQF64 : CmpOp<   CMPEQ,  setoeq, Tf64, IICmpF>;

def CMPUEQF16 : CmpOp<   CMPEQ,  setueq, Tf16, IICmpF>;
def CMPUEQF32 : CmpOp<   CMPEQ,  setueq, Tf32, IICmpF>;
def CMPUEQF64 : CmpOp<   CMPEQ,  setueq, Tf64, IICmpF>;

def CMPGT     : GenericOp<"cmpgt">;
def CMPGTS8   : CmpOp<   CMPGT,  setgt,  Ts8,  IIALU>;
def CMPGTS16  : CmpOp<   CMPGT,  setgt,  Ts16, IIALU>;
def CMPGTS32  : CmpOp<   CMPGT,  setgt,  Ts32, IIALU>;
def CMPGTS64  : CmpOp<   CMPGT,  setgt,  Ts64, IIALU>;
def CMPGTF16  : CmpOp<   CMPGT,  setgt,  Tf16, IICmpF>;
def CMPGTF32  : CmpOp<   CMPGT,  setgt,  Tf32, IICmpF>;
def CMPGTF64  : CmpOp<   CMPGT,  setgt,  Tf64, IICmpF>;

def CMPGTU8   : CmpOp<   CMPGT,  setugt, Tu8,  IIALU>;
def CMPGTU16  : CmpOp<   CMPGT,  setugt, Tu16, IIALU>;
def CMPGTU32  : CmpOp<   CMPGT,  setugt, Tu32, IIALU>;
def CMPGTU64  : CmpOp<   CMPGT,  setugt, Tu64, IIALU>;

def CMPOGTF16 : CmpOp<   CMPGT,  setogt, Tf16, IICmpF>;
def CMPOGTF32 : CmpOp<   CMPGT,  setogt, Tf32, IICmpF>;
def CMPOGTF64 : CmpOp<   CMPGT,  setogt, Tf64, IICmpF>;

def CMPUGTF16 : CmpOp<   CMPGT,  setugt, Tf16, IICmpF>;
def CMPUGTF32 : CmpOp<   CMPGT,  setugt, Tf32, IICmpF>;
def CMPUGTF64 : CmpOp<   CMPGT,  setugt, Tf64, IICmpF>;

def CMPGE     : GenericOp<"cmpge">;
def CMPGES8   : CmpOp<   CMPGE,  setge,  Ts8,  IIALU>;
def CMPGES16  : CmpOp<   CMPGE,  setge,  Ts16, IIALU>;
def CMPGES32  : CmpOp<   CMPGE,  setge,  Ts32, IIALU>;
def CMPGES64  : CmpOp<   CMPGE,  setge,  Ts64, IIALU>;
def CMPGEF16  : CmpOp<   CMPGE,  setge,  Tf16, IICmpF>;
def CMPGEF32  : CmpOp<   CMPGE,  setge,  Tf32, IICmpF>;
def CMPGEF64  : CmpOp<   CMPGE,  setge,  Tf64, IICmpF>;

def CMPGEU8   : CmpOp<   CMPGE,  setuge, Tu8,  IIALU>;
def CMPGEU16  : CmpOp<   CMPGE,  setuge, Tu16, IIALU>;
def CMPGEU32  : CmpOp<   CMPGE,  setuge, Tu32, IIALU>;
def CMPGEU64  : CmpOp<   CMPGE,  setuge, Tu64, IIALU>;

def CMPOGEF16 : CmpOp<   CMPGE,  setoge, Tf16, IICmpF>;
def CMPOGEF32 : CmpOp<   CMPGE,  setoge, Tf32, IICmpF>;
def CMPOGEF64 : CmpOp<   CMPGE,  setoge, Tf64, IICmpF>;

def CMPUGEF16 : CmpOp<   CMPGE,  setuge, Tf16, IICmpF>;
def CMPUGEF32 : CmpOp<   CMPGE,  setuge, Tf32, IICmpF>;
def CMPUGEF64 : CmpOp<   CMPGE,  setuge, Tf64, IICmpF>;

def : Pat<(i1 (setne i1:$op1, i1:$op2)), (XOR1 $op1,$op2)>;
def : Pat<(i1 (setne i1:$op1, (i1 imm:$imm))), (XOR1 $op1,$imm)>;
def CMPNE     : GenericOp<"cmpne">;
def CMPNE8    : CmpOp<   CMPNE,  setne,  Ti8,  IIALU>;
def CMPNE16   : CmpOp<   CMPNE,  setne,  Ti16, IIALU>;
def CMPNE32   : CmpOp<   CMPNE,  setne,  Ti32, IIALU>;
def CMPNE64   : CmpOp<   CMPNE,  setne,  Ti64, IIALU>;
def CMPNEF16  : CmpOp<   CMPNE,  setne,  Tf16, IICmpF>;
def CMPNEF32  : CmpOp<   CMPNE,  setne,  Tf32, IICmpF>;
def CMPNEF64  : CmpOp<   CMPNE,  setne,  Tf64, IICmpF>;

def CMPONEF16 : CmpOp<   CMPNE,  setone, Tf16, IICmpF>;
def CMPONEF32 : CmpOp<   CMPNE,  setone, Tf32, IICmpF>;
def CMPONEF64 : CmpOp<   CMPNE,  setone, Tf64, IICmpF>;

def CMPUNEF16 : CmpOp<   CMPNE,  setune, Tf16, IICmpF>;
def CMPUNEF32 : CmpOp<   CMPNE,  setune, Tf32, IICmpF>;
def CMPUNEF64 : CmpOp<   CMPNE,  setune, Tf64, IICmpF>;

def CMPO      : GenericOp<"cmpo">;
def CMPOF16   : CmpOp<   CMPO,   seto,   Tf16, IICmpF>;
def CMPOF32   : CmpOp<   CMPO,   seto,   Tf32, IICmpF>;
def CMPOF64   : CmpOp<   CMPO,   seto,   Tf64, IICmpF>;

def CMPUO     : GenericOp<"cmpuo">;
def CMPUOF16  : CmpOp<   CMPUO,  setuo,  Tf16, IICmpF>;
def CMPUOF32  : CmpOp<   CMPUO,  setuo,  Tf32, IICmpF>;
def CMPUOF64  : CmpOp<   CMPUO,  setuo,  Tf64, IICmpF>;

def BSEL      : GenericOp<"bsel">;
def BSEL1     : BSelOp<  BSEL,           Ti1,  IIALU>;
def BSEL8     : BSelOp<  BSEL,           Ti8,  IIALU>;
def BSEL16    : BSelOp<  BSEL,           Ti16, IIALU>;
def BSEL32    : BSelOp<  BSEL,           Ti32, IIALU>;
def BSEL64    : BSelOp<  BSEL,           Ti64, IIALU>;

// Shift/add - itinerary depends on constant small shift amt - else IShft...
def SLADD     : GenericOp<"sladd", [HasShAdd]>;
def SLADD8    : ShAdd<   SLADD,shl,  add,  Ti8,  IISAdd>;
def SLADD16   : ShAdd<   SLADD,shl,  add,  Ti16, IISAdd>;
def SLADD32   : ShAdd<   SLADD,shl,  add,  Ti32, IISAdd>;
def SLADD64   : ShAdd<   SLADD,shl,  add,  Ti64, IISAdd>;

def FMA       : GenericOp<"fma", [HasFMA]>;
def FMAF16    : TriOp1<FMA,  fma, Tf16, IIFMAF16>;
def FMAF32    : TriOp1<FMA,  fma, Tf32, IIFMAF32>;
def FMAF64    : TriOp1<FMA,  fma, Tf64, IIFMAF64>;

def FMS       : GenericOp<"fms", [HasFMA]>;
def FMSF16    : TriOp1<FMS,  fms, Tf16, IIFMAF16>;
def FMSF32    : TriOp1<FMS,  fms, Tf32, IIFMAF32>;
def FMSF64    : TriOp1<FMS,  fms, Tf64, IIFMAF64>;

def FMRS      : GenericOp<"fmrs", [HasFMA]>;
def FMRSF16   : TriOp1<FMRS, fmrs, Tf16, IIFMAF16>;
def FMRSF32   : TriOp1<FMRS, fmrs, Tf32, IIFMAF32>;
def FMRSF64   : TriOp1<FMRS, fmrs, Tf64, IIFMAF64>;

def RCPA      : GenericOp<"rcpa", [HasRcpA]>;
def RCPAF32   : RAppOp<RCPA,             Tf32, IIRcpAF32>;
def RCPAF64   : RAppOp<RCPA,             Tf64, IIRcpAF64>;

def RSQRTA    : GenericOp<"rsqrta", [HasRSqrtA]>;
def RSQRTAF32 : RSqrtAppOp<RSQRTA,       Tf32, IIRSqrtAF32>;
def RSQRTAF64 : RSqrtAppOp<RSQRTA,       Tf64, IIRSqrtAF64>;

// Unfortunately, "COPY" is already defined...
def GCOPY     : GenericOp<"copy">;
def COPY0     : CopyOp<  GCOPY, Ti0>;
def COPY1     : CopyOp<  GCOPY, Ti1>;
def COPY8     : CopyOp<  GCOPY, Ti8>;
def COPY16    : CopyOp<  GCOPY, Ti16>;
def COPY32    : CopyOp<  GCOPY, Ti32>;
def COPY64    : CopyOp<  GCOPY, Ti64>;

def MERGE     : GenericOp<"merge">;
def MERGE1    : MergeOp< "merge1",  Ti1,  [HasI1],  IIVir>;
def MERGE8    : MergeOp< "merge8",  Ti8,  [HasI8],  IIVir>;
def MERGE16   : MergeOp< "merge16", Ti16, [HasI16], IIVir>;
def MERGE16f  : MergeOp< "merge16", Tf16, [HasI16], IIVir>;
def MERGE32   : MergeOp< "merge32", Ti32, [HasI32], IIVir>;
def MERGE32f  : MergeOp< "merge32", Tf32, [HasI32], IIVir>;
def MERGE64   : MergeOp< "merge64", Ti64, [HasI64], IIVir>;
def MERGE64f  : MergeOp< "merge64", Tf64, [HasI64], IIVir>;

def SWITCH    : GenericOp<"switch">;
def SWITCH1   : SwitchOp<SWITCH, Ti1,  IIVir>;
def SWITCH8   : SwitchOp<SWITCH, Ti8,  IIVir>;
def SWITCH16  : SwitchOp<SWITCH, Ti16, IIVir>;
def SWITCH32  : SwitchOp<SWITCH, Ti32, IIVir>;
def SWITCH64  : SwitchOp<SWITCH, Ti64, IIVir>;

def SWITCHANY : GenericOp<"switchany">;
def SWITCHANY1  : SwitchAnyOp<SWITCHANY, Ti1,  IIVir>;
def SWITCHANY8  : SwitchAnyOp<SWITCHANY, Ti8,  IIVir>;
def SWITCHANY16 : SwitchAnyOp<SWITCHANY, Ti16, IIVir>;
def SWITCHANY32 : SwitchAnyOp<SWITCHANY, Ti32, IIVir>;
def SWITCHANY64 : SwitchAnyOp<SWITCHANY, Ti64, IIVir>;

def PICK      : GenericOp<"pick">;
def PICK1     : PickOp<PICK, Ti1,  IIVir>;
def PICK8     : PickOp<PICK, Ti8,  IIVir>;
def PICK16    : PickOp<PICK, Ti16, IIVir>;
def PICK32    : PickOp<PICK, Ti32, IIVir>;
def PICK64    : PickOp<PICK, Ti64, IIVir>;

def PICKANY   : GenericOp<"pickany">;
def PICKANY1  : PickAnyOp<PICKANY, Ti1,  IIVir>;
def PICKANY8  : PickAnyOp<PICKANY, Ti8,  IIVir>;
def PICKANY16 : PickAnyOp<PICKANY, Ti16, IIVir>;
def PICKANY32 : PickAnyOp<PICKANY, Ti32, IIVir>;
def PICKANY64 : PickAnyOp<PICKANY, Ti64, IIVir>;

def LAND1 : FMTGEN<
  (outs I1:$dst),
  (ins RCLi1:$op1, RCLi1:$op2, RCLi1:$op3, RCLi1:$op4),
  "land1\t$dst, $op1, $op2, $op3, $op4",
  [], [], IIALU>;

def LOR1 : FMTGEN<
  (outs I1:$dst),
  (ins RCLi1:$op1, RCLi1:$op2, RCLi1:$op3, RCLi1:$op4),
  "lor1\t$dst, $op1, $op2, $op3, $op4",
  [], [], IIALU>;

def ANY0 : FMTGEN<
  (outs I8:$dst),
  (ins RCLi0:$op1, RCLi0:$op2, RCLi0:$op3, RCLi0:$op4),
  "any0\t$dst, $op1, $op2, $op3, $op4",
  [], [], IIVir>;

def ALL0 : FMTGEN<
  (outs I0:$dst),
  (ins RCLi0:$op1, RCLi0:$op2, RCLi0:$op3, RCLi0:$op4),
  "all0\t$dst, $op1, $op2, $op3, $op4",
  [], [], IIVir>;

def ONCOUNT0 : FMTGEN<  // TODO: something to reflect state
  (outs I0:$dst),
  (ins RCLi64:$cnt, RCLi0:$op2, RCLi0:$op3, RCLi0:$op4, RCLi0:$op5),
  "oncount0\t$dst, $cnt, $op2, $op3, $op4, $op5",
  [], [], IIVir>;

def PREDPROP : FMTGEN<
  (outs I1:$efalse, I1:$etrue),
  (ins RCLi1:$pb, RCLi1:$sb),
  "predprop\t$efalse, $etrue, $pb, $sb",
  [], [], IIALU>;

def PREDMERGE : FMTGEN<
  (outs I1:$predres, I1:$index),
  (ins RCLi1:$e0, RCLi1:$e1),
  "predmerge\t$predres, $index, $e0, $e1",
  [], [], IIALU>;

def PREDFILTER : FMTGEN<
  (outs I1:$ctlout),
  (ins RCLi1:$ctl, RCLi1:$pred),
  "predfilter\t$ctlout, $ctl, $pred",
  [], [], IIALU>;

def SEQC      : GenericOp<"seqc">;
def SEQC8     : SeqCOp<  SEQC,  Ti8,  IIALU>;
def SEQC16    : SeqCOp<  SEQC,  Ti16, IIALU>;
def SEQC32    : SeqCOp<  SEQC,  Ti32, IIALU>;
def SEQC64    : SeqCOp<  SEQC,  Ti64, IIALU>;

def SEQLT     : GenericOp<"seqlt">;
def SEQLTS8   : SeqSOp<  SEQLT, Ts8,  IIALU>;
def SEQLTS16  : SeqSOp<  SEQLT, Ts16, IIALU>;
def SEQLTS32  : SeqSOp<  SEQLT, Ts32, IIALU>;
def SEQLTS64  : SeqSOp<  SEQLT, Ts64, IIALU>;

def SEQLTU8   : SeqSOp<  SEQLT, Tu8,  IIALU>;
def SEQLTU16  : SeqSOp<  SEQLT, Tu16, IIALU>;
def SEQLTU32  : SeqSOp<  SEQLT, Tu32, IIALU>;
def SEQLTU64  : SeqSOp<  SEQLT, Tu64, IIALU>;

def SEQLE     : GenericOp<"seqle">;
def SEQLES8   : SeqSOp<  SEQLE, Ts8,  IIALU>;
def SEQLES16  : SeqSOp<  SEQLE, Ts16, IIALU>;
def SEQLES32  : SeqSOp<  SEQLE, Ts32, IIALU>;
def SEQLES64  : SeqSOp<  SEQLE, Ts64, IIALU>;

def SEQLEU8   : SeqSOp<  SEQLE, Tu8,  IIALU>;
def SEQLEU16  : SeqSOp<  SEQLE, Tu16, IIALU>;
def SEQLEU32  : SeqSOp<  SEQLE, Tu32, IIALU>;
def SEQLEU64  : SeqSOp<  SEQLE, Tu64, IIALU>;

def SEQNE     : GenericOp<"seqne">;
def SEQNE8    : SeqSOp<  SEQNE, Ti8,  IIALU>;
def SEQNE16   : SeqSOp<  SEQNE, Ti16, IIALU>;
def SEQNE32   : SeqSOp<  SEQNE, Ti32, IIALU>;
def SEQNE64   : SeqSOp<  SEQNE, Ti64, IIALU>;

def SEQGT     : GenericOp<"seqgt">;
def SEQGTS8   : SeqSOp<  SEQGT, Ts8,  IIALU>;
def SEQGTS16  : SeqSOp<  SEQGT, Ts16, IIALU>;
def SEQGTS32  : SeqSOp<  SEQGT, Ts32, IIALU>;
def SEQGTS64  : SeqSOp<  SEQGT, Ts64, IIALU>;

def SEQGTU8   : SeqSOp<  SEQGT, Tu8,  IIALU>;
def SEQGTU16  : SeqSOp<  SEQGT, Tu16, IIALU>;
def SEQGTU32  : SeqSOp<  SEQGT, Tu32, IIALU>;
def SEQGTU64  : SeqSOp<  SEQGT, Tu64, IIALU>;

def SEQGE     : GenericOp<"seqge">;
def SEQGES8   : SeqSOp<  SEQGE, Ts8,  IIALU>;
def SEQGES16  : SeqSOp<  SEQGE, Ts16, IIALU>;
def SEQGES32  : SeqSOp<  SEQGE, Ts32, IIALU>;
def SEQGES64  : SeqSOp<  SEQGE, Ts64, IIALU>;

def SEQGEU8   : SeqSOp<  SEQGE, Tu8,  IIALU>;
def SEQGEU16  : SeqSOp<  SEQGE, Tu16, IIALU>;
def SEQGEU32  : SeqSOp<  SEQGE, Tu32, IIALU>;
def SEQGEU64  : SeqSOp<  SEQGE, Tu64, IIALU>;


// One-trip variants (seqotXXX) of the sequence operation.
def SEQOTLT     : GenericOp<"seqotlt">;
def SEQOTLTS8   : SeqSOp<  SEQOTLT, Ts8,  IIALU>;
def SEQOTLTS16  : SeqSOp<  SEQOTLT, Ts16, IIALU>;
def SEQOTLTS32  : SeqSOp<  SEQOTLT, Ts32, IIALU>;
def SEQOTLTS64  : SeqSOp<  SEQOTLT, Ts64, IIALU>;

def SEQOTLTU8   : SeqSOp<  SEQOTLT, Tu8,  IIALU>;
def SEQOTLTU16  : SeqSOp<  SEQOTLT, Tu16, IIALU>;
def SEQOTLTU32  : SeqSOp<  SEQOTLT, Tu32, IIALU>;
def SEQOTLTU64  : SeqSOp<  SEQOTLT, Tu64, IIALU>;

def SEQOTLE     : GenericOp<"seqotle">;
def SEQOTLES8   : SeqSOp<  SEQOTLE, Ts8,  IIALU>;
def SEQOTLES16  : SeqSOp<  SEQOTLE, Ts16, IIALU>;
def SEQOTLES32  : SeqSOp<  SEQOTLE, Ts32, IIALU>;
def SEQOTLES64  : SeqSOp<  SEQOTLE, Ts64, IIALU>;

def SEQOTLEU8   : SeqSOp<  SEQOTLE, Tu8,  IIALU>;
def SEQOTLEU16  : SeqSOp<  SEQOTLE, Tu16, IIALU>;
def SEQOTLEU32  : SeqSOp<  SEQOTLE, Tu32, IIALU>;
def SEQOTLEU64  : SeqSOp<  SEQOTLE, Tu64, IIALU>;

def SEQOTNE     : GenericOp<"seqotne">;
def SEQOTNE8    : SeqSOp<  SEQOTNE, Ti8,  IIALU>;
def SEQOTNE16   : SeqSOp<  SEQOTNE, Ti16, IIALU>;
def SEQOTNE32   : SeqSOp<  SEQOTNE, Ti32, IIALU>;
def SEQOTNE64   : SeqSOp<  SEQOTNE, Ti64, IIALU>;

def SEQOTGT     : GenericOp<"seqotgt">;
def SEQOTGTS8   : SeqSOp<  SEQOTGT, Ts8,  IIALU>;
def SEQOTGTS16  : SeqSOp<  SEQOTGT, Ts16, IIALU>;
def SEQOTGTS32  : SeqSOp<  SEQOTGT, Ts32, IIALU>;
def SEQOTGTS64  : SeqSOp<  SEQOTGT, Ts64, IIALU>;

def SEQOTGTU8   : SeqSOp<  SEQOTGT, Tu8,  IIALU>;
def SEQOTGTU16  : SeqSOp<  SEQOTGT, Tu16, IIALU>;
def SEQOTGTU32  : SeqSOp<  SEQOTGT, Tu32, IIALU>;
def SEQOTGTU64  : SeqSOp<  SEQOTGT, Tu64, IIALU>;

def SEQOTGE     : GenericOp<"seqotge">;
def SEQOTGES8   : SeqSOp<  SEQOTGE, Ts8,  IIALU>;
def SEQOTGES16  : SeqSOp<  SEQOTGE, Ts16, IIALU>;
def SEQOTGES32  : SeqSOp<  SEQOTGE, Ts32, IIALU>;
def SEQOTGES64  : SeqSOp<  SEQOTGE, Ts64, IIALU>;

def SEQOTGEU8   : SeqSOp<  SEQOTGE, Tu8,  IIALU>;
def SEQOTGEU16  : SeqSOp<  SEQOTGE, Tu16, IIALU>;
def SEQOTGEU32  : SeqSOp<  SEQOTGE, Tu32, IIALU>;
def SEQOTGEU64  : SeqSOp<  SEQOTGE, Tu64, IIALU>;

def REPEAT    : GenericOp<"repeat">;
def REPEAT1   : RepeatOp<REPEAT, Ti1,  IIALU>;
def REPEAT8   : RepeatOp<REPEAT, Ti8,  IIALU>;
def REPEAT16  : RepeatOp<REPEAT, Ti16, IIALU>;
def REPEAT32  : RepeatOp<REPEAT, Ti32, IIALU>;
def REPEAT64  : RepeatOp<REPEAT, Ti64, IIALU>;

def STRIDE    : GenericOp<"stride">;
def STRIDE8   : StrideOp<STRIDE, Ti8,  IIALU>;
def STRIDE16  : StrideOp<STRIDE, Ti16, IIALU>;
def STRIDE32  : StrideOp<STRIDE, Ti32, IIALU>;
def STRIDE64  : StrideOp<STRIDE, Ti64, IIALU>;

// FMA reductions.

def FMSREDA   : GenericOp<"fmsreda">;
def FMSREDAF32: FMSReduceOp<FMSREDA, Tf32, IIALU>;
def FMSREDAF64: FMSReduceOp<FMSREDA, Tf64, IIALU>;

// Standard sequenced reductions.
def SREDSUB   : GenericOp<"sredsub">;
def SREDSUB8  : SReduceOp<SREDSUB,  Ti8,  IIALU>;
def SREDSUB16 : SReduceOp<SREDSUB,  Ti16, IIALU>;
def SREDSUB32 : SReduceOp<SREDSUB,  Ti32, IIALU>;
def SREDSUB64 : SReduceOp<SREDSUB,  Ti64, IIALU>;
def SREDSUBF32: SReduceOpR<SREDSUB, Tf32, IIALU>;
def SREDSUBF64: SReduceOpR<SREDSUB, Tf64, IIALU>;

def SREDADD   : GenericOp<"sredadd">;
def SREDADD8  : SReduceOp<SREDADD,  Ti8,  IIALU>;
def SREDADD16 : SReduceOp<SREDADD,  Ti16, IIALU>;
def SREDADD32 : SReduceOp<SREDADD,  Ti32, IIALU>;
def SREDADD64 : SReduceOp<SREDADD,  Ti64, IIALU>;
def SREDADDF32: SReduceOpR<SREDADD, Tf32, IIALU>;
def SREDADDF64: SReduceOpR<SREDADD, Tf64, IIALU>;

def SREDMUL   : GenericOp<"sredmul">;
def SREDMUL8  : SReduceOp<SREDMUL,  Ti8,  IIALU>;
def SREDMUL16 : SReduceOp<SREDMUL,  Ti16, IIALU>;
def SREDMUL32 : SReduceOp<SREDMUL,  Ti32, IIALU>;
def SREDMUL64 : SReduceOp<SREDMUL,  Ti64, IIALU>;
def SREDMULF32: SReduceOpR<SREDMUL, Tf32, IIALU>;
def SREDMULF64: SReduceOpR<SREDMUL, Tf64, IIALU>;

def SREDAND   : GenericOp<"sredand">;
def SREDAND8  : SReduceOp<SREDAND,  Ti8,  IIALU>;
def SREDAND16 : SReduceOp<SREDAND,  Ti16, IIALU>;
def SREDAND32 : SReduceOp<SREDAND,  Ti32, IIALU>;
def SREDAND64 : SReduceOp<SREDAND,  Ti64, IIALU>;

def SREDOR    : GenericOp<"sredor">;
def SREDOR8   : SReduceOp<SREDOR,   Ti8,  IIALU>;
def SREDOR16  : SReduceOp<SREDOR,   Ti16, IIALU>;
def SREDOR32  : SReduceOp<SREDOR,   Ti32, IIALU>;
def SREDOR64  : SReduceOp<SREDOR,   Ti64, IIALU>;

def SREDXOR   : GenericOp<"sredxor">;
def SREDXOR8  : SReduceOp<SREDXOR,  Ti8,  IIALU>;
def SREDXOR16 : SReduceOp<SREDXOR,  Ti16, IIALU>;
def SREDXOR32 : SReduceOp<SREDXOR,  Ti32, IIALU>;
def SREDXOR64 : SReduceOp<SREDXOR,  Ti64, IIALU>;


// TBD(jsukha): This info probably needs predicates
// Also, the input can probably be an I0 channel.
def ONEND : FMTGEN<  // TODO: something to reflect state
  (outs I0:$dst),
  (ins I1:$ctrl, I0:$in),
  "onend\t$dst, $ctrl, $in",
  []> { let Itinerary = IIVir; }

// Not clear that "hasSideEffects" is a good description of static initialization...
class Init<CSAOpInfo t, list<Predicate> preds, InstrItinClass itin> :
  PseudoInstCSA<
      (outs t.RC:$dst),
      (ins t.L:$imm),
      ".curr\t$dst;\t.value $imm;\t.avail 0",
      [], preds, itin> { let hasSideEffects = 1; }

let isCodeGenOnly=1 in {
  def INIT0  : Init<Ti0,  [HasI0],  IIVir>;
  def INIT1  : Init<Ti1,  [HasI1],  IIVir>;
  def INIT8  : Init<Ti8,  [HasI8],  IIVir>;
  def INIT16 : Init<Ti16, [HasI16], IIVir>;
  def INIT32 : Init<Ti32, [HasI32], IIVir>;
  def INIT64 : Init<Ti64, [HasI64], IIVir>;
}

// Memory references
defm LD1      : LdOp<    "ld8",   Ti1>;
defm LD8      : LdOp<    "ld8",   Ti8>;
defm LD16     : LdOp<    "ld16",  Ti16>;
defm LD16f    : LdOp<    "ld16",  Tf16>;
defm LD32     : LdOp<    "ld32",  Ti32>;
defm LD32f    : LdOp<    "ld32",  Tf32>;
defm LD64     : LdOp<    "ld64",  Ti64>;
defm LD64f    : LdOp<    "ld64",  Tf64>;

defm OLD1     : OLdOp<   "ld8",   Ti1>;
defm OLD8     : OLdOp<   "ld8",   Ti8>;
defm OLD16    : OLdOp<   "ld16",  Ti16>;
defm OLD16f   : OLdOp<   "ld16",  Tf16>;
defm OLD32    : OLdOp<   "ld32",  Ti32>;
defm OLD32f   : OLdOp<   "ld32",  Tf32>;
defm OLD64    : OLdOp<   "ld64",  Ti64>;
defm OLD64f   : OLdOp<   "ld64",  Tf64>;

defm ST1      : StOp<    "st8",   Ti1>;
defm ST8      : StOp<    "st8",   Ti8>;
defm ST16     : StOp<    "st16",  Ti16>;
defm ST16f    : StOp<    "st16",  Tf16>;
defm ST32     : StOp<    "st32",  Ti32>;
defm ST32f    : StOp<    "st32",  Tf32>;
defm ST64     : StOp<    "st64",  Ti64>;
defm ST64f    : StOp<    "st64",  Tf64>;

defm OST1     : OStOp<   "st8",   Ti1>;
defm OST8     : OStOp<   "st8",   Ti8>;
defm OST16    : OStOp<   "st16",  Ti16>;
defm OST16f   : OStOp<   "st16",  Tf16>;
defm OST32    : OStOp<   "st32",  Ti32>;
defm OST32f   : OStOp<   "st32",  Tf32>;
defm OST64    : OStOp<   "st64",  Ti64>;
defm OST64f   : OStOp<   "st64",  Tf64>;

// Extended multiplication
def MULLOHI   : GenericOp<"mullohi">;
def MULLOHIS8  : MullohiOp<MULLOHI, Ts8,  IIMulI8>;
def MULLOHIS16 : MullohiOp<MULLOHI, Ts16, IIMulI16>;
def MULLOHIS32 : MullohiOp<MULLOHI, Ts32, IIMulI32>;
def MULLOHIS64 : MullohiOp<MULLOHI, Ts64, IIMulI64>;
def MULLOHIU8  : MullohiOp<MULLOHI, Tu8,  IIMulI8>;
def MULLOHIU16 : MullohiOp<MULLOHI, Tu16, IIMulI16>;
def MULLOHIU32 : MullohiOp<MULLOHI, Tu32, IIMulI32>;
def MULLOHIU64 : MullohiOp<MULLOHI, Tu64, IIMulI64>;

// Atomic operations
def ATMAND    : GenericOp<"atmand">;
def ATMAND8   : AtomicOp<ATMAND,  atomic_load_and_8,  Ti8,  IIATM>;
def ATMAND16  : AtomicOp<ATMAND,  atomic_load_and_16, Ti16, IIATM>;
def ATMAND32  : AtomicOp<ATMAND,  atomic_load_and_32, Ti32, IIATM>;
def ATMAND64  : AtomicOp<ATMAND,  atomic_load_and_64, Ti64, IIATM>;
def ATMADD    : GenericOp<"atmadd">;
def ATMADD8   : AtomicOp<ATMADD,  atomic_load_add_8,  Ti8,  IIATM>;
def ATMADD16  : AtomicOp<ATMADD,  atomic_load_add_16, Ti16, IIATM>;
def ATMADD32  : AtomicOp<ATMADD,  atomic_load_add_32, Ti32, IIATM>;
def ATMADD64  : AtomicOp<ATMADD,  atomic_load_add_64, Ti64, IIATM>;
def ATMMIN    : GenericOp<"atmmin">;
def ATMMIN8   : AtomicOp<ATMMIN,  atomic_load_min_8,  Ti8,  IIATM>;
def ATMMIN16  : AtomicOp<ATMMIN,  atomic_load_min_16, Ti16, IIATM>;
def ATMMIN32  : AtomicOp<ATMMIN,  atomic_load_min_32, Ti32, IIATM>;
def ATMMIN64  : AtomicOp<ATMMIN,  atomic_load_min_64, Ti64, IIATM>;
def ATMMAX    : GenericOp<"atmmax">;
def ATMMAX8   : AtomicOp<ATMMAX,  atomic_load_max_8,  Ti8,  IIATM>;
def ATMMAX16  : AtomicOp<ATMMAX,  atomic_load_max_16, Ti16, IIATM>;
def ATMMAX32  : AtomicOp<ATMMAX,  atomic_load_max_32, Ti32, IIATM>;
def ATMMAX64  : AtomicOp<ATMMAX,  atomic_load_max_64, Ti64, IIATM>;
def ATMOR     : GenericOp<"atmor">;
def ATMOR8    : AtomicOp<ATMOR,   atomic_load_or_8,   Ti8,  IIATM>;
def ATMOR16   : AtomicOp<ATMOR,   atomic_load_or_16,  Ti16, IIATM>;
def ATMOR32   : AtomicOp<ATMOR,   atomic_load_or_32,  Ti32, IIATM>;
def ATMOR64   : AtomicOp<ATMOR,   atomic_load_or_64,  Ti64, IIATM>;
def ATMXOR    : GenericOp<"atmxor">;
def ATMXOR8   : AtomicOp<ATMXOR,  atomic_load_xor_8,  Ti8,  IIATM>;
def ATMXOR16  : AtomicOp<ATMXOR,  atomic_load_xor_16, Ti16, IIATM>;
def ATMXOR32  : AtomicOp<ATMXOR,  atomic_load_xor_32, Ti32, IIATM>;
def ATMXOR64  : AtomicOp<ATMXOR,  atomic_load_xor_64, Ti64, IIATM>;
def ATMXCHG   : GenericOp<"atmxchg">;
def ATMXCHG8  : AtomicOp<ATMXCHG, atomic_swap_8,      Ti8,  IIATM>;
def ATMXCHG16 : AtomicOp<ATMXCHG, atomic_swap_16,     Ti16, IIATM>;
def ATMXCHG32 : AtomicOp<ATMXCHG, atomic_swap_32,     Ti32, IIATM>;
def ATMXCHG64 : AtomicOp<ATMXCHG, atomic_swap_64,     Ti64, IIATM>;
def ATMCMPXCHG   : GenericOp<"atmcmpxchg">;
def ATMCMPXCHG8  : AtomicOp2<ATMCMPXCHG, atomic_cmp_swap_8,  Ti8,  IIATM>;
def ATMCMPXCHG16 : AtomicOp2<ATMCMPXCHG, atomic_cmp_swap_16, Ti16, IIATM>;
def ATMCMPXCHG32 : AtomicOp2<ATMCMPXCHG, atomic_cmp_swap_32, Ti32, IIATM>;
def ATMCMPXCHG64 : AtomicOp2<ATMCMPXCHG, atomic_cmp_swap_64, Ti64, IIATM>;

def OATMAND8   : OAtomicOp<ATMAND,  Ti8,  IIATM>;
def OATMAND16  : OAtomicOp<ATMAND,  Ti16, IIATM>;
def OATMAND32  : OAtomicOp<ATMAND,  Ti32, IIATM>;
def OATMAND64  : OAtomicOp<ATMAND,  Ti64, IIATM>;
def OATMADD8   : OAtomicOp<ATMADD,  Ti8,  IIATM>;
def OATMADD16  : OAtomicOp<ATMADD,  Ti16, IIATM>;
def OATMADD32  : OAtomicOp<ATMADD,  Ti32, IIATM>;
def OATMADD64  : OAtomicOp<ATMADD,  Ti64, IIATM>;
def OATMMIN8   : OAtomicOp<ATMMIN,  Ti8,  IIATM>;
def OATMMIN16  : OAtomicOp<ATMMIN,  Ti16, IIATM>;
def OATMMIN32  : OAtomicOp<ATMMIN,  Ti32, IIATM>;
def OATMMIN64  : OAtomicOp<ATMMIN,  Ti64, IIATM>;
def OATMMAX8   : OAtomicOp<ATMMAX,  Ti8,  IIATM>;
def OATMMAX16  : OAtomicOp<ATMMAX,  Ti16, IIATM>;
def OATMMAX32  : OAtomicOp<ATMMAX,  Ti32, IIATM>;
def OATMMAX64  : OAtomicOp<ATMMAX,  Ti64, IIATM>;
def OATMOR8    : OAtomicOp<ATMOR,   Ti8,  IIATM>;
def OATMOR16   : OAtomicOp<ATMOR,   Ti16, IIATM>;
def OATMOR32   : OAtomicOp<ATMOR,   Ti32, IIATM>;
def OATMOR64   : OAtomicOp<ATMOR,   Ti64, IIATM>;
def OATMXOR8   : OAtomicOp<ATMXOR,  Ti8,  IIATM>;
def OATMXOR16  : OAtomicOp<ATMXOR,  Ti16, IIATM>;
def OATMXOR32  : OAtomicOp<ATMXOR,  Ti32, IIATM>;
def OATMXOR64  : OAtomicOp<ATMXOR,  Ti64, IIATM>;
def OATMXCHG8  : OAtomicOp<ATMXCHG, Ti8,  IIATM>;
def OATMXCHG16 : OAtomicOp<ATMXCHG, Ti16, IIATM>;
def OATMXCHG32 : OAtomicOp<ATMXCHG, Ti32, IIATM>;
def OATMXCHG64 : OAtomicOp<ATMXCHG, Ti64, IIATM>;
def OATMCMPXCHG8  : OAtomicOp2<ATMCMPXCHG, Ti8,  IIATM>;
def OATMCMPXCHG16 : OAtomicOp2<ATMCMPXCHG, Ti16, IIATM>;
def OATMCMPXCHG32 : OAtomicOp2<ATMCMPXCHG, Ti32, IIATM>;
def OATMCMPXCHG64 : OAtomicOp2<ATMCMPXCHG, Ti64, IIATM>;

// For now, LLVM cannot match/select these because the IR's atomics only deal
// with integers.
def ATMADDF32 : AtomicOpR<ATMADD, null_frag, Tf32, IIATM>;
def ATMADDF64 : AtomicOpR<ATMADD, null_frag, Tf64, IIATM>;
def ATMMINF32 : AtomicOp<ATMMIN,  null_frag, Tf32, IIATM>;
def ATMMINF64 : AtomicOp<ATMMIN,  null_frag, Tf64, IIATM>;
def ATMMAXF32 : AtomicOp<ATMMAX,  null_frag, Tf32, IIATM>;
def ATMMAXF64 : AtomicOp<ATMMAX,  null_frag, Tf64, IIATM>;

def OATMADDF32 : OAtomicOpR<ATMADD, Tf32, IIATM>;
def OATMADDF64 : OAtomicOpR<ATMADD, Tf64, IIATM>;
def OATMMINF32 : OAtomicOp<ATMMIN,  Tf32, IIATM>;
def OATMMINF64 : OAtomicOp<ATMMIN,  Tf64, IIATM>;
def OATMMAXF32 : OAtomicOp<ATMMAX,  Tf32, IIATM>;
def OATMMAXF64 : OAtomicOp<ATMMAX,  Tf64, IIATM>;

// Unit - type only
def UNIT : PseudoInstCSA<
    (outs),
    (ins UnitOpnd:$immType),
    ".unit\t$immType",
    []>;

// Unit type + index of unit in type only
def UNITI : PseudoInstCSA<
    (outs),
    (ins UnitOpnd:$immType, Operand<i64>:$idx),
    ".unit\t$immType, $idx",
    []>;

// Unit type / allocated - includes coordinate indicies
def UNITA : PseudoInstCSA<
    (outs),
    (ins UnitOpnd:$immType, Operand<i64>:$idx1, Operand<i64>:$idx2),
    ".unit\t$immType, $idx1, $idx2",
    []>;

// TBD: This is a generic csa directive taking an integer constant argument.
// It will not be used in the final compiler, but is a convenient hook for
// experimentation.
def CSA_DIRECTIVE : PseudoInstCSA<
    (outs),
    (ins I32:$md),
    ".csa_directive\t$md",
    [ (int_csa_directive (i32 imm:$md)) ]>;

// This pseudo-op is appears the start of a loop loop that starts with a
// parallel directive (__builtin_csa_parallel_loop() or #pragma omp parallel
// for).
def CSA_PARALLEL_LOOP : PseudoInstCSA<
    (outs),
    (ins),
    "# .csa_parallel_loop",
    [ (int_csa_parallel_loop) ]>;

// Memory dependency sink at the end of each basic block.
def CSA_PARALLEL_MEMDEP :  PseudoInstCSA<
    (outs I0:$out),
    (ins  I0:$in),
    "mov0\t$out, $in  # .csa_parallel_memdep\t$out, $in",
    []>;

// Arbitrary immediate support
def : Pat<(i1 imm:$imm),    (MOV1 imm:$imm)>;
def : Pat<(i8 imm:$imm),    (MOV8 imm:$imm)>;
def : Pat<(i16 imm:$imm),   (MOV16 imm:$imm)>;
def : Pat<(f16 fpimm:$imm), (MOV16 fpimm:$imm)>;
def : Pat<(i32 imm:$imm),   (MOV32 imm:$imm)>;
def : Pat<(f32 fpimm:$imm), (MOV32 fpimm:$imm)>;
def : Pat<(i64 imm:$imm),   (MOV64 imm:$imm)>;
def : Pat<(f64 fpimm:$imm), (MOV64 fpimm:$imm)>;

// sext/zext
def : Pat<(i64 (sext i32:$op1)),  (SEXT64 (COPY_TO_REGCLASS $op1,I64), 32)>;
def : Pat<(i64 (sext i16:$op1)),  (SEXT64 (COPY_TO_REGCLASS $op1,I64), 16)>;
def : Pat<(i64 (sext  i8:$op1)),  (SEXT64 (COPY_TO_REGCLASS $op1,I64),  8)>;
def : Pat<(i64 (sext  i1:$op1)),  (SEXT64 (COPY_TO_REGCLASS $op1,I64),  1)>;

def : Pat<(i32 (sext i16:$op1)),  (SEXT32 (COPY_TO_REGCLASS $op1,I64), 16)>;
def : Pat<(i32 (sext  i8:$op1)),  (SEXT32 (COPY_TO_REGCLASS $op1,I64),  8)>;
def : Pat<(i32 (sext  i1:$op1)),  (SEXT32 (COPY_TO_REGCLASS $op1,I64),  1)>;

def : Pat<(i16 (sext  i8:$op1)),  (SEXT16 (COPY_TO_REGCLASS $op1,I64),  8)>;
def : Pat<(i16 (sext  i1:$op1)),  (SEXT16 (COPY_TO_REGCLASS $op1,I64),  1)>;

def : Pat<(i8  (sext  i1:$op1)),  (SEXT8  (COPY_TO_REGCLASS $op1,I64),  1)>;

// sext_inreg are the same
def : Pat<(i64 (sext_inreg i64:$op1, i32)),  (SEXT64 $op1, 32)>;
def : Pat<(i64 (sext_inreg i64:$op1, i16)),  (SEXT64 $op1, 16)>;
def : Pat<(i64 (sext_inreg i64:$op1,  i8)),  (SEXT64 $op1,  8)>;
def : Pat<(i64 (sext_inreg i64:$op1,  i1)),  (SEXT64 $op1,  1)>;

def : Pat<(i32 (sext_inreg i32:$op1, i16)),  (SEXT32 $op1, 16)>;
def : Pat<(i32 (sext_inreg i32:$op1,  i8)),  (SEXT32 $op1,  8)>;
def : Pat<(i32 (sext_inreg i32:$op1,  i1)),  (SEXT32 $op1,  1)>;

def : Pat<(i16 (sext_inreg i16:$op1,  i8)),  (SEXT16 $op1,  8)>;
def : Pat<(i16 (sext_inreg i16:$op1,  i1)),  (SEXT16 $op1,  1)>;

def : Pat<(i8  (sext_inreg  i8:$op1,  i1)),  (SEXT8  $op1,  1)>;

// zext patterns
// (Is the copy even necessary?  Or can we just return the value?)
// (Or - do we need an explicit mask.  If the incoming value is in range,
// it shouldn't need to be masked...)

def : Pat<(i64 (zext i32:$op1)),       (COPY_TO_REGCLASS $op1, I64)>;
def : Pat<(i64 (zext i16:$op1)),       (COPY_TO_REGCLASS $op1, I64)>;
def : Pat<(i64 (zext  i8:$op1)),       (COPY_TO_REGCLASS $op1, I64)>;
def : Pat<(i64 (zext  i1:$op1)),       (COPY_TO_REGCLASS $op1, I64)>;

def : Pat<(i32 (zext i16:$op1)),       (COPY_TO_REGCLASS $op1, I32)>;
def : Pat<(i32 (zext  i8:$op1)),       (COPY_TO_REGCLASS $op1, I32)>;
def : Pat<(i32 (zext  i1:$op1)),       (COPY_TO_REGCLASS $op1, I32)>;

def : Pat<(i16 (zext  i8:$op1)),       (COPY_TO_REGCLASS $op1, I16)>;
def : Pat<(i16 (zext  i1:$op1)),       (COPY_TO_REGCLASS $op1, I16)>;

def : Pat<(i8  (zext  i1:$op1)),       (COPY_TO_REGCLASS $op1,  I8)>;

// anyext are treated as zext
def : Pat<(i64 (anyext i32:$op1)),     (COPY_TO_REGCLASS $op1, I64)>;
def : Pat<(i64 (anyext i16:$op1)),     (COPY_TO_REGCLASS $op1, I64)>;
def : Pat<(i64 (anyext  i8:$op1)),     (COPY_TO_REGCLASS $op1, I64)>;
def : Pat<(i64 (anyext  i1:$op1)),     (COPY_TO_REGCLASS $op1, I64)>;

def : Pat<(i32 (anyext i16:$op1)),     (COPY_TO_REGCLASS $op1, I32)>;
def : Pat<(i32 (anyext  i8:$op1)),     (COPY_TO_REGCLASS $op1, I32)>;
def : Pat<(i32 (anyext  i1:$op1)),     (COPY_TO_REGCLASS $op1, I32)>;

def : Pat<(i16 (anyext  i8:$op1)),     (COPY_TO_REGCLASS $op1, I16)>;
def : Pat<(i16 (anyext  i1:$op1)),     (COPY_TO_REGCLASS $op1, I16)>;

def : Pat<(i8  (anyext  i1:$op1)),     (COPY_TO_REGCLASS $op1,  I8)>;

// Truncate
def : Pat<(i32 (trunc i64:$op1)),      (COPY_TO_REGCLASS $op1, I32)>;
def : Pat<(i16 (trunc i64:$op1)),      (COPY_TO_REGCLASS $op1, I16)>;
def : Pat<(i8  (trunc i64:$op1)),      (COPY_TO_REGCLASS $op1,  I8)>;
def : Pat<(i1  (trunc i64:$op1)),      (COPY_TO_REGCLASS $op1,  I1)>;

def : Pat<(i16 (trunc i32:$op1)),      (COPY_TO_REGCLASS $op1, I16)>;
def : Pat<(i8  (trunc i32:$op1)),      (COPY_TO_REGCLASS $op1,  I8)>;
def : Pat<(i1  (trunc i32:$op1)),      (COPY_TO_REGCLASS $op1,  I1)>;

def : Pat<(i8  (trunc i16:$op1)),      (COPY_TO_REGCLASS $op1,  I8)>;
def : Pat<(i1  (trunc i16:$op1)),      (COPY_TO_REGCLASS $op1,  I1)>;

def : Pat<(i1  (trunc  i8:$op1)),      (COPY_TO_REGCLASS $op1,  I1)>;

// bitconvert (shows up in exp)
def : Pat<(i64 (bitconvert f64:$op1)), (COPY_TO_REGCLASS $op1, I64)>;
def : Pat<(f64 (bitconvert i64:$op1)), (COPY_TO_REGCLASS $op1, I64)>;

def : Pat<(i32 (bitconvert f32:$op1)), (COPY_TO_REGCLASS $op1, I32)>;
def : Pat<(f32 (bitconvert i32:$op1)), (COPY_TO_REGCLASS $op1, I32)>;

def : Pat<(i16 (bitconvert f16:$op1)), (COPY_TO_REGCLASS $op1, I16)>;
def : Pat<(f16 (bitconvert i16:$op1)), (COPY_TO_REGCLASS $op1, I16)>;

// fcopysign - use the sign bit from the sign opnd, and everything else from
// the other
/*
  After having finally found a solution, for some reason this causes a tablegen
  crash when doing builds on Windows...  Disabled for now...
def : Pat<(fcopysign f32:$other, f32:$sign),
          (BSEL32 0x80000000,$other,$sign)>;
def : Pat<(fcopysign f64:$other, f64:$sign),
          (BSEL64 0x8000000000000000,$other,$sign)>;
*/

// Eventually
//include "CSAIntrinsics.td"
