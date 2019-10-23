// vim:ts=2:sw=2:et:

//promote and expand

  OclBuiltinImpl step_expand  = OclBuiltinImpl <step, [v8f32, v16f32, v3f64, v4f64, v8f64], 0, Expand2>;
  OclBuiltinImpl sign_expand  = OclBuiltinImpl <sign, [v8f32, v16f32, v3f64, v4f64, v8f64], 0, Expand1>;

OclBuiltinImpl ext_mask_to_v4_4 = OclBuiltinImpl<__ocl_extend_mask_v4, [v4i32], 0,
  [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl ext_mask_to_v4_8 = OclBuiltinImpl<__ocl_extend_mask_v4, [v8i32], 0,
  [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
      0, 0, 1, 1, 2, 2, 3, 3);
  }]>;

OclBuiltinImpl ext_mask_to_v4_16 = OclBuiltinImpl<__ocl_extend_mask_v4, [v16i32], 0,
  [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
      0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3);
  }]>;

OclBuiltinImpl ext_mask_to_v4_32 = OclBuiltinImpl<__ocl_extend_mask_v4, [v32i32], 0,
  [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,
      3, 3, 3, 3, 3, 3, 3, 3);
  }]>;

OclBuiltinImpl ext_mask_to_v4_64 = OclBuiltinImpl<__ocl_extend_mask_v4, [v64i64], 0,
  [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3);
  }]>;

OclBuiltinImpl ext_mask_to_v8_8 = OclBuiltinImpl<__ocl_extend_mask_v8, [v8i32], 0,
  [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl ext_mask_to_v8_16 = OclBuiltinImpl<__ocl_extend_mask_v8, [v16i32], 0,
  [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
      0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7);
  }]>;

OclBuiltinImpl ext_mask_to_v8_32 = OclBuiltinImpl<__ocl_extend_mask_v8, [v32i32], 0,
  [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
      0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7);
  }]>;

OclBuiltinImpl ext_mask_to_v8_64 = OclBuiltinImpl<__ocl_extend_mask_v8, [v64i32], 0,
  [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,
      3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5,
      6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7);
  }]>;

OclBuiltinImpl ext_mask_to_v8_128 = OclBuiltinImpl<__ocl_extend_mask_v8, [v128i32], 0,
  [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
      7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7);
  }]>;


OclBuiltinImpl ext_mask_to_v16_16 = OclBuiltinImpl<__ocl_extend_mask_v16, [v16i32], 0,
  [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl ext_mask_to_v16_32 = OclBuiltinImpl<__ocl_extend_mask_v16, [v32i32], 0,
  [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
      0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8,
      9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15);
  }]>;

OclBuiltinImpl ext_mask_to_v16_64 = OclBuiltinImpl<__ocl_extend_mask_v16, [v64i32], 0,
  [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
      0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
      6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 10, 11, 11,
      11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15);
  }]>;


OclBuiltinImpl ext_mask_to_v16_128 = OclBuiltinImpl<__ocl_extend_mask_v16, [v128i32], 0,
  [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,
      3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5,
      6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8,
      9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11,
      11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13,
      13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15);
  }]>;


OclBuiltinImpl ext_mask_to_v16_256 = OclBuiltinImpl<__ocl_extend_mask_v16, [v256i32], 0,
  [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
      7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
      8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
      9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
      10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
      11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
      12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
      15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15);
  }]>;
