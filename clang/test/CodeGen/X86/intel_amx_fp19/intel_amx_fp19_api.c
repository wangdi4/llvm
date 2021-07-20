// REQUIRES: intel_feature_isa_amx_fp19
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown  -target-feature +avx512f  -target-feature +amx-bf16  \
// RUN: -target-feature +amx-fp19 \
// RUN: -emit-llvm -o - -Werror -pedantic | FileCheck %s --check-prefixes=CHECK

#include <immintrin.h>

char buf[1024];
#define STRIDE 32

char buf2[1024];

void test_tile_tmmulfp19ps(__tile1024i a, __tile1024i b, __tile1024i c) {
  //CHECK-LABEL: @test_tile_tmmulfp19ps
  //CHECK: call x86_amx @llvm.x86.tmmulfp19ps.internal
  //CHECK-NEXT: {{%.*}} = bitcast x86_amx {{%.*}} to <256 x i32>
  __tile_tmmulfp19ps(&c, a, b);
}

void test_tile_ttmmulfp19ps(__tile1024i a, __tile1024i b, __tile1024i c) {
  //CHECK-LABEL: @test_tile_ttmmulfp19ps
  //CHECK: call x86_amx @llvm.x86.ttmmulfp19ps.internal
  //CHECK-NEXT: {{%.*}} = bitcast x86_amx {{%.*}} to <256 x i32>
  __tile_ttmmulfp19ps(&c, a, b);
}
