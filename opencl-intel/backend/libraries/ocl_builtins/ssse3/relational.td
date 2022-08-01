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

include "ssse3/code_relational.td"

//==============================================================


OclBuiltinImpl isfinite_double3       =  OclBuiltinImpl  <isfinite,  [v3f64],                                                   0, v3_double_1arg>;

OclBuiltinImpl isinf_double3          =  OclBuiltinImpl  <isinf,  [v3f64],                                                      0, v3_double_1arg>;


OclBuiltinImpl isnan_double3          =  OclBuiltinImpl  <isnan,  [v3f64],                                                      0, v3_double_1arg>;
OclBuiltinImpl isnan_expand           =  OclBuiltinImpl  <isnan,  [v8f32, v4f64, v16f32, v8f64],                                0, Expand1>;

OclBuiltinImpl isnormal_double3       =  OclBuiltinImpl  <isnormal,  [v3f64],                                                   0, v3_double_1arg>;


OclBuiltinImpl isordered_double3      =  OclBuiltinImpl  <isordered,  [v3f64],                                                  0, v3_double_2arg>;
OclBuiltinImpl isordered_expand       =  OclBuiltinImpl  <isordered,  [v8f32, v4f64, v16f32, v8f64],                            0, Expand2>;

OclBuiltinImpl isunordered_double3    =  OclBuiltinImpl  <isunordered,  [v3f64],                                                0, v3_double_2arg>;
OclBuiltinImpl isunordered_expand     =  OclBuiltinImpl  <isunordered,  [v8f32, v4f64, v16f32, v8f64],                          0, Expand2>;

OclBuiltinImpl signbit_double3        =  OclBuiltinImpl  <signbit,  [v3f64],                                                    0, v3_double_1arg>;
OclBuiltinImpl signbit_expand         =  OclBuiltinImpl  <signbit,  [v8f32, v4f64, v16f32, v8f64],                              0, Expand1>;

OclBuiltinImpl any_expand2           =  OclBuiltinImpl  <any,  [v8i32, v4i64],  0, Expand_any>;
OclBuiltinImpl all_expand2           =  OclBuiltinImpl  <all,  [v8i32, v4i64],  0, Expand_all>;

 //===============any & all==============

OclBuiltinImpl any_3i64 = OclBuiltinImpl  <any,  [v3i64], 0, [{
    $ReturnType res1, res2;
    res1 = $Func($Arg0VarName.s01);
    res2 = $Func($Arg0VarName.s2);
    return (res1 || res2);
  }]>;
OclBuiltinImpl all_3i64 = OclBuiltinImpl  <all,  [v3i64], 0, [{
    $ReturnType res1, res2;
    res1 = $Func($Arg0VarName.s01);
    res2 = $Func($Arg0VarName.s2);
    return (res1 && res2);
  }]>;
