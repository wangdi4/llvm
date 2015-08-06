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
def immAny       : PatLeaf<(imm)>;
def immi1        : PatLeaf<(i1 imm)>;
def immi8        : PatLeaf<(i8 imm)>;
def immi16       : PatLeaf<(i16 imm)>;
def immi32       : PatLeaf<(i32 imm)>;
def immi64       : PatLeaf<(i64 imm)>;
def fpimmAny     : PatLeaf<(fpimm)>;
def immf32       : PatLeaf<(f32 fpimm)>;
def immf64       : PatLeaf<(f64 fpimm)>;

def brtarget     : Operand<OtherVT>;

// Memory operands
class Addr<int numArgs, string funcName, dag opInfo> :
  Operand<i64>, ComplexPattern<i64, numArgs,
    funcName, [],
    [SDNPWantParent]> { let MIOperandInfo = opInfo; }

def ADDR_R : Addr<1, "SelectAddrReg", (ops i64)>;

def flog : SDNode<"ISD::FLOG", SDTFPUnaryOp>;
def fexp : SDNode<"ISD::FEXP", SDTFPUnaryOp>;

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
    "mov0\t%c0, 0\n\t"
    "br\t.start",      // very temporary until control flow handled
    [(LPURet)]>;

multiclass MovOp<string opStr, RegisterClass RC, ValueType opType, Operand immOp> {
  def "" : FMTGEN<
    (outs RC:$dst),
    (ins RC:$op1),
    !strconcat(opStr, "\t$dst, $op1"),
    []>;
  def i : FMTGEN<
    (outs RC:$dst),
    (ins immOp:$imm),
    !strconcat(opStr, "\t$dst, $imm"),
    []>;
}

multiclass UnaryOp<string opStr, SDNode opNode, RegisterClass RC> {
  def "" : FMTGEN<
    (outs RC:$dst),
    (ins RC:$op1),
    !strconcat(opStr, "\t$dst, $op1"),
    [(set RC:$dst, (opNode RC:$op1))]>;
}

multiclass NotOp<string opStr, RegisterClass RC> {
  def "" : FMTGEN<
    (outs RC:$dst),
    (ins RC:$op1),
    !strconcat(opStr, "\t$dst, $op1"),
    [(set RC:$dst, (not RC:$op1))]>;
}

multiclass NegOp<string opStr, RegisterClass RC> {
  def "" : FMTGEN<
    (outs RC:$dst),
    (ins RC:$op1),
    !strconcat(opStr, "\t$dst, $op1"),
    [(set RC:$dst, (sub 0, RC:$op1))]>;
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
multiclass BinOpC<string opStr, SDNode opNode, RegisterClass RC,
           ValueType opType, Operand immOpnd> {
  let isCommutable = 1 in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set RC:$dst, (opNode RC:$op1, RC:$op2))]>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, immOpnd:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set RC:$dst, (opNode RC:$op1, (opType immAny:$imm)))]>;
  }
}

multiclass BinOpNC<string opStr, SDNode opNode, RegisterClass RC,
           ValueType opType, Operand immOpnd> {
  let isCommutable = 0 in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set RC:$dst, (opNode RC:$op1, RC:$op2))]>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, immOpnd:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set RC:$dst, (opNode RC:$op1, (opType immAny:$imm)))]>;
    def i1 : FMTGEN<
      (outs RC:$dst),
      (ins immOpnd:$imm, RC:$op2),
      !strconcat(opStr, "\t$dst, $imm, $op2"),
      [(set RC:$dst, (opNode (opType immAny:$imm), RC:$op2))]>;
  }
}

multiclass FBinOpC<string opStr, SDNode opNode, RegisterClass RC,
           ValueType opType, Operand immOpnd> {
  let isCommutable = 1 in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set RC:$dst, (opNode RC:$op1, RC:$op2))]>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, immOpnd:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set RC:$dst, (opNode RC:$op1, (opType fpimmAny:$imm)))]>;
  }
}

multiclass FBinOpNC<string opStr, SDNode opNode, RegisterClass RC,
           ValueType opType, Operand immOpnd> {
  let isCommutable = 0 in {
    def "" : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set RC:$dst, (opNode RC:$op1, RC:$op2))]>;
    def i : FMTGEN<
      (outs RC:$dst),
      (ins RC:$op1, immOpnd:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set RC:$dst, (opNode RC:$op1, (opType fpimmAny:$imm)))]>;
    def i1 : FMTGEN<
      (outs RC:$dst),
      (ins immOpnd:$imm, RC:$op2),
      !strconcat(opStr, "\t$dst, $imm, $op2"),
      [(set RC:$dst, (opNode (opType fpimmAny:$imm), RC:$op2))]>;
  }
}

multiclass FusedOp1<string opStr, SDNode inner, SDNode outer, RegisterClass RC,
           ValueType opType, Operand immOpnd> {
  def "" : FMTGEN<
    (outs RC:$dst),
    (ins RC:$op1, RC:$op2, RC:$op3),
    !strconcat(opStr, "\t$dst, $op1, $op2, $op3"),
    [(set RC:$dst, (outer (inner RC:$op1, RC:$op2), RC:$op3))]>;
  def i : FMTGEN<
    (outs RC:$dst),
    (ins RC:$op1, RC:$op2, immOpnd:$imm),
    !strconcat(opStr, "\t$dst, $op1, $op2, $imm"),
    [(set RC:$dst, (outer (inner RC:$op1, RC:$op2), (opType fpimmAny:$imm)))]>;
  def ix : FMTGEN<
    (outs RC:$dst),
    (ins RC:$op1, immOpnd:$imm, RC:$op3),
    !strconcat(opStr, "\t$dst, $op1, $imm, $op3"),
    [(set RC:$dst, (outer (inner RC:$op1, (opType fpimmAny:$imm)), RC:$op3))]>;
  def ii : FMTGEN<
    (outs RC:$dst),
    (ins RC:$op1, immOpnd:$imm2, RC:$imm3),
    !strconcat(opStr, "\t$dst, $op1, $imm2, $imm3"),
    [(set RC:$dst, (outer (inner RC:$op1, (opType fpimmAny:$imm2)), (opType fpimmAny:$imm3)))]>;
}

multiclass FusedOp2<string opStr, SDNode inner, SDNode outer, RegisterClass RC,
           ValueType opType, Operand immOpnd> {
  def "" : FMTGEN<
    (outs RC:$dst),
    (ins RC:$op1, RC:$op2, RC:$op3),
    !strconcat(opStr, "\t$dst, $op2, $op3, $op1"),
    [(set RC:$dst, (outer RC:$op1, (inner RC:$op2, RC:$op3)))]>;
  def i : FMTGEN<
    (outs RC:$dst),
    (ins immOpnd:$imm, RC:$op2, RC:$op3),
    !strconcat(opStr, "\t$dst, $op2, $op3, $imm"),
    [(set RC:$dst, (outer (opType fpimmAny:$imm), (inner RC:$op2, RC:$op3)))]>;
}

multiclass CmpOpC<string opStr, PatFrag opNode, RegisterClass RC,
           ValueType opType, Operand immOpnd> {
  let isCommutable = 1 in {
    def "" : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set I1:$dst, (opNode RC:$op1, RC:$op2))]>;
    def i : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, immOpnd:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set I1:$dst, (opNode RC:$op1, (opType immAny:$imm)))]>;
  }
}

multiclass CmpOpNC<string opStr, PatFrag opNode, RegisterClass RC,
           ValueType opType, Operand immOpnd> {
  let isCommutable = 0 in {
    def "" : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, RC:$op2),
      !strconcat(opStr, "\t$dst, $op1, $op2"),
      [(set I1:$dst, (opNode RC:$op1, RC:$op2))]>;
    def i : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, immOpnd:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set I1:$dst, (opNode RC:$op1, (opType immAny:$imm)))]>;
    def ix : FMTGEN<
      (outs I1:$dst),
      (ins immOpnd:$imm, RC:$op2),
      !strconcat(opStr, "\t$dst, $imm, $op2"),
      [(set I1:$dst, (opNode (opType immAny:$imm), RC:$op2))]>;
  }
}

multiclass FCmpOp<string opStr, PatFrag opNodeOrd, PatFrag opNodeUnord,
           RegisterClass RC, ValueType opType, Operand immOpnd, bit commutable> {
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
    def i : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, immOpnd:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set i1:$dst, (opNodeOrd RC:$op1, (opType fpimmAny:$imm)))]>;
    def ui : FMTGEN<
      (outs I1:$dst),
      (ins RC:$op1, immOpnd:$imm),
      !strconcat(opStr, "\t$dst, $op1, $imm"),
      [(set i1:$dst, (opNodeUnord RC:$op1, (opType fpimmAny:$imm)))]>;
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
multiclass StOp<string opStr, RegisterClass RC, ValueType opType, Operand immOpnd> {
  def "" : FMTGEN<
    (outs),  // How to handle output flag??
    (ins I64:$addr, RC:$data),
    !strconcat(opStr, "\t$addr, $data"),
    [(store RC:$data, ADDR_R:$addr)]>;
  def i : FMTGEN<
    (outs),  // How to handle output flag??
    (ins I64:$addr, immOpnd:$data),
    !strconcat(opStr, "\t$addr, $data"),
    [(store (opType immAny:$data), ADDR_R:$addr)]>;
}

multiclass FStOp<string opStr, RegisterClass RC, ValueType opType, Operand immOpnd> {
  def "" : FMTGEN<
    (outs),  // How to handle output flag??
    (ins I64:$addr, RC:$data),
    !strconcat(opStr, "\t$addr, $data"),
    [(store RC:$data, ADDR_R:$addr)]>;
}

// The repetition below could be removed if nested multiclass were supported.
// (e.g. the first level would expand to 8/16/32/64, then the 2nd level
// would be as today.)

defm MOV1  : MovOp<"mov1",  I1,  i1,  i1imm>;
defm MOV8  : MovOp<"mov8",  I8,  i8,  i8imm>;
defm MOV16 : MovOp<"mov16", I16, i16, i16imm>;
defm MOV32 : MovOp<"mov32", I32, i32, i32imm>;
defm MOV32f: MovOp<"mov32", F32, f32, f32imm>;
defm MOV64 : MovOp<"mov64", I64, i64, i64imm>;
defm MOV64f: MovOp<"mov64", F64, f64, f64imm>;

defm NOT1      : NotOp  <"not1",             I1>;
defm NOT8      : NotOp  <"not8",             I8>;
defm NOT16     : NotOp  <"not16",            I16>;
defm NOT32     : NotOp  <"not32",            I32>;
defm NOT64     : NotOp  <"not64",            I64>;
defm NEG8      : NegOp  <"neg8",             I8>;
defm NEGS16    : NegOp  <"negs16",           I16>;
defm NEGS32    : NegOp  <"negs32",           I32>;
defm NEGS64    : NegOp  <"negs64",           I64>;
defm NEGF32    : UnaryOp<"negf32",    fneg,  F32>;
defm NEGF64    : UnaryOp<"negf64",    fneg,  F64>;
defm ABSF32    : UnaryOp<"absf32",    fabs,  F32>;
defm ABSF64    : UnaryOp<"absf64",    fabs,  F64>;
defm SQRTF32   : UnaryOp<"sqrtf32",   fsqrt, F32>;
defm SQRTF64   : UnaryOp<"sqrtf64",   fsqrt, F64>;
defm EXP2F32   : UnaryOp<"exp2f32",   fexp2, F32>;
defm EXP2F64   : UnaryOp<"exp2f64",   fexp2, F64>;
defm LOG2F32   : UnaryOp<"log2f32",   flog2, F32>;
defm LOG2F64   : UnaryOp<"log2f64",   flog2, F64>;
defm EXPF32    : UnaryOp<"expf32",    fexp,  F32>;
defm EXPF64    : UnaryOp<"expf64",    fexp,  F64>;
defm LOGF32    : UnaryOp<"logf32",    flog,  F32>;
defm LOGF64    : UnaryOp<"logf64",    flog,  F64>;
defm SINF32    : UnaryOp<"sinf32",    fsin,  F32>;
defm SINF64    : UnaryOp<"sinf64",    fsin,  F64>;
defm COSF32    : UnaryOp<"cosf32",    fcos,  F32>;
defm COSF64    : UnaryOp<"cosf64",    fcos,  F64>;
//defm SINCOSF32 : UnaryOp<"sincosf32", fsincos, F32>;
//defm SINCOSF64 : UnaryOp<"sincosf64", fsincos, F64>;

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

defm AND1  : BinOpC<"and1",  and, I1,  i1,  i1imm>;
defm AND8  : BinOpC<"and8",  and, I8,  i8,  i8imm>;
defm AND16 : BinOpC<"and16", and, I16, i16, i16imm>;
defm AND32 : BinOpC<"and32", and, I32, i32, i32imm>;
defm AND64 : BinOpC<"and64", and, I64, i64, i64imm>;

defm OR1   : BinOpC<"or1",   or,  I1,  i1,  i1imm>;
defm OR8   : BinOpC<"or8",   or,  I8,  i8,  i8imm>;
defm OR16  : BinOpC<"or16",  or,  I16, i16, i16imm>;
defm OR32  : BinOpC<"or32",  or,  I32, i32, i32imm>;
defm OR64  : BinOpC<"or64",  or,  I64, i64, i64imm>;

defm XOR1  : BinOpC<"xor1",  xor, I1,  i1,  i1imm>;
defm XOR8  : BinOpC<"xor8",  xor, I8,  i8,  i8imm>;
defm XOR16 : BinOpC<"xor16", xor, I16, i16, i16imm>;
defm XOR32 : BinOpC<"xor32", xor, I32, i32, i32imm>;
defm XOR64 : BinOpC<"xor64", xor, I64, i64, i64imm>;

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

defm SLL8  : BinOpNC<"sll8",  shl, I8,  i8,  i8imm>;
defm SLL16 : BinOpNC<"sll16", shl, I16, i16, i16imm>;
defm SLL32 : BinOpNC<"sll32", shl, I32, i32, i32imm>;
defm SLL64 : BinOpNC<"sll64", shl, I64, i64, i64imm>;

defm SRL8  : BinOpNC<"srl8",  srl, I8,  i8,  i8imm>;
defm SRL16 : BinOpNC<"srl16", srl, I16, i16, i16imm>;
defm SRL32 : BinOpNC<"srl32", srl, I32, i32, i32imm>;
defm SRL64 : BinOpNC<"srl64", srl, I64, i64, i64imm>;

defm SRA8  : BinOpNC<"sra8",  sra, I8,  i8,  i8imm>;
defm SRA16 : BinOpNC<"sra16", sra, I16, i16, i16imm>;
defm SRA32 : BinOpNC<"sra32", sra, I32, i32, i32imm>;
defm SRA64 : BinOpNC<"sra64", sra, I64, i64, i64imm>;

defm ADD8  : BinOpC<"add8",  add, I8,  i8,  i8imm>;
defm ADD16 : BinOpC<"add16", add, I16, i16, i16imm>;
defm ADD32 : BinOpC<"add32", add, I32, i32, i32imm>;
defm ADD64 : BinOpC<"add64", add, I64, i64, i64imm>;

defm ADDF32 : FBinOpC<"add32f", fadd, F32, f32, f32imm>;
defm ADDF64 : FBinOpC<"addf64", fadd, F64, f64, f64imm>;

defm SUB8  : BinOpNC<"sub8",  sub, I8,  i8,  i8imm>;
defm SUB16 : BinOpNC<"sub16", sub, I16, i16, i16imm>;
defm SUB32 : BinOpNC<"sub32", sub, I32, i32, i32imm>;
defm SUB64 : BinOpNC<"sub64", sub, I64, i64, i64imm>;

defm SUBF32 : FBinOpNC<"sub32f", fsub, F32, f32, f32imm>;
defm SUBF64 : FBinOpNC<"subf64", fsub, F64, f64, f64imm>;

defm MUL8  : BinOpC<"mul8",  mul, I8,  i8,  i8imm>;
defm MUL16 : BinOpC<"mul16", mul, I16, i16, i16imm>;
defm MUL32 : BinOpC<"mul32", mul, I32, i32, i32imm>;
defm MUL64 : BinOpC<"mul64", mul, I64, i64, i64imm>;

defm MULF32 : FBinOpC<"mul32f", fmul, F32, f32, f32imm>;
defm MULF64 : FBinOpC<"mulf64", fmul, F64, f64, f64imm>;

defm DIVS8  : BinOpNC<"divs8",  sdiv, I8,  i8,  i8imm>;
defm DIVS16 : BinOpNC<"divs16", sdiv, I16, i16, i16imm>;
defm DIVS32 : BinOpNC<"divs32", sdiv, I32, i32, i32imm>;
defm DIVS64 : BinOpNC<"divs64", sdiv, I64, i64, i64imm>;

defm DIVU8  : BinOpNC<"divu8",  udiv, I8,  i8,  i8imm>;
defm DIVU16 : BinOpNC<"divu16", udiv, I16, i16, i16imm>;
defm DIVU32 : BinOpNC<"divu32", udiv, I32, i32, i32imm>;
defm DIVU64 : BinOpNC<"divu64", udiv, I64, i64, i64imm>;

defm DIVF32 : FBinOpNC<"div32f", fdiv, F32, f32, f32imm>;
defm DIVF64 : FBinOpNC<"divf64", fdiv, F64, f64, f64imm>;

defm CMPLTS8  : CmpOpNC<"cmplts8",  setlt,  I8,  i8,  i8imm>;
defm CMPLTS16 : CmpOpNC<"cmplts16", setlt,  I16, i16, i16imm>;
defm CMPLTS32 : CmpOpNC<"cmplts32", setlt,  I32, i32, i32imm>;
defm CMPLTS64 : CmpOpNC<"cmplts64", setlt,  I64, i64, i64imm>;

defm CMPLTU8  : CmpOpNC<"cmpltu8",  setult, I8,  i8,  i8imm>;
defm CMPLTU16 : CmpOpNC<"cmpltu16", setult, I16, i16, i16imm>;
defm CMPLTU32 : CmpOpNC<"cmpltu32", setult, I32, i32, i32imm>;
defm CMPLTU64 : CmpOpNC<"cmpltu64", setult, I64, i64, i64imm>;

defm CMPLES8  : CmpOpNC<"cmples8",  setle,  I8,  i8,  i8imm>;
defm CMPLES16 : CmpOpNC<"cmples16", setle,  I16, i16, i16imm>;
defm CMPLES32 : CmpOpNC<"cmples32", setle,  I32, i32, i32imm>;
defm CMPLES64 : CmpOpNC<"cmples64", setle,  I64, i64, i64imm>;

defm CMPLEU8  : CmpOpNC<"cmpleu8",  setule, I8,  i8,  i8imm>;
defm CMPLEU16 : CmpOpNC<"cmpleu16", setule, I16, i16, i16imm>;
defm CMPLEU32 : CmpOpNC<"cmpleu32", setule, I32, i32, i32imm>;
defm CMPLEU64 : CmpOpNC<"cmpleu64", setule, I64, i64, i64imm>;

// Note - there is no cmpeq1.  As long as operands are clean, cmpeq8
// suffices
defm CMPEQ1   : CmpOpC<"cmpeq8",   seteq,  I1,  i1,  i1imm>;
defm CMPEQ8   : CmpOpC<"cmpeq8",   seteq,  I8,  i8,  i8imm>;
defm CMPEQ16  : CmpOpC<"cmpeq16",  seteq,  I16, i16, i16imm>;
defm CMPEQ32  : CmpOpC<"cmpeq32",  seteq,  I32, i32, i32imm>;
defm CMPEQ64  : CmpOpC<"cmpeq64",  seteq,  I64, i64, i64imm>;

defm CMPGTS8  : CmpOpNC<"cmpgts8",  setgt,  I8,  i8,  i8imm>;
defm CMPGTS16 : CmpOpNC<"cmpgts16", setgt,  I16, i16, i16imm>;
defm CMPGTS32 : CmpOpNC<"cmpgts32", setgt,  I32, i32, i32imm>;
defm CMPGTS64 : CmpOpNC<"cmpgts64", setgt,  I64, i64, i64imm>;

defm CMPGTU8  : CmpOpNC<"cmpgtu8",  setugt, I8,  i8,  i8imm>;
defm CMPGTU16 : CmpOpNC<"cmpgtu16", setugt, I16, i16, i16imm>;
defm CMPGTU32 : CmpOpNC<"cmpgtu32", setugt, I32, i32, i32imm>;
defm CMPGTU64 : CmpOpNC<"cmpgtu64", setugt, I64, i64, i64imm>;

defm CMPGES8  : CmpOpNC<"cmpges8",  setge,  I8,  i8,  i8imm>;
defm CMPGES16 : CmpOpNC<"cmpges16", setge,  I16, i16, i16imm>;
defm CMPGES32 : CmpOpNC<"cmpges32", setge,  I32, i32, i32imm>;
defm CMPGES64 : CmpOpNC<"cmpges64", setge,  I64, i64, i64imm>;

defm CMPGEU8  : CmpOpNC<"cmpgeu8",  setuge, I8,  i8,  i8imm>;
defm CMPGEU16 : CmpOpNC<"cmpgeu16", setuge, I16, i16, i16imm>;
defm CMPGEU32 : CmpOpNC<"cmpgeu32", setuge, I32, i32, i32imm>;
defm CMPGEU64 : CmpOpNC<"cmpgeu64", setuge, I64, i64, i64imm>;

def : Pat<(i1 (setne i1:$op1, i1:$op2)), (XOR1 $op1,$op2)>;
def : Pat<(i1 (setne i1:$op1, (i1 immAny:$imm))), (XOR1i $op1,$imm)>;
defm CMPNE8   : CmpOpC<"cmpne8",   setne,  I8,  i8,  i8imm>;
defm CMPNE16  : CmpOpC<"cmpne16",  setne,  I16, i16, i16imm>;
defm CMPNE32  : CmpOpC<"cmpne32",  setne,  I32, i32, i32imm>;
defm CMPNE64  : CmpOpC<"cmpne64",  setne,  I64, i64, i64imm>;

defm CMPLTF32 : FCmpOp<"cmpltf32", setolt, setult, F32, f32, f32imm, 0>;
defm CMPLTF64 : FCmpOp<"cmpltf64", setolt, setult, F64, f64, f64imm, 0>;

defm CMPLEF32 : FCmpOp<"cmplef32", setole, setule, F32, f32, f32imm, 0>;
defm CMPLEF64 : FCmpOp<"cmplef64", setole, setule, F64, f64, f64imm, 0>;

defm CMPEQF32 : FCmpOp<"cmpeqf32", setoeq, setueq, F32, f32, f32imm, 1>;
defm CMPEQF64 : FCmpOp<"cmpeqf64", setoeq, setueq, F64, f64, f64imm, 1>;

defm CMPGTF32 : FCmpOp<"cmpgtf32", setogt, setugt, F32, f32, f32imm, 0>;
defm CMPGTF64 : FCmpOp<"cmpgtf64", setogt, setugt, F64, f64, f64imm, 0>;

defm CMPGEF32 : FCmpOp<"cmpgef32", setoge, setuge, F32, f32, f32imm, 0>;
defm CMPGEF64 : FCmpOp<"cmpgef64", setoge, setuge, F64, f64, f64imm, 0>;

defm CMPNEF32 : FCmpOp<"cmpnef32", setone, setune, F32, f32, f32imm, 1>;
defm CMPNEF64 : FCmpOp<"cmpnef64", setone, setune, F64, f64, f64imm, 1>;

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


defm FMAF32  : FusedOp1<"fmaf32", fmul, fadd, F32, f32, f32imm>;
defm FMAF32x : FusedOp2<"fmaf32", fmul, fadd, F32, f32, f32imm>; // crude handling of comm.
defm FMSF32  : FusedOp1<"fmsf32", fmul, fsub, F32, f32, f32imm>;
defm FMRSF32 : FusedOp2<"fmrsf32", fmul, fsub, F32, f32, f32imm>;

defm FMAF64  : FusedOp1<"fmaf64", fmul, fadd, F64, f64, f64imm>;
defm FMAF64x : FusedOp2<"fmaf64", fmul, fadd, F64, f64, f64imm>; // crude handling of comm
defm FMSF64  : FusedOp1<"fmsf64", fmul, fsub, F64, f64, f64imm>;
defm FMRSF64 : FusedOp2<"fmrsf64", fmul, fsub, F64, f64, f64imm>;

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

defm ST8   : StOp<"st8\t%ign,",  I8,  i8,  i8imm>;
defm ST16  : StOp<"st16\t%ign,", I16, i16, i16imm>;
defm ST32  : StOp<"st32\t%ign,", I32, i32, i32imm>;
defm ST32f : FStOp<"st32\t%ign,", F32, f32, f32imm>;
defm ST64  : StOp<"st64\t%ign,", I64, i64, i64imm>;
defm ST64f : FStOp<"st64\t%ign,", F64, f64, f64imm>;

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

multiclass MergeOp<string opStr, RegisterClass RC, ValueType opType,
                                 Operand immOpnd> {
  def "" : FMTGEN<
    (outs RC:$dst),
    (ins I1:$sel, RC:$v0, RC:$v1),
    !strconcat(opStr, "\t$dst, $sel, $v0, $v1"),
    [(set RC:$dst, (select I1:$sel, RC:$v1, RC:$v0))]>;
  def i : FMTGEN<
    (outs RC:$dst),
    (ins I1:$sel, RC:$v0, immOpnd:$imm),
    !strconcat(opStr, "\t$dst, $sel, $v0, $imm"),
    [(set RC:$dst, (select I1:$sel, (opType immAny:$imm), RC:$v0))]>;
}
defm MERGE1  : MergeOp<"merge1",  I1,  i1,  i1imm>;
defm MERGE8  : MergeOp<"merge8",  I8,  i8,  i8imm>;
defm MERGE16 : MergeOp<"merge16", I16, i16, i16imm>;
defm MERGE32 : MergeOp<"merge32", I32, i32, i32imm>;
defm MERGE64 : MergeOp<"merge64", I64, i64, i64imm>;

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
  (COPY_TO_REGCLASS $op1, ANYC)>;
//  (AND64 $op1, 0xFFFFFFFF)>;  // gets "Type inference contradiction found, merging 'i32' into 'i64'
def : Pat<(i64 (zext i16:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;
//  (AND64 $op1, 0xFFFF)>;
def : Pat<(i64 (zext i8:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;
//  (AND64 $op1, 0xFF)>;
def : Pat<(i64 (zext i1:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;
//  (AND64 $op1, 1)>;

def : Pat<(i32 (zext i16:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;
//  (AND32 $op1, 0xFFFF)>;
def : Pat<(i32 (zext i8:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;
//  (AND32 $op1, 0xFF)>;
def : Pat<(i32 (zext i1:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;
//  (AND32 $op1, 1)>;

def : Pat<(i16 (zext i8:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;
//  (AND16 $op1, 0xFF)>;
def : Pat<(i16 (zext i1:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;
//  (AND16 $op1, 1)>;

def : Pat<(i8 (zext i1:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;
//  (AND8 $op1, 1)>;

// anyext are treated as zext
def : Pat<(i64 (anyext i32:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;
def : Pat<(i64 (anyext i16:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;
def : Pat<(i64 (anyext i8:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;
def : Pat<(i64 (anyext i1:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;

def : Pat<(i32 (anyext i16:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;
def : Pat<(i32 (anyext i8:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;
def : Pat<(i32 (anyext i1:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;

def : Pat<(i16 (anyext i8:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;
def : Pat<(i16 (anyext i1:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;

def : Pat<(i8 (anyext i1:$op1)),
  (COPY_TO_REGCLASS $op1, ANYC)>;

// Truncate
def : Pat<(i32 (trunc i64:$op1)),
    (COPY_TO_REGCLASS $op1, I32)>;
def : Pat<(i16 (trunc i64:$op1)),
    (COPY_TO_REGCLASS $op1, I16)>;
def : Pat<(i8 (trunc i64:$op1)),
    (COPY_TO_REGCLASS $op1, I8)>;

def : Pat<(i16 (trunc i32:$op1)),
    (COPY_TO_REGCLASS $op1, I16)>;
def : Pat<(i8 (trunc i32:$op1)),
    (COPY_TO_REGCLASS $op1, I8)>;

def : Pat<(i8 (trunc i16:$op1)),
    (COPY_TO_REGCLASS $op1, I8)>;


// bitconvert (shows up in exp)
def : Pat<(i64 (bitconvert f64:$op1)),
    (COPY_TO_REGCLASS $op1, I64)>;
def : Pat<(f64 (bitconvert i64:$op1)),
    (COPY_TO_REGCLASS $op1, F64)>;

def : Pat<(i32 (bitconvert f32:$op1)),
    (COPY_TO_REGCLASS $op1, I32)>;
def : Pat<(f32 (bitconvert i32:$op1)),
    (COPY_TO_REGCLASS $op1, F32)>;

def : Pat<(i16 (bitconvert f16:$op1)),
    (COPY_TO_REGCLASS $op1, I16)>;
def : Pat<(f16 (bitconvert i16:$op1)),
    (COPY_TO_REGCLASS $op1, F16)>;


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
  def BR : FMTGEN<(outs), (ins brtarget:$target),
    "br\t$target",
    [(br bb:$target)]>;
}

def : Pat<(brcond (i1 (xor I1:$cond, -1)), bb:$target),
          (BF I1:$cond, bb:$target)>;
def : Pat<(brcond (i1 (setne I1:$cond, -1)), bb:$target),
          (BF I1:$cond, bb:$target)>;

// Eventually
//include "LPUIntrinsics.td"
