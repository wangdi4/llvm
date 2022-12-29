// REQUIRES: intel_feature_isa_avx512_minmax
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +avx512minmax -target-feature +avx512vl \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128bh test_mm_minmaxne_pbf16(__m128bh __A, __m128bh __B) {
  // CHECK-LABEL: @test_mm_minmaxne_pbf16(
  // CHECK: call <8 x i16> @llvm.x86.avx512minmax.vminmaxnepbf16128(
  return _mm_minmaxne_pbf16(__A, __B, 127);
}

__m128bh test_mm_mask_minmaxne_pbf16(__m128bh __A, __mmask8 __B, __m128bh __C, __m128bh __D) {
  // CHECK-LABEL: @test_mm_mask_minmaxne_pbf16(
  // CHECK: call <8 x i16> @llvm.x86.avx512minmax.mask.vminmaxnepbf16128(
  return _mm_mask_minmaxne_pbf16(__A, __B, __C, __D, 127);
}

__m128bh test_mm_maskz_minmaxne_pbf16(__mmask8 __A, __m128bh __B, __m128bh __C) {
  // CHECK-LABEL: @test_mm_maskz_minmaxne_pbf16(
  // CHECK: call <8 x i16> @llvm.x86.avx512minmax.mask.vminmaxnepbf16128(
  return _mm_maskz_minmaxne_pbf16(__A, __B, __C, 127);
}

__m256bh test_mm256_minmaxne_pbf16(__m256bh __A, __m256bh __B) {
  // CHECK-LABEL: @test_mm256_minmaxne_pbf16(
  // CHECK: call <16 x i16> @llvm.x86.avx512minmax.vminmaxnepbf16256(
  return _mm256_minmaxne_pbf16(__A, __B, 127);
}

__m256bh test_mm256_mask_minmaxne_pbf16(__m256bh __A, __mmask16 __B, __m256bh __C, __m256bh __D) {
  // CHECK-LABEL: @test_mm256_mask_minmaxne_pbf16(
  // CHECK: call <16 x i16> @llvm.x86.avx512minmax.mask.vminmaxnepbf16256(
  return _mm256_mask_minmaxne_pbf16(__A, __B, __C, __D, 127);
}

__m256bh test_mm256_maskz_minmaxne_pbf16(__mmask16 __A, __m256bh __B, __m256bh __C) {
  // CHECK-LABEL: @test_mm256_maskz_minmaxne_pbf16(
  // CHECK: call <16 x i16> @llvm.x86.avx512minmax.mask.vminmaxnepbf16256(
  return _mm256_maskz_minmaxne_pbf16(__A, __B, __C, 127);
}

__m128d test_mm_minmax_pd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_minmax_pd(
  // CHECK: call <2 x double> @llvm.x86.avx512minmax.vminmaxpd128(
  return _mm_minmax_pd(__A, __B, 127);
}

__m128d test_mm_mask_minmax_pd(__m128d __A, __mmask8 __B, __m128d __C, __m128d __D) {
  // CHECK-LABEL: @test_mm_mask_minmax_pd(
  // CHECK: call <2 x double> @llvm.x86.avx512minmax.mask.vminmaxpd128(
  return _mm_mask_minmax_pd(__A, __B, __C, __D, 127);
}

__m128d test_mm_maskz_minmax_pd(__mmask8 __A, __m128d __B, __m128d __C) {
  // CHECK-LABEL: @test_mm_maskz_minmax_pd(
  // CHECK: call <2 x double> @llvm.x86.avx512minmax.mask.vminmaxpd128(
  return _mm_maskz_minmax_pd(__A, __B, __C, 127);
}

__m256d test_mm256_minmax_pd(__m256d __A, __m256d __B) {
  // CHECK-LABEL: @test_mm256_minmax_pd(
  // CHECK: call <4 x double> @llvm.x86.avx512minmax.vminmaxpd256(
  return _mm256_minmax_pd(__A, __B, 127);
}

__m256d test_mm256_mask_minmax_pd(__m256d __A, __mmask8 __B, __m256d __C, __m256d __D) {
  // CHECK-LABEL: @test_mm256_mask_minmax_pd(
  // CHECK: call <4 x double> @llvm.x86.avx512minmax.mask.vminmaxpd256(
  return _mm256_mask_minmax_pd(__A, __B, __C, __D, 127);
}

__m256d test_mm256_maskz_minmax_pd(__mmask8 __A, __m256d __B, __m256d __C) {
  // CHECK-LABEL: @test_mm256_maskz_minmax_pd(
  // CHECK: call <4 x double> @llvm.x86.avx512minmax.mask.vminmaxpd256(
  return _mm256_maskz_minmax_pd(__A, __B, __C, 127);
}

__m128h test_mm_minmax_ph(__m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_minmax_ph(
  // CHECK: call <8 x half> @llvm.x86.avx512minmax.vminmaxph128(
  return _mm_minmax_ph(__A, __B, 127);
}

__m128h test_mm_mask_minmax_ph(__m128h __A, __mmask8 __B, __m128h __C, __m128h __D) {
  // CHECK-LABEL: @test_mm_mask_minmax_ph(
  // CHECK: call <8 x half> @llvm.x86.avx512minmax.mask.vminmaxph128(
  return _mm_mask_minmax_ph(__A, __B, __C, __D, 127);
}

__m128h test_mm_maskz_minmax_ph(__mmask8 __A, __m128h __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_maskz_minmax_ph(
  // CHECK: call <8 x half> @llvm.x86.avx512minmax.mask.vminmaxph128(
  return _mm_maskz_minmax_ph(__A, __B, __C, 127);
}

__m256h test_mm256_minmax_ph(__m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_minmax_ph(
  // CHECK: call <16 x half> @llvm.x86.avx512minmax.vminmaxph256(
  return _mm256_minmax_ph(__A, __B, 127);
}

__m256h test_mm256_mask_minmax_ph(__m256h __A, __mmask16 __B, __m256h __C, __m256h __D) {
  // CHECK-LABEL: @test_mm256_mask_minmax_ph(
  // CHECK: call <16 x half> @llvm.x86.avx512minmax.mask.vminmaxph256(
  return _mm256_mask_minmax_ph(__A, __B, __C, __D, 127);
}

__m256h test_mm256_maskz_minmax_ph(__mmask16 __A, __m256h __B, __m256h __C) {
  // CHECK-LABEL: @test_mm256_maskz_minmax_ph(
  // CHECK: call <16 x half> @llvm.x86.avx512minmax.mask.vminmaxph256(
  return _mm256_maskz_minmax_ph(__A, __B, __C, 127);
}

__m128 test_mm_minmax_ps(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_minmax_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512minmax.vminmaxps128(
  return _mm_minmax_ps(__A, __B, 127);
}

__m128 test_mm_mask_minmax_ps(__m128 __A, __mmask8 __B, __m128 __C, __m128 __D) {
  // CHECK-LABEL: @test_mm_mask_minmax_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512minmax.mask.vminmaxps128(
  return _mm_mask_minmax_ps(__A, __B, __C, __D, 127);
}

__m128 test_mm_maskz_minmax_ps(__mmask8 __A, __m128 __B, __m128 __C) {
  // CHECK-LABEL: @test_mm_maskz_minmax_ps(
  // CHECK: call <4 x float> @llvm.x86.avx512minmax.mask.vminmaxps128(
  return _mm_maskz_minmax_ps(__A, __B, __C, 127);
}

__m256 test_mm256_minmax_ps(__m256 __A, __m256 __B) {
  // CHECK-LABEL: @test_mm256_minmax_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512minmax.vminmaxps256(
  return _mm256_minmax_ps(__A, __B, 127);
}

__m256 test_mm256_mask_minmax_ps(__m256 __A, __mmask8 __B, __m256 __C, __m256 __D) {
  // CHECK-LABEL: @test_mm256_mask_minmax_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512minmax.mask.vminmaxps256(
  return _mm256_mask_minmax_ps(__A, __B, __C, __D, 127);
}

__m256 test_mm256_maskz_minmax_ps(__mmask8 __A, __m256 __B, __m256 __C) {
  // CHECK-LABEL: @test_mm256_maskz_minmax_ps(
  // CHECK: call <8 x float> @llvm.x86.avx512minmax.mask.vminmaxps256(
  return _mm256_maskz_minmax_ps(__A, __B, __C, 127);
}


