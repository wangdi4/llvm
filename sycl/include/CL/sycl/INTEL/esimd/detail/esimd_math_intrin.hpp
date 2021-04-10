//==------------ esimd_math_intrin.hpp - DPC++ Explicit SIMD API -----------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Declares Explicit SIMD math intrinsics used to implement working with
// the SIMD classes objects.
//===----------------------------------------------------------------------===//

#pragma once

#include <CL/sycl/INTEL/esimd/detail/esimd_host_util.hpp>
#include <CL/sycl/INTEL/esimd/detail/esimd_types.hpp>
#include <CL/sycl/INTEL/esimd/esimd_enum.hpp>
#include <cstdint>

#define __SIGD sycl::INTEL::gpu::detail

// saturation intrinsics
template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_satf(__SIGD::vector_type_t<T1, SZ> src);

template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_fptoui_sat(__SIGD::vector_type_t<T1, SZ> src);

template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_fptosi_sat(__SIGD::vector_type_t<T1, SZ> src);

template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_uutrunc_sat(__SIGD::vector_type_t<T1, SZ> src);

template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_ustrunc_sat(__SIGD::vector_type_t<T1, SZ> src);

template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_sutrunc_sat(__SIGD::vector_type_t<T1, SZ> src);

template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_sstrunc_sat(__SIGD::vector_type_t<T1, SZ> src);

template <typename T, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T, SZ>
__esimd_abs(__SIGD::vector_type_t<T, SZ> src0);

template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_ssshl(__SIGD::vector_type_t<T1, SZ> src0,
              __SIGD::vector_type_t<T1, SZ> src1);
template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_sushl(__SIGD::vector_type_t<T1, SZ> src0,
              __SIGD::vector_type_t<T1, SZ> src1);
template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_usshl(__SIGD::vector_type_t<T1, SZ> src0,
              __SIGD::vector_type_t<T1, SZ> src1);
template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_uushl(__SIGD::vector_type_t<T1, SZ> src0,
              __SIGD::vector_type_t<T1, SZ> src1);
template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_ssshl_sat(__SIGD::vector_type_t<T1, SZ> src0,
                  __SIGD::vector_type_t<T1, SZ> src1);
template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_sushl_sat(__SIGD::vector_type_t<T1, SZ> src0,
                  __SIGD::vector_type_t<T1, SZ> src1);
template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_usshl_sat(__SIGD::vector_type_t<T1, SZ> src0,
                  __SIGD::vector_type_t<T1, SZ> src1);
template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_uushl_sat(__SIGD::vector_type_t<T1, SZ> src0,
                  __SIGD::vector_type_t<T1, SZ> src1);

template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_rol(__SIGD::vector_type_t<T1, SZ> src0,
            __SIGD::vector_type_t<T1, SZ> src1);
template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_ror(__SIGD::vector_type_t<T1, SZ> src0,
            __SIGD::vector_type_t<T1, SZ> src1);

template <typename T, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T, SZ>
__esimd_umulh(__SIGD::vector_type_t<T, SZ> src0,
              __SIGD::vector_type_t<T, SZ> src1);
template <typename T, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T, SZ>
__esimd_smulh(__SIGD::vector_type_t<T, SZ> src0,
              __SIGD::vector_type_t<T, SZ> src1);

template <int SZ>
SYCL_EXTERNAL SYCL_EXTERNAL __SIGD::vector_type_t<float, SZ>
__esimd_frc(__SIGD::vector_type_t<float, SZ> src0);

/// 3 kinds of max
template <typename T, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T, SZ>
__esimd_fmax(__SIGD::vector_type_t<T, SZ> src0,
             __SIGD::vector_type_t<T, SZ> src1);
template <typename T, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T, SZ>
__esimd_umax(__SIGD::vector_type_t<T, SZ> src0,
             __SIGD::vector_type_t<T, SZ> src1);
template <typename T, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T, SZ>
__esimd_smax(__SIGD::vector_type_t<T, SZ> src0,
             __SIGD::vector_type_t<T, SZ> src1);

template <typename T, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T, SZ>
__esimd_lzd(__SIGD::vector_type_t<T, SZ> src0);

/// 3 kinds of min
template <typename T, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T, SZ>
__esimd_fmin(__SIGD::vector_type_t<T, SZ> src0,
             __SIGD::vector_type_t<T, SZ> src1);
template <typename T, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T, SZ>
__esimd_umin(__SIGD::vector_type_t<T, SZ> src0,
             __SIGD::vector_type_t<T, SZ> src1);
template <typename T, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T, SZ>
__esimd_smin(__SIGD::vector_type_t<T, SZ> src0,
             __SIGD::vector_type_t<T, SZ> src1);

template <typename T0, typename T1, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_bfrev(__SIGD::vector_type_t<T1, SZ> src0);

template <typename T, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<unsigned int, SZ>
__esimd_cbit(__SIGD::vector_type_t<T, SZ> src0);

template <typename T0, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ> __esimd_bfins(
    __SIGD::vector_type_t<T0, SZ> src0, __SIGD::vector_type_t<T0, SZ> src1,
    __SIGD::vector_type_t<T0, SZ> src2, __SIGD::vector_type_t<T0, SZ> src3);

template <typename T0, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<T0, SZ>
__esimd_bfext(__SIGD::vector_type_t<T0, SZ> src0,
              __SIGD::vector_type_t<T0, SZ> src1,
              __SIGD::vector_type_t<T0, SZ> src2);

template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<uint32_t, SZ>
__esimd_fbl(__SIGD::vector_type_t<uint32_t, SZ> src0);

template <typename T0, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<int, SZ>
__esimd_sfbh(__SIGD::vector_type_t<T0, SZ> src0);

template <typename T0, int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<uint32_t, SZ>
__esimd_ufbh(__SIGD::vector_type_t<T0, SZ> src0);

template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<float, SZ>
__esimd_inv(__SIGD::vector_type_t<float, SZ> src0);
template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<float, SZ>
__esimd_log(__SIGD::vector_type_t<float, SZ> src0);
template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<float, SZ>
__esimd_exp(__SIGD::vector_type_t<float, SZ> src0);
template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<float, SZ>
__esimd_sqrt(__SIGD::vector_type_t<float, SZ> src0);
template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<float, SZ>
__esimd_sqrt_ieee(__SIGD::vector_type_t<float, SZ> src0);
template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<float, SZ>
__esimd_rsqrt(__SIGD::vector_type_t<float, SZ> src0);
template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<float, SZ>
__esimd_sin(__SIGD::vector_type_t<float, SZ> src0);
template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<float, SZ>
__esimd_cos(__SIGD::vector_type_t<float, SZ> src0);
template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<float, SZ>
__esimd_pow(__SIGD::vector_type_t<float, SZ> src0,
            __SIGD::vector_type_t<float, SZ> src1);
template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<float, SZ>
__esimd_div_ieee(__SIGD::vector_type_t<float, SZ> src0,
                 __SIGD::vector_type_t<float, SZ> src1);

template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<float, SZ>
__esimd_rndd(__SIGD::vector_type_t<float, SZ> src0);
template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<float, SZ>
__esimd_rndu(__SIGD::vector_type_t<float, SZ> src0);
template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<float, SZ>
__esimd_rnde(__SIGD::vector_type_t<float, SZ> src0);
template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<float, SZ>
__esimd_rndz(__SIGD::vector_type_t<float, SZ> src0);

template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<double, SZ>
__esimd_sqrt_ieee(__SIGD::vector_type_t<double, SZ> src0);
template <int SZ>
SYCL_EXTERNAL __SIGD::vector_type_t<double, SZ>
__esimd_div_ieee(__SIGD::vector_type_t<double, SZ> src0,
                 __SIGD::vector_type_t<double, SZ> src1);

template <int N>
SYCL_EXTERNAL uint32_t
__esimd_pack_mask(__SIGD::vector_type_t<uint16_t, N> src0);

template <int N>
SYCL_EXTERNAL __SIGD::vector_type_t<uint16_t, N>
__esimd_unpack_mask(uint32_t src0);

template <typename T1, typename T2, typename T3, typename T4, int N>
SYCL_EXTERNAL __SIGD::vector_type_t<T1, N>
__esimd_uudp4a(__SIGD::vector_type_t<T2, N> src0,
               __SIGD::vector_type_t<T3, N> src1,
               __SIGD::vector_type_t<T4, N> src2);

template <typename T1, typename T2, typename T3, typename T4, int N>
SYCL_EXTERNAL __SIGD::vector_type_t<T1, N>
__esimd_usdp4a(__SIGD::vector_type_t<T2, N> src0,
               __SIGD::vector_type_t<T3, N> src1,
               __SIGD::vector_type_t<T4, N> src2);

template <typename T1, typename T2, typename T3, typename T4, int N>
SYCL_EXTERNAL __SIGD::vector_type_t<T1, N>
__esimd_sudp4a(__SIGD::vector_type_t<T2, N> src0,
               __SIGD::vector_type_t<T3, N> src1,
               __SIGD::vector_type_t<T4, N> src2);

template <typename T1, typename T2, typename T3, typename T4, int N>
SYCL_EXTERNAL __SIGD::vector_type_t<T1, N>
__esimd_ssdp4a(__SIGD::vector_type_t<T2, N> src0,
               __SIGD::vector_type_t<T3, N> src1,
               __SIGD::vector_type_t<T4, N> src2);

template <typename T1, typename T2, typename T3, typename T4, int N>
SYCL_EXTERNAL __SIGD::vector_type_t<T1, N>
__esimd_uudp4a_sat(__SIGD::vector_type_t<T2, N> src0,
                   __SIGD::vector_type_t<T3, N> src1,
                   __SIGD::vector_type_t<T4, N> src2);

template <typename T1, typename T2, typename T3, typename T4, int N>
SYCL_EXTERNAL __SIGD::vector_type_t<T1, N>
__esimd_usdp4a_sat(__SIGD::vector_type_t<T2, N> src0,
                   __SIGD::vector_type_t<T3, N> src1,
                   __SIGD::vector_type_t<T4, N> src2);

template <typename T1, typename T2, typename T3, typename T4, int N>
SYCL_EXTERNAL __SIGD::vector_type_t<T1, N>
__esimd_sudp4a_sat(__SIGD::vector_type_t<T2, N> src0,
                   __SIGD::vector_type_t<T3, N> src1,
                   __SIGD::vector_type_t<T4, N> src2);

template <typename T1, typename T2, typename T3, typename T4, int N>
SYCL_EXTERNAL __SIGD::vector_type_t<T1, N>
__esimd_ssdp4a_sat(__SIGD::vector_type_t<T2, N> src0,
                   __SIGD::vector_type_t<T3, N> src1,
                   __SIGD::vector_type_t<T4, N> src2);

// Reduction functions
template <typename Ty, int N>
SYCL_EXTERNAL __SIGD::vector_type_t<Ty, N>
__esimd_reduced_fmax(__SIGD::vector_type_t<Ty, N> src1,
                     __SIGD::vector_type_t<Ty, N> src2);

template <typename Ty, int N>
SYCL_EXTERNAL __SIGD::vector_type_t<Ty, N>
__esimd_reduced_umax(__SIGD::vector_type_t<Ty, N> src1,
                     __SIGD::vector_type_t<Ty, N> src2);

template <typename Ty, int N>
SYCL_EXTERNAL __SIGD::vector_type_t<Ty, N>
__esimd_reduced_smax(__SIGD::vector_type_t<Ty, N> src1,
                     __SIGD::vector_type_t<Ty, N> src2);

template <typename Ty, int N>
SYCL_EXTERNAL __SIGD::vector_type_t<Ty, N>
__esimd_reduced_fmin(__SIGD::vector_type_t<Ty, N> src1,
                     __SIGD::vector_type_t<Ty, N> src2);

template <typename Ty, int N>
SYCL_EXTERNAL __SIGD::vector_type_t<Ty, N>
__esimd_reduced_umin(__SIGD::vector_type_t<Ty, N> src1,
                     __SIGD::vector_type_t<Ty, N> src2);

template <typename Ty, int N>
__SIGD::vector_type_t<Ty, N>
    SYCL_EXTERNAL __esimd_reduced_smin(__SIGD::vector_type_t<Ty, N> src1,
                                       __SIGD::vector_type_t<Ty, N> src2);

template <typename Ty, int N>
SYCL_EXTERNAL __SIGD::vector_type_t<Ty, N>
__esimd_dp4(__SIGD::vector_type_t<Ty, N> v1, __SIGD::vector_type_t<Ty, N> v2);

/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ESIMD_EMBARGO */

template <typename T, typename T1, typename T2, int N, int N1, int N2>
SYCL_EXTERNAL sycl::INTEL::gpu::vector_type_t<T, N>
__esimd_dpas(sycl::INTEL::gpu::vector_type_t<T, N> src0,
             sycl::INTEL::gpu::vector_type_t<T1, N1> src1,
             sycl::INTEL::gpu::vector_type_t<T2, N2> src2, int dpas_info);

template <typename T, typename T1, typename T2, int N, int N1, int N2>
SYCL_EXTERNAL sycl::INTEL::gpu::vector_type_t<T, N>
__esimd_dpas2(sycl::INTEL::gpu::vector_type_t<T1, N1> src1,
              sycl::INTEL::gpu::vector_type_t<T2, N2> src2, int dpas_info);

template <typename T, typename T1, typename T2, int N, int N1, int N2>
SYCL_EXTERNAL sycl::INTEL::gpu::vector_type_t<T, N>
__esimd_dpasw(sycl::INTEL::gpu::vector_type_t<T, N> src0,
              sycl::INTEL::gpu::vector_type_t<T1, N1> src1,
              sycl::INTEL::gpu::vector_type_t<T2, N2> src2, int dpas_info);

template <typename T, typename T1, typename T2, int N, int N1, int N2>
SYCL_EXTERNAL sycl::INTEL::gpu::vector_type_t<T, N>
__esimd_dpasw2(sycl::INTEL::gpu::vector_type_t<T1, N1> src1,
               sycl::INTEL::gpu::vector_type_t<T2, N2> src2, int dpas_info);

/* end INTEL_FEATURE_ESIMD_EMBARGO */
/* end INTEL_CUSTOMIZATION */

#ifndef __SYCL_DEVICE_ONLY__

template <typename T>
inline T extract(const uint32_t &width, const uint32_t &offset, uint32_t src,
                 const uint32_t &sign_extend) {
  uint32_t mask = ((1 << width) - 1) << offset;
  T ret = (src & mask) >> offset;
  if (sign_extend) {
    if ((src >> (offset + width - 1)) & 0x1) {
      uint32_t sign_extend = ((1 << (32 - width)) - 1) << width;
      ret = ret | sign_extend;
    }
  }

  return ret;
}

#define __SIGED sycl::INTEL::gpu::emu::detail

template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_satf(__SIGD::vector_type_t<T1, SZ> src) {
  __SIGD::vector_type_t<T0, SZ> retv;
  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = __SIGED::satur<T0>::saturate(src[i], 1);
  }
  return retv;
};

template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_fptoui_sat(__SIGD::vector_type_t<T1, SZ> src) {
  __SIGD::vector_type_t<T0, SZ> retv;
  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = __SIGED::satur<T0>::saturate(src[i], 1);
  }
  return retv;
};

template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_fptosi_sat(__SIGD::vector_type_t<T1, SZ> src) {
  __SIGD::vector_type_t<T0, SZ> retv;
  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = __SIGED::satur<T0>::saturate(src[i], 1);
  }
  return retv;
};

template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_uutrunc_sat(__SIGD::vector_type_t<T1, SZ> src) {
  __SIGD::vector_type_t<T0, SZ> retv;
  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = __SIGED::satur<T0>::saturate(src[i], 1);
  }
  return retv;
};

template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_ustrunc_sat(__SIGD::vector_type_t<T1, SZ> src) {
  __SIGD::vector_type_t<T0, SZ> retv;
  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = __SIGED::satur<T0>::saturate(src[i], 1);
  }
  return retv;
};

template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_sutrunc_sat(__SIGD::vector_type_t<T1, SZ> src) {
  __SIGD::vector_type_t<T0, SZ> retv;
  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = __SIGED::satur<T0>::saturate(src[i], 1);
  }
  return retv;
};

template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_sstrunc_sat(__SIGD::vector_type_t<T1, SZ> src) {
  __SIGD::vector_type_t<T0, SZ> retv;
  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = __SIGED::satur<T0>::saturate(src[i], 1);
  }
  return retv;
};

template <typename T, int SZ>
inline __SIGD::vector_type_t<T, SZ>
__esimd_abs(__SIGD::vector_type_t<T, SZ> src0) {
  int i;
  typename __SIGED::abstype<T>::type ret;
  __SIGD::vector_type_t<T, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    if (src0[i] < 0) {
      ret = -(src0[i]);
    } else {
      ret = (src0[i]);
    }
    retv[i] = ret;
  }
  return retv;
};

template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_ssshl(__SIGD::vector_type_t<T1, SZ> src0,
              __SIGD::vector_type_t<T1, SZ> src1) {
  int i;
  typename __SIGED::maxtype<T1>::type ret;
  __SIGD::vector_type_t<T0, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    ret = src0.get(i) << src1.get(i);
    retv[i] = ret;
  }
  return retv;
};
template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_sushl(__SIGD::vector_type_t<T1, SZ> src0,
              __SIGD::vector_type_t<T1, SZ> src1) {
  int i;
  typename __SIGED::maxtype<T1>::type ret;
  __SIGD::vector_type_t<T0, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    ret = src0.get(i) << src1.get(i);
    retv[i] = ret;
  }
  return retv;
};
template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_usshl(__SIGD::vector_type_t<T1, SZ> src0,
              __SIGD::vector_type_t<T1, SZ> src1) {
  int i;
  typename __SIGED::maxtype<T1>::type ret;
  __SIGD::vector_type_t<T0, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    ret = src0.get(i) << src1.get(i);
    retv[i] = ret;
  }
  return retv;
};
template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_uushl(__SIGD::vector_type_t<T1, SZ> src0,
              __SIGD::vector_type_t<T1, SZ> src1) {
  int i;
  typename __SIGED::maxtype<T1>::type ret;
  __SIGD::vector_type_t<T0, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    ret = src0.get(i) << src1.get(i);
    retv[i] = ret;
  }
  return retv;
};
template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_ssshl_sat(__SIGD::vector_type_t<T1, SZ> src0,
                  __SIGD::vector_type_t<T1, SZ> src1) {
  int i;
  typename __SIGED::maxtype<T1>::type ret;
  __SIGD::vector_type_t<T0, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    ret = src0.get(i) << src1.get(i);
    retv[i] = __SIGED::satur<T0>::saturate(ret, 1);
  }
  return retv;
};
template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_sushl_sat(__SIGD::vector_type_t<T1, SZ> src0,
                  __SIGD::vector_type_t<T1, SZ> src1) {
  int i;
  typename __SIGED::maxtype<T1>::type ret;
  __SIGD::vector_type_t<T0, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    ret = src0.get(i) << src1.get(i);
    retv[i] = __SIGED::satur<T0>::saturate(ret, 1);
  }
  return retv;
};
template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_usshl_sat(__SIGD::vector_type_t<T1, SZ> src0,
                  __SIGD::vector_type_t<T1, SZ> src1) {
  int i;
  typename __SIGED::maxtype<T1>::type ret;
  __SIGD::vector_type_t<T0, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    ret = src0.get(i) << src1.get(i);
    retv[i] = __SIGED::satur<T0>::saturate(ret, 1);
  }
  return retv;
};
template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_uushl_sat(__SIGD::vector_type_t<T1, SZ> src0,
                  __SIGD::vector_type_t<T1, SZ> src1) {
  int i;
  typename __SIGED::maxtype<T1>::type ret;
  __SIGD::vector_type_t<T0, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    ret = src0.get(i) << src1.get(i);
    retv[i] = __SIGED::satur<T0>::saturate(ret, 1);
  }
  return retv;
};

template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_rol(__SIGD::vector_type_t<T1, SZ> src0,
            __SIGD::vector_type_t<T1, SZ> src1){};

template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_ror(__SIGD::vector_type_t<T1, SZ> src0,
            __SIGD::vector_type_t<T1, SZ> src1){};

template <typename T, int SZ>
inline __SIGD::vector_type_t<T, SZ>
__esimd_umulh(__SIGD::vector_type_t<T, SZ> src0,
              __SIGD::vector_type_t<T, SZ> src1) {
  int i;
  __SIGD::vector_type_t<T, SZ> retv;

  for (i = 0; i < SZ; i++) {
    unsigned long long temp;
    SIMDCF_ELEMENT_SKIP(i);
    temp = (long long)src0[i] * (long long)src1[i];
    retv[i] = temp >> 32;
  }
  return retv;
}

template <typename T, int SZ>
inline __SIGD::vector_type_t<T, SZ>
__esimd_smulh(__SIGD::vector_type_t<T, SZ> src0,
              __SIGD::vector_type_t<T, SZ> src1) {
  int i;
  __SIGD::vector_type_t<T, SZ> retv;

  for (i = 0; i < SZ; i++) {
    long long temp;
    SIMDCF_ELEMENT_SKIP(i);
    temp = (long long)src0[i] * (long long)src1[i];
    retv[i] = temp >> 32;
  }
  return retv;
};

template <int SZ>
inline __SIGD::vector_type_t<float, SZ>
__esimd_frc(__SIGD::vector_type_t<float, SZ> src0) {
  __SIGD::vector_type_t<float, SZ> retv;
  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = src0[i] - floor(src0[i]);
  }
  return retv;
};

/// 3 kinds of max
template <typename T, int SZ>
inline __SIGD::vector_type_t<T, SZ>
__esimd_fmax(__SIGD::vector_type_t<T, SZ> src0,
             __SIGD::vector_type_t<T, SZ> src1) {
  int i;
  __SIGD::vector_type_t<T, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    if (src0[i] >= src1[i]) {
      retv[i] = src0[i];
    } else {
      retv[i] = src1[i];
    }
  }

  return retv;
};
template <typename T, int SZ>
inline __SIGD::vector_type_t<T, SZ>
__esimd_umax(__SIGD::vector_type_t<T, SZ> src0,
             __SIGD::vector_type_t<T, SZ> src1) {
  int i;
  __SIGD::vector_type_t<T, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    if (src0[i] >= src1[i]) {
      retv[i] = src0[i];
    } else {
      retv[i] = src1[i];
    }
  }

  return retv;
};
template <typename T, int SZ>
inline __SIGD::vector_type_t<T, SZ>
__esimd_smax(__SIGD::vector_type_t<T, SZ> src0,
             __SIGD::vector_type_t<T, SZ> src1) {
  int i;
  __SIGD::vector_type_t<T, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    if (src0[i] >= src1[i]) {
      retv[i] = src0[i];
    } else {
      retv[i] = src1[i];
    }
  }

  return retv;
};

template <typename T, int SZ>
inline __SIGD::vector_type_t<T, SZ>
__esimd_lzd(__SIGD::vector_type_t<T, SZ> src0) {
  int i;
  T ret;
  __SIGD::vector_type_t<T, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    ret = src0[i];
    uint32_t cnt = 0;
    while ((ret & 1u << 31u) == 0 && cnt != 32) {
      cnt++;
      ret = ret << 1;
    }
    retv[i] = cnt;
  }

  return retv;
};

/// 3 kinds of min
template <typename T, int SZ>
inline __SIGD::vector_type_t<T, SZ>
__esimd_fmin(__SIGD::vector_type_t<T, SZ> src0,
             __SIGD::vector_type_t<T, SZ> src1) {
  int i;
  __SIGD::vector_type_t<T, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    if (src0[i] < src1[i]) {
      retv[i] = src0[i];
    } else {
      retv[i] = src1[i];
    }
  }

  return retv;
};

template <typename T, int SZ>
inline __SIGD::vector_type_t<T, SZ>
__esimd_umin(__SIGD::vector_type_t<T, SZ> src0,
             __SIGD::vector_type_t<T, SZ> src1) {
  int i;
  __SIGD::vector_type_t<T, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    if (src0[i] < src1[i]) {
      retv[i] = src0[i];
    } else {
      retv[i] = src1[i];
    }
  }

  return retv;
};

template <typename T, int SZ>
inline __SIGD::vector_type_t<T, SZ>
__esimd_smin(__SIGD::vector_type_t<T, SZ> src0,
             __SIGD::vector_type_t<T, SZ> src1) {
  int i;
  __SIGD::vector_type_t<T, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    if (src0[i] < src1[i]) {
      retv[i] = src0[i];
    } else {
      retv[i] = src1[i];
    }
  }

  return retv;
};

template <typename T0, typename T1, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_bfrev(__SIGD::vector_type_t<T1, SZ> src0) {
  int i, j;
  __SIGD::vector_type_t<T0, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    T0 input = src0[i];
    T0 output = 0;
    for (j = 0; j < sizeof(T0) * 8; j++) {
      output |= input & 0x1;

      // Don't shift if this was the last one
      if ((j + 1) < (sizeof(T0) * 8)) {
        output <<= 1;
        input >>= 1;
      }
    }
    retv[i] = output;
  }

  return retv;
};

template <typename T, int SZ>
inline __SIGD::vector_type_t<unsigned int, SZ>
__esimd_cbit(__SIGD::vector_type_t<T, SZ> src0) {
  int i;
  uint32_t ret;
  __SIGD::vector_type_t<uint32_t, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    ret = src0[i];
    uint32_t cnt = 0;
    for (int j = 0; j < sizeof(T) * 8; j++) {
      if ((ret & 1u) == 1) {
        cnt++;
      }
      ret = ret >> 1;
    }
    retv[i] = cnt;
  }

  return retv;
};

template <typename T0, int SZ>
inline __SIGD::vector_type_t<T0, SZ> __esimd_bfins(
    __SIGD::vector_type_t<T0, SZ> width, __SIGD::vector_type_t<T0, SZ> offset,
    __SIGD::vector_type_t<T0, SZ> val, __SIGD::vector_type_t<T0, SZ> src) {
  int i;
  typename __SIGED::maxtype<T0>::type ret;
  __SIGD::vector_type_t<T0, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    const uint32_t mask = ((1 << width[i]) - 1) << offset[i];
    const uint32_t imask = ~mask;
    ret = (src[i] & imask) | ((val[i] << offset[i] & mask));
    // Sign extend if signed type
    if constexpr (std::is_signed<T0>::value) {
      int m = 1U << (width[i] - 1);
      ret = (ret ^ m) - m;
    }
    retv[i] = ret;
  }

  return retv;
};

template <typename T0, int SZ>
inline __SIGD::vector_type_t<T0, SZ>
__esimd_bfext(__SIGD::vector_type_t<T0, SZ> width,
              __SIGD::vector_type_t<T0, SZ> offset,
              __SIGD::vector_type_t<T0, SZ> src) {
  int i;
  typename __SIGED::maxtype<T0>::type ret;
  __SIGD::vector_type_t<T0, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    const uint32_t mask = ((1 << width[i]) - 1) << offset[i];
    ret = (src[i] & mask) >> offset[i];
    retv[i] = ret;
  }

  return retv;
};

template <int SZ>
inline __SIGD::vector_type_t<uint32_t, SZ>
__esimd_fbl(__SIGD::vector_type_t<uint32_t, SZ> src0) {
  int i;
  uint32_t ret;
  __SIGD::vector_type_t<uint32_t, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    ret = src0[i];
    uint32_t cnt = 0;
    while ((ret & 1u) == 0 && cnt != 32) {
      cnt++;
      ret = ret >> 1;
    }
    if (src0[i] == 0x0) {
      retv[i] = 0xFFFFFFFF;
    } else {
      retv[i] = cnt;
    }
  }

  return retv;
};

template <typename T0, int SZ>
inline __SIGD::vector_type_t<int, SZ>
__esimd_sfbh(__SIGD::vector_type_t<T0, SZ> src0) {

  int i, cval;
  int ret;
  __SIGD::vector_type_t<int, SZ> retv;

  for (i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    ret = src0[i];
    uint32_t cnt = 0;
    if (((ret >> 31u) & 1u) == 1) {
      cval = 1;
    } else {
      cval = 0;
    }
    while (((ret >> 31u) & 1u) == cval && cnt != 32) {
      cnt++;
      ret = ret << 1;
    }

    if ((src0[i] == 0xFFFFFFFF) || (src0[i] == 0x00000000)) {
      retv[i] = 0xFFFFFFFF;
    } else {
      retv[i] = cnt;
    }
  }

  return retv;
};

template <typename T0, int SZ>
inline __SIGD::vector_type_t<uint32_t, SZ>
__esimd_ufbh(__SIGD::vector_type_t<T0, SZ> src0) {
  uint32_t ret;
  __SIGD::vector_type_t<uint32_t, SZ> retv;

  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    ret = src0[i];
    uint32_t cnt = 0;
    while ((ret & (1u << 31u)) == 0 && cnt != 32) {
      cnt++;
      ret = ret << 1;
    }
    if (src0[i] == 0x00000000) {
      retv[i] = 0xFFFFFFFF;
    } else {
      retv[i] = cnt;
    }
  }

  return retv;
};

template <int SZ>
inline __SIGD::vector_type_t<float, SZ>
__esimd_inv(__SIGD::vector_type_t<float, SZ> src0) {
  __SIGD::vector_type_t<float, SZ> retv;

  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = 1.f / src0[i];
  }
  return retv;
};

template <int SZ>
inline __SIGD::vector_type_t<float, SZ>
__esimd_log(__SIGD::vector_type_t<float, SZ> src0) {
  __SIGD::vector_type_t<float, SZ> retv;

  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = logf(src0[i]) / logf(2.);
  }
  return retv;
};
template <int SZ>
inline __SIGD::vector_type_t<float, SZ>
__esimd_exp(__SIGD::vector_type_t<float, SZ> src0) {
  __SIGD::vector_type_t<float, SZ> retv;

  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = powf(2.f, src0[i]);
  }
  return retv;
};
template <int SZ>
inline __SIGD::vector_type_t<float, SZ>
__esimd_sqrt(__SIGD::vector_type_t<float, SZ> src0) {
  __SIGD::vector_type_t<float, SZ> retv;

  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = sqrt(src0[i]);
  }
  return retv;
};
template <int SZ>
inline __SIGD::vector_type_t<float, SZ>
__esimd_sqrt_ieee(__SIGD::vector_type_t<float, SZ> src0) {
  __SIGD::vector_type_t<float, SZ> retv;

  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = sqrt(src0[i]);
  }
  return retv;
};
template <int SZ>
inline __SIGD::vector_type_t<float, SZ>
__esimd_rsqrt(__SIGD::vector_type_t<float, SZ> src0) {
  __SIGD::vector_type_t<float, SZ> retv;

  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = 1.f / sqrt(src0[i]);
  }
  return retv;
};
template <int SZ>
inline __SIGD::vector_type_t<float, SZ>
__esimd_sin(__SIGD::vector_type_t<float, SZ> src) {
  __SIGD::vector_type_t<float, SZ> retv;
  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = sin(src[i]);
  }
  return retv;
};
template <int SZ>
inline __SIGD::vector_type_t<float, SZ>
__esimd_cos(__SIGD::vector_type_t<float, SZ> src) {
  __SIGD::vector_type_t<float, SZ> retv;
  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = cos(src[i]);
  }
  return retv;
};
template <int SZ>
inline __SIGD::vector_type_t<float, SZ>
__esimd_pow(__SIGD::vector_type_t<float, SZ> src0,
            __SIGD::vector_type_t<float, SZ> src1) {
  __SIGD::vector_type_t<float, SZ> retv;

  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = powf(fabs(src0[i]), src1[i]);
  }
  return retv;
};

template <int SZ>
inline __SIGD::vector_type_t<float, SZ>
__esimd_div_ieee(__SIGD::vector_type_t<float, SZ> src0,
                 __SIGD::vector_type_t<float, SZ> src1) {
  __SIGD::vector_type_t<float, SZ> divinv;
  __SIGD::vector_type_t<float, SZ> retv;

  for (int idx = 0; idx < SZ; idx += 1) {
    SIMDCF_ELEMENT_SKIP(idx);
    if (src1[idx] == 0.0f) {
      /// Handle Divide-by-zero
      retv[idx] = (src0[idx] < 0) ? (-INFINITY) : INFINITY;
    } else {
      retv[idx] = src0[idx] / src1[idx];
    }
  }

  return retv;
};

template <int SZ>
inline __SIGD::vector_type_t<float, SZ>
__esimd_rndd(__SIGD::vector_type_t<float, SZ> src0) {
  __SIGD::vector_type_t<float, SZ> retv;

  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = floor(src0[i]);
  }
  return retv;
};

template <int SZ>
inline __SIGD::vector_type_t<float, SZ>
__esimd_rndu(__SIGD::vector_type_t<float, SZ> src0) {
  __SIGD::vector_type_t<float, SZ> retv;
  int increment;

  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    if (src0.get(i) - floor(src0.get(i)) > 0.0f) {
      increment = 1;
    } else {
      increment = 0;
    }

    retv[i] = floor(src0[i]) + increment;
  }

  return retv;
};

template <int SZ>
inline __SIGD::vector_type_t<float, SZ>
__esimd_rnde(__SIGD::vector_type_t<float, SZ> src0) {
  __SIGD::vector_type_t<float, SZ> retv;
  int increment;

  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    if (src0[i] - floor(src0[i]) > 0.5f) {
      increment = 1;
    } else if (src0[i] - floor(src0[i]) < 0.5f) {
      increment = 0;
    } else {
      increment = (int(floor(src0[i])) % 2 == 1);
    }

    retv[i] = floor(src0[i]) + increment;
  }

  return retv;
};

template <int SZ>
inline __SIGD::vector_type_t<float, SZ>
__esimd_rndz(__SIGD::vector_type_t<float, SZ> src0) {
  __SIGD::vector_type_t<float, SZ> retv;
  int increment;

  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    if (fabs(src0[i]) < fabs(floor(src0[i]))) {
      increment = 1;
    } else {
      increment = 0;
    }
    retv[i] = floor(src0[i]) + increment;
  }

  return retv;
};

template <int SZ>
inline __SIGD::vector_type_t<double, SZ>
__esimd_sqrt_ieee(__SIGD::vector_type_t<double, SZ> src0) {
  __SIGD::vector_type_t<double, SZ> retv;

  for (int i = 0; i < SZ; i++) {
    SIMDCF_ELEMENT_SKIP(i);
    retv[i] = sqrt(src0[i]);
  }
  return retv;
};

template <int SZ>
inline __SIGD::vector_type_t<double, SZ>
__esimd_div_ieee(__SIGD::vector_type_t<double, SZ> src0,
                 __SIGD::vector_type_t<double, SZ> src1) {
  __SIGD::vector_type_t<double, SZ> divinv;
  __SIGD::vector_type_t<double, SZ> retv;

  for (int idx = 0; idx < SZ; idx += 1) {
    SIMDCF_ELEMENT_SKIP(idx);
    if (src1[idx] == 0.0f) {
      /// Handle Divide-by-zero
      retv[idx] = (src0[idx] < 0) ? (-INFINITY) : INFINITY;
    } else {
      retv[idx] = src0[idx] / src1[idx];
    }
  }

  return retv;
};

template <int N>
inline uint32_t __esimd_pack_mask(__SIGD::vector_type_t<uint16_t, N> src0) {
  // We don't check the arguments here as this function is only invoked by
  // wrapper code (which does the checks already)
  uint32_t retv = 0;
  for (int i = 0; i < N; i++) {
    if (src0[i] & 0x1) {
      retv |= 0x1 << i;
    }
  }

  return retv;
};

template <int N>
inline __SIGD::vector_type_t<uint16_t, N> __esimd_unpack_mask(uint32_t src0) {
  __SIGD::vector_type_t<uint16_t, N> retv;
  for (int i = 0; i < N; i++) {
    if ((src0 >> i) & 0x1) {
      retv[i] = 1;
    } else {
      retv[i] = 0;
    }
  }
  return retv;
};

template <typename T1, typename T2, typename T3, typename T4, int N>
inline __SIGD::vector_type_t<T1, N>
__esimd_dp4a(__SIGD::vector_type_t<T2, N> src0,
             __SIGD::vector_type_t<T3, N> src1,
             __SIGD::vector_type_t<T4, N> src2) {
  using sycl::INTEL::gpu::emu::detail::restype_ex;
  typename restype_ex<T2, typename restype_ex<T3, T4>::type>::type reta;
  __SIGD::vector_type_t<T1, N> retv;

  int src1_a, src1_b, src1_c, src1_d, src2_a, src2_b, src2_c, src2_d, ret;

  uint32_t sat1 =
      __SIGED::SetSatur<T2, __SIGED::is_inttype<T1>::value>::set() ||
      __SIGED::SetSatur<T3, __SIGED::is_inttype<T1>::value>::set() ||
      __SIGED::SetSatur<T4, __SIGED::is_inttype<T1>::value>::set();

  for (uint32_t i = 0; i < N; i++) {

    SIMDCF_ELEMENT_SKIP(i);

    src1_a = extract<short>(8, 0, src1[i], 0);
    src1_b = extract<short>(8, 8, src1[i], 0);
    src1_c = extract<short>(8, 16, src1[i], 0);
    src1_d = extract<short>(8, 24, src1[i], 0);
    src2_a = extract<short>(8, 0, src2[i], 0);
    src2_b = extract<short>(8, 8, src2[i], 0);
    src2_c = extract<short>(8, 16, src2[i], 0);
    src2_d = extract<short>(8, 24, src2[i], 0);

    ret = src1_a * src2_a + src1_b * src2_b + src1_c * src2_c + src1_d * src2_d;
    reta = ret + src0[i];
    retv[i] = __SIGED::satur<T1>::saturate(reta, sat1);
  }

  return retv;
};

template <typename Ty, int N>
inline __SIGD::vector_type_t<Ty, N>
__esimd_reduced_max(__SIGD::vector_type_t<Ty, N> src1,
                    __SIGD::vector_type_t<Ty, N> src2) {
  __SIGD::vector_type_t<Ty, N> retv;
  for (int I = 0; I < N; I++) {
    if (src1[I] >= src2[I]) {
      retv[I] = src1[I];
    } else {
      retv[I] = src2[I];
    }
  }
  return retv;
}

template <typename Ty, int N>
inline __SIGD::vector_type_t<Ty, N>
__esimd_reduced_fmax(__SIGD::vector_type_t<Ty, N> src1,
                     __SIGD::vector_type_t<Ty, N> src2) {
  return __esimd_reduced_max<Ty, N>(src1, src2);
}

template <typename Ty, int N>
inline __SIGD::vector_type_t<Ty, N>
__esimd_reduced_umax(__SIGD::vector_type_t<Ty, N> src1,
                     __SIGD::vector_type_t<Ty, N> src2) {
  return __esimd_reduced_max<Ty, N>(src1, src2);
}

template <typename Ty, int N>
inline __SIGD::vector_type_t<Ty, N>
__esimd_reduced_smax(__SIGD::vector_type_t<Ty, N> src1,
                     __SIGD::vector_type_t<Ty, N> src2) {
  return __esimd_reduced_max<Ty, N>(src1, src2);
}

template <typename Ty, int N>
inline __SIGD::vector_type_t<Ty, N>
__esimd_reduced_min(__SIGD::vector_type_t<Ty, N> src1,
                    __SIGD::vector_type_t<Ty, N> src2) {
  __SIGD::vector_type_t<Ty, N> retv;
  for (int I = 0; I < N; I++) {
    if (src1[I] <= src2[I]) {
      retv[I] = src1[I];
    } else {
      retv[I] = src2[I];
    }
  }
  return retv;
}

template <typename Ty, int N>
inline __SIGD::vector_type_t<Ty, N>
__esimd_reduced_fmin(__SIGD::vector_type_t<Ty, N> src1,
                     __SIGD::vector_type_t<Ty, N> src2) {
  return __esimd_reduced_min<Ty, N>(src1, src2);
}

template <typename Ty, int N>
inline __SIGD::vector_type_t<Ty, N>
__esimd_reduced_umin(__SIGD::vector_type_t<Ty, N> src1,
                     __SIGD::vector_type_t<Ty, N> src2) {
  return __esimd_reduced_min<Ty, N>(src1, src2);
}

template <typename Ty, int N>
inline __SIGD::vector_type_t<Ty, N>
__esimd_reduced_smin(__SIGD::vector_type_t<Ty, N> src1,
                     __SIGD::vector_type_t<Ty, N> src2) {
  return __esimd_reduced_min<Ty, N>(src1, src2);
}

/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ESIMD_EMBARGO */

inline constexpr sycl::INTEL::gpu::uint __esimd_dpas_bits_precision(
    sycl::INTEL::gpu::EsimdPrecisionType precisionType) {
  return precisionType == sycl::INTEL::gpu::EsimdPrecisionType::BF16 ||
                 precisionType == sycl::INTEL::gpu::EsimdPrecisionType::FP16
             ? 16
         : precisionType == sycl::INTEL::gpu::EsimdPrecisionType::S8 ||
                 precisionType == sycl::INTEL::gpu::EsimdPrecisionType::U8
             ? 8
         : precisionType == sycl::INTEL::gpu::EsimdPrecisionType::S4 ||
                 precisionType == sycl::INTEL::gpu::EsimdPrecisionType::U4
             ? 4
         : precisionType == sycl::INTEL::gpu::EsimdPrecisionType::S2 ||
                 precisionType == sycl::INTEL::gpu::EsimdPrecisionType::U2
             ? 2
             : 1;
}

template <sycl::INTEL::gpu::EsimdPrecisionType src1_precision,
          sycl::INTEL::gpu::EsimdPrecisionType src2_precision,
          int systolic_depth, int repeat_count, typename RT, typename T1,
          typename T2, sycl::INTEL::gpu::uint SZ, sycl::INTEL::gpu::uint N1,
          sycl::INTEL::gpu::uint N2>
inline sycl::INTEL::gpu::vector_type_t<RT, SZ>
__esimd_dpas_inner(const sycl::INTEL::gpu::vector_type_t<RT, SZ> *src0,
                   const sycl::INTEL::gpu::vector_type_t<T1, N1> &src1,
                   const sycl::INTEL::gpu::vector_type_t<T2, N2> &src2) {
  sycl::INTEL::gpu::vector_type_t<RT, SZ> retv;

  sycl::INTEL::gpu::uint sat1 =
      EsimdEmulSys::SetSatur<T1, is_inttype<RT>::value>::set() ||
              EsimdEmulSys::SetSatur<T2, is_inttype<RT>::value>::set();

  constexpr sycl::INTEL::gpu::uint ops_per_chan =
      src1_precision == sycl::INTEL::gpu::EsimdPrecisionType::BF16 ||
              src1_precision == sycl::INTEL::gpu::EsimdPrecisionType::FP16 ||
              src2_precision == sycl::INTEL::gpu::EsimdPrecisionType::BF16 ||
              src2_precision == sycl::INTEL::gpu::EsimdPrecisionType::FP16
          ? 2
      : src1_precision == sycl::INTEL::gpu::EsimdPrecisionType::S8 ||
              src1_precision == sycl::INTEL::gpu::EsimdPrecisionType::U8 ||
              src2_precision == sycl::INTEL::gpu::EsimdPrecisionType::S8 ||
              src2_precision == sycl::INTEL::gpu::EsimdPrecisionType::U8
          ? 4
          : 8;

  sycl::INTEL::gpu::uint V = 0, U = 0, k = 0, temp = 0, src1_ops_per_dword = 0,
                         p = 0;

  constexpr auto src1_el_bits = __esimd_dpas_bits_precision(src1_precision);
  constexpr auto src2_el_bits = __esimd_dpas_bits_precision(src2_precision);

  uint32_t src1_signed =
      src1_precision == sycl::INTEL::gpu::EsimdPrecisionType::S2 ||
              src1_precision == sycl::INTEL::gpu::EsimdPrecisionType::S4 ||
              src1_precision == sycl::INTEL::gpu::EsimdPrecisionType::S8
          ? 1
          : 0;

  uint32_t src2_signed =
      src2_precision == sycl::INTEL::gpu::EsimdPrecisionType::S2 ||
              src2_precision == sycl::INTEL::gpu::EsimdPrecisionType::S4 ||
              src2_precision == sycl::INTEL::gpu::EsimdPrecisionType::S8
          ? 1
          : 0;

#if defined(ESIMD_GEN12_7)
  constexpr bool isPvc = true;
  constexpr size_t SIMDSize = 16;
#else
  constexpr bool isPvc = false;
  constexpr size_t SIMDSize = 8;
#endif

  constexpr bool
      pvcHfDest = isPvc && std::is_same<RT, half>::value,
      pvcBfDest = isPvc && std::is_same<RT, short>::value,
      pvcBfOrHfDest = pvcBfDest || pvcHfDest,

      pvcBfDestChecks =
          pvcBfDest && src1_precision == sycl::INTEL::gpu::EsimdPrecisionType::BF16 &&
          src2_precision == sycl::INTEL::gpu::EsimdPrecisionType::BF16,

      pvcHfDestChecks =
          pvcHfDest &&
          ((src1_precision == sycl::INTEL::gpu::EsimdPrecisionType::FP16 &&
            src2_precision == sycl::INTEL::gpu::EsimdPrecisionType::FP16) ||
           (src1_precision == sycl::INTEL::gpu::EsimdPrecisionType::BF16 &&
            src2_precision == sycl::INTEL::gpu::EsimdPrecisionType::BF16)),

      destTypeChk = (!pvcBfOrHfDest && is_fp_or_dword_type<RT>::value) ||
                    (pvcBfOrHfDest && (pvcBfDestChecks || pvcHfDestChecks)),

      srcTypeChk = is_dword_type<T1>::value && is_dword_type<T2>::value,

      destSizeChk = SZ >= /*TODO: ==*/SIMDSize * repeat_count,

      systolicDepthAndRepeatCountChk =
          systolic_depth == 8 && repeat_count >= 1 && repeat_count <= 8,

      src1CountChk =
          N1 == ((src1_el_bits * systolic_depth * ops_per_chan * SZ) /
                 (repeat_count * sizeof(T1) * 8)),
      src2CountChk =
          N2 >= ((src2_el_bits * systolic_depth * ops_per_chan * repeat_count) /
                 (sizeof(T2) * 8))
      /*TODO: ==; fix PVCIGEMM24*/
      ;

  if constexpr (!isPvc)
    static_assert(!pvcBfOrHfDest, "dpas: hfloat and bfloat16 destination "
                                  "element type is only supported on PVC.");
  static_assert(destTypeChk, "dpas: unsupported dest and accumulator type.");
  static_assert(srcTypeChk, "dpas: unsupported src element type.");
  static_assert(destSizeChk,
                "dpas: destination size must be SIMDSize x repeat_count.");
  static_assert(systolicDepthAndRepeatCountChk,
                "dpas: only systolic_depth = 8 and repeat_count of 1 to 8 are "
                "supported.");
  static_assert(src1CountChk, "dpas: invalid size for src1.");
  static_assert(src2CountChk, "dpas: invalid size for src2.");

  using TmpAccEl = typename std::conditional<
      pvcBfOrHfDest, float,
      typename restype_ex<RT, typename restype_ex<T1, T2>::type>::type>::type;

  sycl::INTEL::gpu::vector_type_t<TmpAccEl, SIMDSize> simdAcc;

  for (uint r = 0; r < repeat_count; r++) {
    V = r;
    k = 0;

    for (uint n = 0; n < SIMDSize; n++) {
      if (src0 != nullptr) {
        auto src0El = src0[0][r * SIMDSize + n];

        if (pvcBfDest) {
          const auto tmp = (uint32_t)(src0El) << 16;
          simdAcc[n] = reinterpret_cast<const TmpAccEl &>(tmp);
        } else
          simdAcc[n] = src0El;
      } else
        simdAcc[n] = 0;
    }

    for (uint s = 0; s < systolic_depth; s++) {
      src1_ops_per_dword = 32 / (ops_per_chan * src1_el_bits);
      // U = s / src1_ops_per_dword;
      U = s >> uint(log2(src1_ops_per_dword));

      for (uint n = 0; n < SIMDSize; n++) {
        for (uint d = 0; d < ops_per_chan; d++) {
          p = d + (s % src1_ops_per_dword) * ops_per_chan;
          uint32_t extension_temp = false;

          if (src2_precision == sycl::INTEL::gpu::EsimdPrecisionType::BF16) {
            const auto s1 =
                extract<uint32_t>(src1_el_bits, p * src1_el_bits,
                                  src1[U * SIMDSize + n], extension_temp)
                << 16;
            const auto s2 =
                extract<uint32_t>(src2_el_bits, d * src2_el_bits,
                                  src2[V * 8 + k / ops_per_chan], src2_signed)
                << 16;
            simdAcc[n] += reinterpret_cast<const float &>(s2) *
                          reinterpret_cast<const float &>(s1);
          } else if (src2_precision == sycl::INTEL::gpu::EsimdPrecisionType::FP16) {
            const auto s1 =
                extract<short>(src1_el_bits, p * src1_el_bits,
                               src1[U * SIMDSize + n], extension_temp);
            const auto s2 =
                extract<short>(src2_el_bits, d * src2_el_bits,
                               src2[V * 8 + k / ops_per_chan], src2_signed);
            simdAcc[n] += reinterpret_cast<const half &>(s1) *
                          reinterpret_cast<const half &>(s2);
          } else {
            int src = (sizeof(T2) * 8) / (ops_per_chan * src2_el_bits);
            int off = s % src * (ops_per_chan * src2_el_bits);
            int src1_tmp = extract<T1>(src1_el_bits, p * src1_el_bits,
                                       src1[U * SIMDSize + n], src1_signed);
            int src2_tmp = extract<T2>(src2_el_bits, d * src2_el_bits + off,
                                       src2[(V * 8 + k / ops_per_chan) / src],
                                       src2_signed);
            simdAcc[n] += src1_tmp * src2_tmp;
          }
        }
      }

      k += ops_per_chan;

    } // Systolic phase.

    for (uint n = 0; n < SIMDSize; n++) {
      if constexpr (pvcBfDest) {
        // TODO: make abstraction, support saturation, review rounding algo for
        // corner cases.
        auto tmpFloat = simdAcc[n];
        auto tmpUint = reinterpret_cast<uint32_t &>(tmpFloat);
        if (std::isnormal(tmpFloat) && tmpUint & 1ull << 15 &&
            (tmpUint & 0x7fff || tmpUint & 1ull << 16)) {
          tmpUint += 1ull << 16;
        }
        retv[r * SIMDSize + n] =
            static_cast<short>(reinterpret_cast<uint32_t &>(tmpUint) >> 16);
      } else
        retv[r * SIMDSize + n] =
            EsimdEmulSys::satur<RT>::saturate(simdAcc[n], sat1);
    }

  } // Repeat.

  return retv;
}

template <sycl::INTEL::gpu::EsimdPrecisionType src1_precision,
          sycl::INTEL::gpu::EsimdPrecisionType src2_precision,
          int systolic_depth, int repeat_count, typename T, typename T1,
          typename T2, int N, int N1, int N2>
inline sycl::INTEL::gpu::vector_type_t<T, N>
__esimd_dpas(sycl::INTEL::gpu::vector_type_t<T, N> src0,
             sycl::INTEL::gpu::vector_type_t<T1, N1> src1,
             sycl::INTEL::gpu::vector_type_t<T2, N2> src2) {
#ifdef __SYCL_EXPLICIT_SIMD_PLUGIN__
  return __esimd_dpas_inner<src1_precision, src2_precision, systolic_depth,
                            repeat_count, T, T1, T2, N, N1, N2>(
      std::addressof(src0), src1, src2);
#else  // __SYCL_EXPLICIT_SIMD_PLUGIN__
  throw cl::sycl::feature_not_supported();
  return sycl::INTEL::gpu::vector_type_t<T, N>();
#endif // __SYCL_EXPLICIT_SIMD_PLUGIN__
}

template <sycl::INTEL::gpu::EsimdPrecisionType src1_precision,
          sycl::INTEL::gpu::EsimdPrecisionType src2_precision,
          int systolic_depth, int repeat_count, typename T, typename T1,
          typename T2, int N, int N1, int N2>
inline sycl::INTEL::gpu::vector_type_t<T, N>
__esimd_dpas2(sycl::INTEL::gpu::vector_type_t<T1, N1> src1,
              sycl::INTEL::gpu::vector_type_t<T2, N2> src2) {
#ifdef __SYCL_EXPLICIT_SIMD_PLUGIN__
  return __esimd_dpas_inner<src1_precision, src2_precision, systolic_depth,
                            repeat_count, T, T1, T2, N, N1, N2>(nullptr, src1,
                                                                src2);
#else  // __SYCL_EXPLICIT_SIMD_PLUGIN__
  throw cl::sycl::feature_not_supported();
  return sycl::INTEL::gpu::vector_type_t<T, N>();
#endif // __SYCL_EXPLICIT_SIMD_PLUGIN__
}

template <sycl::INTEL::gpu::EsimdPrecisionType src1_precision,
          sycl::INTEL::gpu::EsimdPrecisionType src2_precision,
          int systolic_depth, int repeat_count, typename T, typename T1,
          typename T2, int N, int N1, int N2>
inline sycl::INTEL::gpu::vector_type_t<T, N>
__esimd_dpasw(sycl::INTEL::gpu::vector_type_t<T, N> src0,
              sycl::INTEL::gpu::vector_type_t<T1, N1> src1,
              sycl::INTEL::gpu::vector_type_t<T2, N2> src2) {
  throw cl::sycl::feature_not_supported();
  return sycl::INTEL::gpu::vector_type_t<T, N>();
}

template <sycl::INTEL::gpu::EsimdPrecisionType src1_precision,
          sycl::INTEL::gpu::EsimdPrecisionType src2_precision,
          int systolic_depth, int repeat_count, typename T, typename T1,
          typename T2, int N, int N1, int N2>
inline sycl::INTEL::gpu::vector_type_t<T, N>
__esimd_dpasw2(sycl::INTEL::gpu::vector_type_t<T1, N1> src1,
               sycl::INTEL::gpu::vector_type_t<T2, N2> src2) {
  throw cl::sycl::feature_not_supported();
  return sycl::INTEL::gpu::vector_type_t<T, N>();
}

/* end INTEL_FEATURE_ESIMD_EMBARGO */
/* end INTEL_CUSTOMIZATION */

#undef __SIGED

#endif // #ifndef __SYCL_DEVICE_ONLY__

#undef __SIGD
