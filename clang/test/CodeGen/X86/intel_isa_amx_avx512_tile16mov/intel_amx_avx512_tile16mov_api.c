// REQUIRES: intel_feature_isa_amx_avx512_tile16mov
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-int8 -target-feature +amx-bf16 -target-feature +amx-transpose -target-feature +amx-avx512-tile16mov \
// RUN: -target-feature +avx512f -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression| FileCheck %s

#include <immintrin.h>
#include <stddef.h>

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
