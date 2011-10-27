/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/


#include "functions.h"

#define VECTORIZER_HASH_SIZE 64
#define SCALARIZER_HASH_SIZE 256

// List of hash entries...
VFH::hashEntry __f__node[217] = {
/*[0]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__acosf","__acosf2","__acosf4","__acosf8","__acosf16","__acosf3"},
/*[1]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__acoshf","__acoshf2","__acoshf4","__acoshf8","__acoshf16","__acoshf3"},
/*[2]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__acospif","__acospif2","__acospif4","__acospif8","__acospif16","__acospif3"},
/*[3]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__asinf","__asinf2","__asinf4","__asinf8","__asinf16","__asinf3"},
/*[4]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__asinhf","__asinhf2","__asinhf4","__asinhf8","__asinhf16","__asinhf3"},
/*[5]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__asinpif","__asinpif2","__asinpif4","__asinpif8","__asinpif16","__asinpif3"},
/*[6]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__atanf","__atanf2","__atanf4","__atanf8","__atanf16","__atanf3"},
/*[7]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__atan2f","__atan2f2","__atan2f4","__atan2f8","__atan2f16","__atan2f3"},
/*[8]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__atan2pif","__atan2pif2","__atan2pif4","__atan2pif8","__atan2pif16","__atan2pif3"},
/*[9]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__atanhf","__atanhf2","__atanhf4","__atanhf8","__atanhf16","__atanhf3"},
/*[10]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__atanpif","__atanpif2","__atanpif4","__atanpif8","__atanpif16","__atanpif3"},
/*[11]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__cbrtf","__cbrtf2","__cbrtf4","__cbrtf8","__cbrtf16","__cbrtf3"},
/*[12]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__ceilf","__ceilf2","__ceilf4","__ceilf8","__ceilf16","__ceilf3"},
/*[13]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__copysignf","__copysignf2","__copysignf4","__copysignf8","__copysignf16","__copysignf3"},
/*[14]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__cosf","__cosf2","__cosf4","__cosf8","__cosf16","__cosf3"},
/*[15]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__coshf","__coshf2","__coshf4","__coshf8","__coshf16","__coshf3"},
/*[16]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__cospif","__cospif2","__cospif4","__cospif8","__cospif16","__cospif3"},
/*[17]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__erfcf","__erfcf2","__erfcf4","__erfcf8","__erfcf16","__erfcf3"},
/*[18]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__erff","__erff2","__erff4","__erff8","__erff16","__erff3"},
/*[19]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__expf","__expf2","__expf4","__expf8","__expf16","__expf3"},
/*[20]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__exp2f","__exp2f2","__exp2f4","__exp2f8","__exp2f16","__exp2f3"},
/*[21]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__exp10f","__exp10f2","__exp10f4","__exp10f8","__exp10f16","__exp10f3"},
/*[22]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__expm1f","__expm1f2","__expm1f4","__expm1f8","__expm1f16","__expm1f3"},
/*[23]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__fabsf","__fabsf2","__fabsf4","__fabsf8","__fabsf16","__fabsf3"},
/*[24]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__fdimf","__fdimf2","__fdimf4","__fdimf8","__fdimf16","__fdimf3"},
/*[25]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__floorf","__floorf2","__floorf4","__floorf8","__floorf16","__floorf3"},
/*[26]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,"__fmaf","__fmaf2","__fmaf4","__fmaf8","__fmaf16","__fmaf3"},
/*[27]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__fmaxf","__fmaxf2","__fmaxf4","__fmaxf8","__fmaxf16","__fmaxf3"},
/*[28]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_STATIC,VFH::T_NONE,"__fmaxf","__fmaxff2","__fmaxff4","__fmaxff8","__fmaxff16","__fmaxff3"},
/*[29]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__fminf","__fminf2","__fminf4","__fminf8","__fminf16","__fminf3"},
/*[30]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_STATIC,VFH::T_NONE,"__fminf","__fminff2","__fminff4","__fminff8","__fminff16","__fminff3"},
/*[31]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__fmodf","__fmodf2","__fmodf4","__fmodf8","__fmodf16","__fmodf3"},
/*[32]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__hypotf","__hypotf2","__hypotf4","__hypotf8","__hypotf16","__hypotf3"},
/*[33]*/ {VFH::T_I32,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__ilogbf","__ilogbf2","__ilogbf4","__ilogbf8","__ilogbf16","__ilogbf3"},
/*[34]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_I32,VFH::T_NONE,"__ldexpf","__ldexpf2","__ldexpf4","__ldexpf8","__ldexpf16","__ldexpf3"},
/*[35]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__lgammaf","__lgammaf2","__lgammaf4","__lgammaf8","__lgammaf16","__lgammaf3"},
/*[36]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__logf","__logf2","__logf4","__logf8","__logf16","__logf3"},
/*[37]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__log2f","__log2f2","__log2f4","__log2f8","__log2f16","__log2f3"},
/*[38]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__log10f","__log10f2","__log10f4","__log10f8","__log10f16","__log10f3"},
/*[39]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__log1pf","__log1pf2","__log1pf4","__log1pf8","__log1pf16","__log1pf3"},
/*[40]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__logbf","__logbf2","__logbf4","__logbf8","__logbf16","__logbf3"},
/*[41]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,"__madf","__madf2","__madf4","__madf8","__madf16","__madf3"},
/*[42]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__nextafterf","__nextafterf2","__nextafterf4","__nextafterf8","__nextafterf16","__nextafterf3"},
/*[43]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__powf","__powf2","__powf4","__powf8","__powf16","__powf3"},
/*[44]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_I32,VFH::T_NONE,"__pownf","__pownf2","__pownf4","__pownf8","__pownf16","__pownf3"},
/*[45]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__powrf","__powrf2","__powrf4","__powrf8","__powrf16","__powrf3"},
/*[46]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__remainderf","__remainderf2","__remainderf4","__remainderf8","__remainderf16","__remainderf3"},
/*[47]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__rintf","__rintf2","__rintf4","__rintf8","__rintf16","__rintf3"},
/*[48]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_I32,VFH::T_NONE,"__rootnf","__rootnf2","__rootnf4","__rootnf8","__rootnf16","__rootnf3"},
/*[49]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__roundf","__roundf2","__roundf4","__roundf8","__roundf16","__roundf3"},
/*[50]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__rsqrtf","__rsqrtf2","__rsqrtf4","__rsqrtf8","__rsqrtf16","__rsqrtf3"},
/*[51]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__sinf","__sinf2","__sinf4","__sinf8","__sinf16","__sinf3"},
/*[52]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__sinhf","__sinhf2","__sinhf4","__sinhf8","__sinhf16","__sinhf3"},
/*[53]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__sinpif","__sinpif2","__sinpif4","__sinpif8","__sinpif16","__sinpif3"},
/*[54]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__sqrtf","__sqrtf2","__sqrtf4","__sqrtf8","__sqrtf16","__sqrtf3"},
/*[55]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__tanf","__tanf2","__tanf4","__tanf8","__tanf16","__tanf3"},
/*[56]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__tanhf","__tanhf2","__tanhf4","__tanhf8","__tanhf16","__tanhf3"},
/*[57]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__tanpif","__tanpif2","__tanpif4","__tanpif8","__tanpif16","__tanpif3"},
/*[58]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__tgammaf","__tgammaf2","__tgammaf4","__tgammaf8","__tgammaf16","__tgammaf3"},
/*[59]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__truncf","__truncf2","__truncf4","__truncf8","__truncf16","__truncf3"},
/*[60]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__half_cosf","__half_cosf2","__half_cosf4","__half_cosf8","__half_cosf16","__half_cosf3"},
/*[61]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__half_dividef","__half_dividef2","__half_dividef4","__half_dividef8","__half_dividef16","__half_dividef3"},
/*[62]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__half_expf","__half_expf2","__half_expf4","__half_expf8","__half_expf16","__half_expf3"},
/*[63]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__half_exp2f","__half_exp2f2","__half_exp2f4","__half_exp2f8","__half_exp2f16","__half_exp2f3"},
/*[64]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__half_exp10f","__half_exp10f2","__half_exp10f4","__half_exp10f8","__half_exp10f16","__half_exp10f3"},
/*[65]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__half_logf","__half_logf2","__half_logf4","__half_logf8","__half_logf16","__half_logf3"},
/*[66]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__half_log2f","__half_log2f2","__half_log2f4","__half_log2f8","__half_log2f16","__half_log2f3"},
/*[67]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__half_log10f","__half_log10f2","__half_log10f4","__half_log10f8","__half_log10f16","__half_log10f3"},
/*[68]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__half_powrf","__half_powrf2","__half_powrf4","__half_powrf8","__half_powrf16","__half_powrf3"},
/*[69]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__half_recipf","__half_recipf2","__half_recipf4","__half_recipf8","__half_recipf16","__half_recipf3"},
/*[70]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__half_rsqrtf","__half_rsqrtf2","__half_rsqrtf4","__half_rsqrtf8","__half_rsqrtf16","__half_rsqrtf3"},
/*[71]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__half_sinf","__half_sinf2","__half_sinf4","__half_sinf8","__half_sinf16","__half_sinf3"},
/*[72]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__half_sqrtf","__half_sqrtf2","__half_sqrtf4","__half_sqrtf8","__half_sqrtf16","__half_sqrtf3"},
/*[73]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__half_tanf","__half_tanf2","__half_tanf4","__half_tanf8","__half_tanf16","__half_tanf3"},
/*[74]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__native_cosf","__native_cosf2","__native_cosf4","__native_cosf8","__native_cosf16","__native_cosf3"},
/*[75]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__native_dividef","__native_dividef2","__native_dividef4","__native_dividef8","__native_dividef16","__native_dividef3"},
/*[76]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__native_expf","__native_expf2","__native_expf4","__native_expf8","__native_expf16","__native_expf3"},
/*[77]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__native_exp2f","__native_exp2f2","__native_exp2f4","__native_exp2f8","__native_exp2f16","__native_exp2f3"},
/*[78]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__native_exp10f","__native_exp10f2","__native_exp10f4","__native_exp10f8","__native_exp10f16","__native_exp10f3"},
/*[79]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__native_logf","__native_logf2","__native_logf4","__native_logf8","__native_logf16","__native_logf3"},
/*[80]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__native_log2f","__native_log2f2","__native_log2f4","__native_log2f8","__native_log2f16","__native_log2f3"},
/*[81]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__native_log10f","__native_log10f2","__native_log10f4","__native_log10f8","__native_log10f16","__native_log10f3"},
/*[82]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__native_powrf","__native_powrf2","__native_powrf4","__native_powrf8","__native_powrf16","__native_powrf3"},
/*[83]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__native_recipf","__native_recipf2","__native_recipf4","__native_recipf8","__native_recipf16","__native_recipf3"},
/*[84]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__native_rsqrtf","__native_rsqrtf2","__native_rsqrtf4","__native_rsqrtf8","__native_rsqrtf16","__native_rsqrtf3"},
/*[85]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__native_sinf","__native_sinf2","__native_sinf4","__native_sinf8","__native_sinf16","__native_sinf3"},
/*[86]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__native_sqrtf","__native_sqrtf2","__native_sqrtf4","__native_sqrtf8","__native_sqrtf16","__native_sqrtf3"},
/*[87]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__native_tanf","__native_tanf2","__native_tanf4","__native_tanf8","__native_tanf16","__native_tanf3"},
/*[88]*/ {VFH::T_I8,VFH::T_I8,VFH::T_NONE,VFH::T_NONE,"__abs_1i8","__abs_2i8","__abs_4i8","__abs_8i8","__abs_16i8","__abs_3i8"},
/*[89]*/ {VFH::T_I8,VFH::T_I8,VFH::T_NONE,VFH::T_NONE,"__abs_1u8","__abs_2u8","__abs_4u8","__abs_8u8","__abs_16u8","__abs_3u8"},
/*[90]*/ {VFH::T_I16,VFH::T_I16,VFH::T_NONE,VFH::T_NONE,"__abs_1i16","__abs_2i16","__abs_4i16","__abs_8i16","__abs_16i16","__abs_3i16"},
/*[91]*/ {VFH::T_I16,VFH::T_I16,VFH::T_NONE,VFH::T_NONE,"__abs_1u16","__abs_2u16","__abs_4u16","__abs_8u16","__abs_16u16","__abs_3u16"},
/*[92]*/ {VFH::T_I32,VFH::T_I32,VFH::T_NONE,VFH::T_NONE,"__abs_1i32","__abs_2i32","__abs_4i32","__abs_8i32","__abs_16i32","__abs_3i32"},
/*[93]*/ {VFH::T_I32,VFH::T_I32,VFH::T_NONE,VFH::T_NONE,"__abs_1u32","__abs_2u32","__abs_4u32","__abs_8u32","__abs_16u32","__abs_3u32"},
/*[94]*/ {VFH::T_I64,VFH::T_I64,VFH::T_NONE,VFH::T_NONE,"__abs_1i64","__abs_2i64","__abs_4i64","__abs_8i64","__abs_16i64","__abs_3i64"},
/*[95]*/ {VFH::T_I64,VFH::T_I64,VFH::T_NONE,VFH::T_NONE,"__abs_1u64","__abs_2u64","__abs_4u64","__abs_8u64","__abs_16u64","__abs_3u64"},
/*[96]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__abs_diff_1i8","__abs_diff_2i8","__abs_diff_4i8","__abs_diff_8i8","__abs_diff_16i8","__abs_diff_3i8"},
/*[97]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__abs_diff_1u8","__abs_diff_2u8","__abs_diff_4u8","__abs_diff_8u8","__abs_diff_16u8","__abs_diff_3u8"},
/*[98]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__abs_diff_1i16","__abs_diff_2i16","__abs_diff_4i16","__abs_diff_8i16","__abs_diff_16i16","__abs_diff_3i16"},
/*[99]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__abs_diff_1u16","__abs_diff_2u16","__abs_diff_4u16","__abs_diff_8u16","__abs_diff_16u16","__abs_diff_3u16"},
/*[100]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__abs_diff_1i32","__abs_diff_2i32","__abs_diff_4i32","__abs_diff_8i32","__abs_diff_16i32","__abs_diff_3i32"},
/*[101]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__abs_diff_1u32","__abs_diff_2u32","__abs_diff_4u32","__abs_diff_8u32","__abs_diff_16u32","__abs_diff_3u32"},
/*[102]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__abs_diff_1i64","__abs_diff_2i64","__abs_diff_4i64","__abs_diff_8i64","__abs_diff_16i64","__abs_diff_3i64"},
/*[103]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__abs_diff_1u64","__abs_diff_2u64","__abs_diff_4u64","__abs_diff_8u64","__abs_diff_16u64","__abs_diff_3u64"},
/*[104]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__add_sat_1i8","__add_sat_2i8","__add_sat_4i8","__add_sat_8i8","__add_sat_16i8","__add_sat_3i8"},
/*[105]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__add_sat_1u8","__add_sat_2u8","__add_sat_4u8","__add_sat_8u8","__add_sat_16u8","__add_sat_3u8"},
/*[106]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__add_sat_1i16","__add_sat_2i16","__add_sat_4i16","__add_sat_8i16","__add_sat_16i16","__add_sat_3i16"},
/*[107]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__add_sat_1u16","__add_sat_2u16","__add_sat_4u16","__add_sat_8u16","__add_sat_16u16","__add_sat_3u16"},
/*[108]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__add_sat_1i32","__add_sat_2i32","__add_sat_4i32","__add_sat_8i32","__add_sat_16i32","__add_sat_3i32"},
/*[109]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__add_sat_1u32","__add_sat_2u32","__add_sat_4u32","__add_sat_8u32","__add_sat_16u32","__add_sat_3u32"},
/*[110]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__add_sat_1i64","__add_sat_2i64","__add_sat_4i64","__add_sat_8i64","__add_sat_16i64","__add_sat_3i64"},
/*[111]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__add_sat_1u64","__add_sat_2u64","__add_sat_4u64","__add_sat_8u64","__add_sat_16u64","__add_sat_3u64"},
/*[112]*/ {VFH::T_I8,VFH::T_I8,VFH::T_NONE,VFH::T_NONE,"__clz_1i8","__clz_2i8","__clz_4i8","__clz_8i8","__clz_16i8","__clz_3i8"},
/*[113]*/ {VFH::T_I8,VFH::T_I8,VFH::T_NONE,VFH::T_NONE,"__clz_1u8","__clz_2u8","__clz_4u8","__clz_8u8","__clz_16u8","__clz_3u8"},
/*[114]*/ {VFH::T_I16,VFH::T_I16,VFH::T_NONE,VFH::T_NONE,"__clz_1i16","__clz_2i16","__clz_4i16","__clz_8i16","__clz_16i16","__clz_3i16"},
/*[115]*/ {VFH::T_I16,VFH::T_I16,VFH::T_NONE,VFH::T_NONE,"__clz_1u16","__clz_2u16","__clz_4u16","__clz_8u16","__clz_16u16","__clz_3u16"},
/*[116]*/ {VFH::T_I32,VFH::T_I32,VFH::T_NONE,VFH::T_NONE,"__clz_1i32","__clz_2i32","__clz_4i32","__clz_8i32","__clz_16i32","__clz_3i32"},
/*[117]*/ {VFH::T_I32,VFH::T_I32,VFH::T_NONE,VFH::T_NONE,"__clz_1u32","__clz_2u32","__clz_4u32","__clz_8u32","__clz_16u32","__clz_3u32"},
/*[118]*/ {VFH::T_I64,VFH::T_I64,VFH::T_NONE,VFH::T_NONE,"__clz_1i64","__clz_2i64","__clz_4i64","__clz_8i64","__clz_16i64","__clz_3i64"},
/*[119]*/ {VFH::T_I64,VFH::T_I64,VFH::T_NONE,VFH::T_NONE,"__clz_1u64","__clz_2u64","__clz_4u64","__clz_8u64","__clz_16u64","__clz_3u64"},
/*[120]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__hadd_1i8","__hadd_2i8","__hadd_4i8","__hadd_8i8","__hadd_16i8","__hadd_3i8"},
/*[121]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__hadd_1u8","__hadd_2u8","__hadd_4u8","__hadd_8u8","__hadd_16u8","__hadd_3u8"},
/*[122]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__hadd_1i16","__hadd_2i16","__hadd_4i16","__hadd_8i16","__hadd_16i16","__hadd_3i16"},
/*[123]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__hadd_1u16","__hadd_2u16","__hadd_4u16","__hadd_8u16","__hadd_16u16","__hadd_3u16"},
/*[124]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__hadd_1i32","__hadd_2i32","__hadd_4i32","__hadd_8i32","__hadd_16i32","__hadd_3i32"},
/*[125]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__hadd_1u32","__hadd_2u32","__hadd_4u32","__hadd_8u32","__hadd_16u32","__hadd_3u32"},
/*[126]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__hadd_1i64","__hadd_2i64","__hadd_4i64","__hadd_8i64","__hadd_16i64","__hadd_3i64"},
/*[127]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__hadd_1u64","__hadd_2u64","__hadd_4u64","__hadd_8u64","__hadd_16u64","__hadd_3u64"},
/*[128]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__rhadd_1i8","__rhadd_2i8","__rhadd_4i8","__rhadd_8i8","__rhadd_16i8","__rhadd_3i8"},
/*[129]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__rhadd_1u8","__rhadd_2u8","__rhadd_4u8","__rhadd_8u8","__rhadd_16u8","__rhadd_3u8"},
/*[130]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__rhadd_1i16","__rhadd_2i16","__rhadd_4i16","__rhadd_8i16","__rhadd_16i16","__rhadd_3i16"},
/*[131]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__rhadd_1u16","__rhadd_2u16","__rhadd_4u16","__rhadd_8u16","__rhadd_16u16","__rhadd_3u16"},
/*[132]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__rhadd_1i32","__rhadd_2i32","__rhadd_4i32","__rhadd_8i32","__rhadd_16i32","__rhadd_3i32"},
/*[133]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__rhadd_1u32","__rhadd_2u32","__rhadd_4u32","__rhadd_8u32","__rhadd_16u32","__rhadd_3u32"},
/*[134]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__rhadd_1i64","__rhadd_2i64","__rhadd_4i64","__rhadd_8i64","__rhadd_16i64","__rhadd_3i64"},
/*[135]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__rhadd_1u64","__rhadd_2u64","__rhadd_4u64","__rhadd_8u64","__rhadd_16u64","__rhadd_3u64"},
/*[136]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_I8,"__mad_hi_1i8","__mad_hi_2i8","__mad_hi_4i8","__mad_hi_8i8","__mad_hi_16i8","__mad_hi_3i8"},
/*[137]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_I8,"__mad_hi_1u8","__mad_hi_2u8","__mad_hi_4u8","__mad_hi_8u8","__mad_hi_16u8","__mad_hi_3u8"},
/*[138]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_I16,"__mad_hi_1i16","__mad_hi_2i16","__mad_hi_4i16","__mad_hi_8i16","__mad_hi_16i16","__mad_hi_3i16"},
/*[139]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_I16,"__mad_hi_1u16","__mad_hi_2u16","__mad_hi_4u16","__mad_hi_8u16","__mad_hi_16u16","__mad_hi_3u16"},
/*[140]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_I32,"__mad_hi_1i32","__mad_hi_2i32","__mad_hi_4i32","__mad_hi_8i32","__mad_hi_16i32","__mad_hi_3i32"},
/*[141]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_I32,"__mad_hi_1u32","__mad_hi_2u32","__mad_hi_4u32","__mad_hi_8u32","__mad_hi_16u32","__mad_hi_3u32"},
/*[142]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_I64,"__mad_hi_1i64","__mad_hi_2i64","__mad_hi_4i64","__mad_hi_8i64","__mad_hi_16i64","__mad_hi_3i64"},
/*[143]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_I64,"__mad_hi_1u64","__mad_hi_2u64","__mad_hi_4u64","__mad_hi_8u64","__mad_hi_16u64","__mad_hi_3u64"},
/*[144]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_I8,"__mad_sat_1i8","__mad_sat_2i8","__mad_sat_4i8","__mad_sat_8i8","__mad_sat_16i8","__mad_sat_3i8"},
/*[145]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_I8,"__mad_sat_1u8","__mad_sat_2u8","__mad_sat_4u8","__mad_sat_8u8","__mad_sat_16u8","__mad_sat_3u8"},
/*[146]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_I16,"__mad_sat_1i16","__mad_sat_2i16","__mad_sat_4i16","__mad_sat_8i16","__mad_sat_16i16","__mad_sat_3i16"},
/*[147]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_I16,"__mad_sat_1u16","__mad_sat_2u16","__mad_sat_4u16","__mad_sat_8u16","__mad_sat_16u16","__mad_sat_3u16"},
/*[148]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_I32,"__mad_sat_1i32","__mad_sat_2i32","__mad_sat_4i32","__mad_sat_8i32","__mad_sat_16i32","__mad_sat_3i32"},
/*[149]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_I32,"__mad_sat_1u32","__mad_sat_2u32","__mad_sat_4u32","__mad_sat_8u32","__mad_sat_16u32","__mad_sat_3u32"},
/*[150]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_I64,"__mad_sat_1i64","__mad_sat_2i64","__mad_sat_4i64","__mad_sat_8i64","__mad_sat_16i64","__mad_sat_3i64"},
/*[151]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_I64,"__mad_sat_1u64","__mad_sat_2u64","__mad_sat_4u64","__mad_sat_8u64","__mad_sat_16u64","__mad_sat_3u64"},
/*[152]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__mul_hi_1i8","__mul_hi_2i8","__mul_hi_4i8","__mul_hi_8i8","__mul_hi_16i8","__mul_hi_3i8"},
/*[153]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__mul_hi_1u8","__mul_hi_2u8","__mul_hi_4u8","__mul_hi_8u8","__mul_hi_16u8","__mul_hi_3u8"},
/*[154]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__mul_hi_1i16","__mul_hi_2i16","__mul_hi_4i16","__mul_hi_8i16","__mul_hi_16i16","__mul_hi_3i16"},
/*[155]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__mul_hi_1u16","__mul_hi_2u16","__mul_hi_4u16","__mul_hi_8u16","__mul_hi_16u16","__mul_hi_3u16"},
/*[156]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__mul_hi_1i32","__mul_hi_2i32","__mul_hi_4i32","__mul_hi_8i32","__mul_hi_16i32","__mul_hi_3i32"},
/*[157]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__mul_hi_1u32","__mul_hi_2u32","__mul_hi_4u32","__mul_hi_8u32","__mul_hi_16u32","__mul_hi_3u32"},
/*[158]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__mul_hi_1i64","__mul_hi_2i64","__mul_hi_4i64","__mul_hi_8i64","__mul_hi_16i64","__mul_hi_3i64"},
/*[159]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__mul_hi_1u64","__mul_hi_2u64","__mul_hi_4u64","__mul_hi_8u64","__mul_hi_16u64","__mul_hi_3u64"},
/*[160]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__rotate_1i8","__rotate_2i8","__rotate_4i8","__rotate_8i8","__rotate_16i8","__rotate_3i8"},
/*[161]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__rotate_1u8","__rotate_2u8","__rotate_4u8","__rotate_8u8","__rotate_16u8","__rotate_3u8"},
/*[162]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__rotate_1i16","__rotate_2i16","__rotate_4i16","__rotate_8i16","__rotate_16i16","__rotate_3i16"},
/*[163]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__rotate_1u16","__rotate_2u16","__rotate_4u16","__rotate_8u16","__rotate_16u16","__rotate_3u16"},
/*[164]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__rotate_1i32","__rotate_2i32","__rotate_4i32","__rotate_8i32","__rotate_16i32","__rotate_3i32"},
/*[165]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__rotate_1u32","__rotate_2u32","__rotate_4u32","__rotate_8u32","__rotate_16u32","__rotate_3u32"},
/*[166]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__rotate_1i64","__rotate_2i64","__rotate_4i64","__rotate_8i64","__rotate_16i64","__rotate_3i64"},
/*[167]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__rotate_1u64","__rotate_2u64","__rotate_4u64","__rotate_8u64","__rotate_16u64","__rotate_3u64"},
/*[168]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__sub_sat_1i8","__sub_sat_2i8","__sub_sat_4i8","__sub_sat_8i8","__sub_sat_16i8","__sub_sat_3i8"},
/*[169]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__sub_sat_1u8","__sub_sat_2u8","__sub_sat_4u8","__sub_sat_8u8","__sub_sat_16u8","__sub_sat_3u8"},
/*[170]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__sub_sat_1i16","__sub_sat_2i16","__sub_sat_4i16","__sub_sat_8i16","__sub_sat_16i16","__sub_sat_3i16"},
/*[171]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__sub_sat_1u16","__sub_sat_2u16","__sub_sat_4u16","__sub_sat_8u16","__sub_sat_16u16","__sub_sat_3u16"},
/*[172]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__sub_sat_1i32","__sub_sat_2i32","__sub_sat_4i32","__sub_sat_8i32","__sub_sat_16i32","__sub_sat_3i32"},
/*[173]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__sub_sat_1u32","__sub_sat_2u32","__sub_sat_4u32","__sub_sat_8u32","__sub_sat_16u32","__sub_sat_3u32"},
/*[174]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__sub_sat_1i64","__sub_sat_2i64","__sub_sat_4i64","__sub_sat_8i64","__sub_sat_16i64","__sub_sat_3i64"},
/*[175]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__sub_sat_1u64","__sub_sat_2u64","__sub_sat_4u64","__sub_sat_8u64","__sub_sat_16u64","__sub_sat_3u64"},
/*[176]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_I32,"__mad24_1i32","__mad24_2i32","__mad24_4i32","__mad24_8i32","__mad24_16i32","__mad24_3i32"},
/*[177]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_I32,"__mad24_1u32","__mad24_2u32","__mad24_4u32","__mad24_8u32","__mad24_16u32","__mad24_3u32"},
/*[178]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__mul24_1i32","__mul24_2i32","__mul24_4i32","__mul24_8i32","__mul24_16i32","__mul24_3i32"},
/*[179]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__mul24_1u32","__mul24_2u32","__mul24_4u32","__mul24_8u32","__mul24_16u32","__mul24_3u32"},
/*[180]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,"__clampf","__clampf2","__clampf4","__clampf8","__clampf16","__clampf3"},
/*[181]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_STATIC,VFH::T_STATIC,"__clampf","__clampff2","__clampff4","__clampff8","__clampff16","__clampff3"},
/*[182]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__degreesf","__degreesf2","__degreesf4","__degreesf8","__degreesf16","__degreesf3"},
/*[183]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__max_1i8","__max_2i8","__max_4i8","__max_8i8","__max_16i8","__max_3i8"},
/*[184]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__max_1u8","__max_2u8","__max_4u8","__max_8u8","__max_16u8","__max_3u8"},
/*[185]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__max_1i16","__max_2i16","__max_4i16","__max_8i16","__max_16i16","__max_3i16"},
/*[186]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__max_1u16","__max_2u16","__max_4u16","__max_8u16","__max_16u16","__max_3u16"},
/*[187]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__max_1i32","__max_2i32","__max_4i32","__max_8i32","__max_16i32","__max_3i32"},
/*[188]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__max_1u32","__max_2u32","__max_4u32","__max_8u32","__max_16u32","__max_3u32"},
/*[189]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__max_1i64","__max_2i64","__max_4i64","__max_8i64","__max_16i64","__max_3i64"},
/*[190]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__max_1u64","__max_2u64","__max_4u64","__max_8u64","__max_16u64","__max_3u64"},
/*[191]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__maxf","__maxf2","__maxf4","__maxf8","__maxf16","__maxf3"},
/*[192]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_STATIC,VFH::T_NONE,"__maxf","__maxff2","__maxff4","__maxff8","__maxff16","__maxff3"},
/*[193]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__min_1i8","__min_2i8","__min_4i8","__min_8i8","__min_16i8","__min_3i8"},
/*[194]*/ {VFH::T_I8,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__min_1u8","__min_2u8","__min_4u8","__min_8u8","__min_16u8","__min_3u8"},
/*[195]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__min_1i16","__min_2i16","__min_4i16","__min_8i16","__min_16i16","__min_3i16"},
/*[196]*/ {VFH::T_I16,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__min_1u16","__min_2u16","__min_4u16","__min_8u16","__min_16u16","__min_3u16"},
/*[197]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__min_1i32","__min_2i32","__min_4i32","__min_8i32","__min_16i32","__min_3i32"},
/*[198]*/ {VFH::T_I32,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__min_1u32","__min_2u32","__min_4u32","__min_8u32","__min_16u32","__min_3u32"},
/*[199]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__min_1i64","__min_2i64","__min_4i64","__min_8i64","__min_16i64","__min_3i64"},
/*[200]*/ {VFH::T_I64,VFH::T_I64,VFH::T_I64,VFH::T_NONE,"__min_1u64","__min_2u64","__min_4u64","__min_8u64","__min_16u64","__min_3u64"},
/*[201]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__minf","__minf2","__minf4","__minf8","__minf16","__minf3"},
/*[202]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_STATIC,VFH::T_NONE,"__minf","__minff2","__minff4","__minff8","__minff16","__minff3"},
/*[203]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,"__mixf","__mixf2","__mixf4","__mixf8","__mixf16","__mixf3"},
/*[204]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_STATIC,"__mixf","__mixff2","__mixff4","__mixff8","__mixff16","__mixff3"},
/*[205]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__radiansf","__radiansf2","__radiansf4","__radiansf8","__radiansf16","__radiansf3"},
/*[206]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,VFH::T_NONE,"__signf","__signf2","__signf4","__signf8","__signf16","__signf3"},
/*[207]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,"__smoothstepf","__smoothstepf2","__smoothstepf4","__smoothstepf8","__smoothstepf16","__smoothstepf3"},
/*[208]*/ {VFH::T_FLOAT,VFH::T_STATIC,VFH::T_STATIC,VFH::T_FLOAT,"__smoothstepf","__smoothstepff2","__smoothstepff4","__smoothstepff8","__smoothstepff16","__smoothstepff3"},
/*[209]*/ {VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_FLOAT,VFH::T_NONE,"__stepf","__stepf2","__stepf4","__stepf8","__stepf16","__stepf3"},
/*[210]*/ {VFH::T_FLOAT,VFH::T_STATIC,VFH::T_FLOAT,VFH::T_NONE,"__stepf","__stepff2","__stepff4","__stepff8","__stepff16","__stepff3"},
/*[211]*/ {VFH::T_I16,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__upsample_1i8u8","__upsample_2i8u8","__upsample_4i8u8","__upsample_8i8u8","__upsample_16i8u8","__upsample_3i8u8"},
/*[212]*/ {VFH::T_I16,VFH::T_I8,VFH::T_I8,VFH::T_NONE,"__upsample_1u8u8","__upsample_2u8u8","__upsample_4u8u8","__upsample_8u8u8","__upsample_16u8u8","__upsample_3u8u8"},
/*[213]*/ {VFH::T_I32,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__upsample_1i16u16","__upsample_2i16u16","__upsample_4i16u16","__upsample_8i16u16","__upsample_16i16u16","__upsample_3i16u16"},
/*[214]*/ {VFH::T_I32,VFH::T_I16,VFH::T_I16,VFH::T_NONE,"__upsample_1u16u16","__upsample_2u16u16","__upsample_4u16u16","__upsample_8u16u16","__upsample_16u16u16","__upsample_3u16u16"},
/*[215]*/ {VFH::T_I64,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__upsample_1i32u32","__upsample_2i32u32","__upsample_4i32u32","__upsample_8i32u32","__upsample_16i32u32","__upsample_3i32u32"},
/*[216]*/ {VFH::T_I64,VFH::T_I32,VFH::T_I32,VFH::T_NONE,"__upsample_1u32u32","__upsample_2u32u32","__upsample_4u32u32","__upsample_8u32u32","__upsample_16u32u32","__upsample_3u32u32"}
};


/*** Scalarizer hash ***/
VFH::hashEntry* __f__slist0[] = {&__f__node[39],&__f__node[70],&__f__node[133],&__f__node[172],NULL};
VFH::hashEntry* __f__slist1[] = {&__f__node[39],&__f__node[70],&__f__node[211],NULL};
VFH::hashEntry* __f__slist2[] = {&__f__node[50],&__f__node[130],&__f__node[132],&__f__node[136],&__f__node[202],NULL};
VFH::hashEntry* __f__slist3[] = {&__f__node[64],&__f__node[112],&__f__node[158],NULL};
VFH::hashEntry* __f__slist4[] = {&__f__node[39],&__f__node[64],&__f__node[145],NULL};
VFH::hashEntry* __f__slist5[] = {&__f__node[32],&__f__node[186],NULL};
VFH::hashEntry* __f__slist6[] = {&__f__node[10],&__f__node[32],&__f__node[157],NULL};
VFH::hashEntry* __f__slist7[] = {&__f__node[27],&__f__node[32],&__f__node[79],&__f__node[170],&__f__node[183],NULL};
VFH::hashEntry* __f__slist8[] = {&__f__node[38],&__f__node[81],&__f__node[92],&__f__node[130],&__f__node[145],&__f__node[159],NULL};
VFH::hashEntry* __f__slist9[] = {&__f__node[38],&__f__node[215],NULL};
VFH::hashEntry* __f__slist10[] = {&__f__node[130],NULL};
VFH::hashEntry* __f__slist11[] = {NULL};
VFH::hashEntry* __f__slist12[] = {&__f__node[100],&__f__node[157],NULL};
VFH::hashEntry* __f__slist13[] = {&__f__node[113],&__f__node[134],NULL};
VFH::hashEntry* __f__slist14[] = {&__f__node[211],NULL};
VFH::hashEntry* __f__slist15[] = {&__f__node[18],&__f__node[102],&__f__node[155],&__f__node[191],&__f__node[196],&__f__node[201],&__f__node[211],NULL};
VFH::hashEntry* __f__slist16[] = {&__f__node[90],&__f__node[121],&__f__node[132],&__f__node[201],&__f__node[212],NULL};
VFH::hashEntry* __f__slist17[] = {NULL};
VFH::hashEntry* __f__slist18[] = {&__f__node[49],&__f__node[159],&__f__node[161],&__f__node[213],NULL};
VFH::hashEntry* __f__slist19[] = {&__f__node[15],&__f__node[49],&__f__node[98],&__f__node[103],&__f__node[144],&__f__node[155],&__f__node[163],NULL};
VFH::hashEntry* __f__slist20[] = {&__f__node[15],&__f__node[38],&__f__node[49],&__f__node[100],NULL};
VFH::hashEntry* __f__slist21[] = {&__f__node[6],NULL};
VFH::hashEntry* __f__slist22[] = {&__f__node[29],&__f__node[129],&__f__node[175],NULL};
VFH::hashEntry* __f__slist23[] = {&__f__node[35],&__f__node[198],NULL};
VFH::hashEntry* __f__slist24[] = {&__f__node[35],&__f__node[59],&__f__node[113],NULL};
VFH::hashEntry* __f__slist25[] = {&__f__node[31],&__f__node[35],&__f__node[51],&__f__node[59],NULL};
VFH::hashEntry* __f__slist26[] = {&__f__node[31],&__f__node[184],NULL};
VFH::hashEntry* __f__slist27[] = {&__f__node[31],&__f__node[98],&__f__node[145],&__f__node[165],&__f__node[193],&__f__node[211],NULL};
VFH::hashEntry* __f__slist28[] = {NULL};
VFH::hashEntry* __f__slist29[] = {&__f__node[135],&__f__node[176],&__f__node[201],NULL};
VFH::hashEntry* __f__slist30[] = {&__f__node[83],&__f__node[125],&__f__node[128],NULL};
VFH::hashEntry* __f__slist31[] = {&__f__node[19],&__f__node[30],&__f__node[100],NULL};
VFH::hashEntry* __f__slist32[] = {&__f__node[96],NULL};
VFH::hashEntry* __f__slist33[] = {&__f__node[57],&__f__node[129],&__f__node[179],NULL};
VFH::hashEntry* __f__slist34[] = {&__f__node[57],&__f__node[105],&__f__node[194],NULL};
VFH::hashEntry* __f__slist35[] = {&__f__node[57],&__f__node[144],NULL};
VFH::hashEntry* __f__slist36[] = {&__f__node[133],NULL};
VFH::hashEntry* __f__slist37[] = {&__f__node[63],NULL};
VFH::hashEntry* __f__slist38[] = {&__f__node[63],&__f__node[97],&__f__node[105],&__f__node[123],NULL};
VFH::hashEntry* __f__slist39[] = {&__f__node[63],&__f__node[71],&__f__node[98],&__f__node[99],NULL};
VFH::hashEntry* __f__slist40[] = {&__f__node[71],&__f__node[135],&__f__node[143],&__f__node[167],NULL};
VFH::hashEntry* __f__slist41[] = {&__f__node[35],&__f__node[173],&__f__node[179],&__f__node[183],NULL};
VFH::hashEntry* __f__slist42[] = {&__f__node[46],&__f__node[70],&__f__node[127],&__f__node[171],&__f__node[194],&__f__node[216],NULL};
VFH::hashEntry* __f__slist43[] = {&__f__node[46],&__f__node[89],&__f__node[102],&__f__node[200],NULL};
VFH::hashEntry* __f__slist44[] = {&__f__node[46],&__f__node[133],&__f__node[139],&__f__node[162],&__f__node[166],&__f__node[171],NULL};
VFH::hashEntry* __f__slist45[] = {&__f__node[14],&__f__node[131],NULL};
VFH::hashEntry* __f__slist46[] = {&__f__node[14],&__f__node[82],&__f__node[166],&__f__node[176],&__f__node[177],&__f__node[185],NULL};
VFH::hashEntry* __f__slist47[] = {&__f__node[101],NULL};
VFH::hashEntry* __f__slist48[] = {&__f__node[161],&__f__node[165],&__f__node[193],NULL};
VFH::hashEntry* __f__slist49[] = {&__f__node[210],NULL};
VFH::hashEntry* __f__slist50[] = {&__f__node[28],&__f__node[56],&__f__node[93],&__f__node[162],&__f__node[171],&__f__node[210],NULL};
VFH::hashEntry* __f__slist51[] = {&__f__node[28],&__f__node[56],&__f__node[76],&__f__node[85],&__f__node[140],&__f__node[164],&__f__node[173],&__f__node[178],&__f__node[214],NULL};
VFH::hashEntry* __f__slist52[] = {&__f__node[20],&__f__node[28],&__f__node[56],&__f__node[76],&__f__node[131],&__f__node[141],&__f__node[152],&__f__node[205],NULL};
VFH::hashEntry* __f__slist53[] = {&__f__node[76],&__f__node[151],NULL};
VFH::hashEntry* __f__slist54[] = {&__f__node[95],&__f__node[187],NULL};
VFH::hashEntry* __f__slist55[] = {&__f__node[167],&__f__node[175],NULL};
VFH::hashEntry* __f__slist56[] = {&__f__node[47],&__f__node[142],&__f__node[163],&__f__node[175],NULL};
VFH::hashEntry* __f__slist57[] = {&__f__node[47],&__f__node[69],&__f__node[73],&__f__node[135],&__f__node[178],NULL};
VFH::hashEntry* __f__slist58[] = {&__f__node[47],&__f__node[72],&__f__node[75],&__f__node[91],&__f__node[164],&__f__node[173],NULL};
VFH::hashEntry* __f__slist59[] = {&__f__node[69],&__f__node[72],&__f__node[163],&__f__node[184],NULL};
VFH::hashEntry* __f__slist60[] = {&__f__node[59],&__f__node[106],&__f__node[138],&__f__node[167],&__f__node[195],NULL};
VFH::hashEntry* __f__slist61[] = {&__f__node[133],&__f__node[141],NULL};
VFH::hashEntry* __f__slist62[] = {&__f__node[121],NULL};
VFH::hashEntry* __f__slist63[] = {&__f__node[43],&__f__node[45],&__f__node[137],&__f__node[176],NULL};
VFH::hashEntry* __f__slist64[] = {&__f__node[43],&__f__node[164],NULL};
VFH::hashEntry* __f__slist65[] = {&__f__node[71],&__f__node[212],NULL};
VFH::hashEntry* __f__slist66[] = {&__f__node[158],&__f__node[168],NULL};
VFH::hashEntry* __f__slist67[] = {&__f__node[3],&__f__node[16],&__f__node[108],&__f__node[148],&__f__node[189],&__f__node[193],&__f__node[197],&__f__node[198],NULL};
VFH::hashEntry* __f__slist68[] = {&__f__node[3],&__f__node[140],&__f__node[158],&__f__node[165],NULL};
VFH::hashEntry* __f__slist69[] = {&__f__node[139],NULL};
VFH::hashEntry* __f__slist70[] = {&__f__node[127],&__f__node[154],&__f__node[175],NULL};
VFH::hashEntry* __f__slist71[] = {&__f__node[25],&__f__node[128],&__f__node[200],NULL};
VFH::hashEntry* __f__slist72[] = {&__f__node[25],&__f__node[77],&__f__node[78],&__f__node[142],&__f__node[162],&__f__node[166],NULL};
VFH::hashEntry* __f__slist73[] = {&__f__node[50],&__f__node[55],&__f__node[77],&__f__node[108],&__f__node[120],&__f__node[135],&__f__node[143],&__f__node[147],NULL};
VFH::hashEntry* __f__slist74[] = {&__f__node[26],&__f__node[77],&__f__node[123],&__f__node[124],NULL};
VFH::hashEntry* __f__slist75[] = {&__f__node[107],&__f__node[196],NULL};
VFH::hashEntry* __f__slist76[] = {&__f__node[6],&__f__node[86],&__f__node[142],&__f__node[144],&__f__node[146],&__f__node[169],NULL};
VFH::hashEntry* __f__slist77[] = {&__f__node[6],&__f__node[10],&__f__node[86],NULL};
VFH::hashEntry* __f__slist78[] = {&__f__node[6],&__f__node[10],&__f__node[110],&__f__node[136],&__f__node[156],&__f__node[212],NULL};
VFH::hashEntry* __f__slist79[] = {&__f__node[10],&__f__node[157],NULL};
VFH::hashEntry* __f__slist80[] = {&__f__node[7],&__f__node[111],&__f__node[140],NULL};
VFH::hashEntry* __f__slist81[] = {&__f__node[61],&__f__node[98],&__f__node[125],&__f__node[149],NULL};
VFH::hashEntry* __f__slist82[] = {&__f__node[106],&__f__node[203],NULL};
VFH::hashEntry* __f__slist83[] = {&__f__node[122],&__f__node[216],NULL};
VFH::hashEntry* __f__slist84[] = {&__f__node[109],&__f__node[182],&__f__node[198],NULL};
VFH::hashEntry* __f__slist85[] = {&__f__node[37],&__f__node[118],&__f__node[160],&__f__node[182],NULL};
VFH::hashEntry* __f__slist86[] = {&__f__node[126],NULL};
VFH::hashEntry* __f__slist87[] = {&__f__node[25],&__f__node[138],&__f__node[155],&__f__node[200],NULL};
VFH::hashEntry* __f__slist88[] = {&__f__node[121],&__f__node[167],&__f__node[199],NULL};
VFH::hashEntry* __f__slist89[] = {&__f__node[5],&__f__node[36],&__f__node[47],&__f__node[100],&__f__node[137],&__f__node[156],&__f__node[161],NULL};
VFH::hashEntry* __f__slist90[] = {&__f__node[36],&__f__node[108],&__f__node[192],NULL};
VFH::hashEntry* __f__slist91[] = {&__f__node[125],&__f__node[168],&__f__node[185],&__f__node[192],&__f__node[198],&__f__node[214],NULL};
VFH::hashEntry* __f__slist92[] = {&__f__node[4],&__f__node[27],&__f__node[81],&__f__node[88],&__f__node[163],&__f__node[179],&__f__node[192],&__f__node[197],NULL};
VFH::hashEntry* __f__slist93[] = {&__f__node[4],&__f__node[13],&__f__node[27],&__f__node[81],&__f__node[102],&__f__node[103],NULL};
VFH::hashEntry* __f__slist94[] = {&__f__node[27],&__f__node[110],NULL};
VFH::hashEntry* __f__slist95[] = {&__f__node[120],NULL};
VFH::hashEntry* __f__slist96[] = {&__f__node[110],&__f__node[150],&__f__node[154],&__f__node[189],NULL};
VFH::hashEntry* __f__slist97[] = {&__f__node[89],&__f__node[136],NULL};
VFH::hashEntry* __f__slist98[] = {&__f__node[31],&__f__node[123],NULL};
VFH::hashEntry* __f__slist99[] = {&__f__node[99],&__f__node[122],&__f__node[159],&__f__node[196],NULL};
VFH::hashEntry* __f__slist100[] = {&__f__node[165],&__f__node[187],NULL};
VFH::hashEntry* __f__slist101[] = {&__f__node[143],&__f__node[153],&__f__node[195],NULL};
VFH::hashEntry* __f__slist102[] = {&__f__node[7],&__f__node[43],&__f__node[127],&__f__node[169],&__f__node[184],NULL};
VFH::hashEntry* __f__slist103[] = {&__f__node[7],&__f__node[53],&__f__node[112],&__f__node[200],&__f__node[207],NULL};
VFH::hashEntry* __f__slist104[] = {&__f__node[17],&__f__node[139],&__f__node[177],&__f__node[207],NULL};
VFH::hashEntry* __f__slist105[] = {&__f__node[67],&__f__node[77],&__f__node[114],&__f__node[174],&__f__node[204],&__f__node[207],NULL};
VFH::hashEntry* __f__slist106[] = {&__f__node[40],&__f__node[67],&__f__node[101],&__f__node[106],&__f__node[114],&__f__node[125],NULL};
VFH::hashEntry* __f__slist107[] = {&__f__node[116],&__f__node[124],NULL};
VFH::hashEntry* __f__slist108[] = {&__f__node[168],&__f__node[210],NULL};
VFH::hashEntry* __f__slist109[] = {&__f__node[165],NULL};
VFH::hashEntry* __f__slist110[] = {&__f__node[104],NULL};
VFH::hashEntry* __f__slist111[] = {&__f__node[97],&__f__node[103],&__f__node[168],NULL};
VFH::hashEntry* __f__slist112[] = {&__f__node[58],NULL};
VFH::hashEntry* __f__slist113[] = {&__f__node[101],&__f__node[114],&__f__node[116],&__f__node[128],&__f__node[141],&__f__node[179],&__f__node[187],NULL};
VFH::hashEntry* __f__slist114[] = {&__f__node[116],NULL};
VFH::hashEntry* __f__slist115[] = {&__f__node[69],&__f__node[108],&__f__node[114],&__f__node[146],NULL};
VFH::hashEntry* __f__slist116[] = {&__f__node[24],&__f__node[163],&__f__node[190],NULL};
VFH::hashEntry* __f__slist117[] = {&__f__node[15],&__f__node[104],&__f__node[118],NULL};
VFH::hashEntry* __f__slist118[] = {&__f__node[89],&__f__node[127],&__f__node[207],NULL};
VFH::hashEntry* __f__slist119[] = {&__f__node[44],&__f__node[57],&__f__node[185],NULL};
VFH::hashEntry* __f__slist120[] = {&__f__node[80],&__f__node[167],NULL};
VFH::hashEntry* __f__slist121[] = {&__f__node[80],&__f__node[99],&__f__node[116],&__f__node[143],&__f__node[185],&__f__node[199],NULL};
VFH::hashEntry* __f__slist122[] = {NULL};
VFH::hashEntry* __f__slist123[] = {&__f__node[148],NULL};
VFH::hashEntry* __f__slist124[] = {&__f__node[68],&__f__node[107],NULL};
VFH::hashEntry* __f__slist125[] = {&__f__node[55],&__f__node[64],&__f__node[170],NULL};
VFH::hashEntry* __f__slist126[] = {&__f__node[55],&__f__node[169],NULL};
VFH::hashEntry* __f__slist127[] = {&__f__node[101],&__f__node[150],&__f__node[156],&__f__node[187],NULL};
VFH::hashEntry* __f__slist128[] = {&__f__node[2],&__f__node[23],&__f__node[126],&__f__node[186],NULL};
VFH::hashEntry* __f__slist129[] = {&__f__node[113],&__f__node[179],&__f__node[208],NULL};
VFH::hashEntry* __f__slist130[] = {&__f__node[178],&__f__node[208],NULL};
VFH::hashEntry* __f__slist131[] = {&__f__node[29],&__f__node[79],&__f__node[103],NULL};
VFH::hashEntry* __f__slist132[] = {&__f__node[29],&__f__node[48],&__f__node[79],&__f__node[109],NULL};
VFH::hashEntry* __f__slist133[] = {&__f__node[29],&__f__node[78],&__f__node[189],NULL};
VFH::hashEntry* __f__slist134[] = {&__f__node[78],&__f__node[141],&__f__node[172],NULL};
VFH::hashEntry* __f__slist135[] = {&__f__node[23],&__f__node[78],&__f__node[101],&__f__node[152],NULL};
VFH::hashEntry* __f__slist136[] = {&__f__node[2],&__f__node[41],&__f__node[49],&__f__node[65],&__f__node[99],&__f__node[118],&__f__node[154],&__f__node[188],&__f__node[199],NULL};
VFH::hashEntry* __f__slist137[] = {&__f__node[2],&__f__node[41],&__f__node[62],&__f__node[65],&__f__node[105],&__f__node[113],NULL};
VFH::hashEntry* __f__slist138[] = {&__f__node[62],NULL};
VFH::hashEntry* __f__slist139[] = {&__f__node[62],NULL};
VFH::hashEntry* __f__slist140[] = {&__f__node[20],&__f__node[75],&__f__node[83],NULL};
VFH::hashEntry* __f__slist141[] = {&__f__node[20],&__f__node[75],&__f__node[83],&__f__node[139],NULL};
VFH::hashEntry* __f__slist142[] = {&__f__node[36],&__f__node[145],NULL};
VFH::hashEntry* __f__slist143[] = {NULL};
VFH::hashEntry* __f__slist144[] = {&__f__node[99],&__f__node[112],NULL};
VFH::hashEntry* __f__slist145[] = {&__f__node[8],&__f__node[33],&__f__node[150],&__f__node[180],NULL};
VFH::hashEntry* __f__slist146[] = {&__f__node[8],&__f__node[33],&__f__node[128],&__f__node[151],&__f__node[180],&__f__node[194],&__f__node[214],NULL};
VFH::hashEntry* __f__slist147[] = {&__f__node[8],&__f__node[33],&__f__node[189],NULL};
VFH::hashEntry* __f__slist148[] = {NULL};
VFH::hashEntry* __f__slist149[] = {&__f__node[46],&__f__node[56],&__f__node[107],&__f__node[126],&__f__node[158],&__f__node[197],NULL};
VFH::hashEntry* __f__slist150[] = {&__f__node[17],&__f__node[118],&__f__node[147],&__f__node[149],&__f__node[213],NULL};
VFH::hashEntry* __f__slist151[] = {&__f__node[17],&__f__node[68],&__f__node[105],&__f__node[144],&__f__node[199],NULL};
VFH::hashEntry* __f__slist152[] = {&__f__node[126],NULL};
VFH::hashEntry* __f__slist153[] = {&__f__node[94],&__f__node[109],&__f__node[111],&__f__node[199],NULL};
VFH::hashEntry* __f__slist154[] = {&__f__node[34],&__f__node[42],&__f__node[216],NULL};
VFH::hashEntry* __f__slist155[] = {&__f__node[42],&__f__node[122],&__f__node[195],NULL};
VFH::hashEntry* __f__slist156[] = {&__f__node[42],&__f__node[109],&__f__node[120],&__f__node[158],&__f__node[172],NULL};
VFH::hashEntry* __f__slist157[] = {&__f__node[48],&__f__node[53],&__f__node[73],&__f__node[91],&__f__node[110],&__f__node[147],&__f__node[149],&__f__node[153],&__f__node[195],&__f__node[204],NULL};
VFH::hashEntry* __f__slist158[] = {&__f__node[48],&__f__node[53],&__f__node[58],&__f__node[89],&__f__node[204],&__f__node[215],NULL};
VFH::hashEntry* __f__slist159[] = {&__f__node[0],&__f__node[53],&__f__node[58],&__f__node[152],&__f__node[170],NULL};
VFH::hashEntry* __f__slist160[] = {&__f__node[102],&__f__node[206],NULL};
VFH::hashEntry* __f__slist161[] = {&__f__node[60],&__f__node[84],&__f__node[107],&__f__node[151],&__f__node[156],&__f__node[160],&__f__node[177],NULL};
VFH::hashEntry* __f__slist162[] = {&__f__node[28],&__f__node[40],&__f__node[84],&__f__node[129],NULL};
VFH::hashEntry* __f__slist163[] = {&__f__node[26],&__f__node[52],&__f__node[150],&__f__node[197],&__f__node[208],NULL};
VFH::hashEntry* __f__slist164[] = {&__f__node[26],&__f__node[92],&__f__node[103],&__f__node[124],&__f__node[170],NULL};
VFH::hashEntry* __f__slist165[] = {&__f__node[26],&__f__node[91],&__f__node[93],&__f__node[95],&__f__node[129],&__f__node[146],&__f__node[149],NULL};
VFH::hashEntry* __f__slist166[] = {&__f__node[170],&__f__node[172],NULL};
VFH::hashEntry* __f__slist167[] = {&__f__node[22],&__f__node[119],&__f__node[148],&__f__node[193],NULL};
VFH::hashEntry* __f__slist168[] = {&__f__node[16],&__f__node[22],&__f__node[94],&__f__node[126],&__f__node[136],&__f__node[160],NULL};
VFH::hashEntry* __f__slist169[] = {&__f__node[22],&__f__node[95],&__f__node[108],&__f__node[111],&__f__node[130],&__f__node[154],NULL};
VFH::hashEntry* __f__slist170[] = {&__f__node[1],&__f__node[11],&__f__node[22],&__f__node[72],&__f__node[124],&__f__node[135],&__f__node[174],NULL};
VFH::hashEntry* __f__slist171[] = {&__f__node[178],&__f__node[205],&__f__node[214],NULL};
VFH::hashEntry* __f__slist172[] = {&__f__node[96],&__f__node[205],NULL};
VFH::hashEntry* __f__slist173[] = {&__f__node[3],&__f__node[90],&__f__node[93],&__f__node[111],&__f__node[129],&__f__node[146],&__f__node[197],&__f__node[205],NULL};
VFH::hashEntry* __f__slist174[] = {&__f__node[44],&__f__node[52],&__f__node[76],&__f__node[148],&__f__node[153],&__f__node[172],NULL};
VFH::hashEntry* __f__slist175[] = {&__f__node[44],&__f__node[52],&__f__node[146],NULL};
VFH::hashEntry* __f__slist176[] = {&__f__node[1],&__f__node[9],&__f__node[52],&__f__node[132],&__f__node[137],NULL};
VFH::hashEntry* __f__slist177[] = {&__f__node[91],&__f__node[106],NULL};
VFH::hashEntry* __f__slist178[] = {&__f__node[122],&__f__node[145],&__f__node[150],NULL};
VFH::hashEntry* __f__slist179[] = {NULL};
VFH::hashEntry* __f__slist180[] = {&__f__node[98],&__f__node[122],&__f__node[216],NULL};
VFH::hashEntry* __f__slist181[] = {&__f__node[0],&__f__node[90],&__f__node[92],&__f__node[96],&__f__node[148],&__f__node[168],&__f__node[183],&__f__node[195],NULL};
VFH::hashEntry* __f__slist182[] = {&__f__node[12],&__f__node[194],NULL};
VFH::hashEntry* __f__slist183[] = {&__f__node[0],&__f__node[54],&__f__node[80],&__f__node[87],&__f__node[132],NULL};
VFH::hashEntry* __f__slist184[] = {&__f__node[87],NULL};
VFH::hashEntry* __f__slist185[] = {&__f__node[87],&__f__node[94],&__f__node[174],NULL};
VFH::hashEntry* __f__slist186[] = {&__f__node[93],&__f__node[112],&__f__node[117],&__f__node[155],NULL};
VFH::hashEntry* __f__slist187[] = {&__f__node[115],&__f__node[134],&__f__node[180],NULL};
VFH::hashEntry* __f__slist188[] = {&__f__node[87],&__f__node[124],NULL};
VFH::hashEntry* __f__slist189[] = {&__f__node[67],&__f__node[92],&__f__node[100],NULL};
VFH::hashEntry* __f__slist190[] = {&__f__node[112],NULL};
VFH::hashEntry* __f__slist191[] = {&__f__node[144],NULL};
VFH::hashEntry* __f__slist192[] = {&__f__node[130],&__f__node[188],NULL};
VFH::hashEntry* __f__slist193[] = {NULL};
VFH::hashEntry* __f__slist194[] = {&__f__node[45],&__f__node[51],&__f__node[151],&__f__node[157],NULL};
VFH::hashEntry* __f__slist195[] = {&__f__node[45],&__f__node[51],&__f__node[66],&__f__node[115],&__f__node[141],NULL};
VFH::hashEntry* __f__slist196[] = {&__f__node[45],&__f__node[65],&__f__node[117],&__f__node[189],NULL};
VFH::hashEntry* __f__slist197[] = {&__f__node[190],&__f__node[192],NULL};
VFH::hashEntry* __f__slist198[] = {&__f__node[96],NULL};
VFH::hashEntry* __f__slist199[] = {&__f__node[118],NULL};
VFH::hashEntry* __f__slist200[] = {&__f__node[89],&__f__node[104],&__f__node[119],&__f__node[133],NULL};
VFH::hashEntry* __f__slist201[] = {&__f__node[111],&__f__node[186],NULL};
VFH::hashEntry* __f__slist202[] = {&__f__node[95],&__f__node[137],&__f__node[161],&__f__node[176],NULL};
VFH::hashEntry* __f__slist203[] = {&__f__node[116],&__f__node[117],&__f__node[174],NULL};
VFH::hashEntry* __f__slist204[] = {&__f__node[62],&__f__node[139],NULL};
VFH::hashEntry* __f__slist205[] = {&__f__node[121],NULL};
VFH::hashEntry* __f__slist206[] = {&__f__node[134],&__f__node[184],&__f__node[206],NULL};
VFH::hashEntry* __f__slist207[] = {&__f__node[41],&__f__node[74],&__f__node[88],&__f__node[131],&__f__node[178],&__f__node[206],&__f__node[212],NULL};
VFH::hashEntry* __f__slist208[] = {&__f__node[186],&__f__node[211],NULL};
VFH::hashEntry* __f__slist209[] = {&__f__node[19],&__f__node[30],&__f__node[74],&__f__node[149],&__f__node[187],&__f__node[188],NULL};
VFH::hashEntry* __f__slist210[] = {&__f__node[19],&__f__node[30],&__f__node[114],NULL};
VFH::hashEntry* __f__slist211[] = {NULL};
VFH::hashEntry* __f__slist212[] = {&__f__node[74],&__f__node[190],NULL};
VFH::hashEntry* __f__slist213[] = {&__f__node[97],&__f__node[120],&__f__node[176],NULL};
VFH::hashEntry* __f__slist214[] = {&__f__node[54],&__f__node[109],&__f__node[153],&__f__node[184],NULL};
VFH::hashEntry* __f__slist215[] = {&__f__node[54],&__f__node[88],&__f__node[119],&__f__node[159],&__f__node[188],NULL};
VFH::hashEntry* __f__slist216[] = {&__f__node[54],&__f__node[185],&__f__node[202],NULL};
VFH::hashEntry* __f__slist217[] = {&__f__node[147],&__f__node[181],&__f__node[202],NULL};
VFH::hashEntry* __f__slist218[] = {&__f__node[95],NULL};
VFH::hashEntry* __f__slist219[] = {&__f__node[212],&__f__node[213],NULL};
VFH::hashEntry* __f__slist220[] = {&__f__node[90],&__f__node[177],NULL};
VFH::hashEntry* __f__slist221[] = {&__f__node[34],&__f__node[107],&__f__node[134],&__f__node[138],&__f__node[166],&__f__node[183],NULL};
VFH::hashEntry* __f__slist222[] = {&__f__node[34],&__f__node[115],NULL};
VFH::hashEntry* __f__slist223[] = {NULL};
VFH::hashEntry* __f__slist224[] = {&__f__node[8],&__f__node[143],&__f__node[164],NULL};
VFH::hashEntry* __f__slist225[] = {&__f__node[85],&__f__node[160],&__f__node[182],&__f__node[194],&__f__node[213],NULL};
VFH::hashEntry* __f__slist226[] = {&__f__node[5],&__f__node[32],&__f__node[61],&__f__node[85],NULL};
VFH::hashEntry* __f__slist227[] = {&__f__node[5],&__f__node[61],&__f__node[92],NULL};
VFH::hashEntry* __f__slist228[] = {&__f__node[33],&__f__node[121],&__f__node[140],&__f__node[161],&__f__node[200],&__f__node[215],NULL};
VFH::hashEntry* __f__slist229[] = {&__f__node[9],&__f__node[21],&__f__node[117],&__f__node[177],NULL};
VFH::hashEntry* __f__slist230[] = {&__f__node[9],&__f__node[63],NULL};
VFH::hashEntry* __f__slist231[] = {&__f__node[9],&__f__node[88],&__f__node[154],NULL};
VFH::hashEntry* __f__slist232[] = {&__f__node[18],NULL};
VFH::hashEntry* __f__slist233[] = {&__f__node[14],&__f__node[82],&__f__node[162],&__f__node[198],&__f__node[215],NULL};
VFH::hashEntry* __f__slist234[] = {&__f__node[18],&__f__node[82],&__f__node[119],&__f__node[152],&__f__node[175],NULL};
VFH::hashEntry* __f__slist235[] = {&__f__node[94],&__f__node[97],&__f__node[127],&__f__node[140],NULL};
VFH::hashEntry* __f__slist236[] = {&__f__node[152],&__f__node[183],NULL};
VFH::hashEntry* __f__slist237[] = {&__f__node[166],NULL};
VFH::hashEntry* __f__slist238[] = {&__f__node[11],&__f__node[12],&__f__node[21],&__f__node[151],&__f__node[156],&__f__node[173],NULL};
VFH::hashEntry* __f__slist239[] = {&__f__node[12],&__f__node[21],&__f__node[102],&__f__node[120],&__f__node[123],&__f__node[181],NULL};
VFH::hashEntry* __f__slist240[] = {&__f__node[88],&__f__node[94],&__f__node[181],&__f__node[190],&__f__node[196],NULL};
VFH::hashEntry* __f__slist241[] = {&__f__node[104],&__f__node[164],&__f__node[193],NULL};
VFH::hashEntry* __f__slist242[] = {NULL};
VFH::hashEntry* __f__slist243[] = {&__f__node[37],&__f__node[84],&__f__node[119],&__f__node[137],&__f__node[138],&__f__node[159],&__f__node[169],NULL};
VFH::hashEntry* __f__slist244[] = {&__f__node[24],&__f__node[37],&__f__node[60],&__f__node[113],&__f__node[128],&__f__node[215],NULL};
VFH::hashEntry* __f__slist245[] = {&__f__node[24],&__f__node[60],&__f__node[66],&__f__node[190],NULL};
VFH::hashEntry* __f__slist246[] = {&__f__node[60],&__f__node[66],&__f__node[155],&__f__node[171],&__f__node[209],&__f__node[214],NULL};
VFH::hashEntry* __f__slist247[] = {&__f__node[13],&__f__node[93],&__f__node[117],&__f__node[125],&__f__node[142],&__f__node[209],NULL};
VFH::hashEntry* __f__slist248[] = {&__f__node[4],&__f__node[131],&__f__node[191],&__f__node[209],NULL};
VFH::hashEntry* __f__slist249[] = {&__f__node[142],&__f__node[153],&__f__node[191],&__f__node[203],NULL};
VFH::hashEntry* __f__slist250[] = {&__f__node[96],&__f__node[104],&__f__node[203],NULL};
VFH::hashEntry* __f__slist251[] = {&__f__node[97],&__f__node[132],&__f__node[136],&__f__node[160],&__f__node[169],NULL};
VFH::hashEntry* __f__slist252[] = {&__f__node[86],&__f__node[174],&__f__node[213],NULL};
VFH::hashEntry* __f__slist253[] = {&__f__node[105],&__f__node[188],NULL};
VFH::hashEntry* __f__slist254[] = {&__f__node[91],&__f__node[134],NULL};
VFH::hashEntry* __f__slist255[] = {&__f__node[39],&__f__node[42],&__f__node[115],&__f__node[157],&__f__node[216],NULL};

VFH::hashEntry** __f__scalarizer_hash_table[] = {__f__slist0,__f__slist1,__f__slist2,__f__slist3,__f__slist4,__f__slist5,__f__slist6,__f__slist7,__f__slist8,__f__slist9,__f__slist10,__f__slist11,__f__slist12,__f__slist13,__f__slist14,__f__slist15,__f__slist16,__f__slist17,__f__slist18,__f__slist19,__f__slist20,__f__slist21,__f__slist22,__f__slist23,__f__slist24,__f__slist25,__f__slist26,__f__slist27,__f__slist28,__f__slist29,__f__slist30,__f__slist31,__f__slist32,__f__slist33,__f__slist34,__f__slist35,__f__slist36,__f__slist37,__f__slist38,__f__slist39,__f__slist40,__f__slist41,__f__slist42,__f__slist43,__f__slist44,__f__slist45,__f__slist46,__f__slist47,__f__slist48,__f__slist49,__f__slist50,__f__slist51,__f__slist52,__f__slist53,__f__slist54,__f__slist55,__f__slist56,__f__slist57,__f__slist58,__f__slist59,__f__slist60,__f__slist61,__f__slist62,__f__slist63,__f__slist64,__f__slist65,__f__slist66,__f__slist67,__f__slist68,__f__slist69,__f__slist70,__f__slist71,__f__slist72,__f__slist73,__f__slist74,__f__slist75,__f__slist76,__f__slist77,__f__slist78,__f__slist79,__f__slist80,__f__slist81,__f__slist82,__f__slist83,__f__slist84,__f__slist85,__f__slist86,__f__slist87,__f__slist88,__f__slist89,__f__slist90,__f__slist91,__f__slist92,__f__slist93,__f__slist94,__f__slist95,__f__slist96,__f__slist97,__f__slist98,__f__slist99,__f__slist100,__f__slist101,__f__slist102,__f__slist103,__f__slist104,__f__slist105,__f__slist106,__f__slist107,__f__slist108,__f__slist109,__f__slist110,__f__slist111,__f__slist112,__f__slist113,__f__slist114,__f__slist115,__f__slist116,__f__slist117,__f__slist118,__f__slist119,__f__slist120,__f__slist121,__f__slist122,__f__slist123,__f__slist124,__f__slist125,__f__slist126,__f__slist127,__f__slist128,__f__slist129,__f__slist130,__f__slist131,__f__slist132,__f__slist133,__f__slist134,__f__slist135,__f__slist136,__f__slist137,__f__slist138,__f__slist139,__f__slist140,__f__slist141,__f__slist142,__f__slist143,__f__slist144,__f__slist145,__f__slist146,__f__slist147,__f__slist148,__f__slist149,__f__slist150,__f__slist151,__f__slist152,__f__slist153,__f__slist154,__f__slist155,__f__slist156,__f__slist157,__f__slist158,__f__slist159,__f__slist160,__f__slist161,__f__slist162,__f__slist163,__f__slist164,__f__slist165,__f__slist166,__f__slist167,__f__slist168,__f__slist169,__f__slist170,__f__slist171,__f__slist172,__f__slist173,__f__slist174,__f__slist175,__f__slist176,__f__slist177,__f__slist178,__f__slist179,__f__slist180,__f__slist181,__f__slist182,__f__slist183,__f__slist184,__f__slist185,__f__slist186,__f__slist187,__f__slist188,__f__slist189,__f__slist190,__f__slist191,__f__slist192,__f__slist193,__f__slist194,__f__slist195,__f__slist196,__f__slist197,__f__slist198,__f__slist199,__f__slist200,__f__slist201,__f__slist202,__f__slist203,__f__slist204,__f__slist205,__f__slist206,__f__slist207,__f__slist208,__f__slist209,__f__slist210,__f__slist211,__f__slist212,__f__slist213,__f__slist214,__f__slist215,__f__slist216,__f__slist217,__f__slist218,__f__slist219,__f__slist220,__f__slist221,__f__slist222,__f__slist223,__f__slist224,__f__slist225,__f__slist226,__f__slist227,__f__slist228,__f__slist229,__f__slist230,__f__slist231,__f__slist232,__f__slist233,__f__slist234,__f__slist235,__f__slist236,__f__slist237,__f__slist238,__f__slist239,__f__slist240,__f__slist241,__f__slist242,__f__slist243,__f__slist244,__f__slist245,__f__slist246,__f__slist247,__f__slist248,__f__slist249,__f__slist250,__f__slist251,__f__slist252,__f__slist253,__f__slist254,__f__slist255};
//longest bucket: 10


/*** Vectorizer hash ***/
VFH::hashEntry* __f__vlist0[] = {&__f__node[6],&__f__node[150],NULL};
VFH::hashEntry* __f__vlist1[] = {&__f__node[105],&__f__node[153],NULL};
VFH::hashEntry* __f__vlist2[] = {&__f__node[41],&__f__node[73],&__f__node[97],&__f__node[120],&__f__node[130],&__f__node[196],NULL};
VFH::hashEntry* __f__vlist3[] = {&__f__node[23],&__f__node[100],NULL};
VFH::hashEntry* __f__vlist4[] = {&__f__node[13],&__f__node[27],NULL};
VFH::hashEntry* __f__vlist5[] = {&__f__node[39],&__f__node[50],&__f__node[70],&__f__node[71],&__f__node[139],&__f__node[175],&__f__node[190],NULL};
VFH::hashEntry* __f__vlist6[] = {&__f__node[38],&__f__node[84],&__f__node[91],&__f__node[137],&__f__node[216],NULL};
VFH::hashEntry* __f__vlist7[] = {&__f__node[76],&__f__node[142],&__f__node[145],&__f__node[187],NULL};
VFH::hashEntry* __f__vlist8[] = {&__f__node[42],&__f__node[65],&__f__node[83],&__f__node[199],NULL};
VFH::hashEntry* __f__vlist9[] = {&__f__node[2],&__f__node[178],NULL};
VFH::hashEntry* __f__vlist10[] = {&__f__node[75],&__f__node[94],&__f__node[182],&__f__node[198],&__f__node[206],NULL};
VFH::hashEntry* __f__vlist11[] = {&__f__node[132],NULL};
VFH::hashEntry* __f__vlist12[] = {&__f__node[20],&__f__node[67],&__f__node[109],NULL};
VFH::hashEntry* __f__vlist13[] = {&__f__node[72],&__f__node[141],&__f__node[149],NULL};
VFH::hashEntry* __f__vlist14[] = {&__f__node[92],&__f__node[110],&__f__node[160],&__f__node[214],NULL};
VFH::hashEntry* __f__vlist15[] = {&__f__node[155],&__f__node[185],NULL};
VFH::hashEntry* __f__vlist16[] = {&__f__node[14],NULL};
VFH::hashEntry* __f__vlist17[] = {&__f__node[121],&__f__node[151],NULL};
VFH::hashEntry* __f__vlist18[] = {NULL};
VFH::hashEntry* __f__vlist19[] = {&__f__node[37],&__f__node[103],&__f__node[184],&__f__node[189],&__f__node[209],NULL};
VFH::hashEntry* __f__vlist20[] = {&__f__node[158],&__f__node[165],&__f__node[195],NULL};
VFH::hashEntry* __f__vlist21[] = {&__f__node[69],&__f__node[78],&__f__node[88],&__f__node[107],&__f__node[136],&__f__node[203],NULL};
VFH::hashEntry* __f__vlist22[] = {&__f__node[17],&__f__node[90],&__f__node[119],&__f__node[147],&__f__node[157],&__f__node[172],NULL};
VFH::hashEntry* __f__vlist23[] = {&__f__node[86],&__f__node[200],NULL};
VFH::hashEntry* __f__vlist24[] = {&__f__node[59],&__f__node[61],&__f__node[111],NULL};
VFH::hashEntry* __f__vlist25[] = {&__f__node[29],&__f__node[43],&__f__node[128],&__f__node[188],NULL};
VFH::hashEntry* __f__vlist26[] = {&__f__node[58],&__f__node[60],&__f__node[95],&__f__node[174],NULL};
VFH::hashEntry* __f__vlist27[] = {&__f__node[33],NULL};
VFH::hashEntry* __f__vlist28[] = {&__f__node[5],&__f__node[46],&__f__node[163],&__f__node[197],NULL};
VFH::hashEntry* __f__vlist29[] = {&__f__node[32],&__f__node[62],&__f__node[161],&__f__node[166],NULL};
VFH::hashEntry* __f__vlist30[] = {&__f__node[56],&__f__node[66],&__f__node[148],NULL};
VFH::hashEntry* __f__vlist31[] = {&__f__node[3],&__f__node[10],&__f__node[81],&__f__node[169],&__f__node[170],&__f__node[211],&__f__node[212],NULL};
VFH::hashEntry* __f__vlist32[] = {&__f__node[18],&__f__node[55],&__f__node[154],NULL};
VFH::hashEntry* __f__vlist33[] = {&__f__node[8],&__f__node[116],&__f__node[186],NULL};
VFH::hashEntry* __f__vlist34[] = {&__f__node[125],&__f__node[183],NULL};
VFH::hashEntry* __f__vlist35[] = {&__f__node[57],&__f__node[64],&__f__node[108],&__f__node[159],&__f__node[173],NULL};
VFH::hashEntry* __f__vlist36[] = {&__f__node[16],&__f__node[44],&__f__node[89],NULL};
VFH::hashEntry* __f__vlist37[] = {&__f__node[74],&__f__node[118],&__f__node[146],&__f__node[176],NULL};
VFH::hashEntry* __f__vlist38[] = {&__f__node[52],NULL};
VFH::hashEntry* __f__vlist39[] = {&__f__node[4],&__f__node[31],NULL};
VFH::hashEntry* __f__vlist40[] = {&__f__node[21],&__f__node[126],&__f__node[129],&__f__node[167],NULL};
VFH::hashEntry* __f__vlist41[] = {&__f__node[156],NULL};
VFH::hashEntry* __f__vlist42[] = {&__f__node[47],&__f__node[106],&__f__node[114],&__f__node[164],&__f__node[171],NULL};
VFH::hashEntry* __f__vlist43[] = {&__f__node[34],&__f__node[123],NULL};
VFH::hashEntry* __f__vlist44[] = {NULL};
VFH::hashEntry* __f__vlist45[] = {&__f__node[9],&__f__node[113],&__f__node[131],NULL};
VFH::hashEntry* __f__vlist46[] = {&__f__node[134],&__f__node[168],&__f__node[193],NULL};
VFH::hashEntry* __f__vlist47[] = {&__f__node[12],&__f__node[15],&__f__node[19],&__f__node[40],&__f__node[63],&__f__node[99],NULL};
VFH::hashEntry* __f__vlist48[] = {&__f__node[215],NULL};
VFH::hashEntry* __f__vlist49[] = {&__f__node[35],&__f__node[205],NULL};
VFH::hashEntry* __f__vlist50[] = {&__f__node[24],&__f__node[79],&__f__node[104],&__f__node[152],&__f__node[162],NULL};
VFH::hashEntry* __f__vlist51[] = {&__f__node[7],&__f__node[11],&__f__node[36],&__f__node[82],&__f__node[96],&__f__node[138],NULL};
VFH::hashEntry* __f__vlist52[] = {&__f__node[49],&__f__node[117],&__f__node[133],&__f__node[180],&__f__node[191],NULL};
VFH::hashEntry* __f__vlist53[] = {&__f__node[26],&__f__node[124],&__f__node[177],NULL};
VFH::hashEntry* __f__vlist54[] = {&__f__node[101],NULL};
VFH::hashEntry* __f__vlist55[] = {&__f__node[1],&__f__node[48],&__f__node[53],&__f__node[127],NULL};
VFH::hashEntry* __f__vlist56[] = {&__f__node[68],&__f__node[135],&__f__node[144],&__f__node[201],NULL};
VFH::hashEntry* __f__vlist57[] = {&__f__node[0],&__f__node[80],&__f__node[143],&__f__node[179],&__f__node[213],NULL};
VFH::hashEntry* __f__vlist58[] = {&__f__node[25],&__f__node[45],&__f__node[77],&__f__node[98],NULL};
VFH::hashEntry* __f__vlist59[] = {&__f__node[22],&__f__node[115],&__f__node[140],&__f__node[207],NULL};
VFH::hashEntry* __f__vlist60[] = {&__f__node[54],&__f__node[112],&__f__node[122],NULL};
VFH::hashEntry* __f__vlist61[] = {&__f__node[87],&__f__node[194],NULL};
VFH::hashEntry* __f__vlist62[] = {&__f__node[51],&__f__node[93],NULL};
VFH::hashEntry* __f__vlist63[] = {&__f__node[85],&__f__node[102],NULL};

VFH::hashEntry** __f__vectorizer_hash_table[] = {__f__vlist0,__f__vlist1,__f__vlist2,__f__vlist3,__f__vlist4,__f__vlist5,__f__vlist6,__f__vlist7,__f__vlist8,__f__vlist9,__f__vlist10,__f__vlist11,__f__vlist12,__f__vlist13,__f__vlist14,__f__vlist15,__f__vlist16,__f__vlist17,__f__vlist18,__f__vlist19,__f__vlist20,__f__vlist21,__f__vlist22,__f__vlist23,__f__vlist24,__f__vlist25,__f__vlist26,__f__vlist27,__f__vlist28,__f__vlist29,__f__vlist30,__f__vlist31,__f__vlist32,__f__vlist33,__f__vlist34,__f__vlist35,__f__vlist36,__f__vlist37,__f__vlist38,__f__vlist39,__f__vlist40,__f__vlist41,__f__vlist42,__f__vlist43,__f__vlist44,__f__vlist45,__f__vlist46,__f__vlist47,__f__vlist48,__f__vlist49,__f__vlist50,__f__vlist51,__f__vlist52,__f__vlist53,__f__vlist54,__f__vlist55,__f__vlist56,__f__vlist57,__f__vlist58,__f__vlist59,__f__vlist60,__f__vlist61,__f__vlist62,__f__vlist63};
//longest bucket: 7


static unsigned calculate_hash(std::string funcName, bool isScalarizerHash)
{
	// Modified Bernstein hash function.
	unsigned hashSize = isScalarizerHash ? SCALARIZER_HASH_SIZE : VECTORIZER_HASH_SIZE;
	size_t lngth = funcName.size();
	unsigned Result = 0;
	for (unsigned s = 0; s < lngth; s++)
	{
		Result = Result * 33 ^ funcName.at(s);
	}
	Result = Result + (Result >> 5);
	return ( (Result & ((hashSize-1) << 3)) >> 3);
}


VFH::hashEntry * VFH::findScalarFunctionInHash(std::string &inp_name)
{
	unsigned hash_key = calculate_hash(inp_name, false);
	if (hash_key >= VECTORIZER_HASH_SIZE) return NULL;

	hashEntry ** nodes_list = __f__vectorizer_hash_table[hash_key];

	hashEntry * current = nodes_list[0];
	unsigned i = 0;
	while (current != NULL)
	{
		// Check if current function fits the requested name
		if (current->funcs[0] == inp_name)
			return current;
		// if not - move along to next function
		current = nodes_list[++i];
	}
	return NULL; // failed to find function
}

static unsigned orderToWidth[] = {1, 2, 4, 8, 16, 3};
VFH::hashEntry * VFH::findVectorFunctionInHash(std::string &inp_name, unsigned * vecWidth)
{
	unsigned hash_key = calculate_hash(inp_name, true);
	if (hash_key >= SCALARIZER_HASH_SIZE) return NULL;

	hashEntry ** nodes_list = __f__scalarizer_hash_table[hash_key];

	hashEntry * current = nodes_list[0];
	unsigned i = 0;
	while (current != NULL)
	{
		// Check if current function fits the requested name
		for (unsigned j = 1; j < 6; j++)
		{
			if (current->funcs[j] == inp_name)
			{
				*vecWidth = orderToWidth[j];
				return current;
			}
		}
		// if not - move along to next function
		current = nodes_list[++i];
	}
	return NULL; // failed to find function
}

unsigned VFH::debugGetNumEntries()
{
	return 217;
}

VFH::hashEntry * VFH::debugGetEntry(unsigned num)
{
	return &__f__node[num];
}

