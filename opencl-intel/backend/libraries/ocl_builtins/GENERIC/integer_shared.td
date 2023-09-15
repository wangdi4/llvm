// Copyright (c) 2006 Intel Corporation
// Integers Built-In Functions
// Generic

/*****************************************************************\
 * abs:                                                          *
 *   ugentype abs (gentype x)                                    *
 *   Returns | x |                                               *
\*****************************************************************/
// abs on signed scalar
OclBuiltinImpl abs_iscalar = OclBuiltinImpl<abs, [v1i8, v1i16, v1i32, v1i64], 0, [{
    return ($Arg0VarName < 0) ? -$Arg0VarName : $Arg0VarName;
  }]>;
// abs on signed long vector
OclBuiltinImpl abs_i64 = OclBuiltinImpl<abs, [v2i64, v3i64, v4i64, v16i64], 0, [{
    return as_$ReturnType(($Arg0VarName < 0) ? -$Arg0VarName : $Arg0VarName);
  }]>;
// abs on unsigned integer
OclBuiltinImpl abs_ugeneric = OclBuiltinImpl<abs, [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                                   v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                                                   v1u64, v2u64, v3u64, v4u64, v8u64, v16u64], 0, [{
    return $Arg0VarName;
  }]>;


/*****************************************************************\
 * abs_diff:                                                     *
 *   ugentype abs_diff (gentype x, gentype y)                    *
 *   Returns | x - y |                                           *
\*****************************************************************/
list<OclType> abs_diff_shared_types = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                        v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                        v1i16, v2i16, v3i16, v4i16, v8i16,
                                        v1u16, v2u16, v3u16, v4u16, v8u16,
                                        v1i32, v2i32, v3i32, v4i32, 
                                        v1u32, v2u32, v3u32, v4u32,
                                        v1i64, v2i64, v3i64, v16i64,
                                        v1u64, v2u64, v3u64, v16u64];
OclBuiltinImpl abs_diff_igeneric = OclBuiltinImpl<abs_diff, abs_diff_shared_types, 0, abs_diff_igeneric_code>;


/*****************************************************************\
 * add_sat:                                                      *
 *   gentype add_sat (gentype x, gentype y)                      *
 *   Returns x + y saturated                                     *
\*****************************************************************/
// add_sat on scalar signed
OclBuiltinImpl add_sat_v1i = OclBuiltinImpl<add_sat, [v1i8, v1i16, v1i32, v1i64], 0, [{
    $ReturnType r = $Arg0VarName + $Arg1VarName;
    if( $Arg1VarName > 0 )
    {
      if( r < $Arg0VarName )
        r = generic_max_$Arg0Type;
    }
    else
    {
      if( r > $Arg0VarName )
        r = generic_min_$Arg0Type;
    }
    return r;
  }]>;
// add_sat vector int
OclBuiltinImpl add_sat_svector = OclBuiltinImpl<add_sat, [ v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                          v2i16, v3i16, v4i16, v8i16,
                                                          v2i32, v3i32, v4i32, v8i32, v16i32,
                                                          v2i64, v3i64, v4i64, v8i64, v16i64] , 0, add_sat_svector_code>;
// add_sat on unsigned
OclBuiltinImpl add_sat_unsigned = OclBuiltinImpl<add_sat, [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                           v1u16, v2u16, v3u16, v4u16, v8u16,
                                                           v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                                                           v1u64, v2u64, v3u64, v4u64, v8u64, v16u64] , 0, add_sat_unsigned_code>;


/*****************************************************************\
 * hadd:                                                         *
 *   gentype hadd (gentype x, gentype y)                         *
 *   Returns (x + y) >> 1. The intermediate sum does             *
 *     not modulo overflow.                                      *
\*****************************************************************/
OclBuiltinImpl hadd_igeneric = OclBuiltinImpl<hadd, hadd.Types, 0, [{
    $ReturnType s, c;
    s = $Arg0VarName ^ $Arg1VarName;
    c = $Arg0VarName & $Arg1VarName;
    return (s >> (($ReturnType)1)) + c;
  }]>;


/*****************************************************************\
 * rhadd:                                                        *
 *   gentype rhadd (gentype x, gentype y)                        *
 *   Returns (x + y + 1) >> 1. The intermediate sum does         *
 *     not modulo overflow.                                      *
\*****************************************************************/
list<OclType> abs_rhadd_shared_types = [v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                        v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                        v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                        v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                        v1i32, v2i32, v3i32, v4i32, v8i32,
                                        v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                                        v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
                                        v1u64, v2u64, v3u64, v4u64, v8u64, v16u64];
OclBuiltinImpl rhadd_igeneric = OclBuiltinImpl<rhadd, abs_rhadd_shared_types, 0, rhadd_igeneric_code>;


/*****************************************************************\
 * clamp:                                                        *
 *   gentype clamp (gentype x, gentype minval, gentype maxval)   *
 *   gentype clamp (gentype x, sgentype minval, sgentype maxval) *
 *   Returns min(max(x, minval), maxval).                        *
\*****************************************************************/
OclBuiltinImpl clamp_igeneric = OclBuiltinImpl<clamp,
  [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
    v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
   v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
   v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
   v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
   v1u64, v2u64, v3u64, v4u64, v8u64, v16u64], 0, [{
    return max(min($Arg0VarName, $Arg2VarName), $Arg1VarName);
  }]>;


/*****************************************************************\
 * clz:                                                          *
 *   gentype clz (gentype x)                                     *
 *   Returns the number of leading 0-bits in x, starting at      *
 *     the most significant bit position.                        *
\*****************************************************************/
// clz on signed integer - call unsigned
OclBuiltinImpl clz_igeneric = OclBuiltinImpl<clz, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                   v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                                   v1i64, v2i64, v3i64, v4i64, v8i64, v16i64], 0, [{
    return as_$ReturnType(clz(as_u$Arg0Type($Arg0VarName)));
  }]>;
// clz on unsigned integer for all vectors but 3 demensional
OclBuiltinImpl clz_ugeneric = OclBuiltinImpl<clz, [ v1u8,  v2u8,  v4u8,  v8u8,  v16u8,
                                                   v1u16, v2u16, v4u16, v8u16, v16u16,
                                                   v1u32, v2u32, v4u32, v8u32, v16u32,
                                                   v1u64, v2u64, v4u64, v8u64, v16u64], 0, [{
    return __ocl_helper_clz_$Arg0Symgentype($Arg0VarName);
  }]>;
// clz on unsigned 3 demensional vector promoted to 4 demensional
OclBuiltinImpl clz_promote = OclBuiltinImpl<clz, [v3u8, v3u16, v3u32, v3u64], 0, Promote1_v3_to_v4>;


/*****************************************************************\
 * mad_sat:                                                      *
 *   gentype mad_sat (gentype x , gentype y , gentype z)         *
 *   Returns a * b + c and saturates the result.                 *
\*****************************************************************/
// mad_sat on scalar char
OclBuiltinImpl mad_sat_v1i8 = OclBuiltinImpl<mad_sat, [v1i8], 0, [{
    short res;
    res = ( (short) $Arg0VarName * (short) $Arg1VarName ) + $Arg2VarName;
    res = (res > generic_min_char) ? res : generic_min_char;
    res = (res < generic_max_char) ? res : generic_max_char;
    return ($ReturnType)res;
  }]>;
// mad_sat on scalar uchar
OclBuiltinImpl mad_sat_v1u8 = OclBuiltinImpl<mad_sat, [v1u8], 0, [{
    ushort res;
    res = ( (ushort) $Arg0VarName * (ushort) $Arg1VarName ) + $Arg2VarName;
    res = (res < generic_max_uchar) ? res : generic_max_uchar;
    return ($ReturnType)res;
  }]>;
// mad_sat on scalar short
OclBuiltinImpl mad_sat_v1i16 = OclBuiltinImpl<mad_sat, [v1i16], 0, [{
    int res;
    res = ( (int) $Arg0VarName * (int) $Arg1VarName ) + $Arg2VarName;
    res = (res > generic_min_short) ? res : generic_min_short;
    res = (res < generic_max_short) ? res : generic_max_short;
    return ($ReturnType)res;
  }]>;
// mad_sat on scalar ushort
OclBuiltinImpl mad_sat_v1u16 = OclBuiltinImpl<mad_sat, [v1u16], 0, [{
    uint res;
    res = ( (uint) $Arg0VarName * (uint) $Arg1VarName ) + $Arg2VarName;
    res = (res < generic_max_ushort) ? res : generic_max_ushort;
    return ($ReturnType)res;
  }]>;
// mad_sat on scalar int
OclBuiltinImpl mad_sat_v1i32 = OclBuiltinImpl<mad_sat, [v1i32], 0, [{
    long res;
    res = ( (long) $Arg0VarName * (long) $Arg1VarName ) + $Arg2VarName;
    res = (res > generic_min_int) ? res : generic_min_int;
    res = (res < generic_max_int) ? res : generic_max_int;
    return ($ReturnType)res;
  }]>;
// mad_sat on scalar uint
OclBuiltinImpl mad_sat_v1u32 = OclBuiltinImpl<mad_sat, [v1u32], 0, [{
    ulong res;
    res = ( (ulong) $Arg0VarName * (ulong) $Arg1VarName ) + $Arg2VarName;
    res = (res < generic_max_uint) ? res : generic_max_uint;
    return ($ReturnType)res;
  }]>;
// mad_sat on scalar long
OclBuiltinImpl mad_sat_v1i64 = OclBuiltinImpl<mad_sat, [v1i64], 0, [{
    long multHi;
    ulong multLo;
    multiply_signed_64_by_64($Arg0VarName, $Arg1VarName, &multLo, (ulong*)&multHi);
    ulong sum = multLo + $Arg2VarName;
    // carry if overflow
    if( $Arg2VarName >= 0 )
    {
      if( multLo > sum )
      {
        multHi++;
        if( generic_min_long == multHi )
        {
          multHi = generic_max_long;
          sum = generic_max_ulong;
        }
      }
    }
    else
    {
      if( multLo < sum )
      {
        multHi--;
        if( generic_max_long == multHi )
        {
          multHi = generic_min_long;
          sum = 0;
        }
      }
    }

    // saturate
    if( multHi > 0 )
      sum = generic_max_long;
    else if( multHi < -1 )
      sum = generic_min_long;

    return sum;
  }]>;
// mad_sat on scalar ulong
OclBuiltinImpl mad_sat_v1u64 = OclBuiltinImpl<mad_sat, [v1u64], 0, [{
    ulong multHi, multLo;
    multiply_unsigned_64_by_64($Arg0VarName, $Arg1VarName, &multLo, &multHi);
    multLo += $Arg2VarName;
    multHi += multLo < $Arg2VarName;  // carry if overflow
    if( multHi )
      multLo = 0xFFFFFFFFFFFFFFFFUL;

    return multLo;
  }]>;


/*****************************************************************\
 * mad_hi:                                                       *
 *   gentype mad_hi (gentype a, gentype b, gentype c)            *
 *   Returns mul_hi(a, b) + c.                                   *
\*****************************************************************/
OclBuiltinImpl mad_hi_igeneric = OclBuiltinImpl<mad_hi, mad_hi.Types, 0, [{
    return mul_hi($Arg0VarName, $Arg1VarName) + $Arg2VarName;
  }]>;


/*****************************************************************\
 * mul_hi                                                        *
 *   gentype mul_hi (gentype x , gentype y)                      *
 *   Returns the high half of the product of (x * y)             *
\*****************************************************************/
// mul_hi on scalars
OclBuiltinImpl mul_hi_v1i8 = OclBuiltinImpl<mul_hi, [v1i8], 0, [{
    return ((short)$Arg0VarName * $Arg1VarName) >> 8;
  }]>;
OclBuiltinImpl mul_hi_v1u8 = OclBuiltinImpl<mul_hi, [v1u8], 0, [{
    return ((ushort)$Arg0VarName * $Arg1VarName) >> 8;
  }]>;
OclBuiltinImpl mul_hi_v1i16 = OclBuiltinImpl<mul_hi, [v1i16], 0, [{
    return ((int)$Arg0VarName * $Arg1VarName) >> 16;
  }]>;
OclBuiltinImpl mul_hi_v1u16 = OclBuiltinImpl<mul_hi, [v1u16], 0, [{
    return ((uint)$Arg0VarName * $Arg1VarName) >> 16;
  }]>;
OclBuiltinImpl mul_hi_v1i32 = OclBuiltinImpl<mul_hi, [v1i32], 0, [{
    return ((long)$Arg0VarName * $Arg1VarName) >> 32;
  }]>;
OclBuiltinImpl mul_hi_v1u32 = OclBuiltinImpl<mul_hi, [v1u32], 0, [{
    return ((ulong)$Arg0VarName * $Arg1VarName) >> 32;
  }]>;
OclBuiltinImpl mul_hi_v1i64 = OclBuiltinImpl<mul_hi, [v1i64], 0, [{
    ulong destLow;
    ulong destHi;
    multiply_signed_64_by_64($Arg0VarName, $Arg1VarName, &destLow, &destHi);
    return destHi;
  }]>;
OclBuiltinImpl mul_hi_v1u64 = OclBuiltinImpl<mul_hi, [v1u64], 0, [{
    ulong destLow;
    ulong destHi;
    multiply_unsigned_64_by_64($Arg0VarName, $Arg1VarName, &destLow, &destHi);
    return destHi;
  }]>;


/*****************************************************************\
 * rotate:                                                       *
 *   gentype rotate (gentype v , gentype i)                      *
 *   For each element in v, the bits are shifted left by the     *
 *   number of bits given by the corresponding element in i      *
 *   (subject to usual shift modulo rules described in           *
 *   section 6.3). Bits shifted off the left side of the element *
 *   are shifted back in from the right.                         *
\*****************************************************************/
// rotate on signed integer - call unsigned
OclBuiltinImpl rotate_igeneric = OclBuiltinImpl<rotate, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                         v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                         v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                                         v1i64, v2i64, v3i64, v4i64, v8i64, v16i64], 0, [{
    return as_$ReturnType(rotate(as_u$Arg0Type($Arg0VarName), as_u$Arg1Type($Arg1VarName)));
  }]>;
// rotate on unsigned scalars
OclBuiltinImpl rotate_v1u = OclBuiltinImpl<rotate, [v1u8, v1u16, v1u32, v1u64], 0, [{
        $Arg1VarName &= ($BitWidth - 1);
        return ($Arg0Type)( $Arg0VarName << $Arg1VarName ) | $Arg0VarName >> ( $BitWidth - $Arg1VarName );
  }]>;
// rotate on unsigned vectors
OclBuiltinImpl rotate_uvectors = OclBuiltinImpl<rotate, [ v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                         v2u16, v3u16, v4u16, v8u16, v16u16,
                                                         v2u32, v3u32, v4u32, v8u32, v16u32,
                                                         v2u64, v3u64, v4u64, v8u64, v16u64], 0, [{
    $Arg1VarName &= ($Arg1BaseType)($BitWidth - 1);
    return as_$ReturnType(( $Arg0VarName << $Arg1VarName ) | $Arg0VarName >> ( ($Arg1BaseType)$BitWidth - $Arg1VarName ));
  }]>;

/*****************************************************************\
 * popcount:                                                     *
 *   gentype popcount (gentype x)                                *
 *   Returns the number of non-zero bits in x                    *
\*****************************************************************/
// popcount on signed integer - call unsigned
OclBuiltinImpl popcount_igeneric = OclBuiltinImpl<popcount, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                                             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64], 0, [{
    return as_$ReturnType(popcount(as_u$Arg0Type($Arg0VarName)));
  }]>;
// popcount on scalar unsigned char, short, int
OclBuiltinImpl popcount_v1u8u16u32 = OclBuiltinImpl<popcount, [v1u8, v1u16, v1u32], 0, [{
    return __builtin_popcount($Arg0VarName);
  }]>;
// popcount on scalar unsigned long
OclBuiltinImpl popcount_v1u64 = OclBuiltinImpl<popcount, [v1u64], 0, [{
    return __builtin_popcountl($Arg0VarName);
  }]>;
// parallel bit-count from:
// http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
// popcount unsigned
OclBuiltinImpl popcount_vu8 = OclBuiltinImpl<popcount, [v2u8, v3u8, v4u8, v8u8, v16u8], 0, [{
    $Arg0Type ret = $Arg0VarName - (($Arg0VarName >> ($Arg0BaseType)1) & ($Arg0BaseType)magic_num_B[0]);
    ret = ((ret >> ($Arg0BaseType)magic_num_S[1]) & ($Arg0BaseType)magic_num_B[1]) + (ret & ($Arg0BaseType)magic_num_B[1]);
    ret = ((ret >> ($Arg0BaseType)magic_num_S[2]) + ret) & ($Arg0BaseType)magic_num_B[2];
    return ret;
  }]>;
OclBuiltinImpl popcount_vu16 = OclBuiltinImpl<popcount, [v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    $Arg0Type ret = $Arg0VarName - (($Arg0VarName >> ($Arg0BaseType)1) & ($Arg0BaseType)magic_num_B[0]);
    ret = ((ret >> ($Arg0BaseType)magic_num_S[1]) & ($Arg0BaseType)magic_num_B[1]) + (ret & ($Arg0BaseType)magic_num_B[1]);
    ret = ((ret >> ($Arg0BaseType)magic_num_S[2]) + ret) & ($Arg0BaseType)magic_num_B[2];
    ret = ((ret >> ($Arg0BaseType)magic_num_S[3]) + ret) & ($Arg0BaseType)magic_num_B[3];
    return ret;
  }]>;
OclBuiltinImpl popcount_vu32 = OclBuiltinImpl<popcount, [v2u32, v3u32, v4u32, v8u32, v16u32] , 0, [{
    $Arg0Type ret = $Arg0VarName - (($Arg0VarName >> 1) & ($Arg0BaseType)magic_num_B[0]);
    ret = ((ret >> magic_num_S[1]) & ($Arg0BaseType)magic_num_B[1]) + (ret & ($Arg0BaseType)magic_num_B[1]);
    ret = ((ret >> magic_num_S[2]) + ret) & ($Arg0BaseType)magic_num_B[2];
    ret = ((ret >> magic_num_S[3]) + ret) & ($Arg0BaseType)magic_num_B[3];
    ret = ((ret >> magic_num_S[4]) + ret) & ($Arg0BaseType)magic_num_B[4];
    return ret;
  }]>;
OclBuiltinImpl popcount_vu64 = OclBuiltinImpl<popcount, [v2u64, v3u64, v4u64, v8u64, v16u64] , 0, [{
    $Arg0Type ret = $Arg0VarName - (($Arg0VarName >> 1) & magic_num_B[0]);
    ret = ((ret >> magic_num_S[1]) & magic_num_B[1]) + (ret & magic_num_B[1]);
    ret = ((ret >> magic_num_S[2]) + ret) & magic_num_B[2];
    ret = ((ret >> magic_num_S[3]) + ret) & magic_num_B[3];
    ret = ((ret >> magic_num_S[4]) + ret) & magic_num_B[4];
    ret = ((ret >> magic_num_S[5]) + ret) & magic_num_B[5];
    return ret;
  }]>;


/*****************************************************************\
 * upsample:                                                     *
 *   shortn upsample (charn hi, ucharn lo)                       *
 *   ushortn upsample (ucharn hi, ucharn lo)                     *
 *   intn upsample (shortn hi, ushortn lo)                       *
 *   uintn upsample (ushortn hi, ushortn lo)                     *
 *   longn upsample (intn hi, uintn lo)                          *
 *   ulongn upsample (uintn hi, uintn lo)                        *
 *     result[i] = ((short)hi[i] << 8) | lo[i]                   *
\*****************************************************************/
// upsample on signed integer - call unsigned
OclBuiltinImpl upsample_igeneric = OclBuiltinImpl<upsample, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32], 0, [{
    return as_$ReturnType(upsample(as_u$Arg0Type($Arg0VarName), $Arg1VarName));
  }]>;
// upsample on unsigned scalar
OclBuiltinImpl upsample_iscalar = OclBuiltinImpl<upsample, [v1u8, v1u16, v1u32], 0, [{
    return (($ReturnType)$Arg0VarName << $BitWidth) | $Arg1VarName;
  }]>;
// upsample on unsigned vector
OclBuiltinImpl upsample_ivector = OclBuiltinImpl<upsample, [ v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                           v2u16, v3u16, v4u16, v8u16, v16u16,
                                                           v2u32, v3u32, v4u32, v8u32, v16u32], 0, [{
    return (convert_$ReturnType($Arg0VarName) << ($ReturnType)$BitWidth) | convert_$ReturnType($Arg1VarName);
  }]>;


/*****************************************************************\
 * sub_sat:                                                      *
 *   gentype sub_sat (gentype x, gentype y)                      *
 *   Returns x - y saturated                                     *
\*****************************************************************/
// sub_sat on scalar signed
OclBuiltinImpl sub_sat_v1i = OclBuiltinImpl<sub_sat, [v1i8, v1i16, v1i32, v1i64], 0, [{
    $ReturnType r = $Arg0VarName - $Arg1VarName;
    if( $Arg1VarName < 0 )
    {
      if( r < $Arg0VarName )
        r = generic_max_$Arg0Type;
    }
    else
    {
      if( r > $Arg0VarName )
        r = generic_min_$Arg0Type;
    }
    return r;
  }]>;
// sub_sat on signed vector
OclBuiltinImpl sub_sat_svector = OclBuiltinImpl<sub_sat, [ v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                          v2i16, v3i16, v4i16, v8i16,
                                                          v2i32, v3i32, v4i32, v8i32, v16i32,
                                                          v2i64, v3i64, v4i64, v8i64, v16i64] , 0, sub_sat_svector_code>;
// sub_sat on unsigned
OclBuiltinImpl sub_sat_unsigned = OclBuiltinImpl<sub_sat, [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                           v1u16, v2u16, v3u16, v4u16, v8u16,
                                                           v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                                                           v1u64, v2u64, v3u64, v4u64, v8u64, v16u64] , 0, sub_sat_unsigned_code>;

/*****************************************************************\
 * mul24:                                                        *
 *   gentype mul24 (gentype x, gentype y)                        *
 *   Multiply two 24-bit integer values x and y. x and y         *
 *   are 32-bit integers but only the low 24-bits are used       *
 *   to perform the multiplication.                              *
\*****************************************************************/
OclBuiltinImpl mul24_signed = OclBuiltinImpl<mul24, [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32], 0, [{
    $Arg0VarName = ($Arg0VarName << 8) >> 8;
    $Arg1VarName = ($Arg1VarName << 8) >> 8;
    return $Arg0VarName * $Arg1VarName;
  }]>;
OclBuiltinImpl mul24_unsigned = OclBuiltinImpl<mul24, [v1u32, v2u32, v3u32, v4u32, v8u32, v16u32], 0, [{
    return $Arg0VarName * $Arg1VarName;
  }]>;


/*****************************************************************\
 * mad24:                                                        *
 *   gentype mad24 (gentype x, gentype y, gentype z)             *
 *   Multipy two 24-bit integer values x and y and add           *
 *   the 32-bit integer result to the 32-bit integer z.          *
\*****************************************************************/
OclBuiltinImpl mad24_iall = OclBuiltinImpl<mad24, [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                                  v1u32, v2u32, v3u32, v4u32, v8u32, v16u32], 0, [{
    return mul24($Arg0VarName, $Arg1VarName) + $Arg2VarName;
  }]>;
