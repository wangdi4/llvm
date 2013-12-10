// vim:ts=2:sw=2:et:
// Copyright (c) 2006-2012 Intel Corporation
// Relational
// OpenCL 1.2 rev 15, Table 6.14

// isnan
OclBuiltinImpl isnan_impl = OclBuiltinImpl<isnan, isnan.Types, 0, [{
    return $Arg0VarName != $Arg0VarName;
  }]>;

// isnormal
OclBuiltinImpl isnormal_scalar = OclBuiltinImpl<isnormal, [v1f32, v1f64], 0, [{
    return ( ((as_$Arg0igentype($Arg0VarName) & double_const_expMask) != 0) && ( (as_$Arg0igentype($Arg0VarName) & double_const_expMask) != double_const_expMask) ) ? 1 : 0;
  }]>;
OclBuiltinImpl isnormal_vector = OclBuiltinImpl<isnormal, [v2f32, v3f32, v4f32, v8f32, v16f32, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return ( ((as_$Arg0igentype($Arg0VarName) & double_const_expMask) != 0) && ( (as_$Arg0igentype($Arg0VarName) & double_const_expMask) != double_const_expMask) ) ? ($Arg0igentype)-1 : ($Arg0igentype)0;
  }]>;

// SOA versions of the any/all relational builtins
// Again, 6 different implementations are required.
// The resulting code looks something like this:
// int4 __attribute__((overloadable)) soa_any(long4 x0, long4 x1, long4 x2)
// {
//   long4 ret;
//   ret = x0;
//   ret = ret | x1;
//   ret = ret | x2;
//   char shift = 64 - 1;
//   ret = as_long4(as_ulong4(ret) >> shift);
//   return convert_int4(ret);
// }
OclBuiltinImpl soa_any1_impl = OclBuiltinImpl<soa_any1, [v4i8,  v8i8,  v16i8,
             v4i16, v8i16, v16i16,
             v4i32, v8i32, v16i32,
             v4i64, v8i64, v16i64], 0, [{
    $Arg0Type $ReturnVarName;
    $ReturnVarName = $Arg0VarName;
    const char shift = $BitWidth - 1;
    $ReturnVarName = as_$Arg0Type(as_$Arg0ugentype($ReturnVarName) >> shift);
    return convert_$ReturnType(as_$Arg0ugentype($ReturnVarName));
  }]>;

OclBuiltinImpl soa_any2_impl = OclBuiltinImpl<soa_any2, [v4i8,  v8i8,  v16i8,
         v4i16, v8i16, v16i16,
         v4i32, v8i32, v16i32,
         v4i64, v8i64, v16i64], 0, [{
    $Arg0Type $ReturnVarName;
    $ReturnVarName = $Arg0VarName;
    $ReturnVarName = $ReturnVarName | $Arg1VarName;
    const char shift = $BitWidth - 1;
    $ReturnVarName = as_$Arg0Type(as_$Arg0ugentype($ReturnVarName) >> shift);
    return convert_$ReturnType(as_$Arg0ugentype($ReturnVarName));
  }]>;

 OclBuiltinImpl soa_any3_impl = OclBuiltinImpl<soa_any3, [v4i8,  v8i8,  v16i8,
         v4i16, v8i16, v16i16,
         v4i32, v8i32, v16i32,
         v4i64, v8i64, v16i64], 0, [{
    $Arg0Type $ReturnVarName;
    $ReturnVarName = $Arg0VarName;
    $ReturnVarName = $ReturnVarName | $Arg1VarName;
    $ReturnVarName = $ReturnVarName | $Arg2VarName;
    const char shift = $BitWidth - 1;
    $ReturnVarName = as_$Arg0Type(as_$Arg0ugentype($ReturnVarName) >> shift);
    return convert_$ReturnType(as_$Arg0ugentype($ReturnVarName));
  }]>;

OclBuiltinImpl soa_any4_impl = OclBuiltinImpl<soa_any4, [v4i8,  v8i8,  v16i8,
         v4i16, v8i16, v16i16,
         v4i32, v8i32, v16i32,
         v4i64, v8i64, v16i64], 0, [{
    $Arg0Type $ReturnVarName;
    $ReturnVarName = $Arg0VarName;
    $ReturnVarName = $ReturnVarName | $Arg1VarName;
    $ReturnVarName = $ReturnVarName | $Arg2VarName;
    $ReturnVarName = $ReturnVarName | $Arg3VarName;
    const char shift = $BitWidth - 1;
    $ReturnVarName = as_$Arg0Type(as_$Arg0ugentype($ReturnVarName) >> shift);
    return convert_$ReturnType(as_$Arg0ugentype($ReturnVarName));
  }]>;

OclBuiltinImpl soa_any8_impl = OclBuiltinImpl<soa_any8, [v4i8,  v8i8,  v16i8,
         v4i16, v8i16, v16i16,
         v4i32, v8i32, v16i32,
         v4i64, v8i64, v16i64], 0, [{
    $Arg0Type $ReturnVarName;
    $ReturnVarName = $Arg0VarName;
    $ReturnVarName = $ReturnVarName | $Arg1VarName;
    $ReturnVarName = $ReturnVarName | $Arg2VarName;
    $ReturnVarName = $ReturnVarName | $Arg3VarName;
    $ReturnVarName = $ReturnVarName | $Arg4VarName;
    $ReturnVarName = $ReturnVarName | $Arg5VarName;
    $ReturnVarName = $ReturnVarName | $Arg6VarName;
    $ReturnVarName = $ReturnVarName | $Arg7VarName;
    const char shift = $BitWidth - 1;
    $ReturnVarName = as_$Arg0Type(as_$Arg0ugentype($ReturnVarName) >> shift);
    return convert_$ReturnType(as_$Arg0ugentype($ReturnVarName));
  }]>;

OclBuiltinImpl soa_any16_impl = OclBuiltinImpl<soa_any16, [v4i8,  v8i8,  v16i8,
         v4i16, v8i16, v16i16,
         v4i32, v8i32, v16i32,
         v4i64, v8i64, v16i64], 0, [{
    $Arg0Type $ReturnVarName;
    $ReturnVarName = $Arg0VarName;
    $ReturnVarName = $ReturnVarName | $Arg1VarName;
    $ReturnVarName = $ReturnVarName | $Arg2VarName;
    $ReturnVarName = $ReturnVarName | $Arg3VarName;
    $ReturnVarName = $ReturnVarName | $Arg4VarName;
    $ReturnVarName = $ReturnVarName | $Arg5VarName;
    $ReturnVarName = $ReturnVarName | $Arg6VarName;
    $ReturnVarName = $ReturnVarName | $Arg7VarName;
    $ReturnVarName = $ReturnVarName | $Arg8VarName;
    $ReturnVarName = $ReturnVarName | $Arg9VarName;
    $ReturnVarName = $ReturnVarName | $Arg10VarName;
    $ReturnVarName = $ReturnVarName | $Arg11VarName;
    $ReturnVarName = $ReturnVarName | $Arg12VarName;
    $ReturnVarName = $ReturnVarName | $Arg13VarName;
    $ReturnVarName = $ReturnVarName | $Arg14VarName;
    $ReturnVarName = $ReturnVarName | $Arg15VarName;
    const char shift = $BitWidth - 1;
    $ReturnVarName = as_$Arg0Type(as_$Arg0ugentype($ReturnVarName) >> shift);
    return convert_$ReturnType(as_$Arg0ugentype($ReturnVarName));
  }]>;

OclBuiltinImpl soa_all1_impl = OclBuiltinImpl<soa_all1, [v4i8,  v8i8,  v16i8,
             v4i16, v8i16, v16i16,
             v4i32, v8i32, v16i32,
             v4i64, v8i64, v16i64], 0, [{
    $Arg0Type $ReturnVarName;
    $ReturnVarName = $Arg0VarName;
    const char shift = $BitWidth - 1;
    $ReturnVarName = as_$Arg0Type(as_$Arg0ugentype($ReturnVarName) >> shift);
    return convert_$ReturnType(as_$Arg0ugentype($ReturnVarName));
  }]>;

OclBuiltinImpl soa_all2_impl = OclBuiltinImpl<soa_all2, [v4i8,  v8i8,  v16i8,
         v4i16, v8i16, v16i16,
         v4i32, v8i32, v16i32,
         v4i64, v8i64, v16i64], 0, [{
    $Arg0Type $ReturnVarName;
    $ReturnVarName = $Arg0VarName;
    $ReturnVarName = $ReturnVarName & $Arg1VarName;
    const char shift = $BitWidth - 1;
    $ReturnVarName = as_$Arg0Type(as_$Arg0ugentype($ReturnVarName) >> shift);
    return convert_$ReturnType(as_$Arg0ugentype($ReturnVarName));
  }]>;
 
 OclBuiltinImpl soa_all3_impl = OclBuiltinImpl<soa_all3, [v4i8,  v8i8,  v16i8,
         v4i16, v8i16, v16i16,
         v4i32, v8i32, v16i32,
         v4i64, v8i64, v16i64], 0, [{
    $Arg0Type $ReturnVarName;
    $ReturnVarName = $Arg0VarName;
    $ReturnVarName = $ReturnVarName & $Arg1VarName;
    $ReturnVarName = $ReturnVarName & $Arg2VarName;
    const char shift = $BitWidth - 1;
    $ReturnVarName = as_$Arg0Type(as_$Arg0ugentype($ReturnVarName) >> shift);
    return convert_$ReturnType(as_$Arg0ugentype($ReturnVarName));
  }]>;

OclBuiltinImpl soa_all4_impl = OclBuiltinImpl<soa_all4, [v4i8,  v8i8,  v16i8,
         v4i16, v8i16, v16i16,
         v4i32, v8i32, v16i32,
         v4i64, v8i64, v16i64], 0, [{
    $Arg0Type $ReturnVarName;
    $ReturnVarName = $Arg0VarName;
    $ReturnVarName = $ReturnVarName & $Arg1VarName;
    $ReturnVarName = $ReturnVarName & $Arg2VarName;
    $ReturnVarName = $ReturnVarName & $Arg3VarName;
    const char shift = $BitWidth - 1;
    $ReturnVarName = as_$Arg0Type(as_$Arg0ugentype($ReturnVarName) >> shift);
    return convert_$ReturnType(as_$Arg0ugentype($ReturnVarName));
  }]>;

OclBuiltinImpl soa_all8_impl = OclBuiltinImpl<soa_all8, [v4i8,  v8i8,  v16i8,
         v4i16, v8i16, v16i16,
         v4i32, v8i32, v16i32,
         v4i64, v8i64, v16i64], 0, [{
    $Arg0Type $ReturnVarName;
    $ReturnVarName = $Arg0VarName;
    $ReturnVarName = $ReturnVarName & $Arg1VarName;
    $ReturnVarName = $ReturnVarName & $Arg2VarName;
    $ReturnVarName = $ReturnVarName & $Arg3VarName;
    $ReturnVarName = $ReturnVarName & $Arg4VarName;
    $ReturnVarName = $ReturnVarName & $Arg5VarName;
    $ReturnVarName = $ReturnVarName & $Arg6VarName;
    $ReturnVarName = $ReturnVarName & $Arg7VarName;
    const char shift = $BitWidth - 1;
    $ReturnVarName = as_$Arg0Type(as_$Arg0ugentype($ReturnVarName) >> shift);
    return convert_$ReturnType(as_$Arg0ugentype($ReturnVarName));
  }]>;

OclBuiltinImpl soa_all16_impl = OclBuiltinImpl<soa_all16, [v4i8,  v8i8,  v16i8,
         v4i16, v8i16, v16i16,
         v4i32, v8i32, v16i32,
         v4i64, v8i64, v16i64], 0, [{
    $Arg0Type $ReturnVarName;
    $ReturnVarName = $Arg0VarName;
    $ReturnVarName = $ReturnVarName & $Arg1VarName;
    $ReturnVarName = $ReturnVarName & $Arg2VarName;
    $ReturnVarName = $ReturnVarName & $Arg3VarName;
    $ReturnVarName = $ReturnVarName & $Arg4VarName;
    $ReturnVarName = $ReturnVarName & $Arg5VarName;
    $ReturnVarName = $ReturnVarName & $Arg6VarName;
    $ReturnVarName = $ReturnVarName & $Arg7VarName;
    $ReturnVarName = $ReturnVarName & $Arg8VarName;
    $ReturnVarName = $ReturnVarName & $Arg9VarName;
    $ReturnVarName = $ReturnVarName & $Arg10VarName;
    $ReturnVarName = $ReturnVarName & $Arg11VarName;
    $ReturnVarName = $ReturnVarName & $Arg12VarName;
    $ReturnVarName = $ReturnVarName & $Arg13VarName;
    $ReturnVarName = $ReturnVarName & $Arg14VarName;
    $ReturnVarName = $ReturnVarName & $Arg15VarName;
    const char shift = $BitWidth - 1;
    $ReturnVarName = as_$Arg0Type(as_$Arg0ugentype($ReturnVarName) >> shift);
    return convert_$ReturnType(as_$Arg0ugentype($ReturnVarName));
  }]>;
