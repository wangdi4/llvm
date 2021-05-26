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
  multiclass dspv1_2op_imm8_base1<bits<8> Opcode, string OpcodeStr,
              X86MemOperand x86memop, PatFrag MemOpFrag> {
      def rr : Ii8<Opcode, MRMSrcReg, (outs VR128:$dst),
               (ins VR128:$src1, i32u8imm:$src2),
               !strconcat(OpcodeStr, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"),
               [(set VR128:$dst,
                (!cast<Intrinsic>("int_x86_"#OpcodeStr#"128")
                 VR128:$src1, timm:$src2))]>, VEX;
      def rm : Ii8<Opcode, MRMSrcMem, (outs VR128:$dst),
               (ins x86memop:$src1, i32u8imm:$src2),
               !strconcat(OpcodeStr, "\t{$src2, $src1, $dst|$dst, $src1, $src2}"),
               [(set VR128:$dst,
                (!cast<Intrinsic>("int_x86_"#OpcodeStr#"128")
                 (MemOpFrag addr:$src1), timm:$src2))]>, VEX;
  }

  multiclass dspv1_3op_imm8_base1<bits<8> Opcode, string OpcodeStr,
              X86MemOperand x86memop, PatFrag MemOpFrag> {
    let Constraints = "$src1 = $dst" in {
      def rr : Ii8<Opcode, MRMSrcReg, (outs VR128:$dst),
               (ins VR128:$src1, VR128:$src2, VR128:$src3, i32u8imm:$src4),
               !strconcat(OpcodeStr, "\t{$src4, $src3, $src2, $dst|$dst, $src2, $src3, $src4}"),
               [(set VR128:$dst,
                (!cast<Intrinsic>("int_x86_"#OpcodeStr#"128") VR128:$src1,
                VR128:$src2, VR128:$src3, timm:$src4))]>, VEX_4V;
      def rm : Ii8<Opcode, MRMSrcMem, (outs VR128:$dst),
               (ins VR128:$src1, VR128:$src2, x86memop:$src3, i32u8imm:$src4),
               !strconcat(OpcodeStr, "\t{$src4, $src3, $src2, $dst|$dst, $src2, $src3, $src4}"),
               [(set VR128:$dst,
                (!cast<Intrinsic>("int_x86_"#OpcodeStr#"128") VR128:$src1,
                VR128:$src2, (MemOpFrag addr:$src3), timm:$src4))]>, VEX_4V;
    }
  }
} // let Predicates = [Not64BitMode, HasDSPV1]

let Uses = [MXCSR] in
  defm DVPCR2BFRSW : dspv1_3op_imm8_base1<0xa3, "dvpcr2bfrsw", i128mem,
                     loadv8i16>, TAXD, VEX_W;

defm DVPLUTSINCOSW : dspv1_2op_imm8_base1<0xa3, "dvplutsincosw", i128mem,
                     loadv8i16>, TAPS;
#endif // INTEL_FEATURE_ISA_DSPV1
