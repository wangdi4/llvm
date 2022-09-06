// REQUIRES: intel_feature_isa_amx_tile_evex
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-tile -target-feature +amx-tile-evex    \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic \
// RUN: -Wno-gnu-statement-expression| FileCheck %s

#include <immintrin.h>
#include <stddef.h>
// TILE-EVEX
void test_tile_loadde(const void * base, size_t stride) {
  // CHECK-LABEL: @test_tile_loadde
  // CHECK: call void @llvm.x86.tileloadde64(i8 1, i8* %0, i64 %1)
  _tile_loadde(1, base, stride);
}

void test_tile_tileloaddt164e(const void * base, size_t stride) {
  // CHECK-LABEL: @test_tile_tileloaddt164e
  // CHECK: call void @llvm.x86.tileloaddt1e64(i8 1, i8* %0, i64 %1)
  _tile_stream_loadde(1, base, stride);
}

void test_tile_tilestored64e(const void * base, size_t stride) {
  // CHECK-LABEL: @test_tile_tilestored64e
  // CHECK: call void @llvm.x86.tilestorede64(i8 1, i8* %0, i64 %1)
  _tile_storede(1, base, stride);
}

void test_tile_tilemove(void) {
  // CHECK-LABEL: @test_tile_tilemove
  // CHECK: call void @llvm.x86.tilemove(i8 1, i8 2)
  _tile_tilemove(1, 2);
}

void test_tile_tilezeroe(void) {
  // CHECK-LABEL: @test_tile_tilezeroe
  // CHECK: call void @llvm.x86.tilezeroe(i8 1)
  _tile_zeroe(1);
}
