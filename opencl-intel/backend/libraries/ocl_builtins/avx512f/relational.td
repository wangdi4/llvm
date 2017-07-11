OclBuiltinImpl islessgreater2         =    OclBuiltinImpl    <islessgreater,  [v8f64, v16f32],           0, [{
    return (($Arg0VarName < $Arg1VarName) | ($Arg0VarName > $Arg1VarName)) ;
  }]>;
//==============select===============
code select_vector_v16f = [{
    __mmask16 mask = _mm512_test_epi32_mask((__m512i)$Arg2VarName, (__m512i)((int16)0x80000000));
    return as_$ReturnType(_mm512_mask_blend_ps(mask, __builtin_astype($Arg0VarName, __m512),
      __builtin_astype($Arg1VarName, __m512)));
  }];

code select_vector_v8d = [{
    __mmask8 mask = _mm512_test_epi64_mask((__m512i)$Arg2VarName, (__m512i)((long8)0x8000000000000000));
    return as_$ReturnType(_mm512_mask_blend_pd(mask, __builtin_astype($Arg0VarName, __m512d),
    __builtin_astype($Arg1VarName, __m512d)));
  }];

code select_vector_v16i = [{
    $ReturnType Nulls = ($ReturnType)(0);
    return as_$ReturnType($Arg2VarName == Nulls) ? Arg0VarName : Arg1VarName;
  }];

code select_vector_v8l = [{
    $ReturnType Nulls = ($ReturnType)(0);
    return as_$ReturnType($Arg2VarName == Nulls) ? Arg0VarName : Arg1VarName;
  }];

//======v16f32 and v8f64 implementation===============================

OclBuiltinImpl isnan_v16f32 = OclBuiltinImpl<isnan, [v16f32], 0, [{
    __mmask16 maskResult = _mm512_cmp_ps_mask($Arg0VarName, $Arg0VarName, _CMP_UNORD_Q);
    return as_$ReturnType(_mm512_maskz_set1_epi32(maskResult, (int)-1));
  }]>;

OclBuiltinImpl isnan_v8f64 = OclBuiltinImpl<isnan, [v8f64], 0, [{
   __mmask8 maskResult = _mm512_cmp_pd_mask($Arg0VarName, $Arg0VarName, _CMP_UNORD_Q);
   return as_$ReturnType(_mm512_maskz_set1_epi64(maskResult, (long long)-1));
  }]>;

OclBuiltinImpl isordered_v16f32 = OclBuiltinImpl<isordered, [v16f32], 0, [{
    __mmask16 maskResult = _mm512_cmp_ps_mask($Arg0VarName, $Arg1VarName, _CMP_ORD_Q);
    return as_$ReturnType(_mm512_maskz_set1_epi32(maskResult, (int)-1));
  }]>;

OclBuiltinImpl isordered_v8f64 = OclBuiltinImpl<isordered, [v8f64], 0, [{
    __mmask8 maskResult = _mm512_cmp_pd_mask($Arg0VarName, $Arg1VarName, _CMP_ORD_Q);
    return as_$ReturnType(_mm512_maskz_set1_epi64(maskResult, (long long)-1));
  }]>;

OclBuiltinImpl isunordered_v16f32 = OclBuiltinImpl<isunordered, [v16f32], 0, [{
    __mmask16 maskResult = _mm512_cmp_ps_mask($Arg0VarName, $Arg1VarName, _CMP_UNORD_S);
    return as_$ReturnType(_mm512_maskz_set1_epi32(maskResult, (int)-1));
  }]>;

OclBuiltinImpl isunordered_v8f64 = OclBuiltinImpl<isunordered, [v8f64], 0, [{
    __mmask8 maskResult = _mm512_cmp_pd_mask($Arg0VarName, $Arg1VarName, _CMP_UNORD_S);
    return as_$ReturnType(_mm512_maskz_set1_epi64(maskResult, (long long)-1));
  }]>;

OclBuiltinImpl signbit_v16f32 = OclBuiltinImpl<signbit, [v16f32], 0, [{
    $Arg0VarName = as_$Arg0Type(as_$Arg0igentype($Arg0VarName) & ($Arg0igentype)fsign_mask);
    return as_$ReturnType(select($Arg0VarName, as_$Arg0Type(($Arg0igentype)FFFMask8),  as_$Arg0igentype($Arg0VarName)));
  }]>;

OclBuiltinImpl signbit_v8f64 = OclBuiltinImpl<signbit, [v8f64], 0, [{
    $Arg0VarName = as_$Arg0Type(as_$Arg0igentype($Arg0VarName) & ($Arg0igentype)dsign_mask);
    return as_$ReturnType(select($Arg0VarName,  as_$Arg0Type(($Arg0igentype)FF), as_$Arg0igentype($Arg0VarName)));
  }]>;

OclBuiltinImpl bitselect_v16f32 = OclBuiltinImpl<bitselect, [v16f32, v8f64, v16i32, v16u32], 0, [{
    $Arg0VarName = as_$Arg0Type( ~as_$Arg0igentype($Arg2VarName) & as_$Arg0igentype($Arg0VarName)); //res1 is $Arg0VarName if $Arg2VarName is 0
    $Arg1VarName = as_$Arg0Type( as_$Arg0igentype($Arg2VarName) & as_$Arg0igentype($Arg1VarName)); //res2 bit is $Arg1VarName if $Arg2VarName is 1
    return as_$ReturnType(as_$Arg0igentype($Arg0VarName) | as_$Arg0igentype($Arg1VarName));
  }]>;

OclBuiltinImpl iselect_v16f32 = OclBuiltinImpl<iselect, [v16f32], 0, select_vector_v16f>;

OclBuiltinImpl iselect_v4f64 = OclBuiltinImpl<iselect, [v8f64], 0, select_vector_v8d>;

OclBuiltinImpl uselect_v16f32 = OclBuiltinImpl<uselect, [v16f32], 0, select_vector_v16f>;

OclBuiltinImpl uselect_v4f64 = OclBuiltinImpl<uselect, [v8f64], 0, select_vector_v8d>;

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
OclBuiltinImpl bitselect_v16i32 = OclBuiltinImpl<bitselect, [v8i32, v8u32, v16i32, v16u32, v8i64, v8u64], 0, [{
    $Arg0igentype res1 = (~ as_$Arg0igentype($Arg2VarName) & as_$Arg0igentype($Arg0VarName));
    $Arg0igentype res2 = ( as_$Arg0igentype($Arg2VarName) & as_$Arg0igentype($Arg1VarName));
    return as_$ReturnType(res1 | res2);
  }]>;
/*
OclBuiltinImpl bitselect_v8i64 = OclBuiltinImpl<bitselect, [v8i64, v8u64], 0, [{
    return as_$ReturnType(bitselect(as_int16($Arg0VarName), as_int16($Arg1VarName), as_int16($Arg2VarName)));
  }]>;
*/
OclBuiltinImpl iselect_v8 = OclBuiltinImpl<iselect, [v16i32, v16u32], 0, select_vector_v16i>;

OclBuiltinImpl iselect_v16 = OclBuiltinImpl<iselect, [v8i64, v8u64], 0, select_vector_v8l>;

OclBuiltinImpl uselect_v8 = OclBuiltinImpl<uselect, [v16i32, v16u32], 0, select_vector_v16i>;

OclBuiltinImpl uselect_v16 = OclBuiltinImpl<uselect, [v8i64, v8u64], 0, select_vector_v8l>;
