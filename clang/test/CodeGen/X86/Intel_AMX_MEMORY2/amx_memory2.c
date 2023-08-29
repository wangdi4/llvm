// REQUIRES: intel_feature_isa_amx_memory2
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile -target-feature +amx-memory2  \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression| FileCheck %s

#include <immintrin.h>
#include <stddef.h>
// Memory2
void test_tile_broadcastrowd(const void *A) {
  // CHECK-LABEL: @test_tile_broadcastrowd
  // CHECK: call void @llvm.x86.tbroadcastrowd(i8 1, ptr %{{.*}})
  _tile_broadcastrowd(1, A);
}

void test_tile_storehd(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_storehd
  // CHECK: call void @llvm.x86.tstorehd(ptr %{{.*}}, i64 %{{.*}}, i8 1)
  _tile_storehd(A, B, 1);
}

void test_tile_storehdt1(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_storehdt1
  // CHECK: call void @llvm.x86.tstorehdt1(ptr %{{.*}}, i64 %{{.*}}, i8 1)
  _tile_storehdt1(A, B, 1);
}

void test_tile_storentd(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_storentd
  // CHECK: call void @llvm.x86.tstorentd(ptr %{{.*}}, i64 %{{.*}}, i8 1)
  _tile_storentd(A, B, 1);
}

void test_tile_storeqd(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_storeqd
  // CHECK: call void @llvm.x86.tstoreqd(ptr %{{.*}}, i64 %{{.*}}, i8 1)
  _tile_storeqd(A, B, 1);
}

void test_tile_storeqdt1(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_storeqdt1
  // CHECK: call void @llvm.x86.tstoreqdt1(ptr %{{.*}}, i64 %{{.*}}, i8 1)
  _tile_storeqdt1(A, B, 1);
}

void test_tile_storerowd(void *A) {
  // CHECK-LABEL: @test_tile_storerowd
  // CHECK: call void @llvm.x86.tstorerowd(ptr %{{.*}}, i8 1)
  _tile_storerowd(A, 1);
}
