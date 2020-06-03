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
                    "loadseg\t$mem", []>, PS;
  def STORESEGmr : I<0xb2, MRM0m, (outs), (ins i128mem:$mem),
                     "storeseg\t$mem", []>, PD;

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

  let OpPrefix = XS, OpMap = OB in {
    let Constraints = "$src1 = $dst" in {
      def CREG_XCHG32r : I<0x01, MRMDestReg, (outs GR32:$dst), (ins GR32:$src1, GR32:$src),
                           "creg_xchg{l}\t{$src, $dst|$dst, $src}",
                           [(set GR32:$dst, (int_x86_icecode_creg_xchg_32 GR32:$src, GR32:$src1))]>;
      def CREG_XCHG32i : Ii32<0x81, MRM0r, (outs GR32:$dst), (ins GR32:$src1, i32u32imm:$src),
                              "creg_xchg{l}\t{$src, $dst|$dst, $src}",
                              [(set GR32:$dst, (int_x86_icecode_creg_xchg_32 imm:$src, GR32:$src1))]>;

      def CREG_XCHG64r : RI<0x01, MRMDestReg, (outs GR64:$dst), (ins GR64:$src1, GR32:$src),
                            "creg_xchg{q}\t{$src, $dst|$dst, $src}",
                            [(set GR64:$dst, (int_x86_icecode_creg_xchg_64 GR32:$src, GR64:$src1))]>;
      def CREG_XCHG64i : RIi32<0x81, MRM0r, (outs GR64:$dst), (ins GR64:$src1, i32u32imm:$src),
                               "creg_xchg{q}\t{$src, $dst|$dst, $src}",
                               [(set GR64:$dst, (int_x86_icecode_creg_xchg_64 imm:$src, GR64:$src1))]>;

      def FSCP_XCHG32r : I<0x21, MRMDestReg, (outs GR32:$dst), (ins GR32:$src1, GR32:$src),
                           "fscp_xchg{l}\t{$src, $dst|$dst, $src}",
                           [(set GR32:$dst, (int_x86_icecode_fscp_xchg_32 GR32:$src, GR32:$src1))]>;
      def FSCP_XCHG32i : Ii32<0x81, MRM4r, (outs GR32:$dst), (ins GR32:$src1, i32u32imm:$src),
                              "fscp_xchg{l}\t{$src, $dst|$dst, $src}",
                              [(set GR32:$dst, (int_x86_icecode_fscp_xchg_32 imm:$src, GR32:$src1))]>;

      def FSCP_XCHG64r : RI<0x21, MRMDestReg, (outs GR64:$dst), (ins GR64:$src1, GR32:$src),
                            "fscp_xchg{q}\t{$src, $dst|$dst, $src}",
                            [(set GR64:$dst, (int_x86_icecode_fscp_xchg_64 GR32:$src, GR64:$src1))]>;
      def FSCP_XCHG64i : RIi32<0x81, MRM4r, (outs GR64:$dst), (ins GR64:$src1, i32u32imm:$src),
                               "fscp_xchg{q}\t{$src, $dst|$dst, $src}",
                               [(set GR64:$dst, (int_x86_icecode_fscp_xchg_64 imm:$src, GR64:$src1))]>;
    }

    def CREG_READ32r : I<0x29, MRMDestReg, (outs GR32:$dst), (ins GR32:$src),
                         "creg_read{l}\t{$src, $dst|$dst, $src}",
                         [(set GR32:$dst, (int_x86_icecode_creg_read_32 GR32:$src))]>;
    def CREG_READ32i : Ii32<0x81, MRM5r, (outs GR32:$dst), (ins i32u32imm:$src),
                            "creg_read{l}\t{$src, $dst|$dst, $src}",
                            [(set GR32:$dst, (int_x86_icecode_creg_read_32 imm:$src))]>;

    def CREG_READ64r : RI<0x29, MRMDestReg, (outs GR64:$dst), (ins GR32:$src),
                          "creg_read{q}\t{$src, $dst|$dst, $src}",
                          [(set GR64:$dst, (int_x86_icecode_creg_read_64 GR32:$src))]>;
    def CREG_READ64i : RIi32<0x81, MRM5r, (outs GR64:$dst), (ins i32u32imm:$src),
                             "creg_read{q}\t{$src, $dst|$dst, $src}",
                             [(set GR64:$dst, (int_x86_icecode_creg_read_64 imm:$src))]>;

    def FSCP_READ32r : I<0x09, MRMDestReg, (outs GR32:$dst), (ins GR32:$src),
                         "fscp_read{l}\t{$src, $dst|$dst, $src}",
                         [(set GR32:$dst, (int_x86_icecode_fscp_read_32 GR32:$src))]>;
    def FSCP_READ32i : Ii32<0x81, MRM1r, (outs GR32:$dst), (ins i32u32imm:$src),
                            "fscp_read{l}\t{$src, $dst|$dst, $src}",
                            [(set GR32:$dst, (int_x86_icecode_fscp_read_32 imm:$src))]>;

    def FSCP_READ64r : RI<0x09, MRMDestReg, (outs GR64:$dst), (ins GR32:$src),
                          "fscp_read{q}\t{$src, $dst|$dst, $src}",
                          [(set GR64:$dst, (int_x86_icecode_fscp_read_64 GR32:$src))]>;
    def FSCP_READ64i : RIi32<0x81, MRM1r, (outs GR64:$dst), (ins i32u32imm:$src),
                             "fscp_read{q}\t{$src, $dst|$dst, $src}",
                             [(set GR64:$dst, (int_x86_icecode_fscp_read_64 imm:$src))]>;

    def PORTIN8rm  : I<0x8a, MRMSrcMem, (outs GR8:$dst), (ins opaquemem:$src),
                       "portin{b}\t{$src, $dst|$dst, $src}",
                       [(set GR8:$dst, (int_x86_icecode_portin_8 addr:$src))]>, AdSize32;
    def PORTIN16rm : I<0x8b, MRMSrcMem, (outs GR16:$dst), (ins opaquemem:$src),
                       "portin{w}\t{$src, $dst|$dst, $src}",
                       [(set GR16:$dst, (int_x86_icecode_portin_16 addr:$src))]>, OpSize16, AdSize32;
    def PORTIN32rm : I<0x8b, MRMSrcMem, (outs GR32:$dst), (ins opaquemem:$src),
                       "portin{l}\t{$src, $dst|$dst, $src}",
                       [(set GR32:$dst, (int_x86_icecode_portin_32 addr:$src))]>, OpSize32, AdSize32;
    def PORTIN64rm : RI<0x8b, MRMSrcMem, (outs GR64:$dst), (ins opaquemem:$src),
                        "portin{q}\t{$src, $dst|$dst, $src}",
                        [(set GR64:$dst, (int_x86_icecode_portin_64 addr:$src))]>, AdSize32;

    def PORTOUT8mr  : I<0x88, MRMDestMem, (outs), (ins opaquemem:$dst, GR8:$src),
                        "portout{b}\t{$src, $dst|$dst, $src}",
                        [(int_x86_icecode_portout_8 GR8:$src, addr:$dst)]>, AdSize32;
    def PORTOUT16mr : I<0x89, MRMDestMem, (outs), (ins opaquemem:$dst, GR16:$src),
                        "portout{w}\t{$src, $dst|$dst, $src}",
                        [(int_x86_icecode_portout_16 GR16:$src, addr:$dst)]>, OpSize16, AdSize32;
    def PORTOUT32mr : I<0x89, MRMDestMem, (outs), (ins opaquemem:$dst, GR32:$src),
                        "portout{l}\t{$src, $dst|$dst, $src}",
                        [(int_x86_icecode_portout_32 GR32:$src, addr:$dst)]>, OpSize32, AdSize32;
    def PORTOUT64mr : RI<0x89, MRMDestMem, (outs), (ins opaquemem:$dst, GR64:$src),
                         "portout{q}\t{$src, $dst|$dst, $src}",
                         [(int_x86_icecode_portout_64 GR64:$src, addr:$dst)]>, AdSize32;

    def STA_SPECIALm : I<0x8f, MRM0m, (outs), (ins anymem:$mem), "sta_special\t$mem",
                         [(int_x86_icecode_sta_special addr:$mem)]>, NotMemoryFoldable, AdSize32;
    def NR_READr : Ii8<0x83, MRM6r, (outs GR32:$dst), (ins i32u8imm:$src),
                       "nr_read\t{$src, $dst|$dst, $src}",
                       [(set GR32:$dst, (int_x86_icecode_nr_read (i32 timm:$src)))]>;

    let Uses = [RAX] in
      def SIGEVENTJUMP : Ii32<0x81, MRM2r, (outs), (ins GR32:$src1, i32imm:$src2),
                              "sigeventjump\t{$src2, $src1|$src1, $src2}", []>;

    def SETTRACKER : Ii8<0x6a, RawFrm, (outs), (ins i32u8imm:$src),
                         "settracker\t$src",
                         [(int_x86_icecode_set_tracker (i32 timm:$src))]>;
  }

  def SSERIALIZE : I<0x01, MRM_D4, (outs), (ins), "sserialize", []>, TB;

  def UCODECALL  : I<0x22, MRM0r, (outs), (ins GR32:$src),
                     "ucodecall\t$src", [(int_x86_icecode_ucodecall GR32:$src)]>, TB;

  def NOP_SET_SB  : I<0x27, RawFrm, (outs), (ins), "nop_set_sb", []>;
  def NOP_READ_SB : I<0x2f, RawFrm, (outs), (ins), "nop_read_sb", []>;

  def GET_EXCL_ACC : I<0xfc, RawFrm, (outs), (ins), "get_excl_acc", []>;
  def RELEASE_EXCL_ACC : I<0xfd, RawFrm, (outs), (ins), "release_excl_acc", []>;

  let OpPrefix = XD, Uses = [RDX], Defs = [EAX] in
    def PORTIN24 : I<0xad, RawFrm, (outs), (ins), "portin24", []>;
  let OpPrefix = XD, Uses = [RDX, EAX] in
    def PORTOUT24 : I<0xab, RawFrm, (outs), (ins), "portout24", []>;

  def VIRT_NUKE_POINT : I<0x01, MRM_D5, (outs), (ins), "virt_nuke_point", []>, TB;
  def INT_TRAP_POINT  : I<0x01, MRM_D0, (outs), (ins), "int_trap_point", []>, TB;

  let Uses = [EAX] in
    def ICERET : I<0xaa, RawFrm, (outs), (ins), "iceret\t{%eax|eax}", []>, TB;
  let Uses = [EAX, EBX] in
    def ICERET_INDIRECT : I<0xaa, RawFrm, (outs), (ins),
                            "iceret_indirect\t{%ebx, %eax|eax, ebx}", []>, XS;

  let Constraints = "$src1 = $dst" in
    def CMODEMOVcc : Ii8<0x40, MRMSrcRegCC, (outs GR64:$dst), (ins GR64:$src1, GR64:$src2, i32u4imm:$cond),
                         "cmodemov\t{$cond, $src2, $dst|$dst, $src2, $cond}",
                         [(set GR64:$dst, (int_x86_icecode_cmodemov GR64:$src1, GR64:$src2, (i32 timm:$cond)))]>, XS;
}
// end INTEL_FEATURE_ICECODE
