// REQUIRES: intel_feature_isa_amx_tf32
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile -target-feature +amx-tf32 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

void test_tile_mmultf32ps(void) {
  // CHECK-LABEL: @test_tile_mmultf32ps(
  // CHECK: call void @llvm.x86.tmmultf32ps(i8 1, i8 2, i8 3)
  _tile_mmultf32ps(1, 2, 3);
}

void test_tile_tmmultf32ps(void) {
  // CHECK-LABEL: @test_tile_tmmultf32ps(
  // CHECK: call void @llvm.x86.ttmmultf32ps(i8 1, i8 2, i8 3)
  _tile_tmmultf32ps(1, 2, 3);
}
