// Copyright (c) 2006 Intel Corporation
// Conversions (_sat) Built-In Functions
// AVX


/*****************************************************************\
 * convert_charn_sat:                                            *
\*****************************************************************/
// floats dont call SVML so no need to overload here
// change the double8 to call SVML instead of expanding to 2 double4 calls
// dp2signed_down_sat is defined in the generic section
OclBuiltinImpl avx_v8f64_conv_i8_sat = OclBuiltinImpl<convert_i8_sat, [v8f64] , 0, dp2signed_down_sat>;
OclBuiltinImpl avx_v8f64_conv_i8_sat_rte = OclBuiltinImpl<convert_i8_sat_rte, [v8f64], 0, dp2signed_down_sat_rte>;
OclBuiltinImpl avx_v8f64_conv_i8_sat_rtz = OclBuiltinImpl<convert_i8_sat_rtz, [v8f64], 0, dp2signed_down_sat_rtz>;
OclBuiltinImpl avx_v8f64_conv_i8_sat_rtp = OclBuiltinImpl<convert_i8_sat_rtp, [v8f64], 0, dp2signed_down_sat_rtp>;
OclBuiltinImpl avx_v8f64_conv_i8_sat_rtn = OclBuiltinImpl<convert_i8_sat_rtn, [v8f64], 0, dp2signed_down_sat_rtn>;


/*****************************************************************\
 * convert_ucharn_sat:                                           *
\*****************************************************************/
// floats dont call SVML so no need to overload here
// change the double8 to call SVML instead of expanding to 2 double4 calls
// dp2unsigned_down_sat is defined in the generic section
OclBuiltinImpl avx_v8f64_conv_u8_sat = OclBuiltinImpl<convert_u8_sat, [v8f64] , 0, dp2unsigned_down_sat>;
OclBuiltinImpl avx_v8f64_conv_u8_sat_rte = OclBuiltinImpl<convert_u8_sat_rte, [v8f64], 0, dp2unsigned_down_sat_rte>;
OclBuiltinImpl avx_v8f64_conv_u8_sat_rtz = OclBuiltinImpl<convert_u8_sat_rtz, [v8f64], 0, dp2unsigned_down_sat_rtz>;
OclBuiltinImpl avx_v8f64_conv_u8_sat_rtp = OclBuiltinImpl<convert_u8_sat_rtp, [v8f64], 0, dp2unsigned_down_sat_rtp>;
OclBuiltinImpl avx_v8f64_conv_u8_sat_rtn = OclBuiltinImpl<convert_u8_sat_rtn, [v8f64], 0, dp2unsigned_down_sat_rtn>;


// shorts call the int version so no need to overload them here


/*****************************************************************\
 * convert_intn_sat:                                             *
\*****************************************************************/
// floats dont call SVML so no need to overload here
// change the double8 to call SVML instead of expanding to 2 double4 calls
// dp2signed_down_sat is defined in the generic section
OclBuiltinImpl avx_v8f64_conv_i32_sat = OclBuiltinImpl<convert_i32_sat, [v8f64], 0, dp2signed_down_sat>;
OclBuiltinImpl avx_v8f64_conv_i32_sat_rte = OclBuiltinImpl<convert_i32_sat_rte, [v8f64], 0, dp2signed_down_sat_rte>;
OclBuiltinImpl avx_v8f64_conv_i32_sat_rtz = OclBuiltinImpl<convert_i32_sat_rtz, [v8f64], 0, dp2signed_down_sat_rtz>;
OclBuiltinImpl avx_v8f64_conv_i32_sat_rtp = OclBuiltinImpl<convert_i32_sat_rtp, [v8f64], 0, dp2signed_down_sat_rtp>;
OclBuiltinImpl avx_v8f64_conv_i32_sat_rtn = OclBuiltinImpl<convert_i32_sat_rtn, [v8f64], 0, dp2signed_down_sat_rtn>;


/*****************************************************************\
 * convert_uintn_sat:                                            *
\*****************************************************************/
// floats dont call SVML so no need to overload here
// change the double8 to call SVML instead of expanding to 2 double4 calls
// dp2unsigned_down_sat is defined in the generic section
OclBuiltinImpl avx_v8f64_conv_u32_sat = OclBuiltinImpl<convert_u32_sat, [v8f64], 0, dp2unsigned_down_sat>;
OclBuiltinImpl avx_v8f64_conv_u32_sat_rte = OclBuiltinImpl<convert_u32_sat_rte, [v8f64], 0, dp2unsigned_down_sat_rte>;
OclBuiltinImpl avx_v8f64_conv_u32_sat_rtz = OclBuiltinImpl<convert_u32_sat_rtz, [v8f64], 0, dp2unsigned_down_sat_rtz>;
OclBuiltinImpl avx_v8f64_conv_u32_sat_rtp = OclBuiltinImpl<convert_u32_sat_rtp, [v8f64], 0, dp2unsigned_down_sat_rtp>;
OclBuiltinImpl avx_v8f64_conv_u32_sat_rtn = OclBuiltinImpl<convert_u32_sat_rtn, [v8f64], 0, dp2unsigned_down_sat_rtn>;


/*****************************************************************\
 * convert_longn_sat:                                            *
\*****************************************************************/
// change the float8 to call SVML instead of expanding to 2 float4 calls
// fp2signed_up_sat is defined in the generic section
OclBuiltinImpl avx_v8f32_conv_i64_sat = OclBuiltinImpl<convert_i64_sat, [v8f32], 0, fp2signed_up_sat>;
OclBuiltinImpl avx_v8f32_conv_i64_sat_rte = OclBuiltinImpl<convert_i64_sat_rte, [v8f32], 0, fp2signed_up_sat_rte>;
OclBuiltinImpl avx_v8f32_conv_i64_sat_rtz = OclBuiltinImpl<convert_i64_sat_rtz, [v8f32], 0, fp2signed_up_sat_rtz>;
OclBuiltinImpl avx_v8f32_conv_i64_sat_rtp = OclBuiltinImpl<convert_i64_sat_rtp, [v8f32], 0, fp2signed_up_sat_rtp>;
OclBuiltinImpl avx_v8f32_conv_i64_sat_rtn = OclBuiltinImpl<convert_i64_sat_rtn, [v8f32], 0, fp2signed_up_sat_rtn>;
// change the double8 to call SVML instead of expanding to 2 double4 calls
// dp2signed_flat_sat is defined in the generic section
OclBuiltinImpl avx_v8f64_conv_i64_sat = OclBuiltinImpl<convert_i64_sat, [v8f64], 0, dp2signed_flat_sat>;
OclBuiltinImpl avx_v8f64_conv_i64_sat_rte = OclBuiltinImpl<convert_i64_sat_rte, [v8f64], 0, dp2signed_flat_sat_rte>;
OclBuiltinImpl avx_v8f64_conv_i64_sat_rtz = OclBuiltinImpl<convert_i64_sat_rtz, [v8f64], 0, dp2signed_flat_sat_rtz>;
OclBuiltinImpl avx_v8f64_conv_i64_sat_rtp = OclBuiltinImpl<convert_i64_sat_rtp, [v8f64], 0, dp2signed_flat_sat_rtp>;
OclBuiltinImpl avx_v8f64_conv_i64_sat_rtn = OclBuiltinImpl<convert_i64_sat_rtn, [v8f64], 0, dp2signed_flat_sat_rtn>;


/*****************************************************************\
 * convert_ulongn_sat:                                           *
\*****************************************************************/
// change the float8 to call SVML instead of expanding to 2 float4 calls
// fp2unsigned_up_sat is defined in the generic section
OclBuiltinImpl avx_v8f32_conv_u64_sat = OclBuiltinImpl<convert_u64_sat, [v8f32], 0, fp2unsigned_up_sat>;
OclBuiltinImpl avx_v8f32_conv_u64_sat_rte = OclBuiltinImpl<convert_u64_sat_rte, [v8f32], 0, fp2unsigned_up_sat_rte>;
OclBuiltinImpl avx_v8f32_conv_u64_sat_rtz = OclBuiltinImpl<convert_u64_sat_rtz, [v8f32], 0, fp2unsigned_up_sat_rtz>;
OclBuiltinImpl avx_v8f32_conv_u64_sat_rtp = OclBuiltinImpl<convert_u64_sat_rtp, [v8f32], 0, fp2unsigned_up_sat_rtp>;
OclBuiltinImpl avx_v8f32_conv_u64_sat_rtn = OclBuiltinImpl<convert_u64_sat_rtn, [v8f32], 0, fp2unsigned_up_sat_rtn>;
// change the double8 to call SVML instead of expanding to 2 double4 calls
// dp2unsigned_flat_sat is defined in the generic section
OclBuiltinImpl avx_v8f64_conv_u64_sat = OclBuiltinImpl<convert_u64_sat, [v8f64], 0, dp2unsigned_flat_sat>;
OclBuiltinImpl avx_v8f64_conv_u64_sat_rte = OclBuiltinImpl<convert_u64_sat_rte, [v8f64], 0, dp2unsigned_flat_sat_rte>;
OclBuiltinImpl avx_v8f64_conv_u64_sat_rtz = OclBuiltinImpl<convert_u64_sat_rtz, [v8f64], 0, dp2unsigned_flat_sat_rtz>;
OclBuiltinImpl avx_v8f64_conv_u64_sat_rtp = OclBuiltinImpl<convert_u64_sat_rtp, [v8f64], 0, dp2unsigned_flat_sat_rtp>;
OclBuiltinImpl avx_v8f64_conv_u64_sat_rtn = OclBuiltinImpl<convert_u64_sat_rtn, [v8f64], 0, dp2unsigned_flat_sat_rtn>;









































