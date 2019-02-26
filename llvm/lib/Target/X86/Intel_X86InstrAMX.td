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
      def PTILELOADD64 : PseudoI<(outs), (ins u8imm:$src1,
                                 GR64:$base, GR64:$stride),
                                 [(int_x86_tileloadd64 imm:$src1,
                                   GR64:$base, GR64:$stride)]>;
      def PTILELOADDT164 : PseudoI<(outs), (ins u8imm:$src1,
                                   GR64:$base, GR64:$stride),
                                   [(int_x86_tileloaddt164 imm:$src1,
                                     GR64:$base, GR64:$stride)]>;
      def PTILESTORED64 : PseudoI<(outs), (ins u8imm:$src1,
                                  GR64:$base, GR64:$stride),
                                  [(int_x86_tilestored64 imm:$src1,
                                    GR64:$base, GR64:$stride)]>;
      def PTILEZERO : PseudoI<(outs), (ins u8imm:$src),
                              [(int_x86_tilezero imm:$src)]>;
    }
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
