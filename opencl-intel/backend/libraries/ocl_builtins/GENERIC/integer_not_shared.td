// Copyright (c) 2006 Intel Corporation
// Integers Built-In Functions
// Generic

// abs on signed long vector
OclBuiltinImpl abs_i64 = OclBuiltinImpl<abs, [v8i64], 0, [{
    return as_$ReturnType(($Arg0VarName < 0) ? -$Arg0VarName : $Arg0VarName);
  }]>;

/*****************************************************************\
 * abs_diff:                                                     *
 *   ugentype abs_diff (gentype x, gentype y)                    *
 *   Returns | x - y |                                           *
\*****************************************************************/
OclBuiltinImpl abs_diff_igeneric_not_shared = OclBuiltinImpl<abs_diff, [v16i16, v8i32, v4i64,
                     v16u16, v8u32, v4u64, v16i32, v16u32, v8i64, v8u64], 0, abs_diff_igeneric_code>;


/*****************************************************************\
 * add_sat:                                                      *
 *   gentype add_sat (gentype x, gentype y)                      *
 *   Returns x + y saturated                                     *
\*****************************************************************/

// add_sat vector int
OclBuiltinImpl add_sat_svector_not_shared = OclBuiltinImpl<add_sat, [v16i16] , 0, add_sat_svector_code>;
// add_sat on unsigned
OclBuiltinImpl add_sat_unsigned_not_shared = OclBuiltinImpl<add_sat, [v16u16] , 0, add_sat_unsigned_code>;





/*****************************************************************\
 * rhadd:                                                        *
 *   gentype rhadd (gentype x, gentype y)                        *
 *   Returns (x + y + 1) >> 1. The intermediate sum does         *
 *     not modulo overflow.                                      *
\*****************************************************************/
OclBuiltinImpl rhadd_igeneric_not_shared = OclBuiltinImpl<rhadd, [v16i32], 0, rhadd_igeneric_code>;


/*****************************************************************\
 * sub_sat:                                                      *
 *   gentype sub_sat (gentype x, gentype y)                      *
 *   Returns x - y saturated                                     *
\*****************************************************************/

// sub_sat on signed vector
OclBuiltinImpl sub_sat_svector_not_shared = OclBuiltinImpl<sub_sat, [v16i16] , 0, sub_sat_svector_code>;
// sub_sat on unsigned
OclBuiltinImpl sub_sat_unsigned_not_shared = OclBuiltinImpl<sub_sat, [v16u16] , 0, sub_sat_unsigned_code>;

