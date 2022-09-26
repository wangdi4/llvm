// REQUIRES: intel_feature_isa_avx512_reduction2
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +avx512reduction2 -target-feature +avx512vl \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_phraaddbd_epu32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phraaddbd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraaddbd128(
  return _mm_phraaddbd_epu32(__A, __B);
}

__m128i test_mm_mask_phraaddbd_epu32(__mmask16 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phraaddbd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraaddbd128(
  return _mm_mask_phraaddbd_epu32(__A, __B, __C);
}

__m128i test_mm256_phraaddbd_epu32(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phraaddbd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraaddbd256(
  return _mm256_phraaddbd_epu32(__A, __B);
}

__m128i test_mm256_mask_phraaddbd_epu32(__mmask32 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phraaddbd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraaddbd256(
  return _mm256_mask_phraaddbd_epu32(__A, __B, __C);
}

__m128i test_mm_phraaddwd_epu32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phraaddwd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraaddwd128(
  return _mm_phraaddwd_epu32(__A, __B);
}

__m128i test_mm_mask_phraaddwd_epu32(__mmask8 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phraaddwd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraaddwd128(
  return _mm_mask_phraaddwd_epu32(__A, __B, __C);
}

__m128i test_mm256_phraaddwd_epu32(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phraaddwd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraaddwd256(
  return _mm256_phraaddwd_epu32(__A, __B);
}

__m128i test_mm256_mask_phraaddwd_epu32(__mmask16 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phraaddwd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraaddwd256(
  return _mm256_mask_phraaddwd_epu32(__A, __B, __C);
}

__m128i test_mm_phraaddsbd_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phraaddsbd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraaddsbd128(
  return _mm_phraaddsbd_epi32(__A, __B);
}

__m128i test_mm_mask_phraaddsbd_epi32(__mmask16 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phraaddsbd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraaddsbd128(
  return _mm_mask_phraaddsbd_epi32(__A, __B, __C);
}

__m128i test_mm256_phraaddsbd_epi32(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phraaddsbd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraaddsbd256(
  return _mm256_phraaddsbd_epi32(__A, __B);
}

__m128i test_mm256_mask_phraaddsbd_epi32(__mmask32 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phraaddsbd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraaddsbd256(
  return _mm256_mask_phraaddsbd_epi32(__A, __B, __C);
}

__m128i test_mm_phraaddswd_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phraaddswd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraaddswd128(
  return _mm_phraaddswd_epi32(__A, __B);
}

__m128i test_mm_mask_phraaddswd_epi32(__mmask8 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phraaddswd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraaddswd128(
  return _mm_mask_phraaddswd_epi32(__A, __B, __C);
}

__m128i test_mm256_phraaddswd_epi32(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phraaddswd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraaddswd256(
  return _mm256_phraaddswd_epi32(__A, __B);
}

__m128i test_mm256_mask_phraaddswd_epi32(__mmask16 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phraaddswd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraaddswd256(
  return _mm256_mask_phraaddswd_epi32(__A, __B, __C);
}

__m128i test_mm_phraandb_epu8(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phraandb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.vphraandb128(
  return _mm_phraandb_epu8(__A, __B);
}

__m128i test_mm_mask_phraandb_epu8(__mmask16 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phraandb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.mask.vphraandb128(
  return _mm_mask_phraandb_epu8(__A, __B, __C);
}

__m128i test_mm256_phraandb_epu8(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phraandb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.vphraandb256(
  return _mm256_phraandb_epu8(__A, __B);
}

__m128i test_mm256_mask_phraandb_epu8(__mmask32 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phraandb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.mask.vphraandb256(
  return _mm256_mask_phraandb_epu8(__A, __B, __C);
}

__m128i test_mm_phraandd_epu32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phraandd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraandd128(
  return _mm_phraandd_epu32(__A, __B);
}

__m128i test_mm_mask_phraandd_epu32(__mmask8 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phraandd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraandd128(
  return _mm_mask_phraandd_epu32(__A, __B, __C);
}

__m128i test_mm256_phraandd_epu32(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phraandd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraandd256(
  return _mm256_phraandd_epu32(__A, __B);
}

__m128i test_mm256_mask_phraandd_epu32(__mmask8 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phraandd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraandd256(
  return _mm256_mask_phraandd_epu32(__A, __B, __C);
}

__m128i test_mm_phraandq_epu64(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phraandq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.vphraandq128(
  return _mm_phraandq_epu64(__A, __B);
}

__m128i test_mm_mask_phraandq_epu64(__mmask8 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phraandq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.mask.vphraandq128(
  return _mm_mask_phraandq_epu64(__A, __B, __C);
}

__m128i test_mm256_phraandq_epu64(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phraandq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.vphraandq256(
  return _mm256_phraandq_epu64(__A, __B);
}

__m128i test_mm256_mask_phraandq_epu64(__mmask8 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phraandq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.mask.vphraandq256(
  return _mm256_mask_phraandq_epu64(__A, __B, __C);
}

__m128i test_mm_phraandw_epu16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phraandw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.vphraandw128(
  return _mm_phraandw_epu16(__A, __B);
}

__m128i test_mm_mask_phraandw_epu16(__mmask8 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phraandw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.mask.vphraandw128(
  return _mm_mask_phraandw_epu16(__A, __B, __C);
}

__m128i test_mm256_phraandw_epu16(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phraandw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.vphraandw256(
  return _mm256_phraandw_epu16(__A, __B);
}

__m128i test_mm256_mask_phraandw_epu16(__mmask16 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phraandw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.mask.vphraandw256(
  return _mm256_mask_phraandw_epu16(__A, __B, __C);
}

__m128i test_mm_phramaxsb_epi8(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phramaxsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.vphramaxsb128(
  return _mm_phramaxsb_epi8(__A, __B);
}

__m128i test_mm_mask_phramaxsb_epi8(__mmask16 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phramaxsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.mask.vphramaxsb128(
  return _mm_mask_phramaxsb_epi8(__A, __B, __C);
}

__m128i test_mm256_phramaxsb_epi8(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phramaxsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.vphramaxsb256(
  return _mm256_phramaxsb_epi8(__A, __B);
}

__m128i test_mm256_mask_phramaxsb_epi8(__mmask32 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phramaxsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.mask.vphramaxsb256(
  return _mm256_mask_phramaxsb_epi8(__A, __B, __C);
}

__m128i test_mm_phramaxsd_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phramaxsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphramaxsd128(
  return _mm_phramaxsd_epi32(__A, __B);
}

__m128i test_mm_mask_phramaxsd_epi32(__mmask8 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phramaxsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphramaxsd128(
  return _mm_mask_phramaxsd_epi32(__A, __B, __C);
}

__m128i test_mm256_phramaxsd_epi32(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phramaxsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphramaxsd256(
  return _mm256_phramaxsd_epi32(__A, __B);
}

__m128i test_mm256_mask_phramaxsd_epi32(__mmask8 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phramaxsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphramaxsd256(
  return _mm256_mask_phramaxsd_epi32(__A, __B, __C);
}

__m128i test_mm_phramaxsq_epi64(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phramaxsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.vphramaxsq128(
  return _mm_phramaxsq_epi64(__A, __B);
}

__m128i test_mm_mask_phramaxsq_epi64(__mmask8 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phramaxsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.mask.vphramaxsq128(
  return _mm_mask_phramaxsq_epi64(__A, __B, __C);
}

__m128i test_mm256_phramaxsq_epi64(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phramaxsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.vphramaxsq256(
  return _mm256_phramaxsq_epi64(__A, __B);
}

__m128i test_mm256_mask_phramaxsq_epi64(__mmask8 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phramaxsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.mask.vphramaxsq256(
  return _mm256_mask_phramaxsq_epi64(__A, __B, __C);
}

__m128i test_mm_phramaxsw_epi16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phramaxsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.vphramaxsw128(
  return _mm_phramaxsw_epi16(__A, __B);
}

__m128i test_mm_mask_phramaxsw_epi16(__mmask8 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phramaxsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.mask.vphramaxsw128(
  return _mm_mask_phramaxsw_epi16(__A, __B, __C);
}

__m128i test_mm256_phramaxsw_epi16(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phramaxsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.vphramaxsw256(
  return _mm256_phramaxsw_epi16(__A, __B);
}

__m128i test_mm256_mask_phramaxsw_epi16(__mmask16 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phramaxsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.mask.vphramaxsw256(
  return _mm256_mask_phramaxsw_epi16(__A, __B, __C);
}

__m128i test_mm_phraminb_epu8(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phraminb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.vphraminb128(
  return _mm_phraminb_epu8(__A, __B);
}

__m128i test_mm_mask_phraminb_epu8(__mmask16 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phraminb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.mask.vphraminb128(
  return _mm_mask_phraminb_epu8(__A, __B, __C);
}

__m128i test_mm256_phraminb_epu8(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phraminb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.vphraminb256(
  return _mm256_phraminb_epu8(__A, __B);
}

__m128i test_mm256_mask_phraminb_epu8(__mmask32 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phraminb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.mask.vphraminb256(
  return _mm256_mask_phraminb_epu8(__A, __B, __C);
}

__m128i test_mm_phramind_epu32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phramind_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphramind128(
  return _mm_phramind_epu32(__A, __B);
}

__m128i test_mm_mask_phramind_epu32(__mmask8 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phramind_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphramind128(
  return _mm_mask_phramind_epu32(__A, __B, __C);
}

__m128i test_mm256_phramind_epu32(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phramind_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphramind256(
  return _mm256_phramind_epu32(__A, __B);
}

__m128i test_mm256_mask_phramind_epu32(__mmask8 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phramind_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphramind256(
  return _mm256_mask_phramind_epu32(__A, __B, __C);
}

__m128i test_mm_phraminq_epu64(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phraminq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.vphraminq128(
  return _mm_phraminq_epu64(__A, __B);
}

__m128i test_mm_mask_phraminq_epu64(__mmask8 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phraminq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.mask.vphraminq128(
  return _mm_mask_phraminq_epu64(__A, __B, __C);
}

__m128i test_mm256_phraminq_epu64(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phraminq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.vphraminq256(
  return _mm256_phraminq_epu64(__A, __B);
}

__m128i test_mm256_mask_phraminq_epu64(__mmask8 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phraminq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.mask.vphraminq256(
  return _mm256_mask_phraminq_epu64(__A, __B, __C);
}

__m128i test_mm_phraminw_epu16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phraminw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.vphraminw128(
  return _mm_phraminw_epu16(__A, __B);
}

__m128i test_mm_mask_phraminw_epu16(__mmask8 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phraminw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.mask.vphraminw128(
  return _mm_mask_phraminw_epu16(__A, __B, __C);
}

__m128i test_mm256_phraminw_epu16(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phraminw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.vphraminw256(
  return _mm256_phraminw_epu16(__A, __B);
}

__m128i test_mm256_mask_phraminw_epu16(__mmask16 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phraminw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.mask.vphraminw256(
  return _mm256_mask_phraminw_epu16(__A, __B, __C);
}

__m128i test_mm_phraminsb_epi8(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phraminsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.vphraminsb128(
  return _mm_phraminsb_epi8(__A, __B);
}

__m128i test_mm_mask_phraminsb_epi8(__mmask16 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phraminsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.mask.vphraminsb128(
  return _mm_mask_phraminsb_epi8(__A, __B, __C);
}

__m128i test_mm256_phraminsb_epi8(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phraminsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.vphraminsb256(
  return _mm256_phraminsb_epi8(__A, __B);
}

__m128i test_mm256_mask_phraminsb_epi8(__mmask32 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phraminsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction2.mask.vphraminsb256(
  return _mm256_mask_phraminsb_epi8(__A, __B, __C);
}

__m128i test_mm_phraminsd_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phraminsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraminsd128(
  return _mm_phraminsd_epi32(__A, __B);
}

__m128i test_mm_mask_phraminsd_epi32(__mmask8 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phraminsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraminsd128(
  return _mm_mask_phraminsd_epi32(__A, __B, __C);
}

__m128i test_mm256_phraminsd_epi32(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phraminsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.vphraminsd256(
  return _mm256_phraminsd_epi32(__A, __B);
}

__m128i test_mm256_mask_phraminsd_epi32(__mmask8 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phraminsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction2.mask.vphraminsd256(
  return _mm256_mask_phraminsd_epi32(__A, __B, __C);
}

__m128i test_mm_phraminsq_epi64(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phraminsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.vphraminsq128(
  return _mm_phraminsq_epi64(__A, __B);
}

__m128i test_mm_mask_phraminsq_epi64(__mmask8 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phraminsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.mask.vphraminsq128(
  return _mm_mask_phraminsq_epi64(__A, __B, __C);
}

__m128i test_mm256_phraminsq_epi64(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phraminsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.vphraminsq256(
  return _mm256_phraminsq_epi64(__A, __B);
}

__m128i test_mm256_mask_phraminsq_epi64(__mmask8 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phraminsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction2.mask.vphraminsq256(
  return _mm256_mask_phraminsq_epi64(__A, __B, __C);
}

__m128i test_mm_phraminsw_epi16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_phraminsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.vphraminsw128(
  return _mm_phraminsw_epi16(__A, __B);
}

__m128i test_mm_mask_phraminsw_epi16(__mmask8 __A, __m128i __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_phraminsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.mask.vphraminsw128(
  return _mm_mask_phraminsw_epi16(__A, __B, __C);
}

__m128i test_mm256_phraminsw_epi16(__m128i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_phraminsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.vphraminsw256(
  return _mm256_phraminsw_epi16(__A, __B);
}

__m128i test_mm256_mask_phraminsw_epi16(__mmask16 __A, __m128i __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_phraminsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction2.mask.vphraminsw256(
  return _mm256_mask_phraminsw_epi16(__A, __B, __C);
}

