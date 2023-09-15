// Copyright (C) 2013 Intel Corporation
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

code Expand_default = [{
    $ReturnType res;
    res$ExpandLoPattern = convert_$ReturnBaseType$LoSuffix($Arg0VarName$ExpandLoPattern);
    res$ExpandHiPattern = convert_$ReturnBaseType$HiSuffix($Arg0VarName$ExpandHiPattern);
    return res;
  }];

code Expand_rte = [{
    $ReturnType res;
    res$ExpandLoPattern = convert_$ReturnBaseType$LoSuffix_rte($Arg0VarName$ExpandLoPattern);
    res$ExpandHiPattern = convert_$ReturnBaseType$HiSuffix_rte($Arg0VarName$ExpandHiPattern);
    return res;
  }];

code Expand_rtz = [{
    $ReturnType res;
    res$ExpandLoPattern = convert_$ReturnBaseType$LoSuffix_rtz($Arg0VarName$ExpandLoPattern);
    res$ExpandHiPattern = convert_$ReturnBaseType$HiSuffix_rtz($Arg0VarName$ExpandHiPattern);
    return res;
  }];

code Expand_rtp = [{
    $ReturnType res;
    res$ExpandLoPattern = convert_$ReturnBaseType$LoSuffix_rtp($Arg0VarName$ExpandLoPattern);
    res$ExpandHiPattern = convert_$ReturnBaseType$HiSuffix_rtp($Arg0VarName$ExpandHiPattern);
    return res;
  }];

code Expand_rtn = [{
    $ReturnType res;
    res$ExpandLoPattern = convert_$ReturnBaseType$LoSuffix_rtn($Arg0VarName$ExpandLoPattern);
    res$ExpandHiPattern = convert_$ReturnBaseType$HiSuffix_rtn($Arg0VarName$ExpandHiPattern);
    return res;
  }];

code Promote_rtz = [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnBaseType#4 res = convert_$ReturnBaseType#4_rtz(n$Arg0VarName);
    return res$Pattern;
  }];
OclBuiltinImpl promote_convert_i32_rtz     = OclBuiltinImpl< convert_i32_rtz,     [v3f64],    0, Promote_rtz     >;
/*****************************************************************\
 * convert_int_rte:                                              *
 *   intn convert_intn_rte (gentypen x)                          *
\*****************************************************************/
// convert_intn_rte ( doublen ) - expand
// v2 & v3 will be scalarized, v8 & v16 will call v4
OclBuiltinImpl sse42_convert_i32_rte_f64expand = OclBuiltinImpl<convert_i32_rte,
                                                 [v16f32, v8f64], 0, Expand_rte>;

/*****************************************************************\
 * convert_int_rtz:                                              *
 *   intn convert_intn_rtz (gentypen x)                          *
\*****************************************************************/
// convert_intn_rtz float&double expand
OclBuiltinImpl sse42_convert_i32_rtz_fpexpand = OclBuiltinImpl<convert_i32_rtz,
                            [v8f32, v8f64, v16f32], 0, Expand_rtz>;

OclBuiltinImpl sse42_convert_i32_rtz_v3v4f64 = OclBuiltinImpl<convert_i32_rtz, [v3f64, v4f64], 0, [{
    int4 t1, t2, res;
    t1  = as_int4( _mm_cvttpd_epi32($Arg0VarName.lo) );
    t2  = as_int4( _mm_cvttpd_epi32($Arg0VarName.hi) );
    res = as_int4( _mm_unpacklo_epi64((__m128i)t1, (__m128i)t2) );
    return as_$ReturnType(res);
  }]>;

/*****************************************************************\
 * convert_int_rtp:                                              *
 *   intn convert_intn_rtp (gentypen x)                          *
\*****************************************************************/
// convert_intn_rtp ( doublen ) - expand
OclBuiltinImpl sse42_convert_i32_rtp_f64expand = OclBuiltinImpl<convert_i32_rtp,
                                                       [v8f64, v16f32], 0, Expand_rtp>;

/*****************************************************************\
 * convert_int_rtn:                                              *
 *   intn convert_intn_rtn (gentypen x)                          *
\*****************************************************************/
// convert_intn_rtn ( doublen ) - expand
OclBuiltinImpl sse42_convert_i32_rtn_f64expand = OclBuiltinImpl<convert_i32_rtn,
                                                       [v8f64, v16f32], 0, Expand_rtn>;

/*****************************************************************\
 * convert_uint_rte:                                              *
 *   intn convert_uintn_rte (gentypen x)                          *
\*****************************************************************/
// convert_uintn_rte ( doublen )
// f64 - v2 & v3 will be scalarized, v8 & v16 will call v4
// v16f32 will call v8
OclBuiltinImpl sse42_convert_u32_rte_f64expand = OclBuiltinImpl<convert_u32_rte,
                                                       [v8f64, v16f32], 0, Expand_rte>;

/*****************************************************************\
 * convert_uint_rtz:                                              *
 *   intn convert_uintn_rtz (gentypen x)                          *
\*****************************************************************/
// convert_intn_rtz float&double expand
OclBuiltinImpl sse42_convert_u32_rtz_fpexpand = OclBuiltinImpl<convert_u32_rtz, 
                                [v8f32, v8f64, v16f32], 0, Expand_rtz >;

/*****************************************************************\
 * convert_uint_rtp:                                              *
 *   intn convert_uintn_rtp (gentypen x)                          *
\*****************************************************************/
// convert_uintn_rtp ( doublen ) - expand 
// convert_uintn_rtp ( float16 ) - expand 
OclBuiltinImpl sse42_convert_u32_rtp_fexpand = OclBuiltinImpl<convert_u32_rtp,
                                                       [v8f64, v16f32], 0, Expand_rtp>;

/*****************************************************************\
 * convert_uint_rtn:                                              *
 *   intn convert_uintn_rtn (gentypen x)                          *
\*****************************************************************/
// convert_uintn_rtn ( doublen ) - expand
// convert_uintn_rtn ( float16 ) - expand
OclBuiltinImpl sse42_convert_u32_rtn_fexpand = OclBuiltinImpl<convert_u32_rtn,
                                                       [v8f64, v16f32], 0, Expand_rtn>;

// 64-bit integer output
/*****************************************************************\
 * Expand float16, double8, double16                              *
 * longn convert_long_<rounding_mode>:                            *
 \*****************************************************************/

OclBuiltinImpl sse42_fp2int_i64_f32f64_expand = OclBuiltinImpl<convert_i64,[v16f32, v8f64], 0, Expand_default>;

OclBuiltinImpl sse42_fp2int_i64_f32_rte_expand = OclBuiltinImpl<convert_i64_rte,[v16f32, v8f64], 0, Expand_rte>;

OclBuiltinImpl sse42_fp2int_i64_f32_rtz_expand = OclBuiltinImpl<convert_i64_rtz,[v16f32, v8f64], 0, Expand_rtz>;

OclBuiltinImpl sse42_fp2int_i64_f32_rtp_expand = OclBuiltinImpl<convert_i64_rtp,[v16f32, v8f64], 0, Expand_rtp>;

OclBuiltinImpl sse42_fp2int_i64_f32_rtn_expand = OclBuiltinImpl<convert_i64_rtn,[v16f32, v8f64], 0, Expand_rtn>;


// 64-bit unsign integer output
/*****************************************************************\
 * Expand float16, double8, double16                              *
 * ulongn convert_ulong_<rounding_mode>:                          *
 \*****************************************************************/

OclBuiltinImpl sse42_fp2int_u64_f32_expand = OclBuiltinImpl<convert_u64,[v16f32, v8f64], 0, Expand_default>;

OclBuiltinImpl sse42_fp2int_u64_f32_rte_expand = OclBuiltinImpl<convert_u64_rte, [v16f32, v8f64], 0, Expand_rte>;

OclBuiltinImpl sse42_fp2int_u64_f32_rtz_expand = OclBuiltinImpl<convert_u64_rtz, [v16f32, v8f64], 0, Expand_rtz>;

OclBuiltinImpl sse42_fp2int_u64_f32_rtp_expand = OclBuiltinImpl<convert_u64_rtp, [v16f32, v8f64], 0, Expand_rtp>;

OclBuiltinImpl sse42_fp2int_u64_f32_rtn_expand = OclBuiltinImpl<convert_u64_rtn, [v16f32, v8f64], 0, Expand_rtn>;


// 32-bit floating output

// default rounding mode
/*****************************************************************\
 * convert_float:                                              *
 *   floatn convert_floatn (gentypen x)                          *
\*****************************************************************/
OclBuiltinImpl sse42_cvt_f32_f64 = OclBuiltinImpl<convert_f32,
                                     [v4f64, v8f64], 0, Expand_default>;

OclBuiltinImpl sse42_cvt_f32_i32_i64_expand = OclBuiltinImpl<convert_f32,[v8i32, v8i64, v16i32, v16u32, v16i64], 0, Expand_default>;
OclBuiltinImpl sse42_cvt_f32_u32_u64_expand = OclBuiltinImpl<convert_f32,[v8u64, v16u64], 0, Expand_default>;

// rtz rounding mode
/*****************************************************************\
 * convert_float_rtz:                                              *
 *   floatn convert_floatn_rtz (gentypen x)                        *
\*****************************************************************/

OclBuiltinImpl sse42_rtz_cvt_f32_f64_expand = OclBuiltinImpl<convert_f32_rtz,[v8f64, v16i32, v16u32, v8u64, v16u64, v8i64, v16i64], 0, Expand_rtz>;

// rtp rounding mode
/*****************************************************************\
 * convert_float_rtp:                                              *
 *   floatn convert_floatn_rtp (gentypen x)                          *
\*****************************************************************/

OclBuiltinImpl sse42_rtp_cvt_f32_f64i32u32i64u64_expand = OclBuiltinImpl<convert_f32_rtp,[v8f64, v16i32, v16u32, 
                                             v8i64, v16i64, v8u64, v16u64], 0, Expand_rtp>;

// rtn rounding mode
/*****************************************************************\
 * convert_float_rtn:                                              *
 *   floatn convert_floatn_rtn (gentypen x)                          *
\*****************************************************************/

OclBuiltinImpl sse42_rtn_cvt_f32_f64i32u32i64u64_expand = OclBuiltinImpl<convert_f32_rtn,[v8f64, v16i32, v16u32, 
                                             v8i64, v16i64, v8u64, v16u64], 0, Expand_rtn>;

// 64-bit floating output
// default rounding mode
/*****************************************************************\
 * convert_double:                                              *
 *   doublen convert_doublen (gentypen x)                          *
\*****************************************************************/
OclBuiltinImpl sse42_cvt_f64_v3f32 = OclBuiltinImpl<convert_f64, [v3f32], 0, [{
    double4 res;
    float4 n$Arg0VarName = as_float4($Arg0VarName);
    res.lo = as_double2(_mm_cvtps_pd(__builtin_astype(n$Arg0VarName,__m128)));
    n$Arg0VarName = as_float4(_mm_srli_si128(__builtin_astype(n$Arg0VarName,__m128i), 8));
    res.hi = as_double2(_mm_cvtps_pd(__builtin_astype(n$Arg0VarName,__m128)));
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sse42_cvt_f64_v4f32 = OclBuiltinImpl<convert_f64, [v4f32], 0, [{
    double4 res;
    res.lo = as_double2(_mm_cvtps_pd(__builtin_astype($Arg0VarName,__m128)));
    $Arg0VarName = as_float4(_mm_srli_si128(__builtin_astype($Arg0VarName,__m128i), 8));
    res.hi = as_double2(_mm_cvtps_pd(__builtin_astype($Arg0VarName,__m128)));
    return res;
  }]>;
  
OclBuiltinImpl sse42_cvt_f64_f32i32u32i64u64_expand = OclBuiltinImpl<convert_f64, [v8i32, v8u32, v8i64, v8u64], 0, Expand_default>;
OclBuiltinImpl sse42_cvt_f64_v3i32 = OclBuiltinImpl<convert_f64,[v3i32], 0, [{
    double4 res;
    int4 y = as_int4(x);
    res.lo = as_double2(_mm_cvtepi32_pd(__builtin_astype(y,__m128i)));
    y = as_int4(_mm_srli_si128(__builtin_astype(y,__m128i), 8));
    res.hi = as_double2(_mm_cvtepi32_pd(__builtin_astype(y,__m128i)));
    return as_double3(res);
  }]>;

OclBuiltinImpl sse42_cvt_f64_v4i32 = OclBuiltinImpl<convert_f64,[v4i32], 0, [{
    double4 res;
    res.lo = as_double2(_mm_cvtepi32_pd(__builtin_astype(x,__m128i)));
    x = as_int4(_mm_srli_si128(__builtin_astype(x,__m128i), 8));
    res.hi = as_double2(_mm_cvtepi32_pd(__builtin_astype(x,__m128i)));
    return res;
  }]>;



// rte rounding mode
/*****************************************************************\
 * convert_double_rte:                                            *
 *   doublen convert_doublen_rte (gentypen x)                     *
\*****************************************************************/
OclBuiltinImpl sse42_rte_cvt_f64_f32i32u32i64u64_expand = OclBuiltinImpl<convert_f64_rte, [v8i32, 
                                       v8u32, v8i64, v8u64], 0, Expand_rte>;

// rtz rounding mode
/*****************************************************************\
 * convert_double_rtz:                                            *
 *   doublen convert_doublen_rtz (gentypen x)                     *
\*****************************************************************/
OclBuiltinImpl sse42_rtz_cvt_f64_f32i32u32i64u64_expand = OclBuiltinImpl<convert_f64_rtz, [v8i32, 
                                       v8u32, v8i64, v8u64], 0, Expand_rtz>;

// rtp rounding mode
/*****************************************************************\
 * convert_double_rtp:                                            *
 *   doublen convert_doublen_rtp (gentypen x)                     *
\*****************************************************************/
OclBuiltinImpl sse42_rtp_cvt_f64_f32i32u32i64u64_expand = OclBuiltinImpl<convert_f64_rtp, [v8i32, 
                                       v8u32, v8i64, v8u64], 0, Expand_rtp>;

// rtn rounding mode
/*****************************************************************\
 * convert_double_rtn:                                            *
 *   doublen convert_doublen_rtn (gentypen x)                     *
\*****************************************************************/
OclBuiltinImpl sse42_rtn_cvt_f64_f32i32u32i64u64_expand = OclBuiltinImpl<convert_f64_rtn, [v8i32, 
                                       v8u32, v8i64, v8u64], 0, Expand_rtn>;