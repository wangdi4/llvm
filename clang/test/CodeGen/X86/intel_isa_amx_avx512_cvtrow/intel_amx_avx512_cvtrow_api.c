// REQUIRES: intel_feature_isa_amx_avx512_cvtrow
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown  -target-feature +avx512f  -target-feature +amx-bf16  \
// RUN: -target-feature +amx-avx512-cvtrow -target-feature +avx512fp16 \
// RUN: -emit-llvm -o - -Werror -pedantic | FileCheck %s --check-prefixes=CHECK

#include <immintrin.h>

char buf[1024];
#define STRIDE 32

char buf2[1024];

__m512 test_tile_cvtrowd2ps(__tile1024i a, unsigned b) {
  //CHECK-LABEL: @test_tile_cvtrowd2ps
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <16 x float> @llvm.x86.tcvtrowd2pse.internal
 return __tile_cvtrowd2ps(a, b);
}

__m512 test_tile_cvtrowd2psi(__tile1024i a) {
  //CHECK-LABEL: @test_tile_cvtrowd2psi
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <16 x float> @llvm.x86.tcvtrowd2psi.internal
 return __tile_cvtrowd2psi(a, 15);
}

__m512bh test_tile_cvtrowps2pbf16h(__tile1024i a, unsigned b) {
  //CHECK-LABEL: @test_tile_cvtrowps2pbf16h
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x i16> @llvm.x86.tcvtrowps2pbf16he.internal
 return __tile_cvtrowps2pbf16h(a, b);
}

__m512bh test_tile_cvtrowps2pbf16hi(__tile1024i a) {
  //CHECK-LABEL: @test_tile_cvtrowps2pbf16hi
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x i16> @llvm.x86.tcvtrowps2pbf16hi.internal
 return __tile_cvtrowps2pbf16hi(a, 15);
}

__m512bh test_tile_cvtrowps2pbf16l(__tile1024i a, unsigned b) {
  //CHECK-LABEL: @test_tile_cvtrowps2pbf16l
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x i16> @llvm.x86.tcvtrowps2pbf16le.internal
 return __tile_cvtrowps2pbf16l(a, b);
}

__m512bh test_tile_cvtrowps2pbf16li(__tile1024i a) {
  //CHECK-LABEL: @test_tile_cvtrowps2pbf16li
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x i16> @llvm.x86.tcvtrowps2pbf16li.internal
 return __tile_cvtrowps2pbf16li(a, 15);
}

__m512h test_tile_cvtrowps2phh(__tile1024i a, unsigned b) {
  //CHECK-LABEL: @test_tile_cvtrowps2phh
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x half> @llvm.x86.tcvtrowps2phhe.internal
 return __tile_cvtrowps2phh(a, b);
}

__m512h test_tile_cvtrowps2phhi(__tile1024i a) {
  //CHECK-LABEL: @test_tile_cvtrowps2phhi
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x half> @llvm.x86.tcvtrowps2phhi.internal
 return __tile_cvtrowps2phhi(a, 15);
}

__m512h test_tile_cvtrowps2phl(__tile1024i a, unsigned b) {
  //CHECK-LABEL: @test_tile_cvtrowps2phl
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x half> @llvm.x86.tcvtrowps2phle.internal
 return __tile_cvtrowps2phl(a, b);
}

__m512h test_tile_cvtrowps2phli(__tile1024i a) {
  //CHECK-LABEL: @test_tile_cvtrowps2phli
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x half> @llvm.x86.tcvtrowps2phli.internal
 return __tile_cvtrowps2phli(a, 15);
}
