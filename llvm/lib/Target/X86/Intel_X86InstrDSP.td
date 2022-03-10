#if INTEL_FEATURE_ISA_DSPV1
//===---------------------*- tablegen -*-----------------------------------===//
//=--- Intel_X86InstrDSP.td - DSP Instruction Set Extension -*- tablegen ---*=//
//
//      Copyright (c) 2018-2021 Intel Corporation.
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
// This file describes the instructions that make up the Intel DSP instruction
// set.
//
//===----------------------------------------------------------------------===//

let Predicates = [Not64BitMode, HasDSPV1] in {
  multiclass dspv1_2op_imm8_w<bits<8> Opcode, string OpcodeStr,
              PatFrag MemOpFrag> {
      def rr : Ii8<Opcode, MRMSrcReg, (outs VR128:$dst),
               (ins VR128:$src1, i32u8imm:$src2),
               !strconcat(OpcodeStr, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"),
               [(set VR128:$dst,
                (!cast<Intrinsic>("int_x86_"#OpcodeStr)
                 VR128:$src1, timm:$src2))]>, VEX;
      def rm : Ii8<Opcode, MRMSrcMem, (outs VR128:$dst),
               (ins i128mem:$src1, i32u8imm:$src2),
               !strconcat(OpcodeStr, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"),
               [(set VR128:$dst,
                (!cast<Intrinsic>("int_x86_"#OpcodeStr)
                 (MemOpFrag addr:$src1), timm:$src2))]>, VEX;
  }

  multiclass dspv1_2op_w<bits<8> Opcode, string OpcodeStr, PatFrag MemOpFrag> {
      def rr : I<Opcode, MRMSrcReg, (outs VR128:$dst),
               (ins VR128:$src),
               !strconcat(OpcodeStr, "\t{$src, $dst|$dst, $src}"),
               [(set VR128:$dst,
                (!cast<Intrinsic>("int_x86_"#OpcodeStr) VR128:$src))]>, VEX;
      def rm : I<Opcode, MRMSrcMem, (outs VR128:$dst),
               (ins i128mem:$src),
               !strconcat(OpcodeStr, "\t{$src, $dst|$dst, $src}"),
               [(set VR128:$dst,
                (!cast<Intrinsic>("int_x86_"#OpcodeStr) (MemOpFrag addr:$src)))]>,
                VEX;
  }

  multiclass dspv1_3op_imm8_w<bits<8> Opcode, string OpcodeStr,
              PatFrag MemOpFrag> {
      def rr : Ii8<Opcode, MRMSrcReg, (outs VR128:$dst),
               (ins VR128:$src1, VR128:$src2, i32u8imm:$src3),
               !strconcat(OpcodeStr, "\t{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}"),
               [(set VR128:$dst,
                (!cast<Intrinsic>("int_x86_"#OpcodeStr)
                 VR128:$src1, VR128:$src2, timm:$src3))]>, VEX_4V;
      def rm : Ii8<Opcode, MRMSrcMem, (outs VR128:$dst),
               (ins VR128:$src1, i128mem:$src2, i32u8imm:$src3),
               !strconcat(OpcodeStr, "\t{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}"),
               [(set VR128:$dst,
                (!cast<Intrinsic>("int_x86_"#OpcodeStr)
                 VR128:$src1, (MemOpFrag addr:$src2), timm:$src3))]>, VEX_4V;
  }

  multiclass dspv1_3op_imm8_rw<bits<8> Opcode, string OpcodeStr,
             PatFrag MemOpFrag> {
    let Constraints = "$src1 = $dst" in {
      def rr : Ii8<Opcode, MRMSrcReg, (outs VR128:$dst),
               (ins VR128:$src1, VR128:$src2, VR128:$src3, i32u8imm:$src4),
               !strconcat(OpcodeStr, "\t{$src4, $src3, $src2, $dst|$dst, $src2, $src3, $src4}"),
               [(set VR128:$dst,
                (!cast<Intrinsic>("int_x86_"#OpcodeStr) VR128:$src1,
                VR128:$src2, VR128:$src3, timm:$src4))]>, VEX_4V;
      def rm : Ii8<Opcode, MRMSrcMem, (outs VR128:$dst),
               (ins VR128:$src1, VR128:$src2, i128mem:$src3, i32u8imm:$src4),
               !strconcat(OpcodeStr, "\t{$src4, $src3, $src2, $dst|$dst, $src2, $src3, $src4}"),
               [(set VR128:$dst,
                (!cast<Intrinsic>("int_x86_"#OpcodeStr) VR128:$src1,
                VR128:$src2, (MemOpFrag addr:$src3), timm:$src4))]>, VEX_4V;
    }
  }

  multiclass dspv1_3op_w<bits<8> Opcode, string OpcodeStr, PatFrag MemOpFrag> {
      def rr : I<Opcode, MRMSrcReg, (outs VR128:$dst),
               (ins VR128:$src1, VR128:$src2),
               !strconcat(OpcodeStr, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"),
               [(set VR128:$dst,
                (!cast<Intrinsic>("int_x86_"#OpcodeStr)
                 VR128:$src1, VR128:$src2))]>, VEX_4V;
      def rm : I<Opcode, MRMSrcMem, (outs VR128:$dst),
               (ins VR128:$src1, i128mem:$src2),
               !strconcat(OpcodeStr, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"),
               [(set VR128:$dst,
                (!cast<Intrinsic>("int_x86_"#OpcodeStr)
                 VR128:$src1, (MemOpFrag addr:$src2)))]>, VEX_4V;
  }

  multiclass dspv1_3op_rw<bits<8> Opcode, string OpcodeStr, PatFrag MemOpFrag> {
      let Constraints = "$src1 = $dst" in {
       def rr : I<Opcode, MRMSrcReg, (outs VR128:$dst),
                (ins VR128:$src1, VR128:$src2, VR128:$src3),
                !strconcat(OpcodeStr, "\t{$src3, $src2, $dst|$dst, $src2, $src3}"),
                [(set VR128:$dst,
                 (!cast<Intrinsic>("int_x86_"#OpcodeStr) VR128:$src1,
                  VR128:$src2, VR128:$src3))]>, VEX_4V;
       def rm : I<Opcode, MRMSrcMem, (outs VR128:$dst),
                (ins VR128:$src1, VR128:$src2, i128mem:$src3),
                !strconcat(OpcodeStr, "\t{$src3, $src2, $dst|$dst, $src2, $src3}"),
                [(set VR128:$dst,
                 (!cast<Intrinsic>("int_x86_"#OpcodeStr) VR128:$src1,
                  VR128:$src2, (MemOpFrag addr:$src3)))]>, VEX_4V;
      }
  }
} // let Predicates = [Not64BitMode, HasDSPV1]

let Uses = [MXCSR] in
  defm DVPCR2BFRSW : dspv1_3op_imm8_rw<0xa3, "dvpcr2bfrsw", loadv8i16>,
                     TAXD, VEX_W;

defm DVPLUTSINCOSW : dspv1_2op_imm8_w<0xa3, "dvplutsincosw", loadv8i16>, TAPS;

// Chapter 2
defm DVPMULWS : dspv1_3op_w<0xcd, "dvpmulws", loadv8i16>, T_MAP8PS, VEX_W;
let Uses = [MXCSR] in {
  defm DVPMULUWR : dspv1_3op_w<0xc8, "dvpmuluwr", loadv8i16>, T_MAP8PS, VEX_W;
  defm DVPMULWR : dspv1_3op_w<0xc8, "dvpmulwr", loadv8i16>, T_MAP8PD, VEX_W;
  defm DVPMULWFRS : dspv1_3op_w<0xc9, "dvpmulwfrs", loadv8i16>, T_MAP8XD, VEX_W;
}
// end Chapter 2

// Chapter 3
defm DVPDPWUUQ : dspv1_3op_rw<0xc0, "dvpdpwuuq", loadv8i16>, T_MAP8PS, VEX_W;
defm DVPDPWSSQ : dspv1_3op_rw<0xc0, "dvpdpwssq", loadv8i16>, T_MAP8XD, VEX_W;
defm DVPDPWSUQ : dspv1_3op_rw<0xc0, "dvpdpwsuq", loadv8i16>, T_MAP8XS, VEX_W;
defm DVPDPWUSQ : dspv1_3op_rw<0xc0, "dvpdpwusq", loadv8i16>, T_MAP8PD, VEX_W;
defm DVPNDPWSSQ : dspv1_3op_rw<0xdd, "dvpndpwssq", loadv8i16>, T_MAP8PS, VEX_W;
defm DVPCMULWRS : dspv1_3op_imm8_w<0xa5, "dvpcmulwrs", loadv8i16>, TAPS, VEX_W;
defm DVPCCMULWRS : dspv1_3op_imm8_w<0xa5, "dvpccmulwrs", loadv8i16>, TAPD, VEX_W;
defm DVPCDPWQRE : dspv1_3op_rw<0xd6, "dvpcdpwqre", loadv8i16>, T_MAP8XD, VEX_W;
defm DVPCDPWQIMM : dspv1_3op_rw<0xd6, "dvpcdpwqimm", loadv8i16>, T_MAP8XS, VEX_W;
defm DVPNCDPWQRE : dspv1_3op_rw<0xd4, "dvpncdpwqre", loadv8i16>, T_MAP8XD, VEX_W;
defm DVPNCDPWQIMM : dspv1_3op_rw<0xd4, "dvpncdpwqimm", loadv8i16>, T_MAP8XS, VEX_W;
defm DVPCCDPWQIMM : dspv1_3op_rw<0xd7, "dvpccdpwqimm", loadv8i16>, T_MAP8XS, VEX_W;
defm DVPNCCDPWQIMM : dspv1_3op_rw<0xd5, "dvpnccdpwqimm", loadv8i16>, T_MAP8XS, VEX_W;
// end Chapter 3

// Chapter 4
defm DVPDPBWUUD  : dspv1_3op_imm8_rw<0xa8, "dvpdpbwuud", loadv8i16>, TAPS;
defm DVPDPBWUUDS : dspv1_3op_imm8_rw<0xa9, "dvpdpbwuuds", loadv8i16>, TAPS;
defm DVPDPBWSSD  : dspv1_3op_imm8_rw<0xa8, "dvpdpbwssd", loadv8i16>, TAXD;
defm DVPDPBWSSDS : dspv1_3op_imm8_rw<0xa9, "dvpdpbwssds", loadv8i16>, TAXD;
defm DVPDPBWSUD  : dspv1_3op_imm8_rw<0xa8, "dvpdpbwsud", loadv8i16>, TAXS;
defm DVPDPBWSUDS : dspv1_3op_imm8_rw<0xa9, "dvpdpbwsuds", loadv8i16>, TAXS;
defm DVPDPBWUSD  : dspv1_3op_imm8_rw<0xa8, "dvpdpbwusd", loadv8i16>, TAPD;
defm DVPDPBWUSDS : dspv1_3op_imm8_rw<0xa9, "dvpdpbwusds", loadv8i16>, TAPD;
// end Chapter 4

// Chapter 5
defm DVPMULUWDQ     : dspv1_3op_imm8_w<0xa4, "dvpmuluwdq", loadv4i32>, TAXD, VEX_W;
defm DVPMULWDQ      : dspv1_3op_imm8_w<0xa4, "dvpmulwdq", loadv4i32>, TAXS, VEX_W;
defm DVPDPWDUUQ     : dspv1_3op_imm8_rw<0xa6, "dvpdpwduuq", loadv4i32>, TAXS, VEX_W;
defm DVPDPWDSSQ     : dspv1_3op_imm8_rw<0xa6, "dvpdpwdssq", loadv4i32>, TAXD, VEX_W;
defm DVPCR2BFRSDRE  : dspv1_3op_imm8_rw<0xa5, "dvpcr2bfrsdre", loadv8i16>, TAXD;
defm DVPCR2BFRSDIMM : dspv1_3op_imm8_rw<0xa5, "dvpcr2bfrsdimm", loadv8i16>, TAXS;
// end Chapter 5

// Chapter 6
defm DVPMULUDHHQ : dspv1_3op_w<0xcb, "dvpmuludhhq", loadv4i32>,
                   T_MAP8XS, VEX_W;
defm DVPMULDHHQ : dspv1_3op_w<0xcc, "dvpmuldhhq", loadv4i32>,
                  T_MAP8XS, VEX_W;
defm DVPMULUDLLQ : dspv1_3op_w<0xcb, "dvpmuludllq", loadv4i32>, T_MAP8XD, VEX_W;
defm DVPMULDLLQ  : dspv1_3op_w<0xcc, "dvpmuldllq", loadv4i32>, T_MAP8XD, VEX_W;
defm DVPMULDFRS : dspv1_3op_w<0xca, "dvpmuldfrs", loadv4i32>, T_MAP8XD;
defm DVPMULDS : dspv1_3op_w<0xcd, "dvpmulds", loadv4i32>, T_MAP8XD;
defm DVPMACUDLLSQ : dspv1_3op_rw<0xeb, "dvpmacudllsq", loadv4i32>,
                    T_MAP8PS, VEX_W;
defm DVPMACUDHHSQ : dspv1_3op_rw<0xeb, "dvpmacudhhsq", loadv4i32>,
                    T_MAP8XS, VEX_W;
defm DVPMACUDLLQ : dspv1_3op_rw<0xea, "dvpmacudllq", loadv4i32>,
                   T_MAP8PS, VEX_W;
defm DVPMACUDHHQ : dspv1_3op_rw<0xea, "dvpmacudhhq", loadv4i32>,
                   T_MAP8XS, VEX_W;
defm DVPMACDLLSQ : dspv1_3op_rw<0xe9, "dvpmacdllsq", loadv4i32>,
                   T_MAP8PS, VEX_W;
defm DVPMACDHHSQ : dspv1_3op_rw<0xe9, "dvpmacdhhsq", loadv4i32>,
                   T_MAP8XS, VEX_W;
defm DVPMACDLLQ : dspv1_3op_rw<0xe8, "dvpmacdllq", loadv4i32>, T_MAP8PS, VEX_W;
defm DVPMACDHHQ : dspv1_3op_rw<0xe8, "dvpmacdhhq", loadv4i32>, T_MAP8XS, VEX_W;
defm DVPNMACDLLSQ : dspv1_3op_rw<0xed, "dvpnmacdllsq", loadv4i32>,
                    T_MAP8PS, VEX_W;
defm DVPNMACDHHSQ : dspv1_3op_rw<0xed, "dvpnmacdhhsq", loadv4i32>,
                    T_MAP8XS, VEX_W;
defm DVPNMACDLLQ : dspv1_3op_rw<0xec, "dvpnmacdllq", loadv4i32>,
                   T_MAP8PS, VEX_W;
defm DVPNMACDHHQ : dspv1_3op_rw<0xec, "dvpnmacdhhq", loadv4i32>,
                   T_MAP8XS, VEX_W;
defm DVPMSUBADDDLLQ : dspv1_3op_rw<0xe7, "dvpmsubadddllq", loadv4i32>,
                      T_MAP8PS, VEX_W;
// end Chapter 6

// Chapter 7
defm DVPDPBUUD :  dspv1_3op_rw<0xc1, "dvpdpbuud", loadv16i8>, T_MAP8PS;
defm DVPDPBUUDS : dspv1_3op_rw<0xc2, "dvpdpbuuds", loadv16i8>, T_MAP8PS;
defm DVPDPBSSD :  dspv1_3op_rw<0xc1, "dvpdpbssd", loadv16i8>, T_MAP8XD;
defm DVPDPBSSDS : dspv1_3op_rw<0xc2, "dvpdpbssds", loadv16i8>, T_MAP8XD;
defm DVPDPBSUD :  dspv1_3op_rw<0xc1, "dvpdpbsud", loadv16i8>, T_MAP8XS;
defm DVPDPBSUDS : dspv1_3op_rw<0xc2, "dvpdpbsuds", loadv16i8>, T_MAP8XS;
defm DVPNDPBSSD : dspv1_3op_rw<0xde, "dvpndpbssd", loadv16i8>, T_MAP8XS, VEX_W;

multiclass avxvnni_dsp<bits<8> Opcode, string OpcodeStr, SDNode OpNode> {
  let Constraints = "$src1 = $dst", Predicates = [Not64BitMode, HasDSPV1] in {
   def rr : I<Opcode, MRMSrcReg, (outs VR128:$dst),
            (ins VR128:$src1, VR128:$src2, VR128:$src3),
            !strconcat(OpcodeStr, "\t{$src3, $src2, $dst|$dst, $src2, $src3}"),
            [(set VR128:$dst,
             (v4i32 (OpNode VR128:$src1,
              VR128:$src2, VR128:$src3)))]>, VEX_4V;
   def rm : I<Opcode, MRMSrcMem, (outs VR128:$dst),
            (ins VR128:$src1, VR128:$src2, i128mem:$src3),
            !strconcat(OpcodeStr, "\t{$src3, $src2, $dst|$dst, $src2, $src3}"),
            [(set VR128:$dst,
             (v4i32 (OpNode VR128:$src1,
              VR128:$src2, (loadv4i32 addr:$src3))))]>, VEX_4V;
  }
}

defm DVPDPBUSD :  avxvnni_dsp<0xc1, "dvpdpbusd", X86Vpdpbusd>, T_MAP8PD;
defm DVPDPBUSDS : avxvnni_dsp<0xc2, "dvpdpbusds", X86Vpdpbusds>, T_MAP8PD;
// end Chapter 7

// Chapter 8
defm DVPDPINT4UUD : dspv1_3op_rw<0xc3, "dvpdpint4uud", loadv16i8>, T_MAP8PS;
defm DVPDPINT4SSD : dspv1_3op_rw<0xc3, "dvpdpint4ssd", loadv16i8>, T_MAP8XD;
defm DVPDPINT4USD : dspv1_3op_rw<0xc3, "dvpdpint4usd", loadv16i8>, T_MAP8PD;
defm DVPDPINT4SUD : dspv1_3op_rw<0xc3, "dvpdpint4sud", loadv16i8>, T_MAP8XS;
// end Chapter 8

// Chapter 9
defm DVPSADACCUBWS : dspv1_3op_rw<0xdf, "dvpsadaccubws", loadv16i8>, T_MAP8XD, VEX_W;
// end Chapter 9

// Chapter 10
defm DVPSRRSUQW : dspv1_2op_imm8_w<0xa0, "dvpsrrsuqw", loadv2i64>, TAPS, VEX_W;
defm DVPSRVRSUQW : dspv1_3op_w<0xe0, "dvpsrvrsuqw", loadv2i64>, T_MAP8PS, VEX_W;
defm DVPSLRSUQW : dspv1_2op_imm8_w<0xa0, "dvpslrsuqw", loadv2i64>, TAPD, VEX_W;
defm DVPSLVRSUQW : dspv1_3op_w<0xe0, "dvpslvrsuqw", loadv2i64>, T_MAP8PD, VEX_W;
defm DVPSRARSQW : dspv1_2op_imm8_w<0xa0, "dvpsrarsqw", loadv2i64>, TAXD, VEX_W;
defm DVPSRAVRSQW : dspv1_3op_w<0xe0, "dvpsravrsqw", loadv2i64>, T_MAP8XD, VEX_W;
defm DVPSLRSQW : dspv1_2op_imm8_w<0xa0, "dvpslrsqw", loadv2i64>, TAXS, VEX_W;
defm DVPSLVRSQW : dspv1_3op_w<0xe0, "dvpslvrsqw", loadv2i64>, T_MAP8XS, VEX_W;
defm DVPSRRSUQD : dspv1_2op_imm8_w<0xa1, "dvpsrrsuqd", loadv2i64>, TAPS, VEX_W;
defm DVPSRVRSUQD : dspv1_3op_w<0xe1, "dvpsrvrsuqd", loadv2i64>, T_MAP8PS, VEX_W;
defm DVPSLRSUQD : dspv1_2op_imm8_w<0xa1, "dvpslrsuqd", loadv2i64>, TAPD, VEX_W;
defm DVPSLVRSUQD : dspv1_3op_w<0xe1, "dvpslvrsuqd", loadv2i64>, T_MAP8PD, VEX_W;
defm DVPSRARSQD : dspv1_2op_imm8_w<0xa1, "dvpsrarsqd", loadv2i64>, TAXD, VEX_W;
defm DVPSRAVRSQD : dspv1_3op_w<0xe1, "dvpsravrsqd", loadv2i64>, T_MAP8XD, VEX_W;
defm DVPSLRSQD : dspv1_2op_imm8_w<0xa1, "dvpslrsqd", loadv2i64>, TAXS, VEX_W;
defm DVPSLVRSQD : dspv1_3op_w<0xe1, "dvpslvrsqd", loadv2i64>, T_MAP8XS, VEX_W;
// end Chapter 10

// Chapter 11
let Uses = [MXCSR] in {
  defm DVPSRARD :  dspv1_2op_imm8_w<0xa2, "dvpsrard", loadv4i32>, TAXD;
  defm DVPSRAVRD : dspv1_3op_w<0xe2, "dvpsravrd", loadv4i32>, T_MAP8XD;
  defm DVPSRRUD :  dspv1_2op_imm8_w<0xa2, "dvpsrrud", loadv4i32>, TAPS;
  defm DVPSRVRUD : dspv1_3op_w<0xe2, "dvpsrvrud", loadv4i32>, T_MAP8PS;
}
defm DVPSLVSD :  dspv1_3op_w<0xe2, "dvpslvsd", loadv4i32>, T_MAP8XS;
defm DVPSLSD :   dspv1_2op_imm8_w<0xa2, "dvpslsd", loadv4i32>, TAXS;
defm DVPSLSUD :  dspv1_2op_imm8_w<0xa2, "dvpslsud", loadv4i32>, TAPD;
defm DVPSLVSUD : dspv1_3op_w<0xe2, "dvpslvsud", loadv4i32>, T_MAP8PD;
// end Chapter 11

// Chapter 12
defm DVPHADDLSWUQ : dspv1_2op_w<0xd0, "dvphaddlswuq", loadv8i16>, T_MAP8PD, VEX_W;
defm DVPHADDLSWQ : dspv1_2op_w<0xd0, "dvphaddlswq", loadv8i16>, T_MAP8PS, VEX_W;
defm DVPHADDLSDUQ : dspv1_2op_w<0xd0, "dvphaddlsduq", loadv4i32>, T_MAP8PD;
defm DVPHADDLSDQ : dspv1_2op_w<0xd0, "dvphaddlsdq", loadv4i32>, T_MAP8PS;
// end Chapter 12

// Chapter 14
let Defs = [EFLAGS], Uses = [MXCSR] in
  def DVPTESTMXCSRFLGS128 : I<0x77, RawFrm, (outs), (ins), "dvptestmxcsrflgs", []>, VEX, PD;
// end Chapter 14

// Chapter 13
defm DVPCADDROTSRAW : dspv1_3op_imm8_w<0xa3, "dvpcaddrotsraw", loadv8i16>, TAXS;
defm DVPCADDROTSRAD : dspv1_3op_imm8_w<0xa3, "dvpcaddrotsrad", loadv4i32>, TAPD;
// end Chapter 13

defm DVPUNPCKDQ : dspv1_3op_imm8_w<0xa7, "dvpunpckdq", loadv4i32>, TAXS; // Chapter 18

// Chapter 19
def DVPMASKLDDQUrm : I<0xbc, MRMSrcMem, (outs VR128:$dst),
                     (ins VR128:$src1, i128mem:$src2),
                     "dvpmasklddqu\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     [(set VR128:$dst,
                      (int_x86_dvpmasklddqu VR128:$src1, addr:$src2))]>,
                      //(!cast<Intrinsic>("int_x86_dvpmasklddqu") VR128:$src1, addr:$src2))]>,
                     T_MAP8XS, VEX_4V;
def DVPMASKSTDQUmr : I<0xbe, MRMDestMem, (outs),
                     (ins i128mem:$dst, VR128:$src1, VR128:$src2),
                     "dvpmaskstdqu\t{$src2, $src1, $dst|$dst, $src1, $src2}",
                     [(int_x86_dvpmaskstdqu addr:$dst,
                     //[(!cast<Intrinsic>("int_x86_dvpmaskstdqu") addr:$dst,
                      VR128:$src1, VR128:$src2)]>, VEX_4V, T_MAP8XS;
// end Chapter 19

// Chapter 20
let Predicates = [Not64BitMode, HasDSPV1] in {
  multiclass dspv1_3op_w_ch19<bits<8> Opcode, string OpcodeStr, PatFrag MemOpFrag,
                         SDNode OpNode, ValueType VT> {
      def rr : I<Opcode, MRMSrcReg, (outs VR128:$dst),
               (ins VR128:$src1, VR128:$src2),
               !strconcat(OpcodeStr, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"),
                [(set VR128:$dst, (VT (OpNode VR128:$src1, VR128:$src2)))]>, VEX_4V;
      def rm : I<Opcode, MRMSrcMem, (outs VR128:$dst),
               (ins VR128:$src1, i128mem:$src2),
               !strconcat(OpcodeStr, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"),
               [(set VR128:$dst, (VT (OpNode VR128:$src1, (MemOpFrag addr:$src2))))]>,
               VEX_4V;
  }

  multiclass dspv1_align {
      def rri : Ii8<0xa7, MRMSrcReg, (outs VR128:$dst),
                (ins VR128:$src1, VR128:$src2, u8imm:$src3),
                "dvpalignr\t{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                [(set VR128:$dst, (v16i8 (X86PAlignr VR128:$src1, VR128:$src2,
                  (i8 timm:$src3))))]>, VEX_4V, TAPS;
      def rmi  : Ii8<0xa7, MRMSrcMem, (outs VR128:$dst),
                (ins VR128:$src1, i128mem:$src2, u8imm:$src3),
                "dvpalignr\t{$src3, $src2, $src1, $dst|$dst, $src1, $src2, $src3}",
                [(set VR128:$dst, (v16i8 (X86PAlignr  VR128:$src1,
                 (loadv16i8 addr:$src2), (i8 timm:$src3))))]>, VEX_4V, TAPS;
  }
}

defm DVPALIGNR : dspv1_align;
defm DVPMAXSB : dspv1_3op_w_ch19<0xd1, "dvpmaxsb", loadv16i8, smax, v16i8>, T_MAP8PS;
defm DVPMAXUB : dspv1_3op_w_ch19<0xd1, "dvpmaxub", loadv16i8, umax, v16i8>, T_MAP8PD;
defm DVPMAXSW : dspv1_3op_w_ch19<0xd1, "dvpmaxsw", loadv8i16, smax, v8i16>, T_MAP8PS, VEX_W;
defm DVPMAXSD : dspv1_3op_w_ch19<0xd1, "dvpmaxsd", loadv4i32, smax, v4i32>, T_MAP8XS;
defm DVPMAXUW : dspv1_3op_w_ch19<0xd1, "dvpmaxuw", loadv8i16, umax, v8i16>, T_MAP8PD, VEX_W;
defm DVPMAXUD : dspv1_3op_w_ch19<0xd1, "dvpmaxud", loadv4i32, umax, v4i32>, T_MAP8XD;
defm DVPMINSB : dspv1_3op_w_ch19<0xd2, "dvpminsb", loadv16i8, smin, v16i8>, T_MAP8PS;
defm DVPMINUB : dspv1_3op_w_ch19<0xd2, "dvpminub", loadv16i8, umin, v16i8>, T_MAP8PD;
defm DVPMINSW : dspv1_3op_w_ch19<0xd2, "dvpminsw", loadv8i16, smin, v8i16>, T_MAP8PS, VEX_W;
defm DVPMINSD : dspv1_3op_w_ch19<0xd2, "dvpminsd", loadv4i32, smin, v4i32>, T_MAP8XS;
defm DVPMINUW : dspv1_3op_w_ch19<0xd2, "dvpminuw", loadv8i16, umin, v8i16>, T_MAP8PD, VEX_W;
defm DVPMINUD : dspv1_3op_w_ch19<0xd2, "dvpminud", loadv4i32, umin, v4i32>, T_MAP8XD;
// end Chapter 20
#endif // INTEL_FEATURE_ISA_DSPV1
