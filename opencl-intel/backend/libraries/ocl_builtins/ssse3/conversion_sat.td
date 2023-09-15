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


/*****************************************************************\
 * convert_charn_sat:                                            *
\*****************************************************************/
// convert_charn - expand
OclBuiltinImpl ssse3_expand_conv_i8_sat = OclBuiltinImpl<convert_i8_sat, [v8f64] , 0, Expand_default_sat>;
OclBuiltinImpl ssse3_expend_conv_i8_sat_rte = OclBuiltinImpl<convert_i8_sat_rte, [v8f64], 0, Expand_sat_rte>;
OclBuiltinImpl ssse3_expand_conv_i8_sat_rtz = OclBuiltinImpl<convert_i8_sat_rtz, [v8f64], 0, Expand_sat_rtz>;
OclBuiltinImpl ssse3_expand_conv_i8_sat_rtp = OclBuiltinImpl<convert_i8_sat_rtp, [v8f64], 0, Expand_sat_rtp>;
OclBuiltinImpl ssse3_expand_conv_i8_sat_rtn = OclBuiltinImpl<convert_i8_sat_rtn, [v8f64], 0, Expand_sat_rtn>;

// convert_charn - shorts
OclBuiltinImpl ssse3_v16i16_conv_i8_sat = OclBuiltinImpl<convert_i8_sat, [v16i16] , 0, [{
    return as_$ReturnType(_mm_packs_epi16((__m128i)$Arg0VarName.lo, (__m128i)$Arg0VarName.hi));
  }]>;
OclBuiltinImpl ssse3_shorts_conv_i8_sat = OclBuiltinImpl<convert_i8_sat, [v2i16, v3i16, v4i16, v8i16], 0, [{
    short16 X;
    X$Pattern = $Arg0VarName;
    return convert_char16_sat(X)$Pattern;
  }]>;
// convert_charn - ints
OclBuiltinImpl ssse3_ints_conv_i8_sat = OclBuiltinImpl<convert_i8_sat, [v2i32, v3i32, v4i32, v8i32, v16i32], 0, [{
    return convert_char$VecLength_sat( convert_short$VecLength_sat($Arg0VarName) );
  }]>;
/*****************************************************************\
 * convert_ucharn_sat:                                           *
\*****************************************************************/
// convert_ucharn - expand
OclBuiltinImpl ssse3_expand_conv_u8_sat = OclBuiltinImpl<convert_u8_sat, [v8f64], 0, Expand_default_sat>;
OclBuiltinImpl ssse3_expand_conv_u8_sat_rte = OclBuiltinImpl<convert_u8_sat_rte, [v8f64], 0, Expand_sat_rte>;
OclBuiltinImpl ssse3_expand_conv_u8_sat_rtz = OclBuiltinImpl<convert_u8_sat_rtz, [v8f64], 0, Expand_sat_rtz>;
OclBuiltinImpl ssse3_expand_conv_u8_sat_rtp = OclBuiltinImpl<convert_u8_sat_rtp, [v8f64], 0, Expand_sat_rtp>;
OclBuiltinImpl ssse3_expand_conv_u8_sat_rtn = OclBuiltinImpl<convert_u8_sat_rtn, [v8f64], 0, Expand_sat_rtn>;

// convert_ucharn - shorts
OclBuiltinImpl ssse3_v16i16_conv_u8_sat = OclBuiltinImpl<convert_u8_sat, [v16i16] , 0, [{
    return as_$ReturnType(_mm_packus_epi16((__m128i)$Arg0VarName.lo, (__m128i)$Arg0VarName.hi));
  }]>;
OclBuiltinImpl ssse3_shorts_conv_u8_sat = OclBuiltinImpl<convert_u8_sat, [v2i16, v3i16, v4i16, v8i16], 0, [{
    short16 X;
    X$Pattern = $Arg0VarName;
    return convert_uchar16_sat(X)$Pattern;
  }]>;
// convert_ucharn - ints
OclBuiltinImpl ssse3_ints_conv_u8_sat = OclBuiltinImpl<convert_u8_sat, [v2i32, v3i32, v4i32, v8i32, v16i32], 0, [{
    return convert_uchar$VecLength_sat( convert_short$VecLength_sat($Arg0VarName) );
  }]>;

/*****************************************************************\
 * convert_ushortn_sat:                                          *
\*****************************************************************/
// convert_ushortn - floats & doubles
// promote to int
OclBuiltinImpl ssse3_conv_u16_sat = OclBuiltinImpl<convert_u16_sat, [v8i32], 0, [{
    uint$Suffix t = convert_uint$Suffix_sat($Arg0VarName);
    return convert_$ReturnType_sat(t);
  }]>;

/*****************************************************************\
 * convert_intn_sat:                                             *
\*****************************************************************/
// convert intn - expand

OclBuiltinImpl ssse3_expand_conv_i32_sat = OclBuiltinImpl<convert_i32_sat, [v8f64], 0, Expand_default_sat>;
OclBuiltinImpl ssse3_expand_conv_i32_sat_rte = OclBuiltinImpl<convert_i32_sat_rte, [v8f64], 0, Expand_sat_rte>;
OclBuiltinImpl ssse3_expand_conv_i32_sat_rtz = OclBuiltinImpl<convert_i32_sat_rtz, [v8f64], 0, Expand_sat_rtz>;
OclBuiltinImpl ssse3_expand_conv_i32_sat_rtp = OclBuiltinImpl<convert_i32_sat_rtp, [v8f64], 0, Expand_sat_rtp>;
OclBuiltinImpl ssse3_expand_conv_i32_sat_rtn = OclBuiltinImpl<convert_i32_sat_rtn, [v8f64], 0, Expand_sat_rtn>;

/*****************************************************************\
 * convert_uintn_sat:                                            *
\*****************************************************************/
// convert_uintn - expand
OclBuiltinImpl ssse3_expand_conv_u32_sat = OclBuiltinImpl<convert_u32_sat, [v8f64], 0, Expand_default_sat>;
OclBuiltinImpl ssse3_expand_conv_u32_sat_rte = OclBuiltinImpl<convert_u32_sat_rte, [v8f64], 0, Expand_sat_rte>;
OclBuiltinImpl ssse3_expand_conv_u32_sat_rtz = OclBuiltinImpl<convert_u32_sat_rtz, [v8f64], 0, Expand_sat_rtz>;
OclBuiltinImpl ssse3_expand_conv_u32_sat_rtp = OclBuiltinImpl<convert_u32_sat_rtp, [v8f64], 0, Expand_sat_rtp>;
OclBuiltinImpl ssse3_expand_conv_u32_sat_rtn = OclBuiltinImpl<convert_u32_sat_rtn, [v8f64], 0, Expand_sat_rtn>;


/*****************************************************************\
 * convert_longn_sat:                                            *
\*****************************************************************/
// convert_longn - expand
OclBuiltinImpl ssse3_expand_conv_i64_sat = OclBuiltinImpl<convert_i64_sat, [v8f64], 0, Expand_default_sat>;
OclBuiltinImpl ssse3_expand_conv_i64_sat_rte = OclBuiltinImpl<convert_i64_sat_rte, [v8f64], 0, Expand_sat_rte>;
OclBuiltinImpl ssse3_expand_conv_i64_sat_rtz = OclBuiltinImpl<convert_i64_sat_rtz, [v8f64], 0, Expand_sat_rtz>;
OclBuiltinImpl ssse3_expand_conv_i64_sat_rtp = OclBuiltinImpl<convert_i64_sat_rtp, [v8f64], 0, Expand_sat_rtp>;
OclBuiltinImpl ssse3_expand_conv_i64_sat_rtn = OclBuiltinImpl<convert_i64_sat_rtn, [v8f64], 0, Expand_sat_rtn>;

/*****************************************************************\
 * convert_ulongn_sat:                                           *
\*****************************************************************/
// convert_ulongn - expand
OclBuiltinImpl ssse3_expand_conv_u64_sat = OclBuiltinImpl<convert_u64_sat, [v8f64], 0, Expand_default_sat>;
OclBuiltinImpl ssse3_expand_conv_u64_sat_rte = OclBuiltinImpl<convert_u64_sat_rte, [v8f64], 0, Expand_sat_rte>;
OclBuiltinImpl ssse3_expand_conv_u64_sat_rtz = OclBuiltinImpl<convert_u64_sat_rtz, [v8f64], 0, Expand_sat_rtz>;
OclBuiltinImpl ssse3_expand_conv_u64_sat_rtp = OclBuiltinImpl<convert_u64_sat_rtp, [v8f64], 0, Expand_sat_rtp>;
OclBuiltinImpl ssse3_expand_conv_u64_sat_rtn = OclBuiltinImpl<convert_u64_sat_rtn, [v8f64], 0, Expand_sat_rtn>;