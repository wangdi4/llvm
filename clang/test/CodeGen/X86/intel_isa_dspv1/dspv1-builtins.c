// REQUIRES: intel_feature_isa_dspv1
// RUN: %clang_cc1 -no-opaque-pointers %s -flax-vector-conversions=none -ffreestanding -triple=i686-unknown-unknown -target-feature +dspv1 \
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

__m128i test_mm_dsp_pcaddrotsraw_epi16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pcaddrotsraw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpcaddrotsraw(<8 x i16> %{{.*}}, <8 x i16> %{{.*}}, i32 127)
  return _mm_dsp_pcaddrotsraw_epi16(__A, __B, 127);
}

__m128i test_mm_dsp_pmuluwr_epi16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pmuluwr_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpmuluwr(<8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_dsp_pmuluwr_epi16(__A, __B);
}

__m128i test_mm_dsp_pmulwr_epi16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pmulwr_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpmulwr(<8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_dsp_pmulwr_epi16(__A, __B);
}

__m128i test_mm_dsp_pmulws_epi16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pmulws_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpmulws(<8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_dsp_pmulws_epi16(__A, __B);
}

__m128i test_mm_dsp_pmulwfrs_epi16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pmulwfrs_epi16(
  // CHECK: call <8 x i16> @llvm.x86.dvpmulwfrs(<8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_dsp_pmulwfrs_epi16(__A, __B);
}

__m128i test_mm_dsp_pdpint4uud_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpint4uud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpint4uud(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_dsp_pdpint4uud_epi32(__A, __B, __C);
}

__m128i test_mm_dsp_pdpint4ssd_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpint4ssd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpint4ssd(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_dsp_pdpint4ssd_epi32(__A, __B, __C);
}

__m128i test_mm_dsp_pdpint4usd_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpint4usd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpint4usd(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_dsp_pdpint4usd_epi32(__A, __B, __C);
}

__m128i test_mm_dsp_pdpint4sud_epi32(__m128i __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pdpint4sud_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpdpint4sud(<4 x i32> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_dsp_pdpint4sud_epi32(__A, __B, __C);
}

__m128i test_mm_dsp_pcaddrotsrad_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_pcaddrotsrad_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpcaddrotsrad(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, i32 127)
  return _mm_dsp_pcaddrotsrad_epi32(__A, __B, 127);
}

__m128i test_mm_dsp_punpckdq_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_dsp_punpckdq_epi32(
  // CHECK: call <4 x i32> @llvm.x86.dvpunpckdq(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, i32 127)
  return _mm_dsp_punpckdq_epi32(__A, __B, 127);
}

__m128i test_mm_dsp_pmasklddqu_epi8(__m128i __A, const __m128i * __B) {
  // CHECK-LABEL: @test_mm_dsp_pmasklddqu_epi8(
  // CHECK: call <16 x i8> @llvm.x86.dvpmasklddqu(<16 x i8> %{{.*}}, i8* %{{.*}})
  return _mm_dsp_pmasklddqu_epi8(__A, __B);
}

void test_mm_dsp_pmaskstdqu_epi8(__m128i * __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_dsp_pmaskstdqu_epi8(
  // CHECK: call void @llvm.x86.dvpmaskstdqu(i8* %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  _mm_dsp_pmaskstdqu_epi8(__A, __B, __C);
}

void test_mm_dsp_ptestmxcsrflgs(void) {
  // CHECK-LABEL: @test_mm_dsp_ptestmxcsrflgs(
  // CHECK: call void asm sideeffect "dvptestmxcsrflgs", "~{memory},~{dirflag},~{fpsr},~{flags}"()
  _mm_dsp_ptestmxcsrflgs();
}

