// REQUIRES: intel_feature_isa_amx_avx512_cvtrow
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown  -target-feature +avx512f  -target-feature +amx-bf16  \
// RUN: -target-feature +amx-avx512-cvtrow -target-feature +avx512fp16 \
// RUN: -emit-llvm -o - -Werror -pedantic | FileCheck %s --check-prefixes=CHECK

#include <immintrin.h>

char buf[1024];
#define STRIDE 32

char buf2[1024];

__m512 test_tile_tcvtrowd2psee(__tile1024i a, unsigned b) {
  //CHECK-LABEL: @test_tile_tcvtrowd2psee
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <16 x float> @llvm.x86.tcvtrowd2psee.internal
 return __tile_tcvtrowd2psee(a, b);
}

__m512 test_tile_tcvtrowd2psei(__tile1024i a) {
  //CHECK-LABEL: @test_tile_tcvtrowd2psei
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <16 x float> @llvm.x86.tcvtrowd2psei.internal
 return __tile_tcvtrowd2psei(a, 15);
}

__m512bh test_tile_tcvtrowps2pbf16hee(__tile1024i a, unsigned b) {
  //CHECK-LABEL: @test_tile_tcvtrowps2pbf16hee
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x i16> @llvm.x86.tcvtrowps2pbf16hee.internal
 return __tile_tcvtrowps2pbf16hee(a, b);
}

__m512bh test_tile_tcvtrowps2pbf16hei(__tile1024i a) {
  //CHECK-LABEL: @test_tile_tcvtrowps2pbf16hei
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x i16> @llvm.x86.tcvtrowps2pbf16hei.internal
 return __tile_tcvtrowps2pbf16hei(a, 15);
}

__m512bh test_tile_tcvtrowps2pbf16lee(__tile1024i a, unsigned b) {
  //CHECK-LABEL: @test_tile_tcvtrowps2pbf16lee
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x i16> @llvm.x86.tcvtrowps2pbf16lee.internal
 return __tile_tcvtrowps2pbf16lee(a, b);
}

__m512bh test_tile_tcvtrowps2pbf16lei(__tile1024i a) {
  //CHECK-LABEL: @test_tile_tcvtrowps2pbf16lei
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x i16> @llvm.x86.tcvtrowps2pbf16lei.internal
 return __tile_tcvtrowps2pbf16lei(a, 15);
}

__m512h test_tile_tcvtrowps2phhee(__tile1024i a, unsigned b) {
  //CHECK-LABEL: @test_tile_tcvtrowps2phhee
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x half> @llvm.x86.tcvtrowps2phhee.internal
 return __tile_tcvtrowps2phhee(a, b);
}

__m512h test_tile_tcvtrowps2phhei(__tile1024i a) {
  //CHECK-LABEL: @test_tile_tcvtrowps2phhei
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x half> @llvm.x86.tcvtrowps2phhei.internal
 return __tile_tcvtrowps2phhei(a, 15);
}

__m512h test_tile_tcvtrowps2phlee(__tile1024i a, unsigned b) {
  //CHECK-LABEL: @test_tile_tcvtrowps2phlee
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x half> @llvm.x86.tcvtrowps2phlee.internal
 return __tile_tcvtrowps2phlee(a, b);
}

__m512h test_tile_tcvtrowps2phlei(__tile1024i a) {
  //CHECK-LABEL: @test_tile_tcvtrowps2phlei
  //CHECK-DAG: call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> {{%.*}})
  //CHECK-DAG: call <32 x half> @llvm.x86.tcvtrowps2phlei.internal
 return __tile_tcvtrowps2phlei(a, 15);
}
