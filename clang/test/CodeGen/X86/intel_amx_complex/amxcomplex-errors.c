// REQUIRES: intel_feature_isa_amx_complex
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-complex -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>
void test_tile_tcmmimfp16ps() {
  _tile_tcmmimfp16ps(16, 2, 3); // expected-error {{argument value 16 is outside the valid range [0, 7]}}
  _tile_tcmmimfp16ps(1, 26, 3); // expected-error {{argument value 26 is outside the valid range [0, 7]}}
  _tile_tcmmimfp16ps(1, 2, 36); // expected-error {{argument value 36 is outside the valid range [0, 7]}}
  _tile_tcmmimfp16ps(1, 1, 3);  // expected-error {{tile arguments must refer to different tiles}}
}

void test_tile_tcmmrlfp16ps() {
  _tile_tcmmrlfp16ps(16, 2, 3); // expected-error {{argument value 16 is outside the valid range [0, 7]}}
  _tile_tcmmrlfp16ps(1, 26, 3); // expected-error {{argument value 26 is outside the valid range [0, 7]}}
  _tile_tcmmrlfp16ps(1, 2, 36); // expected-error {{argument value 36 is outside the valid range [0, 7]}}
  _tile_tcmmrlfp16ps(1, 1, 3);  // expected-error {{tile arguments must refer to different tiles}}
}

void test_tile_tconjcmmimfp16ps() {
  _tile_tconjcmmimfp16ps(16, 2, 3); // expected-error {{argument value 16 is outside the valid range [0, 7]}}
  _tile_tconjcmmimfp16ps(1, 26, 3); // expected-error {{argument value 26 is outside the valid range [0, 7]}}
  _tile_tconjcmmimfp16ps(1, 2, 36); // expected-error {{argument value 36 is outside the valid range [0, 7]}}
  _tile_tconjcmmimfp16ps(1, 2, 1);  // expected-error {{tile arguments must refer to different tiles}}
}

void test_tile_tconjfp16() {
  _tile_tconjfp16(16, 2); // expected-error {{argument value 16 is outside the valid range [0, 7]}}
  _tile_tconjfp16(1, 26); // expected-error {{argument value 26 is outside the valid range [0, 7]}}
}

void test_tile_ttcmmimfp16ps() {
  _tile_ttcmmimfp16ps(16, 2, 3); // expected-error {{argument value 16 is outside the valid range [0, 7]}}
  _tile_ttcmmimfp16ps(1, 26, 3); // expected-error {{argument value 26 is outside the valid range [0, 7]}}
  _tile_ttcmmimfp16ps(1, 2, 36); // expected-error {{argument value 36 is outside the valid range [0, 7]}}
  _tile_ttcmmimfp16ps(1, 1, 3);  // expected-error {{tile arguments must refer to different tiles}}
}

void test_tile_ttcmmrlfp16ps() {
  _tile_ttcmmrlfp16ps(16, 2, 3); // expected-error {{argument value 16 is outside the valid range [0, 7]}}
  _tile_ttcmmrlfp16ps(1, 26, 3); // expected-error {{argument value 26 is outside the valid range [0, 7]}}
  _tile_ttcmmrlfp16ps(1, 2, 36); // expected-error {{argument value 36 is outside the valid range [0, 7]}}
  _tile_ttcmmrlfp16ps(1, 1, 3);  // expected-error {{tile arguments must refer to different tiles}}
}

