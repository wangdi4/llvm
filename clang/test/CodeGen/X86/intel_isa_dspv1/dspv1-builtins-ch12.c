// REQUIRES: intel_feature_isa_dspv1
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=i686-unknown-unknown -target-feature +dspv1 -target-feature +dsp \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_dsp_psrard_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_psrard_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpsrard(<4 x i32> %{{.*}}, i32 127)
  return _mm_dsp_psrard_epi32(__A, 127);
}

__m128i test_mm_dsp_psravrd_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_psravrd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpsravrd(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_psravrd_epi32(__A, __B);
}

__m128i test_mm_dsp_pslvsd_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pslvsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpslvsd(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pslvsd_epi32(__A, __B);
}

__m128i test_mm_dsp_pslsd_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_pslsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpslsd(<4 x i32> %{{.*}}, i32 127)
  return _mm_dsp_pslsd_epi32(__A, 127);
}

__m128i test_mm_dsp_psrrud_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_psrrud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpsrrud(<4 x i32> %{{.*}}, i32 127)
  return _mm_dsp_psrrud_epi32(__A, 127);
}

__m128i test_mm_dsp_psrvrud_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_psrvrud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpsrvrud(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_psrvrud_epi32(__A, __B);
}

__m128i test_mm_dsp_pslsud_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_pslsud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpslsud(<4 x i32> %{{.*}}, i32 127)
  return _mm_dsp_pslsud_epi32(__A, 127);
}

__m128i test_mm_dsp_pslvsud_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pslvsud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpslvsud(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_dsp_pslvsud_epi32(__A, __B);
}
