// REQUIRES: intel_feature_isa_avx512_vnni_fp8
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +avx512vnnifp8 -target-feature +avx512vl \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128 test_mm_dpbf8_ps(__m128 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dpbf8_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512vnnifp8.vdpbf8ps128(
  return _mm_dpbf8_ps(__A, __B, __C);
}

__m128 test_mm_mask_dpbf8_ps(__m128 __A, __mmask8 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_mask_dpbf8_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512vnnifp8.vdpbf8ps128(
  // CHECK: select <4 x i1> %{{.*}}, <4 x float> %{{.*}}, <4 x float> %{{.*}}
  return _mm_mask_dpbf8_ps(__A, __B, __C, __D);
}

__m128 test_mm_maskz_dpbf8_ps(__mmask8 __A, __m128 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_maskz_dpbf8_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512vnnifp8.vdpbf8ps128(
  // CHECK: zeroinitializer
  // CHECK: select <4 x i1> %{{.*}}, <4 x float> %{{.*}}, <4 x float> %{{.*}}
  return _mm_maskz_dpbf8_ps(__A, __B, __C, __D);
}

__m256 test_mm256_dpbf8_ps(__m256 __A, __m256i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_dpbf8_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512vnnifp8.vdpbf8ps256(
  return _mm256_dpbf8_ps(__A, __B, __C);
}

__m256 test_mm256_mask_dpbf8_ps(__m256 __A, __mmask8 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_mask_dpbf8_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512vnnifp8.vdpbf8ps256(
  // CHECK: select <8 x i1> %{{.*}}, <8 x float> %{{.*}}, <8 x float> %{{.*}}
  return _mm256_mask_dpbf8_ps(__A, __B, __C, __D);
}

__m256 test_mm256_maskz_dpbf8_ps(__mmask8 __A, __m256 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_maskz_dpbf8_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512vnnifp8.vdpbf8ps256(
  // CHECK: zeroinitializer
  // CHECK: select <8 x i1> %{{.*}}, <8 x float> %{{.*}}, <8 x float> %{{.*}}
  return _mm256_maskz_dpbf8_ps(__A, __B, __C, __D);
}

__m128 test_mm_dpbhf8_ps(__m128 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dpbhf8_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512vnnifp8.vdpbhf8ps128(
  return _mm_dpbhf8_ps(__A, __B, __C);
}

__m128 test_mm_mask_dpbhf8_ps(__m128 __A, __mmask8 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_mask_dpbhf8_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512vnnifp8.vdpbhf8ps128(
  // CHECK: select <4 x i1> %{{.*}}, <4 x float> %{{.*}}, <4 x float> %{{.*}}
  return _mm_mask_dpbhf8_ps(__A, __B, __C, __D);
}

__m128 test_mm_maskz_dpbhf8_ps(__mmask8 __A, __m128 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_maskz_dpbhf8_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512vnnifp8.vdpbhf8ps128(
  // CHECK: zeroinitializer
  // CHECK: select <4 x i1> %{{.*}}, <4 x float> %{{.*}}, <4 x float> %{{.*}}
  return _mm_maskz_dpbhf8_ps(__A, __B, __C, __D);
}

__m256 test_mm256_dpbhf8_ps(__m256 __A, __m256i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_dpbhf8_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512vnnifp8.vdpbhf8ps256(
  return _mm256_dpbhf8_ps(__A, __B, __C);
}

__m256 test_mm256_mask_dpbhf8_ps(__m256 __A, __mmask8 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_mask_dpbhf8_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512vnnifp8.vdpbhf8ps256(
  // CHECK: select <8 x i1> %{{.*}}, <8 x float> %{{.*}}, <8 x float> %{{.*}}
  return _mm256_mask_dpbhf8_ps(__A, __B, __C, __D);
}

__m256 test_mm256_maskz_dpbhf8_ps(__mmask8 __A, __m256 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_maskz_dpbhf8_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512vnnifp8.vdpbhf8ps256(
  // CHECK: zeroinitializer
  // CHECK: select <8 x i1> %{{.*}}, <8 x float> %{{.*}}, <8 x float> %{{.*}}
  return _mm256_maskz_dpbhf8_ps(__A, __B, __C, __D);
}

__m128 test_mm_dphbf8_ps(__m128 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dphbf8_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512vnnifp8.vdphbf8ps128(
  return _mm_dphbf8_ps(__A, __B, __C);
}

__m128 test_mm_mask_dphbf8_ps(__m128 __A, __mmask8 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_mask_dphbf8_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512vnnifp8.vdphbf8ps128(
  // CHECK: select <4 x i1> %{{.*}}, <4 x float> %{{.*}}, <4 x float> %{{.*}}
  return _mm_mask_dphbf8_ps(__A, __B, __C, __D);
}

__m128 test_mm_maskz_dphbf8_ps(__mmask8 __A, __m128 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_maskz_dphbf8_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512vnnifp8.vdphbf8ps128(
  // CHECK: zeroinitializer
  // CHECK: select <4 x i1> %{{.*}}, <4 x float> %{{.*}}, <4 x float> %{{.*}}
  return _mm_maskz_dphbf8_ps(__A, __B, __C, __D);
}

__m256 test_mm256_dphbf8_ps(__m256 __A, __m256i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_dphbf8_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512vnnifp8.vdphbf8ps256(
  return _mm256_dphbf8_ps(__A, __B, __C);
}

__m256 test_mm256_mask_dphbf8_ps(__m256 __A, __mmask8 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_mask_dphbf8_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512vnnifp8.vdphbf8ps256(
  // CHECK: select <8 x i1> %{{.*}}, <8 x float> %{{.*}}, <8 x float> %{{.*}}
  return _mm256_mask_dphbf8_ps(__A, __B, __C, __D);
}

__m256 test_mm256_maskz_dphbf8_ps(__mmask8 __A, __m256 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_maskz_dphbf8_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512vnnifp8.vdphbf8ps256(
  // CHECK: zeroinitializer
  // CHECK: select <8 x i1> %{{.*}}, <8 x float> %{{.*}}, <8 x float> %{{.*}}
  return _mm256_maskz_dphbf8_ps(__A, __B, __C, __D);
}

__m128 test_mm_dphf8_ps(__m128 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dphf8_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512vnnifp8.vdphf8ps128(
  return _mm_dphf8_ps(__A, __B, __C);
}

__m128 test_mm_mask_dphf8_ps(__m128 __A, __mmask8 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_mask_dphf8_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512vnnifp8.vdphf8ps128(
  // CHECK: select <4 x i1> %{{.*}}, <4 x float> %{{.*}}, <4 x float> %{{.*}}
  return _mm_mask_dphf8_ps(__A, __B, __C, __D);
}

__m128 test_mm_maskz_dphf8_ps(__mmask8 __A, __m128 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_maskz_dphf8_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512vnnifp8.vdphf8ps128(
  // CHECK: zeroinitializer
  // CHECK: select <4 x i1> %{{.*}}, <4 x float> %{{.*}}, <4 x float> %{{.*}}
  return _mm_maskz_dphf8_ps(__A, __B, __C, __D);
}

__m256 test_mm256_dphf8_ps(__m256 __A, __m256i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_dphf8_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512vnnifp8.vdphf8ps256(
  return _mm256_dphf8_ps(__A, __B, __C);
}

__m256 test_mm256_mask_dphf8_ps(__m256 __A, __mmask8 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_mask_dphf8_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512vnnifp8.vdphf8ps256(
  // CHECK: select <8 x i1> %{{.*}}, <8 x float> %{{.*}}, <8 x float> %{{.*}}
  return _mm256_mask_dphf8_ps(__A, __B, __C, __D);
}

__m256 test_mm256_maskz_dphf8_ps(__mmask8 __A, __m256 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_maskz_dphf8_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512vnnifp8.vdphf8ps256(
  // CHECK: zeroinitializer
  // CHECK: select <8 x i1> %{{.*}}, <8 x float> %{{.*}}, <8 x float> %{{.*}}
  return _mm256_maskz_dphf8_ps(__A, __B, __C, __D);
}

