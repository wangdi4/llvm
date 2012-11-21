// vim:ts=2:sw=2:et:

#pragma once

#ifndef GENERIC_BUILTIN_DEFINES_H
#define GENERIC_BUILTIN_DEFINES_H

#ifdef cplusplus
extern "C" {
#endif

// pi/180
const float generic_pi_180f = 0.017453292519943295769236907684883f;
const double generic_pi_180 = 0.017453292519943295769236907684883;

// 180/pi
const float generic_inv_pi_180f = 57.295779513082320876798154814105f;
const double generic_inv_pi_180 = 57.295779513082320876798154814105;

// common - math
const int    float_const_signMask      = 0x7FFFFFFF;
const long   double_const_signMask     = 0x7FFFFFFFFFFFFFFFL;
const int    float_const_nanStorage    = 0x7FFFFFFF;
const long   double_const_nanStorage   = 0x7FFFFFFFFFFFFFFFL;
const int    float_const_expMask       = 0x7f800000;
const long   double_const_expMask      = 0x7FF0000000000000L;
const float  float_const_fractLimit    = 0x1.fffffep-1f;
const double double_const_fractLimit   = 0x1.fffffffffffffp-1f;
const int    float_const_mantissaBits  = 23;
const int    double_const_mantissaBits = 52;
const int    float_const_expOffset     = 127;
const int    double_const_expOffset    = 1023;
const float  float_const_tooSmall      = 0x1.0p-63f;
const double double_const_tooSmall     = 0x1.0p-511;
const float  float_const_tooBig        = 0x1.0p+63f;
const double double_const_tooBig       = 0x1.0p+511;

// Geometric
const double exp600        =  0x1.0p600;
const double expMinus600   = 0x1.0p-600;
const double exp700        =  0x1.0p700;
const double expMinus700   = 0x1.0p-700;
const double expMinus512   = 0x1.0p-512;
const double expMinus512_2 = 0x1.0p-512 / 2;

// constants defined to simplify naming them from type name
const char    generic_min_char    =  CHAR_MIN;
const char    generic_max_char    =  CHAR_MAX;
const uchar   generic_min_uchar   =         0;
const uchar   generic_max_uchar   = UCHAR_MAX;
const short   generic_min_short   =  SHRT_MIN;
const short   generic_max_short   =  SHRT_MAX;
const ushort  generic_min_ushort  =         0;
const ushort  generic_max_ushort  = USHRT_MAX;
const int     generic_min_int     =   INT_MIN;
const int     generic_max_int     =   INT_MAX;
const uint    generic_min_uint    =         0;
const uint    generic_max_uint    =  UINT_MAX;
const long    generic_min_long    =  LONG_MIN;
const long    generic_max_long    =  LONG_MAX;
const ulong   generic_min_ulong   =         0;
const ulong   generic_max_ulong   = ULONG_MAX;

// sse - common
const float f4const_oneStorage[4]       = {1.0f, 1.0f, 1.0f, 1.0f};
const float f4const_minusOneStorage[4]  = {-1.0f, -1.0f, -1.0f, -1.0f};
const float f4const_minusZeroStorage[4] = {-0.0f, -0.0f, -0.0f, -0.0f};
const float f4const_nanStorage[4] = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};

const double d2const_oneStorage[2]       = {1.0, 1.0};
const double d2const_minusZeroStorage[2] = {-0.0, -0.0};
const double d2const_minusOneStorage[2]  = {-1.0, -1.0};
const double d2const_nanStorage[2] = {0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF};

// AVX - common
const float f8const_oneStorage[8]       = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
const float f8const_minusZeroStorage[8] = {-0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f};
const float f8const_minusOneStorage[8]  = {-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f};
const float f8const_nanStorage[8] = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};

const double d4const_oneStorage[4]       = {1.0, 1.0, 1.0, 1.0};
const double d4const_minusZeroStorage[4] = {-0.0, -0.0, -0.0, -0.0};
const double d4const_minusOneStorage[4]  = {-1.0, -1.0, -1.0, -1.0};
const double d4const_nanStorage[4] = {0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF};

#ifdef cplusplus
}
#endif

#endif /* GENERIC_BUILTIN_DEFINES_H */
