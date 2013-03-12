// vim:ts=2:sw=2:et:

// Conversions optimized for MIC

// convert_i32
OclBuiltinImpl fp2int_i32_v16f32  = OclBuiltinImpl<convert_i32, [v16f32], 0, [{
    return as_$NativeReturnType(_mm512_cvtfxpnt_round_adjustps_epi32(
        $Arg0VarName, _MM_FROUND_TO_ZERO, _MM_EXPADJ_NONE));
  }]>;
OclBuiltinImpl fp2int_i32_v1_v8_f32  = OclBuiltinImpl<convert_i32, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = as_$NativeReturnType(_mm512_mask_cvtfxpnt_round_adjustps_epi32(
        _mm512_undefined_epi32(), $Mask, arg,
        _MM_FROUND_TO_ZERO, _MM_EXPADJ_NONE));
    return ret$Pattern;
  }]>;
OclBuiltinImpl fp2int_i32_v8f64  = OclBuiltinImpl<convert_i32, [v8f64], 0, [{
    $NativeReturnType ret = as_$NativeReturnType(_mm512_cvtfxpnt_roundpd_epi32lo(
        $Arg0VarName, _MM_FROUND_TO_ZERO));
    return ret$Pattern;
  }]>;
OclBuiltinImpl fp2int_i32_v16f64  = OclBuiltinImpl<convert_i32, [v16f64], 0, Expand1>; 
OclBuiltinImpl fp2int_i32_v1_v4_f64  = OclBuiltinImpl<convert_i32, [v1f64, v2f64, v3f64, v4f64], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = as_$NativeReturnType(
        _mm512_mask_cvtfxpnt_roundpd_epi32lo(
            _mm512_undefined_epi32(), $Mask, arg, _MM_FROUND_TO_ZERO));
    return ret$Pattern;
  }]>;

// convert_i32_rte
OclBuiltinImpl fp2int_i32_v16f32_rte  = OclBuiltinImpl<convert_i32_rte, [v16f32], 0, [{
    return as_$NativeReturnType(_mm512_cvtfxpnt_round_adjustps_epi32(
        $Arg0VarName, _MM_FROUND_TO_NEAREST_INT, _MM_EXPADJ_NONE));
  }]>;
OclBuiltinImpl fp2int_i32_v1_v8_f32_rte  = OclBuiltinImpl<convert_i32_rte, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = as_$NativeReturnType(_mm512_mask_cvtfxpnt_round_adjustps_epi32(
        _mm512_undefined_epi32(), $Mask, arg,
        _MM_FROUND_TO_NEAREST_INT, _MM_EXPADJ_NONE));
    return ret$Pattern;
  }]>;
OclBuiltinImpl fp2int_i32_v8f64_rte  = OclBuiltinImpl<convert_i32_rte, [v8f64], 0, [{
    $NativeReturnType ret = as_$NativeReturnType(_mm512_cvtfxpnt_roundpd_epi32lo(
        $Arg0VarName, _MM_FROUND_TO_NEAREST_INT));
    return ret$Pattern;
  }]>;
OclBuiltinImpl fp2int_i32_v16f64_rte  = OclBuiltinImpl<convert_i32_rte, [v16f64], 0, Expand1>; 
OclBuiltinImpl fp2int_i32_v1_v4_f64_rte  = OclBuiltinImpl<convert_i32_rte, [v1f64, v2f64, v3f64, v4f64], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = as_$NativeReturnType(
        _mm512_mask_cvtfxpnt_roundpd_epi32lo(
            _mm512_undefined_epi32(), $Mask, arg, _MM_FROUND_TO_NEAREST_INT));
    return ret$Pattern;
  }]>;

// convert_i32_rtz
OclBuiltinImpl fp2int_i32_f32_rtz  = OclBuiltinImpl<convert_i32_rtz, convert_i32_rtz.Types, 0, [{
    return convert_$ReturnType($Arg0VarName);
  }]>;

// convert_i32_rtp
OclBuiltinImpl fp2int_i32_v16f32_rtp  = OclBuiltinImpl<convert_i32_rtp, [v16f32], 0, [{
    return as_$NativeReturnType(_mm512_cvtfxpnt_round_adjustps_epi32(
        $Arg0VarName, _MM_FROUND_TO_POS_INF, _MM_EXPADJ_NONE));
  }]>;
OclBuiltinImpl fp2int_i32_v1_v8_f32_rtp  = OclBuiltinImpl<convert_i32_rtp, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = as_$NativeReturnType(_mm512_mask_cvtfxpnt_round_adjustps_epi32(
        _mm512_undefined_epi32(), $Mask, arg,
        _MM_FROUND_TO_POS_INF, _MM_EXPADJ_NONE));
    return ret$Pattern;
  }]>;
OclBuiltinImpl fp2int_i32_v8f64_rtp  = OclBuiltinImpl<convert_i32_rtp, [v8f64], 0, [{
    $NativeReturnType ret = as_$NativeReturnType(_mm512_cvtfxpnt_roundpd_epi32lo(
        $Arg0VarName, _MM_FROUND_TO_POS_INF));
    return ret$Pattern;
  }]>;
OclBuiltinImpl fp2int_i32_v16f64_rtp  = OclBuiltinImpl<convert_i32_rtp, [v16f64], 0, Expand1>; 
OclBuiltinImpl fp2int_i32_v1_v4_f64_rtp  = OclBuiltinImpl<convert_i32_rtp, [v1f64, v2f64, v3f64, v4f64], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = as_$NativeReturnType(
        _mm512_mask_cvtfxpnt_roundpd_epi32lo(
            _mm512_undefined_epi32(), $Mask, arg, _MM_FROUND_TO_POS_INF));
    return ret$Pattern;
  }]>;

// convert_i32_rtn
OclBuiltinImpl fp2int_i32_v16f32_rtn  = OclBuiltinImpl<convert_i32_rtn, [v16f32], 0, [{
    return as_$NativeReturnType(_mm512_cvtfxpnt_round_adjustps_epi32(
        $Arg0VarName, _MM_FROUND_TO_NEG_INF, _MM_EXPADJ_NONE));
  }]>;
OclBuiltinImpl fp2int_i32_v1_v8_f32_rtn  = OclBuiltinImpl<convert_i32_rtn, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = as_$NativeReturnType(_mm512_mask_cvtfxpnt_round_adjustps_epi32(
        _mm512_undefined_epi32(), $Mask, arg,
        _MM_FROUND_TO_NEG_INF, _MM_EXPADJ_NONE));
    return ret$Pattern;
  }]>;
OclBuiltinImpl fp2int_i32_v8f64_rtn  = OclBuiltinImpl<convert_i32_rtn, [v8f64], 0, [{
    $NativeReturnType ret = as_$NativeReturnType(_mm512_cvtfxpnt_roundpd_epi32lo(
        $Arg0VarName, _MM_FROUND_TO_NEG_INF));
    return ret$Pattern;
  }]>;
OclBuiltinImpl fp2int_i32_v16f64_rtn  = OclBuiltinImpl<convert_i32_rtn, [v16f64], 0, Expand1>; 
OclBuiltinImpl fp2int_i32_v1_v4_f64_rtn  = OclBuiltinImpl<convert_i32_rtn, [v1f64, v2f64, v3f64, v4f64], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = as_$NativeReturnType(
        _mm512_mask_cvtfxpnt_roundpd_epi32lo(
            _mm512_undefined_epi32(), $Mask, arg, _MM_FROUND_TO_NEG_INF));
    return ret$Pattern;
  }]>;

// convert_u32
OclBuiltinImpl fp2int_u32_v16f32  = OclBuiltinImpl<convert_u32, [v16f32], 0, [{
    return as_$NativeReturnType(_mm512_cvtfxpnt_round_adjustps_epu32(
        $Arg0VarName, _MM_FROUND_TO_ZERO, _MM_EXPADJ_NONE));
  }]>;
OclBuiltinImpl fp2int_u32_v1_v8_f32  = OclBuiltinImpl<convert_u32, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = as_$NativeReturnType(_mm512_mask_cvtfxpnt_round_adjustps_epu32(
        _mm512_undefined_epi32(), $Mask, arg,
        _MM_FROUND_TO_ZERO, _MM_EXPADJ_NONE));
    return ret$Pattern;
  }]>;
OclBuiltinImpl fp2int_u32_v8f64  = OclBuiltinImpl<convert_u32, [v8f64], 0, [{
    $NativeReturnType ret = as_$NativeReturnType(_mm512_cvtfxpnt_roundpd_epu32lo(
        $Arg0VarName, _MM_FROUND_TO_ZERO));
    return ret$Pattern;
  }]>;
OclBuiltinImpl fp2int_u32_v16f64  = OclBuiltinImpl<convert_u32, [v16f64], 0, Expand1>; 
OclBuiltinImpl fp2int_u32_v1_v4_f64  = OclBuiltinImpl<convert_u32, [v1f64, v2f64, v3f64, v4f64], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = as_$NativeReturnType(
        _mm512_mask_cvtfxpnt_roundpd_epu32lo(
            _mm512_undefined_epi32(), $Mask, arg, _MM_FROUND_TO_ZERO));
    return ret$Pattern;
  }]>;

// convert_u32_rte
OclBuiltinImpl fp2int_u32_v16f32_rte  = OclBuiltinImpl<convert_u32_rte, [v16f32], 0, [{
    return as_$NativeReturnType(_mm512_cvtfxpnt_round_adjustps_epu32(
        $Arg0VarName, _MM_FROUND_TO_NEAREST_INT, _MM_EXPADJ_NONE));
  }]>;
OclBuiltinImpl fp2int_u32_v1_v8_f32_rte  = OclBuiltinImpl<convert_u32_rte, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = as_$NativeReturnType(_mm512_mask_cvtfxpnt_round_adjustps_epu32(
        _mm512_undefined_epi32(), $Mask, arg,
        _MM_FROUND_TO_NEAREST_INT, _MM_EXPADJ_NONE));
    return ret$Pattern;
  }]>;
OclBuiltinImpl fp2int_u32_v8f64_rte  = OclBuiltinImpl<convert_u32_rte, [v8f64], 0, [{
    $NativeReturnType ret = as_$NativeReturnType(_mm512_cvtfxpnt_roundpd_epu32lo(
        $Arg0VarName, _MM_FROUND_TO_NEAREST_INT));
    return ret$Pattern;
  }]>;
OclBuiltinImpl fp2int_u32_v16f64_rte  = OclBuiltinImpl<convert_u32_rte, [v16f64], 0, Expand1>; 
OclBuiltinImpl fp2int_u32_v1_v4_f64_rte  = OclBuiltinImpl<convert_u32_rte, [v1f64, v2f64, v3f64, v4f64], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = as_$NativeReturnType(
        _mm512_mask_cvtfxpnt_roundpd_epu32lo(
            _mm512_undefined_epi32(), $Mask, arg, _MM_FROUND_TO_NEAREST_INT));
    return ret$Pattern;
  }]>;

// convert_u32_rtz
OclBuiltinImpl fp2int_u32_f32_rtz  = OclBuiltinImpl<convert_u32_rtz, convert_u32_rtz.Types, 0, [{
    return convert_$ReturnType($Arg0VarName);
  }]>;

// convert_u32_rtp
OclBuiltinImpl fp2int_u32_v16f32_rtp  = OclBuiltinImpl<convert_u32_rtp, [v16f32], 0, [{
    return as_$NativeReturnType(_mm512_cvtfxpnt_round_adjustps_epu32(
        $Arg0VarName, _MM_FROUND_TO_POS_INF, _MM_EXPADJ_NONE));
  }]>;
OclBuiltinImpl fp2int_u32_v1_v8_f32_rtp  = OclBuiltinImpl<convert_u32_rtp, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = as_$NativeReturnType(_mm512_mask_cvtfxpnt_round_adjustps_epu32(
        _mm512_undefined_epi32(), $Mask, arg,
        _MM_FROUND_TO_POS_INF, _MM_EXPADJ_NONE));
    return ret$Pattern;
  }]>;
OclBuiltinImpl fp2int_u32_v8f64_rtp  = OclBuiltinImpl<convert_u32_rtp, [v8f64], 0, [{
    $NativeReturnType ret = as_$NativeReturnType(_mm512_cvtfxpnt_roundpd_epu32lo(
        $Arg0VarName, _MM_FROUND_TO_POS_INF));
    return ret$Pattern;
  }]>;
OclBuiltinImpl fp2int_u32_v16f64_rtp  = OclBuiltinImpl<convert_u32_rtp, [v16f64], 0, Expand1>; 
OclBuiltinImpl fp2int_u32_v1_v4_f64_rtp  = OclBuiltinImpl<convert_u32_rtp, [v1f64, v2f64, v3f64, v4f64], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = as_$NativeReturnType(
        _mm512_mask_cvtfxpnt_roundpd_epu32lo(
            _mm512_undefined_epi32(), $Mask, arg, _MM_FROUND_TO_POS_INF));
    return ret$Pattern;
  }]>;

// convert_u32_rtn
OclBuiltinImpl fp2int_u32_v16f32_rtn  = OclBuiltinImpl<convert_u32_rtn, [v16f32], 0, [{
    return as_$NativeReturnType(_mm512_cvtfxpnt_round_adjustps_epu32(
        $Arg0VarName, _MM_FROUND_TO_NEG_INF, _MM_EXPADJ_NONE));
  }]>;
OclBuiltinImpl fp2int_u32_v1_v8_f32_rtn  = OclBuiltinImpl<convert_u32_rtn, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = as_$NativeReturnType(_mm512_mask_cvtfxpnt_round_adjustps_epu32(
        _mm512_undefined_epi32(), $Mask, arg,
        _MM_FROUND_TO_NEG_INF, _MM_EXPADJ_NONE));
    return ret$Pattern;
  }]>;
OclBuiltinImpl fp2int_u32_v8f64_rtn  = OclBuiltinImpl<convert_u32_rtn, [v8f64], 0, [{
    $NativeReturnType ret = as_$NativeReturnType(_mm512_cvtfxpnt_roundpd_epu32lo(
        $Arg0VarName, _MM_FROUND_TO_NEG_INF));
    return ret$Pattern;
  }]>;
OclBuiltinImpl fp2int_u32_v16f64_rtn  = OclBuiltinImpl<convert_u32_rtn, [v16f64], 0, Expand1>; 
OclBuiltinImpl fp2int_u32_v1_v4_f64_rtn  = OclBuiltinImpl<convert_u32_rtn, [v1f64, v2f64, v3f64, v4f64], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = as_$NativeReturnType(
        _mm512_mask_cvtfxpnt_roundpd_epu32lo(
            _mm512_undefined_epi32(), $Mask, arg, _MM_FROUND_TO_NEG_INF));
    return ret$Pattern;
  }]>;

// Workarounds for convert_<u|i>64.
// workaround begin
OclBuiltinImpl fp2int_i64_promote  = OclBuiltinImpl<convert_i64, [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                                  v1f64, v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl fp2int_i64_expand  = OclBuiltinImpl<convert_i64, [v16f64], 0, Expand1>;

OclBuiltinImpl fp2int_i64_rte_promote  = OclBuiltinImpl<convert_i64_rte, [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                                          v1f64, v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl fp2int_i64_rte_expand  = OclBuiltinImpl<convert_i64_rte, [v16f64], 0, Expand1>;

OclBuiltinImpl fp2int_i64_rtz_promote  = OclBuiltinImpl<convert_i64_rtz, [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                                          v1f64, v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl fp2int_i64_rtz_expand  = OclBuiltinImpl<convert_i64_rtz, [v16f64], 0, Expand1>;

OclBuiltinImpl fp2int_i64_rtp_promote  = OclBuiltinImpl<convert_i64_rtp, [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                                          v1f64, v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl fp2int_i64_rtp_expand  = OclBuiltinImpl<convert_i64_rtp, [v16f64], 0, Expand1>;

OclBuiltinImpl fp2int_i64_rtn_promote  = OclBuiltinImpl<convert_i64_rtn, [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                                          v1f64, v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl fp2int_i64_rtn_expand  = OclBuiltinImpl<convert_i64_rtn, [v16f64], 0, Expand1>;

OclBuiltinImpl fp2int_u64_promote  = OclBuiltinImpl<convert_u64, [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                                  v1f64, v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl fp2int_u64_expand  = OclBuiltinImpl<convert_u64, [v16f64], 0, Expand1>;

OclBuiltinImpl fp2int_u64_rte_promote  = OclBuiltinImpl<convert_u64_rte, [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                                          v1f64, v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl fp2int_u64_rte_expand  = OclBuiltinImpl<convert_u64_rte, [v16f64], 0, Expand1>;

OclBuiltinImpl fp2int_u64_rtz_promote  = OclBuiltinImpl<convert_u64_rtz, [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                                          v1f64, v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl fp2int_u64_rtz_expand  = OclBuiltinImpl<convert_u64_rtz, [v16f64], 0, Expand1>;

OclBuiltinImpl fp2int_u64_rtp_promote  = OclBuiltinImpl<convert_u64_rtp, [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                                          v1f64, v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl fp2int_u64_rtp_expand  = OclBuiltinImpl<convert_u64_rtp, [v16f64], 0, Expand1>;

OclBuiltinImpl fp2int_u64_rtn_promote  = OclBuiltinImpl<convert_u64_rtn, [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                                          v1f64, v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl fp2int_u64_rtn_expand  = OclBuiltinImpl<convert_u64_rtn, [v16f64], 0, Expand1>;
// workaround end

// convert_f32
OclBuiltinImpl cvt_f32_default = OclBuiltinImpl<convert_f32, convert_f32.Types, 0, [{
    return convert_$ReturnType_rte($Arg0VarName);
  }]>;

// convert_f32_rte
OclBuiltinImpl cvt_f32_rte_v16i32 = OclBuiltinImpl<convert_f32_rte,
                                          [v16i32, v16u32], 0, [{
    __m512 result = _mm512_cvtfxpnt_round_adjust$Arg0Symepuigentype_ps((__m512i)$Arg0VarName, _MM_FROUND_TO_NEAREST_INT, _MM_EXPADJ_NONE);
    return as_$ReturnType(result);
  }]>;

OclBuiltinImpl cvt_f32_rte_v16ui_8_16 = OclBuiltinImpl<convert_f32_rte,
                                          [v16i8, v16u8, v16i16, v16u16], 0, [{
    $Arg0ui32gentype arg = convert_$Arg0ui32gentype($Arg0VarName);
    return $Func(arg);
  }]>;

OclBuiltinImpl cvt_f32_rte_ui_8_32_mask = OclBuiltinImpl<convert_f32_rte,
                                          [v1i8,  v2i8,  v3i8,  v4i8,  v8i8,
                                           v1u8,  v2u8,  v3u8,  v4u8,  v8u8,
                                           v1i16, v2i16, v3i16, v4i16, v8i16,
                                           v1u16, v2u16, v3u16, v4u16, v8u16,
                                           v1i32, v2i32, v3i32, v4i32, v8i32,
                                           v1u32, v2u32, v3u32, v4u32, v8u32], 0, [{
    $Arg0v16ui32gentype arg; arg$Pattern = convert_$Arg0ui32gentype($Arg0VarName);
    __m512 result = _mm512_mask_cvtfxpnt_round_adjust$Arg0Symepuigentype_ps(_mm512_undefined_ps(), $Mask, (__m512i)arg, _MM_FROUND_TO_NEAREST_INT, _MM_EXPADJ_NONE);
    return as_$NativeReturnType(result)$Pattern;
  }]>;

OclBuiltinImpl mic_rte_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rte,
                                                    [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return as_$ReturnType(
            __ocl_trunc_$Arg0Type_$ReturnType($Arg0VarName));
  }]>;
// workaround begin
// v16i64 and v16u64 versions are defined in GENERIC part
OclBuiltinImpl rte_cvt_f32_v1_v8iu64 = OclBuiltinImpl<convert_f32_rte,
                                                     [v1i64, v2i64, v3i64, v4i64, v8i64,
                                                      v1u64, v2u64, v3u64, v4u64, v8u64], 0, [{
     $Arg0BaseType#16 arg; arg$Pattern = $Arg0VarName;
     return convert_$NativeReturnType_rte(arg)$Pattern;
  }]>;
// workaround end 

// convert_f32_rtz
OclBuiltinImpl cvt_f32_rtz_v16i32 = OclBuiltinImpl<convert_f32_rtz,
                                          [v16i32, v16u32], 0, [{
    __m512 result = _mm512_cvtfxpnt_round_adjust$Arg0Symepuigentype_ps((__m512i)$Arg0VarName, _MM_FROUND_TO_ZERO, _MM_EXPADJ_NONE);
    return as_$ReturnType(result);
  }]>;
OclBuiltinImpl cvt_f32_rtz_v16ui_8_16 = OclBuiltinImpl<convert_f32_rtz,
                                          [v16i8, v16u8, v16i16, v16u16], 0, [{
    $Arg0ui32gentype arg = convert_$Arg0ui32gentype($Arg0VarName);
    return convert_$ReturnType_rtz(arg);
  }]>;
OclBuiltinImpl cvt_f32_rtz_ui_8_32_mask = OclBuiltinImpl<convert_f32_rtz,
                                          [v1i8,  v2i8,  v3i8,  v4i8,  v8i8,
                                           v1u8,  v2u8,  v3u8,  v4u8,  v8u8,
                                           v1i16, v2i16, v3i16, v4i16, v8i16,
                                           v1u16, v2u16, v3u16, v4u16, v8u16,
                                           v1i32, v2i32, v3i32, v4i32, v8i32,
                                           v1u32, v2u32, v3u32, v4u32, v8u32], 0, [{
    $Arg0v16ui32gentype arg; arg$Pattern = convert_$Arg0ui32gentype($Arg0VarName);
    __m512 result = _mm512_mask_cvtfxpnt_round_adjust$Arg0Symepuigentype_ps(_mm512_undefined_ps(), $Mask, (__m512i)arg, _MM_FROUND_TO_ZERO, _MM_EXPADJ_NONE);
    return as_$NativeReturnType(result)$Pattern;
  }]>;

OclBuiltinImpl mic_rtz_cvt_f32_v8f64 = OclBuiltinImpl<convert_f32_rtz, [v8f64], 0, [{
    $NativeReturnType ret =  _mm512_cvt_roundpd_pslo((__m512d)$Arg0VarName, _MM_FROUND_TO_ZERO);
    return ret.lo;
  }]>;
OclBuiltinImpl mic_rtz_cvt_f32_v16f64 = OclBuiltinImpl<convert_f32_rtz, [v16f64], 0, Expand1>; 
OclBuiltinImpl mic_rtz_cvt_f32_f64_mask = OclBuiltinImpl<convert_f32_rtz,
                                                    [v1f64, v2f64, v3f64, v4f64], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret =  _mm512_mask_cvt_roundpd_pslo(_mm512_undefined_ps(), $Mask, (__m512d)arg, _MM_FROUND_TO_ZERO);
    return ret$Pattern;
  }]>;

// workaround begin
// v16i64 and v16u64 versions are defined in GENERIC part
OclBuiltinImpl rtz_cvt_f32_v1_v8iu64 = OclBuiltinImpl<convert_f32_rtz,
                                                     [v1i64, v2i64, v3i64, v4i64, v8i64,
                                                      v1u64, v2u64, v3u64, v4u64, v8u64], 0, [{
     $Arg0BaseType#16 arg; arg$Pattern = $Arg0VarName;
     return convert_$NativeReturnType_rtz(arg)$Pattern;
  }]>;
// workaround end 

// convert_f32_rtp
OclBuiltinImpl cvt_f32_rtp_v16i32 = OclBuiltinImpl<convert_f32_rtp,
                                          [v16i32, v16u32], 0, [{
    __m512 result = _mm512_cvtfxpnt_round_adjust$Arg0Symepuigentype_ps((__m512i)$Arg0VarName, _MM_FROUND_TO_POS_INF, _MM_EXPADJ_NONE);
    return as_$ReturnType(result);
  }]>;
OclBuiltinImpl cvt_f32_rtp_v16ui_8_16 = OclBuiltinImpl<convert_f32_rtp,
                                          [v16i8, v16u8, v16i16, v16u16], 0, [{
    $Arg0ui32gentype arg = convert_$Arg0ui32gentype($Arg0VarName);
    return convert_$ReturnType_rtp(arg);
  }]>;
OclBuiltinImpl cvt_f32_rtp_ui_8_32_mask = OclBuiltinImpl<convert_f32_rtp,
                                          [v1i8,  v2i8,  v3i8,  v4i8,  v8i8,
                                           v1u8,  v2u8,  v3u8,  v4u8,  v8u8,
                                           v1i16, v2i16, v3i16, v4i16, v8i16,
                                           v1u16, v2u16, v3u16, v4u16, v8u16,
                                           v1i32, v2i32, v3i32, v4i32, v8i32,
                                           v1u32, v2u32, v3u32, v4u32, v8u32], 0, [{
    $Arg0v16ui32gentype arg; arg$Pattern = convert_$Arg0ui32gentype($Arg0VarName);
    __m512 result = _mm512_mask_cvtfxpnt_round_adjust$Arg0Symepuigentype_ps(_mm512_undefined_ps(), $Mask, (__m512i)arg, _MM_FROUND_TO_POS_INF, _MM_EXPADJ_NONE);
    return as_$NativeReturnType(result)$Pattern;
  }]>;

OclBuiltinImpl mic_rtp_cvt_f32_v8f64 = OclBuiltinImpl<convert_f32_rtp, [v8f64], 0, [{
    $NativeReturnType ret =  _mm512_cvt_roundpd_pslo((__m512d)$Arg0VarName, _MM_FROUND_TO_POS_INF);
    return ret.lo;
  }]>;
OclBuiltinImpl mic_rtp_cvt_f32_v16f64 = OclBuiltinImpl<convert_f32_rtp, [v16f64], 0, Expand1>; 
OclBuiltinImpl mic_rtp_cvt_f32_f64_mask = OclBuiltinImpl<convert_f32_rtp,
                                                    [v1f64, v2f64, v3f64, v4f64], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret =  _mm512_mask_cvt_roundpd_pslo(_mm512_undefined_ps(), $Mask, (__m512d)arg, _MM_FROUND_TO_POS_INF);
    return ret$Pattern;
  }]>;

// workaround begin
OclBuiltinImpl rtp_cvt_f32_v2v16u64 = OclBuiltinImpl<convert_f32_rtp,
                                                  [v1u64, v2u64, v3u64, v4u64, v8u64, v16u64], 0, [{
  // workaround for SVML bug DPD200299224 
  $Arg0f64gentype tmp = convert_$Arg0f64gentype_rtp($Arg0VarName);
  return convert_$Arg0f32gentype_rtp(tmp);
}]>;

// v16i64 version is defined in GENERIC part
OclBuiltinImpl rtp_cvt_f32_v1v8i64 = OclBuiltinImpl<convert_f32_rtp,
                                                     [v1i64, v2i64, v3i64, v4i64, v8i64], 0, [{
     $Arg0BaseType#16 arg; arg$Pattern = $Arg0VarName;
     return convert_$NativeReturnType_rtp(arg)$Pattern;
  }]>;
// workaround end 

// convert_f32_rtn
OclBuiltinImpl cvt_f32_rtn_v16i32 = OclBuiltinImpl<convert_f32_rtn,
                                          [v16i32, v16u32], 0, [{
    __m512 result = _mm512_cvtfxpnt_round_adjust$Arg0Symepuigentype_ps((__m512i)$Arg0VarName, _MM_FROUND_TO_NEG_INF, _MM_EXPADJ_NONE);
    return as_$ReturnType(result);
  }]>;
OclBuiltinImpl cvt_f32_rtn_v16ui_8_16 = OclBuiltinImpl<convert_f32_rtn,
                                          [v16i8, v16u8, v16i16, v16u16], 0, [{
    $Arg0ui32gentype arg = convert_$Arg0ui32gentype($Arg0VarName);
    return convert_$ReturnType_rtn(arg);
  }]>;
OclBuiltinImpl cvt_f32_rtn_ui_8_32_mask = OclBuiltinImpl<convert_f32_rtn,
                                          [v1i8,  v2i8,  v3i8,  v4i8,  v8i8,
                                           v1u8,  v2u8,  v3u8,  v4u8,  v8u8,
                                           v1i16, v2i16, v3i16, v4i16, v8i16,
                                           v1u16, v2u16, v3u16, v4u16, v8u16,
                                           v1i32, v2i32, v3i32, v4i32, v8i32,
                                           v1u32, v2u32, v3u32, v4u32, v8u32], 0, [{
    $Arg0v16ui32gentype arg; arg$Pattern = convert_$Arg0ui32gentype($Arg0VarName);
    __m512 result = _mm512_mask_cvtfxpnt_round_adjust$Arg0Symepuigentype_ps(_mm512_undefined_ps(), $Mask, (__m512i)arg, _MM_FROUND_TO_NEG_INF, _MM_EXPADJ_NONE);
    return as_$NativeReturnType(result)$Pattern;
  }]>;

OclBuiltinImpl mic_rtn_cvt_f32_v8f64 = OclBuiltinImpl<convert_f32_rtn, [v8f64], 0, [{
    $NativeReturnType ret =  _mm512_cvt_roundpd_pslo((__m512d)$Arg0VarName, _MM_FROUND_TO_NEG_INF);
    return ret.lo;
  }]>;
OclBuiltinImpl mic_rtn_cvt_f32_v16f64 = OclBuiltinImpl<convert_f32_rtn, [v16f64], 0, Expand1>; 
OclBuiltinImpl mic_rtn_cvt_f32_f64_mask = OclBuiltinImpl<convert_f32_rtn,
                                                    [v1f64, v2f64, v3f64, v4f64], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret =  _mm512_mask_cvt_roundpd_pslo(_mm512_undefined_ps(), $Mask, (__m512d)arg, _MM_FROUND_TO_NEG_INF);
    return ret$Pattern;
  }]>;

// workaround begin
// v16i64 and v16u64 versions are defined in GENERIC part
OclBuiltinImpl rtn_cvt_f32_v1_v8iu64 = OclBuiltinImpl<convert_f32_rtn,
                                                     [v1i64, v2i64, v3i64, v4i64, v8i64,
                                                      v1u64, v2u64, v3u64, v4u64, v8u64], 0, [{
     $Arg0BaseType#16 arg; arg$Pattern = $Arg0VarName;
     return convert_$NativeReturnType_rtn(arg)$Pattern;
  }]>;
// workaround end 

// convert_f64
OclBuiltinImpl cvt_f64_all = OclBuiltinImpl<convert_f64, convert_f64.Types, 0, [{
    return convert_$ReturnType_rte($Arg0VarName);
  }]>;

// convert_f64_rte
OclBuiltinImpl cvt_f64_rte_v8ui32 = OclBuiltinImpl<convert_f64_rte,
                                          [v8u32, v8i32], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    return _mm512_cvt$Arg0Symepuigentype#lo_pd((__m512i)arg);
  }]>;
OclBuiltinImpl cvt_f64_rte_v8f32 = OclBuiltinImpl<convert_f64_rte, [v8f32], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    return _mm512_cvtpslo_pd(arg);
  }]>;
OclBuiltinImpl cvt_f64_rte_v16x32 = OclBuiltinImpl<convert_f64_rte,
                                          [v16u32, v16i32, v16f32], 0, Expand1>;
OclBuiltinImpl cvt_f64_rte_v1_v4_ui32 = OclBuiltinImpl<convert_f64_rte,
                                          [v1u32, v2u32, v3u32, v4u32,
                                           v1i32, v2i32, v3i32, v4i32], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = _mm512_mask_cvt$Arg0Symepuigentype#lo_pd(_mm512_undefined_pd(), $Mask, (__m512i)arg);
    return ret$Pattern;
  }]>;
OclBuiltinImpl cvt_f64_rte_v1_v16_ui8_ui16 = OclBuiltinImpl<convert_f64_rte,
                                          [v1i8, v2i8, v3i8, v4i8, v8i8, v16i8,
                                           v1u8, v2u8, v3u8, v4u8, v8u8, v16u8,
                                           v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                           v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    $Arg0ui32gentype arg = convert_$Arg0ui32gentype($Arg0VarName);
    return convert_$ReturnType_rte(arg);
  }]>;
OclBuiltinImpl cvt_f64_rte_v1_v4_f32 = OclBuiltinImpl<convert_f64_rte,
                                          [v1f32, v2f32, v3f32, v4f32], 0, [{
    $NativeArg0Type arg; arg$Pattern = $Arg0VarName;
    $NativeReturnType ret = _mm512_mask_cvtpslo_pd(_mm512_undefined_pd(), $Mask, arg);
    return ret$Pattern;
  }]>;
// convert_f64_rte for doubles is defined in GENERIC part

// workaround begin
// v8i64 and v8u64 versions are defined in GENERIC part
OclBuiltinImpl rte_cvt_f64_v1_v4_iu64 = OclBuiltinImpl<convert_f64_rte,
                                                     [v1i64, v2i64, v3i64, v4i64,
                                                      v1u64, v2u64, v3u64, v4u64], 0, [{
     $Arg0BaseType#8 arg; arg$Pattern = $Arg0VarName;
     return convert_$NativeReturnType_rte(arg)$Pattern;
  }]>;
OclBuiltinImpl rte_cvt_f64_v16_iu64 = OclBuiltinImpl<convert_f64_rte, [v16i64, v16u64], 0, Expand1>;
// workaround end 

list<OclType> convert_f64_types = [v1i8, v2i8, v3i8, v4i8, v8i8, v16i8,
                                   v1u8, v2u8, v3u8, v4u8, v8u8, v16u8,
                                   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                   v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                   v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                                   v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
                                   v1f64, v2f64, v3f64, v4f64, v8f64, v16f64];

// convert_f64_rtz
OclBuiltinImpl cvt_f64_rtz = OclBuiltinImpl<convert_f64_rtz, convert_f64_types, 0, [{
    return convert_$ReturnType_rte($Arg0VarName);
  }]>;
// workaround begin
// v8i64 and v8u64 versions are defined in GENERIC part
OclBuiltinImpl rtz_cvt_f64_v1_v4_iu64 = OclBuiltinImpl<convert_f64_rtz,
                                                     [v1i64, v2i64, v3i64, v4i64,
                                                      v1u64, v2u64, v3u64, v4u64], 0, [{
     $Arg0BaseType#8 arg; arg$Pattern = $Arg0VarName;
     return convert_$NativeReturnType_rtz(arg)$Pattern;
  }]>;
OclBuiltinImpl rtz_cvt_f64_v16_iu64 = OclBuiltinImpl<convert_f64_rtz, [v16i64, v16u64], 0, Expand1>;
// workaround end 

// convert_f64_rtp
OclBuiltinImpl cvt_f64_rtp = OclBuiltinImpl<convert_f64_rtp, convert_f64_types, 0, [{
    return convert_$ReturnType_rte($Arg0VarName);
  }]>;
// workaround begin
// v8i64 and v8u64 versions are defined in GENERIC part
OclBuiltinImpl rtp_cvt_f64_v1_v4_iu64 = OclBuiltinImpl<convert_f64_rtp,
                                                     [v1i64, v2i64, v3i64, v4i64,
                                                      v1u64, v2u64, v3u64, v4u64], 0, [{
     $Arg0BaseType#8 arg; arg$Pattern = $Arg0VarName;
     return convert_$NativeReturnType_rtp(arg)$Pattern;
  }]>;
OclBuiltinImpl rtp_cvt_f64_v16_iu64 = OclBuiltinImpl<convert_f64_rtp, [v16i64, v16u64], 0, Expand1>;
// workaround end 

// convert_f64_rtn
OclBuiltinImpl cvt_f64_rtn = OclBuiltinImpl<convert_f64_rtn, convert_f64_types, 0, [{
    return convert_$ReturnType_rte($Arg0VarName);
  }]>;
// workaround begin
// v8i64 and v8u64 versions are defined in GENERIC part
OclBuiltinImpl rtn_cvt_f64_v1_v4_iu64 = OclBuiltinImpl<convert_f64_rtn,
                                                     [v1i64, v2i64, v3i64, v4i64,
                                                      v1u64, v2u64, v3u64, v4u64], 0, [{
     $Arg0BaseType#8 arg; arg$Pattern = $Arg0VarName;
     return convert_$NativeReturnType_rtn(arg)$Pattern;
  }]>;
OclBuiltinImpl rtn_cvt_f64_v16_iu64 = OclBuiltinImpl<convert_f64_rtn, [v16i64, v16u64], 0, Expand1>;
// workaround end 
