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

// acos
OclBuiltinImpl acos_svml_not_shared = OclBuiltinImpl<acos,  math_not_shared_types, 0, [{
    return __ocl_svml_$Target_acos$SVMLSuffix($Arg0VarName);
  }]>;
// acosh
OclBuiltinImpl acosh_svml_not_shared = OclBuiltinImpl<acosh, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_acosh$SVMLSuffix($Arg0VarName);
  }]>;

// acospi
OclBuiltinImpl acospi_svml_not_shared = OclBuiltinImpl<acospi, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_acospi$SVMLSuffix($Arg0VarName);
  }]>;

// asin
OclBuiltinImpl asin_svml_not_shared = OclBuiltinImpl<asin, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_asin$SVMLSuffix($Arg0VarName);

  }]>;

// asinh
OclBuiltinImpl asinh_svml_not_shared = OclBuiltinImpl<asinh, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_asinh$SVMLSuffix($Arg0VarName);
  }]>;

// asinpi
OclBuiltinImpl asinpi_svml_not_shared = OclBuiltinImpl<asinpi, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_asinpi$SVMLSuffix($Arg0VarName);
  }]>;
// atan
OclBuiltinImpl atan_svml_not_shared = OclBuiltinImpl<atan, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_atan$SVMLSuffix($Arg0VarName);
  }]>;

// atan2
OclBuiltinImpl atan2_svml_not_shared = OclBuiltinImpl<atan2, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_atan2$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// atanh
OclBuiltinImpl atanh_svml_not_shared = OclBuiltinImpl<atanh, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_atanh$SVMLSuffix($Arg0VarName);
  }]>;
// atanpi
OclBuiltinImpl atanpi_svml_not_shared = OclBuiltinImpl<atanpi, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_atanpi$SVMLSuffix($Arg0VarName);
  }]>;

// atan2pi
OclBuiltinImpl atan2pi_svml_not_shared = OclBuiltinImpl<atan2pi, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_atan2pi$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// cbrt
OclBuiltinImpl cbrt_svml_not_shared = OclBuiltinImpl<cbrt, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_cbrt$SVMLSuffix($Arg0VarName);
  }]>;
// ceil
// ceil not used in shared lib at all
OclBuiltinImpl ceil_svml = OclBuiltinImpl<ceil, ceil.Types, 0, [{
    return __ocl_svml_$Target_ceil$SVMLSuffix($Arg0VarName);
  }]>;
// cos
OclBuiltinImpl cos_svml_not_shared = OclBuiltinImpl<cos, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_cos$SVMLSuffix($Arg0VarName);
  }]>;

// cosh
OclBuiltinImpl cosh_svml_not_shared = OclBuiltinImpl<cosh, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_cosh$SVMLSuffix($Arg0VarName);
  }]>;

// cospi
OclBuiltinImpl cospi_svml_not_shared = OclBuiltinImpl<cospi, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_cospi$SVMLSuffix($Arg0VarName);
  }]>;

// erfc
OclBuiltinImpl erfc_svml_not_shared = OclBuiltinImpl<erfc, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_erfc$SVMLSuffix($Arg0VarName);
  }]>;

// erf
OclBuiltinImpl erf_svml_not_shared = OclBuiltinImpl<erf, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_erf$SVMLSuffix($Arg0VarName);
  }]>;
// exp
OclBuiltinImpl exp_svml_not_shared = OclBuiltinImpl<exp, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_exp$SVMLSuffix($Arg0VarName);
  }]>;

// exp2
OclBuiltinImpl exp2_svml_not_shared = OclBuiltinImpl<exp2, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_exp2$SVMLSuffix($Arg0VarName);
  }]>;

// exp10
OclBuiltinImpl exp10_svml_not_shared = OclBuiltinImpl<exp10, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_exp10$SVMLSuffix($Arg0VarName);
  }]>;

// expm1
OclBuiltinImpl expm1_svml_not_shared = OclBuiltinImpl<expm1, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_expm1$SVMLSuffix($Arg0VarName);
  }]>;
// floor
OclBuiltinImpl floor_svml_not_shared = OclBuiltinImpl<floor, [v4f32, v8f32, v2f64, v4f64, v8f64, v16f32], 0, [{
    return __ocl_svml_$Target_floor$SVMLSuffix($Arg0VarName);
  }]>;

// fma
OclBuiltinImpl fma_svml_not_shared = OclBuiltinImpl<fma, fma.Types, 0, [{
    return __ocl_svml_$Target_fma$SVMLSuffix($Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
// fmod
OclBuiltinImpl fmod_svml_not_shared = OclBuiltinImpl<fmod, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_fmod$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

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
code sincos_svml_rm = [{
    return __ocl_svml_$Target_sincos$SVMLSuffix_rm($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];
OclBuiltinImpl sincos_pas_svml_rm_not_shared = OclBuiltinImpl<sincos_pas_rm, [v16f32], 0, sincos_svml_rm>;
OclBuiltinImpl sincos_las_svml_rm_not_shared = OclBuiltinImpl<sincos_las_rm, [v16f32], 0, sincos_svml_rm>;
OclBuiltinImpl sincos_gas_svml_rm_not_shared = OclBuiltinImpl<sincos_gas_rm, [v16f32], 0, sincos_svml_rm>;
OclBuiltinImpl sincos_gen_svml_rm_not_shared = OclBuiltinImpl<sincos_gen_rm, [v16f32], 0, sincos_svml_rm>;

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
                                                                     v2i64, v3i64, v4i64, v8i64, v16i64], 0, [{
    return any($Arg0VarName);
  }]>;


// fract svml
code fract_svml = [{
    return __ocl_svml_$Target_fract$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];

// fract float vector
code fract_impl_f32 = [{
    const $Arg0Type fractLimit = $ReturnBaseType_const_fractLimit;
    $ReturnType $ReturnVarName;
    // check NaN or Inf
    $Arg0igentype isNaNOrInf = (as_$Arg0igentype($Arg0VarName) & $ReturnBaseType_const_expMask) == $ReturnBaseType_const_expMask;
    // check for zero
    $Arg0igentype isZero = ($Arg0VarName == ($Arg0Type)0.0);
    // obtain mask with elements zero or Nan or infinity. we need to keep sign
    $Arg0igentype isZeroNaNInf = isNaNOrInf | isZero;

    // floor(x)
    *iptr = floor($Arg0VarName);
    // x - floor(x)
    $Arg0Type f4x_1 = $Arg0VarName - *iptr;
    // fmin( x - floor (x), 0x1.fffffep-1f )
    $ReturnVarName = min(f4x_1, fractLimit);

    // approach with if() instead of unconditional blend
    if( intel_movemask(isZeroNaNInf) )
    {
      // check for NaN
      $Arg0igentype isNaN = isnan($Arg0VarName );
      $Arg0igentype isInf = ~isNaN & isNaNOrInf;
      // make zero with sign
      $Arg0Type xSignedZero = as_$Arg0Type(as_$Arg0igentype($Arg0VarName) & ~$ReturnBaseType_const_expMask);
      // NaN. Put NaN from f4x to res
      $ReturnVarName = select($ReturnVarName, $Arg0VarName, isNaN);
      // +/- inf and zero. Put zero with proper sign
      $ReturnVarName = select($ReturnVarName, xSignedZero, isInf|isZero);
    }
    return $ReturnVarName;
  }];

OclBuiltinImpl fract_pas_v1f32 = OclBuiltinImpl<fract_pas, [v1f32], 0, Promote_fract>;
OclBuiltinImpl fract_pas_f32 = OclBuiltinImpl<fract_pas, [v2f32, v3f32, v4f32, v8f32, v16f32], 0, fract_impl_f32>;
OclBuiltinImpl fract_pas_f64 = OclBuiltinImpl<fract_pas, [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, fract_svml>;

OclBuiltinImpl fract_las_v1f32 = OclBuiltinImpl<fract_las, [v1f32], 0, Promote_fract>;
OclBuiltinImpl fract_las_f32 = OclBuiltinImpl<fract_las, [v2f32, v3f32, v4f32, v8f32, v16f32], 0, fract_impl_f32>;
OclBuiltinImpl fract_las_f64 = OclBuiltinImpl<fract_las, [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, fract_svml>;

OclBuiltinImpl fract_gas_v1f32 = OclBuiltinImpl<fract_gas, [v1f32], 0, Promote_fract>;
OclBuiltinImpl fract_gas_f32 = OclBuiltinImpl<fract_gas, [v2f32, v3f32, v4f32, v8f32, v16f32], 0, fract_impl_f32>;
OclBuiltinImpl fract_gas_f64 = OclBuiltinImpl<fract_gas, [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, fract_svml>;

OclBuiltinImpl fract_gen_v1f32 = OclBuiltinImpl<fract_gen, [v1f32], 0, Promote_fract>;
OclBuiltinImpl fract_gen_f32 = OclBuiltinImpl<fract_gen, [v2f32, v3f32, v4f32, v8f32, v16f32], 0, fract_impl_f32>;
OclBuiltinImpl fract_gen_f64 = OclBuiltinImpl<fract_gen, [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, fract_svml>;

// frexp
code frexp_svml = [{
    return __ocl_svml_$Target_frexp$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];
OclBuiltinImpl frexp_pas_svml = OclBuiltinImpl<frexp_pas, frexp_pas.Types, 0, frexp_svml>;
OclBuiltinImpl frexp_las_svml = OclBuiltinImpl<frexp_las, frexp_las.Types, 0, frexp_svml>;
OclBuiltinImpl frexp_gas_svml = OclBuiltinImpl<frexp_gas, frexp_gas.Types, 0, frexp_svml>;
OclBuiltinImpl frexp_gen_svml = OclBuiltinImpl<frexp_gen, frexp_gen.Types, 0, frexp_svml>;


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
OclBuiltinImpl lgamma_svml_not_shared = OclBuiltinImpl<lgamma, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_lgamma$SVMLSuffix($Arg0VarName);
  }]>;
// lgamma_r
code lgamma_r_svml = [{
    return __ocl_svml_$Target_lgammar$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];
OclBuiltinImpl lgamma_r_pas_svml = OclBuiltinImpl<lgamma_r_pas, lgamma_r_pas.Types, 0, lgamma_r_svml>;
OclBuiltinImpl lgamma_r_las_svml = OclBuiltinImpl<lgamma_r_las, lgamma_r_las.Types, 0, lgamma_r_svml>;
OclBuiltinImpl lgamma_r_gas_svml = OclBuiltinImpl<lgamma_r_gas, lgamma_r_gas.Types, 0, lgamma_r_svml>;
OclBuiltinImpl lgamma_r_gen_svml = OclBuiltinImpl<lgamma_r_gen, lgamma_r_gen.Types, 0, lgamma_r_svml>;

// log
OclBuiltinImpl log_svml_not_shared = OclBuiltinImpl<log, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_log$SVMLSuffix($Arg0VarName);
  }]>;

// log2
OclBuiltinImpl log2_svml_not_shared = OclBuiltinImpl<log2, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_log2$SVMLSuffix($Arg0VarName);
  }]>;

// log10
OclBuiltinImpl log10_svml_not_shared = OclBuiltinImpl<log10, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_log10$SVMLSuffix($Arg0VarName);
  }]>;

// log1p
OclBuiltinImpl log1p_svml_not_shared = OclBuiltinImpl<log1p, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_log1p$SVMLSuffix($Arg0VarName);
  }]>;

// logb
OclBuiltinImpl logb_svml_not_shared = OclBuiltinImpl<logb, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_logb$SVMLSuffix($Arg0VarName);
  }]>;
  
// remainder
OclBuiltinImpl remainder_svml_not_shared = OclBuiltinImpl<remainder, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_remainder$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// rootn
OclBuiltinImpl rootn_svml_not_shared = OclBuiltinImpl<rootn, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_rootn$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// modf
code modf_svml = [{
    return __ocl_svml_$Target_modf$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];
OclBuiltinImpl modf_pas_svml = OclBuiltinImpl<modf_pas, modf_pas.Types, 0, modf_svml>;
OclBuiltinImpl modf_las_svml = OclBuiltinImpl<modf_las, modf_las.Types, 0, modf_svml>;
OclBuiltinImpl modf_gas_svml = OclBuiltinImpl<modf_gas, modf_gas.Types, 0, modf_svml>;
OclBuiltinImpl modf_gen_svml = OclBuiltinImpl<modf_gen, modf_gen.Types, 0, modf_svml>;

// remquo
code remquo_svml = [{
    return __ocl_svml_$Target_remquo$SVMLSuffix($Arg0VarName, $Arg1VarName, ($Arg2NoASType)$Arg2VarName);
  }];
OclBuiltinImpl remquo_pas_svml = OclBuiltinImpl<remquo_pas, remquo_pas.Types, 0, remquo_svml>;
OclBuiltinImpl remquo_las_svml = OclBuiltinImpl<remquo_las, remquo_las.Types, 0, remquo_svml>;
OclBuiltinImpl remquo_gas_svml = OclBuiltinImpl<remquo_gas, remquo_gas.Types, 0, remquo_svml>;
OclBuiltinImpl remquo_gen_svml = OclBuiltinImpl<remquo_gen, remquo_gen.Types, 0, remquo_svml>;

// rint
OclBuiltinImpl rint_svml = OclBuiltinImpl<rint, [v4f32, v8f32, v2f64, v4f64], 0, [{
    return __ocl_svml_$Target_rint$SVMLSuffix($Arg0VarName);
  }]>;

// round
OclBuiltinImpl round_svml_not_shared = OclBuiltinImpl<round, [v4f32, v8f32, v16f32, v2f64, v4f64, v8f64], 0, [{
    return __ocl_svml_$Target_round$SVMLSuffix($Arg0VarName);
  }]>;

// rsqrt
OclBuiltinImpl rsqrt_svml_not_shared = OclBuiltinImpl<rsqrt, [v8f32, v16f32, v2f64, v4f64, v8f64], 0, [{
    return __ocl_svml_$Target_rsqrt$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl hypot_svml = OclBuiltinImpl<hypot, math_not_shared_types, 0, [{
  return __ocl_svml_$Target_hypot$SVMLSuffix($Arg0VarName, $Arg1VarName);
}]>;

// sin
OclBuiltinImpl sin_svml_not_shared = OclBuiltinImpl<sin, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_sin$SVMLSuffix($Arg0VarName);
  }]>;

// sincos
code sincos_svml = [{
    return __ocl_svml_$Target_sincos$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];
OclBuiltinImpl sincos_pas_svml = OclBuiltinImpl<sincos_pas, sincos_pas.Types, 0, sincos_svml>;
OclBuiltinImpl sincos_las_svml = OclBuiltinImpl<sincos_las, sincos_las.Types, 0, sincos_svml>;
OclBuiltinImpl sincos_gas_svml = OclBuiltinImpl<sincos_gas, sincos_gas.Types, 0, sincos_svml>;
OclBuiltinImpl sincos_gen_svml = OclBuiltinImpl<sincos_gen, sincos_gen.Types, 0, sincos_svml>;

// sinh
OclBuiltinImpl sinh_svml_not_shared = OclBuiltinImpl<sinh, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_sinh$SVMLSuffix($Arg0VarName);
  }]>;

// sinpi
OclBuiltinImpl sinpi_svml_not_shared = OclBuiltinImpl<sinpi, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_sinpi$SVMLSuffix($Arg0VarName);
  }]>;
// sqrt
OclBuiltinImpl sqrt_svml_not_shared = OclBuiltinImpl<sqrt, [v8f32, v16f32, v4f64, v8f64], 0, [{
    return __ocl_svml_$Target_sqrt$SVMLSuffix($Arg0VarName);
  }]>;

// tan
OclBuiltinImpl tan_svml_not_shared = OclBuiltinImpl<tan, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_tan$SVMLSuffix($Arg0VarName);
  }]>;

// tanh
OclBuiltinImpl tanh_svml_not_shared = OclBuiltinImpl<tanh, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_tanh$SVMLSuffix($Arg0VarName);
  }]>;

// tanpi
OclBuiltinImpl tanpi_svml_not_shared = OclBuiltinImpl<tanpi, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_tanpi$SVMLSuffix($Arg0VarName);
  }]>;

// tgamma
OclBuiltinImpl tgamma_svml_not_shared = OclBuiltinImpl<tgamma, math_not_shared_types, 0, [{
    return __ocl_svml_$Target_tgamma$SVMLSuffix($Arg0VarName);
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
code native_fract_impl = [{
    *$Arg1VarName = floor($Arg0VarName);
    $Arg0Type m_sub = $Arg0VarName - *$Arg1VarName;
    return min(m_sub, ($Arg0Type) $ReturnBaseType_const_fractLimit);
  }];

OclBuiltinImpl native_fract_pas_f32 = OclBuiltinImpl<native_fract_pas, native_fract_pas.Types, 0, native_fract_impl>;
OclBuiltinImpl native_fract_las_f32 = OclBuiltinImpl<native_fract_las, native_fract_las.Types, 0, native_fract_impl>;
OclBuiltinImpl native_fract_gas_f32 = OclBuiltinImpl<native_fract_gas, native_fract_gas.Types, 0, native_fract_impl>;
OclBuiltinImpl native_fract_gen_f32 = OclBuiltinImpl<native_fract_gen, native_fract_gen.Types, 0, native_fract_impl>;


// native_sincos
code native_sincos_svml = [{
    return __ocl_svml_$Target_sincos$SVMLSuffix_native($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];
OclBuiltinImpl native_sincos_pas_svml = OclBuiltinImpl<native_sincos_pas, native_sincos_pas.Types, 0, native_sincos_svml>;
OclBuiltinImpl native_sincos_las_svml = OclBuiltinImpl<native_sincos_las, native_sincos_las.Types, 0, native_sincos_svml>;
OclBuiltinImpl native_sincos_gas_svml = OclBuiltinImpl<native_sincos_gas, native_sincos_gas.Types, 0, native_sincos_svml>;
OclBuiltinImpl native_sincos_gen_svml = OclBuiltinImpl<native_sincos_gen, native_sincos_gen.Types, 0, native_sincos_svml>;



// This built-in is the one which the Packetizer replaces with the above
code native_sincos_ret2ptr = [{
    $Arg0Type res = native_sincos($Arg0VarName, $Arg2VarName);
    *$Arg1VarName = res;
  }];
OclBuiltinImpl native_sincos_ret2ptr_pas = OclBuiltinImpl<native_sincos_pas2, [v4f32, v8f32, v16f32, v4f64, v8f64, v16f64], 0, native_sincos_ret2ptr>;


// This built-in is the one which the Packetizer replaces with the above
code sincos_ret2ptr = [{
    $Arg0Type res = sincos($Arg0VarName, $Arg2VarName);
    *$Arg1VarName = res;
  }];
OclBuiltinImpl sincos_ret2ptr_pas = OclBuiltinImpl<sincos_pas2, [v4f32, v8f32, v16f32, v4f64, v8f64, v16f64], 0, sincos_ret2ptr>;

// This built-in is the one which the Packetizer replaces with the above
code modf_ret2ptr = [{
    $Arg0Type res = modf($Arg0VarName, $Arg2VarName);
    *$Arg1VarName = res;
  }];
OclBuiltinImpl modf_ret2ptr_pas = OclBuiltinImpl<modf_pas2, [v4f32, v8f32, v16f32, v4f64, v8f64, v16f64], 0, modf_ret2ptr>;

// This built-in is the one which the Packetizer replaces with the above
code native_fract_ret2ptr = [{
    $Arg0Type res = native_fract($Arg0VarName, $Arg2VarName);
    *$Arg1VarName = res;
  }];
OclBuiltinImpl native_fract_ret2ptr_pas = OclBuiltinImpl<native_fract_pas2, [v4f32, v8f32, v16f32, v4f64, v8f64, v16f64], 0, native_fract_ret2ptr>;

// This built-in is the one which the Packetizer replaces with the above
code fract_ret2ptr = [{
    $Arg0Type res = fract($Arg0VarName, $Arg2VarName);
    *$Arg1VarName = res;
  }];
OclBuiltinImpl fract_ret2ptr_pas = OclBuiltinImpl<fract_pas2, [v4f32, v8f32, v16f32, v4f64, v8f64, v16f64], 0, fract_ret2ptr>;

