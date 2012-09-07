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
const int float_const_signMask = 0x7FFFFFFF;
const long double_const_signMask = 0x7FFFFFFFFFFFFFFF;
const int float_const_nanStorage = 0x7FFFFFFF;
const long double_const_nanStorage = 0x7FFFFFFFFFFFFFFF;
const int float_const_expMask = 0x7f800000;
const long double_const_expMask = 0x7FF0000000000000;
const float float_const_fractLimit = 0x1.fffffep-1f;
const double double_const_fractLimit = 0x1.fffffffffffffp-1f;
const int float_const_mantissaBits = 23;
const int double_const_mantissaBits = 52;
const int float_const_expOffset = 127;
const int double_const_expOffset = 1023;


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

#ifdef cplusplus
}
#endif

#endif /* GENERIC_BUILTIN_DEFINES_H */
