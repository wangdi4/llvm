// Copyright (c) 2006 Intel Corporation
// Integers Built-In Functions
// SSSE3

/*****************************************************************\
 * abs:                                                          *
 *   ugentype abs (gentype x)                                    *
 *   Returns | x |                                               *
\*****************************************************************/
// abs expand
OclBuiltinImpl abs_iexpand  = OclBuiltinImpl<abs, [v16i16, v8i32, v16i32], 0, Expand1>;

// abs on signed long vector
OclBuiltinImpl abs_i64 = OclBuiltinImpl<abs, [v8i64], 0, [{
    return as_$ReturnType(($Arg0VarName < 0) ? -$Arg0VarName : $Arg0VarName);
  }]>;

/*****************************************************************\
 * abs_diff:                                                     *
 *   ugentype abs_diff (gentype x, gentype y)                    *
 *   Returns | x - y |                                           *
\*****************************************************************/

// abs_diff on signed vector
OclBuiltinImpl abs_diff_svector = OclBuiltinImpl<abs_diff, [v16i16, v8i32, v4i64, v8i64, v16i32 ] , 0, [{
    $Arg0Type tmp1 = $Arg0VarName - $Arg1VarName;
    $Arg0Type tmp2 = $Arg1VarName - $Arg0VarName;
    $Arg0Type tmpc = $Arg1VarName > $Arg0VarName;
    $Arg0Type res = tmpc & tmp2;
    $Arg0Type reb = (~tmpc) & tmp1;
    return as_u$Arg0Type(res | reb);
  }]>;
// abs_diff on unsigned vector
OclBuiltinImpl abs_diff_uvector = OclBuiltinImpl<abs_diff, [v16u16, v8u32, v4u64, v8u64, v16u32] , 0, [{
    $Arg0Type tmp1 = $Arg0VarName - $Arg1VarName;
    $Arg0Type tmp2 = $Arg1VarName - $Arg0VarName;
    $Arg0Type tmpc = as_$Arg0Type($Arg1VarName > $Arg0VarName);
    $Arg0Type res = tmpc & tmp2;
    $Arg0Type reb = (~tmpc) & tmp1;
    return (res | reb);
  }]>;

/*****************************************************************\
 * add_sat:                                                      *
 *   gentype add_sat (gentype x, gentype y)                      *
 *   Returns x + y saturated                                     *
\*****************************************************************/

// add_sat expand
OclBuiltinImpl add_sat_sseexpand  = OclBuiltinImpl<add_sat, [v16i16, v16u16], 0, Expand2>;

/*****************************************************************\
 * sub_sat:                                                      *
 *   gentype sub_sat (gentype x, gentype y)                      *
 *   Returns x - y saturated                                     *
\*****************************************************************/

// sub_sat expand
OclBuiltinImpl sub_sat_sseexpand  = OclBuiltinImpl<sub_sat, [v16i16, v16u16], 0, Expand2>;





/*****************************************************************\
 * mad_sat:                                                      *
 *   gentype mad_sat (gentype x , gentype y , gentype z)         *
 *   Returns a * b + c and saturates the result.                 *
\*****************************************************************/

// mad_sat expand
OclBuiltinImpl mad_sat_iexpand  = OclBuiltinImpl<mad_sat, [ v8i32, v16u16, v8u32, v16i32, v16u32], 0, Expand3>;

// mad_sat native ushort8 (expanded from scalar, because any smaller vector size (2 & 4) is promoted to 8)
OclBuiltinImpl mad_sat_v8u16 = OclBuiltinImpl<mad_sat, [v8u16] , 0, [{
    ushort8 res;
    res.s0 = mad_sat($Arg0VarName.s0, $Arg1VarName.s0, $Arg2VarName.s0);
    res.s1 = mad_sat($Arg0VarName.s1, $Arg1VarName.s1, $Arg2VarName.s1);
    res.s2 = mad_sat($Arg0VarName.s2, $Arg1VarName.s2, $Arg2VarName.s2);
    res.s3 = mad_sat($Arg0VarName.s3, $Arg1VarName.s3, $Arg2VarName.s3);
    res.s4 = mad_sat($Arg0VarName.s4, $Arg1VarName.s4, $Arg2VarName.s4);
    res.s5 = mad_sat($Arg0VarName.s5, $Arg1VarName.s5, $Arg2VarName.s5);
    res.s6 = mad_sat($Arg0VarName.s6, $Arg1VarName.s6, $Arg2VarName.s6);
    res.s7 = mad_sat($Arg0VarName.s7, $Arg1VarName.s7, $Arg2VarName.s7);
    return res;
  }]>; 
// mad_sat native int4 (expanded from scalar, because any smaller vector size (2) is promoted to 4)
OclBuiltinImpl mad_sat_v4i32 = OclBuiltinImpl<mad_sat, [v4i32] , 0, [{
    int4 res;
    res.s0 = mad_sat($Arg0VarName.s0, $Arg1VarName.s0, $Arg2VarName.s0);
    res.s1 = mad_sat($Arg0VarName.s1, $Arg1VarName.s1, $Arg2VarName.s1);
    res.s2 = mad_sat($Arg0VarName.s2, $Arg1VarName.s2, $Arg2VarName.s2);
    res.s3 = mad_sat($Arg0VarName.s3, $Arg1VarName.s3, $Arg2VarName.s3);
    return res;
  }]>;
// mad_sat native uint4 (expanded from scalar, because any smaller vector size (2) is promoted to 4)
OclBuiltinImpl mad_sat_v4u32 = OclBuiltinImpl<mad_sat, [v4u32] , 0, [{
    uint4 res;
    res.s0 = mad_sat($Arg0VarName.s0, $Arg1VarName.s0, $Arg2VarName.s0);
    res.s1 = mad_sat($Arg0VarName.s1, $Arg1VarName.s1, $Arg2VarName.s1);
    res.s2 = mad_sat($Arg0VarName.s2, $Arg1VarName.s2, $Arg2VarName.s2);
    res.s3 = mad_sat($Arg0VarName.s3, $Arg1VarName.s3, $Arg2VarName.s3);
    return res;
  }]>;

/*****************************************************************\
 * mul_hi:                                                       *
 *   gentype mul_hi (gentype x , gentype y)                      *
 *   Returns the high half of the product of (x * y)             *
\*****************************************************************/

// mul_hi expand
OclBuiltinImpl mul_hi_iexpand  = OclBuiltinImpl<mul_hi, [ v16i16, v8i32, v16u16, v8u32, v4u64, v8i64, v8u64, v16u32, v16i32], 0, Expand2>;

// mul_hi native int4 (expanded from scalar, because any smaller vector size (2) is promoted to 4)
OclBuiltinImpl mul_hi_v4i32 = OclBuiltinImpl<mul_hi, [v4i32] , 0, [{
    $ReturnType res;
    res.s0 = mul_hi($Arg0VarName.s0, $Arg1VarName.s0);
    res.s1 = mul_hi($Arg0VarName.s1, $Arg1VarName.s1);
    res.s2 = mul_hi($Arg0VarName.s2, $Arg1VarName.s2);
    res.s3 = mul_hi($Arg0VarName.s3, $Arg1VarName.s3);
    return res;
  }]>;
// mul_hi native uint4 (expanded from scalar, because any smaller vector size (2) is promoted to 4)
OclBuiltinImpl mul_hi_v4u32 = OclBuiltinImpl<mul_hi, [v4u32] , 0, [{
    $ReturnType res;
    res.s0 = mul_hi($Arg0VarName.s0, $Arg1VarName.s0);
    res.s1 = mul_hi($Arg0VarName.s1, $Arg1VarName.s1);
    res.s2 = mul_hi($Arg0VarName.s2, $Arg1VarName.s2);
    res.s3 = mul_hi($Arg0VarName.s3, $Arg1VarName.s3);
    return res;
  }]>;
