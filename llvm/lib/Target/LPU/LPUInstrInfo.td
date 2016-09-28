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
// - Drop commutable variants and merge with a commutative parameter now that
//   other things are no longer preventing this
// - Possible to merge BinOp and CmpOp (something that can take both a PatFrag
//   and an SDNode?)
// - Use "for" or some other mechanism to deal with 8/16/32/64 redundancy in patterns
// - Merge ADDR_I/R and ADDR_RI/RR similar to RegImm?
// - Why are so many operands in LPUGenInstrInfo.inc unknown

//===----------------------------------------------------------------------===//
// LPU Instruction Predicate Definitions
//===----------------------------------------------------------------------===//

// Maybe predicate for last param of many multiclass?
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
    DAGOperand rcl, DAGOperand immOperand, SDPatternOperator immNode,
    list<Predicate> preds> {
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

  // ImmNode - the operator that should be used to match an immediate
  // of this kind in a pattern (e.g. imm or fpimm)
  SDPatternOperator ImmNode = immNode;

  // Preds - Predicate list associated with type (not yet used by patterns)
  list<Predicate> Preds = preds;
}

// LPU type associations
def Ti0  : LPUTypeInfo<i1,  "0",    I0,  RI0,  CI0,  RCLi0,  i1imm,  imm,   [HasI1]>;
def Ti1  : LPUTypeInfo<i1,  "1",    I1,  RI1,  CI1,  RCLi1,  i1imm,  imm,   [HasI1]>;
def Ti8  : LPUTypeInfo<i8,  "8",    I8,  RI8,  CI8,  RCLi8,  i8imm,  imm,   [HasI8]>;
def Ts8  : LPUTypeInfo<i8,  "s8",   I8,  RI8,  CI8,  RCLi8,  i8imm,  imm,   [HasI8]>;
def Tu8  : LPUTypeInfo<i8,  "u8",   I8,  RI8,  CI8,  RCLi8,  i8imm,  imm,   [HasI8]>;
def Ti16 : LPUTypeInfo<i16, "16",   I16, RI16, CI16, RCLi16, i16imm, imm,   [HasI16]>;
def Ts16 : LPUTypeInfo<i16, "s16",  I16, RI16, CI16, RCLi16, i16imm, imm,   [HasI16]>;
def Tu16 : LPUTypeInfo<i16, "u16",  I16, RI16, CI16, RCLi16, i16imm, imm,   [HasI16]>;
def Tf16 : LPUTypeInfo<f16, "f16",  I16, RI16, CI16, RCLf16, f16imm, fpimm, [HasF16]>;
def Ti32 : LPUTypeInfo<i32, "32",   I32, RI32, CI32, RCLi32, i32imm, imm,   [HasI32]>;
def Ts32 : LPUTypeInfo<i32, "s32",  I32, RI32, CI32, RCLi32, i32imm, imm,   [HasI32]>;
def Tu32 : LPUTypeInfo<i32, "u32",  I32, RI32, CI32, RCLi32, i32imm, imm,   [HasI32]>;
def Tf32 : LPUTypeInfo<f32, "f32",  I32, RI32, CI32, RCLf32, f32imm, fpimm, [HasF32]>;
def Ti64 : LPUTypeInfo<i64, "64",   I64, RI64, CI64, RCLi64, i64imm, imm,   [HasI64]>;
def Ts64 : LPUTypeInfo<i64, "s64",  I64, RI64, CI64, RCLi64, i64imm, imm,   [HasI64]>;
def Tu64 : LPUTypeInfo<i64, "u64",  I64, RI64, CI64, RCLi64, i64imm, imm,   [HasI64]>;
def Tf64 : LPUTypeInfo<f64, "f64",  I64, RI64, CI64, RCLf64, f64imm, fpimm, [HasF64]>;


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

multiclass MovOp<string opStr, RegisterClass RC, ValueType opType,
                 list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs RC:$dst), // Use the more general register class for output
      (ins RC:$op1), // Reg or literal of the appropriate size (e.g. either i32 or f32 for 32b)
      !strconcat(opStr, "\t$dst, $op1"),
      []>;
  }
}

multiclass UnaryOp<string opStr, SDNode opNode, LPUTypeInfo t,
                   list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs t.RC:$dst),
      (ins t.RCL:$op1),
      !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1"),
      [(set t.RC:$dst, (opNode t.RCL:$op1))]>;
  }
}

// Separate pattern because "not" is not an SDNode but a PatFrag
multiclass UnaryOpP<string opStr, PatFrag opNode, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs t.RC:$dst),
      (ins t.RCL:$op1),
      !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1"),
      [(set t.RC:$dst, (opNode t.RCL:$op1))]>;
  }
}

multiclass NegOp<string opStr, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs t.RC:$dst),
      (ins t.RCL:$op1),
      !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1"),
      [(set t.RC:$dst, (sub 0, t.RCL:$op1))]>;
  }
}


// Binary op, no pattern, not commutable
multiclass SExtOp<string opStr, LPUTypeInfo td, LPUTypeInfo ts,
                  list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs td.RC:$dst),
      (ins ts.RCL:$op1, ts.RCL:$op2),
      !strconcat(opStr, td.InstrSuffix, "\t$dst, $op1, $op2"),
      []>;
  }
}

multiclass CvtOp<string opStr, SDNode opNode, LPUTypeInfo td, LPUTypeInfo ts,
           list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs td.RC:$dst),
      (ins ts.RCL:$src),
      !strconcat(opStr, td.InstrSuffix, ts.InstrSuffix, "\t$dst, $src"),
      [(set td.RC:$dst, (opNode ts.RCL:$src))]>;
  }
}

// Handling of commutative
// Currently there are separate commutative "C" and non-commutative "NC"
// classes.  The individual operators use the appropriate multiclass as needed.
// Now that other things have been cleaned up, it should be possible to have
// commutative simply be a parameter and remove the duplication.
//
// Nested multiclasses would be nice - it would allow declaring 4 defs
// per operator, rather than enumerating 8/16/32/64, etc.
// Note: May be able to simplify with "for"

multiclass BinOpC<string opStr, SDNode opNode, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let isCommutable = 1, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs t.RC:$dst),
      (ins t.RCL:$op1, t.RCL:$op2),
      !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1, $op2"),
      [(set t.RC:$dst, (opNode t.RCL:$op1, t.RCL:$op2))]>;
  }
}

multiclass BinOpNC<string opStr, SDNode opNode, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let isCommutable = 0, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs t.RC:$dst),
      (ins t.RCL:$op1, t.RCL:$op2),
      !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1, $op2"),
      [(set t.RC:$dst, (opNode t.RCL:$op1, t.RCL:$op2))]>;
  }
}

// Shift ops specifically have an i8 for the shift amount
multiclass ShiftOp<string opStr, SDNode opNode, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let isCommutable = 0, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs t.RC:$dst),
      (ins t.RCL:$op1, RCLi8:$op2), 
      !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1, $op2"),
      [(set t.RC:$dst, (opNode t.RCL:$op1, RCLi8:$op2))]>;
  }
}

multiclass ShAdd<string opStr, SDNode inner, SDNode outer, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs t.RC:$dst),
      (ins t.RCL:$op1, RCLi8:$op2, t.RCL:$op3),
      !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1, $op2, $op3"),
      [(set t.RC:$dst, (outer (inner t.RCL:$op1, RCLi8:$op2), t.RCL:$op3))]>;
  }
}

multiclass FusedOp1<string opStr, SDNode inner, SDNode outer, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs t.RC:$dst),
      (ins t.RCL:$op1, t.RCL:$op2, t.RCL:$op3),
      !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1, $op2, $op3"),
      [(set t.RC:$dst, (outer (inner t.RCL:$op1, t.RCL:$op2), t.RCL:$op3))]>;
  }
}

multiclass FusedOp2<string opStr, SDNode inner, SDNode outer, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs t.RC:$dst),
      (ins t.RCL:$op1, t.RCL:$op2, t.RCL:$op3),
      !strconcat(opStr, t.InstrSuffix, "\t$dst, $op2, $op3, $op1"),
      [(set t.RC:$dst, (outer t.RCL:$op1, (inner t.RCL:$op2, t.RCL:$op3)))]>;
  }
}

// CmpOp[N]C are identical to BinOp, except opNode is a PatFrag rather than
// an SDNode...
// As with Binop, commutative could be a flag.
multiclass CmpOpC<string opStr, PatFrag opNode, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let isCommutable = 1, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs I1:$dst),
      (ins t.RCL:$op1, t.RCL:$op2),
      !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1, $op2"),
      [(set I1:$dst, (opNode (t.VT t.RCL:$op1), (t.VT t.RCL:$op2)))]>;
  }
}

multiclass CmpOpNC<string opStr, PatFrag opNode, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let isCommutable = 0, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs I1:$dst),
      (ins t.RCL:$op1, t.RCL:$op2),
      !strconcat(opStr, t.InstrSuffix, "\t$dst, $op1, $op2"),
      [(set I1:$dst, (opNode (t.VT t.RCL:$op1), (t.VT t.RCL:$op2)))]>;
  }
}

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

multiclass AtomicOp<string instr_asm, PatFrag opNode, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin = IIATM> {
  let mayLoad = 1, mayStore = 1, hasSideEffects = 1, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN <
      (outs t.RC:$dst),
      (ins ADDR_R:$addr, t.RCL:$op3),
      !strconcat(instr_asm, "\t%ign, $dst, $addr, $op3, %ign\t# NOTE: Generated as unordered."),
      [(set t.VT:$dst, (opNode ADDR_R:$addr, (t.VT t.RCL:$op3)))]>;
  }
}

multiclass OAtomicOp<string instr_asm, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin = IIATM> {
  let mayLoad = 1, mayStore = 1, hasSideEffects = 1, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN <
      (outs t.RC:$dst, I0:$issued),
      (ins ADDR_R:$addr, t.RCL:$op3, I0:$ready),
      !strconcat(instr_asm, "\t$issued, $dst, $addr, $op3, $ready"),
      []>;
  }
}

multiclass AtomicOp2<string instr_asm, PatFrag opNode, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin = IIATM> {
  let mayLoad = 1, mayStore = 1, hasSideEffects = 1, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN <
      (outs t.RC:$dst),
      (ins ADDR_R:$addr, t.RCL:$op3, t.RCL:$op4),
      !strconcat(instr_asm, "\t%ign, $dst, $addr, $op3, $op4, %ign\t# NOTE: Generated as unordered."),
      [(set t.RC:$dst, (opNode ADDR_R:$addr, t.RCL:$op4, t.RCL:$op3))]>;
  }
}

multiclass OAtomicOp2<string instr_asm, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin = IIATM> {
  let mayLoad = 1, mayStore = 1, hasSideEffects = 1, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN <
      (outs t.RC:$dst, I0:$issued),
      (ins ADDR_R:$addr, t.RCL:$op3, t.RCL:$op4, I0:$ready),
      !strconcat(instr_asm, "\t$issued, $dst, $addr, $op3, $op4, $ready"),
      []>;
  }
}

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

defm MOV1      : MovOp<"mov1",  I1,  i1,  [HasI1],  IIALU>;
defm MOV8      : MovOp<"mov8",  I8,  i8,  [HasI8],  IIALU>;
defm MOV16     : MovOp<"mov16", I16, i16, [HasI16], IIALU>;
defm MOV32     : MovOp<"mov32", I32, i32, [HasI32], IIALU>;
defm MOV64     : MovOp<"mov64", I64, i64, [HasI64], IIALU>;

def : Pat<(i64 (LPUWrapper tglobaladdr:$src)),   (MOV64 tglobaladdr:$src)>;
def : Pat<(i64 (LPUWrapper texternalsym:$src)),  (MOV64 texternalsym:$src)>;
def : Pat<(i64 (LPUWrapper tblockaddress:$src)), (MOV64 tblockaddress:$src)>;
def : Pat<(i64 (LPUWrapper tjumptable:$src)),    (MOV64 tjumptable:$src)>;

defm NOT1      : UnaryOpP<"not",    not,   Ti1,  [HasI1],  IIALU>;
defm NOT8      : UnaryOpP<"not",    not,   Ti8,  [HasI8],  IIALU>;
defm NOT16     : UnaryOpP<"not",    not,   Ti16, [HasI16], IIALU>;
defm NOT32     : UnaryOpP<"not",    not,   Ti32, [HasI32], IIALU>;
defm NOT64     : UnaryOpP<"not",    not,   Ti64, [HasI64], IIALU>;
defm NEG8      : NegOp  <"neg",            Ti8,  [HasI8],  IIALU>;
defm NEG16     : NegOp  <"neg",            Ti16, [HasI16], IIALU>;
defm NEG32     : NegOp  <"neg",            Ti32, [HasI32], IIALU>;
defm NEG64     : NegOp  <"neg",            Ti64, [HasI64], IIALU>;
defm NEGF16    : UnaryOp<"neg",     fneg,  Tf16, [HasF16], IIALU>;
defm NEGF32    : UnaryOp<"neg",     fneg,  Tf32, [HasF32], IIALU>;
defm NEGF64    : UnaryOp<"neg",     fneg,  Tf64, [HasF64], IIALU>;
defm ABSF16    : UnaryOp<"abs",     fabs,  Tf16, [HasF16], IIALU>;
defm ABSF32    : UnaryOp<"abs",     fabs,  Tf32, [HasF32], IIALU>;
defm ABSF64    : UnaryOp<"abs",     fabs,  Tf64, [HasF64], IIALU>;
defm SQRTF16   : UnaryOp<"sqrt",    fsqrt, Tf16, [HasF16,HasSqrt], IISqrtF16>;
defm SQRTF32   : UnaryOp<"sqrt",    fsqrt, Tf32, [HasF32,HasSqrt], IISqrtF32>;
defm SQRTF64   : UnaryOp<"sqrt",    fsqrt, Tf64, [HasF64,HasSqrt], IISqrtF64>;
defm EXP2F16   : UnaryOp<"exp2",    fexp2, Tf16, [HasF16,HasMath0], IIMathF16>;
defm EXP2F32   : UnaryOp<"exp2",    fexp2, Tf32, [HasF32,HasMath0], IIMathF32>;
defm EXP2F64   : UnaryOp<"exp2",    fexp2, Tf64, [HasF64,HasMath0], IIMathF64>;
defm LOG2F16   : UnaryOp<"log2",    flog2, Tf16, [HasF16,HasMath0], IIMathF16>;
defm LOG2F32   : UnaryOp<"log2",    flog2, Tf32, [HasF32,HasMath0], IIMathF32>;
defm LOG2F64   : UnaryOp<"log2",    flog2, Tf64, [HasF64,HasMath0], IIMathF64>;
defm EXPF16    : UnaryOp<"exp",     fexp,  Tf16, [HasF16,HasMath0], IIMathF16>;
defm EXPF32    : UnaryOp<"exp",     fexp,  Tf32, [HasF32,HasMath0], IIMathF32>;
defm EXPF64    : UnaryOp<"exp",     fexp,  Tf64, [HasF64,HasMath0], IIMathF64>;
defm LOGF16    : UnaryOp<"log",     flog,  Tf16, [HasF16,HasMath0], IIMathF16>;
defm LOGF32    : UnaryOp<"log",     flog,  Tf32, [HasF32,HasMath0], IIMathF32>;
defm LOGF64    : UnaryOp<"log",     flog,  Tf64, [HasF64,HasMath0], IIMathF64>;
defm SINF16    : UnaryOp<"sin",     fsin,  Tf16, [HasF16,HasMath0], IIMathF16>;
defm SINF32    : UnaryOp<"sin",     fsin,  Tf32, [HasF32,HasMath0], IIMathF32>;
defm SINF64    : UnaryOp<"sin",     fsin,  Tf64, [HasF64,HasMath0], IIMathF64>;
defm COSF16    : UnaryOp<"cos",     fcos,  Tf16, [HasF16,HasMath0], IIMathF16>;
defm COSF32    : UnaryOp<"cos",     fcos,  Tf32, [HasF32,HasMath0], IIMathF32>;
defm COSF64    : UnaryOp<"cos",     fcos,  Tf64, [HasF64,HasMath0], IIMathF64>;
//defm SINCOSF16 : UnaryOp<"sincos", fsincos, Tf16, [HasF16,HasMath0], IIMathF16>;
//defm SINCOSF32 : UnaryOp<"sincos", fsincos, Tf32, [HasF32,HasMath0], IIMathF32>;
//defm SINCOSF64 : UnaryOp<"sincos", fsincos, Tf64, [HasF64,HasMath0], IIMathF64>;

// BitOps. Note that LLVM's SDNodes expect the output type to match the input
// type, while in the simulator all output types are 8-bit.
defm CTPOP8    : UnaryOp<"ctpop",   ctpop, Ti8,  [HasI8,HasBitOp],  IIALU>;
defm CTPOP16   : UnaryOp<"ctpop",   ctpop, Ti16, [HasI16,HasBitOp], IIALU>;
defm CTPOP32   : UnaryOp<"ctpop",   ctpop, Ti32, [HasI32,HasBitOp], IIALU>;
defm CTPOP64   : UnaryOp<"ctpop",   ctpop, Ti64, [HasI64,HasBitOp], IIALU>;
defm CTLZ8     : UnaryOp<"ctlz",    ctlz,  Ti8,  [HasI8,HasBitOp],  IIALU>;
defm CTLZ16    : UnaryOp<"ctlz",    ctlz,  Ti16, [HasI16,HasBitOp], IIALU>;
defm CTLZ32    : UnaryOp<"ctlz",    ctlz,  Ti32, [HasI32,HasBitOp], IIALU>;
defm CTLZ64    : UnaryOp<"ctlz",    ctlz,  Ti64, [HasI64,HasBitOp], IIALU>;
defm CTTZ8     : UnaryOp<"cttz",    cttz,  Ti8,  [HasI8,HasBitOp],  IIALU>;
defm CTTZ16    : UnaryOp<"cttz",    cttz,  Ti16, [HasI16,HasBitOp], IIALU>;
defm CTTZ32    : UnaryOp<"cttz",    cttz,  Ti32, [HasI32,HasBitOp], IIALU>;
defm CTTZ64    : UnaryOp<"cttz",    cttz,  Ti64, [HasI64,HasBitOp], IIALU>;

// Helper fragment to find "parity". Clang's __builtin_parity(x) will result in
// (ctpop(x)&1) in IR, which will be transformed back to parityN(x) by this
// pattern fragment.
def parity : PatFrag<(ops node:$in), (and (ctpop node:$in), 1)>;

defm PARITY8   : UnaryOpP<"parity", parity,Ti8,  [HasI8,HasBitOp],  IIALU>;
defm PARITY16  : UnaryOpP<"parity", parity,Ti16, [HasI16,HasBitOp], IIALU>;
defm PARITY32  : UnaryOpP<"parity", parity,Ti32, [HasI32,HasBitOp], IIALU>;
defm PARITY64  : UnaryOpP<"parity", parity,Ti64, [HasI64,HasBitOp], IIALU>;

// todo: int<=>float Conversions
// first type in convert name is result, second is source type
defm CVTS32F32 : CvtOp<"cvt", fp_to_sint, Ts32, Tf32, [HasI32,HasF32], IICvtIF>;
defm CVTS32F64 : CvtOp<"cvt", fp_to_sint, Ts32, Tf64, [HasI32,HasF64], IICvtIF>;
defm CVTU32F32 : CvtOp<"cvt", fp_to_uint, Tu32, Tf32, [HasI32,HasF32], IICvtIF>;
defm CVTU32F64 : CvtOp<"cvt", fp_to_uint, Tu32, Tf64, [HasI32,HasF64], IICvtIF>;
defm CVTS64F32 : CvtOp<"cvt", fp_to_sint, Ts64, Tf32, [HasI64,HasF32], IICvtIF>;
defm CVTS64F64 : CvtOp<"cvt", fp_to_sint, Ts64, Tf64, [HasI64,HasF64], IICvtIF>;
defm CVTU64F32 : CvtOp<"cvt", fp_to_uint, Tu64, Tf32, [HasI64,HasF32], IICvtIF>;
defm CVTU64F64 : CvtOp<"cvt", fp_to_uint, Tu64, Tf64, [HasI64,HasF64], IICvtIF>;

defm CVTF32S32 : CvtOp<"cvt", sint_to_fp, Tf32, Ts32, [HasF32,HasI32], IICvtFI>;
defm CVTF32S64 : CvtOp<"cvt", sint_to_fp, Tf32, Ts64, [HasF32,HasI64], IICvtFI>;
defm CVTF32U32 : CvtOp<"cvt", uint_to_fp, Tf32, Tu32, [HasF32,HasI32], IICvtFI>;
defm CVTF32U64 : CvtOp<"cvt", uint_to_fp, Tf32, Tu64, [HasF32,HasI64], IICvtFI>;
defm CVTF64S32 : CvtOp<"cvt", sint_to_fp, Tf64, Ts32, [HasF64,HasI32], IICvtFI>;
defm CVTF64S64 : CvtOp<"cvt", sint_to_fp, Tf64, Ts64, [HasF64,HasI64], IICvtFI>;
defm CVTF64U32 : CvtOp<"cvt", uint_to_fp, Tf64, Tu32, [HasF64,HasI32], IICvtFI>;
defm CVTF64U64 : CvtOp<"cvt", uint_to_fp, Tf64, Tu64, [HasF64,HasI64], IICvtFI>;

defm CVTF32F64 : CvtOp<"cvt", fround,   Tf32, Tf64, [HasF32,HasF64], IICvtFF>;
defm CVTF64F32 : CvtOp<"cvt", fextend,  Tf64, Tf32, [HasF32,HasF64], IICvtFF>;

defm SEXT8_1   : SExtOp<"sext", Ti8,  Ti1,  [HasI8],  IIALU>;
defm SEXT8     : SExtOp<"sext", Ti8,  Ti8,  [HasI8],  IIALU>;
defm SEXT16_1  : SExtOp<"sext", Ti16, Ti1,  [HasI16], IIALU>;
defm SEXT16_8  : SExtOp<"sext", Ti16, Ti8,  [HasI16], IIALU>;
defm SEXT16    : SExtOp<"sext", Ti16, Ti16, [HasI16], IIALU>;
defm SEXT32_1  : SExtOp<"sext", Ti32, Ti1,  [HasI32], IIALU>;
defm SEXT32_8  : SExtOp<"sext", Ti32, Ti8,  [HasI32], IIALU>;
defm SEXT32_16 : SExtOp<"sext", Ti32, Ti16, [HasI32], IIALU>;
defm SEXT32    : SExtOp<"sext", Ti32, Ti32, [HasI32], IIALU>;
defm SEXT64_1  : SExtOp<"sext", Ti64, Ti1,  [HasI64], IIALU>;
defm SEXT64_8  : SExtOp<"sext", Ti64, Ti8,  [HasI64], IIALU>;
defm SEXT64_16 : SExtOp<"sext", Ti64, Ti16, [HasI64], IIALU>;
defm SEXT64_32 : SExtOp<"sext", Ti64, Ti32, [HasI64], IIALU>;
defm SEXT64    : SExtOp<"sext", Ti64, Ti64, [HasI64], IIALU>;

defm AND1      : BinOpC< "and", and,  Ti1,  [HasI1],  IIALU>;
defm AND8      : BinOpC< "and", and,  Ti8,  [HasI8],  IIALU>;
defm AND16     : BinOpC< "and", and,  Ti16, [HasI16], IIALU>;
defm AND32     : BinOpC< "and", and,  Ti32, [HasI32], IIALU>;
defm AND64     : BinOpC< "and", and,  Ti64, [HasI64], IIALU>;

defm OR1       : BinOpC< "or",  or,   Ti1,  [HasI1],  IIALU>;
defm OR8       : BinOpC< "or",  or,   Ti8,  [HasI8],  IIALU>;
defm OR16      : BinOpC< "or",  or,   Ti16, [HasI16], IIALU>;
defm OR32      : BinOpC< "or",  or,   Ti32, [HasI32], IIALU>;
defm OR64      : BinOpC< "or",  or,   Ti64, [HasI64], IIALU>;

defm XOR1      : BinOpC< "xor", xor,  Ti1,  [HasI1],  IIALU>;
defm XOR8      : BinOpC< "xor", xor,  Ti8,  [HasI8],  IIALU>;
defm XOR16     : BinOpC< "xor", xor,  Ti16, [HasI16], IIALU>;
defm XOR32     : BinOpC< "xor", xor,  Ti32, [HasI32], IIALU>;
defm XOR64     : BinOpC< "xor", xor,  Ti64, [HasI64], IIALU>;

defm SLL8      : ShiftOp<"sll", shl,  Ti8,  [HasI8],  IIShft>;
defm SLL16     : ShiftOp<"sll", shl,  Ti16, [HasI16], IIShft>;
defm SLL32     : ShiftOp<"sll", shl,  Ti32, [HasI32], IIShft>;
defm SLL64     : ShiftOp<"sll", shl,  Ti64, [HasI64], IIShft>;

defm SRL8      : ShiftOp<"srl", srl,  Ti8,  [HasI8],  IIShft>;
defm SRL16     : ShiftOp<"srl", srl,  Ti16, [HasI16], IIShft>;
defm SRL32     : ShiftOp<"srl", srl,  Ti32, [HasI32], IIShft>;
defm SRL64     : ShiftOp<"srl", srl,  Ti64, [HasI64], IIShft>;

defm SRA8      : ShiftOp<"sra", sra,  Ti8,  [HasI8],  IIShft>;
defm SRA16     : ShiftOp<"sra", sra,  Ti16, [HasI16], IIShft>;
defm SRA32     : ShiftOp<"sra", sra,  Ti32, [HasI32], IIShft>;
defm SRA64     : ShiftOp<"sra", sra,  Ti64, [HasI64], IIShft>;

def : Pat<(i1 (add i1:$op1, i1:$op2)),       (XOR1 $op1,$op2)>;
def : Pat<(i1 (add i1:$op1, (i1 imm:$imm))), (XOR1 $op1,$imm)>;

defm ADD8      : BinOpC< "add", add,  Ti8,  [HasI8],  IIALU>;
defm ADD16     : BinOpC< "add", add,  Ti16, [HasI16], IIALU>;
defm ADD32     : BinOpC< "add", add,  Ti32, [HasI32], IIALU>;
defm ADD64     : BinOpC< "add", add,  Ti64, [HasI64], IIALU>;

defm ADDF16    : BinOpC< "add", fadd, Tf16, [HasF16], IIAddF16>;
defm ADDF32    : BinOpC< "add", fadd, Tf32, [HasF32], IIAddF32>;
defm ADDF64    : BinOpC< "add", fadd, Tf64, [HasF64], IIAddF64>;

def : Pat<(i1 (sub i1:$op1, i1:$op2)),       (XOR1 $op1,$op2)>;
def : Pat<(i1 (sub i1:$op1, (i1 imm:$imm))), (XOR1 $op1,$imm)>;
defm SUB8      : BinOpNC<"sub", sub,  Ti8,  [HasI8],  IIALU>;
defm SUB16     : BinOpNC<"sub", sub,  Ti16, [HasI16], IIALU>;
defm SUB32     : BinOpNC<"sub", sub,  Ti32, [HasI32], IIALU>;
defm SUB64     : BinOpNC<"sub", sub,  Ti64, [HasI64], IIALU>;

defm SUBF16    : BinOpNC<"sub", fsub, Tf16, [HasF16], IIAddF16>;
defm SUBF32    : BinOpNC<"sub", fsub, Tf32, [HasF32], IIAddF32>;
defm SUBF64    : BinOpNC<"sub", fsub, Tf64, [HasF64], IIAddF64>;

defm MUL8      : BinOpC< "mul", mul,  Ti8,  [HasI8],  IIMulI8>;
defm MUL16     : BinOpC< "mul", mul,  Ti16, [HasI16], IIMulI16>;
defm MUL32     : BinOpC< "mul", mul,  Ti32, [HasI32], IIMulI32>;
defm MUL64     : BinOpC< "mul", mul,  Ti64, [HasI64], IIMulI64>;

defm MULF16    : BinOpC< "mul", fmul, Tf16, [HasF16], IIMulF16>;
defm MULF32    : BinOpC< "mul", fmul, Tf32, [HasF32], IIMulF32>;
defm MULF64    : BinOpC< "mul", fmul, Tf64, [HasF64], IIMulF64>;

defm DIVS8     : BinOpNC<"div", sdiv, Ts8,  [HasI8,HasIDiv],  IIDivI8>;
defm DIVS16    : BinOpNC<"div", sdiv, Ts16, [HasI16,HasIDiv], IIDivI16>;
defm DIVS32    : BinOpNC<"div", sdiv, Ts32, [HasI32,HasIDiv], IIDivI32>;
defm DIVS64    : BinOpNC<"div", sdiv, Ts64, [HasI64,HasIDiv], IIDivI64>;

defm DIVU8     : BinOpNC<"div", udiv, Tu8,  [HasI8,HasIDiv],  IIDivI8>;
defm DIVU16    : BinOpNC<"div", udiv, Tu16, [HasI16,HasIDiv], IIDivI16>;
defm DIVU32    : BinOpNC<"div", udiv, Tu32, [HasI32,HasIDiv], IIDivI32>;
defm DIVU64    : BinOpNC<"div", udiv, Tu64, [HasI64,HasIDiv], IIDivI64>;

defm DIVF16    : BinOpNC<"div", fdiv, Tf16, [HasF16,HasFDiv], IIDivF16>;
defm DIVF32    : BinOpNC<"div", fdiv, Tf32, [HasF32,HasFDiv], IIDivF32>;
defm DIVF64    : BinOpNC<"div", fdiv, Tf64, [HasF64,HasFDiv], IIDivF64>;

defm POWF32    : BinOpNC<"pow", fpow, Tf32, [HasF32,HasMath0], IIMathF32>;
defm POWF64    : BinOpNC<"pow", fpow, Tf64, [HasF64,HasMath0], IIMathF64>;

defm CMPLTS8  : CmpOpNC<"cmplt",setlt,  Ts8,  [HasI8],  IIALU>;
defm CMPLTS16 : CmpOpNC<"cmplt",setlt,  Ts16, [HasI16], IIALU>;
defm CMPLTS32 : CmpOpNC<"cmplt",setlt,  Ts32, [HasI32], IIALU>;
defm CMPLTS64 : CmpOpNC<"cmplt",setlt,  Ts64, [HasI64], IIALU>;

defm CMPLTU8  : CmpOpNC<"cmplt",setult, Tu8,  [HasI8],  IIALU>;
defm CMPLTU16 : CmpOpNC<"cmplt",setult, Tu16, [HasI16], IIALU>;
defm CMPLTU32 : CmpOpNC<"cmplt",setult, Tu32, [HasI32], IIALU>;
defm CMPLTU64 : CmpOpNC<"cmplt",setult, Tu64, [HasI64], IIALU>;

defm CMPLES8  : CmpOpNC<"cmple",setle,  Ts8,  [HasI8],  IIALU>;
defm CMPLES16 : CmpOpNC<"cmple",setle,  Ts16, [HasI16], IIALU>;
defm CMPLES32 : CmpOpNC<"cmple",setle,  Ts32, [HasI32], IIALU>;
defm CMPLES64 : CmpOpNC<"cmple",setle,  Ts64, [HasI64], IIALU>;

defm CMPLEU8  : CmpOpNC<"cmple",setule, Tu8,  [HasI8],  IIALU>;
defm CMPLEU16 : CmpOpNC<"cmple",setule, Tu16, [HasI16], IIALU>;
defm CMPLEU32 : CmpOpNC<"cmple",setule, Tu32, [HasI32], IIALU>;
defm CMPLEU64 : CmpOpNC<"cmple",setule, Tu64, [HasI64], IIALU>;

// Note - there is no cmpeq1.  As long as operands are clean, cmpeq8
// suffices
defm CMPEQ8   : CmpOpC<"cmpeq", seteq,  Ti8,  [HasI8],  IIALU>;
defm CMPEQ16  : CmpOpC<"cmpeq", seteq,  Ti16, [HasI16], IIALU>;
defm CMPEQ32  : CmpOpC<"cmpeq", seteq,  Ti32, [HasI32], IIALU>;
defm CMPEQ64  : CmpOpC<"cmpeq", seteq,  Ti64, [HasI64], IIALU>;

defm CMPGTS8  : CmpOpNC<"cmpgt",setgt,  Ts8,  [HasI8],  IIALU>;
defm CMPGTS16 : CmpOpNC<"cmpgt",setgt,  Ts16, [HasI16], IIALU>;
defm CMPGTS32 : CmpOpNC<"cmpgt",setgt,  Ts32, [HasI32], IIALU>;
defm CMPGTS64 : CmpOpNC<"cmpgt",setgt,  Ts64, [HasI64], IIALU>;

defm CMPGTU8  : CmpOpNC<"cmpgt",setugt, Tu8,  [HasI8],  IIALU>;
defm CMPGTU16 : CmpOpNC<"cmpgt",setugt, Tu16, [HasI16], IIALU>;
defm CMPGTU32 : CmpOpNC<"cmpgt",setugt, Tu32, [HasI32], IIALU>;
defm CMPGTU64 : CmpOpNC<"cmpgt",setugt, Tu64, [HasI64], IIALU>;

defm CMPGES8  : CmpOpNC<"cmpge",setge,  Ts8,  [HasI8],  IIALU>;
defm CMPGES16 : CmpOpNC<"cmpge",setge,  Ts16, [HasI16], IIALU>;
defm CMPGES32 : CmpOpNC<"cmpge",setge,  Ts32, [HasI32], IIALU>;
defm CMPGES64 : CmpOpNC<"cmpge",setge,  Ts64, [HasI64], IIALU>;

defm CMPGEU8  : CmpOpNC<"cmpge",setuge, Tu8,  [HasI8],  IIALU>;
defm CMPGEU16 : CmpOpNC<"cmpge",setuge, Tu16, [HasI16], IIALU>;
defm CMPGEU32 : CmpOpNC<"cmpge",setuge, Tu32, [HasI32], IIALU>;
defm CMPGEU64 : CmpOpNC<"cmpge",setuge, Tu64, [HasI64], IIALU>;

def : Pat<(i1 (setne i1:$op1, i1:$op2)), (XOR1 $op1,$op2)>;
def : Pat<(i1 (setne i1:$op1, (i1 imm:$imm))), (XOR1 $op1,$imm)>;
defm CMPNE8   : CmpOpC<"cmpne", setne,  Ti8,  [HasI8],  IIALU>;
defm CMPNE16  : CmpOpC<"cmpne", setne,  Ti16, [HasI16], IIALU>;
defm CMPNE32  : CmpOpC<"cmpne", setne,  Ti32, [HasI32], IIALU>;
defm CMPNE64  : CmpOpC<"cmpne", setne,  Ti64, [HasI64], IIALU>;

defm CMPOLTF16: CmpOpNC<"cmplt",setolt, Tf16, [HasF16], IICmpF>;
defm CMPOLTF32: CmpOpNC<"cmplt",setolt, Tf32, [HasF32], IICmpF>;
defm CMPOLTF64: CmpOpNC<"cmplt",setolt, Tf64, [HasF64], IICmpF>;
defm CMPULTF16: CmpOpNC<"cmplt",setult, Tf16, [HasF16], IICmpF>;
defm CMPULTF32: CmpOpNC<"cmplt",setult, Tf32, [HasF32], IICmpF>;
defm CMPULTF64: CmpOpNC<"cmplt",setult, Tf64, [HasF64], IICmpF>;

defm CMPOLEF16: CmpOpNC<"cmple",setole, Tf16, [HasF16], IICmpF>;
defm CMPOLEF32: CmpOpNC<"cmple",setole, Tf32, [HasF32], IICmpF>;
defm CMPOLEF64: CmpOpNC<"cmple",setole, Tf64, [HasF64], IICmpF>;
defm CMPULEF16: CmpOpNC<"cmple",setule, Tf16, [HasF16], IICmpF>;
defm CMPULEF32: CmpOpNC<"cmple",setule, Tf32, [HasF32], IICmpF>;
defm CMPULEF64: CmpOpNC<"cmple",setule, Tf64, [HasF64], IICmpF>;

defm CMPOEQF16: CmpOpC<"cmpeq", setoeq, Tf16, [HasF16], IICmpF>;
defm CMPOEQF32: CmpOpC<"cmpeq", setoeq, Tf32, [HasF32], IICmpF>;
defm CMPOEQF64: CmpOpC<"cmpeq", setoeq, Tf64, [HasF64], IICmpF>;
defm CMPUEQF16: CmpOpC<"cmpeq", setueq, Tf16, [HasF16], IICmpF>;
defm CMPUEQF32: CmpOpC<"cmpeq", setueq, Tf32, [HasF32], IICmpF>;
defm CMPUEQF64: CmpOpC<"cmpeq", setueq, Tf64, [HasF64], IICmpF>;

defm CMPOGTF16: CmpOpNC<"cmpgt",setogt, Tf16, [HasF16], IICmpF>;
defm CMPOGTF32: CmpOpNC<"cmpgt",setogt, Tf32, [HasF32], IICmpF>;
defm CMPOGTF64: CmpOpNC<"cmpgt",setogt, Tf64, [HasF64], IICmpF>;
defm CMPUGTF16: CmpOpNC<"cmpgt",setugt, Tf16, [HasF16], IICmpF>;
defm CMPUGTF32: CmpOpNC<"cmpgt",setugt, Tf32, [HasF32], IICmpF>;
defm CMPUGTF64: CmpOpNC<"cmpgt",setugt, Tf64, [HasF64], IICmpF>;

defm CMPOGEF16: CmpOpNC<"cmpge",setoge, Tf16, [HasF16], IICmpF>;
defm CMPOGEF32: CmpOpNC<"cmpge",setoge, Tf32, [HasF32], IICmpF>;
defm CMPOGEF64: CmpOpNC<"cmpge",setoge, Tf64, [HasF64], IICmpF>;
defm CMPUGEF16: CmpOpNC<"cmpge",setuge, Tf16, [HasF16], IICmpF>;
defm CMPUGEF32: CmpOpNC<"cmpge",setuge, Tf32, [HasF32], IICmpF>;
defm CMPUGEF64: CmpOpNC<"cmpge",setuge, Tf64, [HasF64], IICmpF>;

defm CMPONEF16: CmpOpC<"cmpne", setone, Tf16, [HasF16], IICmpF>;
defm CMPONEF32: CmpOpC<"cmpne", setone, Tf32, [HasF32], IICmpF>;
defm CMPONEF64: CmpOpC<"cmpne", setone, Tf64, [HasF64], IICmpF>;
defm CMPUNEF16: CmpOpC<"cmpne", setune, Tf16, [HasF16], IICmpF>;
defm CMPUNEF32: CmpOpC<"cmpne", setune, Tf32, [HasF32], IICmpF>;
defm CMPUNEF64: CmpOpC<"cmpne", setune, Tf64, [HasF64], IICmpF>;

defm CMPOF16  : CmpOpC<"cmpo",  seto,   Tf16, [HasF16], IICmpF>;
defm CMPOF32  : CmpOpC<"cmpo",  seto,   Tf32, [HasF32], IICmpF>;
defm CMPOF64  : CmpOpC<"cmpo",  seto,   Tf64, [HasF64], IICmpF>;
defm CMPUOF16 : CmpOpC<"cmpuo", setuo,  Tf16, [HasF16], IICmpF>;
defm CMPUOF32 : CmpOpC<"cmpuo", setuo,  Tf32, [HasF32], IICmpF>;
defm CMPUOF64 : CmpOpC<"cmpuo", setuo,  Tf64, [HasF64], IICmpF>;

multiclass BSelectOp<string opStr, LPUTypeInfo t,
           InstrItinClass itin = NoItinerary> {
  let Itinerary = itin in {
    // BSEL - Only allow constant 1st operand, so CSA network only needs to support
    // 2 operand input boxes.
    def "": FMTGEN<
      (outs t.RC:$dst),
      (ins t.L:$sel, t.RCL:$v0, t.RCL:$v1),
      !strconcat(opStr, t.InstrSuffix, "\t$dst, $sel, $v0, $v1"),
      []>;
  }
}

defm BSEL1  : BSelectOp <"bsel", Ti1>;
defm BSEL8  : BSelectOp <"bsel", Ti8>;
defm BSEL16 : BSelectOp <"bsel", Ti16>;
defm BSEL32 : BSelectOp <"bsel", Ti32>;
defm BSEL64 : BSelectOp <"bsel", Ti64>;

def LAND1 : FMTGEN<
  (outs I1:$dst),
  (ins RCLi1:$op1, RCLi1:$op2, RCLi1:$op3, RCLi1:$op4),
  "land1\t$dst, $op1, $op2, $op3, $op4",
  []> { let Itinerary = IIALU; }

// Shift/add - itinerary depends on constant small shift amt - else IShft...
defm SLADD8  : ShAdd<"sladd", shl,  add,  Ti8,  [HasI8, HasShAdd], IISAdd>;
defm SLADD16 : ShAdd<"sladd", shl,  add,  Ti16, [HasI16,HasShAdd], IISAdd>;
defm SLADD32 : ShAdd<"sladd", shl,  add,  Ti32, [HasI32,HasShAdd], IISAdd>;
defm SLADD64 : ShAdd<"sladd", shl,  add,  Ti64, [HasI64,HasShAdd], IISAdd>;

defm FMAF16  : FusedOp1<"fma",  fmul, fadd, Tf16, [HasF16,HasFMA], IIFMAF16>;
// crude handling of commutative
defm FMAF16x : FusedOp2<"fma",  fmul, fadd, Tf16, [HasF16,HasFMA], IIFMAF16>;
defm FMSF16  : FusedOp1<"fms",  fmul, fsub, Tf16, [HasF16,HasFMA], IIFMAF16>;
defm FMRSF16 : FusedOp2<"fmrs", fmul, fsub, Tf16, [HasF16,HasFMA], IIFMAF16>;

defm FMAF32  : FusedOp1<"fma",  fmul, fadd, Tf32, [HasF32,HasFMA], IIFMAF32>;
// crude handling of commutative
defm FMAF32x : FusedOp2<"fma",  fmul, fadd, Tf32, [HasF32,HasFMA], IIFMAF32>;
defm FMSF32  : FusedOp1<"fms",  fmul, fsub, Tf32, [HasF32,HasFMA], IIFMAF32>;
defm FMRSF32 : FusedOp2<"fmrs", fmul, fsub, Tf32, [HasF32,HasFMA], IIFMAF32>;

defm FMAF64  : FusedOp1<"fma",  fmul, fadd, Tf64, [HasF64,HasFMA], IIFMAF64>;
// crude handling of commutative
defm FMAF64x : FusedOp2<"fma",  fmul, fadd, Tf64, [HasF64,HasFMA], IIFMAF64>;
defm FMSF64  : FusedOp1<"fms",  fmul, fsub, Tf64, [HasF64,HasFMA], IIFMAF64>;
defm FMRSF64 : FusedOp2<"fmrs", fmul, fsub, Tf64, [HasF64,HasFMA], IIFMAF64>;

// Should we do these, or drop and assume sext-at-consumer?
// todo: LDS8
// todo: LDS16
// todo: LDS32
defm LD1   : LdOp<"ld8",   Ti1>;
defm LD8   : LdOp<"ld8",   Ti8>;
defm LD16  : LdOp<"ld16",  Ti16>;
defm LD16f : LdOp<"ld16",  Tf16>;
defm LD32  : LdOp<"ld32",  Ti32>;
defm LD32f : LdOp<"ld32",  Tf32>;
defm LD64  : LdOp<"ld64",  Ti64>;
defm LD64f : LdOp<"ld64",  Tf64>;

defm OLD1   : OLdOp<"ld8",   Ti1>;
defm OLD8   : OLdOp<"ld8",   Ti8>;
defm OLD16  : OLdOp<"ld16",  Ti16>;
defm OLD16f : OLdOp<"ld16",  Tf16>;
defm OLD32  : OLdOp<"ld32",  Ti32>;
defm OLD32f : OLdOp<"ld32",  Tf32>;
defm OLD64  : OLdOp<"ld64",  Ti64>;
defm OLD64f : OLdOp<"ld64",  Tf64>;

defm ST1   : StOp<"st8",   Ti1>;
defm ST8   : StOp<"st8",   Ti8>;
defm ST16  : StOp<"st16",  Ti16>;
defm ST16f : StOp<"st16",  Tf16>;
defm ST32  : StOp<"st32",  Ti32>;
defm ST32f : StOp<"st32",  Tf32>;
defm ST64  : StOp<"st64",  Ti64>;
defm ST64f : StOp<"st64",  Tf64>;

defm OST1   : OStOp<"st8",   Ti1>;
defm OST8   : OStOp<"st8",   Ti8>;
defm OST16  : OStOp<"st16",  Ti16>;
defm OST16f : OStOp<"st16",  Tf16>;
defm OST32  : OStOp<"st32",  Ti32>;
defm OST32f : OStOp<"st32",  Tf32>;
defm OST64  : OStOp<"st64",  Ti64>;
defm OST64f : OStOp<"st64",  Tf64>;

// Atomic operations
defm ATMAND8   : AtomicOp<"atmand8",   atomic_load_and_8,  Ti8,  [HasI8],  IIATM>;
defm ATMAND16  : AtomicOp<"atmand16",  atomic_load_and_16, Ti16, [HasI16], IIATM>;
defm ATMAND32  : AtomicOp<"atmand32",  atomic_load_and_32, Ti32, [HasI32], IIATM>;
defm ATMAND64  : AtomicOp<"atmand64",  atomic_load_and_64, Ti64, [HasI64], IIATM>;
defm ATMADD8   : AtomicOp<"atmadd8",   atomic_load_add_8,  Ti8,  [HasI8],  IIATM>;
defm ATMADD16  : AtomicOp<"atmadd16",  atomic_load_add_16, Ti16, [HasI16], IIATM>;
defm ATMADD32  : AtomicOp<"atmadd32",  atomic_load_add_32, Ti32, [HasI32], IIATM>;
defm ATMADD64  : AtomicOp<"atmadd64",  atomic_load_add_64, Ti64, [HasI64], IIATM>;
defm ATMMIN8   : AtomicOp<"atmmin8",   atomic_load_min_8,  Ti8,  [HasI8],  IIATM>;
defm ATMMIN16  : AtomicOp<"atmmin16",  atomic_load_min_16, Ti16, [HasI16], IIATM>;
defm ATMMIN32  : AtomicOp<"atmmin32",  atomic_load_min_32, Ti32, [HasI32], IIATM>;
defm ATMMIN64  : AtomicOp<"atmmin64",  atomic_load_min_64, Ti64, [HasI64], IIATM>;
defm ATMMAX8   : AtomicOp<"atmmax8",   atomic_load_max_8,  Ti8,  [HasI8],  IIATM>;
defm ATMMAX16  : AtomicOp<"atmmax16",  atomic_load_max_16, Ti16, [HasI16], IIATM>;
defm ATMMAX32  : AtomicOp<"atmmax32",  atomic_load_max_32, Ti32, [HasI32], IIATM>;
defm ATMMAX64  : AtomicOp<"atmmax64",  atomic_load_max_64, Ti64, [HasI64], IIATM>;
defm ATMOR8    : AtomicOp<"atmor8",    atomic_load_or_8,   Ti8,  [HasI8],  IIATM>;
defm ATMOR16   : AtomicOp<"atmor16",   atomic_load_or_16,  Ti16, [HasI16], IIATM>;
defm ATMOR32   : AtomicOp<"atmor32",   atomic_load_or_32,  Ti32, [HasI32], IIATM>;
defm ATMOR64   : AtomicOp<"atmor64",   atomic_load_or_64,  Ti64, [HasI64], IIATM>;
defm ATMXOR8   : AtomicOp<"atmxor8",   atomic_load_xor_8,  Ti8,  [HasI8],  IIATM>;
defm ATMXOR16  : AtomicOp<"atmxor16",  atomic_load_xor_16, Ti16, [HasI16], IIATM>;
defm ATMXOR32  : AtomicOp<"atmxor32",  atomic_load_xor_32, Ti32, [HasI32], IIATM>;
defm ATMXOR64  : AtomicOp<"atmxor64",  atomic_load_xor_64, Ti64, [HasI64], IIATM>;
defm ATMXCHG8  : AtomicOp<"atmxchg8",  atomic_swap_8,      Ti8,  [HasI8],  IIATM>;
defm ATMXCHG16 : AtomicOp<"atmxchg16", atomic_swap_16,     Ti16, [HasI16], IIATM>;
defm ATMXCHG32 : AtomicOp<"atmxchg32", atomic_swap_32,     Ti32, [HasI32], IIATM>;
defm ATMXCHG64 : AtomicOp<"atmxchg64", atomic_swap_64,     Ti64, [HasI64], IIATM>;
defm ATMCMPXCHG8  : AtomicOp2<"atmcmpxchg8",  atomic_cmp_swap_8,  Ti8,  [HasI8],  IIATM>;
defm ATMCMPXCHG16 : AtomicOp2<"atmcmpxchg16", atomic_cmp_swap_16, Ti16, [HasI16], IIATM>;
defm ATMCMPXCHG32 : AtomicOp2<"atmcmpxchg32", atomic_cmp_swap_32, Ti32, [HasI32], IIATM>;
defm ATMCMPXCHG64 : AtomicOp2<"atmcmpxchg64", atomic_cmp_swap_64, Ti64, [HasI64], IIATM>;

defm OATMAND8   : OAtomicOp<"atmand8",   Ti8,  [HasI8],  IIATM>;
defm OATMAND16  : OAtomicOp<"atmand16",  Ti16, [HasI16], IIATM>;
defm OATMAND32  : OAtomicOp<"atmand32",  Ti32, [HasI32], IIATM>;
defm OATMAND64  : OAtomicOp<"atmand64",  Ti64, [HasI64], IIATM>;
defm OATMADD8   : OAtomicOp<"atmadd8",   Ti8,  [HasI8],  IIATM>;
defm OATMADD16  : OAtomicOp<"atmadd16",  Ti16, [HasI16], IIATM>;
defm OATMADD32  : OAtomicOp<"atmadd32",  Ti32, [HasI32], IIATM>;
defm OATMADD64  : OAtomicOp<"atmadd64",  Ti64, [HasI64], IIATM>;
defm OATMMIN8   : OAtomicOp<"atmmin8",   Ti8,  [HasI8],  IIATM>;
defm OATMMIN16  : OAtomicOp<"atmmin16",  Ti16, [HasI16], IIATM>;
defm OATMMIN32  : OAtomicOp<"atmmin32",  Ti32, [HasI32], IIATM>;
defm OATMMIN64  : OAtomicOp<"atmmin64",  Ti64, [HasI64], IIATM>;
defm OATMMAX8   : OAtomicOp<"atmmax8",   Ti8,  [HasI8],  IIATM>;
defm OATMMAX16  : OAtomicOp<"atmmax16",  Ti16, [HasI16], IIATM>;
defm OATMMAX32  : OAtomicOp<"atmmax32",  Ti32, [HasI32], IIATM>;
defm OATMMAX64  : OAtomicOp<"atmmax64",  Ti64, [HasI64], IIATM>;
defm OATMOR8    : OAtomicOp<"atmor8",    Ti8,  [HasI8],  IIATM>;
defm OATMOR16   : OAtomicOp<"atmor16",   Ti16, [HasI16], IIATM>;
defm OATMOR32   : OAtomicOp<"atmor32",   Ti32, [HasI32], IIATM>;
defm OATMOR64   : OAtomicOp<"atmor64",   Ti64, [HasI64], IIATM>;
defm OATMXOR8   : OAtomicOp<"atmxor8",   Ti8,  [HasI8],  IIATM>;
defm OATMXOR16  : OAtomicOp<"atmxor16",  Ti16, [HasI16], IIATM>;
defm OATMXOR32  : OAtomicOp<"atmxor32",  Ti32, [HasI32], IIATM>;
defm OATMXOR64  : OAtomicOp<"atmxor64",  Ti64, [HasI64], IIATM>;
defm OATMXCHG8  : OAtomicOp<"atmxchg8",  Ti8,  [HasI8],  IIATM>;
defm OATMXCHG16 : OAtomicOp<"atmxchg16", Ti16, [HasI16], IIATM>;
defm OATMXCHG32 : OAtomicOp<"atmxchg32", Ti32, [HasI32], IIATM>;
defm OATMXCHG64 : OAtomicOp<"atmxchg64", Ti64, [HasI64], IIATM>;
defm OATMCMPXCHG8  : OAtomicOp2<"atmcmpxchg8",  Ti8,  [HasI8],  IIATM>;
defm OATMCMPXCHG16 : OAtomicOp2<"atmcmpxchg16", Ti16, [HasI16], IIATM>;
defm OATMCMPXCHG32 : OAtomicOp2<"atmcmpxchg32", Ti32, [HasI32], IIATM>;
defm OATMCMPXCHG64 : OAtomicOp2<"atmcmpxchg64", Ti64, [HasI64], IIATM>;

/* // How do we find these?
defm ATMADDF32 : AtomicOp<"atmaddf32", atomic_load_add, F32, [HasF32], IIATM>;
defm ATMADDF64 : AtomicOp<"atmaddf64", atomic_load_add, F64, [HasF64], IIATM>;

defm ATMMINF32 : AtomicOp<"atmminf32", atomic_load_min, F32, [HasF32], IIATM>;
defm ATMMINF64 : AtomicOp<"atmminf64", atomic_load_min, F64, [HasF64], IIATM>;

defm ATMMAXF32 : AtomicOp<"atmmaxf32", atomic_load_max, F32, [HasF32], IIATM>;
defm ATMMAXF64 : AtomicOp<"atmmaxf64", atomic_load_max, F64, [HasF64], IIATM>;
*/

multiclass CopyOp<string opStr, LPUTypeInfo t,
           InstrItinClass itin = IIVir> {
  let Itinerary = itin in {
    def "" : FMTGEN<
      (outs t.RC:$op0, t.RC:$op1, t.RC:$op2, t.RC:$op3),
      (ins t.RCL:$op4),
      !strconcat(opStr, t.InstrSuffix, "\t$op0, $op1, $op2, $op3, $op4"),
      []>;
  }
}
defm COPY0   : CopyOp<"copy", Ti0>;
defm COPY1   : CopyOp<"copy", Ti1>;
defm COPY8   : CopyOp<"copy", Ti8>;
defm COPY16  : CopyOp<"copy", Ti16>;
defm COPY32  : CopyOp<"copy", Ti32>;
defm COPY64  : CopyOp<"copy", Ti64>;

multiclass MergeOp<string opStr, LPUTypeInfo t,
                   list<Predicate> preds, InstrItinClass itin> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs t.RC:$dst),
      (ins I1:$sel, t.RCL:$v0, t.RCL:$v1),
      !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
      [(set t.VT:$dst, (select I1:$sel, (t.VT t.RCL:$v1), (t.VT t.RCL:$v0)))]>;
  }
}
defm MERGE1   : MergeOp<"merge1",  Ti1,  [HasI1],  IIVir>;
defm MERGE8   : MergeOp<"merge8",  Ti8,  [HasI8],  IIVir>;
defm MERGE16  : MergeOp<"merge16", Ti16, [HasI16], IIVir>;
defm MERGE16f : MergeOp<"merge16", Tf16, [HasI16], IIVir>;
defm MERGE32  : MergeOp<"merge32", Ti32, [HasI32], IIVir>;
defm MERGE32f : MergeOp<"merge32", Tf32, [HasI32], IIVir>;
defm MERGE64  : MergeOp<"merge64", Ti64, [HasI64], IIVir>;
defm MERGE64f : MergeOp<"merge64", Tf64, [HasI64], IIVir>;

multiclass SwitchOp<string opStr, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin> {
  let Itinerary = itin in {
    def "" : FMTGEN<
      (outs t.RC:$dst0, t.RC:$dst1),
      (ins I1:$sel, t.RCL:$v),
      !strconcat(opStr, "\t$dst0, $dst1, $sel, $v"),
      []>;
  }
}
defm SWITCH1   : SwitchOp<"switch1",  Ti1,  [HasI1],  IIVir>;
defm SWITCH8   : SwitchOp<"switch8",  Ti8,  [HasI8],  IIVir>;
defm SWITCH16  : SwitchOp<"switch16", Ti16, [HasI16], IIVir>;
defm SWITCH32  : SwitchOp<"switch32", Ti32, [HasI32], IIVir>;
defm SWITCH64  : SwitchOp<"switch64", Ti64, [HasI64], IIVir>;

multiclass SwitchAnyOp<string opStr, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs t.RC:$dst0, t.RC:$dst1, I1:$sel),
      (ins t.RCL:$v),
      !strconcat(opStr, "\t$dst0, $dst1, $sel, $v"),
      []>;
  }
}
defm SWITCHANY1  : SwitchAnyOp<"switchany1",  Ti1,  [HasI1],  IIVir>;
defm SWITCHANY8  : SwitchAnyOp<"switchany8",  Ti8,  [HasI8],  IIVir>;
defm SWITCHANY16 : SwitchAnyOp<"switchany16", Ti16, [HasI16], IIVir>;
defm SWITCHANY32 : SwitchAnyOp<"switchany32", Ti32, [HasI32], IIVir>;
defm SWITCHANY64 : SwitchAnyOp<"switchany64", Ti64, [HasI64], IIVir>;

multiclass PickOp<string opStr, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin> {
  let Itinerary = itin in {
    def "" : FMTGEN<
      (outs t.RC:$dst),
      (ins I1:$sel, t.RCL:$v0, t.RCL:$v1),
      !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
      []>;
  }
}
defm PICK1   : PickOp<"pick1",  Ti1,  [HasI1],  IIVir>;
defm PICK8   : PickOp<"pick8",  Ti8,  [HasI8],  IIVir>;
defm PICK16  : PickOp<"pick16", Ti16, [HasI16], IIVir>;
defm PICK32  : PickOp<"pick32", Ti32, [HasI32], IIVir>;
defm PICK64  : PickOp<"pick64", Ti64, [HasI64], IIVir>;

multiclass PickAnyOp<string opStr, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs t.RC:$dst, I1:$sel),
      (ins t.RCL:$v0, t.RCL:$v1),
      !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
      []>;
  }
}
defm PICKANY1  : PickAnyOp<"pickany1",  Ti1,  [HasI1],  IIVir>;
defm PICKANY8  : PickAnyOp<"pickany8",  Ti8,  [HasI8],  IIVir>;
defm PICKANY16 : PickAnyOp<"pickany16", Ti16, [HasI16], IIVir>;
defm PICKANY32 : PickAnyOp<"pickany32", Ti32, [HasI32], IIVir>;
defm PICKANY64 : PickAnyOp<"pickany64", Ti64, [HasI64], IIVir>;

def ANY0 : FMTGEN<
  (outs I8:$dst),
  (ins RCLi0:$op1, RCLi0:$op2, RCLi0:$op3, RCLi0:$op4),
  "any0\t$dst, $op1, $op2, $op3, $op4",
  []> { let Itinerary = IIVir; }

def ALL0 : FMTGEN<
  (outs I0:$dst),
  (ins RCLi0:$op1, RCLi0:$op2, RCLi0:$op3, RCLi0:$op4),
  "all0\t$dst, $op1, $op2, $op3, $op4",
  []> { let Itinerary = IIVir; }

def ONCOUNT0 : FMTGEN<  // TODO: something to reflect state
  (outs I0:$dst),
  (ins RCLi64:$cnt, RCLi0:$op2, RCLi0:$op3, RCLi0:$op4, RCLi0:$op5),
  "oncount0\t$dst, $cnt, $op2, $op3, $op4, $op5",
  []> { let Itinerary = IIVir; }

def PREDPROP : FMTGEN<
  (outs I1:$efalse, I1:$etrue),
  (ins RCLi1:$pb, RCLi1:$sb),
  "predprop\t$efalse, $etrue, $pb, $sb",
  []> { let Itinerary = IIALU; }

def PREDMERGE : FMTGEN<
  (outs I1:$predres, I1:$index),
  (ins RCLi1:$e0, RCLi1:$e1),
  "predprop\t$predres, $index, $e0, $e1",
  []> { let Itinerary = IIALU; }

multiclass SeqCOp<string opStr, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin> {
  // hasSideEffects attempts to capture the internal state and stream output behavior
  let hasSideEffects = 1, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<  // TODO: something to reflect state
      (outs t.RC:$val, I1:$pred, I1:$first, I1:$last),
      (ins t.RCL:$base, t.RCL:$count, t.RCL:$stride),
      !strconcat(opStr, "\t$val, $pred, $first, $last, $base, $count, $stride"),
      []>;
  }
}

defm SEQC8    : SeqCOp<"seqc8",    Ti8,  [HasI8],  IIALU>;
defm SEQC16   : SeqCOp<"seqc16",   Ti16, [HasI16], IIALU>;
defm SEQC32   : SeqCOp<"seqc32",   Ti32, [HasI32], IIALU>;
defm SEQC64   : SeqCOp<"seqc64",   Ti64, [HasI64], IIALU>;

multiclass SeqSOp<string opStr, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin> {
  // hasSideEffects attempts to capture the internal state and stream output behavior
  let hasSideEffects = 1, Predicates = preds, Itinerary = itin in {
    // TODO: something to reflect state
    def "" : FMTGEN<
      (outs t.RC:$val, I1:$pred, I1:$first, I1:$last),
      (ins t.RCL:$base, t.RCL:$bound, t.RCL:$stride),
      !strconcat(opStr, "\t$val, $pred, $first, $last, $base, $bound, $stride"),
      []>;
  }
}

defm SEQLTS8  : SeqSOp<"seqlts8",  Ti8,  [HasI8],  IIALU>;
defm SEQLTS16 : SeqSOp<"seqlts16", Ti16, [HasI16], IIALU>;
defm SEQLTS32 : SeqSOp<"seqlts32", Ti32, [HasI32], IIALU>;
defm SEQLTS64 : SeqSOp<"seqlts64", Ti64, [HasI64], IIALU>;

defm SEQLTU8  : SeqSOp<"seqltu8",  Ti8,  [HasI8],  IIALU>;
defm SEQLTU16 : SeqSOp<"seqltu16", Ti16, [HasI16], IIALU>;
defm SEQLTU32 : SeqSOp<"seqltu32", Ti32, [HasI32], IIALU>;
defm SEQLTU64 : SeqSOp<"seqltu64", Ti64, [HasI64], IIALU>;

defm SEQLES8  : SeqSOp<"seqles8",  Ti8,  [HasI8],  IIALU>;
defm SEQLES16 : SeqSOp<"seqles16", Ti16, [HasI16], IIALU>;
defm SEQLES32 : SeqSOp<"seqles32", Ti32, [HasI32], IIALU>;
defm SEQLES64 : SeqSOp<"seqles64", Ti64, [HasI64], IIALU>;

defm SEQLEU8  : SeqSOp<"seqleu8",  Ti8,  [HasI8],  IIALU>;
defm SEQLEU16 : SeqSOp<"seqleu16", Ti16, [HasI16], IIALU>;
defm SEQLEU32 : SeqSOp<"seqleu32", Ti32, [HasI32], IIALU>;
defm SEQLEU64 : SeqSOp<"seqleu64", Ti64, [HasI64], IIALU>;

defm SEQNE8   : SeqSOp<"seqne8",   Ti8,  [HasI8],  IIALU>;
defm SEQNE16  : SeqSOp<"seqne16",  Ti16, [HasI16], IIALU>;
defm SEQNE32  : SeqSOp<"seqne32",  Ti32, [HasI32], IIALU>;
defm SEQNE64  : SeqSOp<"seqne64",  Ti64, [HasI64], IIALU>;

defm SEQGTS8  : SeqSOp<"seqgts8",  Ti8,  [HasI8],  IIALU>;
defm SEQGTS16 : SeqSOp<"seqgts16", Ti16, [HasI16], IIALU>;
defm SEQGTS32 : SeqSOp<"seqgts32", Ti32, [HasI32], IIALU>;
defm SEQGTS64 : SeqSOp<"seqgts64", Ti64, [HasI64], IIALU>;

defm SEQGTU8  : SeqSOp<"seqgtu8",  Ti8,  [HasI8],  IIALU>;
defm SEQGTU16 : SeqSOp<"seqgtu16", Ti16, [HasI16], IIALU>;
defm SEQGTU32 : SeqSOp<"seqgtu32", Ti32, [HasI32], IIALU>;
defm SEQGTU64 : SeqSOp<"seqgtu64", Ti64, [HasI64], IIALU>;

defm SEQGES8  : SeqSOp<"seqges8",  Ti8,  [HasI8],  IIALU>;
defm SEQGES16 : SeqSOp<"seqges16", Ti16, [HasI16], IIALU>;
defm SEQGES32 : SeqSOp<"seqges32", Ti32, [HasI32], IIALU>;
defm SEQGES64 : SeqSOp<"seqges64", Ti64, [HasI64], IIALU>;

defm SEQGEU8  : SeqSOp<"seqgeu8",  Ti8,  [HasI8],  IIALU>;
defm SEQGEU16 : SeqSOp<"seqgeu16", Ti16, [HasI16], IIALU>;
defm SEQGEU32 : SeqSOp<"seqgeu32", Ti32, [HasI32], IIALU>;
defm SEQGEU64 : SeqSOp<"seqgeu64", Ti64, [HasI64], IIALU>;

multiclass RepeatOp<string opStr, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin> {
  // hasSideEffects attempts to capture the internal state and stream output behavior
  let hasSideEffects = 1, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<  // TODO: something to reflect state
      (outs I64:$out),
      (ins I1:$pred, t.RCL:$in),
      !strconcat(opStr, "\t$out, $pred, $in"),
      []>;
  }
}

defm REPEAT8  : RepeatOp<"repeat8",  Ti8,  [HasI8],  IIALU>;
defm REPEAT16 : RepeatOp<"repeat16", Ti16, [HasI16], IIALU>;
defm REPEAT32 : RepeatOp<"repeat32", Ti32, [HasI32], IIALU>;
defm REPEAT64 : RepeatOp<"repeat64", Ti64, [HasI64], IIALU>;

multiclass StrideOp<string opStr, LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin> {
  // hasSideEffects attempts to capture the internal state and stream output behavior
  let hasSideEffects = 1, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<  // TODO: something to reflect state
      (outs I64:$out),
      (ins I1:$pred, t.RCL:$base, t.RCL:$stride),
      !strconcat(opStr, "\t$out, $pred, $base, $stride"),
      []>;
  }
}

defm STRIDE8  : StrideOp<"stride8",  Ti8,  [HasI8],  IIALU>;
defm STRIDE16 : StrideOp<"stride16", Ti16, [HasI16], IIALU>;
defm STRIDE32 : StrideOp<"stride32", Ti32, [HasI32], IIALU>;
defm STRIDE64 : StrideOp<"stride64", Ti64, [HasI64], IIALU>;

// TBD(jsukha): This info probably needs predicates
// Also, the input can probably be an I0 channel.
def ONEND : FMTGEN<  // TODO: something to reflect state
  (outs I0:$dst),
  (ins I1:$ctrl, I0:$in),
  "onend\t$dst, $ctrl, $in",
  []> { let Itinerary = IIVir; }
  

multiclass Init<LPUTypeInfo t,
           list<Predicate> preds, InstrItinClass itin> {
  // Not clear that "hasSideEffects" is a good description of static initialization...
  let hasSideEffects = 1, Predicates = preds, Itinerary = itin in {
    def "" : PseudoInstLPU<
      (outs t.RC:$dst),
      (ins t.L:$imm),
      ".curr\t$dst;\t.value $imm;\t.avail 0",
      []>;
  }
}

defm INIT0  : Init<Ti0,  [HasI1],  IIVir>;
defm INIT1  : Init<Ti1,  [HasI1],  IIVir>;
defm INIT8  : Init<Ti8,  [HasI8],  IIVir>;
defm INIT16 : Init<Ti16, [HasI16], IIVir>;
defm INIT32 : Init<Ti32, [HasI32], IIVir>;
defm INIT64 : Init<Ti64, [HasI64], IIVir>;

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
def: Pat<(i1 imm:$imm),  (MOV1 imm:$imm)>;
def: Pat<(i8 imm:$imm),  (MOV8 imm:$imm)>;
def: Pat<(i16 imm:$imm), (MOV16 imm:$imm)>;
def: Pat<(f16 fpimm:$imm), (MOV16 fpimm:$imm)>;
def: Pat<(i32 imm:$imm), (MOV32 imm:$imm)>;
def: Pat<(f32 fpimm:$imm), (MOV32 fpimm:$imm)>;
def: Pat<(i64 imm:$imm), (MOV64 imm:$imm)>;
def: Pat<(f64 fpimm:$imm), (MOV64 fpimm:$imm)>;

// sext/zext
def : Pat<(i64 (sext i32:$op1)),
  (SEXT64_32 (i32 $op1), 32)>;
def : Pat<(i64 (sext i16:$op1)),
  (SEXT64_16 (i16 $op1), 16)>;
def : Pat<(i64 (sext i8:$op1)),
  (SEXT64_8  (i8 $op1),  8)>;
def : Pat<(i64 (sext i1:$op1)),
  (SEXT64_1  (i1 $op1),  1)>;

def : Pat<(i32 (sext i16:$op1)),
  (SEXT32_16 (i16 $op1), 16)>;
def : Pat<(i32 (sext i8:$op1)),
  (SEXT32_8 (i8 $op1), 8)>;
def : Pat<(i32 (sext i1:$op1)),
  (SEXT32_1 (i1 $op1), 1)>;

def : Pat<(i16 (sext i8:$op1)),
  (SEXT16_8 (i8 $op1), 8)>;
def : Pat<(i16 (sext i1:$op1)),
  (SEXT16_1 (i1 $op1), 1)>;

def : Pat<(i8 (sext i1:$op1)),
  (SEXT8_1 (i1 $op1), 1)>;

// sext_inreg are the same
def : Pat<(i64 (sext_inreg i64:$op1, i32)),
  (SEXT64 $op1, 32)>;
def : Pat<(i64 (sext_inreg i64:$op1, i16)),
  (SEXT64 $op1, 16)>;
def : Pat<(i64 (sext_inreg i64:$op1, i8)),
  (SEXT64 $op1, 8)>;
def : Pat<(i64 (sext_inreg i64:$op1, i1)),
  (SEXT64 $op1, 1)>;

def : Pat<(i32 (sext_inreg i32:$op1, i16)),
  (SEXT32 $op1, 16)>;
def : Pat<(i32 (sext_inreg i32:$op1, i8)),
  (SEXT32 $op1, 8)>;
def : Pat<(i32 (sext_inreg i32:$op1, i1)),
  (SEXT32 $op1, 1)>;

def : Pat<(i16 (sext_inreg i16:$op1, i8)),
  (SEXT16 $op1, 8)>;
def : Pat<(i16 (sext_inreg i16:$op1, i1)),
  (SEXT16 $op1, 1)>;

def : Pat<(i8 (sext_inreg i8:$op1, i1)),
  (SEXT8 $op1, 1)>;

// zext patterns
// (Is the copy even necessary?  Or can we just return the value?)
// (Or - do we need an explicit mask.  If the incoming value is in range,
// it shouldn't need to be masked...)

def : Pat<(i64 (zext i32:$op1)),
  (COPY_TO_REGCLASS $op1, I64)>;
//  (AND64 $op1, 0xFFFFFFFF)>;  // gets "Type inference contradiction found,
                                // merging 'i32' into 'i64'
def : Pat<(i64 (zext i16:$op1)),
  (COPY_TO_REGCLASS $op1, I64)>;
//  (AND64 $op1, 0xFFFF)>;
def : Pat<(i64 (zext i8:$op1)),
  (COPY_TO_REGCLASS $op1, I64)>;
//  (AND64 $op1, 0xFF)>;
def : Pat<(i64 (zext i1:$op1)),
  (COPY_TO_REGCLASS $op1, I64)>;
//  (AND64 $op1, 1)>;

def : Pat<(i32 (zext i16:$op1)),
  (COPY_TO_REGCLASS $op1, I32)>;
//  (AND32 $op1, 0xFFFF)>;
def : Pat<(i32 (zext i8:$op1)),
  (COPY_TO_REGCLASS $op1, I32)>;
//  (AND32 $op1, 0xFF)>;
def : Pat<(i32 (zext i1:$op1)),
  (COPY_TO_REGCLASS $op1, I32)>;
//  (AND32 $op1, 1)>;

def : Pat<(i16 (zext i8:$op1)),
  (COPY_TO_REGCLASS $op1, I16)>;
//  (AND16 $op1, 0xFF)>;
def : Pat<(i16 (zext i1:$op1)),
  (COPY_TO_REGCLASS $op1, I16)>;
//  (AND16 $op1, 1)>;

def : Pat<(i8 (zext i1:$op1)),
  (COPY_TO_REGCLASS $op1, I8)>;
//  (AND8 $op1, 1)>;

// anyext are treated as zext
def : Pat<(i64 (anyext i32:$op1)),
  (COPY_TO_REGCLASS $op1, I64)>;
def : Pat<(i64 (anyext i16:$op1)),
  (COPY_TO_REGCLASS $op1, I64)>;
def : Pat<(i64 (anyext i8:$op1)),
  (COPY_TO_REGCLASS $op1, I64)>;
def : Pat<(i64 (anyext i1:$op1)),
  (COPY_TO_REGCLASS $op1, I64)>;

def : Pat<(i32 (anyext i16:$op1)),
  (COPY_TO_REGCLASS $op1, I32)>;
def : Pat<(i32 (anyext i8:$op1)),
  (COPY_TO_REGCLASS $op1, I32)>;
def : Pat<(i32 (anyext i1:$op1)),
  (COPY_TO_REGCLASS $op1, I32)>;

def : Pat<(i16 (anyext i8:$op1)),
  (COPY_TO_REGCLASS $op1, I16)>;
def : Pat<(i16 (anyext i1:$op1)),
  (COPY_TO_REGCLASS $op1, I16)>;

def : Pat<(i8 (anyext i1:$op1)),
  (COPY_TO_REGCLASS $op1, I8)>;

// Truncate
def : Pat<(i32 (trunc i64:$op1)),
    (COPY_TO_REGCLASS $op1, I32)>;
def : Pat<(i16 (trunc i64:$op1)),
    (COPY_TO_REGCLASS $op1, I16)>;
def : Pat<(i8 (trunc i64:$op1)),
    (COPY_TO_REGCLASS $op1, I8)>;
def : Pat<(i1 (trunc i64:$op1)),
    (COPY_TO_REGCLASS $op1, I1)>;

def : Pat<(i16 (trunc i32:$op1)),
    (COPY_TO_REGCLASS $op1, I16)>;
def : Pat<(i8 (trunc i32:$op1)),
    (COPY_TO_REGCLASS $op1, I8)>;
def : Pat<(i1 (trunc i32:$op1)),
    (COPY_TO_REGCLASS $op1, I1)>;

def : Pat<(i8 (trunc i16:$op1)),
    (COPY_TO_REGCLASS $op1, I8)>;
def : Pat<(i1 (trunc i16:$op1)),
    (COPY_TO_REGCLASS $op1, I1)>;

def : Pat<(i1 (trunc i8:$op1)),
    (COPY_TO_REGCLASS $op1, I1)>;


// bitconvert (shows up in exp)
def : Pat<(i64 (bitconvert f64:$op1)),
    (COPY_TO_REGCLASS $op1, I64)>;
def : Pat<(f64 (bitconvert i64:$op1)),
    (COPY_TO_REGCLASS $op1, I64)>;

def : Pat<(i32 (bitconvert f32:$op1)),
    (COPY_TO_REGCLASS $op1, I32)>;
def : Pat<(f32 (bitconvert i32:$op1)),
    (COPY_TO_REGCLASS $op1, I32)>;

def : Pat<(i16 (bitconvert f16:$op1)),
    (COPY_TO_REGCLASS $op1, I16)>;
def : Pat<(f16 (bitconvert i16:$op1)),
    (COPY_TO_REGCLASS $op1, I16)>;

// fcopysign - use the sign bit from the sign opnd, and everything else from the other
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
