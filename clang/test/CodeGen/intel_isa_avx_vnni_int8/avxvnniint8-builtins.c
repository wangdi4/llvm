// REQUIRES: intel_feature_isa_avx_vnni_int8
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-unknown-unknown -target-feature +avxvnniint8 -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

// CHECK-LABEL: @test_mm_vpdpbssd_epi32(
// CHECK:     call <4 x i32> @llvm.x86.avx2.vpdpbssd.128
__m128i test_mm_vpdpbssd_epi32(__m128i __W, __m128i __A, __m128i __B) {
  return _mm_vpdpbssd_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm_vpdpbssds_epi32(
// CHECK:     call <4 x i32> @llvm.x86.avx2.vpdpbssds.128
__m128i test_mm_vpdpbssds_epi32(__m128i __W, __m128i __A, __m128i __B) {
  return _mm_vpdpbssds_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm_vpdpbsud_epi32(
// CHECK:     call <4 x i32> @llvm.x86.avx2.vpdpbsud.128
__m128i test_mm_vpdpbsud_epi32(__m128i __W, __m128i __A, __m128i __B) {
  return _mm_vpdpbsud_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm_vpdpbsuds_epi32(
// CHECK:     call <4 x i32> @llvm.x86.avx2.vpdpbsuds.128
__m128i test_mm_vpdpbsuds_epi32(__m128i __W, __m128i __A, __m128i __B) {
  return _mm_vpdpbsuds_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm_vpdpbuud_epi32(
// CHECK:     call <4 x i32> @llvm.x86.avx2.vpdpbuud.128
__m128i test_mm_vpdpbuud_epi32(__m128i __W, __m128i __A, __m128i __B) {
  return _mm_vpdpbuud_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm_vpdpbuuds_epi32(
// CHECK:     call <4 x i32> @llvm.x86.avx2.vpdpbuuds.128
__m128i test_mm_vpdpbuuds_epi32(__m128i __W, __m128i __A, __m128i __B) {
  return _mm_vpdpbuuds_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm256_vpdpbssd_epi32(
// CHECK:     call <8 x i32> @llvm.x86.avx2.vpdpbssd.256
__m256i test_mm256_vpdpbssd_epi32(__m256i __W, __m256i __A, __m256i __B) {
  return _mm256_vpdpbssd_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm256_vpdpbssds_epi32(
// CHECK:     call <8 x i32> @llvm.x86.avx2.vpdpbssds.256
__m256i test_mm256_vpdpbssds_epi32(__m256i __W, __m256i __A, __m256i __B) {
  return _mm256_vpdpbssds_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm256_vpdpbsud_epi32(
// CHECK:     call <8 x i32> @llvm.x86.avx2.vpdpbsud.256
__m256i test_mm256_vpdpbsud_epi32(__m256i __W, __m256i __A, __m256i __B) {
  return _mm256_vpdpbsud_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm256_vpdpbsuds_epi32(
// CHECK:     call <8 x i32> @llvm.x86.avx2.vpdpbsuds.256
__m256i test_mm256_vpdpbsuds_epi32(__m256i __W, __m256i __A, __m256i __B) {
  return _mm256_vpdpbsuds_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm256_vpdpbuud_epi32(
// CHECK:     call <8 x i32> @llvm.x86.avx2.vpdpbuud.256
__m256i test_mm256_vpdpbuud_epi32(__m256i __W, __m256i __A, __m256i __B) {
  return _mm256_vpdpbuud_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm256_vpdpbuuds_epi32(
// CHECK:     call <8 x i32> @llvm.x86.avx2.vpdpbuuds.256
__m256i test_mm256_vpdpbuuds_epi32(__m256i __W, __m256i __A, __m256i __B) {
  return _mm256_vpdpbuuds_epi32(__W, __A, __B);
}
