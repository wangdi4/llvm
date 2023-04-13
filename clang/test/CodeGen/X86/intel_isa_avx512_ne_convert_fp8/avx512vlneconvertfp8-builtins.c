// REQUIRES: intel_feature_isa_avx512_ne_convert_fp8
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512neconvertfp8 -target-feature +avx512vl \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_cvtbias2ph_pbf8(__m128i __A, __m128h __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_cvtbias2ph_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.vcvtbias2ph2bf8128(
  return _mm_cvtbias2ph_pbf8(__A, __B, __C);
}

__m256i test_mm256_cvtbias2ph_pbf8(__m256i __A, __m256h __B, __m256h __C) {
  // CHECK-LABEL: @test_mm256_cvtbias2ph_pbf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.vcvtbias2ph2bf8256(
  return _mm256_cvtbias2ph_pbf8(__A, __B, __C);
}

__m128i test_mm_cvtbias2ph2bf8s_pbf8(__m128i __A, __m128h __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_cvtbias2ph2bf8s_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.vcvtbias2ph2bf8s128(
  return _mm_cvtbias2ph2bf8s_pbf8(__A, __B, __C);
}

__m256i test_mm256_cvtbias2ph2bf8s_pbf8(__m256i __A, __m256h __B, __m256h __C) {
  // CHECK-LABEL: @test_mm256_cvtbias2ph2bf8s_pbf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.vcvtbias2ph2bf8s256(
  return _mm256_cvtbias2ph2bf8s_pbf8(__A, __B, __C);
}

__m128i test_mm_cvtbias2ph_phf8(__m128i __A, __m128h __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_cvtbias2ph_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.vcvtbias2ph2hf8128(
  return _mm_cvtbias2ph_phf8(__A, __B, __C);
}

__m256i test_mm256_cvtbias2ph_phf8(__m256i __A, __m256h __B, __m256h __C) {
  // CHECK-LABEL: @test_mm256_cvtbias2ph_phf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.vcvtbias2ph2hf8256(
  return _mm256_cvtbias2ph_phf8(__A, __B, __C);
}

__m128i test_mm_cvtbias2ph2hf8s_phf8(__m128i __A, __m128h __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_cvtbias2ph2hf8s_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.vcvtbias2ph2hf8s128(
  return _mm_cvtbias2ph2hf8s_phf8(__A, __B, __C);
}

__m256i test_mm256_cvtbias2ph2hf8s_phf8(__m256i __A, __m256h __B, __m256h __C) {
  // CHECK-LABEL: @test_mm256_cvtbias2ph2hf8s_phf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.vcvtbias2ph2hf8s256(
  return _mm256_cvtbias2ph2hf8s_phf8(__A, __B, __C);
}

__m128i test_mm_cvtbiasph_pbf8(__m128h __A) {
  // CHECK-LABEL: @test_mm_cvtbiasph_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8128(
  return _mm_cvtbiasph_pbf8(__A);
}

__m128i test_mm_mask_cvtbiasph_pbf8(__m128i __A, __mmask8 __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_mask_cvtbiasph_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8128(
  return _mm_mask_cvtbiasph_pbf8(__A, __B, __C);
}

__m128i test_mm_maskz_cvtbiasph_pbf8(__mmask8 __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtbiasph_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8128(
  return _mm_maskz_cvtbiasph_pbf8(__A, __B);
}

__m128i test_mm256_cvtbiasph_pbf8(__m256h __A) {
  // CHECK-LABEL: @test_mm256_cvtbiasph_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8256(
  return _mm256_cvtbiasph_pbf8(__A);
}

__m128i test_mm256_mask_cvtbiasph_pbf8(__m128i __A, __mmask16 __B, __m256h __C) {
  // CHECK-LABEL: @test_mm256_mask_cvtbiasph_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8256(
  return _mm256_mask_cvtbiasph_pbf8(__A, __B, __C);
}

__m128i test_mm256_maskz_cvtbiasph_pbf8(__mmask16 __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtbiasph_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8256(
  return _mm256_maskz_cvtbiasph_pbf8(__A, __B);
}

__m128i test_mm_cvtbiasph2bf8s_pbf8(__m128h __A) {
  // CHECK-LABEL: @test_mm_cvtbiasph2bf8s_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8s128(
  return _mm_cvtbiasph2bf8s_pbf8(__A);
}

__m128i test_mm_mask_cvtbiasph2bf8s_pbf8(__m128i __A, __mmask8 __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_mask_cvtbiasph2bf8s_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8s128(
  return _mm_mask_cvtbiasph2bf8s_pbf8(__A, __B, __C);
}

__m128i test_mm_maskz_cvtbiasph2bf8s_pbf8(__mmask8 __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtbiasph2bf8s_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8s128(
  return _mm_maskz_cvtbiasph2bf8s_pbf8(__A, __B);
}

__m128i test_mm256_cvtbiasph2bf8s_pbf8(__m256h __A) {
  // CHECK-LABEL: @test_mm256_cvtbiasph2bf8s_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8s256(
  return _mm256_cvtbiasph2bf8s_pbf8(__A);
}

__m128i test_mm256_mask_cvtbiasph2bf8s_pbf8(__m128i __A, __mmask16 __B, __m256h __C) {
  // CHECK-LABEL: @test_mm256_mask_cvtbiasph2bf8s_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8s256(
  return _mm256_mask_cvtbiasph2bf8s_pbf8(__A, __B, __C);
}

__m128i test_mm256_maskz_cvtbiasph2bf8s_pbf8(__mmask16 __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtbiasph2bf8s_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8s256(
  return _mm256_maskz_cvtbiasph2bf8s_pbf8(__A, __B);
}

__m128i test_mm_cvtbiasph_phf8(__m128h __A) {
  // CHECK-LABEL: @test_mm_cvtbiasph_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8128(
  return _mm_cvtbiasph_phf8(__A);
}

__m128i test_mm_mask_cvtbiasph_phf8(__m128i __A, __mmask8 __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_mask_cvtbiasph_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8128(
  return _mm_mask_cvtbiasph_phf8(__A, __B, __C);
}

__m128i test_mm_maskz_cvtbiasph_phf8(__mmask8 __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtbiasph_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8128(
  return _mm_maskz_cvtbiasph_phf8(__A, __B);
}

__m128i test_mm256_cvtbiasph_phf8(__m256h __A) {
  // CHECK-LABEL: @test_mm256_cvtbiasph_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8256(
  return _mm256_cvtbiasph_phf8(__A);
}

__m128i test_mm256_mask_cvtbiasph_phf8(__m128i __A, __mmask16 __B, __m256h __C) {
  // CHECK-LABEL: @test_mm256_mask_cvtbiasph_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8256(
  return _mm256_mask_cvtbiasph_phf8(__A, __B, __C);
}

__m128i test_mm256_maskz_cvtbiasph_phf8(__mmask16 __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtbiasph_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8256(
  return _mm256_maskz_cvtbiasph_phf8(__A, __B);
}

__m128i test_mm_cvtbiasph2hf8s_phf8(__m128h __A) {
  // CHECK-LABEL: @test_mm_cvtbiasph2hf8s_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8s128(
  return _mm_cvtbiasph2hf8s_phf8(__A);
}

__m128i test_mm_mask_cvtbiasph2hf8s_phf8(__m128i __A, __mmask8 __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_mask_cvtbiasph2hf8s_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8s128(
  return _mm_mask_cvtbiasph2hf8s_phf8(__A, __B, __C);
}

__m128i test_mm_maskz_cvtbiasph2hf8s_phf8(__mmask8 __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtbiasph2hf8s_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8s128(
  return _mm_maskz_cvtbiasph2hf8s_phf8(__A, __B);
}

__m128i test_mm256_cvtbiasph2hf8s_phf8(__m256h __A) {
  // CHECK-LABEL: @test_mm256_cvtbiasph2hf8s_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8s256(
  return _mm256_cvtbiasph2hf8s_phf8(__A);
}

__m128i test_mm256_mask_cvtbiasph2hf8s_phf8(__m128i __A, __mmask16 __B, __m256h __C) {
  // CHECK-LABEL: @test_mm256_mask_cvtbiasph2hf8s_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8s256(
  return _mm256_mask_cvtbiasph2hf8s_phf8(__A, __B, __C);
}

__m128i test_mm256_maskz_cvtbiasph2hf8s_phf8(__mmask16 __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtbiasph2hf8s_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8s256(
  return _mm256_maskz_cvtbiasph2hf8s_phf8(__A, __B);
}

__m128i test_mm_cvtne2ph_pbf8(__m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_cvtne2ph_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.vcvtne2ph2bf8128(
  return _mm_cvtne2ph_pbf8(__A, __B);
}

__m256i test_mm256_cvtne2ph_pbf8(__m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_cvtne2ph_pbf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.vcvtne2ph2bf8256(
  return _mm256_cvtne2ph_pbf8(__A, __B);
}

__m128i test_mm_cvtne2ph2bf8s_pbf8(__m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_cvtne2ph2bf8s_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.vcvtne2ph2bf8s128(
  return _mm_cvtne2ph2bf8s_pbf8(__A, __B);
}

__m256i test_mm256_cvtne2ph2bf8s_pbf8(__m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_cvtne2ph2bf8s_pbf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.vcvtne2ph2bf8s256(
  return _mm256_cvtne2ph2bf8s_pbf8(__A, __B);
}

__m128i test_mm_cvtne2ph_phf8(__m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_cvtne2ph_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.vcvtne2ph2hf8128(
  return _mm_cvtne2ph_phf8(__A, __B);
}

__m256i test_mm256_cvtne2ph_phf8(__m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_cvtne2ph_phf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.vcvtne2ph2hf8256(
  return _mm256_cvtne2ph_phf8(__A, __B);
}

__m128i test_mm_cvtne2ph2hf8s_phf8(__m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_cvtne2ph2hf8s_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.vcvtne2ph2hf8s128(
  return _mm_cvtne2ph2hf8s_phf8(__A, __B);
}

__m256i test_mm256_cvtne2ph2hf8s_phf8(__m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_cvtne2ph2hf8s_phf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.vcvtne2ph2hf8s256(
  return _mm256_cvtne2ph2hf8s_phf8(__A, __B);
}

__m128h test_mm_cvtnebf8_ph(__m128i __A) {
  // CHECK-LABEL: @test_mm_cvtnebf8_ph(
  // CHECK: call <8 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnebf82ph128(
  return _mm_cvtnebf8_ph(__A);
}

__m128h test_mm_mask_cvtnebf8_ph(__m128h __A, __mmask8 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_cvtnebf8_ph(
  // CHECK: call <8 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnebf82ph128(
  return _mm_mask_cvtnebf8_ph(__A, __B, __C);
}

__m128h test_mm_maskz_cvtnebf8_ph(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtnebf8_ph(
  // CHECK: call <8 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnebf82ph128(
  return _mm_maskz_cvtnebf8_ph(__A, __B);
}

__m256h test_mm256_cvtnebf8_ph(__m128i __A) {
  // CHECK-LABEL: @test_mm256_cvtnebf8_ph(
  // CHECK: call <16 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnebf82ph256(
  return _mm256_cvtnebf8_ph(__A);
}

__m256h test_mm256_mask_cvtnebf8_ph(__m256h __A, __mmask16 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm256_mask_cvtnebf8_ph(
  // CHECK: call <16 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnebf82ph256(
  return _mm256_mask_cvtnebf8_ph(__A, __B, __C);
}

__m256h test_mm256_maskz_cvtnebf8_ph(__mmask16 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtnebf8_ph(
  // CHECK: call <16 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnebf82ph256(
  return _mm256_maskz_cvtnebf8_ph(__A, __B);
}

__m128h test_mm_cvtnehf8_ph(__m128i __A) {
  // CHECK-LABEL: @test_mm_cvtnehf8_ph(
  // CHECK: call <8 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnehf82ph128(
  return _mm_cvtnehf8_ph(__A);
}

__m128h test_mm_mask_cvtnehf8_ph(__m128h __A, __mmask8 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_cvtnehf8_ph(
  // CHECK: call <8 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnehf82ph128(
  return _mm_mask_cvtnehf8_ph(__A, __B, __C);
}

__m128h test_mm_maskz_cvtnehf8_ph(__mmask8 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtnehf8_ph(
  // CHECK: call <8 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnehf82ph128(
  return _mm_maskz_cvtnehf8_ph(__A, __B);
}

__m256h test_mm256_cvtnehf8_ph(__m128i __A) {
  // CHECK-LABEL: @test_mm256_cvtnehf8_ph(
  // CHECK: call <16 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnehf82ph256(
  return _mm256_cvtnehf8_ph(__A);
}

__m256h test_mm256_mask_cvtnehf8_ph(__m256h __A, __mmask16 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm256_mask_cvtnehf8_ph(
  // CHECK: call <16 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnehf82ph256(
  return _mm256_mask_cvtnehf8_ph(__A, __B, __C);
}

__m256h test_mm256_maskz_cvtnehf8_ph(__mmask16 __A, __m128i __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtnehf8_ph(
  // CHECK: call <16 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnehf82ph256(
  return _mm256_maskz_cvtnehf8_ph(__A, __B);
}

__m128i test_mm_cvtneph_pbf8(__m128h __A) {
  // CHECK-LABEL: @test_mm_cvtneph_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8128(
  return _mm_cvtneph_pbf8(__A);
}

__m128i test_mm_mask_cvtneph_pbf8(__m128i __A, __mmask8 __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_mask_cvtneph_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8128(
  return _mm_mask_cvtneph_pbf8(__A, __B, __C);
}

__m128i test_mm_maskz_cvtneph_pbf8(__mmask8 __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtneph_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8128(
  return _mm_maskz_cvtneph_pbf8(__A, __B);
}

__m128i test_mm256_cvtneph_pbf8(__m256h __A) {
  // CHECK-LABEL: @test_mm256_cvtneph_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8256(
  return _mm256_cvtneph_pbf8(__A);
}

__m128i test_mm256_mask_cvtneph_pbf8(__m128i __A, __mmask16 __B, __m256h __C) {
  // CHECK-LABEL: @test_mm256_mask_cvtneph_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8256(
  return _mm256_mask_cvtneph_pbf8(__A, __B, __C);
}

__m128i test_mm256_maskz_cvtneph_pbf8(__mmask16 __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtneph_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8256(
  return _mm256_maskz_cvtneph_pbf8(__A, __B);
}

__m128i test_mm_cvtneph2bf8s_pbf8(__m128h __A) {
  // CHECK-LABEL: @test_mm_cvtneph2bf8s_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8s128(
  return _mm_cvtneph2bf8s_pbf8(__A);
}

__m128i test_mm_mask_cvtneph2bf8s_pbf8(__m128i __A, __mmask8 __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_mask_cvtneph2bf8s_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8s128(
  return _mm_mask_cvtneph2bf8s_pbf8(__A, __B, __C);
}

__m128i test_mm_maskz_cvtneph2bf8s_pbf8(__mmask8 __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtneph2bf8s_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8s128(
  return _mm_maskz_cvtneph2bf8s_pbf8(__A, __B);
}

__m128i test_mm256_cvtneph2bf8s_pbf8(__m256h __A) {
  // CHECK-LABEL: @test_mm256_cvtneph2bf8s_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8s256(
  return _mm256_cvtneph2bf8s_pbf8(__A);
}

__m128i test_mm256_mask_cvtneph2bf8s_pbf8(__m128i __A, __mmask16 __B, __m256h __C) {
  // CHECK-LABEL: @test_mm256_mask_cvtneph2bf8s_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8s256(
  return _mm256_mask_cvtneph2bf8s_pbf8(__A, __B, __C);
}

__m128i test_mm256_maskz_cvtneph2bf8s_pbf8(__mmask16 __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtneph2bf8s_pbf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8s256(
  return _mm256_maskz_cvtneph2bf8s_pbf8(__A, __B);
}

__m128i test_mm_cvtneph_phf8(__m128h __A) {
  // CHECK-LABEL: @test_mm_cvtneph_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8128(
  return _mm_cvtneph_phf8(__A);
}

__m128i test_mm_mask_cvtneph_phf8(__m128i __A, __mmask8 __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_mask_cvtneph_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8128(
  return _mm_mask_cvtneph_phf8(__A, __B, __C);
}

__m128i test_mm_maskz_cvtneph_phf8(__mmask8 __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtneph_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8128(
  return _mm_maskz_cvtneph_phf8(__A, __B);
}

__m128i test_mm256_cvtneph_phf8(__m256h __A) {
  // CHECK-LABEL: @test_mm256_cvtneph_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8256(
  return _mm256_cvtneph_phf8(__A);
}

__m128i test_mm256_mask_cvtneph_phf8(__m128i __A, __mmask16 __B, __m256h __C) {
  // CHECK-LABEL: @test_mm256_mask_cvtneph_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8256(
  return _mm256_mask_cvtneph_phf8(__A, __B, __C);
}

__m128i test_mm256_maskz_cvtneph_phf8(__mmask16 __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtneph_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8256(
  return _mm256_maskz_cvtneph_phf8(__A, __B);
}

__m128i test_mm_cvtneph2hf8s_phf8(__m128h __A) {
  // CHECK-LABEL: @test_mm_cvtneph2hf8s_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8s128(
  return _mm_cvtneph2hf8s_phf8(__A);
}

__m128i test_mm_mask_cvtneph2hf8s_phf8(__m128i __A, __mmask8 __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_mask_cvtneph2hf8s_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8s128(
  return _mm_mask_cvtneph2hf8s_phf8(__A, __B, __C);
}

__m128i test_mm_maskz_cvtneph2hf8s_phf8(__mmask8 __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtneph2hf8s_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8s128(
  return _mm_maskz_cvtneph2hf8s_phf8(__A, __B);
}

__m128i test_mm256_cvtneph2hf8s_phf8(__m256h __A) {
  // CHECK-LABEL: @test_mm256_cvtneph2hf8s_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8s256(
  return _mm256_cvtneph2hf8s_phf8(__A);
}

__m128i test_mm256_mask_cvtneph2hf8s_phf8(__m128i __A, __mmask16 __B, __m256h __C) {
  // CHECK-LABEL: @test_mm256_mask_cvtneph2hf8s_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8s256(
  return _mm256_mask_cvtneph2hf8s_phf8(__A, __B, __C);
}

__m128i test_mm256_maskz_cvtneph2hf8s_phf8(__mmask16 __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtneph2hf8s_phf8(
  // CHECK: call <16 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8s256(
  return _mm256_maskz_cvtneph2hf8s_phf8(__A, __B);
}
