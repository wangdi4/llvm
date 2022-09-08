#if INTEL_FEATURE_ISA_AVX256P
//=- Intel_X86InstrAVX256P.td - AVX256P Instruction Set Extension -*- tablegen -*=//
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//
//
// This file describes the instructions that make up the Intel AVX256P instruction
// set.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// AVX256P RC/SAE instructions

let Predicates = [HasAVX256P], hasEVEX_P10 = 0, OpEnc = EncEVEX in {
multiclass avx256_fp_binop_p_round<bits<8> opc, string OpcodeStr, SDNode OpNodeRnd,
                                   X86SchedWriteSizes sched> {
  defm PHZ256 : avx512_fp_round_packed<opc, OpcodeStr, OpNodeRnd, sched.PH.YMM,
                                       v16f16x_info>, T_MAP5PS, EVEX_CD8<16, CD8VF>;
  defm PSZ256 : avx512_fp_round_packed<opc, OpcodeStr, OpNodeRnd, sched.PS.YMM,
                                       v8f32x_info>, PS, EVEX_CD8<32, CD8VF>;
  defm PDZ256 : avx512_fp_round_packed<opc, OpcodeStr, OpNodeRnd, sched.PD.YMM,
                                       v4f64x_info>, PD, EVEX_CD8<64, CD8VF>, VEX_W;
}

multiclass avx256_fp_binop_p_sae<bits<8> opc, string OpcodeStr, SDNode OpNodeRnd,
                                 X86SchedWriteSizes sched> {
  defm PHZ256 : avx512_fp_sae_packed<opc, OpcodeStr, OpNodeRnd, sched.PH.YMM,
                                     v16f16x_info>, T_MAP5PS, EVEX_CD8<16, CD8VF>;
  defm PSZ256 : avx512_fp_sae_packed<opc, OpcodeStr, OpNodeRnd, sched.PS.YMM,
                                     v8f32x_info>, PS, EVEX_CD8<32, CD8VF>;
  defm PDZ256 : avx512_fp_sae_packed<opc, OpcodeStr, OpNodeRnd, sched.PD.YMM,
                                     v4f64x_info>, PD, EVEX_CD8<64, CD8VF>, VEX_W;
}

multiclass avx256_vcmp_p_sae<X86SchedWriteWidths sched> {
  defm PHZ256 : avx512_vcmp_sae<sched.YMM, v16f16x_info>, AVX512PSIi8Base, EVEX_CD8<16, CD8VF>, TA;
  defm PSZ256 : avx512_vcmp_sae<sched.YMM, v8f32x_info>, AVX512PSIi8Base, EVEX_CD8<32, CD8VF>;
  defm PDZ256 : avx512_vcmp_sae<sched.YMM, v4f64x_info>, AVX512PDIi8Base, EVEX_CD8<64, CD8VF>, VEX_W;
}

multiclass avx256_fixupimm_packed_all<bits<8> opc, string OpcodeStr,
                                      X86SchedWriteWidths sched> {
  defm PSZ256 : avx512_fixupimm_packed_sae<opc, OpcodeStr, sched.YMM, v8f32x_info,
                                           v8i32x_info>, EVEX_CD8<32, CD8VF>;
  defm PDZ256 : avx512_fixupimm_packed_sae<opc, OpcodeStr, sched.YMM, v4f64x_info,
                                           v4i64x_info>, EVEX_CD8<64, CD8VF>, VEX_W;
}

multiclass avx256_vgetexp<bits<8> opc, string OpcodeStr, SDNode OpNodeSAE,
                          X86SchedWriteWidths sched> {
  defm PHZ256 : avx512_fp28_p_sae<opc, OpcodeStr#"ph", v16f16x_info, OpNodeSAE,
                                  sched.YMM>, T_MAP6PD, EVEX_CD8<16, CD8VF>;
  defm PSZ256 : avx512_fp28_p_sae<opc, OpcodeStr#"ps", v8f32x_info, OpNodeSAE,
                                  sched.YMM>, T8PD, EVEX_CD8<32, CD8VF>;
  defm PDZ256 : avx512_fp28_p_sae<opc, OpcodeStr#"pd", v4f64x_info, OpNodeSAE,
                                  sched.YMM>, T8PD, EVEX_CD8<64, CD8VF>, VEX_W;
}

multiclass avx256_unary_fp_sae<string OpcodeStr, bits<8> opcPs, bits<8> opcPd,
                               SDNode OpNodeSAE, X86SchedWriteWidths sched> {
  defm PHZ256 : avx512_unary_fp_sae_packed_imm<opcPs, OpcodeStr, OpNodeSAE, sched.YMM,
                                               v16f16x_info>, AVX512PSIi8Base, TA, EVEX_CD8<16, CD8VF>;
  defm PSZ256 : avx512_unary_fp_sae_packed_imm<opcPs, OpcodeStr, OpNodeSAE, sched.YMM,
                                               v8f32x_info>, AVX512AIi8Base, EVEX_CD8<32, CD8VF>;
  defm PDZ256 : avx512_unary_fp_sae_packed_imm<opcPd, OpcodeStr, OpNodeSAE, sched.YMM,
                                               v4f64x_info>, AVX512AIi8Base, EVEX_CD8<64, CD8VF>, VEX_W;
}

multiclass avx256_common_fp_sae_packed_imm<bits<8> opc, string OpcodeStr, SDNode OpNodeSAE,
                                           X86SchedWriteWidths sched> {
  defm PSZ256 : avx512_fp_sae_packed_imm<opc, OpcodeStr#"ps", OpNodeSAE, sched.YMM,
                                         v8f32x_info>, EVEX_CD8<32, CD8VF>;
  defm PDZ256 : avx512_fp_sae_packed_imm<opc, OpcodeStr#"pd", OpNodeSAE, sched.YMM,
                                         v4f64x_info>, EVEX_CD8<64, CD8VF>, VEX_W;
}

multiclass avx256_fp_scalef_round<bits<8> opc, string OpcodeStr, SDNode OpNodeRnd,
                                  X86SchedWriteWidths sched> {
  defm PHZ256 : avx512_fp_round_packed<opc, OpcodeStr, OpNodeRnd, sched.YMM,
                                       v16f16x_info>, T_MAP6PD, EVEX_CD8<16, CD8VF>;
  defm PSZ256 : avx512_fp_round_packed<opc, OpcodeStr, OpNodeRnd, sched.YMM,
                                       v8f32x_info>, T8PD, EVEX_CD8<32, CD8VF>;
  defm PDZ256 : avx512_fp_round_packed<opc, OpcodeStr, OpNodeRnd, sched.YMM,
                                       v4f64x_info>, T8PD, EVEX_CD8<64, CD8VF>, VEX_W;
}

multiclass avx256_sqrt_packed_all_round<bits<8> opc, string OpcodeStr,
                                        X86SchedWriteSizes sched> {
  defm PHZ256 : avx512_sqrt_packed_round<opc, !strconcat(OpcodeStr, "ph"),
                                         sched.PH.YMM, v16f16x_info>, T_MAP5PS, EVEX_CD8<16, CD8VF>;
  defm PSZ256 : avx512_sqrt_packed_round<opc, !strconcat(OpcodeStr, "ps"),
                                         sched.PS.YMM, v8f32x_info>, PS, EVEX_CD8<32, CD8VF>;
  defm PDZ256 : avx512_sqrt_packed_round<opc, !strconcat(OpcodeStr, "pd"),
                                         sched.PD.YMM, v4f64x_info>, PD, EVEX_CD8<64, CD8VF>, VEX_W;
}

multiclass avx256_vcvtw_rc<string OpcodeStr, SDNode OpNodeRnd> {
  defm PHZ256 : avx512_vcvt_fp_rc<0x7D, OpcodeStr, v16f16x_info, v16i16x_info, OpNodeRnd,
                                  SchedWriteCvtPD2DQ.YMM>, EVEX_CD8<16, CD8VF>;
}

multiclass avx256_cvtdq2fp_rc<bits<8> opc, string OpcodeStr, SDNode OpNodeRnd,
                              X86SchedWriteWidths sched> {
  defm PHZ256 : avx512_vcvt_fp_rc<opc, !strconcat(OpcodeStr, "ph"), v8f16x_info,
                                  v8i32x_info, OpNodeRnd, sched.YMM>, T_MAP5PS, EVEX_CD8<32, CD8VF>;
  defm PSZ256 : avx512_vcvt_fp_rc<opc, !strconcat(OpcodeStr, "ps"), v8f32x_info,
                                  v8i32x_info, OpNodeRnd, sched.YMM>, PS, EVEX_CD8<32, CD8VF>;
}

multiclass avx256_cvtudq2fp_rc<bits<8> opc, string OpcodeStr, SDNode OpNodeRnd,
                               X86SchedWriteWidths sched> {
  defm PHZ256 : avx512_vcvt_fp_rc<opc, !strconcat(OpcodeStr, "ph"), v8f16x_info,
                                  v8i32x_info, OpNodeRnd, sched.YMM>, T_MAP5XD, EVEX_CD8<32, CD8VF>;
  defm PSZ256 : avx512_vcvt_fp_rc<opc, !strconcat(OpcodeStr, "ps"), v8f32x_info,
                                  v8i32x_info, OpNodeRnd, sched.YMM>, XD, EVEX_CD8<32, CD8VF>;
}

multiclass avx256_cvtqq2fp_rc<string OpcodeStr, X86VectorVTInfo _Src> {
  defm PHZ256 : avx512_vcvt_fp_rc<0x5B, !strconcat(OpcodeStr, "ph"), v8f16x_info,
                                  _Src, X86VSintToFpRnd, SchedWriteCvtDQ2PS.YMM>, T_MAP5PS;
  defm PSZ256 : avx512_vcvt_fp_rc<0x5B, !strconcat(OpcodeStr, "ps"), v4f32x_info,
                                  _Src, X86VSintToFpRnd, SchedWriteCvtDQ2PS.YMM>, PS;
  defm PDZ256 : avx512_vcvt_fp_rc<0xE6, !strconcat(OpcodeStr, "pd"), v4f64x_info,
                                  _Src, X86VSintToFpRnd, SchedWriteCvtDQ2PD.YMM>, XS;
}

multiclass avx256_cvtuqq2fp_rc<string OpcodeStr, X86VectorVTInfo _Src> {
  defm PHZ256 : avx512_vcvt_fp_rc<0x7A, !strconcat(OpcodeStr, "ph"), v8f16x_info,
                                  _Src, X86VUintToFpRnd, SchedWriteCvtDQ2PS.YMM>, T_MAP5XD;
  defm PSZ256 : avx512_vcvt_fp_rc<0x7A, !strconcat(OpcodeStr, "ps"), v4f32x_info,
                                  _Src, X86VUintToFpRnd, SchedWriteCvtDQ2PS.YMM>, XD;
  defm PDZ256 : avx512_vcvt_fp_rc<0x7A, !strconcat(OpcodeStr, "pd"), v4f64x_info,
                                  _Src, X86VUintToFpRnd, SchedWriteCvtDQ2PD.YMM>, XS;
}

multiclass avx256_vcvt_pd2<string OpcodeStr, X86VectorVTInfo _Src> {
  defm PHZ256 : avx512_vcvt_fp_rc<0x5A, !strconcat(OpcodeStr, "ph"), v8f16x_info,
                                  _Src, X86vfproundRnd, SchedWriteCvtPD2PS.YMM>, T_MAP5PD;
  defm PSZ256 : avx512_vcvt_fp_rc<0x5A, !strconcat(OpcodeStr, "ps"), v4f32x_info,
                                  _Src, X86vfproundRnd, SchedWriteCvtPD2PS.YMM>, PD;
  defm DQZ256 : avx512_vcvt_fp_rc<0xE6, !strconcat(OpcodeStr, "dq"), v4i32x_info,
                                  _Src, X86cvtp2IntRnd, SchedWriteCvtPD2DQ.YMM>, XD;
  defm QQZ256 : avx512_vcvt_fp_rc<0x7B, !strconcat(OpcodeStr, "qq"), v4i64x_info,
                                  _Src, X86cvtp2IntRnd, SchedWriteCvtPD2DQ.YMM>, PD;
  defm UDQZ256 : avx512_vcvt_fp_rc<0x79, !strconcat(OpcodeStr, "udq"), v4i32x_info,
                                   _Src, X86cvtp2UIntRnd, SchedWriteCvtPD2DQ.YMM>, PS;
  defm UQQZ256 : avx512_vcvt_fp_rc<0x79, !strconcat(OpcodeStr, "uqq"), v4i64x_info,
                                   _Src, X86cvtp2UIntRnd, SchedWriteCvtPD2DQ.YMM>, PD;
}

multiclass avx256_vcvt_ps2<string OpcodeStr> {
  defm PHZ256 : avx512_cvtps2ph_sae<v8i16x_info, v8f32x_info, WriteCvtPS2PHZ>, EVEX_CD8<32, CD8VH>;
  defm PHXZ256 : avx512_vcvt_fp_rc<0x1D, !strconcat(OpcodeStr, "phx"), v8f16x_info, v8f32x_info,
                                   X86vfproundRnd, SchedWriteCvtPD2PS.YMM>, T_MAP5PD, EVEX_CD8<32, CD8VF>;
  defm PDZ256 : avx512_vcvt_fp_sae<0x5A, !strconcat(OpcodeStr, "pd"), v4f64x_info, v4f32x_info,
                                   X86vfpextSAE, SchedWriteCvtPS2PD.YMM>, PS, EVEX_CD8<32, CD8VF>;
  defm DQZ256 : avx512_vcvt_fp_rc<0x5B, !strconcat(OpcodeStr, "dq"), v8i32x_info, v8f32x_info,
                                  X86cvtp2IntRnd, SchedWriteCvtPS2DQ.YMM>, PD, EVEX_CD8<32, CD8VF>;
  defm QQZ256 : avx512_vcvt_fp_rc<0x7B, !strconcat(OpcodeStr, "qq"), v4i64x_info, v4f32x_info,
                                  X86cvtp2IntRnd, SchedWriteCvtPS2DQ.YMM>, PD, EVEX_CD8<32, CD8VF>;
  defm UDQZ256 : avx512_vcvt_fp_rc<0x79, !strconcat(OpcodeStr, "udq"), v8i32x_info, v8f32x_info,
                                   X86cvtp2UIntRnd, SchedWriteCvtPS2DQ.YMM>, PS, EVEX_CD8<32, CD8VF>;
  defm UQQZ256 : avx512_vcvt_fp_rc<0x79, !strconcat(OpcodeStr, "uqq"), v4i64x_info, v4f32x_info,
                                   X86cvtp2UIntRnd, SchedWriteCvtPS2DQ.YMM>, PD, EVEX_CD8<32, CD8VF>;
}

multiclass avx256_vcvt_ph2<string OpcodeStr> {
  defm PSZ256 : avx512_cvtph2ps_sae<v8f32x_info, v8i16x_info, WriteCvtPH2PSZ>, EVEX_CD8<32, CD8VH>;
  defm PSXZ256 : avx512_vcvt_fp_sae<0x13, !strconcat(OpcodeStr, "psx"), v8f32x_info, v8f16x_info,
                                    X86vfpextSAE, SchedWriteCvtPS2PD.YMM>, T_MAP6PD, EVEX_CD8<16, CD8VH>;
  defm PDZ256 : avx512_vcvt_fp_sae<0x5A, !strconcat(OpcodeStr, "pd"), v4f64x_info, v8f16x_info,
                                   X86vfpextSAE, SchedWriteCvtPS2PD.YMM>, T_MAP5PS, EVEX_CD8<16, CD8VQ>;
  defm WZ256 : avx512_vcvt_fp_rc<0x7D, !strconcat(OpcodeStr, "w"), v16i16x_info, v16f16x_info,
                                 X86cvtp2IntRnd, SchedWriteCvtPD2DQ.YMM>, T_MAP5PD, EVEX_CD8<16, CD8VF>;
  defm DQZ256 : avx512_vcvt_fp_rc<0x5B, !strconcat(OpcodeStr, "dq"), v8i32x_info, v8f16x_info,
                                  X86cvtp2IntRnd, SchedWriteCvtPS2DQ.YMM>, T_MAP5PD, EVEX_CD8<16, CD8VH>;
  defm QQZ256 : avx512_vcvt_fp_rc<0x7B, !strconcat(OpcodeStr, "qq"), v4i64x_info, v8f16x_info,
                                  X86cvtp2IntRnd, SchedWriteCvtPS2DQ.YMM>, T_MAP5PD, EVEX_CD8<16, CD8VQ>;
  defm UWZ256 : avx512_vcvt_fp_rc<0x7D, !strconcat(OpcodeStr, "uw"), v16i16x_info, v16f16x_info,
                                  X86cvtp2UIntRnd, SchedWriteCvtPD2DQ.YMM>, T_MAP5PS, EVEX_CD8<16, CD8VF>;
  defm UDQZ256 : avx512_vcvt_fp_rc<0x79, !strconcat(OpcodeStr, "udq"), v8i32x_info, v8f16x_info,
                                   X86cvtp2UIntRnd, SchedWriteCvtPS2DQ.YMM>, T_MAP5PS, EVEX_CD8<16, CD8VH>;
  defm UQQZ256 : avx512_vcvt_fp_rc<0x79, !strconcat(OpcodeStr, "uqq"), v4i64x_info, v8f16x_info,
                                   X86cvtp2UIntRnd, SchedWriteCvtPS2DQ.YMM>, T_MAP5PD, EVEX_CD8<16, CD8VQ>;
}

multiclass avx256_vcvtt_pd2<string OpcodeStr, X86VectorVTInfo _Src> {
  defm DQZ256 : avx512_vcvt_fp_sae<0xE6, !strconcat(OpcodeStr, "dq"), v4i32x_info,
                                   _Src, X86cvttp2siSAE, SchedWriteCvtPD2DQ.YMM>, PD;
  defm QQZ256 : avx512_vcvt_fp_sae<0x7A, !strconcat(OpcodeStr, "qq"), v4i64x_info,
                                   _Src, X86cvttp2siSAE, SchedWriteCvtPD2DQ.YMM>, PD;
  defm UDQZ256 : avx512_vcvt_fp_sae<0x78, !strconcat(OpcodeStr, "udq"), v4i32x_info,
                                    _Src, X86cvttp2uiSAE, SchedWriteCvtPD2DQ.YMM>, PS;
  defm UQQZ256 : avx512_vcvt_fp_sae<0x78, !strconcat(OpcodeStr, "uqq"), v4i64x_info,
                                    _Src, X86cvttp2uiSAE, SchedWriteCvtPD2DQ.YMM>, PD;
}

multiclass avx256_vcvtt_ps2<string OpcodeStr> {
  defm DQZ256 : avx512_vcvt_fp_sae<0x5B, !strconcat(OpcodeStr, "dq"), v8i32x_info, v8f32x_info,
                                   X86cvttp2siSAE, SchedWriteCvtPS2DQ.YMM>, XS, EVEX_CD8<32, CD8VF>;
  defm QQZ256 : avx512_vcvt_fp_sae<0x7A, !strconcat(OpcodeStr, "qq"), v4i64x_info, v4f32x_info,
                                   X86cvttp2siSAE, SchedWriteCvtPS2DQ.YMM>, PD, EVEX_CD8<32, CD8VH>;
  defm UDQZ256 : avx512_vcvt_fp_sae<0x78, !strconcat(OpcodeStr, "udq"), v8i32x_info, v8f32x_info,
                                    X86cvttp2uiSAE, SchedWriteCvtPS2DQ.YMM>, PS, EVEX_CD8<32, CD8VF>;
  defm UQQZ256 : avx512_vcvt_fp_sae<0x78, !strconcat(OpcodeStr, "uqq"), v4i64x_info, v4f32x_info,
                                    X86cvttp2uiSAE, SchedWriteCvtPS2DQ.YMM>, PD, EVEX_CD8<32, CD8VH>;
}

multiclass avx256_vcvtt_ph2<string OpcodeStr> {
  defm WZ256 : avx512_vcvt_fp_sae<0x7C, !strconcat(OpcodeStr, "w"), v16i16x_info, v16f16x_info,
                                  X86cvttp2siSAE, SchedWriteCvtPD2DQ.YMM>, T_MAP5PD, EVEX_CD8<16, CD8VF>;
  defm DQZ256 : avx512_vcvt_fp_sae<0x5B, !strconcat(OpcodeStr, "dq"), v8i32x_info, v8f16x_info,
                                   X86cvttp2siSAE, SchedWriteCvtPS2DQ.YMM>, T_MAP5XS, EVEX_CD8<16, CD8VH>;
  defm QQZ256 : avx512_vcvt_fp_sae<0x7A, !strconcat(OpcodeStr, "qq"), v4i64x_info, v8f16x_info,
                                   X86cvttp2siSAE, SchedWriteCvtPS2DQ.YMM>, T_MAP5PD, EVEX_CD8<16, CD8VQ>;
  defm UWZ256 : avx512_vcvt_fp_sae<0x7C, !strconcat(OpcodeStr, "uw"), v16i16x_info, v16f16x_info,
                                   X86cvttp2uiSAE, SchedWriteCvtPD2DQ.YMM>, T_MAP5PS, EVEX_CD8<16, CD8VF>;
  defm UDQZ256 : avx512_vcvt_fp_sae<0x78, !strconcat(OpcodeStr, "udq"), v8i32x_info, v8f16x_info,
                                    X86cvttp2uiSAE, SchedWriteCvtPS2DQ.YMM>, T_MAP5PS, EVEX_CD8<16, CD8VH>;
  defm UQQZ256 : avx512_vcvt_fp_sae<0x78, !strconcat(OpcodeStr, "uqq"), v4i64x_info, v8f16x_info,
                                    X86cvttp2uiSAE, SchedWriteCvtPS2DQ.YMM>, T_MAP5PD, EVEX_CD8<16, CD8VQ>;
}

multiclass avx256_fma3_132_round<bits<8> opc, string OpcodeStr, SDNode OpNodeRnd> {
  defm PHZ256 : avx512_fma3_132_round<opc, !strconcat(OpcodeStr, "ph"), OpNodeRnd,
                                      SchedWriteFMA.YMM, v16f16x_info>, T_MAP6PD, EVEX_CD8<16, CD8VF>;
  defm PSZ256 : avx512_fma3_132_round<opc, !strconcat(OpcodeStr, "ps"), OpNodeRnd,
                                      SchedWriteFMA.YMM, v8f32x_info>, T8PD, EVEX_CD8<32, CD8VF>;
  defm PDZ256 : avx512_fma3_132_round<opc, !strconcat(OpcodeStr, "pd"), OpNodeRnd,
                                      SchedWriteFMA.YMM, v4f64x_info>, T8PD, EVEX_CD8<64, CD8VF>, VEX_W;
}

multiclass avx256_fma3_213_round<bits<8> opc, string OpcodeStr, SDNode OpNodeRnd> {
  defm PHZ256 : avx512_fma3_213_round<opc, !strconcat(OpcodeStr, "ph"), OpNodeRnd,
                                      SchedWriteFMA.YMM, v16f16x_info>, T_MAP6PD, EVEX_CD8<16, CD8VF>;
  defm PSZ256 : avx512_fma3_213_round<opc, !strconcat(OpcodeStr, "ps"), OpNodeRnd,
                                      SchedWriteFMA.YMM, v8f32x_info>, T8PD, EVEX_CD8<32, CD8VF>;
  defm PDZ256 : avx512_fma3_213_round<opc, !strconcat(OpcodeStr, "pd"), OpNodeRnd,
                                      SchedWriteFMA.YMM, v4f64x_info>, T8PD, EVEX_CD8<64, CD8VF>, VEX_W;
}

multiclass avx256_fma3_231_round<bits<8> opc, string OpcodeStr, SDNode OpNodeRnd> {
  defm PHZ256 : avx512_fma3_231_round<opc, !strconcat(OpcodeStr, "ph"), OpNodeRnd,
                                      SchedWriteFMA.YMM, v16f16x_info>, T_MAP6PD, EVEX_CD8<16, CD8VF>;
  defm PSZ256 : avx512_fma3_231_round<opc, !strconcat(OpcodeStr, "ps"), OpNodeRnd,
                                      SchedWriteFMA.YMM, v8f32x_info>, T8PD, EVEX_CD8<32, CD8VF>;
  defm PDZ256 : avx512_fma3_231_round<opc, !strconcat(OpcodeStr, "pd"), OpNodeRnd,
                                      SchedWriteFMA.YMM, v4f64x_info>, T8PD, EVEX_CD8<64, CD8VF>, VEX_W;
}

multiclass avx256_fma3_round3<bits<8> opc132, bits<8> opc213, bits<8> opc231,
                              string OpcodeStr, SDNode OpNodeRnd> {
  defm NAME#132 : avx256_fma3_132_round<opc132, !strconcat(OpcodeStr, "132"), OpNodeRnd>;
  defm NAME#213 : avx256_fma3_213_round<opc213, !strconcat(OpcodeStr, "213"), OpNodeRnd>;
  defm NAME#231 : avx256_fma3_231_round<opc231, !strconcat(OpcodeStr, "231"), OpNodeRnd>;
}

defm VADD : avx256_fp_binop_p_round<0x58, "vadd", X86faddRnd, SchedWriteFAddSizes>;
defm VMUL : avx256_fp_binop_p_round<0x59, "vmul", X86fmulRnd, SchedWriteFMulSizes>;
defm VSUB : avx256_fp_binop_p_round<0x5C, "vsub", X86fsubRnd, SchedWriteFAddSizes>;
defm VDIV : avx256_fp_binop_p_round<0x5E, "vdiv", X86fdivRnd, SchedWriteFDivSizes>;
defm VMIN : avx256_fp_binop_p_sae<0x5D, "vmin", X86fminSAE, SchedWriteFCmpSizes>;
defm VMAX : avx256_fp_binop_p_sae<0x5F, "vmax", X86fmaxSAE, SchedWriteFCmpSizes>;
defm VCMP : avx256_vcmp_p_sae<SchedWriteFCmp>, EVEX_4V;
defm VFIXUPIMM : avx256_fixupimm_packed_all<0x54, "vfixupimm", SchedWriteFAdd>, AVX512AIi8Base, EVEX_4V;
defm VGETEXP : avx256_vgetexp<0x42, "vgetexp", X86fgetexpSAE, SchedWriteFRnd>;
defm VREDUCE : avx256_unary_fp_sae<"vreduce", 0x56, 0x56, X86VReduceSAE, SchedWriteFRnd>;
defm VRNDSCALE : avx256_unary_fp_sae<"vrndscale", 0x08, 0x09, X86VRndScaleSAE, SchedWriteFRnd>;
defm VGETMANT : avx256_unary_fp_sae<"vgetmant", 0x26, 0x26, X86VGetMantSAE, SchedWriteFRnd>;
defm VRANGE : avx256_common_fp_sae_packed_imm<0x50, "vrange", X86VRangeSAE, SchedWriteFAdd>, AVX512AIi8Base, EVEX_4V;
defm VSCALEF : avx256_fp_scalef_round<0x2C, "vscalef", X86scalefRnd, SchedWriteFAdd>;
defm VSQRT : avx256_sqrt_packed_all_round<0x51, "vsqrt", SchedWriteFSqrtSizes>;
defm VCVTW2 : avx256_vcvtw_rc<"vcvtw2ph", X86VSintToFpRnd>, T_MAP5XS;
defm VCVTDQ2 : avx256_cvtdq2fp_rc<0x5B, "vcvtdq2", X86VSintToFpRnd, SchedWriteCvtDQ2PS>;
defm VCVTQQ2 : avx256_cvtqq2fp_rc<"vcvtqq2", v4i64x_info>, EVEX_CD8<64, CD8VF>, VEX_W;
defm VCVTUW2 : avx256_vcvtw_rc<"vcvtuw2ph", X86VUintToFpRnd>, T_MAP5XD;
defm VCVTUDQ2 : avx256_cvtudq2fp_rc<0x7A, "vcvtudq2", X86VUintToFpRnd, SchedWriteCvtDQ2PS>;
defm VCVTUQQ2 : avx256_cvtuqq2fp_rc<"vcvtuqq2", v4i64x_info>, EVEX_CD8<64, CD8VF>, VEX_W;
defm VCVTPD2 : avx256_vcvt_pd2<"vcvtpd2", v4f64x_info>, EVEX_CD8<64, CD8VF>, VEX_W;
defm VCVTPS2 : avx256_vcvt_ps2<"vcvtps2">;
defm VCVTPH2 : avx256_vcvt_ph2<"vcvtph2">;
defm VCVTTPD2 : avx256_vcvtt_pd2<"vcvttpd2", v4f64x_info>, EVEX_CD8<64, CD8VF>, VEX_W;
defm VCVTTPS2 : avx256_vcvtt_ps2<"vcvttps2">;
defm VCVTTPH2 : avx256_vcvtt_ph2<"vcvttph2">;
defm VFMADD : avx256_fma3_round3<0x98, 0xA8, 0xB8, "vfmadd", X86FmaddRnd>;
defm VFMSUB : avx256_fma3_round3<0x9A, 0xAA, 0xBA, "vfmsub", X86FmsubRnd>;
defm VFMADDSUB : avx256_fma3_round3<0x96, 0xA6, 0xB6, "vfmaddsub", X86FmaddsubRnd>;
defm VFMSUBADD : avx256_fma3_round3<0x97, 0xA7, 0xB7, "vfmsubadd", X86FmsubaddRnd>;
defm VFNMADD : avx256_fma3_round3<0x9C, 0xAC, 0xBC, "vfnmadd", X86FnmaddRnd>;
defm VFNMSUB : avx256_fma3_round3<0x9E, 0xAE, 0xBE, "vfnmsub", X86FnmsubRnd>;
defm VFMULCPHZ256 : avx512_fp_round_packed<0xD6, "vfmulcph", x86vfmulcRnd, SchedWriteFMA.YMM,
                                           v8f32x_info, "", "@earlyclobber $dst">, T_MAP6XS, EVEX_CD8<32, CD8VF>;
defm VFCMULCPHZ256 : avx512_fp_round_packed<0xD6, "vfcmulcph", x86vfcmulcRnd, SchedWriteFMA.YMM,
                                            v8f32x_info, "", "@earlyclobber $dst">, T_MAP6XD, EVEX_CD8<32, CD8VF>;
defm VFMADDCPHZ256 : avx512_cfmaop_round<0x56, "vfmaddcph", x86vfmaddcRnd,
                                         v8f32x_info>, T_MAP6XS, EVEX_CD8<32, CD8VF>;
defm VFCMADDCPHZ256 : avx512_cfmaop_round<0x56, "vfcmaddcph", x86vfcmaddcRnd,
                                          v8f32x_info>, T_MAP6XD, EVEX_CD8<32, CD8VF>;
defm VCVTNE2PS2PHZ256 : avx512_ne_convert_round<0x67, "vcvtne2ps2ph", X86vcvtne2ps2phRnd, SchedWriteCvtPD2PS.YMM,
                                                v8f32x_info, v16f16x_info>, T8PD;
}
#endif // INTEL_FEATURE_ISA_AVX256P
