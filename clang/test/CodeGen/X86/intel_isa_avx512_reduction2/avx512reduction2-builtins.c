// REQUIRES: intel_feature_isa_avx512_reduction2
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +avx512reduction2 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm512_phraaddbd_epu32(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phraaddbd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraaddbd512(
  return _mm512_phraaddbd_epu32(__A, __B);
}

__m128i test_mm512_mask_phraaddbd_epu32(__mmask64 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phraaddbd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraaddbd512(
  return _mm512_mask_phraaddbd_epu32(__A, __B, __C);
}

__m128i test_mm512_phraaddwd_epu32(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phraaddwd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraaddwd512(
  return _mm512_phraaddwd_epu32(__A, __B);
}

__m128i test_mm512_mask_phraaddwd_epu32(__mmask32 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phraaddwd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraaddwd512(
  return _mm512_mask_phraaddwd_epu32(__A, __B, __C);
}

__m128i test_mm512_phraaddsbd_epi32(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phraaddsbd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraaddsbd512(
  return _mm512_phraaddsbd_epi32(__A, __B);
}

__m128i test_mm512_mask_phraaddsbd_epi32(__mmask64 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phraaddsbd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraaddsbd512(
  return _mm512_mask_phraaddsbd_epi32(__A, __B, __C);
}

__m128i test_mm512_phraaddswd_epi32(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phraaddswd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraaddswd512(
  return _mm512_phraaddswd_epi32(__A, __B);
}

__m128i test_mm512_mask_phraaddswd_epi32(__mmask32 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phraaddswd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraaddswd512(
  return _mm512_mask_phraaddswd_epi32(__A, __B, __C);
}

__m128i test_mm512_phraandb_epu8(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phraandb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.vphraandb512(
  return _mm512_phraandb_epu8(__A, __B);
}

__m128i test_mm512_mask_phraandb_epu8(__mmask64 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phraandb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.mask.vphraandb512(
  return _mm512_mask_phraandb_epu8(__A, __B, __C);
}

__m128i test_mm512_phraandd_epu32(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phraandd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraandd512(
  return _mm512_phraandd_epu32(__A, __B);
}

__m128i test_mm512_mask_phraandd_epu32(__mmask16 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phraandd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraandd512(
  return _mm512_mask_phraandd_epu32(__A, __B, __C);
}

__m128i test_mm512_phraandq_epu64(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phraandq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.vphraandq512(
  return _mm512_phraandq_epu64(__A, __B);
}

__m128i test_mm512_mask_phraandq_epu64(__mmask8 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phraandq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.mask.vphraandq512(
  return _mm512_mask_phraandq_epu64(__A, __B, __C);
}

__m128i test_mm512_phraandw_epu16(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phraandw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.vphraandw512(
  return _mm512_phraandw_epu16(__A, __B);
}

__m128i test_mm512_mask_phraandw_epu16(__mmask32 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phraandw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.mask.vphraandw512(
  return _mm512_mask_phraandw_epu16(__A, __B, __C);
}

__m128i test_mm512_phramaxsb_epi8(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phramaxsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.vphramaxsb512(
  return _mm512_phramaxsb_epi8(__A, __B);
}

__m128i test_mm512_mask_phramaxsb_epi8(__mmask64 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phramaxsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.mask.vphramaxsb512(
  return _mm512_mask_phramaxsb_epi8(__A, __B, __C);
}

__m128i test_mm512_phramaxsd_epi32(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phramaxsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphramaxsd512(
  return _mm512_phramaxsd_epi32(__A, __B);
}

__m128i test_mm512_mask_phramaxsd_epi32(__mmask16 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phramaxsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphramaxsd512(
  return _mm512_mask_phramaxsd_epi32(__A, __B, __C);
}

__m128i test_mm512_phramaxsq_epi64(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phramaxsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.vphramaxsq512(
  return _mm512_phramaxsq_epi64(__A, __B);
}

__m128i test_mm512_mask_phramaxsq_epi64(__mmask8 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phramaxsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.mask.vphramaxsq512(
  return _mm512_mask_phramaxsq_epi64(__A, __B, __C);
}

__m128i test_mm512_phramaxsw_epi16(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phramaxsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.vphramaxsw512(
  return _mm512_phramaxsw_epi16(__A, __B);
}

__m128i test_mm512_mask_phramaxsw_epi16(__mmask32 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phramaxsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.mask.vphramaxsw512(
  return _mm512_mask_phramaxsw_epi16(__A, __B, __C);
}

__m128i test_mm512_phraminb_epu8(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phraminb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.vphraminb512(
  return _mm512_phraminb_epu8(__A, __B);
}

__m128i test_mm512_mask_phraminb_epu8(__mmask64 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phraminb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.mask.vphraminb512(
  return _mm512_mask_phraminb_epu8(__A, __B, __C);
}

__m128i test_mm512_phramind_epu32(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phramind_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphramind512(
  return _mm512_phramind_epu32(__A, __B);
}

__m128i test_mm512_mask_phramind_epu32(__mmask16 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phramind_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphramind512(
  return _mm512_mask_phramind_epu32(__A, __B, __C);
}

__m128i test_mm512_phraminq_epu64(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phraminq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.vphraminq512(
  return _mm512_phraminq_epu64(__A, __B);
}

__m128i test_mm512_mask_phraminq_epu64(__mmask8 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phraminq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.mask.vphraminq512(
  return _mm512_mask_phraminq_epu64(__A, __B, __C);
}

__m128i test_mm512_phraminw_epu16(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phraminw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.vphraminw512(
  return _mm512_phraminw_epu16(__A, __B);
}

__m128i test_mm512_mask_phraminw_epu16(__mmask32 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phraminw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.mask.vphraminw512(
  return _mm512_mask_phraminw_epu16(__A, __B, __C);
}

__m128i test_mm512_phraminsb_epi8(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phraminsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.vphraminsb512(
  return _mm512_phraminsb_epi8(__A, __B);
}

__m128i test_mm512_mask_phraminsb_epi8(__mmask64 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phraminsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.mask.vphraminsb512(
  return _mm512_mask_phraminsb_epi8(__A, __B, __C);
}

__m128i test_mm512_phraminsd_epi32(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phraminsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraminsd512(
  return _mm512_phraminsd_epi32(__A, __B);
}

__m128i test_mm512_mask_phraminsd_epi32(__mmask16 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phraminsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraminsd512(
  return _mm512_mask_phraminsd_epi32(__A, __B, __C);
}

__m128i test_mm512_phraminsq_epi64(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phraminsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.vphraminsq512(
  return _mm512_phraminsq_epi64(__A, __B);
}

__m128i test_mm512_mask_phraminsq_epi64(__mmask8 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phraminsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.mask.vphraminsq512(
  return _mm512_mask_phraminsq_epi64(__A, __B, __C);
}

__m128i test_mm512_phraminsw_epi16(__m128i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_phraminsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.vphraminsw512(
  return _mm512_phraminsw_epi16(__A, __B);
}

__m128i test_mm512_mask_phraminsw_epi16(__mmask32 __A, __m128i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_phraminsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.mask.vphraminsw512(
  return _mm512_mask_phraminsw_epi16(__A, __B, __C);
}

