// vim:ts=2:sw=2:et:

// NOTE:
// this is generic implementations for math functions which has some
// differences between particular implementations, for example:

// built-in acos has no specific code for data types
// v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64, v16f64
// and code for built-ins acosf, acosf2, acosf3, acosf4, acosf8,
// acos, acos2, acos3, acos4, acos16 is placed in shared RTL library and in
// another file: math_shared.td

// but the code for built-ins acos8 and acosf16 has some differences
// between sse42 and avx realisations, so built-ins for data types
// v8f64 and v16f32 exist here

// math_shared and math_not_shared MUST not overlap. i.e. contain
// the same functions

list<OclType> math_not_shared_types = [v16f32, v8f64];
list<OclType> math_not_shared_types_v1_to_v16 = ExpandTypesByVFAndFlatten<[v1f32, v1f64], [1, 2, 3, 4, 8, 16]>.Tout;

// ceil
// ceil not used in shared lib at all
OclBuiltinImpl ceil_svml = OclBuiltinImpl<ceil, math_not_shared_types_v1_to_v16, 0, [{
    return __ocl_svml_$Target_ceil$SVMLSuffix($Arg0VarName);
  }]>;

// floor
OclBuiltinImpl floor_svml_not_shared = OclBuiltinImpl<floor, [v4f32, v8f32, v2f64, v4f64, v8f64, v16f32], 0, [{
    return __ocl_svml_$Target_floor$SVMLSuffix($Arg0VarName);
  }]>;

// fma
OclBuiltinImpl fma_svml_not_shared = OclBuiltinImpl<fma, math_not_shared_types_v1_to_v16, 0, [{
    return __ocl_svml_$Target_fma$SVMLSuffix($Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;

OclBuiltinImpl rint_expand = OclBuiltinImpl<rint, [v4f64, v8f32, v8f64, v16f32, v16f64], 0, Expand1>;

// OpenCL 2.0 -cl-fast-relaxed-math functions
// float16 is excluded from shared lib since ssse3 and sse42
// svml libs don't include v16f32 FRM functions
// cos_rm
OclBuiltinImpl cos_svml_rm_not_shared = OclBuiltinImpl<cos_rm, [v16f32], 0, [{
    return __ocl_svml_$Target_cos$SVMLSuffix_rm($Arg0VarName);
  }]>;

// exp_rm
OclBuiltinImpl exp_svml_rm_not_shared = OclBuiltinImpl<exp_rm, [v16f32], 0, [{
    return __ocl_svml_$Target_exp$SVMLSuffix_rm($Arg0VarName);
  }]>;

// exp2_rm
OclBuiltinImpl exp2_svml_rm_not_shared = OclBuiltinImpl<exp2_rm, [v16f32], 0, [{
    return __ocl_svml_$Target_exp2$SVMLSuffix_rm($Arg0VarName);
  }]>;

// exp10_rm
OclBuiltinImpl exp10_svml_rm_not_shared = OclBuiltinImpl<exp10_rm, [v16f32], 0, [{
    return __ocl_svml_$Target_exp10$SVMLSuffix_rm($Arg0VarName);
  }]>;

// log_rm
OclBuiltinImpl log_svml_rm_not_shared = OclBuiltinImpl<log_rm, [v16f32], 0, [{
    return __ocl_svml_$Target_log$SVMLSuffix_rm($Arg0VarName);
  }]>;

// log2_rm
OclBuiltinImpl log2_svml_rm_not_shared = OclBuiltinImpl<log2_rm, [v16f32], 0, [{
    return __ocl_svml_$Target_log2$SVMLSuffix_rm($Arg0VarName);
  }]>;

// pow_rm
OclBuiltinImpl pow_svml_rm_not_shared = OclBuiltinImpl<pow_rm, [v16f32], 0, [{
    return __ocl_svml_$Target_pow$SVMLSuffix_rm($Arg0VarName, $Arg1VarName);
  }]>;

// sin_rm
OclBuiltinImpl sin_svml_rm_not_shared = OclBuiltinImpl<sin_rm, [v16f32], 0, [{
    return __ocl_svml_$Target_sin$SVMLSuffix_rm($Arg0VarName);
  }]>;

// tan_rm
OclBuiltinImpl tan_svml_rm_not_shared = OclBuiltinImpl<tan_rm, [v16f32], 0, [{
    return __ocl_svml_$Target_tan$SVMLSuffix_rm($Arg0VarName);
  }]>;

// sincos_rm
OclBuiltinImpl sincos_pas_svml_rm_not_shared = OclBuiltinImpl<sincos_pas_rm, [v16f32], 0, svml_rm_code_sincos>;
OclBuiltinImpl sincos_las_svml_rm_not_shared = OclBuiltinImpl<sincos_las_rm, [v16f32], 0, svml_rm_code_sincos>;
OclBuiltinImpl sincos_gas_svml_rm_not_shared = OclBuiltinImpl<sincos_gas_rm, [v16f32], 0, svml_rm_code_sincos>;
OclBuiltinImpl sincos_gen_svml_rm_not_shared = OclBuiltinImpl<sincos_gen_rm, [v16f32], 0, svml_rm_code_sincos>;

// divide_rm
OclBuiltinImpl divide_svml_rm_not_shared = OclBuiltinImpl<divide_rm, [v16f32], 0, [{
    return __ocl_svml_$Target_div$SVMLSuffix_rm($Arg0VarName, $Arg1VarName);
  }]>;

// intel_movemask scalar
OclBuiltinImpl intel_scalar_movemask_impl = OclBuiltinImpl<intel_movemask, [v1i8,  v1i16, v1i32, v1i64], 0, [{
    return $Arg0VarName != 0;
  }]>;

// intel_movemask vector
OclBuiltinImpl intel_vector_movemask_impl = OclBuiltinImpl<intel_movemask, [v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                                     v2i16, v3i16, v4i16, v8i16, v16i16,
                                                                     v2i32, v3i32, v4i32, v8i32, v16i32,
                                                                     v2i64, v3i64, v4i64, v8i64, v16i64], 0, Expand_movemask>;

OclBuiltinImpl fract_pas_v1f32 = OclBuiltinImpl<fract_pas, [v1f32], 0, Promote_fract>;
OclBuiltinImpl fract_pas_f32 = OclBuiltinImpl<fract_pas, [v2f32, v3f32, v4f32, v8f32, v16f32], 0, fract_code_f32>;

OclBuiltinImpl fract_las_v1f32 = OclBuiltinImpl<fract_las, [v1f32], 0, Promote_fract>;
OclBuiltinImpl fract_las_f32 = OclBuiltinImpl<fract_las, [v2f32, v3f32, v4f32, v8f32, v16f32], 0, fract_code_f32>;

OclBuiltinImpl fract_gas_v1f32 = OclBuiltinImpl<fract_gas, [v1f32], 0, Promote_fract>;
OclBuiltinImpl fract_gas_f32 = OclBuiltinImpl<fract_gas, [v2f32, v3f32, v4f32, v8f32, v16f32], 0, fract_code_f32>;

OclBuiltinImpl fract_gen_v1f32 = OclBuiltinImpl<fract_gen, [v1f32], 0, Promote_fract>;
OclBuiltinImpl fract_gen_f32 = OclBuiltinImpl<fract_gen, [v2f32, v3f32, v4f32, v8f32, v16f32], 0, fract_code_f32>;

OclBuiltinImpl fract_ret2ptr_pas = OclBuiltinImpl<fract_pas2, [v4f32, v8f32, v16f32, v4f64, v8f64, v16f64], 0, fract_ret2ptr_code>;

// frexp
OclBuiltinImpl frexp_pas_svml = OclBuiltinImpl<frexp_pas, [v3f32], 0, frexp_svml>;
OclBuiltinImpl frexp_las_svml = OclBuiltinImpl<frexp_las, [v3f32], 0, frexp_svml>;
OclBuiltinImpl frexp_gas_svml = OclBuiltinImpl<frexp_gas, [v3f32], 0, frexp_svml>;
OclBuiltinImpl frexp_gen_svml = OclBuiltinImpl<frexp_gen, [v3f32], 0, frexp_svml>;

// ilogb float. call float2 version
// if use for scalar version the same code as vector it does not work because
// select() BI behaviour on scalar is different than on vector. see fdim example
OclBuiltinImpl ilogb_impl_v1f32_promote = OclBuiltinImpl<ilogb, [v1f32], 0, Promote1>;

// ilogb vector float
OclBuiltinImpl ilogb_impl_f32_not_shared = OclBuiltinImpl<ilogb, [v2f32], 0, [{
    const $Arg0ugentype signMask = $Arg0BaseType_const_signMask;
    const $Arg0ugentype expMask  = $Arg0BaseType_const_expMask;
    const $Arg0ugentype StorageI64 = 64;
    const $Arg0ugentype StorageI127 = $Arg0BaseType_const_expOffset;
    const $Arg0ugentype p64Storage = 0x5f800000;
    typedef union{ $Arg0Type f; $Arg0ugentype u; $Arg0igentype i; } f$Suffix_t;
    f$Suffix_t isDenorm, isZero, isNaNorINF, isNotDenorm;
    f$Suffix_t xx, res1, res3, factor;

    xx.f = $Arg0VarName;
    xx.u &= signMask;
    isZero.i = as_$Arg0igentype(xx.u == 0);
    res1.u = xx.u & expMask;
    isNaNorINF.i = as_$Arg0igentype(res1.u == expMask);
    isDenorm.i = as_$Arg0igentype(res1.u == 0);
    factor.u = (isDenorm.u & StorageI64) + StorageI127;
    xx.u &= isDenorm.u;
    isDenorm.u &= p64Storage;
    isDenorm.f *= xx.f;
    isDenorm.u &= expMask;
    res1.u >>= 23;
    res3.u = isDenorm.u >> 23;
    isNotDenorm.i = as_$Arg0igentype(res3.u == 0);
    res1.u -= factor.u;
    res3.u -= factor.u;
    res1.i = select(res3.i, res1.i, isNotDenorm.i);
    res1.i = select(res1.i, INT_MAX, isNaNorINF.i);
    res1.i = select(res1.i, INT_MIN, isZero.i);
    return as_$Arg0igentype(res1.u);
  }]>;

// lgamma
OclBuiltinImpl lgamma_r_pas_svml = OclBuiltinImpl<lgamma_r_pas, [v3f32], 0, lgamma_r_svml>;
OclBuiltinImpl lgamma_r_las_svml = OclBuiltinImpl<lgamma_r_las, [v3f32], 0, lgamma_r_svml>;
OclBuiltinImpl lgamma_r_gas_svml = OclBuiltinImpl<lgamma_r_gas, [v3f32], 0, lgamma_r_svml>;
OclBuiltinImpl lgamma_r_gen_svml = OclBuiltinImpl<lgamma_r_gen, [v3f32], 0, lgamma_r_svml>;

// round
OclBuiltinImpl round_svml_not_shared = OclBuiltinImpl<round, [v4f32, v8f32, v16f32, v2f64, v4f64, v8f64], 0, [{
    return __ocl_svml_$Target_round$SVMLSuffix($Arg0VarName);
  }]>;

// trunc
OclBuiltinImpl trunc_svml_not_shared = OclBuiltinImpl<trunc, [v4f32, v8f32, v16f32, v2f64, v4f64, v8f64], 0, [{
    return __ocl_svml_$Target_trunc$SVMLSuffix($Arg0VarName);
  }]>;

// half_cos
OclBuiltinImpl half_svml_not_shared = OclBuiltinImpl<half_cos, [v16f32], 0, [{
    return __ocl_svml_$Target_cos$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_divide
OclBuiltinImpl half_divide_svml_not_shared = OclBuiltinImpl<half_divide, [v16i32], 0, [{
    return __ocl_svml_$Target_div$SVMLSuffix_half($Arg0VarName, $Arg1VarName);
  }]>;

// half_exp
OclBuiltinImpl half_exp_svml_not_shared = OclBuiltinImpl<half_exp, [v16i32], 0, [{
    return __ocl_svml_$Target_exp$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_exp2
OclBuiltinImpl half_exp2_svml_not_shared = OclBuiltinImpl<half_exp2, [v16i32], 0, [{
    return __ocl_svml_$Target_exp2$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_exp10
OclBuiltinImpl half_exp10_svml_not_shared = OclBuiltinImpl<half_exp10, [v16i32], 0, [{
    return __ocl_svml_$Target_exp10$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_log
OclBuiltinImpl half_log_svml_not_shared = OclBuiltinImpl<half_log, [v16i32], 0, [{
    return __ocl_svml_$Target_log$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_log2
OclBuiltinImpl half_log2_svml_not_shared = OclBuiltinImpl<half_log2, [v16i32], 0, [{
    return __ocl_svml_$Target_log2$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_log10
OclBuiltinImpl half_log10_svml_not_shared = OclBuiltinImpl<half_log10, [v16i32], 0, [{
    return __ocl_svml_$Target_log10$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_powr
OclBuiltinImpl half_powr_svml_not_shared = OclBuiltinImpl<half_powr, [v16i32], 0, [{
    return __ocl_svml_$Target_powr$SVMLSuffix_half($Arg0VarName, $Arg1VarName);
  }]>;

// half_recip
OclBuiltinImpl half_recip_svml_not_shared = OclBuiltinImpl<half_recip, [v16i32], 0, [{
    return __ocl_svml_$Target_inv$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_rsqrt
OclBuiltinImpl half_rsqrt_svml_not_shared = OclBuiltinImpl<half_rsqrt, [v16i32], 0, [{
    return __ocl_svml_$Target_rsqrt$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_sin
OclBuiltinImpl half_sin_svml_not_shared = OclBuiltinImpl<half_sin, [v16i32], 0, [{
    return __ocl_svml_$Target_sin$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_sqrt
OclBuiltinImpl half_sqrt_svml_not_shared = OclBuiltinImpl<half_sqrt, [v16i32], 0, [{
    return __ocl_svml_$Target_sqrt$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_tan
OclBuiltinImpl half_tan_svml_not_shared = OclBuiltinImpl<half_tan, [v16i32], 0, [{
    return __ocl_svml_$Target_tan$SVMLSuffix_half($Arg0VarName);
  }]>;

// native_divide
OclBuiltinImpl native_divide_impl_not_shared = OclBuiltinImpl<native_divide, [v8f32], 0, [{
    return __ocl_svml_$Target_div$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_recip
OclBuiltinImpl native_recip_svml_not_shared = OclBuiltinImpl<native_recip, [v8f32], 0, [{
    return __ocl_svml_$Target_inv$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_rsqrt
OclBuiltinImpl native_rsqrt_impl_not_shared = OclBuiltinImpl<native_rsqrt, [v8f32], 0, [{
    return __ocl_svml_$Target_rsqrt$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_sqrt
OclBuiltinImpl native_sqrt_svml_not_shared = OclBuiltinImpl<native_sqrt, [v8f32, v8f64, v16f32], 0, [{
    // SLOW version. assume that SVML call will be even slower
    return sqrt($Arg0VarName);
  }]>;

// native_fract
// built in beyond OpenCL1.2 spec
OclBuiltinImpl native_fract_pas_f32 = OclBuiltinImpl<native_fract_pas, math_not_shared_types_v1_to_v16, 0, native_fract_code>;
OclBuiltinImpl native_fract_las_f32 = OclBuiltinImpl<native_fract_las, math_not_shared_types_v1_to_v16, 0, native_fract_code>;
OclBuiltinImpl native_fract_gas_f32 = OclBuiltinImpl<native_fract_gas, math_not_shared_types_v1_to_v16, 0, native_fract_code>;
OclBuiltinImpl native_fract_gen_f32 = OclBuiltinImpl<native_fract_gen, math_not_shared_types_v1_to_v16, 0, native_fract_code>;


// native_sincos
OclBuiltinImpl native_sincos_pas_svml = OclBuiltinImpl<native_sincos_pas, math_not_shared_types_v1_to_v16, 0, svml_native_code_sincos>;
OclBuiltinImpl native_sincos_las_svml = OclBuiltinImpl<native_sincos_las, math_not_shared_types_v1_to_v16, 0, svml_native_code_sincos>;
OclBuiltinImpl native_sincos_gas_svml = OclBuiltinImpl<native_sincos_gas, math_not_shared_types_v1_to_v16, 0, svml_native_code_sincos>;
OclBuiltinImpl native_sincos_gen_svml = OclBuiltinImpl<native_sincos_gen, math_not_shared_types_v1_to_v16, 0, svml_native_code_sincos>;



// This built-in is the one which the Packetizer replaces with the above
OclBuiltinImpl native_sincos_ret2ptr_pas = OclBuiltinImpl<native_sincos_pas2, [v4f32, v8f32, v16f32, v4f64, v8f64, v16f64], 0, native_sincos_ret2ptr>;


// This built-in is the one which the Packetizer replaces with the above
OclBuiltinImpl sincos_ret2ptr_pas = OclBuiltinImpl<sincos_pas2, [v4f32, v8f32, v16f32, v4f64, v8f64, v16f64], 0, sincos_ret2ptr>;

// This built-in is the one which the Packetizer replaces with the above
OclBuiltinImpl modf_ret2ptr_pas = OclBuiltinImpl<modf_pas2, [v4f32, v8f32, v16f32, v4f64, v8f64, v16f64], 0, modf_ret2ptr>;

// This built-in is the one which the Packetizer replaces with the above
OclBuiltinImpl native_fract_ret2ptr_pas = OclBuiltinImpl<native_fract_pas2, [v4f32, v8f32, v16f32, v4f64, v8f64, v16f64], 0, native_fract_ret2ptr>;
