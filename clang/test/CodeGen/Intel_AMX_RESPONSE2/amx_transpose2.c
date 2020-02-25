// REQUIRES: intel_feature_isa_amx_transpose2
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-tile -target-feature +amx-transpose2 -emit-llvm -o - -Wall -Werror -pedantic \
// RUN: -Wno-gnu-statement-expression| FileCheck %s

#include <immintrin.h>
#include <stddef.h>
// Transpose2
void test_tile_2transposew(const void *A, size_t B, size_t C) {
  // CHECK-LABEL: @test_tile_2transposew
  // CHECK: call void @llvm.x86.t2transposew(i8 1, i8* %{{.*}}, i64 %{{.*}}, i64 %{{.*}})
  _tile_2transposew(1, A, B, C);
}

void test_tile_2transposewt1(const void *A, size_t B, size_t C) {
  // CHECK-LABEL: @test_tile_2transposewt1
  // CHECK: call void @llvm.x86.t2transposewt1(i8 1, i8* %{{.*}}, i64 %{{.*}}, i64 %{{.*}})
  _tile_2transposewt1(1, A, B, C);
}
