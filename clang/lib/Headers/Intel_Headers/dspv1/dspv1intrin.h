/*===------------------- dspintrin.h - DSP intrinsics ----------------------===
 *
 * Copyright (C) 2021 Intel Corporation. All rights reserved.
 *
 * The information and source code contained herein is the exclusive property
 * of Intel Corporation and may not be disclosed, examined or reproduced in
 * whole or in part without explicit written authorization from the company.
 *
 *===----------------------------------------------------------------------===
 */
#ifndef __IMMINTRIN_H
#error "Never use <dspintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __DSPINTRIN_H
#define __DSPINTRIN_H

/* Below intrinsics defined in other headers can be used for DSPV1 */
// \fn __m128i _mm_alignr_epi8(__m128i a, __m128i b, int imm8)

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("dspv1"),          \
                 __min_vector_width__(128)))

#define _mm_dsp_pcr2bfrsw_epi16(A, B, C, D)                                    \
  ((__m128i) __builtin_ia32_dvpcr2bfrsw((__v8hi)A, (__v8hi)B, (__v8hi)C, (int)D))

#define _mm_dsp_plutsincosw_epi16(A, B)                                        \
  ((__m128i) __builtin_ia32_dvplutsincosw((__v8hi)A, (int)B))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmuludhhq_epi64(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpmuludhhq((__v4su)__A, (__v4su)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmuldhhq_epi64(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpmuldhhq((__v4si)__A, (__v4si)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmuludllq_epi64(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpmuludllq((__v4su)__A, (__v4su)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmuldllq_epi64(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpmuldllq((__v4su)__A, (__v4su)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmuldfrs_epi32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpmuldfrs((__v4si)__A, (__v4si)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmulds_epi32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpmulds((__v4si)__A, (__v4si)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmacudllsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmacudllsq((__v2du)__A, (__v4su)__B,
                                              (__v4su)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmacudhhsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmacudhhsq((__v2du)__A, (__v4su)__B,
                                              (__v4su)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmacudllq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmacudllq((__v2du)__A, (__v4su)__B,
                                             (__v4su)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmacudhhq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmacudhhq((__v2du)__A, (__v4su)__B,
                                             (__v4su)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmacdllsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmacdllsq((__v2di)__A, (__v4si)__B,
                                             (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmacdhhsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmacdhhsq((__v2di)__A, (__v4si)__B,
                                             (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmacdllq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmacdllq((__v2di)__A, (__v4si)__B,
                                            (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmacdhhq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmacdhhq((__v2di)__A, (__v4si)__B,
                                            (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pnmacdllsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpnmacdllsq((__v2di)__A, (__v4si)__B,
                                              (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pnmacdhhsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpnmacdhhsq((__v2di)__A, (__v4si)__B,
                                              (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pnmacdllq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpnmacdllq((__v2di)__A, (__v4si)__B,
                                             (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pnmacdhhq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpnmacdhhq((__v2di)__A, (__v4si)__B,
                                             (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmsubadddllq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmsubadddllq((__v2di)__A, (__v4si)__B,
                                                (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pdpbuud_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpdpbuud((__v4su)__A, (__v16qu)__B,
                                           (__v16qu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pdpbuuds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpdpbuuds((__v4su)__A, (__v16qu)__B,
                                            (__v16qu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pdpbssd_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpdpbssd((__v4su)__A, (__v16qu)__B,
                                           (__v16qu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pdpbssds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpdpbssds((__v4su)__A, (__v16qu)__B,
                                            (__v16qu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pdpbsud_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpdpbsud((__v4su)__A, (__v16qu)__B,
                                           (__v16qu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pdpbsuds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpdpbsuds((__v4su)__A, (__v16qu)__B,
                                            (__v16qu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pndpbssd_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpndpbssd((__v4su)__A, (__v16qu)__B,
                                            (__v16qu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pdpwuuq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpdpwuuq((__v2du)__A, (__v8hu)__B,
                                           (__v8hu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pdpwssq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpdpwssq((__v2du)__A, (__v8hu)__B,
                                           (__v8hu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pdpwsuq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpdpwsuq((__v2du)__A, (__v8hu)__B,
                                           (__v8hu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pdpwusq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpdpwusq((__v2du)__A, (__v8hu)__B,
                                           (__v8hu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pndpwssq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpndpwssq((__v2du)__A, (__v8hu)__B,
                                            (__v8hu)__C);
}

#define _mm_dsp_pcmulwrs_epi16(A, B, C)                                        \
  ((__m128i) __builtin_ia32_dvpcmulwrs((__v8hu)(A), (__v8hu)(B), (int)(C)))

#define _mm_dsp_pccmulwrs_epi16(A, B, C)                                       \
  ((__m128i) __builtin_ia32_dvpccmulwrs((__v8hu)(A), (__v8hu)(B), (int)(C)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pcdpwqre_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpcdpwqre((__v2du)__A, (__v8hu)__B,
                                            (__v8hu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pcdpwqimm_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpcdpwqimm((__v2du)__A, (__v8hu)__B,
                                             (__v8hu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pncdpwqre_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpncdpwqre((__v2du)__A, (__v8hu)__B,
                                             (__v8hu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pncdpwqimm_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpncdpwqimm((__v2du)__A, (__v8hu)__B,
                                              (__v8hu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pccdpwqimm_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpccdpwqimm((__v2du)__A, (__v8hu)__B,
                                              (__v8hu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pnccdpwqimm_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpnccdpwqimm((__v2du)__A, (__v8hu)__B,
                                               (__v8hu)__C);
}

#define _mm_dsp_psrard_epi32(A, B)                                             \
  ((__m128i) __builtin_ia32_dvpsrard((__v4su)(A), (int)(B)))

#define _mm_dsp_pslsd_epi32(A, B)                                              \
  ((__m128i) __builtin_ia32_dvpslsd((__v4su)(A), (int)(B)))

#define _mm_dsp_psrrud_epi32(A, B)                                             \
  ((__m128i) __builtin_ia32_dvpsrrud((__v4su)(A), (int)(B)))

#define _mm_dsp_pslsud_epi32(A, B)                                             \
  ((__m128i) __builtin_ia32_dvpslsud((__v4su)(A), (int)(B)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_psravrd_epi32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpsravrd((__v4su)__A, (__v4su)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pslvsd_epi32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpslvsd((__v4su)__A, (__v4su)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_psrvrud_epi32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpsrvrud((__v4su)__A, (__v4su)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pslvsud_epi32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpslvsud((__v4su)__A, (__v4su)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_phaddlswuq_epi16(__m128i __A) {
  return (__m128i)__builtin_ia32_dvphaddlswuq((__v8hu)__A);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_phaddlswq_epi16(__m128i __A) {
  return (__m128i)__builtin_ia32_dvphaddlswq((__v8hu)__A);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_phaddlsduq_epi32(__m128i __A) {
  return (__m128i)__builtin_ia32_dvphaddlsduq((__v4su)__A);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_phaddlsdq_epi32(__m128i __A) {
  return (__m128i)__builtin_ia32_dvphaddlsdq((__v4su)__A);
}

#define _mm_max_epi16(__a, __b)                                                \
    ((__m128i)__builtin_elementwise_max((__v8hi)(__a), (__v8hi)(__b)))

#define _mm_max_epu8(__a, __b)                                                 \
  ((__m128i)__builtin_elementwise_max((__v16qu)(__a), (__v16qu)(__b)))

#define _mm_min_epi16(__a, __b)                                                \
  ((__m128i)__builtin_elementwise_min((__v8hi)(__a), (__v8hi)(__b)))

#define _mm_min_epu8(__a, __b)                                                 \
  ((__m128i)__builtin_elementwise_min((__v16qu)(__a), (__v16qu)(__b)))

#define _mm_min_epi8(__V1, __V2)                                               \
  ((__m128i)__builtin_elementwise_min((__v16qs)(__V1), (__v16qs)(__V2)))

#define _mm_max_epi8(__V1, __V2)                                               \
  ((__m128i)__builtin_elementwise_max((__v16qs)(__V1), (__v16qs)(__V2)))

#define _mm_min_epu16(__V1, __V2)                                              \
  ((__m128i)__builtin_elementwise_min((__v8hu)(__V1), (__v8hu)(__V2)))

#define _mm_max_epu16(__V1, __V2)                                              \
  ((__m128i)__builtin_elementwise_max((__v8hu)(__V1), (__v8hu)(__V2)))

#define _mm_min_epi32(__V1, __V2)                                              \
  ((__m128i)__builtin_elementwise_min((__v4si)(__V1), (__v4si)(__V2)))

#define _mm_max_epi32(__V1, __V2)                                              \
  ((__m128i)__builtin_elementwise_max((__v4si)(__V1), (__v4si)(__V2)))

#define _mm_min_epu32(__V1, __V2)                                              \
  ((__m128i)__builtin_elementwise_min((__v4su)(__V1), (__v4su)(__V2)))

#define _mm_max_epu32(__V1, __V2)                                              \
  ((__m128i)__builtin_elementwise_max((__v4su)(__V1), (__v4su)(__V2)))

#define _mm_dpbusd_epi32(S, A, B)                                              \
  ((__m128i)__builtin_ia32_vpdpbusd128((__v4si)(S), (__v4si)(A), (__v4si)(B)))

#define _mm_dpbusds_epi32(S, A, B)                                             \
  ((__m128i)__builtin_ia32_vpdpbusds128((__v4si)(S), (__v4si)(A), (__v4si)(B)))

#define _mm_dsp_pdpbwuud_epi32(A, B, C, D)                                     \
  ((__m128i)__builtin_ia32_dvpdpbwuud((__v4su)(A), (__v16qu)(B), (__v8hu)(C),  \
                                      (int)(D)))

#define _mm_dsp_pdpbwuuds_epi32(A, B, C, D)                                    \
  ((__m128i)__builtin_ia32_dvpdpbwuuds((__v4su)(A), (__v16qu)(B), (__v8hu)(C), \
                                       (int)(D)))

#define _mm_dsp_pdpbwssd_epi32(A, B, C, D)                                     \
  ((__m128i)__builtin_ia32_dvpdpbwssd((__v4su)(A), (__v16qu)(B), (__v8hu)(C),  \
                                      (int)(D)))

#define _mm_dsp_pdpbwssds_epi32(A, B, C, D)                                    \
  ((__m128i)__builtin_ia32_dvpdpbwssds((__v4su)(A), (__v16qu)(B), (__v8hu)(C), \
                                       (int)(D)))

#define _mm_dsp_pdpbwsud_epi32(A, B, C, D)                                     \
  ((__m128i)__builtin_ia32_dvpdpbwsud((__v4su)(A), (__v16qu)(B), (__v8hu)(C),  \
                                      (int)(D)))

#define _mm_dsp_pdpbwsuds_epi32(A, B, C, D)                                    \
  ((__m128i)__builtin_ia32_dvpdpbwsuds((__v4su)(A), (__v16qu)(B), (__v8hu)(C), \
                                       (int)(D)))

#define _mm_dsp_pdpbwusd_epi32(A, B, C, D)                                     \
  ((__m128i)__builtin_ia32_dvpdpbwusd((__v4su)(A), (__v16qu)(B), (__v8hu)(C),  \
                                      (int)(D)))

#define _mm_dsp_pdpbwusds_epi32(A, B, C, D)                                    \
  ((__m128i)__builtin_ia32_dvpdpbwusds((__v4su)(A), (__v16qu)(B), (__v8hu)(C), \
                                       (int)(D)))

#define _mm_dsp_pmuluwdq_epi64(A, B, C)                                        \
  ((__m128i)__builtin_ia32_dvpmuluwdq((__v8hu)(A), (__v4su)(B), (int)(C)))

#define _mm_dsp_pmulwdq_epi64(A, B, C)                                         \
  ((__m128i)__builtin_ia32_dvpmulwdq((__v8hu)(A), (__v4su)(B), (int)(C)))

#define _mm_dsp_pdpwduuq_epi64(A, B, C, D)                                     \
  ((__m128i)__builtin_ia32_dvpdpwduuq((__v2du)(A), (__v8hu)(B), (__v4su)(C),   \
                                      (int)(D)))

#define _mm_dsp_pdpwdssq_epi64(A, B, C, D)                                     \
  ((__m128i)__builtin_ia32_dvpdpwdssq((__v2du)(A), (__v8hu)(B), (__v4su)(C),   \
                                      (int)(D)))

#define _mm_dsp_pcr2bfrsdre_epi32(A, B, C, D)                                  \
  ((__m128i)__builtin_ia32_dvpcr2bfrsdre((__v4su)(A), (__v4su)(B),             \
                                         (__v8hu)(C), (int)(D)))

#define _mm_dsp_pcr2bfrsdimm_epi32(A, B, C, D)                                 \
  ((__m128i)__builtin_ia32_dvpcr2bfrsdimm((__v4su)(A), (__v4su)(B),            \
                                          (__v8hu)(C), (int)(D)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_psadaccubws_epi16(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpsadaccubws((__v8hu)__A, (__v16qu)__B,
                                               (__v16qu)__C);
}

#define _mm_dsp_psrrsuqw_epi16(A, B)                                           \
  ((__m128i)__builtin_ia32_dvpsrrsuqw((__v2du)(A), (int)(B)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_psrvrsuqw_epi16(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpsrvrsuqw((__v2du)__A, (__v2du)__B);
}

#define _mm_dsp_pslrsuqw_epi16(A, B)                                           \
  ((__m128i)__builtin_ia32_dvpslrsuqw((__v2du)(A), (int)(B)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pslvrsuqw_epi16(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpslvrsuqw((__v2du)__A, (__v2du)__B);
}

#define _mm_dsp_psrarsqw_epi16(A, B)                                           \
  ((__m128i)__builtin_ia32_dvpsrarsqw((__v2du)(A), (int)(B)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_psravrsqw_epi16(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpsravrsqw((__v2du)__A, (__v2du)__B);
}

#define _mm_dsp_pslrsqw_epi16(A, B)                                            \
  ((__m128i)__builtin_ia32_dvpslrsqw((__v2du)(A), (int)(B)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pslvrsqw_epi16(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpslvrsqw((__v2du)__A, (__v2du)__B);
}

#define _mm_dsp_psrrsuqd_epi32(A, B)                                           \
  ((__m128i)__builtin_ia32_dvpsrrsuqd((__v2du)(A), (int)(B)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_psrvrsuqd_epi32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpsrvrsuqd((__v2du)__A, (__v2du)__B);
}

#define _mm_dsp_pslrsuqd_epi32(A, B)                                           \
  ((__m128i)__builtin_ia32_dvpslrsuqd((__v2du)(A), (int)(B)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pslvrsuqd_epi32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpslvrsuqd((__v2du)__A, (__v2du)__B);
}

#define _mm_dsp_psrarsqd_epi32(A, B)                                           \
  ((__m128i)__builtin_ia32_dvpsrarsqd((__v2du)(A), (int)(B)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_psravrsqd_epi32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpsravrsqd((__v2du)__A, (__v2du)__B);
}

#define _mm_dsp_pslrsqd_epi32(A, B)                                            \
  ((__m128i)__builtin_ia32_dvpslrsqd((__v2du)(A), (int)(B)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pslvrsqd_epi32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpslvrsqd((__v2du)__A, (__v2du)__B);
}

#define _mm_dsp_pcaddrotsraw_epi16(A, B, C)                                    \
  ((__m128i)__builtin_ia32_dvpcaddrotsraw((__v8hu)(A), (__v8hu)(B), (int)(C)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmuluwr_epi16(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpmuluwr((__v8hu)__A, (__v8hu)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmulwr_epi16(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpmulwr((__v8hu)__A, (__v8hu)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmulws_epi16(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpmulws((__v8hu)__A, (__v8hu)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmulwfrs_epi16(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpmulwfrs((__v8hu)__A, (__v8hu)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pdpint4uud_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpdpint4uud((__v4su)__A, (__v16qu)__B,
                                              (__v16qu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pdpint4ssd_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpdpint4ssd((__v4su)__A, (__v16qu)__B,
                                              (__v16qu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pdpint4usd_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpdpint4usd((__v4su)__A, (__v16qu)__B,
                                              (__v16qu)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pdpint4sud_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpdpint4sud((__v4su)__A, (__v16qu)__B,
                                              (__v16qu)__C);
}

#define _mm_dsp_pcaddrotsrad_epi32(A, B, C)                                    \
  ((__m128i)__builtin_ia32_dvpcaddrotsrad((__v4su)(A), (__v4su)(B), (int)(C)))

#define _mm_dsp_punpckdq_epi32(A, B, C)                                        \
  ((__m128i)__builtin_ia32_dvpunpckdq((__v4su)(A), (__v4su)(B), (int)(C)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmasklddqu_epi8(__m128i __A, const __m128i *__B) {
  return (__m128i)__builtin_ia32_dvpmasklddqu((__v16qu)__A,
                                              (const __v16qu *)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_dsp_pmaskstdqu_epi8(__m128i *__A, __m128i __B, __m128i __C) {
  __builtin_ia32_dvpmaskstdqu((__v16qu *)__A, (__v16qu)__B, (__v16qu)__C);
}

#if __has_extension(gnu_asm)
static __inline__ void __DEFAULT_FN_ATTRS128
_mm_dsp_ptestmxcsrflgs(void) {
  __asm__ __volatile__ ("dvptestmxcsrflgs" : : : "memory");
}
#endif // /* __has_extension(gnu_asm) */

#undef __DEFAULT_FN_ATTRS128
#endif // __DSPINTRIN_H
