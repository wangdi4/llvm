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
                 list<Predicate> preds> {
  let Predicates = preds in {
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
                   list<Predicate> preds> {
  let Predicates = preds in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1),
      !strconcat(opStr, "\t$dst, $op1"),
      [(set RC:$dst, (opNode RC:$op1))]>;
  }
}

// Separate pattern because "not" is not an SDNode but a PatFrag
multiclass UnaryOpP<string opStr, PatFrag opNode, RegisterClass RC,
           list<Predicate> preds> {
  let Predicates = preds in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1),
      !strconcat(opStr, "\t$dst, $op1"),
      [(set RC:$dst, (opNode RC:$op1))]>;
  }
}

multiclass NegOp<string opStr, RegisterClass RC, list<Predicate> preds> {
  let Predicates = preds in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1),
      !strconcat(opStr, "\t$dst, $op1"),
      [(set RC:$dst, (sub 0, RC:$op1))]>;
  }
}

// Binary op, no pattern, not commutable
multiclass SExtOp<string opStr, RegisterClass RCt, RegisterClass RCs,
                  list<Predicate> preds> {
  let Predicates = preds in {
    def "" : FMTGEN<
      (outs RCt:$dst),
      (ins RCs:$op1, RCs:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      []>;
  }
}

multiclass CvtOp<string opStr, SDNode opNode, RegisterClass DT,
                 RegisterClass ST, list<Predicate> preds> {
  let Predicates = preds in {
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
           ValueType opType, SDNode immNode, list<Predicate> preds> {
  let isCommutable = 1, Predicates = preds in {
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
           ValueType opType, SDNode immNode, list<Predicate> preds> {
  let isCommutable = 0, Predicates = preds in {
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
           ValueType opType, SDNode immNode, list<Predicate> preds> {
  let isCommutable = 0, Predicates = preds in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, I8:$op2), 
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set RC:$dst, (opNode RC:$op1, I8:$op2))]>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, Operand<opType>:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set RC:$dst, (opNode RC:$op1, (opType immNode:$imm)))]>;
    // Only difference with BinOpC is this additional pattern, allowing
    // literal operation for the 1st of the 2 binary operands
    def i1 : FMTGEN<
      (outs RC:$dst),
      (ins Operand<opType>:$imm, I8:$op2),
      !strconcat(opStr, "\t$dst, $imm, $op2"),
      [(set RC:$dst, (opNode (opType immNode:$imm), I8:$op2))]>;
  }
}




multiclass FusedOp1<string opStr, SDNode inner, SDNode outer, RegisterClass RC,
           ValueType opType, list<Predicate> preds> {
  let Predicates = preds in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, RC:$op2, RC:$op3),
      !strconcat(opStr, "\t$dst, $op1, $op2, $op3"),
      [(set RC:$dst, (outer (inner RC:$op1, RC:$op2), RC:$op3))]>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, RC:$op2, Operand<opType>:$imm),
      !strconcat(opStr, "\t$dst, $op1, $op2, $imm"),
      [(set RC:$dst, (outer (inner RC:$op1, RC:$op2), (opType fpimm:$imm)))]>;
    def ix : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, Operand<opType>:$imm, RC:$op3),
      !strconcat(opStr, "\t$dst, $op1, $imm, $op3"),
      [(set RC:$dst, (outer (inner RC:$op1, (opType fpimm:$imm)), RC:$op3))]>;
    def ii : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, Operand<opType>:$imm2, RC:$imm3),
      !strconcat(opStr, "\t$dst, $op1, $imm2, $imm3"),
      [(set RC:$dst, (outer (inner RC:$op1, (opType fpimm:$imm2)),
                                                    (opType fpimm:$imm3)))]>;
  }
}

multiclass FusedOp2<string opStr, SDNode inner, SDNode outer, RegisterClass RC,
           ValueType opType, list<Predicate> preds> {
  let Predicates = preds in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, RC:$op2, RC:$op3),
      !strconcat(opStr, "\t$dst, $op2, $op3, $op1"),
      [(set RC:$dst, (outer RC:$op1, (inner RC:$op2, RC:$op3)))]>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins Operand<opType>:$imm, RC:$op2, RC:$op3),
      !strconcat(opStr, "\t$dst, $op2, $op3, $imm"),
      [(set RC:$dst, (outer (opType fpimm:$imm), (inner RC:$op2, RC:$op3)))]>;
  }
}

// CmpOp[N]C are identical to BinOp, except opNode is a PatFrag rather than
// an SDNode...
multiclass CmpOpC<string opStr, PatFrag opNode, RegisterClass RC,
           ValueType opType, list<Predicate> preds> {
  let isCommutable = 1, Predicates = preds in {
    def "" : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set I1:$dst, (opNode opType:$op1, opType:$op2))]>;
    def i : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, Operand<opType>:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set I1:$dst, (opNode opType:$op1, (opType imm:$imm)))]>;
  }
}

multiclass CmpOpNC<string opStr, PatFrag opNode, RegisterClass RC,
           ValueType opType, list<Predicate> preds> {
  let isCommutable = 0, Predicates = preds in {
    def "" : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set I1:$dst, (opNode opType:$op1, opType:$op2))]>;
    def i : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, Operand<opType>:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set I1:$dst, (opNode opType:$op1, (opType imm:$imm)))]>;
    def i1 : FMTGEN<
      (outs I1:$dst),
      (ins Operand<opType>:$imm, RC:$op2),
      !strconcat(opStr, "\t$dst, $imm, $op2"),
      [(set I1:$dst, (opNode (opType imm:$imm), opType:$op2))]>;
  }
}

// FCmpOp generates both ordered and unordered forms
multiclass FCmpOp<string opStr, PatFrag opNodeOrd, PatFrag opNodeUnord,
           RegisterClass RC, ValueType opType, bit commutable,
           list<Predicate> preds> {
  let isCommutable = commutable, Predicates = preds in {
    def "" : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set i1:$dst, (opNodeOrd opType:$op1, opType:$op2))]>;
    def u : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set i1:$dst, (opNodeUnord opType:$op1, opType:$op2))]>;
    def i : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, Operand<opType>:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set i1:$dst, (opNodeOrd opType:$op1, (opType fpimm:$imm)))]>;
    def ui : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, Operand<opType>:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set i1:$dst, (opNodeUnord opType:$op1, (opType fpimm:$imm)))]>;
  }
}

multiclass LdOp<string opStr, RegisterClass RC, ValueType opType> {
  let Predicates = [HasIndex] in {
    def X : FMTGEN< // indexed reg+reg
      (outs RC:$dst),
      (ins ADDR_RX:$addr),
      !strconcat(opStr, "x\t$dst, $addr"),
      [(set opType:$dst, (load ADDR_RX:$addr))]>;
  }

  let Predicates = [HasDispl] in {
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

// Ugly combinatorial issue between literal store data and addressing modes
// TODO: How to handle store output flag?
multiclass StOp<string opStr, RegisterClass RC, ValueType opType,
           SDNode immNode> {
  let Predicates = [HasIndex] in {
    def Xi : FMTGEN<
      (outs),
      (ins ADDR_RX:$addr, Operand<opType>:$data),
      !strconcat(opStr, "x\t%ign, $addr, $data"),
      [(store (opType immNode:$data), ADDR_RX:$addr)]>;
    def X : FMTGEN<
      (outs),
      (ins ADDR_RX:$addr, RC:$data),
      !strconcat(opStr, "x\t%ign, $addr, $data"),
      [(store opType:$data, ADDR_RX:$addr)]>;
  }

  let Predicates = [HasDispl] in {
    def Di : FMTGEN<
      (outs),
      (ins ADDR_RI:$addr, Operand<opType>:$data),
      !strconcat(opStr, "d\t%ign, $addr, $data"),
      [(store (opType immNode:$data), ADDR_RI:$addr)]>;
    def D : FMTGEN<
      (outs),
      (ins ADDR_RI:$addr, RC:$data),
      !strconcat(opStr, "d\t%ign, $addr, $data"),
      [(store opType:$data, ADDR_RI:$addr)]>;
    def Ri : FMTGEN<
      (outs),
      (ins ADDR_RR:$addr, Operand<opType>:$data),
      !strconcat(opStr, "d\t%ign, $addr, $data"),
      [(store (opType immNode:$data), ADDR_RR:$addr)]>;
    def R : FMTGEN<
      (outs),
      (ins ADDR_RR:$addr, RC:$data),
      !strconcat(opStr, "d\t%ign, $addr, $data"),
      [(store opType:$data, ADDR_RR:$addr)]>;
  }

  def i : FMTGEN<
    (outs),
    (ins ADDR_R:$addr, Operand<opType>:$data),
    !strconcat(opStr, "\t%ign, $addr, $data"),
    [(store (opType immNode:$data), ADDR_R:$addr)]>;
  def "" : FMTGEN<
    (outs),
    (ins ADDR_R:$addr, RC:$data),
    !strconcat(opStr, "\t%ign, $addr, $data"),
    [(store opType:$data, ADDR_R:$addr)]>;
}

let isBranch=1, isTerminator=1 in {
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

let isReturn=1, isTerminator=1, isBarrier=1 in
def RET : FMTGEN<
    (outs),
    (ins),
    "ret\t%ra",         // implicit use of RA
    [(LPURet)]>;

let isBranch=1, isTerminator=1, isBarrier=1 in
def JMP : FMTGEN<
  (outs),
  (ins I64:$target),
  "jmp\t$target",
  [(brind I64:$target)]>;

let isCall=1,
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

defm MOV1      : MovOp<"mov1",  I1,  i1,  [HasI1]>;
defm MOV8      : MovOp<"mov8",  I8,  i8,  [HasI8]>;
defm MOV16     : MovOp<"mov16", I16, i16, [HasI16]>;
defm MOV32     : MovOp<"mov32", I32, i32, [HasI32]>;
defm MOV64     : MovOp<"mov64", I64, i64, [HasI64]>;

// GlobalAddress, ExternalSymbol

def : Pat<(i64 (LPUWrapper tglobaladdr:$src)),   (MOV64i tglobaladdr:$src)>;
def : Pat<(i64 (LPUWrapper texternalsym:$src)),  (MOV64i texternalsym:$src)>;
def : Pat<(i64 (LPUWrapper tblockaddress:$src)), (MOV64i tblockaddress:$src)>;
def : Pat<(i64 (LPUWrapper tjumptable:$src)),    (MOV64i tjumptable:$src)>;

defm NOT1      : UnaryOpP<"not1",     not,   I1,  [HasI1]>;
defm NOT8      : UnaryOpP<"not8",     not,   I8,  [HasI8]>;
defm NOT16     : UnaryOpP<"not16",    not,   I16, [HasI16]>;
defm NOT32     : UnaryOpP<"not32",    not,   I32, [HasI32]>;
defm NOT64     : UnaryOpP<"not64",    not,   I64, [HasI64]>;
defm NEG8      : NegOp  <"neg8",             I8,  [HasI8]>;
defm NEG16     : NegOp  <"neg16",            I16, [HasI16]>;
defm NEG32     : NegOp  <"neg32",            I32, [HasI32]>;
defm NEG64     : NegOp  <"neg64",            I64, [HasI64]>;
defm NEGF16    : UnaryOp<"negf16",    fneg,  I16, [HasF16]>;
defm NEGF32    : UnaryOp<"negf32",    fneg,  I32, [HasF32]>;
defm NEGF64    : UnaryOp<"negf64",    fneg,  I64, [HasF64]>;
defm ABSF16    : UnaryOp<"absf16",    fabs,  I16, [HasF16]>;
defm ABSF32    : UnaryOp<"absf32",    fabs,  I32, [HasF32]>;
defm ABSF64    : UnaryOp<"absf64",    fabs,  I64, [HasF64]>;
defm SQRTF16   : UnaryOp<"sqrtf16",   fsqrt, I16, [HasF16,HasSqrt]>;
defm SQRTF32   : UnaryOp<"sqrtf32",   fsqrt, I32, [HasF32,HasSqrt]>;
defm SQRTF64   : UnaryOp<"sqrtf64",   fsqrt, I64, [HasF64,HasSqrt]>;
defm EXP2F16   : UnaryOp<"exp2f16",   fexp2, I16, [HasF16,HasMath0]>;
defm EXP2F32   : UnaryOp<"exp2f32",   fexp2, I32, [HasF32,HasMath0]>;
defm EXP2F64   : UnaryOp<"exp2f64",   fexp2, I64, [HasF64,HasMath0]>;
defm LOG2F16   : UnaryOp<"log2f16",   flog2, I16, [HasF16,HasMath0]>;
defm LOG2F32   : UnaryOp<"log2f32",   flog2, I32, [HasF32,HasMath0]>;
defm LOG2F64   : UnaryOp<"log2f64",   flog2, I64, [HasF64,HasMath0]>;
defm EXPF16    : UnaryOp<"expf16",    fexp,  I16, [HasF16,HasMath0]>;
defm EXPF32    : UnaryOp<"expf32",    fexp,  I32, [HasF32,HasMath0]>;
defm EXPF64    : UnaryOp<"expf64",    fexp,  I64, [HasF64,HasMath0]>;
defm LOGF16    : UnaryOp<"logf16",    flog,  I16, [HasF16,HasMath0]>;
defm LOGF32    : UnaryOp<"logf32",    flog,  I32, [HasF32,HasMath0]>;
defm LOGF64    : UnaryOp<"logf64",    flog,  I64, [HasF64,HasMath0]>;
defm SINF16    : UnaryOp<"sinf16",    fsin,  I16, [HasF16,HasMath0]>;
defm SINF32    : UnaryOp<"sinf32",    fsin,  I32, [HasF32,HasMath0]>;
defm SINF64    : UnaryOp<"sinf64",    fsin,  I64, [HasF64,HasMath0]>;
defm COSF16    : UnaryOp<"cosf16",    fcos,  I16, [HasF16,HasMath0]>;
defm COSF32    : UnaryOp<"cosf32",    fcos,  I32, [HasF32,HasMath0]>;
defm COSF64    : UnaryOp<"cosf64",    fcos,  I64, [HasF64,HasMath0]>;
//defm SINCOSF16 : UnaryOp<"sincosf16", fsincos, I16, [HasF16,HasMath0]>;
//defm SINCOSF32 : UnaryOp<"sincosf32", fsincos, I32, [HasF32,HasMath0]>;
//defm SINCOSF64 : UnaryOp<"sincosf64", fsincos, I64, [HasF64,HasMath0]>;

// todo: int<=>float Conversions
// first type in convert name is result, second is source type
defm CVTS32F32 : CvtOp<"cvts32f32", fp_to_sint, I32, I32, [HasI32,HasF32]>;
defm CVTS32F64 : CvtOp<"cvts32f64", fp_to_sint, I32, I64, [HasI32,HasF64]>;
defm CVTU32F32 : CvtOp<"cvtu32f32", fp_to_uint, I32, I32, [HasI32,HasF32]>;
defm CVTU32F64 : CvtOp<"cvtu32f64", fp_to_uint, I32, I64, [HasI32,HasF64]>;
defm CVTS64F32 : CvtOp<"cvts64f32", fp_to_sint, I64, I32, [HasI64,HasF32]>;
defm CVTS64F64 : CvtOp<"cvts64f64", fp_to_sint, I64, I64, [HasI64,HasF64]>;
defm CVTU64F32 : CvtOp<"cvtu64f32", fp_to_uint, I64, I32, [HasI64,HasF32]>;
defm CVTU64F64 : CvtOp<"cvtu64f64", fp_to_uint, I64, I64, [HasI64,HasF64]>;

defm CVTF32S32 : CvtOp<"cvtf32s32", sint_to_fp, I32, I32, [HasF32,HasI32]>;
defm CVTF32S64 : CvtOp<"cvtf32s64", sint_to_fp, I32, I64, [HasF32,HasI64]>;
defm CVTF32U32 : CvtOp<"cvtf32u32", uint_to_fp, I32, I32, [HasF32,HasI32]>;
defm CVTF32U64 : CvtOp<"cvtf32u64", uint_to_fp, I32, I64, [HasF32,HasI64]>;
defm CVTF64S32 : CvtOp<"cvtf64s32", sint_to_fp, I64, I32, [HasF64,HasI32]>;
defm CVTF64S64 : CvtOp<"cvtf64s64", sint_to_fp, I64, I64, [HasF64,HasI64]>;
defm CVTF64U32 : CvtOp<"cvtf64u32", uint_to_fp, I64, I32, [HasF64,HasI32]>;
defm CVTF64U64 : CvtOp<"cvtf64u64", uint_to_fp, I64, I64, [HasF64,HasI64]>;

defm CVTF32F64 : CvtOp<"cvtf32f64", fround,   I32, I64, [HasF32,HasF64]>;
defm CVTF64F32 : CvtOp<"cvtf64f32", fextend,  I64, I32, [HasF32,HasF64]>;

defm SEXT8     : SExtOp<"sext8",  I8,  I8,  [HasI8]>;
defm SEXT16_8  : SExtOp<"sext16", I16, I8,  [HasI16]>;
defm SEXT16    : SExtOp<"sext16", I16, I16, [HasI16]>;
defm SEXT32_8  : SExtOp<"sext32", I32, I8,  [HasI32]>;
defm SEXT32_16 : SExtOp<"sext32", I32, I16, [HasI32]>;
defm SEXT32    : SExtOp<"sext32", I32, I32, [HasI32]>;
defm SEXT64_8  : SExtOp<"sext64", I64, I8,  [HasI64]>;
defm SEXT64_16 : SExtOp<"sext64", I64, I16, [HasI64]>;
defm SEXT64_32 : SExtOp<"sext64", I64, I32, [HasI64]>;
defm SEXT64    : SExtOp<"sext64", I64, I64, [HasI64]>;

defm AND1      : BinOpC< "and1",   and,  I1,  i1,  imm,   [HasI1]>;
defm AND8      : BinOpC< "and8",   and,  I8,  i8,  imm,   [HasI8]>;
defm AND16     : BinOpC< "and16",  and,  I16, i16, imm,   [HasI16]>;
defm AND32     : BinOpC< "and32",  and,  I32, i32, imm,   [HasI32]>;
defm AND64     : BinOpC< "and64",  and,  I64, i64, imm,   [HasI64]>;

defm OR1       : BinOpC< "or1",    or,   I1,  i1,  imm,   [HasI1]>;
defm OR8       : BinOpC< "or8",    or,   I8,  i8,  imm,   [HasI8]>;
defm OR16      : BinOpC< "or16",   or,   I16, i16, imm,   [HasI16]>;
defm OR32      : BinOpC< "or32",   or,   I32, i32, imm,   [HasI32]>;
defm OR64      : BinOpC< "or64",   or,   I64, i64, imm,   [HasI64]>;

defm XOR1      : BinOpC< "xor1",   xor,  I1,  i1,  imm,   [HasI1]>;
defm XOR8      : BinOpC< "xor8",   xor,  I8,  i8,  imm,   [HasI8]>;
defm XOR16     : BinOpC< "xor16",  xor,  I16, i16, imm,   [HasI16]>;
defm XOR32     : BinOpC< "xor32",  xor,  I32, i32, imm,   [HasI32]>;
defm XOR64     : BinOpC< "xor64",  xor,  I64, i64, imm,   [HasI64]>;

defm SLL8      : ShiftOp<"sll8",   shl,  I8,  i8,  imm,   [HasI8]>;
defm SLL16     : ShiftOp<"sll16",  shl,  I16, i16, imm,   [HasI16]>;
defm SLL32     : ShiftOp<"sll32",  shl,  I32, i32, imm,   [HasI32]>;
defm SLL64     : ShiftOp<"sll64",  shl,  I64, i64, imm,   [HasI64]>;

defm SRL8      : ShiftOp<"srl8",   srl,  I8,  i8,  imm,   [HasI8]>;
defm SRL16     : ShiftOp<"srl16",  srl,  I16, i16, imm,   [HasI16]>;
defm SRL32     : ShiftOp<"srl32",  srl,  I32, i32, imm,   [HasI32]>;
defm SRL64     : ShiftOp<"srl64",  srl,  I64, i64, imm,   [HasI64]>;

defm SRA8      : ShiftOp<"sra8",   sra,  I8,  i8,  imm,   [HasI8]>;
defm SRA16     : ShiftOp<"sra16",  sra,  I16, i16, imm,   [HasI16]>;
defm SRA32     : ShiftOp<"sra32",  sra,  I32, i32, imm,   [HasI32]>;
defm SRA64     : ShiftOp<"sra64",  sra,  I64, i64, imm,   [HasI64]>;

defm ADD8      : BinOpC< "add8",   add,  I8,  i8,  imm,   [HasI8]>;
defm ADD16     : BinOpC< "add16",  add,  I16, i16, imm,   [HasI16]>;
defm ADD32     : BinOpC< "add32",  add,  I32, i32, imm,   [HasI32]>;
defm ADD64     : BinOpC< "add64",  add,  I64, i64, imm,   [HasI64]>;

defm ADDF16    : BinOpC< "addf16", fadd, I16, f16, fpimm, [HasF16]>;
defm ADDF32    : BinOpC< "addf32", fadd, I32, f32, fpimm, [HasF32]>;
defm ADDF64    : BinOpC< "addf64", fadd, I64, f64, fpimm, [HasF64]>;

defm SUB8      : BinOpNC<"sub8",   sub,  I8,  i8,  imm,   [HasI8]>;
defm SUB16     : BinOpNC<"sub16",  sub,  I16, i16, imm,   [HasI16]>;
defm SUB32     : BinOpNC<"sub32",  sub,  I32, i32, imm,   [HasI32]>;
defm SUB64     : BinOpNC<"sub64",  sub,  I64, i64, imm,   [HasI64]>;

defm SUBF16    : BinOpNC<"subf16", fsub, I16, f16, fpimm, [HasF16]>;
defm SUBF32    : BinOpNC<"subf32", fsub, I32, f32, fpimm, [HasF32]>;
defm SUBF64    : BinOpNC<"subf64", fsub, I64, f64, fpimm, [HasF64]>;

defm MUL8      : BinOpC< "mul8",   mul,  I8,  i8,  imm,   [HasI8]>;
defm MUL16     : BinOpC< "mul16",  mul,  I16, i16, imm,   [HasI16]>;
defm MUL32     : BinOpC< "mul32",  mul,  I32, i32, imm,   [HasI32]>;
defm MUL64     : BinOpC< "mul64",  mul,  I64, i64, imm,   [HasI64]>;

defm MULF16    : BinOpC<"mulf16",  fmul, I16, f16, fpimm, [HasF16]>;
defm MULF32    : BinOpC<"mulf32",  fmul, I32, f32, fpimm, [HasF32]>;
defm MULF64    : BinOpC<"mulf64",  fmul, I64, f64, fpimm, [HasF64]>;

defm DIVS8     : BinOpNC<"divs8",  sdiv, I8,  i8,  imm,   [HasI8,HasIDiv]>;
defm DIVS16    : BinOpNC<"divs16", sdiv, I16, i16, imm,   [HasI16,HasIDiv]>;
defm DIVS32    : BinOpNC<"divs32", sdiv, I32, i32, imm,   [HasI32,HasIDiv]>;
defm DIVS64    : BinOpNC<"divs64", sdiv, I64, i64, imm,   [HasI64,HasIDiv]>;

defm DIVU8     : BinOpNC<"divu8",  udiv, I8,  i8,  imm,   [HasI8,HasIDiv]>;
defm DIVU16    : BinOpNC<"divu16", udiv, I16, i16, imm,   [HasI16,HasIDiv]>;
defm DIVU32    : BinOpNC<"divu32", udiv, I32, i32, imm,   [HasI32,HasIDiv]>;
defm DIVU64    : BinOpNC<"divu64", udiv, I64, i64, imm,   [HasI64,HasIDiv]>;

defm DIVF16    : BinOpNC<"divf16", fdiv, I16, f16, fpimm, [HasF16,HasFDiv]>;
defm DIVF32    : BinOpNC<"divf32", fdiv, I32, f32, fpimm, [HasF32,HasFDiv]>;
defm DIVF64    : BinOpNC<"divf64", fdiv, I64, f64, fpimm, [HasF64,HasFDiv]>;

defm POWF32    : BinOpNC<"powf32",   fpow,  I32, f32, fpimm, [HasF32,HasMath0]>;
defm POWF64    : BinOpNC<"powf64",   fpow,  I64, f64, fpimm, [HasF64,HasMath0]>;

defm CMPLTS8  : CmpOpNC<"cmplts8", setlt,  I8,  i8,  [HasI8]>;
defm CMPLTS16 : CmpOpNC<"cmplts16",setlt,  I16, i16, [HasI16]>;
defm CMPLTS32 : CmpOpNC<"cmplts32",setlt,  I32, i32, [HasI32]>;
defm CMPLTS64 : CmpOpNC<"cmplts64",setlt,  I64, i64, [HasI64]>;

defm CMPLTU8  : CmpOpNC<"cmpltu8", setult, I8,  i8,  [HasI8]>;
defm CMPLTU16 : CmpOpNC<"cmpltu16",setult, I16, i16, [HasI16]>;
defm CMPLTU32 : CmpOpNC<"cmpltu32",setult, I32, i32, [HasI32]>;
defm CMPLTU64 : CmpOpNC<"cmpltu64",setult, I64, i64, [HasI64]>;

defm CMPLES8  : CmpOpNC<"cmples8", setle,  I8,  i8,  [HasI8]>;
defm CMPLES16 : CmpOpNC<"cmples16",setle,  I16, i16, [HasI16]>;
defm CMPLES32 : CmpOpNC<"cmples32",setle,  I32, i32, [HasI32]>;
defm CMPLES64 : CmpOpNC<"cmples64",setle,  I64, i64, [HasI64]>;

defm CMPLEU8  : CmpOpNC<"cmpleu8", setule, I8,  i8,  [HasI8]>;
defm CMPLEU16 : CmpOpNC<"cmpleu16",setule, I16, i16, [HasI16]>;
defm CMPLEU32 : CmpOpNC<"cmpleu32",setule, I32, i32, [HasI32]>;
defm CMPLEU64 : CmpOpNC<"cmpleu64",setule, I64, i64, [HasI64]>;

// Note - there is no cmpeq1.  As long as operands are clean, cmpeq8
// suffices
defm CMPEQ1   : CmpOpC<"cmpeq8",   seteq,  I1,  i1,  [HasI1]>;
defm CMPEQ8   : CmpOpC<"cmpeq8",   seteq,  I8,  i8,  [HasI8]>;
defm CMPEQ16  : CmpOpC<"cmpeq16",  seteq,  I16, i16, [HasI16]>;
defm CMPEQ32  : CmpOpC<"cmpeq32",  seteq,  I32, i32, [HasI32]>;
defm CMPEQ64  : CmpOpC<"cmpeq64",  seteq,  I64, i64, [HasI64]>;

defm CMPGTS8  : CmpOpNC<"cmpgts8", setgt,  I8,  i8,  [HasI8]>;
defm CMPGTS16 : CmpOpNC<"cmpgts16",setgt,  I16, i16, [HasI16]>;
defm CMPGTS32 : CmpOpNC<"cmpgts32",setgt,  I32, i32, [HasI32]>;
defm CMPGTS64 : CmpOpNC<"cmpgts64",setgt,  I64, i64, [HasI64]>;

defm CMPGTU8  : CmpOpNC<"cmpgtu8", setugt, I8,  i8,  [HasI8]>;
defm CMPGTU16 : CmpOpNC<"cmpgtu16",setugt, I16, i16, [HasI16]>;
defm CMPGTU32 : CmpOpNC<"cmpgtu32",setugt, I32, i32, [HasI32]>;
defm CMPGTU64 : CmpOpNC<"cmpgtu64",setugt, I64, i64, [HasI64]>;

defm CMPGES8  : CmpOpNC<"cmpges8", setge,  I8,  i8,  [HasI8]>;
defm CMPGES16 : CmpOpNC<"cmpges16",setge,  I16, i16, [HasI16]>;
defm CMPGES32 : CmpOpNC<"cmpges32",setge,  I32, i32, [HasI32]>;
defm CMPGES64 : CmpOpNC<"cmpges64",setge,  I64, i64, [HasI64]>;

defm CMPGEU8  : CmpOpNC<"cmpgeu8", setuge, I8,  i8,  [HasI8]>;
defm CMPGEU16 : CmpOpNC<"cmpgeu16",setuge, I16, i16, [HasI16]>;
defm CMPGEU32 : CmpOpNC<"cmpgeu32",setuge, I32, i32, [HasI32]>;
defm CMPGEU64 : CmpOpNC<"cmpgeu64",setuge, I64, i64, [HasI64]>;

def : Pat<(i1 (setne i1:$op1, i1:$op2)), (XOR1 $op1,$op2)>;
def : Pat<(i1 (setne i1:$op1, (i1 imm:$imm))), (XOR1i $op1,$imm)>;
defm CMPNE8   : CmpOpC<"cmpne8",   setne,  I8,  i8,  [HasI8]>;
defm CMPNE16  : CmpOpC<"cmpne16",  setne,  I16, i16, [HasI16]>;
defm CMPNE32  : CmpOpC<"cmpne32",  setne,  I32, i32, [HasI32]>;
defm CMPNE64  : CmpOpC<"cmpne64",  setne,  I64, i64, [HasI64]>;

defm CMPLTF16 : FCmpOp<"cmpltf16", setolt, setult, I16, f16, 0, [HasF16]>;
defm CMPLTF32 : FCmpOp<"cmpltf32", setolt, setult, I32, f32, 0, [HasF32]>;
defm CMPLTF64 : FCmpOp<"cmpltf64", setolt, setult, I64, f64, 0, [HasF64]>;

defm CMPLEF16 : FCmpOp<"cmplef16", setole, setule, I16, f16, 0, [HasF16]>;
defm CMPLEF32 : FCmpOp<"cmplef32", setole, setule, I32, f32, 0, [HasF32]>;
defm CMPLEF64 : FCmpOp<"cmplef64", setole, setule, I64, f64, 0, [HasF64]>;

defm CMPEQF16 : FCmpOp<"cmpeqf16", setoeq, setueq, I16, f16, 1, [HasF16]>;
defm CMPEQF32 : FCmpOp<"cmpeqf32", setoeq, setueq, I32, f32, 1, [HasF32]>;
defm CMPEQF64 : FCmpOp<"cmpeqf64", setoeq, setueq, I64, f64, 1, [HasF64]>;

defm CMPGTF16 : FCmpOp<"cmpgtf16", setogt, setugt, I16, f16, 0, [HasF16]>;
defm CMPGTF32 : FCmpOp<"cmpgtf32", setogt, setugt, I32, f32, 0, [HasF32]>;
defm CMPGTF64 : FCmpOp<"cmpgtf64", setogt, setugt, I64, f64, 0, [HasF64]>;

defm CMPGEF16 : FCmpOp<"cmpgef16", setoge, setuge, I16, f16, 0, [HasF16]>;
defm CMPGEF32 : FCmpOp<"cmpgef32", setoge, setuge, I32, f32, 0, [HasF32]>;
defm CMPGEF64 : FCmpOp<"cmpgef64", setoge, setuge, I64, f64, 0, [HasF64]>;

defm CMPNEF16 : FCmpOp<"cmpnef16", setone, setune, I16, f16, 1, [HasF16]>;
defm CMPNEF32 : FCmpOp<"cmpnef32", setone, setune, I32, f32, 1, [HasF32]>;
defm CMPNEF64 : FCmpOp<"cmpnef64", setone, setune, I64, f64, 1, [HasF64]>;

// Drop?
// todo: def BSEL8
// todo: def BSEL16
// todo: def BSEL32
// todo: def BSEL64

def LAND1 : FMTGEN<
  (outs I1:$dst),
  (ins I1:$op1, I1:$op2, I1:$op3, I1:$op4),
  "land1\t$dst, $op1, $op2, $op3, $op4",
  []>;

defm FMAF16  : FusedOp1<"fmaf16",  fmul, fadd, I16, f16, [HasF16,HasFMA]>;
// crude handling of commutative
defm FMAF16x : FusedOp2<"fmaf16",  fmul, fadd, I16, f16, [HasF16,HasFMA]>;
defm FMSF16  : FusedOp1<"fmsf16",  fmul, fsub, I16, f16, [HasF16,HasFMA]>;
defm FMRSF16 : FusedOp2<"fmrsf16", fmul, fsub, I16, f16, [HasF16,HasFMA]>;

defm FMAF32  : FusedOp1<"fmaf32",  fmul, fadd, I32, f32, [HasF32,HasFMA]>;
// crude handling of commutative
defm FMAF32x : FusedOp2<"fmaf32",  fmul, fadd, I32, f32, [HasF32,HasFMA]>;
defm FMSF32  : FusedOp1<"fmsf32",  fmul, fsub, I32, f32, [HasF32,HasFMA]>;
defm FMRSF32 : FusedOp2<"fmrsf32", fmul, fsub, I32, f32, [HasF32,HasFMA]>;

defm FMAF64  : FusedOp1<"fmaf64",  fmul, fadd, I64, f64, [HasF64,HasFMA]>;
// crude handling of commutative
defm FMAF64x : FusedOp2<"fmaf64",  fmul, fadd, I64, f64, [HasF64,HasFMA]>;
defm FMSF64  : FusedOp1<"fmsf64",  fmul, fsub, I64, f64, [HasF64,HasFMA]>;
defm FMRSF64 : FusedOp2<"fmrsf64", fmul, fsub, I64, f64, [HasF64,HasFMA]>;

// Should we do these, or drop and assume sext-at-consumer?
// todo: LDS8
// todo: LDS16
// todo: LDS32
defm LD8   : LdOp<"ld8",   I8,  i8>;
defm LD16  : LdOp<"ld16",  I16, i16>;
defm LD16f : LdOp<"ld16",  I16, f16>;
defm LD32  : LdOp<"ld32",  I32, i32>;
defm LD32f : LdOp<"ld32",  I32, f32>;
defm LD64  : LdOp<"ld64",  I64, i64>;
defm LD64f : LdOp<"ld64",  I64, f64>;

defm ST8   : StOp<"st8",   I8,  i8,  imm>;
defm ST16  : StOp<"st16",  I16, i16, imm>;
defm ST16f : StOp<"st16",  I16, f16, fpimm>;
defm ST32  : StOp<"st32",  I32, i32, imm>;
defm ST32f : StOp<"st32",  I32, f32, fpimm>;
defm ST64  : StOp<"st64",  I64, i64, imm>;
defm ST64f : StOp<"st64",  I64, f64, fpimm>;

multiclass CopyOp<string opStr, RegisterClass RC> {
  def "" : FMTGEN<
    (outs RC:$op0, RC:$op1, RC:$op2, RC:$op3),
    (ins RC:$op4),
    !strconcat(opStr, "\t$op0, $op1, $op2, $op3, $op4"),
    []>;
}
defm COPY0   : CopyOp<"copy0",  I1>;
defm COPY1   : CopyOp<"copy1",  I1>;
defm COPY8   : CopyOp<"copy8",  I8>;
defm COPY16  : CopyOp<"copy16", I16>;
defm COPY32  : CopyOp<"copy32", I32>;
defm COPY64  : CopyOp<"copy64", I64>;

multiclass MergeOp<string opStr, RegisterClass RC, ValueType opType,
                   SDNode immNode> {
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
defm MERGE1   : MergeOp<"merge1",  I1,  i1,  imm>;
defm MERGE8   : MergeOp<"merge8",  I8,  i8,  imm>;
defm MERGE16  : MergeOp<"merge16", I16, i16, imm>;
defm MERGE16f : MergeOp<"merge16", I16, f16, fpimm>;
defm MERGE32  : MergeOp<"merge32", I32, i32, imm>;
defm MERGE32f : MergeOp<"merge32", I32, f32, fpimm>;
defm MERGE64  : MergeOp<"merge64", I64, i64, imm>;
defm MERGE64f : MergeOp<"merge64", I64, f64, fpimm>;

multiclass SwitchOp<string opStr, RegisterClass RC> {
  def "" : FMTGEN<
    (outs RC:$dst0, RC:$dst1),
    (ins I1:$sel, RC:$v),
    !strconcat(opStr, "\t$dst0, $dst1, $sel, $v"),
    []>;
}
defm SWITCH1   : SwitchOp<"switch1",  I1>;
defm SWITCH8   : SwitchOp<"switch8",  I8>;
defm SWITCH16  : SwitchOp<"switch16", I16>;
defm SWITCH32  : SwitchOp<"switch32", I32>;
defm SWITCH64  : SwitchOp<"switch64", I64>;

multiclass SwitchAnyOp<string opStr, RegisterClass RC> {
  def "" : FMTGEN<
    (outs RC:$dst0, RC:$dst1, I1:$sel),
    (ins RC:$v),
    !strconcat(opStr, "\t$dst0, $dst1, $sel, $v"),
    []>;
}
defm SWITCHANY1  : SwitchAnyOp<"switchany1",  I1>;
defm SWITCHANY8  : SwitchAnyOp<"switchany8",  I8>;
defm SWITCHANY16 : SwitchAnyOp<"switchany16", I16>;
defm SWITCHANY32 : SwitchAnyOp<"switchany32", I32>;
defm SWITCHANY64 : SwitchAnyOp<"switchany64", I64>;

multiclass PickOp<string opStr, RegisterClass RC> {
  def "" : FMTGEN<
    (outs RC:$dst),
    (ins I1:$sel, RC:$v0, RC:$v1),
    !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
    []>;
}
defm PICK1   : PickOp<"pick1",  I1>;
defm PICK8   : PickOp<"pick8",  I8>;
defm PICK16  : PickOp<"pick16", I16>;
defm PICK32  : PickOp<"pick32", I32>;
defm PICK64  : PickOp<"pick64", I64>;

multiclass PickAnyOp<string opStr, RegisterClass RC> {
  def "" : FMTGEN<
    (outs RC:$dst, I1:$sel),
    (ins RC:$v0, RC:$v1),
    !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
    []>;
}
defm PICKANY1   : PickAnyOp<"pickany1",  I1>;
defm PICKANY8   : PickAnyOp<"pickany8",  I8>;
defm PICKANY16  : PickAnyOp<"pickany16", I16>;
defm PICKANY32  : PickAnyOp<"pickany32", I32>;
defm PICKANY64  : PickAnyOp<"pickany64", I64>;

def ANY0 : FMTGEN<
  (outs I64:$dst),
  (ins I0:$op1, I0:$op2, I0:$op3, I0:$op4),
  "any0\t$dst, $op1, $op2, $op3, $op4",
  []>;

def ALL0 : FMTGEN<
  (outs I0:$dst),
  (ins I0:$op1, I0:$op2, I0:$op3, I0:$op4),
  "all0\t$dst, $op1, $op2, $op3, $op4",
  []>;

def ONCOUNT0 : FMTGEN<  // TODO: something to reflect state
  (outs I0:$dst),
  (ins I64:$cnt, I0:$op2, I0:$op3, I0:$op4, I0:$op5),
  "oncount0\t$dst, $cnt, $op2, $op3, $op4, $op5",
  []>;

def SEQC64 : FMTGEN<  // TODO: something to reflect state
  (outs I64:$val, I1:$pred),
  (ins I64:$base, I64:$count, I64:$stride),
  "seqc64\t$val, $pred, $base, $count, $stride",
  []>;

def SEQS64 : FMTGEN<  // TODO: something to reflect state
  (outs I64:$val, I1:$pred),
  (ins I64:$base, I64:$bound, I64:$stride),
  "seqs64\t$val, $pred, $base, $bound, $stride",
  []>;

def REPEAT64 : FMTGEN<  // TODO: something to reflect state
  (outs I64:$out),
  (ins I1:$pred, I64:$in),
  "repeat64\t$out, $pred, $in",
  []>;

// todo: ORDER

// todo: DISAM

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

def : Pat<(i32 (sext i16:$op1)),
  (SEXT32_16 (i16 $op1), 16)>;
def : Pat<(i32 (sext i8:$op1)),
  (SEXT32_8 (i8 $op1), 8)>;

def : Pat<(i16 (sext i8:$op1)),
  (SEXT16_8 (i8 $op1), 8)>;

// sext_inreg are the same
def : Pat<(i64 (sext_inreg i64:$op1, i32)),
  (SEXT64 $op1, 32)>;
def : Pat<(i64 (sext_inreg i64:$op1, i16)),
  (SEXT64 $op1, 16)>;
def : Pat<(i64 (sext_inreg i64:$op1, i8)),
  (SEXT64 $op1, 8)>;

def : Pat<(i32 (sext_inreg i32:$op1, i16)),
  (SEXT32 $op1, 16)>;
def : Pat<(i32 (sext_inreg i32:$op1, i8)),
  (SEXT32 $op1, 8)>;

def : Pat<(i16 (sext_inreg i16:$op1, i8)),
  (SEXT16 $op1, 8)>;

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

// Eventually
//include "LPUIntrinsics.td"
