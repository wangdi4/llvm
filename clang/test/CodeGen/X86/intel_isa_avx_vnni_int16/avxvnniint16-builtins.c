// REQUIRES: intel_feature_isa_avx_vnni_int16
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx2 -target-feature +avxvnniint16 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_dpwsud_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dpwsud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.vpdpwsud128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dpwsud_epi32(__A, __B, __C);
}

__m256i test_mm256_dpwsud_epi32(__m256i __A, __m256i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_dpwsud_epi32(
  // CHECK: call <8 x i32> @llvm.x86.vpdpwsud256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_dpwsud_epi32(__A, __B, __C);
}

__m128i test_mm_dpwsuds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dpwsuds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.vpdpwsuds128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dpwsuds_epi32(__A, __B, __C);
}

__m256i test_mm256_dpwsuds_epi32(__m256i __A, __m256i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_dpwsuds_epi32(
  // CHECK: call <8 x i32> @llvm.x86.vpdpwsuds256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_dpwsuds_epi32(__A, __B, __C);
}

__m128i test_mm_dpwusd_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dpwusd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.vpdpwusd128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dpwusd_epi32(__A, __B, __C);
}

__m256i test_mm256_dpwusd_epi32(__m256i __A, __m256i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_dpwusd_epi32(
  // CHECK: call <8 x i32> @llvm.x86.vpdpwusd256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_dpwusd_epi32(__A, __B, __C);
}

__m128i test_mm_dpwusds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dpwusds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.vpdpwusds128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dpwusds_epi32(__A, __B, __C);
}

__m256i test_mm256_dpwusds_epi32(__m256i __A, __m256i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_dpwusds_epi32(
  // CHECK: call <8 x i32> @llvm.x86.vpdpwusds256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_dpwusds_epi32(__A, __B, __C);
}

__m128i test_mm_dpwuud_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dpwuud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.vpdpwuud128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dpwuud_epi32(__A, __B, __C);
}

__m256i test_mm256_dpwuud_epi32(__m256i __A, __m256i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_dpwuud_epi32(
  // CHECK: call <8 x i32> @llvm.x86.vpdpwuud256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_dpwuud_epi32(__A, __B, __C);
}

__m128i test_mm_dpwuuds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dpwuuds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.vpdpwuuds128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dpwuuds_epi32(__A, __B, __C);
}

__m256i test_mm256_dpwuuds_epi32(__m256i __A, __m256i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_dpwuuds_epi32(
  // CHECK: call <8 x i32> @llvm.x86.vpdpwuuds256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_dpwuuds_epi32(__A, __B, __C);
}
