// REQUIRES: intel_feature_isa_amx_fp19
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile -target-feature +amx-fp19 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

void test_tile_tmmulfp19ps(void) {
  // CHECK-LABEL: @test_tile_tmmulfp19ps(
  // CHECK: call void @llvm.x86.tmmulfp19ps(i8 1, i8 2, i8 3)
  _tile_tmmulfp19ps(1, 2, 3);
}

void test_tile_ttmmulfp19ps(void) {
  // CHECK-LABEL: @test_tile_ttmmulfp19ps(
  // CHECK: call void @llvm.x86.ttmmulfp19ps(i8 1, i8 2, i8 3)
  _tile_ttmmulfp19ps(1, 2, 3);
}
