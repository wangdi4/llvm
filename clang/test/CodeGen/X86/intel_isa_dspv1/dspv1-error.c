// REQUIRES: intel_feature_isa_dspv1
// RUN: %clang_cc1 %s -ffreestanding -triple=i686-unknown-unknown \
// RUN: -target-feature +dspv1  -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_dsp_plutsincosw_epi16(__m128i __A) {
  return _mm_dsp_plutsincosw_epi16(__A, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pcr2bfrsw_epi16(__m128i __A, __m128i __B, __m128i __C) {
  return _mm_dsp_pcr2bfrsw_epi16(__A, __B, __C, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pcmulwrs_epi16(__m128i __A, __m128i __B) {
  return _mm_dsp_pcmulwrs_epi16(__A, __B, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pccmulwrs_epi16(__m128i __A, __m128i __B) {
  return _mm_dsp_pccmulwrs_epi16(__A, __B, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_psrard_epi32(__m128i __A) {
  return _mm_dsp_psrard_epi32(__A, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pslsd_epi32(__m128i __A) {
  return _mm_dsp_pslsd_epi32(__A, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_psrrud_epi32(__m128i __A) {
  return _mm_dsp_psrrud_epi32(__A, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pslsud_epi32(__m128i __A) {
  return _mm_dsp_pslsud_epi32(__A, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pdpbwuud_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return _mm_dsp_pdpbwuud_epi32(__A, __B, __C, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pdpbwuuds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return _mm_dsp_pdpbwuuds_epi32(__A, __B, __C, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pdpbwssd_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return _mm_dsp_pdpbwssd_epi32(__A, __B, __C, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pdpbwssds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return _mm_dsp_pdpbwssds_epi32(__A, __B, __C, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pdpbwsud_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return _mm_dsp_pdpbwsud_epi32(__A, __B, __C, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pdpbwsuds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return _mm_dsp_pdpbwsuds_epi32(__A, __B, __C, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pdpbwusd_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return _mm_dsp_pdpbwusd_epi32(__A, __B, __C, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pdpbwusds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return _mm_dsp_pdpbwusds_epi32(__A, __B, __C, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pmuluwdq_epi64(__m128i __A, __m128i __B) {
  return _mm_dsp_pmuluwdq_epi64(__A, __B, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pmulwdq_epi64(__m128i __A, __m128i __B) {
  return _mm_dsp_pmulwdq_epi64(__A, __B, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pdpwduuq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return _mm_dsp_pdpwduuq_epi64(__A, __B, __C, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pdpwdssq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return _mm_dsp_pdpwdssq_epi64(__A, __B, __C, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pcr2bfrsdre_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return _mm_dsp_pcr2bfrsdre_epi32(__A, __B, __C, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pcr2bfrsdimm_epi32(__m128i __A, __m128i __B, __m128i __C) {
  return _mm_dsp_pcr2bfrsdimm_epi32(__A, __B, __C, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_psrrsuqw_epi16(__m128i __A) {
  return _mm_dsp_psrrsuqw_epi16(__A, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pslrsuqw_epi16(__m128i __A) {
  return _mm_dsp_pslrsuqw_epi16(__A, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_psrarsqw_epi16(__m128i __A) {
  return _mm_dsp_psrarsqw_epi16(__A, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pslrsqw_epi16(__m128i __A) {
  return _mm_dsp_pslrsqw_epi16(__A, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_psrrsuqd_epi32(__m128i __A) {
  return _mm_dsp_psrrsuqd_epi32(__A, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pslrsuqd_epi32(__m128i __A) {
  return _mm_dsp_pslrsuqd_epi32(__A, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_psrarsqd_epi32(__m128i __A) {
  return _mm_dsp_psrarsqd_epi32(__A, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pslrsqd_epi32(__m128i __A) {
  return _mm_dsp_pslrsqd_epi32(__A, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pcaddrotsraw_epi16(__m128i __A, __m128i __B) {
  return _mm_dsp_pcaddrotsraw_epi16(__A, __B, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_pcaddrotsrad_epi32(__m128i __A, __m128i __B) {
  return _mm_dsp_pcaddrotsrad_epi32(__A, __B, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

__m128i test_mm_dsp_punpckdq_epi32(__m128i __A, __m128i __B) {
  return _mm_dsp_punpckdq_epi32(__A, __B, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}
