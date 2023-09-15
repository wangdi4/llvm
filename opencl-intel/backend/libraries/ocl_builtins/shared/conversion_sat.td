// Copyright (c) 2006 Intel Corporation
// Conversions (_sat) Built-In Functions
// ssse3

// sat Expand
code Expand_default_sat = [{
    $ReturnType res;
    res$ExpandLoPattern = convert_$ReturnBaseType$LoSuffix_sat($Arg0VarName$ExpandLoPattern);
    res$ExpandHiPattern = convert_$ReturnBaseType$HiSuffix_sat($Arg0VarName$ExpandHiPattern);
    return res;
  }];

code Expand_sat_rte = [{
    $ReturnType res;
    res$ExpandLoPattern = convert_$ReturnBaseType$LoSuffix_sat_rte($Arg0VarName$ExpandLoPattern);
    res$ExpandHiPattern = convert_$ReturnBaseType$HiSuffix_sat_rte($Arg0VarName$ExpandHiPattern);
    return res;
  }];

code Expand_sat_rtz = [{
    $ReturnType res;
    res$ExpandLoPattern = convert_$ReturnBaseType$LoSuffix_sat_rtz($Arg0VarName$ExpandLoPattern);
    res$ExpandHiPattern = convert_$ReturnBaseType$HiSuffix_sat_rtz($Arg0VarName$ExpandHiPattern);
    return res;
  }];

code Expand_sat_rtp = [{
    $ReturnType res;
    res$ExpandLoPattern = convert_$ReturnBaseType$LoSuffix_sat_rtp($Arg0VarName$ExpandLoPattern);
    res$ExpandHiPattern = convert_$ReturnBaseType$HiSuffix_sat_rtp($Arg0VarName$ExpandHiPattern);
    return res;
  }];

code Expand_sat_rtn = [{
    $ReturnType res;
    res$ExpandLoPattern = convert_$ReturnBaseType$LoSuffix_sat_rtn($Arg0VarName$ExpandLoPattern);
    res$ExpandHiPattern = convert_$ReturnBaseType$HiSuffix_sat_rtn($Arg0VarName$ExpandHiPattern);
    return res;
  }];

// sat Promote
code Promote_default_sat = [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnBaseType#4 res = convert_$ReturnBaseType#4_sat(n$Arg0VarName);
    return res$Pattern;
  }];

code Promote_sat_rte = [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnBaseType#4 res = convert_$ReturnBaseType#4_sat_rte(n$Arg0VarName);
    return res$Pattern;
  }];

code Promote_sat_rtz = [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnBaseType#4 res = convert_$ReturnBaseType#4_sat_rtz(n$Arg0VarName);
    return res$Pattern;
  }];

code Promote_sat_rtp = [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnBaseType#4 res = convert_$ReturnBaseType#4_sat_rtp(n$Arg0VarName);
    return res$Pattern;
  }];

code Promote_sat_rtn = [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnBaseType#4 res = convert_$ReturnBaseType#4_sat_rtn(n$Arg0VarName);
    return res$Pattern;
  }];

code fclamp_then_call_nonsat_conv = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    $Arg0VarName = clamp($Arg0VarName_with_NaN, as_float_min_$ReturnBaseType, as_float_max_$ReturnBaseType);
    return convert_$ReturnType( convert_int$Suffix($Arg0VarName) );
  }];
code fclamp_then_call_nonsat_conv_rte = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    $Arg0VarName = clamp($Arg0VarName_with_NaN, as_float_min_$ReturnBaseType, as_float_max_$ReturnBaseType);
    return convert_$ReturnType( convert_int$Suffix_rte($Arg0VarName) );
  }];
code fclamp_then_call_nonsat_conv_rtz = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    $Arg0VarName = clamp($Arg0VarName_with_NaN, as_float_min_$ReturnBaseType, as_float_max_$ReturnBaseType);
    return convert_$ReturnType( convert_int$Suffix_rtz($Arg0VarName) );
  }];
code fclamp_then_call_nonsat_conv_rtp = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    $Arg0VarName = clamp($Arg0VarName_with_NaN, as_float_min_$ReturnBaseType, as_float_max_$ReturnBaseType);
    return convert_$ReturnType( convert_int$Suffix_rtp($Arg0VarName) );
  }];
code fclamp_then_call_nonsat_conv_rtn = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    $Arg0VarName = clamp($Arg0VarName_with_NaN, as_float_min_$ReturnBaseType, as_float_max_$ReturnBaseType);
    return convert_$ReturnType( convert_int$Suffix_rtn($Arg0VarName) );
  }];

list<OclType> conv_sat_ssse3_fp_types    = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32];
list<OclType> conv_sat_ssse3_fp_dp_types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
                                            v1f64, v2f64, v3f64, v4f64, v8f64, v16f64];

/*****************************************************************\
 * convert_charn_sat:                                            *
\*****************************************************************/
// convert_charn - floats
OclBuiltinImpl ssse3_conv_i8_sat = OclBuiltinImpl<convert_i8_sat, conv_sat_ssse3_fp_types, 0, fclamp_then_call_nonsat_conv>;
OclBuiltinImpl ssse3_conv_i8_sat_rte = OclBuiltinImpl<convert_i8_sat_rte, conv_sat_ssse3_fp_types, 0, fclamp_then_call_nonsat_conv_rte>;
OclBuiltinImpl ssse3_conv_i8_sat_rtz = OclBuiltinImpl<convert_i8_sat_rtz, conv_sat_ssse3_fp_types, 0, fclamp_then_call_nonsat_conv_rtz>;
OclBuiltinImpl ssse3_conv_i8_sat_rtp = OclBuiltinImpl<convert_i8_sat_rtp, conv_sat_ssse3_fp_types, 0, fclamp_then_call_nonsat_conv_rtp>;
OclBuiltinImpl ssse3_conv_i8_sat_rtn = OclBuiltinImpl<convert_i8_sat_rtn, conv_sat_ssse3_fp_types, 0, fclamp_then_call_nonsat_conv_rtn>;
// double1 & double4 are inherited from generic which call SVML
// convert_charn - expand
list<OclType> conv_char_ssse3_Expand_types = [v16f64];
OclBuiltinImpl ssse3_expand_conv_i8_sat = OclBuiltinImpl<convert_i8_sat, conv_char_ssse3_Expand_types , 0, Expand_default_sat>;
OclBuiltinImpl ssse3_expend_conv_i8_sat_rte = OclBuiltinImpl<convert_i8_sat_rte, conv_char_ssse3_Expand_types, 0, Expand_sat_rte>;
OclBuiltinImpl ssse3_expand_conv_i8_sat_rtz = OclBuiltinImpl<convert_i8_sat_rtz, conv_char_ssse3_Expand_types, 0, Expand_sat_rtz>;
OclBuiltinImpl ssse3_expand_conv_i8_sat_rtp = OclBuiltinImpl<convert_i8_sat_rtp, conv_char_ssse3_Expand_types, 0, Expand_sat_rtp>;
OclBuiltinImpl ssse3_expand_conv_i8_sat_rtn = OclBuiltinImpl<convert_i8_sat_rtn, conv_char_ssse3_Expand_types, 0, Expand_sat_rtn>;
// convert_charn - promote
list<OclType> conv_char_ssse3_Promote_types =  [v2f64, v3f64];
OclBuiltinImpl ssse3_promote_conv_i8_sat = OclBuiltinImpl<convert_i8_sat, conv_char_ssse3_Promote_types , 0, Promote_default_sat>;
OclBuiltinImpl ssse3_promote_conv_i8_sat_rte = OclBuiltinImpl<convert_i8_sat_rte, conv_char_ssse3_Promote_types, 0, Promote_sat_rte>;
OclBuiltinImpl ssse3_promote_conv_i8_sat_rtz = OclBuiltinImpl<convert_i8_sat_rtz, conv_char_ssse3_Promote_types, 0, Promote_sat_rtz>;
OclBuiltinImpl ssse3_promote_conv_i8_sat_rtp = OclBuiltinImpl<convert_i8_sat_rtp, conv_char_ssse3_Promote_types, 0, Promote_sat_rtp>;
OclBuiltinImpl ssse3_promote_conv_i8_sat_rtn = OclBuiltinImpl<convert_i8_sat_rtn, conv_char_ssse3_Promote_types, 0, Promote_sat_rtn>;




/*****************************************************************\
 * convert_ucharn_sat:                                           *
\*****************************************************************/
// convert_ucharn - floats
// double1 & double4 are inherited from generic which call SVML
OclBuiltinImpl ssse3_conv_u8_sat = OclBuiltinImpl<convert_u8_sat, conv_sat_ssse3_fp_types, 0, fclamp_then_call_nonsat_conv>;
OclBuiltinImpl ssse3_conv_u8_sat_rte = OclBuiltinImpl<convert_u8_sat_rte, conv_sat_ssse3_fp_types, 0, fclamp_then_call_nonsat_conv_rte>;
OclBuiltinImpl ssse3_conv_u8_sat_rtz = OclBuiltinImpl<convert_u8_sat_rtz, conv_sat_ssse3_fp_types, 0, fclamp_then_call_nonsat_conv_rtz>;
OclBuiltinImpl ssse3_conv_u8_sat_rtp = OclBuiltinImpl<convert_u8_sat_rtp, conv_sat_ssse3_fp_types, 0, fclamp_then_call_nonsat_conv_rtp>;
OclBuiltinImpl ssse3_conv_u8_sat_rtn = OclBuiltinImpl<convert_u8_sat_rtn, conv_sat_ssse3_fp_types, 0, fclamp_then_call_nonsat_conv_rtn>;
// convert_ucharn - expand
OclBuiltinImpl ssse3_expand_conv_u8_sat = OclBuiltinImpl<convert_u8_sat, conv_char_ssse3_Expand_types, 0, Expand_default_sat>;
OclBuiltinImpl ssse3_expand_conv_u8_sat_rte = OclBuiltinImpl<convert_u8_sat_rte, conv_char_ssse3_Expand_types, 0, Expand_sat_rte>;
OclBuiltinImpl ssse3_expand_conv_u8_sat_rtz = OclBuiltinImpl<convert_u8_sat_rtz, conv_char_ssse3_Expand_types, 0, Expand_sat_rtz>;
OclBuiltinImpl ssse3_expand_conv_u8_sat_rtp = OclBuiltinImpl<convert_u8_sat_rtp, conv_char_ssse3_Expand_types, 0, Expand_sat_rtp>;
OclBuiltinImpl ssse3_expand_conv_u8_sat_rtn = OclBuiltinImpl<convert_u8_sat_rtn, conv_char_ssse3_Expand_types, 0, Expand_sat_rtn>;
// convert_ucharn - promote
OclBuiltinImpl ssse3_promote_conv_u8_sat = OclBuiltinImpl<convert_u8_sat, conv_char_ssse3_Promote_types, 0, Promote_default_sat>;
OclBuiltinImpl ssse3_promote_conv_u8_sat_rte = OclBuiltinImpl<convert_u8_sat_rte, conv_char_ssse3_Promote_types, 0, Promote_sat_rte>;
OclBuiltinImpl ssse3_promote_conv_u8_sat_rtz = OclBuiltinImpl<convert_u8_sat_rtz, conv_char_ssse3_Promote_types, 0, Promote_sat_rtz>;
OclBuiltinImpl ssse3_promote_conv_u8_sat_rtp = OclBuiltinImpl<convert_u8_sat_rtp, conv_char_ssse3_Promote_types, 0, Promote_sat_rtp>;
OclBuiltinImpl ssse3_promote_conv_u8_sat_rtn = OclBuiltinImpl<convert_u8_sat_rtn, conv_char_ssse3_Promote_types, 0, Promote_sat_rtn>;



/*****************************************************************\
 * convert_shortn_sat:                                           *
\*****************************************************************/
// convert_shortn - floats & doubles
// promote to int
OclBuiltinImpl ssse3_conv_i16_sat = OclBuiltinImpl<convert_i16_sat, conv_sat_ssse3_fp_dp_types, 0, [{
    int$Suffix t = convert_int$Suffix_sat($Arg0VarName);
    return convert_$ReturnType_sat(t);
  }]>;
OclBuiltinImpl ssse3_conv_i16_sat_rte = OclBuiltinImpl<convert_i16_sat_rte, conv_sat_ssse3_fp_dp_types, 0, [{
    int$Suffix t = convert_int$Suffix_sat_rte($Arg0VarName);
    return convert_$ReturnType_sat(t);
  }]>;
OclBuiltinImpl ssse3_conv_i16_sat_rtz = OclBuiltinImpl<convert_i16_sat_rtz, conv_sat_ssse3_fp_dp_types, 0, [{
    int$Suffix t = convert_int$Suffix_sat_rtz($Arg0VarName);
    return convert_$ReturnType_sat(t);
  }]>;
OclBuiltinImpl ssse3_conv_i16_sat_rtp = OclBuiltinImpl<convert_i16_sat_rtp, conv_sat_ssse3_fp_dp_types, 0, [{
    int$Suffix t = convert_int$Suffix_sat_rtp($Arg0VarName);
    return convert_$ReturnType_sat(t);
  }]>;
OclBuiltinImpl ssse3_conv_i16_sat_rtn = OclBuiltinImpl<convert_i16_sat_rtn, conv_sat_ssse3_fp_dp_types, 0, [{
    int$Suffix t = convert_int$Suffix_sat_rtn($Arg0VarName);
    return convert_$ReturnType_sat(t);
  }]>;

// convert_shortn - ints
OclBuiltinImpl ssse3_v8i32_conv_i16_sat = OclBuiltinImpl<convert_i16_sat, [v8i32] , 0, [{
    return as_$ReturnType(_mm_packs_epi32((__m128i)$Arg0VarName.lo, (__m128i)$Arg0VarName.hi));
  }]>;
OclBuiltinImpl ssse3_v8i32_promote_conv_i16_sat = OclBuiltinImpl<convert_i16_sat, [v2i32, v3i32, v4i32], 0, [{
    int8 X;
    X$Pattern = $Arg0VarName;
    return convert_short8_sat(X)$Pattern;
  }]>;
OclBuiltinImpl ssse3_v8i32_expand_conv_i16_sat = OclBuiltinImpl<convert_i16_sat, [v16i32], 0, Expand_default_sat>;


/*****************************************************************\
 * convert_ushortn_sat:                                          *
\*****************************************************************/
// convert_ushortn - floats & doubles
// promote to int
OclBuiltinImpl ssse3_conv_u16_sat = OclBuiltinImpl<convert_u16_sat, conv_sat_ssse3_fp_dp_types, 0, [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    uint$Suffix t = convert_uint$Suffix_sat($Arg0VarName_with_NaN);
    return convert_$ReturnType_sat(t);
  }]>;
OclBuiltinImpl ssse3_conv_u16_sat_rte = OclBuiltinImpl<convert_u16_sat_rte, conv_sat_ssse3_fp_dp_types, 0, [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    uint$Suffix t = convert_uint$Suffix_sat_rte($Arg0VarName_with_NaN);
    return convert_$ReturnType_sat(t);
  }]>;
OclBuiltinImpl ssse3_conv_u16_sat_rtz = OclBuiltinImpl<convert_u16_sat_rtz, conv_sat_ssse3_fp_dp_types, 0, [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    uint$Suffix t = convert_uint$Suffix_sat_rtz($Arg0VarName_with_NaN);
    return convert_$ReturnType_sat(t);
  }]>;
OclBuiltinImpl ssse3_conv_u16_sat_rtp = OclBuiltinImpl<convert_u16_sat_rtp, conv_sat_ssse3_fp_dp_types, 0, [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    uint$Suffix t = convert_uint$Suffix_sat_rtp($Arg0VarName_with_NaN);
    return convert_$ReturnType_sat(t);
  }]>;
OclBuiltinImpl ssse3_conv_u16_sat_rtn = OclBuiltinImpl<convert_u16_sat_rtn, conv_sat_ssse3_fp_dp_types, 0, [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    uint$Suffix t = convert_uint$Suffix_sat_rtn($Arg0VarName_with_NaN);
    return convert_$ReturnType_sat(t);
  }]>;


OclBuiltinImpl ssse3_v8i32_promote_conv_u16_sat = OclBuiltinImpl<convert_u16_sat, [v2i32, v3i32, v4i32], 0, [{
    int8 X;
    X$Pattern = $Arg0VarName;
    return convert_ushort8_sat(X)$Pattern;
  }]>;
OclBuiltinImpl ssse3_v8i32_expand_conv_u16_sat = OclBuiltinImpl<convert_u16_sat, [v16i32], 0, Expand_default_sat>;


/*****************************************************************\
 * convert_intn_sat:                                             *
\*****************************************************************/
// convert intn - floats
code ssse3_convert_float_to_int_sat = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    $ReturnType outRangePos = $Arg0VarName_with_NaN >= as_float(maxInt32);
    $ReturnType outRangeNeg = as_float(minInt32) >= $Arg0VarName_with_NaN;
    $ReturnType inRange = ~(outRangeNeg | outRangePos) & convert_$ReturnType($Arg0VarName_with_NaN);
    outRangePos = outRangePos & generic_max_int;
    outRangeNeg = outRangeNeg & generic_min_int;
    return inRange | outRangePos | outRangeNeg;
  }];
OclBuiltinImpl ssse3_conv_i32_sat = OclBuiltinImpl<convert_i32_sat, [v4f32], 0, ssse3_convert_float_to_int_sat>;
OclBuiltinImpl ssse3_conv_i32_sat_rtz = OclBuiltinImpl<convert_i32_sat_rtz, [v4f32], 0, ssse3_convert_float_to_int_sat>;
// convert intn - expand
list<OclType> conv_int_ssse3_Expand_types = [v16f32, v16f64];
OclBuiltinImpl ssse3_expand_conv_i32_sat = OclBuiltinImpl<convert_i32_sat, [v8f32, v16f32, v16f64], 0, Expand_default_sat>;
OclBuiltinImpl ssse3_expand_conv_i32_sat_rte = OclBuiltinImpl<convert_i32_sat_rte, conv_int_ssse3_Expand_types, 0, Expand_sat_rte>;
OclBuiltinImpl ssse3_expand_conv_i32_sat_rtz = OclBuiltinImpl<convert_i32_sat_rtz, [v8f32, v16f32, v16f64], 0, Expand_sat_rtz>;
OclBuiltinImpl ssse3_expand_conv_i32_sat_rtp = OclBuiltinImpl<convert_i32_sat_rtp, conv_int_ssse3_Expand_types, 0, Expand_sat_rtp>;
OclBuiltinImpl ssse3_expand_conv_i32_sat_rtn = OclBuiltinImpl<convert_i32_sat_rtn, conv_int_ssse3_Expand_types, 0, Expand_sat_rtn>;
// convert intn - promote
list<OclType> conv_int_ssse3_Promote_types = [v2f32, v3f32, v2f64, v3f64];
OclBuiltinImpl ssse3_promote_conv_i32_sat = OclBuiltinImpl<convert_i32_sat, [v1f32, v2f32, v3f32, v2f64, v3f64], 0, Promote_default_sat>;
OclBuiltinImpl ssse3_promote_conv_i32_sat_rte = OclBuiltinImpl<convert_i32_sat_rte, conv_int_ssse3_Promote_types, 0, Promote_sat_rte>;
OclBuiltinImpl ssse3_promote_conv_i32_sat_rtz = OclBuiltinImpl<convert_i32_sat_rtz, [v1f32, v2f32, v3f32, v8f32, v16f32, v2f64, v3f64], 0, Promote_sat_rtz>;
OclBuiltinImpl ssse3_promote_conv_i32_sat_rtp = OclBuiltinImpl<convert_i32_sat_rtp, conv_int_ssse3_Promote_types, 0, Promote_sat_rtp>;
OclBuiltinImpl ssse3_promote_conv_i32_sat_rtn = OclBuiltinImpl<convert_i32_sat_rtn, conv_int_ssse3_Promote_types, 0, Promote_sat_rtn>;


/*****************************************************************\
 * convert_uintn_sat:                                            *
\*****************************************************************/
// convert uintn - float scalar
code ssse3_convert_float_to_uint_sat = [{
    const float maxUintAsFloat = 4294967040.0f;
    return ($Arg0VarName < 0.0f) ? 0 : (($Arg0VarName > maxUintAsFloat) ? 4294967295 : (uint) $Arg0VarName);
  }];
OclBuiltinImpl ssse3_conv_u32_sat = OclBuiltinImpl<convert_u32_sat, [v1f32], 0, ssse3_convert_float_to_uint_sat>;
OclBuiltinImpl ssse3_conv_u32_sat_rtz = OclBuiltinImpl<convert_u32_sat_rtz, [v1f32], 0, ssse3_convert_float_to_uint_sat>;

// convert_uintn - expand
OclBuiltinImpl ssse3_expand_conv_u32_sat = OclBuiltinImpl<convert_u32_sat, [v2f32, v3f32, v4f32, v8f32, v16f32, 
                                                                            v16f64], 0, Expand_default_sat>;
OclBuiltinImpl ssse3_expand_conv_u32_sat_rte = OclBuiltinImpl<convert_u32_sat_rte, conv_int_ssse3_Expand_types, 0, Expand_sat_rte>;
OclBuiltinImpl ssse3_expand_conv_u32_sat_rtz = OclBuiltinImpl<convert_u32_sat_rtz, [v2f32, v3f32, v4f32, v8f32, v16f32, 
                                                                                    v16f64], 0, Expand_sat_rtz>;
OclBuiltinImpl ssse3_expand_conv_u32_sat_rtp = OclBuiltinImpl<convert_u32_sat_rtp, conv_int_ssse3_Expand_types, 0, Expand_sat_rtp>;
OclBuiltinImpl ssse3_expand_conv_u32_sat_rtn = OclBuiltinImpl<convert_u32_sat_rtn, conv_int_ssse3_Expand_types, 0, Expand_sat_rtn>;

// convert_uintn - promote
OclBuiltinImpl ssse3_promote_conv_u32_sat = OclBuiltinImpl<convert_u32_sat, [v2f64, v3f64], 0, Promote_default_sat>;
OclBuiltinImpl ssse3_promote_conv_u32_sat_rte = OclBuiltinImpl<convert_u32_sat_rte, conv_int_ssse3_Promote_types, 0, Promote_sat_rte>;
OclBuiltinImpl ssse3_promote_conv_u32_sat_rtz = OclBuiltinImpl<convert_u32_sat_rtz, [v2f64, v3f64], 0, Promote_sat_rtz>;
OclBuiltinImpl ssse3_promote_conv_u32_sat_rtp = OclBuiltinImpl<convert_u32_sat_rtp, conv_int_ssse3_Promote_types, 0, Promote_sat_rtp>;
OclBuiltinImpl ssse3_promote_conv_u32_sat_rtn = OclBuiltinImpl<convert_u32_sat_rtn, conv_int_ssse3_Promote_types, 0, Promote_sat_rtn>;


/*****************************************************************\
 * convert_longn_sat:                                            *
\*****************************************************************/
// convert_longn - expand
list<OclType> conv_long_ssse3_Expand_types = [v16f32, v16f64];
OclBuiltinImpl ssse3_expand_conv_i64_sat = OclBuiltinImpl<convert_i64_sat, conv_long_ssse3_Expand_types, 0, Expand_default_sat>;
OclBuiltinImpl ssse3_expand_conv_i64_sat_rte = OclBuiltinImpl<convert_i64_sat_rte, conv_long_ssse3_Expand_types, 0, Expand_sat_rte>;
OclBuiltinImpl ssse3_expand_conv_i64_sat_rtz = OclBuiltinImpl<convert_i64_sat_rtz, conv_long_ssse3_Expand_types, 0, Expand_sat_rtz>;
OclBuiltinImpl ssse3_expand_conv_i64_sat_rtp = OclBuiltinImpl<convert_i64_sat_rtp, conv_long_ssse3_Expand_types, 0, Expand_sat_rtp>;
OclBuiltinImpl ssse3_expand_conv_i64_sat_rtn = OclBuiltinImpl<convert_i64_sat_rtn, conv_long_ssse3_Expand_types, 0, Expand_sat_rtn>;
// convert_longn - promote
list<OclType> conv_long_ssse3_Promote_types = [v2f32, v3f32, v2f64, v3f64];
OclBuiltinImpl ssse3_promote_conv_i64_sat = OclBuiltinImpl<convert_i64_sat, conv_long_ssse3_Promote_types, 0, Promote_default_sat>;
OclBuiltinImpl ssse3_promote_conv_i64_sat_rte = OclBuiltinImpl<convert_i64_sat_rte, conv_long_ssse3_Promote_types, 0, Promote_sat_rte>;
OclBuiltinImpl ssse3_promote_conv_i64_sat_rtz = OclBuiltinImpl<convert_i64_sat_rtz, conv_long_ssse3_Promote_types, 0, Promote_sat_rtz>;
OclBuiltinImpl ssse3_promote_conv_i64_sat_rtp = OclBuiltinImpl<convert_i64_sat_rtp, conv_long_ssse3_Promote_types, 0, Promote_sat_rtp>;
OclBuiltinImpl ssse3_promote_conv_i64_sat_rtn = OclBuiltinImpl<convert_i64_sat_rtn, conv_long_ssse3_Promote_types, 0, Promote_sat_rtn>;


/*****************************************************************\
 * convert_ulongn_sat:                                           *
\*****************************************************************/
// convert_ulongn - expand
OclBuiltinImpl ssse3_expand_conv_u64_sat = OclBuiltinImpl<convert_u64_sat, conv_long_ssse3_Expand_types, 0, Expand_default_sat>;
OclBuiltinImpl ssse3_expand_conv_u64_sat_rte = OclBuiltinImpl<convert_u64_sat_rte, conv_long_ssse3_Expand_types, 0, Expand_sat_rte>;
OclBuiltinImpl ssse3_expand_conv_u64_sat_rtz = OclBuiltinImpl<convert_u64_sat_rtz, conv_long_ssse3_Expand_types, 0, Expand_sat_rtz>;
OclBuiltinImpl ssse3_expand_conv_u64_sat_rtp = OclBuiltinImpl<convert_u64_sat_rtp, conv_long_ssse3_Expand_types, 0, Expand_sat_rtp>;
OclBuiltinImpl ssse3_expand_conv_u64_sat_rtn = OclBuiltinImpl<convert_u64_sat_rtn, conv_long_ssse3_Expand_types, 0, Expand_sat_rtn>;
// convert_ulongn - promote
OclBuiltinImpl ssse3_promote_conv_u64_sat = OclBuiltinImpl<convert_u64_sat, conv_long_ssse3_Promote_types, 0, Promote_default_sat>;
OclBuiltinImpl ssse3_promote_conv_u64_sat_rte = OclBuiltinImpl<convert_u64_sat_rte, conv_long_ssse3_Promote_types, 0, Promote_sat_rte>;
OclBuiltinImpl ssse3_promote_conv_u64_sat_rtz = OclBuiltinImpl<convert_u64_sat_rtz, conv_long_ssse3_Promote_types, 0, Promote_sat_rtz>;
OclBuiltinImpl ssse3_promote_conv_u64_sat_rtp = OclBuiltinImpl<convert_u64_sat_rtp, conv_long_ssse3_Promote_types, 0, Promote_sat_rtp>;
OclBuiltinImpl ssse3_promote_conv_u64_sat_rtn = OclBuiltinImpl<convert_u64_sat_rtn, conv_long_ssse3_Promote_types, 0, Promote_sat_rtn>;
