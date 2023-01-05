// REQUIRES: intel_feature_isa_avx256p
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +avx256p -emit-llvm -o - | FileCheck %s

#include <immintrin.h>

__m256i test_mm256_clmulepi64_epi128(__m256i A, __m256i B) {
  // CHECK: @llvm.x86.pclmulqdq.256
  return _mm256_clmulepi64_epi128(A, B, 0);
}

