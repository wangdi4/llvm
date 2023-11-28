// Copyright (c) 2006 Intel Corporation
// Miscellaneous Built-In Functions
// SSSE3

// For Shuffle() and Suffle2(), signed integers and floating point types call the unsigned
//   integers version, so we need to define only the unsigned integers types here (e.g. uchar,
//   ushort, uint, ulong)

// promote & expand shuffle() by vector (first argument)
code PromoteShuffleByVector = [{
    $ReturnType tX;
    tX$Pattern = $Arg0VarName;
    $Arg1VarName = $Arg1VarName & ($Arg1Type)($VecLength - 1);
    return shuffle(tX, $Arg1VarName);
  }];
code ExpandShuffleByVector = [{
    $Arg1VarName = $Arg1VarName & ($Arg1Type)($VecLength - 1);
    $ReturnType temp1 = shuffle($Arg0VarName.lo, $Arg1VarName & ($Arg1Type)($VecLength/2 - 1));
    $ReturnType temp2 = shuffle($Arg0VarName.hi, $Arg1VarName & ($Arg1Type)($VecLength/2 - 1));
    return ($Arg1VarName < ($Arg1Type)($VecLength/2)) ? temp1 : temp2;
  }];

// promote & expand shuffle() by mask (second argurment)
code PromoteShuffleByMask = [{
    $NativeArg1Type tMask = ($NativeArg1Type)0;
    tMask$MaskPattern = $Arg1VarName;
    return shuffle($Arg0VarName, tMask)$MaskPattern;
  }];
code ExpandShuffleByMask = [{
    $ReturnType res;
    res.lo = shuffle($Arg0VarName, $Arg1VarName.lo);
    res.hi = shuffle($Arg0VarName, $Arg1VarName.hi);
    return res;
  }];

// Intrinsic-like native shuffle
code IntrinsicLikeShuffle = [{
    $Arg1VarName = $Arg1VarName & ($Arg1Type)($VecLength - 1);
    $Arg1VarName = as_$Arg1Type( _mm_shuffle_epi8((__m128i)$Arg1VarName, (__m128i)_shuffle_epi$BitWidth_smask));
    $Arg1VarName = as_$Arg1Type( as_short8($Arg1VarName) << (short8)$ShiftLeftBy );
    $Arg1VarName = as_$Arg1Type( add_sat( as_uchar16($Arg1VarName), _shuffle_epi$BitWidth_amask ) );
    $Arg0VarName = as_$Arg0Type( _mm_shuffle_epi8((__m128i)$Arg0VarName, (__m128i)$Arg1VarName) );
    return $Arg0VarName;
  }];

/*****************************************************************\
 * shuffle:                                                      *
 *   gentypen shuffle (gentypem x, ugentypen mask)               *
 *   The shuffle built-in function constructs a                  *
 *     permutation of elements from input vector, returning a    *
 *     vector with the same element type as the input and        *
 *     length that is the same as the shuffle mask.              *
\*****************************************************************/

/*****************************************************************\
 * shuffle with mask size 2                                      *
\*****************************************************************/
// shuffle with mask size 2 - chars shorts ints => promote to native mask
OclBuiltinImpl shuffle_2_promote = OclBuiltinImpl<shuffle_2, [v2u8,  v4u8,  v8u8,  v16u8,
                                                              v2u16, v4u16, v8u16, v16u16,
                                                              v2u32, v4u32, v8u32, v16u32], 0,
                                                              PromoteShuffleByMask, [["$MaskPattern",".s01"]]>;
// shuffle with mask size 2 - long2
OclBuiltinImpl sse42_shuffle_2_v2u64 = OclBuiltinImpl<shuffle_2, [v2u64], 0, IntrinsicLikeShuffle, [["$ShiftLeftBy","3"]]>;
// shuffle with mask size 2 - long4, long8, long16 => expand to long2
OclBuiltinImpl shuffle_2_v4v8v16u64 = OclBuiltinImpl<shuffle_2, [v4u64, v8u64, v16u64], 0, ExpandShuffleByVector>;


/*****************************************************************\
 * shuffle with mask size 4                                      *
\*****************************************************************/
// shuffle with mask size 4 - chars shorts => promote to native mask
OclBuiltinImpl shuffle_4_promote = OclBuiltinImpl<shuffle_4, [v2u8,  v4u8,  v8u8,  v16u8,
                                                              v2u16, v4u16, v8u16, v16u16], 0,
                                                              PromoteShuffleByMask, [["$MaskPattern",".s0123"]]>;
// shuffle with mask size 4 - longs => expand to native mask
OclBuiltinImpl shuffle_4_expand = OclBuiltinImpl<shuffle_4, [v2u64, v4u64, v8u64, v16u64], 0, ExpandShuffleByMask>;

// shuffle with mask size 4 - int2 => promote to int4
OclBuiltinImpl shuffle_4_v2u32 = OclBuiltinImpl<shuffle_4, [v2u32], 0, PromoteShuffleByVector>;
// shuffle with mask size 4 - int4
OclBuiltinImpl sse42_shuffle_4_v4u32 = OclBuiltinImpl<shuffle_4, [v4u32], 0, IntrinsicLikeShuffle, [["$ShiftLeftBy","2"]]>;
// shuffle with mask size 4 - int8, int16 => expand to int4
OclBuiltinImpl shuffle_4_v8v16u32 = OclBuiltinImpl<shuffle_4, [v8u32, v16u32], 0, ExpandShuffleByVector>;


/*****************************************************************\
 * shuffle with mask size 8                                      *
\*****************************************************************/
// shuffle with mask size 8 - chars => promote to native mask
OclBuiltinImpl shuffle_8_promote = OclBuiltinImpl<shuffle_8, [v2u8,  v4u8,  v8u8,  v16u8], 0,
                                                  PromoteShuffleByMask, [["$MaskPattern",".s01234567"]]>;
// shuffle with mask size 8 - ints longs => expand to native mask
OclBuiltinImpl shuffle_8_expand = OclBuiltinImpl<shuffle_8, [v2u32, v4u32, v8u32, v16u32,
                                                             v2u64, v4u64, v8u64, v16u64], 0, ExpandShuffleByMask>;

// shuffle with mask size 8 - short2, short4 => promote to short8
OclBuiltinImpl shuffle_8_v2v4u16 = OclBuiltinImpl<shuffle_8, [v2u16, v4u16], 0, PromoteShuffleByVector>;
// shuffle with mask size 8 - short8
OclBuiltinImpl sse42_shuffle_8_v8u16 = OclBuiltinImpl<shuffle_8, [v8u16], 0, IntrinsicLikeShuffle, [["$ShiftLeftBy","1"]]>;
// shuffle with mask size 8 - short16 => expand to short8
OclBuiltinImpl shuffle_8_v16u16 = OclBuiltinImpl<shuffle_8, [v16u16], 0, ExpandShuffleByVector>;


/*****************************************************************\
 * shuffle with mask size 16                                     *
\*****************************************************************/
// shuffle with mask size 16 - shorts ints longs => expand to native mask
OclBuiltinImpl shuffle_16_expand = OclBuiltinImpl<shuffle_16, [v2u16, v4u16, v8u16, v16u16,
                                                               v2u32, v4u32, v8u32, v16u32,
                                                               v2u64, v4u64, v8u64, v16u64], 0, ExpandShuffleByMask>;

// shuffle with mask size 16 - char2, char4, char8 => promote to char16 
OclBuiltinImpl shuffle_16_v2v4v8u8 = OclBuiltinImpl<shuffle_16, [v2u8, v4u8, v8u8], 0, PromoteShuffleByVector>;
// shuffle with mask size 16 - char16
OclBuiltinImpl sse42_shuffle_16_v16u8 = OclBuiltinImpl<shuffle_16, [v16u8], 0, [{
    $Arg1VarName = $Arg1VarName & ($Arg1Type)($VecLength - 1);
    $Arg0VarName = as_$Arg0Type( _mm_shuffle_epi8((__m128i)$Arg0VarName, (__m128i)$Arg1VarName) );
    return $Arg0VarName;
  }]>;


/*****************************************************************\
 * Memory Fence Functions                                        *
\*****************************************************************/
OclBuiltinImpl _mem_fence = OclBuiltinImpl<mem_fence, [v1i64], 0, [{
    _mm_mfence();
  }]>;
OclBuiltinImpl _read_mem_fence = OclBuiltinImpl<read_mem_fence, [v1i64], 0, [{
    _mm_lfence();
  }]>;
OclBuiltinImpl _write_mem_fence = OclBuiltinImpl<write_mem_fence, [v1i64], 0, [{
    _mm_sfence();
  }]>;
