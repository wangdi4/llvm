// REQUIRES: intel_feature_isa_amx_lnc
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown  -target-feature +avx512f  -target-feature +amx-bf16  \
// RUN: -target-feature +amx-transpose \
// RUN: -target-feature +amx-avx512 \
// RUN: -emit-llvm -o - -Werror -pedantic | FileCheck %s --check-prefixes=CHECK

#include <immintrin.h>

char buf[1024];
#define STRIDE 32

char buf2[1024];

void test_tile_transposed(__tile1024i dst, __tile1024i src) {
  //CHECK-LABEL: @test_tile_transposed
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call x86_amx @llvm.x86.ttransposed.internal
  //CHECK-DAG: call <256 x i32> @llvm.x86.cast.tile.to.vector.v256i32(x86_amx {{%.*}})
  __tile_transposed(&dst, src);
}

void test_tile_tdpbf16ps(__tile1024i a, __tile1024i b, __tile1024i c) {
  //CHECK-LABEL: @test_tile_tdpbf16ps
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call x86_amx @llvm.x86.ttdpbf16ps.internal
  //CHECK-DAG: call <256 x i32> @llvm.x86.cast.tile.to.vector.v256i32(x86_amx {{%.*}})
  __tile_tdpbf16ps(&c, a, b);
}

void test_tile_tdpfp16ps(__tile1024i a, __tile1024i b, __tile1024i c) {
  //CHECK-LABEL: @test_tile_tdpfp16ps
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call x86_amx @llvm.x86.ttdpfp16ps.internal
  //CHECK-DAG: call <256 x i32> @llvm.x86.cast.tile.to.vector.v256i32(x86_amx {{%.*}})
  __tile_tdpfp16ps(&c, a, b);
}

__m512 test_tile_movrow(__tile1024i a, unsigned b) {
  //CHECK-LABEL: @test_tile_movrow
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <16 x float> @llvm.x86.tilemovrowe.internal
 return __tile_movrow(a, b);
}

__m512 test_tile_movrowi(__tile1024i a) {
  //CHECK-LABEL: @test_tile_movrowi
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <16 x float> @llvm.x86.tilemovrowi.internal
 return __tile_movrowi(a, 15);
}
