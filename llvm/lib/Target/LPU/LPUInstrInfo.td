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

def LICF16 : LIC<f16>;
def LICF32 : LIC<f32>;
def LICF64 : LIC<f64>;

// Operand sets - R - Register, C - Channel, L - Literal
// These match simulator
class OpndR<ValueType t> : Operand<t>;
class OpndL<ValueType t> : Operand<t>;
class OpndC<ValueType t> : Operand<t>;
class OpndRL<ValueType t> : Operand<t>;
class OpndRC<ValueType t> : Operand<t>;
class OpndLC<ValueType t> : Operand<t>;
class OpndRLC<ValueType t> : Operand<t>;

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


// Unary operator
//class UNARY<string opName, SDNode opNode, Operand op> :
//    FMTGEN<(outs RC

let isReturn=1, isTerminator=1, isBarrier=1 in
def RET : FMTGEN<
    (outs), (ins),
    "// return",
    [(LPURet)]>;

def MOV1  : FMTGEN<
    (outs OpndRC<i1>:$dst), (ins OpndRLC<i1>:$op1),
    "$dst = mov1($op1);",
    []>;

def MOV32  : FMTGEN<
    (outs OpndRC<i32>:$dst), (ins OpndRLC<i32>:$op1),
    "$dst = mov32($op1);",
    []>;

def MOV64  : FMTGEN<
    (outs OpndRC<i64>:$dst), (ins OpndRLC<i64>:$op1),
    "$dst = mov64($op1);",
    []>;

// Arbitrary immediate support
def: Pat<(i1 imm:$imm),  (MOV1 imm:$imm)>;
def: Pat<(i32 imm:$imm), (MOV32 imm:$imm)>;
def: Pat<(i64 imm:$imm), (MOV64 imm:$imm)>;

// This uses a multiclass even though it only declares one class because
// class doesn't allow the Opndxx in the ins/outs list.  (Why?)
multiclass BinOp<string opName, SDNode opNode, ValueType T, bit commutable> {
  let isCommutable = commutable in {
    def "" : FMTGEN<
      (outs OpndRC<T>:$dst),
      (ins OpndRLC<T>:$op1, OpndRLC<T>:$op2),
      !strconcat("$dst = ", opName, "($op1,$op2);"),
      [(set T:$dst, (opNode T:$op1, T:$op2))]>;
  }
}

// This provides a full set of result register types.  Some are nonsensical
// (e.g. LD8_F64 or LD32_I64)
multiclass LdOp<string opName, ValueType T> {
  def "" : FMTGEN<
    (outs OpndRC<T>:$dst),
    (ins OpndRLC<i64>:$addr),
    !strconcat("$dst = ", opName, "($addr);"),
    [(set T:$dst, (load ADDR_R:$addr))]>;
}

// TODO: Handling store output
multiclass StOp<string opName, ValueType T> {
  def "" : FMTGEN<
    (outs),  // How to handle output flag??
    (ins OpndRLC<i64>:$addr, OpndRLC<T>:$data),
    !strconcat(opName, "($addr,$data);"),
    [(store T:$data, ADDR_R:$addr)]>;
}

defm LD8  : LdOp<"ld8",  i8>;
defm LD16 : LdOp<"ld16", i16>;
defm LD32 : LdOp<"ld32", i32>;
defm LD64 : LdOp<"ld64", i64>;

defm ST8  : StOp<"st8",  i8>;
defm ST16 : StOp<"st16", i16>;
defm ST32 : StOp<"st32", i32>;
defm ST64 : StOp<"st64", i64>;

//defm AND1  : BinOp<"and1",  and, i1,  1>;
defm AND32 : BinOp<"and32", and, i32, 1>;
defm AND64 : BinOp<"and64", and, i64, 1>;

//defm OR1   : BinOp<"or1",   or,  i1,  1>;
defm OR32  : BinOp<"or32",  or,  i32, 1>;
defm OR64  : BinOp<"or64",  or,  i64, 1>;

//defm XOR1  : BinOp<"xor1",  xor, i1,  1>;
defm XOR32 : BinOp<"xor32", xor, i32, 1>;
defm XOR64 : BinOp<"xor64", xor, i64, 1>;

defm SLL32 : BinOp<"sll32", shl, i32, 0>;
defm SLL64 : BinOp<"sll64", shl, i64, 0>;

defm SRL32 : BinOp<"srl32", srl, i32, 0>;
defm SRL64 : BinOp<"srl64", srl, i64, 0>;

defm SRA32 : BinOp<"sra32", sra, i32, 0>;
defm SRA64 : BinOp<"sra64", sra, i64, 0>;

defm ADD32 : BinOp<"add32", add, i32, 1>;
defm ADD64 : BinOp<"add64", add, i64, 1>;

defm SUB32 : BinOp<"sub32", sub, i32, 0>;
defm SUB64 : BinOp<"sub64", sub, i64, 0>;

defm MUL32 : BinOp<"mul32", mul, i32, 1>;
defm MUL64 : BinOp<"mul64", mul, i64, 1>;

defm DIVS32 : BinOp<"div32s", sdiv, i32, 0>;
defm DIVS64 : BinOp<"divs64", sdiv, i64, 0>;

defm DIVU32 : BinOp<"div32u", udiv, i32, 0>;
defm DIVU64 : BinOp<"divu64", udiv, i64, 0>;

defm ADDF32 : BinOp<"add32f", fadd, f32, 1>;
defm ADDF64 : BinOp<"addf64", fadd, f64, 1>;

defm SUBF32 : BinOp<"sub32f", fsub, f32, 1>;
defm SUBF64 : BinOp<"subf64", fsub, f64, 1>;

defm MULF32 : BinOp<"mul32f", fmul, f32, 1>;
defm MULF64 : BinOp<"mulf64", fmul, f64, 1>;

defm DIVF32 : BinOp<"div32f", fdiv, f32, 0>;
defm DIVF64 : BinOp<"divf64", fdiv, f64, 0>;

def SEXT32 : FMTGEN<
  (outs OpndRLC<i32>:$dst),
  (ins OpndRLC<i32>:$op1, OpndRLC<i32>:$op2),
  "sext32($op1,$op2);",
  []>;

def SEXT64 : FMTGEN<
  (outs OpndRLC<i64>:$dst),
  (ins OpndRLC<i64>:$op1, OpndRLC<i64>:$op2),
  "sext32($op1,$op2);", []>;

// sext/zext
def : Pat<(i64 (sext RCI32:$op1)),
  (SEXT64 (COPY_TO_REGCLASS RCI32:$op1, RCI64), 32)>;
//def : Pat<(i64 (sext RCI16:$op1)),
//  (SEXT64 (COPY_TO_REGCLASS RCI16:$op1, RCI64), 16)>;
//def : Pat<(i64 (sext RCI8:$op1)),
//  (SEXT64 (COPY_TO_REGCLASS RCI8:$op1,  RCI64),  8)>;

//def : Pat<(i32 (sext RCI16:$op1)),
//  (SEXT32 (COPY_TO_REGCLASS RCI16:$op1, RCI64), 16)>;
//def : Pat<(i32 (sext RCI8:$op1)),
//  (SEXT32 (COPY_TO_REGCLASS RCI8:$op1,  RCI64),  8)>;

//def : Pat<(i16 (sext RCI8:$op1)),
//  (SEXT16 (COPY_TO_REGCLASS RCI8:$op1,  RCI64),  8)>;

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

//def : Pat<(i16 (sext_inreg RCI16:$op1, i8)),    (SEXT16_8  RCI16:$op1, 8)>;

// zext patterns
def : Pat<(i64 (zext RCI32:$op1)),
  (AND64 (COPY_TO_REGCLASS RCI32:$op1, RCI64), 0xFFFFFFFF)>;
//def : Pat<(i64 (zext RCI16:$op1)),
//  (AND64 (COPY_TO_REGCLASS RCI16:$op1, RCI64), 0xFFFF)>;
//def : Pat<(i64 (zext RCI8:$op1)),
//  (AND64 (COPY_TO_REGCLASS RCI8:$op1,  RCI64), 0xFF)>;

//def : Pat<(i32 (zext RCI16:$op1)),
//  (AND32 (COPY_TO_REGCLASS RCI16:$op1, RCI32), 0xFFFF)>;
//def : Pat<(i32 (zext RCI8:$op1)),
//  (AND32 (COPY_TO_REGCLASS RCI8:$op1,  RCI32), 0xFF)>;

//def : Pat<(i16 (zext RCI8:$op1)),
//  (AND16 (COPY_TO_REGCLASS RCI8:$op1,  RCI16), 0xFF)>;

// anyext are treated as zext
def : Pat<(i64 (anyext RCI32:$op1)),
  (AND64 (COPY_TO_REGCLASS RCI32:$op1, RCI64), 0xFFFFFFFF)>;
//def : Pat<(i64 (anyext RCI16:$op1)),
//  (AND64 (COPY_TO_REGCLASS RCI16:$op1, RCI64), 0xFFFF)>;
//def : Pat<(i64 (anyext RCI8:$op1)),
//  (AND64 (COPY_TO_REGCLASS RCI8:$op1,  RCI64), 0xFF)>;

//def : Pat<(i32 (anyext RCI16:$op1)),
//  (AND32 (COPY_TO_REGCLASS RCI16:$op1, RCI32), 0xFFFF)>;
//def : Pat<(i32 (anyext RCI8:$op1)),
//  (AND32 (COPY_TO_REGCLASS RCI8:$op1,  RCI32), 0xFF)>;

//def : Pat<(i16 (anyext RCI8:$op1)),
//  (AND16 (COPY_TO_REGCLASS RCI8:$op1,  RCI16), 0xFF)>;

// Eventually
//include "LPUIntrinsics.td"
