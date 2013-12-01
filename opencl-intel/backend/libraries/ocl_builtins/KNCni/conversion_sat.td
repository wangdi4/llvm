// vim:ts=2:sw=2:et:

// Conversions with saturation optimized for MIC

// convert types narrower than 32 bits
code narrow_integers_code = [{
    $Returnui32gentype v32b = convert_$Returnui32gentype_sat_$Rounding($Arg0VarName);
    return convert_$ReturnType_sat_$Rounding(v32b);
  }];

OclBuiltinImpl cvt_i8_sat = OclBuiltinImpl<convert_i8_sat, float_types, 0, narrow_integers_code, ["$Rounding"->"rtz"]>;
OclBuiltinImpl cvt_i8_sat_rtz = OclBuiltinImpl<convert_i8_sat_rtz, float_types, 0, narrow_integers_code, ["$Rounding"->"rtz"]>;
OclBuiltinImpl cvt_i8_sat_rte = OclBuiltinImpl<convert_i8_sat_rte, float_types, 0, narrow_integers_code, ["$Rounding"->"rte"]>;
OclBuiltinImpl cvt_i8_sat_rtp = OclBuiltinImpl<convert_i8_sat_rtp, float_types, 0, narrow_integers_code, ["$Rounding"->"rtp"]>;
OclBuiltinImpl cvt_i8_sat_rtn = OclBuiltinImpl<convert_i8_sat_rtn, float_types, 0, narrow_integers_code, ["$Rounding"->"rtn"]>;

OclBuiltinImpl cvt_u8_sat = OclBuiltinImpl<convert_u8_sat, float_types, 0, narrow_integers_code, ["$Rounding"->"rtz"]>;
OclBuiltinImpl cvt_u8_sat_rtz = OclBuiltinImpl<convert_u8_sat_rtz, float_types, 0, narrow_integers_code, ["$Rounding"->"rtz"]>;
OclBuiltinImpl cvt_u8_sat_rte = OclBuiltinImpl<convert_u8_sat_rte, float_types, 0, narrow_integers_code, ["$Rounding"->"rte"]>;
OclBuiltinImpl cvt_u8_sat_rtp = OclBuiltinImpl<convert_u8_sat_rtp, float_types, 0, narrow_integers_code, ["$Rounding"->"rtp"]>;
OclBuiltinImpl cvt_u8_sat_rtn = OclBuiltinImpl<convert_u8_sat_rtn, float_types, 0, narrow_integers_code, ["$Rounding"->"rtn"]>;

OclBuiltinImpl cvt_i16_sat = OclBuiltinImpl<convert_i16_sat, float_types, 0, narrow_integers_code, ["$Rounding"->"rtz"]>;
OclBuiltinImpl cvt_i16_sat_rtz = OclBuiltinImpl<convert_i16_sat_rtz, float_types, 0, narrow_integers_code, ["$Rounding"->"rtz"]>;
OclBuiltinImpl cvt_i16_sat_rte = OclBuiltinImpl<convert_i16_sat_rte, float_types, 0, narrow_integers_code, ["$Rounding"->"rte"]>;
OclBuiltinImpl cvt_i16_sat_rtp = OclBuiltinImpl<convert_i16_sat_rtp, float_types, 0, narrow_integers_code, ["$Rounding"->"rtp"]>;
OclBuiltinImpl cvt_i16_sat_rtn = OclBuiltinImpl<convert_i16_sat_rtn, float_types, 0, narrow_integers_code, ["$Rounding"->"rtn"]>;

OclBuiltinImpl cvt_u16_sat = OclBuiltinImpl<convert_u16_sat, float_types, 0, narrow_integers_code, ["$Rounding"->"rtz"]>;
OclBuiltinImpl cvt_u16_sat_rtz = OclBuiltinImpl<convert_u16_sat_rtz, float_types, 0, narrow_integers_code, ["$Rounding"->"rtz"]>;
OclBuiltinImpl cvt_u16_sat_rte = OclBuiltinImpl<convert_u16_sat_rte, float_types, 0, narrow_integers_code, ["$Rounding"->"rte"]>;
OclBuiltinImpl cvt_u16_sat_rtp = OclBuiltinImpl<convert_u16_sat_rtp, float_types, 0, narrow_integers_code, ["$Rounding"->"rtp"]>;
OclBuiltinImpl cvt_u16_sat_rtn = OclBuiltinImpl<convert_u16_sat_rtn, float_types, 0, narrow_integers_code, ["$Rounding"->"rtn"]>;

// convert 64 and 32 bit integers
code convert_iu64_iu32_sat_code = [{
    $Arg0Symmicnativegentype arg0; arg0$Pattern = $Arg0VarName;
    $ReturnSymmicnativegentype ret = as_$ReturnSymmicnativegentype(
      _mm512_cvtsat_round$Arg0Sympspdgentype_$ReturnSymepuigentype(arg0, _MM_FROUND_TO_$Rounding));
    return ret$Pattern;
  }];

code convert_iu64_iu32_sat_mask_code = [{
    $Arg0Symmicnativegentype arg0; arg0$Pattern = $Arg0VarName;
    $ReturnSymmicnativegentype ret = as_$ReturnSymmicnativegentype(
      _mm512_mask_cvtsat_round$Arg0Sympspdgentype_$ReturnSymepuigentype(
        _mm512_undefined_epi32(), $Mask, arg0, _MM_FROUND_TO_$Rounding));
    return ret$Pattern;
  }];

// convert_i32_sat
OclBuiltinImpl cvt_i32_sat = OclBuiltinImpl<convert_i32_sat, float_types, 0, [{
    return convert_$ReturnType_sat_rtz($Arg0VarName);
  }]>;

// convert_i32_sat_rtz
OclBuiltinImpl cvt_i32_sat_rtz_v8f = OclBuiltinImpl<convert_i32_sat_rtz,
                                          [v8f64, v16f32], 0, convert_iu64_iu32_sat_code,
                                          ["$Rounding"->"ZERO"]>;
OclBuiltinImpl cvt_i32_sat_rtz_v1f_v4f = OclBuiltinImpl<convert_i32_sat_rtz,
                                          [v1f64, v2f64, v3f64, v4f64,
                                           v1f32, v2f32, v3f32, v4f32, v8f32],
                                           0, convert_iu64_iu32_sat_mask_code,
                                          ["$Rounding"->"ZERO"]>;
OclBuiltinImpl cvt_i32_sat_rtz_v16f64  = OclBuiltinImpl<convert_i32_sat_rtz, [v16f64], 0, Expand1>;

// convert_i32_sat_rte
OclBuiltinImpl cvt_i32_sat_rte_v8f = OclBuiltinImpl<convert_i32_sat_rte,
                                          [v8f64, v16f32], 0, convert_iu64_iu32_sat_code,
                                          ["$Rounding"->"NEAREST_INT"]>;
OclBuiltinImpl cvt_i32_sat_rte_v1f_v4f = OclBuiltinImpl<convert_i32_sat_rte,
                                          [v1f64, v2f64, v3f64, v4f64,
                                           v1f32, v2f32, v3f32, v4f32, v8f32],
                                           0, convert_iu64_iu32_sat_mask_code,
                                          ["$Rounding"->"NEAREST_INT"]>;
OclBuiltinImpl cvt_i32_sat_rte_v16f64  = OclBuiltinImpl<convert_i32_sat_rte, [v16f64], 0, Expand1>;

// convert_i32_sat_rtp
OclBuiltinImpl cvt_i32_sat_rtp_v8f = OclBuiltinImpl<convert_i32_sat_rtp,
                                          [v8f64, v16f32], 0, convert_iu64_iu32_sat_code,
                                          ["$Rounding"->"POS_INF"]>;
OclBuiltinImpl cvt_i32_sat_rtp_v1f_v4f = OclBuiltinImpl<convert_i32_sat_rtp,
                                          [v1f64, v2f64, v3f64, v4f64,
                                           v1f32, v2f32, v3f32, v4f32, v8f32],
                                           0, convert_iu64_iu32_sat_mask_code,
                                          ["$Rounding"->"POS_INF"]>;
OclBuiltinImpl cvt_i32_sat_rtp_v16f64  = OclBuiltinImpl<convert_i32_sat_rtp, [v16f64], 0, Expand1>;

// convert_i32_sat_rtn
OclBuiltinImpl cvt_i32_sat_rtn_v8f = OclBuiltinImpl<convert_i32_sat_rtn,
                                          [v8f64, v16f32], 0, convert_iu64_iu32_sat_code,
                                          ["$Rounding"->"NEG_INF"]>;
OclBuiltinImpl cvt_i32_sat_rtn_v1f_v4f = OclBuiltinImpl<convert_i32_sat_rtn,
                                          [v1f64, v2f64, v3f64, v4f64,
                                           v1f32, v2f32, v3f32, v4f32, v8f32],
                                           0, convert_iu64_iu32_sat_mask_code,
                                          ["$Rounding"->"NEG_INF"]>;
OclBuiltinImpl cvt_i32_sat_rtn_v16f64  = OclBuiltinImpl<convert_i32_sat_rtn, [v16f64], 0, Expand1>;

// convert_u32_sat
OclBuiltinImpl cvt_u32_sat = OclBuiltinImpl<convert_u32_sat, float_types, 0, [{
    return convert_$ReturnType_sat_rtz($Arg0VarName);
  }]>;

// convert_u32_sat_rtz
OclBuiltinImpl cvt_u32_sat_rtz_v8f = OclBuiltinImpl<convert_u32_sat_rtz,
                                          [v8f64, v16f32], 0, convert_iu64_iu32_sat_code,
                                          ["$Rounding"->"ZERO"]>;
OclBuiltinImpl cvt_u32_sat_rtz_v1f_v4f = OclBuiltinImpl<convert_u32_sat_rtz,
                                          [v1f64, v2f64, v3f64, v4f64,
                                           v1f32, v2f32, v3f32, v4f32, v8f32],
                                           0, convert_iu64_iu32_sat_mask_code,
                                          ["$Rounding"->"ZERO"]>;
OclBuiltinImpl cvt_u32_sat_rtz_v16f64  = OclBuiltinImpl<convert_u32_sat_rtz, [v16f64], 0, Expand1>;

// convert_u32_sat_rte
OclBuiltinImpl cvt_u32_sat_rte_v8f = OclBuiltinImpl<convert_u32_sat_rte,
                                          [v8f64, v16f32], 0, convert_iu64_iu32_sat_code,
                                          ["$Rounding"->"NEAREST_INT"]>;
OclBuiltinImpl cvt_u32_sat_rte_v1f_v4f = OclBuiltinImpl<convert_u32_sat_rte,
                                          [v1f64, v2f64, v3f64, v4f64,
                                           v1f32, v2f32, v3f32, v4f32, v8f32],
                                           0, convert_iu64_iu32_sat_mask_code,
                                          ["$Rounding"->"NEAREST_INT"]>;
OclBuiltinImpl cvt_u32_sat_rte_v16f64  = OclBuiltinImpl<convert_u32_sat_rte, [v16f64], 0, Expand1>;

// convert_u32_sat_rtp
OclBuiltinImpl cvt_u32_sat_rtp_v8f = OclBuiltinImpl<convert_u32_sat_rtp,
                                          [v8f64, v16f32], 0, convert_iu64_iu32_sat_code,
                                          ["$Rounding"->"POS_INF"]>;
OclBuiltinImpl cvt_u32_sat_rtp_v1f_v4f = OclBuiltinImpl<convert_u32_sat_rtp,
                                          [v1f64, v2f64, v3f64, v4f64,
                                           v1f32, v2f32, v3f32, v4f32, v8f32],
                                           0, convert_iu64_iu32_sat_mask_code,
                                          ["$Rounding"->"POS_INF"]>;
OclBuiltinImpl cvt_u32_sat_rtp_v16f64  = OclBuiltinImpl<convert_u32_sat_rtp, [v16f64], 0, Expand1>;

// convert_u32_sat_rtn
OclBuiltinImpl cvt_u32_sat_rtn_v8f = OclBuiltinImpl<convert_u32_sat_rtn,
                                          [v8f64, v16f32], 0, convert_iu64_iu32_sat_code,
                                          ["$Rounding"->"NEG_INF"]>;
OclBuiltinImpl cvt_u32_sat_rtn_v1f_v4f = OclBuiltinImpl<convert_u32_sat_rtn,
                                          [v1f64, v2f64, v3f64, v4f64,
                                           v1f32, v2f32, v3f32, v4f32, v8f32],
                                           0, convert_iu64_iu32_sat_mask_code,
                                          ["$Rounding"->"NEG_INF"]>;
OclBuiltinImpl cvt_u32_sat_rtn_v16f64  = OclBuiltinImpl<convert_u32_sat_rtn, [v16f64], 0, Expand1>;

// convert_i64_sat
OclBuiltinImpl cvt_i64_sat = OclBuiltinImpl<convert_i64_sat, float_types, 0, [{
    return convert_$ReturnType_sat_rtz($Arg0VarName);
  }]>;

// convert_i64_sat_rtz
OclBuiltinImpl cvt_i64_sat_rtz_v8f = OclBuiltinImpl<convert_i64_sat_rtz,
                                          [v8f64, v8f32], 0, convert_iu64_iu32_sat_code,
                                          ["$Rounding"->"ZERO"]>;
OclBuiltinImpl cvt_i64_sat_rtz_v1f_v4f = OclBuiltinImpl<convert_i64_sat_rtz,
                                          [v1f64, v2f64, v3f64, v4f64,
                                           v1f32, v2f32, v3f32, v4f32],
                                           0, convert_iu64_iu32_sat_mask_code,
                                          ["$Rounding"->"ZERO"]>;
OclBuiltinImpl cvt_i64_sat_rtz_v16f  = OclBuiltinImpl<convert_i64_sat_rtz, [v16f64, v16f32], 0, Expand1>;

// convert_i64_sat_rte
OclBuiltinImpl cvt_i64_sat_rte_v8f = OclBuiltinImpl<convert_i64_sat_rte,
                                          [v8f64, v8f32], 0, convert_iu64_iu32_sat_code,
                                          ["$Rounding"->"NEAREST_INT"]>;
OclBuiltinImpl cvt_i64_sat_rte_v1f_v4f = OclBuiltinImpl<convert_i64_sat_rte,
                                          [v1f64, v2f64, v3f64, v4f64,
                                           v1f32, v2f32, v3f32, v4f32],
                                           0, convert_iu64_iu32_sat_mask_code,
                                          ["$Rounding"->"NEAREST_INT"]>;
OclBuiltinImpl cvt_i64_sat_rte_v16f  = OclBuiltinImpl<convert_i64_sat_rte, [v16f64, v16f32], 0, Expand1>;

// convert_i64_sat_rtp
OclBuiltinImpl cvt_i64_sat_rtp_v8f = OclBuiltinImpl<convert_i64_sat_rtp,
                                          [v8f64, v8f32], 0, convert_iu64_iu32_sat_code,
                                          ["$Rounding"->"POS_INF"]>;
OclBuiltinImpl cvt_i64_sat_rtp_v1f_v4f = OclBuiltinImpl<convert_i64_sat_rtp,
                                          [v1f64, v2f64, v3f64, v4f64,
                                           v1f32, v2f32, v3f32, v4f32],
                                           0, convert_iu64_iu32_sat_mask_code,
                                          ["$Rounding"->"POS_INF"]>;
OclBuiltinImpl cvt_i64_sat_rtp_v16f  = OclBuiltinImpl<convert_i64_sat_rtp, [v16f64, v16f32], 0, Expand1>;

// convert_i64_sat_rtn
OclBuiltinImpl cvt_i64_sat_rtn_v8f = OclBuiltinImpl<convert_i64_sat_rtn,
                                          [v8f64, v8f32], 0, convert_iu64_iu32_sat_code,
                                          ["$Rounding"->"NEG_INF"]>;
OclBuiltinImpl cvt_i64_sat_rtn_v1f_v4f = OclBuiltinImpl<convert_i64_sat_rtn,
                                          [v1f64, v2f64, v3f64, v4f64,
                                           v1f32, v2f32, v3f32, v4f32],
                                           0, convert_iu64_iu32_sat_mask_code,
                                          ["$Rounding"->"NEG_INF"]>;
OclBuiltinImpl cvt_i64_sat_rtn_v16f  = OclBuiltinImpl<convert_i64_sat_rtn, [v16f64, v16f32], 0, Expand1>;


// convert_u64_sat
OclBuiltinImpl cvt_u64_sat = OclBuiltinImpl<convert_u64_sat, float_types, 0, [{
    return convert_$ReturnType_sat_rtz($Arg0VarName);
  }]>;

// convert_u64_sat_rtz
OclBuiltinImpl cvt_u64_sat_rtz_v8f = OclBuiltinImpl<convert_u64_sat_rtz,
                                          [v8f64, v8f32], 0, convert_iu64_iu32_sat_code,
                                          ["$Rounding"->"ZERO"]>;
OclBuiltinImpl cvt_u64_sat_rtz_v1f_v4f = OclBuiltinImpl<convert_u64_sat_rtz,
                                          [v1f64, v2f64, v3f64, v4f64,
                                           v1f32, v2f32, v3f32, v4f32],
                                           0, convert_iu64_iu32_sat_mask_code,
                                          ["$Rounding"->"ZERO"]>;
OclBuiltinImpl cvt_u64_sat_rtz_v16f  = OclBuiltinImpl<convert_u64_sat_rtz, [v16f64, v16f32], 0, Expand1>;

// convert_u64_sat_rte
OclBuiltinImpl cvt_u64_sat_rte_v8f = OclBuiltinImpl<convert_u64_sat_rte,
                                          [v8f64, v8f32], 0, convert_iu64_iu32_sat_code,
                                          ["$Rounding"->"NEAREST_INT"]>;
OclBuiltinImpl cvt_u64_sat_rte_v1f_v4f = OclBuiltinImpl<convert_u64_sat_rte,
                                          [v1f64, v2f64, v3f64, v4f64,
                                           v1f32, v2f32, v3f32, v4f32],
                                           0, convert_iu64_iu32_sat_mask_code,
                                          ["$Rounding"->"NEAREST_INT"]>;
OclBuiltinImpl cvt_u64_sat_rte_v16f  = OclBuiltinImpl<convert_u64_sat_rte, [v16f64, v16f32], 0, Expand1>;

// convert_u64_sat_rtp
OclBuiltinImpl cvt_u64_sat_rtp_v8f = OclBuiltinImpl<convert_u64_sat_rtp,
                                          [v8f64, v8f32], 0, convert_iu64_iu32_sat_code,
                                          ["$Rounding"->"POS_INF"]>;
OclBuiltinImpl cvt_u64_sat_rtp_v1f_v4f = OclBuiltinImpl<convert_u64_sat_rtp,
                                          [v1f64, v2f64, v3f64, v4f64,
                                           v1f32, v2f32, v3f32, v4f32],
                                           0, convert_iu64_iu32_sat_mask_code,
                                          ["$Rounding"->"POS_INF"]>;
OclBuiltinImpl cvt_u64_sat_rtp_v16f  = OclBuiltinImpl<convert_u64_sat_rtp, [v16f64, v16f32], 0, Expand1>;

// convert_u64_sat_rtn
OclBuiltinImpl cvt_u64_sat_rtn_v8f = OclBuiltinImpl<convert_u64_sat_rtn,
                                          [v8f64, v8f32], 0, convert_iu64_iu32_sat_code,
                                          ["$Rounding"->"NEG_INF"]>;
OclBuiltinImpl cvt_u64_sat_rtn_v1f_v4f = OclBuiltinImpl<convert_u64_sat_rtn,
                                          [v1f64, v2f64, v3f64, v4f64,
                                           v1f32, v2f32, v3f32, v4f32],
                                           0, convert_iu64_iu32_sat_mask_code,
                                          ["$Rounding"->"NEG_INF"]>;
OclBuiltinImpl cvt_u64_sat_rtn_v16f  = OclBuiltinImpl<convert_u64_sat_rtn, [v16f64, v16f32], 0, Expand1>;
