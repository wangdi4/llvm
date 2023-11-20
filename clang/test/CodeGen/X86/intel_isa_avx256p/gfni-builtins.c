// REQUIRES: intel_feature_isa_avx256p_unsupported
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +avx256p -emit-llvm -o - | FileCheck %s

#include <immintrin.h>

__m128i test_mm_gf2p8affineinv_epi64_epi8(__m128i A, __m128i B) {
  // CHECK-LABEL: @test_mm_gf2p8affineinv_epi64_epi8
  // CHECK: @llvm.x86.vgf2p8affineinvqb.128
  return _mm_gf2p8affineinv_epi64_epi8(A, B, 1);
}

__m128i test_mm_gf2p8affine_epi64_epi8(__m128i A, __m128i B) {
  // CHECK-LABEL: @test_mm_gf2p8affine_epi64_epi8
  // CHECK: @llvm.x86.vgf2p8affineqb.128
  return _mm_gf2p8affine_epi64_epi8(A, B, 1);
}

__m128i test_mm_gf2p8mul_epi8(__m128i A, __m128i B) {
  // CHECK-LABEL: @test_mm_gf2p8mul_epi8
  // CHECK: @llvm.x86.vgf2p8mulb.128
  return _mm_gf2p8mul_epi8(A, B);
}

__m256i test_mm256_gf2p8affineinv_epi64_epi8(__m256i A, __m256i B) {
  // CHECK-LABEL: @test_mm256_gf2p8affineinv_epi64_epi8
  // CHECK: @llvm.x86.vgf2p8affineinvqb.256
  return _mm256_gf2p8affineinv_epi64_epi8(A, B, 1);
}

__m256i test_mm256_gf2p8affine_epi64_epi8(__m256i A, __m256i B) {
  // CHECK-LABEL: @test_mm256_gf2p8affine_epi64_epi8
  // CHECK: @llvm.x86.vgf2p8affineqb.256
  return _mm256_gf2p8affine_epi64_epi8(A, B, 1);
}

__m256i test_mm256_gf2p8mul_epi8(__m256i A, __m256i B) {
  // CHECK-LABEL: @test_mm256_gf2p8mul_epi8
  // CHECK: @llvm.x86.vgf2p8mulb.256
  return _mm256_gf2p8mul_epi8(A, B);
}
