// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

// vim:ts=2:sw=2:et:

// Conversion between integers will be implemented as cast, sext, zext, trunc
// based on input/output types.


// NOTE:
// this is generic implementations for conversion functions common for all 
// platforms, so functions collected here has no optimisation for any 
// particular architecture with data types used in this file, for example:

// built-in convert_i32_rte has no specific code for data types
// v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64, v16f64
// and realisation for these data types is placed in shared library

// but the code for convert_i32_rte8 and convert_i32_rtef16 has some 
// differences between ssse3 and sse42 realisations, so built-ins 
// for data types v8f64 and v16f32 exist in another file: conversion_not_shared.td

// convert_shared and convert_not_shared MUST not overlap. i.e. contain 
// the same functions

list<OclType> float_types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
                             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64];

list<OclType> f16_types = [v1f16, v2f16, v3f16, v4f16, v8f16, v16f16];

// 32-bit integer output

OclBuiltinImpl fp2int_i32_f32 = OclBuiltinImpl<convert_i32,
                                               [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
                                                v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i32_f32_rte = OclBuiltinImpl<convert_i32_rte,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$rtnnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i32_f32_rtz = OclBuiltinImpl<convert_i32_rtz,
                                                   [v1f32, v2f32, v3f32, v4f32,
                                                    v1f64, v2f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i32_f32_rtp = OclBuiltinImpl<convert_i32_rtp,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$upnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i32_f32_rtn = OclBuiltinImpl<convert_i32_rtn,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$downnosat$SVMLSuffix($Arg0VarName);
  }]>;


OclBuiltinImpl fp2int_u32_f32 = OclBuiltinImpl<convert_u32,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
                                                    v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptou32$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u32_f32_rte = OclBuiltinImpl<convert_u32_rte,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptou32$rtnnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u32_f32_rtz = OclBuiltinImpl<convert_u32_rtz,
                                                   [v1f32, v2f32, v3f32, v4f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptou32$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u32_f32_rtp = OclBuiltinImpl<convert_u32_rtp,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptou32$upnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u32_f32_rtn = OclBuiltinImpl<convert_u32_rtn,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptou32$downnosat$SVMLSuffix($Arg0VarName);
  }]>;

// 32-bit floating output

// default rounding mode
OclBuiltinImpl nocvt_f32 = OclBuiltinImpl<convert_f32,
                                          [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, NoCvt>;

OclBuiltinImpl cvt_f32_f16 = OclBuiltinImpl<convert_f32, f16_types, 0,
                                            PromoteF16F32, [["Round", ""]]>;

OclBuiltinImpl cvt_f32_f64 = OclBuiltinImpl<convert_f32,
                                          [v1f64, v2f64, v3f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl cvt_f32_i8i16 = OclBuiltinImpl<convert_f32,
                                              [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                               v1i16, v2i16, v3i16, v4i16, v8i16, v16i16], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtn$SVMLSuffix(
            as_$Arg0i32gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl cvt_f32_u8u16 = OclBuiltinImpl<convert_f32,
                                              [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                               v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtn$SVMLSuffix(
            as_$Arg0u32gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl cvt_f32_i32 = OclBuiltinImpl<convert_f32,
                                            [v1i32, v2i32, v3i32, v4i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl cvt_f32_u32 = OclBuiltinImpl<convert_f32,
                                            [v1u32, v2u32, v3u32, v4u32, v8u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl cvt_f32_i64 = OclBuiltinImpl<convert_f32,
                                            [v1i64, v2i64, v3i64, v4i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl cvt_f32_u64 = OclBuiltinImpl<convert_f32,
                                            [v1u64, v2u64, v3u64, v4u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLFSuffix($Arg0VarName);
  }]>;

// rte rounding mode
OclBuiltinImpl rte_nocvt_f32 = OclBuiltinImpl<convert_f32_rte,
                                              [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, NoCvt>;

OclBuiltinImpl rte_cvt_f32_f16 = OclBuiltinImpl<convert_f32_rte, f16_types, 0,
                                                PromoteF16F32, [["Round", "_rte"]]>;

OclBuiltinImpl rte_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rte,
                                                [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rte_cvt_f32_i8i16 = OclBuiltinImpl<convert_f32_rte,
                                                  [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtn$SVMLSuffix(
            as_$Arg0i32gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rte_cvt_f32_u8u16 = OclBuiltinImpl<convert_f32_rte,
                                                  [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtn$SVMLSuffix(
            as_$Arg0u32gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rte_cvt_f32_i32 = OclBuiltinImpl<convert_f32_rte,
                                                [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rte_cvt_f32_u32 = OclBuiltinImpl<convert_f32_rte,
                                                [v1u32, v2u32, v3u32, v4u32, v8u32, v16u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rte_cvt_f32_i64 = OclBuiltinImpl<convert_f32_rte,
                                                [v1i64, v2i64, v3i64, v4i64, v8i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rte_cvt_f32_u64 = OclBuiltinImpl<convert_f32_rte,
                                                [v1u64, v2u64, v3u64, v4u64, v8u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLFSuffix($Arg0VarName);
  }]>;

// rtz rounding mode
OclBuiltinImpl rtz_nocvt_f32 = OclBuiltinImpl<convert_f32_rtz,
                                              [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, NoCvt>;

OclBuiltinImpl rtz_cvt_f32_f16 = OclBuiltinImpl<convert_f32_rtz, f16_types, 0,
                                                PromoteF16F32, [["Round", "_rtz"]]>;

OclBuiltinImpl rtz_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rtz,
                                                [v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$rtz$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtz_cvt_f32_i8i16 = OclBuiltinImpl<convert_f32_rtz,
                                                  [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtz$SVMLSuffix(
            as_$Arg0i32gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtz_cvt_f32_u8u16 = OclBuiltinImpl<convert_f32_rtz,
                                                  [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtz$SVMLSuffix(
            as_$Arg0u32gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtz_cvt_f32_i32 = OclBuiltinImpl<convert_f32_rtz,
                                                [v1i32, v2i32, v3i32, v4i32, v8i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtz$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtz_cvt_f32_u32 = OclBuiltinImpl<convert_f32_rtz,
                                                [v1u32, v2u32, v3u32, v4u32, v8u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtz$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtz_cvt_f32_i64 = OclBuiltinImpl<convert_f32_rtz,
                                                [v1i64, v2i64, v3i64, v4i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtz$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtz_cvt_f32_u64 = OclBuiltinImpl<convert_f32_rtz,
                                                [v1u64, v2u64, v3u64, v4u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtz$SVMLFSuffix($Arg0VarName);
  }]>;

// rtp rounding mode
OclBuiltinImpl rtp_nocvt_f32 = OclBuiltinImpl<convert_f32_rtp,
                                              [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, NoCvt>;

OclBuiltinImpl rtp_cvt_f32_f16 = OclBuiltinImpl<convert_f32_rtp, f16_types, 0,
                                                PromoteF16F32, [["Round", "_rtp"]]>;

OclBuiltinImpl rtp_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rtp,
                                                [v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$up$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtp_cvt_f32_i8i16 = OclBuiltinImpl<convert_f32_rtp,
                                                  [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16], 0, [{
    return __ocl_svml_$Target_cvti32tofp$up$SVMLSuffix(
            as_$Arg0i32gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtp_cvt_f32_u8u16 = OclBuiltinImpl<convert_f32_rtp,
                                                  [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$up$SVMLSuffix(
            as_$Arg0u32gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtp_cvt_f32_i32 = OclBuiltinImpl<convert_f32_rtp,
                                                [v1i32, v2i32, v3i32, v4i32, v8i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$up$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtp_cvt_f32_u32 = OclBuiltinImpl<convert_f32_rtp,
                                                [v1u32, v2u32, v3u32, v4u32, v8u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$up$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtp_cvt_f32_i64 = OclBuiltinImpl<convert_f32_rtp,
                                                [v1i64, v2i64, v3i64, v4i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$up$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtp_cvt_f32_u64 = OclBuiltinImpl<convert_f32_rtp,
                                                [v1u64, v2u64, v3u64, v4u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$up$SVMLFSuffix($Arg0VarName);
  }]>;

// rtn rounding mode
OclBuiltinImpl rtn_nocvt_f32 = OclBuiltinImpl<convert_f32_rtn,
                                              [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, NoCvt>;

OclBuiltinImpl rtn_cvt_f32_f16 = OclBuiltinImpl<convert_f32_rtn, f16_types, 0,
                                                PromoteF16F32, [["Round", "_rtn"]]>;

OclBuiltinImpl rtn_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rtn,
                                                [v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$down$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtn_cvt_f32_i8i16 = OclBuiltinImpl<convert_f32_rtn,
                                                  [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16], 0, [{
    return __ocl_svml_$Target_cvti32tofp$down$SVMLSuffix(
            as_$Arg0i32gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtn_cvt_f32_u8u16 = OclBuiltinImpl<convert_f32_rtn,
                                                  [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$down$SVMLSuffix(
            as_$Arg0u32gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtn_cvt_f32_i32 = OclBuiltinImpl<convert_f32_rtn,
                                                [v1i32, v2i32, v3i32, v4i32, v8i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$down$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtn_cvt_f32_u32 = OclBuiltinImpl<convert_f32_rtn,
                                                [v1u32, v2u32, v3u32, v4u32, v8u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$down$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtn_cvt_f32_i64 = OclBuiltinImpl<convert_f32_rtn,
                                                [v1i64, v2i64, v3i64, v4i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$down$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtn_cvt_f32_u64 = OclBuiltinImpl<convert_f32_rtn,
                                                [v1u64, v2u64, v3u64, v4u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$down$SVMLFSuffix($Arg0VarName);
  }]>;

// 64-bit floating output

// cvt_f64_f32
// conversion from f32 to f64 is easily handled by target insn alone and there
// is no difference among rounding modes.
// |fS1|fE8|fM23| -> |dS1|dE11|dM52|
// where
// - dS   := fS1
// - dE11 := fE8 + 127 - 1023
// - dM52 := fM23[22:0], pad[28:0], where pad = 0.
//

// default rounding mode
OclBuiltinImpl nocvt_f64 = OclBuiltinImpl<convert_f64,
                                          [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, NoCvt>;

OclBuiltinImpl cvt_f64_f16 = OclBuiltinImpl<convert_f64, f16_types, 0,
                                            PromoteF16F64, [["Round", ""]]>;

OclBuiltinImpl cvt_f64_i8i16i32 = OclBuiltinImpl<convert_f64,
                                                 [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                  v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                  v1i32, v2i32, v16i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$VecLength(
            as_$Arg0i64gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl cvt_f64_u8u16u32 = OclBuiltinImpl<convert_f64,
                                                 [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                  v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                                  v1u32, v2u32, v3u32, v4u32, v16u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$VecLength(
            as_$Arg0u64gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl cvt_f64_i64 = OclBuiltinImpl<convert_f64,
                                            [v1i64, v2i64, v3i64, v4i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl cvt_f64_u64 = OclBuiltinImpl<convert_f64,
                                            [v1u64, v2u64, v3u64, v4u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

// rte rounding mode
OclBuiltinImpl rte_nocvt_f64 = OclBuiltinImpl<convert_f64_rte,
                                              [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, NoCvt>;

OclBuiltinImpl rte_cvt_f64_f16 = OclBuiltinImpl<convert_f64_rte, f16_types, 0,
                                                PromoteF16F64, [["Round", "_rte"]]>;

OclBuiltinImpl rte_cvt_f64_i8i16i32 = OclBuiltinImpl<convert_f64_rte,
                                                     [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                      v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                      v1i32, v2i32, v3i32, v4i32, v16i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$VecLength(
            as_$Arg0i64gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rte_cvt_f64_u8u16u32 = OclBuiltinImpl<convert_f64_rte,
                                                     [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                      v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                                      v1u32, v2u32, v3u32, v4u32, v16u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$VecLength(
            as_$Arg0u64gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rte_cvt_f64_i64 = OclBuiltinImpl<convert_f64_rte,
                                                [v1i64, v2i64, v3i64, v4i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rte_cvt_f64_u64 = OclBuiltinImpl<convert_f64_rte,
                                                [v1u64, v2u64, v3u64, v4u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

// rtz rounding mode
OclBuiltinImpl rtz_nocvt_f64 = OclBuiltinImpl<convert_f64_rtz,
                                              [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, NoCvt>;

OclBuiltinImpl rtz_cvt_f64_f16 = OclBuiltinImpl<convert_f64_rtz, f16_types, 0,
                                                PromoteF16F64, [["Round", "_rtz"]]>;

OclBuiltinImpl rtz_cvt_f64_i8i16i32 = OclBuiltinImpl<convert_f64_rtz,
                                                     [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                      v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                      v1i32, v2i32, v3i32, v4i32, v16i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtz$VecLength(
            as_$Arg0i64gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtz_cvt_f64_u8u16u32 = OclBuiltinImpl<convert_f64_rtz,
                                                     [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                      v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                                      v1u32, v2u32, v3u32, v4u32, v16u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtz$VecLength(
            as_$Arg0u64gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtz_cvt_f64_i64 = OclBuiltinImpl<convert_f64_rtz,
                                                [v1i64, v2i64, v3i64, v4i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtz$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtz_cvt_f64_u64 = OclBuiltinImpl<convert_f64_rtz,
                                                [v1u64, v2u64, v3u64, v4u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtz$SVMLSuffix($Arg0VarName);
  }]>;

// rtp rounding mode
OclBuiltinImpl rtp_nocvt_f64 = OclBuiltinImpl<convert_f64_rtp,
                                              [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, NoCvt>;

OclBuiltinImpl rtp_cvt_f64_f16 = OclBuiltinImpl<convert_f64_rtp, f16_types, 0,
                                                PromoteF16F64, [["Round", "_rtp"]]>;

OclBuiltinImpl rtp_cvt_f64_i8i16i32 = OclBuiltinImpl<convert_f64_rtp,
                                                     [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                      v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                      v1i32, v2i32, v3i32, v4i32, v16i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$up$VecLength(
            as_$Arg0i64gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtp_cvt_f64_u8u16u32 = OclBuiltinImpl<convert_f64_rtp,
                                                     [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                      v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                                      v1u32, v2u32, v3u32, v4u32, v16u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$up$VecLength(
            as_$Arg0u64gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtp_cvt_f64_i64 = OclBuiltinImpl<convert_f64_rtp,
                                                [v1i64, v2i64, v3i64, v4i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$up$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtp_cvt_f64_u64 = OclBuiltinImpl<convert_f64_rtp,
                                                [v1u64, v2u64, v3u64, v4u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$up$SVMLSuffix($Arg0VarName);
  }]>;

// rtn rounding mode
OclBuiltinImpl rtn_nocvt_f64 = OclBuiltinImpl<convert_f64_rtn,
                                              [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, NoCvt>;

OclBuiltinImpl rtn_cvt_f64_f16 = OclBuiltinImpl<convert_f64_rtn, f16_types, 0,
                                                PromoteF16F64, [["Round", "_rtn"]]>;

OclBuiltinImpl rtn_cvt_f64_i8i16i32 = OclBuiltinImpl<convert_f64_rtn,
                                                     [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                      v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                      v1i32, v2i32, v3i32, v4i32, v16i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$down$VecLength(
            as_$Arg0i64gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtn_cvt_f64_u8u16u32 = OclBuiltinImpl<convert_f64_rtn,
                                                     [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                      v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                                      v1u32, v2u32, v3u32, v4u32, v16u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$down$VecLength(
            as_$Arg0u64gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtn_cvt_f64_i64 = OclBuiltinImpl<convert_f64_rtn,
                                                [v1i64, v2i64, v3i64, v4i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$down$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtn_cvt_f64_u64 = OclBuiltinImpl<convert_f64_rtn,
                                                [v1u64, v2u64, v3u64, v4u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$down$SVMLSuffix($Arg0VarName);
  }]>;
