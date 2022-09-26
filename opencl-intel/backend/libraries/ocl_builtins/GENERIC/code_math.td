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

defvar svml_code1 = [{
    return __ocl_svml_$Target_$NAME$SVMLSuffix($Arg0VarName);
  }];
defvar svml_code2 = [{
    return __ocl_svml_$Target_$NAME$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }];

defvar copysign_code = [{
    $Arg0igentype ux = __builtin_astype(($Arg0VarName), $Arg0igentype);
    $Arg0igentype uy = __builtin_astype(($Arg1VarName), $Arg1igentype);
    ux &= $ReturnBaseType_const_signMask;
    ux |= uy & ~$ReturnBaseType_const_signMask;
    return __builtin_astype((ux), $Arg0Type);
  }];

defvar fdim_code_vec = [{
    const $ReturnType NaNasFP = __builtin_astype($ReturnBaseType_QNAN_MASK, $ReturnBaseType);
    const $ReturnType Zero = 0;
    $Arg0Type xMinusY = $Arg0VarName - $Arg1VarName;
    $Arg0igentype XorYNaN = isnan($Arg0VarName) | isnan($Arg1VarName);
    $Arg0Type tmp = select(xMinusY, Zero, __builtin_astype((xMinusY), $Arg0igentype));
    return select(tmp, NaNasFP, XorYNaN);
  }];

defvar fmax_code = [{
    // according to OpenCL 1.2 ch. 6.12.2
    // fmax(-0., +0.) has to return -0.
    // WARNING: this implementation is not generic,
    //          it relies on CPU implementation of max builtin
    //          where order of arguments matters and must be preserved
    $Arg0igentype nan = isnan($Arg0VarName);
    $Arg0Type res = max($Arg1VarName, $Arg0VarName);
    res = select(res, $Arg1VarName, nan);
    return res;
  }];

defvar fmax_common_code = [{
    return fmax($Arg0VarName, $Arg1VarName);
  }];

defvar fmax_s_code = [{
    $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
    return $Func($Arg0VarName, vec$Arg1VarName);
  }];

defvar fmin_code = [{
    // according to OpenCL 1.2 ch. 6.12.2
    // fmin(+0., -0.) has to return +0.
    // WARNING: this implementation is not generic,
    //          it relies on CPU implementation of min builtin
    //          where order of arguments matters and must be preserved
    $Arg0igentype nan = isnan($Arg0VarName);
    $Arg0Type res = min($Arg1VarName, $Arg0VarName);
    res = select(res, $Arg1VarName, nan);
    return res;
  }];

defvar fmin_common_code = [{
    return fmin($Arg0VarName, $Arg1VarName);
  }];

defvar fmin_s_code = [{
    $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
    return $Func($Arg0VarName, vec$Arg1VarName);
  }];

// fract float vector
defvar fract_code_f32 = [{
    const $Arg0Type fractLimit = $ReturnBaseType_const_fractLimit;
    $ReturnType $ReturnVarName;
    // check NaN or Inf
    $Arg0igentype isNaNOrInf = (__builtin_astype(($Arg0VarName), $Arg0igentype) & $ReturnBaseType_const_expMask) == $ReturnBaseType_const_expMask;
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
      $Arg0Type xSignedZero = __builtin_astype((__builtin_astype(($Arg0VarName), $Arg0igentype) & ~$ReturnBaseType_const_expMask), $Arg0Type);
      // NaN. Put NaN from f4x to res
      $ReturnVarName = select($ReturnVarName, $Arg0VarName, isNaN);
      // +/- inf and zero. Put zero with proper sign
      $ReturnVarName = select($ReturnVarName, xSignedZero, isInf|isZero);
    }
    return $ReturnVarName;
  }];

defvar fract_ret2ptr_code = [{
    $Arg0Type res = fract($Arg0VarName, $Arg2VarName);
    *$Arg1VarName = res;
  }];

defvar fract_svml = [{
    return __ocl_svml_$Target_fract$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];

defvar frexp_svml = [{
    return __ocl_svml_$Target_frexp$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];

defvar ilogb_code = [{
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
    isZero.i = __builtin_astype((xx.u == 0), $Arg0igentype);
    res1.u = xx.u & expMask;
    isNaNorINF.i = __builtin_astype((res1.u == expMask), $Arg0igentype);
    isDenorm.i = __builtin_astype((res1.u == 0), $Arg0igentype);
    factor.u = (isDenorm.u & StorageI64) + StorageI127;
    xx.u &= isDenorm.u;
    isDenorm.u &= p64Storage;
    isDenorm.f *= xx.f;
    isDenorm.u &= expMask;
    res1.u >>= 23;
    res3.u = isDenorm.u >> 23;
    isNotDenorm.i = __builtin_astype((res3.u == 0), $Arg0igentype);
    res1.u -= factor.u;
    res3.u -= factor.u;
    res1.i = select(res3.i, res1.i, isNotDenorm.i);
    res1.i = select(res1.i, ($Arg0igentype)INT_MAX, isNaNorINF.i);
    res1.i = select(res1.i, ($Arg0igentype)INT_MIN, isZero.i);
    return __builtin_astype((res1.u), $Arg0igentype);
  }];

defvar ldexp_s_code = [{
  $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
  return $Func($Arg0VarName, vec$Arg1VarName);
}];

defvar lgamma_r_svml = [{
    return __ocl_svml_$Target_lgammar$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];

defvar mad_code = [{
    return ($Arg0VarName*$Arg1VarName+$Arg2VarName);
  }];

defvar maxmag_code = [{
    $Arg0Type $ReturnVarName;
    $Arg0Type xabs = fabs($Arg0VarName);
    $Arg1Type yabs = fabs($Arg1VarName);
    $Arg0Type xymax = fmax($Arg0VarName, $Arg1VarName);
    $Arg0igentype xLTy = xabs < yabs;
    $Arg0igentype yLTx = yabs < xabs;
    $ReturnVarName = select(xymax, $Arg1VarName, xLTy);
    $ReturnVarName = select($ReturnVarName, $Arg0VarName, yLTx);
    return $ReturnVarName;
  }];

defvar minmag_code = [{
    $Arg0Type $ReturnVarName;
    $Arg0Type xabs = fabs($Arg0VarName);
    $Arg1Type yabs = fabs($Arg1VarName);
    $Arg0Type xymin = fmin($Arg0VarName, $Arg1VarName);
    $Arg0igentype xGTy = xabs > yabs;
    $Arg0igentype yGTx = yabs > xabs;
    $ReturnVarName = select(xymin, $Arg1VarName, xGTy);
    $ReturnVarName = select($ReturnVarName, $Arg0VarName, yGTx);
    return $ReturnVarName;
  }];

defvar modf_svml = [{
    return __ocl_svml_$Target_modf$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];

defvar modf_ret2ptr = [{
    $Arg0Type res = modf($Arg0VarName, $Arg2VarName);
    *$Arg1VarName = res;
  }];

defvar remquo_svml = [{
    return __ocl_svml_$Target_remquo$SVMLSuffix($Arg0VarName, $Arg1VarName, ($Arg2NoASType)$Arg2VarName);
  }];

defvar sincos_svml = [{
    return __ocl_svml_$Target_sincos$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];

defvar sincos_ret2ptr = [{
    $Arg0Type res = sincos($Arg0VarName, $Arg2VarName);
    *$Arg1VarName = res;
  }];

// General ret2ptr linear strided vector code
// e.g. float16 sincos(float16 x, float *cos_ptr)
// Simply cast `cos_ptr` to `float16 *` type since the access is guaranteed to
// be linear-strided.
defvar ret2ptr_strided_code = [{
    return $FUNC($Arg0VarName, ($Arg0Type *)$Arg1VarName);
  }];

defvar ret2ptr_strided_mask_code = [{
    return $FUNC($Arg0VarName, ($Arg0Type *)$Arg1VarName, $Arg2VarName);
  }];
// e.g. void sincos_ret2ptr(float16 x, float *sin_ptr, float *cos_ptr)
defvar ret2ptr2_strided_code = [{
    $Arg0Type _res = $FUNC($Arg0VarName, $Arg2VarName);
    *($Arg0Type *)$Arg1VarName = _res;
  }];

defvar ret2ptr2_strided_mask_code = [{
    $Arg0Type _res = $FUNC($Arg0VarName, $Arg2VarName, $Arg3VarName);
    *($Arg0Type *)$Arg1VarName = _res;
  }];

// native

defvar svml_native_code1 = [{
    return __ocl_svml_$Target_$NAME$SVMLSuffix_native($Arg0VarName);
  }];
defvar svml_native_code2 = [{
    return __ocl_svml_$Target_$NAME$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }];

defvar native_fdim_code = [{
     return select($Arg0VarName-$Arg1VarName, ($Arg0Type) 0.0, __builtin_astype(($Arg0VarName-$Arg1VarName), $Arg0igentype));
  }];

defvar native_fmax_code = [{
    return max($Arg0VarName, $Arg1VarName);
  }];

defvar native_fmax_s_code = [{
    $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
    return $Func($Arg0VarName, vec$Arg1VarName);
  }];

defvar native_fmin_code = [{
    return min($Arg0VarName, $Arg1VarName);
  }];

defvar native_fmin_s_code = [{
    $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
    return $Func($Arg0VarName, vec$Arg1VarName);
  }];

defvar native_fmod_code = [{
    $Arg0Type m_trunc = trunc($Arg0VarName / $Arg1VarName);
    $Arg0Type m_mul = $Arg1VarName * m_trunc;
    return $Arg0VarName - m_mul;
  }];

defvar native_fract_code = [{
    *$Arg1VarName = floor($Arg0VarName);
    $Arg0Type m_sub = $Arg0VarName - *$Arg1VarName;
    return min(m_sub, ($Arg0Type) $ReturnBaseType_const_fractLimit);
  }];

defvar native_fract_ret2ptr = [{
    $Arg0Type res = native_fract($Arg0VarName, $Arg2VarName);
    *$Arg1VarName = res;
  }];

defvar native_hypot_code = [{
    return native_sqrt($Arg0VarName*$Arg0VarName + $Arg1VarName*$Arg1VarName);
  }];

defvar native_ilogb_code = [{
    $Arg0igentype tmp = __builtin_astype(($Arg0VarName), $Arg0igentype) & $Arg0BaseType_const_expMask;
    tmp >>= $Arg0BaseType_const_mantissaBits;
    tmp -= $Arg0BaseType_const_expOffset;
    return convert_int$Suffix(tmp);
  }];

defvar svml_native_code_sincos = [{
    return __ocl_svml_$Target_sincos$SVMLSuffix_native($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];

defvar native_sincos_ret2ptr = [{
    $Arg0Type res = native_sincos($Arg0VarName, $Arg2VarName);
    *$Arg1VarName = res;
  }];

// Relaxed

defvar svml_rm_code1 = [{
    return __ocl_svml_$Target_$NAME$SVMLSuffix_rm($Arg0VarName);
  }];
defvar svml_rm_code2 = [{
    return __ocl_svml_$Target_$NAME$SVMLSuffix_rm($Arg0VarName, $Arg1VarName);
  }];

defvar svml_rm_code_divide = [{
    return __ocl_svml_$Target_div$SVMLSuffix_rm($Arg0VarName, $Arg1VarName);
  }];

defvar svml_rm_code_sincos = [{
    return __ocl_svml_$Target_sincos$SVMLSuffix_rm($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];

// masked
// Please refer to masked_math.td for naming convention.

// gentype = f(gentype, mask)
defvar masked_impl_code_G_G_M = [{
    $Arg0VecType m$Arg0VarName = bitselect(($Arg0VecType)($DEF0), $Arg0VarName, $Arg1VarName);
    return $Func(m$Arg0VarName);
  }];

// gentype = f(gentype, gentype, mask)
defvar masked_impl_code_G_G2_M = [{
    $Arg0VecType m$Arg0VarName = bitselect(($Arg0VecType)($DEF0), $Arg0VarName, $Arg2VarName);
    $Arg1VecType m$Arg1VarName = bitselect(($Arg1VecType)($DEF1), $Arg1VarName, $Arg2VarName);
    return $Func(m$Arg0VarName, m$Arg1VarName);
  }];

// gentype = f(gentype, gentype, gentype, mask)
defvar masked_impl_code_G_G3_M = [{
    $Arg0VecType m$Arg0VarName = bitselect(($Arg0VecType)($DEF0), $Arg0VarName, $Arg3VarName);
    $Arg1VecType m$Arg1VarName = bitselect(($Arg1VecType)($DEF1), $Arg1VarName, $Arg3VarName);
    $Arg2VecType m$Arg2VarName = bitselect(($Arg2VecType)($DEF2), $Arg2VarName, $Arg3VarName);
    return $Func(m$Arg0VarName, m$Arg1VarName, m$Arg2VarName);
  }];

// gentype = f(gentype, gentype, any*, mask)
defvar masked_impl_code_G_G_P_M = [{
    $Arg0VecType m$Arg0VarName = bitselect(($Arg0VecType)($DEF0), $Arg0VarName, $Arg2VarName);
    return $Func(m$Arg0VarName, $Arg1VarName);
  }];

// void = f(gentype, any*, any*, mask)
defvar masked_impl_code_V_G_P_P_M = [{
    $Arg0VecType m$Arg0VarName = bitselect(($Arg0VecType)($DEF0), $Arg0VarName, $Arg3VarName);
    return $Func(m$Arg0VarName, $Arg1VarName, $Arg2VarName);
  }];

// gentype = f(gentype, gentype, any*, mask)
defvar masked_impl_code_G_G2_P_M = [{
    $Arg0VecType m$Arg0VarName = bitselect(($Arg0VecType)($DEF0), $Arg0VarName, $Arg3VarName);
    $Arg1VecType m$Arg1VarName = bitselect(($Arg1VecType)($DEF1), $Arg1VarName, $Arg3VarName);
    return $Func(m$Arg0VarName, m$Arg1VarName, $Arg2VarName);
  }];

// i32gentype = f(gentypef, i32mask)
defvar masked_impl_code_I32_FG_M = [{
    $Arg0VecType m$Arg0VarName = select(($Arg0VecType)($DEF0), $Arg0VarName, $Arg1VarName);
    return $Func(m$Arg0VarName);
  }];

// i32gentype = f(gentyped, i32mask)
defvar masked_impl_code_I32_DG_M = [{
    $Arg0VecType m$Arg0VarName = select(($Arg0VecType)($DEF0), $Arg0VarName, convert_long$VecLength($Arg1VarName));
    return $Func(m$Arg0VarName);
  }];

// gentypef = f(gentypef, i32gentype, mask)
defvar masked_impl_code_FG_FG_I32_M = [{
    $Arg0VecType m$Arg0VarName = bitselect(($Arg0VecType)($DEF0), $Arg0VarName, $Arg2VarName);
    $Arg1VecType m$Arg1VarName = select(($Arg1VecType)($DEF1), $Arg1VarName, __builtin_astype(($Arg2VarName), int$VecLength));
    return $Func(m$Arg0VarName, m$Arg1VarName);
  }];

// gentyped = f(gentyped, i32gentype, mask)
defvar masked_impl_code_DG_DG_I32_M = [{
    $Arg0VecType m$Arg0VarName = bitselect(($Arg0VecType)($DEF0), $Arg0VarName, $Arg2VarName);
    $Arg1VecType m$Arg1VarName = select(($Arg1VecType)($DEF1), $Arg1VarName, convert_int$VecLength(__builtin_astype(($Arg2VarName), long$VecLength)));
    return $Func(m$Arg0VarName, m$Arg1VarName);
  }];

// gentypef = f(ugentype, mask)
defvar masked_impl_code_FG_UG_M = [{
    $Arg0VecType m$Arg0VarName = select(($Arg0VecType)($DEF0), $Arg0VarName, __builtin_astype(($Arg1VarName), int$VecLength));
    return $Func(m$Arg0VarName);
  }];

// gentyped = f(ugentype, mask)
defvar masked_impl_code_DG_UG_M = [{
    $Arg0VecType m$Arg0VarName = select(($Arg0VecType)($DEF0), $Arg0VarName, __builtin_astype(($Arg1VarName), long$VecLength));
    return $Func(m$Arg0VarName);
  }];
