// REQUIRES: intel_feature_isa_vpinsr_vpextr
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +vpinsr-vpextr \
// RUN:  -emit-llvm -fsyntax-only -verify
#include <immintrin.h>

int test_mm512_extract_epi8(__m512i A) {
  return _mm512_extract_epi8(A, 64); // expected-error {{argument value 64 is outside the valid range [0, 63]}}
}

int test_mm512_extract_epi16(__m512i A) {
  return _mm512_extract_epi16(A, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

int test_mm512_extract_epi32(__m512i A) {
  return _mm512_extract_epi32(A, 16); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

#if __x86_64__
long long test_mm512_extract_epi64(__m512i A) {
  return _mm512_extract_epi64(A, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}
#endif

__m512i test_mm512_insert_epi8(__m512i x, char b) {
  return _mm512_insert_epi8(x, b, 64); // expected-error {{argument value 64 is outside the valid range [0, 63]}}
}

__m512i test_mm512_insert_epi16(__m512i x, int b) {
  return _mm512_insert_epi16(x, b, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

__m512i test_mm512_insert_epi32(__m512i x, int b) {
  return _mm512_insert_epi32(x, b, 16); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

#if __x86_64__
__m512i test_mm512_insert_epi64(__m512i x, long long b) {
  return _mm512_insert_epi64(x, b, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}
#endif
