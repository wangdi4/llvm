// REQUIRES: intel_feature_isa_avx256p_unsupported
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +avx256p -emit-llvm -o - | FileCheck %s

#include <immintrin.h>

__m256i test_mm256_aesenc_epi128(__m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_aesenc_epi128
  // CHECK: @llvm.x86.aesni.aesenc.256
  return _mm256_aesenc_epi128(__A, __B);
}

__m256i test_mm256_aesenclast_epi128(__m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_aesenclast_epi128
  // CHECK: @llvm.x86.aesni.aesenclast.256
  return _mm256_aesenclast_epi128(__A, __B);
}

__m256i test_mm256_aesdec_epi128(__m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_aesdec_epi128
  // CHECK: @llvm.x86.aesni.aesdec.256
  return _mm256_aesdec_epi128(__A, __B);
}

__m256i test_mm256_aesdeclast_epi128(__m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_aesdeclast_epi128
  // CHECK: @llvm.x86.aesni.aesdeclast.256
  return _mm256_aesdeclast_epi128(__A, __B);
}

