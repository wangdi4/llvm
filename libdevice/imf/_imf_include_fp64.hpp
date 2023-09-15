/*******************************************************************************
* INTEL_CUSTOMIZATION
*
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

#ifndef __IMF_INCLUDE_FP64_HPP__
#define __IMF_INCLUDE_FP64_HPP__
#include "../device_imf.hpp"
#ifdef INTEL_CUSTOMIZATION
#ifdef __LIBDEVICE_IMF_ENABLED__
#include "_imf_include_fp32.hpp"
typedef struct
{
    _iml_uint32_t lo_significand:32;
    _iml_uint32_t hi_significand:20;
    _iml_uint32_t exponent:11;
    _iml_uint32_t sign:1;
} _iml_dpbits_t;
typedef struct
{
    _iml_uint32_t lo_dword;
    _iml_uint32_t hi_dword;
} _iml_dpdwords_t;
typedef union
{
    _iml_uint32_t hex[2];
    _iml_dpbits_t bits;
    _iml_dpdwords_t dwords;
    double fp;
} _iml_dp_union_t;
typedef _iml_dpbits_t iml_fp64;
typedef union
{
    _iml_uint64_t w;
    _iml_uint32_t w32[2];
    _iml_int32_t s32[2];
    double f;
} int_double;
inline double as_double (uint64_t a)
{
    return __builtin_bit_cast (double, a);
}

inline uint64_t as_ulong (double a)
{
    return __builtin_bit_cast (uint64_t, a);
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
#endif /*INTEL_CUSTOMIZATION*/
#endif  /* __IMF_INCLUDE_FP64_HPP__ */
