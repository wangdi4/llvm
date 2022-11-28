// REQUIRES: intel_feature_isa_amx_avx512
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile -target-feature +amx-avx512 \
// RUN: -target-feature +avx512fp16 -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m512 test_tile_cvtrowd2ps(unsigned int A) {
  // CHECK-LABEL: @test_tile_cvtrowd2ps(
  // CHECK: call <16 x float> @llvm.x86.tcvtrowd2pse(i8 1, i32 %{{.*}})
  return _tile_cvtrowd2ps(1, A);
}

__m512 test_tile_cvtrowd2psi(void) {
  // CHECK-LABEL: @test_tile_cvtrowd2psi(
  // CHECK: call <16 x float> @llvm.x86.tcvtrowd2psi(i8 1, i32 127)
  return _tile_cvtrowd2psi(1, 127);
}

__m512bh test_tile_cvtrowps2pbf16h(unsigned int A) {
  // CHECK-LABEL: @test_tile_cvtrowps2pbf16h(
  // CHECK: call <32 x i16> @llvm.x86.tcvtrowps2pbf16he(i8 1, i32 %{{.*}})
  return _tile_cvtrowps2pbf16h(1, A);
}

__m512bh test_tile_cvtrowps2pbf16hi(void) {
  // CHECK-LABEL: @test_tile_cvtrowps2pbf16hi(
  // CHECK: call <32 x i16> @llvm.x86.tcvtrowps2pbf16hi(i8 1, i32 127)
  return _tile_cvtrowps2pbf16hi(1, 127);
}

__m512bh test_tile_cvtrowps2pbf16l(unsigned int A) {
  // CHECK-LABEL: @test_tile_cvtrowps2pbf16l(
  // CHECK: call <32 x i16> @llvm.x86.tcvtrowps2pbf16le(i8 1, i32 %{{.*}})
  return _tile_cvtrowps2pbf16l(1, A);
}

__m512bh test_tile_cvtrowps2pbf16li(void) {
  // CHECK-LABEL: @test_tile_cvtrowps2pbf16li(
  // CHECK: call <32 x i16> @llvm.x86.tcvtrowps2pbf16li(i8 1, i32 127)
  return _tile_cvtrowps2pbf16li(1, 127);
}

__m512h test_tile_cvtrowps2phh(unsigned int A) {
  // CHECK-LABEL: @test_tile_cvtrowps2phh(
  // CHECK: call <32 x half> @llvm.x86.tcvtrowps2phhe(i8 1, i32 %{{.*}})
  return _tile_cvtrowps2phh(1, A);
}

__m512h test_tile_cvtrowps2phhi(void) {
  // CHECK-LABEL: @test_tile_cvtrowps2phhi(
  // CHECK: call <32 x half> @llvm.x86.tcvtrowps2phhi(i8 1, i32 127)
  return _tile_cvtrowps2phhi(1, 127);
}

__m512h test_tile_cvtrowps2phl(unsigned int A) {
  // CHECK-LABEL: @test_tile_cvtrowps2phl(
  // CHECK: call <32 x half> @llvm.x86.tcvtrowps2phle(i8 1, i32 %{{.*}})
  return _tile_cvtrowps2phl(1, A);
}

__m512h test_tile_cvtrowps2phli(void) {
  // CHECK-LABEL: @test_tile_cvtrowps2phli(
  // CHECK: call <32 x half> @llvm.x86.tcvtrowps2phli(i8 1, i32 127)
  return _tile_cvtrowps2phli(1, 127);
}

void test_tile_movrowi(void) {
  // CHECK-LABEL: @test_tile_movrowi
  // CHECK: %0 = call <16 x float> @llvm.x86.tilemovei(i8 1, i8 2)
  _tile_movrowi(1, 2);
}

typedef unsigned int uint32_t;
void test_tile_movrow(uint32_t A) {
  // CHECK-LABEL: @test_tile_movrow
  // CHECK: %1 = call <16 x float> @llvm.x86.tilemovee(i8 1, i32 %0)
  _tile_movrow(1, A);
}

typedef float __m128 __attribute__((__vector_size__(16)));
void test_tile_movrowx(__m128 A) {
  // CHECK-LABEL: @test_tile_movrowx
  // CHECK: %1 = call <16 x float> @llvm.x86.tilemovex(i8 1, <4 x float> %0)
  _tile_movrowx(1, A);
}
