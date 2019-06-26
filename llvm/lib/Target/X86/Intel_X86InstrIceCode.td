// INTEL_FEATURE_ICECODE //===------------*- tablegen -*-------------------===//
//===- Intel_X86InstrIceCode.td - IceCode Instruction Set ===*- tablegen -*===//
//
//      Copyright (c) 2019 Intel Corporation.
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
// This file describes the instructions that make up the Intel IceCode
// instruction set.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// IceCode instructions
let Predicates = [InIceCodeMode] in {
  def LOADSEGrm : I<0xb2, MRM0m, (outs), (ins i128mem:$mem),
                    "loadseg\t$mem", []>, TB;
  def STORESEGmr : I<0xb4, MRM0m, (outs), (ins i128mem:$mem),
                     "storeseg\t$mem", []>, TB;

  def GMOVPPHYS16rm : I<0xb4, MRMSrcMem, (outs GR16:$dst), (ins opaquemem:$src),
                        "gmovpphys{w}\t{$src, $dst|$dst, $src}",
                        [(set GR16:$dst, (int_x86_icecode_loadpphys_16 addr:$src))]>, XS, OpSize16;
  def GMOVPPHYS32rm : I<0xb4, MRMSrcMem, (outs GR32:$dst), (ins opaquemem:$src),
                        "gmovpphys{l}\t{$src, $dst|$dst, $src}",
                        [(set GR32:$dst, (int_x86_icecode_loadpphys_32 addr:$src))]>, XS, OpSize32;
  def GMOVPPHYS64rm : RI<0xb4, MRMSrcMem, (outs GR64:$dst), (ins opaquemem:$src),
                         "gmovpphys{q}\t{$src, $dst|$dst, $src}",
                         [(set GR64:$dst, (int_x86_icecode_loadpphys_64 addr:$src))]>, XS;

  def GMOVPPHYS16mr : I<0xb4, MRMDestMem, (outs), (ins opaquemem:$dst, GR16:$src),
                        "gmovpphys{w}\t{$src, $dst|$dst, $src}",
                        [(int_x86_icecode_storepphys_16 GR16:$src, addr:$dst)]>, XD, OpSize16;
  def GMOVPPHYS32mr : I<0xb4, MRMDestMem, (outs), (ins opaquemem:$dst, GR32:$src),
                        "gmovpphys{l}\t{$src, $dst|$dst, $src}",
                        [(int_x86_icecode_storepphys_32 GR32:$src, addr:$dst)]>, XD, OpSize32;
  def GMOVPPHYS64mr : RI<0xb4, MRMDestMem, (outs), (ins opaquemem:$dst, GR64:$src),
                         "gmovpphys{q}\t{$src, $dst|$dst, $src}",
                         [(int_x86_icecode_storepphys_64 GR64:$src, addr:$dst)]>, XD;

  let Defs = [RAX, RCX, RDX, RBX] in {
    def GTRANSLATERD_EPCm : I<0x00, MRM4m, (outs), (ins anymem:$mem),
                              "gtranslaterd_epc\t$mem", []>, XS, NotMemoryFoldable;
    def GTRANSLATEWR_EPCm : I<0x00, MRM4m, (outs), (ins anymem:$mem),
                              "gtranslatewr_epc\t$mem", []>, XD, NotMemoryFoldable;
    def GTRANSLATERD_NOEPCm : I<0x00, MRM5m, (outs), (ins anymem:$mem),
                                "gtranslaterd_noepc\t$mem", []>, XS, NotMemoryFoldable;
    def GTRANSLATEWR_NOEPCm : I<0x00, MRM5m, (outs), (ins anymem:$mem),
                                "gtranslatewr_noepc\t$mem", []>, XD, NotMemoryFoldable;
  }

  def BCAST_SEG_STATE : I<0x01, MRM_EF, (outs), (ins), "bcast_seg_state", []>, XS;
}
// end INTEL_FEATURE_ICECODE
