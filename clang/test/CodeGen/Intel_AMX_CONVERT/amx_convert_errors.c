// REQUIRES: intel_feature_isa_amx_convert
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-convert -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>
// AMX-CONVERT
void test_tile_cvt2ps2bf16(void *A, size_t B) {
  _tile_cvt2ps2bf16(A, B, 32, 2); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_cvt2ps2bf16(A, B, 1, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_cvt2ps2ph(void *A, size_t B) {
  _tile_cvt2ps2ph(A, B, 32, 4); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_cvt2ps2ph(A, B, 2, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_amxconvert_cvtd2ps(void *A, size_t B) {
  _tile_amxconvert_cvtd2ps(A, B, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_amxconvert_cvtps2bf16(void *A, size_t B) {
  _tile_amxconvert_cvtps2bf16(A, B, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_cvtps2ph(void *A, size_t B) {
  _tile_cvtps2ph(A, B, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}
