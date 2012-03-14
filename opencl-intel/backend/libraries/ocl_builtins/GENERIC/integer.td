// vim:ts=2:sw=2:et:


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

// rotate on signed integer
OclBuiltinImpl rotate_igeneric = OclBuiltinImpl<rotate, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                         v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                         v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                                         v1i64, v2i64, v3i64, v4i64, v8i64, v16i64], 0, [{
    return as_$ReturnType(rotate(as_u$Arg0Type($Arg0VarName), as_u$Arg1Type($Arg1VarName)));
  }]>;

// upsample on signed integer
OclBuiltinImpl upsample_igeneric = OclBuiltinImpl<upsample, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32], 0, [{
    return as_$ReturnType(upsample(as_u$Arg0Type($Arg0VarName), $Arg1VarName));
  }]>;

// popcount on signed integer
OclBuiltinImpl popcount_igeneric = OclBuiltinImpl<popcount, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                                             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64], 0, [{
    return as_$ReturnType(popcount(as_u$Arg0Type($Arg0VarName)));
  }]>;
