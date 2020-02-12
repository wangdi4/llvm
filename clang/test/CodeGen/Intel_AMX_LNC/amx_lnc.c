// REQUIRES: intel_feature_isa_amx_lnc
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-int8 -target-feature +amx-bf16 -target-feature +amx-int8-evex -target-feature +amx-bf16-evex -target-feature +amx-tile-evex -target-feature\
// RUN: +amx-transpose -target-feature +amx-avx512 -target-feature +avx512f -target-feature +amx-element-evex -emit-llvm -o - -Wall -Werror -pedantic \
// RUN: -Wno-gnu-statement-expression| FileCheck %s

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

void test_tile_2transposew(const void *A, size_t B, size_t C) {
  // CHECK-LABEL: @test_tile_2transposew
  // CHECK: call void @llvm.x86.t2transposew(i8 1, i8* %{{.*}}, i64 %{{.*}}, i64 %{{.*}})
  _tile_2transposew(1, A, B, C);
}

void test_tile_2transposewt1(const void *A, size_t B, size_t C) {
  // CHECK-LABEL: @test_tile_2transposewt1
  // CHECK: call void @llvm.x86.t2transposewt1(i8 1, i8* %{{.*}}, i64 %{{.*}}, i64 %{{.*}})
  _tile_2transposewt1(1, A, B, C);
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

// BF16-EVEX
void test_tile_dpbf16pse() {
  // CHECK-LABEL: @test_tile_dpbf16pse
  // CHECK: call void @llvm.x86.tdpbf16pse(i8 1, i8 2, i8 3)
  _tile_dpbf16pse(1, 2, 3);
}

// INT8-EVEX
void test_tile_tdpbssde() {
  // CHECK-LABEL: @test_tile_tdpbssde
  // CHECK: call void @llvm.x86.tdpbssde(i8 1, i8 2, i8 3)
  _tile_dpbssde(1, 2, 3);
}

void test_tile_tdpbsude() {
  // CHECK-LABEL: @test_tile_tdpbsude
  // CHECK: call void @llvm.x86.tdpbsude(i8 1, i8 2, i8 3)
  _tile_dpbsude(1, 2, 3);
}

void test_tile_tdpbusde() {
  // CHECK-LABEL: @test_tile_tdpbusde
  // CHECK: call void @llvm.x86.tdpbusde(i8 1, i8 2, i8 3)
  _tile_dpbusde(1, 2, 3);
}

void test_tile_tdpbuude() {
  // CHECK-LABEL: @test_tile_tdpbuude
  // CHECK: call void @llvm.x86.tdpbuude(i8 1, i8 2, i8 3)
  _tile_dpbuude(1, 2, 3);
}

// TILE-EVEX
void test_tile_loadde(const void * base, size_t stride) {
  // CHECK-LABEL: @test_tile_loadde
  // CHECK: call void @llvm.x86.tileloadde64(i8 1, i8* %0, i64 %1)
  _tile_loadde(1, base, stride);
}

void test_tile_tileloaddt164e(const void * base, size_t stride) {
  // CHECK-LABEL: @test_tile_tileloaddt164e
  // CHECK: call void @llvm.x86.tileloaddt1e64(i8 1, i8* %0, i64 %1)
  _tile_stream_loadde(1, base, stride);
}

void test_tile_tilestored64e(const void * base, size_t stride) {
  // CHECK-LABEL: @test_tile_tilestored64e
  // CHECK: call void @llvm.x86.tilestorede64(i8 1, i8* %0, i64 %1)
  _tile_storede(1, base, stride);
}

void test_tile_tilemove() {
  // CHECK-LABEL: @test_tile_tilemove
  // CHECK: call void @llvm.x86.tilemove(i8 1, i8 2)
  _tile_tilemove(1, 2);
}

void test_tile_tilezeroe() {
  // CHECK-LABEL: @test_tile_tilezeroe
  // CHECK: call void @llvm.x86.tilezeroe(i8 1)
  _tile_zeroe(1);
}

//AMX_LNC
void test_tile_cvtd2pse(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_cvtd2pse
  // CHECK: call void @llvm.x86.tcvtd2pse{{.*}}, i64 %{{.*}}, i8 1)
  _tile_cvtd2pse(A, B, 1);
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
