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

// Immediates of a particular type.
def immi1        : PatLeaf<(i1 imm)>;
def immi8        : PatLeaf<(i8 imm)>;
def immi16       : PatLeaf<(i16 imm)>;
def immi32       : PatLeaf<(i32 imm)>;
def immi64       : PatLeaf<(i64 imm)>;
def immf32       : PatLeaf<(f32 fpimm)>;
def immf64       : PatLeaf<(f64 fpimm)>;

// Memory operands
class Addr<int numArgs, string funcName, dag opInfo> :
  Operand<i64>, ComplexPattern<i64, numArgs,
    funcName, [],
    [SDNPWantParent]> { let MIOperandInfo = opInfo; }

def ADDR_R : Addr<1, "SelectAddrReg", (ops i64)>;


//===----------------------------------------------------------------------===//
// LPU Instruction Predicate Definitions
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// LPU Instructions.
//===----------------------------------------------------------------------===//

// These are target-independent nodes, but have target-specific formats.
def SDT_LPUCallSeqStart : SDCallSeqStart<[ SDTCisVT<0, i32> ]>;
def SDT_LPUCallSeqEnd   : SDCallSeqEnd<[ SDTCisVT<0, i32>,
                                        SDTCisVT<1, i32> ]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_LPUCallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END",   SDT_LPUCallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def ADJCALLSTACKDOWN : PseudoInstLPU<(outs), (ins i32imm:$amt),
                               "# ADJCALLSTACKDOWN $amt",
                               [(callseq_start timm:$amt)]>;
def ADJCALLSTACKUP : PseudoInstLPU<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                            "# ADJCALLSTACKUP $amt1",
                            [(callseq_end timm:$amt1, timm:$amt2)]>;

def SDT_LPURet : SDTypeProfile<0, 0, []>;

def LPURet : SDNode<"LPUISD::Ret", SDT_LPURet,
    [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;


let isReturn=1, isTerminator=1, isBarrier=1 in
def RET : FMTGEN<
    (outs), (ins),
    "mov0\tc0, c1",      // very temporary until control flow handled
    [(LPURet)]>;

multiclass MovOp<string opStr, RegisterClass LICt, PatFrag immPat, Operand immOp> {
  def "" : FMTGEN<
    (outs LICt:$dst),
    (ins LICt:$op1),
    !strconcat(opStr, "\t$dst, $op1"),
    []>;
  def i : FMTGEN<
    (outs LICt:$dst),
    (ins immOp:$imm),
    !strconcat(opStr, "\t$dst, $imm"),
    []>;
}

// Binary op, no pattern, not commutable
multiclass SExtOp<string opStr, RegisterClass RCt, RegisterClass RCs> {
  def "" : FMTGEN<
    (outs RCt:$dst),
    (ins RCs:$op1, RCs:$op2),
    !strconcat(opStr, "\t$dst, $op1, $op2"),
    []>;
}

multiclass CvtOp<string opStr, SDNode opNode, RegisterClass DT, RegisterClass ST> {
  def "" : FMTGEN<
    (outs DT:$dst),
    (ins ST:$src),
    !strconcat(opStr, "\t$dst, $src"),
    [(set DT:$dst, (opNode ST:$src))]>;
}

// This uses a multiclass even though it only declares one class because
// class doesn't allow the Opndxx in the ins/outs list.  (Why?)
// (Also, nested multiclass would be nice - it would allow declaring 4 defs
// per operator, rather than enumerating 8/16/32/64, etc.)
multiclass BinOp<string opStr, SDNode opNode, RegisterClass RC,
           PatFrag immPat, Operand immOpnd, bit commutable> {
  let isCommutable = commutable in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set RC:$dst, (opNode RC:$op1, RC:$op2))]>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, immOpnd:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set RC:$dst, (opNode RC:$op1, immPat:$imm))]>;
  }
}

multiclass FusedOp1<string opStr, SDNode inner, SDNode outer, RegisterClass RC> {
  def "" : FMTGEN<
    (outs RC:$dst),
    (ins RC:$op1, RC:$op2, RC:$op3),
    !strconcat(opStr, "\t$dst, $op1, $op2, $op3"),
    [(set RC:$dst, (outer (inner RC:$op1, RC:$op2), RC:$op3))]>;
}

multiclass FusedOp2<string opStr, SDNode inner, SDNode outer, RegisterClass RC> {
  def "" : FMTGEN<
    (outs RC:$dst),
    (ins RC:$op1, RC:$op2, RC:$op3),
    !strconcat(opStr, "\t$dst, $op2, $op3, $op1"),
    [(set RC:$dst, (outer RC:$op1, (inner RC:$op2, RC:$op3)))]>;
}

multiclass CmpOp<string opStr, PatFrag opNode, RegisterClass RC,
           PatFrag immPat, Operand immOpnd, bit commutable> {
  let isCommutable = commutable in {
    def "" : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set I1:$dst, (opNode RC:$op1, RC:$op2))]>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, immOpnd:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set RC:$dst, (opNode RC:$op1, immPat:$imm))]>;
  }
}

multiclass FCmpOp<string opStr, PatFrag opNodeOrd, PatFrag opNodeUnord,
           RegisterClass RC, bit commutable> {
  let isCommutable = commutable in {
    def "" : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set i1:$dst, (opNodeOrd RC:$op1, RC:$op2))]>;
    def u : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set i1:$dst, (opNodeUnord RC:$op1, RC:$op2))]>;
  }
}

// This provides a full set of result register types.  Some are nonsensical
// (e.g. LD8_F64 or LD32_I64)
multiclass LdOp<string opStr, RegisterClass RC> {
  def "" : FMTGEN<
    (outs RC:$dst),
    (ins I64:$addr),
    !strconcat(opStr, "\t$dst, $addr"),
    [(set RC:$dst, (load ADDR_R:$addr))]>;
}

// TODO: How to handle store output flag?
multiclass StOp<string opStr, RegisterClass RC, PatFrag immPat, Operand immOpnd> {
  def "" : FMTGEN<
    (outs),  // How to handle output flag??
    (ins I64:$addr, RC:$data),
    !strconcat(opStr, "\t$addr, $data"),
    [(store RC:$data, ADDR_R:$addr)]>;
  def i : FMTGEN<
    (outs),  // How to handle output flag??
    (ins I64:$addr, immOpnd:$data),
    !strconcat(opStr, "\t$addr, $data"),
    [(store immPat:$data, ADDR_R:$addr)]>;
}

// The repetition below could be removed if nested multiclass were supported.
// (e.g. the first level would expand to 8/16/32/64, then the 2nd level
// would be as today.)

defm MOV1  : MovOp<"mov1",  I1,  immi1,  i1imm>;
defm MOV8  : MovOp<"mov8",  I8,  immi8,  i8imm>;
defm MOV16 : MovOp<"mov16", I16, immi16, i16imm>;
defm MOV32 : MovOp<"mov32", I32, immi32, i32imm>;
defm MOV32f: MovOp<"mov32", F32, immf32, f32imm>;
defm MOV64 : MovOp<"mov64", I64, immi64, i64imm>;
defm MOV64f: MovOp<"mov64", F64, immf64, f64imm>;

// todo: NOT1
// todo: NOT8
// todo: NOT16
// todo: NOT32
// todo: NOT64
// todo: NEG8
// todo: NEGS16
// todo: NEGS32
// todo: NEGS64
// todo: NEGF32
// todo: NEGF64
// todo: ABSF32
// todo: ABSF64
// todo: SQRTF32
// todo: SQRTF64

// todo: int<=>float Conversions
// first type in convert name is result, second is source type
defm CVTS32F32 : CvtOp<"cvts32f32", fp_to_sint, I32, F32>;
defm CVTS32F64 : CvtOp<"cvts32f64", fp_to_sint, I32, F64>;
defm CVTU32F32 : CvtOp<"cvtu32f32", fp_to_uint, I32, F32>;
defm CVTU32F64 : CvtOp<"cvtu32f64", fp_to_uint, I32, F64>;
defm CVTS64F32 : CvtOp<"cvts64f32", fp_to_sint, I64, F32>;
defm CVTS64F64 : CvtOp<"cvts64f64", fp_to_sint, I64, F64>;
defm CVTU64F32 : CvtOp<"cvtu64f32", fp_to_uint, I64, F32>;
defm CVTU64F64 : CvtOp<"cvtu64f64", fp_to_uint, I64, F64>;

defm CVTF32S32 : CvtOp<"cvtf32s32", sint_to_fp, F32, I32>;
defm CVTF32S64 : CvtOp<"cvtf32s64", sint_to_fp, F32, I64>;
defm CVTF32U32 : CvtOp<"cvtf32u32", uint_to_fp, F32, I32>;
defm CVTF32U64 : CvtOp<"cvtf32u64", uint_to_fp, F32, I64>;
defm CVTF64S32 : CvtOp<"cvtf64s32", sint_to_fp, F64, I32>;
defm CVTF64S64 : CvtOp<"cvtf64s64", sint_to_fp, F64, I64>;
defm CVTF64U32 : CvtOp<"cvtf64u32", uint_to_fp, F64, I32>;
defm CVTF64U64 : CvtOp<"cvtf64u64", uint_to_fp, F64, I64>;

defm CVTF32F64 : CvtOp<"cvtf32f64", fround,   F32, F64>;
defm CVTF64F32 : CvtOp<"cvtf64f32", fextend,  F64, F32>;

defm AND1  : BinOp<"and1",  and, I1,  immi1,  i1imm,  1>;
defm AND8  : BinOp<"and8",  and, I8,  immi8,  i8imm,  1>;
defm AND16 : BinOp<"and16", and, I16, immi16, i16imm, 1>;
defm AND32 : BinOp<"and32", and, I32, immi32, i32imm, 1>;
defm AND64 : BinOp<"and64", and, I64, immi64, i64imm, 1>;

defm OR1   : BinOp<"or1",   or,  I1,  immi1,  i1imm,  1>;
defm OR8   : BinOp<"or8",   or,  I8,  immi8,  i8imm,  1>;
defm OR16  : BinOp<"or16",  or,  I16, immi16, i16imm, 1>;
defm OR32  : BinOp<"or32",  or,  I32, immi32, i32imm, 1>;
defm OR64  : BinOp<"or64",  or,  I64, immi64, i64imm, 1>;

defm XOR1  : BinOp<"xor1",  xor, I1,  immi1,  i1imm,  1>;
defm XOR8  : BinOp<"xor8",  xor, I8,  immi8,  i8imm,  1>;
defm XOR16 : BinOp<"xor16", xor, I16, immi16, i16imm, 1>;
defm XOR32 : BinOp<"xor32", xor, I32, immi32, i32imm, 1>;
defm XOR64 : BinOp<"xor64", xor, I64, immi64, i64imm, 1>;

defm SEXT8     : SExtOp<"sext8",  I8,  I8>;
defm SEXT16_8  : SExtOp<"sext16", I16, I8>;
defm SEXT16    : SExtOp<"sext16", I16, I16>;
defm SEXT32_8  : SExtOp<"sext32", I32, I8>;
defm SEXT32_16 : SExtOp<"sext32", I32, I16>;
defm SEXT32    : SExtOp<"sext32", I32, I32>;
defm SEXT64_8  : SExtOp<"sext64", I64, I8>;
defm SEXT64_16 : SExtOp<"sext64", I64, I16>;
defm SEXT64_32 : SExtOp<"sext64", I64, I32>;
defm SEXT64    : SExtOp<"sext64", I64, I64>;

defm SLL8  : BinOp<"sll8",  shl, I8,  immi8,  i8imm,  0>;
defm SLL16 : BinOp<"sll16", shl, I16, immi16, i16imm, 0>;
defm SLL32 : BinOp<"sll32", shl, I32, immi32, i32imm, 0>;
defm SLL64 : BinOp<"sll64", shl, I64, immi64, i64imm, 0>;

defm SRL8  : BinOp<"srl8",  srl, I8,  immi8,  i8imm,  0>;
defm SRL16 : BinOp<"srl16", srl, I16, immi16, i16imm, 0>;
defm SRL32 : BinOp<"srl32", srl, I32, immi32, i32imm, 0>;
defm SRL64 : BinOp<"srl64", srl, I64, immi64, i64imm, 0>;

defm SRA8  : BinOp<"sra8",  sra, I8,  immi8,  i8imm,  0>;
defm SRA16 : BinOp<"sra16", sra, I16, immi16, i16imm, 0>;
defm SRA32 : BinOp<"sra32", sra, I32, immi32, i32imm, 0>;
defm SRA64 : BinOp<"sra64", sra, I64, immi64, i64imm, 0>;

defm ADD8  : BinOp<"add8",  add, I8,  immi8,  i8imm,  1>;
defm ADD16 : BinOp<"add16", add, I16, immi16, i16imm, 1>;
defm ADD32 : BinOp<"add32", add, I32, immi32, i32imm, 1>;
defm ADD64 : BinOp<"add64", add, I64, immi64, i64imm, 1>;

defm ADDF32 : BinOp<"add32f", fadd, F32, immf32, f32imm, 1>;
defm ADDF64 : BinOp<"addf64", fadd, F64, immf64, f64imm, 1>;

defm SUB8  : BinOp<"sub8",  sub, I8,  immi8,  i8imm,  0>;
defm SUB16 : BinOp<"sub16", sub, I16, immi16, i16imm, 0>;
defm SUB32 : BinOp<"sub32", sub, I32, immi32, i32imm, 0>;
defm SUB64 : BinOp<"sub64", sub, I64, immi64, i64imm, 0>;

defm SUBF32 : BinOp<"sub32f", fsub, F32, immf32, f32imm, 0>;
defm SUBF64 : BinOp<"subf64", fsub, F64, immf64, f64imm, 0>;

defm MUL8  : BinOp<"mul8",  mul, I8,  immi8,  i8imm,  1>;
defm MUL16 : BinOp<"mul16", mul, I16, immi16, i16imm, 1>;
defm MUL32 : BinOp<"mul32", mul, I32, immi32, i32imm, 1>;
defm MUL64 : BinOp<"mul64", mul, I64, immi64, i64imm, 1>;

defm MULF32 : BinOp<"mul32f", fmul, F32, immf32, f32imm, 1>;
defm MULF64 : BinOp<"mulf64", fmul, F64, immf64, f64imm, 1>;

defm DIVS8  : BinOp<"divs8",  sdiv, I8,  immi8,  i8imm,  0>;
defm DIVS16 : BinOp<"divs16", sdiv, I16, immi16, i16imm, 0>;
defm DIVS32 : BinOp<"divs32", sdiv, I32, immi32, i32imm, 0>;
defm DIVS64 : BinOp<"divs64", sdiv, I64, immi64, i64imm, 0>;

defm DIVU8  : BinOp<"divu8",  udiv, I8,  immi8,  i8imm,  0>;
defm DIVU16 : BinOp<"divu16", udiv, I16, immi16, i16imm, 0>;
defm DIVU32 : BinOp<"divu32", udiv, I32, immi32, i32imm, 0>;
defm DIVU64 : BinOp<"divu64", udiv, I64, immi64, i64imm, 0>;

defm DIVF32 : BinOp<"div32f", fdiv, F32, immf32, f32imm, 0>;
defm DIVF64 : BinOp<"divf64", fdiv, F64, immf64, f64imm, 0>;

defm CMPLTS8  : CmpOp<"cmplts8",  setlt,  I8,  immi8,  i8imm,  0>;
defm CMPLTS16 : CmpOp<"cmplts16", setlt,  I16, immi16, i16imm, 0>;
defm CMPLTS32 : CmpOp<"cmplts32", setlt,  I32, immi32, i32imm, 0>;
defm CMPLTS64 : CmpOp<"cmplts64", setlt,  I64, immi64, i64imm, 0>;

defm CMPLTU8  : CmpOp<"cmpltu8",  setult, I8,  immi8,  i8imm,  0>;
defm CMPLTU16 : CmpOp<"cmpltu16", setult, I16, immi16, i16imm, 0>;
defm CMPLTU32 : CmpOp<"cmpltu32", setult, I32, immi32, i32imm, 0>;
defm CMPLTU64 : CmpOp<"cmpltu64", setult, I64, immi64, i64imm, 0>;

defm CMPLES8  : CmpOp<"cmples8",  setle,  I8,  immi8,  i8imm,  0>;
defm CMPLES16 : CmpOp<"cmples16", setle,  I16, immi16, i16imm, 0>;
defm CMPLES32 : CmpOp<"cmples32", setle,  I32, immi32, i32imm, 0>;
defm CMPLES64 : CmpOp<"cmples64", setle,  I64, immi64, i64imm, 0>;

defm CMPLEU8  : CmpOp<"cmpleu8",  setule, I8,  immi8,  i8imm,  0>;
defm CMPLEU16 : CmpOp<"cmpleu16", setule, I16, immi16, i16imm, 0>;
defm CMPLEU32 : CmpOp<"cmpleu32", setule, I32, immi32, i32imm, 0>;
defm CMPLEU64 : CmpOp<"cmpleu64", setule, I64, immi64, i64imm, 0>;

defm CMPEQ8   : CmpOp<"cmpeq8",   seteq,  I8,  immi8,  i8imm,  1>;
defm CMPEQ16  : CmpOp<"cmpeq16",  seteq,  I16, immi16, i16imm, 1>;
defm CMPEQ32  : CmpOp<"cmpeq32",  seteq,  I32, immi32, i32imm, 1>;
defm CMPEQ64  : CmpOp<"cmpeq64",  seteq,  I64, immi64, i64imm, 1>;

defm CMPGTS8  : CmpOp<"cmpgts8",  setgt,  I8,  immi8,  i8imm,  0>;
defm CMPGTS16 : CmpOp<"cmpgts16", setgt,  I16, immi16, i16imm, 0>;
defm CMPGTS32 : CmpOp<"cmpgts32", setgt,  I32, immi32, i32imm, 0>;
defm CMPGTS64 : CmpOp<"cmpgts64", setgt,  I64, immi64, i64imm, 0>;

defm CMPGTU8  : CmpOp<"cmpgtu8",  setugt, I8,  immi8,  i8imm,  0>;
defm CMPGTU16 : CmpOp<"cmpgtu16", setugt, I16, immi16, i16imm, 0>;
defm CMPGTU32 : CmpOp<"cmpgtu32", setugt, I32, immi32, i32imm, 0>;
defm CMPGTU64 : CmpOp<"cmpgtu64", setugt, I64, immi64, i64imm, 0>;

defm CMPGES8  : CmpOp<"cmpges8",  setge,  I8,  immi8,  i8imm,  0>;
defm CMPGES16 : CmpOp<"cmpges16", setge,  I16, immi16, i16imm, 0>;
defm CMPGES32 : CmpOp<"cmpges32", setge,  I32, immi32, i32imm, 0>;
defm CMPGES64 : CmpOp<"cmpges64", setge,  I64, immi64, i64imm, 0>;

defm CMPGEU8  : CmpOp<"cmpgeu8",  setuge, I8,  immi8,  i8imm,  0>;
defm CMPGEU16 : CmpOp<"cmpgeu16", setuge, I16, immi16, i16imm, 0>;
defm CMPGEU32 : CmpOp<"cmpgeu32", setuge, I32, immi32, i32imm, 0>;
defm CMPGEU64 : CmpOp<"cmpgeu64", setuge, I64, immi64, i64imm, 0>;

defm CMPNE8   : CmpOp<"cmpne8",   setne,  I8,  immi8,  i8imm,  1>;
defm CMPNE16  : CmpOp<"cmpne16",  setne,  I16, immi16, i16imm, 1>;
defm CMPNE32  : CmpOp<"cmpne32",  setne,  I32, immi32, i32imm, 1>;
defm CMPNE64  : CmpOp<"cmpne64",  setne,  I64, immi64, i64imm, 1>;

defm CMPLTF32 : FCmpOp<"cmpltf32", setolt, setult, F32, 0>;
defm CMPLTF64 : FCmpOp<"cmpltf64", setolt, setult, F64, 0>;

defm CMPLEF32 : FCmpOp<"cmplef32", setole, setule, F32, 0>;
defm CMPLEF64 : FCmpOp<"cmplef64", setole, setule, F64, 0>;

defm CMPEQF32 : FCmpOp<"cmpeqf32", setoeq, setueq, F32, 1>;
defm CMPEQF64 : FCmpOp<"cmpeqf64", setoeq, setueq, F64, 1>;

defm CMPGTF32 : FCmpOp<"cmpgtf32", setogt, setugt, F32, 0>;
defm CMPGTF64 : FCmpOp<"cmpgtf64", setogt, setugt, F64, 0>;

defm CMPGEF32 : FCmpOp<"cmpgef32", setoge, setuge, F32, 0>;
defm CMPGEF64 : FCmpOp<"cmpgef64", setoge, setuge, F64, 0>;

defm CMPNEF32 : FCmpOp<"cmpnef32", setone, setune, F32, 1>;
defm CMPNEF64 : FCmpOp<"cmpnef64", setone, setune, F64, 1>;

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


defm FMAF32  : FusedOp1<"fmaf32", fmul, fadd, F32>;
defm FMAF32x : FusedOp2<"fmaf32", fmul, fadd, F32>; // crude handling of comm.
defm FMSF32  : FusedOp1<"fmsf32", fmul, fsub, F32>;
defm FMRSF32 : FusedOp2<"fmrsf32", fmul, fsub, F32>;

defm FMAF64  : FusedOp1<"fmaf64", fmul, fadd, F64>;
defm FMAF64x : FusedOp2<"fmaf64", fmul, fadd, F64>; // crude handling of comm
defm FMSF64  : FusedOp1<"fmsf64", fmul, fsub, F64>;
defm FMRSF64 : FusedOp2<"fmrsf64", fmul, fsub, F64>;

// Should we do these, or drop and assume sext-at-consumer?
// todo: LDS8
// todo: LDS16
// todo: LDS32
defm LD8   : LdOp<"ld8",  I8>;
defm LD16  : LdOp<"ld16", I16>;
defm LD32  : LdOp<"ld32", I32>;
defm LD32f : LdOp<"ld32", F32>;
defm LD64  : LdOp<"ld64", I64>;
defm LD64f : LdOp<"ld64", F64>;

defm ST8   : StOp<"st8",  I8,  immi8,  i8imm>;
defm ST16  : StOp<"st16", I16, immi16, i16imm>;
defm ST32  : StOp<"st32", I32, immi32, i32imm>;
defm ST32f : StOp<"st32", F32, immf32, f32imm>;
defm ST64  : StOp<"st64", I64, immi64, i64imm>;
defm ST64f : StOp<"st64", F64, immf64, f64imm>;

multiclass CopyOp<string opStr, RegisterClass RC> {
  def "" : FMTGEN<
    (outs RC:$op0, RC:$op1, RC:$op2, RC:$op3),
    (ins RC:$op4),
    !strconcat(opStr, "\t$op0, $op1, $op2, $op3, $op4"),
    []>;
}
defm COPY0  : CopyOp<"copy0",  I1>;
defm COPY1  : CopyOp<"copy1",  I1>;
defm COPY8  : CopyOp<"copy8",  I8>;
defm COPY16 : CopyOp<"copy16", I16>;
defm COPY32 : CopyOp<"copy32", I32>;
defm COPY64 : CopyOp<"copy64", I64>;

multiclass MergeOp<string opStr, RegisterClass RC> {
  def "" : FMTGEN<
    (outs RC:$dst),
    (ins I1:$sel, RC:$v0, RC:$v1),
    !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
    []>;
}
defm MERGE1  : MergeOp<"merge1",  I1>;
defm MERGE8  : MergeOp<"merge8",  I8>;
defm MERGE16 : MergeOp<"merge16", I16>;
defm MERGE32 : MergeOp<"merge32", I32>;
defm MERGE64 : MergeOp<"merge64", I64>;

multiclass SwitchOp<string opStr, RegisterClass RC> {
  def "" : FMTGEN<
    (outs RC:$dst0, RC:$dst1),
    (ins I1:$sel, RC:$v),
    !strconcat(opStr, "\t$dst0, $dst1, $sel, $v"),
    []>;
}
defm SWITCH1  : SwitchOp<"switch1",  I1>;
defm SWITCH8  : SwitchOp<"switch8",  I8>;
defm SWITCH16 : SwitchOp<"switch16", I16>;
defm SWITCH32 : SwitchOp<"switch32", I32>;
defm SWITCH64 : SwitchOp<"switch64", I64>;

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
defm PICK1  : PickOp<"pick1",  I1>;
defm PICK8  : PickOp<"pick8",  I8>;
defm PICK16 : PickOp<"pick16", I16>;
defm PICK32 : PickOp<"pick32", I32>;
defm PICK64 : PickOp<"pick64", I64>;

multiclass PickAnyOp<string opStr, RegisterClass RC> {
  def "" : FMTGEN<
    (outs RC:$dst, I1:$sel),
    (ins RC:$v0, RC:$v1),
    !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
    []>;
}
defm PICKANY1  : PickAnyOp<"pickany1",  I1>;
defm PICKANY8  : PickAnyOp<"pickany8",  I8>;
defm PICKANY16 : PickAnyOp<"pickany16", I16>;
defm PICKANY32 : PickAnyOp<"pickany32", I32>;
defm PICKANY64 : PickAnyOp<"pickany64", I64>;

def ANY0 : FMTGEN<
  (outs I0:$dst),
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
def: Pat<(i32 imm:$imm), (MOV32 imm:$imm)>;
def: Pat<(f32 immf32:$imm), (MOV32f immf32:$imm)>;
def: Pat<(i64 imm:$imm), (MOV64 imm:$imm)>;
def: Pat<(f64 immf64:$imm), (MOV64f immf64:$imm)>;

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
  (COPY_TO_REGCLASS $op1, LIC)>;
//  (AND64 $op1, 0xFFFFFFFF)>;  // gets "Type inference contradiction found, merging 'i32' into 'i64'
def : Pat<(i64 (zext i16:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;
//  (AND64 $op1, 0xFFFF)>;
def : Pat<(i64 (zext i8:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;
//  (AND64 $op1, 0xFF)>;
def : Pat<(i64 (zext i1:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;
//  (AND64 $op1, 1)>;

def : Pat<(i32 (zext i16:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;
//  (AND32 $op1, 0xFFFF)>;
def : Pat<(i32 (zext i8:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;
//  (AND32 $op1, 0xFF)>;
def : Pat<(i32 (zext i1:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;
//  (AND32 $op1, 1)>;

def : Pat<(i16 (zext i8:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;
//  (AND16 $op1, 0xFF)>;
def : Pat<(i16 (zext i1:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;
//  (AND16 $op1, 1)>;

def : Pat<(i8 (zext i1:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;
//  (AND8 $op1, 1)>;

// anyext are treated as zext
def : Pat<(i64 (anyext i32:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;
def : Pat<(i64 (anyext i16:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;
def : Pat<(i64 (anyext i8:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;
def : Pat<(i64 (anyext i1:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;

def : Pat<(i32 (anyext i16:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;
def : Pat<(i32 (anyext i8:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;
def : Pat<(i32 (anyext i1:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;

def : Pat<(i16 (anyext i8:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;
def : Pat<(i16 (anyext i1:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;

def : Pat<(i8 (anyext i1:$op1)),
  (COPY_TO_REGCLASS $op1, LIC)>;

// Eventually
//include "LPUIntrinsics.td"
