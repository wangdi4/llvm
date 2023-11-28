// Copyright (c) 2006 Intel Corporation
// Integers Built-In Functions
// AVX

/*****************************************************************\
 * abs_diff:                                                     *
 *   ugentype abs_diff (gentype x, gentype y)                    *
 *   Returns | x - y |                                           *
\*****************************************************************/
// fix for AVX regressions
OclBuiltinImpl abs_diff_iavx = OclBuiltinImpl<abs_diff, [v16i16, v8i32, v4i64,
                                                        v16u16, v8u32, v4u64], 0, [{
    return as_$ReturnType(($Arg0Type)(max($Arg0VarName, $Arg1VarName) - min($Arg0VarName, $Arg1VarName)));
  }]>;
