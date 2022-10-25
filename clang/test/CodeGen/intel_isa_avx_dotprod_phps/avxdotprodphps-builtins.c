// REQUIRES: intel_feature_isa_avx_dotprod_phps
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-unknown-unknown -target-feature +avxdotprodphps -target-feature +avx512fp16 -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

// CHECK-LABEL: @test_mm_dpph_ps(
// CHECK:     call <4 x float> @llvm.x86.avx2.vdpphps.128
__m128 test_mm_dpph_ps(__m128 __W, __m128h __A, __m128h __B) {
  return _mm_dpph_ps(__W, __A, __B);
}

// CHECK-LABEL: @test_mm256_dpph_ps(
// CHECK:     call <8 x float> @llvm.x86.avx2.vdpphps.256
__m256 test_mm256_dpph_ps(__m256 __W, __m256h __A, __m256h __B) {
  return _mm256_dpph_ps(__W, __A, __B);
}
