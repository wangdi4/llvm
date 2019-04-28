// REQUIRES: intel_feature_isa_fp16
// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-unknown-unknown -target-feature +avx512vl -target-feature +avx512fp16 -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

_Float16 test_mm_cvtsh_h(__m128h __A) {
  // CHECK-LABEL: @test_mm_cvtsh_h
  // CHECK: extractelement <8 x half> %{{.*}}, i32 0
  return _mm_cvtsh_h(__A);
}

__m128h test_mm_set_sh(_Float16 __h)
{
  // CHECK-LABEL: @test_mm_set_sh
  // CHECK: insertelement <8 x half> {{.*}}, i32 0
  // CHECK: insertelement <8 x half> %{{.*}}, half 0xH0000, i32 1
  // CHECK: insertelement <8 x half> %{{.*}}, half 0xH0000, i32 2
  // CHECK: insertelement <8 x half> %{{.*}}, half 0xH0000, i32 3
  // CHECK: insertelement <8 x half> %{{.*}}, half 0xH0000, i32 4
  // CHECK: insertelement <8 x half> %{{.*}}, half 0xH0000, i32 5
  // CHECK: insertelement <8 x half> %{{.*}}, half 0xH0000, i32 6
  // CHECK: insertelement <8 x half> %{{.*}}, half 0xH0000, i32 7
  return _mm_set_sh(__h);
}
