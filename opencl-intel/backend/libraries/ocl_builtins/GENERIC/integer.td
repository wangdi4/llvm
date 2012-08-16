// vim:ts=2:sw=2:et:

// abs on signed integer
OclBuiltinImpl abs_v8_i64 = OclBuiltinImpl<abs, [v8i64], 0, [{
    return as_$ReturnType(($Arg0VarName < 0) ? -$Arg0VarName : $Arg0VarName);
  }]>;

// abs on unsigned integer
OclBuiltinImpl abs_ugeneric = OclBuiltinImpl<abs, [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                                   v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                                                   v1u64, v2u64, v3u64, v4u64, v8u64, v16u64], 0, [{
    return $Arg0VarName;
  }]>;

// abs_diff
OclBuiltinImpl abs_diff_generic = OclBuiltinImpl<abs_diff, abs_diff.Types, 0, [{
    return as_$ReturnType(($Arg0Type)(max($Arg0VarName, $Arg1VarName) - min($Arg0VarName, $Arg1VarName)));
  }]>;

// add_sat
OclBuiltinImpl add_sat_v16_i8 = OclBuiltinImpl<add_sat, [v16i8], 0, [{
    int16 t0 = convert_int16($Arg0VarName);
    int16 t1 = convert_int16($Arg1VarName);
    return convert_$ReturnType(clamp(add_sat(t0, t1), CHAR_MIN, CHAR_MAX));
  }]>;
OclBuiltinImpl add_sat_v16_u8 = OclBuiltinImpl<add_sat, [v16u8], 0, [{
    uint16 t0 = convert_uint16($Arg0VarName);
    uint16 t1 = convert_uint16($Arg1VarName);
    return convert_$ReturnType(clamp(add_sat(t0, t1), (uint)0, (uint)UCHAR_MAX));
  }]>;
OclBuiltinImpl add_sat_v16_i16 = OclBuiltinImpl<add_sat, [v16i16], 0, [{
    int16 t0 = convert_int16($Arg0VarName);
    int16 t1 = convert_int16($Arg1VarName);
    return convert_$ReturnType(clamp(add_sat(t0, t1), SHRT_MIN, SHRT_MAX));
  }]>;
OclBuiltinImpl add_sat_v16_u16 = OclBuiltinImpl<add_sat, [v16u16], 0, [{
    uint16 t0 = convert_uint16($Arg0VarName);
    uint16 t1 = convert_uint16($Arg1VarName);
    return convert_$ReturnType(clamp(add_sat(t0, t1), (uint)0, (uint)USHRT_MAX));
  }]>;
OclBuiltinImpl add_sat_v16_i32 = OclBuiltinImpl<add_sat, [v16i32], 0, [{
    int16 r = $Arg0VarName + $Arg1VarName;
    int16 p8 = (($Arg0VarName | $Arg1VarName) > 0) & (r < 0);
    int16 n8 = (($Arg0VarName & $Arg1VarName) < 0) & (r > 0);
    return  (n8 == 0) ?
                (p8 == 0) ? r : (int16)INT_MAX :
                (int16)INT_MIN;
  }]>;
OclBuiltinImpl add_sat_v16_u32 = OclBuiltinImpl<add_sat, [v16u32], 0, [{
    $ReturnType r = $Arg0VarName + $Arg1VarName;
    return (r < $Arg0VarName) ? ($ReturnType)UINT_MAX : r;
  }]>;
OclBuiltinImpl add_sat_v8_i64 = OclBuiltinImpl<add_sat, [v8i64], 0, [{
    long8 r = $Arg0VarName + $Arg1VarName;
    long8 p8 = (($Arg0VarName | $Arg1VarName) > 0) & (r < 0);
    long8 n8 = (($Arg0VarName & $Arg1VarName) < 0) & (r > 0);
    return  (n8 == 0) ?
                (p8 == 0) ? r : (long8)LONG_MAX :
                (long8)LONG_MIN;
  }]>;
OclBuiltinImpl add_sat_v8_u64 = OclBuiltinImpl<add_sat, [v8u64], 0, [{
    $ReturnType r = $Arg0VarName + $Arg1VarName;
    return (r < $Arg0VarName) ? ($ReturnType)ULONG_MAX : r;
  }]>;
OclBuiltinImpl add_sat_promote = OclBuiltinImpl<add_sat, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8/*,  v16i8*/,
                                                           v1u8,  v2u8,  v3u8,  v4u8,  v8u8/*,  v16u8*/,
                                                          v1i16, v2i16, v3i16, v4i16, v8i16/*, v16i16*/,
                                                          v1u16, v2u16, v3u16, v4u16, v8u16/*, v16u16*/,
                                                          v1i32, v2i32, v3i32, v4i32, v8i32/*, v16i32*/,
                                                          v1u32, v2u32, v3u32, v4u32, v8u32/*, v16u32*/,
                                                          v1i64, v2i64, v3i64, v4i64/*, v8i64, v16i64*/,
                                                          v1u64, v2u64, v3u64, v4u64/*, v8u64, v16u64*/], 0, [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName);
    return $ReturnVarName$Pattern;
  }]>;
OclBuiltinImpl add_sat_expand = OclBuiltinImpl<add_sat, [v16i64, v16u64], 0, Expand2>;

// hadd
OclBuiltinImpl hadd_generic = OclBuiltinImpl<hadd, hadd.Types, 0, [{
    $ReturnType s, c;
    s = $Arg0VarName ^ $Arg1VarName;
    c = $Arg0VarName & $Arg1VarName;
    return (s >> (($ReturnType)1)) + c;
  }]>;

// rhadd
OclBuiltinImpl rhadd_generic = OclBuiltinImpl<rhadd, rhadd.Types, 0, [{
    $ReturnType s, c;
    s = $Arg0VarName ^ $Arg1VarName;
    c = $Arg0VarName & $Arg1VarName;
    return (s >> (($ReturnType)1)) + (c | (s & (($ReturnType)1)));
  }]>;

// clamp on integer
OclBuiltinImpl clamp_igeneric = OclBuiltinImpl<clamp,
  [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
    v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
   v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
   v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
   v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
   v1u64, v2u64, v3u64, v4u64, v8u64, v16u64], 0, [{
    return max(min($Arg0VarName, $Arg2VarName), $Arg1VarName);
  }]>;

// clz on signed integer
OclBuiltinImpl clz_igeneric = OclBuiltinImpl<clz, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                   v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                                   v1i64, v2i64, v3i64, v4i64, v8i64, v16i64], 0, [{
    return as_$ReturnType(clz(as_u$Arg0Type($Arg0VarName)));
  }]>;

// mad_hi
OclBuiltinImpl mad_hi_generic = OclBuiltinImpl<mad_hi, mad_hi.Types, 0, [{
    return mul_hi($Arg0VarName, $Arg1VarName) + $Arg2VarName;
  }]>;

// popcount on signed integer
OclBuiltinImpl popcount_igeneric = OclBuiltinImpl<popcount, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                                             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64], 0, [{
    return as_$ReturnType(popcount(as_u$Arg0Type($Arg0VarName)));
  }]>;

// rotate on signed integer
OclBuiltinImpl rotate_igeneric = OclBuiltinImpl<rotate, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                         v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                         v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                                         v1i64, v2i64, v3i64, v4i64, v8i64, v16i64], 0, [{
    return as_$ReturnType(rotate(as_u$Arg0Type($Arg0VarName), as_u$Arg1Type($Arg1VarName)));
  }]>;

// sub_sat
OclBuiltinImpl sub_sat_v16_i8 = OclBuiltinImpl<sub_sat, [v16i8], 0, [{
    int16 t0 = convert_int16($Arg0VarName);
    int16 t1 = convert_int16($Arg1VarName);
    return convert_$ReturnType(clamp(sub_sat(t0, t1), CHAR_MIN, CHAR_MAX));
  }]>;
OclBuiltinImpl sub_sat_v16_i16 = OclBuiltinImpl<sub_sat, [v16i16], 0, [{
    int16 t0 = convert_int16($Arg0VarName);
    int16 t1 = convert_int16($Arg1VarName);
    return convert_$ReturnType(clamp(sub_sat(t0, t1), SHRT_MIN, SHRT_MAX));
  }]>;
OclBuiltinImpl sub_sat_v16_i32 = OclBuiltinImpl<sub_sat, [v16i32], 0, [{
    $Arg0Type r = $Arg0VarName - $Arg1VarName;
    $Arg0Type p8 = ($Arg1VarName < 0) & (r < $Arg0VarName);
    $Arg0Type n8 = ($Arg1VarName > 0) & (r > $Arg0VarName);

    return  (n8 == 0) ?
                (p8 == 0) ? r : (int16)INT_MAX :
                (int16)INT_MIN;
  }]>;
OclBuiltinImpl sub_sat_v16_i64 = OclBuiltinImpl<sub_sat, [v8i64], 0, [{
    $Arg0Type r = $Arg0VarName - $Arg1VarName;
    $Arg0Type p8 = ($Arg1VarName < 0) & (r < $Arg0VarName);
    $Arg0Type n8 = ($Arg1VarName > 0) & (r > $Arg0VarName);

    return  (n8 == 0) ?
                (p8 == 0) ? r : (long8)LONG_MAX :
                (long8)LONG_MIN;
  }]>;
OclBuiltinImpl sub_sat_v16_u8 = OclBuiltinImpl<sub_sat, [v16u8], 0, [{
    uint16 t0 = convert_uint16($Arg0VarName);
    uint16 t1 = convert_uint16($Arg1VarName);
    return convert_$ReturnType(clamp(sub_sat(t0, t1), (uint)0, (uint)UCHAR_MAX));
  }]>;
OclBuiltinImpl sub_sat_v16_u16 = OclBuiltinImpl<sub_sat, [v16u16], 0, [{
    uint16 t0 = convert_uint16($Arg0VarName);
    uint16 t1 = convert_uint16($Arg1VarName);
    return convert_$ReturnType(clamp(sub_sat(t0, t1), (uint)0, (uint)USHRT_MAX));
  }]>;
OclBuiltinImpl sub_sat_v16_u32 = OclBuiltinImpl<sub_sat, [v16u32, v8u64], 0, [{
    $Arg0Type r = $Arg0VarName - $Arg1VarName;
    return ($Arg0VarName < r) ? 0 : r;
  }]>;
OclBuiltinImpl sub_sat_promote = OclBuiltinImpl<sub_sat, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8/*,  v16i8*/,
                                                           v1u8,  v2u8,  v3u8,  v4u8,  v8u8/*,  v16u8*/,
                                                          v1i16, v2i16, v3i16, v4i16, v8i16/*, v16i16*/,
                                                          v1u16, v2u16, v3u16, v4u16, v8u16/*, v16u16*/,
                                                          v1i32, v2i32, v3i32, v4i32, v8i32/*, v16i32*/,
                                                          v1u32, v2u32, v3u32, v4u32, v8u32/*, v16u32*/,
                                                          v1i64, v2i64, v3i64, v4i64/*, v8i64, v16i64*/,
                                                          v1u64, v2u64, v3u64, v4u64/*, v8u64, v16u64*/], 0, [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName);
    return $ReturnVarName$Pattern;
  }]>;
OclBuiltinImpl sub_sat_expand = OclBuiltinImpl<sub_sat, [v16i64, v16u64], 0, Expand2>;

// upsample on signed integer
OclBuiltinImpl upsample_igeneric = OclBuiltinImpl<upsample, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32], 0, [{
    return as_$ReturnType(upsample(as_u$Arg0Type($Arg0VarName), $Arg1VarName));
  }]>;
