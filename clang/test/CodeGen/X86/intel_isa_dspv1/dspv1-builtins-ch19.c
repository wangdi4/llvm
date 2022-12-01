// REQUIRES: intel_feature_isa_dspv1
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=i386-unknown-unknown -target-feature +dspv1 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_alignr_epi8(__m128i a, __m128i b) {
  // CHECK-LABEL: test_mm_alignr_epi8
  // CHECK: shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17>
  return _mm_alignr_epi8(a, b, 2);
}

__m128i test_mm_max_epi8(__m128i x, __m128i y) {
  // CHECK-LABEL: test_mm_max_epi8
  // CHECK: call <16 x i8> @llvm.smax.v16i8(<16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_max_epi8(x, y);
}

__m128i test_mm_max_epu8(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_max_epu8
  // CHECK: call <16 x i8> @llvm.umax.v16i8(<16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_max_epu8(A, B);
}

__m128i test_mm_max_epi16(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_max_epi16
  // CHECK: call <8 x i16> @llvm.smax.v8i16(<8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_max_epi16(A, B);
}

__m128i test_mm_max_epu16(__m128i x, __m128i y) {
  // CHECK-LABEL: test_mm_max_epu16
  // CHECK: call <8 x i16> @llvm.umax.v8i16(<8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_max_epu16(x, y);
}

__m128i test_mm_max_epi32(__m128i x, __m128i y) {
  // CHECK-LABEL: test_mm_max_epi32
  // CHECK: call <4 x i32> @llvm.smax.v4i32(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_max_epi32(x, y);
}

__m128i test_mm_max_epu32(__m128i x, __m128i y) {
  // CHECK-LABEL: test_mm_max_epu32
  // CHECK: call <4 x i32> @llvm.umax.v4i32(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_max_epu32(x, y);
}

__m128i test_mm_min_epi8(__m128i x, __m128i y) {
  // CHECK-LABEL: test_mm_min_epi8
  // CHECK: call <16 x i8> @llvm.smin.v16i8(<16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_min_epi8(x, y);
}

__m128i test_mm_min_epu8(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_min_epu8
  // CHECK: call <16 x i8> @llvm.umin.v16i8(<16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_min_epu8(A, B);
}

__m128i test_mm_min_epi16(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_min_epi16
  // CHECK: call <8 x i16> @llvm.smin.v8i16(<8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_min_epi16(A, B);
}

__m128i test_mm_min_epu16(__m128i x, __m128i y) {
  // CHECK-LABEL: test_mm_min_epu16
  // CHECK: call <8 x i16> @llvm.umin.v8i16(<8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_min_epu16(x, y);
}

__m128i test_mm_min_epi32(__m128i x, __m128i y) {
  // CHECK-LABEL: test_mm_min_epi32
  // CHECK: call <4 x i32> @llvm.smin.v4i32(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_min_epi32(x, y);
}

__m128i test_mm_min_epu32(__m128i x, __m128i y) {
  // CHECK-LABEL: test_mm_min_epu32
  // CHECK: call <4 x i32> @llvm.umin.v4i32(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_min_epu32(x, y);
}
