// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +sse2 -emit-llvm -o - -Wall -Werror | FileCheck %s
// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +avx2 -emit-llvm -o - -Wall -Werror | FileCheck %s --check-prefixes=CHECK,CHECK-AVX2
// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +avx512f -emit-llvm -o - -Wall -Werror | FileCheck %s --check-prefixes=CHECK,CHECK-AVX2,CHECK-AVX512F

#include <immintrin.h>

// FIXME: Add all the SVML intrinsics

__m128i test_mm_irem_epi32(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_irem_epi32
  // CHECK: call <4 x i32> @__svml_irem4(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_irem_epi32(A, B);
}

__m128i test_mm_idiv_epi32(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_idiv_epi32
  // CHECK: call <4 x i32> @__svml_idiv4(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_idiv_epi32(A, B);
}

__m128i test_mm_urem_epi32(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_urem_epi32
  // CHECK: call <4 x i32> @__svml_urem4(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_urem_epi32(A, B);
}

__m128i test_mm_udiv_epi32(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_udiv_epi32
  // CHECK: call <4 x i32> @__svml_udiv4(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_udiv_epi32(A, B);
}

#ifdef __AVX2__
__m256i test_mm256_rem_epi32(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_rem_epi32
  // CHECK-AVX2: call <8 x i32> @__svml_irem8(<8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_rem_epi32(A, B);
}

__m256i test_mm256_div_epi32(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_div_epi32
  // CHECK-AVX2: call <8 x i32> @__svml_idiv8(<8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_div_epi32(A, B);
}

__m256i test_mm256_rem_epu32(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_rem_epu32
  // CHECK-AVX2: call <8 x i32> @__svml_urem8(<8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_rem_epu32(A, B);
}

__m256i test_mm256_div_epu32(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_div_epu32
  // CHECK-AVX2: call <8 x i32> @__svml_udiv8(<8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_div_epu32(A, B);
}
#endif // __AVX2__

#ifdef __AVX512F__
__m512i test_mm512_rem_epi32(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_rem_epi32
  // CHECK-AVX512F: call <16 x i32> @__svml_irem16(<16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  return _mm512_rem_epi32(A, B);
}

__m512i test_mm512_div_epi32(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_div_epi32
  // CHECK-AVX512F: call <16 x i32> @__svml_idiv16(<16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  return _mm512_div_epi32(A, B);
}

__m512i test_mm512_rem_epu32(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_rem_epu32
  // CHECK-AVX512F: call <16 x i32> @__svml_urem16(<16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  return _mm512_rem_epu32(A, B);
}

__m512i test_mm512_div_epu32(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_div_epu32
  // CHECK-AVX512F: call <16 x i32> @__svml_udiv16(<16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  return _mm512_div_epu32(A, B);
}
#endif // __AVX512F__
