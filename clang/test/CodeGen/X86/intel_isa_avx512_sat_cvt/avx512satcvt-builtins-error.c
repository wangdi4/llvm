// REQUIRES: intel_feature_isa_avx512_sat_cvt
// UN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +avx512satcvt \
// UN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %si
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +avx512satcvt -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>

__m512i test_mm512_cvtnebf162ibs_epi8(__m512bh __A) {
  return _mm512_cvtnebf162ibs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_mask_cvtnebf162ibs_epi8(__m512i __S, __mmask32 __A, __m512bh __B) {
  return _mm512_mask_cvtnebf162ibs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_maskz_cvtnebf162ibs_epi8(__mmask32 __A, __m512bh __B) {
  return _mm512_maskz_cvtnebf162ibs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_cvtnebf162iubs_epi8(__m512bh __A) {
  return _mm512_cvtnebf162iubs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_mask_cvtnebf162iubs_epi8(__m512i __S, __mmask32 __A, __m512bh __B) {
  return _mm512_mask_cvtnebf162iubs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_maskz_cvtnebf162iubs_epi8(__mmask32 __A, __m512bh __B) {
  return _mm512_maskz_cvtnebf162iubs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_cvtph2ibs_epi8(__m512h __A) {
  return _mm512_cvtph2ibs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_mask_cvtph2ibs_epi8(__m512i __S, __mmask32 __A, __m512h __B) {
  return _mm512_mask_cvtph2ibs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_maskz_cvtph2ibs_epi8(__mmask32 __A, __m512h __B) {
  return _mm512_maskz_cvtph2ibs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_cvtph2ibs_round_epi8(__m512h __A) {
  return _mm512_cvtph2ibs_round_epi8(__A, 2, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_mask_cvtph2ibs_round_epi8(__m512i __S, __mmask32 __A, __m512h __B) {
  return _mm512_mask_cvtph2ibs_round_epi8(__S, __A, __B, 2, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_maskz_cvtph2ibs_round_epi8(__mmask32 __A, __m512h __B) {
  return _mm512_maskz_cvtph2ibs_round_epi8(__A, __B, 2, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_cvtph2iubs_epi8(__m512h __A) {
  return _mm512_cvtph2iubs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_mask_cvtph2iubs_epi8(__m512i __S, __mmask32 __A, __m512h __B) {
  return _mm512_mask_cvtph2iubs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_maskz_cvtph2iubs_epi8(__mmask32 __A, __m512h __B) {
  return _mm512_maskz_cvtph2iubs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_cvtph2iubs_round_epi8(__m512h __A) {
  return _mm512_cvtph2iubs_round_epi8(__A, 2, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_mask_cvtph2iubs_round_epi8(__m512i __S, __mmask32 __A, __m512h __B) {
  return _mm512_mask_cvtph2iubs_round_epi8(__S, __A, __B, 2, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_maskz_cvtph2iubs_round_epi8(__mmask32 __A, __m512h __B) {
  return _mm512_maskz_cvtph2iubs_round_epi8(__A, __B, 2, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_cvtps2ibs_epi8(__m512 __A) {
  return _mm512_cvtps2ibs_epi8(__A, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_mask_cvtps2ibs_epi8(__m512i __S, __mmask16 __A, __m512 __B) {
  return _mm512_mask_cvtps2ibs_epi8(__S, __A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_maskz_cvtps2ibs_epi8(__mmask16 __A, __m512 __B) {
  return _mm512_maskz_cvtps2ibs_epi8(__A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_cvtps2ibs_round_epi8(__m512 __A) {
  return _mm512_cvtps2ibs_round_epi8(__A, 4, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_mask_cvtps2ibs_round_epi8(__m512i __S, __mmask16 __A, __m512 __B) {
  return _mm512_mask_cvtps2ibs_round_epi8(__S, __A, __B, 4, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_maskz_cvtps2ibs_round_epi8(__mmask16 __A, __m512 __B) {
  return _mm512_maskz_cvtps2ibs_round_epi8(__A, __B, 4, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_cvtps2iubs_epi8(__m512 __A) {
  return _mm512_cvtps2iubs_epi8(__A, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_mask_cvtps2iubs_epi8(__m512i __S, __mmask16 __A, __m512 __B) {
  return _mm512_mask_cvtps2iubs_epi8(__S, __A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_maskz_cvtps2iubs_epi8(__mmask16 __A, __m512 __B) {
  return _mm512_maskz_cvtps2iubs_epi8(__A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_cvtps2iubs_round_epi8(__m512 __A) {
  return _mm512_cvtps2iubs_round_epi8(__A, 4, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_mask_cvtps2iubs_round_epi8(__m512i __S, __mmask16 __A, __m512 __B) {
  return _mm512_mask_cvtps2iubs_round_epi8(__S, __A, __B, 4, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_maskz_cvtps2iubs_round_epi8(__mmask16 __A, __m512 __B) {
  return _mm512_maskz_cvtps2iubs_round_epi8(__A, __B, 4, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_cvttnebf162ibs_epi8(__m512bh __A) {
  return _mm512_cvttnebf162ibs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_mask_cvttnebf162ibs_epi8(__m512i __S, __mmask32 __A, __m512bh __B) {
  return _mm512_mask_cvttnebf162ibs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_maskz_cvttnebf162ibs_epi8(__mmask32 __A, __m512bh __B) {
  return _mm512_maskz_cvttnebf162ibs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_cvttnebf162iubs_epi8(__m512bh __A) {
  return _mm512_cvttnebf162iubs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_mask_cvttnebf162iubs_epi8(__m512i __S, __mmask32 __A, __m512bh __B) {
  return _mm512_mask_cvttnebf162iubs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_maskz_cvttnebf162iubs_epi8(__mmask32 __A, __m512bh __B) {
  return _mm512_maskz_cvttnebf162iubs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_cvttph2ibs_epi8(__m512h __A) {
  return _mm512_cvttph2ibs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_mask_cvttph2ibs_epi8(__m512i __S, __mmask32 __A, __m512h __B) {
  return _mm512_mask_cvttph2ibs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_maskz_cvttph2ibs_epi8(__mmask32 __A, __m512h __B) {
  return _mm512_maskz_cvttph2ibs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_cvttph2ibs_round_epi8(__m512h __A) {
  return _mm512_cvttph2ibs_round_epi8(__A, 2, _MM_FROUND_NO_EXC); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_mask_cvttph2ibs_round_epi8(__m512i __S, __mmask32 __A, __m512h __B) {
  return _mm512_mask_cvttph2ibs_round_epi8(__S, __A, __B, 2, _MM_FROUND_NO_EXC); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_maskz_cvttph2ibs_round_epi8(__mmask32 __A, __m512h __B) {
  return _mm512_maskz_cvttph2ibs_round_epi8(__A, __B, 2, _MM_FROUND_NO_EXC); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_cvttph2iubs_epi8(__m512h __A) {
  return _mm512_cvttph2iubs_epi8(__A, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_mask_cvttph2iubs_epi8(__m512i __S, __mmask32 __A, __m512h __B) {
  return _mm512_mask_cvttph2iubs_epi8(__S, __A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_maskz_cvttph2iubs_epi8(__mmask32 __A, __m512h __B) {
  return _mm512_maskz_cvttph2iubs_epi8(__A, __B, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_cvttph2iubs_round_epi8(__m512h __A) {
  return _mm512_cvttph2iubs_round_epi8(__A, 2, _MM_FROUND_NO_EXC); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_mask_cvttph2iubs_round_epi8(__m512i __S, __mmask32 __A, __m512h __B) {
  return _mm512_mask_cvttph2iubs_round_epi8(__S, __A, __B, 2, _MM_FROUND_NO_EXC); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_maskz_cvttph2iubs_round_epi8(__mmask32 __A, __m512h __B) {
  return _mm512_maskz_cvttph2iubs_round_epi8(__A, __B, 2, _MM_FROUND_NO_EXC); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512i test_mm512_cvttps2ibs_epi8(__m512 __A) {
  return _mm512_cvttps2ibs_epi8(__A, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_mask_cvttps2ibs_epi8(__m512i __S, __mmask16 __A, __m512 __B) {
  return _mm512_mask_cvttps2ibs_epi8(__S, __A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_maskz_cvttps2ibs_epi8(__mmask16 __A, __m512 __B) {
  return _mm512_maskz_cvttps2ibs_epi8(__A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_cvttps2ibs_round_epi8(__m512 __A) {
  return _mm512_cvttps2ibs_round_epi8(__A, 4, _MM_FROUND_NO_EXC); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_mask_cvttps2ibs_round_epi8(__m512i __S, __mmask16 __A, __m512 __B) {
  return _mm512_mask_cvttps2ibs_round_epi8(__S, __A, __B, 4, _MM_FROUND_NO_EXC); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_maskz_cvttps2ibs_round_epi8(__mmask16 __A, __m512 __B) {
  return _mm512_maskz_cvttps2ibs_round_epi8(__A, __B, 4, _MM_FROUND_NO_EXC); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_cvttps2iubs_epi8(__m512 __A) {
  return _mm512_cvttps2iubs_epi8(__A, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_mask_cvttps2iubs_epi8(__m512i __S, __mmask16 __A, __m512 __B) {
  return _mm512_mask_cvttps2iubs_epi8(__S, __A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_maskz_cvttps2iubs_epi8(__mmask16 __A, __m512 __B) {
  return _mm512_maskz_cvttps2iubs_epi8(__A, __B, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_cvttps2iubs_round_epi8(__m512 __A) {
  return _mm512_cvttps2iubs_round_epi8(__A, 4, _MM_FROUND_NO_EXC); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_mask_cvttps2iubs_round_epi8(__m512i __S, __mmask16 __A, __m512 __B) {
  return _mm512_mask_cvttps2iubs_round_epi8(__S, __A, __B, 4, _MM_FROUND_NO_EXC); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}

__m512i test_mm512_maskz_cvttps2iubs_round_epi8(__mmask16 __A, __m512 __B) {
  return _mm512_maskz_cvttps2iubs_round_epi8(__A, __B, 4, _MM_FROUND_NO_EXC); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}
