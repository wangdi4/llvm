// REQUIRES: intel_feature_isa_amx_sparse
// RUN: %clang_cc1 -no-opaque-pointers %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile -target-feature +amx-sparse \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

void test_tile_dpsbssd(void) {
  // CHECK-LABEL: @test_tile_dpsbssd(
  // CHECK: call void @llvm.x86.tdpsbssd(i8 1, i8 3, i8 5)
  _tile_dpsbssd(1, 3, 5);
}

void test_tile_dpsbsud(void) {
  // CHECK-LABEL: @test_tile_dpsbsud(
  // CHECK: call void @llvm.x86.tdpsbsud(i8 1, i8 3, i8 5)
  _tile_dpsbsud(1, 3, 5);
}

void test_tile_dpsbusd(void) {
  // CHECK-LABEL: @test_tile_dpsbusd(
  // CHECK: call void @llvm.x86.tdpsbusd(i8 1, i8 3, i8 5)
  _tile_dpsbusd(1, 3, 5);
}

void test_tile_dpsbuud(void) {
  // CHECK-LABEL: @test_tile_dpsbuud(
  // CHECK: call void @llvm.x86.tdpsbuud(i8 1, i8 3, i8 5)
  _tile_dpsbuud(1, 3, 5);
}

void test_tile_dpsbf16ps(void) {
  // CHECK-LABEL: @test_tile_dpsbf16ps(
  // CHECK: call void @llvm.x86.tdpsbf16ps(i8 1, i8 3, i8 5)
  _tile_dpsbf16ps(1, 3, 5);
}

void test_tile_dpsfp16ps(void) {
  // CHECK-LABEL: @test_tile_dpsfp16ps(
  // CHECK: call void @llvm.x86.tdpsfp16ps(i8 1, i8 3, i8 5)
  _tile_dpsfp16ps(1, 3, 5);
}

void test_tile_ldexpandb(const void *A, size_t B, unsigned int C) {
  // CHECK-LABEL: @test_tile_ldexpandb(
  // CHECK: call void @llvm.x86.tldexpandb(i8 1, i8* %{{.*}}, i64 %{{.*}}, i32 %{{.*}})
  _tile_ldexpandb(1, A, B, C);
}

void test_tile_ldexpandbt1(const void *A, size_t B, unsigned int C) {
  // CHECK-LABEL: @test_tile_ldexpandbt1(
  // CHECK: call void @llvm.x86.tldexpandbt1(i8 1, i8* %{{.*}}, i64 %{{.*}}, i32 %{{.*}})
  _tile_ldexpandbt1(1, A, B, C);
}

void test_tile_ldexpandw(const void *A, size_t B, unsigned int C) {
  // CHECK-LABEL: @test_tile_ldexpandw(
  // CHECK: call void @llvm.x86.tldexpandw(i8 1, i8* %{{.*}}, i64 %{{.*}}, i32 %{{.*}})
  _tile_ldexpandw(1, A, B, C);
}

void test_tile_ldexpandwt1(const void *A, size_t B, unsigned int C) {
  // CHECK-LABEL: @test_tile_ldexpandwt1(
  // CHECK: call void @llvm.x86.tldexpandwt1(i8 1, i8* %{{.*}}, i64 %{{.*}}, i32 %{{.*}})
  _tile_ldexpandwt1(1, A, B, C);
}
