// REQUIRES: intel_feature_isa_amx_avx512_cvtrow
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile -target-feature +amx-avx512-cvtrow \
// RUN: -target-feature +avx512fp16 -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m512 test_tile_tcvtrowd2psee(unsigned int A) {
  // CHECK-LABEL: @test_tile_tcvtrowd2psee(
  // CHECK: call <16 x float> @llvm.x86.tcvtrowd2psee(i8 1, i32 %{{.*}})
  return _tile_tcvtrowd2psee(1, A);
}

__m512 test_tile_tcvtrowd2psei(void) {
  // CHECK-LABEL: @test_tile_tcvtrowd2psei(
  // CHECK: call <16 x float> @llvm.x86.tcvtrowd2psei(i8 1, i32 127)
  return _tile_tcvtrowd2psei(1, 127);
}

__m512bh test_tile_tcvtrowps2pbf16hee(unsigned int A) {
  // CHECK-LABEL: @test_tile_tcvtrowps2pbf16hee(
  // CHECK: call <32 x i16> @llvm.x86.tcvtrowps2pbf16hee(i8 1, i32 %{{.*}})
  return _tile_tcvtrowps2pbf16hee(1, A);
}

__m512bh test_tile_tcvtrowps2pbf16hei(void) {
  // CHECK-LABEL: @test_tile_tcvtrowps2pbf16hei(
  // CHECK: call <32 x i16> @llvm.x86.tcvtrowps2pbf16hei(i8 1, i32 127)
  return _tile_tcvtrowps2pbf16hei(1, 127);
}

__m512bh test_tile_tcvtrowps2pbf16lee(unsigned int A) {
  // CHECK-LABEL: @test_tile_tcvtrowps2pbf16lee(
  // CHECK: call <32 x i16> @llvm.x86.tcvtrowps2pbf16lee(i8 1, i32 %{{.*}})
  return _tile_tcvtrowps2pbf16lee(1, A);
}

__m512bh test_tile_tcvtrowps2pbf16lei(void) {
  // CHECK-LABEL: @test_tile_tcvtrowps2pbf16lei(
  // CHECK: call <32 x i16> @llvm.x86.tcvtrowps2pbf16lei(i8 1, i32 127)
  return _tile_tcvtrowps2pbf16lei(1, 127);
}

__m512h test_tile_tcvtrowps2phhee(unsigned int A) {
  // CHECK-LABEL: @test_tile_tcvtrowps2phhee(
  // CHECK: call <32 x half> @llvm.x86.tcvtrowps2phhee(i8 1, i32 %{{.*}})
  return _tile_tcvtrowps2phhee(1, A);
}

__m512h test_tile_tcvtrowps2phhei(void) {
  // CHECK-LABEL: @test_tile_tcvtrowps2phhei(
  // CHECK: call <32 x half> @llvm.x86.tcvtrowps2phhei(i8 1, i32 127)
  return _tile_tcvtrowps2phhei(1, 127);
}

__m512h test_tile_tcvtrowps2phlee(unsigned int A) {
  // CHECK-LABEL: @test_tile_tcvtrowps2phlee(
  // CHECK: call <32 x half> @llvm.x86.tcvtrowps2phlee(i8 1, i32 %{{.*}})
  return _tile_tcvtrowps2phlee(1, A);
}

__m512h test_tile_tcvtrowps2phlei(void) {
  // CHECK-LABEL: @test_tile_tcvtrowps2phlei(
  // CHECK: call <32 x half> @llvm.x86.tcvtrowps2phlei(i8 1, i32 127)
  return _tile_tcvtrowps2phlei(1, 127);
}
