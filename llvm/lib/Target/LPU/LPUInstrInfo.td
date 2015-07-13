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

// Latency insensitive channels
/*
class LIC<ValueType t> : Operand<t> {
  let PrintMethod = "printLIC";
  let OperandType = "OPERAND_LIC";
}

// any way?  def LICI0 : LIC<?>;
def LICI1  : LIC< i1>;
def LICI8  : LIC< i8>;
def LICI16 : LIC<i16>;
def LICI32 : LIC<i32>;
def LICI64 : LIC<i64>;

def LICF32 : LIC<f32>;
def LICF64 : LIC<f64>;
*/

// Operand sets - R - Register, C - Channel, L - Literal
// These match simulator
class OpndC<ValueType t> : Operand<t>;
class OpndR<ValueType t> : Operand<t>;
class OpndL<ValueType t> : Operand<t>;
class OpndCR<ValueType t> : Operand<t>;
class OpndCL<ValueType t> : Operand<t>;
class OpndRL<ValueType t> : Operand<t>;
class OpndCRL<ValueType t> : Operand<t>;

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
    "mov0\tc1,c0",      // very temporary until control flow handled
    [(LPURet)]>;

multiclass MovOp<string opStr, ValueType T> {
  def "" : FMTGEN<
    (outs OpndCR<T>:$dst),
    (ins OpndCRL<T>:$op1),
    !strconcat(opStr, "\t$dst, $op1"),
    []>;
}

// Binary op, no pattern, not commutable
multiclass SExtOp<string opStr, ValueType T, ValueType TSRC> {
  def "" : FMTGEN<
    (outs OpndCR<T>:$dst),
    (ins OpndCRL<TSRC>:$op1, OpndCRL<T>:$op2),
    !strconcat(opStr, "\t$dst, $op1, $op2"),
    []>;
}

multiclass CvtOp<string opStr, SDNode opNode, ValueType DT, ValueType ST> {
  def "" : FMTGEN<
    (outs OpndCR<DT>:$dst),
    (ins OpndCRL<ST>:$src),
    !strconcat(opStr, "\t$dst, $src"),
    [(set DT:$dst, (opNode ST:$src))]>;
}

// This uses a multiclass even though it only declares one class because
// class doesn't allow the Opndxx in the ins/outs list.  (Why?)
// (Also, nested multiclass would be nice - it would allow declaring 4 defs
// per operator, rather than enumerating 8/16/32/64, etc.)
multiclass BinOp<string opStr, SDNode opNode, ValueType T,
           PatFrag immOpnd, bit commutable> {
  let isCommutable = commutable in {
    def "" : FMTGEN<
      (outs OpndCR<T>:$dst),
      (ins OpndCRL<T>:$op1, OpndCRL<T>:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set T:$dst, (opNode T:$op1, T:$op2))]>;
    def i : FMTGEN<
      (outs OpndCR<T>:$dst),
      (ins OpndCRL<T>:$op1, OpndCRL<T>:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set T:$dst, (opNode T:$op1, immOpnd:$op2))]>;
  }
}

multiclass FusedOp1<string opStr, SDNode inner, SDNode outer, ValueType T> {
  def "" : FMTGEN<
    (outs OpndCR<T>:$dst),
    (ins OpndCRL<T>:$op1, OpndCRL<T>:$op2, OpndCRL<T>:$op3),
    !strconcat(opStr, "\t$dst, $op1, $op2, $op3"),
    [(set T:$dst, (outer (inner T:$op1, T:$op2), T:$op3))]>;
}

multiclass FusedOp2<string opStr, SDNode inner, SDNode outer, ValueType T> {
  def "" : FMTGEN<
    (outs OpndCR<T>:$dst),
    (ins OpndCRL<T>:$op1, OpndCRL<T>:$op2, OpndCRL<T>:$op3),
    !strconcat(opStr, "\t$dst, $op2, $op3, $op1"),
    [(set T:$dst, (outer T:$op1, (inner T:$op2, T:$op3)))]>;
}

multiclass CmpOp<string opStr, PatFrag opNode, ValueType T, bit commutable> {
  let isCommutable = commutable in {
    def "" : FMTGEN<
      (outs OpndCR<i1>:$dst),
      (ins OpndCRL<T>:$op1, OpndCRL<T>:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set i1:$dst, (opNode T:$op1, T:$op2))]>;
  }
}

multiclass FCmpOp<string opStr, PatFrag opNodeOrd, PatFrag opNodeUnord,
           ValueType T, bit commutable> {
  let isCommutable = commutable in {
    def "" : FMTGEN<
      (outs OpndCR<i1>:$dst),
      (ins OpndCRL<T>:$op1, OpndCRL<T>:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set i1:$dst, (opNodeOrd T:$op1, T:$op2))]>;
    def u : FMTGEN<
      (outs OpndCR<i1>:$dst),
      (ins OpndCRL<T>:$op1, OpndCRL<T>:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set i1:$dst, (opNodeUnord T:$op1, T:$op2))]>;
  }
}

// This provides a full set of result register types.  Some are nonsensical
// (e.g. LD8_F64 or LD32_I64)
multiclass LdOp<string opStr, ValueType T> {
  def "" : FMTGEN<
    (outs OpndCR<T>:$dst),
    (ins OpndCRL<i64>:$addr),
    !strconcat(opStr, "\t$dst, $addr"),
    [(set T:$dst, (load ADDR_R:$addr))]>;
}

// TODO: How to handle store output flag?
multiclass StOp<string opStr, ValueType T, PatFrag immOpnd> {
  def "" : FMTGEN<
    (outs),  // How to handle output flag??
    (ins OpndCRL<i64>:$addr, OpndCRL<T>:$data),
    !strconcat(opStr, "\t$addr, $data"),
    [(store T:$data, ADDR_R:$addr)]>;
  def i : FMTGEN<
    (outs),  // How to handle output flag??
    (ins OpndCRL<i64>:$addr, OpndCRL<T>:$data),
    !strconcat(opStr, "\t$addr, $data"),
    [(store immOpnd:$data, ADDR_R:$addr)]>;
}

// The repetition below could be removed if nested multiclass were supported.
// (e.g. the first level would expand to 8/16/32/64, then the 2nd level
// would be as today.)

defm MOV1  : MovOp<"mov1",  i1>;
defm MOV8  : MovOp<"mov8",  i8>;
defm MOV16 : MovOp<"mov16", i16>;
defm MOV32 : MovOp<"mov32", i32>;
defm MOV32f: MovOp<"mov32", f32>;
defm MOV64 : MovOp<"mov64", i64>;
defm MOV64f: MovOp<"mov64", f64>;

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
defm CVTS32F32 : CvtOp<"cvts32f32", fp_to_sint, i32, f32>;
defm CVTS32F64 : CvtOp<"cvts32f64", fp_to_sint, i32, f64>;
defm CVTU32F32 : CvtOp<"cvtu32f32", fp_to_uint, i32, f32>;
defm CVTU32F64 : CvtOp<"cvtu32f64", fp_to_uint, i32, f64>;
defm CVTS64F32 : CvtOp<"cvts64f32", fp_to_sint, i64, f32>;
defm CVTS64F64 : CvtOp<"cvts64f64", fp_to_sint, i64, f64>;
defm CVTU64F32 : CvtOp<"cvtu64f32", fp_to_uint, i64, f32>;
defm CVTU64F64 : CvtOp<"cvtu64f64", fp_to_uint, i64, f64>;

defm CVTF32S32 : CvtOp<"cvtf32s32", sint_to_fp, f32, i32>;
defm CVTF32S64 : CvtOp<"cvtf32s64", sint_to_fp, f32, i64>;
defm CVTF32U32 : CvtOp<"cvtf32u32", uint_to_fp, f32, i32>;
defm CVTF32U64 : CvtOp<"cvtf32u64", uint_to_fp, f32, i64>;
defm CVTF64S32 : CvtOp<"cvtf64s32", sint_to_fp, f64, i32>;
defm CVTF64S64 : CvtOp<"cvtf64s64", sint_to_fp, f64, i64>;
defm CVTF64U32 : CvtOp<"cvtf64u32", uint_to_fp, f64, i32>;
defm CVTF64U64 : CvtOp<"cvtf64u64", uint_to_fp, f64, i64>;

defm CVTF32F64 : CvtOp<"cvtf32f64", fround,   f32, f64>;
defm CVTF64F32 : CvtOp<"cvtf64f32", fextend,  f64, f32>;

defm AND1  : BinOp<"and1",  and, i1,  immi1, 1>;
defm AND8  : BinOp<"and8",  and, i8,  immi8, 1>;
defm AND16 : BinOp<"and16", and, i16, immi16, 1>;
defm AND32 : BinOp<"and32", and, i32, immi32, 1>;
defm AND64 : BinOp<"and64", and, i64, immi64, 1>;

defm OR1   : BinOp<"or1",   or,  i1,  immi1, 1>;
defm OR8   : BinOp<"or8",   or,  i8,  immi8, 1>;
defm OR16  : BinOp<"or16",  or,  i16, immi16, 1>;
defm OR32  : BinOp<"or32",  or,  i32, immi32, 1>;
defm OR64  : BinOp<"or64",  or,  i64, immi64, 1>;

defm XOR1  : BinOp<"xor1",  xor, i1,  immi1, 1>;
defm XOR8  : BinOp<"xor8",  xor, i8,  immi8, 1>;
defm XOR16 : BinOp<"xor16", xor, i16, immi16, 1>;
defm XOR32 : BinOp<"xor32", xor, i32, immi32, 1>;
defm XOR64 : BinOp<"xor64", xor, i64, immi64, 1>;

defm SEXT8     : SExtOp<"sext8",  i8,  i8>;
defm SEXT16_8  : SExtOp<"sext16", i16, i8>;
defm SEXT16    : SExtOp<"sext16", i16, i16>;
defm SEXT32_8  : SExtOp<"sext32", i32, i8>;
defm SEXT32_16 : SExtOp<"sext32", i32, i16>;
defm SEXT32    : SExtOp<"sext32", i32, i32>;
defm SEXT64_8  : SExtOp<"sext64", i64, i8>;
defm SEXT64_16 : SExtOp<"sext64", i64, i16>;
defm SEXT64_32 : SExtOp<"sext64", i64, i32>;
defm SEXT64    : SExtOp<"sext64", i64, i64>;

defm SLL8  : BinOp<"sll8",  shl, i8,  immi8, 0>;
defm SLL16 : BinOp<"sll16", shl, i16, immi16, 0>;
defm SLL32 : BinOp<"sll32", shl, i32, immi32, 0>;
defm SLL64 : BinOp<"sll64", shl, i64, immi64, 0>;

defm SRL8  : BinOp<"srl8",  srl, i8,  immi8, 0>;
defm SRL16 : BinOp<"srl16", srl, i16, immi16, 0>;
defm SRL32 : BinOp<"srl32", srl, i32, immi32, 0>;
defm SRL64 : BinOp<"srl64", srl, i64, immi64, 0>;

defm SRA8  : BinOp<"sra8",  sra, i8,  immi8, 0>;
defm SRA16 : BinOp<"sra16", sra, i16, immi16, 0>;
defm SRA32 : BinOp<"sra32", sra, i32, immi32, 0>;
defm SRA64 : BinOp<"sra64", sra, i64, immi64, 0>;

defm ADD8  : BinOp<"add8",  add, i8,  immi8, 1>;
defm ADD16 : BinOp<"add16", add, i16, immi16, 1>;
defm ADD32 : BinOp<"add32", add, i32, immi32, 1>;
defm ADD64 : BinOp<"add64", add, i64, immi64, 1>;

defm ADDF32 : BinOp<"add32f", fadd, f32, immf32, 1>;
defm ADDF64 : BinOp<"addf64", fadd, f64, immf64, 1>;

defm SUB8  : BinOp<"sub8",  sub, i8,  immi8, 0>;
defm SUB16 : BinOp<"sub16", sub, i16, immi16, 0>;
defm SUB32 : BinOp<"sub32", sub, i32, immi32, 0>;
defm SUB64 : BinOp<"sub64", sub, i64, immi64, 0>;

defm SUBF32 : BinOp<"sub32f", fsub, f32, immf32, 1>;
defm SUBF64 : BinOp<"subf64", fsub, f64, immf64, 1>;

defm MUL8  : BinOp<"mul8",  mul, i8,  immi8, 1>;
defm MUL16 : BinOp<"mul16", mul, i16, immi16, 1>;
defm MUL32 : BinOp<"mul32", mul, i32, immi32, 1>;
defm MUL64 : BinOp<"mul64", mul, i64, immi64, 1>;

defm MULF32 : BinOp<"mul32f", fmul, f32, immf32, 1>;
defm MULF64 : BinOp<"mulf64", fmul, f64, immf64, 1>;

defm DIVS8  : BinOp<"divs8",  sdiv, i8,  immi8, 0>;
defm DIVS16 : BinOp<"divs16", sdiv, i16, immi16, 0>;
defm DIVS32 : BinOp<"divs32", sdiv, i32, immi32, 0>;
defm DIVS64 : BinOp<"divs64", sdiv, i64, immi64, 0>;

defm DIVU8  : BinOp<"divu8",  udiv, i8,  immi8, 0>;
defm DIVU16 : BinOp<"divu16", udiv, i16, immi16, 0>;
defm DIVU32 : BinOp<"divu32", udiv, i32, immi32, 0>;
defm DIVU64 : BinOp<"divu64", udiv, i64, immi64, 0>;

defm DIVF32 : BinOp<"div32f", fdiv, f32, immf32, 0>;
defm DIVF64 : BinOp<"divf64", fdiv, f64, immf64, 0>;

defm CMPLTS8  : CmpOp<"cmplts8",  setlt,  i8,  0>;
defm CMPLTS16 : CmpOp<"cmplts16", setlt,  i16, 0>;
defm CMPLTS32 : CmpOp<"cmplts32", setlt,  i32, 0>;
defm CMPLTS64 : CmpOp<"cmplts64", setlt,  i64, 0>;

defm CMPLTU8  : CmpOp<"cmpltu8",  setult, i8,  0>;
defm CMPLTU16 : CmpOp<"cmpltu16", setult, i16, 0>;
defm CMPLTU32 : CmpOp<"cmpltu32", setult, i32, 0>;
defm CMPLTU64 : CmpOp<"cmpltu64", setult, i64, 0>;

defm CMPLES8  : CmpOp<"cmples8",  setle,  i8,  0>;
defm CMPLES16 : CmpOp<"cmples16", setle,  i16, 0>;
defm CMPLES32 : CmpOp<"cmples32", setle,  i32, 0>;
defm CMPLES64 : CmpOp<"cmples64", setle,  i64, 0>;

defm CMPLEU8  : CmpOp<"cmpleu8",  setule, i8,  0>;
defm CMPLEU16 : CmpOp<"cmpleu16", setule, i16, 0>;
defm CMPLEU32 : CmpOp<"cmpleu32", setule, i32, 0>;
defm CMPLEU64 : CmpOp<"cmpleu64", setule, i64, 0>;

defm CMPEQ8   : CmpOp<"cmpeq8",   seteq,  i8,  1>;
defm CMPEQ16  : CmpOp<"cmpeq16",  seteq,  i16, 1>;
defm CMPEQ32  : CmpOp<"cmpeq32",  seteq,  i32, 1>;
defm CMPEQ64  : CmpOp<"cmpeq64",  seteq,  i64, 1>;

defm CMPGTS8  : CmpOp<"cmpgts8",  setgt,  i8,  0>;
defm CMPGTS16 : CmpOp<"cmpgts16", setgt,  i16, 0>;
defm CMPGTS32 : CmpOp<"cmpgts32", setgt,  i32, 0>;
defm CMPGTS64 : CmpOp<"cmpgts64", setgt,  i64, 0>;

defm CMPGTU8  : CmpOp<"cmpgtu8",  setugt, i8,  0>;
defm CMPGTU16 : CmpOp<"cmpgtu16", setugt, i16, 0>;
defm CMPGTU32 : CmpOp<"cmpgtu32", setugt, i32, 0>;
defm CMPGTU64 : CmpOp<"cmpgtu64", setugt, i64, 0>;

defm CMPGES8  : CmpOp<"cmpges8",  setge,  i8,  0>;
defm CMPGES16 : CmpOp<"cmpges16", setge,  i16, 0>;
defm CMPGES32 : CmpOp<"cmpges32", setge,  i32, 0>;
defm CMPGES64 : CmpOp<"cmpges64", setge,  i64, 0>;

defm CMPGEU8  : CmpOp<"cmpgeu8",  setuge, i8,  0>;
defm CMPGEU16 : CmpOp<"cmpgeu16", setuge, i16, 0>;
defm CMPGEU32 : CmpOp<"cmpgeu32", setuge, i32, 0>;
defm CMPGEU64 : CmpOp<"cmpgeu64", setuge, i64, 0>;

defm CMPNE8   : CmpOp<"cmpne8",   setne,  i8,  1>;
defm CMPNE16  : CmpOp<"cmpne16",  setne,  i16, 1>;
defm CMPNE32  : CmpOp<"cmpne32",  setne,  i32, 1>;
defm CMPNE64  : CmpOp<"cmpne64",  setne,  i64, 1>;

defm CMPLTF32 : FCmpOp<"cmpltf32", setolt, setult, f32, 0>;
defm CMPLTF64 : FCmpOp<"cmpltf64", setolt, setult, f64, 0>;

defm CMPLEF32 : FCmpOp<"cmplef32", setole, setule, f32, 0>;
defm CMPLEF64 : FCmpOp<"cmplef64", setole, setule, f64, 0>;

defm CMPEQF32 : FCmpOp<"cmpeqf32", setoeq, setueq, f32, 1>;
defm CMPEQF64 : FCmpOp<"cmpeqf64", setoeq, setueq, f64, 1>;

defm CMPGTF32 : FCmpOp<"cmpgtf32", setogt, setugt, f32, 0>;
defm CMPGTF64 : FCmpOp<"cmpgtf64", setogt, setugt, f64, 0>;

defm CMPGEF32 : FCmpOp<"cmpgef32", setoge, setuge, f32, 0>;
defm CMPGEF64 : FCmpOp<"cmpgef64", setoge, setuge, f64, 0>;

defm CMPNEF32 : FCmpOp<"cmpnef32", setone, setune, f32, 1>;
defm CMPNEF64 : FCmpOp<"cmpnef64", setone, setune, f64, 1>;

// todo: def BSEL8
// todo: def BSEL16
// todo: def BSEL32
// todo: def BSEL64
// todo: def LAND1

defm FMAF32  : FusedOp1<"fmaf32", fmul, fadd, f32>;
defm FMAF32x : FusedOp2<"fmaf32", fmul, fadd, f32>; // crude handling of comm.
defm FMSF32  : FusedOp1<"fmsf32", fmul, fsub, f32>;
defm FMRSF32 : FusedOp2<"fmrsf32", fmul, fsub, f32>;

defm FMAF64  : FusedOp1<"fmaf64", fmul, fadd, f64>;
defm FMAF64x : FusedOp2<"fmaf64", fmul, fadd, f64>; // crude handling of comm
defm FMSF64  : FusedOp1<"fmsf64", fmul, fsub, f64>;
defm FMRSF64 : FusedOp2<"fmrsf64", fmul, fsub, f64>;

// Should we do these, or drop and assume sext-at-consumer?
// todo: LDS8
// todo: LDS16
// todo: LDS32
defm LD8   : LdOp<"ld8",  i8>;
defm LD16  : LdOp<"ld16", i16>;
defm LD32  : LdOp<"ld32", i32>;
defm LD32f : LdOp<"ld32", f32>;
defm LD64  : LdOp<"ld64", i64>;
defm LD64f : LdOp<"ld64", f64>;

defm ST8   : StOp<"st8",  i8,  immi8>;
defm ST16  : StOp<"st16", i16, immi16>;
defm ST32  : StOp<"st32", i32, immi32>;
defm ST32f : StOp<"st32", f32, immf32>;
defm ST64  : StOp<"st64", i64, immi64>;
defm ST64f : StOp<"st64", f64, immf64>;


// todo: COPY0
// todo: COPY1
// todo: COPY8
// todo: COPY16
// todo: COPY32
// todo: COPY64
// todo: MERGE1
// todo: MERGE8
// todo: MERGE16
// todo: MERGE32
// todo: MERGE64
// todo: SWITCH1
// todo: SWITCH8
// todo: SWITCH16
// todo: SWITCH32
// todo: SWITCH64
// todo: SWITCHANY1
// todo: SWITCHANY8
// todo: SWITCHANY16
// todo: SWITCHANY32
// todo: SWITCHANY64
// todo: PICK1
// todo: PICK8
// todo: PICK16
// todo: PICK32
// todo: PICK64
// todo: PICKANY1
// todo: PICKANY8
// todo: PICKANY16
// todo: PICKANY32
// todo: PICKANY64
// todo: ANY0
// todo: ALL0
// todo: ONCOUNT0
// todo: SEQC64
// todo: SEQS64
// todo: REPEAT64
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
  (COPY_TO_REGCLASS $op1, CC)>;
//  (AND64 $op1, 0xFFFFFFFF)>;  // gets "Type inference contradiction found, merging 'i32' into 'i64'
def : Pat<(i64 (zext i16:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;
//  (AND64 $op1, 0xFFFF)>;
def : Pat<(i64 (zext i8:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;
//  (AND64 $op1, 0xFF)>;
def : Pat<(i64 (zext i1:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;
//  (AND64 $op1, 1)>;

def : Pat<(i32 (zext i16:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;
//  (AND32 $op1, 0xFFFF)>;
def : Pat<(i32 (zext i8:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;
//  (AND32 $op1, 0xFF)>;
def : Pat<(i32 (zext i1:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;
//  (AND32 $op1, 1)>;

def : Pat<(i16 (zext i8:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;
//  (AND16 $op1, 0xFF)>;
def : Pat<(i16 (zext i1:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;
//  (AND16 $op1, 1)>;

def : Pat<(i8 (zext i1:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;
//  (AND8 $op1, 1)>;

// anyext are treated as zext
def : Pat<(i64 (anyext i32:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;
def : Pat<(i64 (anyext i16:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;
def : Pat<(i64 (anyext i8:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;
def : Pat<(i64 (anyext i1:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;

def : Pat<(i32 (anyext i16:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;
def : Pat<(i32 (anyext i8:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;
def : Pat<(i32 (anyext i1:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;

def : Pat<(i16 (anyext i8:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;
def : Pat<(i16 (anyext i1:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;

def : Pat<(i8 (anyext i1:$op1)),
  (COPY_TO_REGCLASS $op1, CC)>;

// Eventually
//include "LPUIntrinsics.td"
