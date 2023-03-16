// REQUIRES: intel_feature_isa_avx512_sat_cvt
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512vl -target-feature +avx512satcvt \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_cvtnebf162ibs_epi8(__m128bh __A) {
  // CHECK-LABEL: @test_mm_cvtnebf162ibs_epi8(
  // CHECK: @llvm.x86.vcvtnebf162ibs128
  return _mm_cvtnebf162ibs_epi8(__A);
}

__m128i test_mm_mask_cvtnebf162ibs_epi8(__m128i __S, __mmask8 __A, __m128bh __B) {
  // CHECK-LABEL: @test_mm_mask_cvtnebf162ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvtnebf162ibs128
  return _mm_mask_cvtnebf162ibs_epi8(__S, __A, __B);
}

__m128i test_mm_maskz_cvtnebf162ibs_epi8(__mmask8 __A, __m128bh __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtnebf162ibs_epi8(
  // CHECK: @llvm.x86.maskz.vcvtnebf162ibs128
  return _mm_maskz_cvtnebf162ibs_epi8(__A, __B);
}

__m256i test_mm256_cvtnebf162ibs_epi8(__m256bh __A) {
  // CHECK-LABEL: @test_mm256_cvtnebf162ibs_epi8(
  // CHECK: @llvm.x86.vcvtnebf162ibs256
  return _mm256_cvtnebf162ibs_epi8(__A);
}

__m256i test_mm256_mask_cvtnebf162ibs_epi8(__m256i __S, __mmask16 __A, __m256bh __B) {
  // CHECK-LABEL: @test_mm256_mask_cvtnebf162ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvtnebf162ibs256
  return _mm256_mask_cvtnebf162ibs_epi8(__S, __A, __B);
}

__m256i test_mm256_maskz_cvtnebf162ibs_epi8(__mmask16 __A, __m256bh __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtnebf162ibs_epi8(
  // CHECK: @llvm.x86.maskz.vcvtnebf162ibs256
  return _mm256_maskz_cvtnebf162ibs_epi8(__A, __B);
}

__m128i test_mm_cvtnebf162iubs_epi8(__m128bh __A) {
  // CHECK-LABEL: @test_mm_cvtnebf162iubs_epi8(
  // CHECK: @llvm.x86.vcvtnebf162iubs128
  return _mm_cvtnebf162iubs_epi8(__A);
}

__m128i test_mm_mask_cvtnebf162iubs_epi8(__m128i __S, __mmask8 __A, __m128bh __B) {
  // CHECK-LABEL: @test_mm_mask_cvtnebf162iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvtnebf162iubs128
  return _mm_mask_cvtnebf162iubs_epi8(__S, __A, __B);
}

__m128i test_mm_maskz_cvtnebf162iubs_epi8(__mmask8 __A, __m128bh __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtnebf162iubs_epi8(
  // CHECK: @llvm.x86.maskz.vcvtnebf162iubs128
  return _mm_maskz_cvtnebf162iubs_epi8(__A, __B);
}

__m256i test_mm256_cvtnebf162iubs_epi8(__m256bh __A) {
  // CHECK-LABEL: @test_mm256_cvtnebf162iubs_epi8(
  // CHECK: @llvm.x86.vcvtnebf162iubs256
  return _mm256_cvtnebf162iubs_epi8(__A);
}

__m256i test_mm256_mask_cvtnebf162iubs_epi8(__m256i __S, __mmask16 __A, __m256bh __B) {
  // CHECK-LABEL: @test_mm256_mask_cvtnebf162iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvtnebf162iubs256
  return _mm256_mask_cvtnebf162iubs_epi8(__S, __A, __B);
}

__m256i test_mm256_maskz_cvtnebf162iubs_epi8(__mmask16 __A, __m256bh __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtnebf162iubs_epi8(
  // CHECK: @llvm.x86.maskz.vcvtnebf162iubs256
  return _mm256_maskz_cvtnebf162iubs_epi8(__A, __B);
}

__m128i test_mm_cvtph2ibs_epi8(__m128h __A) {
  // CHECK-LABEL: @test_mm_cvtph2ibs_epi8(
  // CHECK: @llvm.x86.vcvtph2ibs128
  return _mm_cvtph2ibs_epi8(__A);
}

__m128i test_mm_mask_cvtph2ibs_epi8(__m128i __S, __mmask8 __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_mask_cvtph2ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvtph2ibs128
  return _mm_mask_cvtph2ibs_epi8(__S, __A, __B);
}

__m128i test_mm_maskz_cvtph2ibs_epi8(__mmask8 __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtph2ibs_epi8(
  // CHECK: @llvm.x86.maskz.vcvtph2ibs128
  return _mm_maskz_cvtph2ibs_epi8(__A, __B);
}

__m256i test_mm256_cvtph2ibs_epi8(__m256h __A) {
  // CHECK-LABEL: @test_mm256_cvtph2ibs_epi8(
  // CHECK: @llvm.x86.vcvtph2ibs256
  return _mm256_cvtph2ibs_epi8(__A);
}

__m256i test_mm256_mask_cvtph2ibs_epi8(__m256i __S, __mmask16 __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_mask_cvtph2ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvtph2ibs256
  return _mm256_mask_cvtph2ibs_epi8(__S, __A, __B);
}

__m256i test_mm256_maskz_cvtph2ibs_epi8(__mmask16 __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtph2ibs_epi8(
  // CHECK: @llvm.x86.maskz.vcvtph2ibs256
  return _mm256_maskz_cvtph2ibs_epi8(__A, __B);
}

__m128i test_mm_cvtph2iubs_epi8(__m128h __A) {
  // CHECK-LABEL: @test_mm_cvtph2iubs_epi8(
  // CHECK: @llvm.x86.vcvtph2iubs128
  return _mm_cvtph2iubs_epi8(__A);
}

__m128i test_mm_mask_cvtph2iubs_epi8(__m128i __S, __mmask8 __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_mask_cvtph2iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvtph2iubs128
  return _mm_mask_cvtph2iubs_epi8(__S, __A, __B);
}

__m128i test_mm_maskz_cvtph2iubs_epi8(__mmask8 __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtph2iubs_epi8(
  // CHECK: @llvm.x86.maskz.vcvtph2iubs128
  return _mm_maskz_cvtph2iubs_epi8(__A, __B);
}

__m256i test_mm256_cvtph2iubs_epi8(__m256h __A) {
  // CHECK-LABEL: @test_mm256_cvtph2iubs_epi8(
  // CHECK: @llvm.x86.vcvtph2iubs256
  return _mm256_cvtph2iubs_epi8(__A);
}

__m256i test_mm256_mask_cvtph2iubs_epi8(__m256i __S, __mmask16 __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_mask_cvtph2iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvtph2iubs256
  return _mm256_mask_cvtph2iubs_epi8(__S, __A, __B);
}

__m256i test_mm256_maskz_cvtph2iubs_epi8(__mmask16 __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtph2iubs_epi8(
  // CHECK: @llvm.x86.maskz.vcvtph2iubs256
  return _mm256_maskz_cvtph2iubs_epi8(__A, __B);
}

__m128i test_mm_cvtps2ibs_epi8(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvtps2ibs_epi8(
  // CHECK: @llvm.x86.vcvtps2ibs128
  return _mm_cvtps2ibs_epi8(__A);
}

__m128i test_mm_mask_cvtps2ibs_epi8(__m128i __S, __mmask8 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_cvtps2ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvtps2ibs128
  return _mm_mask_cvtps2ibs_epi8(__S, __A, __B);
}

__m128i test_mm_maskz_cvtps2ibs_epi8(__mmask8 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtps2ibs_epi8(
  // CHECK: @llvm.x86.maskz.vcvtps2ibs128
  return _mm_maskz_cvtps2ibs_epi8(__A, __B);
}

__m256i test_mm256_cvtps2ibs_epi8(__m256 __A) {
  // CHECK-LABEL: @test_mm256_cvtps2ibs_epi8(
  // CHECK: @llvm.x86.vcvtps2ibs256
  return _mm256_cvtps2ibs_epi8(__A);
}

__m256i test_mm256_mask_cvtps2ibs_epi8(__m256i __S, __mmask8 __A, __m256 __B) {
  // CHECK-LABEL: @test_mm256_mask_cvtps2ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvtps2ibs256
  return _mm256_mask_cvtps2ibs_epi8(__S, __A, __B);
}

__m256i test_mm256_maskz_cvtps2ibs_epi8(__mmask8 __A, __m256 __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtps2ibs_epi8(
  // CHECK: @llvm.x86.maskz.vcvtps2ibs256
  return _mm256_maskz_cvtps2ibs_epi8(__A, __B);
}

__m128i test_mm_cvtps2iubs_epi8(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvtps2iubs_epi8(
  // CHECK: @llvm.x86.vcvtps2iubs128
  return _mm_cvtps2iubs_epi8(__A);
}

__m128i test_mm_mask_cvtps2iubs_epi8(__m128i __S, __mmask8 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_cvtps2iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvtps2iubs128
  return _mm_mask_cvtps2iubs_epi8(__S, __A, __B);
}

__m128i test_mm_maskz_cvtps2iubs_epi8(__mmask8 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtps2iubs_epi8(
  // CHECK: @llvm.x86.maskz.vcvtps2iubs128
  return _mm_maskz_cvtps2iubs_epi8(__A, __B);
}

__m256i test_mm256_cvtps2iubs_epi8(__m256 __A) {
  // CHECK-LABEL: @test_mm256_cvtps2iubs_epi8(
  // CHECK: @llvm.x86.vcvtps2iubs256
  return _mm256_cvtps2iubs_epi8(__A);
}

__m256i test_mm256_mask_cvtps2iubs_epi8(__m256i __S, __mmask8 __A, __m256 __B) {
  // CHECK-LABEL: @test_mm256_mask_cvtps2iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvtps2iubs256
  return _mm256_mask_cvtps2iubs_epi8(__S, __A, __B);
}

__m256i test_mm256_maskz_cvtps2iubs_epi8(__mmask8 __A, __m256 __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtps2iubs_epi8(
  // CHECK: @llvm.x86.maskz.vcvtps2iubs256
  return _mm256_maskz_cvtps2iubs_epi8(__A, __B);
}

__m128i test_mm_cvttnebf162ibs_epi8(__m128bh __A) {
  // CHECK-LABEL: @test_mm_cvttnebf162ibs_epi8(
  // CHECK: @llvm.x86.vcvttnebf162ibs128
  return _mm_cvttnebf162ibs_epi8(__A);
}

__m128i test_mm_mask_cvttnebf162ibs_epi8(__m128i __S, __mmask8 __A, __m128bh __B) {
  // CHECK-LABEL: @test_mm_mask_cvttnebf162ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvttnebf162ibs128
  return _mm_mask_cvttnebf162ibs_epi8(__S, __A, __B);
}

__m128i test_mm_maskz_cvttnebf162ibs_epi8(__mmask8 __A, __m128bh __B) {
  // CHECK-LABEL: @test_mm_maskz_cvttnebf162ibs_epi8(
  // CHECK: @llvm.x86.maskz.vcvttnebf162ibs128
  return _mm_maskz_cvttnebf162ibs_epi8(__A, __B);
}

__m256i test_mm256_cvttnebf162ibs_epi8(__m256bh __A) {
  // CHECK-LABEL: @test_mm256_cvttnebf162ibs_epi8(
  // CHECK: @llvm.x86.vcvttnebf162ibs256
  return _mm256_cvttnebf162ibs_epi8(__A);
}

__m256i test_mm256_mask_cvttnebf162ibs_epi8(__m256i __S, __mmask16 __A, __m256bh __B) {
  // CHECK-LABEL: @test_mm256_mask_cvttnebf162ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvttnebf162ibs256
  return _mm256_mask_cvttnebf162ibs_epi8(__S, __A, __B);
}

__m256i test_mm256_maskz_cvttnebf162ibs_epi8(__mmask16 __A, __m256bh __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvttnebf162ibs_epi8(
  // CHECK: @llvm.x86.maskz.vcvttnebf162ibs256
  return _mm256_maskz_cvttnebf162ibs_epi8(__A, __B);
}

__m128i test_mm_cvttnebf162iubs_epi8(__m128bh __A) {
  // CHECK-LABEL: @test_mm_cvttnebf162iubs_epi8(
  // CHECK: @llvm.x86.vcvttnebf162iubs128
  return _mm_cvttnebf162iubs_epi8(__A);
}

__m128i test_mm_mask_cvttnebf162iubs_epi8(__m128i __S, __mmask8 __A, __m128bh __B) {
  // CHECK-LABEL: @test_mm_mask_cvttnebf162iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvttnebf162iubs128
  return _mm_mask_cvttnebf162iubs_epi8(__S, __A, __B);
}

__m128i test_mm_maskz_cvttnebf162iubs_epi8(__mmask8 __A, __m128bh __B) {
  // CHECK-LABEL: @test_mm_maskz_cvttnebf162iubs_epi8(
  // CHECK: @llvm.x86.maskz.vcvttnebf162iubs128
  return _mm_maskz_cvttnebf162iubs_epi8(__A, __B);
}

__m256i test_mm256_cvttnebf162iubs_epi8(__m256bh __A) {
  // CHECK-LABEL: @test_mm256_cvttnebf162iubs_epi8(
  // CHECK: @llvm.x86.vcvttnebf162iubs256
  return _mm256_cvttnebf162iubs_epi8(__A);
}

__m256i test_mm256_mask_cvttnebf162iubs_epi8(__m256i __S, __mmask16 __A, __m256bh __B) {
  // CHECK-LABEL: @test_mm256_mask_cvttnebf162iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvttnebf162iubs256
  return _mm256_mask_cvttnebf162iubs_epi8(__S, __A, __B);
}

__m256i test_mm256_maskz_cvttnebf162iubs_epi8(__mmask16 __A, __m256bh __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvttnebf162iubs_epi8(
  // CHECK: @llvm.x86.maskz.vcvttnebf162iubs256
  return _mm256_maskz_cvttnebf162iubs_epi8(__A, __B);
}

__m128i test_mm_cvttph2ibs_epi8(__m128h __A) {
  // CHECK-LABEL: @test_mm_cvttph2ibs_epi8(
  // CHECK: @llvm.x86.vcvttph2ibs128
  return _mm_cvttph2ibs_epi8(__A);
}

__m128i test_mm_mask_cvttph2ibs_epi8(__m128i __S, __mmask8 __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_mask_cvttph2ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvttph2ibs128
  return _mm_mask_cvttph2ibs_epi8(__S, __A, __B);
}

__m128i test_mm_maskz_cvttph2ibs_epi8(__mmask8 __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_cvttph2ibs_epi8(
  // CHECK: @llvm.x86.maskz.vcvttph2ibs128
  return _mm_maskz_cvttph2ibs_epi8(__A, __B);
}

__m256i test_mm256_cvttph2ibs_epi8(__m256h __A) {
  // CHECK-LABEL: @test_mm256_cvttph2ibs_epi8(
  // CHECK: @llvm.x86.vcvttph2ibs256
  return _mm256_cvttph2ibs_epi8(__A);
}

__m256i test_mm256_mask_cvttph2ibs_epi8(__m256i __S, __mmask16 __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_mask_cvttph2ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvttph2ibs256
  return _mm256_mask_cvttph2ibs_epi8(__S, __A, __B);
}

__m256i test_mm256_maskz_cvttph2ibs_epi8(__mmask16 __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvttph2ibs_epi8(
  // CHECK: @llvm.x86.maskz.vcvttph2ibs256
  return _mm256_maskz_cvttph2ibs_epi8(__A, __B);
}

__m128i test_mm_cvttph2iubs_epi8(__m128h __A) {
  // CHECK-LABEL: @test_mm_cvttph2iubs_epi8(
  // CHECK: @llvm.x86.vcvttph2iubs128
  return _mm_cvttph2iubs_epi8(__A);
}

__m128i test_mm_mask_cvttph2iubs_epi8(__m128i __S, __mmask8 __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_mask_cvttph2iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvttph2iubs128
  return _mm_mask_cvttph2iubs_epi8(__S, __A, __B);
}

__m128i test_mm_maskz_cvttph2iubs_epi8(__mmask8 __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_cvttph2iubs_epi8(
  // CHECK: @llvm.x86.maskz.vcvttph2iubs128
  return _mm_maskz_cvttph2iubs_epi8(__A, __B);
}

__m256i test_mm256_cvttph2iubs_epi8(__m256h __A) {
  // CHECK-LABEL: @test_mm256_cvttph2iubs_epi8(
  // CHECK: @llvm.x86.vcvttph2iubs256
  return _mm256_cvttph2iubs_epi8(__A);
}

__m256i test_mm256_mask_cvttph2iubs_epi8(__m256i __S, __mmask16 __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_mask_cvttph2iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvttph2iubs256
  return _mm256_mask_cvttph2iubs_epi8(__S, __A, __B);
}

__m256i test_mm256_maskz_cvttph2iubs_epi8(__mmask16 __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvttph2iubs_epi8(
  // CHECK: @llvm.x86.maskz.vcvttph2iubs256
  return _mm256_maskz_cvttph2iubs_epi8(__A, __B);
}

__m128i test_mm_cvttps2ibs_epi8(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvttps2ibs_epi8(
  // CHECK: @llvm.x86.vcvttps2ibs128
  return _mm_cvttps2ibs_epi8(__A);
}

__m128i test_mm_mask_cvttps2ibs_epi8(__m128i __S, __mmask8 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_cvttps2ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvttps2ibs128
  return _mm_mask_cvttps2ibs_epi8(__S, __A, __B);
}

__m128i test_mm_maskz_cvttps2ibs_epi8(__mmask8 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_cvttps2ibs_epi8(
  // CHECK: @llvm.x86.maskz.vcvttps2ibs128
  return _mm_maskz_cvttps2ibs_epi8(__A, __B);
}

__m256i test_mm256_cvttps2ibs_epi8(__m256 __A) {
  // CHECK-LABEL: @test_mm256_cvttps2ibs_epi8(
  // CHECK: @llvm.x86.vcvttps2ibs256
  return _mm256_cvttps2ibs_epi8(__A);
}

__m256i test_mm256_mask_cvttps2ibs_epi8(__m256i __S, __mmask8 __A, __m256 __B) {
  // CHECK-LABEL: @test_mm256_mask_cvttps2ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvttps2ibs256
  return _mm256_mask_cvttps2ibs_epi8(__S, __A, __B);
}

__m256i test_mm256_maskz_cvttps2ibs_epi8(__mmask8 __A, __m256 __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvttps2ibs_epi8(
  // CHECK: @llvm.x86.maskz.vcvttps2ibs256
  return _mm256_maskz_cvttps2ibs_epi8(__A, __B);
}

__m128i test_mm_cvttps2iubs_epi8(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvttps2iubs_epi8(
  // CHECK: @llvm.x86.vcvttps2iubs128
  return _mm_cvttps2iubs_epi8(__A);
}

__m128i test_mm_mask_cvttps2iubs_epi8(__m128i __S, __mmask8 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_cvttps2iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvttps2iubs128
  return _mm_mask_cvttps2iubs_epi8(__S, __A, __B);
}

__m128i test_mm_maskz_cvttps2iubs_epi8(__mmask8 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_cvttps2iubs_epi8(
  // CHECK: @llvm.x86.maskz.vcvttps2iubs128
  return _mm_maskz_cvttps2iubs_epi8(__A, __B);
}

__m256i test_mm256_cvttps2iubs_epi8(__m256 __A) {
  // CHECK-LABEL: @test_mm256_cvttps2iubs_epi8(
  // CHECK: @llvm.x86.vcvttps2iubs256
  return _mm256_cvttps2iubs_epi8(__A);
}

__m256i test_mm256_mask_cvttps2iubs_epi8(__m256i __S, __mmask8 __A, __m256 __B) {
  // CHECK-LABEL: @test_mm256_mask_cvttps2iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvttps2iubs256
  return _mm256_mask_cvttps2iubs_epi8(__S, __A, __B);
}

__m256i test_mm256_maskz_cvttps2iubs_epi8(__mmask8 __A, __m256 __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvttps2iubs_epi8(
  // CHECK: @llvm.x86.maskz.vcvttps2iubs256
  return _mm256_maskz_cvttps2iubs_epi8(__A, __B);
}
