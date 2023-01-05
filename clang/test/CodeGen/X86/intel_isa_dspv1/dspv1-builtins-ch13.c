// REQUIRES: intel_feature_isa_dspv1
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=i386-unknown-unknown -target-feature +dspv1 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_dsp_phaddlswuq_epi16(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_phaddlswuq_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvphaddlswuq(<8 x i16> %{{.*}})
  return _mm_dsp_phaddlswuq_epi16(__A);
}

__m128i test_mm_dsp_phaddlswq_epi16(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_phaddlswq_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvphaddlswq(<8 x i16> %{{.*}})
  return _mm_dsp_phaddlswq_epi16(__A);
}

__m128i test_mm_dsp_phaddlsduq_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_phaddlsduq_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvphaddlsduq(<4 x i32> %{{.*}})
  return _mm_dsp_phaddlsduq_epi32(__A);
}

__m128i test_mm_dsp_phaddlsdq_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_phaddlsdq_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvphaddlsdq(<4 x i32> %{{.*}})
  return _mm_dsp_phaddlsdq_epi32(__A);
}

