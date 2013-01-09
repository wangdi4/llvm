// Copyright (c) 2006-2012 Intel Corporation
// Misc Built-In Functions
// AVX


/*****************************************************************\
 * shuffle:                                                      *
 *   gentypen shuffle (gentypem x, ugentypen mask)               *
 *   The shuffle built-in function constructs a                  *
 *     permutation of elements from input vector, returning a    *
 *     vector with the same element type as the input and        *
 *     length that is the same as the shuffle mask.              *
\*****************************************************************/
// shuffle with mask size 2 - int2
OclBuiltinImpl shuffle_2_v2u32 = OclBuiltinImpl<shuffle_2, [v2u32], 0, [{
    uint4 tMask;
    uint4 tx;
    tMask.s01 = $Arg1VarName;
    tx.s01 = $Arg0VarName;
    return shuffle(tx, tMask).s01;
  }]>;
// shuffle with mask size 4 - int2
OclBuiltinImpl shuffle_4_v2u32 = OclBuiltinImpl<shuffle_4, [v2u32], 0, [{
    uint4 tx;
    tx.s01 = $Arg0VarName;
    return shuffle(tx, $Arg1VarName);
  }]>;
// shuffle with mask size 4 - int4
OclBuiltinImpl shuffle_4_v4u32 = OclBuiltinImpl<shuffle_4, [v4u32], 0, [{
    return as_$ReturnType( _mm_permutevar_ps((__m128)$Arg0VarName,(__m128i)$Arg1VarName) );
  }]>;
// shuffle with mask size 16 - int2
OclBuiltinImpl shuffle_16_v2u32 = OclBuiltinImpl<shuffle_16, [v2u32], 0, [{
    uint8 tx;
    $ReturnType res;
    tx.s01 = $Arg0VarName;
    res.lo = shuffle(tx, $Arg1VarName.lo);
    res.hi = shuffle(tx, $Arg1VarName.hi);
    return res;
  }]>;


/*****************************************************************\
 * shuffle2:                                                     *
 *   gentypen shuffle2 (gentypem x, gentypem y, ugentypen mask)  *
 *   The shuffle2 built-in function constructs a                 *
 *     permutation of elements from two input vectors,           *
 *     returning a vector with the same element type as the      *
 *     input and length that is the same as the shuffle mask.    *
\*****************************************************************/
// shuffle2 with mask size 4 - long2
OclBuiltinImpl shuffle2_4_v2u64 = OclBuiltinImpl<shuffle2_4, [v2u64], 0, [{
    return shuffle((ulong4)($Arg0VarName, $Arg1VarName), $Arg2VarName);
  }]>;
// shuffle2 with mask size 8 - int2
OclBuiltinImpl shuffle2_8_v2u32 = OclBuiltinImpl<shuffle2_8, [v2u32], 0, [{
    return shuffle((uint4)($Arg0VarName, $Arg1VarName), $Arg2VarName);
  }]>;
// shuffle2 with mask size 8 - int4
OclBuiltinImpl shuffle2_8_v4u32 = OclBuiltinImpl<shuffle2_8, [v4u32], 0, [{
    return shuffle((uint8)($Arg0VarName, $Arg1VarName), $Arg2VarName);
  }]>;
// shuffle2 with mask size 8 - long2
OclBuiltinImpl shuffle2_8_v2u64 = OclBuiltinImpl<shuffle2_8, [v2u64], 0, [{
    $ReturnType res;
    res.lo = shuffle((ulong4)($Arg0VarName, $Arg1VarName), $Arg2VarName.lo);
    res.hi = shuffle((ulong4)($Arg0VarName, $Arg1VarName), $Arg2VarName.hi);
    return res;
  }]>;
// shuffle2 with mask size 16 - int2
OclBuiltinImpl shuffle2_16_v2u32 = OclBuiltinImpl<shuffle2_16, [v2u32], 0, [{
    $ReturnType res;
    res.lo = shuffle((uint4)($Arg0VarName, $Arg1VarName), $Arg2VarName.lo);
    res.hi = shuffle((uint4)($Arg0VarName, $Arg1VarName), $Arg2VarName.hi);
    return res;
  }]>;
// shuffle2 with mask size 16 - int4
OclBuiltinImpl shuffle2_16_v4u32 = OclBuiltinImpl<shuffle2_16, [v4u32], 0, [{
    $ReturnType res;
    res.lo = shuffle((uint8)($Arg0VarName, $Arg1VarName), $Arg2VarName.lo);
    res.hi = shuffle((uint8)($Arg0VarName, $Arg1VarName), $Arg2VarName.hi);
    return res;
  }]>;
// shuffle2 with mask size 16 - long2
OclBuiltinImpl shuffle2_16_v2u64 = OclBuiltinImpl<shuffle2_16, [v2u64], 0, [{
    $ReturnType res;
    res.lo.lo = shuffle((ulong4)($Arg0VarName, $Arg1VarName), $Arg2VarName.lo.lo);
    res.lo.hi = shuffle((ulong4)($Arg0VarName, $Arg1VarName), $Arg2VarName.lo.hi);
    res.hi.lo = shuffle((ulong4)($Arg0VarName, $Arg1VarName), $Arg2VarName.hi.lo);
    res.hi.hi = shuffle((ulong4)($Arg0VarName, $Arg1VarName), $Arg2VarName.hi.hi);
    return res;
  }]>;

