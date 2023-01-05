// REQUIRES: intel_feature_isa_amx_avx512_tile16mov
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-tile -target-feature +avx512f -target-feature +amx-avx512-tile16mov -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>

// AMXAVX512
typedef float __m512 __attribute__((__vector_size__(64)));
void test_tile_mov16zmm(__m512 tsrc1, __m512 tsrc2, __m512 tsrc3, __m512 tsrc4,
  __m512 tsrc5, __m512 tsrc6, __m512 tsrc7, __m512 tsrc8, __m512 tsrc9,
  __m512 tsrc10, __m512 tsrc11, __m512 tsrc12, __m512 tsrc13, __m512 tsrc14,
  __m512 tsrc15, __m512 tsrc16) {
  _tile_tile16move(16, tsrc1, tsrc2, tsrc3, tsrc4, tsrc5, tsrc6, tsrc7, tsrc8,// expected-error {{argument value 16 is outside the valid range [0, 7]}}
            tsrc9, tsrc10, tsrc11, tsrc12, tsrc13, tsrc14, tsrc15, tsrc16);
}
