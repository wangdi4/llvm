// Copyright (C) 2016 Intel Corporation
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

//////////////////////////////////////////////////////////////////////////////////////////////
// AVX-512F math implementation
//////////////////////////////////////////////////////////////////////////////////////////////

// ceil float16
OclBuiltinImpl ceil_v16f32_impl = OclBuiltinImpl<ceil, [v16f32], 0, [{
  return _mm512_ceil_ps($Arg0VarName);
  }]>;

// ceil double8
OclBuiltinImpl ceil_v8f64_impl = OclBuiltinImpl<ceil, [v8f64], 0, [{
  return _mm512_ceil_pd($Arg0VarName);
  }]>;

// floor float16
OclBuiltinImpl floor_v16f32_impl = OclBuiltinImpl<floor, [v16f32], 0, [{
  return _mm512_floor_ps($Arg0VarName);
  }]>;

// floor double8
OclBuiltinImpl floor_v8f64_impl = OclBuiltinImpl<floor, [v8f64], 0, [{
  return _mm512_floor_pd($Arg0VarName);
  }]>;

// fma float16
OclBuiltinImpl fma_v16f32_impl = OclBuiltinImpl<fma, [v16f32], 0, [{
    return _mm512_fmadd_ps($Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;

// fma double8
OclBuiltinImpl fma_v8f64_impl = OclBuiltinImpl<fma, [v8f64], 0, [{
    return _mm512_fmadd_pd($Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;

OclBuiltinImpl rint_v8f64_impl = OclBuiltinImpl<rint, [v8f64], 0, [{
  return _mm512_roundscale_pd($Arg0VarName, _MM_FROUND_TO_NEAREST_INT);
  }]>;
OclBuiltinImpl rint_v16f32_impl = OclBuiltinImpl<rint, [v16f32], 0, [{
  return _mm512_roundscale_ps($Arg0VarName, _MM_FROUND_TO_NEAREST_INT);
  }]>;

// sqrt
OclBuiltinImpl sqrt_v8f64_impl = OclBuiltinImpl<sqrt, [v8f64], 0, [{
    return _mm512_sqrt_pd($Arg0VarName);
  }]>;

OclBuiltinImpl sqrt_v16f32_impl = OclBuiltinImpl<sqrt, [v16f32], 0, [{
    return _mm512_sqrt_ps($Arg0VarName);
  }]>;

// SVML calls

list<OclType> math_FP512_types = [v16f32, v32f32, v8f64, v16f64];

// modf
OclBuiltinImpl modf_ret2ptr_pas = OclBuiltinImpl<modf_pas2, math_FP512_types, 0, modf_ret2ptr>;

// sincos
OclBuiltinImpl sincos_ret2ptr_pas = OclBuiltinImpl<sincos_pas2, math_FP512_types, 0, sincos_ret2ptr>;

// TODO: calls to trunc svml functions can be replaced with avx512f intrinsic
// after upgrade to llvm supported it to avoid redundant calls
/* trunc float16
OclBuiltinImpl trunc_v16f32_impl = OclBuiltinImpl<trunc, [v16f32], 0, [{
  // trancate, round to nearest smallest magnitude integer
    return _mm512_roundscale_round_ps($Arg0VarName, 0x3, _MM_FROUND_TO_ZERO);
  }]>;

// trunc double
OclBuiltinImpl trunc_v8f64_impl = OclBuiltinImpl<trunc, [v8f64], 0, [{
  // trancate, round to nearest smallest magnitude integer
    return _mm512_roundscale_round_pd($Arg0VarName, 0x3, _MM_FROUND_TO_ZERO);
  }]>;
*/
// trunc
OclBuiltinImpl trunc_svml = OclBuiltinImpl<trunc, math_FP512_types, 0, [{
    return __ocl_svml_$Target_trunc$SVMLSuffix($Arg0VarName);
  }]>;

// half_cos
OclBuiltinImpl half_cos_svml = OclBuiltinImpl<half_cos, [v16f32], 0, [{
    return __ocl_svml_$Target_cos$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_divide
OclBuiltinImpl half_divide_svml = OclBuiltinImpl<half_divide, [v16f32], 0, [{
    return __ocl_svml_$Target_div$SVMLSuffix_half($Arg0VarName, $Arg1VarName);
  }]>;

// half_exp
OclBuiltinImpl half_exp_svml = OclBuiltinImpl<half_exp, [v16f32], 0, [{
    return __ocl_svml_$Target_exp$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_exp2
OclBuiltinImpl half_exp2_svml = OclBuiltinImpl<half_exp2, [v16f32], 0, [{
    return __ocl_svml_$Target_exp2$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_exp10
OclBuiltinImpl half_exp10_svml = OclBuiltinImpl<half_exp10, [v16f32], 0, [{
    return __ocl_svml_$Target_exp10$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_log
OclBuiltinImpl half_log_svml = OclBuiltinImpl<half_log, [v16f32], 0, [{
    return __ocl_svml_$Target_log$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_log2
OclBuiltinImpl half_log2_svml = OclBuiltinImpl<half_log2, [v16f32], 0, [{
    return __ocl_svml_$Target_log2$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_log10
OclBuiltinImpl half_log10_svml = OclBuiltinImpl<half_log10, [v16f32], 0, [{
    return __ocl_svml_$Target_log10$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_powr
OclBuiltinImpl half_powr_svml = OclBuiltinImpl<half_powr, [v16f32], 0, [{
    return __ocl_svml_$Target_powr$SVMLSuffix_half($Arg0VarName, $Arg1VarName);
  }]>;

// half_sin
OclBuiltinImpl half_sin_svml = OclBuiltinImpl<half_sin, [v16f32], 0, [{
    return __ocl_svml_$Target_sin$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_tan
OclBuiltinImpl half_tan_svml = OclBuiltinImpl<half_tan, [v16f32], 0, [{
    return __ocl_svml_$Target_tan$SVMLSuffix_half($Arg0VarName);
  }]>;

// masked
defvar v32v64f32Types = ExpandTypesByVFAndFlatten<[v1f32], [32, 64]>.Tout;
OclBuiltinImpl ilogb_mask_svml_float = OclBuiltinImpl<ilogb_mask, v32v64f32Types, 0, masked_impl_code_I32_FG_M, [["$DEF0", "1"]]>;
OclBuiltinImpl ldexp_mask_svml_float = OclBuiltinImpl<ldexp_mask, v32v64f32Types, 0, masked_impl_code_FG_FG_I32_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
OclBuiltinImpl pown_mask_svml_float = OclBuiltinImpl<pown_mask, v32v64f32Types, 0, masked_impl_code_FG_FG_I32_M, [["$DEF0", "5"], ["$DEF1", "5"]]>;
OclBuiltinImpl rootn_mask_svml_float = OclBuiltinImpl<rootn_mask, v32v64f32Types, 0, masked_impl_code_FG_FG_I32_M, [["$DEF0", "1"], ["$DEF1", "1"]]>;
defvar v32v64f64Types = ExpandTypesByVFAndFlatten<[v1f64], [32, 64]>.Tout;
OclBuiltinImpl ilogb_mask_svml_double = OclBuiltinImpl<ilogb_mask, v32v64f64Types, 0, masked_impl_code_I32_DG_M, [["$DEF0", "1"]]>;
OclBuiltinImpl ldexp_mask_svml_double = OclBuiltinImpl<ldexp_mask, v32v64f64Types, 0, masked_impl_code_DG_DG_I32_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
OclBuiltinImpl pown_mask_svml_double = OclBuiltinImpl<pown_mask, v32v64f64Types, 0, masked_impl_code_DG_DG_I32_M, [["$DEF0", "5"], ["$DEF1", "5"]]>;
OclBuiltinImpl rootn_mask_svml_double = OclBuiltinImpl<rootn_mask, v32v64f64Types, 0, masked_impl_code_DG_DG_I32_M, [["$DEF0", "1"], ["$DEF1", "1"]]>;

// native

OclBuiltinImpl native_divide_v16f32_impl = OclBuiltinImpl<native_divide, [v16f32], 0, [{
    $ReturnType $ReturnVarName = _mm512_rcp14_ps($Arg1VarName);
    return $Arg0VarName * $ReturnVarName;
  }]>;

// native divide double8
OclBuiltinImpl native_divide_svml = OclBuiltinImpl<native_divide, [v8f64], 0, [{
    return __ocl_svml_$Target_div$VecLength($Arg0VarName, $Arg1VarName);
  }]>;

OclBuiltinImpl native_recip_v16f32_impl = OclBuiltinImpl<native_recip, [v16f32], 0, [{
    return _mm512_rcp14_ps($Arg0VarName);
  }]>;

// native recip double8
OclBuiltinImpl native_recip_svml = OclBuiltinImpl<native_recip, [v8f64], 0, [{
    return __ocl_svml_$Target_inv$VecLength($Arg0VarName);
  }]>;

// native sqrt
OclBuiltinImpl native_sqrt_v16f32_impl = OclBuiltinImpl<native_sqrt, [v16f32], 0, [{
    return sqrt($Arg0VarName);
  }]>;

OclBuiltinImpl native_sqrt_v8f64_impl = OclBuiltinImpl<native_sqrt, [v8f64], 0, [{
    return sqrt($Arg0VarName);
  }]>;

// native rsqrt
OclBuiltinImpl native_rsqrt_v16f32_impl = OclBuiltinImpl<native_rsqrt, [v16f32], 0, [{
    return _mm512_rsqrt14_ps($Arg0VarName);
  }]>;

OclBuiltinImpl native_rsqrt_svml = OclBuiltinImpl<native_rsqrt, [v8f64], 0, [{
    return _mm512_rsqrt14_pd($Arg0VarName);
  }]>;

OclBuiltinImpl native_sincos_pas_svml = OclBuiltinImpl<native_sincos_pas, [v32f32], 0, svml_native_code_sincos>;
OclBuiltinImpl native_sincos_las_svml = OclBuiltinImpl<native_sincos_las, [v32f32], 0, svml_native_code_sincos>;
OclBuiltinImpl native_sincos_gas_svml = OclBuiltinImpl<native_sincos_gas, [v32f32], 0, svml_native_code_sincos>;
OclBuiltinImpl native_sincos_gen_svml = OclBuiltinImpl<native_sincos_gen, [v32f32], 0, svml_native_code_sincos>;
OclBuiltinImpl native_sincos_ret2ptr_pas = OclBuiltinImpl<native_sincos_pas2, [v32f32], 0, native_sincos_ret2ptr>;

// native masked
OclBuiltinImpl native_ilogb_mask_svml_float = OclBuiltinImpl<native_ilogb_mask, v32v64f32Types, 0, masked_impl_code_I32_FG_M, [["$DEF0", "1"]]>;
OclBuiltinImpl native_pown_mask_svml_float = OclBuiltinImpl<native_pown_mask, v32v64f32Types, 0, masked_impl_code_FG_FG_I32_M, [["$DEF0", "5"], ["$DEF1", "5"]]>;
OclBuiltinImpl native_rootn_mask_svml_float = OclBuiltinImpl<native_rootn_mask, v32v64f32Types, 0, masked_impl_code_FG_FG_I32_M, [["$DEF0", "1"], ["$DEF1", "1"]]>;

OclBuiltinImpl native_ilogb_mask_svml_double = OclBuiltinImpl<native_ilogb_mask, v32v64f64Types, 0, masked_impl_code_I32_DG_M, [["$DEF0", "1"]]>;
OclBuiltinImpl native_pown_mask_svml_double = OclBuiltinImpl<native_pown_mask, v32v64f64Types, 0, masked_impl_code_DG_DG_I32_M, [["$DEF0", "5"], ["$DEF1", "5"]]>;
OclBuiltinImpl native_rootn_mask_svml_double = OclBuiltinImpl<native_rootn_mask, v32v64f64Types, 0, masked_impl_code_DG_DG_I32_M, [["$DEF0", "1"], ["$DEF1", "1"]]>;

// integer

// integer remainder
OclBuiltinImpl irem_svml = OclBuiltinImpl<irem, [v16i32], 0, [{
    return __ocl_svml_$Target_irem$VecLength($Arg0VarName, $Arg1VarName);
  }]>;

// unsigned integer remainder
OclBuiltinImpl urem_svml = OclBuiltinImpl<urem, [v16u32], 0, [{
    return __ocl_svml_$Target_urem$VecLength($Arg0VarName, $Arg1VarName);
  }]>;

// integer division
OclBuiltinImpl idiv_svml = OclBuiltinImpl<idiv, [v16i32], 0, [{
    return __ocl_svml_$Target_idiv$VecLength($Arg0VarName, $Arg1VarName);
  }]>;

// unsigned integer division
OclBuiltinImpl udiv_svml = OclBuiltinImpl<udiv, [v16u32], 0, [{
    return __ocl_svml_$Target_udiv$VecLength($Arg0VarName, $Arg1VarName);
  }]>;

// misc
OclBuiltinImpl intel_movemask_v32i8 = OclBuiltinImpl<intel_movemask, [v32i8], 0, [{
    return as_int(_mm256_movepi8_mask((__m256i)$Arg0VarName));
  }]>;
OclBuiltinImpl intel_movemask_v64i8 = OclBuiltinImpl<intel_movemask_v64, [v64i8], 0, [{
    return as_long(_mm512_movepi8_mask((__m512i)$Arg0VarName));
  }]>;
OclBuiltinImpl intel_movemask_v16i16 = OclBuiltinImpl<intel_movemask, [v16i16], 0, [{
    return convert_int(as_short(_mm256_movepi16_mask((__m256i)$Arg0VarName)));
  }]>;
OclBuiltinImpl intel_movemask_v32i16 = OclBuiltinImpl<intel_movemask, [v32i16], 0, [{
    return as_int(_mm512_movepi16_mask((__m512i)$Arg0VarName));
  }]>;
OclBuiltinImpl intel_movemask_v8i32 = OclBuiltinImpl<intel_movemask, [v8i32], 0, [{
    return convert_int(as_char(_mm256_movepi32_mask((__m256i)$Arg0VarName)));
  }]>;
OclBuiltinImpl intel_movemask_v16i32 = OclBuiltinImpl<intel_movemask, [v16i32], 0, [{
    return convert_int(as_short(_mm512_movepi32_mask((__m512i)$Arg0VarName)));
  }]>;
OclBuiltinImpl intel_movemask_v4i64 = OclBuiltinImpl<intel_movemask, [v4i64], 0, [{
    return convert_int(as_char(_mm256_movepi64_mask((__m256i)$Arg0VarName)));
  }]>;
OclBuiltinImpl intel_movemask_v8i64 = OclBuiltinImpl<intel_movemask, [v8i64], 0, [{
    return convert_int(as_char(_mm512_movepi64_mask((__m512i)$Arg0VarName)));
  }]>;

OclBuiltinImpl intel_movemask_v32_expand = OclBuiltinImpl<intel_movemask, [v32i32, v32i64], 0, Expand_movemask>;
OclBuiltinImpl intel_movemask_v64_expand = OclBuiltinImpl<intel_movemask_v64, [v64i16, v64i32, v64i64], 0, Expand_movemask64>;
