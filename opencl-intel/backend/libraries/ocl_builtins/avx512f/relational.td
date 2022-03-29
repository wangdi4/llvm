OclBuiltinImpl islessgreater2         =    OclBuiltinImpl    <islessgreater,  [v8f64, v16f32],           0, [{
    return (($Arg0VarName < $Arg1VarName) | ($Arg0VarName > $Arg1VarName)) ;
  }]>;

//======v16f32 and v8f64 implementation===============================

OclBuiltinImpl isnan_v16f32 = OclBuiltinImpl<isnan, [v16f32], 0, [{
    __mmask16 maskResult = _mm512_cmp_ps_mask($Arg0VarName, $Arg0VarName, _CMP_UNORD_Q);
    return as_$ReturnType(_mm512_maskz_set1_epi32(maskResult, (int)-1));
  }]>;

OclBuiltinImpl isnan_v8f64 = OclBuiltinImpl<isnan, [v8f64], 0, [{
   __mmask8 maskResult = _mm512_cmp_pd_mask($Arg0VarName, $Arg0VarName, _CMP_UNORD_Q);
   return _mm512_maskz_mov_epi64(maskResult, (long8)-1);
  }]>;

OclBuiltinImpl isordered_v16f32 = OclBuiltinImpl<isordered, [v16f32], 0, [{
    __mmask16 maskResult = _mm512_cmp_ps_mask($Arg0VarName, $Arg1VarName, _CMP_ORD_Q);
    return as_$ReturnType(_mm512_maskz_set1_epi32(maskResult, (int)-1));
  }]>;

OclBuiltinImpl isordered_v8f64 = OclBuiltinImpl<isordered, [v8f64], 0, [{
   __mmask8 maskResult = _mm512_cmp_pd_mask($Arg0VarName, $Arg1VarName, _CMP_ORD_Q);
   return _mm512_maskz_mov_epi64(maskResult, (long8)-1);
  }]>;

OclBuiltinImpl isunordered_v16f32 = OclBuiltinImpl<isunordered, [v16f32], 0, [{
    __mmask16 maskResult = _mm512_cmp_ps_mask($Arg0VarName, $Arg1VarName, _CMP_UNORD_S);
    return as_$ReturnType(_mm512_maskz_set1_epi32(maskResult, (int)-1));
  }]>;

OclBuiltinImpl isunordered_v8f64 = OclBuiltinImpl<isunordered, [v8f64], 0, [{
   __mmask8 maskResult = _mm512_cmp_pd_mask($Arg0VarName, $Arg1VarName, _CMP_UNORD_S);
   return _mm512_maskz_mov_epi64(maskResult, (long8)-1);
  }]>;

OclBuiltinImpl signbit_v16f32 = OclBuiltinImpl<signbit, [v16f32], 0, [{
    $Arg0VarName = as_$Arg0Type(as_$Arg0igentype($Arg0VarName) & ($Arg0igentype)fsign_mask);
    return as_$ReturnType(select($Arg0VarName, as_$Arg0Type(($Arg0igentype)FFFMask8),  as_$Arg0igentype($Arg0VarName)));
  }]>;

OclBuiltinImpl signbit_v8f64 = OclBuiltinImpl<signbit, [v8f64], 0, [{
    $Arg0VarName = as_$Arg0Type(as_$Arg0igentype($Arg0VarName) & ($Arg0igentype)dsign_mask);
    return as_$ReturnType(select($Arg0VarName,  as_$Arg0Type(($Arg0igentype)FF), as_$Arg0igentype($Arg0VarName)));
  }]>;

/*
//======v16i32\v16u32 and v8i64\v8u64 implementation===============================
OclBuiltinImpl any_v16i32 = OclBuiltinImpl<any, [v16i32], 0, [{
    int mask = _mm256_movemask_ps(__builtin_astype($Arg0VarName,__m256));
    return as_$ReturnType(((mask & 0xFF) != 0));
  }]>;

OclBuiltinImpl any_v8i64 = OclBuiltinImpl<any, [v8i64], 0, [{
    int mask = _mm256_movemask_ps(__builtin_astype($Arg0VarName,__m256));
    return as_$ReturnType(((mask & 0xAA) != 0));
  }]>;

OclBuiltinImpl all_v16i32 = OclBuiltinImpl<all, [v16i32], 0, [{
    int mask = _mm256_movemask_ps(__builtin_astype($Arg0VarName,__m256));
    return as_$ReturnType(((mask & 0xFF) == 0xFF));
  }]>;

OclBuiltinImpl all_v8i64 = OclBuiltinImpl<all, [v8i64], 0, [{
    int mask = _mm256_movemask_ps(__builtin_astype($Arg0VarName,__m256));
    return as_$ReturnType(((mask & 0xAA) == 0xAA));
  }]>;
*/

// v32v64
list<OclType> math_v32v64_expand = ExpandTypesByVFAndFlatten<[v1f32, v1f64], [32, 64]>.Tout;
OclBuiltinImpl isnan_expand_v32v64 = OclBuiltinImpl<isnan, math_v32v64_expand, 0, Expand1>;
