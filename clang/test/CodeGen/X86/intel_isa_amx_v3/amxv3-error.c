// REQUIRES: intel_feature_isa_amx_v3
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-v3 \
// RUN:  -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>

void test_tile_loadtransposed(const void *A, size_t B) {
  _tile_loadtransposed(8, A, B); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_loadtransposedt1(const void *A, size_t B) {
  _tile_loadtransposedt1(8, A, B); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_rpntlvwz0(const void *A, size_t B) {
  _tile_rpntlvwz0(8, A, B); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_rpntlvwz0t1(const void *A, size_t B) {
  _tile_rpntlvwz0t1(8, A, B); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_rpntlvwz1(const void *A, size_t B) {
  _tile_rpntlvwz1(8, A, B); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_rpntlvwz1t1(const void *A, size_t B) {
  _tile_rpntlvwz1t1(8, A, B); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_storetransposed(void *A, size_t B) {
  _tile_storetransposed(A, B, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}
