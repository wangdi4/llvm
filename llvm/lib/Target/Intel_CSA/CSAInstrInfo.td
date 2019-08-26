//===- CSAInstrInfo.td - CSA Instruction defs -----------------*- tblgen-*-===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file describes the CSA instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

include "CSAInstrFormats.td"

// Possible TODO:
// - Possible to merge BinOp and CmpOp (something that can take both a PatFrag
//   and an SDNode?)
// - Use "foreach" or some other mechanism to deal with 8/16/32/64 redundancy
//   in patterns? (but will likely eventually need opcode bitpatterns, so
//   that may not be feasible/desirable long term)
// - Why are so many operands in CSAGenInstrInfo.inc unknown?

//===----------------------------------------------------------------------===//
// CSA Instruction Predicate Definitions
//===----------------------------------------------------------------------===//

def IsOrdered    : Predicate<"Subtarget->isOrdered()">;
def HasI0        : Predicate<"Subtarget->hasI0()">;
def HasI1        : Predicate<"Subtarget->hasI1()">;
def HasI8        : Predicate<"Subtarget->hasI8()">;
def HasI16       : Predicate<"Subtarget->hasI16()">;
def HasI32       : Predicate<"Subtarget->hasI32()">;
def HasI64       : Predicate<"Subtarget->hasI64()">;
def HasF16       : Predicate<"Subtarget->hasF16()">;
def HasF32       : Predicate<"Subtarget->hasF32()">;
def HasF64       : Predicate<"Subtarget->hasF64()">;
def HasSextL     : Predicate<"Subtarget->hasSextL()">;
def HasDispl     : Predicate<"Subtarget->hasDispl()">;
def HasIndex     : Predicate<"Subtarget->hasIndex()">;
def HasShAdd     : Predicate<"Subtarget->hasShAdd()">;
def HasBitOp     : Predicate<"Subtarget->hasBitOp()">;
def HasIDiv      : Predicate<"Subtarget->hasIDiv()">;
def HasFDiv      : Predicate<"Subtarget->hasFDiv()">;
def HasFMA       : Predicate<"Subtarget->hasFMA()">;
def HasRcpA      : Predicate<"Subtarget->hasRcpA()">;
def HasRSqrtA    : Predicate<"Subtarget->hasRSqrtA()">;
def HasSqrt      : Predicate<"Subtarget->hasSqrt()">;
def HasMath0     : Predicate<"Subtarget->hasMath0()">;
def HasRMWAtomic : Predicate<"Subtarget->hasRMWAtomic()">;
def IsSequential : Predicate<"Subtarget->isSequential()">;
def IsDataflow   : Predicate<"!Subtarget->isSequential()">;

//===----------------------------------------------------------------------===//
// CSA Operand Definitions.
//===----------------------------------------------------------------------===//

def brtarget     : Operand<OtherVT>;
def calltarget   : Operand<i64>;

def UnitOpnd : Operand<i64> {
  let PrintMethod = "printUnitOperand";
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

// This version is meant to be used for operations that are agnostic to the
// actual integer/floating/vector form. It relies on a hack in the typechecking:
// if you merge {iPTR} and {i32}, you get {i32}... and you get {f32} if you try
// to merge {f32} with {iPTR}.
class RegImmOperandUntyped<ValueType vt> : Operand<iPTR>, ComplexPattern<vt,1,"SelectRegImm", [imm, fpimm]> {
    let OperandType = "OPERAND_REG_IMM";
    // We need to give TableGen some clues (along with some custom function in
    // CSAAsmParser) to deal with these particularly flexible operands.
    let ParserMatchClass = RegImmAsmOperand;
  }

// Rounding mode operands. These are literal integers underneath, but are
// printed and parsed as their symbolic names. The default numerically matches
// the simulator's default value in csa.h. (This is CSA::ROUND_NEAREST.)
// Note that RMODE operands are optional in the sense that the compiler doesn't
// need to add them to MachineInstrs.
def RmodeAsmOperand : AsmOperandClass {
  let Name = "RMode";
  let IsOptional = 1;
}
def RMODE : OperandWithDefaultOps <i64, (ops (i64 0))>
{
  let PrintMethod = "printRModeOperand";
  let ParserMatchClass = RmodeAsmOperand;
}

// A 0-defaulting literal i1 operand. This is used by the various "any"
// operators to choose between fixed left-to-right and least-recently-used
// priority ordering. The default tends to be 0, for the fixed ordering.
def PrioOrderAsmOperand : AsmOperandClass {
  let Name = "PrioOrder";
  let IsOptional = 1;
}
def PrioOrderOperand : OperandWithDefaultOps<i1, (ops (i1 0))> {
  let ParserMatchClass = PrioOrderAsmOperand;
  let PrintMethod = "printPrioOrderOperand";
}

// Operands for floating-point comparison operators
foreach Kind = [ "FPOrdered", "FPSignaling" ] in {
  def Kind ## AsmOperand : AsmOperandClass {
    let Name = Kind;
    let IsOptional = 1;
  }
  def Kind ## Operand : OperandWithDefaultOps<i1, (ops (i1 0))> {
    let ParserMatchClass = !cast<AsmOperandClass>(Kind ## AsmOperand);
    let PrintMethod = !strconcat("print", Kind, "Operand");
  }
}

// Memory level operands. For simplicity these use numeric values that match
// the intrinsic rather than the simulator (so the default value is T0, not NTA).
def MemLvlAsmOperand : AsmOperandClass {
  let Name = "MemLvl";
  let IsOptional = 1;
}
let OperandType = "OPERAND_IMMEDIATE",
  PrintMethod = "printMemLvlOperand",
  ParserMatchClass = MemLvlAsmOperand
in {
  def MEMLVL : OperandWithDefaultOps<i32, (ops (i32 3))>;
  def MEMLVLExplicit : Operand<i32>;
}

// RCL (Reg/Chan/Literal) is the same as used in the simulator
def RCLi0  : RegImmOperand<i1>;
def RCLi1  : RegImmOperand<i1>;
def RCLi8  : RegImmOperand<i8>;
def RCLi16 : RegImmOperand<i16>;
def RCLi32 : RegImmOperand<i32>;
def RCLf32 : RegImmOperand<f32>;
def RCLi64 : RegImmOperand<i64>;
def RCLf64 : RegImmOperand<f64>;
def RCLf32x2 : RegImmOperand<v2f32>;
def RCL32  : RegImmOperandUntyped<i32>;
def RCL64  : RegImmOperandUntyped<i64>;

// Memory operands
class Addr<int numArgs, string funcName, dag opInfo> :
  Operand<i64>, ComplexPattern<i64, numArgs, funcName, [], [SDNPWantParent]> {
      let MIOperandInfo = opInfo;
      let OperandType = "OPERAND_MEMORY";
    }

let PrintMethod = "printMemOperand" in {
def ADDR_RX : Addr<2, "SelectAddrRegIdx", (ops I64:$base, I64:$offset)>;
def ADDR_RI : Addr<2, "SelectAddrRegImm", (ops I64:$base, i64imm:$offset)>;
}
def ADDR_R : RegImmOperand<i64> {
  let OperandType = "OPERAND_MEMORY";
}

// Memory ordering operands
def OptionalRegAsmOperand : AsmOperandClass {
  let Name = "OptionalReg";
  let IsOptional = 1;
}
def MemOrdDef : Operand<i1> {
  let MIOperandInfo = (ops I0);
  let ParserMatchClass = OptionalRegAsmOperand;
}
def MemOrdUse : Operand<i1> {
  let MIOperandInfo = (ops I0);
  let ParserMatchClass = OptionalRegAsmOperand;
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
def Ti32 : CSAOpInfo<i32, 32,  "32", I32, RI32, CI32, RCLi32, i32imm, [HasI32]>;
def Ts32 : CSAOpInfo<i32, 32, "s32", I32, RI32, CI32, RCLi32, i32imm, [HasI32]>;
def Tu32 : CSAOpInfo<i32, 32, "u32", I32, RI32, CI32, RCLi32, i32imm, [HasI32]>;
def Tf32 : CSAOpInfo<f32, 32, "f32", I32, RI32, CI32, RCLf32, f32imm, [HasF32]>;
def Ti64 : CSAOpInfo<i64, 64,  "64", I64, RI64, CI64, RCLi64, i64imm, [HasI64]>;
def Ts64 : CSAOpInfo<i64, 64, "s64", I64, RI64, CI64, RCLi64, i64imm, [HasI64]>;
def Tu64 : CSAOpInfo<i64, 64, "u64", I64, RI64, CI64, RCLi64, i64imm, [HasI64]>;
def Tf64 : CSAOpInfo<f64, 64, "f64", I64, RI64, CI64, RCLf64, f64imm, [HasF64]>;
def Tf32x2 : CSAOpInfo<v2f32, 64, "f32x2", I64, RI64, CI64, RCLf32x2, i64imm, []>;
def T32  : CSAOpInfo<i32, 32,  "32", I32, RI32, CI32, RCL32, i32imm, [HasI32]>;
def T64  : CSAOpInfo<i64, 64,  "64", I64, RI64, CI64, RCL64, i64imm, [HasI64]>;


def flog : SDNode<"ISD::FLOG", SDTFPUnaryOp>;
def fexp : SDNode<"ISD::FEXP", SDTFPUnaryOp>;

//===----------------------------------------------------------------------===//
// CSA profiles and nodes
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// CSA Instructions.
//===----------------------------------------------------------------------===//

// These are target-independent nodes, but have target-specific formats.
def SDT_CSACall         : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_CSACallDF       : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_CSAReturn       : SDTypeProfile<0, -1, []>;
def SDT_CSACallSeqStart : SDCallSeqStart<[ SDTCisVT<0, i64>, SDTCisVT<1, i64> ]>;
def SDT_CSACallSeqEnd   : SDCallSeqEnd<[ SDTCisVT<0, i64>, SDTCisVT<1, i64> ]>;

def SDT_CSAEntryPseudo  : SDTypeProfile<1, 0, []>;
def SDT_CSAContinuePseudo : SDTypeProfile<1, 0, []>;
def SDT_CSAGetVal       : SDTypeProfile<1, 1, []>;
def SDT_CSAWrapper      : SDTypeProfile<1, 1, [SDTCisSameAs<0,1>,
                                               SDTCisPtrTy<0>]>;

def SDT_CSAMinMax       : SDTypeProfile<
  2, 2, [SDTCisSameAs<0, 1>, SDTCisSameAs<0, 2>, SDTCisVT<1, i1>]
>;
def SDT_CSASwizzle      : SDTypeProfile<
  1, 2, [SDTCisSameAs<0, 1>, SDTCisVT<2, i8>]>;

// Function entry points will be lowered to this SD node
// followed by a list of CSAGetVal Nodes.
// Output of this node is input to each of the CSAGetVal nodes
def CSAEntryPseudo : SDNode<"CSAISD::EntryPseudo", SDT_CSAEntryPseudo,
    [SDNPHasChain, SDNPSideEffect]>;

// Function call sites will be lowered to CSACallDF node followed by this SD node
// which is in turn followed by a list of CSAGetVal Nodes.
// Output of this node is input to each of the CSAGetVal nodes
def CSAContinuePseudo : SDNode<"CSAISD::ContinuePseudo", SDT_CSAContinuePseudo,
    [SDNPHasChain, SDNPSideEffect]>;

// This node is used as a way to hold multiple outputs
// for the CSA_ENTRY and CSA_CONTINUE instructions at the ISel Lowering stage
def CSAGetVal : SDNode<"CSAISD::GetVal", SDT_CSAGetVal,
    [SDNPHasChain, SDNPSideEffect]>;

def CSARet : SDNode<"CSAISD::Ret", SDT_CSAReturn,
    [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

// Call
def CSACall : SDNode<"CSAISD::Call", SDT_CSACall,
                      [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                      SDNPVariadic]>;

def CSACallDF : SDNode<"CSAISD::CallDF", SDT_CSACallDF,
                      [SDNPHasChain, SDNPVariadic, SDNPSideEffect]>;

def CSATailCall : SDNode<"CSAISD::TailCall", SDT_CSACall,
                      [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_CSACallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END",   SDT_CSACallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def CSAWrapper : SDNode<"CSAISD::Wrapper", SDT_CSAWrapper>;

def CSAMin  : SDNode<"CSAISD::Min",  SDT_CSAMinMax>;
def CSAUMin : SDNode<"CSAISD::UMin", SDT_CSAMinMax>;
def CSAMax  : SDNode<"CSAISD::Max",  SDT_CSAMinMax>;
def CSAUMax : SDNode<"CSAISD::UMax", SDT_CSAMinMax>;

def CSAAddSub : SDNode<"CSAISD::ADDSUB", SDTFPBinOp>;
def CSASubAdd : SDNode<"CSAISD::SUBADD", SDTFPBinOp>;
def CSAFMAddSub : SDNode<"CSAISD::FMADDSUB", SDTFPTernaryOp>;
def CSAFMSubAdd : SDNode<"CSAISD::FMSUBADD", SDTFPTernaryOp>;
def CSASwizzleOp : SDNode<"CSAISD::Swizzle", SDT_CSASwizzle>;

let Defs = [SP], Uses = [SP] in {
def ADJCALLSTACKDOWN : PseudoInstCSA<(outs), (ins i64imm:$amt1, i64imm:$amt2),
                               "# ADJCALLSTACKDOWN $amt1, $amt2",
                               [(callseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKUP : PseudoInstCSA<(outs), (ins i64imm:$amt1, i64imm:$amt2),
                            "# ADJCALLSTACKUP $amt1",
                            [(callseq_end timm:$amt1, timm:$amt2)]>;
}

def TRAMPOLINE_START : PseudoInstCSA<
    (outs),
    (ins),
    ".trampoline_start",
    []>;

def TRAMPOLINE_END : PseudoInstCSA<
    (outs),
    (ins),
    ".trampoline_end",
    []>;

// Call directives
def CSA_ENTRY : PseudoInstCSA<
    (outs calltarget:$caller_cont_point, I64:$context, MemOrdDef:$mem_ord, variable_ops),
    (ins),
    "csa_entry",
    []>;

def CSA_ENTRYPSEUDO : PseudoInstCSA<
    (outs CI0:$link),
    (ins),
    "csa_entry_pseudo",
    [(set CI0:$link, (CSAEntryPseudo))],
    [IsDataflow]> {
      let hasSideEffects = 1;
    }

class CSAGetValOp<GenericOp gen, CSAOpInfo oi,
            InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  PseudoInstCSA<(outs oi.RC:$val),
    (ins CI0:$link),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$val, $link"),
    [(set oi.VT:$val, (CSAGetVal CI0:$link))],
    !listconcat(preds,oi.Preds), itin> {
      let GenOp = gen; let OpInfo = oi; let hasSideEffects = 1;
    }

def CSA_GETVAL         : GenericOp<"get_val">;
def CSA_GETVAL1        : CSAGetValOp<CSA_GETVAL, Ti1,  IIALU>;
def CSA_GETVAL8        : CSAGetValOp<CSA_GETVAL, Ti8,  IIALU>;
def CSA_GETVAL16       : CSAGetValOp<CSA_GETVAL, Ti16,  IIALU>;
def CSA_GETVAL32       : CSAGetValOp<CSA_GETVAL, Ti32,  IIALU>;
def CSA_GETVAL64       : CSAGetValOp<CSA_GETVAL, Ti64,  IIALU>;

def : Pat<(f32 (CSAGetVal CI0:$link)), (CSA_GETVAL32 $link)>;
def : Pat<(f64 (CSAGetVal CI0:$link)), (CSA_GETVAL64 $link)>;
def : Pat<(v2f32 (CSAGetVal CI0:$link)), (CSA_GETVAL64 $link)>;


def CSA_CALL : PseudoInstCSA<
    (outs),
    (ins calltarget:$callee_func, CI0:$mem_ord, variable_ops),
    "csa_call",
    []> {
      let hasSideEffects = 1;
    }
def : Pat<(CSACallDF tglobaladdr:$dst, CI0:$mem_ord),      (CSA_CALL tglobaladdr:$dst, CI0:$mem_ord)>;
def : Pat<(CSACallDF texternalsym:$dst, CI0:$mem_ord),     (CSA_CALL texternalsym:$dst,CI0:$mem_ord)>;

def CSA_CONTINUE : PseudoInstCSA<
    (outs I64:$context, MemOrdDef:$mem_ord, variable_ops),
    (ins I64:$call_site_index),
    "csa_continue",
    []>;
def CSA_CONTINUEPSEUDO : PseudoInstCSA<
    (outs CI0:$link),
    (ins),
    "csa_continue_pseudo",
    [(set CI0:$link, (CSAContinuePseudo))],
    [IsDataflow]> {
      let hasSideEffects = 1;
    }

let isReturn=1, isTerminator=1, isBarrier=1, Itinerary=IICtl in
def CSA_RETURN : PseudoInstCSA<
    (outs),
    (ins CI0:$mem_ord, variable_ops),
    "csa_return",
    [(CSARet CI0:$mem_ord)], [IsDataflow]>;

class MovOp<GenericOp gen, CSAOpInfo oi,
            InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.RC:$dst),
    (ins oi.RC:$op1), // Reg or literal (e.g. either i32 or f32 for 32b)
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1"),
    [],
    !listconcat(preds,oi.Preds), itin> {
      let OpInfo = oi;
    }

class UnaryOp<GenericOp gen, SDNode opNode, CSAOpInfo oi,
              InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1"),
    [(set oi.RC:$dst, (opNode oi.RCL:$op1))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

// Separate pattern because not/ineg are PatFrags rather than SDNodes
class UnaryOpP<GenericOp gen, PatFrag opNode, CSAOpInfo oi,
               InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
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
  FMTGEN<gen, oid,
    (outs oid.RC:$dst),
    (ins ois.RCL:$op1, ois.RCL:$op2),
    !strconcat(gen.AsmString, oid.InstrSuffix, "\t$dst, $op1, $op2"),
    [],
    !listconcat(preds,gen.Preds,oid.Preds), itin>;

class CvtOp<GenericOp gen, SDNode opNode, CSAOpInfo oid, CSAOpInfo ois,
            InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<gen, oid,
    (outs oid.RC:$dst),
    (ins ois.RCL:$src),
    !strconcat(gen.AsmString, oid.InstrSuffix, ois.InstrSuffix, "\t$dst, $src"),
    [(set oid.RC:$dst, (opNode ois.RCL:$src))],
    !listconcat(preds,gen.Preds,oid.Preds,ois.Preds), itin>;

class CvtOpR<GenericOp gen, SDNode opNode, CSAOpInfo oid, CSAOpInfo ois,
             InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<gen, oid,
    (outs oid.RC:$dst),
    (ins ois.RCL:$src, RMODE:$rm),
    !strconcat(gen.AsmString, oid.InstrSuffix, ois.InstrSuffix, "\t$dst, $src, $rm"),
    [(set oid.RC:$dst, (opNode ois.RCL:$src))],
    !listconcat(preds,gen.Preds,oid.Preds,ois.Preds), itin>;

// We do not bother trying to specify commutative, since there doesn't
// appear to be an advantage.
class BinOp<GenericOp gen, SDNode opNode, CSAOpInfo oi,
            InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1, oi.RCL:$op2),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1, $op2"),
    [(set oi.RC:$dst, (opNode oi.RCL:$op1, oi.RCL:$op2))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

// Same as BinOp, but with an optional rounding mode.
class BinOpR<GenericOp gen, SDNode opNode, CSAOpInfo oi,
            InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
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
  FMTGEN<gen, oi,
    (outs oi.RC:$dst, I1:$cout),
    (ins oi.RCL:$op1, oi.RCL:$op2, Ti1.RCL:$cin),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $cout, $op1, $op2, $cin"),
    [],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

// Min/max operations have an extra sel output.
class MinMaxOp<GenericOp gen, CSAOpInfo oi,
              InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.RC:$res, I1:$sel),
    (ins oi.RCL:$op1, oi.RCL:$op2),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$res, $sel, $op1, $op2"),
    [],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

// Shift ops specifically have an i8 for the shift amount
class ShiftOp<GenericOp gen, SDNode opNode, CSAOpInfo oi,
              InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1, oi.RCL:$op2),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1, $op2"),
    [(set oi.RC:$dst, (opNode oi.RCL:$op1, oi.RCL:$op2))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

// CmpOp is identical to BinOp, except opNode is a PatFrag rather than
// an SDNode...
class CmpOp<GenericOp gen, PatFrag opNode, CSAOpInfo oi,
            InstrItinClass itin = NoItinerary,
            list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs I1:$dst),
    (ins oi.RCL:$op1, oi.RCL:$op2),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1, $op2"),
    [(set I1:$dst, (opNode (oi.VT oi.RCL:$op1), (oi.VT oi.RCL:$op2)))],
    !listconcat(preds,gen.Preds,oi.Preds), itin> {
  let isCompare = 1;
}

class FloatCmpOp<GenericOp gen, PatFrag opNode, CSAOpInfo oi,
            InstrItinClass itin = NoItinerary, string floatFlags = "",
            list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs I1:$dst),
    (ins oi.RCL:$op1, oi.RCL:$op2, FPSignalingOperand:$sig),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1, $op2, ",
               floatFlags, ", $sig"),
    [(set I1:$dst, (opNode (oi.VT oi.RCL:$op1), (oi.VT oi.RCL:$op2)))],
    !listconcat(preds,gen.Preds,oi.Preds), itin> {
  let isCompare = 1;
}

class ShAdd<GenericOp gen, SDNode inner, SDNode outer, CSAOpInfo oi,
            InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1, oi.RCL:$op2, oi.RCL:$op3),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1, $op2, $op3"),
    [(set oi.RC:$dst, (outer (inner oi.RCL:$op1, oi.RCL:$op2), oi.RCL:$op3))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

// 'fma' is a pattern already defined by LLVM.
// Create missing equivalents for fms and fmrs.
class TriOpFrag<dag res> : PatFrag<(ops node:$LHS, node:$MHS, node:$RHS), res>;
def fms : TriOpFrag<(fma node:$LHS, node:$MHS, (fneg node:$RHS))>;
def fmrs: TriOpFrag<(fma (fneg node:$LHS), node:$MHS, node:$RHS)>;

class FusedOp1<GenericOp gen, SDNode inner, SDNode outer, CSAOpInfo oi,
               InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1, oi.RCL:$op2, oi.RCL:$op3, RMODE:$rm),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1, $op2, $op3, $rm"),
    [(set oi.RC:$dst, (outer (inner oi.RCL:$op1, oi.RCL:$op2), oi.RCL:$op3))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

class FusedOp2<GenericOp gen, SDNode inner, SDNode outer, CSAOpInfo oi,
               InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1, oi.RCL:$op2, oi.RCL:$op3, RMODE:$rm),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op2, $op3, $op1, $rm"),
    [(set oi.RC:$dst, (outer oi.RCL:$op1, (inner oi.RCL:$op2, oi.RCL:$op3)))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

class TriOp1<GenericOp gen, SDPatternOperator node, CSAOpInfo oi,
               InstrItinClass itin = NoItinerary, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1, oi.RCL:$op2, oi.RCL:$op3, RMODE:$rm),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $op1, $op2, $op3, $rm"),
    [(set oi.RC:$dst, (node oi.RCL:$op1, oi.RCL:$op2, oi.RCL:$op3))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

class CopyOp<GenericOp gen, CSAOpInfo oi,
             InstrItinClass itin = IIVir, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.RC:$op0, oi.RC:$op1, oi.RC:$op2, oi.RC:$op3),
    (ins oi.RCL:$op4),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$op0, $op1, $op2, $op3, $op4"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin>;

def MERGE     : GenericOp<"merge">;
class MergeOp<string opStr, CSAOpInfo t,
              list<Predicate> preds, InstrItinClass itin> :
  FMTGEN<MERGE, t,
    (outs t.RC:$dst),
    (ins RCLi1:$sel, t.RCL:$v0, t.RCL:$v1),
    !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
    [(set t.VT:$dst, (select RCLi1:$sel, (t.VT t.RCL:$v1), (t.VT t.RCL:$v0)))],
    preds, itin>;

class GateOp<GenericOp gen, CSAOpInfo oi,
             InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.RC:$dst),
    (ins CI0:$sel, oi.RCL:$v),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $sel, $v"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin>;

let isMultiTriggered = 1 in {
class SwitchOp<GenericOp gen, CSAOpInfo oi,
               InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.RC:$dst0, oi.RC:$dst1),
    (ins RCLi1:$sel, oi.RCL:$v),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst0, $dst1, $sel, $v"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin>;

class SwitchAnyOp<GenericOp gen, CSAOpInfo oi,
                  InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.RC:$dst0, oi.RC:$dst1, I1:$sel),
    (ins oi.RCL:$v, PrioOrderOperand:$mode),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst0, $dst1, $sel, $v, $mode"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin>;

class FilterOp<GenericOp gen, CSAOpInfo oi,
               InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.RC:$dst),
    (ins CI1:$sel, oi.RCL:$v),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $sel, $v"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin>;

class PickOp<GenericOp gen, CSAOpInfo oi,
             InstrItinClass itin, list<Predicate> preds = []>:
  FMTGEN<gen, oi,
    (outs oi.RC:$dst),
    (ins RCLi1:$sel, oi.RCL:$v0, oi.RCL:$v1),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $sel, $v0, $v1"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin>;

class PickAnyOp<GenericOp gen, CSAOpInfo oi,
                InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.RC:$dst, I1:$sel),
    (ins oi.RCL:$v0, oi.RCL:$v1, PrioOrderOperand:$mode),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$dst, $sel, $v0, $v1, $mode"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin>;

class CompletionOp<GenericOp gen, CSAOpInfo oi,
             InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs CI8:$availidx, oi.LICRC:$ldres),
    (ins CI8:$stidx, oi.LICRC:$stdata, i8imm:$size),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$availidx, $ldres, $stidx, $stdata, $size"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin>;

// We thought that hasSideEffects would capture the internal state and stream
// output behavior, but it's really not the right thing.  From the standpoint
// of retained state, each time a SEQ or REPEAT is triggered, it has a fresh
// new state and is uneffected by previous "side effects".  Marking these
// operations as having side effects has not benefit and interferes with dead
// instruction detection.
class SeqCOp<GenericOp gen, CSAOpInfo oi,
             InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<gen, oi,  // TODO: something to reflect state
    (outs oi.LICRC:$val, CI1:$pred, CI1:$first, CI1:$last),
    (ins oi.RCL:$base, oi.RCL:$count, oi.RCL:$stride),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$val, $pred, $first, $last, $base, $count, $stride"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin> { }

// See comment for SeqCOp, above, as to why hasSideEffects is not being set.
// TODO: something to reflect state
class SeqSOp<GenericOp gen, CSAOpInfo oi,
             InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.LICRC:$val, CI1:$pred, CI1:$first, CI1:$last),
    (ins oi.RCL:$base, oi.RCL:$bound, oi.RCL:$stride),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$val, $pred, $first, $last, $base, $bound, $stride"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin> { }

// See comment for SeqCOp, above, as to why hasSideEffects is not being set.
class RepeatOp<GenericOp gen, CSAOpInfo oi,
               InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<gen, oi,  // TODO: something to reflect state
    (outs oi.LICRC:$out),
    (ins RCLi1:$pred, oi.RCL:$in),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$out, $pred, $in"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin> { }

// See comment for SeqCOp, above, as to why hasSideEffects is not being set.
class StrideOp<GenericOp gen, CSAOpInfo oi,
               InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<gen, oi,  // TODO: something to reflect state
    (outs oi.LICRC:$out),
    (ins RCLi1:$pred, oi.RCL:$base, oi.RCL:$stride),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$out, $pred, $base, $stride"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin> { }

class ReduceOpR<GenericOp gen, CSAOpInfo oi,
                InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.LICRC:$result),
    (ins oi.RCL:$init, oi.RCL:$inval, RCLi1:$ctl, RMODE:$rm),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$result, $init, $inval, $ctl, $rm"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin> { }

class FMReduceOp<GenericOp gen, CSAOpInfo oi,
                 InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.LICRC:$result),
    (ins oi.RCL:$init, oi.RCL:$inval1, oi.RCL:$inval2, RCLi1:$ctl, RMODE:$rm),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$result, $init, $inval1, $inval2, $ctl, $rm"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin> { }

class FountainOp<GenericOp gen, CSAOpInfo oi,
                 InstrItinClass itin, list<Predicate> preds = []> :
  FMTGEN<gen, oi,
    (outs oi.RC:$res),
    (ins i64imm:$addr, i64imm:$len, RCLi1:$seq),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$res, $addr, $len, $seq"),
    [], !listconcat(preds,gen.Preds,oi.Preds), itin> { }

} // let isMultiTriggered = 1

def LDX : GenericOp<"ldx">;
def LDD : GenericOp<"ldd">;
def LD  : GenericOp<"ld">;
def LDSP: GenericOp<"ldsp">;
multiclass LdOp<string opStr, CSAOpInfo t,
           InstrItinClass itin = IILD> {

  let mayLoad = 1, Itinerary = itin in {
    let Predicates = [HasIndex] in {
      def X : FMTGEN<LDX, t, // indexed reg+reg
        (outs t.RC:$res, MemOrdDef:$outord),
        (ins ADDR_RX:$addr, MEMLVL:$memlvl, MemOrdUse:$inord),
        !strconcat(opStr, "x\t$res, $addr, $outord, $inord, $memlvl"),
        []>;
    }

    let Predicates = [HasDispl] in {
      def D : FMTGEN<LDD, t, // normal literal displacement form (val+k)
        (outs t.RC:$res, MemOrdDef:$outord),
        (ins ADDR_RI:$addr, MEMLVL:$memlvl, MemOrdUse:$inord),
        !strconcat(opStr, "d\t$res, $addr, $outord, $inord, $memlvl"),
        []>;
    }

    def "" : FMTGEN<LD, t, // normal addrreg form
      (outs t.RC:$res, MemOrdDef:$outord),
      (ins ADDR_R:$addr, MEMLVL:$memlvl, MemOrdDef:$inord),
      !strconcat(opStr, "\t$res, $addr, $outord, $inord, $memlvl"),
      []>;

    def SP : FMTGEN<LDSP, t,
      (outs t.RC:$res, MemOrdDef:$outord),
      (ins i64imm:$base, RCLi64:$index, MemOrdUse:$inord),
      !strconcat("ldsp", !cast<string>(t.OpBitSize),
        "\t$res, $base, $index, $outord, $inord"),
      []>;
  }
}

// Ugly combinatorial issue between literal store data and addressing modes
def STX : GenericOp<"stx">;
def STD : GenericOp<"std">;
def ST  : GenericOp<"st">;
def STSP: GenericOp<"stsp">;
multiclass StOp<string opStr, CSAOpInfo t,
           InstrItinClass itin = IIST> {

  let mayStore = 1, Itinerary = itin in {
    let Predicates = [HasIndex] in {
      def X : FMTGEN<STX, t,
        (outs MemOrdDef:$outord),
        (ins ADDR_RX:$addr, t.RCL:$val, MEMLVL:$memlvl, MemOrdUse:$inord),
        !strconcat(opStr, "x\t$addr, $val, $outord, $inord, $memlvl"),
        []>;
    }

    let Predicates = [HasDispl] in {
      def D : FMTGEN<STD, t,
        (outs MemOrdDef:$outord),
        (ins ADDR_RI:$addr, t.RCL:$val, MEMLVL:$memlvl, MemOrdUse:$inord),
        !strconcat(opStr, "d\t$addr, $val, $outord, $inord, $memlvl"),
        []>;
    }

    def "" : FMTGEN<ST, t,
      (outs MemOrdDef:$outord),
      (ins ADDR_R:$addr, t.RCL:$val, MEMLVL:$memlvl, MemOrdDef:$inord),
      !strconcat(opStr, "\t$addr, $val, $outord, $inord, $memlvl"),
      []>;

    def SP : FMTGEN<STSP, t,
      (outs MemOrdDef:$outord),
      (ins i64imm:$base, RCLi64:$index, t.RCL:$val, MemOrdUse:$inord),
      !strconcat("stsp", !cast<string>(t.OpBitSize),
        "\t$base, $index, $val, $outord, $inord"),
      []>;
  }
}

def SLD   : GenericOp<"sld">;
def SLDX2 : GenericOp<"sldx2">;
def SLDX8 : GenericOp<"sldx8">;
multiclass StreamLdOp<string opStr, CSAOpInfo t, InstrItinClass itin = IILD> {
  let mayLoad = 1, Itinerary = itin, isMultiTriggered = 1 in {
    def "" : FMTGEN<SLD, t,
      (outs t.RC:$dst, MemOrdDef:$outord),
      (ins ADDR_R:$addr, RCLi64:$len, RCLi64:$stride, MEMLVL:$memlvl, MemOrdUse:$inord),
      !strconcat(opStr, "\t$dst, $addr, $len, $stride, $outord, $inord, $memlvl"),
      []>;
    def X2 : FMTGEN<SLDX2, t,
      (outs t.RC:$dst0, t.RC:$dst1, MemOrdDef:$outord),
      (ins ADDR_R:$addr, RCLi64:$len, RCLi64:$stride, MEMLVL:$memlvl, MemOrdUse:$inord),
      !strconcat(opStr, "x2 \t$dst0, $dst1, $addr, $len, $stride, $outord, $inord, $memlvl"),
      []>;
    def X8 : FMTGEN<SLDX8, t,
      (outs t.RC:$dst0, t.RC:$dst1, t.RC:$dst2, t.RC:$dst3,
            t.RC:$dst4, t.RC:$dst5, t.RC:$dst6, t.RC:$dst7,
            MemOrdDef:$outord),
      (ins ADDR_R:$addr, RCLi64:$len, RCLi64:$stride, MEMLVL:$memlvl, MemOrdUse:$inord),
      !strconcat(opStr, "x8 \t$dst0, $dst1, $dst2, $dst3, $dst4, $dst5, $dst6, $dst7, $addr, $len, $stride, $outord, $inord, $memlvl"),
      []>;
  }
}

def SST   : GenericOp<"sst">;
def SSTX8 : GenericOp<"sstx8">;
multiclass StreamStOp<string opStr, CSAOpInfo t, InstrItinClass itin = IIST> {
  let mayStore = 1, Itinerary = itin, isMultiTriggered = 1 in {
    def "" : FMTGEN<SST, t,
      (outs MemOrdDef:$outord),
      (ins ADDR_R:$addr, RCLi64:$len, RCLi64:$stride, t.RCL:$data, MEMLVL:$memlvl, MemOrdUse:$inord),
      !strconcat(opStr, "\t$addr, $len, $stride, $data, $outord, $inord, $memlvl"),
      []>;
    def X8 : FMTGEN<SSTX8, t,
      (outs MemOrdDef:$outord),
      (ins ADDR_R:$addr, RCLi64:$len, RCLi64:$stride,
           t.RCL:$data0, t.RCL:$data1, t.RCL:$data2, t.RCL:$data3,
           t.RCL:$data4, t.RCL:$data5, t.RCL:$data6, t.RCL:$data7,
           MEMLVL:$memlvl, MemOrdUse:$inord),
      !strconcat(opStr, "x8 \t$addr, $len, $stride, $data0, $data1, $data2, $data3, $data4, $data5, $data6, $data7, $outord, $inord, $memlvl"),
      []>;
  }
}

class XmulOp<GenericOp gen, CSAOpInfo oi, CSAOpInfo oi2, SDNode ext,
             InstrItinClass itin = IIALU, list<Predicate> preds = []> :
  FMTGEN <gen, oi,
    (outs oi2.RC:$res),
    (ins  oi.RCL:$op1, oi.RCL:$op2),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$res, $op1, $op2"),
    [(set oi2.RC:$res, (mul (ext oi.RCL:$op1), (ext oi.RCL:$op2)))],
    !listconcat(preds,oi.Preds), itin>
    { let mayLoad = 0; let mayStore = 0; let hasSideEffects = 0; }

class ClxmulOp<GenericOp gen, CSAOpInfo oi, CSAOpInfo oi2,
               InstrItinClass itin = IIALU, list<Predicate> preds = []> :
  FMTGEN <gen, oi,
    (outs oi2.RC:$res),
    (ins  oi.RCL:$op1, oi.RCL:$op2),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$res, $op1, $op2"),
    [],
    !listconcat(preds,oi.Preds), itin>
    { let mayLoad = 0; let mayStore = 0; let hasSideEffects = 0; }


class FModOp<GenericOp gen, CSAOpInfo oi,
               InstrItinClass itin = IIALU, list<Predicate> preds = []> :
  FMTGEN <gen, oi,
    (outs oi.RC:$intp, oi.RC:$fracp),
    (ins  oi.RCL:$op1),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$intp, $fracp, $op1"),
    [],
    !listconcat(preds,oi.Preds), itin>
    { let mayLoad = 0; let mayStore = 0; let hasSideEffects = 0; }

class AtomicOp<GenericOp gen, SDPatternOperator opNode, CSAOpInfo oi,
               InstrItinClass itin = IIATM, list<Predicate> preds = []> :
  FMTGEN <gen, oi,
    (outs oi.RC:$dst, MemOrdDef:$outord),
    (ins ADDR_R:$addr, oi.RCL:$val, MEMLVL:$memlvl, MemOrdUse:$inord),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$outord, $dst, $addr, $val, $inord, $memlvl"),
    [],
    !listconcat(preds,gen.Preds,oi.Preds), itin>
    { let mayLoad = 1; let mayStore = 1; let hasSideEffects = 1; }

class AtomicOpR<GenericOp gen, SDPatternOperator opNode, CSAOpInfo oi,
               InstrItinClass itin = IIATM, list<Predicate> preds = []> :
  FMTGEN <gen, oi,
    (outs oi.RC:$dst, MemOrdDef:$outord),
    (ins ADDR_R:$addr, oi.RCL:$val, RMODE:$rm, MEMLVL:$memlvl, MemOrdUse:$inord),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$outord, $dst, $addr, $val, $inord, $rm, $memlvl"),
    [],
    !listconcat(preds,gen.Preds,oi.Preds), itin>
    { let mayLoad = 1; let mayStore = 1; let hasSideEffects = 1; }

class AtomicOp2<GenericOp gen, PatFrag opNode, CSAOpInfo oi,
                InstrItinClass itin = IIATM, list<Predicate> preds = []> :
  FMTGEN <gen, oi,
    (outs oi.RC:$res, MemOrdDef:$outord),
    (ins ADDR_R:$addr, oi.RCL:$cmp, oi.RCL:$repl, MEMLVL:$memlvl, MemOrdUse:$ready),
    !strconcat(gen.AsmString, oi.InstrSuffix, "\t$outord, $res, $addr, $cmp, $repl, $ready, $memlvl"),
    [],
    !listconcat(preds,oi.Preds), itin>
    { let mayLoad = 1; let mayStore = 1; let hasSideEffects = 1; }

multiclass PrefetchOp<string opStr, int rw, InstrItinClass itin = IILD> {
  let mayLoad = 1, mayStore = 1, Itinerary = itin in {
    def "" : FMTGEN <?, ?,
      (outs MemOrdDef:$ctlout),
      (ins ADDR_R:$addr, MEMLVLExplicit:$memlvl, MemOrdUse:$ctlin),
      !strconcat(opStr, "\t$addr, $ctlout, $ctlin, $memlvl"),
      []>;
  }
}

let isBranch=1, isTerminator=1, Itinerary = IICtl in {
  // Branch true
  def BT : FMTGEN<?, ?, (outs), (ins I1:$cond, brtarget:$target),
    "bt\t$cond, $target",
    [(brcond I1:$cond, bb:$target)]>;
  // Branch false
  def BF : FMTGEN<?, ?, (outs), (ins I1:$cond, brtarget:$target),
    "bf\t$cond, $target",
    [(brcond (not I1:$cond), bb:$target)]>;
  // Unconditional branch
  let isBarrier=1 in {
    def BR : FMTGEN<?, ?, (outs), (ins brtarget:$target),
      "br\t$target",
      [(br bb:$target)]>;
  }
}

def : Pat<(brcond (i1 (xor I1:$cond, -1)), bb:$target),
          (BF I1:$cond, bb:$target)>;
def : Pat<(brcond (i1 (setne I1:$cond, -1)), bb:$target),
          (BF I1:$cond, bb:$target)>;

let isReturn=1, isTerminator=1, isBarrier=1, Itinerary = IICtl in
def RET : FMTGEN<?, ?,
    (outs),
    (ins),
    "ret\t%ra",         // implicit use of RA
    [(CSARet)], [/*IsSequential*/]>;

let isBranch=1, isTerminator=1, isBarrier=1, Itinerary = IICtl in
def JMP : FMTGEN<?, ?,
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

  def JSR : FMTGEN<?, ?,
    (outs),
    (ins I64:$target, variable_ops),
    "jsr\t%ra, $target",        // implicit use of RA
    []>;

  def JSRi : FMTGEN<?, ?,
    (outs),
    (ins calltarget:$target, variable_ops),
    "jsr\t%ra, $target",        // implicit use of RA
    []>;

  let isTerminator=1, isReturn=1, isBarrier=1, hasExtraSrcRegAllocReq=1,
    isCodeGenOnly=1 in {
    def JTR : FMTGEN<?, ?,
      (outs),
      (ins I64:$target, variable_ops),
      "jmp\t$target",
      []>;

    def JTRi : FMTGEN<?, ?,
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

def NEGF32    : UnaryOp< NEG,    fneg,   Tf32, IIALU>;
def NEGF64    : UnaryOp< NEG,    fneg,   Tf64, IIALU>;

def ABS       : GenericOp<"abs">;
def ABSF32    : UnaryOp< ABS,    fabs,   Tf32, IIALU>;
def ABSF64    : UnaryOp< ABS,    fabs,   Tf64, IIALU>;

def SQRT      : GenericOp<"sqrt", [HasSqrt]>;
def SQRTF32   : UnaryOp< SQRT,   fsqrt,  Tf32, IISqrtF32>;
def SQRTF64   : UnaryOp< SQRT,   fsqrt,  Tf64, IISqrtF64>;

def FLOOR     : GenericOp<"floor", [HasMath0]>;
def FLOORF32  : UnaryOp< FLOOR,  ffloor, Tf32, IIMathF32>;
def FLOORF64  : UnaryOp< FLOOR,  ffloor, Tf64, IIMathF64>;

def CEIL      : GenericOp<"ceil", [HasMath0]>;
def CEILF32   : UnaryOp< CEIL,   fceil,  Tf32, IIMathF32>;
def CEILF64   : UnaryOp< CEIL,   fceil,  Tf64, IIMathF64>;

def ROUND     : GenericOp<"round", [HasMath0]>;
def ROUNDF32  : UnaryOp< ROUND,  fround, Tf32, IIMathF32>;
def ROUNDF64  : UnaryOp< ROUND,  fround, Tf64, IIMathF64>;

def TRUNC     : GenericOp<"trunc", [HasMath0]>;
def TRUNCF32  : UnaryOp< TRUNC,  ftrunc, Tf32, IIMathF32>;
def TRUNCF64  : UnaryOp< TRUNC,  ftrunc, Tf64, IIMathF64>;

def EXP2      : GenericOp<"exp2", [HasMath0]>;
def EXP2F32   : UnaryOp< EXP2,   fexp2,  Tf32, IIMathF32>;
def EXP2F64   : UnaryOp< EXP2,   fexp2,  Tf64, IIMathF64>;

def LOG2      : GenericOp<"log2", [HasMath0]>;
def LOG2F32   : UnaryOp< LOG2,   flog2,  Tf32, IIMathF32>;
def LOG2F64   : UnaryOp< LOG2,   flog2,  Tf64, IIMathF64>;

def EXP       : GenericOp<"exp", [HasMath0]>;
def EXPF32    : UnaryOp< EXP,    fexp,   Tf32, IIMathF32>;
def EXPF64    : UnaryOp< EXP,    fexp,   Tf64, IIMathF64>;

def LOG       : GenericOp<"log", [HasMath0]>;
def LOGF32    : UnaryOp< LOG,    flog,   Tf32, IIMathF32>;
def LOGF64    : UnaryOp< LOG,    flog,   Tf64, IIMathF64>;

def SIN       : GenericOp<"sin", [HasMath0]>;
def SINF32    : UnaryOp< SIN,    fsin,   Tf32, IIMathF32>;
def SINF64    : UnaryOp< SIN,    fsin,   Tf64, IIMathF64>;

def COS       : GenericOp<"cos", [HasMath0]>;
def COSF32    : UnaryOp< COS,    fcos,   Tf32, IIMathF32>;
def COSF64    : UnaryOp< COS,    fcos,   Tf64, IIMathF64>;

def TAN       : GenericOp<"tan", [HasMath0]>;
def TANF32    : UnaryOp< TAN,    ftan,   Tf32, IIMathF32>;
def TANF64    : UnaryOp< TAN,    ftan,   Tf64, IIMathF64>;

def ATAN      : GenericOp<"atan", [HasMath0]>;
def ATANF32   : UnaryOp< ATAN,   fatan,  Tf32, IIMathF32>;
def ATANF64   : UnaryOp< ATAN,   fatan,  Tf64, IIMathF64>;

def MOD       : GenericOp<"mod", [HasMath0]>;
def MODF32    : FModOp<  MOD,            Tf32, IIMathF32>;
def MODF64    : FModOp<  MOD,            Tf64, IIMathF64>;

//def SINCOS : GenericOp<"sincos", [HasMath0]>;
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
def CVTS32F32 : CvtOpR<   CVT, fp_to_sint,Ts32, Tf32, IICvtIF>;
def CVTS32F64 : CvtOpR<   CVT, fp_to_sint,Ts32, Tf64, IICvtIF>;

def CVTU32F32 : CvtOpR<   CVT, fp_to_uint,Tu32, Tf32, IICvtIF>;
def CVTU32F64 : CvtOpR<   CVT, fp_to_uint,Tu32, Tf64, IICvtIF>;

def CVTS64F32 : CvtOpR<   CVT, fp_to_sint,Ts64, Tf32, IICvtIF>;
def CVTS64F64 : CvtOpR<   CVT, fp_to_sint,Ts64, Tf64, IICvtIF>;

def CVTU64F32 : CvtOpR<   CVT, fp_to_uint,Tu64, Tf32, IICvtIF>;
def CVTU64F64 : CvtOpR<   CVT, fp_to_uint,Tu64, Tf64, IICvtIF>;

def CVTF32S32 : CvtOpR<   CVT, sint_to_fp,Tf32, Ts32, IICvtFI>;
def CVTF32S64 : CvtOpR<   CVT, sint_to_fp,Tf32, Ts64, IICvtFI>;
def CVTF32U32 : CvtOpR<   CVT, uint_to_fp,Tf32, Tu32, IICvtFI>;
def CVTF32U64 : CvtOpR<   CVT, uint_to_fp,Tf32, Tu64, IICvtFI>;

def CVTF64S32 : CvtOpR<   CVT, sint_to_fp,Tf64, Ts32, IICvtFI>;
def CVTF64S64 : CvtOpR<   CVT, sint_to_fp,Tf64, Ts64, IICvtFI>;
def CVTF64U32 : CvtOpR<   CVT, uint_to_fp,Tf64, Tu32, IICvtFI>;
def CVTF64U64 : CvtOpR<   CVT, uint_to_fp,Tf64, Tu64, IICvtFI>;

def CVTF32F64 : CvtOpR<   CVT, fpround,    Tf32, Tf64, IICvtFF>;

def CVTF64F32 : CvtOpR<   CVT, fpextend,   Tf64, Tf32, IICvtFF>;

def AND       : GenericOp<"and">;
let isCommutable = 1 in {
def AND1      : BinOp<   AND,    and,    Ti1,  IIALU>;
def AND8      : BinOp<   AND,    and,    Ti8,  IIALU>;
def AND16     : BinOp<   AND,    and,    Ti16, IIALU>;
def AND32     : BinOp<   AND,    and,    Ti32, IIALU>;
def AND64     : BinOp<   AND,    and,    Ti64, IIALU>;
}

def OR        : GenericOp<"or">;
let isCommutable = 1 in {
def OR1       : BinOp<   OR,     or,     Ti1,  IIALU>;
def OR8       : BinOp<   OR,     or,     Ti8,  IIALU>;
def OR16      : BinOp<   OR,     or,     Ti16, IIALU>;
def OR32      : BinOp<   OR,     or,     Ti32, IIALU>;
def OR64      : BinOp<   OR,     or,     Ti64, IIALU>;
}

def XOR       : GenericOp<"xor">;
let isCommutable = 1 in {
def XOR1      : BinOp<   XOR,    xor,    Ti1,  IIALU>;
def XOR8      : BinOp<   XOR,    xor,    Ti8,  IIALU>;
def XOR16     : BinOp<   XOR,    xor,    Ti16, IIALU>;
def XOR32     : BinOp<   XOR,    xor,    Ti32, IIALU>;
def XOR64     : BinOp<   XOR,    xor,    Ti64, IIALU>;
}

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
let isCommutable = 1 in {
def ADD8      : BinOp<   ADD,    add,    Ti8,  IIALU>;
def ADD16     : BinOp<   ADD,    add,    Ti16, IIALU>;
def ADD32     : BinOp<   ADD,    add,    Ti32, IIALU>;
def ADD64     : BinOp<   ADD,    add,    Ti64, IIALU>;
}

def ADC       : GenericOp<"adc">;
def ADC8      : BinOpC<  ADC,            Ti8,  IIALU>;
def ADC16     : BinOpC<  ADC,            Ti16, IIALU>;
def ADC32     : BinOpC<  ADC,            Ti32, IIALU>;
def ADC64     : BinOpC<  ADC,            Ti64, IIALU>;

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

def SUBF32    : BinOpR<  SUB,    fsub,   Tf32, IIAddF32>;
def SUBF64    : BinOpR<  SUB,    fsub,   Tf64, IIAddF64>;

def MUL       : GenericOp<"mul">;
let isCommutable = 1 in {
def MUL8      : BinOp<   MUL,    mul,    Ti8,  IIMulI8>;
def MUL16     : BinOp<   MUL,    mul,    Ti16, IIMulI16>;
def MUL32     : BinOp<   MUL,    mul,    Ti32, IIMulI32>;
def MUL64     : BinOp<   MUL,    mul,    Ti64, IIMulI64>;
}

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

def DIVF32    : BinOpR<  DIV,    fdiv,   Tf32, IIDivF32, [HasFDiv]>;
def DIVF64    : BinOpR<  DIV,    fdiv,   Tf64, IIDivF64, [HasFDiv]>;

def POW       : GenericOp<"pow", [HasMath0]>;
def POWF32    : BinOp<   POW,    fpow,   Tf32, IIMathF32>;
def POWF64    : BinOp<   POW,    fpow,   Tf64, IIMathF64>;

def ATAN2     : GenericOp<"atan2", [HasMath0]>;
def ATAN2F32  : BinOp<   ATAN2,  fatan2, Tf32, IIMathF32>;
def ATAN2F64  : BinOp<   ATAN2,  fatan2, Tf64, IIMathF64>;

def MIN       : GenericOp<"min">;
def MINS8     : MinMaxOp<MIN,            Ts8,  IIALU>;
def MINS16    : MinMaxOp<MIN,            Ts16, IIALU>;
def MINS32    : MinMaxOp<MIN,            Ts32, IIALU>;
def MINS64    : MinMaxOp<MIN,            Ts64, IIALU>;
def MINU8     : MinMaxOp<MIN,            Tu8,  IIALU>;
def MINU16    : MinMaxOp<MIN,            Tu16, IIALU>;
def MINU32    : MinMaxOp<MIN,            Tu32, IIALU>;
def MINU64    : MinMaxOp<MIN,            Tu64, IIALU>;
def MINF32    : MinMaxOp<MIN,            Tf32, IIALU>;
def MINF64    : MinMaxOp<MIN,            Tf64, IIALU>;

def MAX       : GenericOp<"max">;
def MAXS8     : MinMaxOp<MAX,            Ts8,  IIALU>;
def MAXS16    : MinMaxOp<MAX,            Ts16, IIALU>;
def MAXS32    : MinMaxOp<MAX,            Ts32, IIALU>;
def MAXS64    : MinMaxOp<MAX,            Ts64, IIALU>;
def MAXU8     : MinMaxOp<MAX,            Tu8,  IIALU>;
def MAXU16    : MinMaxOp<MAX,            Tu16, IIALU>;
def MAXU32    : MinMaxOp<MAX,            Tu32, IIALU>;
def MAXU64    : MinMaxOp<MAX,            Tu64, IIALU>;
def MAXF32    : MinMaxOp<MAX,            Tf32, IIALU>;
def MAXF64    : MinMaxOp<MAX,            Tf64, IIALU>;

multiclass SignedCmp<GenericOp generic, PatFrag frag> {
  def S8  : CmpOp<generic, frag, Ts8,  IIALU>;
  def S16 : CmpOp<generic, frag, Ts16, IIALU>;
  def S32 : CmpOp<generic, frag, Ts32, IIALU>;
  def S64 : CmpOp<generic, frag, Ts64, IIALU>;
}
multiclass UnsignedCmp<GenericOp generic, PatFrag frag> {
  def U8  : CmpOp<generic, frag, Tu8,  IIALU>;
  def U16 : CmpOp<generic, frag, Tu16, IIALU>;
  def U32 : CmpOp<generic, frag, Tu32, IIALU>;
  def U64 : CmpOp<generic, frag, Tu64, IIALU>;
}
multiclass FloatCmp<GenericOp generic, PatFrag frag, string floatFlags> {
  def F32 : FloatCmpOp<generic, frag, Tf32, IICmpF, floatFlags>;
  def F64 : FloatCmpOp<generic, frag, Tf64, IICmpF, floatFlags>;
}

def CMPLT  :   GenericOp<"cmplt">;
defm CMPLT :   SignedCmp<CMPLT, setlt>;
defm CMPLT : UnsignedCmp<CMPLT, setult>;
def CMPLE  :   GenericOp<"cmple">;
defm CMPLE :   SignedCmp<CMPLE, setle>;
defm CMPLE : UnsignedCmp<CMPLE, setule>;
def CMPGT  :   GenericOp<"cmpgt">;
defm CMPGT :   SignedCmp<CMPGT, setgt>;
defm CMPGT : UnsignedCmp<CMPGT, setugt>;
def CMPGE  :   GenericOp<"cmpge">;
defm CMPGE :   SignedCmp<CMPGE, setge>;
defm CMPGE : UnsignedCmp<CMPGE, setuge>;
def CMPEQ  :   GenericOp<"cmpeq">;
def CMPNE  :   GenericOp<"cmpne">;

def CMPOLT :   GenericOp<"cmplt">;
def CMPOLE :   GenericOp<"cmple">;
def CMPOGT :   GenericOp<"cmpgt">;
def CMPOGE :   GenericOp<"cmpge">;
def CMPOEQ :   GenericOp<"cmpeq">;
def CMPONE :   GenericOp<"cmpne">;
def CMPULT :   GenericOp<"cmplt">;
def CMPULE :   GenericOp<"cmple">;
def CMPUGT :   GenericOp<"cmpgt">;
def CMPUGE :   GenericOp<"cmpge">;
def CMPUEQ :   GenericOp<"cmpeq">;
def CMPUNE :   GenericOp<"cmpne">;
defm CMPOLT:   FloatCmp<CMPOLT, setolt, "FLT_ORDERED">;
defm CMPOLE:   FloatCmp<CMPOLE, setole, "FLT_ORDERED">;
defm CMPOGT:   FloatCmp<CMPOGT, setogt, "FLT_ORDERED">;
defm CMPOGE:   FloatCmp<CMPOGE, setoge, "FLT_ORDERED">;
defm CMPOEQ:   FloatCmp<CMPOEQ, setoeq, "FLT_ORDERED">;
defm CMPONE:   FloatCmp<CMPONE, setone, "FLT_ORDERED">;
defm CMPULT:   FloatCmp<CMPULT, setult, "FLT_UNORDERED">;
defm CMPULE:   FloatCmp<CMPULE, setule, "FLT_UNORDERED">;
defm CMPUGT:   FloatCmp<CMPUGT, setugt, "FLT_UNORDERED">;
defm CMPUGE:   FloatCmp<CMPUGE, setuge, "FLT_UNORDERED">;
defm CMPUEQ:   FloatCmp<CMPUEQ, setueq, "FLT_UNORDERED">;
defm CMPUNE:   FloatCmp<CMPUNE, setune, "FLT_UNORDERED">;

// These are the flags for we-don't-care-about-NaNs. When the architecture
// finalizes NaN support, these should map into the cheaper ones.
def : Pat<(i1 (setlt f32:$op1, f32:$op2)), (CMPOLTF32 $op1, $op2)>;
def : Pat<(i1 (setle f32:$op1, f32:$op2)), (CMPOLEF32 $op1, $op2)>;
def : Pat<(i1 (setgt f32:$op1, f32:$op2)), (CMPOGTF32 $op1, $op2)>;
def : Pat<(i1 (setge f32:$op1, f32:$op2)), (CMPOGEF32 $op1, $op2)>;
def : Pat<(i1 (seteq f32:$op1, f32:$op2)), (CMPOEQF32 $op1, $op2)>;
def : Pat<(i1 (setne f32:$op1, f32:$op2)), (CMPONEF32 $op1, $op2)>;
def : Pat<(i1 (setlt f64:$op1, f64:$op2)), (CMPOLTF64 $op1, $op2)>;
def : Pat<(i1 (setle f64:$op1, f64:$op2)), (CMPOLEF64 $op1, $op2)>;
def : Pat<(i1 (setgt f64:$op1, f64:$op2)), (CMPOGTF64 $op1, $op2)>;
def : Pat<(i1 (setge f64:$op1, f64:$op2)), (CMPOGEF64 $op1, $op2)>;
def : Pat<(i1 (seteq f64:$op1, f64:$op2)), (CMPOEQF64 $op1, $op2)>;
def : Pat<(i1 (setne f64:$op1, f64:$op2)), (CMPONEF64 $op1, $op2)>;

def CMPO   :   GenericOp<"cmpo">;
def CMPUO  :   GenericOp<"cmpuo">;
defm CMPO  :   FloatCmp<CMPO, seto, "FLT_ORDERED">;
defm CMPUO :   FloatCmp<CMPUO, setuo, "FLT_UNORDERED">;

def : Pat<(i1 (seteq i1:$op1, i1:$op2)), (NOT1 (XOR1 $op1,$op2))>;
def : Pat<(i1 (seteq i1:$op1, (i1 imm:$imm))), (NOT1 (XOR1 $op1,$imm))>;
def CMPEQ8    : CmpOp<   CMPEQ,  seteq,  Ti8,  IIALU>;
def CMPEQ16   : CmpOp<   CMPEQ,  seteq,  Ti16, IIALU>;
def CMPEQ32   : CmpOp<   CMPEQ,  seteq,  Ti32, IIALU>;
def CMPEQ64   : CmpOp<   CMPEQ,  seteq,  Ti64, IIALU>;

def : Pat<(i1 (setne i1:$op1, i1:$op2)), (XOR1 $op1,$op2)>;
def : Pat<(i1 (setne i1:$op1, (i1 imm:$imm))), (XOR1 $op1,$imm)>;
def CMPNE8    : CmpOp<   CMPNE,  setne,  Ti8,  IIALU>;
def CMPNE16   : CmpOp<   CMPNE,  setne,  Ti16, IIALU>;
def CMPNE32   : CmpOp<   CMPNE,  setne,  Ti32, IIALU>;
def CMPNE64   : CmpOp<   CMPNE,  setne,  Ti64, IIALU>;

// Shift/add - itinerary depends on constant small shift amt - else IShft...
def SLADD     : GenericOp<"sladd", [HasShAdd]>;
def SLADD8    : ShAdd<   SLADD,shl,  add,  Ti8,  IISAdd>;
def SLADD16   : ShAdd<   SLADD,shl,  add,  Ti16, IISAdd>;
def SLADD32   : ShAdd<   SLADD,shl,  add,  Ti32, IISAdd>;
def SLADD64   : ShAdd<   SLADD,shl,  add,  Ti64, IISAdd>;

def FMA       : GenericOp<"fma", [HasFMA]>;
def FMAF32    : TriOp1<FMA,  fma, Tf32, IIFMAF32>;
def FMAF64    : TriOp1<FMA,  fma, Tf64, IIFMAF64>;

def FMS       : GenericOp<"fms", [HasFMA]>;
def FMSF32    : TriOp1<FMS,  fms, Tf32, IIFMAF32>;
def FMSF64    : TriOp1<FMS,  fms, Tf64, IIFMAF64>;

def FMRS      : GenericOp<"fmrs", [HasFMA]>;
def FMRSF32   : TriOp1<FMRS, fmrs, Tf32, IIFMAF32>;
def FMRSF64   : TriOp1<FMRS, fmrs, Tf64, IIFMAF64>;

// Unfortunately, "COPY" is already defined...
def GCOPY     : GenericOp<"copy">;
def COPY0     : CopyOp<  GCOPY, Ti0>;
def COPY1     : CopyOp<  GCOPY, Ti1>;
def COPY8     : CopyOp<  GCOPY, Ti8>;
def COPY16    : CopyOp<  GCOPY, Ti16>;
def COPY32    : CopyOp<  GCOPY, Ti32>;
def COPY64    : CopyOp<  GCOPY, Ti64>;

def MERGE0    : MergeOp< "merge0",  Ti0,  [HasI0],  IIVir>;
def MERGE1    : MergeOp< "merge1",  Ti1,  [HasI1],  IIVir>;
def MERGE8    : MergeOp< "merge8",  Ti8,  [HasI8],  IIVir>;
def MERGE16   : MergeOp< "merge16", Ti16, [HasI16], IIVir>;
def MERGE32   : MergeOp< "merge32", T32, [HasI32], IIVir>;
def MERGE64   : MergeOp< "merge64", T64, [HasI64], IIVir>;
def : Pat<(f32 (select RCLi1:$sel, RCLf32:$v1, RCLf32:$v0)),
          (MERGE32 $sel, $v0, $v1)>;
def : Pat<(f64 (select RCLi1:$sel, RCLf64:$v1, RCLf64:$v0)),
          (MERGE64 $sel, $v0, $v1)>;
def : Pat<(v2f32 (select RCLi1:$sel, RCLf32x2:$v1, RCLf32x2:$v0)),
          (MERGE64 $sel, $v0, $v1)>;

def SWITCH    : GenericOp<"switch">;
def SWITCH0   : SwitchOp<SWITCH, Ti0,  IIVir>;
def SWITCH1   : SwitchOp<SWITCH, Ti1,  IIVir>;
def SWITCH8   : SwitchOp<SWITCH, Ti8,  IIVir>;
def SWITCH16  : SwitchOp<SWITCH, Ti16, IIVir>;
def SWITCH32  : SwitchOp<SWITCH, Ti32, IIVir>;
def SWITCH64  : SwitchOp<SWITCH, Ti64, IIVir>;

def SWITCHANY : GenericOp<"switchany">;
def SWITCHANY0  : SwitchAnyOp<SWITCHANY, Ti0,  IIVir>;
def SWITCHANY1  : SwitchAnyOp<SWITCHANY, Ti1,  IIVir>;
def SWITCHANY8  : SwitchAnyOp<SWITCHANY, Ti8,  IIVir>;
def SWITCHANY16 : SwitchAnyOp<SWITCHANY, Ti16, IIVir>;
def SWITCHANY32 : SwitchAnyOp<SWITCHANY, Ti32, IIVir>;
def SWITCHANY64 : SwitchAnyOp<SWITCHANY, Ti64, IIVir>;

def FILTER    : GenericOp<"filter">;
def FILTER0   : FilterOp<FILTER, Ti0,  IIVir>;
def FILTER1   : FilterOp<FILTER, Ti1,  IIVir>;
def FILTER8   : FilterOp<FILTER, Ti8,  IIVir>;
def FILTER16  : FilterOp<FILTER, Ti16, IIVir>;
def FILTER32  : FilterOp<FILTER, Ti32, IIVir>;
def FILTER64  : FilterOp<FILTER, Ti64, IIVir>;

def PICK      : GenericOp<"pick">;
def PICK0     : PickOp<PICK, Ti0,  IIVir>;
def PICK1     : PickOp<PICK, Ti1,  IIVir>;
def PICK8     : PickOp<PICK, Ti8,  IIVir>;
def PICK16    : PickOp<PICK, Ti16, IIVir>;
def PICK32    : PickOp<PICK, Ti32, IIVir>;
def PICK64    : PickOp<PICK, Ti64, IIVir>;

def PICKANY   : GenericOp<"pickany">;
def PICKANY0  : PickAnyOp<PICKANY, Ti0,  IIVir>;
def PICKANY1  : PickAnyOp<PICKANY, Ti1,  IIVir>;
def PICKANY8  : PickAnyOp<PICKANY, Ti8,  IIVir>;
def PICKANY16 : PickAnyOp<PICKANY, Ti16, IIVir>;
def PICKANY32 : PickAnyOp<PICKANY, Ti32, IIVir>;
def PICKANY64 : PickAnyOp<PICKANY, Ti64, IIVir>;

def GATE      : GenericOp<"gate">;
def GATE0     : GateOp<GATE, Ti0,  IIVir>;
def GATE1     : GateOp<GATE, Ti1,  IIVir>;
def GATE8     : GateOp<GATE, Ti8,  IIVir>;
def GATE16    : GateOp<GATE, Ti16, IIVir>;
def GATE32    : GateOp<GATE, Ti32, IIVir>;
def GATE64    : GateOp<GATE, Ti64, IIVir>;

def LAND : GenericOp<"land">;
def LAND1 : FMTGEN<LAND, Ti1,
  (outs I1:$dst),
  (ins RCLi1:$op1, RCLi1:$op2, RCLi1:$op3, RCLi1:$op4),
  "land1\t$dst, $op1, $op2, $op3, $op4",
  [], [], IIALU> { let isMultiTriggered = 1; }

def LOR : GenericOp<"lor">;
def LOR1 : FMTGEN<LOR, Ti1,
  (outs I1:$dst),
  (ins RCLi1:$op1, RCLi1:$op2, RCLi1:$op3, RCLi1:$op4),
  "lor1\t$dst, $op1, $op2, $op3, $op4",
  [], [], IIALU> { let isMultiTriggered = 1; }

def ANY : GenericOp<"any">;
def ANY0 : FMTGEN<ANY, Ti0,
  (outs I64:$dst),
  (ins RCLi0:$op1, RCLi0:$op2, PrioOrderOperand:$ord),
  "any0\t$dst, $op1, $op2, $ord",
  [], [], IIVir> { let isMultiTriggered = 1; }

// TODO: all0 is supposed to support an arbitrary number of inputs, but the late
// tools currently only recognize 4. When CBSDK-222 is implemented to fix that,
// switch to a variadic instruction.
def ALL : GenericOp<"all">;
def ALL0 : FMTGEN<ALL, Ti0,
  (outs I0:$o),
  (ins
    RCLi0:$i0, RCLi0:$i1, RCLi0:$i2, RCLi0:$i3
    //variable_ops
  ),
  "all0\t$o, $i0, $i1, $i2, $i3",
  //"all0",
  [], [], IIVir>;

def ONCOUNT : GenericOp<"oncount">;
def ONCOUNT0 : FMTGEN<ONCOUNT, Ti0,  // TODO: something to reflect state
  (outs I0:$dst),
  (ins RCLi64:$cnt, RCLi0:$op2, RCLi0:$op3, RCLi0:$op4, RCLi0:$op5),
  "oncount0\t$dst, $cnt, $op2, $op3, $op4, $op5",
  [], [], IIVir> { let isMultiTriggered = 1; }

def PREDPROP_G : GenericOp<"predprop">;
def PREDPROP : FMTGEN<PREDPROP_G, Ti1,
  (outs I1:$efalse, I1:$etrue),
  (ins RCLi1:$pb, RCLi1:$sb),
  "predprop\t$efalse, $etrue, $pb, $sb",
  [], [], IIALU> { let isMultiTriggered = 1; }

def PREDMERGE_G : GenericOp<"predmerge">;
def PREDMERGE : FMTGEN<PREDMERGE_G, Ti1,
  (outs I1:$predres, I1:$index),
  (ins RCLi1:$e0, RCLi1:$e1),
  "predmerge\t$predres, $index, $e0, $e1",
  [], [], IIALU> { let isMultiTriggered = 1; }

def PREDFILTER_G : GenericOp<"predfilter">;
def PREDFILTER : FMTGEN<PREDFILTER_G, Ti1,
  (outs I1:$ctlout),
  (ins RCLi1:$ctl, RCLi1:$pred),
  "predfilter\t$ctlout, $ctl, $pred",
  [], [], IIALU> { let isMultiTriggered = 1; }

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
def REPEAT0   : RepeatOp<REPEAT, Ti0,  IIALU>;
def REPEAT1   : RepeatOp<REPEAT, Ti1,  IIALU>;
def REPEAT8   : RepeatOp<REPEAT, Ti8,  IIALU>;
def REPEAT16  : RepeatOp<REPEAT, Ti16, IIALU>;
def REPEAT32  : RepeatOp<REPEAT, Ti32, IIALU>;
def REPEAT64  : RepeatOp<REPEAT, Ti64, IIALU>;

def REPEATO   : GenericOp<"repeato">;
def REPEATO0  : RepeatOp<REPEATO, Ti0,  IIALU>;
def REPEATO1  : RepeatOp<REPEATO, Ti1,  IIALU>;
def REPEATO8  : RepeatOp<REPEATO, Ti8,  IIALU>;
def REPEATO16 : RepeatOp<REPEATO, Ti16, IIALU>;
def REPEATO32 : RepeatOp<REPEATO, Ti32, IIALU>;
def REPEATO64 : RepeatOp<REPEATO, Ti64, IIALU>;

def STRIDE    : GenericOp<"stride">;
def STRIDE8   : StrideOp<STRIDE, Ti8,  IIALU>;
def STRIDE16  : StrideOp<STRIDE, Ti16, IIALU>;
def STRIDE32  : StrideOp<STRIDE, Ti32, IIALU>;
def STRIDE64  : StrideOp<STRIDE, Ti64, IIALU>;

def STRIDEO    : GenericOp<"strideo">;
def STRIDEO8   : StrideOp<STRIDEO, Ti8,  IIALU>;
def STRIDEO16  : StrideOp<STRIDEO, Ti16, IIALU>;
def STRIDEO32  : StrideOp<STRIDEO, Ti32, IIALU>;
def STRIDEO64  : StrideOp<STRIDEO, Ti64, IIALU>;

// FMA reductions.

def FMREDA     : GenericOp<"fmreda">;
def FMREDAF32  : FMReduceOp<FMREDA, Tf32,   IIALU>;
def FMREDAF64  : FMReduceOp<FMREDA, Tf64,   IIALU>;
def FMREDAF32x2: FMReduceOp<FMREDA, Tf32x2, IIALU>;

// Standard sequenced reductions.
def REDSUB     : GenericOp<"redsub">;
def REDSUBF32  : ReduceOpR<REDSUB, Tf32, IIALU>;
def REDSUBF64  : ReduceOpR<REDSUB, Tf64, IIALU>;
def REDSUBF32x2: ReduceOpR<REDSUB, Tf32x2, IIALU>;

def REDADD     : GenericOp<"redadd">;
def REDADDF32  : ReduceOpR<REDADD, Tf32, IIALU>;
def REDADDF64  : ReduceOpR<REDADD, Tf64, IIALU>;
def REDADDF32x2: ReduceOpR<REDADD, Tf32x2, IIALU>;

def REDMUL     : GenericOp<"redmul">;
def REDMULF32  : ReduceOpR<REDMUL, Tf32, IIALU>;
def REDMULF64  : ReduceOpR<REDMUL, Tf64, IIALU>;
def REDMULF32x2: ReduceOpR<REDMUL, Tf32x2, IIALU>;

// TODO: new itinerary class for this one?
def COMPLETION   : GenericOp<"completion">;
def COMPLETION1  : CompletionOp<COMPLETION, Ti1,  IIST>;
def COMPLETION8  : CompletionOp<COMPLETION, Ti8,  IIST>;
def COMPLETION16 : CompletionOp<COMPLETION, Ti16, IIST>;
def COMPLETION32 : CompletionOp<COMPLETION, Ti32, IIST>;
def COMPLETION64 : CompletionOp<COMPLETION, Ti64, IIST>;

def FOUNTAIN   : GenericOp<"fountain">;
def FOUNTAIN1  : FountainOp<FOUNTAIN, Ti1,  IIALU>;
def FOUNTAIN8  : FountainOp<FOUNTAIN, Ti8,  IIALU>;
def FOUNTAIN16 : FountainOp<FOUNTAIN, Ti16, IIALU>;
def FOUNTAIN32 : FountainOp<FOUNTAIN, Ti32, IIALU>;
def FOUNTAIN64 : FountainOp<FOUNTAIN, Ti64, IIALU>;

def REPLICATE  : GenericOp<"replicate">;
def REPLICATE1 : FMTGEN<REPLICATE, Ti1,
  (outs I1:$outchan),
  (ins RCLi1:$inchan, i1imm:$match, i64imm:$count, i64imm:$initpos),
  "replicate1\t$outchan, $inchan, $match, $count, $initpos",
  [], [], IIALU>;

def REPLACE    : GenericOp<"replace">;
def REPLACE1   : FMTGEN<REPLACE, Ti1,
  (outs I1:$out),
  (ins RCLi1:$in, i64imm:$patlen, i64imm:$patbits, i64imm:$repllen, i64imm:$replbits),
  "replace1 $out, $in, $patlen, $patbits, $repllen, $replbits", []> {
    let isMultiTriggered = 1;
}

// TBD(jsukha): This info probably needs predicates
// Also, the input can probably be an I0 channel.
def ONEND_G : GenericOp<"onend">;
def ONEND : FMTGEN<ONEND_G, Ti0,  // TODO: something to reflect state
  (outs I0:$dst),
  (ins RCLi1:$ctrl, RCLi0:$in),
  "onend\t$dst, $ctrl, $in",
  []> { let Itinerary = IIVir; let isMultiTriggered = 1; }

// Not clear that "hasSideEffects" is a good description of static initialization...
def INIT : GenericOp<"init">;
class Init<CSAOpInfo t, list<Predicate> preds, InstrItinClass itin> :
  PseudoInstCSA<
      (outs t.RC:$dst),
      (ins t.L:$imm),
      ".curr\t$dst;\t.value $imm",
      [], preds, itin> {
  let hasSideEffects = 1;
  let GenOp = INIT;
  let OpInfo = t;
  let isMultiTriggered = 1;
}

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
defm LD32     : LdOp<    "ld32",  Ti32>;
defm LD64     : LdOp<    "ld64",  Ti64>;

defm ST1      : StOp<    "st8",   Ti1>;
defm ST8      : StOp<    "st8",   Ti8>;
defm ST16     : StOp<    "st16",  Ti16>;
defm ST32     : StOp<    "st32",  Ti32>;
defm ST64     : StOp<    "st64",  Ti64>;

defm SLD1     : StreamLdOp<"sld8",   Ti1>;
defm SLD8     : StreamLdOp<"sld8",   Ti8>;
defm SLD16    : StreamLdOp<"sld16",  Ti16>;
defm SLD32    : StreamLdOp<"sld32",  Ti32>;
defm SLD64    : StreamLdOp<"sld64",  Ti64>;

defm SST1     : StreamStOp<"sst8",   Ti1>;
defm SST8     : StreamStOp<"sst8",   Ti8>;
defm SST16    : StreamStOp<"sst16",  Ti16>;
defm SST32    : StreamStOp<"sst32",  Ti32>;
defm SST64    : StreamStOp<"sst64",  Ti64>;

// Extended multiplication
def XMUL    : GenericOp<"xmul">;
def XMULS8  : XmulOp<XMUL, Ts8,  Ts16, sext, IIMulI8>;
def XMULU8  : XmulOp<XMUL, Tu8,  Tu16, zext, IIMulI8>;
def XMULS16 : XmulOp<XMUL, Ts16, Ts32, sext, IIMulI16>;
def XMULU16 : XmulOp<XMUL, Tu16, Tu32, zext, IIMulI16>;
def XMULS32 : XmulOp<XMUL, Ts32, Ts64, sext, IIMulI32>;
def XMULU32 : XmulOp<XMUL, Tu32, Tu64, zext, IIMulI32>;

def CLXMUL   : GenericOp<"clxmul">;
def CLXMUL8  : ClxmulOp<CLXMUL, Ti8,  Ti16, IIMulI8>;
def CLXMUL16 : ClxmulOp<CLXMUL, Ti16, Ti32, IIMulI16>;
def CLXMUL32 : ClxmulOp<CLXMUL, Ti32, Ti64, IIMulI32>;

// Atomic operations
def ATMAND    : GenericOp<"atmand", [HasRMWAtomic]>;
def ATMAND8   : AtomicOp<ATMAND,  atomic_load_and_8,  Ti8,  IIATM>;
def ATMAND16  : AtomicOp<ATMAND,  atomic_load_and_16, Ti16, IIATM>;
def ATMAND32  : AtomicOp<ATMAND,  atomic_load_and_32, Ti32, IIATM>;
def ATMAND64  : AtomicOp<ATMAND,  atomic_load_and_64, Ti64, IIATM>;
def ATMADD    : GenericOp<"atmadd", [HasRMWAtomic]>;
def ATMADD8   : AtomicOp<ATMADD,  atomic_load_add_8,  Ti8,  IIATM>;
def ATMADD16  : AtomicOp<ATMADD,  atomic_load_add_16, Ti16, IIATM>;
def ATMADD32  : AtomicOp<ATMADD,  atomic_load_add_32, Ti32, IIATM>;
def ATMADD64  : AtomicOp<ATMADD,  atomic_load_add_64, Ti64, IIATM>;
def ATMMIN    : GenericOp<"atmmin", [HasRMWAtomic]>;
def ATMMIN8   : AtomicOp<ATMMIN,  atomic_load_min_8,  Ti8,  IIATM>;
def ATMMIN16  : AtomicOp<ATMMIN,  atomic_load_min_16, Ti16, IIATM>;
def ATMMIN32  : AtomicOp<ATMMIN,  atomic_load_min_32, Ti32, IIATM>;
def ATMMIN64  : AtomicOp<ATMMIN,  atomic_load_min_64, Ti64, IIATM>;
def ATMMAX    : GenericOp<"atmmax", [HasRMWAtomic]>;
def ATMMAX8   : AtomicOp<ATMMAX,  atomic_load_max_8,  Ti8,  IIATM>;
def ATMMAX16  : AtomicOp<ATMMAX,  atomic_load_max_16, Ti16, IIATM>;
def ATMMAX32  : AtomicOp<ATMMAX,  atomic_load_max_32, Ti32, IIATM>;
def ATMMAX64  : AtomicOp<ATMMAX,  atomic_load_max_64, Ti64, IIATM>;
def ATMOR     : GenericOp<"atmor", [HasRMWAtomic]>;
def ATMOR8    : AtomicOp<ATMOR,   atomic_load_or_8,   Ti8,  IIATM>;
def ATMOR16   : AtomicOp<ATMOR,   atomic_load_or_16,  Ti16, IIATM>;
def ATMOR32   : AtomicOp<ATMOR,   atomic_load_or_32,  Ti32, IIATM>;
def ATMOR64   : AtomicOp<ATMOR,   atomic_load_or_64,  Ti64, IIATM>;
def ATMXOR    : GenericOp<"atmxor", [HasRMWAtomic]>;
def ATMXOR8   : AtomicOp<ATMXOR,  atomic_load_xor_8,  Ti8,  IIATM>;
def ATMXOR16  : AtomicOp<ATMXOR,  atomic_load_xor_16, Ti16, IIATM>;
def ATMXOR32  : AtomicOp<ATMXOR,  atomic_load_xor_32, Ti32, IIATM>;
def ATMXOR64  : AtomicOp<ATMXOR,  atomic_load_xor_64, Ti64, IIATM>;
def ATMXCHG   : GenericOp<"atmxchg", [HasRMWAtomic]>;
def ATMXCHG8  : AtomicOp<ATMXCHG, atomic_swap_8,      Ti8,  IIATM>;
def ATMXCHG16 : AtomicOp<ATMXCHG, atomic_swap_16,     Ti16, IIATM>;
def ATMXCHG32 : AtomicOp<ATMXCHG, atomic_swap_32,     Ti32, IIATM>;
def ATMXCHG64 : AtomicOp<ATMXCHG, atomic_swap_64,     Ti64, IIATM>;
def ATMCMPXCHG   : GenericOp<"atmcmpxchg">;
def ATMCMPXCHG8  : AtomicOp2<ATMCMPXCHG, atomic_cmp_swap_8,  Ti8,  IIATM>;
def ATMCMPXCHG16 : AtomicOp2<ATMCMPXCHG, atomic_cmp_swap_16, Ti16, IIATM>;
def ATMCMPXCHG32 : AtomicOp2<ATMCMPXCHG, atomic_cmp_swap_32, Ti32, IIATM>;
def ATMCMPXCHG64 : AtomicOp2<ATMCMPXCHG, atomic_cmp_swap_64, Ti64, IIATM>;

// For now, LLVM cannot match/select these because the IR's atomics only deal
// with integers.
def ATMADDF32 : AtomicOpR<ATMADD, null_frag, Tf32, IIATM, [HasRMWAtomic]>;
def ATMADDF64 : AtomicOpR<ATMADD, null_frag, Tf64, IIATM, [HasRMWAtomic]>;
def ATMMINF32 : AtomicOp<ATMMIN,  null_frag, Tf32, IIATM, [HasRMWAtomic]>;
def ATMMINF64 : AtomicOp<ATMMIN,  null_frag, Tf64, IIATM, [HasRMWAtomic]>;
def ATMMAXF32 : AtomicOp<ATMMAX,  null_frag, Tf32, IIATM, [HasRMWAtomic]>;
def ATMMAXF64 : AtomicOp<ATMMAX,  null_frag, Tf64, IIATM, [HasRMWAtomic]>;

// Prefetch operations
defm PREFETCH  : PrefetchOp<"prefetch",  0>;
defm PREFETCHW : PrefetchOp<"prefetchw", 1>;

// The fence operation
def FENCE : FMTGEN<?, ?,
  (outs MemOrdDef:$outord),
  (ins MemOrdUse:$inord),
  "fence\t$outord, $inord",
  [], [], IIATM>;

////////////////////////////////////////////////////////////////////////////////
// Math lib helper functions (these have no patterns to match.                //
////////////////////////////////////////////////////////////////////////////////

foreach Kind = [ "Interval", "Signctl" ] in {
  def Kind ## AsmOperand : AsmOperandClass {
    let Name = Kind;
    let IsOptional = 1;
  }
  def Kind ## Operand : OperandWithDefaultOps<i64, (ops (i64 0))> {
    let ParserMatchClass = !cast<AsmOperandClass>(Kind ## AsmOperand);
    let PrintMethod = !strconcat("print", Kind, "Operand");
  }
}

class UnaryNoPatOp<GenericOp op, CSAOpInfo oi, InstrItinClass itin> : FMTGEN<
  op, oi,
  (outs oi.RC:$result),
  (ins oi.RCL:$value),
  !strconcat(op.AsmString, oi.InstrSuffix, "\t$result, $value"),
  [], // No pattern
  [], // No predicates
  itin>;

class GetmantOp<GenericOp op, CSAOpInfo oi, InstrItinClass itin> : FMTGEN<
  op, oi,
  (outs oi.RC:$result),
  (ins oi.RCL:$value, SignctlOperand:$signctl, IntervalOperand:$interval),
  !strconcat(op.AsmString, oi.InstrSuffix, "\t$result, $value, $signctl, $interval"),
  [], // No pattern
  [], // No predicates
  itin>;

class BinaryNoPatOpR<GenericOp op, CSAOpInfo oi, InstrItinClass itin> : FMTGEN<
  op, oi,
  (outs oi.RC:$result),
  (ins oi.RCL:$op1, oi.RCL:$op2, RMODE:$rndmode),
  !strconcat(op.AsmString, oi.InstrSuffix, "\t$result, $op1, $op2, $rndmode"),
  [], // No pattern
  [], // No predicates
  itin>;

class RndscaleOp<GenericOp op, CSAOpInfo oi, InstrItinClass itin> : FMTGEN<
  op, oi,
  (outs oi.RC:$result),
  (ins oi.RCL:$op1, i8imm:$fract, RMODE:$rndmode),
  !strconcat(op.AsmString, oi.InstrSuffix, "\t$result, $op1, $fract, $rndmode"),
  [], // No pattern
  [], // No predicates
  itin>;

class ScaleirsOp<GenericOp op, CSAOpInfo oi, InstrItinClass itin> : FMTGEN<
  op, oi,
  (outs oi.RC:$result),
  (ins oi.RCL:$op1, oi.RCL:$op2, I1:$rndbit, I1:$scalebit, RMODE:$rndmode),
  !strconcat(op.AsmString, oi.InstrSuffix, "\t$result, $op1, $op2, $rndbit, $scalebit, $rndmode"),
  [], // No pattern
  [], // No predicates
  itin>;

class DivcheckOp<GenericOp op, CSAOpInfo oi, InstrItinClass itin> : FMTGEN<
  op, oi,
  (outs oi.RC:$result, I1:$special),
  (ins oi.RCL:$op1, oi.RCL:$op2),
  !strconcat(op.AsmString, oi.InstrSuffix, "\t$result, $special, $op1, $op2"),
  [], // No pattern
  [], // No predicates
  itin>;

class SqrtcheckOp<GenericOp op, CSAOpInfo oi, InstrItinClass itin> : FMTGEN<
  op, oi,
  (outs oi.RC:$result, I1:$special),
  (ins oi.RCL:$op),
  !strconcat(op.AsmString, oi.InstrSuffix, "\t$result, $special, $op"),
  [], // No pattern
  [], // No predicates
  itin>;

class ThreeArgRSOpR<GenericOp op, CSAOpInfo oi, InstrItinClass itin> : FMTGEN<
  op, oi,
  (outs oi.RC:$result, I1:$rndbit, I1:$stickybit),
  (ins oi.RCL:$op1, oi.RCL:$op2, oi.RCL:$op3, RMODE:$rndmode),
  !strconcat(op.AsmString, oi.InstrSuffix,
    "\t$result, $rndbit, $stickybit, $op1, $op2, $op3, $rndmode"),
  [], // No pattern
  [], // No predicates
  itin>;

def GETEXP       : GenericOp<"getexp">;
def GETEXPF32    : UnaryNoPatOp<GETEXP, Tf32, IIMathF32>;
def GETEXPF64    : UnaryNoPatOp<GETEXP, Tf64, IIMathF64>;
def GETMANT      : GenericOp<"getmant">;
def GETMANTF32   : GetmantOp<GETMANT, Tf32, IIMathF32>;
def GETMANTF64   : GetmantOp<GETMANT, Tf64, IIMathF64>;
def SCALE        : GenericOp<"scale">;
def SCALEF32     : BinaryNoPatOpR<SCALE, Tf32, IIMathF32>;
def SCALEF64     : BinaryNoPatOpR<SCALE, Tf64, IIMathF64>;
def RNDSCALE     : GenericOp<"rndscale">;
def RNDSCALEF32  : RndscaleOp<RNDSCALE, Tf32, IIMathF32>;
def RNDSCALEF64  : RndscaleOp<RNDSCALE, Tf64, IIMathF64>;
def RNDSCALESPE  : GenericOp<"rndscalespe">;
def RNDSCALESPEF32:RndscaleOp<RNDSCALESPE, Tf32, IIMathF32>;
def RNDSCALESPEF64:RndscaleOp<RNDSCALESPE, Tf64, IIMathF64>;
def SCALEIRS     : GenericOp<"scaleirs">;
def SCALEIRSF32  : ScaleirsOp<SCALEIRS, Tf32, IIMathF32>;
def SCALEIRSF64  : ScaleirsOp<SCALEIRS, Tf64, IIMathF64>;
def DIVCHECK     : GenericOp<"divcheck">;
def DIVCHECKF32  : DivcheckOp<DIVCHECK, Tf32, IIMathF32>;
def DIVCHECKF64  : DivcheckOp<DIVCHECK, Tf64, IIMathF64>;
def SQRTCHECK    : GenericOp<"sqrtcheck">;
def SQRTCHECKF32 : SqrtcheckOp<SQRTCHECK, Tf32, IIMathF32>;
def SQRTCHECKF64 : SqrtcheckOp<SQRTCHECK, Tf64, IIMathF64>;
def FMAORS       : GenericOp<"fmaors">;
def FMAORSF32    : ThreeArgRSOpR<FMAORS, Tf32, IIMathF32>;
def FMAORSF64    : ThreeArgRSOpR<FMAORS, Tf64, IIMathF64>;
def FMSORS       : GenericOp<"fmsors">;
def FMSORSF32    : ThreeArgRSOpR<FMSORS, Tf32, IIMathF32>;
def FMSORSF64    : ThreeArgRSOpR<FMSORS, Tf64, IIMathF64>;
def FMRSORS      : GenericOp<"fmrsors">;
def FMRSORSF32   : ThreeArgRSOpR<FMRSORS, Tf32, IIMathF32>;
def FMRSORSF64   : ThreeArgRSOpR<FMRSORS, Tf64, IIMathF64>;
def RCP14        : GenericOp<"rcp14", [HasRcpA]>;
def RCP14F32     : UnaryNoPatOp<RCP14, Tf32, IIRcpAF32>;
def RCP14F64     : UnaryNoPatOp<RCP14, Tf64, IIRcpAF64>;
def RSQRT14      : GenericOp<"rsqrt14", [HasRSqrtA]>;
def RSQRT14F32   : UnaryNoPatOp<RSQRT14, Tf32, IIRSqrtAF32>;
def RSQRT14F64   : UnaryNoPatOp<RSQRT14, Tf64, IIRSqrtAF64>;



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

// This instruction serves to preserve the "pipelineable" intrinsic into MIR.
// It is essentially a loop marker. Unlike the user-inserted intrinsics, it
// does not have entry/exit bookends because 1) it's not capturing a region of
// memory operations, and 2) it's generated late and thus doesn't need to
// endure problematic optimizations.
def CSA_PIPELINEABLE_LOOP : PseudoInstCSA<
    (outs),
    (ins I64:$md),
    "# .csa_pipelineable_loop_marker\t$md",
    [ (int_csa_pipelineable_loop_marker (i64 imm:$md)) ]>;

// This pseudo-op appears at the start of a loop that has
// parallel directive (__builtin_csa_parallel_depth_limited_loop() or
// #pragma omp parallel for dataflow(pipeline(depth))).
def CSA_PIPELINE_DEPTH_TOKEN_TAKE : PseudoInstCSA<
    (outs I64:$token, MemOrdDef:$outord),
    (ins ADDR_R:$token_pool, I32:$framesize, I32:$depth, MemOrdUse:$inord),
    "# .csa_pipeline_depth_token_take",
    []>;

// This pseudo-op appears at the end of a loop that has
// parallel directive (__builtin_csa_parallel_depth_limited_loop() or
// #pragma omp parallel for dataflow(pipeline(depth))).
def CSA_PIPELINE_DEPTH_TOKEN_RETURN : PseudoInstCSA<
    (outs MemOrdDef:$outord),
    (ins ADDR_R:$token_pool, I64:$token, MemOrdUse:$inord),
    "# .csa_pipeline_depth_token_return",
    []>;

// Arbitrary immediate support
def : Pat<(i1 imm:$imm),    (MOV1 imm:$imm)>;
def : Pat<(i8 imm:$imm),    (MOV8 imm:$imm)>;
def : Pat<(i16 imm:$imm),   (MOV16 imm:$imm)>;
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
def : Pat<(v2f32 (bitconvert i64:$op1)), (COPY_TO_REGCLASS $op1, I64)>;
def : Pat<(i64 (bitconvert v2f32:$op1)), (COPY_TO_REGCLASS $op1, I64)>;

def : Pat<(i32 (bitconvert f32:$op1)), (COPY_TO_REGCLASS $op1, I32)>;
def : Pat<(f32 (bitconvert i32:$op1)), (COPY_TO_REGCLASS $op1, I32)>;

// The debugprint instruction
let hasSideEffects = 1 in {
  def DEBUGPRINT : FMTGEN<?, ?,
    (outs),
    (ins RCLi64:$op1),
    "debugprint\t$op1",
    [],
    [],
    IIVir
  >;
}

// Floating-point simd instructions
multiclass SimdOp<GenericOp gen, SDNode opNode, SDNode intrinsic, CSAOpInfo oi,
             InstrItinClass itin = NoItinerary, list<Predicate> preds = []> {
  def "" : FMTGEN<gen, oi,
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1, oi.RCL:$op2, i8imm:$disable, i8imm:$swizzle1,
         i8imm:$swizzle2, RMODE:$rm),
    !strconcat(gen.AsmString, oi.InstrSuffix,
        "\t$dst, $op1, $op2, $disable, $swizzle1, $swizzle2, $rm"),
    [(set oi.RC:$dst, (intrinsic oi.RCL:$op1, oi.RCL:$op2, (i8 imm:$disable),
        (i8 imm:$swizzle1), (i8 imm:$swizzle2)))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

  def : Pat<(v2f32 (opNode (CSASwizzleOp oi.RCL:$op1, (i8 imm:$swizzle1)),
                           (CSASwizzleOp oi.RCL:$op2, (i8 imm:$swizzle2)))),
      (!cast<Instruction>(NAME) $op1, $op2, 0, imm:$swizzle1, imm:$swizzle2)>;
  def : Pat<(v2f32 (opNode (CSASwizzleOp oi.RCL:$op1, (i8 imm:$swizzle1)),
                           oi.RCL:$op2)),
      (!cast<Instruction>(NAME) $op1, $op2, 0, imm:$swizzle1, 0)>;
  def : Pat<(v2f32 (opNode oi.RCL:$op1,
                           (CSASwizzleOp oi.RCL:$op2, (i8 imm:$swizzle2)))),
      (!cast<Instruction>(NAME) $op1, $op2, 0, 0, imm:$swizzle2)>;
  def : Pat<(v2f32 (opNode oi.RCL:$op1, oi.RCL:$op2)),
      (!cast<Instruction>(NAME) $op1, $op2, 0, 0, 0)>;
}

multiclass SimdTriOp<GenericOp gen, SDNode opNode, SDNode intrinsic, CSAOpInfo oi,
             InstrItinClass itin = NoItinerary, list<Predicate> preds = []> {
  def "" : FMTGEN<gen, oi,
    (outs oi.RC:$dst),
    (ins oi.RCL:$op1, oi.RCL:$op2, oi.RCL:$op3, i8imm:$disable, i8imm:$swizzle1,
         i8imm:$swizzle2, RMODE:$rm),
    !strconcat(gen.AsmString, oi.InstrSuffix,
        "\t$dst, $op1, $op2, $op3, $disable, $swizzle1, $swizzle2, $rm"),
    [(set oi.RC:$dst, (intrinsic oi.RCL:$op1, oi.RCL:$op2, oi.RCL:$op3,
        (i8 imm:$disable), (i8 imm:$swizzle1), (i8 imm:$swizzle2)))],
    !listconcat(preds,gen.Preds,oi.Preds), itin>;

  def : Pat<(v2f32 (opNode (CSASwizzleOp oi.RCL:$op1, (i8 imm:$swizzle1)),
                           (CSASwizzleOp oi.RCL:$op2, (i8 imm:$swizzle2)),
                           oi.RCL:$op3)),
      (!cast<Instruction>(NAME) $op1, $op2, $op3, 0, imm:$swizzle1, imm:$swizzle2)>;
  def : Pat<(v2f32 (opNode (CSASwizzleOp oi.RCL:$op1, (i8 imm:$swizzle1)),
                           oi.RCL:$op2, oi.RCL:$op3)),
      (!cast<Instruction>(NAME) $op1, $op2, $op3, 0, imm:$swizzle1, 0)>;
  def : Pat<(v2f32 (opNode oi.RCL:$op1,
                           (CSASwizzleOp oi.RCL:$op2, (i8 imm:$swizzle2)),
                           oi.RCL:$op3)),
      (!cast<Instruction>(NAME) $op1, $op2, $op3, 0, 0, imm:$swizzle2)>;
  def : Pat<(v2f32 (opNode oi.RCL:$op1, oi.RCL:$op2, oi.RCL:$op3)),
      (!cast<Instruction>(NAME) $op1, $op2, $op3, 0, 0, 0)>;
}

def ADDSUB : GenericOp<"addsub">;
def SUBADD : GenericOp<"subadd">;
def FMAS : GenericOp<"fmas">;
def FMSA : GenericOp<"fmsa">;

defm ADDF32X2:    SimdOp<ADD, fadd, int_csa_addf32x2, Tf32x2>;
defm SUBF32X2:    SimdOp<SUB, fsub, int_csa_subf32x2, Tf32x2>;
defm ADDSUBF32X2: SimdOp<ADDSUB, CSAAddSub, int_csa_addsubf32x2, Tf32x2>;
defm SUBADDF32X2: SimdOp<SUBADD, CSASubAdd, int_csa_subaddf32x2, Tf32x2>;
defm MULF32X2:    SimdOp<MUL, fmul, int_csa_mulf32x2, Tf32x2>;

defm FMAF32X2:  SimdTriOp<FMA,  fma, int_csa_fmaf32x2, Tf32x2>;
defm FMSF32X2:  SimdTriOp<FMS,  fms, int_csa_fmsf32x2, Tf32x2>;
defm FMRSF32X2: SimdTriOp<FMRS, fmrs, int_csa_fmrsf32x2, Tf32x2>;
defm FMASF32X2: SimdTriOp<FMAS, CSAFMAddSub, int_csa_fmasf32x2, Tf32x2>;
defm FMSAF32X2: SimdTriOp<FMSA, CSAFMSubAdd, int_csa_fmsaf32x2, Tf32x2>;

// Pack, unpack
def PACK: GenericOp<"pack">;
def UNPACK: GenericOp<"unpack">;
def PACK32_64 : FMTGEN<PACK, Ti32,
    (outs I64:$dst),
    (ins RCLf32:$lo, RCLf32:$hi),
    "pack32_64\t$dst, $lo, $hi",
    [(set v2f32:$dst, (build_vector RCLf32:$lo, RCLf32:$hi))]>;
def UNPACK64_32 : FMTGEN<UNPACK, Ti32,
    (outs I32:$lo, I32:$hi),
    (ins RCLi64:$src),
    "unpack64_32\t$src, $lo, $hi",
    []>;

let AddedComplexity = 4 in
def : Pat<(v2f32 (build_vector RCLf32:$lo, (f32 (undef)))),
    (COPY_TO_REGCLASS $lo, I64)>;

def SHUFI32X2: FMTGEN<?, Tf32x2,
    (outs I64:$res),
    (ins RCLf32x2:$op1, RCLf32x2:$op2, i8imm:$sellow, i8imm:$selhigh),
    "shufi32x2\t$res, $op1, $op2, $sellow, $selhigh",
    []>;

def : Pat<(v2f32 (insertelt RCLf32x2:$vec, f32:$val, 0)),
    (SHUFI32X2 $vec, (COPY_TO_REGCLASS $val, I64), 2, 1)>;

def : Pat<(v2f32 (insertelt RCLf32x2:$vec, f32:$val, 1)),
    (SHUFI32X2 $vec, (COPY_TO_REGCLASS $val, I64), 0, 3)>;

def CSA_LIC_INIT : PseudoInstCSA<
    (outs),
    (ins i32imm:$lic, i8imm:$size, i64imm:$depth, i64imm:$desired_depth),
    "# .csa_lower_lic_init $lic, $size, $depth, $desired_depth",
    [(int_csa_lower_lic_init imm:$lic, imm:$size, imm:$depth, imm:$desired_depth)]>;

def CSA_LIC_WRITE : PseudoInstCSA<
    (outs),
    (ins i32imm:$lic, CI64:$val),
    "# .csa_lower_lic_write \t$lic, $val",
    []>;

def CSA_LIC_READ : PseudoInstCSA<
    (outs CI64:$val),
    (ins i32imm:$lic),
    "# .csa_lower_lic_read \t$lic, $val",
    []>;

multiclass PushPop<ValueType VT, RegisterClass RC> {
  def : Pat<(int_csa_lower_lic_write (i32 imm:$lic), VT:$val),
            (CSA_LIC_WRITE (i32 imm:$lic), (COPY_TO_REGCLASS $val, I64))>;
  def : Pat<(VT (int_csa_lower_lic_read (i32 imm:$lic))),
            (COPY_TO_REGCLASS (CSA_LIC_READ (i32 imm:$lic)), RC)>;
}

defm : PushPop<   i1, CI1>;
defm : PushPop<   i8, CI8>;
defm : PushPop<  i16, CI16>;
defm : PushPop<  i32, CI32>;
defm : PushPop<  i64, CI64>;
defm : PushPop<  f32, CI32>;
defm : PushPop<  f64, CI64>;
defm : PushPop<v2f32, CI64>;

// Eventually
//include "CSAIntrinsics.td"
