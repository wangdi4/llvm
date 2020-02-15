// REQUIRES: intel_feature_isa_amx_lnc
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-int8 -target-feature +amx-bf16 -target-feature +amx-transpose -target-feature +amx-avx512 \
// RUN: -target-feature +avx512f -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression| FileCheck %s

#include <immintrin.h>
#include <stddef.h>
// Transpose
void test_tile_2rpntlvw(const void *A, size_t B, size_t C) {
  // CHECK-LABEL: @test_tile_2rpntlvw
  // CHECK: call void @llvm.x86.t2rpntlvw(i8 1, i8* %{{.*}}, i64 %{{.*}}, i64 %{{.*}})
  _tile_2rpntlvw(1, A, B, C);
}

void test_tile_2rpntlvwt1(const void *A, size_t B, size_t C) {
  // CHECK-LABEL: @test_tile_2rpntlvwt1
  // CHECK: call void @llvm.x86.t2rpntlvwt1(i8 1, i8* %{{.*}}, i64 %{{.*}}, i64 %{{.*}})
  _tile_2rpntlvwt1(1, A, B, C);
}

typedef float __m512 __attribute__((__vector_size__(64)));
// Tile to AVX512
void test_tile_mov16zmm(__m512 tsrc1, __m512 tsrc2, __m512 tsrc3, __m512 tsrc4,
  __m512 tsrc5, __m512 tsrc6, __m512 tsrc7, __m512 tsrc8, __m512 tsrc9,
  __m512 tsrc10, __m512 tsrc11, __m512 tsrc12, __m512 tsrc13, __m512 tsrc14,
  __m512 tsrc15, __m512 tsrc16) {
  // CHECK-LABEL: @test_tile_mov16zmm
  // CHECK: call void @llvm.x86.tile16move(i8 1, <16 x float> %0,{{.*}}<16 x float> %15
  _tile_tile16move(1, tsrc1, tsrc2, tsrc3, tsrc4, tsrc5, tsrc6, tsrc7, tsrc8,
              tsrc9, tsrc10, tsrc11, tsrc12, tsrc13, tsrc14, tsrc15, tsrc16);
}

void test_tile_tilemovrowei() {
  // CHECK-LABEL: @test_tile_tilemovrowei
  // CHECK: %0 = call <16 x float> @llvm.x86.tilemovei(i8 1, i8 2)
  _tile_tilemovrowei(1, 2);
}

typedef unsigned int uint32_t;
void test_tile_tilemovrowee(uint32_t A) {
  // CHECK-LABEL: @test_tile_tilemovrowee
  // CHECK: %1 = call <16 x float> @llvm.x86.tilemovee(i8 1, i32 %0)
  _tile_tilemovrowee(1, A);
}

typedef float __m128 __attribute__((__vector_size__(16)));
void test_tile_tilemovrowex(__m128 A) {
  // CHECK-LABEL: @test_tile_tilemovrowex
  // CHECK: %1 = call <16 x float> @llvm.x86.tilemovex(i8 1, <4 x float> %0)
  _tile_tilemovrowex(1, A);
}

void test_tile_cvtrowd2psei() {
  // CHECK-LABEL: @test_tile_cvtrowd2psei
  // CHECK: %0 = call <16 x float> @llvm.x86.tcvtrowd2psei(i8 1, i32 2)
  _tile_cvtrowd2psei(1, 2);
}

typedef unsigned int uint32_t;
void test_tile_cvtrowd2psee(uint32_t A) {
  // CHECK-LABEL: @test_tile_cvtrowd2psee
  // CHECK: %1 = call <16 x float> @llvm.x86.tcvtrowd2psee(i8 1, i32 %0)
  _tile_cvtrowd2psee(1, A);
}
