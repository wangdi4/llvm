// REQUIRES: intel_feature_isa_dspv1
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=i686-unknown-unknown -target-feature +dspv1\
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_dsp_pdpwuuq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpwuuq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpdpwuuq(<2 x i64> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_dsp_pdpwuuq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pdpwssq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpwssq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpdpwssq(<2 x i64> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_dsp_pdpwssq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pdpwsuq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpwsuq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpdpwsuq(<2 x i64> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_dsp_pdpwsuq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pdpwusq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpwusq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpdpwusq(<2 x i64> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_dsp_pdpwusq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pndpwssq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pndpwssq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpndpwssq(<2 x i64> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_dsp_pndpwssq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pcmulwrs_epi16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pcmulwrs_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpcmulwrs(<8 x i16> %{{.*}}, <8 x i16> %{{.*}}, i32 127)
  return _mm_dsp_pcmulwrs_epi16(__A, __B, 127);
}

__m128i test_mm_dsp_pccmulwrs_epi16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pccmulwrs_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpccmulwrs(<8 x i16> %{{.*}}, <8 x i16> %{{.*}}, i32 127)
  return _mm_dsp_pccmulwrs_epi16(__A, __B, 127);
}

__m128i test_mm_dsp_pcdpwqre_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pcdpwqre_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpcdpwqre(<2 x i64> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_dsp_pcdpwqre_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pcdpwqimm_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pcdpwqimm_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpcdpwqimm(<2 x i64> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_dsp_pcdpwqimm_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pncdpwqre_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pncdpwqre_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpncdpwqre(<2 x i64> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_dsp_pncdpwqre_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pncdpwqimm_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pncdpwqimm_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpncdpwqimm(<2 x i64> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_dsp_pncdpwqimm_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pccdpwqimm_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pccdpwqimm_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpccdpwqimm(<2 x i64> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_dsp_pccdpwqimm_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pnccdpwqimm_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pnccdpwqimm_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpnccdpwqimm(<2 x i64> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_dsp_pnccdpwqimm_epi64(__A, __B, __C);
}
