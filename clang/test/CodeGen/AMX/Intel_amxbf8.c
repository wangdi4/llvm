// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown  -target-feature +amx-bf8  \
// RUN: -emit-llvm -o - -Werror -pedantic | FileCheck %s --check-prefixes=CHECK
// REQUIRES: intel_feature_isa_amx_bf8
#include <immintrin.h>

void test_amx(void *data) {
  //CHECK-LABEL: @test_amx
  //CHECK: call void @llvm.x86.tdpbf8ps(i8 1, i8 2, i8 3)
  _tile_dpbf8ps(1, 2, 3);
}
