// REQUIRES: intel_feature_isa_amx_tile2
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile -target-feature +amx-tile2 -emit-llvm -o - -Wall -Werror -pedantic \
// RUN: -Wno-gnu-statement-expression| FileCheck %s

#include <immintrin.h>
#include <stddef.h>
// AMX-TILE2
void test_tile_tilemov(void)
{
  // CHECK-LABEL: @test_tile_tilemov
  // CHECK: call void @llvm.x86.tilemov(i8 1, i8 2)
  _tile_tilemov(1, 2);
}
