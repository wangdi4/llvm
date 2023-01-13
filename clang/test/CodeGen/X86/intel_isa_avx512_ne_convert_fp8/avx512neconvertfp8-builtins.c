// REQUIRES: intel_feature_isa_avx512_ne_convert_fp8
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512neconvertfp8 -target-feature +avx512bw\
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m512i test_mm512_cvtbias2ph_pbf8(__m512i __A, __m512h __B, __m512h __C) {
  // CHECK-LABEL: @test_mm512_cvtbias2ph_pbf8(
  // CHECK: call <64 x i8> @llvm.x86.avx512neconvertfp8.vcvtbias2ph2bf8512(
  return _mm512_cvtbias2ph_pbf8(__A, __B, __C);
}

__m512i test_mm512_cvtbias2ph2bf8s_pbf8(__m512i __A, __m512h __B, __m512h __C) {
  // CHECK-LABEL: @test_mm512_cvtbias2ph2bf8s_pbf8(
  // CHECK: call <64 x i8> @llvm.x86.avx512neconvertfp8.vcvtbias2ph2bf8s512(
  return _mm512_cvtbias2ph2bf8s_pbf8(__A, __B, __C);
}

__m512i test_mm512_cvtbias2ph_phf8(__m512i __A, __m512h __B, __m512h __C) {
  // CHECK-LABEL: @test_mm512_cvtbias2ph_phf8(
  // CHECK: call <64 x i8> @llvm.x86.avx512neconvertfp8.vcvtbias2ph2hf8512(
  return _mm512_cvtbias2ph_phf8(__A, __B, __C);
}

__m512i test_mm512_cvtbias2ph2hf8s_phf8(__m512i __A, __m512h __B, __m512h __C) {
  // CHECK-LABEL: @test_mm512_cvtbias2ph2hf8s_phf8(
  // CHECK: call <64 x i8> @llvm.x86.avx512neconvertfp8.vcvtbias2ph2hf8s512(
  return _mm512_cvtbias2ph2hf8s_phf8(__A, __B, __C);
}

__m256i test_mm512_cvtbiasph_pbf8(__m512h __A) {
  // CHECK-LABEL: @test_mm512_cvtbiasph_pbf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8512(
  return _mm512_cvtbiasph_pbf8(__A);
}

__m256i test_mm512_mask_cvtbiasph_pbf8(__m256i __A, __mmask32 __B, __m512h __C) {
  // CHECK-LABEL: @test_mm512_mask_cvtbiasph_pbf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8512(
  return _mm512_mask_cvtbiasph_pbf8(__A, __B, __C);
}

__m256i test_mm512_maskz_cvtbiasph_pbf8(__mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtbiasph_pbf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8512(
  return _mm512_maskz_cvtbiasph_pbf8(__A, __B);
}

__m256i test_mm512_cvtbiasph2bf8s_pbf8(__m512h __A) {
  // CHECK-LABEL: @test_mm512_cvtbiasph2bf8s_pbf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8s512(
  return _mm512_cvtbiasph2bf8s_pbf8(__A);
}

__m256i test_mm512_mask_cvtbiasph2bf8s_pbf8(__m256i __A, __mmask32 __B, __m512h __C) {
  // CHECK-LABEL: @test_mm512_mask_cvtbiasph2bf8s_pbf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8s512(
  return _mm512_mask_cvtbiasph2bf8s_pbf8(__A, __B, __C);
}

__m256i test_mm512_maskz_cvtbiasph2bf8s_pbf8(__mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtbiasph2bf8s_pbf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2bf8s512(
  return _mm512_maskz_cvtbiasph2bf8s_pbf8(__A, __B);
}

__m256i test_mm512_cvtbiasph_phf8(__m512h __A) {
  // CHECK-LABEL: @test_mm512_cvtbiasph_phf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8512(
  return _mm512_cvtbiasph_phf8(__A);
}

__m256i test_mm512_mask_cvtbiasph_phf8(__m256i __A, __mmask32 __B, __m512h __C) {
  // CHECK-LABEL: @test_mm512_mask_cvtbiasph_phf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8512(
  return _mm512_mask_cvtbiasph_phf8(__A, __B, __C);
}

__m256i test_mm512_maskz_cvtbiasph_phf8(__mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtbiasph_phf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8512(
  return _mm512_maskz_cvtbiasph_phf8(__A, __B);
}

__m256i test_mm512_cvtbiasph2hf8s_phf8(__m512h __A) {
  // CHECK-LABEL: @test_mm512_cvtbiasph2hf8s_phf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8s512(
  return _mm512_cvtbiasph2hf8s_phf8(__A);
}

__m256i test_mm512_mask_cvtbiasph2hf8s_phf8(__m256i __A, __mmask32 __B, __m512h __C) {
  // CHECK-LABEL: @test_mm512_mask_cvtbiasph2hf8s_phf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8s512(
  return _mm512_mask_cvtbiasph2hf8s_phf8(__A, __B, __C);
}

__m256i test_mm512_maskz_cvtbiasph2hf8s_phf8(__mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtbiasph2hf8s_phf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtbiasph2hf8s512(
  return _mm512_maskz_cvtbiasph2hf8s_phf8(__A, __B);
}

__m512i test_mm512_cvtne2ph_pbf8(__m512h __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_cvtne2ph_pbf8(
  // CHECK: call <64 x i8> @llvm.x86.avx512neconvertfp8.vcvtne2ph2bf8512(
  return _mm512_cvtne2ph_pbf8(__A, __B);
}

__m512i test_mm512_cvtne2ph2bf8s_pbf8(__m512h __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_cvtne2ph2bf8s_pbf8(
  // CHECK: call <64 x i8> @llvm.x86.avx512neconvertfp8.vcvtne2ph2bf8s512(
  return _mm512_cvtne2ph2bf8s_pbf8(__A, __B);
}

__m512i test_mm512_cvtne2ph_phf8(__m512h __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_cvtne2ph_phf8(
  // CHECK: call <64 x i8> @llvm.x86.avx512neconvertfp8.vcvtne2ph2hf8512(
  return _mm512_cvtne2ph_phf8(__A, __B);
}

__m512i test_mm512_cvtne2ph2hf8s_phf8(__m512h __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_cvtne2ph2hf8s_phf8(
  // CHECK: call <64 x i8> @llvm.x86.avx512neconvertfp8.vcvtne2ph2hf8s512(
  return _mm512_cvtne2ph2hf8s_phf8(__A, __B);
}

__m512h test_mm512_cvtnebf8_ph(__m256i __A) {
  // CHECK-LABEL: @test_mm512_cvtnebf8_ph(
  // CHECK: call <32 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnebf82ph512(
  return _mm512_cvtnebf8_ph(__A);
}

__m512h test_mm512_mask_cvtnebf8_ph(__m512h __A, __mmask32 __B, __m256i __C) {
  // CHECK-LABEL: @test_mm512_mask_cvtnebf8_ph(
  // CHECK: call <32 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnebf82ph512(
  return _mm512_mask_cvtnebf8_ph(__A, __B, __C);
}

__m512h test_mm512_maskz_cvtnebf8_ph(__mmask32 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtnebf8_ph(
  // CHECK: call <32 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnebf82ph512(
  return _mm512_maskz_cvtnebf8_ph(__A, __B);
}

__m512h test_mm512_cvtnehf8_ph(__m256i __A) {
  // CHECK-LABEL: @test_mm512_cvtnehf8_ph(
  // CHECK: call <32 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnehf82ph512(
  return _mm512_cvtnehf8_ph(__A);
}

__m512h test_mm512_mask_cvtnehf8_ph(__m512h __A, __mmask32 __B, __m256i __C) {
  // CHECK-LABEL: @test_mm512_mask_cvtnehf8_ph(
  // CHECK: call <32 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnehf82ph512(
  return _mm512_mask_cvtnehf8_ph(__A, __B, __C);
}

__m512h test_mm512_maskz_cvtnehf8_ph(__mmask32 __A, __m256i __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtnehf8_ph(
  // CHECK: call <32 x half> @llvm.x86.avx512neconvertfp8.mask.vcvtnehf82ph512(
  return _mm512_maskz_cvtnehf8_ph(__A, __B);
}

__m256i test_mm512_cvtneph_pbf8(__m512h __A) {
  // CHECK-LABEL: @test_mm512_cvtneph_pbf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8512(
  return _mm512_cvtneph_pbf8(__A);
}

__m256i test_mm512_mask_cvtneph_pbf8(__m256i __A, __mmask32 __B, __m512h __C) {
  // CHECK-LABEL: @test_mm512_mask_cvtneph_pbf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8512(
  return _mm512_mask_cvtneph_pbf8(__A, __B, __C);
}

__m256i test_mm512_maskz_cvtneph_pbf8(__mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtneph_pbf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8512(
  return _mm512_maskz_cvtneph_pbf8(__A, __B);
}

__m256i test_mm512_cvtneph2bf8s_pbf8(__m512h __A) {
  // CHECK-LABEL: @test_mm512_cvtneph2bf8s_pbf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8s512(
  return _mm512_cvtneph2bf8s_pbf8(__A);
}

__m256i test_mm512_mask_cvtneph2bf8s_pbf8(__m256i __A, __mmask32 __B, __m512h __C) {
  // CHECK-LABEL: @test_mm512_mask_cvtneph2bf8s_pbf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8s512(
  return _mm512_mask_cvtneph2bf8s_pbf8(__A, __B, __C);
}

__m256i test_mm512_maskz_cvtneph2bf8s_pbf8(__mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtneph2bf8s_pbf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2bf8s512(
  return _mm512_maskz_cvtneph2bf8s_pbf8(__A, __B);
}

__m256i test_mm512_cvtneph_phf8(__m512h __A) {
  // CHECK-LABEL: @test_mm512_cvtneph_phf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8512(
  return _mm512_cvtneph_phf8(__A);
}

__m256i test_mm512_mask_cvtneph_phf8(__m256i __A, __mmask32 __B, __m512h __C) {
  // CHECK-LABEL: @test_mm512_mask_cvtneph_phf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8512(
  return _mm512_mask_cvtneph_phf8(__A, __B, __C);
}

__m256i test_mm512_maskz_cvtneph_phf8(__mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtneph_phf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8512(
  return _mm512_maskz_cvtneph_phf8(__A, __B);
}

__m256i test_mm512_cvtneph2hf8s_phf8(__m512h __A) {
  // CHECK-LABEL: @test_mm512_cvtneph2hf8s_phf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8s512(
  return _mm512_cvtneph2hf8s_phf8(__A);
}

__m256i test_mm512_mask_cvtneph2hf8s_phf8(__m256i __A, __mmask32 __B, __m512h __C) {
  // CHECK-LABEL: @test_mm512_mask_cvtneph2hf8s_phf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8s512(
  return _mm512_mask_cvtneph2hf8s_phf8(__A, __B, __C);
}

__m256i test_mm512_maskz_cvtneph2hf8s_phf8(__mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtneph2hf8s_phf8(
  // CHECK: call <32 x i8> @llvm.x86.avx512neconvertfp8.mask.vcvtneph2hf8s512(
  return _mm512_maskz_cvtneph2hf8s_phf8(__A, __B);
}
