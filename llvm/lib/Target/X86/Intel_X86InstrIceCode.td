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
multiclass ce_xchg_i<Format F, string asm, SDNode node> {
  let Constraints = "$src1 = $dst" in {
    def 32i : Ii32<0x81, F, (outs GR32:$dst), (ins GR32:$src1, i32u32imm:$src),
                   !strconcat(asm, "{l}\t{$src, $dst|$dst, $src}"),
                   [(set GR32:$dst, (!cast<Intrinsic>("int_x86_icecode_" # asm # "_32") node:$src, GR32:$src1))]>;
    def 64i : RIi32<0x81, F, (outs GR64:$dst), (ins GR64:$src1, i32u32imm:$src),
                    !strconcat(asm, "{q}\t{$src, $dst|$dst, $src}"),
                    [(set GR64:$dst, (!cast<Intrinsic>("int_x86_icecode_" # asm # "_64") node:$src, GR64:$src1))]>;
  }
}

multiclass ce_xchg<bits<8> O, string asm> {
  let Constraints = "$src1 = $dst" in {
    def 32r : I<O, MRMDestReg, (outs GR32:$dst), (ins GR32:$src1, GR32:$src),
                !strconcat(asm, "{l}\t{$src, $dst|$dst, $src}"),
                [(set GR32:$dst, (!cast<Intrinsic>("int_x86_icecode_" # asm # "_32") GR32:$src, GR32:$src1))]>;
    def 64r : RI<O, MRMDestReg, (outs GR64:$dst), (ins GR64:$src1, GR32:$src),
                 !strconcat(asm, "{q}\t{$src, $dst|$dst, $src}"),
                 [(set GR64:$dst, (!cast<Intrinsic>("int_x86_icecode_" # asm # "_64") GR32:$src, GR64:$src1))]>;
  }
}

multiclass ce_read<bits<8> O, Format F, string asm> {
  def 32r : I<O, MRMDestReg, (outs GR32:$dst), (ins GR32:$src),
              !strconcat(asm, "{l}\t{$src, $dst|$dst, $src}"),
              [(set GR32:$dst, (!cast<Intrinsic>("int_x86_icecode_" # asm # "_32") GR32:$src))]>;
  def 32i : Ii32<0x81, F, (outs GR32:$dst), (ins i32u32imm:$src),
                 !strconcat(asm, "{l}\t{$src, $dst|$dst, $src}"),
                 [(set GR32:$dst, (!cast<Intrinsic>("int_x86_icecode_" # asm # "_32") imm:$src))]>;

  def 64r : RI<O, MRMDestReg, (outs GR64:$dst), (ins GR32:$src),
               !strconcat(asm, "{q}\t{$src, $dst|$dst, $src}"),
               [(set GR64:$dst, (!cast<Intrinsic>("int_x86_icecode_" # asm # "_64") GR32:$src))]>;
  def 64i : RIi32<0x81, F, (outs GR64:$dst), (ins i32u32imm:$src),
                  !strconcat(asm, "{q}\t{$src, $dst|$dst, $src}"),
                  [(set GR64:$dst, (!cast<Intrinsic>("int_x86_icecode_" # asm # "_64") imm:$src))]>;
}

multiclass ce_load_store_v<bits<8> O, string asm> {
  def 16rm : I<O, MRMSrcMem, (outs GR16:$dst), (ins opaquemem:$src),
               !strconcat("gmov", asm, "{w}\t{$src, $dst|$dst, $src}"),
               [(set GR16:$dst, (!cast<Intrinsic>("int_x86_icecode_load" # asm # "_16") addr:$src))]>, XS, OpSize16;
  def 32rm : I<O, MRMSrcMem, (outs GR32:$dst), (ins opaquemem:$src),
               !strconcat("gmov", asm, "{l}\t{$src, $dst|$dst, $src}"),
               [(set GR32:$dst, (!cast<Intrinsic>("int_x86_icecode_load" # asm # "_32") addr:$src))]>, XS, OpSize32;
  def 64rm : RI<O, MRMSrcMem, (outs GR64:$dst), (ins opaquemem:$src),
                !strconcat("gmov", asm, "{q}\t{$src, $dst|$dst, $src}"),
                [(set GR64:$dst, (!cast<Intrinsic>("int_x86_icecode_load" # asm # "_64") addr:$src))]>, XS;

  def 16mr : I<O, MRMDestMem, (outs), (ins opaquemem:$dst, GR16:$src),
               !strconcat("gmov", asm, "{w}\t{$src, $dst|$dst, $src}"),
               [(!cast<Intrinsic>("int_x86_icecode_store" # asm # "_16") GR16:$src, addr:$dst)]>, XD, OpSize16;
  def 32mr : I<O, MRMDestMem, (outs), (ins opaquemem:$dst, GR32:$src),
               !strconcat("gmov", asm, "{l}\t{$src, $dst|$dst, $src}"),
               [(!cast<Intrinsic>("int_x86_icecode_store" # asm # "_32") GR32:$src, addr:$dst)]>, XD, OpSize32;
  def 64mr : RI<O, MRMDestMem, (outs), (ins opaquemem:$dst, GR64:$src),
                !strconcat("gmov", asm, "{q}\t{$src, $dst|$dst, $src}"),
                [(!cast<Intrinsic>("int_x86_icecode_store" # asm # "_64") GR64:$src, addr:$dst)]>, XD;
}

multiclass ce_load_store_p<bits<8> O1, bits<8> O2, bits<8> O3, bits<8> O4,
                           string r, string rs, string w, string ws> {
  def 8rm  : I<O1, MRMSrcMem, (outs GR8:$dst), (ins opaquemem:$src),
               !strconcat(r, "{b}\t{$src, $dst|$dst, $src}"),
               [(set GR8:$dst, (!cast<Intrinsic>("int_x86_icecode_" # rs # "_8") addr:$src))]>;
  def 16rm : I<O2, MRMSrcMem, (outs GR16:$dst), (ins opaquemem:$src),
               !strconcat(r, "{w}\t{$src, $dst|$dst, $src}"),
               [(set GR16:$dst, (!cast<Intrinsic>("int_x86_icecode_" # rs # "_16") addr:$src))]>, OpSize16;
  def 32rm : I<O2, MRMSrcMem, (outs GR32:$dst), (ins opaquemem:$src),
               !strconcat(r, "{l}\t{$src, $dst|$dst, $src}"),
               [(set GR32:$dst, (!cast<Intrinsic>("int_x86_icecode_" # rs # "_32") addr:$src))]>, OpSize32;
  def 64rm : RI<O2, MRMSrcMem, (outs GR64:$dst), (ins opaquemem:$src),
                !strconcat(r, "{q}\t{$src, $dst|$dst, $src}"),
                [(set GR64:$dst, (!cast<Intrinsic>("int_x86_icecode_" # rs # "_64") addr:$src))]>;

  def 8mr  : I<O3, MRMDestMem, (outs), (ins opaquemem:$dst, GR8:$src),
               !strconcat(w, "{b}\t{$src, $dst|$dst, $src}"),
               [(!cast<Intrinsic>("int_x86_icecode_" # ws # "_8") GR8:$src, addr:$dst)]>;
  def 16mr : I<O4, MRMDestMem, (outs), (ins opaquemem:$dst, GR16:$src),
               !strconcat(w, "{w}\t{$src, $dst|$dst, $src}"),
               [(!cast<Intrinsic>("int_x86_icecode_" # ws # "_16") GR16:$src, addr:$dst)]>, OpSize16;
  def 32mr : I<O4, MRMDestMem, (outs), (ins opaquemem:$dst, GR32:$src),
               !strconcat(w, "{l}\t{$src, $dst|$dst, $src}"),
               [(!cast<Intrinsic>("int_x86_icecode_" # ws # "_32") GR32:$src, addr:$dst)]>, OpSize32;
  def 64mr : RI<O4, MRMDestMem, (outs), (ins opaquemem:$dst, GR64:$src),
                !strconcat(w, "{q}\t{$src, $dst|$dst, $src}"),
                [(!cast<Intrinsic>("int_x86_icecode_" # ws # "_64") GR64:$src, addr:$dst)]>;
}

let Predicates = [InIceCodeMode] in {
  def LOADSEGrm : I<0xb2, MRM0m, (outs), (ins i128mem:$mem),
                    "loadseg\t$mem", []>, PS;
  def STORESEGmr : I<0xb2, MRM0m, (outs), (ins i128mem:$mem),
                     "storeseg\t$mem", []>, PD;

  defm GMOVLIN   : ce_load_store_v<0x02, "lin">;

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
    defm PORT : ce_load_store_p<0x8a, 0x8b, 0x88, 0x89, "portin", "portin", "portout", "portout">, AdSize32;
    defm GMOVPPHYS : ce_load_store_p<0x12, 0x13, 0x10, 0x11, "gmovpphys", "loadpphys", "gmovpphys", "storepphys">;

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

  defm CREG_READ: ce_read<0x29, MRM5r, "creg_read">, OBXS;
  defm FSCP_READ: ce_read<0x09, MRM1r, "fscp_read">, OBXS;
  defm CREG_XCHG: ce_xchg<0x01, "creg_xchg">, ce_xchg_i<MRM0r, "creg_xchg", imm>, OBXS;
  defm FSCP_XCHG: ce_xchg<0x21, "fscp_xchg">, ce_xchg_i<MRM4r, "fscp_xchg", imm>, OBXS;
  defm FSCP_OR : ce_xchg_i<MRM1r, "fscp_or", timm>, OBXD;
  defm CREG_OR_MT : ce_xchg_i<MRM5r, "creg_or_mt", timm>, OBXD;
  defm FSCP_ANDNOT : ce_xchg_i<MRM4r, "fscp_andnot", timm>, OBXD;
  defm CREG_ANDNOT_MT : ce_xchg_i<MRM0r, "creg_andnot_mt", timm>, OBXD;

  let OpPrefix = XD, OpMap = OB in {
    let Constraints = "$src1 = $dst" in {
      def CREG_XCHG_MT32r : I<0x01, MRMDestReg, (outs GR32:$dst), (ins GR32:$src1, GR32:$src),
                              "creg_xchg_mt{l}\t{$src, $dst|$dst, $src}",
                              [(set GR32:$dst, (int_x86_icecode_creg_xchg_mt_32 GR32:$src, GR32:$src1))]>;
      def CREG_XCHG_MT64r : RI<0x01, MRMDestReg, (outs GR64:$dst), (ins GR64:$src1, GR32:$src),
                               "creg_xchg_mt{q}\t{$src, $dst|$dst, $src}",
                               [(set GR64:$dst, (int_x86_icecode_creg_xchg_mt_64 GR32:$src, GR64:$src1))]>;
    }

    def CREG_READ_MT32r : I<0x29, MRMDestReg, (outs GR32:$dst), (ins GR32:$src),
                            "creg_read_mt{l}\t{$src, $dst|$dst, $src}",
                            [(set GR32:$dst, (int_x86_icecode_creg_read_mt_32 GR32:$src))]>;
    def CREG_READ_MT64r : RI<0x29, MRMDestReg, (outs GR64:$dst), (ins GR32:$src),
                             "creg_read_mt{q}\t{$src, $dst|$dst, $src}",
                             [(set GR64:$dst, (int_x86_icecode_creg_read_mt_64 GR32:$src))]>;
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

  let Uses = [RAX], OpMap = TB in {
    def LOAD_TICKLE_GPA : I<0x01, MRM_CA, (outs), (ins), "load_tickle_gpa", []>;
    def STORE_TICKLE_GPA : I<0x01, MRM_CB, (outs), (ins), "store_tickle_gpa", []>;
  }
  let Uses = [RAX] in
    def FLUSH_IFU : I<0x01, MRM_D4, (outs), (ins), "flush_ifu\t{%rax|rax}", []>, XS;

  let Constraints = "$src = $dst", Defs = [EFLAGS] in {
    def CCCM : RI<0xc8, AddRegFrm, (outs GR64:$dst), (ins GR64:$src),
                  "cccm\t$dst", []>, XS;
    def CCCP : RI<0xc8, AddRegFrm, (outs GR64:$dst), (ins GR64:$src),
                  "cccp\t$dst", []>, XD;
  }

  let isBranch = 1, isTerminator = 1, isBarrier = 1, isIndirectBranch = 1 in
    def JMP_NOPRED : RI<0xc7, MRM7r, (outs), (ins GR64:$src), "jmp_nopred\t$src", []>, XS;

  def FE_SERIZLIZE : I<0x01, MRM_D1, (outs), (ins), "fe_serizlize", []>, TB;
  let Uses = [RAX] in
    def IO_TICKLE_DEBUG : I<0x06, RawFrm, (outs), (ins), "io_tickle_debug\t{%rax|rax}", []>, TB;

  def LOADVPHYS : I<0xc7, MRM6m, (outs), (ins anymem:$mem), "loaduphys\t$mem",
                    [(int_x86_icecode_loaduphys addr:$mem)]>, NotMemoryFoldable;
  def STOREVPHYS : I<0xc7, MRM7m, (outs), (ins anymem:$mem), "storeuphys\t$mem",
                     [(int_x86_icecode_storeuphys addr:$mem)]>, NotMemoryFoldable;
}
// end INTEL_FEATURE_ICECODE
