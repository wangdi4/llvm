// REQUIRES: intel_feature_isa_dspv1
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=i686-unknown-unknown -target-feature +dspv1 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_dsp_pdpbuud_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpbuud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpbuud(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_dsp_pdpbuud_epi32(__A, __B, __C);
}

__m128i test_mm_dsp_pdpbuuds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpbuuds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpbuuds(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_dsp_pdpbuuds_epi32(__A, __B, __C);
}

__m128i test_mm_dsp_pdpbssd_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpbssd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpbssd(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_dsp_pdpbssd_epi32(__A, __B, __C);
}

__m128i test_mm_dsp_pdpbssds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpbssds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpbssds(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_dsp_pdpbssds_epi32(__A, __B, __C);
}

__m128i test_mm_dsp_pdpbsud_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpbsud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpbsud(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_dsp_pdpbsud_epi32(__A, __B, __C);
}

__m128i test_mm_dsp_pdpbsuds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpbsuds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpbsuds(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_dsp_pdpbsuds_epi32(__A, __B, __C);
}

__m128i test_mm_dsp_pndpbssd_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pndpbssd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpndpbssd(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_dsp_pndpbssd_epi32(__A, __B, __C);
}

__m128i test_mm_dpbusd_epi32(__m128i __S, __m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dpbusd_epi32
  // CHECK: @llvm.x86.avx512.vpdpbusd.128
  return _mm_dpbusd_epi32(__S, __A, __B);
}

__m128i test_mm_dpbusds_epi32(__m128i __S, __m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dpbusds_epi32
  // CHECK: @llvm.x86.avx512.vpdpbusds.128
  return _mm_dpbusds_epi32(__S, __A, __B);
}
