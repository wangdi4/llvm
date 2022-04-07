/*****************************************************************\
 * convert_int_rte:                                              *
 *   intn convert_intn_rte (gentypen x)                          *
\*****************************************************************/
// convert_intn_rte ( doublen )
// v16 will call v8
OclBuiltinImpl avx_fp2int_i32_f32f64_rte = OclBuiltinImpl<convert_i32_rte,
                                                   [v16f32, v8f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$rtnnosat$SVMLSuffix($Arg0VarName);
  }]>;
  
/*****************************************************************\
 * convert_int_rtz:                                              *
 *   intn convert_intn_rtz (gentypen x)                          *
\*****************************************************************/
OclBuiltinImpl avx_convert_i32_rtz_v8f32 = OclBuiltinImpl<convert_i32_rtz, [v8f32], 0, [{
    return as_$ReturnType( _mm256_cvttps_epi32($Arg0VarName) );
  }]>;

OclBuiltinImpl avx_convert_i32_rtz_v4f64 = OclBuiltinImpl<convert_i32_rtz, [v4f64], 0, [{
    return as_$ReturnType(_mm256_cvttpd_epi32($Arg0VarName));
  }]>;
 
OclBuiltinImpl avx_convert_i32_rtz_v3f64 = OclBuiltinImpl<convert_i32_rtz, [v3f64], 0, [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnBaseType#4 res = convert_$ReturnBaseType#4_rtz(n$Arg0VarName);
    return res$Pattern;
  }]>;

OclBuiltinImpl avx_convert_i32_rtz_v4f64 = OclBuiltinImpl<convert_i32_rtz, [v4f64], 0, [{
    return as_int4(_mm256_cvttpd_epi32($Arg0VarName));
  }]>;

/*****************************************************************\
 * convert_int_rtp:                                              *
 *   intn convert_intn_rtp (gentypen x)                          *
\*****************************************************************/
OclBuiltinImpl avx_convert_i32_rtp_f64f32 = OclBuiltinImpl<convert_i32_rtp, [v8f64, v16f32], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$upnosat$SVMLSuffix($Arg0VarName);                             
  }]>;
/*****************************************************************\
 * convert_int_rtn:                                              *
 *   intn convert_intn_rtn (gentypen x)                          *
\*****************************************************************/
OclBuiltinImpl avx_convert_i32_rtn_f64f32 = OclBuiltinImpl<convert_i32_rtn, [v8f64, v16f32], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$downnosat$SVMLSuffix($Arg0VarName);
  }]>;
// 32-bit unsign integer output
/*****************************************************************\
 * convert_uint_rte:                                              *
 *   intn convert_uintn_rte (gentypen x)                          *
\*****************************************************************/
OclBuiltinImpl avx_convert_u32_rte_f64 = OclBuiltinImpl<convert_u32_rte,[v8f64, v16f32], 0, [{
    return __ocl_svml_$Target_cvtfptou32$rtnnosat$SVMLSuffix($Arg0VarName);
  }]>;
/*****************************************************************\
 * convert_uint_rtz:                                              *
 *   intn convert_uintn_rtz (gentypen x)                          *
\*****************************************************************/

OclBuiltinImpl avx_convert_u32_rtz_v8f32 = OclBuiltinImpl<convert_u32_rtz, [v8f32], 0, [{
    uint8 res;
    int8 mask_gt = as_int8(_mm256_cmp_ps($Arg0VarName,  __builtin_astype((int8)maxInt32,__m256), _CMP_GT_OS ));
    if(any(mask_gt)){
      res.s0 = (uint)$Arg0VarName.s0;
      res.s1 = (uint)$Arg0VarName.s1;
      res.s2 = (uint)$Arg0VarName.s2;
      res.s3 = (uint)$Arg0VarName.s3;
      res.s4 = (uint)$Arg0VarName.s4;
      res.s5 = (uint)$Arg0VarName.s5;
      res.s6 = (uint)$Arg0VarName.s6;
      res.s7 = (uint)$Arg0VarName.s7;
    }
    else{
      res = as_uint8(_mm256_cvttps_epi32($Arg0VarName));
    }
    return res;
  }]>;

// convert_intn_rtz double
OclBuiltinImpl avx_convert_u32_rtz_fpexpand = OclBuiltinImpl<convert_u32_rtz, [v8f64], 0, [{
    return __ocl_svml_$Target_cvtfptou32$rtznosat$SVMLSuffix($Arg0VarName);                                
  }]>;

/*****************************************************************\
 * convert_uint_rtn:                                              *
 *   intn convert_uintn_rtn (gentypen x)                          *
\*****************************************************************/
// convert_uintn_rtn ( doublen ) 
// convert_uintn_rtn ( float16 ) 
OclBuiltinImpl avx_convert_u32_rtn_v8f64_v16f32 = OclBuiltinImpl<convert_u32_rtn,
                                                       [v8f64, v16f32], 0, [{
    return __ocl_svml_$Target_cvtfptou32$downnosat$SVMLSuffix($Arg0VarName);
  }]>;
/*****************************************************************\
 * convert_uint_rtp:                                              *
 *   intn convert_uintn_rtp (gentypen x)                          *
\*****************************************************************/
// convert_uintn_rtp ( double8 )
// convert_uintn_rtp ( float16 )
OclBuiltinImpl avx_convert_u32_rtp_v8f64_v16f32 = OclBuiltinImpl<convert_u32_rtp,
                            [v8f64, v16f32], 0, [{
    return __ocl_svml_$Target_cvtfptou32$upnosat$SVMLSuffix($Arg0VarName);
  }]>;

// 64-bit integer output
/*****************************************************************\
 * Replacing the Expand from SSE42 back to SVML in: float16, double8 *
 * longn convert_long_<rounding_mode>:                               *
 \*****************************************************************/

OclBuiltinImpl avx_fp2int_i64_f32f64 = OclBuiltinImpl<convert_i64,
                                                   [v16f32, v8f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi64$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;
  
OclBuiltinImpl avx_fp2int_i64_f32_rte = OclBuiltinImpl<convert_i64_rte,
                                                   [v16f32, v8f64], 0,  [{
    return __ocl_svml_$Target_cvtfptoi64$rtnnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_fp2int_i64_f32_rtz = OclBuiltinImpl<convert_i64_rtz,
                                                   [v16f32, v8f64], 0,  [{
    return __ocl_svml_$Target_cvtfptoi64$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;
  
OclBuiltinImpl avx_fp2int_i64_f32_rtp = OclBuiltinImpl<convert_i64_rtp,
                                                   [v16f32, v8f64], 0,  [{
    return __ocl_svml_$Target_cvtfptoi64$upnosat$SVMLSuffix($Arg0VarName);
  }]>;
  
OclBuiltinImpl avx_fp2int_i64_f32_rtn = OclBuiltinImpl<convert_i64_rtn,
                                                   [v16f32, v8f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi64$downnosat$SVMLSuffix($Arg0VarName);
  }]>;
  
// 64-bit unsign integer output
/*****************************************************************\
 * Replacing the Expand from SSE42 back to SVML in: float16, double8*
 * ulongn convert_ulong_<rounding_mode>:                           *
 \*****************************************************************/
 
OclBuiltinImpl avx_fp2int_u64_f32 = OclBuiltinImpl<convert_u64,
                                                   [v16f32, v8f64], 0, [{
    return  __ocl_svml_$Target_cvtfptou64$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;
  
OclBuiltinImpl avx_fp2int_u64_f32_rte = OclBuiltinImpl<convert_u64_rte,
                                                   [v16f32, v8f64], 0,  [{
    return  __ocl_svml_$Target_cvtfptou64$rtnnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_fp2int_u64_f32_rtz = OclBuiltinImpl<convert_u64_rtz,
                                                   [v16f32, v8f64], 0, [{
    return  __ocl_svml_$Target_cvtfptou64$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_fp2int_u64_f32_rtp = OclBuiltinImpl<convert_u64_rtp,
                                                   [v16f32, v8f64], 0, [{
    return  __ocl_svml_$Target_cvtfptou64$upnosat$SVMLSuffix($Arg0VarName);
  }]>;
  
OclBuiltinImpl avx_fp2int_u64_f32_rtn = OclBuiltinImpl<convert_u64_rtn,
                                                   [v16f32, v8f64], 0, [{
    return  __ocl_svml_$Target_cvtfptou64$downnosat$SVMLSuffix($Arg0VarName);
  }]>;

// 32-bit floating output
// default rounding mode
/*****************************************************************\
 * convert_float:                                              *
 *   floatn convert_floatn (gentypen x)                          *
\*****************************************************************/

OclBuiltinImpl avx_cvt_v4f32_v4f64 = OclBuiltinImpl<convert_f32, [v4f64], 0, [{
  return as_$ReturnType(_mm256_cvtpd_ps($Arg0VarName));
  }]>;

OclBuiltinImpl avx_cvt_v8f32_v8i32 = OclBuiltinImpl<convert_f32,[v8i32], 0, [{
    return as_$ReturnType(_mm256_cvtepi32_ps( __builtin_astype($Arg0VarName,__m256i)));
  }]>;
  
OclBuiltinImpl avx_cvt_f32_i64 = OclBuiltinImpl<convert_f32,
                                            [v8i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_cvt_f32_u64 = OclBuiltinImpl<convert_f32,
                                            [v8u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLFSuffix($Arg0VarName);
  }]>;

// rtz rounding mode
/*****************************************************************\
 * convert_float_rtz:                                              *
 *   floatn convert_floatn_rtz (gentypen x)                        *
\*****************************************************************/

OclBuiltinImpl avx_rtz_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rtz,
                                                [v8f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$rtz$SVMLSuffix($Arg0VarName);
  }]>;
  
OclBuiltinImpl avx_rtz_cvt_f32_i32 = OclBuiltinImpl<convert_f32_rtz,
                                                [v16i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtz$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtz_cvt_f32_u32 = OclBuiltinImpl<convert_f32_rtz,
                                                [v16u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtz$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtz_cvt_f32_i64 = OclBuiltinImpl<convert_f32_rtz,
                                                [v8i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtz$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtz_cvt_f32_u64 = OclBuiltinImpl<convert_f32_rtz,
                                                [v8u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtz$SVMLFSuffix($Arg0VarName);
  }]>;

// rtp rounding mode
/*****************************************************************\
 * convert_float_rtp:                                              *
 *   floatn convert_floatn_rtp (gentypen x)                          *
\*****************************************************************/

OclBuiltinImpl avx_rtp_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rtp,
                                                [v8f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$up$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtp_cvt_f32_i32 = OclBuiltinImpl<convert_f32_rtp,
                                                [v16i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$up$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtp_cvt_f32_u32 = OclBuiltinImpl<convert_f32_rtp,
                                                [v16u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$up$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtp_cvt_f32_i64 = OclBuiltinImpl<convert_f32_rtp,
                                                [v8i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$up$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtp_cvt_f32_u64 = OclBuiltinImpl<convert_f32_rtp,
                                                [v8u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$up$SVMLFSuffix($Arg0VarName);
  }]>;

// rtn rounding mode
/*****************************************************************\
 * convert_float_rtn:                                              *
 *   floatn convert_floatn_rtn (gentypen x)                          *
\*****************************************************************/

OclBuiltinImpl avx_rtn_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rtn,
                                                [v8f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$down$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtn_cvt_f32_i32 = OclBuiltinImpl<convert_f32_rtn,
                                                [v16i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$down$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtn_cvt_f32_u32 = OclBuiltinImpl<convert_f32_rtn,
                                                [v16u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$down$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtn_cvt_f32_i64 = OclBuiltinImpl<convert_f32_rtn,
                                                [v8i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$down$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtn_cvt_f32_u64 = OclBuiltinImpl<convert_f32_rtn,
                                                [v8u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$down$SVMLFSuffix($Arg0VarName);
  }]>;

// 64-bit floating output
// default rounding mode
/*****************************************************************\
 * convert_double:                                              *
 *   doublen convert_doublen (gentypen x)                          *
\*****************************************************************/

OclBuiltinImpl avx_cvt_f64_v3f32 = OclBuiltinImpl<convert_f64, [v3f32], 0, [{
    float4 n$Arg0VarName = as_float4($Arg0VarName);
    return as_$ReturnType(_mm256_cvtps_pd(__builtin_astype(n$Arg0VarName,__m128)));
  }]>;

OclBuiltinImpl avx_cvt_f64_v4f32 = OclBuiltinImpl<convert_f64, [v4f32], 0, [{
    return as_$ReturnType(_mm256_cvtps_pd(__builtin_astype($Arg0VarName,__m128)));
  }]>;

OclBuiltinImpl avx_cvt_f64_v3i32 = OclBuiltinImpl<convert_f64,[v3i32], 0, [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnBaseType#4 res = convert_$ReturnBaseType#4(n$Arg0VarName);
    return res$Pattern;
  }]>;

OclBuiltinImpl avx_cvt_f64_v4i32 = OclBuiltinImpl<convert_f64,[v4i32], 0, [{
    return as_double4(_mm256_cvtepi32_pd(__builtin_astype(x,__m128i)));
  }]>;

OclBuiltinImpl avx_cvt_f32_u8u16u32 = OclBuiltinImpl<convert_f64,[v8u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtn$VecLength($Arg0VarName);
  }]>;

OclBuiltinImpl avx_cvt_f64_i64 = OclBuiltinImpl<convert_f64, [v8i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_cvt_f64_u64 = OclBuiltinImpl<convert_f64, [v8u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

// rte rounding mode
/*****************************************************************\
 * convert_double_rte:                                            *
 *   doublen convert_doublen_rte (gentypen x)                     *
\*****************************************************************/

OclBuiltinImpl avx_rte_cvt_f64_i32 = OclBuiltinImpl<convert_f64_rte,
                                                     [v8i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtn$VecLength($Arg0VarName);
  }]>;
  
OclBuiltinImpl avx_rte_cvt_f32_u32 = OclBuiltinImpl<convert_f64_rte,
                                                     [v8u32], 0, [{
   return __ocl_svml_$Target_cvtu32tofp$rtn$VecLength($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rte_cvt_f64_i64 = OclBuiltinImpl<convert_f64_rte,
                                                [v8i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rte_cvt_f64_u64 = OclBuiltinImpl<convert_f64_rte,
                                                [v8u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

// rtz rounding mode
/*****************************************************************\
 * convert_double_rtz:                                            *
 *   doublen convert_doublen_rtz (gentypen x)                     *
\*****************************************************************/

OclBuiltinImpl avx_rtz_cvt_f64_u32 = OclBuiltinImpl<convert_f64_rtz,[v8u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtz$VecLength($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtz_cvt_f64_i32 = OclBuiltinImpl<convert_f64_rtz,[v8i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtz$VecLength($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtz_cvt_f64_i64 = OclBuiltinImpl<convert_f64_rtz,[v8i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtz$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtz_cvt_f64_u64 = OclBuiltinImpl<convert_f64_rtz, [v8u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtz$SVMLSuffix($Arg0VarName);
  }]>;

// rtp rounding mode
/*****************************************************************\
 * convert_double_rtp:                                            *
 *   doublen convert_doublen_rtp (gentypen x)                     *
\*****************************************************************/

OclBuiltinImpl avx_rtp_cvt_f64_u32 = OclBuiltinImpl<convert_f64_rtp,[v8u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$up$VecLength($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtp_cvt_f64_i32 = OclBuiltinImpl<convert_f64_rtp,[v8i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$up$VecLength($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtp_cvt_f64_i64 = OclBuiltinImpl<convert_f64_rtp,[v8i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$up$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtp_cvt_f64_u64 = OclBuiltinImpl<convert_f64_rtp,[v8u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$up$SVMLSuffix($Arg0VarName);
  }]>;

// rtn rounding mode
/*****************************************************************\
 * convert_double_rtn:                                            *
 *   doublen convert_doublen_rtn (gentypen x)                     *
\*****************************************************************/

OclBuiltinImpl avx_rtn_cvt_f64_i32 = OclBuiltinImpl<convert_f64_rtn,[v8i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$down$VecLength($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtn_cvt_f64_u32 = OclBuiltinImpl<convert_f64_rtn,[v8u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$down$VecLength($Arg0VarName);
  }]>;
OclBuiltinImpl avx_rtn_cvt_f64_i64 = OclBuiltinImpl<convert_f64_rtn,[v8i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$down$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl avx_rtn_cvt_f64_u64 = OclBuiltinImpl<convert_f64_rtn,[v8u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$down$SVMLSuffix($Arg0VarName);
  }]>;
  
