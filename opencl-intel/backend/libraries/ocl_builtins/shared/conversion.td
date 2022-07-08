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

list<OclType> conversion_fp_types  = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
                                      v1f64, v2f64, v3f64, v4f64, v8f64, v16f64];

/*-------------------------------------------------------
Expand and Promote of converion functions are not simular to all other function since
in the function name we also have the return type vector size.
for example, convert_float16 will promote to:
  convert_float8(input.lo)
  convert_float8(input.hi)
that's the reason we have those new code decleration for Expand and Promote.
---------------------------------------------------------*/

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
  
// Promote
code Promote_default = [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnBaseType#4 res = convert_$ReturnBaseType#4(n$Arg0VarName);
    return res$Pattern;
  }];

code Promote_rte = [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnBaseType#4 res = convert_$ReturnBaseType#4_rte(n$Arg0VarName);
    return res$Pattern;  
  }];

code Promote_rtz = [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnBaseType#4 res = convert_$ReturnBaseType#4_rtz(n$Arg0VarName);
    return res$Pattern;
  }];

code Promote_rtp = [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnBaseType#4 res = convert_$ReturnBaseType#4_rtp(n$Arg0VarName);
    return res$Pattern;
  }];

code Promote_rtn = [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnBaseType#4 res = convert_$ReturnBaseType#4_rtn(n$Arg0VarName);
    return res$Pattern;
  }];

list<OclType> Promote_fp_types      =  [v2f32, v3f32, v2f64, v3f64];
list<OclType> Promote_integer_types =  [v2i32, v3i32, v2i64, v3i64, v2u32, v3u32, v2u64, v3u64];

// promote v2f32, v3f32, v2f64, v3f64 to v4f32, v4f64
                                                    
OclBuiltinImpl promote_convert_i32         = OclBuiltinImpl< convert_i32,         Promote_fp_types,    0, Promote_default >;
OclBuiltinImpl promote_convert_i32_rte     = OclBuiltinImpl< convert_i32_rte,     Promote_fp_types,    0, Promote_rte     >;
//convert_i32_rtz for v3f64 is re-defined for avx, so we exclude it from shared rtl
OclBuiltinImpl promote_convert_i32_rtz     = OclBuiltinImpl< convert_i32_rtz,     [v2f32, v3f32, v2f64],    0, Promote_rtz     >;
OclBuiltinImpl promote_convert_i32_rtp     = OclBuiltinImpl< convert_i32_rtp,     Promote_fp_types,    0, Promote_rtp     >;
OclBuiltinImpl promote_convert_i32_rtn     = OclBuiltinImpl< convert_i32_rtn,     Promote_fp_types,    0, Promote_rtn     >;
OclBuiltinImpl promote_convert_u32         = OclBuiltinImpl< convert_u32,         Promote_fp_types,    0, Promote_default >;
OclBuiltinImpl promote_convert_u32_rte     = OclBuiltinImpl< convert_u32_rte,     Promote_fp_types,    0, Promote_rte     >;
OclBuiltinImpl promote_convert_u32_rtz     = OclBuiltinImpl< convert_u32_rtz,     Promote_fp_types,    0, Promote_rtz     >;
OclBuiltinImpl promote_convert_u32_rtp     = OclBuiltinImpl< convert_u32_rtp,     Promote_fp_types,    0, Promote_rtp     >;
OclBuiltinImpl promote_convert_u32_rtn     = OclBuiltinImpl< convert_u32_rtn,     Promote_fp_types,    0, Promote_rtn     >;
                                                             
OclBuiltinImpl promote_convert_i64         = OclBuiltinImpl< convert_i64,         Promote_fp_types,    0,  Promote_default >;
OclBuiltinImpl promote_convert_i64_rte     = OclBuiltinImpl< convert_i64_rte,     Promote_fp_types,    0,  Promote_rte     >;
OclBuiltinImpl promote_convert_i64_rtz     = OclBuiltinImpl< convert_i64_rtz,     Promote_fp_types,    0,  Promote_rtz     >;
OclBuiltinImpl promote_convert_i64_rtp     = OclBuiltinImpl< convert_i64_rtp,     Promote_fp_types,    0,  Promote_rtp     >;
OclBuiltinImpl promote_convert_i64_rtn     = OclBuiltinImpl< convert_i64_rtn,     Promote_fp_types,    0,  Promote_rtn     >;
OclBuiltinImpl promote_convert_u64         = OclBuiltinImpl< convert_u64,         Promote_fp_types,    0,  Promote_default >;
OclBuiltinImpl promote_convert_u64_rte     = OclBuiltinImpl< convert_u64_rte,     Promote_fp_types,    0,  Promote_rte     >;
OclBuiltinImpl promote_convert_u64_rtz     = OclBuiltinImpl< convert_u64_rtz,     Promote_fp_types,    0,  Promote_rtz     >;
OclBuiltinImpl promote_convert_u64_rtp     = OclBuiltinImpl< convert_u64_rtp,     Promote_fp_types,    0,  Promote_rtp     >;
OclBuiltinImpl promote_convert_u64_rtn     = OclBuiltinImpl< convert_u64_rtn,     Promote_fp_types,    0,  Promote_rtn     >;

OclBuiltinImpl promote_convert_f32         = OclBuiltinImpl< convert_f32,         Promote_integer_types,    0, Promote_default >;
OclBuiltinImpl promote_convert_f32_rte     = OclBuiltinImpl< convert_f32_rte,     Promote_integer_types,    0, Promote_rte     >;
OclBuiltinImpl promote_convert_f32_rtz     = OclBuiltinImpl< convert_f32_rtz,     Promote_integer_types,    0, Promote_rtz     >;
OclBuiltinImpl promote_convert_f32_rtp     = OclBuiltinImpl< convert_f32_rtp,     Promote_integer_types,    0, Promote_rtp     >;
OclBuiltinImpl promote_convert_f32_rtn     = OclBuiltinImpl< convert_f32_rtn,     Promote_integer_types,    0, Promote_rtn     >;

OclBuiltinImpl promote_convert_f32_f64         = OclBuiltinImpl< convert_f32,         [v3f64],    0, Promote_default >;
OclBuiltinImpl promote_convert_f32_f64_rte     = OclBuiltinImpl< convert_f32_rte,     [v2f64, v3f64],    0, Promote_rte     >;
OclBuiltinImpl promote_convert_f32_f64_rtz     = OclBuiltinImpl< convert_f32_rtz,     [v2f64, v3f64],    0, Promote_rtz     >;
OclBuiltinImpl promote_convert_f32_f64_rtp     = OclBuiltinImpl< convert_f32_rtp,     [v2f64, v3f64],    0, Promote_rtp     >;
OclBuiltinImpl promote_convert_f32_f64_rtn     = OclBuiltinImpl< convert_f32_rtn,     [v2f64, v3f64],    0, Promote_rtn     >;

OclBuiltinImpl promote_convert_f64         = OclBuiltinImpl< convert_f64 ,         [v2i64, v3i64, v2u32, v3u32, v2u64, v3u64],    0, Promote_default >;
OclBuiltinImpl promote_convert_f64_rte     = OclBuiltinImpl< convert_f64_rte ,     Promote_integer_types,    0, Promote_rte     >;
OclBuiltinImpl promote_convert_f64_rtz     = OclBuiltinImpl< convert_f64_rtz ,     Promote_integer_types,    0, Promote_rtz     >;
OclBuiltinImpl promote_convert_f64_rtp     = OclBuiltinImpl< convert_f64_rtp ,     Promote_integer_types,    0, Promote_rtp     >;
OclBuiltinImpl promote_convert_f64_rtn     = OclBuiltinImpl< convert_f64_rtn ,     Promote_integer_types,    0, Promote_rtn     >;

// 8-bit integer output
/*****************************************************************\
 * convert_char:                                                  *
 *   charn convert_charn (floatn x)                               *
 *   charn convert_charn (doublen x)                              *
\*****************************************************************/
OclBuiltinImpl sse42_fp2int_i8_f32f64  = OclBuiltinImpl<convert_i8,
                                               conversion_fp_types, 0, [{
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
              convert_$Arg0i32gentype($Arg0VarName)));
  }]>;
/*****************************************************************\
 * convert_char_rte:                                              *
 *   charn convert_charn_rte (floatn x)                           *
 *   charn convert_charn_rte (doublen x)                          *
\*****************************************************************/

OclBuiltinImpl sse42_fp2int_i8_f32f64_rte  = OclBuiltinImpl<convert_i8_rte,
                                                   conversion_fp_types, 0, [{
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
              convert_$Arg0i32gentype_rte($Arg0VarName)));
  }]>;
/*****************************************************************\
 * convert_char_rtz:                                              *
 *   charn convert_charn_rtz (floatn x)                           *
 *   charn convert_charn_rtz (doublen x)                          *
\*****************************************************************/

OclBuiltinImpl sse42_fp2int_i8_f32f64_rtz  = OclBuiltinImpl<convert_i8_rtz,
                                                   conversion_fp_types, 0, [{
     return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
              convert_$Arg0i32gentype_rtz($Arg0VarName)));
  }]>;
/*****************************************************************\
 * convert_char_rtp:                                              *
 *   charn convert_charn_rtp (floatn x)                           *
 *   charn convert_charn_rtp (doublen x)                          *
\*****************************************************************/
OclBuiltinImpl sse42_fp2int_i8_f32f64_rtp  = OclBuiltinImpl<convert_i8_rtp,
                                                 conversion_fp_types, 0, [{
     return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
              convert_$Arg0i32gentype_rtp($Arg0VarName)));
  }]>;

/*****************************************************************\
 * convert_char_rtn:                                              *
 *   charn convert_charn_rtn (floatn x)                           *
 *   charn convert_charn_rtn (doublen x)                          *
\*****************************************************************/

OclBuiltinImpl sse42_fp2int_i8_f32f64_rtn  = OclBuiltinImpl<convert_i8_rtn,
                                                   conversion_fp_types, 0, [{
     return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
              convert_$Arg0i32gentype_rtn($Arg0VarName)));
  }]>;
  
// 8-bit unsign integer output
/*****************************************************************\
 * convert_uchar:                                              *
 *   charn convert_ucharn (floatn x)                           *
 *   charn convert_ucharn (doublen x)                          *
\*****************************************************************/

OclBuiltinImpl sse42_fp2int_u8_f32f64  = OclBuiltinImpl<convert_u8,
                                                conversion_fp_types, 0, [{
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
              as_$Arg0i32gentype(
                convert_$Arg0u32gentype($Arg0VarName))));
  }]>;
/*****************************************************************\
 * convert_uchar_rte:                                              *
 *   charn convert_ucharn_rte (floatn x)                           *
 *   charn convert_ucharn_rte (doublen x)                          *
\*****************************************************************/

OclBuiltinImpl sse42_fp2int_u8_f32f64_rte  = OclBuiltinImpl<convert_u8_rte,
                                                    conversion_fp_types, 0, [{
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
              as_$Arg0i32gentype(
                convert_$Arg0u32gentype_rte($Arg0VarName))));
  }]>;
/*****************************************************************\
 * convert_uchar_rtz:                                              *
 *   charn convert_ucharn_rtz (floatn x)                           *
 *   charn convert_ucharn_rtz (doublen x)                          *
\*****************************************************************/

OclBuiltinImpl sse42_fp2int_u8_f32f64_rtz  = OclBuiltinImpl<convert_u8_rtz,
                                                    conversion_fp_types, 0, [{
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
              as_$Arg0i32gentype(
                convert_$Arg0u32gentype_rtz($Arg0VarName))));
  }]>;
/*****************************************************************\
 * convert_uchar_rtp:                                              *
 *   charn convert_ucharn_rtp (floatn x)                           *
 *   charn convert_ucharn_rtp (doublen x)                          *
\*****************************************************************/

OclBuiltinImpl sse42_fp2int_u8_f32f64_rtp  = OclBuiltinImpl<convert_u8_rtp,
                                                    conversion_fp_types, 0, [{
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
              as_$Arg0i32gentype(
                convert_$Arg0u32gentype_rtp($Arg0VarName))));
  }]>;
/*****************************************************************\
 * convert_uchar_rtn:                                            *
 *   charn convert_ucharn_rtn (floatn x)                         *
 *   charn convert_ucharn_rtn (doublen x)                        *
\*****************************************************************/

OclBuiltinImpl sse42_fp2int_u8_f32f64_rtn  = OclBuiltinImpl<convert_u8_rtn,
                                                    conversion_fp_types, 0, [{
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
              as_$Arg0i32gentype(
                convert_$Arg0u32gentype_rtn($Arg0VarName))));
  }]>;

  // 16-bit integer output
/*****************************************************************\
 * convert_short:                                                *
 *   shortn convert_shortn (floatn x)                            *
 *   shortn convert_shortn (doublen x)                           *
\*****************************************************************/

OclBuiltinImpl sse42_fp2int_i16_f32f64 = OclBuiltinImpl<convert_i16,
                          conversion_fp_types, 0, [{
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
                convert_$Arg0i32gentype($Arg0VarName)));
  }]>;
  
/*****************************************************************\
 * convert_short_rte:                                            *
 *   shortn convert_shortn_rte (floatn x)                        *
 *   shortn convert_shortn_rte (doublen x)                       *
\*****************************************************************/

OclBuiltinImpl sse42_fp2int_i16_f32f64_rte = OclBuiltinImpl<convert_i16_rte,
                                                    conversion_fp_types, 0, [{
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
              convert_$Arg0i32gentype_rte($Arg0VarName)));
  }]>;
  
/*****************************************************************\
 * convert_short_rtz:                                            *
 *   shortn convert_shortn_rtz (floatn x)                        *
 *   shortn convert_shortn_rtz (doublen x)                       *
\*****************************************************************/

OclBuiltinImpl sse42_fp2int_i16_f32f64_rtz = OclBuiltinImpl<convert_i16_rtz,
                                                    conversion_fp_types, 0, [{
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
              convert_$Arg0i32gentype_rtz($Arg0VarName)));
  }]>;
  
/*****************************************************************\
 * convert_short_rtp:                                            *
 *   shortn convert_shortn_rtp (floatn x)                        *
 *   shortn convert_shortn_rtp (doublen x)                       *
\*****************************************************************/

OclBuiltinImpl sse42_fp2int_i16_f32f64_rtp = OclBuiltinImpl<convert_i16_rtp,
                                                    conversion_fp_types, 0, [{
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
              convert_$Arg0i32gentype_rtp($Arg0VarName)));
  }]>;
  
/*****************************************************************\
 * convert_short_rtn:                                            *
 *   shortn convert_shortn_rtn (floatn x)                        *
 *   shortn convert_shortn_rtn (doublen x)                       *
\*****************************************************************/

OclBuiltinImpl sse42_fp2int_i16_f32f64_rtn = OclBuiltinImpl<convert_i16_rtn,
                                                    conversion_fp_types, 0, [{
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
              convert_$Arg0i32gentype_rtn($Arg0VarName)));
  }]>;

// 32-bit integer output
/*****************************************************************\
 * convert_int:                                                  *
 *   intn convert_intn (gentypen x)                              *
\*****************************************************************/
// convert_intn( floatn ) & convert_intn( doublen )
OclBuiltinImpl sse42_convert_i32_f32f64 = OclBuiltinImpl<convert_i32, conversion_fp_types, 0, [{
    return convert_$ReturnType_rtz($Arg0VarName);
  }]>;


/*****************************************************************\
 * convert_int_rte:                                              *
 *   intn convert_intn_rte (gentypen x)                          *
\*****************************************************************/
// convert_intn_rte ( doublen ) - expand
// v2 & v3 will be scalarized, v8 & v16 will call v4
OclBuiltinImpl sse42_convert_i32_rte_f64expand = OclBuiltinImpl<convert_i32_rte,
                      [v2f64, v3f64, v16f64], 0, Expand_rte>;


/*****************************************************************\
 * convert_int_rtz:                                              *
 *   intn convert_intn_rtz (gentypen x)                          *
\*****************************************************************/
// convert_intn_rtz( floatn ) 
OclBuiltinImpl sse42_convert_i32_rtz_v1f32 = OclBuiltinImpl<convert_i32_rtz, [v1f32], 0, [{
    float4 temp;
    temp$Pattern = $Arg0VarName;
    return _mm_cvttss_si32((__m128)temp);
  }]>;

OclBuiltinImpl sse42_convert_i32_rtz_v2v3f32 = OclBuiltinImpl<convert_i32_rtz, [v2f32, v3f32], 0, [{
    float4 temp;
    temp$Pattern = $Arg0VarName;
    return as_int4( _mm_cvttps_epi32(temp) )$Pattern;
  }]>;
  
OclBuiltinImpl sse42_convert_i32_rtz_v4f32 = OclBuiltinImpl<convert_i32_rtz, [v4f32], 0, [{
    return as_$ReturnType( _mm_cvttps_epi32($Arg0VarName) );
  }]>;
// convert_intn_rtz( doublen )
OclBuiltinImpl sse42_convert_i32_rtz_v1f64 = OclBuiltinImpl<convert_i32_rtz, [v1f64], 0, [{
    int4 res;
    double2 param;
    param.lo = $Arg0VarName;
    res = as_int4( _mm_cvttpd_epi32(param) );
    return res.s0;
  }]>;
OclBuiltinImpl sse42_convert_i32_rtz_v2f64 = OclBuiltinImpl<convert_i32_rtz, [v2f64], 0, [{
    return as_int4(_mm_cvttpd_epi32($Arg0VarName)).s01;
  }]>;

// convert_intn_rtz float&double expand
OclBuiltinImpl sse42_convert_i32_rtz_fpexpand = OclBuiltinImpl<convert_i32_rtz,
                            [v16f64], 0, Expand_rtz>;

/*****************************************************************\
 * convert_int_rtp:                                              *
 *   intn convert_intn_rtp (gentypen x)                          *
\*****************************************************************/
// convert_intn_rtp ( doublen ) - expand
OclBuiltinImpl sse42_convert_i32_rtp_f64expand = OclBuiltinImpl<convert_i32_rtp,
                                                       [v2f64, v3f64, v16f64], 0, Expand_rtp>;


/*****************************************************************\
 * convert_int_rtn:                                              *
 *   intn convert_intn_rtn (gentypen x)                          *
\*****************************************************************/
// convert_intn_rtn ( doublen ) - expand
OclBuiltinImpl sse42_convert_i32_rtn_f64expand = OclBuiltinImpl<convert_i32_rtn,
                                                       [v2f64, v3f64, v16f64], 0, Expand_rtn>;

// 32-bit unsign integer output
/*****************************************************************\
 * convert_uint:                                                  *
 *   intn convert_uintn (gentypen x)                              *
\*****************************************************************/
// convert_uintn( floatn ) & convert_uintn( doublen )
OclBuiltinImpl sse42_convert_u32_f32f64 = OclBuiltinImpl<convert_u32, conversion_fp_types, 0, [{
    return convert_$ReturnType_rtz($Arg0VarName);
  }]>;

/*****************************************************************\
 * convert_uint_rte:                                              *
 *   intn convert_uintn_rte (gentypen x)                          *
\*****************************************************************/
// convert_uintn_rte ( doublen )
// f64 - v2 & v3 will be scalarized, v8 & v16 will call v4
// v16f32 will call v8
OclBuiltinImpl sse42_convert_u32_rte_f64expand = OclBuiltinImpl<convert_u32_rte,
                                                       [v2f64, v3f64, v16f64], 0, Expand_rte>;

/*****************************************************************\
 * convert_uint_rtz:                                              *
 *   intn convert_uintn_rtz (gentypen x)                          *
\*****************************************************************/
// convert_uintn_rtz( floatn ) 
OclBuiltinImpl sse42_convert_u32_rtz_v1f32 = OclBuiltinImpl<convert_u32_rtz, [v1f32], 0, [{
    if ($Arg0VarName > maxIntVal32) return (uint)$Arg0VarName;
    float4 p;
    p.s0 = $Arg0VarName;
    uint res = as_uint(_mm_cvttss_si32(p));
    return res;   
  }]>;

OclBuiltinImpl sse42_convert_u32_rtz_v4f32 = OclBuiltinImpl<convert_u32_rtz, [v4f32], 0, [{
    uint4 res;
    int4 mask_gt = as_int4(_mm_cmpgt_ps($Arg0VarName,  __builtin_astype((int4)maxInt32,__m128)));
    if(any(mask_gt)) {
      res.s0 = (uint)$Arg0VarName.s0;
      res.s1 = (uint)$Arg0VarName.s1;
      res.s2 = (uint)$Arg0VarName.s2;
      res.s3 = (uint)$Arg0VarName.s3;
    }
    else {
      res = as_uint4(_mm_cvttps_epi32($Arg0VarName));
    }
    return res;
  }]>;

// convert_intn_rtz float&double expand
OclBuiltinImpl sse42_convert_u32_rtz_fpexpand = OclBuiltinImpl<convert_u32_rtz, 
                                [v16f64], 0, Expand_rtz >;

/*****************************************************************\
 * convert_uint_rtp:                                              *
 *   intn convert_uintn_rtp (gentypen x)                          *
\*****************************************************************/
// convert_uintn_rtp ( doublen ) - expand 
// convert_uintn_rtp ( float16 ) - expand 
OclBuiltinImpl sse42_convert_u32_rtp_fexpand = OclBuiltinImpl<convert_u32_rtp,
                                                       [v16f64], 0, Expand_rtp>;

/*****************************************************************\
 * convert_uint_rtn:                                              *
 *   intn convert_uintn_rtn (gentypen x)                          *
\*****************************************************************/
// convert_uintn_rtn ( doublen ) - expand
// convert_uintn_rtn ( float16 ) - expand
OclBuiltinImpl sse42_convert_u32_rtn_fexpand = OclBuiltinImpl<convert_u32_rtn,
                                                       [v16f64], 0, Expand_rtn>;

// 64-bit integer output
/*****************************************************************\
 * Expand float16, double8, double16                              *
 * longn convert_long_<rounding_mode>:                                  *
 \*****************************************************************/

OclBuiltinImpl sse42_fp2int_i64_f32f64_expand = OclBuiltinImpl<convert_i64,[v16f64], 0, Expand_default>;

OclBuiltinImpl sse42_fp2int_i64_f32_rte_expand = OclBuiltinImpl<convert_i64_rte,[v16f64], 0, Expand_rte>;
                                                                                 
OclBuiltinImpl sse42_fp2int_i64_f32_rtz_expand = OclBuiltinImpl<convert_i64_rtz,[v16f64], 0, Expand_rtz>;
                                                                                 
OclBuiltinImpl sse42_fp2int_i64_f32_rtp_expand = OclBuiltinImpl<convert_i64_rtp,[v16f64], 0, Expand_rtp>;
                                                                                 
OclBuiltinImpl sse42_fp2int_i64_f32_rtn_expand = OclBuiltinImpl<convert_i64_rtn,[v16f64], 0, Expand_rtn>;
  
// 64-bit unsign integer output
/*****************************************************************\
 * Expand float16, double8, double16                              *
 * ulongn convert_ulong_<rounding_mode>:                           *
 \*****************************************************************/

OclBuiltinImpl sse42_fp2int_u64_f32_expand = OclBuiltinImpl<convert_u64,[v16f64], 0, Expand_default>;

OclBuiltinImpl sse42_fp2int_u64_f32_rte_expand = OclBuiltinImpl<convert_u64_rte, [v16f64], 0, Expand_rte>;

OclBuiltinImpl sse42_fp2int_u64_f32_rtz_expand = OclBuiltinImpl<convert_u64_rtz, [v16f64], 0, Expand_rtz>;

OclBuiltinImpl sse42_fp2int_u64_f32_rtp_expand = OclBuiltinImpl<convert_u64_rtp, [v16f64], 0, Expand_rtp>;

OclBuiltinImpl sse42_fp2int_u64_f32_rtn_expand = OclBuiltinImpl<convert_u64_rtn, [v16f64], 0, Expand_rtn>;

// 32-bit floating output

// default rounding mode
/*****************************************************************\
 * convert_float:                                              *
 *   floatn convert_floatn (gentypen x)                          *
\*****************************************************************/

OclBuiltinImpl sse42_cvt_v1f32_v1f64 = OclBuiltinImpl<convert_f32, [v1f64], 0, [{
    double2 n$Arg0VarName;
    n$Arg0VarName$ExpandLoPattern = $Arg0VarName;
    return as_float4(_mm_cvtpd_ps(n$Arg0VarName)).s0;
  }]>;

OclBuiltinImpl sse42_cvt_v2f32_v2f64 = OclBuiltinImpl<convert_f32, [v2f64], 0, [{
    return as_float4(_mm_cvtpd_ps($Arg0VarName))$ExpandLoPattern;
  }]>;
  
OclBuiltinImpl sse42_cvt_f32_f64 = OclBuiltinImpl<convert_f32,
                                          [v16f64], 0, Expand_default>;
  

OclBuiltinImpl sse42_cvt_f32_i8i16 = OclBuiltinImpl<convert_f32,
                                              [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                               v1i16, v2i16, v3i16, v4i16, v8i16, v16i16], 0, [{
    return convert_$ReturnType(convert_$Arg0i32gentype($Arg0VarName));
  }]>;

OclBuiltinImpl sse42_cvt_f32_u8u16 = OclBuiltinImpl<convert_f32,
                                              [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                               v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return convert_$ReturnType(convert_$Arg0i32gentype($Arg0VarName));
  }]>;

OclBuiltinImpl sse42_cvt_f32_i32_promote = OclBuiltinImpl<convert_f32, [v1i32], 0, Promote_default>;

OclBuiltinImpl sse42_cvt_v4f32_v4i32 = OclBuiltinImpl<convert_f32,[v4i32], 0, [{
    return as_$ReturnType(_mm_cvtepi32_ps( __builtin_astype($Arg0VarName,__m128i)));
  }]>;

OclBuiltinImpl sse42_cvt_v1f32_v1u32 = OclBuiltinImpl<convert_f32, [v1u32], 0, [{
    int tlo, thi;
    float flo, fhi;
    tlo = $Arg0VarName & 0xFFFF;
    flo = convert_float(tlo);
    thi = $Arg0VarName >> 16;
    fhi = convert_float(thi);
    return mad(65536.0f, fhi, flo);
  }]>;

OclBuiltinImpl sse42_cvt_v4f32_v4u32 = OclBuiltinImpl<convert_f32,[v4u32, v8u32], 0, [{
    $Arg0igentype tlo, thi;
    $ReturnType flo, fhi;
    tlo = as_$Arg0igentype($Arg0VarName & 0xFFFF);
    flo = convert_$ReturnType(tlo);
    thi = as_$Arg0igentype($Arg0VarName >> 16);
    fhi = convert_$ReturnType(thi);
    return mad(($ReturnType)65536.0f, fhi, flo);
  }]>;

OclBuiltinImpl sse42_cvt_f32_i64 = OclBuiltinImpl<convert_f32,
                                            [v1i64, v4i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl sse42_cvt_f32_u64 = OclBuiltinImpl<convert_f32,
                                            [v1u64, v4u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLFSuffix($Arg0VarName);
  }]>;

// rte rounding mode
/*****************************************************************\
 * convert_float_rte:                                              *
 *   floatn convert_floatn_rte (gentypen x)                          *
\*****************************************************************/

OclBuiltinImpl sse42_rte_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rte,
                                                [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return convert_$ReturnType($Arg0VarName);
  }]>;

OclBuiltinImpl sse42_rte_cvt_f32_i8i16u8u16 = OclBuiltinImpl<convert_f32_rte,
                                                  [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                   v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return convert_$ReturnType_rte(convert_$Arg0i32gentype($Arg0VarName));
  }]>;

// call to convert_float(intn/longn)
OclBuiltinImpl sse42_rte_cvt_f32_i32u32i64 = OclBuiltinImpl<convert_f32_rte,
                                                [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                         v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                         v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
                         v1u64, v2u64, v3u64, v4u64, v8u64, v16u64], 0, [{
    return convert_$ReturnType($Arg0VarName);
  }]>;

// rtz rounding mode
/*****************************************************************\
 * convert_float_rtz:                                              *
 *   floatn convert_floatn_rtz (gentypen x)                        *
\*****************************************************************/

OclBuiltinImpl sse42_rtz_cvt_f32_f64_expand = OclBuiltinImpl<convert_f32_rtz,[v16f64], 0, Expand_rtz>;

OclBuiltinImpl sse42_rtz_cvt_f32_i8i16 = OclBuiltinImpl<convert_f32_rtz,
                                                  [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16, 
                                                   v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return convert_$ReturnType(convert_$Arg0i32gentype_rtz($Arg0VarName));
  }]>;

// rtp rounding mode
/*****************************************************************\
 * convert_float_rtp:                                              *
 *   floatn convert_floatn_rtp (gentypen x)                          *
\*****************************************************************/

OclBuiltinImpl sse42_rtp_cvt_f32_i8i16u8u16 = OclBuiltinImpl<convert_f32_rtp,
                                                  [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                   v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return convert_$ReturnType(convert_$Arg0i32gentype_rtp($Arg0VarName));
  }]>;

OclBuiltinImpl sse42_rtp_cvt_f32_f64i32u32i64u64_expand = OclBuiltinImpl<convert_f32_rtp,[v16f64], 0, Expand_rtp>;

// rtn rounding mode
/*****************************************************************\
 * convert_float_rtn:                                              *
 *   floatn convert_floatn_rtn (gentypen x)                          *
\*****************************************************************/

OclBuiltinImpl sse42_rtn_cvt_f32_f64i32u32i64u64_expand = OclBuiltinImpl<convert_f32_rtn, [v16f64], 0, Expand_rtn>;

OclBuiltinImpl sse42_rtn_cvt_f32_i8i16 = OclBuiltinImpl<convert_f32_rtn,
                                                  [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                   v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return convert_$ReturnType(convert_$Arg0i32gentype_rtn($Arg0VarName));
  }]>;
  
// 64-bit floating output
// default rounding mode
/*****************************************************************\
 * convert_double:                                              *
 *   doublen convert_doublen (gentypen x)                          *
\*****************************************************************/

OclBuiltinImpl sse42_cvt_f64_v1f32 = OclBuiltinImpl<convert_f64, [v1f32], 0, [{
    float4 param;
    double2 res;
    param.s0 = $Arg0VarName;
    res = _mm_cvtps_pd(param);
    return res.lo;
  }]>;

OclBuiltinImpl sse42_cvt_f64_v2f32 = OclBuiltinImpl<convert_f64, [v2f32], 0, [{
    float4 param;
    double2 res;
    param.lo = $Arg0VarName;
    res = _mm_cvtps_pd(param);
    return res;
  }]>;


OclBuiltinImpl sse42_cvt_f64_f32i32u32i64u64_expand = OclBuiltinImpl<convert_f64, [v8f32, v16f32, v16i32, 
                                       v16u32, v16i64, v16u64], 0, Expand_default>;

OclBuiltinImpl sse42_cvt_f64_v1i32 = OclBuiltinImpl<convert_f64,[v1i32], 0, [{
    double2 res;
    int4 param;
    param.s0 = $Arg0VarName;
    res = as_double2(_mm_cvtepi32_pd(__builtin_astype(param,__m128i)));
    return res.lo;
  }]>;

OclBuiltinImpl sse42_cvt_f64_v2i32 = OclBuiltinImpl<convert_f64,[v2i32], 0, [{
    double2 res;
    int4 param;
    param.lo = $Arg0VarName;
    res = as_double2(_mm_cvtepi32_pd(__builtin_astype(param,__m128i)));
    return res;
  }]>;

OclBuiltinImpl sse42_cvt_f64_i8i16u8u16 = OclBuiltinImpl<convert_f64,
                                                 [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                  v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                  v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                  v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return convert_$ReturnType(convert_$Arg0i32gentype($Arg0VarName));
  }]>;

OclBuiltinImpl sse42_cvt_f64_u32 = OclBuiltinImpl<convert_f64,
                                                 [ v1u32, v4u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtn$VecLength($Arg0VarName);
  }]>;

// rte rounding mode
/*****************************************************************\
 * convert_double_rte:                                            *
 *   doublen convert_doublen_rte (gentypen x)                     *
\*****************************************************************/

OclBuiltinImpl sse42_rte_cvt_f64_f32 = OclBuiltinImpl<convert_f64_rte,
                                                    [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    return convert_$ReturnType($Arg0VarName);
  }]>;

OclBuiltinImpl sse42_rte_cvt_f64_f32i32u32i64u64_expand = OclBuiltinImpl<convert_f64_rte, [v16i32, 
                                       v16u32, v16i64, v16u64], 0, Expand_rte>;

OclBuiltinImpl sse42_rte_cvt_f64_i32 = OclBuiltinImpl<convert_f64_rte,
                                                     [v1i32, v4i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtn$VecLength($Arg0VarName);
  }]>;

OclBuiltinImpl sse42_rte_cvt_f64_i8i16u8u16 = OclBuiltinImpl<convert_f64_rte,
                                                     [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                      v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                      v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                      v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return convert_$ReturnType(convert_$Arg0i32gentype_rte($Arg0VarName));
  }]>;

OclBuiltinImpl sse42_rte_cvt_f32_u32 = OclBuiltinImpl<convert_f64_rte,
                                                     [v1u32, v4u32], 0, [{
   return __ocl_svml_$Target_cvtu32tofp$rtn$VecLength($Arg0VarName);
  }]>;

// rtz rounding mode
/*****************************************************************\
 * convert_double_rtz:                                            *
 *   doublen convert_doublen_rtz (gentypen x)                     *
\*****************************************************************/

OclBuiltinImpl sse42_rtz_cvt_f64_f32 = OclBuiltinImpl<convert_f64_rtz,
                                                    [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    return convert_$ReturnType($Arg0VarName);
  }]>;

OclBuiltinImpl sse42_rtz_cvt_f64_f32i32u32i64u64_expand = OclBuiltinImpl<convert_f64_rtz, [v16i32, 
                                       v16u32, v16i64, v16u64], 0, Expand_rtz>;

OclBuiltinImpl sse42_rtz_cvt_f64_i8i16u8u16 = OclBuiltinImpl<convert_f64_rtz,
                                                     [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                      v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                      v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                      v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
     return convert_$ReturnType(convert_$Arg0i32gentype_rtz($Arg0VarName));
  }]>;

OclBuiltinImpl sse42_rtz_cvt_f64_u32 = OclBuiltinImpl<convert_f64_rtz,
                                                     [v1u32, v4u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtz$VecLength($Arg0VarName);
  }]>;

OclBuiltinImpl sse42_rtz_cvt_f64_i32 = OclBuiltinImpl<convert_f64_rtz,
                                                     [v1i32, v4i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtz$VecLength($Arg0VarName);
  }]>;


// rtp rounding mode
/*****************************************************************\
 * convert_double_rtp:                                            *
 *   doublen convert_doublen_rtp (gentypen x)                     *
\*****************************************************************/

OclBuiltinImpl sse42_rtp_cvt_f64_f32 = OclBuiltinImpl<convert_f64_rtp,
                                                    [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    return convert_$ReturnType($Arg0VarName);
  }]>;

OclBuiltinImpl sse42_rtp_cvt_f64_i8i16u8u16 = OclBuiltinImpl<convert_f64_rtp,
                                                     [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                      v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                      v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                      v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return convert_$ReturnType(convert_$Arg0i32gentype_rtp($Arg0VarName));
  }]>;

OclBuiltinImpl sse42_rtp_cvt_f64_i32 = OclBuiltinImpl<convert_f64_rtp,
                                                     [v1i32, v4i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$up$VecLength($Arg0VarName);
  }]>;
  
OclBuiltinImpl sse42_rtp_cvt_f64_u32 = OclBuiltinImpl<convert_f64_rtp,
                                                     [v1u32, v4u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$up$VecLength($Arg0VarName);
  }]>;
  
OclBuiltinImpl sse42_rtp_cvt_f64_f32i32u32i64u64_expand = OclBuiltinImpl<convert_f64_rtp, [v16i32, 
                                       v16u32, v16i64, v16u64], 0, Expand_rtp>;

// rtn rounding mode
/*****************************************************************\
 * convert_double_rtn:                                            *
 *   doublen convert_doublen_rtn (gentypen x)                     *
\*****************************************************************/

OclBuiltinImpl sse42_rtn_cvt_f64_f32 = OclBuiltinImpl<convert_f64_rtn,
                                                    [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    return convert_$ReturnType($Arg0VarName);
  }]>;

OclBuiltinImpl sse42_rtn_cvt_f64_i8i16u8u16 = OclBuiltinImpl<convert_f64_rtn,
                                                     [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                      v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                      v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                      v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return convert_$ReturnType(convert_$Arg0i32gentype_rtn($Arg0VarName));
  }]>;

OclBuiltinImpl sse42_rtn_cvt_f64_i32 = OclBuiltinImpl<convert_f64_rtn,
                                                     [ v1i32, v4i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$down$VecLength($Arg0VarName);
  }]>;

OclBuiltinImpl sse42_rtn_cvt_f64_u32 = OclBuiltinImpl<convert_f64_rtn,
                                                     [v1u32, v4u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$down$VecLength($Arg0VarName);
  }]>;
  
OclBuiltinImpl sse42_rtn_cvt_f64_f32i32u32i64u64_expand = OclBuiltinImpl<convert_f64_rtn, [v16i32, 
                                       v16u32, v16i64, v16u64], 0, Expand_rtn>;
