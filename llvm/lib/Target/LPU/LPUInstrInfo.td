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

def LIT : PatLeaf<(imm), [{return true;}]>;

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

// TODO: How do define operand that is REGx OR LICx OR LIT?
// Want same combos as assembly...

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


// TODO: What do we want in place of RegisterClass?  PatFrag?

class BINOP<bits<8> opc, string opName, SDNode opNode, RegisterClass RC> :
  FMTGEN<opc, (outs RC:$dst), (ins RC:$op1, RC:$op2),
    	    !strconcat("$dst = ", opName, "($op1, $op2);"),
	    [(set RC:$dst, (opNode RC:$op1, RC:$op2))]>;

def ADD32 : BINOP<0b00000000, "add", add, RCI32>;


// Eventually
//include "LPUIntrinsics.td"
