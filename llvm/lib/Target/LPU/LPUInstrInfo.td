//===- LPUInstrInfo.td - LPU Instruction defs -----------------*- tblgen-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the LPU instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

include "LPUInstrFormats.td"

// Possible TODO:
// - Get rid of "f" variants for stores and other operations?  (e.g. ideally
//   there should be a single LD32, ST32, PICK32, etc.)
// - Get rid of separate ordered loads/stores by having default operands?
//   (Also, should %ign be zero_reg defined in Target.td, and use that
//   as default for order operands for stores/loads?)
// - Possible to merge BinOp and CmpOp (something that can take both a PatFrag
//   and an SDNode?)
// - Use "for" or some other mechanism to deal with 8/16/32/64 redundancy
//   in patterns? (but will likely eventually need opcode bitpatterns, so
//   that may not be feasible/desirable long term)
// - Merge ADDR_I/R and ADDR_RI/RR similar to RegImm?
// - Why are so many operands in LPUGenInstrInfo.inc unknown?

//===----------------------------------------------------------------------===//
// LPU Instruction Predicate Definitions
//===----------------------------------------------------------------------===//

def IsOrdered    : Predicate<"Subtarget.isOrdered()">;
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
def HasSqrt      : Predicate<"Subtarget.hasSqrt()">;
def HasMath0     : Predicate<"Subtarget.hasMath0()">;

//===----------------------------------------------------------------------===//
// LPU Operand Definitions.
//===----------------------------------------------------------------------===//

def brtarget     : Operand<OtherVT>;
def calltarget   : Operand<i64>;

def UnitOpnd : Operand<i64> {
  let PrintMethod = "printUnitOperand";
}

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
def ADDR_R : Addr<1, "SelectAddrReg", (ops I64:$addr)>;

// Not in Target.td with the other *imms
let OperandType = "OPERAND_IMMEDIATE" in {
def f16imm : Operand<f16>;
}

// Register or immedate
class RegImmOperand<ValueType vt> :
  Operand<vt>,
  ComplexPattern<vt,1,"SelectRegImm", [imm, fpimm]> {
    let OperandType = "OPERAND_REG_IMM";
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


//===----------------------------------------------------------------------===//
// LPU Type associations
//===----------------------------------------------------------------------===//
// LPUTypeInfo - information that describes LPU information about value
// types.  For example, what register class and immediate to use.
// (Approach taken from X86 target.)
//
class LPUTypeInfo<ValueType vt, string instrSuffix,
    RegisterClass rc, RegisterClass regRC, RegisterClass licRC,
    DAGOperand rcl, DAGOperand immOperand, list<Predicate> preds> {
  // VT - the value type itself
  ValueType VT = vt;

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

// LPU type associations
def Ti0  : LPUTypeInfo<i1,  "0",    I0,  RI0,  CI0,  RCLi0,  i1imm,  [HasI1]>;
def Ti1  : LPUTypeInfo<i1,  "1",    I1,  RI1,  CI1,  RCLi1,  i1imm,  [HasI1]>;
def Ti8  : LPUTypeInfo<i8,  "8",    I8,  RI8,  CI8,  RCLi8,  i8imm,  [HasI8]>;
def Ts8  : LPUTypeInfo<i8,  "s8",   I8,  RI8,  CI8,  RCLi8,  i8imm,  [HasI8]>;
def Tu8  : LPUTypeInfo<i8,  "u8",   I8,  RI8,  CI8,  RCLi8,  i8imm,  [HasI8]>;
def Ti16 : LPUTypeInfo<i16, "16",   I16, RI16, CI16, RCLi16, i16imm, [HasI16]>;
def Ts16 : LPUTypeInfo<i16, "s16",  I16, RI16, CI16, RCLi16, i16imm, [HasI16]>;
def Tu16 : LPUTypeInfo<i16, "u16",  I16, RI16, CI16, RCLi16, i16imm, [HasI16]>;
def Tf16 : LPUTypeInfo<f16, "f16",  I16, RI16, CI16, RCLf16, f16imm, [HasF16]>;
def Ti32 : LPUTypeInfo<i32, "32",   I32, RI32, CI32, RCLi32, i32imm, [HasI32]>;
def Ts32 : LPUTypeInfo<i32, "s32",  I32, RI32, CI32, RCLi32, i32imm, [HasI32]>;
def Tu32 : LPUTypeInfo<i32, "u32",  I32, RI32, CI32, RCLi32, i32imm, [HasI32]>;
def Tf32 : LPUTypeInfo<f32, "f32",  I32, RI32, CI32, RCLf32, f32imm, [HasF32]>;
def Ti64 : LPUTypeInfo<i64, "64",   I64, RI64, CI64, RCLi64, i64imm, [HasI64]>;
def Ts64 : LPUTypeInfo<i64, "s64",  I64, RI64, CI64, RCLi64, i64imm, [HasI64]>;
def Tu64 : LPUTypeInfo<i64, "u64",  I64, RI64, CI64, RCLi64, i64imm, [HasI64]>;
def Tf64 : LPUTypeInfo<f64, "f64",  I64, RI64, CI64, RCLf64, f64imm, [HasF64]>;


def flog : SDNode<"ISD::FLOG", SDTFPUnaryOp>;
def fexp : SDNode<"ISD::FEXP", SDTFPUnaryOp>;

//===----------------------------------------------------------------------===//
// LPU profiles and nodes
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// LPU Instructions.
//===----------------------------------------------------------------------===//

// These are target-independent nodes, but have target-specific formats.
def SDT_LPUCall         : SDTypeProfile<0, 1, [SDTCisVT<0, iPTR>]>;
def SDT_LPUCallSeqStart : SDCallSeqStart<[ SDTCisVT<0, i64> ]>;
def SDT_LPUCallSeqEnd   : SDCallSeqEnd<[ SDTCisVT<0, i64>, SDTCisVT<1, i64> ]>;

def SDT_LPUWrapper      : SDTypeProfile<1, 1, [SDTCisSameAs<0,1>,
                                               SDTCisPtrTy<0>]>;

def LPURet : SDNode<"LPUISD::Ret", SDTNone,
    [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

// Call
def LPUCall : SDNode<"LPUISD::Call", SDT_LPUCall,
                      [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                      SDNPVariadic]>;

def LPUTailCall : SDNode<"LPUISD::TailCall", SDT_LPUCall,
                      [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_LPUCallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END",   SDT_LPUCallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def LPUWrapper : SDNode<"LPUISD::Wrapper", SDT_LPUWrapper>;

let Defs = [SP], Uses = [SP] in {
def ADJCALLSTACKDOWN : PseudoInstLPU<(outs), (ins i64imm:$amt),
                               "# ADJCALLSTACKDOWN $amt",
                               [(callseq_start timm:$amt)]>;
def ADJCALLSTACKUP : PseudoInstLPU<(outs), (ins i64imm:$amt1, i64imm:$amt2),
                            "# ADJCALLSTACKUP $amt1",
                            [(callseq_end timm:$amt1, timm:$amt2)]>;
}

class MovOp<string opStr, RegisterClass RC, ValueType opType,
            list<Predicate> preds, InstrItinClass itin = NoItinerary> :
  FMTGEN<
    (outs RC:$dst), // Use the more general register class for output
    (ins RC:$op1), // Reg or literal of the appropriate size (e.g. either i32 or f32 for 32b)
    !strconcat(opStr, "\t$dst, $op1"),
    [], preds, itin>;

class UnaryOp<string opStr, SDNode opNode, LPUTypeInfo t,
              list<Predicate> preds, InstrItinClass itin = NoItinerary> :
  FMTGEN<
    (outs t.RC:$dst),
    (ins t.RCL:$op1),
    !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1"),
    [(set t.RC:$dst, (opNode t.RCL:$op1))],
    preds, itin>;

// Separate pattern because "not" is not an SDNode but a PatFrag
class UnaryOpP<string opStr, PatFrag opNode, LPUTypeInfo t,
               list<Predicate> preds, InstrItinClass itin = NoItinerary> :
  FMTGEN<
    (outs t.RC:$dst),
    (ins t.RCL:$op1),
    !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1"),
    [(set t.RC:$dst, (opNode t.RCL:$op1))],
    preds, itin>;

class NegOp<string opStr, LPUTypeInfo t,
            list<Predicate> preds, InstrItinClass itin = NoItinerary> :
  FMTGEN<
    (outs t.RC:$dst),
    (ins t.RCL:$op1),
    !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1"),
    [(set t.RC:$dst, (sub 0, t.RCL:$op1))],
    preds, itin>;

class SExtOp<string opStr, LPUTypeInfo td, LPUTypeInfo ts,
             list<Predicate> preds, InstrItinClass itin = NoItinerary> :
  FMTGEN<
    (outs td.RC:$dst),
    (ins ts.RCL:$op1, ts.RCL:$op2),
    !strconcat(opStr, td.InstrSuffix, "\t$dst, $op1, $op2"),
    [], preds, itin>;

class CvtOp<string opStr, SDNode opNode, LPUTypeInfo td, LPUTypeInfo ts,
            list<Predicate> preds, InstrItinClass itin = NoItinerary> :
  FMTGEN<
    (outs td.RC:$dst),
    (ins ts.RCL:$src),
    !strconcat(opStr, td.InstrSuffix, ts.InstrSuffix, "\t$dst, $src"),
    [(set td.RC:$dst, (opNode ts.RCL:$src))],
    preds, itin>;

// We do not bother trying to specify commutative, since there doesn't
// appear to be an advantage.
class BinOp<string opStr, SDNode opNode, LPUTypeInfo t,
            list<Predicate> preds, InstrItinClass itin = NoItinerary> :
  FMTGEN<
    (outs t.RC:$dst),
    (ins t.RCL:$op1, t.RCL:$op2),
    !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1, $op2"),
    [(set t.RC:$dst, (opNode t.RCL:$op1, t.RCL:$op2))],
    preds, itin>;

// Shift ops specifically have an i8 for the shift amount
class ShiftOp<string opStr, SDNode opNode, LPUTypeInfo t,
              list<Predicate> preds, InstrItinClass itin = NoItinerary> :
  FMTGEN<
    (outs t.RC:$dst),
    (ins t.RCL:$op1, RCLi8:$op2), 
    !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1, $op2"),
    [(set t.RC:$dst, (opNode t.RCL:$op1, RCLi8:$op2))],
    preds, itin>;

// CmpOp is identical to BinOp, except opNode is a PatFrag rather than
// an SDNode...
class CmpOp<string opStr, PatFrag opNode, LPUTypeInfo t,
            list<Predicate> preds, InstrItinClass itin = NoItinerary> :
  FMTGEN<
    (outs I1:$dst),
    (ins t.RCL:$op1, t.RCL:$op2),
    !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1, $op2"),
    [(set I1:$dst, (opNode (t.VT t.RCL:$op1), (t.VT t.RCL:$op2)))],
    preds, itin>;

// BSEL - Only allow constant 1st operand, so CSA network only needs to support
// 2 operand input boxes.
class BSelOp<string opStr, LPUTypeInfo t,
             list<Predicate> preds, InstrItinClass itin = NoItinerary> :
  FMTGEN<
    (outs t.RC:$dst),
    (ins t.L:$sel, t.RCL:$v0, t.RCL:$v1),
    !strconcat(opStr, t.InstrSuffix, "\t$dst, $sel, $v0, $v1"),
    [], preds, itin>;

class ShAdd<string opStr, SDNode inner, SDNode outer, LPUTypeInfo t,
            list<Predicate> preds, InstrItinClass itin = NoItinerary> :
  FMTGEN<
    (outs t.RC:$dst),
    (ins t.RCL:$op1, RCLi8:$op2, t.RCL:$op3),
    !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1, $op2, $op3"),
    [(set t.RC:$dst, (outer (inner t.RCL:$op1, RCLi8:$op2), t.RCL:$op3))],
    preds, itin>;

class FusedOp1<string opStr, SDNode inner, SDNode outer, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin = NoItinerary> :
  FMTGEN<
    (outs t.RC:$dst),
    (ins t.RCL:$op1, t.RCL:$op2, t.RCL:$op3),
    !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1, $op2, $op3"),
    [(set t.RC:$dst, (outer (inner t.RCL:$op1, t.RCL:$op2), t.RCL:$op3))],
    preds, itin>;

class FusedOp2<string opStr, SDNode inner, SDNode outer, LPUTypeInfo t,
               list<Predicate> preds, InstrItinClass itin = NoItinerary> :
  FMTGEN<
    (outs t.RC:$dst),
    (ins t.RCL:$op1, t.RCL:$op2, t.RCL:$op3),
    !strconcat(opStr, t.InstrSuffix, "\t$dst, $op2, $op3, $op1"),
    [(set t.RC:$dst, (outer t.RCL:$op1, (inner t.RCL:$op2, t.RCL:$op3)))],
    preds, itin>;

class CopyOp<string opStr, LPUTypeInfo t,
             list<Predicate> preds = [], InstrItinClass itin = IIVir> :
  FMTGEN<
    (outs t.RC:$op0, t.RC:$op1, t.RC:$op2, t.RC:$op3),
    (ins t.RCL:$op4),
    !strconcat(opStr, t.InstrSuffix, "\t$op0, $op1, $op2, $op3, $op4"),
    [], preds, itin>;

class MergeOp<string opStr, LPUTypeInfo t,
              list<Predicate> preds, InstrItinClass itin> :
  FMTGEN<
    (outs t.RC:$dst),
    (ins I1:$sel, t.RCL:$v0, t.RCL:$v1),
    !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
    [(set t.VT:$dst, (select I1:$sel, (t.VT t.RCL:$v1), (t.VT t.RCL:$v0)))],
    preds, itin>;

class SwitchOp<string opStr, LPUTypeInfo t,
               list<Predicate> preds, InstrItinClass itin> :
  FMTGEN<
    (outs t.RC:$dst0, t.RC:$dst1),
    (ins I1:$sel, t.RCL:$v),
    !strconcat(opStr, "\t$dst0, $dst1, $sel, $v"),
    [], preds, itin>;

class SwitchAnyOp<string opStr, LPUTypeInfo t,
                  list<Predicate> preds, InstrItinClass itin> :
  FMTGEN<
    (outs t.RC:$dst0, t.RC:$dst1, I1:$sel),
    (ins t.RCL:$v),
    !strconcat(opStr, "\t$dst0, $dst1, $sel, $v"),
    [], preds, itin>;

class PickOp<string opStr, LPUTypeInfo t,
             list<Predicate> preds, InstrItinClass itin> :
  FMTGEN<
    (outs t.RC:$dst),
    (ins I1:$sel, t.RCL:$v0, t.RCL:$v1),
    !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
    [], preds, itin>;

class PickAnyOp<string opStr, LPUTypeInfo t,
                list<Predicate> preds, InstrItinClass itin> :
  FMTGEN<
    (outs t.RC:$dst, I1:$sel),
    (ins t.RCL:$v0, t.RCL:$v1),
    !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
    [], preds, itin>;

// hasSideEffects attempts to capture the internal state and stream output behavior
class SeqCOp<string opStr, LPUTypeInfo t,
             list<Predicate> preds, InstrItinClass itin> :
  FMTGEN<  // TODO: something to reflect state
    (outs t.RC:$val, I1:$pred, I1:$first, I1:$last),
    (ins t.RCL:$base, t.RCL:$count, t.RCL:$stride),
    !strconcat(opStr, "\t$val, $pred, $first, $last, $base, $count, $stride"),
    [], preds, itin> { let hasSideEffects = 1; }

// hasSideEffects attempts to capture the internal state and stream output behavior
// TODO: something to reflect state
class SeqSOp<string opStr, LPUTypeInfo t,
             list<Predicate> preds, InstrItinClass itin> :
  FMTGEN<
    (outs t.RC:$val, I1:$pred, I1:$first, I1:$last),
    (ins t.RCL:$base, t.RCL:$bound, t.RCL:$stride),
    !strconcat(opStr, "\t$val, $pred, $first, $last, $base, $bound, $stride"),
    [], preds, itin> { let hasSideEffects = 1; }

// hasSideEffects attempts to capture the internal state and stream output behavior
class RepeatOp<string opStr, LPUTypeInfo t,
               list<Predicate> preds, InstrItinClass itin> :
  FMTGEN<  // TODO: something to reflect state
    (outs I64:$out),
    (ins I1:$pred, t.RCL:$in),
    !strconcat(opStr, "\t$out, $pred, $in"),
    [], preds, itin> { let hasSideEffects = 1; }

// hasSideEffects attempts to capture the internal state and stream output behavior
class StrideOp<string opStr, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin> :
  FMTGEN<  // TODO: something to reflect state
    (outs I64:$out),
    (ins I1:$pred, t.RCL:$base, t.RCL:$stride),
    !strconcat(opStr, "\t$out, $pred, $base, $stride"),
    [], preds, itin> { let hasSideEffects = 1; }

multiclass LdOp<string opStr, LPUTypeInfo t,
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
multiclass StOp<string opStr, LPUTypeInfo t,
           InstrItinClass itin = IIST> {
  let Predicates = [HasIndex], Itinerary = itin in {
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

multiclass OLdOp<string opStr, LPUTypeInfo t,
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

multiclass OStOp<string opStr, LPUTypeInfo t,
           InstrItinClass itin = IIST> {
  let Predicates = [HasIndex], Itinerary = itin in {
    def X : FMTGEN<
      (outs I0:$issued),
      (ins ADDR_RX:$addr, t.RCL:$data, I0:$ready),
      !strconcat(opStr, "x\t$addr, $data, $issued, $ready"),
      []>;
  }

  let Predicates = [HasDispl], Itinerary = itin in {
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

  let Itinerary = itin in {
      def "" : FMTGEN<
        (outs I0:$issued),
        (ins ADDR_R:$addr, t.RCL:$data, I0:$ready),
        !strconcat(opStr, "\t$addr, $data, $issued, $ready"),
        []>;
  }
}

class AtomicOp<string instr_asm, PatFrag opNode, LPUTypeInfo t,
               list<Predicate> preds, InstrItinClass itin = IIATM> :
  FMTGEN <
    (outs t.RC:$dst),
    (ins ADDR_R:$addr, t.RCL:$op3),
    !strconcat(instr_asm, "\t%ign, $dst, $addr, $op3, %ign\t# NOTE: Generated as unordered."),
    [(set t.VT:$dst, (opNode ADDR_R:$addr, (t.VT t.RCL:$op3)))],
    preds, itin>
    { let mayLoad = 1; let mayStore = 1; let hasSideEffects = 1; }

class OAtomicOp<string instr_asm, LPUTypeInfo t,
                list<Predicate> preds, InstrItinClass itin = IIATM> :
  FMTGEN <
    (outs t.RC:$dst, I0:$issued),
    (ins ADDR_R:$addr, t.RCL:$op3, I0:$ready),
    !strconcat(instr_asm, "\t$issued, $dst, $addr, $op3, $ready"),
    [], preds, itin>
    { let mayLoad = 1; let mayStore = 1; let hasSideEffects = 1; }

class AtomicOp2<string instr_asm, PatFrag opNode, LPUTypeInfo t,
                list<Predicate> preds, InstrItinClass itin = IIATM> :
  FMTGEN <
    (outs t.RC:$dst),
    (ins ADDR_R:$addr, t.RCL:$op3, t.RCL:$op4),
    !strconcat(instr_asm, "\t%ign, $dst, $addr, $op3, $op4, %ign\t# NOTE: Generated as unordered."),
    [(set t.RC:$dst, (opNode ADDR_R:$addr, t.RCL:$op4, t.RCL:$op3))]>
    { let mayLoad = 1; let mayStore = 1; let hasSideEffects = 1; }

class OAtomicOp2<string instr_asm, LPUTypeInfo t,
                 list<Predicate> preds, InstrItinClass itin = IIATM> :
  FMTGEN <
    (outs t.RC:$dst, I0:$issued),
    (ins ADDR_R:$addr, t.RCL:$op3, t.RCL:$op4, I0:$ready),
    !strconcat(instr_asm, "\t$issued, $dst, $addr, $op3, $op4, $ready"),
    [], preds, itin>
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
    [(LPURet)]>;

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

def : Pat<(LPUCall tglobaladdr:$dst),
          (JSRi tglobaladdr:$dst)>;
def : Pat<(LPUCall texternalsym:$dst),
          (JSRi texternalsym:$dst)>;
def : Pat<(LPUCall imm:$dst),
          (JSRi imm:$dst)>;
def : Pat<(LPUCall I64:$dst),
          (JSR I64:$dst)>;

// Tail call
def : Pat<(LPUTailCall tglobaladdr:$dst),
          (JTRi tglobaladdr:$dst)>;
def : Pat<(LPUTailCall texternalsym:$dst),
          (JTRi texternalsym:$dst)>;
def : Pat<(LPUTailCall imm:$dst),
          (JTRi imm:$dst)>;
def : Pat<(LPUTailCall I64:$dst),
          (JTR I64:$dst)>;

def MOV1      : MovOp<   "mov1",  I1,  i1,  [HasI1],  IIALU>;
def MOV8      : MovOp<   "mov8",  I8,  i8,  [HasI8],  IIALU>;
def MOV16     : MovOp<   "mov16", I16, i16, [HasI16], IIALU>;
def MOV32     : MovOp<   "mov32", I32, i32, [HasI32], IIALU>;
def MOV64     : MovOp<   "mov64", I64, i64, [HasI64], IIALU>;

def : Pat<(i64 (LPUWrapper tglobaladdr:$src)),   (MOV64 tglobaladdr:$src)>;
def : Pat<(i64 (LPUWrapper texternalsym:$src)),  (MOV64 texternalsym:$src)>;
def : Pat<(i64 (LPUWrapper tblockaddress:$src)), (MOV64 tblockaddress:$src)>;
def : Pat<(i64 (LPUWrapper tjumptable:$src)),    (MOV64 tjumptable:$src)>;

def NOT1      : UnaryOpP<"not",    not,    Ti1,  [HasI1],  IIALU>;
def NOT8      : UnaryOpP<"not",    not,    Ti8,  [HasI8],  IIALU>;
def NOT16     : UnaryOpP<"not",    not,    Ti16, [HasI16], IIALU>;
def NOT32     : UnaryOpP<"not",    not,    Ti32, [HasI32], IIALU>;
def NOT64     : UnaryOpP<"not",    not,    Ti64, [HasI64], IIALU>;
def NEG8      : NegOp<   "neg",            Ti8,  [HasI8],  IIALU>;
def NEG16     : NegOp<   "neg",            Ti16, [HasI16], IIALU>;
def NEG32     : NegOp<   "neg",            Ti32, [HasI32], IIALU>;
def NEG64     : NegOp<   "neg",            Ti64, [HasI64], IIALU>;
def NEGF16    : UnaryOp< "neg",    fneg,   Tf16, [HasF16], IIALU>;
def NEGF32    : UnaryOp< "neg",    fneg,   Tf32, [HasF32], IIALU>;
def NEGF64    : UnaryOp< "neg",    fneg,   Tf64, [HasF64], IIALU>;
def ABSF16    : UnaryOp< "abs",    fabs,   Tf16, [HasF16], IIALU>;
def ABSF32    : UnaryOp< "abs",    fabs,   Tf32, [HasF32], IIALU>;
def ABSF64    : UnaryOp< "abs",    fabs,   Tf64, [HasF64], IIALU>;
def SQRTF16   : UnaryOp< "sqrt",   fsqrt,  Tf16, [HasF16,HasSqrt], IISqrtF16>;
def SQRTF32   : UnaryOp< "sqrt",   fsqrt,  Tf32, [HasF32,HasSqrt], IISqrtF32>;
def SQRTF64   : UnaryOp< "sqrt",   fsqrt,  Tf64, [HasF64,HasSqrt], IISqrtF64>;
def EXP2F16   : UnaryOp< "exp2",   fexp2,  Tf16, [HasF16,HasMath0], IIMathF16>;
def EXP2F32   : UnaryOp< "exp2",   fexp2,  Tf32, [HasF32,HasMath0], IIMathF32>;
def EXP2F64   : UnaryOp< "exp2",   fexp2,  Tf64, [HasF64,HasMath0], IIMathF64>;
def LOG2F16   : UnaryOp< "log2",   flog2,  Tf16, [HasF16,HasMath0], IIMathF16>;
def LOG2F32   : UnaryOp< "log2",   flog2,  Tf32, [HasF32,HasMath0], IIMathF32>;
def LOG2F64   : UnaryOp< "log2",   flog2,  Tf64, [HasF64,HasMath0], IIMathF64>;
def EXPF16    : UnaryOp< "exp",    fexp,   Tf16, [HasF16,HasMath0], IIMathF16>;
def EXPF32    : UnaryOp< "exp",    fexp,   Tf32, [HasF32,HasMath0], IIMathF32>;
def EXPF64    : UnaryOp< "exp",    fexp,   Tf64, [HasF64,HasMath0], IIMathF64>;
def LOGF16    : UnaryOp< "log",    flog,   Tf16, [HasF16,HasMath0], IIMathF16>;
def LOGF32    : UnaryOp< "log",    flog,   Tf32, [HasF32,HasMath0], IIMathF32>;
def LOGF64    : UnaryOp< "log",    flog,   Tf64, [HasF64,HasMath0], IIMathF64>;
def SINF16    : UnaryOp< "sin",    fsin,   Tf16, [HasF16,HasMath0], IIMathF16>;
def SINF32    : UnaryOp< "sin",    fsin,   Tf32, [HasF32,HasMath0], IIMathF32>;
def SINF64    : UnaryOp< "sin",    fsin,   Tf64, [HasF64,HasMath0], IIMathF64>;
def COSF16    : UnaryOp< "cos",    fcos,   Tf16, [HasF16,HasMath0], IIMathF16>;
def COSF32    : UnaryOp< "cos",    fcos,   Tf32, [HasF32,HasMath0], IIMathF32>;
def COSF64    : UnaryOp< "cos",    fcos,   Tf64, [HasF64,HasMath0], IIMathF64>;
//def SINCOSF16 : UnaryOp<"sincos",fsincos,Tf16, [HasF16,HasMath0], IIMathF16>;
//def SINCOSF32 : UnaryOp<"sincos",fsincos,Tf32, [HasF32,HasMath0], IIMathF32>;
//def SINCOSF64 : UnaryOp<"sincos",fsincos,Tf64, [HasF64,HasMath0], IIMathF64>;

// BitOps. Note that LLVM's SDNodes expect the output type to match the input
// type, while in the simulator all output types are 8-bit.
def CTPOP8    : UnaryOp< "ctpop",  ctpop,  Ti8,  [HasI8,HasBitOp],  IIALU>;
def CTPOP16   : UnaryOp< "ctpop",  ctpop,  Ti16, [HasI16,HasBitOp], IIALU>;
def CTPOP32   : UnaryOp< "ctpop",  ctpop,  Ti32, [HasI32,HasBitOp], IIALU>;
def CTPOP64   : UnaryOp< "ctpop",  ctpop,  Ti64, [HasI64,HasBitOp], IIALU>;
def CTLZ8     : UnaryOp< "ctlz",   ctlz,   Ti8,  [HasI8,HasBitOp],  IIALU>;
def CTLZ16    : UnaryOp< "ctlz",   ctlz,   Ti16, [HasI16,HasBitOp], IIALU>;
def CTLZ32    : UnaryOp< "ctlz",   ctlz,   Ti32, [HasI32,HasBitOp], IIALU>;
def CTLZ64    : UnaryOp< "ctlz",   ctlz,   Ti64, [HasI64,HasBitOp], IIALU>;
def CTTZ8     : UnaryOp< "cttz",   cttz,   Ti8,  [HasI8,HasBitOp],  IIALU>;
def CTTZ16    : UnaryOp< "cttz",   cttz,   Ti16, [HasI16,HasBitOp], IIALU>;
def CTTZ32    : UnaryOp< "cttz",   cttz,   Ti32, [HasI32,HasBitOp], IIALU>;
def CTTZ64    : UnaryOp< "cttz",   cttz,   Ti64, [HasI64,HasBitOp], IIALU>;

// Helper fragment to find "parity". Clang's __builtin_parity(x) will result in
// (ctpop(x)&1) in IR, which will be transformed back to parityN(x) by this
// pattern fragment.
def parity : PatFrag<(ops node:$in), (and (ctpop node:$in), 1)>;

def PARITY8   : UnaryOpP<"parity", parity, Ti8,  [HasI8,HasBitOp],  IIALU>;
def PARITY16  : UnaryOpP<"parity", parity, Ti16, [HasI16,HasBitOp], IIALU>;
def PARITY32  : UnaryOpP<"parity", parity, Ti32, [HasI32,HasBitOp], IIALU>;
def PARITY64  : UnaryOpP<"parity", parity, Ti64, [HasI64,HasBitOp], IIALU>;

// todo: int<=>float Conversions
// first type in convert name is result, second is source type
def CVTS32F32 : CvtOp<   "cvt", fp_to_sint,Ts32, Tf32, [HasI32,HasF32], IICvtIF>;
def CVTS32F64 : CvtOp<   "cvt", fp_to_sint,Ts32, Tf64, [HasI32,HasF64], IICvtIF>;
def CVTU32F32 : CvtOp<   "cvt", fp_to_uint,Tu32, Tf32, [HasI32,HasF32], IICvtIF>;
def CVTU32F64 : CvtOp<   "cvt", fp_to_uint,Tu32, Tf64, [HasI32,HasF64], IICvtIF>;
def CVTS64F32 : CvtOp<   "cvt", fp_to_sint,Ts64, Tf32, [HasI64,HasF32], IICvtIF>;
def CVTS64F64 : CvtOp<   "cvt", fp_to_sint,Ts64, Tf64, [HasI64,HasF64], IICvtIF>;
def CVTU64F32 : CvtOp<   "cvt", fp_to_uint,Tu64, Tf32, [HasI64,HasF32], IICvtIF>;
def CVTU64F64 : CvtOp<   "cvt", fp_to_uint,Tu64, Tf64, [HasI64,HasF64], IICvtIF>;

def CVTF32S32 : CvtOp<   "cvt", sint_to_fp,Tf32, Ts32, [HasF32,HasI32], IICvtFI>;
def CVTF32S64 : CvtOp<   "cvt", sint_to_fp,Tf32, Ts64, [HasF32,HasI64], IICvtFI>;
def CVTF32U32 : CvtOp<   "cvt", uint_to_fp,Tf32, Tu32, [HasF32,HasI32], IICvtFI>;
def CVTF32U64 : CvtOp<   "cvt", uint_to_fp,Tf32, Tu64, [HasF32,HasI64], IICvtFI>;
def CVTF64S32 : CvtOp<   "cvt", sint_to_fp,Tf64, Ts32, [HasF64,HasI32], IICvtFI>;
def CVTF64S64 : CvtOp<   "cvt", sint_to_fp,Tf64, Ts64, [HasF64,HasI64], IICvtFI>;
def CVTF64U32 : CvtOp<   "cvt", uint_to_fp,Tf64, Tu32, [HasF64,HasI32], IICvtFI>;
def CVTF64U64 : CvtOp<   "cvt", uint_to_fp,Tf64, Tu64, [HasF64,HasI64], IICvtFI>;

def CVTF32F64 : CvtOp<   "cvt", fround,    Tf32, Tf64, [HasF32,HasF64], IICvtFF>;
def CVTF64F32 : CvtOp<   "cvt", fextend,   Tf64, Tf32, [HasF32,HasF64], IICvtFF>;

def SEXT8     : SExtOp<  "sext",           Ti8,  Ti8,  [HasI8],  IIALU>;
def SEXT16    : SExtOp<  "sext",           Ti16, Ti16, [HasI16], IIALU>;
def SEXT32    : SExtOp<  "sext",           Ti32, Ti32, [HasI32], IIALU>;
def SEXT64    : SExtOp<  "sext",           Ti64, Ti64, [HasI64], IIALU>;

def AND1      : BinOp<   "and",    and,    Ti1,  [HasI1],  IIALU>;
def AND8      : BinOp<   "and",    and,    Ti8,  [HasI8],  IIALU>;
def AND16     : BinOp<   "and",    and,    Ti16, [HasI16], IIALU>;
def AND32     : BinOp<   "and",    and,    Ti32, [HasI32], IIALU>;
def AND64     : BinOp<   "and",    and,    Ti64, [HasI64], IIALU>;

def OR1       : BinOp<   "or",     or,     Ti1,  [HasI1],  IIALU>;
def OR8       : BinOp<   "or",     or,     Ti8,  [HasI8],  IIALU>;
def OR16      : BinOp<   "or",     or,     Ti16, [HasI16], IIALU>;
def OR32      : BinOp<   "or",     or,     Ti32, [HasI32], IIALU>;
def OR64      : BinOp<   "or",     or,     Ti64, [HasI64], IIALU>;

def XOR1      : BinOp<   "xor",    xor,    Ti1,  [HasI1],  IIALU>;
def XOR8      : BinOp<   "xor",    xor,    Ti8,  [HasI8],  IIALU>;
def XOR16     : BinOp<   "xor",    xor,    Ti16, [HasI16], IIALU>;
def XOR32     : BinOp<   "xor",    xor,    Ti32, [HasI32], IIALU>;
def XOR64     : BinOp<   "xor",    xor,    Ti64, [HasI64], IIALU>;

def SLL8      : ShiftOp< "sll",    shl,    Ti8,  [HasI8],  IIShft>;
def SLL16     : ShiftOp< "sll",    shl,    Ti16, [HasI16], IIShft>;
def SLL32     : ShiftOp< "sll",    shl,    Ti32, [HasI32], IIShft>;
def SLL64     : ShiftOp< "sll",    shl,    Ti64, [HasI64], IIShft>;

def SRL8      : ShiftOp< "srl",    srl,    Ti8,  [HasI8],  IIShft>;
def SRL16     : ShiftOp< "srl",    srl,    Ti16, [HasI16], IIShft>;
def SRL32     : ShiftOp< "srl",    srl,    Ti32, [HasI32], IIShft>;
def SRL64     : ShiftOp< "srl",    srl,    Ti64, [HasI64], IIShft>;

def SRA8      : ShiftOp< "sra",    sra,    Ti8,  [HasI8],  IIShft>;
def SRA16     : ShiftOp< "sra",    sra,    Ti16, [HasI16], IIShft>;
def SRA32     : ShiftOp< "sra",    sra,    Ti32, [HasI32], IIShft>;
def SRA64     : ShiftOp< "sra",    sra,    Ti64, [HasI64], IIShft>;

def : Pat<(i1 (add i1:$op1, i1:$op2)),       (XOR1 $op1,$op2)>;
def : Pat<(i1 (add i1:$op1, (i1 imm:$imm))), (XOR1 $op1,$imm)>;

def ADD8      : BinOp<   "add",    add,    Ti8,  [HasI8],  IIALU>;
def ADD16     : BinOp<   "add",    add,    Ti16, [HasI16], IIALU>;
def ADD32     : BinOp<   "add",    add,    Ti32, [HasI32], IIALU>;
def ADD64     : BinOp<   "add",    add,    Ti64, [HasI64], IIALU>;

def ADDF16    : BinOp<   "add",    fadd,   Tf16, [HasF16], IIAddF16>;
def ADDF32    : BinOp<   "add",    fadd,   Tf32, [HasF32], IIAddF32>;
def ADDF64    : BinOp<   "add",    fadd,   Tf64, [HasF64], IIAddF64>;

def : Pat<(i1 (sub i1:$op1, i1:$op2)),       (XOR1 $op1,$op2)>;
def : Pat<(i1 (sub i1:$op1, (i1 imm:$imm))), (XOR1 $op1,$imm)>;
def SUB8      : BinOp<   "sub",    sub,    Ti8,  [HasI8],  IIALU>;
def SUB16     : BinOp<   "sub",    sub,    Ti16, [HasI16], IIALU>;
def SUB32     : BinOp<   "sub",    sub,    Ti32, [HasI32], IIALU>;
def SUB64     : BinOp<   "sub",    sub,    Ti64, [HasI64], IIALU>;

def SUBF16    : BinOp<   "sub",    fsub,   Tf16, [HasF16], IIAddF16>;
def SUBF32    : BinOp<   "sub",    fsub,   Tf32, [HasF32], IIAddF32>;
def SUBF64    : BinOp<   "sub",    fsub,   Tf64, [HasF64], IIAddF64>;

def MUL8      : BinOp<   "mul",    mul,    Ti8,  [HasI8],  IIMulI8>;
def MUL16     : BinOp<   "mul",    mul,    Ti16, [HasI16], IIMulI16>;
def MUL32     : BinOp<   "mul",    mul,    Ti32, [HasI32], IIMulI32>;
def MUL64     : BinOp<   "mul",    mul,    Ti64, [HasI64], IIMulI64>;

def MULF16    : BinOp<   "mul",    fmul,   Tf16, [HasF16], IIMulF16>;
def MULF32    : BinOp<   "mul",    fmul,   Tf32, [HasF32], IIMulF32>;
def MULF64    : BinOp<   "mul",    fmul,   Tf64, [HasF64], IIMulF64>;

def DIVS8     : BinOp<   "div",    sdiv,   Ts8,  [HasI8,HasIDiv],  IIDivI8>;
def DIVS16    : BinOp<   "div",    sdiv,   Ts16, [HasI16,HasIDiv], IIDivI16>;
def DIVS32    : BinOp<   "div",    sdiv,   Ts32, [HasI32,HasIDiv], IIDivI32>;
def DIVS64    : BinOp<   "div",    sdiv,   Ts64, [HasI64,HasIDiv], IIDivI64>;

def DIVU8     : BinOp<   "div",    udiv,   Tu8,  [HasI8,HasIDiv],  IIDivI8>;
def DIVU16    : BinOp<   "div",    udiv,   Tu16, [HasI16,HasIDiv], IIDivI16>;
def DIVU32    : BinOp<   "div",    udiv,   Tu32, [HasI32,HasIDiv], IIDivI32>;
def DIVU64    : BinOp<   "div",    udiv,   Tu64, [HasI64,HasIDiv], IIDivI64>;

def DIVF16    : BinOp<   "div",    fdiv,   Tf16, [HasF16,HasFDiv], IIDivF16>;
def DIVF32    : BinOp<   "div",    fdiv,   Tf32, [HasF32,HasFDiv], IIDivF32>;
def DIVF64    : BinOp<   "div",    fdiv,   Tf64, [HasF64,HasFDiv], IIDivF64>;

def POWF32    : BinOp<   "pow",    fpow,   Tf32, [HasF32,HasMath0], IIMathF32>;
def POWF64    : BinOp<   "pow",    fpow,   Tf64, [HasF64,HasMath0], IIMathF64>;

def CMPLTS8   : CmpOp<   "cmplt",  setlt,  Ts8,  [HasI8],  IIALU>;
def CMPLTS16  : CmpOp<   "cmplt",  setlt,  Ts16, [HasI16], IIALU>;
def CMPLTS32  : CmpOp<   "cmplt",  setlt,  Ts32, [HasI32], IIALU>;
def CMPLTS64  : CmpOp<   "cmplt",  setlt,  Ts64, [HasI64], IIALU>;

def CMPLTU8   : CmpOp<   "cmplt",  setult, Tu8,  [HasI8],  IIALU>;
def CMPLTU16  : CmpOp<   "cmplt",  setult, Tu16, [HasI16], IIALU>;
def CMPLTU32  : CmpOp<   "cmplt",  setult, Tu32, [HasI32], IIALU>;
def CMPLTU64  : CmpOp<   "cmplt",  setult, Tu64, [HasI64], IIALU>;

def CMPLES8   : CmpOp<   "cmple",  setle,  Ts8,  [HasI8],  IIALU>;
def CMPLES16  : CmpOp<   "cmple",  setle,  Ts16, [HasI16], IIALU>;
def CMPLES32  : CmpOp<   "cmple",  setle,  Ts32, [HasI32], IIALU>;
def CMPLES64  : CmpOp<   "cmple",  setle,  Ts64, [HasI64], IIALU>;

def CMPLEU8   : CmpOp<   "cmple",  setule, Tu8,  [HasI8],  IIALU>;
def CMPLEU16  : CmpOp<   "cmple",  setule, Tu16, [HasI16], IIALU>;
def CMPLEU32  : CmpOp<   "cmple",  setule, Tu32, [HasI32], IIALU>;
def CMPLEU64  : CmpOp<   "cmple",  setule, Tu64, [HasI64], IIALU>;

// Note - there is no cmpeq1.  As long as operands are clean, cmpeq8
// suffices
def CMPEQ8    : CmpOp<   "cmpeq",  seteq,  Ti8,  [HasI8],  IIALU>;
def CMPEQ16   : CmpOp<   "cmpeq",  seteq,  Ti16, [HasI16], IIALU>;
def CMPEQ32   : CmpOp<   "cmpeq",  seteq,  Ti32, [HasI32], IIALU>;
def CMPEQ64   : CmpOp<   "cmpeq",  seteq,  Ti64, [HasI64], IIALU>;

def CMPGTS8   : CmpOp<   "cmpgt",  setgt,  Ts8,  [HasI8],  IIALU>;
def CMPGTS16  : CmpOp<   "cmpgt",  setgt,  Ts16, [HasI16], IIALU>;
def CMPGTS32  : CmpOp<   "cmpgt",  setgt,  Ts32, [HasI32], IIALU>;
def CMPGTS64  : CmpOp<   "cmpgt",  setgt,  Ts64, [HasI64], IIALU>;

def CMPGTU8   : CmpOp<   "cmpgt",  setugt, Tu8,  [HasI8],  IIALU>;
def CMPGTU16  : CmpOp<   "cmpgt",  setugt, Tu16, [HasI16], IIALU>;
def CMPGTU32  : CmpOp<   "cmpgt",  setugt, Tu32, [HasI32], IIALU>;
def CMPGTU64  : CmpOp<   "cmpgt",  setugt, Tu64, [HasI64], IIALU>;

def CMPGES8   : CmpOp<   "cmpge",  setge,  Ts8,  [HasI8],  IIALU>;
def CMPGES16  : CmpOp<   "cmpge",  setge,  Ts16, [HasI16], IIALU>;
def CMPGES32  : CmpOp<   "cmpge",  setge,  Ts32, [HasI32], IIALU>;
def CMPGES64  : CmpOp<   "cmpge",  setge,  Ts64, [HasI64], IIALU>;

def CMPGEU8   : CmpOp<   "cmpge",  setuge, Tu8,  [HasI8],  IIALU>;
def CMPGEU16  : CmpOp<   "cmpge",  setuge, Tu16, [HasI16], IIALU>;
def CMPGEU32  : CmpOp<   "cmpge",  setuge, Tu32, [HasI32], IIALU>;
def CMPGEU64  : CmpOp<   "cmpge",  setuge, Tu64, [HasI64], IIALU>;

def : Pat<(i1 (setne i1:$op1, i1:$op2)), (XOR1 $op1,$op2)>;
def : Pat<(i1 (setne i1:$op1, (i1 imm:$imm))), (XOR1 $op1,$imm)>;
def CMPNE8    : CmpOp<   "cmpne",  setne,  Ti8,  [HasI8],  IIALU>;
def CMPNE16   : CmpOp<   "cmpne",  setne,  Ti16, [HasI16], IIALU>;
def CMPNE32   : CmpOp<   "cmpne",  setne,  Ti32, [HasI32], IIALU>;
def CMPNE64   : CmpOp<   "cmpne",  setne,  Ti64, [HasI64], IIALU>;

def CMPOLTF16 : CmpOp<   "cmplt",  setolt, Tf16, [HasF16], IICmpF>;
def CMPOLTF32 : CmpOp<   "cmplt",  setolt, Tf32, [HasF32], IICmpF>;
def CMPOLTF64 : CmpOp<   "cmplt",  setolt, Tf64, [HasF64], IICmpF>;
def CMPULTF16 : CmpOp<   "cmplt",  setult, Tf16, [HasF16], IICmpF>;
def CMPULTF32 : CmpOp<   "cmplt",  setult, Tf32, [HasF32], IICmpF>;
def CMPULTF64 : CmpOp<   "cmplt",  setult, Tf64, [HasF64], IICmpF>;

def CMPOLEF16 : CmpOp<   "cmple",  setole, Tf16, [HasF16], IICmpF>;
def CMPOLEF32 : CmpOp<   "cmple",  setole, Tf32, [HasF32], IICmpF>;
def CMPOLEF64 : CmpOp<   "cmple",  setole, Tf64, [HasF64], IICmpF>;
def CMPULEF16 : CmpOp<   "cmple",  setule, Tf16, [HasF16], IICmpF>;
def CMPULEF32 : CmpOp<   "cmple",  setule, Tf32, [HasF32], IICmpF>;
def CMPULEF64 : CmpOp<   "cmple",  setule, Tf64, [HasF64], IICmpF>;

def CMPOEQF16 : CmpOp<   "cmpeq",  setoeq, Tf16, [HasF16], IICmpF>;
def CMPOEQF32 : CmpOp<   "cmpeq",  setoeq, Tf32, [HasF32], IICmpF>;
def CMPOEQF64 : CmpOp<   "cmpeq",  setoeq, Tf64, [HasF64], IICmpF>;
def CMPUEQF16 : CmpOp<   "cmpeq",  setueq, Tf16, [HasF16], IICmpF>;
def CMPUEQF32 : CmpOp<   "cmpeq",  setueq, Tf32, [HasF32], IICmpF>;
def CMPUEQF64 : CmpOp<   "cmpeq",  setueq, Tf64, [HasF64], IICmpF>;

def CMPOGTF16 : CmpOp<   "cmpgt",  setogt, Tf16, [HasF16], IICmpF>;
def CMPOGTF32 : CmpOp<   "cmpgt",  setogt, Tf32, [HasF32], IICmpF>;
def CMPOGTF64 : CmpOp<   "cmpgt",  setogt, Tf64, [HasF64], IICmpF>;
def CMPUGTF16 : CmpOp<   "cmpgt",  setugt, Tf16, [HasF16], IICmpF>;
def CMPUGTF32 : CmpOp<   "cmpgt",  setugt, Tf32, [HasF32], IICmpF>;
def CMPUGTF64 : CmpOp<   "cmpgt",  setugt, Tf64, [HasF64], IICmpF>;

def CMPOGEF16 : CmpOp<   "cmpge",  setoge, Tf16, [HasF16], IICmpF>;
def CMPOGEF32 : CmpOp<   "cmpge",  setoge, Tf32, [HasF32], IICmpF>;
def CMPOGEF64 : CmpOp<   "cmpge",  setoge, Tf64, [HasF64], IICmpF>;
def CMPUGEF16 : CmpOp<   "cmpge",  setuge, Tf16, [HasF16], IICmpF>;
def CMPUGEF32 : CmpOp<   "cmpge",  setuge, Tf32, [HasF32], IICmpF>;
def CMPUGEF64 : CmpOp<   "cmpge",  setuge, Tf64, [HasF64], IICmpF>;

def CMPONEF16 : CmpOp<   "cmpne",  setone, Tf16, [HasF16], IICmpF>;
def CMPONEF32 : CmpOp<   "cmpne",  setone, Tf32, [HasF32], IICmpF>;
def CMPONEF64 : CmpOp<   "cmpne",  setone, Tf64, [HasF64], IICmpF>;
def CMPUNEF16 : CmpOp<   "cmpne",  setune, Tf16, [HasF16], IICmpF>;
def CMPUNEF32 : CmpOp<   "cmpne",  setune, Tf32, [HasF32], IICmpF>;
def CMPUNEF64 : CmpOp<   "cmpne",  setune, Tf64, [HasF64], IICmpF>;

def CMPOF16   : CmpOp<   "cmpo",   seto,   Tf16, [HasF16], IICmpF>;
def CMPOF32   : CmpOp<   "cmpo",   seto,   Tf32, [HasF32], IICmpF>;
def CMPOF64   : CmpOp<   "cmpo",   seto,   Tf64, [HasF64], IICmpF>;
def CMPUOF16  : CmpOp<   "cmpuo",  setuo,  Tf16, [HasF16], IICmpF>;
def CMPUOF32  : CmpOp<   "cmpuo",  setuo,  Tf32, [HasF32], IICmpF>;
def CMPUOF64  : CmpOp<   "cmpuo",  setuo,  Tf64, [HasF64], IICmpF>;

def BSEL1     : BSelOp<  "bsel",           Ti1,  [HasI1]>;
def BSEL8     : BSelOp<  "bsel",           Ti8,  [HasI8]>;
def BSEL16    : BSelOp<  "bsel",           Ti16, [HasI16]>;
def BSEL32    : BSelOp<  "bsel",           Ti32, [HasI32]>;
def BSEL64    : BSelOp<  "bsel",           Ti64, [HasI64]>;

// Shift/add - itinerary depends on constant small shift amt - else IShft...
def SLADD8    : ShAdd<   "sladd",shl,  add,  Ti8,  [HasI8, HasShAdd], IISAdd>;
def SLADD16   : ShAdd<   "sladd",shl,  add,  Ti16, [HasI16,HasShAdd], IISAdd>;
def SLADD32   : ShAdd<   "sladd",shl,  add,  Ti32, [HasI32,HasShAdd], IISAdd>;
def SLADD64   : ShAdd<   "sladd",shl,  add,  Ti64, [HasI64,HasShAdd], IISAdd>;

def FMAF16    : FusedOp1<"fma",  fmul, fadd, Tf16, [HasF16,HasFMA], IIFMAF16>;
// crude handling of commutative
def FMAF16x   : FusedOp2<"fma",  fmul, fadd, Tf16, [HasF16,HasFMA], IIFMAF16>;
def FMSF16    : FusedOp1<"fms",  fmul, fsub, Tf16, [HasF16,HasFMA], IIFMAF16>;
def FMRSF16   : FusedOp2<"fmrs", fmul, fsub, Tf16, [HasF16,HasFMA], IIFMAF16>;

def FMAF32    : FusedOp1<"fma",  fmul, fadd, Tf32, [HasF32,HasFMA], IIFMAF32>;
// crude handling of commutative
def FMAF32x   : FusedOp2<"fma",  fmul, fadd, Tf32, [HasF32,HasFMA], IIFMAF32>;
def FMSF32    : FusedOp1<"fms",  fmul, fsub, Tf32, [HasF32,HasFMA], IIFMAF32>;
def FMRSF32   : FusedOp2<"fmrs", fmul, fsub, Tf32, [HasF32,HasFMA], IIFMAF32>;

def FMAF64    : FusedOp1<"fma",  fmul, fadd, Tf64, [HasF64,HasFMA], IIFMAF64>;
// crude handling of commutative
def FMAF64x   : FusedOp2<"fma",  fmul, fadd, Tf64, [HasF64,HasFMA], IIFMAF64>;
def FMSF64    : FusedOp1<"fms",  fmul, fsub, Tf64, [HasF64,HasFMA], IIFMAF64>;
def FMRSF64   : FusedOp2<"fmrs", fmul, fsub, Tf64, [HasF64,HasFMA], IIFMAF64>;

def COPY0     : CopyOp<  "copy", Ti0>;
def COPY1     : CopyOp<  "copy", Ti1>;
def COPY8     : CopyOp<  "copy", Ti8>;
def COPY16    : CopyOp<  "copy", Ti16>;
def COPY32    : CopyOp<  "copy", Ti32>;
def COPY64    : CopyOp<  "copy", Ti64>;

def MERGE1    : MergeOp< "merge1",  Ti1,  [HasI1],  IIVir>;
def MERGE8    : MergeOp< "merge8",  Ti8,  [HasI8],  IIVir>;
def MERGE16   : MergeOp< "merge16", Ti16, [HasI16], IIVir>;
def MERGE16f  : MergeOp< "merge16", Tf16, [HasI16], IIVir>;
def MERGE32   : MergeOp< "merge32", Ti32, [HasI32], IIVir>;
def MERGE32f  : MergeOp< "merge32", Tf32, [HasI32], IIVir>;
def MERGE64   : MergeOp< "merge64", Ti64, [HasI64], IIVir>;
def MERGE64f  : MergeOp< "merge64", Tf64, [HasI64], IIVir>;

def SWITCH1   : SwitchOp<"switch1",  Ti1,  [HasI1],  IIVir>;
def SWITCH8   : SwitchOp<"switch8",  Ti8,  [HasI8],  IIVir>;
def SWITCH16  : SwitchOp<"switch16", Ti16, [HasI16], IIVir>;
def SWITCH32  : SwitchOp<"switch32", Ti32, [HasI32], IIVir>;
def SWITCH64  : SwitchOp<"switch64", Ti64, [HasI64], IIVir>;

def SWITCHANY1: SwitchAnyOp<"switchany1",  Ti1,  [HasI1],  IIVir>;
def SWITCHANY8: SwitchAnyOp<"switchany8",  Ti8,  [HasI8],  IIVir>;
def SWITCHANY16:SwitchAnyOp<"switchany16", Ti16, [HasI16], IIVir>;
def SWITCHANY32:SwitchAnyOp<"switchany32", Ti32, [HasI32], IIVir>;
def SWITCHANY64:SwitchAnyOp<"switchany64", Ti64, [HasI64], IIVir>;

def PICK1     : PickOp<  "pick1",  Ti1,  [HasI1],  IIVir>;
def PICK8     : PickOp<  "pick8",  Ti8,  [HasI8],  IIVir>;
def PICK16    : PickOp<  "pick16", Ti16, [HasI16], IIVir>;
def PICK32    : PickOp<  "pick32", Ti32, [HasI32], IIVir>;
def PICK64    : PickOp<  "pick64", Ti64, [HasI64], IIVir>;

def PICKANY1  :PickAnyOp<"pickany1",  Ti1,  [HasI1],  IIVir>;
def PICKANY8  :PickAnyOp<"pickany8",  Ti8,  [HasI8],  IIVir>;
def PICKANY16 :PickAnyOp<"pickany16", Ti16, [HasI16], IIVir>;
def PICKANY32 :PickAnyOp<"pickany32", Ti32, [HasI32], IIVir>;
def PICKANY64 :PickAnyOp<"pickany64", Ti64, [HasI64], IIVir>;

def LAND1 : FMTGEN<
  (outs I1:$dst),
  (ins RCLi1:$op1, RCLi1:$op2, RCLi1:$op3, RCLi1:$op4),
  "land1\t$dst, $op1, $op2, $op3, $op4",
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
  "predprop\t$predres, $index, $e0, $e1",
  [], [], IIALU>;

def SEQC8     : SeqCOp<  "seqc8",    Ti8,  [HasI8],  IIALU>;
def SEQC16    : SeqCOp<  "seqc16",   Ti16, [HasI16], IIALU>;
def SEQC32    : SeqCOp<  "seqc32",   Ti32, [HasI32], IIALU>;
def SEQC64    : SeqCOp<  "seqc64",   Ti64, [HasI64], IIALU>;

def SEQLTS8   : SeqSOp<  "seqlts8",  Ti8,  [HasI8],  IIALU>;
def SEQLTS16  : SeqSOp<  "seqlts16", Ti16, [HasI16], IIALU>;
def SEQLTS32  : SeqSOp<  "seqlts32", Ti32, [HasI32], IIALU>;
def SEQLTS64  : SeqSOp<  "seqlts64", Ti64, [HasI64], IIALU>;

def SEQLTU8   : SeqSOp<  "seqltu8",  Ti8,  [HasI8],  IIALU>;
def SEQLTU16  : SeqSOp<  "seqltu16", Ti16, [HasI16], IIALU>;
def SEQLTU32  : SeqSOp<  "seqltu32", Ti32, [HasI32], IIALU>;
def SEQLTU64  : SeqSOp<  "seqltu64", Ti64, [HasI64], IIALU>;

def SEQLES8   : SeqSOp<  "seqles8",  Ti8,  [HasI8],  IIALU>;
def SEQLES16  : SeqSOp<  "seqles16", Ti16, [HasI16], IIALU>;
def SEQLES32  : SeqSOp<  "seqles32", Ti32, [HasI32], IIALU>;
def SEQLES64  : SeqSOp<  "seqles64", Ti64, [HasI64], IIALU>;

def SEQLEU8   : SeqSOp<  "seqleu8",  Ti8,  [HasI8],  IIALU>;
def SEQLEU16  : SeqSOp<  "seqleu16", Ti16, [HasI16], IIALU>;
def SEQLEU32  : SeqSOp<  "seqleu32", Ti32, [HasI32], IIALU>;
def SEQLEU64  : SeqSOp<  "seqleu64", Ti64, [HasI64], IIALU>;

def SEQNE8    : SeqSOp<  "seqne8",   Ti8,  [HasI8],  IIALU>;
def SEQNE16   : SeqSOp<  "seqne16",  Ti16, [HasI16], IIALU>;
def SEQNE32   : SeqSOp<  "seqne32",  Ti32, [HasI32], IIALU>;
def SEQNE64   : SeqSOp<  "seqne64",  Ti64, [HasI64], IIALU>;

def SEQGTS8   : SeqSOp<  "seqgts8",  Ti8,  [HasI8],  IIALU>;
def SEQGTS16  : SeqSOp<  "seqgts16", Ti16, [HasI16], IIALU>;
def SEQGTS32  : SeqSOp<  "seqgts32", Ti32, [HasI32], IIALU>;
def SEQGTS64  : SeqSOp<  "seqgts64", Ti64, [HasI64], IIALU>;

def SEQGTU8   : SeqSOp<  "seqgtu8",  Ti8,  [HasI8],  IIALU>;
def SEQGTU16  : SeqSOp<  "seqgtu16", Ti16, [HasI16], IIALU>;
def SEQGTU32  : SeqSOp<  "seqgtu32", Ti32, [HasI32], IIALU>;
def SEQGTU64  : SeqSOp<  "seqgtu64", Ti64, [HasI64], IIALU>;

def SEQGES8   : SeqSOp<  "seqges8",  Ti8,  [HasI8],  IIALU>;
def SEQGES16  : SeqSOp<  "seqges16", Ti16, [HasI16], IIALU>;
def SEQGES32  : SeqSOp<  "seqges32", Ti32, [HasI32], IIALU>;
def SEQGES64  : SeqSOp<  "seqges64", Ti64, [HasI64], IIALU>;

def SEQGEU8   : SeqSOp<  "seqgeu8",  Ti8,  [HasI8],  IIALU>;
def SEQGEU16  : SeqSOp<  "seqgeu16", Ti16, [HasI16], IIALU>;
def SEQGEU32  : SeqSOp<  "seqgeu32", Ti32, [HasI32], IIALU>;
def SEQGEU64  : SeqSOp<  "seqgeu64", Ti64, [HasI64], IIALU>;

def REPEAT8   : RepeatOp<"repeat8",  Ti8,  [HasI8],  IIALU>;
def REPEAT16  : RepeatOp<"repeat16", Ti16, [HasI16], IIALU>;
def REPEAT32  : RepeatOp<"repeat32", Ti32, [HasI32], IIALU>;
def REPEAT64  : RepeatOp<"repeat64", Ti64, [HasI64], IIALU>;

def STRIDE8   : StrideOp<"stride8",  Ti8,  [HasI8],  IIALU>;
def STRIDE16  : StrideOp<"stride16", Ti16, [HasI16], IIALU>;
def STRIDE32  : StrideOp<"stride32", Ti32, [HasI32], IIALU>;
def STRIDE64  : StrideOp<"stride64", Ti64, [HasI64], IIALU>;

// TBD(jsukha): This info probably needs predicates
// Also, the input can probably be an I0 channel.
def ONEND : FMTGEN<  // TODO: something to reflect state
  (outs I0:$dst),
  (ins I1:$ctrl, I0:$in),
  "onend\t$dst, $ctrl, $in",
  []> { let Itinerary = IIVir; }
  
// Not clear that "hasSideEffects" is a good description of static initialization...
class Init<LPUTypeInfo t, list<Predicate> preds, InstrItinClass itin> :
  PseudoInstLPU<
      (outs t.RC:$dst),
      (ins t.L:$imm),
      ".curr\t$dst;\t.value $imm;\t.avail 0",
      [], preds, itin> { let hasSideEffects = 1; }

def INIT0  : Init<Ti0,  [HasI1],  IIVir>;
def INIT1  : Init<Ti1,  [HasI1],  IIVir>;
def INIT8  : Init<Ti8,  [HasI8],  IIVir>;
def INIT16 : Init<Ti16, [HasI16], IIVir>;
def INIT32 : Init<Ti32, [HasI32], IIVir>;
def INIT64 : Init<Ti64, [HasI64], IIVir>;

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
//defm ST16f    : StOp<    "st16",  Tf16>;
defm ST32     : StOp<    "st32",  Ti32>;
//defm ST32f    : StOp<    "st32",  Tf32>;
defm ST64     : StOp<    "st64",  Ti64>;
//defm ST64f    : StOp<    "st64",  Tf64>;

defm OST1     : OStOp<   "st8",   Ti1>;
defm OST8     : OStOp<   "st8",   Ti8>;
defm OST16    : OStOp<   "st16",  Ti16>;
//defm OST16f   : OStOp<   "st16",  Tf16>;
defm OST32    : OStOp<   "st32",  Ti32>;
//defm OST32f   : OStOp<   "st32",  Tf32>;
defm OST64    : OStOp<   "st64",  Ti64>;
//defm OST64f   : OStOp<   "st64",  Tf64>;

// Atomic operations
def ATMAND8   : AtomicOp<"atmand8",   atomic_load_and_8,  Ti8,  [HasI8],  IIATM>;
def ATMAND16  : AtomicOp<"atmand16",  atomic_load_and_16, Ti16, [HasI16], IIATM>;
def ATMAND32  : AtomicOp<"atmand32",  atomic_load_and_32, Ti32, [HasI32], IIATM>;
def ATMAND64  : AtomicOp<"atmand64",  atomic_load_and_64, Ti64, [HasI64], IIATM>;
def ATMADD8   : AtomicOp<"atmadd8",   atomic_load_add_8,  Ti8,  [HasI8],  IIATM>;
def ATMADD16  : AtomicOp<"atmadd16",  atomic_load_add_16, Ti16, [HasI16], IIATM>;
def ATMADD32  : AtomicOp<"atmadd32",  atomic_load_add_32, Ti32, [HasI32], IIATM>;
def ATMADD64  : AtomicOp<"atmadd64",  atomic_load_add_64, Ti64, [HasI64], IIATM>;
def ATMMIN8   : AtomicOp<"atmmin8",   atomic_load_min_8,  Ti8,  [HasI8],  IIATM>;
def ATMMIN16  : AtomicOp<"atmmin16",  atomic_load_min_16, Ti16, [HasI16], IIATM>;
def ATMMIN32  : AtomicOp<"atmmin32",  atomic_load_min_32, Ti32, [HasI32], IIATM>;
def ATMMIN64  : AtomicOp<"atmmin64",  atomic_load_min_64, Ti64, [HasI64], IIATM>;
def ATMMAX8   : AtomicOp<"atmmax8",   atomic_load_max_8,  Ti8,  [HasI8],  IIATM>;
def ATMMAX16  : AtomicOp<"atmmax16",  atomic_load_max_16, Ti16, [HasI16], IIATM>;
def ATMMAX32  : AtomicOp<"atmmax32",  atomic_load_max_32, Ti32, [HasI32], IIATM>;
def ATMMAX64  : AtomicOp<"atmmax64",  atomic_load_max_64, Ti64, [HasI64], IIATM>;
def ATMOR8    : AtomicOp<"atmor8",    atomic_load_or_8,   Ti8,  [HasI8],  IIATM>;
def ATMOR16   : AtomicOp<"atmor16",   atomic_load_or_16,  Ti16, [HasI16], IIATM>;
def ATMOR32   : AtomicOp<"atmor32",   atomic_load_or_32,  Ti32, [HasI32], IIATM>;
def ATMOR64   : AtomicOp<"atmor64",   atomic_load_or_64,  Ti64, [HasI64], IIATM>;
def ATMXOR8   : AtomicOp<"atmxor8",   atomic_load_xor_8,  Ti8,  [HasI8],  IIATM>;
def ATMXOR16  : AtomicOp<"atmxor16",  atomic_load_xor_16, Ti16, [HasI16], IIATM>;
def ATMXOR32  : AtomicOp<"atmxor32",  atomic_load_xor_32, Ti32, [HasI32], IIATM>;
def ATMXOR64  : AtomicOp<"atmxor64",  atomic_load_xor_64, Ti64, [HasI64], IIATM>;
def ATMXCHG8  : AtomicOp<"atmxchg8",  atomic_swap_8,      Ti8,  [HasI8],  IIATM>;
def ATMXCHG16 : AtomicOp<"atmxchg16", atomic_swap_16,     Ti16, [HasI16], IIATM>;
def ATMXCHG32 : AtomicOp<"atmxchg32", atomic_swap_32,     Ti32, [HasI32], IIATM>;
def ATMXCHG64 : AtomicOp<"atmxchg64", atomic_swap_64,     Ti64, [HasI64], IIATM>;
def ATMCMPXCHG8  : AtomicOp2<"atmcmpxchg8",  atomic_cmp_swap_8,  Ti8,  [HasI8],  IIATM>;
def ATMCMPXCHG16 : AtomicOp2<"atmcmpxchg16", atomic_cmp_swap_16, Ti16, [HasI16], IIATM>;
def ATMCMPXCHG32 : AtomicOp2<"atmcmpxchg32", atomic_cmp_swap_32, Ti32, [HasI32], IIATM>;
def ATMCMPXCHG64 : AtomicOp2<"atmcmpxchg64", atomic_cmp_swap_64, Ti64, [HasI64], IIATM>;

def OATMAND8   : OAtomicOp<"atmand8",   Ti8,  [HasI8],  IIATM>;
def OATMAND16  : OAtomicOp<"atmand16",  Ti16, [HasI16], IIATM>;
def OATMAND32  : OAtomicOp<"atmand32",  Ti32, [HasI32], IIATM>;
def OATMAND64  : OAtomicOp<"atmand64",  Ti64, [HasI64], IIATM>;
def OATMADD8   : OAtomicOp<"atmadd8",   Ti8,  [HasI8],  IIATM>;
def OATMADD16  : OAtomicOp<"atmadd16",  Ti16, [HasI16], IIATM>;
def OATMADD32  : OAtomicOp<"atmadd32",  Ti32, [HasI32], IIATM>;
def OATMADD64  : OAtomicOp<"atmadd64",  Ti64, [HasI64], IIATM>;
def OATMMIN8   : OAtomicOp<"atmmin8",   Ti8,  [HasI8],  IIATM>;
def OATMMIN16  : OAtomicOp<"atmmin16",  Ti16, [HasI16], IIATM>;
def OATMMIN32  : OAtomicOp<"atmmin32",  Ti32, [HasI32], IIATM>;
def OATMMIN64  : OAtomicOp<"atmmin64",  Ti64, [HasI64], IIATM>;
def OATMMAX8   : OAtomicOp<"atmmax8",   Ti8,  [HasI8],  IIATM>;
def OATMMAX16  : OAtomicOp<"atmmax16",  Ti16, [HasI16], IIATM>;
def OATMMAX32  : OAtomicOp<"atmmax32",  Ti32, [HasI32], IIATM>;
def OATMMAX64  : OAtomicOp<"atmmax64",  Ti64, [HasI64], IIATM>;
def OATMOR8    : OAtomicOp<"atmor8",    Ti8,  [HasI8],  IIATM>;
def OATMOR16   : OAtomicOp<"atmor16",   Ti16, [HasI16], IIATM>;
def OATMOR32   : OAtomicOp<"atmor32",   Ti32, [HasI32], IIATM>;
def OATMOR64   : OAtomicOp<"atmor64",   Ti64, [HasI64], IIATM>;
def OATMXOR8   : OAtomicOp<"atmxor8",   Ti8,  [HasI8],  IIATM>;
def OATMXOR16  : OAtomicOp<"atmxor16",  Ti16, [HasI16], IIATM>;
def OATMXOR32  : OAtomicOp<"atmxor32",  Ti32, [HasI32], IIATM>;
def OATMXOR64  : OAtomicOp<"atmxor64",  Ti64, [HasI64], IIATM>;
def OATMXCHG8  : OAtomicOp<"atmxchg8",  Ti8,  [HasI8],  IIATM>;
def OATMXCHG16 : OAtomicOp<"atmxchg16", Ti16, [HasI16], IIATM>;
def OATMXCHG32 : OAtomicOp<"atmxchg32", Ti32, [HasI32], IIATM>;
def OATMXCHG64 : OAtomicOp<"atmxchg64", Ti64, [HasI64], IIATM>;
def OATMCMPXCHG8  : OAtomicOp2<"atmcmpxchg8",  Ti8,  [HasI8],  IIATM>;
def OATMCMPXCHG16 : OAtomicOp2<"atmcmpxchg16", Ti16, [HasI16], IIATM>;
def OATMCMPXCHG32 : OAtomicOp2<"atmcmpxchg32", Ti32, [HasI32], IIATM>;
def OATMCMPXCHG64 : OAtomicOp2<"atmcmpxchg64", Ti64, [HasI64], IIATM>;

/* // How do we find these?
def ATMADDF32 : AtomicOp<"atmaddf32", atomic_load_add, F32, [HasF32], IIATM>;
def ATMADDF64 : AtomicOp<"atmaddf64", atomic_load_add, F64, [HasF64], IIATM>;

def ATMMINF32 : AtomicOp<"atmminf32", atomic_load_min, F32, [HasF32], IIATM>;
def ATMMINF64 : AtomicOp<"atmminf64", atomic_load_min, F64, [HasF64], IIATM>;

def ATMMAXF32 : AtomicOp<"atmmaxf32", atomic_load_max, F32, [HasF32], IIATM>;
def ATMMAXF64 : AtomicOp<"atmmaxf64", atomic_load_max, F64, [HasF64], IIATM>;
*/

// Unit - type only
def UNIT : PseudoInstLPU<
    (outs),
    (ins UnitOpnd:$immType),
    ".unit\t$immType",
    []>;

// Unit type + index of unit in type only
def UNITI : PseudoInstLPU<
    (outs),
    (ins UnitOpnd:$immType, Operand<i64>:$idx),
    ".unit\t$immType, $idx",
    []>;

// Unit type / allocated - includes coordinate indicies
def UNITA : PseudoInstLPU<
    (outs),
    (ins UnitOpnd:$immType, Operand<i64>:$idx1, Operand<i64>:$idx2),
    ".unit\t$immType, $idx1, $idx2",
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
//include "LPUIntrinsics.td"
