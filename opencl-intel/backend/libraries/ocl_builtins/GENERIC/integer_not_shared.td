// Copyright (c) 2006-2012 Intel Corporation
// Integers Built-In Functions
// Generic

/*****************************************************************\
 * abs_diff:                                                     *
 *   ugentype abs_diff (gentype x, gentype y)                    *
 *   Returns | x - y |                                           *
\*****************************************************************/
OclBuiltinImpl abs_diff_igeneric_not_shared = OclBuiltinImpl<abs_diff, [v16i16, v8i32, v4i64,
                                                             v16u16, v8u32, v4u64], 0, abs_diff_igeneric_code>;


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
 * max:                                                          *
 *   gentype max (gentype x , gentype y)                         *
 *   gentype max (gentype x , sgentype y)                        *
 *   Returns y if x < y, otherwise it returns x                  *
\*****************************************************************/

// max vector
OclBuiltinImpl max_ivector_not_shared = OclBuiltinImpl<max, [ v16i8, v8i16, v4i32, v16u8, v8u16, v4u32,
                                                   v16i16, v8i32, v16u16, v8u32], 0, max_ivector_code>;


/*****************************************************************\
 * min:                                                          *
 *   gentype min (gentype x , gentype y)                         *
 *   gentype min (gentype x , sgentype y)                        *
 *   Returns y if y < x, otherwise it returns x                  *
\*****************************************************************/

// min vectors
OclBuiltinImpl min_ivector_not_shared = OclBuiltinImpl<min, [ v16i8, v8i16, v4i32, v16u8, v8u16, v4u32,
                                                   v16i16, v8i32, v16u16, v8u32] , 0, min_ivector_code>;



/*****************************************************************\
 * sub_sat:                                                      *
 *   gentype sub_sat (gentype x, gentype y)                      *
 *   Returns x - y saturated                                     *
\*****************************************************************/

// sub_sat on signed vector
OclBuiltinImpl sub_sat_svector_not_shared = OclBuiltinImpl<sub_sat, [v16i16] , 0, sub_sat_svector_code>;
// sub_sat on unsigned
OclBuiltinImpl sub_sat_unsigned_not_shared = OclBuiltinImpl<sub_sat, [v16u16] , 0, sub_sat_unsigned_code>;

