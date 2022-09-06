// REQUIRES: intel_feature_isa_amx_convert
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile -target-feature +amx-convert -emit-llvm -o - -Wall -Werror -pedantic \
// RUN: -Wno-gnu-statement-expression| FileCheck %s

#include <immintrin.h>
#include <stddef.h>
// AMX-CONVERT
void test_tile_cvt2ps2bf16(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_cvt2ps2bf16
  // CHECK: call void @llvm.x86.tcvt2ps2bf16(i8* %0, i64 %1, i8 1, i8 2)
  _tile_cvt2ps2bf16(A, B, 1, 2);
}

void test_tile_cvt2ps2ph(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_cvt2ps2ph
  // CHECK:  call void @llvm.x86.tcvt2ps2ph(i8* %0, i64 %1, i8 3, i8 4)
  _tile_cvt2ps2ph(A, B, 3, 4);
}

void test_tile_amxconvert_cvtd2ps(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_amxconvert_cvtd2ps
  // CHECK: call void @llvm.x86.amxconvert.tcvtd2ps(i8* %0, i64 %1, i8 1)
  _tile_amxconvert_cvtd2ps(A, B, 1);
}

void test_tile_amxconvert_cvtps2bf16(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_amxconvert_cvtps2bf16
  // CHECK: call void @llvm.x86.amxconvert.tcvtps2bf16(i8* %0, i64 %1, i8 2)
  _tile_amxconvert_cvtps2bf16(A, B, 2);
}

void test_tile_cvtps2ph(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_cvtps2ph
  // CHECK: call void @llvm.x86.tcvtps2ph(i8* %0, i64 %1, i8 3)
  _tile_cvtps2ph(A, B, 3);
}
