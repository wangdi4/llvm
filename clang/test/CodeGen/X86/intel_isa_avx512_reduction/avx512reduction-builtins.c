// REQUIRES: intel_feature_isa_avx512_reduction
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +avx512reduction \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm512_phraddbd_epu32(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phraddbd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddbd512(
  return _mm512_phraddbd_epu32(__A);
}

__m128i test_mm512_mask_phraddbd_epu32(__mmask64 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phraddbd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddbd512(
  return _mm512_mask_phraddbd_epu32(__A, __B);
}

__m128i test_mm512_phraddd_epu32(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phraddd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddd512(
  return _mm512_phraddd_epu32(__A);
}

__m128i test_mm512_mask_phraddd_epu32(__mmask16 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phraddd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddd512(
  return _mm512_mask_phraddd_epu32(__A, __B);
}

__m128i test_mm512_phraddq_epu64(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phraddq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphraddq512(
  return _mm512_phraddq_epu64(__A);
}

__m128i test_mm512_mask_phraddq_epu64(__mmask8 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phraddq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphraddq512(
  return _mm512_mask_phraddq_epu64(__A, __B);
}

__m128i test_mm512_phraddwd_epu32(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phraddwd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddwd512(
  return _mm512_phraddwd_epu32(__A);
}

__m128i test_mm512_mask_phraddwd_epu32(__mmask32 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phraddwd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddwd512(
  return _mm512_mask_phraddwd_epu32(__A, __B);
}

__m128i test_mm512_phraddsbd_epi32(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phraddsbd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddsbd512(
  return _mm512_phraddsbd_epi32(__A);
}

__m128i test_mm512_mask_phraddsbd_epi32(__mmask64 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phraddsbd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddsbd512(
  return _mm512_mask_phraddsbd_epi32(__A, __B);
}

__m128i test_mm512_phraddsd_epi32(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phraddsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddsd512(
  return _mm512_phraddsd_epi32(__A);
}

__m128i test_mm512_mask_phraddsd_epi32(__mmask16 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phraddsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddsd512(
  return _mm512_mask_phraddsd_epi32(__A, __B);
}

__m128i test_mm512_phraddsq_epi64(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phraddsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphraddsq512(
  return _mm512_phraddsq_epi64(__A);
}

__m128i test_mm512_mask_phraddsq_epi64(__mmask8 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phraddsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphraddsq512(
  return _mm512_mask_phraddsq_epi64(__A, __B);
}

__m128i test_mm512_phraddswd_epi32(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phraddswd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddswd512(
  return _mm512_phraddswd_epi32(__A);
}

__m128i test_mm512_mask_phraddswd_epi32(__mmask32 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phraddswd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddswd512(
  return _mm512_mask_phraddswd_epi32(__A, __B);
}

__m128i test_mm512_phrandb_epu8(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrandb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrandb512(
  return _mm512_phrandb_epu8(__A);
}

__m128i test_mm512_mask_phrandb_epu8(__mmask64 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrandb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrandb512(
  return _mm512_mask_phrandb_epu8(__A, __B);
}

__m128i test_mm512_phrandd_epu32(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrandd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrandd512(
  return _mm512_phrandd_epu32(__A);
}

__m128i test_mm512_mask_phrandd_epu32(__mmask16 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrandd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrandd512(
  return _mm512_mask_phrandd_epu32(__A, __B);
}

__m128i test_mm512_phranddq_epu64(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phranddq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphranddq512(
  return _mm512_phranddq_epu64(__A);
}

__m128i test_mm512_phrandq_epu64(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrandq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrandq512(
  return _mm512_phrandq_epu64(__A);
}

__m128i test_mm512_mask_phrandq_epu64(__mmask8 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrandq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrandq512(
  return _mm512_mask_phrandq_epu64(__A, __B);
}

__m256i test_mm512_phrandqq_epu64(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrandqq_epu64(
  // CHECK: call <4 x i64> @llvm.x86.avx512reduction.vphrandqq512(
  return _mm512_phrandqq_epu64(__A);
}

__m128i test_mm512_phrandw_epu16(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrandw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrandw512(
  return _mm512_phrandw_epu16(__A);
}

__m128i test_mm512_mask_phrandw_epu16(__mmask32 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrandw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrandw512(
  return _mm512_mask_phrandw_epu16(__A, __B);
}

__m128i test_mm512_phrmaxb_epu8(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrmaxb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrmaxb512(
  return _mm512_phrmaxb_epu8(__A);
}

__m128i test_mm512_mask_phrmaxb_epu8(__mmask64 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrmaxb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrmaxb512(
  return _mm512_mask_phrmaxb_epu8(__A, __B);
}

__m128i test_mm512_phrmaxd_epu32(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrmaxd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrmaxd512(
  return _mm512_phrmaxd_epu32(__A);
}

__m128i test_mm512_mask_phrmaxd_epu32(__mmask16 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrmaxd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrmaxd512(
  return _mm512_mask_phrmaxd_epu32(__A, __B);
}

__m128i test_mm512_phrmaxq_epu64(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrmaxq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrmaxq512(
  return _mm512_phrmaxq_epu64(__A);
}

__m128i test_mm512_mask_phrmaxq_epu64(__mmask8 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrmaxq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrmaxq512(
  return _mm512_mask_phrmaxq_epu64(__A, __B);
}

__m128i test_mm512_phrmaxw_epu16(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrmaxw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrmaxw512(
  return _mm512_phrmaxw_epu16(__A);
}

__m128i test_mm512_mask_phrmaxw_epu16(__mmask32 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrmaxw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrmaxw512(
  return _mm512_mask_phrmaxw_epu16(__A, __B);
}

__m128i test_mm512_phrmaxsb_epi8(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrmaxsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrmaxsb512(
  return _mm512_phrmaxsb_epi8(__A);
}

__m128i test_mm512_mask_phrmaxsb_epi8(__mmask64 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrmaxsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrmaxsb512(
  return _mm512_mask_phrmaxsb_epi8(__A, __B);
}

__m128i test_mm512_phrmaxsd_epi32(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrmaxsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrmaxsd512(
  return _mm512_phrmaxsd_epi32(__A);
}

__m128i test_mm512_mask_phrmaxsd_epi32(__mmask16 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrmaxsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrmaxsd512(
  return _mm512_mask_phrmaxsd_epi32(__A, __B);
}

__m128i test_mm512_phrmaxsq_epi64(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrmaxsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrmaxsq512(
  return _mm512_phrmaxsq_epi64(__A);
}

__m128i test_mm512_mask_phrmaxsq_epi64(__mmask8 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrmaxsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrmaxsq512(
  return _mm512_mask_phrmaxsq_epi64(__A, __B);
}

__m128i test_mm512_phrmaxsw_epi16(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrmaxsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrmaxsw512(
  return _mm512_phrmaxsw_epi16(__A);
}

__m128i test_mm512_mask_phrmaxsw_epi16(__mmask32 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrmaxsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrmaxsw512(
  return _mm512_mask_phrmaxsw_epi16(__A, __B);
}

__m128i test_mm512_phrminb_epu8(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrminb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrminb512(
  return _mm512_phrminb_epu8(__A);
}

__m128i test_mm512_mask_phrminb_epu8(__mmask64 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrminb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrminb512(
  return _mm512_mask_phrminb_epu8(__A, __B);
}

__m128i test_mm512_phrmind_epu32(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrmind_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrmind512(
  return _mm512_phrmind_epu32(__A);
}

__m128i test_mm512_mask_phrmind_epu32(__mmask16 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrmind_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrmind512(
  return _mm512_mask_phrmind_epu32(__A, __B);
}

__m128i test_mm512_phrminq_epu64(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrminq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrminq512(
  return _mm512_phrminq_epu64(__A);
}

__m128i test_mm512_mask_phrminq_epu64(__mmask8 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrminq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrminq512(
  return _mm512_mask_phrminq_epu64(__A, __B);
}

__m128i test_mm512_phrminw_epu16(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrminw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrminw512(
  return _mm512_phrminw_epu16(__A);
}

__m128i test_mm512_mask_phrminw_epu16(__mmask32 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrminw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrminw512(
  return _mm512_mask_phrminw_epu16(__A, __B);
}

__m128i test_mm512_phrminsb_epi8(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrminsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrminsb512(
  return _mm512_phrminsb_epi8(__A);
}

__m128i test_mm512_mask_phrminsb_epi8(__mmask64 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrminsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrminsb512(
  return _mm512_mask_phrminsb_epi8(__A, __B);
}

__m128i test_mm512_phrminsd_epi32(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrminsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrminsd512(
  return _mm512_phrminsd_epi32(__A);
}

__m128i test_mm512_mask_phrminsd_epi32(__mmask16 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrminsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrminsd512(
  return _mm512_mask_phrminsd_epi32(__A, __B);
}

__m128i test_mm512_phrminsq_epi64(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrminsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrminsq512(
  return _mm512_phrminsq_epi64(__A);
}

__m128i test_mm512_mask_phrminsq_epi64(__mmask8 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrminsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrminsq512(
  return _mm512_mask_phrminsq_epi64(__A, __B);
}

__m128i test_mm512_phrminsw_epi16(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrminsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrminsw512(
  return _mm512_phrminsw_epi16(__A);
}

__m128i test_mm512_mask_phrminsw_epi16(__mmask32 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrminsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrminsw512(
  return _mm512_mask_phrminsw_epi16(__A, __B);
}

__m128i test_mm512_phrorb_epu8(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrorb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrorb512(
  return _mm512_phrorb_epu8(__A);
}

__m128i test_mm512_mask_phrorb_epu8(__mmask64 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrorb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrorb512(
  return _mm512_mask_phrorb_epu8(__A, __B);
}

__m128i test_mm512_phrord_epu32(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrord_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrord512(
  return _mm512_phrord_epu32(__A);
}

__m128i test_mm512_mask_phrord_epu32(__mmask16 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrord_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrord512(
  return _mm512_mask_phrord_epu32(__A, __B);
}

__m128i test_mm512_phrordq_epu64(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrordq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrordq512(
  return _mm512_phrordq_epu64(__A);
}

__m128i test_mm512_phrorq_epu64(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrorq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrorq512(
  return _mm512_phrorq_epu64(__A);
}

__m128i test_mm512_mask_phrorq_epu64(__mmask8 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrorq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrorq512(
  return _mm512_mask_phrorq_epu64(__A, __B);
}

__m256i test_mm512_phrorqq_epu64(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrorqq_epu64(
  // CHECK: call <4 x i64> @llvm.x86.avx512reduction.vphrorqq512(
  return _mm512_phrorqq_epu64(__A);
}

__m128i test_mm512_phrorw_epu16(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrorw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrorw512(
  return _mm512_phrorw_epu16(__A);
}

__m128i test_mm512_mask_phrorw_epu16(__mmask32 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrorw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrorw512(
  return _mm512_mask_phrorw_epu16(__A, __B);
}

__m128i test_mm512_phrxorb_epu8(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrxorb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrxorb512(
  return _mm512_phrxorb_epu8(__A);
}

__m128i test_mm512_mask_phrxorb_epu8(__mmask64 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrxorb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrxorb512(
  return _mm512_mask_phrxorb_epu8(__A, __B);
}

__m128i test_mm512_phrxord_epu32(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrxord_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrxord512(
  return _mm512_phrxord_epu32(__A);
}

__m128i test_mm512_mask_phrxord_epu32(__mmask16 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrxord_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrxord512(
  return _mm512_mask_phrxord_epu32(__A, __B);
}

__m128i test_mm512_phrxordq_epu64(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrxordq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrxordq512(
  return _mm512_phrxordq_epu64(__A);
}

__m128i test_mm512_phrxorq_epu64(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrxorq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrxorq512(
  return _mm512_phrxorq_epu64(__A);
}

__m128i test_mm512_mask_phrxorq_epu64(__mmask8 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrxorq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrxorq512(
  return _mm512_mask_phrxorq_epu64(__A, __B);
}

__m256i test_mm512_phrxorqq_epu64(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrxorqq_epu64(
  // CHECK: call <4 x i64> @llvm.x86.avx512reduction.vphrxorqq512(
  return _mm512_phrxorqq_epu64(__A);
}

__m128i test_mm512_phrxorw_epu16(__m512i __A) {
  // CHECK-LABEL: @test_mm512_phrxorw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrxorw512(
  return _mm512_phrxorw_epu16(__A);
}

__m128i test_mm512_mask_phrxorw_epu16(__mmask32 __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_phrxorw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrxorw512(
  return _mm512_mask_phrxorw_epu16(__A, __B);
}

