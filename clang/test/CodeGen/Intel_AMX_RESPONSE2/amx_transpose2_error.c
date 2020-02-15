// REQUIRES: intel_feature_isa_amx_transpose2
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-fp16 -target-feature +amx-transpose2 \
// RUN: -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>

// Transpose2
void test_tile_2transposew(const void *A, size_t B, unsigned short C) {
  _tile_2transposew(16, A, B, C); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}
void test_tile_2transposewt1(const void *A, size_t B, unsigned short C) {
  _tile_2transposewt1(16, A, B, C); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}
