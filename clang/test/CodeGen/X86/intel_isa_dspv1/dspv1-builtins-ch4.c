// REQUIRES: intel_feature_isa_dspv1
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=i386-unknown-unknown -target-feature +dspv1 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_dsp_pdpbwuud_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpbwuud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpbwuud(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <8 x i16> %{{.*}}, i32 127)
  return _mm_dsp_pdpbwuud_epi32(__A, __B, __C, 127);
}

__m128i test_mm_dsp_pdpbwuuds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpbwuuds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpbwuuds(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <8 x i16> %{{.*}}, i32 127)
  return _mm_dsp_pdpbwuuds_epi32(__A, __B, __C, 127);
}

__m128i test_mm_dsp_pdpbwssd_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpbwssd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpbwssd(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <8 x i16> %{{.*}}, i32 127)
  return _mm_dsp_pdpbwssd_epi32(__A, __B, __C, 127);
}

__m128i test_mm_dsp_pdpbwssds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpbwssds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpbwssds(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <8 x i16> %{{.*}}, i32 127)
  return _mm_dsp_pdpbwssds_epi32(__A, __B, __C, 127);
}

__m128i test_mm_dsp_pdpbwsud_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpbwsud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpbwsud(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <8 x i16> %{{.*}}, i32 127)
  return _mm_dsp_pdpbwsud_epi32(__A, __B, __C, 127);
}

__m128i test_mm_dsp_pdpbwsuds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpbwsuds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpbwsuds(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <8 x i16> %{{.*}}, i32 127)
  return _mm_dsp_pdpbwsuds_epi32(__A, __B, __C, 127);
}

__m128i test_mm_dsp_pdpbwusd_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpbwusd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpbwusd(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <8 x i16> %{{.*}}, i32 127)
  return _mm_dsp_pdpbwusd_epi32(__A, __B, __C, 127);
}

__m128i test_mm_dsp_pdpbwusds_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpbwusds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpbwusds(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <8 x i16> %{{.*}}, i32 127)
  return _mm_dsp_pdpbwusds_epi32(__A, __B, __C, 127);
}

