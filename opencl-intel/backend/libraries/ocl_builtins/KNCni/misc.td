// vim:ts=2:sw=2:et:
// Copyright (c) 2006-2012 Intel Corporation
// Miscellaneous
// OpenCL 1.2 rev 15, Table 6.20

// shuffle
code shuffle_promote = [{
    $ReturnType result;
    result.lo = shuffle($Arg0VarName, $Arg1VarName.lo);
    result.hi = shuffle($Arg0VarName, $Arg1VarName.hi);
    return result;
  }];

list<OclType> ShortPromoteTypes = [ v2i8,  v4i8,  v8i8,  v2u8,  v4u8,  v8u8,
                                   v2i16, v4i16, v8i16, v2u16, v4u16, v8u16,
                                   v2i32, v4i32, v8i32, v2u32, v4u32, v8u32,
                                   v2f32, v4f32, v8f32];
list<OclType> LongPromoteTypes  = [v2i64, v4i64, v2u64, v4u64, v2f64, v4f64];
list<OclType> ShortNativeTypes  = [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32];
list<OclType> LongNativeTypes   = [v8i64, v8u64, v8f64];
list<OclType> ExpandTypes       = [v16i64, v16u64, v16f64];
list<OclType> LongTypes         = [v2i64, v4i64, v8i64, v16i64,
                                   v2u64, v4u64, v8u64, v16u64,
                                   v2f64, v4f64, v8f64, v16f64];

OclBuiltinImpl shuffle_2_main_promote = OclBuiltinImpl<shuffle_2,
  [ v2i8,  v4i8,  v8i8,  v2u8,  v4u8,  v8u8,
   v2i16, v4i16, v8i16, v2u16, v4u16, v8u16,
   v2i32, v4i32, v8i32, v2u32, v4u32, v8u32,
   v2f32, v4f32, v8f32], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeArg1Type native_mask; native_mask.s01 = $Arg1VarName;
    __m512i masked_index = (__m512i)((int16)($VecLength -1));
    uint16 new_index = ext_from(native_mask);
    __m512i ext_index = (__m512i)new_index;
    ext_index = _mm512_and_epi32(ext_index, masked_index);
    $NativeReturnType result = trunc_to_$NativeReturnType(as_int16(gather(as_int16(ext_index), x_reg)));
    return result.s01;
  }]>;
OclBuiltinImpl shuffle_2_main = OclBuiltinImpl<shuffle_2, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32], 0, [{
    $NativeArg1Type native_mask; native_mask.s01 = $Arg1VarName;
    __m512i masked_index = (__m512i)((int16)($VecLength -1));
    uint16 new_index = ext_from(native_mask);
    __m512i ext_index = (__m512i)new_index;
    ext_index = _mm512_and_epi32(ext_index, masked_index);
    $NativeReturnType result = trunc_to_$NativeReturnType(as_int16(gather(as_int16(ext_index), $Arg0VarName)));
    return result.s01;
  }]>;
OclBuiltinImpl shuffle_2_long_promote = OclBuiltinImpl<shuffle_2, [v2i64, v4i64, v2u64, v4u64, v2f64, v4f64], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeArg1Type native_mask; native_mask.s01 = $Arg1VarName;
    $NativeReturnType result = shuffle_long_x8(x_reg, native_mask, (ulong8)($VecLength -1));
    return result.s01;
  }]>;
OclBuiltinImpl shuffle_2_long = OclBuiltinImpl<shuffle_2, [v8i64, v8u64, v8f64], 0, [{
    $NativeArg1Type native_mask; native_mask.s01 = $Arg1VarName;
    $NativeReturnType result = shuffle_long_x8($Arg0VarName, native_mask, (ulong8)($VecLength -1));
    return result.s01;
  }]>;
OclBuiltinImpl shuffle_2_long_v16 = OclBuiltinImpl<shuffle_2, [v16i64, v16u64, v16f64], 0, [{
    $NativeArg1Type native_mask;
    native_mask.s01 = $Arg1VarName;
    $NativeReturnType result = shuffle_long_x16($Arg0VarName, native_mask, (ulong8)15);
    return result.s01;
  }]>;
OclBuiltinImpl shuffle_4_main_promote = OclBuiltinImpl<shuffle_4, [v2i8, v4i8, v8i8, v2u8, v4u8, v8u8,
                                v2i16, v4i16, v8i16, v2u16, v4u16, v8u16,
                                v2i32, v4i32, v8i32, v2u32, v4u32, v8u32,
                                v2f32, v4f32, v8f32], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeArg1Type native_mask; native_mask.s0123 = $Arg1VarName;
    __m512i masked_index = (__m512i)((int16)($VecLength -1));
    uint16 new_index = ext_from(native_mask);
    __m512i ext_index = (__m512i)new_index;
    ext_index = _mm512_and_epi32(ext_index, masked_index);
    $NativeArg0Type result = trunc_to_$NativeArg0Type(as_int16(gather(as_int16(ext_index), x_reg)));
    return result.s0123;
  }]>;
OclBuiltinImpl shuffle_4_main = OclBuiltinImpl<shuffle_4, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32], 0, [{
    $NativeArg1Type native_mask; native_mask.s0123 = $Arg1VarName;
    __m512i masked_index = (__m512i)((int16)($VecLength -1));
    uint16 new_index = ext_from(native_mask);
    __m512i ext_index = (__m512i)new_index;
    ext_index = _mm512_and_epi32(ext_index, masked_index);
    $NativeReturnType result = trunc_to_$NativeReturnType(as_int16(gather(as_int16(ext_index), $Arg0VarName)));
    return result.s0123;
  }]>;
OclBuiltinImpl shuffle_4_long_promote = OclBuiltinImpl<shuffle_4, [v2i64, v4i64, v2u64, v4u64, v2f64, v4f64], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeArg1Type native_mask; native_mask.s0123 = $Arg1VarName;
    $NativeReturnType data = shuffle_long_x8(x_reg, native_mask, (ulong8)($VecLength-1));
    return data.s0123;
  }]>;
OclBuiltinImpl shuffle_4_long = OclBuiltinImpl<shuffle_4, [v8i64, v8u64, v8f64], 0, [{
    $NativeArg1Type native_mask; native_mask.s0123 = $Arg1VarName;
    $NativeReturnType data = shuffle_long_x8($Arg0VarName, native_mask, (ulong8)($VecLength-1));
    return data.s0123;
  }]>;
OclBuiltinImpl shuffle_4_long_v16 = OclBuiltinImpl<shuffle_4, [v16i64, v16u64, v16f64], 0, [{
    $NativeArg1Type native_mask;
    native_mask.s0123 = $Arg1VarName;
    $NativeReturnType result = shuffle_long_x16($Arg0VarName, native_mask, (ulong8)15);
    return result.s0123;
  }]>;
OclBuiltinImpl shuffle_8_main_promote = OclBuiltinImpl<shuffle_8, [v2i8, v4i8, v8i8, v2u8, v4u8, v8u8,
                                v2i16, v4i16, v8i16, v2u16, v4u16, v8u16,
                                v2i32, v4i32, v8i32, v2u32, v4u32, v8u32,
                                v2f32, v4f32, v8f32], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeArg1Type native_mask; native_mask.s01234567 = $Arg1VarName;
    __m512i masked_index = (__m512i)((int16)($VecLength-1));
    uint16 new_index = ext_from(native_mask);
    __m512i ext_index = (__m512i)new_index;
    ext_index = _mm512_and_epi32(ext_index, masked_index);
    $NativeReturnType result = trunc_to_$NativeReturnType(as_int16(gather(as_int16(ext_index), x_reg)));
    return result.s01234567;
  }]>;
OclBuiltinImpl shuffle_8_main = OclBuiltinImpl<shuffle_8, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32], 0, [{
    $NativeArg1Type native_mask; native_mask.s01234567 = $Arg1VarName;
    __m512i masked_index = (__m512i)((int16)($VecLength-1));
    uint16 new_index = ext_from(native_mask);
    __m512i ext_index = (__m512i)new_index;
    ext_index = _mm512_and_epi32(ext_index, masked_index);
    $NativeReturnType result = trunc_to_$NativeReturnType(as_int16(gather(as_int16(ext_index), $Arg0VarName)));
    return result.s01234567;
  }]>;
OclBuiltinImpl shuffle_8_long_promote = OclBuiltinImpl<shuffle_8, [v2i64, v4i64, v2u64, v4u64, v2f64, v4f64], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $ReturnType result = shuffle_long_x8(x_reg, $Arg1VarName, (ulong8)($VecLength-1));
    return result;
  }]>;
OclBuiltinImpl shuffle_8_long = OclBuiltinImpl<shuffle_8, [v8i64, v8u64, v8f64], 0, [{
    $ReturnType result = shuffle_long_x8($Arg0VarName, $Arg1VarName, (ulong8)($VecLength-1));
    return result;
  }]>;
OclBuiltinImpl shuffle_8_long_v16 = OclBuiltinImpl<shuffle_8, [v16i64, v16u64, v16f64], 0, [{
    $ReturnType result = shuffle_long_x16($Arg0VarName, $Arg1VarName, (ulong8)15);
    return result;
  }]>;
OclBuiltinImpl shuffle_16_main_promote = OclBuiltinImpl<shuffle_16, [v2i8, v4i8, v8i8, v2u8, v4u8, v8u8,
                                v2i16, v4i16, v8i16, v2u16, v4u16, v8u16,
                                v2i32, v4i32, v8i32, v2u32, v4u32, v8u32,
                                v2f32, v4f32, v8f32], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    __m512i masked_index = (__m512i)((int16)($VecLength-1));
    uint16 new_index = ext_from($Arg1VarName);
    __m512i ext_index = (__m512i)new_index;
    ext_index = _mm512_and_epi32(ext_index, masked_index);
    return trunc_to_$ReturnType(as_int16(gather(as_int16(ext_index), x_reg)));
  }]>;
OclBuiltinImpl shuffle_16_main = OclBuiltinImpl<shuffle_16, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32], 0, [{
    __m512i masked_index = (__m512i)((int16)($VecLength-1));
    uint16 new_index = ext_from($Arg1VarName);
    __m512i ext_index = (__m512i)new_index;
    ext_index = _mm512_and_epi32(ext_index, masked_index);
    return trunc_to_$ReturnType(as_int16(gather(as_int16(ext_index), $Arg0VarName)));
  }]>;
OclBuiltinImpl shuffle_16_long = OclBuiltinImpl<shuffle_16, [v2i64, v4i64, v8i64, v16i64,
                             v2u64, v4u64, v8u64, v16u64,
                             v2f64, v4f64, v8f64, v16f64], 0, shuffle_promote>;

// shuffle2

// Common procedure for shuffle2:
// 1. Convert inputs into native vector length (8 or 16 element).
// 2. Zeroing ignored bits in shuffle mask.
// 3. If original vector size was increased to native (2->16), mask values have to be updated accordingly.
// 4. Shuffle converted inputs.
// 5. Extract required size result from shuffle output.

code shuffle2_promote = [{
    $ReturnType result;
    result.lo = shuffle2($Arg0VarName, $Arg1VarName, $Arg2VarName.lo);
    result.hi = shuffle2($Arg0VarName, $Arg1VarName, $Arg2VarName.hi);
    return result;
  }];
// shuffle2 to two-element vector
OclBuiltinImpl shuffle2_2_main_promote = OclBuiltinImpl<shuffle2_2,
                                          [v2u8, v4u8, v8u8, v2i8, v4i8, v8i8,
                                           v2u16, v4u16, v8u16, v2i16, v4i16, v8i16,
                                           v2u32, v4u32, v8u32, v2i32, v4i32, v8i32,
                                           v2f32, v4f32, v8f32], 0, [{
    // Convert inputs into 16-element vectors
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeArg1Type y_reg; y_reg$Pattern = $Arg1VarName;
    $NativeArg2Type native_mask; native_mask.s01 = $Arg2VarName;
    $NativeReturnType result = shuffle2_x16(x_reg, y_reg, native_mask, $VecLength);
    return result.s01;
  }]>;
OclBuiltinImpl shuffle2_2_main = OclBuiltinImpl<shuffle2_2, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32], 0, [{
    $NativeArg2Type native_mask; native_mask.s01 = $Arg2VarName;
    $NativeReturnType result = shuffle2_x16($Arg0VarName, $Arg1VarName, native_mask, $VecLength);
    return result.s01;
  }]>;
OclBuiltinImpl shuffle2_2_long_promote = OclBuiltinImpl<shuffle2_2, [v2i64, v4i64, v2u64, v4u64, v2f64, v4f64], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeArg1Type y_reg; y_reg$Pattern = $Arg1VarName;
    $NativeArg2Type native_mask; native_mask.s01 = $Arg2VarName;
    $NativeReturnType result = shuffle2_long_x8(x_reg, y_reg, native_mask, $VecLength);
    return result.s01;
  }]>;
OclBuiltinImpl shuffle2_2_long = OclBuiltinImpl<shuffle2_2, [v8i64, v8u64, v8f64], 0, [{
    $NativeArg2Type native_mask; native_mask.s01 = $Arg2VarName;
    $NativeReturnType result = shuffle2_long_x8($Arg0VarName, $Arg1VarName, native_mask, $VecLength);
    return result.s01;
  }]>;
OclBuiltinImpl shuffle2_2_long_v16 = OclBuiltinImpl<shuffle2_2, [v16i64, v16u64, v16f64], 0, [{
    $NativeArg2Type native_mask;
    native_mask.s01 = $Arg2VarName;
    $NativeReturnType result = shuffle2_long_x16($Arg0VarName, $Arg1VarName, native_mask, (ulong8)31);
    return result.s01;
  }]>;

OclBuiltinImpl shuffle2_4_main_promote = OclBuiltinImpl<shuffle2_4,
                                          [v2u8, v4u8, v8u8, v2i8, v4i8, v8i8,
                                           v2u16, v4u16, v8u16, v2i16, v4i16, v8i16,
                                           v2u32, v4u32, v8u32, v2i32, v4i32, v8i32,
                                           v2f32, v4f32, v8f32], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeArg1Type y_reg; y_reg$Pattern = $Arg1VarName;
    $NativeArg2Type native_mask; native_mask.s0123 = $Arg2VarName;
    $NativeReturnType result = shuffle2_x16(x_reg, y_reg, native_mask, $VecLength);
    return result.s0123;
  }]>;
 OclBuiltinImpl shuffle2_4_main = OclBuiltinImpl<shuffle2_4, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32], 0, [{
    $NativeArg2Type native_mask; native_mask.s0123 = $Arg2VarName;
    $NativeReturnType result = shuffle2_x16($Arg0VarName, $Arg1VarName, native_mask, $VecLength);
    return result.s0123;
  }]>;
OclBuiltinImpl shuffle2_4_long_promote = OclBuiltinImpl<shuffle2_4, [v2i64, v4i64, v2u64, v4u64, v2f64, v4f64], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeArg1Type y_reg; y_reg$Pattern = $Arg1VarName;
    $NativeArg2Type native_mask; native_mask.s0123 = $Arg2VarName;
    $NativeReturnType result = shuffle2_long_x8(x_reg, y_reg, native_mask, $VecLength);
   return result.s0123;
  }]>;
OclBuiltinImpl shuffle2_4_long = OclBuiltinImpl<shuffle2_4, [v8i64, v8u64, v8f64], 0, [{
    $NativeArg2Type native_mask; native_mask.s0123 = $Arg2VarName;
    $NativeReturnType result = shuffle2_long_x8($Arg0VarName, $Arg1VarName, native_mask, $VecLength);
    return result.s0123;
  }]>;
OclBuiltinImpl shuffle2_4_long_v16 = OclBuiltinImpl<shuffle2_4, [v16i64, v16u64, v16f64], 0, [{
    $NativeArg2Type native_mask;
    native_mask.s0123 = $Arg2VarName;
    $NativeReturnType result = shuffle2_long_x16($Arg0VarName, $Arg1VarName, native_mask, (ulong8)31);
    return result.s0123;
  }]>;
OclBuiltinImpl shuffle2_8_main_promote = OclBuiltinImpl<shuffle2_8,
                                          [v2u8, v4u8, v8u8, v2i8, v4i8, v8i8,
                                           v2u16, v4u16, v8u16, v2i16, v4i16, v8i16,
                                           v2u32, v4u32, v8u32, v2i32, v4i32, v8i32,
                                           v2f32, v4f32, v8f32],  0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeArg1Type y_reg; y_reg$Pattern = $Arg1VarName;
    $NativeArg2Type native_mask; native_mask.s01234567 = $Arg2VarName;
    $NativeReturnType result = shuffle2_x16(x_reg, y_reg, native_mask, $VecLength);
    return result.s01234567;
  }]>;
OclBuiltinImpl shuffle2_8_main = OclBuiltinImpl<shuffle2_8, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32], 0, [{
    $NativeArg2Type native_mask; native_mask.s01234567 = $Arg2VarName;
    $NativeReturnType result = shuffle2_x16($Arg0VarName, $Arg1VarName, native_mask, $VecLength);
    return result.s01234567;
  }]>;
OclBuiltinImpl shuffle2_8_long_promote = OclBuiltinImpl<shuffle2_8, [v2i64, v4i64, v2u64, v4u64, v2f64, v4f64], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeArg1Type y_reg; y_reg$Pattern = $Arg1VarName;
    $ReturnType result = shuffle2_long_x8(x_reg, y_reg, $Arg2VarName, $VecLength);
    return result;
  }]>;
OclBuiltinImpl shuffle2_8_long = OclBuiltinImpl<shuffle2_8, [v8i64, v8u64, v8f64], 0, [{
    $ReturnType result = shuffle2_long_x8($Arg0VarName, $Arg1VarName, $Arg2VarName, $VecLength);
    return result;
  }]>;
OclBuiltinImpl shuffle2_8_long_v16 = OclBuiltinImpl<shuffle2_8, [v16i64, v16u64, v16f64], 0, [{
    $ReturnType result = shuffle2_long_x16($Arg0VarName, $Arg1VarName, $Arg2VarName, (ulong8)31);
    return result;
  }]>;
OclBuiltinImpl shuffle2_16_main_promote = OclBuiltinImpl<shuffle2_16,
                                          [v2u8, v4u8, v8u8, v2i8, v4i8, v8i8,
                                           v2u16, v4u16, v8u16, v2i16, v4i16, v8i16,
                                           v2u32, v4u32, v8u32, v2i32, v4i32, v8i32,
                                           v2f32, v4f32, v8f32],  0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeArg1Type y_reg; y_reg$Pattern = $Arg1VarName;
    $ReturnType result = shuffle2_x16(x_reg, y_reg, $Arg2VarName, $VecLength);
    return result;
  }]>;
 OclBuiltinImpl shuffle2_16_main = OclBuiltinImpl<shuffle2_16, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32], 0, [{
    $ReturnType result = shuffle2_x16($Arg0VarName, $Arg1VarName, $Arg2VarName, $VecLength);
    return result;
  }]>;
OclBuiltinImpl shuffle2_16_long = OclBuiltinImpl<shuffle2_16, [v2i64, v4i64, v8i64, v16i64,
                             v2u64, v4u64, v8u64, v16u64,
                             v2f64, v4f64, v8f64, v16f64], 0, shuffle2_promote>;

OclBuiltinImpl _mem_fence = OclBuiltinImpl<mem_fence, [v1i64], 0, [{
    _mm_micfence();
  }]>;

OclBuiltinImpl _read_mem_fence = OclBuiltinImpl<read_mem_fence, [v1i64], 0, [{
    _mm_miclfence();
  }]>;

OclBuiltinImpl _write_mem_fence = OclBuiltinImpl<write_mem_fence, [v1i64], 0, [{
    _mm_micsfence();
  }]>;



