/*******************************************************************************
* INTEL_CUSTOMIZATION
* INTEL CONFIDENTIAL
* Modifications, Copyright (C) 1996 Intel Corporation.
*
* This software and the related documents are Intel copyrighted  materials,  and
* your use of  them is  governed by the  express license  under which  they were
* provided to you ("License"). Unless the License provides otherwise, you may not
* use, modify, copy, publish, distribute,  disclose or transmit this software or
* the related documents without Intel's prior written permission.
*
* This software and the related documents  are provided as  is,  with no express
* or implied  warranties,  other  than those  that are  expressly stated  in the
* License.
* END INTEL_CUSTOMIZATION
*******************************************************************************/

#ifndef __IMF_INCLUDE_FP32_HPP__
#define __IMF_INCLUDE_FP32_HPP__
#ifdef INTEL_CUSTOMIZATION
#include "../device_imf.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
typedef int16_t VINT16;
typedef int16_t VSINT16;
typedef uint16_t VUINT16;
typedef int32_t VINT32;
typedef int32_t VSINT32;
typedef uint32_t VUINT32;
typedef int64_t VINT64;
typedef int64_t VSINT64;
typedef uint64_t VUINT64;
typedef char _iml_int8_t;
typedef unsigned char _iml_uint8_t;
typedef short _iml_int16_t;
typedef unsigned short _iml_uint16_t;
typedef int _iml_int32_t;
typedef unsigned int _iml_uint32_t;
typedef long long _iml_int64_t;
typedef unsigned long long _iml_uint64_t;
typedef struct
{
    _iml_uint32_t significand:23;
    _iml_uint32_t exponent:8;
    _iml_uint32_t sign:1;
} _iml_spbits_t;
typedef union
{
    _iml_uint32_t hex[1];
    _iml_spbits_t bits;
    float fp;
} _iml_sp_union_t;
typedef _iml_spbits_t iml_fp32;
typedef union
{
    _iml_uint32_t w;
    float f;
} int_float;
static inline _iml_half as_half (uint16_t a)
{
    return __builtin_bit_cast (_iml_half, a);
}

static inline uint16_t as_short (_iml_half a)
{
    return __builtin_bit_cast (uint16_t, a);
}

inline float as_float (uint32_t a)
{
    return __builtin_bit_cast (float, a);
}

inline uint32_t as_uint (float a)
{
    return __builtin_bit_cast (uint32_t, a);
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
#endif /*INTEL_CUSTOMIZATION*/
#endif  /* __IMF_INCLUDE_FP32_HPP__ */
