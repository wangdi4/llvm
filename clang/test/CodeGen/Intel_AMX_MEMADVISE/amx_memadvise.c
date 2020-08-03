// REQUIRES: intel_feature_isa_amx_memadvise
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-memadvise  -emit-llvm -o - -Wall -Werror -pedantic \
// RUN: -Wno-gnu-statement-expression| FileCheck %s

#include <immintrin.h>
#include <stddef.h>

void test_tile_movadvise(void *A) {
  // CHECK-LABEL: @test_tile_movadvise
  // CHECK: call void @llvm.x86.tmovadvise.load(i8 1, i8* %{{.*}}, i64 32, i8 16)
  _tile_movadvise_load(1, A, 32, 16);

  // CHECK: call void @llvm.x86.tmovadvise.store(i8* %{{.*}}, i64 32, i8 7, i8 18)
  _tile_movadvise_store(A, 32, 7, 18);
}
