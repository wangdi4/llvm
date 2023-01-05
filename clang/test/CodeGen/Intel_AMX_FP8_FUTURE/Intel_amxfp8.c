// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown  -target-feature +amx-fp8  \
// RUN: -emit-llvm -o - -Werror -pedantic | FileCheck %s --check-prefixes=CHECK
// REQUIRES: intel_feature_isa_amx_fp8_future
#include <immintrin.h>

void test_amx5(void *data) {
  //CHECK-LABEL: @test_amx5
  //CHECK: call void @llvm.x86.ttdpbf8ps(i8 1, i8 2, i8 3)
  _tile_tdpbf8ps(1, 2, 3);
}

void test_amx6(void *data) {
  //CHECK-LABEL: @test_amx6
  //CHECK: call void @llvm.x86.ttdpbhf8ps(i8 1, i8 2, i8 3)
  _tile_tdpbhf8ps(1, 2, 3);
}

void test_amx7(void *data) {
  //CHECK-LABEL: @test_amx7
  //CHECK: call void @llvm.x86.ttdphbf8ps(i8 1, i8 2, i8 3)
  _tile_tdphbf8ps(1, 2, 3);
}

void test_amx8(void *data) {
  //CHECK-LABEL: @test_amx8
  //CHECK: call void @llvm.x86.ttdphf8ps(i8 1, i8 2, i8 3)
  _tile_tdphf8ps(1, 2, 3);
}
