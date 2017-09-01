//===- Nios2InstrInfo.td - Target Description for Nios2 Target -*- tablegen -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Nios2 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

//===----------------------------------
// Nios2 Instruction predicates
//===----------------------------------
def RelocStatic : Predicate<"TM.getRelocationModel() == Reloc::Static">;


//===----------------------------------------------------------------------===//
// Nios2 profiles and nodes
//===----------------------------------------------------------------------===//

def SDT_Nios2JmpLink      : SDTypeProfile<0, 1, [SDTCisVT<0, iPTR>]>;
def SDT_Nios2CMov         : SDTypeProfile<1, 4, [SDTCisSameAs<0, 1>,
                                                SDTCisSameAs<1, 2>,
                                                SDTCisSameAs<3, 4>,
                                                SDTCisInt<4>]>;
def SDT_Nios2CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>]>;
def SDT_Nios2CallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_Nios2MAddMSub     : SDTypeProfile<0, 4,
                                         [SDTCisVT<0, i32>, SDTCisSameAs<0, 1>,
                                          SDTCisSameAs<1, 2>,
                                          SDTCisSameAs<2, 3>]>;
def SDT_Nios2DivRem       : SDTypeProfile<0, 2,
                                         [SDTCisInt<0>,
                                          SDTCisSameAs<0, 1>]>;

def SDT_Nios2ThreadPointer : SDTypeProfile<1, 0, [SDTCisPtrTy<0>]>;

def SDT_Nios2DynAlloc    : SDTypeProfile<1, 1, [SDTCisVT<0, iPTR>,
                                               SDTCisSameAs<0, 1>]>;
def SDT_Sync             : SDTypeProfile<0, 1, [SDTCisVT<0, i32>]>;

def SDT_Ext : SDTypeProfile<1, 3, [SDTCisInt<0>, SDTCisSameAs<0, 1>,
                                   SDTCisVT<2, i32>, SDTCisSameAs<2, 3>]>;
def SDT_Ins : SDTypeProfile<1, 4, [SDTCisInt<0>, SDTCisSameAs<0, 1>,
                                   SDTCisVT<2, i32>, SDTCisSameAs<2, 3>,
                                   SDTCisSameAs<0, 4>]>;

def SDTNios2LoadLR  : SDTypeProfile<1, 2,
                                   [SDTCisInt<0>, SDTCisPtrTy<1>,
                                    SDTCisSameAs<0, 2>]>;

// Call
def Nios2JmpLink : SDNode<"Nios2ISD::JmpLink",SDT_Nios2JmpLink,
                         [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                          SDNPVariadic]>;

// Hi and Lo nodes are used to handle global addresses. Used on
// Nios2ISelLowering to lower stuff like GlobalAddress, ExternalSymbol
// static model. (nothing to do with Nios2 Registers Hi and Lo)
def Nios2Hi    : SDNode<"Nios2ISD::Hi", SDTIntUnaryOp>;
def Nios2Lo    : SDNode<"Nios2ISD::Lo", SDTIntUnaryOp>;
def Nios2Low    : SDNode<"Nios2ISD::Lo", SDTIntBinOp, []>;
def Nios2GPRel : SDNode<"Nios2ISD::GPRel", SDTIntUnaryOp>;

// Return
def Nios2Ret : SDNode<"Nios2ISD::Ret", SDTNone,
    [SDNPHasChain, SDNPOptInGlue,SDNPVariadic]>;

// Select
def Nios2Select : SDNode<"Nios2ISD::Select", SDTSelect>;

def Nios2DivRem  : SDNode<"Nios2ISD::DivRem",  SDT_Nios2DivRem>;
def Nios2DivRemU : SDNode<"Nios2ISD::DivRemU", SDT_Nios2DivRem>;

// These are target-independent nodes, but have target-specific formats.
def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_Nios2CallSeqStart,
                           [SDNPHasChain, SDNPSideEffect, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_Nios2CallSeqEnd,
                           [SDNPHasChain, SDNPSideEffect,
                            SDNPOptInGlue, SDNPOutGlue]>;

// Target constant nodes that are not part of any isel patterns and remain
// unchanged can cause instructions with illegal operands to be emitted.
// Wrapper node patterns give the instruction selector a chance to replace
// target constant nodes that would otherwise remain unchanged with ADDiu
// nodes. Without these wrapper node patterns, the following conditional move
// instrucion is emitted when function cmov2 in test/CodeGen/Nios2/cmov.ll is
// compiled:
//  movn  %got(d)($gp), %got(c)($gp), $4
// This instruction is illegal since movn can take only register operands.

def Nios2Wrapper : SDNode<"Nios2ISD::Wrapper", SDTIntBinOp>;
def Nios2Sync : SDNode<"Nios2ISD::Sync", SDTNone, [SDNPHasChain,SDNPSideEffect]>;

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//

include "Nios2InstrFormats.td"

//===----------------------------------------------------------------------===//
// Nios2 Operand, Complex Patterns and Transformations Definitions.
//===----------------------------------------------------------------------===//

// Instruction operand types
def jmptarget   : Operand<OtherVT> {
  let EncoderMethod = "getJumpTargetOpValue";
}
def brtarget    : Operand<OtherVT> {
  let EncoderMethod = "getBranch16TargetOpValue";
  let OperandType = "OPERAND_PCREL";
  let DecoderMethod = "DecodeBranch16Target";
}
def calltarget  : Operand<iPTR> {
  let EncoderMethod = "getJumpTargetOpValue";
}
def calltarget64: Operand<i64>;
def simm16      : Operand<i32> {
  let DecoderMethod= "DecodeSimm16";
}
def simm16_64   : Operand<i64>;
def shamt       : Operand<i32>;

// Unsigned Operand
def uimm16      : Operand<i32> {
  let PrintMethod = "printUnsignedImm";
}

def uimm6       : Operand<i32> {
  let PrintMethod = "printUnsignedImm";
}

// Address operand
def mem : Operand<i32> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops CPURegs, simm16);
}

def mem6 : Operand<i8> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops CPURegs, uimm6);
}

def mem_ea : Operand<i32> {
  let PrintMethod = "printMemOperandEA";
  let MIOperandInfo = (ops CPURegs, simm16);
}

// size operand of ext instruction
def size_ext : Operand<i32> {
  let EncoderMethod = "getSizeExtEncoding";
  let DecoderMethod = "DecodeExtSize";
}

// size operand of ins instruction
def size_ins : Operand<i32> {
  let EncoderMethod = "getSizeInsEncoding";
  let DecoderMethod = "DecodeInsSize";
}

// Transformation Function - get the lower 16 bits.
def LO16 : SDNodeXForm<imm, [{
  return getImm(N, N->getZExtValue() & 0xFFFF);
}]>;

// Transformation Function - get the higher 16 bits.
def HI16 : SDNodeXForm<imm, [{
  return getImm(N, (N->getZExtValue() >> 16) & 0xFFFF);
}]>;

// Transformation Function - get the negative number
def NEG16 : SDNodeXForm<imm, [{
  return getImm(N, -N->getSExtValue() );
}]>;

// Transformation Function - get the immediate + 1
def IMMPLUS1 : SDNodeXForm<imm, [{
  return getImm(N, N->getSExtValue()+1);
}]>;

// Map immediates to control registers
def ICTLREG : SDNodeXForm<imm, [{
  return getRegister(Nios2::CTL0 + N->getZExtValue(), MVT::i32);
}]>;

// Node immediate fits as 16-bit sign extended on target immediate.
// e.g. addi, andi
def immSExt16  : PatLeaf<(imm), [{ return isInt<16>(N->getSExtValue()); }]>;

// Node immediate fits as 16-bit zero extended on target immediate.
// The LO16 param means that only the lower 16 bits of the node
// immediate are caught.
// e.g. addiu, sltiu
def immZExt16  : PatLeaf<(imm), [{
  if (N->getValueType(0) == MVT::i32)
    return (uint32_t)N->getZExtValue() == (unsigned short)N->getZExtValue();
  else
    return (uint64_t)N->getZExtValue() == (unsigned short)N->getZExtValue();
}], LO16>;

// Immediate can be loaded with LUi (32-bit int with lower 16-bit cleared).
def immLow16Zero : PatLeaf<(imm), [{
  int64_t Val = N->getSExtValue();
  return isInt<32>(Val) && !(Val & 0xffff);
}]>;

// shamt field must fit in 5 bits.
def immZExt5 : ImmLeaf<i32, [{return Imm == (Imm & 0x1f);}]>;

// Nios2 Address Mode! SDNode frameindex could possibily be a match
// since load and store instructions from stack used it.
def addr :
  ComplexPattern<iPTR, 2, "SelectAddr", [frameindex], [SDNPWantParent]>;

def addrCdx :
  ComplexPattern<iPTR, 2, "SelectAddrCDX", [frameindex], [SDNPWantParent]>;

//===----------------------------------------------------------------------===//
// Instructions specific format
//===----------------------------------------------------------------------===//

// Arithmetic and logical instructions with 3 register operands.
multiclass ArithLogicReg<bits<6> opx, string mnemonic,
                         SDNode opNode, InstrItinClass itin>:
           CommonInstr_R_F3X6<opx, (outs CPURegs:$rC),
                              (ins CPURegs:$rA, CPURegs:$rB),
                              !strconcat(mnemonic, "\t$rC, $rA, $rB"),
                              [(set CPURegs:$rC, (opNode CPURegs:$rA, CPURegs:$rB))],
                              itin>;

// Arithmetic and logical instructions with 2 registers and 5-bit immediate value.
multiclass ArithLogicRegImm5<bits<6> opx, string mnemonic,
                             SDNode opNode, InstrItinClass itin>:
           CommonInstr_R_F3X6<opx, (outs CPURegs:$rC),
                              (ins CPURegs:$rA, CPURegs:$imm),
                              !strconcat(mnemonic, "\t$rC, $rA, $imm"),
                              [(set CPURegs:$rC, (opNode CPURegs:$rA, immZExt5:$imm))],
                              itin>;

// Arithmetic and logical instructions with 2 registers and 16-bit immediate value.
multiclass ArithLogicRegImm16<bits<6> op, string mnemonic, SDNode opNode,
                              Operand immOp, PatLeaf immType>:
           CommonInstr_I_F2I16<op, (outs CPURegs:$rB), (ins CPURegs:$rA, immOp:$imm),
                               !strconcat(mnemonic, "\t$rB, $rA, $imm"),
                               [(set CPURegs:$rB, (opNode CPURegs:$rA, immType:$imm))],
                               IIAlu>;

// Arithmetic and logical instructions with 2 registers and 16-bit immediate value.
// Without defined pattern, used for instruction with 'h' suffix.
multiclass ArithLogicRegImm16NoPat<bits<6> op, string mnemonic, SDNode opNode,
                                   Operand immOp, PatLeaf immType>:
           CommonInstr_I_F2I16<op, (outs CPURegs:$rB), (ins CPURegs:$rA, immOp:$imm),
                               !strconcat(mnemonic, "\t$rB, $rA, $imm"),
                               [],
                               IIAlu>;

multiclass FlushOrInit<bits<6> opR1, bits<5> opyR2, string mnemonic> {
  let rB = 0 in
  def NAME#_R1 : FI<opR1, (outs), (ins mem:$addr), !strconcat(mnemonic, "\t$addr"), [], IIFlush>;

  def NAME#_R2 : F1X4I12<opyR2, (outs), (ins mem:$addr), !strconcat(mnemonic, "\t$addr"), [], IIFlush>;
}


multiclass Load<bits<6> opR1, bits<6> opR2, string mnemonic, PatFrag opNode> {
  let canFoldAsLoad = 1 in
  defm NAME# : CommonInstr_I_F2I16_op<opR1, opR2, (outs CPURegs:$rB), (ins mem:$addr),
                                      !strconcat(mnemonic, "\t$rB, $addr"),
                                      [(set CPURegs:$rB, (opNode addr:$addr))], IILoad>;
}

multiclass LoadImm<bits<6> opR1, bits<4> opxR2, string mnemonic, PatFrag opNode> {
  let canFoldAsLoad = 1 in {
    def NAME#_R1 : FI<opR1, (outs CPURegs:$rB), (ins mem:$addr),
                      !strconcat(mnemonic, "\t$rB, $addr"),
                      [(set CPURegs:$rB, (opNode addr:$addr))], IILoad>;
    def NAME#_R2 : F2X4I12<opxR2, (outs CPURegs:$rB), (ins mem:$addr),
                          !strconcat(mnemonic, "\t$rB, $addr"),
                          [(set CPURegs:$rB, (opNode addr:$addr))], IILoad>;
  }
}

multiclass Store<bits<6> opR1, bits<6> opR2, string mnemonic, PatFrag opNode> {
  defm NAME# : CommonInstr_I_F2I16_op<opR1, opR2, (outs), (ins CPURegs:$rB, mem:$addr),
                                      !strconcat(mnemonic, "\t$rB, $addr"),
                                      [(opNode CPURegs:$rB, addr:$addr)], IIStore>;
}

multiclass StoreImm<bits<6> opR1, bits<4> opxR2, string mnemonic, PatFrag opNode> {
  def NAME#_R1 : FI<opR1, (outs), (ins CPURegs:$rB, mem:$addr),
                    !strconcat(mnemonic, "\t$rB, $addr"),
                    //[(opNode CPURegs:$rB, addr:$addr)], IIStore>;
                    [], IIStore>;
  def NAME#_R2 : F2X4I12<opxR2, (outs), (ins CPURegs:$rB, mem:$addr),
                         !strconcat(mnemonic, "\t$rB, $addr"),
                         //[(opNode CPURegs:$rB, addr:$addr)], IIStore>;
                         [], IIStore>;
}

multiclass Return<bits<6> opx, dag outs, dag ins, string mnemonic> {
  let rB = 0, rC = 0,
      isReturn = 1,
      isCodeGenOnly = 1,
      hasCtrlDep = 1,
      hasExtraSrcRegAllocReq = 1 in {
    defm NAME# : CommonInstr_R_F3X6<opx, outs, ins, mnemonic, [], IIBranch>;
  }
}

multiclass ConditionalBranch<bits<6> opR1, bits<6> opR2, string mnemonic, PatFrag cond_op> {
  let isBranch = 1,
      isTerminator = 1,
      hasDelaySlot = 1,
      Defs = [PC] in {
    defm NAME# :
      CommonInstr_I_F2I16_op<opR1, opR2, (outs),
                             (ins CPURegs:$rA, CPURegs:$rB, brtarget:$imm),
                             !strconcat(mnemonic, "\t$rA, $rB, $imm"),
                             [(brcond (i32 (cond_op CPURegs:$rA, CPURegs:$rB)), bb:$imm)],
                             IIBranch>;
  }
}

multiclass UnconditionalBranch<bits<6> opR1, bits<6> opR2, string mnemonic> {
  let rA = 0, rB = 0,
      isBranch = 1,
      isTerminator = 1,
      isBarrier = 1,
      hasDelaySlot = 1,
      Defs = [PC] in {
    defm NAME# : CommonInstr_I_F2I16_op<opR1, opR2, (outs), (ins brtarget:$imm16),
                                        !strconcat(mnemonic, "\t$imm16"),
                                        [(br bb:$imm16)], IIBranch>;
  }
}

// Pseudo Conditional Branch
class CBranchPseudo<string instr_asm, PatFrag cond_op, RegisterClass RC>:
  Nios2Pseudo<(outs), (ins RC:$rA, RC:$rB, brtarget:$imm16),
             !strconcat(instr_asm, "\t$rA, $rB, $imm16"),
             [(brcond (i32 (cond_op RC:$rA, RC:$rB)), bb:$imm16)], IIBranch> {
  let isBranch = 1;
  let isTerminator = 1;
  let hasDelaySlot = 1;
  let Defs = [PC];
}

// For comparison FI's op and related opx for FR instruction are always the same
multiclass Compare<bits<6> opR1, bits<6> opR2, bits<6> opxR2, string mnemonic, PatFrag cond_op> {
  // Instruction operating on 3 registers:
  defm NAME#  : CommonInstr_R_F3X6_opx<opR1, opxR2, (outs CPURegs:$rC), (ins CPURegs:$rA, CPURegs:$rB),
                                !strconcat(mnemonic, "\t$rC, $rA, $rB"), 
                                [(set CPURegs:$rC, (cond_op CPURegs:$rA, CPURegs:$rB))], IIAlu>;
  // Instruction using 2 register and immediate:
  defm NAME#I : CommonInstr_I_F2I16_op<opR1, opR2, (outs CPURegs:$rB), (ins CPURegs:$rA, simm16:$imm),
                                !strconcat(mnemonic, "i\t$rB, $rA, $imm"), 
                                [(set CPURegs:$rB, (cond_op CPURegs:$rA, immSExt16:$imm))], IIAlu>;
}

multiclass Jump<bits<6> opR1, bits<6> opR2, string menemonic> {
  let isBranch=1,
      isTerminator=1,
      isBarrier=1,
      hasDelaySlot = 1,
      Defs = [PC] in {
    def NAME#_R1 : FJ<opR1, (outs), (ins jmptarget:$target),
                      !strconcat(menemonic, "\t$target"), [(br bb:$target)],
                      IIBranch>;
    def NAME#_R2 : L26<opR2, (ins jmptarget:$target),
                       !strconcat(menemonic, "\t$target"), [(br bb:$target)],
                       IIBranch>;
  }
}

multiclass Call<bits<6> op, string menemonic> {
  let isCall = 1,
      hasDelaySlot = 1,
      Defs = [RA],
      DecoderMethod = "DecodeJumpTarget" in {
    def NAME#_R1 : FJ<op, (outs), (ins calltarget:$target),
                      !strconcat(menemonic, "\t$target"), [(Nios2JmpLink imm:$target)],
                      IIBranch>;
    def NAME#_R2 : L26<op, (ins calltarget:$target),
                       !strconcat(menemonic, "\t$target"), [(Nios2JmpLink imm:$target)],
                       IIBranch>;
  }
}

//===----------------------------------------------------------------------===//
// Specific format for CDX (16-bit) instructions 
//===----------------------------------------------------------------------===//

multiclass CDXArith<bits<1> opx, string baseMnemonic, SDNode op> {
  // operation on registers:
  def NAME#_N_R2 : T3X1<opx, (outs CPURegs:$c3), (ins CPURegs:$a3, CPURegs:$b3),
                        !strconcat(baseMnemonic, ".n\t$c3, $a3, $b3"),
                        [(set CPURegs:$c3, (op CPURegs:$a3, CPURegs:$b3))],
                        IIAlu>;
  // with immediate value:
  def NAME#I_N_R2 : T2X1I3<opx, (outs CPURegs:$b3), (ins CPURegs:$a3, simm16:$imm),
                           !strconcat(baseMnemonic, "i.n\t$b3, $a3, $imm"),
                           [(set CPURegs:$b3, (op CPURegs:$a3, immZExt5:$imm))],
                           IIAlu>;
}

multiclass CDXShift<bits<3> opxR, bits<1> opxI, string baseMnemonic, SDNode op> {
  // operation on registers:
  def NAME#_N_R2 : T2X3<opxR, (outs CPURegs:$a3), (ins CPURegs:$_a3, CPURegs:$b3),
                        !strconcat(baseMnemonic, ".n\t$a3, $a3, $b3"),
                        [(set CPURegs:$a3, (op CPURegs:$_a3, CPURegs:$b3))],
                        IIAlu>;
  // with immediate value:
  def NAME#I_N_R2 : T2X1L3<opxI, (outs CPURegs:$b3), (ins CPURegs:$a3, simm16:$imm),
                           !strconcat(baseMnemonic, "i.n\t$b3, $a3, $imm"),
                           [(set CPURegs:$b3, (op CPURegs:$a3, immZExt5:$imm))],
                           IIAlu>;
}

class CDXLogic<bits<3> opx, string mnemonic, SDNode op>:
      T2X3<opx, (outs CPURegs:$a3), (ins CPURegs:$_a3, CPURegs:$b3),
           !strconcat(mnemonic, "\t$a3, $a3, $b3"),
           [(set CPURegs:$a3, (op CPURegs:$_a3, CPURegs:$b3))],
           IIAlu>;

class CDXLogicSingleOp<bits<3> opx, string mnemonic, PatFrag op>:
      T2X3<opx, (outs CPURegs:$a3), (ins CPURegs:$b3),
           !strconcat(mnemonic, "\t$a3, $b3"),
           [(set CPURegs:$a3, (op CPURegs:$b3))],
           IIAlu>;

class CDXLoad<bits<6> op, string mnemonic, PatFrag oper> :
      T2I4<op, (outs CPURegs:$b3), (ins mem:$a3), 
           !strconcat(mnemonic, "\t$b3, $a3"),
           [(set CPURegs:$b3, (oper addr:$a3))], IILoad>;

class CDXStore<bits<6> op, string mnemonic, PatFrag oper> :
      T2I4<op, (outs), (ins CPURegs:$b3, mem:$a3), 
           !strconcat(mnemonic, "\t$b3, $a3"),
           [(oper CPURegs:$b3, addrCdx:$a3)], IIStore>;

class CDXConditionalBranch<bits<6> op, string mnemonic, PatFrag oper>:
      T1I7<op, (outs), (ins CPURegs:$a3, brtarget:$imm),
           !strconcat(mnemonic, "\t$a3, $imm"), 
           [(brcond (i32 (oper CPURegs:$a3, ZERO)), bb:$imm)],
           IIBranch>;

//===----------------------------------------------------------------------===//
// Pseudo instructions
//===----------------------------------------------------------------------===//

// Return RA.
let isReturn=1, isTerminator=1, hasDelaySlot=1, isBarrier=1, hasCtrlDep=1 in
def RetRA : Nios2Pseudo<(outs), (ins), "", [(Nios2Ret)]>;

let Defs = [SP], Uses = [SP], hasSideEffects = 1 in {
def ADJCALLSTACKDOWN : Nios2Pseudo<(outs), (ins i32imm:$amt),
                                  "!ADJCALLSTACKDOWN $amt",
                                  [(callseq_start timm:$amt)]>;
def ADJCALLSTACKUP   : Nios2Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                  "!ADJCALLSTACKUP $amt1",
                                  [(callseq_end timm:$amt1, timm:$amt2)]>;
}

// The MOVaddr instruction should match only when the add is not folded
// into a load or store address.
def MOVaddr
    : Nios2Pseudo<(outs CPURegs:$dst), (ins i32imm:$hi, i32imm:$low),
             "",
             [(set CPURegs:$dst, (Nios2Low (Nios2Hi tglobaladdr:$hi),
                                            tglobaladdr:$low))]>;

// When handling PIC code the assembler needs .cpload and .cprestore
// directives. If the real instructions corresponding these directives
// are used, we have the same behavior, but get also a bunch of warnings
// from the assembler.
let hasSideEffects = 0 in
def CPRESTORE : Nios2Pseudo<(outs), (ins i32imm:$loc, CPURegs:$gp),
                         ".cprestore\t$loc", []>;

//===----------------------------------------------------------------------===//
// Instruction definition
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Nios2 common R1 and R2 instructions
//===----------------------------------------------------------------------===//

// Arithmetic logic operations:
let isCommutable = 1,
    isReMaterializable = 1 in {
  defm ADD    : ArithLogicReg<0x31, "add",    add,   IIAlu>;
  defm AND    : ArithLogicReg<0x0e, "and",    and,   IIAlu>;
  defm OR     : ArithLogicReg<0x16, "or",     or,    IIAlu>;
  defm XOR    : ArithLogicReg<0x1e, "xor",    xor,   IIAlu>;
  defm MUL    : ArithLogicReg<0x27, "mul",    mul,   IIAlu>;
  defm MULXSS : ArithLogicReg<0x1f, "mulxss", mulhs, IIAlu>;
  defm MULXSU : ArithLogicReg<0x17, "mulxsu", mulhs, IIAlu>;
  defm MULXUU : ArithLogicReg<0x07, "mulxuu", mulhu, IIAlu>;
}

let isReMaterializable = 1 in {
  defm SUB    : ArithLogicReg<0x39, "sub",    sub,   IIAlu>;
  defm DIVU   : ArithLogicReg<0x24, "divu",   udiv,  IIIdiv>;
  defm DIV    : ArithLogicReg<0x25, "div",    sdiv,  IIIdiv>;
}

defm ROR  : ArithLogicReg<0x0b, "ror",  rotr, IIAlu>;
defm ROL  : ArithLogicReg<0x03, "rol",  rotl, IIAlu>;

defm SLL  : ArithLogicReg<0x13, "sll",  shl,  IIAlu>;
defm SRL  : ArithLogicReg<0x1b, "srl",  srl,  IIAlu>;
defm SRA  : ArithLogicReg<0x3b, "sra",  sra,  IIAlu>;

defm ROLI : ArithLogicRegImm5<0x02, "roli", rotl, IIAlu>;
defm SLLI : ArithLogicRegImm5<0x12, "slli", shl,  IIAlu>;
defm SRLI : ArithLogicRegImm5<0x1a, "srli", srl,  IIAlu>;
defm SRAI : ArithLogicRegImm5<0x3a, "srai", sra,  IIAlu>;

defm ADDI  : ArithLogicRegImm16<0x04, "addi",  add, simm16, immSExt16>;
defm MULI  : ArithLogicRegImm16<0x24, "muli",  mul, simm16, immSExt16>;

defm ANDI  : ArithLogicRegImm16<0x0c, "andi",  and, uimm16, immZExt16>;
defm ORI   : ArithLogicRegImm16<0x14, "ori",   or,  uimm16, immZExt16>;
defm XORI  : ArithLogicRegImm16<0x1c, "xori",  xor, uimm16, immZExt16>;

defm ANDHI : ArithLogicRegImm16NoPat<0x2c, "andhi", and, uimm16, immLow16Zero>;
defm ORHI  : ArithLogicRegImm16NoPat<0x34, "orhi",  or,  uimm16, immLow16Zero>;
defm XORHI : ArithLogicRegImm16NoPat<0x3c, "xorhi", xor, uimm16, immLow16Zero>;

// Signed comparisons:
defm CMPEQ  : Compare<0x20, 0x2e, 0x28, "cmpeq",  seteq>;
defm CMPNE  : Compare<0x18, 0x26, 0x20, "cmpne",  setne>;
defm CMPLT  : Compare<0x10, 0x1e, 0x18, "cmplt",  setlt>;
defm CMPGE  : Compare<0x08, 0x16, 0x10, "cmpge",  setge>;
// Unsigned comparisons:
defm CMPLTU : Compare<0x30, 0x3e, 0x38, "cmpltu", setult>;
defm CMPGEU : Compare<0x28, 0x36, 0x30, "cmpgeu", setuge>;
// Conditional branches:
defm BEQ  : ConditionalBranch<0x26, 0x2a, "beq",  seteq>;
defm BNE  : ConditionalBranch<0x1e, 0x22, "bne",  setne>;
defm BGE  : ConditionalBranch<0x0e, 0x12, "bge",  setge>;
defm BGEU : ConditionalBranch<0x2e, 0x32, "bgeu", setuge>;
defm BLT  : ConditionalBranch<0x16, 0x1a, "blt",  setlt>;
defm BLTU : ConditionalBranch<0x36, 0x3a, "bltu", setult>;
// Unconditional branches:
defm BR   : UnconditionalBranch<0x06, 0x02, "br">;
// Returns:
defm RET  : Return<0x05, (outs), (ins CPURegs:$rA),             "ret">;
defm BRET : Return<0x09, (outs CPURegs:$rC), (ins CPURegs:$rA), "bret">;
defm ERET : Return<0x01, (outs CPURegs:$rC), (ins CPURegs:$rA), "eret">;
// Calls and jumps:
defm CALL : Call<0x00, "call">;
defm JMPI : Jump<0x01, 0x08, "jmpi">, Requires<[RelocStatic]>;
// Aligned loads:
defm LDB    : Load<0x07, 0x07, "ldb",  sextloadi8>;
defm LDH    : Load<0x0f, 0x0f, "ldh",  sextloadi16>;
defm LDW    : Load<0x17, 0x17, "ldw",  load>;
defm LDBU   : Load<0x03, 0x06, "ldbu", zextloadi8>;
defm LDHU   : Load<0x0b, 0x0e, "ldhu", zextloadi16>;
defm LDBIO  : LoadImm<0x27, 0x0, "ldbio",  sextloadi8>;
defm LDBUIO : LoadImm<0x23, 0x2, "ldbuio", zextloadi8>;
defm LDHIO  : LoadImm<0x2f, 0x4, "ldhio",  sextloadi16>;
defm LDHUIO : LoadImm<0x2b, 0x6, "ldhuio", zextloadi16>;
defm LDWIO  : LoadImm<0x37, 0x8, "ldwio",  load>;
// Aligned stores:
defm STB   : Store<0x05, 0x27, "stb", truncstorei8>;
defm STH   : Store<0x0d, 0x2f, "sth", truncstorei16>;
defm STW   : Store<0x15, 0x37, "stw", store>;
defm STBIO : StoreImm<0x25, 0x01, "stbio", truncstorei8>;
defm STHIO : StoreImm<0x2d, 0x05, "sthio", truncstorei16>;
defm STWIO : StoreImm<0x35, 0x09, "stwio", store>;
// Memory flush:
defm FLUSHD  : FlushOrInit<0x3b, 0x02, "flushd">;
defm FLUSHDA : FlushOrInit<0x1b, 0x03, "flushda">;
defm INITD   : FlushOrInit<0x33, 0x00, "initd">;
defm INITDA  : FlushOrInit<0x13, 0x01, "initda">;
// Special cases:
let isCommutable = 1 in {
  defm NOR : CommonInstr_R_F3X6<0x06, (outs CPURegs:$rC),
                                (ins CPURegs:$rA, CPURegs:$rB), "nor\t$rC, $rA, $rB",
                                [(set CPURegs:$rC, (not (or CPURegs:$rA, CPURegs:$rB)))],
                                IIAlu>;
}

let rB = 0, rC = 0x1f,
    isCall=1, 
    hasDelaySlot=1,
    Defs = [RA] in {
  defm CALLR : CommonInstr_R_F3X6<0x1d, (outs), (ins CPURegs:$rA), "callr\t$rA",
                                  [(Nios2JmpLink CPURegs:$rA)], IIBranch>;
}

let rB = 0, rC = 0,
    isTerminator=1,
    isBarrier=1,
    hasDelaySlot = 1,
    isBranch = 1,
    isIndirectBranch = 1 in {
  defm JMP : CommonInstr_R_F3X6<0x0d, (outs), (ins CPURegs:$rA), "jmp\t$rA",
                                [(brind CPURegs:$rA)], IIBranch>;
}

let rA = 0, rB = 0, rC = 0 in {
  let hasSideEffects = 1 in {
    defm FLUSHI : CommonInstr_R_F3X6<0x0c, (outs), (ins CPURegs:$rA), "flushi\t$rA", [], IIFlush>;
    defm FLUSHP : CommonInstr_R_F3X6<0x04, (outs), (ins), "flushp", [], IIFlush>;
    defm INITI  : CommonInstr_R_F3X6<0x29, (outs), (ins CPURegs:$rA), "initi\t$rA", [], NoItinerary>;
    defm SYNC   : CommonInstr_R_F3X6<0x36, (outs), (ins), "sync", [], NoItinerary>;
    defm WRCTL  : CommonInstr_R_F3X6<0x2e, (outs), (ins CPURegs:$imm, CPURegs:$rA),
                                     "wrctl\t$imm, $rA", [], NoItinerary>;
    defm BREAK : CommonInstr_R_F2X6L5<0x34, (outs), (ins CPURegs:$imm), "break\t$imm", [], NoItinerary>;
    defm TRAP  : CommonInstr_R_F2X6L5<0x2d, (outs), (ins CPURegs:$imm), "break\t$imm", [], NoItinerary>;
  }
  defm NEXTPC : CommonInstr_R_F3X6<0x1c, (outs CPURegs:$rC), (ins), "nextpc\t$rC", [], NoItinerary>;
  defm WRPRS  : CommonInstr_R_F3X6<0x14, (outs CPURegs:$rC), (ins CPURegs:$rA),
                                   "wrprs\t$rC, i$rA", [], NoItinerary>;
  defm RDCTL  : CommonInstr_R_F3X6_opx<0x26, 0x20, (outs CPURegs:$rC), (ins CPURegs:$imm),
                                       "rdctl\t$rC, $imm", [], NoItinerary>;
} 

def RDPRS_R1 : FI<0x38, (outs CPURegs:$rB), (ins CPURegs:$rA, uimm16:$imm16),
                  "rdprs\t$rB, $rA, $imm16", [], NoItinerary>;
def RDPRS_R2 : F2X4I12<0x07, (outs CPURegs:$rB), (ins CPURegs:$rA, uimm16:$imm16),
                       "rdprs\t$rB, $rA, $imm16", [], NoItinerary>;

//===----------------------------------------------------------------------===//
// Nios2 R2 instructions
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Nios2 R2 mandatory instructions
//===----------------------------------------------------------------------===//

def WRPIE_R2 : F3X6<0x00, (outs CPURegs:$rC), (ins CPURegs:$rA),
                    "wrpie\t$rC, $rA", [], NoItinerary>;
let rC = 0 in {
  def ENI_R2   : F3X6<0x08, (outs), (ins CPURegs:$rA), 
                      "eni\t$rA", [], NoItinerary>;
  def LDEX_R2  : F3X6<0x35, (outs CPURegs:$rC), (ins mem:$rA),
                      "ldex\t$rC, $rA", [], NoItinerary>;
}
def STEX_R2  : F3X6<0x3d, (outs CPURegs:$rC), (ins CPURegs:$rA, mem:$rB),
                    "stex\t$rC, $rA, $rB", [], NoItinerary>;
def LDSEX_R2 : F3X6<0x37, (outs CPURegs:$rC), (ins mem:$rA),
                    "ldsex\t$rC, $rA", [], NoItinerary>;
def STSEX_R2 : F3X6<0x3f, (outs CPURegs:$rC), (ins CPURegs:$rA, mem:$rB),
                    "stsex\t$rC, $rA, $rB", [], NoItinerary>;

//===----------------------------------------------------------------------===//
// Nios2 R2 BMX instructions
//===----------------------------------------------------------------------===//

// TODO: Find patterns for those guys:
def EXTRACT_R2 : F2X6L10<0x33, (outs CPURegs:$rB),
                         (ins CPURegs:$rA, uimm16:$msb, uimm16:$lsb),
                         "extract\t$rB, $rA, $msb, $lsb", [], NoItinerary>;
def INSERT_R2  : F2X6L10<0x23, (outs CPURegs:$rB),
                         (ins CPURegs:$rA, uimm16:$msb, uimm16:$lsb),
                         "insert\t$rB, $rA, $msb, $lsb", [], NoItinerary>;
def MERGE_R2   : F2X6L10<0x2b, (outs CPURegs:$rB), 
                         (ins CPURegs:$rA, uimm16:$msb, uimm16:$lsb),
                         "merge\t$rB, $rA, $msb, $lsb", [], NoItinerary>;

//===----------------------------------------------------------------------===//
// Nios2 R2 CDX (16-bit) instructions
//===----------------------------------------------------------------------===//

// TODO: 3-bit register encoding:
// TODO: imm encoding

defm ADD : CDXArith<0x0, "add", add>;
defm SUB : CDXArith<0x1, "sub", sub>;

defm SLL : CDXShift<0x4, 0x0, "sll", shl>;
defm SRL : CDXShift<0x5, 0x1, "srl", srl>;

def AND_N_R2 : CDXLogic<0x0, "and.n", and>;
def  OR_N_R2 : CDXLogic<0x2, "or.n",  or>;
def XOR_N_R2 : CDXLogic<0x3, "xor.n", xor>;

def NOT_N_R2 : CDXLogicSingleOp<0x6, "not.n", not>;
def NEG_N_R2 : CDXLogicSingleOp<0x7, "neg.n", not>; // TODO: find IR op

def ANDI_N_R2 : T2I4<0x0b, (outs CPURegs:$b3), (ins CPURegs:$a3, simm16:$imm),
                     "andi.n\t$b3, $a3, $imm",
                     [(set CPURegs:$b3, (and CPURegs:$a3, immSExt16:$imm))],
                     IIAlu>;

def LDBU_N_R2 : CDXLoad<0x05, "ldbu.n", zextloadi8>;
def LDHU_N_R2 : CDXLoad<0x0d, "ldhu.n", zextloadi16>;
def LDW_N_R2  : CDXLoad<0x15, "ldw.n",  load>;

def STB_N_R2 : CDXStore<0x25, "stb.n", truncstorei8>;
def STH_N_R2 : CDXStore<0x2d, "sth.n", truncstorei16>;
def STW_N_R2 : CDXStore<0x35, "stw.n", store>;

def LDWSP_N_R2 : F1I5<0x13, (outs CPURegs:$rB), (ins mem:$imm),
                      "ldwsp.n\t$rB, $imm", [], IILoad>;
def STWSP_N_R2 : F1I5<0x33, (outs), (ins mem:$imm), 
                      "stwsp.n\t$imm", [], IIStore>;

def STWZ_N_R2 : T1X1I6<0x0, (outs), (ins mem:$a3), "stwz.n\t$a3", [], IIStore>;
def STBZ_N_R2 : T1X1I6<0x1, (outs), (ins mem:$a3), "stbz.n\t$a3", [], IIStore>;

def BR_N_R2 : I10<0x03, (ins brtarget:$imm), "br.n\t$imm", [(br bb:$imm)], IIBranch>;

def BREAK_N_R2 : X2L5<0x0, (outs), (ins simm16:$lit), "break.n\t$lit", [], NoItinerary>;
def TRAP_N_R2  : X2L5<0x1, (outs), (ins simm16:$lit), "trap\t$lit", [], NoItinerary>;
def RET_N_R2   : X2L5<0x2, (outs), (ins), "ret.n", [], NoItinerary>;

def CALLR_N_R2 : F1X1<0x0, (outs), (ins CPURegs:$rA), "callr.n\t$rA", 
                      [(Nios2JmpLink CPURegs:$rA)], IIBranch>;
def JMPR_N_R2  : F1X1<0x1, (outs), (ins brtarget:$rA), "jmpr.n\t$rA",
                      [(br bb:$rA)], IIBranch>;

def MOV_N_R2  :   F2<0x3b, (outs CPURegs:$rB), (ins CPURegs:$rA), 
                     "mov.n\t$rB, $rA", [], NoItinerary>;
def MOVI_N_R2 : T1I7<0x1b, (outs CPURegs:$a3), (ins simm16:$imm),
                     "movi.n\t$a3, $imm", [], NoItinerary>;

def BEQZ_N_R2 : CDXConditionalBranch<0x2b, "beqz.n", seteq>;
def BNEZ_N_R2 : CDXConditionalBranch<0x23, "bnez.n", setne>;

def POP_N_R2  : L5I4X1<0x0, (outs), (ins simm16:$reg, simm16:$imm),
                       "pop.n\t$reg, $imm", [], NoItinerary>;
def PUSH_N_R2 : L5I4X1<0x1, (outs), (ins simm16:$reg, simm16:$imm),
                       "push.n\t$reg, $imm", [], NoItinerary>;

def SPADDI_N_R2 : T1I7<0x3d, (outs CPURegs:$a3), (ins simm16:$imm),
                       "spaddi.n\t$a3, $imm", [], IIAlu>;
def SPDECI_N_R2 : X1I7<0x1, (outs), (ins simm16:$imm), "spdeci\t$imm", [], IIAlu>;
def SPINCI_N_R2 : X1I7<0x0, (outs), (ins simm16:$imm), "spinci\t$imm", [], IIAlu>;

//===----------------------------------------------------------------------===//
// Pseudo instructions
//===----------------------------------------------------------------------===//

def NOP : Nios2Pseudo<(outs), (ins), "nop", []>;

//===----------------------------------------------------------------------===//
// Instruction aliases
//===----------------------------------------------------------------------===//

// This multiclass will definie given alias for both R1 and R2 instructions.
// It expects result pattern for R1 and will create R2 pattern by substition of
// R1 instruction for R2 instruction. The instrictions for substituion are
// given as multiclass arguments.
multiclass CommonAlias<string asm, dag result, Instruction r1, Instruction r2> {
   let Predicates = [isNios2r1] in def : InstAlias<asm, result>;
   let Predicates = [isNios2r2] in def : InstAlias<asm, !subst(r1, r2, result)>;
}

defm : CommonAlias<"mov $rC, $rA", (ADD_R1 CPURegs:$rC, CPURegs:$rA, ZERO),
                   ADD_R1, ADD_R2>;
defm : CommonAlias<"nop", (ADD_R1 ZERO, ZERO, ZERO),
                   ADD_R1, ADD_R2>;
defm : CommonAlias<"movhi $rB, $imm",(ORHI_R1 CPURegs:$rB, ZERO, simm16:$imm),
                   ORHI_R1, ORHI_R2>;
defm : CommonAlias<"movi $rB, $imm", (ADDI_R1 CPURegs:$rB, ZERO, simm16:$imm),
                   ADDI_R1, ADDI_R2>;
//defm : CommonAlias<"movia $rB, $imm", (ADDI_R1 CPURegs:$rB, ZERO, (LO16 simm16:$imm)),
//                   ADDI_R1, ADDI_R2>;
defm : CommonAlias<"movui $rB, $imm", (ORI_R1 CPURegs:$rB, ZERO, simm16:$imm),
                   ORI_R1, ORI_R2>;

defm : CommonAlias<"bgt $rA,$rB,$imm",
                   (BLT_R1 CPURegs:$rB, CPURegs:$rA, brtarget:$imm),
                   BLT_R1, BLT_R2>;
defm : CommonAlias<"bgtu $rA,$rB,$imm",
                   (BLTU_R1 CPURegs:$rB, CPURegs:$rA, brtarget:$imm),
                   BLTU_R1, BLTU_R2>;
defm : CommonAlias<"ble $rA,$rB,$imm",
                   (BGE_R1 CPURegs:$rB, CPURegs:$rA, brtarget:$imm),
                   BGE_R1, BGE_R2>;
defm : CommonAlias<"bleu $rA,$rB,$imm",
                   (BGEU_R1 CPURegs:$rB, CPURegs:$rA, brtarget:$imm),
                   BGEU_R1, BGEU_R2>;
//defm : CommonAlias<"subi $rB,$rA,$imm",
//                   (ADDI_R1 CPURegs:$rB, CPURegs:$rA, (NEG16 simm16:$imm)),
//                   ADDI_R1, ADDI_R2>;

// Nios2 R2 aliases:
def : InstAlias<"nop.n", (MOV_N_R2 ZERO, ZERO)>;
  
//===----------------------------------------------------------------------===//
//  Arbitrary patterns that map to one or more instructions
//===----------------------------------------------------------------------===//

// This multiclass will definie given pattern for both R1 and R2 instructions.
// It expects result pattern for R1 and will create R2 pattern by substition of
// R1 instruction for R2 instruction. The instrictions for substituion are
// given as multiclass arguments.
multiclass CommonPat<dag pattern, dag result, Instruction r1, Instruction r2> {
   let Predicates = [isNios2r1] in def : Pat<pattern, result>;
   let Predicates = [isNios2r2] in def : Pat<pattern, !subst(r1, r2, result)>;
}

// Small immediates
defm : CommonPat<(i32 immSExt16:$in),    (ADDI_R1 ZERO, imm:$in), ADDI_R1, ADDI_R2>;
defm : CommonPat<(i32 immZExt16:$in),    (ORI_R1  ZERO, imm:$in), ORI_R1,  ORI_R2>;

// Logic operations on high bits
defm : CommonPat<(and CPURegs:$rA, immLow16Zero:$in),
                 (ANDHI_R1 CPURegs:$rA, (HI16 imm:$in)),
                 ANDHI_R1, ANDHI_R2>;
defm : CommonPat<(or CPURegs:$rA, immLow16Zero:$in),
                 (ORHI_R1 CPURegs:$rA, (HI16 imm:$in)),
                 ORHI_R1, ORHI_R2>;
defm : CommonPat<(xor CPURegs:$rA, immLow16Zero:$in),
                 (XORHI_R1 CPURegs:$rA, (HI16 imm:$in)),
                 XORHI_R1, XORHI_R2>;

// Arbitrary immediates
defm : CommonPat<(i32 imm:$imm),
                 (ORI_R1 (ORHI_R1 ZERO, (HI16 imm:$imm)), (LO16 imm:$imm)),
                 ORI_R1, ORI_R2>;

// Carry patterns
defm : CommonPat<(add CPURegs:$src, immSExt16:$imm), (ADDI_R1 CPURegs:$src, imm:$imm),
                  ADDI_R1, ADDI_R2>;
defm : CommonPat<(sub CPURegs:$lhs, immSExt16:$imm), (ADDI_R1 CPURegs:$lhs, (NEG16 imm:$imm)),
                  ADDI_R1, ADDI_R2>;
// hi/lo relocs
defm : CommonPat<(Nios2Hi tglobaladdr:$in),    (ORHI_R1 ZERO, tglobaladdr:$in),    ORHI_R1, ORHI_R2>;
defm : CommonPat<(Nios2Hi tblockaddress:$in),  (ORHI_R1 ZERO, tblockaddress:$in),  ORHI_R1, ORHI_R2>;
defm : CommonPat<(Nios2Hi tjumptable:$in),     (ORHI_R1 ZERO, tjumptable:$in),     ORHI_R1, ORHI_R2>;
defm : CommonPat<(Nios2Hi tconstpool:$in),     (ORHI_R1 ZERO, tconstpool:$in),     ORHI_R1, ORHI_R2>;
defm : CommonPat<(Nios2Hi tglobaltlsaddr:$in), (ORHI_R1 ZERO, tglobaltlsaddr:$in), ORHI_R1, ORHI_R2>;

defm : CommonPat<(Nios2Lo tglobaladdr:$in),    (ORI_R1 ZERO, tglobaladdr:$in),    ORI_R1, ORI_R2>;
defm : CommonPat<(Nios2Lo tblockaddress:$in),  (ORI_R1 ZERO, tblockaddress:$in),  ORI_R1, ORI_R2>;
defm : CommonPat<(Nios2Lo tjumptable:$in),     (ORI_R1 ZERO, tjumptable:$in),     ORI_R1, ORI_R2>;
defm : CommonPat<(Nios2Lo tconstpool:$in),     (ORI_R1 ZERO, tconstpool:$in),     ORI_R1, ORI_R2>;
defm : CommonPat<(Nios2Lo tglobaltlsaddr:$in), (ORI_R1 ZERO, tglobaltlsaddr:$in), ORI_R1, ORI_R2>;

defm : CommonPat<(add CPURegs:$hi, (Nios2Lo tglobaladdr:$lo)),
                 (ADDI_R1 CPURegs:$hi, tglobaladdr:$lo), ADDI_R1, ADDI_R2>;
defm : CommonPat<(add CPURegs:$hi, (Nios2Lo tblockaddress:$lo)),
                 (ADDI_R1 CPURegs:$hi, tblockaddress:$lo), ADDI_R1, ADDI_R2>;
defm : CommonPat<(add CPURegs:$hi, (Nios2Lo tjumptable:$lo)),
                 (ADDI_R1 CPURegs:$hi, tjumptable:$lo), ADDI_R1, ADDI_R2>;
defm : CommonPat<(add CPURegs:$hi, (Nios2Lo tconstpool:$lo)),
                 (ADDI_R1 CPURegs:$hi, tconstpool:$lo), ADDI_R1, ADDI_R2>;
defm : CommonPat<(add CPURegs:$hi, (Nios2Lo tglobaltlsaddr:$lo)),
                 (ADDI_R1 CPURegs:$hi, tglobaltlsaddr:$lo), ADDI_R1, ADDI_R2>;
// gp_rel relocs
defm : CommonPat<(add CPURegs:$gp, (Nios2GPRel tglobaladdr:$in)),
                 (ADDI_R1 CPURegs:$gp, tglobaladdr:$in), ADDI_R1, ADDI_R2>;
defm : CommonPat<(add CPURegs:$gp, (Nios2GPRel tconstpool:$in)),
                 (ADDI_R1 CPURegs:$gp, tconstpool:$in), ADDI_R1, ADDI_R2>;

defm : CommonPat<(Nios2JmpLink (i32 tglobaladdr:$dst)),
                 (CALL_R1 tglobaladdr:$dst), CALL_R1, CALL_R2>;
defm : CommonPat<(Nios2JmpLink (i32 texternalsym:$dst)),
                 (CALL_R2 texternalsym:$dst), CALL_R1, CALL_R2>;

// wrapper_pic
class WrapperPat<SDNode node, Instruction ADDiOp>:
      Pat<(Nios2Wrapper CPURegs:$gp, node:$in),
              (ADDiOp CPURegs:$gp, node:$in)>;

multiclass CommonWrapperPat<SDNode node, Instruction r1, Instruction r2> {
  let Predicates = [isNios2r1] in def : WrapperPat<node, r1>;
  let Predicates = [isNios2r2] in def : WrapperPat<node, r2>;
}

defm : CommonWrapperPat<tglobaladdr,    ADDI_R1, ADDI_R2>;
defm : CommonWrapperPat<tconstpool,     ADDI_R1, ADDI_R2>;
defm : CommonWrapperPat<texternalsym,   ADDI_R1, ADDI_R2>;
defm : CommonWrapperPat<tblockaddress,  ADDI_R1, ADDI_R2>;
defm : CommonWrapperPat<tjumptable,     ADDI_R1, ADDI_R2>;
defm : CommonWrapperPat<tglobaltlsaddr, ADDI_R1, ADDI_R2>;

defm : CommonPat<(not CPURegs:$in), (NOR_R1 CPURegs:$in, ZERO), NOR_R1, NOR_R2>;

// extended loads
defm : CommonPat<(i32 (extloadi1  addr:$src)), (LDBU_R1 addr:$src), LDBU_R1, LDBU_R2>;
defm : CommonPat<(i32 (extloadi8  addr:$src)), (LDBU_R1 addr:$src), LDBU_R1, LDBU_R2>;
defm : CommonPat<(i32 (extloadi16 addr:$src)), (LDHU_R1 addr:$src), LDHU_R1, LDHU_R2>;
defm : CommonPat<(i32 (extloadi1  addr:$src)), (LDBU_R1 addr:$src), LDBU_R1, LDBU_R2>;
defm : CommonPat<(i32 (extloadi8  addr:$src)), (LDBU_R1 addr:$src), LDBU_R1, LDBU_R2>;
defm : CommonPat<(i32 (extloadi16 addr:$src)), (LDHU_R1 addr:$src), LDHU_R1, LDHU_R2>;

defm : CommonPat<(store (i32 0), addr:$dst), (STW_R1 ZERO, addr:$dst), STW_R1, STW_R2>;

defm : CommonPat<(brcond (i32 (setgt CPURegs:$rA, CPURegs:$rB)), bb:$imm),
                 (BLT_R1 CPURegs:$rB, CPURegs:$rA, bb:$imm),
                 BLT_R1, BLT_R2>;
defm : CommonPat<(brcond (i32 (setugt CPURegs:$rA, CPURegs:$rB)), bb:$imm),
                 (BLTU_R1 CPURegs:$rB, CPURegs:$rA, bb:$imm),
                 BLTU_R1, BLTU_R2>;
defm : CommonPat<(brcond (i32 (setle CPURegs:$rA, CPURegs:$rB)), bb:$imm),
                 (BGE_R1 CPURegs:$rB, CPURegs:$rA, bb:$imm),
                 BGE_R1, BGE_R2>;
defm : CommonPat<(brcond (i32 (setule CPURegs:$rA, CPURegs:$rB)), bb:$imm),
                 (BGEU_R1 CPURegs:$rB, CPURegs:$rA, bb:$imm),
                 BGEU_R1, BGEU_R2>;

//(brcond (i32 (cond_op CPURegs:$rA, CPURegs:$rB)), bb:$imm)
//def BLE  : CBranchPseudo<"ble", setle, CPURegs>;
//def BLEU : CBranchPseudo<"bleu", setule, CPURegs>;

// brcond patterns
multiclass BrcondPats<RegisterClass RC, Instruction BEQOp, Instruction BNEOp,
                      Instruction SLTOp, Instruction SLTuOp, Instruction SLTiOp,
                      Instruction SLTiuOp, Register ZEROReg> {
  def : Pat<(brcond (i32 (setne RC:$lhs, 0)), bb:$dst), (BNEOp RC:$lhs, ZEROReg, bb:$dst)>;
  def : Pat<(brcond (i32 (seteq RC:$lhs, 0)), bb:$dst), (BEQOp RC:$lhs, ZEROReg, bb:$dst)>;
  
  def : Pat<(brcond (i32 (setge RC:$lhs, RC:$rhs)), bb:$dst),
           (BEQOp (SLTOp RC:$lhs, RC:$rhs), ZERO, bb:$dst)>;
  def : Pat<(brcond (i32 (setuge RC:$lhs, RC:$rhs)), bb:$dst),
           (BEQOp (SLTuOp RC:$lhs, RC:$rhs), ZERO, bb:$dst)>;
  def : Pat<(brcond (i32 (setge RC:$lhs, immSExt16:$rhs)), bb:$dst),
           (BEQOp (SLTiOp RC:$lhs, immSExt16:$rhs), ZERO, bb:$dst)>;
  def : Pat<(brcond (i32 (setuge RC:$lhs, immSExt16:$rhs)), bb:$dst),
           (BEQOp (SLTiuOp RC:$lhs, immSExt16:$rhs), ZERO, bb:$dst)>;
  
  def : Pat<(brcond (i32 (setle RC:$lhs, RC:$rhs)), bb:$dst),
           (BEQOp (SLTOp RC:$rhs, RC:$lhs), ZERO, bb:$dst)>;
  def : Pat<(brcond (i32 (setule RC:$lhs, RC:$rhs)), bb:$dst),
           (BEQOp (SLTuOp RC:$rhs, RC:$lhs), ZERO, bb:$dst)>;
  
  def : Pat<(brcond RC:$cond, bb:$dst), (BNEOp RC:$cond, ZEROReg, bb:$dst)>;
}

defm : BrcondPats<CPURegs, BEQ_R1, BNE_R2, CMPLT_R1, CMPLTU_R1, CMPLTI_R1, CMPLTUI_R1, ZERO>;
defm : BrcondPats<CPURegs, BEQ_R1, BNE_R2, CMPLT_R2, CMPLTU_R2, CMPLTI_R2, CMPLTUI_R2, ZERO>;

/// Use existent compares with imm + 1
multiclass SetCmpImmPats<RegisterClass RC, Instruction cmpi, Instruction cmpui> {
  def i : Pat<(setgt RC:$lhs, immSExt16:$rhs), (cmpi RC:$lhs, (IMMPLUS1 imm:$rhs))>;
  def ui : Pat<(setugt RC:$lhs, immSExt16:$rhs), (cmpui RC:$lhs, (IMMPLUS1 imm:$rhs))>;
}

/// Swapped rhs and lhs to match existent compares
multiclass SetCmpPats<RegisterClass RC, PatFrag setop, PatFrag setopu,
           Instruction cmp, Instruction cmpu,
           Instruction cmpi, Instruction cmpui>:
  SetCmpImmPats<RC, cmpi, cmpui> {
  def #NAME# : Pat<(setop RC:$lhs, RC:$rhs),
                (cmp RC:$rhs, RC:$lhs)>;
  def u : Pat<(setopu RC:$lhs, RC:$rhs),
                (cmpu RC:$rhs, RC:$lhs)>;
}

defm CMPGT_R1 : SetCmpPats<CPURegs, setgt, setugt, CMPLT_R1, CMPLTU_R1, CMPGEI_R1, CMPGEUI_R1>;
defm CMPLE_R1 : SetCmpPats<CPURegs, setle, setule, CMPGE_R1, CMPGEU_R1, CMPLTI_R1, CMPLTUI_R1>;
defm CMPGT_R2 : SetCmpPats<CPURegs, setgt, setugt, CMPLT_R2, CMPLTU_R2, CMPGEI_R2, CMPGEUI_R2>;
defm CMPLE_R2 : SetCmpPats<CPURegs, setle, setule, CMPGE_R2, CMPGEU_R2, CMPLTI_R2, CMPLTUI_R2>;

// Generic select instruction
let usesCustomInserter = 1 in
def SELECT   : Nios2Pseudo<(outs CPURegs:$res), (ins CPURegs:$a, CPURegs:$x, CPURegs:$y),
                                  "!select $res, $a, $x, $y",
                                  [(set CPURegs:$res, (Nios2Select CPURegs:$a, CPURegs:$x, CPURegs:$y))]>;

let usesCustomInserter = 1 in
def MOVFI : Nios2Pseudo<(outs CPURegs:$rA), (ins mem:$addr), "movfi",
    [(set CPURegs:$rA, addr:$addr)]>;

//===----------------------------------------------------------------------===//
//  Intrinsics
//===----------------------------------------------------------------------===//

defm : CommonPat<(int_nios2_sync), (SYNC_R1), SYNC_R1, SYNC_R2>;
defm : CommonPat<(int_nios2_wrctl  i32:$rCtl, i32:$rA), 
                 (WRCTL_R1 i32:$rCtl, i32:$rA), WRCTL_R1, WRCTL_R2>;
defm : CommonPat<(int_nios2_rdctl  i32:$a),
                 (RDCTL_R1 i32:$a), RDCTL_R1, RDCTL_R2>;
defm : CommonPat<(int_nios2_ldbio  addr:$a),
                 (LDBIO_R1  addr:$a), LDBIO_R1, LDBIO_R2>;
defm : CommonPat<(int_nios2_ldbuio addr:$a),
                 (LDBUIO_R1 addr:$a), LDBUIO_R1, LDBUIO_R2>;
defm : CommonPat<(int_nios2_ldhio  addr:$a),
                 (LDHIO_R1  addr:$a), LDHIO_R1, LDHIO_R2>;
defm : CommonPat<(int_nios2_ldhuio addr:$a),
                 (LDHUIO_R1 addr:$a), LDHUIO_R1, LDHUIO_R2>;
defm : CommonPat<(int_nios2_ldwio  addr:$a),
                 (LDWIO_R1  addr:$a), LDWIO_R1, LDWIO_R2>;
defm : CommonPat<(int_nios2_ldwuio addr:$a),
                 (LDWIO_R1 addr:$a), LDWIO_R1, LDWIO_R2>;
defm : CommonPat<(int_nios2_flushd addr:$a),
                 (FLUSHD_R1 addr:$a), FLUSHD_R1, FLUSHD_R2>;
defm : CommonPat<(int_nios2_flushda addr:$a),
                 (FLUSHDA_R1 addr:$a), FLUSHDA_R1, FLUSHDA_R2>;
defm : CommonPat<(int_nios2_stbio addr:$a, i32:$v),
                 (STBIO_R1 i32:$v, addr:$a), STBIO_R1, STBIO_R2>;
defm : CommonPat<(int_nios2_sthio addr:$a, i32:$v),
                 (STHIO_R1 i32:$v, addr:$a), STHIO_R1, STHIO_R2>;
defm : CommonPat<(int_nios2_stwio addr:$a, i32:$v),
                 (STWIO_R1 i32:$v, addr:$a), STWIO_R1, STWIO_R2>;
defm : CommonPat<(int_nios2_rdprs  i32:$rCtl, i32:$rA),
                 (RDPRS_R1 i32:$rCtl, i32:$rA), RDPRS_R1, RDPRS_R2>;

/// R2 intrinsics:
let Predicates = [isNios2r2] in {
  def : Pat<(int_nios2_wrpie i32:$rCtl), (WRPIE_R2 i32:$rCtl)>;
  def : Pat<(int_nios2_eni   i32:$rCtl), (ENI_R2 i32:$rCtl)>;
  def : Pat<(int_nios2_ldex  i32:$a),         (LDEX_R2  mem:$a)>;
  def : Pat<(int_nios2_stex  i32:$a, i32:$v), (STEX_R2  i32:$a, mem:$v)>;
  def : Pat<(int_nios2_ldsex i32:$a),         (LDSEX_R2 mem:$a)>;
  def : Pat<(int_nios2_stsex i32:$a, i32:$v), (STSEX_R2 i32:$a, mem:$v)>;
}

