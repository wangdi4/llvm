// REQUIRES: intel_feature_isa_amx_transpose2
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown \
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

void test_tile_4rqntlvbz0(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_4rqntlvbz0
  // CHECK: call void @llvm.x86.t4rqntlvbz0(i8 0, i8* %{{.*}}, i64 %{{.*}})
  _tile_4rqntlvbz0(0, A, B);
}

void test_tile_4rqntlvbz0t1(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_4rqntlvbz0t1
  // CHECK: call void @llvm.x86.t4rqntlvbz0t1(i8 0, i8* %{{.*}}, i64 %{{.*}})
  _tile_4rqntlvbz0t1(0, A, B);
}

void test_tile_4rqntlvbz1(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_4rqntlvbz1
  // CHECK: call void @llvm.x86.t4rqntlvbz1(i8 4, i8* %{{.*}}, i64 %{{.*}})
  _tile_4rqntlvbz1(4, A, B);
}

void test_tile_4rqntlvbz1t1(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_4rqntlvbz1t1
  // CHECK: call void @llvm.x86.t4rqntlvbz1t1(i8 4, i8* %{{.*}}, i64 %{{.*}})
  _tile_4rqntlvbz1t1(4, A, B);
}

void test_tile_4rqntlvbz2(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_4rqntlvbz2
  // CHECK: call void @llvm.x86.t4rqntlvbz2(i8 7, i8* %{{.*}}, i64 %{{.*}})
  _tile_4rqntlvbz2(7, A, B);
}

void test_tile_4rqntlvbz2t1(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_4rqntlvbz2t1
  // CHECK: call void @llvm.x86.t4rqntlvbz2t1(i8 1, i8* %{{.*}}, i64 %{{.*}})
  _tile_4rqntlvbz2t1(1, A, B);
}

void test_tile_4rqntlvbz3(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_4rqntlvbz3
  // CHECK: call void @llvm.x86.t4rqntlvbz3(i8 1, i8* %{{.*}}, i64 %{{.*}})
  _tile_4rqntlvbz3(1, A, B);
}

void test_tile_4rqntlvbz3t1(const void *A, size_t B) {
  // CHECK-LABEL: @test_tile_4rqntlvbz3t1
  // CHECK: call void @llvm.x86.t4rqntlvbz3t1(i8 1, i8* %{{.*}}, i64 %{{.*}})
  _tile_4rqntlvbz3t1(1, A, B);
}

void test_tile_tdpbssd(void) {
  // CHECK-LABEL: @test_tile_tdpbssd
  // CHECK: call void @llvm.x86.ttdpbssd(i8 1, i8 2, i8 3)
  _tile_tdpbssd(1, 2, 3);
}

void test_tile_tdpbsud(void) {
  // CHECK-LABEL: @test_tile_tdpbsud
  // CHECK: call void @llvm.x86.ttdpbsud(i8 1, i8 2, i8 3)
  _tile_tdpbsud(1, 2, 3);
}

void test_tile_tdpbusd(void) {
  // CHECK-LABEL: @test_tile_tdpbusd
  // CHECK: call void @llvm.x86.ttdpbusd(i8 1, i8 2, i8 3)
  _tile_tdpbusd(1, 2, 3);
}

void test_tile_tdpbuud(void) {
  // CHECK-LABEL: @test_tile_tdpbuud
  // CHECK: call void @llvm.x86.ttdpbuud(i8 1, i8 2, i8 3)
  _tile_tdpbuud(1, 2, 3);
}
