include "ssse3/code_relational.td"
//==============================================================

OclBuiltinImpl isfinite_promote       =  OclBuiltinImpl  <isfinite,  [v2f32, v3f32],                                            0, Promote1>;

OclBuiltinImpl isinf_promotef2        =  OclBuiltinImpl  <isinf,  [v2f32, v3f32],                                               0, Promote1>;

OclBuiltinImpl isnan_promote          =  OclBuiltinImpl  <isnan,  [v2f32, v3f32],                                               0, Promote1>;

OclBuiltinImpl isnan_expand           =  OclBuiltinImpl  <isnan,  [v16f64],                                                     0, Expand1>;
OclBuiltinImpl isnormal_promote       =  OclBuiltinImpl  <isnormal,  [v2f32, v3f32],                                            0, Promote1>;

OclBuiltinImpl isordered_promote      =  OclBuiltinImpl  <isordered,  [v2f32, v3f32],                                           0, Promote2>;

OclBuiltinImpl isordered_expand       =  OclBuiltinImpl  <isordered,  [v16f64],                                                 0, Expand2>;
OclBuiltinImpl isunordered_promote    =  OclBuiltinImpl  <isunordered,  [v2f32, v3f32],                                         0, Promote2>;

OclBuiltinImpl isunordered_expand     =  OclBuiltinImpl  <isunordered,  [v16f64],                  0, Expand2>;
OclBuiltinImpl signbit_promote        =  OclBuiltinImpl  <signbit,  [v2f32, v3f32],                                             0, Promote1>;

OclBuiltinImpl signbit_expand         =  OclBuiltinImpl  <signbit,  [v16f64],                      0, Expand1>;

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
OclBuiltinImpl bitselect_promote     =  OclBuiltinImpl  <bitselect,  [v2f32, v3f32, v2i32, v2u32, v3i32, v3u32, v2i8, 
                                                                      v2u8, v3i8, v3u8, v4i8, v4u8, v2i16, v2u16, 
                                                                      v3i16, v3u16, v4i16, v4u16, v8i8, v8u8],                           0, Promote3>;
OclBuiltinImpl bitselect_expand      =  OclBuiltinImpl  <bitselect,  [v16f64, v16i64, v16u64],                             0, Expand3>;

OclBuiltinImpl iselect_promote       =  OclBuiltinImpl  <iselect,  [v2f32, v3f32, v2i32, v2u32, v3i32, v3u32, v2i8,
                                                                    v2u8, v3i8, v3u8, v4i8, v4u8, v2i16, v2u16,
                                                                    v3i16, v3u16, v4i16, v4u16, v8i8, v8u8],               0, Promote3>;
OclBuiltinImpl iselect_expand        =  OclBuiltinImpl  <iselect,  [v16f64, v16u16, v16i16, v16i64, v16u64],               0, Expand3>;
OclBuiltinImpl uselect_promote       =  OclBuiltinImpl  <uselect,  [v2f32, v3f32, v2i32, v2u32, v3i32, v3u32, v2i8,
                                                                    v2u8, v3i8, v3u8, v4i8, v4u8, v2i16, v2u16,
                                                                    v3i16, v3u16, v4i16, v4u16, v8i8, v8u8],               0, Promote3>;
OclBuiltinImpl uselect_expand        =  OclBuiltinImpl  <uselect,  [v16f64, v16u16, v16i16, v16i64, v16u64],               0, Expand3>;

OclBuiltinImpl islessgreater1        =   OclBuiltinImpl  <islessgreater,  [v1f32, v1f64,  v2f64, v4f32], 0, [{
    return (($Arg0VarName < $Arg1VarName) | ($Arg0VarName > $Arg1VarName)) ;
  }]>;
OclBuiltinImpl islessgreater_E       =  OclBuiltinImpl  <islessgreater, [ v8f32, v16f64],     0, Expand2>;
OclBuiltinImpl islessgreater_P       =  OclBuiltinImpl  <islessgreater, [v2f32,  v3f32],     0, Promote2>;

OclBuiltinImpl isnan_v4f32_v2f64 = OclBuiltinImpl<isnan, [v4f32, v2f64, v1f32, v1f64], 0, [{
    return as_$ReturnType($Arg0VarName != $Arg0VarName);
  }]>;
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

OclBuiltinImpl isordered_scalar = OclBuiltinImpl<isordered, [v1f32, v1f64], 0, [{
    return ($Arg0VarName == $Arg0VarName && $Arg1VarName == $Arg1VarName);
  }]>;

OclBuiltinImpl isunordered_scalar = OclBuiltinImpl<isunordered, [v1f32, v1f64], 0, [{
    return (($Arg0VarName!=$Arg0VarName) || ($Arg1VarName!=$Arg1VarName));
  }]>;


OclBuiltinImpl signbit_scalarf = OclBuiltinImpl<signbit, [v1f32], 0, [{
    return (*((int*)&$Arg0VarName) & 0x80000000) != 0;
  }]>;

OclBuiltinImpl signbit_scalard = OclBuiltinImpl<signbit, [v1f64], 0, [{
    return (*((long*)&$Arg0VarName) & 0x8000000000000000) != 0;
  }]>;

OclBuiltinImpl any__scalar8 = OclBuiltinImpl<any, [v1i8], 0, any_all_scalar8>;

OclBuiltinImpl any__scalar16 = OclBuiltinImpl<any, [v1i16], 0, any_all_scalar16>;

OclBuiltinImpl any__scalar32 = OclBuiltinImpl<any, [v1i32], 0, any_all_scalar32>;

OclBuiltinImpl any__scalar64 = OclBuiltinImpl<any, [v1i64], 0, any_all_scalar64>;

OclBuiltinImpl all__scalar8 = OclBuiltinImpl<all, [v1i8], 0, any_all_scalar8>;

OclBuiltinImpl all__scalar16 = OclBuiltinImpl<all, [v1i16], 0, any_all_scalar16>;

OclBuiltinImpl all__scalar32 = OclBuiltinImpl<all, [v1i32], 0, any_all_scalar32>;

OclBuiltinImpl all__scalar64 = OclBuiltinImpl<all, [v1i64], 0, any_all_scalar64>;


OclBuiltinImpl bitselect__scalarf = OclBuiltinImpl<bitselect, [v1f64,v1f32], 0, [{
    $Arg0igentype inA = as_$Arg0igentype($Arg0VarName);
    $Arg0igentype inB = as_$Arg0igentype($Arg1VarName);
    $Arg0igentype inC = as_$Arg0igentype($Arg2VarName);

    inB = (inB & inC);
    inC = (~inC & inA);
    inC = (inC | inB);
    return as_$ReturnType(inC);
  }]>;

OclBuiltinImpl bitselect__scalar = OclBuiltinImpl<bitselect, [v1i8, v1i16, v1i32, v1i64, v1u8, v1u16, v1u32, v1u64], 0, [{
    return ($Arg0VarName & ~$Arg2VarName) | ($Arg1VarName & $Arg2VarName);
  }]>;

OclBuiltinImpl iselect_scalar = OclBuiltinImpl<iselect, [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64], 0, select_scalar>;

OclBuiltinImpl uselect_scalar = OclBuiltinImpl<uselect, [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64], 0, select_scalar>;

//======v4f32 and v2f64 implementation===============================

OclBuiltinImpl isfinite_v4f32 = OclBuiltinImpl<isfinite, [v4f32, v8f32, v16f32], 0, [{
    //calculate abs($Arg0VarName)
    //Remove sign bit of the float
    $Arg0VarName = as_$Arg0Type(as_$Arg0igentype($Arg0VarName) & ($Arg0igentype)abs_inf);
    return isless($Arg0VarName, as_$Arg0Type(($Arg0igentype)fnan_min));
  }]>;

OclBuiltinImpl isfinite_v2f64 = OclBuiltinImpl<isfinite, [v2f64, v4f64, v8f64, v16f64], 0, [{
    $Arg0VarName = as_$Arg0Type(as_$Arg0igentype($Arg0VarName) & ($Arg0igentype)dnan_min);
    $Arg0Type dRes = as_$Arg0Type( as_$Arg0igentype($Arg0VarName) != ($Arg0igentype)dnan_min);
    $ReturnType res = as_$ReturnType(dRes);
    return res;
  }]>;

OclBuiltinImpl isinf_v4f32 = OclBuiltinImpl<isinf, [v4f32, v8f32, v16f32], 0, [{
    //calculate abs($Arg0VarName)
    //Remove sign bit of the float
    $Arg0VarName = as_$Arg0Type(as_$Arg0igentype($Arg0VarName) &  ($Arg0igentype)abs_inf);
    return (isgreaterequal($Arg0VarName, (as_$Arg0Type(($Arg0igentype)fnan_min))));
  }]>;

OclBuiltinImpl isinf_v2f64 = OclBuiltinImpl<isinf, [v2f64, v4f64, v8f64, v16f64], 0, [{
    //calculate abs($Arg0VarName)
    //Remove sign bit of the float
    $Arg0VarName = as_$Arg0Type( as_$Arg0igentype($Arg0VarName) & ($Arg0igentype)dnan_max);
    $Arg0Type dRes = as_$Arg0Type( as_$Arg0igentype($Arg0VarName) == ($Arg0igentype)dnan_min);
    return as_$ReturnType(dRes);
  }]>;

OclBuiltinImpl isnormal_v4f32 = OclBuiltinImpl<isnormal, [v4f32, v8f32, v16f32], 0, [{
    $Arg0VarName = as_$Arg0Type(as_$Arg0igentype($Arg0VarName) &  ($Arg0igentype)abs_inf);
    $Arg0igentype res = as_$Arg0igentype(isless($Arg0VarName, as_$Arg0Type(($Arg0igentype)fnan_min))); //$Arg0VarName<INFINITY
    $Arg0igentype res1 = as_$Arg0igentype(isgreaterequal($Arg0VarName, ($Arg0Type)fltm)); //$Arg0VarName >= FLT_MIN 
    return as_$ReturnType(res & res1);
  }]>;

OclBuiltinImpl isnormal_v2f64 = OclBuiltinImpl<isnormal, [v2f64, v4f64, v8f64, v16f64], 0, [{
    $Arg0igentype exp = (as_$Arg0igentype($Arg0VarName) & ($Arg0igentype)dexp_mask);
    $ReturnType NaNorINF = as_$ReturnType(exp == ($Arg0igentype)dexp_mask);
    $ReturnType denom = as_$ReturnType(exp == ($Arg0igentype)0);
    $ReturnType res = (NaNorINF | denom);
    return (res ^ ($Arg0igentype)FF);
  }]>;

OclBuiltinImpl isordered_v4f32 = OclBuiltinImpl<isordered, [v4f32], 0, [{
    $Arg0Type res = _mm_cmpord_ps($Arg0VarName, $Arg1VarName);
    //convert to inetegr
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl isordered_v2f64 = OclBuiltinImpl<isordered, [v2f64], 0, [{
    $Arg0Type res = _mm_cmpord_pd($Arg0VarName, $Arg1VarName);
    //convert to inetegr
    return  as_$ReturnType(res);
  }]>;

OclBuiltinImpl isunordered_v4f32 = OclBuiltinImpl<isunordered, [v4f32], 0, [{
    $Arg0Type res = _mm_cmpunord_ps($Arg0VarName, $Arg1VarName);
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl isunordered_v2f64 = OclBuiltinImpl<isunordered, [v2f64], 0, [{
    $Arg0Type res = _mm_cmpunord_pd($Arg0VarName, $Arg1VarName);
    //convert to inetegr
    return  as_$ReturnType(res);
  }]>;

OclBuiltinImpl signbit_v4f32 = OclBuiltinImpl<signbit, [v4f32], 0, [{
    $Arg0igentype mask = ($Arg0igentype)fsign_mask;
    $Arg0igentype signs = (as_$Arg0igentype($Arg0VarName) & mask);
    return as_$ReturnType (signs == mask);
  }]>;

OclBuiltinImpl signbit_v2f64 = OclBuiltinImpl<signbit, [v2f64], 0, [{
    $Arg0igentype sign = as_$Arg0igentype(as_$Arg0igentype($Arg0VarName) & ($Arg0igentype)dsign_mask);
    __m128i res = __builtin_astype((sign ==  ($Arg0igentype)dsign_mask), __m128i);
    res = _mm_shuffle_epi32(res, 0b11110101);
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl bitselect_v2f = OclBuiltinImpl<bitselect, [v2f64, v4f32], 0, [{
    $ReturnType res1 = as_$ReturnType( ~as_$Arg2igentype($Arg2VarName) & as_$Arg0igentype($Arg0VarName));
    $ReturnType res2 = as_$ReturnType( as_$Arg2igentype($Arg2VarName) & as_$Arg1igentype($Arg1VarName));
    return as_$ReturnType(as_$Arg0igentype(res1) | as_$Arg0igentype(res2));
  }]>;



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

OclBuiltinImpl bitselect_v8i16 = OclBuiltinImpl<bitselect, [v8i16, v8u16, v16i8, v16u8], 0, [{
    $Arg0VarName = as_$Arg0Type((~ $Arg2VarName) &  $Arg0VarName);
    $Arg1VarName = as_$Arg0Type( $Arg2VarName &  $Arg1VarName);
    return as_$ReturnType($Arg0VarName |  $Arg1VarName);
  }]>;


OclBuiltinImpl iselect_v8 = OclBuiltinImpl<iselect, [v8i16, v8u16, v4i32, v4u32], 0, select_vector_v16_8>;

OclBuiltinImpl uselect_v8 = OclBuiltinImpl<uselect, [v8i16, v8u16, v4i32, v4u32], 0, select_vector_v16_8>;

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

OclBuiltinImpl bitselect_v4i32 = OclBuiltinImpl<bitselect, [v4i32, v4u32], 0, [{
    $Arg0igentype res1 = as_$Arg0igentype((~ $Arg2VarName) &  $Arg0VarName);
    $Arg0igentype res2 = as_$Arg0igentype( $Arg2VarName &  $Arg1VarName);
    return as_$ReturnType(res1 | res2);
  }]>;

OclBuiltinImpl bitselect_v2i64 = OclBuiltinImpl<bitselect, [v2i64], 0, [{
    return as_$ReturnType(bitselect(as_int4($Arg0VarName), as_int4($Arg1VarName), as_int4($Arg2VarName)));
  }]>;
OclBuiltinImpl bitselect_v2u64 = OclBuiltinImpl<bitselect, [v2u64], 0, [{
    return as_$ReturnType(bitselect(as_uint4($Arg0VarName), as_uint4($Arg1VarName), as_uint4($Arg2VarName)));
  }]>;

OclBuiltinImpl iselect_v2 = OclBuiltinImpl<iselect, [v2i64, v2u64], 0, select_vector_v2_64>;

OclBuiltinImpl uselect_v2 = OclBuiltinImpl<uselect, [v2i64, v2u64], 0, select_vector_v2_64>;
