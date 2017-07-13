//===========================select===================================
code select_vector_v16_8_sse4 = [{ 
    return $Arg2VarName ? $Arg1VarName : $Arg0VarName;
  }];

code select_vector_v4f32_sse4 = [{ 
    return $Arg2VarName ? $Arg1VarName : $Arg0VarName;
  }];

code select_vector_v2f64_sse4 = [{ 
    return $Arg2VarName ? $Arg1VarName : $Arg0VarName;
  }];


OclBuiltinImpl iselect_v4f32_sse4 = OclBuiltinImpl<iselect, [v4f32], 0, select_vector_v4f32_sse4>;

OclBuiltinImpl iselect_v2f64_sse4 = OclBuiltinImpl<iselect, [v2f64], 0, select_vector_v2f64_sse4>;

OclBuiltinImpl uselect_v4f32_sse4 = OclBuiltinImpl<uselect, [v4f32], 0, select_vector_v4f32_sse4>;

OclBuiltinImpl uselect_v2f64_sse4 = OclBuiltinImpl<uselect, [v2f64], 0, select_vector_v2f64_sse4>;

OclBuiltinImpl iselect_v16_sse4 = OclBuiltinImpl<iselect, [v16i8, v16u8], 0, select_vector_v16_8_sse4>;

OclBuiltinImpl uselect_v16_sse4 = OclBuiltinImpl<uselect, [v16i8, v16u8], 0, select_vector_v16_8_sse4>;

