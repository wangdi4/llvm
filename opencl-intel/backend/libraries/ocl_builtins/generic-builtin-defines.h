// vim:ts=2:sw=2:et:

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
extern const constant float f4const_oneStorage[4];
extern const constant float f4const_minusOneStorage[4];
extern const constant float f4const_minusZeroStorage[4];
extern const constant float f4const_nanStorage[4];

extern const constant double d2const_oneStorage[2];
extern const constant double d2const_minusZeroStorage[2];
extern const constant double d2const_minusOneStorage[2];
extern const constant double d2const_nanStorage[2];

// AVX - common
extern const constant float f8const_oneStorage[8];
extern const constant float f8const_minusZeroStorage[8];
extern const constant float f8const_minusOneStorage[8];
extern const constant float f8const_nanStorage[8];

extern const constant double d4const_oneStorage[4];
extern const constant double d4const_minusZeroStorage[4];
extern const constant double d4const_minusOneStorage[4];
extern const constant double d4const_nanStorage[4];

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
void INTERNAL_INLINE_ATTRIBUTE multiply_unsigned_64_by_64( ulong sourceA, ulong sourceB, ulong *destLow, ulong *destHi );
void INTERNAL_INLINE_ATTRIBUTE multiply_signed_64_by_64( long sourceA, long sourceB, ulong * destLow, ulong * destHi );

// shuffle & shuffle2
void* memcpy(void*, const void*, size_t);

//atomic
typedef int intrin_type;

#endif /* GENERIC_BUILTIN_DEFINES_H */
