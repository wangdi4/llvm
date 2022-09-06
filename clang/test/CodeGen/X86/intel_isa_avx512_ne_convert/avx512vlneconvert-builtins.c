// REQUIRES: intel_feature_isa_avx512_ne_convert
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512fp16 -target-feature +avx512neconvert -target-feature +avx512vl \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128 test_mm_bcstnebf16_ps(const __bfloat16 * __A) {
  // CHECK-LABEL: @test_mm_bcstnebf16_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.vbcstnebf162ps128(i8* %{{.*}})
  return _mm_bcstnebf16_ps(__A);
}

__m128 test_mm_mask_bcstnebf16_ps(__m128 __W, __mmask8 __A, const __bfloat16 * __B) {
  // CHECK-LABEL: @test_mm_mask_bcstnebf16_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.mask.vbcstnebf162ps128(<4 x float> %{{.*}}, i8 %{{.*}}, i8* %{{.*}})
  return _mm_mask_bcstnebf16_ps(__W, __A, __B);
}

__m128 test_mm_maskz_bcstnebf16_ps(__mmask8 __A, const __bfloat16 * __B) {
  // CHECK-LABEL: @test_mm_maskz_bcstnebf16_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.maskz.vbcstnebf162ps128(i8 %{{.*}}, i8* %{{.*}})
  return _mm_maskz_bcstnebf16_ps(__A, __B);
}

__m256 test_mm256_bcstnebf16_ps(const __bfloat16 * __A) {
  // CHECK-LABEL: @test_mm256_bcstnebf16_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.vbcstnebf162ps256(i8* %{{.*}})
  return _mm256_bcstnebf16_ps(__A);
}

__m256 test_mm256_mask_bcstnebf16_ps(__m256 __W, __mmask8 __A, const __bfloat16 * __B) {
  // CHECK-LABEL: @test_mm256_mask_bcstnebf16_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.mask.vbcstnebf162ps256(<8 x float> %{{.*}}, i8 %{{.*}}, i8* %{{.*}})
  return _mm256_mask_bcstnebf16_ps(__W, __A, __B);
}

__m256 test_mm256_maskz_bcstnebf16_ps(__mmask8 __A, const __bfloat16 * __B) {
  // CHECK-LABEL: @test_mm256_maskz_bcstnebf16_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.maskz.vbcstnebf162ps256(i8 %{{.*}}, i8* %{{.*}})
  return _mm256_maskz_bcstnebf16_ps(__A, __B);
}

__m128 test_mm_bcstnesh_ps(const _Float16 * __A) {
  // CHECK-LABEL: @test_mm_bcstnesh_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.vbcstnesh2ps128(i8* %{{.*}})
  return _mm_bcstnesh_ps(__A);
}

__m128 test_mm_mask_bcstnesh_ps(__m128 __W, __mmask8 __A, const _Float16 * __B) {
  // CHECK-LABEL: @test_mm_mask_bcstnesh_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.mask.vbcstnesh2ps128(<4 x float> %{{.*}}, i8 %{{.*}}, i8* %{{.*}})
  return _mm_mask_bcstnesh_ps(__W, __A, __B);
}

__m128 test_mm_maskz_bcstnesh_ps(__mmask8 __A, const _Float16 * __B) {
  // CHECK-LABEL: @test_mm_maskz_bcstnesh_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.maskz.vbcstnesh2ps128(i8 %{{.*}}, i8* %{{.*}})
  return _mm_maskz_bcstnesh_ps(__A, __B);
}

__m256 test_mm256_bcstnesh_ps(const _Float16 * __A) {
  // CHECK-LABEL: @test_mm256_bcstnesh_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.vbcstnesh2ps256(i8* %{{.*}})
  return _mm256_bcstnesh_ps(__A);
}

__m256 test_mm256_mask_bcstnesh_ps(__m256 __W, __mmask8 __A, const _Float16 * __B) {
  // CHECK-LABEL: @test_mm256_mask_bcstnesh_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.mask.vbcstnesh2ps256(<8 x float> %{{.*}}, i8 %{{.*}}, i8* %{{.*}})
  return _mm256_mask_bcstnesh_ps(__W, __A, __B);
}

__m256 test_mm256_maskz_bcstnesh_ps(__mmask8 __A, const _Float16 * __B) {
  // CHECK-LABEL: @test_mm256_maskz_bcstnesh_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.maskz.vbcstnesh2ps256(i8 %{{.*}}, i8* %{{.*}})
  return _mm256_maskz_bcstnesh_ps(__A, __B);
}

__m128h test_mm_cvtne2ps_ph(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_cvtne2ps_ph(
  // CHECK: call <8 x half> @llvm.x86.avx512.vcvtne2ps2ph128(
  return _mm_cvtne2ps_ph(__A, __B);
}

__m256h test_mm256_cvtne2ps_ph(__m256 __A, __m256 __B) {
  // CHECK-LABEL: @test_mm256_cvtne2ps_ph(
  // CHECK: call <16 x half> @llvm.x86.avx512.vcvtne2ps2ph256(
  return _mm256_cvtne2ps_ph(__A, __B);
}

__m128 test_mm_cvtneebf16_ps(const __m128bh * __A) {
  // CHECK-LABEL: @test_mm_cvtneebf16_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.vcvtneebf162ps128(i8* %{{.*}})
  return _mm_cvtneebf16_ps(__A);
}

__m128 test_mm_mask_cvtneebf16_ps(__m128 __W, __mmask8 __A, const __m128bh * __B) {
  // CHECK-LABEL: @test_mm_mask_cvtneebf16_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.mask.vcvtneebf162ps128(<4 x float> %{{.*}}, i8 %{{.*}}, i8* %{{.*}})
  return _mm_mask_cvtneebf16_ps(__W, __A, __B);
}

__m128 test_mm_maskz_cvtneebf16_ps(__mmask8 __A, const __m128bh * __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtneebf16_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.maskz.vcvtneebf162ps128(i8 %{{.*}}, i8* %{{.*}})
  return _mm_maskz_cvtneebf16_ps(__A, __B);
}

__m256 test_mm256_cvtneebf16_ps(const __m256bh * __A) {
  // CHECK-LABEL: @test_mm256_cvtneebf16_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.vcvtneebf162ps256(i8* %{{.*}})
  return _mm256_cvtneebf16_ps(__A);
}

__m256 test_mm256_mask_cvtneebf16_ps(__m256 __W, __mmask8 __A, const __m256bh * __B) {
  // CHECK-LABEL: @test_mm256_mask_cvtneebf16_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.mask.vcvtneebf162ps256(<8 x float> %{{.*}}, i8 %{{.*}}, i8* %{{.*}})
  return _mm256_mask_cvtneebf16_ps(__W, __A, __B);
}

__m256 test_mm256_maskz_cvtneebf16_ps(__mmask8 __A, const __m256bh * __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtneebf16_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.maskz.vcvtneebf162ps256(i8 %{{.*}}, i8* %{{.*}})
  return _mm256_maskz_cvtneebf16_ps(__A, __B);
}

__m128 test_mm_cvtneeph_ps(const __m128h * __A) {
  // CHECK-LABEL: @test_mm_cvtneeph_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.vcvtneeph2ps128(i8* %{{.*}})
  return _mm_cvtneeph_ps(__A);
}

__m128 test_mm_mask_cvtneeph_ps(__m128 __W, __mmask8 __A, const __m128h * __B) {
  // CHECK-LABEL: @test_mm_mask_cvtneeph_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.mask.vcvtneeph2ps128(<4 x float> %{{.*}}, i8 %{{.*}}, i8* %{{.*}})
  return _mm_mask_cvtneeph_ps(__W, __A, __B);
}

__m128 test_mm_maskz_cvtneeph_ps(__mmask8 __A, const __m128h * __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtneeph_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.maskz.vcvtneeph2ps128(i8 %{{.*}}, i8* %{{.*}})
  return _mm_maskz_cvtneeph_ps(__A, __B);
}

__m256 test_mm256_cvtneeph_ps(const __m256h * __A) {
  // CHECK-LABEL: @test_mm256_cvtneeph_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.vcvtneeph2ps256(i8* %{{.*}})
  return _mm256_cvtneeph_ps(__A);
}

__m256 test_mm256_mask_cvtneeph_ps(__m256 __W, __mmask8 __A, const __m256h * __B) {
  // CHECK-LABEL: @test_mm256_mask_cvtneeph_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.mask.vcvtneeph2ps256(<8 x float> %{{.*}}, i8 %{{.*}}, i8* %{{.*}})
  return _mm256_mask_cvtneeph_ps(__W, __A, __B);
}

__m256 test_mm256_maskz_cvtneeph_ps(__mmask8 __A, const __m256h * __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtneeph_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.maskz.vcvtneeph2ps256(i8 %{{.*}}, i8* %{{.*}})
  return _mm256_maskz_cvtneeph_ps(__A, __B);
}

__m128 test_mm_cvtneobf16_ps(const __m128bh * __A) {
  // CHECK-LABEL: @test_mm_cvtneobf16_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.vcvtneobf162ps128(i8* %{{.*}})
  return _mm_cvtneobf16_ps(__A);
}

__m128 test_mm_mask_cvtneobf16_ps(__m128 __W, __mmask8 __A, const __m128bh * __B) {
  // CHECK-LABEL: @test_mm_mask_cvtneobf16_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.mask.vcvtneobf162ps128(<4 x float> %{{.*}}, i8 %{{.*}}, i8* %{{.*}})
  return _mm_mask_cvtneobf16_ps(__W, __A, __B);
}

__m128 test_mm_maskz_cvtneobf16_ps(__mmask8 __A, const __m128bh * __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtneobf16_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.maskz.vcvtneobf162ps128(i8 %{{.*}}, i8* %{{.*}})
  return _mm_maskz_cvtneobf16_ps(__A, __B);
}

__m256 test_mm256_cvtneobf16_ps(const __m256bh * __A) {
  // CHECK-LABEL: @test_mm256_cvtneobf16_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.vcvtneobf162ps256(i8* %{{.*}})
  return _mm256_cvtneobf16_ps(__A);
}

__m256 test_mm256_mask_cvtneobf16_ps(__m256 __W, __mmask8 __A, const __m256bh * __B) {
  // CHECK-LABEL: @test_mm256_mask_cvtneobf16_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.mask.vcvtneobf162ps256(<8 x float> %{{.*}}, i8 %{{.*}}, i8* %{{.*}})
  return _mm256_mask_cvtneobf16_ps(__W, __A, __B);
}

__m256 test_mm256_maskz_cvtneobf16_ps(__mmask8 __A, const __m256bh * __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtneobf16_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.maskz.vcvtneobf162ps256(i8 %{{.*}}, i8* %{{.*}})
  return _mm256_maskz_cvtneobf16_ps(__A, __B);
}

__m128 test_mm_cvtneoph_ps(const __m128h * __A) {
  // CHECK-LABEL: @test_mm_cvtneoph_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.vcvtneoph2ps128(i8* %{{.*}})
  return _mm_cvtneoph_ps(__A);
}

__m128 test_mm_mask_cvtneoph_ps(__m128 __W, __mmask8 __A, const __m128h * __B) {
  // CHECK-LABEL: @test_mm_mask_cvtneoph_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.mask.vcvtneoph2ps128(<4 x float> %{{.*}}, i8 %{{.*}}, i8* %{{.*}})
  return _mm_mask_cvtneoph_ps(__W, __A, __B);
}

__m128 test_mm_maskz_cvtneoph_ps(__mmask8 __A, const __m128h * __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtneoph_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512.maskz.vcvtneoph2ps128(i8 %{{.*}}, i8* %{{.*}})
  return _mm_maskz_cvtneoph_ps(__A, __B);
}

__m256 test_mm256_cvtneoph_ps(const __m256h * __A) {
  // CHECK-LABEL: @test_mm256_cvtneoph_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.vcvtneoph2ps256(i8* %{{.*}})
  return _mm256_cvtneoph_ps(__A);
}

__m256 test_mm256_mask_cvtneoph_ps(__m256 __W, __mmask8 __A, const __m256h * __B) {
  // CHECK-LABEL: @test_mm256_mask_cvtneoph_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.mask.vcvtneoph2ps256(<8 x float> %{{.*}}, i8 %{{.*}}, i8* %{{.*}})
  return _mm256_mask_cvtneoph_ps(__W, __A, __B);
}

__m256 test_mm256_maskz_cvtneoph_ps(__mmask8 __A, const __m256h * __B) {
  // CHECK-LABEL: @test_mm256_maskz_cvtneoph_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512.maskz.vcvtneoph2ps256(i8 %{{.*}}, i8* %{{.*}})
  return _mm256_maskz_cvtneoph_ps(__A, __B);
}
