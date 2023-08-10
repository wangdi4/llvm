// REQUIRES: intel_feature_isa_avx512_vnni_int16
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512vnniint16 -target-feature +avx512vl \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_dpwsud_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dpwsud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwsud.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dpwsud_epi32(__A, __B, __C);
}

__m128i test_mm_mask_dpwsud_epi32(__m128i __A, __mmask8 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_mask_dpwsud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwsud.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  // CHECK: select <4 x i1> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}}
  return _mm_mask_dpwsud_epi32(__A, __B, __C, __D);
}

__m128i test_mm_maskz_dpwsud_epi32(__m128i __A, __mmask8 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_maskz_dpwsud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwsud.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  // CHECK: select <4 x i1> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}}
  return _mm_maskz_dpwsud_epi32(__A, __B, __C, __D);
}

__m256i test_mm256_dpwsud_epi32(__m256i __A, __m256i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_dpwsud_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwsud.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_dpwsud_epi32(__A, __B, __C);
}

__m256i test_mm256_mask_dpwsud_epi32(__m256i __A, __mmask8 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_mask_dpwsud_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwsud.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  // CHECK: select <8 x i1> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}}
  return _mm256_mask_dpwsud_epi32(__A, __B, __C, __D);
}

__m256i test_mm256_maskz_dpwsud_epi32(__m256i __A, __mmask8 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_maskz_dpwsud_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwsud.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  // CHECK: select <8 x i1> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}}
  return _mm256_maskz_dpwsud_epi32(__A, __B, __C, __D);
}

__m128i test_mm_dpwsuds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dpwsuds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwsuds.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dpwsuds_epi32(__A, __B, __C);
}

__m128i test_mm_mask_dpwsuds_epi32(__m128i __A, __mmask8 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_mask_dpwsuds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwsuds.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  // CHECK: select <4 x i1> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}}
  return _mm_mask_dpwsuds_epi32(__A, __B, __C, __D);
}

__m128i test_mm_maskz_dpwsuds_epi32(__m128i __A, __mmask8 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_maskz_dpwsuds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwsuds.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  // CHECK: select <4 x i1> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}}
  return _mm_maskz_dpwsuds_epi32(__A, __B, __C, __D);
}

__m256i test_mm256_dpwsuds_epi32(__m256i __A, __m256i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_dpwsuds_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwsuds.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_dpwsuds_epi32(__A, __B, __C);
}

__m256i test_mm256_mask_dpwsuds_epi32(__m256i __A, __mmask8 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_mask_dpwsuds_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwsuds.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  // CHECK: select <8 x i1> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}}
  return _mm256_mask_dpwsuds_epi32(__A, __B, __C, __D);
}

__m256i test_mm256_maskz_dpwsuds_epi32(__m256i __A, __mmask8 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_maskz_dpwsuds_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwsuds.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  // CHECK: select <8 x i1> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}}
  return _mm256_maskz_dpwsuds_epi32(__A, __B, __C, __D);
}

__m128i test_mm_dpwusd_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dpwusd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwusd.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dpwusd_epi32(__A, __B, __C);
}

__m128i test_mm_mask_dpwusd_epi32(__m128i __A, __mmask8 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_mask_dpwusd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwusd.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  // CHECK: select <4 x i1> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}}
  return _mm_mask_dpwusd_epi32(__A, __B, __C, __D);
}

__m128i test_mm_maskz_dpwusd_epi32(__m128i __A, __mmask8 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_maskz_dpwusd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwusd.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  // CHECK: select <4 x i1> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}}
  return _mm_maskz_dpwusd_epi32(__A, __B, __C, __D);
}

__m256i test_mm256_dpwusd_epi32(__m256i __A, __m256i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_dpwusd_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwusd.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_dpwusd_epi32(__A, __B, __C);
}

__m256i test_mm256_mask_dpwusd_epi32(__m256i __A, __mmask8 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_mask_dpwusd_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwusd.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  // CHECK: select <8 x i1> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}}
  return _mm256_mask_dpwusd_epi32(__A, __B, __C, __D);
}

__m256i test_mm256_maskz_dpwusd_epi32(__m256i __A, __mmask8 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_maskz_dpwusd_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwusd.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  // CHECK: select <8 x i1> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}}
  return _mm256_maskz_dpwusd_epi32(__A, __B, __C, __D);
}

__m128i test_mm_dpwusds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dpwusds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwusds.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dpwusds_epi32(__A, __B, __C);
}

__m128i test_mm_mask_dpwusds_epi32(__m128i __A, __mmask8 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_mask_dpwusds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwusds.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  // CHECK: select <4 x i1> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}}
  return _mm_mask_dpwusds_epi32(__A, __B, __C, __D);
}

__m128i test_mm_maskz_dpwusds_epi32(__m128i __A, __mmask8 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_maskz_dpwusds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwusds.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  // CHECK: select <4 x i1> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}}
  return _mm_maskz_dpwusds_epi32(__A, __B, __C, __D);
}

__m256i test_mm256_dpwusds_epi32(__m256i __A, __m256i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_dpwusds_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwusds.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_dpwusds_epi32(__A, __B, __C);
}

__m256i test_mm256_mask_dpwusds_epi32(__m256i __A, __mmask8 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_mask_dpwusds_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwusds.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  // CHECK: select <8 x i1> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}}
  return _mm256_mask_dpwusds_epi32(__A, __B, __C, __D);
}

__m256i test_mm256_maskz_dpwusds_epi32(__m256i __A, __mmask8 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_maskz_dpwusds_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwusds.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  // CHECK: select <8 x i1> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}}
  return _mm256_maskz_dpwusds_epi32(__A, __B, __C, __D);
}

__m128i test_mm_dpwuud_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dpwuud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwuud.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dpwuud_epi32(__A, __B, __C);
}

__m128i test_mm_mask_dpwuud_epi32(__m128i __A, __mmask8 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_mask_dpwuud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwuud.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  // CHECK: select <4 x i1> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}}
  return _mm_mask_dpwuud_epi32(__A, __B, __C, __D);
}

__m128i test_mm_maskz_dpwuud_epi32(__m128i __A, __mmask8 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_maskz_dpwuud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwuud.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  // CHECK: select <4 x i1> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}}
  return _mm_maskz_dpwuud_epi32(__A, __B, __C, __D);
}

__m256i test_mm256_dpwuud_epi32(__m256i __A, __m256i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_dpwuud_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwuud.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_dpwuud_epi32(__A, __B, __C);
}

__m256i test_mm256_mask_dpwuud_epi32(__m256i __A, __mmask8 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_mask_dpwuud_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwuud.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  // CHECK: select <8 x i1> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}}
  return _mm256_mask_dpwuud_epi32(__A, __B, __C, __D);
}

__m256i test_mm256_maskz_dpwuud_epi32(__m256i __A, __mmask8 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_maskz_dpwuud_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwuud.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  // CHECK: select <8 x i1> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}}
  return _mm256_maskz_dpwuud_epi32(__A, __B, __C, __D);
}

__m128i test_mm_dpwuuds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dpwuuds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwuuds.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dpwuuds_epi32(__A, __B, __C);
}

__m128i test_mm_mask_dpwuuds_epi32(__m128i __A, __mmask8 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_mask_dpwuuds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwuuds.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  // CHECK: select <4 x i1> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}}
  return _mm_mask_dpwuuds_epi32(__A, __B, __C, __D);
}

__m128i test_mm_maskz_dpwuuds_epi32(__m128i __A, __mmask8 __B, __m128i __C, __m128i __D) {
  // CHECK-LABEL: @test_mm_maskz_dpwuuds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpdpwuuds.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  // CHECK: select <4 x i1> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}}
  return _mm_maskz_dpwuuds_epi32(__A, __B, __C, __D);
}

__m256i test_mm256_dpwuuds_epi32(__m256i __A, __m256i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_dpwuuds_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwuuds.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_dpwuuds_epi32(__A, __B, __C);
}

__m256i test_mm256_mask_dpwuuds_epi32(__m256i __A, __mmask8 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_mask_dpwuuds_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwuuds.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  // CHECK: select <8 x i1> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}}
  return _mm256_mask_dpwuuds_epi32(__A, __B, __C, __D);
}

__m256i test_mm256_maskz_dpwuuds_epi32(__m256i __A, __mmask8 __B, __m256i __C, __m256i __D) {
  // CHECK-LABEL: @test_mm256_maskz_dpwuuds_epi32(
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpdpwuuds.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  // CHECK: select <8 x i1> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}}
  return _mm256_maskz_dpwuuds_epi32(__A, __B, __C, __D);
}
