// Copyright (c) 2006 Intel Corporation
// Miscellaneous Built-In Functions
// Generic

// For shuffle() built-in we implement the unsigned version in the platform-optimized 
//   section and here we expand all other types to use the unsigned version
// For Shuffle2() built-in we expand to 2 calls of shuffle() for all types
// For example: shuffle2(double2 x, double2 y, ulong2 mask) would call 2 times
//                shuffle(double2 x, ulong2 mask) which in turn would call 
//                  shuffle(ulong2 x, ulong2 mask)


/*****************************************************************\
 * shuffle:                                                      *
 *   gentypen shuffle (gentypem x, ugentypen mask)               *
 *   The shuffle built-in function constructs a                  *
 *     permutation of elements from input vector, returning a    *
 *     vector with the same element type as the input and        *
 *     length that is the same as the shuffle mask.              *
\*****************************************************************/
list<OclType> ShuffleExpandTypes = [v2f32, v4f32, v8f32, v16f32,
                                    v2f64, v4f64, v8f64, v16f64,
                                     v2i8,  v4i8,  v8i8,  v16i8,
                                    v2i16, v4i16, v8i16, v16i16,
                                    v2i32, v4i32, v8i32, v16i32,
                                    v2i64, v4i64, v8i64, v16i64];
defvar ShuffleExpandToUnsignedInteger = [{
    return as_$ReturnType( shuffle(as_$Arg0ugentype($Arg0VarName), $Arg1VarName) );
  }];
// shuffle expand:
//             char => uchar, short => ushort, int => uint, long => ulong
//             float => uint, double => ulong
OclBuiltinImpl shuffle_2_expandTypes  = OclBuiltinImpl<shuffle_2,  ShuffleExpandTypes, 0, ShuffleExpandToUnsignedInteger>;
OclBuiltinImpl shuffle_4_expandTypes  = OclBuiltinImpl<shuffle_4,  ShuffleExpandTypes, 0, ShuffleExpandToUnsignedInteger>;
OclBuiltinImpl shuffle_8_expandTypes  = OclBuiltinImpl<shuffle_8,  ShuffleExpandTypes, 0, ShuffleExpandToUnsignedInteger>;
OclBuiltinImpl shuffle_16_expandTypes = OclBuiltinImpl<shuffle_16, ShuffleExpandTypes, 0, ShuffleExpandToUnsignedInteger>;


/*****************************************************************\
 * shuffle2:                                                     *
 *   gentypen shuffle2 (gentypem x, gentypem y, ugentypen mask)  *
 *   The shuffle2 built-in function constructs a                 *
 *     permutation of elements from two input vectors,           *
 *     returning a vector with the same element type as the      *
 *     input and length that is the same as the shuffle mask.    *
\*****************************************************************/
// expand the shuffle2() to 2 calls of shuffle() for all types
defvar ExpandShuffle2toShuffle = [{
    $Arg2VarName = $Arg2VarName & ($Arg2Type)($VecLength * 2 - 1);
    $ReturnType temp1 = shuffle($Arg0VarName, $Arg2VarName);
    $ReturnType temp2 = shuffle($Arg1VarName, $Arg2VarName);
    return ($Arg2VarName < ($Arg2Type)($VecLength)) ? temp1 : temp2;
  }];
OclBuiltinImpl shuffle2_2_all  = OclBuiltinImpl<shuffle2_2,  shuffle2_2.Types,  0, ExpandShuffle2toShuffle>;
OclBuiltinImpl shuffle2_4_all  = OclBuiltinImpl<shuffle2_4,  shuffle2_4.Types,  0, ExpandShuffle2toShuffle>;
OclBuiltinImpl shuffle2_8_all  = OclBuiltinImpl<shuffle2_8,  shuffle2_8.Types,  0, ExpandShuffle2toShuffle>;
OclBuiltinImpl shuffle2_16_all = OclBuiltinImpl<shuffle2_16, shuffle2_16.Types, 0, ExpandShuffle2toShuffle>;
