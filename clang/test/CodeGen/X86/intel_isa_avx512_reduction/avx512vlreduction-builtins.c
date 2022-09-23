// REQUIRES: intel_feature_isa_avx512_reduction
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +avx512reduction -target-feature +avx512vl \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_phraddbd_epu32(__m128i __A) {
  // CHECK-LABEL: @test_mm_phraddbd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddbd128(
  return _mm_phraddbd_epu32(__A);
}

__m128i test_mm_mask_phraddbd_epu32(__mmask16 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phraddbd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddbd128(
  return _mm_mask_phraddbd_epu32(__A, __B);
}

__m128i test_mm256_phraddbd_epu32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phraddbd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddbd256(
  return _mm256_phraddbd_epu32(__A);
}

__m128i test_mm256_mask_phraddbd_epu32(__mmask32 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phraddbd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddbd256(
  return _mm256_mask_phraddbd_epu32(__A, __B);
}

__m128i test_mm_phraddd_epu32(__m128i __A) {
  // CHECK-LABEL: @test_mm_phraddd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddd128(
  return _mm_phraddd_epu32(__A);
}

__m128i test_mm_mask_phraddd_epu32(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phraddd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddd128(
  return _mm_mask_phraddd_epu32(__A, __B);
}

__m128i test_mm256_phraddd_epu32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phraddd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddd256(
  return _mm256_phraddd_epu32(__A);
}

__m128i test_mm256_mask_phraddd_epu32(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phraddd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddd256(
  return _mm256_mask_phraddd_epu32(__A, __B);
}

__m128i test_mm_phraddq_epu64(__m128i __A) {
  // CHECK-LABEL: @test_mm_phraddq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphraddq128(
  return _mm_phraddq_epu64(__A);
}

__m128i test_mm_mask_phraddq_epu64(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phraddq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphraddq128(
  return _mm_mask_phraddq_epu64(__A, __B);
}

__m128i test_mm256_phraddq_epu64(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phraddq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphraddq256(
  return _mm256_phraddq_epu64(__A);
}

__m128i test_mm256_mask_phraddq_epu64(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phraddq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphraddq256(
  return _mm256_mask_phraddq_epu64(__A, __B);
}

__m128i test_mm_phraddwd_epu32(__m128i __A) {
  // CHECK-LABEL: @test_mm_phraddwd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddwd128(
  return _mm_phraddwd_epu32(__A);
}

__m128i test_mm_mask_phraddwd_epu32(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phraddwd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddwd128(
  return _mm_mask_phraddwd_epu32(__A, __B);
}

__m128i test_mm256_phraddwd_epu32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phraddwd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddwd256(
  return _mm256_phraddwd_epu32(__A);
}

__m128i test_mm256_mask_phraddwd_epu32(__mmask16 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phraddwd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddwd256(
  return _mm256_mask_phraddwd_epu32(__A, __B);
}

__m128i test_mm_phraddsbd_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_phraddsbd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddsbd128(
  return _mm_phraddsbd_epi32(__A);
}

__m128i test_mm_mask_phraddsbd_epi32(__mmask16 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phraddsbd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddsbd128(
  return _mm_mask_phraddsbd_epi32(__A, __B);
}

__m128i test_mm256_phraddsbd_epi32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phraddsbd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddsbd256(
  return _mm256_phraddsbd_epi32(__A);
}

__m128i test_mm256_mask_phraddsbd_epi32(__mmask32 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phraddsbd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddsbd256(
  return _mm256_mask_phraddsbd_epi32(__A, __B);
}

__m128i test_mm_phraddsd_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_phraddsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddsd128(
  return _mm_phraddsd_epi32(__A);
}

__m128i test_mm_mask_phraddsd_epi32(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phraddsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddsd128(
  return _mm_mask_phraddsd_epi32(__A, __B);
}

__m128i test_mm256_phraddsd_epi32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phraddsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddsd256(
  return _mm256_phraddsd_epi32(__A);
}

__m128i test_mm256_mask_phraddsd_epi32(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phraddsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddsd256(
  return _mm256_mask_phraddsd_epi32(__A, __B);
}

__m128i test_mm_phraddsq_epi64(__m128i __A) {
  // CHECK-LABEL: @test_mm_phraddsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphraddsq128(
  return _mm_phraddsq_epi64(__A);
}

__m128i test_mm_mask_phraddsq_epi64(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phraddsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphraddsq128(
  return _mm_mask_phraddsq_epi64(__A, __B);
}

__m128i test_mm256_phraddsq_epi64(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phraddsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphraddsq256(
  return _mm256_phraddsq_epi64(__A);
}

__m128i test_mm256_mask_phraddsq_epi64(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phraddsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphraddsq256(
  return _mm256_mask_phraddsq_epi64(__A, __B);
}

__m128i test_mm_phraddswd_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_phraddswd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddswd128(
  return _mm_phraddswd_epi32(__A);
}

__m128i test_mm_mask_phraddswd_epi32(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phraddswd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddswd128(
  return _mm_mask_phraddswd_epi32(__A, __B);
}

__m128i test_mm256_phraddswd_epi32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phraddswd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphraddswd256(
  return _mm256_phraddswd_epi32(__A);
}

__m128i test_mm256_mask_phraddswd_epi32(__mmask16 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phraddswd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphraddswd256(
  return _mm256_mask_phraddswd_epi32(__A, __B);
}

__m128i test_mm_phrandb_epu8(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrandb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrandb128(
  return _mm_phrandb_epu8(__A);
}

__m128i test_mm_mask_phrandb_epu8(__mmask16 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrandb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrandb128(
  return _mm_mask_phrandb_epu8(__A, __B);
}

__m128i test_mm256_phrandb_epu8(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrandb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrandb256(
  return _mm256_phrandb_epu8(__A);
}

__m128i test_mm256_mask_phrandb_epu8(__mmask32 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrandb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrandb256(
  return _mm256_mask_phrandb_epu8(__A, __B);
}

__m128i test_mm_phrandd_epu32(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrandd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrandd128(
  return _mm_phrandd_epu32(__A);
}

__m128i test_mm_mask_phrandd_epu32(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrandd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrandd128(
  return _mm_mask_phrandd_epu32(__A, __B);
}

__m128i test_mm256_phrandd_epu32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrandd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrandd256(
  return _mm256_phrandd_epu32(__A);
}

__m128i test_mm256_mask_phrandd_epu32(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrandd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrandd256(
  return _mm256_mask_phrandd_epu32(__A, __B);
}

__m128i test_mm256_phranddq_epu64(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phranddq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphranddq256(
  return _mm256_phranddq_epu64(__A);
}

__m128i test_mm_phrandq_epu64(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrandq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrandq128(
  return _mm_phrandq_epu64(__A);
}

__m128i test_mm_mask_phrandq_epu64(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrandq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrandq128(
  return _mm_mask_phrandq_epu64(__A, __B);
}

__m128i test_mm256_phrandq_epu64(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrandq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrandq256(
  return _mm256_phrandq_epu64(__A);
}

__m128i test_mm256_mask_phrandq_epu64(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrandq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrandq256(
  return _mm256_mask_phrandq_epu64(__A, __B);
}

__m128i test_mm_phrandw_epu16(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrandw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrandw128(
  return _mm_phrandw_epu16(__A);
}

__m128i test_mm_mask_phrandw_epu16(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrandw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrandw128(
  return _mm_mask_phrandw_epu16(__A, __B);
}

__m128i test_mm256_phrandw_epu16(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrandw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrandw256(
  return _mm256_phrandw_epu16(__A);
}

__m128i test_mm256_mask_phrandw_epu16(__mmask16 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrandw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrandw256(
  return _mm256_mask_phrandw_epu16(__A, __B);
}

__m128i test_mm_phrmaxb_epu8(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrmaxb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrmaxb128(
  return _mm_phrmaxb_epu8(__A);
}

__m128i test_mm_mask_phrmaxb_epu8(__mmask16 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrmaxb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrmaxb128(
  return _mm_mask_phrmaxb_epu8(__A, __B);
}

__m128i test_mm256_phrmaxb_epu8(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrmaxb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrmaxb256(
  return _mm256_phrmaxb_epu8(__A);
}

__m128i test_mm256_mask_phrmaxb_epu8(__mmask32 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrmaxb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrmaxb256(
  return _mm256_mask_phrmaxb_epu8(__A, __B);
}

__m128i test_mm_phrmaxd_epu32(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrmaxd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrmaxd128(
  return _mm_phrmaxd_epu32(__A);
}

__m128i test_mm_mask_phrmaxd_epu32(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrmaxd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrmaxd128(
  return _mm_mask_phrmaxd_epu32(__A, __B);
}

__m128i test_mm256_phrmaxd_epu32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrmaxd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrmaxd256(
  return _mm256_phrmaxd_epu32(__A);
}

__m128i test_mm256_mask_phrmaxd_epu32(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrmaxd_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrmaxd256(
  return _mm256_mask_phrmaxd_epu32(__A, __B);
}

__m128i test_mm_phrmaxq_epu64(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrmaxq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrmaxq128(
  return _mm_phrmaxq_epu64(__A);
}

__m128i test_mm_mask_phrmaxq_epu64(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrmaxq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrmaxq128(
  return _mm_mask_phrmaxq_epu64(__A, __B);
}

__m128i test_mm256_phrmaxq_epu64(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrmaxq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrmaxq256(
  return _mm256_phrmaxq_epu64(__A);
}

__m128i test_mm256_mask_phrmaxq_epu64(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrmaxq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrmaxq256(
  return _mm256_mask_phrmaxq_epu64(__A, __B);
}

__m128i test_mm_phrmaxw_epu16(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrmaxw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrmaxw128(
  return _mm_phrmaxw_epu16(__A);
}

__m128i test_mm_mask_phrmaxw_epu16(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrmaxw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrmaxw128(
  return _mm_mask_phrmaxw_epu16(__A, __B);
}

__m128i test_mm256_phrmaxw_epu16(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrmaxw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrmaxw256(
  return _mm256_phrmaxw_epu16(__A);
}

__m128i test_mm256_mask_phrmaxw_epu16(__mmask16 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrmaxw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrmaxw256(
  return _mm256_mask_phrmaxw_epu16(__A, __B);
}

__m128i test_mm_phrmaxsb_epi8(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrmaxsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrmaxsb128(
  return _mm_phrmaxsb_epi8(__A);
}

__m128i test_mm_mask_phrmaxsb_epi8(__mmask16 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrmaxsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrmaxsb128(
  return _mm_mask_phrmaxsb_epi8(__A, __B);
}

__m128i test_mm256_phrmaxsb_epi8(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrmaxsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrmaxsb256(
  return _mm256_phrmaxsb_epi8(__A);
}

__m128i test_mm256_mask_phrmaxsb_epi8(__mmask32 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrmaxsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrmaxsb256(
  return _mm256_mask_phrmaxsb_epi8(__A, __B);
}

__m128i test_mm_phrmaxsd_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrmaxsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrmaxsd128(
  return _mm_phrmaxsd_epi32(__A);
}

__m128i test_mm_mask_phrmaxsd_epi32(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrmaxsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrmaxsd128(
  return _mm_mask_phrmaxsd_epi32(__A, __B);
}

__m128i test_mm256_phrmaxsd_epi32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrmaxsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrmaxsd256(
  return _mm256_phrmaxsd_epi32(__A);
}

__m128i test_mm256_mask_phrmaxsd_epi32(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrmaxsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrmaxsd256(
  return _mm256_mask_phrmaxsd_epi32(__A, __B);
}

__m128i test_mm_phrmaxsq_epi64(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrmaxsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrmaxsq128(
  return _mm_phrmaxsq_epi64(__A);
}

__m128i test_mm_mask_phrmaxsq_epi64(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrmaxsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrmaxsq128(
  return _mm_mask_phrmaxsq_epi64(__A, __B);
}

__m128i test_mm256_phrmaxsq_epi64(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrmaxsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrmaxsq256(
  return _mm256_phrmaxsq_epi64(__A);
}

__m128i test_mm256_mask_phrmaxsq_epi64(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrmaxsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrmaxsq256(
  return _mm256_mask_phrmaxsq_epi64(__A, __B);
}

__m128i test_mm_phrmaxsw_epi16(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrmaxsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrmaxsw128(
  return _mm_phrmaxsw_epi16(__A);
}

__m128i test_mm_mask_phrmaxsw_epi16(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrmaxsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrmaxsw128(
  return _mm_mask_phrmaxsw_epi16(__A, __B);
}

__m128i test_mm256_phrmaxsw_epi16(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrmaxsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrmaxsw256(
  return _mm256_phrmaxsw_epi16(__A);
}

__m128i test_mm256_mask_phrmaxsw_epi16(__mmask16 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrmaxsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrmaxsw256(
  return _mm256_mask_phrmaxsw_epi16(__A, __B);
}

__m128i test_mm_phrminb_epu8(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrminb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrminb128(
  return _mm_phrminb_epu8(__A);
}

__m128i test_mm_mask_phrminb_epu8(__mmask16 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrminb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrminb128(
  return _mm_mask_phrminb_epu8(__A, __B);
}

__m128i test_mm256_phrminb_epu8(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrminb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrminb256(
  return _mm256_phrminb_epu8(__A);
}

__m128i test_mm256_mask_phrminb_epu8(__mmask32 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrminb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrminb256(
  return _mm256_mask_phrminb_epu8(__A, __B);
}

__m128i test_mm_phrmind_epu32(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrmind_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrmind128(
  return _mm_phrmind_epu32(__A);
}

__m128i test_mm_mask_phrmind_epu32(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrmind_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrmind128(
  return _mm_mask_phrmind_epu32(__A, __B);
}

__m128i test_mm256_phrmind_epu32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrmind_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrmind256(
  return _mm256_phrmind_epu32(__A);
}

__m128i test_mm256_mask_phrmind_epu32(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrmind_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrmind256(
  return _mm256_mask_phrmind_epu32(__A, __B);
}

__m128i test_mm_phrminq_epu64(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrminq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrminq128(
  return _mm_phrminq_epu64(__A);
}

__m128i test_mm_mask_phrminq_epu64(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrminq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrminq128(
  return _mm_mask_phrminq_epu64(__A, __B);
}

__m128i test_mm256_phrminq_epu64(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrminq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrminq256(
  return _mm256_phrminq_epu64(__A);
}

__m128i test_mm256_mask_phrminq_epu64(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrminq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrminq256(
  return _mm256_mask_phrminq_epu64(__A, __B);
}

__m128i test_mm_phrminw_epu16(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrminw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrminw128(
  return _mm_phrminw_epu16(__A);
}

__m128i test_mm_mask_phrminw_epu16(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrminw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrminw128(
  return _mm_mask_phrminw_epu16(__A, __B);
}

__m128i test_mm256_phrminw_epu16(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrminw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrminw256(
  return _mm256_phrminw_epu16(__A);
}

__m128i test_mm256_mask_phrminw_epu16(__mmask16 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrminw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrminw256(
  return _mm256_mask_phrminw_epu16(__A, __B);
}

__m128i test_mm_phrminsb_epi8(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrminsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrminsb128(
  return _mm_phrminsb_epi8(__A);
}

__m128i test_mm_mask_phrminsb_epi8(__mmask16 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrminsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrminsb128(
  return _mm_mask_phrminsb_epi8(__A, __B);
}

__m128i test_mm256_phrminsb_epi8(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrminsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrminsb256(
  return _mm256_phrminsb_epi8(__A);
}

__m128i test_mm256_mask_phrminsb_epi8(__mmask32 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrminsb_epi8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrminsb256(
  return _mm256_mask_phrminsb_epi8(__A, __B);
}

__m128i test_mm_phrminsd_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrminsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrminsd128(
  return _mm_phrminsd_epi32(__A);
}

__m128i test_mm_mask_phrminsd_epi32(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrminsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrminsd128(
  return _mm_mask_phrminsd_epi32(__A, __B);
}

__m128i test_mm256_phrminsd_epi32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrminsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrminsd256(
  return _mm256_phrminsd_epi32(__A);
}

__m128i test_mm256_mask_phrminsd_epi32(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrminsd_epi32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrminsd256(
  return _mm256_mask_phrminsd_epi32(__A, __B);
}

__m128i test_mm_phrminsq_epi64(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrminsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrminsq128(
  return _mm_phrminsq_epi64(__A);
}

__m128i test_mm_mask_phrminsq_epi64(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrminsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrminsq128(
  return _mm_mask_phrminsq_epi64(__A, __B);
}

__m128i test_mm256_phrminsq_epi64(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrminsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrminsq256(
  return _mm256_phrminsq_epi64(__A);
}

__m128i test_mm256_mask_phrminsq_epi64(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrminsq_epi64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrminsq256(
  return _mm256_mask_phrminsq_epi64(__A, __B);
}

__m128i test_mm_phrminsw_epi16(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrminsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrminsw128(
  return _mm_phrminsw_epi16(__A);
}

__m128i test_mm_mask_phrminsw_epi16(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrminsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrminsw128(
  return _mm_mask_phrminsw_epi16(__A, __B);
}

__m128i test_mm256_phrminsw_epi16(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrminsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrminsw256(
  return _mm256_phrminsw_epi16(__A);
}

__m128i test_mm256_mask_phrminsw_epi16(__mmask16 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrminsw_epi16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrminsw256(
  return _mm256_mask_phrminsw_epi16(__A, __B);
}

__m128i test_mm_phrorb_epu8(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrorb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrorb128(
  return _mm_phrorb_epu8(__A);
}

__m128i test_mm_mask_phrorb_epu8(__mmask16 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrorb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrorb128(
  return _mm_mask_phrorb_epu8(__A, __B);
}

__m128i test_mm256_phrorb_epu8(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrorb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrorb256(
  return _mm256_phrorb_epu8(__A);
}

__m128i test_mm256_mask_phrorb_epu8(__mmask32 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrorb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrorb256(
  return _mm256_mask_phrorb_epu8(__A, __B);
}

__m128i test_mm_phrord_epu32(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrord_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrord128(
  return _mm_phrord_epu32(__A);
}

__m128i test_mm_mask_phrord_epu32(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrord_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrord128(
  return _mm_mask_phrord_epu32(__A, __B);
}

__m128i test_mm256_phrord_epu32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrord_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrord256(
  return _mm256_phrord_epu32(__A);
}

__m128i test_mm256_mask_phrord_epu32(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrord_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrord256(
  return _mm256_mask_phrord_epu32(__A, __B);
}

__m128i test_mm256_phrordq_epu64(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrordq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrordq256(
  return _mm256_phrordq_epu64(__A);
}

__m128i test_mm_phrorq_epu64(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrorq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrorq128(
  return _mm_phrorq_epu64(__A);
}

__m128i test_mm_mask_phrorq_epu64(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrorq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrorq128(
  return _mm_mask_phrorq_epu64(__A, __B);
}

__m128i test_mm256_phrorq_epu64(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrorq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrorq256(
  return _mm256_phrorq_epu64(__A);
}

__m128i test_mm256_mask_phrorq_epu64(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrorq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrorq256(
  return _mm256_mask_phrorq_epu64(__A, __B);
}

__m128i test_mm_phrorw_epu16(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrorw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrorw128(
  return _mm_phrorw_epu16(__A);
}

__m128i test_mm_mask_phrorw_epu16(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrorw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrorw128(
  return _mm_mask_phrorw_epu16(__A, __B);
}

__m128i test_mm256_phrorw_epu16(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrorw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrorw256(
  return _mm256_phrorw_epu16(__A);
}

__m128i test_mm256_mask_phrorw_epu16(__mmask16 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrorw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrorw256(
  return _mm256_mask_phrorw_epu16(__A, __B);
}

__m128i test_mm_phrxorb_epu8(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrxorb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrxorb128(
  return _mm_phrxorb_epu8(__A);
}

__m128i test_mm_mask_phrxorb_epu8(__mmask16 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrxorb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrxorb128(
  return _mm_mask_phrxorb_epu8(__A, __B);
}

__m128i test_mm256_phrxorb_epu8(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrxorb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.vphrxorb256(
  return _mm256_phrxorb_epu8(__A);
}

__m128i test_mm256_mask_phrxorb_epu8(__mmask32 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrxorb_epu8(
  // CHECK: call <16 x i8> @llvm.x86.avx512reduction.mask.vphrxorb256(
  return _mm256_mask_phrxorb_epu8(__A, __B);
}

__m128i test_mm_phrxord_epu32(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrxord_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrxord128(
  return _mm_phrxord_epu32(__A);
}

__m128i test_mm_mask_phrxord_epu32(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrxord_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrxord128(
  return _mm_mask_phrxord_epu32(__A, __B);
}

__m128i test_mm256_phrxord_epu32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrxord_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.vphrxord256(
  return _mm256_phrxord_epu32(__A);
}

__m128i test_mm256_mask_phrxord_epu32(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrxord_epu32(
  // CHECK: call <4 x i32> @llvm.x86.avx512reduction.mask.vphrxord256(
  return _mm256_mask_phrxord_epu32(__A, __B);
}

__m128i test_mm256_phrxordq_epu64(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrxordq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrxordq256(
  return _mm256_phrxordq_epu64(__A);
}

__m128i test_mm_phrxorq_epu64(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrxorq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrxorq128(
  return _mm_phrxorq_epu64(__A);
}

__m128i test_mm_mask_phrxorq_epu64(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrxorq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrxorq128(
  return _mm_mask_phrxorq_epu64(__A, __B);
}

__m128i test_mm256_phrxorq_epu64(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrxorq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.vphrxorq256(
  return _mm256_phrxorq_epu64(__A);
}

__m128i test_mm256_mask_phrxorq_epu64(__mmask8 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrxorq_epu64(
  // CHECK: call <2 x i64> @llvm.x86.avx512reduction.mask.vphrxorq256(
  return _mm256_mask_phrxorq_epu64(__A, __B);
}

__m128i test_mm_phrxorw_epu16(__m128i __A) {
  // CHECK-LABEL: @test_mm_phrxorw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrxorw128(
  return _mm_phrxorw_epu16(__A);
}

__m128i test_mm_mask_phrxorw_epu16(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_phrxorw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrxorw128(
  return _mm_mask_phrxorw_epu16(__A, __B);
}

__m128i test_mm256_phrxorw_epu16(__m256i __A) {
  // CHECK-LABEL: @test_mm256_phrxorw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.vphrxorw256(
  return _mm256_phrxorw_epu16(__A);
}

__m128i test_mm256_mask_phrxorw_epu16(__mmask16 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_phrxorw_epu16(
  // CHECK: call <8 x i16> @llvm.x86.avx512reduction.mask.vphrxorw256(
  return _mm256_mask_phrxorw_epu16(__A, __B);
}

