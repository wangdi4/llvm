// REQUIRES: intel_feature_isa_avx_bf16
//  RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-apple-darwin \
//  RUN: -target-feature +avxbf16 -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

__m128bh test_mm_avx2_cvtne2ps2bf16(__m128 A, __m128 B) {
// CHECK-LABEL: @test_mm_avx2_cvtne2ps2bf16
// CHECK: call <8 x bfloat> @llvm.x86.avx2bf16.cvtne2ps2bf16.128
        return _mm_cvtne2ps_avx_pbh(A, B);
}

__m256bh test_mm256_avx2_cvtne2ps2bf16(__m256 A, __m256 B) {
// CHECK-LABEL: @test_mm256_avx2_cvtne2ps2bf16
// CHECK: call <16 x bfloat> @llvm.x86.avx2bf16.cvtne2ps2bf16.256
        return _mm256_cvtne2ps_avx_pbh(A, B);
}

__m128 test_mm_avx2_dpbf16_ps(__m128 D, __m128bh A, __m128bh B) {
// CHECK-LABEL: @test_mm_avx2_dpbf16_ps
// CHECK: call <4 x float> @llvm.x86.avx2bf16.dpbf16ps.128
        return _mm_dpbf16_avx_ps(D, A, B);
}

__m256 test_mm256_avx2_dpbf16_ps(__m256 D, __m256bh A, __m256bh B) {
// CHECK-LABEL: @test_mm256_avx2_dpbf16_ps
// CHECK: call <8 x float> @llvm.x86.avx2bf16.dpbf16ps.256
        return _mm256_dpbf16_avx_ps(D, A, B);
}
