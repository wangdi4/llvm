// vim:ts=2:sw=2:et:

// NOTE:
// this is generic implementations for math functions common for all cpu
// platforms, so functions collected here has no optimisation for any
// particular architecture with data types used in this file, for example:

// built-in acos has no specific code for data types
// v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64
// and code for built-ins acosf, acosf2, acosf3, acosf4, acosf8,
// acos, acos2, acos3, acos4, acos16 is placed in shared library

// but the code for built-ins acos8 and acosf16 has some differences
// between sse42 and avx realisations, so built-ins for data types
// v8f64 and v16f32 exist in another file: math_not_shared.td

// math_shared and math_not_shared MUST not overlap. i.e. contain
// the same functions

// Why v8f64 and v16f32 is not here?
// Because there is no svml shared function for this types
list<OclType> math_shared_types = [v1f32, v2f32, v3f32, v4f32, v8f32,
                              v1f64, v2f64, v3f64, v4f64];

list<OclType> math_shared_f32_types = [v1f32, v2f32, v3f32, v4f32, v8f32];

// acos
OclBuiltinImpl acos_svml = OclBuiltinImpl<acos, math_shared_types, 0, [{
    return __ocl_svml_$Target_acos$SVMLSuffix($Arg0VarName);
  }]>;

// acosh
OclBuiltinImpl acosh_svml = OclBuiltinImpl<acosh, math_shared_types, 0, [{
    return __ocl_svml_$Target_acosh$SVMLSuffix($Arg0VarName);
  }]>;

// acospi
OclBuiltinImpl acospi_svml = OclBuiltinImpl<acospi, math_shared_types, 0, [{
    return __ocl_svml_$Target_acospi$SVMLSuffix($Arg0VarName);
  }]>;

// asin
OclBuiltinImpl asin_svml = OclBuiltinImpl<asin, math_shared_types, 0, [{
    return __ocl_svml_$Target_asin$SVMLSuffix($Arg0VarName);
  }]>;

// asinh
OclBuiltinImpl asinh_svml = OclBuiltinImpl<asinh, math_shared_types, 0, [{
    return __ocl_svml_$Target_asinh$SVMLSuffix($Arg0VarName);
  }]>;

// asinpi
OclBuiltinImpl asinpi_svml = OclBuiltinImpl<asinpi, math_shared_types, 0, [{
    return __ocl_svml_$Target_asinpi$SVMLSuffix($Arg0VarName);
  }]>;

// atan
OclBuiltinImpl atan_svml = OclBuiltinImpl<atan, math_shared_types, 0, [{
    return __ocl_svml_$Target_atan$SVMLSuffix($Arg0VarName);
  }]>;

// atan2
OclBuiltinImpl atan2_svml = OclBuiltinImpl<atan2, math_shared_types, 0, [{
    return __ocl_svml_$Target_atan2$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// atanh
OclBuiltinImpl atanh_svml = OclBuiltinImpl<atanh, math_shared_types, 0, [{
    return __ocl_svml_$Target_atanh$SVMLSuffix($Arg0VarName);
  }]>;

// atanpi
OclBuiltinImpl atanpi_svml = OclBuiltinImpl<atanpi, math_shared_types, 0, [{
    return __ocl_svml_$Target_atanpi$SVMLSuffix($Arg0VarName);
  }]>;

// atan2pi
OclBuiltinImpl atan2pi_svml = OclBuiltinImpl<atan2pi, math_shared_types, 0, [{
    return __ocl_svml_$Target_atan2pi$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// cbrt
OclBuiltinImpl cbrt_svml = OclBuiltinImpl<cbrt, math_shared_types, 0, [{
    return __ocl_svml_$Target_cbrt$SVMLSuffix($Arg0VarName);
  }]>;

// copysign
OclBuiltinImpl copysign_impl = OclBuiltinImpl<copysign, copysign.Types, 0, [{
    $Arg0Type $ReturnVarName;
    $Arg0igentype ux = as_$Arg0igentype($Arg0VarName);
    $Arg0igentype uy = as_$Arg1igentype($Arg1VarName);
    ux &= $ReturnBaseType_const_signMask;
    ux |= uy & ~$ReturnBaseType_const_signMask;
    return as_$Arg0Type(ux);
  }]>;

// cos
OclBuiltinImpl cos_svml = OclBuiltinImpl<cos, math_shared_types, 0, [{
    return __ocl_svml_$Target_cos$SVMLSuffix($Arg0VarName);
  }]>;

// cosh
OclBuiltinImpl cosh_svml = OclBuiltinImpl<cosh, math_shared_types, 0, [{
    return __ocl_svml_$Target_cosh$SVMLSuffix($Arg0VarName);
  }]>;

// cospi
OclBuiltinImpl cospi_svml = OclBuiltinImpl<cospi, math_shared_types, 0, [{
    return __ocl_svml_$Target_cospi$SVMLSuffix($Arg0VarName);
  }]>;

// erfc
OclBuiltinImpl erfc_svml = OclBuiltinImpl<erfc, math_shared_types, 0, [{
    return __ocl_svml_$Target_erfc$SVMLSuffix($Arg0VarName);
  }]>;

// erf
OclBuiltinImpl erf_svml = OclBuiltinImpl<erf, math_shared_types, 0, [{
    return __ocl_svml_$Target_erf$SVMLSuffix($Arg0VarName);
  }]>;

// exp
OclBuiltinImpl exp_svml = OclBuiltinImpl<exp, math_shared_types, 0, [{
    return __ocl_svml_$Target_exp$SVMLSuffix($Arg0VarName);
  }]>;

// exp2
OclBuiltinImpl exp2_svml = OclBuiltinImpl<exp2, math_shared_types, 0, [{
    return __ocl_svml_$Target_exp2$SVMLSuffix($Arg0VarName);
  }]>;

// exp10
OclBuiltinImpl exp10_svml = OclBuiltinImpl<exp10, math_shared_types, 0, [{
    return __ocl_svml_$Target_exp10$SVMLSuffix($Arg0VarName);
  }]>;

// expm1
OclBuiltinImpl expm1_svml = OclBuiltinImpl<expm1, math_shared_types, 0, [{
    return __ocl_svml_$Target_expm1$SVMLSuffix($Arg0VarName);
  }]>;

// fabs
OclBuiltinImpl fabs_impl = OclBuiltinImpl<fabs, fabs.Types, 0, [{
    return as_$Arg0Type(as_$Arg0igentype($Arg0VarName) & $ReturnBaseType_const_signMask);
  }]>;

// OpenCL 2.0 -cl-fast-relaxed-math functions
// float16 is excluded from shared lib since ssse3 and sse42
// svml libs don't include v16f32 FRM functions
// cos_rm
OclBuiltinImpl cos_svml_rm = OclBuiltinImpl<cos_rm, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_cos$SVMLSuffix_rm($Arg0VarName);
  }]>;

// exp_rm
OclBuiltinImpl exp_svml_rm = OclBuiltinImpl<exp_rm, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_exp$SVMLSuffix_rm($Arg0VarName);
  }]>;

// exp2_rm
OclBuiltinImpl exp2_svml_rm = OclBuiltinImpl<exp2_rm, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_exp2$SVMLSuffix_rm($Arg0VarName);
  }]>;

// exp10_rm
OclBuiltinImpl exp10_svml_rm = OclBuiltinImpl<exp10_rm, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_exp10$SVMLSuffix_rm($Arg0VarName);
  }]>;

// log_rm
OclBuiltinImpl log_svml_rm = OclBuiltinImpl<log_rm, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_log$SVMLSuffix_rm($Arg0VarName);
  }]>;

// log2_rm
OclBuiltinImpl log2_svml_rm = OclBuiltinImpl<log2_rm, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_log2$SVMLSuffix_rm($Arg0VarName);
  }]>;

// pow_rm
OclBuiltinImpl pow_svml_rm = OclBuiltinImpl<pow_rm, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_pow$SVMLSuffix_rm($Arg0VarName, $Arg1VarName);
  }]>;

// sin_rm
OclBuiltinImpl sin_svml_rm = OclBuiltinImpl<sin_rm, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_sin$SVMLSuffix_rm($Arg0VarName);
  }]>;

// tan_rm
OclBuiltinImpl tan_svml_rm = OclBuiltinImpl<tan_rm, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_tan$SVMLSuffix_rm($Arg0VarName);
  }]>;

// sincos_rm
code sincos_svml_rm = [{
    return __ocl_svml_$Target_sincos$SVMLSuffix_rm($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];
OclBuiltinImpl sincos_pas_svml_rm = OclBuiltinImpl<sincos_pas_rm, math_shared_f32_types, 0, sincos_svml_rm>;
OclBuiltinImpl sincos_las_svml_rm = OclBuiltinImpl<sincos_las_rm, math_shared_f32_types, 0, sincos_svml_rm>;
OclBuiltinImpl sincos_gas_svml_rm = OclBuiltinImpl<sincos_gas_rm, math_shared_f32_types, 0, sincos_svml_rm>;

// fdim scalar
// scalar implementation is separate from vector because of select() behaviour
// In scalar case select uses all bits in mask to check if not zero
// In vector case select uses most significant bit in mask
// fdim scalar float promotes to $Native version
OclBuiltinImpl fdim_impl_v1f32_promote = OclBuiltinImpl<fdim, [v1f32, v1f64], 0, Promote2>;

// fdim vector
OclBuiltinImpl fdim_impl_vec = OclBuiltinImpl<fdim, [v2f32, v3f32, v4f32, v8f32, v16f32,
                          v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    const $ReturnType NaNasFP = as_$ReturnBaseType($ReturnBaseType_const_nanStorage);
    const $ReturnType Zero = 0;
    $Arg0Type xMinusY = $Arg0VarName - $Arg1VarName;
    $Arg0igentype XorYNaN = isnan($Arg0VarName) | isnan($Arg1VarName);
    $Arg0Type tmp = select(xMinusY, Zero, as_$Arg0igentype(xMinusY));
    return select(tmp, NaNasFP, XorYNaN);
  }]>;

// floor
OclBuiltinImpl floor_svml = OclBuiltinImpl<floor, [v1f32, v2f32, v3f32, v1f64, v3f64], 0, [{
    return __ocl_svml_$Target_floor$SVMLSuffix($Arg0VarName);
  }]>;

// fmax
OclBuiltinImpl fmax_impl = OclBuiltinImpl<fmax, fmax.Types, 0, [{
    // according to OpenCL 1.2 ch. 6.12.2
    // fmax(-0., +0.) has to return -0.
    // WARNING: this implementation is not generic,
    //          it relies on CPU implementation of max builtin
    //          where order of arguments matters and must be preserved
    $Arg0igentype nan = isnan($Arg0VarName);
    $Arg0Type res = max($Arg1VarName, $Arg0VarName);
    res = select(res, $Arg1VarName, nan);
    return res;
  }]>;

// fmax_common
OclBuiltinImpl fmax_common_impl = OclBuiltinImpl<fmax_common, fmax_common.Types, 0, [{
    return fmax($Arg0VarName, $Arg1VarName);
  }]>;

// The following code implements the builtin variants with scalar parameters.
// The implementation will prompt these scalar parameters into vector ones
// and call the corresponding builtins with all vector parameters.
// fmax_s
OclBuiltinImpl fmax_s_generic = OclBuiltinImpl<fmax_s, fmax_s.Types, 0, [{
  $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
  return $Func($Arg0VarName, vec$Arg1VarName);
}]>;

// fmin implementation
OclBuiltinImpl fmin_impl = OclBuiltinImpl<fmin, fmin.Types, 0, [{
    // according to OpenCL 1.2 ch. 6.12.2
    // fmin(+0., -0.) has to return +0.
    // WARNING: this implementation is not generic,
    //          it relies on CPU implementation of min builtin
    //          where order of arguments matters and must be preserved
    $Arg0igentype nan = isnan($Arg0VarName);
    $Arg0Type res = min($Arg1VarName, $Arg0VarName);
    res = select(res, $Arg1VarName, nan);
    return res;
  }]>;

// fmin_common
OclBuiltinImpl fmin_common_impl = OclBuiltinImpl<fmin_common, fmin_common.Types, 0, [{
    return fmin($Arg0VarName, $Arg1VarName);
  }]>;

// fmin_s
OclBuiltinImpl fmin_s_generic = OclBuiltinImpl<fmin_s, fmin_s.Types, 0, [{
  $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
  return $Func($Arg0VarName, vec$Arg1VarName);
}]>;


// fmod
OclBuiltinImpl fmod_svml = OclBuiltinImpl<fmod, math_shared_types, 0, [{
    return __ocl_svml_$Target_fmod$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// hypot
// NOTE: there is fast inlined implementation of hypot using newton-raphson and quick native_rsqrt function
// See: hypot_impl_nr in SSE42
OclBuiltinImpl hypot_svml = OclBuiltinImpl<hypot, math_shared_types, 0, [{
    return __ocl_svml_$Target_hypot$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// ilogb vector float
OclBuiltinImpl ilogb_impl_f32 = OclBuiltinImpl<ilogb, [v3f32, v4f32, v8f32, v16f32], 0, [{
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
    res1.i = select(res1.i, ($Arg0igentype)INT_MAX, isNaNorINF.i);
    res1.i = select(res1.i, ($Arg0igentype)INT_MIN, isZero.i);
    return as_$Arg0igentype(res1.u);
  }]>;

// ilogb double
OclBuiltinImpl ilogb_svml_f64 = OclBuiltinImpl<ilogb, [v1f64, v2f64, v3f64, v4f64],  0, [{
    return __ocl_svml_$Target_ilogb$SVMLSuffix($Arg0VarName);
  }]>;

// ldexp types is a special case, math_shared_types are not used
// ldexp
OclBuiltinImpl ldexp_svml = OclBuiltinImpl<ldexp, math_shared_types, 0, [{
    return __ocl_svml_$Target_ldexp$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// ldexp_s
OclBuiltinImpl ldexp_s_generic = OclBuiltinImpl<ldexp_s, ldexp_s.Types, 0, [{
  $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
  return $Func($Arg0VarName, vec$Arg1VarName);
}]>;

// lgamma
OclBuiltinImpl lgamma_svml = OclBuiltinImpl<lgamma, math_shared_types, 0, [{
    return __ocl_svml_$Target_lgamma$SVMLSuffix($Arg0VarName);
  }]>;

// log
OclBuiltinImpl log_svml = OclBuiltinImpl<log, math_shared_types, 0, [{
    return __ocl_svml_$Target_log$SVMLSuffix($Arg0VarName);
  }]>;

// log2
OclBuiltinImpl log2_svml = OclBuiltinImpl<log2, math_shared_types, 0, [{
    return __ocl_svml_$Target_log2$SVMLSuffix($Arg0VarName);
  }]>;

// log10
OclBuiltinImpl log10_svml = OclBuiltinImpl<log10, math_shared_types, 0, [{
    return __ocl_svml_$Target_log10$SVMLSuffix($Arg0VarName);
  }]>;

// log1p
OclBuiltinImpl log1p_svml = OclBuiltinImpl<log1p, math_shared_types, 0, [{
    return __ocl_svml_$Target_log1p$SVMLSuffix($Arg0VarName);
  }]>;

// logb
OclBuiltinImpl logb_svml = OclBuiltinImpl<logb, math_shared_types, 0, [{
    return __ocl_svml_$Target_logb$SVMLSuffix($Arg0VarName);
  }]>;

// mad
OclBuiltinImpl mad_impl = OclBuiltinImpl<mad, mad.Types, 0, [{
    return ($Arg0VarName*$Arg1VarName+$Arg2VarName);
  }]>;

// maxmag types is a special case, math_shared_types are not used
// maxmag
OclBuiltinImpl maxmag_impl = OclBuiltinImpl<maxmag, maxmag.Types, 0, [{
    $Arg0Type $ReturnVarName;
    $Arg0Type xabs = fabs($Arg0VarName);
    $Arg1Type yabs = fabs($Arg1VarName);
    $Arg0Type xymax = fmax($Arg0VarName, $Arg1VarName);
    $Arg0igentype xLTy = xabs < yabs;
    $Arg0igentype yLTx = yabs < xabs;
    $ReturnVarName = select(xymax, $Arg1VarName, xLTy);
    $ReturnVarName = select($ReturnVarName, $Arg0VarName, yLTx);
    return $ReturnVarName;
  }]>;

// minmag types is a special case, math_shared_types are not used
// minmag
OclBuiltinImpl minmag_impl = OclBuiltinImpl<minmag, minmag.Types, 0, [{
    $Arg0Type $ReturnVarName;
    $Arg0Type xabs = fabs($Arg0VarName);
    $Arg1Type yabs = fabs($Arg1VarName);
    $Arg0Type xymin = fmin($Arg0VarName, $Arg1VarName);
    $Arg0igentype xGTy = xabs > yabs;
    $Arg0igentype yGTx = yabs > xabs;
    $ReturnVarName = select(xymin, $Arg1VarName, xGTy);
    $ReturnVarName = select($ReturnVarName, $Arg0VarName, yGTx);
    return $ReturnVarName;
  }]>;

// nan
OclBuiltinImpl nan_impl = OclBuiltinImpl<nan, nan.Types, 0, [{
    return as_$ReturnType($Arg0VarName | $ReturnBaseType_const_nanStorage);
  }]>;

// nextafter
OclBuiltinImpl nextafter_svml = OclBuiltinImpl<nextafter, math_shared_types, 0, [{
    return __ocl_svml_$Target_nextafter$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// pow
OclBuiltinImpl pow_svml = OclBuiltinImpl<pow, math_shared_types, 0, [{
    return __ocl_svml_$Target_pow$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// pown types is a special case, math_shared_types are not used
// pown
OclBuiltinImpl pown_svml = OclBuiltinImpl<pown, math_shared_types, 0, [{
    return __ocl_svml_$Target_pown$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// powr
OclBuiltinImpl powr_svml = OclBuiltinImpl<powr, math_shared_types, 0, [{
    return __ocl_svml_$Target_powr$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// remainder
OclBuiltinImpl remainder_svml = OclBuiltinImpl<remainder, math_shared_types, 0, [{
    return __ocl_svml_$Target_remainder$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// rint
OclBuiltinImpl rint_svml = OclBuiltinImpl<rint, [v1f32, v2f32, v3f32, v1f64, v3f64], 0, [{
    return __ocl_svml_$Target_rint$SVMLSuffix($Arg0VarName);
  }]>;

// round
OclBuiltinImpl round_svml = OclBuiltinImpl<round, [v1f32, v2f32, v3f32, v1f64, v3f64], 0, [{
    return __ocl_svml_$Target_round$SVMLSuffix($Arg0VarName);
  }]>;

// rsqrt
OclBuiltinImpl rsqrt_svml = OclBuiltinImpl<rsqrt, [v1f32, v2f32, v3f32, v4f32, v1f64, v3f64], 0, [{
    return __ocl_svml_$Target_rsqrt$SVMLSuffix($Arg0VarName);
  }]>;

// rootn types is a special case, math_shared_types are not used
// rootn
OclBuiltinImpl rootn_svml = OclBuiltinImpl<rootn, math_shared_types, 0, [{
    return __ocl_svml_$Target_rootn$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// sin
OclBuiltinImpl sin_svml = OclBuiltinImpl<sin, math_shared_types, 0, [{
    return __ocl_svml_$Target_sin$SVMLSuffix($Arg0VarName);
  }]>;

// sinh
OclBuiltinImpl sinh_svml = OclBuiltinImpl<sinh, math_shared_types, 0, [{
    return __ocl_svml_$Target_sinh$SVMLSuffix($Arg0VarName);
  }]>;

// sinpi
OclBuiltinImpl sinpi_svml = OclBuiltinImpl<sinpi, math_shared_types, 0, [{
    return __ocl_svml_$Target_sinpi$SVMLSuffix($Arg0VarName);
  }]>;

// sqrt
OclBuiltinImpl sqrt_svml = OclBuiltinImpl<sqrt, [v1f32, v2f32, v3f32, v4f32, v1f64, v2f64, v3f64], 0, [{
    return __ocl_svml_$Target_sqrt$SVMLSuffix($Arg0VarName);
  }]>;

// tan
OclBuiltinImpl tan_svml = OclBuiltinImpl<tan, math_shared_types, 0, [{
    return __ocl_svml_$Target_tan$SVMLSuffix($Arg0VarName);
  }]>;

// tanh
OclBuiltinImpl tanh_svml = OclBuiltinImpl<tanh, math_shared_types, 0, [{
    return __ocl_svml_$Target_tanh$SVMLSuffix($Arg0VarName);
  }]>;

// tanpi
OclBuiltinImpl tanpi_svml = OclBuiltinImpl<tanpi, math_shared_types, 0, [{
    return __ocl_svml_$Target_tanpi$SVMLSuffix($Arg0VarName);
  }]>;

// tgamma
OclBuiltinImpl tgamma_svml = OclBuiltinImpl<tgamma, math_shared_types, 0, [{
    return __ocl_svml_$Target_tgamma$SVMLSuffix($Arg0VarName);
  }]>;

// trunc
OclBuiltinImpl trunc_svml = OclBuiltinImpl<trunc, [v1f32, v2f32, v3f32, v1f64, v3f64], 0, [{
    return __ocl_svml_$Target_trunc$SVMLSuffix($Arg0VarName);
  }]>;

// half_cos
OclBuiltinImpl half_cos_svml = OclBuiltinImpl<half_cos, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_cos$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_divide
OclBuiltinImpl half_divide_svml = OclBuiltinImpl<half_divide, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_div$SVMLSuffix_half($Arg0VarName, $Arg1VarName);
  }]>;

// half_exp
OclBuiltinImpl half_exp_svml = OclBuiltinImpl<half_exp, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_exp$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_exp2
OclBuiltinImpl half_exp2_svml = OclBuiltinImpl<half_exp2, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_exp2$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_exp10
OclBuiltinImpl half_exp10_svml = OclBuiltinImpl<half_exp10, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_exp10$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_log
OclBuiltinImpl half_log_svml = OclBuiltinImpl<half_log, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_log$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_log2
OclBuiltinImpl half_log2_svml = OclBuiltinImpl<half_log2, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_log2$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_log10
OclBuiltinImpl half_log10_svml = OclBuiltinImpl<half_log10, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_log10$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_powr
OclBuiltinImpl half_powr_svml = OclBuiltinImpl<half_powr, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_powr$SVMLSuffix_half($Arg0VarName, $Arg1VarName);
  }]>;

// half_recip
OclBuiltinImpl half_recip_svml = OclBuiltinImpl<half_recip, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_inv$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_rsqrt
OclBuiltinImpl half_rsqrt_svml = OclBuiltinImpl<half_rsqrt, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_rsqrt$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_sin
OclBuiltinImpl half_sin_svml = OclBuiltinImpl<half_sin, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_sin$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_sqrt
OclBuiltinImpl half_sqrt_svml = OclBuiltinImpl<half_sqrt, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_sqrt$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_tan
OclBuiltinImpl half_tan_svml = OclBuiltinImpl<half_tan, math_shared_f32_types, 0, [{
    return __ocl_svml_$Target_tan$SVMLSuffix_half($Arg0VarName);
  }]>;


// native_cos
OclBuiltinImpl native_cos_svml = OclBuiltinImpl<native_cos, math_shared_types, 0, [{
    return __ocl_svml_$Target_cos$SVMLSuffix_native($Arg0VarName);
  }]>;


// native_divide
OclBuiltinImpl native_divide_impl = OclBuiltinImpl<native_divide, [v1f32, v2f32, v3f32, v4f32, v1f64, v2f64, v3f64, v4f64], 0, [{
    return __ocl_svml_$Target_div$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_exp
OclBuiltinImpl native_exp_svml = OclBuiltinImpl<native_exp, math_shared_types, 0, [{
    return __ocl_svml_$Target_exp$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_exp2
OclBuiltinImpl native_exp2_svml = OclBuiltinImpl<native_exp2, math_shared_types, 0, [{
    return __ocl_svml_$Target_exp2$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_exp10
OclBuiltinImpl native_exp10_svml = OclBuiltinImpl<native_exp10, math_shared_types, 0, [{
    return __ocl_svml_$Target_exp10$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_fmax implementation
// beyond OpenCL1.2 spec
OclBuiltinImpl native_fmax_impl = OclBuiltinImpl<native_fmax, native_fmax.Types, 0, [{
    return max($Arg0VarName, $Arg1VarName);
  }]>;

// native_fmax_s
OclBuiltinImpl native_fmax_s_impl = OclBuiltinImpl<native_fmax_s, native_fmax_s.Types, 0, [{
  $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
  return $Func($Arg0VarName, vec$Arg1VarName);
}]>;

// native_fmin implementation
// beyond OpenCL1.2 spec
OclBuiltinImpl native_fmin_impl = OclBuiltinImpl<native_fmin, native_fmin.Types, 0, [{
    return min($Arg0VarName, $Arg1VarName);
  }]>;

// native_fmin_s
OclBuiltinImpl native_fmin_s_impl = OclBuiltinImpl<native_fmin_s, native_fmin_s.Types, 0, [{
  $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
  return $Func($Arg0VarName, vec$Arg1VarName);
}]>;

// native_log
OclBuiltinImpl native_log_svml = OclBuiltinImpl<native_log, math_shared_types, 0, [{
    return __ocl_svml_$Target_log$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_log2
OclBuiltinImpl native_log2_svml = OclBuiltinImpl<native_log2, math_shared_types, 0, [{
    return __ocl_svml_$Target_log2$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_log10
OclBuiltinImpl native_log10_svml = OclBuiltinImpl<native_log10, math_shared_types, 0, [{
    return __ocl_svml_$Target_log10$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_powr
OclBuiltinImpl native_powr_svml = OclBuiltinImpl<native_powr, math_shared_types, 0, [{
    return __ocl_svml_$Target_powr$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_recip
OclBuiltinImpl native_recip_svml = OclBuiltinImpl<native_recip, [v2f32, v3f32, v4f32, v2f64, v3f64, v4f64], 0, [{
    return __ocl_svml_$Target_inv$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_rsqrt
OclBuiltinImpl native_rsqrt_impl = OclBuiltinImpl<native_rsqrt, [v2f32, v3f32, v4f32, v2f64, v3f64, v4f64], 0, [{
    return __ocl_svml_$Target_rsqrt$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_sin
OclBuiltinImpl native_sin_svml = OclBuiltinImpl<native_sin, math_shared_types, 0, [{
    return __ocl_svml_$Target_sin$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_sqrt
OclBuiltinImpl native_sqrt_svml = OclBuiltinImpl<native_sqrt, [v2f32, v3f32, v4f32, v2f64, v3f64, v4f64, v16f64], 0, [{
    // SLOW version. assume that SVML call will be even slower
    return sqrt($Arg0VarName);
  }]>;

// native_tan
OclBuiltinImpl native_tan_svml = OclBuiltinImpl<native_tan, math_shared_types, 0, [{
    return __ocl_svml_$Target_tan$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_acos
OclBuiltinImpl native_acos_svml = OclBuiltinImpl<native_acos, math_shared_types, 0, [{
    return __ocl_svml_$Target_acos$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_acosh
OclBuiltinImpl native_acosh_svml = OclBuiltinImpl<native_acosh, math_shared_types, 0, [{
    return __ocl_svml_$Target_acosh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_acospi
OclBuiltinImpl native_acospi_svml = OclBuiltinImpl<native_acospi, math_shared_types, 0, [{
    return __ocl_svml_$Target_acospi$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_asin
OclBuiltinImpl native_asin_svml = OclBuiltinImpl<native_asin, math_shared_types, 0, [{
    return __ocl_svml_$Target_asin$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_asinh
OclBuiltinImpl native_asinh_svml = OclBuiltinImpl<native_asinh, math_shared_types, 0, [{
    return __ocl_svml_$Target_asinh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_asinpi
OclBuiltinImpl native_asinpi_svml = OclBuiltinImpl<native_asinpi, math_shared_types, 0, [{
    return __ocl_svml_$Target_asinpi$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_atan
OclBuiltinImpl native_atan_svml = OclBuiltinImpl<native_atan, math_shared_types, 0, [{
    return __ocl_svml_$Target_atan$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_atan2
OclBuiltinImpl native_atan2_svml = OclBuiltinImpl<native_atan2, math_shared_types, 0, [{
    return __ocl_svml_$Target_atan2$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_atanh
OclBuiltinImpl native_atanh_svml = OclBuiltinImpl<native_atanh, math_shared_types, 0, [{
    return __ocl_svml_$Target_atanh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_atanpi
OclBuiltinImpl native_atanpi_svml = OclBuiltinImpl<native_atanpi, math_shared_types, 0, [{
    return __ocl_svml_$Target_atanpi$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_atan2pi
OclBuiltinImpl native_atan2pi_svml = OclBuiltinImpl<native_atan2pi, math_shared_types, 0, [{
    return __ocl_svml_$Target_atan2pi$SVMLSuffix_native($Arg0VarName, $Arg0VarName);
  }]>;

// native_cbrt
OclBuiltinImpl native_cbrt_svml = OclBuiltinImpl<native_cbrt, math_shared_types, 0, [{
    return __ocl_svml_$Target_cbrt$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_cosh
OclBuiltinImpl native_cosh_svml = OclBuiltinImpl<native_cosh, math_shared_types, 0, [{
    return __ocl_svml_$Target_cosh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_cospi
OclBuiltinImpl native_cospi_svml = OclBuiltinImpl<native_cospi, math_shared_types, 0, [{
    return __ocl_svml_$Target_cospi$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_erfc
OclBuiltinImpl native_erfc_svml = OclBuiltinImpl<native_erfc, math_shared_types, 0, [{
    return __ocl_svml_$Target_erfc$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_erf
OclBuiltinImpl native_erf_svml = OclBuiltinImpl<native_erf, math_shared_types, 0, [{
    return __ocl_svml_$Target_erf$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_expm1
OclBuiltinImpl native_expm1_svml = OclBuiltinImpl<native_expm1, math_shared_types, 0, [{
    return __ocl_svml_$Target_expm1$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_fdim
// built in beyond OpenCL1.2 spec
OclBuiltinImpl native_fdim_f32_impl = OclBuiltinImpl<native_fdim, native_fdim.Types, 0, [{
     return select($Arg0VarName-$Arg1VarName, ($Arg0Type) 0.0, as_$Arg0igentype($Arg0VarName-$Arg1VarName));
  }]>;

// native_fmod
// built in beyond OpenCL1.2 spec
OclBuiltinImpl native_fmod_f32_impl = OclBuiltinImpl<native_fmod, native_fmod.Types, 0, [{
    $Arg0Type m_trunc = trunc($Arg0VarName / $Arg1VarName);
    $Arg0Type m_mul = $Arg1VarName * m_trunc;
    return $Arg0VarName - m_mul;
  }]>;

  // native_hypot
OclBuiltinImpl native_hypot_impl = OclBuiltinImpl<native_hypot, native_hypot.Types, 0, [{
    return native_sqrt($Arg0VarName*$Arg0VarName + $Arg1VarName*$Arg1VarName);
  }]>;

// native_ilogb
// built in beyond OpenCL1.2 spec
OclBuiltinImpl native_ilogb_f32_impl = OclBuiltinImpl<native_ilogb, native_ilogb.Types, 0, [{
    $Arg0igentype tmp = as_$Arg0igentype($Arg0VarName) & $Arg0BaseType_const_expMask;
    tmp >>= $Arg0BaseType_const_mantissaBits;
    tmp -= $Arg0BaseType_const_expOffset;
    return convert_int$Suffix(tmp);
  }]>;

// native_log1p
OclBuiltinImpl native_log1p_svml = OclBuiltinImpl<native_log1p, math_shared_types, 0, [{
    return __ocl_svml_$Target_log1p$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_pow
OclBuiltinImpl native_pow_svml = OclBuiltinImpl<native_pow, math_shared_types, 0, [{
    return __ocl_svml_$Target_pow$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_pown
OclBuiltinImpl native_pown_svml = OclBuiltinImpl<native_pown, math_shared_types, 0, [{
    return __ocl_svml_$Target_pown$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_rcbrt
OclBuiltinImpl native_rcbrt_svml = OclBuiltinImpl<native_rcbrt, math_shared_types, 0, [{
    return __ocl_svml_$Target_rcbrt$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_rootn
OclBuiltinImpl native_rootn_svml = OclBuiltinImpl<native_rootn, math_shared_types, 0, [{
    return __ocl_svml_$Target_rootn$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_sinh
OclBuiltinImpl native_sinh_svml = OclBuiltinImpl<native_sinh, math_shared_types, 0, [{
    return __ocl_svml_$Target_sinh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_sinpi
OclBuiltinImpl native_sinpi_svml = OclBuiltinImpl<native_sinpi, math_shared_types, 0, [{
    return __ocl_svml_$Target_sinpi$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_tanh
OclBuiltinImpl native_tanh_svml = OclBuiltinImpl<native_tanh, math_shared_types, 0, [{
    return __ocl_svml_$Target_tanh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_tanpi
OclBuiltinImpl native_tanpi_svml = OclBuiltinImpl<native_tanpi, math_shared_types, 0, [{
    return __ocl_svml_$Target_tanpi$SVMLSuffix_native($Arg0VarName);
  }]>;
