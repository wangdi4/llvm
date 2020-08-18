// REQUIRES: intel_feature_isa_amx_bf16_evex
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile \
// RUN: -target-feature +amx-bf16-evex -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>
// BF16-EVEX
void test_tile_dpbf16pse() {
  _tile_dpbf16pse(1, 1, 3); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpbf16pse(1, 2, 2); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpbf16pse(1, 2, 1); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpbf16pse(32, 2, 1); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbf16pse(1, 32, 3); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbf16pse(1, 2, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}
