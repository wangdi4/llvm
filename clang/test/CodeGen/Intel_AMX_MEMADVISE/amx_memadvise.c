// REQUIRES: intel_feature_isa_amx_memadvise
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown \
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

void test_tile_t2rpntlvwz0advise(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_t2rpntlvwz0advise(
  // CHECK: call void @llvm.x86.t2rpntlvwz0advise(i8 1, i8* %{{.*}}, i64 %{{.*}}, i32 127)
  _tile_t2rpntlvwz0advise(1, A, B, 127);
}

void test_tile_t2rpntlvwz1advise(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_t2rpntlvwz1advise(
  // CHECK: call void @llvm.x86.t2rpntlvwz1advise(i8 1, i8* %{{.*}}, i64 %{{.*}}, i32 127)
  _tile_t2rpntlvwz1advise(1, A, B, 127);
}
