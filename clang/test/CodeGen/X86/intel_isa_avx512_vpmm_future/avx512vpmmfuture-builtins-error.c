// REQUIRES: intel_feature_isa_avx512_vpmm
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +avx512vpmm -target-feature +avx512vl -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>

__m128 test_mm_vmmif16_ps(__m128h Src1, __m128h Src2, __m128 SrcDst) {
  return _mm_vmmif16_ps(Src1, Src2, SrcDst, 1); // expected-error {{argument value 1 is outside the valid range [0, 0]}}
}

__m256 test_mm256_vmmif16_ps(__m256h Src1, __m256h Src2, __m256 SrcDst) {
  return _mm256_vmmif16_ps(Src1, Src2, SrcDst, 2); // expected-error {{argument value 2 is outside the valid range [0, 1]}}
}

__m512 test_mm512_vmmif16_ps(__m512h Src1, __m512h Src2, __m512 SrcDst) {
  return _mm512_vmmif16_ps(Src1, Src2, SrcDst, 4); // expected-error {{argument value 4 is outside the valid range [0, 3]}}
}
