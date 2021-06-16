// REQUIRES: intel_feature_isa_dspv1
// RUN: %clang_cc1 %s -ffreestanding -triple=i686-unknown-unknown -target-feature +dspv1 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_dsp_plutsincosw_epi16(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_plutsincosw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvplutsincosw(<8 x i16> %{{.*}}, i32 127)
  return _mm_dsp_plutsincosw_epi16(__A, 127);
}

__m128i test_mm_dsp_pcr2bfrsw_epi16(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pcr2bfrsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpcr2bfrsw(<8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, i32 127)
  return _mm_dsp_pcr2bfrsw_epi16(__A, __B, __C, 127);
}
