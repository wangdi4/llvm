// Copyright (C) 2012 Intel Corporation
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

#include "generic-builtin-defines.h"
// pi/180
const constant float generic_pi_180f = 0.017453292519943295769236907684883f;
const constant double generic_pi_180 = 0.017453292519943295769236907684883;

// 180/pi
const constant float generic_inv_pi_180f = 57.295779513082320876798154814105f;
const constant double generic_inv_pi_180 = 57.295779513082320876798154814105;

// common - math
const constant short half_const_signMask = 0x7FFF;
const constant int float_const_signMask = 0x7FFFFFFF;
const constant long double_const_signMask = 0x7FFFFFFFFFFFFFFFL;
const constant short half_const_expMask = 0x7C00;
const constant int float_const_expMask = 0x7f800000;
const constant long double_const_expMask = 0x7FF0000000000000L;
const constant float float_const_fractLimit = 0x1.fffffep-1f;
const constant double double_const_fractLimit = 0x1.fffffffffffffp-1f;
const constant int float_const_mantissaBits = 23;
const constant int double_const_mantissaBits = 52;
const constant int float_const_expOffset = 127;
const constant int double_const_expOffset = 1023;
const constant float float_const_tooSmall = 0x1.0p-63f;
const constant double double_const_tooSmall = 0x1.0p-511;
const constant float float_const_tooBig = 0x1.0p+63f;
const constant double double_const_tooBig = 0x1.0p+511;

// Geometric
const constant double exp600 = 0x1.0p600;
const constant double expMinus600 = 0x1.0p-600;
const constant double exp700 = 0x1.0p700;
const constant double expMinus700 = 0x1.0p-700;
const constant double expMinus512 = 0x1.0p-512;
const constant double expMinus512_2 = 0x1.0p-512 / 2;

// constants defined to simplify naming them from type name
const constant char generic_min_char = CHAR_MIN;
const constant char generic_max_char = CHAR_MAX;
const constant uchar generic_min_uchar = 0;
const constant uchar generic_max_uchar = UCHAR_MAX;
const constant short generic_min_short = SHRT_MIN;
const constant short generic_max_short = SHRT_MAX;
const constant ushort generic_min_ushort = 0;
const constant ushort generic_max_ushort = USHRT_MAX;
const constant int generic_min_int = INT_MIN;
const constant int generic_max_int = INT_MAX;
const constant uint generic_min_uint = 0;
const constant uint generic_max_uint = UINT_MAX;
const constant long generic_min_long = LONG_MIN;
const constant long generic_max_long = LONG_MAX;
const constant ulong generic_min_ulong = 0;
const constant ulong generic_max_ulong = ULONG_MAX;

// sse - common
const constant float4 f4const_oneStorage = {1.0f, 1.0f, 1.0f, 1.0f};
const constant float4 f4const_minusOneStorage = {-1.0f, -1.0f, -1.0f, -1.0f};
const constant float4 f4const_minusZeroStorage = {-0.0f, -0.0f, -0.0f, -0.0f};

const constant double2 d2const_oneStorage = {1.0, 1.0};
const constant double2 d2const_minusZeroStorage = {-0.0, -0.0};
const constant double2 d2const_minusOneStorage = {-1.0, -1.0};

// AVX - common
const constant float8 f8const_oneStorage = {1.0f, 1.0f, 1.0f, 1.0f,
                                            1.0f, 1.0f, 1.0f, 1.0f};
const constant float8 f8const_minusZeroStorage = {-0.0f, -0.0f, -0.0f, -0.0f,
                                                  -0.0f, -0.0f, -0.0f, -0.0f};
const constant float8 f8const_minusOneStorage = {-1.0f, -1.0f, -1.0f, -1.0f,
                                                 -1.0f, -1.0f, -1.0f, -1.0f};

const constant double4 d4const_oneStorage = {1.0, 1.0, 1.0, 1.0};
const constant double4 d4const_minusZeroStorage = {-0.0, -0.0, -0.0, -0.0};
const constant double4 d4const_minusOneStorage = {-1.0, -1.0, -1.0, -1.0};
