// REQUIRES: intel_feature_isa_amx_v3
// RUN: %clang_cc1 -no-opaque-pointers %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-v3 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

void test_tile_loadtransposed(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_loadtransposed(
  // CHECK: call void @llvm.x86.tloadtransposed(i8 1, i8* %{{.*}}, i64 %{{.*}})
  _tile_loadtransposed(1, A, B);
}

void test_tile_loadtransposedt1(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_loadtransposedt1(
  // CHECK: call void @llvm.x86.tloadtransposedt1(i8 1, i8* %{{.*}}, i64 %{{.*}})
  _tile_loadtransposedt1(1, A, B);
}

void test_tile_rpntlvwz0(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_rpntlvwz0(
  // CHECK: call void @llvm.x86.trpntlvwz0(i8 1, i8* %{{.*}}, i64 %{{.*}})
  _tile_rpntlvwz0(1, A, B);
}

void test_tile_rpntlvwz0t1(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_rpntlvwz0t1(
  // CHECK: call void @llvm.x86.trpntlvwz0t1(i8 1, i8* %{{.*}}, i64 %{{.*}})
  _tile_rpntlvwz0t1(1, A, B);
}

void test_tile_rpntlvwz1(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_rpntlvwz1(
  // CHECK: call void @llvm.x86.trpntlvwz1(i8 1, i8* %{{.*}}, i64 %{{.*}})
  _tile_rpntlvwz1(1, A, B);
}

void test_tile_rpntlvwz1t1(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_rpntlvwz1t1(
  // CHECK: call void @llvm.x86.trpntlvwz1t1(i8 1, i8* %{{.*}}, i64 %{{.*}})
  _tile_rpntlvwz1t1(1, A, B);
}

void test_tile_storetransposed(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_storetransposed(
  // CHECK: call void @llvm.x86.tstoretransposed(i8* %{{.*}}, i64 %{{.*}}, i8 1)
  _tile_storetransposed(A, B, 1);
}
