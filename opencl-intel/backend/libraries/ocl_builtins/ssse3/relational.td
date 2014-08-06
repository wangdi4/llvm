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

OclBuiltinImpl bitselect_expand      =  OclBuiltinImpl  <bitselect,  [v8f32, v3f64, v4f64, v8i32, v3i64,
                                                                      v8u32, v3u64, v16i16, v16u16,
                                                                      v4u64, v4i64, v8u64, v16u32, v8f64,
                                                                      v16i32, v8i64, v16f32  ],                          0, Expand3>;

OclBuiltinImpl iselect_expand        =  OclBuiltinImpl  <iselect,  [v8f32, v4f64, v8i32, v8u32, v4u64, v4i64, v16f32,
                                                                    v8f64, v8i64, v16u32, v16i32, v8u64],                0, Expand3>;
OclBuiltinImpl uselect_expand        =  OclBuiltinImpl  <uselect,  [v8f32, v4f64, v8u32, v8i32, v4u64, v4i64, v16f32,
                                                                    v8f64, v8i64, v16u32, v16i32, v8u64],                0, Expand3>;

OclBuiltinImpl islessgreater_E       =  OclBuiltinImpl  <islessgreater, [v4f64, v8f64, v16f32],     0, Expand2>;

OclBuiltinImpl islessgreater_3f64    =  OclBuiltinImpl  <islessgreater, [v3f64],     0, v3_double_2arg>;

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

//==============select===============

OclBuiltinImpl uselect_3u64       =  OclBuiltinImpl  <uselect,  [v3u64],                              0, [{
    ulong4 tmpX, tmpY, res, tmpZ;
    tmpX.s012 = $Arg0VarName;
    tmpY.s012 = $Arg1VarName;
    tmpZ.s012 = $Arg2VarName;
    res = $Func(tmpX, tmpY, tmpZ);
    return res.s012;
  }]>;
OclBuiltinImpl iselect_3u64       =  OclBuiltinImpl  <iselect,  [v3u64],                              0, [{
    ulong4 tmpX, tmpY, res;
    long4 tmpZ;
    tmpX.s012 = $Arg0VarName;
    tmpY.s012 = $Arg1VarName;
    tmpZ.s012 = $Arg2VarName;
    res = $Func(tmpX, tmpY, tmpZ);
    return res.s012;
  }]>;
OclBuiltinImpl uselect_3i64       =  OclBuiltinImpl  <uselect,  [v3i64],                              0, [{
    long4 tmpX, tmpY, res;
    ulong4 tmpZ;
    tmpX.s012 = $Arg0VarName;
    tmpY.s012 = $Arg1VarName;
    tmpZ.s012 = $Arg2VarName;
    res = $Func(tmpX, tmpY, tmpZ);
    return res.s012;
  }]>;
OclBuiltinImpl iselect_3i64       =  OclBuiltinImpl  <iselect,  [v3i64],                              0, [{
    long4 tmpX, tmpY, res, tmpZ;
    tmpX.s012 = $Arg0VarName;
    tmpY.s012 = $Arg1VarName;
    tmpZ.s012 = $Arg2VarName;
    res = $Func(tmpX, tmpY, tmpZ);
    return res.s012;
  }]>;
OclBuiltinImpl uselect_3f64       =  OclBuiltinImpl  <uselect,  [v3f64],                              0, select_double3>;
OclBuiltinImpl iselect_3f64       =  OclBuiltinImpl  <iselect,  [v3f64],                              0, select_double3>;
  




//======v4f32 and v2f64 implementation===============================



OclBuiltinImpl iselect_v4f32 = OclBuiltinImpl<iselect, [v4f32], 0, select_vector_v4f32>;

OclBuiltinImpl iselect_v2f64 = OclBuiltinImpl<iselect, [v2f64], 0, select_vector_v2f64>;

OclBuiltinImpl uselect_v4f32 = OclBuiltinImpl<uselect, [v4f32], 0, select_vector_v4f32>;

OclBuiltinImpl uselect_v2f64 = OclBuiltinImpl<uselect, [v2f64], 0, select_vector_v2f64>;

//======v8i16 and v16i8 implementation===============================



OclBuiltinImpl iselect_v8 = OclBuiltinImpl<iselect, [v16i8, v16u8], 0, select_vector_v16_8>;

OclBuiltinImpl uselect_v8 = OclBuiltinImpl<uselect, [v16i8, v16u8], 0, select_vector_v16_8>;
