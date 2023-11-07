//
// Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

OclBuiltinImpl HalfToFloat2 = OclBuiltinImpl<__ocl_HalfToFloat, [v1i16], 0, [{
    unsigned short expHalf16 = $Arg0VarName & 0x7C00;
    int exp1 = (int)expHalf16;
    unsigned short mantissa16 = $Arg0VarName & 0x03FF;
    int mantissa1 = (int)mantissa16;
    int sign = (int)($Arg0VarName & 0x8000);
    sign = sign << 16;

    if (expHalf16 == 0x7C00) // nan or inf
    {
      if (mantissa16 > 0) // nan
      {
        int res = (0x7FC00000 | sign); //silance the nans
        float fres = *((float*)(&res));
        return fres;
      }
      // inf
      int res = (0x7F800000 | sign);
      float fres = *((float*)(&res));
      return fres;
    }
    if (expHalf16 != 0) // normalized number
    {
      exp1 += ((127 - 15) << 10); //exponents converted to float32 bias
      int res = (exp1 | mantissa1);
      res = res << 13 ;
      res = ( res | sign );
      float fres = *((float*)(&res));
      return fres;
    }

    int xmm1 = max (exp1, (1 << 10));
    xmm1 = (xmm1 << 13);
    xmm1 += ((127 - 15 - 10) << 23);  // add the bias difference to xmm1
    xmm1 = xmm1 | sign; // Combine with the sign mask

    float res = (float)mantissa1;  // Convert mantissa to float
    res *= *((float*) (&xmm1));

    return res;
  }]>;

OclBuiltinImpl Half4ToFloat42 = OclBuiltinImpl<__ocl_Half4ToFloat4, [v8i16], 0, [{
    int4 xmm7 = (int4)0;
    int4 xmm1 = as_int4($Arg0VarName & ((short8)Fvec8Float16ExponentMask));
    xmm1 = as_int4( _mm_unpacklo_epi16(__builtin_astype(xmm1, __m128i), __builtin_astype(xmm7, __m128i))); // xmm1 = exponents as DWORDS
    int4 xmm2 = as_int4($Arg0VarName & (short8)Fvec8Float16MantissaMask);
    xmm2 = as_int4( _mm_unpacklo_epi16(__builtin_astype(xmm2, __m128i), __builtin_astype(xmm7, __m128i))); // xmm2 = mantissas as DWORDS
    $Arg0VarName = $Arg0VarName & (short8)Fvec8Float16SignMask;
    int4 xmm6 = as_int4(_mm_unpacklo_epi16(__builtin_astype(xmm7, __m128i), __builtin_astype($Arg0VarName, __m128i))); // xmm6 = sign mask as DWORDS

    // We need to handle the case where the number is NaN or INF
    // If the float16 is one of these, then we create an all '1' exponent for the 32bit float and store it in xmm6 for later use
    $Arg0VarName = as_short8(xmm1 == (int4)Fvec4Float16NaNExpMask); // $Arg0VarName.any dword = 0xFFFFFFFF if exponent is all '1'
    int4 xmm4 = as_int4(xmm2 > xmm7); // xmm4.any dword = 0xFFFFFFFF if mantissa > 0
    xmm4 = as_int4(xmm4 & as_int4($Arg0VarName)); // xmm4.any dword = 0xFFFFFFFF if NAN
    xmm4 = as_int4(xmm4 & (int4)Fvec4Float32NanMask); // silence the SNaNs
    $Arg0VarName = ( $Arg0VarName & as_short8((int4)Fvec4Float32ExponentMask)); // // $Arg0VarName = If float16 has all '1' exp, then convert to 32 bit float all '1' exp, otherwise 0
    $Arg0VarName = ($Arg0VarName | as_short8(xmm4));

    int4 xmm3 = as_int4(xmm1 == xmm7); // xmm3.any dword = 0xFFFFFFFF if exp is zero
    int normals = _mm_movemask_epi8(__builtin_astype(xmm3, __m128i));
    if(normals == 0)
    {
      xmm1 = xmm1 + (int4)Fvec4Float16ExpBiasDifference;// xmm1 = exponents converted to float32 bias
      xmm1 = (xmm1 | xmm2); // xmm1 = exponent + mantissa
      xmm1 = as_int4( _mm_slli_epi32(__builtin_astype(xmm1, __m128i), 13));
      xmm1 = (xmm1 | xmm6);  // xmm1 = signed number
      float4 res = as_float4(as_short8(xmm1) | $Arg0VarName);
      return res;  // If the original number was NaN or INF, then xmm6 has all '1' for exp. $Arg0VarName will hold a float32 that is NaN or INF
    }

    xmm3 = (~xmm3 & (int4)FVec4Float16Implicit1Mask);
    xmm2 = (xmm2 | xmm3); // add implicit 1 to the mantissa
    xmm1 = as_int4( _mm_max_epi16(__builtin_astype(xmm1, __m128i), __builtin_astype((int4)Fvec4Float16ExpMin,__m128i))); // xmm1 = max(exp, 1)

    // we can do the comparison on words since we know that the high word of each dword is 0
    xmm1 = as_int4( _mm_slli_epi32(__builtin_astype(xmm1, __m128i), 13));
    xmm1 = (xmm1 +(int4)Fvec4Float16BiasDiffDenorm);  // add the bias difference to xmm1
    xmm1 = (xmm1 | xmm6); // Combine with the sign mask

    float4 xmm5 = convert_float4(xmm2);  // Convert mantissa to float
    xmm5 = (xmm5 * as_float4(xmm1));

    xmm5 = as_float4(as_short8(xmm5) | $Arg0VarName);// If the original number was NaN or INF, then xmm6 has all '1' for exp. $Arg0VarName will hold a float32 that is NaN or INF
    return xmm5;
  }]>;

OclBuiltinImpl Half8ToFloat82 = OclBuiltinImpl<__ocl_Half8ToFloat8, [v8i16], 0, [{
    float8 res;
    short8 inp1;
    inp1.s0123 = $Arg0VarName.s4567;
    res.lo = __ocl_Half4ToFloat4($Arg0VarName);
    res.hi = __ocl_Half4ToFloat4(inp1);
    return res;
  }]>;

OclBuiltinImpl __ocl_float2half_rte2 = OclBuiltinImpl<__ocl_float2half_rte, [v4f32], 0, [{
    //cl_uint sign = (u.u >> 16) & 0x8000;
    short8 temp = as_short8(_mm_srli_epi32(__builtin_astype($Arg0VarName, __m128i), 0x10));
    short8 signs = (temp & as_short8((int4)x8000));
    float4 absParam = as_float4(as_int4($Arg0VarName) &(int4)x7fffffff);

    //Nan
    //if( x != x )
    short8 eq0 = as_short8(_mm_cmpneq_ps(absParam, absParam));
    short8 eq = as_short8(absParam) & eq0;
    //u.u >>= (24-11);
    eq = as_short8( _mm_srli_epi32(__builtin_astype(eq, __m128i), 0x0d));
    //u.u &= 0x7fff;
    eq = (eq & as_short8((int4)x7fff));
    //u.u |= 0x0200;   -- silence the NaN
    eq = (eq |as_short8((int4)x0200));
    //return u.u | sign;
    eq = (eq | signs);
    eq = (eq & eq0);
    float4 dflt = as_float4 (eq0);

    // overflow
    //if( x >= MAKE_HEX_FLOAT(0x1.ffcp15f, 0x1ffcL, 3) )
    //return 0x7c00 | sign;

    float4 eq1 = _mm_cmpge_ps(absParam, as_float4((int4)x477ff000));
    eq0 = as_short8 (eq1) & as_short8((int4)x7c00);
    eq0 = (signs | eq0);
    eq0 = (~as_short8(dflt) & eq0);
    eq = (eq | eq0);
    dflt = as_float4 (as_int4(eq1) | as_int4(dflt));

    // underflow
    // if( x <= MAKE_HEX_FLOAT(0x1.0p-25f, 0x1L, -25) )
    //   return sign
    eq1 = _mm_cmple_ps(absParam, as_float4((int4)x33000000));
    eq0 = as_short8(eq1) & signs;
    eq0 = (~as_short8(dflt) & eq0);
    eq = (eq | eq0);
    dflt = as_float4 (as_int4(eq1) | as_int4(dflt));


    // very small
    // if( x < MAKE_HEX_FLOAT(0x1.8p-24f, 0x18L, -28) )
    //   return sign | 1;
    eq1 = _mm_cmplt_ps(absParam, as_float4((int4)x33c00000));
    eq0 = as_short8(eq1) & (signs | as_short8((int4)conversion_ones));
    eq0 = (~as_short8(dflt) & eq0);
    eq = (eq | eq0);
    dflt = as_float4 (as_int4(eq1) | as_int4(dflt));

    // half denormal
    //  if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
    //    x *= MAKE_HEX_FLOAT(0x1.0p-125f, 0x1L, -125);
    //  return sign | x;
    eq1 = _mm_cmplt_ps(absParam, as_float4((int4)x38800000));
    float4 eq2 = (absParam * as_float4((int4)x01000000));  //x
    eq0 = as_short8(eq1) &(signs | as_short8(eq2));
    eq0 = (~as_short8(dflt) & eq0);
    eq = (eq | eq0);
    dflt = as_float4 (as_int4(eq1) | as_int4(dflt));

    // u.f *= MAKE_HEX_FLOAT(0x1.0p13f, 0x1L, 13);
    // u.u &= 0x7f800000;
    // x += u.f;
    // u.f = x - u.f;
    // u.f *= MAKE_HEX_FLOAT(0x1.0p-112f, 0x1L, -112);
    // return (u.u >> (24-11)) | sign;

    eq1 = ($Arg0VarName * as_float4((int4)x46000000));
    eq0 = as_short8(eq1) & as_short8((int4)x7f800000); //u
    eq1 = (as_float4(eq0)  +absParam); //x
    eq1 = (eq1 - as_float4(eq0)); //u
    eq1 = (eq1 * as_float4((int4)x07800000));
    eq0 = as_short8( _mm_srli_epi32(__builtin_astype(eq1, __m128i), 0x0d));
    eq0 = (eq0 | signs);
    eq0 = (~as_short8(dflt) & eq0);
    eq = (eq | eq0);

    eq1 = as_float4(_mm_shuffle_epi8(__builtin_astype(eq, __m128i), __builtin_astype( g_vls_4x32to4x16,__m128i)));

    return eq1;
  }]>;

OclBuiltinImpl __ocl_float2half_rtz2 = OclBuiltinImpl<__ocl_float2half_rtz, [v4f32], 0, [{
    //cl_uint sign = (u.u >> 16) & 0x8000;
    int4 temp = as_int4(_mm_srli_epi32(__builtin_astype($Arg0VarName, __m128i), 0x10));
    int4 signs = temp & (int4)x8000;
    $Arg0VarName = as_float4(as_int4($Arg0VarName) & (int4)x7fffffff);

    //Nan
    //if( x != x )
    int4 eq0 = as_int4(_mm_cmpneq_ps($Arg0VarName, $Arg0VarName));
    int4 eq = as_int4($Arg0VarName) & eq0;
    //u.u >>= (24-11);
    eq = as_int4(_mm_srli_epi32(__builtin_astype(eq, __m128i), 0x0d));
    //u.u &= 0x7fff;
    eq = eq & (int4)x7fff;
    //u.u |= 0x0200;   -- silence the NaN
    eq = eq | (int4)x0200;
    //return u.u | sign;
    eq = eq | signs;
    eq = eq & eq0;
    int4 dflt = eq0;

    // overflow
    //if( x >= MAKE_HEX_FLOAT(0x1.0p16f, 0x1L, 16) )

    int4 eq1 = as_int4(_mm_cmpge_ps($Arg0VarName, __builtin_astype((int4)x47800000,__m128)));

    //if( x == INFINITY )
    //return 0x7c00 | sign;
    eq0 = as_int4(_mm_cmpeq_epi32(__builtin_astype($Arg0VarName,__m128i), __builtin_astype((int4)x7f800000,__m128i)));
    eq0 = eq0& eq1;
    int4 eq2 = eq0 & (int4)x7c00;
    eq2 = eq2 | signs;
    eq2 = eq0 & eq2;
    eq2 = ~dflt & eq2;
    eq = eq | eq2;
    dflt = eq0 | dflt;


    //else return 0x7bff | sign;
    eq0 = eq1 ^ eq;
    eq2 = eq0 & (int4)x7bff;
    eq2 = eq2 | signs;
    eq2 = eq2 & eq0;
    eq2 = ~dflt & eq2;
    eq = eq | eq2;
    dflt = eq0 | dflt;


    // underflow
    //  if( x < MAKE_HEX_FLOAT(0x1.0p-24f, 0x1L, -24) )
    //  return sign;    -- The halfway case can return 0x0001 or 0. 0 is even.
    eq1 = as_int4(_mm_cmplt_ps($Arg0VarName, __builtin_astype((int4)x33800000,__m128)));
    eq0 = eq1 & signs;
    eq0 = ~dflt & eq0;
    eq = eq | eq0;
    dflt = eq1 | dflt;


    // half denormal
    //  if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
    //  x *= MAKE_HEX_FLOAT(0x1.0p24f, 0x1L, 24);
    //  return (short)( (int)x | sign);
    eq1 = as_int4(_mm_cmplt_ps($Arg0VarName, __builtin_astype((int4)x38800000,__m128)));
    eq2 = eq1;
    eq1 = eq1 & as_int4($Arg0VarName);
    eq1 = as_int4( as_float4(eq1) *  as_float4((int4)x4b800000));
    eq0 = convert_int4(as_float4(eq1));
    eq0 = eq0 | signs;
    eq0 = eq2 & eq0;
    eq0 = ~dflt & eq0;
    eq  = eq | eq0;
    dflt = eq2 | dflt;


    //u.u &= 0xFFFFE000U;
    eq0 = as_int4($Arg0VarName) & (int4)xffffe000;
    //u.u -= 0x38000000U;
    eq0 = eq0 - ((int4)x38000000);
    eq0 = as_int4(_mm_srli_epi32((__builtin_astype(eq0,__m128i)), 13));
    eq0 = eq0 | signs;
    eq0 = ~dflt & eq0;
    eq  = eq | eq0;

    eq1 = as_int4(_mm_shuffle_epi8(__builtin_astype(eq, __m128i), __builtin_astype( g_vls_4x32to4x16,__m128i)));
    return as_float4(eq1);
  }]>;

OclBuiltinImpl __ocl_float2half_rtp2 = OclBuiltinImpl<__ocl_float2half_rtp, [v4f32], 0, [{
    int4 zeros = (int4)0;

    //cl_uint sign = (u.u >> 16) & 0x8000;
    int4 temp = as_int4(_mm_srli_epi32(__builtin_astype($Arg0VarName,__m128i), 0x10));
    int4 signs = temp & ((int4)x8000);
    int4 absParam = as_int4($Arg0VarName) & ((int4)x7fffffff);

    //Nan
    //if( x != x )
    int4 eq0 = as_int4(_mm_cmpneq_ps(as_float4(absParam), as_float4(absParam)));
    int4 eq = absParam & eq0;
    //u.u >>= (24-11);
    eq = as_int4(_mm_srli_epi32(__builtin_astype(eq, __m128i), 0x0d));
    //u.u &= 0x7fff;
    eq = eq & (int4)x7fff;
    //u.u |= 0x0200;   -- silence the NaN
    eq = eq | (int4)x0200;
    //return u.u | sign;
    eq = eq | signs;
    eq = eq & eq0;
    int4 dflt = eq0;

    // overflow
    //if( f > MAKE_HEX_FLOAT(0x1.ffcp15f, 0x1ffcL, 3) )
    //return 0x7c00;

    int4 eq1 = as_int4(_mm_cmpgt_ps($Arg0VarName, as_float4((int4)x477ff000)));
    eq0 = eq1 & (int4)x7c00;
    eq0 = (~(dflt) & eq0);
    eq = eq| eq0;
    dflt = eq1| dflt;

    // if( f <= MAKE_HEX_FLOAT(-0x1.0p16f, -0x1L, 16) )
    eq1 = as_int4(_mm_cmple_ps($Arg0VarName, as_float4((int4)xc7800000)));

    //if( f == -INFINITY )
    //return 0xfc00;
    eq0 = as_int4(_mm_cmpeq_epi32(__builtin_astype($Arg0VarName,__m128i), __builtin_astype((int4)xff800000,__m128i)));
    eq0 = eq0& eq1;
    int4 eq2 = eq0 & (int4)xfc00;
    eq2 = (~(dflt) & eq2);
    eq = eq| eq2;
    dflt = eq0 | dflt;

    //else return 0xfbff;
    eq0 = eq1 ^ eq0;
    eq2 = eq0 & (int4)xfbff;
    eq2 = (~(dflt) & eq2);
    eq = eq | eq2;
    dflt = eq0 | dflt;

    // underflow
    // if( x < MAKE_HEX_FLOAT(0x1.0p-24f, 0x1L, -24) )
    eq1 = as_int4(_mm_cmplt_ps(as_float4(absParam), as_float4((int4)x33800000)));

    // if(f > 0) return 1;
    eq0 = as_int4(_mm_cmpgt_ps($Arg0VarName, as_float4(zeros)));
    eq0 = eq0 & eq1;
    eq2 = eq0 &(int4)conversion_ones;
    eq2 = (~(dflt) & eq2);
    eq = eq | eq2;
    dflt = eq0 | dflt;
    // else return sign
    eq0 = eq1 ^ eq0;
    eq2 = eq0 & signs;
    eq2 = (~(dflt) & eq2);
    eq = eq | eq2;
    dflt = eq0 | dflt;

    // half denormal
    //  if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
    //    x *= MAKE_HEX_FLOAT(0x1.0p24f, 0x1L, 24);
    //  int r = (int)x;
    eq1 = as_int4(_mm_cmplt_ps(as_float4(absParam), as_float4((int4)x38800000)));
    eq2 = eq1;
    eq1 = eq1 & absParam;
    eq1 = as_int4(as_float4(eq1) * as_float4((int4)x4b800000));  //x
    eq0 = as_int4(_mm_cvttps_epi32(as_float4(eq1))); //r
    // r += (float)r != x && f > 0.0f;
    int4 eq3 = as_int4(_mm_cvtepi32_ps(_mm_castps_si128(as_float4(eq0)))); //(float)r
    eq1 = as_int4(_mm_cmpneq_ps(as_float4(eq1), as_float4(eq3))); // (float)r != x
    eq3 = as_int4(_mm_cmpgt_ps($Arg0VarName, as_float4(zeros))); //f > 0.0f
    eq1 = eq1 & eq3; //(float)r != x && f > 0.0f
    int4 eq4 = eq1 & ((int4)conversion_ones);
    eq0 = eq0 + eq4;
    // return (short)(r | sign)
    eq0 = eq0 | signs;
    eq0 = eq2 & eq0;
    eq0 = (~(dflt) & eq0);
    eq  = eq | eq0;
    dflt = eq2 | dflt;

    //u.u &= 0xFFFFE000U;
    eq0 = as_int4(as_int4($Arg0VarName) & ((int4)xffffe000));
    //if (f > u.f)
    //u.u += 0x00002000U;
    eq1 = as_int4(_mm_cmpgt_ps($Arg0VarName, as_float4(eq0)));
    eq2 = eq0 + (eq1 &(int4)x00002000);
    //u.u -= 0x38000000U;
    //return ((u.u >> 13) | sign);
    eq2 = eq2 - ((int4)x38000000);
    eq2 = as_int4(_mm_srli_epi32(__builtin_astype(eq2, __m128i), 13));
    eq2 = eq2 | signs;
    eq2 = (~(dflt) & eq2);
    eq  = eq | eq2;

    eq1 = as_int4(_mm_shuffle_epi8(__builtin_astype(eq, __m128i), __builtin_astype( g_vls_4x32to4x16,__m128i)));

    return as_float4(eq1);
  }]>;

OclBuiltinImpl __ocl_float2half_rtn2 = OclBuiltinImpl<__ocl_float2half_rtn, [v4f32], 0, [{
    int4 zeros = (int4)0;

    //cl_uint sign = (u.u >> 16) & 0x8000;
    int4 temp = as_int4(_mm_srli_epi32(__builtin_astype($Arg0VarName,__m128i), 0x10));
    int4 signs = temp & ((int4)x8000);
    int4 absParam = as_int4($Arg0VarName) & ((int4)x7fffffff);

    //Nan
    //if( x != x )
    int4 eq0 = as_int4(_mm_cmpneq_ps(as_float4(absParam), as_float4(absParam)));
    int4 eq = absParam & eq0;
    //u.u >>= (24-11);
    eq = as_int4(_mm_srli_epi32(__builtin_astype(eq, __m128i), 0x0d));
    //u.u &= 0x7fff;
    eq = eq & ((int4)x7fff);
    //u.u |= 0x0200;   -- silence the NaN
    eq = eq | ((int4)x0200);
    //return u.u | sign;
    eq = eq | signs;
    eq = eq & eq0;
    int4 dflt = eq0;


    // overflow
    //if( f >= MAKE_HEX_FLOAT(0x1.0p16f, 0x1L, 16) )
    int4 eq1 = as_int4(_mm_cmpge_ps($Arg0VarName, as_float4((int4)x47800000)));

    //if( f == INFINITY )
    //return 0x7c00;
    eq0 = as_int4(_mm_cmpeq_epi32(_mm_castps_si128($Arg0VarName), __builtin_astype((int4)x7f800000,__m128i)));
    eq0 = eq0 & eq1;
    int4 eq2 = eq0 & ((int4)x7c00);
    eq2 = ~(dflt) & eq2;
    eq = eq | eq2;
    dflt = eq0 | dflt;

    //else return 0x7bff;
    eq0 = eq1 ^ eq0;
    eq2 = eq0 & ((int4)x7bff);
    eq2 = ~(dflt) & eq2;
    eq = eq | eq2;
    dflt = eq0 | dflt;

    //if( f < MAKE_HEX_FLOAT(-0x1.ffcp15f, -0x1ffcL, 3) )
    //return 0xfc00;
    eq1 = as_int4(_mm_cmplt_ps($Arg0VarName, as_float4((int4)xc77fe000)));
    eq0 = eq1 & ((int4)xfc00);
    eq0 = ~(dflt) & eq0;
    eq = eq | eq0;
    dflt = eq1 | dflt;

    // underflow
    // if( x < MAKE_HEX_FLOAT(0x1.0p-24f, 0x1L, -24) )
    eq1 = as_int4(_mm_cmplt_ps(as_float4(absParam), as_float4((int4)x33800000)));

    // if(f < 0) return 0x8001;
    eq0 = as_int4(_mm_cmplt_ps($Arg0VarName, as_float4(zeros)));
    eq0 = eq0 & eq1;
    eq2 = eq0 & ((int4)x8001);
    eq2 = ~(dflt) & eq2;
    eq = eq | eq2;
    dflt = eq0 | dflt;
    // else return sign
    eq0 = eq1 ^ eq0;
    eq2 = eq0 & signs;
    eq2 = ~(dflt) & eq2;
    eq = eq | eq2;
    dflt = eq0 | dflt;

    // half denormal
    //  if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
    //    x *= MAKE_HEX_FLOAT(0x1.0p24f, 0x1L, 24);
    //  int r = (int)x;
    eq1 = as_int4(_mm_cmplt_ps(as_float4(absParam), as_float4((int4)x38800000)));
    eq2 = eq1;
    eq1 = eq1 & absParam;
    eq1 = as_int4(as_float4(eq1) * as_float4((int4)x4b800000));  //x
    eq0 = as_int4(_mm_cvttps_epi32(as_float4(eq1))); //r

    // r += (float)r != x && f < 0.0f;
    int4 eq3 = as_int4(_mm_cvtepi32_ps(_mm_castps_si128(as_float4(eq0)))); //(float)r
    eq1 = as_int4(_mm_cmpneq_ps(as_float4(eq1), as_float4(eq3))); // (float)r != x
    eq3 = as_int4(_mm_cmplt_ps($Arg0VarName, as_float4(zeros))); //f < 0.0f
    eq1 = eq1 & eq3; //(float)r != x && f < 0.0f
    int4 eq4 = eq1 & ((int4)conversion_ones);
    eq0 = (eq0 + eq4);
    // return (short)(r | sign)
    eq0 = eq0 | signs;
    eq0 = eq2 & eq0;
    eq0 = ~(dflt) & eq0;
    eq  = eq | eq0;
    dflt = eq2 | dflt;

    //u.u &= 0xFFFFE000U;
    eq0 = (as_int4($Arg0VarName) & ((int4)xffffe000));
    //if (u.f > f)
    //u.u += 0x00002000U;
    eq1 = as_int4(_mm_cmpgt_ps(as_float4(eq0), $Arg0VarName));
    eq2 = eq0 + ((eq1 & ((int4)x00002000)));
    //u.u -= 0x38000000U;
    //return ((u.u >> 13) | sign);
    eq2 = (eq2 - ((int4)x38000000));
    eq2 = as_int4(_mm_srli_epi32(__builtin_astype(eq2, __m128i), 13));
    eq2 = eq2 | signs;
    eq2 = ~(dflt) & eq2;
    eq  = eq | eq2;

    eq1 = as_int4(_mm_shuffle_epi8(__builtin_astype(eq, __m128i), __builtin_astype( g_vls_4x32to4x16,__m128i)));

    return as_float4(eq1);
  }]>;

OclBuiltinImpl __ocl_float2half8_rte2 = OclBuiltinImpl<__ocl_float2half8_rte, [v8f32], 0, [{
    float4 res;
    res.lo = __ocl_float2half_rte(param.lo).lo;
    res.hi = __ocl_float2half_rte(param.hi).lo;
    return res;
  }]>;

OclBuiltinImpl __ocl_float2half8_rtz2 = OclBuiltinImpl<__ocl_float2half8_rtz, [v8f32], 0, [{
    float4 res;
    res.lo = __ocl_float2half_rtz(param.lo).lo;
    res.hi = __ocl_float2half_rtz(param.hi).lo;
    return res;
  }]>;

OclBuiltinImpl __ocl_float2half8_rtn2 = OclBuiltinImpl<__ocl_float2half8_rtn, [v8f32], 0, [{
    float4 res;
    res.lo = __ocl_float2half_rtn(param.lo).lo;
    res.hi = __ocl_float2half_rtn(param.hi).lo;
    return res;
  }]>;

OclBuiltinImpl __ocl_float2half8_rtp2 = OclBuiltinImpl<__ocl_float2half8_rtp, [v8f32], 0, [{
    float4 res;
    res.lo = __ocl_float2half_rtp(param.lo).lo;
    res.hi = __ocl_float2half_rtp(param.hi).lo;
    return res;
  }]>;

OclBuiltinImpl __ocl_float2half2 = OclBuiltinImpl<__ocl_float2half, [v4f32], 0, [{
    return __ocl_float2half_rte($Arg0VarName);
  }]>;

OclBuiltinImpl __ocl_float2half82 = OclBuiltinImpl<__ocl_float2half8, [v8f32], 0, [{
    return __ocl_float2half8_rte($Arg0VarName);
  }]>;

OclBuiltinImpl __ocl_double2ToHalf2_rtp2 = OclBuiltinImpl<__ocl_double2ToHalf2_rtp, [v2f64], 0, [{
    float2 f2 = convert_float2_rtp($Arg0VarName);
    float4 res;
    short2 t;
    vstore_half2_rtp(f2, 0, (__private half*)&t);
    res.s0 = as_float(t);
    return res;
  }]>;

OclBuiltinImpl __ocl_double2ToHalf2_rtn2 = OclBuiltinImpl<__ocl_double2ToHalf2_rtn, [v2f64], 0, [{
    float2 f2 = convert_float2_rtn($Arg0VarName);
    float4 res;
    short2 t;
    vstore_half2_rtn(f2, 0, (__private half*)&t);
    res.s0 = as_float(t);
    return res;
  }]>;

OclBuiltinImpl __ocl_double2ToHalf2_rtz2 = OclBuiltinImpl<__ocl_double2ToHalf2_rtz, [v2f64], 0, [{
    float2 f2 = convert_float2_rtz($Arg0VarName);
    float4 res;
    short2 t;
    vstore_half2_rtz(f2, 0, (__private half*)&t);
    res.s0 = as_float(t);
    return res;
  }]>;

OclBuiltinImpl __ocl_double2ToHalf2_rte2 = OclBuiltinImpl<__ocl_double2ToHalf2_rte, [v2f64], 0, [{
    //cl_ulong sign = (u.u >> 48) & 0x8000;
    //double x = fabs(f);
    double2 temp = as_double2(as_long2($Arg0VarName) >> (long2)(48));
    double2 signs = as_double2(as_long2(temp) & as_long2((int4)x8000));
    double2 absParam = as_double2(as_long2($Arg0VarName) & (long2)x7fffffffffffffff);

    //Nan
    //if( x != x )
    //  u.u >>= (53-11);
    //  u.u &= 0x7fff;
    //  u.u |= 0x0200;   -- silence the NaN
    //  return u.u | sign;

    double2 eq0 = _mm_cmpneq_pd(absParam, absParam);
    double2 eq = as_double2(as_long2(absParam) & as_long2(eq0));
    eq = as_double2(as_long2(eq) >> (long2)(42));
    eq = as_double2(as_long2(eq) & as_long2((int4)x7fff));
    eq = as_double2(as_long2(eq) | as_long2((int4)x0200));
    eq = as_double2(as_long2(eq) | as_long2(signs));
    eq = as_double2(as_long2(eq) & as_long2(eq0));
    double2 dflt = eq0;

    //// overflow
    ////if( x >= MAKE_HEX_DOUBLE(0x1.ffep15, 0x1ffeL, 3) )
    ////         0x40effe0000000000
    ////return 0x7c00 | sign;

    double2 eq1 = _mm_cmpge_pd(absParam, as_double2((long2)x40effe0000000000));
    eq0 = as_double2(as_long2(eq1) & as_long2((int4)x7c00));
    eq0 = as_double2(as_long2(signs) | as_long2(eq0));
    eq0 = as_double2(~as_long2(dflt) & as_long2(eq0));
    eq = as_double2(as_long2(eq) | as_long2(eq0));
    dflt = as_double2(as_long2(eq1) | as_long2(dflt));

    //// underflow
    ////  if( x <= MAKE_HEX_DOUBLE(0x1.0p-25, 0x1L, -25) )
    ////       0x3e60000000000000
    //// return sign
    eq1 = _mm_cmple_pd(absParam, as_double2((long2)x3e60000000000000));
    eq0 = as_double2(as_long2(eq1) & as_long2(signs));
    eq0 = as_double2(~as_long2(dflt) & as_long2(eq0));
    eq = as_double2(as_long2(eq) | as_long2(eq0));
    dflt = as_double2(as_long2(eq1) | as_long2(dflt));


    //// very small
    ////  if( x < MAKE_HEX_DOUBLE(0x1.8p-24, 0x18L, -28) )
    ////      0x3e78000000000000
    //// return sign | 1;
    eq1 = _mm_cmplt_pd(absParam, as_double2((long2)x3e78000000000000));
    eq0 = as_double2(as_long2(eq1) & (as_long2(signs) | as_long2((long2)dones)));
    eq0 = as_double2(~as_long2(dflt) & as_long2(eq0));
    eq = as_double2( as_long2(eq) | as_long2(eq0));
    dflt = as_double2(as_long2(eq1) | as_long2(dflt));

    //// half denormal
    ////  if( x < MAKE_HEX_DOUBLE(0x1.0p-14, 0x1L, -14) )
    ////      0x3f10000000000000
    ////  u.f = x * MAKE_HEX_DOUBLE(0x1.0p-1050, 0x1L, -1050);
    ////      0x0000000001000000
    ////  return sign | x;
    eq1 = _mm_cmplt_pd(absParam, as_double2((long2)x3f10000000000000));
    double2 eq2 = (absParam * as_double2((long2)x0000000001000000));  //x
    eq0 = as_double2(as_long2(eq1) & (as_long2(signs) | as_long2(eq2)));
    eq0 = as_double2(~as_long2(dflt) & as_long2(eq0));
    eq = as_double2(as_long2(eq) | as_long2(eq0));
    dflt = as_double2(as_long2(eq1) | as_long2(dflt));

    //// u.f *= MAKE_HEX_DOUBLE(0x1.0p42, 0x1L, 42);
    ////      0x4290000000000000
    //// u.u &= 0x7ff0000000000000UL;
    //// x += u.f;
    //// u.f = x - u.f;
    //// u.f *= MAKE_HEX_DOUBLE(0x1.0p-1008, 0x1L, -1008);
    ////      0x00f0000000000000
    //// return (u.u >> (53-11)) | sign;
    //
    double2 res = ($Arg0VarName * as_double2((long2)x4290000000000000));
    double2 tmp = as_double2(as_long2(res) & (long2)x7ff0000000000000); //u
    res = (tmp  + absParam); //x
    res = (res - tmp); //u
    res = (res * as_double2((long2)x00f0000000000000));
    res = as_double2(as_long2(res) >> (long2)(42));
    res = as_double2(as_long2(res) | as_long2(signs));
    res = as_double2(~as_long2(dflt) & as_long2(res));
    eq = as_double2(as_long2(eq) | as_long2(res));

    return as_$ReturnType(_mm_shuffle_epi8( __builtin_astype(eq, __m128i), __builtin_astype( g_vls_2x64to2x16, __m128i)));

  }]>;

OclBuiltinImpl __ocl_double4ToHalf4_rte2 = OclBuiltinImpl<__ocl_double4ToHalf4_rte, [v4f64], 0, [{
    float4 res;
    res.s0 = __ocl_double2ToHalf2_rte($Arg0VarName.lo).s0;
    res.s1 = __ocl_double2ToHalf2_rte($Arg0VarName.hi).s0;
    return res;
  }]>;

OclBuiltinImpl __ocl_double4ToHalf4_rtp2 = OclBuiltinImpl<__ocl_double4ToHalf4_rtp, [v4f64], 0, [{
    float4 f4 = convert_float4_rtp($Arg0VarName);
    float4 res;
    short4 t;
    vstore_half4_rtp(f4, 0, (__private half*)&t);
    res.s01 = as_float2(t);
    return res;
  }]>;

OclBuiltinImpl __ocl_double4ToHalf4_rtn2 = OclBuiltinImpl<__ocl_double4ToHalf4_rtn, [v4f64], 0, [{
    float4 f4 = convert_float4_rtn($Arg0VarName);
    float4 res;
    short4 t;
    vstore_half4_rtn(f4, 0, (__private half*)&t);
    res.s01 = as_float2(t);
    return res;
  }]>;

OclBuiltinImpl __ocl_double4ToHalf4_rtz2 = OclBuiltinImpl<__ocl_double4ToHalf4_rtz, [v4f64], 0, [{
    float4 f4 = convert_float4_rtz($Arg0VarName);
    float4 res;
    short4 t;
    vstore_half4_rtz(f4, 0, (__private half*)&t);
    res.s01 = as_float2(t);
    return res;
  }]>;

OclBuiltinImpl __ocl_double2ToHalf22 = OclBuiltinImpl<__ocl_double2ToHalf2, [v2f64], 0, [{
    return __ocl_double2ToHalf2_rte($Arg0VarName);
  }]>;

OclBuiltinImpl __ocl_double4ToHalf42 = OclBuiltinImpl<__ocl_double4ToHalf4, [v4f64], 0, [{
    return __ocl_double4ToHalf4_rte($Arg0VarName);
  }]>;

//===========load===================
code Load = [{
    __attribute__((aligned(sizeof($ReturnBaseType)))) typedef struct {$ReturnType data;} LoadTy;
    const LoadTy *pSrc = (LoadTy*)((char*)$Arg1VarName + (offset * $VecLength * sizeof($ReturnBaseType)));
    return pSrc->data;
  }];
code LoadC = [{
    __attribute__((aligned(1))) typedef struct {$ReturnType data;} LoadTy;
    const constant LoadTy *pSrc = (constant LoadTy*)((constant char*)$Arg1VarName +
                                                     (offset * $VecLength * sizeof($ReturnBaseType)));
    return pSrc->data;
  }];

code Load3 = [{
    __attribute__((aligned(sizeof($ReturnBaseType)))) typedef struct {$ReturnType data;} LoadTy;
    const LoadTy *pSrc = (LoadTy*)((char*)$Arg1VarName + (offset * $VecLength * sizeof($ReturnBaseType)));
    return ($ReturnType)(pSrc->data.lo, pSrc->data.s2);
  }];
code LoadC3 = [{
    __attribute__((aligned(1))) typedef struct {$ReturnType data;} LoadTy;
    const constant LoadTy *pSrc = (constant LoadTy*)((constant char*)$Arg1VarName +
                                                     (offset * $VecLength * sizeof($ReturnBaseType)));
    return ($ReturnType)(pSrc->data.lo, pSrc->data.s2);
  }];

code Load_half = [{
    return __ocl_HalfToFloat(*((short*)($Arg1VarName+$Arg0VarName)));
  }];
code Load_const_half = [{
    return __ocl_HalfToFloat(*((constant short*)($Arg1VarName+$Arg0VarName)));
  }];
code Load_half2 = [{
    short8 tmp;
    tmp.s01= *((short2 *)($Arg1VarName + 2*$Arg0VarName));
    float4 res = __ocl_Half4ToFloat4(tmp);
    return res.lo;
  }];
code Load_const_half2 = [{
    short8 tmp;
    tmp.s01= *((constant short2 *)($Arg1VarName + 2*$Arg0VarName));
    float4 res = __ocl_Half4ToFloat4(tmp);
    return res.lo;
  }];
code Load_half3 = [{
    short8 tmp;
    // vload_half3 by spec. must load 6 bytes
    short * base = (short*)$Arg1VarName + 3*$Arg0VarName;
    tmp.s012 = (short3)(*(short2*)base, base[2]);
    float4 temp = __ocl_Half4ToFloat4(tmp);
    return temp.s012;
  }];
code Load_const_half3 = [{
    short8 tmp;
    // vload_half3 by spec. must load 6 bytes
    constant short * base = (constant short*)$Arg1VarName + 3*$Arg0VarName;
    tmp.s012 = (short3)(*(constant short2*)base, base[2]);
    float4 temp = __ocl_Half4ToFloat4(tmp);
    return temp.s012;
  }];
code Load_half4 = [{
    short8 tmp;
    tmp.lo = *((short4 *)($Arg1VarName + 4*$Arg0VarName));
    return __ocl_Half4ToFloat4(tmp);
  }];
code Load_const_half4 = [{
    short8 tmp;
    tmp.lo = *((constant short4 *)($Arg1VarName + 4*$Arg0VarName));
    return __ocl_Half4ToFloat4(tmp);
  }];
code Load_half8 = [{
    const short8 inp = vload8($Arg0VarName, ($Arg1igentype)$Arg1VarName);
    return __ocl_Half8ToFloat8(inp);
  }];
code Load_half16 = [{
    float16 res;
    const short16 inp = vload16($Arg0VarName, ($Arg1igentype)$Arg1VarName);
    res.lo = __ocl_Half8ToFloat8(inp.lo);
    res.hi = __ocl_Half8ToFloat8(inp.hi);
    return res;
  }];
code Loada_half3 = [{
    short8 tmp;
    // vloada_half3 by spec. must load 6 bytes
    short * base = (short*)$Arg1VarName + 4*$Arg0VarName;
    tmp.s012 = (short3)(*(short2*)base, base[2]);
    float4 temp = __ocl_Half4ToFloat4(tmp);
    return temp.s012;
  }];
code Loada_const_half3 = [{
    short8 tmp;
    // vloada_half3 by spec. must load 6 bytes
    constant short * base = (constant short*)$Arg1VarName + 4*$Arg0VarName;
    tmp.s012 = (short3)(*(constant short2*)base, base[2]);
    float4 temp = __ocl_Half4ToFloat4(tmp);
    return temp.s012;
  }];
//==============store==================
code Store = [{
    __attribute__((aligned(sizeof($Arg0BaseType)))) typedef struct {$Arg0Type data;} StoreTy;
    StoreTy* pDst = (StoreTy*)((char*)$Arg2VarName + ($Arg1VarName * $VecLength * sizeof($Arg0BaseType)));
    pDst->data = $Arg0VarName;
  }];
code Store3 = [{
    __attribute__((aligned(sizeof($Arg0BaseType)))) typedef struct {$Arg0Type data;} StoreTy;
    StoreTy* pDst = (StoreTy*)((char*)$Arg2VarName + ($Arg1VarName * $VecLength * sizeof($Arg0BaseType)));
    pDst->data.lo = $Arg0VarName.lo;
    pDst->data.s2 = $Arg0VarName.s2;
  }];

code Stored_half = [{
    double2 d2 = (double2)(0.0);
    d2.x = $Arg0VarName;
    float4 f4 = __ocl_double2ToHalf2(d2);
    ((short *)$Arg2VarName)[$Arg1VarName] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 0);
  }];
code Stored_half2 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*2);
    float4 f4 = __ocl_double2ToHalf2($Arg0VarName);
    *((float*)$Arg2VarName) = f4.s0;
  }];
code Stored_half3 = [{
    float4 t1, t2, f4;
    t1 = __ocl_double2ToHalf2($Arg0VarName.xy);
    t2 = __ocl_double2ToHalf2($Arg0VarName.zz);
    f4.x = t1.x;
    f4.y = t2.x;
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*3);
    *((float*)$Arg2VarName) = f4.s0;
    ((short *)$Arg2VarName)[2] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 2);
  }];
code Stored_half4 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    float4 t1;
    t1 = __ocl_double4ToHalf4($Arg0VarName);
    _mm_storel_epi64((__m128i*)$Arg2VarName, __builtin_astype(t1,__m128i));
  }];
code Stored_half8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8;
    float4 t1, t2;
    t1 = __ocl_double4ToHalf4($Arg0VarName.s0123);
    t2 = __ocl_double4ToHalf4($Arg0VarName.s4567);
    f8.xy = t1.lo;
    f8.zw = t2.lo;
    _mm_storeu_ps((private float*)(float*)$Arg2VarName, f8.lo);
  }];
code Stored_half16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float16 f16;
    float4 t1, t2, t3, t4, t5, t6, t7, t8;
    t1 = __ocl_double2ToHalf2($Arg0VarName.s01);
    t2 = __ocl_double2ToHalf2($Arg0VarName.s23);
    t3 = __ocl_double2ToHalf2($Arg0VarName.s45);
    t4 = __ocl_double2ToHalf2($Arg0VarName.s67);
    t5 = __ocl_double2ToHalf2($Arg0VarName.s89);
    t6 = __ocl_double2ToHalf2($Arg0VarName.sAB);
    t7 = __ocl_double2ToHalf2($Arg0VarName.sCD);
    t8 = __ocl_double2ToHalf2($Arg0VarName.sEF);
    f16.s0 = t1.x;
    f16.s1 = t2.x;
    f16.s2 = t3.x;
    f16.s3 = t4.x;
    f16.s4 = t5.x;
    f16.s5 = t6.x;
    f16.s6 = t7.x;
    f16.s7 = t8.x;
    _mm_storeu_ps((private float*)(float*)$Arg2VarName, f16.lo.lo);
    $Arg2VarName = $Arg2VarName + 8;
    _mm_storeu_ps((private float*)(float*)$Arg2VarName, f16.lo.hi);
  }];
code Storead_half3 = [{
    float4 t1, t2, f4;
    t1 = __ocl_double2ToHalf2($Arg0VarName.xy);
    t2 = __ocl_double2ToHalf2($Arg0VarName.zz);
    f4.x = t1.x;
    f4.y = t2.x;
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    _mm_storel_epi64((__m128i*)$Arg2VarName, __builtin_astype(f4,__m128i));
  }];
code Storead_half8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8;
    float4 t1, t2;
    t1 = __ocl_double4ToHalf4($Arg0VarName.s0123);
    t2 = __ocl_double4ToHalf4($Arg0VarName.s4567);
    f8.xy = t1.lo;
    f8.zw = t2.lo;
    _mm_store_ps((float*)$Arg2VarName, f8.lo);
  }];
code Storead_half16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float16 f16;
    float4 t1, t2, t3, t4, t5, t6, t7, t8;
    t1 = __ocl_double2ToHalf2($Arg0VarName.s01);
    t2 = __ocl_double2ToHalf2($Arg0VarName.s23);
    t3 = __ocl_double2ToHalf2($Arg0VarName.s45);
    t4 = __ocl_double2ToHalf2($Arg0VarName.s67);
    t5 = __ocl_double2ToHalf2($Arg0VarName.s89);
    t6 = __ocl_double2ToHalf2($Arg0VarName.sAB);
    t7 = __ocl_double2ToHalf2($Arg0VarName.sCD);
    t8 = __ocl_double2ToHalf2($Arg0VarName.sEF);
    f16.s0 = t1.x;
    f16.s1 = t2.x;
    f16.s2 = t3.x;
    f16.s3 = t4.x;
    f16.s4 = t5.x;
    f16.s5 = t6.x;
    f16.s6 = t7.x;
    f16.s7 = t8.x;
    _mm_store_ps((float*)$Arg2VarName, f16.lo.lo);
    $Arg2VarName = $Arg2VarName + 8;
    _mm_store_ps((float*)$Arg2VarName, f16.lo.hi);
  }];
code Storef_half = [{
    float4 f4;
    f4.s0 = $Arg0VarName;
    f4 = __ocl_float2half(f4);
    ((short *)$Arg2VarName)[$Arg1VarName] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 0);
  }];
code Storef_half2 = [{
    float4 f4;
    f4.lo = $Arg0VarName;
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*2);
    f4 = __ocl_float2half(f4);
    *((float*)$Arg2VarName) = f4.s0;
  }];
code Storeaf_half3 = [{
    float4 f4;
    f4.s012 = $Arg0VarName;
    f4 = __ocl_float2half(f4);
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    _mm_storel_epi64((__m128i*)$Arg2VarName, __builtin_astype(f4,__m128i));
  }];
code Storef_half4 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    $Arg0VarName = __ocl_float2half($Arg0VarName);
    *((float2 *)$Arg2VarName) = $Arg0VarName.lo;
  }];
code Storef_half8 = [{
    float4 res = __ocl_float2half8($Arg0VarName);
    vstore8(*(short8*)&res, $Arg1VarName, ($Arg2igentype)$Arg2VarName);
  }];
code Storef_half16 = [{
    float8 res;
    res.lo = __ocl_float2half8($Arg0VarName.lo);
    res.hi = __ocl_float2half8($Arg0VarName.hi);
    vstore16(*(short16*)&res, $Arg1VarName, ($Arg2igentype)$Arg2VarName);
  }];
code Storef_half3 = [{
    float4 f4;
    f4.s012 = $Arg0VarName;
    f4 = __ocl_float2half(f4);
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*3);
    *((float*)$Arg2VarName) = f4.s0;
    ((short*)$Arg2VarName)[2] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 2);
  }];

code Storef_half_rte = [{
    float4 f4;
    f4.s0 = $Arg0VarName;
    f4 = __ocl_float2half_rte(f4);
    ((short *)$Arg2VarName)[$Arg1VarName] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 0);
  }];
code Storef_half_rte2 = [{
    float4 f4;
    f4.lo = $Arg0VarName;
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*2);
    f4 = __ocl_float2half_rte(f4);
    *((float*)$Arg2VarName) = f4.s0;
  }];
code Storeaf_half_rte3 = [{
    float4 f4;
    f4.s012 = $Arg0VarName;
    f4 = __ocl_float2half_rte(f4);
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    _mm_storel_epi64((__m128i*)$Arg2VarName, __builtin_astype(f4,__m128i));
  }];
code Storef_half_rte3 = [{
    float4 f4;
    f4.s012 = $Arg0VarName;
    f4 = __ocl_float2half_rte(f4);
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*3);
    *((float*)$Arg2VarName) = f4.s0;
    ((short*)$Arg2VarName)[2] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 2);
  }];
code Storef_half_rte4 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    $Arg0VarName = __ocl_float2half_rte($Arg0VarName);
    *((float2 *)$Arg2VarName) = $Arg0VarName.lo;
  }];
code Storef_half_rte8 = [{
    float4 res = __ocl_float2half8_rte($Arg0VarName);
    vstore8(*(short8*)&res, $Arg1VarName, ($Arg2igentype)$Arg2VarName);
  }];
code Storef_half_rte16 = [{
    float8 res;
    res.lo = __ocl_float2half8_rte($Arg0VarName.lo);
    res.hi = __ocl_float2half8_rte($Arg0VarName.hi);
    vstore16(*(short16*)&res, $Arg1VarName, ($Arg2igentype)$Arg2VarName);
  }];



code Stored_half_rte = [{
    double2 d2 = (double2)(0.0);
    d2.x = $Arg0VarName;
    float4 f4 = __ocl_double2ToHalf2_rte(d2);
    ((short *)$Arg2VarName)[$Arg1VarName] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 0);
  }];
code Stored_half_rte2 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*2);
    float4 f4 = __ocl_double2ToHalf2_rte($Arg0VarName);
    *((float*)$Arg2VarName) = f4.s0;
  }];
code Storead_half_rte3 = [{
    float4 t1, t2, f4;
    t1 = __ocl_double2ToHalf2_rte($Arg0VarName.xy);
    t2 = __ocl_double2ToHalf2_rte($Arg0VarName.zz);
    f4.x = t1.x;
    f4.y = t2.x;
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    _mm_storel_epi64((__m128i*)$Arg2VarName, __builtin_astype(f4,__m128i));
  }];
code Stored_half_rte3 = [{
    float4 t1, t2, f4;
    t1 = __ocl_double2ToHalf2_rte($Arg0VarName.xy);
    t2 = __ocl_double2ToHalf2_rte($Arg0VarName.zz);
    f4.x = t1.x;
    f4.y = t2.x;
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*3);
    *((float*)$Arg2VarName) = f4.s0;
    ((short *)$Arg2VarName)[2] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 2);
  }];
code Stored_half_rte4 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    float4 t1;
    t1 = __ocl_double4ToHalf4_rte($Arg0VarName);
    _mm_storel_epi64((__m128i*)$Arg2VarName, __builtin_astype(t1,__m128i));
  }];
code Storead_half_rte8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8;
    float4 t1, t2;
    t1 = __ocl_double4ToHalf4_rte($Arg0VarName.s0123);
    t2 = __ocl_double4ToHalf4_rte($Arg0VarName.s4567);
    f8.xy = t1.lo;
    f8.zw = t2.lo;
    _mm_store_ps((float*)$Arg2VarName, f8.lo);
  }];
code Stored_half_rte8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8;
    float4 t1, t2;
    t1 = __ocl_double4ToHalf4_rte($Arg0VarName.s0123);
    t2 = __ocl_double4ToHalf4_rte($Arg0VarName.s4567);
    f8.xy = t1.lo;
    f8.zw = t2.lo;
    _mm_storeu_ps((private float*)(float*)$Arg2VarName, f8.lo);
  }];
code Storead_half_rte16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float16 f16;
    float4 t1, t2, t3, t4, t5, t6, t7, t8;
    t1 = __ocl_double2ToHalf2_rte($Arg0VarName.s01);
    t2 = __ocl_double2ToHalf2_rte($Arg0VarName.s23);
    t3 = __ocl_double2ToHalf2_rte($Arg0VarName.s45);
    t4 = __ocl_double2ToHalf2_rte($Arg0VarName.s67);
    t5 = __ocl_double2ToHalf2_rte($Arg0VarName.s89);
    t6 = __ocl_double2ToHalf2_rte($Arg0VarName.sAB);
    t7 = __ocl_double2ToHalf2_rte($Arg0VarName.sCD);
    t8 = __ocl_double2ToHalf2_rte($Arg0VarName.sEF);
    f16.s0 = t1.x;
    f16.s1 = t2.x;
    f16.s2 = t3.x;
    f16.s3 = t4.x;
    f16.s4 = t5.x;
    f16.s5 = t6.x;
    f16.s6 = t7.x;
    f16.s7 = t8.x;
    _mm_store_ps((float*)$Arg2VarName, f16.lo.lo);
    $Arg2VarName = $Arg2VarName + 8;
    _mm_store_ps((float*)$Arg2VarName, f16.lo.hi);
  }];
code Stored_half_rte16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float16 f16;
    float4 t1, t2, t3, t4, t5, t6, t7, t8;
    t1 = __ocl_double2ToHalf2_rte($Arg0VarName.s01);
    t2 = __ocl_double2ToHalf2_rte($Arg0VarName.s23);
    t3 = __ocl_double2ToHalf2_rte($Arg0VarName.s45);
    t4 = __ocl_double2ToHalf2_rte($Arg0VarName.s67);
    t5 = __ocl_double2ToHalf2_rte($Arg0VarName.s89);
    t6 = __ocl_double2ToHalf2_rte($Arg0VarName.sAB);
    t7 = __ocl_double2ToHalf2_rte($Arg0VarName.sCD);
    t8 = __ocl_double2ToHalf2_rte($Arg0VarName.sEF);
    f16.s0 = t1.x;
    f16.s1 = t2.x;
    f16.s2 = t3.x;
    f16.s3 = t4.x;
    f16.s4 = t5.x;
    f16.s5 = t6.x;
    f16.s6 = t7.x;
    f16.s7 = t8.x;
    _mm_storeu_ps((private float*)(float*)$Arg2VarName, f16.lo.lo);
    $Arg2VarName = $Arg2VarName + 8;
    _mm_storeu_ps((private float*)(float*)$Arg2VarName, f16.lo.hi);
  }];





code Storef_half_rtn = [{
    float4 f4;
    f4.s0 = $Arg0VarName;
    f4 = __ocl_float2half_rtn(f4);
    ((short *)$Arg2VarName)[$Arg1VarName] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 0);
  }];
code Storef_half_rtn2 = [{
    float4 f4;
    f4.lo = $Arg0VarName;
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*2);
    f4 = __ocl_float2half_rtn(f4);
    *((float*)$Arg2VarName) = f4.s0;
  }];
code Storeaf_half_rtn3 = [{
    float4 f4;
    f4.s012 = $Arg0VarName;
    f4 = __ocl_float2half_rtn(f4);
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    _mm_storel_epi64((__m128i*)$Arg2VarName, __builtin_astype(f4,__m128i));
  }];
code Storef_half_rtn3 = [{
    float4 f4;
    f4.s012 = $Arg0VarName;
    f4 = __ocl_float2half_rtn(f4);
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*3);
    *((float*)$Arg2VarName) = f4.s0;
    ((short*)$Arg2VarName)[2] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 2);
  }];
code Storef_half_rtn4 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    $Arg0VarName = __ocl_float2half_rtn($Arg0VarName);
    *((float2 *)$Arg2VarName) = $Arg0VarName.lo;
  }];
code Storef_half_rtn8 = [{
    float4 res = __ocl_float2half8_rtn($Arg0VarName);
    vstore8(*(short8*)&res, $Arg1VarName, ($Arg2igentype)$Arg2VarName);
  }];
code Storef_half_rtn16 = [{
    float8 res;
    res.lo = __ocl_float2half8_rtn($Arg0VarName.lo);
    res.hi = __ocl_float2half8_rtn($Arg0VarName.hi);
    vstore16(*(short16*)&res, $Arg1VarName, ($Arg2igentype)$Arg2VarName);
  }];



code Stored_half_rtn = [{
    double2 d2 = (double2)(0.0);
    d2.x = $Arg0VarName;
    float4 f4 = __ocl_double2ToHalf2_rtn(d2);
    ((short *)$Arg2VarName)[$Arg1VarName] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 0);
  }];
code Stored_half_rtn2 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*2);
    float4 f4 = __ocl_double2ToHalf2_rtn($Arg0VarName);
    *((float*)$Arg2VarName) = f4.s0;
  }];
code Storead_half_rtn3 = [{
    float4 t1, t2, f4;
    t1 = __ocl_double2ToHalf2_rtn($Arg0VarName.xy);
    t2 = __ocl_double2ToHalf2_rtn($Arg0VarName.zz);
    f4.x = t1.x;
    f4.y = t2.x;
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    _mm_storel_epi64((__m128i*)$Arg2VarName, __builtin_astype(f4,__m128i));
  }];
code Stored_half_rtn3 = [{
    float4 t1, t2, f4;
    t1 = __ocl_double2ToHalf2_rtn($Arg0VarName.xy);
    t2 = __ocl_double2ToHalf2_rtn($Arg0VarName.zz);
    f4.x = t1.x;
    f4.y = t2.x;
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*3);
    *((float*)$Arg2VarName) = f4.s0;
    ((short *)$Arg2VarName)[2] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 2);
  }];
code Stored_half_rtn4 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    float4 t1;
    t1 = __ocl_double4ToHalf4_rtn($Arg0VarName);
    _mm_storel_epi64((__m128i*)$Arg2VarName, __builtin_astype(t1,__m128i));
  }];
code Storead_half_rtn8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8;
    float4 t1, t2;
    t1 = __ocl_double4ToHalf4_rtn($Arg0VarName.s0123);
    t2 = __ocl_double4ToHalf4_rtn($Arg0VarName.s4567);
    f8.xy = t1.lo;
    f8.zw = t2.lo;
    _mm_store_ps((float*)$Arg2VarName, f8.lo);
  }];
code Stored_half_rtn8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8;
    float4 t1, t2;
    t1 = __ocl_double4ToHalf4_rtn($Arg0VarName.s0123);
    t2 = __ocl_double4ToHalf4_rtn($Arg0VarName.s4567);
    f8.xy = t1.lo;
    f8.zw = t2.lo;
    _mm_storeu_ps((private float*)(float*)$Arg2VarName, f8.lo);
  }];
code Storead_half_rtn16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float16 f16;
    float4 t1, t2, t3, t4, t5, t6, t7, t8;
    t1 = __ocl_double2ToHalf2_rtn($Arg0VarName.s01);
    t2 = __ocl_double2ToHalf2_rtn($Arg0VarName.s23);
    t3 = __ocl_double2ToHalf2_rtn($Arg0VarName.s45);
    t4 = __ocl_double2ToHalf2_rtn($Arg0VarName.s67);
    t5 = __ocl_double2ToHalf2_rtn($Arg0VarName.s89);
    t6 = __ocl_double2ToHalf2_rtn($Arg0VarName.sAB);
    t7 = __ocl_double2ToHalf2_rtn($Arg0VarName.sCD);
    t8 = __ocl_double2ToHalf2_rtn($Arg0VarName.sEF);
    f16.s0 = t1.x;
    f16.s1 = t2.x;
    f16.s2 = t3.x;
    f16.s3 = t4.x;
    f16.s4 = t5.x;
    f16.s5 = t6.x;
    f16.s6 = t7.x;
    f16.s7 = t8.x;
    _mm_store_ps((float*)$Arg2VarName, f16.lo.lo);
    $Arg2VarName = $Arg2VarName + 8;
    _mm_store_ps((float*)$Arg2VarName, f16.lo.hi);
  }];
code Stored_half_rtn16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float16 f16;
    float4 t1, t2, t3, t4, t5, t6, t7, t8;
    t1 = __ocl_double2ToHalf2_rtn($Arg0VarName.s01);
    t2 = __ocl_double2ToHalf2_rtn($Arg0VarName.s23);
    t3 = __ocl_double2ToHalf2_rtn($Arg0VarName.s45);
    t4 = __ocl_double2ToHalf2_rtn($Arg0VarName.s67);
    t5 = __ocl_double2ToHalf2_rtn($Arg0VarName.s89);
    t6 = __ocl_double2ToHalf2_rtn($Arg0VarName.sAB);
    t7 = __ocl_double2ToHalf2_rtn($Arg0VarName.sCD);
    t8 = __ocl_double2ToHalf2_rtn($Arg0VarName.sEF);
    f16.s0 = t1.x;
    f16.s1 = t2.x;
    f16.s2 = t3.x;
    f16.s3 = t4.x;
    f16.s4 = t5.x;
    f16.s5 = t6.x;
    f16.s6 = t7.x;
    f16.s7 = t8.x;
    _mm_storeu_ps((private float*)(float*)$Arg2VarName, f16.lo.lo);
    $Arg2VarName = $Arg2VarName + 8;
    _mm_storeu_ps((private float*)(float*)$Arg2VarName, f16.lo.hi);
  }];


code Storef_half_rtp = [{
    float4 f4;
    f4.s0 = $Arg0VarName;
    f4 = __ocl_float2half_rtp(f4);
    ((short *)$Arg2VarName)[$Arg1VarName] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 0);
  }];
code Storef_half_rtp2 = [{
    float4 f4;
    f4.lo = $Arg0VarName;
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*2);
    f4 = __ocl_float2half_rtp(f4);
    *((float*)$Arg2VarName) = f4.s0;
  }];
code Storeaf_half_rtp3 = [{
    float4 f4;
    f4.s012 = $Arg0VarName;
    f4 = __ocl_float2half_rtp(f4);
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    _mm_storel_epi64((__m128i*)$Arg2VarName, __builtin_astype(f4,__m128i));
  }];
code Storef_half_rtp3 = [{
    float4 f4;
    f4.s012 = $Arg0VarName;
    f4 = __ocl_float2half_rtp(f4);
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*3);
    *((float*)$Arg2VarName) = f4.s0;
    ((short*)$Arg2VarName)[2] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 2);
  }];
code Storef_half_rtp4 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    $Arg0VarName = __ocl_float2half_rtp($Arg0VarName);
    *((float2 *)$Arg2VarName) = $Arg0VarName.lo;
  }];
code Storef_half_rtp8 = [{
    float4 res = __ocl_float2half8_rtp($Arg0VarName);
    vstore8(*(short8*)&res, $Arg1VarName, ($Arg2igentype)$Arg2VarName);
  }];
code Storef_half_rtp16 = [{
    float8 res;
    res.lo = __ocl_float2half8_rtp($Arg0VarName.lo);
    res.hi = __ocl_float2half8_rtp($Arg0VarName.hi);
    vstore16(*(short16*)&res, $Arg1VarName, ($Arg2igentype)$Arg2VarName);
  }];



code Stored_half_rtp = [{
    double2 d2 = (double2)(0.0);
    d2.x = $Arg0VarName;
    float4 f4 = __ocl_double2ToHalf2_rtp(d2);
    ((short *)$Arg2VarName)[$Arg1VarName] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 0);
  }];
code Stored_half_rtp2 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*2);
    float4 f4 = __ocl_double2ToHalf2_rtp($Arg0VarName);
    *((float*)$Arg2VarName) = f4.s0;
  }];
code Storead_half_rtp3 = [{
    float4 t1, t2, f4;
    t1 = __ocl_double2ToHalf2_rtp($Arg0VarName.xy);
    t2 = __ocl_double2ToHalf2_rtp($Arg0VarName.zz);
    f4.x = t1.x;
    f4.y = t2.x;
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    _mm_storel_epi64((__m128i*)$Arg2VarName, __builtin_astype(f4,__m128i));
  }];
code Stored_half_rtp3 = [{
    float4 t1, t2, f4;
    t1 = __ocl_double2ToHalf2_rtp($Arg0VarName.xy);
    t2 = __ocl_double2ToHalf2_rtp($Arg0VarName.zz);
    f4.x = t1.x;
    f4.y = t2.x;
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*3);
    *((float*)$Arg2VarName) = f4.s0;
    ((short *)$Arg2VarName)[2] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 2);
  }];
code Stored_half_rtp4 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    float4 t1;
    t1 = __ocl_double4ToHalf4_rtp($Arg0VarName);
    _mm_storel_epi64((__m128i*)$Arg2VarName, __builtin_astype(t1,__m128i));
  }];
code Storead_half_rtp8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8;
    float4 t1, t2;
    t1 = __ocl_double4ToHalf4_rtp($Arg0VarName.s0123);
    t2 = __ocl_double4ToHalf4_rtp($Arg0VarName.s4567);
    f8.xy = t1.lo;
    f8.zw = t2.lo;
    _mm_store_ps((float*)$Arg2VarName, f8.lo);
  }];
code Stored_half_rtp8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8;
    float4 t1, t2;
    t1 = __ocl_double4ToHalf4_rtp($Arg0VarName.s0123);
    t2 = __ocl_double4ToHalf4_rtp($Arg0VarName.s4567);
    f8.xy = t1.lo;
    f8.zw = t2.lo;
    _mm_storeu_ps((private float*)(float*)$Arg2VarName, f8.lo);
  }];
code Storead_half_rtp16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float16 f16;
    float4 t1, t2, t3, t4, t5, t6, t7, t8;
    t1 = __ocl_double2ToHalf2_rtp($Arg0VarName.s01);
    t2 = __ocl_double2ToHalf2_rtp($Arg0VarName.s23);
    t3 = __ocl_double2ToHalf2_rtp($Arg0VarName.s45);
    t4 = __ocl_double2ToHalf2_rtp($Arg0VarName.s67);
    t5 = __ocl_double2ToHalf2_rtp($Arg0VarName.s89);
    t6 = __ocl_double2ToHalf2_rtp($Arg0VarName.sAB);
    t7 = __ocl_double2ToHalf2_rtp($Arg0VarName.sCD);
    t8 = __ocl_double2ToHalf2_rtp($Arg0VarName.sEF);
    f16.s0 = t1.x;
    f16.s1 = t2.x;
    f16.s2 = t3.x;
    f16.s3 = t4.x;
    f16.s4 = t5.x;
    f16.s5 = t6.x;
    f16.s6 = t7.x;
    f16.s7 = t8.x;
    _mm_store_ps((float*)$Arg2VarName, f16.lo.lo);
    $Arg2VarName = $Arg2VarName + 8;
    _mm_store_ps((float*)$Arg2VarName, f16.lo.hi);
  }];
code Stored_half_rtp16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float16 f16;
    float4 t1, t2, t3, t4, t5, t6, t7, t8;
    t1 = __ocl_double2ToHalf2_rtp($Arg0VarName.s01);
    t2 = __ocl_double2ToHalf2_rtp($Arg0VarName.s23);
    t3 = __ocl_double2ToHalf2_rtp($Arg0VarName.s45);
    t4 = __ocl_double2ToHalf2_rtp($Arg0VarName.s67);
    t5 = __ocl_double2ToHalf2_rtp($Arg0VarName.s89);
    t6 = __ocl_double2ToHalf2_rtp($Arg0VarName.sAB);
    t7 = __ocl_double2ToHalf2_rtp($Arg0VarName.sCD);
    t8 = __ocl_double2ToHalf2_rtp($Arg0VarName.sEF);
    f16.s0 = t1.x;
    f16.s1 = t2.x;
    f16.s2 = t3.x;
    f16.s3 = t4.x;
    f16.s4 = t5.x;
    f16.s5 = t6.x;
    f16.s6 = t7.x;
    f16.s7 = t8.x;
    _mm_storeu_ps((private float*)(float*)$Arg2VarName, f16.lo.lo);
    $Arg2VarName = $Arg2VarName + 8;
    _mm_storeu_ps((private float*)(float*)$Arg2VarName, f16.lo.hi);
  }];





code Storef_half_rtz = [{
    float4 f4;
    f4.s0 = $Arg0VarName;
    f4 = __ocl_float2half_rtz(f4);
    ((short *)$Arg2VarName)[$Arg1VarName] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 0);
  }];
code Storef_half_rtz2 = [{
    float4 f4;
    f4.lo = $Arg0VarName;
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*2);
    f4 = __ocl_float2half_rtz(f4);
    *((float*)$Arg2VarName) = f4.s0;
  }];
code Storeaf_half_rtz3 = [{
    float4 f4;
    f4.s012 = $Arg0VarName;
    f4 = __ocl_float2half_rtz(f4);
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    _mm_storel_epi64((__m128i*)$Arg2VarName, __builtin_astype(f4,__m128i));
  }];
code Storef_half_rtz3 = [{
    float4 f4;
    f4.s012 = $Arg0VarName;
    f4 = __ocl_float2half_rtz(f4);
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*3);
    *((float*)$Arg2VarName) = f4.s0;
    ((short*)$Arg2VarName)[2] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 2);
  }];
code Storef_half_rtz4 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    $Arg0VarName = __ocl_float2half_rtz($Arg0VarName);
    *((float2 *)$Arg2VarName) = $Arg0VarName.lo;
  }];
code Storef_half_rtz8 = [{
    float4 res = __ocl_float2half8_rtz($Arg0VarName);
    vstore8(*(short8*)&res, $Arg1VarName, ($Arg2igentype)$Arg2VarName);
  }];
code Storef_half_rtz16 = [{
    float8 res;
    res.lo = __ocl_float2half8_rtz($Arg0VarName.lo);
    res.hi = __ocl_float2half8_rtz($Arg0VarName.hi);
    vstore16(*(short16*)&res, $Arg1VarName, ($Arg2igentype)$Arg2VarName);
  }];



code Stored_half_rtz = [{
    double2 d2 = (double2)(0.0);
    d2.x = $Arg0VarName;
    float4 f4 = __ocl_double2ToHalf2_rtz(d2);
    ((short *)$Arg2VarName)[$Arg1VarName] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 0);
  }];
code Stored_half_rtz2 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*2);
    float4 f4 = __ocl_double2ToHalf2_rtz($Arg0VarName);
    *((float*)$Arg2VarName) = f4.s0;
  }];
code Storead_half_rtz3 = [{
    float4 t1, t2, f4;
    t1 = __ocl_double2ToHalf2_rtz($Arg0VarName.xy);
    t2 = __ocl_double2ToHalf2_rtz($Arg0VarName.zz);
    f4.x = t1.x;
    f4.y = t2.x;
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    _mm_storel_epi64((__m128i*)$Arg2VarName, __builtin_astype(f4,__m128i));
  }];
code Stored_half_rtz3 = [{
    float4 t1, t2, f4;
    t1 = __ocl_double2ToHalf2_rtz($Arg0VarName.xy);
    t2 = __ocl_double2ToHalf2_rtz($Arg0VarName.zz);
    f4.x = t1.x;
    f4.y = t2.x;
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*3);
    *((float*)$Arg2VarName) = f4.s0;
    ((short *)$Arg2VarName)[2] = (short)_mm_extract_epi16( __builtin_astype(f4,__m128i), 2);
  }];
code Stored_half_rtz4 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*4);
    float4 t1;
    t1 = __ocl_double4ToHalf4_rtz($Arg0VarName);
    _mm_storel_epi64((__m128i*)$Arg2VarName, __builtin_astype(t1,__m128i));
  }];
code Storead_half_rtz8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8;
    float4 t1, t2;
    t1 = __ocl_double4ToHalf4_rtz($Arg0VarName.s0123);
    t2 = __ocl_double4ToHalf4_rtz($Arg0VarName.s4567);
    f8.xy = t1.lo;
    f8.zw = t2.lo;
    _mm_store_ps((float*)$Arg2VarName, f8.lo);
  }];
code Stored_half_rtz8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8;
    float4 t1, t2;
    t1 = __ocl_double4ToHalf4_rtz($Arg0VarName.s0123);
    t2 = __ocl_double4ToHalf4_rtz($Arg0VarName.s4567);
    f8.xy = t1.lo;
    f8.zw = t2.lo;
    _mm_storeu_ps((private float*)(float*)$Arg2VarName, f8.lo);
  }];
code Storead_half_rtz16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float16 f16;
    float4 t1, t2, t3, t4, t5, t6, t7, t8;
    t1 = __ocl_double2ToHalf2_rtz($Arg0VarName.s01);
    t2 = __ocl_double2ToHalf2_rtz($Arg0VarName.s23);
    t3 = __ocl_double2ToHalf2_rtz($Arg0VarName.s45);
    t4 = __ocl_double2ToHalf2_rtz($Arg0VarName.s67);
    t5 = __ocl_double2ToHalf2_rtz($Arg0VarName.s89);
    t6 = __ocl_double2ToHalf2_rtz($Arg0VarName.sAB);
    t7 = __ocl_double2ToHalf2_rtz($Arg0VarName.sCD);
    t8 = __ocl_double2ToHalf2_rtz($Arg0VarName.sEF);
    f16.s0 = t1.x;
    f16.s1 = t2.x;
    f16.s2 = t3.x;
    f16.s3 = t4.x;
    f16.s4 = t5.x;
    f16.s5 = t6.x;
    f16.s6 = t7.x;
    f16.s7 = t8.x;
    _mm_store_ps((float*)$Arg2VarName, f16.lo.lo);
    $Arg2VarName = $Arg2VarName + 8;
    _mm_store_ps((float*)$Arg2VarName, f16.lo.hi);
  }];
code Stored_half_rtz16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float16 f16;
    float4 t1, t2, t3, t4, t5, t6, t7, t8;
    t1 = __ocl_double2ToHalf2_rtz($Arg0VarName.s01);
    t2 = __ocl_double2ToHalf2_rtz($Arg0VarName.s23);
    t3 = __ocl_double2ToHalf2_rtz($Arg0VarName.s45);
    t4 = __ocl_double2ToHalf2_rtz($Arg0VarName.s67);
    t5 = __ocl_double2ToHalf2_rtz($Arg0VarName.s89);
    t6 = __ocl_double2ToHalf2_rtz($Arg0VarName.sAB);
    t7 = __ocl_double2ToHalf2_rtz($Arg0VarName.sCD);
    t8 = __ocl_double2ToHalf2_rtz($Arg0VarName.sEF);
    f16.s0 = t1.x;
    f16.s1 = t2.x;
    f16.s2 = t3.x;
    f16.s3 = t4.x;
    f16.s4 = t5.x;
    f16.s5 = t6.x;
    f16.s6 = t7.x;
    f16.s7 = t8.x;
    _mm_storeu_ps((private float*)(float*)$Arg2VarName, f16.lo.lo);
    $Arg2VarName = $Arg2VarName + 8;
    _mm_storeu_ps((private float*)(float*)$Arg2VarName, f16.lo.hi);
  }];














OclBuiltinImpl vloadcas = OclBuiltinImpl<vload_cas, [v2f16, v4f16, v8f16, v16f16, v2f32, v4f32, v8f32, v16f32, v2f64, v4f64, v8f64, v16f64,
                                                     v2i32, v4i32, v8i32, v16i32, v2i64, v4i64, v8i64, v16i64,
                                                     v2u32, v4u32, v8u32, v16u32, v2u64, v4u64, v8u64, v16u64,
                                                     v2i8,  v4i8, v8i8, v16i8, v2i16, v4i16, v8i16, v16i16,
                                                     v2u8,  v4u8, v8u8, v16u8, v2u16, v4u16, v8u16, v16u16], 0, LoadC>;
OclBuiltinImpl vloadgas = OclBuiltinImpl<vload_gas, [v2f16, v4f16, v8f16, v16f16, v2f32, v4f32, v8f32, v16f32, v2f64, v4f64, v8f64, v16f64,
                                                     v2i32, v4i32, v8i32, v16i32, v2i64, v4i64, v8i64, v16i64,
                                                     v2u32, v4u32, v8u32, v16u32, v2u64, v4u64, v8u64, v16u64,
                                                     v2i8,  v4i8, v8i8, v16i8, v2i16, v4i16, v8i16, v16i16,
                                                     v2u8,  v4u8, v8u8, v16u8, v2u16, v4u16, v8u16, v16u16], 0, Load>;
OclBuiltinImpl vloadlas = OclBuiltinImpl<vload_las, [v2f16, v4f16, v8f16, v16f16, v2f32, v4f32, v8f32, v16f32, v2f64, v4f64, v8f64, v16f64,
                                                     v2i32, v4i32, v8i32, v16i32, v2i64, v4i64, v8i64, v16i64,
                                                     v2u32, v4u32, v8u32, v16u32, v2u64, v4u64, v8u64, v16u64,
                                                     v2i8,  v4i8, v8i8, v16i8, v2i16, v4i16, v8i16, v16i16,
                                                     v2u8,  v4u8, v8u8, v16u8, v2u16, v4u16, v8u16, v16u16], 0, Load>;
OclBuiltinImpl vloadpas = OclBuiltinImpl<vload_pas, [v2f16, v4f16, v8f16, v16f16, v2f32, v4f32, v8f32, v16f32, v2f64, v4f64, v8f64, v16f64,
                                                     v2i32, v4i32, v8i32, v16i32, v2i64, v4i64, v8i64, v16i64,
                                                     v2u32, v4u32, v8u32, v16u32, v2u64, v4u64, v8u64, v16u64,
                                                     v2i8,  v4i8, v8i8, v16i8, v2i16, v4i16, v8i16, v16i16,
                                                     v2u8,  v4u8, v8u8, v16u8, v2u16, v4u16, v8u16, v16u16], 0, Load>;

OclBuiltinImpl vloadgen = OclBuiltinImpl<vload_gen, [v2f16, v4f16, v8f16, v16f16, v2f32, v4f32, v8f32, v16f32, v2f64, v4f64, v8f64, v16f64,
                                                     v2i32, v4i32, v8i32, v16i32, v2i64, v4i64, v8i64, v16i64,
                                                     v2u32, v4u32, v8u32, v16u32, v2u64, v4u64, v8u64, v16u64,
                                                     v2i8,  v4i8, v8i8, v16i8, v2i16, v4i16, v8i16, v16i16,
                                                     v2u8,  v4u8, v8u8, v16u8, v2u16, v4u16, v8u16, v16u16], 0, Load>;

OclBuiltinImpl vloadcas3 = OclBuiltinImpl<vload_cas, [v3i8, v3u8, v3f16, v3i16, v3u16, v3f32, v3i32, v3u32, v3f64, v3i64, v3u64], 0, LoadC3>;
OclBuiltinImpl vloadgas3 = OclBuiltinImpl<vload_gas, [v3i8, v3u8, v3f16, v3i16, v3u16, v3f32, v3i32, v3u32, v3f64, v3i64, v3u64], 0, Load3>;
OclBuiltinImpl vloadlas3 = OclBuiltinImpl<vload_las, [v3i8, v3u8, v3f16, v3i16, v3u16, v3f32, v3i32, v3u32, v3f64, v3i64, v3u64], 0, Load3>;
OclBuiltinImpl vloadpas3 = OclBuiltinImpl<vload_pas, [v3i8, v3u8, v3f16, v3i16, v3u16, v3f32, v3i32, v3u32, v3f64, v3i64, v3u64], 0, Load3>;
OclBuiltinImpl vloadgen3 = OclBuiltinImpl<vload_gen, [v3i8, v3u8, v3f16, v3i16, v3u16, v3f32, v3i32, v3u32, v3f64, v3i64, v3u64], 0, Load3>;


OclBuiltinImpl vload_halfcas = OclBuiltinImpl<vload_half_cas, [v1f16], 0, Load_const_half>;
OclBuiltinImpl vload_halfgas = OclBuiltinImpl<vload_half_gas, [v1f16], 0, Load_half>;
OclBuiltinImpl vload_halflas = OclBuiltinImpl<vload_half_las, [v1f16], 0, Load_half>;
OclBuiltinImpl vload_halfpas = OclBuiltinImpl<vload_half_pas, [v1f16], 0, Load_half>;
OclBuiltinImpl vload_halfgen = OclBuiltinImpl<vload_half_gen, [v1f16], 0, Load_half>;
OclBuiltinImpl vloada_halfcas = OclBuiltinImpl<vloada_half_cas, [v1f16], 0, Load_const_half>;
OclBuiltinImpl vloada_halfgas = OclBuiltinImpl<vloada_half_gas, [v1f16], 0, Load_half>;
OclBuiltinImpl vloada_halflas = OclBuiltinImpl<vloada_half_las, [v1f16], 0, Load_half>;
OclBuiltinImpl vloada_halfpas = OclBuiltinImpl<vloada_half_pas, [v1f16], 0, Load_half>;
OclBuiltinImpl vloada_halfgen = OclBuiltinImpl<vloada_half_gen, [v1f16], 0, Load_half>;

OclBuiltinImpl vload_half_cas2 = OclBuiltinImpl<vload_half_cas, [v2f16], 0, Load_const_half2>;
OclBuiltinImpl vload_half_gas2 = OclBuiltinImpl<vload_half_gas, [v2f16], 0, Load_half2>;
OclBuiltinImpl vload_half_las2 = OclBuiltinImpl<vload_half_las, [v2f16], 0, Load_half2>;
OclBuiltinImpl vload_half_pas2 = OclBuiltinImpl<vload_half_pas, [v2f16], 0, Load_half2>;
OclBuiltinImpl vload_half_gen2 = OclBuiltinImpl<vload_half_gen, [v2f16], 0, Load_half2>;
OclBuiltinImpl vloada_half_cas2 = OclBuiltinImpl<vloada_half_cas, [v2f16], 0, Load_const_half2>;
OclBuiltinImpl vloada_half_gas2 = OclBuiltinImpl<vloada_half_gas, [v2f16], 0, Load_half2>;
OclBuiltinImpl vloada_half_las2 = OclBuiltinImpl<vloada_half_las, [v2f16], 0, Load_half2>;
OclBuiltinImpl vloada_half_pas2 = OclBuiltinImpl<vloada_half_pas, [v2f16], 0, Load_half2>;
OclBuiltinImpl vloada_half_gen2 = OclBuiltinImpl<vloada_half_gen, [v2f16], 0, Load_half2>;

OclBuiltinImpl vload_half_cas3 = OclBuiltinImpl<vload_half_cas, [v3f16], 0, Load_const_half3>;
OclBuiltinImpl vload_half_gas3 = OclBuiltinImpl<vload_half_gas, [v3f16], 0, Load_half3>;
OclBuiltinImpl vload_half_las3 = OclBuiltinImpl<vload_half_las, [v3f16], 0, Load_half3>;
OclBuiltinImpl vload_half_pas3 = OclBuiltinImpl<vload_half_pas, [v3f16], 0, Load_half3>;
OclBuiltinImpl vload_half_gen3 = OclBuiltinImpl<vload_half_gen, [v3f16], 0, Load_half3>;
OclBuiltinImpl vloada_half_cas3 = OclBuiltinImpl<vloada_half_cas, [v3f16], 0, Loada_const_half3>;
OclBuiltinImpl vloada_half_gas3 = OclBuiltinImpl<vloada_half_gas, [v3f16], 0, Loada_half3>;
OclBuiltinImpl vloada_half_las3 = OclBuiltinImpl<vloada_half_las, [v3f16], 0, Loada_half3>;
OclBuiltinImpl vloada_half_pas3 = OclBuiltinImpl<vloada_half_pas, [v3f16], 0, Loada_half3>;
OclBuiltinImpl vloada_half_gen3 = OclBuiltinImpl<vloada_half_gen, [v3f16], 0, Loada_half3>;

OclBuiltinImpl vload_half_cas4 = OclBuiltinImpl<vload_half_cas, [v4f16], 0, Load_const_half4>;
OclBuiltinImpl vload_half_gas4 = OclBuiltinImpl<vload_half_gas, [v4f16], 0, Load_half4>;
OclBuiltinImpl vload_half_las4 = OclBuiltinImpl<vload_half_las, [v4f16], 0, Load_half4>;
OclBuiltinImpl vload_half_pas4 = OclBuiltinImpl<vload_half_pas, [v4f16], 0, Load_half4>;
OclBuiltinImpl vload_half_gen4 = OclBuiltinImpl<vload_half_gen, [v4f16], 0, Load_half4>;
OclBuiltinImpl vloada_half_cas4 = OclBuiltinImpl<vloada_half_cas, [v4f16], 0, Load_const_half4>;
OclBuiltinImpl vloada_half_gas4 = OclBuiltinImpl<vloada_half_gas, [v4f16], 0, Load_half4>;
OclBuiltinImpl vloada_half_las4 = OclBuiltinImpl<vloada_half_las, [v4f16], 0, Load_half4>;
OclBuiltinImpl vloada_half_pas4 = OclBuiltinImpl<vloada_half_pas, [v4f16], 0, Load_half4>;
OclBuiltinImpl vloada_half_gen4 = OclBuiltinImpl<vloada_half_gen, [v4f16], 0, Load_half4>;

OclBuiltinImpl vload_half_cas8 = OclBuiltinImpl<vload_half_cas, [v8f16], 0, Load_half8>;
OclBuiltinImpl vload_half_gas8 = OclBuiltinImpl<vload_half_gas, [v8f16], 0, Load_half8>;
OclBuiltinImpl vload_half_las8 = OclBuiltinImpl<vload_half_las, [v8f16], 0, Load_half8>;
OclBuiltinImpl vload_half_pas8 = OclBuiltinImpl<vload_half_pas, [v8f16], 0, Load_half8>;
OclBuiltinImpl vload_half_gen8 = OclBuiltinImpl<vload_half_gen, [v8f16], 0, Load_half8>;
OclBuiltinImpl vloada_half_cas8 = OclBuiltinImpl<vloada_half_cas, [v8f16], 0, Load_half8>;
OclBuiltinImpl vloada_half_gas8 = OclBuiltinImpl<vloada_half_gas, [v8f16], 0, Load_half8>;
OclBuiltinImpl vloada_half_las8 = OclBuiltinImpl<vloada_half_las, [v8f16], 0, Load_half8>;
OclBuiltinImpl vloada_half_pas8 = OclBuiltinImpl<vloada_half_pas, [v8f16], 0, Load_half8>;
OclBuiltinImpl vloada_half_gen8 = OclBuiltinImpl<vloada_half_gen, [v8f16], 0, Load_half8>;

OclBuiltinImpl vload_half_cas16 = OclBuiltinImpl<vload_half_cas, [v16f16], 0, Load_half16>;
OclBuiltinImpl vload_half_gas16 = OclBuiltinImpl<vload_half_gas, [v16f16], 0, Load_half16>;
OclBuiltinImpl vload_half_las16 = OclBuiltinImpl<vload_half_las, [v16f16], 0, Load_half16>;
OclBuiltinImpl vload_half_pas16 = OclBuiltinImpl<vload_half_pas, [v16f16], 0, Load_half16>;
OclBuiltinImpl vload_half_gen16 = OclBuiltinImpl<vload_half_gen, [v16f16], 0, Load_half16>;
OclBuiltinImpl vloada_half_cas16 = OclBuiltinImpl<vloada_half_cas, [v16f16], 0, Load_half16>;
OclBuiltinImpl vloada_half_gas16 = OclBuiltinImpl<vloada_half_gas, [v16f16], 0, Load_half16>;
OclBuiltinImpl vloada_half_las16 = OclBuiltinImpl<vloada_half_las, [v16f16], 0, Load_half16>;
OclBuiltinImpl vloada_half_pas16 = OclBuiltinImpl<vloada_half_pas, [v16f16], 0, Load_half16>;
OclBuiltinImpl vloada_half_gen16 = OclBuiltinImpl<vloada_half_gen, [v16f16], 0, Load_half16>;

OclBuiltinImpl vstoregas = OclBuiltinImpl<vstore_gas, [v2f16, v4f16, v8f16, v16f16, v2f32, v4f32, v8f32, v16f32, v2f64, v4f64, v8f64, v16f64,
                                                     v2i32, v4i32, v8i32, v16i32, v2i64, v4i64, v8i64, v16i64,
                                                     v2u32, v4u32, v8u32, v16u32, v2u64, v4u64, v8u64, v16u64,
                                                     v2i8,  v4i8, v8i8, v16i8, v2i16, v4i16, v8i16, v16i16,
                                                     v2u8,  v4u8, v8u8, v16u8, v2u16, v4u16, v8u16, v16u16], 0, Store>;
OclBuiltinImpl vstorelas = OclBuiltinImpl<vstore_las, [v2f16, v4f16, v8f16, v16f16, v2f32, v4f32, v8f32, v16f32, v2f64, v4f64, v8f64, v16f64,
                                                     v2i32, v4i32, v8i32, v16i32, v2i64, v4i64, v8i64, v16i64,
                                                     v2u32, v4u32, v8u32, v16u32, v2u64, v4u64, v8u64, v16u64,
                                                     v2i8,  v4i8, v8i8, v16i8, v2i16, v4i16, v8i16, v16i16,
                                                     v2u8,  v4u8, v8u8, v16u8, v2u16, v4u16, v8u16, v16u16], 0, Store>;
OclBuiltinImpl vstorepas = OclBuiltinImpl<vstore_pas, [v2f16, v4f16, v8f16, v16f16, v2f32, v4f32, v8f32, v16f32, v2f64, v4f64, v8f64, v16f64,
                                                     v2i32, v4i32, v8i32, v16i32, v2i64, v4i64, v8i64, v16i64,
                                                     v2u32, v4u32, v8u32, v16u32, v2u64, v4u64, v8u64, v16u64,
                                                     v2i8,  v4i8, v8i8, v16i8, v2i16, v4i16, v8i16, v16i16,
                                                     v2u8,  v4u8, v8u8, v16u8, v2u16, v4u16, v8u16, v16u16], 0, Store>;
OclBuiltinImpl vstoregen = OclBuiltinImpl<vstore_gen, [v2f16, v4f16, v8f16, v16f16, v2f32, v4f32, v8f32, v16f32, v2f64, v4f64, v8f64, v16f64,
                                                     v2i32, v4i32, v8i32, v16i32, v2i64, v4i64, v8i64, v16i64,
                                                     v2u32, v4u32, v8u32, v16u32, v2u64, v4u64, v8u64, v16u64,
                                                     v2i8,  v4i8, v8i8, v16i8, v2i16, v4i16, v8i16, v16i16,
                                                     v2u8,  v4u8, v8u8, v16u8, v2u16, v4u16, v8u16, v16u16], 0, Store>;

OclBuiltinImpl vstoregas3 = OclBuiltinImpl<vstore_gas, [v3i8, v3u8, v3f16, v3i16, v3u16, v3f32, v3i32, v3u32, v3f64, v3i64, v3u64], 0, Store3>;
OclBuiltinImpl vstorelas3 = OclBuiltinImpl<vstore_las, [v3i8, v3u8, v3f16, v3i16, v3u16, v3f32, v3i32, v3u32, v3f64, v3i64, v3u64], 0, Store3>;
OclBuiltinImpl vstorepas3 = OclBuiltinImpl<vstore_pas, [v3i8, v3u8, v3f16, v3i16, v3u16, v3f32, v3i32, v3u32, v3f64, v3i64, v3u64], 0, Store3>;
OclBuiltinImpl vstoregen3 = OclBuiltinImpl<vstore_gen, [v3i8, v3u8, v3f16, v3i16, v3u16, v3f32, v3i32, v3u32, v3f64, v3i64, v3u64], 0, Store3>;

OclBuiltinImpl vstoread_halfgas = OclBuiltinImpl<vstoread_half_gas, [v1f16], 0, Stored_half>;
OclBuiltinImpl vstoread_halflas = OclBuiltinImpl<vstoread_half_las, [v1f16], 0, Stored_half>;
OclBuiltinImpl vstoread_halfpas = OclBuiltinImpl<vstoread_half_pas, [v1f16], 0, Stored_half>;
OclBuiltinImpl vstoread_halfgen = OclBuiltinImpl<vstoread_half_gen, [v1f16], 0, Stored_half>;
OclBuiltinImpl vstoread_halfrte_gas = OclBuiltinImpl<vstoread_half_gas, [v1f16], 0, Stored_half_rte>;
OclBuiltinImpl vstoread_halfrte_las = OclBuiltinImpl<vstoread_half_las, [v1f16], 0, Stored_half_rte>;
OclBuiltinImpl vstoread_halfrte_pas = OclBuiltinImpl<vstoread_half_pas, [v1f16], 0, Stored_half_rte>;
OclBuiltinImpl vstoread_halfrte_gen = OclBuiltinImpl<vstoread_half_gen, [v1f16], 0, Stored_half_rte>;
OclBuiltinImpl vstoread_halfrtn_gas = OclBuiltinImpl<vstoread_half_gas, [v1f16], 0, Stored_half_rtn>;
OclBuiltinImpl vstoread_halfrtn_las = OclBuiltinImpl<vstoread_half_las, [v1f16], 0, Stored_half_rtn>;
OclBuiltinImpl vstoread_halfrtn_pas = OclBuiltinImpl<vstoread_half_pas, [v1f16], 0, Stored_half_rtn>;
OclBuiltinImpl vstoread_halfrtn_gen = OclBuiltinImpl<vstoread_half_gen, [v1f16], 0, Stored_half_rtn>;
OclBuiltinImpl vstoread_halfrtp_gas = OclBuiltinImpl<vstoread_half_gas, [v1f16], 0, Stored_half_rtp>;
OclBuiltinImpl vstoread_halfrtp_las = OclBuiltinImpl<vstoread_half_las, [v1f16], 0, Stored_half_rtp>;
OclBuiltinImpl vstoread_halfrtp_pas = OclBuiltinImpl<vstoread_half_pas, [v1f16], 0, Stored_half_rtp>;
OclBuiltinImpl vstoread_halfrtp_gen = OclBuiltinImpl<vstoread_half_gen, [v1f16], 0, Stored_half_rtp>;
OclBuiltinImpl vstoread_halfrtz_gas = OclBuiltinImpl<vstoread_half_gas, [v1f16], 0, Stored_half_rtz>;
OclBuiltinImpl vstoread_halfrtz_las = OclBuiltinImpl<vstoread_half_las, [v1f16], 0, Stored_half_rtz>;
OclBuiltinImpl vstoread_halfrtz_pas = OclBuiltinImpl<vstoread_half_pas, [v1f16], 0, Stored_half_rtz>;
OclBuiltinImpl vstoread_halfrtz_gen = OclBuiltinImpl<vstoread_half_gen, [v1f16], 0, Stored_half_rtz>;
OclBuiltinImpl vstoreaf_halfgas = OclBuiltinImpl<vstoreaf_half_gas, [v1f16], 0, Storef_half>;
OclBuiltinImpl vstoreaf_halflas = OclBuiltinImpl<vstoreaf_half_las, [v1f16], 0, Storef_half>;
OclBuiltinImpl vstoreaf_halfpas = OclBuiltinImpl<vstoreaf_half_pas, [v1f16], 0, Storef_half>;
OclBuiltinImpl vstoreaf_halfgen = OclBuiltinImpl<vstoreaf_half_gen, [v1f16], 0, Storef_half>;
OclBuiltinImpl vstoreaf_halfrte_gas = OclBuiltinImpl<vstoreaf_half_rte_gas, [v1f16], 0, Storef_half_rte>;
OclBuiltinImpl vstoreaf_halfrte_las = OclBuiltinImpl<vstoreaf_half_rte_las, [v1f16], 0, Storef_half_rte>;
OclBuiltinImpl vstoreaf_halfrte_pas = OclBuiltinImpl<vstoreaf_half_rte_pas, [v1f16], 0, Storef_half_rte>;
OclBuiltinImpl vstoreaf_halfrte_gen = OclBuiltinImpl<vstoreaf_half_rte_gen, [v1f16], 0, Storef_half_rte>;
OclBuiltinImpl vstoreaf_halfrtn_gas = OclBuiltinImpl<vstoreaf_half_rtn_gas, [v1f16], 0, Storef_half_rtn>;
OclBuiltinImpl vstoreaf_halfrtn_las = OclBuiltinImpl<vstoreaf_half_rtn_las, [v1f16], 0, Storef_half_rtn>;
OclBuiltinImpl vstoreaf_halfrtn_pas = OclBuiltinImpl<vstoreaf_half_rtn_pas, [v1f16], 0, Storef_half_rtn>;
OclBuiltinImpl vstoreaf_halfrtn_gen = OclBuiltinImpl<vstoreaf_half_rtn_gen, [v1f16], 0, Storef_half_rtn>;
OclBuiltinImpl vstoreaf_halfrtp_gas = OclBuiltinImpl<vstoreaf_half_rtp_gas, [v1f16], 0, Storef_half_rtp>;
OclBuiltinImpl vstoreaf_halfrtp_las = OclBuiltinImpl<vstoreaf_half_rtp_las, [v1f16], 0, Storef_half_rtp>;
OclBuiltinImpl vstoreaf_halfrtp_pas = OclBuiltinImpl<vstoreaf_half_rtp_pas, [v1f16], 0, Storef_half_rtp>;
OclBuiltinImpl vstoreaf_halfrtp_gen = OclBuiltinImpl<vstoreaf_half_rtp_gen, [v1f16], 0, Storef_half_rtp>;
OclBuiltinImpl vstoreaf_halfrtz_gas = OclBuiltinImpl<vstoreaf_half_rtz_gas, [v1f16], 0, Storef_half_rtz>;
OclBuiltinImpl vstoreaf_halfrtz_las = OclBuiltinImpl<vstoreaf_half_rtz_las, [v1f16], 0, Storef_half_rtz>;
OclBuiltinImpl vstoreaf_halfrtz_pas = OclBuiltinImpl<vstoreaf_half_rtz_pas, [v1f16], 0, Storef_half_rtz>;
OclBuiltinImpl vstoreaf_halfrtz_gen = OclBuiltinImpl<vstoreaf_half_rtz_gen, [v1f16], 0, Storef_half_rtz>;
OclBuiltinImpl vstored_halfgas = OclBuiltinImpl<vstored_half_gas, [v1f16], 0, Stored_half>;
OclBuiltinImpl vstored_halflas = OclBuiltinImpl<vstored_half_las, [v1f16], 0, Stored_half>;
OclBuiltinImpl vstored_halfpas = OclBuiltinImpl<vstored_half_pas, [v1f16], 0, Stored_half>;
OclBuiltinImpl vstored_halfgen = OclBuiltinImpl<vstored_half_gen, [v1f16], 0, Stored_half>;
OclBuiltinImpl vstored_halfrte_gas = OclBuiltinImpl<vstored_half_rte_gas, [v1f16], 0, Stored_half_rte>;
OclBuiltinImpl vstored_halfrte_las = OclBuiltinImpl<vstored_half_rte_las, [v1f16], 0, Stored_half_rte>;
OclBuiltinImpl vstored_halfrte_pas = OclBuiltinImpl<vstored_half_rte_pas, [v1f16], 0, Stored_half_rte>;
OclBuiltinImpl vstored_halfrte_gen = OclBuiltinImpl<vstored_half_rte_gen, [v1f16], 0, Stored_half_rte>;
OclBuiltinImpl vstored_halfrtn_gas = OclBuiltinImpl<vstored_half_rtn_gas, [v1f16], 0, Stored_half_rtn>;
OclBuiltinImpl vstored_halfrtn_las = OclBuiltinImpl<vstored_half_rtn_las, [v1f16], 0, Stored_half_rtn>;
OclBuiltinImpl vstored_halfrtn_pas = OclBuiltinImpl<vstored_half_rtn_pas, [v1f16], 0, Stored_half_rtn>;
OclBuiltinImpl vstored_halfrtn_gen = OclBuiltinImpl<vstored_half_rtn_gen, [v1f16], 0, Stored_half_rtn>;
OclBuiltinImpl vstored_halfrtp_gas = OclBuiltinImpl<vstored_half_rtp_gas, [v1f16], 0, Stored_half_rtp>;
OclBuiltinImpl vstored_halfrtp_las = OclBuiltinImpl<vstored_half_rtp_las, [v1f16], 0, Stored_half_rtp>;
OclBuiltinImpl vstored_halfrtp_pas = OclBuiltinImpl<vstored_half_rtp_pas, [v1f16], 0, Stored_half_rtp>;
OclBuiltinImpl vstored_halfrtp_gen = OclBuiltinImpl<vstored_half_rtp_gen, [v1f16], 0, Stored_half_rtp>;
OclBuiltinImpl vstored_halfrtz_gas = OclBuiltinImpl<vstored_half_rtz_gas, [v1f16], 0, Stored_half_rtz>;
OclBuiltinImpl vstored_halfrtz_las = OclBuiltinImpl<vstored_half_rtz_las, [v1f16], 0, Stored_half_rtz>;
OclBuiltinImpl vstored_halfrtz_pas = OclBuiltinImpl<vstored_half_rtz_pas, [v1f16], 0, Stored_half_rtz>;
OclBuiltinImpl vstored_halfrtz_gen = OclBuiltinImpl<vstored_half_rtz_gen, [v1f16], 0, Stored_half_rtz>;
OclBuiltinImpl vstoref_halfgas = OclBuiltinImpl<vstoref_half_gas, [v1f16], 0, Storef_half>;
OclBuiltinImpl vstoref_halflas = OclBuiltinImpl<vstoref_half_las, [v1f16], 0, Storef_half>;
OclBuiltinImpl vstoref_halfpas = OclBuiltinImpl<vstoref_half_pas, [v1f16], 0, Storef_half>;
OclBuiltinImpl vstoref_halfgen = OclBuiltinImpl<vstoref_half_gen, [v1f16], 0, Storef_half>;
OclBuiltinImpl vstoref_halfrte_gas = OclBuiltinImpl<vstoref_half_rte_gas, [v1f16], 0, Storef_half_rte>;
OclBuiltinImpl vstoref_halfrte_las = OclBuiltinImpl<vstoref_half_rte_las, [v1f16], 0, Storef_half_rte>;
OclBuiltinImpl vstoref_halfrte_pas = OclBuiltinImpl<vstoref_half_rte_pas, [v1f16], 0, Storef_half_rte>;
OclBuiltinImpl vstoref_halfrte_gen = OclBuiltinImpl<vstoref_half_rte_gen, [v1f16], 0, Storef_half_rte>;
OclBuiltinImpl vstoref_halfrtn_gas = OclBuiltinImpl<vstoref_half_rtn_gas, [v1f16], 0, Storef_half_rtn>;
OclBuiltinImpl vstoref_halfrtn_las = OclBuiltinImpl<vstoref_half_rtn_las, [v1f16], 0, Storef_half_rtn>;
OclBuiltinImpl vstoref_halfrtn_pas = OclBuiltinImpl<vstoref_half_rtn_pas, [v1f16], 0, Storef_half_rtn>;
OclBuiltinImpl vstoref_halfrtn_gen = OclBuiltinImpl<vstoref_half_rtn_gen, [v1f16], 0, Storef_half_rtn>;
OclBuiltinImpl vstoref_halfrtp_gas = OclBuiltinImpl<vstoref_half_rtp_gas, [v1f16], 0, Storef_half_rtp>;
OclBuiltinImpl vstoref_halfrtp_las = OclBuiltinImpl<vstoref_half_rtp_las, [v1f16], 0, Storef_half_rtp>;
OclBuiltinImpl vstoref_halfrtp_pas = OclBuiltinImpl<vstoref_half_rtp_pas, [v1f16], 0, Storef_half_rtp>;
OclBuiltinImpl vstoref_halfrtp_gen = OclBuiltinImpl<vstoref_half_rtp_gen, [v1f16], 0, Storef_half_rtp>;
OclBuiltinImpl vstoref_halfrtz_gas = OclBuiltinImpl<vstoref_half_rtz_gas, [v1f16], 0, Storef_half_rtz>;
OclBuiltinImpl vstoref_halfrtz_las = OclBuiltinImpl<vstoref_half_rtz_las, [v1f16], 0, Storef_half_rtz>;
OclBuiltinImpl vstoref_halfrtz_pas = OclBuiltinImpl<vstoref_half_rtz_pas, [v1f16], 0, Storef_half_rtz>;
OclBuiltinImpl vstoref_halfrtz_gen = OclBuiltinImpl<vstoref_half_rtz_gen, [v1f16], 0, Storef_half_rtz>;


OclBuiltinImpl vstoread_half_gas2 = OclBuiltinImpl<vstoread_half_gas, [v2f16], 0, Stored_half2>;
OclBuiltinImpl vstoread_half_las2 = OclBuiltinImpl<vstoread_half_las, [v2f16], 0, Stored_half2>;
OclBuiltinImpl vstoread_half_pas2 = OclBuiltinImpl<vstoread_half_pas, [v2f16], 0, Stored_half2>;
OclBuiltinImpl vstoread_half_gen2 = OclBuiltinImpl<vstoread_half_gen, [v2f16], 0, Stored_half2>;
OclBuiltinImpl vstoread_half_rte_gas2 = OclBuiltinImpl<vstoread_half_rte_gas, [v2f16], 0, Stored_half_rte2>;
OclBuiltinImpl vstoread_half_rte_las2 = OclBuiltinImpl<vstoread_half_rte_las, [v2f16], 0, Stored_half_rte2>;
OclBuiltinImpl vstoread_half_rte_pas2 = OclBuiltinImpl<vstoread_half_rte_pas, [v2f16], 0, Stored_half_rte2>;
OclBuiltinImpl vstoread_half_rte_gen2 = OclBuiltinImpl<vstoread_half_rte_gen, [v2f16], 0, Stored_half_rte2>;
OclBuiltinImpl vstoread_half_rtn_gas2 = OclBuiltinImpl<vstoread_half_rtn_gas, [v2f16], 0, Stored_half_rtn2>;
OclBuiltinImpl vstoread_half_rtn_las2 = OclBuiltinImpl<vstoread_half_rtn_las, [v2f16], 0, Stored_half_rtn2>;
OclBuiltinImpl vstoread_half_rtn_pas2 = OclBuiltinImpl<vstoread_half_rtn_pas, [v2f16], 0, Stored_half_rtn2>;
OclBuiltinImpl vstoread_half_rtn_gen2 = OclBuiltinImpl<vstoread_half_rtn_gen, [v2f16], 0, Stored_half_rtn2>;
OclBuiltinImpl vstoread_half_rtp_gas2 = OclBuiltinImpl<vstoread_half_rtp_gas, [v2f16], 0, Stored_half_rtp2>;
OclBuiltinImpl vstoread_half_rtp_las2 = OclBuiltinImpl<vstoread_half_rtp_las, [v2f16], 0, Stored_half_rtp2>;
OclBuiltinImpl vstoread_half_rtp_pas2 = OclBuiltinImpl<vstoread_half_rtp_pas, [v2f16], 0, Stored_half_rtp2>;
OclBuiltinImpl vstoread_half_rtp_gen2 = OclBuiltinImpl<vstoread_half_rtp_gen, [v2f16], 0, Stored_half_rtp2>;
OclBuiltinImpl vstoread_half_rtz_gas2 = OclBuiltinImpl<vstoread_half_rtz_gas, [v2f16], 0, Stored_half_rtz2>;
OclBuiltinImpl vstoread_half_rtz_las2 = OclBuiltinImpl<vstoread_half_rtz_las, [v2f16], 0, Stored_half_rtz2>;
OclBuiltinImpl vstoread_half_rtz_pas2 = OclBuiltinImpl<vstoread_half_rtz_pas, [v2f16], 0, Stored_half_rtz2>;
OclBuiltinImpl vstoread_half_rtz_gen2 = OclBuiltinImpl<vstoread_half_rtz_gen, [v2f16], 0, Stored_half_rtz2>;
OclBuiltinImpl vstoreaf_half_gas2 = OclBuiltinImpl<vstoreaf_half_gas, [v2f16], 0, Storef_half2>;
OclBuiltinImpl vstoreaf_half_las2 = OclBuiltinImpl<vstoreaf_half_las, [v2f16], 0, Storef_half2>;
OclBuiltinImpl vstoreaf_half_pas2 = OclBuiltinImpl<vstoreaf_half_pas, [v2f16], 0, Storef_half2>;
OclBuiltinImpl vstoreaf_half_gen2 = OclBuiltinImpl<vstoreaf_half_gen, [v2f16], 0, Storef_half2>;
OclBuiltinImpl vstoreaf_half_rte_gas2 = OclBuiltinImpl<vstoreaf_half_rte_gas, [v2f16], 0, Storef_half_rte2>;
OclBuiltinImpl vstoreaf_half_rte_las2 = OclBuiltinImpl<vstoreaf_half_rte_las, [v2f16], 0, Storef_half_rte2>;
OclBuiltinImpl vstoreaf_half_rte_pas2 = OclBuiltinImpl<vstoreaf_half_rte_pas, [v2f16], 0, Storef_half_rte2>;
OclBuiltinImpl vstoreaf_half_rte_gen2 = OclBuiltinImpl<vstoreaf_half_rte_gen, [v2f16], 0, Storef_half_rte2>;
OclBuiltinImpl vstoreaf_half_rtn_gas2 = OclBuiltinImpl<vstoreaf_half_rtn_gas, [v2f16], 0, Storef_half_rtn2>;
OclBuiltinImpl vstoreaf_half_rtn_las2 = OclBuiltinImpl<vstoreaf_half_rtn_las, [v2f16], 0, Storef_half_rtn2>;
OclBuiltinImpl vstoreaf_half_rtn_pas2 = OclBuiltinImpl<vstoreaf_half_rtn_pas, [v2f16], 0, Storef_half_rtn2>;
OclBuiltinImpl vstoreaf_half_rtn_gen2 = OclBuiltinImpl<vstoreaf_half_rtn_gen, [v2f16], 0, Storef_half_rtn2>;
OclBuiltinImpl vstoreaf_half_rtp_gas2 = OclBuiltinImpl<vstoreaf_half_rtp_gas, [v2f16], 0, Storef_half_rtp2>;
OclBuiltinImpl vstoreaf_half_rtp_las2 = OclBuiltinImpl<vstoreaf_half_rtp_las, [v2f16], 0, Storef_half_rtp2>;
OclBuiltinImpl vstoreaf_half_rtp_pas2 = OclBuiltinImpl<vstoreaf_half_rtp_pas, [v2f16], 0, Storef_half_rtp2>;
OclBuiltinImpl vstoreaf_half_rtp_gen2 = OclBuiltinImpl<vstoreaf_half_rtp_gen, [v2f16], 0, Storef_half_rtp2>;
OclBuiltinImpl vstoreaf_half_rtz_gas2 = OclBuiltinImpl<vstoreaf_half_rtz_gas, [v2f16], 0, Storef_half_rtz2>;
OclBuiltinImpl vstoreaf_half_rtz_las2 = OclBuiltinImpl<vstoreaf_half_rtz_las, [v2f16], 0, Storef_half_rtz2>;
OclBuiltinImpl vstoreaf_half_rtz_pas2 = OclBuiltinImpl<vstoreaf_half_rtz_pas, [v2f16], 0, Storef_half_rtz2>;
OclBuiltinImpl vstoreaf_half_rtz_gen2 = OclBuiltinImpl<vstoreaf_half_rtz_gen, [v2f16], 0, Storef_half_rtz2>;
OclBuiltinImpl vstored_half_gas2 = OclBuiltinImpl<vstored_half_gas, [v2f16], 0, Stored_half2>;
OclBuiltinImpl vstored_half_las2 = OclBuiltinImpl<vstored_half_las, [v2f16], 0, Stored_half2>;
OclBuiltinImpl vstored_half_pas2 = OclBuiltinImpl<vstored_half_pas, [v2f16], 0, Stored_half2>;
OclBuiltinImpl vstored_half_gen2 = OclBuiltinImpl<vstored_half_gen, [v2f16], 0, Stored_half2>;
OclBuiltinImpl vstored_half_rte_gas2 = OclBuiltinImpl<vstored_half_rte_gas, [v2f16], 0, Stored_half_rte2>;
OclBuiltinImpl vstored_half_rte_las2 = OclBuiltinImpl<vstored_half_rte_las, [v2f16], 0, Stored_half_rte2>;
OclBuiltinImpl vstored_half_rte_pas2 = OclBuiltinImpl<vstored_half_rte_pas, [v2f16], 0, Stored_half_rte2>;
OclBuiltinImpl vstored_half_rte_gen2 = OclBuiltinImpl<vstored_half_rte_gen, [v2f16], 0, Stored_half_rte2>;
OclBuiltinImpl vstored_half_rtn_gas2 = OclBuiltinImpl<vstored_half_rtn_gas, [v2f16], 0, Stored_half_rtn2>;
OclBuiltinImpl vstored_half_rtn_las2 = OclBuiltinImpl<vstored_half_rtn_las, [v2f16], 0, Stored_half_rtn2>;
OclBuiltinImpl vstored_half_rtn_pas2 = OclBuiltinImpl<vstored_half_rtn_pas, [v2f16], 0, Stored_half_rtn2>;
OclBuiltinImpl vstored_half_rtn_gen2 = OclBuiltinImpl<vstored_half_rtn_gen, [v2f16], 0, Stored_half_rtn2>;
OclBuiltinImpl vstored_half_rtp_gas2 = OclBuiltinImpl<vstored_half_rtp_gas, [v2f16], 0, Stored_half_rtp2>;
OclBuiltinImpl vstored_half_rtp_las2 = OclBuiltinImpl<vstored_half_rtp_las, [v2f16], 0, Stored_half_rtp2>;
OclBuiltinImpl vstored_half_rtp_pas2 = OclBuiltinImpl<vstored_half_rtp_pas, [v2f16], 0, Stored_half_rtp2>;
OclBuiltinImpl vstored_half_rtp_gen2 = OclBuiltinImpl<vstored_half_rtp_gen, [v2f16], 0, Stored_half_rtp2>;
OclBuiltinImpl vstored_half_rtz_gas2 = OclBuiltinImpl<vstored_half_rtz_gas, [v2f16], 0, Stored_half_rtz2>;
OclBuiltinImpl vstored_half_rtz_las2 = OclBuiltinImpl<vstored_half_rtz_las, [v2f16], 0, Stored_half_rtz2>;
OclBuiltinImpl vstored_half_rtz_pas2 = OclBuiltinImpl<vstored_half_rtz_pas, [v2f16], 0, Stored_half_rtz2>;
OclBuiltinImpl vstored_half_rtz_gen2 = OclBuiltinImpl<vstored_half_rtz_gen, [v2f16], 0, Stored_half_rtz2>;
OclBuiltinImpl vstoref_half_gas2 = OclBuiltinImpl<vstoref_half_gas, [v2f16], 0, Storef_half2>;
OclBuiltinImpl vstoref_half_las2 = OclBuiltinImpl<vstoref_half_las, [v2f16], 0, Storef_half2>;
OclBuiltinImpl vstoref_half_pas2 = OclBuiltinImpl<vstoref_half_pas, [v2f16], 0, Storef_half2>;
OclBuiltinImpl vstoref_half_gen2 = OclBuiltinImpl<vstoref_half_gen, [v2f16], 0, Storef_half2>;
OclBuiltinImpl vstoref_half_rte_gas2 = OclBuiltinImpl<vstoref_half_rte_gas, [v2f16], 0, Storef_half_rte2>;
OclBuiltinImpl vstoref_half_rte_las2 = OclBuiltinImpl<vstoref_half_rte_las, [v2f16], 0, Storef_half_rte2>;
OclBuiltinImpl vstoref_half_rte_pas2 = OclBuiltinImpl<vstoref_half_rte_pas, [v2f16], 0, Storef_half_rte2>;
OclBuiltinImpl vstoref_half_rte_gen2 = OclBuiltinImpl<vstoref_half_rte_gen, [v2f16], 0, Storef_half_rte2>;
OclBuiltinImpl vstoref_half_rtn_gas2 = OclBuiltinImpl<vstoref_half_rtn_gas, [v2f16], 0, Storef_half_rtn2>;
OclBuiltinImpl vstoref_half_rtn_las2 = OclBuiltinImpl<vstoref_half_rtn_las, [v2f16], 0, Storef_half_rtn2>;
OclBuiltinImpl vstoref_half_rtn_pas2 = OclBuiltinImpl<vstoref_half_rtn_pas, [v2f16], 0, Storef_half_rtn2>;
OclBuiltinImpl vstoref_half_rtn_gen2 = OclBuiltinImpl<vstoref_half_rtn_gen, [v2f16], 0, Storef_half_rtn2>;
OclBuiltinImpl vstoref_half_rtp_gas2 = OclBuiltinImpl<vstoref_half_rtp_gas, [v2f16], 0, Storef_half_rtp2>;
OclBuiltinImpl vstoref_half_rtp_las2 = OclBuiltinImpl<vstoref_half_rtp_las, [v2f16], 0, Storef_half_rtp2>;
OclBuiltinImpl vstoref_half_rtp_pas2 = OclBuiltinImpl<vstoref_half_rtp_pas, [v2f16], 0, Storef_half_rtp2>;
OclBuiltinImpl vstoref_half_rtp_gen2 = OclBuiltinImpl<vstoref_half_rtp_gen, [v2f16], 0, Storef_half_rtp2>;
OclBuiltinImpl vstoref_half_rtz_gas2 = OclBuiltinImpl<vstoref_half_rtz_gas, [v2f16], 0, Storef_half_rtz2>;
OclBuiltinImpl vstoref_half_rtz_las2 = OclBuiltinImpl<vstoref_half_rtz_las, [v2f16], 0, Storef_half_rtz2>;
OclBuiltinImpl vstoref_half_rtz_pas2 = OclBuiltinImpl<vstoref_half_rtz_pas, [v2f16], 0, Storef_half_rtz2>;
OclBuiltinImpl vstoref_half_rtz_gen2 = OclBuiltinImpl<vstoref_half_rtz_gen, [v2f16], 0, Storef_half_rtz2>;

OclBuiltinImpl vstoread_half_gas3 = OclBuiltinImpl<vstoread_half_gas, [v3f16], 0, Storead_half3>;
OclBuiltinImpl vstoread_half_las3 = OclBuiltinImpl<vstoread_half_las, [v3f16], 0, Storead_half3>;
OclBuiltinImpl vstoread_half_pas3 = OclBuiltinImpl<vstoread_half_pas, [v3f16], 0, Storead_half3>;
OclBuiltinImpl vstoread_half_gen3 = OclBuiltinImpl<vstoread_half_gen, [v3f16], 0, Storead_half3>;
OclBuiltinImpl vstoread_half_rte_gas3 = OclBuiltinImpl<vstoread_half_rte_gas, [v3f16], 0, Storead_half_rte3>;
OclBuiltinImpl vstoread_half_rte_las3 = OclBuiltinImpl<vstoread_half_rte_las, [v3f16], 0, Storead_half_rte3>;
OclBuiltinImpl vstoread_half_rte_pas3 = OclBuiltinImpl<vstoread_half_rte_pas, [v3f16], 0, Storead_half_rte3>;
OclBuiltinImpl vstoread_half_rte_gen3 = OclBuiltinImpl<vstoread_half_rte_gen, [v3f16], 0, Storead_half_rte3>;
OclBuiltinImpl vstoread_half_rtn_gas3 = OclBuiltinImpl<vstoread_half_rtn_gas, [v3f16], 0, Storead_half_rtn3>;
OclBuiltinImpl vstoread_half_rtn_las3 = OclBuiltinImpl<vstoread_half_rtn_las, [v3f16], 0, Storead_half_rtn3>;
OclBuiltinImpl vstoread_half_rtn_pas3 = OclBuiltinImpl<vstoread_half_rtn_pas, [v3f16], 0, Storead_half_rtn3>;
OclBuiltinImpl vstoread_half_rtn_gen3 = OclBuiltinImpl<vstoread_half_rtn_gen, [v3f16], 0, Storead_half_rtn3>;
OclBuiltinImpl vstoread_half_rtp_gas3 = OclBuiltinImpl<vstoread_half_rtp_gas, [v3f16], 0, Storead_half_rtp3>;
OclBuiltinImpl vstoread_half_rtp_las3 = OclBuiltinImpl<vstoread_half_rtp_las, [v3f16], 0, Storead_half_rtp3>;
OclBuiltinImpl vstoread_half_rtp_pas3 = OclBuiltinImpl<vstoread_half_rtp_pas, [v3f16], 0, Storead_half_rtp3>;
OclBuiltinImpl vstoread_half_rtp_gen3 = OclBuiltinImpl<vstoread_half_rtp_gen, [v3f16], 0, Storead_half_rtp3>;
OclBuiltinImpl vstoread_half_rtz_gas3 = OclBuiltinImpl<vstoread_half_rtz_gas, [v3f16], 0, Storead_half_rtz3>;
OclBuiltinImpl vstoread_half_rtz_las3 = OclBuiltinImpl<vstoread_half_rtz_las, [v3f16], 0, Storead_half_rtz3>;
OclBuiltinImpl vstoread_half_rtz_pas3 = OclBuiltinImpl<vstoread_half_rtz_pas, [v3f16], 0, Storead_half_rtz3>;
OclBuiltinImpl vstoread_half_rtz_gen3 = OclBuiltinImpl<vstoread_half_rtz_gen, [v3f16], 0, Storead_half_rtz3>;
OclBuiltinImpl vstoreaf_half_gas3 = OclBuiltinImpl<vstoreaf_half_gas, [v3f16], 0, Storeaf_half3>;
OclBuiltinImpl vstoreaf_half_las3 = OclBuiltinImpl<vstoreaf_half_las, [v3f16], 0, Storeaf_half3>;
OclBuiltinImpl vstoreaf_half_pas3 = OclBuiltinImpl<vstoreaf_half_pas, [v3f16], 0, Storeaf_half3>;
OclBuiltinImpl vstoreaf_half_gen3 = OclBuiltinImpl<vstoreaf_half_gen, [v3f16], 0, Storeaf_half3>;
OclBuiltinImpl vstoreaf_half_rte_gas3 = OclBuiltinImpl<vstoreaf_half_rte_gas, [v3f16], 0, Storeaf_half_rte3>;
OclBuiltinImpl vstoreaf_half_rte_las3 = OclBuiltinImpl<vstoreaf_half_rte_las, [v3f16], 0, Storeaf_half_rte3>;
OclBuiltinImpl vstoreaf_half_rte_pas3 = OclBuiltinImpl<vstoreaf_half_rte_pas, [v3f16], 0, Storeaf_half_rte3>;
OclBuiltinImpl vstoreaf_half_rte_gen3 = OclBuiltinImpl<vstoreaf_half_rte_gen, [v3f16], 0, Storeaf_half_rte3>;
OclBuiltinImpl vstoreaf_half_rtn_gas3 = OclBuiltinImpl<vstoreaf_half_rtn_gas, [v3f16], 0, Storeaf_half_rtn3>;
OclBuiltinImpl vstoreaf_half_rtn_las3 = OclBuiltinImpl<vstoreaf_half_rtn_las, [v3f16], 0, Storeaf_half_rtn3>;
OclBuiltinImpl vstoreaf_half_rtn_pas3 = OclBuiltinImpl<vstoreaf_half_rtn_pas, [v3f16], 0, Storeaf_half_rtn3>;
OclBuiltinImpl vstoreaf_half_rtn_gen3 = OclBuiltinImpl<vstoreaf_half_rtn_gen, [v3f16], 0, Storeaf_half_rtn3>;
OclBuiltinImpl vstoreaf_half_rtp_gas3 = OclBuiltinImpl<vstoreaf_half_rtp_gas, [v3f16], 0, Storeaf_half_rtp3>;
OclBuiltinImpl vstoreaf_half_rtp_las3 = OclBuiltinImpl<vstoreaf_half_rtp_las, [v3f16], 0, Storeaf_half_rtp3>;
OclBuiltinImpl vstoreaf_half_rtp_pas3 = OclBuiltinImpl<vstoreaf_half_rtp_pas, [v3f16], 0, Storeaf_half_rtp3>;
OclBuiltinImpl vstoreaf_half_rtp_gen3 = OclBuiltinImpl<vstoreaf_half_rtp_gen, [v3f16], 0, Storeaf_half_rtp3>;
OclBuiltinImpl vstoreaf_half_rtz_gas3 = OclBuiltinImpl<vstoreaf_half_rtz_gas, [v3f16], 0, Storeaf_half_rtz3>;
OclBuiltinImpl vstoreaf_half_rtz_las3 = OclBuiltinImpl<vstoreaf_half_rtz_las, [v3f16], 0, Storeaf_half_rtz3>;
OclBuiltinImpl vstoreaf_half_rtz_pas3 = OclBuiltinImpl<vstoreaf_half_rtz_pas, [v3f16], 0, Storeaf_half_rtz3>;
OclBuiltinImpl vstoreaf_half_rtz_gen3 = OclBuiltinImpl<vstoreaf_half_rtz_gen, [v3f16], 0, Storeaf_half_rtz3>;
OclBuiltinImpl vstored_half_gas3 = OclBuiltinImpl<vstored_half_gas, [v3f16], 0, Stored_half3>;
OclBuiltinImpl vstored_half_las3 = OclBuiltinImpl<vstored_half_las, [v3f16], 0, Stored_half3>;
OclBuiltinImpl vstored_half_pas3 = OclBuiltinImpl<vstored_half_pas, [v3f16], 0, Stored_half3>;
OclBuiltinImpl vstored_half_gen3 = OclBuiltinImpl<vstored_half_gen, [v3f16], 0, Stored_half3>;
OclBuiltinImpl vstored_half_rte_gas3 = OclBuiltinImpl<vstored_half_rte_gas, [v3f16], 0, Stored_half_rte3>;
OclBuiltinImpl vstored_half_rte_las3 = OclBuiltinImpl<vstored_half_rte_las, [v3f16], 0, Stored_half_rte3>;
OclBuiltinImpl vstored_half_rte_pas3 = OclBuiltinImpl<vstored_half_rte_pas, [v3f16], 0, Stored_half_rte3>;
OclBuiltinImpl vstored_half_rte_gen3 = OclBuiltinImpl<vstored_half_rte_gen, [v3f16], 0, Stored_half_rte3>;
OclBuiltinImpl vstored_half_rtn_gas3 = OclBuiltinImpl<vstored_half_rtn_gas, [v3f16], 0, Stored_half_rtn3>;
OclBuiltinImpl vstored_half_rtn_las3 = OclBuiltinImpl<vstored_half_rtn_las, [v3f16], 0, Stored_half_rtn3>;
OclBuiltinImpl vstored_half_rtn_pas3 = OclBuiltinImpl<vstored_half_rtn_pas, [v3f16], 0, Stored_half_rtn3>;
OclBuiltinImpl vstored_half_rtn_gen3 = OclBuiltinImpl<vstored_half_rtn_gen, [v3f16], 0, Stored_half_rtn3>;
OclBuiltinImpl vstored_half_rtp_gas3 = OclBuiltinImpl<vstored_half_rtp_gas, [v3f16], 0, Stored_half_rtp3>;
OclBuiltinImpl vstored_half_rtp_las3 = OclBuiltinImpl<vstored_half_rtp_las, [v3f16], 0, Stored_half_rtp3>;
OclBuiltinImpl vstored_half_rtp_pas3 = OclBuiltinImpl<vstored_half_rtp_pas, [v3f16], 0, Stored_half_rtp3>;
OclBuiltinImpl vstored_half_rtp_gen3 = OclBuiltinImpl<vstored_half_rtp_gen, [v3f16], 0, Stored_half_rtp3>;
OclBuiltinImpl vstored_half_rtz_gas3 = OclBuiltinImpl<vstored_half_rtz_gas, [v3f16], 0, Stored_half_rtz3>;
OclBuiltinImpl vstored_half_rtz_las3 = OclBuiltinImpl<vstored_half_rtz_las, [v3f16], 0, Stored_half_rtz3>;
OclBuiltinImpl vstored_half_rtz_pas3 = OclBuiltinImpl<vstored_half_rtz_pas, [v3f16], 0, Stored_half_rtz3>;
OclBuiltinImpl vstored_half_rtz_gen3 = OclBuiltinImpl<vstored_half_rtz_gen, [v3f16], 0, Stored_half_rtz3>;
OclBuiltinImpl vstoref_half_gas3 = OclBuiltinImpl<vstoref_half_gas, [v3f16], 0, Storef_half3>;
OclBuiltinImpl vstoref_half_las3 = OclBuiltinImpl<vstoref_half_las, [v3f16], 0, Storef_half3>;
OclBuiltinImpl vstoref_half_pas3 = OclBuiltinImpl<vstoref_half_pas, [v3f16], 0, Storef_half3>;
OclBuiltinImpl vstoref_half_gen3 = OclBuiltinImpl<vstoref_half_gen, [v3f16], 0, Storef_half3>;
OclBuiltinImpl vstoref_half_rte_gas3 = OclBuiltinImpl<vstoref_half_rte_gas, [v3f16], 0, Storef_half_rte3>;
OclBuiltinImpl vstoref_half_rte_las3 = OclBuiltinImpl<vstoref_half_rte_las, [v3f16], 0, Storef_half_rte3>;
OclBuiltinImpl vstoref_half_rte_pas3 = OclBuiltinImpl<vstoref_half_rte_pas, [v3f16], 0, Storef_half_rte3>;
OclBuiltinImpl vstoref_half_rte_gen3 = OclBuiltinImpl<vstoref_half_rte_gen, [v3f16], 0, Storef_half_rte3>;
OclBuiltinImpl vstoref_half_rtn_gas3 = OclBuiltinImpl<vstoref_half_rtn_gas, [v3f16], 0, Storef_half_rtn3>;
OclBuiltinImpl vstoref_half_rtn_las3 = OclBuiltinImpl<vstoref_half_rtn_las, [v3f16], 0, Storef_half_rtn3>;
OclBuiltinImpl vstoref_half_rtn_pas3 = OclBuiltinImpl<vstoref_half_rtn_pas, [v3f16], 0, Storef_half_rtn3>;
OclBuiltinImpl vstoref_half_rtn_gen3 = OclBuiltinImpl<vstoref_half_rtn_gen, [v3f16], 0, Storef_half_rtn3>;
OclBuiltinImpl vstoref_half_rtp_gas3 = OclBuiltinImpl<vstoref_half_rtp_gas, [v3f16], 0, Storef_half_rtp3>;
OclBuiltinImpl vstoref_half_rtp_las3 = OclBuiltinImpl<vstoref_half_rtp_las, [v3f16], 0, Storef_half_rtp3>;
OclBuiltinImpl vstoref_half_rtp_pas3 = OclBuiltinImpl<vstoref_half_rtp_pas, [v3f16], 0, Storef_half_rtp3>;
OclBuiltinImpl vstoref_half_rtp_gen3 = OclBuiltinImpl<vstoref_half_rtp_gen, [v3f16], 0, Storef_half_rtp3>;
OclBuiltinImpl vstoref_half_rtz_gas3 = OclBuiltinImpl<vstoref_half_rtz_gas, [v3f16], 0, Storef_half_rtz3>;
OclBuiltinImpl vstoref_half_rtz_las3 = OclBuiltinImpl<vstoref_half_rtz_las, [v3f16], 0, Storef_half_rtz3>;
OclBuiltinImpl vstoref_half_rtz_pas3 = OclBuiltinImpl<vstoref_half_rtz_pas, [v3f16], 0, Storef_half_rtz3>;
OclBuiltinImpl vstoref_half_rtz_gen3 = OclBuiltinImpl<vstoref_half_rtz_gen, [v3f16], 0, Storef_half_rtz3>;

OclBuiltinImpl vstoread_half_gas4 = OclBuiltinImpl<vstoread_half_gas, [v4f16], 0, Stored_half4>;
OclBuiltinImpl vstoread_half_las4 = OclBuiltinImpl<vstoread_half_las, [v4f16], 0, Stored_half4>;
OclBuiltinImpl vstoread_half_pas4 = OclBuiltinImpl<vstoread_half_pas, [v4f16], 0, Stored_half4>;
OclBuiltinImpl vstoread_half_gen4 = OclBuiltinImpl<vstoread_half_gen, [v4f16], 0, Stored_half4>;
OclBuiltinImpl vstoread_half_rte_gas4 = OclBuiltinImpl<vstoread_half_rte_gas, [v4f16], 0, Stored_half_rte4>;
OclBuiltinImpl vstoread_half_rte_las4 = OclBuiltinImpl<vstoread_half_rte_las, [v4f16], 0, Stored_half_rte4>;
OclBuiltinImpl vstoread_half_rte_pas4 = OclBuiltinImpl<vstoread_half_rte_pas, [v4f16], 0, Stored_half_rte4>;
OclBuiltinImpl vstoread_half_rte_gen4 = OclBuiltinImpl<vstoread_half_rte_gen, [v4f16], 0, Stored_half_rte4>;
OclBuiltinImpl vstoread_half_rtn_gas4 = OclBuiltinImpl<vstoread_half_rtn_gas, [v4f16], 0, Stored_half_rtn4>;
OclBuiltinImpl vstoread_half_rtn_las4 = OclBuiltinImpl<vstoread_half_rtn_las, [v4f16], 0, Stored_half_rtn4>;
OclBuiltinImpl vstoread_half_rtn_pas4 = OclBuiltinImpl<vstoread_half_rtn_pas, [v4f16], 0, Stored_half_rtn4>;
OclBuiltinImpl vstoread_half_rtn_gen4 = OclBuiltinImpl<vstoread_half_rtn_gen, [v4f16], 0, Stored_half_rtn4>;
OclBuiltinImpl vstoread_half_rtp_gas4 = OclBuiltinImpl<vstoread_half_rtp_gas, [v4f16], 0, Stored_half_rtp4>;
OclBuiltinImpl vstoread_half_rtp_las4 = OclBuiltinImpl<vstoread_half_rtp_las, [v4f16], 0, Stored_half_rtp4>;
OclBuiltinImpl vstoread_half_rtp_pas4 = OclBuiltinImpl<vstoread_half_rtp_pas, [v4f16], 0, Stored_half_rtp4>;
OclBuiltinImpl vstoread_half_rtp_gen4 = OclBuiltinImpl<vstoread_half_rtp_gen, [v4f16], 0, Stored_half_rtp4>;
OclBuiltinImpl vstoread_half_rtz_gas4 = OclBuiltinImpl<vstoread_half_rtz_gas, [v4f16], 0, Stored_half_rtz4>;
OclBuiltinImpl vstoread_half_rtz_las4 = OclBuiltinImpl<vstoread_half_rtz_las, [v4f16], 0, Stored_half_rtz4>;
OclBuiltinImpl vstoread_half_rtz_pas4 = OclBuiltinImpl<vstoread_half_rtz_pas, [v4f16], 0, Stored_half_rtz4>;
OclBuiltinImpl vstoread_half_rtz_gen4 = OclBuiltinImpl<vstoread_half_rtz_gen, [v4f16], 0, Stored_half_rtz4>;
OclBuiltinImpl vstoreaf_half_gas4 = OclBuiltinImpl<vstoreaf_half_gas, [v4f16], 0, Storef_half4>;
OclBuiltinImpl vstoreaf_half_las4 = OclBuiltinImpl<vstoreaf_half_las, [v4f16], 0, Storef_half4>;
OclBuiltinImpl vstoreaf_half_pas4 = OclBuiltinImpl<vstoreaf_half_pas, [v4f16], 0, Storef_half4>;
OclBuiltinImpl vstoreaf_half_gen4 = OclBuiltinImpl<vstoreaf_half_gen, [v4f16], 0, Storef_half4>;
OclBuiltinImpl vstoreaf_half_rte_gas4 = OclBuiltinImpl<vstoreaf_half_rte_gas, [v4f16], 0, Storef_half_rte4>;
OclBuiltinImpl vstoreaf_half_rte_las4 = OclBuiltinImpl<vstoreaf_half_rte_las, [v4f16], 0, Storef_half_rte4>;
OclBuiltinImpl vstoreaf_half_rte_pas4 = OclBuiltinImpl<vstoreaf_half_rte_pas, [v4f16], 0, Storef_half_rte4>;
OclBuiltinImpl vstoreaf_half_rte_gen4 = OclBuiltinImpl<vstoreaf_half_rte_gen, [v4f16], 0, Storef_half_rte4>;
OclBuiltinImpl vstoreaf_half_rtn_gas4 = OclBuiltinImpl<vstoreaf_half_rtn_gas, [v4f16], 0, Storef_half_rtn4>;
OclBuiltinImpl vstoreaf_half_rtn_las4 = OclBuiltinImpl<vstoreaf_half_rtn_las, [v4f16], 0, Storef_half_rtn4>;
OclBuiltinImpl vstoreaf_half_rtn_pas4 = OclBuiltinImpl<vstoreaf_half_rtn_pas, [v4f16], 0, Storef_half_rtn4>;
OclBuiltinImpl vstoreaf_half_rtn_gen4 = OclBuiltinImpl<vstoreaf_half_rtn_gen, [v4f16], 0, Storef_half_rtn4>;
OclBuiltinImpl vstoreaf_half_rtp_gas4 = OclBuiltinImpl<vstoreaf_half_rtp_gas, [v4f16], 0, Storef_half_rtp4>;
OclBuiltinImpl vstoreaf_half_rtp_las4 = OclBuiltinImpl<vstoreaf_half_rtp_las, [v4f16], 0, Storef_half_rtp4>;
OclBuiltinImpl vstoreaf_half_rtp_pas4 = OclBuiltinImpl<vstoreaf_half_rtp_pas, [v4f16], 0, Storef_half_rtp4>;
OclBuiltinImpl vstoreaf_half_rtp_gen4 = OclBuiltinImpl<vstoreaf_half_rtp_gen, [v4f16], 0, Storef_half_rtp4>;
OclBuiltinImpl vstoreaf_half_rtz_gas4 = OclBuiltinImpl<vstoreaf_half_rtz_gas, [v4f16], 0, Storef_half_rtz4>;
OclBuiltinImpl vstoreaf_half_rtz_las4 = OclBuiltinImpl<vstoreaf_half_rtz_las, [v4f16], 0, Storef_half_rtz4>;
OclBuiltinImpl vstoreaf_half_rtz_pas4 = OclBuiltinImpl<vstoreaf_half_rtz_pas, [v4f16], 0, Storef_half_rtz4>;
OclBuiltinImpl vstoreaf_half_rtz_gen4 = OclBuiltinImpl<vstoreaf_half_rtz_gen, [v4f16], 0, Storef_half_rtz4>;
OclBuiltinImpl vstored_half_gas4 = OclBuiltinImpl<vstored_half_gas, [v4f16], 0, Stored_half4>;
OclBuiltinImpl vstored_half_las4 = OclBuiltinImpl<vstored_half_las, [v4f16], 0, Stored_half4>;
OclBuiltinImpl vstored_half_pas4 = OclBuiltinImpl<vstored_half_pas, [v4f16], 0, Stored_half4>;
OclBuiltinImpl vstored_half_gen4 = OclBuiltinImpl<vstored_half_gen, [v4f16], 0, Stored_half4>;
OclBuiltinImpl vstored_half_rte_gas4 = OclBuiltinImpl<vstored_half_rte_gas, [v4f16], 0, Stored_half_rte4>;
OclBuiltinImpl vstored_half_rte_las4 = OclBuiltinImpl<vstored_half_rte_las, [v4f16], 0, Stored_half_rte4>;
OclBuiltinImpl vstored_half_rte_pas4 = OclBuiltinImpl<vstored_half_rte_pas, [v4f16], 0, Stored_half_rte4>;
OclBuiltinImpl vstored_half_rte_gen4 = OclBuiltinImpl<vstored_half_rte_gen, [v4f16], 0, Stored_half_rte4>;
OclBuiltinImpl vstored_half_rtn_gas4 = OclBuiltinImpl<vstored_half_rtn_gas, [v4f16], 0, Stored_half_rtn4>;
OclBuiltinImpl vstored_half_rtn_las4 = OclBuiltinImpl<vstored_half_rtn_las, [v4f16], 0, Stored_half_rtn4>;
OclBuiltinImpl vstored_half_rtn_pas4 = OclBuiltinImpl<vstored_half_rtn_pas, [v4f16], 0, Stored_half_rtn4>;
OclBuiltinImpl vstored_half_rtn_gen4 = OclBuiltinImpl<vstored_half_rtn_gen, [v4f16], 0, Stored_half_rtn4>;
OclBuiltinImpl vstored_half_rtp_gas4 = OclBuiltinImpl<vstored_half_rtp_gas, [v4f16], 0, Stored_half_rtp4>;
OclBuiltinImpl vstored_half_rtp_las4 = OclBuiltinImpl<vstored_half_rtp_las, [v4f16], 0, Stored_half_rtp4>;
OclBuiltinImpl vstored_half_rtp_pas4 = OclBuiltinImpl<vstored_half_rtp_pas, [v4f16], 0, Stored_half_rtp4>;
OclBuiltinImpl vstored_half_rtp_gen4 = OclBuiltinImpl<vstored_half_rtp_gen, [v4f16], 0, Stored_half_rtp4>;
OclBuiltinImpl vstored_half_rtz_gas4 = OclBuiltinImpl<vstored_half_rtz_gas, [v4f16], 0, Stored_half_rtz4>;
OclBuiltinImpl vstored_half_rtz_las4 = OclBuiltinImpl<vstored_half_rtz_las, [v4f16], 0, Stored_half_rtz4>;
OclBuiltinImpl vstored_half_rtz_pas4 = OclBuiltinImpl<vstored_half_rtz_pas, [v4f16], 0, Stored_half_rtz4>;
OclBuiltinImpl vstored_half_rtz_gen4 = OclBuiltinImpl<vstored_half_rtz_gen, [v4f16], 0, Stored_half_rtz4>;
OclBuiltinImpl vstoref_half_gas4 = OclBuiltinImpl<vstoref_half_gas, [v4f16], 0, Storef_half4>;
OclBuiltinImpl vstoref_half_las4 = OclBuiltinImpl<vstoref_half_las, [v4f16], 0, Storef_half4>;
OclBuiltinImpl vstoref_half_pas4 = OclBuiltinImpl<vstoref_half_pas, [v4f16], 0, Storef_half4>;
OclBuiltinImpl vstoref_half_gen4 = OclBuiltinImpl<vstoref_half_gen, [v4f16], 0, Storef_half4>;
OclBuiltinImpl vstoref_half_rte_gas4 = OclBuiltinImpl<vstoref_half_rte_gas, [v4f16], 0, Storef_half_rte4>;
OclBuiltinImpl vstoref_half_rte_las4 = OclBuiltinImpl<vstoref_half_rte_las, [v4f16], 0, Storef_half_rte4>;
OclBuiltinImpl vstoref_half_rte_pas4 = OclBuiltinImpl<vstoref_half_rte_pas, [v4f16], 0, Storef_half_rte4>;
OclBuiltinImpl vstoref_half_rte_gen4 = OclBuiltinImpl<vstoref_half_rte_gen, [v4f16], 0, Storef_half_rte4>;
OclBuiltinImpl vstoref_half_rtn_gas4 = OclBuiltinImpl<vstoref_half_rtn_gas, [v4f16], 0, Storef_half_rtn4>;
OclBuiltinImpl vstoref_half_rtn_las4 = OclBuiltinImpl<vstoref_half_rtn_las, [v4f16], 0, Storef_half_rtn4>;
OclBuiltinImpl vstoref_half_rtn_pas4 = OclBuiltinImpl<vstoref_half_rtn_pas, [v4f16], 0, Storef_half_rtn4>;
OclBuiltinImpl vstoref_half_rtn_gen4 = OclBuiltinImpl<vstoref_half_rtn_gen, [v4f16], 0, Storef_half_rtn4>;
OclBuiltinImpl vstoref_half_rtp_gas4 = OclBuiltinImpl<vstoref_half_rtp_gas, [v4f16], 0, Storef_half_rtp4>;
OclBuiltinImpl vstoref_half_rtp_las4 = OclBuiltinImpl<vstoref_half_rtp_las, [v4f16], 0, Storef_half_rtp4>;
OclBuiltinImpl vstoref_half_rtp_pas4 = OclBuiltinImpl<vstoref_half_rtp_pas, [v4f16], 0, Storef_half_rtp4>;
OclBuiltinImpl vstoref_half_rtp_gen4 = OclBuiltinImpl<vstoref_half_rtp_gen, [v4f16], 0, Storef_half_rtp4>;
OclBuiltinImpl vstoref_half_rtz_gas4 = OclBuiltinImpl<vstoref_half_rtz_gas, [v4f16], 0, Storef_half_rtz4>;
OclBuiltinImpl vstoref_half_rtz_las4 = OclBuiltinImpl<vstoref_half_rtz_las, [v4f16], 0, Storef_half_rtz4>;
OclBuiltinImpl vstoref_half_rtz_pas4 = OclBuiltinImpl<vstoref_half_rtz_pas, [v4f16], 0, Storef_half_rtz4>;
OclBuiltinImpl vstoref_half_rtz_gen4 = OclBuiltinImpl<vstoref_half_rtz_gen, [v4f16], 0, Storef_half_rtz4>;

OclBuiltinImpl vstoread_half_gas8 = OclBuiltinImpl<vstoread_half_gas, [v8f16], 0, Storead_half8>;
OclBuiltinImpl vstoread_half_las8 = OclBuiltinImpl<vstoread_half_las, [v8f16], 0, Storead_half8>;
OclBuiltinImpl vstoread_half_pas8 = OclBuiltinImpl<vstoread_half_pas, [v8f16], 0, Storead_half8>;
OclBuiltinImpl vstoread_half_gen8 = OclBuiltinImpl<vstoread_half_gen, [v8f16], 0, Storead_half8>;
OclBuiltinImpl vstoread_half_rte_gas8 = OclBuiltinImpl<vstoread_half_rte_gas, [v8f16], 0, Storead_half_rte8>;
OclBuiltinImpl vstoread_half_rte_las8 = OclBuiltinImpl<vstoread_half_rte_las, [v8f16], 0, Storead_half_rte8>;
OclBuiltinImpl vstoread_half_rte_pas8 = OclBuiltinImpl<vstoread_half_rte_pas, [v8f16], 0, Storead_half_rte8>;
OclBuiltinImpl vstoread_half_rte_gen8 = OclBuiltinImpl<vstoread_half_rte_gen, [v8f16], 0, Storead_half_rte8>;
OclBuiltinImpl vstoread_half_rtn_gas8 = OclBuiltinImpl<vstoread_half_rtn_gas, [v8f16], 0, Storead_half_rtn8>;
OclBuiltinImpl vstoread_half_rtn_las8 = OclBuiltinImpl<vstoread_half_rtn_las, [v8f16], 0, Storead_half_rtn8>;
OclBuiltinImpl vstoread_half_rtn_pas8 = OclBuiltinImpl<vstoread_half_rtn_pas, [v8f16], 0, Storead_half_rtn8>;
OclBuiltinImpl vstoread_half_rtn_gen8 = OclBuiltinImpl<vstoread_half_rtn_gen, [v8f16], 0, Storead_half_rtn8>;
OclBuiltinImpl vstoread_half_rtp_gas8 = OclBuiltinImpl<vstoread_half_rtp_gas, [v8f16], 0, Storead_half_rtp8>;
OclBuiltinImpl vstoread_half_rtp_las8 = OclBuiltinImpl<vstoread_half_rtp_las, [v8f16], 0, Storead_half_rtp8>;
OclBuiltinImpl vstoread_half_rtp_pas8 = OclBuiltinImpl<vstoread_half_rtp_pas, [v8f16], 0, Storead_half_rtp8>;
OclBuiltinImpl vstoread_half_rtp_gen8 = OclBuiltinImpl<vstoread_half_rtp_gen, [v8f16], 0, Storead_half_rtp8>;
OclBuiltinImpl vstoread_half_rtz_gas8 = OclBuiltinImpl<vstoread_half_rtz_gas, [v8f16], 0, Storead_half_rtz8>;
OclBuiltinImpl vstoread_half_rtz_las8 = OclBuiltinImpl<vstoread_half_rtz_las, [v8f16], 0, Storead_half_rtz8>;
OclBuiltinImpl vstoread_half_rtz_pas8 = OclBuiltinImpl<vstoread_half_rtz_pas, [v8f16], 0, Storead_half_rtz8>;
OclBuiltinImpl vstoread_half_rtz_gen8 = OclBuiltinImpl<vstoread_half_rtz_gen, [v8f16], 0, Storead_half_rtz8>;
OclBuiltinImpl vstoreaf_half_gas8 = OclBuiltinImpl<vstoreaf_half_gas, [v8f16], 0, Storef_half8>;
OclBuiltinImpl vstoreaf_half_las8 = OclBuiltinImpl<vstoreaf_half_las, [v8f16], 0, Storef_half8>;
OclBuiltinImpl vstoreaf_half_pas8 = OclBuiltinImpl<vstoreaf_half_pas, [v8f16], 0, Storef_half8>;
OclBuiltinImpl vstoreaf_half_gen8 = OclBuiltinImpl<vstoreaf_half_gen, [v8f16], 0, Storef_half8>;
OclBuiltinImpl vstoreaf_half_rte_gas8 = OclBuiltinImpl<vstoreaf_half_rte_gas, [v8f16], 0, Storef_half_rte8>;
OclBuiltinImpl vstoreaf_half_rte_las8 = OclBuiltinImpl<vstoreaf_half_rte_las, [v8f16], 0, Storef_half_rte8>;
OclBuiltinImpl vstoreaf_half_rte_pas8 = OclBuiltinImpl<vstoreaf_half_rte_pas, [v8f16], 0, Storef_half_rte8>;
OclBuiltinImpl vstoreaf_half_rte_gen8 = OclBuiltinImpl<vstoreaf_half_rte_gen, [v8f16], 0, Storef_half_rte8>;
OclBuiltinImpl vstoreaf_half_rtn_gas8 = OclBuiltinImpl<vstoreaf_half_rtn_gas, [v8f16], 0, Storef_half_rtn8>;
OclBuiltinImpl vstoreaf_half_rtn_las8 = OclBuiltinImpl<vstoreaf_half_rtn_las, [v8f16], 0, Storef_half_rtn8>;
OclBuiltinImpl vstoreaf_half_rtn_pas8 = OclBuiltinImpl<vstoreaf_half_rtn_pas, [v8f16], 0, Storef_half_rtn8>;
OclBuiltinImpl vstoreaf_half_rtn_gen8 = OclBuiltinImpl<vstoreaf_half_rtn_gen, [v8f16], 0, Storef_half_rtn8>;
OclBuiltinImpl vstoreaf_half_rtp_gas8 = OclBuiltinImpl<vstoreaf_half_rtp_gas, [v8f16], 0, Storef_half_rtp8>;
OclBuiltinImpl vstoreaf_half_rtp_las8 = OclBuiltinImpl<vstoreaf_half_rtp_las, [v8f16], 0, Storef_half_rtp8>;
OclBuiltinImpl vstoreaf_half_rtp_pas8 = OclBuiltinImpl<vstoreaf_half_rtp_pas, [v8f16], 0, Storef_half_rtp8>;
OclBuiltinImpl vstoreaf_half_rtp_gen8 = OclBuiltinImpl<vstoreaf_half_rtp_gen, [v8f16], 0, Storef_half_rtp8>;
OclBuiltinImpl vstoreaf_half_rtz_gas8 = OclBuiltinImpl<vstoreaf_half_rtz_gas, [v8f16], 0, Storef_half_rtz8>;
OclBuiltinImpl vstoreaf_half_rtz_las8 = OclBuiltinImpl<vstoreaf_half_rtz_las, [v8f16], 0, Storef_half_rtz8>;
OclBuiltinImpl vstoreaf_half_rtz_pas8 = OclBuiltinImpl<vstoreaf_half_rtz_pas, [v8f16], 0, Storef_half_rtz8>;
OclBuiltinImpl vstoreaf_half_rtz_gen8 = OclBuiltinImpl<vstoreaf_half_rtz_gen, [v8f16], 0, Storef_half_rtz8>;
OclBuiltinImpl vstored_half_gas8 = OclBuiltinImpl<vstored_half_gas, [v8f16], 0, Stored_half8>;
OclBuiltinImpl vstored_half_las8 = OclBuiltinImpl<vstored_half_las, [v8f16], 0, Stored_half8 >;
OclBuiltinImpl vstored_half_pas8 = OclBuiltinImpl<vstored_half_pas, [v8f16], 0, Stored_half8>;
OclBuiltinImpl vstored_half_gen8 = OclBuiltinImpl<vstored_half_gen, [v8f16], 0, Stored_half8>;
OclBuiltinImpl vstored_half_rte_gas8 = OclBuiltinImpl<vstored_half_rte_gas, [v8f16], 0, Stored_half_rte8>;
OclBuiltinImpl vstored_half_rte_las8 = OclBuiltinImpl<vstored_half_rte_las, [v8f16], 0, Stored_half_rte8>;
OclBuiltinImpl vstored_half_rte_pas8 = OclBuiltinImpl<vstored_half_rte_pas, [v8f16], 0, Stored_half_rte8>;
OclBuiltinImpl vstored_half_rte_gen8 = OclBuiltinImpl<vstored_half_rte_gen, [v8f16], 0, Stored_half_rte8>;
OclBuiltinImpl vstored_half_rtn_gas8 = OclBuiltinImpl<vstored_half_rtn_gas, [v8f16], 0, Stored_half_rtn8>;
OclBuiltinImpl vstored_half_rtn_las8 = OclBuiltinImpl<vstored_half_rtn_las, [v8f16], 0, Stored_half_rtn8>;
OclBuiltinImpl vstored_half_rtn_pas8 = OclBuiltinImpl<vstored_half_rtn_pas, [v8f16], 0, Stored_half_rtn8>;
OclBuiltinImpl vstored_half_rtn_gen8 = OclBuiltinImpl<vstored_half_rtn_gen, [v8f16], 0, Stored_half_rtn8>;
OclBuiltinImpl vstored_half_rtp_gas8 = OclBuiltinImpl<vstored_half_rtp_gas, [v8f16], 0, Stored_half_rtp8>;
OclBuiltinImpl vstored_half_rtp_las8 = OclBuiltinImpl<vstored_half_rtp_las, [v8f16], 0, Stored_half_rtp8>;
OclBuiltinImpl vstored_half_rtp_pas8 = OclBuiltinImpl<vstored_half_rtp_pas, [v8f16], 0, Stored_half_rtp8>;
OclBuiltinImpl vstored_half_rtp_gen8 = OclBuiltinImpl<vstored_half_rtp_gen, [v8f16], 0, Stored_half_rtp8>;
OclBuiltinImpl vstored_half_rtz_gas8 = OclBuiltinImpl<vstored_half_rtz_gas, [v8f16], 0, Stored_half_rtz8>;
OclBuiltinImpl vstored_half_rtz_las8 = OclBuiltinImpl<vstored_half_rtz_las, [v8f16], 0, Stored_half_rtz8>;
OclBuiltinImpl vstored_half_rtz_pas8 = OclBuiltinImpl<vstored_half_rtz_pas, [v8f16], 0, Stored_half_rtz8>;
OclBuiltinImpl vstored_half_rtz_gen8 = OclBuiltinImpl<vstored_half_rtz_gen, [v8f16], 0, Stored_half_rtz8>;
OclBuiltinImpl vstoref_half_gas8 = OclBuiltinImpl<vstoref_half_gas, [v8f16], 0, Storef_half8>;
OclBuiltinImpl vstoref_half_las8 = OclBuiltinImpl<vstoref_half_las, [v8f16], 0, Storef_half8>;
OclBuiltinImpl vstoref_half_pas8 = OclBuiltinImpl<vstoref_half_pas, [v8f16], 0, Storef_half8>;
OclBuiltinImpl vstoref_half_gen8 = OclBuiltinImpl<vstoref_half_gen, [v8f16], 0, Storef_half8>;
OclBuiltinImpl vstoref_half_rte_gas8 = OclBuiltinImpl<vstoref_half_rte_gas, [v8f16], 0, Storef_half_rte8>;
OclBuiltinImpl vstoref_half_rte_las8 = OclBuiltinImpl<vstoref_half_rte_las, [v8f16], 0, Storef_half_rte8>;
OclBuiltinImpl vstoref_half_rte_pas8 = OclBuiltinImpl<vstoref_half_rte_pas, [v8f16], 0, Storef_half_rte8>;
OclBuiltinImpl vstoref_half_rte_gen8 = OclBuiltinImpl<vstoref_half_rte_gen, [v8f16], 0, Storef_half_rte8>;
OclBuiltinImpl vstoref_half_rtn_gas8 = OclBuiltinImpl<vstoref_half_rtn_gas, [v8f16], 0, Storef_half_rtn8>;
OclBuiltinImpl vstoref_half_rtn_las8 = OclBuiltinImpl<vstoref_half_rtn_las, [v8f16], 0, Storef_half_rtn8>;
OclBuiltinImpl vstoref_half_rtn_pas8 = OclBuiltinImpl<vstoref_half_rtn_pas, [v8f16], 0, Storef_half_rtn8>;
OclBuiltinImpl vstoref_half_rtn_gen8 = OclBuiltinImpl<vstoref_half_rtn_gen, [v8f16], 0, Storef_half_rtn8>;
OclBuiltinImpl vstoref_half_rtp_gas8 = OclBuiltinImpl<vstoref_half_rtp_gas, [v8f16], 0, Storef_half_rtp8>;
OclBuiltinImpl vstoref_half_rtp_las8 = OclBuiltinImpl<vstoref_half_rtp_las, [v8f16], 0, Storef_half_rtp8>;
OclBuiltinImpl vstoref_half_rtp_pas8 = OclBuiltinImpl<vstoref_half_rtp_pas, [v8f16], 0, Storef_half_rtp8>;
OclBuiltinImpl vstoref_half_rtp_gen8 = OclBuiltinImpl<vstoref_half_rtp_gen, [v8f16], 0, Storef_half_rtp8>;
OclBuiltinImpl vstoref_half_rtz_gas8 = OclBuiltinImpl<vstoref_half_rtz_gas, [v8f16], 0, Storef_half_rtz8>;
OclBuiltinImpl vstoref_half_rtz_las8 = OclBuiltinImpl<vstoref_half_rtz_las, [v8f16], 0, Storef_half_rtz8>;
OclBuiltinImpl vstoref_half_rtz_pas8 = OclBuiltinImpl<vstoref_half_rtz_pas, [v8f16], 0, Storef_half_rtz8>;
OclBuiltinImpl vstoref_half_rtz_gen8 = OclBuiltinImpl<vstoref_half_rtz_gen, [v8f16], 0, Storef_half_rtz8>;

OclBuiltinImpl vstoread_half_gas16 = OclBuiltinImpl<vstoread_half_gas, [v16f16], 0, Storead_half16>;
OclBuiltinImpl vstoread_half_las16 = OclBuiltinImpl<vstoread_half_las, [v16f16], 0, Storead_half16>;
OclBuiltinImpl vstoread_half_pas16 = OclBuiltinImpl<vstoread_half_pas, [v16f16], 0, Storead_half16>;
OclBuiltinImpl vstoread_half_gen16 = OclBuiltinImpl<vstoread_half_gen, [v16f16], 0, Storead_half16>;
OclBuiltinImpl vstoread_half_rte_gas16 = OclBuiltinImpl<vstoread_half_rte_gas, [v16f16], 0, Storead_half_rte16>;
OclBuiltinImpl vstoread_half_rte_las16 = OclBuiltinImpl<vstoread_half_rte_las, [v16f16], 0, Storead_half_rte16>;
OclBuiltinImpl vstoread_half_rte_pas16 = OclBuiltinImpl<vstoread_half_rte_pas, [v16f16], 0, Storead_half_rte16>;
OclBuiltinImpl vstoread_half_rte_gen16 = OclBuiltinImpl<vstoread_half_rte_gen, [v16f16], 0, Storead_half_rte16>;
OclBuiltinImpl vstoread_half_rtn_gas16 = OclBuiltinImpl<vstoread_half_rtn_gas, [v16f16], 0, Storead_half_rtn16>;
OclBuiltinImpl vstoread_half_rtn_las16 = OclBuiltinImpl<vstoread_half_rtn_las, [v16f16], 0, Storead_half_rtn16>;
OclBuiltinImpl vstoread_half_rtn_pas16 = OclBuiltinImpl<vstoread_half_rtn_pas, [v16f16], 0, Storead_half_rtn16>;
OclBuiltinImpl vstoread_half_rtn_gen16 = OclBuiltinImpl<vstoread_half_rtn_gen, [v16f16], 0, Storead_half_rtn16>;
OclBuiltinImpl vstoread_half_rtp_gas16 = OclBuiltinImpl<vstoread_half_rtp_gas, [v16f16], 0, Storead_half_rtp16>;
OclBuiltinImpl vstoread_half_rtp_las16 = OclBuiltinImpl<vstoread_half_rtp_las, [v16f16], 0, Storead_half_rtp16>;
OclBuiltinImpl vstoread_half_rtp_pas16 = OclBuiltinImpl<vstoread_half_rtp_pas, [v16f16], 0, Storead_half_rtp16>;
OclBuiltinImpl vstoread_half_rtp_gen16 = OclBuiltinImpl<vstoread_half_rtp_gen, [v16f16], 0, Storead_half_rtp16>;
OclBuiltinImpl vstoread_half_rtz_gas16 = OclBuiltinImpl<vstoread_half_rtz_gas, [v16f16], 0, Storead_half_rtz16>;
OclBuiltinImpl vstoread_half_rtz_las16 = OclBuiltinImpl<vstoread_half_rtz_las, [v16f16], 0, Storead_half_rtz16>;
OclBuiltinImpl vstoread_half_rtz_pas16 = OclBuiltinImpl<vstoread_half_rtz_pas, [v16f16], 0, Storead_half_rtz16>;
OclBuiltinImpl vstoread_half_rtz_gen16 = OclBuiltinImpl<vstoread_half_rtz_gen, [v16f16], 0, Storead_half_rtz16>;
OclBuiltinImpl vstoreaf_half_gas16 = OclBuiltinImpl<vstoreaf_half_gas, [v16f16], 0, Storef_half16>;
OclBuiltinImpl vstoreaf_half_las16 = OclBuiltinImpl<vstoreaf_half_las, [v16f16], 0, Storef_half16>;
OclBuiltinImpl vstoreaf_half_pas16 = OclBuiltinImpl<vstoreaf_half_pas, [v16f16], 0, Storef_half16>;
OclBuiltinImpl vstoreaf_half_gen16 = OclBuiltinImpl<vstoreaf_half_gen, [v16f16], 0, Storef_half16>;
OclBuiltinImpl vstoreaf_half_rte_gas16 = OclBuiltinImpl<vstoreaf_half_rte_gas, [v16f16], 0, Storef_half_rte16>;
OclBuiltinImpl vstoreaf_half_rte_las16 = OclBuiltinImpl<vstoreaf_half_rte_las, [v16f16], 0, Storef_half_rte16>;
OclBuiltinImpl vstoreaf_half_rte_pas16 = OclBuiltinImpl<vstoreaf_half_rte_pas, [v16f16], 0, Storef_half_rte16>;
OclBuiltinImpl vstoreaf_half_rte_gen16 = OclBuiltinImpl<vstoreaf_half_rte_gen, [v16f16], 0, Storef_half_rte16>;
OclBuiltinImpl vstoreaf_half_rtn_gas16 = OclBuiltinImpl<vstoreaf_half_rtn_gas, [v16f16], 0, Storef_half_rtn16>;
OclBuiltinImpl vstoreaf_half_rtn_las16 = OclBuiltinImpl<vstoreaf_half_rtn_las, [v16f16], 0, Storef_half_rtn16>;
OclBuiltinImpl vstoreaf_half_rtn_pas16 = OclBuiltinImpl<vstoreaf_half_rtn_pas, [v16f16], 0, Storef_half_rtn16>;
OclBuiltinImpl vstoreaf_half_rtn_gen16 = OclBuiltinImpl<vstoreaf_half_rtn_gen, [v16f16], 0, Storef_half_rtn16>;
OclBuiltinImpl vstoreaf_half_rtp_gas16 = OclBuiltinImpl<vstoreaf_half_rtp_gas, [v16f16], 0, Storef_half_rtp16>;
OclBuiltinImpl vstoreaf_half_rtp_las16 = OclBuiltinImpl<vstoreaf_half_rtp_las, [v16f16], 0, Storef_half_rtp16>;
OclBuiltinImpl vstoreaf_half_rtp_pas16 = OclBuiltinImpl<vstoreaf_half_rtp_pas, [v16f16], 0, Storef_half_rtp16>;
OclBuiltinImpl vstoreaf_half_rtp_gen16 = OclBuiltinImpl<vstoreaf_half_rtp_gen, [v16f16], 0, Storef_half_rtp16>;
OclBuiltinImpl vstoreaf_half_rtz_gas16 = OclBuiltinImpl<vstoreaf_half_rtz_gas, [v16f16], 0, Storef_half_rtz16>;
OclBuiltinImpl vstoreaf_half_rtz_las16 = OclBuiltinImpl<vstoreaf_half_rtz_las, [v16f16], 0, Storef_half_rtz16>;
OclBuiltinImpl vstoreaf_half_rtz_pas16 = OclBuiltinImpl<vstoreaf_half_rtz_pas, [v16f16], 0, Storef_half_rtz16>;
OclBuiltinImpl vstoreaf_half_rtz_gen16 = OclBuiltinImpl<vstoreaf_half_rtz_gen, [v16f16], 0, Storef_half_rtz16>;
OclBuiltinImpl vstored_half_gas16 = OclBuiltinImpl<vstored_half_gas, [v16f16], 0, Stored_half16>;
OclBuiltinImpl vstored_half_las16 = OclBuiltinImpl<vstored_half_las, [v16f16], 0, Stored_half16>;
OclBuiltinImpl vstored_half_pas16 = OclBuiltinImpl<vstored_half_pas, [v16f16], 0, Stored_half16>;
OclBuiltinImpl vstored_half_gen16 = OclBuiltinImpl<vstored_half_gen, [v16f16], 0, Stored_half16>;
OclBuiltinImpl vstored_half_rte_gas16 = OclBuiltinImpl<vstored_half_rte_gas, [v16f16], 0, Stored_half_rte16>;
OclBuiltinImpl vstored_half_rte_las16 = OclBuiltinImpl<vstored_half_rte_las, [v16f16], 0, Stored_half_rte16>;
OclBuiltinImpl vstored_half_rte_pas16 = OclBuiltinImpl<vstored_half_rte_pas, [v16f16], 0, Stored_half_rte16>;
OclBuiltinImpl vstored_half_rte_gen16 = OclBuiltinImpl<vstored_half_rte_gen, [v16f16], 0, Stored_half_rte16>;
OclBuiltinImpl vstored_half_rtn_gas16 = OclBuiltinImpl<vstored_half_rtn_gas, [v16f16], 0, Stored_half_rtn16>;
OclBuiltinImpl vstored_half_rtn_las16 = OclBuiltinImpl<vstored_half_rtn_las, [v16f16], 0, Stored_half_rtn16>;
OclBuiltinImpl vstored_half_rtn_pas16 = OclBuiltinImpl<vstored_half_rtn_pas, [v16f16], 0, Stored_half_rtn16>;
OclBuiltinImpl vstored_half_rtn_gen16 = OclBuiltinImpl<vstored_half_rtn_gen, [v16f16], 0, Stored_half_rtn16>;
OclBuiltinImpl vstored_half_rtp_gas16 = OclBuiltinImpl<vstored_half_rtp_gas, [v16f16], 0, Stored_half_rtp16>;
OclBuiltinImpl vstored_half_rtp_las16 = OclBuiltinImpl<vstored_half_rtp_las, [v16f16], 0, Stored_half_rtp16>;
OclBuiltinImpl vstored_half_rtp_pas16 = OclBuiltinImpl<vstored_half_rtp_pas, [v16f16], 0, Stored_half_rtp16>;
OclBuiltinImpl vstored_half_rtp_gen16 = OclBuiltinImpl<vstored_half_rtp_gen, [v16f16], 0, Stored_half_rtp16>;
OclBuiltinImpl vstored_half_rtz_gas16 = OclBuiltinImpl<vstored_half_rtz_gas, [v16f16], 0, Stored_half_rtz16>;
OclBuiltinImpl vstored_half_rtz_las16 = OclBuiltinImpl<vstored_half_rtz_las, [v16f16], 0, Stored_half_rtz16>;
OclBuiltinImpl vstored_half_rtz_pas16 = OclBuiltinImpl<vstored_half_rtz_pas, [v16f16], 0, Stored_half_rtz16>;
OclBuiltinImpl vstored_half_rtz_gen16 = OclBuiltinImpl<vstored_half_rtz_gen, [v16f16], 0, Stored_half_rtz16>;
OclBuiltinImpl vstoref_half_gas16 = OclBuiltinImpl<vstoref_half_gas, [v16f16], 0, Storef_half16>;
OclBuiltinImpl vstoref_half_las16 = OclBuiltinImpl<vstoref_half_las, [v16f16], 0, Storef_half16>;
OclBuiltinImpl vstoref_half_pas16 = OclBuiltinImpl<vstoref_half_pas, [v16f16], 0, Storef_half16>;
OclBuiltinImpl vstoref_half_gen16 = OclBuiltinImpl<vstoref_half_gen, [v16f16], 0, Storef_half16>;
OclBuiltinImpl vstoref_half_rte_gas16 = OclBuiltinImpl<vstoref_half_rte_gas, [v16f16], 0, Storef_half_rte16>;
OclBuiltinImpl vstoref_half_rte_las16 = OclBuiltinImpl<vstoref_half_rte_las, [v16f16], 0, Storef_half_rte16>;
OclBuiltinImpl vstoref_half_rte_pas16 = OclBuiltinImpl<vstoref_half_rte_pas, [v16f16], 0, Storef_half_rte16>;
OclBuiltinImpl vstoref_half_rte_gen16 = OclBuiltinImpl<vstoref_half_rte_gen, [v16f16], 0, Storef_half_rte16>;
OclBuiltinImpl vstoref_half_rtn_gas16 = OclBuiltinImpl<vstoref_half_rtn_gas, [v16f16], 0, Storef_half_rtn16>;
OclBuiltinImpl vstoref_half_rtn_las16 = OclBuiltinImpl<vstoref_half_rtn_las, [v16f16], 0, Storef_half_rtn16>;
OclBuiltinImpl vstoref_half_rtn_pas16 = OclBuiltinImpl<vstoref_half_rtn_pas, [v16f16], 0, Storef_half_rtn16>;
OclBuiltinImpl vstoref_half_rtn_gen16 = OclBuiltinImpl<vstoref_half_rtn_gen, [v16f16], 0, Storef_half_rtn16>;
OclBuiltinImpl vstoref_half_rtp_gas16 = OclBuiltinImpl<vstoref_half_rtp_gas, [v16f16], 0, Storef_half_rtp16>;
OclBuiltinImpl vstoref_half_rtp_las16 = OclBuiltinImpl<vstoref_half_rtp_las, [v16f16], 0, Storef_half_rtp16>;
OclBuiltinImpl vstoref_half_rtp_pas16 = OclBuiltinImpl<vstoref_half_rtp_pas, [v16f16], 0, Storef_half_rtp16>;
OclBuiltinImpl vstoref_half_rtp_gen16 = OclBuiltinImpl<vstoref_half_rtp_gen, [v16f16], 0, Storef_half_rtp16>;
OclBuiltinImpl vstoref_half_rtz_gas16 = OclBuiltinImpl<vstoref_half_rtz_gas, [v16f16], 0, Storef_half_rtz16>;
OclBuiltinImpl vstoref_half_rtz_las16 = OclBuiltinImpl<vstoref_half_rtz_las, [v16f16], 0, Storef_half_rtz16>;
OclBuiltinImpl vstoref_half_rtz_pas16 = OclBuiltinImpl<vstoref_half_rtz_pas, [v16f16], 0, Storef_half_rtz16>;
OclBuiltinImpl vstoref_half_rtz_gen16 = OclBuiltinImpl<vstoref_half_rtz_gen, [v16f16], 0, Storef_half_rtz16>;
