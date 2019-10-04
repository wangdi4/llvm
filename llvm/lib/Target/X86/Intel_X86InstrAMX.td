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
    def TILESTORED : I<0x4b, MRMDestMemFSIB, (outs),
                       (ins i8mem:$src1, VTILE:$src2),
                       "tilestored\t{$src2, $src1|$src1, $src2}", []>,
                       VEX, T8XS;
    def TILEZERO : I<0x49, MRMr0, (outs), (ins VTILE:$src),
                     "tilezero\t$src", []>,
                     VEX, T8XD;
  } // SchedRW
} // HasAMXTILE

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
  }
} // HasAMXTILE, HasAMXINT8

let Predicates = [HasAMXBF16, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TDPBF16PS : I<0x5c, MRMSrcReg4VOp3, (outs),
                      (ins VTILE:$src3, VTILE:$src2, VTILE:$src1),
                      "tdpbf16ps\t{$src1, $src2, $src3|$src3, $src2, $src1}",
                      []>, VEX_4V, T8XS;
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

  }
} // HasAMXTILE, HasAMXTRANSPOSE

// TAPPDS check tmemrow
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

// TBROADCASTROWD
let Predicates = [HasAMXMEMORY, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TBROADCASTROWD : I<0xe9, MRMSrcMem, (outs VTILE:$dst),
                           (ins i8mem:$src),
                           "tbroadcastrowd\t{$src, $dst|$dst, $src}",
                           []>, VEX, T_MAP5XD;
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

let Predicates = [HasAMXAVX512, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TILE16MOVE : Ii8<0x5f, MRMSrcReg, (outs VTILEX:$dst),
                        (ins VR512:$src1),
                        "tile16move\t{$src1, $dst|$dst, $src1}",
                        []>, T8PD, EVEX, EVEX_V512;
  }
} // HasAMXAVX512, In64BitMode

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
  }
} // HasAMXINT8EVEX, In64BitMode

let Predicates = [HasAMXBF16EVEX, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TDPBF16PSE : I<0x5c, MRMSrcReg4VOp3, (outs),
                      (ins VTILEX:$src3, VTILEX:$src2, VTILEX:$src1),
                      "tdpbf16pse\t{$src1, $src2, $src3|$src3, $src2, $src1}",
                      []>, EVEX_4V, T8XS;
  }
} // HasAMXBF16EVEX, In64BitMode

let Predicates = [HasAMXTILEEVEX, In64BitMode] in {
  let SchedRW = [WriteSystem] in {
    def TILEMOVE : I<0x4b, MRMSrcReg, (outs VTILEX:$dst),
                        (ins VTILEX:$src1),
                        "tilemove\t{$src1, $dst|$dst, $src1}",
                        []>, T8XD, EVEX;
  }
} // HasAMXTILEEVEX, In64BitMode

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

// end INTEL_FEATURE_ISA_AMX2
