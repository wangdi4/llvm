// REQUIRES: intel_feature_isa_avx512_movget
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +avx512movget \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m512i test_mm512_vmovget_epi32(const __m512i *__A) {
  // CHECK-LABEL: @test_mm512_vmovget_epi32(
  // CHECK: call <16 x i32> @llvm.x86.vmovget512(i8* %{{.*}})
  return _mm512_vmovget_epi32(__A);
}
