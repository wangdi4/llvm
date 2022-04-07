// vim:ts=2:sw=2:et:

//////////////////////////////////////////////////////////////////////////////////////////////
// AVX2 math implementation
//////////////////////////////////////////////////////////////////////////////////////////////

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
