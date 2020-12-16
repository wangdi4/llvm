// REQUIRES: intel_feature_isa_amx_complex_evex
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-complex-evex -target-feature +avx512fp16 -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>

void test_tile_tcvtrowps2phiee(__m512h A, unsigned int B) {
  _tile_tcvtrowps2phiee(A, 32, B); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tcvtrowps2phiei(__m512h A) {
  _tile_tcvtrowps2phiei(A, 32, 127); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_tcvtrowps2phiei(A, 31, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}
