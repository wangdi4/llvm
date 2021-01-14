// REQUIRES: intel_feature_isa_amx_complex_evex
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile -target-feature +amx-complex-evex  -target-feature +avx512fp16 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>
__m512h test_tile_tcvtrowps2phiee(__m512h A, unsigned int B) {
  // CHECK-LABEL: @test_tile_tcvtrowps2phiee(
  // CHECK: call <32 x half> @llvm.x86.tcvtrowps2phiee(<32 x half> %{{.*}}, i8 1, i32 %{{.*}})
  return _tile_tcvtrowps2phiee(A, 1, B);
}

__m512h test_tile_tcvtrowps2phiei(__m512h A) {
  // CHECK-LABEL: @test_tile_tcvtrowps2phiei(
  // CHECK: call <32 x half> @llvm.x86.tcvtrowps2phiei(<32 x half> %{{.*}}, i8 1, i32 127)
  return _tile_tcvtrowps2phiei(A, 1, 127);
}
