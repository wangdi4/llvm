// REQUIRES: intel_feature_isa_avx512_sat_cvt
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +avx512satcvt \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m512i test_mm512_cvtnebf162ibs_epi8(__m512bh __A) {
  // CHECK-LABEL: @test_mm512_cvtnebf162ibs_epi8(
  // CHECK: @llvm.x86.vcvtnebf162ibs512(
  return _mm512_cvtnebf162ibs_epi8(__A);
}

__m512i test_mm512_mask_cvtnebf162ibs_epi8(__m512i __S, __mmask32 __A, __m512bh __B) {
  // CHECK-LABEL: @test_mm512_mask_cvtnebf162ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvtnebf162ibs512(
  return _mm512_mask_cvtnebf162ibs_epi8(__S, __A, __B);
}

__m512i test_mm512_maskz_cvtnebf162ibs_epi8(__mmask32 __A, __m512bh __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtnebf162ibs_epi8
  // CHECK: @llvm.x86.maskz.vcvtnebf162ibs512(
  return _mm512_maskz_cvtnebf162ibs_epi8(__A, __B);
}

__m512i test_mm512_cvtnebf162iubs_epi8(__m512bh __A) {
  // CHECK-LABEL: @test_mm512_cvtnebf162iubs_epi8(
  // CHECK: @llvm.x86.vcvtnebf162iubs512(
  return _mm512_cvtnebf162iubs_epi8(__A);
}

__m512i test_mm512_mask_cvtnebf162iubs_epi8(__m512i __S, __mmask32 __A, __m512bh __B) {
  // CHECK-LABEL: @test_mm512_mask_cvtnebf162iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvtnebf162iubs512(
  return _mm512_mask_cvtnebf162iubs_epi8(__S, __A, __B);
}

__m512i test_mm512_maskz_cvtnebf162iubs_epi8(__mmask32 __A, __m512bh __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtnebf162iubs_epi8
  // CHECK: @llvm.x86.maskz.vcvtnebf162iubs512(
  return _mm512_maskz_cvtnebf162iubs_epi8(__A, __B);
}

__m512i test_mm512_cvtph2ibs_epi8(__m512h __A) {
  // CHECK-LABEL: @test_mm512_cvtph2ibs_epi8(
  // CHECK: @llvm.x86.vcvtph2ibs512
  return _mm512_cvtph2ibs_epi8(__A);
}

__m512i test_mm512_mask_cvtph2ibs_epi8(__m512i __S, __mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_mask_cvtph2ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvtph2ibs512
  return _mm512_mask_cvtph2ibs_epi8(__S, __A, __B);
}

__m512i test_mm512_maskz_cvtph2ibs_epi8(__mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtph2ibs_epi8(
  // CHECK: @llvm.x86.maskz.vcvtph2ibs512
  return _mm512_maskz_cvtph2ibs_epi8(__A, __B);
}

__m512i test_mm512_cvtph2ibs_round_epi8(__m512h __A) {
  // CHECK-LABEL: @test_mm512_cvtph2ibs_round_epi8(
  // CHECK: @llvm.x86.vcvtph2ibs.round.512
  return _mm512_cvtph2ibs_round_epi8(__A, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m512i test_mm512_mask_cvtph2ibs_round_epi8(__m512i __S, __mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_mask_cvtph2ibs_round_epi8
  // CHECK: @llvm.x86.mask.vcvtph2ibs.round.512
  return _mm512_mask_cvtph2ibs_round_epi8(__S, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m512i test_mm512_maskz_cvtph2ibs_round_epi8(__mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtph2ibs_round_epi8
  // CHECK: @llvm.x86.maskz.vcvtph2ibs.round.512
  return _mm512_maskz_cvtph2ibs_round_epi8(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m512i test_mm512_cvtph2iubs_epi8(__m512h __A) {
  // CHECK-LABEL: @test_mm512_cvtph2iubs_epi8(
  // CHECK: @llvm.x86.vcvtph2iubs512
  return _mm512_cvtph2iubs_epi8(__A);
}

__m512i test_mm512_mask_cvtph2iubs_epi8(__m512i __S, __mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_mask_cvtph2iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvtph2iubs512
  return _mm512_mask_cvtph2iubs_epi8(__S, __A, __B);
}

__m512i test_mm512_maskz_cvtph2iubs_epi8(__mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtph2iubs_epi8(
  // CHECK: @llvm.x86.maskz.vcvtph2iubs512
  return _mm512_maskz_cvtph2iubs_epi8(__A, __B);
}

__m512i test_mm512_cvtph2iubs_round_epi8(__m512h __A) {
  // CHECK-LABEL: @test_mm512_cvtph2iubs_round_epi8(
  // CHECK: @llvm.x86.vcvtph2iubs.round.512
  return _mm512_cvtph2iubs_round_epi8(__A, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m512i test_mm512_mask_cvtph2iubs_round_epi8(__m512i __S, __mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_mask_cvtph2iubs_round_epi8
  // CHECK: @llvm.x86.mask.vcvtph2iubs.round.512
  return _mm512_mask_cvtph2iubs_round_epi8(__S, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m512i test_mm512_maskz_cvtph2iubs_round_epi8(__mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtph2iubs_round_epi8
  // CHECK: @llvm.x86.maskz.vcvtph2iubs.round.512
  return _mm512_maskz_cvtph2iubs_round_epi8(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m512i test_mm512_cvtps2ibs_epi8(__m512 __A) {
  // CHECK-LABEL: @test_mm512_cvtps2ibs_epi8(
  // CHECK: @llvm.x86.vcvtps2ibs512
  return _mm512_cvtps2ibs_epi8(__A);
}

__m512i test_mm512_mask_cvtps2ibs_epi8(__m512i __S, __mmask16 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_mask_cvtps2ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvtps2ibs512
  return _mm512_mask_cvtps2ibs_epi8(__S, __A, __B);
}

__m512i test_mm512_maskz_cvtps2ibs_epi8(__mmask16 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtps2ibs_epi8(
  // CHECK: @llvm.x86.maskz.vcvtps2ibs512
  return _mm512_maskz_cvtps2ibs_epi8(__A, __B);
}

__m512i test_mm512_cvtps2ibs_round_epi8(__m512 __A) {
  // CHECK-LABEL: @test_mm512_cvtps2ibs_round_epi8(
  // CHECK: @llvm.x86.vcvtps2ibs.round.512
  return _mm512_cvtps2ibs_round_epi8(__A, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m512i test_mm512_mask_cvtps2ibs_round_epi8(__m512i __S, __mmask16 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_mask_cvtps2ibs_round_epi8
  // CHECK: @llvm.x86.mask.vcvtps2ibs.round.512
  return _mm512_mask_cvtps2ibs_round_epi8(__S, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m512i test_mm512_maskz_cvtps2ibs_round_epi8(__mmask16 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtps2ibs_round_epi8
  // CHECK: @llvm.x86.maskz.vcvtps2ibs.round.512
  return _mm512_maskz_cvtps2ibs_round_epi8(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m512i test_mm512_cvtps2iubs_epi8(__m512 __A) {
  // CHECK-LABEL: @test_mm512_cvtps2iubs_epi8(
  // CHECK: @llvm.x86.vcvtps2iubs512
  return _mm512_cvtps2iubs_epi8(__A);
}

__m512i test_mm512_mask_cvtps2iubs_epi8(__m512i __S, __mmask16 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_mask_cvtps2iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvtps2iubs512
  return _mm512_mask_cvtps2iubs_epi8(__S, __A, __B);
}

__m512i test_mm512_maskz_cvtps2iubs_epi8(__mmask16 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtps2iubs_epi8(
  // CHECK: @llvm.x86.maskz.vcvtps2iubs512
  return _mm512_maskz_cvtps2iubs_epi8(__A, __B);
}

__m512i test_mm512_cvtps2iubs_round_epi8(__m512 __A) {
  // CHECK-LABEL: @test_mm512_cvtps2iubs_round_epi8(
  // CHECK: @llvm.x86.vcvtps2iubs.round.512
  return _mm512_cvtps2iubs_round_epi8(__A, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m512i test_mm512_mask_cvtps2iubs_round_epi8(__m512i __S, __mmask16 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_mask_cvtps2iubs_round_epi8
  // CHECK: @llvm.x86.mask.vcvtps2iubs.round.512
  return _mm512_mask_cvtps2iubs_round_epi8(__S, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m512i test_mm512_maskz_cvtps2iubs_round_epi8(__mmask16 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtps2iubs_round_epi8
  // CHECK: @llvm.x86.maskz.vcvtps2iubs.round.512
  return _mm512_maskz_cvtps2iubs_round_epi8(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m512i test_mm512_cvttnebf162ibs_epi8(__m512bh __A) {
  // CHECK-LABEL: @test_mm512_cvttnebf162ibs_epi8(
  // CHECK: @llvm.x86.vcvttnebf162ibs512(
  return _mm512_cvttnebf162ibs_epi8(__A);
}

__m512i test_mm512_mask_cvttnebf162ibs_epi8(__m512i __S, __mmask32 __A, __m512bh __B) {
  // CHECK-LABEL: @test_mm512_mask_cvttnebf162ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvttnebf162ibs512(
  return _mm512_mask_cvttnebf162ibs_epi8(__S, __A, __B);
}

__m512i test_mm512_maskz_cvttnebf162ibs_epi8(__mmask32 __A, __m512bh __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvttnebf162ibs_epi8
  // CHECK: @llvm.x86.maskz.vcvttnebf162ibs512(
  return _mm512_maskz_cvttnebf162ibs_epi8(__A, __B);
}

__m512i test_mm512_cvttnebf162iubs_epi8(__m512bh __A) {
  // CHECK-LABEL: @test_mm512_cvttnebf162iubs_epi8(
  // CHECK: @llvm.x86.vcvttnebf162iubs512(
  return _mm512_cvttnebf162iubs_epi8(__A);
}

__m512i test_mm512_mask_cvttnebf162iubs_epi8(__m512i __S, __mmask32 __A, __m512bh __B) {
  // CHECK-LABEL: @test_mm512_mask_cvttnebf162iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvttnebf162iubs512(
  return _mm512_mask_cvttnebf162iubs_epi8(__S, __A, __B);
}

__m512i test_mm512_maskz_cvttnebf162iubs_epi8(__mmask32 __A, __m512bh __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvttnebf162iubs_epi8
  // CHECK: @llvm.x86.maskz.vcvttnebf162iubs512(
  return _mm512_maskz_cvttnebf162iubs_epi8(__A, __B);
}

__m512i test_mm512_cvttph2ibs_epi8(__m512h __A) {
  // CHECK-LABEL: @test_mm512_cvttph2ibs_epi8(
  // CHECK: @llvm.x86.vcvttph2ibs512
  return _mm512_cvttph2ibs_epi8(__A);
}

__m512i test_mm512_mask_cvttph2ibs_epi8(__m512i __S, __mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_mask_cvttph2ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvttph2ibs512
  return _mm512_mask_cvttph2ibs_epi8(__S, __A, __B);
}

__m512i test_mm512_maskz_cvttph2ibs_epi8(__mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvttph2ibs_epi8
  // CHECK: @llvm.x86.maskz.vcvttph2ibs512
  return _mm512_maskz_cvttph2ibs_epi8(__A, __B);
}

__m512i test_mm512_cvttph2ibs_round_epi8(__m512h __A) {
  // CHECK-LABEL: @test_mm512_cvttph2ibs_round_epi8
  // CHECK: @llvm.x86.vcvttph2ibs.round.512
  return _mm512_cvttph2ibs_round_epi8(__A, _MM_FROUND_NO_EXC);
}

__m512i test_mm512_mask_cvttph2ibs_round_epi8(__m512i __S, __mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_mask_cvttph2ibs_round_epi8
  // CHECK: @llvm.x86.mask.vcvttph2ibs.round.512
  return _mm512_mask_cvttph2ibs_round_epi8(__S, __A, __B, _MM_FROUND_NO_EXC);
}

__m512i test_mm512_maskz_cvttph2ibs_round_epi8(__mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvttph2ibs_round_epi8
  // CHECK: @llvm.x86.maskz.vcvttph2ibs.round.512
  return _mm512_maskz_cvttph2ibs_round_epi8(__A, __B, _MM_FROUND_NO_EXC);
}

__m512i test_mm512_cvttph2iubs_epi8(__m512h __A) {
  // CHECK-LABEL: @test_mm512_cvttph2iubs_epi8(
  // CHECK: @llvm.x86.vcvttph2iubs512
  return _mm512_cvttph2iubs_epi8(__A);
}

__m512i test_mm512_mask_cvttph2iubs_epi8(__m512i __S, __mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_mask_cvttph2iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvttph2iubs512
  return _mm512_mask_cvttph2iubs_epi8(__S, __A, __B);
}

__m512i test_mm512_maskz_cvttph2iubs_epi8(__mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvttph2iubs_epi8
  // CHECK: @llvm.x86.maskz.vcvttph2iubs512
  return _mm512_maskz_cvttph2iubs_epi8(__A, __B);
}

__m512i test_mm512_cvttph2iubs_round_epi8(__m512h __A) {
  // CHECK-LABEL: @test_mm512_cvttph2iubs_round_epi8
  // CHECK: @llvm.x86.vcvttph2iubs.round.512
  return _mm512_cvttph2iubs_round_epi8(__A, _MM_FROUND_NO_EXC);
}

__m512i test_mm512_mask_cvttph2iubs_round_epi8(__m512i __S, __mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_mask_cvttph2iubs_round_epi8
  // CHECK: @llvm.x86.mask.vcvttph2iubs.round.512
  return _mm512_mask_cvttph2iubs_round_epi8(__S, __A, __B, _MM_FROUND_NO_EXC);
}

__m512i test_mm512_maskz_cvttph2iubs_round_epi8(__mmask32 __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvttph2iubs_round_epi8
  // CHECK: @llvm.x86.maskz.vcvttph2iubs.round.512
  return _mm512_maskz_cvttph2iubs_round_epi8(__A, __B, _MM_FROUND_NO_EXC);
}

__m512i test_mm512_cvttps2ibs_epi8(__m512 __A) {
  // CHECK-LABEL: @test_mm512_cvttps2ibs_epi8(
  // CHECK: @llvm.x86.vcvttps2ibs512
  return _mm512_cvttps2ibs_epi8(__A);
}

__m512i test_mm512_mask_cvttps2ibs_epi8(__m512i __S, __mmask16 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_mask_cvttps2ibs_epi8(
  // CHECK: @llvm.x86.mask.vcvttps2ibs512
  return _mm512_mask_cvttps2ibs_epi8(__S, __A, __B);
}

__m512i test_mm512_maskz_cvttps2ibs_epi8(__mmask16 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvttps2ibs_epi8
  // CHECK: @llvm.x86.maskz.vcvttps2ibs512
  return _mm512_maskz_cvttps2ibs_epi8(__A, __B);
}

__m512i test_mm512_cvttps2ibs_round_epi8(__m512 __A) {
  // CHECK-LABEL: @test_mm512_cvttps2ibs_round_epi8
  // CHECK: @llvm.x86.vcvttps2ibs.round.512
  return _mm512_cvttps2ibs_round_epi8(__A, _MM_FROUND_NO_EXC);
}

__m512i test_mm512_mask_cvttps2ibs_round_epi8(__m512i __S, __mmask16 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_mask_cvttps2ibs_round_epi8
  // CHECK: @llvm.x86.mask.vcvttps2ibs.round.512
  return _mm512_mask_cvttps2ibs_round_epi8(__S, __A, __B, _MM_FROUND_NO_EXC);
}


__m512i test_mm512_maskz_cvttps2ibs_round_epi8(__mmask16 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvttps2ibs_round_epi8
  // CHECK: @llvm.x86.maskz.vcvttps2ibs.round.512
  return _mm512_maskz_cvttps2ibs_round_epi8(__A, __B, _MM_FROUND_NO_EXC);
}

__m512i test_mm512_cvttps2iubs_epi8(__m512 __A) {
  // CHECK-LABEL: @test_mm512_cvttps2iubs_epi8(
  // CHECK: @llvm.x86.vcvttps2iubs512
  return _mm512_cvttps2iubs_epi8(__A);
}

__m512i test_mm512_mask_cvttps2iubs_epi8(__m512i __S, __mmask16 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_mask_cvttps2iubs_epi8(
  // CHECK: @llvm.x86.mask.vcvttps2iubs512
  return _mm512_mask_cvttps2iubs_epi8(__S, __A, __B);
}

__m512i test_mm512_maskz_cvttps2iubs_epi8(__mmask16 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvttps2iubs_epi8
  // CHECK: @llvm.x86.maskz.vcvttps2iubs512
  return _mm512_maskz_cvttps2iubs_epi8(__A, __B);
}

__m512i test_mm512_cvttps2iubs_round_epi8(__m512 __A) {
  // CHECK-LABEL: @test_mm512_cvttps2iubs_round_epi8
  // CHECK: @llvm.x86.vcvttps2iubs.round.512
  return _mm512_cvttps2iubs_round_epi8(__A, _MM_FROUND_NO_EXC);
}

__m512i test_mm512_mask_cvttps2iubs_round_epi8(__m512i __S, __mmask16 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_mask_cvttps2iubs_round_epi8
  // CHECK: @llvm.x86.mask.vcvttps2iubs.round.512
  return _mm512_mask_cvttps2iubs_round_epi8(__S, __A, __B, _MM_FROUND_NO_EXC);
}

__m512i test_mm512_maskz_cvttps2iubs_round_epi8(__mmask16 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvttps2iubs_round_epi8
  // CHECK: @llvm.x86.maskz.vcvttps2iubs.round.512
  return _mm512_maskz_cvttps2iubs_round_epi8(__A, __B, _MM_FROUND_NO_EXC);
}
