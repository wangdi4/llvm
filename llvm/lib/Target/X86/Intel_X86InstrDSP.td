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

// Charpter 7
defm DVPMULUDHHQ : dspv1_3op_w<0xcb, "dvpmuludhhq", loadv4i32>,
                   T_MAP8XS, VEX_W;
defm DVPMULDHHQ : dspv1_3op_w<0xcc, "dvpmuldhhq", loadv4i32>,
                  T_MAP8XS, VEX_W;
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
// end Charpter 7
#endif // INTEL_FEATURE_ISA_DSPV1
