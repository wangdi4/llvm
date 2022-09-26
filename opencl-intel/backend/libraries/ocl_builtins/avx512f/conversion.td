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

/*****************************************************************\
 * convert_int_rte:                                              *
 *   intn convert_intn_rte (gentypen x)                          *
\*****************************************************************/
// convert_intn_rte ( doublen )

OclBuiltinImpl avx512_fp2int_i32_rte_v16f32 = OclBuiltinImpl<convert_i32_rte,
                                                   [v16f32], 0, [{
    float16 rounded = _mm512_roundscale_ps($Arg0VarName,
                                _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT);
    return as_$ReturnType(_mm512_cvt_roundps_epi32(rounded, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT));
  }]>;
OclBuiltinImpl avx512_fp2int_i32_rte_v8f64 = OclBuiltinImpl<convert_i32_rte,
                                                   [v8f64], 0, [{
    double8 rounded = _mm512_roundscale_pd($Arg0VarName,
                                _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT);
    return as_$ReturnType(_mm512_cvt_roundpd_epi32(rounded, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT));
  }]>;
  
/*****************************************************************\
 * convert_int_rtz:                                              *
 *   intn convert_intn_rtz (gentypen x)                          *
\*****************************************************************/
OclBuiltinImpl avx512_convert_i32_rtz_v16f32 = OclBuiltinImpl<convert_i32_rtz, [v16f32], 0, [{
    float16 rounded = _mm512_roundscale_ps($Arg0VarName,
                                           _MM_FROUND_NO_EXC | _MM_FROUND_TO_ZERO);
    return as_$ReturnType(_mm512_cvt_roundps_epi32(rounded, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT));
  }]>;

OclBuiltinImpl avx512_convert_i32_rtz_v8f64 = OclBuiltinImpl<convert_i32_rtz, [v8f64], 0, [{
    double8 rounded = _mm512_roundscale_pd($Arg0VarName,
                                           _MM_FROUND_NO_EXC | _MM_FROUND_TO_ZERO);
    return as_$ReturnType(_mm512_cvt_roundpd_epi32(rounded, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT));
  }]>;

/*****************************************************************\
 * convert_int_rtp:                                              *
 *   intn convert_intn_rtp (gentypen x)                          *
\*****************************************************************/
OclBuiltinImpl avx512_convert_i32_rtp_v16f32 = OclBuiltinImpl<convert_i32_rtp, [v16f32], 0, [{
    float16 rounded = _mm512_roundscale_ps($Arg0VarName,
                                           _MM_FROUND_NO_EXC | _MM_FROUND_TO_POS_INF);
    return as_$ReturnType(_mm512_cvt_roundps_epi32(rounded, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT));
  }]>;

OclBuiltinImpl avx512_convert_i32_rtp_v8f64 = OclBuiltinImpl<convert_i32_rtp, [v8f64], 0, [{
    double8 rounded = _mm512_roundscale_pd($Arg0VarName,
                                      _MM_FROUND_NO_EXC | _MM_FROUND_TO_POS_INF);
    return as_$ReturnType(_mm512_cvt_roundpd_epi32(rounded, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT));
  }]>;

/*****************************************************************\
 * convert_int_rtn:                                              *
 *   intn convert_intn_rtn (gentypen x)                          *
\*****************************************************************/
OclBuiltinImpl avx512_convert_i32_rtn_v16f32 = OclBuiltinImpl<convert_i32_rtn, [v16f32], 0, [{
    float16 rounded = _mm512_roundscale_ps($Arg0VarName,
                                      _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEG_INF);
    return as_$ReturnType(_mm512_cvt_roundps_epi32(rounded, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT));
  }]>;

OclBuiltinImpl avx512_convert_i32_rtn_v8f64 = OclBuiltinImpl<convert_i32_rtn, [v8f64], 0, [{
    double8 rounded = _mm512_roundscale_pd($Arg0VarName,
                                     _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEG_INF);
    return as_$ReturnType(_mm512_cvt_roundpd_epi32(rounded, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT));
  }]>;

// 32-bit unsign integer output
/*****************************************************************\
 * convert_uint_rte:                                              *
 *   intn convert_uintn_rte (gentypen x)                          *
\*****************************************************************/
OclBuiltinImpl avx512_fp2int_i32_rte_v16f32 = OclBuiltinImpl<convert_u32_rte,
                                                   [v16f32], 0, [{
    float16 rounded = _mm512_roundscale_ps($Arg0VarName,
                                 _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT);
    return as_$ReturnType(_mm512_cvt_roundps_epu32(rounded, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT));
  }]>;
OclBuiltinImpl avx512_fp2int_i32_rte_v8f64 = OclBuiltinImpl<convert_i32_rte,
                                                   [v8f64], 0, [{
    double8 rounded = _mm512_roundscale_pd($Arg0VarName,
                                 _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT);
    return as_$ReturnType(_mm512_cvt_roundpd_epi32(rounded, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT));
  }]>;
/*****************************************************************\
 * convert_uint_rtz:                                              *
 *   intn convert_uintn_rtz (gentypen x)                          *
\*****************************************************************/

OclBuiltinImpl avx512_convert_u32_rtz_v16f32 = OclBuiltinImpl<convert_u32_rtz,
                                                   [v16f32], 0, [{
    float16 rounded = _mm512_roundscale_ps($Arg0VarName,
                                         _MM_FROUND_NO_EXC | _MM_FROUND_TO_ZERO);
    return as_$ReturnType(_mm512_cvt_roundps_epu32(rounded, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT));
  }]>;

// convert_intn_rtz double
OclBuiltinImpl avx512_convert_u32_rtz_fpexpand = OclBuiltinImpl<convert_u32_rtz, [v8f64], 0, [{
    double8 rounded = _mm512_roundscale_pd($Arg0VarName,
                                 _MM_FROUND_NO_EXC | _MM_FROUND_TO_ZERO);
    return as_$ReturnType(_mm512_cvt_roundpd_epu32(rounded, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT));
  }]>;

/*****************************************************************\
 * convert_uint_rtn:                                              *
 *   intn convert_uintn_rtn (gentypen x)                          *
\*****************************************************************/
// convert_uintn_rtn ( doublen ) 
// convert_uintn_rtn ( float16 ) 
OclBuiltinImpl avx512_convert_u32_rtn_v16f32 = OclBuiltinImpl<convert_u32_rtn,
                                                   [v16f32], 0, [{
    float16 rounded = _mm512_roundscale_ps($Arg0VarName,
                                         _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEG_INF);
    return as_$ReturnType(_mm512_cvt_roundps_epu32(rounded, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT));
  }]>;

OclBuiltinImpl avx512_convert_u32_rtn_fpexpand = OclBuiltinImpl<convert_u32_rtn, [v8f64], 0, [{
    double8 rounded = _mm512_roundscale_pd($Arg0VarName,
                                 _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEG_INF);
    return as_$ReturnType(_mm512_cvt_roundpd_epu32(rounded, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT));
  }]>;
/*****************************************************************\
 * convert_uint_rtp:                                              *
 *   intn convert_uintn_rtp (gentypen x)                          *
\*****************************************************************/
// convert_uintn_rtp ( double8 )
// convert_uintn_rtp ( float16 )
OclBuiltinImpl avx512_convert_u32_rtp_v16f32 = OclBuiltinImpl<convert_u32_rtp,
                                                   [v16f32], 0, [{
    float16 rounded = _mm512_roundscale_ps($Arg0VarName,
                                         _MM_FROUND_NO_EXC | _MM_FROUND_TO_POS_INF);
    return as_$ReturnType(_mm512_cvt_roundps_epu32(rounded, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT));
  }]>;

OclBuiltinImpl avx512_convert_u32_rtp_fpexpand = OclBuiltinImpl<convert_u32_rtp, [v8f64], 0, [{
    double8 rounded = _mm512_roundscale_pd($Arg0VarName,
                                 _MM_FROUND_NO_EXC | _MM_FROUND_TO_POS_INF);
    return as_$ReturnType(_mm512_cvt_roundpd_epu32(rounded, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT));
  }]>;

// 32-bit floating output
// default rounding mode
/*****************************************************************\
 * convert_float:                                              *
 *   floatn convert_floatn (gentypen x)                          *
\*****************************************************************/

OclBuiltinImpl avx_cvt_v16f32_v16i32 = OclBuiltinImpl<convert_f32,[v16i32], 0, [{
    return as_$ReturnType(_mm512_cvtepi32_ps( __builtin_astype($Arg0VarName,__m512i)));
  }]>;

OclBuiltinImpl avx_cvt_v16f32_v16u32 = OclBuiltinImpl<convert_f32,[v16u32], 0, [{
    return as_$ReturnType(_mm512_cvtepu32_ps( __builtin_astype($Arg0VarName,__m512i)));
  }]>;

// rtz rounding mode
/*****************************************************************\
 * convert_float_rtz:                                              *
 *   floatn convert_floatn_rtz (gentypen x)                        *
\*****************************************************************/

OclBuiltinImpl avx512_rtz_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rtz,
                                                [v8f64], 0, [{
    return _mm512_cvt_roundpd_ps($Arg0VarName,
                                 _MM_FROUND_NO_EXC | _MM_FROUND_TO_ZERO);
  }]>;
  
OclBuiltinImpl avx512_rtz_cvt_f32_i32 = OclBuiltinImpl<convert_f32_rtz,
                                                [v16i32], 0, [{
    return _mm512_cvt_roundepi32_ps((__m512i)$Arg0VarName,
                                    _MM_FROUND_NO_EXC | _MM_FROUND_TO_ZERO);
  }]>;

OclBuiltinImpl avx512_rtz_cvt_f32_u32 = OclBuiltinImpl<convert_f32_rtz,
                                                [v16u32], 0, [{
    return _mm512_cvt_roundepu32_ps((__m512i)$Arg0VarName,
                                    _MM_FROUND_NO_EXC | _MM_FROUND_TO_ZERO);
  }]>;

// rtp rounding mode
/*****************************************************************\
 * convert_float_rtp:                                              *
 *   floatn convert_floatn_rtp (gentypen x)                          *
\*****************************************************************/

OclBuiltinImpl avx512_rtp_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rtp,
                                                [v8f64], 0, [{
    return _mm512_cvt_roundpd_ps($Arg0VarName,
                                 _MM_FROUND_NO_EXC | _MM_FROUND_TO_POS_INF);
  }]>;

OclBuiltinImpl avx512_rtp_cvt_f32_i32 = OclBuiltinImpl<convert_f32_rtp,
                                                [v16i32], 0, [{
    return _mm512_cvt_roundepi32_ps((__m512i)$Arg0VarName,
                                    _MM_FROUND_NO_EXC | _MM_FROUND_TO_POS_INF);
  }]>;

OclBuiltinImpl avx512_rtp_cvt_f32_u32 = OclBuiltinImpl<convert_f32_rtp,
                                                [v16u32], 0, [{
    return _mm512_cvt_roundepu32_ps((__m512i)$Arg0VarName,
                                    _MM_FROUND_NO_EXC | _MM_FROUND_TO_POS_INF);
  }]>;

// rtn rounding mode
/*****************************************************************\
 * convert_float_rtn:                                              *
 *   floatn convert_floatn_rtn (gentypen x)                          *
\*****************************************************************/

OclBuiltinImpl avx512_rtn_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rtn,
                                                [v8f64], 0, [{
    return _mm512_cvt_roundpd_ps($Arg0VarName,
                                 _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEG_INF);
  }]>;

OclBuiltinImpl avx512_rtn_cvt_f32_i32 = OclBuiltinImpl<convert_f32_rtn,
                                                [v16i32], 0, [{
    return _mm512_cvt_roundepi32_ps((__m512i)$Arg0VarName,
                                    _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEG_INF);
  }]>;

OclBuiltinImpl avx512_rtn_cvt_f32_u32 = OclBuiltinImpl<convert_f32_rtn,
                                                [v16u32], 0, [{
    return _mm512_cvt_roundepu32_ps((__m512i)$Arg0VarName,
                                    _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEG_INF);
  }]>;

// rte rounding mode
/*****************************************************************\
 * convert_double_rte:                                            *
 *   doublen convert_doublen_rte (gentypen x)                     *
\*****************************************************************/

OclBuiltinImpl avx512_rte_cvt_f64_i32 = OclBuiltinImpl<convert_f64_rte,
                                                     [v8i32], 0, [{
    return _mm512_cvtepi32_pd((__m256i)$Arg0VarName);
  }]>;
  
OclBuiltinImpl avx512_rte_cvt_f32_u32 = OclBuiltinImpl<convert_f64_rte,
                                                     [v8u32], 0, [{
   return _mm512_cvtepu32_pd((__m256i)$Arg0VarName);
  }]>;

// rtz rounding mode
/*****************************************************************\
 * convert_double_rtz:                                            *
 *   doublen convert_doublen_rtz (gentypen x)                     *
\*****************************************************************/

OclBuiltinImpl avx512_rtz_cvt_f64_u32 = OclBuiltinImpl<convert_f64_rtz,[v8u32], 0, [{
    return _mm512_cvtepu32_pd((__m256i)$Arg0VarName);
  }]>;

OclBuiltinImpl avx512_rtz_cvt_f64_i32 = OclBuiltinImpl<convert_f64_rtz,[v8i32], 0, [{
    return _mm512_cvtepi32_pd((__m256i)$Arg0VarName);
  }]>;

// rtp rounding mode
/*****************************************************************\
 * convert_double_rtp:                                            *
 *   doublen convert_doublen_rtp (gentypen x)                     *
\*****************************************************************/

OclBuiltinImpl avx512_rtp_cvt_f64_u32 = OclBuiltinImpl<convert_f64_rtp,[v8u32], 0, [{
    return _mm512_cvtepu32_pd((__m256i)$Arg0VarName);
  }]>;

OclBuiltinImpl avx512_rtp_cvt_f64_i32 = OclBuiltinImpl<convert_f64_rtp,[v8i32], 0, [{
    return _mm512_cvtepi32_pd((__m256i)$Arg0VarName);
  }]>;

// convert double to float
OclBuiltinImpl sse42_cvt_f32_f64 = OclBuiltinImpl<convert_f32,
                                                [v8f64], 0, [{
    return _mm512_cvt_roundpd_ps($Arg0VarName, _MM_FROUND_CUR_DIRECTION);
  }]>;

/*****************************************************************\
 * convert_double:                                               *
 *   doublen convert_doublen (gentypen x)                        *
\*****************************************************************/
OclBuiltinImpl avx512_cvt_f64_i32 = OclBuiltinImpl<convert_f64,[v8i32], 0, [{
    return _mm512_cvtepi32_pd((__m256i)$Arg0VarName);
  }]>;

OclBuiltinImpl avx512_cvt_f64_u32 = OclBuiltinImpl<convert_f64,[v8u32], 0, [{
    return _mm512_cvtepu32_pd((__m256i)$Arg0VarName);
  }]>;

// convert_shortn - ints
  /*
OclBuiltinImpl avx2_v8i32_conv_i16_sat = OclBuiltinImpl<convert_i16_sat, [v16i32], 0, [{
    return as_$ReturnType(_mm256_packs_epi32((__m256i)$Arg0VarName.lo, (__m256i)$Arg0VarName.hi));
  }]>;
  
OclBuiltinImpl avx2_v8i32_conv_u16_sat = OclBuiltinImpl<convert_u16_sat, [v16i32] , 0, [{
    return as_$ReturnType(_mm256_packus_epi32((__m256i)$Arg0VarName.lo, (__m256i)$Arg0VarName.hi));
  }]>;
*/
