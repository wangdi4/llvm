// REQUIRES: intel_feature_isa_amx_element_evex
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile \
// RUN: -target-feature +amx-element-evex -emit-llvm -o - -Wall -Werror -pedantic \
// RUN: -Wno-gnu-statement-expression| FileCheck %s

#include <immintrin.h>
#include <stddef.h>
//AMX_ELEMENT_EVEX
void test_tile_cvtd2pse(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_cvtd2pse
  // CHECK: call void @llvm.x86.tcvtd2pse{{.*}}, i64 %{{.*}}, i8 1)
  _tile_cvtd2pse(A, B, 1);
}
