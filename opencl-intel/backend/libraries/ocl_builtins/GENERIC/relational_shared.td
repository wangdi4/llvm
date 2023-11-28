// Copyright (C) 2006 Intel Corporation
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

// Relational
// OpenCL 1.2 rev 15, Table 6.14

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
