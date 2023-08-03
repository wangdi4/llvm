// REQUIRES: intel_feature_isa_amx_tile_evex
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-tile -target-feature +amx-tile-evex    \
// RUN: -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>

// TILE-EVEX
void test_tile_loadde(const void * base, size_t stride) {
  _tile_loadde(32, base, stride); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tileloaddt164e(const void * base, size_t stride) {
  _tile_stream_loadde(32, base, stride); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tilestored64e(const void * base, size_t stride) {
  _tile_storede(32, base, stride); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tilemove() {
  _tile_tilemove(1, 1); // expected-error {{tile arguments must refer to different tiles}}
  _tile_tilemove(32, 2); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_tilemove(1, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tilezeroe() {
  _tile_zeroe(32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}
