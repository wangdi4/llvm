// REQUIRES: intel_feature_isa_avx512_sat_cvt
// UN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512vl -target-feature +avx512satcvt \
// UN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +avx512satcvt  -target-feature +avx512vl -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_cvtnebf162ibs_epi8(__m128bh __A) {
  return _mm_cvtnebf162ibs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_mask_cvtnebf162ibs_epi8(__m128i __S, __mmask8 __A, __m128bh __B) {
  return _mm_mask_cvtnebf162ibs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_maskz_cvtnebf162ibs_epi8(__mmask8 __A, __m128bh __B) {
  return _mm_maskz_cvtnebf162ibs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_cvtnebf162ibs_epi8(__m256bh __A) {
  return _mm256_cvtnebf162ibs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_mask_cvtnebf162ibs_epi8(__m256i __S, __mmask16 __A, __m256bh __B) {
  return _mm256_mask_cvtnebf162ibs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_maskz_cvtnebf162ibs_epi8(__mmask16 __A, __m256bh __B) {
  return _mm256_maskz_cvtnebf162ibs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_cvtnebf162iubs_epi8(__m128bh __A) {
  return _mm_cvtnebf162iubs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_mask_cvtnebf162iubs_epi8(__m128i __S, __mmask8 __A, __m128bh __B) {
  return _mm_mask_cvtnebf162iubs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_maskz_cvtnebf162iubs_epi8(__mmask8 __A, __m128bh __B) {
  return _mm_maskz_cvtnebf162iubs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_cvtnebf162iubs_epi8(__m256bh __A) {
  return _mm256_cvtnebf162iubs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_mask_cvtnebf162iubs_epi8(__m256i __S, __mmask16 __A, __m256bh __B) {
  return _mm256_mask_cvtnebf162iubs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_maskz_cvtnebf162iubs_epi8(__mmask16 __A, __m256bh __B) {
  return _mm256_maskz_cvtnebf162iubs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_cvtph2ibs_epi8(__m128h __A) {
  return _mm_cvtph2ibs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_mask_cvtph2ibs_epi8(__m128i __S, __mmask8 __A, __m128h __B) {
  return _mm_mask_cvtph2ibs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_maskz_cvtph2ibs_epi8(__mmask8 __A, __m128h __B) {
  return _mm_maskz_cvtph2ibs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_cvtph2ibs_epi8(__m256h __A) {
  return _mm256_cvtph2ibs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_mask_cvtph2ibs_epi8(__m256i __S, __mmask16 __A, __m256h __B) {
  return _mm256_mask_cvtph2ibs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_maskz_cvtph2ibs_epi8(__mmask16 __A, __m256h __B) {
  return _mm256_maskz_cvtph2ibs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_cvtph2iubs_epi8(__m128h __A) {
  return _mm_cvtph2iubs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_mask_cvtph2iubs_epi8(__m128i __S, __mmask8 __A, __m128h __B) {
  return _mm_mask_cvtph2iubs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_maskz_cvtph2iubs_epi8(__mmask8 __A, __m128h __B) {
  return _mm_maskz_cvtph2iubs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_cvtph2iubs_epi8(__m256h __A) {
  return _mm256_cvtph2iubs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_mask_cvtph2iubs_epi8(__m256i __S, __mmask16 __A, __m256h __B) {
  return _mm256_mask_cvtph2iubs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_maskz_cvtph2iubs_epi8(__mmask16 __A, __m256h __B) {
  return _mm256_maskz_cvtph2iubs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_cvtps2ibs_epi8(__m128 __A) {
  return _mm_cvtps2ibs_epi8(__A, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m128i test_mm_mask_cvtps2ibs_epi8(__m128i __S, __mmask8 __A, __m128 __B) {
  return _mm_mask_cvtps2ibs_epi8(__S, __A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m128i test_mm_maskz_cvtps2ibs_epi8(__mmask8 __A, __m128 __B) {
  return _mm_maskz_cvtps2ibs_epi8(__A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m256i test_mm256_cvtps2ibs_epi8(__m256 __A) {
  return _mm256_cvtps2ibs_epi8(__A, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m256i test_mm256_mask_cvtps2ibs_epi8(__m256i __S, __mmask8 __A, __m256 __B) {
  return _mm256_mask_cvtps2ibs_epi8(__S, __A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m256i test_mm256_maskz_cvtps2ibs_epi8(__mmask8 __A, __m256 __B) {
  return _mm256_maskz_cvtps2ibs_epi8(__A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m128i test_mm_cvtps2iubs_epi8(__m128 __A) {
  return _mm_cvtps2iubs_epi8(__A, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m128i test_mm_mask_cvtps2iubs_epi8(__m128i __S, __mmask8 __A, __m128 __B) {
  return _mm_mask_cvtps2iubs_epi8(__S, __A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m128i test_mm_maskz_cvtps2iubs_epi8(__mmask8 __A, __m128 __B) {
  return _mm_maskz_cvtps2iubs_epi8(__A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m256i test_mm256_cvtps2iubs_epi8(__m256 __A) {
  return _mm256_cvtps2iubs_epi8(__A, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m256i test_mm256_mask_cvtps2iubs_epi8(__m256i __S, __mmask8 __A, __m256 __B) {
  return _mm256_mask_cvtps2iubs_epi8(__S, __A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m256i test_mm256_maskz_cvtps2iubs_epi8(__mmask8 __A, __m256 __B) {
  return _mm256_maskz_cvtps2iubs_epi8(__A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m128i test_mm_cvttnebf162ibs_epi8(__m128bh __A) {
  return _mm_cvttnebf162ibs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_mask_cvttnebf162ibs_epi8(__m128i __S, __mmask8 __A, __m128bh __B) {
  return _mm_mask_cvttnebf162ibs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_maskz_cvttnebf162ibs_epi8(__mmask8 __A, __m128bh __B) {
  return _mm_maskz_cvttnebf162ibs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_cvttnebf162ibs_epi8(__m256bh __A) {
  return _mm256_cvttnebf162ibs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_mask_cvttnebf162ibs_epi8(__m256i __S, __mmask16 __A, __m256bh __B) {
  return _mm256_mask_cvttnebf162ibs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_maskz_cvttnebf162ibs_epi8(__mmask16 __A, __m256bh __B) {
  return _mm256_maskz_cvttnebf162ibs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_cvttnebf162iubs_epi8(__m128bh __A) {
  return _mm_cvttnebf162iubs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_mask_cvttnebf162iubs_epi8(__m128i __S, __mmask8 __A, __m128bh __B) {
  return _mm_mask_cvttnebf162iubs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_maskz_cvttnebf162iubs_epi8(__mmask8 __A, __m128bh __B) {
  return _mm_maskz_cvttnebf162iubs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_cvttnebf162iubs_epi8(__m256bh __A) {
  return _mm256_cvttnebf162iubs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_mask_cvttnebf162iubs_epi8(__m256i __S, __mmask16 __A, __m256bh __B) {
  return _mm256_mask_cvttnebf162iubs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_maskz_cvttnebf162iubs_epi8(__mmask16 __A, __m256bh __B) {
  return _mm256_maskz_cvttnebf162iubs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_cvttph2ibs_epi8(__m128h __A) {
  return _mm_cvttph2ibs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_mask_cvttph2ibs_epi8(__m128i __S, __mmask8 __A, __m128h __B) {
  return _mm_mask_cvttph2ibs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_maskz_cvttph2ibs_epi8(__mmask8 __A, __m128h __B) {
  return _mm_maskz_cvttph2ibs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_cvttph2ibs_epi8(__m256h __A) {
  return _mm256_cvttph2ibs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_mask_cvttph2ibs_epi8(__m256i __S, __mmask16 __A, __m256h __B) {
  return _mm256_mask_cvttph2ibs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_maskz_cvttph2ibs_epi8(__mmask16 __A, __m256h __B) {
  return _mm256_maskz_cvttph2ibs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_cvttph2iubs_epi8(__m128h __A) {
  return _mm_cvttph2iubs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_mask_cvttph2iubs_epi8(__m128i __S, __mmask8 __A, __m128h __B) {
  return _mm_mask_cvttph2iubs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_maskz_cvttph2iubs_epi8(__mmask8 __A, __m128h __B) {
  return _mm_maskz_cvttph2iubs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_cvttph2iubs_epi8(__m256h __A) {
  return _mm256_cvttph2iubs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_mask_cvttph2iubs_epi8(__m256i __S, __mmask16 __A, __m256h __B) {
  return _mm256_mask_cvttph2iubs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m256i test_mm256_maskz_cvttph2iubs_epi8(__mmask16 __A, __m256h __B) {
  return _mm256_maskz_cvttph2iubs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m128i test_mm_cvttps2ibs_epi8(__m128 __A) {
  return _mm_cvttps2ibs_epi8(__A, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m128i test_mm_mask_cvttps2ibs_epi8(__m128i __S, __mmask8 __A, __m128 __B) {
  return _mm_mask_cvttps2ibs_epi8(__S, __A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m128i test_mm_maskz_cvttps2ibs_epi8(__mmask8 __A, __m128 __B) {
  return _mm_maskz_cvttps2ibs_epi8(__A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m256i test_mm256_cvttps2ibs_epi8(__m256 __A) {
  return _mm256_cvttps2ibs_epi8(__A, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m256i test_mm256_mask_cvttps2ibs_epi8(__m256i __S, __mmask8 __A, __m256 __B) {
  return _mm256_mask_cvttps2ibs_epi8(__S, __A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m256i test_mm256_maskz_cvttps2ibs_epi8(__mmask8 __A, __m256 __B) {
  return _mm256_maskz_cvttps2ibs_epi8(__A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m128i test_mm_cvttps2iubs_epi8(__m128 __A) {
  return _mm_cvttps2iubs_epi8(__A, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m128i test_mm_mask_cvttps2iubs_epi8(__m128i __S, __mmask8 __A, __m128 __B) {
  return _mm_mask_cvttps2iubs_epi8(__S, __A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m128i test_mm_maskz_cvttps2iubs_epi8(__mmask8 __A, __m128 __B) {
  return _mm_maskz_cvttps2iubs_epi8(__A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m256i test_mm256_cvttps2iubs_epi8(__m256 __A) {
  return _mm256_cvttps2iubs_epi8(__A, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m256i test_mm256_mask_cvttps2iubs_epi8(__m256i __S, __mmask8 __A, __m256 __B) {
  return _mm256_mask_cvttps2iubs_epi8(__S, __A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m256i test_mm256_maskz_cvttps2iubs_epi8(__mmask8 __A, __m256 __B) {
  return _mm256_maskz_cvttps2iubs_epi8(__A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}
