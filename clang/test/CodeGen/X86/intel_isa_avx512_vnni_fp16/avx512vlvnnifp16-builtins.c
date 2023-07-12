// REQUIRES: intel_feature_isa_avx512_vnni_fp16
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-unknown-unknown -target-feature +avx512vl -target-feature +avx512vnnifp16 -target-feature +avx512fp16 -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

__m128 test_mm_mask_dpph_ps(__m128 __W, __mmask8 __U, __m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_mask_dpph_ps(
  // CHECK: call <4 x float> @llvm.x86.avx2.vdpphps.128
  // CHECK: select <4 x i1> %{{.*}}, <4 x float> %{{.*}}, <4 x float> %{{.*}}
  return _mm_mask_dpph_ps(__W, __U, __A, __B);
}

__m128 test_mm_maskz_dpph_ps(__mmask8 __U, __m128 __W, __m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_dpph_ps(
  // CHECK: call <4 x float> @llvm.x86.avx2.vdpphps.128
  // CHECK: zeroinitializer
  // CHECK: select <4 x i1> %{{.*}}, <4 x float> %{{.*}}, <4 x float> %{{.*}}
  return _mm_maskz_dpph_ps(__U, __W, __A, __B);
}

__m256 test_mm256_mask_dpph_ps(__m256 __W, __mmask8 __U, __m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_mask_dpph_ps(
  // CHECK: call <8 x float> @llvm.x86.avx2.vdpphps.256
  // CHECK: select <8 x i1> %{{.*}}, <8 x float> %{{.*}}, <8 x float> %{{.*}}
  return _mm256_mask_dpph_ps(__W, __U,  __A, __B);
}

__m256 test_mm256_maskz_dpph_ps(__mmask8 __U, __m256 __W, __m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_dpph_ps(
  // CHECK: call <8 x float> @llvm.x86.avx2.vdpphps.256
  // CHECK: zeroinitializer
  // CHECK: select <8 x i1> %{{.*}}, <8 x float> %{{.*}}, <8 x float> %{{.*}}
  return _mm256_maskz_dpph_ps(__U, __W, __A, __B);
}
