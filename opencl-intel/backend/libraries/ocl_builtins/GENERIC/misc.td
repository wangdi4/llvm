// Copyright (c) 2006-2012 Intel Corporation
// Misc Built-In Functions
// Generic

code ShuffleFPToInteger = [{
    $Arg0igentype i$Arg0VarName = as_$Arg0igentype($Arg0VarName);
    return as_$ReturnType( shuffle(i$Arg0VarName, $Arg1VarName) );
  }];

code ShuffleSignedToUnsigned = [{
    return as_$ReturnType( shuffle( as_u$Arg0Type($Arg0VarName), $Arg1VarName ));
  }];

code Shuffle2FPToInteger = [{
    $Arg0igentype i$Arg0VarName = as_$Arg0igentype($Arg0VarName);
    $Arg1igentype i$Arg1VarName = as_$Arg1igentype($Arg1VarName);
    return as_$ReturnType( shuffle2(i$Arg0VarName, i$Arg1VarName, $Arg2VarName) );
  }];

code Shuffle2SignedToUnsigned = [{
    return as_$ReturnType( shuffle2( as_u$Arg0Type($Arg0VarName),
                                     as_u$Arg1Type($Arg1VarName), $Arg2VarName ));
  }];

list<OclType> ShuffleFPTypes = [v2f32, v4f32, v8f32, v16f32,
                                v2f64, v4f64, v8f64, v16f64];

list<OclType> ShuffleSignedTypes = [ v2i8,  v4i8,  v8i8,  v16i8,
                                    v2i16, v4i16, v8i16, v16i16,
                                    v2i32, v4i32, v8i32, v16i32,
                                    v2i64, v4i64, v8i64, v16i64];


/*****************************************************************\
 * shuffle:                                                      *
 *   gentypen shuffle (gentypem x, ugentypen mask)               *
 *   The shuffle built-in function constructs a                  *
 *     permutation of elements from input vector, returning a    *
 *     vector with the same element type as the input and        *
 *     length that is the same as the shuffle mask.              *
\*****************************************************************/
// shuffle with mask size 2 - longs
OclBuiltinImpl shuffle_2_i64 = OclBuiltinImpl<shuffle_2, [v2u64, v4u64, v8u64, v16u64], 0, [{
    $ReturnType res;
    $Arg0BaseType xVec[$VecLength];
    memcpy(xVec, &$Arg0VarName, sizeof($Arg0Type));
    res.s0 = xVec[$Arg1VarName.s0 & ($VecLength - 1)];
    res.s1 = xVec[$Arg1VarName.s1 & ($VecLength - 1)];
    return res;
  }]>;
// shuffle with mask size 4 - longs
OclBuiltinImpl shuffle_4_i64 = OclBuiltinImpl<shuffle_4, [v2u64, v4u64, v8u64, v16u64], 0, [{
    $ReturnType res;
    $Arg0BaseType xVec[$VecLength];
    memcpy(xVec, &$Arg0VarName, sizeof($Arg0Type));
    res.s0 = xVec[$Arg1VarName.s0 & ($VecLength - 1)];
    res.s1 = xVec[$Arg1VarName.s1 & ($VecLength - 1)];
    res.s2 = xVec[$Arg1VarName.s2 & ($VecLength - 1)];
    res.s3 = xVec[$Arg1VarName.s3 & ($VecLength - 1)];
    return res;
  }]>;
// shuffle with mask size 8 - longs
OclBuiltinImpl shuffle_8_i64 = OclBuiltinImpl<shuffle_8, [v2u64, v4u64, v8u64, v16u64], 0, [{
    $ReturnType res;
    $Arg0BaseType xVec[$VecLength];
    memcpy(xVec, &$Arg0VarName, sizeof($Arg0Type));
    res.s0 = xVec[$Arg1VarName.s0 & ($VecLength - 1)];
    res.s1 = xVec[$Arg1VarName.s1 & ($VecLength - 1)];
    res.s2 = xVec[$Arg1VarName.s2 & ($VecLength - 1)];
    res.s3 = xVec[$Arg1VarName.s3 & ($VecLength - 1)];
    res.s4 = xVec[$Arg1VarName.s4 & ($VecLength - 1)];
    res.s5 = xVec[$Arg1VarName.s5 & ($VecLength - 1)];
    res.s6 = xVec[$Arg1VarName.s6 & ($VecLength - 1)];
    res.s7 = xVec[$Arg1VarName.s7 & ($VecLength - 1)];
    return res;
  }]>;
// shuffle with mask size 16 - longs
OclBuiltinImpl shuffle_16_i64 = OclBuiltinImpl<shuffle_16, [v2u64, v4u64, v8u64, v16u64], 0, [{
    $ReturnType res;
    $Arg0BaseType xVec[$VecLength];
    memcpy(xVec, &$Arg0VarName, sizeof($Arg0Type));
    res.s0 = xVec[$Arg1VarName.s0 & ($VecLength - 1)];
    res.s1 = xVec[$Arg1VarName.s1 & ($VecLength - 1)];
    res.s2 = xVec[$Arg1VarName.s2 & ($VecLength - 1)];
    res.s3 = xVec[$Arg1VarName.s3 & ($VecLength - 1)];
    res.s4 = xVec[$Arg1VarName.s4 & ($VecLength - 1)];
    res.s5 = xVec[$Arg1VarName.s5 & ($VecLength - 1)];
    res.s6 = xVec[$Arg1VarName.s6 & ($VecLength - 1)];
    res.s7 = xVec[$Arg1VarName.s7 & ($VecLength - 1)];
    res.s8 = xVec[$Arg1VarName.s8 & ($VecLength - 1)];
    res.s9 = xVec[$Arg1VarName.s9 & ($VecLength - 1)];
    res.sA = xVec[$Arg1VarName.sA & ($VecLength - 1)];
    res.sB = xVec[$Arg1VarName.sB & ($VecLength - 1)];
    res.sC = xVec[$Arg1VarName.sC & ($VecLength - 1)];
    res.sD = xVec[$Arg1VarName.sD & ($VecLength - 1)];
    res.sE = xVec[$Arg1VarName.sE & ($VecLength - 1)];
    res.sF = xVec[$Arg1VarName.sF & ($VecLength - 1)];
    return res;
  }]>;
// shuffle on floats and doubles => use int and long
OclBuiltinImpl shuffle_2_fp  = OclBuiltinImpl<shuffle_2,  ShuffleFPTypes, 0, ShuffleFPToInteger>;
OclBuiltinImpl shuffle_4_fp  = OclBuiltinImpl<shuffle_4,  ShuffleFPTypes, 0, ShuffleFPToInteger>;
OclBuiltinImpl shuffle_8_fp  = OclBuiltinImpl<shuffle_8,  ShuffleFPTypes, 0, ShuffleFPToInteger>;
OclBuiltinImpl shuffle_16_fp = OclBuiltinImpl<shuffle_16, ShuffleFPTypes, 0, ShuffleFPToInteger>;
// shuffle on signed integers => use unsigned
OclBuiltinImpl shuffle_2_signed  = OclBuiltinImpl<shuffle_2,  ShuffleSignedTypes, 0, ShuffleSignedToUnsigned>;
OclBuiltinImpl shuffle_4_signed  = OclBuiltinImpl<shuffle_4,  ShuffleSignedTypes, 0, ShuffleSignedToUnsigned>;
OclBuiltinImpl shuffle_8_signed  = OclBuiltinImpl<shuffle_8,  ShuffleSignedTypes, 0, ShuffleSignedToUnsigned>;
OclBuiltinImpl shuffle_16_signed = OclBuiltinImpl<shuffle_16, ShuffleSignedTypes, 0, ShuffleSignedToUnsigned>;


/*****************************************************************\
 * shuffle2:                                                     *
 *   gentypen shuffle2 (gentypem x, gentypem y, ugentypen mask)  *
 *   The shuffle2 built-in function constructs a                 *
 *     permutation of elements from two input vectors,           *
 *     returning a vector with the same element type as the      *
 *     input and length that is the same as the shuffle mask.    *
\*****************************************************************/
// shuffle2 with mask size 2 - longs
OclBuiltinImpl shuffle2_2_i64 = OclBuiltinImpl<shuffle2_2, [v2u64, v4u64, v8u64, v16u64], 0, [{
    $ReturnType res;
    $Arg0BaseType xVec[$VecLength * 2];
    memcpy(&xVec[0], &$Arg0VarName, sizeof($Arg0Type));
    memcpy(&xVec[$VecLength], &$Arg1VarName, sizeof($Arg1Type));
    res.s0 = xVec[$Arg2VarName.s0 & ($VecLength * 2 - 1)];
    res.s1 = xVec[$Arg2VarName.s1 & ($VecLength * 2 - 1)];
    return res;
  }]>;
// shuffle2 with mask size 4 - longs
OclBuiltinImpl shuffle2_4_i64 = OclBuiltinImpl<shuffle2_4, [v2u64, v4u64, v8u64, v16u64], 0, [{
    $ReturnType res;
    $Arg0BaseType xVec[$VecLength * 2];
    memcpy(&xVec[0], &$Arg0VarName, sizeof($Arg0Type));
    memcpy(&xVec[$VecLength], &$Arg1VarName, sizeof($Arg1Type));
    res.s0 = xVec[$Arg2VarName.s0 & ($VecLength * 2 - 1)];
    res.s1 = xVec[$Arg2VarName.s1 & ($VecLength * 2 - 1)];
    res.s2 = xVec[$Arg2VarName.s2 & ($VecLength * 2 - 1)];
    res.s3 = xVec[$Arg2VarName.s3 & ($VecLength * 2 - 1)];
    return res;
  }]>;
// shuffle2 with mask size 8 - longs
OclBuiltinImpl shuffle2_8_i64 = OclBuiltinImpl<shuffle2_8, [v2u64, v4u64, v8u64, v16u64], 0, [{
    $ReturnType res;
    $Arg0BaseType xVec[$VecLength * 2];
    memcpy(&xVec[0], &$Arg0VarName, sizeof($Arg0Type));
    memcpy(&xVec[$VecLength], &$Arg1VarName, sizeof($Arg1Type));
    res.s0 = xVec[$Arg2VarName.s0 & ($VecLength * 2 - 1)];
    res.s1 = xVec[$Arg2VarName.s1 & ($VecLength * 2 - 1)];
    res.s2 = xVec[$Arg2VarName.s2 & ($VecLength * 2 - 1)];
    res.s3 = xVec[$Arg2VarName.s3 & ($VecLength * 2 - 1)];
    res.s4 = xVec[$Arg2VarName.s4 & ($VecLength * 2 - 1)];
    res.s5 = xVec[$Arg2VarName.s5 & ($VecLength * 2 - 1)];
    res.s6 = xVec[$Arg2VarName.s6 & ($VecLength * 2 - 1)];
    res.s7 = xVec[$Arg2VarName.s7 & ($VecLength * 2 - 1)];
    return res;
  }]>;
// shuffle2 with mask size 16 - longs
OclBuiltinImpl shuffle2_16_i64 = OclBuiltinImpl<shuffle2_16, [v2u64, v4u64, v8u64, v16u64], 0, [{
    $ReturnType res;
    $Arg0BaseType xVec[$VecLength * 2];
    memcpy(&xVec[0], &$Arg0VarName, sizeof($Arg0Type));
    memcpy(&xVec[$VecLength], &$Arg1VarName, sizeof($Arg1Type));
    res.s0 = xVec[$Arg2VarName.s0 & ($VecLength * 2 - 1)];
    res.s1 = xVec[$Arg2VarName.s1 & ($VecLength * 2 - 1)];
    res.s2 = xVec[$Arg2VarName.s2 & ($VecLength * 2 - 1)];
    res.s3 = xVec[$Arg2VarName.s3 & ($VecLength * 2 - 1)];
    res.s4 = xVec[$Arg2VarName.s4 & ($VecLength * 2 - 1)];
    res.s5 = xVec[$Arg2VarName.s5 & ($VecLength * 2 - 1)];
    res.s6 = xVec[$Arg2VarName.s6 & ($VecLength * 2 - 1)];
    res.s7 = xVec[$Arg2VarName.s7 & ($VecLength * 2 - 1)];
    res.s8 = xVec[$Arg2VarName.s8 & ($VecLength * 2 - 1)];
    res.s9 = xVec[$Arg2VarName.s9 & ($VecLength * 2 - 1)];
    res.sA = xVec[$Arg2VarName.sA & ($VecLength * 2 - 1)];
    res.sB = xVec[$Arg2VarName.sB & ($VecLength * 2 - 1)];
    res.sC = xVec[$Arg2VarName.sC & ($VecLength * 2 - 1)];
    res.sD = xVec[$Arg2VarName.sD & ($VecLength * 2 - 1)];
    res.sE = xVec[$Arg2VarName.sE & ($VecLength * 2 - 1)];
    res.sF = xVec[$Arg2VarName.sF & ($VecLength * 2 - 1)];
    return res;
  }]>;
// shuffle2 on floats and doubles => use int and long
OclBuiltinImpl shuffle2_2_fp  = OclBuiltinImpl<shuffle2_2,  ShuffleFPTypes, 0, Shuffle2FPToInteger>;
OclBuiltinImpl shuffle2_4_fp  = OclBuiltinImpl<shuffle2_4,  ShuffleFPTypes, 0, Shuffle2FPToInteger>;
OclBuiltinImpl shuffle2_8_fp  = OclBuiltinImpl<shuffle2_8,  ShuffleFPTypes, 0, Shuffle2FPToInteger>;
OclBuiltinImpl shuffle2_16_fp = OclBuiltinImpl<shuffle2_16, ShuffleFPTypes, 0, Shuffle2FPToInteger>;
// shuffle2 on signed integers => use unsigned
OclBuiltinImpl shuffle2_2_signed  = OclBuiltinImpl<shuffle2_2,  ShuffleSignedTypes, 0, Shuffle2SignedToUnsigned>;
OclBuiltinImpl shuffle2_4_signed  = OclBuiltinImpl<shuffle2_4,  ShuffleSignedTypes, 0, Shuffle2SignedToUnsigned>;
OclBuiltinImpl shuffle2_8_signed  = OclBuiltinImpl<shuffle2_8,  ShuffleSignedTypes, 0, Shuffle2SignedToUnsigned>;
OclBuiltinImpl shuffle2_16_signed = OclBuiltinImpl<shuffle2_16, ShuffleSignedTypes, 0, Shuffle2SignedToUnsigned>;

