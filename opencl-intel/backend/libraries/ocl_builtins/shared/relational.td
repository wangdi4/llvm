// Copyright (C) 2014-2022 Intel Corporation
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

OclBuiltinImpl isfinite_promote       =  OclBuiltinImpl  <isfinite,  [v2f32, v3f32],                                            0, Promote1>;

OclBuiltinImpl isinf_promotef2        =  OclBuiltinImpl  <isinf,  [v2f32, v3f32],                                               0, Promote1>;

OclBuiltinImpl isnormal_promote       =  OclBuiltinImpl  <isnormal,  [v2f32, v3f32],                                            0, Promote1>;

OclBuiltinImpl any_expand1           =  OclBuiltinImpl  <any,  [v3i8, v3i16],                                              0,Expand_any_3i8_3i16 >;
OclBuiltinImpl any_expand2           =  OclBuiltinImpl  <any,  [v2i8, v16i16, v16i32, v2i16, v8i64, v8i64],  0, Expand_any>;
OclBuiltinImpl any_expand3           =  OclBuiltinImpl  <any,  [v4i8],                                                     0, Expand_any_all_char4 >;
OclBuiltinImpl any_expand4           =  OclBuiltinImpl  <any,  [v8i8],                                                     0, Expand_any_all_char8 >;
OclBuiltinImpl any_expand5           =  OclBuiltinImpl  <any,  [v4i16],                                                    0, Expand_4i16 >;
OclBuiltinImpl any_expand6           =  OclBuiltinImpl  <any,  [v2i32],                                                    0, Expand_2i32 >;
OclBuiltinImpl any_expand7           =  OclBuiltinImpl  <any,  [v3i32],                                                    0, Expand_3i32 >;
OclBuiltinImpl all_expand1           =  OclBuiltinImpl  <all,  [v3i8, v3i16],                                              0,Expand_all_3i8_3i16 >;
OclBuiltinImpl all_expand2           =  OclBuiltinImpl  <all,  [v2i8, v16i16, v16i32, v2i16, v8i64, v8i64],  0, Expand_all>;
OclBuiltinImpl all_expand3           =  OclBuiltinImpl  <all,  [v4i8],                                                     0, Expand_any_all_char4>;
OclBuiltinImpl all_expand4           =  OclBuiltinImpl  <all,  [v8i8],                                                     0, Expand_any_all_char8>;
OclBuiltinImpl all_expand5           =  OclBuiltinImpl  <all,  [v4i16],                                                    0,Expand_4i16 >;
OclBuiltinImpl all_expand6           =  OclBuiltinImpl  <all,  [v2i32],                                                    0,Expand_2i32 >;
OclBuiltinImpl all_expand7           =  OclBuiltinImpl  <all,  [v3i32],                                                    0,Expand_3i32 >;

 //===============any & all==============
OclBuiltinImpl any_16i64 = OclBuiltinImpl  < any,  [v16i64], 0,   [{
    $ReturnType res1, res2, res3, res4;
    res1 = $Func($Arg0VarName.lo.lo);
    res2 = $Func($Arg0VarName.lo.hi);
    res3 = $Func($Arg0VarName.hi.lo);
    res4 = $Func($Arg0VarName.hi.hi);
    return res1 || res2 || res3 || res4;
  }]>;
OclBuiltinImpl all_16i64 = OclBuiltinImpl  < all,  [v16i64], 0,  [{
    $ReturnType res1, res2, res3, res4;
    res1 = $Func($Arg0VarName.lo.lo);
    res2 = $Func($Arg0VarName.lo.hi);
    res3 = $Func($Arg0VarName.hi.lo);
    res4 = $Func($Arg0VarName.hi.hi);
    return res1 && res2 && res3 && res4;
  }]>;
OclBuiltinImpl any_3i16 = OclBuiltinImpl  <any,  [v3i16], 0, [{
    $ReturnType res1, res2, res3;
    res1 =  $Func($Arg0VarName.s0);
    res2 =  $Func($Arg0VarName.s1);
    res3 =  $Func($Arg0VarName.s2);
    return res1 || res2 || res3;
  }]>;
OclBuiltinImpl all_3i16 = OclBuiltinImpl  <all,  [v3i16], 0, [{
    $ReturnType res1, res2, res3;
    res1 =  $Func($Arg0VarName.s0);
    res2 =  $Func($Arg0VarName.s1);
    res3 =  $Func($Arg0VarName.s2);
    return res1 && res2 && res3;
  }]>;




 



//======scalar implementation===============================
OclBuiltinImpl isfinite_scalarf = OclBuiltinImpl<isfinite, [v1f32], 0, [{
    $Arg0igentype fX = as_$Arg0igentype($Arg0VarName);
    fX = (fX & abs_inf);
    float res = as_float(fX);
    if (res < INFINITY)
    {
      return 1;
    }
    return 0;
  }]>;

OclBuiltinImpl isfinite_scalard = OclBuiltinImpl<isfinite, [v1f64], 0, [{
    long xAsLong = *((long*)&$Arg0VarName);
    if( ( xAsLong & dnan_min ) != dnan_min )
    {
      return 1;
    }
    return 0;
  }]>;

OclBuiltinImpl isinf_scalarf = OclBuiltinImpl<isinf, [v1f32], 0, [{
    $Arg0igentype fX = as_$Arg0igentype($Arg0VarName);
    fX = (fX & abs_inf);
    float res = as_float(fX);
    if (INFINITY == res)
    {
      return 1;
    }
    return 0;
  }]>;

OclBuiltinImpl isinf_scalard = OclBuiltinImpl<isinf, [v1f64], 0, [{
    long xAsLong = *((long*)&$Arg0VarName);
    if( ( xAsLong & dnan_max ) == dnan_min )
    {
      return 1;
    }
    return 0;
  }]>;



OclBuiltinImpl isnormal_scalarf = OclBuiltinImpl<isnormal, [v1f32], 0, [{
    int iAsIntAbs = (*(int*)&$Arg0VarName) & abs_inf;
    $Arg0Type fAbs = *(($Arg0Type*)&iAsIntAbs);
    if ((fAbs < INFINITY) && (fAbs >= FLT_MIN))
    {
      return 1;
    }
    return 0;
  }]>;

OclBuiltinImpl isnormal_scalard = OclBuiltinImpl<isnormal, [v1f64], 0, [{
    long iAsLong = *((long*)&$Arg0VarName);
    bool NaNorINF = ( iAsLong & dexp_mask ) == dexp_mask;                  //NaN or INF
    bool denom = ( iAsLong & dexp_mask ) == 0;  //denom
    if( NaNorINF || denom )
    {
      return 0;
    }
    return 1;
  }]>;

OclBuiltinImpl any__scalar8 = OclBuiltinImpl<any, [v1i8], 0, any_all_scalar8>;

OclBuiltinImpl any__scalar16 = OclBuiltinImpl<any, [v1i16], 0, any_all_scalar16>;

OclBuiltinImpl any__scalar32 = OclBuiltinImpl<any, [v1i32], 0, any_all_scalar32>;

OclBuiltinImpl any__scalar64 = OclBuiltinImpl<any, [v1i64], 0, any_all_scalar64>;

OclBuiltinImpl all__scalar8 = OclBuiltinImpl<all, [v1i8], 0, any_all_scalar8>;

OclBuiltinImpl all__scalar16 = OclBuiltinImpl<all, [v1i16], 0, any_all_scalar16>;

OclBuiltinImpl all__scalar32 = OclBuiltinImpl<all, [v1i32], 0, any_all_scalar32>;

OclBuiltinImpl all__scalar64 = OclBuiltinImpl<all, [v1i64], 0, any_all_scalar64>;

//======v4f32 and v2f64 implementation===============================

OclBuiltinImpl isfinite_v4f32 = OclBuiltinImpl<isfinite, [v4f32, v8f32, v16f32], 0, isfinite_f32_code>;
OclBuiltinImpl isfinite_v2f64 = OclBuiltinImpl<isfinite, [v2f64, v4f64, v8f64, v16f64], 0, isfinite_f64_code>;

OclBuiltinImpl isinf_v4f32 = OclBuiltinImpl<isinf, [v4f32, v8f32, v16f32], 0, isinf_f32_code>;
OclBuiltinImpl isinf_v2f64 = OclBuiltinImpl<isinf, [v2f64, v4f64, v8f64, v16f64], 0, isinf_f64_code>;

OclBuiltinImpl isnormal_v4f32 = OclBuiltinImpl<isnormal, [v4f32, v8f32, v16f32], 0, isnormal_f32_code>;
OclBuiltinImpl isnormal_v2f64 = OclBuiltinImpl<isnormal, [v2f64, v4f64, v8f64, v16f64], 0, isnormal_f64_code>;

//======v8i16 and v16i8 implementation===============================

OclBuiltinImpl any_v8i16 = OclBuiltinImpl<any, [v8i16], 0, [{
    int mask = _mm_movemask_epi8((__m128i)$Arg0VarName);
    return as_$ReturnType((mask & 0xAAAA) != 0);
  }]>;

OclBuiltinImpl any_v16i8 = OclBuiltinImpl<any, [v16i8], 0, [{
    int mask = _mm_movemask_epi8((__m128i)$Arg0VarName);
    return as_$ReturnType((mask & 0xFFFF) != 0);
  }]>;

OclBuiltinImpl all_v8i16 = OclBuiltinImpl<all, [v8i16], 0, [{
    int mask = _mm_movemask_epi8((__m128i)$Arg0VarName);
    return as_$ReturnType((mask & 0xAAAA) == 0xAAAA);
  }]>;

OclBuiltinImpl all_v16i8 = OclBuiltinImpl<all, [v16i8], 0, [{
    int mask = _mm_movemask_epi8((__m128i)$Arg0VarName);
    return as_$ReturnType(mask == 0xFFFF);
  }]>;

//======v4i32 and v2i64 implementation===============================

OclBuiltinImpl any_v4i32 = OclBuiltinImpl<any, [v4i32], 0, [{
    int mask = _mm_movemask_epi8((__m128i)$Arg0VarName);
    return as_$ReturnType((mask & 0x8888) != 0);
  }]>;

OclBuiltinImpl any_v2i64 = OclBuiltinImpl<any, [v2i64], 0, [{
    int mask = _mm_movemask_epi8((__m128i)$Arg0VarName);
    return as_$ReturnType((mask & 0x8080) != 0);
  }]>;

OclBuiltinImpl all_v4i32 = OclBuiltinImpl<all, [v4i32], 0, [{
    int mask = _mm_movemask_epi8((__m128i)$Arg0VarName);
    return as_$ReturnType((mask & 0x8888) == 0x8888);
  }]>;

OclBuiltinImpl all_v2i64 = OclBuiltinImpl<all, [v2i64], 0, [{
    int mask = _mm_movemask_epi8((__m128i)$Arg0VarName);
    return as_$ReturnType((mask & 0x8080) == 0x8080);
  }]>;
