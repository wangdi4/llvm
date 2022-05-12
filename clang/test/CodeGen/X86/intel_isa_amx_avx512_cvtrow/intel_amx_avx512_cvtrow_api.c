// REQUIRES: intel_feature_isa_amx_avx512_cvtrow
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown  -target-feature +avx512f  -target-feature +amx-bf16  \
// RUN: -target-feature +amx-avx512-cvtrow -target-feature +avx512fp16 \
// RUN: -emit-llvm -o - -Werror -pedantic | FileCheck %s --check-prefixes=CHECK

#include <immintrin.h>

char buf[1024];
#define STRIDE 32

char buf2[1024];

__m512 test_tile_tcvtrowd2pse(__tile1024i a, unsigned b) {
  //CHECK-LABEL: @test_tile_tcvtrowd2pse
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <16 x float> @llvm.x86.tcvtrowd2pse.internal
 return __tile_tcvtrowd2pse(a, b);
}

__m512 test_tile_tcvtrowd2psi(__tile1024i a) {
  //CHECK-LABEL: @test_tile_tcvtrowd2psi
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <16 x float> @llvm.x86.tcvtrowd2psi.internal
 return __tile_tcvtrowd2psi(a, 15);
}

__m512bh test_tile_tcvtrowps2pbf16he(__tile1024i a, unsigned b) {
  //CHECK-LABEL: @test_tile_tcvtrowps2pbf16he
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x i16> @llvm.x86.tcvtrowps2pbf16he.internal
 return __tile_tcvtrowps2pbf16he(a, b);
}

__m512bh test_tile_tcvtrowps2pbf16hi(__tile1024i a) {
  //CHECK-LABEL: @test_tile_tcvtrowps2pbf16hi
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x i16> @llvm.x86.tcvtrowps2pbf16hi.internal
 return __tile_tcvtrowps2pbf16hi(a, 15);
}

__m512bh test_tile_tcvtrowps2pbf16le(__tile1024i a, unsigned b) {
  //CHECK-LABEL: @test_tile_tcvtrowps2pbf16le
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x i16> @llvm.x86.tcvtrowps2pbf16le.internal
 return __tile_tcvtrowps2pbf16le(a, b);
}

__m512bh test_tile_tcvtrowps2pbf16li(__tile1024i a) {
  //CHECK-LABEL: @test_tile_tcvtrowps2pbf16li
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x i16> @llvm.x86.tcvtrowps2pbf16li.internal
 return __tile_tcvtrowps2pbf16li(a, 15);
}

__m512h test_tile_tcvtrowps2phhe(__tile1024i a, unsigned b) {
  //CHECK-LABEL: @test_tile_tcvtrowps2phhe
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x half> @llvm.x86.tcvtrowps2phhe.internal
 return __tile_tcvtrowps2phhe(a, b);
}

__m512h test_tile_tcvtrowps2phhi(__tile1024i a) {
  //CHECK-LABEL: @test_tile_tcvtrowps2phhi
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x half> @llvm.x86.tcvtrowps2phhi.internal
 return __tile_tcvtrowps2phhi(a, 15);
}

__m512h test_tile_tcvtrowps2phle(__tile1024i a, unsigned b) {
  //CHECK-LABEL: @test_tile_tcvtrowps2phle
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x half> @llvm.x86.tcvtrowps2phle.internal
 return __tile_tcvtrowps2phle(a, b);
}

__m512h test_tile_tcvtrowps2phli(__tile1024i a) {
  //CHECK-LABEL: @test_tile_tcvtrowps2phli
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x half> @llvm.x86.tcvtrowps2phli.internal
 return __tile_tcvtrowps2phli(a, 15);
}
