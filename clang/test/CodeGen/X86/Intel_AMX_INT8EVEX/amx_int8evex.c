// REQUIRES: intel_feature_isa_amx_int8_evex
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-tile -target-feature +amx-int8-evex    \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>
// INT8-EVEX
void test_tile_tdpbssde(void) {
  // CHECK-LABEL: @test_tile_tdpbssde
  // CHECK: call void @llvm.x86.tdpbssde(i8 1, i8 2, i8 3)
  _tile_dpbssde(1, 2, 3);
}

void test_tile_tdpbsude(void) {
  // CHECK-LABEL: @test_tile_tdpbsude
  // CHECK: call void @llvm.x86.tdpbsude(i8 1, i8 2, i8 3)
  _tile_dpbsude(1, 2, 3);
}

void test_tile_tdpbusde(void) {
  // CHECK-LABEL: @test_tile_tdpbusde
  // CHECK: call void @llvm.x86.tdpbusde(i8 1, i8 2, i8 3)
  _tile_dpbusde(1, 2, 3);
}

void test_tile_tdpbuude(void) {
  // CHECK-LABEL: @test_tile_tdpbuude
  // CHECK: call void @llvm.x86.tdpbuude(i8 1, i8 2, i8 3)
  _tile_dpbuude(1, 2, 3);
}
