// REQUIRES: intel_feature_isa_amx
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile -target-feature +amx-int8 -target-feature +amx-bf16 -emit-llvm -fsyntax-only -verify

#include <immintrin.h>

void test_amx(void *data) {
  _tile_zero(8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
  _tile_loadd(9, data, 8); // expected-error {{argument value 9 is outside the valid range [0, 7]}}
  _tile_stream_loadd(13, data, 1); // expected-error {{argument value 13 is outside the valid range [0, 7]}}
  _tile_stored(88, data, 1); // expected-error {{argument value 88 is outside the valid range [0, 7]}}
  _tile_dpbssd(8, 2, 3); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
  _tile_dpbssd(0, 8, 3); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
  _tile_dpbuud(0, 2, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
  _tile_dpbsud(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbsud(7, 1, 7); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbsud(4, 3, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbf16ps(4, 3, 3); // expected-error {{tmul arguments must refer to different tiles}}
}
