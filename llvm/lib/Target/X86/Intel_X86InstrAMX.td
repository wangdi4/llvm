// INTEL_FEATURE_ISA_AMX //===------------*- tablegen -*-------------------===//
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

let Predicates = [HasAMXTILE, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def LDTILECFG : I <0x49, MRM0m, (outs), (ins i512mem:$src),
                       "ldtilecfg\t$src",
                       [(int_x86_ldtilecfg addr:$src)]>, VEX, T8PS;
    def STTILECFG : I <0x49, MRM0m, (outs), (ins i512mem:$src),
                       "sttilecfg\t$src",
                       [(int_x86_sttilecfg addr:$src)]>, VEX, T8PD;
    def TILELOADD : I<0x4b, MRMSrcMemFSIB, (outs),
                      (ins VTILE:$src1, i8mem:$src2),
                      "tileloadd\t{$src2, $src1|$src1, $src2}", []>,
                      VEX, T8XD;
    def TILELOADDT1 : I<0x4b, MRMSrcMemFSIB, (outs),
                        (ins VTILE:$src1, i8mem:$src2),
                        "tileloaddt1\t{$src2, $src1|$src1, $src2}", []>,
                        VEX, T8PD;
    def TILERELEASE : I<0x49, MRM_C0, (outs), (ins),
                        "tilerelease", [(int_x86_tilerelease)]>, VEX, T8PS;
    def TILESTORED : I<0x4b, MRMSrcMemFSIB, (outs),
                       (ins VTILE:$src1, i8mem:$src2),
                       "tilestored\t{$src1, $src2|$src2, $src1}", []>,
                       VEX, T8XS;
    def TILEZERO : I<0x49, MRMr0, (outs), (ins VTILE:$src),
                     "tilezero\t$src", []>,
                     VEX, T8XD;

    let usesCustomInserter = 1 in {
      // Pseudo instructions, using immediates instead of tile registers.
      // To be translated to the actual instructions in X86ISelLowering.cpp
      def PTILELOADD : PseudoI<(outs), (ins u8imm:$src1, i8mem:$src2), []>;
      def PTILELOADDT1 : PseudoI<(outs), (ins u8imm:$src1, i8mem:$src2), []>;
      def PTILESTORED : PseudoI<(outs), (ins u8imm:$src1, i8mem:$src2), []>;
      def PTILEZERO : PseudoI<(outs), (ins u8imm:$src),
                              [(int_x86_tilezero imm:$src)]>;
    }
  } // SchedRW
} // HasAMXTILE

// FIXME : Change srcdest operand when fix AMX1 code.
let Predicates = [HasAMXINT8, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TDPBSSD : I<0x5e, MRMSrcReg4VOp3, (outs),
                    (ins VTILE:$src3, VTILE:$src2, VTILE:$src1),
                    "tdpbssd\t{$src1, $src2, $src3|$src3, $src2, $src1}", []>,
                    VEX_4V, T8XD;
    def TDPBSUD : I<0x5e, MRMSrcReg4VOp3, (outs),
                    (ins VTILE:$src3, VTILE:$src2, VTILE:$src1),
                    "tdpbsud\t{$src1, $src2, $src3|$src3, $src2, $src1}", []>,
                    VEX_4V, T8XS;
    def TDPBUSD : I<0x5e, MRMSrcReg4VOp3, (outs),
                    (ins VTILE:$src3, VTILE:$src2, VTILE:$src1),
                    "tdpbusd\t{$src1, $src2, $src3|$src3, $src2, $src1}", []>,
                    VEX_4V, T8PD;
    def TDPBUUD : I<0x5e, MRMSrcReg4VOp3, (outs),
                    (ins VTILE:$src3, VTILE:$src2, VTILE:$src1),
                    "tdpbuud\t{$src1, $src2, $src3|$src3, $src2, $src1}", []>,
                    VEX_4V, T8PS;

    let usesCustomInserter = 1 in {
      // Pseudo instructions, using immediates instead of tile registers.
      // To be translated to the actual instructions in X86ISelLowering.cpp
      def PTDPBSSD : PseudoI<(outs), (ins u8imm:$src1,
                             u8imm:$src2, u8imm:$src3),
                             [(int_x86_tdpbssd imm:$src1,
                               imm:$src2, imm:$src3)]>;
      def PTDPBSUD : PseudoI<(outs), (ins u8imm:$src1,
                             u8imm:$src2, u8imm:$src3),
                             [(int_x86_tdpbsud imm:$src1,
                               imm:$src2, imm:$src3)]>;
      def PTDPBUSD : PseudoI<(outs), (ins u8imm:$src1,
                             u8imm:$src2, u8imm:$src3),
                             [(int_x86_tdpbusd imm:$src1,
                               imm:$src2, imm:$src3)]>;
      def PTDPBUUD : PseudoI<(outs), (ins u8imm:$src1,
                             u8imm:$src2, u8imm:$src3),
                             [(int_x86_tdpbuud imm:$src1,
                               imm:$src2, imm:$src3)]>;
    }
  }
} // HasAMXTILE, HasAMXTMUL

let Predicates = [HasAMXBF16, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TDPBF16PS : I<0x5c, MRMSrcReg4VOp3, (outs),
                      (ins VTILE:$src3, VTILE:$src2, VTILE:$src1),
                      "tdpbf16ps\t{$src1, $src2, $src3|$src3, $src2, $src1}",
                      []>, VEX_4V, T8XS;

    let usesCustomInserter = 1 in {
      // Pseudo instructions, using immediates instead of tile registers.
      // To be translated to the actual instructions in X86ISelLowering.cpp
      def PTDPBF16PS : PseudoI<(outs), (ins u8imm:$src1,
                               u8imm:$src2, u8imm:$src3),
                               [(int_x86_tdpbf16ps imm:$src1,
                                 imm:$src2, imm:$src3)]>;
    }
  }
} // HasAMXTILE, HasAMXBF16

// end INTEL_FEATURE_ISA_AMX
// INTEL_FEATURE_ISA_AMX2
// TDPFP16PS
let Predicates = [HasAMXFP16, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    let Constraints = "$src1 = $dst" in {
      def TDPFP16PS : I<0x5c, MRMSrcReg4VOp3, (outs VTILE:$dst),
                        (ins VTILE:$src1, VTILE:$src2, VTILE:$src3),
                        "tdpfp16ps\t{$src3, $src2, $src1|$src1, $src2, $src3}",
                        []>, VEX_4V, T8XD;
    }
    let  usesCustomInserter = 1 in {
      def PTDPFP16PS : PseudoI<(outs), (ins u8imm:$src1,
                               u8imm:$src2, u8imm:$src3),
                               [(int_x86_tdpfp16ps timm:$src1,
                                 timm:$src2, timm:$src3)]>;
    }
  }
} // HasAMXTILE, HasAMXFP16

// T2RPNTLVW T2RPNTLVWT1 T2TRANSPOSEW T2TRANSPOSEWT1
let Predicates = [HasAMXTRANSPOSE, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def T2RPNTLVW : I<0xe9, MRMSrcMem4VOp3FSIB, (outs VTILEPair:$dst),
                      (ins i8mem:$src1, GR64:$src2),
                      "t2rpntlvw\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                      []>, VEX, T_MAP5PS;

    def T2RPNTLVWT1 : I<0xe9, MRMSrcMem4VOp3FSIB, (outs VTILEPair:$dst),
                        (ins i8mem:$src1, GR64:$src2),
                        "t2rpntlvwt1\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                        []>, VEX, T_MAP5PD;

    def T2TRANSPOSEW : I<0xe7, MRMSrcMem4VOp3FSIB, (outs VTILEPair:$dst),
                         (ins i8mem:$src1, GR64:$src2),
                         "t2transposew\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                         []>, VEX, T_MAP5XD;

    def T2TRANSPOSEWT1 : I<0xe7, MRMSrcMem4VOp3FSIB, (outs VTILEPair:$dst),
                           (ins i8mem:$src1, GR64:$src2),
                           "t2transposewt1\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                           []>, VEX, T_MAP5XS;
    let usesCustomInserter = 1 in {
      def PT2RPNTLVW : PseudoI<(outs), (ins u8imm:$dst,
                               i8mem:$src1, GR64:$src2), []>;
      def PT2RPNTLVWT1 : PseudoI<(outs), (ins u8imm:$dst,
                                 i8mem:$src1, GR64:$src2), []>;
      def PT2TRANSPOSEW : PseudoI<(outs), (ins u8imm:$dst,
                                  i8mem:$src1, GR64:$src2), []>;
      def PT2TRANSPOSEWT1 : PseudoI<(outs), (ins u8imm:$dst,
                                    i8mem:$src1, GR64:$src2), []>;
    }
  }
} // HasAMXTILE, HasAMXTRANSPOSE

// TADDPS check tmemrow
multiclass m_tadds {
  let Predicates = [HasAMXELEMENT, In64BitMode] in {
    let SchedRW = [WriteSystem] in {
      def rr : I<0xe0, MRMSrcReg, (outs VTILE:$dst),
                 (ins VTILE:$src1, VTILE:$src2),
                 "taddps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, VEX_4V, T_MAP5PS;
      def rm : I<0xe0, MRMSrcMem, (outs VTILE:$dst),
                 (ins VTILE:$src1, i8mem:$src2),
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
      def rr : I<0xe0, MRMSrcReg, (outs VTILE:$dst),
                 (ins VTILE:$src1, VTILE:$src2),
                 "tandd\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, VEX_4V, T_MAP5PD;
      def rm : I<0xe0, MRMSrcMem, (outs VTILE:$dst),
                 (ins VTILE:$src1, i8mem:$src2),
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
      def rr : I<0xe0, MRMSrcReg, (outs VTILE:$dst),
                 (ins VTILE:$src1, VTILE:$src2),
                 "tandnd\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, VEX_4V, T_MAP5XD;
      def rm : I<0xe0, MRMSrcMem, (outs VTILE:$dst),
                 (ins VTILE:$src1, i8mem:$src2),
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
    def TBLENDVD : I<0xe6, MRMSrcReg, (outs VTILE:$dst),
                     (ins VTILE:$src1, VTILE:$src2, VTILE:$src3),
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

// TBROADCASTROWD
let Predicates = [HasAMXMEMORY, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TBROADCASTROWD : I<0xe9, MRMSrcMem, (outs VTILE:$dst),
                           (ins i8mem:$src),
                           "tbroadcastrowd\t{$src, $dst|$dst, $src}",
                           []>, VEX, T_MAP5XD;
    let usesCustomInserter = 1 in {
      def PTBROADCASTROWD : PseudoI<(outs), (ins u8imm:$dst, i8mem:$src),
                                    [(int_x86_tbroadcastrowd timm:$dst, addr:$src)]>;
    }
  }
} // HasAMXTILE, HasAMXMEMORY

// TCMPPS VCMPPH MRMSrcMem
multiclass m_tcmpps {
  let Predicates = [HasAMXELEMENT, In64BitMode] in {
    let SchedRW = [WriteSystem] in {
      def rr : Ii8<0x59, MRMSrcReg, (outs VTILE:$dst),
                   (ins VTILE:$src1, VTILE:$src2, u8imm:$src3),
                   "tcmpps\t{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                   []>, VEX_4V, TAXS;
      def rm : Ii8<0x59, MRMSrcMem, (outs VTILE:$dst),
                   (ins VTILE:$src1, i8mem:$src2, u8imm:$src3),
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
    def TCOLADDBCASTPS : I<0xe6, MRMSrcReg, (outs VTILE:$dst),
                           (ins VTILE:$src),
                           "tcoladdbcastps\t{$src, $dst|$dst, $src}",
                           []>, VEX, T_MAP5PD;
    def TCOLADDPS : I<0xe6, MRMDestMem, (outs),
                      (ins i8mem:$dst, VTILE:$src),
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
      def B2PS : I<0xe1, MRMSrcReg, (outs VTILE:$dst),
                   (ins VTILE:$src),
                   "tcvtb2ps\t{$src, $dst|$dst, $src}",
                   []>, VEX, T_MAP5PS;
      def BF162PS : I<0xe1, MRMSrcReg, (outs VTILE:$dst),
                      (ins VTILE:$src),
                      "tcvtbf162ps\t{$src, $dst|$dst, $src}",
                      []>, VEX, T_MAP5XD;
      def D2PS : I<0xe1, MRMSrcReg, (outs VTILE:$dst),
                   (ins VTILE:$src),
                   "tcvtd2ps\t{$src, $dst|$dst, $src}",
                   []>, VEX, T_MAP5XS;
      def PS2BF16 : I<0xe2, MRMSrcReg, (outs VTILE:$dst),
                      (ins VTILE:$src),
                      "tcvtps2bf16\t{$src, $dst|$dst, $src}",
                      []>, VEX, T_MAP5XD;
      def PS2BS : I<0xe2, MRMSrcReg, (outs VTILE:$dst),
                    (ins VTILE:$src),
                    "tcvtps2bs\t{$src, $dst|$dst, $src}",
                    []>, VEX, T_MAP5PS;
      def PS2UBS : I<0xe2, MRMSrcReg, (outs VTILE:$dst),
                     (ins VTILE:$src),
                     "tcvtps2ubs\t{$src, $dst|$dst, $src}",
                     []>, VEX, T_MAP5PD;
      def UB2PS : I<0xe1, MRMSrcReg, (outs VTILE:$dst),
                    (ins VTILE:$src),
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
    def TFMADDPSrr : I<0xe2, MRMSrcReg, (outs VTILE:$dst),
                       (ins VTILE:$src1, VTILE:$src2, VTILE:$src3),
                       "tfmaddps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                       []>, VEX_4V, T_MAP5XS;
    def TFMADDPSrm : I<0xe2, MRMSrcMem, (outs VTILE:$dst),
                       (ins VTILE:$src1, VTILE:$src2, i8mem:$src3),
                       "tfmaddps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                       []>, VEX_4V, T_MAP5XS;
    def TFMSUBPSrr : I<0xe3, MRMSrcReg, (outs VTILE:$dst),
                       (ins VTILE:$src1, VTILE:$src2, VTILE:$src3),
                       "tfmsubps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                       []>, VEX_4V, T_MAP5PS;
    def TFMSUBPSrm : I<0xe3, MRMSrcMem, (outs VTILE:$dst),
                       (ins VTILE:$src1, VTILE:$src2, i8mem:$src3),
                       "tfmsubps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                       []>, VEX_4V, T_MAP5PS;
    def TFNMADDPSrr : I<0xe3, MRMSrcReg, (outs VTILE:$dst),
                       (ins VTILE:$src1, VTILE:$src2, VTILE:$src3),
                       "tfnmaddps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                       []>, VEX_4V, T_MAP5PD;
    def TFNMADDPSrm : I<0xe3, MRMSrcMem, (outs VTILE:$dst),
                       (ins VTILE:$src1, VTILE:$src2, i8mem:$src3),
                       "tfnmaddps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                       []>, VEX_4V, T_MAP5PD;
    def TFNMSUBPSrr : I<0xe3, MRMSrcReg, (outs VTILE:$dst),
                       (ins VTILE:$src1, VTILE:$src2, VTILE:$src3),
                       "tfnmsubps\t{$src3, $src2, $dst|$dst, $src2, $src3}",
                       []>, VEX_4V, T_MAP5XD;
    def TFNMSUBPSrm : I<0xe3, MRMSrcMem, (outs VTILE:$dst),
                       (ins VTILE:$src1, VTILE:$src2, i8mem:$src3),
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

// TGATHERROW sibmem2
multiclass m_tgatherrow {
  let Predicates = [HasAMXMEMORY, In64BitMode] in {
    let SchedRW = [WriteSystem] in {
      def D : I<0xe9, MRMSrcMemFSIB, (outs VTILE:$dst),
                (ins i8mem:$src),
                "tgatherrowd\t{$src, $dst|$dst, $src}",
                []>, VEX, T_MAP5XS;
      def DT1 : I<0xea, MRMSrcMemFSIB, (outs VTILE:$dst),
                  (ins i8mem:$src),
                  "tgatherrowdt1\t{$src, $dst|$dst, $src}",
                  []>, VEX, T_MAP5PS;
      def Q : I<0xea, MRMSrcMemFSIB, (outs VTILE:$dst),
                (ins i8mem:$src),
                "tgatherrowq\t{$src, $dst|$dst, $src}",
                []>, VEX, T_MAP5PD;
      def QT1 : I<0xea, MRMSrcMemFSIB, (outs VTILE:$dst),
                  (ins i8mem:$src),
                  "tgatherrowqt1\t{$src, $dst|$dst, $src}",
                  []>, VEX, T_MAP5XD;
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

// TINTERLEAVE
multiclass m_tinterleave {
  let Predicates = [HasAMXFORMAT, In64BitMode] in {
    let SchedRW = [WriteSystem] in {
      def EB : I<0xe8, MRMSrcReg, (outs VTILE:$dst),
                 (ins VTILE:$src1, VTILE:$src2),
                 "tinterleaveeb\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, VEX_4V, T_MAP5XD;
      def EW : I<0xe8, MRMSrcReg, (outs VTILE:$dst),
                 (ins VTILE:$src1, VTILE:$src2),
                 "tinterleaveew\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, VEX_4V, T_MAP5XS;
      def OB : I<0xe8, MRMSrcReg, (outs VTILE:$dst),
                 (ins VTILE:$src1, VTILE:$src2),
                 "tinterleaveob\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, VEX_4V, T_MAP5PS;
      def OW : I<0xe8, MRMSrcReg, (outs VTILE:$dst),
                 (ins VTILE:$src1, VTILE:$src2),
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
    def TMAXPSrr : I<0xe3, MRMSrcReg, (outs VTILE:$dst),
                     (ins VTILE:$src1, VTILE:$src2),
                     "tmaxps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5XS;
    def TMAXPSrm : I<0xe3, MRMSrcMem, (outs VTILE:$dst),
                     (ins VTILE:$src1, i8mem:$src2),
                     "tmaxps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5XS;
    def TMINPSrr : I<0xe4, MRMSrcReg, (outs VTILE:$dst),
                     (ins VTILE:$src1, VTILE:$src2),
                     "tminps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PS;
    def TMINPSrm : I<0xe4, MRMSrcMem, (outs VTILE:$dst),
                     (ins VTILE:$src1, i8mem:$src2),
                     "tminps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PS;
    def TMULPSrr : I<0xe4, MRMSrcReg, (outs VTILE:$dst),
                     (ins VTILE:$src1, VTILE:$src2),
                     "tmulps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PD;
    def TMULPSrm : I<0xe4, MRMSrcMem, (outs VTILE:$dst),
                     (ins VTILE:$src1, i8mem:$src2),
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
    def TNARROWB : Ii8<0x58, MRMSrcReg, (outs VTILE:$dst),
                       (ins VTILE:$src1, u8imm:$src2),
                       "tnarrowb\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                       []>, VEX, TAXS;
    def TNARROWW : Ii8<0x59, MRMSrcReg, (outs VTILE:$dst),
                       (ins VTILE:$src1, u8imm:$src2),
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
    def TORDrr : I<0xe4, MRMSrcReg, (outs VTILE:$dst),
                   (ins VTILE:$src1, VTILE:$src2),
                   "tord\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                   []>, VEX_4V, T_MAP5XD;
    def TORDrm : I<0xe4, MRMSrcMem, (outs VTILE:$dst),
                   (ins VTILE:$src1, i8mem:$src2),
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
    def TPERMBrr : I<0xe6, MRMSrcReg, (outs VTILE:$dst),
                     (ins VTILE:$src1, VTILE:$src2),
                     "tpermb\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5XS;
    def TPERMBrm : I<0xe6, MRMSrcMem, (outs VTILE:$dst),
                     (ins VTILE:$src1, i8mem:$src2),
                     "tpermb\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5XS;
    def TPERMDrr : I<0xe7, MRMSrcReg, (outs VTILE:$dst),
                     (ins VTILE:$src1, VTILE:$src2),
                     "tpermd\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PD;
    def TPERMDrm : I<0xe7, MRMSrcMem, (outs VTILE:$dst),
                     (ins VTILE:$src1, i8mem:$src2),
                     "tpermd\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PD;
    def TPERMWrr : I<0xe7, MRMSrcReg, (outs VTILE:$dst),
                     (ins VTILE:$src1, VTILE:$src2),
                     "tpermw\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PS;
    def TPERMWrm : I<0xe7, MRMSrcMem, (outs VTILE:$dst),
                     (ins VTILE:$src1, i8mem:$src2),
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
    def TRCP14PS : I<0xe4, MRMSrcReg, (outs VTILE:$dst),
                     (ins VTILE:$src1),
                     "trcp14ps\t{$src1, $dst|$dst, $src1}",
                     []>, VEX, T_MAP5XS;
    def TREDUCEPS : Ii8<0x58, MRMSrcReg, (outs VTILE:$dst),
                        (ins VTILE:$src1, u8imm:$src2),
                        "treduceps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                        []>, VEX, TAPS;
    def TSCALEFPSrr : I<0xe5, MRMSrcReg, (outs VTILE:$dst),
                        (ins VTILE:$src1, VTILE:$src2),
                        "tscalefps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                        []>, VEX_4V, T_MAP5PS;
    def TSCALEFPSrm : I<0xe5, MRMSrcMem, (outs VTILE:$dst),
                        (ins VTILE:$src1, i8mem:$src2),
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

// TSCATTERROW[D, Q][T1] sibmem2
let Predicates = [HasAMXMEMORY, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TSCATTERROWD : I<0xea, MRMDestMemFSIB, (outs),
                         (ins i8mem:$dst, VTILE:$src),
                         "tscatterrowd\t{$src, $dst|$dst, $src}",
                         []>, VEX, T_MAP5XS;
    def TSCATTERROWDT1 : I<0xeb, MRMDestMemFSIB, (outs),
                           (ins i8mem:$dst, VTILE:$src),
                           "tscatterrowdt1\t{$src, $dst|$dst, $src}",
                           []>, VEX, T_MAP5PS;
    def TSCATTERROWQ : I<0xeb, MRMDestMemFSIB, (outs),
                         (ins i8mem:$dst, VTILE:$src),
                         "tscatterrowq\t{$src, $dst|$dst, $src}",
                         []>, VEX, T_MAP5PD;
    def TSCATTERROWQT1 : I<0xeb, MRMDestMemFSIB, (outs),
                           (ins i8mem:$dst, VTILE:$src),
                           "tscatterrowqt1\t{$src, $dst|$dst, $src}",
                           []>, VEX, T_MAP5XD;
    let usesCustomInserter = 1 in {
      def PTSCATTERROWD : PseudoI<(outs), (ins i8mem:$dst, u8imm:$src), []>;
      def PTSCATTERROWDT1 : PseudoI<(outs), (ins i8mem:$dst, u8imm:$src), []>;
      def PTSCATTERROWQ : PseudoI<(outs), (ins i8mem:$dst, u8imm:$src), []>;
      def PTSCATTERROWQT1 : PseudoI<(outs), (ins i8mem:$dst, u8imm:$src), []>;
    }
  }
} // HasAMXTILE, HasAMXMEMORY

// TSLLD check tmemrow
let Predicates = [HasAMXELEMENT, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TSLLD : Ii8<0x58, MRMSrcReg, (outs VTILE:$dst),
                    (ins VTILE:$src1, u8imm:$src2),
                    "tslld\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                    []>, VEX, TAPD;
    def TSRLD : Ii8<0x58, MRMSrcReg, (outs VTILE:$dst),
                    (ins VTILE:$src1, u8imm:$src2),
                    "tsrld\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                    []>, VEX, TAXD;
    def TSRLVDrr : I<0xe5, MRMSrcReg, (outs VTILE:$dst),
                     (ins VTILE:$src1, VTILE:$src2),
                     "tsrlvd\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5PD;
    def TSRLVDrm : I<0xe5, MRMSrcMem, (outs VTILE:$dst),
                     (ins VTILE:$src1, i8mem:$src2),
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

// TSTORE[H, Q]D[T1] sibmem
let Predicates = [HasAMXMEMORY, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TSTOREHD : I<0xeb, MRMDestMemFSIB, (outs),
                     (ins i8mem:$dst, VTILE:$src),
                     "tstorehd\t{$src, $dst|$dst, $src}",
                     []>, VEX, T_MAP5XS;
    def TSTOREHDT1 : I<0xec, MRMDestMemFSIB, (outs),
                       (ins i8mem:$dst, VTILE:$src),
                       "tstorehdt1\t{$src, $dst|$dst, $src}",
                       []>, VEX, T_MAP5PS;
    def TSTOREQD : I<0xec, MRMDestMemFSIB, (outs),
                     (ins i8mem:$dst, VTILE:$src),
                     "tstoreqd\t{$src, $dst|$dst, $src}",
                     []>, VEX, T_MAP5PD;
    def TSTOREQDT1 : I<0xec, MRMDestMemFSIB, (outs),
                       (ins i8mem:$dst, VTILE:$src),
                       "tstoreqdt1\t{$src, $dst|$dst, $src}",
                       []>, VEX, T_MAP5XD;
    def TSTORENTD : I<0xed, MRMDestMemFSIB, (outs),
                      (ins i8mem:$dst, VTILE:$src),
                      "tstorentd\t{$src, $dst|$dst, $src}",
                      []>, VEX, T_MAP5PS;
    def TSTOREROWD : I<0xec, MRMDestMemFSIB, (outs),
                       (ins i8mem:$dst, VTILE:$src),
                       "tstorerowd\t{$src, $dst|$dst, $src}",
                       []>, VEX, T_MAP5XS;
    let usesCustomInserter = 1 in {
      def PTSTOREHD : PseudoI<(outs), (ins i8mem:$dst, u8imm:$src), []>;
      def PTSTOREHDT1 : PseudoI<(outs), (ins i8mem:$dst, u8imm:$src), []>;
      def PTSTOREQD : PseudoI<(outs), (ins i8mem:$dst, u8imm:$src), []>;
      def PTSTOREQDT1 : PseudoI<(outs), (ins i8mem:$dst, u8imm:$src), []>;
      def PTSTORENTD : PseudoI<(outs), (ins i8mem:$dst, u8imm:$src), []>;
      def PTSTOREROWD : PseudoI<(outs), (ins i8mem:$dst, u8imm:$src2),
                                [(int_x86_tstorerowd addr:$dst, timm:$src2)]>;
    }
  }
} // HasAMXTILE, HasAMXMEMORY

// TSUBPS check tmemrow
let Predicates = [HasAMXELEMENT, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TSUBPSrr : I<0xe5, MRMSrcReg, (outs VTILE:$dst),
                     (ins VTILE:$src1, VTILE:$src2),
                     "tsubps\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     []>, VEX_4V, T_MAP5XD;
    def TSUBPSrm : I<0xe5, MRMSrcMem, (outs VTILE:$dst),
                     (ins VTILE:$src1, i8mem:$src2),
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
    def TWIDENB : Ii8<0x59, MRMSrcReg, (outs VTILE:$dst),
                      (ins VTILE:$src1, u8imm:$src2),
                      "twidenb\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                      []>, VEX, TAPD;
    def TWIDENW : Ii8<0x59, MRMSrcReg, (outs VTILE:$dst),
                      (ins VTILE:$src1, u8imm:$src2),
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
    def TXORDrr : I<0xe5, MRMSrcReg, (outs VTILE:$dst),
                    (ins VTILE:$src1, VTILE:$src2),
                    "txord\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                    []>, VEX_4V, T_MAP5XS;
    def TXORDrm : I<0xe5, MRMSrcMem, (outs VTILE:$dst),
                    (ins VTILE:$src1, i8mem:$src2),
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

let Predicates = [HasAMXTILEEVEX, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TILELOADDE : I<0x4b, MRMSrcMemFSIB, (outs),
                      (ins VTILEX:$src1, i8mem:$src2),
                      "tileloadde\t{$src2, $src1|$src1, $src2}", []>,
                      EVEX, T8XD;
    def TILELOADDT1E : I<0x4b, MRMSrcMemFSIB, (outs),
                        (ins VTILEX:$src1, i8mem:$src2),
                        "tileloaddt1e\t{$src2, $src1|$src1, $src2}", []>,
                        EVEX, T8PD;
    def TILESTOREDE : I<0x4b, MRMSrcMemFSIB, (outs),
                       (ins VTILEX:$src1, i8mem:$src2),
                       "tilestorede\t{$src1, $src2|$src2, $src1}", []>,
                       EVEX, T8XS;
    def TILEZEROE : I<0x49, MRMr0, (outs), (ins VTILEX:$src),
                     "tilezeroe\t$src", []>,
                     EVEX, T8XD;
    let usesCustomInserter = 1 in {
      // Pseudo instructions, using immediates instead of tile registers.
      // To be translated to the actual instructions in X86ISelLowering.cpp
      def PTILELOADDE : PseudoI<(outs), (ins u8imm:$src1, i8mem:$src2), []>;
      def PTILELOADDT1E : PseudoI<(outs), (ins u8imm:$src1, i8mem:$src2), []>;
      def PTILESTOREDE : PseudoI<(outs), (ins u8imm:$src1, i8mem:$src2), []>;
      def PTILEZEROE : PseudoI<(outs), (ins u8imm:$src),
                              [(int_x86_tilezeroe timm:$src)]>;
    }
  } // SchedRW
} // HasAMXTILEEVEX, In64BitMode

let Predicates = [HasAMXINT8EVEX, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TDPBSSDE : I<0x5e, MRMSrcReg4VOp3, (outs),
                    (ins VTILEX:$src3, VTILEX:$src2, VTILEX:$src1),
                    "tdpbssde\t{$src1, $src2, $src3|$src3, $src2, $src1}", []>,
                    EVEX_4V, T8XD;
    def TDPBSUDE : I<0x5e, MRMSrcReg4VOp3, (outs),
                    (ins VTILEX:$src3, VTILEX:$src2, VTILEX:$src1),
                    "tdpbsude\t{$src1, $src2, $src3|$src3, $src2, $src1}", []>,
                    EVEX_4V, T8XS;
    def TDPBUSDE : I<0x5e, MRMSrcReg4VOp3, (outs),
                    (ins VTILEX:$src3, VTILEX:$src2, VTILEX:$src1),
                    "tdpbusde\t{$src1, $src2, $src3|$src3, $src2, $src1}", []>,
                    EVEX_4V, T8PD;
    def TDPBUUDE : I<0x5e, MRMSrcReg4VOp3, (outs),
                    (ins VTILEX:$src3, VTILEX:$src2, VTILEX:$src1),
                    "tdpbuude\t{$src1, $src2, $src3|$src3, $src2, $src1}", []>,
                    EVEX_4V, T8PS;
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

let Predicates = [HasAMXBF16EVEX, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TDPBF16PSE : I<0x5c, MRMSrcReg4VOp3, (outs),
                      (ins VTILEX:$src3, VTILEX:$src2, VTILEX:$src1),
                      "tdpbf16pse\t{$src1, $src2, $src3|$src3, $src2, $src1}",
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

let Predicates = [HasAMXTILEEVEX, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TILEMOVE : I<0x4b, MRMSrcReg, (outs VTILEX:$dst),
                     (ins VTILEX:$src),
                     "tilemove\t{$src, $dst|$dst, $src}",
                     []>, T8XD, EVEX;
    let  usesCustomInserter = 1 in {
      def PTILEMOVE : PseudoI<(outs), (ins u8imm:$dst, u8imm:$src),
                              [(int_x86_tilemove timm:$dst, timm:$src)]>;
    }
  }
} // HasAMXTILEEVEX, In64BitMode

let Predicates = [HasAMXAVX512, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TILE16MOVE : Ii8<0x5f, MRMSrcReg, (outs VTILEX:$dst),
                         (ins ZMM16Tuples:$src),
                         "tile16move\t{$src, $dst|$dst, $src}",
                         []>, T8PD, EVEX, EVEX_V512;
    def TILE16MOVEPseudo : PseudoI<(outs VTILEX:$dst), (ins ZMM16TUPLES:$src),
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

multiclass m_tilemovrowe {
  let Predicates = [HasAMXAVX512, In64BitMode] in {
    let SchedRW = [WriteSystem] in {
      def rri : Ii8<0x7, MRMSrcReg, (outs VR512:$dst),
                    (ins VTILEX:$src1, u8imm:$src2),
                    "tilemovrowe\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                    []>, TAPD, EVEX, EVEX_V512;
      def rre : I<0x4A, MRMSrcReg4VOp3, (outs VR512:$dst),
                  (ins VTILEX:$src1, GR32:$src2),
                  "tilemovrowe\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                  []>, T8PD, EVEX_4V, EVEX_V512;
      def rrx: I<0x4A, MRMSrcReg4VOp3, (outs VR512:$dst),
                 (ins VTILEX:$src1, VR128X:$src2),
                 "tilemovrowe\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                 []>, T8PS, EVEX_4V, EVEX_V512;
    }
  } // HasAMXAVX512, In64BitMode
}

defm TILEMOVROWE : m_tilemovrowe;

let Predicates = [HasAMXAVX512, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    let  usesCustomInserter = 1 in {
      def PTILEMOVROWErri : PseudoI<(outs VR512:$dst), (ins u8imm:$src1, u8imm:$src2),
                                    [(set VR512:$dst, (int_x86_tilemovei timm:$src1,
                                      timm:$src2))]>;
      def PTILEMOVROWErre : PseudoI<(outs VR512:$dst), (ins u8imm:$src1, GR32:$src2),
                                    [(set VR512:$dst, (int_x86_tilemovee timm:$src1,
                                      GR32:$src2))]>;
      def PTILEMOVROWErrx : PseudoI<(outs VR512:$dst), (ins u8imm:$src1, VR128X:$src2),
                                    [(set VR512:$dst, (int_x86_tilemovex timm:$src1,
                                      VR128X:$src2))]>;
    }
  }
}

// end INTEL_FEATURE_ISA_AMX2
