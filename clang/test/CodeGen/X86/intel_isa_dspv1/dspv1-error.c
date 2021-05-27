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
