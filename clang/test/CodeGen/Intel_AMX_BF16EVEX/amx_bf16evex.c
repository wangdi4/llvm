// REQUIRES: intel_feature_isa_amx_bf16_evex
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile -target-feature +amx-bf16-evex \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression| FileCheck %s

#include <immintrin.h>
#include <stddef.h>
// BF16-EVEX
void test_tile_dpbf16pse(void) {
  // CHECK-LABEL: @test_tile_dpbf16pse
  // CHECK: call void @llvm.x86.tdpbf16pse(i8 1, i8 2, i8 3)
  _tile_dpbf16pse(1, 2, 3);
}
