//===========================select===================================
code select_vector_v16_8_sse4 = [{ 
    return as_$ReturnType (_mm_blendv_epi8((__m128i)$Arg0VarName,(__m128i)$Arg1VarName,(__m128i)$Arg2VarName));
  }];

code select_vector_v4f32_sse4 = [{ 
    return _mm_blendv_ps($Arg0VarName, $Arg1VarName, __builtin_astype($Arg2VarName,__m128));
  }];

code select_vector_v2f64_sse4 = [{ 
    return _mm_blendv_pd($Arg0VarName, $Arg1VarName, __builtin_astype($Arg2VarName, __m128d));
  }];


OclBuiltinImpl iselect_v4f32_sse4 = OclBuiltinImpl<iselect, [v4f32], 0, select_vector_v4f32_sse4>;

OclBuiltinImpl iselect_v2f64_sse4 = OclBuiltinImpl<iselect, [v2f64], 0, select_vector_v2f64_sse4>;

OclBuiltinImpl uselect_v4f32_sse4 = OclBuiltinImpl<uselect, [v4f32], 0, select_vector_v4f32_sse4>;

OclBuiltinImpl uselect_v2f64_sse4 = OclBuiltinImpl<uselect, [v2f64], 0, select_vector_v2f64_sse4>;

OclBuiltinImpl iselect_v16_sse4 = OclBuiltinImpl<iselect, [v16i8, v16u8], 0, select_vector_v16_8_sse4>;

OclBuiltinImpl uselect_v16_sse4 = OclBuiltinImpl<uselect, [v16i8, v16u8], 0, select_vector_v16_8_sse4>;

