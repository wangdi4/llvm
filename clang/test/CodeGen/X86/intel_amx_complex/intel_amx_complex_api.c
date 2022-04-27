// REQUIRES: intel_feature_isa_amx_complex
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown  -target-feature +avx512f  -target-feature +amx-bf16  \
// RUN: -target-feature +amx-complex \
// RUN: -emit-llvm -o - -Werror -pedantic | FileCheck %s --check-prefixes=CHECK

#include <immintrin.h>

char buf[1024];
#define STRIDE 32

char buf2[1024];

void test_tile_tcmmimfp16ps(__tile1024i a, __tile1024i b, __tile1024i c) {
  //CHECK-LABEL: @test_tile_tcmmimfp16ps
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call x86_amx @llvm.x86.tcmmimfp16ps.internal
  //CHECK-DAG: call <256 x i32> @llvm.x86.cast.tile.to.vector.v256i32(x86_amx {{%.*}})
  __tile_tcmmimfp16ps(&c, a, b);
}

void test_tile_tcmmrlfp16ps(__tile1024i a, __tile1024i b, __tile1024i c) {
  //CHECK-LABEL: @test_tile_tcmmrlfp16ps
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call x86_amx @llvm.x86.tcmmrlfp16ps.internal
  //CHECK-DAG: call <256 x i32> @llvm.x86.cast.tile.to.vector.v256i32(x86_amx {{%.*}})
  __tile_tcmmrlfp16ps(&c, a, b);
}

void test_tile_ttcmmimfp16ps(__tile1024i a, __tile1024i b, __tile1024i c) {
  //CHECK-LABEL: @test_tile_ttcmmimfp16ps
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call x86_amx @llvm.x86.ttcmmimfp16ps.internal
  //CHECK-DAG: call <256 x i32> @llvm.x86.cast.tile.to.vector.v256i32(x86_amx {{%.*}})
  __tile_ttcmmimfp16ps(&c, a, b);
}

void test_tile_ttcmmrlfp16ps(__tile1024i a, __tile1024i b, __tile1024i c) {
  //CHECK-LABEL: @test_tile_ttcmmrlfp16ps
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call x86_amx @llvm.x86.ttcmmrlfp16ps.internal
  //CHECK-DAG: call <256 x i32> @llvm.x86.cast.tile.to.vector.v256i32(x86_amx {{%.*}})
  __tile_ttcmmrlfp16ps(&c, a, b);
}

void test_tile_tconjtcmmimfp16ps(__tile1024i a, __tile1024i b, __tile1024i c) {
  //CHECK-LABEL: @test_tile_tconjtcmmimfp16ps
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call x86_amx @llvm.x86.tconjtcmmimfp16ps.internal
  //CHECK-DAG: call <256 x i32> @llvm.x86.cast.tile.to.vector.v256i32(x86_amx {{%.*}})
  __tile_tconjtcmmimfp16ps(&c, a, b);
}

void test_tile_tconjtfp16(__tile1024i dst, __tile1024i src) {
  //CHECK-LABEL: @test_tile_tconjtfp16
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call x86_amx @llvm.x86.tconjtfp16.internal
  //CHECK-DAG: call <256 x i32> @llvm.x86.cast.tile.to.vector.v256i32(x86_amx {{%.*}})
  __tile_tconjtfp16(&dst, src);
}
