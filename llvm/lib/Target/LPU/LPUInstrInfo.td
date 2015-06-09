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

//def imm64 : PatLeaf<(imm), [{return true;}]>;
//def imm64 : Operand<i64>;

// Latency insensitive channels

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

// Operand sets - R - Register, C - Channel, L - Literal
// These match simulator
class OpndC<ValueType t> : Operand<t>;
class OpndR<ValueType t> : Operand<t>;
class OpndL<ValueType t> : Operand<t>;
class OpndCR<ValueType t> : Operand<t>;
class OpndCL<ValueType t> : Operand<t>;
class OpndRL<ValueType t> : Operand<t>;
class OpndCRL<ValueType t> : Operand<t>;

// Memory operands
class Addr<int numArgs, string funcName, dag opInfo> :
  Operand<i64>, ComplexPattern<i64, numArgs,
    funcName, [],
    [SDNPWantParent]> { let MIOperandInfo = opInfo; }

def ADDR_R : Addr<1, "SelectAddrReg", (ops RCI64:$base)>;


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
                               "// ADJCALLSTACKDOWN $amt",
                               [(callseq_start timm:$amt)]>;
def ADJCALLSTACKUP : PseudoInstLPU<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                            "// ADJCALLSTACKUP $amt1",
                            [(callseq_end timm:$amt1, timm:$amt2)]>;

def SDT_LPURet : SDTypeProfile<0, 0, []>;

def LPURet : SDNode<"LPUISD::Ret", SDT_LPURet,
    [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;


let isReturn=1, isTerminator=1, isBarrier=1 in
def RET : FMTGEN<
    (outs), (ins),
    "// return",
    [(LPURet)]>;

multiclass MovOp<string opName, ValueType T> {
  def "" : FMTGEN<
    (outs OpndCR<T>:$dst),
    (ins OpndCRL<T>:$op1),
    !strconcat("$dst = ", opName, "($op1);"),
    []>;
}

// Binary op, no pattern, not commutable
multiclass SExtOp<string opName, ValueType T> {
  def "" : FMTGEN<
    (outs OpndCR<T>:$dst),
    (ins OpndCRL<T>:$op1, OpndCRL<T>:$op2),
    !strconcat("$dst = ", opName, "($op1,$op2);"),
    []>;
}

// This uses a multiclass even though it only declares one class because
// class doesn't allow the Opndxx in the ins/outs list.  (Why?)
// (Also, nested multiclass would be nice - it would allow declaring 4 defs
// per operator, rather than enumerating 8/16/32/64, etc.)
multiclass BinOp<string opName, SDNode opNode, ValueType T, bit commutable> {
  let isCommutable = commutable in {
    def "" : FMTGEN<
      (outs OpndCR<T>:$dst),
      (ins OpndCRL<T>:$op1, OpndCRL<T>:$op2),
      !strconcat("$dst = ", opName, "($op1,$op2);"),
      [(set T:$dst, (opNode T:$op1, T:$op2))]>;
  }
}

multiclass CmpOp<string opName, PatFrag opNode, ValueType T, bit commutable> {
  let isCommutable = commutable in {
    def "" : FMTGEN<
      (outs OpndCR<i1>:$dst),
      (ins OpndCRL<T>:$op1, OpndCRL<T>:$op2),
      !strconcat("$dst = ", opName, "($op1,$op2);"),
      [(set i1:$dst, (opNode T:$op1, T:$op2))]>;
  }
}

// This provides a full set of result register types.  Some are nonsensical
// (e.g. LD8_F64 or LD32_I64)
multiclass LdOp<string opName, ValueType T> {
  def "" : FMTGEN<
    (outs OpndCR<T>:$dst),
    (ins OpndCRL<i64>:$addr),
    !strconcat("$dst = ", opName, "($addr);"),
    [(set T:$dst, (load ADDR_R:$addr))]>;
}

// TODO: Handling store output
multiclass StOp<string opName, ValueType T> {
  def "" : FMTGEN<
    (outs),  // How to handle output flag??
    (ins OpndCRL<i64>:$addr, OpndCRL<T>:$data),
    !strconcat(opName, "($addr,$data);"),
    [(store T:$data, ADDR_R:$addr)]>;
}

defm MOV1  : MovOp<"mov1",  i1>;
defm MOV8  : MovOp<"mov8",  i8>;
defm MOV16 : MovOp<"mov16", i16>;
defm MOV32 : MovOp<"mov32", i32>;
defm MOV64 : MovOp<"mov64", i64>;

//Todo: NOT1-64, NEG8-64, ABS32-64, SQRT32-64
//Todo: Conversions

// Todo: LDS8, LDS16, LDS32
defm LD8   : LdOp<"ld8",  i8>;
defm LD16  : LdOp<"ld16", i16>;
defm LD32  : LdOp<"ld32", i32>;
defm LD64  : LdOp<"ld64", i64>;

defm ST8   : StOp<"st8",  i8>;
defm ST16  : StOp<"st16", i16>;
defm ST32  : StOp<"st32", i32>;
defm ST64  : StOp<"st64", i64>;

defm AND1  : BinOp<"and1",  and, i1,  1>;
defm AND8  : BinOp<"and8",  and, i8,  1>;
defm AND16 : BinOp<"and16", and, i16, 1>;
defm AND32 : BinOp<"and32", and, i32, 1>;
defm AND64 : BinOp<"and64", and, i64, 1>;

defm OR1   : BinOp<"or1",   or,  i1,  1>;
defm OR8   : BinOp<"or8",   or,  i8,  1>;
defm OR16  : BinOp<"or16",  or,  i16, 1>;
defm OR32  : BinOp<"or32",  or,  i32, 1>;
defm OR64  : BinOp<"or64",  or,  i64, 1>;

defm XOR1  : BinOp<"xor1",  xor, i1,  1>;
defm XOR8  : BinOp<"xor8",  xor, i8,  1>;
defm XOR16 : BinOp<"xor16", xor, i16, 1>;
defm XOR32 : BinOp<"xor32", xor, i32, 1>;
defm XOR64 : BinOp<"xor64", xor, i64, 1>;

defm SEXT8  : SExtOp<"sext8",  i8>;
defm SEXT16 : SExtOp<"sext16", i16>;
defm SEXT32 : SExtOp<"sext32", i32>;
defm SEXT64 : SExtOp<"sext64", i64>;

defm SLL8  : BinOp<"sll8",  shl, i8,  0>;
defm SLL16 : BinOp<"sll16", shl, i16, 0>;
defm SLL32 : BinOp<"sll32", shl, i32, 0>;
defm SLL64 : BinOp<"sll64", shl, i64, 0>;

defm SRL8  : BinOp<"srl8",  srl, i8,  0>;
defm SRL16 : BinOp<"srl16", srl, i16, 0>;
defm SRL32 : BinOp<"srl32", srl, i32, 0>;
defm SRL64 : BinOp<"srl64", srl, i64, 0>;

defm SRA8  : BinOp<"sra8",  sra, i8,  0>;
defm SRA16 : BinOp<"sra16", sra, i16, 0>;
defm SRA32 : BinOp<"sra32", sra, i32, 0>;
defm SRA64 : BinOp<"sra64", sra, i64, 0>;

defm ADD8  : BinOp<"add8",  add, i8,  1>;
defm ADD16 : BinOp<"add16", add, i16, 1>;
defm ADD32 : BinOp<"add32", add, i32, 1>;
defm ADD64 : BinOp<"add64", add, i64, 1>;

defm ADDF32 : BinOp<"add32f", fadd, f32, 1>;
defm ADDF64 : BinOp<"addf64", fadd, f64, 1>;

defm SUB8  : BinOp<"sub8",  sub, i8,  0>;
defm SUB16 : BinOp<"sub16", sub, i16, 0>;
defm SUB32 : BinOp<"sub32", sub, i32, 0>;
defm SUB64 : BinOp<"sub64", sub, i64, 0>;

defm SUBF32 : BinOp<"sub32f", fsub, f32, 1>;
defm SUBF64 : BinOp<"subf64", fsub, f64, 1>;

defm MUL8  : BinOp<"mul8",  mul, i8,  1>;
defm MUL16 : BinOp<"mul16", mul, i16, 1>;
defm MUL32 : BinOp<"mul32", mul, i32, 1>;
defm MUL64 : BinOp<"mul64", mul, i64, 1>;

defm MULF32 : BinOp<"mul32f", fmul, f32, 1>;
defm MULF64 : BinOp<"mulf64", fmul, f64, 1>;

defm DIVS8  : BinOp<"divs8",  sdiv, i8,  0>;
defm DIVS16 : BinOp<"divs16", sdiv, i16, 0>;
defm DIVS32 : BinOp<"divs32", sdiv, i32, 0>;
defm DIVS64 : BinOp<"divs64", sdiv, i64, 0>;

defm DIVU8  : BinOp<"divu8",  udiv, i8,  0>;
defm DIVU16 : BinOp<"divu16", udiv, i16, 0>;
defm DIVU32 : BinOp<"divu32", udiv, i32, 0>;
defm DIVU64 : BinOp<"divu64", udiv, i64, 0>;

defm DIVF32 : BinOp<"div32f", fdiv, f32, 0>;
defm DIVF64 : BinOp<"divf64", fdiv, f64, 0>;

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

defm CMPEQS8  : CmpOp<"cmpeqs8",  seteq,  i8,  1>;
defm CMPEQS16 : CmpOp<"cmpeqs16", seteq,  i16, 1>;
defm CMPEQS32 : CmpOp<"cmpeqs32", seteq,  i32, 1>;
defm CMPEQS64 : CmpOp<"cmpeqs64", seteq,  i64, 1>;

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

defm CMPNES8  : CmpOp<"cmpnes8",  setne,  i8,  1>;
defm CMPNES16 : CmpOp<"cmpnes16", setne,  i16, 1>;
defm CMPNES32 : CmpOp<"cmpnes32", setne,  i32, 1>;
defm CMPNES64 : CmpOp<"cmpnes64", setne,  i64, 1>;


// Arbitrary immediate support
def: Pat<(i1 imm:$imm),  (MOV1 imm:$imm)>;
def: Pat<(i32 imm:$imm), (MOV32 imm:$imm)>;
def: Pat<(i64 imm:$imm), (MOV64 imm:$imm)>;

// sext/zext
def : Pat<(i64 (sext RCI32:$op1)),
  (SEXT64 (COPY_TO_REGCLASS RCI32:$op1, RCI64), 32)>;
def : Pat<(i64 (sext RCI16:$op1)),
  (SEXT64 (COPY_TO_REGCLASS RCI16:$op1, RCI64), 16)>;
def : Pat<(i64 (sext RCI8:$op1)),
  (SEXT64 (COPY_TO_REGCLASS RCI8:$op1,  RCI64),  8)>;

def : Pat<(i32 (sext RCI16:$op1)),
  (SEXT32 (COPY_TO_REGCLASS RCI16:$op1, RCI64), 16)>;
def : Pat<(i32 (sext RCI8:$op1)),
  (SEXT32 (COPY_TO_REGCLASS RCI8:$op1,  RCI64),  8)>;

def : Pat<(i16 (sext RCI8:$op1)),
  (SEXT16 (COPY_TO_REGCLASS RCI8:$op1,  RCI64),  8)>;

// sext_inreg are the same
def : Pat<(i64 (sext_inreg RCI64:$op1, i32)),
  (SEXT64 RCI64:$op1, 32)>;
def : Pat<(i64 (sext_inreg RCI64:$op1, i16)),
  (SEXT64 RCI64:$op1, 16)>;
def : Pat<(i64 (sext_inreg RCI64:$op1, i8)),
  (SEXT64 RCI64:$op1, 8)>;

def : Pat<(i32 (sext_inreg RCI32:$op1, i16)),
  (SEXT32 RCI32:$op1, 16)>;
def : Pat<(i32 (sext_inreg RCI32:$op1, i8)),
  (SEXT32 RCI32:$op1, 8)>;

def : Pat<(i16 (sext_inreg RCI16:$op1, i8)),
  (SEXT16 RCI16:$op1, 8)>;

// zext patterns
def : Pat<(i64 (zext RCI32:$op1)),
  (COPY_TO_REGCLASS RCI32:$op1, RCI64)>;
def : Pat<(i64 (zext RCI16:$op1)),
  (COPY_TO_REGCLASS RCI16:$op1, RCI64)>;
def : Pat<(i64 (zext RCI8:$op1)),
  (COPY_TO_REGCLASS RCI8:$op1,  RCI64)>;
def : Pat<(i64 (zext RCI1:$op1)),
  (COPY_TO_REGCLASS RCI1:$op1,  RCI64)>;

def : Pat<(i32 (zext RCI16:$op1)),
  (COPY_TO_REGCLASS RCI16:$op1, RCI32)>;
def : Pat<(i32 (zext RCI8:$op1)),
  (COPY_TO_REGCLASS RCI8:$op1,  RCI32)>;
def : Pat<(i32 (zext RCI1:$op1)),
  (COPY_TO_REGCLASS RCI1:$op1,  RCI32)>;

def : Pat<(i16 (zext RCI8:$op1)),
  (COPY_TO_REGCLASS RCI8:$op1,  RCI16)>;
def : Pat<(i16 (zext RCI1:$op1)),
  (COPY_TO_REGCLASS RCI1:$op1,  RCI16)>;

def : Pat<(i8 (zext RCI1:$op1)),
  (COPY_TO_REGCLASS RCI1:$op1,  RCI8)>;

// anyext are treated as zext
def : Pat<(i64 (anyext RCI32:$op1)),
  (COPY_TO_REGCLASS RCI32:$op1, RCI64)>;
def : Pat<(i64 (anyext RCI16:$op1)),
  (COPY_TO_REGCLASS RCI16:$op1, RCI64)>;
def : Pat<(i64 (anyext RCI8:$op1)),
  (COPY_TO_REGCLASS RCI8:$op1,  RCI64)>;
def : Pat<(i64 (anyext RCI1:$op1)),
  (COPY_TO_REGCLASS RCI1:$op1,  RCI64)>;

def : Pat<(i32 (anyext RCI16:$op1)),
  (COPY_TO_REGCLASS RCI16:$op1, RCI32)>;
def : Pat<(i32 (anyext RCI8:$op1)),
  (COPY_TO_REGCLASS RCI8:$op1,  RCI32)>;
def : Pat<(i32 (anyext RCI1:$op1)),
  (COPY_TO_REGCLASS RCI1:$op1,  RCI32)>;

def : Pat<(i16 (anyext RCI8:$op1)),
  (COPY_TO_REGCLASS RCI8:$op1,  RCI16)>;
def : Pat<(i16 (anyext RCI1:$op1)),
  (COPY_TO_REGCLASS RCI1:$op1,  RCI16)>;

def : Pat<(i8 (anyext RCI1:$op1)),
  (COPY_TO_REGCLASS RCI1:$op1,  RCI8)>;

// Eventually
//include "LPUIntrinsics.td"
