// Copyright (c) 2006-2012 Intel Corporation
// Integers Built-In Functions
// SSE42

code ExpandShuffleTo2Calls = [{
    $ReturnType res;
    res.lo = shuffle($Arg0VarName, $Arg1VarName.lo);
    res.hi = shuffle($Arg0VarName, $Arg1VarName.hi);
    return res;
  }];
code ExpandShuffle2To2Calls = [{
    $ReturnType res;
    res.lo = shuffle2($Arg0VarName, $Arg1VarName, $Arg2VarName.lo);
    res.hi = shuffle2($Arg0VarName, $Arg1VarName, $Arg2VarName.hi);
    return res;
  }];

code ExpandShuffleTo4Calls = [{
    $ReturnType res;
    res.lo.lo = shuffle($Arg0VarName, $Arg1VarName.lo.lo);
    res.lo.hi = shuffle($Arg0VarName, $Arg1VarName.lo.hi);
    res.hi.lo = shuffle($Arg0VarName, $Arg1VarName.hi.lo);
    res.hi.hi = shuffle($Arg0VarName, $Arg1VarName.hi.hi);
    return res;
  }];
code ExpandShuffle2To4Calls = [{
    $ReturnType res;
    res.lo.lo = shuffle2($Arg0VarName, $Arg1VarName, $Arg2VarName.lo.lo);
    res.lo.hi = shuffle2($Arg0VarName, $Arg1VarName, $Arg2VarName.lo.hi);
    res.hi.lo = shuffle2($Arg0VarName, $Arg1VarName, $Arg2VarName.hi.lo);
    res.hi.hi = shuffle2($Arg0VarName, $Arg1VarName, $Arg2VarName.hi.hi);
    return res;
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
// shuffle with mask size 2 - chars shorts ints
OclBuiltinImpl shuffle_2_promote = OclBuiltinImpl<shuffle_2, [ v2u8,  v4u8,  v8u8,  v16u8,
                                                              v2u16, v4u16, v8u16, v16u16,
                                                              v2u32, v4u32, v8u32, v16u32], 0, [{
    $NativeArg1Type tMask = ($NativeArg1Type)0;
    tMask.s01 = $Arg1VarName;
    return shuffle($Arg0VarName, tMask).s01;
  }]>;
// shuffle with mask size 2 - fix for long2
OclBuiltinImpl shuffle_2_v2u64 = OclBuiltinImpl<shuffle_2, [v2u64], 0, [{
    $Arg1VarName = $Arg1VarName & ($Arg1Type)1;
    $Arg0VarName = _MM_SHUFFLE_EPI64($Arg0VarName, $Arg1VarName);
    return $Arg0VarName;
  }]>;


/*****************************************************************\
 * shuffle with mask size 4                                      *
\*****************************************************************/
// shuffle with mask size 4 - chars shorts 
OclBuiltinImpl shuffle_4_promote = OclBuiltinImpl<shuffle_4, [ v2u8,  v4u8,  v8u8,  v16u8,
                                                              v2u16, v4u16, v8u16, v16u16], 0, [{
    $NativeArg1Type tMask = ($NativeArg1Type)0;
    tMask.s0123 = $Arg1VarName;
    return shuffle($Arg0VarName, tMask).s0123;
  }]>;
// shuffle with mask size 4 - int2
OclBuiltinImpl shuffle_4_v2u32 = OclBuiltinImpl<shuffle_4, [v2u32], 0, [{
    $ReturnType tX;
    tX.s01 = $Arg0VarName;
    $Arg1VarName = $Arg1VarName & ($Arg1Type)1;
    tX = _MM_SHUFFLE_EPI32(tX, $Arg1VarName);
    return tX;
  }]>;
// shuffle with mask size 4 - int4
OclBuiltinImpl shuffle_4_v4u32 = OclBuiltinImpl<shuffle_4, [v4u32], 0, [{
    $Arg1VarName = $Arg1VarName & ($Arg1Type)3;
    $Arg0VarName = _MM_SHUFFLE_EPI32($Arg0VarName, $Arg1VarName);
    return $Arg0VarName;
  }]>;
// shuffle with mask size 4 - int8
OclBuiltinImpl shuffle_4_v8u32 = OclBuiltinImpl<shuffle_4, [v8u32], 0, [{
    $Arg1VarName = $Arg1VarName & ($Arg1Type)7;
    $ReturnType res1 = _MM_SHUFFLE_EPI32($Arg0VarName.lo, $Arg1VarName + ($Arg1Type)(28));
    $ReturnType res2 = _MM_SHUFFLE_EPI32($Arg0VarName.hi, $Arg1VarName - ($Arg1Type)(4));
    return res1 | res2;
  }]>;
// shuffle with mask size 4 - int16
OclBuiltinImpl shuffle_4_v16u32 = OclBuiltinImpl<shuffle_4, [v16u32], 0, [{
    $Arg1VarName = $Arg1VarName & ($Arg1Type)15;

    uchar16 int4_28 = as_uchar16(($Arg1Type)28);
    $Arg1Type t1 = as_$Arg1Type( add_sat(as_uchar16($Arg1VarName), int4_28) );
    $Arg1Type t2 = as_$Arg1Type( add_sat(as_uchar16($Arg1VarName - ($Arg1Type)(4)), int4_28) );
    $Arg1Type t3 = as_$Arg1Type( add_sat(as_uchar16($Arg1VarName - ($Arg1Type)(8)), int4_28) );
    $Arg1Type t4 = as_$Arg1Type( add_sat(as_uchar16($Arg1VarName - ($Arg1Type)(12)), int4_28) );

    $ReturnType res1 = _MM_SHUFFLE_EPI32($Arg0VarName.s0123, t1);
    $ReturnType res2 = _MM_SHUFFLE_EPI32($Arg0VarName.s4567, t2);
    $ReturnType res3 = _MM_SHUFFLE_EPI32($Arg0VarName.s89AB, t3);
    $ReturnType res4 = _MM_SHUFFLE_EPI32($Arg0VarName.sCDEF, t4);

    return res1 | res2 | res3 | res4;
  }]>;


/*****************************************************************\
 * shuffle with mask size 8                                      *
\*****************************************************************/
// shuffle with mask size 8 - chars 
OclBuiltinImpl shuffle_8_promote = OclBuiltinImpl<shuffle_8, [v2u8,  v4u8,  v8u8,  v16u8], 0, [{
    $NativeArg1Type tMask = ($NativeArg1Type)0;
    tMask.s01234567 = $Arg1VarName;
    return shuffle($Arg0VarName, tMask).s01234567;
  }]>;
// shuffle with mask size 8 - short2
OclBuiltinImpl shuffle_8_v2u16 = OclBuiltinImpl<shuffle_8, [v2u16], 0, [{
    $ReturnType tX;
    tX.s01 = $Arg0VarName;
    $Arg1VarName = $Arg1VarName & ($Arg1Type)1;
    tX = _MM_SHUFFLE_EPI16(tX, $Arg1VarName);
    return tX;
  }]>;
// shuffle with mask size 8 - short4
OclBuiltinImpl shuffle_8_v4u16 = OclBuiltinImpl<shuffle_8, [v4u16], 0, [{
    $ReturnType tX;
    tX.s0123 = $Arg0VarName;
    $Arg1VarName = $Arg1VarName & ($Arg1Type)3;
    tX = _MM_SHUFFLE_EPI16(tX, $Arg1VarName);
    return tX;
  }]>;
// shuffle with mask size 8 - short8
OclBuiltinImpl shuffle_8_v8u16 = OclBuiltinImpl<shuffle_8, [v8u16], 0, [{
    $Arg1VarName = $Arg1VarName & ($Arg1Type)7;
    $Arg0VarName = _MM_SHUFFLE_EPI16($Arg0VarName, $Arg1VarName);
    return $Arg0VarName;
  }]>;
// shuffle with mask size 8 - short16
OclBuiltinImpl shuffle_8_v16u16 = OclBuiltinImpl<shuffle_8, [v16u16], 0, [{
    $Arg1VarName = $Arg1VarName & ($Arg1Type)15;
    $ReturnType res1 = _MM_SHUFFLE_EPI16($Arg0VarName.lo, $Arg1VarName + ($Arg1Type)56);
    $ReturnType res2 = _MM_SHUFFLE_EPI16($Arg0VarName.hi, $Arg1VarName - ($Arg1Type)8);
    return res1 | res2;
  }]>;
// shuffle with mask size 8 - ints
OclBuiltinImpl shuffle_8_i32 = OclBuiltinImpl<shuffle_8, [v2u32, v4u32, v8u32, v16u32], 0, ExpandShuffleTo2Calls>;


/*****************************************************************\
 * shuffle with mask size 16                                     *
\*****************************************************************/
// shuffle with mask size 16 - char2 
OclBuiltinImpl shuffle_16_v2u8 = OclBuiltinImpl<shuffle_16, [v2u8], 0, [{
    $ReturnType tX;
    tX.s01 = $Arg0VarName;
    $Arg1VarName = $Arg1VarName & ($Arg1Type)1;
    tX = _MM_SHUFFLE_EPI8(tX, $Arg1VarName);
    return tX;
  }]>;
// shuffle with mask size 16 - char4
OclBuiltinImpl shuffle_16_v4u8 = OclBuiltinImpl<shuffle_16, [v4u8], 0, [{
    $ReturnType tX;
    tX.s0123 = $Arg0VarName;
    $Arg1VarName = $Arg1VarName & ($Arg1Type)3;
    tX = _MM_SHUFFLE_EPI8(tX, $Arg1VarName);
    return tX;
  }]>;
// shuffle with mask size 16 - char8
OclBuiltinImpl shuffle_16_v8u8 = OclBuiltinImpl<shuffle_16, [v8u8], 0, [{
    $ReturnType tX;
    tX.s01234567 = $Arg0VarName;
    $Arg1VarName = $Arg1VarName & ($Arg1Type)7;
    tX = _MM_SHUFFLE_EPI8(tX, $Arg1VarName);
    return tX;
  }]>;
// shuffle with mask size 16 - char16
OclBuiltinImpl shuffle_16_v16u8 = OclBuiltinImpl<shuffle_16, [v16u8], 0, [{
    $Arg1VarName = $Arg1VarName & ($Arg1Type)15;
    $Arg0VarName = _MM_SHUFFLE_EPI8($Arg0VarName, $Arg1VarName);
    return $Arg0VarName;
  }]>;
// shuffle with mask size 16 - shorts
OclBuiltinImpl shuffle_16_i16 = OclBuiltinImpl<shuffle_16, [v2u16, v4u16, v8u16, v16u16], 0, ExpandShuffleTo2Calls>;
// shuffle with mask size 16 - ints
OclBuiltinImpl shuffle_16_i32 = OclBuiltinImpl<shuffle_16, [v2u32, v4u32, v8u32, v16u32], 0, ExpandShuffleTo4Calls>;


/*****************************************************************\
 * shuffle2:                                                     *
 *   gentypen shuffle2 (gentypem x, gentypem y, ugentypen mask)  *
 *   The shuffle2 built-in function constructs a                 *
 *     permutation of elements from two input vectors,           *
 *     returning a vector with the same element type as the      *
 *     input and length that is the same as the shuffle mask.    *
\*****************************************************************/

/*****************************************************************\
 * shuffle2 with mask size 2                                     *
\*****************************************************************/
// shuffle2 with mask size 2 - chars shorts ints
OclBuiltinImpl shuffle2_2_promote = OclBuiltinImpl<shuffle2_2, [ v2u8,  v4u8,  v8u8,  v16u8,
                                                                v2u16, v4u16, v8u16, v16u16,
                                                                v2u32, v4u32, v8u32, v16u32], 0, [{
    $NativeArg2Type tMask = ($NativeArg2Type)0;
    tMask.s01 = $Arg2VarName;
    return shuffle2($Arg0VarName, $Arg1VarName, tMask).s01;
  }]>;


/*****************************************************************\
 * shuffle2 with mask size 4                                     *
\*****************************************************************/
// shuffle2 with mask size 4 - chars shorts 
OclBuiltinImpl shuffle2_4_promote = OclBuiltinImpl<shuffle2_4, [ v2u8,  v4u8,  v8u8,  v16u8,
                                                                v2u16, v4u16, v8u16, v16u16], 0, [{
    $NativeArg2Type tMask = ($NativeArg2Type)0;
    tMask.s0123 = $Arg2VarName;
    return shuffle2($Arg0VarName, $Arg1VarName, tMask).s0123;
  }]>;
// shuffle2 with mask size 4 - int2
OclBuiltinImpl shuffle2_4_v2u32 = OclBuiltinImpl<shuffle2_4, [v2u32], 0, [{
    uint4 tX;
    tX.s01 = $Arg0VarName;
    tX.s23 = $Arg1VarName;
    return shuffle(tX, $Arg2VarName);
  }]>;
// shuffle2 with mask size 4 - int4
OclBuiltinImpl shuffle2_4_v4u32 = OclBuiltinImpl<shuffle2_4, [v4u32], 0, [{
    $Arg2VarName = $Arg2VarName & ($Arg2Type)7;
    $ReturnType res1 = _MM_SHUFFLE_EPI32($Arg0VarName, $Arg2VarName + ($Arg2Type)(28));
    $ReturnType res2 = _MM_SHUFFLE_EPI32($Arg1VarName, $Arg2VarName - ($Arg2Type)(4));
    return res1 | res2;
  }]>;
// shuffle2 with mask size 4 - int8
OclBuiltinImpl shuffle2_4_v8u32 = OclBuiltinImpl<shuffle2_4, [v8u32], 0, [{
    $Arg2VarName = $Arg2VarName & ($Arg2Type)15;

    uchar16 int4_28 = as_uchar16(($Arg2Type)28);
    $Arg2Type t1 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName), int4_28) );
    $Arg2Type t2 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName - ($Arg2Type)(4)), int4_28) );
    $Arg2Type t3 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName - ($Arg2Type)(8)), int4_28) );
    $Arg2Type t4 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName - ($Arg2Type)(12)), int4_28) );

    $ReturnType res1 = _MM_SHUFFLE_EPI32($Arg0VarName.lo, t1);
    $ReturnType res2 = _MM_SHUFFLE_EPI32($Arg0VarName.hi, t2);
    $ReturnType res3 = _MM_SHUFFLE_EPI32($Arg1VarName.lo, t3);
    $ReturnType res4 = _MM_SHUFFLE_EPI32($Arg1VarName.hi, t4);

    return res1 | res2 | res3 | res4;
  }]>;
// shuffle2 with mask size 4 - int16
OclBuiltinImpl shuffle2_4_v16u32 = OclBuiltinImpl<shuffle2_4, [v16u32], 0, [{
    $Arg2VarName = $Arg2VarName & ($Arg2Type)31;

    uchar16 int4_28 = as_uchar16(($Arg2Type)28);
    $Arg2Type t1 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName), int4_28) );
    $Arg2Type t2 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName - ($Arg2Type)(4)), int4_28) );
    $Arg2Type t3 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName - ($Arg2Type)(8)), int4_28) );
    $Arg2Type t4 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName - ($Arg2Type)(12)), int4_28) );
    $Arg2Type t5 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName - ($Arg2Type)(16)), int4_28) );
    $Arg2Type t6 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName - ($Arg2Type)(20)), int4_28) );
    $Arg2Type t7 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName - ($Arg2Type)(24)), int4_28) );
    $Arg2Type t8 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName - ($Arg2Type)(28)), int4_28) );

    $ReturnType res1 = _MM_SHUFFLE_EPI32($Arg0VarName.s0123, t1);
    $ReturnType res2 = _MM_SHUFFLE_EPI32($Arg0VarName.s4567, t2);
    $ReturnType res3 = _MM_SHUFFLE_EPI32($Arg0VarName.s89AB, t3);
    $ReturnType res4 = _MM_SHUFFLE_EPI32($Arg0VarName.sCDEF, t4);
    $ReturnType res5 = _MM_SHUFFLE_EPI32($Arg1VarName.s0123, t5);
    $ReturnType res6 = _MM_SHUFFLE_EPI32($Arg1VarName.s4567, t6);
    $ReturnType res7 = _MM_SHUFFLE_EPI32($Arg1VarName.s89AB, t7);
    $ReturnType res8 = _MM_SHUFFLE_EPI32($Arg1VarName.sCDEF, t8);

    return res1 | res2 | res3 | res4 | res5 | res6 | res7 | res8;
  }]>;


/*****************************************************************\
 * shuffle2 with mask size 8                                     *
\*****************************************************************/
// shuffle2 with mask size 8 - chars 
OclBuiltinImpl shuffle2_8_promote = OclBuiltinImpl<shuffle2_8, [v2u8,  v4u8,  v8u8,  v16u8], 0, [{
    $NativeArg2Type tMask = ($NativeArg2Type)0;
    tMask.s01234567 = $Arg2VarName;
    return shuffle2($Arg0VarName, $Arg1VarName, tMask).s01234567;
  }]>;
// shuffle2 with mask size 8 - short2
OclBuiltinImpl shuffle2_8_v2u16 = OclBuiltinImpl<shuffle2_8, [v2u16], 0, [{
    ushort4 tX;
    tX.s01 = $Arg0VarName;
    tX.s23 = $Arg1VarName;
    return shuffle(tX, $Arg2VarName);
  }]>;
// shuffle2 with mask size 8 - short4
OclBuiltinImpl shuffle2_8_v4u16 = OclBuiltinImpl<shuffle2_8, [v4u16], 0, [{
    ushort8 tX;
    tX.s0123 = $Arg0VarName;
    tX.s4567 = $Arg1VarName;
    return shuffle(tX, $Arg2VarName);
  }]>;
// shuffle2 with mask size 8 - short8
OclBuiltinImpl shuffle2_8_v8u16 = OclBuiltinImpl<shuffle2_8, [v8u16], 0, [{
    $Arg2VarName = $Arg2VarName & ($Arg2Type)15;
    uchar16 short8_56 = as_uchar16(($Arg2Type)56);
    $Arg2Type t1 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName), short8_56) );
    $Arg2Type t2 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName - ($Arg2Type)8), short8_56) );
    $ReturnType res1 = _MM_SHUFFLE_EPI16($Arg0VarName, t1);
    $ReturnType res2 = _MM_SHUFFLE_EPI16($Arg1VarName, t2);
    return res1 | res2;
  }]>;
// shuffle2 with mask size 8 - short16
OclBuiltinImpl shuffle2_8_v16u16 = OclBuiltinImpl<shuffle2_8, [v16u16], 0, [{
    $Arg2VarName = $Arg2VarName & ($Arg2Type)31;

    uchar16 short8_56 = as_uchar16(($Arg2Type)56);
    $Arg2Type t1 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName), short8_56) );
    $Arg2Type t2 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName - ($Arg2Type)(8)), short8_56) );
    $Arg2Type t3 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName - ($Arg2Type)(16)), short8_56) );
    $Arg2Type t4 = as_$Arg2Type( add_sat(as_uchar16($Arg2VarName - ($Arg2Type)(24)), short8_56) );

    $ReturnType res1 = _MM_SHUFFLE_EPI16($Arg0VarName.lo, t1);
    $ReturnType res2 = _MM_SHUFFLE_EPI16($Arg0VarName.hi, t2);
    $ReturnType res3 = _MM_SHUFFLE_EPI16($Arg1VarName.lo, t3);
    $ReturnType res4 = _MM_SHUFFLE_EPI16($Arg1VarName.hi, t4);

    return res1 | res2 | res3 | res4;
  }]>;
// shuffle2 with mask size 8 - ints
OclBuiltinImpl shuffle2_8_i32 = OclBuiltinImpl<shuffle2_8, [v2u32, v4u32, v8u32, v16u32], 0, ExpandShuffle2To2Calls>;


/*****************************************************************\
 * shuffle2 with mask size 16                                    *
\*****************************************************************/
// shuffle2 with mask size 16 - char2 
OclBuiltinImpl shuffle2_16_v2u8 = OclBuiltinImpl<shuffle2_16, [v2u8], 0, [{
    uchar4 tX;
    tX.s01 = $Arg0VarName;
    tX.s23 = $Arg1VarName;
    return shuffle(tX, $Arg2VarName);
  }]>;
// shuffle2 with mask size 16 - char4 
OclBuiltinImpl shuffle2_16_v4u8 = OclBuiltinImpl<shuffle2_16, [v4u8], 0, [{
    uchar8 tX;
    tX.s0123 = $Arg0VarName;
    tX.s4567 = $Arg1VarName;
    return shuffle(tX, $Arg2VarName);
  }]>;
// shuffle2 with mask size 16 - char8 
OclBuiltinImpl shuffle2_16_v8u8 = OclBuiltinImpl<shuffle2_16, [v8u8], 0, [{
    uchar16 tX;
    tX.s01234567 = $Arg0VarName;
    tX.s89ABCDEF = $Arg1VarName;
    return shuffle(tX, $Arg2VarName);
  }]>;
// shuffle2 with mask size 16 - char16 
OclBuiltinImpl shuffle2_16_v16u8 = OclBuiltinImpl<shuffle2_16, [v16u8], 0, [{
    $Arg2Type mask1 = ($Arg2VarName + ($Arg2Type)112) & ($Arg2Type)0x8F;
    $Arg0VarName = _MM_SHUFFLE_EPI8($Arg0VarName, mask1);

    $Arg2Type mask2 = ($Arg2VarName - ($Arg2Type)16) & ($Arg2Type)0x8F;
    $Arg1VarName = _MM_SHUFFLE_EPI8($Arg1VarName, mask2);

    return $Arg0VarName | $Arg1VarName;
  }]>;
// shuffle2 with mask size 16 - shorts
OclBuiltinImpl shuffle2_16_i16 = OclBuiltinImpl<shuffle2_16, [v2u16, v4u16, v8u16, v16u16], 0, ExpandShuffle2To2Calls>;
// shuffle2 with mask size 16 - ints
OclBuiltinImpl shuffle2_16_i32 = OclBuiltinImpl<shuffle2_16, [v2u32, v4u32, v8u32, v16u32], 0, ExpandShuffle2To4Calls>;


OclBuiltinImpl _mem_fence = OclBuiltinImpl<mem_fence, [v1i64], 0, [{
    _mm_mfence();
  }]>;

OclBuiltinImpl _read_mem_fence = OclBuiltinImpl<read_mem_fence, [v1i64], 0, [{
    _mm_lfence();
  }]>;

OclBuiltinImpl _write_mem_fence = OclBuiltinImpl<write_mem_fence, [v1i64], 0, [{
    _mm_sfence();
  }]>;

