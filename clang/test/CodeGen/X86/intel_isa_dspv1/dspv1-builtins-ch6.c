// REQUIRES: intel_feature_isa_dspv1
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=i386-unknown-unknown -target-feature +dspv1 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_dsp_pmuluwdq_epi64(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pmuluwdq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpmuluwdq(<8 x i16> %{{.*}}, <4 x i32> %{{.*}}, i32 127)
  return _mm_dsp_pmuluwdq_epi64(__A, __B, 127);
}

__m128i test_mm_dsp_pmulwdq_epi64(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pmulwdq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpmulwdq(<8 x i16> %{{.*}}, <4 x i32> %{{.*}}, i32 127)
  return _mm_dsp_pmulwdq_epi64(__A, __B, 127);
}

__m128i test_mm_dsp_pdpwduuq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpwduuq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpdpwduuq(<2 x i64> %{{.*}}, <8 x i16> %{{.*}}, <4 x i32> %{{.*}}, i32 127)
  return _mm_dsp_pdpwduuq_epi64(__A, __B, __C, 127);
}

__m128i test_mm_dsp_pdpwdssq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpwdssq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpdpwdssq(<2 x i64> %{{.*}}, <8 x i16> %{{.*}}, <4 x i32> %{{.*}}, i32 127)
  return _mm_dsp_pdpwdssq_epi64(__A, __B, __C, 127);
}

__m128i test_mm_dsp_pcr2bfrsdre_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pcr2bfrsdre_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpcr2bfrsdre(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <8 x i16> %{{.*}}, i32 127)
  return _mm_dsp_pcr2bfrsdre_epi32(__A, __B, __C, 127);
}

__m128i test_mm_dsp_pcr2bfrsdimm_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pcr2bfrsdimm_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpcr2bfrsdimm(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <8 x i16> %{{.*}}, i32 127)
  return _mm_dsp_pcr2bfrsdimm_epi32(__A, __B, __C, 127);
}

