// ****************************************************************************
//                                 int16
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_int16_AVX512 = OclBuiltinImpl<__ocl_imasked_load16, [v16i32],  0, [{
    return as_int16(_mm512_maskz_loadu_epi32((__mmask16) $Arg1VarName, (void const *)$Arg0VarName));
  }]>;

OclBuiltinImpl __ocl_masked_store_int16_AVX512 = OclBuiltinImpl<__ocl_imasked_store16, [v16i32],  0, [{
    _mm512_mask_storeu_epi32((void *)$Arg0VarName, (__mmask16) $Arg2VarName, (__m512i)$Arg1VarName);
  }]>;

// ****************************************************************************
//                                 long8
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_long8_AVX512 = OclBuiltinImpl<__ocl_imasked_load8, [v8i64],  0, [{
    return as_long8(_mm512_maskz_loadu_epi64((__mmask8) $Arg1VarName, (void const *)$Arg0VarName));
  }]>;

OclBuiltinImpl __ocl_masked_store_long8_AVX512 = OclBuiltinImpl<__ocl_imasked_store8, [v8i64],  0, [{
    _mm512_mask_storeu_epi64((void *)$Arg0VarName, (__mmask8) $Arg2VarName, (__m512i)$Arg1VarName);
  }]>;

// ****************************************************************************
//                                 float16
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_float16_AVX512 = OclBuiltinImpl<__ocl_imasked_load16, [v16f32],  0, [{
    return as_float16(_mm512_maskz_loadu_ps((__mmask16) $Arg1VarName, (void const *)$Arg0VarName));
  }]>;

OclBuiltinImpl __ocl_masked_store_float16_AVX512 = OclBuiltinImpl<__ocl_imasked_store16, [v16f32],  0, [{
    _mm512_mask_storeu_ps((void *)$Arg0VarName, (__mmask16) $Arg2VarName, __builtin_astype($Arg1VarName, __m512));
  }]>;

// ****************************************************************************
//                                 double8
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_double8_AVX512 = OclBuiltinImpl<__ocl_imasked_load8, [v8f64],  0, [{
    return as_double8(_mm512_maskz_loadu_pd((__mmask8) $Arg1VarName, (void const *)$Arg0VarName));
  }]>;

OclBuiltinImpl __ocl_masked_store_double8_AVX512 = OclBuiltinImpl<__ocl_imasked_store8, [v8f64],  0, [{
    _mm512_mask_storeu_pd((void *)$Arg0VarName, (__mmask8) $Arg2VarName, __builtin_astype($Arg1VarName, __m512d));
  }]>;

