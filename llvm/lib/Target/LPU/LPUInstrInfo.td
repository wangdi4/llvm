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

def flog : SDNode<"ISD::FLOG", SDTFPUnaryOp>;
def fexp : SDNode<"ISD::FEXP", SDTFPUnaryOp>;

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
      (outs RC:$dst),
      (ins RC:$op1),
      !strconcat(opStr, "\t$dst, $op1"),
      []>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins Operand<opType>:$imm),
      !strconcat(opStr, "\t$dst, $imm"),
      []>;
  }
}

multiclass UnaryOp<string opStr, SDNode opNode, RegisterClass RC,
                   list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1),
      !strconcat(opStr, "\t$dst, $op1"),
      [(set RC:$dst, (opNode RC:$op1))]>;
  }
}

// Separate pattern because "not" is not an SDNode but a PatFrag
multiclass UnaryOpP<string opStr, PatFrag opNode, RegisterClass RC,
           list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1),
      !strconcat(opStr, "\t$dst, $op1"),
      [(set RC:$dst, (opNode RC:$op1))]>;
  }
}

multiclass NegOp<string opStr, RegisterClass RC, list<Predicate> preds,
           InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1),
      !strconcat(opStr, "\t$dst, $op1"),
      [(set RC:$dst, (sub 0, RC:$op1))]>;
  }
}

// Binary op, no pattern, not commutable
multiclass SExtOp<string opStr, RegisterClass RCt, RegisterClass RCs,
                  list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs RCt:$dst),
      (ins RCs:$op1, RCs:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      []>;
  }
}

multiclass CvtOp<string opStr, SDNode opNode, RegisterClass DT,
  RegisterClass ST, list<Predicate> preds, InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs DT:$dst),
      (ins ST:$src),
      !strconcat(opStr, "\t$dst, $src"),
      [(set DT:$dst, (opNode ST:$src))]>;
  }
}

// Handling of commutative
// Currently there are separate commutative "C" and non-commutative "NC"
// classes.  The individual operators use the appropriate multiclass as needed.
// In theory there could just be a single BinOp class.  In that case, you
// would get warnings for the "i1" suffix pattern, which would lead you to not
// have operators marked commutative at all.  Given it isn't clear what negative
// impact that might have, we simply have BinOpC and BinOpNC classes for now.

// Nested multiclasses would be nice - it would allow declaring 4 defs
// per operator, rather than enumerating 8/16/32/64, etc.
multiclass BinOpC<string opStr, SDNode opNode, RegisterClass RC,
           ValueType opType, SDNode immNode, list<Predicate> preds,
           InstrItinClass itin = NoItinerary> {
  let isCommutable = 1, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set RC:$dst, (opNode RC:$op1, RC:$op2))]>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, Operand<opType>:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set RC:$dst, (opNode RC:$op1, (opType immNode:$imm)))]>;
  }
}

multiclass BinOpNC<string opStr, SDNode opNode, RegisterClass RC,
           ValueType opType, SDNode immNode, list<Predicate> preds,
           InstrItinClass itin = NoItinerary> {
  let isCommutable = 0, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set RC:$dst, (opNode RC:$op1, RC:$op2))]>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, Operand<opType>:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set RC:$dst, (opNode RC:$op1, (opType immNode:$imm)))]>;
    // Only difference with BinOpC is this additional pattern, allowing
    // literal operation for the 1st of the 2 binary operands
    def i1 : FMTGEN<
      (outs RC:$dst),
      (ins Operand<opType>:$imm, RC:$op2),
      !strconcat(opStr, "\t$dst, $imm, $op2"),
      [(set RC:$dst, (opNode (opType immNode:$imm), RC:$op2))]>;
  }
}

multiclass ShiftOp<string opStr, SDNode opNode, RegisterClass RC,
           ValueType opType, SDNode immNode, list<Predicate> preds,
           InstrItinClass itin = NoItinerary> {
  let isCommutable = 0, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, I8:$op2), 
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set RC:$dst, (opNode RC:$op1, I8:$op2))]>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, Operand<i8>:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set RC:$dst, (opNode RC:$op1, (i8 immNode:$imm)))]>;
    // Only difference with BinOpC is this additional pattern, allowing
    // literal operation for the 1st of the 2 binary operands
    def i1 : FMTGEN<
      (outs RC:$dst),
      (ins Operand<opType>:$imm, I8:$op2),
      !strconcat(opStr, "\t$dst, $imm, $op2"),
      [(set RC:$dst, (opNode (opType immNode:$imm), I8:$op2))]>;
  }
}

multiclass ShAdd<string opStr, SDNode inner, SDNode outer, RegisterClass RC,
           ValueType opType, SDNode immNode, list<Predicate> preds,
           InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, I8:$op2, RC:$op3),
      !strconcat(opStr, "\t$dst, $op1, $op2, $op3"),
      [(set RC:$dst, (outer (inner RC:$op1, I8:$op2), RC:$op3))]>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, I8:$op2, Operand<opType>:$imm),
      !strconcat(opStr, "\t$dst, $op1, $op2, $imm"),
      [(set RC:$dst, (outer (inner RC:$op1, I8:$op2), (opType immNode:$imm)))]>;
    def ix : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, Operand<i8>:$imm, RC:$op3),
      !strconcat(opStr, "\t$dst, $op1, $imm, $op3"),
      [(set RC:$dst, (outer (inner RC:$op1, (i8 immNode:$imm)), RC:$op3))]>;
    def ii : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, Operand<i8>:$imm2, RC:$imm3),
      !strconcat(opStr, "\t$dst, $op1, $imm2, $imm3"),
      [(set RC:$dst, (outer (inner RC:$op1, (i8 immNode:$imm2)),
                                                    (opType immNode:$imm3)))]>;
  }
}

multiclass FusedOp1<string opStr, SDNode inner, SDNode outer, RegisterClass RC,
           ValueType opType, SDNode immNode, list<Predicate> preds,
           InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, RC:$op2, RC:$op3),
      !strconcat(opStr, "\t$dst, $op1, $op2, $op3"),
      [(set RC:$dst, (outer (inner RC:$op1, RC:$op2), RC:$op3))]>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, RC:$op2, Operand<opType>:$imm),
      !strconcat(opStr, "\t$dst, $op1, $op2, $imm"),
      [(set RC:$dst, (outer (inner RC:$op1, RC:$op2), (opType immNode:$imm)))]>;
    def ix : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, Operand<opType>:$imm, RC:$op3),
      !strconcat(opStr, "\t$dst, $op1, $imm, $op3"),
      [(set RC:$dst, (outer (inner RC:$op1, (opType immNode:$imm)), RC:$op3))]>;
    def ii : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, Operand<opType>:$imm2, RC:$imm3),
      !strconcat(opStr, "\t$dst, $op1, $imm2, $imm3"),
      [(set RC:$dst, (outer (inner RC:$op1, (opType immNode:$imm2)),
                                                    (opType immNode:$imm3)))]>;
  }
}

multiclass FusedOp2<string opStr, SDNode inner, SDNode outer, RegisterClass RC,
           ValueType opType, SDNode immNode, list<Predicate> preds,
           InstrItinClass itin = NoItinerary> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, RC:$op2, RC:$op3),
      !strconcat(opStr, "\t$dst, $op2, $op3, $op1"),
      [(set RC:$dst, (outer RC:$op1, (inner RC:$op2, RC:$op3)))]>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins Operand<opType>:$imm, RC:$op2, RC:$op3),
      !strconcat(opStr, "\t$dst, $op2, $op3, $imm"),
      [(set RC:$dst, (outer (opType immNode:$imm), (inner RC:$op2, RC:$op3)))]>;
  }
}

// CmpOp[N]C are identical to BinOp, except opNode is a PatFrag rather than
// an SDNode...
multiclass CmpOpC<string opStr, PatFrag opNode, RegisterClass RC,
           ValueType opType, SDNode immNode, list<Predicate> preds,
           InstrItinClass itin = NoItinerary> {
  let isCommutable = 1, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set I1:$dst, (opNode opType:$op1, opType:$op2))]>;
    def i : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, Operand<opType>:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set I1:$dst, (opNode opType:$op1, (opType immNode:$imm)))]>;
  }
}

multiclass CmpOpNC<string opStr, PatFrag opNode, RegisterClass RC,
           ValueType opType, SDNode immNode, list<Predicate> preds,
           InstrItinClass itin = NoItinerary> {
  let isCommutable = 0, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set I1:$dst, (opNode opType:$op1, opType:$op2))]>;
    def i : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, Operand<opType>:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set I1:$dst, (opNode opType:$op1, (opType immNode:$imm)))]>;
    def i1 : FMTGEN<
      (outs I1:$dst),
      (ins Operand<opType>:$imm, RC:$op2),
      !strconcat(opStr, "\t$dst, $imm, $op2"),
      [(set I1:$dst, (opNode (opType immNode:$imm), opType:$op2))]>;
  }
}

multiclass LdOp<string opStr, RegisterClass RC, ValueType opType,
           InstrItinClass itin = IILD> {
  let Predicates = [HasIndex], Itinerary = itin in {
    def X : FMTGEN< // indexed reg+reg
      (outs RC:$dst),
      (ins ADDR_RX:$addr),
      !strconcat(opStr, "x\t$dst, $addr"),
      [(set opType:$dst, (load ADDR_RX:$addr))]>;
  }

  let Predicates = [HasDispl], Itinerary = itin in {
    def D : FMTGEN< // normal literal displacement form (val+k)
      (outs RC:$dst),
      (ins ADDR_RI:$addr),
      !strconcat(opStr, "d\t$dst, $addr"),
      [(set opType:$dst, (load ADDR_RI:$addr))]>;
    def R : FMTGEN< // non-indexed reg+reg
      (outs RC:$dst),
      (ins ADDR_RR:$addr),
      !strconcat(opStr, "d\t$dst, $addr"),
      [(set opType:$dst, (load ADDR_RR:$addr))]>;
  }

  let Itinerary = itin in {
    def I : FMTGEN<  // e.g. ld of literal address - ld rx,sym
      (outs RC:$dst),
      (ins ADDR_I:$addr),
      !strconcat(opStr, "\t$dst, $addr"),
      [(set opType:$dst, (load ADDR_I:$addr))]>;
    def "" : FMTGEN< // normal basereg form
      (outs RC:$dst),
      (ins ADDR_R:$addr),
      !strconcat(opStr, "\t$dst, $addr"),
      [(set opType:$dst, (load ADDR_R:$addr))]>;
    }
}

// Ugly combinatorial issue between literal store data and addressing modes
// TODO: How to handle store output flag?
multiclass StOp<string opStr, RegisterClass RC, ValueType opType,
           SDNode immNode, InstrItinClass itin = IIST> {
  let Predicates = [HasIndex], Itinerary = itin in {
    def Xi : FMTGEN<
      (outs),
      (ins ADDR_RX:$addr, Operand<opType>:$data),
      !strconcat(opStr, "x\t$addr, $data, %ign"),
      [(store (opType immNode:$data), ADDR_RX:$addr)]>;
    def X : FMTGEN<
      (outs),    
      (ins ADDR_RX:$addr, RC:$data),
      !strconcat(opStr, "x\t$addr, $data"),
      [(store opType:$data, ADDR_RX:$addr)]>;
  }

  let Predicates = [HasDispl], Itinerary = itin in {
    def Di : FMTGEN<
      (outs),        
      (ins ADDR_RI:$addr, Operand<opType>:$data),
      !strconcat(opStr, "d\t$addr, $data, %ign"),
      [(store (opType immNode:$data), ADDR_RI:$addr)]>;
    def D : FMTGEN<
      (outs),            
      (ins ADDR_RI:$addr, RC:$data),
      !strconcat(opStr, "d\t$addr, $data, %ign"),
      [(store opType:$data, ADDR_RI:$addr)]>;
    def Ri : FMTGEN<
      (outs),                
      (ins ADDR_RR:$addr, Operand<opType>:$data),
      !strconcat(opStr, "d\t$addr, $data, %ign"),
      [(store (opType immNode:$data), ADDR_RR:$addr)]>;
    def R : FMTGEN<
      (outs),                    
      (ins ADDR_RR:$addr, RC:$data),
      !strconcat(opStr, "d\t$addr, $data, %ign"),
      [(store opType:$data, ADDR_RR:$addr)]>;
  }

  let Itinerary = itin in {
    def i : FMTGEN<
      (outs),                        
      (ins ADDR_R:$addr, Operand<opType>:$data),
      !strconcat(opStr, "\t$addr, $data, %ign"),
      [(store (opType immNode:$data), ADDR_R:$addr)]>;
    def "" : FMTGEN<
      (outs),                            
      (ins ADDR_R:$addr, RC:$data),
      !strconcat(opStr, "\t$addr, $data, %ign"),
      [(store opType:$data, ADDR_R:$addr)]>;
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

multiclass OLdOp<string opStr, RegisterClass RC, ValueType opType,
           InstrItinClass itin = IILD> {
  let Predicates = [HasIndex], Itinerary = itin in {
    def X : FMTGEN< // indexed reg+reg
      (outs RC:$dst, I0:$issued),
      (ins ADDR_RX:$addr, I0:$ready),
      !strconcat(opStr, "x\t$dst, $addr, $issued, $ready"),
      []>;
  }

  let Predicates = [HasDispl], Itinerary = itin in {
    def D : FMTGEN< // normal literal displacement form (val+k)
      (outs RC:$dst, I0:$issued),
      (ins ADDR_RI:$addr, I0:$ready),
      !strconcat(opStr, "d\t$dst, $addr, $issued, $ready"),
      []>;
    def R : FMTGEN< // non-indexed reg+reg
      (outs RC:$dst, I0:$issued),    
      (ins ADDR_RR:$addr, I0:$ready),
      !strconcat(opStr, "d\t$dst, $addr, $issued, $ready"),
      []>;
  }

  let Itinerary = itin in {
    def I : FMTGEN<  // e.g. ld of literal address - ld rx,sym
      (outs RC:$dst, I0:$issued),
      (ins ADDR_I:$addr, I0:$ready),
      !strconcat(opStr, "\t$dst, $addr, $issued, $ready"),
      []>;
    def "" : FMTGEN< // normal basereg form
      (outs RC:$dst, I0:$issued),
      (ins ADDR_R:$addr, I0:$ready),
      !strconcat(opStr, "\t$dst, $addr, $issued, $ready"),
      []>;
    }
}



multiclass OStOp<string opStr, RegisterClass RC, ValueType opType,
           SDNode immNode, InstrItinClass itin = IIST> {
  let Predicates = [HasIndex], Itinerary = itin in {
    def Xi : FMTGEN<
      (outs I0:$issued),
      (ins ADDR_RX:$addr, Operand<opType>:$data, I0:$ready),
      !strconcat(opStr, "x\t$addr, $data, $issued, $ready"),
      []>;
    def X : FMTGEN<
      (outs I0:$issued),    
      (ins ADDR_RX:$addr, RC:$data, I0:$ready),
      !strconcat(opStr, "x\t$addr, $data, $issued, $ready"),
      []>;
  }

  let Predicates = [HasDispl], Itinerary = itin in {
    def Di : FMTGEN<
      (outs I0:$issued),        
      (ins ADDR_RI:$addr, Operand<opType>:$data, I0:$ready),
      !strconcat(opStr, "d\t$addr, $data, $issued, $ready"),
      []>;
    def D : FMTGEN<
      (outs I0:$issued),            
      (ins ADDR_RI:$addr, RC:$data, I0:$ready),
      !strconcat(opStr, "d\t$addr, $data, $issued, $ready"),
      []>;
    def Ri : FMTGEN<
      (outs I0:$issued),            
      (ins ADDR_RR:$addr, Operand<opType>:$data,  I0:$ready),
      !strconcat(opStr, "d\t$addr, $data, $issued, $ready"),
      []>;
    def R : FMTGEN<
      (outs I0:$issued),    
      (ins ADDR_RR:$addr, RC:$data, I0:$ready),
      !strconcat(opStr, "d\t$addr, $data, $issued, $ready"),
      []>;
  }

  let Itinerary = itin in {
      def i : FMTGEN<
        (outs I0:$issued),    
        (ins ADDR_R:$addr, Operand<opType>:$data, I0:$ready),
        !strconcat(opStr, "\t$addr, $data, $issued, $ready"),
        []>;
      def "" : FMTGEN<
        (outs I0:$issued),    
        (ins ADDR_R:$addr, RC:$data, I0:$ready),
        !strconcat(opStr, "\t$addr, $data, $issued, $ready"),
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


// The repetition below could be removed if nested multiclass were supported.
// (e.g. the first level would expand to 8/16/32/64, then the 2nd level
// would be as today.)

defm MOV1      : MovOp<"mov1",  I1,  i1,  [HasI1],  IIALU>;
defm MOV8      : MovOp<"mov8",  I8,  i8,  [HasI8],  IIALU>;
defm MOV16     : MovOp<"mov16", I16, i16, [HasI16], IIALU>;
defm MOV32     : MovOp<"mov32", I32, i32, [HasI32], IIALU>;
defm MOV64     : MovOp<"mov64", I64, i64, [HasI64], IIALU>;

// GlobalAddress, ExternalSymbol

def : Pat<(i64 (LPUWrapper tglobaladdr:$src)),   (MOV64i tglobaladdr:$src)>;
def : Pat<(i64 (LPUWrapper texternalsym:$src)),  (MOV64i texternalsym:$src)>;
def : Pat<(i64 (LPUWrapper tblockaddress:$src)), (MOV64i tblockaddress:$src)>;
def : Pat<(i64 (LPUWrapper tjumptable:$src)),    (MOV64i tjumptable:$src)>;

defm NOT1      : UnaryOpP<"not1",     not,   I1,  [HasI1],  IIALU>;
defm NOT8      : UnaryOpP<"not8",     not,   I8,  [HasI8],  IIALU>;
defm NOT16     : UnaryOpP<"not16",    not,   I16, [HasI16], IIALU>;
defm NOT32     : UnaryOpP<"not32",    not,   I32, [HasI32], IIALU>;
defm NOT64     : UnaryOpP<"not64",    not,   I64, [HasI64], IIALU>;
defm NEG8      : NegOp  <"neg8",             I8,  [HasI8],  IIALU>;
defm NEG16     : NegOp  <"neg16",            I16, [HasI16], IIALU>;
defm NEG32     : NegOp  <"neg32",            I32, [HasI32], IIALU>;
defm NEG64     : NegOp  <"neg64",            I64, [HasI64], IIALU>;
defm NEGF16    : UnaryOp<"negf16",    fneg,  I16, [HasF16], IIALU>;
defm NEGF32    : UnaryOp<"negf32",    fneg,  I32, [HasF32], IIALU>;
defm NEGF64    : UnaryOp<"negf64",    fneg,  I64, [HasF64], IIALU>;
defm ABSF16    : UnaryOp<"absf16",    fabs,  I16, [HasF16], IIALU>;
defm ABSF32    : UnaryOp<"absf32",    fabs,  I32, [HasF32], IIALU>;
defm ABSF64    : UnaryOp<"absf64",    fabs,  I64, [HasF64], IIALU>;
defm SQRTF16   : UnaryOp<"sqrtf16",   fsqrt, I16, [HasF16,HasSqrt], IISqrtF16>;
defm SQRTF32   : UnaryOp<"sqrtf32",   fsqrt, I32, [HasF32,HasSqrt], IISqrtF32>;
defm SQRTF64   : UnaryOp<"sqrtf64",   fsqrt, I64, [HasF64,HasSqrt], IISqrtF64>;
defm EXP2F16   : UnaryOp<"exp2f16",   fexp2, I16, [HasF16,HasMath0], IIMathF16>;
defm EXP2F32   : UnaryOp<"exp2f32",   fexp2, I32, [HasF32,HasMath0], IIMathF32>;
defm EXP2F64   : UnaryOp<"exp2f64",   fexp2, I64, [HasF64,HasMath0], IIMathF64>;
defm LOG2F16   : UnaryOp<"log2f16",   flog2, I16, [HasF16,HasMath0], IIMathF16>;
defm LOG2F32   : UnaryOp<"log2f32",   flog2, I32, [HasF32,HasMath0], IIMathF32>;
defm LOG2F64   : UnaryOp<"log2f64",   flog2, I64, [HasF64,HasMath0], IIMathF64>;
defm EXPF16    : UnaryOp<"expf16",    fexp,  I16, [HasF16,HasMath0], IIMathF16>;
defm EXPF32    : UnaryOp<"expf32",    fexp,  I32, [HasF32,HasMath0], IIMathF32>;
defm EXPF64    : UnaryOp<"expf64",    fexp,  I64, [HasF64,HasMath0], IIMathF64>;
defm LOGF16    : UnaryOp<"logf16",    flog,  I16, [HasF16,HasMath0], IIMathF16>;
defm LOGF32    : UnaryOp<"logf32",    flog,  I32, [HasF32,HasMath0], IIMathF32>;
defm LOGF64    : UnaryOp<"logf64",    flog,  I64, [HasF64,HasMath0], IIMathF64>;
defm SINF16    : UnaryOp<"sinf16",    fsin,  I16, [HasF16,HasMath0], IIMathF16>;
defm SINF32    : UnaryOp<"sinf32",    fsin,  I32, [HasF32,HasMath0], IIMathF32>;
defm SINF64    : UnaryOp<"sinf64",    fsin,  I64, [HasF64,HasMath0], IIMathF64>;
defm COSF16    : UnaryOp<"cosf16",    fcos,  I16, [HasF16,HasMath0], IIMathF16>;
defm COSF32    : UnaryOp<"cosf32",    fcos,  I32, [HasF32,HasMath0], IIMathF32>;
defm COSF64    : UnaryOp<"cosf64",    fcos,  I64, [HasF64,HasMath0], IIMathF64>;
//defm SINCOSF16 : UnaryOp<"sincosf16", fsincos, I16, [HasF16,HasMath0], IIMathF16>;
//defm SINCOSF32 : UnaryOp<"sincosf32", fsincos, I32, [HasF32,HasMath0], IIMathF32>;
//defm SINCOSF64 : UnaryOp<"sincosf64", fsincos, I64, [HasF64,HasMath0], IIMathF64>;

// BitOps. Note that LLVM's SDNodes expect the output type to match the input
// type, while in the simulator all output types are 8-bit.
defm CTPOP8    : UnaryOp<"ctpop8",   ctpop,            I8,  [HasI8,HasBitOp],  IIALU>;
defm CTPOP16   : UnaryOp<"ctpop16",  ctpop,            I16, [HasI16,HasBitOp], IIALU>;
defm CTPOP32   : UnaryOp<"ctpop32",  ctpop,            I32, [HasI32,HasBitOp], IIALU>;
defm CTPOP64   : UnaryOp<"ctpop64",  ctpop,            I64, [HasI64,HasBitOp], IIALU>;
defm CTLZ8     : UnaryOp<"ctlz8",    ctlz,             I8,  [HasI8,HasBitOp],  IIALU>;
defm CTLZ16    : UnaryOp<"ctlz16",   ctlz,             I16, [HasI16,HasBitOp], IIALU>;
defm CTLZ32    : UnaryOp<"ctlz32",   ctlz,             I32, [HasI32,HasBitOp], IIALU>;
defm CTLZ64    : UnaryOp<"ctlz64",   ctlz,             I64, [HasI64,HasBitOp], IIALU>;
defm CTTZ8     : UnaryOp<"cttz8",    cttz,             I8,  [HasI8,HasBitOp],  IIALU>;
defm CTTZ16    : UnaryOp<"cttz16",   cttz,             I16, [HasI16,HasBitOp], IIALU>;
defm CTTZ32    : UnaryOp<"cttz32",   cttz,             I32, [HasI32,HasBitOp], IIALU>;
defm CTTZ64    : UnaryOp<"cttz64",   cttz,             I64, [HasI64,HasBitOp], IIALU>;

// Helper fragment to find "parity". Clang's __builtin_parity(x) will result in
// (ctpop(x)&1) in IR, which will be transformed back to parityN(x) by this
// pattern fragment.
def parity : PatFrag<(ops node:$in), (and (ctpop node:$in), 1)>;

defm PARITY8   : UnaryOpP<"parity8",  parity,          I8,  [HasI8,HasBitOp],  IIALU>;
defm PARITY16  : UnaryOpP<"parity16", parity,          I16, [HasI16,HasBitOp], IIALU>;
defm PARITY32  : UnaryOpP<"parity32", parity,          I32, [HasI32,HasBitOp], IIALU>;
defm PARITY64  : UnaryOpP<"parity64", parity,          I64, [HasI64,HasBitOp], IIALU>;

// todo: int<=>float Conversions
// first type in convert name is result, second is source type
defm CVTS32F32 : CvtOp<"cvts32f32", fp_to_sint, I32, I32, [HasI32,HasF32], IICvtIF>;
defm CVTS32F64 : CvtOp<"cvts32f64", fp_to_sint, I32, I64, [HasI32,HasF64], IICvtIF>;
defm CVTU32F32 : CvtOp<"cvtu32f32", fp_to_uint, I32, I32, [HasI32,HasF32], IICvtIF>;
defm CVTU32F64 : CvtOp<"cvtu32f64", fp_to_uint, I32, I64, [HasI32,HasF64], IICvtIF>;
defm CVTS64F32 : CvtOp<"cvts64f32", fp_to_sint, I64, I32, [HasI64,HasF32], IICvtIF>;
defm CVTS64F64 : CvtOp<"cvts64f64", fp_to_sint, I64, I64, [HasI64,HasF64], IICvtIF>;
defm CVTU64F32 : CvtOp<"cvtu64f32", fp_to_uint, I64, I32, [HasI64,HasF32], IICvtIF>;
defm CVTU64F64 : CvtOp<"cvtu64f64", fp_to_uint, I64, I64, [HasI64,HasF64], IICvtIF>;

defm CVTF32S32 : CvtOp<"cvtf32s32", sint_to_fp, I32, I32, [HasF32,HasI32], IICvtFI>;
defm CVTF32S64 : CvtOp<"cvtf32s64", sint_to_fp, I32, I64, [HasF32,HasI64], IICvtFI>;
defm CVTF32U32 : CvtOp<"cvtf32u32", uint_to_fp, I32, I32, [HasF32,HasI32], IICvtFI>;
defm CVTF32U64 : CvtOp<"cvtf32u64", uint_to_fp, I32, I64, [HasF32,HasI64], IICvtFI>;
defm CVTF64S32 : CvtOp<"cvtf64s32", sint_to_fp, I64, I32, [HasF64,HasI32], IICvtFI>;
defm CVTF64S64 : CvtOp<"cvtf64s64", sint_to_fp, I64, I64, [HasF64,HasI64], IICvtFI>;
defm CVTF64U32 : CvtOp<"cvtf64u32", uint_to_fp, I64, I32, [HasF64,HasI32], IICvtFI>;
defm CVTF64U64 : CvtOp<"cvtf64u64", uint_to_fp, I64, I64, [HasF64,HasI64], IICvtFI>;

defm CVTF32F64 : CvtOp<"cvtf32f64", fround,   I32, I64, [HasF32,HasF64], IICvtFF>;
defm CVTF64F32 : CvtOp<"cvtf64f32", fextend,  I64, I32, [HasF32,HasF64], IICvtFF>;

defm SEXT8_1   : SExtOp<"sext8",  I8,  I1,  [HasI8],  IIALU>;
defm SEXT8     : SExtOp<"sext8",  I8,  I8,  [HasI8],  IIALU>;
defm SEXT16_1  : SExtOp<"sext16", I16, I1,  [HasI16], IIALU>;
defm SEXT16_8  : SExtOp<"sext16", I16, I8,  [HasI16], IIALU>;
defm SEXT16    : SExtOp<"sext16", I16, I16, [HasI16], IIALU>;
defm SEXT32_1  : SExtOp<"sext32", I32, I1,  [HasI32], IIALU>;
defm SEXT32_8  : SExtOp<"sext32", I32, I8,  [HasI32], IIALU>;
defm SEXT32_16 : SExtOp<"sext32", I32, I16, [HasI32], IIALU>;
defm SEXT32    : SExtOp<"sext32", I32, I32, [HasI32], IIALU>;
defm SEXT64_1  : SExtOp<"sext64", I64, I1,  [HasI64], IIALU>;
defm SEXT64_8  : SExtOp<"sext64", I64, I8,  [HasI64], IIALU>;
defm SEXT64_16 : SExtOp<"sext64", I64, I16, [HasI64], IIALU>;
defm SEXT64_32 : SExtOp<"sext64", I64, I32, [HasI64], IIALU>;
defm SEXT64    : SExtOp<"sext64", I64, I64, [HasI64], IIALU>;

defm AND1      : BinOpC< "and1",   and,  I1,  i1,  imm,   [HasI1],  IIALU>;
defm AND8      : BinOpC< "and8",   and,  I8,  i8,  imm,   [HasI8],  IIALU>;
defm AND16     : BinOpC< "and16",  and,  I16, i16, imm,   [HasI16], IIALU>;
defm AND32     : BinOpC< "and32",  and,  I32, i32, imm,   [HasI32], IIALU>;
defm AND64     : BinOpC< "and64",  and,  I64, i64, imm,   [HasI64], IIALU>;

defm OR1       : BinOpC< "or1",    or,   I1,  i1,  imm,   [HasI1],  IIALU>;
defm OR8       : BinOpC< "or8",    or,   I8,  i8,  imm,   [HasI8],  IIALU>;
defm OR16      : BinOpC< "or16",   or,   I16, i16, imm,   [HasI16], IIALU>;
defm OR32      : BinOpC< "or32",   or,   I32, i32, imm,   [HasI32], IIALU>;
defm OR64      : BinOpC< "or64",   or,   I64, i64, imm,   [HasI64], IIALU>;

defm XOR1      : BinOpC< "xor1",   xor,  I1,  i1,  imm,   [HasI1],  IIALU>;
defm XOR8      : BinOpC< "xor8",   xor,  I8,  i8,  imm,   [HasI8],  IIALU>;
defm XOR16     : BinOpC< "xor16",  xor,  I16, i16, imm,   [HasI16], IIALU>;
defm XOR32     : BinOpC< "xor32",  xor,  I32, i32, imm,   [HasI32], IIALU>;
defm XOR64     : BinOpC< "xor64",  xor,  I64, i64, imm,   [HasI64], IIALU>;

defm SLL8      : ShiftOp<"sll8",   shl,  I8,  i8,  imm,   [HasI8],  IIShft>;
defm SLL16     : ShiftOp<"sll16",  shl,  I16, i16, imm,   [HasI16], IIShft>;
defm SLL32     : ShiftOp<"sll32",  shl,  I32, i32, imm,   [HasI32], IIShft>;
defm SLL64     : ShiftOp<"sll64",  shl,  I64, i64, imm,   [HasI64], IIShft>;

defm SRL8      : ShiftOp<"srl8",   srl,  I8,  i8,  imm,   [HasI8],  IIShft>;
defm SRL16     : ShiftOp<"srl16",  srl,  I16, i16, imm,   [HasI16], IIShft>;
defm SRL32     : ShiftOp<"srl32",  srl,  I32, i32, imm,   [HasI32], IIShft>;
defm SRL64     : ShiftOp<"srl64",  srl,  I64, i64, imm,   [HasI64], IIShft>;

defm SRA8      : ShiftOp<"sra8",   sra,  I8,  i8,  imm,   [HasI8],  IIShft>;
defm SRA16     : ShiftOp<"sra16",  sra,  I16, i16, imm,   [HasI16], IIShft>;
defm SRA32     : ShiftOp<"sra32",  sra,  I32, i32, imm,   [HasI32], IIShft>;
defm SRA64     : ShiftOp<"sra64",  sra,  I64, i64, imm,   [HasI64], IIShft>;

def : Pat<(i1 (add i1:$op1, i1:$op2)),       (XOR1 $op1,$op2)>;
def : Pat<(i1 (add i1:$op1, (i1 imm:$imm))), (XOR1 $op1,$imm)>;
defm ADD8      : BinOpC< "add8",   add,  I8,  i8,  imm,   [HasI8],  IIALU>;
defm ADD16     : BinOpC< "add16",  add,  I16, i16, imm,   [HasI16], IIALU>;
defm ADD32     : BinOpC< "add32",  add,  I32, i32, imm,   [HasI32], IIALU>;
defm ADD64     : BinOpC< "add64",  add,  I64, i64, imm,   [HasI64], IIALU>;

defm ADDF16    : BinOpC< "addf16", fadd, I16, f16, fpimm, [HasF16], IIAddF16>;
defm ADDF32    : BinOpC< "addf32", fadd, I32, f32, fpimm, [HasF32], IIAddF32>;
defm ADDF64    : BinOpC< "addf64", fadd, I64, f64, fpimm, [HasF64], IIAddF64>;

def : Pat<(i1 (sub i1:$op1, i1:$op2)),       (XOR1 $op1,$op2)>;
def : Pat<(i1 (sub i1:$op1, (i1 imm:$imm))), (XOR1 $op1,$imm)>;
defm SUB8      : BinOpNC<"sub8",   sub,  I8,  i8,  imm,   [HasI8],  IIALU>;
defm SUB16     : BinOpNC<"sub16",  sub,  I16, i16, imm,   [HasI16], IIALU>;
defm SUB32     : BinOpNC<"sub32",  sub,  I32, i32, imm,   [HasI32], IIALU>;
defm SUB64     : BinOpNC<"sub64",  sub,  I64, i64, imm,   [HasI64], IIALU>;

defm SUBF16    : BinOpNC<"subf16", fsub, I16, f16, fpimm, [HasF16], IIAddF16>;
defm SUBF32    : BinOpNC<"subf32", fsub, I32, f32, fpimm, [HasF32], IIAddF32>;
defm SUBF64    : BinOpNC<"subf64", fsub, I64, f64, fpimm, [HasF64], IIAddF64>;

defm MUL8      : BinOpC< "mul8",   mul,  I8,  i8,  imm,   [HasI8],  IIMulI8>;
defm MUL16     : BinOpC< "mul16",  mul,  I16, i16, imm,   [HasI16], IIMulI16>;
defm MUL32     : BinOpC< "mul32",  mul,  I32, i32, imm,   [HasI32], IIMulI32>;
defm MUL64     : BinOpC< "mul64",  mul,  I64, i64, imm,   [HasI64], IIMulI64>;

defm MULF16    : BinOpC<"mulf16",  fmul, I16, f16, fpimm, [HasF16], IIMulF16>;
defm MULF32    : BinOpC<"mulf32",  fmul, I32, f32, fpimm, [HasF32], IIMulF32>;
defm MULF64    : BinOpC<"mulf64",  fmul, I64, f64, fpimm, [HasF64], IIMulF64>;

defm DIVS8     : BinOpNC<"divs8",  sdiv, I8,  i8,  imm,   [HasI8,HasIDiv],  IIDivI8>;
defm DIVS16    : BinOpNC<"divs16", sdiv, I16, i16, imm,   [HasI16,HasIDiv], IIDivI16>;
defm DIVS32    : BinOpNC<"divs32", sdiv, I32, i32, imm,   [HasI32,HasIDiv], IIDivI32>;
defm DIVS64    : BinOpNC<"divs64", sdiv, I64, i64, imm,   [HasI64,HasIDiv], IIDivI64>;

defm DIVU8     : BinOpNC<"divu8",  udiv, I8,  i8,  imm,   [HasI8,HasIDiv],  IIDivI8>;
defm DIVU16    : BinOpNC<"divu16", udiv, I16, i16, imm,   [HasI16,HasIDiv], IIDivI16>;
defm DIVU32    : BinOpNC<"divu32", udiv, I32, i32, imm,   [HasI32,HasIDiv], IIDivI32>;
defm DIVU64    : BinOpNC<"divu64", udiv, I64, i64, imm,   [HasI64,HasIDiv], IIDivI64>;

defm DIVF16    : BinOpNC<"divf16", fdiv, I16, f16, fpimm, [HasF16,HasFDiv], IIDivF16>;
defm DIVF32    : BinOpNC<"divf32", fdiv, I32, f32, fpimm, [HasF32,HasFDiv], IIDivF32>;
defm DIVF64    : BinOpNC<"divf64", fdiv, I64, f64, fpimm, [HasF64,HasFDiv], IIDivF64>;

defm POWF32    : BinOpNC<"powf32",   fpow,  I32, f32, fpimm, [HasF32,HasMath0], IIMathF32>;
defm POWF64    : BinOpNC<"powf64",   fpow,  I64, f64, fpimm, [HasF64,HasMath0], IIMathF64>;

defm CMPLTS8  : CmpOpNC<"cmplts8", setlt,  I8,  i8,  imm,   [HasI8],  IIALU>;
defm CMPLTS16 : CmpOpNC<"cmplts16",setlt,  I16, i16, imm,   [HasI16], IIALU>;
defm CMPLTS32 : CmpOpNC<"cmplts32",setlt,  I32, i32, imm,   [HasI32], IIALU>;
defm CMPLTS64 : CmpOpNC<"cmplts64",setlt,  I64, i64, imm,   [HasI64], IIALU>;

defm CMPLTU8  : CmpOpNC<"cmpltu8", setult, I8,  i8,  imm,   [HasI8],  IIALU>;
defm CMPLTU16 : CmpOpNC<"cmpltu16",setult, I16, i16, imm,   [HasI16], IIALU>;
defm CMPLTU32 : CmpOpNC<"cmpltu32",setult, I32, i32, imm,   [HasI32], IIALU>;
defm CMPLTU64 : CmpOpNC<"cmpltu64",setult, I64, i64, imm,   [HasI64], IIALU>;

defm CMPLES8  : CmpOpNC<"cmples8", setle,  I8,  i8,  imm,   [HasI8],  IIALU>;
defm CMPLES16 : CmpOpNC<"cmples16",setle,  I16, i16, imm,   [HasI16], IIALU>;
defm CMPLES32 : CmpOpNC<"cmples32",setle,  I32, i32, imm,   [HasI32], IIALU>;
defm CMPLES64 : CmpOpNC<"cmples64",setle,  I64, i64, imm,   [HasI64], IIALU>;

defm CMPLEU8  : CmpOpNC<"cmpleu8", setule, I8,  i8,  imm,   [HasI8],  IIALU>;
defm CMPLEU16 : CmpOpNC<"cmpleu16",setule, I16, i16, imm,   [HasI16], IIALU>;
defm CMPLEU32 : CmpOpNC<"cmpleu32",setule, I32, i32, imm,   [HasI32], IIALU>;
defm CMPLEU64 : CmpOpNC<"cmpleu64",setule, I64, i64, imm,   [HasI64], IIALU>;

// Note - there is no cmpeq1.  As long as operands are clean, cmpeq8
// suffices
defm CMPEQ1   : CmpOpC<"cmpeq8",   seteq,  I1,  i1,  imm,   [HasI1],  IIALU>;
defm CMPEQ8   : CmpOpC<"cmpeq8",   seteq,  I8,  i8,  imm,   [HasI8],  IIALU>;
defm CMPEQ16  : CmpOpC<"cmpeq16",  seteq,  I16, i16, imm,   [HasI16], IIALU>;
defm CMPEQ32  : CmpOpC<"cmpeq32",  seteq,  I32, i32, imm,   [HasI32], IIALU>;
defm CMPEQ64  : CmpOpC<"cmpeq64",  seteq,  I64, i64, imm,   [HasI64], IIALU>;

defm CMPGTS8  : CmpOpNC<"cmpgts8", setgt,  I8,  i8,  imm,   [HasI8],  IIALU>;
defm CMPGTS16 : CmpOpNC<"cmpgts16",setgt,  I16, i16, imm,   [HasI16], IIALU>;
defm CMPGTS32 : CmpOpNC<"cmpgts32",setgt,  I32, i32, imm,   [HasI32], IIALU>;
defm CMPGTS64 : CmpOpNC<"cmpgts64",setgt,  I64, i64, imm,   [HasI64], IIALU>;

defm CMPGTU8  : CmpOpNC<"cmpgtu8", setugt, I8,  i8,  imm,   [HasI8],  IIALU>;
defm CMPGTU16 : CmpOpNC<"cmpgtu16",setugt, I16, i16, imm,   [HasI16], IIALU>;
defm CMPGTU32 : CmpOpNC<"cmpgtu32",setugt, I32, i32, imm,   [HasI32], IIALU>;
defm CMPGTU64 : CmpOpNC<"cmpgtu64",setugt, I64, i64, imm,   [HasI64], IIALU>;

defm CMPGES8  : CmpOpNC<"cmpges8", setge,  I8,  i8,  imm,   [HasI8],  IIALU>;
defm CMPGES16 : CmpOpNC<"cmpges16",setge,  I16, i16, imm,   [HasI16], IIALU>;
defm CMPGES32 : CmpOpNC<"cmpges32",setge,  I32, i32, imm,   [HasI32], IIALU>;
defm CMPGES64 : CmpOpNC<"cmpges64",setge,  I64, i64, imm,   [HasI64], IIALU>;

defm CMPGEU8  : CmpOpNC<"cmpgeu8", setuge, I8,  i8,  imm,   [HasI8],  IIALU>;
defm CMPGEU16 : CmpOpNC<"cmpgeu16",setuge, I16, i16, imm,   [HasI16], IIALU>;
defm CMPGEU32 : CmpOpNC<"cmpgeu32",setuge, I32, i32, imm,   [HasI32], IIALU>;
defm CMPGEU64 : CmpOpNC<"cmpgeu64",setuge, I64, i64, imm,   [HasI64], IIALU>;

def : Pat<(i1 (setne i1:$op1, i1:$op2)), (XOR1 $op1,$op2)>;
def : Pat<(i1 (setne i1:$op1, (i1 imm:$imm))), (XOR1i $op1,$imm)>;
defm CMPNE8   : CmpOpC<"cmpne8",   setne,  I8,  i8,  imm,   [HasI8],  IIALU>;
defm CMPNE16  : CmpOpC<"cmpne16",  setne,  I16, i16, imm,   [HasI16], IIALU>;
defm CMPNE32  : CmpOpC<"cmpne32",  setne,  I32, i32, imm,   [HasI32], IIALU>;
defm CMPNE64  : CmpOpC<"cmpne64",  setne,  I64, i64, imm,   [HasI64], IIALU>;

defm CMPOLTF16: CmpOpNC<"cmpltf16",setolt, I16, f16, fpimm, [HasF16], IICmpF>;
defm CMPOLTF32: CmpOpNC<"cmpltf32",setolt, I32, f32, fpimm, [HasF32], IICmpF>;
defm CMPOLTF64: CmpOpNC<"cmpltf64",setolt, I64, f64, fpimm, [HasF64], IICmpF>;
defm CMPULTF16: CmpOpNC<"cmpltf16",setult, I16, f16, fpimm, [HasF16], IICmpF>;
defm CMPULTF32: CmpOpNC<"cmpltf32",setult, I32, f32, fpimm, [HasF32], IICmpF>;
defm CMPULTF64: CmpOpNC<"cmpltf64",setult, I64, f64, fpimm, [HasF64], IICmpF>;

defm CMPOLEF16: CmpOpNC<"cmplef16",setole, I16, f16, fpimm, [HasF16], IICmpF>;
defm CMPOLEF32: CmpOpNC<"cmplef32",setole, I32, f32, fpimm, [HasF32], IICmpF>;
defm CMPOLEF64: CmpOpNC<"cmplef64",setole, I64, f64, fpimm, [HasF64], IICmpF>;
defm CMPULEF16: CmpOpNC<"cmplef16",setule, I16, f16, fpimm, [HasF16], IICmpF>;
defm CMPULEF32: CmpOpNC<"cmplef32",setule, I32, f32, fpimm, [HasF32], IICmpF>;
defm CMPULEF64: CmpOpNC<"cmplef64",setule, I64, f64, fpimm, [HasF64], IICmpF>;

defm CMPOEQF16: CmpOpC<"cmpeqf16", setoeq, I16, f16, fpimm, [HasF16], IICmpF>;
defm CMPOEQF32: CmpOpC<"cmpeqf32", setoeq, I32, f32, fpimm, [HasF32], IICmpF>;
defm CMPOEQF64: CmpOpC<"cmpeqf64", setoeq, I64, f64, fpimm, [HasF64], IICmpF>;
defm CMPUEQF16: CmpOpC<"cmpeqf16", setueq, I16, f16, fpimm, [HasF16], IICmpF>;
defm CMPUEQF32: CmpOpC<"cmpeqf32", setueq, I32, f32, fpimm, [HasF32], IICmpF>;
defm CMPUEQF64: CmpOpC<"cmpeqf64", setueq, I64, f64, fpimm, [HasF64], IICmpF>;

defm CMPOGTF16: CmpOpNC<"cmpgtf16",setogt, I16, f16, fpimm, [HasF16], IICmpF>;
defm CMPOGTF32: CmpOpNC<"cmpgtf32",setogt, I32, f32, fpimm, [HasF32], IICmpF>;
defm CMPOGTF64: CmpOpNC<"cmpgtf64",setogt, I64, f64, fpimm, [HasF64], IICmpF>;
defm CMPUGTF16: CmpOpNC<"cmpgtf16",setugt, I16, f16, fpimm, [HasF16], IICmpF>;
defm CMPUGTF32: CmpOpNC<"cmpgtf32",setugt, I32, f32, fpimm, [HasF32], IICmpF>;
defm CMPUGTF64: CmpOpNC<"cmpgtf64",setugt, I64, f64, fpimm, [HasF64], IICmpF>;

defm CMPOGEF16: CmpOpNC<"cmpgef16",setoge, I16, f16, fpimm, [HasF16], IICmpF>;
defm CMPOGEF32: CmpOpNC<"cmpgef32",setoge, I32, f32, fpimm, [HasF32], IICmpF>;
defm CMPOGEF64: CmpOpNC<"cmpgef64",setoge, I64, f64, fpimm, [HasF64], IICmpF>;
defm CMPUGEF16: CmpOpNC<"cmpgef16",setuge, I16, f16, fpimm, [HasF16], IICmpF>;
defm CMPUGEF32: CmpOpNC<"cmpgef32",setuge, I32, f32, fpimm, [HasF32], IICmpF>;
defm CMPUGEF64: CmpOpNC<"cmpgef64",setuge, I64, f64, fpimm, [HasF64], IICmpF>;

defm CMPONEF16: CmpOpC<"cmpnef16", setone, I16, f16, fpimm, [HasF16], IICmpF>;
defm CMPONEF32: CmpOpC<"cmpnef32", setone, I32, f32, fpimm, [HasF32], IICmpF>;
defm CMPONEF64: CmpOpC<"cmpnef64", setone, I64, f64, fpimm, [HasF64], IICmpF>;
defm CMPUNEF16: CmpOpC<"cmpnef16", setune, I16, f16, fpimm, [HasF16], IICmpF>;
defm CMPUNEF32: CmpOpC<"cmpnef32", setune, I32, f32, fpimm, [HasF32], IICmpF>;
defm CMPUNEF64: CmpOpC<"cmpnef64", setune, I64, f64, fpimm, [HasF64], IICmpF>;

defm CMPOF16  : CmpOpC<"cmpof16",  seto,   I16, f16, fpimm, [HasF16], IICmpF>;
defm CMPOF32  : CmpOpC<"cmpof32",  seto,   I32, f32, fpimm, [HasF32], IICmpF>;
defm CMPOF64  : CmpOpC<"cmpof64",  seto,   I64, f64, fpimm, [HasF64], IICmpF>;
defm CMPUOF16 : CmpOpC<"cmpuof16", setuo,  I16, f16, fpimm, [HasF16], IICmpF>;
defm CMPUOF32 : CmpOpC<"cmpuof32", setuo,  I32, f32, fpimm, [HasF32], IICmpF>;
defm CMPUOF64 : CmpOpC<"cmpuof64", setuo,  I64, f64, fpimm, [HasF64], IICmpF>;


multiclass BSelectOp<string opStr, RegisterClass RC, ValueType opType, SDNode immNode,InstrItinClass itin = NoItinerary> {
  let Itinerary = itin in {
    // BSEL - Only allow constant 1st operand, so CSA network only needs to support
    // 2 operand input boxes.
    def "": FMTGEN<
      (outs RC:$dst),
      (ins Operand<opType>:$sel, RC:$v0, RC:$v1),
      !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
      []>;
  }
}

defm BSEL1    :BSelectOp <"bsel1",   I1, i1, imm>;
defm BSEL8    :BSelectOp <"bsel8",   I8, i8, imm>;
defm BSEL16   :BSelectOp <"bsel16",  I16, i16, imm>;
defm BSEL32   :BSelectOp <"bsel32",  I32, i32, imm>;
defm BSEL64   :BSelectOp <"bsel64",  I64, i64, imm>;

def LAND1 : FMTGEN<
  (outs I1:$dst),
  (ins I1:$op1, I1:$op2, I1:$op3, I1:$op4),
  "land1\t$dst, $op1, $op2, $op3, $op4",
  []> { let Itinerary = IIALU; }

// Shift/add - itinerary depends on constant small shift amt - else IShft...
defm SLADD8  : ShAdd<"sladd8",  shl,  add,  I8,  i8,  imm,   [HasI8, HasShAdd], IISAdd>;
defm SLADD16 : ShAdd<"sladd16", shl,  add,  I16, i16, imm,   [HasI16,HasShAdd], IISAdd>;
defm SLADD32 : ShAdd<"sladd32", shl,  add,  I32, i32, imm,   [HasI32,HasShAdd], IISAdd>;
defm SLADD64 : ShAdd<"sladd64", shl,  add,  I64, i64, imm,   [HasI64,HasShAdd], IISAdd>;

defm FMAF16  : FusedOp1<"fmaf16",  fmul, fadd, I16, f16, fpimm, [HasF16,HasFMA], IIFMAF16>;
// crude handling of commutative
defm FMAF16x : FusedOp2<"fmaf16",  fmul, fadd, I16, f16, fpimm, [HasF16,HasFMA], IIFMAF16>;
defm FMSF16  : FusedOp1<"fmsf16",  fmul, fsub, I16, f16, fpimm, [HasF16,HasFMA], IIFMAF16>;
defm FMRSF16 : FusedOp2<"fmrsf16", fmul, fsub, I16, f16, fpimm, [HasF16,HasFMA], IIFMAF16>;

defm FMAF32  : FusedOp1<"fmaf32",  fmul, fadd, I32, f32, fpimm, [HasF32,HasFMA], IIFMAF32>;
// crude handling of commutative
defm FMAF32x : FusedOp2<"fmaf32",  fmul, fadd, I32, f32, fpimm, [HasF32,HasFMA], IIFMAF32>;
defm FMSF32  : FusedOp1<"fmsf32",  fmul, fsub, I32, f32, fpimm, [HasF32,HasFMA], IIFMAF32>;
defm FMRSF32 : FusedOp2<"fmrsf32", fmul, fsub, I32, f32, fpimm, [HasF32,HasFMA], IIFMAF32>;

defm FMAF64  : FusedOp1<"fmaf64",  fmul, fadd, I64, f64, fpimm, [HasF64,HasFMA], IIFMAF64>;
// crude handling of commutative
defm FMAF64x : FusedOp2<"fmaf64",  fmul, fadd, I64, f64, fpimm, [HasF64,HasFMA], IIFMAF64>;
defm FMSF64  : FusedOp1<"fmsf64",  fmul, fsub, I64, f64, fpimm, [HasF64,HasFMA], IIFMAF64>;
defm FMRSF64 : FusedOp2<"fmrsf64", fmul, fsub, I64, f64, fpimm, [HasF64,HasFMA], IIFMAF64>;

// Should we do these, or drop and assume sext-at-consumer?
// todo: LDS8
// todo: LDS16
// todo: LDS32
defm LD1   : LdOp<"ld8",   I1,  i1>;
defm LD8   : LdOp<"ld8",   I8,  i8>;
defm LD16  : LdOp<"ld16",  I16, i16>;
defm LD16f : LdOp<"ld16",  I16, f16>;
defm LD32  : LdOp<"ld32",  I32, i32>;
defm LD32f : LdOp<"ld32",  I32, f32>;
defm LD64  : LdOp<"ld64",  I64, i64>;
defm LD64f : LdOp<"ld64",  I64, f64>;

defm OLD1   : OLdOp<"ld8",   I1,  i1>;
defm OLD8   : OLdOp<"ld8",   I8,  i8>;
defm OLD16  : OLdOp<"ld16",  I16, i16>;
defm OLD16f : OLdOp<"ld16",  I16, f16>;
defm OLD32  : OLdOp<"ld32",  I32, i32>;
defm OLD32f : OLdOp<"ld32",  I32, f32>;
defm OLD64  : OLdOp<"ld64",  I64, i64>;
defm OLD64f : OLdOp<"ld64",  I64, f64>;

defm ST1   : StOp<"st8",   I1,  i1,  imm>;
defm ST8   : StOp<"st8",   I8,  i8,  imm>;
defm ST16  : StOp<"st16",  I16, i16, imm>;
defm ST16f : StOp<"st16",  I16, f16, fpimm>;
defm ST32  : StOp<"st32",  I32, i32, imm>;
defm ST32f : StOp<"st32",  I32, f32, fpimm>;
defm ST64  : StOp<"st64",  I64, i64, imm>;
defm ST64f : StOp<"st64",  I64, f64, fpimm>;

defm OST1   : OStOp<"st8",   I1,  i1,  imm>;
defm OST8   : OStOp<"st8",   I8,  i8,  imm>;
defm OST16  : OStOp<"st16",  I16, i16, imm>;
defm OST16f : OStOp<"st16",  I16, f16, fpimm>;
defm OST32  : OStOp<"st32",  I32, i32, imm>;
defm OST32f : OStOp<"st32",  I32, f32, fpimm>;
defm OST64  : OStOp<"st64",  I64, i64, imm>;
defm OST64f : OStOp<"st64",  I64, f64, fpimm>;

multiclass CopyOp<string opStr, RegisterClass RC,
           InstrItinClass itin = IIVir> {
  let Itinerary = itin in {
    def "" : FMTGEN<
      (outs RC:$op0, RC:$op1, RC:$op2, RC:$op3),
      (ins RC:$op4),
      !strconcat(opStr, "\t$op0, $op1, $op2, $op3, $op4"),
      []>;
  }
}
defm COPY0   : CopyOp<"copy0",  I1>;
defm COPY1   : CopyOp<"copy1",  I1>;
defm COPY8   : CopyOp<"copy8",  I8>;
defm COPY16  : CopyOp<"copy16", I16>;
defm COPY32  : CopyOp<"copy32", I32>;
defm COPY64  : CopyOp<"copy64", I64>;

multiclass MergeOp<string opStr, RegisterClass RC, ValueType opType,
                   SDNode immNode,
                   list<Predicate> preds, InstrItinClass itin> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins I1:$sel, RC:$v0, RC:$v1),
      !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
      [(set opType:$dst, (select I1:$sel, opType:$v1, opType:$v0))]>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins I1:$sel, RC:$v0, Operand<opType>:$imm),
      !strconcat(opStr, "\t$dst, $sel, $v0, $imm"),
      [(set opType:$dst, (select I1:$sel, (opType immNode:$imm), opType:$v0))]>;
    // allow literal operation for the 1st of the 2 selected operands
    def i1 : FMTGEN<
      (outs RC:$dst),
      (ins I1:$sel, Operand<opType>:$imm, RC:$v1),
      !strconcat(opStr, "\t$dst, $sel, $imm, $v1"),
      [(set opType:$dst, (select I1:$sel, opType:$v1, (opType immNode:$imm)))]>;
  }
}
defm MERGE1   : MergeOp<"merge1",  I1,  i1,  imm,   [HasI1],  IIVir>;
defm MERGE8   : MergeOp<"merge8",  I8,  i8,  imm,   [HasI8],  IIVir>;
defm MERGE16  : MergeOp<"merge16", I16, i16, imm,   [HasI16], IIVir>;
defm MERGE16f : MergeOp<"merge16", I16, f16, fpimm, [HasI16], IIVir>;
defm MERGE32  : MergeOp<"merge32", I32, i32, imm,   [HasI32], IIVir>;
defm MERGE32f : MergeOp<"merge32", I32, f32, fpimm, [HasI32], IIVir>;
defm MERGE64  : MergeOp<"merge64", I64, i64, imm,   [HasI64], IIVir>;
defm MERGE64f : MergeOp<"merge64", I64, f64, fpimm, [HasI64], IIVir>;

multiclass SwitchOp<string opStr, RegisterClass RC,
           list<Predicate> preds, InstrItinClass itin> {
  let Itinerary = itin in {
    def "" : FMTGEN<
      (outs RC:$dst0, RC:$dst1),
      (ins I1:$sel, RC:$v),
      !strconcat(opStr, "\t$dst0, $dst1, $sel, $v"),
      []>;
  }
}
defm SWITCH1   : SwitchOp<"switch1",  I1,  [HasI1],  IIVir>;
defm SWITCH8   : SwitchOp<"switch8",  I8,  [HasI8],  IIVir>;
defm SWITCH16  : SwitchOp<"switch16", I16, [HasI16], IIVir>;
defm SWITCH32  : SwitchOp<"switch32", I32, [HasI32], IIVir>;
defm SWITCH64  : SwitchOp<"switch64", I64, [HasI64], IIVir>;

multiclass SwitchAnyOp<string opStr, RegisterClass RC,
           list<Predicate> preds, InstrItinClass itin> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs RC:$dst0, RC:$dst1, I1:$sel),
      (ins RC:$v),
      !strconcat(opStr, "\t$dst0, $dst1, $sel, $v"),
      []>;
  }
}
defm SWITCHANY1  : SwitchAnyOp<"switchany1",  I1,  [HasI1],  IIVir>;
defm SWITCHANY8  : SwitchAnyOp<"switchany8",  I8,  [HasI8],  IIVir>;
defm SWITCHANY16 : SwitchAnyOp<"switchany16", I16, [HasI16], IIVir>;
defm SWITCHANY32 : SwitchAnyOp<"switchany32", I32, [HasI32], IIVir>;
defm SWITCHANY64 : SwitchAnyOp<"switchany64", I64, [HasI64], IIVir>;

multiclass PickOp<string opStr, RegisterClass RC,
           list<Predicate> preds, InstrItinClass itin> {
  let Itinerary = itin in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins I1:$sel, RC:$v0, RC:$v1),
      !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
      []>;
  }
}
defm PICK1   : PickOp<"pick1",  I1,  [HasI1],  IIVir>;
defm PICK8   : PickOp<"pick8",  I8,  [HasI8],  IIVir>;
defm PICK16  : PickOp<"pick16", I16, [HasI16], IIVir>;
defm PICK32  : PickOp<"pick32", I32, [HasI32], IIVir>;
defm PICK64  : PickOp<"pick64", I64, [HasI64], IIVir>;

multiclass PickAnyOp<string opStr, RegisterClass RC,
           list<Predicate> preds, InstrItinClass itin> {
  let Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<
      (outs RC:$dst, I1:$sel),
      (ins RC:$v0, RC:$v1),
      !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
      []>;
  }
}
defm PICKANY1  : PickAnyOp<"pickany1",  I1,  [HasI1],  IIVir>;
defm PICKANY8  : PickAnyOp<"pickany8",  I8,  [HasI8],  IIVir>;
defm PICKANY16 : PickAnyOp<"pickany16", I16, [HasI16], IIVir>;
defm PICKANY32 : PickAnyOp<"pickany32", I32, [HasI32], IIVir>;
defm PICKANY64 : PickAnyOp<"pickany64", I64, [HasI64], IIVir>;

def ANY0 : FMTGEN<
  (outs I64:$dst),
  (ins I0:$op1, I0:$op2, I0:$op3, I0:$op4),
  "any0\t$dst, $op1, $op2, $op3, $op4",
  []> { let Itinerary = IIVir; }

def ALL0 : FMTGEN<
  (outs I0:$dst),
  (ins I0:$op1, I0:$op2, I0:$op3, I0:$op4),
  "all0\t$dst, $op1, $op2, $op3, $op4",
  []> { let Itinerary = IIVir; }

def ONCOUNT0 : FMTGEN<  // TODO: something to reflect state
  (outs I0:$dst),
  (ins I64:$cnt, I0:$op2, I0:$op3, I0:$op4, I0:$op5),
  "oncount0\t$dst, $cnt, $op2, $op3, $op4, $op5",
  []> { let Itinerary = IIVir; }

multiclass SeqCOp<string opStr, RegisterClass RC, ValueType opType,
           list<Predicate> preds, InstrItinClass itin> {
  // hasSideEffects attempts to capture the internal state and stream output behavior
  let hasSideEffects = 1, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<  // TODO: something to reflect state
      (outs RC:$val, I1:$pred, I1:$first, I1:$last),
      (ins RC:$base, RC:$count, RC:$stride),
      !strconcat(opStr, "\t$val, $pred, $first, $last, $base, $count, $stride"),
      []>;
  }
}

defm SEQC8    : SeqCOp<"seqc8",    I8, i8,   [HasI8],  IIALU>;
defm SEQC16   : SeqCOp<"seqc16",   I16, i16, [HasI16], IIALU>;
defm SEQC32   : SeqCOp<"seqc32",   I32, i32, [HasI32], IIALU>;
defm SEQC64   : SeqCOp<"seqc64",   I64, i64, [HasI64], IIALU>;

multiclass SeqSOp<string opStr, RegisterClass RC, ValueType opType,
           list<Predicate> preds, InstrItinClass itin> {
  // hasSideEffects attempts to capture the internal state and stream output behavior
  let hasSideEffects = 1, Predicates = preds, Itinerary = itin in {
    // TODO: something to reflect state
    // Unfortunately, 
    def "" : FMTGEN< // rrr form
      (outs RC:$val, I1:$pred, I1:$first, I1:$last),
      (ins RC:$base, RC:$bound, RC:$stride),
      !strconcat(opStr, "\t$val, $pred, $first, $last, $base, $bound, $stride"),
      []>;
  }
}

defm SEQLTS8  : SeqSOp<"seqlts8",  I8,  i8,  [HasI8],  IIALU>;
defm SEQLTS16 : SeqSOp<"seqlts16", I16, i16, [HasI16], IIALU>;
defm SEQLTS32 : SeqSOp<"seqlts32", I32, i32, [HasI32], IIALU>;
defm SEQLTS64 : SeqSOp<"seqlts64", I64, i64, [HasI64], IIALU>;

defm SEQLTU8  : SeqSOp<"seqltu8",  I8, i8,   [HasI8],  IIALU>;
defm SEQLTU16 : SeqSOp<"seqltu16", I16, i16, [HasI16], IIALU>;
defm SEQLTU32 : SeqSOp<"seqltu32", I32, i32, [HasI32], IIALU>;
defm SEQLTU64 : SeqSOp<"seqltu64", I64, i64, [HasI64], IIALU>;

defm SEQLES8  : SeqSOp<"seqles8",  I8,  i8,  [HasI8],  IIALU>;
defm SEQLES16 : SeqSOp<"seqles16", I16, i16, [HasI16], IIALU>;
defm SEQLES32 : SeqSOp<"seqles32", I32, i32, [HasI32], IIALU>;
defm SEQLES64 : SeqSOp<"seqles64", I64, i64, [HasI64], IIALU>;

defm SEQLEU8  : SeqSOp<"seqleu8",  I8,  i8,  [HasI8],  IIALU>;
defm SEQLEU16 : SeqSOp<"seqleu16", I16, i16, [HasI16], IIALU>;
defm SEQLEU32 : SeqSOp<"seqleu32", I32, i32, [HasI32], IIALU>;
defm SEQLEU64 : SeqSOp<"seqleu64", I64, i64, [HasI64], IIALU>;

defm SEQNE8   : SeqSOp<"seqne8",   I8,  i8,  [HasI8],  IIALU>;
defm SEQNE16  : SeqSOp<"seqne16",  I16, i16, [HasI16], IIALU>;
defm SEQNE32  : SeqSOp<"seqne32",  I32, i32, [HasI32], IIALU>;
defm SEQNE64  : SeqSOp<"seqne64",  I64, i64, [HasI64], IIALU>;

defm SEQGTS8  : SeqSOp<"seqgts8",  I8,  i8,  [HasI8],  IIALU>;
defm SEQGTS16 : SeqSOp<"seqgts16", I16, i16, [HasI16], IIALU>;
defm SEQGTS32 : SeqSOp<"seqgts32", I32, i32, [HasI32], IIALU>;
defm SEQGTS64 : SeqSOp<"seqgts64", I64, i64, [HasI64], IIALU>;

defm SEQGTU8  : SeqSOp<"seqgtu8",  I8,  i8,  [HasI8],  IIALU>;
defm SEQGTU16 : SeqSOp<"seqgtu16", I16, i16, [HasI16], IIALU>;
defm SEQGTU32 : SeqSOp<"seqgtu32", I32, i32, [HasI32], IIALU>;
defm SEQGTU64 : SeqSOp<"seqgtu64", I64, i64, [HasI64], IIALU>;

defm SEQGES8  : SeqSOp<"seqges8",  I8,  i8,  [HasI8],  IIALU>;
defm SEQGES16 : SeqSOp<"seqges16", I16, i16, [HasI16], IIALU>;
defm SEQGES32 : SeqSOp<"seqges32", I32, i32, [HasI32], IIALU>;
defm SEQGES64 : SeqSOp<"seqges64", I64, i64, [HasI64], IIALU>;

defm SEQGEU8  : SeqSOp<"seqgeu8",  I8,  i8,  [HasI8],  IIALU>;
defm SEQGEU16 : SeqSOp<"seqgeu16", I16, i16, [HasI16], IIALU>;
defm SEQGEU32 : SeqSOp<"seqgeu32", I32, i32, [HasI32], IIALU>;
defm SEQGEU64 : SeqSOp<"seqgeu64", I64, i64, [HasI64], IIALU>;

multiclass RepeatOp<string opStr, RegisterClass RC, ValueType opType,
           list<Predicate> preds, InstrItinClass itin> {
  // hasSideEffects attempts to capture the internal state and stream output behavior
  let hasSideEffects = 1, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<  // TODO: something to reflect state
      (outs I64:$out),
      (ins I1:$pred, RC:$in),
      !strconcat(opStr, "\t$out, $pred, $in"),
      []>;
  }
}

defm REPEAT8  : RepeatOp<"repeat8",  I8,  i8,  [HasI8],  IIALU>;
defm REPEAT16 : RepeatOp<"repeat16", I16, i16, [HasI16], IIALU>;
defm REPEAT32 : RepeatOp<"repeat32", I32, i32, [HasI32], IIALU>;
defm REPEAT64 : RepeatOp<"repeat64", I64, i64, [HasI64], IIALU>;

multiclass StrideOp<string opStr, RegisterClass RC, ValueType opType,
           list<Predicate> preds, InstrItinClass itin> {
  // hasSideEffects attempts to capture the internal state and stream output behavior
  let hasSideEffects = 1, Predicates = preds, Itinerary = itin in {
    def "" : FMTGEN<  // TODO: something to reflect state
      (outs I64:$out),
      (ins I1:$pred, RC:$base, RC:$stride),
      !strconcat(opStr, "\t$out, $pred, $base, $stride"),
      []>;
  }
}

defm STRIDE8  : StrideOp<"stride8",  I8,  i8,  [HasI8],  IIALU>;
defm STRIDE16 : StrideOp<"stride16", I16, i16, [HasI16], IIALU>;
defm STRIDE32 : StrideOp<"stride32", I32, i32, [HasI32], IIALU>;
defm STRIDE64 : StrideOp<"stride64", I64, i64, [HasI64], IIALU>;

// TBD(jsukha): This info probably needs predicates
// Also, the input can probably be an I0 channel.
def ONEND : FMTGEN<  // TODO: something to reflect state
  (outs I0:$dst),
  (ins I1:$ctrl, I0:$in),
  "onend\t$dst, $ctrl, $in",
  []> { let Itinerary = IIVir; }
  

multiclass Init<RegisterClass RC, ValueType opType,
           list<Predicate> preds, InstrItinClass itin> {
  // Not clear that "hasSideEffects" is a good description of static initialization...
  let hasSideEffects = 1, Predicates = preds, Itinerary = itin in {
    def "" : PseudoInstLPU<
      (outs RC:$dst),
      (ins Operand<opType>:$imm),
      ".curr\t$dst;\t.value $imm;\t.avail 0",
      []>;
  }
}

defm INIT0  : Init<I1,  i1,  [HasI1],  IIVir>;
defm INIT1  : Init<I1,  i1,  [HasI1],  IIVir>;
defm INIT8  : Init<I8,  i8,  [HasI8],  IIVir>;
defm INIT16 : Init<I16, i16, [HasI16], IIVir>;
defm INIT32 : Init<I32, i32, [HasI32], IIVir>;
defm INIT64 : Init<I64, i64, [HasI64], IIVir>;

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
