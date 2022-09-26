// REQUIRES: intel_feature_isa_avx512_movget
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +avx512movget -target-feature +avx512vl \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m128i test_mm_vmovget_epi32(const __m128i *__A) {
  // CHECK-LABEL: @test_mm_vmovget_epi32(
  // CHECK: call <4 x i32> @llvm.x86.vmovget128(i8* %{{.*}})
  return _mm_vmovget_epi32(__A);
}

__m256i test_mm256_vmovget_epi32(const __m256i *__A) {
  // CHECK-LABEL: @test_mm256_vmovget_epi32(
  // CHECK: call <8 x i32> @llvm.x86.vmovget256(i8* %{{.*}})
  return _mm256_vmovget_epi32(__A);
}
