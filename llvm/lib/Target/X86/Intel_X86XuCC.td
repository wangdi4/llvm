#if INTEL_FEATURE_XUCC
//===---------------------*- tablegen -*-----------------------------------===//
//=--- Intel_X86XuCC.td - XuCC Instruction Set Extension -*- tablegen ---*=//
//
//      Copyright (c) 2018-2022 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
//===----------------------------------------------------------------------===//
//
// This file describes the instructions that make up the Intel XuCC instruction
// set.
//
//===----------------------------------------------------------------------===//

multiclass XuCCBase<bits<8> Opcode, string OpcodeStr> {
    def 16rm : I<Opcode, MRMSrcMem, (outs GR16:$dst), (ins i16mem:$src),
                 OpcodeStr#"\t{$src, $dst|$dst, $src}", []>, XS, OpSize16;
    def 32rm : I<Opcode, MRMSrcMem, (outs GR32:$dst), (ins i32mem:$src),
                 OpcodeStr#"\t{$src, $dst|$dst, $src}", []>, XS, OpSize32;
    def 64rm : I<Opcode, MRMSrcMem, (outs GR64:$dst), (ins i64mem:$src),
                 OpcodeStr#"\t{$src, $dst|$dst, $src}", []>, XS, REX_W;
}

multiclass GMOVPPHYS_BASE {
    def 16mr : I<0xb4, MRMDestMem, (outs i16mem:$dst), (ins GR16:$src),
                 "gmovpphys\t{$src, $dst|$dst, $src}", []>, XD, OpSize16;
    def 32mr : I<0xb4, MRMDestMem, (outs i32mem:$dst), (ins GR32:$src),
                 "gmovpphys\t{$src, $dst|$dst, $src}", []>, XD, OpSize32;
    def 64mr : I<0xb4, MRMDestMem, (outs i64mem:$dst), (ins GR64:$src),
                 "gmovpphys\t{$src, $dst|$dst, $src}", []>, XD, REX_W;
}

class XuCC_OneMem64<bits<8> Opcode, string OpcodeStr, Format F> :
      I<Opcode, F, (outs),
       (ins i64mem:$src), OpcodeStr#"\t{$src|$src}",[]>;

let Predicates = [InXuCCMode] in {
// XED unsupports.
let Defs = [RAX] in
  def ACQUIREEPOCHrm : I<0x00, MRM2m, (outs), (ins i128mem:$src),
                         "acquireepoch\t{$src, %rax|rax, $src}", []>, TB;

// FIXME: XED supports (ASIDSWITCH_TLBFLUSH REG/MEM).
// FIXME: Is MRMSrcREG/MRMSrcMem is right?
def ASIDSWITCH_TLBFLUSHrm : I<0x03, MRMSrcMem, (outs GR64:$dst), (ins i64mem:$src),
                              "asidswitch_tlbflush\t{$src, $dst|$dst, $src}", []>, TB, XuCCPD;
def ASIDSWITCH_TLBFLUSHrr : I<0x03, MRMSrcReg, (outs GR64:$dst), (ins GR64:$src),
                              "asidswitch_tlbflush\t{$src, $dst|$dst, $src}", []>, TB, XuCCPD;

let Defs = [RCX] in {
  def CMODEMOVrr : I<0x03, MRMSrcReg, (outs GR64:$dst), (ins GR64:$src),
                    "cmodemov\t{%rcx, $src, $dst|$dst, $src, rcx}", []>, TB;
  def CMODEMOVrm : I<0x03, MRMSrcMem, (outs GR64:$dst), (ins i64mem:$src),
                    "cmodemov\t{%rcx, $src, $dst|$dst, $src, rcx}", []>, TB;
}

def GETBASEKEYr : I<0x00, MRM4r,(outs), (ins GR64:$src),
                    "getbasekey\t{$src|$src}", []>, TB;

defm GMOVLIN : XuCCBase<0x02, "gmovlin">;
defm GMOVPPHYS : XuCCBase<0xb4, "gmovpphys">;

defm GMOVPPHYS : GMOVPPHYS_BASE;

def GTRANSLATERD_NOEPC : XuCC_OneMem64<0x00, "gtranslaterd_noepc", MRM5m>, XS;
def GTRANSLATEWR_NOEPC : XuCC_OneMem64<0x00, "gtranslatewr_noepc", MRM5m>, XD;

def GTRANSLATERD_EPC : XuCC_OneMem64<0x00, "gtranslaterd_epc", MRM4m>, XS;
def GTRANSLATEWR_EPC : XuCC_OneMem64<0x00, "gtranslatewr_epc", MRM4m>, XD;

def LOADSEGm : I<0xb2, MRM0m, (outs), (ins i128mem:$mem), "loadseg\t$mem", []>, TB;
def STORESEGm : I<0xb4, MRM0m, (outs), (ins i128mem:$mem), "storeseg\t$mem", []>, TB;

// XED unsupports
def GTRANSLATERD_TIOPRMm : I<0x00, MRM5m, (outs), (ins i64mem:$mem),
                            "gtranslaterd_tioprm\t$mem", []>, XS, XuCCPD;
// XED unsupports
def GTRANSLATEWR_TIOPRMm : I<0x00, MRM5m, (outs), (ins i64mem:$mem),
                            "gtranslatewr_tioprm\t$mem", []>, XD, XuCCPD;

// The encoding of XED: F2 66
def SPBUSMSGrr : I<0x03, MRMDestReg, (outs GR64:$dst), (ins GR64:$src),
                   "spbusmsg\t{$src, $dst|$dst, $src}", []>, XD, XuCCPD;
def SPBUSMSGrm : I<0x03, MRMSrcMem, (outs GR64:$dst), (ins i64mem:$src),
                   "spbusmsg\t{$src, $dst|$dst, $src}", []>, XD, XuCCPD;

def RSWORLDmr : I<0xb5, MRMDestMem, (outs i32mem:$dst), (ins GR64:$src),
                 "rsworld\t{$src, $dst|$dst, $src}", []>, PD, XuCCXS;

def SVWORLDmr : I<0xb5, MRMDestMem, (outs i32mem:$dst), (ins GR64:$src),
                  "svworld\t{$src, $dst|$dst, $src}", []>, PD, XuCCXD;
}
#endif // INTEL_FEATURE_XUCC
