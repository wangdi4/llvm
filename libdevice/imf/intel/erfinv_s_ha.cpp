/*******************************************************************************
* INTEL CONFIDENTIAL
* Modifications, Copyright (C) 1996-2022 Intel Corporation
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
*******************************************************************************/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_erfinv_s_ha
{
namespace {
static const union
{
    uint32_t w;
    float f;
} __serfinv_ha_two32 = { 0x4f800000u };

static const union
{
    uint32_t w;
    float f;
} __serfinv_ha_c10 = { 0xbd8770dcu };

static const union
{
    uint32_t w;
    float f;
} __serfinv_ha_c9 = { 0x3e1c5827u };

static const union
{
    uint32_t w;
    float f;
} __serfinv_ha_c8 = { 0xbe434bdeu };

static const union
{
    uint32_t w;
    float f;
} __serfinv_ha_c7 = { 0x3e55e9b0u };

static const union
{
    uint32_t w;
    float f;
} __serfinv_ha_c6 = { 0xbe75d6cau };

static const union
{
    uint32_t w;
    float f;
} __serfinv_ha_c5 = { 0x3e93a7cbu };

static const union
{
    uint32_t w;
    float f;
} __serfinv_ha_c4 = { 0xbeb8aabcu };

static const union
{
    uint32_t w;
    float f;
} __serfinv_ha_c3 = { 0x3ef6389fu };

static const union
{
    uint32_t w;
    float f;
} __serfinv_ha_c2h = { 0xbf38aa3bu };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_ldc1 = { 0xbff715476395bd86UL };

///////////////////////////////////////////////////////////////////
// erfinv coefficients for [2^(-13), 0.75)
static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_dc13 = { 0x40364513fb32496aUL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_dc12 = { 0xc055e615de9c23d8UL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_dc11 = { 0x4063250adeb67481UL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_dc10 = { 0xc063642141c3fcd8UL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_dc9 = { 0x40591a9ba3a93979UL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_dc8 = { 0xc0458e3d3fb254dbUL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_dc7 = { 0x402900c22cb40c15UL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_dc6 = { 0xc002eb6a9fc1820aUL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_dc5 = { 0x3fda8c7e93681dabUL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_dc4 = { 0xbf95453043a8bf6aUL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_dc3 = { 0x3fcdcc80066bd690UL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_dc2 = { 0xbee9e6381d15b179UL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_dc1 = { 0x3fec5bf8a8908326UL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_dc0 = { 0xbda14da505e70051UL };

// coefficients for erfinv(1-2^(-x*x))
static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_nc11 = { 0xbe81916980246624UL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_nc10 = { 0x3ed607a64a827a60UL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_nc9 = { 0xbf19297f8a2f6c59UL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_nc8 = { 0x3f5152e913d4b64cUL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_nc7 = { 0xbf8008e722b9f5f4UL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_nc6 = { 0x3fa50d4270b5e77cUL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_nc5 = { 0xbfc42188ef6e882bUL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_nc4 = { 0x3fdc47bb483a77e3UL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_nc3 = { 0xbfece3f2a959013bUL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_nc2 = { 0x3ff496898ae92338UL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_nc1 = { 0xbfd194293932af46UL };

static const union
{
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
} __serfinv_ha_nc0 = { 0x3fa9002affe368c5UL };

// sqrt(pi)/2
static const union
{
    uint32_t w;
    float f;
} __serfinv_ha_small_coeff = { 0x4f62DFC4u };

static const union
{
    uint32_t w;
    float f;
} __serfinv_ha_small_coeff_l = { 0x430DA77Bu };

// 2^(-32)
static const union
{
    uint32_t w;
    float f;
} __serfinv_ha_two_m32 = { 0x2f800000u };

// 2^(-13)
static const union
{
    uint32_t w;
    float f;
} __serfinv_ha_small_thres = { 0x39000000u };

inline int __devicelib_imf_internal_serfinv (const float *a, float *r)
{
    int nRet = 0;
    float xin = *a;
    union
    {
        uint32_t w;
        float f;
    } x, xa, mant, res, small_res;
    int32_t iexpon;
    uint32_t sgn_x;
    float R, poly, RS, ea;
    double dR, dRS, lpoly, expon, dS, dRS2, eps;
    union
    {
        uint32_t w;
        float f;
    } rcpf, fnpoly, fdpoly;
    union
    {
        uint64_t w;
        uint32_t w32[2];
        int32_t s32[2];
        double f;
    } npoly, dpoly, rcp;
    xa.f = xin;
    sgn_x = xa.w & 0x80000000u;
    // |xin|
    xa.w ^= sgn_x;
    // 1 - |x|
    x.f = 1.0f - xa.f;
    // normalize mantissa to [0.75, 1.5)
    // normalized, unbiased exponent
    iexpon = (x.w - 0x3f400000u) & 0xff800000u;
    // normalized mantissa
    mant.w = x.w - iexpon;
    // exponent
    iexpon >>= 23;
    // reduced argument
    R = mant.f - 1.0f;
    expon = (double) iexpon;
    // polynomial
    poly = __fma (__serfinv_ha_c10.f, R, __serfinv_ha_c9.f);
    poly = __fma (poly, R, __serfinv_ha_c8.f);
    poly = __fma (poly, R, __serfinv_ha_c7.f);
    poly = __fma (poly, R, __serfinv_ha_c6.f);
    poly = __fma (poly, R, __serfinv_ha_c5.f);
    poly = __fma (poly, R, __serfinv_ha_c4.f);
    poly = __fma (poly, R, __serfinv_ha_c3.f);
    poly = __fma (poly, R, __serfinv_ha_c2h.f);
    dR = (double) R;
    lpoly = (double) poly;
    lpoly = __fma (-(lpoly), dR, __serfinv_ha_ldc1.f);
    lpoly = __fma (lpoly, dR, -(expon));
    // dRS ~ 1.0/sqrt(-log2(1-xin))
    poly = (float) lpoly;
    RS = 1.0f / __sqrt (poly);
    dRS = (double) RS;
    // sqrt(-log2(1-xin))
    dRS2 = __fma (dRS, 0.5, 0.0);
    dS = __fma (lpoly, dRS, 0.0);
    eps = __fma (-(dS), dRS2, 0.5);
    // dRS ~ 1.0/sqrt(-log2(1-xin))
    dS = __fma (dS, eps, dS);
    dR = (double) xa.f;
    npoly.f = __fma (__serfinv_ha_nc11.f, dS, __serfinv_ha_nc10.f);
    dpoly.f = __fma (__serfinv_ha_dc13.f, dR, __serfinv_ha_dc12.f);
    npoly.f = __fma (npoly.f, dS, __serfinv_ha_nc9.f);
    dpoly.f = __fma (dpoly.f, dR, __serfinv_ha_dc11.f);
    npoly.f = __fma (npoly.f, dS, __serfinv_ha_nc8.f);
    dpoly.f = __fma (dpoly.f, dR, __serfinv_ha_dc10.f);
    npoly.f = __fma (npoly.f, dS, __serfinv_ha_nc7.f);
    dpoly.f = __fma (dpoly.f, dR, __serfinv_ha_dc9.f);
    npoly.f = __fma (npoly.f, dS, __serfinv_ha_nc6.f);
    dpoly.f = __fma (dpoly.f, dR, __serfinv_ha_dc8.f);
    npoly.f = __fma (npoly.f, dS, __serfinv_ha_nc5.f);
    dpoly.f = __fma (dpoly.f, dR, __serfinv_ha_dc7.f);
    npoly.f = __fma (npoly.f, dS, __serfinv_ha_nc4.f);
    dpoly.f = __fma (dpoly.f, dR, __serfinv_ha_dc6.f);
    npoly.f = __fma (npoly.f, dS, __serfinv_ha_nc3.f);
    dpoly.f = __fma (dpoly.f, dR, __serfinv_ha_dc5.f);
    npoly.f = __fma (npoly.f, dS, __serfinv_ha_nc2.f);
    dpoly.f = __fma (dpoly.f, dR, __serfinv_ha_dc4.f);
    npoly.f = __fma (npoly.f, dS, __serfinv_ha_nc1.f);
    dpoly.f = __fma (dpoly.f, dR, __serfinv_ha_dc3.f);
    npoly.f = __fma (npoly.f, dS, __serfinv_ha_nc0.f);
    dpoly.f = __fma (dpoly.f, dR, __serfinv_ha_dc2.f);
    dpoly.f = __fma (dpoly.f, dR, __serfinv_ha_dc1.f);
    dpoly.f = __fma (dpoly.f, dR, __serfinv_ha_dc0.f);
    dpoly.f = (dR < 0.75) ? dpoly.f : npoly.f;
    poly = (float) (dpoly.f);
    // fix result for |x|<2^(-13)
    small_res.f = __fma (xa.f, __serfinv_ha_small_coeff.f, __fma (xa.f, __serfinv_ha_small_coeff_l.f, 0.0f));
    small_res.f = __fma (small_res.f, __serfinv_ha_two_m32.f, 0.0f);
    res.f = (xa.f < __serfinv_ha_small_thres.f) ? small_res.f : poly;
    // fix special cases
    if (!(xa.f < 1.0f))
    {
        // rsqrt(1-xa), to cover special cases
        res.f = 1.0f / __sqrt (x.f);
        nRet = (xa.f == 1.0) ? 2 : 1;
    }
    res.w ^= sgn_x;
    *r = res.f;
    return nRet;
}
} // namespace
} // namespace __imf_impl_erfinv_s_ha

DEVICE_EXTERN_C_INLINE float __devicelib_imf_erfinvf (float a)
{
    float r;
    __imf_impl_erfinv_s_ha::__devicelib_imf_internal_serfinv (&a, &r);
    return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
