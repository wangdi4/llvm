// REQUIRES: intel_feature_isa_dspv1
// RUN: %clang_cc1 %s -ffreestanding -triple=i686-unknown-unknown -target-feature +dspv1 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_dsp_pmuludhhq_epi64(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pmuludhhq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpmuludhhq(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pmuludhhq_epi64(__A, __B);
}

__m128i test_mm_dsp_pmuldhhq_epi64(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pmuldhhq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpmuldhhq(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pmuldhhq_epi64(__A, __B);
}

__m128i test_mm_dsp_pmuldfrs_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pmuldfrs_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpmuldfrs(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pmuldfrs_epi32(__A, __B);
}

__m128i test_mm_dsp_pmulds_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pmulds_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpmulds(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pmulds_epi32(__A, __B);
}

__m128i test_mm_dsp_pmacudllsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pmacudllsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpmacudllsq(<2 x i64> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pmacudllsq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pmacudhhsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pmacudhhsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpmacudhhsq(<2 x i64> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pmacudhhsq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pmacudllq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pmacudllq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpmacudllq(<2 x i64> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pmacudllq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pmacudhhq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pmacudhhq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpmacudhhq(<2 x i64> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pmacudhhq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pmacdllsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pmacdllsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpmacdllsq(<2 x i64> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pmacdllsq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pmacdhhsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pmacdhhsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpmacdhhsq(<2 x i64> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pmacdhhsq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pmacdllq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pmacdllq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpmacdllq(<2 x i64> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pmacdllq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pmacdhhq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pmacdhhq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpmacdhhq(<2 x i64> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pmacdhhq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pnmacdllsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pnmacdllsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpnmacdllsq(<2 x i64> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pnmacdllsq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pnmacdhhsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pnmacdhhsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpnmacdhhsq(<2 x i64> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pnmacdhhsq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pnmacdllq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pnmacdllq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpnmacdllq(<2 x i64> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pnmacdllq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pnmacdhhq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pnmacdhhq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpnmacdhhq(<2 x i64> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pnmacdhhq_epi64(__A, __B, __C);
}

__m128i test_mm_dsp_pmsubadddllq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pmsubadddllq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.dvpmsubadddllq(<2 x i64> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pmsubadddllq_epi64(__A, __B, __C);
}
