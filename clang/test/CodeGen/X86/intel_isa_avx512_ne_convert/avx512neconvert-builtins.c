// REQUIRES: intel_feature_isa_avx512_ne_convert
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512fp16 -target-feature +avx512neconvert \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m512 test_mm512_bcstnebf16_ps(const __bfloat16 * __A) {
  // CHECK-LABEL: @test_mm512_bcstnebf16_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.vbcstnebf162ps512(i8* %{{.*}})
  return _mm512_bcstnebf16_ps(__A);
}

__m512 test_mm512_mask_bcstnebf16_ps(__m512 __W, __mmask16 __A, const __bfloat16 * __B) {
  // CHECK-LABEL: @test_mm512_mask_bcstnebf16_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.mask.vbcstnebf162ps512(<16 x float> %{{.*}}, i16 %{{.*}}, i8* %{{.*}})
  return _mm512_mask_bcstnebf16_ps(__W, __A, __B);
}

__m512 test_mm512_maskz_bcstnebf16_ps(__mmask16 __A, const __bfloat16 * __B) {
  // CHECK-LABEL: @test_mm512_maskz_bcstnebf16_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.maskz.vbcstnebf162ps512(
  return _mm512_maskz_bcstnebf16_ps(__A, __B);
}

__m512 test_mm512_bcstnesh_ps(const _Float16 * __A) {
  // CHECK-LABEL: @test_mm512_bcstnesh_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.vbcstnesh2ps512(i8* %{{.*}})
  return _mm512_bcstnesh_ps(__A);
}

__m512 test_mm512_mask_bcstnesh_ps(__m512 __W, __mmask16 __A, const _Float16 * __B) {
  // CHECK-LABEL: @test_mm512_mask_bcstnesh_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.mask.vbcstnesh2ps512(<16 x float> %{{.*}}, i16 %{{.*}}, i8* %{{.*}})
  return _mm512_mask_bcstnesh_ps(__W, __A, __B);
}

__m512 test_mm512_maskz_bcstnesh_ps(__mmask16 __A, const _Float16 * __B) {
  // CHECK-LABEL: @test_mm512_maskz_bcstnesh_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.maskz.vbcstnesh2ps512(i16 %{{.*}}, i8* %{{.*}})
  return _mm512_maskz_bcstnesh_ps(__A, __B);
}

__m512h test_mm512_cvtne2ps_ph(__m512 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_cvtne2ps_ph(
  // CHECK: call <32 x half> @llvm.x86.avx512.vcvtne2ps2ph512(<16 x float> %{{.*}}, <16 x float> %{{.*}}, i32 4)
  return _mm512_cvtne2ps_ph(__A, __B);
}

__m512h test_mm512_cvtne_round2ps_ph(__m512 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_cvtne_round2ps_ph(
  // CHECK: call <32 x half> @llvm.x86.avx512.vcvtne2ps2ph512(<16 x float> %{{.*}}, <16 x float> %{{.*}}, i32 11)
  return _mm512_cvtne_round2ps_ph(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m512 test_mm512_cvtneebf16_ps(const __m512bh * __A) {
  // CHECK-LABEL: @test_mm512_cvtneebf16_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.vcvtneebf162ps512(i8* %{{.*}})
  return _mm512_cvtneebf16_ps(__A);
}

__m512 test_mm512_mask_cvtneebf16_ps(__m512 __W, __mmask16 __A, const __m512bh * __B) {
  // CHECK-LABEL: @test_mm512_mask_cvtneebf16_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.mask.vcvtneebf162ps512(<16 x float> %{{.*}}, i16 %{{.*}}, i8* %{{.*}})
  return _mm512_mask_cvtneebf16_ps(__W, __A, __B);
}

__m512 test_mm512_maskz_cvtneebf16_ps(__mmask16 __A, const __m512bh * __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtneebf16_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.maskz.vcvtneebf162ps512(i16 %{{.*}}, i8* %{{.*}})
  return _mm512_maskz_cvtneebf16_ps(__A, __B);
}

__m512 test_mm512_cvtneeph_ps(const __m512h * __A) {
  // CHECK-LABEL: @test_mm512_cvtneeph_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.vcvtneeph2ps512(i8* %{{.*}})
  return _mm512_cvtneeph_ps(__A);
}

__m512 test_mm512_mask_cvtneeph_ps(__m512 __W, __mmask16 __A, const __m512h * __B) {
  // CHECK-LABEL: @test_mm512_mask_cvtneeph_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.mask.vcvtneeph2ps512(<16 x float> %{{.*}}, i16 %{{.*}}, i8* %{{.*}})
  return _mm512_mask_cvtneeph_ps(__W, __A, __B);
}

__m512 test_mm512_maskz_cvtneeph_ps(__mmask16 __A, const __m512h * __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtneeph_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.maskz.vcvtneeph2ps512(i16 %{{.*}}, i8* %{{.*}})
  return _mm512_maskz_cvtneeph_ps(__A, __B);
}

__m512 test_mm512_cvtneobf16_ps(const __m512bh * __A) {
  // CHECK-LABEL: @test_mm512_cvtneobf16_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.vcvtneobf162ps512(i8* %{{.*}})
  return _mm512_cvtneobf16_ps(__A);
}

__m512 test_mm512_mask_cvtneobf16_ps(__m512 __W, __mmask16 __A, const __m512bh * __B) {
  // CHECK-LABEL: @test_mm512_mask_cvtneobf16_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.mask.vcvtneobf162ps512(<16 x float> %{{.*}}, i16 %{{.*}}, i8* %{{.*}})
  return _mm512_mask_cvtneobf16_ps(__W, __A, __B);
}

__m512 test_mm512_maskz_cvtneobf16_ps(__mmask16 __A, const __m512bh * __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtneobf16_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.maskz.vcvtneobf162ps512(i16 %{{.*}}, i8* %{{.*}})
  return _mm512_maskz_cvtneobf16_ps(__A, __B);
}

__m512 test_mm512_cvtneoph_ps(const __m512h * __A) {
  // CHECK-LABEL: @test_mm512_cvtneoph_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.vcvtneoph2ps512(i8* %{{.*}})
  return _mm512_cvtneoph_ps(__A);
}

__m512 test_mm512_mask_cvtneoph_ps(__m512 __W, __mmask16 __A, const __m512h * __B) {
  // CHECK-LABEL: @test_mm512_mask_cvtneoph_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.mask.vcvtneoph2ps512(<16 x float> %{{.*}}, i16 %{{.*}}, i8* %{{.*}})
  return _mm512_mask_cvtneoph_ps(__W, __A, __B);
}

__m512 test_mm512_maskz_cvtneoph_ps(__mmask16 __A, const __m512h * __B) {
  // CHECK-LABEL: @test_mm512_maskz_cvtneoph_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.maskz.vcvtneoph2ps512(i16 %{{.*}}, i8* %{{.*}})
  return _mm512_maskz_cvtneoph_ps(__A, __B);
}
