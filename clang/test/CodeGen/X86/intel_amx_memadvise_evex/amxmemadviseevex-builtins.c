// REQUIRES: intel_feature_isa_amx_memadvise_evex
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile -target-feature +amx-memadvise-evex \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

void test_tile_t2rpntlvwz0advisee(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_t2rpntlvwz0advisee(
  // CHECK: call void @llvm.x86.t2rpntlvwz0advisee(i8 1, i8* %{{.*}}, i64 %{{.*}}, i32 127)
  _tile_t2rpntlvwz0advisee(1, A, B, 127);
}

void test_tile_t2rpntlvwz1advisee(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_t2rpntlvwz1advisee(
  // CHECK: call void @llvm.x86.t2rpntlvwz1advisee(i8 1, i8* %{{.*}}, i64 %{{.*}}, i32 127)
  _tile_t2rpntlvwz1advisee(1, A, B, 127);
}
