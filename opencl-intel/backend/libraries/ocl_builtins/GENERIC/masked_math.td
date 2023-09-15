// Copyright (C) 2020 Intel Corporation
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

// Masked implementation codes
// Naming convention:
// 1. Always prefixed with 'masked_impl_code_'
// 2. Following '_' seperated params type identifiers
//    a. the first identifier indicates the return type
//    b. the following identifiers are as the same order with the input parameters
// 3. Identifier lookup table
//    - G: gentype, G2: 2 consecutive gentypes, ...
//    - V: void
//    - M: mask type (the last parameter, usually same as the return type)
//    - P: pointer type
//    - IG: igentype
//    - UG: ugentype
//    - FG: float gentype
//    - DG: double gentype
//    - I32: i32gentype

list<OclType> math_all_types = [
  v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
  v1f64, v2f64, v3f64, v4f64, v8f64, v16f64
];

list<OclType> math_float_types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32];
list<OclType> math_double_types = [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64];

list<OclType> all_types = [
  v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
  v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
  v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
  v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
  v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
  v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
  v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
  v1u64, v2u64, v3u64, v4u64, v8u64, v16u64,
  v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
  v1f64, v2f64, v3f64, v4f64, v8f64, v16f64
];

// generated cl code example: acosh
// float8 __attribute__((const)) __attribute__((overloadable)) acosh(float8 x, float8 mask)
// {
//   float8 mx = bitselect((float8)(4.2), x, mask);
//   return acosh(mx);
// }
OclBuiltinImpl acos_mask_svml = OclBuiltinImpl<acos_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl acosh_mask_svml = OclBuiltinImpl<acosh_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl acospi_mask_svml = OclBuiltinImpl<acospi_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl asin_mask_svml = OclBuiltinImpl<asin_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl asinh_mask_svml = OclBuiltinImpl<asinh_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl asinpi_mask_svml = OclBuiltinImpl<asinpi_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl atan_mask_svml = OclBuiltinImpl<atan_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl atanh_mask_svml = OclBuiltinImpl<atanh_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0.42"]]>;
OclBuiltinImpl atanpi_mask_svml = OclBuiltinImpl<atanpi_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl cbrt_mask_svml = OclBuiltinImpl<cbrt_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl ceil_mask_svml = OclBuiltinImpl<ceil_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl cos_mask_svml = OclBuiltinImpl<cos_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl cosh_mask_svml = OclBuiltinImpl<cosh_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl cospi_mask_svml = OclBuiltinImpl<cospi_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl erfc_mask_svml = OclBuiltinImpl<erfc_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl erf_mask_svml = OclBuiltinImpl<erf_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0.88"]]>;
OclBuiltinImpl exp_mask_svml = OclBuiltinImpl<exp_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl exp2_mask_svml = OclBuiltinImpl<exp2_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl exp10_mask_svml = OclBuiltinImpl<exp10_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl expm1_mask_svml = OclBuiltinImpl<expm1_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl floor_mask_svml = OclBuiltinImpl<floor_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl lgamma_mask_svml = OclBuiltinImpl<lgamma_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl log_mask_svml = OclBuiltinImpl<log_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl log2_mask_svml = OclBuiltinImpl<log2_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl log10_mask_svml = OclBuiltinImpl<log10_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl log1p_mask_svml = OclBuiltinImpl<log1p_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl logb_mask_svml = OclBuiltinImpl<logb_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl rint_mask_svml = OclBuiltinImpl<rint_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl round_mask_svml = OclBuiltinImpl<round_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl rsqrt_mask_svml = OclBuiltinImpl<rsqrt_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "1"]]>;
OclBuiltinImpl sin_mask_svml = OclBuiltinImpl<sin_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl sinh_mask_svml = OclBuiltinImpl<sinh_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl sinpi_mask_svml = OclBuiltinImpl<sinpi_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl sqrt_mask_svml = OclBuiltinImpl<sqrt_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl tan_mask_svml = OclBuiltinImpl<tan_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl tanh_mask_svml = OclBuiltinImpl<tanh_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl tanpi_mask_svml = OclBuiltinImpl<tanpi_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl tgamma_mask_svml = OclBuiltinImpl<tgamma_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "1"]]>;
OclBuiltinImpl trunc_mask_svml = OclBuiltinImpl<trunc_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;

// half
OclBuiltinImpl half_cos_mask_svml = OclBuiltinImpl<half_cos_mask, math_float_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl half_exp_mask_svml = OclBuiltinImpl<half_exp_mask, math_float_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl half_exp2_mask_svml = OclBuiltinImpl<half_exp2_mask, math_float_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl half_exp10_mask_svml = OclBuiltinImpl<half_exp10_mask, math_float_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl half_log_mask_svml = OclBuiltinImpl<half_log_mask, math_float_types, 0, masked_impl_code_G_G_M, [["$DEF0", "1"]]>;
OclBuiltinImpl half_log2_mask_svml = OclBuiltinImpl<half_log2_mask, math_float_types, 0, masked_impl_code_G_G_M, [["$DEF0", "1"]]>;
OclBuiltinImpl half_log10_mask_svml = OclBuiltinImpl<half_log10_mask, math_float_types, 0, masked_impl_code_G_G_M, [["$DEF0", "1"]]>;
OclBuiltinImpl half_recip_mask_svml = OclBuiltinImpl<half_recip_mask, math_float_types, 0, masked_impl_code_G_G_M, [["$DEF0", "1"]]>;
OclBuiltinImpl half_rsqrt_mask_svml = OclBuiltinImpl<half_rsqrt_mask, math_float_types, 0, masked_impl_code_G_G_M, [["$DEF0", "1"]]>;
OclBuiltinImpl half_sin_mask_svml = OclBuiltinImpl<half_sin_mask, math_float_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl half_sqrt_mask_svml = OclBuiltinImpl<half_sqrt_mask, math_float_types, 0, masked_impl_code_G_G_M, [["$DEF0", "1"]]>;
OclBuiltinImpl half_tan_mask_svml = OclBuiltinImpl<half_tan_mask, math_float_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;

// native
OclBuiltinImpl native_cos_mask_svml = OclBuiltinImpl<native_cos_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_exp_mask_svml = OclBuiltinImpl<native_exp_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_exp2_mask_svml = OclBuiltinImpl<native_exp2_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_exp10_mask_svml = OclBuiltinImpl<native_exp10_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_log_mask_svml = OclBuiltinImpl<native_log_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl native_log2_mask_svml = OclBuiltinImpl<native_log2_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl native_log10_mask_svml = OclBuiltinImpl<native_log10_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl native_recip_mask_svml = OclBuiltinImpl<native_recip_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "1"]]>;
OclBuiltinImpl native_rsqrt_mask_svml = OclBuiltinImpl<native_rsqrt_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "1"]]>;
OclBuiltinImpl native_sin_mask_svml = OclBuiltinImpl<native_sin_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_sqrt_mask_svml = OclBuiltinImpl<native_sqrt_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_tan_mask_svml = OclBuiltinImpl<native_tan_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;

// extended native
OclBuiltinImpl native_acos_mask_svml = OclBuiltinImpl<native_acos_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_acosh_mask_svml = OclBuiltinImpl<native_acosh_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl native_acospi_mask_svml = OclBuiltinImpl<native_acospi_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_asin_mask_svml = OclBuiltinImpl<native_asin_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_asinh_mask_svml = OclBuiltinImpl<native_asinh_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl native_asinpi_mask_svml = OclBuiltinImpl<native_asinpi_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_atan_mask_svml = OclBuiltinImpl<native_atan_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_atanh_mask_svml = OclBuiltinImpl<native_atanh_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0.42"]]>;
OclBuiltinImpl native_atanpi_mask_svml = OclBuiltinImpl<native_atanpi_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_cbrt_mask_svml = OclBuiltinImpl<native_cbrt_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_cosh_mask_svml = OclBuiltinImpl<native_cosh_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_cospi_mask_svml = OclBuiltinImpl<native_cospi_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_erfc_mask_svml = OclBuiltinImpl<native_erfc_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_erf_mask_svml = OclBuiltinImpl<native_erf_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0.88"]]>;
OclBuiltinImpl native_expm1_mask_svml = OclBuiltinImpl<native_expm1_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_log1p_mask_svml = OclBuiltinImpl<native_log1p_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl native_logb_mask_svml = OclBuiltinImpl<native_logb_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl native_rcbrt_mask_svml = OclBuiltinImpl<native_rcbrt_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "1"]]>;
OclBuiltinImpl native_sinh_mask_svml = OclBuiltinImpl<native_sinh_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_sinpi_mask_svml = OclBuiltinImpl<native_sinpi_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_tanh_mask_svml = OclBuiltinImpl<native_tanh_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_tanpi_mask_svml = OclBuiltinImpl<native_tanpi_mask, math_all_types, 0, masked_impl_code_G_G_M, [["$DEF0", "0"]]>;

// generated cl code example: pow
// float16 __attribute__((const)) __attribute__((overloadable)) pow(float16 x, float16 y, float16 mask)
// {
//   float16 mx = bitselect((float16)(5), x, mask);
//   float16 my = bitselect((float16)(5), y, mask);
//   return pow(mx, my);
// }
OclBuiltinImpl atan2_mask_svml = OclBuiltinImpl<atan2_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "1"]]>;
OclBuiltinImpl atan2pi_mask_svml = OclBuiltinImpl<atan2pi_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "1"]]>;
OclBuiltinImpl copysign_mask_svml = OclBuiltinImpl<copysign_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "1"]]>;
OclBuiltinImpl fdim_mask_svml = OclBuiltinImpl<fdim_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
OclBuiltinImpl fmax_mask_svml = OclBuiltinImpl<fmax_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
OclBuiltinImpl fmax_common_mask_svml = OclBuiltinImpl<fmax_common_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
OclBuiltinImpl fmin_mask_svml = OclBuiltinImpl<fmin_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
OclBuiltinImpl fmin_common_mask_svml = OclBuiltinImpl<fmin_common_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
OclBuiltinImpl fmod_mask_svml = OclBuiltinImpl<fmod_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "1"]]>;
OclBuiltinImpl hypot_mask_svml = OclBuiltinImpl<hypot_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
OclBuiltinImpl maxmag_mask_svml = OclBuiltinImpl<maxmag_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
OclBuiltinImpl minmag_mask_svml = OclBuiltinImpl<minmag_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
OclBuiltinImpl nextafter_mask_svml = OclBuiltinImpl<nextafter_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "1"]]>;
OclBuiltinImpl pow_mask_svml = OclBuiltinImpl<pow_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "5"], ["$DEF1", "5"]]>;
OclBuiltinImpl powr_mask_svml = OclBuiltinImpl<powr_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "5"], ["$DEF1", "5"]]>;
OclBuiltinImpl remainder_mask_svml = OclBuiltinImpl<remainder_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "42"]]>;

// half
OclBuiltinImpl half_divide_mask_svml = OclBuiltinImpl<half_divide_mask, math_float_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "1"], ["$DEF1", "1"]]>;
OclBuiltinImpl half_powr_mask_svml = OclBuiltinImpl<half_powr_mask, math_float_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "1"], ["$DEF1", "1"]]>;

// native
OclBuiltinImpl native_divide_mask_svml = OclBuiltinImpl<native_divide_mask, math_float_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "1"], ["$DEF1", "1"]]>;
OclBuiltinImpl native_powr_mask_svml = OclBuiltinImpl<native_powr_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "5"], ["$DEF1", "5"]]>;

// extended native
OclBuiltinImpl native_atan2_mask_svml = OclBuiltinImpl<native_atan2_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "1"]]>;
OclBuiltinImpl native_atan2pi_mask_svml = OclBuiltinImpl<native_atan2pi_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "1"]]>;
OclBuiltinImpl native_fdim_mask_svml = OclBuiltinImpl<native_fdim_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
OclBuiltinImpl native_fmax_mask_svml = OclBuiltinImpl<native_fmax_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
OclBuiltinImpl native_fmin_mask_svml = OclBuiltinImpl<native_fmin_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
OclBuiltinImpl native_fmod_mask_svml = OclBuiltinImpl<native_fmod_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "1"]]>;
OclBuiltinImpl native_hypot_mask_svml = OclBuiltinImpl<native_hypot_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
OclBuiltinImpl native_pow_mask_svml = OclBuiltinImpl<native_pow_mask, math_all_types, 0, masked_impl_code_G_G2_M, [["$DEF0", "5"], ["$DEF1", "5"]]>;

// generated cl code example: fma
// float2 __attribute__((const)) __attribute__((overloadable)) fma(float2 a, float2 b, float2 c, float2 mask)
// {
//   float2 ma = bitselect((float2)(0), a, mask);
//   float2 mb = bitselect((float2)(0), b, mask);
//   float2 mc = bitselect((float2)(0), c, mask);
//   return fma(ma, mb, mc);
// }
OclBuiltinImpl fma_mask_svml = OclBuiltinImpl<fma_mask, math_all_types, 0, masked_impl_code_G_G3_M, [["$DEF0", "0"], ["$DEF1", "0"], ["$DEF2", "0"]]>;
OclBuiltinImpl mad_mask_svml = OclBuiltinImpl<mad_mask, math_all_types, 0, masked_impl_code_G_G3_M, [["$DEF0", "0"], ["$DEF1", "0"], ["$DEF2", "0"]]>;

// generated cl code example: lgamma_r_gas
// double16 __attribute__((overloadable)) lgamma_r(double16 x, __global int16 *signp, double16 mask)
// {
//   double16 mx = bitselect((double16)(4.2), x, mask);
//   return lgamma_r(mx, signp);
// }
OclBuiltinImpl fract_pas_mask_svml = OclBuiltinImpl<fract_pas_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl fract_las_mask_svml = OclBuiltinImpl<fract_las_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl fract_gas_mask_svml = OclBuiltinImpl<fract_gas_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl fract_gen_mask_svml = OclBuiltinImpl<fract_gen_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl frexp_pas_mask_svml = OclBuiltinImpl<frexp_pas_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl frexp_las_mask_svml = OclBuiltinImpl<frexp_las_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl frexp_gas_mask_svml = OclBuiltinImpl<frexp_gas_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl frexp_gen_mask_svml = OclBuiltinImpl<frexp_gen_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl lgamma_r_pas_mask_svml = OclBuiltinImpl<lgamma_r_pas_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl lgamma_r_las_mask_svml = OclBuiltinImpl<lgamma_r_las_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl lgamma_r_gas_mask_svml = OclBuiltinImpl<lgamma_r_gas_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl lgamma_r_gen_mask_svml = OclBuiltinImpl<lgamma_r_gen_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "4.2"]]>;
OclBuiltinImpl modf_pas_mask_svml = OclBuiltinImpl<modf_pas_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl modf_las_mask_svml = OclBuiltinImpl<modf_las_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl modf_gas_mask_svml = OclBuiltinImpl<modf_gas_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl modf_gen_mask_svml = OclBuiltinImpl<modf_gen_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl sincos_pas_mask_svml = OclBuiltinImpl<sincos_pas_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl sincos_las_mask_svml = OclBuiltinImpl<sincos_las_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl sincos_gas_mask_svml = OclBuiltinImpl<sincos_gas_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl sincos_gen_mask_svml = OclBuiltinImpl<sincos_gen_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;

// extended native
OclBuiltinImpl native_fract_pas_mask_svml = OclBuiltinImpl<native_fract_pas_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_fract_las_mask_svml = OclBuiltinImpl<native_fract_las_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_fract_gas_mask_svml = OclBuiltinImpl<native_fract_gas_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_fract_gen_mask_svml = OclBuiltinImpl<native_fract_gen_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_sincos_pas_mask_svml = OclBuiltinImpl<native_sincos_pas_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_sincos_las_mask_svml = OclBuiltinImpl<native_sincos_las_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_sincos_gas_mask_svml = OclBuiltinImpl<native_sincos_gas_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_sincos_gen_mask_svml = OclBuiltinImpl<native_sincos_gen_mask, math_all_types, 0, masked_impl_code_G_G_P_M, [["$DEF0", "0"]]>;

// generated cl code example: fract_pas2
// void __attribute__((overloadable)) fract_ret2ptr(float2 x, __private float2 *ret0, __private float2 *ret1, float2 mask)
// {
//   float2 mx = bitselect((float2)(0), x, mask);
//   return fract_ret2ptr(mx, ret0, ret1);
// }
OclBuiltinImpl fract_pas2_mask_svml = OclBuiltinImpl<fract_pas2_mask, math_all_types, 0, masked_impl_code_V_G_P_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl modf_pas2_mask_svml = OclBuiltinImpl<modf_pas2_mask, math_all_types, 0, masked_impl_code_V_G_P_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl sincos_pas2_mask_svml = OclBuiltinImpl<sincos_pas2_mask, math_all_types, 0, masked_impl_code_V_G_P_P_M, [["$DEF0", "0"]]>;

// extended native
OclBuiltinImpl native_fract_pas2_mask_svml = OclBuiltinImpl<native_fract_pas2_mask, math_all_types, 0, masked_impl_code_V_G_P_P_M, [["$DEF0", "0"]]>;
OclBuiltinImpl native_sincos_pas2_mask_svml = OclBuiltinImpl<native_sincos_pas2_mask, math_all_types, 0, masked_impl_code_V_G_P_P_M, [["$DEF0", "0"]]>;

// generated cl code example: remquo_las
// float3 __attribute__((overloadable)) remquo(float3 x, float3 y, __local int3 *quo, float3 mask)
// {
//   float3 mx = bitselect((float3)(0), x, mask);
//   float3 my = bitselect((float3)(42), y, mask);
//   return remquo(mx, my, quo);
// }
OclBuiltinImpl remquo_pas_mask_svml = OclBuiltinImpl<remquo_pas_mask, math_all_types, 0, masked_impl_code_G_G2_P_M, [["$DEF0", "0"], ["$DEF1", "42"]]>;
OclBuiltinImpl remquo_las_mask_svml = OclBuiltinImpl<remquo_las_mask, math_all_types, 0, masked_impl_code_G_G2_P_M, [["$DEF0", "0"], ["$DEF1", "42"]]>;
OclBuiltinImpl remquo_gas_mask_svml = OclBuiltinImpl<remquo_gas_mask, math_all_types, 0, masked_impl_code_G_G2_P_M, [["$DEF0", "0"], ["$DEF1", "42"]]>;
OclBuiltinImpl remquo_gen_mask_svml = OclBuiltinImpl<remquo_gen_mask, math_all_types, 0, masked_impl_code_G_G2_P_M, [["$DEF0", "0"], ["$DEF1", "42"]]>;

// generated cl code example: ilogb float
// int4 __attribute__((const)) __attribute__((overloadable)) ilogb(float4 x, int4 mask)
// {
//   float4 mx = select((float4)(1), x, mask);
//   return ilogb(mx);
// }
OclBuiltinImpl ilogb_mask_svml_float = OclBuiltinImpl<ilogb_mask, math_float_types, 0, masked_impl_code_I32_FG_M, [["$DEF0", "1"]]>;

// extended native
OclBuiltinImpl native_ilogb_mask_svml_float = OclBuiltinImpl<native_ilogb_mask, math_float_types, 0, masked_impl_code_I32_FG_M, [["$DEF0", "1"]]>;

// generated cl code example: ilogb double
// int4 __attribute__((const)) __attribute__((overloadable)) ilogb(double4 x, int4 mask)
// {
//   double4 mx = select((double4)(1), x, convert_long4(mask));
//   return ilogb(mx);
// }
OclBuiltinImpl ilogb_mask_svml_double = OclBuiltinImpl<ilogb_mask, math_double_types, 0, masked_impl_code_I32_DG_M, [["$DEF0", "1"]]>;

// extended native
OclBuiltinImpl native_ilogb_mask_svml_double = OclBuiltinImpl<native_ilogb_mask, math_double_types, 0, masked_impl_code_I32_DG_M, [["$DEF0", "1"]]>;

// generated cl code example: rootn float
// float8 __attribute__((const)) __attribute__((overloadable)) rootn(float8 x, int8 y, float8 mask)
// {
//   float8 mx = bitselect((float8)(1), x, mask);
//   int8 my = select((int8)(1), y, as_int8(mask));
//   return rootn(mx, my);
// }
OclBuiltinImpl ldexp_mask_svml_float = OclBuiltinImpl<ldexp_mask, math_float_types, 0, masked_impl_code_FG_FG_I32_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
OclBuiltinImpl pown_mask_svml_float = OclBuiltinImpl<pown_mask, math_float_types, 0, masked_impl_code_FG_FG_I32_M, [["$DEF0", "5"], ["$DEF1", "5"]]>;
OclBuiltinImpl rootn_mask_svml_float = OclBuiltinImpl<rootn_mask, math_float_types, 0, masked_impl_code_FG_FG_I32_M, [["$DEF0", "1"], ["$DEF1", "1"]]>;

// extended native
OclBuiltinImpl native_pown_mask_svml_float = OclBuiltinImpl<native_pown_mask, math_float_types, 0, masked_impl_code_FG_FG_I32_M, [["$DEF0", "5"], ["$DEF1", "5"]]>;
OclBuiltinImpl native_rootn_mask_svml_float = OclBuiltinImpl<native_rootn_mask, math_float_types, 0, masked_impl_code_FG_FG_I32_M, [["$DEF0", "1"], ["$DEF1", "1"]]>;

// generated cl code example: rootn double
// double8 __attribute__((const)) __attribute__((overloadable)) rootn(double8 x, int8 y, double8 mask)
// {
//   double8 mx = bitselect((double8)(1), x, mask);
//   int8 my = select((int8)(1), y, convert_int8(as_long8(mask)));
//   return rootn(mx, my);
// }
OclBuiltinImpl ldexp_mask_svml_double = OclBuiltinImpl<ldexp_mask, math_double_types, 0, masked_impl_code_DG_DG_I32_M, [["$DEF0", "0"], ["$DEF1", "0"]]>;
OclBuiltinImpl pown_mask_svml_double = OclBuiltinImpl<pown_mask, math_double_types, 0, masked_impl_code_DG_DG_I32_M, [["$DEF0", "5"], ["$DEF1", "5"]]>;
OclBuiltinImpl rootn_mask_svml_double = OclBuiltinImpl<rootn_mask, math_double_types, 0, masked_impl_code_DG_DG_I32_M, [["$DEF0", "1"], ["$DEF1", "1"]]>;

// extended native
OclBuiltinImpl native_pown_mask_svml_double = OclBuiltinImpl<native_pown_mask, math_double_types, 0, masked_impl_code_DG_DG_I32_M, [["$DEF0", "5"], ["$DEF1", "5"]]>;
OclBuiltinImpl native_rootn_mask_svml_double = OclBuiltinImpl<native_rootn_mask, math_double_types, 0, masked_impl_code_DG_DG_I32_M, [["$DEF0", "1"], ["$DEF1", "1"]]>;
