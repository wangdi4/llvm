// REQUIRES: intel_feature_isa_dspv1
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=i686-unknown-unknown -target-feature +dspv1 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_dsp_psadaccubws_epi16(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_psadaccubws_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpsadaccubws(<8 x i16> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_dsp_psadaccubws_epi16(__A, __B, __C);
}

__m128i test_mm_dsp_psrrsuqw_epi16(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_psrrsuqw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpsrrsuqw(<2 x i64> %{{.*}}, i32 127)
  return _mm_dsp_psrrsuqw_epi16(__A, 127);
}

__m128i test_mm_dsp_psrvrsuqw_epi16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_psrvrsuqw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpsrvrsuqw(<2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  return _mm_dsp_psrvrsuqw_epi16(__A, __B);
}

__m128i test_mm_dsp_pslrsuqw_epi16(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_pslrsuqw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpslrsuqw(<2 x i64> %{{.*}}, i32 127)
  return _mm_dsp_pslrsuqw_epi16(__A, 127);
}

__m128i test_mm_dsp_pslvrsuqw_epi16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pslvrsuqw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpslvrsuqw(<2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  return _mm_dsp_pslvrsuqw_epi16(__A, __B);
}

__m128i test_mm_dsp_psrarsqw_epi16(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_psrarsqw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpsrarsqw(<2 x i64> %{{.*}}, i32 127)
  return _mm_dsp_psrarsqw_epi16(__A, 127);
}

__m128i test_mm_dsp_psravrsqw_epi16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_psravrsqw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpsravrsqw(<2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  return _mm_dsp_psravrsqw_epi16(__A, __B);
}

__m128i test_mm_dsp_pslrsqw_epi16(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_pslrsqw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpslrsqw(<2 x i64> %{{.*}}, i32 127)
  return _mm_dsp_pslrsqw_epi16(__A, 127);
}

__m128i test_mm_dsp_pslvrsqw_epi16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pslvrsqw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpslvrsqw(<2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  return _mm_dsp_pslvrsqw_epi16(__A, __B);
}

__m128i test_mm_dsp_psrrsuqd_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_psrrsuqd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpsrrsuqd(<2 x i64> %{{.*}}, i32 127)
  return _mm_dsp_psrrsuqd_epi32(__A, 127);
}

__m128i test_mm_dsp_psrvrsuqd_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_psrvrsuqd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpsrvrsuqd(<2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  return _mm_dsp_psrvrsuqd_epi32(__A, __B);
}

__m128i test_mm_dsp_pslrsuqd_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_pslrsuqd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpslrsuqd(<2 x i64> %{{.*}}, i32 127)
  return _mm_dsp_pslrsuqd_epi32(__A, 127);
}

__m128i test_mm_dsp_pslvrsuqd_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pslvrsuqd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpslvrsuqd(<2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  return _mm_dsp_pslvrsuqd_epi32(__A, __B);
}

__m128i test_mm_dsp_psrarsqd_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_psrarsqd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpsrarsqd(<2 x i64> %{{.*}}, i32 127)
  return _mm_dsp_psrarsqd_epi32(__A, 127);
}

__m128i test_mm_dsp_psravrsqd_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_psravrsqd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpsravrsqd(<2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  return _mm_dsp_psravrsqd_epi32(__A, __B);
}

__m128i test_mm_dsp_pslrsqd_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_dsp_pslrsqd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpslrsqd(<2 x i64> %{{.*}}, i32 127)
  return _mm_dsp_pslrsqd_epi32(__A, 127);
}

__m128i test_mm_dsp_pslvrsqd_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pslvrsqd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpslvrsqd(<2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  return _mm_dsp_pslvrsqd_epi32(__A, __B);
}

