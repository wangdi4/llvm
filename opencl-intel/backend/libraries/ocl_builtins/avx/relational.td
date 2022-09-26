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

OclBuiltinImpl isfinite_promote       =    OclBuiltinImpl    <isfinite,       [v3f64],                  0, Promote1>;
OclBuiltinImpl isinf_promote          =    OclBuiltinImpl    <isinf,          [v3f64],                  0, Promote1>;
OclBuiltinImpl isnormal_promote       =    OclBuiltinImpl    <isnormal,       [v3f64],                  0, Promote1>;
//OclBuiltinImpl any_promote          =    OclBuiltinImpl    <any,            [v3i64],                  0, Promote1>;
//OclBuiltinImpl all_promote          =    OclBuiltinImpl    <all,            [v3i64],                  0, Promote1>;

//======v8i32\v8u32 and v4i64\v4u64 implementation=============================== 
OclBuiltinImpl any_v8i32 = OclBuiltinImpl<any, [v8i32], 0, [{
    int mask = _mm256_movemask_ps(__builtin_astype($Arg0VarName,__m256));
    return as_$ReturnType(((mask & 0xFF) != 0));
  }]>;

OclBuiltinImpl any_v4i64 = OclBuiltinImpl<any, [v4i64], 0, [{
    int mask = _mm256_movemask_ps(__builtin_astype($Arg0VarName,__m256));
    return as_$ReturnType(((mask & 0xAA) != 0));
  }]>;

OclBuiltinImpl all_v8i32 = OclBuiltinImpl<all, [v8i32], 0, [{
    int mask = _mm256_movemask_ps(__builtin_astype($Arg0VarName,__m256));
    return as_$ReturnType(((mask & 0xFF) == 0xFF));
  }]>;

OclBuiltinImpl all_v4i64 = OclBuiltinImpl<all, [v4i64], 0, [{
    int mask = _mm256_movemask_ps(__builtin_astype($Arg0VarName,__m256));
    return as_$ReturnType(((mask & 0xAA) == 0xAA));
  }]>;
