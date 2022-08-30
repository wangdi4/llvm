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

//////////////////////////////////////////////////////////////////////////////////////////////
// AVX2 math implementation
//////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////
//               v32 and v64 section
//////////////////////////////////////////////////////////////////////////////////////////////

defvar v32v64FTypes = ExpandTypesByVFAndFlatten<[v1f32, v1f64], [32, 64]>.Tout;
defvar v32v64F32Types = ExpandTypesByVFAndFlatten<[v1f32], [32, 64]>.Tout;

OclBuiltinImpl copysign_v32v64_impl = OclBuiltinImpl<copysign, v32v64FTypes, 0, copysign_code>;

OclBuiltinImpl fdim_v32v64_impl = OclBuiltinImpl<fdim, v32v64FTypes, 0, fdim_code_vec>;

OclBuiltinImpl fmax_v32v64_impl = OclBuiltinImpl<fmax, v32v64FTypes, 0, fmax_code>;

OclBuiltinImpl fmax_common_v32v64_impl = OclBuiltinImpl<fmax_common, v32v64FTypes, 0, fmax_common_code>;

OclBuiltinImpl fmax_s_v32v64_generic = OclBuiltinImpl<fmax_s, v32v64FTypes, 0, fmax_s_code>;

OclBuiltinImpl fmin_v32v64_impl = OclBuiltinImpl<fmin, v32v64FTypes, 0, fmin_code>;

OclBuiltinImpl fmin_common_v32v64_impl = OclBuiltinImpl<fmin_common, v32v64FTypes, 0, fmin_common_code>;

OclBuiltinImpl fmin_s_v32v64_generic = OclBuiltinImpl<fmin_s, v32v64FTypes, 0, fmin_s_code>;

OclBuiltinImpl fract_pas_f32 = OclBuiltinImpl<fract_pas, v32v64F32Types, 0, fract_code_f32>;
OclBuiltinImpl fract_las_f32 = OclBuiltinImpl<fract_las, v32v64F32Types, 0, fract_code_f32>;
OclBuiltinImpl fract_gas_f32 = OclBuiltinImpl<fract_gas, v32v64F32Types, 0, fract_code_f32>;
OclBuiltinImpl fract_gen_f32 = OclBuiltinImpl<fract_gen, v32v64F32Types, 0, fract_code_f32>;
OclBuiltinImpl fract_ret2ptr_pas = OclBuiltinImpl<fract_pas2, v32v64F32Types, 0, fract_ret2ptr_code>;

OclBuiltinImpl ilogb_f32_v32v64 = OclBuiltinImpl<ilogb, [v32f32, v64f32], 0, ilogb_code>;

OclBuiltinImpl ldexp_s_v32v64_generic = OclBuiltinImpl<ldexp_s, v32v64FTypes, 0, ldexp_s_code>;

OclBuiltinImpl mad_v32v64_impl = OclBuiltinImpl<mad, v32v64FTypes, 0, mad_code>;

OclBuiltinImpl maxmag_v32v64_impl = OclBuiltinImpl<maxmag, v32v64FTypes, 0, maxmag_code>;

OclBuiltinImpl minmag_v32v64_impl = OclBuiltinImpl<minmag, v32v64FTypes, 0, minmag_code>;

OclBuiltinImpl native_fdim_impl = OclBuiltinImpl<native_fdim, v32v64FTypes, 0, native_fdim_code>;

OclBuiltinImpl native_fmax_impl = OclBuiltinImpl<native_fmax, v32v64FTypes, 0, native_fmax_code>;

OclBuiltinImpl native_fmax_s_impl = OclBuiltinImpl<native_fmax_s, v32v64FTypes, 0, native_fmax_s_code>;

OclBuiltinImpl native_fmin_impl = OclBuiltinImpl<native_fmin, v32v64FTypes, 0, native_fmin_code>;

OclBuiltinImpl native_fmin_s_impl = OclBuiltinImpl<native_fmin_s, v32v64FTypes, 0, native_fmin_s_code>;

OclBuiltinImpl native_fmod_impl = OclBuiltinImpl<native_fmod, v32v64FTypes, 0, native_fmod_code>;

OclBuiltinImpl native_fract_pas_f32 = OclBuiltinImpl<native_fract_pas, v32v64F32Types, 0, native_fract_code>;
OclBuiltinImpl native_fract_las_f32 = OclBuiltinImpl<native_fract_las, v32v64F32Types, 0, native_fract_code>;
OclBuiltinImpl native_fract_gas_f32 = OclBuiltinImpl<native_fract_gas, v32v64F32Types, 0, native_fract_code>;
OclBuiltinImpl native_fract_gen_f32 = OclBuiltinImpl<native_fract_gen, v32v64F32Types, 0, native_fract_code>;
OclBuiltinImpl native_fract_ret2ptr_pas = OclBuiltinImpl<native_fract_pas2, v32v64F32Types, 0, native_fract_ret2ptr>;

OclBuiltinImpl native_fract_pas_f32 = OclBuiltinImpl<native_fract_pas, v32v64FTypes, 0, native_fract_code>;
OclBuiltinImpl native_fract_las_f32 = OclBuiltinImpl<native_fract_las, v32v64FTypes, 0, native_fract_code>;
OclBuiltinImpl native_fract_gas_f32 = OclBuiltinImpl<native_fract_gas, v32v64FTypes, 0, native_fract_code>;
OclBuiltinImpl native_fract_gen_f32 = OclBuiltinImpl<native_fract_gen, v32v64FTypes, 0, native_fract_code>;
OclBuiltinImpl native_fract_ret2ptr_pas = OclBuiltinImpl<native_fract_pas2, v32v64FTypes, 0, native_fract_ret2ptr>;

OclBuiltinImpl native_hypot_impl = OclBuiltinImpl<native_hypot, v32v64FTypes, 0, native_hypot_code>;

OclBuiltinImpl native_ilogb_impl = OclBuiltinImpl<native_ilogb, v32v64FTypes, 0, native_ilogb_code>;

//////////////////////////////////////////////////////////////////////////////////////////////
//                Float f32 section
//////////////////////////////////////////////////////////////////////////////////////////////

// fma float4
OclBuiltinImpl fma_v4f32_impl = OclBuiltinImpl<fma, [v4f32], 0, [{
    return _mm_fmadd_ps($Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;

// fma float8
OclBuiltinImpl fma_v8f32_impl = OclBuiltinImpl<fma, [v8f32], 0, [{
    return _mm256_fmadd_ps($Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;

// fma expand and promote
OclBuiltinImpl fma_v23f32_impl = OclBuiltinImpl<fma, [v1f32, v2f32, v3f32], 0, Promote3>;
OclBuiltinImpl fma_f1632_impl = OclBuiltinImpl<fma, [v16f32], 0, Expand3>;

//////////////////////////////////////////////////////////////////////////////////////////////
//                Double f64 section
//////////////////////////////////////////////////////////////////////////////////////////////

// fma double2
OclBuiltinImpl fma_v2f64_impl = OclBuiltinImpl<fma, [v2f64], 0, [{
    return _mm_fmadd_pd($Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;

// fma double4
OclBuiltinImpl fma_v4f64_impl = OclBuiltinImpl<fma, [v4f64], 0, [{
    return _mm256_fmadd_pd($Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;

// fma expand and promote
OclBuiltinImpl fma_v3f64_impl = OclBuiltinImpl<fma, [v1f64, v3f64], 0, Promote3>;
OclBuiltinImpl fma_f81664_impl = OclBuiltinImpl<fma, [v8f64, v16f64], 0, Expand3>;

//////////////////////////////////////////////////////////////////////////////////////////////

OclBuiltinImpl intel_movemask_v32i8 = OclBuiltinImpl<intel_movemask, [v32i8], 0, [{
    return as_int(_mm256_movemask_epi8((__m256i)$Arg0VarName));
  }]>;
