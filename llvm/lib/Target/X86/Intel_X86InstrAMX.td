//===---------------------*- tablegen -*-----------------------------------===//
//===- Intel_X86InstrAMX.td - AMX Instruction Set Extension -*- tablegen -*===//
//
//      Copyright (c) 2018-2019 Intel Corporation.
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
// This file describes the instructions that make up the Intel AMX instruction
// set.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// AMX instructions

// INTEL_FEATURE_ISA_AMX_FUTURE

// TADDPS check tmemrow
multiclass m_tadds {
  let Predicates = [HasAMXELEMENT, In64BitMode] in {
    let SchedRW = [WriteSystem] in {
      def rr : I<0xe0, MRMSrcReg, (outs TILE:$dst),
                 (ins TILE:$src1, TILE:$src2),
                 "taddps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, VEX_4V, T_MAP5PS;
      def rm : I<0xe0, MRMSrcMem, (outs TILE:$dst),
                 (ins TILE:$src1, i8mem:$src2),
                 "taddps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, VEX_4V, T_MAP5PS;
    }
  } // HasAMXTILE, HasAMXELEMENT
}

defm TADDPS : m_tadds;

// TANDD check tmemrow
multiclass m_tandd {
  let Predicates = [HasAMXELEMENT, In64BitMode] in {
    let SchedRW = [WriteSystem] in {
      def rr : I<0xe0, MRMSrcReg, (outs TILE:$dst),
                 (ins TILE:$src1, TILE:$src2),
                 "tandd\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, VEX_4V, T_MAP5PD;
      def rm : I<0xe0, MRMSrcMem, (outs TILE:$dst),
                 (ins TILE:$src1, i8mem:$src2),
                 "tandd\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, VEX_4V, T_MAP5PD;
    }
  } // HasAMXTILE, HasAMXELEMENT
}

defm TANDD : m_tandd;

// TANDND check tmemrow
multiclass m_tandnd {
  let Predicates = [HasAMXELEMENT, In64BitMode] in {
    let SchedRW = [WriteSystem] in {
      def rr : I<0xe0, MRMSrcReg, (outs TILE:$dst),
                 (ins TILE:$src1, TILE:$src2),
                 "tandnd\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, VEX_4V, T_MAP5XD;
      def rm : I<0xe0, MRMSrcMem, (outs TILE:$dst),
                 (ins TILE:$src1, i8mem:$src2),
                 "tandnd\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, VEX_4V, T_MAP5XD;
    }
  } // HasAMXTILE, HasAMXELEMENT
}

defm TANDND : m_tandnd;

// Pseudo of TADDPS TANDD TANDND
let Predicates = [HasAMXELEMENT, In64BitMode] in {
  let SchedRW = [WriteSystem],
      usesCustomInserter = 1 in {
    // Pseudo instructions, using immediates instead of tile registers.
    // To be translated to the actual instructions in X86ISelLowering.cpp
    def PTADDPS : PseudoI<(outs), (ins u8imm:$dst,
                          u8imm:$src1, u8imm:$src2),
                          [(int_x86_taddps_reg timm:$dst,
                            timm:$src1, timm:$src2)]>;
    def PTADDPSrm : PseudoI<(outs), (ins u8imm:$dst,
                          u8imm:$src1, i8mem:$src2),
                          [(int_x86_taddps_mem timm:$dst,
                            timm:$src1, addr:$src2)]>;
    def PTANDD : PseudoI<(outs), (ins u8imm:$dst,
                         u8imm:$src1, u8imm:$src2),
                         [(int_x86_tandd_reg timm:$dst,
                           timm:$src1, timm:$src2)]>;
    def PTANDDrm : PseudoI<(outs), (ins u8imm:$dst,
                          u8imm:$src1, i8mem:$src2),
                          [(int_x86_tandd_mem timm:$dst,
                            timm:$src1, addr:$src2)]>;
    def PTANDND : PseudoI<(outs), (ins u8imm:$dst,
                          u8imm:$src1, u8imm:$src2),
                          [(int_x86_tandnd_reg timm:$dst,
                            timm:$src1, timm:$src2)]>;
    def PTANDNDrm : PseudoI<(outs), (ins u8imm:$dst,
                          u8imm:$src1, i8mem:$src2),
                          [(int_x86_tandnd_mem timm:$dst,
                            timm:$src1, addr:$src2)]>;
  }
} // HasAMXTILE, HasAMXELEMENT

// TBLENDVD
let Predicates = [HasAMXFORMAT, In64BitMode],
    SchedRW = [WriteSystem] in {
  let Constraints = "$src1 = $dst" in {
    def TBLENDVD : I<0xe6, MRMSrcReg, (outs TILE:$dst),
                     (ins TILE:$src1, TILE:$src2, TILE:$src3),
                     "tblendvd\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                     []>, VEX_4V, T_MAP5XD;
  }
} // HasAMXTILE, HasAMXFORMAT

let Predicates = [HasAMXFORMAT, In64BitMode],
    SchedRW = [WriteSystem] in {
  let  usesCustomInserter = 1 in {
    def PTBLENDVD : PseudoI<(outs), (ins u8imm:$src1,
                            u8imm:$src2, u8imm:$src3),
                            [(int_x86_tblendvd timm:$src1,
                              timm:$src2, timm:$src3)]>;
  }
} // HasAMXTILE, HasAMXFORMAT

// TCMPPS VCMPPH MRMSrcMem
multiclass m_tcmpps {
  let Predicates = [HasAMXELEMENT, In64BitMode] in {
    let SchedRW = [WriteSystem] in {
      def rr : Ii8<0x59, MRMSrcReg, (outs TILE:$dst),
                   (ins TILE:$src1, TILE:$src2, u8imm:$src3),
                   "tcmpps\t{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                   []>, VEX_4V, TAXS;
      def rm : Ii8<0x59, MRMSrcMem, (outs TILE:$dst),
                   (ins TILE:$src1, i8mem:$src2, u8imm:$src3),
                   "tcmpps\t{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                   []>, VEX_4V, TAXS;
    }
  } // HasAMXTILE, HasAMXELEMENT
}

defm TCMPPS : m_tcmpps;

// Pseudo of TCMPPS
let Predicates = [HasAMXELEMENT, In64BitMode] in {
  let SchedRW = [WriteSystem],
      usesCustomInserter = 1 in {
    def PTCMPPS : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src1,
                          u8imm:$src2, u8imm:$src3),
                          [(int_x86_tcmpps_reg timm:$dst, timm:$src1,
                            timm:$src2, timm:$src3)]>;
    def PTCMPPSrm : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src1,
                            i8mem:$src2, u8imm:$src3),
                            [(int_x86_tcmpps_mem timm:$dst, timm:$src1,
                              addr:$src2, timm:$src3)]>;
  }
} // HasAMXTILE, HasAMXELEMENT

// TCOLADDBCASTPS TCOLADDPS
let Predicates = [HasAMXREDUCE, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TCOLADDBCASTPS : I<0xe6, MRMSrcReg, (outs TILE:$dst),
                           (ins TILE:$src),
                           "tcoladdbcastps\t{$src, $dst|$dst, $src}",
                           []>, VEX, T_MAP5PD;
    def TCOLADDPS : I<0xe6, MRMDestMem, (outs),
                      (ins i8mem:$dst, TILE:$src),
                      "tcoladdps\t{$src, $dst|$dst, $src}",
                      []>, VEX, T_MAP5PS;
    let usesCustomInserter = 1 in {
      def PTCOLADDBCASTPS : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src),
                            [(int_x86_tcoladdbcastps timm:$dst, timm:$src)]>;
      def PTCOLADDPS : PseudoI<(outs), (ins i8mem:$dst, u8imm:$src),
                            [(int_x86_tcoladdps addr:$dst, timm:$src)]>;
    }
  }
} // HasAMXTILE, HasAMXREDUCE

// TCVT
multiclass m_tcvt {
  let Predicates = [HasAMXELEMENT, In64BitMode] in {
    let SchedRW = [WriteSystem] in {
      def B2PS : I<0xe1, MRMSrcReg, (outs TILE:$dst),
                   (ins TILE:$src),
                   "tcvtb2ps\t{$src, $dst|$dst, $src}",
                   []>, VEX, T_MAP5PS;
      def BF162PS : I<0xe1, MRMSrcReg, (outs TILE:$dst),
                      (ins TILE:$src),
                      "tcvtbf162ps\t{$src, $dst|$dst, $src}",
                      []>, VEX, T_MAP5XD;
      def D2PS : I<0xe1, MRMSrcReg, (outs TILE:$dst),
                   (ins TILE:$src),
                   "tcvtd2ps\t{$src, $dst|$dst, $src}",
                   []>, VEX, T_MAP5XS;
      def PS2BF16 : I<0xe2, MRMSrcReg, (outs TILE:$dst),
                      (ins TILE:$src),
                      "tcvtps2bf16\t{$src, $dst|$dst, $src}",
                      []>, VEX, T_MAP5XD;
      def PS2BS : I<0xe2, MRMSrcReg, (outs TILE:$dst),
                    (ins TILE:$src),
                    "tcvtps2bs\t{$src, $dst|$dst, $src}",
                    []>, VEX, T_MAP5PS;
      def PS2UBS : I<0xe2, MRMSrcReg, (outs TILE:$dst),
                     (ins TILE:$src),
                     "tcvtps2ubs\t{$src, $dst|$dst, $src}",
                     []>, VEX, T_MAP5PD;
      def UB2PS : I<0xe1, MRMSrcReg, (outs TILE:$dst),
                    (ins TILE:$src),
                    "tcvtub2ps\t{$src, $dst|$dst, $src}",
                    []>, VEX, T_MAP5PD;
    }
  } // HasAMXTILE, HasAMXELEMENT
}

defm TCVT : m_tcvt;

// Pseudo of TCVT
let Predicates = [HasAMXELEMENT, In64BitMode] in {
  let SchedRW = [WriteSystem],
      usesCustomInserter = 1 in {
    def PTCVTB2PS : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src),
                            [(int_x86_tcvtb2ps timm:$dst, timm:$src)]>;
    def PTCVTBF162PS : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src),
                            [(int_x86_tcvtbf162ps timm:$dst, timm:$src)]>;
    def PTCVTD2PS : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src),
                            [(int_x86_tcvtd2ps timm:$dst, timm:$src)]>;
    def PTCVTPS2BF16 : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src),
                            [(int_x86_tcvtps2bf16 timm:$dst, timm:$src)]>;
    def PTCVTPS2BS : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src),
                            [(int_x86_tcvtps2bs timm:$dst, timm:$src)]>;
    def PTCVTPS2UBS : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src),
                            [(int_x86_tcvtps2ubs timm:$dst, timm:$src)]>;
    def PTCVTUB2PS : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src),
                            [(int_x86_tcvtub2ps timm:$dst, timm:$src)]>;
  }
} // HasAMXTILE, HasAMXELEMENT

// TFMADDPS TFMSUBPS check tmemrow
let Predicates = [HasAMXELEMENT, In64BitMode],
    SchedRW = [WriteSystem] in {
  let Constraints = "$src1 = $dst" in {
    def TFMADDPSrr : I<0xe2, MRMSrcReg, (outs TILE:$dst),
                       (ins TILE:$src1, TILE:$src2, TILE:$src3),
                       "tfmaddps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                       []>, VEX_4V, T_MAP5XS;
    def TFMADDPSrm : I<0xe2, MRMSrcMem, (outs TILE:$dst),
                       (ins TILE:$src1, TILE:$src2, i8mem:$src3),
                       "tfmaddps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                       []>, VEX_4V, T_MAP5XS;
    def TFMSUBPSrr : I<0xe3, MRMSrcReg, (outs TILE:$dst),
                       (ins TILE:$src1, TILE:$src2, TILE:$src3),
                       "tfmsubps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                       []>, VEX_4V, T_MAP5PS;
    def TFMSUBPSrm : I<0xe3, MRMSrcMem, (outs TILE:$dst),
                       (ins TILE:$src1, TILE:$src2, i8mem:$src3),
                       "tfmsubps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                       []>, VEX_4V, T_MAP5PS;
    def TFNMADDPSrr : I<0xe3, MRMSrcReg, (outs TILE:$dst),
                       (ins TILE:$src1, TILE:$src2, TILE:$src3),
                       "tfnmaddps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                       []>, VEX_4V, T_MAP5PD;
    def TFNMADDPSrm : I<0xe3, MRMSrcMem, (outs TILE:$dst),
                       (ins TILE:$src1, TILE:$src2, i8mem:$src3),
                       "tfnmaddps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                       []>, VEX_4V, T_MAP5PD;
    def TFNMSUBPSrr : I<0xe3, MRMSrcReg, (outs TILE:$dst),
                       (ins TILE:$src1, TILE:$src2, TILE:$src3),
                       "tfnmsubps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                       []>, VEX_4V, T_MAP5XD;
    def TFNMSUBPSrm : I<0xe3, MRMSrcMem, (outs TILE:$dst),
                       (ins TILE:$src1, TILE:$src2, i8mem:$src3),
                       "tfnmsubps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                       []>, VEX_4V, T_MAP5XD;
  }
} // HasAMXTILE, HasAMXELEMENT

let Predicates = [HasAMXELEMENT, In64BitMode],
    SchedRW = [WriteSystem] in {
  let  usesCustomInserter = 1 in {
    def PTFMADDPS : PseudoI<(outs), (ins u8imm:$src1,
                            u8imm:$src2, u8imm:$src3),
                            [(int_x86_tfmaddps_reg timm:$src1,
                              timm:$src2, timm:$src3)]>;
    def PTFMADDPSrm : PseudoI<(outs), (ins u8imm:$src1,
                              u8imm:$src2, i8mem:$src3),
                              [(int_x86_tfmaddps_mem timm:$src1,
                                timm:$src2, addr:$src3)]>;
    def PTFMSUBPS : PseudoI<(outs), (ins u8imm:$src1,
                            u8imm:$src2, u8imm:$src3),
                            [(int_x86_tfmsubps_reg timm:$src1,
                              timm:$src2, timm:$src3)]>;
    def PTFMSUBPSrm : PseudoI<(outs), (ins u8imm:$src1,
                              u8imm:$src2, i8mem:$src3),
                              [(int_x86_tfmsubps_mem timm:$src1,
                                timm:$src2, addr:$src3)]>;
    def PTFNMADDPS : PseudoI<(outs), (ins u8imm:$src1,
                             u8imm:$src2, u8imm:$src3),
                             [(int_x86_tfnmaddps_reg timm:$src1,
                               timm:$src2, timm:$src3)]>;
    def PTFNMADDPSrm : PseudoI<(outs), (ins u8imm:$src1,
                               u8imm:$src2, i8mem:$src3),
                               [(int_x86_tfnmaddps_mem timm:$src1,
                                 timm:$src2, addr:$src3)]>;
    def PTFNMSUBPS : PseudoI<(outs), (ins u8imm:$src1,
                             u8imm:$src2, u8imm:$src3),
                             [(int_x86_tfnmsubps_reg timm:$src1,
                               timm:$src2, timm:$src3)]>;
    def PTFNMSUBPSrm : PseudoI<(outs), (ins u8imm:$src1,
                               u8imm:$src2, i8mem:$src3),
                               [(int_x86_tfnmsubps_mem timm:$src1,
                                 timm:$src2, addr:$src3)]>;
  }
} // HasAMXTILE, HasAMXELEMENT

// TINTERLEAVE
multiclass m_tinterleave {
  let Predicates = [HasAMXFORMAT, In64BitMode] in {
    let SchedRW = [WriteSystem] in {
      def EB : I<0xe8, MRMSrcReg, (outs TILE:$dst),
                 (ins TILE:$src1, TILE:$src2),
                 "tinterleaveeb\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, VEX_4V, T_MAP5XD;
      def EW : I<0xe8, MRMSrcReg, (outs TILE:$dst),
                 (ins TILE:$src1, TILE:$src2),
                 "tinterleaveew\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, VEX_4V, T_MAP5XS;
      def OB : I<0xe8, MRMSrcReg, (outs TILE:$dst),
                 (ins TILE:$src1, TILE:$src2),
                 "tinterleaveob\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, VEX_4V, T_MAP5PS;
      def OW : I<0xe8, MRMSrcReg, (outs TILE:$dst),
                 (ins TILE:$src1, TILE:$src2),
                 "tinterleaveow\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, VEX_4V, T_MAP5PD;
    }
  }
} // HasAMXTILE, HasAMXFORMAT

defm TINTERLEAVE : m_tinterleave;

let Predicates = [HasAMXFORMAT, In64BitMode] in {
  let SchedRW = [WriteSystem],
      usesCustomInserter = 1 in {
    def PTINTERLEAVEEB : PseudoI<(outs), (ins u8imm:$dst,
                                 u8imm:$src1, u8imm:$src2),
                                 [(int_x86_tinterleaveeb timm:$dst,
                                   timm:$src1, timm:$src2)]>;
    def PTINTERLEAVEEW : PseudoI<(outs), (ins u8imm:$dst,
                                 u8imm:$src1, u8imm:$src2),
                                 [(int_x86_tinterleaveew timm:$dst,
                                   timm:$src1, timm:$src2)]>;
    def PTINTERLEAVEOB : PseudoI<(outs), (ins u8imm:$dst,
                                 u8imm:$src1, u8imm:$src2),
                                 [(int_x86_tinterleaveob timm:$dst,
                                   timm:$src1, timm:$src2)]>;
    def PTINTERLEAVEOW : PseudoI<(outs), (ins u8imm:$dst,
                                 u8imm:$src1, u8imm:$src2),
                                 [(int_x86_tinterleaveow timm:$dst,
                                   timm:$src1, timm:$src2)]>;
  }
} // HasAMXTILE,HasAMXFORMAT

// TMAXPS TMINPS TMULPS check tmemrow
let Predicates = [HasAMXELEMENT, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TMAXPSrr : I<0xe3, MRMSrcReg, (outs TILE:$dst),
                     (ins TILE:$src1, TILE:$src2),
                     "tmaxps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5XS;
    def TMAXPSrm : I<0xe3, MRMSrcMem, (outs TILE:$dst),
                     (ins TILE:$src1, i8mem:$src2),
                     "tmaxps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5XS;
    def TMINPSrr : I<0xe4, MRMSrcReg, (outs TILE:$dst),
                     (ins TILE:$src1, TILE:$src2),
                     "tminps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PS;
    def TMINPSrm : I<0xe4, MRMSrcMem, (outs TILE:$dst),
                     (ins TILE:$src1, i8mem:$src2),
                     "tminps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PS;
    def TMULPSrr : I<0xe4, MRMSrcReg, (outs TILE:$dst),
                     (ins TILE:$src1, TILE:$src2),
                     "tmulps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PD;
    def TMULPSrm : I<0xe4, MRMSrcMem, (outs TILE:$dst),
                     (ins TILE:$src1, i8mem:$src2),
                     "tmulps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PD;
    let  usesCustomInserter = 1 in {
      def PTMAXPS : PseudoI<(outs), (ins u8imm:$dst,
                            u8imm:$src1, u8imm:$src2),
                            [(int_x86_tmaxps_reg timm:$dst,
                              timm:$src1, timm:$src2)]>;
      def PTMAXPSrm : PseudoI<(outs), (ins u8imm:$dst,
                              u8imm:$src1, i8mem:$src2),
                              [(int_x86_tmaxps_mem timm:$dst,
                                timm:$src1, addr:$src2)]>;
      def PTMINPS : PseudoI<(outs), (ins u8imm:$dst,
                            u8imm:$src1, u8imm:$src2),
                            [(int_x86_tminps_reg timm:$dst,
                              timm:$src1, timm:$src2)]>;
      def PTMINPSrm : PseudoI<(outs), (ins u8imm:$dst,
                              u8imm:$src1, i8mem:$src2),
                              [(int_x86_tminps_mem timm:$dst,
                                timm:$src1, addr:$src2)]>;
      def PTMULPS : PseudoI<(outs), (ins u8imm:$dst,
                            u8imm:$src1, u8imm:$src2),
                            [(int_x86_tmulps_reg timm:$dst,
                              timm:$src1, timm:$src2)]>;
      def PTMULPSrm : PseudoI<(outs), (ins u8imm:$dst,
                              u8imm:$src1, i8mem:$src2),
                              [(int_x86_tmulps_mem timm:$dst,
                                timm:$src1, addr:$src2)]>;
    }
  }
} // HasAMXTILE, HasAMXELEMENT

// TNARROW[B,W]
let Predicates = [HasAMXELEMENT, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TNARROWB : Ii8<0x58, MRMSrcReg, (outs TILE:$dst),
                       (ins TILE:$src1, u8imm:$src2),
                       "tnarrowb\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                       []>, VEX, TAXS;
    def TNARROWW : Ii8<0x59, MRMSrcReg, (outs TILE:$dst),
                       (ins TILE:$src1, u8imm:$src2),
                       "tnarroww\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                       []>, VEX, TAPS;
    let  usesCustomInserter = 1 in {
      def PTNARROWB : PseudoI<(outs), (ins u8imm:$dst,
                              u8imm:$src1, u8imm:$src2),
                              [(int_x86_tnarrowb timm:$dst,
                                timm:$src1, timm:$src2)]>;
      def PTNARROWW : PseudoI<(outs), (ins u8imm:$dst,
                              u8imm:$src1, u8imm:$src2),
                              [(int_x86_tnarroww timm:$dst,
                                timm:$src1, timm:$src2)]>;
    }
  }
} // HasAMXTILE, HasAMXELEMENT

// TORD check tmemrow
let Predicates = [HasAMXELEMENT, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TORDrr : I<0xe4, MRMSrcReg, (outs TILE:$dst),
                   (ins TILE:$src1, TILE:$src2),
                   "tord\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                   []>, VEX_4V, T_MAP5XD;
    def TORDrm : I<0xe4, MRMSrcMem, (outs TILE:$dst),
                   (ins TILE:$src1, i8mem:$src2),
                   "tord\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                   []>, VEX_4V, T_MAP5XD;
    let  usesCustomInserter = 1 in {
      def PTORD : PseudoI<(outs), (ins u8imm:$dst,
                          u8imm:$src1, u8imm:$src2),
                          [(int_x86_tord_reg timm:$dst,
                            timm:$src1, timm:$src2)]>;
      def PTORDrm : PseudoI<(outs), (ins u8imm:$dst,
                            u8imm:$src1, i8mem:$src2),
                            [(int_x86_tord_mem timm:$dst,
                              timm:$src1, addr:$src2)]>;
    }
  }
} // HasAMXTILE, HasAMXELEMENT

// TPERM[B,W,D] check tmemrow
let Predicates = [HasAMXFORMAT, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TPERMBrr : I<0xe6, MRMSrcReg, (outs TILE:$dst),
                     (ins TILE:$src1, TILE:$src2),
                     "tpermb\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5XS;
    def TPERMBrm : I<0xe6, MRMSrcMem, (outs TILE:$dst),
                     (ins TILE:$src1, i8mem:$src2),
                     "tpermb\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5XS;
    def TPERMDrr : I<0xe7, MRMSrcReg, (outs TILE:$dst),
                     (ins TILE:$src1, TILE:$src2),
                     "tpermd\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PD;
    def TPERMDrm : I<0xe7, MRMSrcMem, (outs TILE:$dst),
                     (ins TILE:$src1, i8mem:$src2),
                     "tpermd\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PD;
    def TPERMWrr : I<0xe7, MRMSrcReg, (outs TILE:$dst),
                     (ins TILE:$src1, TILE:$src2),
                     "tpermw\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PS;
    def TPERMWrm : I<0xe7, MRMSrcMem, (outs TILE:$dst),
                     (ins TILE:$src1, i8mem:$src2),
                     "tpermw\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PS;
    let  usesCustomInserter = 1 in {
      def PTPERMB: PseudoI<(outs), (ins u8imm:$dst,
                            u8imm:$src1, u8imm:$src2),
                            [(int_x86_tpermb_reg timm:$dst,
                              timm:$src1, timm:$src2)]>;
      def PTPERMBrm: PseudoI<(outs), (ins u8imm:$dst,
                            u8imm:$src1, i8mem:$src2),
                            [(int_x86_tpermb_mem timm:$dst,
                              timm:$src1, addr:$src2)]>;
      def PTPERMD: PseudoI<(outs), (ins u8imm:$dst,
                            u8imm:$src1, u8imm:$src2),
                            [(int_x86_tpermd_reg timm:$dst,
                              timm:$src1, timm:$src2)]>;
      def PTPERMDrm: PseudoI<(outs), (ins u8imm:$dst,
                            u8imm:$src1, i8mem:$src2),
                            [(int_x86_tpermd_mem timm:$dst,
                              timm:$src1, addr:$src2)]>;
      def PTPERMW: PseudoI<(outs), (ins u8imm:$dst,
                            u8imm:$src1, u8imm:$src2),
                            [(int_x86_tpermw_reg timm:$dst,
                              timm:$src1, timm:$src2)]>;
      def PTPERMWrm: PseudoI<(outs), (ins u8imm:$dst,
                            u8imm:$src1, i8mem:$src2),
                            [(int_x86_tpermw_mem timm:$dst,
                              timm:$src1, addr:$src2)]>;
    }
  }
} // HasAMXTILE, HasAMXFORMAT

// TRCP14PS TREDUCEPS TSCALEFPS check tmemrow
let Predicates = [HasAMXELEMENT, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TRCP14PS : I<0xe4, MRMSrcReg, (outs TILE:$dst),
                     (ins TILE:$src1),
                     "trcp14ps\t{$src1, $dst|$dst, $src1}",
                     []>, VEX, T_MAP5XS;
    def TREDUCEPS : Ii8<0x58, MRMSrcReg, (outs TILE:$dst),
                        (ins TILE:$src1, u8imm:$src2),
                        "treduceps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                        []>, VEX, TAPS;
    def TSCALEFPSrr : I<0xe5, MRMSrcReg, (outs TILE:$dst),
                        (ins TILE:$src1, TILE:$src2),
                        "tscalefps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                        []>, VEX_4V, T_MAP5PS;
    def TSCALEFPSrm : I<0xe5, MRMSrcMem, (outs TILE:$dst),
                        (ins TILE:$src1, i8mem:$src2),
                        "tscalefps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                        []>, VEX_4V, T_MAP5PS;
    let  usesCustomInserter = 1 in {
      def PTRCP14PS : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src1),
                              [(int_x86_trcp14ps timm:$dst, timm:$src1)]>;
      def PTREDUCEPS: PseudoI<(outs), (ins u8imm:$dst,
                              u8imm:$src1, u8imm:$src2),
                              [(int_x86_treduceps timm:$dst,
                                timm:$src1, timm:$src2)]>;
      def PTSCALEFPS: PseudoI<(outs), (ins u8imm:$dst,
                              u8imm:$src1, u8imm:$src2),
                              [(int_x86_tscalefps_reg timm:$dst,
                                timm:$src1, timm:$src2)]>;
      def PTSCALEFPSrm : PseudoI<(outs), (ins u8imm:$dst,
                                 u8imm:$src1, i8mem:$src2),
                                 [(int_x86_tscalefps_mem timm:$dst,
                                   timm:$src1, addr:$src2)]>;
    }
  }
} // HasAMXTILE, HasAMXELEMENT

// TSLLD check tmemrow
let Predicates = [HasAMXELEMENT, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TSLLD : Ii8<0x58, MRMSrcReg, (outs TILE:$dst),
                    (ins TILE:$src1, u8imm:$src2),
                    "tslld\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                    []>, VEX, TAPD;
    def TSRLD : Ii8<0x58, MRMSrcReg, (outs TILE:$dst),
                    (ins TILE:$src1, u8imm:$src2),
                    "tsrld\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                    []>, VEX, TAXD;
    def TSRLVDrr : I<0xe5, MRMSrcReg, (outs TILE:$dst),
                     (ins TILE:$src1, TILE:$src2),
                     "tsrlvd\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PD;
    def TSRLVDrm : I<0xe5, MRMSrcMem, (outs TILE:$dst),
                     (ins TILE:$src1, i8mem:$src2),
                     "tsrlvd\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PD;
    let usesCustomInserter = 1 in {
      def PTSLLD : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src1, u8imm:$src2),
                           [(int_x86_tslld timm:$dst, timm:$src1, timm:$src2)]>;
      def PTSRLD : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src1, u8imm:$src2),
                           [(int_x86_tsrld timm:$dst, timm:$src1, timm:$src2)]>;
      def PTSRLVD : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src1, u8imm:$src2),
                            [(int_x86_tsrlvd_reg timm:$dst, timm:$src1, timm:$src2)]>;
      def PTSRLVDrm : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src1, i8mem:$src2),
                            [(int_x86_tsrlvd_mem timm:$dst, timm:$src1, addr:$src2)]>;
    }
  }
} // HasAMXTILE, HasAMXELEMENT

// TSUBPS check tmemrow
let Predicates = [HasAMXELEMENT, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TSUBPSrr : I<0xe5, MRMSrcReg, (outs TILE:$dst),
                     (ins TILE:$src1, TILE:$src2),
                     "tsubps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5XD;
    def TSUBPSrm : I<0xe5, MRMSrcMem, (outs TILE:$dst),
                     (ins TILE:$src1, i8mem:$src2),
                     "tsubps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5XD;
    let usesCustomInserter = 1 in {
      def PTSUBPS : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src1, u8imm:$src2),
                           [(int_x86_tsubps_reg timm:$dst, timm:$src1, timm:$src2)]>;
      def PTSUBPSrm : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src1, i8mem:$src2),
                           [(int_x86_tsubps_mem timm:$dst, timm:$src1, addr:$src2)]>;
    }
  }
} // HasAMXTILE, HasAMXELEMENT

// TWIDEN[B,W]
let Predicates = [HasAMXFORMAT, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TWIDENB : Ii8<0x59, MRMSrcReg, (outs TILE:$dst),
                      (ins TILE:$src1, u8imm:$src2),
                      "twidenb\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                      []>, VEX, TAPD;
    def TWIDENW : Ii8<0x59, MRMSrcReg, (outs TILE:$dst),
                      (ins TILE:$src1, u8imm:$src2),
                      "twidenw\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                      []>, VEX, TAXD;
    let usesCustomInserter = 1 in {
      def PTWIDENB : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src1, u8imm:$src2),
                             [(int_x86_twidenb timm:$dst, timm:$src1, timm:$src2)]>;
      def PTWIDENW : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src1, u8imm:$src2),
                             [(int_x86_twidenw timm:$dst, timm:$src1, timm:$src2)]>;
    }
  }
} // HasAMXTILE, HasAMXFORMAT

// TXORD check tmemrow
let Predicates = [HasAMXELEMENT, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TXORDrr : I<0xe5, MRMSrcReg, (outs TILE:$dst),
                    (ins TILE:$src1, TILE:$src2),
                    "txord\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                    []>, VEX_4V, T_MAP5XS;
    def TXORDrm : I<0xe5, MRMSrcMem, (outs TILE:$dst),
                    (ins TILE:$src1, i8mem:$src2),
                    "txord\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                    []>, VEX_4V, T_MAP5XS;
    let usesCustomInserter = 1 in {
      def PTXORD : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src1, u8imm:$src2),
                           [(int_x86_txord_reg timm:$dst, timm:$src1, timm:$src2)]>;
      def PTXORDrm : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src1, i8mem:$src2),
                           [(int_x86_txord_mem timm:$dst, timm:$src1, addr:$src2)]>;
    }
  }
} // HasAMXTILE, HasAMXELEMENT

let Predicates = [HasAMXAVX512, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TILE16MOVE : Ii8<0x5f, MRMSrcReg, (outs TILEX:$dst),
                         (ins ZMM16Tuples:$src),
                         "tile16move\t{$src, $dst|$dst, $src}",
                         []>, T8PD, EVEX, EVEX_V512;
    def TILE16MOVEPseudo : PseudoI<(outs TILEX:$dst), (ins ZMM16TUPLES:$src),
                                   []>;
    let  usesCustomInserter = 1 in {
      def PTILE16MOVE : PseudoI<(outs), (ins u8imm:$dst, ZMM16TUPLES:$src0),
                                []>;
    }
  }

  def : Pat<(int_x86_tile16move timm:$dst, VR512:$src0, VR512:$src1,
                                VR512:$src2, VR512:$src3, VR512:$src4,
                                VR512:$src5, VR512:$src6, VR512:$src7,
                                VR512:$src8, VR512:$src9, VR512:$src10,
                                VR512:$src11, VR512:$src12, VR512:$src13,
                                VR512:$src14, VR512:$src15),
            (PTILE16MOVE u8imm:$dst,
             (REG_SEQUENCE ZMM16TUPLES, VR512:$src0, sub_z0,
                                        VR512:$src1, sub_z1,
                                        VR512:$src2, sub_z2,
                                        VR512:$src3, sub_z3,
                                        VR512:$src4, sub_z4,
                                        VR512:$src5, sub_z5,
                                        VR512:$src6, sub_z6,
                                        VR512:$src7, sub_z7,
                                        VR512:$src8, sub_z8,
                                        VR512:$src9, sub_z9,
                                        VR512:$src10, sub_z10,
                                        VR512:$src11, sub_z11,
                                        VR512:$src12, sub_z12,
                                        VR512:$src13, sub_z13,
                                        VR512:$src14, sub_z14,
                                        VR512:$src15, sub_z15))>;
} // HasAMXAVX512, In64BitMode

multiclass m_tilemovrow {
  let Predicates = [HasAMXAVX512, In64BitMode] in {
    let SchedRW = [WriteSystem] in {
      def rri : Ii8<0x7, MRMSrcReg, (outs VR512:$dst),
                    (ins TILEX:$src1, u8imm:$src2),
                    "tilemovrow\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                    []>, TAPD, EVEX, EVEX_V512;
      def rre : I<0x4A, MRMSrcReg4VOp3, (outs VR512:$dst),
                  (ins TILEX:$src1, GR32:$src2),
                  "tilemovrow\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                  []>, T8PD, EVEX_4V, EVEX_V512;
      def rrx: I<0x4A, MRMSrcReg4VOp3, (outs VR512:$dst),
                 (ins TILEX:$src1, VR128X:$src2),
                 "tilemovrow\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, T8PS, EVEX_4V, EVEX_V512;
    }
  } // HasAMXAVX512, In64BitMode
}

defm TILEMOVROW : m_tilemovrow;

let Predicates = [HasAMXAVX512, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    let  usesCustomInserter = 1 in {
      def PTILEMOVROWrri : PseudoI<(outs VR512:$dst), (ins u8imm:$src1, u8imm:$src2),
                                    [(set VR512:$dst, (int_x86_tilemovei timm:$src1,
                                      timm:$src2))]>;
      def PTILEMOVROWrre : PseudoI<(outs VR512:$dst), (ins u8imm:$src1, GR32:$src2),
                                    [(set VR512:$dst, (int_x86_tilemovee timm:$src1,
                                      GR32:$src2))]>;
      def PTILEMOVROWrrx : PseudoI<(outs VR512:$dst), (ins u8imm:$src1, VR128X:$src2),
                                    [(set VR512:$dst, (int_x86_tilemovex timm:$src1,
                                      VR128X:$src2))]>;
    }

// TODO Support x86amx with TILEX, then change TILE --> TILEX.
    def PTILEMOVROWrriV : PseudoI<(outs VR512:$dst),
                                (ins GR16:$src1, GR16:$src2, TILEX:$src3, i32u8imm:$src4),
                                [(set VR512: $dst,
                                 (int_x86_tilemovrowi_internal GR16:$src1, GR16:$src2,
                                  TILEX:$src3, timm:$src4))]>;
    def PTILEMOVROWrreV : PseudoI<(outs VR512:$dst),
                                (ins GR16:$src1, GR16:$src2, TILEX:$src3, GR32:$src4),
                                [(set VR512: $dst,
                                 (int_x86_tilemovrowe_internal GR16:$src1, GR16:$src2,
                                  TILEX:$src3, GR32:$src4))]>;
  }
}
// end INTEL_FEATURE_ISA_AMX_FUTURE

// INTEL_FEATURE_ISA_AMX_LNC
//AMX-MEMORY
// TGATHERROW sibmem2
multiclass m_tgatherrow {
  let Predicates = [HasAMXMEMORY, In64BitMode] in {
    let SchedRW = [WriteSystem] in {
      def D : I<0x6a, MRMSrcMemFSIB, (outs TILE:$dst),
                (ins sibmem:$src),
                "tgatherrowd\t{$src, $dst|$dst, $src}",
                []>, VEX, T8XD;
      def DT1 : I<0x6b, MRMSrcMemFSIB, (outs TILE:$dst),
                  (ins sibmem:$src),
                  "tgatherrowdt1\t{$src, $dst|$dst, $src}",
                  []>, VEX, T8XD;
      def Q : I<0x6a, MRMSrcMemFSIB, (outs TILE:$dst),
                (ins sibmem:$src),
                "tgatherrowq\t{$src, $dst|$dst, $src}",
                []>, VEX, VEX_W, T8XD;
      def QT1 : I<0x6b, MRMSrcMemFSIB, (outs TILE:$dst),
                  (ins sibmem:$src),
                  "tgatherrowqt1\t{$src, $dst|$dst, $src}",
                  []>, VEX, VEX_W, T8XD;
    }
  } // HasAMXTILE, HasAMXMEMORY
}

defm TGATHERROW : m_tgatherrow;

let Predicates = [HasAMXMEMORY, In64BitMode],
    SchedRW = [WriteSystem] in {
  let  usesCustomInserter = 1 in {
    def PTGATHERROWD: PseudoI<(outs), (ins u8imm:$dst, i8mem:$src), []>;
    def PTGATHERROWDT1: PseudoI<(outs), (ins u8imm:$dst, i8mem:$src), []>;
    def PTGATHERROWQ: PseudoI<(outs), (ins u8imm:$dst, i8mem:$src), []>;
    def PTGATHERROWQT1: PseudoI<(outs), (ins u8imm:$dst, i8mem:$src), []>;
  }
} // HasAMXMEMORY

// TSCATTERROW[D, Q][T1] sibmem2
let Predicates = [HasAMXMEMORY, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TSCATTERROWD : I<0x6a, MRMDestMemFSIB, (outs),
                         (ins sibmem:$dst, TILE:$src),
                         "tscatterrowd\t{$src, $dst|$dst, $src}",
                         []>, VEX, T8XS;
    def TSCATTERROWDT1 : I<0x6b, MRMDestMemFSIB, (outs),
                           (ins sibmem:$dst, TILE:$src),
                           "tscatterrowdt1\t{$src, $dst|$dst, $src}",
                           []>, VEX, T8XS;
    def TSCATTERROWQ : I<0x6a, MRMDestMemFSIB, (outs),
                         (ins sibmem:$dst, TILE:$src),
                         "tscatterrowq\t{$src, $dst|$dst, $src}",
                         []>, VEX, VEX_W, T8XS;
    def TSCATTERROWQT1 : I<0x6b, MRMDestMemFSIB, (outs),
                           (ins sibmem:$dst, TILE:$src),
                           "tscatterrowqt1\t{$src, $dst|$dst, $src}",
                           []>, VEX, VEX_W, T8XS;
    let usesCustomInserter = 1 in {
      def PTSCATTERROWD : PseudoI<(outs), (ins sibmem:$dst, u8imm:$src), []>;
      def PTSCATTERROWDT1 : PseudoI<(outs), (ins sibmem:$dst, u8imm:$src), []>;
      def PTSCATTERROWQ : PseudoI<(outs), (ins sibmem:$dst, u8imm:$src), []>;
      def PTSCATTERROWQT1 : PseudoI<(outs), (ins sibmem:$dst, u8imm:$src), []>;
    }
  }
} // HasAMXTILE, HasAMXMEMORY

// AMX-TRANSPOSE
let Predicates = [HasAMXTRANSPOSE, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def T2RPNTLVW : I<0xe9, MRMSrcMem4VOp3FSIB, (outs TILEPair:$dst),
                      (ins sibmem:$src1, GR64:$src2),
                      "t2rpntlvw\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                      []>, VEX, T_MAP5PS;

    def T2RPNTLVWT1 : I<0xe9, MRMSrcMem4VOp3FSIB, (outs TILEPair:$dst),
                        (ins sibmem:$src1, GR64:$src2),
                        "t2rpntlvwt1\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                        []>, VEX, T_MAP5PD;

    def T2RPNTLVWZ0 : I<0x6e, MRMSrcMemFSIB, (outs TILEPair:$dst),
                       (ins sibmem:$src),
                       "t2rpntlvwz0\t{$src, $dst|$dst, $src}",
                       []>, VEX, VEX_WIG, T8PS;

    def T2RPNTLVWZ0T1 : I<0x6f, MRMSrcMemFSIB, (outs TILEPair:$dst),
                         (ins sibmem:$src),
                         "t2rpntlvwz0t1\t{$src, $dst|$dst, $src}",
                         []>, VEX, T8PS;

    def T2RPNTLVWZ1 : I<0x6e, MRMSrcMemFSIB, (outs TILEPair:$dst),
                       (ins sibmem:$src),
                       "t2rpntlvwz1\t{$src, $dst|$dst, $src}",
                       []>, VEX, T8PD;

    def T2RPNTLVWZ1T1 : I<0x6f, MRMSrcMemFSIB, (outs TILEPair:$dst),
                         (ins sibmem:$src),
                         "t2rpntlvwz1t1\t{$src, $dst|$dst, $src}",
                         []>, VEX, T8PD;

    let Constraints = "$src1 = $dst" in
    def TTDPBF16PS : I<0x6c, MRMSrcReg4VOp3, (outs TILE:$dst),
                       (ins TILE:$src1, TILE:$src2, TILE:$src3),
                       "ttdpbf16ps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                       []>, VEX_4V, T8XS;

    let Constraints = "$src1 = $dst" in
      def TTDPFP16PS : I<0x6c, MRMSrcReg4VOp3, (outs TILE:$dst),
                        (ins TILE:$src1, TILE:$src2, TILE:$src3),
                        "ttdpfp16ps\t{$src3, $src2, $src1|$src1, $src2, $src3}",
                        []>, VEX_4V, T8XD;

    def TTRANSPOSED : I<0x5f, MRMSrcReg, (outs TILE:$dst),
                     (ins TILE:$src),
                     "ttransposed\t{$src, $dst|$dst, $src}",
                     []>, VEX, T8XS;

    def PTTRANSPOSEDV : PseudoI<(outs TILE:$dst),
                                (ins GR16:$src1, GR16:$src2, TILE:$src),
                                [(set TILE: $dst,
                                 (int_x86_ttransposed_internal GR16:$src1, GR16:$src2,
                                  TILE:$src))]>;
    let Constraints = "$src4 = $dst" in {
      def PTTDPBF16PSV : PseudoI<(outs TILE:$dst), (ins GR16:$src1,
                                  GR16:$src2, GR16:$src3, TILE:$src4,
                                  TILE:$src5, TILE:$src6),
                                  [(set TILE: $dst,
                                  (int_x86_ttdpbf16ps_internal GR16:$src1, GR16:$src2,
                                   GR16:$src3, TILE:$src4, TILE:$src5, TILE:$src6))]>;
      def PTTDPFP16PSV : PseudoI<(outs TILE:$dst), (ins GR16:$src1,
                                  GR16:$src2, GR16:$src3, TILE:$src4,
                                  TILE:$src5, TILE:$src6),
                                  [(set TILE: $dst,
                                  (int_x86_ttdpfp16ps_internal GR16:$src1, GR16:$src2,
                                   GR16:$src3, TILE:$src4, TILE:$src5, TILE:$src6))]>;
    }

    let usesCustomInserter = 1 in {
      def PT2RPNTLVW : PseudoI<(outs), (ins u8imm:$dst,
                               sibmem:$src1, GR64:$src2), []>;
      def PT2RPNTLVWT1 : PseudoI<(outs), (ins u8imm:$dst,
                                 sibmem:$src1, GR64:$src2), []>;
      def PT2RPNTLVWZ0 : PseudoI<(outs), (ins u8imm:$dst,
                                 sibmem:$src1), []>;
      def PT2RPNTLVWZ0T1 : PseudoI<(outs), (ins u8imm:$dst,
                                   sibmem:$src1), []>;
      def PT2RPNTLVWZ1 : PseudoI<(outs), (ins u8imm:$dst,
                                 sibmem:$src1), []>;
      def PT2RPNTLVWZ1T1 : PseudoI<(outs), (ins u8imm:$dst,
                                   sibmem:$src1), []>;
      def PTTDPBF16PS : PseudoI<(outs), (ins u8imm:$src1,
                                u8imm:$src2, u8imm:$src3),
                                [(int_x86_ttdpbf16ps timm:$src1,
                                  timm:$src2, timm:$src3)]>;
      def PTTDPFP16PS : PseudoI<(outs), (ins u8imm:$src1,
                                u8imm:$src2, u8imm:$src3),
                                [(int_x86_ttdpfp16ps timm:$src1,
                                  timm:$src2, timm:$src3)]>;
      def PTTRANSPOSED : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src),
                                 [(int_x86_ttransposed timm:$dst, timm:$src)]>;
    }
  }
} // HasAMXTILE, HasAMXTRANSPOSE

// end INTEL_FEATURE_ISA_AMX_LNC

// INTEL_FEATURE_ISA_AMX_MEMORY2
// AMX-MEMORY2
// TBROADCASTROWD
let Predicates = [HasAMXMEMORY2, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TBROADCASTROWD : I<0xe9, MRMSrcMem, (outs TILE:$dst),
                           (ins i8mem:$src),
                           "tbroadcastrowd\t{$src, $dst|$dst, $src}",
                           []>, VEX, T_MAP5XD;
    let usesCustomInserter = 1 in {
      def PTBROADCASTROWD : PseudoI<(outs), (ins u8imm:$dst, i8mem:$src),
                                    [(int_x86_tbroadcastrowd timm:$dst, addr:$src)]>;
    }
  }
} // HasAMXTILE, HasAMXMEMORY2

// TSTORE[H, Q]D[T1] sibmem
let Predicates = [HasAMXMEMORY2, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TSTOREHD : I<0xeb, MRMDestMemFSIB, (outs),
                     (ins sibmem:$dst, TILE:$src),
                     "tstorehd\t{$src, $dst|$dst, $src}",
                     []>, VEX, T_MAP5XS;
    def TSTOREHDT1 : I<0xec, MRMDestMemFSIB, (outs),
                       (ins sibmem:$dst, TILE:$src),
                       "tstorehdt1\t{$src, $dst|$dst, $src}",
                       []>, VEX, T_MAP5PS;
    def TSTOREQD : I<0xec, MRMDestMemFSIB, (outs),
                     (ins sibmem:$dst, TILE:$src),
                     "tstoreqd\t{$src, $dst|$dst, $src}",
                     []>, VEX, T_MAP5PD;
    def TSTOREQDT1 : I<0xec, MRMDestMemFSIB, (outs),
                       (ins sibmem:$dst, TILE:$src),
                       "tstoreqdt1\t{$src, $dst|$dst, $src}",
                       []>, VEX, T_MAP5XD;
    def TSTOREROWD : I<0xec, MRMDestMemFSIB, (outs),
                       (ins sibmem:$dst, TILE:$src),
                       "tstorerowd\t{$src, $dst|$dst, $src}",
                       []>, VEX, T_MAP5XS;
    let usesCustomInserter = 1 in {
      def PTSTOREHD : PseudoI<(outs), (ins sibmem:$dst, u8imm:$src), []>;
      def PTSTOREHDT1 : PseudoI<(outs), (ins sibmem:$dst, u8imm:$src), []>;
      def PTSTOREQD : PseudoI<(outs), (ins sibmem:$dst, u8imm:$src), []>;
      def PTSTOREQDT1 : PseudoI<(outs), (ins sibmem:$dst, u8imm:$src), []>;
      def PTSTOREROWD : PseudoI<(outs), (ins sibmem:$dst, u8imm:$src2),
                                [(int_x86_tstorerowd addr:$dst, timm:$src2)]>;
    }
  }
} // HasAMXTILE, HasAMXMEMORY2

// TSTORENTD
let Predicates = [HasAMXMEMORY2, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TSTORENTD : I<0xed, MRMDestMemFSIB, (outs),
                      (ins sibmem:$dst, TILE:$src),
                      "tstorentd\t{$src, $dst|$dst, $src}",
                      []>, VEX, T_MAP5PS;
    let usesCustomInserter = 1 in {
      def PTSTORENTD : PseudoI<(outs), (ins sibmem:$dst, u8imm:$src), []>;
    }
  }
} // HasAMXTILE, HasAMXMEMORY2

// end INTEL_FEATURE_ISA_AMX_MEMORY2

// INTEL_FEATURE_ISA_AMX_BF16_EVEX
//AMX-BF16-EVEX
let Predicates = [HasAMXBF16EVEX, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    let Constraints = "$src1 = $dst" in
    def TDPBF16PSE : I<0x5c, MRMSrcReg4VOp3, (outs TILEX:$dst),
                      (ins TILEX:$src1, TILEX:$src2, TILEX:$src3),
                      "tdpbf16pse\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                      []>, EVEX_4V, T8XS;
    let usesCustomInserter = 1 in {
      // Pseudo instructions, using immediates instead of tile registers.
      // To be translated to the actual instructions in X86ISelLowering.cpp
      def PTDPBF16PSE : PseudoI<(outs), (ins u8imm:$src1,
                                u8imm:$src2, u8imm:$src3),
                                [(int_x86_tdpbf16pse timm:$src1,
                                  timm:$src2, timm:$src3)]>;
    }
  }
} // HasAMXBF16EVEX, In64BitMode

// end INTEL_FEATURE_ISA_AMX_BF16_EVEX

// INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
//AMX-CONVERT-EVEX
let Predicates = [HasAMXELEMENTEVEX, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TCVTD2PSE : I<0xe1, MRMDestMemFSIB, (outs),
                     (ins sibmem:$dst, TILEX:$src),
                     "tcvtd2pse\t{$src, $dst|$dst, $src}",
                     []>, EVEX, T_MAP5XS;
    let usesCustomInserter = 1 in {
      def PTCVTD2PSE : PseudoI<(outs), (ins sibmem:$dst, u8imm:$src), []>;
    }
  }
} // HasAMXTILE, HasAMXCONVERTEVEX

// end INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX

#if INTEL_FEATURE_ISA_AMX_CONVERT
let Predicates = [HasAMXCONVERT, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TCVTD2PSmr : I<0x5f, MRMDestMemFSIB, (outs),
                     (ins sibmem:$dst, TILE:$src),
                     "tcvtd2ps\t{$src, $dst|$dst, $src}",
                     []>, VEX, T8XD;
    let usesCustomInserter = 1 in {
      def PTCVTD2PSmr : PseudoI<(outs), (ins sibmem:$dst, u8imm:$src), []>;
    }

    def TCVT2PS2BF16mr : I<0x5d, MRMDestMem4VOp2FSIB, (outs),
                         (ins sibmem:$dst, TILE:$src1, TILE:$src2),
                         "tcvt2ps2bf16\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                         []>, VEX_4V, VEX_WIG, T8XS;
    let usesCustomInserter = 1 in {
      def PTCVT2PS2BF16mr : PseudoI<(outs),
                                    (ins sibmem:$dst,
                                     u8imm:$src1, u8imm:$src2), []>;
    }

    def TCVT2PS2PHmr : I<0x5d, MRMDestMem4VOp2FSIB, (outs),
                       (ins sibmem:$dst, TILE:$src1, TILE:$src2),
                       "tcvt2ps2ph\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                       []>, VEX_4V, VEX_WIG, T8XD;
    let usesCustomInserter = 1 in {
      def PTCVT2PS2PHmr : PseudoI<(outs),
                                  (ins sibmem:$dst,
                                   u8imm:$src1, u8imm:$src2), []>;
    }

    def TCVTPS2BF16mr : I<0x5d, MRMDestMemFSIB, (outs),
                       (ins sibmem:$dst, TILE:$src),
                       "tcvtps2bf16\t{$src, $dst|$dst, $src}",
                       []>, VEX, T8PD;
    let usesCustomInserter = 1 in {
      def PTCVTPS2BF16mr : PseudoI<(outs), (ins sibmem:$dst, u8imm:$src), []>;
    }

    def TCVTPS2PHmr : I<0x5d, MRMDestMemFSIB, (outs),
                      (ins sibmem:$dst, TILE:$src),
                      "tcvtps2ph\t{$src, $dst|$dst, $src}",
                      []>, VEX, T8PS;
    let usesCustomInserter = 1 in {
      def PTCVTPS2PHmr : PseudoI<(outs), (ins sibmem:$dst, u8imm:$src), []>;
    }
  }
}

#endif // INTEL_FEATURE_ISA_AMX_CONVERT

// INTEL_FEATURE_ISA_AMX_INT8_EVEX
//AMX-INT8-EVEX
let Predicates = [HasAMXINT8EVEX, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    let Constraints = "$src1 = $dst" in {
      def TDPBSSDE : I<0x5e, MRMSrcReg4VOp3, (outs TILEX:$dst),
                      (ins TILEX:$src1, TILEX:$src2, TILEX:$src3),
                      "tdpbssde\t{$src3, $src2, $dst|$dst, $src2, $src3}", []>,
                      EVEX_4V, T8XD;
      def TDPBSUDE : I<0x5e, MRMSrcReg4VOp3, (outs TILEX:$dst),
                      (ins TILEX:$src1, TILEX:$src2, TILEX:$src3),
                      "tdpbsude\t{$src3, $src2, $dst|$dst, $src2, $src3}", []>,
                      EVEX_4V, T8XS;
      def TDPBUSDE : I<0x5e, MRMSrcReg4VOp3, (outs TILEX:$dst),
                      (ins TILEX:$src1, TILEX:$src2, TILEX:$src3),
                      "tdpbusde\t{$src3, $src2, $dst|$dst, $src2, $src3}", []>,
                      EVEX_4V, T8PD;
      def TDPBUUDE : I<0x5e, MRMSrcReg4VOp3, (outs TILEX:$dst),
                      (ins TILEX:$src1, TILEX:$src2, TILEX:$src3),
                      "tdpbuude\t{$src3, $src2, $dst|$dst, $src2, $src3}", []>,
                      EVEX_4V, T8PS;
    }
    let usesCustomInserter = 1 in {
      // Pseudo instructions, using immediates instead of tile registers.
      // To be translated to the actual instructions in X86ISelLowering.cpp
      def PTDPBSSDE : PseudoI<(outs), (ins u8imm:$src1,
                              u8imm:$src2, u8imm:$src3),
                              [(int_x86_tdpbssde timm:$src1,
                                timm:$src2, timm:$src3)]>;
      def PTDPBSUDE : PseudoI<(outs), (ins u8imm:$src1,
                              u8imm:$src2, u8imm:$src3),
                              [(int_x86_tdpbsude timm:$src1,
                                timm:$src2, timm:$src3)]>;
      def PTDPBUSDE : PseudoI<(outs), (ins u8imm:$src1,
                              u8imm:$src2, u8imm:$src3),
                              [(int_x86_tdpbusde timm:$src1,
                                timm:$src2, timm:$src3)]>;
      def PTDPBUUDE : PseudoI<(outs), (ins u8imm:$src1,
                              u8imm:$src2, u8imm:$src3),
                              [(int_x86_tdpbuude timm:$src1,
                                timm:$src2, timm:$src3)]>;
    }
  }
} // HasAMXINT8EVEX, In64BitMode

// end INTEL_FEATURE_ISA_AMX_INT8_EVEX

// INTEL_FEATURE_ISA_AMX_TILE_EVEX
//AMX-TILE-EVEX
let Predicates = [HasAMXTILEEVEX, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TILELOADDE : I<0x4b, MRMSrcMemFSIB, (outs TILEX:$dst),
                      (ins sibmem:$src),
                      "tileloadde\t{$src, $dst|$dst, $src}", []>,
                      EVEX, T8XD;
    def TILELOADDT1E : I<0x4b, MRMSrcMemFSIB, (outs TILEX:$dst),
                        (ins sibmem:$src),
                        "tileloaddt1e\t{$src, $dst|$dst, $src}", []>,
                        EVEX, T8PD;
    def TILESTOREDE : I<0x4b, MRMDestMemFSIB, (outs),
                       (ins sibmem:$dst, TILEX:$src),
                       "tilestorede\t{$src, $dst|$dst, $src}", []>,
                       EVEX, T8XS;
    def TILEZEROE : I<0x49, MRMr0, (outs TILEX:$dst), (ins),
                     "tilezeroe\t$dst", []>,
                     EVEX, T8XD;
    let usesCustomInserter = 1 in {
      // Pseudo instructions, using immediates instead of tile registers.
      // To be translated to the actual instructions in X86ISelLowering.cpp
      def PTILELOADDE : PseudoI<(outs), (ins u8imm:$src1, sibmem:$src2), []>;
      def PTILELOADDT1E : PseudoI<(outs), (ins u8imm:$src1, sibmem:$src2), []>;
      def PTILESTOREDE : PseudoI<(outs), (ins sibmem:$dst, u8imm:$src), []>;
      def PTILEZEROE : PseudoI<(outs), (ins u8imm:$src),
                              [(int_x86_tilezeroe timm:$src)]>;
    }
  } // SchedRW
} // HasAMXTILEEVEX, In64BitMode

let Predicates = [HasAMXTILEEVEX, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TILEMOVE : I<0x4b, MRMSrcReg, (outs TILEX:$dst),
                     (ins TILEX:$src),
                     "tilemove\t{$src, $dst|$dst, $src}",
                     []>, T8XD, EVEX;
    let  usesCustomInserter = 1 in {
      def PTILEMOVE : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src),
                              [(int_x86_tilemove timm:$dst, timm:$src)]>;
    }
  }
} // HasAMXTILEEVEX, In64BitMode

// end INTEL_FEATURE_ISA_AMX_TILE_EVEX

// INTEL_FEATURE_ISA_AMX_TRANSPOSE2
// AMX-TRANSPOSE2
// T2TRANSPOSEW T2TRANSPOSEWT1
let Predicates = [HasAMXTRANSPOSE2, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def T2TRANSPOSEW : I<0xe7, MRMSrcMem4VOp3FSIB, (outs TILEPair:$dst),
                         (ins sibmem:$src1, GR64:$src2),
                         "t2transposew\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                         []>, VEX, T_MAP5XD;

    def T2TRANSPOSEWT1 : I<0xe7, MRMSrcMem4VOp3FSIB, (outs TILEPair:$dst),
                           (ins sibmem:$src1, GR64:$src2),
                           "t2transposewt1\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                           []>, VEX, T_MAP5XS;

  let Constraints = "$src1 = $dst" in {
      def TTDPBSSD : I<0xfe, MRMSrcReg4VOp3, (outs TILE:$dst),
                           (ins TILE:$src1, TILE:$src2, TILE:$src3),
                           "ttdpbssd\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                           []>, VEX_4V, T_MAP5XD;
      def TTDPBSUD : I<0xfe, MRMSrcReg4VOp3, (outs TILE:$dst),
                           (ins TILE:$src1, TILE:$src2, TILE:$src3),
                           "ttdpbsud\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                           []>, VEX_4V, T_MAP5XS;
      def TTDPBUSD : I<0xfe, MRMSrcReg4VOp3, (outs TILE:$dst),
                           (ins TILE:$src1, TILE:$src2, TILE:$src3),
                           "ttdpbusd\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                           []>, VEX_4V, T_MAP5PD;
      def TTDPBUUD : I<0xfe, MRMSrcReg4VOp3, (outs TILE:$dst),
                           (ins TILE:$src1, TILE:$src2, TILE:$src3),
                           "ttdpbuud\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                           []>, VEX_4V, T_MAP5PS;
  }

    let usesCustomInserter = 1 in {
      def PT2TRANSPOSEW : PseudoI<(outs), (ins u8imm:$dst,
                                  sibmem:$src1, GR64:$src2), []>;
      def PT2TRANSPOSEWT1 : PseudoI<(outs), (ins u8imm:$dst,
                                    sibmem:$src1, GR64:$src2), []>;
      def PTTDPBSSD       : PseudoI<(outs), (ins u8imm:$src1,
                                    u8imm:$src2, u8imm:$src3),
                                    [(int_x86_ttdpbssd timm:$src1,
                                     timm:$src2, timm:$src3)]>;
      def PTTDPBSUD       : PseudoI<(outs), (ins u8imm:$src1,
                                    u8imm:$src2, u8imm:$src3),
                                    [(int_x86_ttdpbsud timm:$src1,
                                     timm:$src2, timm:$src3)]>;
      def PTTDPBUSD       : PseudoI<(outs), (ins u8imm:$src1,
                                    u8imm:$src2, u8imm:$src3),
                                    [(int_x86_ttdpbusd timm:$src1,
                                     timm:$src2, timm:$src3)]>;
      def PTTDPBUUD       : PseudoI<(outs), (ins u8imm:$src1,
                                    u8imm:$src2, u8imm:$src3),
                                    [(int_x86_ttdpbuud timm:$src1,
                                     timm:$src2, timm:$src3)]>;
    }
  }
} // HasAMXTILE, HasAMXTRANSPOSE2

let Predicates = [HasAMXTRANSPOSE2, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def T4RQNTLVBZ0   : I<0xee, MRMSrcMemFSIB, (outs TILEQuad:$dst),
                        (ins sibmem:$src),
                        "t4rqntlvbz0\t{$src, $dst|$dst, $src}",
                        []>, VEX, T_MAP5PS;
    def T4RQNTLVBZ0T1 : I<0xef, MRMSrcMemFSIB, (outs TILEQuad:$dst),
                        (ins sibmem:$src),
                        "t4rqntlvbz0t1\t{$src, $dst|$dst, $src}",
                        []>, VEX, T_MAP5PS;
    def T4RQNTLVBZ1   : I<0xee, MRMSrcMemFSIB, (outs TILEQuad:$dst),
                        (ins sibmem:$src),
                        "t4rqntlvbz1\t{$src, $dst|$dst, $src}",
                        []>, VEX, T_MAP5PD;
    def T4RQNTLVBZ1T1 : I<0xef, MRMSrcMemFSIB, (outs TILEQuad:$dst),
                        (ins sibmem:$src),
                        "t4rqntlvbz1t1\t{$src, $dst|$dst, $src}",
                        []>, VEX, T_MAP5PD;
    def T4RQNTLVBZ2   : I<0xee, MRMSrcMemFSIB, (outs TILEQuad:$dst),
                        (ins sibmem:$src),
                        "t4rqntlvbz2\t{$src, $dst|$dst, $src}",
                        []>, VEX, T_MAP5XD;
    def T4RQNTLVBZ2T1 : I<0xef, MRMSrcMemFSIB, (outs TILEQuad:$dst),
                        (ins sibmem:$src),
                        "t4rqntlvbz2t1\t{$src, $dst|$dst, $src}",
                        []>, VEX, T_MAP5XD;
    def T4RQNTLVBZ3   : I<0xee, MRMSrcMemFSIB, (outs TILEQuad:$dst),
                        (ins sibmem:$src),
                        "t4rqntlvbz3\t{$src, $dst|$dst, $src}",
                        []>, VEX, T_MAP5XS;
    def T4RQNTLVBZ3T1 : I<0xef, MRMSrcMemFSIB, (outs TILEQuad:$dst),
                        (ins sibmem:$src),
                        "t4rqntlvbz3t1\t{$src, $dst|$dst, $src}",
                        []>, VEX, T_MAP5XS;

    let usesCustomInserter = 1 in {
      def PT4RQNTLVBZ0   : PseudoI<(outs), (ins u8imm:$dst,
                                 sibmem:$src1), []>;
      def PT4RQNTLVBZ0T1 : PseudoI<(outs), (ins u8imm:$dst,
                                   sibmem:$src1), []>;
      def PT4RQNTLVBZ1   : PseudoI<(outs), (ins u8imm:$dst,
                                 sibmem:$src1), []>;
      def PT4RQNTLVBZ1T1 : PseudoI<(outs), (ins u8imm:$dst,
                                   sibmem:$src1), []>;
      def PT4RQNTLVBZ2   : PseudoI<(outs), (ins u8imm:$dst,
                                 sibmem:$src1), []>;
      def PT4RQNTLVBZ2T1 : PseudoI<(outs), (ins u8imm:$dst,
                                   sibmem:$src1), []>;
      def PT4RQNTLVBZ3   : PseudoI<(outs), (ins u8imm:$dst,
                                 sibmem:$src1), []>;
      def PT4RQNTLVBZ3T1 : PseudoI<(outs), (ins u8imm:$dst,
                                   sibmem:$src1), []>;
    }
  }
}

// end INTEL_FEATURE_ISA_AMX_TRANSPOSE2

#if INTEL_FEATURE_ISA_AMX_TILE2
let Predicates = [HasAMXTILE2, In64BitMode] in {
    def TILEMOV : I<0x4b, MRMSrcReg, (outs TILE:$dst),
                    (ins TILE:$src),
                    "tilemov\t{$src, $dst|$dst, $src}",
                    []>, VEX, T8XD;

let usesCustomInserter = 1 in
    def PTILEMOV : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src),
                           [(int_x86_tilemov timm:$dst, timm:$src)]>;
}
#endif // INTEL_FEATURE_ISA_AMX_TILE2

#if INTEL_FEATURE_ISA_AMX_MEMADVISE
multiclass m_tmovadvise {
  let Predicates = [HasAMXMEMADVISE, In64BitMode] in {
    let SchedRW = [WriteSystem] in {
      def rmi : Ii8<0x12, MRMSrcMemFSIB, (outs TILE:$dst),
                    (ins sibmem:$src1, u8imm:$src2),
                    "tmovadvise\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                    []>, VEX, TAXD;
      def mri : Ii8<0x13, MRMDestMemFSIB, (outs),
                    (ins sibmem:$dst, TILE:$src1, u8imm:$src2),
                    "tmovadvise\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                    []>, VEX, TAXD;
    }
  } // HasAMXTILE, HasAMXMEMADVISZE
}

defm TMOVADVISE : m_tmovadvise;

multiclass ADVISE_BASE {
  let Predicates = [HasAMXMEMADVISE, In64BitMode], SchedRW = [WriteSystem] in {
    def Z0ADVISE : Ii8<0x76, MRMSrcMemFSIB, (outs TILEPair:$dst),
                      (ins sibmem:$src1, i32u8imm:$src2),
                      "t2rpntlvwz0advise\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                      []>, VEX, TAPS;
    def Z1ADVISE : Ii8<0x76, MRMSrcMemFSIB, (outs TILEPair:$dst),
                      (ins sibmem:$src1, i32u8imm:$src2),
                      "t2rpntlvwz1advise\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                      []>, VEX, TAPD;
  }
}

defm T2RPNTLVW : ADVISE_BASE;

let Predicates = [HasAMXMEMADVISE, In64BitMode],
    SchedRW = [WriteSystem] in {
  let  usesCustomInserter = 1 in {
    def PTMOVADVISErmi: PseudoI<(outs), (ins u8imm:$dst, sibmem:$src1, u8imm:$src2), []>;
    def PTMOVADVISEmri: PseudoI<(outs), (ins sibmem:$dst, u8imm:$src1, u8imm:$src2), []>;
    def PT2RPNTLVWZ0ADVISE : PseudoI<(outs), (ins u8imm:$dst, sibmem:$src1, i32u8imm:$src2), []>;
    def PT2RPNTLVWZ1ADVISE : PseudoI<(outs), (ins u8imm:$dst, sibmem:$src1, i32u8imm:$src2), []>;
  }
} // HasAMXMEMADVISE

#endif // INTEL_FEATURE_ISA_AMX_MEMADVISE

#if INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX
multiclass ADVISEE_BASE {
  let Predicates = [HasAMXMEMADVISEEVEX, In64BitMode], SchedRW = [WriteSystem] in {
    def Z0ADVISEE : Ii8<0x76, MRMSrcMemFSIB, (outs TILEPairX:$dst),
                      (ins sibmem:$src1, i32u8imm:$src2),
                      "t2rpntlvwz0advisee\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                      []>, EVEX, TAPS;
    def Z1ADVISEE : Ii8<0x76, MRMSrcMemFSIB, (outs TILEPairX:$dst),
                      (ins sibmem:$src1, i32u8imm:$src2),
                      "t2rpntlvwz1advisee\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                      []>, EVEX, TAPD;
  }
}

defm T2RPNTLVW : ADVISEE_BASE;

let Predicates = [HasAMXMEMADVISEEVEX, In64BitMode],
    SchedRW = [WriteSystem] in {
  let  usesCustomInserter = 1 in {
    def PT2RPNTLVWZ0ADVISEE : PseudoI<(outs), (ins u8imm:$dst, sibmem:$src1, i32u8imm:$src2), []>;
    def PT2RPNTLVWZ1ADVISEE : PseudoI<(outs), (ins u8imm:$dst, sibmem:$src1, i32u8imm:$src2), []>;
  }
} // HasAMXMEMADVISEEVEX
#endif // INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX

#if INTEL_FEATURE_ISA_AMX_COMPLEX
let Predicates = [HasAMXCOMPLEX, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    let Constraints = "$src1 = $dst" in {
      def TCMMIMFP16PS   : I<0x6c, MRMSrcReg4VOp3, (outs TILE:$dst),
                            (ins TILE:$src1, TILE:$src2, TILE:$src3),
                            "tcmmimfp16ps\t{$src3, $src2, $src1|$src1, $src2, $src3}",
                            []>, T8PD, VEX_4V;
      def TCMMRLFP16PS : I<0x6c, MRMSrcReg4VOp3, (outs TILE:$dst),
                            (ins TILE:$src1, TILE:$src2, TILE:$src3),
                            "tcmmrlfp16ps\t{$src3, $src2, $src1|$src1, $src2, $src3}",
                            []>, VEX_4V, VEX_WIG, T8PS;
      def TCONJTCMMIMFP16PS : I<0x6b, MRMSrcReg4VOp3, (outs TILE:$dst),
                            (ins TILE:$src1, TILE:$src2, TILE:$src3),
                            "tconjtcmmimfp16ps\t{$src3, $src2, $src1|$src1, $src2, $src3}",
                            []>, VEX_4V, VEX_WIG, T8PS;
      def TTCMMIMFP16PS : I<0x6b, MRMSrcReg4VOp3, (outs TILE:$dst),
                            (ins TILE:$src1, TILE:$src2, TILE:$src3),
                            "ttcmmimfp16ps\t{$src3, $src2, $src1|$src1, $src2, $src3}",
                            []>, VEX_4V, T8XD;
      def TTCMMRLFP16PS: I<0x6b, MRMSrcReg4VOp3, (outs TILE:$dst),
                           (ins TILE:$src1, TILE:$src2, TILE:$src3),
                           "ttcmmrlfp16ps\t{$src3, $src2, $src1|$src1, $src2, $src3}",
                           []>, VEX_4V, T8XS;

    } // Constraints = "$src1 = $dst"

    let Constraints = "$src4 = $dst" in {
      def PTCMMIMFP16PSV : PseudoI<(outs TILE:$dst), (ins GR16:$src1,
                                  GR16:$src2, GR16:$src3, TILE:$src4,
                                  TILE:$src5, TILE:$src6),
                                  [(set TILE: $dst,
                                  (int_x86_tcmmimfp16ps_internal GR16:$src1, GR16:$src2,
                                   GR16:$src3, TILE:$src4, TILE:$src5, TILE:$src6))]>;
      def PTCMMRLFP16PSV : PseudoI<(outs TILE:$dst), (ins GR16:$src1,
                                  GR16:$src2, GR16:$src3, TILE:$src4,
                                  TILE:$src5, TILE:$src6),
                                  [(set TILE: $dst,
                                  (int_x86_tcmmrlfp16ps_internal GR16:$src1, GR16:$src2,
                                   GR16:$src3, TILE:$src4, TILE:$src5, TILE:$src6))]>;
      def PTCONJTCMMIMFP16PSV : PseudoI<(outs TILE:$dst), (ins GR16:$src1,
                                  GR16:$src2, GR16:$src3, TILE:$src4,
                                  TILE:$src5, TILE:$src6),
                                  [(set TILE: $dst,
                                  (int_x86_tconjtcmmimfp16ps_internal GR16:$src1, GR16:$src2,
                                   GR16:$src3, TILE:$src4, TILE:$src5, TILE:$src6))]>;
      def PTTCMMIMFP16PSV : PseudoI<(outs TILE:$dst), (ins GR16:$src1,
                                  GR16:$src2, GR16:$src3, TILE:$src4,
                                  TILE:$src5, TILE:$src6),
                                  [(set TILE: $dst,
                                  (int_x86_ttcmmimfp16ps_internal GR16:$src1, GR16:$src2,
                                   GR16:$src3, TILE:$src4, TILE:$src5, TILE:$src6))]>;
      def PTTCMMRLFP16PSV : PseudoI<(outs TILE:$dst), (ins GR16:$src1,
                                  GR16:$src2, GR16:$src3, TILE:$src4,
                                  TILE:$src5, TILE:$src6),
                                  [(set TILE: $dst,
                                  (int_x86_ttcmmrlfp16ps_internal GR16:$src1, GR16:$src2,
                                   GR16:$src3, TILE:$src4, TILE:$src5, TILE:$src6))]>;
    }
    def PTCONJTFP16V : PseudoI<(outs TILE:$dst),
                                (ins GR16:$src1, GR16:$src2, TILE:$src3),
                                [(set TILE: $dst,
                                 (int_x86_tconjtfp16_internal GR16:$src1, GR16:$src2,
                                  TILE:$src3))]>;

    def TCONJTFP16   : I<0x6b, MRMSrcReg, (outs TILE:$dst), (ins TILE:$src),
                        "tconjtfp16\t{$src, $dst|$dst, $src}",
                        []>, VEX, T8PD;

    let usesCustomInserter = 1 in {
      def PTCMMIMFP16PS : PseudoI<(outs), (ins u8imm:$src1,
                                u8imm:$src2, u8imm:$src3),
                                [(int_x86_tcmmimfp16ps timm:$src1,
                                  timm:$src2, timm:$src3)]>;
      def PTCMMRLFP16PS : PseudoI<(outs), (ins u8imm:$src1,
                                u8imm:$src2, u8imm:$src3),
                                [(int_x86_tcmmrlfp16ps timm:$src1,
                                  timm:$src2, timm:$src3)]>;
      def PTCONJTCMMIMFP16PS : PseudoI<(outs), (ins u8imm:$src1,
                                u8imm:$src2, u8imm:$src3),
                                [(int_x86_tconjtcmmimfp16ps timm:$src1,
                                  timm:$src2, timm:$src3)]>;
      def PTCONJTFP16 : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src),
                              [(int_x86_tconjtfp16 timm:$dst, timm:$src)]>;
      def PTTCMMIMFP16PS : PseudoI<(outs), (ins u8imm:$src1,
                                u8imm:$src2, u8imm:$src3),
                                [(int_x86_ttcmmimfp16ps timm:$src1,
                                  timm:$src2, timm:$src3)]>;
      def PTTCMMRLFP16PS : PseudoI<(outs), (ins u8imm:$src1,
                                u8imm:$src2, u8imm:$src3),
                                [(int_x86_ttcmmrlfp16ps timm:$src1,
                                  timm:$src2, timm:$src3)]>;
    }
  } // SchedRW = [WriteSystem]
}
#endif // INTEL_FEATURE_ISA_AMX_COMPLEX

#if INTEL_FEATURE_ISA_AMX_TF32
let Predicates = [HasAMXTF32, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    let Constraints = "$src1 = $dst" in {
    def TMMULTF32PS : I<0x6d, MRMSrcReg, (outs TILE:$dst),
                        (ins TILE:$src1, TILE:$src2, TILE:$src3),
                        "tmmultf32ps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                        []>, VEX_4V, T8PD;
    def TTMMULTF32PS : I<0x6d, MRMSrcReg, (outs TILE:$dst),
                         (ins TILE:$src1, TILE:$src2, TILE:$src3),
                         "ttmmultf32ps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                         []>, VEX_4V, T8XS;
    }

    let Constraints = "$src4 = $dst" in {
      def PTMMULTF32PSV : PseudoI<(outs TILE:$dst), (ins GR16:$src1,
                                  GR16:$src2, GR16:$src3, TILE:$src4,
                                  TILE:$src5, TILE:$src6),
                                  [(set TILE: $dst,
                                  (int_x86_tmmultf32ps_internal GR16:$src1, GR16:$src2,
                                   GR16:$src3, TILE:$src4, TILE:$src5, TILE:$src6))]>;
      def PTTMMULTF32PSV : PseudoI<(outs TILE:$dst), (ins GR16:$src1,
                                  GR16:$src2, GR16:$src3, TILE:$src4,
                                  TILE:$src5, TILE:$src6),
                                  [(set TILE: $dst,
                                  (int_x86_ttmmultf32ps_internal GR16:$src1, GR16:$src2,
                                   GR16:$src3, TILE:$src4, TILE:$src5, TILE:$src6))]>;
    }

    let usesCustomInserter = 1 in {
      def PTMMULTF32PS : PseudoI<(outs), (ins u8imm:$src1,
                                 u8imm:$src2, u8imm:$src3),
                                 [(int_x86_tmmultf32ps timm:$src1,
                                   timm:$src2, timm:$src3)]>;
      def PTTMMULTF32PS : PseudoI<(outs), (ins u8imm:$src1,
                                  u8imm:$src2, u8imm:$src3),
                                  [(int_x86_ttmmultf32ps timm:$src1,
                                    timm:$src2, timm:$src3)]>;
    }
  } // SchedRW = [WriteSystem]
}
#endif // INTEL_FEATURE_ISA_AMX_TF32

#if INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
multiclass m_tcvtrowd2ps {
  let Predicates = [HasAMXAVX512CVTROW, In64BitMode] in {
    let SchedRW = [WriteSystem] in {
      def rri : Ii8<0x7, MRMSrcReg, (outs VR512:$dst),
                    (ins TILEX:$src1, i32u8imm:$src2),
                    "tcvtrowd2ps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                    []>, TAXS, EVEX, EVEX_V512;
      def rre : I<0x4A, MRMSrcReg4VOp3, (outs VR512:$dst),
                  (ins TILEX:$src1, GR32:$src2),
                  "tcvtrowd2ps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                  []>, T8XS, EVEX_4V, EVEX_V512;
    }
  } // HasAMXAVX512CVTROW, In64BitMode
}

defm TCVTROWD2PS : m_tcvtrowd2ps;

let Predicates = [HasAMXAVX512CVTROW, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    let  usesCustomInserter = 1 in {
      def PTCVTROWD2PSrri : PseudoI<(outs VR512:$dst), (ins u8imm:$src1, i32u8imm:$src2),
                                    [(set VR512:$dst, (int_x86_tcvtrowd2psi timm:$src1,
                                      timm:$src2))]>;
      def PTCVTROWD2PSrre : PseudoI<(outs VR512:$dst), (ins u8imm:$src1, GR32:$src2),
                                    [(set VR512:$dst, (int_x86_tcvtrowd2pse timm:$src1,
                                      GR32:$src2))]>;
    }

    // TODO Support x86amx with TILEX, then change TILE --> TILEX.
    def PTCVTROWD2PSrriV : PseudoI<(outs VR512:$dst),
                                (ins GR16:$src1, GR16:$src2, TILEX:$src3, i32u8imm:$src4),
                                [(set VR512: $dst,
                                 (int_x86_tcvtrowd2psi_internal GR16:$src1, GR16:$src2,
                                  TILEX:$src3, timm:$src4))]>;
    def PTCVTROWD2PSrreV : PseudoI<(outs VR512:$dst),
                                (ins GR16:$src1, GR16:$src2, TILEX:$src3, GR32:$src4),
                                [(set VR512: $dst,
                                 (int_x86_tcvtrowd2pse_internal GR16:$src1, GR16:$src2,
                                  TILEX:$src3, GR32:$src4))]>;
    def PTCVTROWPS2PBF16HrriV : PseudoI<(outs VR512:$dst),
                                (ins GR16:$src1, GR16:$src2, TILEX:$src3, i32u8imm:$src4),
                                [(set VR512: $dst,
                                 (int_x86_tcvtrowps2pbf16hi_internal GR16:$src1, GR16:$src2,
                                  TILEX:$src3, timm:$src4))]>;
    def PTCVTROWPS2PBF16HrreV : PseudoI<(outs VR512:$dst),
                                (ins GR16:$src1, GR16:$src2, TILEX:$src3, GR32:$src4),
                                [(set VR512: $dst,
                                 (int_x86_tcvtrowps2pbf16he_internal GR16:$src1, GR16:$src2,
                                  TILEX:$src3, GR32:$src4))]>;
    def PTCVTROWPS2PBF16LrriV : PseudoI<(outs VR512:$dst),
                                (ins GR16:$src1, GR16:$src2, TILEX:$src3, i32u8imm:$src4),
                                [(set VR512: $dst,
                                 (int_x86_tcvtrowps2pbf16li_internal GR16:$src1, GR16:$src2,
                                  TILEX:$src3, timm:$src4))]>;
    def PTCVTROWPS2PBF16LrreV : PseudoI<(outs VR512:$dst),
                                (ins GR16:$src1, GR16:$src2, TILEX:$src3, GR32:$src4),
                                [(set VR512: $dst,
                                 (int_x86_tcvtrowps2pbf16le_internal GR16:$src1, GR16:$src2,
                                  TILEX:$src3, GR32:$src4))]>;
    def PTCVTROWPS2PHHrriV : PseudoI<(outs VR512:$dst),
                                (ins GR16:$src1, GR16:$src2, TILEX:$src3, i32u8imm:$src4),
                                [(set VR512: $dst,
                                 (int_x86_tcvtrowps2phhi_internal GR16:$src1, GR16:$src2,
                                  TILEX:$src3, timm:$src4))]>;
    def PTCVTROWPS2PHHrreV : PseudoI<(outs VR512:$dst),
                                (ins GR16:$src1, GR16:$src2, TILEX:$src3, GR32:$src4),
                                [(set VR512: $dst,
                                 (int_x86_tcvtrowps2phhe_internal GR16:$src1, GR16:$src2,
                                  TILEX:$src3, GR32:$src4))]>;
    def PTCVTROWPS2PHLrriV : PseudoI<(outs VR512:$dst),
                                (ins GR16:$src1, GR16:$src2, TILEX:$src3, i32u8imm:$src4),
                                [(set VR512: $dst,
                                 (int_x86_tcvtrowps2phli_internal GR16:$src1, GR16:$src2,
                                  TILEX:$src3, timm:$src4))]>;
    def PTCVTROWPS2PHLrreV : PseudoI<(outs VR512:$dst),
                                (ins GR16:$src1, GR16:$src2, TILEX:$src3, GR32:$src4),
                                [(set VR512: $dst,
                                 (int_x86_tcvtrowps2phle_internal GR16:$src1, GR16:$src2,
                                  TILEX:$src3, GR32:$src4))]>;
  }
}

multiclass AMXAVX512CVTROW_BASE<bits<8> Opcode1, bits<8> Opcode2, string Opstr,
                                Prefix P1, Prefix P2> {
  let Predicates = [HasAMXAVX512CVTROW, In64BitMode] in {
    let OpPrefix = P1 in
      def rre : I<Opcode1, MRMSrcReg4VOp3, (outs VR512:$dst),
                  (ins TILEX:$src1, GR32:$src2),
                  !strconcat(Opstr, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"),
                  []>, EVEX_4V, EVEX_V512, T8;
    let OpPrefix = P2 in
      def rri : Ii8<Opcode2, MRMSrcReg, (outs VR512:$dst),
                    (ins TILEX:$src1, i32u8imm:$src2),
                    !strconcat(Opstr, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"),
                    []>, EVEX, EVEX_V512, TA;
    let usesCustomInserter = 1 in {
      def "P"#NAME#"rre" : PseudoI<(outs VR512:$dst), (ins u8imm:$src1, GR32:$src2),
                       [(set VR512:$dst, (!cast<Intrinsic>("int_x86_"#Opstr#"e")
                        timm:$src1, GR32:$src2))]>;
      def "P"#NAME#"rri" : PseudoI<(outs VR512:$dst), (ins u8imm:$src1, i32u8imm:$src2),
                       [(set VR512:$dst, (!cast<Intrinsic>("int_x86_"#Opstr#"i")
                        timm:$src1, timm:$src2))]>;
    }
  }
}

defm TCVTROWPS2PHH : AMXAVX512CVTROW_BASE<0x6d, 0x07, "tcvtrowps2phh", PS, PS>;
defm TCVTROWPS2PHL : AMXAVX512CVTROW_BASE<0x6d, 0x77, "tcvtrowps2phl", PD, XD>;
defm TCVTROWPS2PBF16H : AMXAVX512CVTROW_BASE<0x6d, 0x07, "tcvtrowps2pbf16h", XD, XD>;
defm TCVTROWPS2PBF16L : AMXAVX512CVTROW_BASE<0x6d, 0x77, "tcvtrowps2pbf16l", XS, XS>;
#endif // INTEL_FEATURE_ISA_AMX_AVX512_CVTROW

#if INTEL_FEATURE_ISA_AMX_SPARSE
let Predicates = [HasAMXSPARSE, In64BitMode], Constraints = "$src1 = $dst" in
let Constraints = "$src1 = $dst" in
class AMX_SPARSE_BASE1<bits<8> Opcode, string Opstr> :
            I<Opcode, MRMSrcReg4VOp3, (outs TILE:$dst),
            (ins TILE:$src1, TILEPair:$src2, TILEPair:$src3),
            !strconcat(Opstr, "\t{$src3, $src2, $dst|$dst, $src2, $src3}"),
            []>, VEX_4V, VEX_W;

def TDPSBF16PS : AMX_SPARSE_BASE1<0x5c, "tdpsbf16ps">, T8XS;
def TDPSFP16PS : AMX_SPARSE_BASE1<0x5c, "tdpsfp16ps">, T8XD;
def TDPSBSSD   : AMX_SPARSE_BASE1<0x5e, "tdpsbssd">, T8XD;
def TDPSBSUD   : AMX_SPARSE_BASE1<0x5e, "tdpsbsud">, T8XS;
def TDPSBUSD   : AMX_SPARSE_BASE1<0x5e, "tdpsbusd">, T8PD;
def TDPSBUUD   : AMX_SPARSE_BASE1<0x5e, "tdpsbuud">, T8PS;

class AMX_SPARSE_PseudoI<string Opstr> :
                  PseudoI<(outs), (ins u8imm:$src1, u8imm:$src2, u8imm:$src3),
                          [(!cast<Intrinsic>("int_x86_"#Opstr) timm:$src1,
                            timm:$src2, timm:$src3)]>;

let usesCustomInserter = 1 in {
  def PTDPSBF16PS : AMX_SPARSE_PseudoI<"tdpsbf16ps">;
  def PTDPSFP16PS : AMX_SPARSE_PseudoI<"tdpsfp16ps">;
  def PTDPSBSSD   : AMX_SPARSE_PseudoI<"tdpsbssd">;
  def PTDPSBSUD   : AMX_SPARSE_PseudoI<"tdpsbsud">;
  def PTDPSBUSD   : AMX_SPARSE_PseudoI<"tdpsbusd">;
  def PTDPSBUUD   : AMX_SPARSE_PseudoI<"tdpsbuud">;
}

let Predicates = [HasAMXSPARSE, In64BitMode] in
class AMX_SPARSE_BASE2<string Opstr> :
        I<0x69, MRMSrcMem4VOp3FSIB, (outs TILE:$dst),
         (ins sibmem:$src1, GR32:$src2),
         !strconcat(Opstr, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"),[]>, VEX_4V;

def TLDEXPANDB :   AMX_SPARSE_BASE2<"tldexpandb">, T8XS;
def TLDEXPANDBT1 : AMX_SPARSE_BASE2<"tldexpandbt1">, T8XD;
def TLDEXPANDW :   AMX_SPARSE_BASE2<"tldexpandw">, T8PS;
def TLDEXPANDWT1 : AMX_SPARSE_BASE2<"tldexpandwt1">, T8PD;

let usesCustomInserter = 1 in {
  def PTLDEXPANDB   : PseudoI<(outs), (ins u8imm:$dst, sibmem:$src1, GR32:$src2), []>;
  def PTLDEXPANDBT1 : PseudoI<(outs), (ins u8imm:$dst, sibmem:$src1, GR32:$src2), []>;
  def PTLDEXPANDW   : PseudoI<(outs), (ins u8imm:$dst, sibmem:$src1, GR32:$src2), []>;
  def PTLDEXPANDWT1 : PseudoI<(outs), (ins u8imm:$dst, sibmem:$src1, GR32:$src2), []>;
}
#endif // INTEL_FEATURE_ISA_AMX_SPARSE

#if INTEL_FEATURE_ISA_AMX_V3
let Predicates = [HasAMXV3, In64BitMode] in
  class AMX_V3_BASE<bits<8> Opcode, string Opstr> :
        I<Opcode, MRMSrcMemFSIB, (outs TILE:$dst), (ins sibmem:$src),
        !strconcat(Opstr, "\t{$src, $dst|$dst, $src}"),
        []>, VEX;

def TLOADTRANSPOSED   : AMX_V3_BASE<0x5f, "tloadtransposed">, T8XS;
def TLOADTRANSPOSEDT1 : AMX_V3_BASE<0x5f, "tloadtransposedt1">, T8PS;
def TRPNTLVWZ0        : AMX_V3_BASE<0x6e, "trpntlvwz0">, T8XD;
def TRPNTLVWZ0T1      : AMX_V3_BASE<0x6f, "trpntlvwz0t1">, T8XD;
def TRPNTLVWZ1        : AMX_V3_BASE<0x6e, "trpntlvwz1">, T8XS;
def TRPNTLVWZ1T1      : AMX_V3_BASE<0x6f, "trpntlvwz1t1">, T8XS;

let Predicates = [HasAMXV3, In64BitMode], usesCustomInserter = 1 in {
  def PTLOADTRANSPOSED   : PseudoI<(outs), (ins u8imm:$src1, sibmem:$src2), []>;
  def PTLOADTRANSPOSEDT1 : PseudoI<(outs), (ins u8imm:$src1, sibmem:$src2), []>;
  def PTRPNTLVWZ0   : PseudoI<(outs), (ins u8imm:$src1, sibmem:$src2), []>;
  def PTRPNTLVWZ0T1 : PseudoI<(outs), (ins u8imm:$src1, sibmem:$src2), []>;
  def PTRPNTLVWZ1   : PseudoI<(outs), (ins u8imm:$src1, sibmem:$src2), []>;
  def PTRPNTLVWZ1T1 : PseudoI<(outs), (ins u8imm:$src1, sibmem:$src2), []>;
}

let Predicates = [HasAMXV3, In64BitMode] in {
  def TSTORETRANSPOSED : I<0x5f, MRMDestMemFSIB, (outs),
                          (ins sibmem:$dst, TILE:$src),
                          "tstoretransposed\t{$src, $dst|$dst, $src}",
                          []>, VEX, T8PD;

  let usesCustomInserter = 1 in
    def PTSTORETRANSPOSED : PseudoI<(outs), (ins sibmem:$dst, u8imm:$src), []>;
}
#endif // INTEL_FEATURE_ISA_AMX_V3
#if INTEL_FEATURE_ISA_AMX_FP8
let Predicates = [HasAMXFP8, In64BitMode], Constraints = "$src1 = $dst" in
  class AMX_FP8_BASE<bits<8> Opcode, string Opstr> :
  I<Opcode, MRMSrcReg4VOp3, (outs TILE:$dst),
    (ins TILE:$src1, TILE:$src2, TILE:$src3),
    !strconcat(Opstr, "\t{$src3, $src2, $dst|$dst, $src2, $src3}"),
    []>, VEX_4V;

def TDPBF8PS : AMX_FP8_BASE<0x5c, "tdpbf8ps">, T8PD;
def TDPBHF8PS : AMX_FP8_BASE<0xfd, "tdpbhf8ps">, T_MAP5XD;
def TDPHBF8PS : AMX_FP8_BASE<0xfd, "tdphbf8ps">, T_MAP5XS;
def TDPHF8PS : AMX_FP8_BASE<0xfd, "tdphf8ps">, T_MAP5PD;

let usesCustomInserter = 1, Predicates = [HasAMXFP8, In64BitMode] in {
  // Pseudo instructions, using immediates instead of tile registers.
  // To be translated to the actual instructions in X86ISelLowering.cpp
  def PTDPBF8PS : PseudoI<(outs), (ins u8imm:$src1,
                           u8imm:$src2, u8imm:$src3),
                           [(int_x86_tdpbf8ps timm:$src1,
                             timm:$src2, timm:$src3)]>;
  def PTDPBHF8PS : PseudoI<(outs), (ins u8imm:$src1,
                           u8imm:$src2, u8imm:$src3),
                           [(int_x86_tdpbhf8ps timm:$src1,
                             timm:$src2, timm:$src3)]>;
  def PTDPHBF8PS : PseudoI<(outs), (ins u8imm:$src1,
                           u8imm:$src2, u8imm:$src3),
                           [(int_x86_tdphbf8ps timm:$src1,
                             timm:$src2, timm:$src3)]>;
  def PTDPHF8PS : PseudoI<(outs), (ins u8imm:$src1,
                           u8imm:$src2, u8imm:$src3),
                           [(int_x86_tdphf8ps timm:$src1,
                             timm:$src2, timm:$src3)]>;
}
#endif // INTEL_FEATURE_ISA_AMX_FP8

#if INTEL_FEATURE_ISA_AMX_FP8_FUTURE
def TTDPBF8PS : AMX_FP8_BASE<0xfc, "ttdpbf8ps">, T_MAP5PS;
def TTDPBHF8PS : AMX_FP8_BASE<0xfc, "ttdpbhf8ps">, T_MAP5XD;
def TTDPHBF8PS : AMX_FP8_BASE<0xfc, "ttdphbf8ps">, T_MAP5XS;
def TTDPHF8PS : AMX_FP8_BASE<0xfc, "ttdphf8ps">, T_MAP5PD;

let usesCustomInserter = 1, Predicates = [HasAMXFP8, In64BitMode] in {
  // Pseudo instructions, using immediates instead of tile registers.
  // To be translated to the actual instructions in X86ISelLowering.cpp
  def PTTDPBF8PS : PseudoI<(outs), (ins u8imm:$src1,
                           u8imm:$src2, u8imm:$src3),
                           [(int_x86_ttdpbf8ps timm:$src1,
                             timm:$src2, timm:$src3)]>;
  def PTTDPBHF8PS : PseudoI<(outs), (ins u8imm:$src1,
                           u8imm:$src2, u8imm:$src3),
                           [(int_x86_ttdpbhf8ps timm:$src1,
                             timm:$src2, timm:$src3)]>;
  def PTTDPHBF8PS : PseudoI<(outs), (ins u8imm:$src1,
                           u8imm:$src2, u8imm:$src3),
                           [(int_x86_ttdphbf8ps timm:$src1,
                             timm:$src2, timm:$src3)]>;
  def PTTDPHF8PS : PseudoI<(outs), (ins u8imm:$src1,
                           u8imm:$src2, u8imm:$src3),
                           [(int_x86_ttdphf8ps timm:$src1,
                             timm:$src2, timm:$src3)]>;
}
#endif // INTEL_FEATURE_ISA_AMX_FP8_FUTURE
