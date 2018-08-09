// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#ifndef GENERIC_BUILTIN_DEFINES_H
#define GENERIC_BUILTIN_DEFINES_H

// pi/180
extern const constant float generic_pi_180f;
extern const constant double generic_pi_180;

// 180/pi
extern const constant float generic_inv_pi_180f;
extern const constant double generic_inv_pi_180;

// common - math
extern const constant int    float_const_signMask;
extern const constant long   double_const_signMask;
extern const constant int    float_const_nanStorage;
extern const constant long   double_const_nanStorage;
extern const constant int    float_const_expMask;
extern const constant long   double_const_expMask;
extern const constant float  float_const_fractLimit;
extern const constant double double_const_fractLimit;
extern const constant int    float_const_mantissaBits;
extern const constant int    double_const_mantissaBits;
extern const constant int    float_const_expOffset;
extern const constant int    double_const_expOffset;
extern const constant float  float_const_tooSmall;
extern const constant double double_const_tooSmall;
extern const constant float  float_const_tooBig;
extern const constant double double_const_tooBig;

// Geometric
extern const constant double exp600;
extern const constant double expMinus600;
extern const constant double exp700;
extern const constant double expMinus700;
extern const constant double expMinus512;
extern const constant double expMinus512_2;

// constants defined to simplify naming them from type name
extern const constant char    generic_min_char;
extern const constant char    generic_max_char;
extern const constant uchar   generic_min_uchar;
extern const constant uchar   generic_max_uchar;
extern const constant short   generic_min_short;
extern const constant short   generic_max_short;
extern const constant ushort  generic_min_ushort;
extern const constant ushort  generic_max_ushort;
extern const constant int     generic_min_int;
extern const constant int     generic_max_int;
extern const constant uint    generic_min_uint;
extern const constant uint    generic_max_uint;
extern const constant long    generic_min_long;
extern const constant long    generic_max_long;
extern const constant ulong   generic_min_ulong;
extern const constant ulong   generic_max_ulong;

// sse - common
extern const constant float4 f4const_oneStorage;
extern const constant float4 f4const_minusOneStorage;
extern const constant float4 f4const_minusZeroStorage;
extern const constant float4 f4const_nanStorage;

extern const constant double2 d2const_oneStorage;
extern const constant double2 d2const_minusZeroStorage;
extern const constant double2 d2const_minusOneStorage;
extern const constant double2 d2const_nanStorage;

// AVX - common
extern const constant float8 f8const_oneStorage;
extern const constant float8 f8const_minusZeroStorage;
extern const constant float8 f8const_minusOneStorage;
extern const constant float8 f8const_nanStorage;

extern const constant double4 d4const_oneStorage;
extern const constant double4 d4const_minusZeroStorage;
extern const constant double4 d4const_minusOneStorage;
extern const constant double4 d4const_nanStorage;

typedef int ocl_int32 __attribute__((ext_vector_type(32)));
typedef uint ocl_uint32 __attribute__((ext_vector_type(32)));
typedef float ocl_float32 __attribute__((ext_vector_type(32)));


#ifndef INLINE_ATTRIBUTE
#define INLINE_ATTRIBUTE __attribute__((always_inline))
#endif

#ifndef INTERNAL_INLINE_ATTRIBUTE
#define INTERNAL_INLINE_ATTRIBUTE inline INLINE_ATTRIBUTE
#endif

//Functions copied from the conformance tests
//Functions copied from the conformance tests
void INTERNAL_INLINE_ATTRIBUTE multiply_unsigned_64_by_64( ulong sourceA, ulong sourceB, ulong *destLow, ulong *destHi )
{
    ulong lowA, lowB;
    ulong highA, highB;

    // Split up the values
    lowA = sourceA & 0xffffffff;
    highA = sourceA >> 32;
    lowB = sourceB & 0xffffffff;
    highB = sourceB >> 32;

    // Note that, with this split, our multiplication becomes:
    //     ( a * b )
    // = ( ( aHI << 32 + aLO ) * ( bHI << 32 + bLO ) ) >> 64
    // = ( ( aHI << 32 * bHI << 32 ) + ( aHI << 32 * bLO ) + ( aLO * bHI << 32 ) + ( aLO * bLO ) ) >> 64
    // = ( ( aHI * bHI << 64 ) + ( aHI * bLO << 32 ) + ( aLO * bHI << 32 ) + ( aLO * bLO ) ) >> 64
    // = ( aHI * bHI ) + ( aHI * bLO >> 32 ) + ( aLO * bHI >> 32 ) + ( aLO * bLO >> 64 )

    // Now, since each value is 32 bits, the max size of any multiplication is:
    // ( 2 ^ 32 - 1 ) * ( 2 ^ 32 - 1 ) = 2^64 - 4^32 + 1 = 2^64 - 2^33 + 1, which fits within 64 bits
    // Which means we can do each component within a 64-bit integer as necessary (each component above marked as AB1 - AB4)
    ulong aHibHi = highA * highB;
    ulong aHibLo = highA * lowB;
    ulong aLobHi = lowA * highB;
    ulong aLobLo = lowA * lowB;

    // Assemble terms.
    //  We note that in certain cases, sums of products cannot overflow:
    //
    //      The maximum product of two N-bit unsigned numbers is
    //
    //          (2**N-1)^2 = 2**2N - 2**(N+1) + 1
    //
    //      We note that we can add the maximum N-bit number to the 2N-bit product twice without overflow:
    //
    //          (2**N-1)^2 + 2*(2**N-1) = 2**2N - 2**(N+1) + 1 + 2**(N+1) - 2 = 2**2N - 1
    //
    //  If we breakdown the product of two numbers a,b into high and low halves of partial products as follows:
    //
    //                                              a.hi                a.lo
    // x                                            b.hi                b.lo
    //===============================================================================
    //  (b.hi*a.hi).hi      (b.hi*a.hi).lo
    //                      (b.lo*a.hi).hi      (b.lo*a.hi).lo
    //                      (b.hi*a.lo).hi      (b.hi*a.lo).lo
    // +                                        (b.lo*a.lo).hi      (b.lo*a.lo).lo
    //===============================================================================
    //
    // The (b.lo*a.lo).lo term cannot cause a carry, so we can ignore them for now.  We also know from above, that we can add (b.lo*a.lo).hi
    // and (b.hi*a.lo).lo to the 2N bit term [(b.lo*a.hi).hi + (b.lo*a.hi).lo] without overflow.  That takes care of all of the terms
    // on the right half that might carry.  Do that now.
    //
    ulong aLobLoHi = aLobLo >> 32;
    ulong aLobHiLo = aLobHi & 0xFFFFFFFFUL;
    aHibLo += aLobLoHi + aLobHiLo;

    // That leaves us with these terms:
    //
    //                                              a.hi                a.lo
    // x                                            b.hi                b.lo
    //===============================================================================
    //  (b.hi*a.hi).hi      (b.hi*a.hi).lo
    //                      (b.hi*a.lo).hi
    //                    [ (b.lo*a.hi).hi + (b.lo*a.hi).lo + other ]
    // +                                                                (b.lo*a.lo).lo
    //===============================================================================

    // All of the overflow potential from the right half has now been accumulated into the [ (b.lo*a.hi).hi + (b.lo*a.hi).lo ] 2N bit term.
    // We can safely separate into high and low parts. Per our rule above, we know we can accumulate the high part of that and (b.hi*a.lo).hi
    // into the 2N bit term (b.lo*a.hi) without carry.  The low part can be pieced together with (b.lo*a.lo).lo, to give the final low result

    (*destHi) = aHibHi + (aHibLo >> 32 ) + (aLobHi >> 32);             // Cant overflow
    (*destLow) = (aHibLo << 32) | ( aLobLo & 0xFFFFFFFFUL );
}

void INTERNAL_INLINE_ATTRIBUTE multiply_signed_64_by_64( long sourceA, long sourceB, ulong * destLow, ulong * destHi )
{
    // Find sign of result
    long aSign = sourceA >> 63;
    long bSign = sourceB >> 63;
    long resultSign = aSign ^ bSign;

    // take absolute values of the argument
    sourceA = (sourceA ^ aSign) - aSign;
    sourceB = (sourceB ^ bSign) - bSign;

    ulong hi;
    multiply_unsigned_64_by_64( (ulong) sourceA, (ulong) sourceB, destLow, &hi );

    // Fix the sign
    if( resultSign )
    {
        (*destLow) ^= resultSign;
        hi  ^= resultSign;
        (*destLow) -= resultSign;

        //carry if necessary
        if( 0 == (*destLow) )
            hi -= resultSign;
    }

    (*destHi) = (long) hi;
}

// shuffle & shuffle2
void* memcpy(void*, const void*, size_t);

//atomic
typedef int intrin_type;

#endif /* GENERIC_BUILTIN_DEFINES_H */
