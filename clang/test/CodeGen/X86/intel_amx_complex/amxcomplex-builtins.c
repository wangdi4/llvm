// REQUIRES: intel_feature_isa_amx_complex
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile -target-feature +amx-complex \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>
void test_tile_tcmmimfp16ps(void) {
  // CHECK-LABEL: @test_tile_tcmmimfp16ps
  // CHECK: call void @llvm.x86.tcmmimfp16ps(i8 1, i8 2, i8 3)
  _tile_tcmmimfp16ps(1, 2, 3);
}

void test_tile_tcmmrlfp16ps(void) {
  // CHECK-LABEL: @test_tile_tcmmrlfp16ps
  // CHECK: call void @llvm.x86.tcmmrlfp16ps(i8 1, i8 2, i8 3)
  _tile_tcmmrlfp16ps(1, 2, 3);
}

void test_tile_tconjtcmmimfp16ps(void) {
  // CHECK-LABEL: @test_tile_tconjtcmmimfp16ps
  // CHECK: call void @llvm.x86.tconjtcmmimfp16ps(i8 1, i8 2, i8 3)
  _tile_tconjtcmmimfp16ps(1, 2, 3);
}

void test_tile_tconjtfp16(void) {
  // CHECK-LABEL: @test_tile_tconjtfp16
  // CHECK: call void @llvm.x86.tconjtfp16(i8 1, i8 2)
  _tile_tconjtfp16(1, 2);
}

void test_tile_ttcmmimfp16ps(void) {
  // CHECK-LABEL: @test_tile_ttcmmimfp16ps
  // CHECK: call void @llvm.x86.ttcmmimfp16ps(i8 1, i8 2, i8 3)
  _tile_ttcmmimfp16ps(1, 2, 3);
}

void test_tile_ttcmmrlfp16ps(void) {
  // CHECK-LABEL: @test_tile_ttcmmrlfp16ps
  // CHECK: call void @llvm.x86.ttcmmrlfp16ps(i8 1, i8 2, i8 3)
  _tile_ttcmmrlfp16ps(1, 2, 3);
}

