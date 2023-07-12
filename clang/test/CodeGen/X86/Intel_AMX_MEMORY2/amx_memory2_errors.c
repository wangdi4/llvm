// REQUIRES: intel_feature_isa_amx_memory2
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile -target-feature +amx-memory2 \
// RUN: -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>

// Memory2
void test_tile_broadcastrowd(const void *A) {
  _tile_broadcastrowd(8, A); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_storehd(void *A, size_t B) {
  _tile_storehd(A, B, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_storehdt1(void *A, size_t B) {
  _tile_storehdt1(A, B, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_storentd(void *A, size_t B) {
  _tile_storentd(A, B, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_storeqd(void *A, size_t B) {
  _tile_storeqd(A, B, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_storeqdt1(void *A, size_t B) {
  _tile_storeqdt1(A, B, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_storerowd(void *A) {
  _tile_storerowd(A, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}
